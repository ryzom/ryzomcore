/**
 * \file patch_topo.cpp
 * \brief Topological transforms over the decoded PatchMesh + RPatchMesh pair
 * \date 2026-07-29
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * See patch_topo.h for the contract.
 */

/*
 * Copyright (C) 2026 by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "patch_topo.h"

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

/// Compact a cross-reference list through a map: entries for deleted elements drop, the
/// rest rewrite. Order is preserved.
void remapList(std::vector<sint32> &list, const std::vector<sint32> &map)
{
	size_t w = 0;
	for (size_t r = 0; r < list.size(); ++r)
	{
		const sint32 v = list[r];
		if (v < 0 || (size_t)v >= map.size())
			continue; // out-of-table entries are dropped rather than kept stale
		if (map[v] < 0)
			continue;
		list[w++] = map[v];
	}
	list.resize(w);
}

sint32 remapIdx(sint32 v, const std::vector<sint32> &map)
{
	if (v < 0 || (size_t)v >= map.size())
		return -1;
	return map[v];
}

/// Compact a selection BitArray through a map (survivor keeps its bit).
void remapBits(SPmBitArray &sel, const std::vector<sint32> &map, sint32 newCount)
{
	if (!sel.Present)
		return;
	std::vector<uint32> nb((size_t)(newCount > 0 ? (newCount + 31) / 32 : 0), 0);
	const sint32 oldCount = sel.Count;
	for (sint32 i = 0; i < oldCount && (size_t)i < map.size(); ++i)
	{
		if ((size_t)(i >> 5) >= sel.Bits.size())
			break;
		if (!((sel.Bits[i >> 5] >> (i & 31)) & 1))
			continue;
		const sint32 ni = remapIdx(i, map);
		if (ni >= 0 && ni < newCount)
			nb[ni >> 5] |= 1u << (ni & 31);
	}
	sel.Count = newCount;
	sel.Bits.swap(nb);
}

/// Release every bind record targeting the same (patch, edge) as record `vert`
/// (UnbindRelatedVertex port; caches to -1, target fields kept, like the interactive op).
void unbindRelated(SRPatchMesh &rp, size_t vert)
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

bool topoDeletePatches(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                       const std::set<uint> &delPatches, STopoRemap &remap, std::string &err)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (delPatches.empty())
	{ err = "nothing to delete"; return false; }
	for (std::set<uint>::const_iterator it = delPatches.begin(); it != delPatches.end(); ++it)
		if (*it >= pm.Patches.size())
		{ err = "patch index out of range"; return false; }
	if (delPatches.size() >= pm.Patches.size())
	{ err = "deleting every patch would leave an empty object"; return false; }

	const size_t nV = pm.Verts.size(), nVec = pm.Vecs.size();
	const size_t nE = pm.Edges.size(), nP = pm.Patches.size();

	// --- The sweep rule: touched by a deleted patch, kept only if a survivor still uses it.
	std::vector<uint8> vTouched(nV, 0), vecTouched(nVec, 0), eTouched(nE, 0);
	std::vector<uint8> vUsed(nV, 0), vecUsed(nVec, 0), eUsed(nE, 0);
	for (size_t p = 0; p < nP; ++p)
	{
		const bool del = delPatches.count((uint)p) != 0;
		const SPmPatch &pp = pm.Patches[p];
		std::vector<uint8> &vs = del ? vTouched : vUsed;
		std::vector<uint8> &vcs = del ? vecTouched : vecUsed;
		std::vector<uint8> &es = del ? eTouched : eUsed;
		for (int i = 0; i < 4; ++i)
		{
			if (pp.V[i] >= 0 && (size_t)pp.V[i] < nV) vs[pp.V[i]] = 1;
			if (pp.Interior[i] >= 0 && (size_t)pp.Interior[i] < nVec) vcs[pp.Interior[i]] = 1;
			if (pp.Edge[i] >= 0 && (size_t)pp.Edge[i] < nE) es[pp.Edge[i]] = 1;
		}
		for (int i = 0; i < 8; ++i)
			if (pp.Vec[i] >= 0 && (size_t)pp.Vec[i] < nVec) vcs[pp.Vec[i]] = 1;
	}

	// --- Remap tables (compaction preserves order).
	remap.Vert.assign(nV, -1);
	remap.Vec.assign(nVec, -1);
	remap.Edge.assign(nE, -1);
	remap.Patch.assign(nP, -1);
	{
		sint32 w = 0;
		for (size_t i = 0; i < nV; ++i)
			remap.Vert[i] = (vTouched[i] && !vUsed[i]) ? -1 : w++;
		w = 0;
		for (size_t i = 0; i < nVec; ++i)
			remap.Vec[i] = (vecTouched[i] && !vecUsed[i]) ? -1 : w++;
		w = 0;
		for (size_t i = 0; i < nE; ++i)
			remap.Edge[i] = (eTouched[i] && !eUsed[i]) ? -1 : w++;
		w = 0;
		for (size_t i = 0; i < nP; ++i)
			remap.Patch[i] = delPatches.count((uint)i) ? -1 : w++;
	}

	// --- RPatchMesh side first (DeleteAndSweep port), while old indices are still live.
	// Release the bind groups of deleted bound vertices...
	for (size_t i = 0; i < nV; ++i)
		if (remap.Vert[i] < 0)
			unbindRelated(rp, i);
	// ...and of the deleted patches' bound corners.
	for (std::set<uint>::const_iterator it = delPatches.begin(); it != delPatches.end(); ++it)
	{
		const SPmPatch &pp = pm.Patches[*it];
		for (int c = 0; c < 4; ++c)
			if (pp.V[c] >= 0 && (size_t)pp.V[c] < rp.Verts.size())
				unbindRelated(rp, (size_t)pp.V[c]);
	}
	// Compact the per-patch paint records and the per-vertex bind records.
	{
		size_t w = 0;
		for (size_t i = 0; i < rp.Patches.size(); ++i)
			if (remap.Patch[i] >= 0)
			{
				if (w != i) rp.Patches[w] = rp.Patches[i];
				++w;
			}
		rp.Patches.resize(w);
		w = 0;
		for (size_t i = 0; i < rp.Verts.size(); ++i)
			if (remap.Vert[i] >= 0)
			{
				if (w != i) rp.Verts[w] = rp.Verts[i];
				++w;
			}
		rp.Verts.resize(w);
	}
	// Remap the surviving binds; release when the anchor or the target patch died. The
	// caches reset regardless - they hold VECTOR indices this transform just moved, and
	// both the legacy loader and the eval refresh rebuild them from topology.
	for (size_t i = 0; i < rp.Verts.size(); ++i)
	{
		SRpoVertexBind &b = rp.Verts[i];
		if (!b.Binded)
			continue;
		const sint32 nPrim = remapIdx((sint32)b.PrimVert, remap.Vert);
		const sint32 nPatch = remapIdx((sint32)b.Patch, remap.Patch);
		b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		if (nPrim < 0 || nPatch < 0)
		{
			b.Binded = 0;
			continue;
		}
		b.PrimVert = (uint32)nPrim;
		b.Patch = (uint32)nPatch;
	}

	// --- PatchMesh side: compact each element table and rewrite its cross references.
	{
		size_t w = 0;
		for (size_t i = 0; i < nV; ++i)
		{
			if (remap.Vert[i] < 0)
				continue;
			SPmVert &v = pm.Verts[i];
			remapList(v.Vectors, remap.Vec);
			remapList(v.Patches, remap.Patch);
			remapList(v.Edges, remap.Edge);
			if (w != i) pm.Verts[w] = v;
			++w;
		}
		pm.Verts.resize(w);
		w = 0;
		for (size_t i = 0; i < nVec; ++i)
		{
			if (remap.Vec[i] < 0)
				continue;
			SPmVec &v = pm.Vecs[i];
			v.Vert = remapIdx(v.Vert, remap.Vert);
			remapList(v.Patches, remap.Patch);
			if (w != i) pm.Vecs[w] = v;
			++w;
		}
		pm.Vecs.resize(w);
		w = 0;
		for (size_t i = 0; i < nE; ++i)
		{
			if (remap.Edge[i] < 0)
				continue;
			SPmEdge &e = pm.Edges[i];
			e.V1 = remapIdx(e.V1, remap.Vert);
			e.Vec12 = remapIdx(e.Vec12, remap.Vec);
			e.Vec21 = remapIdx(e.Vec21, remap.Vec);
			e.V2 = remapIdx(e.V2, remap.Vert);
			remapList(e.Patches, remap.Patch);
			if (w != i) pm.Edges[w] = e;
			++w;
		}
		pm.Edges.resize(w);
		w = 0;
		for (size_t i = 0; i < nP; ++i)
		{
			if (remap.Patch[i] < 0)
				continue;
			SPmPatch &p = pm.Patches[i];
			for (int k = 0; k < 4; ++k)
			{
				p.V[k] = remapIdx(p.V[k], remap.Vert);
				p.Interior[k] = remapIdx(p.Interior[k], remap.Vec);
				p.Edge[k] = remapIdx(p.Edge[k], remap.Edge);
			}
			for (int k = 0; k < 8; ++k)
				p.Vec[k] = remapIdx(p.Vec[k], remap.Vec);
			if (w != i) pm.Patches[w] = p;
			++w;
		}
		pm.Patches.resize(w);
	}

	// --- Side tables.
	remapBits(pm.VertSel, remap.Vert, (sint32)pm.Verts.size());
	remapBits(pm.PatchSel, remap.Patch, (sint32)pm.Patches.size());
	remapBits(pm.EdgeSel, remap.Edge, (sint32)pm.Edges.size());
	if (pm.HasTvPatches)
	{
		size_t w = 0;
		for (size_t i = 0; i < pm.TvPatches.size() && i < nP; ++i)
			if (remap.Patch[i] >= 0)
			{
				if (w != i) pm.TvPatches[w] = pm.TvPatches[i];
				++w;
			}
		pm.TvPatches.resize(w);
	}

	// --- Mapper: input-indexed record slots stay; dead outputs flip, the rest remap.
	if (mapper)
	{
		for (size_t i = 0; i < mapper->VertMap.size(); ++i)
			if (mapper->VertMap[i].Vert >= 0)
				mapper->VertMap[i].Vert = remapIdx(mapper->VertMap[i].Vert, remap.Vert);
		for (size_t i = 0; i < mapper->VecMap.size(); ++i)
			if (mapper->VecMap[i].Vert >= 0)
				mapper->VecMap[i].Vert = remapIdx(mapper->VecMap[i].Vert, remap.Vec);
	}
	return true;
}

namespace {

/// One CCW quarter turn of a single quad patch (TurnPatch port; see the header note on the
/// edge-flag and TVPatch corrections).
void turnOnePatchCcw(SPatchMesh &pm, SRPatchMesh &rp, uint p)
{
	SPmPatch &pp = pm.Patches[p];
	// Rings rotate: corners/interiors/edges by one, tangent pairs by two.
	{
		const SPmPatch old = pp;
		for (int k = 0; k < 4; ++k)
		{
			pp.V[k] = old.V[(k + 1) & 3];
			pp.Interior[k] = old.Interior[(k + 1) & 3];
			pp.Edge[k] = old.Edge[(k + 1) & 3];
		}
		for (int k = 0; k < 8; ++k)
			pp.Vec[k] = old.Vec[(k + 2) & 7];
	}
	// Map channel follows the same groups (correction; legacy left it misaligned).
	if (pm.HasTvPatches && p < pm.TvPatches.size())
	{
		const SPmTvPatch old = pm.TvPatches[p];
		SPmTvPatch &tv = pm.TvPatches[p];
		for (int k = 0; k < 4; ++k)
		{
			tv.Tv[k] = old.Tv[(k + 1) & 3];
			tv.Tv[12 + k] = old.Tv[12 + ((k + 1) & 3)];
		}
		for (int k = 0; k < 8; ++k)
			tv.Tv[4 + k] = old.Tv[4 + ((k + 2) & 7)];
	}
	// The tile grid transposes with the tessellation orders swapped: the tile at (u, v)
	// lands at (oldV-1-v, u) in the new (oldV x oldU) grid, every layer rotated one
	// quarter CCW (TurnPatch's rotate(3) on the 0..3 CW-encoded field). Colors follow on
	// the (+1) lattice. Edge flags rotate with the edge ring (correction).
	SRpoPatch &up = rp.Patches[p];
	{
		const SRpoPatch old = up;
		const sint32 oldU = 1 << old.NbTilesU;
		const sint32 oldV = 1 << old.NbTilesV;
		up.NbTilesU = old.NbTilesV;
		up.NbTilesV = old.NbTilesU;
		for (sint32 v = 0; v < oldV; ++v)
			for (sint32 u = 0; u < oldU; ++u)
			{
				const sint32 newU = oldV - 1 - v;
				const sint32 newV = u;
				SRpoTile &t = up.Tiles[newU + newV * oldV];
				t = old.Tiles[u + v * oldU];
				for (int l = 0; l < 3; ++l)
					t.Layer[l].Rotate = (t.Layer[l].Rotate + 3) & 3;
			}
		for (sint32 v = 0; v < oldV + 1; ++v)
			for (sint32 u = 0; u < oldU + 1; ++u)
			{
				const sint32 newU = oldV - v;
				const sint32 newV = u;
				up.Colors[newU + newV * (oldV + 1)] = old.Colors[u + v * (oldU + 1)];
			}
		for (int k = 0; k < 4; ++k)
			up.EdgeFlags[k] = old.EdgeFlags[(k + 1) & 3];
	}
}

} /* anonymous namespace */

