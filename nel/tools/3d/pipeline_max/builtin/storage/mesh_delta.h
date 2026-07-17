/**
 * \file mesh_delta.h
 * \brief CMeshDelta
 * \date 2026-07-17 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CMeshDelta
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

#ifndef PIPELINE_MESH_DELTA_H
#define PIPELINE_MESH_DELTA_H
#include <nel/misc/types_nl.h>

// STL includes
#include <cstring>
#include <string>
#include <vector>

// NeL includes

// Project includes
#include "../../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

/**
 * \brief CMeshDelta
 * \date 2026-07-17 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The Edit Mesh modifier's per-node MeshDelta record — the 0x2512 LocalModData payload of an
 * Edit Mesh (ClassId (0x50, 0), superclass 0x810) modifier slot on an OSM Derived wrapper
 * (CDerivedObject::localModData(i)). Format per max_geometry_formats Part L / design-doc
 * §10w/§10x/§10z-ter/§10z-quinze, corpus-validated by the ig/cmb/shape exporters:
 *
 * - 0x2512 is a CONTAINER whose children are the modern local-data chunks (§L.5.1): 0x2740
 *   instance flags (4 B, every instance), the optional named-selection-set lists
 *   0x2845/0x2847/0x2846 (containers; only 0x2846 observed, 12 instances), and the 0x4000
 *   MDELTA container that carries the geometry deltas. The legacy pre-MDELTA chunk set
 *   (§L.5.2: 0x2750/0x2755/0x2756/0x2760/0x2761/0x2765/0x3000) does not occur anywhere in
 *   the corpus (Max 3 snowballs included) and is NOT decoded here — such ids would surface
 *   through unknownLocalDataIds().
 * - 0x4000 children, each a count-prefixed record table (uint32 count + count x stride):
 *   0x0140 vertex moves (16 B: index + Point3 delta), 0x0130 created verts (16 B: srcTag +
 *   Point3 — srcTag == -1 fresh/absolute, else CLONE of input vert srcTag with Pos = offset
 *   from the source's PRE-move position), 0x0208 created base-topology faces (24 B: srcTag +
 *   v[3] + smGroup + flagsMatId), 0x0210 face-vertex remap (20 B: faceIdx + applyMask + v[3]),
 *   0x0220 per-face attribute changes (12 B: faceIdx + applyMask + values); plus the 4-byte
 *   leaves 0x0100 input vert count / 0x0110 input face count (every instance) / 0x0300
 *   subobject level, the delete bitmaps 0x0170 (verts) / 0x0270 (faces) — containers holding
 *   EXACTLY ONE 0x2700 bit-array leaf (uint32 bitCount + LSB-first packed bits, dword-padded
 *   with zero pad bits on every corpus instance) — the selection containers
 *   0x0400/0x0410/0x0420/0x0430 (same one-0x2700 shape; state, not geometry; kept raw), and
 *   a recognized-but-untyped map-channel/UI family kept raw (0x0320/0x0324/0x0328 4-byte
 *   triple, record tables 0x0230/0x0330/0x0334/0x0338/0x033b, container 0x0340, rare
 *   0x0120/0x0200/0x0360 — see mesh_delta.cpp for the corpus-inventoried shapes).
 *
 * CRITICAL FORMAT FACT: the 0x0210 records carry UNINITIALIZED WRITER BYTES in the v[i] words
 * of corners not covered by applyMask (corpus-witnessed). Every record field is therefore
 * stored as raw uint32 words — bit-exact rows with semantic accessors on top, the
 * CBipedAnimTrack discipline (§10t) — so re-encoding an unmodified record reproduces the
 * stored bytes verbatim, uninitialized corners included.
 *
 * This is an OVERLAY CODEC, not a serialized storage class: the raw 0x2512 chunk tree remains
 * the serialization authority (nothing here touches createChunkById or the lifecycle), and
 * decode() reads the tree WITHOUT modifying it. Decoded state holds non-owning pointers into
 * the source tree (for selfTestReencode) — valid only while the scene is parsed, like
 * CDerivedObject's slot model. selfTestReencode proves the typed rows reconstruct every typed
 * chunk's payload bit-exactly; chunks outside the typed set stay raw in the tree and are
 * surfaced through the diagnostics (irregular/unknown id lists, expected empty corpus-wide).
 */
