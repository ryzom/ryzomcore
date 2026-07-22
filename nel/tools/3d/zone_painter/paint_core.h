/**
 * \file paint_core.h
 * \brief Tile painting core of the standalone zone painter (design doc §14-paint, P3b)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Port of the in-Max painter's TILE path (plugin_max/nel_patch_paint paint.cpp) onto the typed
 * pipeline_max model: the EPM_PaintTile/metaTile adjacency graph, the tile put/transition
 * recursion (PutATile/PropagateBorder/RecursTile with the bank xref), 128/256 tiles, rotation,
 * frozen-zone locks, the CNelPatchChanger live-landscape mirror, a bounded undo, and the
 * carrier write-back that feeds the whole-file save.
 *
 * PRISTINE-COPY DISCIPLINE (correctness-critical, see the P3b plan): per unique carrier blob
 * (the P2 write-target: topmost modifier slot whose 0x2512->0x1000 local data carries an
 * RFINALPATCH 0x4001, else the base RPO 0x08FD) ONE SRPatchMesh is decoded from the raw bytes
 * and kept as the authoritative paint state. Tile ops mutate ONLY the tile-record fields of
 * that pristine copy (SRpoTile Num/Flags/Noise/Layer[].Tile/Rotate); the evaluated
 * SEvalPatch (whose PatchMesh positions and rp.Verts bind caches the eval/refresh path may
 * rewrite) is used for topology/display only and is NEVER encoded back. An untouched pristine
 * copy re-encodes byte-identical (P1), so the paint save path is a proven no-op for a null
 * edit.
 *
 * DISPLAY INSTANCES / SHARED BACKING (ui M4a/M12): carriers are keyed by the leaf or base RPO
 * POINTER. Multiple SPaintZoneInput entries that resolve to the same pointer (same Node stack
 * — e.g. ecosystem brick self-instances at world offsets / rotations) share ONE pristine
 * SRPatchMesh and one SCarrier::Zones membership list. setTile / setColorRaw fan the live-
 * landscape mirror to every zone in that list (with per-zone transformDesc for tile orients);
 * writeBack encodes the carrier once. Ops addressed at any instance zone id therefore mutate
 * the same paint state as the primary. Per-zone Symmetry/Rotate (plugin EPM_Mesh appdata,
 * land CZoneRegion Flip/Rot) assemble the transformDesc display space: GetTile uses
 * transformDesc(sym, 4-rot), SetTile uses transformInvDesc(sym, 4-rot) for the Max write and
 * remaps the landscape mirror through each shared zone's transform (plugin SetTile port).
 * Color display remaps S under symmetry only (plugin paint_vcolor). tileDesc mapping and
 * write-target policy are unchanged.
 *
 * tileDesc <-> SRpoTile mapping (plugin nel_patch_lib/nel_patch_mesh.h <-> nelpatch/rpo_data.h,
 * the on-disk v9 record P1 encodes):
 *   _Num                 <-> SRpoTile.Num       used-layer count 0..3 (0 = empty tile)
 *   _Flags bits 0-2      <-> SRpoTile.Flags     tile case: 0 = 128x128, 1..4 = 256 quadrant+1
 *   _Flags bits 3-6      <-> SRpoTile.Flags     displace map index 0..15 ...
 *   (displace)           <-> SRpoTile.Noise     ... duplicated in the v9 noise byte; the
 *                            original loader's setDisplace(Noise) makes Noise authoritative on
 *                            read, so displace READS from Noise and writes keep both in sync
 *   _Flags bits 7-15     <-> SRpoTile.Flags     legacy/dead bits many corpus records carry;
 *                            the original loader keeps them in memory and re-saves them, so
 *                            descs preserve them verbatim through get -> set (fresh descs
 *                            start zeroed, exactly like the plugin's tileDesc())
 *   _MatIDTab[l].Tile    <-> SRpoTile.Layer[l].Tile    bank tile index (layer 0 = base 1111)
 *   _MatIDTab[l].Rotate  <-> SRpoTile.Layer[l].Rotate  0..3 CCW
 *   untouched on edit: Layer[l].Reserved, OldA/OldB (write-direction retention fields).
 * Grid order: SRpoPatch.Tiles is u + v*OrderS (OrderS = 1<<NbTilesU); Colors (P3c) is
 * u + v*(OrderS+1), 0x00RRGGBB.
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
namespace BUILTIN {
class CNodeImpl;
}
namespace NELPATCH {
class CRklPatchObject;
struct SRPatchMesh;
struct SPatchMesh;
}
}
}

namespace ZPPAINT {

// Plugin grid constants (nel_patch_mesh.h): the metaTile grid is a fixed 16x16 per patch, only
// OrderS x OrderT entries used; tile id = patch*NUM_TILE_SEL + v*MAX_TILE_IN_PATCH + u.
#define ZP_MAX_TILE_IN_PATCH 16
#define ZP_NUM_TILE_SEL (ZP_MAX_TILE_IN_PATCH * ZP_MAX_TILE_IN_PATCH)

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
	sint16 Zone;   // paint zone index (== landscape zone id)
	uint8 U, V;
	bool Frozen;
	uint8 Locked; // per-edge bit: neighbor missing or frozen
	SPaintTile *Voisins[4];
	uint8 Rotate[4];
	NLMISC::CVector Center; // world center of the tile quad (color brush range test)
	float Radius;           // max corner distance from Center

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
// One paintable zone as the core sees it (built by the tool main from the P3a scene assembly).

struct SPaintZoneInput
{
	PIPELINE::MAX::BUILTIN::CNodeImpl *Node;
	bool Frozen;
	uint ZoneId; // node collection index == landscape zone id
	std::string Name;
	const std::vector<NL3D::CPatchInfo> *Patches;       // display patchinfo, world space
	const PIPELINE::MAX::NELPATCH::SPatchMesh *Pm;      // evaluated topology (edges, patch verts)
	const PIPELINE::MAX::NELPATCH::SRPatchMesh *EvalRp; // evaluated rp (binds, tile orders)
	// Display-space transform of this zone (plugin EPM_Mesh::Rotate/Symmetry from
	// NEL3D_APPDATA_ZONE_ROTATE / _SYMMETRY; land placement CZoneRegion Rot/Flip).
	// Primary authored bricks are (0, false). Rotated/mirrored self-instances set these
	// so getTile/setTile assemble transformDesc (ui M12).
	uint Rotate;   // 0..3, 90° CCW steps
	bool Symmetry; // mirror (Flip)
	SPaintZoneInput() : Node(NULL), Frozen(false), ZoneId(0), Patches(NULL), Pm(NULL), EvalRp(NULL),
	                    Rotate(0), Symmetry(false) { }
};

// Undo delta: one tile-record or color-vertex change (bounded LIFO of strokes).
struct SUndoTile
{
	uint8 Kind; // 0 = tile record, 1 = color vertex
	uint Zone;
	sint32 TileId; // tile kind
	CTileDescP Old;
	CTileDescP New;
	sint32 Patch, S, T;         // color kind: grid slot
	uint32 OldColor, NewColor;  // color kind: raw 0xAARRGGBB values
	SUndoTile() : Kind(0), Zone(0), TileId(-1), Patch(-1), S(0), T(0), OldColor(0), NewColor(0) { }
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
	bool init(const std::vector<SPaintZoneInput> &zones, NL3D::CTileBank *bank,
	          float cellSize, float snap, bool lockBorders, std::string &err);

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
	// topVector), sampled bilinearly, luminance mean (R+G+B)/3 scaling the blend — an all-white
	// mask is bit-identical to no mask (blend*255/255). Per-hit stamp; not stroke-oriented.
	bool opColorBrush(uint zone, sint32 seedTileId, const NLMISC::CVector &hit, float radius,
	                  NLMISC::CRGBA color, uint hardness, uint opacity, std::string &err);
	// Region fills (CFillPatch ports): tile fill (tileSet -1 = clear; incompatible borders are
	// cleared, the plugin rule), color fill, displace fill.
	bool opFillTile(uint zone, uint patch, int tileSet, int rot, bool _256, std::string &err);
	bool opFillColor(uint zone, uint patch, NLMISC::CRGBA color, uint blend, std::string &err);
	bool opFillDisplace(uint zone, uint patch, uint displace, std::string &err);
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
	// state in the plugin — the first one found wins here). -1 = no painter modifier carries
	// the chunk in this file.
	sint storedIncludeMeshes() const { return m_StoredIncludeMeshes; }
	sint storedPreloadTiles() const { return m_StoredPreloadTiles; }

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

	// Dirty detection (ui M6b multi-file save): compare re-encoded pristine to OriginalBytes.
	// Zone id is the landscape/session id (SPaintZoneInput::ZoneId). Frozen zones are never dirty.
	bool isZoneDirty(uint zoneId) const;
	/** True when any unfrozen carrier of the listed zone ids differs from its load-time blob. */
	bool anyZoneDirty(const std::vector<uint> &zoneIds) const;
	/** After a successful per-file save: refresh OriginalBytes from the current pristine encode
	 *	so subsequent dirty checks go false (write-target policy unchanged). */
	void markZonesSaved(const std::vector<uint> &zoneIds);

	/**
	 * Working-set rebuild helpers (ui M11a session hub).
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
	// carrier: the P2 write-target of one node stack
	struct SCarrier
	{
		PIPELINE::MAX::NELPATCH::CRklPatchObject *Rpo; // base 0x08FD carrier (when SnapLeaf NULL)
		PIPELINE::MAX::CStorageRaw *SnapLeaf;          // topmost 0x4001 leaf, or NULL
		PIPELINE::MAX::NELPATCH::SRPatchMesh *Pristine;
		std::vector<uint8> OriginalBytes; // raw blob bytes as loaded (0x4001 payload or 0x08FD payload)
		std::vector<uint> Zones;          // paint zones sharing this carrier
		bool AnyUnfrozen;
		SCarrier() : Rpo(NULL), SnapLeaf(NULL), Pristine(NULL), AnyUnfrozen(false) { }
	};

	struct SZone
	{
		SPaintZoneInput In;
		uint Carrier; // index into m_Carriers
		NL3D::CZoneSymmetrisation Sym;
		std::vector<SPaintTile> Meta; // numPatches * ZP_NUM_TILE_SEL
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
	uint m_BrushSize;  // 0-2 (plugin brushSize)
	uint m_TileGroup;  // 0 = none, 1..12 (plugin TileGroup)
	// color-brush mask state (CPaintColor _BrushBitmap/_bBrush port)
	NLMISC::CBitmap m_BrushMask;
	bool m_BrushMaskLoaded;
	bool m_BrushMaskMode;
	std::string m_BrushMaskName;
	sint m_StoredIncludeMeshes; // -1 unknown / 0 / 1 (RPO_INCLUDE_MESHES 0x4003)
	sint m_StoredPreloadTiles;  // -1 unknown / 0 / 1 (RPO_PRELOAD_TILES 0x4010)

	// Per-tileset group tile lists (paint_ui CBankCont port): set-local 128/256 indices whose
	// bank tile carries the group flag and a diffuse name.
	std::vector<std::vector<std::vector<uint> > > m_GroupTile128; // [tileSet][group][i]
	std::vector<std::vector<std::vector<uint> > > m_GroupTile256;

	// undo
	std::vector<SUndoTile> m_CurStroke;
	std::deque<std::vector<SUndoTile> > m_UndoStack;
	std::deque<std::vector<SUndoTile> > m_RedoStack;
	uint m_StrokeSets;

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
	// zone (or, under lockBorders, an open/frozen border)
	bool setVertexColorShared(const std::vector<SColorSlot> &slots, NLMISC::CRGBA color, uint blend);
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
	// Self-instance duals: other zones sharing the same carrier+tileId (same pristine slot,
	// distinct meta graphs). M13b — paint must visit/propagate through every dual or the
	// instance-side seam graph is under-constrained relative to the primary-side path.
	void collectDuals(SPaintTile *tile, std::vector<SPaintTile *> &out) const;
	void markVisitedWithDuals(SPaintTile *tile, std::set<SPaintTile *> &visited) const;
	// Local seam legality around a tile (GetBorderDesc corner TileSet identity, same as checkSeams).
	bool tileSeamsLegal(SPaintTile *tile) const;
	// Only tiles actually written this put (backup stack), not duals merely marked visited —
	// mirrored duals can look "illegal" under getTileIdx(sym) while authored space is fine.
	bool writtenSeamsLegal(const std::vector<SUndoTile> &backupStack) const;
	void buildMeta(std::string &err);
	void stitchEdge(uint zi, uint p, uint e, uint zj, uint pp, uint ee, int dividEdge, int offsetEdge);
	int getBindedEdge(uint zone, int nPatch, int nVertInPatch) const;
	void applyUndoList(const std::vector<SUndoTile> &list, bool useOld);
};

} /* namespace ZPPAINT */

#endif /* ZONE_PAINTER_PAINT_CORE_H */

/* end of file */
