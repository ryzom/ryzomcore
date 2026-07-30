/**
 * \file patch_topo_subdiv.cpp
 * \brief topoSubdividePatches: exact bicubic 4-way subdivision with paint inheritance
 * \date 2026-07-29
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * See the contract in patch_topo.h. Derived from the on-disk stream semantics and textbook
 * Bezier subdivision (de Casteljau at 0.5); the bind construction reproduces the corpus
 * T-junction shape (BIND_SINGLE onto the unsplit neighbor edge).
 *
 * Conventions used throughout (established by the turn op and the eval code):
 *  - Edge e of a quad connects V[e] -> V[(e+1)&3]; Vec[2e] leaves V[e], Vec[2e+1] arrives
 *    at V[(e+1)&3]; Interior[j] sits near corner j.
 *  - The 4x4 control grid P[b][a] has V0 at (0,0), V1 at (a=3), V3 at (b=3):
 *      P[0] = V0    Vec0  Vec1  V1
 *      P[1] = Vec7  I0    I1    Vec2
 *      P[2] = Vec6  I3    I2    Vec3
 *      P[3] = V3    Vec5  Vec4  V2
 *  - Children are named by sub-domain quadrant (qa, qb); every child's ring starts at its
 *    sub-domain origin ((a_lo,b_lo), (a_hi,b_lo), (a_hi,b_hi), (a_lo,b_hi)), which is what
 *    keeps every child's grid aligned with the parent's and the tile copy rotation-free.
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

#include <cstring>
#include <map>

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

// Long-double point (x87 house style: intermediates extended, rounded at the store).
struct PtL
{
	long double x, y, z;
};

PtL ptOf(const float *p) { PtL r; r.x = p[0]; r.y = p[1]; r.z = p[2]; return r; }
void ptTo(float *dst, const PtL &p) { dst[0] = (float)p.x; dst[1] = (float)p.y; dst[2] = (float)p.z; }
PtL ptAvg(const PtL &a, const PtL &b)
{
	PtL r;
	r.x = (a.x + b.x) / 2.0L;
	r.y = (a.y + b.y) / 2.0L;
	r.z = (a.z + b.z) / 2.0L;
	return r;
}

/// De Casteljau split of one cubic at 0.5: left (c0, l1, l2, m), right (m, r1, r2, c3).
void splitCubic(const PtL c[4], PtL left[4], PtL right[4])
{
	const PtL m01 = ptAvg(c[0], c[1]);
	const PtL m12 = ptAvg(c[1], c[2]);
	const PtL m23 = ptAvg(c[2], c[3]);
	const PtL a = ptAvg(m01, m12);
	const PtL b = ptAvg(m12, m23);
	const PtL m = ptAvg(a, b);
	left[0] = c[0]; left[1] = m01; left[2] = a; left[3] = m;
	right[0] = m; right[1] = b; right[2] = m23; right[3] = c[3];
}

/// One half of a split parent edge.
struct SEdgeSplit
{
	sint32 Mid;        ///< midpoint vertex
	sint32 Half[2];    ///< edge records: [0] = origV1..Mid, [1] = Mid..origV2 (record direction)
	sint32 OrigV1, OrigV2;
	bool Bound;        ///< T-junction: original edge kept whole, Mid binds onto the neighbor
	SEdgeSplit() : Mid(-1), OrigV1(-1), OrigV2(-1), Bound(false) { Half[0] = Half[1] = -1; }
};

/// The half of a split whose endpoints are exactly {a, b}.
sint32 halfBetween(const SPatchMesh &pm, const SEdgeSplit &sp, sint32 a, sint32 b)
{
	for (int h = 0; h < 2; ++h)
	{
		const SPmEdge &e = pm.Edges[sp.Half[h]];
		if ((e.V1 == a && e.V2 == b) || (e.V1 == b && e.V2 == a))
			return sp.Half[h];
	}
	return -1;
}

void listAdd(std::vector<sint32> &list, sint32 v)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] == v)
			return;
	list.push_back(v);
}

void listRemove(std::vector<sint32> &list, sint32 v)
{
	size_t w = 0;
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] != v)
			list[w++] = list[i];
	list.resize(w);
}

void listReplace(std::vector<sint32> &list, sint32 from, sint32 to)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] == from)
			list[i] = to;
}

/// Grow a selection BitArray for appended (unselected) elements.
void growBits(SPmBitArray &sel, sint32 newCount)
{
	if (!sel.Present)
		return;
	sel.Count = newCount;
	sel.Bits.resize(newCount > 0 ? ((size_t)newCount + 31) / 32 : 0, 0);
}

/// One parent patch's 4x4 control grid, captured before any edge split mutates it.
struct SParentGrid
{
	PtL P[4][4];
};

/// Effective (evaluated) position of a vert / vec: a mapper-driven output's stored
/// position is a cache, so when the caller supplies its evaluated mirror, positions read
/// from there. Freshly appended elements sit past the mirror's counts and are unmapped,
/// so the fallthrough to the stored table is exact for them too.
PtL effVert(const SPatchMesh &pm, const SPatchMesh *evalPm, sint32 i)
{
	if (evalPm && i >= 0 && (size_t)i < evalPm->Verts.size())
		return ptOf(evalPm->Verts[i].Pos);
	return ptOf(pm.Verts[i].Pos);
}

PtL effVec(const SPatchMesh &pm, const SPatchMesh *evalPm, sint32 i)
{
	if (evalPm && i >= 0 && (size_t)i < evalPm->Vecs.size())
		return ptOf(evalPm->Vecs[i].Pos);
	return ptOf(pm.Vecs[i].Pos);
}

/// Position write to a possibly MAPPED vec output: the stored bytes always take the value,
/// and a mapped record's Delta shifts by (desired - current eval) so evaluation lands on
/// the new position (within an ulp - the input mesh itself is not addressable here).
void writeVecPos(SPatchMesh &pm, SPmVertMapper *mapper, const SPatchMesh *evalPm,
                 sint32 idx, const PtL &pos)
{
	if (mapper)
	{
		for (size_t r = 0; r < mapper->VecMap.size(); ++r)
		{
			SPmMapVert &m = mapper->VecMap[r];
			if (m.Vert != idx)
				continue;
			const PtL cur = effVec(pm, evalPm, idx);
			m.Delta[0] = (float)((long double)m.Delta[0] + (pos.x - cur.x));
			m.Delta[1] = (float)((long double)m.Delta[1] + (pos.y - cur.y));
			m.Delta[2] = (float)((long double)m.Delta[2] + (pos.z - cur.z));
			break;
		}
	}
	ptTo(pm.Vecs[idx].Pos, pos);
}

} /* anonymous namespace */

