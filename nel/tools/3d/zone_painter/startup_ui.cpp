/**
 * \file startup_ui.cpp
 * \brief NeL-GUI startup screens for zone_painter (world / zone / folder) — ui M2
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Include contract: NLMISC + NL3D UDriver + NLGUI + editor_ui + workspace_discovery.
 * No patch_eval / SCENELIB / context_display.
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

#include "startup_ui.h"
#include "editor_ui.h"
#include "max_thumbnail.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <utility>

#include <nel/misc/algo.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <nel/3d/driver_user.h>
#include <nel/3d/u_driver.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_button.h>
#include <nel/gui/ctrl_scroll.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/group_container.h>
#include <nel/gui/group_list.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/interface_parser.h>
#include <nel/gui/view_bitmap.h>
#include <nel/gui/view_text.h>
#include <nel/gui/widget_manager.h>

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

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
	/** Pending multi-select (M6b): zone indices into Zones (used cells only). */
	std::set<int> PendingSelect;
	std::string FolderPath;
	std::string StatusMsg;
	/** Ecosystem open layout (M4b): "1x1" default; options 2x1/1x2/2x2/3x3. */
	std::string InstanceLayout;
	/** M11a: true while zone_browser is the in-session board over the live viewer. */
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

static SStartupSession s_Sess;
static SSessionBoardBridge *s_SessionBridge = NULL;
static bool s_SessionBoardVisible = false;

