/**
 * \file edit_mesh_mod.cpp
 * \brief See edit_mesh_mod.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#include "edit_mesh_mod.h"

#include <cstring>

using namespace PIPELINE::MAX;

namespace EDITMESH {

// Read a 0x2700 bit-array container (uint32 bit-count + LSB-first packed bits, dword padded) into
// a caller-supplied bool vector. Silent no-op on any structural surprise (returns the vector
// untouched) — matches the ig decode's failure discipline (raw chunk survives regardless).
static bool readBitArray(CStorageContainer *cont, std::vector<bool> &out)
{
	if (!cont) return false;
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != 0x2700) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 4) return false;
		uint32 n;
		memcpy(&n, nlVectorData(raw->Value), 4);
		if (raw->Value.size() < 4 + ((size_t)n + 7) / 8) return false;
		out.resize(n);
		for (uint32 i = 0; i < n; ++i)
			out[i] = (raw->Value[4 + i / 8] >> (i % 8)) & 1;
		return true;
	}
	return false;
}

bool readModApp(CStorageContainer *c2500, SEdits &out)
{
	if (!c2500) return false;
	for (CStorageContainer::TStorageObjectConstIt it = c2500->chunks().begin(); it != c2500->chunks().end(); ++it)
	{
		if (it->first != 0x2512) continue;
		CStorageContainer *c2512 = dynamic_cast<CStorageContainer *>(it->second);
		if (!c2512) continue;
		for (CStorageContainer::TStorageObjectConstIt jt = c2512->chunks().begin(); jt != c2512->chunks().end(); ++jt)
		{
			if (jt->first != 0x4000) continue;
			CStorageContainer *c4000 = dynamic_cast<CStorageContainer *>(jt->second);
			if (!c4000) continue;
			for (CStorageContainer::TStorageObjectConstIt kt = c4000->chunks().begin(); kt != c4000->chunks().end(); ++kt)
			{
				if (kt->first == 0x0140)
				{
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (!raw || raw->Value.size() < 4) continue;
					uint32 n;
					memcpy(&n, nlVectorData(raw->Value), 4);
					if (raw->Value.size() < 4 + (size_t)n * 16) continue;
					out.Moves.reserve(out.Moves.size() + n);
					for (uint32 i = 0; i < n; ++i)
					{
						uint32 idx;
						float v[3];
						memcpy(&idx, nlVectorData(raw->Value) + 4 + i * 16, 4);
						memcpy(v, nlVectorData(raw->Value) + 4 + i * 16 + 4, 12);
						out.Moves.push_back(std::make_pair(idx, NLMISC::CVector(v[0], v[1], v[2])));
					}
				}
				else if (kt->first == 0x0130)
				{
					// Created verts: `uint32 count + (uint32 srcTag, Point3 pos)[count]`, 16-byte
					// stride. srcTag = Max's cloned-from-vert authoring history; ignored — the
					// geometric position is what evaluation needs. Design-doc §10w.
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (!raw || raw->Value.size() < 4) continue;
					uint32 n;
					memcpy(&n, nlVectorData(raw->Value), 4);
					if (raw->Value.size() < 4 + (size_t)n * 16) continue;
					out.CreatedVerts.reserve(out.CreatedVerts.size() + n);
					for (uint32 i = 0; i < n; ++i)
					{
						float v[3];
						memcpy(v, nlVectorData(raw->Value) + 4 + (size_t)i * 16 + 4, 12);
						out.CreatedVerts.push_back(NLMISC::CVector(v[0], v[1], v[2]));
					}
				}
				else if (kt->first == 0x0208)
				{
					// Created faces variant A: `uint32 count + (uint32 srcTag, uint32 v[3],
					// uint32 smGrp, uint32 flagsMatId)[count]`, 24-byte stride. srcTag ignored,
					// same reasoning as 0x0130. This is the record the corpus proves the
					// reference plugin actually consumes (see the SEdits header comment for the
					// fy_hall_reunion face-count match).
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (!raw || raw->Value.size() < 4) continue;
					uint32 n;
					memcpy(&n, nlVectorData(raw->Value), 4);
					if (raw->Value.size() < 4 + (size_t)n * 24) continue;
					out.CreatedFacesA.reserve(out.CreatedFacesA.size() + n);
					for (uint32 i = 0; i < n; ++i)
					{
						const uint8 *p = nlVectorData(raw->Value) + 4 + (size_t)i * 24;
						SFace f;
						memcpy(f.V, p + 4, 12);
						memcpy(&f.SmGroup, p + 16, 4);
						memcpy(&f.FaceFlags, p + 20, 4);
						out.CreatedFacesA.push_back(f);
					}
				}
				else if (kt->first == 0x0210)
				{
					// Face-vertex remap (modern equivalent of legacy TOPO_FACEMAP_CHUNK 0x2780):
					// `uint32 count + (uint32 faceIdx, uint32 applyMask, uint32 v[3])[count]`,
					// 20-byte stride. ApplyMask bits 0..2 select which of face `faceIdx`'s
					// corners get replaced with the corresponding v[i]. Corpus-validated across
					// 445 files / 113 chunks / 2881 entries: every observed mask is 0..7 (0 =
					// no-op remap on a face that's about to be deleted; 3 = most common; 7 =
					// full replacement, e.g. fy_hall_reunion face 18 → (76, 81, 74) matching the
					// reference `.cmb` exactly). Corners not covered by mask carry undefined
					// bytes in the writer and must be ignored — see SFaceVertRemap::applyCorner.
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (!raw || raw->Value.size() < 4) continue;
					uint32 n;
					memcpy(&n, nlVectorData(raw->Value), 4);
					if (raw->Value.size() < 4 + (size_t)n * 20) continue;
					out.FaceRemap.reserve(out.FaceRemap.size() + n);
					for (uint32 i = 0; i < n; ++i)
					{
						const uint8 *p = nlVectorData(raw->Value) + 4 + (size_t)i * 20;
						SFaceVertRemap r;
						memcpy(&r.Index, p + 0, 4);
						memcpy(&r.ApplyMask, p + 4, 4);
						memcpy(r.V, p + 8, 12);
						out.FaceRemap.push_back(r);
					}
				}
				else if (kt->first == 0x0220)
				{
					// Per-face attribute changes: `uint32 count + (uint32 idx, uint32 apply,
					// uint32 values)[count]`, 12-byte stride. Modern-format counterpart of
					// LEGACY TOPO_ATTRIBS_CHUNK — see edit_mesh_mod.h SFaceAttribChange for the
					// bit layouts. Pinned by the fy_hall_reunion corpus match: 82 entries (=
					// input face count), most with ApplyMask=0x10 and Values where
					// (Values>>5)&0xFFFF exactly reproduces the reference `.cmb`'s per-face
					// matID column (69 matID-60 faces, 48 matID-59 etc. — the raw base mesh has
					// matID 0 everywhere; 0x0220 is what promotes them).
					CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
					if (!raw || raw->Value.size() < 4) continue;
					uint32 n;
					memcpy(&n, nlVectorData(raw->Value), 4);
					if (raw->Value.size() < 4 + (size_t)n * 12) continue;
					out.FaceAttribs.reserve(out.FaceAttribs.size() + n);
					for (uint32 i = 0; i < n; ++i)
					{
						const uint8 *p = nlVectorData(raw->Value) + 4 + (size_t)i * 12;
						SFaceAttribChange fa;
						memcpy(&fa.Index, p + 0, 4);
						memcpy(&fa.ApplyMask, p + 4, 4);
						memcpy(&fa.Values, p + 8, 4);
						out.FaceAttribs.push_back(fa);
					}
				}
				else if (kt->first == 0x0170)
					readBitArray(dynamic_cast<CStorageContainer *>(kt->second), out.DelVerts);
				else if (kt->first == 0x0270)
					readBitArray(dynamic_cast<CStorageContainer *>(kt->second), out.DelFaces);
			}
			return true;
		}
	}
	return false;
}

} /* namespace EDITMESH */

/* end of file */
