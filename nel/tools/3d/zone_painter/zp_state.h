/**
 * \file zp_state.h
 * \brief Cross-TU state, types, and forward decls for zone_painter.
 * \author Jan Boon (Kaetemi)
 *
 * Shared header for the tool's implementation TUs: extern globals for the live session,
 * CLI settings, board/footprint state, plus the struct/enum definitions they operate on.
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

#ifndef ZONE_PAINTER_ZP_STATE_H
#define ZONE_PAINTER_ZP_STATE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/vector.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/patch.h>
#include <nel/3d/tile_bank.h>

#include <string>
#include <vector>
#include <map>
#include <set>

// Cross-module workspace + patch_eval + script_api types the extern decls reference.
// (We include full defs when the definitions are cheap - forward-decls fail for
// nested types the shared structs embed.)
#include "workspace_discovery.h"
#include "../pipeline_max_export_common/patch_eval.h"

// Forward decls (heavy headers pulled in by the .cpp TUs themselves)
namespace NL3D
{
	class CTileBank;
	class CLandscapeModel;
	class CCamera;
	class UDriver;
	class UScene;
	class IDriver;
	class CViewport;
	class CNavMouseListener;
}

namespace PMAXLOAD
{
	struct SLoadedMax;
}

namespace PIPELINE
{
	namespace MAX
	{
		class CScene;
		namespace BUILTIN
		{
			class CNodeImpl;
		}
	}
}

namespace ZPPAINT
{
	class CPaintCore;
	struct SPaintZoneInput;
}

namespace ZPUI
{
	struct SPaintUIBridge;
	class CEditorUI;
	// ESessionCellState comes in via startup_ui.h (included by TUs that need it).
	enum ESessionCellState;
}

namespace ZPSCRIPT
{
	struct SScriptHost;
	struct SZoneInfo;
}

// ---------------------------------------------------------------------------------------------
// Key bindings.
//
// A binding is one uint: `TKey | (modifiers << 16)`. A plugin-era keys.cfg carries plain key
// values, so it still parses verbatim (modifiers 0). Modifier matching is EXACT - Z and
// Shift+Z are different bindings, which is what lets Z and Shift+Z mean different things
// without the ad-hoc "and shift is not held" guards the viewer used to need. 0 = unbound.
#define ZPKM_CTRL 0x0001
#define ZPKM_SHIFT 0x0002
#define ZPKM_ALT 0x0004
#define ZPK_MODS(binding) (((binding) >> 16) & 0x7)
#define ZPK_KEY(binding) ((binding) & 0xffff)
#define ZPK_BIND(key, mods) ((uint)(key) | ((uint)(mods) << 16))

// Mode scope. Each action names the paint modes it is live in; `kPainterKeyModes` holds the
// mask. Bit N = CPaintMouseListener mode N (0 Tile, 1 Colour, 2 Displace, 3 Prop).
//
// This is the dimension the patch-edit sub-object levels need: `1`-`5` and `W`/`E`/`R` are
// Selection-level and transform keys, and the painter already spends the digit row on
// tile sets. Scoping the paint bindings to the PAINT modes lets both coexist - the digits
// keep selecting tile sets while painting and are free to mean sub-object levels once the
// patch-edit modes exist. Nothing changes today: no mode outside 0-3 exists yet, so
// ZPKS_PAINT and ZPKS_ANY currently select the same set.
#define ZPKS_PAINT 0x0f // Tile | Colour | Displace | Prop
#define ZPKS_PATCH 0x10 // Patch edit (sub-object levels; the digit row means something else)
#define ZPKS_ANY 0xff // live in every mode

// ---------------------------------------------------------------------------------------------
// TPainterKey enum (paint_ui.cpp PainterKeys port, plus the tool's own actions)

enum TPainterKey
{
	ZPK_Select = 0,
	ZPK_Pick,
	ZPK_Fill0,
	ZPK_Fill1,
	ZPK_Fill2,
	ZPK_Fill3,
	ZPK_MModeTile,
	ZPK_MModeColor,
	ZPK_MModeDisplace,
	ZPK_MModeProp,
	ZPK_ToggleColor,
	ZPK_SizeUp,
	ZPK_SizeDown,
	ZPK_ToggleTileSize,
	ZPK_GroupUp,
	ZPK_GroupDown,
	ZPK_BackgroundColor,
	ZPK_ToggleArrows,
	ZPK_HardnessUp,
	ZPK_HardnessDown,
	ZPK_OpacityUp,
	ZPK_OpacityDown,
	ZPK_Zouille,
	ZPK_AutomaticLighting,
	ZPK_SelectColorBrush,
	ZPK_ToggleColorBrushMode,
	ZPK_LockBorders,
	ZPK_ZoomIn,
	ZPK_ZoomOut,
	ZPK_GetState,
	ZPK_ResetPatch,
	ZPK_ToggleUI,
	ZPK_SeasonNext,
	ZPK_TogglePalette,
	ZPK_ToggleBoard,
	ZPK_ZoomExtentsSel,
	// Actions that used to be hardcoded in the viewer loop. They live in the table so they
	// can be rebound and, more to the point, so they can be MODE-SCOPED.
	ZPK_TileSetPrev,
	ZPK_TileSetNext,
	ZPK_TileSetDigits, // base of the 0-9 run: digit k selects tile set k
	ZPK_DisplacePrev,
	ZPK_DisplaceNext,
	ZPK_Undo,
	ZPK_Redo,
	ZPK_Redo2, // second redo binding; the tool shipped Ctrl+E before the usual Ctrl+Y
	ZPK_ViewUndo,
	ZPK_ViewRedo,
	ZPK_Screenshot,
	// Patch edit. The mode switch is ZPKS_ANY like the other four; the sub-object digits are
	// ZPKS_PATCH, which is the whole point of the scope mask - they share the digit row with
	// tile-set selection and only one of the two is ever live.
	ZPK_MModePatch,
	ZPK_SubObjDigits, // base of the 1-5 run: digit k selects sub-object level k-1 (EP_*)
	// Transform keys. ZPKS_PATCH, which is what lets R mean SCALE here while it still
	// means Prop mode in the paint modes - the same trick the digit row uses.
	ZPK_XformMove,
	ZPK_XformRotate,
	ZPK_XformScale,
	ZPK_KeyCounter
};

// ---------------------------------------------------------------------------------------------
// Shared struct definitions

struct SContextFile
{
	std::string Path;
	std::string Basename;
	int CellX, CellY;
	bool TranslateGeom;
	uint Rot;
	bool Mirror;
	int CellsW, CellsH;
	std::vector<bool> Mask;
	PMAXLOAD::SLoadedMax *Lm;
	SContextFile() : CellX(0), CellY(0), TranslateGeom(false), Rot(0), Mirror(false),
	                 CellsW(1), CellsH(1), Lm(NULL) {}
};

struct SSessionHintCell
{
	int CellX, CellY;
	std::string Basename;
	SSessionHintCell() : CellX(0), CellY(0) {}
	SSessionHintCell(int x, int y, const std::string &b) : CellX(x), CellY(y), Basename(b) {}
};

struct SEditableFileInfo
{
	std::string Path;
	std::string Basename;
	PMAXLOAD::SLoadedMax *Lm;
	std::vector<uint> ZoneIds;
	bool Editable;
	int CellX, CellY;
	int CellsW, CellsH;
	std::vector<bool> Mask;
	float PlacedDX, PlacedDY;
	SEditableFileInfo() : Lm(NULL), Editable(true), CellX(0), CellY(0), CellsW(1), CellsH(1),
	                      PlacedDX(0.f), PlacedDY(0.f) {}
};

/**
 * Id allocator ONLY: session-added nodes take landscape ids from here, above the per-file
 * bases (file index * 1000), which is what caps a session at ten open files.
 *
 * It says nothing about what a node IS. Editability is SPaintZone::Editable, object identity
 * is the Node pointer, and whether a node came from the file is SPaintZone::InFile. Comparing
 * a zone id against this to decide any of those is how the display frame and the write frame
 * drifted apart in the first place; the ids are landscape ids and nothing more.
 */
