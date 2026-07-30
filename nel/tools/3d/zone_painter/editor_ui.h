/**
 * \file editor_ui.h
 * \brief In-engine NLGUI facade for the standalone zone painter
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 *
 * Embeds NLGUI over the viewer's UDriver (the sample at nel/samples/gui is the
 * embedding contract). Owns atlas/parser init, per-frame update+draw, visibility
 * (F10 / ToggleUI), and wantsMouse() so paint/orbit do not punch through windows.
 *
 * Paint ops never live here: a SPaintUIBridge points at named handlers owned by
 * runViewer so keys and GUI buttons share one implementation.
 *
 * TU hygiene: this unit must NOT include patch_eval.h or context_display.h /
 * SCENELIB headers (see main.cpp include contract).
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

#ifndef ZONE_PAINTER_EDITOR_UI_H
#define ZONE_PAINTER_EDITOR_UI_H

#include <nel/misc/types_nl.h>

#include <set>
#include <string>

namespace NL3D {
class UDriver;
class UTextContext;
class CTileBank;
}

namespace NLGUI {
class CEventListener;
}

namespace ZPUI {

/** Pointer-button state mirror (NLGUI does not feed button-down into the
 *	view pointer on its own; the client / GUI sample do this). */
class CPointerButtonListener;

/**
 * Bridge between the viewer paint state and the NLGUI action handlers.
 * runViewer fills the state fields each frame and installs the function
 * pointers once; action handlers call the functions (never reimplement ops).
 */
