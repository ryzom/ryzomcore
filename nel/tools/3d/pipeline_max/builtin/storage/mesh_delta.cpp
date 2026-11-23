/**
 * \file mesh_delta.cpp
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

#include <nel/misc/types_nl.h>
#include "mesh_delta.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

// The 0x2512 LocalModData children (modern MDELTA format).
#define PMB_MD_FLAGS_CHUNK_ID 0x2740
#define PMB_MD_VSELSET_CHUNK_ID 0x2845
#define PMB_MD_FSELSET_CHUNK_ID 0x2846
#define PMB_MD_ESELSET_CHUNK_ID 0x2847
#define PMB_MD_MDELTA_CHUNK_ID 0x4000
// The 0x4000 MDELTA children (per-record delta chunks).
#define PMB_MD_INPUT_VERTS_CHUNK_ID 0x0100
#define PMB_MD_INPUT_FACES_CHUNK_ID 0x0110
#define PMB_MD_CREATED_VERTS_CHUNK_ID 0x0130
#define PMB_MD_MOVES_CHUNK_ID 0x0140
#define PMB_MD_DEL_VERTS_CHUNK_ID 0x0170
#define PMB_MD_CREATED_FACES_CHUNK_ID 0x0208
#define PMB_MD_FACE_REMAP_CHUNK_ID 0x0210
#define PMB_MD_FACE_ATTRIBS_CHUNK_ID 0x0220
#define PMB_MD_FACE_SMOOTH_CHUNK_ID 0x0230
#define PMB_MD_DEL_FACES_CHUNK_ID 0x0270
#define PMB_MD_MAP_COUNT_CHUNK_ID 0x0300
// One map channel group: 0x0320 opens it, the others belong to the last opened group.
#define PMB_MD_MAP_CHANNEL_CHUNK_ID 0x0320
#define PMB_MD_MAP_INPUT_VERTS_CHUNK_ID 0x0324
#define PMB_MD_MAP_INPUT_FACES_CHUNK_ID 0x0328
#define PMB_MD_MAP_VERT_SETS_CHUNK_ID 0x0330
#define PMB_MD_MAP_CREATED_VERTS_CHUNK_ID 0x0334
#define PMB_MD_MAP_CREATED_FACES_CHUNK_ID 0x0338
#define PMB_MD_MAP_FACE_REMAP_CHUNK_ID 0x033b
#define PMB_MD_SEL_VERTS_CHUNK_ID 0x0400
#define PMB_MD_SEL_EDGES_CHUNK_ID 0x0410 // 3 bits per output face (zo_cn_mairie.max: 189, 510, 60 bits)
#define PMB_MD_SEL_FACES_CHUNK_ID 0x0420 // 1 bit per output face
#define PMB_MD_SEL_UNKNOWN430_CHUNK_ID 0x0430
// The bit-array leaf inside 0x0170/0x0270/0x04x0 containers.
#define PMB_MD_BIT_ARRAY_CHUNK_ID 0x2700

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

void CMeshDelta::SBitArray::bits(std::vector<bool> &out) const
{
	out.resize(BitCount);
	for (uint32 i = 0; i < BitCount; ++i)
		out[i] = ((i / 8) < Packed.size()) && ((Packed[i / 8] >> (i % 8)) & 1);
}

CMeshDelta::CMeshDelta()
{
	clear();
}

void CMeshDelta::clear()
{
	m_Moves.clear();
	m_CreatedVerts.clear();
	m_CreatedFaces.clear();
	m_FaceRemap.clear();
	m_FaceAttribs.clear();
	m_FaceSmooth.clear();
	m_MapDeltas.clear();
	m_DelVerts = SBitArray();
	m_DelFaces = SBitArray();
	m_HasInputVertCount = m_HasInputFaceCount = m_HasMapCount = false;
	m_InputVertCount = m_InputFaceCount = m_MapCount = 0;
	m_LocalDataChildren.clear();
	m_DeltaChildren.clear();
	m_UnknownLocalDataIds.clear();
	m_UnknownDeltaIds.clear();
	m_IrregularIds.clear();
	m_ExtraDeltaContainers = 0;
	m_Segments.clear();
}

static void noteId(std::vector<uint16> &ids, uint16 id)
{
	for (std::vector<uint16>::const_iterator it = ids.begin(); it != ids.end(); ++it)
		if (*it == id) return;
	ids.push_back(id);
}

static void noteChunk(std::vector<CMeshDelta::SChunkNote> &notes, uint16 id, IStorageObject *obj)
{
	CMeshDelta::SChunkNote n;
	n.Id = id;
	n.Container = obj && obj->isContainer();
	const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(obj);
	n.Size = raw ? (uint32)raw->Value.size() : 0;
	notes.push_back(n);
}

// Decode one count-prefixed record table: uint32 count + count x stride bytes, size EXACT (a
// trailing-byte surplus would silently break the re-encode proof, so it demotes to irregular).
bool CMeshDelta::decodeRows(uint16 id, IStorageObject *obj, uint stride, uint kind)
{
	const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(obj);
	if (!raw || raw->Value.size() < 4) return false;
	uint32 n;
	memcpy(&n, nlVectorData(raw->Value), 4);
	if (raw->Value.size() != 4 + (size_t)n * stride) return false;
	const uint8 *p = nlVectorData(raw->Value) + 4;
	SSegment seg;
	seg.Id = id;
	seg.kind = kind;
	seg.Raw = raw;
	seg.Rows = n;
	seg.Value = 0;
	seg.Bits = nullptr;
	seg.MapIdx = 0;
	if (kind >= 8 && kind <= 11)
	{
		// a map channel record table outside a 0x0320 group
		if (m_MapDeltas.empty()) return false;
		seg.MapIdx = m_MapDeltas.size() - 1;
	}
	switch (kind)
	{
	case 0:
		seg.Row0 = m_Moves.size();
		m_Moves.resize(m_Moves.size() + n);
		if (n) memcpy(&m_Moves[seg.Row0], p, (size_t)n * stride);
		break;
	case 1:
		seg.Row0 = m_CreatedVerts.size();
		m_CreatedVerts.resize(m_CreatedVerts.size() + n);
		if (n) memcpy(&m_CreatedVerts[seg.Row0], p, (size_t)n * stride);
		break;
	case 2:
		seg.Row0 = m_CreatedFaces.size();
		m_CreatedFaces.resize(m_CreatedFaces.size() + n);
		if (n) memcpy(&m_CreatedFaces[seg.Row0], p, (size_t)n * stride);
		break;
	case 3:
		seg.Row0 = m_FaceRemap.size();
		m_FaceRemap.resize(m_FaceRemap.size() + n);
		if (n) memcpy(&m_FaceRemap[seg.Row0], p, (size_t)n * stride);
		break;
	case 4:
		seg.Row0 = m_FaceAttribs.size();
		m_FaceAttribs.resize(m_FaceAttribs.size() + n);
		if (n) memcpy(&m_FaceAttribs[seg.Row0], p, (size_t)n * stride);
		break;
	case 7:
		seg.Row0 = m_FaceSmooth.size();
		m_FaceSmooth.resize(m_FaceSmooth.size() + n);
		if (n) memcpy(&m_FaceSmooth[seg.Row0], p, (size_t)n * stride);
		break;
	case 8:
	{
		std::vector<SMapVertSet> &t = m_MapDeltas[seg.MapIdx].VertSets;
		seg.Row0 = t.size();
		t.resize(t.size() + n);
		if (n) memcpy(&t[seg.Row0], p, (size_t)n * stride);
		break;
	}
	case 9:
	{
		std::vector<SMapVert> &t = m_MapDeltas[seg.MapIdx].CreatedVerts;
		seg.Row0 = t.size();
		t.resize(t.size() + n);
		if (n) memcpy(&t[seg.Row0], p, (size_t)n * stride);
		break;
	}
	case 10:
	{
		std::vector<SMapFace> &t = m_MapDeltas[seg.MapIdx].CreatedFaces;
		seg.Row0 = t.size();
		t.resize(t.size() + n);
		if (n) memcpy(&t[seg.Row0], p, (size_t)n * stride);
		break;
	}
	case 11:
	{
		std::vector<SFaceVertRemap> &t = m_MapDeltas[seg.MapIdx].FaceRemap;
		seg.Row0 = t.size();
		t.resize(t.size() + n);
		if (n) memcpy(&t[seg.Row0], p, (size_t)n * stride);
		break;
	}
	default:
		return false;
	}
	m_Segments.push_back(seg);
	return true;
}

bool CMeshDelta::decodeCountLeaf(uint16 id, IStorageObject *obj, uint32 &value, bool &present)
{
	const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(obj);
	if (!raw || raw->Value.size() != 4) return false;
	memcpy(&value, nlVectorData(raw->Value), 4);
	present = true;
	SSegment seg;
	seg.Id = id;
	seg.kind = 5;
	seg.Raw = raw;
	seg.Row0 = 0;
	seg.Rows = 0;
	seg.Value = value;
	seg.Bits = nullptr;
	seg.MapIdx = 0;
	m_Segments.push_back(seg);
	return true;
}

// Decode a delete-bitmap container: exactly one 0x2700 leaf, uint32 bitCount + at least
// ceil(bitCount / 8) packed bytes (all payload bytes after the count are retained verbatim).
bool CMeshDelta::decodeBitArray(uint16 id, IStorageObject *obj, SBitArray &out)
{
	const CStorageContainer *cont = dynamic_cast<const CStorageContainer *>(obj);
	if (!cont) return false;
	const CStorageRaw *bitsRaw = nullptr;
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != PMB_MD_BIT_ARRAY_CHUNK_ID) return false;
		if (bitsRaw) return false; // more than one 0x2700
		bitsRaw = dynamic_cast<const CStorageRaw *>(it->second);
		if (!bitsRaw) return false;
	}
	if (!bitsRaw || bitsRaw->Value.size() < 4) return false;
	uint32 n;
	memcpy(&n, nlVectorData(bitsRaw->Value), 4);
	if (bitsRaw->Value.size() < 4 + ((size_t)n + 7) / 8) return false;
	out.Present = true;
	out.BitCount = n;
	out.Packed.assign(bitsRaw->Value.begin() + 4, bitsRaw->Value.end());
	SSegment seg;
	seg.Id = id;
	seg.kind = 6;
	seg.Raw = bitsRaw;
	seg.Row0 = 0;
	seg.Rows = 0;
	seg.Value = n;
	seg.Bits = &out;
	seg.MapIdx = 0;
	m_Segments.push_back(seg);
	return true;
}

bool CMeshDelta::decode(IStorageObject *localModData)
{
	clear();
	CStorageContainer *c2512 = dynamic_cast<CStorageContainer *>(localModData);
	if (!c2512) return false;

	CStorageContainer *c4000 = nullptr;
	for (CStorageContainer::TStorageObjectConstIt it = c2512->chunks().begin(); it != c2512->chunks().end(); ++it)
	{
		noteChunk(m_LocalDataChildren, it->first, it->second);
		switch (it->first)
		{
		case PMB_MD_MDELTA_CHUNK_ID:
		{
			CStorageContainer *cont = dynamic_cast<CStorageContainer *>(it->second);
			if (!cont) { noteId(m_IrregularIds, it->first); break; }
			if (c4000) ++m_ExtraDeltaContainers;
			else c4000 = cont;
			break;
		}
		case PMB_MD_FLAGS_CHUNK_ID:
		case PMB_MD_VSELSET_CHUNK_ID:
		case PMB_MD_FSELSET_CHUNK_ID:
		case PMB_MD_ESELSET_CHUNK_ID:
			break; // recognized companions, kept raw
		default:
			noteId(m_UnknownLocalDataIds, it->first);
			break;
		}
	}
	if (!c4000) return false;

	for (CStorageContainer::TStorageObjectConstIt it = c4000->chunks().begin(); it != c4000->chunks().end(); ++it)
	{
		noteChunk(m_DeltaChildren, it->first, it->second);
		bool ok = true;
		switch (it->first)
		{
		case PMB_MD_MOVES_CHUNK_ID: ok = decodeRows(it->first, it->second, 16, 0); break;
		case PMB_MD_CREATED_VERTS_CHUNK_ID: ok = decodeRows(it->first, it->second, 16, 1); break;
		case PMB_MD_CREATED_FACES_CHUNK_ID: ok = decodeRows(it->first, it->second, 24, 2); break;
		case PMB_MD_FACE_REMAP_CHUNK_ID: ok = decodeRows(it->first, it->second, 20, 3); break;
		case PMB_MD_FACE_ATTRIBS_CHUNK_ID: ok = decodeRows(it->first, it->second, 12, 4); break;
		case PMB_MD_FACE_SMOOTH_CHUNK_ID: ok = decodeRows(it->first, it->second, 12, 7); break;
		case PMB_MD_INPUT_VERTS_CHUNK_ID: ok = decodeCountLeaf(it->first, it->second, m_InputVertCount, m_HasInputVertCount); break;
		case PMB_MD_INPUT_FACES_CHUNK_ID: ok = decodeCountLeaf(it->first, it->second, m_InputFaceCount, m_HasInputFaceCount); break;
		case PMB_MD_MAP_COUNT_CHUNK_ID: ok = decodeCountLeaf(it->first, it->second, m_MapCount, m_HasMapCount); break;
		case PMB_MD_DEL_VERTS_CHUNK_ID: ok = decodeBitArray(it->first, it->second, m_DelVerts); break;
		case PMB_MD_DEL_FACES_CHUNK_ID: ok = decodeBitArray(it->first, it->second, m_DelFaces); break;
		case PMB_MD_MAP_CHANNEL_CHUNK_ID:
		{
			// pushed even when the leaf is unreadable, so the records that follow do not land in the
			// previous group
			m_MapDeltas.push_back(SMapDelta());
			ok = decodeCountLeaf(it->first, it->second, m_MapDeltas.back().Channel, m_MapDeltas.back().HasChannel);
			break;
		}
		case PMB_MD_MAP_INPUT_VERTS_CHUNK_ID:
			ok = !m_MapDeltas.empty()
				&& decodeCountLeaf(it->first, it->second, m_MapDeltas.back().VertCount, m_MapDeltas.back().HasVertCount);
			break;
		case PMB_MD_MAP_INPUT_FACES_CHUNK_ID:
			ok = !m_MapDeltas.empty()
				&& decodeCountLeaf(it->first, it->second, m_MapDeltas.back().FaceCount, m_MapDeltas.back().HasFaceCount);
			break;
		case PMB_MD_MAP_VERT_SETS_CHUNK_ID: ok = decodeRows(it->first, it->second, 16, 8); break;
		case PMB_MD_MAP_CREATED_VERTS_CHUNK_ID: ok = decodeRows(it->first, it->second, 12, 9); break;
		case PMB_MD_MAP_CREATED_FACES_CHUNK_ID: ok = decodeRows(it->first, it->second, 12, 10); break;
		case PMB_MD_MAP_FACE_REMAP_CHUNK_ID: ok = decodeRows(it->first, it->second, 20, 11); break;
		case PMB_MD_SEL_VERTS_CHUNK_ID:
		case PMB_MD_SEL_FACES_CHUNK_ID:
		case PMB_MD_SEL_EDGES_CHUNK_ID:
		case PMB_MD_SEL_UNKNOWN430_CHUNK_ID:
			break; // recognized selection state, kept raw (ignored on evaluation)
		// Recognized but untyped; kept raw, ignored on evaluation. 0x0340 is a container holding
		// one tiny 0x2700 bit array (8 B corpus-wide); 0x0120 (52 B) and 0x0200 (44 B) are
		// single-instance; 0x0360 (5 instances) is not count-prefixed at any common stride.
		case 0x0120:
		case 0x0200:
		case 0x0340:
		case 0x0360:
			break;
		default:
			noteId(m_UnknownDeltaIds, it->first);
			break;
		}
		if (!ok) noteId(m_IrregularIds, it->first);
	}
	return true;
}

bool CMeshDelta::selfTestReencode(std::string &err) const
{
	for (std::vector<SSegment>::const_iterator it = m_Segments.begin(); it != m_Segments.end(); ++it)
	{
		const SSegment &seg = *it;
		std::vector<uint8> enc;
		if (seg.kind <= 4 || (seg.kind >= 7 && seg.kind <= 11))
		{
			uint stride = 0;
			const uint8 *rows = nullptr;
			switch (seg.kind)
			{
			case 0: stride = 16; rows = seg.Rows ? (const uint8 *)&m_Moves[seg.Row0] : nullptr; break;
			case 1: stride = 16; rows = seg.Rows ? (const uint8 *)&m_CreatedVerts[seg.Row0] : nullptr; break;
			case 2: stride = 24; rows = seg.Rows ? (const uint8 *)&m_CreatedFaces[seg.Row0] : nullptr; break;
			case 3: stride = 20; rows = seg.Rows ? (const uint8 *)&m_FaceRemap[seg.Row0] : nullptr; break;
			case 4: stride = 12; rows = seg.Rows ? (const uint8 *)&m_FaceAttribs[seg.Row0] : nullptr; break;
			case 7: stride = 12; rows = seg.Rows ? (const uint8 *)&m_FaceSmooth[seg.Row0] : nullptr; break;
			case 8: stride = 16; rows = seg.Rows ? (const uint8 *)&m_MapDeltas[seg.MapIdx].VertSets[seg.Row0] : nullptr; break;
			case 9: stride = 12; rows = seg.Rows ? (const uint8 *)&m_MapDeltas[seg.MapIdx].CreatedVerts[seg.Row0] : nullptr; break;
			case 10: stride = 12; rows = seg.Rows ? (const uint8 *)&m_MapDeltas[seg.MapIdx].CreatedFaces[seg.Row0] : nullptr; break;
			case 11: stride = 20; rows = seg.Rows ? (const uint8 *)&m_MapDeltas[seg.MapIdx].FaceRemap[seg.Row0] : nullptr; break;
			}
			enc.resize(4 + seg.Rows * stride);
			uint32 n = (uint32)seg.Rows;
			memcpy(nlVectorData(enc), &n, 4);
			if (rows) memcpy(nlVectorData(enc) + 4, rows, seg.Rows * stride);
		}
		else if (seg.kind == 5)
		{
			enc.resize(4);
			memcpy(nlVectorData(enc), &seg.Value, 4);
		}
		else // kind 6: bit array
		{
			enc.resize(4 + seg.Bits->Packed.size());
			memcpy(nlVectorData(enc), &seg.Bits->BitCount, 4);
			if (!seg.Bits->Packed.empty())
				memcpy(nlVectorData(enc) + 4, nlVectorData(seg.Bits->Packed), seg.Bits->Packed.size());
		}
		if (enc != seg.Raw->Value)
		{
			std::stringstream ss;
			ss << "chunk 0x" << std::hex << seg.Id << std::dec << " re-encode mismatch ("
			   << enc.size() << " vs " << seg.Raw->Value.size() << " bytes)";
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
