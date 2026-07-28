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
 * One NODE: a placement of an object in the session, in Max's sense.
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
	/// This is the node half of the Max node/object split. ObjectTM alone is the file's own
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

/** Index-based readback, so callers that cannot include this header still reach the set. */
uint zpPatchVertSelCount();
bool zpPatchVertSelAt(uint index, uint &zoneOut, uint &vertOut);

/**
 * Displayed world position of one cage vertex - where the marker is actually drawn.
 *
 * Exists for the gates: it is the only quantity that shows whether an edit came back through
 * the node's placement correctly. A move can write the right bytes to the .max and still put
 * the display a board cell away, and nothing else observable distinguishes the two.
 */
bool zpPatchVertWorld(uint zoneId, uint vertIdx, float outPos[3]);

/** Selection ops (recorded). Op: 0 replace, 1 add, 2 remove. */
void zpPatchVertSelect(uint zoneId, uint vertIdx, int op);
void zpPatchVertClear();
/** Nearest editable-zone vertex within the screen pick radius. False if nothing is close. */
bool zpPickPatchVertex(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                       uint &zoneOut, uint16 &vertOut);
/** Centroid of the current vertex selection in world space. False when nothing is selected. */
bool zpPatchSelCentroid(NLMISC::CVector &out);
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
 * Gizmo drag. The delta is a PREVIEW: the cage draws with it applied, nothing is written to
 * the .max, and endDrag currently discards it - the write path does not exist yet, and a
 * preview that silently failed to persist would be worse than one that says so.
 */
bool zpPatchGizmoBeginDrag(int handle, NL3D::CCamera *camera, const NL3D::CViewport &vp,
                           float mouseX, float mouseY);
void zpPatchGizmoUpdateDrag(NL3D::CCamera *camera, const NL3D::CViewport &vp,
                            float mouseX, float mouseY);
void zpPatchGizmoEndDrag();
bool zpPatchGizmoDragging();
/** Preview offset for one vertex: the live delta if it is selected and free, else zero. */
const NLMISC::CVector &zpPatchVertDragOffset(uint zoneId, uint16 vertIdx);

/**
 * Commit a world-space move of the current vertex selection into the .max. Returns the number
 * of vertices written; `msg` explains a zero (or a partial). Skips bound vertices by policy.
 */
uint zpApplyPatchMove(const NLMISC::CVector &worldDelta, std::string &msg);
extern bool g_PatchLiveUpdate;
extern bool g_PatchWeldSelect;
/**
 * Push the selection's CURRENT display positions into the live landscape and refresh the
 * tessellation of every patch they touch, plus its bind neighbours - the same shape as the
 * paint path's applyChanges. `preview` adds the in-flight drag delta; false pushes what is
 * already committed. Returns false when a control point fell outside the zone's packed range,
 * which only a rebuild can fix.
 */
bool zpPatchPushLive(bool preview);

/** Core geom-changed sink: keeps Ep.Pm and the display patchinfo in step with the .max. */
void zpGeomVertChanged(uint zoneId, uint16 vertIdx, const float *objDelta);

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
bool rebuildWorkingSet(std::string &err, uint &outWelds);
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
