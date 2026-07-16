/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 */
// Ig export: .max -> .ig, replicating the NelExportInstanceGroup path of the 3ds Max plugin
// (build_gamedata processes/ig) without 3ds Max.
//
// The ig maxscript (processes/ig/maxscript/ig_export.ms) scans every scene object for the
// NEL3D_APPDATA_IGNAME appdata, then per distinct ig name selects the geometry, light and
// helper nodes carrying that name (three passes, in that order) and calls the plugin's
// NelExportInstanceGroup over the selection; the plugin side is CExportNel::buildInstanceGroup
// (plugin_max/nel_mesh_lib/export_scene.cpp) + CInstanceGroup::serial. This tool replicates
// that per-file: one <igname>.ig per distinct ig name into the output directory.
//
// Instance transforms replicate GetNodeTM(0) (PRS controller values at t=0 composed through the
// node hierarchy in Max float Matrix3 math) -> localTM = nodeTM * Inverse(parentTM) ->
// decomp_affine (see max_math.cpp) -> the decompMatrix Rot/Pos/Scale convention.
//
// Not yet implemented (tracked in pipeline_max_design.md): point lights (Max light object
// decode), clusters/portals and the clusterize instance-cluster linking (accelerator meshes).
// Files needing those report a warning per feature so the corpus harness can bucket them.

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
#include <nel/misc/algo.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/quat.h>
#include <nel/misc/vector.h>

#include <nel/3d/texture.h>
#include <nel/3d/particle_system_shape.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/scene_group.h>
#include <nel/3d/shape.h>

#include "../pipeline_max/storage_ole.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/geom_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"

#include "../pipeline_max_export_common/max_math.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/edit_mesh_mod.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

// Shared Max scene-graph transform helpers (formerly file-local copies here); see max_scene.h.
using MAXSCENE::CLASSID_PRS_CTRL;
using MAXSCENE::CLASSID_LOOKAT_CTRL;
using MAXSCENE::posValueAt0;
using MAXSCENE::rotValueAt0;
using MAXSCENE::scaleValueAt0;
using MAXSCENE::readObjectOffset;
using MAXSCENE::getNodeTM;
using MAXSCENE::SNodeTMCache;

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_IGNAME 1423062564
#define NEL3D_APPDATA_ACCEL 1423062561
#define NEL3D_APPDATA_ACCEL_DEFAULT 32
#define NEL3D_APPDATA_INSTANCE_NAME 1423062562
#define NEL3D_APPDATA_DONT_ADD_TO_SCENE 1423062563
#define NEL3D_APPDATA_INSTANCE_SHAPE 1970
#define NEL3D_APPDATA_COLLISION 1423062613
#define NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION 1423062671
#define NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR 1423062636
#define NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR 1423062637
#define NEL3D_APPDATA_EXPORT_REALTIME_LIGHT 1423062588
#define NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT 1423062591
#define NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN 1423062672
#define NEL3D_APPDATA_OCC_MODEL 84682540
#define NEL3D_APPDATA_OPEN_OCC_MODEL 84682541
#define NEL3D_APPDATA_SOUND_GROUP 84682542
#define NEL3D_APPDATA_ENV_FX 84682543

// AppData script-entry key (the MaxScript utility panel writes these)
static const NLMISC::CClassId APPDATA_SCRIPT_CLASS_ID(0x04d64858, 0x16d1751d);
static const uint32 APPDATA_SCRIPT_SUPER_CLASS_ID = 4128;

// Scene class ids (CLASSID_PRS_CTRL / CLASSID_LOOKAT_CTRL come from MAXSCENE, imported above)
static const NLMISC::CClassId CLASSID_OSM_DERIVED(0x29263a68, 0x405f22f5);
static const NLMISC::CClassId CLASSID_WSM_DERIVED(0x4ec13906, 0x5578130e);
static const NLMISC::CClassId CLASSID_RPO(0x368c679f, 0x711c22ee);
static const NLMISC::CClassId CLASSID_TARGET(0x00001020, 0x00000000);
static const uint32 CLASSID_PARTA_DUMMY = 0x876234;
static const uint32 CLASSID_PARTA_NEL_PS = 0x58ce2893;
static const NLMISC::CClassId CLASSID_PARAM_BLOCK_2(0x00000082, 0x00000000);

// Superclass ids
static const TSClassId SCLASS_GEOMOBJECT = 0x00000010;
static const TSClassId SCLASS_SHAPE = 0x00000040;
static const TSClassId SCLASS_LIGHT = 0x00000030;
static const TSClassId SCLASS_CAMERA = 0x00000020;
static const TSClassId SCLASS_HELPER = 0x00000050;
static const TSClassId SCLASS_OSMODIFIER = 0x00000810;
static const TSClassId SCLASS_WSMODIFIER = 0x00000820;

static bool g_verbose = false;
// Search directories for .ps shapes (the clusterize link test needs the FX AABBox, like the
// reference exporter's CPath::lookup of ps_file_name); set via --ps-path, repeatable.
static std::vector<std::string> g_psSearchPaths;
// Database root for XRef resolution (the ryzomcore_graphics checkout) lives in DBPATH now
// (shared with pipeline_max_export_shape) — deduced from the input path or passed via --db;
// --path-alias registers additional DBPATH::addAlias() roots for corpus content that doesn't
// follow the "R:\graphics\..." / "R:\database\..." convention.
static CSceneClassRegistry *g_registry = NULL;

// ---------------------------------------------------------------------------------------------
// AppData access. Script entries are keyed (MAXSCRIPT_UTILITY_CLASS_ID, 4128, subId) and hold
// null-terminated strings.

static bool getScriptAppData(CSceneClass *sc, uint32 subId, std::string &out)
{
	CAnimatable *anim = dynamic_cast<CAnimatable *>(sc);
	if (!anim) return false;
	STORAGE::CAppData *ad = anim->appData();
	if (!ad) return false;
	STORAGE::CAppData::TMap::const_iterator it = ad->entries().find(
		STORAGE::CAppData::TKey(APPDATA_SCRIPT_CLASS_ID, APPDATA_SCRIPT_SUPER_CLASS_ID, subId));
	if (it == ad->entries().end()) return false;
	CStorageRaw *raw = it->second->value<CStorageRaw>();
	if (!raw) return false;
	// getScriptAppData (string variant) requires the last byte to be the null terminator.
	if (raw->Value.empty() || raw->Value[raw->Value.size() - 1] != '\0') return false;
	out = std::string(raw->Value.begin(), raw->Value.end() - 1);
	return true;
}

static std::string getScriptAppDataStr(CSceneClass *sc, uint32 subId, const std::string &def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	return s;
}

static int getScriptAppDataInt(CSceneClass *sc, uint32 subId, int def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	int value = 0;
	if (NLMISC::fromString(s, value)) return value;
	return def;
}

// ---------------------------------------------------------------------------------------------
// Object resolution.

static CSceneClass *resolveXRefObject(CSceneClass *xrefObj, int depth);

// The node's object reference, derived-object wrappers unwrapped down to the base object,
// XRef objects resolved into the referenced scene (EvalWorldState semantics).
static CSceneClass *baseObjectOfObj(CSceneClass *obj, int depth)
{
	int guard = 16;
	while (obj && guard-- > 0)
	{
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid.a() == 0x92aab38c)
		{
			// XRefObject: resolve to the referenced file's object.
			CSceneClass *resolved = resolveXRefObject(obj, depth);
			if (!resolved) return obj; // unresolvable: keep the wrapper (classified none)
			obj = resolved;
			continue;
		}
		if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
		// Derived object: modifiers (superclass 0x810/0x820) + the base object. Take the last
		// reference that is not a modifier.
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		CSceneClass *base = NULL;
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (!r) continue;
			TSClassId scid = r->classDesc()->superClassId();
			if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER) continue;
			base = r;
		}
		if (!base) break;
		obj = base;
	}
	return obj;
}

static CSceneClass *baseObjectOf(INode &node)
{
	return baseObjectOfObj(dynamic_cast<CSceneClass *>(node.getReference(1)), 0);
}

// ---------------------------------------------------------------------------------------------
// XRef resolution: the XRefObject's 0x0170 record stores the source file (UTF-16, authored-era
// Windows path like R:\graphics\stuff\...) and the source node name. Load the referenced .max
// (cached per resolved path) and resolve the named node's base object.

struct SLoadedMax
{
	CDllDirectory *Dll;
	CClassDirectory3 *Cd;
	CScene *Scene;
	SLoadedMax() : Dll(NULL), Cd(NULL), Scene(NULL) { }
};

static std::map<std::string, SLoadedMax> g_xrefScenes;

static SLoadedMax *loadMaxFileCached(const std::string &path)
{
	std::map<std::string, SLoadedMax>::iterator it = g_xrefScenes.find(path);
	if (it != g_xrefScenes.end()) return it->second.Scene ? &it->second : NULL;
	SLoadedMax &lm = g_xrefScenes[path]; // inserted empty: failure is cached too
	CStorageOleIn in;
	if (!in.open(path)) { fprintf(stderr, "WARNING: xref: not an OLE compound file: %s\n", path.c_str()); return NULL; }
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(g_registry, dll, cd);
	bool ok = true;
	{ std::vector<uint8> b; if (in.readStream("DllDirectory", b)) { CStorageStream st(b); dll->serial(st); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("ClassDirectory3", b)) { CStorageStream st(b); cd->serial(st); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("Scene", b)) { CStorageStream st(b); scene->serial(st); scene->parse(VersionUnknown); } else ok = false; }
	if (!ok)
	{
		fprintf(stderr, "WARNING: xref: missing streams in %s\n", path.c_str());
		delete scene; delete cd; delete dll;
		return NULL;
	}
	lm.Dll = dll;
	lm.Cd = cd;
	lm.Scene = scene;
	return &lm;
}

// Read the ucstring value of a child chunk in a raw container.
static bool xrefChildString(CStorageContainer *cont, uint16 id, std::string &out)
{
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != id) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) return false;
		ucstring us;
		us.resize(raw->Value.size() / 2);
		if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
		out = us.toUtf8();
		return true;
	}
	return false;
}

static CSceneClass *resolveXRefObject(CSceneClass *xrefObj, int depth)
{
	if (depth > 8)
	{
		fprintf(stderr, "WARNING: xref: recursion depth exceeded\n");
		return NULL;
	}
	// Find the 0x0170 record among the orphaned chunks.
	CStorageContainer *rec = NULL;
	const CStorageContainer::TStorageObjectContainer &orphans = xrefObj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != 0x0170) continue;
		rec = dynamic_cast<CStorageContainer *>(it->second);
		break;
	}
	if (!rec)
	{
		fprintf(stderr, "WARNING: xref: no 0x0170 record on XRefObject\n");
		return NULL;
	}
	std::string file, objName;
	if (!xrefChildString(rec, 0x0100, file) || !xrefChildString(rec, 0x0110, objName))
	{
		fprintf(stderr, "WARNING: xref: incomplete 0x0170 record\n");
		return NULL;
	}

	// Authored path (R:\graphics\... or an explicit --path-alias prefix) -> on-disk path.
	std::string resolved;
	if (!DBPATH::resolve(file, resolved))
	{
		fprintf(stderr, "WARNING: xref: cannot resolve '%s' under db root '%s'\n",
		        file.c_str(), DBPATH::defaultRoot().c_str());
		return NULL;
	}

	SLoadedMax *lm = loadMaxFileCached(resolved);
	if (!lm) return NULL;

	// Find the named node in the referenced scene.
	CSceneClassContainer *ssc = lm->Scene->container();
	std::string wantLower = NLMISC::toLowerAscii(objName);
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (NLMISC::toLowerAscii(ucstring(node->userName()).toUtf8()) != wantLower) continue;
		return baseObjectOfObj(dynamic_cast<CSceneClass *>(node->getReference(1)), depth + 1);
	}
	fprintf(stderr, "WARNING: xref: node '%s' not found in %s\n", objName.c_str(), resolved.c_str());
	return NULL;
}

// The category superclass the maxscript selection passes see (geometry/lights/helpers object
// sets in MaxScript go by the evaluated object's superclass).
static TSClassId nodeCategory(INode &node)
{
	CSceneClass *obj = baseObjectOf(node);
	if (!obj) return 0;
	return obj->classDesc()->superClassId();
}

static bool isZone(INode &node)
{
	CSceneClass *obj = baseObjectOf(node);
	return obj && obj->classDesc()->classId() == CLASSID_RPO;
}

static bool objIsParticleSystem(CSceneClass *obj)
{
	return obj && obj->classDesc()->classId().a() == CLASSID_PARTA_NEL_PS;
}

// CExportNel::isMesh (excludeCollision=true variant): the evaluated object converts to a
// TriObject. Headless: GeomObject superclass, excluding the camera/light target object (the
// only corpus GeomObject that refuses TriObject conversion), excluding collision-flagged nodes.
static bool isMesh(INode &node, bool excludeCollision = true)
{
	CSceneClass *obj = baseObjectOf(node);
	if (!obj) return false;
	if (obj->classDesc()->superClassId() != SCLASS_GEOMOBJECT) return false;
	if (obj->classDesc()->classId() == CLASSID_TARGET) return false;
	if (excludeCollision)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
		if (n && getScriptAppDataInt(n, NEL3D_APPDATA_COLLISION, 0) == 1) return false;
	}
	return true;
}

static bool isDummy(INode &node)
{
	CSceneClass *obj = baseObjectOf(node);
	return obj && obj->classDesc()->classId().a() == CLASSID_PARTA_DUMMY;
}

// ---------------------------------------------------------------------------------------------
// ParamBlock2 constant string parameter (ps_file_name on the scripted particle system object).
// PB2 0x000e param records: [u16 param-id][u16 type][10 bytes][flag byte(+payload)]; flag bit
// 0x40 = constant value follows. For TYPE_STRING the payload is a length-prefixed string.

static bool getPB2StringParam(CSceneClass *obj, uint16 paramId, std::string &out)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	if (!rm) return false;
	CReferenceMaker *pb2 = NULL;
	for (uint i = 0; i < rm->nbReferences() && !pb2; ++i)
	{
		CReferenceMaker *r = dynamic_cast<CReferenceMaker *>(rm->getReference(i));
		if (r && dynamic_cast<CSceneClass *>(r) && dynamic_cast<CSceneClass *>(r)->classDesc()->classId() == CLASSID_PARAM_BLOCK_2)
			pb2 = r;
	}
	if (!pb2) return false;
	const CStorageContainer::TStorageObjectContainer &orphans = pb2->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != 0x000e) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 15) continue;
		uint16 id;
		memcpy(&id, nlVectorData(raw->Value), 2);
		if (id != paramId) continue;
		uint8 flag = raw->Value[14];
		if (!(flag & 0x40)) return false; // controller-backed, not a constant
		// Constant payload after the flag byte. Strings are u32-length-prefixed, then chars
		// (null included in the length in the observed corpus records).
		if (raw->Value.size() < 15 + 4) return false;
		uint32 len;
		memcpy(&len, nlVectorData(raw->Value) + 15, 4);
		if (len > raw->Value.size() - 19) len = (uint32)(raw->Value.size() - 19);
		std::string s((const char *)nlVectorData(raw->Value) + 19, len);
		while (!s.empty() && s[s.size() - 1] == '\0') s.resize(s.size() - 1);
		out = s;
		return true;
	}
	return false;
}

// CExportNel::getNelObjectName.
static std::string getNelObjectName(INode &node)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string nodeName = n ? ucstring(n->userName()).toUtf8() : std::string();
	CSceneClass *obj = baseObjectOf(node);

	// Particle system: shape name from the PB2 ps_file_name parameter.
	if (objIsParticleSystem(obj))
	{
		std::string psName;
		if (getPB2StringParam(obj, 0, psName) && !psName.empty())
			return NLMISC::CFile::getFilename(psName);
	}

	// Node-level NEL3D_APPDATA_INSTANCE_SHAPE
	std::string s;
	if (n && getScriptAppData(n, NEL3D_APPDATA_INSTANCE_SHAPE, s))
		return s.empty() ? nodeName : s;

	// Object-level NEL3D_APPDATA_INSTANCE_SHAPE
	if (obj && getScriptAppData(obj, NEL3D_APPDATA_INSTANCE_SHAPE, s))
		return s.empty() ? nodeName : NLMISC::CFile::getFilename(s);

	return nodeName;
}

