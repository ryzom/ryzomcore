/**
 * \file storage_ole.cpp
 * \brief CStorageOleIn / CStorageOleOut
 * \date 2026-07-07
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 * OLE2 / MS-CFB container access for .max files.
 *
 * Two backends, selected at compile time:
 *   NL_PIPELINE_NATIVE_OLE defined  -> self-contained native CFB reader/writer (default).
 *   NL_PIPELINE_NATIVE_OLE undefined -> libgsf-backed (kept for cross-validation).
 *
 * The MS-CFB (Compound File Binary Format, a.k.a. OLE2 structured storage) layout implemented here
 * follows [MS-CFB]. A .max file is a flat container: a root storage with the scene streams as
 * direct children and the root-storage CLSID carrying the .max "class id". The native reader
 * accepts both v3 (512-byte sectors) and v4 (4096-byte sectors); the writer emits v4 (matching the
 * whole modern corpus) with a deterministic sector layout.
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
#include "storage_ole.h"

// STL includes
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>

// NeL includes
#include <nel/misc/debug.h>

namespace PIPELINE {
namespace MAX {

namespace {

// The 8-byte compound file signature.
static const uint8 s_CfbSignature[8] = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };

// Special sector / stream-id values ([MS-CFB] 2.1).
static const uint32 CFB_FREESECT   = 0xFFFFFFFFu;
static const uint32 CFB_ENDOFCHAIN = 0xFFFFFFFEu;
static const uint32 CFB_FATSECT    = 0xFFFFFFFDu;
static const uint32 CFB_DIFSECT    = 0xFFFFFFFCu;
static const uint32 CFB_MAXREGSECT = 0xFFFFFFFAu;
static const uint32 CFB_NOSTREAM   = 0xFFFFFFFFu;

// Explicit little-endian access so the format is interpreted identically on any host (the whole
// point of rolling our own — no reliance on host byte order).
static inline uint16 rdU16(const uint8 *p) { return (uint16)((uint16)p[0] | ((uint16)p[1] << 8)); }
static inline uint32 rdU32(const uint8 *p) { return (uint32)p[0] | ((uint32)p[1] << 8) | ((uint32)p[2] << 16) | ((uint32)p[3] << 24); }
static inline uint64 rdU64(const uint8 *p) { return (uint64)rdU32(p) | ((uint64)rdU32(p + 4) << 32); }
static inline void wrU16(uint8 *p, uint16 v) { p[0] = (uint8)v; p[1] = (uint8)(v >> 8); }
static inline void wrU32(uint8 *p, uint32 v) { p[0] = (uint8)v; p[1] = (uint8)(v >> 8); p[2] = (uint8)(v >> 16); p[3] = (uint8)(v >> 24); }
static inline void wrU64(uint8 *p, uint64 v) { wrU32(p, (uint32)v); wrU32(p + 4, (uint32)(v >> 32)); }

// Read the whole file into memory. Returns false on any error.
static bool readWholeFile(const std::string &path, std::vector<uint8> &out)
{
	FILE *f = fopen(path.c_str(), "rb");
	if (!f) return false;
	if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
	long sz = ftell(f);
	if (sz < 0) { fclose(f); return false; }
	if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return false; }
	out.resize((size_t)sz);
	bool ok = sz == 0 || fread(&out[0], 1, (size_t)sz, f) == (size_t)sz;
	fclose(f);
	return ok;
}

static bool writeWholeFile(const std::string &path, const std::vector<uint8> &data)
{
	FILE *f = fopen(path.c_str(), "wb");
	if (!f) return false;
	bool ok = data.empty() || fwrite(&data[0], 1, data.size(), f) == data.size();
	if (fclose(f) != 0) ok = false;
	return ok;
}

} /* anonymous namespace */

// ==============================================================================================
#ifdef NL_PIPELINE_NATIVE_OLE
// ==============================================================================================
// Native MS-CFB implementation.

namespace {

struct SCfbDirEntry
{
	std::string Name;   // decoded from UTF-16LE (streams here are ASCII, incl. a \05 control prefix)
	uint8 Type;         // 0 unallocated, 1 storage, 2 stream, 5 root storage
	uint32 Left, Right, Child;
	uint8 Clsid[16];
	uint32 StartSector; // big-sector chain start, or mini-sector chain start when size < cutoff
	uint64 Size;
};

// -------------------------------------------------------------------------------------------
// Reader

struct CCfbReader
{
	std::vector<uint8> File;
	uint32 SectorSize;
	uint32 MiniSectorSize;
	uint32 MiniCutoff;
	std::vector<uint32> Fat;
	std::vector<uint32> MiniFat;
	std::vector<SCfbDirEntry> Dir;
	std::vector<uint8> MiniStream;
	uint8 RootClsid[16];
	bool HaveClsid;

	std::vector<std::string> Names;         // root-level stream names, directory order
	std::map<std::string, size_t> NameIdx;  // name -> dir index