static CInterfaceGroup *findGroup(const char *id)
{
	return dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CViewText *findText(const char *id)
{
	return dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CGroupList *findList(const char *id)
{
	return dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CGroupContainer *findContainer(const char *id)
{
	return dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static void setContainerActive(const char *id, bool active)
{
	if (CGroupContainer *c = findContainer(id))
		c->setActive(active);
	else if (CInterfaceGroup *g = findGroup(id))
		g->setActive(active);
}

void startupHideAllScreens()
{
	setContainerActive("ui:zp:world_select", false);
	setContainerActive("ui:zp:zone_browser", false);
	setContainerActive("ui:zp:folder_browser", false);
}

void startupShowPainter(bool show)
{
	setContainerActive("ui:zp:painter", show);
}

// ---------------------------------------------------------------------------------------------

static void setStatus(const char *windowStatusId, const std::string &msg)
{
	s_Sess.StatusMsg = msg;
	if (CViewText *t = findText(windowStatusId))
		t->setHardText(msg);
}

static void clearList(const char *listId)
{
	if (CGroupList *list = findList(listId))
		list->deleteAllChildren();
}

static CInterfaceGroup *spawnRow(const char *templateName, const char *parentListId,
                                 const std::vector<std::pair<std::string, std::string> > &params)
{
	CGroupList *list = findList(parentListId);
	if (!list)
	{
		fprintf(stderr, "WARNING: startup UI: list '%s' not found\n", parentListId);
		return NULL;
	}
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *row = NULL;
	if (!params.empty())
		row = parser->createGroupInstance(templateName, list->getId(), &params[0], (uint)params.size());
	else
		row = parser->createGroupInstance(templateName, list->getId(), (const std::pair<std::string, std::string> *)NULL, 0);
	if (!row)
	{
		fprintf(stderr, "WARNING: startup UI: createGroupInstance(%s) failed under %s\n",
		        templateName, parentListId);
		return NULL;
	}
	// createGroupInstance leaves parent NULL; CGroupList owns the child
	row->setActive(true);
	list->addChild(row);
	// Force a known size so the list layout has non-zero row heights before the first
	// full coords pass (group templates can report H=0 until their children update).
	if (row->getH() <= 0)
		row->setH(40);
	if (row->getW() <= 0)
		row->setW(list->getW() > 0 ? list->getW() : 500);
	return row;
}

// ---------------------------------------------------------------------------------------------
// Screen population

static void populateWorldList()
{
	clearList("ui:zp:world_select:content:list_scroll:text_list");
	if (!s_Sess.Worlds)
		return;

	// Section headers as plain text rows (group labels)
	bool wroteEcoHdr = false, wroteContHdr = false;
	const std::vector<ZPWS::SWorldEntry> &worlds = *s_Sess.Worlds;
	for (size_t i = 0; i < worlds.size(); ++i)
	{
		const ZPWS::SWorldEntry &w = worlds[i];
		if (w.Kind == ZPWS::Ecosystem && !wroteEcoHdr)
		{
			wroteEcoHdr = true;
			std::vector<std::pair<std::string, std::string> > hp;
			hp.push_back(std::make_pair(std::string("id"), std::string("hdr_eco")));
			hp.push_back(std::make_pair(std::string("title"), std::string("— Ecosystems —")));
			hp.push_back(std::make_pair(std::string("subtitle"), std::string("")));
			hp.push_back(std::make_pair(std::string("idx"), std::string("-1")));
			if (CInterfaceGroup *row = spawnRow("zp_world_row", "ui:zp:world_select:content:list_scroll:text_list", hp))
			{
				// freeze the header button
				if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
					btn->setFrozen(true);
			}
		}
		if (w.Kind == ZPWS::Continent && !wroteContHdr)
		{
			wroteContHdr = true;
			std::vector<std::pair<std::string, std::string> > hp;
			hp.push_back(std::make_pair(std::string("id"), std::string("hdr_cont")));
			hp.push_back(std::make_pair(std::string("title"), std::string("— Continents —")));
			hp.push_back(std::make_pair(std::string("subtitle"), std::string("")));
			hp.push_back(std::make_pair(std::string("idx"), std::string("-1")));
			if (CInterfaceGroup *row = spawnRow("zp_world_row", "ui:zp:world_select:content:list_scroll:text_list", hp))
			{
				if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
					btn->setFrozen(true);
			}
		}

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32];
		snprintf(idbuf, sizeof(idbuf), "w%u", (uint)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		std::string title = w.WorldName;
		if (!w.BankOk)
			title += " (no bank)";
		p.push_back(std::make_pair(std::string("title"), title));
		p.push_back(std::make_pair(std::string("subtitle"), w.GraphicsRoot));
		char idxbuf[32];
		snprintf(idxbuf, sizeof(idxbuf), "%d", (int)i);
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		if (CInterfaceGroup *row = spawnRow("zp_world_row", "ui:zp:world_select:content:list_scroll:text_list", p))
		{
			if (!w.BankOk)
			{
				if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
					btn->setFrozen(true);
			}
		}
	}

	if (CGroupList *list = findList("ui:zp:world_select:content:list_scroll:text_list"))
		list->invalidateCoords();

	if (worlds.empty())
		setStatus("ui:zp:world_select:content:status", "No workspaces found. Use Browse...");
	else
		setStatus("ui:zp:world_select:content:status",
		          toString("%u workspace(s)", (uint)worlds.size()));
}

/**
 * Resolve browser thumbnail texture name for a zone (M5b).
 * Priority: embedded .max OLE thumb (cached tga) → zonebitmaps png → none (empty).
 * Adds the cache/png directory to the search path when needed.
 */
static std::string thumbTextureName(ZPWS::SZoneEntry &z)
{
	// Prefer embedded OLE thumbnail (lazy extract into thumbcache/)
	if (!z.MaxPath.empty())
	{
		std::string cached;
		if (ZPTHUMB::ensureCachedThumbnail(z.MaxPath, cached) && !cached.empty())
		{
			z.ThumbnailPath = cached;
			std::string dir = CFile::getPath(cached);
			if (!dir.empty())
				CPath::addSearchPath(dir, false, false);
			return CFile::getFilenameWithoutExtension(cached) + ".tga";
		}
	}
	// Fall back to zonebitmaps png (ecosystem)
	if (!z.ThumbnailPath.empty() && CFile::fileExists(z.ThumbnailPath))
	{
		std::string dir = CFile::getPath(z.ThumbnailPath);
		if (!dir.empty())
			CPath::addSearchPath(dir, false, false);
		// png→tga remap is set in editor_ui init
		return CFile::getFilenameWithoutExtension(z.ThumbnailPath) + ".tga";
	}
	z.ThumbnailPath.clear();
	return std::string(); // caller hides the thumb slot
}

/** Parent a template instance under an existing group at an absolute offset (continent board). */
static CInterfaceGroup *spawnUnder(CInterfaceGroup *parent, const char *templateName,
                                   const std::vector<std::pair<std::string, std::string> > &params,
                                   sint32 x, sint32 y, sint32 w, sint32 h)
{
	if (!parent)
		return NULL;
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *g = NULL;
	if (!params.empty())
		g = parser->createGroupInstance(templateName, parent->getId(), &params[0], (uint)params.size());
	else
		g = parser->createGroupInstance(templateName, parent->getId(),
		                                (const std::pair<std::string, std::string> *)NULL, 0);
	if (!g)
		return NULL;
	g->setParent(parent);
	g->setParentPos(parent);
	g->setPosRef(Hotspot_TL);
	g->setParentPosRef(Hotspot_TL);
	g->setX(x);
	g->setY(y);
	g->setW(w);
	g->setH(h);
	g->setActive(true);
	parent->addGroup(g);
	return g;
}

/** Toggle Screen B list (ecosystem) vs minesweeper board (continent). */
static void setZoneBrowserMode(bool continentBoard)
{
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:list_scroll"))
		el->setActive(!continentBoard);
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_host"))
		el->setActive(continentBoard);
	// Multi-select chrome is continent-board only (M6b/M6c); hidden in session hub (M11a)
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_legend"))
		el->setActive(continentBoard);
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:btn_open_sel"))
		el->setActive(continentBoard && !s_Sess.SessionMode);
	// Session hub chrome
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:btn_back_paint"))
		el->setActive(s_Sess.SessionMode && continentBoard);
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:btn_back")))
	{
		// Startup: Back to world list. Session: hide (use BACK TO PAINTING).
		btn->setActive(!s_Sess.SessionMode);
	}
}

/** Update Open-selection button label/frozen + optional status (M6b). */
static void refreshBoardSelectionUI()
{
	if (s_Sess.SessionMode)
	{
		// Session hub legend (M11a)
		if (CViewText *t = findText("ui:zp:zone_browser:content:board_legend"))
		{
			// M11c: full legend — interaction + live-state fills (Explorer anatomy)
			t->setHardText(
			    "L-click closed=open · open=Close/Save/Toggle · fill=edit · dim=RO · *=dirty · O/BOARD");
		}
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:btn_open_sel")))
			btn->setActive(false);
		return;
	}
	const uint n = (uint)s_Sess.PendingSelect.size();
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:btn_open_sel")))
	{
		btn->setFrozen(n == 0);
		char buf[64];
		if (n == 0)
			snprintf(buf, sizeof(buf), "Open selection");
		else
			snprintf(buf, sizeof(buf), "Open selection (%u)", n);
		if (CCtrlTextButton *tb = dynamic_cast<CCtrlTextButton *>(btn))
			tb->setHardText(buf);
	}
	// Append selection count on the board legend when non-zero
	if (CViewText *t = findText("ui:zp:zone_browser:content:board_legend"))
	{
		if (n == 0)
			t->setHardText("L-click open · R-click select · fringe = empty");
		else
			t->setHardText(NLMISC::toString(
			    "L-click open · R-click select · fringe = empty  |  %u selected", n));
	}
}

/**
 * Explorer-style board multi-select fill (M10e) + session live-state fills (M11a).
 * Same tone as palette cells: NeL CGroupTree col_select default (255 128 128 128).
 * Applied as blank.tga button face color (not setPushed brick chrome).
 *
 * Session states (M11a):
 *   closed         — transparent fill (default used-cell look)
 *   open-editable  — selection-fill (255 128 128 128)
 *   open-read-only — dimmer cool tint (80 100 140 110)
 *   dirty          — editable fill + '*' on the label
 */
static const CRGBA kBoardCellSelFill(255, 128, 128, 128);
static const CRGBA kBoardCellSelHover(255, 128, 128, 64);
static const CRGBA kBoardCellSelNone(0, 0, 0, 0);
static const CRGBA kBoardCellReadOnly(80, 100, 140, 110);
static const CRGBA kBoardCellReadOnlyHover(80, 100, 140, 80);
static const CRGBA kBoardCellDirty(255, 160, 80, 150);
static const CRGBA kBoardCellDirtyHover(255, 160, 80, 90);

static void setBoardCellFillRGBA(CInterfaceGroup *cell, const CRGBA &c, const CRGBA &hover)
{
	if (!cell)
		return;
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(cell->getCtrl("btn")))
	{
		btn->setColor(c);
		btn->setColorPushed(c);
		btn->setColorOver(hover);
	}
}

static void setBoardCellSelFill(CInterfaceGroup *cell, bool selected)
{
	setBoardCellFillRGBA(cell,
	                     selected ? kBoardCellSelFill : kBoardCellSelNone,
	                     selected ? kBoardCellSelFill : kBoardCellSelHover);
}

/** Apply M11a live state fill + dirty marker on the cell title. */
static void applySessionCellState(CInterfaceGroup *cell, const std::string &basename,
                                  ESessionCellState st)
{
	if (!cell)
		return;
	switch (st)
	{
	case CellOpenEditable:
		setBoardCellFillRGBA(cell, kBoardCellSelFill, kBoardCellSelFill);
		break;
	case CellOpenReadOnly:
		setBoardCellFillRGBA(cell, kBoardCellReadOnly, kBoardCellReadOnlyHover);
		break;
	case CellDirtyEditable:
		setBoardCellFillRGBA(cell, kBoardCellDirty, kBoardCellDirtyHover);
		break;
	case CellClosed:
	default:
		setBoardCellFillRGBA(cell, kBoardCellSelNone, kBoardCellSelHover);
		break;
	}
	// Dirty marker: '*' suffix on label (keep base name for closed/open clean)
	if (CViewText *t = dynamic_cast<CViewText *>(cell->getView("t")))
	{
		if (st == CellDirtyEditable)
			t->setHardText(basename + " *");
		else
		{
			// Short grid form if available
			int r = 0, c = 0;
			if (ZPWS::parseContinentZoneName(basename, r, c))
				t->setHardText(ZPWS::continentZoneName(r, c));
			else
				t->setHardText(basename);
		}
	}
}

/** Set board cell flat selection fill for one zone index (if the cell exists). */
static void setBoardCellSelected(int zoneIdx, bool selected)
{
	if (zoneIdx < 0) return;
	int r = 0, c = 0;
	if ((size_t)zoneIdx >= s_Sess.Zones.size())
		return;
	if (!ZPWS::parseContinentZoneName(s_Sess.Zones[zoneIdx].Basename, r, c))
		return;
	char idbuf[96];
	snprintf(idbuf, sizeof(idbuf), "ui:zp:zone_browser:content:board_host:board:gc%d_%d", r, c);
	if (CInterfaceGroup *cell = findGroup(idbuf))
		setBoardCellSelFill(cell, selected);
}

static void clearBoard()
{
	if (CInterfaceGroup *board = findGroup("ui:zp:zone_browser:content:board_host:board"))
	{
		board->clearGroups();
		board->setOfsX(0);
		board->setOfsY(0);
	}
}

/**
 * Continent Screen B (M5a): minesweeper-style square cell board.
 * Cell set = used zones + empty 8-neighbor fringe (nothing beyond). Dual-axis scroll
 * when the board exceeds the host viewport (NLGUI group max_w/max_h + CCtrlScroll).
 */
static void populateContinentGrid(const ZPWS::SWorldEntry &world)
{
	const int kCell = 52; // square; matches zp_board_* templates

	std::map<std::pair<int, int>, int> used; // (row,col) -> zone index
	uint nParse = 0;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		int r = 0, c = 0;
		if (!ZPWS::parseContinentZoneName(s_Sess.Zones[i].Basename, r, c))
			continue;
		used[std::make_pair(r, c)] = (int)i;
		++nParse;
	}

	if (nParse == 0)
	{
		// Unparseable names: flat list fallback (ecosystem-style)
		setZoneBrowserMode(false);
		for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
		{
			std::vector<std::pair<std::string, std::string> > p;
			char idbuf[32], idxbuf[32];
			snprintf(idbuf, sizeof(idbuf), "z%u", (uint)i);
			snprintf(idxbuf, sizeof(idxbuf), "%d", (int)i);
			p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
			p.push_back(std::make_pair(std::string("title"), s_Sess.Zones[i].Basename));
			p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
			p.push_back(std::make_pair(std::string("thumb"), std::string("w_box_blank.tga")));
			if (CInterfaceGroup *row = spawnRow("zp_zone_row", "ui:zp:zone_browser:content:list_scroll:text_list", p))
			{
				if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(row->getView("thumb")))
					thumb->setActive(false);
				if (CInterfaceGroup *fr = row->getGroup("thumb_frame"))
					fr->setActive(false);
			}
		}
		return;
	}

	// Fringe = empty 8-neighbors of at least one used cell
	std::set<std::pair<int, int> > fringe;
	for (std::map<std::pair<int, int>, int>::const_iterator it = used.begin(); it != used.end(); ++it)
	{
		const int r = it->first.first;
		const int c = it->first.second;
		for (int dr = -1; dr <= 1; ++dr)
		{
			for (int dc = -1; dc <= 1; ++dc)
			{
				if (dr == 0 && dc == 0)
					continue;
				const int nr = r + dr;
				const int nc = c + dc;
				const std::pair<int, int> key(nr, nc);
				if (used.find(key) == used.end())
					fringe.insert(key);
			}
		}
	}

	// Bounding box of used+fringe for board pixel size (north-up: higher row at top)
	int minR = INT_MAX, maxR = INT_MIN, minC = INT_MAX, maxC = INT_MIN;
	for (std::map<std::pair<int, int>, int>::const_iterator it = used.begin(); it != used.end(); ++it)
	{
		if (it->first.first < minR) minR = it->first.first;
		if (it->first.first > maxR) maxR = it->first.first;
		if (it->first.second < minC) minC = it->first.second;
		if (it->first.second > maxC) maxC = it->first.second;
	}
	for (std::set<std::pair<int, int> >::const_iterator it = fringe.begin(); it != fringe.end(); ++it)
	{
		if (it->first < minR) minR = it->first;
		if (it->first > maxR) maxR = it->first;
		if (it->second < minC) minC = it->second;
		if (it->second > maxC) maxC = it->second;
	}

	const int nRows = maxR - minR + 1;
	const int nCols = maxC - minC + 1;
	const int boardW = nCols * kCell;
	const int boardH = nRows * kCell;

	setZoneBrowserMode(true);
	clearBoard();

	CInterfaceGroup *board = findGroup("ui:zp:zone_browser:content:board_host:board");
	if (!board)
	{
		fprintf(stderr, "WARNING: startup UI: continent board group missing\n");
		return;
	}

	// Content size + viewport clip (scroll bars target this group)
	board->setW(boardW);
	board->setH(boardH);
	board->setMaxW(730);
	board->setMaxH(330);
	board->setOfsX(0);
	board->setOfsY(0);

	// Used cells (clickable)
	for (std::map<std::pair<int, int>, int>::const_iterator it = used.begin(); it != used.end(); ++it)
	{
		const int r = it->first.first;
		const int c = it->first.second;
		const int zi = it->second;
		const ZPWS::SZoneEntry &z = s_Sess.Zones[zi];
		// x increases east (col); y is negative down from TL, so higher row (north) is closer to 0
		const sint32 x = (c - minC) * kCell;
		const sint32 y = -((maxR - r) * kCell);

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[48], idxbuf[32];
		snprintf(idbuf, sizeof(idbuf), "gc%d_%d", r, c);
		snprintf(idxbuf, sizeof(idxbuf), "%d", zi);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		// Cell title: short grid form (4_AC / 193_EC) so prefixed converted names fit the stamp
		{
			std::string title = ZPWS::continentZoneName(r, c);
			if (title.empty())
				title = z.Basename;
			p.push_back(std::make_pair(std::string("title"), title));
		}
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		// Mutable zone entry for cache path write-back
		std::string thumbTex = thumbTextureName(s_Sess.Zones[zi]);
		p.push_back(std::make_pair(std::string("thumb"), thumbTex.empty() ? std::string("w_box_blank.tga") : thumbTex));
		CInterfaceGroup *cell = spawnUnder(board, "zp_board_cell", p, x, y, kCell, kCell);
		if (cell)
		{
			const bool hasThumb = !thumbTex.empty();
			if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(cell->getView("thumb")))
				thumb->setActive(hasThumb);
			// M10a: activate 9-slice well only when a real thumb is bound
			if (CInterfaceGroup *fr = cell->getGroup("thumb_frame"))
				fr->setActive(hasThumb);
			// Force full-cell hit/fill size. blank.tga is 4×4; without scale, updateCoords
			// resets the button to texture size every frame. Include CCtrlButton::setScale.
			if (CCtrlButton *btn = dynamic_cast<CCtrlButton *>(cell->getCtrl("btn")))
			{
				btn->setScale(true);
				btn->setW(kCell);
				btn->setH(kCell);
			}
			else if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(cell->getCtrl("btn")))
			{
				btn->setW(kCell);
				btn->setH(kCell);
			}
			// Session board (M11a): live open/dirty state. Startup multi-select fill otherwise.
			if (s_Sess.SessionMode && s_SessionBridge && s_SessionBridge->getCellState)
			{
				ESessionCellState st = CellClosed;
				s_SessionBridge->getCellState(z.Basename, st);
				applySessionCellState(cell, z.Basename, st);
			}
			else
				setBoardCellSelFill(cell, s_Sess.PendingSelect.count(zi) != 0);
		}
	}

	// Fringe placeholders (non-interactive)
	for (std::set<std::pair<int, int> >::const_iterator it = fringe.begin(); it != fringe.end(); ++it)
	{
		const int r = it->first;
		const int c = it->second;
		const sint32 x = (c - minC) * kCell;
		const sint32 y = -((maxR - r) * kCell);
		std::vector<std::pair<std::string, std::string> > ep;
		char eid[48];
		snprintf(eid, sizeof(eid), "ge%d_%d", r, c);
		ep.push_back(std::make_pair(std::string("id"), std::string(eid)));
		spawnUnder(board, "zp_board_empty", ep, x, y, kCell, kCell);
	}

	// (Re)bind dual-axis scroll targets after board_host is active (XML target may miss while inactive).
	if (CCtrlScroll *sv = dynamic_cast<CCtrlScroll *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:board_host:sv")))
		sv->setTarget(board);
	if (CCtrlScroll *sh = dynamic_cast<CCtrlScroll *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:board_host:sh")))
		sh->setTarget(board);

	board->invalidateCoords();
	if (CInterfaceGroup *host = findGroup("ui:zp:zone_browser:content:board_host"))
		host->invalidateCoords();

	// Status: board extent + silhouette stats
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
	{
		// Letter labels clamp to AA for negative fringe cols (display only).
		const int colLoIdx = minC < 0 ? 0 : minC;
		const int colHiIdx = maxC < 0 ? 0 : maxC;
		std::string colLo = ZPWS::continentZoneName(0, colLoIdx);
		std::string colHi = ZPWS::continentZoneName(0, colHiIdx);
		std::string::size_type u1 = colLo.find('_');
		std::string::size_type u2 = colHi.find('_');
		std::string lettersLo = (u1 != std::string::npos) ? colLo.substr(u1 + 1) : colLo;
		std::string lettersHi = (u2 != std::string::npos) ? colHi.substr(u2 + 1) : colHi;
		if (minC < 0) lettersLo = NLMISC::toString("%d", minC);
		if (maxC < 0) lettersHi = NLMISC::toString("%d", maxC);
		t->setHardText(world.GraphicsRoot
		               + NLMISC::toString(
		                     "  |  rows %d..%d  cols %s..%s  (%u zones + %u fringe; open loads neighbors)",
		                     minR, maxR, lettersLo.c_str(), lettersHi.c_str(),
		                     (uint)used.size(), (uint)fringe.size()));
	}
	refreshBoardSelectionUI();
}

/** Show/hide ecosystem layout radios and push list below them when active. */
static void setLayoutSelectorVisible(bool visible)
{
	static const char *ids[] = {
		"ui:zp:zone_browser:content:layout_label",
		"ui:zp:zone_browser:content:layout_1x1",
		"ui:zp:zone_browser:content:layout_2x1",
		"ui:zp:zone_browser:content:layout_1x2",
		"ui:zp:zone_browser:content:layout_2x2",
		"ui:zp:zone_browser:content:layout_3x3",
		NULL
	};
	for (int i = 0; ids[i]; ++i)
	{
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(ids[i]))
			el->setActive(visible);
	}
	// Drop the zone list / board under the layout row (or tight under world_sub on continents)
	const sint32 y = visible ? -40 : -8;
	if (CInterfaceElement *list = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:list_scroll"))
		list->setY(y);
	if (CInterfaceElement *board = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_host"))
		board->setY(y);
}

/** Sync radio pushed state to s_Sess.InstanceLayout. */
static void syncLayoutRadios()
{
	const char *layouts[] = { "1x1", "2x1", "1x2", "2x2", "3x3", NULL };
	const char *ids[] = {
		"ui:zp:zone_browser:content:layout_1x1",
		"ui:zp:zone_browser:content:layout_2x1",
		"ui:zp:zone_browser:content:layout_1x2",
		"ui:zp:zone_browser:content:layout_2x2",
		"ui:zp:zone_browser:content:layout_3x3",
		NULL
	};
	for (int i = 0; layouts[i]; ++i)
	{
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId(ids[i])))
			btn->setPushed(s_Sess.InstanceLayout == layouts[i]);
	}
}

