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

#include <nel/3d/register_3d.h>
#include <nel/3d/scene_group.h>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

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

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"

#include "max_math.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

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

// AppData script-entry key (the MaxScript utility panel writes these)
static const NLMISC::CClassId APPDATA_SCRIPT_CLASS_ID(0x04d64858, 0x16d1751d);
static const uint32 APPDATA_SCRIPT_SUPER_CLASS_ID = 4128;

// Scene class ids
static const NLMISC::CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);
static const NLMISC::CClassId CLASSID_LOOKAT_CTRL(0x00002006, 0x00000000);
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

// PRS sub-controller default-value chunk ids
#define CHUNK_CTRL_POS_VALUE 0x2503
#define CHUNK_CTRL_ROT_VALUE 0x2504
#define CHUNK_CTRL_SCALE_VALUE 0x2505

static bool g_verbose = false;
// Database root for XRef resolution (the ryzomcore_graphics checkout); deduced from the input
// path or passed via --db.
static std::string g_dbRoot;
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

// Case-insensitive path resolution under a root (the database checkout may differ in case from
// the authored Windows paths).
static bool resolvePathCI(const std::string &root, const std::string &relative, std::string &out)
{
	std::string cur = root;
	std::vector<std::string> parts;
	NLMISC::splitString(relative, "/", parts);
	for (uint i = 0; i < parts.size(); ++i)
	{
		if (parts[i].empty()) continue;
		std::string direct = cur + "/" + parts[i];
		if (NLMISC::CFile::fileExists(direct) || NLMISC::CFile::isDirectory(direct))
		{
			cur = direct;
			continue;
		}
		// scan for a case-insensitive match
		std::vector<std::string> contents;
		NLMISC::CPath::getPathContent(cur, false, true, true, contents);
		std::string want = NLMISC::toLower(parts[i]);
		bool found = false;
		for (uint j = 0; j < contents.size(); ++j)
		{
			std::string name = NLMISC::CFile::getFilename(NLMISC::CPath::standardizePath(contents[j], false));
			if (NLMISC::toLower(name) == want)
			{
				cur = cur + "/" + name;
				found = true;
				break;
			}
		}
		if (!found) return false;
	}
	out = cur;
	return true;
}

static SLoadedMax *loadMaxFileCached(const std::string &path)
{
	std::map<std::string, SLoadedMax>::iterator it = g_xrefScenes.find(path);
	if (it != g_xrefScenes.end()) return it->second.Scene ? &it->second : NULL;
	SLoadedMax &lm = g_xrefScenes[path]; // inserted empty: failure is cached too
	GsfInput *src = gsf_input_stdio_new(path.c_str(), NULL);
	if (!src) { fprintf(stderr, "WARNING: xref: cannot open %s\n", path.c_str()); return NULL; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	g_object_unref(src);
	if (!in) { fprintf(stderr, "WARNING: xref: not an OLE compound file: %s\n", path.c_str()); return NULL; }
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(g_registry, dll, cd);
	bool ok = true;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); if (s) { CStorageStream st(s); dll->serial(st); g_object_unref(s); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); if (s) { CStorageStream st(s); cd->serial(st); g_object_unref(s); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { GsfInput *s = gsf_infile_child_by_name(in, "Scene"); if (s) { CStorageStream st(s); scene->serial(st); g_object_unref(s); scene->parse(VersionUnknown); } else ok = false; }
	g_object_unref(in);
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
		if (!us.empty()) memcpy(&us[0], raw->Value.data(), us.size() * 2);
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

	// Authored path -> database-relative: strip the drive and the top-level "graphics"/"database"
	// component, then resolve case-insensitively under the database root.
	std::string rel = file;
	for (uint i = 0; i < rel.size(); ++i)
		if (rel[i] == '\\') rel[i] = '/';
	std::string::size_type colon = rel.find(':');
	if (colon != std::string::npos) rel = rel.substr(colon + 1);
	while (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
	{
		std::string::size_type slash = rel.find('/');
		if (slash != std::string::npos)
		{
			std::string first = NLMISC::toLower(rel.substr(0, slash));
			if (first == "graphics" || first == "database")
				rel = rel.substr(slash + 1);
		}
	}
	std::string resolved;
	if (g_dbRoot.empty() || !resolvePathCI(g_dbRoot, rel, resolved))
	{
		fprintf(stderr, "WARNING: xref: cannot resolve '%s' (relative '%s') under db root '%s'\n",
		        file.c_str(), rel.c_str(), g_dbRoot.c_str());
		return NULL;
	}

	SLoadedMax *lm = loadMaxFileCached(resolved);
	if (!lm) return NULL;

	// Find the named node in the referenced scene.
	CSceneClassContainer *ssc = lm->Scene->container();
	std::string wantLower = NLMISC::toLower(objName);
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (NLMISC::toLower(ucstring(node->userName()).toUtf8()) != wantLower) continue;
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
		memcpy(&id, raw->Value.data(), 2);
		if (id != paramId) continue;
		uint8 flag = raw->Value[14];
		if (!(flag & 0x40)) return false; // controller-backed, not a constant
		// Constant payload after the flag byte. Strings are u32-length-prefixed, then chars
		// (null included in the length in the observed corpus records).
		if (raw->Value.size() < 15 + 4) return false;
		uint32 len;
		memcpy(&len, raw->Value.data() + 15, 4);
		if (len > raw->Value.size() - 19) len = (uint32)(raw->Value.size() - 19);
		std::string s((const char *)raw->Value.data() + 19, len);
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
		memcpy(&fl, flags->Value.data(), 4);
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
			memcpy(&fl, raw->Value.data(), 4);
			found = true;
		}
		break;
	}
	return fl;
}