	CCfbReader() : SectorSize(0), MiniSectorSize(0), MiniCutoff(0), HaveClsid(false)
	{
		memset(RootClsid, 0, 16);
	}

	inline uint64 sectorOffset(uint32 s) const { return (uint64)SectorSize * ((uint64)s + 1); }

	bool parse();
	bool readBigChain(uint32 start, uint64 size, std::vector<uint8> &out) const;
	bool readMiniChain(uint32 start, uint64 size, std::vector<uint8> &out) const;
	bool readEntry(size_t dirIndex, std::vector<uint8> &out) const;
	void collectRootStreams(uint32 nodeId);
};

bool CCfbReader::parse()
{
	if (File.size() < 512) return false;
	const uint8 *h = &File[0];
	if (memcmp(h, s_CfbSignature, 8) != 0) return false;
	if (rdU16(h + 28) != 0xFFFE) return false; // byte order marker (little-endian)

	uint16 sectorShift = rdU16(h + 30);
	if (sectorShift != 9 && sectorShift != 12) return false; // v3 (512) / v4 (4096) only
	SectorSize = 1u << sectorShift;
	uint16 miniShift = rdU16(h + 32);
	if (miniShift == 0 || miniShift > 20) return false;
	MiniSectorSize = 1u << miniShift;
	MiniCutoff = rdU32(h + 56);
	if (MiniCutoff == 0) MiniCutoff = 4096;

	uint32 nFatSectors = rdU32(h + 44);
	uint32 firstDirSector = rdU32(h + 48);
	uint32 firstMiniFat = rdU32(h + 60);
	uint32 nMiniFatSectors = rdU32(h + 64);
	uint32 firstDifat = rdU32(h + 68);
	uint32 nDifatSectors = rdU32(h + 72);

	uint32 entriesPerSector = SectorSize / 4;
	uint64 totalSectorsInFile = File.size() >= SectorSize ? (File.size() - SectorSize) / SectorSize : 0;

	// 1. Gather FAT sector numbers from the DIFAT (109 in the header, then the DIFAT sector chain).
	std::vector<uint32> fatSectors;
	fatSectors.reserve(nFatSectors);
	for (uint32 i = 0; i < 109 && fatSectors.size() < nFatSectors; ++i)
	{
		uint32 v = rdU32(h + 76 + i * 4);
		if (v <= CFB_MAXREGSECT) fatSectors.push_back(v);
	}
	{
		uint32 sec = firstDifat;
		uint32 guard = nDifatSectors + 1;
		while (sec != CFB_ENDOFCHAIN && sec <= CFB_MAXREGSECT && guard-- && fatSectors.size() < nFatSectors)
		{
			uint64 off = sectorOffset(sec);
			if (off + SectorSize > File.size()) return false;
			const uint8 *p = &File[off];
			for (uint32 i = 0; i + 1 < entriesPerSector && fatSectors.size() < nFatSectors; ++i)
			{
				uint32 v = rdU32(p + i * 4);
				if (v <= CFB_MAXREGSECT) fatSectors.push_back(v);
			}
			sec = rdU32(p + (entriesPerSector - 1) * 4);
		}
	}

	// 2. Read the FAT itself.
	Fat.clear();
	Fat.reserve((size_t)fatSectors.size() * entriesPerSector);
	for (size_t i = 0; i < fatSectors.size(); ++i)
	{
		uint32 sec = fatSectors[i];
		uint64 off = sectorOffset(sec);
		if (off + SectorSize > File.size()) return false;
		const uint8 *p = &File[off];
		for (uint32 j = 0; j < entriesPerSector; ++j)
			Fat.push_back(rdU32(p + j * 4));
	}
	if (Fat.empty()) return false;

	// 3. Read the directory chain and parse entries.
	std::vector<uint8> dirBytes;
	{
		uint32 sec = firstDirSector;
		uint64 guard = totalSectorsInFile + 1;
		while (sec != CFB_ENDOFCHAIN && sec <= CFB_MAXREGSECT && guard--)
		{
			uint64 off = sectorOffset(sec);
			if (off + SectorSize > File.size()) return false;
			dirBytes.insert(dirBytes.end(), File.begin() + off, File.begin() + off + SectorSize);
			if (sec >= Fat.size()) return false;
			sec = Fat[sec];
		}
	}
	size_t nEntries = dirBytes.size() / 128;
	if (nEntries == 0) return false;
	Dir.resize(nEntries);
	sint rootIndex = -1;
	for (size_t i = 0; i < nEntries; ++i)
	{
		const uint8 *e = &dirBytes[i * 128];
		SCfbDirEntry &d = Dir[i];
		uint16 nameLen = rdU16(e + 64); // bytes, including the UTF-16 null terminator
		uint16 nChars = nameLen >= 2 ? (uint16)(nameLen / 2 - 1) : 0;
		if (nChars > 31) nChars = 31;
		d.Name.resize(nChars);
		for (uint16 c = 0; c < nChars; ++c)
			d.Name[c] = (char)(uint8)rdU16(e + c * 2); // low byte (ASCII / control)
		d.Type = e[66];
		d.Left = rdU32(e + 68);
		d.Right = rdU32(e + 72);
		d.Child = rdU32(e + 76);
		memcpy(d.Clsid, e + 80, 16);
		d.StartSector = rdU32(e + 116);
		d.Size = rdU64(e + 120);
		if (SectorSize == 512) d.Size &= 0xFFFFFFFFu; // v3: high dword reserved
		if (d.Type == 5 && rootIndex < 0) rootIndex = (sint)i;
	}
	if (rootIndex < 0) return false;
	SCfbDirEntry &root = Dir[rootIndex];
	memcpy(RootClsid, root.Clsid, 16);
	HaveClsid = true;

	// 4. Read the mini-stream (the root storage's own stream, held in big sectors) and the mini FAT.
	if (root.Size > 0)
	{
		if (!readBigChain(root.StartSector, root.Size, MiniStream)) return false;
	}
	if (nMiniFatSectors > 0)
	{
		std::vector<uint8> mfBytes;
		uint32 sec = firstMiniFat;
		uint64 guard = totalSectorsInFile + 1;
		while (sec != CFB_ENDOFCHAIN && sec <= CFB_MAXREGSECT && guard--)
		{
			uint64 off = sectorOffset(sec);
			if (off + SectorSize > File.size()) return false;
			mfBytes.insert(mfBytes.end(), File.begin() + off, File.begin() + off + SectorSize);
			if (sec >= Fat.size()) return false;
			sec = Fat[sec];
		}
		MiniFat.resize(mfBytes.size() / 4);
		for (size_t i = 0; i < MiniFat.size(); ++i)
			MiniFat[i] = rdU32(&mfBytes[i * 4]);
	}

	// 5. Walk the root's child tree to collect the root-level streams in directory order.
	if (root.Child != CFB_NOSTREAM)
		collectRootStreams(root.Child);
	return true;
}

void CCfbReader::collectRootStreams(uint32 nodeId)
{
	// In-order traversal with a visited guard against malformed cyclic trees.
	std::set<uint32> visited;
	// Iterative to avoid deep recursion on pathological files.
	std::vector<uint32> stack;
	uint32 cur = nodeId;
	while (cur != CFB_NOSTREAM || !stack.empty())
	{
		while (cur != CFB_NOSTREAM)
		{
			if (cur >= Dir.size() || !visited.insert(cur).second) { cur = CFB_NOSTREAM; break; }
			stack.push_back(cur);
			cur = Dir[cur].Left;
		}
		if (stack.empty()) break;
		uint32 n = stack.back();
		stack.pop_back();
		const SCfbDirEntry &d = Dir[n];
		if (d.Type == 2) // stream
		{
			if (NameIdx.find(d.Name) == NameIdx.end())
			{
				NameIdx[d.Name] = n;
				Names.push_back(d.Name);
			}
		}
		cur = d.Right;
	}
}

bool CCfbReader::readBigChain(uint32 start, uint64 size, std::vector<uint8> &out) const
{
	out.clear();
	out.reserve((size_t)size);
	uint32 sec = start;
	uint64 remaining = size;
	uint64 guard = Fat.size() + 1;
	while (remaining > 0 && sec != CFB_ENDOFCHAIN && sec <= CFB_MAXREGSECT && guard--)
	{
		uint64 off = sectorOffset(sec);
		if (off + SectorSize > File.size()) return false;
		uint64 n = std::min<uint64>(remaining, SectorSize);
		out.insert(out.end(), File.begin() + off, File.begin() + off + n);
		remaining -= n;
		if (sec >= Fat.size()) return false;
		sec = Fat[sec];
	}
	return remaining == 0;
}

bool CCfbReader::readMiniChain(uint32 start, uint64 size, std::vector<uint8> &out) const
{
	out.clear();
	out.reserve((size_t)size);
	uint32 sec = start;
	uint64 remaining = size;
	uint64 guard = MiniFat.size() + 1;
	while (remaining > 0 && sec != CFB_ENDOFCHAIN && sec <= CFB_MAXREGSECT && guard--)
	{
		uint64 off = (uint64)sec * MiniSectorSize;
		if (off >= MiniStream.size()) return false;
		uint64 avail = MiniStream.size() - off;
		uint64 n = std::min<uint64>(remaining, std::min<uint64>(MiniSectorSize, avail));
		out.insert(out.end(), MiniStream.begin() + off, MiniStream.begin() + off + n);
		remaining -= n;
		if (sec >= MiniFat.size()) return false;
		sec = MiniFat[sec];
	}
	return remaining == 0;
}

bool CCfbReader::readEntry(size_t dirIndex, std::vector<uint8> &out) const
{
	const SCfbDirEntry &d = Dir[dirIndex];
	if (d.Size == 0) { out.clear(); return true; }
	if (d.Size < MiniCutoff)
		return readMiniChain(d.StartSector, d.Size, out);
	return readBigChain(d.StartSector, d.Size, out);
}

// -------------------------------------------------------------------------------------------
// Writer (v4, 4096-byte sectors, deterministic layout)

// CFB directory-tree key order: shorter names first, then case-insensitive by UTF-16 code unit.
static int cfbNameCompare(const std::string &a, const std::string &b)
{
	if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
	for (size_t i = 0; i < a.size(); ++i)
	{
		uint16 ca = (uint8)a[i], cb = (uint8)b[i];
		if (ca >= 'a' && ca <= 'z') ca = (uint16)(ca - 32);
		if (cb >= 'a' && cb <= 'z') cb = (uint16)(cb - 32);
		if (ca != cb) return ca < cb ? -1 : 1;
	}
	return 0;
}

// Sort comparator for the directory tree (functor, not a lambda, so the file compiles under
// VS2008 / MSVC 9.0 as well as the modern toolchain).
struct CCfbDirNameLess
{
	const std::vector<SCfbDirEntry> *Dir;
	CCfbDirNameLess(const std::vector<SCfbDirEntry> *dir) : Dir(dir) { }
	bool operator()(uint32 a, uint32 b) const { return cfbNameCompare((*Dir)[a].Name, (*Dir)[b].Name) < 0; }
};

struct CCfbWriter
{
	const std::vector<std::pair<std::string, std::vector<uint8> > > *Streams;
	const uint8 *Clsid;

