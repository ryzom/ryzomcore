/**
 * \file paint_core.h
 * \brief Tile painting core of the standalone zone painter.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 *
 * Port of the in-Max painter's TILE path (plugin_max/nel_patch_paint paint.cpp) onto the typed
 * pipeline_max model: the EPM_PaintTile/metaTile adjacency graph, the tile put/transition
 * recursion (PutATile/PropagateBorder/RecursTile with the bank xref), 128/256 tiles, rotation,
 * frozen-zone locks, the CNelPatchChanger live-landscape mirror, a bounded undo, and the
 * carrier write-back that feeds the whole-file save.
 *
 * PRISTINE-COPY DISCIPLINE (correctness-critical): per unique carrier blob (write-target:
 * topmost modifier slot whose 0x2512->0x1000 local data carries an RFINALPATCH 0x4001, else
 * the base RPO 0x08FD) ONE SRPatchMesh is decoded from the raw bytes and kept as the
 * authoritative paint state. Ops mutate ONLY their own fields of that pristine copy: tile
 * ops the tile-record fields (SRpoTile Num/Flags/Noise/Layer[].Tile/Rotate), bind ops the
 * per-vertex bind records (opEditBinds), edge-flag ops the per-patch EdgeFlags
 * (opSetEdgeFlags). The evaluated mesh (PatchMesh positions and rp.Verts bind caches the
 * eval/refresh path may rewrite) is for topology/display only and is NEVER encoded back. An
 * untouched pristine copy re-encodes byte-identical, so the paint save path is a no-op for
 * a null edit.
 *
 * DISPLAY INSTANCES / SHARED BACKING: carriers are keyed by the leaf or base RPO POINTER.
 * Multiple SPaintZoneInput entries that resolve to the same pointer (same Node stack, e.g.
 * ecosystem brick self-instances at world offsets/rotations) share ONE pristine SRPatchMesh
 * and one SCarrier::Zones membership list. setTile / setColorRaw fan the live-landscape
 * mirror to every zone in that list (with per-zone transformDesc for tile orients);
 * writeBack encodes the carrier once. Ops addressed at any instance zone id therefore mutate
 * the same paint state as the primary. Per-zone Symmetry/Rotate (plugin EPM_Mesh appdata,
 * land CZoneRegion Flip/Rot) assemble the transformDesc display space: GetTile uses
 * transformDesc(sym, 4-rot), SetTile uses transformInvDesc(sym, 4-rot) for the Max write and
 * remaps the landscape mirror through each shared zone's transform (plugin SetTile port).
 * Color display remaps S under symmetry only (plugin paint_vcolor).
 *
 * tileDesc <-> SRpoTile mapping (plugin nel_patch_lib/nel_patch_mesh.h <-> nelpatch/rpo_data.h,
 * the on-disk v9 record):
 * _Num <-> SRpoTile.Num used-layer count 0..3 (0 = empty tile)
 * _Flags bits 0-2 <-> SRpoTile.Flags tile case: 0 = 128x128, 1..4 = 256 quadrant+1
 * _Flags bits 3-6 <-> SRpoTile.Flags displace map index 0..15 ...
 * (displace) <-> SRpoTile.Noise ... duplicated in the v9 noise byte; the
 * original loader's setDisplace(Noise) makes Noise authoritative on
 * read, so displace READS from Noise and writes keep both in sync
 * _Flags bits 7-15 <-> SRpoTile.Flags legacy/dead bits many corpus records carry;
 * the original loader keeps them in memory and re-saves them, so
 * descs preserve them verbatim through get -> set (fresh descs
 * start zeroed, exactly like the plugin's tileDesc())
 * _MatIDTab[l].Tile <-> SRpoTile.Layer[l].Tile bank tile index (layer 0 = base 1111)
 * _MatIDTab[l].Rotate <-> SRpoTile.Layer[l].Rotate 0..3 CCW
 * untouched on edit: Layer[l].Reserved, OldA/OldB (write-direction retention fields).
 * Grid order: SRpoPatch.Tiles is u + v*OrderS (OrderS = 1<<NbTilesU); Colors is
 * u + v*(OrderS+1), 0x00RRGGBB.
 */

