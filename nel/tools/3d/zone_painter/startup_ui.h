/**
 * \file startup_ui.h
 * \brief NeL-GUI startup screens for zone_painter (world select / zone browser) — ui M2
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Include contract: NLMISC + NL3D UDriver + NLGUI + editor_ui.h + workspace_discovery.h.
 * Must NOT include patch_eval.h, context_display.h, or SCENELIB headers. Zone .max loading
 * stays in main.cpp; this unit only discovers/selects and returns the chosen paths.
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

#ifndef ZONE_PAINTER_STARTUP_UI_H
#define ZONE_PAINTER_STARTUP_UI_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

#include "workspace_discovery.h"

namespace NL3D {
class UDriver;
}

namespace ZPUI {

class CEditorUI;

enum EStartupResult
{
	StartupQuit = 0,         ///< ESC / window close
	StartupOpenZone,         ///< user (or auto) picked a zone — fill selection
	StartupScreenshotDone,   ///< --startup-screenshot wrote a frame
	StartupError = -1
};

struct SStartupSelection
{
	ZPWS::SWorldEntry World;
	ZPWS::SZoneEntry Zone; // first / primary (compat with single-open callers)
	/** All editable zones for this open (M6b multi-select). Empty means {Zone} only. */
	std::vector<ZPWS::SZoneEntry> EditableZones;
	/** @deprecated M12 — NxN open layout retired; always empty / "1x1". Prefer --place / scratch board. */
	std::string InstanceLayout;
};

/**
 * Run Screens A/B (and C when available) on an already-created UDriver + CEditorUI.
 * worlds may be empty (Screen C / empty list). On StartupOpenZone, *selection is filled
 * and the same select functions the buttons call were used.
 *
 * screenshotPath: if non-empty, render one frame and return StartupScreenshotDone.
 * screenshotWorld: with screenshotPath, pre-select this world (WorldName or GraphicsRoot
 *   basename) and show Screen B (continent grid / ecosystem list) instead of Screen A.
 *
 * folderBrowserEnabled: when true (M2c+), empty worlds open Screen C; Browse works.
 */
EStartupResult runStartupFlow(NL3D::UDriver *driver,
                              CEditorUI *editorUI,
                              std::vector<ZPWS::SWorldEntry> &worlds,
                              const std::string &screenshotPath,
                              SStartupSelection &selection,
                              bool folderBrowserEnabled,
                              const std::string &initialBrowsePath = std::string(),
                              const std::string &screenshotWorld = std::string());

/** Same selection path buttons use — for --startup-auto thin wrapper reuse.
 *	Supports multi: "world/zoneA+zoneB" fills EditableZones (M6b).
 *	preferRoot: pass the CLI seed folder so a seed workspace wins over LastGraphicsFolder
 *	when both expose the same WorldName (see ZPWS::selectAutoMulti). */
bool startupSelectWorldZone(const std::vector<ZPWS::SWorldEntry> &worlds,
                            const std::string &autoPath,
                            SStartupSelection &selection,
                            std::string &err,
                            const std::string &preferRoot = std::string());

/** Show/hide startup windows + painter panel. */
void startupHideAllScreens();
void startupShowPainter(bool show);

/** M24d board drag: pointer listener feeds left-button transitions here (eco board only).
 *	Begin arms a drag from the cell under the pointer; end drops onto the cell under the
 *	pointer (copyModifier = Ctrl/Shift held on release). No-ops when the board is hidden. */
void sessionBoardDragBegin();
void sessionBoardDragEnd(bool copyModifier);

// ---------------------------------------------------------------------------------------------
// Session board hub (ui M11a/M12c) — over the live viewer (session intact).
// Continent: working-set open/close/save/toggle. Ecosystem: scratch board for brick instances
// (home + place/rotate/mirror/remove). NxN Screen B layout selector retired (M12).

/** Per-cell live state for the session board (Explorer-style fill variants). */
enum ESessionCellState
{
	CellClosed = 0,       ///< not in working set (default used-cell look)
	CellOpenEditable,     ///< open as paint target (selection-fill tone)
	CellOpenReadOnly,     ///< open as frozen context (dimmer tint)
	CellDirtyEditable,    ///< open-editable with unsaved paint (fill + dirty marker)
	// Ecosystem scratch (M12c/M16c):
	CellScratchHome,      ///< open brick home cell (editable fill)
	CellScratchInstance,  ///< placed instance (distinct tint; label carries R90/M glyphs)
	CellScratchEmpty,     ///< UNLOCKED empty well (edge-adjacent to occupied / hint-named)
	CellScratchContext,   ///< read-only context brick at cell (M16c; dim RO tint + name)
	CellScratchLocked     ///< M24d: locked cell — dim, no open menu (drag target only)
};

