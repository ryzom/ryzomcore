/**
 * \file rpo_data.cpp
 * \brief RPO / NeL patch mesh data decoding
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * See rpo_data.h.
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
#include "rpo_data.h"

// STL includes
#include <cstring>

// Project includes

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

// ---------------------------------------------------------------------------------------------
// Sequential byte reader over a raw blob.

namespace {

struct CBlobReader
{
	const uint8 *Data;
	size_t Size;
	size_t Pos;
	CBlobReader(const uint8 *data, size_t size) : Data(data), Size(size), Pos(0) { }
	bool read(void *dst, size_t n)
	{
		if (Pos + n > Size) return false;
		memcpy(dst, Data + Pos, n);
		Pos += n;
		return true;
	}
	template <typename T>
	bool value(T &v) { return read(&v, sizeof(T)); }
};

} /* anonymous namespace */

// ---------------------------------------------------------------------------------------------

bool decodeRPatchMesh(const uint8 *data, size_t size, SRPatchMesh &out, std::string &err)
{
	CBlobReader r(data, size);
	if (!r.value(out.Version)) { err = "blob truncated at version"; return false; }
	if (out.Version < 1 || out.Version > 9) { err = "unknown RPatchMesh version"; return false; }
	const uint32 v = out.Version;

	sint32 patchCount;
	if (!r.value(patchCount) || patchCount < 0) { err = "blob truncated at patch count"; return false; }
	out.Patches.resize(patchCount);
	for (sint32 i = 0; i < patchCount; ++i)
	{
		SRpoPatch &p = out.Patches[i];
		if (!r.value(p.NbTilesU) || !r.value(p.NbTilesV)) { err = "blob truncated at patch header"; return false; }
		sint32 tileCount;
		if (!r.value(tileCount) || tileCount < 0) { err = "blob truncated at tile count"; return false; }
		p.Tiles.resize(tileCount);
		for (sint32 j = 0; j < tileCount; ++j)
		{
			SRpoTile &t = p.Tiles[j];
			if (v < 3)
			{
				// Two discarded ints per tile; tiles empty.
				sint32 oldA, oldB;
				if (!r.value(oldA) || !r.value(oldB)) { err = "blob truncated at v1/v2 tile"; return false; }
				t.Num = 0; t.Flags = 0; t.Noise = 0;
				memset(t.Layer, 0, sizeof(t.Layer));
				continue;
			}
			if (!r.value(t.Num)) { err = "blob truncated at tile num"; return false; }
			if (v >= 5)
			{
				if (!r.value(t.Flags)) { err = "blob truncated at tile flags"; return false; }
			}
			else t.Flags = 0;
			if (v >= 9)
			{
				if (!r.value(t.Noise)) { err = "blob truncated at tile noise"; return false; }
			}
			else t.Noise = 0; // pre-v9: no noise stored; the original loader randomizes at load
			for (int k = 0; k < 3; ++k)
			{
				uint8 reserved;
				if (!r.value(reserved) || !r.value(t.Layer[k].Tile) || !r.value(t.Layer[k].Rotate))
				{ err = "blob truncated at tile layer"; return false; }
			}
		}
		sint32 colorCount;
		if (!r.value(colorCount) || colorCount < 0) { err = "blob truncated at color count"; return false; }
		p.Colors.resize(colorCount);
		for (sint32 j = 0; j < colorCount; ++j)
			if (!r.value(p.Colors[j])) { err = "blob truncated at colors"; return false; }
		if (v >= 7)
		{
			for (int e = 0; e < 4; ++e)
				if (!r.value(p.EdgeFlags[e])) { err = "blob truncated at edge flags"; return false; }
		}
		else memset(p.EdgeFlags, 0, sizeof(p.EdgeFlags));
	}

	sint32 vertexCount;
	if (!r.value(vertexCount) || vertexCount < 0) { err = "blob truncated at vertex count"; return false; }
	out.Verts.resize(vertexCount);
	for (sint32 i = 0; i < vertexCount; ++i)
	{
		SRpoVertexBind &b = out.Verts[i];
		if (!r.value(b.Binded) || !r.value(b.Type) || !r.value(b.Edge) || !r.value(b.Patch)
			|| !r.value(b.Before) || !r.value(b.Before2) || !r.value(b.After) || !r.value(b.After2)
			|| !r.value(b.T) || !r.value(b.Type2) || !r.value(b.PrimVert))
		{ err = "blob truncated at vertex bind"; return false; }
	}

	if (!r.value(out.TileTessLevel) || !r.value(out.ModeTile) || !r.value(out.KeepMapping))
	{ err = "blob truncated at tess trailer"; return false; }
	// The writer emits TransitionType then SelLevel for every version (the original reader
	// consumes TransitionType only for v4 and misreads its own trailer; the writer is the
	// authority — wiki Part A.4). Genuinely old writers may end earlier; bound by size.
	out.TransitionType = 0;
	out.SelLevel = 0;
	if (r.Pos + 4 <= r.Size) r.value(out.TransitionType);
	if (r.Pos + 4 <= r.Size) r.value(out.SelLevel);
	if (r.Pos != r.Size) { err = "blob trailing bytes"; return false; }
	return true;
}

