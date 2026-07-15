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
#include <nel/3d/animation.h>
#include <nel/3d/u_track.h>
#include <nel/misc/mem_stream.h>

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
#include "../pipeline_max_export_shape/water_build.h"
#include "../pipeline_max_export_shape/remanence_build.h"
#include "../pipeline_max_export_shape/flare_build.h"
#include "../pipeline_max_export_common/physique_skin.h"
#include "../pipeline_max_export_common/biped_rig.h"
#include "../pipeline_max_export_common/export_ids.h"
#include "../pipeline_max_export_zone/pmb_zone_gltf.h"

#include <nel/3d/shape.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/bezier_patch.h>

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"

#include "gltf_build.h"
#include "../nel_gltf/gltf_special_shape.h"
#include "../nel_gltf/shape_export_bytes.h"
#include "../nel_gltf/hex_blob.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;
using namespace MESHBUILD;
using namespace GLTFBUILD;
using namespace NLGLTF;
using namespace PMAX_RIG;

// NeL export AppData sub-ids + special-object class ids: pipeline_max_export_common/export_ids.h
using PMAX_EXPORT_IDS::CLASSID_PACS_BOX;
using PMAX_EXPORT_IDS::CLASSID_PACS_CYL;
using PMAX_EXPORT_IDS::CLASSID_MAP_EXTENDER;

static bool g_verbose = false;
static std::string g_zoneBank;
static float g_zoneCellSize = 160.0f;
static float g_zoneSnap = 1.0f;

// The per-process flows compiled in with their PMB_*_NO_MAIN guarded mains. Every flow
// receives the SAME already-parsed scene (the writer parses each .max exactly once — the
// pipeline_max parse-once design); the standalone tools load via PMAXLOAD::loadMaxFile and
// call the identical flow.

// From ../pipeline_max_export_ig/main.cpp: the ig process's full selection +
// buildInstanceGroup flow, returning each ig's serialized bytes.
int pmbExportIgsForGltf(PMAXLOAD::SLoadedMax &lm,
                        std::vector<std::pair<std::string, std::vector<uint8> > > &igsOut);
void pmbIgAddPsSearchPath(const std::string &path);

// From ../pipeline_max_export_anim/main.cpp: the anim process's whole-file flow, returning the
// serialized CAnimation (1 = produced, 3 = nothing). bareNodesOut: selection nodes exported
// with an EMPTY prefix — they own the bare "pos"/"rotquat"/"scale"/"<target>MorphFactor" track
// names (Ryzom per-node convention).
int pmbExportAnimForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                         std::vector<uint8> &animOut,
                         std::vector<std::string> *bareNodesOut = NULL);

// From ../pipeline_max_export_swt/main.cpp and ../pipeline_max_export_pacs_prim/main.cpp:
// single-output whole-file flows — 1 = produced, 3 = nothing to export, -1 = error.
int pmbExportSwtForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                        std::vector<uint8> &out);
int pmbExportPacsPrimForGltf(PMAXLOAD::SLoadedMax &lm, std::vector<uint8> &out);

// From ../pipeline_max_export_veget/main.cpp: the veget process's whole-file flow, one
// (node name, .veget bytes) pair per vegetable node.
int pmbExportVegetsForGltf(PMAXLOAD::SLoadedMax &lm, bool exportLighting,
                           std::vector<std::pair<std::string, std::vector<uint8> > > &out,
                           uint &skipped);

// From ../pipeline_max_export_clod/main.cpp: the clod process's whole-file flow, one
// (node name, .clod bytes) pair per character-lod node.
int pmbExportClodsForGltf(PMAXLOAD::SLoadedMax &lm, bool exportLighting,
                          std::vector<std::pair<std::string, std::vector<uint8> > > &out,
                          uint &skipped);

// From ../pipeline_max_export_cmb/main.cpp: the cmb process's whole-file flow, one
// (igname, .cmb bytes) pair per collision group. ligoMode follows the zone filename protocol
// (zonematerial-/zonetransition-/zonespecial- sources take the --ligo path with XRef collision
// resolution, like the build pipeline invokes it).
int pmbExportCmbsForGltf(const std::string &inputPath, PMAXLOAD::SLoadedMax &lm, bool ligoMode,
                         std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut);

// From ../pipeline_max_export_skel/main.cpp: the skel process's Bip01-rooted whole-file flow —
// 1 = produced, 3 = no Bip01, -1 = error.
int pmbExportSkelForGltf(PMAXLOAD::SLoadedMax &lm, std::vector<uint8> &out);

