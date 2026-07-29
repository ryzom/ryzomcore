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

namespace {

/// The 0x0C26/0x0C30/0x0C3A containers wrap a single 0x2700 raw: int32 count + dword-padded
/// bit words.
bool decodeBitArray(const IStorageObject *obj, SPmBitArray &out, const char *what, std::string &err)
{
	const CStorageContainer *c = dynamic_cast<const CStorageContainer *>(obj);
	if (!c || c->chunks().size() != 1 || c->chunks().begin()->first != 0x2700)
	{ err = std::string(what) + ": not a single-0x2700 wrapper"; return false; }
	const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(c->chunks().begin()->second);
	if (!raw || raw->Value.size() < 4) { err = std::string(what) + ": bad payload"; return false; }
	sint32 count;
	memcpy(&count, nlVectorData(raw->Value), 4);
	const size_t words = count > 0 ? ((size_t)count + 31) / 32 : 0;
	if (count < 0 || raw->Value.size() != 4 + words * 4)
	{ err = std::string(what) + ": count/size mismatch"; return false; }
	out.Present = 1;
	out.Count = count;
	out.Bits.resize(words);
	if (words) memcpy(&out.Bits[0], nlVectorData(raw->Value) + 4, words * 4);
	return true;
}

} /* anonymous namespace */

