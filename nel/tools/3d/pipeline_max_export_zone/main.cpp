/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Zone export: .max -> .zone / .ligozone, replicating the ExportRykolZone and NeLLigoExportZone
// paths of the 3ds Max plugins (build_gamedata processes/zone and processes/ligo) without
// 3ds Max.
//
// The ligo maxscript (processes/ligo/maxscript/nel_ligo_export.ms) classifies files by name:
//   zonematerial-<mat>-<cell>.max   -> one brick: select the single non-frozen RklPatch,
//                                      NeLLigoExportZone -> zones/<mat>-<cell>.zone +
//                                      zoneLigos/<mat>-<cell>.ligozone
//   zonetransition-<a>-<b>-<t>.max  -> nine bricks: each non-frozen RklPatch is classified on
//                                      the transition scheme grid by its cell position, then
//                                      exported through a mirrored/rotated/translated instance
//                                      transform with the symmetry/rotate appdata set
//   zonespecial-<name>.max          -> one brick, material "special"
// Frozen RklPatch nodes are boundary-repetition references, never exported (frozen marker =
// empty node chunk 0x0976, established corpus-wide). The direct zone process
// (processes/zone/maxscript/zone_export.ms, ExportRykolZone) exports the first RklPatch in
// scene order with the zone id derived from the node name (findID).
//
// The plugin side both paths share is RPatchMesh::exportZone (plugin_max/nel_patch_lib/
// rpo2nel.cpp): PatchMesh+RPatchMesh (+node object TM at t=0 in Max float math) ->
// CPatchInfo[] -> optional CPatchInfo::transform (symmetry/rotate with the tile bank) ->
// CZone::build -> serial. This tool replicates the conversion on the storage-level data (see
// pipeline_max/nelpatch/) and links real NL3D/NLLIGO for everything downstream. NeL Edit Patch
// modifier stacks are evaluated from the modifier's per-node local data (final patch 0x1140 +
// RPatchMesh 0x4001 + vertex-mapper deltas 0x1130 applied against the base patch, replicating
// EditPatchData::Apply + EPVertMapper::UpdateAndApplyDeltas).
//
// The .zone is written as serial version 4 (the reference era; current CZone::serial writes 5,
// which differs from 4 only in the version byte itself — verified by roundtripping reference
// zones through the current serializer).

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

// MSVC 9.0 (VS2008) has no C99 snprintf; _snprintf is equivalent for our fixed-size formatting.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/path.h>

#include <nel/3d/register_3d.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_symmetrisation.h>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/ligo_error.h>
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_template.h>

#include "../pipeline_max/storage_ole.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
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
#include "../pipeline_max/builtin/control_keyframer.h"

#include "../pipeline_max_export_common/max_math.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

// Scene class ids
static const NLMISC::CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);
static const NLMISC::CClassId CLASSID_OSM_DERIVED(0x29263a68, 0x405f22f5);
static const NLMISC::CClassId CLASSID_WSM_DERIVED(0x4ec13906, 0x5578130e);
static const NLMISC::CClassId CLASSID_NEL_EDIT_PATCH(0x4dd14a3c, 0x4ac23c0c);
// NeL Patch Painter (nel_patch_paint): local data is the same final-patch shape (0x1140 +
// 0x4001) with no vertex mapper — the stored final patch is the evaluated output verbatim
// (PaintPatchData::Apply).
static const NLMISC::CClassId CLASSID_NEL_PATCH_PAINT(0x0c49560f, 0x3c3d68e7);

// Superclass ids
static const TSClassId SCLASS_OSMODIFIER = 0x00000810;
static const TSClassId SCLASS_WSMODIFIER = 0x00000820;

// PRS sub-controller default-value chunk ids
#define CHUNK_CTRL_POS_VALUE 0x2503
#define CHUNK_CTRL_ROT_VALUE 0x2504
#define CHUNK_CTRL_SCALE_VALUE 0x2505

// Node frozen marker: empty chunk 0x0976 on the node (corpus-established: the boundary
// reference zones carry it, the exported zone does not; transition cells carry none).
#define NODE_FROZEN_CHUNK_ID 0x0976

static bool g_verbose = false;

// ---------------------------------------------------------------------------------------------
// AppData access (same shape as the other exporters).

// Shared script AppData readers (pipeline_max_export_common/appdata_util) — formerly
// file-local copies here (the local Int variant parsed via sscanf; fromString is equivalent
// over the decimal-string values this convention stores).
using APPDATA::getScriptAppData;
using APPDATA::getScriptAppDataInt;

static bool hasScriptAppData(CSceneClass *sc, uint32 subId)
{
	std::string s;
	return getScriptAppData(sc, subId, s);
}

// ---------------------------------------------------------------------------------------------
// PRS controller values at t=0 and node TM (GetNodeTM(0)); same replication as the ig exporter.

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
		memcpy(dst, nlVectorData(raw->Value), nBytes);
		return true;
	}
	return false;
}

template <typename TKey>
static uint keyIndexAt0(const TKey *keys, uint numKeys)
{
	if (keys[0].Time >= 0) return 0;
	if (keys[numKeys - 1].Time <= 0) return numKeys - 1;
	for (uint i = 0; i + 1 < numKeys; ++i)
		if (keys[i].Time <= 0 && keys[i + 1].Time >= 0)
			return (keys[i + 1].Time == 0) ? i + 1 : i;
	return 0;
}