static const uint kInstanceZoneIdBase = 10000;

struct SInstancePlace
{
	int CellX, CellY;
	uint Rot;
	bool Mirror;
	std::string SourceBasename;
	SInstancePlace() : CellX(0), CellY(0), Rot(0), Mirror(false) { }
	SInstancePlace(int x, int y, uint r, bool m) : CellX(x), CellY(y), Rot(r), Mirror(m) { }
};

struct SNeighborHint
{
	int Dx, Dy;
	std::string Basename;
	uint Rot;
	bool Mirror;
	SNeighborHint() : Dx(0), Dy(0), Rot(0), Mirror(false) {}
	SNeighborHint(int dx, int dy, const std::string &b)
		: Dx(dx), Dy(dy), Basename(b), Rot(0), Mirror(false) {}
};

/**
 * One NODE: a placement of an object in the session, as a node.
 *
 * `Node` is the object handle - several SPaintZone entries may carry the same pointer, which
 * means they show the SAME storage from different places on the board. There is deliberately
 * no "primary" among them: whichever one an artist points at is the one they are editing, and
 * an edit made through any of them changes all of them. `ZoneId` is a landscape id and nothing
 * more; it does not encode whether a node is editable or a copy.
 */
struct SPaintZone
{
	PIPELINE::MAX::BUILTIN::CNodeImpl *Node;
	bool Frozen;
	/**
	 * The object may be written through this node.
	 *
	 * A property of the OBJECT - its file is open for editing and this node is not read-only
	 * context - never a comparison against an id range. Every node of an editable object is
	 * editable, which is what makes a second node of the same object a working viewpoint
	 * rather than a decoration.
	 */
	bool Editable;
	/**
	 * This node exists in the .max on disk; nodes the session added do not.
	 *
	 * Provenance, not rank. A board placement of an already-open object is a node in every
	 * sense - editable, selectable, and as good a viewpoint on the object as any other - but
	 * it is not written back, because the OBJECT is written once and the file never held a
	 * second node for it. This is the only remaining reason to tell nodes of one object
	 * apart, and it is a fact about the file rather than about an id range.
	 */
	bool InFile;
	std::string Name;
	uint ZoneId;
	std::vector<NL3D::CPatchInfo> Patches;
	std::vector<NL3D::CBorderVertex> BorderVertices;
	SEvalPatch Ep;
	/// offsetTM * nodeTM, as buildPatchInfo used it. Kept so a world-space edit can be taken
	/// back to the object space the .max actually stores.
	MAXMATH::Matrix3M ObjectTM;
	/// Object space -> DISPLAYED world space: ObjectTM followed by everything the session did
	/// to put this zone where it is on the board (file placement, instance rot/mirror).
	///
	/// This is the node half of the node/object split. ObjectTM alone is the file's own
	/// authored frame, which is the DISPLAY frame only for a file sitting at the board origin
	/// with no transform. Anything that recomputes a display position from a stored object
	/// position must use this, or a placed file's geometry lands one board cell away from
	/// where it is drawn. Every routine that moves display geometry composes onto it, so the
	/// matrix and the vertices can never disagree.
	MAXMATH::Matrix3M DisplayTM;
	uint Rotate;
	bool Symmetry;
	SPaintZone()
	    : Node(NULL), Frozen(false), Editable(false), InFile(true), ZoneId(0),
	      ObjectTM(MAXMATH::Matrix3M::identity()),
	      DisplayTM(MAXMATH::Matrix3M::identity()), Rotate(0), Symmetry(false)
	{
	}
};

struct SPlaceContextSpec
{
	int Dx, Dy;
	std::string Basename;
	int CellsW, CellsH;
	std::vector<bool> Mask;
	uint Rot;
	bool Mirror;
	SPlaceContextSpec() : Dx(0), Dy(0), CellsW(1), CellsH(1), Rot(0), Mirror(false) {}
};

struct SOpenEditableSpec
{
	int Cx, Cy;
	std::string Basename;
	SOpenEditableSpec() : Cx(0), Cy(0) {}
};

struct SPaintCtx
{
	bool Active;
	ZPPAINT::CPaintCore *Core;
	class CPaintMouseListener *Paint;
	std::string InputPath;
	std::string SavePath;
	PIPELINE::MAX::CScene *Scene;
	bool InteractiveSave;
	bool WantThumbnail;
	NL3D::UDriver *UDriver;
	NL3D::UScene *UScene;
	NL3D::CLandscapeModel *Land;
	NL3D::CCamera *Camera;
	std::vector<SPaintZone> *Zones;
	NL3D::CTileBank *Bank;
	std::string BankPath;
	std::vector<std::string> *AvailableSeasons;
	SPaintCtx()
		: Active(false), Core(NULL), Paint(NULL), Scene(NULL), InteractiveSave(false),
		  WantThumbnail(false), UDriver(NULL), UScene(NULL), Land(NULL), Camera(NULL), Zones(NULL),
		  Bank(NULL), AvailableSeasons(NULL)
	{
	}
};

struct SBoardOpScope
{
	SBoardOpScope();
	~SBoardOpScope();
};

struct SZoneProps
{
	int Rotate;
	bool Symmetry;
	bool Passable;
	bool UseBoundingBox;
	bool HasRotate, HasSymmetry, HasPassable, HasUseBB;
	SZoneProps()
		: Rotate(0), Symmetry(false), Passable(false), UseBoundingBox(false),
		  HasRotate(false), HasSymmetry(false), HasPassable(false), HasUseBB(false)
	{
	}
};

struct SZpBoundEdge
{
	uint Patch;
	uint Edge;
	sint32 V0;
	sint32 V1;
	bool Reverse;
};

struct SScriptPumpCtx
{
	NL3D::UDriver *Driver;
	NL3D::UScene *Scene;
	ZPUI::CEditorUI *Ui;
	NL3D::CNavMouseListener *Nav;
	NLMISC::TTime LastPump;
	SScriptPumpCtx() : Driver(NULL), Scene(NULL), Ui(NULL), Nav(NULL), LastPump(0) { }
};

// ---------------------------------------------------------------------------------------------
// Window constants used by viewer/main

extern const uint kMainWidth;
extern const uint kMainHeight;

// ---------------------------------------------------------------------------------------------
// Global state. Definitions live in main.cpp.

extern bool g_verbose;
extern int g_ViewerScriptRc;
extern NLMISC::CRGBA g_ViewerBrushColor;
extern float g_ViewerBrushRadius;
extern uint g_ViewerBrushHardness;
extern uint g_ViewerBrushOpacity;
extern uint g_ViewerDisplaceIndex;
extern bool g_PreloadTiles;
extern bool g_IncludeMeshes;
extern std::string g_InputPath;
extern std::string g_BankPath;
extern std::string g_StartupTexturePath;
extern bool g_ForceBankRecursive;
extern bool g_InteractiveSave;
extern bool g_LoadNeighbors;
extern bool g_BoardSession;
extern bool g_EmbeddedContext;
extern ZPWS::SWorldEntry g_StartupWorld;
extern ZPWS::SZoneEntry g_StartupZone;
extern std::vector<ZPWS::SZoneEntry> g_StartupEditableZones;

extern std::vector<SContextFile> g_ContextFiles;
extern std::vector<SSessionHintCell> g_SessionHintCells;
extern std::vector<PMAXLOAD::SLoadedMax *> g_NeighborScenes;
extern std::vector<PMAXLOAD::SLoadedMax *> g_ExtraEditableScenes;
extern std::vector<SEditableFileInfo> g_EditableFiles;
extern PMAXLOAD::SLoadedMax *g_PrimaryLm;
extern bool g_AllZones;
extern float g_SessionCellSize;
extern float g_SessionSnap;
extern bool g_SessionLockBorders;
extern NL3D::CTileBank *g_SessionBank;

extern std::vector<SInstancePlace> g_Places;
extern int g_FootprintCellsW;
extern int g_FootprintCellsH;
extern std::vector<bool> g_FootprintMask;
extern float g_FootprintOriginX;
extern float g_FootprintOriginY;
extern bool g_FootprintFromTemplate;

