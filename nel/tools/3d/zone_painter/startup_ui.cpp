/**
 * \file startup_ui.cpp
 * \brief NeL-GUI startup screens for zone_painter (world / zone / folder select + pump)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * World list, zone browser (ecosystem list / continent minesweeper board / eco card-grid
 * grid), folder browser, and the runStartupFlow pump. SStartupSession + NLGUI helpers
 * are shared with session_board_ui.cpp through startup_ui_internal.h.
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
#include "startup_ui_internal.h"
#include "script_api.h"
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
#include <nel/gui/group_editbox.h>
#include <nel/gui/group_list.h>
#include <nel/gui/group_menu.h>
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
// Session state (definitions; declarations live in startup_ui_internal.h). Both TUs share this.

SStartupSession s_Sess;
SSessionBoardBridge *s_SessionBridge = nullptr;

CInterfaceGroup *findGroup(const char *id)
{
	return dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(id));
}

CViewText *findText(const char *id)
{
	return dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(id));
}

CViewTextMenu *findMenuLine(const char *id)
{
	return dynamic_cast<CViewTextMenu *>(CWidgetManager::getInstance()->getElementFromId(id));
}

CGroupList *findList(const char *id)
{
	return dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId(id));
}

CGroupContainer *findContainer(const char *id)
{
	return dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(id));
}

CGroupEditBox *findEditBox(const char *id)
{
	return dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId(id));
}

void setContainerActive(const char *id, bool active)
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

void setStatus(const char *windowStatusId, const std::string &msg)
{
	s_Sess.StatusMsg = msg;
	if (CViewText *t = findText(windowStatusId))
		t->setHardText(msg);
}

void clearList(const char *listId)
{
	if (CGroupList *list = findList(listId))
		list->deleteAllChildren();
}

CInterfaceGroup *spawnRow(const char *templateName, const char *parentListId,
                          const std::vector<std::pair<std::string, std::string> > &params)
{
	CGroupList *list = findList(parentListId);
	if (!list)
	{
		fprintf(stderr, "WARNING: startup UI: list '%s' not found\n", parentListId);
		return nullptr;
	}
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *row = nullptr;
	if (!params.empty())
		row = parser->createGroupInstance(templateName, list->getId(), &params[0], (uint)params.size());
	else
		row = parser->createGroupInstance(templateName, list->getId(), (const std::pair<std::string, std::string> *)nullptr, 0);
	if (!row)
	{
		fprintf(stderr, "WARNING: startup UI: createGroupInstance(%s) failed under %s\n",
		        templateName, parentListId);
		return nullptr;
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
			hp.push_back(std::make_pair(std::string("title"), std::string("Ecosystems")));
			spawnRow("zp_list_header", "ui:zp:world_select:content:list_scroll:text_list", hp);
		}
		if (w.Kind == ZPWS::Continent && !wroteContHdr)
		{
			wroteContHdr = true;
			std::vector<std::pair<std::string, std::string> > hp;
			hp.push_back(std::make_pair(std::string("id"), std::string("hdr_cont")));
			hp.push_back(std::make_pair(std::string("title"), std::string("Continents")));
			spawnRow("zp_list_header", "ui:zp:world_select:content:list_scroll:text_list", hp);
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
 * Resolve browser thumbnail texture name for a zone.
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
CInterfaceGroup *spawnUnder(CInterfaceGroup *parent, const char *templateName,
                            const std::vector<std::pair<std::string, std::string> > &params,
                            sint32 x, sint32 y, sint32 w, sint32 h)
{
	if (!parent)
		return nullptr;
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *g = nullptr;
	if (!params.empty())
		g = parser->createGroupInstance(templateName, parent->getId(), &params[0], (uint)params.size());
	else
		g = parser->createGroupInstance(templateName, parent->getId(),
		                                (const std::pair<std::string, std::string> *)nullptr, 0);
	if (!g)
		return nullptr;
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
void setZoneBrowserMode(bool continentBoard)
{
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:list_scroll"))
		el->setActive(!continentBoard);
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_host"))
		el->setActive(continentBoard);
	// Multi-select chrome is continent-board only; hidden in session hub
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_legend"))
		el->setActive(continentBoard);
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:btn_open_sel"))
		el->setActive(continentBoard && !s_Sess.SessionMode);
	// Display-mode toggle: eco Screen B only. Any board (continent or session hub)
	// hides it; populateZoneList's eco branch is the sole activation site. Without this,
	// the toggle stayed live over the session board and a click ran populateZoneList,
	// which clears the board cells then early-returns on Worlds == NULL (empty hub).
	if (continentBoard || s_Sess.SessionMode)
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
		        "ui:zp:zone_browser:content:btn_view_large"))
			el->setActive(false);
	// Session hub chrome: BACK TO PAINTING stays available in session mode even when
	// the continent board falls back to a flat list (unparseable zone names).
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:btn_back_paint"))
		el->setActive(s_Sess.SessionMode);
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:zone_browser:content:btn_back")))
	{
		// Startup: Back to world list. Session: hide (use BACK TO PAINTING).
		btn->setActive(!s_Sess.SessionMode);
	}
}

/** Update Open-selection button label/frozen + optional status. */
void refreshBoardSelectionUI()
{
	if (s_Sess.SessionMode)
	{
		// Session hub legend (continent working set / ecosystem scratch)
		if (CViewText *t = findText("ui:zp:zone_browser:content:board_legend"))
		{
			const bool eco = s_SessionBridge && s_SessionBridge->World
			                 && s_SessionBridge->World->Kind == ZPWS::Ecosystem;
			if (eco)
				t->setHardText(
				    "Scratch: file=Close/Save/Save as/Toggle · empty=place (block origin) · "
				    "inst=R CW/CCW/Mirror/Remove · glyph R90/M · O/BOARD back");
			else
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
			t->setHardText("L-click open · R-click select · fringe = edge-empty");
		else
			t->setHardText(NLMISC::toString(
			    "L-click open · R-click select · fringe = edge-empty  |  %u selected", n));
	}
}

/**
 * Explorer-style board multi-select fill + session live-state fills.
 * Same tone as palette cells: NeL CGroupTree col_select default (255 128 128 128).
 * Applied as blank.tga button face color (not setPushed brick chrome).
 *
 * Session states:
 *   closed: transparent fill (default used-cell look)
 *   open-editable: selection-fill (255 128 128 128)
 *   open-read-only: dimmer cool tint (80 100 140 110)
 *   dirty: editable fill + '*' on the label
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

/**
 * Parse scratch basenames (open files are F: cells; no H: tokens):
 *   "I:ox,oy"                 instance ORIGIN cell (label lives here)
 *   "I:ox,oy@cx,cy"           non-origin cell of an instance block (tint only)
 *   "E:cx,cy"                 empty
 *   "L:cx,cy"                 locked empty
 *   "F:ox,oy:name" / "F:ox,oy@cx,cy:name"  open-file block
 *
 * For I: forms, (cx,cy) out-params are the actual board cell (from @ tail when present,
 * else the origin). Use isInstanceOrigin to know if the label should show.
 */
bool parseScratchBasename(const std::string &base, char &kind, int &cx, int &cy,
                          bool *isInstanceOrigin)
{
	kind = 0;
	cx = cy = 0;
	if (isInstanceOrigin) *isInstanceOrigin = false;
	// First-opened file is an F: open-file cell like any other
	if (base.size() >= 4 && (base[0] == 'E' || base[0] == 'L') && base[1] == ':')
	{
		kind = base[0];
		std::string rest = base.substr(2);
		std::string::size_type comma = rest.find(',');
		if (comma == std::string::npos) return false;
		if (!NLMISC::fromString(rest.substr(0, comma), cx)
		    || !NLMISC::fromString(rest.substr(comma + 1), cy))
			return false;
		return true;
	}
	if (base.size() >= 4 && base[0] == 'I' && base[1] == ':')
	{
		kind = 'I';
		std::string rest = base.substr(2);
		// Optional @cx,cy for non-origin cells of a multi-cell block
		std::string::size_type at = rest.find('@');
		std::string originPart = at == std::string::npos ? rest : rest.substr(0, at);
		std::string::size_type comma = originPart.find(',');
		if (comma == std::string::npos) return false;
		int ox = 0, oy = 0;
		if (!NLMISC::fromString(originPart.substr(0, comma), ox)
		    || !NLMISC::fromString(originPart.substr(comma + 1), oy))
			return false;
		if (at == std::string::npos)
		{
			cx = ox; cy = oy;
			if (isInstanceOrigin) *isInstanceOrigin = true;
		}
		else
		{
			std::string cellPart = rest.substr(at + 1);
			std::string::size_type c2 = cellPart.find(',');
			if (c2 == std::string::npos) return false;
			if (!NLMISC::fromString(cellPart.substr(0, c2), cx)
			    || !NLMISC::fromString(cellPart.substr(c2 + 1), cy))
				return false;
			if (isInstanceOrigin) *isInstanceOrigin = false;
		}
		return true;
	}
	if (base.size() >= 4 && base[0] == 'F' && base[1] == ':')
	{
		// Open file: F:ox,oy:name (origin) / F:ox,oy@cx,cy:name (block cell).
		// cx,cy return the ORIGIN cell (per-file ops are origin/basename addressed);
		// isInstanceOrigin doubles as the origin-cell flag.
		kind = 'F';
		std::string rest = base.substr(2);
		std::string::size_type at = rest.find('@');
		std::string::size_type colon = rest.find(':');
		if (colon == std::string::npos) return false;
		std::string originPart = rest.substr(0, at == std::string::npos ? colon
		                                                                : std::min(at, colon));
		std::string::size_type comma = originPart.find(',');
		if (comma == std::string::npos) return false;
		if (!NLMISC::fromString(originPart.substr(0, comma), cx)
		    || !NLMISC::fromString(originPart.substr(comma + 1), cy))
			return false;
		if (isInstanceOrigin) *isInstanceOrigin = (at == std::string::npos || at > colon);
		return true;
	}
	return false;
}

/** Brick basename from an F:ox,oy[:@cx,cy]:name coded cell (empty when not F:). */
std::string scratchEditableName(const std::string &base)
{
	if (base.size() < 4 || base[0] != 'F' || base[1] != ':')
		return std::string();
	std::string::size_type lastColon = base.rfind(':');
	if (lastColon == std::string::npos || lastColon + 1 >= base.size() || lastColon < 2)
		return std::string();
	return base.substr(lastColon + 1);
}

/**
 * Strip redundant ligo family prefixes so board stamps lead with the distinctive part
 * (material-bassin → bassin, zonematerial-converted-200_dz → converted-200_dz).
 * Browser LIST rows keep the full name (they have width).
 */
std::string stripLigoFamilyPrefix(const std::string &name)
{
	static const char *kPrefixes[] = {
		"zonetransition-",
		"zonematerial-",
		"zonespecial-",
		"transition-",
		"material-",
		nullptr
	};
	for (int i = 0; kPrefixes[i]; ++i)
	{
		const size_t n = strlen(kPrefixes[i]);
		if (name.size() > n && NLMISC::toLowerAscii(name.substr(0, n)) == kPrefixes[i])
			return name.substr(n);
	}
	return name;
}

/** Configure board cell text for multi-line wrap (2-3 lines, ellipsis via multi_max_line). */
static void configureBoardLabel(CViewText *t)
{
	if (!t) return;
	t->setMultiLine(true);
	t->setMultiLineMaxWOnly(true);
	t->setLineMaxW(48, true);
	t->setMultiLineSpace(0);
	t->setMultiMaxLine(3);
	t->setOverflowText("…");
	t->setFontSize(9);
}

/** Apply board label text with prefix strip + multi-line wrap. */
static void setBoardCellLabel(CViewText *t, const std::string &raw)
{
	if (!t) return;
	configureBoardLabel(t);
	t->setHardText(stripLigoFamilyPrefix(raw));
}

/** Apply live state fill + label (dirty * / R90 / M glyphs; prefix strip). */
void applySessionCellState(CInterfaceGroup *cell, const std::string &basename,
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
	case CellScratchInstance:
		// Distinct teal tint for instances
		setBoardCellFillRGBA(cell, NLMISC::CRGBA(40, 90, 100, 200), NLMISC::CRGBA(50, 110, 120, 220));
		break;
	case CellScratchEmpty:
		// UNLOCKED slot (edge-adjacent to occupied, or hint-named): clearly
		// lighter than the backdrop and the locked cells
		setBoardCellFillRGBA(cell, NLMISC::CRGBA(96, 108, 122, 255), NLMISC::CRGBA(126, 140, 156, 255));
		break;
	case CellScratchLocked:
		// Locked cell: dim/inert (no open menu; still a drag-drop target)
		setBoardCellFillRGBA(cell, NLMISC::CRGBA(46, 49, 54, 255), NLMISC::CRGBA(56, 60, 66, 255));
		break;
	case CellScratchContext:
		// Read-only context brick: same dim RO tint as continent RO
		setBoardCellFillRGBA(cell, kBoardCellReadOnly, kBoardCellReadOnlyHover);
		break;
	case CellClosed:
	default:
		setBoardCellFillRGBA(cell, kBoardCellSelNone, kBoardCellSelHover);
		break;
	}
	if (CViewText *t = dynamic_cast<CViewText *>(cell->getView("t")))
	{
		char sk = 0;
		int cx = 0, cy = 0;
		if (basename.size() >= 4 && basename[0] == 'F' && basename[1] == ':')
		{
			// Open-file block: label only the origin cell with the brick name
			bool isOrigin = false;
			parseScratchBasename(basename, sk, cx, cy, &isOrigin);
			if (!isOrigin)
				t->setHardText("");
			else
			{
				std::string lab = scratchEditableName(basename);
				if (st == CellDirtyEditable) lab += " *";
				else if (st == CellOpenReadOnly) lab += " (ro)";
				setBoardCellLabel(t, lab);
			}
		}
		else if (st == CellScratchInstance)
		{
			// Label only on the block origin cell (basename "I:ox,oy"); non-origin
			// cells use "I:ox,oy@cx,cy" and stay tint-only.
			bool isOrigin = false;
			if (!parseScratchBasename(basename, sk, cx, cy, &isOrigin) || !isOrigin)
				t->setHardText("");
			else
			{
				int ox = cx, oy = cy;
				uint rot = 0;
				bool mir = false;
				if (s_SessionBridge && s_SessionBridge->scratchGetInstanceOrigin)
					s_SessionBridge->scratchGetInstanceOrigin(cx, cy, ox, oy, rot, mir);
				else if (s_SessionBridge && s_SessionBridge->scratchGetInstance)
					s_SessionBridge->scratchGetInstance(cx, cy, rot, mir);
				std::string lab;
				// Instance labels lead with the source brick's short name
				if (s_SessionBridge && s_SessionBridge->scratchGetInstanceSource)
				{
					std::string srcName;
					if (s_SessionBridge->scratchGetInstanceSource(ox, oy, srcName) && !srcName.empty())
						lab = stripLigoFamilyPrefix(srcName) + " ";
				}
				lab += NLMISC::toString("%d,%d", ox, oy);
				if (rot) lab += NLMISC::toString(" R%u", rot * 90);
				if (mir) lab += " M";
				configureBoardLabel(t);
				t->setHardText(lab);
			}
		}
		else if (st == CellScratchLocked)
			t->setHardText("");
		else if (st == CellScratchEmpty)
		{
			// Unlocked well named by a saved hint shows the neighbor's short name
			// (dim) so the remembered layout is visible before opening anything
			std::string hintName;
			int hx = 0, hy = 0;
			if (parseScratchBasename(basename, sk, hx, hy) && sk == 'E'
			    && s_SessionBridge && s_SessionBridge->scratchGetHintAt
			    && s_SessionBridge->scratchGetHintAt(hx, hy, hintName))
			{
				setBoardCellLabel(t, stripLigoFamilyPrefix(hintName));
				t->setColor(NLMISC::CRGBA(180, 190, 200, 150));
			}
			else
			{
				t->setHardText("");
				t->setColor(NLMISC::CRGBA(255, 255, 255, 255));
			}
		}
		else if (st == CellScratchContext)
		{
			// Basename form C:cx,cy:brickname; label the brick (short)
			std::string lab;
			if (basename.size() >= 4 && basename[0] == 'C')
			{
				std::string::size_type lastColon = basename.rfind(':');
				if (lastColon != std::string::npos && lastColon + 1 < basename.size())
					lab = basename.substr(lastColon + 1);
			}
			if (lab.empty() && s_SessionBridge && s_SessionBridge->scratchGetContext)
			{
				char sk = 0;
				int cx = 0, cy = 0;
				if (parseScratchBasename(basename, sk, cx, cy) || true)
				{
					// try C: parse
					if (basename.size() > 2 && basename[0] == 'C' && basename[1] == ':')
					{
						std::string rest = basename.substr(2);
						std::string::size_type comma = rest.find(',');
						std::string::size_type colon = rest.find(':', comma == std::string::npos ? 0 : comma);
						if (comma != std::string::npos && colon != std::string::npos)
						{
							NLMISC::fromString(rest.substr(0, comma), cx);
							NLMISC::fromString(rest.substr(comma + 1, colon - comma - 1), cy);
						}
					}
					s_SessionBridge->scratchGetContext(cx, cy, lab);
				}
			}
			// Transform glyphs (same R90/M vocabulary as instances)
			{
				char csk = 0;
				int ccx = 0, ccy = 0;
				uint crot = 0;
				bool cmir = false;
				std::string rest = basename.size() > 2 ? basename.substr(2) : std::string();
				std::string::size_type comma = rest.find(',');
				std::string::size_type colon = rest.find(':', comma == std::string::npos ? 0 : comma);
				if (comma != std::string::npos && colon != std::string::npos
				    && NLMISC::fromString(rest.substr(0, comma), ccx)
				    && NLMISC::fromString(rest.substr(comma + 1, colon - comma - 1), ccy)
				    && s_SessionBridge && s_SessionBridge->scratchGetContextTransform
				    && s_SessionBridge->scratchGetContextTransform(ccx, ccy, crot, cmir))
				{
					if (crot) lab += NLMISC::toString(" R%u", crot * 90);
					if (cmir) lab += " M";
				}
				(void)csk;
			}
			setBoardCellLabel(t, lab.empty() ? std::string("ctx") : lab);
		}
		else if (st == CellDirtyEditable)
			setBoardCellLabel(t, basename + " *");
		else
		{
			int r = 0, c = 0;
			if (ZPWS::parseContinentZoneName(basename, r, c))
			{
				// Short grid form is already distinctive; still run through multi-line setup
				configureBoardLabel(t);
				t->setHardText(ZPWS::continentZoneName(r, c));
			}
			else
				setBoardCellLabel(t, basename);
		}
	}
}

