/**
 * \file max_thumbnail.cpp
 * \brief OLE SummaryInformation thumbnail R/W for zone_painter (ui M5)
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Parses MS-OLEPS property set streams (SummaryInformation) to extract/replace
 * PIDSI_THUMBNAIL (id 17, VT_CF). Does not modify pipeline_max.
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

#include "max_thumbnail.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

#include <nel/misc/bitmap.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/stream.h>
#include <nel/misc/common.h>

#include "../pipeline_max/storage_ole.h"

using namespace NLMISC;
using namespace PIPELINE::MAX;

namespace ZPTHUMB {

// Leading 0x05 = '\05' stream name marker used by OLE property storage.
const char *kSummaryInformationStream = "\05SummaryInformation";

// MS-OLEPS / PIDSI
static const uint32 kPidCodePage = 1;
static const uint32 kPidThumbnail = 17;
static const uint32 kVtI2 = 2;
static const uint32 kVtCf = 71;
static const uint32 kCfDib = 8;
static const uint32 kCfMetafilePict = 3;

// FMTID_SummaryInformation
static const uint8 kFmtidSummaryInformation[16] = {
	0xe0, 0x85, 0x9f, 0xf2, 0xf9, 0x4f, 0x68, 0x10,
	0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9
};

// ---------------------------------------------------------------------------------------------

std::string thumbCacheDir()
{
	std::string dir = CPath::getApplicationDirectory("zone_painter", true);
	if (!dir.empty() && dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
		dir += "/";
	dir += "thumbcache";
	if (!CFile::isDirectory(dir))
		CFile::createDirectoryTree(dir);
	return dir;
}

static uint32 hashPath(const std::string &s)
{
	// FNV-1a 32-bit
	uint32 h = 2166136261u;
	for (size_t i = 0; i < s.size(); ++i)
	{
		h ^= (uint8)s[i];
		h *= 16777619u;
	}
	return h;
}

std::string cachedThumbPath(const std::string &maxPath)
{
	if (maxPath.empty())
		return std::string();
	// makePathAbsolute(..., true) may append a trailing slash which breaks stat() on files.
	std::string abs = CPath::makePathAbsolute(maxPath, CPath::getCurrentPath(), /*addTrailingSlash=*/false);
	while (!abs.empty() && (abs[abs.size() - 1] == '/' || abs[abs.size() - 1] == '\\'))
		abs.resize(abs.size() - 1);
	uint32 mtime = CFile::getFileModificationDate(abs);
	uint32 h = hashPath(abs);
	return thumbCacheDir() + "/" + NLMISC::toString("%08x_%08x.tga", h, mtime);
}

bool readSummaryInformationStream(const std::string &maxPath, std::vector<uint8> &out)
{
	out.clear();
	CStorageOleIn in;
	if (!in.open(maxPath))
		return false;
	if (!in.readStream(kSummaryInformationStream, out))
		return false;
	return !out.empty();
}

// ---------------------------------------------------------------------------------------------
// Property set parse helpers

struct SPropEntry
{
	uint32 Pid;
	uint32 Offset; // relative to section start
	uint32 Type;
	std::vector<uint8> Value; // full typed value including VT dword
};

struct SPropSection
{
	uint8 FmtId[16];
	uint32 Size;
	std::vector<SPropEntry> Props;
	// Raw section bytes (size field through end) for faithful rebuild when needed
	std::vector<uint8> Raw;
};

struct SPropSet
{
	uint16 ByteOrder;
	uint16 Version;
	uint32 SysId;
	uint8 Clsid[16];
	std::vector<SPropSection> Sections;
};

static bool readU32(const std::vector<uint8> &b, size_t off, uint32 &v)
{
	if (off + 4 > b.size())
		return false;
	v = (uint32)b[off] | ((uint32)b[off + 1] << 8) | ((uint32)b[off + 2] << 16) | ((uint32)b[off + 3] << 24);
	return true;
}

static bool readU16(const std::vector<uint8> &b, size_t off, uint16 &v)
{
	if (off + 2 > b.size())
		return false;
	v = (uint16)b[off] | ((uint16)b[off + 1] << 8);
	return true;
}

static void appendU16(std::vector<uint8> &o, uint16 v)
{
	o.push_back((uint8)(v & 0xff));
	o.push_back((uint8)((v >> 8) & 0xff));
}

static void appendU32(std::vector<uint8> &o, uint32 v)
{
	o.push_back((uint8)(v & 0xff));
	o.push_back((uint8)((v >> 8) & 0xff));
	o.push_back((uint8)((v >> 16) & 0xff));
	o.push_back((uint8)((v >> 24) & 0xff));
}