extern uint g_InstanceCols;
extern uint g_InstanceRows;
extern uint g_InstanceCount;
extern std::vector<std::string> g_MaskFiles;
extern int g_MaskCycle;

extern bool g_HavePropSelection;
extern uint g_SelectedZoneId;

/**
 * Patch-edit vertex selection, keyed by (zone id, CPatchInfo::BaseVertices index).
 *
 * That pair is the identity the cage already draws by: BaseVertices is NeL's own per-zone
 * vertex table, so a corner shared by four patches is ONE entry here, and selecting it once
 * selects the thing the artist actually pointed at. Mapping it onto a .max write target is a
 * separate problem and deliberately not represented here.
 */
typedef std::pair<uint, uint16> TPatchVertId;
extern std::set<TPatchVertId> g_PatchVertSel;

/**
 * Selected tangent handles, keyed (zone, PatchMesh Vecs index).
 *
 * The same shape as the vertex selection and for the same reason: SPmPatch::Vec[8] indexes
 * Vecs exactly as V[4] indexes Verts, so a handle shared by two patches along a common edge is
 * ONE handle, one marker, one selection entry. Handles are shown for selected corners only -
 * The rule, and the reason a dense cage does not turn into a field of dots.
 */
extern std::set<TPatchVertId> g_PatchTanSel;

/**
 * Vertex-level hit-test filters (the legacy Filter Vertices / Vectors pair) and Lock
 * Handles. The filters gate what a vertex-level click can pick - corner points, tangent
 * handles - never what is drawn; both cannot be off at once (the setters refuse, and the
 * panel freezes the other box). Lock Handles: a handle move takes the OTHER handles of
 * the same corner along, each through its own per-element delta - the effective tangent
 * selection expands over the owner group (zpTanSelectedEffective).
 */
extern bool g_PatchFilterVerts;
extern bool g_PatchFilterVecs;
extern bool g_PatchLockHandles;
void zpSetPatchFilterVerts(bool on);
void zpSetPatchFilterVecs(bool on);
void zpSetPatchLockHandles(bool on);
void zpPatchFilterVertsClicked();
void zpPatchFilterVecsClicked();
void zpPatchLockHandlesClicked();

/**
 * Orientation arrows (the legacy painter's ToggleArrows, C in the painter): every bank
 * tile gains an ADDITIVE arrow layer, so each painted tile shows its frame orientation -
 * rotation included - and patch mode draws a per-patch frame arrow on the overlay. The
 * display that makes Turn CW/CCW visible at all: a turn moves no geometry, only the
 * frame the tiles ride.
 */
extern bool g_ShowArrows;
void zpSetShowArrows(bool on);
void zpToggleShowArrows();
/** Selected, or a Lock Handles companion of a selected handle on the same corner. */
bool zpTanSelectedEffective(const SPaintZone &pz, uint16 vecIdx);

/**
 * The corner a handle belongs to, 0xffff if it is not a tangent of this zone (an interior
 * handle, or an index the patch table does not use as a tangent).
 *
 * Derived from the patch table rather than read from SPmVec::Vert, which is absent on Max 3
 * files: tangent 2e is attached to corner e and 2e+1 to corner (e+1)&3, the same convention
 * the cage is drawn with.
 */
uint16 zpTangentOwner(const SPaintZone &pz, uint16 vecIdx);

/** Selection ops for handles (recorded). Op: 0 replace, 1 add, 2 remove. */
void zpPatchTangentSelect(uint zoneId, uint vecIdx, int op);
uint zpPatchTangentSelCount();
bool zpPatchTangentSelAt(uint index, uint &zoneOut, uint &vecOut);
/** Displayed world position of a handle, for the gates. */
bool zpPatchTangentWorld(uint zoneId, uint vecIdx, float outPos[3]);

/** Index-based readback, so callers that cannot include this header still reach the set. */
uint zpPatchVertSelCount();
uint zpPatchEdgeSelCount();
uint zpPatchFaceSelCount();
bool zpPatchVertSelAt(uint index, uint &zoneOut, uint &vertOut);

/**
 * Displayed world position of one cage vertex - where the marker is actually drawn.
 *
 * Exists for the gates: it is the only quantity that shows whether an edit came back through
 * the node's placement correctly. A move can write the right bytes to the .max and still put
 * the display a board cell away, and nothing else observable distinguishes the two.
 */
bool zpPatchVertWorld(uint zoneId, uint vertIdx, float outPos[3]);

/**
 * Edge identity: the corner PAIR, ordered, not (patch, edge).
 *
 * An edge between two patches is drawn twice and reached from either, but it is one edge - the
 * same reason `BaseVertices` makes a corner touched by four patches one vertex. Keying on the
 * pair dedupes it for free and makes "the two ends of the selected edge" the obvious thing.
 */
struct SPatchEdgeId
{
	uint Zone;
	uint16 A, B; // A < B
	SPatchEdgeId() : Zone(0), A(0), B(0) { }
	SPatchEdgeId(uint z, uint16 a, uint16 b)
	    : Zone(z), A(a < b ? a : b), B(a < b ? b : a) { }
	bool operator<(const SPatchEdgeId &o) const
	{
		if (Zone != o.Zone) return Zone < o.Zone;
		if (A != o.A) return A < o.A;
		return B < o.B;
	}
};
extern std::set<SPatchEdgeId> g_PatchEdgeSel;

/** Patch (face) identity: (zone, patch index). Patches are not shared, so this is enough. */
typedef std::pair<uint, uint> TPatchFaceId;
extern std::set<TPatchFaceId> g_PatchFaceSel;

/**
 * Session-only HIDE set, patch-keyed (the legacy Hide / Unhide All, plan mA3). A hidden
 * patch drops from the cage, the pick paths and the overlay arrows; its elements refuse
 * selection. The LANDSCAPE keeps rendering it - hiding is a cage/editing concept, not a
 * rendering hole (deliberate divergence from the host, right for terrain). Session-only
 * by the corpus probe's verdict: PATCH_HIDDEN (bit 1) appears on 4 patches corpus-wide,
 * so the on-disk bit is preserved verbatim and never interpreted - no byte churn.
 * Cleared by every working-set rebuild (topology ops, open/close: indices shift).
 */
extern std::set<TPatchFaceId> g_PatchHidden;
bool zpPatchIsHidden(uint zoneId, uint patchIdx);
/** A vertex is hidden iff it is used by at least one patch and ALL of them are hidden
 *  (a rim vertex shared with a visible patch stays workable). */
bool zpVertIsHidden(const SPaintZone &pz, uint16 vertIdx);
/** Hide the patches the current level's selection touches (vertex/edge/patch levels);
 *  the selection clears - hidden elements cannot stay selected. NOT undoable (display
 *  state, like the weld view and the arrows). */
uint zpHideSelection();
uint zpUnhideAll();
void zpPatchHideClicked();
void zpPatchUnhideAllClicked();

/** Selection ops (recorded). Op: 0 replace, 1 add, 2 remove. */
void zpPatchVertSelect(uint zoneId, uint vertIdx, int op);
void zpPatchEdgeSelect(uint zoneId, uint vertA, uint vertB, int op);
void zpPatchFaceSelect(uint zoneId, uint patchIdx, int op);
void zpPatchVertClear();

/**
 * Edge and patch levels move VERTICES, so their selections are projected onto g_PatchVertSel
 * and everything downstream - gizmo, preview, weld propagation, the write path - is untouched.
 * The level's own set stays the authority, so removing one edge of a pair that shared a corner
 * correctly leaves that corner selected.
 */
void zpRebuildVertSelFromSubObject();

/** Picking at the edge and patch levels, mirroring zpPickPatchVertex. */
bool zpPickPatchEdge(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                     uint &zoneOut, uint16 &vertAOut, uint16 &vertBOut);
bool zpPickPatchFace(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                     uint &zoneOut, uint &patchOut);
bool zpPickPatchTangent(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                        uint &zoneOut, uint16 &vecOut);

