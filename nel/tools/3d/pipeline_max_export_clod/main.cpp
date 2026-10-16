/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 */
// Lod-character export: .max -> .clod, replicating the NelExportLodCharacter path of the 3ds Max
// plugin (build_gamedata processes/clodbank) without 3ds Max.
//
// The clod maxscript (processes/clodbank/maxscript/clod_export.ms::runNelMaxExport) iterates
// $geometry, keeps only scene-root nodes (parent == undefined) that pass isToBeExported (not
// RklPatch / nel_ps / nel_pacs_* / DONOTEXPORT) and isLodCharacter (NEL3D_APPDATA_CHARACTER_LOD
// == "1"), and emits one .clod per node named after the node. Per node: NelExportLodCharacter
// (nel_export/nel_export_export.cpp) -> CExportNel::buildLodCharacter
// (nel_mesh_lib/export_lod_character.cpp):
//   - require skinning (Physique/Skin); build a temporary skeleton bone-id map
//   - convert to TriObject, build CMeshBase / CMesh via the shared mesh path (skinned WORLD space)
//   - build a CMeshMRM with NLods=1 / Divisor=1 (no poly reduction — historic simpler path than
//     CMesh, which uses matrix blocks)
//   - copy VB positions + SkinWeights + UVs + normals + rdr-pass triangle indices + BonesNames
//     into CLodCharacterShapeBuild; flag material-0 passes for TextureInfo selection; compile()
//   - serial CLodCharacterShapeBuild (NEL_CLODBULD)
//
// Shared infrastructure: scene_lib / mesh_eval / material_build / mesh_build (shape exporter) +
// PHYSIQUESKIN (pipeline_max_export_common — weight-exact since §10z-treize).

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/mem_stream.h>

#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mesh_mrm.h>
#include <nel/3d/mrm_parameters.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/lod_character_shape.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../pipeline_max_export_shape/scene_lib.h"
#include "../pipeline_max_export_shape/mesh_eval.h"
#include "../pipeline_max_export_shape/material_build.h"
#include "../pipeline_max_export_shape/mesh_build.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/physique_skin.h"
#include "../pipeline_max_export_common/export_ids.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;
using namespace MESHBUILD;

// NeL appdata sub-ids + special-object class ids: pipeline_max_export_common/export_ids.h
// (RklPatch: SCENELIB::CLASSID_RPO)
using PMAX_EXPORT_IDS::CLASSID_PACS_BOX;
using PMAX_EXPORT_IDS::CLASSID_PACS_CYL;

static bool g_verbose = false;

// ---------------------------------------------------------------------------------------------

// Is this node a scene-root geometry node eligible for lod-character export?
// Mirrors clod_export.ms::isToBeExported + isLodCharacter (parent == undefined, CHARACTER_LOD).
static bool isToBeExported(INode &node)
{
	// Root only: parent must be the scene root (not a CNodeImpl)
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return false;
	INode *p = n->parent();
	if (p && dynamic_cast<CNodeImpl *>(p))
		return false; // has a non-root parent

	CSceneClass *base = baseObjectOf(node, nullptr, nullptr);
	if (!base) return false;

	NLMISC::CClassId cid = base->classDesc()->classId();
	// Skip RklPatch
	if (cid == CLASSID_RPO)
		return false;
	// Skip nel_ps / pacs prims
	if (cid.a() == CLASSID_PARTA_NEL_PS
	    || cid == CLASSID_PACS_BOX
	    || cid == CLASSID_PACS_CYL)
		return false;

	// DONOTEXPORT
	if (getScriptAppDataStr(n, NEL3D_APPDATA_DONOTEXPORT, "") == "1")
		return false;

	// CHARACTER_LOD must be "1" (maxscript isLodCharacter)
	if (getScriptAppDataStr(n, NEL3D_APPDATA_CHARACTER_LOD, "") != "1")
		return false;

	return true;
}

// Does the node carry a Physique (or Skin) modifier? Lod characters require skinning.
static bool detectSkinning(INode &node,
                           std::vector<CSceneClass *> &mods,
                           std::vector<CStorageContainer *> &modApps,
                           bool &hasPhysique, bool &hasSkin)
{
	hasPhysique = false;
	hasSkin = false;
	baseObjectOf(node, &mods, &modApps);
	for (uint i = 0; i < mods.size(); ++i)
	{
		if (PHYSIQUESKIN::isPhysiqueModifier(mods[i]))
			hasPhysique = true;
		if (PHYSIQUESKIN::isSkinModifier(mods[i]))
			hasSkin = true;
	}
	return hasPhysique || hasSkin;
}