static Point3M posValueAt0(CSceneClass *ctrl)
{
	Point3M p = { 0.0f, 0.0f, 0.0f };
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (kf && kf->keyCount())
	{
		if (CControlPosLinear *c = dynamic_cast<CControlPosLinear *>(kf))
		{
			const CStorageLinPoint3Key *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
			return p;
		}
		if (CControlPosBezier *c = dynamic_cast<CControlPosBezier *>(kf))
		{
			const CStorageBezPoint3Key *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
			return p;
		}
		if (CControlPosTCB *c = dynamic_cast<CControlPosTCB *>(kf))
		{
			const CStorageTCBPoint3Key *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			p.x = k[i].Val[0]; p.y = k[i].Val[1]; p.z = k[i].Val[2];
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
		if (CControlRotLinear *c = dynamic_cast<CControlRotLinear *>(kf))
		{
			const CStorageLinRotKey *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			q.x = k[i].Quat[0]; q.y = k[i].Quat[1]; q.z = k[i].Quat[2]; q.w = k[i].Quat[3];
			return q;
		}
		if (CControlRotTCB *c = dynamic_cast<CControlRotTCB *>(kf))
		{
			const CStorageTCBRotKey *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			q.x = k[i].AbsQuat[0]; q.y = k[i].AbsQuat[1]; q.z = k[i].AbsQuat[2]; q.w = k[i].AbsQuat[3];
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
		if (CControlScaleLinear *c = dynamic_cast<CControlScaleLinear *>(kf))
		{
			const CStorageLinScaleKey *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			return s;
		}
		if (CControlScaleBezier *c = dynamic_cast<CControlScaleBezier *>(kf))
		{
			const CStorageBezScaleKey *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			return s;
		}
		if (CControlScaleTCB *c = dynamic_cast<CControlScaleTCB *>(kf))
		{
			const CStorageTCBScaleKey *k = c->keys();
			uint i = keyIndexAt0(k, kf->keyCount());
			memcpy(&s.s, k[i].S, 12);
			memcpy(&s.q, k[i].Q, 16);
			return s;
		}
	}
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

struct SNodeTMCache
{
	std::map<INode *, Matrix3M> TM;
};

static Matrix3M getNodeTM(INode *node, SNodeTMCache &cache)
{
	if (!node || dynamic_cast<CRootNode *>(node)) return Matrix3M::identity();
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

// Node object-offset TRS (chunks 0x096a pos, 0x096b rot, 0x096c ScaleValue) -> the offset
// matrix; objectTM = offsetTM * nodeTM.
static Matrix3M getObjectTM(CNodeImpl *node, SNodeTMCache &cache)
{
	Point3M pos = { 0.0f, 0.0f, 0.0f };
	QuatM rot = { 0.0f, 0.0f, 0.0f, 1.0f };
	ScaleValueM scale;
	scale.s.x = scale.s.y = scale.s.z = 1.0f;
	scale.q.x = scale.q.y = scale.q.z = 0.0f;
	scale.q.w = 1.0f;
	// Typed CNodeImpl overlay via the shared reader (formerly an inline orphan walk here).
	bool any = MAXSCENE::readObjectOffset(node, pos, rot, scale);
	Matrix3M nodeTM = getNodeTM(node, cache);
	if (!any) return nodeTM;
	Matrix3M offsetTM = composePRS(pos, rot, scale);
	return offsetTM * nodeTM;
}

static bool isNodeFrozen(CNodeImpl *node)
{
	const CStorageContainer::TStorageObjectContainer &orphans = node->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		if (it->first == NODE_FROZEN_CHUNK_ID) return true;
	return false;
}

// ---------------------------------------------------------------------------------------------
// Evaluated patch resolution: the node's object is either the RklPatch itself or an OSM Derived
// stack of NeL Edit Patch modifiers over it. Modifier evaluation replicates EditPatchData::
// Apply: the stored final patch (0x1140 + 0x4001) with the vertex-mapper deltas (0x1130)
// applied against the input patch's positions.

struct SEvalPatch
{
	SPatchMesh Pm;
	SRPatchMesh Rp;
};

static CStorageContainer *containerChild(const CStorageContainer *c, uint16 id, uint skip = 0)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
	{
		if (it->first != id) continue;
		if (skip) { --skip; continue; }
		return dynamic_cast<CStorageContainer *>(it->second);
	}
	return NULL;
}

static CStorageRaw *rawChildOf(const CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
	{
		if (it->first != id) continue;
		return dynamic_cast<CStorageRaw *>(it->second);
	}
	return NULL;
}

// The modifier per-node local data of derived-object modifier slot modIndex: the wrapper's
// 0x2500 containers in reference order, each containing 0x2512 -> 0x1000 (the edit-patch
// local data wrapper).
static CStorageContainer *editPatchLocalData(CSceneClass *derived, uint modIndex)
{
	const CStorageContainer::TStorageObjectContainer &orphans = derived->orphanedChunks();
	uint slot = 0;
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != 0x2500) continue;
		if (slot++ != modIndex) continue;
		CStorageContainer *slotC = dynamic_cast<CStorageContainer *>(it->second);
		if (!slotC) return NULL;
		CStorageContainer *data = containerChild(slotC, 0x2512);
		if (!data) return NULL;
		return containerChild(data, 0x1000);
	}
	return NULL;
}

// Apply the NeL Edit Patch modifier local data over the current patch state.
static bool applyEditPatch(CStorageContainer *localData, SEvalPatch &current, std::string &err)
{
	const char *traceEnv = getenv("PMB_ZONE_TRACE_VEC");
	sint32 traceVec = traceEnv ? atoi(traceEnv) : -1;
	if (traceVec >= 0 && (size_t)traceVec < current.Pm.Vecs.size())
		fprintf(stderr, "TRACE vec %d input: %a %a %a\n", traceVec,
			current.Pm.Vecs[traceVec].Pos[0], current.Pm.Vecs[traceVec].Pos[1], current.Pm.Vecs[traceVec].Pos[2]);
	CStorageContainer *pmChunk = containerChild(localData, 0x1140);
	CStorageRaw *rpChunk = rawChildOf(localData, 0x4001);
	if (!pmChunk || !rpChunk) { err = "edit patch local data without 0x1140/0x4001 (legacy record format?)"; return false; }

	SEvalPatch out;
	if (!decodePatchMesh(pmChunk->chunks(), out.Pm, err)) { err = "0x1140: " + err; return false; }
	if (!decodeRPatchMesh(nlVectorData(rpChunk->Value), rpChunk->Value.size(), out.Rp, err)) { err = "0x4001: " + err; return false; }

	// Vertex mapper (0x1130 -> child 0x1000), optional: without it the stored final patch is
	// authoritative as-is.
	CStorageContainer *mapperC = containerChild(localData, 0x1130);
	if (mapperC)
	{
		CStorageRaw *mapperRaw = rawChildOf(mapperC, 0x1000);
		if (mapperRaw)
		{
			SPmVertMapper mapper;
			if (!decodeVertMapper(nlVectorData(mapperRaw->Value), mapperRaw->Value.size(), mapper, err)) { err = "0x1130: " + err; return false; }
			// EPVertMapper::UpdateAndApplyDeltas (over the INPUT patch 'current'). The refresh
			// pass sets originalStored=TRUE for every mapped record within the input's range
			// BEFORE the apply pass, so a stored originalStored=FALSE is only load-bearing for
			// records past the input's counts.
			for (size_t i = 0; i < mapper.VertMap.size(); ++i)
			{
				SPmMapVert &m = mapper.VertMap[i];
				if (m.Vert >= 0 && i < current.Pm.Verts.size()) m.OriginalStored = 1;
				if (m.Vert < 0 || !m.OriginalStored) continue;
				if ((size_t)m.Vert >= out.Pm.Verts.size()) { err = "vert mapper output index out of range"; return false; }
				float *dst = out.Pm.Verts[m.Vert].Pos;
				if (i >= current.Pm.Verts.size())
				{
					dst[0] = dst[1] = dst[2] = 0.0f;
				}
				else
				{
					const float *src = current.Pm.Verts[i].Pos;
					dst[0] = src[0] + m.Delta[0];
					dst[1] = src[1] + m.Delta[1];
					dst[2] = src[2] + m.Delta[2];
				}
			}
			for (size_t i = 0; i < mapper.VecMap.size(); ++i)
			{
				SPmMapVert &m = mapper.VecMap[i];
				if (m.Vert >= 0 && i < current.Pm.Vecs.size()) m.OriginalStored = 1;
				if (m.Vert < 0 || !m.OriginalStored) continue;
				if ((size_t)m.Vert >= out.Pm.Vecs.size()) { err = "vec mapper output index out of range"; return false; }
				float *dst = out.Pm.Vecs[m.Vert].Pos;
				if (i >= current.Pm.Vecs.size())
				{
					dst[0] = dst[1] = dst[2] = 0.0f;
				}
				else
				{
					const float *src = current.Pm.Vecs[i].Pos;
					dst[0] = src[0] + m.Delta[0];
					dst[1] = src[1] + m.Delta[1];
					dst[2] = src[2] + m.Delta[2];
				}
			}
		}
	}

	if (traceVec >= 0 && (size_t)traceVec < out.Pm.Vecs.size())
	{
		fprintf(stderr, "TRACE vec %d output: %a %a %a\n", traceVec,
			out.Pm.Vecs[traceVec].Pos[0], out.Pm.Vecs[traceVec].Pos[1], out.Pm.Vecs[traceVec].Pos[2]);
		CStorageContainer *mapperC2 = containerChild(localData, 0x1130);
		if (mapperC2)
		{
			CStorageRaw *mr = rawChildOf(mapperC2, 0x1000);
			SPmVertMapper mv;
			std::string e2;
			if (mr && decodeVertMapper(nlVectorData(mr->Value), mr->Value.size(), mv, e2))
				for (size_t k = 0; k < mv.VecMap.size(); ++k)
					if (mv.VecMap[k].Vert == traceVec)
						fprintf(stderr, "TRACE   vecmap[%u] os=%d orig %a %a %a delta %a %a %a\n", (uint)k,
							mv.VecMap[k].OriginalStored,
							mv.VecMap[k].Original[0], mv.VecMap[k].Original[1], mv.VecMap[k].Original[2],
							mv.VecMap[k].Delta[0], mv.VecMap[k].Delta[1], mv.VecMap[k].Delta[2]);
		}
	}
	current = out;
	return true;
}

// RPatchMesh::UpdateBindingPos + PatchMesh::computeInteriors replication. In the reference
// flow the exporter ran inside 3ds Max after the scene was loaded and displayed, which runs
// RPatchMesh::UpdateBinding on the evaluated object (BuildMesh/Display): every bound vertex's
// position and cached tangents (nBefore2/nBefore/nAfter/nAfter2, nT translated by the delta)
// are recomputed from the bind target edge's bezier, then auto patches get their interior
// vectors recomputed. Stored values in the .max can be stale; the recompute is authoritative.

// Bind type enum (typeBind)
enum { BIND_25 = 0, BIND_75 = 1, BIND_50 = 2, BIND_SINGLE = 3 };

// The refresh arithmetic runs in long double: the reference exporter was a 32-bit x87 build
// whose inlined Point3 math kept intermediates at extended precision, rounding only when
// storing into the patch arrays. Stored .max data carries the bits of Max's own last refresh,
// so matching that precision model keeps the refresh bit-idempotent on already-refreshed data.
struct PtF
{
	long double x, y, z;
};

static inline PtF ptOf(const float *p) { PtF r; r.x = p[0]; r.y = p[1]; r.z = p[2]; return r; }
static inline void ptTo(float *dst, const PtF &p) { dst[0] = (float)p.x; dst[1] = (float)p.y; dst[2] = (float)p.z; }
static inline PtF ptAvg(const PtF &a, const PtF &b)
{
	PtF r;
	r.x = (a.x + b.x) / 2.0L;
	r.y = (a.y + b.y) / 2.0L;
	r.z = (a.z + b.z) / 2.0L;
	return r;
}

// InterpCenter (nel_patch_mesh.cpp): bezier edge split at 0.5.
static PtF interpCenter(const PtF &e1, const PtF &i1, const PtF &i2, const PtF &e2,
                        PtF *v1 = NULL, PtF *v2 = NULL, PtF *v3 = NULL, PtF *v4 = NULL)
{
	PtF e1i1 = ptAvg(e1, i1);
	PtF i1i2 = ptAvg(i1, i2);
	PtF i2e2 = ptAvg(i2, e2);
	PtF a = ptAvg(e1i1, i1i2);
	PtF b = ptAvg(i1i2, i2e2);
	if (v1) *v1 = e1i1;
	if (v2) *v2 = a;
	if (v3) *v3 = b;
	if (v4) *v4 = i2e2;
	return ptAvg(a, b);
}

#define PM_PATCH_AUTO 0x1

// RPatchMesh::UpdateBindingInfo replication: rebuild the bound vertices' cache indices
// (nBefore/nBefore2/nAfter/nAfter2/nT) from the topology — the stored ones can be stale, and
// the reference flow rebuilds them right before UpdateBindingPos (ValidBindingInfo is empty
// after load).
static bool isVertexInEdge(const SPatchMesh &pm, sint32 vert, sint32 edge)
{
	return pm.Edges[edge].V1 == vert || pm.Edges[edge].V2 == vert;
}

static sint32 getOtherVertexOfEdge(const SPatchMesh &pm, sint32 vert, sint32 edge)
{
	if (pm.Edges[edge].V1 == vert) return pm.Edges[edge].V2;
	if (pm.Edges[edge].V2 == vert) return pm.Edges[edge].V1;
	return -1;
}

static bool updateBindingInfo(SEvalPatch &ep, std::string &err)
{
	const SPatchMesh &pm = ep.Pm;
	SRPatchMesh &rp = ep.Rp;
	// Per-vertex incident edge lists, ascending edge index (CVertexNeighborhood::build order).
	std::vector<std::vector<uint> > neighbors(pm.Verts.size());
	for (size_t e = 0; e < pm.Edges.size(); ++e)
	{
		if (pm.Edges[e].V1 >= 0 && (size_t)pm.Edges[e].V1 < pm.Verts.size()) neighbors[pm.Edges[e].V1].push_back((uint)e);
		if (pm.Edges[e].V2 >= 0 && (size_t)pm.Edges[e].V2 < pm.Verts.size()) neighbors[pm.Edges[e].V2].push_back((uint)e);
	}
	for (size_t n = 0; n < rp.Verts.size() && n < pm.Verts.size(); ++n)
	{
		SRpoVertexBind &b = rp.Verts[n];
		if (!b.Binded) continue;
		if (b.Patch >= pm.Patches.size() || b.Edge >= 4) { err = "bind target out of range"; return false; }
		sint32 targetEdge = pm.Patches[b.Patch].Edge[b.Edge];
		if (targetEdge < 0 || (size_t)targetEdge >= pm.Edges.size()) { err = "bind edge out of range"; return false; }
		sint32 v0 = pm.Edges[targetEdge].V1;
		sint32 v1 = pm.Edges[targetEdge].V2;
		sint32 vB = -1, vA = -1, vT = -1;
		const std::vector<uint> &list = neighbors[n];
		for (size_t nn = 0; nn < list.size(); ++nn)
		{
			sint32 e = (sint32)list[nn];
			if (b.Type == BIND_25)
			{
				if (isVertexInEdge(pm, v0, e))
				{
					if (vB != -1) { err = "bind info: ambiguous before edge"; return false; }
					vB = e;
				}
				else if (isVertexInEdge(pm, (sint32)b.PrimVert, e))
				{
					if (vA != -1) { err = "bind info: ambiguous after edge"; return false; }
					vA = e;
				}
				else vT = e;
			}
			else if (b.Type == BIND_75)
			{
				if (isVertexInEdge(pm, (sint32)b.PrimVert, e))
				{
					if (vB != -1) { err = "bind info: ambiguous before edge"; return false; }
					vB = e;
				}
				else if (isVertexInEdge(pm, v1, e))
				{
					if (vA != -1) { err = "bind info: ambiguous after edge"; return false; }
					vA = e;
				}
				else vT = e;
			}
			else if (b.Type == BIND_50)
			{
				sint32 nother = getOtherVertexOfEdge(pm, (sint32)n, e);
				if (nother < 0 || (size_t)nother >= rp.Verts.size()) { err = "bind info: bad neighbor"; return false; }
				const SRpoVertexBind &ob = rp.Verts[nother];
				if (ob.Binded && ob.Type == BIND_25 && ob.PrimVert == (uint32)n)
				{
					if (vB != -1) { err = "bind info: ambiguous before edge"; return false; }
					vB = e;
				}
				else if (ob.Binded && ob.Type == BIND_75 && ob.PrimVert == (uint32)n)
				{
					if (vA != -1) { err = "bind info: ambiguous after edge"; return false; }
					vA = e;
				}
				else vT = e;
			}
			else if (b.Type == BIND_SINGLE)
			{
				if (isVertexInEdge(pm, v0, e))
				{
					if (vB != -1) { err = "bind info: ambiguous before edge"; return false; }
					vB = e;
				}
				else if (isVertexInEdge(pm, v1, e))
				{
					if (vA != -1) { err = "bind info: ambiguous after edge"; return false; }
					vA = e;
				}
				else vT = e;
			}
			else { err = "unknown bind type"; return false; }
		}
		if (vB == -1 || vA == -1 || vT == -1) { err = "bind info: incomplete neighbor classification"; return false; }
		b.Before = (pm.Edges[vB].V1 == (sint32)n) ? pm.Edges[vB].Vec12 : pm.Edges[vB].Vec21;
		b.After = (pm.Edges[vA].V1 == (sint32)n) ? pm.Edges[vA].Vec12 : pm.Edges[vA].Vec21;
		b.T = (pm.Edges[vT].V1 == (sint32)n) ? pm.Edges[vT].Vec12 : pm.Edges[vT].Vec21;
		b.Before2 = (pm.Edges[vB].V1 == (sint32)n) ? pm.Edges[vB].Vec21 : pm.Edges[vB].Vec12;
		b.After2 = (pm.Edges[vA].V1 == (sint32)n) ? pm.Edges[vA].Vec21 : pm.Edges[vA].Vec12;
	}
	return true;
}



static bool updateBindingPos(SEvalPatch &ep, std::string &err)
{
	SPatchMesh &pm = ep.Pm;
	const SRPatchMesh &rp = ep.Rp;
	for (size_t nV = 0; nV < rp.Verts.size(); ++nV)
	{
		const SRpoVertexBind &b = rp.Verts[nV];
		if (!b.Binded) continue;
		if (b.Patch >= pm.Patches.size() || b.Edge >= 4) { err = "bind target out of range"; return false; }
		sint32 edgeIndex = pm.Patches[b.Patch].Edge[b.Edge];
		if (edgeIndex < 0 || (size_t)edgeIndex >= pm.Edges.size()) { err = "bind edge out of range"; return false; }
		const SPmEdge &edge = pm.Edges[edgeIndex];
		if ((size_t)edge.V1 >= pm.Verts.size() || (size_t)edge.V2 >= pm.Verts.size()
			|| (size_t)edge.Vec12 >= pm.Vecs.size() || (size_t)edge.Vec21 >= pm.Vecs.size())
		{ err = "bind edge indices out of range"; return false; }
		// Cache indices
		bool cachesOk = b.Before2 < pm.Vecs.size() && b.Before < pm.Vecs.size()
			&& b.After < pm.Vecs.size() && b.After2 < pm.Vecs.size() && b.T < pm.Vecs.size();

		PtF vOld = ptOf(pm.Verts[nV].Pos);
		PtF e1 = ptOf(pm.Verts[edge.V1].Pos);
		PtF i1 = ptOf(pm.Vecs[edge.Vec12].Pos);
		PtF i2 = ptOf(pm.Vecs[edge.Vec21].Pos);
		PtF e2 = ptOf(pm.Verts[edge.V2].Pos);
		PtF newPos = vOld;
		switch (b.Type)
		{
		case BIND_25:
		{
			PtF v0, v1, v2, v3, v4;
			v2 = interpCenter(e1, i1, i2, e2, &v0, &v1, &v3, &v4);
			if (!cachesOk) { err = "bind cache indices out of range"; return false; }
			PtF c1, c2, c3, c4;
			newPos = interpCenter(e1, v0, v1, v2, &c1, &c2, &c3, &c4);
			ptTo(pm.Vecs[b.Before2].Pos, c1);
			ptTo(pm.Vecs[b.Before].Pos, c2);
			ptTo(pm.Vecs[b.After].Pos, c3);
			ptTo(pm.Vecs[b.After2].Pos, c4);
			break;
		}
		case BIND_50:
			newPos = interpCenter(e1, i1, i2, e2);
			break;
		case BIND_75:
		{
			PtF v0, v1, v2, v3, v4;
			v2 = interpCenter(e1, i1, i2, e2, &v0, &v1, &v3, &v4);
			if (!cachesOk) { err = "bind cache indices out of range"; return false; }
			PtF c1, c2, c3, c4;
			newPos = interpCenter(v2, v3, v4, e2, &c1, &c2, &c3, &c4);
			ptTo(pm.Vecs[b.Before2].Pos, c1);
			ptTo(pm.Vecs[b.Before].Pos, c2);
			ptTo(pm.Vecs[b.After].Pos, c3);
			ptTo(pm.Vecs[b.After2].Pos, c4);
			break;
		}
		case BIND_SINGLE:
		{
			if (!cachesOk) { err = "bind cache indices out of range"; return false; }
			PtF c1, c2, c3, c4;
			newPos = interpCenter(e1, i1, i2, e2, &c1, &c2, &c3, &c4);
			ptTo(pm.Vecs[b.Before2].Pos, c1);
			ptTo(pm.Vecs[b.Before].Pos, c2);
			ptTo(pm.Vecs[b.After].Pos, c3);
			ptTo(pm.Vecs[b.After2].Pos, c4);
			break;
		}
		default:
			err = "unknown bind type";
			return false;
		}
		ptTo(pm.Verts[nV].Pos, newPos);
		if (b.T < pm.Vecs.size())
		{
			pm.Vecs[b.T].Pos[0] = (float)(pm.Vecs[b.T].Pos[0] + (newPos.x - vOld.x));
			pm.Vecs[b.T].Pos[1] = (float)(pm.Vecs[b.T].Pos[1] + (newPos.y - vOld.y));
			pm.Vecs[b.T].Pos[2] = (float)(pm.Vecs[b.T].Pos[2] + (newPos.z - vOld.z));
		}
	}

	// patch.computeInteriors(): auto patches get interior vectors from the corner + the two
	// adjacent edge tangents (parallelogram rule, Max Patch::computeInterior).
	for (size_t i = 0; i < pm.Patches.size(); ++i)
	{
		const SPmPatch &p = pm.Patches[i];
		if (!(p.Flags & PM_PATCH_AUTO)) continue;
		for (int j = 0; j < 4; ++j)
		{
			if ((size_t)p.Interior[j] >= pm.Vecs.size()) continue;
			const float *v = pm.Verts[p.V[j]].Pos;
			const float *a = pm.Vecs[p.Vec[j * 2]].Pos;         // outgoing tangent first
			const float *c = pm.Vecs[p.Vec[(j * 2 + 7) & 7]].Pos; // incoming tangent second
			float *dst = pm.Vecs[p.Interior[j]].Pos;
			// v + (a - v) + (c - v) at extended precision, rounded at the store (x87 model).
			dst[0] = (float)((long double)v[0] + ((long double)a[0] - v[0]) + ((long double)c[0] - v[0]));
			dst[1] = (float)((long double)v[1] + ((long double)a[1] - v[1]) + ((long double)c[1] - v[1]));
			dst[2] = (float)((long double)v[2] + ((long double)a[2] - v[2]) + ((long double)c[2] - v[2]));
		}
	}
	return true;
}

// Resolve a node's evaluated NeL patch. Returns false (with err) when the node's object is not
// an RklPatch or the stack cannot be evaluated.
static bool evalNodePatch(CNodeImpl *node, SEvalPatch &out, std::string &err)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	if (!obj) { err = "node without object"; return false; }

	// Collect the modifier stack (top first) down to the base object.
	std::vector<std::pair<CSceneClass *, uint> > editMods; // (derived wrapper, modifier index)
	int guard = 8;
	while (obj && guard-- > 0)
	{
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid != CLASSID_OSM_DERIVED && cid != CLASSID_WSM_DERIVED) break;
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		if (!rm) { err = "derived object is not a reference maker"; return false; }
		CSceneClass *base = NULL;
		uint modIndex = 0;
		for (uint i = 0; i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (!r) continue;
			TSClassId scid = r->classDesc()->superClassId();
			if (scid == SCLASS_OSMODIFIER || scid == SCLASS_WSMODIFIER)
			{
				if (r->classDesc()->classId() == CLASSID_NEL_EDIT_PATCH
					|| r->classDesc()->classId() == CLASSID_NEL_PATCH_PAINT)
				{
					editMods.push_back(std::pair<CSceneClass *, uint>(obj, modIndex));
				}
				else
				{
					fprintf(stderr, "WARNING: node '%s': unsupported modifier %s on patch stack, treated as pass-through\n",
					        ucstring(node->userName()).toUtf8().c_str(), r->classDesc()->classId().toString().c_str());
				}
				++modIndex;
				continue;
			}
			base = r;
		}
		if (!base) { err = "derived object without base object"; return false; }
		obj = base;
	}

	CRklPatchObject *rpo = dynamic_cast<CRklPatchObject *>(obj);
	if (!rpo) { err = "object is not an RklPatch"; return false; }

	if (!rpo->decodePatch(out.Pm, err)) { err = "base PatchMesh: " + err; return false; }
	if (!rpo->decodeRPatch(out.Rp, err)) { err = "base RPatchMesh: " + err; return false; }

	// Apply modifiers bottom-up (reference order is top-first).
	for (size_t i = editMods.size(); i-- > 0; )
	{
		CStorageContainer *localData = editPatchLocalData(editMods[i].first, editMods[i].second);
		if (!localData) { err = "edit patch modifier without per-node local data"; return false; }
		if (!applyEditPatch(localData, out, err)) return false;
	}
	// Binding/interior refresh: gated for A/B testing against the references (the batch
	// reference exports ran 3ds Max in silent mode, where the viewport display path that
	// triggers RPatchMesh::UpdateBinding may never have run).
	if (getenv("PMB_ZONE_REFRESH_BINDS"))
	{
		if (!getenv("PMB_ZONE_NO_REBUILD_INFO"))
		{
			if (!updateBindingInfo(out, err)) return false;
		}
		if (!updateBindingPos(out, err)) return false;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// RPatchMesh::exportZone replication (plugin_max/nel_patch_lib/rpo2nel.cpp).

static int getCommonEdge(const SPatchMesh &pm, sint32 edge, const SPmPatch &patch2)
{
	for (int e = 0; e < 4; ++e)
		if (patch2.Edge[e] == edge) return e;
	return -1;
}

static int getCommonVertex(const SPatchMesh &pm, int ipatch1, int ipatch2, int *pordervtx = NULL)
{
	const SPmPatch &patch1 = pm.Patches[ipatch1];
	const SPmPatch &patch2 = pm.Patches[ipatch2];
	int i;
	for (i = 0; i < 4; ++i)
	{
		if (patch1.V[i] == patch2.V[0]) break;
		if (patch1.V[i] == patch2.V[1]) break;
		if (patch1.V[i] == patch2.V[2]) break;
		if (patch1.V[i] == patch2.V[3]) break;
	}
	if (i == 4) return -1;
	if (pordervtx) *pordervtx = i;
	return patch1.V[i];
}

static int getOtherBindedVertex(const SRPatchMesh &rpm, const SPatchMesh &pm, int ipatch1, int ipatch2, int iOtherVertex)
{
	const SPmPatch &patch1 = pm.Patches[ipatch1];
	for (int i = 0; i < 4; ++i)
	{
		const SRpoVertexBind &uiv = rpm.Verts[patch1.V[i]];
		if (uiv.Binded && (int)uiv.Patch == ipatch2 && i != iOtherVertex)
			return patch1.V[i];
	}
	return -1;
}

static int getEdge(const SPatchMesh &pm, const SPmPatch &patch, int iv1, int iv2)
{
	for (int i = 0; i < 4; ++i)
	{
		const SPmEdge &edge = pm.Edges[patch.Edge[i]];
		if (edge.V1 == iv1 && edge.V2 == iv2) return i;
		if (edge.V2 == iv1 && edge.V1 == iv2) return i;
	}
	return -1;
}

static bool buildPatchInfo(const SEvalPatch &ep, const Matrix3M &objectTM, int zoneId,
                           std::vector<NL3D::CPatchInfo> &patchinfo, std::string &err)
{
	using namespace NL3D;
	const SPatchMesh &pm = ep.Pm;
	const SRPatchMesh &rpm = ep.Rp;

	if (pm.Patches.size() != rpm.Patches.size()) { err = "PatchMesh/RPatchMesh patch count mismatch"; return false; }
	if (pm.Verts.size() != rpm.Verts.size()) { err = "PatchMesh/RPatchMesh vertex count mismatch"; return false; }

	// --- Basic checks: triple edge detection
	std::map<std::pair<uint, uint>, uint> edgeSet;
	std::set<uint> patchError;
	for (uint patch = 0; patch < pm.Patches.size(); ++patch)
	{
		for (uint edge = 0; edge < 4; ++edge)
		{
			sint32 ei = pm.Patches[patch].Edge[edge];
			if (ei < 0 || (size_t)ei >= pm.Edges.size()) { err = "patch edge index out of range"; return false; }
			uint v1 = (uint)pm.Edges[ei].V1;
			uint v2 = (uint)pm.Edges[ei].V2;
			std::pair<uint, uint> key(std::min(v1, v2), std::max(v1, v2));
			std::map<std::pair<uint, uint>, uint>::iterator ite = edgeSet.find(key);
			if (ite == edgeSet.end()) edgeSet[key] = 1;
			else
			{
				++ite->second;
				if (ite->second >= 3) patchError.insert(patch);
			}
		}
	}
	if (!patchError.empty())
	{
		err = "triple edge detected in";
		for (std::set<uint>::iterator ite = patchError.begin(); ite != patchError.end(); ++ite)
			err += NLMISC::toString(" patch %d", (*ite) + 1);
		return false;
	}

	// --- Basic exports
	patchinfo.clear();
	patchinfo.reserve(pm.Patches.size());
	CPatchInfo pi; // reused across patches, matching the original's single-instance loop
	for (uint i = 0; i < pm.Patches.size(); ++i)
	{
		const SPmPatch &pPatch = pm.Patches[i];
		if (pPatch.Type != 4) { err = NLMISC::toString("patch %u is not a quad", i); return false; }
		for (int j = 0; j < 4; ++j)
		{
			if (pPatch.V[j] < 0 || (size_t)pPatch.V[j] >= pm.Verts.size()) { err = "patch vertex index out of range"; return false; }
			Point3M v = { pm.Verts[pPatch.V[j]].Pos[0], pm.Verts[pPatch.V[j]].Pos[1], pm.Verts[pPatch.V[j]].Pos[2] };
			v = transformPoint(v, objectTM);
			pi.Patch.Vertices[j].x = v.x;
			pi.Patch.Vertices[j].y = v.y;
			pi.Patch.Vertices[j].z = v.z;
		}
		for (int j = 0; j < 8; ++j)
		{
			if (pPatch.Vec[j] < 0 || (size_t)pPatch.Vec[j] >= pm.Vecs.size()) { err = "patch tangent index out of range"; return false; }
			Point3M v = { pm.Vecs[pPatch.Vec[j]].Pos[0], pm.Vecs[pPatch.Vec[j]].Pos[1], pm.Vecs[pPatch.Vec[j]].Pos[2] };
			v = transformPoint(v, objectTM);
			pi.Patch.Tangents[j].x = v.x;
			pi.Patch.Tangents[j].y = v.y;
			pi.Patch.Tangents[j].z = v.z;
		}
		for (int j = 0; j < 4; ++j)
		{
			if (pPatch.Interior[j] < 0 || (size_t)pPatch.Interior[j] >= pm.Vecs.size()) { err = "patch interior index out of range"; return false; }
			Point3M v = { pm.Vecs[pPatch.Interior[j]].Pos[0], pm.Vecs[pPatch.Interior[j]].Pos[1], pm.Vecs[pPatch.Interior[j]].Pos[2] };
			v = transformPoint(v, objectTM);
			pi.Patch.Interiors[j].x = v.x;
			pi.Patch.Interiors[j].y = v.y;
			pi.Patch.Interiors[j].z = v.z;
		}
		const SRpoPatch &ui = rpm.Patches[i];
		pi.OrderS = 1 << ui.NbTilesU;
		pi.OrderT = 1 << ui.NbTilesV;
		pi.BindEdges[0].ZoneId = zoneId;
		pi.BindEdges[1].ZoneId = zoneId;
		pi.BindEdges[2].ZoneId = zoneId;
		pi.BindEdges[3].ZoneId = zoneId;
		pi.BaseVertices[0] = pPatch.V[0];
		pi.BaseVertices[1] = pPatch.V[1];
		pi.BaseVertices[2] = pPatch.V[2];
		pi.BaseVertices[3] = pPatch.V[3];
		// The original reuses one CPatchInfo across the loop and only resizes Tiles, leaving the
		// unclaimed CTileElement flag bits (empty-layer rotations, bit 15) to whatever the memory
		// held — that is the uninitialized-bits class masked in the reference comparison. Ours is
		// deterministic: fresh zeroed tiles per patch.
		pi.Tiles.clear();
		pi.Tiles.resize(pi.OrderS * pi.OrderT);

		// Tile colors (0xRRGGBB -> 565)
		pi.TileColors.resize((pi.OrderS + 1) * (pi.OrderT + 1));
		if (ui.Colors.size() != pi.TileColors.size()) { err = "color table size mismatch"; return false; }
		for (sint v = 0; v < pi.OrderT + 1; ++v)
		for (sint u = 0; u < pi.OrderS + 1; ++u)
		{
			uint color = ui.Colors[u + v * (pi.OrderS + 1)];
			NLMISC::CRGBA rgba((color & 0xff0000) >> 16, (color & 0x00ff00) >> 8, color & 0xff);
			pi.TileColors[u + v * (pi.OrderS + 1)].Color565 = rgba.get565();
		}

		// Tile shading
		pi.Lumels.resize((pi.OrderS * 4) * (pi.OrderT * 4), 255);
		std::fill(pi.Lumels.begin(), pi.Lumels.end(), 255);

		// Smooth flags: UI edge flag bit 0 = no-smooth
		pi.Flags &= ~0xf;
		for (int edge = 0; edge < 4; ++edge)
		{
			if (!(ui.EdgeFlags[edge] & 0x1))
				pi.Flags |= (1 << edge);
		}

		patchinfo.push_back(pi);
	}

	// --- Pass 1: bindings from the UI vertex info
	for (uint isrcpatch = 0; isrcpatch < pm.Patches.size(); ++isrcpatch)
	{
		const SPmPatch &srcpatch = pm.Patches[isrcpatch];
		for (int nv = 0; nv < 4; ++nv)
		{
			const SRpoVertexBind &uiv = rpm.Verts[srcpatch.V[nv]];
			if (!uiv.Binded) continue;

			int idstpatch = (int)uiv.Patch;
			int idstedge = (int)uiv.Edge;
			if (idstpatch < 0 || (size_t)idstpatch >= pm.Patches.size() || idstedge < 0 || idstedge >= 4)
			{ err = "bind record out of range"; return false; }
			int n = -1;
			int icv = -1;
			if (uiv.Type == BIND_SINGLE)
			{
				int orderdstvtx;
				icv = getCommonVertex(pm, idstpatch, isrcpatch, &orderdstvtx);
				if (icv == -1) { err = "invalid bind"; return false; }
				n = (idstedge == orderdstvtx) ? 0 : 1;
			}
			else if (uiv.Type == BIND_25)
			{
				n = 1;
				icv = getOtherBindedVertex(rpm, pm, isrcpatch, idstpatch, nv);
				if (icv == -1)
				{
					n = 0;
					icv = getCommonVertex(pm, idstpatch, isrcpatch);
					if (icv == -1) { err = "invalid bind"; return false; }
				}
			}
			else if (uiv.Type == BIND_75)
			{
				n = 2;
				icv = getOtherBindedVertex(rpm, pm, isrcpatch, idstpatch, nv);
				if (icv == -1)
				{
					n = 3;
					icv = getCommonVertex(pm, idstpatch, isrcpatch);
					if (icv == -1) { err = "invalid bind"; return false; }
				}
			}
			if (n != -1)
			{
				int isrcedge = getEdge(pm, srcpatch, srcpatch.V[nv], icv);
				if (isrcedge == -1) { err = "invalid edge"; return false; }
				patchinfo[idstpatch].BindEdges[idstedge].NPatchs++;
				patchinfo[idstpatch].BindEdges[idstedge].Edge[n] = isrcedge;
				patchinfo[idstpatch].BindEdges[idstedge].Next[n] = isrcpatch;
				patchinfo[isrcpatch].BindEdges[isrcedge].NPatchs = 5;
				patchinfo[isrcpatch].BindEdges[isrcedge].Edge[0] = idstedge;
				patchinfo[isrcpatch].BindEdges[isrcedge].Next[0] = idstpatch;
			}
		}
	}

	// --- Pass 2: one/one cases from the edge patch lists
	for (uint i = 0; i < pm.Patches.size(); ++i)
	{
		const SPmPatch &pPatch = pm.Patches[i];
		for (int e = 0; e < 4; ++e)
		{
			const SPmEdge &edge = pm.Edges[pPatch.Edge[e]];
			if (edge.Patches.size() > 1)
			{
				patchinfo[i].BindEdges[e].NPatchs = 1;
				sint32 other = (edge.Patches[1] != (sint32)i) ? edge.Patches[1] : edge.Patches[0];
				if (other < 0 || (size_t)other >= pm.Patches.size()) { err = "edge patch index out of range"; return false; }
				int ce = getCommonEdge(pm, pPatch.Edge[e], pm.Patches[other]);
				if (ce == -1) { err = "common edge not found"; return false; }
				patchinfo[i].BindEdges[e].Next[0] = other;
				patchinfo[i].BindEdges[e].Edge[0] = ce;
			}
		}
	}

	// --- Tile info
	for (uint i = 0; i < pm.Patches.size(); ++i)
	{
		using NL3D::CTileElement;
		NL3D::CPatchInfo &pinf = patchinfo[i];
		const SRpoPatch &ui = rpm.Patches[i];
		if (ui.Tiles.size() != (size_t)(pinf.OrderS * pinf.OrderT)) { err = "tile table size mismatch"; return false; }
		for (sint v = 0; v < pinf.OrderT; ++v)
		for (sint u = 0; u < pinf.OrderS; ++u)
		{
			const SRpoTile &desc = ui.Tiles[u + v * pinf.OrderS];
			NL3D::CTileElement &te = pinf.Tiles[u + v * pinf.OrderS];
			for (int l = 0; l < 3; ++l)
			{
				if (l >= desc.Num)
				{
					te.Tile[l] = 0xffff;
				}
				else
				{
					te.Tile[l] = desc.Layer[l].Tile;
					te.setTileOrient(l, (uint8)(desc.Layer[l].Rotate & 3));
				}
			}
			// tileDesc case = Flags bits 0-2; displace = the v9 noise byte (the loader's
			// setDisplace(noise) overrides the flags' displace bits).
			int tcase = desc.Flags & 0x7;
			if (te.Tile[0] == 0xffff)
				te.setTile256Info(false, 0);
			else
			{
				if (tcase == 0)
					te.setTile256Info(false, 0);
				else
					te.setTile256Info(true, tcase - 1);
			}
			te.setTileSubNoise(desc.Noise);
			te.setVegetableState(CTileElement::AboveWater);
		}
	}

	return true;
}

// ---------------------------------------------------------------------------------------------
// Zone writing: current CZone::serial writes version 5; the references are version 4, and the
// two encodings differ only in the version byte (verified by reference roundtrip). Serialize
// to memory and write the version byte as 4.

static bool writeZoneV4(NL3D::CZone &zone, const std::string &path)
{
	NLMISC::CMemStream mem;
	nlassert(!mem.isReading());
	zone.serial(mem);
	if (mem.length() < 1) return false;
	std::vector<uint8> buf(mem.buffer(), mem.buffer() + mem.length());
	if (buf[0] != 5)
	{
		fprintf(stderr, "WARNING: unexpected CZone serial version %u, version byte left untouched\n", buf[0]);
	}
	else
	{
		buf[0] = 4;
	}
	NLMISC::COFile f;
	if (!f.open(path)) return false;
	f.serialBuffer(nlVectorData(buf), (uint)buf.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ligozone: zone template mask (CMaxToLigo::buildZoneTemplate replication) or square mask, and
// the CZoneBankElement categories protocol of NeLLigoExportZone.

static bool buildZoneMask(const SEvalPatch &ep, const Matrix3M &objectTM, bool symmetry,
                          const NLLIGO::CLigoConfig &config, std::vector<bool> &mask, uint &width, uint &height,
                          std::string &err)
{
	// Vertices in world space
	std::vector<NLMISC::CVector> vertices(ep.Pm.Verts.size());
	for (size_t i = 0; i < ep.Pm.Verts.size(); ++i)
	{
		Point3M v = { ep.Pm.Verts[i].Pos[0], ep.Pm.Verts[i].Pos[1], ep.Pm.Verts[i].Pos[2] };
		v = transformPoint(v, objectTM);
		vertices[i].x = v.x;
		vertices[i].y = v.y;
		vertices[i].z = v.z;
	}
	// Open edges
	std::vector<std::pair<uint, uint> > indexes;
	for (size_t e = 0; e < ep.Pm.Edges.size(); ++e)
	{
		const SPmEdge &edge = ep.Pm.Edges[e];
		if (edge.Patches.size() < 2)
		{
			if (symmetry)
				indexes.push_back(std::pair<uint, uint>((uint)edge.V2, (uint)edge.V1));
			else
				indexes.push_back(std::pair<uint, uint>((uint)edge.V1, (uint)edge.V2));
		}
	}
	NLLIGO::CZoneTemplate zoneTemplate;
	NLLIGO::CLigoError errors;
	if (!zoneTemplate.build(vertices, indexes, config, errors))
	{
		err = NLMISC::toString("zone template build failed (main error %d)", (int)errors.MainError);
		return false;
	}
	zoneTemplate.getMask(mask, width, height);
	return true;
}

// getSquareMask replication (ligo plugin script.cpp): bounding box over the built zone's patch
// vertices, all cells filled.
static void buildSquareMask(const std::vector<NL3D::CPatchInfo> &patchinfo, float cellSize,
                            std::vector<bool> &mask, uint &width, uint &height)
{
	sint maxX = 1;
	sint maxY = 1;
	for (size_t i = 0; i < patchinfo.size(); ++i)
	{
		for (uint v = 0; v < 4; ++v)
		{
			sint positionX = (sint)((patchinfo[i].Patch.Vertices[v].x + cellSize / 2) / cellSize);
			sint positionY = (sint)((patchinfo[i].Patch.Vertices[v].y + cellSize / 2) / cellSize);
			if (positionX > maxX) maxX = positionX;
			if (positionY > maxY) maxY = positionY;
		}
	}
	width = (uint)maxX;
	height = (uint)maxY;
	mask.clear();
	mask.resize(width * height, true);
}

static std::string toLowerAsciiStr(const std::string &s)
{
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z') r[i] = r[i] - 'A' + 'a';
	return r;
}

static bool writeLigozone(const std::vector<std::pair<std::string, std::string> > &categoriesIn,
                          const std::vector<bool> &mask, uint width, uint height,
                          const std::string &path)
{
	std::vector<std::pair<std::string, std::string> > categories = categoriesIn;

	// Is filled ?
	uint j;
	for (j = 0; j < mask.size(); ++j)
		if (!mask[j]) break;
	categories.push_back(std::pair<std::string, std::string>("filled", (j >= mask.size()) ? "yes" : "no"));
	categories.push_back(std::pair<std::string, std::string>("square", (width == height) ? "yes" : "no"));
	categories.push_back(std::pair<std::string, std::string>("size", NLMISC::toString("%dx%d", width, height)));

	NLLIGO::CZoneBankElement bankElm;
	bankElm.setMask(mask, (uint8)width, (uint8)height);
	for (j = 0; j < categories.size(); ++j)
		bankElm.addCategory(toLowerAsciiStr(categories[j].first), toLowerAsciiStr(categories[j].second));

	NLMISC::COFile outputLigoZone;
	if (!outputLigoZone.open(path)) return false;
	try
	{
		NLMISC::COXml outputXml;
		outputXml.init(&outputLigoZone);
		bankElm.serial(outputXml);
		outputXml.flush();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: writing %s: %s\n", path.c_str(), e.what());
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// The shared exportZone driver: evaluated patch + TM + symmetry/rotate -> patchinfo (+zone).

struct SExportContext
{
	NL3D::CTileBank Bank;
	bool BankLoaded;
	std::string BankPath;
	SExportContext() : BankLoaded(false) { }
	bool loadBank(std::string &err)
	{
		if (BankLoaded) return true;
		if (BankPath.empty()) { err = "tile bank required (symmetry/rotate) but no --bank given"; return false; }
		try
		{
			NLMISC::CIFile f;
			if (!f.open(BankPath)) { err = "cannot open bank " + BankPath; return false; }
			Bank.clear();
			Bank.serial(f);
			Bank.computeXRef();
			BankLoaded = true;
			return true;
		}
		catch (const NLMISC::Exception &e)
		{
			err = std::string("bank: ") + e.what();
			return false;
		}
	}
};

static bool exportZoneToPatchInfo(const SEvalPatch &ep, const Matrix3M &objectTM, int zoneId,
                                  bool symmetry, int rotate, float snapCell, float weldThreshold,
                                  SExportContext &ctx, std::vector<NL3D::CPatchInfo> &patchinfo,
                                  std::string &err)
{
	if (!buildPatchInfo(ep, objectTM, zoneId, patchinfo, err)) return false;
	if (symmetry || rotate)
	{
		if (!ctx.loadBank(err)) return false;
		NLMISC::CMatrix sym, rot;
		sym.identity();
		rot.identity();
		if (symmetry)
			sym.scale(NLMISC::CVector(1, -1, 1));
		rot.rotateZ((float)NLMISC::Pi * (float)rotate / 2.f);
		sym *= rot;
		sym.invert();
		NL3D::CZoneSymmetrisation zoneSymmetry;
		if (!NL3D::CPatchInfo::transform(patchinfo, zoneSymmetry, ctx.Bank, symmetry, rotate, snapCell, weldThreshold, sym))
		{
			err = "CPatchInfo::transform failed";
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Node collection.

struct SZoneNode
{
	CNodeImpl *Node;
	bool Frozen;
};

static bool nodeIsRklPatch(CNodeImpl *node)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	int guard = 8;
	while (obj && guard-- > 0)
	{
		NLMISC::CClassId cid = obj->classDesc()->classId();
		if (cid == CLASSID_OSM_DERIVED || cid == CLASSID_WSM_DERIVED)
		{
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
			obj = base;
			continue;
		}
		break;
	}
	return obj && dynamic_cast<CRklPatchObject *>(obj);
}

// The ligo maxscript deletes "[NELLIGO]*" debug markers before selecting patches.
static bool isDebugMarker(CNodeImpl *node)
{
	std::string name = ucstring(node->userName()).toUtf8();
	return name.size() >= 9 && name.compare(0, 9, "[NELLIGO]") == 0;
}

// Scene-order RklPatch nodes.
static void collectZoneNodes(CScene &scene, std::vector<SZoneNode> &out)
{
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (!nodeIsRklPatch(node)) continue;
		if (isDebugMarker(node)) continue;
		SZoneNode zn;
		zn.Node = node;
		zn.Frozen = isNodeFrozen(node);
		out.push_back(zn);
	}
}

// ---------------------------------------------------------------------------------------------
// Transition scheme tables (nel_ligo_export.ms).

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
static const char *TransitionType[9] = { "CornerA", "CornerA", "Flat", "CornerA", "CornerB", "CornerB", "Flat", "Flat", "CornerB" };
static const int TransitionNumBis[9] = { 5, 4, 2, 3, 7, 6, 0, 1, 8 };

// buildTransitionMatrix (MAXScript float ops): zero the translation, optional mirror
// (post-multiply scale [-1,1,1]), optional rotateZ (90*rot degrees), then translate to
// TransitionPos*cellSize + original position.
static Matrix3M buildTransitionMatrix(const Matrix3M &mt, int transitionZone, float cellSize)
{
	Matrix3M copyMt = mt;
	float backupPos[3] = { copyMt.m[3][0], copyMt.m[3][1], copyMt.m[3][2] };
	copyMt.m[3][0] = copyMt.m[3][1] = copyMt.m[3][2] = 0.0f;

	if (TransitionScale[transitionZone])
	{
		// scale copyMt [-1,1,1]: post-multiply by diag(-1,1,1) -> negate column 0
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

	// translate copyMt (TransitionPos*cellSize + backupPos)
	copyMt.m[3][0] = TransitionPos[transitionZone][0] * cellSize + backupPos[0];
	copyMt.m[3][1] = TransitionPos[transitionZone][1] * cellSize + backupPos[1];
	copyMt.m[3][2] = TransitionPos[transitionZone][2] * cellSize + backupPos[2];
	return copyMt;
}

// Node world bbox center over the evaluated patch's control points (MAXScript node.center
// equivalent for the 160m cell classification).
static bool nodeCenter(const SEvalPatch &ep, const Matrix3M &objectTM, float center[3])
{
	if (ep.Pm.Verts.empty()) return false;
	float bbMin[3] = { 0, 0, 0 }, bbMax[3] = { 0, 0, 0 };
	bool first = true;
	for (size_t i = 0; i < ep.Pm.Verts.size(); ++i)
	{
		Point3M v = { ep.Pm.Verts[i].Pos[0], ep.Pm.Verts[i].Pos[1], ep.Pm.Verts[i].Pos[2] };
		v = transformPoint(v, objectTM);
		float p[3] = { v.x, v.y, v.z };
		for (int a = 0; a < 3; ++a)
		{
			if (first || p[a] < bbMin[a]) bbMin[a] = p[a];
			if (first || p[a] > bbMax[a]) bbMax[a] = p[a];
		}
		first = false;
	}
	for (int a = 0; a < 3; ++a)
		center[a] = (bbMin[a] + bbMax[a]) * 0.5f;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ligo protocol driver.

struct SLigoOutputs
{
	std::string ZonesDir;     // <out>/zones
	std::string ZoneLigosDir; // <out>/zoneligos
};

static bool exportLigoBrick(const SEvalPatch &ep, const Matrix3M &objectTM,
                            bool symmetry, int rotate, bool useBoundingBox, bool passable,
                            const std::vector<std::pair<std::string, std::string> > &categories,
                            const NLLIGO::CLigoConfig &config, SExportContext &ctx,
                            const SLigoOutputs &out, const std::string &name, std::string &err)
{
	// The zone
	std::vector<NL3D::CPatchInfo> patchinfo;
	if (!exportZoneToPatchInfo(ep, objectTM, 0, symmetry, rotate, config.CellSize, config.Snap, ctx, patchinfo, err))
		return false;

	// The mask
	std::vector<bool> mask;
	uint width, height;
	if (useBoundingBox)
	{
		buildSquareMask(patchinfo, config.CellSize, mask, width, height);
	}
	else
	{
		if (!buildZoneMask(ep, objectTM, symmetry, config, mask, width, height, err))
			return false;
	}

	// The .zone
	NL3D::CZone zone;
	zone.build(0, patchinfo, std::vector<NL3D::CBorderVertex>());
	std::string zonePath = out.ZonesDir + "/" + name + ".zone";
	if (!writeZoneV4(zone, zonePath)) { err = "cannot write " + zonePath; return false; }

	// The .ligozone
	std::vector<std::pair<std::string, std::string> > cats = categories;
	cats.push_back(std::pair<std::string, std::string>("passable", passable ? "yes" : "no"));
	std::string ligozonePath = out.ZoneLigosDir + "/" + name + ".ligozone";
	if (!writeLigozone(cats, mask, width, height, ligozonePath)) { err = "cannot write " + ligozonePath; return false; }

	if (g_verbose) printf("OK %s (%s)\n", zonePath.c_str(), ligozonePath.c_str());
	return true;
}

// tokenize a filename base by '-'
static void tokenize(const std::string &s, std::vector<std::string> &tokens)
{
	tokens.clear();
	std::string::size_type pos = 0;
	while (pos <= s.size())
	{
		std::string::size_type dash = s.find('-', pos);
		if (dash == std::string::npos) { tokens.push_back(s.substr(pos)); break; }
		tokens.push_back(s.substr(pos, dash - pos));
		pos = dash + 1;
	}
}

static int exportLigoFile(const std::string &inputBase, CScene &scene, const NLLIGO::CLigoConfig &config,
                          SExportContext &ctx, const SLigoOutputs &out)
{
	std::vector<std::string> tokens;
	tokenize(inputBase, tokens);

	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;

	if (tokens.size() == 3 && tokens[0] == "zonematerial")
	{
		// Single brick: the one non-frozen patch.
		std::vector<SZoneNode> sel;
		for (size_t i = 0; i < nodes.size(); ++i)
			if (!nodes[i].Frozen) sel.push_back(nodes[i]);
		if (sel.size() > 1)
		{
			fprintf(stderr, "ERROR: %s: multiple NelPatchMesh (%u), can't export\n", inputBase.c_str(), (uint)sel.size());
			return 1;
		}
		if (sel.empty())
		{
			fprintf(stderr, "WARNING: %s: no NelPatchMesh to export\n", inputBase.c_str());
			return 0;
		}
		CNodeImpl *node = sel[0].Node;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
		bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
		std::vector<std::pair<std::string, std::string> > cats;
		cats.push_back(std::pair<std::string, std::string>("zone", tokens[1] + "-" + tokens[2]));
		cats.push_back(std::pair<std::string, std::string>("material", tokens[1]));
		if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
		                     tokens[1] + "-" + tokens[2], err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		return 0;
	}
	else if (tokens.size() == 4 && tokens[0] == "zonetransition")
	{
		// Nine bricks from the transition scheme grid.
		int rc = 0;
		// Classify the non-frozen patches on the grid.
		CNodeImpl *transitionZone[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].Frozen) continue;
			CNodeImpl *node = nodes[i].Node;
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(node, ep, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
				return 1;
			}
			Matrix3M objectTM = getObjectTM(node, tmCache);
			float center[3];
			if (!nodeCenter(ep, objectTM, center))
			{
				fprintf(stderr, "ERROR: %s: empty patch\n", inputBase.c_str());
				return 1;
			}
			int x = (int)(center[0] / config.CellSize);
			int y = (int)(center[1] / config.CellSize);
			if (y < 0 || y >= 4 || x < 0 || x >= TransitionIdsLen[y] || TransitionIds[y][x] < 0)
			{
				fprintf(stderr, "ERROR: %s: node '%s' is not at a transition scheme position (cell %d,%d)\n",
				        inputBase.c_str(), ucstring(node->userName()).toUtf8().c_str(), x, y);
				return 1;
			}
			transitionZone[TransitionIds[y][x] - 1] = node;
		}
		for (int zone = 0; zone < 9; ++zone)
		{
			std::string zoneBaseName = tokens[1] + "-" + tokens[2] + "-" + tokens[3] + "-" + NLMISC::toString(zone);
			CNodeImpl *node = transitionZone[zone];
			if (!node) continue;
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(node, ep, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", zoneBaseName.c_str(), err.c_str());
				rc = 1;
				continue;
			}
			// The transformed instance: node TM through the transition matrix; the appdata
			// symmetry/rotate the maxscript sets on the instance.
			Matrix3M nodeTM = getNodeTM(node, tmCache);
			Matrix3M instTM = buildTransitionMatrix(nodeTM, zone, config.CellSize);
			// objectTM = offset * instance node TM
			Matrix3M objectTM;
			{
				// re-derive the offset from the node, then compose with the transformed node TM
				SNodeTMCache dummy;
				// offsetTM * nodeTM == getObjectTM; extract offset by objectTM * inverse(nodeTM)
				Matrix3M objTM = getObjectTM(node, tmCache);
				Matrix3M offset = objTM * inverseM3(nodeTM);
				objectTM = offset * instTM;
			}
			bool symmetry = TransitionScale[zone];
			int rotate = TransitionRot[zone];
			bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
			bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
			std::vector<std::pair<std::string, std::string> > cats;
			cats.push_back(std::pair<std::string, std::string>("zone", zoneBaseName));
			cats.push_back(std::pair<std::string, std::string>("transname", tokens[1] + "-" + tokens[2]));
			cats.push_back(std::pair<std::string, std::string>("transtype", TransitionType[zone]));
			cats.push_back(std::pair<std::string, std::string>("transnum", NLMISC::toString(TransitionNumBis[zone])));
			if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
			                     zoneBaseName, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", zoneBaseName.c_str(), err.c_str());
				rc = 1;
			}
		}
		return rc;
	}
	else if (tokens.size() == 2 && tokens[0] == "zonespecial")
	{
		std::vector<SZoneNode> sel;
		for (size_t i = 0; i < nodes.size(); ++i)
			if (!nodes[i].Frozen) sel.push_back(nodes[i]);
		if (sel.size() > 1)
		{
			fprintf(stderr, "ERROR: %s: multiple NelPatchMesh (%u), can't export\n", inputBase.c_str(), (uint)sel.size());
			return 1;
		}
		if (sel.empty())
		{
			fprintf(stderr, "WARNING: %s: no NelPatchMesh to export\n", inputBase.c_str());
			return 0;
		}
		CNodeImpl *node = sel[0].Node;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
		bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
		std::vector<std::pair<std::string, std::string> > cats;
		cats.push_back(std::pair<std::string, std::string>("zone", tokens[1]));
		cats.push_back(std::pair<std::string, std::string>("material", "special"));
		if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
		                     tokens[1], err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		return 0;
	}

	fprintf(stderr, "WARNING: %s: not a zonematerial/zonetransition/zonespecial file, nothing to do\n", inputBase.c_str());
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Direct zone export (processes/zone, ExportRykolZone with findID).

static bool findID(const std::string &name, int &zoneId)
{
	// Node name "NUM_AB..." split by '_': NameTab[1] = number, NameTab[2] = two letters.
	std::vector<std::string> parts;
	std::string::size_type pos = 0;
	while (pos <= name.size())
	{
		std::string::size_type us = name.find('_', pos);
		if (us == std::string::npos) { parts.push_back(name.substr(pos)); break; }
		parts.push_back(name.substr(pos, us - pos));
		pos = us + 1;
	}
	if (parts.size() < 2 || parts[1].size() < 2) return false;
	char l1 = parts[1][0], l2 = parts[1][1];
	if (l1 < 'A' || l1 > 'Z' || l2 < 'A' || l2 > 'Z') return false;
	int num = 0;
	if (sscanf(parts[0].c_str(), "%d", &num) != 1) return false;
	int alphaSub = ((l1 - 'A') * 26 + (l2 - 'A' + 1)) - 1;
	zoneId = (num - 1) * 256 + alphaSub;
	return true;
}

static int exportDirectZone(CScene &scene, const std::string &outPath, SExportContext &ctx, int zoneIdOverride)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		// ExportRykolZone: don't-export appdata check
		if (getScriptAppDataInt(node, NEL3D_APPDATA_DONOTEXPORT, 0)) continue;

		int zoneId = zoneIdOverride;
		if (zoneId < 0)
		{
			if (!findID(ucstring(node->userName()).toUtf8(), zoneId))
			{
				fprintf(stderr, "ERROR: cannot derive zone id from node name '%s'\n", ucstring(node->userName()).toUtf8().c_str());
				continue;
			}
		}

		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: node '%s': %s\n", ucstring(node->userName()).toUtf8().c_str(), err.c_str());
			continue;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		std::vector<NL3D::CPatchInfo> patchinfo;
		if (!exportZoneToPatchInfo(ep, objectTM, zoneId, symmetry, rotate, 160.0f, 1.0f, ctx, patchinfo, err))
		{
			fprintf(stderr, "ERROR: node '%s': %s\n", ucstring(node->userName()).toUtf8().c_str(), err.c_str());
			continue;
		}
		NL3D::CZone zone;
		zone.build(zoneId, patchinfo, std::vector<NL3D::CBorderVertex>());
		if (!writeZoneV4(zone, outPath))
		{
			fprintf(stderr, "ERROR: cannot write %s\n", outPath.c_str());
			return 1;
		}
		if (g_verbose) printf("OK %s (zone id %d)\n", outPath.c_str(), zoneId);
		return 0;
	}
	fprintf(stderr, "WARNING: no zone found in project\n");
	return 2;
}

// ---------------------------------------------------------------------------------------------
// Debug: dump the RPatchMesh tile data of the exported node's patch 0..N.

static int dumpRpoTiles(CScene &scene, uint maxPatches)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		for (uint p = 0; p < ep.Rp.Patches.size() && p < maxPatches; ++p)
		{
			const SRpoPatch &up = ep.Rp.Patches[p];
			printf("patch %u: %dx%d tiles\n", p, 1 << up.NbTilesU, 1 << up.NbTilesV);
			for (uint t = 0; t < up.Tiles.size(); ++t)
			{
				const SRpoTile &tl = up.Tiles[t];
				printf("  tile %u: num=%u flags=0x%04x noise=%u layers", t, tl.Num, tl.Flags, tl.Noise);
				for (int l = 0; l < 3; ++l)
					printf(" (%d,r%d)", tl.Layer[l].Tile, tl.Layer[l].Rotate);
				printf("\n");
			}
		}
		break;
	}
	return 0;
}

// Debug: inspect one patch of the first non-frozen node: vec ids + binds referencing them.
static int inspectPatch(CScene &scene, int patchIdx)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		if ((size_t)patchIdx >= ep.Pm.Patches.size()) { fprintf(stderr, "patch out of range\n"); return 1; }
		const SPmPatch &p = ep.Pm.Patches[patchIdx];
		printf("patch %d: v=%d,%d,%d,%d\n", patchIdx, p.V[0], p.V[1], p.V[2], p.V[3]);
		for (int j = 0; j < 8; ++j)
		{
			const float *vv = ep.Pm.Vecs[p.Vec[j]].Pos;
			printf("  tan%d = vec %d: %.6f %.6f %.6f (%a %a %a)\n", j, p.Vec[j], vv[0], vv[1], vv[2], vv[0], vv[1], vv[2]);
		}
		for (int j = 0; j < 4; ++j)
		{
			const SPmEdge &e = ep.Pm.Edges[p.Edge[j]];
			printf("  edge%d = %d: v1=%d vec12=%d vec21=%d v2=%d npatches=%u\n", j, p.Edge[j], e.V1, e.Vec12, e.Vec21, e.V2, (uint)e.Patches.size());
		}
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			printf("  bind: vert %u type %u target patch %u edge %u prim %u B2/B/A/A2/T = %u %u %u %u %u\n",
				(uint)v, b.Type, b.Patch, b.Edge, b.PrimVert, b.Before2, b.Before, b.After, b.After2, b.T);
			// is any cache one of this patch's vecs?
			for (int j = 0; j < 8; ++j)
			{
				uint32 vid = (uint32)p.Vec[j];
				if (b.Before2 == vid || b.Before == vid || b.After == vid || b.After2 == vid || b.T == vid)
					printf("    ^ cache touches patch %d tan%d (vec %u)\n", patchIdx, j, vid);
			}
		}
		// full data of this patch's four corner vertices
		for (int j = 0; j < 4; ++j)
		{
			const SPmVert &vt = ep.Pm.Verts[p.V[j]];
			printf("  corner%d = vert %d flags %d pos %a %a %a\n", j, p.V[j], vt.Flags, vt.Pos[0], vt.Pos[1], vt.Pos[2]);
			for (size_t k = 0; k < vt.Vectors.size(); ++k)
			{
				sint32 vc = vt.Vectors[k];
				const SPmVec &vv = ep.Pm.Vecs[vc];
				printf("    vec %d flags %d vert %d pos %a %a %a\n", vc, vv.Flags, vv.Vert, vv.Pos[0], vv.Pos[1], vv.Pos[2]);
			}
		}
		// verts of this patch that are bind TARGETS (edge of this patch is some bind's target)
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			if ((int)b.Patch == patchIdx)
				printf("  bind target on this patch: vert %u edge %u\n", (uint)v, b.Edge);
		}
		return 0;
	}
	return 1;
}

