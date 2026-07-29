/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * \author Grok 4.5
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
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/texture_blend.h>
#include <nel/3d/water_shape.h>
#include <nel/3d/seg_remanence_shape.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define PMB_GETPID _getpid
#else
#include <unistd.h>
#define PMB_GETPID getpid
#endif

#include "scene_lib.h"
#include "../pipeline_max_export_common/db_path.h"
#include "mesh_eval.h"
#include "material_build.h"
#include "mesh_build.h"
#include "anim_build.h"
#include "water_build.h"
#include "flare_build.h"
#include "remanence_build.h"
#include "../pipeline_max_export_common/physique_skin.h"
#include "interface_build.h"
#include "lm_scene_build.h"

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
// PMB_SKIN_DUMP diagnostic — recursive dump of a Physique/Skin mod-app payload.
// Emits (chunk id, container flag, raw byte size, depth) so a future decode session has the
// structure map. Sizes come from CStorageRaw::Value size for leaves; containers recurse.
static void dumpModAppTree(const CStorageContainer *c, uint depth, uint maxDepth)
{
	if (!c || depth > maxDepth) return;
	const CStorageContainer::TStorageObjectContainer &children = c->chunks();
	std::string indent(depth * 2, ' ');
	for (CStorageContainer::TStorageObjectConstIt it = children.begin(); it != children.end(); ++it)
	{
		CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (sub && !raw)
		{
			// Container
			uint childCount = (uint)sub->chunks().size();
			fprintf(stderr, "%s0x%04x container children=%u\n", indent.c_str(), it->first, childCount);
			if (depth + 1 <= maxDepth)
				dumpModAppTree(sub, depth + 1, maxDepth);
		}
		else if (raw)
		{
			size_t sz = raw->Value.size();
			fprintf(stderr, "%s0x%04x leaf size=%u", indent.c_str(), it->first, (uint)sz);
			if (sz >= 4 && sz <= 128)
			{
				fprintf(stderr, "  bytes[");
				for (size_t k = 0; k < sz && k < 128; ++k)
				{
					if (k && k % 4 == 0) fprintf(stderr, " ");
					fprintf(stderr, "%02x", raw->Value[k]);
				}
				fprintf(stderr, "]");
			}
			fprintf(stderr, "\n");
		}
		else
		{
			fprintf(stderr, "%s0x%04x (typed non-raw)\n", indent.c_str(), it->first);
		}
	}
}

static void dumpPhysiquePayload(const CStorageContainer *app)
{
	dumpModAppTree(app, 1, 5);
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
                             NL3D::CMesh::CMeshBuild &buildMesh, SExportStats &stats,
                             bool skinned = false)
{
	std::string name = nodeName(node);
	SEvalMesh mesh;
	std::vector<std::string> warnings;
	if (!evalNodeMesh(node, mesh, &warnings))
	{
		stats.skip("mesh-eval");
		return false;
	}
	buildMeshInterface(mesh, buildMesh, buildBaseMesh, maxBaseBuild, node, tmCache, skinned);
	return true;
}

// Apply Physique skinning onto a just-built CMeshBuild (fills SkinWeights + BonesNames + the
// PaletteSkin vertex flag). Returns false only on hard failure (caller may still ship the
// unskinned mesh or skip — currently we skip to keep the harness honest).
static bool tryApplyPhysique(INode &node, NL3D::CMesh::CMeshBuild &buildMesh,
                             const std::vector<CSceneClass *> &mods,
                             const std::vector<CStorageContainer *> &modApps,
                             CSceneClassContainer *ssc, SExportStats &stats)
{
	std::string err;
	if (!PHYSIQUESKIN::applyPhysiqueSkinning(buildMesh, node, mods, modApps, ssc, &err))
	{
		stats.skip("skinned-physique");
		fprintf(stderr, "SKIP shape '%s': Physique skinning failed: %s\n",
		        nodeName(node).c_str(), err.c_str());
		return false;
	}
	// Reference sets PaletteSkinFlag after a successful buildSkinning (export_mesh.cpp).
	buildMesh.VertexFlags |= NL3D::CVertexBuffer::PaletteSkinFlag;
	return true;
}

// Interface-weld world matrix per the reference call sites (export_mesh.cpp:1111): Identity
// for skinned meshes (their VBs are already world space), else worldObjectTM * FromExportSpace
// (maps the build's local/offset-space verts back to world).
static NLMISC::CMatrix interfaceToWorldMat(INode &node, SNodeTMCache &tmCache, bool skinned)
{
	if (skinned)
		return NLMISC::CMatrix::Identity;
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	Matrix3M nodeTM = getNodeTM(&node, tmCache);
	Point3M opos;
	QuatM orot;
	ScaleValueM oscale;
	readObjectOffset(n, opos, orot, oscale);
	Matrix3M objectTM = composePRS(opos, orot, oscale) * nodeTM;
	Matrix3M objectToLocal = objectTM * inverseM3(nodeTM);
	NLMISC::CMatrix toWorld, fromExportSpace;
	MAXSCENE::convertMatrix(toWorld, objectTM);
	MAXSCENE::convertMatrix(fromExportSpace, objectToLocal);
	fromExportSpace.invert();
	return toWorld * fromExportSpace;
}

// Apply the interface weld when the node carries the appdata (post skinning, like the
// reference; morph targets would skip — none are built yet).
static void tryApplyInterface(INode &node, NL3D::CMesh::CMeshBuild &buildMesh,
                              SNodeTMCache &tmCache, bool skinned)
{
	if (!IFACEBUILD::useInterfaceMesh(node))
		return;
	IFACEBUILD::applyInterfaceToMeshBuild(node, buildMesh,
	                                      interfaceToWorldMat(node, tmCache, skinned), tmCache);
}

