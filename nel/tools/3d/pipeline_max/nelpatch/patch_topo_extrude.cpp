/**
 * \file patch_topo_extrude.cpp
 * \brief Extrude the selected patches: detach-split the boundary, bridge walls, translate.
 * \date 2026-07-30
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The legacy Extrude recomposed from the tool's own proven pieces. The selection's
 * boundary splits exactly as topoDetachElements does it (the duplicated ring carries the
 * selection, the originals stay with the complement), WALLS bridge the two rings - one
 * quad per shared boundary edge, with the vertical side edges SHARED between adjacent
 * walls - and the island then translates by the extrude vector. Open boundary edges
 * (zone borders) rise without a wall: a ligo border profile is a cross-file contract,
 * and walling it would break the brick's edge.
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

#include "patch_topo.h"

#include <cmath>
#include <cstring>
#include <map>

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

void xListAdd(std::vector<sint32> &list, sint32 v)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] == v)
			return;
	list.push_back(v);
}

/// Effective (evaluated) position of a vert / vec (see the subdivide TU's helpers).
const float *xEffVert(const SPatchMesh &pm, const SPatchMesh *evalPm, sint32 i)
{
	if (evalPm && i >= 0 && (size_t)i < evalPm->Verts.size())
		return evalPm->Verts[i].Pos;
	return pm.Verts[i].Pos;
}

/// Mapper record of an OUTPUT vert / vec, or NULL.
SPmMapVert *xVertRec(SPmVertMapper *mapper, sint32 idx)
{
	if (!mapper)
		return NULL;
	for (size_t i = 0; i < mapper->VertMap.size(); ++i)
		if (mapper->VertMap[i].Vert == idx)
			return &mapper->VertMap[i];
	return NULL;
}

SPmMapVert *xVecRec(SPmVertMapper *mapper, sint32 idx)
{
	if (!mapper)
		return NULL;
	for (size_t i = 0; i < mapper->VecMap.size(); ++i)
		if (mapper->VecMap[i].Vert == idx)
			return &mapper->VecMap[i];
	return NULL;
}

/// Translate one element by (dx, dy, dz), the Tier A rule: a MAPPED output moves through
/// its Delta (the stored position is a dead cache), an unmapped one through its stored
/// position.
void xShift(float *stored, SPmMapVert *rec, float dx, float dy, float dz)
{
	if (rec)
	{
		rec->Delta[0] = (float)((long double)rec->Delta[0] + dx);
		rec->Delta[1] = (float)((long double)rec->Delta[1] + dy);
		rec->Delta[2] = (float)((long double)rec->Delta[2] + dz);
	}
	else
	{
		stored[0] = (float)((long double)stored[0] + dx);
		stored[1] = (float)((long double)stored[1] + dy);
		stored[2] = (float)((long double)stored[2] + dz);
	}
}

} /* anonymous namespace */