/*
 * Copyright (C) 2026 by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef ZONE_PAINTER_PAINT_CORE_H
#define ZONE_PAINTER_PAINT_CORE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/vector.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_symmetrisation.h>

#include <cstdio>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace NL3D {
class CLandscape;
class IDriver;
}

namespace PIPELINE {
namespace MAX {
class CStorageRaw;
class CStorageContainer;
namespace BUILTIN {
class CNodeImpl;
}
namespace NELPATCH {
class CRklPatchObject;
struct SRPatchMesh;
struct SPatchMesh;
struct SRpoTile;
}
}
}

namespace ZPPAINT {

struct STopoSnapshot; // patch_topo_snapshot.h (kept out of this header: pipeline types)

// Plugin grid constants (nel_patch_mesh.h): the metaTile grid is a fixed 16x16 per patch, only
// OrderS x OrderT entries used; tile id = patch*NUM_TILE_SEL + v*MAX_TILE_IN_PATCH + u.
#define ZP_MAX_TILE_IN_PATCH 16
#define ZP_NUM_TILE_SEL (ZP_MAX_TILE_IN_PATCH * ZP_MAX_TILE_IN_PATCH)

// Paint-core tunables. Namespace-scope const has internal linkage (C++03).
const int ZP_MAX_UNDO = 64;
const int ZP_DEPTH_SEARCH_MAX = 10; // CalcRotPath BFS depth cap
const int ZP_BRUSH_VALUE[3] = { 0, 4, 8 }; // brush size -> tile recursion depth

// ---------------------------------------------------------------------------------------------
// tileIndex / tileDesc (exact port of the plugin's bit semantics; see the mapping block above).

struct CTileIdx
{
	uint16 Tile;
	uint8 Rotate; // 0..3
	CTileIdx() : Tile(0), Rotate(0) { }
	CTileIdx(uint tile, uint rotate) : Tile((uint16)tile), Rotate((uint8)(rotate & 3)) { }
	bool operator==(const CTileIdx &o) const { return Tile == o.Tile && Rotate == o.Rotate; }
};

class CTileDescP
{
public:
	CTileDescP() { setEmpty(); }
	void setTile(int num, int ncase, int displace, const CTileIdx &t0, const CTileIdx &t1, const CTileIdx &t2)
	{
		Num = (uint16)num;
		Mat[0] = t0;
		Mat[1] = t1;
		Mat[2] = t2;
		setCase(ncase);
		setDisplace((uint8)displace);
	}
	CTileIdx &getLayer(int l) { return Mat[l]; }
	const CTileIdx &getLayer(int l) const { return Mat[l]; }
	int getNumLayer() const { return Num; }
	void setEmpty() { Num = 0; Flags = 0; Mat[0] = CTileIdx(); Mat[1] = CTileIdx(); Mat[2] = CTileIdx(); }
	bool isEmpty() const { return Num == 0; }
	int getCase() const { return Flags & 0x7; }
	void setCase(int ncase) { Flags = (uint16)((Flags & ~0x7) | (ncase & 0x7)); }
	uint8 getDisplace() const { return (uint8)((Flags & 0x78) >> 3); }
	void setDisplace(uint8 d) { Flags = (uint16)((Flags & ~0x78) | ((d & 0xf) << 3)); }
	bool operator==(const CTileDescP &o) const
	{
		if (Num != o.Num || Flags != o.Flags) return false;
		for (int l = 0; l < (int)Num; ++l)
			if (!(Mat[l] == o.Mat[l])) return false;
		return true;
	}
	bool operator!=(const CTileDescP &o) const { return !(*this == o); }

	CTileIdx Mat[3];
	uint16 Num;
	uint16 Flags;
};

// tileSetIndex (plugin): tile set + rotation with the rotation-aware strict order used by the
// transition solver.
struct CTileSetIdx
{
	int TileSet;
	int Rotate;
	bool operator<(const CTileSetIdx &other) const
	{
		if (TileSet < other.TileSet) return true;
		if (TileSet > other.TileSet) return false;
		int delta = (other.Rotate - Rotate) & 3;
		if (delta == 1) return true;
		return false; // delta 3 or 0 (delta 2 is rejected before comparison, plugin asserts)
	}
	bool operator!=(const CTileSetIdx &o) const { return TileSet != o.TileSet || Rotate != o.Rotate; }
	bool operator==(const CTileSetIdx &o) const { return TileSet == o.TileSet && Rotate == o.Rotate; }
};

// ---------------------------------------------------------------------------------------------
// The metaTile record (EPM_PaintTile port): one per (patch, u, v), with neighbor links across
// patch edges, intra-mesh binds and welded cross-zone borders. Rotate[e] is the CCW rotation to
// apply when stepping to Voisins[e]; edge e of a tile crosses to the neighbor found via patch
// edge e (0 = -u, 1 = +v, 2 = +u, 3 = -v in grid terms).

struct SPaintTile
{
	sint32 Patch;
	sint32 TileId; // patch*ZP_NUM_TILE_SEL + v*ZP_MAX_TILE_IN_PATCH + u
	sint16 Zone; // paint zone index (== landscape zone id)
	uint8 U, V;
	bool Frozen;
	uint8 Locked; // per-edge bit: neighbor missing or frozen
	SPaintTile *Voisins[4];
	uint8 Rotate[4];
	NLMISC::CVector Center; // world center of the tile quad (color brush range test)
	float Radius; // max corner distance from Center

	SPaintTile() : Patch(-1), TileId(-1), Zone(-1), U(0), V(0), Frozen(false), Locked(0), Radius(0.f)
	{
		Voisins[0] = Voisins[1] = Voisins[2] = Voisins[3] = NULL;
		Rotate[0] = Rotate[1] = Rotate[2] = Rotate[3] = 0;
	}

	SPaintTile *get2Voisin(int i)
	{
		if (Voisins[i]) return Voisins[i]->Voisins[(i + Rotate[i]) & 3];
		return NULL;
	}
	int get2VoisinRotate(int i)
	{
		if (Voisins[i]) return Rotate[i] + Voisins[i]->Rotate[(i + Rotate[i]) & 3];
		return 0;
	}
	SPaintTile *getRight256(int rot, int &rotate) { rotate = Rotate[(2 + rot) & 3]; return Voisins[(2 + rot) & 3]; }
	SPaintTile *getBottom256(int rot, int &rotate) { rotate = Rotate[(1 + rot) & 3]; return Voisins[(1 + rot) & 3]; }
	SPaintTile *getRightBottom256(int rot, int &rotate)
	{
		int rightRot;
		SPaintTile *right = getRight256(rot, rightRot);
		if (right) return right->getBottom256((rot - rightRot) & 3, rotate);
		return NULL;
	}
	SPaintTile *getBottomRight256(int rot, int &rotate)
	{
		int bottomRot;
		SPaintTile *bottom = getBottom256(rot, bottomRot);
		if (bottom) return bottom->getRight256((rot - bottomRot) & 3, rotate);
		return NULL;
	}
	bool validFor256(int rot)
	{
		int r;
		if (!getRight256(rot, r)) return false;
		if (!getBottom256(rot, r)) return false;
		if (!getRightBottom256(rot, r)) return false;
		return getRightBottom256(rot, r) == getBottomRight256(rot, r);
	}
};

// ---------------------------------------------------------------------------------------------
// One paintable zone as the core sees it (built by the tool main from the scene assembly).

struct SPaintZoneInput
{
	PIPELINE::MAX::BUILTIN::CNodeImpl *Node;
	bool Frozen;
	uint ZoneId; // node collection index == landscape zone id
	std::string Name;
	const std::vector<NL3D::CPatchInfo> *Patches; // display patchinfo, world space
	const PIPELINE::MAX::NELPATCH::SPatchMesh *Pm; // evaluated topology (edges, patch verts)
	const PIPELINE::MAX::NELPATCH::SRPatchMesh *EvalRp; // evaluated rp (binds, tile orders)
	// Display-space transform of this zone (plugin EPM_Mesh::Rotate/Symmetry from
	// NEL3D_APPDATA_ZONE_ROTATE / _SYMMETRY; land placement CZoneRegion Rot/Flip).
	// Primary authored bricks are (0, false). Rotated/mirrored self-instances set these
	// so getTile/setTile assemble transformDesc.
	uint Rotate; // 0..3, 90° CCW steps
	bool Symmetry; // mirror (Flip)
	SPaintZoneInput() : Node(NULL), Frozen(false), ZoneId(0), Patches(NULL), Pm(NULL), EvalRp(NULL),
	                    Rotate(0), Symmetry(false) { }
};

/**
 * GEOMETRY WRITE TARGET (patch-edit vertex move).
 *
 * Distinct from the tile carrier above, and it has to be: a tile record lives in the RPO
 * (0x4001 / 0x08FD), while a vertex POSITION lives in the PatchMesh - and may not be the
 * authoritative copy of itself.
 *
 * The evaluation chain is the base RklPatch PatchMesh, then a stack of NeL Edit Patch
 * modifiers. Each modifier stores a final PatchMesh (0x1140) and, optionally, a vertex mapper
 * (0x1130 -> 0x1000). The mapper is what makes this non-obvious: for every record whose
 * Vert >= 0, evaluation OVERWRITES the stored final position with
 *
 * out.Verts[record.Vert].Pos = input.Verts[recordIndex].Pos + record.Delta
 *
 * so for a mapped vertex the stored 0x1140 position is dead bytes. Writing it changes
 * nothing. The delta is the live value.
 *
 * Hence, per output vertex of the TOPMOST edit-patch modifier (its output is what is
 * displayed, so the policy is local to it - no recursion into the stack):
 *
 * mapped -> write Delta += move. Record layout is a flat 32-byte stride
 * (OriginalStored, Vert, Original[3], Delta[3]) so Delta sits at a computable
 * offset: 4 + i*32 + 20 for VertMap[i].
 * unmapped -> write the stored 0x1140 position += move.
 * no modifier stack at all -> write the base PatchMesh position += move.
 *
 * All three are 12-byte in-place overwrites of three floats - no chunk resizes, no re-encode,
 * so an untouched file stays byte-identical. That is what makes this Tier A.
 *
 * Prevalence is why both mapper and non-mapper paths are mainline rather than one being an
 * edge case. Survey over 40 lacustre ligo files / 72 zone nodes: 67 modifier slots, all 67
 * carrying a 0x1140; 46 nodes with a mapper; 2752 mapped vertices. 26 nodes have no modifier
 * at all and go straight to the base. Stacks can be deep - transition-fond-bassin has 16
 * modifier slots, 3 of them with mappers.
 */