// Morpher blend-shape targets — the reference's getBSMeshBuild (export_mesh.cpp:1192): for each
// non-NULL Morpher ref 101+i, evaluate the TARGET node through the non-skinned mesh path with
// finalSpace = the SOURCE node's NodeTM when the source is skinned (else Identity) — landing the
// target's verts in the same space as the source's build — then copy the source's (welded)
// corner normals onto every corner whose vertex is an interface vertex (the reference computes
// them off a fresh base MeshBuild in the objectToLocal·finalSpace frame, which equals the
// skinned world frame; our export buildMesh IS that frame with the weld applied, so it serves
// as the base directly). Targets that fail to evaluate or whose vertex count diverges from the
// base are skipped with a warning — NL3D's CMRMBuilder::buildBlendShapes dereferences and
// asserts equal counts (the reference could rely on live Max never failing there).
static void buildBSList(INode &node, SNodeTMCache &tmCache,
                        const std::vector<CSceneClass *> &mods,
                        const NL3D::CMesh::CMeshBuild &exportMesh, bool skinned,
                        bool exportLighting,
                        std::vector<NL3D::CMesh::CMeshBuild *> &bsList)
{
	static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);
	CReferenceMaker *morph = nullptr;
	for (uint i = 0; i < mods.size() && !morph; ++i)
		if (mods[i]->classDesc()->classId() == CLASSID_MORPHER)
			morph = dynamic_cast<CReferenceMaker *>(mods[i]);
	if (!morph)
		return;

	NLMISC::CMatrix finalSpace = NLMISC::CMatrix::Identity;
	if (skinned)
		MAXSCENE::convertMatrix(finalSpace, getNodeTM(&node, tmCache));

	for (uint i = 0; i < 100; ++i)
	{
		if (101 + i >= morph->nbReferences())
			break;
		INode *target = dynamic_cast<INode *>(morph->getReference(101 + i));
		if (!target)
			continue;
		SEvalMesh tmesh;
		if (!MESHEVAL::evalNodeMesh(*target, tmesh, nullptr))
		{
			fprintf(stderr, "WARNING: morph target '%s' of '%s' failed mesh eval; channel dropped\n",
			        nodeName(*target).c_str(), nodeName(node).c_str());
			continue;
		}
		SMaxMeshBaseBuild tMax;
		NL3D::CMeshBase::CMeshBaseBuild tBase;
		buildBaseMeshInterface(tBase, tMax, *target, tmCache, getLocalMatrix(*target, tmCache),
		                       exportLighting);
		NL3D::CMesh::CMeshBuild *mb = new NL3D::CMesh::CMeshBuild;
		buildMeshInterface(tmesh, *mb, tBase, tMax, *target, tmCache, false, &finalSpace);
		if (mb->Vertices.size() != exportMesh.Vertices.size())
		{
			fprintf(stderr, "WARNING: morph target '%s' of '%s' has %u verts vs base %u; channel dropped\n",
			        nodeName(*target).c_str(), nodeName(node).c_str(),
			        (uint)mb->Vertices.size(), (uint)exportMesh.Vertices.size());
			delete mb;
			continue;
		}
		// Interface-vert corner normals come from the (welded) base.
		if (exportMesh.InterfaceVertexFlag.size() != 0)
		{
			for (uint k = 0; k < mb->Faces.size() && k < exportMesh.Faces.size(); ++k)
				for (uint l = 0; l < 3; ++l)
				{
					uint vert = mb->Faces[k].Corner[l].Vertex;
					if (vert < exportMesh.InterfaceVertexFlag.size() && exportMesh.InterfaceVertexFlag.get(vert))
						mb->Faces[k].Corner[l].Normal = exportMesh.Faces[k].Corner[l].Normal;
				}
		}
		bsList.push_back(mb);
	}
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
                                       bool exportLighting, SExportStats &stats,
                                       CSceneClassContainer *ssc,
                                       LMSCENE::SCollector *lmc)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string name = nodeName(node);

	std::vector<CSceneClass *> mods;
	std::vector<CStorageContainer *> modApps;
	CSceneClass *base = baseObjectOf(node, &mods, &modApps);
	if (!base) return nullptr;
	NLMISC::CClassId cid = base->classDesc()->classId();

	// FX/special shape classes (not yet implemented; each reports for the harness)
	if (cid.a() == CLASSID_PARTA_NEL_WAVE_MAKER)
	{
		stats.skip("wavemaker");
		fprintf(stderr, "SKIP shape '%s': wave maker not implemented\n", name.c_str());
		return nullptr;
	}
	if (getScriptAppDataInt(n, NEL3D_APPDATA_USE_REMANENCE, 0))
	{
		NL3D::IShape *rs = REMANENCEBUILD::buildRemanenceShape(node, tmCache, exportLighting);
		if (!rs)
		{
			stats.skip("remanence");
			return nullptr;
		}
		return rs;
	}
	if (cid.a() == CLASSID_PARTA_NEL_FLARE)
	{
		NL3D::IShape *fs = FLAREBUILD::buildFlareShape(node, tmCache);
		if (!fs)
		{
			stats.skip("flare");
			return nullptr;
		}
		return fs;
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
			return nullptr;
		}
		return ws;
	}

	// Skinning detection (Physique/Skin). Physique has a production decode path (§10z-six /
	// PHYSIQUESKIN); Skin (Max 4+) still skips — zero corpus hits under Max 3-era assets.
	// Detection is on (ClassId, SuperClassId) — Max shares ClassId (0x100, 0) across four
	// classes (Physique is SuperClassId 0x810).
	bool hasPhysique = false;
	for (uint i = 0; i < mods.size(); ++i)
	{
		NLMISC::CClassId mcid = mods[i]->classDesc()->classId();
		TSClassId mscid = mods[i]->classDesc()->superClassId();
		bool isPhysique = (mcid == SCENELIB::CLASSID_PHYSIQUE && mscid == SCENELIB::SCLASS_OSMODIFIER)
		                  || PHYSIQUESKIN::isPhysiqueModifier(mods[i]);
		bool isSkin = (mcid == SCENELIB::CLASSID_SKIN) || PHYSIQUESKIN::isSkinModifier(mods[i]);
		if (isPhysique || isSkin)
		{
			if (getenv("PMB_SKIN_DUMP"))
			{
				CStorageContainer *app = (i < modApps.size()) ? modApps[i] : nullptr;
				fprintf(stderr, "PMB_SKIN_DUMP node='%s' modifier=%s cid=(0x%x,0x%x) sup=0x%x\n",
				        name.c_str(), isPhysique ? "Physique" : "Skin",
				        mcid.a(), mcid.b(), (uint32)mscid);
				if (app)
					dumpPhysiquePayload(app);
				else
					fprintf(stderr, "  (no mod-app slot)\n");
				CReferenceMaker *mrm = dynamic_cast<CReferenceMaker *>(mods[i]);
				if (mrm)
				{
					fprintf(stderr, "  MODREFS count=%u\n", (uint)mrm->nbReferences());
					for (uint r = 0; r < mrm->nbReferences(); ++r)
					{
						CSceneClass *ref = dynamic_cast<CSceneClass *>(mrm->getReference(r));
						if (!ref) { fprintf(stderr, "    ref[%u] = NULL\n", r); continue; }
						INode *rn = dynamic_cast<INode *>(ref);
						fprintf(stderr, "    ref[%u] cid=(0x%x,0x%x) sup=0x%x %s%s\n", r,
						        ref->classDesc()->classId().a(), ref->classDesc()->classId().b(),
						        (uint32)ref->classDesc()->superClassId(),
						        rn ? "NODE '" : "", rn ? nodeName(*rn).c_str() : "");
					}
				}
			}
			if (isSkin)
			{
				stats.skip("skinned-skin");
				fprintf(stderr, "SKIP shape '%s': Skin modifier not implemented (Max 4+ ISkin path;"
				                " corpus era is Physique-only — see design §10z-quat)\n",
				        name.c_str());
				return nullptr;
			}
			hasPhysique = true;
			// Physique: continue into the mesh path; skinning is applied after mesh eval.
		}
	}

	// Multi-lod object detection (LOD_NAME_COUNT > 0 = the node has LOD slaves — LOD 0 is the
	// node itself + up to 9 slaves resolved by NAME from the LOD_NAME + i appdata; each slave
	// contributes ONE MeshGeom slot to a CMeshMultiLod. Materials, transform and shadow flags
	// live at the base level and come from the parent node; slave matIDs get clamped through
	// buildMeshInterface's `% numMaterials` so a slave with fewer materials is safe).
	// Note: multi-lod + Physique is rare in the corpus; we still apply skinning per slot when
	// the parent/slave carries Physique (hasPhysique covers the parent; slaves re-detect).
	uint lodCount = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);

	// Interface meshes (border normal welding against another .max via INTERFACE_FILE appdata)
	// are applied post-build per mesh below (IFACEBUILD::applyInterfaceToMeshBuild — the
	// headless replication of the reference's export_mesh_interface.cpp).

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
	bool hasLightMapMaterial = false;
	for (uint i = 0; i < buildBaseMesh.Materials.size(); ++i)
	{
		if (buildBaseMesh.Materials[i].getShader() == NL3D::CMaterial::LightMap)
		{
			hasLightMapMaterial = true;
			printf("LIGHTMAP %s\n", NLMISC::toLowerAscii(name).c_str());
			break;
		}
	}

	// Lightmap scene-graph collection (--lm-scene, design doc §11-lm): record this node as a
	// receiver — pre-build base + per-geom mesh builds (copied BEFORE the shape build mutates
	// them) + the shape-build recipe. The standalone lightmapper re-runs calc_lm on these and
	// finishes the build; the exporter's own output stays unmapped as before.
	bool lmCollect = lmc && hasLightMapMaterial;
	if (lmCollect && hasPhysique)
	{
		// Skinned receivers would break the object-space convention (skinned VBs export in
		// world space, §10z-treize) and the reference never lightmaps skinned content.
		fprintf(stderr, "WARNING: '%s' is skinned AND lightmapped — not collected as a"
		                " lightmap receiver\n", name.c_str());
		lmCollect = false;
	}
	NL3D::CLightmapReceiver lmRecv;
	if (lmCollect)
	{
		lmRecv.NodeName = name;
		lmRecv.BaseBuild = buildBaseMesh;
		lmRecv.MultiLod = lodCount > 0;
		lmRecv.StaticLod = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_DYNAMIC_MESH, "") != "1";
		lmRecv.WantMrm = getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0) != 0;
		if (lmRecv.WantMrm)
		{
			NL3D::CMRMParameters parameters;
			buildMRMParameters(n, parameters);
			lmRecv.MrmNLods = parameters.NLods;
			lmRecv.MrmDivisor = parameters.Divisor;
			lmRecv.MrmSkinReduction = (uint32)parameters.SkinReduction;
			lmRecv.MrmDistanceFinest = parameters.DistanceFinest;
			lmRecv.MrmDistanceMiddle = parameters.DistanceMiddle;
			lmRecv.MrmDistanceCoarsest = parameters.DistanceCoarsest;
		}
		for (uint i = 0; i < maxBaseBuild.MaterialInfo.size(); ++i)
			lmRecv.MaterialNames.push_back(maxBaseBuild.MaterialInfo[i].MaterialName);
		lmRecv.AnimatedMaterials = getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0;
		lmRecv.AutoAnim = getScriptAppDataInt(n, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0) != 0;
		lmRecv.DistMax = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
		lmRecv.CoarseOutput = lmc->CurrentCoarse;
	}

	NL3D::CMeshBase *meshBase = nullptr;
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
			if (evalAndBuildMesh(node, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats,
			                     hasPhysique))
			{
				if (hasPhysique && !tryApplyPhysique(node, buildMesh, mods, modApps, ssc, stats))
					return nullptr;
				tryApplyInterface(node, buildMesh, tmCache, hasPhysique);
				NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot slot;
				slot.DistMax = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
				slot.BlendLength = getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_BLEND_LENGTH, 0.f);
				slot.Flags = lodSlotFlags(n);
				if (lmCollect)
				{
					NL3D::CLightmapReceiverGeom lmGeom;
					lmGeom.NodeName = name;
					lmGeom.MeshBuild = buildMesh; // pre-build copy
					lmGeom.DistMax = slot.DistMax;
					lmGeom.BlendLength = slot.BlendLength;
					lmGeom.SlotFlags = slot.Flags;
					lmGeom.FirstMaterial = maxBaseBuild.FirstMaterial;
					lmGeom.GeomMrm = getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0) != 0;
					if (lmGeom.GeomMrm)
					{
						NL3D::CMRMParameters gp;
						buildMRMParameters(n, gp);
						lmGeom.MrmNLods = gp.NLods; lmGeom.MrmDivisor = gp.Divisor;
						lmGeom.MrmSkinReduction = (uint32)gp.SkinReduction;
						lmGeom.MrmDistanceFinest = gp.DistanceFinest;
						lmGeom.MrmDistanceMiddle = gp.DistanceMiddle;
						lmGeom.MrmDistanceCoarsest = gp.DistanceCoarsest;
					}
					lmGeom.GeomMrm = lmRecv.WantMrm;
			LMSCENE::fillGeomAppData(lmGeom, n, *lmc, name);
					lmRecv.Geoms.push_back(lmGeom);
				}
				slot.MeshGeom = buildMeshGeomFor(node, buildMesh, numMaterials);
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
			// Slave may carry its own Physique (rare); detect BEFORE the mesh build so the
			// vertex export space (world for skinned) is decided up front like the parent's.
			std::vector<CSceneClass *> sMods;
			std::vector<CStorageContainer *> sApps;
			baseObjectOf(*slave, &sMods, &sApps);
			bool slavePhys = false;
			for (uint mi = 0; mi < sMods.size(); ++mi)
				if (PHYSIQUESKIN::isPhysiqueModifier(sMods[mi])) { slavePhys = true; break; }
			NL3D::CMesh::CMeshBuild buildMesh;
			if (!evalAndBuildMesh(*slave, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats,
			                      slavePhys))
				continue;
			if (slavePhys && !tryApplyPhysique(*slave, buildMesh, sMods, sApps, ssc, stats))
				continue;
			tryApplyInterface(*slave, buildMesh, tmCache, slavePhys);
			NL3D::CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot slot;
			slot.DistMax = getScriptAppDataFloat(sn, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f);
			slot.BlendLength = getScriptAppDataFloat(sn, NEL3D_APPDATA_LOD_BLEND_LENGTH, 0.f);
			slot.Flags = lodSlotFlags(sn);
			if (lmCollect && !slavePhys)
			{
				NL3D::CLightmapReceiverGeom lmGeom;
				lmGeom.NodeName = nodeName(*slave);
				lmGeom.MeshBuild = buildMesh; // pre-build copy
				lmGeom.DistMax = slot.DistMax;
				lmGeom.BlendLength = slot.BlendLength;
				lmGeom.SlotFlags = slot.Flags;
				lmGeom.FirstMaterial = maxBaseBuild.FirstMaterial;
				lmGeom.GeomMrm = getScriptAppDataInt(sn, NEL3D_APPDATA_LOD_MRM, 0) != 0;
				if (lmGeom.GeomMrm)
				{
					NL3D::CMRMParameters gp;
					buildMRMParameters(sn, gp);
					lmGeom.MrmNLods = gp.NLods; lmGeom.MrmDivisor = gp.Divisor;
					lmGeom.MrmSkinReduction = (uint32)gp.SkinReduction;
					lmGeom.MrmDistanceFinest = gp.DistanceFinest;
					lmGeom.MrmDistanceMiddle = gp.DistanceMiddle;
					lmGeom.MrmDistanceCoarsest = gp.DistanceCoarsest;
				}
				LMSCENE::fillGeomAppData(lmGeom, sn, *lmc, lmGeom.NodeName);
				lmRecv.Geoms.push_back(lmGeom);
			}
			slot.MeshGeom = buildMeshGeomFor(*slave, buildMesh, numMaterials);
			mlBuild.LodMeshes.push_back(slot);
			anyOk = true;
		}

		if (!anyOk)
		{
			// No LOD came through — bail rather than serialize an empty multi-lod.
			stats.skip("mesh-eval");
			return nullptr;
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
		if (!evalAndBuildMesh(node, tmCache, buildBaseMesh, maxBaseBuild, buildMesh, stats,
		                      hasPhysique))
			return nullptr;
		if (hasPhysique && !tryApplyPhysique(node, buildMesh, mods, modApps, ssc, stats))
			return nullptr;
		tryApplyInterface(node, buildMesh, tmCache, hasPhysique);

		if (lmCollect)
		{
			NL3D::CLightmapReceiverGeom lmGeom;
			lmGeom.NodeName = name;
			lmGeom.MeshBuild = buildMesh; // pre-build copy (CMesh::build mutates VertLink etc.)
			lmGeom.FirstMaterial = maxBaseBuild.FirstMaterial;
			LMSCENE::fillGeomAppData(lmGeom, n, *lmc, name);
			lmRecv.Geoms.push_back(lmGeom);
		}

		// LOD_MRM alone selects the MRM branch, matching the reference plugin
		// (export_mesh.cpp:360): a skinned node with LOD_MRM=0 exports as a plain CMesh carrying
		// SkinWeights (that's how ca_spaceship2/ship_tank_karavan ship in the reference). Earlier
		// this branch OR'd `!SkinWeights.empty()` in to force MRM for any skinned mesh, which
		// mislabelled those two files as CMeshMRMSkinned and blew NL3D_MESH_SKIN_MANAGER_MAXVERTICES
		// on their post-MRM VB (both are big ship meshes without LOD_MRM authored).
		const bool wantMrm = getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0) != 0;

		if (wantMrm)
		{
			NL3D::CMRMParameters parameters;
			buildMRMParameters(n, parameters);

			// Morpher blend-shape targets — a non-empty bsList forces the CMeshMRM branch (the
			// reference's isCompatible gate below), which is how the visage files ship as
			// CMeshMRM-with-SkinWeights instead of CMeshMRMSkinned.
			std::vector<NL3D::CMesh::CMeshBuild *> bsList;
			buildBSList(node, tmCache, mods, buildMesh, hasPhysique, exportLighting, bsList);

			if (NL3D::CMeshMRMSkinned::isCompatible(buildMesh) && bsList.empty())
			{
				NL3D::CMeshMRMSkinned *meshMRMSkinned = new NL3D::CMeshMRMSkinned;
				meshMRMSkinned->build(buildBaseMesh, buildMesh, parameters);
				// CMeshMRMSkinned::isCompatible gates the INPUT vertex count, but MRM
				// construction can grow the vertex count at smoothing-group/material/bone
				// boundaries past NL3D_MESH_SKIN_MANAGER_MAXVERTICES=5000 (the skin-manager's
				// fixed shared VB size). CMeshMRMSkinnedGeom::compileRunTime logs the failure
				// and clears _RuntimeCompiled when that happens; skip the node with an artist-
				// facing message so the authoring gets fixed (LOD_MRM=0 or split the mesh).
				if (!meshMRMSkinned->isRuntimeCompiled())
				{
					fprintf(stderr, "SKIP shape '%s': CMeshMRMSkinned post-MRM vertex count "
					                "exceeds NL3D_MESH_SKIN_MANAGER_MAXVERTICES. Author "
					                "LOD_MRM=0 to export as plain CMesh with SkinWeights, or "
					                "split the geometry so each part's post-MRM VB fits in "
					                "the skin-manager buffer.\n",
					        name.c_str());
					delete meshMRMSkinned;
					stats.skip("skinned-maxverts");
					return nullptr;
				}
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
			for (uint i = 0; i < bsList.size(); ++i)
				delete bsList[i];
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

	// Dist max — only for the mesh/multilod path, matching the reference exporter's
	// buildShape which gates its setDistMax call on the mesh path (`!multiLodObject && buildLods`
	// block at export_mesh.cpp:466) and returns early before that block for flare/water/
	// remanence/wavemaker/particle-system builds. See the corresponding comment in exportFile.
	meshBase->setDistMax(getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f));

	if (lmCollect && !lmRecv.Geoms.empty())
		lmc->Scene.Receivers.push_back(lmRecv);

	return meshBase;
}