struct SPaintUIBridge
{
	// Named handlers (same paths as the keyboard)
	void (*selectMode)(int mode); // 0=Tile 1=Color 2=Displace 3=Prop 4=Patch
	void (*selectSubObject)(int level); // patch-edit sub-object, EP_* order
	void (*selectTileSetDelta)(int d); // -1 / +1
	/** Absolute tile-set select (palette cell / digit keys); shared path. */
	void (*selectTileSetAbs)(int idx);
	void (*toggleTileSize)(); // 128 <-> 256
	void (*brushSizeDelta)(int d); // -1 / +1 (0..2 for tile/disp)
	void (*groupDelta)(int d); // -1 / +1 (mod 13)
	void (*toggleLockBorders)();
	void (*undo)();
	void (*redo)();
	void (*fill)(int rot); // 0..3
	void (*save)(); // panel Save: modal (interactive) or direct (--save)
	/** Write-back + whole-file save to an arbitrary path (modal "Save copy", test hook). */
	bool (*saveTo)(const std::string &target);
	/** In-place overwrite: temp → optional one-time .bak → rename (modal "Overwrite").
	 * Multi-file this is save-all; toolbar SAVE calls it directly in board sessions. */
	bool (*saveOverwrite)();
	/** Per-file board saves (cell "Save as…" dialog): overwrite ONE editable
	 * file in place, or save a copy under `name` (absolute, or relative to the FILE's
	 * own directory). wantThumb = the dialog's checkbox (custom save option). */
	bool (*saveFileOverwrite)(const std::string &basename, bool wantThumb);
	bool (*saveFileCopy)(const std::string &basename, const std::string &name, bool wantThumb);
	/** Directory of one OPEN editable's .max, "" if unknown. The bound "Save as…"
	 * form's exists-check must resolve relative names against the SAME directory
	 * saveFileCopy does (the file's own), not InputDir (the first-opened file's). */
	std::string (*fileDir)(const std::string &basename);
	/** Cycle landscape season textures; no-op when <2 seasons available. */
	void (*seasonNext)();
	/** Select a specific season code (sp|su|au|wi); same live-flush path as seasonNext. */
	void (*seasonSelect)(const std::string &code);
	/** Fill a CGroupMenu with available seasons (toolbar menu). Menu is CGroupMenu*. */
	void (*seasonMenuFill)(void *menu);
	/** Pivot-point mode select, and "put the user pivot where the selection is". */
	void (*selectPivotMode)(int mode);
	void (*userPivotToSelection)();
	// Color / displace: same paths as keyboard Home/End/Ins/Del/S/Q/[ ]
	/** Color brush radius ± (×1.5 / ÷1.5, clamp 2..32); panel always; keys via brushSizeDelta in Color mode. */
	void (*colorRadiusDelta)(int d);
	void (*hardnessDelta)(int d); // ± step (keys use ±51 on 0..255)
	void (*opacityDelta)(int d);
	void (*cycleBrushMask)(); // none → mask1 → … → none (S key)
	void (*toggleMaskMode)(); // Q key
	void (*displaceIndexDelta)(int d); // ±1 mod 16 ([ ] keys)
	/** Absolute displace index 0-15 (palette cell); shared path with [ ] / panel stepper. */
	void (*displaceIndexAbs)(int idx);
	/** Show/hide the Tiles palette window (key TogglePalette / panel button). */
	void (*togglePalette)();
	/** Show/hide the session board hub (key ToggleBoard / panel BOARD). Continent only. */
	void (*toggleBoard)();
	/** Set brush color RGB 0-255 (color picker / panel); same field as --color. */
	void (*setBrushColor)(int r, int g, int b);
	// Prop mode: export-property steppers / toggles (selected zone)
	void (*propRotateDelta)(int d); // ±1 mod 4
	void (*propToggleSymmetry)();
	void (*propTogglePassable)();
	void (*propToggleUseBBox)();
	// Patch mode: bind / unbind / no-smooth / delete (op surface lives in patch_edit_ops /
	// patch_topo_ops; the panel buttons, hotkeys and painterscript all alias these)
	void (*patchBind)();
	void (*patchUnbind)();
	void (*patchNoSmooth)();
	void (*patchDelete)();
	void (*patchTurnCcw)();
	void (*patchTurnCw)();
	void (*patchSubdivide)();
	void (*patchWeld)();
	void (*patchAddQuad)();
	void (*patchDetach)();
	void (*patchElement)(); // expand face selection to whole elements
	void (*moveToZoneDir)(int dir); // scene-menu compass, 0=N..7=NW
	void (*weldTargetToggle)(); // arm/disarm the target-weld drag mode
	void (*patchWeldThreshold)(float distance); // weld dialog OK
	void (*patchExtrude)(float dz); // shift-drag commit (Z-constrained)
	void (*patchExtrudeEx)(float h, float outline, bool local); // extrude dialog OK
	void (*patchFilterVertsToggle)(); // Selection block: pick filter checkboxes
	void (*patchFilterVecsToggle)();
	void (*patchLockHandlesToggle)();
	void (*arrowsToggle)();          // orientation arrows (tile additive layer + patch overlay)
	void (*patchVertCoplanar)(int on); // scene-menu vertex type pair (1 coplanar, 0 corner)
	void (*patchInteriorMode)(int on); // scene-menu interior pair (1 auto, 0 manual+bake)
	void (*patchHide)();             // hide the current level's selection (session-only)
	void (*patchUnhideAll)();
	void (*patchSubdivPropToggle)(); // edge-subdivide Propagate checkbox
	void (*patchSmGroup)(int bit);   // Surface Properties: 32-button grid (tri-state click)
	void (*patchSmGroupClear)();     // Clear All
	void (*patchTessDelta)(int axis, int d); // 0 = U, 1 = V; +-1 steps, absolute-set apply
	void (*patchBalance)();          // even the selection's orders (max per axis)
	// Painterscript absolute state setters (recorder-replay faithful; the frame-synced
	// snapshot fields below are STALE mid-script, so scripts must not derive from them)
	void (*setTileSize256)(bool on);
	void (*setHardnessAbs)(int v); // 0..255
	void (*setOpacityAbs)(int v); // 0..255
	void (*setColorRadiusAbs)(float m); // clamp 2..32

