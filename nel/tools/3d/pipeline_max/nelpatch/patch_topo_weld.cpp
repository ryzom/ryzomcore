/**
 * \file patch_topo_weld.cpp
 * \brief topoWeldVerts: target-weld vertex merge with open-edge fusion (the stitch)
 * \date 2026-07-29
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * See the contract in patch_topo.h. Derived from the on-disk stream semantics; the
 * element-sweep/compaction machinery mirrors the delete transform.
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
#include "patch_topo.h"

#include <cmath>
#include <map>

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

void wListRemove(std::vector<sint32> &list, sint32 v)
{
	size_t w = 0;
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] != v)
			list[w++] = list[i];
	list.resize(w);
}

void wListAdd(std::vector<sint32> &list, sint32 v)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] == v)
			return;
	list.push_back(v);
}

void wRemapList(std::vector<sint32> &list, const std::vector<sint32> &map)
{
	// Remap entries, drop dead ones, dedupe (two entries can land on one survivor).
	std::vector<sint32> out;
	out.reserve(list.size());
	for (size_t i = 0; i < list.size(); ++i)
	{
		const sint32 v = list[i];
		if (v < 0 || (size_t)v >= map.size() || map[v] < 0)
			continue;
		wListAdd(out, map[v]);
	}
	list.swap(out);
}

sint32 wRemapIdx(sint32 v, const std::vector<sint32> &map)
{
	if (v < 0 || (size_t)v >= map.size())
		return -1;
	return map[v];
}

void wRemapBits(SPmBitArray &sel, const std::vector<sint32> &map, sint32 newCount)
{
	if (!sel.Present)
		return;
	std::vector<uint32> nb((size_t)(newCount > 0 ? (newCount + 31) / 32 : 0), 0);
	for (sint32 i = 0; i < sel.Count && (size_t)i < map.size(); ++i)
	{
		if ((size_t)(i >> 5) >= sel.Bits.size())
			break;
		if (!((sel.Bits[i >> 5] >> (i & 31)) & 1))
			continue;
		const sint32 ni = wRemapIdx(i, map);
		if (ni >= 0 && ni < newCount)
			nb[ni >> 5] |= 1u << (ni & 31);
	}
	sel.Count = newCount;
	sel.Bits.swap(nb);
}

} /* anonymous namespace */