// ---------------------------------------------------------------------------------------------
// Per-file export

static int exportFile(const std::string &maxPath, const std::string &outDir, const std::string &outDirCoarse,
                      const std::string &animDir, const std::string &lmSceneDir,
                      bool exportLighting, SExportStats &stats)
{
	SLoadedMax lm;
	if (!loadMaxFile(maxPath, lm))
		return 1;

	CSceneClassContainer *ssc = lm.Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = nullptr;

	LMSCENE::SCollector lmCollector;
	LMSCENE::SCollector *lmc = lmSceneDir.empty() ? nullptr : &lmCollector;

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
			if (nm.empty())
				continue;
			lodNames.insert(NLMISC::toLowerAscii(nm));
			if (lmc)
			{
				// LOD relations for the scene-graph exclusion sets, on RESOLVED node names
				// (the appdata value's case may differ from the actual node name)
				std::map<std::string, INode *>::iterator sit = nodesByName.find(NLMISC::toLowerAscii(nm));
				if (sit != nodesByName.end())
				{
					std::string slaveName = nodeName(*sit->second);
					lmc->LodSlaveNames.insert(slaveName);
					lmc->LodParents[NLMISC::toLowerAscii(slaveName)].insert(nodeName(*allNodes[i]));
				}
			}
		}
	}

	// Per node
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
		std::string name = nodeName(node);

		CSceneClass *base = baseObjectOf(node, nullptr, nullptr);
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

		if (lmc)
			lmc->CurrentCoarse = haveCoarse;

		NL3D::IShape *shape = buildShapeForNode(node, tmCache, nodesByName, exportLighting, stats, ssc, lmc);
		if (!shape)
			continue;

		// setDistMax was previously called uniformly here; the reference exporter's
		// CExportNel::buildShape gates it on the mesh path only (`!multiLodObject && buildLods`
		// block at export_mesh.cpp:466) and RETURNS EARLY before that block for flare/water/
		// remanence/wavemaker/particle-system builds, so those keep their shape ctor default
		// (CFlareShape default = 1000). Discovered when flare export was off by exactly
		// LOD_DIST_MAX on zo_flare (300 vs ref 1000) even though every other field matched
		// byte-for-byte. Now applied inside the mesh/multi-lod branch of buildShapeForNode.

		try
		{
			// Serialize via a PID+node temp file, then patch the serialMeshBase version byte 10 -> 9:
			// current NL3D writes CMeshBase version 10 ("Ryzom Core release check", 2024) which adds
			// no fields over the export-era version 9 — same pure-version-byte class as the zone
			// v4/v5 byte (see pipeline_max_design.md §10h). Stream layout for the CMeshBase shapes:
			// "SHAP" + u64 0x1 + u32 nameLen + name + classVersion byte + meshBase version byte.
			//
			// COFile-not-CMemStream: CMeshMRMGeom::save uses a seek-back-then-forward pattern to
			// patch its per-lod offsets after writing each lod (mesh_mrm.cpp:1942-1972). CMemStream
			// rejects the forward seek because its `length()` (checked in `seek(begin)`) returns the
			// current write position, not the max ever written — so the writer silently overwrites
			// its own lod-offset placeholders and the resulting file cannot be loaded (see
			// pipeline_max_design.md §9 T2 note for the same limitation on the corpus tester). Route
			// the initial serialize through a real file to sidestep the CMemStream constraint,
			// exactly the way pipeline_max_corpus_test T2 does.
			char tmpPath[256];
			sprintf(tmpPath, "/tmp/pipeline_max_export_shape.%d.tmp", (int)PMB_GETPID());
			{
				NLMISC::COFile ofile;
				if (!ofile.open(tmpPath))
				{
					fprintf(stderr, "ERROR: cannot open %s for writing\n", tmpPath);
					delete shape;
					continue;
				}
				NL3D::CShapeStream shapeStream(shape);
				shapeStream.serial(ofile);
				ofile.close();
			}
			std::vector<uint8> memBuf;
			{
				NLMISC::CIFile ifile;
				if (!ifile.open(tmpPath))
				{
					fprintf(stderr, "ERROR: cannot open %s for reading\n", tmpPath);
					delete shape;
					continue;
				}
				memBuf.resize(ifile.getFileSize());
				if (!memBuf.empty())
					ifile.serialBuffer(&memBuf[0], (uint)memBuf.size());
				ifile.close();
			}
			uint8 *buf = memBuf.empty() ? nullptr : &memBuf[0];
			uint32 len = (uint32)memBuf.size();
			{
				std::string className = shape->getClassName();
				// CMeshBase-derived shapes: CMesh/CMeshMRM/CMeshMRMSkinned write their OWN
				// version byte first, THEN CMeshBase's version byte — the "+1" skips over the
				// outer class version to reach CMeshBase's (see the two-level serial in
				// CMesh::serial calling CMeshBase::serial). For a plain-IShape-derived shape
				// like CWaterShape there's no inner class, so the version byte sits right at
				// the end of the class name (no +1 offset).
				uint32 meshBaseVerOff = 4 + 8 + 4 + (uint32)className.size() + 1;
				if ((className == "CMesh" || className == "CMeshMRM" || className == "CMeshMRMSkinned")
					&& meshBaseVerOff < len && buf[meshBaseVerOff] == 10)
					buf[meshBaseVerOff] = 9;
				// CWaterShape: reference-era exports are stream version 4 (2004 plugin build).
				// Our v7 adds RealtimeReflection (1 byte) + fresnel bias/scale/power (12 bytes)
				// + EnvMapCalcReflectivity (1 byte) at the END of the serial, in that order. To
				// emit a v4 stream from a v7 in-memory shape: patch the version byte + truncate
				// the trailing 14 bytes. Safe because those 4 fields sit right at the end and
				// aren't referenced by any later field (the CMeshBase v10→v9 pattern is a pure
				// version bump with no data change — this one has data to peel off too, but the
				// same structural principle applies).
				uint32 waterVerOff = 4 + 8 + 4 + (uint32)className.size();
				if (className == "CWaterShape" && waterVerOff < len && buf[waterVerOff] == 7
					&& len > 14)
				{
					buf[waterVerOff] = 4;
					len -= 14;
				}
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
	}

	// Lightmap scene-graph: when receivers were collected, add the lights and the occluder
	// meshes and write <lmSceneDir>/<maxstem>.lmscene (design doc §11-lm). Occluder selection
	// replicates the reference flow: the shape maxscript selects ROOT-level geometry nodes
	// with the cast-shadow node flag, and CRTWorld::addNode then filters non-zone,
	// non-accelerator, shadow-casting meshes (bExcludeNonSelected=true in the batch export).
	// Receivers themselves ride the lightmapper's include path, not this list.
	if (lmc && !lmc->Scene.Receivers.empty())
	{
		// Persistent node handles (chunk 0x0a32) — light exclusion lists reference nodes
		// by handle.
		std::map<uint32, std::string> nodeByHandle;
		for (uint i = 0; i < allNodes.size(); ++i)
		{
			CNodeImpl *n = dynamic_cast<CNodeImpl *>(allNodes[i]);
			uint32 h;
			if (n && LMSCENE::nodeHandle(n, h))
				nodeByHandle[h] = nodeName(*allNodes[i]);
		}

		// Lights (all scene lights checked for lightmap export, hidden included). The
		// reference enumerates them by NODE-TREE pre-order recursion (getLightNodeList),
		// not storage order — the enumeration order decides the light order inside each
		// lightmap layer and the layer creation order (the §10w tree-vs-storage lesson).
		{
			std::map<INode *, std::vector<INode *> > kids;
			for (uint i = 0; i < allNodes.size(); ++i)
			{
				CNodeImpl *n = dynamic_cast<CNodeImpl *>(allNodes[i]);
				if (n) kids[n->parent()].push_back(n);
			}
			std::vector<INode *> stack;
			INode *sceneRoot = ssc->scene()->rootNode();
			INode *seeds[2] = { sceneRoot, nullptr };
			for (int s = 0; s < 2; ++s)
			{
				if (s == 1 && seeds[0] == seeds[1]) continue;
				std::map<INode *, std::vector<INode *> >::iterator it = kids.find(seeds[s]);
				if (it == kids.end()) continue;
				for (std::vector<INode *>::const_reverse_iterator ki = it->second.rbegin(); ki != it->second.rend(); ++ki)
					stack.push_back(*ki);
			}
			while (!stack.empty())
			{
				INode *node = stack.back();
				stack.pop_back();
				NL3D::CLightmapLight light;
				if (LMSCENE::convertLightmapLight(light, *node, tmCache, nodeByHandle))
					lmc->Scene.Lights.push_back(light);
				std::map<INode *, std::vector<INode *> >::iterator it = kids.find(node);
				if (it != kids.end())
					for (std::vector<INode *>::const_reverse_iterator ki = it->second.rbegin(); ki != it->second.rend(); ++ki)
						stack.push_back(*ki);
			}
		}

		// Occluders
		SExportStats occStats; // throwaway (occluder eval failures don't count as skips)
		for (uint i = 0; i < allNodes.size(); ++i)
		{
			INode &node = *allNodes[i];
			CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
			if (!n) continue;
			// Root-level only (the maxscript's `node.parent == undefined` selection)
			if (dynamic_cast<CNodeImpl *>(n->parent())) continue;
			std::string name = nodeName(node);
			CSceneClass *base = baseObjectOf(node, nullptr, nullptr);
			if (!base || base->classDesc()->superClassId() != SCLASS_GEOMOBJECT) continue;
			NLMISC::CClassId cid = base->classDesc()->classId();
			if (cid == CLASSID_RPO) continue; // zones never occlude (RPO::isZone in addNode)
			if (cid.a() == CLASSID_PARTA_NEL_PS) continue;
			if (cid == CLASSID_PACS_BOX || cid == CLASSID_PACS_CYL) continue;
			// Accelerators don't occlude ((accel & 3) == 0 filter in addNode)
			{
				int accel = getScriptAppDataInt(n, NEL3D_APPDATA_ACCEL, 32);
				if ((accel & 3) != 0) continue;
			}
			if (lmc->OccluderNames.count(name)) continue;

			NL3D::CLightmapSceneMesh occ;
			occ.NodeName = name;
			SMaxMeshBaseBuild occMaxBase;
			Matrix3M occLocalTM = getLocalMatrix(node, tmCache);
			buildBaseMeshInterface(occ.BaseBuild, occMaxBase, node, tmCache, occLocalTM, exportLighting);
			// The cast-shadow node flag gates the world (same flag the maxscript selection
			// and addNode's bCastShadows check read)
			if (!occ.BaseBuild.bCastShadows) continue;
			// Occluders evaluate as plain meshes (the reference's createMeshBuild ignores
			// skinning for the raytrace world)
			if (!evalAndBuildMesh(node, tmCache, occ.BaseBuild, occMaxBase, occ.MeshBuild, occStats, false))
				continue;
			lmc->OccluderNames.insert(name);
			lmc->Scene.Occluders.push_back(occ);
		}

		// Project name = the source scene's file stem (prefixes the lightmap texture names)
		lmc->Scene.ProjectName = NLMISC::CFile::getFilenameWithoutExtension(maxPath);

		std::string scenePath = lmSceneDir + "/"
			+ NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath)) + ".lmscene";
		try
		{
			lmc->Scene.save(scenePath);
			printf("LMSCENE %s (%u receivers, %u occluders, %u lights)\n", scenePath.c_str(),
			       (uint)lmc->Scene.Receivers.size(), (uint)lmc->Scene.Occluders.size(),
			       (uint)lmc->Scene.Lights.size());
		}
		catch (const NLMISC::Exception &e)
		{
			fprintf(stderr, "ERROR: cannot write %s: %s\n", scenePath.c_str(), e.what());
		}
	}

	// Shape process "Export default animations" (shape_export.ms):
	//   for node in objects do if isAnimToBeExported node then NelExportAnimation #(node)
	// This is a SEPARATE pass over ALL scene objects — not just geometry. Lights with
	// AUTOMATIC_ANIMATION + LM_ANIMATED produce LightmapController tracks (brazero/lanterne);
	// mesh nodes produce transform / material-texmat / morph tracks. Gating only on the
	// geometry branch previously dropped every light-only and transform-only .anim.
	if (!animDir.empty())
	{
		for (uint i = 0; i < allNodes.size(); ++i)
		{
			INode &node = *allNodes[i];
			if (!SHAPEANIM::isAnimToBeExported(node))
				continue;
			std::string name = nodeName(node);
			NL3D::CAnimation animation;
			uint nTracks = SHAPEANIM::buildNodeAnim(node, animation);
			// Always write when the node is flagged — empty NEL_ANIM files exist in the
			// reference corpus (Sun.anim, 29 bytes) for nodes with AUTOMATIC_ANIMATION but
			// no exportable keyed controllers.
			(void)nTracks;
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

	return 0;
}

// ---------------------------------------------------------------------------------------------
// Compare mode: byte compare, then a field-level triage over the loaded shapes.

static NL3D::IShape *loadShape(const std::string &path)
{
	try
	{
		NLMISC::CIFile f;
		if (!f.open(path)) return nullptr;
		NL3D::CShapeStream ss;
		ss.serial(f);
		return ss.getShapePointer();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "load %s: %s\n", path.c_str(), e.what());
		return nullptr;
	}
}

