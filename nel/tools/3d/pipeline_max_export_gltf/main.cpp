/**
 * \file main.cpp
 * \brief max2gltf: whole-.max-file conversion to glTF 2.0 with nel_* extras (wiki
 * drafts/max2gltf_plan.md stage 1). Branches at the pre-build model: the same scene_lib/
 * mesh_eval/material_build/mesh_build extraction the direct .shape exporter consumes feeds the
 * glTF encoder, so glTF fidelity measures format loss only, never decode drift. One .gltf +
 * .bin per .max: the full node hierarchy (every node, bit-exact TRS extras), evaluated meshes
 * on the nodes the shape process would export (plus LOD slaves, which the direct route folds
 * into their parent's CMeshMultiLod), materials as PBR interop + exact nel_* extras, and the
 * per-node NeL appdata the shape build consumes downstream.
 *
 * Node selection replicates pipeline_max_export_shape's exportFile; special shape classes the
 * mesh path can't carry yet (water/flare/remanence/wavemaker) tag the node with nel_skip_class
 * instead of a mesh — the differential harness buckets them like the direct route's SKIP lines.
 * Physique skinning and MRM morph targets ARE carried (nel_skin_*, nel_bs_* — stage 2).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

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
#include <nel/3d/register_3d.h>

#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../pipeline_max_export_shape/scene_lib.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_shape/mesh_eval.h"
#include "../pipeline_max_export_shape/material_build.h"
#include "../pipeline_max_export_shape/mesh_build.h"
#include "../pipeline_max_export_shape/interface_build.h"
#include "../pipeline_max_export_common/physique_skin.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"

#include "gltf_build.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;
using namespace MESHBUILD;
using namespace GLTFBUILD;
using namespace NLGLTF;

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

#define NEL3D_APPDATA_IGNAME 1423062564

#define CLASSID_PARTA_NEL_PS 0x58ce2893
#define CLASSID_PARTA_NEL_FLARE 0x4e913532
#define CLASSID_PARTA_NEL_WAVE_MAKER 0x77e24828
static const NLMISC::CClassId CLASSID_PACS_BOX(0x7f374277, 0x5d3971df);
static const NLMISC::CClassId CLASSID_PACS_CYL(0x62a56810, 0x4b3d601c);
static const NLMISC::CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);

static bool g_verbose = false;

// From ../pipeline_max_export_ig/main.cpp (compiled in with PMB_IG_NO_MAIN): the ig process's
// full selection + buildInstanceGroup flow, returning each ig's serialized bytes.
int pmbExportIgsForGltf(const std::string &maxPath,
                        std::vector<std::pair<std::string, std::vector<uint8> > > &igsOut);
void pmbIgAddPsSearchPath(const std::string &path);

struct SExportStats
{
	uint Meshes;
	uint Skipped;
	uint Igs;
	std::map<std::string, uint> SkipReasons;
	SExportStats() : Meshes(0), Skipped(0), Igs(0) { }
	void skip(const std::string &reason)
	{
		++Skipped;
		++SkipReasons[reason];
	}
};

static bool isGeometryOrShape(CSceneClass *base)
{
	if (!base) return false;
	TSClassId scid = base->classDesc()->superClassId();
	return scid == SCLASS_GEOMOBJECT || scid == SCLASS_SHAPE;
}

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

// Interface-weld world matrix — same derivation as pipeline_max_export_shape/main.cpp
// (export_mesh.cpp:1111 semantics, non-skinned form).
static NLMISC::CMatrix interfaceToWorldMat(INode &node, SNodeTMCache &tmCache)
{
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

// Morpher blend-shape targets — verbatim replica of pipeline_max_export_shape's buildBSList
// (itself the reference's getBSMeshBuild): evaluate each Morpher ref 101+i target through the
// non-skinned mesh path with finalSpace = the SOURCE node's NodeTM when the source is skinned,
// then copy the source's (welded) corner normals onto interface-vertex corners. Consumed only
// by the single-mesh MRM branch, exactly like the direct route.
static void buildBSList(INode &node, SNodeTMCache &tmCache,
                        const std::vector<CSceneClass *> &mods,
                        const NL3D::CMesh::CMeshBuild &exportMesh, bool skinned,
                        bool exportLighting,
                        std::vector<NL3D::CMesh::CMeshBuild *> &bsList)
{
	static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);
	CReferenceMaker *morph = NULL;
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
		if (!MESHEVAL::evalNodeMesh(*target, tmesh, NULL))
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

// Base-mesh context of an export node (materials + base build); LOD slaves build their meshes
// against their PARENT's context, exactly like the direct route's multi-lod path.
struct SBaseCtx
{
	bool Ok;
	bool HasLightMap;
	NL3D::CMeshBase::CMeshBaseBuild Bbm;
	SMaxMeshBaseBuild MaxBB;
	std::vector<sint> GltfMats;
	SBaseCtx() : Ok(false), HasLightMap(false) { }
};

typedef std::map<INode *, SBaseCtx> TBaseCtxCache;

static SBaseCtx *baseCtxFor(INode *node, SNodeTMCache &tmCache, bool exportLighting,
                            CGltfBuilder &b, TBaseCtxCache &cache)
{
	TBaseCtxCache::iterator it = cache.find(node);
	if (it != cache.end())
		return it->second.Ok ? &it->second : NULL;
	SBaseCtx &ctx = cache[node];
	Matrix3M localTM = getLocalMatrix(*node, tmCache);
	buildBaseMeshInterface(ctx.Bbm, ctx.MaxBB, *node, tmCache, localTM, exportLighting);
	for (uint i = 0; i < ctx.Bbm.Materials.size(); ++i)
	{
		if (ctx.Bbm.Materials[i].getShader() == NL3D::CMaterial::LightMap)
			ctx.HasLightMap = true;
		std::string err;
		sint mi = b.addMaterial(ctx.Bbm.Materials[i], ctx.MaxBB.MaterialInfo[i].MaterialName, &err);
		if (mi < 0)
		{
			fprintf(stderr, "SKIP gltf '%s': %s\n", nodeName(*node).c_str(), err.c_str());
			return NULL;
		}
		ctx.GltfMats.push_back(mi);
	}
	if (ctx.HasLightMap)
		printf("LIGHTMAP %s\n", NLMISC::toLowerAscii(nodeName(*node)).c_str());
	ctx.Ok = true;
	return &ctx;
}

static int exportFile(const std::string &maxPath, const std::string &outPath, bool exportLighting,
                      SExportStats &stats)
{
	SLoadedMax lm;
	if (!loadMaxFile(maxPath, lm))
		return 1;

	CSceneClassContainer *ssc = lm.Scene->container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = NULL;

	// Node collection (storage order — the same enumeration the direct exporters walk)
	std::map<std::string, INode *> nodesByName;
	std::vector<INode *> allNodes;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		allNodes.push_back(node);
		nodesByName[NLMISC::toLowerAscii(nodeName(*node))] = node;
	}

	// LOD slave set + slave -> parent (first claimant wins; duplicates warn)
	std::set<std::string> lodNames;
	std::map<std::string, INode *> slaveOwner;
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(allNodes[i]);
		uint count = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
		for (uint l = 0; l < count && l < 10; ++l)
		{
			std::string nm = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_NAME + l, "");
			if (nm.empty()) continue;
			std::string key = NLMISC::toLowerAscii(nm);
			lodNames.insert(key);
			if (slaveOwner.count(key) && slaveOwner[key] != allNodes[i])
				fprintf(stderr, "WARNING: LOD slave '%s' claimed by multiple parents\n", nm.c_str());
			else
				slaveOwner[key] = allNodes[i];
		}
	}

	CGltfBuilder b;
	b.assetExtras()->setString("nel_source", NLMISC::CFile::getFilename(maxPath));

	// Pass 1: every scene node becomes a glTF node (bit-exact TRS extras)
	std::map<INode *, sint> nodeIdx;
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		NLMISC::CVector pos, scale;
		NLMISC::CQuat rot;
		MAXSCENE::decompMatrix(scale, rot, pos, getLocalMatrix(node, tmCache));
		nodeIdx[&node] = b.addNode(nodeName(node), pos, rot, scale);
	}
	{
		std::vector<sint> roots;
		std::map<INode *, std::vector<sint> > children;
		for (uint i = 0; i < allNodes.size(); ++i)
		{
			INode *p = allNodes[i]->parent();
			if (p && dynamic_cast<CNodeImpl *>(p) && nodeIdx.count(p))
				children[p].push_back(nodeIdx[allNodes[i]]);
			else
				roots.push_back(nodeIdx[allNodes[i]]);
		}
		for (std::map<INode *, std::vector<sint> >::iterator it = children.begin(); it != children.end(); ++it)
			b.setNodeChildren(nodeIdx[it->first], it->second);
		b.setSceneRoots(roots);
	}

	// Pass 2: meshes + per-node NeL appdata on the shape-process node selection
	TBaseCtxCache ctxCache;
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
		std::string name = nodeName(node);
		CJsonValue *extras = b.nodeExtras(nodeIdx[&node]);

		// IG membership tag (artist-visible; the byte-exact ig data rides the asset-level
		// nel_igs blob — see below)
		{
			std::string igName = getScriptAppDataStr(n, NEL3D_APPDATA_IGNAME, "");
			if (!igName.empty())
				extras->setString("nel_ig_name", igName);
		}

		std::vector<CSceneClass *> mods;
		std::vector<CStorageContainer *> modApps;
		CSceneClass *base = baseObjectOf(node, &mods, &modApps);
		if (!isGeometryOrShape(base))
			continue;
		NLMISC::CClassId cid = base->classDesc()->classId();

		bool isSlave = lodNames.count(NLMISC::toLowerAscii(name)) != 0;

		// Standalone selection filters (the direct route's exportFile gate). LOD slaves bypass
		// ALL of them: the direct multilod branch resolves slaves by name and takes them straight
		// to the mesh eval — no bip/classid/appdata checks on slave nodes.
		if (!isSlave)
		{
			if (startsWithBip(name) || startsWithBip(nodeName(*rootOf(&node))))
				continue;
			if (cid == CLASSID_RPO || cid.a() == CLASSID_PARTA_NEL_PS
				|| cid == CLASSID_PACS_BOX || cid == CLASSID_PACS_CYL || cid == CLASSID_TARGET)
				continue;
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
		}

		// Special shape classes — same dispatch order as buildShapeForNode; the glTF mesh path
		// can't carry these yet, so the node is tagged for the harness instead. Physique is
		// CARRIED (skinning applies onto the mesh build below, like the direct route); the
		// Max-4+ Skin modifier skips the whole node exactly like the direct route does. The
		// direct multilod slave loop re-detects Physique ONLY — mirror that asymmetry.
		const char *skipClass = NULL;
		bool hasPhysique = false;
		if (!isSlave && cid.a() == CLASSID_PARTA_NEL_WAVE_MAKER)
			skipClass = "wavemaker";
		else if (!isSlave && getScriptAppDataInt(n, NEL3D_APPDATA_USE_REMANENCE, 0))
			skipClass = "remanence";
		else if (!isSlave && cid.a() == CLASSID_PARTA_NEL_FLARE)
			skipClass = "flare";
		else if (!isSlave && hasWaterMaterial(node))
			skipClass = "water";
		else if (!isSlave)
		{
			for (uint m = 0; m < mods.size(); ++m)
			{
				NLMISC::CClassId mcid = mods[m]->classDesc()->classId();
				TSClassId mscid = mods[m]->classDesc()->superClassId();
				if ((mcid == SCENELIB::CLASSID_PHYSIQUE && mscid == SCLASS_OSMODIFIER)
					|| PHYSIQUESKIN::isPhysiqueModifier(mods[m]))
					hasPhysique = true;
				else if (PHYSIQUESKIN::isSkinModifier(mods[m]) || mcid == SCENELIB::CLASSID_SKIN)
				{
					skipClass = "skinned-skin";
					break;
				}
			}
		}
		else
		{
			for (uint m = 0; m < mods.size(); ++m)
				if (PHYSIQUESKIN::isPhysiqueModifier(mods[m]))
				{
					hasPhysique = true;
					break;
				}
		}
		if (skipClass)
		{
			extras->setString("nel_skip_class", skipClass);
			stats.skip(skipClass);
			fprintf(stderr, "SKIP gltf '%s': %s not carried yet\n", name.c_str(), skipClass);
			continue;
		}

		// Map Extender tag (garbage-UV bucket, design §9)
		for (uint m = 0; m < mods.size(); ++m)
		{
			if (mods[m]->classDesc()->classId() == CLASSID_MAP_EXTENDER)
			{
				printf("MAPEXT %s\n", NLMISC::toLowerAscii(name).c_str());
				extras->setBool("nel_mapext", true);
				break;
			}
		}

		// Base context: the node itself, or the owning parent for a LOD slave
		INode *ctxNode = &node;
		if (isSlave)
		{
			std::map<std::string, INode *>::iterator so = slaveOwner.find(NLMISC::toLowerAscii(name));
			if (so == slaveOwner.end())
			{
				stats.skip("slave-orphan");
				continue;
			}
			ctxNode = so->second;
		}
		SBaseCtx *ctx = baseCtxFor(ctxNode, tmCache, exportLighting, b, ctxCache);
		if (!ctx)
		{
			extras->setString("nel_skip_class", "material");
			stats.skip("material");
			continue;
		}

		uint lodCount = isSlave ? 0 : (uint)getScriptAppDataInt(n, NEL3D_APPDATA_LOD_NAME_COUNT, 0);

		// Mesh evaluation + CMeshBuild (identical calls to the direct route's evalAndBuildMesh).
		// A failed eval or encode tags the node but does NOT abandon it: the direct route drops a
		// failed slot / standalone shape the same way, but a multilod PARENT still exports from
		// its surviving slave slots — so the lod extras and nel_shape below must emit regardless.
		// Exception: a failed Physique decode on the node itself kills the whole shape on the
		// direct route (only a failed SLAVE decode merely drops its slot) — shapeSuppressed
		// mirrors that.
		bool haveMesh = false;
		bool shapeSuppressed = false;
		{
			SEvalMesh mesh;
			std::vector<std::string> warnings;
			if (!evalNodeMesh(node, mesh, &warnings))
			{
				extras->setString("nel_skip_class", "mesh-eval");
				stats.skip("mesh-eval");
			}
			else
			{
				NL3D::CMesh::CMeshBuild mb;
				buildMeshInterface(mesh, mb, ctx->Bbm, ctx->MaxBB, node, tmCache, hasPhysique);
				std::string err;
				bool physOk = true;
				if (hasPhysique)
				{
					if (!PHYSIQUESKIN::applyPhysiqueSkinning(mb, node, mods, modApps, ssc, &err))
					{
						extras->setString("nel_skip_class", "skinned-physique");
						stats.skip("skinned-physique");
						fprintf(stderr, "SKIP gltf '%s': Physique skinning failed: %s\n",
						        name.c_str(), err.c_str());
						physOk = false;
						if (!isSlave)
							shapeSuppressed = true;
					}
					else
						mb.VertexFlags |= NL3D::CVertexBuffer::PaletteSkinFlag;
				}
				if (physOk)
				{
					if (IFACEBUILD::useInterfaceMesh(node))
					{
						// Identity for skinned meshes — their VBs are already world space
						// (direct route's interfaceToWorldMat skinned case)
						IFACEBUILD::applyInterfaceToMeshBuild(node, mb,
							hasPhysique ? NLMISC::CMatrix::Identity : interfaceToWorldMat(node, tmCache),
							tmCache);
						extras->setBool("nel_interface", true);
					}

					// MRM morph targets — the direct route consumes bsList only in the
					// single-mesh wantMrm branch (multilod slots and plain meshes never do)
					std::vector<NL3D::CMesh::CMeshBuild *> bsList;
					if (!isSlave && lodCount == 0
						&& getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0))
						buildBSList(node, tmCache, mods, mb, hasPhysique, exportLighting, bsList);

					sint meshIdx = b.addMesh(name, mb, ctx->GltfMats, &err,
					                         bsList.empty() ? NULL : &bsList);
					for (uint bi = 0; bi < bsList.size(); ++bi)
						delete bsList[bi];
					if (meshIdx < 0)
					{
						extras->setString("nel_skip_class", "encode");
						stats.skip("encode");
						fprintf(stderr, "SKIP gltf '%s': %s\n", name.c_str(), err.c_str());
					}
					else
					{
						b.attachMesh(nodeIdx[&node], meshIdx);
						++stats.Meshes;
						haveMesh = true;
					}
				}
			}
		}

		// Per-node NeL data the shape build consumes downstream
		extras->setBool("nel_cast_shadows", ctx->Bbm.bCastShadows);
		extras->setBool("nel_rcv_shadows", ctx->Bbm.bRcvShadows);
		extras->setBool("nel_lighting_local_atten", ctx->Bbm.UseLightingLocalAttenuation);
		extras->setInt("nel_collision_mesh_gen", (sint64)ctx->Bbm.CollisionMeshGeneration);
		if (ctx->HasLightMap)
			extras->setBool("nel_lightmap", true);
		if (!ctx->Bbm.BSNames.empty() && ctxNode == &node)
		{
			extras->setBool("nel_has_morpher", true);
			CJsonValue *bsn = extras->setArray("nel_bs_names");
			for (uint k = 0; k < ctx->Bbm.BSNames.size(); ++k)
				bsn->pushString(ctx->Bbm.BSNames[k]);
			CJsonValue *bsf = extras->setArray("nel_bs_factors");
			for (uint k = 0; k < ctx->Bbm.DefaultBSFactors.size(); ++k)
				bsf->pushDouble(ctx->Bbm.DefaultBSFactors[k]);
		}

		// LOD slot data (used whether this node is a multilod LOD0, a plain mesh, or a slave)
		extras->setDouble("nel_lod_dist_max", getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_DIST_MAX, 1000.f));
		extras->setDouble("nel_lod_blend_length", getScriptAppDataFloat(n, NEL3D_APPDATA_LOD_BLEND_LENGTH, 0.f));
		if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_BLEND_IN, "") == "1")
			extras->setBool("nel_lod_blend_in", true);
		if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_BLEND_OUT, "") == "1")
			extras->setBool("nel_lod_blend_out", true);
		if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_COARSE_MESH, "") == "1")
			extras->setBool("nel_lod_coarse_mesh", true);
		if (getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0))
		{
			extras->setBool("nel_lod_mrm", true);
			NL3D::CMRMParameters params;
			buildMRMParameters(n, params);
			extras->setInt("nel_mrm_nlods", params.NLods);
			extras->setInt("nel_mrm_divisor", params.Divisor);
			extras->setInt("nel_mrm_skin_reduction", (sint64)params.SkinReduction);
			extras->setDouble("nel_mrm_dist_finest", params.DistanceFinest);
			extras->setDouble("nel_mrm_dist_middle", params.DistanceMiddle);
			extras->setDouble("nel_mrm_dist_coarsest", params.DistanceCoarsest);
		}

		if (isSlave)
		{
			extras->setBool("nel_lod_slave", true);
			continue;
		}

		// No standalone shape from a meshless node (direct parity: buildShapeForNode returns
		// NULL when the eval fails) — unless it is a multilod parent, whose shape can still
		// assemble from the slave slots alone (not after a Physique-decode failure, which kills
		// the whole shape on the direct route).
		if (!haveMesh && (lodCount == 0 || shapeSuppressed))
			continue;

		// Exportable shape node
		extras->setBool("nel_shape", true);
		extras->setString("nel_shape_name", NLMISC::toLowerAscii(name));

		bool haveCoarse = false;
		if (lodCount > 0)
		{
			extras->setInt("nel_lod_count", lodCount);
			for (uint l = 0; l < lodCount && l < 10; ++l)
			{
				std::string nm = getScriptAppDataStr(n, NEL3D_APPDATA_LOD_NAME + l, "");
				char keyBuf[32];
				snprintf(keyBuf, sizeof(keyBuf), "nel_lod_name_%u", l);
				extras->setString(keyBuf, nm);
				if (!nm.empty())
				{
					std::map<std::string, INode *>::iterator lodIt = nodesByName.find(NLMISC::toLowerAscii(nm));
					if (lodIt != nodesByName.end()
						&& getScriptAppDataStr(dynamic_cast<CNodeImpl *>(lodIt->second), NEL3D_APPDATA_LOD_COARSE_MESH, "") == "1")
						haveCoarse = true;
				}
			}
			if (getScriptAppDataStr(n, NEL3D_APPDATA_LOD_DYNAMIC_MESH, "") == "1")
				extras->setBool("nel_lod_dynamic_mesh", true);
		}
		extras->setBool("nel_coarse", haveCoarse);
		if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0))
			extras->setBool("nel_animated_materials", true);
		if (getScriptAppDataInt(n, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0))
			extras->setBool("nel_auto_anim", true);
	}

	// Instance groups: the ig process's full flow over this file (selection order, XRef
	// resolution, clusters/portals/lights — pipeline_max_export_ig's code compiled in), each
	// built CInstanceGroup serialized verbatim into the asset-level nel_igs blob list. The
	// structural per-node nel_ig_name tags above are the artist-visible view; the blob is the
	// byte-exact carrier (dual representation, same rule as animation tracks).
	{
		std::vector<std::pair<std::string, std::vector<uint8> > > igs;
		int nIgs = pmbExportIgsForGltf(maxPath, igs);
		if (nIgs > 0)
		{
			CJsonValue *jigs = b.assetExtras()->setArray("nel_igs");
			for (size_t i = 0; i < igs.size(); ++i)
			{
				CJsonValue *e = jigs->push();
				e->setString("name", igs[i].first);
				std::string hex;
				hex.reserve(igs[i].second.size() * 2);
				char buf[4];
				for (size_t k = 0; k < igs[i].second.size(); ++k)
				{
					snprintf(buf, sizeof(buf), "%02x", igs[i].second[k]);
					hex += buf;
				}
				e->set("data")->setString(hex);
			}
			stats.Igs = (uint)igs.size();
		}
	}

	if (!b.save(outPath))
	{
		fprintf(stderr, "ERROR: cannot write %s\n", outPath.c_str());
		return 1;
	}
	if (g_verbose)
		printf("OK %s\n", outPath.c_str());
	return 0;
}

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext appContext;
	NL3D::registerSerial3d();

	std::string input, outDir;
	bool exportLighting = true;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose" || arg == "-v")
			g_verbose = true;
		else if (arg == "--db" && i + 1 < argc)
			setDatabaseRoot(argv[++i]);
		else if (arg == "--path-alias" && i + 1 < argc)
		{
			std::string kv = argv[++i];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n", kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
		}
		else if (arg == "--ps-path" && i + 1 < argc)
			pmbIgAddPsSearchPath(argv[++i]);
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
		fprintf(stderr, "usage: pipeline_max_export_gltf [--db <graphics-root>] [--no-lighting] [-v] <input.max> <outdir>\n");
		return 2;
	}

	// Deduce the database root from the input path when not given (same as the shape exporter)
	if (databaseRoot().empty())
	{
		std::string abs = NLMISC::CPath::getFullPath(input, false);
		std::string::size_type p = abs.find("/stuff/");
		if (p == std::string::npos) p = abs.find("/landscape/");
		if (p == std::string::npos) p = abs.find("/sky_v2/");
		if (p != std::string::npos)
			setDatabaseRoot(abs.substr(0, p));
	}

	std::string stem = NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(input));
	std::string outPath = outDir + "/" + stem + ".gltf";

	SExportStats stats;
	int ret = exportFile(input, outPath, exportLighting, stats);
	printf("GLTF %s (%u meshes, %u igs, %u skipped)\n", outPath.c_str(), stats.Meshes, stats.Igs,
	       stats.Skipped);
	for (std::map<std::string, uint>::iterator it = stats.SkipReasons.begin(); it != stats.SkipReasons.end(); ++it)
		printf("SKIPCLASS %s %u\n", it->first.c_str(), it->second);
	return ret;
}

/* end of file */
