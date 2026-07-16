/**
 * \file map_extender_mod.cpp
 * \brief See map_extender_mod.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
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

#include "map_extender_mod.h"

#include <cstring>

#include <nel/misc/common.h>

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_value.h"

using namespace PIPELINE::MAX;
using namespace NLMISC;

namespace MAPEXT {

const CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);

// Chunk ids inside the 0x2512 Mesh-like map-channel cache (corpus-constant functional set).
// The plugin writes a flattened Mesh-style stream for the single map channel it owns; only the
// four count/vert/face records are required for export. Remaining siblings (0x03ec edge flags,
// 0x03ed–0x03fa selection/named-sets/etc., 0x044c) are Mesh-format companions and are ignored.
enum
{
	CHUNK_MAP_VERT_COUNT = 0x03e8, // uint32 nVerts
	CHUNK_MAP_VERTS      = 0x03e9, // nVerts × Point3 (UVW)
	CHUNK_MAP_FACE_COUNT = 0x03ea, // uint32 nFaces
	CHUNK_MAP_FACES      = 0x03eb, // nFaces × (uint32 t0, t1, t2)
	CHUNK_MAP_CHANNEL    = 0x03f3  // uint32 channel index (1 or 2 in the corpus; default 1)
};

bool isMapExtenderModifier(CSceneClass *mod)
{
	if (!mod || !mod->classDesc()) return false;
	return mod->classDesc()->classId() == CLASSID_MAP_EXTENDER
	    && mod->classDesc()->superClassId() == SCLASS_OSMODIFIER;
}

// ---------------------------------------------------------------------------------------------
// Raw Max chunk-stream walker over a byte buffer (0x2512 is almost always a LEAF whose payload
// is still chunk-formatted — the container bit is clear in every corpus instance surveyed).

struct SChunkView
{
	uint16 Id;
	bool Container;
	const uint8 *Data;
	uint32 Size;
};

static bool walkTopLevelChunks(const uint8 *p, const uint8 *end, std::vector<SChunkView> &out)
{
	out.clear();
	while (p + 6 <= end)
	{
		uint16 id = 0;
		uint32 size = 0;
		memcpy(&id, p, 2);
		memcpy(&size, p + 2, 4);
		uint hdr = 6;
		bool cont = false;
		if (size == 0)
		{
			// 64-bit size extension
			if (p + 14 > end) return false;
			uint64 size64 = 0;
			memcpy(&size64, p + 6, 8);
			cont = (size64 & ((uint64)1 << 63)) != 0;
			size64 &= ~((uint64)1 << 63);
			if (size64 > 0x7FFFFFFFull) return false;
			size = (uint32)size64;
			hdr = 14;
		}
		else
		{
			cont = (size & 0x80000000u) != 0;
			size &= 0x7FFFFFFFu;
		}
		if (size < hdr || p + size > end) return false;
		SChunkView cv;
		cv.Id = id;
		cv.Container = cont;
		cv.Data = p + hdr;
		cv.Size = size - hdr;
		out.push_back(cv);
		p += size;
	}
	return p == end || p < end; // trailing padding is tolerated (none observed)
}

static const SChunkView *findChunk(const std::vector<SChunkView> &chunks, uint16 id)
{
	for (uint i = 0; i < chunks.size(); ++i)
		if (chunks[i].Id == id) return &chunks[i];
	return NULL;
}

// Collect top-level children of 0x2512 whether it is a typed container or a raw leaf.
static bool load2512Children(CStorageContainer *modApp, std::vector<SChunkView> &out,
                             std::vector<uint8> &ownedRaw, std::string *err)
{
	out.clear();
	ownedRaw.clear();
	if (!modApp)
	{
		if (err) *err = "null modApp";
		return false;
	}

	// Prefer a direct 0x2512 child of the 0x2500 app. Some call sites hand the OSM wrapper
	// (with 0x2500 still nested) — fall through like PHYSIQUESKIN.
	CStorageContainer *app = modApp;
	CStorageContainer *c2512 = NULL;
	CStorageRaw *raw2512 = NULL;
	for (CStorageContainer::TStorageObjectConstIt it = app->chunks().begin();
	     it != app->chunks().end(); ++it)
	{
		if (it->first == 0x2512)
		{
			c2512 = dynamic_cast<CStorageContainer *>(it->second);
			raw2512 = dynamic_cast<CStorageRaw *>(it->second);
			break;
		}
		if (it->first == 0x2500)
		{
			CStorageContainer *nested = dynamic_cast<CStorageContainer *>(it->second);
			if (nested)
			{
				for (CStorageContainer::TStorageObjectConstIt jt = nested->chunks().begin();
				     jt != nested->chunks().end(); ++jt)
				{
					if (jt->first != 0x2512) continue;
					c2512 = dynamic_cast<CStorageContainer *>(jt->second);
					raw2512 = dynamic_cast<CStorageRaw *>(jt->second);
					break;
				}
			}
		}
	}

	if (c2512)
	{
		// Container form (rewrite_assets also handles both): children are CStorageRaw leaves
		// holding the payload bytes. Typed CStorageValue<uint32> is accepted for the count
		// chunks if a future typed parse claims them.
		// First pass: collect raws and size typed values so we can pack ownedRaw stably.
		std::vector<std::pair<uint16, uint32> > typedU32;
		for (CStorageContainer::TStorageObjectConstIt it = c2512->chunks().begin();
		     it != c2512->chunks().end(); ++it)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (raw)
			{
				SChunkView cv;
				cv.Id = it->first;
				cv.Container = false;
				cv.Data = (const uint8 *)nlVectorData(raw->Value);
				cv.Size = (uint32)raw->Value.size();
				out.push_back(cv);
				continue;
			}
			CStorageValue<uint32> *u32 = dynamic_cast<CStorageValue<uint32> *>(it->second);
			if (u32)
				typedU32.push_back(std::make_pair(it->first, u32->Value));
		}
		if (!typedU32.empty())
		{
			ownedRaw.resize(typedU32.size() * 4);
			for (uint i = 0; i < typedU32.size(); ++i)
			{
				memcpy(&ownedRaw[i * 4], &typedU32[i].second, 4);
				SChunkView cv;
				cv.Id = typedU32[i].first;
				cv.Container = false;
				cv.Data = &ownedRaw[i * 4];
				cv.Size = 4;
				out.push_back(cv);
			}
		}
		if (!out.empty()) return true;
	}

	if (!raw2512 || raw2512->Value.empty())
	{
		if (err) *err = "Map Extender mod-app missing 0x2512 cache";
		return false;
	}

	const uint8 *p = (const uint8 *)nlVectorData(raw2512->Value);
	const uint8 *end = p + raw2512->Value.size();
	if (!walkTopLevelChunks(p, end, out))
	{
		if (err) *err = "Map Extender 0x2512 is not a well-formed chunk stream";
		return false;
	}
	return !out.empty();
}

static bool decodeFromChildren(const std::vector<SChunkView> &kids, SMapChannel &out, std::string *err)
{
	const SChunkView *cVerts = findChunk(kids, CHUNK_MAP_VERT_COUNT);
	const SChunkView *verts = findChunk(kids, CHUNK_MAP_VERTS);
	const SChunkView *cFaces = findChunk(kids, CHUNK_MAP_FACE_COUNT);
	const SChunkView *faces = findChunk(kids, CHUNK_MAP_FACES);
	if (!cVerts || !verts || !cFaces || !faces)
	{
		if (err) *err = "Map Extender cache missing 0x03e8/0x03e9/0x03ea/0x03eb";
		return false;
	}
	if (cVerts->Size < 4 || cFaces->Size < 4)
	{
		if (err) *err = "Map Extender count chunks too small";
		return false;
	}
	uint32 nVerts = 0, nFaces = 0;
	memcpy(&nVerts, cVerts->Data, 4);
	memcpy(&nFaces, cFaces->Data, 4);
	if (verts->Size != nVerts * 12u)
	{
		if (err) *err = "Map Extender 0x03e9 size mismatch vs 0x03e8 count";
		return false;
	}
	if (faces->Size != nFaces * 12u)
	{
		if (err) *err = "Map Extender 0x03eb size mismatch vs 0x03ea count";
		return false;
	}

	int channel = 1;
	const SChunkView *ch = findChunk(kids, CHUNK_MAP_CHANNEL);
	if (ch && ch->Size >= 4)
	{
		uint32 c = 0;
		memcpy(&c, ch->Data, 4);
		channel = (int)c;
	}

	out.Channel = channel;
	out.UVs.resize(nVerts);
	if (nVerts)
		memcpy(&out.UVs[0], verts->Data, (size_t)nVerts * 12);
	out.FaceUVs.resize((size_t)nFaces * 3);
	if (nFaces)
		memcpy(&out.FaceUVs[0], faces->Data, (size_t)nFaces * 12);

	// Validate face indices reference the UV array.
	for (uint32 i = 0; i < nFaces * 3; ++i)
	{
		if (out.FaceUVs[i] >= nVerts)
		{
			if (err) *err = "Map Extender face index out of range";
			return false;
		}
	}
	return true;
}

bool readMapExtenderCache(CStorageContainer *modApp, SMapChannel &out, std::string *err)
{
	out = SMapChannel();
	std::vector<SChunkView> kids;
	std::vector<uint8> owned;
	if (!load2512Children(modApp, kids, owned, err))
		return false;
	return decodeFromChildren(kids, out, err);
}

bool applyMapExtender(CSceneClass *mod, CStorageContainer *modApp, uint currentFaceCount,
                      int &outChannel, std::vector<CVector> &outUVs, std::vector<uint32> &outFaceUVs,
                      std::string *err)
{
	outChannel = 1;
	outUVs.clear();
	outFaceUVs.clear();
	if (mod && !isMapExtenderModifier(mod))
	{
		if (err) *err = "not a Map Extender modifier";
		return false;
	}
	SMapChannel ch;
	if (!readMapExtenderCache(modApp, ch, err))
		return false;
	if (ch.numFaces() != currentFaceCount)
	{
		if (err)
			*err = toString("Map Extender cache face count %u != mesh face count %u",
			                ch.numFaces(), currentFaceCount);
		return false;
	}
	outChannel = ch.Channel;
	outUVs.swap(ch.UVs);
	outFaceUVs.swap(ch.FaceUVs);
	return true;
}

} /* namespace MAPEXT */

/* end of file */