// ---------------------------------------------------------------------------------------------
// Node flag chunks on CNodeImpl:
//   0x0963 (8 bytes, 2 dwords): node state flags; bit 0x40 of dword 0 = hidden (established by
//     the swt exporter).
//   0x0974 (4 bytes): node wireframe color (BGR dword) — NOT flags; documented here because two
//     earlier cast-shadow hypotheses pattern-matched on it by coincidence.
//   0x099c (4 bytes): the rendering-control flag word (object-properties Rendering Control).
//     Observed 0x4b00f0xx / 0x4b00f6xx / 0x4b00d6xx; bits 0x0200/0x0400 clear exactly on the
//     nodes whose reference ig instances carry DontCastShadow=true (cast+receive shadow pair;
//     0x0200 = cast-shadows, 0x0400 = receive-shadows — discriminated by the canope class: 0x4b00d4xx receives but does not cast, matching DontCastShadow=true in its reference).
#define NODE_FLAGS_CHUNK_ID 0x0963
#define NODE_FLAG_HIDDEN 0x00000040
#define NODE_RENDERFLAGS_CHUNK_ID 0x099c
#define NODE_RENDERFLAG_CASTSHADOW 0x00000200

static uint32 readNodeDword(CNodeImpl *node, uint16 chunkId, bool &found)
{
	found = false;
	uint32 fl = 0;
	CStorageRaw *flags = dynamic_cast<CStorageRaw *>(node->findStorageObject(chunkId));
	if (flags && flags->Value.size() >= 4)
	{
		memcpy(&fl, nlVectorData(flags->Value), 4);
		found = true;
		return fl;
	}
	const CStorageContainer::TStorageObjectContainer &orphans = node->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt oit = orphans.begin(); oit != orphans.end(); ++oit)
	{
		if (oit->first != chunkId) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(oit->second);
		if (raw && raw->Value.size() >= 4)
		{
			memcpy(&fl, nlVectorData(raw->Value), 4);
			found = true;
		}
		break;
	}
	return fl;
}

// ---------------------------------------------------------------------------------------------
// Ligo brick ig export (build_gamedata processes/ligo, nel_ligo_export.ms): the same
// zonematerial-<mat>-<cell>.max / zonespecial-<name>.max / zonetransition-<a>-<b>-<t>.max
// filename protocol pipeline_max_export_zone classifies. Ligo bricks carry the same
// NEL3D_APPDATA_IGNAME-tagged ig content as the standalone ig process, with two differences
// (see exportInstanceGroupFromZone / getIg in the maxscript): the ig name is matched and
// filenamed LOWERCASED (the standalone tool keeps the appdata's original casing), and
// zonetransition files export one ig PER GRID SLOT (0..8) rather than one per distinct name —
// only nodes whose ig name matches that exact slot's zoneBaseName are selected, and each
// selected node is repositioned into the slot via buildTransitionMatrixObj before the normal
// instance-group build. zonematerial/zonespecial export every distinct name in the file, same
// as the standalone tool otherwise (igName == "" in the maxscript).
//
// The transition tables below are the same literal game constants pipeline_max_export_zone
// already carries (duplicated here rather than shared — they are static data, not design
// knowledge that could drift; see max_math.cpp's per-tool duplication for the established
// precedent). buildTransitionMatrixObj is a DIFFERENT function from zone's buildTransitionMatrix
// despite the similar shape: zone's version repositions the RklPatch's own vertices (zero pos,
// mirror/rotate about the origin, translate to the grid slot plus the original offset); this
// one repositions a live node's WORLD transform by recentering the pivot at the CELL CENTER
// before the mirror/rotate (so a mirrored/rotated template object flips about the cell it sits
// in, not about the world origin) and composing the result as (original world TM) * (placement
// matrix) — matching the maxscript composing `mt * copyMt` rather than overwriting `mt` in
// place.

static const bool TransitionScale[9] = { false, false, false, false, true, false, false, false, false };
static const int TransitionRot[9]    = { 2, 1, 3, 0, 1, 3, 0, 0, 0 };
static const float TransitionPos[9][3] = {
	{ 0, 0, 0 }, { -1, 0, 0 }, { -1, -1, 0 }, { -1, -2, 0 }, { 0, -2, 0 },
	{ 0, -3, 0 }, { -1, -3, 0 }, { -2, -3, 0 }, { -3, -3, 0 }
};
// TransitionIds[y][x], -1 = undefined; y rows of variable length (x < len)
static const int TransitionIdsRow0[] = { 1, 2 };
static const int TransitionIdsRow1[] = { -1, 3 };
static const int TransitionIdsRow2[] = { 5, 4 };
static const int TransitionIdsRow3[] = { 6, 7, 8, 9 };
static const int *TransitionIds[4] = { TransitionIdsRow0, TransitionIdsRow1, TransitionIdsRow2, TransitionIdsRow3 };
static const int TransitionIdsLen[4] = { 2, 2, 2, 4 };

static Matrix3M buildTransitionMatrixObj(const Matrix3M &mt, int transitionZone, float cellSize)
{
	// copyMt = transMatrix(TransitionPos[z]*cellSize)
	Matrix3M copyMt = Matrix3M::identity();
	copyMt.m[3][0] = TransitionPos[transitionZone][0] * cellSize;
	copyMt.m[3][1] = TransitionPos[transitionZone][1] * cellSize;
	copyMt.m[3][2] = TransitionPos[transitionZone][2] * cellSize;

	// copyMt = translate(copyMt, [-cellSize/2, -cellSize/2, 0])
	copyMt.m[3][0] += -cellSize / 2.0f;
	copyMt.m[3][1] += -cellSize / 2.0f;

	if (TransitionScale[transitionZone])
	{
		// scale copyMt [-1,1,1]: post-multiply by diag(-1,1,1) -> negate column 0 of every row,
		// including the just-set translation (the recentering above puts the cell center at the
		// origin at exactly this point in the composition, so the mirror pivots about the cell).
		for (int i = 0; i < 4; ++i)
			copyMt.m[i][0] = -copyMt.m[i][0];
	}

	if (TransitionRot[transitionZone] != 0)
	{
		// rotateZ copyMt (90*rot): post-multiply by RotateZMatrix(degrees)
		double rad = (double)(90 * TransitionRot[transitionZone]) * 3.14159265358979323846 / 180.0;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		Matrix3M rz = Matrix3M::identity();
		rz.m[0][0] = c; rz.m[0][1] = s;
		rz.m[1][0] = -s; rz.m[1][1] = c;
		copyMt = copyMt * rz;
	}

	// copyMt = translate(copyMt, [cellSize/2, cellSize/2, 0])
	copyMt.m[3][0] += cellSize / 2.0f;
	copyMt.m[3][1] += cellSize / 2.0f;

	// return (mt * copyMt)
	return mt * copyMt;
}

static bool nodeIsFrozen(CNodeImpl *node)
{
	const CStorageContainer::TStorageObjectContainer &orphans = node->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		if (it->first == 0x0976) return true;
	return false;
}

// The bbox center of an RklPatch node's own (pre-modifier) patch vertices, for 160m-cell grid
// classification only (the maxscript's node.center via getTransitionZoneCoordinates). Using the
// RklPatch's raw stored vertices rather than evaluating the NeL Edit/Paint modifier stack is
// sufficient here: modifier deltas are geometry-local, far smaller than a cell, and this check
// only needs to land in the right 160m bucket, not be vertex-exact (unlike the .zone/.ligozone
// export itself, which does replicate that full modifier evaluation).
static bool rklPatchCenter(NELPATCH::CRklPatchObject *rpo, CNodeImpl *node, SNodeTMCache &tmCache, float center[3])
{
	NELPATCH::SPatchMesh pm;
	std::string err;
	if (!rpo->decodePatch(pm, err) || pm.Verts.empty()) return false;
	Point3M offPos = { 0.0f, 0.0f, 0.0f };
	QuatM offRot = { 0.0f, 0.0f, 0.0f, 1.0f };
	ScaleValueM offScale;
	offScale.s.x = offScale.s.y = offScale.s.z = 1.0f;
	offScale.q.x = offScale.q.y = offScale.q.z = 0.0f;
	offScale.q.w = 1.0f;
	bool hasOffset = readObjectOffset(node, offPos, offRot, offScale);
	Matrix3M objectTM = hasOffset ? (composePRS(offPos, offRot, offScale) * getNodeTM(node, tmCache)) : getNodeTM(node, tmCache);
	float bbMin[3] = { 0, 0, 0 }, bbMax[3] = { 0, 0, 0 };
	bool first = true;
	for (size_t i = 0; i < pm.Verts.size(); ++i)
	{
		Point3M v = { pm.Verts[i].Pos[0], pm.Verts[i].Pos[1], pm.Verts[i].Pos[2] };
		Point3M w = transformPoint(v, objectTM);
		float p[3] = { w.x, w.y, w.z };
		for (int a = 0; a < 3; ++a)
		{
			if (first || p[a] < bbMin[a]) bbMin[a] = p[a];
			if (first || p[a] > bbMax[a]) bbMax[a] = p[a];
		}
		first = false;
	}
	center[0] = (bbMin[0] + bbMax[0]) * 0.5f;
	center[1] = (bbMin[1] + bbMax[1]) * 0.5f;
	center[2] = (bbMin[2] + bbMax[2]) * 0.5f;
	return true;
}

// True if every non-frozen RklPatch node in the scene classifies into a valid transition grid
// cell (the maxscript's getTransitionZoneCoordinates gate: exportInstanceGroupFromZone only runs
// per zone when this holds for the whole file, same as the .ligozone export it runs alongside).
static bool classifyTransitionGrid(CSceneClassContainer *ssc, SNodeTMCache &tmCache, float cellSize, std::string &err)
{
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (nodeIsFrozen(node)) continue;
		CSceneClass *obj = baseObjectOf(*node);
		NELPATCH::CRklPatchObject *rpo = dynamic_cast<NELPATCH::CRklPatchObject *>(obj);
		if (!rpo) continue;
		float center[3];
		if (!rklPatchCenter(rpo, node, tmCache, center))
		{
			err = "node '" + ucstring(node->userName()).toUtf8() + "': empty patch";
			return false;
		}
		int x = (int)(center[0] / cellSize);
		int y = (int)(center[1] / cellSize);
		if (y < 0 || y >= 4 || x < 0 || x >= TransitionIdsLen[y] || TransitionIds[y][x] < 0)
		{
			err = "node '" + ucstring(node->userName()).toUtf8() + "' is not at a transition scheme position (cell "
			      + NLMISC::toString(x) + "," + NLMISC::toString(y) + ")";
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Max light objects (superclass 0x30). Storage decode (correlated against the reference igs'
// point lights, light-by-light; see pipeline_max_design.md §10g):
//   class (0x1011,0) = Omni, (0x1012,0) = target spot, (0x1013,0)/(0x1015,0) = directional,
//   (0x1014,0) = free spot.
//   ParamBlock (reference 0): omni 15-param layout: 0 = color (Point3 0..1), 1 = multiplier,
//     6/7 = attenuation start/end. Spot 18-param layout: 0 = color, 4 = hotspot (degrees),
//     5 = falloff (degrees), 9/10 = attenuation start/end.
//   Object flag words (2-byte chunks): 0x2561 = affect diffuse, 0x2562 = USE ATTENUATION
//     (discriminated by ref lights with atten (10,10) = off while carrying real pblock radii).
//     affect-specular storage not yet located (every reference light has specular == diffuse);
//     defaults to the affect-diffuse value pending the raw-intermediate reference round.
//   Chunk 0x2600 (empty, presence flag) = "ambient only".
//   AppData: 41654685 = animated light name (NEL3D_APPDATA_LM_ANIMATED_LIGHT; the values "Sun",
//     "GlobalLight" and "(Use NelLight Modifier)" mean none), 41654687 = light group
//     (NEL3D_APPDATA_LM_LIGHT_GROUP).

#define NEL3D_APPDATA_LM_ANIMATED_LIGHT 41654685
#define NEL3D_APPDATA_LM_LIGHT_GROUP 41654687

struct SPBlockParam
{
	bool IsPoint3;
	bool IsInt;
	sint32 I;
	float V[3];
};

static void readPBlockParams(CSceneClass *pblock, std::map<sint32, SPBlockParam> &out)
{
	const CStorageContainer::TStorageObjectContainer &po = pblock->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = po.begin(); it != po.end(); ++it)
	{
		if (it->first != 0x0002) continue;
		CStorageContainer *pc = dynamic_cast<CStorageContainer *>(it->second);
		if (!pc) continue;
		sint32 idx = -1;
		for (CStorageContainer::TStorageObjectConstIt cit = pc->chunks().begin(); cit != pc->chunks().end(); ++cit)
		{
			CStorageRaw *cr = dynamic_cast<CStorageRaw *>(cit->second);
			if (!cr) continue;
			if (cit->first == 0x0003 && cr->Value.size() == 4)
				memcpy(&idx, nlVectorData(cr->Value), 4);
			else if (cit->first == 0x0102 && cr->Value.size() == 12 && idx >= 0)
			{
				SPBlockParam p;
				p.IsPoint3 = true;
				memcpy(p.V, nlVectorData(cr->Value), 12);
				out[idx] = p;
			}
			else if (cit->first != 0x0004 && cr->Value.size() == 4 && idx >= 0)
			{
				SPBlockParam p;
				p.IsPoint3 = false;
				p.IsInt = (cit->first == 0x0101);
				p.V[1] = p.V[2] = 0.0f;
				memcpy(p.V, nlVectorData(cr->Value), 4);
				memcpy(&p.I, nlVectorData(cr->Value), 4);
				out[idx] = p;
			}
		}
	}
}

// Objects' 2-byte flag words + presence flags.
static bool lightWord(CSceneClass *obj, uint16 chunkId, uint16 &out)
{
	const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != chunkId) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 2) return false;
		memcpy(&out, nlVectorData(raw->Value), 2);
		return true;
	}
	return false;
}

static bool lightHasChunk(CSceneClass *obj, uint16 chunkId)
{
	const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		if (it->first == chunkId) return true;
	return false;
}

enum TMaxLightKind
{
	maxLightNone,
	maxLightOmni,
	maxLightTargetSpot,
	maxLightFreeSpot,
	maxLightDir
};

static TMaxLightKind maxLightKind(CSceneClass *obj)
{
	if (!obj || obj->classDesc()->superClassId() != SCLASS_LIGHT) return maxLightNone;
	switch (obj->classDesc()->classId().a())
	{
	case 0x1011: return maxLightOmni;
	case 0x1012: return maxLightTargetSpot;
	case 0x1014: return maxLightFreeSpot;
	case 0x1013:
	case 0x1015: return maxLightDir;
	}
	fprintf(stderr, "WARNING: unknown light class %s, skipped\n", obj->classDesc()->classId().toString().c_str());
	return maxLightNone;
}

