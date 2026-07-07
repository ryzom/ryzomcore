/**
 * \file scene_lib.cpp
 * \brief See scene_lib.h.
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
#include "scene_lib.h"

#include <cstdio>
#include <cstring>

#include <nel/misc/common.h>
#include <nel/misc/algo.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

namespace SCENELIB {

const NLMISC::CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);
const NLMISC::CClassId CLASSID_LOOKAT_CTRL(0x00002006, 0x00000000);
const NLMISC::CClassId CLASSID_OSM_DERIVED(0x29263a68, 0x405f22f5);
const NLMISC::CClassId CLASSID_WSM_DERIVED(0x4ec13906, 0x5578130e);
const NLMISC::CClassId CLASSID_RPO(0x368c679f, 0x711c22ee);
const NLMISC::CClassId CLASSID_TARGET(0x00001020, 0x00000000);
const NLMISC::CClassId CLASSID_EDITABLE_MESH(0xe44f10b3, 0x00000000);
const NLMISC::CClassId CLASSID_EDITABLE_POLY(0x1bf8338d, 0x192f6098);
const NLMISC::CClassId CLASSID_NEL_MTL(0x64c75fec, 0x222b9eb9);
const NLMISC::CClassId CLASSID_MULTI_MTL(0x00000200, 0x00000000);
const NLMISC::CClassId CLASSID_STDMAT(0x00000002, 0x00000000);
const NLMISC::CClassId CLASSID_BMTEX(0x00000240, 0x00000000);
const NLMISC::CClassId CLASSID_NEL_BMTEX(0x5a8003f9, 0x043e0955);

// AppData script-entry key (the MaxScript utility panel writes these)
static const NLMISC::CClassId APPDATA_SCRIPT_CLASS_ID(0x04d64858, 0x16d1751d);
static const uint32 APPDATA_SCRIPT_SUPER_CLASS_ID = 4128;

// PRS sub-controller default-value chunk ids
#define CHUNK_CTRL_POS_VALUE 0x2503
#define CHUNK_CTRL_ROT_VALUE 0x2504
#define CHUNK_CTRL_SCALE_VALUE 0x2505
#define CHUNK_CTRL_FLOAT_VALUE 0x2501

// ---------------------------------------------------------------------------------------------

static std::string g_dbRoot;

void setDatabaseRoot(const std::string &root)
{
	g_dbRoot = root;
}

const std::string &databaseRoot()
{
	return g_dbRoot;
}

CSceneClassRegistry *sceneRegistry()
{
	static CSceneClassRegistry *registry = NULL;
	if (!registry)
	{
		registry = new CSceneClassRegistry();
		CBuiltin::registerClasses(registry);
		UPDATE1::CUpdate1::registerClasses(registry);
		EPOLY::CEPoly::registerClasses(registry);
		BIPED::CBiped::registerClasses(registry);
		NELPATCH::CNelPatch::registerClasses(registry);
	}
	return registry;
}

bool loadMaxFile(const std::string &path, SLoadedMax &lm)
{
	GsfInput *src = gsf_input_stdio_new(path.c_str(), NULL);
	if (!src)
	{
		fprintf(stderr, "WARNING: cannot open %s\n", path.c_str());
		return false;
	}
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	g_object_unref(src);
	if (!in)
	{
		fprintf(stderr, "WARNING: not an OLE compound file: %s\n", path.c_str());
		return false;
	}
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(sceneRegistry(), dll, cd);
	bool ok = true;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); if (s) { CStorageStream st(s); dll->serial(st); g_object_unref(s); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); if (s) { CStorageStream st(s); cd->serial(st); g_object_unref(s); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { GsfInput *s = gsf_infile_child_by_name(in, "Scene"); if (s) { CStorageStream st(s); scene->serial(st); g_object_unref(s); scene->parse(VersionUnknown); } else ok = false; }
	g_object_unref(in);
	if (!ok)
	{
		fprintf(stderr, "WARNING: missing streams in %s\n", path.c_str());
		delete scene;
		delete cd;
		delete dll;
		return false;
	}
	lm.Dll = dll;
	lm.Cd = cd;
	lm.Scene = scene;
	return true;
}

static std::map<std::string, SLoadedMax> g_loadedScenes;

SLoadedMax *loadMaxFileCached(const std::string &path)
{
	std::map<std::string, SLoadedMax>::iterator it = g_loadedScenes.find(path);
	if (it != g_loadedScenes.end()) return it->second.Scene ? &it->second : NULL;
	SLoadedMax &lm = g_loadedScenes[path]; // inserted empty: failure is cached too
	if (!loadMaxFile(path, lm)) return NULL;
	return &lm;
}

// Case-insensitive path resolution under the database checkout: directories lowercased,
// filename lowercase first then verbatim (matches the on-disk convention).
static bool resolvePathCI(const std::string &root, const std::string &relative, std::string &out)
{
	std::vector<std::string> parts;
	NLMISC::splitString(relative, "/", parts);
	while (!parts.empty() && parts[0].empty()) parts.erase(parts.begin());
	if (parts.empty()) return false;
	std::string dir = root;
	for (uint i = 0; i + 1 < parts.size(); ++i)
	{
		if (parts[i].empty()) continue;
		dir += "/" + NLMISC::toLowerAscii(parts[i]);
	}
	const std::string &file = parts[parts.size() - 1];
	std::string lower = dir + "/" + NLMISC::toLowerAscii(file);
	if (NLMISC::CFile::fileExists(lower) || NLMISC::CFile::isDirectory(lower))
	{
		out = lower;
		return true;
	}
	std::string verbatim = dir + "/" + file;
	if (NLMISC::CFile::fileExists(verbatim) || NLMISC::CFile::isDirectory(verbatim))
	{
		out = verbatim;
		return true;
	}
	return false;
}

bool resolveDbPath(const std::string &authoredPath, std::string &out)
{
	std::string rel = authoredPath;
	for (uint i = 0; i < rel.size(); ++i)
		if (rel[i] == '\\') rel[i] = '/';
	std::string::size_type colon = rel.find(':');
	if (colon != std::string::npos) rel = rel.substr(colon + 1);
	while (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
	{
		std::string::size_type slash = rel.find('/');
		if (slash != std::string::npos)
		{
			std::string first = NLMISC::toLowerAscii(rel.substr(0, slash));
			if (first == "graphics" || first == "database")
				rel = rel.substr(slash + 1);
		}
	}
	if (g_dbRoot.empty()) return false;
	return resolvePathCI(g_dbRoot, rel, out);
}

// ---------------------------------------------------------------------------------------------
// AppData

bool getScriptAppData(CSceneClass *sc, uint32 subId, std::string &out)
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

std::string getScriptAppDataStr(CSceneClass *sc, uint32 subId, const std::string &def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	return s;
}

int getScriptAppDataInt(CSceneClass *sc, uint32 subId, int def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	int value = 0;
	if (NLMISC::fromString(s, value)) return value;
	return def;
}

float getScriptAppDataFloat(CSceneClass *sc, uint32 subId, float def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	float value = 0.0f;
	if (NLMISC::fromString(s, value)) return value;
	return def;
}

// ---------------------------------------------------------------------------------------------
// Chunk access

IStorageObject *findChunk(CSceneClass *sc, uint16 id)
{
	if (!sc) return NULL;
	IStorageObject *so = sc->findStorageObject(id);
	if (so) return so;
	const CStorageContainer::TStorageObjectContainer &orphans = sc->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		if (it->first == id) return it->second;
	return NULL;
}

CStorageRaw *findRawChunk(CSceneClass *sc, uint16 id)
{
	return dynamic_cast<CStorageRaw *>(findChunk(sc, id));
}

uint32 readNodeDword(CNodeImpl *node, uint16 chunkId, bool &found)
{
	found = false;
	uint32 fl = 0;
	CStorageRaw *raw = findRawChunk(node, chunkId);
	if (raw && raw->Value.size() >= 4)
	{
		memcpy(&fl, raw->Value.data(), 4);
		found = true;
	}
	return fl;
}

bool readObjectOffset(CNodeImpl *node, Point3M &pos, QuatM &rot, ScaleValueM &scale)
{
	pos.x = pos.y = pos.z = 0.0f;
	rot.x = rot.y = rot.z = 0.0f;
	rot.w = 1.0f;
	scale.s.x = scale.s.y = scale.s.z = 1.0f;
	scale.q.x = scale.q.y = scale.q.z = 0.0f;
	scale.q.w = 1.0f;
	bool any = false;
	CStorageRaw *raw = findRawChunk(node, 0x096a);
	if (raw && raw->Value.size() >= 12) { memcpy(&pos, raw->Value.data(), 12); any = true; }
	raw = findRawChunk(node, 0x096b);
	if (raw && raw->Value.size() >= 16) { memcpy(&rot, raw->Value.data(), 16); any = true; }
	raw = findRawChunk(node, 0x096c);
	if (raw && raw->Value.size() >= 28) { memcpy(&scale, raw->Value.data(), 28); any = true; }
	return any;
}

// ---------------------------------------------------------------------------------------------
// Old ParamBlock

void readPBlockParams(CSceneClass *pblock, std::map<sint32, SPBlockParam> &out)
{
	if (!pblock) return;
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
				memcpy(&idx, cr->Value.data(), 4);
			else if (cit->first == 0x0102 && cr->Value.size() == 12 && idx >= 0)
			{
				SPBlockParam p;
				p.IsPoint3 = true;
				memcpy(p.V, cr->Value.data(), 12);
				out[idx] = p;
			}
			else if (cit->first != 0x0004 && cr->Value.size() == 4 && idx >= 0)
			{
				SPBlockParam p;
				p.IsPoint3 = false;
				p.IsInt = (cit->first == 0x0101);
				p.V[1] = p.V[2] = 0.0f;
				memcpy(p.V, cr->Value.data(), 4);
				memcpy(&p.I, cr->Value.data(), 4);
				out[idx] = p;
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// ParamBlock2

static bool pb2TypeIsRefKind(uint16 type)
{
	switch (type & 0x07ff)
	{
	case PB2_MTL:
	case PB2_TEXMAP:
	case PB2_NODE:
	case PB2_REFTARG:
		return true;
	}
	return false;
}

bool readPB2Block(CSceneClass *pb2, SPB2Block &out)
{
	if (!pb2) return false;
	out.Object = pb2;
	out.ScriptVersion = 0;
	out.BlockId = 0;
	out.ParamCount = 0;
	out.Params.clear();
	sint refSlot = 0;
	const CStorageContainer::TStorageObjectContainer &orphans = pb2->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) continue;
		if (it->first == 0x0009 && raw->Value.size() >= 16)
		{
			memcpy(&out.ScriptVersion, raw->Value.data(), 4);
			memcpy(&out.BlockId, raw->Value.data() + 4, 2);
			memcpy(&out.ParamCount, raw->Value.data() + 10, 2);
		}
		else if (it->first == 0x000e && raw->Value.size() >= 15)
		{
			SPB2Param p;
			memcpy(&p.Id, raw->Value.data(), 2);
			memcpy(&p.Type, raw->Value.data() + 2, 2);
			uint8 flagByte = raw->Value[14];
			p.HasConstant = false;
			p.RefBacked = false;
			p.RefSlot = -1;
			p.F[0] = p.F[1] = p.F[2] = p.F[3] = 0.0f;
			p.I = 0;
			const uint8 *payload = raw->Value.data() + 15;
			size_t payloadSize = raw->Value.size() - 15;
			bool refKind = pb2TypeIsRefKind(p.Type);
			bool isConstant = (flagByte & 0x40) != 0;
			if (refKind || !isConstant)
			{
				// reftarget-kind params and controller-backed value params own the PB2's
				// reference slots in record order
				p.RefBacked = true;
				p.RefSlot = refSlot++;
			}
			if (isConstant && !refKind && payloadSize > 0)
			{
				p.HasConstant = true;
				switch (p.Type & 0x07ff)
				{
				case PB2_FLOAT:
				case PB2_ANGLE:
				case PB2_PCNT_FRAC:
				case PB2_WORLD:
				case PB2_COLOR_CHANNEL:
					if (payloadSize >= 4) memcpy(&p.F[0], payload, 4);
					break;
				case PB2_INT:
				case PB2_BOOL:
				case PB2_TIMEVALUE:
				case PB2_RADIOBTN_INDEX:
					if (payloadSize >= 4)
					{
						memcpy(&p.I, payload, 4);
						p.F[0] = (float)p.I;
					}
					break;
				case PB2_RGBA:
				case PB2_POINT3:
				case PB2_HSV:
					if (payloadSize >= 12) memcpy(p.F, payload, 12);
					break;
				case PB2_STRING:
				case PB2_FILENAME:
					if (payloadSize >= 4)
					{
						uint32 len;
						memcpy(&len, payload, 4);
						if (len > payloadSize - 4) len = (uint32)(payloadSize - 4);
						std::string s((const char *)payload + 4, len);
						while (!s.empty() && s[s.size() - 1] == '\0') s.resize(s.size() - 1);
						p.S = s;
					}
					break;
				default:
					// unknown/tab types: keep the record id/type, no decoded value
					p.HasConstant = false;
					break;
				}
			}
			out.Params[p.Id] = p;
		}
	}
	return out.ParamCount != 0 || !out.Params.empty();
}

void readObjectPB2Blocks(CSceneClass *obj, std::vector<SPB2Block> &out)
{
	out.clear();
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	if (!rm) return;
	for (uint i = 0; i < rm->nbReferences(); ++i)
	{
		CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
		if (!r) continue;
		if (r->classDesc()->superClassId() != SCLASS_PBLOCK2) continue;
		SPB2Block block;
		if (readPB2Block(r, block))
			out.push_back(block);
	}
}

const SPB2Param *findPB2Param(const std::vector<SPB2Block> &blocks, uint blockIndex, uint16 paramId)
{
	if (blockIndex >= blocks.size()) return NULL;
	std::map<uint16, SPB2Param>::const_iterator it = blocks[blockIndex].Params.find(paramId);
	if (it == blocks[blockIndex].Params.end()) return NULL;
	return &it->second;
}

CSceneClass *pb2RefValue(const SPB2Block &block, const SPB2Param &param)
{
	if (!param.RefBacked || param.RefSlot < 0) return NULL;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(block.Object);
	if (!rm) return NULL;
	return dynamic_cast<CSceneClass *>(rm->getReference(param.RefSlot));
}

// ---------------------------------------------------------------------------------------------
// Controller values at t=0

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
	CStorageRaw *raw = findRawChunk(sc, chunkId);
	if (raw && raw->Value.size() >= nBytes)
	{
		memcpy(dst, raw->Value.data(), nBytes);
		return true;
	}
	return false;
}

// Index of the key bracketing t=0
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

Point3M posValueAt0(CSceneClass *ctrl)
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

QuatM rotValueAt0(CSceneClass *ctrl)
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

ScaleValueM scaleValueAt0(CSceneClass *ctrl)
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

float floatValueAt0(CSceneClass *ctrl, float def)
{
	float v = def;
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (kf && kf->keyCount())
	{
		bool lerp; float f;
		if (CControlFloatBezier *c = dynamic_cast<CControlFloatBezier *>(kf))
		{
			uint i = keyIndexAt0(c->keys(), kf->keyCount(), lerp, f);
			const CStorageBezFloatKey *k = c->keys();
			v = k[i].Val;
			if (lerp) fprintf(stderr, "WARNING: bezier float mid-interval at t=0, key value used\n");
			return v;
		}
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_FLOAT_VALUE, &v, 4);
	return v;
}

// ---------------------------------------------------------------------------------------------
// Node TM

Matrix3M getNodeTM(INode *node, SNodeTMCache &cache)
{
	if (!node || node == cache.SceneRoot) return Matrix3M::identity();
	if (!dynamic_cast<CNodeImpl *>(node)) return Matrix3M::identity(); // scene root / unknown node class
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
		// not needed for the current consumers.
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
// Object chain

CSceneClass *objectRefOf(INode &node)
{
	return dynamic_cast<CSceneClass *>(node.getReference(1));
}

// XRef resolution: 0x0170 record = source file (0x0100, UTF-16) + source node name (0x0110).
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
	std::string resolved;
	if (!resolveDbPath(file, resolved))
	{
		fprintf(stderr, "WARNING: xref: cannot resolve '%s' under db root '%s'\n", file.c_str(), g_dbRoot.c_str());
		return NULL;
	}
	SLoadedMax *lm = loadMaxFileCached(resolved);
	if (!lm) return NULL;
	CSceneClassContainer *ssc = lm->Scene->container();
	std::string wantLower = NLMISC::toLowerAscii(objName);
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (NLMISC::toLowerAscii(ucstring(node->userName()).toUtf8()) != wantLower) continue;
		return baseObjectOf(dynamic_cast<CSceneClass *>(node->getReference(1)), NULL, NULL);
	}
	fprintf(stderr, "WARNING: xref: node '%s' not found in %s\n", objName.c_str(), resolved.c_str());
	return NULL;
}

CSceneClass *baseObjectOf(CSceneClass *obj, std::vector<CSceneClass *> *mods,
                          std::vector<CStorageContainer *> *modApps)
{
	int guard = 16;
	while (obj && guard-- > 0)
	{
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid.a() == 0x92aab38c)
		{
			CSceneClass *resolved = resolveXRefObject(obj, 0);
			if (!resolved) return obj; // unresolvable: keep the wrapper
			obj = resolved;
			continue;
		}
		if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		CSceneClass *base = NULL;
		uint modCountBefore = mods ? (uint)mods->size() : 0;
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (!r) continue;
			TSClassId scid = r->classDesc()->superClassId();
			if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER)
			{
				if (mods) mods->push_back(r);
				continue;
			}
			base = r;
		}
		if (modApps)
		{
			// mod-app local data: the wrapper's orphaned 0x2500 containers, one per modifier
			// slot in reference order
			std::vector<CStorageContainer *> apps;
			const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
			for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
				if (it->first == 0x2500)
					apps.push_back(dynamic_cast<CStorageContainer *>(it->second));
			// pad/truncate to the modifier count of THIS wrapper
			uint nMods = mods ? (uint)mods->size() - modCountBefore : (uint)apps.size();
			for (uint m = 0; m < nMods; ++m)
				modApps->push_back(m < apps.size() ? apps[m] : NULL);
		}
		if (!base) break;
		obj = base;
	}
	return obj;
}

CSceneClass *baseObjectOf(INode &node, std::vector<CSceneClass *> *mods,
                          std::vector<CStorageContainer *> *modApps)
{
	return baseObjectOf(objectRefOf(node), mods, modApps);
}

CSceneClass *materialOf(INode &node)
{
	return dynamic_cast<CSceneClass *>(node.getReference(3));
}

std::string nodeName(INode &node)
{
	return ucstring(node.userName()).toUtf8();
}

} /* namespace SCENELIB */

/* end of file */