bool decodeRpoChunk(const uint8 *data, size_t size, SRPatchMesh &out, std::string &err)
{
	if (size < 4) { err = "0x08fd chunk too small"; return false; }
	uint32 rpoVersion;
	memcpy(&rpoVersion, data, 4);
	if (rpoVersion != 0) { err = "unexpected rpoVersion"; return false; }
	return decodeRPatchMesh(data + 4, size - 4, out, err);
}

// ---------------------------------------------------------------------------------------------
// PatchMesh stream decode.

namespace {

const CStorageRaw *rawSub(const CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
		if (it->first == id) return dynamic_cast<const CStorageRaw *>(it->second);
	return NULL;
}

bool readRawInts(const CStorageRaw *raw, sint32 *dst, size_t n, const char *what, std::string &err)
{
	if (!raw || raw->Value.size() != n * 4) { err = std::string("bad ") + what; return false; }
	memcpy(dst, nlVectorData(raw->Value), n * 4);
	return true;
}

bool readRawFloats(const CStorageRaw *raw, float *dst, size_t n, const char *what, std::string &err)
{
	if (!raw || raw->Value.size() != n * 4) { err = std::string("bad ") + what; return false; }
	memcpy(dst, nlVectorData(raw->Value), n * 4);
	return true;
}

bool readCountedInts(const CStorageRaw *raw, std::vector<sint32> &dst, const char *what, std::string &err)
{
	if (!raw || raw->Value.size() < 4) { err = std::string("bad ") + what; return false; }
	sint32 count;
	memcpy(&count, nlVectorData(raw->Value), 4);
	if (count < 0 || raw->Value.size() != 4 + (size_t)count * 4) { err = std::string("bad count in ") + what; return false; }
	dst.resize(count);
	if (count) memcpy(&dst[0], nlVectorData(raw->Value) + 4, count * 4);
	return true;
}

} /* anonymous namespace */

bool decodePatchMesh(const CStorageContainer::TStorageObjectContainer &chunks, SPatchMesh &out, std::string &err)
{
	out.Verts.clear();
	out.Vecs.clear();
	out.Edges.clear();
	out.Patches.clear();
	// The count chunks (0x0BD6 verts, 0x0BC2 vecs, 0x0BD1 edges, 0x0BEA patches) precede their
	// element containers, but decode is tolerant of order: collect elements by id, then verify
	// against the counts.
	sint32 numVerts = -1, numVecs = -1, numEdges = -1, numPatches = -1;
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		const uint16 id = it->first;
		switch (id)
		{
		case 0x0bd6: case 0x0bc2: case 0x0bd1: case 0x0bea:
		{
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			sint32 v;
			if (!readRawInts(raw, &v, 1, "count chunk", err)) return false;
			if (id == 0x0bd6) numVerts = v;
			else if (id == 0x0bc2) numVecs = v;
			else if (id == 0x0bd1) numEdges = v;
			else numPatches = v;
			break;
		}
		case 0x0be0: // vertex
		{
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "vertex chunk not a container"; return false; }
			out.Verts.resize(out.Verts.size() + 1);
			SPmVert &vt = out.Verts.back();
			if (!readRawFloats(rawSub(c, 0x03e8), vt.Pos, 3, "vertex pos", err)) return false;
			if (!readRawInts(rawSub(c, 0x03fc), &vt.Flags, 1, "vertex flags", err)) return false;
			if (!readCountedInts(rawSub(c, 0x0406), vt.Vectors, "vertex vectors", err)) return false;
			if (!readCountedInts(rawSub(c, 0x0410), vt.Patches, "vertex patches", err)) return false;
			if (!readCountedInts(rawSub(c, 0x041a), vt.Edges, "vertex edges", err)) return false;
			break;
		}
		case 0x0bcc: // vector
		{
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "vector chunk not a container"; return false; }
			out.Vecs.resize(out.Vecs.size() + 1);
			SPmVec &vc = out.Vecs.back();
			if (!readRawFloats(rawSub(c, 0x03e8), vc.Pos, 3, "vector pos", err)) return false;
			if (!readRawInts(rawSub(c, 0x03fc), &vc.Flags, 1, "vector flags", err)) return false;
			if (!readRawInts(rawSub(c, 0x0410), &vc.Vert, 1, "vector vert", err)) return false;
			if (!readCountedInts(rawSub(c, 0x0406), vc.Patches, "vector patches", err)) return false;
			break;
		}
		case 0x0bd2: // edge
		{
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "edge chunk not a container"; return false; }
			out.Edges.resize(out.Edges.size() + 1);
			SPmEdge &e = out.Edges.back();
			sint32 quad[4];
			if (!readRawInts(rawSub(c, 0x03e8), quad, 4, "edge record", err)) return false;
			e.V1 = quad[0]; e.Vec12 = quad[1]; e.Vec21 = quad[2]; e.V2 = quad[3];
			if (!readCountedInts(rawSub(c, 0x03f2), e.Patches, "edge patches", err)) return false;
			break;
		}
		case 0x0bf4: // patch
		{
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "patch chunk not a container"; return false; }
			out.Patches.resize(out.Patches.size() + 1);
			SPmPatch &p = out.Patches.back();
			if (!readRawInts(rawSub(c, 0x0424), &p.Type, 1, "patch type", err)) return false;
			if (!readRawInts(rawSub(c, 0x03e8), &p.NumVerts, 1, "patch numverts", err)) return false;
			if (!readRawInts(rawSub(c, 0x03f2), p.V, 4, "patch verts", err)) return false;
			if (!readRawInts(rawSub(c, 0x03fc), p.Vec, 8, "patch vecs", err)) return false;
			if (!readRawInts(rawSub(c, 0x0406), p.Interior, 4, "patch interiors", err)) return false;
			if (!readRawInts(rawSub(c, 0x0410), &p.SmGroup, 1, "patch smgroup", err)) return false;
			if (!readRawInts(rawSub(c, 0x041a), &p.Flags, 1, "patch flags", err)) return false;
			if (!readRawInts(rawSub(c, 0x042e), p.Edge, 4, "patch edges", err)) return false;
			break;
		}
		default:
			break; // header/trailer/mesh chunks: not interpreted here
		}
	}
	if (numVerts >= 0 && (size_t)numVerts != out.Verts.size()) { err = "vertex count mismatch"; return false; }
	if (numVecs >= 0 && (size_t)numVecs != out.Vecs.size()) { err = "vector count mismatch"; return false; }
	if (numEdges >= 0 && (size_t)numEdges != out.Edges.size()) { err = "edge count mismatch"; return false; }
	if (numPatches >= 0 && (size_t)numPatches != out.Patches.size()) { err = "patch count mismatch"; return false; }
	return true;
}

