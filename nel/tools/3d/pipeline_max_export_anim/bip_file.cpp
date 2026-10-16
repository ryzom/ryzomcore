/**
 * \file bip_file.cpp
 * \brief See bip_file.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 */

#include "bip_file.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <map>

namespace BIPANIM {
namespace {

enum ETrackSlot
{
	SlotH, SlotTurn, SlotV, SlotPelvis,
	SlotArmR, SlotArmL, SlotLegR, SlotLegL,
	SlotSpine, SlotHead, SlotTail, SlotPony1, SlotPony2,
	SlotCount
};

struct STrackSpec
{
	uint16 DataId;
	uint16 TimeId;
	int DataRecFloats; // 0 = infer from body size
	bool Vertical;
	ETrackSlot Slot;
};

// Track table — same ids as parseBipAnimKeys / CBipedAnimTrack::Descs.
static const STrackSpec SPECS[] = {
	{ 0x012c, 0x012d, 10, false, SlotH },
	{ 0x012e, 0x012f, 13, false, SlotTurn },
	{ 0x0130, 0x0131, 13, true,  SlotV },
	{ 0x0132, 0x0133, 7,  false, SlotPelvis },
	{ 0x0134, 0x0135, 110, false, SlotArmR },
	{ 0x0136, 0x0137, 110, false, SlotArmL },
	{ 0x0138, 0x0139, 110, false, SlotLegR },
	{ 0x013a, 0x013b, 110, false, SlotLegL },
	{ 0x013c, 0x013d, 0,  false, SlotSpine },
	{ 0x013e, 0x013f, 0,  false, SlotHead },
	{ 0x0142, 0x0143, 0,  false, SlotTail },
	{ 0x0147, 0x0148, 0,  false, SlotPony1 },
	{ 0x0149, 0x014a, 0,  false, SlotPony2 },
};
static const size_t NSPECS = sizeof(SPECS) / sizeof(SPECS[0]);

static SBipKeyTrack &trackSlot(SBipAnimKeys &out, ETrackSlot s)
{
	switch (s)
	{
	case SlotH: return out.Horizontal;
	case SlotTurn: return out.Turn;
	case SlotV: return out.Vertical;
	case SlotPelvis: return out.Pelvis;
	case SlotArmR: return out.ArmR;
	case SlotArmL: return out.ArmL;
	case SlotLegR: return out.LegR;
	case SlotLegL: return out.LegL;
	case SlotSpine: return out.Spine;
	case SlotHead: return out.Head;
	case SlotTail: return out.Tail;
	case SlotPony1: return out.Pony1;
	case SlotPony2: return out.Pony2;
	default: return out.Horizontal;
	}
}

static bool isTrackId(uint16 id)
{
	for (size_t i = 0; i < NSPECS; ++i)
		if (SPECS[i].DataId == id || SPECS[i].TimeId == id) return true;
	return false;
}

static const STrackSpec *specByData(uint16 id)
{
	for (size_t i = 0; i < NSPECS; ++i)
		if (SPECS[i].DataId == id) return &SPECS[i];
	return nullptr;
}

static const STrackSpec *specByTime(uint16 id)
{
	for (size_t i = 0; i < NSPECS; ++i)
		if (SPECS[i].TimeId == id) return &SPECS[i];
	return nullptr;
}

// Locate the first data-track id (0x012c) that starts a contiguous track run.
static size_t findTrackStart(const std::vector<uint8> &data)
{
	for (size_t off = 0; off + 6 <= data.size(); ++off)
	{
		uint16 id;
		memcpy(&id, &data[off], 2);
		if (id != 0x012c) continue;
		uint32 count;
		memcpy(&count, &data[off + 2], 4);
		if (count == 0 || count > 100000) continue;
		// Plausible: next few bytes look like floats / next track id later.
		return off;
	}
	return (size_t)-1;
}

// Expected body size in bytes for a track (Snowballs CS Max 3 BIP measurements).
// Time tracks: count×10 dwords + 3 trailing UI-TCB floats (25,25,25) = count*40 + 12.
// Data tracks: count × recFloats × 4 (vertical = two banks of 13 without a second count).
// Returns 0 when unknown — fall back to scan-to-next-id.
static size_t expectedBodySize(uint16 id, uint32 count)
{
	if (count == 0 || count > 100000) return 0;
	if (const STrackSpec *sd = specByData(id))
	{
		if (sd->Vertical)
			return (size_t)count * 13 * 2 * 4; // two banks, no inter-count
		if (sd->DataRecFloats > 0)
		{
			// Limb records on Snowballs measure 104 floats/key (+ optional 4-byte pad):
			// body = count*104*4 + (count==3 ? 4 : 0) for arms/legs. Prefer next-id scan
			// with a tight min, but for the LAST track (no next id) use floor formula.
			if (sd->DataRecFloats == 110)
				return (size_t)count * 104 * 4; // Max 3 era shorter limb record
			return (size_t)count * (size_t)sd->DataRecFloats * 4;
		}
		// Variable spine/head/tail/pony DATA: unknown fixed size — scan only.
		return 0;
	}
	if (specByTime(id))
		return (size_t)count * 10 * 4 + 12; // +3 trailing TCB floats
	return 0;
}

// Walk sequential (u16 id, u32 count, body…) from start.
// Prefer expectedBodySize when it lands on the next track id or EOF-minus-footer; otherwise
// scan for the next track id after minBody. Critical for the last track (0x014a): without a
// size cap it swallowed the BIP footer and produced garbage tick values → idle range blow-up.
static bool walkTracks(const std::vector<uint8> &data, size_t start,
                       std::map<uint16, std::vector<uint8> > &bodies,
                       std::map<uint16, uint32> &counts,
                       std::string &err)
{
	size_t pos = start;
	while (pos + 6 <= data.size())
	{
		uint16 id;
		uint32 count;
		memcpy(&id, &data[pos], 2);
		memcpy(&count, &data[pos + 2], 4);
		if (!isTrackId(id) || count == 0 || count > 100000)
			break;
		size_t bodyStart = pos + 6;
		size_t expect = expectedBodySize(id, count);
		size_t minBody = expect;
		if (minBody == 0)
		{
			if (const STrackSpec *sd = specByData(id))
			{
				if (sd->Vertical) minBody = (size_t)count * 13 * 4;
				else if (sd->DataRecFloats > 0)
					minBody = (size_t)count * 40; // lower bound
				else
					minBody = (size_t)count * 4;
			}
			else
				minBody = (size_t)count * 10 * 4;
		}
		if (bodyStart + minBody > data.size())
		{
			err = "truncated BIP track body";
			return false;
		}

		size_t bodyEnd = data.size();
		// If expected size lands exactly on a following track header, take it.
		if (expect > 0 && bodyStart + expect <= data.size())
		{
			size_t cand = bodyStart + expect;
			if (cand + 6 <= data.size())
			{
				uint16 nid;
				uint32 ncount;
				memcpy(&nid, &data[cand], 2);
				memcpy(&ncount, &data[cand + 2], 4);
				if (isTrackId(nid) && ncount > 0 && ncount <= 100000)
					bodyEnd = cand;
				else if (cand + 4 <= data.size() && !isTrackId(nid))
				{
					// Pad: limb bodies sometimes have +4 trailing bytes before next id.
					if (cand + 4 + 6 <= data.size())
					{
						memcpy(&nid, &data[cand + 4], 2);
						memcpy(&ncount, &data[cand + 4 + 2], 4);
						if (isTrackId(nid) && ncount > 0 && ncount <= 100000)
							bodyEnd = cand + 4;
					}
				}
				else
					bodyEnd = cand; // last track or footer — trust expected size
			}
			else
				bodyEnd = cand; // exact end of file
		}

		// Scan for next track id when expected size didn't pin a boundary.
		if (bodyEnd == data.size() && expect == 0)
		{
			for (size_t p = bodyStart + minBody; p + 6 <= data.size(); ++p)
			{
				uint16 nid;
				memcpy(&nid, &data[p], 2);
				if (!isTrackId(nid) || (p % 2) != 0) continue;
				uint32 ncount;
				memcpy(&ncount, &data[p + 2], 4);
				if (ncount == 0 || ncount > 100000) continue;
				bodyEnd = p;
				break;
			}
		}
		// Last-resort scan even when expect was set but didn't match (variable data tracks).
		if (bodyEnd == data.size() && expect > 0)
		{
			bool foundNext = false;
			for (size_t p = bodyStart + minBody; p + 6 <= data.size(); ++p)
			{
				uint16 nid;
				memcpy(&nid, &data[p], 2);
				if (!isTrackId(nid) || (p % 2) != 0) continue;
				uint32 ncount;
				memcpy(&ncount, &data[p + 2], 4);
				if (ncount == 0 || ncount > 100000) continue;
				bodyEnd = p;
				foundNext = true;
				break;
			}
			if (!foundNext && bodyStart + expect <= data.size())
				bodyEnd = bodyStart + expect; // EOF: do NOT swallow footer past expect
		}

		if (bodyEnd < bodyStart)
		{
			err = "BIP track body underflow";
			return false;
		}
		std::vector<uint8> body(data.begin() + bodyStart, data.begin() + bodyEnd);
		bodies[id] = body;
		counts[id] = count;
		pos = bodyEnd;
	}
	return !bodies.empty();
}

static void fillDataTrack(SBipKeyTrack &out, uint32 count, const std::vector<uint8> &body,
                          int recFloats, bool vertical)
{
	out.Times.clear();
	out.Recs.clear();
	out.Tens.clear();
	out.Cont.clear();
	out.Bias.clear();
	out.EaseTo.clear();
	out.EaseFrom.clear();
	if (count == 0 || body.size() < 4) return;

	const size_t nf = body.size() / 4;
	const float *f = reinterpret_cast<const float *>(&body[0]);

	size_t rec = 0;
	size_t dataOff = 0;
	if (vertical)
	{
		// BIP vertical: one count already consumed; body = bank1[count*13] + bank2[count*13]
		// (no second count dword — measured on Snowballs idle/avance).
		rec = 13;
		if (nf < count * rec) return;
		dataOff = 0;
		// Prefer first bank (same as parseTrack vertical=true).
	}
	else if (recFloats > 0)
	{
		rec = (size_t)recFloats;
		if (nf < (size_t)count) return;
		// Prefer exact division; otherwise take floor floats/key and ignore a short tail.
		size_t per = nf / count;
		if (per == 0) return;
		if (per != rec)
			rec = per; // Max 3 CS limb records can be shorter/longer than the Max 9 110-float form
	}
	else
	{
		if (count == 0 || nf < (size_t)count) return;
		rec = nf / count;
		if (rec == 0) return;
	}

	out.Recs.resize(count);
	for (uint32 k = 0; k < count; ++k)
	{
		const size_t off = dataOff + (size_t)k * rec;
		if (off + rec > nf) { out.Recs.resize(k); break; }
		out.Recs[k].assign(f + off, f + off + rec);
		// Pad short limb records to 110 so [109] ankle tension etc. are defined.
		if (recFloats == 110 && out.Recs[k].size() < 110)
			out.Recs[k].resize(110, 0.f);
	}
}

// BIP time records (Snowballs Max 3 CS): body is count × 10 dwords, Max-compatible layout
// after a possible short alignment pad:
//   [0] time ticks (sint32)
//   [1] key index
//   [2..4] unused / p*
//   [5] easeTo UI 0..50
//   [6] easeFrom UI 0..50
//   [7] tension UI (25 default)
//   [8] bias UI
//   [9] continuity UI
// Some keys store TCB-first (25,25,25,time,index,…) — detect per-key when [0] is a
// plausible UI TCB (0..50) and [3] is a plausible tick count.
static bool plausibleTicks(sint32 t)
{
	return t >= 0 && t <= 10 * 60 * 4800; // ≤ 10 minutes at 4800 ticks/s
}

static void fillTimeTrack(SBipKeyTrack &out, uint32 count, const std::vector<uint8> &body)
{
	if (count == 0 || body.size() < 4) return;
	const size_t nf = body.size() / 4;
	const float *f = reinterpret_cast<const float *>(&body[0]);
	const sint32 *i = reinterpret_cast<const sint32 *>(&body[0]);

	// Body layout: count×10 key dwords [+ optional 3 trailing TCB floats 25,25,25].
	// Always take the FIRST count×10 floats — never trailing-align (that mis-parsed keys).
	size_t per = 10;
	size_t base = 0;
	if (nf >= (size_t)count * per)
		/* ok — ignore trailing floats past count*10 */;
	else if (count > 0 && (nf % count) == 0)
		per = nf / count;
	else
		return;

	std::vector<sint32> times;
	std::vector<float> tens, cont, bias, easeTo, easeFrom;
	times.reserve(count);
	for (uint32 k = 0; k < count; ++k)
	{
		const size_t o = base + (size_t)k * per;
		if (o + per > nf) break;

		sint32 timeTicks = 0;
		float tensUI = 25.f, contUI = 25.f, biasUI = 25.f, easeToUI = 0.f, easeFromUI = 0.f;

		const sint32 i0 = i[o + 0];
		const float f0 = f[o + 0];
		const sint32 i3 = (per > 3) ? i[o + 3] : 0;
		const bool tcbFirst = (f0 >= 0.f && f0 <= 50.f) && plausibleTicks(i3)
			&& !plausibleTicks(i0);

		if (tcbFirst)
		{
			tensUI = f[o + 0];
			contUI = (per > 1) ? f[o + 1] : 25.f;
			biasUI = (per > 2) ? f[o + 2] : 25.f;
			timeTicks = i3;
			easeToUI = (per > 5) ? f[o + 5] : 0.f;
			easeFromUI = (per > 6) ? f[o + 6] : 0.f;
			if (tensUI == 0.f && contUI == 0.f && biasUI == 0.f)
				tensUI = contUI = biasUI = 25.f;
		}
		else
		{
			timeTicks = i0;
			if (!plausibleTicks(timeTicks))
			{
				// Fall back: scan the 10-slot window for a plausible tick.
				timeTicks = 0;
				for (size_t s = 0; s < per; ++s)
					if (plausibleTicks(i[o + s])) { timeTicks = i[o + s]; break; }
			}
			easeToUI = (per > 5) ? f[o + 5] : 0.f;
			easeFromUI = (per > 6) ? f[o + 6] : 0.f;
			tensUI = (per > 7) ? f[o + 7] : 25.f;
			biasUI = (per > 8) ? f[o + 8] : 25.f;
			contUI = (per > 9) ? f[o + 9] : 25.f;
			if (tensUI == 0.f && contUI == 0.f && biasUI == 0.f)
				tensUI = contUI = biasUI = 25.f;
		}

		times.push_back(timeTicks);
		tens.push_back((tensUI - 25.f) / 25.f);
		cont.push_back((contUI - 25.f) / 25.f);
		bias.push_back((biasUI - 25.f) / 25.f);
		easeTo.push_back(easeToUI / 50.f);
		easeFrom.push_back(easeFromUI / 50.f);
	}

	// Ensure non-decreasing times (corrupt BIP windows can scramble).
	for (size_t k = 1; k < times.size(); ++k)
		if (times[k] < times[k - 1])
			times[k] = times[k - 1];

	out.Times.swap(times);
	out.Tens.swap(tens);
	out.Cont.swap(cont);
	out.Bias.swap(bias);
	out.EaseTo.swap(easeTo);
	out.EaseFrom.swap(easeFrom);
	if (out.Recs.size() != out.Times.size())
	{
		// Keep data recs only when counts match; otherwise drop both (safe).
		if (out.Recs.size() > out.Times.size())
			out.Recs.resize(out.Times.size());
	}
}

static void computeRange(SBipAnimKeys &out)
{
	out.HasRange = false;
	out.RangeMin = 0;
	out.RangeMax = 0;
	const SBipKeyTrack *tracks[] = {
		&out.Horizontal, &out.Turn, &out.Vertical, &out.Pelvis,
		&out.ArmR, &out.ArmL, &out.LegR, &out.LegL,
		&out.Spine, &out.Head, &out.Tail, &out.Pony1, &out.Pony2
	};
	for (size_t t = 0; t < sizeof(tracks) / sizeof(tracks[0]); ++t)
	{
		const SBipKeyTrack &tr = *tracks[t];
		// Skip tracks with mismatched time/rec counts (would crash the evaluator).
		if (tr.Times.empty() || tr.Times.size() != tr.Recs.size()) continue;
		if (tr.Tens.size() != tr.Times.size()) continue;
		sint32 a = tr.Times.front();
		sint32 b = tr.Times.back();
		if (!plausibleTicks(a) || !plausibleTicks(b)) continue;
		if (!out.HasRange) { out.HasRange = true; out.RangeMin = a; out.RangeMax = b; }
		else
		{
			if (a < out.RangeMin) out.RangeMin = a;
			if (b > out.RangeMax) out.RangeMax = b;
		}
	}
	// Hard clamp insane ranges (defensive).
	if (out.HasRange && (out.RangeMax - out.RangeMin) > 60 * 4800)
	{
		out.RangeMax = out.RangeMin + 60 * 4800;
	}
}

} /* anonymous namespace */

bool loadBipFile(const std::string &path, SBipAnimKeys &out, std::string &err)
{
	out = SBipAnimKeys();
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp) { err = "cannot open BIP file"; return false; }
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (sz < 64) { fclose(fp); err = "BIP file too small"; return false; }
	std::vector<uint8> data((size_t)sz);
	if (fread(&data[0], 1, (size_t)sz, fp) != (size_t)sz)
	{ fclose(fp); err = "BIP short read"; return false; }
	fclose(fp);

