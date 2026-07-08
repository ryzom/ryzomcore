/**
 * \file edit_mesh_mod.h
 * \brief Decode + apply the Edit Mesh modifier (class {0x50, 0}) per-node data — the Max SDK
 * `MeshDelta` record stored in the OSM Derived wrapper's orphaned 0x2500 modifier-app containers.
 * Consolidated from pipeline_max_export_ig's corpus-validated decode (see design-doc §10w: the
 * correct chunk map is 0x0130 = created verts (16-byte srcTag+Point3), 0x0208 = created faces
 * variant A (24-byte, with srcTag), 0x0210 = created faces variant B (20-byte); the ig session's
 * fix to §10g's earlier "0x0210 = created verts" misread closed ~65 ligo diffs by itself), plus
 * an equally-decoded created-faces reader (0x0208 / 0x0210) that ig does not consume (its cluster-
 * containment link test uses vertices only, and appending faces would change its cluster
 * volumes) but cmb does (its `.cmb` output must carry those faces with their material id and
 * edge-visibility). One decode + one apply, both consumers on it, so the two copies don't drift
 * again.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_EDIT_MESH_MOD_H
#define PIPELINE_MAX_EXPORT_COMMON_EDIT_MESH_MOD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>

#include <utility>
#include <vector>

#include "../pipeline_max/storage_object.h"

namespace EDITMESH {

/// A created face carried through the modifier apply: the three vertex indices reference the
/// mesh vertex space at apply time (see applyEdits — created verts are appended before created
/// faces are laid down), `SmGroup` is Max's per-face smoothing bitmask, and `FaceFlags` is the
/// packed face-flag word (matID lives in the high 16 bits; the low bits carry Max's own edge-
/// visibility bits: EDGE_A=1, EDGE_B=2, EDGE_C=4).
struct SFace
{
	uint32 V[3];
	uint32 SmGroup;
	uint32 FaceFlags;
};

/// One per-face attribute change record (0x0220): update the input face at `Index` with the
/// packed `Values` word, gated by the `ApplyMask` word — the modern MDELTA_CHUNK's counterpart
/// to the LEGACY TOPO_ATTRIBS_CHUNK (see max_geometry_formats "Edit Mesh Modifier
/// Serialization Format" § 2.3.3).
///
/// `Values` bit layout (matches legacy):
///   bits 0..2 = edge-visibility mask (three edges, one bit each)
///   bit  3    = face-hidden flag
///   bits 5..20 = material ID (16 bits, extract via `(Values >> 5) & 0xFFFF`)
/// `ApplyMask` bit layout (modern-format hypothesis, pinned by corpus fy_hall_reunion where
/// most entries have ApplyMask=0x10 and Values with matID exactly matching the reference `.cmb`:
///   bit  0..2 = apply per-edge visibility (bit i = update edge i)
///   bit  3    = apply face-hidden flag
///   bit  4    = apply material ID
///   (legacy apply bits sat at 28..31 in a single packed word; the modern split puts them here
///    in the parallel word — 0x0220 is 12 bytes per entry vs the legacy chunk's 4)
struct SFaceAttribChange
{
	uint32 Index;
	uint32 ApplyMask;
	uint32 Values;

	inline bool applyMatId() const { return (ApplyMask & 0x10) != 0; }
	inline uint32 matId() const     { return (Values >> 5) & 0xFFFF; }
	inline bool applyEdge(int e) const { return (ApplyMask & (1u << e)) != 0; }
	inline bool edgeVis(int e) const   { return (Values & (1u << e)) != 0; }
	inline bool applyHidden() const { return (ApplyMask & 0x08) != 0; }
	inline bool hidden() const      { return (Values & 0x08) != 0; }
};

/// The decoded modifier-app record for one Edit Mesh modifier slot.
///
/// `CreatedFacesA` (0x0208, 24-byte with srcTag) and `CreatedFacesB` (0x0210, 20-byte no srcTag)
/// are BOTH created-face records, but for different meshes: `CreatedFacesA` = the base-topology
/// created faces (LEGACY TOPO_NFACES_CHUNK), `CreatedFacesB` = the map-1 texture-vertex created
/// faces (LEGACY TOPO_NTVFACES_CHUNK). Because they reference DIFFERENT vertex spaces (base vs
/// texture), applying CreatedFacesB to the base mesh causes edge-inconsistent topology
/// (`fy_hall_reunion`'s CCollisionMeshBuild::link rejects it outright — the corpus signal that
/// pinned this classification). Cmb consumers should apply CreatedFacesA only; CreatedFacesB is
/// kept for a future texture-map-aware consumer.
///
/// `FaceAttribs` (0x0220) is the per-face attribute-change table — matID + edge-vis + hidden
/// updates to the INPUT face set (before deletes/appends). Not applied by the shared applyEdits
/// (which is templated on the caller's face type and can't reach into caller-specific matID /
/// edge-vis fields); callers apply it themselves before the shared apply — see cmb's
/// `applyFaceAttribs` for the pattern. This is what makes reference-quality material IDs land
/// on the output (`fy_hall_reunion`'s reference `.cmb` has matID 60 on 69 faces where the raw
/// base mesh has matID 0; 0x0220 rewrites them).
struct SEdits
{
	std::vector<std::pair<uint32, NLMISC::CVector> > Moves;      ///< 0x0140 vertex-delta records
	std::vector<NLMISC::CVector> CreatedVerts;                    ///< 0x0130 created verts (object space)
	std::vector<SFace> CreatedFacesA;                             ///< 0x0208 base-topo created faces
	std::vector<SFace> CreatedFacesB;                             ///< 0x0210 map-1 tex created faces
	std::vector<SFaceAttribChange> FaceAttribs;                   ///< 0x0220 per-face attrib changes
	std::vector<bool> DelVerts;                                   ///< 0x0170 → 0x2700 bit array
	std::vector<bool> DelFaces;                                   ///< 0x0270 → 0x2700 bit array
};

/// Read the `MeshDelta` sub-tree off one Edit Mesh mod-app container (the OSM Derived wrapper's
/// orphaned 0x2500 chunk; the caller is responsible for locating that container per modifier
/// slot). Walks 0x2500 → 0x2512 → 0x4000 → the record children. Silently ignores chunks it does
/// not know, so future format additions leave the corpus green. Returns true iff the 0x2512 →
/// 0x4000 mesh-delta subtree was found (an Edit Mesh that touched nothing still writes an empty
/// 0x4000, so a true return + all-empty vectors is the norm).
bool readModApp(PIPELINE::MAX::CStorageContainer *c2500, SEdits &out);

/// Apply a decoded edit-record to an object-space mesh, mirroring the Max SDK's `MeshDelta::Apply`
/// semantics: moves, then face-deletes on the input face set, then **created verts appended to
/// the input vert set** (0x0130), then — per \a facesMode — **the 0x0208 created faces appended
/// with V[3] as stored** (they reference the union input⊕created vert space, no offset needed),
/// then **vert-deletes remap ALL face indices** (both surviving-original and appended-created).
/// Order matters: earlier iterations offset created V[3] onto the post-delete vert count,
/// double-counting the original vert range and producing edge-inconsistent meshes that
/// `CCollisionMeshBuild::link` rejected outright.
///
/// Templated on the caller's face type (must expose a mutable `V[3]` index array); \a FaceFactory
/// is a functor `Face(uint32 vOffset, const SFace &)` returning the caller's face type. Only
/// invoked for the applied face records. \a facesMode selects which face record to apply:
///   0 = no created faces (ig's cluster-containment test, verts-only)
///   1 = only `CreatedFacesA` (0x0208) — cmb's default, corpus-validated against
///       `~/pipeline_export/.../FY_hall_reunion.cmb` (input 82 - 2 deleted + 19 from 0x0208 =
///       99 for this node; total across nodes matches ref exact). 0x0210 is deliberately NOT
///       applied here; see the SEdits header comment for the reasoning.
template <typename Face, typename FaceFactory>
void applyEdits(const SEdits &e, std::vector<NLMISC::CVector> &verts, std::vector<Face> &faces,
                FaceFactory faceFactory, int facesMode)
{
	// Moves — position deltas on original verts, indices in input space.
	for (uint i = 0; i < e.Moves.size(); ++i)
		if (e.Moves[i].first < verts.size())
			verts[e.Moves[i].first] += e.Moves[i].second;
	// Face deletes — bitmap indexes the input face list.
	if (!e.DelFaces.empty())
	{
		std::vector<Face> kept;
		kept.reserve(faces.size());
		for (uint i = 0; i < faces.size(); ++i)
			if (i >= e.DelFaces.size() || !e.DelFaces[i])
				kept.push_back(faces[i]);
		faces.swap(kept);
	}
	// Created verts appended to input verts — creates the union space that created-face V[3]
	// references.
	verts.insert(verts.end(), e.CreatedVerts.begin(), e.CreatedVerts.end());
	// Created faces appended — V[3] is stored in the union space, so vOffset arg = 0.
	if (facesMode >= 1)
	{
		faces.reserve(faces.size() + e.CreatedFacesA.size());
		for (uint i = 0; i < e.CreatedFacesA.size(); ++i)
			faces.push_back(faceFactory(0, e.CreatedFacesA[i]));
	}
	// Vert deletes LAST — remap ALL face indices (input + appended created).
	if (!e.DelVerts.empty())
	{
		std::vector<uint32> remap(verts.size());
		std::vector<NLMISC::CVector> kept;
		kept.reserve(verts.size());
		for (uint i = 0; i < verts.size(); ++i)
		{
			remap[i] = (uint32)kept.size();
			if (i >= e.DelVerts.size() || !e.DelVerts[i])
				kept.push_back(verts[i]);
		}
		verts.swap(kept);
		for (uint i = 0; i < faces.size(); ++i)
			for (int j = 0; j < 3; ++j)
				if (faces[i].V[j] < remap.size())
					faces[i].V[j] = remap[faces[i].V[j]];
	}
}

} /* namespace EDITMESH */

#endif /* PIPELINE_MAX_EXPORT_COMMON_EDIT_MESH_MOD_H */

/* end of file */