// Replicates the PointLight part of buildInstanceGroup + SLightBuild::convertFromMaxLight for
// one node. Returns false when the node is not a light or is a directional (skipped); sunLight
// is set when a directional light carries the EXPORT_AS_SUN_LIGHT appdata.
static bool convertMaxLight(NL3D::CPointLightNamed &plNamed, INode &node, SNodeTMCache &tmCache, bool &sunLight)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return false;
	CSceneClass *obj = baseObjectOf(node);
	TMaxLightKind kind = maxLightKind(obj);
	if (kind == maxLightNone) return false;

	// Directional: only the sun check.
	if (kind == maxLightDir)
	{
		if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, 0) == 1)
			sunLight = true;
		return false;
	}

	// And if this light is checked to realtime export (default checked)
	if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, 1) != 1)
		return false;

	std::map<sint32, SPBlockParam> params;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	for (uint r = 0; rm && r < rm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
		if (ref && ref->classDesc()->superClassId() == 0x8)
		{
			readPBlockParams(ref, params);
			break; // reference 0 is the light's own param block
		}
	}
	if (params.find(0) == params.end() || !params[0].IsPoint3)
	{
		fprintf(stderr, "WARNING: light '%s' without color param, skipped\n", ucstring(n->userName()).toUtf8().c_str());
		return false;
	}

	// Color
	NLMISC::CRGBAF nelFColor;
	nelFColor.R = params[0].V[0];
	nelFColor.G = params[0].V[1];
	nelFColor.B = params[0].V[2];
	nelFColor.A = 1.f;
	NLMISC::CRGBA nelColor = nelFColor;

	uint16 w = 0;
	bool ambientOnly = lightHasChunk(obj, 0x2600);
	// Affect-diffuse/specular storage is not located (0x2561 was a false lead: the outgame
	// lights carry 0 there yet export colored diffuse in the raw intermediates). Every corpus
	// light exports with both enabled — unconditional until a counterexample appears.
	bool affectDiffuse = true;
	bool affectSpecular = true;
	bool useAtten = lightWord(obj, 0x2562, w) && w != 0;

	NLMISC::CRGBA ambient(0, 0, 0), diffuse(0, 0, 0), specular(0, 0, 0);
	if (ambientOnly)
	{
		ambient = nelColor;
	}
	else
	{
		if (affectDiffuse) diffuse = nelColor;
		if (affectSpecular) specular = nelColor;
	}

	// Position from the node TM
	Matrix3M nodeTM = getNodeTM(&node, tmCache);
	NLMISC::CVector position(nodeTM.m[3][0], nodeTM.m[3][1], nodeTM.m[3][2]);

	// Direction: target node when present, else -K of the node TM
	NLMISC::CVector direction(0, 0, -1);
	if (kind == maxLightTargetSpot)
	{
		CReferenceMaker *tm = dynamic_cast<CReferenceMaker *>(node.getReference(0));
		CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm);
		INode *target = NULL;
		if (tmsc && tmsc->classDesc()->classId() == CLASSID_LOOKAT_CTRL)
			target = dynamic_cast<INode *>(tm->getReference(0));
		if (target)
		{
			Matrix3M targetTM = getNodeTM(target, tmCache);
			direction = NLMISC::CVector(targetTM.m[3][0], targetTM.m[3][1], targetTM.m[3][2]) - position;
			direction.normalize();
		}
		else
		{
			fprintf(stderr, "WARNING: target spot '%s' without target node\n", ucstring(n->userName()).toUtf8().c_str());
		}
	}
	else if (kind == maxLightFreeSpot)
	{
		// -Z row of the node TM (the NeL K column), normalized
		direction = -NLMISC::CVector(nodeTM.m[2][0], nodeTM.m[2][1], nodeTM.m[2][2]);
		direction.normalize();
	}

	// Attenuation radii
	float rRadiusMin = 10.0f, rRadiusMax = 10.0f;
	sint attenStartIdx = (kind == maxLightOmni) ? 6 : 9;
	if (useAtten)
	{
		if (params.find(attenStartIdx) != params.end() && params.find(attenStartIdx + 1) != params.end())
		{
			rRadiusMin = params[attenStartIdx].V[0];
			rRadiusMax = params[attenStartIdx + 1].V[0];
		}
		else
		{
			fprintf(stderr, "WARNING: light '%s' with attenuation but no radii params\n", ucstring(n->userName()).toUtf8().c_str());
		}
	}

	// Fill the CPointLightNamed like the reference PointLight block.
	plNamed.setPosition(position);
	plNamed.setupAttenuation(rRadiusMin, rRadiusMax);
	ambient.A = 255;
	plNamed.setDefaultAmbient(ambient);
	plNamed.setAmbient(ambient);
	plNamed.setDefaultDiffuse(diffuse);
	plNamed.setDiffuse(diffuse);
	plNamed.setDefaultSpecular(specular);
	plNamed.setSpecular(specular);

	// GroupName
	std::string anim = getScriptAppDataStr(n, NEL3D_APPDATA_LM_ANIMATED_LIGHT, "");
	if (anim == "Sun" || anim == "GlobalLight" || anim == "(Use NelLight Modifier)")
		anim.clear();
	plNamed.AnimatedLight = anim;
	plNamed.LightGroup = (uint32)getScriptAppDataInt(n, NEL3D_APPDATA_LM_LIGHT_GROUP, 0);

	if (ambientOnly)
	{
		plNamed.setType(NL3D::CPointLight::AmbientLight);
		plNamed.setAddAmbientWithSun(getScriptAppDataInt(n, NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN, 0) == 1);
	}
	else if (kind == maxLightOmni)
	{
		plNamed.setType(NL3D::CPointLight::PointLight);
	}
	else
	{
		plNamed.setType(NL3D::CPointLight::SpotLight);
		plNamed.setupSpotDirection(direction);
		// rHotspot/rFallof: degrees -> the reference exporter's half-angle radians
		float hotspot = 50.0f, fallsize = 45.0f;
		sint hotIdx = 4, fallIdx = 5;
		if (params.find(hotIdx) != params.end()) hotspot = params[hotIdx].V[0];
		if (params.find(fallIdx) != params.end()) fallsize = params[fallIdx].V[0];
		float rHotspot = (float)(NLMISC::Pi * hotspot / (2.0 * 180.0));
		float rFallof = (float)(NLMISC::Pi * fallsize / (2.0 * 180.0));
		plNamed.setupSpotAngle(rHotspot, rFallof);
	}

	return true;
}

// ---------------------------------------------------------------------------------------------
// Mesh geometry for the accelerator (cluster/portal) and clusterize-linking paths, replicating
// createMeshBuild + convertToWorldCoordinate: world verts = m1 * (objectToLocal * v) where
// objectToLocal = convertMatrix(objectTM * Inverse(nodeTM)) (object offset, chunks 0x096a/b/c
// on the node) and m1 is the node's LOCAL TRS re-composed through NeL CMatrix translate/rotate/
// scale from the decompMatrix parts (the reference recomposes from the DECOMPOSED local matrix).

struct SMeshTri
{
	uint32 A, B, C;
};

struct SMeshData
{
	std::vector<NLMISC::CVector> Vertices; // world space
	std::vector<SMeshTri> Tris;
};

// Parametric primitive mesh generation, GROUND-TRUTH EXACT per ~/prim_mesh_dataset
// (gen_prim_mesh_dataset.ms, Max 9 run 2026-07-06): vertex order, face order, windings,
// including multi-segment grids and the negative-height winding flip. Validated by
// prim_check.py against the dataset manifest (ctest pipeline_max_prim_mesh).
static bool buildParametricMesh(const NLMISC::CClassId &cid, std::map<sint32, SPBlockParam> &params, std::vector<NLMISC::CVector> &verts, std::vector<SMeshTri> &tris)
{
	#define PBF(i) (params.find(i) != params.end() ? params[i].V[0] : 0.0f)
	#define PBI(i) (params.find(i) != params.end() ? (params[i].IsInt ? (sint)params[i].I : (sint)params[i].V[0]) : 0)
	// Parametric primitive topology below is GROUND-TRUTH EXACT per ~/prim_mesh_dataset
	// (gen_prim_mesh_dataset.ms, Max 9 run 2026-07-06): vertex order, face order, windings,
	// including multi-segment grids and the negative-height winding flip. Validated by
	// prim_check.py against the dataset manifest.

	// Box (0x10, 0): params 0/1/2 = length/width/height, 3/4/5 = w/l/h segments.
	if (cid == NLMISC::CClassId(0x00000010, 0x00000000))
	{
		float l = PBF(0), w = PBF(1), h = PBF(2);
		sint ws = std::max(1, PBI(3)), ls = std::max(1, PBI(4)), hs = std::max(1, PBI(5));
		float dx = w / ws, dy = l / ls, dz = h / hs;
		float x0 = -w / 2.0f, y0 = -l / 2.0f;
		sint row = ws + 1;
		sint gridN = (ws + 1) * (ls + 1);
		verts.clear();
		// bottom grid, top grid (ix fastest, x/y ascending by formula — negative dims flip
		// coordinates naturally), then hs-1 middle perimeter rings bottom-up
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, 0.0f));
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, h));
		// perimeter walk from P00: +x, +y, -x, -y
		std::vector<sint> perIx, perIy;
		for (sint ix = 0; ix < ws; ++ix) { perIx.push_back(ix); perIy.push_back(0); }
		for (sint iy = 0; iy < ls; ++iy) { perIx.push_back(ws); perIy.push_back(iy); }
		for (sint ix = ws; ix > 0; --ix) { perIx.push_back(ix); perIy.push_back(ls); }
		for (sint iy = ls; iy > 0; --iy) { perIx.push_back(0); perIy.push_back(iy); }
		sint perN = (sint)perIx.size();
		for (sint r = 1; r < hs; ++r)
			for (sint k = 0; k < perN; ++k)
				verts.push_back(NLMISC::CVector(x0 + dx * perIx[k], y0 + dy * perIy[k], dz * r));
		// level ring index: v = 0 -> bottom grid perim, v = hs -> top grid perim, else mid ring
		#define BOX_RING(v, k) ((v) == 0 ? perIy[(k) % perN] * row + perIx[(k) % perN] \
			: ((v) == hs ? gridN + perIy[(k) % perN] * row + perIx[(k) % perN] \
			: 2 * gridN + ((v) - 1) * perN + ((k) % perN)))
		tris.clear();
		bool flip = h < 0.0f; // per box_negheight GT; negative l/w flip via coordinates only
		#define BOX_TRI(a, b, c) { SMeshTri t = { (uint32)(flip ? (b) : (a)), (uint32)(flip ? (a) : (b)), (uint32)(c) }; tris.push_back(t); }
		// bottom cells: (a, a+row, a+row+1), (a+row+1, a+1, a)
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				sint a = iy * row + ix;
				BOX_TRI(a, a + row, a + row + 1)
				BOX_TRI(a + row + 1, a + 1, a)
			}
		// top cells: (a, a+1, a+1+row), (a+1+row, a+row, a)
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				sint a = gridN + iy * row + ix;
				BOX_TRI(a, a + 1, a + 1 + row)
				BOX_TRI(a + 1 + row, a + row, a)
			}
		// sides: 4 sides in perimeter order (-y, +x, +y, -x), levels bottom-up, segments along;
		// (lo[k], lo[k+1], up[k+1]), (up[k+1], up[k], lo[k])
		sint sideStart[4] = { 0, ws, ws + ls, 2 * ws + ls };
		sint sideLen[4] = { ws, ls, ws, ls };
		for (sint sd = 0; sd < 4; ++sd)
			for (sint v = 0; v < hs; ++v)
				for (sint k = 0; k < sideLen[sd]; ++k)
				{
					sint pk = sideStart[sd] + k;
					sint lo1 = BOX_RING(v, pk), lo2 = BOX_RING(v, pk + 1);
					sint up1 = BOX_RING(v + 1, pk), up2 = BOX_RING(v + 1, pk + 1);
					BOX_TRI(lo1, lo2, up2)
					BOX_TRI(up2, up1, lo1)
				}
		#undef BOX_TRI
		#undef BOX_RING
		return true;
	}

	// Cylinder (0x12, 0): params 0/1 = radius/height, 2/3 = height/cap segments, 4 = sides.
	// Verts: bottom center, rings bottom-up (angle k*2pi/sides CCW from +x), top center.
	if (cid == NLMISC::CClassId(0x00000012, 0x00000000))
	{
		float r = PBF(0), h = PBF(1);
		sint hs = std::max(1, PBI(2)), sides = std::max(3, PBI(4));
		float dz = h / hs;
		verts.clear();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, 0.0f));
		for (sint v = 0; v <= hs; ++v)
			for (sint k = 0; k < sides; ++k)
			{
				double a = 2.0 * NLMISC::Pi * k / sides;
				verts.push_back(NLMISC::CVector(r * (float)cos(a), r * (float)sin(a), dz * v));
			}
		uint32 tc = (uint32)verts.size();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, h));
		#define CYL_RING(v, k) (1 + (v) * sides + ((k) % sides))
		tris.clear();
		#define CYL_TRI(a, b, c) { SMeshTri t = { (uint32)(a), (uint32)(b), (uint32)(c) }; tris.push_back(t); }
		for (sint k = 0; k < sides; ++k) // bottom cap: (c, r[k+1], r[k])
			CYL_TRI(0, CYL_RING(0, k + 1), CYL_RING(0, k))
		for (sint v = 0; v < hs; ++v) // sides: (lo[k], up[k+1], up[k]), (lo[k], lo[k+1], up[k+1])
			for (sint k = 0; k < sides; ++k)
			{
				CYL_TRI(CYL_RING(v, k), CYL_RING(v + 1, k + 1), CYL_RING(v + 1, k))
				CYL_TRI(CYL_RING(v, k), CYL_RING(v, k + 1), CYL_RING(v + 1, k + 1))
			}
		for (sint k = 0; k < sides; ++k) // top cap: (tc, t[k], t[k+1])
			CYL_TRI(tc, CYL_RING(hs, k), CYL_RING(hs, k + 1))
		#undef CYL_TRI
		#undef CYL_RING
		return true;
	}

	// Sphere (0x11, 0): params 0/1 = radius/segments. Verts: top pole, rings top-down with
	// meridians starting at +Y going CCW, bottom pole. The nel_flare delegate.
	if (cid == NLMISC::CClassId(0x00000011, 0x00000000))
	{
		float r = PBF(0);
		sint segs = std::max(4, PBI(1));
		sint rows = segs / 2;
		verts.clear();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, r));
		for (sint i = 1; i < rows; ++i)
		{
			double phi = NLMISC::Pi * i / rows;
			float z = r * (float)cos(phi);
			float rr = r * (float)sin(phi);
			for (sint k = 0; k < segs; ++k)
			{
				double a = NLMISC::Pi / 2.0 + 2.0 * NLMISC::Pi * k / segs;
				verts.push_back(NLMISC::CVector(rr * (float)cos(a), rr * (float)sin(a), z));
			}
		}
		uint32 bp = (uint32)verts.size();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, -r));
		#define SPH_RING(i, k) (1 + ((i) - 1) * segs + ((k) % segs))
		tris.clear();
		#define SPH_TRI(a, b, c) { SMeshTri t = { (uint32)(a), (uint32)(b), (uint32)(c) }; tris.push_back(t); }
		for (sint k = 0; k < segs; ++k) // top fan
			SPH_TRI(0, SPH_RING(1, k), SPH_RING(1, k + 1))
		for (sint i = 1; i < rows - 1; ++i) // quad rows: (u[k], lo[k], lo[k+1]), (u[k], lo[k+1], u[k+1])
			for (sint k = 0; k < segs; ++k)
			{
				SPH_TRI(SPH_RING(i, k), SPH_RING(i + 1, k), SPH_RING(i + 1, k + 1))
				SPH_TRI(SPH_RING(i, k), SPH_RING(i + 1, k + 1), SPH_RING(i, k + 1))
			}
		for (sint k = 0; k < segs; ++k) // bottom fan
			SPH_TRI(bp, SPH_RING(rows - 1, k + 1), SPH_RING(rows - 1, k))
		#undef SPH_TRI
		#undef SPH_RING
		return true;
	}

	// Plane (0x081f1dfc, 0x77566f65): params 0/1 = length/width, 2/3 = length/width segments.
	// Grid at z=0 (ix fastest); per cell (d, a, c), (b, c, a).
	if (cid == NLMISC::CClassId(0x081f1dfc, 0x77566f65))
	{
		float l = PBF(0), w = PBF(1);
		sint ls = std::max(1, PBI(2)), ws = std::max(1, PBI(3));
		float dx = w / ws, dy = l / ls;
		float x0 = -w / 2.0f, y0 = -l / 2.0f;
		sint row = ws + 1;
		verts.clear();
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, 0.0f));
		tris.clear();
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				uint32 a = iy * row + ix;
				uint32 b = a + 1;
				uint32 c = a + row + 1;
				uint32 d = a + row;
				SMeshTri t1 = { d, a, c };
				tris.push_back(t1);
				SMeshTri t2 = { b, c, a };
				tris.push_back(t2);
			}
		return true;
	}

	#undef PBF
	#undef PBI
	return false;
}

// Vertex/face extraction in Max OBJECT space per source object type.
static bool extractObjectMesh(CSceneClass *obj, std::vector<NLMISC::CVector> &verts, std::vector<SMeshTri> &tris, const std::string &nodeName)
{
	NLMISC::CClassId cid = obj->classDesc()->classId();

	// Editable mesh: GeomBuffers 0x08fe, tri A vertex chunk 0x0914 (uint32 count + CVector[]),
	// tri A index chunk 0x0912 (uint32 count + (a,b,c,alwaysOne,smoothing)[]).
	if (cid == NLMISC::CClassId(0xe44f10b3, 0x00000000))
	{
		CGeomObject *geom = dynamic_cast<CGeomObject *>(obj);
		STORAGE::CGeomBuffers *gb = geom ? geom->geomBuffers() : NULL;
		if (!gb)
		{
			fprintf(stderr, "WARNING: accelerator mesh '%s' without geom buffers\n", nodeName.c_str());
			return false;
		}
		// Typed geom buffers (PMBS_GEOM_BUFFERS_PARSE): vertices as CVector[], faces as
		// CGeomTriIndexInfo[] (a,b,c indices + two per-face dwords).
		const std::vector<NLMISC::CVector> *vv = gb->triVertices();
		const std::vector<STORAGE::CGeomTriIndexInfo> *ff = gb->triFaces();
		if (!vv || !ff)
		{
			fprintf(stderr, "WARNING: accelerator mesh '%s' with missing vertex/index chunks\n", nodeName.c_str());
			return false;
		}
		verts = *vv;
		tris.resize(ff->size());
		for (uint32 i = 0; i < ff->size(); ++i)
		{
			tris[i].A = (*ff)[i].a;
			tris[i].B = (*ff)[i].b;
			tris[i].C = (*ff)[i].c;
		}
		return true;
	}

	// Dummies have no mesh; the reference createMeshBuild yields an empty build for them, which
	// still runs the (vacuous) cluster test. Return an empty mesh rather than a warning.
	if (cid.a() == 0x876234)
	{
		verts.clear();
		tris.clear();
		return true;
	}

	// Parametric primitives: pblock reference 0.
	std::map<sint32, SPBlockParam> params;
	{
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		for (uint r = 0; rm && r < rm->nbReferences(); ++r)
		{
			CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
			if (ref && ref->classDesc()->superClassId() == 0x8)
			{
				readPBlockParams(ref, params);
				break;
			}
		}
	}
	if (buildParametricMesh(cid, params, verts, tris))
		return true;

	// Scripted plugin objects (nel_flare extends Sphere, nel_ps extends Box, ...) carry their
	// geometry DELEGATE as a reference — route extraction to it.
	{
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (r && r->classDesc()->superClassId() == 0x10 && r != obj)
			{
				#undef PBF
				#undef PBI
				return extractObjectMesh(r, verts, tris, nodeName);
			}
		}
	}

	#undef PBF
	#undef PBI
	fprintf(stderr, "WARNING: mesh extraction for object class %s ('%s') not implemented\n",
	        cid.toString().c_str(), nodeName.c_str());
	return false;
}