static void populateZoneList()
{
	clearList("ui:zp:zone_browser:content:list_scroll:text_list");
	clearBoard();
	s_Sess.PendingSelect.clear();
	if (!s_Sess.Worlds || s_Sess.SelectedWorld < 0
	    || s_Sess.SelectedWorld >= (int)s_Sess.Worlds->size())
		return;

	const ZPWS::SWorldEntry &world = (*s_Sess.Worlds)[s_Sess.SelectedWorld];
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_title"))
		t->setHardText(world.WorldName
		               + (world.Kind == ZPWS::Ecosystem ? "  (ecosystem)" : "  (continent)"));
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
		t->setHardText(world.GraphicsRoot);

	// Layout selector is ecosystem-only (self-instances). Continents hide it.
	const bool eco = (world.Kind == ZPWS::Ecosystem);
	setLayoutSelectorVisible(eco);
	if (eco)
		syncLayoutRadios();

	ZPWS::listZones(world, s_Sess.Zones);

	// Continents: minesweeper-style board (M5a). Ecosystems keep the grouped list.
	if (world.Kind == ZPWS::Continent)
	{
		populateContinentGrid(world);
		return;
	}

	setZoneBrowserMode(false);

	std::string lastGroup;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		ZPWS::SZoneEntry &z = s_Sess.Zones[i];
		if (z.Group != lastGroup)
		{
			lastGroup = z.Group;
			std::vector<std::pair<std::string, std::string> > hp;
			hp.push_back(std::make_pair(std::string("id"), std::string("zg_") + lastGroup));
			hp.push_back(std::make_pair(std::string("title"), std::string("— ") + lastGroup + " —"));
			hp.push_back(std::make_pair(std::string("idx"), std::string("-1")));
			hp.push_back(std::make_pair(std::string("thumb"), std::string("w_box_blank.tga")));
			if (CInterfaceGroup *row = spawnRow("zp_zone_row", "ui:zp:zone_browser:content:list_scroll:text_list", hp))
			{
				if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
					btn->setFrozen(true);
				// Group headers never show a thumb slot / frame well
				if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(row->getView("thumb")))
					thumb->setActive(false);
				if (CInterfaceGroup *fr = row->getGroup("thumb_frame"))
					fr->setActive(false);
			}
		}

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32];
		snprintf(idbuf, sizeof(idbuf), "z%u", (uint)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		p.push_back(std::make_pair(std::string("title"), z.Basename));
		char idxbuf[32];
		snprintf(idxbuf, sizeof(idxbuf), "%d", (int)i);
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		std::string thumbTex = thumbTextureName(z);
		p.push_back(std::make_pair(std::string("thumb"), thumbTex.empty() ? std::string("w_box_blank.tga") : thumbTex));
		if (CInterfaceGroup *row = spawnRow("zp_zone_row", "ui:zp:zone_browser:content:list_scroll:text_list", p))
		{
			const bool hasThumb = !thumbTex.empty();
			if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(row->getView("thumb")))
				thumb->setActive(hasThumb);
			// M10a: 9-slice well only when a real thumb is bound (no empty dark square)
			if (CInterfaceGroup *fr = row->getGroup("thumb_frame"))
				fr->setActive(hasThumb);
			// When no thumb, tuck the open button to the left edge
			if (!hasThumb)
			{
				if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(row->getCtrl("btn")))
				{
					btn->setParentPos(row);
					btn->setPosRef(Hotspot_ML);
					btn->setParentPosRef(Hotspot_ML);
					btn->setX(4);
				}
			}
		}
	}
}

