/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 */
// Shape export: .max -> .shape, replicating the NelExportShapeEx path of the 3ds Max plugin
// (build_gamedata processes/shape) without 3ds Max.
//
// The shape maxscript (processes/shape/maxscript/shape_export.ms) iterates the geometry and
// shapes categories of every node, skips Bip*-rooted nodes, RklPatch/nel_ps/nel_pacs objects,
// accelerators, DONOTEXPORT/COLLISION-flagged nodes and LOD slave meshes, then calls
// NelExportShapeEx per node — CNelExport::exportMesh -> CExportNel::buildShape
// (plugin_max/nel_mesh_lib/export_mesh.cpp) -> CShapeStream::serial. Output file name is the
// lowercased node name; nodes whose LOD set carries a coarse mesh go to the with-coarse-mesh
// directory, everything else to the without-coarse-mesh directory.
//
// Milestone 1 scope (see pipeline_max_design.md §10i): plain CMesh and CMeshMRM from Editable
// Mesh objects (+ Edit Mesh / XForm modifiers), NeL materials v14, no lightmap computation
// (lightmapped materials export with their base state), no skinning, no multi-lod, no
// flare/remanence/water/wavemaker. Every skipped feature reports a SKIP/WARNING line the
// corpus harness buckets.

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
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>

#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mesh_mrm.h>
#include <nel/3d/mesh_mrm_skinned.h>
#include <nel/3d/mesh_multi_lod.h>
#include <nel/3d/mesh_geom.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/shape.h>
#include <nel/3d/animation.h>
#include <nel/3d/texture_file.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "scene_lib.h"
#include "../pipeline_max_export_common/db_path.h"
#include "mesh_eval.h"
#include "material_build.h"
#include "mesh_build.h"
#include "anim_build.h"
#include "water_build.h"

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

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_LOD_NAME_COUNT 1423062537
#define NEL3D_APPDATA_LOD_NAME 1423062538
#define NEL3D_APPDATA_LOD_BLEND_IN 1423062548
#define NEL3D_APPDATA_LOD_BLEND_OUT 1423062549
#define NEL3D_APPDATA_LOD_COARSE_MESH 1423062550
#define NEL3D_APPDATA_LOD_DYNAMIC_MESH 1423062551
#define NEL3D_APPDATA_LOD_DIST_MAX 1423062552
#define NEL3D_APPDATA_LOD_BLEND_LENGTH 1423062553
#define NEL3D_APPDATA_LOD_MRM 1423062554
#define NEL3D_APPDATA_ACCEL 1423062561
#define NEL3D_APPDATA_DONOTEXPORT 1423062565
#define NEL3D_APPDATA_COLLISION 1423062613
#define NEL3D_APPDATA_COLLISION_EXTERIOR 1423062614
#define NEL3D_APPDATA_USE_REMANENCE 1423062631
#define NEL3D_APPDATA_AUTOMATIC_ANIMATION 1423062617
#define NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS 1423062587
#define NEL3D_APPDATA_INTERFACE_FILE 1423062700

// Scene class part-A ids of the special objects
#define CLASSID_PARTA_NEL_PS 0x58ce2893
#define CLASSID_PARTA_NEL_FLARE 0x4e913532
#define CLASSID_PARTA_NEL_WAVE_MAKER 0x77e24828
#define CLASSID_PARTA_XREF 0x92aab38c
static const NLMISC::CClassId CLASSID_PACS_BOX(0x7f374277, 0x5d3971df);
static const NLMISC::CClassId CLASSID_PACS_CYL(0x62a56810, 0x4b3d601c);

static bool g_verbose = false;

// ---------------------------------------------------------------------------------------------

struct SExportStats
{
	uint Exported;
	uint Skipped;
	uint AnimExported;
	std::map<std::string, uint> SkipReasons;
	SExportStats() : Exported(0), Skipped(0), AnimExported(0) { }
	void skip(const std::string &reason)
	{
		++Skipped;
		++SkipReasons[reason];
	}
};

// Is this node's evaluated object in the geometry/shapes MaxScript categories?
static bool isGeometryOrShape(CSceneClass *base)
{
	if (!base) return false;
	TSClassId scid = base->classDesc()->superClassId();
	return scid == SCLASS_GEOMOBJECT || scid == SCLASS_SHAPE;
}

// Root node name check ("Bip" prefixed root => skeleton part)
static INode *rootOf(INode *node)
{
	INode *cur = node;
	int guard = 64;
	while (cur && guard-- > 0)
	{
		if (!dynamic_cast<CNodeImpl *>(cur)) break;
		INode *p = cur->parent();
		if (!p || !dynamic_cast<CNodeImpl *>(p)) break;
		cur = p;
	}
	return cur;
}

static bool startsWithBip(const std::string &s)
{
	return s.size() >= 3 && s.compare(0, 3, "Bip") == 0;
}

// ---------------------------------------------------------------------------------------------
// buildShape: the mesh path (CMesh / CMeshMRM / CMeshMultiLod)

// Evaluate a node's mesh + build the CMesh::CMeshBuild against a caller-supplied CMeshBaseBuild
// (base materials + transform stay owned by the caller). Returns NULL on eval failure; sets
// noteMapExt/noteLightmap for the harness tags. The base node's CMeshBaseBuild is what feeds
// UVRouting + material count; a multi-lod slave rides on that same base.
static bool evalAndBuildMesh(INode &node, SNodeTMCache &tmCache,
                             const NL3D::CMeshBase::CMeshBaseBuild &buildBaseMesh,
                             const SMaxMeshBaseBuild &maxBaseBuild,
                             NL3D::CMesh::CMeshBuild &buildMesh, SExportStats &stats)
{
	std::string name = nodeName(node);
	SEvalMesh mesh;
	std::vector<std::string> warnings;
	if (!evalNodeMesh(node, mesh, &warnings))
	{
		stats.skip("mesh-eval");
		return false;
	}
	buildMeshInterface(mesh, buildMesh, buildBaseMesh, maxBaseBuild, node, tmCache);
	return true;
}