/** Set board cell flat selection fill for one zone index (if the cell exists). */
void setBoardCellSelected(int zoneIdx, bool selected)
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

void clearBoard()
{
	if (CInterfaceGroup *board = findGroup("ui:zp:zone_browser:content:board_host:board"))
	{
		board->clearGroups();
		board->setOfsX(0);
		board->setOfsY(0);
	}
}

/**
 * Continent Screen B: minesweeper-style square cell board.
 * Cell set = used zones + empty edge-adjacent (4-neighbor) fringe; no diagonals.
 * Diagonal contact alone does not decide a ligo border type, so those cells suggested
 * undecidable placements. Neighbor auto-load for open zones stays the 8-ring (weld).
 * Dual-axis scroll when the board exceeds the host viewport.
 */
void populateContinentGrid(const ZPWS::SWorldEntry &world)
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

	// Fringe = empty edge-adjacent (N/E/S/W) neighbors only; no diagonals
	static const int kEdgeDr[4] = { -1, 0, 1, 0 };
	static const int kEdgeDc[4] = { 0, 1, 0, -1 };
	std::set<std::pair<int, int> > fringe;
	for (std::map<std::pair<int, int>, int>::const_iterator it = used.begin(); it != used.end(); ++it)
	{
		const int r = it->first.first;
		const int c = it->first.second;
		for (int k = 0; k < 4; ++k)
		{
			const int nr = r + kEdgeDr[k];
			const int nc = c + kEdgeDc[k];
			const std::pair<int, int> key(nr, nc);
			if (used.find(key) == used.end())
				fringe.insert(key);
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
		// Cell title: short grid form (4_AC / 193_EC); else strip ligo family prefix
		{
			std::string title = ZPWS::continentZoneName(r, c);
			if (title.empty())
				title = stripLigoFamilyPrefix(z.Basename);
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
			// Activate 9-slice well only when a real thumb is bound
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
			// Session board: live open/dirty state. Startup multi-select fill otherwise.
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

/** Unused layout selector chrome: always hide; list/board sit under world_sub. */
void setLayoutSelectorVisible(bool /*visible*/)
{
	static const char *ids[] = {
		"ui:zp:zone_browser:content:layout_label",
		"ui:zp:zone_browser:content:layout_1x1",
		"ui:zp:zone_browser:content:layout_2x1",
		"ui:zp:zone_browser:content:layout_1x2",
		"ui:zp:zone_browser:content:layout_2x2",
		"ui:zp:zone_browser:content:layout_3x3",
		nullptr
	};
	for (int i = 0; ids[i]; ++i)
	{
		if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(ids[i]))
			el->setActive(false);
	}
	if (CInterfaceElement *list = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:list_scroll"))
		list->setY(-8);
	if (CInterfaceElement *board = CWidgetManager::getInstance()->getElementFromId(
	        "ui:zp:zone_browser:content:board_host"))
		board->setY(-8);
}

static void syncLayoutRadios()
{
	// no-op (layout selector unused)
}

// Zone browser display mode: false = detail-row list, true = large-thumbnail
// grid (the tileset-palette idiom). Remembered in startup.cfg (ZoneBrowserLarge).
static bool s_ZoneListLarge = false;

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

	setLayoutSelectorVisible(false);

	ZPWS::listZones(world, s_Sess.Zones);

	// Continents: minesweeper-style board. Ecosystems keep the grouped list.
	if (world.Kind == ZPWS::Continent)
	{
		if (CCtrlBaseButton *tog = dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()
		        ->getElementFromId("ui:zp:zone_browser:content:btn_view_large")))
			tog->setActive(false);
		populateContinentGrid(world);
		return;
	}

	setZoneBrowserMode(false);
	// Display-mode toggle lives on eco Screen B only
	if (CCtrlBaseButton *tog = dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()
	        ->getElementFromId("ui:zp:zone_browser:content:btn_view_large")))
	{
		tog->setActive(true);
		tog->setPushed(s_ZoneListLarge);
	}

	static const char *kList = "ui:zp:zone_browser:content:list_scroll:text_list";
	// Large mode: one auto-wrapping zp_flow SECTION per zone group. The flow
	// group reflows its items from its own width inside updateCoords (the Ryzom client
	// inventory mechanism), so resizes re-wrap with no repopulation and no width watch.
	const int kCardW = 178, kCardH = 106;
	CInterfaceGroup *flowSec = nullptr;
	uint flowSecCount = 0;

	std::string lastGroup;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		ZPWS::SZoneEntry &z = s_Sess.Zones[i];
		if (z.Group != lastGroup)
		{
			lastGroup = z.Group;
			std::vector<std::pair<std::string, std::string> > hp;
			hp.push_back(std::make_pair(std::string("id"), std::string("zg_") + lastGroup));
			hp.push_back(std::make_pair(std::string("title"), lastGroup));
			spawnRow("zp_list_header", kList, hp);
			flowSec = nullptr; // group headers start a new flow section
		}

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32];
		snprintf(idbuf, sizeof(idbuf), "z%u", (uint)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		char idxbuf[32];
		snprintf(idxbuf, sizeof(idxbuf), "%d", (int)i);
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		std::string thumbTex = thumbTextureName(z);
		p.push_back(std::make_pair(std::string("thumb"), thumbTex.empty() ? std::string("w_box_blank.tga") : thumbTex));
		const bool hasThumb = !thumbTex.empty();

		if (s_ZoneListLarge)
		{
			// Large-icon card into the current section's flow; the flow assigns
			// the grid slot, positions here are placeholders.
			p.push_back(std::make_pair(std::string("title"), stripLigoFamilyPrefix(z.Basename)));
			if (!flowSec)
			{
				std::vector<std::pair<std::string, std::string> > lp;
				lp.push_back(std::make_pair(std::string("id"),
				                            NLMISC::toString("zflow%u", flowSecCount++)));
				flowSec = spawnRow("zp_zone_flow_sec", kList, lp);
				if (flowSec)
				{
					// spawnRow force-sets W on rows declaring <=0, but with sizeref="w"
					// the stored W is an OFFSET from the size parent; restore it. And
					// the size parent must be the LIST explicitly: CGroupList re-chains
					// each child's pos parent to the PREVIOUS element and sizeref follows
					// the pos parent, so without this the flow sizes to the header row
					// above it (template sizeparent can't express it; the parent is
					// NULL at template-parse time).
					flowSec->setW(-4);
					if (CGroupList *gl = findList(kList))
						flowSec->setParentSize(gl);
					// Stale-build tripwire: with flow_group.cpp missing from the link
					// (FILE(GLOB) + skipped reconfigure) the parser silently falls back
					// to a plain interface_group and every card stacks at slot 0,0.
					if (flowSec->getClassName() != "CZPGroupFlow")
						nlwarning("zone browser: zp_flow factory missing; grid will not wrap (stale build?)");
				}
			}
			if (!flowSec)
				continue;
			if (CInterfaceGroup *card = spawnUnder(flowSec, "zp_zone_card", p,
			                                       0, 0, kCardW, kCardH))
			{
				if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(card->getView("thumb")))
					thumb->setActive(hasThumb);
				if (CInterfaceGroup *fr = card->getGroup("thumb_frame"))
					fr->setActive(hasThumb);
			}
			continue;
		}

		p.push_back(std::make_pair(std::string("title"), z.Basename));
		if (CInterfaceGroup *row = spawnRow("zp_zone_row", kList, p))
		{
			if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(row->getView("thumb")))
				thumb->setActive(hasThumb);
			// 9-slice well only when a real thumb is bound (no empty dark square)
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