static void populateFolderList()
{
	clearList("ui:zp:folder_browser:content:list_scroll:text_list");
	if (s_Sess.FolderPath.empty())
		s_Sess.FolderPath = ZPWS::normalizeDir(CPath::getCurrentPath());

	if (CViewText *t = findText("ui:zp:folder_browser:content:path_label"))
		t->setHardText(s_Sess.FolderPath);
	setStatus("ui:zp:folder_browser:content:status", "");

	// ".." parent row
	{
		std::string parent = CFile::getPath(s_Sess.FolderPath);
		parent = ZPWS::normalizeDir(parent);
		if (!parent.empty() && parent != s_Sess.FolderPath)
		{
			std::vector<std::pair<std::string, std::string> > p;
			p.push_back(std::make_pair(std::string("id"), std::string("dotdot")));
			p.push_back(std::make_pair(std::string("title"), std::string("..")));
			p.push_back(std::make_pair(std::string("path"), parent));
			spawnRow("zp_folder_row", "ui:zp:folder_browser:content:list_scroll:text_list", p);
		}
	}

	std::vector<std::string> children;
	CPath::getPathContent(s_Sess.FolderPath, false, true, false, children);
	std::sort(children.begin(), children.end());
	for (size_t i = 0; i < children.size(); ++i)
	{
		std::string child = children[i];
		while (!child.empty() && (child[child.size() - 1] == '/' || child[child.size() - 1] == '\\'))
			child.resize(child.size() - 1);
		std::string name = ZPWS::dirBasename(child);
		if (name.empty() || name[0] == '.')
			continue; // skip .nel etc. as navigable clutter? still allow .nel parent via path
		// Allow entering .nel's parent dirs; skip only "." and ".."
		if (name == "." || name == "..")
			continue;

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32];
		snprintf(idbuf, sizeof(idbuf), "f%u", (uint)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		p.push_back(std::make_pair(std::string("title"), name));
		p.push_back(std::make_pair(std::string("path"), ZPWS::normalizeDir(child)));
		spawnRow("zp_folder_row", "ui:zp:folder_browser:content:list_scroll:text_list", p);
	}

	if (CGroupList *list = findList("ui:zp:folder_browser:content:list_scroll:text_list"))
		list->invalidateCoords();
}