class CMeshDelta
{
public:
	//! One 0x0140 vertex-move record: input vertex \a Index moved by the Point3 delta in \a P
	//! (raw float bits).
	struct SMove
	{
		uint32 Index;
		uint32 P[3];
	};

	//! One 0x0130 created-vertex record. SrcTag == 0xFFFFFFFF: fresh vertex, \a P is the
	//! absolute object-space Point3. Otherwise: CLONE of input vertex SrcTag, \a P is the
	//! offset from the source vertex's PRE-move position (§10z-quinze).
	struct SCreatedVert
	{
		uint32 SrcTag;
		uint32 P[3];
	};

	//! One 0x0208 created base-topology face record. SrcTag is the authoring-history hint
	//! (ignored on evaluation). V[3] references the union input+created vertex space. SmGroup
	//! is the smoothing bitmask; FaceFlags packs edge visibility (low 3 bits) + matID (high
	//! 16 bits).
	struct SCreatedFace
	{
		uint32 SrcTag;
		uint32 V[3];
		uint32 SmGroup;
		uint32 FaceFlags;
	};

	//! One 0x0210 face-vertex-remap record: on input face \a Index, per corner c in {0,1,2},
	//! if ApplyMask bit c is set, replace the corner's vertex index with V[c]. Corners not
	//! covered by ApplyMask carry uninitialized writer bytes (retained verbatim; never read
	//! them semantically).
	struct SFaceVertRemap
	{
		uint32 Index;
		uint32 ApplyMask;
		uint32 V[3];

		inline bool applyCorner(int c) const { return (ApplyMask & (1u << c)) != 0; }
	};

	//! One 0x0220 per-face attribute-change record: update input face \a Index with the packed
	//! \a Values word, gated by \a ApplyMask. Values: bits 0..2 edge visibility, bit 3 hidden,
	//! bits 5..20 matID. ApplyMask: bits 0..2 apply per-edge, bit 3 apply hidden, bit 4 apply
	//! matID (the legacy TOPO_ATTRIBS bits 28..31 split into a parallel word, §L.5.3).
	struct SFaceAttrib
	{
		uint32 Index;
		uint32 ApplyMask;
		uint32 Values;

		inline bool applyMatId() const { return (ApplyMask & 0x10) != 0; }
		inline uint32 matId() const { return (Values >> 5) & 0xFFFF; }
		inline bool applyEdge(int e) const { return (ApplyMask & (1u << e)) != 0; }
		inline bool edgeVis(int e) const { return (Values & (1u << e)) != 0; }
		inline bool applyHidden() const { return (ApplyMask & 0x08) != 0; }
		inline bool hidden() const { return (Values & 0x08) != 0; }
	};

	//! A decoded 0x2700 bit array (delete bitmap): uint32 bit count + the packed payload bytes
	//! AFTER the count dword, verbatim (LSB-first bits; any padding bytes retained as stored).
	struct SBitArray
	{
		bool Present;
		uint32 BitCount;
		std::vector<uint8> Packed;

		SBitArray() : Present(false), BitCount(0) { }
		//! Expand to one bool per bit (LSB-first within each byte).
		void bits(std::vector<bool> &out) const;
	};

	//! Inventory note for one child chunk (diagnostics; Size is the raw payload size, 0 for
	//! containers).
	struct SChunkNote
	{
		uint16 Id;
		uint32 Size;
		bool Container;
	};

public:
	CMeshDelta();

	/// Decode the 0x2512 LocalModData payload of an Edit Mesh modifier slot. Returns true iff
	/// \a localModData is a container carrying a 0x4000 MDELTA container (an Edit Mesh that
	/// touched nothing still writes an empty 0x4000, so true + all-empty tables is the norm).
	/// The tree is read only, never modified; decoded state references it (valid while parsed).
	bool decode(IStorageObject *localModData);