// Debug: trace patch 0 geometry indices + binds of the first non-frozen node.
static int debugPatch0(CScene &scene)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		CSceneClass *obj = dynamic_cast<CSceneClass *>(nodes[i].Node->getReference(1));
		// raw, without binding refresh:
		{
			// duplicate evalNodePatch without refresh
			SEvalPatch raw;
			CRklPatchObject *rpo = dynamic_cast<CRklPatchObject *>(obj);
			// go through modifier stack unwrap via evalNodePatch but disable refresh: quick hack —
			// call evalNodePatch then re-decode raw for comparison is complex; just print final.
		}
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		const SPmPatch &p0 = ep.Pm.Patches[0];
		printf("patch0: v=%d,%d,%d,%d vec=%d,%d,%d,%d,%d,%d,%d,%d int=%d,%d,%d,%d edge=%d,%d,%d,%d flags=%d\n",
			p0.V[0], p0.V[1], p0.V[2], p0.V[3],
			p0.Vec[0], p0.Vec[1], p0.Vec[2], p0.Vec[3], p0.Vec[4], p0.Vec[5], p0.Vec[6], p0.Vec[7],
			p0.Interior[0], p0.Interior[1], p0.Interior[2], p0.Interior[3],
			p0.Edge[0], p0.Edge[1], p0.Edge[2], p0.Edge[3], p0.Flags);
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			printf("bind: vert %u type %u target patch %u edge %u prim %u before2/before/after/after2/T = %u %u %u %u %u\n",
				(uint)v, b.Type, b.Patch, b.Edge, b.PrimVert, b.Before2, b.Before, b.After, b.After2, b.T);
		}
		// which verts are on patch0's edges
		for (int e = 0; e < 4; ++e)
		{
			const SPmEdge &ed = ep.Pm.Edges[p0.Edge[e]];
			printf("patch0 edge%d (index %d): v1=%d vec12=%d vec21=%d v2=%d patches=%u\n", e, p0.Edge[e], ed.V1, ed.Vec12, ed.Vec21, ed.V2, (uint)ed.Patches.size());
		}
		return 0;
	}
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Survey mode.