// Build a CLodCharacterShapeBuild from a skinned node — the reference's buildLodCharacter path.
static bool buildLodCharacter(INode &node, SNodeTMCache &tmCache, CSceneClassContainer *ssc,
                              NL3D::CLodCharacterShapeBuild &lodBuild, bool exportLighting)
{
	std::string name = nodeName(node);

	std::vector<CSceneClass *> mods;
	std::vector<CStorageContainer *> modApps;
	bool hasPhysique = false, hasSkin = false;
	if (!detectSkinning(node, mods, modApps, hasPhysique, hasSkin))
	{
		fprintf(stderr, "ERROR: clod '%s' has no skinning (Physique/Skin required)\n", name.c_str());
		return false;
	}

	// Map Extender tag (same convention as the shape exporter): the reference .clod of a
	// mapext-carrying node was exported with the plugin missing (pass-through → the base
	// map channel), while we apply the recovered cache — the reference's UV channel is not
	// a valid oracle for these assets (design doc §9 / §10z-quatorze; the recovered UVs are
	// the correct ones). The harness masks UV comparison on tagged nodes.
	{
		static const NLMISC::CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);
		for (uint i = 0; i < mods.size(); ++i)
		{
			if (mods[i]->classDesc() && mods[i]->classDesc()->classId() == CLASSID_MAP_EXTENDER)
			{
				printf("MAPEXT %s\n", NLMISC::toLowerAscii(name).c_str());
				break;
			}
		}
	}
	if (hasSkin && !hasPhysique)
	{
		// Skin modifier: zero corpus hits under Max 3-era assets; refuse like the shape exporter.
		fprintf(stderr, "ERROR: clod '%s': Skin modifier not implemented (Physique-only corpus)\n",
		        name.c_str());
		return false;
	}

	// Base mesh (materials, transform) — exportLighting is irrelevant for lod characters (no
	// lightmap path) but kept for interface parity with the shared mesh builders.
	Matrix3M localTM = getLocalMatrix(node, tmCache);
	SMaxMeshBaseBuild maxBaseBuild;
	NL3D::CMeshBase::CMeshBaseBuild buildBaseMesh;
	buildBaseMeshInterface(buildBaseMesh, maxBaseBuild, node, tmCache, localTM, exportLighting);

	// Mesh eval (Physique is skipped by mesh_eval — same as the shape path; skinning applied
	// after). Skinned = true → vertices land in WORLD space (export_mesh.cpp:671).
	SEvalMesh evalMesh;
	std::vector<std::string> warnings;
	if (!evalNodeMesh(node, evalMesh, &warnings))
	{
		fprintf(stderr, "ERROR: clod '%s' mesh eval failed", name.c_str());
		for (uint w = 0; w < warnings.size(); ++w)
			fprintf(stderr, " (%s)", warnings[w].c_str());
		fprintf(stderr, "\n");
		return false;
	}
	NL3D::CMesh::CMeshBuild buildMesh;
	buildMeshInterface(evalMesh, buildMesh, buildBaseMesh, maxBaseBuild, node, tmCache,
	                   /*skinned=*/true);

	// Physique → SkinWeights + BonesNames
	std::string err;
	if (!PHYSIQUESKIN::applyPhysiqueSkinning(buildMesh, node, mods, modApps, ssc, &err))
	{
		fprintf(stderr, "ERROR: clod '%s' Physique skinning failed: %s\n", name.c_str(), err.c_str());
		return false;
	}
	buildMesh.VertexFlags |= NL3D::CVertexBuffer::PaletteSkinFlag;

	// Build a CMeshMRM with a single lod and no poly reduction — the reference's historic path
	// (export_lod_character.cpp:94-99): simpler than CMesh because no matrix blocks.
	NL3D::CMRMParameters mrmParams;
	mrmParams.Divisor = 1;
	mrmParams.NLods = 1;
	NL3D::CMeshMRM meshMRM;
	std::vector<NL3D::CMesh::CMeshBuild *> emptyBs;
	meshMRM.build(buildBaseMesh, buildMesh, emptyBs, mrmParams);
	const NL3D::CMeshMRMGeom &meshMRMGeom = meshMRM.getMeshGeom();
	if (meshMRMGeom.getNbLod() != 1)
	{
		fprintf(stderr, "ERROR: clod '%s' MRM produced %u lods (expected 1)\n",
		        name.c_str(), meshMRMGeom.getNbLod());
		return false;
	}

	const NL3D::CVertexBuffer &VB = meshMRMGeom.getVertexBuffer();
	uint32 format = VB.getVertexFormat();
	uint numVerts = VB.getNumVertices();

	// The mesh must have at least skinning + positions.
	if (!(format & NL3D::CVertexBuffer::PositionFlag) || !meshMRMGeom.isSkinned())
	{
		fprintf(stderr, "ERROR: clod '%s' MRM mesh is not skinned or has no positions "
		                "(format=0x%x skinned=%d)\n",
		        name.c_str(), (unsigned)format, (int)meshMRMGeom.isSkinned());
		return false;
	}

	// Vertices + skin weights
	{
		NL3D::CVertexBufferRead vba;
		VB.lock(vba);
		lodBuild.Vertices.resize(numVerts);
		for (uint i = 0; i < numVerts; ++i)
			lodBuild.Vertices[i] = *(const NLMISC::CVector *)vba.getVertexCoordPointer(i);
	}
	lodBuild.SkinWeights = meshMRMGeom.getSkinWeights();
	if (lodBuild.SkinWeights.size() != numVerts)
	{
		fprintf(stderr, "ERROR: clod '%s' SkinWeights size %u != verts %u\n",
		        name.c_str(), (uint)lodBuild.SkinWeights.size(), numVerts);
		return false;
	}

	// UVs + normals (default fill, then overwrite when present)
	lodBuild.UVs.clear();
	lodBuild.Normals.clear();
	lodBuild.UVs.resize(numVerts, NLMISC::CUV(0.f, 0.f));
	lodBuild.Normals.resize(numVerts, NLMISC::CVector::K);
	{
		NL3D::CVertexBufferRead vba;
		VB.lock(vba);
		if (format & NL3D::CVertexBuffer::TexCoord0Flag)
		{
			for (uint i = 0; i < numVerts; ++i)
				lodBuild.UVs[i] = *(const NLMISC::CUV *)vba.getTexCoordPointer(i);
		}
		if (format & NL3D::CVertexBuffer::NormalFlag)
		{
			for (uint i = 0; i < numVerts; ++i)
				lodBuild.Normals[i] = *(const NLMISC::CVector *)vba.getNormalCoordPointer(i);
		}
	}

	// Triangles from every rdr pass of lod 0; flag material-0 passes for TextureInfo selection
	// (export_lod_character.cpp:140-170). Triangle count is buildMesh.Faces.size() (pre-MRM
	// face count — with NLods=1/Divisor=1 MRM does not reduce faces, so this matches).
	std::vector<bool> triangleSelection;
	lodBuild.TriangleIndices.resize(buildMesh.Faces.size() * 3);
	triangleSelection.resize(buildMesh.Faces.size(), false);
	uint dstTriIdx = 0;
	for (uint i = 0; i < meshMRMGeom.getNbRdrPass(0); ++i)
	{
		const NL3D::CIndexBuffer &pb = meshMRMGeom.getRdrPassPrimitiveBlock(0, i);
		NL3D::CIndexBufferRead iba;
		pb.lock(iba);
		if (dstTriIdx + pb.getNumIndexes() > lodBuild.TriangleIndices.size())
		{
			fprintf(stderr, "ERROR: clod '%s' triangle overflow (pass %u: %u indexes, dst %u, cap %u)\n",
			        name.c_str(), i, pb.getNumIndexes(), dstTriIdx,
			        (uint)lodBuild.TriangleIndices.size());
			return false;
		}
		uint32 *dst = &lodBuild.TriangleIndices[dstTriIdx];
		if (pb.getFormat() == NL3D::CIndexBuffer::Indices32)
		{
			memcpy(dst, iba.getPtr(), pb.getNumIndexes() * sizeof(uint32));
		}
		else
		{
			nlassert(pb.getFormat() == NL3D::CIndexBuffer::Indices16);
			const uint16 *src = (const uint16 *)iba.getPtr();
			for (uint n = pb.getNumIndexes(); n > 0; --n)
				*(dst++) = *(src++);
		}
		// Material 0 → TextureInfo selection
		if (meshMRMGeom.getRdrPassMaterial(0, i) == 0)
		{
			for (uint tri = dstTriIdx / 3; tri < dstTriIdx / 3 + pb.getNumIndexes() / 3; ++tri)
				triangleSelection[tri] = true;
		}
		dstTriIdx += pb.getNumIndexes();
	}
	if (dstTriIdx != lodBuild.TriangleIndices.size())
	{
		// With NLods=1 the rdr-pass index total should equal Faces*3. If MRM dropped faces or
		// reorganized, shrink to the actual count (and selection) rather than pad garbage.
		fprintf(stderr, "WARNING: clod '%s' triangle count %u vs expected %u — trimming\n",
		        name.c_str(), dstTriIdx, (uint)lodBuild.TriangleIndices.size());
		lodBuild.TriangleIndices.resize(dstTriIdx);
		triangleSelection.resize(dstTriIdx / 3);
	}

	// Bone names
	lodBuild.BonesNames = meshMRMGeom.getBonesName();

	// Compile texture information (32×32 Pos/Normal map for the clod bank)
	lodBuild.compile(triangleSelection);
	return true;
}