/** Scripted click, using the session's camera and driver (headless picking gate). */
bool zpPatchClickAt(float x, float y, uint buttons);
/** Nearest editable-zone vertex within the screen pick radius. False if nothing is close. */
bool zpPickPatchVertex(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                       uint &zoneOut, uint16 &vertOut);
/** Centroid of the current vertex selection in world space. False when nothing is selected. */
bool zpPatchSelCentroid(NLMISC::CVector &out);

/** What a transform gizmo is currently doing. W / E / R. */
enum TXformKind
{
	ZPXF_Move = 0,
	ZPXF_Rotate,
	ZPXF_Scale
};

/**
 * One transform, in DISPLAYED world space, anchored on the current pivot.
 *
 * A move is expressed as a delta rather than living here, because a translation is the same
 * for every element and needs no anchor. Rotate and scale need both, so they carry theirs.
 */
struct SPatchXform
{
	TXformKind Kind;
	NLMISC::CVector Pivot;
	NLMISC::CVector Axis; ///< rotate: the axis, normalised
	float Angle; ///< rotate: radians
	NLMISC::CVector Scale; ///< scale: per-axis factors
	SPatchXform() : Kind(ZPXF_Move), Pivot(NLMISC::CVector::Null), Axis(0.f, 0.f, 1.f),
	                Angle(0.f), Scale(1.f, 1.f, 1.f) { }
};

/** Map one displayed-world point through a transform. */
NLMISC::CVector zpTransformPoint(const SPatchXform &xf, const NLMISC::CVector &p);
/** Apply a transform to the selection; worldDelta is used only for ZPXF_Move. */
uint zpApplyPatchXform(const SPatchXform &xf, const NLMISC::CVector &worldDelta, std::string &msg);
uint zpApplyPatchRotate(int axis, float degrees, std::string &msg);
uint zpApplyPatchRotateAxis(float ax, float ay, float az, float degrees, std::string &msg);
uint zpApplyPatchScale(float sx, float sy, float sz, std::string &msg);

/** Current gizmo transform kind, and the switch (recorded). */
extern int g_XformKind;
void zpSetXformKind(int kind);
const char *zpXformKindName(int kind);

/**
 * Where a transform is anchored - the pivot-point control.
 *
 * Irrelevant to a move (a translation is a translation wherever it is anchored) and decisive
 * for rotate and scale, which is why the gizmo is drawn AT the pivot rather than at the
 * selection: the artist has to be able to see what the next rotate will turn around before
 * turning it.
 */
enum TPivotMode
{
	ZPPIV_Selection = 0, ///< centroid of the selected sub-objects
	ZPPIV_World, ///< the world origin
	ZPPIV_AllObjects, ///< centre of every editable node, re-fitted BETWEEN interactions
	ZPPIV_SelObjects, ///< centre of the nodes that own the selection
	ZPPIV_User, ///< a point the artist placed
	ZPPIV_Count
};
extern int g_PivotMode;
extern NLMISC::CVector g_UserPivot;
extern bool g_HaveUserPivot;

/** The pivot for the current mode. False when it cannot be formed (nothing selected, etc). */
bool zpTransformPivot(NLMISC::CVector &out);
/** Switch pivot mode (recorded); out of range is clamped. */
void zpSetPivotMode(int mode);
/** Put the user pivot at the current selection's centre (recorded). False if nothing selected. */
bool zpSetUserPivotToSelection();
void zpUserPivotToSelection();
/** Place the user pivot at an absolute point (recorded) - the recorder preamble's restore op. */
void zpSetUserPivotXYZ(float x, float y, float z);
/** The placed user pivot, if any (flat form for the script TU). */
bool zpUserPivotXYZ(float outPos[3]);
/** Open the scene context menu at the pointer (patch mode right click). */
void zpOpenSceneMenu();
/** Short label for the toolbar button and the HUD. */
const char *zpPivotModeName(int mode);
/** Re-fit the "all objects" centre. Called between interactions, never during one. */
void zpPivotNoteInteractionEnd();
/**
 * Move gizmo on the vertex selection. Screen-projected like every other patch overlay, but
 * sized from a WORLD length that is only re-fitted between interactions - see the sample's
 * fit-at-rest model. Not drawn while the view is moving.
 */
void zpDrawPatchGizmo(NL3D::IDriver *driver, NL3D::CCamera *camera,
                      float mouseX, float mouseY, bool navigating, uint32 viewSerial);

/**
 * Gizmo handles. The screen handle owns no shape: it is the empty middle where the three
 * plane corners meet, and hovering it lights all three - which reads as "the whole plane set
 * is live", exactly what a view-parallel move is.
 */
enum TPatchGizmoHandle
{
	ZPGIZ_NONE = -1,
	ZPGIZ_AXIS_X = 0, ZPGIZ_AXIS_Y, ZPGIZ_AXIS_Z,
	ZPGIZ_SCREEN,
	ZPGIZ_PLANE_XY, ZPGIZ_PLANE_YZ, ZPGIZ_PLANE_ZX
};

/** Hovered gizmo handle, ZPGIZ_NONE for none. Set by the last zpDrawPatchGizmo. */
int zpPatchGizmoHover();
/** Force the gizmo to re-take its screen fit; call whenever the selection centroid moves. */
void zpPatchGizmoInvalidate();

/**
 * Gizmo drag. While it runs the transform is a PREVIEW - offsets over an untouched cage -
 * and endDrag is the commit: it applies the transform through zpApplyPatchXform, which
 * writes the .max, records the op and pushes the live surface. cancelDrag abandons it -
 * right-click-while-dragging (and ESC) - nothing written, the preview pushed back out
 * of the live surface.
 */
bool zpPatchGizmoBeginDrag(int handle, NL3D::CCamera *camera, const NL3D::CViewport &vp,
                           float mouseX, float mouseY);
void zpPatchGizmoUpdateDrag(NL3D::CCamera *camera, const NL3D::CViewport &vp,
                            float mouseX, float mouseY);
void zpPatchGizmoEndDrag();
void zpPatchGizmoCancelDrag();
bool zpPatchGizmoDragging();

/**
 * Commit a world-space move of the current vertex selection into the .max. Returns the number
 * of vertices written; `msg` explains a zero (or a partial). Skips bound vertices by policy.
 */
uint zpApplyPatchMove(const NLMISC::CVector &worldDelta, std::string &msg);
extern bool g_PatchLiveUpdate;
/**
 * Are the landscape zones currently built WELDED to each other?
 *
 * Tracks the paint mode: welded while painting, apart in patch edit. Read by
 * buildDisplayZone, so every build path picks it up without being told.
 */
extern bool g_WeldedLandscape;

/**
 * Re-build every landscape zone for the current weld state. No-op when nothing changed.
 * Returns false only if a zone could not be re-added.
 */
bool zpSyncLandscapeWeld();
/**
 * Push the selection's CURRENT display positions into the live landscape and refresh the
 * tessellation of every patch they touch, plus its bind neighbours - the same shape as the
 * paint path's applyChanges. `preview` adds the in-flight drag delta; false pushes what is
 * already committed. Returns false when a control point fell outside the zone's packed range,
 * which only a rebuild can fix.
 */
bool zpPatchPushLive(bool preview);

/** Core geom-changed sink: keeps Ep.Pm and the display patchinfo in step with the .max. */
void zpGeomVertChanged(uint zoneId, uint16 elemIdx, int elem, const float *objDelta);

// Bind / edge-flag ops (patch_edit_ops.cpp). Panel buttons, hotkeys and painterscript all
// come through these - the single op surface.
/** Core rp-state sink: bind records or edge flags changed (forward, undo or redo). */
void zpRpStateChanged(uint zoneId);
/** Re-derive one zone's BindEdges + smooth-flag bits from its eval mirror, in place. */
void zpRederiveBindEdges(SPaintZone &pz);
/** Release the bound vertices of the selection, whole bind groups at a time. */
uint zpUnbindPatchSelection();
/** Bind each selected free vertex onto its nearest valid open edge. */
uint zpBindPatchSelection();
/** Bind one vertex onto edge `edgeSlot` of `patchIdx` (validated; snaps in-stroke). */
bool zpBindPatchVertexToEdge(uint zoneId, uint vertIdx, uint patchIdx, uint edgeSlot,
                             std::string &msg);