// FX instances: the clusterize test uses the .ps shape's AABBox corners transformed to world
// (the reference reads ps_file_name via ParamBlock2, loads the shape through CPath, and takes
// CParticleSystemShape::getAABBox; fallback is the helper mesh). Returns false when the shape
// cannot be resolved so the caller falls back to the placeholder mesh.
static bool psShapeBBoxVerts(INode &node, CSceneClass *obj, SNodeTMCache &tmCache, std::vector<NLMISC::CVector> &out)
{
	std::string psFilePath;
	if (!getPB2StringParam(obj, 0, psFilePath) || psFilePath.empty())
		psFilePath = getNelObjectName(node);
	if (psFilePath.empty()) return false;
	std::string base = NLMISC::toLowerAscii(NLMISC::CFile::getFilename(psFilePath));
	std::string found;
	for (uint i = 0; i < g_psSearchPaths.size() && found.empty(); ++i)
	{
		std::string cand = g_psSearchPaths[i] + "/" + base;
		if (NLMISC::CFile::fileExists(cand)) found = cand;
	}
	if (found.empty()) return false;

	NL3D::CParticleSystemShape *pss = NULL;
	try
	{
		NLMISC::CIFile f;
		if (!f.open(found)) return false;
		NL3D::CShapeStream ss;
		f.serial(ss);
		pss = dynamic_cast<NL3D::CParticleSystemShape *>(ss.getShapePointer());
		if (!pss)
		{
			fprintf(stderr, "ERROR: Node %s shape is not a FX\n", ucstring(node.userName()).toUtf8().c_str());
			delete ss.getShapePointer();
			return false;
		}
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "WARNING: %s\n", e.what());
		delete pss;
		return false;
	}

	NLMISC::CAABBox bbox;
	pss->getAABBox(bbox);
	// Reference behavior: re-compute an axis-aligned enclosing box in world space via
	// `CAABBox::transformAABBox` before taking the 8 corners. This axis-aligned-in-world
	// inflation is what the plugin does; corpus regression confirmed swapping to a direct
	// 8-corner transform breaks Matis hall_conseil / hall_vitrine (their FX instances land in
	// FEWER clusters than the reference under the tight box).
	Matrix3M tm = getNodeTM(&node, tmCache);
	NLMISC::CMatrix nelXForm;
	nelXForm.identity();
	{
		float m[16];
		m[0] = tm.m[0][0]; m[4] = tm.m[1][0]; m[8] = tm.m[2][0]; m[12] = tm.m[3][0];
		m[1] = tm.m[0][1]; m[5] = tm.m[1][1]; m[9] = tm.m[2][1]; m[13] = tm.m[3][1];
		m[2] = tm.m[0][2]; m[6] = tm.m[1][2]; m[10] = tm.m[2][2]; m[14] = tm.m[3][2];
		m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
		nelXForm.set(m);
	}
	bbox = NLMISC::CAABBox::transformAABBox(nelXForm, bbox);
	out.clear();
	out.reserve(8);
	for (uint k = 0; k < 8; ++k)
	{
		out.push_back(NLMISC::CVector(((k & 1) ? 1 : -1) * bbox.getHalfSize().x + bbox.getCenter().x,
		                              ((k & 2) ? 1 : -1) * bbox.getHalfSize().y + bbox.getCenter().y,
		                              ((k & 4) ? 1 : -1) * bbox.getHalfSize().z + bbox.getCenter().z));
	}
	delete pss;
	return true;
}

// Edit Mesh modifier (class 0x50) evaluation is shared with pipeline_max_export_cmb via
// pipeline_max_export_common/edit_mesh_mod.h — see that header for the MDELTA_CHUNK layout, the
// created-verts/created-faces/face-attrib decoding, and the SEdits struct. This tool applies the
// verts-and-moves-only slice (facesMode=0) because its cluster-containment link test operates on
// vertices only; created faces (0x0208) and the face-attrib rewrites (0x0220) are decoded but
// unused. See design-doc §10w for the corpus-validated chunk-ID mapping and edit_mesh_mod.h for
// the SEdits header comment on why the shared apply order is byte-preserving for this consumer.
typedef EDITMESH::SEdits SEditMeshEdits;

// Mirror modifier (mods.dlm (0xef92aa7c, 0x511bbe75)): ParamBlock params 0 = axis (0..5 =
// X,Y,Z,XY,YZ,ZX), 1 = offset; modifier chunk 0x1000 = copy flag; gizmo = the modifier's own
// PRS controller (reference 0) over the mod-context TM (0x2510 of the paired mod-app slot).
struct SModOp
{
	int Type; // 0 = Edit Mesh, 1 = Mirror
	SEditMeshEdits Edits;
	Matrix3M GizmoTM;
	Matrix3M CtxTM;
	sint MirrorAxis;
	float MirrorOffset;
	bool MirrorCopy;
};

static void applyMirror(const SModOp &op, std::vector<NLMISC::CVector> &verts, std::vector<SMeshTri> &tris)
{
	Matrix3M objToGizmo = op.CtxTM * inverseM3(op.GizmoTM);
	Matrix3M gizmoToObj = inverseM3(objToGizmo);
	static const float FLIPS[6][3] = {
		{ -1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 }, { -1, -1, 1 }, { 1, -1, -1 }, { -1, 1, -1 }
	};
	const float *f = FLIPS[op.MirrorAxis >= 0 && op.MirrorAxis < 6 ? op.MirrorAxis : 0];
	uint32 nv = (uint32)verts.size(), nf = (uint32)tris.size();
	std::vector<NLMISC::CVector> mirrored(nv);
	#define M3_XFORM(M, ix, iy, iz, ox, oy, oz) \
		ox = (ix) * (M).m[0][0] + (iy) * (M).m[1][0] + (iz) * (M).m[2][0] + (M).m[3][0]; \
		oy = (ix) * (M).m[0][1] + (iy) * (M).m[1][1] + (iz) * (M).m[2][1] + (M).m[3][1]; \
		oz = (ix) * (M).m[0][2] + (iy) * (M).m[1][2] + (iz) * (M).m[2][2] + (M).m[3][2];
	for (uint32 i = 0; i < nv; ++i)
	{
		float gx, gy, gz, ox, oy, oz;
		M3_XFORM(objToGizmo, verts[i].x, verts[i].y, verts[i].z, gx, gy, gz)
		// per the dataset offset case: mirrored coord = offset - coord along flipped axes
		gx = gx * f[0] + (f[0] < 0 ? op.MirrorOffset : 0.0f);
		gy = gy * f[1] + (f[1] < 0 ? op.MirrorOffset : 0.0f);
		gz = gz * f[2] + (f[2] < 0 ? op.MirrorOffset : 0.0f);
		M3_XFORM(gizmoToObj, gx, gy, gz, ox, oy, oz)
		mirrored[i] = NLMISC::CVector(ox, oy, oz);
	}
	#undef M3_XFORM
	if (op.MirrorCopy)
	{
		verts.insert(verts.end(), mirrored.begin(), mirrored.end());
		for (uint32 i = 0; i < nf; ++i)
		{
			SMeshTri t = { tris[i].A + nv, tris[i].C + nv, tris[i].B + nv };
			tris.push_back(t);
		}
	}
	else
	{
		verts.swap(mirrored);
		for (uint32 i = 0; i < nf; ++i)
			std::swap(tris[i].B, tris[i].C);
	}
}

// Bridge to the shared apply: SMeshTri exposes A/B/C, the shared code expects V[3]. A tiny
// wrapper adapts the field names for the templated apply call.
struct SMeshTriAdapter
{
	uint32 V[3];
	SMeshTri toTri() const { SMeshTri t = { V[0], V[1], V[2] }; return t; }
};

struct SMeshTriFromCreated
{
	SMeshTriAdapter operator()(uint32 /*vBase*/, const EDITMESH::SFace &cf) const
	{
		// Not called: facesMode=0 in the applyEdits call below.
		SMeshTriAdapter a; a.V[0] = cf.V[0]; a.V[1] = cf.V[1]; a.V[2] = cf.V[2]; return a;
	}
};

// Apply Edit Mesh edits (verts + moves only) via the shared EDITMESH::applyEdits: moves, face
// deletes, vert deletes with face reindexing, created verts appended. facesMode=0 skips created
// faces (ig's cluster-containment link test operates on vertices only; appending faces would
// change its cluster volumes — see edit_mesh_mod.h). Byte-preserving vs the pre-refactor
// inline decode.
static void applyEditMeshEdits(const SEditMeshEdits &e, std::vector<NLMISC::CVector> &verts, std::vector<SMeshTri> &tris)
{
	std::vector<SMeshTriAdapter> adapted(tris.size());
	for (size_t i = 0; i < tris.size(); ++i)
	{
		adapted[i].V[0] = tris[i].A; adapted[i].V[1] = tris[i].B; adapted[i].V[2] = tris[i].C;
	}
	EDITMESH::applyEdits(e, verts, adapted, SMeshTriFromCreated(), 0);
	tris.resize(adapted.size());
	for (size_t i = 0; i < adapted.size(); ++i)
	{
		tris[i].A = adapted[i].V[0]; tris[i].B = adapted[i].V[1]; tris[i].C = adapted[i].V[2];
	}
}

// World-space mesh of a node, replicating createMeshBuild + convertToWorldCoordinate.
static bool nodeWorldMesh(INode &node, SNodeTMCache &tmCache, SMeshData &out)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return false;
	// Walk the object chain manually so Edit Mesh modifier stacks are EVALUATED (EvalWorldState
	// semantics), collecting each derived wrapper's 0x2500 mod-app edits along the way.
	std::vector<SModOp> opStack; // collected outermost-first
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node.getReference(1));
	{
		int guard = 16;
		while (obj && guard-- > 0)
		{
			NLMISC::CClassId cid = obj->classDesc()->classId();
			if (cid.a() == 0x92aab38c)
			{
				CSceneClass *resolved = resolveXRefObject(obj, 0);
				if (!resolved) break;
				obj = resolved;
				continue;
			}
			if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
			CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
			CSceneClass *base = NULL;
			std::vector<CSceneClass *> mods;
			for (uint i = 0; rm && i < rm->nbReferences(); ++i)
			{
				CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
				if (!r) continue;
				TSClassId scid = r->classDesc()->superClassId();
				if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER)
				{
					mods.push_back(r);
					continue;
				}
				base = r;
			}
			// mod-app local data: the wrapper's orphaned 0x2500 containers, one per modifier
			// slot in reference order
			std::vector<CStorageContainer *> modApps;
			{
				const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
				for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
					if (it->first == 0x2500)
						modApps.push_back(dynamic_cast<CStorageContainer *>(it->second));
			}
			for (uint m = 0; m < mods.size(); ++m)
			{
				NLMISC::CClassId mcid = mods[m]->classDesc()->classId();
				CStorageContainer *app = m < modApps.size() ? modApps[m] : NULL;
				if (mcid == NLMISC::CClassId(0x00000050, 0x00000000)) // Edit Mesh
				{
					SModOp op;
					op.Type = 0;
					if (app && EDITMESH::readModApp(app, op.Edits))
						opStack.push_back(op);
				}
				else if (mcid == NLMISC::CClassId(0xef92aa7c, 0x511bbe75)) // Mirror
				{
					SModOp op;
					op.Type = 1;
					op.GizmoTM = Matrix3M::identity();
					op.CtxTM = Matrix3M::identity();
					op.MirrorAxis = 0;
					op.MirrorOffset = 0.0f;
					op.MirrorCopy = false;
					CReferenceMaker *mrm = dynamic_cast<CReferenceMaker *>(mods[m]);
					for (uint r = 0; mrm && r < mrm->nbReferences(); ++r)
					{
						CSceneClass *ref = dynamic_cast<CSceneClass *>(mrm->getReference(r));
						if (!ref) continue;
						if (ref->classDesc()->superClassId() == 0x8)
						{
							std::map<sint32, SPBlockParam> params;
							readPBlockParams(ref, params);
							if (params.find(0) != params.end()) op.MirrorAxis = params[0].IsInt ? (sint)params[0].I : (sint)params[0].V[0];
							if (params.find(1) != params.end()) op.MirrorCopy = (params[1].IsInt ? params[1].I : (sint)params[1].V[0]) != 0;
							if (params.find(2) != params.end() && !params[2].IsInt) op.MirrorOffset = params[2].V[0];
						}
						else if (ref->classDesc()->classId() == NLMISC::CClassId(0x00002005, 0x00000000))
						{
							CSceneClass *pc = dynamic_cast<CSceneClass *>(dynamic_cast<CReferenceMaker *>(ref)->getReference(0));
							Point3M gp = posValueAt0(pc);
							QuatM gr = rotValueAt0(dynamic_cast<CSceneClass *>(dynamic_cast<CReferenceMaker *>(ref)->getReference(1)));
							ScaleValueM gs = scaleValueAt0(dynamic_cast<CSceneClass *>(dynamic_cast<CReferenceMaker *>(ref)->getReference(2)));
							op.GizmoTM = composePRS(gp, gr, gs);
						}
					}
					if (app)
					{
						for (CStorageContainer::TStorageObjectConstIt it = app->chunks().begin(); it != app->chunks().end(); ++it)
						{
							if (it->first != 0x2510) continue;
							CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
							if (raw && raw->Value.size() >= 48)
								memcpy(op.CtxTM.m, nlVectorData(raw->Value), 48);
						}
					}
					opStack.push_back(op);
				}
				else if (mcid != NLMISC::CClassId(0x000f72b1, 0x00000000)) // UVW Map: geometry-neutral
				{
					fprintf(stderr, "WARNING: node '%s' has unhandled modifier %s; geometry evaluated without it\n",
					        ucstring(n->userName()).toUtf8().c_str(), mcid.toString().c_str());
				}
			}
			if (!base) break;
			obj = base;
		}
	}
	if (!obj) return false;
	std::vector<NLMISC::CVector> objVerts;
	if (!extractObjectMesh(obj, objVerts, out.Tris, ucstring(n->userName()).toUtf8()))
		return false;
	// Apply modifier ops base-upward (stack order = reverse of collection order, which walked
	// outermost wrapper first; within a wrapper, reference order = stack order bottom-up already,
	// so replay the collected list back-to-front).
	const char *dbgOps = getenv("PMB_DEBUG_MESH");
	bool dbgThis = dbgOps && ucstring(n->userName()).toUtf8() == dbgOps;
	for (uint i = (uint)opStack.size(); i > 0; --i)
	{
		if (opStack[i - 1].Type == 0)
			applyEditMeshEdits(opStack[i - 1].Edits, objVerts, out.Tris);
		else
			applyMirror(opStack[i - 1], objVerts, out.Tris);
		if (dbgThis)
		{
			float zmin = 1e30f, zmax = -1e30f;
			for (uint v = 0; v < objVerts.size(); ++v) { zmin = std::min(zmin, objVerts[v].z); zmax = std::max(zmax, objVerts[v].z); }
			fprintf(stderr, "DEBUG op[%u] type=%d -> %u verts %u tris, objz [%g, %g] (moves=%u delv=%u delf=%u)\n",
			        i - 1, opStack[i - 1].Type, (uint)objVerts.size(), (uint)out.Tris.size(), zmin, zmax,
			        (uint)opStack[i - 1].Edits.Moves.size(), (uint)opStack[i - 1].Edits.DelVerts.size(), (uint)opStack[i - 1].Edits.DelFaces.size());
		}
	}

	// objectToLocal = objectTM * Inverse(nodeTM) in Max float ops; objectTM = offsetTM * nodeTM.
	Matrix3M nodeTM = getNodeTM(&node, tmCache);
	Point3M opos;
	QuatM orot;
	ScaleValueM oscale;
	readObjectOffset(n, opos, orot, oscale);
	Matrix3M offsetTM = composePRS(opos, orot, oscale);
	Matrix3M objectTM = offsetTM * nodeTM;
	Matrix3M objectToLocal = objectTM * inverseM3(nodeTM);

	// convertMatrix: Max row-vector Matrix3 -> NeL column CMatrix
	NLMISC::CMatrix toExportSpace;
	toExportSpace.identity();
	{
		float m[16];
		m[0] = objectToLocal.m[0][0]; m[4] = objectToLocal.m[1][0]; m[8] = objectToLocal.m[2][0]; m[12] = objectToLocal.m[3][0];
		m[1] = objectToLocal.m[0][1]; m[5] = objectToLocal.m[1][1]; m[9] = objectToLocal.m[2][1]; m[13] = objectToLocal.m[3][1];
		m[2] = objectToLocal.m[0][2]; m[6] = objectToLocal.m[1][2]; m[10] = objectToLocal.m[2][2]; m[14] = objectToLocal.m[3][2];
		m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
		toExportSpace.set(m);
	}

	// m1 = translate(DefaultPos) * rotate(DefaultRotQuat) * scale(DefaultScale) from the
	// decompMatrix parts of the node's local matrix.
	Matrix3M parentTM = getNodeTM(node.parent(), tmCache);
	Matrix3M localTM = nodeTM * inverseM3(parentTM);
	AffinePartsM parts;
	decompAffine(localTM, parts);
	NLMISC::CVector pos(parts.t.x, parts.t.y, parts.t.z);
	NLMISC::CQuat rot(parts.q.x, parts.q.y, parts.q.z, -parts.q.w);
	Matrix3M srtm = quatToMatrix3(parts.u);
	Matrix3M stm = Matrix3M::identity();
	stm.m[0][0] = parts.k.x;
	stm.m[1][1] = parts.k.y;
	stm.m[2][2] = parts.k.z;
	Matrix3M smat = inverseM3(srtm) * stm * srtm;
	NLMISC::CVector scale(parts.f * smat.m[0][0], parts.f * smat.m[1][1], parts.f * smat.m[2][2]);

	NLMISC::CMatrix m1;
	m1.identity();
	m1.translate(pos /* + DefaultPivot(0) */);
	m1.rotate(rot);
	m1.scale(scale);

	out.Vertices.resize(objVerts.size());
	for (uint i = 0; i < objVerts.size(); ++i)
		out.Vertices[i] = m1 * (toExportSpace * objVerts[i]);
	{
		const char *dbg = getenv("PMB_DEBUG_MESH");
		if (dbg && ucstring(n->userName()).toUtf8() == dbg)
		{
			fprintf(stderr, "DEBUG mesh '%s': %u verts %u tris\n", dbg, (uint)out.Vertices.size(), (uint)out.Tris.size());
			for (uint i = 0; i < out.Vertices.size(); ++i)
				fprintf(stderr, "  v%u (%.9g, %.9g, %.9g)\n", i, out.Vertices[i].x, out.Vertices[i].y, out.Vertices[i].z);
		}
	}
	return true;
}