// ---------------------------------------------------------------------------------------------
// Per-file export — whole-file flow shared by the standalone tool and the max2gltf writer
// (PMB_CLOD_NO_MAIN + nel_clods blob list): one code path, the blob and the tool's files
// cannot drift. Serialization uses the export-era stream flags (saved/restored — the writer
// process doesn't set them globally like this tool's main does).

int pmbExportClodsForGltf(PMAXLOAD::SLoadedMax &lm, bool exportLighting,
                          std::vector<std::pair<std::string, std::vector<uint8> > > &out,
                          uint &skipped)
{
	CSceneClassContainer *ssc = lm.Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = nullptr;

	std::vector<INode *> allNodes;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin();
	     it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		allNodes.push_back(node);
	}

	bool oldVB = NL3D::CVertexBuffer::SerialOldPreferredMemory;
	bool oldIB = NL3D::CIndexBuffer::SerialOldPreferredMemory;
	NL3D::CVertexBuffer::SerialOldPreferredMemory = true;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = true;

	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		if (!isToBeExported(node))
			continue;

		std::string name = nodeName(node);

		try
		{
			NL3D::CLodCharacterShapeBuild lodBuild;
			if (!buildLodCharacter(node, tmCache, ssc, lodBuild, exportLighting))
			{
				++skipped;
				continue;
			}

			NLMISC::CMemStream ms;
			lodBuild.serial(ms);
			out.push_back(std::make_pair(name,
				std::vector<uint8>(ms.buffer(), ms.buffer() + ms.length())));
			if (g_verbose)
				printf("OK %s.clod (verts=%u tris=%u bones=%u)\n", name.c_str(),
				       (uint)lodBuild.Vertices.size(),
				       (uint)(lodBuild.TriangleIndices.size() / 3),
				       (uint)lodBuild.BonesNames.size());
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: clod serialization failed for %s: %s\n",
			        name.c_str(), e.what());
			++skipped;
		}
	}

	NL3D::CVertexBuffer::SerialOldPreferredMemory = oldVB;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = oldIB;
	return 0;
}