/** Write the no-smooth bit on every patch side of every selected edge. */
uint zpSetEdgeNoSmooth(bool noSmooth);
/** Panel checkbox state over the edge selection: 0 none, 1 all, 2 mixed/none selected. */
int zpEdgeNoSmoothTriState();
/** One edge's flag by corner pair: 0 clear, 1 flagged, -1 unknown edge. */
int zpEdgeNoSmoothQuery(uint zoneId, uint vertA, uint vertB);
/** Panel click handlers (legacy tri-state cycle for the checkbox). */
void zpPatchNoSmoothClicked();
void zpPatchBindClicked();
void zpPatchUnbindClicked();

// Topological ops (patch_topo_ops.cpp). Storage-rewriting; the working set rebuilds after.
/** Delete the selected patches; paint travels with the survivors. Undoable (Kind 6). */
uint zpDeletePatchSelection();
void zpPatchDeleteClicked();
/** Turn the selected quad patches a quarter turn (tile frame rotation). Undoable. */
uint zpTurnPatchSelection(bool ccw);
void zpPatchTurnCcwClicked();
void zpPatchTurnCwClicked();
/** Subdivide selected quads 4-way; paint quadrants inherit; T-junction binds. Undoable. */
uint zpSubdividePatchSelection();
/** Edge-level subdivide (mA4): 1->2 across each selected edge; Propagate walks the
 *  strip. The Subdiv button serves both levels. Undoable. */
uint zpSubdivideEdgeSelection();
void zpSetSubdividePropagate(bool on);
bool zpSubdividePropagate();
void zpSubdivPropToggleClicked();
void zpPatchSubdivideClicked();
/** Weld selected vertices (target-weld; coincident open edges fuse). Undoable. */
uint zpWeldPatchSelection(float threshold);
void zpPatchWeldClicked();
float zpLastWeldThreshold();
void zpPatchWeldThresholdClicked(float distance);
/** Directed weld: srcVert merges into dstVert (the target keeps position and identity) -
 *  the drag-onto-a-vertex gesture's op. Undoable. */
uint zpWeldVertexInto(uint zoneId, uint srcVert, uint dstVert);
/** Surface Properties (patch level): smoothing groups and per-patch tessellation. */
uint zpExtrudePatchSelection(float dz);
/** Full extrude (mA5/mA6): height along Z or the selection's area-weighted eval normal
 *  (Group semantics), plus the bevel outline (in/out along the boundary ring's outward
 *  XY direction). Undoable. */
uint zpExtrudePatchSelectionEx(float h, float outline, bool localNormal);
float zpLastExtrudeHeight();
float zpLastExtrudeOutline();
bool zpLastExtrudeLocal();
void zpPatchExtrudeClicked(float dz);
void zpPatchExtrudeExClicked(float h, float outline, bool local);
/** Begin a SHIFT-drag extrude at patch level: a Z-constrained gizmo drag whose release
 *  commits an extrude of the drag's height instead of a move. */
bool zpPatchGizmoBeginExtrudeDrag(NL3D::CCamera *camera, const NL3D::CViewport &vp,
                                  float mouseX, float mouseY);
uint zpSetSmoothGroup(uint bit, bool on);
uint zpClearSmoothGroups();
/** Vertex continuity type (PVERT_COPLANAR, vertex Flags bit 0 - corpus-pinned): value op
 *  through the runner; the constraint applies at handle-move time. Undoable (Kind 6). */
uint zpSetVertexCoplanar(bool on);
/** Tri-state over the vertex selection: 0 = all corner, 1 = all coplanar, 2 = mixed/empty. */
int zpVertCoplanarTriState();
void zpPatchCoplanarClicked(int on);
bool zpPatchVertFlagsQuery(uint zoneId, uint vertIdx, sint32 &out);
/** Auto/Manual interior (PATCH_AUTO, patch Flags bit 0; mA2): auto -> manual BAKES the
 *  derived interiors (they become authored, editable points); manual -> auto abandons
 *  them. Undoable (Kind 6). */
uint zpSetPatchAuto(bool on);
int zpPatchAutoTriState();
void zpPatchAutoClicked(int on);
bool zpPatchFlagsQuery(uint zoneId, uint patchIdx, sint32 &out);
bool zpPatchInteriorIndexQuery(uint zoneId, uint patchIdx, uint slot, uint &out);
/** Is this vec index an INTERIOR of an AUTO patch (derived - refuses selection)? */
bool zpVecIsAutoInterior(const SPaintZone &pz, uint16 vecIdx);
uint zpSetPatchTess(int u, int v);
uint zpBalanceTessSelection();
bool zpPatchSmGroupsQuery(uint zoneId, uint patchIdx, uint32 &maskOut);
bool zpPatchTessQuery(uint zoneId, uint patchIdx, int &uOut, int &vOut);
void zpPatchSmGroupClicked(int bit);
void zpPatchSmGroupClearClicked();
void zpPatchTessDeltaClicked(int axis, int delta);
void zpPatchBalanceClicked();
/** Target-weld command mode (vertex level): armed by the panel Target toggle; dragging a
 *  vertex onto another welds it INTO the target. Nothing moves during the drag. */
extern bool g_WeldTargetArmed;
bool zpWeldDragActive();
bool zpWeldDragBegin(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my);
void zpWeldDragUpdate(float mx, float my);
void zpWeldDragFinish(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my);
void zpWeldDragCancel();
void zpDrawWeldDrag(NL3D::IDriver *driver, NL3D::CCamera *camera);
void zpWeldTargetToggleClicked();
bool zpWeldDragAt(float x0, float y0, float x1, float y1);
bool zpExtrudeDragAt(float x0, float y0, float x1, float y1);
bool zpPatchVertScreen(uint zoneId, uint vertIdx, float &sxOut, float &syOut);
bool zpPatchTangentScreen(uint zoneId, uint vecIdx, float &sxOut, float &syOut);
/** Grow a quad from each selected open edge (legacy Add Quad). Undoable. */
uint zpAddQuadPatchSelection();
void zpPatchAddQuadClicked();
/** Detach the selection as its own ISLAND inside the same zone (detach-to-element:
 *  boundary splits, nothing moves, still one exported node). Undoable. */
uint zpDetachPatchSelection();
void zpPatchDetachClicked();
/** SHELVED off the panel: detach the selection into a new brick file (script-only until
 *  fresh-zone-file creation is designed; empty name = auto-bumped "<source>-det"). */
uint zpDetachToFile(const std::string &nameIn);
/** Expand the face selection to whole elements (shared-vertex connected components). */
uint zpExpandSelectionToElement();
void zpPatchElementClicked();
/** Merge zone srcZone into targetZone (source file saved and closed; the attach lands as
 *  the fresh undo stack's first stroke - undo rolls the target back). */
uint zpAttachZone(uint targetZone, uint srcZone, std::string &msg);
/** Move the selected patches into dstZone (cross-file transfer; both files stay open;
 *  one two-snapshot Kind 6 stroke - fully undoable). */
uint zpMovePatchSelectionToZone(uint dstZone, std::string &msg);
/** The editable neighbor zone in compass direction 0..7 (N NE E SE S SW W NW) of the
 *  selection's file, by board-cell adjacency. */
bool zpMoveDirTarget(int dir, uint &dstZoneOut);
void zpMoveToZoneDirClicked(int dir);
/** Displayed patch count of a zone (script/gate read access). */
bool zpZonePatchCount(uint zoneId, uint &countOut);
/** Eval-mirror vertex count of a zone (script/gate read access). */
bool zpZoneVertCount(uint zoneId, uint &countOut);
/** Layer-0 tile of one grid tile (script/gate read access; display transform applied). */
bool zpTileQuery(uint zoneId, uint patchIdx, uint u, uint v, int &tileOut, int &rotOut,
                 int &numOut);
