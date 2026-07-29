/**
 * \file patch_topo.h
 * \brief Topological transforms over the decoded PatchMesh + RPatchMesh pair
 * \date 2026-07-29
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Pure struct-level topology operations for the zone painter's Tier B ops: each transform
 * mutates an SPatchMesh / SRPatchMesh pair (and optionally the per-node vertex mapper) IN
 * MEMORY and reports the old -> new element index maps. Serialization stays with the
 * caller (encodePatchMesh / encodeRPatchMesh / encodeVertMapper), so one transform serves
 * the base stream and any modifier snapshot alike.
 *
 * The old -> new maps are the paint-survival contract: the per-patch paint data in the
 * SRPatchMesh travels with its compacted patch record, the bind records travel with their
 * vertices, and the caller migrates everything else (landscape tiles, selections) through
 * the same maps. A surviving element NEVER changes content here, only index.
 *
 * The SRPatchMesh side follows RPatchMesh::DeleteAndSweep and the unbind-related rules
 * from nel_patch_lib. The PatchMesh side follows the on-disk stream semantics (wiki plan
 * Part 6.1) - element cross-reference tables compact by the "still referenced by a
 * surviving patch" rule.
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

#ifndef PIPELINE_PATCH_TOPO_H
#define PIPELINE_PATCH_TOPO_H
#include <nel/misc/types_nl.h>

#include <set>
#include <string>
#include <vector>

#include "rpo_data.h"

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

/** Old -> new element index maps of one topological transform; -1 = deleted. */
struct STopoRemap
{
	std::vector<sint32> Vert;
	std::vector<sint32> Vec;
	std::vector<sint32> Edge;
	std::vector<sint32> Patch;
};

/**
 * Delete the listed patches.
 *
 * Element sweep rule: a vertex / vector / edge is deleted when it is used by at least one
 * deleted patch and by no surviving patch; elements referenced by no patch at all are left
 * alone. Survivors compact in order, their cross-reference tables (adjacency lists, patch
 * element indices, edge endpoints) rewritten through the maps and entries for deleted
 * elements dropped. Selection BitArrays compact through the same maps; the map-channel
 * TVPatch table compacts parallel to the patch table (texture vertices keep their slots -
 * an unreferenced PatchTVert is valid).
 *
 * SRPatchMesh side (DeleteAndSweep port): bind groups touching a deleted vertex or a
 * deleted patch's corners release whole (the caches of every surviving bind reset to -1,
 * the loader rebuilds them); per-patch records - the painted tiles, colors and edge flags -
 * travel verbatim with their surviving patch; surviving bind records remap PrimVert/Patch
 * and release when either died.
 *
 * The optional mapper keeps its input-indexed record slots; records whose output element
 * died flip Vert to -1, the rest remap.
 *
 * Refuses (false + err): parallel-size mismatch, reconstructed (Max 3) edge tables, and
 * meshes with a non-empty hook table (hook remap is deferred until a real corpus case
 * exists; the observed corpus hook count is zero).
 */
bool topoDeletePatches(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                       const std::set<uint> &delPatches, STopoRemap &remap, std::string &err);

/**
 * Turn the listed quad patches a quarter turn CCW (ccw=false applies the turn three times,
 * the legacy rule). Element counts and identities never change - the patch's internal
 * rings rotate (V/Interior/Edge by one, Vec by two), the tile grid transposes with its
 * tessellation orders swapped and every layer rotated, the color grid follows, and bind
 * records targeting a turned patch decrement their edge slot. Ports RPatchMesh::TurnPatch
 * + the modifier-side ring rotation (both NeL-original).
 *
 * Two deliberate corrections over the legacy op, both keeping data attached to the same
 * GEOMETRIC edge/corner it was authored on: the per-edge no-smooth flags rotate with the
 * edge ring (legacy left them at their old slots, so a turn silently moved smoothing
 * breaks to different edges), and the map-channel TVPatch entries rotate with their
 * corner/handle/interior groups (legacy left the mapping misaligned on turned patches).
 */
bool topoTurnPatches(SPatchMesh &pm, SRPatchMesh &rp,
                     const std::set<uint> &patches, bool ccw, std::string &err);

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PATCH_TOPO_H */

/* end of file */