// ---------------------------------------------------------------------------------------------

#ifndef PMB_CLOD_NO_MAIN
int main(int argc, char **argv)
{
	NLMISC::CApplicationContext appContext;
	NL3D::registerSerial3d();

	// Write export-era stream versions (VB/IB preferred-memory enum); the lod build doesn't
	// serial those containers directly, but CMeshMRM construction touches them.
	NL3D::CVertexBuffer::SerialOldPreferredMemory = true;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = true;

	std::string input, outDir, dbRoot;
	bool exportLighting = true;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v")
			g_verbose = true;
		else if (arg == "--db" && i + 1 < argc)
			dbRoot = argv[++i];
		else if (arg == "--path-alias" && i + 1 < argc)
		{
			std::string kv = argv[++i];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n",
				        kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
		}
		else if (arg == "--no-lighting")
			exportLighting = false;
		else if (input.empty())
			input = arg;
		else if (outDir.empty())
			outDir = arg;
		else
		{
			fprintf(stderr, "unexpected argument: %s\n", arg.c_str());
			return 2;
		}
	}

	if (input.empty() || outDir.empty())
	{
		fprintf(stderr,
		        "usage: pipeline_max_export_clod [--db <graphics-root>] [--no-lighting] [-v] "
		        "<input.max> <outdir>\n");
		return 2;
	}

	// Deduce db root from input path when not given
	if (dbRoot.empty())
	{
		std::string abs = NLMISC::CPath::getFullPath(input, false);
		std::string::size_type p = abs.find("/stuff/");
		if (p == std::string::npos) p = abs.find("/landscape/");
		if (p == std::string::npos) p = abs.find("/sky_v2/");
		if (p != std::string::npos)
			dbRoot = abs.substr(0, p);
	}
	setDatabaseRoot(dbRoot);

	// Ensure outdir exists (corpus harness creates per-file dirs; hand invocation often
	// points at a fresh path).
	NLMISC::CFile::createDirectoryTree(outDir);

	uint skipped = 0;
	std::vector<std::pair<std::string, std::vector<uint8> > > files;
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm))
		return 1;
	int ret = pmbExportClodsForGltf(lm, exportLighting, files, skipped);
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string outPath = outDir + "/" + files[i].first + ".clod";
		NLMISC::COFile file;
		if (!file.open(outPath))
		{
			fprintf(stderr, "ERROR: cannot open %s for writing\n", outPath.c_str());
			++skipped;
			continue;
		}
		try
		{
			file.serialBuffer(&files[i].second[0], (uint)files[i].second.size());
			file.close();
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: write failed for %s: %s\n", outPath.c_str(), e.what());
			++skipped;
		}
	}
	printf("EXPORTED %u clods, %u skipped\n", (uint)files.size(), skipped);
	return ret;
}
#endif /* PMB_CLOD_NO_MAIN */

/* end of file */