struct SGeomWriteTarget
{
	enum TKind { None = 0, BasePatchMesh, ModifierPatchMesh, MapperDelta };
	TKind Kind;
	/// Raw chunk carrying the bytes, and the byte offset of the three floats within it.
	PIPELINE::MAX::CStorageRaw *Raw;
	size_t Offset;
	/// Container form (0x03E8 position chunk of an element container), when the target is a
	/// PatchMesh vertex or vec rather than a mapper record.
	PIPELINE::MAX::CStorageContainer *VertChunk;
	SGeomWriteTarget() : Kind(None), Raw(NULL), Offset(0), VertChunk(NULL) { }
};

/**
 * Which of the PatchMesh's two position tables an element lives in.
 *
 * A patch's corners index Verts and its tangent/interior handles index Vecs
 * (SPmPatch::V[4] and Vec[8]/Interior[4]), and the two are addressed identically at every
 * level below: their own element container in the PatchMesh stream, their own half of the
 * vertex mapper. So the whole write policy is shared and only the table changes.
 */
enum EGeomElem
{
	GeomVert = 0, ///< PatchMesh Verts: container 0x0BE0, mapper VertMap
	GeomVec       ///< PatchMesh Vecs (tangent and interior handles): 0x0BCC, mapper VecMap
};

/**
 * Resolve where a move of output element `elemIdx` must be written for `node`. See
 * SGeomWriteTarget for the policy. Returns false with a message when the chain is malformed;
 * Kind == None means "nothing to write" rather than an error.
 */
bool resolveGeomWriteTarget(PIPELINE::MAX::BUILTIN::CNodeImpl *node, uint elemIdx, EGeomElem elem,
                            SGeomWriteTarget &out, std::string &err);
/** Read / write the three floats a resolved target points at. */
bool geomTargetGet(const SGeomWriteTarget &t, float *xyz);
bool geomTargetSet(const SGeomWriteTarget &t, const float *xyz);

/** One addressable position in a zone's PatchMesh: an index plus which table it indexes. */
struct SGeomElemRef
{
	uint16 Idx;
	EGeomElem Elem;
	SGeomElemRef() : Idx(0), Elem(GeomVert) { }
	SGeomElemRef(uint16 i, EGeomElem e) : Idx(i), Elem(e) { }
};

// Undo delta: one tile-record, color-vertex, or export-prop change (bounded LIFO of strokes).
// Kind 2: raw AppData restore so delete-vs-"0"/presence semantics reapply exactly.
// Kind 4: one vertex BIND record (whole on-disk record verbatim, so undo of an unbind puts
// back the original bind caches byte for byte). Kind 5: one patch-edge FLAG word (reuses
// Patch + S for the edge slot and OldColor/NewColor for the flag values). Kind 6: one
// TOPOLOGY snapshot (TileId = index into the core's snapshot registry; restore goes through
// the topo-restore callback and the caller rebuilds the working set).
struct SUndoTile
{
	uint8 Kind; // 0 = tile record, 1 = color vertex, 2 = export prop appdata, 3 = vertex move,
	            // 4 = vertex bind record, 5 = patch-edge flags, 6 = topology snapshot
	uint Zone;
	sint32 TileId; // tile kind
	CTileDescP Old;
	CTileDescP New;
	sint32 Patch, S, T; // color kind: grid slot
	uint32 OldColor, NewColor; // color kind: raw 0xAARRGGBB values
	// Kind 2:
	uint32 AppDataId;
	bool OldHas, NewHas; // entry present?
	std::string OldValue, NewValue; // string payload without trailing NUL
	// Kind 3: geometry. The write target is RE-RESOLVED on undo rather than cached - a
	// CStorageRaw pointer does not survive a working-set rebuild, and the policy (mapper
	// delta vs stored position) is cheap to redo and must not drift from the forward path.
	uint16 VertIdx;
	uint8 ElemKind; // EGeomElem: which PatchMesh table VertIdx indexes
	float OldPos[3], NewPos[3];
	// Kind 0 only: raw pristine tile-record snapshots (authored space, exact on-disk values)
	// captured around setTileDesc. Undo/redo restores these verbatim after the desc-based
	// replay, so restoration is byte-exact even where the 16-bit desc round trip is lossy
	// (unused-layer -1/0x7fffffff markers, non-canonical rotate/flag bits in stale sources).
	bool HaveRaw;
	uint16 OldRawNum, NewRawNum;
	uint16 OldRawFlags, NewRawFlags;
	uint8 OldRawNoise, NewRawNoise;
	sint32 OldRawTile[3], NewRawTile[3];
	sint32 OldRawRot[3], NewRawRot[3];
	// Kind 4: the full SRpoVertexBind, flattened in field order (Binded, Type, Edge, Patch,
	// Before, Before2, After, After2, T, Type2, PrimVert). VertIdx names the vertex.
	uint32 OldBind[11], NewBind[11];
	SUndoTile()
		: Kind(0), Zone(0), TileId(-1), Patch(-1), S(0), T(0), OldColor(0), NewColor(0),
		  AppDataId(0), OldHas(false), NewHas(false), VertIdx(0), ElemKind(0),
		  HaveRaw(false), OldRawNum(0), NewRawNum(0), OldRawFlags(0), NewRawFlags(0),
		  OldRawNoise(0), NewRawNoise(0)
	{
		for (int l = 0; l < 3; ++l)
		{
			OldRawTile[l] = NewRawTile[l] = 0;
			OldRawRot[l] = NewRawRot[l] = 0;
			OldPos[l] = NewPos[l] = 0.f;
		}
		for (int l = 0; l < 11; ++l)
			OldBind[l] = NewBind[l] = 0;
	}
};