// From ../pipeline_max_export_shape/main.cpp (PMB_SHAPE_NO_MAIN): the shape exporter's whole
// per-file flow in lightmap-scene-only mode — the .lmscene bytes a direct `--lm-scene` run
// writes (1 = produced, 3 = no lightmap receivers, -1 = error).
int pmbExportLmSceneForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                            bool exportLighting,
                            std::string &nameOut, std::vector<uint8> &out);

struct SExportStats
{
	uint Meshes;
	uint Skipped;
	uint Igs;
	uint Anims;
	uint Specials;
	uint Zones;
	uint Others; // single-output process blobs (swt, pacs_prim, ...)
	std::map<std::string, uint> SkipReasons;
	SExportStats() : Meshes(0), Skipped(0), Igs(0), Anims(0), Specials(0), Zones(0), Others(0) { }
	void skip(const std::string &reason)
	{
		++Skipped;
		++SkipReasons[reason];
	}
};

// Asset-level blob embed: every byte-exact process output rides asset.extras as
// { "name": <output stem>, "data": <hex> } — one object for single-output processes, a list of
// them for per-node/per-group processes.
static void embedBlob(CGltfBuilder &b, const char *key, const std::string &name,
                      const std::vector<uint8> &bytes)
{
	CJsonValue *js = b.assetExtras()->setObject(key);
	js->setString("name", name);
	js->set("data")->setString(bytesToHex(bytes));
}

static void embedBlobList(CGltfBuilder &b, const char *key,
                          const std::vector<std::pair<std::string, std::vector<uint8> > > &files)
{
	CJsonValue *jv = b.assetExtras()->setArray(key);
	for (size_t i = 0; i < files.size(); ++i)
	{
		CJsonValue *e = jv->push();
		e->setString("name", files[i].first);
		e->set("data")->setString(bytesToHex(files[i].second));
	}
}

// isGeometryOrShape / rootOf / startsWithBip (scene_lib) and interfaceToWorldMat
// (interface_build) are shared with the direct shape exporter — the selection gate and the
// weld-space derivation must never drift between the routes.

// Per-node glTF TRS + composed world matrix (pass 1). Biped rig nodes decode through the skel
// exporter's figure-mode reconstruction (biped TM controllers are not PRS — the plain
// getLocalMatrix path leaves the whole rig at identity, useless as a glTF rest pose): COM nodes
// from the rig's COM record, biped bones via getBipedLocal, and the PRS-child-of-COM
// world-frame-rotation rule replayed from walkNode (biped_rig.cpp). Every other node keeps the
// established decompMatrix(getLocalMatrix(...)) path unchanged. Viewing tier only — the exact
// tier's mesh data never reads node TRS; the composed worlds feed the glTF skins' inverse bind
// matrices, so viewers (which compose exactly these floats) cancel at bind pose.
struct SNodeTRS
{
	NLMISC::CVector Pos, Scale;
	NLMISC::CQuat Rot;
	NLMISC::CMatrix World;
	std::string Name; // duplicate scene names get the skeleton walk's "_Second" suffix — glTF
	                  // node names stay unique (assimp refuses duplicate bone names) and match
	                  // nel_bones_names exactly
};