static int surveyFile(const std::string &inputBase, CScene &scene)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;
	uint unfrozen = 0;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		SEvalPatch ep;
		std::string err;
		bool ok = evalNodePatch(node, ep, err);
		if (!nodes[i].Frozen) ++unfrozen;
		float center[3] = { 0, 0, 0 };
		if (ok)
		{
			Matrix3M objectTM = getObjectTM(node, tmCache);
			nodeCenter(ep, objectTM, center);
		}
		printf("  node '%s'%s: %s", ucstring(node->userName()).toUtf8().c_str(),
		       nodes[i].Frozen ? " FROZEN" : "", ok ? "ok" : ("FAIL " + err).c_str());
		if (ok)
			printf(" (%u patches, %u verts, center %.1f %.1f %.1f)",
			       (uint)ep.Pm.Patches.size(), (uint)ep.Pm.Verts.size(), center[0], center[1], center[2]);
		printf("\n");
	}
	printf("%s: %u rklpatch nodes, %u unfrozen\n", inputBase.c_str(), (uint)nodes.size(), unfrozen);
	return 0;
}

// ---------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// Entry point for the max2gltf writer (this file compiled with PMB_ZONE_NO_MAIN — see
// pmb_zone_gltf.h): the standalone --ligo flow into a private temp dir, every produced file
// handed back as (relative name, bytes) for the glTF's nel_zones blob list, plus the authored
// patch sets (buildPatchInfo pre-transform, world space) for the tessellated nel_proxy viewing
// meshes. The blob is authoritative; the proxy is visualization only.

