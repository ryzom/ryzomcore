/**
 * \file scene_lib.h
 * \brief Generic headless .max scene access for the shape exporter: scene loading, script
 * AppData, ParamBlock/ParamBlock2 reading, PRS controller values at t=0, node TM computation,
 * derived-object chain walking and XRef resolution. Adapted from the ig exporter's in-file
 * helpers (pipeline_max_export_ig/main.cpp) with a full typed ParamBlock2 record decode on top
 * (values by (block, param) or by script parameter name through per-script tables).
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_SCENE_LIB_H
#define PIPELINE_MAX_EXPORT_SHAPE_SCENE_LIB_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/misc/ucstring.h>

#include <map>
#include <string>
#include <vector>

#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/storage_object.h"

#include "max_math.h"

namespace PIPELINE {
namespace MAX {
class CDllDirectory;
class CClassDirectory3;
class CSceneClassRegistry;
namespace BUILTIN {
class CNodeImpl;
class INode;
class CReferenceMaker;
}
}
}

namespace SCENELIB {

using namespace PIPELINE::MAX;
using PIPELINE::MAX::BUILTIN::CNodeImpl;
using PIPELINE::MAX::BUILTIN::INode;
using PIPELINE::MAX::BUILTIN::CReferenceMaker;

// Well-known scene class identities
extern const NLMISC::CClassId CLASSID_PRS_CTRL;
extern const NLMISC::CClassId CLASSID_LOOKAT_CTRL;
extern const NLMISC::CClassId CLASSID_OSM_DERIVED;
extern const NLMISC::CClassId CLASSID_WSM_DERIVED;
extern const NLMISC::CClassId CLASSID_RPO;
extern const NLMISC::CClassId CLASSID_TARGET;
extern const NLMISC::CClassId CLASSID_EDITABLE_MESH;
extern const NLMISC::CClassId CLASSID_EDITABLE_POLY;
extern const NLMISC::CClassId CLASSID_NEL_MTL;
extern const NLMISC::CClassId CLASSID_MULTI_MTL;
extern const NLMISC::CClassId CLASSID_STDMAT;
extern const NLMISC::CClassId CLASSID_BMTEX;
extern const NLMISC::CClassId CLASSID_NEL_BMTEX;

// Superclass ids
const TSClassId SCLASS_GEOMOBJECT = 0x00000010;
const TSClassId SCLASS_SHAPE = 0x00000040;
const TSClassId SCLASS_LIGHT = 0x00000030;
const TSClassId SCLASS_CAMERA = 0x00000020;
const TSClassId SCLASS_HELPER = 0x00000050;
const TSClassId SCLASS_OSMODIFIER = 0x00000810;
const TSClassId SCLASS_WSMODIFIER = 0x00000820;
const TSClassId SCLASS_PBLOCK = 0x00000008;
const TSClassId SCLASS_PBLOCK2 = 0x00000082;

// ---------------------------------------------------------------------------------------------
// Scene loading

struct SLoadedMax
{
	CDllDirectory *Dll;
	CClassDirectory3 *Cd;
	CScene *Scene;
	SLoadedMax() : Dll(NULL), Cd(NULL), Scene(NULL) { }
};

// One-time registry construction (builtin + update1 + epoly + biped + nelpatch classes).
CSceneClassRegistry *sceneRegistry();

// Load and parse a .max file's DllDirectory/ClassDirectory3/Scene streams. Returns false and
// leaves lm empty on failure. Caller owns the pointers (or use loadMaxFileCached).
bool loadMaxFile(const std::string &path, SLoadedMax &lm);

// Cached load, keyed by resolved path (also caches failure). Used for XRef and interface files.
SLoadedMax *loadMaxFileCached(const std::string &path);

// Database root used for XRef / interface-file resolution (the ryzomcore_graphics checkout).
void setDatabaseRoot(const std::string &root);
const std::string &databaseRoot();

// Resolve an authored (Windows, case-insensitive) path against the database root: strips the
// drive and the leading graphics/database component, lowercases directory components.
bool resolveDbPath(const std::string &authoredPath, std::string &out);

// ---------------------------------------------------------------------------------------------
// Script AppData (MAXSCRIPT_UTILITY_CLASS_ID / 4128 / subId string entries)

bool getScriptAppData(CSceneClass *sc, uint32 subId, std::string &out);
std::string getScriptAppDataStr(CSceneClass *sc, uint32 subId, const std::string &def);
int getScriptAppDataInt(CSceneClass *sc, uint32 subId, int def);
float getScriptAppDataFloat(CSceneClass *sc, uint32 subId, float def);

// ---------------------------------------------------------------------------------------------
// Old-style ParamBlock (superclass 0x8) params

struct SPBlockParam
{
	bool IsPoint3;
	bool IsInt;
	sint32 I;
	float V[3];
	SPBlockParam() : IsPoint3(false), IsInt(false), I(0) { V[0] = V[1] = V[2] = 0.0f; }
};

void readPBlockParams(CSceneClass *pblock, std::map<sint32, SPBlockParam> &out);

// ---------------------------------------------------------------------------------------------
// ParamBlock2 (superclass 0x82) typed record decode.
//
// PB2 storage (corpus-established, see pipeline_max_design.md): header chunk 0x0009 =
// { u32 scriptVersion, u16 blockId, u16 (owner class marker), u16 0x2328, u16 paramCount,
// u32 ownerSceneIndex }; one 0x000e chunk per param = { u16 paramId, u16 type, u32, u8 flags1,
// u32, u8, u8 flagByte, payload }. flagByte bit 0x40 = constant value follows inline (except
// reftarget-kind types, whose value is a reference slot on the PB2 object); records without
// 0x40 are controller-backed and own reference slots in record order alongside the
// reftarget-kind params.

enum TPB2Type
{
	PB2_FLOAT = 0x0,
	PB2_INT = 0x1,
	PB2_RGBA = 0x2,
	PB2_POINT3 = 0x3,
	PB2_BOOL = 0x4,
	PB2_ANGLE = 0x5,
	PB2_PCNT_FRAC = 0x6,
	PB2_WORLD = 0x7,
	PB2_STRING = 0x8,
	PB2_FILENAME = 0x9,
	PB2_HSV = 0xa,
	PB2_COLOR_CHANNEL = 0xb,
	PB2_TIMEVALUE = 0xc,
	PB2_RADIOBTN_INDEX = 0xd,
	PB2_MTL = 0xe,
	PB2_TEXMAP = 0xf,
	PB2_BITMAP = 0x10,
	PB2_NODE = 0x11,
	PB2_REFTARG = 0x12
};

struct SPB2Param
{
	uint16 Id;
	uint16 Type;
	bool HasConstant;    // inline constant payload present
	bool RefBacked;      // owns a reference slot (reftarget kind, or controller-backed value)
	sint RefSlot;        // reference slot index on the PB2 object, -1 when not ref-backed
	// Constant payloads by kind
	float F[4];          // float/int/bool/color components (F[0] for scalars)
	sint32 I;
	std::string S;       // string/filename
	// Tab (array) params (type flag 0x800): per-element inline values. For reference-kind
	// element types (TEXMAP etc.) TabI holds the PB2 reference slot per element (-1 = none).
	bool IsTab;
	std::vector<sint32> TabI;
	std::vector<float> TabF;
};

struct SPB2Block
{
	uint32 ScriptVersion;
	uint16 BlockId;
	uint16 ParamCount;
	CSceneClass *Object; // the PB2 scene object
	std::map<uint16, SPB2Param> Params;
};

// Decode one PB2 scene object's records.
bool readPB2Block(CSceneClass *pb2, SPB2Block &out);

// All PB2 blocks referenced by a scene object (reference order), decoded.
void readObjectPB2Blocks(CSceneClass *obj, std::vector<SPB2Block> &out);

// Look up a param value across an object's PB2 blocks by (blockIndex, paramId).
// blockIndex is the index into the object's PB2 reference order.
const SPB2Param *findPB2Param(const std::vector<SPB2Block> &blocks, uint blockIndex, uint16 paramId);

// The scene object referenced by a ref-backed PB2 param (texmap etc.), NULL when absent.
CSceneClass *pb2RefValue(const SPB2Block &block, const SPB2Param &param);

/// The On/Off bool controller (0x984b8d27) state at tick 0: state before the first key (chunk
/// 0x0140) toggled by every key (0x0100 time) at or before 0.
bool onOffControllerAt0(CReferenceMaker *ctrl);

/// A NeL-material bool param's value at t=0: the inline constant, else a controller-backed value
/// (an On/Off controller keyed onto the flag — e.g. bExportTextureMatrix), else the default.
bool resolveNelBoolAt0(const std::vector<SPB2Block> &blocks, uint block, uint16 id, bool def);

// ---------------------------------------------------------------------------------------------
// Controller values at t=0 (PRS sub-controllers; typed keyframer key tables bracketed at t=0,
// else the default-value chunks 0x2503/0x2504/0x2505)

MAXMATH::Point3M posValueAt0(CSceneClass *ctrl);
MAXMATH::QuatM rotValueAt0(CSceneClass *ctrl);
MAXMATH::ScaleValueM scaleValueAt0(CSceneClass *ctrl);

// Bezier float controller value at t=0 (morph channel weights); returns def when absent.
float floatValueAt0(CSceneClass *ctrl, float def);

// ---------------------------------------------------------------------------------------------
// Node access

struct SNodeTMCache
{
	std::map<INode *, MAXMATH::Matrix3M> TM;
	INode *SceneRoot;
	SNodeTMCache() : SceneRoot(NULL) { }
};

// GetNodeTM(0) replication with memoization.
MAXMATH::Matrix3M getNodeTM(INode *node, SNodeTMCache &cache);

// Node state flags chunk 0x0963 bit 0x40 = hidden; rendering-control chunk 0x099c bits
// 0x0200 = cast-shadows, 0x0400 = receive-shadows.
uint32 readNodeDword(CNodeImpl *node, uint16 chunkId, bool &found);

// The node's object-offset TRS (chunks 0x096a pos, 0x096b rot, 0x096c ScaleValue).
bool readObjectOffset(CNodeImpl *node, MAXMATH::Point3M &pos, MAXMATH::QuatM &rot, MAXMATH::ScaleValueM &scale);

// ---------------------------------------------------------------------------------------------
// Object chain

// The node's object reference (reference 1).
CSceneClass *objectRefOf(INode &node);

// Unwrap derived-object wrappers and XRefs down to the base object. When mods is non-NULL,
// collects the modifier scene objects (outermost wrapper first, reference order within each
// wrapper) and their per-node mod-app 0x2500 containers (parallel array, NULL when absent).
CSceneClass *baseObjectOf(CSceneClass *obj, std::vector<CSceneClass *> *mods = NULL,
                          std::vector<CStorageContainer *> *modApps = NULL);
CSceneClass *baseObjectOf(INode &node, std::vector<CSceneClass *> *mods = NULL,
                          std::vector<CStorageContainer *> *modApps = NULL);

// The material reference of a node (reference 3), NULL if none.
CSceneClass *materialOf(INode &node);

// Node user name (utf8)
std::string nodeName(INode &node);

// Find a raw chunk by id among a scene class's orphans (and pre-parse chunks).
IStorageObject *findChunk(CSceneClass *sc, uint16 id);
CStorageRaw *findRawChunk(CSceneClass *sc, uint16 id);

} /* namespace SCENELIB */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_SCENE_LIB_H */

/* end of file */
