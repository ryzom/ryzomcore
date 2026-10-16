/**
 * \file patch_topo_attach.cpp
 * \brief topoAppendMesh: append one decoded patch mesh onto another (the attach merge)
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

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

namespace {

/// p' = M * p, row-major 3x4, double intermediates, rounded at the store.
void xformPos(const double m[12], float *p)
{
	const double x = p[0], y = p[1], z = p[2];
	p[0] = (float)(m[0] * x + m[1] * y + m[2] * z + m[3]);
	p[1] = (float)(m[4] * x + m[5] * y + m[6] * z + m[7]);
	p[2] = (float)(m[8] * x + m[9] * y + m[10] * z + m[11]);
}

void aOffsetList(std::vector<sint32> &list, sint32 off)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (list[i] >= 0)
			list[i] += off;
}

void aGrowBits(SPmBitArray &sel, sint32 newCount)
{
	if (!sel.Present)
		return;
	sel.Count = newCount;
	sel.Bits.resize(newCount > 0 ? ((size_t)newCount + 31) / 32 : 0, 0);
}

} /* anonymous namespace */

bool topoAppendMesh(SPatchMesh &pm, SRPatchMesh &rp,
                    const SPatchMesh &src, const SRPatchMesh &srcRp,
                    const double relTM[12], std::string &err)
{
	if (pm.EdgesReconstructed || src.EdgesReconstructed)
	{ err = "reconstructed (Max 3) edge table: topology cannot be written back"; return false; }
	if (rp.Patches.size() != pm.Patches.size() || rp.Verts.size() != pm.Verts.size())
	{ err = "target PatchMesh/RPatchMesh size mismatch"; return false; }
	if (srcRp.Patches.size() != src.Patches.size() || srcRp.Verts.size() != src.Verts.size())
	{ err = "source PatchMesh/RPatchMesh size mismatch"; return false; }
	if (!pm.Hooks.empty() || !src.Hooks.empty())
	{ err = "mesh carries hook records; hook remap is not implemented"; return false; }
	if (pm.HasTvPatches || src.HasTvPatches || pm.HasTvVerts || src.HasTvVerts)
	{ err = "map-channel mesh: merged TVPatch assignment is not implemented"; return false; }
	if (src.Patches.empty())
	{ err = "source has no patches"; return false; }

	const sint32 V0 = (sint32)pm.Verts.size();
	const sint32 VEC0 = (sint32)pm.Vecs.size();
	const sint32 E0 = (sint32)pm.Edges.size();
	const sint32 P0 = (sint32)pm.Patches.size();

	// Element tables: copy, shift every cross reference, transform every position. The
	// copies keep their own presence flags - the encoder writes per-element children, so
	// mixed presence across the merged file is representable.
	for (size_t i = 0; i < src.Verts.size(); ++i)
	{
		SPmVert v = src.Verts[i];
		xformPos(relTM, v.Pos);
		aOffsetList(v.Vectors, VEC0);
		aOffsetList(v.Patches, P0);
		aOffsetList(v.Edges, E0);
		pm.Verts.push_back(v);
	}
	for (size_t i = 0; i < src.Vecs.size(); ++i)
	{
		SPmVec v = src.Vecs[i];
		xformPos(relTM, v.Pos);
		if (v.Vert >= 0)
			v.Vert += V0;
		aOffsetList(v.Patches, P0);
		pm.Vecs.push_back(v);
	}
	for (size_t i = 0; i < src.Edges.size(); ++i)
	{
		SPmEdge e = src.Edges[i];
		if (e.V1 >= 0) e.V1 += V0;
		if (e.V2 >= 0) e.V2 += V0;
		if (e.Vec12 >= 0) e.Vec12 += VEC0;
		if (e.Vec21 >= 0) e.Vec21 += VEC0;
		aOffsetList(e.Patches, P0);
		pm.Edges.push_back(e);
	}
	for (size_t i = 0; i < src.Patches.size(); ++i)
	{
		SPmPatch p = src.Patches[i];
		for (int k = 0; k < 4; ++k)
		{
			if (p.V[k] >= 0) p.V[k] += V0;
			if (p.Interior[k] >= 0) p.Interior[k] += VEC0;
			if (p.Edge[k] >= 0) p.Edge[k] += E0;
		}
		for (int k = 0; k < 8; ++k)
			if (p.Vec[k] >= 0)
				p.Vec[k] += VEC0;
		pm.Patches.push_back(p);
	}

	// Selection BitArrays: the target's grow (appended elements unselected); the source's
	// selection state does not travel.
	aGrowBits(pm.VertSel, (sint32)pm.Verts.size());
	aGrowBits(pm.PatchSel, (sint32)pm.Patches.size());
	aGrowBits(pm.EdgeSel, (sint32)pm.Edges.size());

	// rp side: the per-patch paint records travel VERBATIM (tiles are authored in the
	// patch frame, which the reorientation does not touch); bind records retarget through
	// the offsets with their caches reset (they hold vector indices of the source file).
	for (size_t i = 0; i < srcRp.Patches.size(); ++i)
		rp.Patches.push_back(srcRp.Patches[i]);
	for (size_t i = 0; i < srcRp.Verts.size(); ++i)
	{
		SRpoVertexBind b = srcRp.Verts[i];
		b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		if (b.Binded)
		{
			b.Patch += (uint32)P0;
			b.PrimVert += (uint32)V0;
		}
		rp.Verts.push_back(b);
	}
	return true;
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
