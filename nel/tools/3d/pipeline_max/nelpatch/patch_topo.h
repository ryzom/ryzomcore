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

/**
 * Subdivide the listed quad patches into four children each (patch_topo_subdiv.cpp).
 *
 * Geometry is the exact bicubic split: de Casteljau at 0.5 in both directions (long-double
 * midpoint cascades, the house x87 style), so the surface is unchanged. Children keep the
 * PARENT'S ring orientation - each child's ring starts at its sub-domain's origin corner -
 * so the painted tile grid copies by plain quadrant translation, no rotation: child
 * NbTiles are the parent's minus one, tiles and colors copy verbatim from their quadrant
 * (the color midlines duplicate into both children), and outer edge flags follow their
 * parent edge. This is the paint-inheritance headline: subdividing a painted patch keeps
 * its painted appearance exactly.
 *
 * Edges: a split edge reuses its record for the first half (midpoint replaces V2) and its
 * tangent slots for the outer controls, so vec owners never change; the inner half and the
 * four centre cross edges are new records. An edge shared with an UNSELECTED patch is NOT
 * split: the neighbor keeps the whole original edge, the children ride two new half edges,
 * and the midpoint vertex binds BIND_SINGLE onto the neighbor's edge - the canonical NeL
 * T-junction, exactly the shape the corpus authored (and the midpoint IS the bindWhere
 * 0.5 point by construction, so the bind refresh moves nothing).
 *
 * Refuses: reconstructed (Max 3) streams, hook tables, map-channel meshes (TVPatch
 * assignment for children needs a design pass), patches with a tile order of 1 (halving
 * needs NbTiles >= 1 on both axes), patches whose corners are bound, and patches whose
 * edges are bind targets (splitting a T-junction target breaks the junction).
 *
 * Mapper meshes: a mapper-driven output's stored position is a CACHE - evaluation rebuilds
 * it as input + Delta - so `evalPm` (the caller's evaluated mirror, parallel element
 * tables) supplies the live positions the split is computed FROM, and a position write to
 * a mapped output shifts its record's Delta so the value shows after evaluation. Without
 * them (NULL) the stored positions are treated as live, which is only right for
 * un-mapped streams.
 */
bool topoSubdividePatches(SPatchMesh &pm, SRPatchMesh &rp,
                          const std::set<uint> &patches, std::string &err,
                          SPmVertMapper *mapper = NULL, const SPatchMesh *evalPm = NULL);

/**
 * Weld the listed vertices: clusters within `threshold` (transitive, measured on the
 * EFFECTIVE object-space positions - `evalPm` when given, stored otherwise) merge onto
 * their lowest-index member, which KEEPS ITS OWN position - the target-weld shape. No
 * position is written at all, so the op is mapper-safe by construction (a mapper-driven
 * vertex's stored position is dead bytes; moving it would silently not show).
 *
 * When a merge makes two OPEN edges coincide (same endpoint pair), they fuse into one
 * shared edge - the stitch that joins two sheets. The surviving (lower) edge keeps its
 * tangent pair; patches of the dropped edge rewire their edge slot AND their tangent
 * slots to the survivor's (orientation-aware), so both sides render the same curve and
 * the seam closes exactly. Dropped cluster verts, fused-away edges and their tangent
 * vecs sweep out with the delete-style compaction; the mapper's dropped outputs flip to
 * -1, survivors remap.
 *
 * Refuses: reconstructed (Max 3) streams, hooks, bound cluster members (release binds
 * first), merges that would put more than two patches on one edge, and merges that would
 * degenerate a patch (two of its corners in one cluster). Returns with err "nothing
 * within threshold" when no cluster has two members. `remap` reports the vertex map like
 * the delete op (paint is per patch and does not move).
 */
bool topoWeldVerts(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                   const std::set<uint> &verts, float threshold,
                   STopoRemap &remap, std::string &err,
                   const SPatchMesh *evalPm = NULL);