	std::vector<SCfbDirEntry> Dir; // [0] = root, then one per stream (by add index + 1)

	// Build the balanced directory BST over the sorted stream dir-ids; returns the subtree root id.
	uint32 buildTree(const std::vector<uint32> &sortedIds, sint lo, sint hi)
	{
		if (lo > hi) return CFB_NOSTREAM;
		sint mid = (lo + hi) / 2;
		uint32 id = sortedIds[(size_t)mid];
		Dir[id].Left = buildTree(sortedIds, lo, mid - 1);
		Dir[id].Right = buildTree(sortedIds, mid + 1, hi);
		return id;
	}

	bool assemble(std::vector<uint8> &fileOut);
};

bool CCfbWriter::assemble(std::vector<uint8> &fileOut)
{
	const uint32 SS = 4096;
	const uint32 MSS = 64;
	const uint32 MINI_CUTOFF = 4096;
	const uint32 FAT_PER_SEC = SS / 4;   // 1024
	const uint32 DIR_PER_SEC = SS / 128; // 32

	const std::vector<std::pair<std::string, std::vector<uint8> > > &streams = *Streams;
	size_t N = streams.size();

	// Per-stream placement decision.
	enum EPlace { PL_EMPTY, PL_MINI, PL_BIG };
	std::vector<EPlace> place(N);
	std::vector<uint32> streamStart(N, CFB_ENDOFCHAIN); // mini-sector index (mini) or absolute big sector

	// --- mini stream + mini FAT -------------------------------------------------------------
	std::vector<uint8> miniStreamData;
	std::vector<uint32> miniFat;
	for (size_t i = 0; i < N; ++i)
	{
		uint64 sz = streams[i].second.size();
		if (sz == 0) { place[i] = PL_EMPTY; continue; }
		if (sz >= MINI_CUTOFF) { place[i] = PL_BIG; continue; }
		place[i] = PL_MINI;
		uint32 startMini = (uint32)(miniStreamData.size() / MSS);
		streamStart[i] = startMini;
		uint32 nsec = (uint32)((sz + MSS - 1) / MSS);
		size_t base = miniStreamData.size();
		miniStreamData.resize(base + (size_t)nsec * MSS, 0);
		memcpy(&miniStreamData[base], &streams[i].second[0], (size_t)sz);
		for (uint32 k = 0; k < nsec; ++k)
			miniFat.push_back(k + 1 < nsec ? startMini + k + 1 : CFB_ENDOFCHAIN);
	}
	uint64 miniStreamRealSize = miniStreamData.size(); // already 64-aligned
	// Pad mini FAT to whole sectors.
	uint32 nMiniFatSectors = (uint32)((miniFat.size() + FAT_PER_SEC - 1) / FAT_PER_SEC);
	miniFat.resize((size_t)nMiniFatSectors * FAT_PER_SEC, CFB_FREESECT);
	// Pad mini stream container to whole big sectors.
	uint32 nMiniContainerSectors = (uint32)((miniStreamData.size() + SS - 1) / SS);
	miniStreamData.resize((size_t)nMiniContainerSectors * SS, 0);

	// --- directory entry count --------------------------------------------------------------
	size_t nDirEntries = N + 1;
	size_t nDirEntriesPadded = ((nDirEntries + DIR_PER_SEC - 1) / DIR_PER_SEC) * DIR_PER_SEC;
	uint32 nDirSectors = (uint32)(nDirEntriesPadded / DIR_PER_SEC);

	// --- region sector assignment -----------------------------------------------------------
	uint32 cursor = 0;
	std::vector<uint32> bigStart(N, 0);
	std::vector<uint32> bigCount(N, 0);
	for (size_t i = 0; i < N; ++i)
	{
		if (place[i] != PL_BIG) continue;
		uint32 cnt = (uint32)((streams[i].second.size() + SS - 1) / SS);
		bigStart[i] = cursor;
		bigCount[i] = cnt;
		streamStart[i] = cursor;
		cursor += cnt;
	}
	uint32 miniContainerStart = cursor; cursor += nMiniContainerSectors;
	uint32 miniFatStart = cursor;       cursor += nMiniFatSectors;
	uint32 dirStart = cursor;           cursor += nDirSectors;
	uint32 base = cursor; // everything before FAT/DIFAT

	// FAT + DIFAT sector count fixpoint.
	uint32 nFat = 0, nDifat = 0;
	for (;;)
	{
		uint64 total = (uint64)base + nFat + nDifat;
		uint32 newFat = (uint32)((total + FAT_PER_SEC - 1) / FAT_PER_SEC);
		uint32 newDifat = newFat <= 109 ? 0 : (uint32)(((uint64)newFat - 109 + (FAT_PER_SEC - 1) - 1) / (FAT_PER_SEC - 1));
		if (newFat == nFat && newDifat == nDifat) break;
		nFat = newFat; nDifat = newDifat;
	}
	uint32 fatStart = cursor;   cursor += nFat;
	uint32 difatStart = cursor; cursor += nDifat;
	uint32 totalSectors = cursor;

	// --- build the FAT ----------------------------------------------------------------------
	std::vector<uint32> fat((size_t)nFat * FAT_PER_SEC, CFB_FREESECT);
	// chainRun: mark a contiguous run of sectors as one chain.
	// (defined inline below via a small lambda-free loop helper)
	#define CFB_CHAIN_RUN(startSec, count) do { \
		uint32 _s = (startSec), _c = (count); \
		for (uint32 _k = 0; _k < _c; ++_k) fat[_s + _k] = (_k + 1 < _c) ? (_s + _k + 1) : CFB_ENDOFCHAIN; \
	} while (0)
	for (size_t i = 0; i < N; ++i)
		if (place[i] == PL_BIG) CFB_CHAIN_RUN(bigStart[i], bigCount[i]);
	if (nMiniContainerSectors) CFB_CHAIN_RUN(miniContainerStart, nMiniContainerSectors);
	if (nMiniFatSectors) CFB_CHAIN_RUN(miniFatStart, nMiniFatSectors);
	if (nDirSectors) CFB_CHAIN_RUN(dirStart, nDirSectors);
	#undef CFB_CHAIN_RUN
	for (uint32 k = 0; k < nFat; ++k) fat[fatStart + k] = CFB_FATSECT;
	for (uint32 k = 0; k < nDifat; ++k) fat[difatStart + k] = CFB_DIFSECT;