bool decodePatchMesh(const CStorageContainer::TStorageObjectContainer &chunks, SPatchMesh &out, std::string &err)
{
	out = SPatchMesh();
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
			vt.HasVectors = rawSub(c, 0x0406) != NULL;
			vt.HasPatches = rawSub(c, 0x0410) != NULL;
			vt.HasEdges = rawSub(c, 0x041a) != NULL;
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
			vc.HasVert = rawSub(c, 0x0410) != NULL;
			vc.HasPatches = rawSub(c, 0x0406) != NULL;
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
			e.HasPatches = rawSub(c, 0x03f2) != NULL;
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
			p.HasType = rawSub(c, 0x0424) != NULL;
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
				p.HasEdgeIdx = 1;
				if (!readRawInts(rawSub(c, 0x042e), p.Edge, 4, "patch edges", err)) return false;
			}
			else
			{
				p.Edge[0] = p.Edge[1] = p.Edge[2] = p.Edge[3] = -1;
			}
			break;
		}
		case 0x0c26:
			if (!decodeBitArray(it->second, out.VertSel, "vert sel bitarray", err)) return false;
			break;
		case 0x0c30:
			if (!decodeBitArray(it->second, out.PatchSel, "patch sel bitarray", err)) return false;
			break;
		case 0x0c3a:
			if (!decodeBitArray(it->second, out.EdgeSel, "edge sel bitarray", err)) return false;
			break;
		case 0x0d48: // hook count
		{
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			sint32 v;
			if (!readRawInts(raw, &v, 1, "hook count", err)) return false;
			out.HasHookCount = 1;
			if (v < 0) { err = "negative hook count"; return false; }
			out.Hooks.resize((size_t)v);
			break;
		}
		case 0x0d52: // hook array (count x 52 bytes; validated against 0x0D48 after the loop)
		{
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			if (!raw || raw->Value.size() % 52) { err = "bad hook array"; return false; }
			out.HasHookArray = 1;
			const size_t n = raw->Value.size() / 52;
			if (out.HasHookCount && out.Hooks.size() != n) { err = "hook count/array mismatch"; return false; }
			out.Hooks.resize(n);
			for (size_t h = 0; h < n; ++h)
				memcpy(out.Hooks[h].I, nlVectorData(raw->Value) + h * 52, 52);
			break;
		}
		case 0x0c80: // TVPatch array: int32 header + 16 ints per patch
		{
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			if (!raw || raw->Value.size() < 4 || (raw->Value.size() - 4) % 64)
			{ err = "bad tvpatch array"; return false; }
			out.HasTvPatches = 1;
			memcpy(&out.TvPatchHeader, nlVectorData(raw->Value), 4);
			const size_t n = (raw->Value.size() - 4) / 64;
			out.TvPatches.resize(n);
			for (size_t t = 0; t < n; ++t)
				memcpy(out.TvPatches[t].Tv, nlVectorData(raw->Value) + 4 + t * 64, 64);
			break;
		}
		case 0x0c8a: // PatchTVert array: int32 header, int32 count, count x UVW
		{
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			if (!raw || raw->Value.size() < 8) { err = "bad patchtvert array"; return false; }
			sint32 count;
			memcpy(&out.TvVertHeader, nlVectorData(raw->Value), 4);
			memcpy(&count, nlVectorData(raw->Value) + 4, 4);
			if (count < 0 || raw->Value.size() != 8 + (size_t)count * 12)
			{ err = "patchtvert count mismatch"; return false; }
			out.HasTvVerts = 1;
			out.TvVerts.resize((size_t)count);
			for (sint32 t = 0; t < count; ++t)
				memcpy(out.TvVerts[t].Pos, nlVectorData(raw->Value) + 8 + (size_t)t * 12, 12);
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
		out.EdgesReconstructed = 1;
		if (!reconstructEdgesFromPatches(out, err)) return false;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// encodePatchMesh: the write direction of the stream above.

namespace {

void writeRawPayload(CStorageRaw *raw, const void *data, size_t size)
{
	raw->Value.resize(size);
	if (size) memcpy(nlVectorData(raw->Value), data, size);
}

void appendInts(std::vector<uint8> &out, const sint32 *v, size_t n)
{
	const size_t at = out.size();
	out.resize(at + n * 4);
	if (n) memcpy(&out[at], v, n * 4);
}

void appendCountedInts(std::vector<uint8> &out, const std::vector<sint32> &v)
{
	const sint32 count = (sint32)v.size();
	appendInts(out, &count, 1);
	if (count) appendInts(out, &v[0], v.size());
}

/// Find the writable raw child of an element container, NULL when absent.
CStorageRaw *rawSubMut(CStorageContainer *c, uint16 id)
{
	for (CStorageContainer::TStorageObjectConstIt it = c->chunks().begin(); it != c->chunks().end(); ++it)
		if (it->first == id) return dynamic_cast<CStorageRaw *>(it->second);
	return NULL;
}

/// Rewrite one child raw of an element container: payload replaced when present, chunk
/// APPENDED when `want` but absent, ERASED when !want but present. Order on append is the
/// caller's responsibility (canonical child order = the order these helpers are called in,
/// which only matters for brand-new containers; existing children keep their position).
bool putElemChild(CStorageContainer *c, uint16 id, bool want, const std::vector<uint8> &payload,
                  const char *what, std::string &err)
{
	CStorageContainer::TStorageObjectContainer &list = c->chunksMut();
	for (CStorageContainer::TStorageObjectContainer::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (it->first != id)
			continue;
		if (!want)
		{
			delete it->second;
			list.erase(it);
			return true;
		}
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) { err = std::string(what) + ": child not a raw"; return false; }
		raw->Value = payload;
		return true;
	}
	if (!want)
		return true;
	CStorageRaw *raw = new CStorageRaw();
	raw->Value = payload;
	list.push_back(CStorageContainer::TStorageObjectWithId(id, raw));
	return true;
}

bool encodeVertContainer(CStorageContainer *c, const SPmVert &vt, std::string &err)
{
	std::vector<uint8> b;
	b.resize(12); memcpy(&b[0], vt.Pos, 12);
	if (!putElemChild(c, 0x03e8, true, b, "vertex pos", err)) return false;
	b.clear(); appendInts(b, &vt.Flags, 1);
	if (!putElemChild(c, 0x03fc, true, b, "vertex flags", err)) return false;
	b.clear(); appendCountedInts(b, vt.Vectors);
	if (!putElemChild(c, 0x0406, vt.HasVectors != 0, b, "vertex vectors", err)) return false;
	b.clear(); appendCountedInts(b, vt.Patches);
	if (!putElemChild(c, 0x0410, vt.HasPatches != 0, b, "vertex patches", err)) return false;
	b.clear(); appendCountedInts(b, vt.Edges);
	if (!putElemChild(c, 0x041a, vt.HasEdges != 0, b, "vertex edges", err)) return false;
	return true;
}

bool encodeVecContainer(CStorageContainer *c, const SPmVec &vc, std::string &err)
{
	std::vector<uint8> b;
	b.resize(12); memcpy(&b[0], vc.Pos, 12);
	if (!putElemChild(c, 0x03e8, true, b, "vector pos", err)) return false;
	b.clear(); appendInts(b, &vc.Flags, 1);
	if (!putElemChild(c, 0x03fc, true, b, "vector flags", err)) return false;
	// Owner BEFORE patch list - the observed vector-container child order.
	b.clear(); appendInts(b, &vc.Vert, 1);
	if (!putElemChild(c, 0x0410, vc.HasVert != 0, b, "vector vert", err)) return false;
	b.clear(); appendCountedInts(b, vc.Patches);
	if (!putElemChild(c, 0x0406, vc.HasPatches != 0, b, "vector patches", err)) return false;
	return true;
}

bool encodeEdgeContainer(CStorageContainer *c, const SPmEdge &e, std::string &err)
{
	std::vector<uint8> b;
	const sint32 quad[4] = { e.V1, e.Vec12, e.Vec21, e.V2 };
	appendInts(b, quad, 4);
	if (!putElemChild(c, 0x03e8, true, b, "edge record", err)) return false;
	b.clear(); appendCountedInts(b, e.Patches);
	if (!putElemChild(c, 0x03f2, e.HasPatches != 0, b, "edge patches", err)) return false;
	return true;
}

bool encodePatchContainer(CStorageContainer *c, const SPmPatch &p, std::string &err)
{
	std::vector<uint8> b;
	// Type FIRST - the observed patch-container child order.
	b.clear(); appendInts(b, &p.Type, 1);
	if (!putElemChild(c, 0x0424, p.HasType != 0, b, "patch type", err)) return false;
	b.clear(); appendInts(b, &p.NumVerts, 1);
	if (!putElemChild(c, 0x03e8, true, b, "patch numverts", err)) return false;
	b.clear(); appendInts(b, p.V, 4);
	if (!putElemChild(c, 0x03f2, true, b, "patch verts", err)) return false;
	b.clear(); appendInts(b, p.Vec, 8);
	if (!putElemChild(c, 0x03fc, true, b, "patch vecs", err)) return false;
	b.clear(); appendInts(b, p.Interior, 4);
	if (!putElemChild(c, 0x0406, true, b, "patch interiors", err)) return false;
	b.clear(); appendInts(b, &p.SmGroup, 1);
	if (!putElemChild(c, 0x0410, true, b, "patch smgroup", err)) return false;
	b.clear(); appendInts(b, &p.Flags, 1);
	if (!putElemChild(c, 0x041a, true, b, "patch flags", err)) return false;
	b.clear(); appendInts(b, p.Edge, 4);
	if (!putElemChild(c, 0x042e, p.HasEdgeIdx != 0, b, "patch edges", err)) return false;
	return true;
}

/// Rewrite one element stream in the chunk list: the count raw and the run of element
/// containers. Containers are reused in place (per-chunk header-width flags survive);
/// extras are erased; new elements are inserted after the last existing one, or after the
/// count chunk when the stream is empty.
template <class TElem>
bool encodeElemStream(CStorageContainer::TStorageObjectContainer &chunks,
                      uint16 countId, uint16 elemId,
                      const std::vector<TElem> &elems,
                      bool (*writer)(CStorageContainer *, const TElem &, std::string &),
                      const char *what, std::string &err)
{
	typedef CStorageContainer::TStorageObjectContainer TList;
	TList::iterator countIt = chunks.end();
	std::vector<TList::iterator> found;
	for (TList::iterator it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first == countId)
			countIt = it;
		else if (it->first == elemId)
			found.push_back(it);
	}
	if (countIt == chunks.end())
	{
		// No count chunk (Max 3 shape, or a stream this list never had): only an empty
		// element set is representable without inventing stream placement.
		if (!elems.empty() && found.empty())
		{ err = std::string(what) + ": no count chunk to anchor a new stream"; return false; }
	}
	else
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(countIt->second);
		if (!raw) { err = std::string(what) + ": count chunk not a raw"; return false; }
		const sint32 n = (sint32)elems.size();
		writeRawPayload(raw, &n, 4);
	}
	// Reuse, then erase extras.
	size_t i = 0;
	for (; i < found.size() && i < elems.size(); ++i)
	{
		CStorageContainer *c = dynamic_cast<CStorageContainer *>(found[i]->second);
		if (!c) { err = std::string(what) + ": element not a container"; return false; }
		if (!writer(c, elems[i], err)) return false;
	}
	for (size_t k = found.size(); k > i; --k)
	{
		delete found[k - 1]->second;
		chunks.erase(found[k - 1]);
	}
	// Append new elements after the last kept one (or right after the count chunk).
	if (i < elems.size())
	{
		TList::iterator insertAfter = i ? found[i - 1] : countIt;
		TList::iterator insertPos = insertAfter;
		++insertPos;
		for (; i < elems.size(); ++i)
		{
			CStorageContainer *c = new CStorageContainer();
			if (!writer(c, elems[i], err)) { delete c; return false; }
			insertPos = chunks.insert(insertPos, CStorageContainer::TStorageObjectWithId(elemId, c));
			++insertPos;
		}
	}
	return true;
}

