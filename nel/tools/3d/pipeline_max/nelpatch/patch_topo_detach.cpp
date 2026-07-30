/**
 * \file patch_topo_detach.cpp
 * \brief topoDetachElements: split the selection off as its own island (detach-to-element)
 * \date 2026-07-30
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * See the contract in patch_topo.h.
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

/// Release every bind record targeting the same (patch, edge) as record `vert` (the
/// UnbindRelatedVertex rule, local copy - the delete op's helper is TU-static).
void dUnbindRelated(SRPatchMesh &rp, size_t vert)
{
	if (vert >= rp.Verts.size() || !rp.Verts[vert].Binded)
		return;
	const uint32 tp = rp.Verts[vert].Patch;
	const uint32 te = rp.Verts[vert].Edge;
	for (size_t j = 0; j < rp.Verts.size(); ++j)
	{
		SRpoVertexBind &b = rp.Verts[j];
		if (!b.Binded || b.Patch != tp || b.Edge != te)
			continue;
		b.Binded = 0;
		b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
	}
}

} /* anonymous namespace */

bool topoDetachElements(SPatchMesh &pm, SRPatchMesh &rp,
                        const std::set<uint> &sel, std::string &err)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (sel.empty())
	{ err = "nothing to detach"; return false; }
	for (std::set<uint>::const_iterator it = sel.begin(); it != sel.end(); ++it)
		if (*it >= pm.Patches.size())
		{ err = "patch index out of range"; return false; }
	if (sel.size() >= pm.Patches.size())
	{ err = "selection is the whole zone"; return false; }

	const size_t nV0 = pm.Verts.size();
	const size_t nE0 = pm.Edges.size();

	// --- Sides. A vertex used as a corner by both sides is SHARED and splits.
	std::vector<uint8> vSel(nV0, 0), vComp(nV0, 0);
	for (size_t p = 0; p < pm.Patches.size(); ++p)
	{
		const bool s = sel.count((uint)p) != 0;
		for (int k = 0; k < 4; ++k)
		{
			const sint32 v = pm.Patches[p].V[k];
			if (v < 0 || (size_t)v >= nV0)
				continue;
			if (s) vSel[v] = 1; else vComp[v] = 1;
		}
	}
	std::map<sint32, sint32> dup; // shared vertex -> its selection-side copy
	for (size_t v = 0; v < nV0; ++v)
		if (vSel[v] && vComp[v])
			dup[(sint32)v] = -1; // allocated below
	// A boundary also exists when a shared EDGE record carries both sides even if... (a
	// shared edge implies shared endpoints, covered above). No shared vertex at all means
	// the selection is already its own island.
	if (dup.empty())
	{ err = "selection is already a separate element"; return false; }

	// --- Release every bind whose anchor and target end up on different sides. The
	// original record of a SHARED anchor stays with the complement side (its selection
	// copy starts unbound), so "anchor side" is the complement for shared vertices.
	for (size_t v = 0; v < nV0; ++v)
	{
		const SRpoVertexBind &b = rp.Verts[v];
		if (!b.Binded)
			continue;
		if (b.Patch >= pm.Patches.size())
			continue;
		const bool targetSel = sel.count((uint)b.Patch) != 0;
		const bool anchorSel = vSel[v] && !vComp[v];
		const bool anchorShared = vSel[v] && vComp[v];
		if ((anchorShared && targetSel) || (!anchorShared && anchorSel != targetSel))
			dUnbindRelated(rp, v);
	}

	// --- Duplicate the shared vertices (positions and flags copy; adjacency rebuilt at
	// the end; the copy starts with a fresh unbound rp record).
	for (std::map<sint32, sint32>::iterator it = dup.begin(); it != dup.end(); ++it)
	{
		SPmVert v = pm.Verts[it->first];
		v.Vectors.clear();
		v.Patches.clear();
		v.Edges.clear();
		pm.Verts.push_back(v);
		SRpoVertexBind b;
		memset(&b, 0, sizeof(b));
		b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		rp.Verts.push_back(b);
		it->second = (sint32)pm.Verts.size() - 1;
	}

	// --- Edges. A BOUNDARY edge (patches on both sides) keeps its record and tangents
	// with the complement; the selection side rides a NEW edge with COPIED tangent vecs -
	// both sides render the same curve until one of them is edited. A selection-internal
	// edge just re-ends on the duplicates (its tangent vec owners follow below).
	std::map<sint32, sint32> edgeSelCopy; // boundary edge -> selection-side copy
	for (size_t e = 0; e < nE0; ++e)
	{
		// BY VALUE: the boundary branch appends to pm.Edges/pm.Vecs, so no reference into
		// them survives an allocation. Writes go through pm.Edges[e] directly.
		const SPmEdge ed = pm.Edges[e];
		bool anySel = false, anyComp = false;
		for (size_t k = 0; k < ed.Patches.size(); ++k)
		{
			const sint32 p = ed.Patches[k];
			if (p < 0 || (size_t)p >= pm.Patches.size())
				continue;
			if (sel.count((uint)p)) anySel = true; else anyComp = true;
		}
		if (!anySel)
			continue; // complement-only or orphan: untouched
		if (anySel && anyComp)
		{
			// Boundary: the original keeps the complement side only...
			SPmEdge ne = ed;
			ne.Patches.clear();
			{
				std::vector<sint32> keep;
				for (size_t k = 0; k < ed.Patches.size(); ++k)
				{
					if (ed.Patches[k] >= 0 && sel.count((uint)ed.Patches[k]))
						ne.Patches.push_back(ed.Patches[k]);
					else
						keep.push_back(ed.Patches[k]);
				}
				pm.Edges[e].Patches.swap(keep);
			}
			// ...and its tangent vecs drop their selection-side patch entries.
			const sint32 tv[2] = { ed.Vec12, ed.Vec21 };
			for (int t = 0; t < 2; ++t)
				if (tv[t] >= 0 && (size_t)tv[t] < pm.Vecs.size() && pm.Vecs[tv[t]].HasPatches)
				{
					std::vector<sint32> keep;
					const std::vector<sint32> &lst = pm.Vecs[tv[t]].Patches;
					for (size_t k = 0; k < lst.size(); ++k)
						if (!(lst[k] >= 0 && sel.count((uint)lst[k])))
							keep.push_back(lst[k]);
					pm.Vecs[tv[t]].Patches = keep;
				}
			// The selection side rides the copy: duplicate endpoints, copied tangent vecs.
			if (dup.count(ne.V1)) ne.V1 = dup[ne.V1];
			if (dup.count(ne.V2)) ne.V2 = dup[ne.V2];
			if (ne.Vec12 >= 0 && (size_t)ne.Vec12 < pm.Vecs.size())
			{
				SPmVec v = pm.Vecs[ne.Vec12];
				v.Patches = ne.Patches;
				if (v.HasVert) v.Vert = ne.V1;
				pm.Vecs.push_back(v);
				ne.Vec12 = (sint32)pm.Vecs.size() - 1;
			}
			if (ne.Vec21 >= 0 && (size_t)ne.Vec21 < pm.Vecs.size())
			{
				SPmVec v = pm.Vecs[ne.Vec21];
				v.Patches = ne.Patches;
				if (v.HasVert) v.Vert = ne.V2;
				pm.Vecs.push_back(v);
				ne.Vec21 = (sint32)pm.Vecs.size() - 1;
			}
			pm.Edges.push_back(ne);
			edgeSelCopy[(sint32)e] = (sint32)pm.Edges.size() - 1;
		}
		else
		{
			// Selection-internal: re-end on duplicates; tangent owners follow.
			if (dup.count(ed.V1))
			{
				if (ed.Vec12 >= 0 && (size_t)ed.Vec12 < pm.Vecs.size()
				    && pm.Vecs[ed.Vec12].HasVert && pm.Vecs[ed.Vec12].Vert == ed.V1)
					pm.Vecs[ed.Vec12].Vert = dup[ed.V1];
				pm.Edges[e].V1 = dup[ed.V1];
			}
			if (dup.count(ed.V2))
			{
				if (ed.Vec21 >= 0 && (size_t)ed.Vec21 < pm.Vecs.size()
				    && pm.Vecs[ed.Vec21].HasVert && pm.Vecs[ed.Vec21].Vert == ed.V2)
					pm.Vecs[ed.Vec21].Vert = dup[ed.V2];
				pm.Edges[e].V2 = dup[ed.V2];
			}
		}
	}

	// --- Patch rings of the selection: corners to duplicates, boundary edge slots and
	// their tangent pair to the copies, interior owners follow their corner.
	for (std::set<uint>::const_iterator it = sel.begin(); it != sel.end(); ++it)
	{
		SPmPatch &pp = pm.Patches[*it];
		for (int k = 0; k < 4; ++k)
		{
			const sint32 e = pp.Edge[k];
			if (e >= 0 && edgeSelCopy.count(e))
			{
				const SPmEdge &ne = pm.Edges[edgeSelCopy[e]];
				const SPmEdge &oe = pm.Edges[e];
				// Ring direction vs record direction decides which copy tangent lands
				// where; compare against the ORIGINAL record's endpoints (the ring still
				// names the original corners here).
				if (oe.V1 == pp.V[k])
				{
					pp.Vec[2 * k] = ne.Vec12;
					pp.Vec[2 * k + 1] = ne.Vec21;
				}
				else
				{
					pp.Vec[2 * k] = ne.Vec21;
					pp.Vec[2 * k + 1] = ne.Vec12;
				}
				pp.Edge[k] = edgeSelCopy[e];
			}
		}
		for (int k = 0; k < 4; ++k)
		{
			if (pp.Interior[k] >= 0 && (size_t)pp.Interior[k] < pm.Vecs.size()
			    && pm.Vecs[pp.Interior[k]].HasVert && dup.count(pm.Vecs[pp.Interior[k]].Vert))
				pm.Vecs[pp.Interior[k]].Vert = dup[pm.Vecs[pp.Interior[k]].Vert];
			if (pp.V[k] >= 0 && dup.count(pp.V[k]))
				pp.V[k] = dup[pp.V[k]];
		}
	}

	// --- Rebuild the adjacency lists of the split vertices (original keeps the
	// complement side, the duplicate carries the selection side). Scan order = index
	// order; only vertices whose content changed get derived ordering.
	for (std::map<sint32, sint32>::const_iterator it = dup.begin(); it != dup.end(); ++it)
	{
		const sint32 pair[2] = { it->first, it->second };
		for (int s = 0; s < 2; ++s)
		{
			SPmVert &v = pm.Verts[pair[s]];
			if (v.HasVectors) v.Vectors.clear();
			if (v.HasPatches) v.Patches.clear();
			if (v.HasEdges) v.Edges.clear();
		}
	}
	for (size_t p = 0; p < pm.Patches.size(); ++p)
		for (int k = 0; k < 4; ++k)
		{
			const sint32 v = pm.Patches[p].V[k];
			// Only split originals and their duplicates get rebuilt lists.
			if (!(dup.count(v) || v >= (sint32)nV0))
				continue;
			if (v >= 0 && (size_t)v < pm.Verts.size() && pm.Verts[v].HasPatches)
			{
				std::vector<sint32> &lst = pm.Verts[v].Patches;
				bool have = false;
				for (size_t i = 0; i < lst.size(); ++i) have = have || lst[i] == (sint32)p;
				if (!have) lst.push_back((sint32)p);
			}
		}
	for (size_t e = 0; e < pm.Edges.size(); ++e)
	{
		const SPmEdge &ed = pm.Edges[e];
		const sint32 ends[2] = { ed.V1, ed.V2 };
		for (int s = 0; s < 2; ++s)
		{
			const sint32 v = ends[s];
			if (v < 0 || (size_t)v >= pm.Verts.size())
				continue;
			if (!(dup.count(v) || v >= (sint32)nV0))
				continue;
			if (pm.Verts[v].HasEdges)
			{
				std::vector<sint32> &lst = pm.Verts[v].Edges;
				bool have = false;
				for (size_t i = 0; i < lst.size(); ++i) have = have || lst[i] == (sint32)e;
				if (!have) lst.push_back((sint32)e);
			}
		}
	}
	for (size_t vc = 0; vc < pm.Vecs.size(); ++vc)
	{
		const sint32 o = pm.Vecs[vc].Vert;
		if (o < 0 || (size_t)o >= pm.Verts.size())
			continue;
		if (!(dup.count(o) || o >= (sint32)nV0))
			continue;
		if (pm.Verts[o].HasVectors)
		{
			std::vector<sint32> &lst = pm.Verts[o].Vectors;
			bool have = false;
			for (size_t i = 0; i < lst.size(); ++i) have = have || lst[i] == (sint32)vc;
			if (!have) lst.push_back((sint32)vc);
		}
	}

	// --- Side tables sized to the new counts (new elements unselected).
	if (pm.VertSel.Present)
	{
		pm.VertSel.Count = (sint32)pm.Verts.size();
		pm.VertSel.Bits.resize(((size_t)pm.VertSel.Count + 31) / 32, 0);
	}
	if (pm.EdgeSel.Present)
	{
		pm.EdgeSel.Count = (sint32)pm.Edges.size();
		pm.EdgeSel.Bits.resize(((size_t)pm.EdgeSel.Count + 31) / 32, 0);
	}
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