/** Flip list/grid display mode, persist, repopulate. */
class CAHZpZoneViewToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		// Screen B only: in the session hub Worlds is NULL and populateZoneList would
		// clear the board then early-return, wiping the working-set display.
		if (s_Sess.SessionMode || !s_Sess.Worlds) return;
		s_ZoneListLarge = !s_ZoneListLarge;
		{
			ZPWS::SStartupCfg cfg;
			ZPWS::loadStartupCfg(cfg); // keep the other remembered fields
			cfg.ZoneBrowserLarge = s_ZoneListLarge;
			ZPWS::saveStartupCfg(cfg);
		}
		populateZoneList();
	}
};
REGISTER_ACTION_HANDLER(CAHZpZoneViewToggle, "zp_zone_view_toggle");

/** Tag shown at the right of a folder row: a cheap fingerprint (a handful of
 *  isDirectory/fileExists checks, the same call scanChildrenForWorkspaces already makes for
 *  every candidate during startup discovery) so browsing shows which folders actually lead
 *  somewhere instead of it being a blind guess which subdir to open. */
static std::string folderWorkspaceTag(const std::string &dirPath)
{
	std::vector<ZPWS::SWorldEntry> found;
	ZPWS::fingerprintWorkspace(dirPath, found);
	if (found.empty())
		return std::string();
	uint eco = 0, cont = 0;
	for (size_t i = 0; i < found.size(); ++i)
	{
		if (found[i].Kind == ZPWS::Ecosystem)
			++eco;
		else
			++cont;
	}
	if (eco && !cont)
		return eco == 1 ? std::string("ecosystem") : toString("%u ecosystems", eco);
	if (cont && !eco)
		return std::string("continent");
	return std::string("workspace");
}