/** Kind 6 undo/redo sink: re-encode a topology snapshot's matching side into storage. */
namespace ZPPAINT { struct STopoSnapshot; }
void zpTopoRestore(const ZPPAINT::STopoSnapshot &snap, bool useOld);
/** After any opUndo/opRedo: run the working-set rebuild a replayed topology record needs. */
void zpHandleTopoRestorePending();

/** Left-click in patch/vertex mode; `buttons` carries the modifier bits (Ctrl add, Alt remove). */
void zpPatchVertexClick(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my, uint buttons);
extern std::string g_PropStatusMsg;

extern const char *kPainterKeysName[ZPK_KeyCounter];
extern uint g_PainterKeys[ZPK_KeyCounter];
extern const uint8 kPainterKeyModes[ZPK_KeyCounter];

extern NLMISC::CVector g_LightDirection;
extern NLMISC::CRGBA g_LightDiffuse;
extern NLMISC::CRGBA g_LightAmbiant;
extern float g_LightMultiply;
extern float g_ZoomSpeed;

extern NLMISC::CEventListenerAsync *g_ViewerAsync;
extern std::string g_LuaScriptPath;
extern std::string g_StartupLuaPath;
extern bool g_ScriptUiLock;

extern std::vector<SPlaceContextSpec> g_PlaceContextSpecs;
extern std::vector<SOpenEditableSpec> g_OpenEditableSpecs;

extern float g_SessionAnchorX, g_SessionAnchorY;
extern std::string g_LegacyPlaceSourceName;
extern bool g_SessionAnchorSet;

extern bool g_HintStampEnabled;
extern std::vector<std::string> g_AvailableSeasons;
extern SPaintCtx g_PaintCtx;
extern bool g_CliWantThumbnail;
extern bool g_NoThumbnailWrites;
extern NLMISC::CBitmap g_CapturedThumb;
extern bool g_HaveCapturedThumb;
extern std::string g_LastSaveStatus;
extern int g_BoardOpDepth;

extern bool (*g_ScriptScreenshotFn)(const std::string &path, std::string &err);
extern void (*g_ScriptPumpFn)();
extern bool g_ScriptCancel;
extern bool g_SessionOpsAvailable;
extern ZPSCRIPT::SScriptHost g_ScriptHost;
extern SScriptPumpCtx g_PumpCtx;

// ---------------------------------------------------------------------------------------------
// Cross-TU function forward decls. Definitions live in the .cpp named on each block.

// main.cpp
bool zpKeyPushed(TPainterKey action);
bool zpKeyDown(TPainterKey action);
/** Digit k (0-9) of the ZPK_TileSetDigits run was pushed, mode scope honored. */
bool zpKeyDigitPushed(uint digit);

// scene_paint.cpp
bool writeZoneV4(NL3D::CZone &zone, const std::string &path);
void computeZoneEligibility(const std::vector<SZoneNode> &nodes,
                            const std::string &fileBasename,
                            std::vector<bool> &eligible);
bool boardSkipEmbedded();
bool buildPaintZones(PIPELINE::MAX::CScene &scene, std::vector<SPaintZone> &zones,
                     uint zoneIdOffset, bool forceFrozen,
                     const std::string &fileBasename = std::string());
uint nextZoneIdBase(const std::vector<SPaintZone> &zones);
void translateZonesXY(std::vector<SPaintZone> &zones, size_t begin, size_t end,
                      float dx, float dy);
void scaleZonesXYZ(std::vector<SPaintZone> &zones, size_t begin, size_t end, float s);
bool authoredAABB(const std::vector<SPaintZone> &zones, size_t begin, size_t end,
                  NLMISC::CAABBox &out);
void freeLoadedMax(PMAXLOAD::SLoadedMax *lm);
SPaintZone cloneInstanceZone(const SPaintZone &src, uint zoneId, float dx, float dy,
                             float pivotX, float pivotY, const SInstancePlace &place);
uint appendInstanceZones(std::vector<SPaintZone> &zones, size_t primaryCount,
                         const std::vector<SInstancePlace> &places, float cellSize);
void applyInstanceDisplayTiles(std::vector<SPaintZone> &zones, NL3D::CTileBank *bank);
void addBorderVertexPair(std::vector<SPaintZone> &zones, uint zi, uint16 va, uint zj, uint16 vb,
                         std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > &seen);
uint weldPaintZones(std::vector<SPaintZone> &zones);
void buildDisplayZone(const SPaintZone &pz, NL3D::CZone &zone);
std::string sanitizeName(const std::string &s);
int dumpZones(std::vector<SPaintZone> &zones, uint welds, const std::string &outDir);
void buildPaintInputs(std::vector<SPaintZone> &zones, std::vector<ZPPAINT::SPaintZoneInput> &inputs);

// hints_and_footprint.cpp
bool parseNeighborHintsString(const std::string &raw, std::vector<SNeighborHint> &out);
bool neighborHintLess(const SNeighborHint &a, const SNeighborHint &b);
std::string encodeNeighborHintsString(std::vector<SNeighborHint> hints);
bool readNeighborHintsFromNode(PIPELINE::MAX::BUILTIN::CNodeImpl *node,
                               std::vector<SNeighborHint> &out,
                               std::string *rawOut = NULL);
bool readNeighborHintsFromScene(PIPELINE::MAX::CScene &scene, const std::string &fileBasename,
                                std::vector<SNeighborHint> &out, std::string *rawOut = NULL);
bool writeNeighborHintsToScene(PIPELINE::MAX::CScene &scene, const std::string &fileBasename,
                               const std::vector<SNeighborHint> &hints);
void collectHintsFromLoadedContext(std::vector<SNeighborHint> &out);
void writeNeighborHintsIfBoardSession();
bool zoneNodeAuthoredFootprintRawMin(PIPELINE::MAX::BUILTIN::CNodeImpl *node,
                                     SNodeTMCache &tmCache, float &minX, float &minY);
bool zoneNodeAuthoredFootprintOrigin(PIPELINE::MAX::BUILTIN::CNodeImpl *node,
                                     SNodeTMCache &tmCache, float cellSize,
                                     float &originX, float &originY);
bool extractEmbeddedNeighborHints(PIPELINE::MAX::CScene &scene, const std::string &fileBasename,
                                  float cellSize, std::vector<SNeighborHint> &out);
bool resolveHintToZone(const ZPWS::SWorldEntry &world, const std::string &hintBase,
                       ZPWS::SZoneEntry &out);
void collectNeighborHints(PIPELINE::MAX::CScene &scene, const std::string &fileBasename,
                          const ZPWS::SWorldEntry &world, float cellSize,
                          std::vector<SNeighborHint> &hintsOut,
                          std::vector<ZPWS::SZoneEntry> &resolvedOut,
                          std::string &sourceOut);
void normalizePlaceContextSpecBasenames();
struct CCmdArgsFwd;
namespace NLMISC { class CCmdArgs; }
bool parsePlaceSpec(const std::string &sIn, SInstancePlace &out, std::string &err);
bool parseInstanceLayout(const std::string &s, uint &cols, uint &rows,
                         std::vector<SInstancePlace> &places, std::string &err,
                         int fw = 0, int fh = 0);
bool parseCliPlaces(NLMISC::CCmdArgs &args);
void computeFootprintRect(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                          size_t primaryEnd, float cellSize,
                          float &originX, float &originY,
                          float &stepX, float &stepY,
                          int &cellsW, int &cellsH);
void computeFootprintStep(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                          size_t primaryEnd, float cellSize, float &stepX, float &stepY);
bool buildZoneMaskFromEval(const SEvalPatch &ep, const MAXMATH::Matrix3M &objectTM, bool symmetry,
                           const NLLIGO::CLigoConfig &config,
                           std::vector<bool> &mask, uint &width, uint &height,
                           std::string &err);
void buildSquareMaskFromPatches(const std::vector<NL3D::CPatchInfo> &patchinfo,
                                float cellSize, std::vector<bool> &mask,
                                uint &width, uint &height);
