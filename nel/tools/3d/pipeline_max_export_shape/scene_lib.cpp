/**
 * \file scene_lib.cpp
 * \brief See scene_lib.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
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
#include <set>

#include <nel/misc/common.h>
#include <nel/misc/algo.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include "../pipeline_max/storage_ole.h"

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
#include "../pipeline_max_export_common/export_ids.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/param_block_2.h"

#include "../pipeline_max_export_common/db_path.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

namespace SCENELIB {

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
// Physique = SDK PHYSIQUE_CLASS_ID = Class_ID(0x00100, 0x00000); Skin = iskin.h SKIN_CLASSID =
// Class_ID(9815843, 87654) = Class_ID(0x0095c6a3, 0x00015666). Corpus-verified against Physique
// class entries in armor/character .max files (idx=41 name="Physique" superclass 0x810).
const NLMISC::CClassId CLASSID_PHYSIQUE(0x00000100, 0x00000000);
const NLMISC::CClassId CLASSID_SKIN(0x0095c6a3, 0x00015666);

// ---------------------------------------------------------------------------------------------

void setDatabaseRoot(const std::string &root)
{
	DBPATH::setDefaultRoot(root);
}

const std::string &databaseRoot()
{
	return DBPATH::defaultRoot();
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
	CStorageOleIn in;
	if (!in.open(path))
	{
		fprintf(stderr, "WARNING: not an OLE compound file: %s\n", path.c_str());
		return false;
	}
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(sceneRegistry(), dll, cd);
	bool ok = true;
	{ std::vector<uint8> b; if (in.readStream("DllDirectory", b)) { CStorageStream st(b); dll->serial(st); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("ClassDirectory3", b)) { CStorageStream st(b); cd->serial(st); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("Scene", b)) { CStorageStream st(b); scene->serial(st); scene->parse(VersionUnknown); } else ok = false; }
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

bool resolveDbPath(const std::string &authoredPath, std::string &out)
{
	return DBPATH::resolve(authoredPath, out);
}

// AppData readers live in pipeline_max_export_common/appdata_util (re-exported by scene_lib.h).

// ---------------------------------------------------------------------------------------------
// Node classification helpers

bool isGeometryOrShape(CSceneClass *base)
{
	if (!base) return false;
	TSClassId scid = base->classDesc()->superClassId();
	return scid == SCLASS_GEOMOBJECT || scid == SCLASS_SHAPE;
}

INode *rootOf(INode *node)
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

bool startsWithBip(const std::string &s)
{
	return s.size() >= 3 && s.compare(0, 3, "Bip") == 0;
}

bool shapeProcessSelectsNode(INode &node, const NLMISC::CClassId &cid)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);

	// Skeleton parts
	if (startsWithBip(nodeName(node)) || startsWithBip(nodeName(*rootOf(&node))))
		return false;

	if (cid == CLASSID_RPO)
		return false;
	if (cid.a() == CLASSID_PARTA_NEL_PS)
		return false;
	if (cid == PMAX_EXPORT_IDS::CLASSID_PACS_BOX || cid == PMAX_EXPORT_IDS::CLASSID_PACS_CYL)
		return false;
	// Target objects ((0x1020,0), light/camera look-at anchors) never yield reference
	// shapes (0 of 3518 references) — the reference exporter produces nothing for them.
	if (cid == CLASSID_TARGET)
		return false;

	// Accelerator?
	{
		std::string accel = getScriptAppDataStr(n, NEL3D_APPDATA_ACCEL, "");
		if (!accel.empty() && accel != "0" && accel != "32")
			return false;
	}

	if (getScriptAppDataStr(n, NEL3D_APPDATA_DONOTEXPORT, "") == "1")
		return false;
	if (getScriptAppDataStr(n, NEL3D_APPDATA_COLLISION, "") == "1")
		return false;
	if (getScriptAppDataStr(n, NEL3D_APPDATA_COLLISION_EXTERIOR, "") == "1")
		return false;

	return true;
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
		memcpy(&fl, nlVectorData(raw->Value), 4);
		found = true;
	}
	return fl;
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

// ---------------------------------------------------------------------------------------------
// ParamBlock2

bool readPB2Block(CSceneClass *pb2, SPB2Block &out)
{
	if (!pb2) return false;
	out.Object = pb2;
	out.ScriptVersion = 0;
	out.BlockId = 0;
	out.ParamCount = 0;
	out.Params.clear();

	// ParamBlock2 objects are typed in pipeline_max proper (BUILTIN::CParamBlock2, registered
	// for superclass 0x82): the header + parameter record decode, the reference-slot counting
	// and the tab-element handling all live there now. Copy its typed model into the exporter's
	// SPB2Block (the read-side struct material_build consumes).
	CParamBlock2 *tp = dynamic_cast<CParamBlock2 *>(pb2);
	if (!tp) return false;
	out.ScriptVersion = tp->scriptVersion();
	out.BlockId = tp->blockId();
	out.ParamCount = tp->declaredParamCount();
	const std::vector<CParamBlock2::SParam> &tps = tp->params();
	for (std::vector<CParamBlock2::SParam>::const_iterator it = tps.begin(); it != tps.end(); ++it)
	{
		SPB2Param p;
		p.Id = it->Id;
		p.Type = it->Type;
		p.HasConstant = it->HasConstant;
		p.RefBacked = it->RefBacked;
		p.RefSlot = it->RefSlot;
		p.F[0] = it->F[0]; p.F[1] = it->F[1]; p.F[2] = it->F[2]; p.F[3] = it->F[3];
		p.I = it->I;
		p.S = it->S;
		p.IsTab = it->IsTab;
		p.TabI = it->TabI;
		p.TabF = it->TabF;
		out.Params[p.Id] = p;
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

bool onOffControllerAt0(CReferenceMaker *ctrl)
{
	if (!ctrl) return false;
	uint32 initState = 0;
	std::vector<sint32> times;
	const CStorageContainer::TStorageObjectContainer &orphans = ctrl->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() != 4) continue;
		uint32 v;
		memcpy(&v, nlVectorData(raw->Value), 4);
		if (it->first == 0x0100) times.push_back((sint32)v);
		else if (it->first == 0x0140) initState = v;
	}
	bool state = initState != 0;
	for (uint i = 0; i < times.size(); ++i)
		if (times[i] <= 0) state = !state;
	return state;
}

bool resolveNelBoolAt0(const std::vector<SPB2Block> &blocks, uint block, uint16 id, bool def)
{
	const SPB2Param *p = findPB2Param(blocks, block, id);
	if (!p) return def;
	if (p->HasConstant) return p->I != 0;
	// Controller-backed: an On/Off controller keyed onto the flag (bExportTextureMatrix is animated
	// this way on some materials); evaluate its state at tick 0.
	if (p->RefBacked)
	{
		CSceneClass *rv = pb2RefValue(blocks[block], *p);
		if (rv && rv->classDesc()->classId().a() == 0x984b8d27)
			return onOffControllerAt0(dynamic_cast<CReferenceMaker *>(rv));
	}
	return def;
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
		fprintf(stderr, "WARNING: xref: cannot resolve '%s' under db root '%s'\n", file.c_str(), DBPATH::defaultRoot().c_str());
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
	// Deep OSM chains exist in the corpus (cococlaw LOD: 20+ nested OSM Derived wrappers
	// before the Editable Mesh). Guard must clear that depth; a seen-set breaks pure cycles.
	// SuperClassId of OSM Derived is often 0 on the unknown-class path — identify by ClassId.
	int guard = 256;
	std::set<CSceneClass *> seen;
	while (obj && guard-- > 0)
	{
		if (!seen.insert(obj).second)
			break; // cycle
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
			// GeomObject / Shape / derived-object wrappers are bases. Modifiers are stack
			// entries. Controllers and other non-geometry refs on the OSM array are ignored.
			if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER)
			{
				if (mods) mods->push_back(r);
				continue;
			}
			if (scid == SCLASS_GEOMOBJECT || scid == SCLASS_SHAPE
			    || r->classDesc()->classId() == CLASSID_OSM_DERIVED
			    || r->classDesc()->classId() == CLASSID_WSM_DERIVED
			    || r->classDesc()->classId().a() == 0x92aab38c)
			{
				base = r;
			}
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