#include "pmb_zone_gltf.h"

#ifdef NL_OS_WINDOWS
#include <process.h>
#define PMB_ZONE_GETPID _getpid
#else
#include <unistd.h>
#define PMB_ZONE_GETPID getpid
#endif

static void pmbCollectDirFiles(const std::string &dir, const std::string &relPrefix,
                               std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut)
{
	std::vector<std::string> content;
	NLMISC::CPath::getPathContent(dir, false, false, true, content);
	std::sort(content.begin(), content.end());
	for (size_t i = 0; i < content.size(); ++i)
	{
		NLMISC::CIFile f;
		if (!f.open(content[i]))
			continue;
		std::vector<uint8> bytes(f.getFileSize());
		if (!bytes.empty())
			f.serialBuffer(&bytes[0], (uint)bytes.size());
		f.close();
		filesOut.push_back(std::make_pair(relPrefix + NLMISC::CFile::getFilename(content[i]), bytes));
		NLMISC::CFile::deleteFile(content[i]);
	}
}

int pmbExportZonesForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                          const std::string &bankPath,
                          float cellSize, float snap,
                          std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut,
                          std::vector<SPmbZoneProxy> *proxiesOut)
{
	// Ligo protocol names only (same filename dispatch as exportLigoFile)
	std::string inputBase = NLMISC::CFile::getFilenameWithoutExtension(maxPath);
	{
		std::vector<std::string> tokens;
		tokenize(inputBase, tokens);
		bool ligoName = (tokens.size() == 3 && tokens[0] == "zonematerial")
			|| (tokens.size() == 4 && tokens[0] == "zonetransition")
			|| (tokens.size() == 2 && tokens[0] == "zonespecial");
		if (!ligoName)
			return 0;
	}

	NL3D::registerSerial3d(); // internally guarded

	CScene &scene = *lm.Scene;

	SExportContext ctx;
	ctx.BankPath = bankPath;

	// Authored patch sets for the viewing proxies — every RklPatch node as the artist sees it
	// (frozen ones are the neighbor-reference bricks; flagged so viewers can dim them)
	if (proxiesOut)
	{
		std::vector<SZoneNode> nodes;
		collectZoneNodes(scene, nodes);
		SNodeTMCache tmCache;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(nodes[i].Node, ep, err))
			{
				fprintf(stderr, "WARNING: zone proxy '%s': %s\n",
				        ucstring(nodes[i].Node->userName()).toUtf8().c_str(), err.c_str());
				continue;
			}
			Matrix3M objectTM = getObjectTM(nodes[i].Node, tmCache);
			SPmbZoneProxy proxy;
			proxy.NodeName = ucstring(nodes[i].Node->userName()).toUtf8();
			proxy.Frozen = nodes[i].Frozen;
			if (!buildPatchInfo(ep, objectTM, 0, proxy.Patches, err))
			{
				fprintf(stderr, "WARNING: zone proxy '%s': %s\n", proxy.NodeName.c_str(), err.c_str());
				continue;
			}
			proxiesOut->push_back(proxy);
		}
	}

	// The ligo flow into a private temp dir, collected back as blobs
	char tmpBuf[256];
	snprintf(tmpBuf, sizeof(tmpBuf), "/tmp/pipeline_max_export_gltf_zone.%d", (int)PMB_ZONE_GETPID());
	std::string tmpDir = tmpBuf;
	NLLIGO::CLigoConfig config;
	config.CellSize = cellSize;
	config.Snap = snap;
	SLigoOutputs out;
	out.ZonesDir = tmpDir + "/zones";
	out.ZoneLigosDir = tmpDir + "/zoneligos";
	NLMISC::CFile::createDirectoryTree(out.ZonesDir);
	NLMISC::CFile::createDirectoryTree(out.ZoneLigosDir);
	int rc = exportLigoFile(inputBase, scene, config, ctx, out);
	pmbCollectDirFiles(out.ZonesDir, "zones/", filesOut);
	pmbCollectDirFiles(out.ZoneLigosDir, "zoneligos/", filesOut);
	::remove(out.ZonesDir.c_str());
	::remove(out.ZoneLigosDir.c_str());
	::remove(tmpDir.c_str());
	if (rc != 0)
		return -1;
	return (int)filesOut.size();
}