bool maskHasHole(const std::vector<bool> &mask);
std::string maskToTFString(const std::vector<bool> &mask, int w, int h);
bool deriveZoneFootprintMask(const SPaintZone &pz, float cellSize, float snap,
                             std::vector<bool> &mask, int &cellsW, int &cellsH,
                             float &originX, float &originY, bool &fromTemplate,
                             std::string &err);
void derivePrimaryFootprint(const std::vector<SPaintZone> &zones, size_t begin, size_t end,
                            float cellSize, float snap);
bool compareFootprintToLigozone(const std::string &ligozonePath);
void maskRotFlip(std::vector<bool> &mask, int &w, int &h, uint rot, bool flip);
void footprintBlockSize(int cellsW, int cellsH, uint rot, bool mirror,
                        int &outW, int &outH);
bool maskCellOccupied(const std::vector<bool> &mask, int w, int h,
                      int ox, int oy, uint rot, bool mirror, int cx, int cy);
bool masksCollide(const std::vector<bool> &a, int aw, int ah, int aox, int aoy,
                  uint aRot, bool aMir,
                  const std::vector<bool> &b, int bw, int bh, int box, int boy,
                  uint bRot, bool bMir);
void footprintBlockAfterTransform(int cellsW, int cellsH, uint rot, bool mirror,
                                  int &originDX, int &originDY, int &outW, int &outH);
void unitCheckFootprintOccupancy();
void transformInstanceXY(float &x, float &y, float pivotX, float pivotY,
                         float dx, float dy, uint rot, bool mirror);
MAXMATH::Matrix3M instanceDisplayTM(float pivotX, float pivotY,
                                    float dx, float dy, uint rot, bool mirror);
void computePrimaryPivot(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                         size_t primaryEnd, float cellSize, float &px, float &py);
void computePlaceTranslationFrom(float originX, float originY, float stepX, float stepY,
                                 float pivotX, float pivotY,
                                 float boardOriginX, float boardOriginY, float cellSize,
                                 int placeX, int placeY, uint rot, bool mirror,
                                 float &outDx, float &outDy);
void computePlaceTranslation(float originX, float originY, float stepX, float stepY,
                             float pivotX, float pivotY, float cellSize,
                             int placeX, int placeY, uint rot, bool mirror,
                             float &outDx, float &outDy);

// save_ops.cpp
PIPELINE::MAX::CScene *editableScene(const SEditableFileInfo &efi);
std::string absFilePath(const std::string &path);
SEditableFileInfo *findEditableByPath(const std::string &path);
uint countDirtyEditableFiles();
bool saveOneOverwrite(const std::string &orig, PIPELINE::MAX::CScene &scene, bool doThumb,
                      const std::vector<uint> *zoneIds = NULL);
bool saveCopyAtomic(const std::string &src, const std::string &target,
                    PIPELINE::MAX::CScene &scene, const std::vector<uint8> *si);
bool zpSaveTo(const std::string &target);
bool zpSaveOverwrite();
bool zpSaveOverwriteImpl();
bool zpSaveFileOverwrite(const std::string &basename, bool wantThumb);
bool zpSaveFileCopy(const std::string &basename, const std::string &name, bool wantThumb);
std::string zpFileDir(const std::string &basename);
void zpSaveDirect();
bool captureTopDownThumbnail(NLMISC::CBitmap &out, const std::vector<uint> *zoneIds = NULL);
bool zpLegacyWantThumbnail();
bool prepareThumbnailOverride(const std::string &srcMax, std::vector<uint8> &siOut,
                              bool &haveOverride, bool want,
                              const std::vector<uint> *zoneIds = NULL);
int saveWholeFile(const std::string &input, const std::string &output,
                  PIPELINE::MAX::CScene &scene, bool verifyIdentical,
                  const std::vector<uint8> *summaryOverride = NULL);
std::vector<uint8> writeContainerToTemp(PIPELINE::MAX::CStorageContainer &ctr, const std::string &tempPath);

// script_and_ui.cpp
/** Draw one zone's patch control cage (and, at vertex level, its corner markers). */
void zpDrawPatchLattice(NL3D::IDriver *driver, NL3D::CCamera *camera,
                        const SPaintZone &pz, int subObj);

/** Cage for every editable zone; used by both the interactive and screenshot overlay passes. */
void zpDrawPatchLatticeAll(NL3D::IDriver *driver, NL3D::CCamera *camera, int subObj);
/** The depth-tested wireframe pass (cage + frame arrows), called BEFORE editorUI->draw(). */
void zpDrawPatchWire3DAll(NL3D::IDriver *driver, NL3D::CCamera *camera, int subObj);

/** HUD label for a paint mode / sub-object level; "?" when out of range. */
const char *zpModeName(int mode);
const char *zpSubObjName(int level);

/** Set the patch-edit sub-object level (EP_* order); no-op outside ModePatch. */
void zpSelectSubObject(int level);

/** Sub-object level digit k (1..5 -> EP_OBJECT..EP_TILE) was pushed this frame. */
bool zpKeySubObjPushed(uint level);

/** Current CPaintMouseListener mode, or -1 when no viewer session is live. */
int zpCurrentPaintMode();
std::string luaQuote(const std::string &s);
void recordBoardOp(const std::string &line);
void zpSelectMode(int mode);
void zpFill(int rot);
void zpUndo();
void zpRedo();
void zpSelectTileSetDelta(int d);
void zpSelectTileSetAbs(int idx);
void zpSetTileSize256(bool on);
void zpToggleTileSize();
void zpColorRadiusAbs(float m);
void zpColorRadiusDelta(int d);
void zpBrushSizeDelta(int d);
void zpGroupDelta(int d);
void zpHardnessAbs(int v);
void zpHardnessDelta(int d);
void zpOpacityAbs(int v);
void zpOpacityDelta(int d);
void zpCycleBrushMask();
void zpToggleMaskMode();
void zpDisplaceIndexDelta(int d);
void zpDisplaceIndexAbs(int idx);
void zpSetBrushColor(int r, int g, int b);
void zpToggleLockBorders();

bool zpZoneIsPropSelectable(uint zoneId);
void zpClearPropSelection();
const SPaintZone *zpFindPaintZone(uint zoneId);
SPaintZone *zpFindPaintZoneMut(uint zoneId);
bool zpEraseScriptAppData(PIPELINE::MAX::BUILTIN::CNodeImpl *node, uint32 subId);
bool zpSetScriptAppDataStr(PIPELINE::MAX::BUILTIN::CNodeImpl *node, uint32 subId,
                           const std::string &value);
void zpReadZoneProps(PIPELINE::MAX::BUILTIN::CNodeImpl *node, SZoneProps &out);
bool zpWriteZoneProp(uint zoneId, const std::string &which, int value, std::string &err);
bool zpZoneIsFootprintSource(uint zoneId);
void zpOnPropChanged(uint zoneId, uint32 appDataId);
void zpPropRotateDelta(int d);
void zpPropToggleSymmetry();
void zpPropTogglePassable();
void zpPropToggleUseBBox();
std::string zpZoneFileBasename(uint zoneId);
int dumpZoneProps(const std::string &path);
void zpEvalPatchEdgePoint(const NL3D::CBezierPatch &bp, uint e, float t,
                          NLMISC::CVector &out);
bool zpIsZoneOuterBoundaryEdge(const SPaintZone &pz, size_t p, uint e);
void zpCollectZoneBoundaryPolylines(const SPaintZone &pz, uint segsPerEdge,
                                    std::vector<NLMISC::CVector> &outPts,
                                    std::vector<uint> &outSegCounts,
                                    uint *outLoopCount = NULL,
                                    uint *outEdgeCount = NULL);
void zpDrawZoneOutline(NL3D::IDriver *driver, NL3D::CCamera *camera,
                       const NL3D::CViewport &viewport,
                       const SPaintZone &pz, const NLMISC::CRGBA &col, bool thick);

void zpRebuildTilesetPalette();
void zpTogglePalette();
void zpToggleBoard();
std::string zpSeasonCacheKey();
void zpSeasonApplyFlush();
void zpSeasonNext();
void zpSeasonSelect(const std::string &code);
void zpSeasonMenuFill(void *unused);
void zpFillBridgeState(ZPUI::SPaintUIBridge &bridge);
void zpScriptRefreshBridge();