bool topoSubdividePatches(SPatchMesh &pm, SRPatchMesh &rp,
                          const std::set<uint> &patches, std::string &err,
                          SPmVertMapper *mapper, const SPatchMesh *evalPm)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (pm.HasTvPatches)
	{ err = "map-channel mesh: child TVPatch assignment is not implemented"; return false; }
	if (patches.empty())
	{ err = "nothing to subdivide"; return false; }
	if (pm.Verts.empty() || pm.Vecs.empty() || pm.Edges.empty() || pm.Patches.empty())
	{ err = "empty element table"; return false; }

	// --- Validation over the selection.
	for (std::set<uint>::const_iterator it = patches.begin(); it != patches.end(); ++it)
	{
		if (*it >= pm.Patches.size()) { err = "patch index out of range"; return false; }
		const SPmPatch &pp = pm.Patches[*it];
		if (pp.Type != 4) { err = "only quad patches can be subdivided"; return false; }
		const SRpoPatch &up = rp.Patches[*it];
		if (up.NbTilesU < 1 || up.NbTilesV < 1)
		{ err = "tile order 1 cannot be halved (subdivide refused)"; return false; }
		const size_t nt = ((size_t)1 << up.NbTilesU) * ((size_t)1 << up.NbTilesV);
		const size_t nc = (((size_t)1 << up.NbTilesU) + 1) * (((size_t)1 << up.NbTilesV) + 1);
		if (up.Tiles.size() != nt || up.Colors.size() != nc)
		{ err = "tile/color table size mismatch"; return false; }
		for (int c = 0; c < 4; ++c)
		{
			if (pp.V[c] < 0 || (size_t)pp.V[c] >= rp.Verts.size())
			{ err = "patch corner out of range"; return false; }
			if (rp.Verts[pp.V[c]].Binded)
			{ err = "a corner of the selection is bound (unbind first)"; return false; }
			if (pp.Edge[c] < 0 || (size_t)pp.Edge[c] >= pm.Edges.size())
			{ err = "patch edge out of range"; return false; }
		}
		for (int k = 0; k < 8; ++k)
			if (pp.Vec[k] < 0 || (size_t)pp.Vec[k] >= pm.Vecs.size())
			{ err = "patch tangent slot out of range"; return false; }
		for (int k = 0; k < 4; ++k)
			if (pp.Interior[k] < 0 || (size_t)pp.Interior[k] >= pm.Vecs.size())
			{ err = "patch interior slot out of range"; return false; }
	}
	// No bind record may target an edge slot of a selected patch: splitting a T-junction
	// target would strand the bound vertices.
	for (size_t v = 0; v < rp.Verts.size(); ++v)
		if (rp.Verts[v].Binded && patches.count((uint)rp.Verts[v].Patch))
		{ err = "an edge of the selection is a bind target (unbind first)"; return false; }

	// Presence/flags templates for freshly created elements (uniform per file in practice).
	const SPmVert vertTpl = pm.Verts[0];
	const SPmVec vecTpl = pm.Vecs[0];
	const SPmEdge edgeTpl = pm.Edges[0];

	// Convenience creators.
	struct SNew
	{
		SPatchMesh *Pm;
		SRPatchMesh *Rp;
		const SPmVert *VertTpl;
		const SPmVec *VecTpl;
		const SPmEdge *EdgeTpl;
		sint32 newVert(const PtL &pos)
		{
			SPmVert v;
			v.Flags = VertTpl->Flags;
			v.HasVectors = VertTpl->HasVectors;
			v.HasPatches = VertTpl->HasPatches;
			v.HasEdges = VertTpl->HasEdges;
			ptTo(v.Pos, pos);
			Pm->Verts.push_back(v);
			SRpoVertexBind b;
			memset(&b, 0, sizeof(b));
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
			Rp->Verts.push_back(b);
			return (sint32)Pm->Verts.size() - 1;
		}
		sint32 newVec(const PtL &pos, sint32 owner)
		{
			SPmVec v;
			v.Flags = VecTpl->Flags;
			v.HasVert = VecTpl->HasVert;
			v.HasPatches = VecTpl->HasPatches;
			v.Vert = v.HasVert ? owner : -1;
			ptTo(v.Pos, pos);
			Pm->Vecs.push_back(v);
			return (sint32)Pm->Vecs.size() - 1;
		}
		sint32 newInteriorVec(const PtL &pos)
		{
			// Authored interiors carry PVEC_INTERIOR (bit 0), no vertex attachment, and
			// never appear in a vertex's Vectors list - the identity the detach ownership
			// derivation and Max's own linkage machinery key on.
			SPmVec v;
			v.Flags = VecTpl->Flags | 1;
			v.HasVert = VecTpl->HasVert;
			v.HasPatches = VecTpl->HasPatches;
			v.Vert = -1;
			ptTo(v.Pos, pos);
			Pm->Vecs.push_back(v);
			return (sint32)Pm->Vecs.size() - 1;
		}
		sint32 newEdge(sint32 v1, sint32 vec12, sint32 vec21, sint32 v2)
		{
			SPmEdge e;
			e.HasPatches = EdgeTpl->HasPatches;
			e.V1 = v1; e.Vec12 = vec12; e.Vec21 = vec21; e.V2 = v2;
			Pm->Edges.push_back(e);
			return (sint32)Pm->Edges.size() - 1;
		}
	} mk;
	mk.Pm = &pm;
	mk.Rp = &rp;
	mk.VertTpl = &vertTpl;
	mk.VecTpl = &vecTpl;
	mk.EdgeTpl = &edgeTpl;

	// --- Phase 0: capture every selected patch's control grid BEFORE any edge splits.
	// Phase 1's plain split reuses the edge record and overwrites its two tangent vecs
	// with the half-curve controls, so a grid built afterwards reads halved boundary
	// tangents - exact on a uniform lattice (where the halved controls happen to agree)
	// and wrong on any sculpted patch, corrupting the centre vertex, the cross-edge
	// tangents and every child interior derived from it.
	std::map<uint, SParentGrid> grids;
	for (std::set<uint>::const_iterator it = patches.begin(); it != patches.end(); ++it)
	{
		const SPmPatch &pp = pm.Patches[*it];
		SParentGrid &g = grids[*it];
		g.P[0][0] = effVert(pm, evalPm, pp.V[0]);
		g.P[0][3] = effVert(pm, evalPm, pp.V[1]);
		g.P[3][3] = effVert(pm, evalPm, pp.V[2]);
		g.P[3][0] = effVert(pm, evalPm, pp.V[3]);
		g.P[0][1] = effVec(pm, evalPm, pp.Vec[0]);
		g.P[0][2] = effVec(pm, evalPm, pp.Vec[1]);
		g.P[1][3] = effVec(pm, evalPm, pp.Vec[2]);
		g.P[2][3] = effVec(pm, evalPm, pp.Vec[3]);
		g.P[3][2] = effVec(pm, evalPm, pp.Vec[4]);
		g.P[3][1] = effVec(pm, evalPm, pp.Vec[5]);
		g.P[2][0] = effVec(pm, evalPm, pp.Vec[6]);
		g.P[1][0] = effVec(pm, evalPm, pp.Vec[7]);
		g.P[1][1] = effVec(pm, evalPm, pp.Interior[0]);
		g.P[1][2] = effVec(pm, evalPm, pp.Interior[1]);
		g.P[2][2] = effVec(pm, evalPm, pp.Interior[2]);
		g.P[2][1] = effVec(pm, evalPm, pp.Interior[3]);
	}

	// --- Phase 1: split every edge of the selection once, globally keyed by edge index.
	std::map<sint32, SEdgeSplit> splits;
	for (std::set<uint>::const_iterator it = patches.begin(); it != patches.end(); ++it)
	{
		const SPmPatch &pp = pm.Patches[*it];
		for (int e = 0; e < 4; ++e)
		{
			const sint32 ei = pp.Edge[e];
			if (splits.count(ei))
				continue;
			// BY VALUE: every mk.new* below may reallocate the element vectors, so no
			// reference into them survives an allocation.
			const SPmEdge ed = pm.Edges[ei];
			if (ed.V1 < 0 || ed.V2 < 0 || ed.Vec12 < 0 || ed.Vec21 < 0
			    || (size_t)ed.Vec12 >= pm.Vecs.size() || (size_t)ed.Vec21 >= pm.Vecs.size())
			{ err = "edge record incomplete"; return false; }

			// Split the edge's own cubic (record direction V1 -> V2), on the EVALUATED
			// curve - the stored positions of mapped outputs are stale caches.
			PtL c[4], le[4], ri[4];
			c[0] = effVert(pm, evalPm, ed.V1);
			c[1] = effVec(pm, evalPm, ed.Vec12);
			c[2] = effVec(pm, evalPm, ed.Vec21);
			c[3] = effVert(pm, evalPm, ed.V2);
			splitCubic(c, le, ri);

			SEdgeSplit sp;
			sp.OrigV1 = ed.V1;
			sp.OrigV2 = ed.V2;
			sp.Mid = mk.newVert(le[3]);

			// The other patch on this edge, if any, and whether it splits too.
			sint32 other = -1;
			for (size_t k = 0; k < ed.Patches.size(); ++k)
				if (ed.Patches[k] != (sint32)*it)
					other = ed.Patches[k];
			sp.Bound = other >= 0 && !patches.count((uint)other);

			if (sp.Bound)
			{
				// T-junction: the neighbor keeps the WHOLE original edge (its curve and
				// tangent records untouched); the children ride two new half edges and the
				// midpoint binds onto the neighbor's edge slot. The midpoint IS the curve's
				// 0.5 point, so the bind refresh derives exactly this position.
				const sint32 l1 = mk.newVec(le[1], ed.V1);
				const sint32 l2 = mk.newVec(le[2], sp.Mid);
				const sint32 r1 = mk.newVec(ri[1], sp.Mid);
				const sint32 r2 = mk.newVec(ri[2], ed.V2);
				sp.Half[0] = mk.newEdge(ed.V1, l1, l2, sp.Mid);
				sp.Half[1] = mk.newEdge(sp.Mid, r1, r2, ed.V2);
				// Bind record: BIND_SINGLE onto (other, its slot for this edge).
				sint32 slot = -1;
				for (int s = 0; s < 4; ++s)
					if (pm.Patches[other].Edge[s] == ei) { slot = s; break; }
				if (slot < 0) { err = "neighbor slot for shared edge not found"; return false; }
				SRpoVertexBind &b = rp.Verts[sp.Mid];
				b.Binded = 1;
				b.Type = 3; // BIND_SINGLE
				b.Type2 = 3;
				b.Patch = (uint32)other;
				b.Edge = (uint32)slot;
				b.PrimVert = (uint32)sp.Mid;
			}
			else
			{
				// Plain split (open edge, or shared with another SELECTED patch): reuse the
				// record for the first half and its tangent slots for the outer controls,
				// so both vec owners stay what they were. The reused slots may be MAPPED
				// outputs, so the write goes through the delta-shifting form.
				writeVecPos(pm, mapper, evalPm, ed.Vec12, le[1]);
				const sint32 l2 = mk.newVec(le[2], sp.Mid);
				const sint32 r1 = mk.newVec(ri[1], sp.Mid);
				writeVecPos(pm, mapper, evalPm, ed.Vec21, ri[2]);
				sp.Half[1] = mk.newEdge(sp.Mid, r1, ed.Vec21, ed.V2);
				// Rewire the reused record (fresh reference, taken after all allocations).
				SPmEdge &edm = pm.Edges[ei];
				edm.Vec21 = l2;
				edm.V2 = sp.Mid;
				sp.Half[0] = ei;
				// Adjacency: origV2 leaves the reused record for the new half.
				if (pm.Verts[sp.OrigV2].HasEdges)
				{
					listRemove(pm.Verts[sp.OrigV2].Edges, ei);
					listAdd(pm.Verts[sp.OrigV2].Edges, sp.Half[1]);
				}
			}
			splits[ei] = sp;
		}
	}

	// --- Phase 2/3: per selected patch, split the control grid and build the children.
	std::vector<uint> parents(patches.begin(), patches.end());
	for (size_t pi = 0; pi < parents.size(); ++pi)
	{
		const uint p = parents[pi];
		const SPmPatch par = pm.Patches[p]; // by value: the slot is rewritten below
		const SRpoPatch upar = rp.Patches[p];

		// Control grid P[b][a], captured in phase 0 - the element tables no longer hold
		// the parent's curves (phase 1 halved the reused tangent records in place).
		const PtL(&P)[4][4] = grids[p].P;

		// Split rows at a = 0.5, then columns at b = 0.5 -> G[qa][qb] 4x4 child grids,
		// child-local layout identical to the parent's (G[qa][qb][b'][a']).
		PtL rowL[4][4], rowR[4][4];
		for (int b = 0; b < 4; ++b)
			splitCubic(P[b], rowL[b], rowR[b]);
		PtL G[2][2][4][4];
		for (int a = 0; a < 4; ++a)
		{
			PtL colInL[4], colInR[4], lo[4], hi[4];
			for (int b = 0; b < 4; ++b) { colInL[b] = rowL[b][a]; colInR[b] = rowR[b][a]; }
			splitCubic(colInL, lo, hi);
			for (int b = 0; b < 4; ++b) { G[0][0][b][a] = lo[b]; G[0][1][b][a] = hi[b]; }
			splitCubic(colInR, lo, hi);
			for (int b = 0; b < 4; ++b) { G[1][0][b][a] = lo[b]; G[1][1][b][a] = hi[b]; }
		}

		// The four boundary midpoints and their splits (already created in phase 1).
		const SEdgeSplit &s0 = splits[par.Edge[0]];
		const SEdgeSplit &s1 = splits[par.Edge[1]];
		const SEdgeSplit &s2 = splits[par.Edge[2]];
		const SEdgeSplit &s3 = splits[par.Edge[3]];
		const sint32 M0 = s0.Mid, M1 = s1.Mid, M2 = s2.Mid, M3 = s3.Mid;

		// Centre vertex and the four internal cross edges Mk -> C with fresh tangents.
		// The a=0.5 column of the b-lo half is G[0][0][.][3] == G[1][0][.][0] (same values
		// by construction); the internal tangents come off these shared columns/rows.
		const sint32 C = mk.newVert(G[0][0][3][3]);
		// M0 -> C: parent (a=.5, b: 0 -> .5): controls G[0][0][b][3], b = 0..3.
		const sint32 e0c = mk.newEdge(M0, mk.newVec(G[0][0][1][3], M0), mk.newVec(G[0][0][2][3], C), C);
		// M1 -> C: parent (a: 1 -> .5, b=.5): controls G[1][0][3][a], a = 3..0.
		const sint32 e1c = mk.newEdge(M1, mk.newVec(G[1][0][3][2], M1), mk.newVec(G[1][0][3][1], C), C);
		// M2 -> C: parent (a=.5, b: 1 -> .5): controls G[0][1][b][3], b = 3..0.
		const sint32 e2c = mk.newEdge(M2, mk.newVec(G[0][1][2][3], M2), mk.newVec(G[0][1][1][3], C), C);
		// M3 -> C: parent (a: 0 -> .5, b=.5): controls G[0][0][3][a], a = 0..3.
		const sint32 e3c = mk.newEdge(M3, mk.newVec(G[0][0][3][1], M3), mk.newVec(G[0][0][3][2], C), C);

		// Child rings and edge assignments (see the header block).
		// ring[q] = { v0, v1, v2, v3 }, edges[q] = { e0, e1, e2, e3 }, grid[q]
		const sint32 ring[4][4] = {
			{ par.V[0], M0, C, M3 },      // Q(0,0)
			{ M0, par.V[1], M1, C },      // Q(1,0)
			{ C, M1, par.V[2], M2 },      // Q(1,1)
			{ M3, C, M2, par.V[3] },      // Q(0,1)
		};
		const sint32 qedge[4][4] = {
			{ halfBetween(pm, s0, par.V[0], M0), e0c, e3c, halfBetween(pm, s3, M3, par.V[0]) },
			{ halfBetween(pm, s0, M0, par.V[1]), halfBetween(pm, s1, par.V[1], M1), e1c, e0c },
			{ e1c, halfBetween(pm, s1, M1, par.V[2]), halfBetween(pm, s2, par.V[2], M2), e2c },
			{ e3c, e2c, halfBetween(pm, s2, M2, par.V[3]), halfBetween(pm, s3, par.V[3], M3) },
		};
		const int qa[4] = { 0, 1, 1, 0 };
		const int qb[4] = { 0, 0, 1, 1 };
		for (int q = 0; q < 4; ++q)
			for (int e = 0; e < 4; ++e)
				if (qedge[q][e] < 0) { err = "internal: child half edge unresolved"; return false; }

		// Child patch slots: parent slot -> Q(0,0); three appended in q order 1..3.
		sint32 childIdx[4];
		childIdx[0] = (sint32)p;
		for (int q = 1; q < 4; ++q)
		{
			pm.Patches.push_back(par); // placeholder, filled below
			rp.Patches.push_back(SRpoPatch());
			childIdx[q] = (sint32)pm.Patches.size() - 1;
		}

		for (int q = 0; q < 4; ++q)
		{
			SPmPatch &ch = pm.Patches[childIdx[q]];
			ch = par; // Type/NumVerts/SmGroup/Flags/presence inherit
			for (int k = 0; k < 4; ++k)
			{
				ch.V[k] = ring[q][k];
				ch.Edge[k] = qedge[q][k];
			}
			// Tangents and interiors: boundary tangent SLOTS come from the child's edge
			// records so the patch and edge tables stay consistent by construction; the
			// interiors are fresh vecs from the child grid (parent interior slots are
			// reused for Q(0,0)).
			for (int k = 0; k < 4; ++k)
			{
				const SPmEdge &ce = pm.Edges[ch.Edge[k]];
				// ch edge k runs ring[k] -> ring[(k+1)&3]; the record may store either
				// direction. Vec[2k] leaves ring[k]: pick the record tangent adjacent to it.
				if (ce.V1 == ch.V[k])
				{
					ch.Vec[2 * k] = ce.Vec12;
					ch.Vec[2 * k + 1] = ce.Vec21;
				}
				else
				{
					ch.Vec[2 * k] = ce.Vec21;
					ch.Vec[2 * k + 1] = ce.Vec12;
				}
			}
			const PtL(&g)[4][4] = G[qa[q]][qb[q]];
			if (q == 0)
			{
				// Reuse the parent's interior slots (owners follow the child corners).
				ch.Interior[0] = par.Interior[0];
				ch.Interior[1] = par.Interior[1];
				ch.Interior[2] = par.Interior[2];
				ch.Interior[3] = par.Interior[3];
				// Positions only: authored interiors have Vert = -1, and the reused records
				// already carry the right identity - rewriting Vert to a corner gave them a
				// tangent-shaped attachment no authored mesh has. Reused slots may be
				// mapped, so the writes shift their deltas.
				writeVecPos(pm, mapper, evalPm, ch.Interior[0], g[1][1]);
				writeVecPos(pm, mapper, evalPm, ch.Interior[1], g[1][2]);
				writeVecPos(pm, mapper, evalPm, ch.Interior[2], g[2][2]);
				writeVecPos(pm, mapper, evalPm, ch.Interior[3], g[2][1]);
			}
			else
			{
				ch.Interior[0] = mk.newInteriorVec(g[1][1]);
				ch.Interior[1] = mk.newInteriorVec(g[1][2]);
				ch.Interior[2] = mk.newInteriorVec(g[2][2]);
				ch.Interior[3] = mk.newInteriorVec(g[2][1]);
			}
		}

		// --- rp side: tile quadrants, colors, edge flags. Child grids align with the
		// parent's, so the copy is a plain offset. The (u, v) <-> (a, b) axis pairing is
		// validated by the m43 marker gate; here: v runs along a (edge 0), u along b.
		{
			const sint32 S = 1 << upar.NbTilesU;  // u count
			const sint32 T = 1 << upar.NbTilesV;  // v count
			for (int q = 0; q < 4; ++q)
			{
				SRpoPatch &cu = rp.Patches[childIdx[q]];
				cu = SRpoPatch();
				cu.NbTilesU = upar.NbTilesU - 1;
				cu.NbTilesV = upar.NbTilesV - 1;
				const sint32 cs = S / 2, ct = T / 2;
				cu.Tiles.resize((size_t)cs * ct);
				cu.Colors.resize((size_t)(cs + 1) * (ct + 1));
				const sint32 du = qb[q] * cs;
				const sint32 dv = qa[q] * ct;
				for (sint32 v = 0; v < ct; ++v)
					for (sint32 u = 0; u < cs; ++u)
						cu.Tiles[u + v * cs] = upar.Tiles[(u + du) + (v + dv) * S];
				for (sint32 v = 0; v < ct + 1; ++v)
					for (sint32 u = 0; u < cs + 1; ++u)
						cu.Colors[u + v * (cs + 1)] = upar.Colors[(u + du) + (v + dv) * (S + 1)];
			}
			// Outer edges inherit the parent flag, internal edges start clear.
			rp.Patches[childIdx[0]].EdgeFlags[0] = upar.EdgeFlags[0];
			rp.Patches[childIdx[0]].EdgeFlags[3] = upar.EdgeFlags[3];
			rp.Patches[childIdx[1]].EdgeFlags[0] = upar.EdgeFlags[0];
			rp.Patches[childIdx[1]].EdgeFlags[1] = upar.EdgeFlags[1];
			rp.Patches[childIdx[2]].EdgeFlags[1] = upar.EdgeFlags[1];
			rp.Patches[childIdx[2]].EdgeFlags[2] = upar.EdgeFlags[2];
			rp.Patches[childIdx[3]].EdgeFlags[2] = upar.EdgeFlags[2];
			rp.Patches[childIdx[3]].EdgeFlags[3] = upar.EdgeFlags[3];
		}

		// --- Cross-reference maintenance for this parent's neighborhood.
		// Edge patch lists: children onto their edges; the parent leaves every edge it sat
		// on (reused half records inherited the parent entry - replace it).
		for (int q = 0; q < 4; ++q)
			for (int e = 0; e < 4; ++e)
			{
				SPmEdge &ce = pm.Edges[pm.Patches[childIdx[q]].Edge[e]];
				if (ce.HasPatches)
				{
					listRemove(ce.Patches, (sint32)p);
					listAdd(ce.Patches, childIdx[q]);
				}
			}
		// T-junction originals keep the neighbor only - the edge record and its two
		// tangent vecs (no child references them; the dead parent index must not stay,
		// it now names child Q(0,0)).
		for (int e = 0; e < 4; ++e)
		{
			const SEdgeSplit &sp = splits[par.Edge[e]];
			if (!sp.Bound)
				continue;
			const SPmEdge &oe = pm.Edges[par.Edge[e]];
			if (oe.HasPatches)
				listRemove(pm.Edges[par.Edge[e]].Patches, (sint32)p);
			if (oe.Vec12 >= 0 && pm.Vecs[oe.Vec12].HasPatches)
				listRemove(pm.Vecs[oe.Vec12].Patches, (sint32)p);
			if (oe.Vec21 >= 0 && pm.Vecs[oe.Vec21].HasPatches)
				listRemove(pm.Vecs[oe.Vec21].Patches, (sint32)p);
		}
		// Corner verts: parent patch -> the corner's child; ring vec/edge lists follow.
		for (int c = 0; c < 4; ++c)
		{
			SPmVert &cv = pm.Verts[par.V[c]];
			if (cv.HasPatches)
				listReplace(cv.Patches, (sint32)p, childIdx[c]);
			if (cv.HasEdges)
			{
				// The T-junction case keeps the original edge in the corner's list (it
				// still ends there, on the neighbor's side) and adds the new half.
				for (int e = 0; e < 4; ++e)
				{
					const SEdgeSplit &sp = splits[par.Edge[e]];
					const sint32 h = halfBetween(pm, sp,
						par.V[c],
						(sp.OrigV1 == par.V[c]) ? sp.Mid : (sp.OrigV2 == par.V[c] ? sp.Mid : -2));
					if (h >= 0)
						listAdd(cv.Edges, h);
				}
			}
			if (cv.HasVectors)
			{
				// New boundary tangents owned by this corner (T-junction halves).
				const SPmPatch &ch = pm.Patches[childIdx[c]];
				listAdd(cv.Vectors, ch.Vec[2 * c]);
				listAdd(cv.Vectors, ch.Vec[((c + 3) & 3) * 2 + 1]);
			}
		}
		// Midpoint and centre verts: full lists.
		const sint32 mids[4] = { M0, M1, M2, M3 };
		const sint32 crossEdges[4] = { e0c, e1c, e2c, e3c };
		for (int e = 0; e < 4; ++e)
		{
			SPmVert &mv = pm.Verts[mids[e]];
			const SEdgeSplit &sp = splits[par.Edge[e]];
			if (mv.HasEdges)
			{
				listAdd(mv.Edges, sp.Half[0]);
				listAdd(mv.Edges, sp.Half[1]);
				listAdd(mv.Edges, crossEdges[e]);
			}
			if (mv.HasPatches)
			{
				// The two children sharing this midpoint on this parent.
				for (int q = 0; q < 4; ++q)
					for (int k = 0; k < 4; ++k)
						if (ring[q][k] == mids[e])
							listAdd(mv.Patches, childIdx[q]);
			}
			if (mv.HasVectors)
			{
				for (int h = 0; h < 2; ++h)
				{
					const SPmEdge &he = pm.Edges[sp.Half[h]];
					if (he.V1 == mids[e]) listAdd(mv.Vectors, he.Vec12);
					if (he.V2 == mids[e]) listAdd(mv.Vectors, he.Vec21);
				}
				listAdd(mv.Vectors, pm.Edges[crossEdges[e]].Vec12);
			}
		}
		{
			SPmVert &cvC = pm.Verts[C];
			if (cvC.HasEdges)
				for (int e = 0; e < 4; ++e)
					listAdd(cvC.Edges, crossEdges[e]);
			if (cvC.HasPatches)
				for (int q = 0; q < 4; ++q)
					listAdd(cvC.Patches, childIdx[q]);
			if (cvC.HasVectors)
				for (int e = 0; e < 4; ++e)
					listAdd(cvC.Vectors, pm.Edges[crossEdges[e]].Vec21);
		}
		// Vec patch lists: every vec a child references lists that child (and drops the
		// parent entry it may have inherited).
		for (int q = 0; q < 4; ++q)
		{
			const SPmPatch &ch = pm.Patches[childIdx[q]];
			for (int k = 0; k < 8; ++k)
				if (pm.Vecs[ch.Vec[k]].HasPatches)
				{
					listRemove(pm.Vecs[ch.Vec[k]].Patches, (sint32)p);
					listAdd(pm.Vecs[ch.Vec[k]].Patches, childIdx[q]);
				}
			for (int k = 0; k < 4; ++k)
				if (pm.Vecs[ch.Interior[k]].HasPatches)
				{
					listRemove(pm.Vecs[ch.Interior[k]].Patches, (sint32)p);
					listAdd(pm.Vecs[ch.Interior[k]].Patches, childIdx[q]);
				}
		}
	}

	// --- Side tables sized to the new counts (new elements unselected).
	growBits(pm.VertSel, (sint32)pm.Verts.size());
	growBits(pm.PatchSel, (sint32)pm.Patches.size());
	growBits(pm.EdgeSel, (sint32)pm.Edges.size());
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