static void computeNodeTRS(INode *node, const NLMISC::CMatrix &parentWorld,
                           const std::map<INode *, std::vector<INode *> > &childrenOf,
                           SNodeTMCache &tmCache, CSceneClassContainer *ssc,
                           std::map<INode *, SNodeTRS> &out, std::set<std::string> &nameSet)
{
	SNodeTRS t;
	t.Name = nodeName(*node);
	if (!nameSet.insert(t.Name).second)
		t.Name += "_Second";
	CReferenceMaker *tmCtrl = node->getReference(0);
	CSceneClass *bipedSys = bipedSystemOfCtrl(tmCtrl);
	if (bipedSys && isBipedComNode(node) && rigFor(bipedSys, ssc).HasCom)
	{
		SBipedRig &rig = rigFor(bipedSys, ssc);
		g_rig = &rig;
		t.Scale = NLMISC::CVector(1, 1, 1);
		NLMISC::CQuat pinv = parentWorld.getRot();
		pinv.invert();
		t.Rot = pinv * rig.ComRot;
		NLMISC::CMatrix pinvM = parentWorld;
		pinvM.invert();
		t.Pos = pinvM * rig.ComPos;
		t.Rot.normalize();
	}
	else if (bipedSys && isBipedBoneNode(node))
	{
		SBipedRig &rig = rigFor(bipedSys, ssc);
		g_rig = &rig;
		NLMISC::CQuat worldRotOut;
		getBipedLocal(node, parentWorld, t.Pos, t.Rot, t.Scale, worldRotOut);
		t.Rot.normalize();
	}
	else if (node->parent() && isBipedComNode(node->parent()))
	{
		// PRS child of a COM node: stored rotation is world-frame, position parent-relative
		// (walkNode's rule, verified bit-exact on the corpus marker bones)
		getLocalTransform(tmCtrl, t.Pos, t.Rot, t.Scale);
		NLMISC::CQuat pinv = parentWorld.getRot();
		pinv.invert();
		t.Rot = pinv * t.Rot;
		t.Rot.normalize();
	}
	else
	{
		MAXSCENE::decompMatrix(t.Scale, t.Rot, t.Pos, getLocalMatrix(*node, tmCache));
	}
	t.World = parentWorld * makeLocalTM(t.Pos, t.Rot, t.Scale);
	out[node] = t;

	std::map<INode *, std::vector<INode *> >::const_iterator ci = childrenOf.find(node);
	if (ci != childrenOf.end())
		for (size_t i = 0; i < ci->second.size(); ++i)
			computeNodeTRS(ci->second[i], t.World, childrenOf, tmCache, ssc, out, nameSet);
}

// Morph-carrying mesh node record (viewing tier): feeds the sampled "weights" animation
// channels — track names are "<raw node name>.<target name>MorphFactor".
struct SMorphMeshNode
{
	std::string RawName;
	sint Node;
	std::vector<std::string> Names;
	std::vector<float> Defaults; // 0..1 (NeL percents / 100)
};

