/**
 * \file biped_anim_track.cpp
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

#include <nel/misc/types_nl.h>
#include "biped_anim_track.h"

// Project includes
#include "../storage_object.h" // nlVectorData

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BIPED {

const CBipedAnimTrack::SDesc CBipedAnimTrack::Descs[CBipedAnimTrack::TrackCount] = {
	{ TrackHorizontal, 0x012c, 0x012d, 1, false, "horizontal" },
	{ TrackTurn,       0x012e, 0x012f, 3, false, "turn" },
	{ TrackVertical,   0x0130, 0x0131, 1, true,  "vertical" },
	{ TrackPelvis,     0x0132, 0x0133, 3, false, "pelvis" },
	{ TrackArmR,       0x0134, 0x0135, 4, false, "armR" },
	{ TrackArmL,       0x0136, 0x0137, 4, false, "armL" },
	{ TrackLegR,       0x0138, 0x0139, 4, false, "legR" },
	{ TrackLegL,       0x013a, 0x013b, 4, false, "legL" },
	{ TrackSpine,      0x013c, 0x013d, 4, false, "spine" },
	{ TrackHead,       0x013e, 0x013f, 3, false, "head" },
	{ TrackTail,       0x0142, 0x0143, 4, false, "tail" },
	{ TrackPony1,      0x0147, 0x0148, 4, false, "pony1" },
	{ TrackPony2,      0x0149, 0x014a, 4, false, "pony2" },
};

const CBipedAnimTrack::SDesc *CBipedAnimTrack::descByDataId(uint16 id)
{
	for (int i = 0; i < TrackCount; ++i)
		if (Descs[i].DataId == id) return &Descs[i];
	return NULL;
}

const CBipedAnimTrack::SDesc *CBipedAnimTrack::descByTimeId(uint16 id)
{
	for (int i = 0; i < TrackCount; ++i)
		if (Descs[i].TimeId == id) return &Descs[i];
	return NULL;
}

CBipedAnimTrack::CBipedAnimTrack() : m_Desc(NULL)
{

}

// Little-endian dword views over the chunk byte arrays.
static inline const uint32 *dwords(const std::vector<uint8> &v) { return reinterpret_cast<const uint32 *>(nlVectorData(v)); }

bool CBipedAnimTrack::decode(const SDesc &desc, const std::vector<uint8> &data, const std::vector<uint8> &time)
{
	m_Desc = NULL;
	DataHdrExtra.clear();
	TimeHdrExtra.clear();
	Keys.clear();
	Bank2.clear();

	if (data.size() % 4 || time.size() % 4) return false;
	size_t dn = data.size() / 4, tn = time.size() / 4;
	if (dn < desc.DataHdrDwords || tn < 7) return false;
	const uint32 *d = dwords(data);
	const uint32 *t = dwords(time);

	sint32 count = asI(d[0]);
	sint32 tcount = asI(t[0]);
	if (count < 0 || count > 100000) return false;
	if (tcount != count) return false;

	// time records
	size_t trec = 0;
	if (count)
	{
		if ((tn - 7) % (size_t)count) return false;
		trec = (tn - 7) / (size_t)count;
		if (trec < 10) return false;
	}
	else if (tn != 7) return false;

	// data records
	size_t rec = 0;
	sint32 count2 = 0;
	size_t bank2At = 0; // dword offset of the second bank's count
	if (desc.DualBank)
	{
		// [count][count x rec][count2][count2 x rec]; rec inferred over both banks
		if (count == 0)
		{
			if (dn < 2) return false;
			count2 = asI(d[1]);
			if (count2 < 0 || count2 > 100000) return false;
			if (count2 == 0) { if (dn != 2) return false; }
			else
			{
				if ((dn - 2) % (size_t)count2) return false;
				rec = (dn - 2) / (size_t)count2;
			}
			bank2At = 1;
		}
		else
		{
			// bank1 rec size: solve rec from total = 1 + c1*rec + 1 + c2*rec with c2 read at the
			// bank boundary. The corpus vertical record is 13 dwords — try that first (a blind
			// scan could mis-split a pathological byte pattern), then scan for any consistent
			// total (rec identical across banks; the second count dword sits at 1 + count*rec).
			bool found = false;
			for (size_t pass = 0; !found && pass < 2; ++pass)
			{
				size_t rFrom = pass ? 1 : 13, rTo = pass ? dn : 13;
				for (size_t r = rFrom; r <= rTo; ++r)
				{
					size_t at = 1 + (size_t)count * r;
					if (at + 1 > dn) break;
					sint32 c2 = asI(d[at]);
					if (c2 < 0 || c2 > 100000) continue;
					if (at + 1 + (size_t)c2 * r == dn)
					{
						rec = r; count2 = c2; bank2At = at; found = true;
						break;
					}
				}
			}
			if (!found) return false;
		}
	}
	else
	{
		if (count == 0)
		{
			if (dn != desc.DataHdrDwords) return false;
		}
		else
		{
			if ((dn - desc.DataHdrDwords) % (size_t)count) return false;
			rec = (dn - desc.DataHdrDwords) / (size_t)count;
			if (!rec) return false;
		}
	}

	// lift
	if (!desc.DualBank)
		DataHdrExtra.assign(d + 1, d + desc.DataHdrDwords);
	TimeHdrExtra.assign(t + 1, t + 7);
	Keys.resize(count);
	for (sint32 k = 0; k < count; ++k)
	{
		Keys[k].TimeRec.assign(t + 7 + (size_t)k * trec, t + 7 + (size_t)(k + 1) * trec);
		Keys[k].DataRec.assign(d + desc.DataHdrDwords + (size_t)k * rec, d + desc.DataHdrDwords + (size_t)(k + 1) * rec);
	}
	if (desc.DualBank)
	{
		Bank2.resize(count2);
		for (sint32 k = 0; k < count2; ++k)
			Bank2[k].assign(d + bank2At + 1 + (size_t)k * rec, d + bank2At + 1 + (size_t)(k + 1) * rec);
	}

	m_Desc = &desc;
	return true;
}

static inline void pushDword(std::vector<uint8> &out, uint32 v)
{
	out.push_back((uint8)(v & 0xff));
	out.push_back((uint8)((v >> 8) & 0xff));
	out.push_back((uint8)((v >> 16) & 0xff));
	out.push_back((uint8)((v >> 24) & 0xff));
}

void CBipedAnimTrack::encodeData(std::vector<uint8> &out) const
{
	out.clear();
	uint32 count = (uint32)Keys.size();
	pushDword(out, count);
	if (m_Desc && m_Desc->DualBank)
	{
		for (size_t k = 0; k < Keys.size(); ++k)
			for (size_t i = 0; i < Keys[k].DataRec.size(); ++i)
				pushDword(out, Keys[k].DataRec[i]);
		pushDword(out, (uint32)Bank2.size());
		for (size_t k = 0; k < Bank2.size(); ++k)
			for (size_t i = 0; i < Bank2[k].size(); ++i)
				pushDword(out, Bank2[k][i]);
	}
	else
	{
		for (size_t i = 0; i < DataHdrExtra.size(); ++i)
			pushDword(out, DataHdrExtra[i]);
		for (size_t k = 0; k < Keys.size(); ++k)
			for (size_t i = 0; i < Keys[k].DataRec.size(); ++i)
				pushDword(out, Keys[k].DataRec[i]);
	}
}

void CBipedAnimTrack::encodeTime(std::vector<uint8> &out) const
{
	out.clear();
	pushDword(out, (uint32)Keys.size());
	for (size_t i = 0; i < TimeHdrExtra.size(); ++i)
		pushDword(out, TimeHdrExtra[i]);
	for (size_t k = 0; k < Keys.size(); ++k)
		for (size_t i = 0; i < Keys[k].TimeRec.size(); ++i)
			pushDword(out, Keys[k].TimeRec[i]);
}

void CBipedAnimTrack::resetKeys(size_t count, size_t dataRecDwords, size_t timeRecDwords)
{
	Keys.clear();
	Keys.resize(count);
	Bank2.clear();
	for (size_t k = 0; k < count; ++k)
	{
		Keys[k].DataRec.assign(dataRecDwords, 0);
		Keys[k].TimeRec.assign(timeRecDwords, 0);
		Keys[k].TimeRec[1] = fBits((float)k); // key index, stored as float
		if (timeRecDwords >= 10)
		{
			Keys[k].TimeRec[7] = fBits(25.0f); // tension (UI units, 25 = default)
			Keys[k].TimeRec[8] = fBits(25.0f); // bias
			Keys[k].TimeRec[9] = fBits(25.0f); // continuity
		}
	}
}

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
