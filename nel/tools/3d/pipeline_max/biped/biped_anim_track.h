/**
 * \file biped_anim_track.h
 * \brief CBipedAnimTrack
 * \date 2026-07-08
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CBipedAnimTrack
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

#ifndef PIPELINE_BIPED_ANIM_TRACK_H
#define PIPELINE_BIPED_ANIM_TRACK_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <cstring>

// Project includes

namespace PIPELINE {
namespace MAX {
namespace BIPED {

/**
 * \brief CBipedAnimTrack
 * \date 2026-07-08
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * One biped animation keytrack: a (data chunk, time chunk) pair on the Biped (0x9155) system
 * object. Storage decode per pipeline_max_design.md §10c:
 *
 *   data chunk = hdr dwords (count first) + count x recSize dwords
 *                (vertical: TWO banks, each its own count dword + count x recSize)
 *   time chunk = 7 hdr dwords (count first) + count x timeRecSize dwords
 *                (timeRecSize 10 normally, 26 on tail-style per-link TCB records; time record =
 *                 (ticks, index, p0..p4, easeTo, easeFrom in UI 0..50 at [5]/[6]... see §10c —
 *                 slots [7..9] are tension, BIAS, CONTINUITY in UI units, 25 = default))
 *
 * The typed storage here is BIT-EXACT (uint32 rows, no float interpretation), so decode+encode
 * of an unmodified track reproduces the source bytes verbatim; record sizes are inferred from
 * the chunk sizes (this covers the +2-per-extra-leg-link head shift of 4-link rigs and the
 * 26-dword tail time records without special cases). Semantic float access rides on top.
 * Gated by the full-corpus T1/T2 roundtrip (byte-identical .max rebuild).
 */
class CBipedAnimTrack
{
public:
	enum ETrack
	{
		TrackHorizontal, TrackTurn, TrackVertical, TrackPelvis,
		TrackArmR, TrackArmL, TrackLegR, TrackLegL,
		TrackSpine, TrackHead, TrackTail, TrackPony1, TrackPony2,
		TrackCount
	};

	struct SDesc
	{
		ETrack Track;
		uint16 DataId, TimeId;
		uint8 DataHdrDwords;  // dwords before the first record, count included (vertical: per-bank count)
		bool DualBank;        // vertical: two banks in the data chunk
		const char *Name;
	};
	static const SDesc Descs[TrackCount];
	static const SDesc *descByDataId(uint16 id);
	static const SDesc *descByTimeId(uint16 id);

	struct SKey
	{
		std::vector<uint32> TimeRec; // full time record, raw bits; [0] = time ticks (sint32)
		std::vector<uint32> DataRec; // full data record, raw bits
	};

public:
	CBipedAnimTrack();

	/// Decode a (data, time) chunk pair. Returns false (and leaves the object unusable) on any
	/// size inconsistency — callers keep the raw chunks verbatim then.
	bool decode(const SDesc &desc, const std::vector<uint8> &data, const std::vector<uint8> &time);
	/// Re-encode; bit-exact inverse of decode for unmodified content.
	void encodeData(std::vector<uint8> &out) const;
	void encodeTime(std::vector<uint8> &out) const;

	const SDesc *desc() const { return m_Desc; }
	bool valid() const { return m_Desc != nullptr; }

	// raw typed content (bit-exact)
	std::vector<uint32> DataHdrExtra; // data hdr dwords AFTER the count
	std::vector<uint32> TimeHdrExtra; // time hdr dwords AFTER the count (6)
	std::vector<SKey> Keys;
	std::vector<std::vector<uint32> > Bank2; // vertical only (second bank records)

	// bit/float helpers
	static inline float asF(uint32 b) { float f; memcpy(&f, &b, 4); return f; }
	static inline uint32 fBits(float f) { uint32 b; memcpy(&b, &f, 4); return b; }
	static inline sint32 asI(uint32 b) { sint32 i; memcpy(&i, &b, 4); return i; }

	// semantic access
	inline size_t keyCount() const { return Keys.size(); }
	inline sint32 keyTime(size_t k) const { return asI(Keys[k].TimeRec[0]); }
	inline float dataF(size_t k, size_t idx) const { return asF(Keys[k].DataRec[idx]); }
	inline void setDataF(size_t k, size_t idx, float v) { Keys[k].DataRec[idx] = fBits(v); }
	inline void setDataI(size_t k, size_t idx, sint32 v) { uint32 b; memcpy(&b, &v, 4); Keys[k].DataRec[idx] = b; }

	/// Reset the track to freshly authored keys: count keys with the given record sizes, records
	/// zero-filled, time records (ticks, index as float, 0 x 5, 25, 25, 25) — the corpus-default
	/// TCB/ease slots. Header extras must already hold (or be set to) the per-track constants.
	void resetKeys(size_t count, size_t dataRecDwords, size_t timeRecDwords);

private:
	const SDesc *m_Desc;

}; /* class CBipedAnimTrack */

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_BIPED_ANIM_TRACK_H */

/* end of file */