#ifndef PMB_ZONE_NO_MAIN
int main(int argc, char **argv)
{
	// Parse args
	std::string input;
	std::string ligoOut;
	std::string zoneOut;
	std::string bankPath;
	float cellSize = 160.0f;
	float snap = 1.0f;
	int zoneIdOverride = -1;
	bool survey = false;
	bool dumpTiles = false;
	bool debugP0 = false;
	int inspectIdx = -1;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose") g_verbose = true;
		else if (arg == "--survey") survey = true;
		else if (arg == "--dump-tiles") dumpTiles = true;
		else if (arg == "--debug-patch0") dumpTiles = false, survey = false, debugP0 = true;
		else if (arg == "--inspect" && i + 1 < argc) NLMISC::fromString(argv[++i], inspectIdx);
		else if (arg == "--ligo" && i + 1 < argc) ligoOut = argv[++i];
		else if (arg == "--zone" && i + 1 < argc) zoneOut = argv[++i];
		else if (arg == "--bank" && i + 1 < argc) bankPath = argv[++i];
		else if (arg == "--cellsize" && i + 1 < argc) NLMISC::fromString(argv[++i], cellSize);
		else if (arg == "--snap" && i + 1 < argc) NLMISC::fromString(argv[++i], snap);
		else if (arg == "--zoneid" && i + 1 < argc) NLMISC::fromString(argv[++i], zoneIdOverride);
		else if (arg[0] == '-')
		{
			fprintf(stderr, "unknown option %s\n", arg.c_str());
			return 1;
		}
		else input = arg;
	}
	if (input.empty() || (ligoOut.empty() && zoneOut.empty() && !survey && !dumpTiles && !debugP0 && inspectIdx < 0))
	{
		fprintf(stderr, "usage: pipeline_max_export_zone [--ligo <outdir> | --zone <out.zone> | --survey]\n"
		                "                                [--bank <bank.smallbank>] [--cellsize 160] [--snap 1]\n"
		                "                                [--zoneid N] [--verbose] <input.max>\n"
		                "--ligo: NeLLigoExportZone protocol by input filename (zonematerial/zonetransition/\n"
		                "        zonespecial), writing <outdir>/zones/*.zone + <outdir>/zoneligos/*.ligozone\n"
		                "--zone: ExportRykolZone protocol (zone id from the node name unless --zoneid)\n");
		return 1;
	}

	NL3D::registerSerial3d();

	// Load the max file
	CStorageOleIn in;
	if (!in.open(input)) { fprintf(stderr, "ERROR: not an OLE compound file: %s\n", input.c_str()); return 1; }

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);

	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(&reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { fprintf(stderr, "ERROR: no DllDirectory stream\n"); return 1; }
		{ CStorageStream st(b); dll.serial(st); }
		dll.parse(VersionUnknown);
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { fprintf(stderr, "ERROR: no ClassDirectory3 stream\n"); return 1; }
		{ CStorageStream st(b); cd.serial(st); }
		cd.parse(VersionUnknown);
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { fprintf(stderr, "ERROR: no Scene stream\n"); return 1; }
		{ CStorageStream st(b); scene.serial(st); }
		scene.parse(VersionUnknown);
	}

	SExportContext ctx;
	ctx.BankPath = bankPath;

	std::string inputBase = NLMISC::CFile::getFilenameWithoutExtension(input);

	int rc = 0;
	if (inspectIdx >= 0)
	{
		rc = inspectPatch(scene, inspectIdx);
	}
	else if (debugP0)
	{
		rc = debugPatch0(scene);
	}
	else if (dumpTiles)
	{
		rc = dumpRpoTiles(scene, 2);
	}
	else if (survey)
	{
		rc = surveyFile(inputBase, scene);
	}
	else if (!ligoOut.empty())
	{
		NLLIGO::CLigoConfig config;
		config.CellSize = cellSize;
		config.Snap = snap;
		SLigoOutputs out;
		out.ZonesDir = ligoOut + "/zones";
		out.ZoneLigosDir = ligoOut + "/zoneligos";
		NLMISC::CFile::createDirectoryTree(out.ZonesDir);
		NLMISC::CFile::createDirectoryTree(out.ZoneLigosDir);
		rc = exportLigoFile(inputBase, scene, config, ctx, out);
	}
	else
	{
		rc = exportDirectZone(scene, zoneOut, ctx, zoneIdOverride);
	}

	return rc;
}
#endif /* PMB_ZONE_NO_MAIN */

/* end of file */