bool topoExtrudePatches(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                        const std::set<uint> &sel, float dx, float dy, float dz,
                        std::string &err, const SPatchMesh *evalPm, float outline)
{
	if (pm.HasTvPatches)
	{ err = "map-channel mesh: wall TVPatch assignment is not implemented"; return false; }
	if (pm.Verts.empty() || pm.Vecs.empty() || pm.Edges.empty() || pm.Patches.empty())
	{ err = "empty element table"; return false; }
	const float len2 = dx * dx + dy * dy + dz * dz;
	if (len2 < 1e-8f)
	{ err = "nothing to extrude (zero height)"; return false; }

	// A vertex where MORE than two shared boundary edges meet would over-share its
	// vertical edge (three walls on one edge record) - a self-touching selection outline.
	// Scanned before the split mutates anything.
	{
		std::map<sint32, int> boundaryEnds;
		for (size_t e = 0; e < pm.Edges.size(); ++e)
		{
			const SPmEdge &ed = pm.Edges[e];
			bool anySel = false, anyComp = false;
			for (size_t k = 0; k < ed.Patches.size(); ++k)
			{
				if (ed.Patches[k] < 0)
					continue;
				if (sel.count((uint)ed.Patches[k])) anySel = true;
				else anyComp = true;
			}
			if (!anySel || !anyComp)
				continue;
			++boundaryEnds[ed.V1];
			++boundaryEnds[ed.V2];
		}
		for (std::map<sint32, int>::const_iterator it = boundaryEnds.begin();
		     it != boundaryEnds.end(); ++it)
			if (it->second > 2)
			{ err = "the selection's boundary touches itself (walls would over-share an edge)"; return false; }
	}

	// --- The boundary split, exactly the detach transform (its refusals apply too).
	STopoDetachBoundary boundary;
	if (!topoDetachElements(pm, rp, sel, err, evalPm, &boundary))
		return false;
	std::map<sint32, sint32> dup(boundary.Verts.begin(), boundary.Verts.end());

	// --- Outline (the legacy Bevel's second stage): per boundary vertex, the outward
	// direction in the XY plane - the average of its adjacent boundary edges' XY normals,
	// oriented AWAY from the island (checked against the island patch's centroid).
	// Positive outline pushes the top ring outward, negative pulls it in (the classic
	// tapered cliff). Computed on the pre-translate geometry (the copies are coincident),
	// keyed by the ORIGINAL vertex - the vertical-edge thirds and the top-ring shift both
	// read it. Wall-less open border edges contribute nothing (they are not in the
	// boundary list), so a border-only ring cannot be outlined.
	struct SXY
	{
		float X, Y;
		SXY() : X(0.f), Y(0.f) { }
	};
	std::map<sint32, SXY> outDir;
	if (outline != 0.f)
	{
		for (size_t b = 0; b < boundary.Edges.size(); ++b)
		{
			const SPmEdge &eo = pm.Edges[boundary.Edges[b].first];
			const SPmEdge &ec = pm.Edges[boundary.Edges[b].second];
			if (ec.Patches.size() != 1)
				continue;
			const sint32 ip = ec.Patches[0];
			if (ip < 0 || (size_t)ip >= pm.Patches.size())
				continue;
			const float *P = xEffVert(pm, evalPm, eo.V1);
			const float *Q = xEffVert(pm, evalPm, eo.V2);
			float nx = -(Q[1] - P[1]);
			float ny = Q[0] - P[0];
			const float nl = std::sqrt(nx * nx + ny * ny);
			if (nl < 1e-6f)
				continue; // an edge vertical in XY has no in-plane normal
			nx /= nl;
			ny /= nl;
			// Orient away from the island: against the island patch's XY centroid.
			float cx = 0.f, cy = 0.f;
			for (int k = 0; k < 4; ++k)
			{
				const float *v = xEffVert(pm, evalPm, pm.Patches[ip].V[k]);
				cx += v[0] / 4.f;
				cy += v[1] / 4.f;
			}
			const float mx = (P[0] + Q[0]) / 2.f, my = (P[1] + Q[1]) / 2.f;
			if (nx * (cx - mx) + ny * (cy - my) > 0.f)
			{
				nx = -nx;
				ny = -ny;
			}
			outDir[eo.V1].X += nx; outDir[eo.V1].Y += ny;
			outDir[eo.V2].X += nx; outDir[eo.V2].Y += ny;
		}
		for (std::map<sint32, SXY>::iterator it = outDir.begin(); it != outDir.end(); ++it)
		{
			const float l = std::sqrt(it->second.X * it->second.X
			                          + it->second.Y * it->second.Y);
			if (l < 1e-6f)
			{
				it->second.X = it->second.Y = 0.f;
				continue;
			}
			it->second.X = it->second.X / l * outline;
			it->second.Y = it->second.Y / l * outline;
		}
	}

	// Presence/flags templates (uniform per file in practice; the add-quad rule).
	const SPmVec vecTpl = pm.Vecs[0];
	const SPmEdge edgeTpl = pm.Edges[0];

	// The wall's VERTICAL tile order from the height, at the standard 2 m/tile density.
	const float h = (float)std::sqrt((double)len2);
	sint32 vOrder = 1;
	while (vOrder < 4 && (float)(1 << vOrder) * 2.f < h)
		++vOrder;

	// --- One wall per shared boundary edge; vertical edges shared between adjacent
	// walls through this map (also handles open chains at zone borders - the chain ends
	// simply own unshared vertical edges).
	std::map<sint32, sint32> verticalEdge; // original vert -> vertical edge index
	for (size_t b = 0; b < boundary.Edges.size(); ++b)
	{
		const sint32 bo = boundary.Edges[b].first;   // bottom: original, complement side
		const sint32 bc = boundary.Edges[b].second;  // top: the island's copy
		const SPmEdge eo = pm.Edges[bo]; // BY VALUE: allocations below reallocate
		// The complement patch across the bottom edge, and its ring traversal A -> B; the
		// wall's ring opposes it, as any two neighbors' rings do (the add-quad rule).
		if (eo.Patches.size() != 1)
		{ err = "boundary edge without exactly one complement patch"; return false; }
		const sint32 cp = eo.Patches[0];
		if (cp < 0 || (size_t)cp >= pm.Patches.size())
		{ err = "complement patch out of range"; return false; }
		sint32 slot = -1;
		for (int k = 0; k < 4; ++k)
			if (pm.Patches[cp].Edge[k] == bo) { slot = k; break; }
		if (slot < 0)
		{ err = "complement slot for boundary edge not found"; return false; }
		const sint32 A = pm.Patches[cp].V[slot];
		const sint32 B = pm.Patches[cp].V[(slot + 1) & 3];
		if (!dup.count(A) || !dup.count(B))
		{ err = "boundary corner without a duplicate"; return false; }
		const sint32 dA = dup[A], dB = dup[B];

		// Vertical edges A -> dA and B -> dB, created once and shared with the adjacent
		// wall. Tangents at the thirds of the extrude vector, owned bottom and top.
		sint32 vertEdges[2];
		const sint32 bases[2] = { A, B };
		const sint32 tops[2] = { dA, dB };
		for (int s = 0; s < 2; ++s)
		{
			std::map<sint32, sint32>::const_iterator it = verticalEdge.find(bases[s]);
			if (it != verticalEdge.end())
			{
				vertEdges[s] = it->second;
				continue;
			}
			const float *base = xEffVert(pm, evalPm, bases[s]);
			// The full base -> top vector for THIS vertex: extrude plus its outline
			// offset, so an outlined wall's side curve leans smoothly bottom to top.
			float total[3] = { dx, dy, dz };
			{
				std::map<sint32, SXY>::const_iterator ot = outDir.find(bases[s]);
				if (ot != outDir.end())
				{
					total[0] += ot->second.X;
					total[1] += ot->second.Y;
				}
			}
			float t1[3], t2[3];
			for (int c = 0; c < 3; ++c)
			{
				t1[c] = (float)((long double)base[c] + total[c] / 3.f);
				t2[c] = (float)((long double)base[c] + total[c] * 2.f / 3.f);
			}
			SPmVec v1 = vecTpl;
			v1.Patches.clear();
			v1.Vert = v1.HasVert ? bases[s] : -1;
			memcpy(v1.Pos, t1, 12);
			pm.Vecs.push_back(v1);
			const sint32 vec12 = (sint32)pm.Vecs.size() - 1;
			SPmVec v2 = vecTpl;
			v2.Patches.clear();
			v2.Vert = v2.HasVert ? tops[s] : -1;
			memcpy(v2.Pos, t2, 12);
			pm.Vecs.push_back(v2);
			const sint32 vec21 = (sint32)pm.Vecs.size() - 1;
			SPmEdge ne = edgeTpl;
			ne.Patches.clear();
			ne.V1 = bases[s];
			ne.Vec12 = vec12;
			ne.Vec21 = vec21;
			ne.V2 = tops[s];
			pm.Edges.push_back(ne);
			vertEdges[s] = (sint32)pm.Edges.size() - 1;
			verticalEdge[bases[s]] = vertEdges[s];
			if (pm.Verts[bases[s]].HasEdges) xListAdd(pm.Verts[bases[s]].Edges, vertEdges[s]);
			if (pm.Verts[tops[s]].HasEdges) xListAdd(pm.Verts[tops[s]].Edges, vertEdges[s]);
			if (pm.Verts[bases[s]].HasVectors) xListAdd(pm.Verts[bases[s]].Vectors, vec12);
			if (pm.Verts[tops[s]].HasVectors) xListAdd(pm.Verts[tops[s]].Vectors, vec21);
		}

		// The island patch across the top copy, for flag inheritance and the horizontal
		// tile order (the wall's tiles must line up with the top patch's along the seam).
		const SPmEdge ec = pm.Edges[bc];
		if (ec.Patches.size() != 1)
		{ err = "island copy edge without exactly one patch"; return false; }
		const sint32 ip = ec.Patches[0];
		if (ip < 0 || (size_t)ip >= pm.Patches.size() || (size_t)ip >= rp.Patches.size())
		{ err = "island patch out of range"; return false; }
		sint32 islot = -1;
		for (int k = 0; k < 4; ++k)
			if (pm.Patches[ip].Edge[k] == bc) { islot = k; break; }
		if (islot < 0)
		{ err = "island slot for copy edge not found"; return false; }
		// Ring edge slots 0 and 2 run along the tile v axis, 1 and 3 along u.
		const sint32 hOrder = (islot == 0 || islot == 2) ? rp.Patches[ip].NbTilesV
		                                                 : rp.Patches[ip].NbTilesU;

		// The wall: ring (B, A, dA, dB) - bottom opposes the complement, top opposes the
		// island. Bottom rides the original record (it gains its second patch), top rides
		// the island's copy (likewise).
		SPmPatch np = pm.Patches[ip]; // Type/NumVerts/SmGroup/Flags/presence inherit
		np.V[0] = B; np.V[1] = A; np.V[2] = dA; np.V[3] = dB;
		np.Edge[0] = bo; np.Edge[1] = vertEdges[0]; np.Edge[2] = bc; np.Edge[3] = vertEdges[1];
		// Tangent slots per ring direction against each record's stored direction.
		{
			const SPmEdge &e0 = pm.Edges[bo];
			if (e0.V1 == B) { np.Vec[0] = e0.Vec12; np.Vec[1] = e0.Vec21; }
			else            { np.Vec[0] = e0.Vec21; np.Vec[1] = e0.Vec12; }
			const SPmEdge &e1 = pm.Edges[vertEdges[0]]; // A -> dA stored
			if (e1.V1 == A) { np.Vec[2] = e1.Vec12; np.Vec[3] = e1.Vec21; }
			else            { np.Vec[2] = e1.Vec21; np.Vec[3] = e1.Vec12; }
			const SPmEdge &e2 = pm.Edges[bc];
			if (e2.V1 == dA) { np.Vec[4] = e2.Vec12; np.Vec[5] = e2.Vec21; }
			else             { np.Vec[4] = e2.Vec21; np.Vec[5] = e2.Vec12; }
			const SPmEdge &e3 = pm.Edges[vertEdges[1]]; // B -> dB stored; ring runs dB -> B
			if (e3.V1 == dB) { np.Vec[6] = e3.Vec12; np.Vec[7] = e3.Vec21; }
			else             { np.Vec[6] = e3.Vec21; np.Vec[7] = e3.Vec12; }
		}
		// Interiors: fresh parallelogram seeds with the authored interior identity
		// (PVEC_INTERIOR, no vertex attachment) - recomputed at eval for AUTO owners.
		for (int k = 0; k < 4; ++k)
		{
			const float *v = pm.Verts[np.V[k]].Pos;
			const float *a = pm.Vecs[np.Vec[k * 2]].Pos;
			const float *c = pm.Vecs[np.Vec[(k * 2 + 7) & 7]].Pos;
			float ip3[3];
			for (int q = 0; q < 3; ++q)
				ip3[q] = (float)((long double)v[q] + ((long double)a[q] - v[q])
				                 + ((long double)c[q] - v[q]));
			SPmVec iv = vecTpl;
			iv.Patches.clear();
			iv.Flags |= 1;
			iv.Vert = -1;
			memcpy(iv.Pos, ip3, 12);
			pm.Vecs.push_back(iv);
			np.Interior[k] = (sint32)pm.Vecs.size() - 1;
		}
		pm.Patches.push_back(np);
		const sint32 wall = (sint32)pm.Patches.size() - 1;

		// Cross references: the two horizontal records gain the wall; the vertical
		// records, corner patch lists and tangent patch lists follow.
		if (pm.Edges[bo].HasPatches) xListAdd(pm.Edges[bo].Patches, wall);
		if (pm.Edges[bc].HasPatches) xListAdd(pm.Edges[bc].Patches, wall);
		if (pm.Edges[vertEdges[0]].HasPatches) xListAdd(pm.Edges[vertEdges[0]].Patches, wall);
		if (pm.Edges[vertEdges[1]].HasPatches) xListAdd(pm.Edges[vertEdges[1]].Patches, wall);
		for (int k = 0; k < 4; ++k)
			if (pm.Verts[np.V[k]].HasPatches)
				xListAdd(pm.Verts[np.V[k]].Patches, wall);
		for (int k = 0; k < 8; ++k)
			if (np.Vec[k] >= 0 && pm.Vecs[np.Vec[k]].HasPatches)
				xListAdd(pm.Vecs[np.Vec[k]].Patches, wall);

		// rp record: fresh paintable surface; horizontal (v) order lines up with the top
		// patch, vertical (u) order from the height.
		SRpoPatch up;
		up.NbTilesU = vOrder;
		up.NbTilesV = hOrder;
		const size_t nt = ((size_t)1 << up.NbTilesU) * ((size_t)1 << up.NbTilesV);
		const size_t nc = (((size_t)1 << up.NbTilesU) + 1) * (((size_t)1 << up.NbTilesV) + 1);
		up.Tiles.assign(nt, SRpoTile());
		up.Colors.assign(nc, 0x00ffffffu);
		up.EdgeFlags[0] = up.EdgeFlags[1] = up.EdgeFlags[2] = up.EdgeFlags[3] = 0;
		rp.Patches.push_back(up);
	}

	// --- Translate the island: every vert and vec the selection's patches use, once.
	// Bound vertices are skipped (they are derived; the refresh re-lands them). Mapped
	// elements move through their Delta, the Tier A rule.
	{
		std::set<sint32> verts, vecs;
		for (std::set<uint>::const_iterator it = sel.begin(); it != sel.end(); ++it)
		{
			const SPmPatch &pp = pm.Patches[*it];
			for (int k = 0; k < 4; ++k)
			{
				if (pp.V[k] >= 0)
					verts.insert(pp.V[k]);
				if (pp.Interior[k] >= 0)
					vecs.insert(pp.Interior[k]);
			}
			for (int k = 0; k < 8; ++k)
				if (pp.Vec[k] >= 0)
					vecs.insert(pp.Vec[k]);
		}
		for (std::set<sint32>::const_iterator it = verts.begin(); it != verts.end(); ++it)
		{
			if ((size_t)*it < rp.Verts.size() && rp.Verts[*it].Binded)
				continue;
			xShift(pm.Verts[*it].Pos, xVertRec(mapper, *it), dx, dy, dz);
		}
		for (std::set<sint32>::const_iterator it = vecs.begin(); it != vecs.end(); ++it)
			xShift(pm.Vecs[*it].Pos, xVecRec(mapper, *it), dx, dy, dz);

		// --- Outline the top ring: each island boundary vertex (the detach COPY) moves
		// by its outward XY offset, its riding island tangents with it (the ride-in-file
		// rule - slots 2c and 2*((c+3)&3)+1 of the corner). Interior island verts stay;
		// the wall tops follow for free (they ARE these vertices), and the vertical
		// tangents already sit at the thirds of the outlined vector.
		if (outline != 0.f)
		{
			std::set<sint32> outVecs;
			std::map<sint32, SXY> copyOut; // island copy vert -> its offset
			for (std::map<sint32, SXY>::const_iterator it = outDir.begin();
			     it != outDir.end(); ++it)
			{
				std::map<sint32, sint32>::const_iterator dit = dup.find(it->first);
				if (dit == dup.end())
					continue;
				copyOut[dit->second] = it->second;
			}
			for (std::map<sint32, SXY>::const_iterator it = copyOut.begin();
			     it != copyOut.end(); ++it)
			{
				if ((size_t)it->first < rp.Verts.size() && rp.Verts[it->first].Binded)
					continue;
				xShift(pm.Verts[it->first].Pos, xVertRec(mapper, it->first),
				       it->second.X, it->second.Y, 0.f);
			}
			for (std::set<uint>::const_iterator it = sel.begin(); it != sel.end(); ++it)
			{
				const SPmPatch &pp = pm.Patches[*it];
				for (int c = 0; c < 4; ++c)
				{
					std::map<sint32, SXY>::const_iterator co = copyOut.find(pp.V[c]);
					if (co == copyOut.end())
						continue;
					const sint32 slots[2] = { pp.Vec[2 * c], pp.Vec[2 * ((c + 3) & 3) + 1] };
					for (int s = 0; s < 2; ++s)
					{
						if (slots[s] < 0 || !outVecs.insert(slots[s]).second)
							continue;
						xShift(pm.Vecs[slots[s]].Pos, xVecRec(mapper, slots[s]),
						       co->second.X, co->second.Y, 0.f);
					}
				}
			}
		}
	}

	// --- Side tables sized to the new counts (walls unselected).
	if (pm.PatchSel.Present)
	{
		pm.PatchSel.Count = (sint32)pm.Patches.size();
		pm.PatchSel.Bits.resize(((size_t)pm.PatchSel.Count + 31) / 32, 0);
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