static void populateFolderList()
{
	clearList("ui:zp:folder_browser:content:list_scroll:text_list");
	if (s_Sess.FolderPath.empty())
		s_Sess.FolderPath = ZPWS::normalizeDir(CPath::getCurrentPath());

	if (CGroupEditBox *eb = findEditBox("ui:zp:folder_browser:content:goto_frame:goto_path"))
		eb->setInputString(s_Sess.FolderPath);
	setStatus("ui:zp:folder_browser:content:status", "");

	// Up: fixed control instead of a ".." row buried in a scrolling list; same destination,
	// but always in the same place and disabled outright at the filesystem root.
	{
		std::string parent = ZPWS::normalizeDir(CFile::getPath(s_Sess.FolderPath));
		const bool hasParent = !parent.empty() && parent != s_Sess.FolderPath;
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId("ui:zp:folder_browser:content:btn_up")))
			btn->setFrozen(!hasParent);
	}

	std::vector<std::string> children;
	CPath::getPathContent(s_Sess.FolderPath, false, true, false, children);
	std::sort(children.begin(), children.end());
	uint nShown = 0;
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

		std::string childPath = ZPWS::normalizeDir(child);
		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32];
		snprintf(idbuf, sizeof(idbuf), "f%u", (uint)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		p.push_back(std::make_pair(std::string("title"), name));
		p.push_back(std::make_pair(std::string("path"), childPath));
		p.push_back(std::make_pair(std::string("tag"), folderWorkspaceTag(childPath)));
		spawnRow("zp_folder_row", "ui:zp:folder_browser:content:list_scroll:text_list", p);
		++nShown;
	}

	if (CGroupList *list = findList("ui:zp:folder_browser:content:list_scroll:text_list"))
		list->invalidateCoords();

	if (CViewText *t = findText("ui:zp:folder_browser:content:count"))
		t->setHardText(nShown == 0 ? std::string("No subfolders here")
		                           : toString("%u folder(s)", nShown));
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
                            std::string &err,
                            const std::string &preferRoot)
{
	ZPWS::SWorldEntry w;
	std::vector<ZPWS::SZoneEntry> zones;
	if (!ZPWS::selectAutoMulti(worlds, autoPath, w, zones, err, preferRoot))
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

/** L-click: startup = open immediately; session hub = continent open/popup or ecosystem scratch. */
static void applyZoneSelection(int idx)
{
	if (idx < 0 || idx >= (int)s_Sess.Zones.size())
		return;

	// Session board
	if (s_Sess.SessionMode)
	{
		const std::string &base = s_Sess.Zones[idx].Basename;
		s_Sess.SelectedZone = idx;
		s_Sess.PendingActionBasename = base;

		// Ecosystem scratch
		char sk = 0;
		int cx = 0, cy = 0;
		if (s_SessionBridge && s_SessionBridge->World
		    && s_SessionBridge->World->Kind == ZPWS::Ecosystem)
		{
			// Context cell C:cx,cy:name
			if (base.size() >= 4 && base[0] == 'C' && base[1] == ':')
			{
				openContextActionPopup(base);
				return;
			}
			// Open-file cell F:...:name → the continent per-file popup, addressed
			// by the real brick basename (close/save/toggle ride the same bridge ops).
			if (base.size() >= 4 && base[0] == 'F' && base[1] == ':')
			{
				const std::string fname = scratchEditableName(base);
				if (!fname.empty())
				{
					s_Sess.PendingActionBasename = fname;
					openCellActionPopup(fname);
				}
				return;
			}
			if (parseScratchBasename(base, sk, cx, cy))
			{
				std::string err;
				if (sk == 'L')
					return; // locked cell: no menu (drag target only)
				// Empty: popup Place instance / Place context
				if (sk == 'E')
				{
					// If a context occupies this empty-looking cell, open context popup
					std::string cname;
					if (s_SessionBridge->scratchGetContext
					    && s_SessionBridge->scratchGetContext(cx, cy, cname))
					{
						s_Sess.PendingActionBasename = NLMISC::toString("C:%d,%d:%s", cx, cy, cname.c_str());
						openContextActionPopup(s_Sess.PendingActionBasename);
						return;
					}
					openEmptyCellPopup(base);
					return;
				}
				if (sk == 'I')
				{
					openInstanceActionPopup(base);
					return;
				}
			}
		}

		// Continent working-set
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

static bool isSupportedInstanceLayout(const std::string &s)
{
	return s == "1x1" || s == "2x1" || s == "1x2" || s == "2x2" || s == "3x3";
}

// ---------------------------------------------------------------------------------------------
// Action handlers

class CAHZpSelectWorld : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase *pCaller, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active) return;
		if (s_Sess.PendingSelect.empty())
			return;
		// Prefer lowest index as primary for title/compat
		s_Sess.SelectedZone = *s_Sess.PendingSelect.begin();
		s_Sess.OpenZone = true;
	}
};
REGISTER_ACTION_HANDLER(CAHZpOpenSelection, "zp_open_selection");

