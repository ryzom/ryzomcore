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

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <map>
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
	int SelectedZone;  // index into Zones
	std::string FolderPath;
	std::string StatusMsg;
	SStartupSession()
		: Active(false), Quit(false), OpenZone(false), FolderBrowserEnabled(false),
		  Screen(ScreenNone), Worlds(NULL), SelectedWorld(-1), SelectedZone(-1)
	{
	}
};

static SStartupSession s_Sess;

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

static std::string thumbTextureName(const ZPWS::SZoneEntry &z)
{
	if (z.ThumbnailPath.empty())
		return std::string("w_box_blank.tga"); // atlas placeholder when no thumb
	// Ensure the directory is on the search path so the basename resolves
	std::string dir = CFile::getPath(z.ThumbnailPath);
	if (!dir.empty())
		CPath::addSearchPath(dir, false, false);
	// png→tga remap is set in editor_ui init; request as .tga name
	std::string base = CFile::getFilenameWithoutExtension(z.ThumbnailPath);
	return base + ".tga";
}

/** Parent a template instance under an existing group at an absolute offset (continent grid). */
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

/** Continent Screen B: name-derived grid (row vertical, col letters horizontal). */
static void populateContinentGrid(const ZPWS::SWorldEntry &world)
{
	std::map<std::pair<int, int>, int> cell; // (row,col) -> zone index
	int minR = INT_MAX, maxR = INT_MIN, minC = INT_MAX, maxC = INT_MIN;
	uint nParse = 0;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		int r = 0, c = 0;
		if (!ZPWS::parseContinentZoneName(s_Sess.Zones[i].Basename, r, c))
			continue;
		cell[std::make_pair(r, c)] = (int)i;
		if (r < minR) minR = r;
		if (r > maxR) maxR = r;
		if (c < minC) minC = c;
		if (c > maxC) maxC = c;
		++nParse;
	}

	if (nParse == 0)
	{
		// Unparseable names: flat list fallback
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
			spawnRow("zp_zone_row", "ui:zp:zone_browser:content:list_scroll:text_list", p);
		}
		return;
	}

	const int cellW = 46;
	const int cellH = 26;
	const int nCols = maxC - minC + 1;
	const int rowW = nCols * cellW + 4;

	// Higher row numbers at the top (map-like: north / higher Y up)
	for (int r = maxR; r >= minR; --r)
	{
		std::vector<std::pair<std::string, std::string> > rp;
		char rid[32];
		snprintf(rid, sizeof(rid), "gr%d", r);
		rp.push_back(std::make_pair(std::string("id"), std::string(rid)));
		CInterfaceGroup *row = spawnRow("zp_zone_grid_row",
		                                "ui:zp:zone_browser:content:list_scroll:text_list", rp);
		if (!row)
			continue;
		row->setW(rowW);
		row->setH(cellH);

		for (int c = minC; c <= maxC; ++c)
		{
			const int x = (c - minC) * cellW;
			std::map<std::pair<int, int>, int>::const_iterator it = cell.find(std::make_pair(r, c));
			if (it == cell.end())
			{
				std::vector<std::pair<std::string, std::string> > ep;
				char eid[40];
				snprintf(eid, sizeof(eid), "ge%d_%d", r, c);
				ep.push_back(std::make_pair(std::string("id"), std::string(eid)));
				spawnUnder(row, "zp_zone_grid_empty", ep, x, 0, cellW, cellH);
				continue;
			}
			const int zi = it->second;
			const ZPWS::SZoneEntry &z = s_Sess.Zones[zi];
			std::vector<std::pair<std::string, std::string> > p;
			char idbuf[40], idxbuf[32];
			snprintf(idbuf, sizeof(idbuf), "gc%d_%d", r, c);
			snprintf(idxbuf, sizeof(idxbuf), "%d", zi);
			p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
			// Cell shows the zone basename (actual casing); neighbor count in status only
			p.push_back(std::make_pair(std::string("title"), z.Basename));
			p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
			spawnUnder(row, "zp_zone_grid_cell", p, x, 0, cellW, cellH);
		}
	}

	if (CGroupList *list = findList("ui:zp:zone_browser:content:list_scroll:text_list"))
		list->invalidateCoords();

	// Status: grid extent + hint that neighbors load on open
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
	{
		std::string colLo = ZPWS::continentZoneName(0, minC);
		std::string colHi = ZPWS::continentZoneName(0, maxC);
		// continentZoneName is "0_XX"; show just the two letters
		std::string::size_type u1 = colLo.find('_');
		std::string::size_type u2 = colHi.find('_');
		std::string lettersLo = (u1 != std::string::npos) ? colLo.substr(u1 + 1) : colLo;
		std::string lettersHi = (u2 != std::string::npos) ? colHi.substr(u2 + 1) : colHi;
		t->setHardText(world.GraphicsRoot
		               + NLMISC::toString("  |  rows %d..%d  cols %s..%s  (%u zones; open loads neighbors)",
		                                  minR, maxR, lettersLo.c_str(), lettersHi.c_str(),
		                                  (uint)s_Sess.Zones.size()));
	}
}

static void populateZoneList()
{
	clearList("ui:zp:zone_browser:content:list_scroll:text_list");
	if (!s_Sess.Worlds || s_Sess.SelectedWorld < 0
	    || s_Sess.SelectedWorld >= (int)s_Sess.Worlds->size())
		return;

	const ZPWS::SWorldEntry &world = (*s_Sess.Worlds)[s_Sess.SelectedWorld];
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_title"))
		t->setHardText(world.WorldName
		               + (world.Kind == ZPWS::Ecosystem ? "  (ecosystem)" : "  (continent)"));
	if (CViewText *t = findText("ui:zp:zone_browser:content:world_sub"))
		t->setHardText(world.GraphicsRoot);

	ZPWS::listZones(world, s_Sess.Zones);

	// Continents: name-derived coordinate grid (M3c). Ecosystems keep the grouped list.
	if (world.Kind == ZPWS::Continent)
	{
		populateContinentGrid(world);
		return;
	}

	std::string lastGroup;
	for (size_t i = 0; i < s_Sess.Zones.size(); ++i)
	{
		const ZPWS::SZoneEntry &z = s_Sess.Zones[i];
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
		p.push_back(std::make_pair(std::string("thumb"), thumbTextureName(z)));
		spawnRow("zp_zone_row", "ui:zp:zone_browser:content:list_scroll:text_list", p);
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
	ZPWS::SZoneEntry z;
	if (!ZPWS::selectAuto(worlds, autoPath, w, z, err))
		return false;
	selection.World = w;
	selection.Zone = z;
	return true;
}

static void applyWorldSelection(int idx)
{
	if (!s_Sess.Worlds || idx < 0 || idx >= (int)s_Sess.Worlds->size())
		return;
	if (!(*s_Sess.Worlds)[idx].BankOk)
		return;
	s_Sess.SelectedWorld = idx;
	showScreen(ScreenZone);
}

static void applyZoneSelection(int idx)
{
	if (idx < 0 || idx >= (int)s_Sess.Zones.size())
		return;
	s_Sess.SelectedZone = idx;
	s_Sess.OpenZone = true;
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

class CAHZpZoneBack : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		if (!s_Sess.Active) return;
		s_Sess.SelectedWorld = -1;
		s_Sess.Zones.clear();
		showScreen(ScreenWorld);
	}
};
REGISTER_ACTION_HANDLER(CAHZpZoneBack, "zp_zone_back");

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

} // namespace ZPUI

/* end of file */
