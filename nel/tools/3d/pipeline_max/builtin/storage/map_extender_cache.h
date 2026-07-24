/**
 * \file map_extender_cache.h
 * \brief CMapExtenderCache
 * \date 2026-07-17 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CMapExtenderCache
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

#ifndef PIPELINE_MAP_EXTENDER_CACHE_H
#define PIPELINE_MAP_EXTENDER_CACHE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <cstring>
#include <string>
#include <vector>

// NeL includes
#include <nel/misc/class_id.h>

// Project includes
#include "../../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

/**
 * \brief CMapExtenderCache
 * \date 2026-07-17 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The Map Extender modifier's per-node LocalModData cache: the 0x2512 payload of a Map
 * Extender (ClassId (0x2ec82081, 0x045a6271), superclass 0x810, mapext198m3.dlm) modifier
 * slot on an OSM Derived wrapper (CDerivedObject::localModData(i)). The plugin object stores
 * no settings of its own (empty 0x39bf pair); the computed UVW map channel is saved flat in
 * this cache. Format corpus-validated across every exporting instance and replayed verbatim
 * by the drop-in plugin in live Max:
 *
 * - 0x2512 is a raw LEAF whose payload is still a chunk stream on nearly every corpus
 *   instance (the container bit is clear), and a genuine typed CONTAINER elsewhere — this
 *   codec handles both forms.
 * - Functional chunk set: 0x03e8 uint32 nVerts, 0x03e9 nVerts x Point3 UVW, 0x03ea uint32
 *   nFaces, 0x03eb nFaces x (uint32 t0,t1,t2) map-face corners, 0x03f3 uint32 target map
 *   channel (1 or 2 in the corpus; default 1 when absent).
 * - Companion chunks (Mesh-format state, ignored for evaluation, kept raw): 0x03ec per-face
 *   uint16 words, 0x03ed, 0x03ee 33-byte settings blob, 0x03ef, the optional secondary
 *   channel snapshot 0x03f0/0x03f1/0x03f2, rare 0x03f4/0x03f7, 0x03f5 per-face dwords,
 *   0x03f8, the 0x03f9/0x03fa sub-containers, and 0x044c version stamp {3100, 198}.
 *
 * UVW values and face corners are stored as raw uint32 words (bit-exact rows, semantic float
 * views on top; the CBipedAnimTrack discipline), so re-encoding reproduces the stored
 * payloads verbatim.
 *
 * This is an OVERLAY CODEC, not a serialized storage class: the raw 0x2512 chunk (leaf bytes
 * or container tree) remains the serialization authority, and decode() reads it WITHOUT
 * modifying it. Decoded state holds non-owning views into the source (for selfTestReencode) —
 * valid only while the scene is parsed. selfTestReencode proves the typed model reconstructs
 * every functional chunk payload bit-exactly, and (leaf form) that the chunk-stream walk
 * covered the whole leaf; companion chunks stay raw and are surfaced through the diagnostics.
 */
class CMapExtenderCache
{
public:
	//! The Map Extender modifier's ClassId ((0x2ec82081, 0x045a6271), superclass 0x810).
	static const NLMISC::CClassId ModifierClassId;

	//! Inventory note for one top-level cache chunk (diagnostics; Size is the payload size,
	//! 0 for typed containers in the container form).
	struct SChunkNote
	{
		uint16 Id;
		uint32 Size;
		bool Container;
	};

public:
	CMapExtenderCache();

	/// Decode the 0x2512 LocalModData payload of a Map Extender modifier slot — a raw leaf
	/// (chunk-stream payload) or a typed container. Returns true iff the functional set is
	/// present and size-consistent (0x03e9 == nVerts x 12, 0x03eb == nFaces x 12); lastError()
	/// carries the reason on failure. The source is read only, never modified.
	bool decode(IStorageObject *localModData);

	//! \name Typed cache content (valid after a successful decode)
	//@{
	inline uint32 numVerts() const { return m_NumVerts; }
	inline uint32 numFaces() const { return m_NumFaces; }
	/// The target map channel (0x03f3; defaults to 1 when the chunk is absent).
	inline sint channel() const { return m_Channel; }
	inline bool hasChannel() const { return m_HasChannel; }
	/// UVW vertices as raw float-bit words, numVerts x 3 (u, v, w per vertex).
	inline const std::vector<uint32> &uvwWords() const { return m_UVWords; }
	/// Map-face corner indices into the UVW array, numFaces x 3.
	inline const std::vector<uint32> &faceCorners() const { return m_FaceCorners; }
	/// True when every face corner index is < numVerts.
	bool faceCornersValid() const;
	//@}

	//! Raw float bits to float (bit-exact; no FPU pass).
	static inline float asF(uint32 b) { float f; memcpy(&f, &b, 4); return f; }

	//! \name Diagnostics (for the corpus selftest)
	//@{
	/// True when 0x2512 was the raw-leaf form (payload re-walked as a chunk stream).
	inline bool leafForm() const { return m_LeafForm; }
	/// True when 0x2512 was an EMPTY raw leaf — a corpus-witnessed legitimate state (the
	/// modifier never evaluated, so the plugin saved an empty LocalModData; decode fails with
	/// this flag set so callers can distinguish "no cache saved" from a malformed cache).
	inline bool emptyLeaf() const { return m_EmptyLeaf; }
	/// All top-level cache chunks, in order (inventory; populated even when decode fails
	/// past the form detection).
	inline const std::vector<SChunkNote> &children() const { return m_Children; }
	/// Cache chunk ids outside the Part P known set (expected empty corpus-wide).
	inline const std::vector<uint16> &unknownIds() const { return m_UnknownIds; }
	/// Failure reason of the last decode.
	inline const std::string &lastError() const { return m_Error; }
	/// Re-encode the functional chunks (0x03e8/0x03e9/0x03ea/0x03eb/0x03f3) from the typed
	/// model and byte-compare against the stored payloads; leaf form additionally requires
	/// the walk to have covered the whole leaf. Returns false and fills \a err on mismatch.
	/// Requires the source to still be alive.
	bool selfTestReencode(std::string &err) const;
	//@}

private:
	// A view onto one top-level cache chunk's payload (non-owning).
	struct SView
	{
		uint16 Id;
		const uint8 *Data;
		uint32 Size;
	};

	void clear();
	bool walkLeaf(const uint8 *p, const uint8 *end);
	const SView *findView(uint16 id) const;
	bool decodeFunctional();

	bool m_LeafForm;
	bool m_EmptyLeaf;
	bool m_WalkComplete;
	uint32 m_NumVerts, m_NumFaces;
	sint m_Channel;
	bool m_HasChannel;
	std::vector<uint32> m_UVWords;
	std::vector<uint32> m_FaceCorners;
	std::vector<SView> m_Views;
	std::vector<SChunkNote> m_Children;
	std::vector<uint16> m_UnknownIds;
	std::string m_Error;

}; /* class CMapExtenderCache */

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MAP_EXTENDER_CACHE_H */

/* end of file */