// Dump a loaded skinned shape's bone names + per-vertex SkinWeights (CMeshMRM and CMeshMRMSkinned
// both expose getSkinWeights()/getBonesName()). Used to validate the Physique decode (boneRef =
// ~index into the modifier's reference array; raw weight normalized top-4) field-by-field against
// the reference .shape before committing to the full build path.
static const std::vector<NL3D::CMesh::CSkinWeight> *dumpSkinWeightsOf(NL3D::IShape *shape,
                                                                       std::vector<std::string> &bonesNames)
{
	NL3D::CMeshMRM *mrm = dynamic_cast<NL3D::CMeshMRM *>(shape);
	NL3D::CMeshMRMSkinned *mrms = dynamic_cast<NL3D::CMeshMRMSkinned *>(shape);
	if (mrm)
	{
		const NL3D::CMeshMRMGeom &g = mrm->getMeshGeom();
		bonesNames = g.getBonesName();
		// _SkinWeights lives on the geom; store a copy so the caller holds stable storage.
		static std::vector<NL3D::CMesh::CSkinWeight> sw;
		sw = g.getSkinWeights();
		return &sw;
	}
	if (mrms)
	{
		// CMeshMRMSkinned reconstructs the full SkinWeights array on demand.
		static std::vector<NL3D::CMesh::CSkinWeight> sw;
		const NL3D::CMeshMRMSkinnedGeom &g = mrms->getMeshGeom();
		g.getSkinWeights(sw);
		bonesNames = g.getBonesName();
		return &sw;
	}
	return nullptr;
}

