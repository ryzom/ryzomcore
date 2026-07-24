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
				// Two legacy ints per tile; tiles empty (retained for byte-faithful re-encode).
				if (!r.value(t.OldA) || !r.value(t.OldB)) { err = "blob truncated at v1/v2 tile"; return false; }
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
				if (!r.value(t.Layer[k].Reserved) || !r.value(t.Layer[k].Tile) || !r.value(t.Layer[k].Rotate))
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
	out.TrailerCount = 0;
	if (r.Pos + 4 <= r.Size) { r.value(out.TransitionType); out.TrailerCount = 1; }
	if (r.Pos + 4 <= r.Size) { r.value(out.SelLevel); out.TrailerCount = 2; }
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
// RPatchMesh blob encode: the exact inverse of decodeRPatchMesh (mirrors RPatchMesh::Save with
// the same version branches the decoder consumes, so decode-encode is the identity byte-wise).

namespace {

struct CBlobWriter
{
	std::vector<uint8> &Out;
	CBlobWriter(std::vector<uint8> &out) : Out(out) { }
	void write(const void *src, size_t n)
	{
		size_t pos = Out.size();
		Out.resize(pos + n);
		memcpy(&Out[pos], src, n);
	}
	template <typename T>
	void value(const T &v) { write(&v, sizeof(T)); }
};

} /* anonymous namespace */

void encodeRPatchMesh(const SRPatchMesh &in, std::vector<uint8> &out)
{
	out.clear();
	CBlobWriter w(out);
	const uint32 v = in.Version;
	w.value(in.Version);

	w.value((sint32)in.Patches.size());
	for (size_t i = 0; i < in.Patches.size(); ++i)
	{
		const SRpoPatch &p = in.Patches[i];
		w.value(p.NbTilesU);
		w.value(p.NbTilesV);
		w.value((sint32)p.Tiles.size());
		for (size_t j = 0; j < p.Tiles.size(); ++j)
		{
			const SRpoTile &t = p.Tiles[j];
			if (v < 3)
			{
				w.value(t.OldA);
				w.value(t.OldB);
				continue;
			}
			w.value(t.Num);
			if (v >= 5) w.value(t.Flags);
			if (v >= 9) w.value(t.Noise);
			for (int k = 0; k < 3; ++k)
			{
				w.value(t.Layer[k].Reserved);
				w.value(t.Layer[k].Tile);
				w.value(t.Layer[k].Rotate);
			}
		}
		w.value((sint32)p.Colors.size());
		for (size_t j = 0; j < p.Colors.size(); ++j)
			w.value(p.Colors[j]);
		if (v >= 7)
			for (int e = 0; e < 4; ++e)
				w.value(p.EdgeFlags[e]);
	}

	w.value((sint32)in.Verts.size());
	for (size_t i = 0; i < in.Verts.size(); ++i)
	{
		const SRpoVertexBind &b = in.Verts[i];
		w.value(b.Binded);
		w.value(b.Type);
		w.value(b.Edge);
		w.value(b.Patch);
		w.value(b.Before);
		w.value(b.Before2);
		w.value(b.After);
		w.value(b.After2);
		w.value(b.T);
		w.value(b.Type2);
		w.value(b.PrimVert);
	}

	w.value(in.TileTessLevel);
	w.value(in.ModeTile);
	w.value(in.KeepMapping);
	if (in.TrailerCount >= 1) w.value(in.TransitionType);
	if (in.TrailerCount >= 2) w.value(in.SelLevel);
}