	//! \name Typed record tables (valid after a successful decode)
	//@{
	inline const std::vector<SMove> &moves() const { return m_Moves; }
	inline const std::vector<SCreatedVert> &createdVerts() const { return m_CreatedVerts; }
	inline const std::vector<SCreatedFace> &createdFaces() const { return m_CreatedFaces; }
	inline const std::vector<SFaceVertRemap> &faceRemap() const { return m_FaceRemap; }
	inline const std::vector<SFaceAttrib> &faceAttribs() const { return m_FaceAttribs; }
	inline const SBitArray &delVerts() const { return m_DelVerts; }
	inline const SBitArray &delFaces() const { return m_DelFaces; }
	inline bool hasInputVertCount() const { return m_HasInputVertCount; }
	inline uint32 inputVertCount() const { return m_InputVertCount; }
	inline bool hasInputFaceCount() const { return m_HasInputFaceCount; }
	inline uint32 inputFaceCount() const { return m_InputFaceCount; }
	inline bool hasSubObjLevel() const { return m_HasSubObjLevel; }
	inline uint32 subObjLevel() const { return m_SubObjLevel; }
	//@}

	//! Raw float bits to float (bit-exact; no FPU pass).
	static inline float asF(uint32 b) { float f; memcpy(&f, &b, 4); return f; }

	//! \name Diagnostics (for the corpus selftest)
	//@{
	/// All direct children of the 0x2512 container, in order (inventory).
	inline const std::vector<SChunkNote> &localDataChildren() const { return m_LocalDataChildren; }
	/// All direct children of the 0x4000 MDELTA container, in order (inventory).
	inline const std::vector<SChunkNote> &deltaChildren() const { return m_DeltaChildren; }
	/// 0x2512-level child ids outside {0x2740, 0x2845, 0x2846, 0x2847, 0x4000} (expected empty).
	inline const std::vector<uint16> &unknownLocalDataIds() const { return m_UnknownLocalDataIds; }
	/// 0x4000-level child ids outside the typed + recognized set (expected empty).
	inline const std::vector<uint16> &unknownDeltaIds() const { return m_UnknownDeltaIds; }
	/// Recognized ids whose observed shape did not match the format rule (wrong leaf/container
	/// kind, size not exactly 4 + count x stride, bitmap container without exactly one wellformed
	/// 0x2700 leaf). Such chunks are left untyped (raw stays authoritative). Expected empty.
	inline const std::vector<uint16> &irregularIds() const { return m_IrregularIds; }
	/// Count of 0x4000 containers beyond the first under 0x2512 (expected 0).
	inline uint extraDeltaContainers() const { return m_ExtraDeltaContainers; }
	/// Re-encode every typed chunk from the typed rows and byte-compare against the stored
	/// payloads (uninitialized corner words included — they ride the raw rows). Returns false
	/// and fills \a err on the first mismatch. Requires the source tree to still be alive.
	bool selfTestReencode(std::string &err) const;
	//@}

private:
	void clear();
	bool decodeRows(uint16 id, IStorageObject *obj, uint stride, uint kind);
	bool decodeCountLeaf(uint16 id, IStorageObject *obj, uint32 &value, bool &present);
	bool decodeBitArray(uint16 id, IStorageObject *obj, SBitArray &out);

	// One typed chunk occurrence, for the re-encode proof: which rows of which table came from
	// which raw chunk.
	struct SSegment
	{
		uint16 Id;
		uint kind; // 0 moves, 1 createdVerts, 2 createdFaces, 3 faceRemap, 4 faceAttribs,
		           // 5 count leaf, 6 bit array
		const CStorageRaw *Raw; // the record table leaf (kinds 0..5) or the 0x2700 leaf (kind 6)
		size_t Row0, Rows;      // kinds 0..4: row range in the typed table
		uint32 Value;           // kind 5: the count value
		const SBitArray *Bits;  // kind 6
	};

	std::vector<SMove> m_Moves;
	std::vector<SCreatedVert> m_CreatedVerts;
	std::vector<SCreatedFace> m_CreatedFaces;
	std::vector<SFaceVertRemap> m_FaceRemap;
	std::vector<SFaceAttrib> m_FaceAttribs;
	SBitArray m_DelVerts;
	SBitArray m_DelFaces;
	bool m_HasInputVertCount, m_HasInputFaceCount, m_HasSubObjLevel;
	uint32 m_InputVertCount, m_InputFaceCount, m_SubObjLevel;

	std::vector<SChunkNote> m_LocalDataChildren;
	std::vector<SChunkNote> m_DeltaChildren;
	std::vector<uint16> m_UnknownLocalDataIds;
	std::vector<uint16> m_UnknownDeltaIds;
	std::vector<uint16> m_IrregularIds;
	uint m_ExtraDeltaContainers;
	std::vector<SSegment> m_Segments;

}; /* class CMeshDelta */

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MESH_DELTA_H */

/* end of file */