/**
 * One vertex-bind edit for opEditBinds. Binded true = bind the vertex to `Edge` of target
 * patch `Patch` with the given typeBind and bind-group primary; the rebuildable cache
 * indices are written as -1 (the loader recomputes them, exactly as the legacy
 * BindingVertex left them). Binded false = release the vertex: the record keeps its
 * Type/Patch/Edge/PrimVert and only the flag and caches are cleared, matching the legacy
 * UnBindingVertex. Group semantics (a BIND_25/50/75 trio releases together) are the
 * CALLER's policy, like the bound-vertex move filter.
 */
struct SBindEdit
{
	uint16 Vert;
	bool Binded;
	uint32 Type;     // typeBind (0=BIND_25, 1=BIND_75, 2=BIND_50, 3=BIND_SINGLE)
	uint32 Edge;     // target patch edge 0..3
	uint32 Patch;    // target patch
	uint32 PrimVert; // primary vertex of the bind group
	SBindEdit() : Vert(0), Binded(false), Type(0), Edge(0), Patch(0), PrimVert(0) { }
};

/** One patch-edge flag write for opSetEdgeFlags (bit 0 = no-smooth). */
struct SEdgeFlagEdit
{
	uint16 Patch;
	uint8 EdgeSlot; // 0..3
	uint32 NewFlags;
	SEdgeFlagEdit() : Patch(0), EdgeSlot(0), NewFlags(0) { }
};

// One color-grid slot (zone index + patch + grid coordinates); closures of co-located slots
// across patch seams and welded cross-zone borders receive identical colors.
struct SColorSlot
{
	uint ZoneIdx;
	sint32 Patch;
	sint32 S, T;
	bool operator<(const SColorSlot &o) const
	{
		if (ZoneIdx != o.ZoneIdx) return ZoneIdx < o.ZoneIdx;
		if (Patch != o.Patch) return Patch < o.Patch;
		if (S != o.S) return S < o.S;
		return T < o.T;
	}
	bool operator==(const SColorSlot &o) const
	{
		return ZoneIdx == o.ZoneIdx && Patch == o.Patch && S == o.S && T == o.T;
	}
};

// ---------------------------------------------------------------------------------------------
// The painting core. Single op implementation shared by the script mode and the viewer mouse
// path. All tile state lives in the pristine carrier blobs; the optional live landscape mirror
// is attached by the viewer.

class CPaintCore
{
public:
	CPaintCore();
	~CPaintCore();

	// Resolve carriers, decode pristine blobs, build the metaTile graph and the per-zone
	// CZoneSymmetrisation (rpo2nel forceBuildZoneSymmetry replication with cellSize/snap).
	// keepUndo: preserve the undo/redo stacks across the re-init - the TOPOLOGY rebuild
	// path only. Session working-set changes (open/close) must keep clearing them: their
	// records reference zones that may no longer exist. A topology rebuild keeps the same
	// zone set, and the Kind 6 record at the top of the stack restores the older records'
	// index space before they replay.
	bool init(const std::vector<SPaintZoneInput> &zones, NL3D::CTileBank *bank,
	          float cellSize, float snap, bool lockBorders, std::string &err,
	          bool keepUndo = false);

	// Live display mirror (viewer only; NULL = headless).
	void attachLandscape(NL3D::CLandscape *landscape) { m_Landscape = landscape; }

	// --- ops (one stroke each unless part of a mouse stroke) ---
	// Paint tileSet (bank tile set index; -1 = clear) at (zone, patch, u, v) with rotation.
	bool opTile(uint zone, uint patch, uint u, uint v, int tileSet, int rot, bool _256, std::string &err);
	// Continue a mouse stroke on tile id (rotation tracked via CalcRotPath like the plugin).
	bool opTileStroke(uint zone, sint32 tileId, int tileSet, bool _256, bool first, std::string &err);
	bool opClear(uint zone, uint patch, uint u, uint v, bool _256, std::string &err);
	// Exact vertex color write (blend 0-256; 256 = replace). Writes the whole co-location
	// closure (seam continuity, see setVertexColorShared).
	bool opColorVertex(uint zone, uint patch, sint32 s, sint32 t, NLMISC::CRGBA color, uint blend, std::string &err);
	// The vertex color brush at a world hit point (CPaintColor::paint port): radius in meters,
	// hardness/opacity 0-255. Mouse and the script `cbrush` share this (the script derives the
	// hit from a vertical pick at the given XY). With an active brush mask (loadBrush port) the
	// per-vertex blend is additionally modulated by the mask bitmap: the mask spans the brush
	// circle on a plane perpendicular to the seed tile's display normal (the plugin's hit-quad
	// topVector), sampled bilinearly, luminance mean (R+G+B)/3 scaling the blend. An all-white
	// mask is bit-identical to no mask (blend*255/255). Per-hit stamp; not stroke-oriented.
	bool opColorBrush(uint zone, sint32 seedTileId, const NLMISC::CVector &hit, float radius,
	                  NLMISC::CRGBA color, uint hardness, uint opacity, std::string &err);
	// Region fills (CFillPatch ports): tile fill (tileSet -1 = clear; incompatible borders are
	// cleared, the plugin rule), color fill, displace fill.
	bool opFillTile(uint zone, uint patch, int tileSet, int rot, bool _256, std::string &err);
	bool opFillColor(uint zone, uint patch, NLMISC::CRGBA color, uint blend, std::string &err);
	bool opFillDisplace(uint zone, uint patch, uint displace, std::string &err);
	// Reset one zone's paint wholesale (the painter's ResetPatch, plan mA8): every patch
	// fillTile(-1) + fillColor white + fillDisplace 0, as ONE undo stroke (the per-op
	// endStroke is suspended for the sweep). Danger-guarded at the UI; scripts call bare.
	bool opResetZone(uint zone, std::string &err);
	// Displace paint (PutADisplacetile port; explicit index 0-15, Noise kept in sync). The
	// brush size applies exactly like the plugin's displace path (PutDisplace -> RecursTile in
	// displace mode: recursion depths {0,4,8}, 128 grid, one PutADisplacetile per reached
	// tile); brush 0 keeps the historical single-tile behavior.
	bool opDisplace(uint zone, uint patch, uint u, uint v, uint displace, std::string &err);
	// DEBUG op: write one raw single-layer record with NO transition solving (negative control
	// for checkSeams and a low-level repair affordance; not a plugin op).
	bool opRawTile(uint zone, uint patch, uint u, uint v, int tile, int rot, std::string &err);
	bool opUndo();
	bool opRedo();
	void endStroke();
	/**
	 * Write one export-prop AppData entry (or delete when newHas=false) and push a
	 * single-entry undo stroke. Raw old/new restore delete-vs-value exactly. Shared by the
	 * Prop panel and paint-script `prop` ops.
	 */
	bool opProp(uint zoneId, uint32 appDataId, bool newHas, const std::string &newValue,
	            std::string &err);
	/** Optional: after prop apply/undo/redo for USE_BOUNDINGBOX (main re-derives footprint). */
	void setPropChangedCallback(void (*cb)(uint zoneId, uint32 appDataId))
	{
		m_PropChangedCb = cb;
	}