	// State snapshot for panel sync (filled by runViewer each frame)
	bool HaveCore;
	int Mode;
	int SubObj; // patch-edit sub-object level; meaningful only while Mode == Patch
	// Patch panel snapshot: per-level selection counts and the edge no-smooth tri-state
	// (0 none flagged, 1 all flagged, 2 mixed or nothing selected).
	uint PatchSelVerts, PatchSelEdges, PatchSelFaces, PatchSelTans;
	int PatchNoSmooth;
	// Where the current level's selection lives: one zone -> its name, several -> the count.
	uint PatchSelZones;
	char PatchSelZoneName[128];
	// Editable-neighbor availability per compass direction (bit 0=N .. bit 7=NW) for the
	// scene menu's "Move to zone" block; patch level with a face selection only.
	uint MoveDirMask;
	bool WeldTargetArmed; // target-weld command mode (panel Target toggle pushed state)
	float WeldThreshold;  // last-used weld distance (seeds the dialog)
	float ExtrudeHeight;  // last-used extrude height (seeds the dialog)
	float ExtrudeOutline; // last-used bevel outline (seeds the dialog)
	bool ExtrudeLocal;    // last-used normal mode (seeds the radio pair)
	int VertCoplanar; // vertex selection's continuity type: 0 corner, 1 coplanar, 2 mixed/empty
	int PatchAuto;    // face selection's interior mode: 0 manual, 1 auto, 2 mixed/empty
	uint HiddenCount; // session hide set size (freezes Unhide All at 0)
	bool SubdivPropagate; // edge-subdivide strip walk toggle
	bool FilterVerts, FilterVecs; // vertex-level pick filters (both off is impossible)
	bool LockHandles;             // a handle move takes the corner's other handles along
	bool ShowArrows;              // orientation arrows toggle state
	uint SmGroupAll, SmGroupAny;  // smoothing bits on ALL / on ANY selected patch
	int TessU, TessV;             // first selected patch's tile orders (0 = no selection)
	int PivotMode;          // TPivotMode; what the transform gizmo is anchored on
	char PivotLabel[16];    // short face for the toolbar button
	int CurTileSet;
	uint TileSetCount;
	char TileSetName[128];
	bool Mode256;
	uint BrushSize;
	uint TileGroup;
	bool LockBorders;
	uint UndoDepth;
	bool CanSave;
	/** When true, Save opens the overwrite/copy modal (startup interactive, no --save). */
	bool InteractiveSave;
	/** True in board sessions: toolbar SAVE is one-click save-all there. */
	bool BoardSession;
	char EditableBasename[128]; // zone basename for default copy name
	char InputDir[512]; // directory of the opened .max (copy targets)
	/** Self-instance count (1 = off); panel shows INSTANCED xN when > 1. */
	uint InstanceCount;
	/** Modal "Update thumbnail" checkbox (default true for interactive save). */
	bool UpdateThumbnail;
	/** --no-thumbnail hard kill-switch: the modal hides the checkbox row so the
	 * UI never claims a thumbnail write that prepareThumbnailOverride will drop. */
	bool ThumbnailsDisabled;
	/** Current season label for the panel ("spring" / "auto" / ...). */
	char SeasonLabel[32];
	/** Number of discovered season variants (0 = no seasonal tiles; button frozen). */
	uint SeasonCount;
	/** Editable file count + dirty count (multi-session; 1/0 for single). */
	uint EditableFileCount;
	uint DirtyFileCount;
	// Color brush snapshot
	float ColorRadius; // meters, 2..32
	uint ColorHardness; // 0..255
	uint ColorOpacity; // 0..255
	uint ColorR, ColorG, ColorB; // swatch
	char BrushMaskLabel[64]; // basename or "none"
	bool BrushMaskMode; // Q-key mask-mode toggle
	uint DisplaceIndex; // 0..15
	// Prop panel snapshot
	bool PropHaveSelection;
	uint PropZoneId;
	char PropZoneName[128];
	char PropFileBasename[128];
	char PropFootprint[64]; // "WxH (source=template|aabb)"
	bool PropFootprintFilled;
	bool PropEditable;
	bool PropDirty;
	int PropRotate; // 0..3
	bool PropSymmetry;
	bool PropPassable;
	bool PropUseBBox;
	char PropStatus[96]; // "read-only" / last edit

