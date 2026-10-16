/**
 * \file edit_mesh_mod.h
 * \brief Decode + apply the Edit Mesh modifier (class {0x50, 0}) per-node data — the Max SDK
 * `MeshDelta` record stored in the OSM Derived wrapper's orphaned 0x2500 modifier-app containers.
 * Consolidated from pipeline_max_export_ig's corpus-validated decode (see design-doc §10w: the
 * correct chunk map is 0x0130 = created verts (16-byte srcTag+Point3), 0x0208 = created faces
 * (24-byte, with srcTag); the ig session's fix to §10g's earlier "0x0210 = created verts" misread
 * closed ~65 ligo diffs by itself). 0x0210 is now decoded as the FACE-VERTEX-REMAP table (modern
 * equivalent of legacy TOPO_FACEMAP_CHUNK 0x2780) — 20-byte stride records `(uint32 faceIdx,
 * uint32 applyMask, uint32 v[3])` where `applyMask` bits 0..2 select which of face `faceIdx`'s
 * corners get replaced with the corresponding `v[i]`. Corpus-verified across 445 files / 113
 * chunks / 2881 entries: every observed mask is in 0..7 and every faceIdx fits in a real face
 * range. This closes the "one face-index remap" §10x listed as an open item, and generalises to
 * the face diffs on the other Edit-Mesh direct-tier files. One decode + one apply, both consumers
 * on it, so the two copies don't drift again.
 *
 * The chunk-level decode has since graduated into the pipeline_max library
 * (BUILTIN::STORAGE::CMeshDelta, bit-exact rows corpus-selftested — design-doc §10j-sept);
 * readModApp is a thin copy from that typed model into SEdits. The evaluation semantics
 * (applyEdits and the SEdits shape) stay here — they are exporter policy, not file format.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
 * \author Claude Fable 5
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

#include <map>
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

/// One face-vertex-remap record (0x0210 modern-format, ex-legacy TOPO_FACEMAP_CHUNK 0x2780):
/// on face `Index`, per-corner `i` in {0,1,2}, if `ApplyMask & (1u<<i)` is set, replace the
/// corner's vertex index with `V[i]`. Corners not covered by ApplyMask carry undefined bytes in
/// the file (uninitialised writer memory — corpus-witnessed) and must be ignored. In legacy the
/// same semantic lived in a per-corner `V[3]` with sentinel `0xFFFFFFFF` (§ L.5.3); modern splits
/// the apply bits into a parallel word, exactly the same shape the modern 0x0220 face-attrib
/// change record has (parallel apply + values words).
struct SFaceVertRemap
{
	uint32 Index;
	uint32 ApplyMask;
	uint32 V[3];

	inline bool applyCorner(int c) const { return (ApplyMask & (1u << c)) != 0; }
};

/// One 0x0130 created-vertex record. `SrcTag == 0xFFFFFFFF` → a fresh vertex, `Pos` is the
/// absolute object-space position. `SrcTag != -1` → a CLONE of input vertex `SrcTag`, `Pos` is
/// the OFFSET from the source vertex's (post-move) position — the modern merge of legacy
/// TOPO_CVERTS_CHUNK clone-sources + the clone's own move (§ L.5/§ L.6; chamfer/extrude record
/// their geometry this way). Decoded off the primes_racines sky domes (design doc §10z-quinze).
struct SCreatedVert
{
	uint32 SrcTag;
	NLMISC::CVector Pos;
};

/// The decoded modifier-app record for one Edit Mesh modifier slot.
///
/// `CreatedFacesA` (0x0208, 24-byte with srcTag) is the base-topology created-faces record
/// (legacy TOPO_NFACES_CHUNK) — the record cmb appends after the input faces have been remapped
/// and the deletes applied. `FaceRemap` (0x0210, 20-byte) is the FACE-VERTEX-REMAP table (legacy
/// TOPO_FACEMAP_CHUNK 0x2780; see `SFaceVertRemap` — per-face per-corner index rewrites) —
/// applied to the INPUT face set BEFORE deletes so the remapped face indices are what the deleter
/// sees, and BEFORE the created-vert append so a remap that points to a not-yet-appended created
/// vert stays valid (the append is later in the same pass and lands the created verts at exactly
/// the referenced indices).
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
	std::vector<SCreatedVert> CreatedVerts;                       ///< 0x0130 created verts (absolute or clone+offset)
	std::vector<SFace> CreatedFacesA;                             ///< 0x0208 base-topo created faces
	std::vector<SFaceVertRemap> FaceRemap;                        ///< 0x0210 face-vertex remap
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
/// semantics: moves, then the 0x0210 FACE-VERTEX REMAP over the input face set (per-corner index
/// rewrites, indices may reference verts that don't exist yet — the append below will land the
/// created verts at exactly those indices), then face-deletes on the input face set, then
/// **created verts appended to the input vert set** (0x0130), then — per \a facesMode — **the
/// 0x0208 created faces appended with V[3] as stored** (they reference the union input⊕created
/// vert space, no offset needed), then **vert-deletes remap ALL face indices** (both surviving-
/// original and appended-created). Order matters: earlier iterations offset created V[3] onto the
/// post-delete vert count, double-counting the original vert range and producing edge-
/// inconsistent meshes that `CCollisionMeshBuild::link` rejected outright.
///
/// Templated on the caller's face type (must expose a mutable `V[3]` index array); \a FaceFactory
/// is a functor `Face(uint32 vOffset, const SFace &)` returning the caller's face type. Only
/// invoked for the appended CreatedFacesA records. \a facesMode selects which face records the
/// caller wants applied downstream (both branches ALWAYS apply the FaceRemap on the input set —
/// remap is topology-preserving so ig's cluster-containment link test benefits too):
///   0 = no created faces (ig's cluster-containment test, verts-only)
///   1 = append `CreatedFacesA` (0x0208) — cmb's default, corpus-validated against
///       `~/pipeline_export/.../FY_hall_reunion.cmb` (input 82 - 2 deleted + 19 from 0x0208 =
///       99 for this node; total across nodes matches ref exact).
template <typename Face, typename FaceFactory>
void applyEdits(const SEdits &e, std::vector<NLMISC::CVector> &verts, std::vector<Face> &faces,
                FaceFactory faceFactory, int facesMode)
{
	// Clone-source snapshot — a clone record's offset is relative to the source vertex's
	// PRE-move position (the legacy effect order emits clones at step 3 and moves at step 13,
	// §L.6; corpus-proven: zo_bt_hall_reunion_vitrine's 25 clones-of-moved-sources land exactly
	// one move-delta off when resolved post-move). Snapshot just the needed sources.
	std::map<uint32, NLMISC::CVector> cloneSrc;
	for (uint i = 0; i < e.CreatedVerts.size(); ++i)
		if (e.CreatedVerts[i].SrcTag != 0xFFFFFFFF && e.CreatedVerts[i].SrcTag < verts.size())
			cloneSrc[e.CreatedVerts[i].SrcTag] = verts[e.CreatedVerts[i].SrcTag];
	// Moves — position deltas on original verts, indices in input space.
	for (uint i = 0; i < e.Moves.size(); ++i)
		if (e.Moves[i].first < verts.size())
			verts[e.Moves[i].first] += e.Moves[i].second;
	// Face-vertex remap — per-corner index rewrites on input faces. Applied BEFORE deletes so
	// the deleter sees post-remap indices (irrelevant to which face gets deleted, but keeps the
	// invariant "each stage sees the mesh state the next stage expects"), and BEFORE created-vert
	// append so a remap pointing to a not-yet-appended created vert index stays valid — the append
	// below lands the created verts at exactly those indices. Corners not covered by ApplyMask
	// carry undefined bytes in the file and must not be touched.
	for (uint i = 0; i < e.FaceRemap.size(); ++i)
	{
		const SFaceVertRemap &r = e.FaceRemap[i];
		if (r.Index >= faces.size()) continue;
		for (int c = 0; c < 3; ++c)
			if (r.applyCorner(c))
				faces[r.Index].V[c] = r.V[c];
	}
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
	// references. A clone record (SrcTag != -1) resolves against the source vertex's PRE-move
	// position (the snapshot above); a fresh record (SrcTag == -1) is an absolute position.
	verts.reserve(verts.size() + e.CreatedVerts.size());
	for (uint i = 0; i < e.CreatedVerts.size(); ++i)
	{
		const SCreatedVert &cv = e.CreatedVerts[i];
		std::map<uint32, NLMISC::CVector>::const_iterator src = cloneSrc.find(cv.SrcTag);
		if (src != cloneSrc.end())
			verts.push_back(src->second + cv.Pos);
		else
			verts.push_back(cv.Pos);
	}
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
