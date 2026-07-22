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
	/** Ecosystem self-instance layout ("1x1".."3x3"); empty means 1x1. Continent ignores. */
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
 *	Supports multi: "world/zoneA+zoneB" fills EditableZones (M6b). */
bool startupSelectWorldZone(const std::vector<ZPWS::SWorldEntry> &worlds,
                            const std::string &autoPath,
                            SStartupSelection &selection,
                            std::string &err);

/** Show/hide startup windows + painter panel. */
void startupHideAllScreens();
void startupShowPainter(bool show);

// ---------------------------------------------------------------------------------------------
// Session board hub (ui M11a) — continent grid over the live viewer (session intact).
// Ecosystems keep the single-file flow this milestone; only continents use the session board.

/** Per-cell live state for the session board (Explorer-style fill variants). */
enum ESessionCellState
{
	CellClosed = 0,       ///< not in working set (default used-cell look)
	CellOpenEditable,     ///< open as paint target (selection-fill tone)
	CellOpenReadOnly,     ///< open as frozen context (dimmer tint)
	CellDirtyEditable     ///< open-editable with unsaved paint (fill + dirty marker)
};

/**
 * Bridge for mid-session board actions. main.cpp fills callbacks; startup_ui drives the
 * board window. Interaction idiom (documented in --help / legend):
 *   L-click CLOSED used cell → open editable (+ auto RO ring)
 *   L-click OPEN cell        → action popup: Close / Save / Toggle editable↔RO / Cancel
 *   ToggleBoard key / BACK TO PAINTING → hide board, return to painting
 */
struct SSessionBoardBridge
{
	const ZPWS::SWorldEntry *World; // current continent world (non-null while session board usable)
	/** Return live state for a zone basename; false if unknown. */
	bool (*getCellState)(const std::string &basename, ESessionCellState &out);
	/** Open a closed zone as editable (rebuilds assembly). */
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

	SSessionBoardBridge()
		: World(NULL), getCellState(NULL), openZone(NULL), closeZone(NULL),
		  saveZone(NULL), toggleEditable(NULL), isDirty(NULL), isOpen(NULL), isEditable(NULL)
	{
	}
};

void setSessionBoardBridge(SSessionBoardBridge *bridge);
SSessionBoardBridge *getSessionBoardBridge();

/** Show/hide the session board over the live viewer (continent only). */
void setSessionBoardVisible(bool visible);
void toggleSessionBoard();
bool isSessionBoardVisible();

/** Rebuild cell fills/markers from the bridge (call after open/close/save/paint). */
void refreshSessionBoardStates();

/** Dev/test: open the close-confirm modal for one screenshot frame. */
void forceShowCloseConfirmForShot(const std::string &basename = std::string());

/** Dev/test: open the cell-action popup for one screenshot frame. */
void forceShowCellActionForShot(const std::string &basename = std::string());

} // namespace ZPUI

#endif // ZONE_PAINTER_STARTUP_UI_H

/* end of file */
