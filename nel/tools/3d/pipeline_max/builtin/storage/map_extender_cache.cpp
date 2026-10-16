/**
 * \file map_extender_cache.cpp
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

#include <nel/misc/types_nl.h>
#include "map_extender_cache.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

// Functional chunk set (Part P.3).
#define PMB_MX_VERT_COUNT_CHUNK_ID 0x03e8
#define PMB_MX_VERTS_CHUNK_ID 0x03e9
#define PMB_MX_FACE_COUNT_CHUNK_ID 0x03ea
#define PMB_MX_FACES_CHUNK_ID 0x03eb
#define PMB_MX_CHANNEL_CHUNK_ID 0x03f3

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

const NLMISC::CClassId CMapExtenderCache::ModifierClassId = NLMISC::CClassId(0x2ec82081, 0x045a6271);

// The full Part P.3 known-id table — anything else is surfaced through unknownIds().
static bool mxKnownId(uint16 id)
{
	if (id >= 0x03e8 && id <= 0x03fa) return id != 0x03f6; // 0x03f6 never observed
	return id == 0x044c;
}

CMapExtenderCache::CMapExtenderCache()
{
	clear();
}

void CMapExtenderCache::clear()
{
	m_LeafForm = false;
	m_EmptyLeaf = false;
	m_WalkComplete = false;
	m_NumVerts = m_NumFaces = 0;
	m_Channel = 1;
	m_HasChannel = false;
	m_UVWords.clear();
	m_FaceCorners.clear();
	m_Views.clear();
	m_Children.clear();
	m_UnknownIds.clear();
	m_Error.clear();
}

static void mxNoteId(std::vector<uint16> &ids, uint16 id)
{
	for (std::vector<uint16>::const_iterator it = ids.begin(); it != ids.end(); ++it)
		if (*it == id) return;
	ids.push_back(id);
}

// Walk the raw-leaf form's payload as a chunk stream (32-bit headers, container bit in the
// size dword's MSB, 64-bit size extension when size32 == 0 — the same framing CStorageChunks
// reads, re-implemented over a flat byte view because the leaf payload was never parsed as
// chunks by the storage layer).
bool CMapExtenderCache::walkLeaf(const uint8 *p, const uint8 *end)
{
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
		if (size < hdr || (size_t)(end - p) < size) return false;
		SView v;
		v.Id = id;
		v.Data = p + hdr;
		v.Size = size - hdr;
		m_Views.push_back(v);
		SChunkNote n;
		n.Id = id;
		n.Size = v.Size;
		n.Container = cont;
		m_Children.push_back(n);
		if (!mxKnownId(id)) mxNoteId(m_UnknownIds, id);
		p += size;
	}
	return p == end;
}

const CMapExtenderCache::SView *CMapExtenderCache::findView(uint16 id) const
{
	for (std::vector<SView>::const_iterator it = m_Views.begin(); it != m_Views.end(); ++it)
		if (it->Id == id) return &*it;
	return nullptr;
}

bool CMapExtenderCache::decodeFunctional()
{
	const SView *cVerts = findView(PMB_MX_VERT_COUNT_CHUNK_ID);
	const SView *verts = findView(PMB_MX_VERTS_CHUNK_ID);
	const SView *cFaces = findView(PMB_MX_FACE_COUNT_CHUNK_ID);
	const SView *faces = findView(PMB_MX_FACES_CHUNK_ID);
	if (!cVerts || !verts || !cFaces || !faces)
	{
		m_Error = "Map Extender cache missing 0x03e8/0x03e9/0x03ea/0x03eb";
		return false;
	}
	if (cVerts->Size != 4 || cFaces->Size != 4)
	{
		m_Error = "Map Extender count chunks are not 4 bytes";
		return false;
	}
	memcpy(&m_NumVerts, cVerts->Data, 4);
	memcpy(&m_NumFaces, cFaces->Data, 4);
	if (verts->Size != m_NumVerts * 12u)
	{
		m_Error = "Map Extender 0x03e9 size mismatch vs 0x03e8 count";
		return false;
	}
	if (faces->Size != m_NumFaces * 12u)
	{
		m_Error = "Map Extender 0x03eb size mismatch vs 0x03ea count";
		return false;
	}
	m_UVWords.resize((size_t)m_NumVerts * 3);
	if (m_NumVerts)
		memcpy(nlVectorData(m_UVWords), verts->Data, (size_t)m_NumVerts * 12);
	m_FaceCorners.resize((size_t)m_NumFaces * 3);
	if (m_NumFaces)
		memcpy(nlVectorData(m_FaceCorners), faces->Data, (size_t)m_NumFaces * 12);
	const SView *ch = findView(PMB_MX_CHANNEL_CHUNK_ID);
	if (ch && ch->Size == 4)
	{
		uint32 c = 0;
		memcpy(&c, ch->Data, 4);
		m_Channel = (sint)c;
		m_HasChannel = true;
	}
	return true;
}

bool CMapExtenderCache::decode(IStorageObject *localModData)
{
	clear();
	if (!localModData)
	{
		m_Error = "no 0x2512 LocalModData";
		return false;
	}
	CStorageContainer *cont = dynamic_cast<CStorageContainer *>(localModData);
	if (cont)
	{
		// Container form: children are raw leaves (payload bytes) or sub-containers
		// (0x03f9/0x03fa; no functional data, inventoried only).
		for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
		{
			SChunkNote n;
			n.Id = it->first;
			n.Container = it->second && it->second->isContainer();
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			n.Size = raw ? (uint32)raw->Value.size() : 0;
			m_Children.push_back(n);
			if (!mxKnownId(it->first)) mxNoteId(m_UnknownIds, it->first);
			if (raw)
			{
				SView v;
				v.Id = it->first;
				v.Data = nlVectorData(raw->Value);
				v.Size = (uint32)raw->Value.size();
				m_Views.push_back(v);
			}
		}
		return decodeFunctional();
	}
	const CStorageRaw *leaf = dynamic_cast<const CStorageRaw *>(localModData);
	if (!leaf)
	{
		m_Error = "Map Extender 0x2512 is neither a container nor a raw leaf";
		return false;
	}
	if (leaf->Value.empty())
	{
		// Corpus-witnessed: a handful of instances (collision work files) carry an EMPTY
		// 0x2512 leaf — the modifier never evaluated, no cache was saved.
		m_EmptyLeaf = true;
		m_Error = "Map Extender 0x2512 cache is empty (modifier never evaluated)";
		return false;
	}
	m_LeafForm = true;
	const uint8 *p = nlVectorData(leaf->Value);
	if (!walkLeaf(p, p + leaf->Value.size()))
	{
		m_Error = "Map Extender 0x2512 leaf is not a well-formed chunk stream";
		return false;
	}
	m_WalkComplete = true;
	return decodeFunctional();
}

bool CMapExtenderCache::faceCornersValid() const
{
	for (std::vector<uint32>::const_iterator it = m_FaceCorners.begin(); it != m_FaceCorners.end(); ++it)
		if (*it >= m_NumVerts) return false;
	return true;
}

bool CMapExtenderCache::selfTestReencode(std::string &err) const
{
	if (m_LeafForm && !m_WalkComplete)
	{
		err = "leaf walk did not cover the whole 0x2512 payload";
		return false;
	}
	for (uint i = 0; i < 5; ++i)
	{
		static const uint16 ids[5] = {
			PMB_MX_VERT_COUNT_CHUNK_ID, PMB_MX_VERTS_CHUNK_ID, PMB_MX_FACE_COUNT_CHUNK_ID,
			PMB_MX_FACES_CHUNK_ID, PMB_MX_CHANNEL_CHUNK_ID
		};
		const SView *v = findView(ids[i]);
		if (!v)
		{
			if (ids[i] == PMB_MX_CHANNEL_CHUNK_ID && !m_HasChannel) continue; // optional
			std::stringstream ss;
			ss << "functional chunk 0x" << std::hex << ids[i] << " lost";
			err = ss.str();
			return false;
		}
		std::vector<uint8> enc;
		switch (ids[i])
		{
		case PMB_MX_VERT_COUNT_CHUNK_ID:
			enc.resize(4);
			memcpy(nlVectorData(enc), &m_NumVerts, 4);
			break;
		case PMB_MX_FACE_COUNT_CHUNK_ID:
			enc.resize(4);
			memcpy(nlVectorData(enc), &m_NumFaces, 4);
			break;
		case PMB_MX_VERTS_CHUNK_ID:
			enc.resize(m_UVWords.size() * 4);
			if (!m_UVWords.empty())
				memcpy(nlVectorData(enc), nlVectorData(m_UVWords), enc.size());
			break;
		case PMB_MX_FACES_CHUNK_ID:
			enc.resize(m_FaceCorners.size() * 4);
			if (!m_FaceCorners.empty())
				memcpy(nlVectorData(enc), nlVectorData(m_FaceCorners), enc.size());
			break;
		case PMB_MX_CHANNEL_CHUNK_ID:
		{
			if (!m_HasChannel) continue; // present but not 4 bytes never happens (decode gates on == 4)
			enc.resize(4);
			uint32 c = (uint32)m_Channel;
			memcpy(nlVectorData(enc), &c, 4);
			break;
		}
		}
		if (enc.size() != v->Size || (v->Size && memcmp(nlVectorData(enc), v->Data, v->Size) != 0))
		{
			std::stringstream ss;
			ss << "chunk 0x" << std::hex << ids[i] << std::dec << " re-encode mismatch ("
			   << enc.size() << " vs " << v->Size << " bytes)";
			err = ss.str();
			return false;
		}
	}
	return true;
}

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