static int dumpSkin(const std::string &path)
{
	NL3D::IShape *shape = loadShape(path);
	if (!shape) { fprintf(stderr, "cannot load %s\n", path.c_str()); return 2; }
	printf("class %s\n", shape->getClassName().c_str());
	std::vector<std::string> bones;
	const std::vector<NL3D::CMesh::CSkinWeight> *sw = dumpSkinWeightsOf(shape, bones);
	if (!sw) { printf("(not a skinned MRM shape)\n"); delete shape; return 0; }
	printf("bones %u\n", (uint)bones.size());
	for (uint i = 0; i < bones.size(); ++i)
		printf("  bone[%u] '%s'\n", i, bones[i].c_str());
	printf("verts %u\n", (uint)sw->size());
	// Positions: both CMeshMRM and CMeshMRMSkinned expose the final VB (the skinned class
	// dequantizes its packed int16 positions), parallel to the SkinWeights array — used for
	// position-matched weight comparison against a reference shape (PMB_SKIN_DUMP_ALL for the
	// full listing; default keeps the original 16-vert preview).
	NL3D::CVertexBuffer vbStore;
	const NL3D::CVertexBuffer *vbp = nullptr;
	if (NL3D::CMeshMRM *mrm = dynamic_cast<NL3D::CMeshMRM *>(shape))
		vbp = &mrm->getMeshGeom().getVertexBuffer();
	else if (NL3D::CMeshMRMSkinned *mrms = dynamic_cast<NL3D::CMeshMRMSkinned *>(shape))
	{
		mrms->getMeshGeom().getVertexBuffer(vbStore);
		vbp = &vbStore;
	}
	uint nShow = getenv("PMB_SKIN_DUMP_ALL") ? (uint)sw->size() : std::min((uint)sw->size(), (uint)16u);
	NL3D::CVertexBufferRead vbr;
	const uint8 *posPtr = nullptr;
	uint stride = 0;
	if (vbp && vbp->getNumVertices() >= sw->size())
	{
		const_cast<NL3D::CVertexBuffer *>(vbp)->lock(vbr);
		posPtr = (const uint8 *)vbr.getVertexCoordPointer();
		stride = vbp->getVertexSize();
	}
	for (uint v = 0; v < nShow; ++v)
	{
		const NL3D::CMesh::CSkinWeight &s = (*sw)[v];
		printf("  v%u:", v);
		if (posPtr)
		{
			float p[3];
			memcpy(p, posPtr + v * stride, 12);
			printf(" pos(%.9g %.9g %.9g)", p[0], p[1], p[2]);
		}
		for (uint i = 0; i < 4; ++i) printf(" [%u]=%.6g", s.MatrixId[i], s.Weights[i]);
		printf("  |");
		for (uint i = 0; i < 4; ++i)
			if (s.Weights[i] != 0.f && s.MatrixId[i] < bones.size())
				printf(" %s=%.6g", bones[s.MatrixId[i]].c_str(), s.Weights[i]);
		printf("\n");
	}
	delete shape;
	return 0;
}

