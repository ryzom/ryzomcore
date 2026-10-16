/**
 * \file patch_eval.h
 * \brief Zone patch-state evaluation + RPO->CPatchInfo conversion (shared implementation unit)
 * \date 2026-07-18
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The zone exporter's evaluated-patch machinery, shared with the standalone zone painter
 * (design doc §14-paint): node TM at t=0 (the zone-local no-lerp readers — deliberately NOT
 * MAXSCENE's interpolating ones, see §10j-dix), the NeL Edit Patch / NeL Patch Painter
 * modifier-stack evaluation (editPatchLocalData / applyEditPatch / evalNodePatch), the
 * binding/interior refresh at x87-model extended precision, buildPatchInfo (the
 * RPatchMesh::exportZone replication), exportZoneToPatchInfo (+ SExportContext tile-bank
 * loading) and RklPatch node collection.
 *
 * DELIBERATELY A HEADER-ONLY STATIC IMPLEMENTATION UNIT, moved verbatim from
 * pipeline_max_export_zone/main.cpp and included at the exact position the code occupied
 * there: the zone x87 tier is bit-sensitive to the translation unit's optimization context
 * (extended-precision intermediates round at codegen-chosen spill points), so the extraction
 * must not move the code into a separate TU. Every consumer gets its own static copy — the
 * zone exporter TU is textually unchanged, so its output stays bit-exact by construction.
 *
 * Include contract (the delegating-main convention): include AFTER nel/misc + nel/3d
 * (tile_bank, zone, zone_symmetrisation), the pipeline_max scene headers (node_impl, i_node,
 * derived_object, control_keyframer, control_transform, nelpatch/rkl_patch_object) and
 * pipeline_max_export_common/max_math.h + max_scene.h, with
 * `using namespace PIPELINE::MAX / ::BUILTIN / ::NELPATCH / MAXMATH;` in effect.
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

#ifndef PIPELINE_PATCH_EVAL_H
#define PIPELINE_PATCH_EVAL_H

// Scene class ids (OSM/WSM Derived wrappers are the typed CDerivedObject/CWSMDerivedObject now;
// the PRS TM controller is the typed CControlPRS since §10j-dix)
static const NLMISC::CClassId CLASSID_NEL_EDIT_PATCH(0x4dd14a3c, 0x4ac23c0c);
// NeL Patch Painter (nel_patch_paint): local data is the same final-patch shape (0x1140 +
// 0x4001) with no vertex mapper — the stored final patch is the evaluated output verbatim
// (PaintPatchData::Apply).
static const NLMISC::CClassId CLASSID_NEL_PATCH_PAINT(0x0c49560f, 0x3c3d68e7);

// Node frozen marker: empty chunk 0x0976 on the node (corpus-established: the boundary
// reference zones carry it, the exported zone does not; transition cells carry none).
#define NODE_FROZEN_CHUNK_ID 0x0976

// ---------------------------------------------------------------------------------------------
// PRS controller values at t=0 and node TM (GetNodeTM(0)); same replication as the ig exporter.

// Read a controller's default-value chunk through the typed keyframer. No raw-chunk fallback:
// the corpus-wide 0x9008 inventory (design doc §10j-dix) established that no non-keyframer
// sub-controller carries a 0x2503/0x2504/0x2505 chunk anywhere, so the historical raw-orphan
// scan here never fired. (The zone TM path otherwise keeps its own local, no-lerp t=0
// evaluation — zone is x87-precision-sensitive, so it is deliberately NOT folded onto
// MAXSCENE's interpolating readers.)
static bool readCtrlDefaultBytes(CSceneClass *sc, void *dst, size_t nBytes)
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
	readCtrlDefaultBytes(ctrl, &p, 12);
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
	readCtrlDefaultBytes(ctrl, &q, 16);
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
	if (readCtrlDefaultBytes(ctrl, buf, 28))
	{
		memcpy(&s.s, buf, 12);
		memcpy(&s.q, buf + 12, 16);
	}
	else if (readCtrlDefaultBytes(ctrl, buf, 12))
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
	if (CControlPRS *prs = dynamic_cast<CControlPRS *>(tm))
	{
		pos = posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController()));
		rot = rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController()));
		scale = scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController()));
	}
	else if (CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm))
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
	return nullptr;
}

static CStorageRaw *rawChildOf(const CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
	{
		if (it->first != id) continue;
		return dynamic_cast<CStorageRaw *>(it->second);
	}
	return nullptr;
}

// The modifier per-node local data of derived-object modifier slot modIndex: the typed slot's
// 0x2512 LocalModData container -> 0x1000 (the edit-patch local data wrapper).
static CStorageContainer *editPatchLocalData(CDerivedObject *derived, uint modIndex)
{
	CStorageContainer *data = dynamic_cast<CStorageContainer *>(derived->localModData(modIndex));
	if (!data) return nullptr;
	return containerChild(data, 0x1000);
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
                        PtF *v1 = nullptr, PtF *v2 = nullptr, PtF *v3 = nullptr, PtF *v4 = nullptr)
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
	std::vector<std::pair<CDerivedObject *, uint> > editMods; // (derived wrapper, modifier slot)
	int guard = 8;
	while (obj && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		for (uint m = 0; m < derived->modifierCount(); ++m)
		{
			NLMISC::CClassId mcid = derived->modifier(m)->classDesc()->classId();
			if (mcid == CLASSID_NEL_EDIT_PATCH || mcid == CLASSID_NEL_PATCH_PAINT)
			{
				editMods.push_back(std::pair<CDerivedObject *, uint>(derived, m));
			}
			else
			{
				fprintf(stderr, "WARNING: node '%s': unsupported modifier %s on patch stack, treated as pass-through\n",
				        ucstring(node->userName()).toUtf8().c_str(), mcid.toString().c_str());
			}
		}
		CSceneClass *base = derived->baseObject();
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

static int getCommonVertex(const SPatchMesh &pm, int ipatch1, int ipatch2, int *pordervtx = nullptr)
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
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		obj = derived->baseObject();
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

#endif /* #ifndef PIPELINE_PATCH_EVAL_H */

/* end of file */