// ---------------------------------------------------------------------------------------------
// PRS controller values at t=0 (GetNodeTM(0) inputs). For keyed typed keyframers the value at
// tick 0 is the bracketing key's value (clamped before/after the key range); a t=0 that falls
// strictly between two keys warns and linearly interpolates (not hit in the ig corpus).

static bool readCtrlDefaultBytes(CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes)
{
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(sc);
	if (kf)
	{
		uint size = 0;
		const uint8 *data = kf->defaultValue(size);
		if (data && size >= nBytes)
		{
			memcpy(dst, data, nBytes);
			return true;
		}
	}
	if (!sc) return false;
	// Fallback: scan the class's chunks (pre-parse) and orphans (post-parse).
	IStorageObject *so = sc->findStorageObject(chunkId);
	if (!so)
	{
		const CStorageContainer::TStorageObjectContainer &orphans = sc->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		{
			if (it->first == chunkId) { so = it->second; break; }
		}
	}
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(so);
	if (raw && raw->Value.size() >= nBytes)
	{
		memcpy(dst, raw->Value.data(), nBytes);
		return true;
	}
	return false;
}

// Index of the key bracketing t=0: returns the value-index to use and, via lerpNext/lerpFactor,
// whether interpolation toward the next key is needed.
template <typename TKey>
static uint keyIndexAt0(const TKey *keys, uint numKeys, bool &lerpNext, float &lerpFactor)
{
	lerpNext = false;
	lerpFactor = 0.0f;
	if (keys[0].Time >= 0) return 0;
	if (keys[numKeys - 1].Time <= 0) return numKeys - 1;
	for (uint i = 0; i + 1 < numKeys; ++i)
	{
		if (keys[i].Time <= 0 && keys[i + 1].Time >= 0)
		{
			if (keys[i + 1].Time == 0) return i + 1;
			if (keys[i].Time == 0) return i;
			lerpNext = true;
			lerpFactor = (0.0f - (float)keys[i].Time) / ((float)keys[i + 1].Time - (float)keys[i].Time);
			fprintf(stderr, "WARNING: t=0 falls between keys (%d..%d ticks); linear interpolation used\n",
			        keys[i].Time, keys[i + 1].Time);
			return i;
		}
	}
	return 0;
}