	SPaintUIBridge()
		: selectMode(NULL), selectSubObject(NULL), selectTileSetDelta(NULL), selectTileSetAbs(NULL),
		  toggleTileSize(NULL),
		  brushSizeDelta(NULL), groupDelta(NULL), toggleLockBorders(NULL),
		  undo(NULL), redo(NULL), fill(NULL), save(NULL), saveTo(NULL), saveOverwrite(NULL),
		  saveFileOverwrite(NULL), saveFileCopy(NULL), fileDir(NULL),
		  seasonNext(NULL), seasonSelect(NULL), seasonMenuFill(NULL),
		  colorRadiusDelta(NULL), hardnessDelta(NULL), opacityDelta(NULL),
		  cycleBrushMask(NULL), toggleMaskMode(NULL), displaceIndexDelta(NULL),
		  displaceIndexAbs(NULL), togglePalette(NULL), toggleBoard(NULL), setBrushColor(NULL),
		  propRotateDelta(NULL), propToggleSymmetry(NULL), propTogglePassable(NULL),
		  propToggleUseBBox(NULL),
		  patchBind(NULL), patchUnbind(NULL), patchNoSmooth(NULL), patchDelete(NULL),
		  patchTurnCcw(NULL), patchTurnCw(NULL), patchSubdivide(NULL), patchWeld(NULL), patchAddQuad(NULL),
		  patchDetach(NULL), patchElement(NULL), moveToZoneDir(NULL), weldTargetToggle(NULL),
		  patchWeldThreshold(NULL), patchExtrude(NULL), patchExtrudeEx(NULL),
		  patchVertCoplanar(NULL), patchInteriorMode(NULL),
		  patchHide(NULL), patchUnhideAll(NULL), patchSubdivPropToggle(NULL),
		  patchFilterVertsToggle(NULL), patchFilterVecsToggle(NULL), patchLockHandlesToggle(NULL),
		  arrowsToggle(NULL),
		  patchSmGroup(NULL), patchSmGroupClear(NULL), patchTessDelta(NULL), patchBalance(NULL),
		  setTileSize256(NULL), setHardnessAbs(NULL), setOpacityAbs(NULL),
		  setColorRadiusAbs(NULL),
		  HaveCore(false), Mode(0), SubObj(0),
		  PatchSelVerts(0), PatchSelEdges(0), PatchSelFaces(0), PatchSelTans(0),
		  PatchNoSmooth(2), PatchSelZones(0), MoveDirMask(0), WeldTargetArmed(false),
		  WeldThreshold(0.1f), ExtrudeHeight(8.f), ExtrudeOutline(0.f), ExtrudeLocal(false),
		  VertCoplanar(2), PatchAuto(2), HiddenCount(0), SubdivPropagate(false),
		  FilterVerts(true), FilterVecs(true), LockHandles(false),
		  ShowArrows(false),
		  SmGroupAll(0), SmGroupAny(0), TessU(0), TessV(0),
		  CurTileSet(0), TileSetCount(0), Mode256(false),
		  BrushSize(0), TileGroup(0), LockBorders(false), UndoDepth(0), CanSave(false),
		  InteractiveSave(false), BoardSession(false), InstanceCount(1), UpdateThumbnail(true),
		  ThumbnailsDisabled(false), SeasonCount(0),
		  EditableFileCount(1), DirtyFileCount(0),
		  ColorRadius(8.f), ColorHardness(128), ColorOpacity(255),
		  ColorR(255), ColorG(255), ColorB(255), BrushMaskMode(false), DisplaceIndex(0),
		  PropHaveSelection(false), PropZoneId(0), PropFootprintFilled(true),
		  PropEditable(false), PropDirty(false), PropRotate(0), PropSymmetry(false),
		  PropPassable(false), PropUseBBox(false)
	{
		TileSetName[0] = 0;
		PatchSelZoneName[0] = 0;
		EditableBasename[0] = 0;
		InputDir[0] = 0;
		SeasonLabel[0] = 0;
		BrushMaskLabel[0] = 'n';
		BrushMaskLabel[1] = 'o';
		BrushMaskLabel[2] = 'n';
		BrushMaskLabel[3] = 'e';
		BrushMaskLabel[4] = 0;
		PropZoneName[0] = 0;
		PropFileBasename[0] = 0;
		PropFootprint[0] = 0;
		PropStatus[0] = 0;
	}
};