class CAHZpSetInstances : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		setSessionBoardVisible(false);
	}
};
REGISTER_ACTION_HANDLER(CAHZpBackToPaint, "zp_back_to_paint");

static void populateFolderList();
static void showScreen(EScreen screen);

class CAHZpBrowse : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active) return;
		s_Sess.Quit = true;
	}
};
REGISTER_ACTION_HANDLER(CAHZpStartupQuit, "zp_startup_quit");

class CAHZpFolderEnter : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active) return;
		if (params.empty()) return;
		// params is the absolute path (may be ".." encoded as the parent path already)
		s_Sess.FolderPath = ZPWS::normalizeDir(params);
		populateFolderList();
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderEnter, "zp_folder_enter");

/** Dedicated Up control (not a ".." row mixed into the scrolling list). */
class CAHZpFolderUp : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active) return;
		std::string parent = ZPWS::normalizeDir(CFile::getPath(s_Sess.FolderPath));
		if (parent.empty() || parent == s_Sess.FolderPath)
			return; // already at the filesystem root
		s_Sess.FolderPath = parent;
		populateFolderList();
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderUp, "zp_folder_up");

/** Address-bar-style direct path entry: type or paste any path, Go jumps to it.
 *  Complements the breadcrumb (ancestors only) with arbitrary destinations. */