static void showScreen(EScreen screen)
{
	s_Sess.Screen = screen;
	startupHideAllScreens();
	startupShowPainter(false);
	switch (screen)
	{
	case ScreenWorld:
		setContainerActive("ui:zp:world_select", true);
		populateWorldList();
		break;
	case ScreenZone:
		setContainerActive("ui:zp:zone_browser", true);
		populateZoneList();
		break;
	case ScreenFolder:
		setContainerActive("ui:zp:folder_browser", true);
		if (s_Sess.FolderPath.empty())
			s_Sess.FolderPath = ZPWS::normalizeDir(CPath::getCurrentPath());
		populateFolderList();
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------------------------
// Selection (shared with buttons and --startup-auto)

bool startupSelectWorldZone(const std::vector<ZPWS::SWorldEntry> &worlds,
                            const std::string &autoPath,
                            SStartupSelection &selection,
                            std::string &err)
{
	ZPWS::SWorldEntry w;
	std::vector<ZPWS::SZoneEntry> zones;
	if (!ZPWS::selectAutoMulti(worlds, autoPath, w, zones, err))
		return false;
	if (zones.empty())
	{
		err = "startup-auto: no zones resolved";
		return false;
	}
	selection.World = w;
	selection.Zone = zones[0];
	selection.EditableZones = zones;
	return true;
}

static void applyWorldSelection(int idx)
{
	if (!s_Sess.Worlds || idx < 0 || idx >= (int)s_Sess.Worlds->size())
		return;
	if (!(*s_Sess.Worlds)[idx].BankOk)
		return;
	s_Sess.SelectedWorld = idx;
	s_Sess.PendingSelect.clear();
	showScreen(ScreenZone);
}

static void openCellActionPopup(const std::string &basename);
static void openCloseConfirmModal(const std::string &basename, const std::string &purpose);

/** L-click: startup = open immediately; session hub = open closed / popup for open. */
static void applyZoneSelection(int idx)
{
	if (idx < 0 || idx >= (int)s_Sess.Zones.size())
		return;

	// M11a session board: closed → open; open → action popup
	if (s_Sess.SessionMode)
	{
		const std::string &base = s_Sess.Zones[idx].Basename;
		s_Sess.SelectedZone = idx;
		s_Sess.PendingActionBasename = base;
		if (s_SessionBridge && s_SessionBridge->isOpen && s_SessionBridge->isOpen(base))
		{
			openCellActionPopup(base);
			return;
		}
		if (s_SessionBridge && s_SessionBridge->openZone)
		{
			std::string err;
			if (!s_SessionBridge->openZone(base, err))
			{
				fprintf(stderr, "session board open '%s': %s\n", base.c_str(), err.c_str());
				if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
					t->setHardText(err.empty() ? "open failed" : err);
			}
			else
				refreshSessionBoardStates();
		}
		return;
	}

	s_Sess.SelectedZone = idx;
	s_Sess.PendingSelect.clear();
	s_Sess.PendingSelect.insert(idx); // for EditableZones fill
	s_Sess.OpenZone = true;
}

/** Commit PendingSelect (or single SelectedZone) into an open request. */
static void applyOpenSelection()
{
	if (s_Sess.PendingSelect.empty())
		return;
	// Prefer lowest index as primary for title/compat
	s_Sess.SelectedZone = *s_Sess.PendingSelect.begin();
	s_Sess.OpenZone = true;
}

static bool isSupportedInstanceLayout(const std::string &s)
{
	return s == "1x1" || s == "2x1" || s == "1x2" || s == "2x2" || s == "3x3";
}

// ---------------------------------------------------------------------------------------------
// Action handlers

class CAHZpSelectWorld : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		if (!s_Sess.Active) return;
		int idx = -1;
		fromString(params, idx);
		if (idx < 0) return;
		applyWorldSelection(idx);
	}
};
REGISTER_ACTION_HANDLER(CAHZpSelectWorld, "zp_select_world");

class CAHZpSelectZone : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		if (!s_Sess.Active) return;
		int idx = -1;
		fromString(params, idx);
		if (idx < 0) return;
		applyZoneSelection(idx);
	}
};
REGISTER_ACTION_HANDLER(CAHZpSelectZone, "zp_select_zone");

class CAHZpToggleZoneSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		if (!s_Sess.Active) return;
		// Session hub: R-click unused (L-click handles open/actions)
		if (s_Sess.SessionMode) return;
		int idx = -1;
		fromString(params, idx);
		if (idx < 0 || idx >= (int)s_Sess.Zones.size())
			return;
		// Only used cells (fringe has no button / this AH)
		if (s_Sess.PendingSelect.count(idx))
			s_Sess.PendingSelect.erase(idx);
		else
			s_Sess.PendingSelect.insert(idx);
		const bool on = s_Sess.PendingSelect.count(idx) != 0;
		// Drive Explorer-style blank.tga fill via setColor (not setPushed brick chrome).
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(pCaller))
		{
			const CRGBA c = on ? kBoardCellSelFill : kBoardCellSelNone;
			btn->setColor(c);
			btn->setColorPushed(c);
			btn->setColorOver(on ? kBoardCellSelFill : kBoardCellSelHover);
		}
		else
			setBoardCellSelected(idx, on);
		refreshBoardSelectionUI();
	}
};
REGISTER_ACTION_HANDLER(CAHZpToggleZoneSelect, "zp_toggle_zone_select");

class CAHZpOpenSelection : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active) return;
		if (s_Sess.PendingSelect.empty())
			return;
		applyOpenSelection();
	}
};
REGISTER_ACTION_HANDLER(CAHZpOpenSelection, "zp_open_selection");

class CAHZpSetInstances : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		if (!s_Sess.Active) return;
		std::string layout = NLMISC::toLowerAscii(params);
		if (!isSupportedInstanceLayout(layout))
			return;
		s_Sess.InstanceLayout = layout;
		syncLayoutRadios();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSetInstances, "zp_set_instances");

class CAHZpZoneBack : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (s_Sess.SessionMode)
		{
			// Session hub: treat as back-to-painting
			setSessionBoardVisible(false);
			return;
		}
		if (!s_Sess.Active) return;
		s_Sess.SelectedWorld = -1;
		s_Sess.Zones.clear();
		showScreen(ScreenWorld);
	}
};
REGISTER_ACTION_HANDLER(CAHZpZoneBack, "zp_zone_back");

class CAHZpBackToPaint : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		setSessionBoardVisible(false);
	}
};
REGISTER_ACTION_HANDLER(CAHZpBackToPaint, "zp_back_to_paint");

static void populateFolderList();
static void showScreen(EScreen screen);

class CAHZpBrowse : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active) return;
		if (!s_Sess.FolderBrowserEnabled)
		{
			setStatus("ui:zp:world_select:content:status", "Folder browser disabled");
			return;
		}
		if (s_Sess.FolderPath.empty())
			s_Sess.FolderPath = ZPWS::normalizeDir(CPath::getCurrentPath());
		showScreen(ScreenFolder);
	}
};
REGISTER_ACTION_HANDLER(CAHZpBrowse, "zp_browse");

class CAHZpStartupQuit : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active) return;
		s_Sess.Quit = true;
	}
};
REGISTER_ACTION_HANDLER(CAHZpStartupQuit, "zp_startup_quit");

class CAHZpFolderEnter : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		if (!s_Sess.Active) return;
		if (params.empty()) return;
		// params is the absolute path (may be ".." encoded as the parent path already)
		s_Sess.FolderPath = ZPWS::normalizeDir(params);
		populateFolderList();
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderEnter, "zp_folder_enter");

class CAHZpFolderSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active || !s_Sess.Worlds) return;
		// Fingerprint this folder / scan subdirs like root detection, then go to Screen A
		std::vector<ZPWS::SWorldEntry> found;
		ZPWS::discoverWorkspaces(s_Sess.FolderPath, std::string(), found);
		if (found.empty())
		{
			setStatus("ui:zp:folder_browser:content:status", "No workspaces found here");
			return;
		}
		// Replace candidate set with the selection (keep any that were already known? Task:
		// "goes to Screen A with the found workspaces")
		*s_Sess.Worlds = found;
		setStatus("ui:zp:folder_browser:content:status", "");
		showScreen(ScreenWorld);
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderSelect, "zp_folder_select");

class CAHZpFolderCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active) return;
		// Cancel back to world list even if empty
		showScreen(ScreenWorld);
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderCancel, "zp_folder_cancel");

// ---------------------------------------------------------------------------------------------

// Window-close listener local to the startup pump
class CStartupCloseListener : public IEventListener
{
public:
	bool Active;
	CStartupCloseListener() : Active(true) {}
	virtual void operator()(const CEvent &event)
	{
		if (event == EventDestroyWindowId || event == EventCloseWindowId)
			Active = false;
	}
};