/** Open the Save modal (Overwrite / Save copy / Cancel). Prefills copy name. */
void openSaveDialog();
/** Open the save dialog bound to ONE editable file (board cell "Save as…"). */
void openSaveDialogForFile(const std::string &basename);

/** Dev/test: force the Save modal open for one screenshot frame (env/guard callers). */
void forceShowSaveDialogForShot();

/**
 * Rebuild the Tiles palette from the loaded bank.
 * Fills the tileset grid (64px diffuse previews under thumbcache/tileset/) and the
 * Displace section (indices 0-15, noise-map previews under thumbcache/displace/).
 * tilesetForDisplace selects which set's _DisplacementBitmap maps sub-index → file
 * (current painter tileset; falls back to 0). bank may be NULL (clears both grids).
 */
void rebuildTilesetPalette(NL3D::CTileBank *bank, const std::string &bankPath,
                           const std::string &seasonKey, int tilesetForDisplace = 0);

/** Show/hide / toggle the Tiles palette window. */
void setTilesetPaletteVisible(bool visible);
/** Show/hide the painterscript window (SCRIPT toolbar button / shot hook). */
void setScriptWindowVisible(bool visible);
void toggleTilesetPalette();
bool isTilesetPaletteVisible();

/** Dev/test: scroll the palette body so the Displace section is in view. */
void scrollPaletteToDisplaceSection();

/** Show/hide the brush color picker window. */
void setColorPickerVisible(bool visible);
void toggleColorPicker();
bool isColorPickerVisible();
/** Dev/test: force the color picker open for one screenshot frame. */
void forceShowColorPickerForShot();

/** Install / clear the process-wide bridge (action handlers look it up). */
void setPaintUIBridge(SPaintUIBridge *bridge);
SPaintUIBridge *getPaintUIBridge();

/**
 * Viewer-side NLGUI shell. init() is optional-fail: if the atlas or interface
 * XML is missing the viewer continues with keyboard+HUD only.
 */
class CEditorUI
{
public:
	CEditorUI();
	~CEditorUI();

	/** Search paths → CViewRenderer → atlas → parse ui/*.xml → activate master
	 *	group. Returns false on soft failure (viewer keeps working without UI). */
	bool init(NL3D::UDriver *driver, const std::string &fontPathHint);

	void shutdown();

	/** Per-frame: push bridge state into widgets, then widget clock + coords. */
	void update();

	/** Draw GUI over the 3D scene (after scene render, before swap). */
	void draw();

	/** True when the cursor is over a GUI window or the GUI has pointer capture.
	 *	Paint strokes and orbit navigation must not consume those events. */
	bool wantsMouse() const;

	bool isReady() const { return _Ready; }
	bool isVisible() const { return _Visible; }
	void setVisible(bool visible);
	void toggleVisible();

private:
	void syncPanelFromBridge();

	bool _Ready;
	bool _Visible;
	NL3D::UDriver *_Driver;
	NL3D::UTextContext *_TextContext;
	NLGUI::CEventListener *_GuiListener;
	CPointerButtonListener *_PointerButtons;
	std::set<std::string> _HwCursors;

	static const char *MASTER_GROUP;
};

} // namespace ZPUI

#endif // ZONE_PAINTER_EDITOR_UI_H

/* end of file */