bool topoTurnPatches(SPatchMesh &pm, SRPatchMesh &rp,
                     const std::set<uint> &patches, bool ccw, std::string &err)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (patches.empty())
	{ err = "nothing to turn"; return false; }
	for (std::set<uint>::const_iterator it = patches.begin(); it != patches.end(); ++it)
	{
		if (*it >= pm.Patches.size()) { err = "patch index out of range"; return false; }
		if (pm.Patches[*it].Type != 4) { err = "only quad patches can be turned"; return false; }
		const SRpoPatch &up = rp.Patches[*it];
		const size_t nt = ((size_t)1 << up.NbTilesU) * ((size_t)1 << up.NbTilesV);
		const size_t nc = (((size_t)1 << up.NbTilesU) + 1) * (((size_t)1 << up.NbTilesV) + 1);
		if (up.Tiles.size() != nt || up.Colors.size() != nc)
		{ err = "tile/color table size mismatch"; return false; }
	}
	// CW is three CCW turns, the legacy rule - one code path, provably its own inverse.
	const int turns = ccw ? 1 : 3;
	for (int t = 0; t < turns; ++t)
	{
		for (std::set<uint>::const_iterator it = patches.begin(); it != patches.end(); ++it)
			turnOnePatchCcw(pm, rp, *it);
		// Bind records aimed at a turned patch follow the edge ring (TurnPatch port).
		for (size_t v = 0; v < rp.Verts.size(); ++v)
		{
			SRpoVertexBind &b = rp.Verts[v];
			if (b.Binded && patches.count((uint)b.Patch))
				b.Edge = (b.Edge + 3) & 3;
		}
	}
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