	// --- directory entries ------------------------------------------------------------------
	Dir.assign(N + 1, SCfbDirEntry());
	// Root.
	{
		SCfbDirEntry &r = Dir[0];
		r.Name = "Root Entry";
		r.Type = 5;
		r.Left = CFB_NOSTREAM; r.Right = CFB_NOSTREAM; r.Child = CFB_NOSTREAM;
		if (Clsid) memcpy(r.Clsid, Clsid, 16); else memset(r.Clsid, 0, 16);
		r.StartSector = nMiniContainerSectors ? miniContainerStart : CFB_ENDOFCHAIN;
		r.Size = miniStreamRealSize;
	}
	for (size_t i = 0; i < N; ++i)
	{
		SCfbDirEntry &d = Dir[i + 1];
		d.Name = streams[i].first;
		d.Type = 2;
		d.Left = CFB_NOSTREAM; d.Right = CFB_NOSTREAM; d.Child = CFB_NOSTREAM;
		memset(d.Clsid, 0, 16);
		d.StartSector = streamStart[i];
		d.Size = streams[i].second.size();
	}
	// Sorted balanced tree over the stream dir-ids.
	if (N)
	{
		std::vector<uint32> ids(N);
		for (size_t i = 0; i < N; ++i) ids[i] = (uint32)(i + 1);
		std::sort(ids.begin(), ids.end(), CCfbDirNameLess(&Dir));
		Dir[0].Child = buildTree(ids, 0, (sint)N - 1);
	}

