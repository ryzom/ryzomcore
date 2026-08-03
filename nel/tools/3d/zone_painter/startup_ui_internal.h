/**
 * \file startup_ui_internal.h
 * \brief SStartupSession + NLGUI lookup/spawn helpers for the zone_painter UI TUs
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Not a public header: consumers are the zone_painter UI TUs. The paint target board
 * reuses zone_browser widgets in session mode, so both TUs need the pump's session state.
 * Helper definitions live in startup_ui.cpp.
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

#ifndef ZONE_PAINTER_STARTUP_UI_INTERNAL_H
#define ZONE_PAINTER_STARTUP_UI_INTERNAL_H

#include "startup_ui.h"

#include <set>
#include <utility>

namespace NLGUI
{
class CInterfaceGroup;
class CViewText;
class CViewTextMenu;
class CGroupList;
class CGroupContainer;
class CGroupEditBox;
}

namespace ZPUI {

// ---------------------------------------------------------------------------------------------
// Session state for the startup pump (action handlers + runStartupFlow share this)

enum EScreen
{
	ScreenNone = 0,
	ScreenWorld,
	ScreenZone,
	ScreenFolder
};

struct SStartupSession
{
	bool Active;
	bool Quit;
	bool OpenZone;
	bool FolderBrowserEnabled;
	EScreen Screen;
	std::vector<ZPWS::SWorldEntry> *Worlds;
	std::vector<ZPWS::SZoneEntry> Zones;
	int SelectedWorld; // index into *Worlds
	int SelectedZone;  // index into Zones (single open; last L-clicked)
	/** Pending multi-select: zone indices into Zones (used cells only). */
	std::set<int> PendingSelect;
	std::string FolderPath;
	std::string StatusMsg;
	/** Unused ecosystem open layout field; kept at "1x1". */
	std::string InstanceLayout;
	/** True while zone_browser is the in-session board over the live viewer. */
	bool SessionMode;
	/** Basename pending close-confirm / cell-action popup. */
	std::string PendingActionBasename;
	SStartupSession()
		: Active(false), Quit(false), OpenZone(false), FolderBrowserEnabled(false),
		  Screen(ScreenNone), Worlds(NULL), SelectedWorld(-1), SelectedZone(-1),
		  InstanceLayout("1x1"), SessionMode(false)
	{
	}
};

extern SStartupSession s_Sess;
extern SSessionBoardBridge *s_SessionBridge;

// ---------------------------------------------------------------------------------------------
// NLGUI lookup / spawn helpers implemented in startup_ui.cpp; called from session_board_ui.cpp too.

NLGUI::CInterfaceGroup *findGroup(const char *id);
NLGUI::CViewText *findText(const char *id);
/** A context-menu action line (CGroupMenu's <action> items: text + grayed state,
 *  no separate "frozen button" concept; grayed lines are also unclickable). */
NLGUI::CViewTextMenu *findMenuLine(const char *id);
NLGUI::CGroupList *findList(const char *id);
NLGUI::CGroupContainer *findContainer(const char *id);
NLGUI::CGroupEditBox *findEditBox(const char *id);

void setContainerActive(const char *id, bool active);
void setStatus(const char *windowStatusId, const std::string &msg);
void clearList(const char *listId);

NLGUI::CInterfaceGroup *spawnRow(const char *templateName, const char *parentListId,
                                 const std::vector<std::pair<std::string, std::string> > &params);
NLGUI::CInterfaceGroup *spawnUnder(NLGUI::CInterfaceGroup *parent, const char *templateName,
                                   const std::vector<std::pair<std::string, std::string> > &params,
                                   sint32 x, sint32 y, sint32 w, sint32 h);

// ---------------------------------------------------------------------------------------------
// Board helpers implemented in startup_ui.cpp; the session hub reuses them.

/** Toggle Screen B list (ecosystem) vs minesweeper board (continent). */
void setZoneBrowserMode(bool continentBoard);

/** Update Open-selection button label/frozen + optional status. */
void refreshBoardSelectionUI();

bool parseScratchBasename(const std::string &base, char &kind, int &cx, int &cy,
                          bool *isInstanceOrigin = NULL);
std::string scratchEditableName(const std::string &base);
std::string stripLigoFamilyPrefix(const std::string &name);

/** Apply live state fill + label (dirty * / R90 / M glyphs; prefix strip). */
void applySessionCellState(NLGUI::CInterfaceGroup *cell, const std::string &basename,
                           ESessionCellState st);

void setBoardCellSelected(int zoneIdx, bool selected);
void clearBoard();
void populateContinentGrid(const ZPWS::SWorldEntry &world);
/** Unused layout selector chrome: always hide; list/board sit under world_sub. */
void setLayoutSelectorVisible(bool visible);

// ---------------------------------------------------------------------------------------------
// Session board popups implemented in session_board_ui.cpp; invoked from startup_ui.cpp too.

void openCellActionPopup(const std::string &basename);
void openInstanceActionPopup(const std::string &basename);
void openEmptyCellPopup(const std::string &basename);
void openContextActionPopup(const std::string &basename);

} // namespace ZPUI

#endif // ZONE_PAINTER_STARTUP_UI_INTERNAL_H

/* end of file */