bool zpExecScriptOp(ZPPAINT::CPaintCore &core, const std::string &rawLine,
                    std::string &err, std::string *opName, bool *unknownCmd);
int runPaintScript(ZPPAINT::CPaintCore &core, const std::string &path);
bool zpScriptExecOp(const std::string &line, std::string &err);
void zpScriptZonesInfo(std::vector<ZPSCRIPT::SZoneInfo> &out);
bool zpScriptGetZoneProp(uint zoneId, const std::string &which, int &value, std::string &err);
bool zpScriptCancelRequested();
void zpScriptResetCancel();
bool zpScriptOpenZone(const std::string &basename, std::string &err);
bool zpScriptCloseZone(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
bool zpScriptEcoGate(const char *op, std::string &err);
bool zpScriptOpenZoneAt(const std::string &basename, int cx, int cy, std::string &err);
bool zpScriptPlaceInstance(int cx, int cy, const std::string &basename, std::string &err);
bool zpScriptRemoveInstance(int cx, int cy, std::string &err);
bool zpScriptRotateInstance(int cx, int cy, int delta, std::string &err);
bool zpScriptMirrorInstance(int cx, int cy, std::string &err);
bool zpScriptPlaceContext(int cx, int cy, const std::string &basename, std::string &err);
bool zpScriptRemoveContext(int cx, int cy, std::string &err);
bool zpScriptRotateContext(int cx, int cy, int delta, std::string &err);
bool zpScriptMirrorContext(int cx, int cy, std::string &err);
bool zpScriptMakeEditable(int cx, int cy, std::string &err);
bool zpScriptDragCell(int fx, int fy, int tx, int ty, bool copy, std::string &err);
bool zpScriptToggleZone(const std::string &basename, bool saveFirst, bool forceDiscard,
                       std::string &err);
bool zpScriptSaveZone(const std::string &basename, std::string &err);
void zpInstallScriptHost(ZPUI::SPaintUIBridge *bridgePtr);
void zpScriptPumpImpl();
bool zpScriptScreenshotImpl(const std::string &path, std::string &err);

// board_session.cpp
void clearContextFiles();
void loadNeighborContextFiles(std::vector<SPaintZone> &zones, float cellSize,
                              const std::vector<std::string> &skipBasenames);
bool loadOnePlaceContext(std::vector<SPaintZone> &zones, float cellSize,
                         const SPlaceContextSpec &pc, std::string &err);
void placeContextRange(std::vector<SPaintZone> &zones, size_t rb, size_t re,
                       float cOx, float cOy, int cw, int ch,
                       float boardOriginX, float boardOriginY, float cellSize,
                       int dx, int dy, uint rot, bool mirror);
void placeEcoEditableRange(std::vector<SPaintZone> &zones, SEditableFileInfo &efi,
                           size_t rb, size_t re, float cellSize, float snap);
/** skipWriteBack: the caller already flushed paint AND mutated carrier bytes since - the
 *  rebuild's own write-back would re-encode the stale pristine over the mutation.
 *  keepUndo: preserve the paint core's undo/redo stacks (TOPOLOGY rebuild paths only -
 *  same zone set; session open/close must keep clearing). */
bool rebuildWorkingSet(std::string &err, uint &outWelds, bool skipWriteBack = false,
                       bool keepUndo = false);
SEditableFileInfo *findEditableByBasename(const std::string &basename);
const ZPWS::SZoneEntry *findWorldZone(const std::string &basename);
bool sessionSaveOneFile(SEditableFileInfo &efi, std::string &err);

int scratchFw();
int scratchFh();
const std::vector<bool> &scratchHomeMask();
void scratchBoardAnchor(float &ax, float &ay);
void instanceSourceFootprint(const SInstancePlace &pl, std::vector<bool> &mask,
                             int &fw, int &fh);
bool scratchPlaceOccupies(const SInstancePlace &pl, int cx, int cy);
bool scratchFindPlace(int cx, int cy, size_t &idx);
bool contextBasenameHasSpec(const std::string &basename);
bool hintContextConflicts(const std::vector<bool> &cmask, int cfw, int cfh,
                          int ox, int oy, uint rot, bool mirror, std::string &err);
bool scratchMaskConflictsSrc(const std::vector<bool> &cmask, int cfw, int cfh,
                             int ox, int oy, uint rot, bool mirror,
                             int skipIdx, std::string &err);
bool placeDupInstanceNear(const std::string &srcName, int cx, int cy);
bool scratchRebuild(std::string &err);
void scratchRecoveryRebuild(const char *op);
bool scratchFindContext(int cx, int cy, size_t &idx);
bool scratchFindEditableAt(int cx, int cy, size_t &idx);
bool scratchGetEditableAt(int cx, int cy, int &ox, int &oy, std::string &basename,
                          bool &editable);
bool scratchGetCellState(const std::string &basename, ZPUI::ESessionCellState &out);
bool contextCandidateConflicts(size_t idx, int nx, int ny, uint nrot, bool nmirror,
                               std::string &err);
bool scratchPlaceContext(int cx, int cy, const std::string &basename, std::string &err);
bool scratchRemoveContext(int cx, int cy, std::string &err);
void contextSpecMask(const SPlaceContextSpec &pc, std::vector<bool> &m, int &cw, int &ch);
bool scratchRotateContext(int cx, int cy, int delta, std::string &err);
bool scratchMirrorContext(int cx, int cy, std::string &err);
bool scratchGetContextTransform(int cx, int cy, uint &rot, bool &mirror);
bool scratchPlace(int cx, int cy, std::string &err);
bool scratchPlaceInstanceOf(int cx, int cy, const std::string &basenameIn, std::string &err);
bool scratchEditableConflicts(size_t idx, std::string &err);
bool scratchDragDrop(int fx, int fy, int tx, int ty, bool copy, std::string &err);
bool scratchGetContext(int cx, int cy, std::string &basename);
bool scratchOpenEditable(int cx, int cy, const std::string &basenameIn, std::string &err);
bool scratchContextToEditable(int cx, int cy, std::string &err);
int scratchOpenFileCount();
bool scratchGetInstanceSource(int cx, int cy, std::string &basename);
bool scratchGetHintAt(int cx, int cy, std::string &basename);
void scratchHintNames(std::vector<std::string> &out);
bool scratchRotate(int cx, int cy, int delta, std::string &err);
bool scratchMirror(int cx, int cy, std::string &err);
bool scratchRemove(int cx, int cy, std::string &err);
bool scratchGetInstance(int cx, int cy, uint &rot, bool &mirror);
bool scratchGetInstanceOrigin(int cx, int cy, int &ox, int &oy, uint &rot, bool &mirror);
bool sessionGetCellState(const std::string &basename, ZPUI::ESessionCellState &out);
bool sessionIsDirty(const std::string &basename);
bool sessionIsOpen(const std::string &basename);
bool sessionIsEditable(const std::string &basename);
bool sessionOpenZone(const std::string &basename, std::string &err);
bool sessionCloseZone(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
bool sessionSaveZone(const std::string &basename, std::string &err);
bool sessionToggleEditable(const std::string &basename, bool saveFirst, bool forceDiscard,
                           std::string &err);

// viewer.cpp
class CWindowCloseListener;
class CPaintMouseListener;
bool loadBankFile(const std::string &bankPath, bool bankRecursive,
                  const std::vector<std::string> &searchPaths, NL3D::CTileBank &bank);
void zpViewerScreenshot(NL3D::IDriver *driver, NLMISC::CEventListenerAsync &async);
int runViewer(std::vector<SPaintZone> &zones, NL3D::CTileBank &bank, ZPPAINT::CPaintCore *core,
              PMAXLOAD::SLoadedMax &lm,
              const std::string &screenshotPath, const std::string &fontPath,
              const std::string &scriptPath, const std::string &savePath,
              NL3D::UDriver *externalDriver = NULL,
              ZPUI::CEditorUI *externalEditorUI = NULL);

#endif // ZONE_PAINTER_ZP_STATE_H