// ---------------------------------------------------------------------------------------------
// The buildInstanceGroup replication.

struct SIgBuildStats
{
	uint UnimplementedAccel;
	SIgBuildStats() : UnimplementedAccel(0) { }
};

static NL3D::CInstanceGroup *buildInstanceGroup(const std::vector<INode *> &vectNode, SNodeTMCache &tmCache, SIgBuildStats &stats)
{
	NL3D::CInstanceGroup::TInstanceArray aIGArray;
	uint32 i, nNumIG;
	uint32 j;

	aIGArray.resize(vectNode.size());

	sint nNbInstance = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		int nAccelType = getScriptAppDataInt(dynamic_cast<CNodeImpl *>(pNode), NEL3D_APPDATA_ACCEL, 32);
		if ((nAccelType & 3) == 0)
			if (!isZone(*pNode))
				if (isMesh(*pNode) || isDummy(*pNode))
					++nNbInstance;
	}

	// Check integrity of the hierarchy and set the parents
	nNumIG = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		CNodeImpl *pNodeImpl = dynamic_cast<CNodeImpl *>(pNode);
		int nAccelType = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType & 3) == 0)
		if (!isZone(*pNode))
		if (isMesh(*pNode) || isDummy(*pNode))
		{
			aIGArray[nNumIG].DontAddToScene = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_DONT_ADD_TO_SCENE, 0) ? true : false;
			aIGArray[nNumIG].InstanceName = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_INSTANCE_NAME, "");
			if (aIGArray[nNumIG].InstanceName.empty())
				aIGArray[nNumIG].InstanceName = ucstring(pNode->userName()).toUtf8();

			// Visible? always true, but if special flag for camera collision
			sint appDataCameraCol = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0);
			aIGArray[nNumIG].Visible = appDataCameraCol != 3;

			// DontCastShadow from the node's CastShadows rendering-control flag.
			bool flagsFound = false;
			uint32 rendFlags = readNodeDword(pNodeImpl, NODE_RENDERFLAGS_CHUNK_ID, flagsFound);
			aIGArray[nNumIG].DontCastShadow = flagsFound && (rendFlags & NODE_RENDERFLAG_CASTSHADOW) == 0;

			aIGArray[nNumIG].DontCastShadowForInterior = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR, 0) ? true : false;
			aIGArray[nNumIG].DontCastShadowForExterior = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR, 0) ? true : false;

			// Deterministic values for the fields the reference exporter leaves untouched
			// (CInstance ctor doesn't initialize these; the 2004 exporter serialized whatever the
			// fresh allocation held — compare against the intermediate exports before trusting).
			aIGArray[nNumIG].StaticLightEnabled = false;
			aIGArray[nNumIG].AvoidStaticLightPreCompute = false;
			aIGArray[nNumIG].SunContribution = 0;
			aIGArray[nNumIG].Light[0] = 0xFF;
			aIGArray[nNumIG].Light[1] = 0xFF;
			aIGArray[nNumIG].LocalAmbientId = 0xFF;

			INode *pParent = pNode->parent();

			// Is the pNode has the root node for parent ?
			if (pParent && pParent != tmCache.SceneRoot)
			{
				// Look if the parent is in the selection. NB: replicates the reference quirk —
				// the search loop counts isMesh-only nodes while instance indices count
				// isMesh||isDummy, and the no-match test compares against the instance count.
				sint nNumIG2 = 0;
				for (j = 0; j < vectNode.size(); ++j)
				{
					INode *pNode2 = vectNode[j];
					int nAccelType2 = getScriptAppDataInt(dynamic_cast<CNodeImpl *>(pNode2), NEL3D_APPDATA_ACCEL, 32);
					if ((nAccelType2 & 3) == 0)
					if (!isZone(*pNode2))
					if (isMesh(*pNode2))
					{
						if (pNode2 == pParent)
							break;
						++nNumIG2;
					}
				}
				if (nNumIG2 == nNbInstance)
					aIGArray[nNumIG].nParent = -1;
				else
					aIGArray[nNumIG].nParent = nNumIG2;
			}
			else
			{
				aIGArray[nNumIG].nParent = -1;
			}
			++nNumIG;
		}
	}
	aIGArray.resize(nNumIG);

	// Build the array of node (transforms)
	nNumIG = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		int nAccelType = getScriptAppDataInt(dynamic_cast<CNodeImpl *>(pNode), NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType & 3) == 0)
		if (!isZone(*pNode))
		if (isMesh(*pNode) || isDummy(*pNode))
		{
			aIGArray[nNumIG].Name = getNelObjectName(*pNode);

			// localTM = nodeTM(0) * Inverse(parentTM(0))
			Matrix3M nodeTM = getNodeTM(pNode, tmCache);
			Matrix3M parentTM = getNodeTM(pNode->parent(), tmCache);
			Matrix3M localTM = nodeTM * inverseM3(parentTM);

			// decompMatrix
			AffinePartsM parts;
			decompAffine(localTM, parts);
			aIGArray[nNumIG].Rot = NLMISC::CQuat(parts.q.x, parts.q.y, parts.q.z, -parts.q.w);
			aIGArray[nNumIG].Pos = NLMISC::CVector(parts.t.x, parts.t.y, parts.t.z);
			// Scale via the stretch matrix diagonal
			Matrix3M srtm = quatToMatrix3(parts.u);
			Matrix3M stm = Matrix3M::identity();
			stm.m[0][0] = parts.k.x;
			stm.m[1][1] = parts.k.y;
			stm.m[2][2] = parts.k.z;
			Matrix3M mat = inverseM3(srtm) * stm * srtm;
			aIGArray[nNumIG].Scale = NLMISC::CVector(parts.f * mat.m[0][0], parts.f * mat.m[1][1], parts.f * mat.m[2][2]);

			++nNumIG;
		}
	}

	// Accelerator Portal/Cluster part
	std::vector<NL3D::CCluster> vClusters;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		CNodeImpl *pNodeImpl = dynamic_cast<CNodeImpl *>(pNode);
		int nAccelType = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
		bool bFatherVisible = (nAccelType & 4) != 0;          // NEL3D_APPDATA_ACCEL_FATHER_VISIBLE
		bool bVisibleFromFather = (nAccelType & 8) != 0;      // NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER
		bool bAudibleLikeVisible = (nAccelType & 64) == 0;    // NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE
		bool bFatherAudible = bAudibleLikeVisible ? bFatherVisible : (nAccelType & 128) != 0;
		bool bAudibleFromFather = bAudibleLikeVisible ? bVisibleFromFather : (nAccelType & 256) != 0;

		if ((nAccelType & 3) == 2) // cluster
		if (!isZone(*pNode))
		if (isMesh(*pNode))
		{
			NL3D::CCluster clusterTemp;
			std::string temp;

			temp = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_SOUND_GROUP, "no sound");
			clusterTemp.setSoundGroup(temp != "no sound" ? temp : "");
			temp = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_ENV_FX, "no fx");
			clusterTemp.setEnvironmentFx(temp != "no fx" ? temp : "");

			SMeshData mesh;
			if (nodeWorldMesh(*pNode, tmCache, mesh))
			{
				for (uint f = 0; f < mesh.Tris.size(); ++f)
				{
					if (!clusterTemp.makeVolume(mesh.Vertices[mesh.Tris[f].A],
					                            mesh.Vertices[mesh.Tris[f].B],
					                            mesh.Vertices[mesh.Tris[f].C]))
					{
						fprintf(stderr, "ERROR: The cluster %s is not convex.\n", ucstring(pNode->userName()).toUtf8().c_str());
					}
				}
			}

			clusterTemp.FatherVisible = bFatherVisible;
			clusterTemp.VisibleFromFather = bVisibleFromFather;
			clusterTemp.FatherAudible = bFatherAudible;
			clusterTemp.AudibleFromFather = bAudibleFromFather;
			clusterTemp.Name = ucstring(pNode->userName()).toUtf8();

			vClusters.push_back(clusterTemp);
		}
	}

	// Creation of all the portals
	std::vector<NL3D::CPortal> vPortals;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		CNodeImpl *pNodeImpl = dynamic_cast<CNodeImpl *>(pNode);
		int nAccelType = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType & 3) == 1) // portal
		if (!isZone(*pNode))
		if (isMesh(*pNode))
		{
			NL3D::CPortal portalTemp;
			std::string temp;

			temp = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_OCC_MODEL, "no occlusion");
			portalTemp.setOcclusionModel(temp != "no occlusion" ? temp : "");
			temp = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_OPEN_OCC_MODEL, "no occlusion");
			portalTemp.setOpenOcclusionModel(temp != "no occlusion" ? temp : "");

			SMeshData mesh;
			if (nodeWorldMesh(*pNode, tmCache, mesh) && !mesh.Tris.empty())
			{
				// Stitch the faces into one ordered polygon (the reference's edge-walk loop).
				std::vector<sint32> poly;
				std::vector<bool> facechecked(mesh.Tris.size(), false);
				poly.push_back(mesh.Tris[0].A);
				poly.push_back(mesh.Tris[0].B);
				poly.push_back(mesh.Tris[0].C);
				facechecked[0] = true;
				for (uint32 f = 0; f < mesh.Tris.size(); ++f)
				if (!facechecked[f])
				{
					uint32 corner[3] = { mesh.Tris[f].A, mesh.Tris[f].B, mesh.Tris[f].C };
					bool found = false;
					uint32 k = 0, m = 0;
					for (k = 0; k < 3; ++k)
					{
						for (m = 0; m < poly.size(); ++m)
						{
							if (((sint32)corner[k] == poly[m] && (sint32)corner[(k + 1) % 3] == poly[(m + 1) % poly.size()]) ||
							    ((sint32)corner[(k + 1) % 3] == poly[m] && (sint32)corner[k] == poly[(m + 1) % poly.size()]))
							{
								found = true;
								break;
							}
						}
						if (found)
							break;
					}
					if (found)
					{
						poly.resize(poly.size() + 1);
						for (uint32 a = (uint32)poly.size() - 2; a > m; --a)
							poly[a + 1] = poly[a];
						poly[m + 1] = corner[(k + 2) % 3];
						facechecked[f] = true;
						f = 0;
						// the reference loop restarts via j=0 then ++j; replicate by continuing
						// from face 1 next iteration (the for's ++ runs after this)
						f = (uint32)-1;
					}
				}
				std::vector<NLMISC::CVector> polyv(poly.size());
				for (uint32 v = 0; v < poly.size(); ++v)
					polyv[v] = mesh.Vertices[poly[v]];

				if (!portalTemp.setPoly(polyv))
				{
					fprintf(stderr, "ERROR: The portal %s is not convex.\n", ucstring(pNode->userName()).toUtf8().c_str());
				}

				if (nAccelType & 16) // dynamic portal
				{
					std::string instanceName = getScriptAppDataStr(pNodeImpl, NEL3D_APPDATA_INSTANCE_NAME, "");
					if (!instanceName.empty())
						portalTemp.setName(instanceName);
					else
						portalTemp.setName(ucstring(pNode->userName()).toUtf8());
				}

				// Check if portal has 2 cluster
				sint nNbCluster = 0;
				for (uint32 c = 0; c < vClusters.size(); ++c)
				{
					bool bPortalInCluster = true;
					for (uint32 v = 0; v < polyv.size(); ++v)
						if (!vClusters[c].isIn(polyv[v]))
						{
							bPortalInCluster = false;
							break;
						}
					if (bPortalInCluster)
						++nNbCluster;
				}
				if (nNbCluster != 2)
				{
					fprintf(stderr, "ERROR: The portal %s has not 2 clusters but %d\n",
					        ucstring(pNode->userName()).toUtf8().c_str(), nNbCluster);
				}
			}

			vPortals.push_back(portalTemp);
		}
	}

	// Link instances to clusters
	nNumIG = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];
		CNodeImpl *pNodeImpl = dynamic_cast<CNodeImpl *>(pNode);
		int nAccelType = getScriptAppDataInt(pNodeImpl, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType & 3) == 0)
		if (!isZone(*pNode))
		if (isMesh(*pNode) || isDummy(*pNode))
		{
			if (nAccelType & 32) // clusterize flag
			{
				// The vertices tested against the clusters: FX instances use the .ps shape's
				// world-transformed AABBox corners; everything else uses the node's mesh in
				// world space (PS placeholder mesh as fallback when the shape is unresolvable,
				// like the reference).
				SMeshData mesh;
				CSceneClass *baseObj = baseObjectOf(*pNode);
				bool haveVerts = false;
				if (objIsParticleSystem(baseObj))
				{
					haveVerts = psShapeBBoxVerts(*pNode, baseObj, tmCache, mesh.Vertices);
					if (!haveVerts)
						fprintf(stderr, "ERROR: Can't get bbox of a particle system from its shape, using helper bbox instead\n");
				}
				if (!haveVerts)
					haveVerts = nodeWorldMesh(*pNode, tmCache, mesh);
				if (haveVerts)
				{
					for (uint32 c = 0; c < vClusters.size(); ++c)
					{
						bool bMeshInCluster = false;
						for (uint32 v = 0; v < mesh.Vertices.size(); ++v)
						{
							if (vClusters[c].isIn(mesh.Vertices[v]))
							{
								bMeshInCluster = true;
								break;
							}
						}
						if (bMeshInCluster)
							aIGArray[nNumIG].Clusters.push_back(c);
					}
				}
				if (!vClusters.empty() && aIGArray[nNumIG].Clusters.empty())
				{
					fprintf(stderr, "ERROR: Object %s is not attached to any cluster but his flag clusterize is set\n",
					        ucstring(pNode->userName()).toUtf8().c_str());
				}
			}
			++nNumIG;
		}
	}

	// PointLight part
	std::vector<NL3D::CPointLightNamed> pointLights;
	bool sunLightEnabled = false;
	pointLights.resize(vectNode.size());
	sint nNumPointLight = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		if (convertMaxLight(pointLights[nNumPointLight], *vectNode[i], tmCache, sunLightEnabled))
			++nNumPointLight;
	}
	pointLights.resize(nNumPointLight);

	NL3D::CInstanceGroup *pIG = new NL3D::CInstanceGroup;
	pIG->build(NLMISC::CVector(0, 0, 0), aIGArray, vClusters, vPortals, pointLights);
	pIG->enableRealTimeSunContribution(sunLightEnabled);
	return pIG;
}