bool topoWeldVerts(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                   const std::set<uint> &verts, float threshold,
                   STopoRemap &remap, std::string &err)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (verts.size() < 2)
	{ err = "select at least two vertices"; return false; }
	for (std::set<uint>::const_iterator it = verts.begin(); it != verts.end(); ++it)
		if (*it >= pm.Verts.size()) { err = "vertex index out of range"; return false; }

	const size_t nV = pm.Verts.size(), nVec = pm.Vecs.size(), nE = pm.Edges.size();

	// --- Cluster the selection: union-find, transitive within threshold.
	std::vector<uint> sel(verts.begin(), verts.end());
	std::map<uint, uint> parent;
	for (size_t i = 0; i < sel.size(); ++i)
		parent[sel[i]] = sel[i];
	// find with path compression (iterative, C++03)
	struct SFind
	{
		std::map<uint, uint> *P;
		uint operator()(uint v)
		{
			uint r = v;
			while ((*P)[r] != r) r = (*P)[r];
			while ((*P)[v] != r) { const uint n = (*P)[v]; (*P)[v] = r; v = n; }
			return r;
		}
	} find;
	find.P = &parent;
	const double th2 = (double)threshold * (double)threshold;
	for (size_t i = 0; i < sel.size(); ++i)
		for (size_t j = i + 1; j < sel.size(); ++j)
		{
			const float *a = pm.Verts[sel[i]].Pos, *b = pm.Verts[sel[j]].Pos;
			const double dx = (double)a[0] - b[0], dy = (double)a[1] - b[1], dz = (double)a[2] - b[2];
			if (dx * dx + dy * dy + dz * dz <= th2)
			{
				const uint ra = find(sel[i]), rb = find(sel[j]);
				if (ra != rb)
					parent[ra < rb ? rb : ra] = ra < rb ? ra : rb; // lower index is the root
			}
		}
	// vertex remap seed: cluster member -> lowest member (the target-weld survivor).
	std::vector<sint32> vertTo(nV);
	for (size_t i = 0; i < nV; ++i)
		vertTo[i] = (sint32)i;
	uint merged = 0;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		const uint r = find(sel[i]);
		if (r != sel[i])
		{
			vertTo[sel[i]] = (sint32)r;
			++merged;
		}
	}
	if (!merged)
	{ err = "nothing within threshold"; return false; }

	// --- Refusals against the merge plan. Every member of a MERGING cluster (its root
	// included) must be unbound: the merged record is the survivor's, and a silently
	// dropped or retargeted bind is a crack.
	{
		std::map<uint, uint> clusterSize;
		for (size_t i = 0; i < sel.size(); ++i)
			++clusterSize[find(sel[i])];
		for (size_t i = 0; i < sel.size(); ++i)
			if (clusterSize[find(sel[i])] >= 2 && rp.Verts[sel[i]].Binded)
			{ err = "a welding vertex is bound (unbind first)"; return false; }
	}
	for (size_t p = 0; p < pm.Patches.size(); ++p)
	{
		const SPmPatch &pp = pm.Patches[p];
		for (int a = 0; a < 4; ++a)
			for (int b = a + 1; b < 4; ++b)
				if (pp.V[a] >= 0 && pp.V[b] >= 0
				    && vertTo[pp.V[a]] == vertTo[pp.V[b]])
				{ err = "weld would degenerate a patch (two corners in one cluster)"; return false; }
	}

	// --- Edge fusion plan: after the vertex merge, edges with the same endpoint pair
	// fuse. Key on the remapped unordered pair.
	std::vector<sint32> edgeTo(nE);
	for (size_t i = 0; i < nE; ++i)
		edgeTo[i] = (sint32)i;
	std::vector<uint8> vecDead(nVec, 0);
	{
		std::map<std::pair<sint32, sint32>, sint32> firstByPair;
		for (size_t e = 0; e < nE; ++e)
		{
			const SPmEdge &ed = pm.Edges[e];
			const sint32 a = wRemapIdx(ed.V1, vertTo), b = wRemapIdx(ed.V2, vertTo);
			if (a < 0 || b < 0)
				continue;
			const std::pair<sint32, sint32> key(a < b ? a : b, a < b ? b : a);
			std::map<std::pair<sint32, sint32>, sint32>::iterator ft = firstByPair.find(key);
			if (ft == firstByPair.end())
			{
				firstByPair[key] = (sint32)e;
				continue;
			}
			// Fusing pair: both must be open (one patch each), and the union has two.
			const sint32 s = ft->second;
			SPmEdge &surv = pm.Edges[s];
			SPmEdge &dead = pm.Edges[e];
			// Only a coincidence the MERGE created fuses: a pair of edges that already
			// shared their endpoints before the weld is pre-existing mesh state, not part
			// of this op (welding an unrelated cluster must neither fuse nor refuse it).
			const bool survTouched = vertTo[surv.V1] != surv.V1 || vertTo[surv.V2] != surv.V2;
			const bool deadTouched = vertTo[dead.V1] != dead.V1 || vertTo[dead.V2] != dead.V2;
			if (!survTouched && !deadTouched)
				continue;
			if (surv.Patches.size() + dead.Patches.size() > 2)
			{ err = "weld would put more than two patches on one edge"; return false; }
			edgeTo[e] = s;
			// The dropped edge's tangents sweep; its patch rewires to the survivor's
			// tangents, orientation-aware, so both sides render one curve.
			if (dead.Vec12 >= 0 && (size_t)dead.Vec12 < nVec) vecDead[dead.Vec12] = 1;
			if (dead.Vec21 >= 0 && (size_t)dead.Vec21 < nVec) vecDead[dead.Vec21] = 1;
			for (size_t k = 0; k < dead.Patches.size(); ++k)
			{
				const sint32 pi = dead.Patches[k];
				if (pi < 0 || (size_t)pi >= pm.Patches.size())
					continue;
				SPmPatch &pp = pm.Patches[pi];
				for (int slot = 0; slot < 4; ++slot)
				{
					if (pp.Edge[slot] != (sint32)e)
						continue;
					pp.Edge[slot] = s;
					// Ring direction of this slot: V[slot] -> V[(slot+1)&3]; match against
					// the SURVIVOR record's direction (compare through the vert remap - the
					// survivor's endpoints may be cluster roots the ring still names by
					// their pre-merge indices).
					const sint32 ringA = wRemapIdx(pp.V[slot], vertTo);
					const sint32 survA = wRemapIdx(surv.V1, vertTo);
					if (ringA == survA)
					{
						pp.Vec[2 * slot] = surv.Vec12;
						pp.Vec[2 * slot + 1] = surv.Vec21;
					}
					else
					{
						pp.Vec[2 * slot] = surv.Vec21;
						pp.Vec[2 * slot + 1] = surv.Vec12;
					}
				}
				wListAdd(surv.Patches, pi);
			}
			dead.Patches.clear();
		}
	}

	// --- Compaction maps.
	remap.Vert.assign(nV, -1);
	remap.Vec.assign(nVec, -1);
	remap.Edge.assign(nE, -1);
	remap.Patch.assign(pm.Patches.size(), -1);
	{
		sint32 w = 0;
		for (size_t i = 0; i < nV; ++i)
			remap.Vert[i] = (vertTo[i] == (sint32)i) ? w++ : -1;
		// merged members land on their root's new slot
		for (size_t i = 0; i < nV; ++i)
			if (vertTo[i] != (sint32)i)
				remap.Vert[i] = remap.Vert[vertTo[i]];
		w = 0;
		for (size_t i = 0; i < nVec; ++i)
			remap.Vec[i] = vecDead[i] ? -1 : w++;
		w = 0;
		for (size_t i = 0; i < nE; ++i)
			remap.Edge[i] = (edgeTo[i] == (sint32)i) ? w++ : -1;
		for (size_t i = 0; i < nE; ++i)
			if (edgeTo[i] != (sint32)i)
				remap.Edge[i] = remap.Edge[edgeTo[i]];
		for (size_t i = 0; i < pm.Patches.size(); ++i)
			remap.Patch[i] = (sint32)i;
	}

	// --- rp side first (old indices live): survivors keep their bind records; merged
	// members' records drop with them. Remap the survivors' targets (patches unchanged,
	// PrimVert through the vert map).
	{
		size_t w = 0;
		for (size_t i = 0; i < rp.Verts.size(); ++i)
			if (vertTo[i] == (sint32)i)
			{
				if (w != i) rp.Verts[w] = rp.Verts[i];
				++w;
			}
		rp.Verts.resize(w);
		for (size_t i = 0; i < rp.Verts.size(); ++i)
		{
			SRpoVertexBind &b = rp.Verts[i];
			if (!b.Binded)
				continue;
			const sint32 np = wRemapIdx((sint32)b.PrimVert, remap.Vert);
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
			if (np < 0)
			{
				b.Binded = 0;
				continue;
			}
			b.PrimVert = (uint32)np;
		}
	}

	// --- PatchMesh compaction + cross-reference rewrite.
	{
		// Merge the dropped members' adjacency into their survivors before compacting.
		for (size_t i = 0; i < nV; ++i)
		{
			if (vertTo[i] == (sint32)i)
				continue;
			SPmVert &from = pm.Verts[i];
			SPmVert &to = pm.Verts[vertTo[i]];
			for (size_t k = 0; k < from.Vectors.size(); ++k) wListAdd(to.Vectors, from.Vectors[k]);
			for (size_t k = 0; k < from.Patches.size(); ++k) wListAdd(to.Patches, from.Patches[k]);
			for (size_t k = 0; k < from.Edges.size(); ++k) wListAdd(to.Edges, from.Edges[k]);
		}
		size_t w = 0;
		for (size_t i = 0; i < nV; ++i)
		{
			if (vertTo[i] != (sint32)i)
				continue;
			SPmVert &v = pm.Verts[i];
			wRemapList(v.Vectors, remap.Vec);
			wRemapList(v.Patches, remap.Patch);
			wRemapList(v.Edges, remap.Edge);
			if (w != i) pm.Verts[w] = v;
			++w;
		}
		pm.Verts.resize(w);
		w = 0;
		for (size_t i = 0; i < nVec; ++i)
		{
			if (vecDead[i])
				continue;
			SPmVec &v = pm.Vecs[i];
			v.Vert = wRemapIdx(v.Vert, remap.Vert);
			wRemapList(v.Patches, remap.Patch);
			if (w != i) pm.Vecs[w] = v;
			++w;
		}
		pm.Vecs.resize(w);
		w = 0;
		for (size_t i = 0; i < nE; ++i)
		{
			if (edgeTo[i] != (sint32)i)
				continue;
			SPmEdge &e = pm.Edges[i];
			e.V1 = wRemapIdx(e.V1, remap.Vert);
			e.Vec12 = wRemapIdx(e.Vec12, remap.Vec);
			e.Vec21 = wRemapIdx(e.Vec21, remap.Vec);
			e.V2 = wRemapIdx(e.V2, remap.Vert);
			wRemapList(e.Patches, remap.Patch);
			if (w != i) pm.Edges[w] = e;
			++w;
		}
		pm.Edges.resize(w);
		for (size_t i = 0; i < pm.Patches.size(); ++i)
		{
			SPmPatch &p = pm.Patches[i];
			for (int k = 0; k < 4; ++k)
			{
				p.V[k] = wRemapIdx(p.V[k], remap.Vert);
				p.Interior[k] = wRemapIdx(p.Interior[k], remap.Vec);
				p.Edge[k] = wRemapIdx(p.Edge[k], remap.Edge);
			}
			for (int k = 0; k < 8; ++k)
				p.Vec[k] = wRemapIdx(p.Vec[k], remap.Vec);
		}
	}

	// --- Side tables.
	wRemapBits(pm.VertSel, remap.Vert, (sint32)pm.Verts.size());
	wRemapBits(pm.EdgeSel, remap.Edge, (sint32)pm.Edges.size());
	// PatchSel counts are unchanged (identity map).

	// --- Mapper: dropped outputs flip to -1 (two inputs may not drive one output), the
	// rest remap. The survivor keeps whichever input record already drove it.
	if (mapper)
	{
		for (size_t i = 0; i < mapper->VertMap.size(); ++i)
		{
			SPmMapVert &m = mapper->VertMap[i];
			if (m.Vert < 0)
				continue;
			if ((size_t)m.Vert < nV && vertTo[m.Vert] != m.Vert)
				m.Vert = -1; // its output merged away; the survivor has its own driver
			else
				m.Vert = wRemapIdx(m.Vert, remap.Vert);
		}
		for (size_t i = 0; i < mapper->VecMap.size(); ++i)
		{
			SPmMapVert &m = mapper->VecMap[i];
			if (m.Vert < 0)
				continue;
			if ((size_t)m.Vert < nVec && vecDead[m.Vert])
				m.Vert = -1;
			else
				m.Vert = wRemapIdx(m.Vert, remap.Vec);
		}
	}
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