// --dump-mesh: print a CMesh's VB (all values) + per-rdrpass index triples — GT extraction for
// decode work (spline caps, UVW Map projections) against reference shapes.
static int dumpMesh(const std::string &path)
{
	NL3D::IShape *shape = loadShape(path);
	if (!shape) { fprintf(stderr, "cannot load %s\n", path.c_str()); return 2; }
	printf("class %s\n", shape->getClassName().c_str());
	NL3D::CMesh *m = dynamic_cast<NL3D::CMesh *>(shape);
	if (!m) { printf("(not a CMesh)\n"); delete shape; return 0; }
	const NL3D::CVertexBuffer &vb = m->getVertexBuffer();
	printf("verts %u format 0x%x size %u\n", vb.getNumVertices(), vb.getVertexFormat(), vb.getVertexSize());
	NL3D::CVertexBufferRead vbr;
	const_cast<NL3D::CVertexBuffer &>(vb).lock(vbr);
	const uint8 *p = (const uint8 *)vbr.getVertexCoordPointer();
	uint stride = vb.getVertexSize();
	for (uint v = 0; v < vb.getNumVertices(); ++v)
	{
		printf("  v%u:", v);
		for (uint value = 0; value < NL3D::CVertexBuffer::NumValue; ++value)
		{
			if (!(vb.getVertexFormat() & (1 << value))) continue;
			uint off = vb.getValueOffEx((NL3D::CVertexBuffer::TValue)value);
			NL3D::CVertexBuffer::TType type = vb.getValueType(value);
			uint nComp = NL3D::CVertexBuffer::NumComponentsType[type];
			bool isFloat = type == NL3D::CVertexBuffer::Float1 || type == NL3D::CVertexBuffer::Float2
				|| type == NL3D::CVertexBuffer::Float3 || type == NL3D::CVertexBuffer::Float4;
			printf(" val%u(", value);
			for (uint c = 0; c < nComp; ++c)
			{
				if (isFloat)
				{
					float f;
					memcpy(&f, p + v * stride + off + c * 4, 4);
					printf("%s%.9g", c ? " " : "", f);
				}
				else
				{
					printf("%s%u", c ? " " : "", (uint)p[v * stride + off + c]);
				}
			}
			printf(")");
		}
		printf("\n");
	}
	for (uint mbk = 0; mbk < m->getNbMatrixBlock(); ++mbk)
	{
		for (uint rp = 0; rp < m->getNbRdrPass(mbk); ++rp)
		{
			const NL3D::CIndexBuffer &ib = m->getRdrPassPrimitiveBlock(mbk, rp);
			printf("rdrpass %u material %u tris %u:", rp, m->getRdrPassMaterial(mbk, rp), ib.getNumIndexes() / 3);
			NL3D::CIndexBufferRead ibr;
			const_cast<NL3D::CIndexBuffer &>(ib).lock(ibr);
			if (ib.getFormat() == NL3D::CIndexBuffer::Indices16)
			{
				const uint16 *ix = (const uint16 *)ibr.getPtr();
				for (uint i = 0; i + 2 < ib.getNumIndexes(); i += 3)
					printf(" (%u %u %u)", ix[i], ix[i+1], ix[i+2]);
			}
			else
			{
				const uint32 *ix = (const uint32 *)ibr.getPtr();
				for (uint i = 0; i + 2 < ib.getNumIndexes(); i += 3)
					printf(" (%u %u %u)", ix[i], ix[i+1], ix[i+2]);
			}
			printf("\n");
		}
	}
	delete shape;
	return 0;
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

// --compare-mapext-mask mode: the reference .shape UVs of Map Extender assets are garbage
// (the 2004 plugin leaked uninitialized memory — design doc §9 / defective_max_files §4.4),
// which poisons everything the vertex dedup touches: UV values, vertex count/order, index
// content. This mode compares what garbage UVs canNOT poison — transform, the full material
// serialization, matrix-block/rdrpass counts, per-rdrpass material assignment and index
// COUNT (3× the per-material face count, dedup-independent) — upgrading the mapext bucket
// from "excluded" to "verified except UV". Shares the VB/index handling with the lightmap
// mask via g_uvMask; the LightMap material masking stays lightmap-only.
static bool g_mapextMask = false;

// Either mask that makes VB content/count and index CONTENT incomparable-by-design.
static bool uvMask()
{
	return g_lightmapMask || g_mapextMask;
}

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
	if (uvMask())
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

		// Under EITHER UV mask: skip appended-texture + calc-lm-touched compare paths, verify
		// only the base fields the lightmapper does not modify. LightMap-shaded materials in
		// the reference carry (a) lightmap textures appended after slot 0, (b) calc-lm-modified
		// ambient/specular/shininess, (c) an added Stage-1 texenv. Compare shader type + blend
		// + z-write + two-sided + color + slot-0 texture presence and STOP. Initially lightmap-
		// mask-only; extended to the mapext mask 2026-07-10 — 27 of the 47 mapext-diff files
		// (the gen_bt construction class) are mapext+LightMap combos whose reference materials
		// carry the same calc-lm modifications regardless of which bucket routed the compare.
		if (uvMask() && m_b.getShader() == NL3D::CMaterial::LightMap)
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
			if ((ta != nullptr) != (tb != nullptr))
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

// Compare two textures by class name + file path. Handles CTextureFile, CTextureMultiFile,
// CTextureBlend (recursive on its two blend inputs). NULL vs NULL = match; NULL vs !NULL = raise.
// Takes non-const refs because ITexture::getClassName is non-const (IClassable convention).
static void compareTexture(const char *label, NL3D::ITexture *ta, NL3D::ITexture *tb)
{
	if (!ta && !tb) return;
	if (!ta || !tb)
	{
		printf("  %s: %s vs %s\n", label, ta ? ta->getClassName().c_str() : "NULL",
		       tb ? tb->getClassName().c_str() : "NULL");
		raiseVerdict(2);
		return;
	}
	if (ta->getClassName() != tb->getClassName())
	{
		printf("  %s: class %s vs %s\n", label, ta->getClassName().c_str(), tb->getClassName().c_str());
		raiseVerdict(2);
		return;
	}
	NL3D::CTextureFile *fa = dynamic_cast<NL3D::CTextureFile *>(ta);
	NL3D::CTextureFile *fb = dynamic_cast<NL3D::CTextureFile *>(tb);
	if (fa && fb)
	{
		if (NLMISC::toLowerAscii(fa->getFileName()) != NLMISC::toLowerAscii(fb->getFileName()))
		{
			printf("  %s: file '%s' vs '%s'\n", label, fa->getFileName().c_str(), fb->getFileName().c_str());
			raiseVerdict(2);
		}
		return;
	}
	NL3D::CTextureMultiFile *mfa = dynamic_cast<NL3D::CTextureMultiFile *>(ta);
	NL3D::CTextureMultiFile *mfb = dynamic_cast<NL3D::CTextureMultiFile *>(tb);
	if (mfa && mfb)
	{
		if (mfa->getNumFileName() != mfb->getNumFileName())
		{
			printf("  %s: multi-file slot count %u vs %u\n", label,
			       mfa->getNumFileName(), mfb->getNumFileName());
			raiseVerdict(2);
			return;
		}
		for (uint i = 0; i < mfa->getNumFileName(); ++i)
			if (NLMISC::toLowerAscii(mfa->getFileName(i)) != NLMISC::toLowerAscii(mfb->getFileName(i)))
			{
				printf("  %s: multi-file slot %u '%s' vs '%s'\n", label, i,
				       mfa->getFileName(i).c_str(), mfb->getFileName(i).c_str());
				raiseVerdict(2);
			}
		return;
	}
	NL3D::CTextureBlend *ba = dynamic_cast<NL3D::CTextureBlend *>(ta);
	NL3D::CTextureBlend *bb = dynamic_cast<NL3D::CTextureBlend *>(tb);
	if (ba && bb)
	{
		compareTexture(label, ba->getBlendtexture(0), bb->getBlendtexture(0));
		compareTexture(label, ba->getBlendtexture(1), bb->getBlendtexture(1));
		return;
	}
	// Fall through: unknown texture class — flag structurally
	printf("  %s: unhandled texture class %s\n", label, ta->getClassName().c_str());
	raiseVerdict(2);
}

// CWaterShape field walk. CWaterShape does NOT derive from CMeshBase (it's plain IShape) but it
// carries the same default transform triple through separate CTrackDefault* members. Reason for
// the walk: byte-compare of otherwise-correct water shapes fails on the DefaultRotQuat MSB-of-
// x/y/z triple that decompMatrix computes as (-x,-y,-z,-w) on our tree vs (+x,+y,+z,-w) on the
// reference build (double-cover boundary; §10z). Everything else — bump/displace params, textures
// (incl. day/night CTextureBlend), pool ID, splash flag — was already byte-exact in §10z round 2.
static void compareWaterShape(NL3D::CWaterShape *wa, NL3D::CWaterShape *wb)
{
	// Default transform (same triple as CMeshBase; double-cover aware quat, float-noise triple)
	{
		NLMISC::CVector va = wa->getDefaultPos()->getDefaultValue();
		NLMISC::CVector vb = wb->getDefaultPos()->getDefaultValue();
		if (va != vb)
		{
			printf("  DefaultPos: (%.9g %.9g %.9g) vs (%.9g %.9g %.9g)\n", va.x, va.y, va.z, vb.x, vb.y, vb.z);
			raiseVerdict((floatNoise(va.x, vb.x) && floatNoise(va.y, vb.y) && floatNoise(va.z, vb.z)) ? 1 : 2);
		}
	}
	{
		NLMISC::CQuat qa = wa->getDefaultRotQuat()->getDefaultValue();
		NLMISC::CQuat qb = wb->getDefaultRotQuat()->getDefaultValue();
		if (qa.x != qb.x || qa.y != qb.y || qa.z != qb.z || qa.w != qb.w)
		{
			printf("  DefaultRotQuat: (%.9g %.9g %.9g %.9g) vs (%.9g %.9g %.9g %.9g)\n",
			       qa.x, qa.y, qa.z, qa.w, qb.x, qb.y, qb.z, qb.w);
			bool direct = floatNoise(qa.x, qb.x) && floatNoise(qa.y, qb.y) && floatNoise(qa.z, qb.z) && floatNoise(qa.w, qb.w);
			bool negated = floatNoise(qa.x, -qb.x) && floatNoise(qa.y, -qb.y) && floatNoise(qa.z, -qb.z) && floatNoise(qa.w, -qb.w);
			raiseVerdict((direct || negated) ? 1 : 2);
		}
	}
	{
		NLMISC::CVector sa = wa->getDefaultScale()->getDefaultValue();
		NLMISC::CVector sb = wb->getDefaultScale()->getDefaultValue();
		if (sa != sb)
		{
			printf("  DefaultScale: (%.9g %.9g %.9g) vs (%.9g %.9g %.9g)\n", sa.x, sa.y, sa.z, sb.x, sb.y, sb.z);
			raiseVerdict((floatNoise(sa.x, sb.x) && floatNoise(sa.y, sb.y) && floatNoise(sa.z, sb.z)) ? 1 : 2);
		}
	}
	// WaterPoolID, WaveHeightFactor, splash flag, use-scene-env-map
	if (wa->getWaterPoolID() != wb->getWaterPoolID())
	{
		printf("  WaterPoolID: %u vs %u\n", wa->getWaterPoolID(), wb->getWaterPoolID());
		raiseVerdict(2);
	}
	if (wa->getWaveHeightFactor() != wb->getWaveHeightFactor())
	{
		printf("  WaveHeightFactor: %g vs %g\n", wa->getWaveHeightFactor(), wb->getWaveHeightFactor());
		raiseVerdict(floatNoise(wa->getWaveHeightFactor(), wb->getWaveHeightFactor()) ? 1 : 2);
	}
	if (wa->isSplashEnabled() != wb->isSplashEnabled())
	{
		printf("  Splash: %d vs %d\n", (int)wa->isSplashEnabled(), (int)wb->isSplashEnabled());
		raiseVerdict(2);
	}
	for (uint i = 0; i < 2; ++i)
	{
		if (wa->getUseSceneWaterEnvMap(i) != wb->getUseSceneWaterEnvMap(i))
		{
			printf("  UsesSceneWaterEnvMap[%u]: %d vs %d\n", i, (int)wa->getUseSceneWaterEnvMap(i),
			       (int)wb->getUseSceneWaterEnvMap(i));
			raiseVerdict(2);
		}
		NLMISC::CVector2f hsa = wa->getHeightMapScale(i);
		NLMISC::CVector2f hsb = wb->getHeightMapScale(i);
		if (hsa.x != hsb.x || hsa.y != hsb.y)
		{
			printf("  HeightMapScale[%u]: (%g %g) vs (%g %g)\n", i, hsa.x, hsa.y, hsb.x, hsb.y);
			raiseVerdict((floatNoise(hsa.x, hsb.x) && floatNoise(hsa.y, hsb.y)) ? 1 : 2);
		}
		NLMISC::CVector2f hspa = wa->getHeightMapSpeed(i);
		NLMISC::CVector2f hspb = wb->getHeightMapSpeed(i);
		if (hspa.x != hspb.x || hspa.y != hspb.y)
		{
			printf("  HeightMapSpeed[%u]: (%g %g) vs (%g %g)\n", i, hspa.x, hspa.y, hspb.x, hspb.y);
			raiseVerdict((floatNoise(hspa.x, hspb.x) && floatNoise(hspa.y, hspb.y)) ? 1 : 2);
		}
	}
	// Polygon: same vertex count, position float-noise tolerated (world-vertex xform noise, same
	// POS_EPS family as §10g).
	const NLMISC::CPolygon2D &pa = wa->getShape();
	const NLMISC::CPolygon2D &pb = wb->getShape();
	if (pa.Vertices.size() != pb.Vertices.size())
	{
		printf("  Poly vertices: %u vs %u\n", (uint)pa.Vertices.size(), (uint)pb.Vertices.size());
		raiseVerdict(2);
	}
	else
	{
		uint firstDiff = 0xFFFFFFFF;
		uint nDiff = 0;
		float maxAbs = 0.0f;
		for (uint i = 0; i < pa.Vertices.size(); ++i)
		{
			const NLMISC::CVector2f &va = pa.Vertices[i];
			const NLMISC::CVector2f &vb = pb.Vertices[i];
			if (va.x != vb.x || va.y != vb.y)
			{
				++nDiff;
				if (firstDiff == 0xFFFFFFFF) firstDiff = i;
				maxAbs = std::max(maxAbs, (float)std::max(fabs(va.x - vb.x), fabs(va.y - vb.y)));
				raiseVerdict((floatNoise(va.x, vb.x) && floatNoise(va.y, vb.y)) ? 1 : 2);
			}
		}
		if (nDiff)
			printf("  Poly: %u verts differ (first %u), maxAbs %g\n", nDiff, firstDiff, maxAbs);
	}
	// Textures
	compareTexture("EnvMap[0]", wa->getEnvMap(0), wb->getEnvMap(0));
	compareTexture("EnvMap[1]", wa->getEnvMap(1), wb->getEnvMap(1));
	compareTexture("HeightMap[0]", wa->getHeightMap(0), wb->getHeightMap(0));
	compareTexture("HeightMap[1]", wa->getHeightMap(1), wb->getHeightMap(1));
	compareTexture("ColorMap", wa->getColorMap(), wb->getColorMap());
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
	NL3D::CWaterShape *wa = dynamic_cast<NL3D::CWaterShape *>(sa);
	NL3D::CWaterShape *wb = dynamic_cast<NL3D::CWaterShape *>(sb);
	if (wa && wb)
	{
		compareWaterShape(wa, wb);
		delete sa;
		delete sb;
		return;
	}
	NL3D::CSegRemanenceShape *ra = dynamic_cast<NL3D::CSegRemanenceShape *>(sa);
	NL3D::CSegRemanenceShape *rb = dynamic_cast<NL3D::CSegRemanenceShape *>(sb);
	if (ra && rb)
	{
		// Default transform
		{
			NLMISC::CVector pa = ra->getDefaultPos()->getDefaultValue();
			NLMISC::CVector pb = rb->getDefaultPos()->getDefaultValue();
			if (!floatNoise(pa.x, pb.x) || !floatNoise(pa.y, pb.y) || !floatNoise(pa.z, pb.z))
			{ printf("  DefaultPos differs\n"); raiseVerdict(1); }
			NLMISC::CQuat qa = ra->getDefaultRotQuat()->getDefaultValue();
			NLMISC::CQuat qb = rb->getDefaultRotQuat()->getDefaultValue();
			// double-cover aware
			bool qSame = (floatNoise(qa.x, qb.x) && floatNoise(qa.y, qb.y)
			              && floatNoise(qa.z, qb.z) && floatNoise(qa.w, qb.w))
			          || (floatNoise(qa.x, -qb.x) && floatNoise(qa.y, -qb.y)
			              && floatNoise(qa.z, -qb.z) && floatNoise(qa.w, -qb.w));
			if (!qSame) { printf("  DefaultRotQuat differs\n"); raiseVerdict(1); }
			NLMISC::CVector sa2 = ra->getDefaultScale()->getDefaultValue();
			NLMISC::CVector sb2 = rb->getDefaultScale()->getDefaultValue();
			if (!floatNoise(sa2.x, sb2.x) || !floatNoise(sa2.y, sb2.y) || !floatNoise(sa2.z, sb2.z))
			{ printf("  DefaultScale differs\n"); raiseVerdict(1); }
		}
		if (ra->getNumSlices() != rb->getNumSlices())
		{ printf("  NumSlices %u vs %u\n", ra->getNumSlices(), rb->getNumSlices()); raiseVerdict(2); }
		if (!floatNoise(ra->getSliceTime(), rb->getSliceTime()))
		{ printf("  SliceTime differs\n"); raiseVerdict(1); }
		if (!floatNoise(ra->getRollupRatio(), rb->getRollupRatio()))
		{ printf("  RollupRatio differs\n"); raiseVerdict(1); }
		if (ra->getTextureShifting() != rb->getTextureShifting())
		{ printf("  TextureShifting differs\n"); raiseVerdict(2); }
		if (ra->getNumCorners() != rb->getNumCorners())
		{
			printf("  NumCorners %u vs %u\n", ra->getNumCorners(), rb->getNumCorners());
			raiseVerdict(2);
		}
		else
		{
			for (uint i = 0; i < ra->getNumCorners(); ++i)
			{
				NLMISC::CVector ca = ra->getCorner(i), cb = rb->getCorner(i);
				if (!floatNoise(ca.x, cb.x) || !floatNoise(ca.y, cb.y) || !floatNoise(ca.z, cb.z))
				{
					printf("  Corner[%u] (%.9g,%.9g,%.9g) vs (%.9g,%.9g,%.9g)\n",
					       i, ca.x, ca.y, ca.z, cb.x, cb.y, cb.z);
					raiseVerdict(1);
				}
			}
		}
		// Material — structural (shader/blend); float noise on colors is FLOATEQ
		if (ra->getMaterial().getShader() != rb->getMaterial().getShader())
		{ printf("  Material shader differs\n"); raiseVerdict(2); }
		delete sa;
		delete sb;
		return;
	}
	// CMeshMRM / CMeshMRMSkinned: compare bones, skin weights, the (dequantized) VB, and the
	// per-lod rdrpass structure. MRM construction is deterministic on bit-equal inputs (whole
	// skinned files compare byte-identical since the world-space fix), so same-topology output
	// within the quantization/float-noise tier is FLOATEQ; topology differences (different
	// simplification order from float-noise inputs) stay DIFF.
	{
		NL3D::CMeshMRM *mrma = dynamic_cast<NL3D::CMeshMRM *>(sa);
		NL3D::CMeshMRM *mrmb = dynamic_cast<NL3D::CMeshMRM *>(sb);
		NL3D::CMeshMRMSkinned *msa = dynamic_cast<NL3D::CMeshMRMSkinned *>(sa);
		NL3D::CMeshMRMSkinned *msb = dynamic_cast<NL3D::CMeshMRMSkinned *>(sb);
		if ((mrma && mrmb) || (msa && msb))
		{
			compareMeshBase(dynamic_cast<NL3D::CMeshBase *>(sa), dynamic_cast<NL3D::CMeshBase *>(sb));
			// Bones + weights
			std::vector<std::string> bonesA, bonesB;
			const std::vector<NL3D::CMesh::CSkinWeight> *swa = dumpSkinWeightsOf(sa, bonesA);
			std::vector<NL3D::CMesh::CSkinWeight> swbStore;
			const std::vector<NL3D::CMesh::CSkinWeight> *swb = nullptr;
			if (mrmb) { bonesB = mrmb->getMeshGeom().getBonesName(); swbStore = mrmb->getMeshGeom().getSkinWeights(); swb = &swbStore; }
			else { msb->getMeshGeom().getSkinWeights(swbStore); bonesB = msb->getMeshGeom().getBonesName(); swb = &swbStore; }
			if (bonesA != bonesB)
			{
				// Under a UV mask the used-bone ORDER legitimately differs: BonesNames is the
				// skeleton list reduced by first-use order over the VB, and the reference's VB
				// order follows its garbage-UV/lightmap-UV dedup. The bone SET is still exact
				// signal — compare sorted (2026-07-10, the 12-file mapext bones class).
				std::vector<std::string> sortA = bonesA, sortB = bonesB;
				std::sort(sortA.begin(), sortA.end());
				std::sort(sortB.begin(), sortB.end());
				if (!uvMask() || sortA != sortB)
				{
					printf("  bones: %u vs %u (%s mismatch)\n", (uint)bonesA.size(), (uint)bonesB.size(),
					       sortA == sortB ? "order" : "set");
					raiseVerdict(2);
				}
			}
			if (swa && swb)
			{
				if (swa->size() != swb->size())
				{
					printf("  skin weights: %u vs %u verts\n", (uint)swa->size(), (uint)swb->size());
					// Under a UV mask the vertex sets legitimately differ (dedup on garbage or
					// lightmap UVs) — count mismatch is not a verdict by itself there.
					if (!uvMask()) raiseVerdict(2);
				}
				else if (!uvMask())
				{
					// Per-vertex weight compare needs matching vertex ORDER; under a UV mask the
					// dedup orders diverge even when the counts happen to match, so this is
					// skipped there (the bone SET + rdrpass material assignment stay gated).
					uint nDiff = 0;
					for (uint v = 0; v < swa->size(); ++v)
					{
						const NL3D::CMesh::CSkinWeight &A = (*swa)[v], &B = (*swb)[v];
						for (uint k = 0; k < NL3D_MESH_SKINNING_MAX_MATRIX; ++k)
						{
							if (A.MatrixId[k] != B.MatrixId[k]
								&& (A.Weights[k] != 0.f || B.Weights[k] != 0.f))
							{ ++nDiff; break; }
							// CMeshMRMSkinned stores weights quantized uint8/255; tolerate one
							// quantum plus float noise for the CMeshMRM float path.
							if (fabsf(A.Weights[k] - B.Weights[k]) > (msa ? 1.5f / 255.f : 2e-4f))
							{ ++nDiff; break; }
						}
					}
					if (nDiff)
					{
						printf("  skin weights: %u verts differ\n", nDiff);
						raiseVerdict(2);
					}
				}
			}
			// VB (dequantized for the skinned class; positions there snap to int16 quanta, so
			// matching inputs give bit-equal dequantized floats).
			{
				NL3D::CVertexBuffer va, vbb;
				if (mrma) va = mrma->getMeshGeom().getVertexBuffer();
				else msa->getMeshGeom().getVertexBuffer(va);
				if (mrmb) vbb = mrmb->getMeshGeom().getVertexBuffer();
				else msb->getMeshGeom().getVertexBuffer(vbb);
				compareVB(va, vbb);
			}
			// Lods / rdrpass structure
			uint nLodA = mrma ? mrma->getMeshGeom().getNbLod() : msa->getMeshGeom().getNbLod();
			uint nLodB = mrmb ? mrmb->getMeshGeom().getNbLod() : msb->getMeshGeom().getNbLod();
			if (nLodA != nLodB)
			{
				printf("  lods: %u vs %u\n", nLodA, nLodB);
				raiseVerdict(2);
			}
			else
			{
				for (uint l = 0; l < nLodA; ++l)
				{
					uint rpA = mrma ? mrma->getMeshGeom().getNbRdrPass(l) : msa->getMeshGeom().getNbRdrPass(l);
					uint rpB = mrmb ? mrmb->getMeshGeom().getNbRdrPass(l) : msb->getMeshGeom().getNbRdrPass(l);
					if (rpA != rpB)
					{
						printf("  lod %u rdrpass %u vs %u\n", l, rpA, rpB);
						raiseVerdict(2);
						continue;
					}
					for (uint rp = 0; rp < rpA; ++rp)
					{
						uint mA = mrma ? mrma->getMeshGeom().getRdrPassMaterial(l, rp) : msa->getMeshGeom().getRdrPassMaterial(l, rp);
						uint mB = mrmb ? mrmb->getMeshGeom().getRdrPassMaterial(l, rp) : msb->getMeshGeom().getRdrPassMaterial(l, rp);
						if (mA != mB)
						{
							printf("  lod %u rdrpass %u material %u vs %u\n", l, rp, mA, mB);
							raiseVerdict(2);
						}
						NL3D::CIndexBuffer iaStore, ibStore;
						if (mrma) iaStore = mrma->getMeshGeom().getRdrPassPrimitiveBlock(l, rp);
						else msa->getMeshGeom().getRdrPassPrimitiveBlock(l, rp, iaStore);
						if (mrmb) ibStore = mrmb->getMeshGeom().getRdrPassPrimitiveBlock(l, rp);
						else msb->getMeshGeom().getRdrPassPrimitiveBlock(l, rp, ibStore);
						const NL3D::CIndexBuffer &ia = iaStore;
						const NL3D::CIndexBuffer &ib2 = ibStore;
						if (ia.getNumIndexes() != ib2.getNumIndexes())
						{
							printf("  lod %u rdrpass %u indexes %u vs %u\n", l, rp,
							       ia.getNumIndexes(), ib2.getNumIndexes());
							// Per-lod MRM index counts shift with the vertex set under a UV
							// mask (simplification order differs); total face counts are
							// checked via the plain-CMesh path only.
							if (!uvMask()) raiseVerdict(2);
						}
						else if (!uvMask())
						{
							NL3D::CIndexBufferRead ira, irb;
							const_cast<NL3D::CIndexBuffer &>(ia).lock(ira);
							const_cast<NL3D::CIndexBuffer &>(ib2).lock(irb);
							uint bytesPer = ia.getFormat() == NL3D::CIndexBuffer::Indices16 ? 2 : 4;
							if (ia.getFormat() != ib2.getFormat()
								|| memcmp(ira.getPtr(), irb.getPtr(), ia.getNumIndexes() * bytesPer) != 0)
							{
								printf("  lod %u rdrpass %u index content differs\n", l, rp);
								raiseVerdict(2);
							}
						}
					}
				}
			}
			delete sa;
			delete sb;
			return;
		}
	}
	NL3D::CMesh *ma = dynamic_cast<NL3D::CMesh *>(sa);
	NL3D::CMesh *mb = dynamic_cast<NL3D::CMesh *>(sb);
	if (!ma || !mb)
		raiseVerdict(2); // CMeshMultiLod and anything else unclassified stays DIFF for now
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
					else if (!uvMask())
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
	g_verdict = (da.size() == db.size() || uvMask()) ? 0 : 2;
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

	std::string input, outDir, outDirCoarse, animDir, dbRoot, lmSceneDir;
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
		else if (arg == "--lm-scene" && i + 1 < argc)
			lmSceneDir = argv[++i];
		else if (arg == "--no-lighting")
			exportLighting = false;
		else if (arg == "--compare")
		{
			for (int j = i + 1; j < argc; ++j)
				compareArgs.push_back(argv[j]);
			break;
		}
		else if (arg == "--compare-mapext-mask")
		{
			g_mapextMask = true;
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

	for (int i = 1; i < argc; ++i)
	{
		if (std::string(argv[i]) == "--dump-skin" && i + 1 < argc)
			return dumpSkin(argv[i + 1]);
		if (std::string(argv[i]) == "--dump-mesh" && i + 1 < argc)
			return dumpMesh(argv[i + 1]);
	}

	if (input.empty() || outDir.empty())
	{
		fprintf(stderr, "usage: pipeline_max_export_shape [--db <graphics-root>] [--coarse-out <dir>] [--anim-out <dir>] [--lm-scene <dir>] [--no-lighting] [-v] <input.max> <outdir>\n");
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
	int ret = exportFile(input, outDir, outDirCoarse, animDir, lmSceneDir, exportLighting, stats);
	printf("EXPORTED %u shapes, %u skipped, %u anims\n", stats.Exported, stats.Skipped, stats.AnimExported);
	for (std::map<std::string, uint>::iterator it = stats.SkipReasons.begin(); it != stats.SkipReasons.end(); ++it)
		printf("SKIPCLASS %s %u\n", it->first.c_str(), it->second);
	return ret;
}

/* end of file */