EStartupResult runStartupFlow(UDriver *driver,
                              CEditorUI *editorUI,
                              std::vector<ZPWS::SWorldEntry> &worlds,
                              const std::string &screenshotPath,
                              SStartupSelection &selection,
                              bool folderBrowserEnabled,
                              const std::string &initialBrowsePath,
                              const std::string &screenshotWorld)
{
	if (!driver || !editorUI || !editorUI->isReady())
	{
		fprintf(stderr, "ERROR: startup UI: driver/editor UI not ready\n");
		return StartupError;
	}

	s_Sess = SStartupSession();
	s_Sess.Active = true;
	s_Sess.Worlds = &worlds;
	s_Sess.FolderBrowserEnabled = folderBrowserEnabled;
	if (!initialBrowsePath.empty() && CFile::isDirectory(initialBrowsePath))
		s_Sess.FolderPath = ZPWS::normalizeDir(initialBrowsePath);
	else
		s_Sess.FolderPath = ZPWS::normalizeDir(CPath::getCurrentPath());
	// Remembered open layout (ecosystem self-instances)
	{
		ZPWS::SStartupCfg cfg;
		if (ZPWS::loadStartupCfg(cfg) && isSupportedInstanceLayout(cfg.LastInstances))
			s_Sess.InstanceLayout = cfg.LastInstances;
		else
			s_Sess.InstanceLayout = "1x1";
	}

	// First screen: world list when non-empty; folder browser only when enabled + empty.
	// --startup-screen <world> (with --startup-screenshot) jumps to Screen B for that world.
	bool openedScreenB = false;
	if (!screenshotPath.empty() && !screenshotWorld.empty() && !worlds.empty())
	{
		int wi = -1;
		for (size_t i = 0; i < worlds.size(); ++i)
		{
			if (worlds[i].WorldName == screenshotWorld
			    || ZPWS::dirBasename(worlds[i].GraphicsRoot) == screenshotWorld)
			{
				if (worlds[i].BankOk)
				{
					wi = (int)i;
					break;
				}
				if (wi < 0)
					wi = (int)i; // remember disabled match but prefer BankOk
			}
		}
		if (wi >= 0 && worlds[wi].BankOk)
		{
			s_Sess.SelectedWorld = wi;
			showScreen(ScreenZone);
			openedScreenB = true;
			printf("startup-screenshot: Screen B for world '%s'\n", worlds[wi].WorldName.c_str());
			// Dev-only: ZONE_PAINTER_BOARD_SELECT pre-selects a used cell so M10e
			// Explorer-style multi-select fill is visible in headless board shots.
			// "1"/"true"/"yes" => first zone (index 0). Numeric >=0 => that zone index.
			if (worlds[wi].Kind == ZPWS::Continent && !s_Sess.Zones.empty())
			{
				const char *bsel = getenv("ZONE_PAINTER_BOARD_SELECT");
				if (bsel && bsel[0] && !(bsel[0] == '0' && bsel[1] == 0))
				{
					int zi = 0;
					// bare "1" is the common truthy flag (first cell), not index 1
					if (std::string(bsel) == "1" || std::string(bsel) == "true"
					    || std::string(bsel) == "yes")
						zi = 0;
					else if (bsel[0] >= '0' && bsel[0] <= '9')
						fromString(std::string(bsel), zi);
					if (zi < 0 || zi >= (int)s_Sess.Zones.size())
						zi = 0;
					s_Sess.PendingSelect.clear();
					s_Sess.PendingSelect.insert(zi);
					setBoardCellSelected(zi, true);
					refreshBoardSelectionUI();
					printf("startup-screenshot: board multi-select zone %d ('%s')\n",
					       zi, s_Sess.Zones[zi].Basename.c_str());
				}
			}
		}
		else
		{
			fprintf(stderr, "WARNING: startup-screen: world '%s' not found or no bank; showing Screen A\n",
			        screenshotWorld.c_str());
		}
	}
	if (!openedScreenB)
	{
		if (worlds.empty() && folderBrowserEnabled)
			showScreen(ScreenFolder);
		else
			showScreen(ScreenWorld);
	}

	driver->setWindowTitle(ucstring("zone_painter — startup"));

	// One-frame screenshot of the selected screen
	if (!screenshotPath.empty())
	{
		// Extra update passes so dynamically-spawned grid/list rows get real coords
		driver->EventServer.pump();
		editorUI->update();
		editorUI->update();
		editorUI->update();
		driver->clearBuffers(CRGBA(40, 44, 52));
		editorUI->draw();
		driver->swapBuffers();

		IDriver *drv = static_cast<CDriverUser *>(driver)->getDriver();
		if (drv)
		{
			CBitmap btm;
			drv->getBuffer(btm);
			COFile fs;
			if (!fs.open(screenshotPath))
			{
				fprintf(stderr, "ERROR: cannot write %s\n", screenshotPath.c_str());
				startupHideAllScreens();
				s_Sess.Active = false;
				return StartupError;
			}
			btm.writeTGA(fs, 24);
			printf("OK startup-screenshot: %ux%u -> %s\n", btm.getWidth(), btm.getHeight(), screenshotPath.c_str());
		}

		startupHideAllScreens();
		s_Sess.Active = false;
		return StartupScreenshotDone;
	}

	CStartupCloseListener closeListener;
	driver->EventServer.addListener(EventDestroyWindowId, &closeListener);
	driver->EventServer.addListener(EventCloseWindowId, &closeListener);

	EStartupResult result = StartupQuit;
	do
	{
		driver->EventServer.pump();
		if (driver->AsyncListener.isKeyPushed(KeyESCAPE))
			s_Sess.Quit = true;
		if (!closeListener.Active)
			s_Sess.Quit = true;

		editorUI->update();
		driver->clearBuffers(CRGBA(40, 44, 52));
		editorUI->draw();
		driver->swapBuffers();

		if (s_Sess.OpenZone)
		{
			if (s_Sess.SelectedWorld >= 0 && s_Sess.SelectedWorld < (int)worlds.size()
			    && s_Sess.SelectedZone >= 0 && s_Sess.SelectedZone < (int)s_Sess.Zones.size())
			{
				selection.World = worlds[s_Sess.SelectedWorld];
				selection.Zone = s_Sess.Zones[s_Sess.SelectedZone];
				// Multi-select editable set (M6b); empty PendingSelect still has one via L-click path
				selection.EditableZones.clear();
				if (!s_Sess.PendingSelect.empty())
				{
					for (std::set<int>::const_iterator it = s_Sess.PendingSelect.begin();
					     it != s_Sess.PendingSelect.end(); ++it)
					{
						if (*it >= 0 && *it < (int)s_Sess.Zones.size())
							selection.EditableZones.push_back(s_Sess.Zones[*it]);
					}
					// Primary = first pending (lowest index) for title/compat
					if (!selection.EditableZones.empty())
						selection.Zone = selection.EditableZones[0];
				}
				else
					selection.EditableZones.push_back(selection.Zone);
				// Ecosystem layout for self-instances; continents always 1x1
				if (selection.World.Kind == ZPWS::Ecosystem)
					selection.InstanceLayout = s_Sess.InstanceLayout;
				else
					selection.InstanceLayout = "1x1";
				result = StartupOpenZone;
				break;
			}
			s_Sess.OpenZone = false;
		}
	}
	while (!s_Sess.Quit && driver->isActive());

	driver->EventServer.removeListener(EventDestroyWindowId, &closeListener);
	driver->EventServer.removeListener(EventCloseWindowId, &closeListener);
	startupHideAllScreens();
	s_Sess.Active = false;
	s_Sess.Worlds = NULL;
	return result;
}

// ---------------------------------------------------------------------------------------------
// Session board hub (M11a)

void setSessionBoardBridge(SSessionBoardBridge *bridge)
{
	s_SessionBridge = bridge;
}

SSessionBoardBridge *getSessionBoardBridge()
{
	return s_SessionBridge;
}

static void openCellActionPopup(const std::string &basename)
{
	s_Sess.PendingActionBasename = basename;
	// Enable/disable buttons based on state
	const bool dirty = s_SessionBridge && s_SessionBridge->isDirty
		&& s_SessionBridge->isDirty(basename);
	const bool editable = s_SessionBridge && s_SessionBridge->isEditable
		&& s_SessionBridge->isEditable(basename);
	if (CViewText *t = findText("ui:zp:cell_action:content:title"))
		t->setHardText(basename + (dirty ? " *" : ""));
	if (CViewText *t = findText("ui:zp:cell_action:content:status"))
	{
		if (editable && dirty)
			t->setHardText("open-editable, dirty");
		else if (editable)
			t->setHardText("open-editable");
		else
			t->setHardText("open read-only");
	}
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:cell_action:content:btn_save")))
		btn->setFrozen(!dirty || !editable);
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:cell_action:content:btn_toggle")))
		btn->setHardText(editable ? "Make read-only" : "Make editable");
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:cell_action");
}

static void openCloseConfirmModal(const std::string &basename, const std::string &purpose)
{
	s_Sess.PendingActionBasename = basename;
	if (CViewText *t = findText("ui:zp:close_confirm:content:title"))
		t->setHardText(purpose.empty() ? "Close dirty zone?" : purpose);
	if (CViewText *t = findText("ui:zp:close_confirm:content:status"))
		t->setHardText(basename + " has unsaved paint changes");
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:close_confirm");
}

void forceShowCloseConfirmForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty() && !s_Sess.Zones.empty())
		b = s_Sess.Zones[0].Basename;
	if (b.empty())
		b = "zone";
	openCloseConfirmModal(b, "Close dirty zone?");
}

void forceShowCellActionForShot(const std::string &basename)
{
	std::string b = basename;
	if (b.empty() && !s_Sess.Zones.empty())
		b = s_Sess.Zones[0].Basename;
	if (b.empty())
		b = "zone";
	openCellActionPopup(b);
}