	// Serialize directory to bytes.
	std::vector<uint8> dirBytes(nDirEntriesPadded * 128, 0);
	for (size_t i = 0; i < nDirEntriesPadded; ++i)
	{
		uint8 *e = &dirBytes[i * 128];
		if (i < Dir.size())
		{
			const SCfbDirEntry &d = Dir[i];
			size_t nChars = std::min<size_t>(d.Name.size(), 31);
			for (size_t c = 0; c < nChars; ++c)
				wrU16(e + c * 2, (uint16)(uint8)d.Name[c]);
			wrU16(e + 64, (uint16)((nChars + 1) * 2)); // name length in bytes incl. null
			e[66] = d.Type;
			e[67] = 1; // color: black
			wrU32(e + 68, d.Left);
			wrU32(e + 72, d.Right);
			wrU32(e + 76, d.Child);
			memcpy(e + 80, d.Clsid, 16);
			wrU32(e + 116, d.StartSector);
			wrU64(e + 120, d.Size);
		}
		else
		{
			// Unused directory entry: type 0, all-NOSTREAM links.
			wrU32(e + 68, CFB_NOSTREAM);
			wrU32(e + 72, CFB_NOSTREAM);
			wrU32(e + 76, CFB_NOSTREAM);
		}
	}

	// --- DIFAT ------------------------------------------------------------------------------
	uint32 headerDifat[109];
	for (uint32 i = 0; i < 109; ++i) headerDifat[i] = CFB_FREESECT;
	std::vector<uint8> difatBytes((size_t)nDifat * SS, 0);
	{
		uint32 written = 0;
		for (; written < nFat && written < 109; ++written)
			headerDifat[written] = fatStart + written;
		// Remaining FAT sector numbers into DIFAT sectors.
		for (uint32 ds = 0; ds < nDifat; ++ds)
		{
			uint8 *p = &difatBytes[(size_t)ds * SS];
			for (uint32 j = 0; j + 1 < FAT_PER_SEC; ++j)
			{
				uint32 v = written < nFat ? (fatStart + written) : CFB_FREESECT;
				wrU32(p + j * 4, v);
				if (written < nFat) ++written;
			}
			uint32 next = (ds + 1 < nDifat) ? (difatStart + ds + 1) : CFB_ENDOFCHAIN;
			wrU32(p + (FAT_PER_SEC - 1) * 4, next);
		}
	}