// ---------------------------------------------------------------------------------------------

bool decodeVertMapper(const uint8 *data, size_t size, SPmVertMapper &out, std::string &err)
{
	// int32 vertCount, vertCount records, int32 vecCount, vecCount records; record size is
	// compiler-defined (wiki B.3) — derive it from the payload size and validate.
	if (size < 8) { err = "vert mapper too small"; return false; }
	sint32 vertCount;
	memcpy(&vertCount, data, 4);
	if (vertCount < 0) { err = "vert mapper bad vert count"; return false; }
	// Find the record size: records are [vertCount] then int32 vecCount then [vecCount].
	// The known layout is 32 bytes (int32 + 3 floats + 3 floats + BOOL, no padding).
	const size_t recSize = 32;
	if (size < 4 + (size_t)vertCount * recSize + 4) { err = "vert mapper truncated"; return false; }
	sint32 vecCount;
	memcpy(&vecCount, data + 4 + (size_t)vertCount * recSize, 4);
	if (vecCount < 0 || size != 8 + ((size_t)vertCount + (size_t)vecCount) * recSize)
	{ err = "vert mapper size mismatch (record size not 32?)"; return false; }
	out.VertMap.resize(vertCount);
	out.VecMap.resize(vecCount);
	// EPMapVert declaration order (editpat.h): BOOL originalStored; int vert; Point3 original;
	// Point3 delta; — raw struct dump, no padding.
	const uint8 *p = data + 4;
	for (sint32 i = 0; i < vertCount; ++i, p += recSize)
	{
		SPmMapVert &m = out.VertMap[i];
		memcpy(&m.OriginalStored, p, 4);
		memcpy(&m.Vert, p + 4, 4);
		memcpy(m.Original, p + 8, 12);
		memcpy(m.Delta, p + 20, 12);
	}
	p += 4;
	for (sint32 i = 0; i < vecCount; ++i, p += recSize)
	{
		SPmMapVert &m = out.VecMap[i];
		memcpy(&m.OriginalStored, p, 4);
		memcpy(&m.Vert, p + 4, 4);
		memcpy(m.Original, p + 8, 12);
		memcpy(m.Delta, p + 20, 12);
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Chunk id inventories (established by the 1231-file ligo corpus survey, 2026-07-06; the
// sub-chunk sets inside the element containers are uniform corpus-wide).

const uint16 PatchMeshChunkIds[] = {
	0x0bb8, 0x0bb9, 0x0bc2, 0x0bcc, 0x0bd1, 0x0bd2, 0x0bd6, 0x0be0, 0x0bea, 0x0bf4,
	0x0bfe, 0x0c12, 0x0c1c, 0x0c26, 0x0c30, 0x0c3a, 0x0c44, 0x0c4e, 0x0c58, 0x0c62,
	0x0c6c, 0x0c76, 0x0c80, 0x0c8a, 0x0c94, 0x0d48, 0x0d52, 0x0d5c, 0x0d66, 0x0d7a,
	0x3440,
	0
};

const uint16 MeshChunkIds[] = {
	0x0906, 0x0908, 0x0912, 0x0914, 0x0924, 0x0928, 0x092a, 0x092c, 0x092e, 0x094c,
	0x0959, 0x2394, 0x2396, 0x2398,
	0
};

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