class CAHZpFolderGo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active) return;
		CGroupEditBox *eb = findEditBox("ui:zp:folder_browser:content:goto_frame:goto_path");
		if (!eb) return;
		std::string typed = eb->getInputString();
		// Trim surrounding whitespace (stray copy-paste padding).
		size_t b = typed.find_first_not_of(" \t");
		size_t e = typed.find_last_not_of(" \t");
		typed = (b == std::string::npos) ? std::string() : typed.substr(b, e - b + 1);
		if (typed.empty())
			return;
		std::string norm = ZPWS::normalizeDir(typed);
		if (norm.empty() || !CFile::isDirectory(norm))
		{
			setStatus("ui:zp:folder_browser:content:status", "Not a folder: " + typed);
			return;
		}
		s_Sess.FolderPath = norm;
		populateFolderList();
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderGo, "zp_folder_go");

class CAHZpFolderSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		if (!s_Sess.Active || !s_Sess.Worlds) return;
		// Fingerprint this folder / scan subdirs like root detection, then go to Screen A
		std::vector<ZPWS::SWorldEntry> found;
		ZPWS::discoverWorkspaces(s_Sess.FolderPath, std::string(), found);
		if (found.empty())
		{
			setStatus("ui:zp:folder_browser:content:status", "No workspaces found here");
			return;
		}
		// Replace candidate set with the found workspaces and return to Screen A
		*s_Sess.Worlds = found;
		setStatus("ui:zp:folder_browser:content:status", "");
		showScreen(ScreenWorld);
	}
};
REGISTER_ACTION_HANDLER(CAHZpFolderSelect, "zp_folder_select");

class CAHZpFolderCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void operator()(const CEvent &event) NL_OVERRIDE
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
	// Remembered open layout (unused) + zone browser display mode
	{
		ZPWS::SStartupCfg cfg;
		const bool haveCfg = ZPWS::loadStartupCfg(cfg);
		if (haveCfg && isSupportedInstanceLayout(cfg.LastInstances))
			s_Sess.InstanceLayout = cfg.LastInstances;
		else
			s_Sess.InstanceLayout = "1x1";
		// loadStartupCfg's return means "has folder/world content"; the display-mode
		// flag applies whenever the file parsed (ctor default covers the missing case).
		s_ZoneListLarge = cfg.ZoneBrowserLarge;
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
			// Dev/test: ZONE_PAINTER_BOARD_SELECT pre-selects a used cell for multi-select fill shots.
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
		{
			showScreen(ScreenFolder);
			// Dev/test: ZONE_PAINTER_FOLDER_PATH re-navigates for headless shots of workspace tags.
			if (!screenshotPath.empty())
			{
				const char *fp = getenv("ZONE_PAINTER_FOLDER_PATH");
				if (fp && fp[0] && CFile::isDirectory(fp))
				{
					s_Sess.FolderPath = ZPWS::normalizeDir(fp);
					populateFolderList();
				}
			}
		}
		else
			showScreen(ScreenWorld);
	}

	driver->setWindowTitle(ucstring("zone_painter - startup"));

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
				// Multi-select editable set; empty PendingSelect still has one via L-click path
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
				// Layout field unused; open primary only (use BOARD/--place for instances)
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
	s_Sess.Worlds = nullptr;
	return result;
}


} // namespace ZPUI

/* end of file */