	// --- header -----------------------------------------------------------------------------
	std::vector<uint8> header(SS, 0);
	{
		uint8 *h = &header[0];
		memcpy(h, s_CfbSignature, 8);
		// header CLSID (offset 8..23) stays zero; the real class id lives in the root dir entry.
		wrU16(h + 24, 0x003E); // minor version
		wrU16(h + 26, 4);      // major version (v4 = 4096-byte sectors)
		wrU16(h + 28, 0xFFFE); // byte order
		wrU16(h + 30, 12);     // sector shift (2^12 = 4096)
		wrU16(h + 32, 6);      // mini sector shift (2^6 = 64)
		// 34..39 reserved = 0
		wrU32(h + 40, nDirSectors); // number of directory sectors (v4)
		wrU32(h + 44, nFat);
		wrU32(h + 48, dirStart);
		wrU32(h + 52, 0); // transaction signature
		wrU32(h + 56, MINI_CUTOFF);
		wrU32(h + 60, nMiniFatSectors ? miniFatStart : CFB_ENDOFCHAIN);
		wrU32(h + 64, nMiniFatSectors);
		wrU32(h + 68, nDifat ? difatStart : CFB_ENDOFCHAIN);
		wrU32(h + 72, nDifat);
		for (uint32 i = 0; i < 109; ++i) wrU32(h + 76 + i * 4, headerDifat[i]);
	}

	// --- assemble ---------------------------------------------------------------------------
	fileOut.clear();
	fileOut.reserve((size_t)(totalSectors + 1) * SS);
	fileOut.insert(fileOut.end(), header.begin(), header.end());
	// Big stream data (each padded to a whole sector), in add order == sector order.
	for (size_t i = 0; i < N; ++i)
	{
		if (place[i] != PL_BIG) continue;
		const std::vector<uint8> &b = streams[i].second;
		size_t padded = (size_t)bigCount[i] * SS;
		size_t at = fileOut.size();
		fileOut.resize(at + padded, 0);
		memcpy(&fileOut[at], &b[0], b.size());
	}
	// Mini container.
	fileOut.insert(fileOut.end(), miniStreamData.begin(), miniStreamData.end());
	// Mini FAT.
	if (nMiniFatSectors)
	{
		size_t at = fileOut.size();
		fileOut.resize(at + (size_t)nMiniFatSectors * SS, 0);
		for (size_t i = 0; i < miniFat.size(); ++i) wrU32(&fileOut[at + i * 4], miniFat[i]);
	}
	// Directory.
	fileOut.insert(fileOut.end(), dirBytes.begin(), dirBytes.end());
	// FAT.
	{
		size_t at = fileOut.size();
		fileOut.resize(at + (size_t)nFat * SS, 0);
		for (size_t i = 0; i < fat.size(); ++i) wrU32(&fileOut[at + i * 4], fat[i]);
	}
	// DIFAT.
	fileOut.insert(fileOut.end(), difatBytes.begin(), difatBytes.end());