static Point3M posValueAt0(CSceneClass *ctrl)
{
	Point3M p = { 0.0f, 0.0f, 0.0f };
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (kf && kf->keyCount())
	{
		bool lerp; float f;
		if (CControlPosLinear *c = dynamic_cast<CControlPosLinear *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageLinPoint3Key *k = c->keys();
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
			if (lerp)
			{
				p.x += f * (k[i + 1].Val[0] - k[i].Val[0]);
				p.y += f * (k[i + 1].Val[1] - k[i].Val[1]);
				p.z += f * (k[i + 1].Val[2] - k[i].Val[2]);
			}
			return p;
		}
		if (CControlPosBezier *c = dynamic_cast<CControlPosBezier *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageBezPoint3Key *k = c->keys();
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
			if (lerp) fprintf(stderr, "WARNING: bezier pos mid-interval at t=0, key value used\n");
			return p;
		}
		if (CControlPosTCB *c = dynamic_cast<CControlPosTCB *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageTCBPoint3Key *k = c->keys();
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
			if (lerp) fprintf(stderr, "WARNING: tcb pos mid-interval at t=0, key value used\n");
			return p;
		}
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_POS_VALUE, &p, 12);
	return p;
}

static QuatM rotValueAt0(CSceneClass *ctrl)
{
	QuatM q = { 0.0f, 0.0f, 0.0f, 1.0f };
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (kf && kf->keyCount())
	{
		bool lerp; float f;
		if (CControlRotLinear *c = dynamic_cast<CControlRotLinear *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageLinRotKey *k = c->keys();
			q.x = k[i].Quat[0]; q.y = k[i].Quat[1]; q.z = k[i].Quat[2]; q.w = k[i].Quat[3];
			if (lerp) fprintf(stderr, "WARNING: linear rot mid-interval at t=0, key value used\n");
			return q;
		}
		if (CControlRotTCB *c = dynamic_cast<CControlRotTCB *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageTCBRotKey *k = c->keys();
			q.x = k[i].AbsQuat[0]; q.y = k[i].AbsQuat[1]; q.z = k[i].AbsQuat[2]; q.w = k[i].AbsQuat[3];
			if (lerp) fprintf(stderr, "WARNING: tcb rot mid-interval at t=0, key value used\n");
			return q;
		}
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_ROT_VALUE, &q, 16);
	return q;
}

static ScaleValueM scaleValueAt0(CSceneClass *ctrl)
{
	ScaleValueM s;
	s.s.x = s.s.y = s.s.z = 1.0f;
	s.q.x = s.q.y = s.q.z = 0.0f;
	s.q.w = 1.0f;
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (kf && kf->keyCount())
	{
		bool lerp; float f;
		if (CControlScaleLinear *c = dynamic_cast<CControlScaleLinear *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageLinScaleKey *k = c->keys();
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			if (lerp) fprintf(stderr, "WARNING: linear scale mid-interval at t=0, key value used\n");
			return s;
		}
		if (CControlScaleBezier *c = dynamic_cast<CControlScaleBezier *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageBezScaleKey *k = c->keys();
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			if (lerp) fprintf(stderr, "WARNING: bezier scale mid-interval at t=0, key value used\n");
			return s;
		}
		if (CControlScaleTCB *c = dynamic_cast<CControlScaleTCB *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageTCBScaleKey *k = c->keys();
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			if (lerp) fprintf(stderr, "WARNING: tcb scale mid-interval at t=0, key value used\n");
			return s;
		}
	}
	// Default chunk 0x2505: CVector scale + CQuat axis system (28 bytes).
	uint8 buf[28];
	if (readCtrlDefaultBytes(ctrl, CHUNK_CTRL_SCALE_VALUE, buf, 28))
	{
		memcpy(&s.s, buf, 12);
		memcpy(&s.q, buf + 12, 16);
	}
	else if (readCtrlDefaultBytes(ctrl, CHUNK_CTRL_SCALE_VALUE, buf, 12))
	{
		memcpy(&s.s, buf, 12);
	}
	return s;
}