static void appendI32(std::vector<uint8> &o, sint32 v)
{
	appendU32(o, (uint32)v);
}

/** Property value size (including VT dword), best-effort for types we care about. */
static bool propValueSize(const std::vector<uint8> &stream, size_t absOff, uint32 &sizeOut)
{
	uint32 vt = 0;
	if (!readU32(stream, absOff, vt))
		return false;
	if (vt == kVtI2)
	{
		// type(4) + i2(2) + pad(2) = 8
		sizeOut = 8;
		return true;
	}
	if (vt == 19) // VT_UI4
	{
		sizeOut = 8;
		return true;
	}
	if (vt == kVtCf)
	{
		uint32 cb = 0;
		if (!readU32(stream, absOff + 4, cb))
			return false;
		if (cb > 0xFFFFFF00)
			return false; // 8 + cb would wrap uint32 — structurally invalid claim
		// type(4) + cbSize(4) + data(cb). Some Max files truncate the stream a few
		// bytes short of the claimed cbSize; caller clamps to available section bytes.
		sizeOut = 8 + cb;
		return true;
	}
	// VT_LPSTR (30): type + size + chars + pad to 4
	if (vt == 30)
	{
		uint32 n = 0;
		if (!readU32(stream, absOff + 4, n))
			return false;
		uint32 body = 4 + n;
		uint32 pad = (4 - (body % 4)) % 4;
		sizeOut = 4 + body + pad;
		return true;
	}
	// VT_FILETIME (64): type + 8
	if (vt == 64)
	{
		sizeOut = 12;
		return true;
	}
	// VT_I4 (3)
	if (vt == 3)
	{
		sizeOut = 8;
		return true;
	}
	// Unknown: refuse
	return false;
}

static bool parsePropertySet(const std::vector<uint8> &stream, SPropSet &ps)
{
	ps = SPropSet();
	if (stream.size() < 28)
		return false;
	if (!readU16(stream, 0, ps.ByteOrder) || ps.ByteOrder != 0xFFFE)
		return false;
	if (!readU16(stream, 2, ps.Version))
		return false;
	if (!readU32(stream, 4, ps.SysId))
		return false;
	memcpy(ps.Clsid, &stream[8], 16);
	uint32 nsets = 0;
	if (!readU32(stream, 24, nsets) || nsets == 0 || nsets > 16)
		return false;

	size_t dirOff = 28;
	for (uint32 i = 0; i < nsets; ++i)
	{
		if (dirOff + 20 > stream.size())
			return false;
		SPropSection sec;
		memcpy(sec.FmtId, &stream[dirOff], 16);
		uint32 sectOff = 0;
		if (!readU32(stream, dirOff + 16, sectOff))
			return false;
		dirOff += 20;

		// All offset/size sums below in size_t: these are untrusted uint32 fields, and a
		// 32-bit sum wraps past the check (e.g. secSize 0xFFFFFFD0 + sectOff 48 → 0), then
		// the .assign() end iterator lands ~4 GB out of bounds.
		if ((size_t)sectOff + 8 > stream.size())
			return false;
		uint32 secSize = 0, nprop = 0;
		if (!readU32(stream, sectOff, secSize) || !readU32(stream, sectOff + 4, nprop))
			return false;
		if (secSize < 8 || (size_t)sectOff + (size_t)secSize > stream.size() || nprop > 256)
			return false;
		sec.Size = secSize;
		sec.Raw.assign(stream.begin() + sectOff, stream.begin() + sectOff + secSize);

		for (uint32 j = 0; j < nprop; ++j)
		{
			size_t eoff = sectOff + 8 + j * 8;
			if (eoff + 8 > stream.size())
				return false;
			SPropEntry pe;
			if (!readU32(stream, eoff, pe.Pid) || !readU32(stream, eoff + 4, pe.Offset))
				return false;
			size_t abs = (size_t)sectOff + (size_t)pe.Offset;
			if (abs >= (size_t)sectOff + (size_t)secSize)
				return false;
			// Bound this property by the next property offset (or section end).
			uint32 nextRel = secSize;
			for (uint32 k = 0; k < nprop; ++k)
			{
				uint32 opid = 0, ooff = 0;
				readU32(stream, sectOff + 8 + k * 8, opid);
				readU32(stream, sectOff + 8 + k * 8 + 4, ooff);
				if (ooff > pe.Offset && ooff < nextRel)
					nextRel = ooff;
			}
			uint32 avail = nextRel - pe.Offset;
			uint32 vsz = 0;
			if (!propValueSize(stream, abs, vsz) || vsz > avail)
				vsz = avail; // clamp: Max SI streams are sometimes short of claimed cbSize
			if (vsz < 4)
				return false;
			if (!readU32(stream, abs, pe.Type))
				return false;
			pe.Value.assign(stream.begin() + abs, stream.begin() + abs + vsz);
			sec.Props.push_back(pe);
		}
		ps.Sections.push_back(sec);
	}
	return !ps.Sections.empty();
}