	nlassert(fileOut.size() == (size_t)(totalSectors + 1) * SS);
	return true;
}

} /* anonymous namespace */

// ---- CStorageOleIn (native) ------------------------------------------------------------------

CStorageOleIn::CStorageOleIn() : m_Impl(nullptr) { }
CStorageOleIn::~CStorageOleIn() { close(); }

bool CStorageOleIn::open(const std::string &path)
{
	close();
	CCfbReader *r = new CCfbReader();
	if (!readWholeFile(path, r->File) || !r->parse())
	{
		delete r;
		return false;
	}
	m_Impl = r;
	return true;
}

bool CStorageOleIn::isOpen() const { return m_Impl != nullptr; }

void CStorageOleIn::close()
{
	delete static_cast<CCfbReader *>(m_Impl);
	m_Impl = nullptr;
}

bool CStorageOleIn::getClassId(uint8 classId[16]) const
{
	if (!m_Impl) return false;
	const CCfbReader *r = static_cast<const CCfbReader *>(m_Impl);
	if (!r->HaveClsid) return false;
	memcpy(classId, r->RootClsid, 16);
	return true;
}

bool CStorageOleIn::hasStream(const std::string &name) const
{
	if (!m_Impl) return false;
	const CCfbReader *r = static_cast<const CCfbReader *>(m_Impl);
	return r->NameIdx.find(name) != r->NameIdx.end();
}

bool CStorageOleIn::readStream(const std::string &name, std::vector<uint8> &out) const
{
	out.clear();
	if (!m_Impl) return false;
	const CCfbReader *r = static_cast<const CCfbReader *>(m_Impl);
	std::map<std::string, size_t>::const_iterator it = r->NameIdx.find(name);
	if (it == r->NameIdx.end()) return false;
	return r->readEntry(it->second, out);
}

const std::vector<std::string> &CStorageOleIn::streamNames() const
{
	static const std::vector<std::string> s_empty;
	if (!m_Impl) return s_empty;
	return static_cast<const CCfbReader *>(m_Impl)->Names;
}

// ---- CStorageOleOut (native) -----------------------------------------------------------------

namespace {
struct CNativeOut
{
	std::vector<std::pair<std::string, std::vector<uint8> > > Streams;
	uint8 Clsid[16];
	bool HaveClsid;
	CNativeOut() : HaveClsid(false) { memset(Clsid, 0, 16); }
};
} /* anonymous namespace */

CStorageOleOut::CStorageOleOut() : m_Impl(new CNativeOut()) { }
CStorageOleOut::~CStorageOleOut() { delete static_cast<CNativeOut *>(m_Impl); }

void CStorageOleOut::setClassId(const uint8 classId[16])
{
	CNativeOut *o = static_cast<CNativeOut *>(m_Impl);
	memcpy(o->Clsid, classId, 16);
	o->HaveClsid = true;
}

void CStorageOleOut::addStream(const std::string &name, const std::vector<uint8> &data)
{
	CNativeOut *o = static_cast<CNativeOut *>(m_Impl);
	o->Streams.push_back(std::make_pair(name, data));
}

void CStorageOleOut::addStreamSwap(const std::string &name, std::vector<uint8> &data)
{
	CNativeOut *o = static_cast<CNativeOut *>(m_Impl);
	o->Streams.push_back(std::pair<std::string, std::vector<uint8> >(name, std::vector<uint8>()));
	o->Streams.back().second.swap(data);
}

bool CStorageOleOut::write(const std::string &path)
{
	CNativeOut *o = static_cast<CNativeOut *>(m_Impl);
	CCfbWriter w;
	w.Streams = &o->Streams;
	w.Clsid = o->HaveClsid ? o->Clsid : nullptr;
	std::vector<uint8> bytes;
	if (!w.assemble(bytes)) return false;
	return writeWholeFile(path, bytes);
}

// ==============================================================================================
#else // NL_PIPELINE_NATIVE_OLE
// ==============================================================================================
// libgsf-backed implementation (cross-validation backend).

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-outfile-msole.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-utils.h>

namespace {

static bool s_GsfInit = false;
static void ensureGsfInit()
{
	if (!s_GsfInit) { gsf_init(); s_GsfInit = true; }
}

struct CGsfIn
{
	GsfInput *Src;
	GsfInfile *Infile;
	std::vector<std::string> Names;
	CGsfIn() : Src(NULL), Infile(NULL) { }
	~CGsfIn()
	{
		if (Infile) g_object_unref(Infile);
		if (Src) g_object_unref(Src);
	}
};

} /* anonymous namespace */

CStorageOleIn::CStorageOleIn() : m_Impl(NULL) { }
CStorageOleIn::~CStorageOleIn() { close(); }