	// Tile brush size (0-2 -> the plugin's recursTile depths {0,4,8}; tile mouse strokes and
	// the displace op) and tile group bias (0 = none, 1..12 = bank group).
	void setBrushSize(uint s) { m_BrushSize = s > 2 ? 2 : s; }
	uint brushSize() const { return m_BrushSize; }
	void setTileGroup(uint g) { m_TileGroup = g > 12 ? 12 : g; }
	uint tileGroup() const { return m_TileGroup; }
	// Live lockBorders toggle (plugin LockBorders key; init sets the CLI default).
	void setLockBorders(bool b) { m_LockBorders = b; }
	bool lockBordersOn() const { return m_LockBorders; }

	// Color-brush bitmap mask (CPaintColor loadBrush/setBrushMode port). The mask file is any
	// .tga (grayscale loads as luminance, converted to RGBA like the plugin); loading turns the
	// mask mode on. setBrushMaskMode only turns on when a mask is loaded (plugin semantics);
	// returns the resulting mode.
	bool loadBrushMask(const std::string &fileName, std::string &err);
	void clearBrushMask();
	bool setBrushMaskMode(bool on);
	bool brushMaskMode() const { return m_BrushMaskMode; }
	const std::string &brushMaskName() const { return m_BrushMaskName; }

	// Seam legality report (validation surface): every adjacent non-empty tile pair must agree
	// on the shared corner tile sets (the GetBorderDesc invariant PropagateBorder maintains).
	// Returns the number of illegal seams; prints them to out.
	uint checkSeams(uint zone, FILE *out);
	// Print a color vertex's co-location closure (continuity validation surface).
	bool dumpClosure(uint zone, uint patch, sint32 s, sint32 t, FILE *out);

	// Preload flush (plugin preloadTiles): flush every tile set's 128/256/transition tiles
	// into the attached landscape's driver.
	void preloadTiles(NL3D::IDriver *driver);

	// Stored painter UI flags from the FIRST paint-bearing NeL Patch Painter modifier's local
	// data (RPO_INCLUDE_MESHES 0x4003 / RPO_PRELOAD_TILES 0x4010, BOOL payloads; per-modifier
	// state in the plugin, first one found wins here). -1 = no painter modifier carries the
	// chunk in this file.
	sint storedIncludeMeshes() const { return m_StoredIncludeMeshes; }
	sint storedPreloadTiles() const { return m_StoredPreloadTiles; }

	/**
	 * Where the last edit landed, in world space, with the extent that edit covered
	 * (tile radius for tile/displace ops, brush radius for the colour brush, patch extent
	 * for fills). Feeds the viewer's Zoom-Extents-Selected fallback chain, which wants to
	 * frame "what I just worked on" in paint modes.
	 *
	 * Recorded by the OP LAYER, not by setTile: a transition solve touches a spray of
	 * neighbouring tiles and the interesting point is the one the artist aimed at. Cleared
	 * by init() - a working-set rebuild invalidates the zone ids the point came from.
	 */
	bool lastEditPos(NLMISC::CVector &pos, float &radius) const;

	// Mouse pick: world ray -> (zone, tileId). Uses the display bezier patches.
	bool pickTile(const NLMISC::CVector &pos, const NLMISC::CVector &dir, uint &zone, sint32 &tileId,
	              NLMISC::CVector &hit);
	// Nearest grid tile of a zone to a world point (explicit-hit brush seeding).
	bool nearestTile(uint zone, const NLMISC::CVector &pos, sint32 &tileId);

	// Read a tile (from the pristine carrier, display-space transform applied).
	void getTile(uint zone, sint32 tileId, CTileDescP &desc) const;
	// Read a color vertex (raw 0xAARRGGBB from the pristine carrier); zone id based.
	bool getColor(uint zone, uint patch, sint32 s, sint32 t, uint32 &color) const;

	// Carrier write-back: encode each (possibly tile-mutated) pristine blob into its carrier
	// (0x4001 leaf via encodeRPatchMesh, base 0x08FD via setRPatch). Carriers whose owning
	// zones are all frozen are skipped (never mutated either; ops enforce it per tile).
	bool writeBack(std::string &err);