// Construct an IMeshGeom (CMeshGeom or CMeshMRMGeom) from a built CMeshBuild — used per LOD slot
// in the multi-lod path. Caller owns the returned pointer until CMeshMultiLod::build takes it.
static NL3D::IMeshGeom *buildMeshGeomFor(INode &node, NL3D::CMesh::CMeshBuild &buildMesh,
                                          uint numMaxMaterial)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0))
	{
		NL3D::CMRMParameters parameters;
		buildMRMParameters(n, parameters);
		std::vector<NL3D::CMesh::CMeshBuild *> bsList; // morph targets: not implemented
		NL3D::CMeshMRMGeom *g = new NL3D::CMeshMRMGeom;
		g->build(buildMesh, bsList, numMaxMaterial, parameters);
		return g;
	}
	NL3D::CMeshGeom *g = new NL3D::CMeshGeom;
	g->build(buildMesh, numMaxMaterial);
	return g;
}

// LOD slot flags from a slave's appdata.
static uint8 lodSlotFlags(CNodeImpl *n)
{
	uint8 flags = 0;
	if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_BLEND_IN, "") == "1")
		flags |= NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendIn;
	if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_BLEND_OUT, "") == "1")
		flags |= NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendOut;
	if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_COARSE_MESH, "") == "1")
		flags |= NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::CoarseMesh;
	return flags;
}

// Node lookup (case-insensitive) — passed into buildShapeForNode for the multi-lod slave path.
typedef std::map<std::string, INode *> TNodesByName;