// Depth-first pre-order index by tree walk from the scene root; children of a given parent are
// visited in their storage (creation) order — matches MaxScript's `$geometry`/`$lights`/
// `$helpers` iteration, which walks the scene tree from root DFS. Storage-container order
// coincides with tree order on most corpus files but diverges on a subset (desert `nb01..nb05`,
// jungle `foret-18..21_village_a/b/c/d`, some `ilot_butte`) where children of two sibling
// parents are interleaved in file order; the tree walk pins the per-category iteration to what
// the reference `$geometry` enumerator sees.
static void buildTreeOrder(CSceneClassContainer *ssc, INode *root, std::map<INode *, int> &orderMap)
{
	// Build parent -> ordered children[] (in storage order); collect every CNodeImpl.
	std::map<INode *, std::vector<INode *> > kids;
	std::vector<INode *> allNodes;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (!n) continue;
		allNodes.push_back(n);
		kids[n->parent()].push_back(n);
	}
	// DFS pre-order. Seed with root's children (and any NULL-parent orphans, defensive) in
	// storage order — push in reverse so the top of the stack is the first child.
	std::vector<INode *> stack;
	INode *seeds[2] = { root, NULL };
	for (int s = 0; s < 2; ++s)
	{
		std::map<INode *, std::vector<INode *> >::iterator it = kids.find(seeds[s]);
		if (it == kids.end()) continue;
		for (std::vector<INode *>::const_reverse_iterator ki = it->second.rbegin(); ki != it->second.rend(); ++ki)
			stack.push_back(*ki);
	}
	int idx = 0;
	while (!stack.empty())
	{
		INode *n = stack.back();
		stack.pop_back();
		if (orderMap.count(n)) continue; // parent-loop or duplicate defense
		orderMap[n] = idx++;
		std::map<INode *, std::vector<INode *> >::iterator it = kids.find(n);
		if (it == kids.end()) continue;
		for (std::vector<INode *>::const_reverse_iterator ki = it->second.rbegin(); ki != it->second.rend(); ++ki)
			stack.push_back(*ki);
	}
	// Any node the tree walk didn't reach (isolated parents, broken hierarchies): append in
	// storage order after the reachable set. Doesn't affect corpus files (every corpus node is
	// tree-reachable) but keeps the map total.
	for (uint i = 0; i < allNodes.size(); ++i)
		if (!orderMap.count(allNodes[i]))
			orderMap[allNodes[i]] = idx++;
}

static int treeOrderOf(const std::map<INode *, int> &orderMap, INode *n)
{
	std::map<INode *, int>::const_iterator it = orderMap.find(n);
	return it == orderMap.end() ? (int)0x7fffffff : it->second;
}

// Select nodes (geometry, then lights, then helpers — the maxscript's three selectmore passes)
// whose ig name appdata matches igNameMatch, build the instance group, or return NULL when
// nothing matched (the maxscript's "ig_array.count != 0" gate is implicit: an empty selection
// yields no output file rather than an error). When transitionZone >= 0, each matched node's
// world TM is overridden in tmCache via buildTransitionMatrixObj before the build — this mirrors
// the maxscript literally overwriting node.transform for the SAME nodes it just selected.
//
// includeXRefFirst=true replicates the ligo maxscript's `exportInstanceGroupFromZone` extra
// XRef pre-pass (`for node in objects where classOf node == XRefObject`) that adds XRef-object
// nodes in scene-tree order BEFORE the three geometry/lights/helpers passes; the ligo maxscript
// treats `$selection as array` as returning in `selectmore` insertion order (empirically
// confirmed by corpus regression: enabling the XRef pass drops the ligo diff count from ~88
// to ~29 in one commit, disabling it swings the same delta back — Max's `$selection` iterator
// isn't documented as insertion-vs-scene ordered so we replicate the observed behavior). The
// standalone `processes/ig` maxscript doesn't have this XRef pass, so the standalone caller
// passes false and the selection is purely the three per-category tree-ordered walks.
static NL3D::CInstanceGroup *exportIgForName(CSceneClassContainer *ssc, SNodeTMCache &tmCache,
                                              const std::string &igNameMatch, bool lowercaseCompare,
                                              int transitionZone, float cellSize, SIgBuildStats &stats,
                                              bool includeXRefFirst)
{
	std::string want = lowercaseCompare ? NLMISC::toLowerAscii(igNameMatch) : igNameMatch;
	std::map<INode *, int> treeOrder;
	buildTreeOrder(ssc, tmCache.SceneRoot, treeOrder);

	std::vector<INode *> vectNode;
	std::set<INode *> picked;

	// Ligo-only XRef-first pass. The maxscript filter is (per Kaetemi's Max 9
	// gen_ig_selorder_probe.ms run on `zonematerial-desert-nb01.max`/`fy_module_village_nb_01`):
	//   for node in objects where classOf node == XRefObject do (
	//     sourceObject = node.GetSourceObject false   -- SINGLE-STEP resolve, no recursion
	//     if (classOf sourceObject == XRefObject) then FAIL   -- nested XRef → skipped
	//     else if (superclassOf sourceObject in {Geom, Helper, Light}) then selectmore
	//     -- anything else (e.g. sClass=shape 0x40) → dropped, no selectmore branch fires
	//   )
	// Not the ordering-inert catch-all it looked like — probing this specific file exposed both
	// exclusion cases: 5 "ascenseur" XRefs whose source is a chained XRefObject (not resolved
	// further by the maxscript, ours single-step resolve returns them as such), plus
	// `fy_module_col_nb01` whose source is a `SplineShape`/sClass=shape 0x40 (none of the three
	// `selectmore` branches fire). Both were being incorrectly added to our pre-pass before this
	// fix — see §10w for the full ligo diff-count drop and the 23 village-bundle-file class.
	if (includeXRefFirst)
	{
		std::vector<INode *> xrefNodes;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
			if (!node) continue;
			CSceneClass *directObj = dynamic_cast<CSceneClass *>(node->getReference(1));
			if (!directObj || directObj->classDesc()->classId().a() != 0x92aab38c) continue;
			std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
			if (lowercaseCompare) ig = NLMISC::toLowerAscii(ig);
			if (ig != want) continue;

			// Single-step XRef source resolution. The maxscript's `GetSourceObject false` never
			// unwraps a nested XRef — it returns the direct target object as stored on the XRef,
			// which may itself be an XRefObject if the source .max chains a reference.
			// resolveXRefObject() by itself would recurse through baseObjectOfObj at the end,
			// so we call it and then peel back one step: what we want is the OBJECT REFERENCE of
			// the resolved source node, not its fully-unwrapped base — inline the same loader
			// path resolveXRefObject uses and stop at `node->getReference(1)`.
			CSceneClass *source = NULL;
			{
				CStorageContainer *rec170 = NULL;
				const CStorageContainer::TStorageObjectContainer &orphans = directObj->orphanedChunks();
				for (CStorageContainer::TStorageObjectConstIt oi = orphans.begin(); oi != orphans.end(); ++oi)
					if (oi->first == 0x0170) { rec170 = dynamic_cast<CStorageContainer *>(oi->second); break; }
				std::string srcFile, srcObj, srcOnDisk;
				SLoadedMax *lm = NULL;
				if (rec170 &&
				    xrefChildString(rec170, 0x0100, srcFile) &&
				    xrefChildString(rec170, 0x0110, srcObj) &&
				    DBPATH::resolve(srcFile, srcOnDisk))
					lm = loadMaxFileCached(srcOnDisk);
				if (lm)
				{
					CSceneClassContainer *sub = lm->Scene->container();
					std::string wantLower = NLMISC::toLowerAscii(srcObj);
					for (CStorageContainer::TStorageObjectConstIt si = sub->chunks().begin(); si != sub->chunks().end(); ++si)
					{
						CNodeImpl *n2 = dynamic_cast<CNodeImpl *>(si->second);
						if (!n2) continue;
						if (NLMISC::toLowerAscii(ucstring(n2->userName()).toUtf8()) != wantLower) continue;
						source = dynamic_cast<CSceneClass *>(n2->getReference(1));
						break;
					}
				}
			}
			if (!source) continue; // unresolvable → maxscript's `sourceObject == undefined` fall-through
			// Nested-XRef check: maxscript raises "FAIL XREF STILL XREF" and skips.
			if (source->classDesc()->classId().a() == 0x92aab38c) continue;
			// Category filter: only Geom/Helper/Light superclasses have a matching `selectmore`
			// branch in the maxscript. Everything else (Shape 0x40, Camera 0x20, ...) falls
			// through unadded. UNWRAP OSM/WSM derived-object wrappers first — the maxscript's
			// `superclassOf sourceObject` sees the base-of-derived, not the wrapper.
			CSceneClass *unwrapped = source;
			for (int guard = 0; unwrapped && guard < 8; ++guard)
			{
				NLMISC::CClassId cid = unwrapped->classDesc()->classId();
				if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
				CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(unwrapped);
				CSceneClass *base = NULL;
				for (uint i = 0; rm && i < rm->nbReferences(); ++i)
				{
					CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
					if (!r) continue;
					TSClassId scid_i = r->classDesc()->superClassId();
					if (scid_i == SCLASS_OSMODIFIER || scid_i == SCLASS_WSMODIFIER) continue;
					base = r;
				}
				if (!base) break;
				unwrapped = base;
			}
			TSClassId scid = unwrapped ? unwrapped->classDesc()->superClassId() : (TSClassId)0;
			if (scid != SCLASS_GEOMOBJECT && scid != SCLASS_HELPER && scid != SCLASS_LIGHT) continue;

			xrefNodes.push_back(node);
		}
		// Sort by tree order; the maxscript's `for node in objects` uses the scene walk.
		for (uint a = 0; a < xrefNodes.size(); ++a)
			for (uint b = a + 1; b < xrefNodes.size(); ++b)
				if (treeOrderOf(treeOrder, xrefNodes[a]) > treeOrderOf(treeOrder, xrefNodes[b]))
					std::swap(xrefNodes[a], xrefNodes[b]);
		for (uint i = 0; i < xrefNodes.size(); ++i)
		{
			vectNode.push_back(xrefNodes[i]);
			picked.insert(xrefNodes[i]);
		}
	}

	static const TSClassId cats[3] = { SCLASS_GEOMOBJECT, SCLASS_LIGHT, SCLASS_HELPER };
	for (int c = 0; c < 3; ++c)
	{
		std::vector<INode *> catNodes;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
			if (!node) continue;
			if (picked.count(node)) continue; // already added by the XRef-first pass
			if (nodeCategory(*node) != cats[c]) continue;
			std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
			if (lowercaseCompare) ig = NLMISC::toLowerAscii(ig);
			if (ig != want) continue;
			catNodes.push_back(node);
		}
		// Sort matched nodes in tree-walk order (matches Max's per-category collection order).
		for (uint a = 0; a < catNodes.size(); ++a)
			for (uint b = a + 1; b < catNodes.size(); ++b)
				if (treeOrderOf(treeOrder, catNodes[a]) > treeOrderOf(treeOrder, catNodes[b]))
					std::swap(catNodes[a], catNodes[b]);
		for (uint i = 0; i < catNodes.size(); ++i)
		{
			vectNode.push_back(catNodes[i]);
			picked.insert(catNodes[i]);
		}
	}

	if (vectNode.empty()) return NULL;

	if (transitionZone >= 0)
	{
		for (uint i = 0; i < vectNode.size(); ++i)
		{
			Matrix3M orig = getNodeTM(vectNode[i], tmCache);
			tmCache.TM[vectNode[i]] = buildTransitionMatrixObj(orig, transitionZone, cellSize);
		}
	}

	return buildInstanceGroup(vectNode, tmCache, stats);
}

// ---------------------------------------------------------------------------------------------
// Ligo brick file classification and export (see the ligo brick export block above for the
// per-mode contract). Returns 0 on success (including "nothing to export"), 1 on error — a
// tagThisFile=false in the maxscript. Writes one <name>.ig per exported instance group directly
// into outDir (no "igs" subdirectory: outDir already denotes the igs output for the caller, same
// convention as the standalone per-file mode).
static int exportLigoIg(CSceneClassContainer *ssc, SNodeTMCache &tmCache, const std::string &inputBase,
                         const std::string &outDir, float cellSize)
{
	std::vector<std::string> tokens;
	{
		size_t start = 0;
		for (size_t i = 0; i <= inputBase.size(); ++i)
		{
			if (i == inputBase.size() || inputBase[i] == '-')
			{
				tokens.push_back(inputBase.substr(start, i - start));
				start = i + 1;
			}
		}
	}

	SIgBuildStats stats;
	int ret = 0;

	if ((tokens.size() == 3 && tokens[0] == "zonematerial") || (tokens.size() == 2 && tokens[0] == "zonespecial"))
	{
		// igName == "" in the maxscript: every distinct (lowercased) ig name in the file.
		std::vector<std::string> igNames;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
			if (!node) continue;
			std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
			if (ig.empty()) continue;
			ig = NLMISC::toLowerAscii(ig);
			if (std::find(igNames.begin(), igNames.end(), ig) == igNames.end())
				igNames.push_back(ig);
		}
		for (uint i = 0; i < igNames.size(); ++i)
		{
			// igNames[i] is already lowercased, but a matching node's OWN igname appdata may
			// carry any casing (the corpus mixes "converted-164_eg" and "converted-164_EG"
			// tags on nodes that belong to the same ig) — lowercaseCompare must stay true so
			// exportIgForName lowercases each candidate node's igname before comparing.
			NL3D::CInstanceGroup *ig = exportIgForName(ssc, tmCache, igNames[i], /*lowercaseCompare*/ true, -1, cellSize, stats, /*includeXRefFirst*/ true);
			if (!ig) continue;
			std::string outPath = NLMISC::CPath::standardizePath(outDir, true) + igNames[i] + ".ig";
			try
			{
				NLMISC::COFile file;
				if (!file.open(outPath)) { std::cerr << "ERROR: cannot open output " << outPath << "\n"; delete ig; return 1; }
				ig->serial(file);
				file.close();
				if (g_verbose) printf("OK %s (%u instances)\n", outPath.c_str(), ig->getNumInstance());
			}
			catch (const NLMISC::Exception &e)
			{
				std::cerr << "ERROR: serial failed for " << outPath << ": " << e.what() << "\n";
				ret = 1;
			}
			delete ig;
		}
		return ret;
	}
	else if (tokens.size() == 4 && tokens[0] == "zonetransition")
	{
		std::string err;
		if (!classifyTransitionGrid(ssc, tmCache, cellSize, err))
		{
			std::cerr << "ERROR: " << inputBase << ": " << err << "\n";
			return 1;
		}
		for (int zone = 0; zone < 9; ++zone)
		{
			std::string zoneBaseName = tokens[1] + "-" + tokens[2] + "-" + tokens[3] + "-" + NLMISC::toString(zone);
			NL3D::CInstanceGroup *ig = exportIgForName(ssc, tmCache, zoneBaseName, /*lowercaseCompare*/ true, zone, cellSize, stats, /*includeXRefFirst*/ true);
			if (!ig) continue;
			std::string outPath = NLMISC::CPath::standardizePath(outDir, true) + NLMISC::toLowerAscii(zoneBaseName) + ".ig";
			try
			{
				NLMISC::COFile file;
				if (!file.open(outPath)) { std::cerr << "ERROR: cannot open output " << outPath << "\n"; delete ig; return 1; }
				ig->serial(file);
				file.close();
				if (g_verbose) printf("OK %s (%u instances)\n", outPath.c_str(), ig->getNumInstance());
			}
			catch (const NLMISC::Exception &e)
			{
				std::cerr << "ERROR: serial failed for " << outPath << ": " << e.what() << "\n";
				ret = 1;
			}
			delete ig;
		}
		return ret;
	}
	// Not a ligo brick filename (e.g. a debug/scratch file in the ligo source dir): nothing to do.
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Debug dump of the per-node classification.

