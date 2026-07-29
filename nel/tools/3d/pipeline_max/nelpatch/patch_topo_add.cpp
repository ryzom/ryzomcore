/**
 * \file patch_topo_add.cpp
 * \brief topoAddQuads: grow a quad from an open edge (the legacy Add Quad)
 * \date 2026-07-29
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

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

void aListAdd(std::vector<sint32> &list, sint32 v)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] == v)
			return;
	list.push_back(v);
}

/// 2*a - b per component, extended precision, rounded at the store.
void reflectPt(const float *a, const float *b, float *out)
{
	out[0] = (float)(2.0L * (long double)a[0] - (long double)b[0]);
	out[1] = (float)(2.0L * (long double)a[1] - (long double)b[1]);
	out[2] = (float)(2.0L * (long double)a[2] - (long double)b[2]);
}

void aGrowBits(SPmBitArray &sel, sint32 newCount)
{
	if (!sel.Present)
		return;
	sel.Count = newCount;
	sel.Bits.resize(newCount > 0 ? ((size_t)newCount + 31) / 32 : 0, 0);
}

} /* anonymous namespace */

bool topoAddQuads(SPatchMesh &pm, SRPatchMesh &rp,
                  const std::set<uint> &edges, std::string &err)
{
	if (pm.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (pm.HasTvPatches)
	{ err = "map-channel mesh: new-patch TVPatch assignment is not implemented"; return false; }
	if (edges.empty())
	{ err = "no open edges selected"; return false; }
	if (pm.Verts.empty() || pm.Vecs.empty() || pm.Edges.empty() || pm.Patches.empty())
	{ err = "empty element table"; return false; }

	for (std::set<uint>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if (*it >= pm.Edges.size()) { err = "edge index out of range"; return false; }
		const SPmEdge &ed = pm.Edges[*it];
		if (ed.Patches.size() != 1)
		{ err = "an edge of the selection is not open"; return false; }
		if (ed.V1 < 0 || ed.V2 < 0 || ed.Vec12 < 0 || ed.Vec21 < 0)
		{ err = "edge record incomplete"; return false; }
	}
	for (size_t v = 0; v < rp.Verts.size(); ++v)
		if (rp.Verts[v].Binded)
		{
			const uint32 tp = rp.Verts[v].Patch;
			const uint32 te = rp.Verts[v].Edge;
			if (tp < pm.Patches.size() && te < 4
			    && edges.count((uint)pm.Patches[tp].Edge[te]))
			{ err = "an edge of the selection is a bind target"; return false; }
		}

	const SPmVert vertTpl = pm.Verts[0];
	const SPmVec vecTpl = pm.Vecs[0];
	const SPmEdge edgeTpl = pm.Edges[0];

	for (std::set<uint>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		// All fields BY VALUE before any allocation.
		const sint32 ei = (sint32)*it;
		const SPmEdge ed = pm.Edges[ei];
		const sint32 ownerIdx = ed.Patches[0];
		if (ownerIdx < 0 || (size_t)ownerIdx >= pm.Patches.size())
		{ err = "owner patch out of range"; return false; }
		const SPmPatch owner = pm.Patches[ownerIdx];
		const SRpoPatch ownerUp = rp.Patches[ownerIdx];
		// The owner's slot for this edge, and how its ring traverses it.
		sint32 slot = -1;
		for (int s = 0; s < 4; ++s)
			if (owner.Edge[s] == ei) { slot = s; break; }
		if (slot < 0) { err = "owner slot for open edge not found"; return false; }
		// Ring traversal of the owner across this edge: A = V[slot] -> B = V[(slot+1)&3].
		const sint32 A = owner.V[slot];
		const sint32 B = owner.V[(slot + 1) & 3];
		// The owner's corners OPPOSITE the edge (for the mirror): across from A is
		// V[(slot+3)&3] (the other end of the edge arriving at A), across from B is
		// V[(slot+2)&3].
		const sint32 Aopp = owner.V[(slot + 3) & 3];
		const sint32 Bopp = owner.V[(slot + 2) & 3];
		// The owner's two side edges (slot+3: Aopp -> A, slot+1: B -> Bopp) point-reflect
		// through their shared corner to seed the new patch's sides, so all four of their
		// tangents participate: the pair near the shared corners and the pair near the
		// opposite corners.
		const sint32 AoppTan = owner.Vec[((slot + 3) & 3) * 2 + 1];   // arrives at A
		const sint32 AoppLeave = owner.Vec[((slot + 3) & 3) * 2];     // leaves Aopp
		const sint32 BoppTan = owner.Vec[((slot + 1) & 3) * 2];       // leaves B
		const sint32 BoppArrive = owner.Vec[((slot + 1) & 3) * 2 + 1]; // arrives at Bopp

		// New far corners: point reflection through the shared corners.
		float posA2[3], posB2[3];
		reflectPt(pm.Verts[A].Pos, pm.Verts[Aopp].Pos, posA2);
		reflectPt(pm.Verts[B].Pos, pm.Verts[Bopp].Pos, posB2);

		// New verts.
		SPmVert nv = vertTpl;
		nv.Vectors.clear(); nv.Patches.clear(); nv.Edges.clear();
		memcpy(nv.Pos, posA2, 12);
		pm.Verts.push_back(nv);
		const sint32 nA = (sint32)pm.Verts.size() - 1;
		memcpy(nv.Pos, posB2, 12);
		pm.Verts.push_back(nv);
		const sint32 nB = (sint32)pm.Verts.size() - 1;
		{
			SRpoVertexBind b;
			memset(&b, 0, sizeof(b));
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
			rp.Verts.push_back(b);
			rp.Verts.push_back(b);
		}

		// New patch ring: (B, A, nA, nB) - opposes the owner across the shared edge.
		// Side edge A->nA: tangents mirror the owner's tangents around A; far edge
		// nA->nB mirrors the shared edge's own curve; side nB->B mirrors around B.
		struct SMk
		{
			SPatchMesh *Pm;
			const SPmVec *Tpl;
			sint32 vec(const float *pos, sint32 ownerVert)
			{
				SPmVec v = *Tpl;
				v.Patches.clear();
				v.Vert = v.HasVert ? ownerVert : -1;
				memcpy(v.Pos, pos, 12);
				Pm->Vecs.push_back(v);
				return (sint32)Pm->Vecs.size() - 1;
			}
		} mkv;
		mkv.Pm = &pm;
		mkv.Tpl = &vecTpl;

		float t1[3], t2[3];
		// Side A -> nA: the point reflection through A of the owner's edge Aopp -> A
		// (whose image runs A -> nA, since nA = 2A - Aopp), control for control.
		reflectPt(pm.Verts[A].Pos, pm.Vecs[AoppTan].Pos, t1);           // leaves A
		reflectPt(pm.Verts[A].Pos, pm.Vecs[AoppLeave].Pos, t2);         // arrives nA
		const sint32 sA12 = mkv.vec(t1, A);
		const sint32 sA21 = mkv.vec(t2, nA);
		// Side nB -> B: the point reflection through B of the owner's edge B -> Bopp.
		reflectPt(pm.Verts[B].Pos, pm.Vecs[BoppTan].Pos, t1);           // arrives B (mirror of leave)
		reflectPt(pm.Verts[B].Pos, pm.Vecs[BoppArrive].Pos, t2);        // leaves nB
		const sint32 sB12 = mkv.vec(t2, nB);
		const sint32 sB21 = mkv.vec(t1, B);
		// Far edge nA -> nB: translate the shared edge's tangents by the same offset
		// their corners took, so the far edge seeds with the same shape as the shared
		// one. Shared record direction vs ring: tangent near A is Vec12 when ed.V1 == A.
		const sint32 tanNearA = (ed.V1 == A) ? ed.Vec12 : ed.Vec21;
		const sint32 tanNearB = (ed.V1 == A) ? ed.Vec21 : ed.Vec12;
		float fA[3], fB[3];
		{
			const float *pa = pm.Verts[A].Pos, *na = pm.Verts[nA].Pos;
			const float *pb = pm.Verts[B].Pos, *nb2 = pm.Verts[nB].Pos;
			const float *ta = pm.Vecs[tanNearA].Pos, *tb = pm.Vecs[tanNearB].Pos;
			fA[0] = (float)((long double)ta[0] + na[0] - pa[0]);
			fA[1] = (float)((long double)ta[1] + na[1] - pa[1]);
			fA[2] = (float)((long double)ta[2] + na[2] - pa[2]);
			fB[0] = (float)((long double)tb[0] + nb2[0] - pb[0]);
			fB[1] = (float)((long double)tb[1] + nb2[1] - pb[1]);
			fB[2] = (float)((long double)tb[2] + nb2[2] - pb[2]);
		}
		const sint32 f12 = mkv.vec(fA, nA);
		const sint32 f21 = mkv.vec(fB, nB);

		SPmEdge ne = edgeTpl;
		ne.Patches.clear();
		ne.V1 = A; ne.Vec12 = sA12; ne.Vec21 = sA21; ne.V2 = nA;
		pm.Edges.push_back(ne);
		const sint32 eSideA = (sint32)pm.Edges.size() - 1;
		ne.V1 = nA; ne.Vec12 = f12; ne.Vec21 = f21; ne.V2 = nB;
		pm.Edges.push_back(ne);
		const sint32 eFar = (sint32)pm.Edges.size() - 1;
		ne.V1 = nB; ne.Vec12 = sB12; ne.Vec21 = sB21; ne.V2 = B;
		pm.Edges.push_back(ne);
		const sint32 eSideB = (sint32)pm.Edges.size() - 1;

		// The new patch. Ring (B, A, nA, nB): e0 = shared (B->A), e1 = A->nA,
		// e2 = nA->nB, e3 = nB->B. Interiors: fresh AUTO-style parallelogram seeds
		// (corner + adjacent tangent offsets), recomputed at eval for AUTO owners anyway.
		SPmPatch np = owner; // Type/NumVerts/SmGroup/Flags/presence inherit
		np.V[0] = B; np.V[1] = A; np.V[2] = nA; np.V[3] = nB;
		np.Edge[0] = ei; np.Edge[1] = eSideA; np.Edge[2] = eFar; np.Edge[3] = eSideB;
		// Tangent slots per ring direction against each record's stored direction.
		np.Vec[0] = tanNearB; np.Vec[1] = tanNearA;      // B -> A rides the shared record
		np.Vec[2] = sA12; np.Vec[3] = sA21;              // A -> nA
		np.Vec[4] = f12; np.Vec[5] = f21;                // nA -> nB
		np.Vec[6] = sB12; np.Vec[7] = sB21;              // nB -> B
		for (int k = 0; k < 4; ++k)
		{
			// Parallelogram interior seed: corner + (out tangent - corner) + (in tangent - corner).
			const float *v = pm.Verts[np.V[k]].Pos;
			const float *a = pm.Vecs[np.Vec[k * 2]].Pos;
			const float *c = pm.Vecs[np.Vec[(k * 2 + 7) & 7]].Pos;
			float ip[3];
			ip[0] = (float)((long double)v[0] + ((long double)a[0] - v[0]) + ((long double)c[0] - v[0]));
			ip[1] = (float)((long double)v[1] + ((long double)a[1] - v[1]) + ((long double)c[1] - v[1]));
			ip[2] = (float)((long double)v[2] + ((long double)a[2] - v[2]) + ((long double)c[2] - v[2]));
			np.Interior[k] = mkv.vec(ip, np.V[k]);
		}
		pm.Patches.push_back(np);
		const sint32 npIdx = (sint32)pm.Patches.size() - 1;

		// rp record: owner's tile orders, EMPTY tiles, white colors, clear edge flags.
		{
			SRpoPatch up;
			up.NbTilesU = ownerUp.NbTilesU;
			up.NbTilesV = ownerUp.NbTilesV;
			const size_t nt = ((size_t)1 << up.NbTilesU) * ((size_t)1 << up.NbTilesV);
			const size_t nc = (((size_t)1 << up.NbTilesU) + 1) * (((size_t)1 << up.NbTilesV) + 1);
			up.Tiles.assign(nt, SRpoTile());
			up.Colors.assign(nc, 0x00ffffffu);
			up.EdgeFlags[0] = up.EdgeFlags[1] = up.EdgeFlags[2] = up.EdgeFlags[3] = 0;
			rp.Patches.push_back(up);
		}

		// Cross references.
		{
			SPmEdge &shared = pm.Edges[ei];
			if (shared.HasPatches)
				aListAdd(shared.Patches, npIdx);
			SPmEdge &ea = pm.Edges[eSideA];
			if (ea.HasPatches) aListAdd(ea.Patches, npIdx);
			SPmEdge &ef = pm.Edges[eFar];
			if (ef.HasPatches) aListAdd(ef.Patches, npIdx);
			SPmEdge &eb = pm.Edges[eSideB];
			if (eb.HasPatches) aListAdd(eb.Patches, npIdx);
		}
		{
			SPmVert &va = pm.Verts[A];
			if (va.HasPatches) aListAdd(va.Patches, npIdx);
			if (va.HasEdges) aListAdd(va.Edges, eSideA);
			if (va.HasVectors) aListAdd(va.Vectors, sA12);
			SPmVert &vb = pm.Verts[B];
			if (vb.HasPatches) aListAdd(vb.Patches, npIdx);
			if (vb.HasEdges) aListAdd(vb.Edges, eSideB);
			if (vb.HasVectors) aListAdd(vb.Vectors, sB21);
			SPmVert &vna = pm.Verts[nA];
			if (vna.HasPatches) aListAdd(vna.Patches, npIdx);
			if (vna.HasEdges) { aListAdd(vna.Edges, eSideA); aListAdd(vna.Edges, eFar); }
			if (vna.HasVectors) { aListAdd(vna.Vectors, sA21); aListAdd(vna.Vectors, f12); }
			SPmVert &vnb = pm.Verts[nB];
			if (vnb.HasPatches) aListAdd(vnb.Patches, npIdx);
			if (vnb.HasEdges) { aListAdd(vnb.Edges, eFar); aListAdd(vnb.Edges, eSideB); }
			if (vnb.HasVectors) { aListAdd(vnb.Vectors, f21); aListAdd(vnb.Vectors, sB12); }
		}
		for (int k = 0; k < 8; ++k)
			if (pm.Vecs[pm.Patches[npIdx].Vec[k]].HasPatches)
				aListAdd(pm.Vecs[pm.Patches[npIdx].Vec[k]].Patches, npIdx);
		for (int k = 0; k < 4; ++k)
			if (pm.Vecs[pm.Patches[npIdx].Interior[k]].HasPatches)
				aListAdd(pm.Vecs[pm.Patches[npIdx].Interior[k]].Patches, npIdx);
	}

	aGrowBits(pm.VertSel, (sint32)pm.Verts.size());
	aGrowBits(pm.PatchSel, (sint32)pm.Patches.size());
	aGrowBits(pm.EdgeSel, (sint32)pm.Edges.size());
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