/** Serialize a property set stream from parsed sections (prop Values used as-is). */
static bool serializePropertySet(const SPropSet &ps, std::vector<uint8> &out)
{
	out.clear();
	if (ps.Sections.empty())
		return false;

	// Header
	appendU16(out, ps.ByteOrder ? ps.ByteOrder : 0xFFFE);
	appendU16(out, ps.Version);
	appendU32(out, ps.SysId);
	out.insert(out.end(), ps.Clsid, ps.Clsid + 16);
	appendU32(out, (uint32)ps.Sections.size());

	// Directory placeholders
	size_t dirStart = out.size();
	for (size_t i = 0; i < ps.Sections.size(); ++i)
	{
		out.insert(out.end(), ps.Sections[i].FmtId, ps.Sections[i].FmtId + 16);
		appendU32(out, 0); // offset filled later
	}

	// Sections
	for (size_t si = 0; si < ps.Sections.size(); ++si)
	{
		const SPropSection &sec = ps.Sections[si];
		// Patch directory offset
		uint32 sectOff = (uint32)out.size();
		size_t dirEntry = dirStart + si * 20 + 16;
		out[dirEntry + 0] = (uint8)(sectOff & 0xff);
		out[dirEntry + 1] = (uint8)((sectOff >> 8) & 0xff);
		out[dirEntry + 2] = (uint8)((sectOff >> 16) & 0xff);
		out[dirEntry + 3] = (uint8)((sectOff >> 24) & 0xff);

		// Build section body
		std::vector<uint8> body;
		// size placeholder + nprop
		appendU32(body, 0);
		appendU32(body, (uint32)sec.Props.size());
		// prop directory (pid + offset placeholders)
		size_t propDir = body.size();
		for (size_t j = 0; j < sec.Props.size(); ++j)
		{
			appendU32(body, sec.Props[j].Pid);
			appendU32(body, 0);
		}
		// values (4-byte aligned offsets from section start)
		for (size_t j = 0; j < sec.Props.size(); ++j)
		{
			while (body.size() % 4)
				body.push_back(0);
			uint32 rel = (uint32)body.size();
			body[propDir + j * 8 + 4] = (uint8)(rel & 0xff);
			body[propDir + j * 8 + 5] = (uint8)((rel >> 8) & 0xff);
			body[propDir + j * 8 + 6] = (uint8)((rel >> 16) & 0xff);
			body[propDir + j * 8 + 7] = (uint8)((rel >> 24) & 0xff);
			body.insert(body.end(), sec.Props[j].Value.begin(), sec.Props[j].Value.end());
		}
		// Final size
		uint32 sz = (uint32)body.size();
		body[0] = (uint8)(sz & 0xff);
		body[1] = (uint8)((sz >> 8) & 0xff);
		body[2] = (uint8)((sz >> 16) & 0xff);
		body[3] = (uint8)((sz >> 24) & 0xff);
		out.insert(out.end(), body.begin(), body.end());
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// DIB decode

static bool decodeDib24At(const uint8 *dib, size_t dibLen, CBitmap &out)
{
	if (dibLen < 40)
		return false;
	uint32 biSize = (uint32)dib[0] | ((uint32)dib[1] << 8) | ((uint32)dib[2] << 16) | ((uint32)dib[3] << 24);
	if (biSize != 40)
		return false;
	sint32 w = (sint32)((uint32)dib[4] | ((uint32)dib[5] << 8) | ((uint32)dib[6] << 16) | ((uint32)dib[7] << 24));
	sint32 h = (sint32)((uint32)dib[8] | ((uint32)dib[9] << 8) | ((uint32)dib[10] << 16) | ((uint32)dib[11] << 24));
	uint16 planes = (uint16)dib[12] | ((uint16)dib[13] << 8);
	uint16 bpp = (uint16)dib[14] | ((uint16)dib[15] << 8);
	uint32 comp = (uint32)dib[16] | ((uint32)dib[17] << 8) | ((uint32)dib[18] << 16) | ((uint32)dib[19] << 24);
	if (planes != 1 || bpp != 24 || comp != 0)
		return false;
	bool bottomUp = true;
	if (h < 0)
	{
		bottomUp = false;
		h = -h;
	}
	if (w <= 0 || h <= 0 || w > 4096 || h > 4096)
		return false;
	const uint32 rowBytes = ((uint32)w * 3 + 3) & ~3u;
	const size_t pixOff = biSize; // no color table for 24bpp
	const size_t need = pixOff + (size_t)rowBytes * (size_t)h;
	if (need > dibLen)
		return false;

	out.resize(w, h, CBitmap::RGBA, true);
	CObjectVector<uint8> &px = out.getPixels();
	for (sint32 y = 0; y < h; ++y)
	{
		sint32 srcY = bottomUp ? (h - 1 - y) : y;
		const uint8 *row = dib + pixOff + (size_t)srcY * rowBytes;
		for (sint32 x = 0; x < w; ++x)
		{
			const size_t di = ((size_t)y * (size_t)w + (size_t)x) * 4;
			px[di + 0] = row[x * 3 + 2]; // R
			px[di + 1] = row[x * 3 + 1]; // G
			px[di + 2] = row[x * 3 + 0]; // B
			px[di + 3] = 255;
		}
	}
	return true;
}

/** Find a 24bpp BITMAPINFOHEADER in blob and decode it. */
static bool decodeDibFromBlob(const uint8 *data, size_t len, CBitmap &out)
{
	if (!data || len < 40)
		return false;
	// Direct DIB
	if (decodeDib24At(data, len, out))
		return true;
	// Scan for biSize=40 with sane dims (CF_METAFILEPICT / STRETCHDIBITS embed)
	for (size_t i = 0; i + 40 < len; ++i)
	{
		if (data[i] == 40 && data[i + 1] == 0 && data[i + 2] == 0 && data[i + 3] == 0)
		{
			if (decodeDib24At(data + i, len - i, out))
				return true;
		}
	}
	return false;
}

static bool decodeVtCfThumbnail(const std::vector<uint8> &propValue, CBitmap &out)
{
	// propValue: VT(4) + cbSize(4) + ulClipFmt(4) + pClipData
	// Note: some Max SummaryInformation streams are a few bytes short of the claimed
	// cbSize; clamp to the bytes we actually have.
	if (propValue.size() < 12)
		return false;
	uint32 vt = 0, cb = 0;
	if (!readU32(propValue, 0, vt) || vt != kVtCf)
		return false;
	if (!readU32(propValue, 4, cb) || cb < 4)
		return false;
	const size_t dataAvail = propValue.size() - 8; // after type + cbSize fields
	const size_t dataLen = std::min((size_t)cb, dataAvail);
	if (dataLen < 4)
		return false;
	sint32 ulClipFmt = (sint32)((uint32)propValue[8] | ((uint32)propValue[9] << 8)
	                            | ((uint32)propValue[10] << 16) | ((uint32)propValue[11] << 24));
	const uint8 *clip = &propValue[12];
	size_t clipLen = dataLen - 4; // pClipData after ulClipFmt

	if (ulClipFmt == -1)
	{
		// First DWORD = clipboard format id
		if (clipLen < 4)
			return false;
		uint32 cf = (uint32)clip[0] | ((uint32)clip[1] << 8) | ((uint32)clip[2] << 16) | ((uint32)clip[3] << 24);
		if (cf == kCfDib)
			return decodeDibFromBlob(clip + 4, clipLen - 4, out);
		if (cf == kCfMetafilePict)
			return decodeDibFromBlob(clip + 4, clipLen - 4, out);
		// Unknown CF: still try to find an embedded DIB
		return decodeDibFromBlob(clip, clipLen, out);
	}
	// Named format or other: best-effort DIB scan
	return decodeDibFromBlob(clip, clipLen, out);
}

// ---------------------------------------------------------------------------------------------

bool extractThumbnailProperty(const std::string &maxPath, std::vector<uint8> &outPropValue)
{
	outPropValue.clear();
	std::vector<uint8> stream;
	if (!readSummaryInformationStream(maxPath, stream))
		return false;
	SPropSet ps;
	if (!parsePropertySet(stream, ps))
		return false;
	for (size_t s = 0; s < ps.Sections.size(); ++s)
	{
		for (size_t p = 0; p < ps.Sections[s].Props.size(); ++p)
		{
			if (ps.Sections[s].Props[p].Pid == kPidThumbnail
			    && ps.Sections[s].Props[p].Type == kVtCf)
			{
				outPropValue = ps.Sections[s].Props[p].Value;
				return true;
			}
		}
	}
	return false;
}

bool extractThumbnailBitmap(const std::string &maxPath, CBitmap &out)
{
	std::vector<uint8> prop;
	if (!extractThumbnailProperty(maxPath, prop))
		return false;
	return decodeVtCfThumbnail(prop, out);
}

/** Write a cache .tga; on ANY failure delete the partial file — COFile creates it on
 *  open, and the caches are keyed by source mtime, so a truncated file left behind
 *  (disk full, kill) would be served forever as a permanently broken preview. */
static bool writeCacheTGA(const std::string &cache, CBitmap &bmp)
{
	bool ok = false;
	try
	{
		COFile of(cache);
		ok = bmp.writeTGA(of, 32, false);
	}
	catch (...)
	{
		ok = false;
	}
	if (!ok)
	{
		CFile::deleteFile(cache);
		return false;
	}
	return CFile::fileExists(cache);
}

bool ensureCachedThumbnail(const std::string &maxPath, std::string &outTgaPath)
{
	outTgaPath.clear();
	if (maxPath.empty() || !CFile::fileExists(maxPath))
		return false;

	std::string cache = cachedThumbPath(maxPath);
	if (cache.empty())
		return false;
	if (CFile::fileExists(cache))
	{
		outTgaPath = cache;
		return true;
	}

	CBitmap bmp;
	if (!extractThumbnailBitmap(maxPath, bmp))
		return false;
	if (bmp.getWidth() == 0 || bmp.getHeight() == 0)
		return false;

	// Ensure parent exists
	thumbCacheDir();
	if (!writeCacheTGA(cache, bmp))
		return false;
	outTgaPath = cache;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Tileset palette previews (ui M8)

std::string tilesetPreviewCacheDir()
{
	std::string dir = thumbCacheDir() + "/tileset";
	if (!CFile::isDirectory(dir))
		CFile::createDirectoryTree(dir);
	return dir;
}

std::string cachedTilesetPreviewPath(const std::string &bankPath, int setIndex,
                                     const std::string &seasonKey, uint32 sourceMtime)
{
	if (bankPath.empty() || setIndex < 0)
		return std::string();
	std::string abs = CPath::makePathAbsolute(bankPath, CPath::getCurrentPath(), false);
	while (!abs.empty() && (abs[abs.size() - 1] == '/' || abs[abs.size() - 1] == '\\'))
		abs.resize(abs.size() - 1);
	uint32 h = hashPath(abs);
	std::string season = seasonKey.empty() ? std::string("auto") : seasonKey;
	// Sanitize season tag for filenames
	for (size_t i = 0; i < season.size(); ++i)
	{
		char c = season[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-'))
			season[i] = '_';
	}
	return tilesetPreviewCacheDir() + "/"
		+ NLMISC::toString("%08x_s%03d_%s_%08x.tga", h, setIndex, season.c_str(), sourceMtime);
}

bool ensureTilesetPreview(const std::string &bankPath, int setIndex,
                          const std::string &seasonKey, const std::string &sourcePath,
                          std::string &outTgaPath, uint sidePx)
{
	outTgaPath.clear();
	if (sourcePath.empty() || !CFile::fileExists(sourcePath))
		return false;
	if (sidePx == 0)
		sidePx = 64;

	uint32 mtime = CFile::getFileModificationDate(sourcePath);
	std::string cache = cachedTilesetPreviewPath(bankPath, setIndex, seasonKey, mtime);
	if (cache.empty())
		return false;
	if (CFile::fileExists(cache))
	{
		outTgaPath = cache;
		return true;
	}

	// Load source (png/tga/dds — CBitmap handles all three). Never used as an image-view
	// source for the agent; only written as a small cache TGA for NLGUI.
	CBitmap bmp;
	try
	{
		CIFile in;
		if (!in.open(sourcePath))
			return false;
		bmp.load(in);
	}
	catch (...)
	{
		return false;
	}
	if (bmp.getWidth() == 0 || bmp.getHeight() == 0)
		return false;
	if (bmp.getPixelFormat() != CBitmap::RGBA)
		bmp.convertToType(CBitmap::RGBA);
	if (bmp.getWidth() != sidePx || bmp.getHeight() != sidePx)
		bmp.resample(sidePx, sidePx);

	tilesetPreviewCacheDir();
	if (!writeCacheTGA(cache, bmp))
		return false;
	outTgaPath = cache;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Displacement palette previews (ui M9a)

std::string displacePreviewCacheDir()
{
	std::string dir = thumbCacheDir() + "/displace";
	if (!CFile::isDirectory(dir))
		CFile::createDirectoryTree(dir);
	return dir;
}

std::string cachedDisplacePreviewPath(const std::string &bankPath, int mapIndex, uint32 sourceMtime)
{
	if (bankPath.empty() || mapIndex < 0)
		return std::string();
	std::string abs = CPath::makePathAbsolute(bankPath, CPath::getCurrentPath(), false);
	while (!abs.empty() && (abs[abs.size() - 1] == '/' || abs[abs.size() - 1] == '\\'))
		abs.resize(abs.size() - 1);
	uint32 h = hashPath(abs);
	// _g2 suffix: M10c — same tone map as _g1, but cache key bumped so writers always
	// re-emit after the CPath-index fix (see rebuildTilesetPalette). Previews are
	// true 32-bit RGBA (type-2 TGA); bump again if tone-map or pixel layout changes.
	return displacePreviewCacheDir() + "/"
		+ NLMISC::toString("%08x_m%03d_%08x_g2.tga", h, mapIndex, sourceMtime);
}

bool ensureDisplacePreview(const std::string &bankPath, int mapIndex,
                           const std::string &sourcePath, std::string &outTgaPath,
                           uint sidePx)
{
	outTgaPath.clear();
	if (sourcePath.empty() || !CFile::fileExists(sourcePath))
		return false;
	if (sidePx == 0)
		sidePx = 64;

	uint32 mtime = CFile::getFileModificationDate(sourcePath);
	std::string cache = cachedDisplacePreviewPath(bankPath, mapIndex, mtime);
	if (cache.empty())
		return false;
	if (CFile::fileExists(cache))
	{
		outTgaPath = cache;
		return true;
	}

	// Load noise map (typically 32x32 grayscale). Never viewed by the agent as an image.
	CBitmap bmp;
	try
	{
		CIFile in;
		if (!in.open(sourcePath))
			return false;
		bmp.load(in);
	}
	catch (...)
	{
		return false;
	}
	if (bmp.getWidth() == 0 || bmp.getHeight() == 0)
		return false;
	if (bmp.getPixelFormat() != CBitmap::RGBA)
		bmp.convertToType(CBitmap::RGBA);

	const uint sw = bmp.getWidth();
	const uint sh = bmp.getHeight();
	const CObjectVector<uint8> &srcPx = bmp.getPixels();

	// Luminance min/max (displacement maps are signed-ish around mid-gray; stretch for readability)
	uint8 minL = 255, maxL = 0;
	for (uint y = 0; y < sh; ++y)
	{
		for (uint x = 0; x < sw; ++x)
		{
			const size_t si = ((size_t)y * sw + x) * 4;
			// Prefer R (luminance textures store gray in RGB equally after convert)
			uint8 l = srcPx[si];
			if (l < minL) minL = l;
			if (l > maxL) maxL = l;
		}
	}
	const int range = (int)maxL - (int)minL;

	// Point upsample + min..max stretch + sqrt gamma lift (M10a) so structure is
	// readable at 64px — raw noise often clusters near black after linear stretch alone.
	// Written as 32-bit RGBA (type-2 TGA), same path as tileset previews. ImageMagick
	// may still label equal-channel content "GrayscaleAlpha"; that is content typing,
	// not an 8-bit luminance container — NLGUI loads these via createTextureFile fine
	// once CPath can resolve the basename (M10c re-indexes after write).
	CBitmap out;
	out.resize(sidePx, sidePx, CBitmap::RGBA);
	CObjectVector<uint8> &dstPx = out.getPixels();
	for (uint y = 0; y < sidePx; ++y)
	{
		const uint sy = (sh == 0) ? 0 : (y * sh) / sidePx;
		for (uint x = 0; x < sidePx; ++x)
		{
			const uint sx = (sw == 0) ? 0 : (x * sw) / sidePx;
			const size_t si = ((size_t)sy * sw + sx) * 4;
			const uint8 raw = srcPx[si];
			float t;
			if (range > 0)
				t = (float)((int)raw - (int)minL) / (float)range;
			else
				t = 0.5f;
			if (t < 0.f) t = 0.f;
			if (t > 1.f) t = 1.f;
			// sqrt gamma lift: mid-tones pop; near-black structure stays visible
			t = std::sqrt(t);
			const uint8 l = (uint8)(t * 255.f + 0.5f);
			const size_t di = ((size_t)y * sidePx + x) * 4;
			dstPx[di + 0] = l;
			dstPx[di + 1] = l;
			dstPx[di + 2] = l;
			dstPx[di + 3] = 255;
		}
	}
	// Defensive: keep writer consistent with ensureTilesetPreview (RGBA before TGA).
	if (out.getPixelFormat() != CBitmap::RGBA)
		out.convertToType(CBitmap::RGBA);

	displacePreviewCacheDir();
	if (!writeCacheTGA(cache, out))
		return false;
	outTgaPath = cache;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Encode / write helpers (M5c)

bool encodeDib24(const CBitmap &bmpIn, std::vector<uint8> &outDib, uint maxDim)
{
	outDib.clear();
	if (bmpIn.getWidth() == 0 || bmpIn.getHeight() == 0)
		return false;

	CBitmap bmp = bmpIn;
	if (bmp.getPixelFormat() != CBitmap::RGBA)
		bmp.convertToType(CBitmap::RGBA);

	uint w = bmp.getWidth();
	uint h = bmp.getHeight();
	if (maxDim > 0 && (w > maxDim || h > maxDim))
	{
		float s = (float)maxDim / (float)std::max(w, h);
		uint nw = std::max(1u, (uint)(w * s + 0.5f));
		uint nh = std::max(1u, (uint)(h * s + 0.5f));
		bmp.resample(nw, nh);
		w = bmp.getWidth();
		h = bmp.getHeight();
	}

	const uint32 rowBytes = (w * 3 + 3) & ~3u;
	outDib.resize(40 + rowBytes * h, 0);
	// BITMAPINFOHEADER
	outDib[0] = 40;
	outDib[4] = (uint8)(w & 0xff);
	outDib[5] = (uint8)((w >> 8) & 0xff);
	outDib[6] = (uint8)((w >> 16) & 0xff);
	outDib[7] = (uint8)((w >> 24) & 0xff);
	outDib[8] = (uint8)(h & 0xff); // positive = bottom-up
	outDib[9] = (uint8)((h >> 8) & 0xff);
	outDib[10] = (uint8)((h >> 16) & 0xff);
	outDib[11] = (uint8)((h >> 24) & 0xff);
	outDib[12] = 1;  // planes
	outDib[14] = 24; // bpp
	// biSizeImage
	uint32 imgSz = rowBytes * h;
	outDib[20] = (uint8)(imgSz & 0xff);
	outDib[21] = (uint8)((imgSz >> 8) & 0xff);
	outDib[22] = (uint8)((imgSz >> 16) & 0xff);
	outDib[23] = (uint8)((imgSz >> 24) & 0xff);
	// 72 DPI placeholders (common)
	outDib[24] = 0x12;
	outDib[25] = 0x0b;
	outDib[28] = 0x12;
	outDib[29] = 0x0b;

	const CObjectVector<uint8> &px = bmp.getPixels();
	for (uint y = 0; y < h; ++y)
	{
		// bottom-up: write row (h-1-y) of source into row y of DIB
		uint srcY = h - 1 - y;
		uint8 *row = &outDib[40 + y * rowBytes];
		for (uint x = 0; x < w; ++x)
		{
			const size_t si = ((size_t)srcY * w + x) * 4;
			row[x * 3 + 0] = px[si + 2]; // B
			row[x * 3 + 1] = px[si + 1]; // G
			row[x * 3 + 2] = px[si + 0]; // R
		}
	}
	return true;
}

void wrapDibAsVtCfProperty(const std::vector<uint8> &dib, std::vector<uint8> &outPropValue)
{
	outPropValue.clear();
	// VT_CF + cbSize + ulClipFmt(-1) + CF_DIB + dib
	// cbSize is written SPEC-EXACT ([MS-OLEPS] CLIPDATA: ulClipFmt + data). Genuine Max
	// streams claim a few bytes MORE than present (the parse clamp above tolerates it);
	// whether Max's own reader compensates for its writer's convention is unverified —
	// if a painter-written thumbnail ever renders truncated in Max's open dialog, match
	// the observed Max claim (+8) here instead. Spec-exact is what every conformant
	// reader (Explorer property handlers etc.) expects.
	const uint32 pClipDataLen = 4 + (uint32)dib.size(); // cf dword + dib
	const uint32 cbSize = 4 + pClipDataLen;             // ulClipFmt + pClipData
	appendU32(outPropValue, kVtCf);
	appendU32(outPropValue, cbSize);
	appendI32(outPropValue, -1);
	appendU32(outPropValue, kCfDib);
	outPropValue.insert(outPropValue.end(), dib.begin(), dib.end());
}

bool rebuildSummaryInformationWithThumbnail(const std::vector<uint8> &inStream,
                                            const CBitmap *bmp,
                                            std::vector<uint8> &outStream,
                                            uint maxDim,
                                            bool keepExisting)
{
	outStream.clear();
	SPropSet ps;
	if (!parsePropertySet(inStream, ps))
		return false;

	// Prefer the FMTID_SummaryInformation section; else first section.
	size_t secIdx = 0;
	for (size_t i = 0; i < ps.Sections.size(); ++i)
	{
		if (memcmp(ps.Sections[i].FmtId, kFmtidSummaryInformation, 16) == 0)
		{
			secIdx = i;
			break;
		}
	}
	SPropSection &sec = ps.Sections[secIdx];

	if (!keepExisting && bmp)
	{
		std::vector<uint8> dib;
		if (!encodeDib24(*bmp, dib, maxDim))
			return false;
		std::vector<uint8> prop;
		wrapDibAsVtCfProperty(dib, prop);

		bool found = false;
		for (size_t p = 0; p < sec.Props.size(); ++p)
		{
			if (sec.Props[p].Pid == kPidThumbnail)
			{
				sec.Props[p].Type = kVtCf;
				sec.Props[p].Value.swap(prop);
				found = true;
				break;
			}
		}
		if (!found)
		{
			SPropEntry pe;
			pe.Pid = kPidThumbnail;
			pe.Offset = 0;
			pe.Type = kVtCf;
			pe.Value.swap(prop);
			sec.Props.push_back(pe);
		}
		// Ensure code page exists (PID 1) — many Max files have it
		bool haveCp = false;
		for (size_t p = 0; p < sec.Props.size(); ++p)
			if (sec.Props[p].Pid == kPidCodePage)
				haveCp = true;
		if (!haveCp)
		{
			SPropEntry pe;
			pe.Pid = kPidCodePage;
			pe.Type = kVtI2;
			// VT_I2 + value 1252 + pad
			pe.Value.resize(8, 0);
			pe.Value[0] = (uint8)kVtI2;
			pe.Value[4] = 0xe4; // 1252
			pe.Value[5] = 0x04;
			sec.Props.insert(sec.Props.begin(), pe);
		}
	}
	// else keepExisting: leave prop Values untouched

	return serializePropertySet(ps, outStream);
}

bool thumbRoundtripIdentical(const std::string &maxPath, std::string &err)
{
	err.clear();
	std::vector<uint8> inStream;
	if (!readSummaryInformationStream(maxPath, inStream))
	{
		err = "no SummaryInformation stream";
		return false;
	}
	std::vector<uint8> outStream;
	if (!rebuildSummaryInformationWithThumbnail(inStream, NULL, outStream, 128, /*keepExisting=*/true))
	{
		err = "parse/reserialize failed";
		return false;
	}
	if (outStream != inStream)
	{
		err = NLMISC::toString("byte mismatch (in %u out %u)", (uint)inStream.size(), (uint)outStream.size());
		// Optional: if only layout padding differs but props match, still report fail (strict gate)
		return false;
	}
	return true;
}

bool buildSummaryInformationWithThumbnail(const std::string &maxPath,
                                          const CBitmap &bmp,
                                          std::vector<uint8> &outStream,
                                          uint maxDim,
                                          bool gateRoundtrip,
                                          std::string *err)
{
	outStream.clear();
	if (err) err->clear();

	std::vector<uint8> inStream;
	const bool haveSI = readSummaryInformationStream(maxPath, inStream);

	if (haveSI && gateRoundtrip)
	{
		std::vector<uint8> rt;
		if (!rebuildSummaryInformationWithThumbnail(inStream, NULL, rt, 128, true) || rt != inStream)
		{
			if (err) *err = "roundtrip gate failed; refusing to rewrite SummaryInformation";
			return false;
		}
	}

	if (haveSI)
	{
		if (!rebuildSummaryInformationWithThumbnail(inStream, &bmp, outStream, maxDim, false))
		{
			if (err) *err = "rebuild with new thumbnail failed";
			return false;
		}
		return true;
	}

	// Synthesize a minimal SummaryInformation when the stream is absent.
	SPropSet ps;
	ps.ByteOrder = 0xFFFE;
	ps.Version = 0;
	ps.SysId = 0x00020105; // Windows NT
	memset(ps.Clsid, 0, 16);
	SPropSection sec;
	memcpy(sec.FmtId, kFmtidSummaryInformation, 16);
	{
		SPropEntry pe;
		pe.Pid = kPidCodePage;
		pe.Type = kVtI2;
		pe.Value.resize(8, 0);
		pe.Value[0] = (uint8)kVtI2;
		pe.Value[4] = 0xe4;
		pe.Value[5] = 0x04; // 1252
		sec.Props.push_back(pe);
	}
	{
		std::vector<uint8> dib;
		if (!encodeDib24(bmp, dib, maxDim))
		{
			if (err) *err = "DIB encode failed";
			return false;
		}
		SPropEntry pe;
		pe.Pid = kPidThumbnail;
		pe.Type = kVtCf;
		wrapDibAsVtCfProperty(dib, pe.Value);
		sec.Props.push_back(pe);
	}
	ps.Sections.push_back(sec);
	if (!serializePropertySet(ps, outStream))
	{
		if (err) *err = "synthesize SummaryInformation failed";
		return false;
	}
	return true;
}

} // namespace ZPTHUMB

/* end of file */