/**
 * Directed (target) weld: `srcVert` merges into `dstVert`, which keeps its position and
 * identity - the drag-onto-a-vertex gesture, no threshold, no clustering. Same machinery
 * and refusals as topoWeldVerts otherwise; when the merge fuses two open edges, the edge
 * whose endpoints did not move wins the seam curve (the target-weld rule, shared with the
 * cluster weld).
 */
bool topoWeldVertInto(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                      uint srcVert, uint dstVert, STopoRemap &remap, std::string &err);

/**
 * Grow one new quad patch from each listed OPEN edge (edge indices into pm.Edges; each
 * must carry exactly one patch and no bind records). The new patch mirrors its owner
 * across the edge: each far corner is the point reflection of the owner's opposite corner
 * through the shared corner (2*Ve - Vopp), tangents mirrored the same way, so the seed
 * continues the surface C1-ish - the artist moves the new corners where they belong and
 * welds. The shared side reuses the open edge record (it gains its second patch); far and
 * side edges, their tangents and four interiors are new. Ring winding opposes the owner
 * across the shared edge, as any two neighbors' rings do.
 *
 * The new patch inherits the owner's SmGroup/Flags and tile ORDERS; its tiles start
 * EMPTY and its colors white - fresh paintable surface, the legacy default state.
 * Refuses: reconstructed streams, hooks, map-channel meshes, non-open edges, and edges
 * with bind records targeting them. `evalPm` supplies the live positions the mirror
 * seeds are computed from on mapper meshes (see topoSubdividePatches).
 */
bool topoAddQuads(SPatchMesh &pm, SRPatchMesh &rp,
                  const std::set<uint> &edges, std::string &err,
                  const SPatchMesh *evalPm = NULL);

/**
 * Detach the listed patches as their own ISLAND inside the same mesh
 * (patch_topo_detach.cpp) - detach-to-element. Every vertex and edge shared between the
 * selection and the rest splits: the original keeps the complement side, the selection
 * rides duplicates (same positions, copied tangent curves), so both sides render
 * identically until one is edited - nothing moves, nothing dies, nothing renumbers, and
 * the zone still exports as one node (the ligo brick export refuses multi-node files,
 * which is why detach never creates a scene node here). Binds whose anchor and target
 * end up on different sides release whole (the unbind group rule). Paint is untouched.
 * Map-channel meshes are fine: patch count and TVPatch rows do not change.
 *
 * Refuses: reconstructed (Max 3) streams, hooks, the whole-mesh selection, and a
 * selection that is already a separate element (no shared vertex). `evalPm` supplies the
 * positions the (unmapped) duplicates copy, so a mapper-driven boundary stays an
 * invisible seam (see topoSubdividePatches).
 */
bool topoDetachElements(SPatchMesh &pm, SRPatchMesh &rp,
                        const std::set<uint> &sel, std::string &err,
                        const SPatchMesh *evalPm = NULL);

/**
 * Append `src`/`srcRp` onto `pm`/`rp` (patch_topo_attach.cpp) - the attach merge. Every
 * source element is copied with its cross references shifted by the target's element
 * counts and its positions transformed through `relTM` (row-major 3x4, source object
 * space -> target object space). Per-patch paint records travel verbatim - tiles are
 * authored in the patch frame, which the reorientation does not touch - so attach keeps
 * the source's painted appearance. Source bind records retarget through the offsets with
 * their caches reset; the source's selection state does not travel, the target's
 * selection BitArrays grow with the appended elements unselected. The target keeps its
 * own RPatchMesh header fields (tile mode, trailer shape).
 *
 * Refuses: reconstructed (Max 3) streams and hook tables on either side, map-channel
 * meshes on either side, parallel-size mismatches, and an empty source.
 */
bool topoAppendMesh(SPatchMesh &pm, SRPatchMesh &rp,
                    const SPatchMesh &src, const SRPatchMesh &srcRp,
                    const double relTM[12], std::string &err);

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PATCH_TOPO_H */

/* end of file */