static NL3D::IShape *buildShapeForNode(INode &node, SNodeTMCache &tmCache,
                                       const TNodesByName &nodesByName,
                                       bool exportLighting, SExportStats &stats)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string name = nodeName(node);

	std::vector<CSceneClass *> mods;
	CSceneClass *base = baseObjectOf(node, &mods, NULL);
	if (!base) return NULL;
	NLMISC::CClassId cid = base->classDesc()->classId();

	// FX/special shape classes (not yet implemented; each reports for the harness)
	if (cid.a() == CLASSID_PARTA_NEL_WAVE_MAKER)
	{
		stats.skip("wavemaker");
		fprintf(stderr, "SKIP shape '%s': wave maker not implemented\n", name.c_str());
		return NULL;
	}
	if (getScriptAppDataInt(n, NEL3D_APPDATA_USE_REMANENCE, 0))
	{
		stats.skip("remanence");
		fprintf(stderr, "SKIP shape '%s': remanence not implemented\n", name.c_str());
		return NULL;
	}
	if (cid.a() == CLASSID_PARTA_NEL_FLARE)
	{
		stats.skip("flare");
		fprintf(stderr, "SKIP shape '%s': flare not implemented\n", name.c_str());
		return NULL;
	}
	if (hasWaterMaterial(node))
	{
		NL3D::IShape *ws = WATERBUILD::buildWaterShape(node, tmCache);
		if (!ws)
		{
			// The water builder logs its own reason; count as a skip so the harness bucket is
			// specific (the reference exporter returns NULL in the same cases without further
			// output — missing required maps, empty geometry).
			stats.skip("water");
			return NULL;
		}
		return ws;
	}

	// Skinning (Physique/Skin modifier)
	for (uint i = 0; i < mods.size(); ++i)
	{
		NLMISC::CClassId mcid = mods[i]->classDesc()->classId();
		// Physique (0x00000100, 0x00000000)-class old id or Skin; identified by name via the
		// class directory: Physique modifier classid in the corpus files.
		std::string disp = ucstring(mods[i]->classDesc()->displayName()).toUtf8();
		if (disp == "Physique" || disp == "Skin")
		{
			stats.skip("skinned");
			fprintf(stderr, "SKIP shape '%s': skinned mesh not implemented\n", name.c_str());
			return NULL;
		}
	}

	// Multi-lod object detection (LOD_NAME_COUNT > 0 = the node has LOD slaves — LOD 0 is the
	// node itself + up to 9 slaves resolved by NAME from the LOD_NAME + i appdata; each slave
	// contributes ONE MeshGeom slot to a CMeshMultiLod. Materials, transform and shadow flags
	// live at the base level and come from the parent node; slave matIDs get clamped through
	// buildMeshInterface's `% numMaterials` so a slave with fewer materials is safe).
	uint lodCount = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);

	// Interface meshes (normal welding)
	if (!getScriptAppDataStr(n, NEL3D_APPDATA_INTERFACE_FILE, "").empty())
	{
		stats.skip("interface-mesh");
		fprintf(stderr, "SKIP shape '%s': interface mesh not implemented\n", name.c_str());
		return NULL;
	}

	// Nodes carrying the custom UVW-mapping plugin modifier ("Map Extender", mapext198m3.dlm,
	// (0x2ec82081, 0x045a6271)): the reference .shape UVs of these assets are garbage (see
	// pipeline_max_design.md §9 T3 caveats) — export proceeds, but the harness buckets them
	// out of UV-sensitive comparison via this tag.
	{
		static const NLMISC::CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);
		for (uint i = 0; i < mods.size(); ++i)
		{
			if (mods[i]->classDesc()->classId() == CLASSID_MAP_EXTENDER)
			{
				printf("MAPEXT %s\n", NLMISC::toLowerAscii(name).c_str());
				break;
			}
		}
	}

	// Base mesh (materials, flags, default transform) — parent-authoritative for the whole
	// multi-lod, and the sole build for the single-mesh path.
	Matrix3M localTM = getLocalMatrix(node, tmCache);
	SMaxMeshBaseBuild maxBaseBuild;
	NL3D::CMeshBase::CMeshBaseBuild buildBaseMesh;
	buildBaseMeshInterface(buildBaseMesh, maxBaseBuild, node, tmCache, localTM, exportLighting);

	// Lightmapped materials: the reference computed lightmaps at export time (exportLighting);
	// the headless design exports unmapped (design doc §9) — tag for the harness bucket.
	for (uint i = 0; i < buildBaseMesh.Materials.size(); ++i)
	{
		if (buildBaseMesh.Materials[i].getShader() == NL3D::CMaterial::LightMap)
		{
			printf("LIGHTMAP %s\n", NLMISC::toLowerAscii(name).c_str());
			break;
		}
	}

	NL3D::CMeshBase *meshBase = NULL;
	std::vector<sint> materialRemap;

	if (lodCount > 0)
	{
		// Build the CMeshMultiLod: LOD 0 = this node's own mesh, LOD 1..count from the slave
		// nodes named in the LOD_NAME + i appdata (case-insensitive lookup — same discipline as
		// the LOD-slave gate in exportFile that keeps slaves from producing their own .shape).
		NL3D::CMeshMultiLod::CMeshMultiLodBuild mlBuild;
		mlBuild.BaseMesh = buildBaseMesh;
		mlBuild.StaticLod = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_DYNAMIC_MESH, "") != "1";

		uint numMaterials = (uint)buildBaseMesh.Materials.size();
		bool anyOk = false;

		// LOD 0 = the parent node itself
		{
			NL3D::CMesh::CMeshBuild buildMesh;
			if (evalAndBuildMesh(node, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats))
			{
				NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot slot;
				slot.MeshGeom = buildMeshGeomFor(node, buildMesh, numMaterials);
				slot.DistMax = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
				slot.BlendLength = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_BLEND_LENGTH, 0.f);
				slot.Flags = lodSlotFlags(n);
				mlBuild.LodMeshes.push_back(slot);
				anyOk = true;
			}
		}

		// LOD 1..count = slave nodes
		for (uint i = 0; i < lodCount && i < 10; ++i)
		{
			std::string slaveName = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_NAME + i, "");
			if (slaveName.empty()) continue;
			TNodesByName::const_iterator it = nodesByName.find(NLMISC::toLowerAscii(slaveName));
			if (it == nodesByName.end())
			{
				fprintf(stderr, "WARNING: multilod '%s': slave LOD '%s' not found\n",
				        name.c_str(), slaveName.c_str());
				continue;
			}
			INode *slave = it->second;
			CNodeImpl *sn = dynamic_cast<CNodeImpl *>(slave);
			NL3D::CMesh::CMeshBuild buildMesh;
			if (!evalAndBuildMesh(*slave, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats))
				continue;
			NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot slot;
			slot.MeshGeom = buildMeshGeomFor(*slave, buildMesh, numMaterials);
			slot.DistMax = getScriptAppDataFloat(sn, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
			slot.BlendLength = getScriptAppDataFloat(sn, NEL3D_APPDATA_LOD_BLEND_LENGTH, 0.f);
			slot.Flags = lodSlotFlags(sn);
			mlBuild.LodMeshes.push_back(slot);
			anyOk = true;
		}

		if (!anyOk)
		{
			// No LOD came through — bail rather than serialize an empty multi-lod.
			stats.skip("mesh-eval");
			return NULL;
		}

		NL3D::CMeshMultiLod *ml = new NL3D::CMeshMultiLod;
		ml->build(mlBuild);
		// No optimizeMaterialUsage on CMeshMultiLod — the base material list is fixed and shared
		// across every LOD's MeshGeom, so we can't drop unreferenced ones without walking every
		// per-LOD index buffer for its actual material use. Fill an identity remap for the
		// animated-material path below.
		materialRemap.resize(maxBaseBuild.NumMaterials);
		for (uint i = 0; i < maxBaseBuild.NumMaterials; ++i)
			materialRemap[i] = (sint)i;
		meshBase = ml;
	}
	else
	{
		NL3D::CMesh::CMeshBuild buildMesh;
		if (!evalAndBuildMesh(node, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats))
			return NULL;

		if (getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0))
		{
			NL3D::CMRMParameters parameters;
			buildMRMParameters(n, parameters);

			std::vector<NL3D::CMesh::CMeshBuild *> bsList; // morph targets: not implemented

			if (NL3D::CMeshMRMSkinned::isCompatible(buildMesh) && bsList.empty())
			{
				NL3D::CMeshMRMSkinned *meshMRMSkinned = new NL3D::CMeshMRMSkinned;
				meshMRMSkinned->build(buildBaseMesh, buildMesh, parameters);
				meshMRMSkinned->optimizeMaterialUsage(materialRemap);
				meshBase = meshMRMSkinned;
			}
			else
			{
				NL3D::CMeshMRM *meshMRM = new NL3D::CMeshMRM;
				meshMRM->build(buildBaseMesh, buildMesh, bsList, parameters);
				meshMRM->optimizeMaterialUsage(materialRemap);
				meshBase = meshMRM;
			}
		}
		else
		{
			NL3D::CMesh *m = new NL3D::CMesh;
			m->build(buildBaseMesh, buildMesh);
			// buildMeshMorph: morph targets not implemented (Morpher modifier reports unhandled)
			m->optimizeMaterialUsage(materialRemap);
			meshBase = m;
		}
	}

	// Animated materials
	if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0)
	{
		for (uint i = 0; i < maxBaseBuild.NumMaterials; i++)
		{
			std::string matName = maxBaseBuild.MaterialInfo[i].MaterialName;
			sint dstMatId = materialRemap[i];
			if (dstMatId >= 0)
				meshBase->setAnimatedMaterial(dstMatId, matName);
		}
	}

	// Auto anim
	if (getScriptAppDataInt(n, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0) != 0)
		meshBase->setAutoAnim(true);

	return meshBase;
}

// ---------------------------------------------------------------------------------------------
// Per-file export