/**
 * Bridge for mid-session board actions. main.cpp fills callbacks; startup_ui drives the
 * board window.
 *
 * Continent idiom:
 *   L-click CLOSED used cell → open editable (+ auto RO ring)
 *   L-click OPEN cell        → action popup: Close / Save / Toggle editable↔RO / Cancel
 *
 * Ecosystem scratch idiom (M12c):
 *   L-click EMPTY cell     → place instance of the open brick (current placement rot/mirror)
 *   L-click INSTANCE cell  → popup: Rotate CW / CCW / Mirror / Remove
 *   L-click HOME           → no-op (primary stays)
 *   ToggleBoard / BACK TO PAINTING → hide board
 */
struct SSessionBoardBridge
{
	const ZPWS::SWorldEntry *World; // non-null while session board usable
	/** Return live state for a zone basename; false if unknown. */
	bool (*getCellState)(const std::string &basename, ESessionCellState &out);
	/** Open a closed zone as editable (rebuilds assembly). Continent only. */
	bool (*openZone)(const std::string &basename, std::string &err);
	/** Close an open zone. If dirty editable and forceDiscard is false, UI shows confirm first. */
	bool (*closeZone)(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
	/** Save one dirty editable (same per-file path as save-all). */
	bool (*saveZone)(const std::string &basename, std::string &err);
	/** Toggle editable↔read-only. Dirty editable→RO requires saveFirst or forceDiscard. */
	bool (*toggleEditable)(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
	/** True when basename is currently dirty (editable only). */
	bool (*isDirty)(const std::string &basename);
	/** True when basename is open in the working set (editable or RO). */
	bool (*isOpen)(const std::string &basename);
	/** True when basename is open as editable (not RO neighbor). */
	bool (*isEditable)(const std::string &basename);

	// Ecosystem scratch board (M12c/M14a) — cell basenames:
	//   "H:cx,cy" home cell of multi-cell footprint (also bare "H" for 1×1 origin)
	//   "I:ox,oy" instance origin (labels); non-origin block cells share state via lookup
	//   "E:cx,cy" empty
	/** Place instance with block origin at empty cell (cx,cy); refuses overlap. */
	bool (*scratchPlace)(int cx, int cy, std::string &err);
	/** Rotate instance CW (+1) or CCW (-1) about its footprint-block center; updates origin. */
	bool (*scratchRotate)(int cx, int cy, int delta, std::string &err);
	/** Toggle mirror on instance. */
	bool (*scratchMirror)(int cx, int cy, std::string &err);
	/** Remove instance whose block contains cell. */
	bool (*scratchRemove)(int cx, int cy, std::string &err);
	/** Query instance transform for label glyphs; false if not an instance cell. */
	bool (*scratchGetInstance)(int cx, int cy, uint &rot, bool &mirror);
	/** Resolve any cell in a block to the place origin (ox,oy) + transform. */
	bool (*scratchGetInstanceOrigin)(int cx, int cy, int &ox, int &oy, uint &rot, bool &mirror);
	// M16c: read-only context brick placement on the ecosystem scratch board
	/** Place existing world brick basename as RO context at fine cell (cx,cy). */
	bool (*scratchPlaceContext)(int cx, int cy, const std::string &basename, std::string &err);
	/** Remove RO context whose origin is at / covers (cx,cy). */
	bool (*scratchRemoveContext)(int cx, int cy, std::string &err);
	/** Query context at cell; fills basename. */
	bool (*scratchGetContext)(int cx, int cy, std::string &basename);
	// M24a eco multi-editable: open files placed on the scratch board ("F:ox,oy:name" cells).
	// close/save/toggle for these ride the continent per-file callbacks above (basename-addressed).
	/** Open world brick as EDITABLE with footprint origin at (cx,cy); refuses overlap. */
	bool (*scratchOpenEditable)(int cx, int cy, const std::string &basename, std::string &err);
	/** Open-file block covering (cx,cy) → origin cell + basename + editable flag. */
	bool (*scratchGetEditableAt)(int cx, int cy, int &ox, int &oy, std::string &basename, bool &editable);
	/** Convert the RO context brick at (cx,cy) into an editable open file (same cell). */
	bool (*scratchContextToEditable)(int cx, int cy, std::string &err);
	// M24b: instance ANY open brick (home or open editable) — clones share the source's
	// carriers so painting the source repaints every instance live.
	bool (*scratchPlaceInstanceOf)(int cx, int cy, const std::string &basename, std::string &err);
	/** Open-brick count (home + editables); 1 = only home (skip the source picker). */
	int (*scratchOpenFileCount)();
	/** Instance source basename at cell (empty = home instance). */
	bool (*scratchGetInstanceSource)(int cx, int cy, std::string &basename);
	// M24c hint offers: the saved working set's per-cell suggestions surface in the board
	// menus (empty cell: one-click open of the remembered neighbor) and sort the picker.
	/** Saved-neighbor hint naming board cell (cx,cy), if any. */
	bool (*scratchGetHintAt)(int cx, int cy, std::string &basename);
	/** All hinted basenames for this session (picker priority). */
	void (*scratchHintNames)(std::vector<std::string> &out);
	// M24c: context brick transform (rotate/mirror about its footprint-block center;
	// same convention as instances; persists in the neighbor hints)
	bool (*scratchRotateContext)(int cx, int cy, int delta, std::string &err);
	bool (*scratchMirrorContext)(int cx, int cy, std::string &err);
	bool (*scratchGetContextTransform)(int cx, int cy, uint &rot, bool &mirror);
	/** M24d: drag move/copy — shift the block whose cell is (fromCx,fromCy) by the drag
	 *	delta; copy=true duplicates (Ctrl/Shift-drag; home copies as a home instance). */
	bool (*scratchDragDrop)(int fromCx, int fromCy, int toCx, int toCy, bool copy, std::string &err);
	/** Home brick display name. */
	std::string ScratchHomeName;
	/** Primary footprint size in fine cells (M14a multi-cell occupancy). */
	int FootprintCellsW;
	int FootprintCellsH;
	/**
	 * Optional exporter-identical occupancy mask over [0,W)×[0,H) (M17).
	 * NULL / empty → treat every cell in the W×H rect as occupied (legacy AABB).
	 * Row-major index = x + y * FootprintCellsW; true = claimed.
	 */
	const std::vector<bool> *FootprintMask;

	SSessionBoardBridge()
		: World(NULL), getCellState(NULL), openZone(NULL), closeZone(NULL),
		  saveZone(NULL), toggleEditable(NULL), isDirty(NULL), isOpen(NULL), isEditable(NULL),
		  scratchPlace(NULL), scratchRotate(NULL), scratchMirror(NULL), scratchRemove(NULL),
		  scratchGetInstance(NULL), scratchGetInstanceOrigin(NULL),
		  scratchPlaceContext(NULL), scratchRemoveContext(NULL), scratchGetContext(NULL),
		  scratchOpenEditable(NULL), scratchGetEditableAt(NULL), scratchContextToEditable(NULL),
		  scratchPlaceInstanceOf(NULL), scratchOpenFileCount(NULL), scratchGetInstanceSource(NULL),
		  scratchGetHintAt(NULL), scratchHintNames(NULL),
		  scratchRotateContext(NULL), scratchMirrorContext(NULL), scratchGetContextTransform(NULL),
		  scratchDragDrop(NULL),
		  FootprintCellsW(1), FootprintCellsH(1), FootprintMask(NULL)
	{
	}
};

void setSessionBoardBridge(SSessionBoardBridge *bridge);
SSessionBoardBridge *getSessionBoardBridge();

/** Show/hide the session board over the live viewer (continent working set / ecosystem scratch). */
void setSessionBoardVisible(bool visible);
void toggleSessionBoard();
bool isSessionBoardVisible();

/** Rebuild cell fills/markers from the bridge (call after open/close/save/paint/place). */
void refreshSessionBoardStates();

/** Repopulate + refresh after an op that changes eco occupancy (blocks appear/move). */
void refreshBoardAfterSessionOp();

/** Dev/test: open the close-confirm modal for one screenshot frame. */
void forceShowCloseConfirmForShot(const std::string &basename = std::string());

/** Dev/test: open the cell-action popup for one screenshot frame. */
void forceShowCellActionForShot(const std::string &basename = std::string());

/** Dev/test: open the instance-action popup (ecosystem scratch) for one screenshot frame. */
void forceShowInstanceActionForShot(const std::string &basename = std::string());

/** Dev/test: open the empty-cell popup (ecosystem scratch) for one screenshot frame. */
void forceShowEmptyCellForShot(const std::string &basename = std::string());

/** Dev/test: open the context-brick action popup (ecosystem scratch) for one screenshot frame. */
void forceShowContextActionForShot(const std::string &basename = std::string());

} // namespace ZPUI

#endif // ZONE_PAINTER_STARTUP_UI_H

/* end of file */