/// Rewrite one selection BitArray (single-0x2700 wrapper container). Presence must match:
/// creating or dropping the wrapper would need a stream position policy, and no op changes
/// presence - absence is preserved corpus behaviour.
bool encodeBitArrayChunk(CStorageContainer::TStorageObjectContainer &chunks, uint16 id,
                         const SPmBitArray &sel, const char *what, std::string &err)
{
	for (CStorageContainer::TStorageObjectContainer::iterator it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first != id)
			continue;
		if (!sel.Present) { err = std::string(what) + ": chunk present but struct absent"; return false; }
		CStorageContainer *c = dynamic_cast<CStorageContainer *>(it->second);
		if (!c || c->chunks().size() != 1 || c->chunks().begin()->first != 0x2700)
		{ err = std::string(what) + ": not a single-0x2700 wrapper"; return false; }
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(c->chunks().begin()->second);
		if (!raw) { err = std::string(what) + ": payload not a raw"; return false; }
		const size_t words = sel.Count > 0 ? ((size_t)sel.Count + 31) / 32 : 0;
		if (sel.Bits.size() != words) { err = std::string(what) + ": bits/count mismatch"; return false; }
		raw->Value.resize(4 + words * 4);
		memcpy(nlVectorData(raw->Value), &sel.Count, 4);
		if (words) memcpy(nlVectorData(raw->Value) + 4, &sel.Bits[0], words * 4);
		return true;
	}
	if (sel.Present) { err = std::string(what) + ": struct present but chunk absent"; return false; }
	return true;
}