	// Dirty detection (multi-file save): compare re-encoded pristine to OriginalBytes.
	// Zone id is the landscape/session id (SPaintZoneInput::ZoneId). Frozen zones are never dirty.
	// Also ORs export-prop appdata drift (ROTATE/SYMMETRY/PASSABLE/USE_BOUNDINGBOX) against a
	// per-zone snapshot taken at init and refreshed by markZonesSaved: appdata lives outside
	// the carriers, so blob compare alone cannot see prop edits.
	/**
	 * Geometry edits are invisible to the blob compare below: it re-encodes the RPO pristine
	 * copy, while a vertex move writes the PatchMesh (0x1140 / base 0x0BE0) or a mapper delta
	 * (0x1130). Those chunks are mutated in place, so nothing about them shows up in an RPO
	 * encode. This flag is the geometry half of the dirty signal.
	 */
	/**
	 * Move elements of one zone by PER-ELEMENT object-space deltas.
	 *
	 * Per element rather than one shared delta because rotate and scale give every element its
	 * own; a move simply passes the same value repeated. The whole list lands as a single undo
	 * step, because a selection transform is one action to the artist however many elements it
	 * touched. Bound vertices, and handles that ride a moving corner, must already be excluded
	 * by the caller - this layer trusts the policy rather than re-deriving it.
	 */
	uint opMovePatchElems(uint zoneId, const std::vector<SGeomElemRef> &elems,
	                      const std::vector<NLMISC::CVector> &objDeltas, std::string &err);
	/**
	 * Edit vertex-bind records in the pristine carrier, with an optional geometry snap in
	 * the SAME undo stroke (a new bind writes the bound vertices onto their bindWhere
	 * points; the caller computes the deltas because they come from the evaluated mesh,
	 * which the core does not own). Returns the number of records changed. Bind state
	 * lives in the carrier blob, so dirty detection sees it without markGeomDirty - but
	 * the snap writes go through the Kind 3 geometry path and mark it themselves.
	 */
	uint opEditBinds(uint zoneId, const std::vector<SBindEdit> &binds,
	                 const std::vector<SGeomElemRef> &snapElems,
	                 const std::vector<NLMISC::CVector> &snapDeltas, std::string &err);
	/** Write per-patch edge flag words (no-smooth bit) as one undo stroke. */
	uint opSetEdgeFlags(uint zoneId, const std::vector<SEdgeFlagEdit> &writes, std::string &err);
	/** Read a pristine bind record (the on-disk truth, not the eval copy). */
	bool getVertBind(uint zoneId, uint16 vert, bool &binded, uint32 &type, uint32 &edge,
	                 uint32 &patch, uint32 &primVert) const;
	/** Read a pristine patch's four edge-flag words. */
	bool getPatchEdgeFlags(uint zoneId, uint16 patch, uint32 outFlags[4]) const;
	/**
	 * Push one TOPOLOGY stroke (Kind 6): raw pre/post snapshots of every stream the op
	 * mutated, one record per snapshot. Takes ownership of the pointers (freed with the
	 * core, or when a non-keepUndo init clears history). Undo/redo hand each snapshot to
	 * the restore callback, which re-encodes the matching side into storage; the CALLER of
	 * opUndo/opRedo then checks topoRestorePending() and rebuilds the working set - the
	 * core cannot rebuild the session from inside its own undo replay.
	 */
	void opTopoStroke(const std::vector<STopoSnapshot *> &snaps);
	void setTopoRestoreCb(void (*cb)(const STopoSnapshot &snap, bool useOld))
	{ m_TopoRestoreCb = cb; }
	bool topoRestorePending() const { return m_TopoRestorePending; }
	void clearTopoRestorePending() { m_TopoRestorePending = false; }
	/**
	 * Notified after bind records or edge flags change (forward op, undo or redo), once per
	 * op with the zone id the op addressed. The display re-derives everything downstream
	 * (eval mirror, BindEdges, smooth flags, landscape rebuild) for every node of the
	 * zone's object - same fan-out contract as the geometry callback.
	 */
	void setRpStateChangedCb(void (*cb)(uint zoneId)) { m_RpStateChangedCb = cb; }
	/**
	 * Notified whenever a vertex position changes, forward or by undo/redo.
	 *
	 * The payload is the OBJECT-SPACE DELTA the vertex just took, never an absolute position.
	 * That is not a convenience: the three write targets do not store the same quantity. A
	 * PatchMesh slot holds an absolute position while a mapper record holds a delta from its
	 * own Original, so there is no single stored value the callback could hand over and mean
	 * the same thing. The difference between two values of one target is well defined
	 * whichever target it is, and it is also all a display update needs.
	 */
	void setGeomChangedCb(void (*cb)(uint zoneId, uint16 elemIdx, int elem, const float *objDelta))
	{ m_GeomChangedCb = cb; }

	void markGeomDirty(uint zoneId);
	bool geomDirty(uint zoneId) const;

	bool isZoneDirty(uint zoneId) const;
	/** True when any unfrozen carrier of the listed zone ids differs from its load-time blob. */
	bool anyZoneDirty(const std::vector<uint> &zoneIds) const;
	/** After a successful per-file save: refresh OriginalBytes from the current pristine encode
	 *	so subsequent dirty checks go false (write-target policy unchanged). Also re-snaps
	 *	export-prop appdata. */
	void markZonesSaved(const std::vector<uint> &zoneIds);
	/** Re-snapshot export props for one zone after a live write (footprint re-derive callers
	 *	that already committed the value; optional, dirty also re-reads live). */
	void snapZoneProps(uint zoneId);

	/**
	 * Working-set rebuild helpers (session hub).
	 *
	 * Before re-init after open/close: call writeBack() so carrier blobs hold the current paint
	 * state, then stashOriginalBytes(). After init() on the retained Node stacks (same SLoadedMax
	 * scenes, do not reload files), restoreOriginalBytes() so dirty detection still compares
	 * against the ORIGINAL load-time bytes rather than the just-re-decoded paint state.
	 * Undo is always cleared by init() on any working-set change (session semantics).
	 * Key = SnapLeaf* or Rpo* (same pointer keying as carrier sharing).
	 */
	void stashOriginalBytes(std::map<const void *, std::vector<uint8> > &out) const;
	void restoreOriginalBytes(const std::map<const void *, std::vector<uint8> > &in);
	/** Discard paint on listed zones: restore pristine from OriginalBytes (close without saving). */
	void revertZones(const std::vector<uint> &zoneIds);

	// Carrier tile-record dump (the mechanical verification surface).
	void dumpRpo(FILE *out) const;
	// Bank xref dump: tile -> (tileSet, number, type) for every bank tile.
	void dumpBankXRef(FILE *out) const;
	// Original carrier blob bytes (as loaded), for the surgical-diff check.
	bool dumpCarrierBlob(uint zone, std::vector<uint8> &out) const;

