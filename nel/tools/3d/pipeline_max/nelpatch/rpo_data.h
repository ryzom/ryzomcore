/**
 * \file rpo_data.h
 * \brief RPO / NeL patch mesh data decoding
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * Plain-data views of the Rykol Patch Object payloads: the RPatchMesh blob (chunk 0x08FD on
 * the RklPatch scene object, or chunk 0x4001 in NeL Edit Patch per-node local data) and the
 * Max PatchMesh chunk stream (flat on the RklPatch object, or nested under chunk 0x1140 in
 * the modifier local data). Decoding is read-only over the raw storage chunks — the chunks
 * themselves stay authoritative for serialization (roundtrip is byte-exact by construction).
 * See wiki drafts/max_geometry_formats.md Parts A/B for the formats.
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

#ifndef PIPELINE_RPO_DATA_H
#define PIPELINE_RPO_DATA_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>

// Project includes
#include "../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

// ---------------------------------------------------------------------------------------------
// RPatchMesh blob (nel_patch_lib/nel_patch_mesh.cpp RPatchMesh::Load/Save, versions 1-9; the
// whole ligo corpus is v9). All fields little-endian, raw sequential, no chunk framing inside.

/// One tile layer reference (bool8 reserved + int32 tile + int32 rotate on disk)
struct SRpoTileLayer
{
	sint32 Tile;     // tile index in the bank, or -1/0x7fffffff-style empty markers per _Num
	sint32 Rotate;   // 0..3
};

/// Per-tile record (v9: uint16 num, uint16 flags, uint8 noise, 3 layers)
struct SRpoTile
{
	uint16 Num;      // number of used layers (0..3)
	uint16 Flags;    // case (bits 0-2) + displace noise (bits 3-6), see tileDesc
	uint8 Noise;     // v9 displace noise byte (duplicates the flags bits through setDisplace)
	SRpoTileLayer Layer[3];
};

/// Per-patch user info
struct SRpoPatch
{
	sint32 NbTilesU; // log2 tile columns (OrderS = 1 << NbTilesU)
	sint32 NbTilesV; // log2 tile rows (OrderT = 1 << NbTilesV)
	std::vector<SRpoTile> Tiles;    // (1<<NbTilesU) * (1<<NbTilesV)
	std::vector<uint32> Colors;     // ((1<<NbTilesU)+1) * ((1<<NbTilesV)+1), 0x00RRGGBB
	uint32 EdgeFlags[4];            // v7+: bit 0 = no-smooth
};

/// Per-vertex bind record (all versions identical)
struct SRpoVertexBind
{
	uint8 Binded;     // bool8
	uint32 Type;      // typeBind: 0=BIND_25, 1=BIND_75, 2=BIND_50, 3=BIND_SINGLE
	uint32 Edge;      // edge 0..3 of the target patch
	uint32 Patch;     // target patch index
	uint32 Before, Before2, After, After2, T; // rebuildable tangent cache indices
	uint32 Type2;     // duplicate of Type (written twice by the original)
	uint32 PrimVert;  // primary vertex of the bind group
};

/// The decoded RPatchMesh blob
struct SRPatchMesh
{
	uint32 Version;
	std::vector<SRpoPatch> Patches;
	std::vector<SRpoVertexBind> Verts;
	sint32 TileTessLevel;
	uint8 ModeTile;
	uint8 KeepMapping;
	sint32 TransitionType; // trailer as written by the v9-era writer (see A.4)
	sint32 SelLevel;
};

/// Decode an RPatchMesh blob from raw bytes (chunk 0x4001 payload, or 0x08FD payload past the
/// leading uint32 rpoVersion). Returns false with a message on malformed data.
bool decodeRPatchMesh(const uint8 *data, size_t size, SRPatchMesh &out, std::string &err);

/// Decode the 0x08FD chunk payload (uint32 rpoVersion == 0, then the blob).
bool decodeRpoChunk(const uint8 *data, size_t size, SRPatchMesh &out, std::string &err);

// ---------------------------------------------------------------------------------------------
// Max PatchMesh chunk stream. Chunk ids scoped to the PatchMesh stream (they appear flat on the
// RklPatch scene object between 0x08FD and the Mesh chunks, and nested inside modifier chunk
// 0x1140). Layouts established against the ligo corpus (uniform across all 8482 instances) and
// cross-validated against the reference .zone exports.

/// PatchMesh vertex (container 0x0BE0)
struct SPmVert
{
	float Pos[3];                 // 0x03E8
	sint32 Flags;                 // 0x03FC
	std::vector<sint32> Vectors;  // 0x0406, count-prefixed
	std::vector<sint32> Patches;  // 0x0410, count-prefixed
	std::vector<sint32> Edges;    // 0x041A, count-prefixed
};

/// PatchMesh vector — tangent or interior handle (container 0x0BCC)
struct SPmVec
{
	float Pos[3];                 // 0x03E8
	sint32 Flags;                 // 0x03FC
	sint32 Vert;                  // 0x0410, owner vertex
	std::vector<sint32> Patches;  // 0x0406, count-prefixed
};

/// PatchMesh edge (container 0x0BD2)
struct SPmEdge
{
	sint32 V1, Vec12, Vec21, V2;  // 0x03E8 (16 bytes)
	std::vector<sint32> Patches;  // 0x03F2, count-prefixed
};

/// PatchMesh patch (container 0x0BF4)
struct SPmPatch
{
	sint32 Type;       // 0x0424 (4 = quad, 3 = tri; the zone corpus is all quads)
	sint32 NumVerts;   // 0x03E8 (duplicates the vertex count implied by Type)
	sint32 V[4];       // 0x03F2
	sint32 Vec[8];     // 0x03FC
	sint32 Interior[4];// 0x0406
	sint32 SmGroup;    // 0x0410
	sint32 Flags;      // 0x041A
	sint32 Edge[4];    // 0x042E
};

/// The decoded PatchMesh (geometry/topology; header and trailer chunks are not interpreted)
struct SPatchMesh
{
	std::vector<SPmVert> Verts;    // count chunk 0x0BD6
	std::vector<SPmVec> Vecs;      // count chunk 0x0BC2
	std::vector<SPmEdge> Edges;    // count chunk 0x0BD1
	std::vector<SPmPatch> Patches; // count chunk 0x0BEA
};

/// Decode a PatchMesh from a chunk list (the claimed/orphaned chunks of an RklPatch scene
/// object, or the children of a 0x1140 modifier chunk). Unrelated ids in the list are ignored.
bool decodePatchMesh(const CStorageContainer::TStorageObjectContainer &chunks, SPatchMesh &out, std::string &err);

// ---------------------------------------------------------------------------------------------
// NeL Edit Patch modifier per-node vertex mapper (chunk 0x1130 -> child 0x1000; see wiki Part
// B.3). Evaluation semantics (EPVertMapper::UpdateAndApplyDeltas): for each mapped record
// (Vert >= 0 && OriginalStored), the final patch's vertex/vector [record.Vert] position is the
// INPUT patch's position at the record's index plus Delta — the stored final-patch position is
// overwritten. Unmapped records leave the final patch's stored position in place.

struct SPmMapVert
{
	sint32 Vert;       // output index, -1 = unmapped
	float Original[3]; // input position at save time (refreshed from the input at eval)
	float Delta[3];    // authored delta
	sint32 OriginalStored; // BOOL
};

struct SPmVertMapper
{
	std::vector<SPmMapVert> VertMap; // indexed by input vertex
	std::vector<SPmMapVert> VecMap;  // indexed by input vector
};

/// Decode the vertex mapper from the 0x1130 container's 0x1000 child payload.
bool decodeVertMapper(const uint8 *data, size_t size, SPmVertMapper &out, std::string &err);

/// Chunk ids of the PatchMesh stream (for claim lists); terminated by 0.
extern const uint16 PatchMeshChunkIds[];
/// Chunk ids of the Mesh cache stream that follows on RklPatch scene objects; terminated by 0.
extern const uint16 MeshChunkIds[];

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_RPO_DATA_H */

/* end of file */