bool CStorageOleIn::open(const std::string &path)
{
	close();
	ensureGsfInit();
	CGsfIn *g = new CGsfIn();
	g->Src = gsf_input_stdio_new(path.c_str(), NULL);
	if (!g->Src) { delete g; return false; }
	g->Infile = gsf_infile_msole_new(g->Src, NULL);
	if (!g->Infile) { delete g; return false; }
	int n = gsf_infile_num_children(g->Infile);
	for (int i = 0; i < n; ++i)
	{
		const char *nm = gsf_infile_name_by_index(g->Infile, i);
		if (nm) g->Names.push_back(nm);
	}
	m_Impl = g;
	return true;
}

bool CStorageOleIn::isOpen() const { return m_Impl != NULL; }

void CStorageOleIn::close()
{
	delete static_cast<CGsfIn *>(m_Impl);
	m_Impl = NULL;
}

bool CStorageOleIn::getClassId(uint8 classId[16]) const
{
	if (!m_Impl) return false;
	CGsfIn *g = static_cast<CGsfIn *>(m_Impl);
	return gsf_infile_msole_get_class_id((GsfInfileMSOle *)g->Infile, classId) != FALSE;
}

bool CStorageOleIn::hasStream(const std::string &name) const
{
	if (!m_Impl) return false;
	CGsfIn *g = static_cast<CGsfIn *>(m_Impl);
	GsfInput *s = gsf_infile_child_by_name(g->Infile, name.c_str());
	if (!s) return false;
	g_object_unref(s);
	return true;
}

bool CStorageOleIn::readStream(const std::string &name, std::vector<uint8> &out) const
{
	out.clear();
	if (!m_Impl) return false;
	CGsfIn *g = static_cast<CGsfIn *>(m_Impl);
	GsfInput *s = gsf_infile_child_by_name(g->Infile, name.c_str());
	if (!s) return false;
	gsf_off_t sz = gsf_input_size(s);
	out.resize((size_t)sz);
	bool ok = true;
	if (sz > 0)
	{
		gsf_input_seek(s, 0, G_SEEK_SET);
		if (!gsf_input_read(s, (size_t)sz, &out[0])) { out.clear(); ok = false; }
	}
	g_object_unref(s);
	return ok;
}

const std::vector<std::string> &CStorageOleIn::streamNames() const
{
	static const std::vector<std::string> s_empty;
	if (!m_Impl) return s_empty;
	return static_cast<CGsfIn *>(m_Impl)->Names;
}

// ---- CStorageOleOut (gsf) --------------------------------------------------------------------

namespace {
struct CGsfOut
{
	std::vector<std::pair<std::string, std::vector<uint8> > > Streams;
	uint8 Clsid[16];
	bool HaveClsid;
	CGsfOut() : HaveClsid(false) { memset(Clsid, 0, 16); }
};
} /* anonymous namespace */

CStorageOleOut::CStorageOleOut() : m_Impl(new CGsfOut()) { }
CStorageOleOut::~CStorageOleOut() { delete static_cast<CGsfOut *>(m_Impl); }

void CStorageOleOut::setClassId(const uint8 classId[16])
{
	CGsfOut *o = static_cast<CGsfOut *>(m_Impl);
	memcpy(o->Clsid, classId, 16);
	o->HaveClsid = true;
}

void CStorageOleOut::addStream(const std::string &name, const std::vector<uint8> &data)
{
	static_cast<CGsfOut *>(m_Impl)->Streams.push_back(std::make_pair(name, data));
}

void CStorageOleOut::addStreamSwap(const std::string &name, std::vector<uint8> &data)
{
	CGsfOut *o = static_cast<CGsfOut *>(m_Impl);
	o->Streams.push_back(std::pair<std::string, std::vector<uint8> >(name, std::vector<uint8>()));
	o->Streams.back().second.swap(data);
}

bool CStorageOleOut::write(const std::string &path)
{
	ensureGsfInit();
	CGsfOut *o = static_cast<CGsfOut *>(m_Impl);
	GError *err = NULL;
	GsfOutput *output = gsf_output_stdio_new(path.c_str(), &err);
	if (!output) { if (err) g_error_free(err); return false; }
	GsfOutfile *outfile = gsf_outfile_msole_new(output);
	g_object_unref(G_OBJECT(output));
	bool ok = true;
	for (size_t i = 0; i < o->Streams.size(); ++i)
	{
		GsfOutput *child = GSF_OUTPUT(gsf_outfile_new_child(outfile, o->Streams[i].first.c_str(), FALSE));
		if (!child) { ok = false; break; }
		if (!o->Streams[i].second.empty())
			gsf_output_write(child, o->Streams[i].second.size(), &o->Streams[i].second[0]);
		gsf_output_close(child);
		g_object_unref(G_OBJECT(child));
	}
	if (o->HaveClsid)
		gsf_outfile_msole_set_class_id((GsfOutfileMSOle *)outfile, o->Clsid);
	gsf_output_close(GSF_OUTPUT(outfile));
	g_object_unref(G_OBJECT(outfile));
	return ok;
}

// ==============================================================================================
#endif // NL_PIPELINE_NATIVE_OLE
// ==============================================================================================

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