static const char *g_dumpObjName = NULL;
static const char *g_dumpLightName = NULL;
static void dumpLightNode(CNodeImpl *node);

static void dumpNodes(CSceneClassContainer *ssc, SNodeTMCache &tmCache)
{
	if (g_dumpLightName)
	{
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
			if (!node) continue;
			if (std::string(g_dumpLightName) != "*" && ucstring(node->userName()).toUtf8() != g_dumpLightName) continue;
			if (std::string(g_dumpLightName) == "*" && nodeCategory(*node) != SCLASS_LIGHT) continue;
			dumpLightNode(node);
		}
		return;
	}
	if (g_dumpObjName)
	{
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
			if (!node) continue;
			if (ucstring(node->userName()).toUtf8() != g_dumpObjName) continue;
			{
				printf("=== node '%s' own chunks:\n", g_dumpObjName);
				std::stringstream ssn;
				node->toString(ssn);
				printf("%s\n", ssn.str().c_str());
			}
			CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
			printf("=== node '%s' direct object:\n", g_dumpObjName);
			if (obj)
			{
				std::stringstream ss;
				obj->toString(ss);
				printf("%s\n", ss.str().c_str());
				CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
				for (uint i = 0; rm && i < rm->nbReferences(); ++i)
				{
					CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
					printf("--- ref %u: %s\n", i, r ? r->classDesc()->classId().toString().c_str() : "null");
					if (r)
					{
						std::stringstream ss2;
						r->toString(ss2);
						printf("%s\n", ss2.str().c_str());
					}
				}
			}
			return;
		}
		printf("node '%s' not found\n", g_dumpObjName);
		return;
	}
	std::map<INode *, int> treeOrder;
	buildTreeOrder(ssc, tmCache.SceneRoot, treeOrder);
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		CSceneClass *obj = baseObjectOf(*node);
		bool flagsFound = false;
		uint32 flags = readNodeDword(node, NODE_FLAGS_CHUNK_ID, flagsFound);
		std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
		Matrix3M tm = getNodeTM(node, tmCache);
		INode *parent = node->parent();
		std::string parentName = parent && dynamic_cast<CNodeImpl *>(parent) ? ucstring(dynamic_cast<CNodeImpl *>(parent)->userName()).toUtf8() : std::string("<root>");
		printf("node '%s' tree=%d parent='%s' ig='%s' cat=0x%x obj=%s (%s) accel=%d flags=0x%08x%s shape='%s' pos=(%g,%g,%g)\n",
		       ucstring(node->userName()).toUtf8().c_str(),
		       treeOrderOf(treeOrder, node),
		       parentName.c_str(),
		       ig.c_str(),
		       obj ? (uint32)obj->classDesc()->superClassId() : 0,
		       obj ? obj->classDesc()->classId().toString().c_str() : "none",
		       obj ? obj->className().c_str() : "-",
		       getScriptAppDataInt(node, NEL3D_APPDATA_ACCEL, 32),
		       flags, flagsFound ? "" : " (noflags)",
		       getNelObjectName(*node).c_str(),
		       tm.m[3][0], tm.m[3][1], tm.m[3][2]);
	}
}

// ---------------------------------------------------------------------------------------------
// .ig comparison (T3 structural helper): load two igs through NL3D and compare field by field.

static bool quatEq(const NLMISC::CQuat &a, const NLMISC::CQuat &b, float eps)
{
	float d1 = std::max(std::max(fabsf(a.x - b.x), fabsf(a.y - b.y)), std::max(fabsf(a.z - b.z), fabsf(a.w - b.w)));
	float d2 = std::max(std::max(fabsf(a.x + b.x), fabsf(a.y + b.y)), std::max(fabsf(a.z + b.z), fabsf(a.w + b.w)));
	return std::min(d1, d2) <= eps;
}

static int compareIgs(const char *pathA, const char *pathB, bool maskLighting, bool maskZ, bool maskUninit)
{
	NL3D::CInstanceGroup igA, igB;
	try
	{
		NLMISC::CIFile fa;
		if (!fa.open(pathA)) { fprintf(stderr, "ERROR: cannot open %s\n", pathA); return 1; }
		igA.serial(fa);
		NLMISC::CIFile fb;
		if (!fb.open(pathB)) { fprintf(stderr, "ERROR: cannot open %s\n", pathB); return 1; }
		igB.serial(fb);
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: serial failed: %s\n", e.what());
		return 1;
	}

	int fails = 0;
	const float EPS = 1e-5f;
	// Positions compare with a magnitude-aware epsilon: the reference exporter's x87 matrix
	// arithmetic leaves +-1-2 ULP noise on large world coordinates (the sky-dome moon anchors at
	// |pos| ~ 200 differ by exactly 1 ULP); ~2.5 ULP allowance, floor 1e-5.
	#define POS_EPS(v) (std::max(EPS, fabsf(v) * 3e-7f))

	if (igA.getNumInstance() != igB.getNumInstance())
	{
		printf("DIFF numInstance %u vs %u\n", igA.getNumInstance(), igB.getNumInstance());
		++fails;
	}
	uint n = std::min(igA.getNumInstance(), igB.getNumInstance());
	for (uint i = 0; i < n; ++i)
	{
		const NL3D::CInstanceGroup::CInstance &a = igA.getInstance(i);
		const NL3D::CInstanceGroup::CInstance &b = igB.getInstance(i);
		std::string diffs;
		if (NLMISC::toLowerAscii(a.Name) != NLMISC::toLowerAscii(b.Name)) diffs += NLMISC::toString(" Name('%s' vs '%s')", a.Name.c_str(), b.Name.c_str());
		if (a.InstanceName != b.InstanceName) diffs += NLMISC::toString(" InstanceName('%s' vs '%s')", a.InstanceName.c_str(), b.InstanceName.c_str());
		if (a.nParent != b.nParent) diffs += NLMISC::toString(" nParent(%d vs %d)", a.nParent, b.nParent);
		if (fabsf(a.Pos.x - b.Pos.x) > POS_EPS(b.Pos.x) || fabsf(a.Pos.y - b.Pos.y) > POS_EPS(b.Pos.y) || (!maskZ && fabsf(a.Pos.z - b.Pos.z) > POS_EPS(b.Pos.z)))
			diffs += NLMISC::toString(" Pos((%g,%g,%g) vs (%g,%g,%g))", a.Pos.x, a.Pos.y, a.Pos.z, b.Pos.x, b.Pos.y, b.Pos.z);
		if (!quatEq(a.Rot, b.Rot, EPS))
			diffs += NLMISC::toString(" Rot((%g,%g,%g,%g) vs (%g,%g,%g,%g))", a.Rot.x, a.Rot.y, a.Rot.z, a.Rot.w, b.Rot.x, b.Rot.y, b.Rot.z, b.Rot.w);
		if (fabsf(a.Scale.x - b.Scale.x) > EPS || fabsf(a.Scale.y - b.Scale.y) > EPS || fabsf(a.Scale.z - b.Scale.z) > EPS)
			diffs += NLMISC::toString(" Scale((%g,%g,%g) vs (%g,%g,%g))", a.Scale.x, a.Scale.y, a.Scale.z, b.Scale.x, b.Scale.y, b.Scale.z);
		if (a.DontAddToScene != b.DontAddToScene) diffs += " DontAddToScene";
		if (a.Visible != b.Visible) diffs += " Visible";
		if (a.DontCastShadow != b.DontCastShadow) diffs += NLMISC::toString(" DontCastShadow(%d vs %d)", (int)a.DontCastShadow, (int)b.DontCastShadow);
		if (a.DontCastShadowForInterior != b.DontCastShadowForInterior) diffs += " DontCastShadowForInterior";
		if (a.DontCastShadowForExterior != b.DontCastShadowForExterior) diffs += " DontCastShadowForExterior";
		if (a.Clusters != b.Clusters) diffs += NLMISC::toString(" Clusters(%u vs %u entries)", (uint)a.Clusters.size(), (uint)b.Clusters.size());
		if (!maskLighting)
		{
			if (a.AvoidStaticLightPreCompute != b.AvoidStaticLightPreCompute) diffs += " AvoidStaticLightPreCompute";
			if (a.StaticLightEnabled != b.StaticLightEnabled) diffs += " StaticLightEnabled";
			// SunContribution and Light[] are uninitialized memory in the reference exporter's
			// raw output (CInstance ctor doesn't set them; see pipeline_max_design.md whitelist).
			if (!maskUninit)
			{
				if (a.SunContribution != b.SunContribution) diffs += " SunContribution";
				if (a.Light[0] != b.Light[0] || a.Light[1] != b.Light[1]) diffs += " Light";
			}
			if (a.LocalAmbientId != b.LocalAmbientId) diffs += " LocalAmbientId";
		}
		if (!diffs.empty())
		{
			printf("DIFF instance %u '%s':%s\n", i, a.InstanceName.c_str(), diffs.c_str());
			++fails;
		}
	}

	// Point lights. maskLighting compares B's lights as a SUBSET of A's: the ig_lighter drops
	// lights that influence no instance and reorders survivors, so the processed references
	// carry a filtered list; every reference light must still exist in A with matching fields.
	if (maskLighting)
	{
		for (uint i = 0; i < igB.getNumPointLights(); ++i)
		{
			NL3D::CPointLightNamed &b = igB.getPointLightNamed(i);
			bool found = false;
			std::string nearest;
			for (uint j = 0; j < igA.getNumPointLights() && !found; ++j)
			{
				NL3D::CPointLightNamed &a = igA.getPointLightNamed(j);
				NLMISC::CVector dp = a.getPosition() - b.getPosition();
				if (maskZ) dp.z = 0.0f; // heightmap elevation moved the reference light
				if (dp.norm() > 1e-3f) continue;
				std::string diffs;
				if (a.AnimatedLight != b.AnimatedLight) diffs += " AnimatedLight";
				if (a.LightGroup != b.LightGroup) diffs += " LightGroup";
				if (a.getType() != b.getType()) diffs += NLMISC::toString(" Type(%d vs %d)", (int)a.getType(), (int)b.getType());
				if (a.getDefaultAmbient() != b.getDefaultAmbient()) diffs += " Ambient";
				if (a.getDefaultDiffuse() != b.getDefaultDiffuse()) diffs += " Diffuse";
				if (a.getDefaultSpecular() != b.getDefaultSpecular()) diffs += " Specular";
				if (fabsf(a.getAttenuationBegin() - b.getAttenuationBegin()) > EPS || fabsf(a.getAttenuationEnd() - b.getAttenuationEnd()) > EPS) diffs += " Attenuation";
				if (a.getType() == NL3D::CPointLight::SpotLight && b.getType() == NL3D::CPointLight::SpotLight)
				{
					if ((a.getSpotDirection() - b.getSpotDirection()).norm() > EPS) diffs += " SpotDirection";
					if (fabsf(a.getSpotAngleBegin() - b.getSpotAngleBegin()) > EPS || fabsf(a.getSpotAngleEnd() - b.getSpotAngleEnd()) > EPS) diffs += " SpotAngle";
				}
				if (a.getType() == NL3D::CPointLight::AmbientLight && b.getType() == NL3D::CPointLight::AmbientLight
				    && a.getAddAmbientWithSun() != b.getAddAmbientWithSun()) diffs += " AddAmbientWithSun";
				if (diffs.empty()) found = true;
				else nearest = diffs;
			}
			if (!found)
			{
				printf("DIFF ref pointlight %u pos=(%g,%g,%g) LightMissing%s\n", i,
				       b.getPosition().x, b.getPosition().y, b.getPosition().z,
				       nearest.empty() ? "" : (" nearest:" + nearest).c_str());
				++fails;
			}
		}
	}
	else
	{
		// Direct-tier: exact set of lights, but ORDER-TOLERANT within (AnimatedLight, LightGroup)
		// ties — CPointLightNamedArray::build sorts with std::sort, whose tie permutation is
		// STL-implementation-defined; the reference exporter's MSVC permutation is not
		// reproducible (documented as an accepted class in pipeline_max_design.md). Every light
		// must 1:1 match by full field equality.
		if (igA.getNumPointLights() != igB.getNumPointLights())
		{
			printf("DIFF numPointLights %u vs %u\n", igA.getNumPointLights(), igB.getNumPointLights());
			++fails;
		}
		std::vector<bool> used(igA.getNumPointLights(), false);
		for (uint i = 0; i < igB.getNumPointLights(); ++i)
		{
			NL3D::CPointLightNamed &b = igB.getPointLightNamed(i);
			bool found = false;
			std::string nearest;
			for (uint j = 0; j < igA.getNumPointLights() && !found; ++j)
			{
				if (used[j]) continue;
				NL3D::CPointLightNamed &a = igA.getPointLightNamed(j);
				if ((a.getPosition() - b.getPosition()).norm() > EPS * (1.0f + b.getPosition().norm())) continue;
				std::string diffs;
				if (a.AnimatedLight != b.AnimatedLight) diffs += " AnimatedLight";
				if (a.LightGroup != b.LightGroup) diffs += " LightGroup";
				if (a.getType() != b.getType()) diffs += NLMISC::toString(" Type(%d vs %d)", (int)a.getType(), (int)b.getType());
				if (a.getDefaultAmbient() != b.getDefaultAmbient()) diffs += " Ambient";
				if (a.getDefaultDiffuse() != b.getDefaultDiffuse()) diffs += " Diffuse";
				if (a.getDefaultSpecular() != b.getDefaultSpecular()) diffs += " Specular";
				if (fabsf(a.getAttenuationBegin() - b.getAttenuationBegin()) > EPS || fabsf(a.getAttenuationEnd() - b.getAttenuationEnd()) > EPS) diffs += " Attenuation";
				if (a.getType() == NL3D::CPointLight::SpotLight && b.getType() == NL3D::CPointLight::SpotLight)
				{
					if ((a.getSpotDirection() - b.getSpotDirection()).norm() > EPS) diffs += " SpotDirection";
					if (fabsf(a.getSpotAngleBegin() - b.getSpotAngleBegin()) > EPS || fabsf(a.getSpotAngleEnd() - b.getSpotAngleEnd()) > EPS) diffs += " SpotAngle";
				}
				if (a.getType() == NL3D::CPointLight::AmbientLight && b.getType() == NL3D::CPointLight::AmbientLight
				    && a.getAddAmbientWithSun() != b.getAddAmbientWithSun()) diffs += " AddAmbientWithSun";
				if (diffs.empty()) { found = true; used[j] = true; }
				else nearest = diffs;
			}
			if (!found)
			{
				printf("DIFF pointlight %u pos=(%g,%g,%g) LightMissing%s\n", i,
				       b.getPosition().x, b.getPosition().y, b.getPosition().z,
				       nearest.empty() ? "" : (" nearest:" + nearest).c_str());
				++fails;
			}
		}
	}

	if (igA._ClusterInfos.size() != igB._ClusterInfos.size())
	{
		printf("DIFF numClusters %u vs %u\n", (uint)igA._ClusterInfos.size(), (uint)igB._ClusterInfos.size());
		++fails;
	}
	if (igA._Portals.size() != igB._Portals.size())
	{
		printf("DIFF numPortals %u vs %u\n", (uint)igA._Portals.size(), (uint)igB._Portals.size());
		++fails;
	}
	if (igA.getRealTimeSunContribution() != igB.getRealTimeSunContribution())
	{
		printf("DIFF realTimeSunContribution %d vs %d\n", (int)igA.getRealTimeSunContribution(), (int)igB.getRealTimeSunContribution());
		++fails;
	}

	if (!fails) printf("MATCH\n");
	return fails ? 2 : 0;
}