	// Viewer helpers
	uint tileSetCount() const;
	std::string tileSetName(int tileSet) const;
	int tileSetOfTile(uint tile) const; // bank xref; -1 when unknown
	sint32 tileCorners(uint zone, sint32 tileId, NLMISC::CVector corners[4]) const; // -1 fail
	bool zoneFrozen(uint zone) const;
	uint undoDepth() const { return (uint)m_UndoStack.size(); }
	uint strokeSetCount() const { return m_StrokeSets; } // SetTile count in the last op (debug)

private:
	// carrier: the write-target of one node stack (0x4001 leaf or base 0x08FD)
	struct SCarrier
	{
		PIPELINE::MAX::NELPATCH::CRklPatchObject *Rpo; // base 0x08FD carrier (when SnapLeaf NULL)
		PIPELINE::MAX::CStorageRaw *SnapLeaf; // topmost 0x4001 leaf, or NULL
		PIPELINE::MAX::NELPATCH::SRPatchMesh *Pristine;
		std::vector<uint8> OriginalBytes; // raw blob bytes as loaded (0x4001 payload or 0x08FD payload)
		std::vector<uint> Zones; // paint zones sharing this carrier
		bool AnyUnfrozen;
		SCarrier() : Rpo(NULL), SnapLeaf(NULL), Pristine(NULL), AnyUnfrozen(false) { }
	};

	// Snapshot of the four Max-export props (script AppData, string payloads).
	// Present=false means the entry is absent (passable delete-style; usebbox default 0).
	struct SPropSnap
	{
		bool HasRotate, HasSymmetry, HasPassable, HasUseBB;
		std::string Rotate, Symmetry, Passable, UseBB;
		SPropSnap() : HasRotate(false), HasSymmetry(false), HasPassable(false), HasUseBB(false) { }
	};

	struct SZone
	{
		SPaintZoneInput In;
		uint Carrier; // index into m_Carriers
		NL3D::CZoneSymmetrisation Sym;
		std::vector<SPaintTile> Meta; // numPatches * ZP_NUM_TILE_SEL
		SPropSnap PropSnap; // export-prop baseline for dirty
	};

	std::vector<SZone> m_Zones;
	std::vector<SCarrier> m_Carriers;
	NL3D::CTileBank *m_Bank;
	NL3D::CLandscape *m_Landscape;
	bool m_LockBorders;
	uint32 m_TileCycle; // selectTile cycle counter (plugin TileIndex)
	int m_StrokeRotation; // plugin EPM_PaintMouseProc::Rotation
	sint32 m_StrokeOldTile;
	sint32 m_StrokeOldZone;
	uint m_BrushSize; // 0-2 (plugin brushSize)
	uint m_TileGroup; // 0 = none, 1..12 (plugin TileGroup)
	// color-brush mask state (CPaintColor _BrushBitmap/_bBrush port)
	NLMISC::CBitmap m_BrushMask;
	bool m_BrushMaskLoaded;
	bool m_BrushMaskMode;
	std::string m_BrushMaskName;
	sint m_StoredIncludeMeshes; // -1 unknown / 0 / 1 (RPO_INCLUDE_MESHES 0x4003)
	sint m_StoredPreloadTiles; // -1 unknown / 0 / 1 (RPO_PRELOAD_TILES 0x4010)
	// last-edit marker (see lastEditPos)
	bool m_HaveLastEdit;
	NLMISC::CVector m_LastEditPos;
	float m_LastEditRadius;

	// Per-tileset group tile lists (paint_ui CBankCont port): set-local 128/256 indices whose
	// bank tile carries the group flag and a diffuse name.
	std::vector<std::vector<std::vector<uint> > > m_GroupTile128; // [tileSet][group][i]
	std::vector<std::vector<std::vector<uint> > > m_GroupTile256;

	// undo
	// opResetZone sweeps every patch through the fill ops, whose own endStroke must not
	// split the sweep into per-patch strokes - one reset, one undo.
	bool m_SuspendStroke;
	std::vector<SUndoTile> m_CurStroke;
	std::deque<std::vector<SUndoTile> > m_UndoStack;
	std::deque<std::vector<SUndoTile> > m_RedoStack;
	uint m_StrokeSets;
	void (*m_PropChangedCb)(uint zoneId, uint32 appDataId);

	// display mirror batch (CNelPatchChanger port; tiles + colors per (zoneId, patch))
	typedef std::map<std::pair<int, int>, std::vector<NL3D::CTileElement> > TChangeMap;
	typedef std::map<std::pair<int, int>, std::vector<NL3D::CTileColor> > TColorChangeMap;
	TChangeMap m_Changes;
	TColorChangeMap m_ColorChanges;