// ---------------------------------------------------------------------------------------------
// Node TM computation (GetNodeTM(0)) with memoization.

struct SNodeTMCache
{
	std::map<INode *, Matrix3M> TM;
	INode *SceneRoot;
};

static Matrix3M getNodeTM(INode *node, SNodeTMCache &cache)
{
	if (!node || node == cache.SceneRoot) return Matrix3M::identity();
	std::map<INode *, Matrix3M>::iterator it = cache.TM.find(node);
	if (it != cache.TM.end()) return it->second;

	Point3M pos = { 0.0f, 0.0f, 0.0f };
	QuatM rot = { 0.0f, 0.0f, 0.0f, 1.0f };
	ScaleValueM scale;
	scale.s.x = scale.s.y = scale.s.z = 1.0f;
	scale.q.x = scale.q.y = scale.q.z = 0.0f;
	scale.q.w = 1.0f;

	CReferenceMaker *tm = dynamic_cast<CReferenceMaker *>(node->getReference(0));
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm);
	if (tmsc && tmsc->classDesc()->classId() == CLASSID_PRS_CTRL && tm->nbReferences() >= 3)
	{
		pos = posValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(0)));
		rot = rotValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(1)));
		scale = scaleValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(2)));
	}
	else if (tmsc && tmsc->classDesc()->classId() == CLASSID_LOOKAT_CTRL && tm->nbReferences() >= 2)
	{
		// LookAt (target lights/cameras): position from ref 1; rotation is target-computed and
		// not needed for the current consumers (light positions).
		pos = posValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(1)));
	}
	else if (tmsc)
	{
		fprintf(stderr, "WARNING: node '%s' TM controller %s is not PRS; identity local TM used\n",
		        ucstring(node->userName()).toUtf8().c_str(), tmsc->classDesc()->classId().toString().c_str());
	}

	Matrix3M local = composePRS(pos, rot, scale);
	Matrix3M world = local * getNodeTM(node->parent(), cache);
	cache.TM[node] = world;
	return world;
}

// ---------------------------------------------------------------------------------------------
// The buildInstanceGroup replication.

struct SIgBuildStats
{
	uint UnimplementedLights;
	uint UnimplementedAccel;
	SIgBuildStats() : UnimplementedLights(0), UnimplementedAccel(0) { }
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