/// Rewrite a flat raw chunk when present; presence must match the struct.
bool encodeFlatRaw(CStorageContainer::TStorageObjectContainer &chunks, uint16 id, bool want,
                   const std::vector<uint8> &payload, const char *what, std::string &err)
{
	for (CStorageContainer::TStorageObjectContainer::iterator it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first != id)
			continue;
		if (!want) { err = std::string(what) + ": chunk present but struct absent"; return false; }
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) { err = std::string(what) + ": not a raw"; return false; }
		raw->Value = payload;
		return true;
	}
	if (want) { err = std::string(what) + ": struct present but chunk absent"; return false; }
	return true;
}

} /* anonymous namespace */

bool encodePatchMesh(const SPatchMesh &pm, CStorageContainer::TStorageObjectContainer &chunks, std::string &err)
{
	if (pm.EdgesReconstructed)
	{
		err = "reconstructed (Max 3) edge table: derived data must not be written back";
		return false;
	}
	if (!encodeElemStream(chunks, 0x0bd6, 0x0be0, pm.Verts, encodeVertContainer, "vert stream", err)) return false;
	if (!encodeElemStream(chunks, 0x0bc2, 0x0bcc, pm.Vecs, encodeVecContainer, "vec stream", err)) return false;
	if (!encodeElemStream(chunks, 0x0bd1, 0x0bd2, pm.Edges, encodeEdgeContainer, "edge stream", err)) return false;
	if (!encodeElemStream(chunks, 0x0bea, 0x0bf4, pm.Patches, encodePatchContainer, "patch stream", err)) return false;
	if (!encodeBitArrayChunk(chunks, 0x0c26, pm.VertSel, "vert sel", err)) return false;
	if (!encodeBitArrayChunk(chunks, 0x0c30, pm.PatchSel, "patch sel", err)) return false;
	if (!encodeBitArrayChunk(chunks, 0x0c3a, pm.EdgeSel, "edge sel", err)) return false;
	{
		std::vector<uint8> b;
		const sint32 n = (sint32)pm.Hooks.size();
		appendInts(b, &n, 1);
		if (!encodeFlatRaw(chunks, 0x0d48, pm.HasHookCount != 0, b, "hook count", err)) return false;
		b.clear();
		b.resize(pm.Hooks.size() * 52);
		for (size_t h = 0; h < pm.Hooks.size(); ++h)
			memcpy(&b[h * 52], pm.Hooks[h].I, 52);
		if (!encodeFlatRaw(chunks, 0x0d52, pm.HasHookArray != 0, b, "hook array", err)) return false;
	}
	{
		std::vector<uint8> b;
		appendInts(b, &pm.TvPatchHeader, 1);
		if (pm.HasTvPatches && pm.TvPatches.size() != pm.Patches.size())
		{ err = "tvpatch array not sized to the patch table"; return false; }
		b.resize(4 + pm.TvPatches.size() * 64);
		for (size_t t = 0; t < pm.TvPatches.size(); ++t)
			memcpy(&b[4 + t * 64], pm.TvPatches[t].Tv, 64);
		if (!encodeFlatRaw(chunks, 0x0c80, pm.HasTvPatches != 0, b, "tvpatch array", err)) return false;
	}
	{
		std::vector<uint8> b;
		appendInts(b, &pm.TvVertHeader, 1);
		const sint32 n = (sint32)pm.TvVerts.size();
		appendInts(b, &n, 1);
		b.resize(8 + pm.TvVerts.size() * 12);
		for (size_t t = 0; t < pm.TvVerts.size(); ++t)
			memcpy(&b[8 + t * 12], pm.TvVerts[t].Pos, 12);
		if (!encodeFlatRaw(chunks, 0x0c8a, pm.HasTvVerts != 0, b, "patchtvert array", err)) return false;
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