// buildBSList (mesh_build, shared with the direct route): Morpher blend-shape targets,
// consumed only by the single-mesh MRM branch exactly like the direct route.

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

	// Pass 1: every scene node becomes a glTF node (bit-exact TRS extras). TRS + world matrices
	// come from a hierarchy walk so biped rig nodes get their figure-mode rest pose — see
	// computeNodeTRS.
	std::map<INode *, std::vector<INode *> > childrenNodes;
	std::vector<INode *> rootNodes;
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode *p = allNodes[i]->parent();
		if (p && dynamic_cast<CNodeImpl *>(p))
			childrenNodes[p].push_back(allNodes[i]);
		else
			rootNodes.push_back(allNodes[i]);
	}
	std::map<INode *, SNodeTRS> nodeTRS;
	{
		NLMISC::CMatrix ident;
		ident.identity();
		std::set<std::string> nameSet;
		for (uint i = 0; i < rootNodes.size(); ++i)
			computeNodeTRS(rootNodes[i], ident, childrenNodes, tmCache, ssc, nodeTRS, nameSet);
	}
	std::map<INode *, sint> nodeIdx;
	for (uint i = 0; i < allNodes.size(); ++i)
	{
		INode &node = *allNodes[i];
		const SNodeTRS &t = nodeTRS[&node];
		nodeIdx[&node] = b.addNode(t.Name, t.Pos, t.Rot, t.Scale);
	}
	std::vector<sint> roots; // scene roots — proxy nodes append later, setSceneRoots at the end
	{
		for (uint i = 0; i < rootNodes.size(); ++i)
			roots.push_back(nodeIdx[rootNodes[i]]);
		for (std::map<INode *, std::vector<INode *> >::iterator it = childrenNodes.begin();
		     it != childrenNodes.end(); ++it)
		{
			std::vector<sint> ch(it->second.size());
			for (size_t c = 0; c < it->second.size(); ++c)
				ch[c] = nodeIdx[it->second[c]];
			b.setNodeChildren(nodeIdx[it->first], ch);
		}
	}

	// Pass 2: meshes + per-node NeL appdata on the shape-process node selection
	TBaseCtxCache ctxCache;
	std::vector<SMorphMeshNode> morphNodes;
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

		// The shared standalone-node selection gate (scene_lib — the direct route's exportFile
		// gate). LOD slaves bypass it entirely: the direct multilod branch resolves slaves by
		// name and takes them straight to the mesh eval — no bip/classid/appdata checks on
		// slave nodes.
		if (!isSlave && !shapeProcessSelectsNode(node, cid))
			continue;

		// Special shape classes — same dispatch order as buildShapeForNode. Water/remanence/
		// flare build through the SAME class builders the direct route uses; the built shape
		// serializes (same version patches) into a per-node nel_shape_blob — dual
		// representation, structural extras when Blender authoring needs them. A NULL builder
		// result tags the node like the direct route's skip. Physique is CARRIED (skinning
		// applies onto the mesh build below); the Max-4+ Skin modifier skips the whole node
		// exactly like the direct route does. The direct multilod slave loop re-detects
		// Physique ONLY — mirror that asymmetry.
		const char *skipClass = NULL;
		bool hasPhysique = false;
		NL3D::IShape *specialShape = NULL;
		const char *specialClass = NULL;
		if (!isSlave && cid.a() == CLASSID_PARTA_NEL_WAVE_MAKER)
			skipClass = "wavemaker";
		else if (!isSlave && getScriptAppDataInt(n, NEL3D_APPDATA_USE_REMANENCE, 0))
		{
			specialShape = REMANENCEBUILD::buildRemanenceShape(node, tmCache, exportLighting);
			if (specialShape) specialClass = "remanence"; else skipClass = "remanence";
		}
		else if (!isSlave && cid.a() == CLASSID_PARTA_NEL_FLARE)
		{
			specialShape = FLAREBUILD::buildFlareShape(node, tmCache);
			if (specialShape) specialClass = "flare"; else skipClass = "flare";
		}
		else if (!isSlave && hasWaterMaterial(node))
		{
			specialShape = WATERBUILD::buildWaterShape(node, tmCache);
			if (specialShape) specialClass = "water"; else skipClass = "water";
		}
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
			fprintf(stderr, "SKIP gltf '%s': %s not carried\n", name.c_str(), skipClass);
			continue;
		}
		if (specialShape)
		{
			std::vector<uint8> bytes;
			std::string serr;
			bool ok = shapeToExportFileBytes(specialShape, bytes, &serr);
			if (!ok || bytes.empty())
			{
				delete specialShape;
				extras->setString("nel_skip_class", "shape-serial");
				stats.skip("shape-serial");
				fprintf(stderr, "SKIP gltf '%s': %s\n", name.c_str(), serr.c_str());
				continue;
			}
			extras->setBool("nel_shape", true);
			extras->setString("nel_shape_name", NLMISC::toLowerAscii(name));
			extras->setString("nel_shape_class", specialClass);

			// Structural extras (full parity: every builder-set field as artist-visible keys),
			// self-checked by construction — rebuild from the extras and byte-compare against
			// the directly-built shape. On match the structural form is the carrier; any gap
			// falls back to the opaque blob with a warning, never silent loss.
			bool structuralOk = false;
			{
				CJsonValue structural(CJsonValue::Object);
				std::string cls, cerr;
				if (specialShapeToExtras(specialShape, structural, &cls, &cerr) && cls == specialClass)
				{
					NL3D::IShape *re = specialShapeFromExtras(structural, cls, &cerr);
					if (re)
					{
						std::vector<uint8> rebytes;
						structuralOk = shapeToExportFileBytes(re, rebytes, &cerr) && rebytes == bytes;
						if (!structuralOk && cerr.empty())
							cerr = "rebuilt shape bytes differ";
						delete re;
					}
				}
				if (structuralOk)
				{
					// decompose is deterministic — emit the verified extras straight onto the node
					std::string cls2, tmp;
					specialShapeToExtras(specialShape, *extras, &cls2, &tmp);
				}
				else
					fprintf(stderr, "WARNING gltf '%s': structural %s extras self-check failed (%s); carrying blob\n",
					        name.c_str(), specialClass, cerr.c_str());
			}
			delete specialShape;
			if (!structuralOk)
				extras->setString("nel_shape_blob", bytesToHex(bytes));
			++stats.Specials;
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
				std::vector<INode *> skinJointNodes; // BonesNames order (glTF skins tier)
				if (hasPhysique)
				{
					if (!PHYSIQUESKIN::applyPhysiqueSkinning(mb, node, mods, modApps, ssc, &err,
							&skinJointNodes))
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
						IFACEBUILD::applyInterfaceToMeshBuild(node, mb,
							IFACEBUILD::interfaceToWorldMat(node, tmCache, hasPhysique),
							tmCache);
						extras->setBool("nel_interface", true);
					}

					// MRM morph targets — the direct route consumes bsList only in the
					// single-mesh wantMrm branch (multilod slots and plain meshes never do)
					std::vector<NL3D::CMesh::CMeshBuild *> bsList;
					if (!isSlave && lodCount == 0
						&& getScriptAppDataInt(n, NEL3D_APPDATA_LOD_MRM, 0))
						buildBSList(node, tmCache, mods, mb, hasPhysique, exportLighting, bsList);

					// glTF-skins viewing tier: the joints are the exact scene nodes behind
					// BonesNames (from the physique decode itself — duplicate-name-safe),
					// all-or-nothing. Failure only drops the viewing skin, never the exact-tier
					// nel_skin_* emission.
					bool skinInterop = false;
					std::vector<sint> skinJoints;
					if (((uint32)mb.VertexFlags & NL3D::CVertexBuffer::PaletteSkinFlag)
						&& skinJointNodes.size() == mb.BonesNames.size()
						&& !skinJointNodes.empty())
					{
						skinInterop = true;
						for (uint bn = 0; bn < skinJointNodes.size(); ++bn)
						{
							if (!skinJointNodes[bn] || !nodeIdx.count(skinJointNodes[bn]))
							{
								fprintf(stderr, "WARNING gltf '%s': skin bone '%s' not in scene; no glTF skin emitted\n",
								        name.c_str(), mb.BonesNames[bn].c_str());
								skinInterop = false;
								break;
							}
							skinJoints.push_back(nodeIdx[skinJointNodes[bn]]);
						}
					}

					sint meshIdx = b.addMesh(name, mb, ctx->GltfMats, &err,
					                         bsList.empty() ? NULL : &bsList, &skinInterop);
					size_t bsCount = bsList.size();
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
						if (skinInterop)
						{
							std::vector<float> ibms(skinJoints.size() * 16);
							for (size_t ji = 0; ji < skinJointNodes.size(); ++ji)
							{
								NLMISC::CMatrix w = nodeTRS[skinJointNodes[ji]].World;
								w.invert();
								w.get(&ibms[ji * 16]);
							}
							b.setNodeSkin(nodeIdx[&node], b.addSkin(skinJoints, &ibms[0]));
						}
						// Morph meta (viewing tier): mesh.weights defaults + targetNames; record
						// the node for sampled "weights" channels (BSNames parallels bsList)
						if (bsCount && ctx->Bbm.BSNames.size() == bsCount)
						{
							SMorphMeshNode mn;
							mn.RawName = name;
							mn.Node = nodeIdx[&node];
							mn.Names = ctx->Bbm.BSNames;
							mn.Defaults.resize(bsCount);
							for (size_t k = 0; k < bsCount; ++k)
								mn.Defaults[k] = (k < ctx->Bbm.DefaultBSFactors.size()
									? ctx->Bbm.DefaultBSFactors[k] : 0.0f) / 100.0f;
							b.setMeshMorphMeta(meshIdx, mn.Names, mn.Defaults);
							morphNodes.push_back(mn);
						}
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
		int nIgs = pmbExportIgsForGltf(lm, igs);
		if (nIgs > 0)
		{
			embedBlobList(b, "nel_igs", igs);
			stats.Igs = (uint)igs.size();
		}
	}

	// Animation: the anim process's whole-file flow ($Bip01-first + EXPORT_NODE_ANIMATION
	// selection, biped COM decode, track build — pipeline_max_export_anim's code compiled in),
	// the built CAnimation serialized verbatim into the asset-level nel_anim blob. Dual
	// representation per the plan: sampled glTF animation channels are a later additive interop
	// tier; the blob is the byte-exact carrier the corpus gates on.
	{
		std::vector<uint8> anim;
		std::vector<std::string> bareNodes;
		if (pmbExportAnimForGltf(maxPath, lm, anim, &bareNodes) == 1 && !anim.empty())
		{
			std::string animName = NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath));
			embedBlob(b, "nel_anim", animName, anim);
			stats.Anims = 1;

			// Sampled glTF animation channels (viewing tier): the blob deserialized back into a
			// CAnimation, TRS tracks LINEAR-sampled at 30 fps onto the nodes whose names match
			// the track prefixes (rig rest pose landed with the skins tier). Lossy view — DCC
			// edits do not flow back; the blob stays authoritative. Non-TRS tracks (materials,
			// morph factors, note tracks) are not sampled yet.
			try
			{
				NLMISC::CMemStream ms(true);
				ms.fill(&anim[0], (uint32)anim.size());
				NL3D::CAnimation na;
				na.serial(ms);
				std::map<std::string, sint> byName;
				for (uint i = 0; i < allNodes.size(); ++i)
					byName[nodeTRS[allNodes[i]].Name] = nodeIdx[allNodes[i]];
				std::set<std::string> trackNames;
				na.getTrackNames(trackNames);
				const float fps = 30.0f;
				// Bare track names ("pos", not "Node.pos") belong to the selection node exported
				// with an empty prefix — the Ryzom per-node convention.
				std::string bareRoot = bareNodes.empty() ? std::string() : bareNodes[0];
				for (std::set<std::string>::const_iterator tn = trackNames.begin(); tn != trackNames.end(); ++tn)
				{
					std::string::size_type dot = tn->rfind('.');
					std::string chan = (dot == std::string::npos) ? *tn : tn->substr(dot + 1);
					std::string target = (dot == std::string::npos) ? bareRoot : tn->substr(0, dot);
					const char *path;
					int nComp;
					if (chan == "pos") { path = "translation"; nComp = 3; }
					else if (chan == "rotquat") { path = "rotation"; nComp = 4; }
					else if (chan == "scale") { path = "scale"; nComp = 3; }
					else continue;
					std::map<std::string, sint>::iterator ni = byName.find(target);
					if (target.empty() || ni == byName.end())
						continue;
					NL3D::UTrack *tr = na.getTrackByName(tn->c_str());
					if (!tr)
						continue;
					float t0 = tr->getBeginTime(), t1 = tr->getEndTime();
					uint n = (t1 > t0) ? (uint)((t1 - t0) * fps + 0.5f) + 1 : 1;
					std::vector<float> times(n), values;
					values.reserve((size_t)n * nComp);
					NLMISC::CQuat prevQ;
					bool ok = true, havePrev = false;
					for (uint s = 0; s < n && ok; ++s)
					{
						float t = (s + 1 == n) ? t1 : t0 + (float)s / fps;
						times[s] = t;
						if (nComp == 4)
						{
							NLMISC::CQuat q;
							ok = tr->interpolate(t, q);
							// hemisphere-align consecutive samples so viewers' slerp stays short
							if (havePrev && (q.x * prevQ.x + q.y * prevQ.y + q.z * prevQ.z + q.w * prevQ.w) < 0.0f)
							{
								q.x = -q.x; q.y = -q.y; q.z = -q.z; q.w = -q.w;
							}
							prevQ = q;
							havePrev = true;
							values.push_back(q.x);
							values.push_back(q.y);
							values.push_back(q.z);
							values.push_back(q.w);
						}
						else
						{
							NLMISC::CVector v;
							ok = tr->interpolate(t, v);
							values.push_back(v.x);
							values.push_back(v.y);
							values.push_back(v.z);
						}
					}
					if (!ok || times.empty())
						continue;
					b.addAnimChannel(animName, ni->second, path, times, values, nComp);
				}

				// Morph factor tracks -> "weights" channels (percent tracks / 100; targets
				// without a track hold their default). Track name replicates the anim
				// exporter's addMorphTracks: "<raw node name>.<target name>MorphFactor".
				for (size_t mi2 = 0; mi2 < morphNodes.size(); ++mi2)
				{
					const SMorphMeshNode &mn = morphNodes[mi2];
					std::vector<NL3D::UTrack *> ftracks(mn.Names.size(), (NL3D::UTrack *)NULL);
					float t0 = 0.0f, t1 = 0.0f;
					bool any = false;
					for (size_t k = 0; k < mn.Names.size(); ++k)
					{
						NL3D::UTrack *tr = na.getTrackByName((mn.RawName + "." + mn.Names[k] + "MorphFactor").c_str());
						if (!tr && mn.RawName == bareRoot)
							tr = na.getTrackByName((mn.Names[k] + "MorphFactor").c_str());
						if (!tr)
							continue;
						ftracks[k] = tr;
						if (!any) { t0 = tr->getBeginTime(); t1 = tr->getEndTime(); any = true; }
						else
						{
							t0 = std::min(t0, tr->getBeginTime());
							t1 = std::max(t1, tr->getEndTime());
						}
					}
					if (!any)
						continue;
					uint n = (t1 > t0) ? (uint)((t1 - t0) * fps + 0.5f) + 1 : 1;
					std::vector<float> times(n), weights;
					weights.reserve((size_t)n * mn.Names.size());
					bool ok = true;
					for (uint s = 0; s < n && ok; ++s)
					{
						float t = (s + 1 == n) ? t1 : t0 + (float)s / fps;
						times[s] = t;
						for (size_t k = 0; k < mn.Names.size(); ++k)
						{
							float w = mn.Defaults[k];
							if (ftracks[k])
							{
								float f = 0.0f;
								ok = ok && ftracks[k]->interpolate(t, f);
								w = f / 100.0f;
							}
							weights.push_back(w);
						}
					}
					if (!ok)
						continue;
					b.addWeightsChannel(animName, mn.Node, times, weights, (int)mn.Names.size());
				}
			}
			catch (const std::exception &e)
			{
				fprintf(stderr, "WARNING: sampled animation channels failed: %s\n", e.what());
			}
		}
	}

	// Skeleton (.skel): the skel process's Bip01-rooted whole-file flow. It clears the
	// pointer-keyed rig caches before its walk (a fresh reconstruction from the shared scene —
	// deterministic, so flows before/after are unaffected; the clear predates the single-parse
	// refactor, when a previous flow's freed scene could alias under allocator reuse).
	{
		std::vector<uint8> bytes;
		if (pmbExportSkelForGltf(lm, bytes) == 1 && !bytes.empty())
		{
			embedBlob(b, "nel_skel", NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath)), bytes);
			++stats.Others;
		}
	}

	// Lightmap scene graph (.lmscene, design doc §11-lm): the 1_export half of the standalone
	// lightmapper, as produced by the direct exporter's --lm-scene run — receivers as pre-build
	// mesh data, occluders, and source-fidelity lights. Rides as an asset blob (the pre-build
	// intermediate is its own versioned NeL contract; the glTF mesh accessors feed the SHAPE
	// build, not the lightmapper's unwrap).
	{
		std::string lmName;
		std::vector<uint8> bytes;
		if (pmbExportLmSceneForGltf(maxPath, lm, exportLighting, lmName, bytes) == 1 && !bytes.empty())
		{
			embedBlob(b, "nel_lmscene", lmName, bytes);
			++stats.Others;
		}
	}

	// Skeleton weight (.swt) and PACS primitives (.pacs_prim): single-output whole-file
	// processes — the shared flows (also the standalone tools' only code path) serialize into
	// asset-level blobs; mesh_export re-emits <stem>.swt / <stem>.pacs_prim. Full-parity rule:
	// every artifact the .max pipeline produces must be producible from the .gltf.
	{
		std::vector<uint8> bytes;
		if (pmbExportSwtForGltf(maxPath, lm, bytes) == 1 && !bytes.empty())
		{
			embedBlob(b, "nel_swt", NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath)), bytes);
			++stats.Others;
		}
	}
	{
		std::vector<uint8> bytes;
		if (pmbExportPacsPrimForGltf(lm, bytes) == 1 && !bytes.empty())
		{
			embedBlob(b, "nel_pacs_prim", NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath)), bytes);
			++stats.Others;
		}
	}

	// Vegetables (.veget): per-node outputs from the veget process's whole-file flow, in the
	// nel_vegets blob list (names as authored — the process writes <node>.veget).
	{
		std::vector<std::pair<std::string, std::vector<uint8> > > vegets;
		uint vskipped = 0;
		if (pmbExportVegetsForGltf(lm, exportLighting, vegets, vskipped) == 0 && !vegets.empty())
		{
			embedBlobList(b, "nel_vegets", vegets);
			stats.Others += (uint)vegets.size();
		}
	}

	// Character LODs (.clod): per-node outputs from the clod process, in the nel_clods list.
	{
		std::vector<std::pair<std::string, std::vector<uint8> > > clods;
		uint cskipped = 0;
		if (pmbExportClodsForGltf(lm, exportLighting, clods, cskipped) == 0 && !clods.empty())
		{
			embedBlobList(b, "nel_clods", clods);
			stats.Others += (uint)clods.size();
		}
	}

	// Collision mesh builds (.cmb): per-igname outputs from the cmb process, in the nel_cmbs
	// list. Ligo brick sources (zone filename protocol) take the --ligo path.
	{
		std::string stem = NLMISC::toLowerAscii(NLMISC::CFile::getFilenameWithoutExtension(maxPath));
		bool ligoCmb = stem.compare(0, 13, "zonematerial-") == 0
			|| stem.compare(0, 15, "zonetransition-") == 0
			|| stem.compare(0, 12, "zonespecial-") == 0;
		std::vector<std::pair<std::string, std::vector<uint8> > > cmbs;
		if (pmbExportCmbsForGltf(maxPath, lm, ligoCmb, cmbs) == 0 && !cmbs.empty())
		{
			embedBlobList(b, "nel_cmbs", cmbs);
			stats.Others += (uint)cmbs.size();
		}
	}

	// Ligo zone sources (zonematerial-*/zonetransition-*/zonespecial-*): the zone process's
	// whole-file flow (pipeline_max_export_zone compiled in with PMB_ZONE_NO_MAIN). Outputs
	// ride verbatim in the nel_zones blob list (authoritative — PatchMesh has no faithful glTF
	// form); the AUTHORED patches additionally tessellate into nel_proxy viewing meshes any
	// glTF consumer renders and the exact-tier importer ignores.
	{
		std::vector<std::pair<std::string, std::vector<uint8> > > zoneFiles;
		std::vector<SPmbZoneProxy> proxies;
		int nZones = pmbExportZonesForGltf(maxPath, lm, g_zoneBank, g_zoneCellSize, g_zoneSnap,
		                                   zoneFiles, &proxies);
		if (nZones < 0)
			fprintf(stderr, "WARNING: ligo zone flow failed for %s (nel_zones not emitted)\n",
			        maxPath.c_str());
		if (nZones > 0)
		{
			embedBlobList(b, "nel_zones", zoneFiles);
			stats.Zones = (uint)zoneFiles.size();
		}
		for (size_t i = 0; i < proxies.size(); ++i)
		{
			const SPmbZoneProxy &pr = proxies[i];
			std::vector<float> pos, norm, uv;
			std::vector<uint32> idx;
			for (size_t p = 0; p < pr.Patches.size(); ++p)
			{
				const NL3D::CPatchInfo &pi = pr.Patches[p];
				const NL3D::CBezierPatch &patch = pi.Patch;
				uint ns = (pi.OrderS ? (uint)pi.OrderS : 1) * 2;
				uint nt = (pi.OrderT ? (uint)pi.OrderT : 1) * 2;
				uint32 base = (uint32)(pos.size() / 3);
				for (uint j = 0; j <= nt; ++j)
					for (uint k = 0; k <= ns; ++k)
					{
						float s = (float)k / (float)ns, t = (float)j / (float)nt;
						NLMISC::CVector v = patch.eval(s, t);
						NLMISC::CVector nv = patch.evalNormal(s, t);
						pos.push_back(v.x); pos.push_back(v.y); pos.push_back(v.z);
						norm.push_back(nv.x); norm.push_back(nv.y); norm.push_back(nv.z);
						uv.push_back(s); uv.push_back(t);
					}
				for (uint j = 0; j < nt; ++j)
					for (uint k = 0; k < ns; ++k)
					{
						uint32 va = base + j * (ns + 1) + k;
						uint32 vb = va + 1;
						uint32 vc = va + (ns + 1);
						uint32 vd = vc + 1;
						idx.push_back(va); idx.push_back(vc); idx.push_back(vb);
						idx.push_back(vb); idx.push_back(vc); idx.push_back(vd);
					}
			}
			sint meshIdx = b.addProxyMesh(pr.NodeName, pos, norm, uv, idx);
			if (meshIdx < 0)
				continue;
			sint nodeIdx2 = b.addNode(pr.NodeName + ".zoneproxy", NLMISC::CVector(0, 0, 0),
			                          NLMISC::CQuat(0, 0, 0, 1), NLMISC::CVector(1, 1, 1));
			b.attachMesh(nodeIdx2, meshIdx);
			CJsonValue *px = b.nodeExtras(nodeIdx2);
			px->setBool("nel_proxy", true);
			if (pr.Frozen)
				px->setBool("nel_proxy_frozen", true);
			roots.push_back(nodeIdx2);
		}
	}

	b.setSceneRoots(roots);

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
		else if (arg == "--zone-bank" && i + 1 < argc)
			g_zoneBank = argv[++i];
		else if (arg == "--zone-cellsize" && i + 1 < argc)
			NLMISC::fromString(argv[++i], g_zoneCellSize);
		else if (arg == "--zone-snap" && i + 1 < argc)
			NLMISC::fromString(argv[++i], g_zoneSnap);
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
	printf("GLTF %s (%u meshes, %u special shapes, %u igs, %u anims, %u zones, %u others, %u skipped)\n", outPath.c_str(),
	       stats.Meshes, stats.Specials, stats.Igs, stats.Anims, stats.Zones, stats.Others, stats.Skipped);
	for (std::map<std::string, uint>::iterator it = stats.SkipReasons.begin(); it != stats.SkipReasons.end(); ++it)
		printf("SKIPCLASS %s %u\n", it->first.c_str(), it->second);
	return ret;
}

/* end of file */