	// Accelerators (clusters/portals) and the clusterize linking: not implemented yet.
	std::vector<NL3D::CCluster> vClusters;
	std::vector<NL3D::CPortal> vPortals;
	for (i = 0; i < vectNode.size(); ++i)
	{
		int nAccelType = getScriptAppDataInt(dynamic_cast<CNodeImpl *>(vectNode[i]), NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
		if ((nAccelType & 3) == 2 || (nAccelType & 3) == 1)
			++stats.UnimplementedAccel;
	}

	// PointLights: not implemented yet (light-object decode pending).
	std::vector<NL3D::CPointLightNamed> pointLights;
	bool sunLightEnabled = false;
	for (i = 0; i < vectNode.size(); ++i)
	{
		if (nodeCategory(*vectNode[i]) == SCLASS_LIGHT)
			++stats.UnimplementedLights;
	}

	NL3D::CInstanceGroup *pIG = new NL3D::CInstanceGroup;
	pIG->build(NLMISC::CVector(0, 0, 0), aIGArray, vClusters, vPortals, pointLights);
	pIG->enableRealTimeSunContribution(sunLightEnabled);
	return pIG;
}

// ---------------------------------------------------------------------------------------------
// Debug dump of the per-node classification.

static const char *g_dumpObjName = NULL;

static void dumpNodes(CSceneClassContainer *ssc, SNodeTMCache &tmCache)
{
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
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		CSceneClass *obj = baseObjectOf(*node);
		bool flagsFound = false;
		uint32 flags = readNodeDword(node, NODE_FLAGS_CHUNK_ID, flagsFound);
		std::string ig = getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "");
		Matrix3M tm = getNodeTM(node, tmCache);
		printf("node '%s' ig='%s' cat=0x%x obj=%s (%s) accel=%d flags=0x%08x%s shape='%s' pos=(%g,%g,%g)\n",
		       ucstring(node->userName()).toUtf8().c_str(),
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

static int compareIgs(const char *pathA, const char *pathB, bool maskLighting, bool maskZ)
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
		if (NLMISC::toLower(a.Name) != NLMISC::toLower(b.Name)) diffs += NLMISC::toString(" Name('%s' vs '%s')", a.Name.c_str(), b.Name.c_str());
		if (a.InstanceName != b.InstanceName) diffs += NLMISC::toString(" InstanceName('%s' vs '%s')", a.InstanceName.c_str(), b.InstanceName.c_str());
		if (a.nParent != b.nParent) diffs += NLMISC::toString(" nParent(%d vs %d)", a.nParent, b.nParent);
		if (fabsf(a.Pos.x - b.Pos.x) > EPS || fabsf(a.Pos.y - b.Pos.y) > EPS || (!maskZ && fabsf(a.Pos.z - b.Pos.z) > EPS))
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
			if (a.SunContribution != b.SunContribution) diffs += " SunContribution";
			if (a.Light[0] != b.Light[0] || a.Light[1] != b.Light[1]) diffs += " Light";
			if (a.LocalAmbientId != b.LocalAmbientId) diffs += " LocalAmbientId";
		}
		if (!diffs.empty())
		{
			printf("DIFF instance %u '%s':%s\n", i, a.InstanceName.c_str(), diffs.c_str());
			++fails;
		}
	}

	if (igA.getNumPointLights() != igB.getNumPointLights())
	{
		printf("DIFF numPointLights %u vs %u\n", igA.getNumPointLights(), igB.getNumPointLights());
		++fails;
	}
	uint nl = std::min(igA.getNumPointLights(), igB.getNumPointLights());
	for (uint i = 0; i < nl; ++i)
	{
		NL3D::CPointLightNamed &a = igA.getPointLightNamed(i);
		NL3D::CPointLightNamed &b = igB.getPointLightNamed(i);
		std::string diffs;
		if (a.AnimatedLight != b.AnimatedLight) diffs += " AnimatedLight";
		if (a.LightGroup != b.LightGroup) diffs += " LightGroup";
		if ((a.getPosition() - b.getPosition()).norm() > EPS * (1.0f + a.getPosition().norm())) diffs += " Position";
		if (a.getType() != b.getType()) diffs += NLMISC::toString(" Type(%d vs %d)", (int)a.getType(), (int)b.getType());
		if (a.getDefaultAmbient() != b.getDefaultAmbient()) diffs += " Ambient";
		if (a.getDefaultDiffuse() != b.getDefaultDiffuse()) diffs += " Diffuse";
		if (a.getDefaultSpecular() != b.getDefaultSpecular()) diffs += " Specular";
		if (fabsf(a.getAttenuationBegin() - b.getAttenuationBegin()) > EPS || fabsf(a.getAttenuationEnd() - b.getAttenuationEnd()) > EPS) diffs += " Attenuation";
		if (!diffs.empty())
		{
			printf("DIFF pointlight %u:%s\n", i, diffs.c_str());
			++fails;
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
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		std::string arg = argv[argi];
		if (arg == "--dump") { dump = true; ++argi; }
		else if (arg == "--dump-obj" && argi + 1 < argc) { dump = true; g_dumpObjName = argv[argi + 1]; argi += 2; }
		else if (arg == "--db" && argi + 1 < argc) { g_dbRoot = argv[argi + 1]; argi += 2; }
		else if (arg == "--info" && argi + 1 < argc)
		{
			NL3D::registerSerial3d();
			return infoIg(argv[argi + 1]);
		}
		else if (arg == "--verbose") { g_verbose = true; ++argi; }
		else if (arg == "--compare" && argi + 2 < argc)
		{
			bool maskLighting = false, maskZ = false;
			for (int k = argi + 3; k < argc; ++k)
			{
				if (std::string(argv[k]) == "--mask-lighting") maskLighting = true;
				if (std::string(argv[k]) == "--mask-z") maskZ = true;
			}
			NL3D::registerSerial3d();
			return compareIgs(argv[argi + 1], argv[argi + 2], maskLighting, maskZ);
		}
		else break;
	}
	if (argc - argi < 2 && !(dump && argc - argi >= 1))
	{
		std::cerr << "usage: pipeline_max_export_ig <input.max> <output_dir>\n";
		std::cerr << "       pipeline_max_export_ig --dump <input.max>\n";
		std::cerr << "       pipeline_max_export_ig --compare <a.ig> <b.ig> [--mask-lighting] [--mask-z]\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export\n";
		return 1;
	}
	const char *maxFile = argv[argi];
	const char *outDir = (argc - argi >= 2) ? argv[argi + 1] : NULL;

	gsf_init();
	NL3D::registerSerial3d();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	g_registry = &reg;

	// Deduce the database root for XRef resolution when not passed: the ancestor directory of
	// the input that holds the database tree (contains "stuff").
	if (g_dbRoot.empty())
	{
		std::string p = NLMISC::CPath::standardizePath(NLMISC::CPath::getFullPath(NLMISC::CFile::getPath(maxFile), false), false);
		while (!p.empty() && p != "/")
		{
			if (NLMISC::CFile::isDirectory(p + "/stuff") || NLMISC::CFile::isDirectory(p + "/Stuff"))
			{
				g_dbRoot = p;
				break;
			}
			p = NLMISC::CFile::getPath(p);
			while (!p.empty() && p[p.size() - 1] == '/' && p != "/") p.resize(p.size() - 1);
		}
	}

	GsfInput *src = gsf_input_stdio_new(maxFile, NULL);
	if (!src) { std::cerr << "ERROR: cannot open " << maxFile << "\n"; return 1; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	g_object_unref(src);
	if (!in) { std::cerr << "ERROR: not an OLE compound file: " << maxFile << "\n"; return 1; }

	CDllDirectory dll;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); if (!s) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(s); dll.serial(st); g_object_unref(s); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); if (!s) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(s); cd.serial(st); g_object_unref(s); }
	cd.parse(VersionUnknown);

	CScene scene(&reg, &dll, &cd);
	{ GsfInput *s = gsf_infile_child_by_name(in, "Scene"); if (!s) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(s); scene.serial(st); g_object_unref(s); }
	scene.parse(VersionUnknown);
	g_object_unref(in);

	CSceneClassContainer *ssc = scene.container();
	SNodeTMCache tmCache;
	tmCache.SceneRoot = ssc->scene()->rootNode();

	if (dump)
	{
		dumpNodes(ssc, tmCache);
		return 0;
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
		// keep selection order).
		std::vector<INode *> vectNode;
		static const TSClassId cats[3] = { SCLASS_GEOMOBJECT, SCLASS_LIGHT, SCLASS_HELPER };
		for (int c = 0; c < 3; ++c)
		{
			for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
			{
				CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
				if (!node) continue;
				if (nodeCategory(*node) != cats[c]) continue;
				if (getScriptAppDataStr(node, NEL3D_APPDATA_IGNAME, "") != igName) continue;
				vectNode.push_back(node);
			}
		}

		SIgBuildStats stats;
		NL3D::CInstanceGroup *ig = buildInstanceGroup(vectNode, tmCache, stats);
		if (stats.UnimplementedLights)
			fprintf(stderr, "WARNING: ig '%s' has %u light node(s); point lights are not implemented yet\n", igName.c_str(), stats.UnimplementedLights);
		if (stats.UnimplementedAccel)
			fprintf(stderr, "WARNING: ig '%s' has %u accelerator node(s); clusters/portals are not implemented yet\n", igName.c_str(), stats.UnimplementedAccel);

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