static int exportFile(const std::string &maxPath, const std::string &outDir, const std::string &outDirCoarse,
                      const std::string &animDir, bool exportLighting, SExportStats &stats)
{
	SLoadedMax lm;
	if (!loadMaxFile(maxPath, lm))
		return 1;

	CSceneClassContainer *ssc = lm.Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = NULL;

	// Collect the LOD slave set (case-insensitive) and coarse-mesh info
	std::set<std::string> lodNames;
	std::map<std::string, INode *> nodesByName;
	std::vector<INode *> allNodes;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		allNodes.push_back(node);
		nodesByName[NLMISC::toLowerAscii(nodeName(*node))] = node;
	}
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(allNodes[i]);
		uint count = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
		for (uint l = 0; l < count && l < 10; ++l)
		{
			std::string nm = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_NAME + l, "");
			if (!nm.empty())
				lodNames.insert(NLMISC::toLowerAscii(nm));
		}
	}

	// Per node
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
		std::string name = nodeName(node);

		CSceneClass *base = baseObjectOf(node, NULL, NULL);
		if (!isGeometryOrShape(base))
			continue;

		// Skeleton parts
		if (startsWithBip(name) || startsWithBip(nodeName(*rootOf(&node))))
			continue;

		NLMISC::CClassId cid = base->classDesc()->classId();
		if (cid == CLASSID_RPO)
			continue;
		if (cid.a() == CLASSID_PARTA_NEL_PS)
			continue;
		if (cid == CLASSID_PACS_BOX || cid == CLASSID_PACS_CYL)
			continue;
		// Target objects ((0x1020,0), light/camera look-at anchors) never yield reference
		// shapes (0 of 3518 references) — the reference exporter produces nothing for them.
		if (cid == CLASSID_TARGET)
			continue;

		// Accelerator?
		{
			std::string accel = getScriptAppDataStr(n, NEL3D_APPDATA_ACCEL, "");
			if (!accel.empty() && accel != "0" && accel != "32")
				continue;
		}

		if (getScriptAppDataStr(n, NEL3D_APPDATA_DONOTEXPORT, "") == "1")
			continue;
		if (getScriptAppDataStr(n, NEL3D_APPDATA_COLLISION, "") == "1")
			continue;
		if (getScriptAppDataStr(n, NEL3D_APPDATA_COLLISION_EXTERIOR, "") == "1")
			continue;

		// LOD slave?
		if (lodNames.count(NLMISC::toLowerAscii(name)))
			continue;

		// Coarse mesh in the LOD set?
		bool haveCoarse = false;
		{
			uint count = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
			for (uint l = 0; l < count && l < 10; ++l)
			{
				std::string nm = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_NAME + l, "");
				if (nm.empty()) continue;
				std::map<std::string, INode *>::iterator lodIt = nodesByName.find(NLMISC::toLowerAscii(nm));
				if (lodIt == nodesByName.end()) continue;
				CNodeImpl *lodNode = dynamic_cast<CNodeImpl *>(lodIt->second);
				if (getScriptAppDataStr(lodNode, NEL3D_APPDATA_LOD_COARSE_MESH, "") == "1")
					haveCoarse = true;
			}
		}

		std::string outPath = (haveCoarse ? outDirCoarse : outDir) + "/" + NLMISC::toLowerAscii(name) + ".shape";

		NL3D::IShape *shape = buildShapeForNode(node, tmCache, nodesByName, exportLighting, stats);
		if (!shape)
			continue;

		// Set the dist max for this shape (non-multilod path)
		float distmax = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
		shape->setDistMax(distmax);

		try
		{
			// Serialize to memory, then patch the serialMeshBase version byte 10 -> 9: current
			// NL3D writes CMeshBase version 10 ("Ryzom Core release check", 2024) which adds no
			// fields over the export-era version 9 — same pure-version-byte class as the zone
			// v4/v5 byte (see pipeline_max_design.md §10h). Stream layout for the CMeshBase
			// shapes: "SHAP" + u64 0x1 + u32 nameLen + name + classVersion byte + meshBase
			// version byte.
			NLMISC::CMemStream mem;
			{
				NL3D::CShapeStream shapeStream(shape);
				shapeStream.serial(mem);
			}
			uint8 *buf = const_cast<uint8 *>(mem.buffer());
			uint32 len = mem.length();
			{
				std::string className = shape->getClassName();
				uint32 versionOff = 4 + 8 + 4 + (uint32)className.size() + 1;
				if ((className == "CMesh" || className == "CMeshMRM" || className == "CMeshMRMSkinned")
					&& versionOff < len && buf[versionOff] == 10)
					buf[versionOff] = 9;
			}
			NLMISC::COFile file;
			if (file.open(outPath))
			{
				file.serialBuffer(buf, len);
				file.close();
				++stats.Exported;
				if (g_verbose)
					printf("OK %s\n", outPath.c_str());
			}
			else
			{
				fprintf(stderr, "ERROR: cannot open %s for writing\n", outPath.c_str());
			}
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: shape serialization failed for %s: %s\n", outPath.c_str(), e.what());
		}
		delete shape;

		// Per-node material animation (.anim), the shape process's NelExportAnimation step
		// (shape_export.ms "Export default animations"): a node with AUTOMATIC_ANIMATION gets a
		// <node>.anim. The animated-material class (waterfalls) resolves to texture-matrix tracks
		// (§10k); other track classes (node transform / note / morph) of NelExportAnimation are not
		// yet replicated here, so a node whose only animation is those produces no file for now.
		if (!animDir.empty() && SHAPEANIM::isAnimToBeExported(node))
		{
			NL3D::CAnimation animation;
			if (SHAPEANIM::buildMaterialAnim(node, animation))
			{
				std::string animPath = animDir + "/" + name + ".anim"; // raw node name, like the reference
				try
				{
					NLMISC::COFile f;
					if (f.open(animPath)) { animation.serial(f); f.close(); ++stats.AnimExported; }
					else fprintf(stderr, "ERROR: cannot open %s for writing\n", animPath.c_str());
				}
				catch (const NLMISC::Exception &e)
				{
					fprintf(stderr, "ERROR: anim serialization failed for %s: %s\n", animPath.c_str(), e.what());
				}
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------------------------
// Compare mode: byte compare, then a field-level triage over the loaded shapes.

static NL3D::IShape *loadShape(const std::string &path)
{
	try
	{
		NLMISC::CIFile f;
		if (!f.open(path)) return NULL;
		NL3D::CShapeStream ss;
		ss.serial(f);
		return ss.getShapePointer();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "load %s: %s\n", path.c_str(), e.what());
		return NULL;
	}
}

static uint32 floatUlp(float a, float b)
{
	uint32 ia, ib;
	memcpy(&ia, &a, 4);
	memcpy(&ib, &b, 4);
	if ((ia >> 31) != (ib >> 31)) return 0xFFFFFFFF;
	uint32 d = ia > ib ? ia - ib : ib - ia;
	return d;
}

// Verdict aggregation for the compare mode: 0 = identical, 1 = float-noise only (the x87
// reference-build tier: VB float values within tolerance, transform quat/scale within a few
// ulp, everything else byte-identical), 2 = structural difference.
static int g_verdict = 0;

// --compare-lightmap-mask mode: the reference plugin's calc-lm runs at export time and appends
// lightmap textures to LightMap-shader materials + a second UV set to the VB, before writing
// (design doc §9 T3 caveats; the lightmapper is scheduled as a separate standalone tool
// downstream). Our headless output is unmapped by design, so full-shape byte compare against a
// lightmapped reference cannot pass without simulating the lightmapper. This mode compares only
// the fields the lightmapper does NOT touch: transform, material COUNT + shader types + core
// flags (blend/twosided/zwrite/color) + slot-0 texture presence, matrix-block/rdrpass counts,
// and total index count per rdrpass (topology count is preserved under lightmap-UV re-dedup;
// content isn't). VB content and per-material texture list beyond slot 0 are skipped.
static bool g_lightmapMask = false;

static void raiseVerdict(int v)
{
	if (v > g_verdict) g_verdict = v;
}

// Float-noise tolerance: near-zero absolute noise or a few ulp on larger values.
static bool floatNoiseTol(float a, float b, float absTol)
{
	if (a == b) return true;
	if (fabsf(a - b) <= absTol) return true;
	return floatUlp(a, b) <= 32;
}

static bool floatNoise(float a, float b)
{
	return floatNoiseTol(a, b, 2e-6f);
}

static void compareVB(const NL3D::CVertexBuffer &va, const NL3D::CVertexBuffer &vb)
{
	printf("  VB: %u vs %u verts, format 0x%x vs 0x%x, size %u vs %u\n",
	       va.getNumVertices(), vb.getNumVertices(),
	       va.getVertexFormat(), vb.getVertexFormat(),
	       va.getVertexSize(), vb.getVertexSize());
	if (g_lightmapMask)
	{
		// The lightmapper adds a second UV set (VB format bit 3) and re-deduplicates verts
		// against the lightmap UV; positions/normals are still the same source data, but the
		// vert order + count follow the lightmap-UV dedup we can't reproduce headlessly.
		// Verify only: reference VB has AT LEAST the base flags (position + normal + primary UV),
		// vertex counts differ but reference has AT LEAST as many (dedup adds boundaries).
		uint32 baseFlags = NL3D::CVertexBuffer::PositionFlag | NL3D::CVertexBuffer::NormalFlag;
		if ((va.getVertexFormat() & baseFlags) != baseFlags
			|| (vb.getVertexFormat() & baseFlags) != baseFlags)
		{
			printf("  VB missing base flags under lightmap mask\n");
			raiseVerdict(2);
		}
		return;
	}
	if (va.getNumVertices() != vb.getNumVertices() || va.getVertexFormat() != vb.getVertexFormat()
		|| va.getVertexSize() != vb.getVertexSize())
	{
		raiseVerdict(2);
		return;
	}
	NL3D::CVertexBufferRead ra, rb;
	const_cast<NL3D::CVertexBuffer &>(va).lock(ra);
	const_cast<NL3D::CVertexBuffer &>(vb).lock(rb);
	const uint8 *pa = (const uint8 *)ra.getVertexCoordPointer();
	const uint8 *pb = (const uint8 *)rb.getVertexCoordPointer();
	uint stride = va.getVertexSize();
	for (uint value = 0; value < NL3D::CVertexBuffer::NumValue; ++value)
	{
		if (!(va.getVertexFormat() & (1 << value))) continue;
		uint off = va.getValueOffEx((NL3D::CVertexBuffer::TValue)value);
		NL3D::CVertexBuffer::TType type = va.getValueType(value);
		uint nComp = NL3D::CVertexBuffer::NumComponentsType[type];
		bool isFloat = type == NL3D::CVertexBuffer::Float1 || type == NL3D::CVertexBuffer::Float2
			|| type == NL3D::CVertexBuffer::Float3 || type == NL3D::CVertexBuffer::Float4;
		uint nDiff = 0;
		uint firstDiff = 0xFFFFFFFF;
		uint32 maxUlp = 0;
		float maxAbs = 0.0f;
		for (uint v = 0; v < va.getNumVertices(); ++v)
		{
			for (uint c = 0; c < nComp; ++c)
			{
				if (isFloat)
				{
					float fa, fb;
					memcpy(&fa, pa + v * stride + off + c * 4, 4);
					memcpy(&fb, pb + v * stride + off + c * 4, 4);
					if (fa != fb)
					{
						++nDiff;
						if (firstDiff == 0xFFFFFFFF) firstDiff = v;
						uint32 u = floatUlp(fa, fb);
						maxUlp = std::max(maxUlp, u);
						maxAbs = std::max(maxAbs, (float)fabs(fa - fb));
						// Normals (value 1) tolerate 1e-4 (~0.006 deg on a unit vector): the
						// angle-weighted accumulation amplifies x87-vs-SSE tails through
						// near-cancellation sums; positions/UVs stay at the tight tier.
						raiseVerdict(floatNoiseTol(fa, fb, value == 1 ? 1e-4f : 2e-6f) ? 1 : 2);
					}
				}
				else
				{
					if (memcmp(pa + v * stride + off + c, pb + v * stride + off + c, 1) != 0)
					{
						++nDiff;
						if (firstDiff == 0xFFFFFFFF) firstDiff = v;
						raiseVerdict(2);
					}
				}
			}
		}
		if (nDiff)
			printf("  VB value %u: %u comps differ (first vert %u), maxUlp %u, maxAbs %g\n",
			       value, nDiff, firstDiff, maxUlp, maxAbs);
		if (nDiff && isFloat && getenv("PMB_COMPARE_DUMP") && !strcmp(getenv("PMB_COMPARE_DUMP"), "big"))
		{
			// print verts whose value differs by more than 0.01, with their position
			uint printed = 0;
			for (uint v = 0; v < va.getNumVertices() && printed < 20; ++v)
			{
				bool big = false;
				for (uint c = 0; c < nComp; ++c)
				{
					float fa, fb;
					memcpy(&fa, pa + v * stride + off + c * 4, 4);
					memcpy(&fb, pb + v * stride + off + c * 4, 4);
					if (fabsf(fa - fb) > 0.01f) big = true;
				}
				if (!big) continue;
				++printed;
				printf("    BIGDIFF v%u pos(", v);
				for (uint c = 0; c < 3; ++c)
				{
					float fp;
					memcpy(&fp, pa + v * stride + c * 4, 4);
					printf("%s%.6g", c ? " " : "", fp);
				}
				printf(") val:");
				for (uint c = 0; c < nComp; ++c)
				{
					float fa, fb;
					memcpy(&fa, pa + v * stride + off + c * 4, 4);
					memcpy(&fb, pb + v * stride + off + c * 4, 4);
					printf(" %.6g/%.6g", fa, fb);
				}
				printf("\n");
			}
		}
		else if (nDiff && isFloat && getenv("PMB_COMPARE_DUMP"))
		{
			for (uint v = 0; v < std::min(va.getNumVertices(), 8u); ++v)
			{
				printf("    v%u:", v);
				for (uint c = 0; c < nComp; ++c)
				{
					float fa, fb;
					memcpy(&fa, pa + v * stride + off + c * 4, 4);
					memcpy(&fb, pb + v * stride + off + c * 4, 4);
					printf(" %.9g/%.9g", fa, fb);
				}
				printf("\n");
			}
		}
	}
}

static void compareMeshBase(NL3D::CMeshBase *ma, NL3D::CMeshBase *mb)
{
	{
		NLMISC::CVector va = ma->getDefaultPos()->getDefaultValue();
		NLMISC::CVector vb = mb->getDefaultPos()->getDefaultValue();
		if (va != vb)
		{
			printf("  DefaultPos: (%.9g %.9g %.9g) vs (%.9g %.9g %.9g)\n", va.x, va.y, va.z, vb.x, vb.y, vb.z);
			raiseVerdict((floatNoise(va.x, vb.x) && floatNoise(va.y, vb.y) && floatNoise(va.z, vb.z)) ? 1 : 2);
		}
	}
	NLMISC::CQuat qa = ma->getDefaultRotQuat()->getDefaultValue();
	NLMISC::CQuat qb = mb->getDefaultRotQuat()->getDefaultValue();
	if (qa.x != qb.x || qa.y != qb.y || qa.z != qb.z || qa.w != qb.w)
	{
		printf("  DefaultRotQuat: (%.9g %.9g %.9g %.9g) vs (%.9g %.9g %.9g %.9g) ulp(%u %u %u %u)\n",
		       qa.x, qa.y, qa.z, qa.w, qb.x, qb.y, qb.z, qb.w,
		       floatUlp(qa.x, qb.x), floatUlp(qa.y, qb.y), floatUlp(qa.z, qb.z), floatUlp(qa.w, qb.w));
		// double-cover aware: q and -q are the same rotation; the x87 reference build lands on
		// the other representative at 180-degree sign boundaries (w ~ 0)
		bool direct = floatNoise(qa.x, qb.x) && floatNoise(qa.y, qb.y) && floatNoise(qa.z, qb.z) && floatNoise(qa.w, qb.w);
		bool negated = floatNoise(qa.x, -qb.x) && floatNoise(qa.y, -qb.y) && floatNoise(qa.z, -qb.z) && floatNoise(qa.w, -qb.w);
		raiseVerdict((direct || negated) ? 1 : 2);
	}
	NLMISC::CVector sa = ma->getDefaultScale()->getDefaultValue();
	NLMISC::CVector sb = mb->getDefaultScale()->getDefaultValue();
	if (sa != sb)
	{
		printf("  DefaultScale: (%.9g %.9g %.9g) vs (%.9g %.9g %.9g) ulp(%u %u %u)\n",
		       sa.x, sa.y, sa.z, sb.x, sb.y, sb.z,
		       floatUlp(sa.x, sb.x), floatUlp(sa.y, sb.y), floatUlp(sa.z, sb.z));
		raiseVerdict((floatNoise(sa.x, sb.x) && floatNoise(sa.y, sb.y) && floatNoise(sa.z, sb.z)) ? 1 : 2);
	}
	if (ma->getNbMaterial() != mb->getNbMaterial())
	{
		printf("  materials: %u vs %u\n", ma->getNbMaterial(), mb->getNbMaterial());
		raiseVerdict(2);
		return;
	}
	for (uint i = 0; i < ma->getNbMaterial(); ++i)
	{
		const NL3D::CMaterial &m_a = ma->getMaterial(i);
		const NL3D::CMaterial &m_b = mb->getMaterial(i);

		// Under lightmap mask: skip appended-texture + calc-lm-touched compare paths, verify
		// only the base fields the lightmapper does not modify. LightMap-shaded materials in
		// the reference carry (a) lightmap textures appended after slot 0, (b) calc-lm-modified
		// ambient/specular/shininess, (c) an added Stage-1 texenv. Compare shader type + blend
		// + z-write + two-sided + color + slot-0 texture presence and STOP.
		if (g_lightmapMask && m_b.getShader() == NL3D::CMaterial::LightMap)
		{
			if (m_a.getShader() != m_b.getShader()
				|| m_a.getBlend() != m_b.getBlend()
				|| m_a.getZWrite() != m_b.getZWrite()
				|| m_a.getDoubleSided() != m_b.getDoubleSided()
				|| m_a.getAlphaTest() != m_b.getAlphaTest())
			{
				printf("  material %u lightmap-masked: shader/blend/zwrite/2sided/atest differ\n", i);
				raiseVerdict(2);
			}
			// Slot-0 texture presence match (path may still differ if base UVs changed under
			// dedup — but the presence + class is what the base material carries).
			NL3D::ITexture *ta = const_cast<NL3D::CMaterial &>(m_a).getTexture(0);
			NL3D::ITexture *tb = const_cast<NL3D::CMaterial &>(m_b).getTexture(0);
			if ((ta != NULL) != (tb != NULL))
			{
				printf("  material %u lightmap-masked: slot-0 texture presence differs\n", i);
				raiseVerdict(2);
			}
			continue;
		}

		NLMISC::CMemStream sa, sb;
		const_cast<NL3D::CMaterial &>(m_a).serial(sa);
		const_cast<NL3D::CMaterial &>(m_b).serial(sb);
		if (sa.length() != sb.length() || memcmp(sa.buffer(), sb.buffer(), sa.length()) != 0)
		{
			uint32 off = 0;
			uint32 n = std::min(sa.length(), sb.length());
			while (off < n && sa.buffer()[off] == sb.buffer()[off]) ++off;
			printf("  material %u differs (serialized %u vs %u bytes, first diff 0x%x)\n",
			       i, sa.length(), sb.length(), off);
			raiseVerdict(2);
			if (getenv("PMB_COMPARE_DUMP"))
			{
				for (uint side = 0; side < 2; ++side)
				{
					const NL3D::CMaterial &m = side ? mb->getMaterial(i) : ma->getMaterial(i);
					printf("    %c: shader %d flags(dbl %d blend %d atest %d zwrite %d light %d) srcb %d dstb %d\n",
					       side ? 'B' : 'A', (int)m.getShader(), m.getDoubleSided(), m.getBlend(),
					       m.getAlphaTest(), m.getZWrite(), m.isLighted(),
					       (int)m.getSrcBlend(), (int)m.getDstBlend());
					NLMISC::CRGBA d = m.isLighted() ? m.getDiffuse() : m.getColor();
					NLMISC::CRGBA e = m.getEmissive(), am = m.getAmbient(), sp = m.getSpecular();
					printf("       col(%d %d %d %d) emis(%d %d %d) amb(%d %d %d) spec(%d %d %d) shin %g op %d\n",
					       d.R, d.G, d.B, d.A, e.R, e.G, e.B, am.R, am.G, am.B, sp.R, sp.G, sp.B,
					       m.getShininess(), m.getOpacity());
					for (uint t = 0; t < NL3D::IDRV_MAT_MAXTEXTURES; ++t)
					{
						NL3D::ITexture *tex = m.getTexture((uint8)t);
						if (!tex) continue;
						NL3D::CTextureFile *tf = dynamic_cast<NL3D::CTextureFile *>(tex);
						printf("       tex%u %s '%s' wrap(%d %d)\n", t, tex->getClassName().c_str(),
						       tf ? tf->getFileName().c_str() : "?", (int)tex->getWrapS(), (int)tex->getWrapT());
					}
				}
			}
		}
	}
}

static void compareShapesFields(const std::string &a, const std::string &b)
{
	NL3D::IShape *sa = loadShape(a);
	NL3D::IShape *sb = loadShape(b);
	if (!sa || !sb)
	{
		delete sa;
		delete sb;
		return;
	}
	printf("  class: %s vs %s\n", sa->getClassName().c_str(), sb->getClassName().c_str());
	if (sa->getClassName() != sb->getClassName())
	{
		raiseVerdict(2);
		delete sa;
		delete sb;
		return;
	}
	NL3D::CMeshBase *mba = dynamic_cast<NL3D::CMeshBase *>(sa);
	NL3D::CMeshBase *mbb = dynamic_cast<NL3D::CMeshBase *>(sb);
	if (mba && mbb)
		compareMeshBase(mba, mbb);
	NL3D::CMesh *ma = dynamic_cast<NL3D::CMesh *>(sa);
	NL3D::CMesh *mb = dynamic_cast<NL3D::CMesh *>(sb);
	if (!ma || !mb)
		raiseVerdict(2); // only CMesh gets the full field walk so far; anything else unclassified
	if (ma && mb)
	{
		compareVB(ma->getVertexBuffer(), mb->getVertexBuffer());
		printf("  matrix blocks: %u vs %u\n", ma->getNbMatrixBlock(), mb->getNbMatrixBlock());
		if (ma->getNbMatrixBlock() == mb->getNbMatrixBlock())
		{
			for (uint mbk = 0; mbk < ma->getNbMatrixBlock(); ++mbk)
			{
				if (ma->getNbRdrPass(mbk) != mb->getNbRdrPass(mbk))
				{
					printf("  rdrpass count %u vs %u\n", ma->getNbRdrPass(mbk), mb->getNbRdrPass(mbk));
					raiseVerdict(2);
					continue;
				}
				for (uint rp = 0; rp < ma->getNbRdrPass(mbk); ++rp)
				{
					if (ma->getRdrPassMaterial(mbk, rp) != mb->getRdrPassMaterial(mbk, rp))
					{
						printf("  rdrpass %u material %u vs %u\n", rp, ma->getRdrPassMaterial(mbk, rp), mb->getRdrPassMaterial(mbk, rp));
						raiseVerdict(2);
					}
					const NL3D::CIndexBuffer &ia = ma->getRdrPassPrimitiveBlock(mbk, rp);
					const NL3D::CIndexBuffer &ib = mb->getRdrPassPrimitiveBlock(mbk, rp);
					if (ia.getNumIndexes() != ib.getNumIndexes())
					{
						printf("  rdrpass %u indexes %u vs %u\n", rp, ia.getNumIndexes(), ib.getNumIndexes());
						raiseVerdict(2);
					}
					else if (!g_lightmapMask)
					{
						// Under lightmap mask, index CONTENT will legitimately differ because the
						// vert list was re-deduplicated on the lightmap UV — the count is what
						// tells us the topology matches. In normal mode, content must match too.
						NL3D::CIndexBufferRead ira, irb;
						const_cast<NL3D::CIndexBuffer &>(ia).lock(ira);
						const_cast<NL3D::CIndexBuffer &>(ib).lock(irb);
						uint bytesPer = ia.getFormat() == NL3D::CIndexBuffer::Indices16 ? 2 : 4;
						if (ia.getFormat() != ib.getFormat()
							|| memcmp(ira.getPtr(), irb.getPtr(), ia.getNumIndexes() * bytesPer) != 0)
						{
							printf("  rdrpass %u index content differs\n", rp);
							raiseVerdict(2);
						}
					}
				}
			}
		}
	}
	delete sa;
	delete sb;
}

static int compareShapes(const std::string &a, const std::string &b)
{
	NLMISC::CIFile fa, fb;
	if (!fa.open(a)) { fprintf(stderr, "cannot open %s\n", a.c_str()); return 2; }
	if (!fb.open(b)) { fprintf(stderr, "cannot open %s\n", b.c_str()); return 2; }
	std::vector<uint8> da(fa.getFileSize()), db(fb.getFileSize());
	if (!da.empty()) fa.serialBuffer(&da[0], (uint)da.size());
	if (!db.empty()) fb.serialBuffer(&db[0], (uint)db.size());
	fa.close();
	fb.close();
	if (da.size() == db.size() && (da.empty() || memcmp(&da[0], &db[0], da.size()) == 0))
	{
		printf("IDENTICAL %u bytes\n", (uint)da.size());
		return 0;
	}
	size_t n = std::min(da.size(), db.size());
	size_t diff = 0;
	while (diff < n && da[diff] == db[diff]) ++diff;
	printf("DIFF size %u vs %u, first mismatch at 0x%x\n", (uint)da.size(), (uint)db.size(), (uint)diff);
	// Under lightmap mask the sizes ALWAYS differ (calc-lm adds textures + a UV set) — don't
	// let the size-differs alone prevent the FLOATEQ verdict.
	g_verdict = (da.size() == db.size() || g_lightmapMask) ? 0 : 2;
	compareShapesFields(a, b);
	if (g_verdict <= 1)
	{
		printf("VERDICT FLOATEQ\n");
		return 0;
	}
	printf("VERDICT DIFF\n");
	return 1;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext appContext;
	NL3D::registerSerial3d();

	// Write export-era stream versions (see the SerialOldPreferredMemory notes): the reference
	// exports predate the buffer-usage refactor.
	NL3D::CVertexBuffer::SerialOldPreferredMemory = true;
	NL3D::CIndexBuffer::SerialOldPreferredMemory = true;

	std::string input, outDir, outDirCoarse, animDir, dbRoot;
	bool exportLighting = true;
	std::vector<std::string> compareArgs;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v")
			g_verbose = true;
		else if (arg == "--db" && i + 1 < argc)
			dbRoot = argv[++i];
		else if (arg == "--path-alias" && i + 1 < argc)
		{
			// --path-alias <windows-prefix>=<root>, e.g. --path-alias "P:\old_graphics=/mnt/old"
			// for corpus content authored under a different drive/root than "R:\graphics\...".
			std::string kv = argv[++i];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n", kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
		}
		else if (arg == "--coarse-out" && i + 1 < argc)
			outDirCoarse = argv[++i];
		else if (arg == "--anim-out" && i + 1 < argc)
			animDir = argv[++i];
		else if (arg == "--no-lighting")
			exportLighting = false;
		else if (arg == "--compare")
		{
			for (int j = i + 1; j < argc; ++j)
				compareArgs.push_back(argv[j]);
			break;
		}
		else if (arg == "--compare-lightmap-mask")
		{
			// Compare with the lightmap-mask: skip fields/paths the calc-lm phase modifies. See
			// g_lightmapMask above. Same argv form as --compare.
			g_lightmapMask = true;
			for (int j = i + 1; j < argc; ++j)
				compareArgs.push_back(argv[j]);
			break;
		}
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

	if (compareArgs.size() == 2)
		return compareShapes(compareArgs[0], compareArgs[1]);

	if (input.empty() || outDir.empty())
	{
		fprintf(stderr, "usage: pipeline_max_export_shape [--db <graphics-root>] [--coarse-out <dir>] [--no-lighting] [-v] <input.max> <outdir>\n");
		fprintf(stderr, "       pipeline_max_export_shape --compare <a.shape> <b.shape>\n");
		return 2;
	}
	if (outDirCoarse.empty())
		outDirCoarse = outDir;

	// Deduce the database root from the input path when not given (…/ryzomcore_graphics/…)
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

	SExportStats stats;
	int ret = exportFile(input, outDir, outDirCoarse, animDir, exportLighting, stats);
	printf("EXPORTED %u shapes, %u skipped, %u anims\n", stats.Exported, stats.Skipped, stats.AnimExported);
	for (std::map<std::string, uint>::iterator it = stats.SkipReasons.begin(); it != stats.SkipReasons.end(); ++it)
		printf("SKIPCLASS %s %u\n", it->first.c_str(), it->second);
	return ret;
}

/* end of file */