	// --- internals (ports of the plugin functions of the same names) ---
	uint orderS(uint zone, uint patch) const;
	uint orderT(uint zone, uint patch) const;
	const PIPELINE::MAX::NELPATCH::SRPatchMesh *pristineOf(uint zone) const;
	PIPELINE::MAX::NELPATCH::SRPatchMesh *pristineOf(uint zone);
	// NB: private tile accessors take the ZONES-VECTOR INDEX; the public getTile takes a zone ID.
	void getTileIdx(uint zoneIdx, sint32 tileId, CTileDescP &desc) const;
	void getTileRaw(uint zone, sint32 tileId, CTileDescP &desc) const; // no display transform
	void setTileDesc(uint zone, sint32 tileId, const CTileDescP &desc); // pristine write
	// Raw pristine tile-record access for the byte-exact undo snapshots (SUndoTile::*Raw*).
	PIPELINE::MAX::NELPATCH::SRpoTile *pristineTileRecord(uint zone, sint32 tileId);
	void captureRawTile(uint zone, sint32 tileId, uint16 &num, uint16 &flags, uint8 &noise,
	                    sint32 tile[3], sint32 rot[3]);
	void restoreRawTile(const SUndoTile &u, bool useOld);
	void transformDesc(CTileDescP &desc, bool symmetry, uint rotate, uint zone, sint32 tileId) const;
	void transformInvDesc(CTileDescP &desc, bool symmetry, uint rotate, uint zone, sint32 tileId) const;
	void setTile(uint zone, sint32 tileId, const CTileDescP &desc,
	             std::vector<SUndoTile> *backupStack, bool undo, bool updateDisplace = false);
	std::vector<NL3D::CTileElement> *changeTileArray(uint zone, uint patch);
	std::vector<NL3D::CTileColor> *changeColorArray(uint zone, uint patch);
	void applyChanges();
	int selectTile(uint tileSet, bool selectCycle, bool _256) { return selectTile(tileSet, selectCycle, _256, m_TileGroup); }
	int selectTile(uint tileSet, bool selectCycle, bool _256, uint group);
	// colors (index-based internals)
	uint32 getColorRaw(uint zoneIdx, uint patch, sint32 s, sint32 t) const;
	void setColorRaw(uint zoneIdx, uint patch, sint32 s, sint32 t, uint32 color, bool undo);
	// co-location closure of a grid vertex across seams (getVertexInNeighbor port, transitive)
	void vertexClosure(uint zoneIdx, SPaintTile *tile, int vertexId, std::vector<SColorSlot> &out);
	// blend once, write the whole closure identically; false when the closure touches a frozen
	// zone. lockBorders refusal happens at the callers (brush/vertex ops) via
	// colorVertexBorderLocked. The plugin's fill wrote vertices with no lock checks at all,
	// so opFillColor must stay exempt.
	bool setVertexColorShared(const std::vector<SColorSlot> &slots, NLMISC::CRGBA color, uint blend);
	// paint_vcolor.cpp port: under lockBorders a vertex adjacent to an OPEN (unbound) or
	// frozen border refuses to paint. Decided from the vertex's owner tile exactly like the
	// plugin: interior vertex = tile whose top-left corner it is (left/top sides); right/
	// bottom patch-edge vertices = the edge tile's right/bottom sides.
	bool colorVertexBorderLocked(uint zoneIdx, uint patch, sint32 s, sint32 t) const;
	bool fillTileImpl(uint zoneIdx, uint patch, int tileSet, int rot, bool _256);
	bool isLockedEx(SPaintTile *tile);
	bool isLocked256(SPaintTile *tile);
	void displaceOne(SPaintTile *tile, uint displace);
	// RecursTile displace-mode port (PutDisplace path): displaceOne per reached tile, 128 grid.
	void recursDisplace(SPaintTile *tile, uint displace, int recurs, std::set<SPaintTile *> &alreadyRecursed);
	bool isLocked(SPaintTile *tile, uint8 mask = 0xff) const;
	bool getBorderDesc(SPaintTile *tile, CTileSetIdx corner[4], NL3D::CTileSet::TFlagBorder border[4][3],
	                   CTileDescP *index);
	const NL3D::CTileSetTransition *findTransition(int tileSet, int rotate, const NL3D::CTileSet::TFlagBorder *border) const;
	int getLayer(SPaintTile *tile, int border, int tileSet, int rotate);
	bool propagateBorder(SPaintTile *tile, int curRotation, int curTileSet, std::set<SPaintTile *> &visited,
	                     std::vector<SUndoTile> &backupStack, bool recurseNoDiff = true);
	bool clearATile(SPaintTile *tile, bool _256, bool force128 = false);
	bool putATile(SPaintTile *tile, int tileSet, int curRotation, bool selectCycle,
	              std::set<SPaintTile *> &visited, bool _256);
	void recursTile(SPaintTile *tile, int tileSet, int recurs, std::set<SPaintTile *> &alreadyRecursed,
	                bool first, int rotation, bool _256);
	uint8 calcRotPath(SPaintTile *from, SPaintTile *to, int depth, int rotate, int &dx, int &dy, int &cost);
	SPaintTile *metaAt(uint zone, sint32 tileId);
	// last-edit marker helpers (zone index based, like the rest of the private layer)
	void noteEditAt(const NLMISC::CVector &pos, float radius);
	void noteEditTile(uint zoneIdx, sint32 tileId);
	void noteEditPatch(uint zoneIdx, uint patch);
	// Self-instance duals: other zones sharing the same carrier+tileId (same pristine slot,
	// distinct meta graphs). Paint must visit/propagate through every dual or the
	// instance-side seam graph is under-constrained relative to the primary-side path.
	void collectDuals(SPaintTile *tile, std::vector<SPaintTile *> &out) const;
	void markVisitedWithDuals(SPaintTile *tile, std::set<SPaintTile *> &visited) const;
	// Local seam legality around a tile (GetBorderDesc corner TileSet identity, same as checkSeams).
	bool tileSeamsLegal(SPaintTile *tile) const;
	// Only tiles actually written this put (backup stack), not duals merely marked visited.
	// Mirrored duals can look "illegal" under getTileIdx(sym) while authored space is fine.
	bool writtenSeamsLegal(const std::vector<SUndoTile> &backupStack) const;
	void buildMeta(std::string &err);
	void stitchEdge(uint zi, uint p, uint e, uint zj, uint pp, uint ee, int dividEdge, int offsetEdge);
	int getBindedEdge(uint zone, int nPatch, int nVertInPatch) const;
	void applyUndoList(const std::vector<SUndoTile> &list, bool useOld);
	// Roll back m_CurStroke entries written after `mark` (desc + raw) without pushing
	// further undo and without committing. Used by putATile abort paths so a refused
	// paint leaves no stroke pollution and restores pristine bytes by construction.
	void abortStrokeTo(size_t mark);
	// Export-prop snapshot helpers (appdata outside carriers)
	static void readPropSnap(PIPELINE::MAX::BUILTIN::CNodeImpl *node, SPropSnap &out);
	bool propsDirty(uint zoneIdx) const;
	void applyGeomUndo(const SUndoTile &rec, bool useOld);
	void applyBindUndo(const SUndoTile &rec, bool useOld);
	void applyEdgeFlagUndo(const SUndoTile &rec, bool useOld);
	void applyTopoUndo(const SUndoTile &rec, bool useOld);
	std::set<uint> m_GeomDirty; // zone ids with an uncommitted geometry write
	void (*m_GeomChangedCb)(uint zoneId, uint16 elemIdx, int elem, const float *objDelta);
	void (*m_RpStateChangedCb)(uint zoneId);
	// Topology snapshots (Kind 6 payloads). The registry owns them for the session: a
	// bounded-undo eviction may strand one (freed with the registry), which is bounded
	// and simpler than refcounting strokes across two deques.
	std::vector<STopoSnapshot *> m_TopoSnaps;
	void (*m_TopoRestoreCb)(const STopoSnapshot &snap, bool useOld);
	bool m_TopoRestorePending;
	void clearTopoSnaps();
};

} /* namespace ZPPAINT */

#endif /* ZONE_PAINTER_PAINT_CORE_H */

/* end of file */