	size_t start = findTrackStart(data);
	if (start == (size_t)-1) { err = "no BIP keytrack run (0x012c) found"; return false; }

	std::map<uint16, std::vector<uint8> > bodies;
	std::map<uint16, uint32> counts;
	if (!walkTracks(data, start, bodies, counts, err))
	{
		if (err.empty()) err = "no BIP tracks decoded";
		return false;
	}

	for (size_t i = 0; i < NSPECS; ++i)
	{
		const STrackSpec &sp = SPECS[i];
		SBipKeyTrack &tr = trackSlot(out, sp.Slot);
		std::map<uint16, std::vector<uint8> >::iterator di = bodies.find(sp.DataId);
		std::map<uint16, std::vector<uint8> >::iterator ti = bodies.find(sp.TimeId);
		if (di == bodies.end() || ti == bodies.end()) continue;
		uint32 count = counts[sp.DataId];
		if (counts[sp.TimeId] != count) continue;
		fillDataTrack(tr, count, di->second, sp.DataRecFloats, sp.Vertical);
		fillTimeTrack(tr, count, ti->second);
		// Require matching key counts; otherwise drop the track (evaluator assumes parity).
		if (tr.Times.empty() || tr.Recs.empty() || tr.Times.size() != tr.Recs.size()
			|| tr.Tens.size() != tr.Times.size())
		{
			tr = SBipKeyTrack();
			continue;
		}
	}

	computeRange(out);
	if (!out.HasRange) { err = "BIP produced no keyed range"; return false; }
	return true;
}

} /* namespace BIPANIM */