void refreshSessionBoardStates()
{
	if (!s_SessionBoardVisible || !s_Sess.SessionMode)
		return;
	if (!s_SessionBridge || !s_SessionBridge->getCellState)
		return;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		int r = 0, c = 0;
		if (!ZPWS::parseContinentZoneName(s_Sess.Zones[i].Basename, r, c))
			continue;
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "ui:zp:zone_browser:content:board_host:board:gc%d_%d", r, c);
		if (CInterfaceGroup *cell = findGroup(idbuf))
		{
			ESessionCellState st = CellClosed;
			s_SessionBridge->getCellState(s_Sess.Zones[i].Basename, st);
			applySessionCellState(cell, s_Sess.Zones[i].Basename, st);
		}
	}
	refreshBoardSelectionUI();
}

void setSessionBoardVisible(bool visible)
{
	if (!s_SessionBridge || !s_SessionBridge->World)
	{
		// No continent session bridge: no-op
		if (visible)
			fprintf(stderr, "session board: no continent bridge (ecosystems use single-file flow)\n");
		s_SessionBoardVisible = false;
		return;
	}
	if (s_SessionBridge->World->Kind != ZPWS::Continent)
	{
		if (visible)
			fprintf(stderr, "session board: continent worlds only this milestone\n");
		s_SessionBoardVisible = false;
		return;
	}

	s_SessionBoardVisible = visible;
	s_Sess.SessionMode = visible;
	if (visible)
	{
		// Populate board for the session world (reuses Screen B chrome)
		s_Sess.Active = true; // allow board AHs while viewer runs
		s_Sess.Worlds = NULL; // not used in session mode
		s_Sess.SelectedWorld = 0;
		// Fake a one-world list via Zones from the bridge world
		ZPWS::listZones(*s_SessionBridge->World, s_Sess.Zones);
		if (CViewText *t = findText("ui:zp:zone_browser:content:world_title"))
			t->setHardText(s_SessionBridge->World->WorldName + "  (session board)");
		if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
			t->setHardText("working set — O / BACK TO PAINTING returns");
		setLayoutSelectorVisible(false);
		populateContinentGrid(*s_SessionBridge->World);
		// Show zone_browser; keep painter visible underneath
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser"))
			el->setActive(true);
		if (CGroupContainer *gc = dynamic_cast<CGroupContainer *>(
		        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser")))
			gc->setActive(true);
		refreshSessionBoardStates();
		refreshBoardSelectionUI();
		printf("session board: open (%u zones)\n", (uint)s_Sess.Zones.size());
	}
	else
	{
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser"))
			el->setActive(false);
		// Leave modal if any
		CWidgetManager::getInstance()->disableModalWindow();
		s_Sess.SessionMode = false;
		// Keep s_Sess.Active false so startup AHs do not fire mid-viewer except via SessionMode checks
		s_Sess.Active = false;
		s_Sess.PendingActionBasename.clear();
		printf("session board: closed (back to painting)\n");
	}
}

void toggleSessionBoard()
{
	setSessionBoardVisible(!s_SessionBoardVisible);
}

bool isSessionBoardVisible()
{
	return s_SessionBoardVisible;
}

// Cell-action popup handlers
class CAHZpCellClose : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->closeZone)
			return;
		const bool dirty = s_SessionBridge->isDirty && s_SessionBridge->isDirty(base);
		const bool editable = s_SessionBridge->isEditable && s_SessionBridge->isEditable(base);
		if (dirty && editable)
		{
			openCloseConfirmModal(base, "Close dirty zone?");
			return;
		}
		std::string err;
		if (!s_SessionBridge->closeZone(base, false, false, err))
			fprintf(stderr, "session board close '%s': %s\n", base.c_str(), err.c_str());
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellClose, "zp_cell_close");

class CAHZpCellSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->saveZone)
			return;
		std::string err;
		if (!s_SessionBridge->saveZone(base, err))
			fprintf(stderr, "session board save '%s': %s\n", base.c_str(), err.c_str());
		else
			printf("session board: saved '%s'\n", base.c_str());
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellSave, "zp_cell_save");

class CAHZpCellToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge || !s_SessionBridge->toggleEditable)
			return;
		const bool dirty = s_SessionBridge->isDirty && s_SessionBridge->isDirty(base);
		const bool editable = s_SessionBridge->isEditable && s_SessionBridge->isEditable(base);
		if (editable && dirty)
		{
			// Confirm: save first or discard before demoting to RO
			s_Sess.PendingActionBasename = base;
			if (CViewText *t = findText("ui:zp:close_confirm:content:title"))
				t->setHardText("Make read-only?");
			if (CViewText *t = findText("ui:zp:close_confirm:content:status"))
				t->setHardText(base + " is dirty — save before making read-only?");
			// Reuse close_confirm; btn labels still say Save first / Close without saving
			// "Close without saving" path demotes without save via forceDiscard
			CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:close_confirm");
			// Tag purpose via status prefix
			s_Sess.StatusMsg = "toggle_ro";
			return;
		}
		std::string err;
		if (!s_SessionBridge->toggleEditable(base, false, false, err))
			fprintf(stderr, "session board toggle '%s': %s\n", base.c_str(), err.c_str());
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellToggle, "zp_cell_toggle");

class CAHZpCellActionCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		s_Sess.PendingActionBasename.clear();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCellActionCancel, "zp_cell_action_cancel");

// Close-confirm: Save first / Close without saving / Cancel
class CAHZpCloseConfirmSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge)
			return;
		const bool isToggle = (s_Sess.StatusMsg == "toggle_ro");
		s_Sess.StatusMsg.clear();
		std::string err;
		if (isToggle)
		{
			if (s_SessionBridge->toggleEditable
			    && !s_SessionBridge->toggleEditable(base, /*saveFirst=*/true, false, err))
				fprintf(stderr, "session board toggle(save) '%s': %s\n", base.c_str(), err.c_str());
		}
		else if (s_SessionBridge->closeZone
		         && !s_SessionBridge->closeZone(base, /*saveFirst=*/true, false, err))
		{
			fprintf(stderr, "session board close(save) '%s': %s\n", base.c_str(), err.c_str());
		}
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmSave, "zp_close_confirm_save");

class CAHZpCloseConfirmDiscard : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		const std::string base = s_Sess.PendingActionBasename;
		if (base.empty() || !s_SessionBridge)
			return;
		const bool isToggle = (s_Sess.StatusMsg == "toggle_ro");
		s_Sess.StatusMsg.clear();
		std::string err;
		if (isToggle)
		{
			if (s_SessionBridge->toggleEditable
			    && !s_SessionBridge->toggleEditable(base, false, /*forceDiscard=*/true, err))
				fprintf(stderr, "session board toggle(discard) '%s': %s\n", base.c_str(), err.c_str());
		}
		else if (s_SessionBridge->closeZone
		         && !s_SessionBridge->closeZone(base, false, /*forceDiscard=*/true, err))
		{
			fprintf(stderr, "session board close(discard) '%s': %s\n", base.c_str(), err.c_str());
		}
		refreshSessionBoardStates();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmDiscard, "zp_close_confirm_discard");

class CAHZpCloseConfirmCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		s_Sess.StatusMsg.clear();
		s_Sess.PendingActionBasename.clear();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCloseConfirmCancel, "zp_close_confirm_cancel");

} // namespace ZPUI

/* end of file */