// Compact dump of a light node's object words + param block values (decode workbench).
static void dumpLightNode(CNodeImpl *node)
{
	CSceneClass *obj = baseObjectOf(*node);
	if (!obj) { printf("no object\n"); return; }
	printf("light '%s' class=%s (%s)\n", ucstring(node->userName()).toUtf8().c_str(),
	       obj->classDesc()->classId().toString().c_str(), obj->className().c_str());
	const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() > 4 || raw->Value.empty()) continue;
		uint32 v = 0;
		memcpy(&v, nlVectorData(raw->Value), raw->Value.size());
		printf("  word 0x%04x = %u\n", it->first, v);
	}
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	for (uint r = 0; rm && r < rm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
		if (!ref || ref->classDesc()->superClassId() != 0x8) continue;
		printf("  pblock ref %u:\n", r);
		// Param entries: 0x0002 containers with 0x0003 index + 0x0100 (4B) / 0x0102 (12B) value.
		const CStorageContainer::TStorageObjectContainer &po = ref->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt it = po.begin(); it != po.end(); ++it)
		{
			if (it->first != 0x0002) continue;
			CStorageContainer *pc = dynamic_cast<CStorageContainer *>(it->second);
			if (!pc) continue;
			sint32 idx = -1;
			for (CStorageContainer::TStorageObjectConstIt cit = pc->chunks().begin(); cit != pc->chunks().end(); ++cit)
			{
				CStorageRaw *cr = dynamic_cast<CStorageRaw *>(cit->second);
				if (!cr) continue;
				if (cit->first == 0x0003 && cr->Value.size() == 4)
					memcpy(&idx, nlVectorData(cr->Value), 4);
				else if (cit->first == 0x0100 && cr->Value.size() == 4)
				{
					float f;
					memcpy(&f, nlVectorData(cr->Value), 4);
					printf("    param %d = %.9g\n", idx, f);
				}
				else if (cit->first == 0x0102 && cr->Value.size() == 12)
				{
					float f[3];
					memcpy(f, nlVectorData(cr->Value), 12);
					printf("    param %d = (%.9g, %.9g, %.9g)\n", idx, f[0], f[1], f[2]);
				}
				else if (cit->first != 0x0004 && cr->Value.size() == 4)
				{
					uint32 u;
					float f;
					memcpy(&u, nlVectorData(cr->Value), 4);
					memcpy(&f, nlVectorData(cr->Value), 4);
					printf("    param %d = int %u / float %.9g (chunk 0x%04x)\n", idx, u, f, cit->first);
				}
				else if (cit->first != 0x0004)
					printf("    param %d: chunk 0x%04x size %u\n", idx, cit->first, (uint)cr->Value.size());
			}
		}
	}
}

// Print an ig's instance fields (reference inspection).
static int infoIg(const char *path)
{
	NL3D::CInstanceGroup ig;
	try
	{
		NLMISC::CIFile f;
		if (!f.open(path)) { fprintf(stderr, "ERROR: cannot open %s\n", path); return 1; }
		ig.serial(f);
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: serial failed: %s\n", e.what());
		return 1;
	}
	for (uint i = 0; i < ig.getNumInstance(); ++i)
	{
		const NL3D::CInstanceGroup::CInstance &a = ig.getInstance(i);
		printf("%3u '%s' shape='%s' parent=%d pos=(%.9g,%.9g,%.9g) rot=(%.9g,%.9g,%.9g,%.9g) scale=(%.9g,%.9g,%.9g)\n"
		       "    dontAdd=%d visible=%d dontCast=%d dontCastInt=%d dontCastExt=%d avoidStatic=%d staticLight=%d sun=%u light=[%02x,%02x] amb=%02x clusters=%u\n",
		       i, a.InstanceName.c_str(), a.Name.c_str(), a.nParent,
		       a.Pos.x, a.Pos.y, a.Pos.z, a.Rot.x, a.Rot.y, a.Rot.z, a.Rot.w, a.Scale.x, a.Scale.y, a.Scale.z,
		       (int)a.DontAddToScene, (int)a.Visible, (int)a.DontCastShadow, (int)a.DontCastShadowForInterior, (int)a.DontCastShadowForExterior,
		       (int)a.AvoidStaticLightPreCompute, (int)a.StaticLightEnabled, a.SunContribution, a.Light[0], a.Light[1], a.LocalAmbientId,
		       (uint)a.Clusters.size());
		if (!a.Clusters.empty())
		{
			printf("    clusterIdx:");
			for (uint c = 0; c < a.Clusters.size(); ++c)
				printf(" %d", a.Clusters[c]);
			printf("\n");
		}
	}
	for (uint i = 0; i < ig.getNumPointLights(); ++i)
	{
		NL3D::CPointLightNamed &l = ig.getPointLightNamed(i);
		printf("L%3u type=%d pos=(%.9g,%.9g,%.9g) amb=(%d,%d,%d,%d) dif=(%d,%d,%d,%d) spec=(%d,%d,%d,%d) atten=(%.9g,%.9g) group=%u anim='%s'",
		       i, (int)l.getType(), l.getPosition().x, l.getPosition().y, l.getPosition().z,
		       l.getDefaultAmbient().R, l.getDefaultAmbient().G, l.getDefaultAmbient().B, l.getDefaultAmbient().A,
		       l.getDefaultDiffuse().R, l.getDefaultDiffuse().G, l.getDefaultDiffuse().B, l.getDefaultDiffuse().A,
		       l.getDefaultSpecular().R, l.getDefaultSpecular().G, l.getDefaultSpecular().B, l.getDefaultSpecular().A,
		       l.getAttenuationBegin(), l.getAttenuationEnd(), l.LightGroup, l.AnimatedLight.c_str());
		if (l.getType() == NL3D::CPointLight::SpotLight)
			printf(" spotDir=(%.9g,%.9g,%.9g) spotAI=%.9g spotAO=%.9g",
			       l.getSpotDirection().x, l.getSpotDirection().y, l.getSpotDirection().z,
			       l.getSpotAngleBegin(), l.getSpotAngleEnd());
		printf(" ambAddSun=%d\n", (int)l.getAddAmbientWithSun());
	}
	// Clusters: private plane list — decode through a serial roundtrip (version, Name,
	// _LocalVolume, _LocalBBox, flags, soundGroup, envFx).
	for (uint i = 0; i < ig._ClusterInfos.size(); ++i)
	{
		NL3D::CCluster &c = ig._ClusterInfos[i];
		NLMISC::CMemStream mem;
		c.serial(mem);
		mem.invert();
		uint8 ver = 0;
		mem.serial(ver);
		std::string name;
		if (ver >= 1) mem.serial(name);
		std::vector<NLMISC::CPlane> planes;
		mem.serialCont(planes);
		NLMISC::CAABBox bbox;
		mem.serial(bbox);
		printf("C%3u '%s' planes=%u fatherVis=%d visFromFather=%d sound='%s' fx='%s' bbox=(%g,%g,%g|%g,%g,%g)\n",
		       i, name.c_str(), (uint)planes.size(), (int)c.FatherVisible, (int)c.VisibleFromFather,
		       c.getSoundGroup().c_str(), c.getEnvironmentFx().c_str(),
		       bbox.getCenter().x, bbox.getCenter().y, bbox.getCenter().z,
		       bbox.getHalfSize().x, bbox.getHalfSize().y, bbox.getHalfSize().z);
		for (uint j = 0; j < planes.size(); ++j)
			printf("    plane %u: (%.9g, %.9g, %.9g, %.9g)\n", j, planes[j].a, planes[j].b, planes[j].c, planes[j].d);
	}
	for (uint i = 0; i < ig._Portals.size(); ++i)
	{
		NL3D::CPortal &pt = ig._Portals[i];
		std::vector<NLMISC::CVector> poly;
		pt.getPoly(poly);
		printf("P%3u '%s' verts=%u occ='%s' openOcc='%s'\n", i, pt.getName().c_str(), (uint)poly.size(),
		       pt.getOcclusionModel().c_str(), pt.getOpenOcclusionModel().c_str());
		for (uint j = 0; j < poly.size(); ++j)
			printf("    v %u: (%.9g, %.9g, %.9g)\n", j, poly[j].x, poly[j].y, poly[j].z);
	}
	printf("pointLights=%u clusters=%u portals=%u realTimeSun=%d globalPos=(%g,%g,%g)\n",
	       ig.getNumPointLights(), (uint)ig._ClusterInfos.size(), (uint)ig._Portals.size(),
	       (int)ig.getRealTimeSunContribution(), ig.getGlobalPos().x, ig.getGlobalPos().y, ig.getGlobalPos().z);
	return 0;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (!NLMISC::INelContext::isContextInitialised())
		new NLMISC::CApplicationContext();

	bool dump = false;
	std::string ligoOutDir;
	float cellSize = 160.0f;
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		std::string arg = argv[argi];
		if (arg == "--dump") { dump = true; ++argi; }
		else if (arg == "--dump-obj" && argi + 1 < argc) { dump = true; g_dumpObjName = argv[argi + 1]; argi += 2; }
		else if (arg == "--dump-light" && argi + 1 < argc) { dump = true; g_dumpLightName = argv[argi + 1]; argi += 2; }
		else if (arg == "--db" && argi + 1 < argc) { DBPATH::setDefaultRoot(argv[argi + 1]); argi += 2; }
		else if (arg == "--path-alias" && argi + 1 < argc)
		{
			// --path-alias <windows-prefix>=<root>, e.g. --path-alias "P:\old_graphics=/mnt/old"
			// for corpus content authored under a different drive/root than "R:\graphics\...".
			std::string kv = argv[argi + 1];
			std::string::size_type eq = kv.find('=');
			if (eq == std::string::npos)
				fprintf(stderr, "WARNING: --path-alias expects <prefix>=<root>, got '%s'\n", kv.c_str());
			else
				DBPATH::addAlias(kv.substr(0, eq), kv.substr(eq + 1));
			argi += 2;
		}
		else if (arg == "--ps-path" && argi + 1 < argc) { g_psSearchPaths.push_back(argv[argi + 1]); argi += 2; }
		else if (arg == "--ligo" && argi + 1 < argc) { ligoOutDir = argv[argi + 1]; argi += 2; }
		else if (arg == "--cellsize" && argi + 1 < argc) { NLMISC::fromString(argv[argi + 1], cellSize); argi += 2; }
		else if (arg == "--dump-prim" && argi + 2 < argc)
		{
			// --dump-prim <box|plane|cylinder|sphere> <p0> [p1 ...] — prints manifest-format
			// V/F lines (1-based) for prim_check.py validation against ~/prim_mesh_dataset.
			std::string kind = argv[argi + 1];
			std::map<sint32, SPBlockParam> params;
			for (int k = argi + 2; k < argc; ++k)
			{
				SPBlockParam prm;
				prm.IsPoint3 = false;
				std::string v = argv[k];
				prm.IsInt = v.find('.') == std::string::npos;
				prm.V[0] = (float)atof(v.c_str());
				prm.V[1] = prm.V[2] = 0.0f;
				prm.I = atoi(v.c_str());
				params[k - argi - 2] = prm;
			}
			NLMISC::CClassId cid;
			if (kind == "box") cid = NLMISC::CClassId(0x00000010, 0x00000000);
			else if (kind == "cylinder") cid = NLMISC::CClassId(0x00000012, 0x00000000);
			else if (kind == "sphere") cid = NLMISC::CClassId(0x00000011, 0x00000000);
			else if (kind == "plane") cid = NLMISC::CClassId(0x081f1dfc, 0x77566f65);
			else { fprintf(stderr, "unknown primitive kind\n"); return 1; }
			std::vector<NLMISC::CVector> verts;
			std::vector<SMeshTri> tris;
			if (!buildParametricMesh(cid, params, verts, tris)) return 1;
			printf("MESH\t%s\tverts\t%u\tfaces\t%u\n", kind.c_str(), (uint)verts.size(), (uint)tris.size());
			for (uint i = 0; i < verts.size(); ++i)
				printf("  V\t%u\t%.9g\t%.9g\t%.9g\n", i + 1, verts[i].x, verts[i].y, verts[i].z);
			for (uint i = 0; i < tris.size(); ++i)
				printf("  F\t%u\t%u\t%u\t%u\n", i + 1, tris[i].A + 1, tris[i].B + 1, tris[i].C + 1);
			return 0;
		}
		else if (arg == "--info" && argi + 1 < argc)
		{
			NL3D::registerSerial3d();
			return infoIg(argv[argi + 1]);
		}
		else if (arg == "--verbose") { g_verbose = true; ++argi; }
		else if (arg == "--compare" && argi + 2 < argc)
		{
			bool maskLighting = false, maskZ = false, maskUninit = false;
			for (int k = argi + 3; k < argc; ++k)
			{
				if (std::string(argv[k]) == "--mask-lighting") maskLighting = true;
				if (std::string(argv[k]) == "--mask-z") maskZ = true;
				if (std::string(argv[k]) == "--mask-uninit") maskUninit = true;
			}
			NL3D::registerSerial3d();
			return compareIgs(argv[argi + 1], argv[argi + 2], maskLighting, maskZ, maskUninit);
		}
		else break;
	}
	bool ligoMode = !ligoOutDir.empty();
	if (argc - argi < 1 || (argc - argi < 2 && !dump && !ligoMode))
	{
		std::cerr << "usage: pipeline_max_export_ig <input.max> <output_dir>\n";
		std::cerr << "       pipeline_max_export_ig --dump <input.max>\n";
		std::cerr << "       pipeline_max_export_ig --ligo <outdir> [--cellsize 160] <input.max>\n";
		std::cerr << "       pipeline_max_export_ig --compare <a.ig> <b.ig> [--mask-lighting] [--mask-z]\n";
		std::cerr << "--ligo: NeLLigoBuild-ig-from-zone protocol by input filename (zonematerial/\n";
		std::cerr << "        zonespecial: every distinct ig name, lowercased; zonetransition: one\n";
		std::cerr << "        ig per grid slot 0..8, repositioned) — see the ligo maxscript's\n";
		std::cerr << "        exportInstanceGroupFromZone.\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export\n";
		return 1;
	}
	const char *maxFile = argv[argi];
	const char *outDir = ligoMode ? ligoOutDir.c_str() : ((argc - argi >= 2) ? argv[argi + 1] : NULL);

	NL3D::registerSerial3d();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);
	g_registry = &reg;

	// Deduce the database root for XRef resolution when not passed: the ancestor directory of
	// the input that holds the database tree (contains "stuff").
	if (DBPATH::defaultRoot().empty())
	{
		std::string p = NLMISC::CPath::standardizePath(NLMISC::CPath::getFullPath(NLMISC::CFile::getPath(maxFile), false), false);
		while (!p.empty() && p != "/")
		{
			if (NLMISC::CFile::isDirectory(p + "/stuff") || NLMISC::CFile::isDirectory(p + "/Stuff"))
			{
				DBPATH::setDefaultRoot(p);
				break;
			}
			p = NLMISC::CFile::getPath(p);
			while (!p.empty() && p[p.size() - 1] == '/' && p != "/") p.resize(p.size() - 1);
		}
	}

	CStorageOleIn in;
	if (!in.open(maxFile)) { std::cerr << "ERROR: not an OLE compound file: " << maxFile << "\n"; return 1; }

	CDllDirectory dll;
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(b); dll.serial(st); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(b); cd.serial(st); }
	cd.parse(VersionUnknown);

	CScene scene(&reg, &dll, &cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(b); scene.serial(st); }
	scene.parse(VersionUnknown);

	CSceneClassContainer *ssc = scene.container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = ssc->scene()->rootNode();

	if (dump)
	{
		dumpNodes(ssc, tmCache);
		return 0;
	}

	if (ligoMode)
	{
		std::string inputBase = NLMISC::CFile::getFilenameWithoutExtension(maxFile);
		NLMISC::CFile::createDirectoryTree(outDir);
		return exportLigoIg(ssc, tmCache, inputBase, outDir, cellSize);
	}

	// --- Scan all objects for distinct ig names, in scene order (ig_export.ms).
	std::vector<std::string> igNames;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
		if (ig.empty()) continue;
		if (std::find(igNames.begin(), igNames.end(), ig) == igNames.end())
			igNames.push_back(ig);
	}

	if (igNames.empty())
	{
		std::cerr << "WARNING: nothing exported from ig max file " << maxFile << "\n";
		return 3;
	}

	int ret = 0;
	for (uint igIdx = 0; igIdx < igNames.size(); ++igIdx)
	{
		const std::string &igName = igNames[igIdx];

		// Selection: geometry, then lights, then helpers (selectmore passes; selection arrays
		// keep selection order). Reuses the shared tree-walk-ordered selector (the standalone
		// process/ig maxscript doesn't do the XRef-first pass — that lives in the ligo
		// exportInstanceGroupFromZone maxscript only).
		SIgBuildStats stats;
		NL3D::CInstanceGroup *ig = exportIgForName(ssc, tmCache, igName, /*lowercaseCompare*/ false,
		                                            /*transitionZone*/ -1, /*cellSize*/ 160.0f, stats,
		                                            /*includeXRefFirst*/ false);
		if (!ig) continue;

		std::string outPath = NLMISC::CPath::standardizePath(outDir, true) + igName + ".ig";
		try
		{
			NLMISC::COFile file;
			if (!file.open(outPath))
			{
				std::cerr << "ERROR: cannot open output " << outPath << "\n";
				delete ig;
				return 1;
			}
			ig->serial(file);
			file.close();
			if (g_verbose)
				printf("OK %s (%u instances)\n", outPath.c_str(), ig->getNumInstance());
		}
		catch (const NLMISC::Exception &e)
		{
			std::cerr << "ERROR: serial failed for " << outPath << ": " << e.what() << "\n";
			ret = 1;
		}
		delete ig;
	}

	return ret;
}

/* end of file */