void encodeRpoChunk(const SRPatchMesh &in, std::vector<uint8> &out)
{
	out.clear();
	const uint32 rpoVersion = 0;
	CBlobWriter w(out);
	w.value(rpoVersion);
	std::vector<uint8> blob;
	encodeRPatchMesh(in, blob);
	if (!blob.empty()) w.write(&blob[0], blob.size());
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

// Optional adjacency tables: Max 3 (Scene version 0x2004) PatchMesh omits the count-prefixed
// adjacency lists on verts/vecs and the entire edge stream. Absent → empty (or -1 for owner).
bool readOptionalCountedInts(const CStorageRaw *raw, std::vector<sint32> &dst, const char *what, std::string &err)
{
	if (!raw) { dst.clear(); return true; }
	return readCountedInts(raw, dst, what, err);
}

bool readOptionalInt(const CStorageRaw *raw, sint32 &dst, sint32 defaultVal)
{
	if (!raw) { dst = defaultVal; return true; }
	if (raw->Value.size() != 4) return false;
	memcpy(&dst, nlVectorData(raw->Value), 4);
	return true;
}

/// Rebuild the Max 4+ edge table from patch topology. Max 3 stores only verts/vecs/patches;
/// each quad edge is (V[i], Vec[2i], Vec[2i+1], V[(i+1)%n]). Shared undirected sides reuse one
/// edge record; edge.Patches lists every patch that uses it (order = first writer, then later).
bool reconstructEdgesFromPatches(SPatchMesh &out, std::string &err)
{
	out.Edges.clear();
	// key = (loVert, hiVert) — undirected; value = edge index
	// Linear scan is fine: snowballs zones are ~36 patches / ~70 edges.
	for (size_t pi = 0; pi < out.Patches.size(); ++pi)
	{
		SPmPatch &p = out.Patches[pi];
		const sint32 n = (p.NumVerts > 0 && p.NumVerts <= 4) ? p.NumVerts : 4;
		if (p.Type == 0) p.Type = n;
		for (sint32 e = 0; e < n; ++e)
		{
			const sint32 v1 = p.V[e];
			const sint32 v2 = p.V[(e + 1) % n];
			const sint32 vec12 = p.Vec[2 * e];
			const sint32 vec21 = p.Vec[2 * e + 1];
			if (v1 < 0 || v2 < 0) { err = "patch vertex index out of range during edge rebuild"; return false; }
			// Find existing edge with same endpoints (either orientation).
			sint32 found = -1;
			for (size_t ei = 0; ei < out.Edges.size(); ++ei)
			{
				const SPmEdge &ed = out.Edges[ei];
				if ((ed.V1 == v1 && ed.V2 == v2) || (ed.V1 == v2 && ed.V2 == v1))
				{
					found = (sint32)ei;
					break;
				}
			}
			if (found < 0)
			{
				found = (sint32)out.Edges.size();
				SPmEdge ne;
				ne.V1 = v1;
				ne.Vec12 = vec12;
				ne.Vec21 = vec21;
				ne.V2 = v2;
				ne.Patches.clear();
				out.Edges.push_back(ne);
			}
			SPmEdge &ed = out.Edges[(size_t)found];
			// Append this patch if not already listed.
			bool already = false;
			for (size_t k = 0; k < ed.Patches.size(); ++k)
				if (ed.Patches[k] == (sint32)pi) { already = true; break; }
			if (!already) ed.Patches.push_back((sint32)pi);
			p.Edge[e] = found;
		}
		// Zero unused edge slots for tri patches.
		for (sint32 e = n; e < 4; ++e)
			p.Edge[e] = -1;
	}
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
	// Max 3 (Scene 0x2004): no 0x0BD1/0x0BD2 edges; vertex/vector adjacency sub-chunks absent;
	// patch Type (0x0424) and Edge (0x042E) absent. Adjacency is optional; edges are rebuilt.
	sint32 numVerts = -1, numVecs = -1, numEdges = -1, numPatches = -1;
	bool anyEdgeChunk = false;
	bool anyPatchEdgeIdx = false;
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
			// Max 3 omits 0x0406/0x0410/0x041a adjacency tables entirely.
			if (!readOptionalCountedInts(rawSub(c, 0x0406), vt.Vectors, "vertex vectors", err)) return false;
			if (!readOptionalCountedInts(rawSub(c, 0x0410), vt.Patches, "vertex patches", err)) return false;
			if (!readOptionalCountedInts(rawSub(c, 0x041a), vt.Edges, "vertex edges", err)) return false;
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
			// Max 3: no owner vert (0x0410) / patch list (0x0406).
			if (!readOptionalInt(rawSub(c, 0x0410), vc.Vert, -1)) { err = "bad vector vert"; return false; }
			if (!readOptionalCountedInts(rawSub(c, 0x0406), vc.Patches, "vector patches", err)) return false;
			break;
		}
		case 0x0bd2: // edge
		{
			anyEdgeChunk = true;
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "edge chunk not a container"; return false; }
			out.Edges.resize(out.Edges.size() + 1);
			SPmEdge &e = out.Edges.back();
			sint32 quad[4];
			if (!readRawInts(rawSub(c, 0x03e8), quad, 4, "edge record", err)) return false;
			e.V1 = quad[0]; e.Vec12 = quad[1]; e.Vec21 = quad[2]; e.V2 = quad[3];
			if (!readOptionalCountedInts(rawSub(c, 0x03f2), e.Patches, "edge patches", err)) return false;
			break;
		}
		case 0x0bf4: // patch
		{
			const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(it->second);
			if (!c) { err = "patch chunk not a container"; return false; }
			out.Patches.resize(out.Patches.size() + 1);
			SPmPatch &p = out.Patches.back();
			// Max 3: Type (0x0424) absent — NumVerts (0x03e8) carries 3/4; Type defaults to it.
			if (!readOptionalInt(rawSub(c, 0x0424), p.Type, 0)) { err = "bad patch type"; return false; }
			if (!readRawInts(rawSub(c, 0x03e8), &p.NumVerts, 1, "patch numverts", err)) return false;
			if (p.Type == 0) p.Type = p.NumVerts;
			if (!readRawInts(rawSub(c, 0x03f2), p.V, 4, "patch verts", err)) return false;
			if (!readRawInts(rawSub(c, 0x03fc), p.Vec, 8, "patch vecs", err)) return false;
			if (!readRawInts(rawSub(c, 0x0406), p.Interior, 4, "patch interiors", err)) return false;
			if (!readRawInts(rawSub(c, 0x0410), &p.SmGroup, 1, "patch smgroup", err)) return false;
			if (!readRawInts(rawSub(c, 0x041a), &p.Flags, 1, "patch flags", err)) return false;
			// Max 3: Edge indices (0x042E) absent — rebuilt below when the edge stream is missing.
			if (rawSub(c, 0x042e))
			{
				anyPatchEdgeIdx = true;
				if (!readRawInts(rawSub(c, 0x042e), p.Edge, 4, "patch edges", err)) return false;
			}
			else
			{
				p.Edge[0] = p.Edge[1] = p.Edge[2] = p.Edge[3] = -1;
			}
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

	// Max 3: no edge stream and no per-patch edge indices — derive both from patch topology so
	// exportZone's bind pass (edge.Patches adjacency) and getEdge lookups work unchanged.
	if (!anyEdgeChunk || !anyPatchEdgeIdx)
	{
		if (!reconstructEdgesFromPatches(out, err)) return false;
	}
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
// Chunk id inventories (corpus-established over the full ligo landscape corpus; sub-chunk
// sets inside the element containers are uniform corpus-wide).

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
