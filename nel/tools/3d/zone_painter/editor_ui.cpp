/**
 * \file editor_ui.cpp
 * \brief In-engine NLGUI facade for the standalone zone painter (ui M1)
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
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

#include "editor_ui.h"
#include "max_thumbnail.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <utility>

#include <nel/misc/algo.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i18n.h>
#include <nel/misc/path.h>

#include <nel/3d/tile_bank.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/event_listener.h>
#include <nel/gui/group_container.h>
#include <nel/gui/group_editbox.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/interface_link.h>
#include <nel/gui/view_bitmap.h>
#include <nel/gui/view_pointer.h>
#include <nel/gui/view_renderer.h>
#include <nel/gui/view_text.h>
#include <nel/gui/widget_manager.h>

#ifndef ZONE_PAINTER_UI_DIR
#define ZONE_PAINTER_UI_DIR "."
#endif
#ifndef ZONE_PAINTER_ASSETS_DIR
#define ZONE_PAINTER_ASSETS_DIR "."
#endif
#ifndef ZONE_PAINTER_FONT
#define ZONE_PAINTER_FONT "ingame.ttf"
#endif

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

namespace ZPUI {

const char *CEditorUI::MASTER_GROUP = "ui:zp";

// Process-wide bridge (runViewer installs; action handlers call through it)
static SPaintUIBridge *s_Bridge = NULL;

void setPaintUIBridge(SPaintUIBridge *bridge) { s_Bridge = bridge; }
SPaintUIBridge *getPaintUIBridge() { return s_Bridge; }

// ---------------------------------------------------------------------------------------------
// Pointer button mirror (from nel/samples/gui)

class CPointerButtonListener : public NLMISC::IEventListener
{
	uint _DownButtons;
	virtual void operator()(const NLMISC::CEvent &event)
	{
		CViewPointerBase *pointer = CWidgetManager::getInstance()->getPointer();
		if (!pointer)
			return;
		if (event == EventMouseDownId)
		{
			CEventMouseDown &em = (CEventMouseDown &)event;
			_DownButtons |= em.Button & (leftButton | middleButton | rightButton);
			pointer->setPointerDown(em.Button == leftButton);
			pointer->setPointerMiddleDown(em.Button == middleButton);
			pointer->setPointerRightDown(em.Button == rightButton);
		}
		else if (event == EventMouseUpId)
		{
			CEventMouseUp &em = (CEventMouseUp &)event;
			_DownButtons &= ~(em.Button & (leftButton | middleButton | rightButton));
			if (_DownButtons == 0)
			{
				pointer->setPointerDown(false);
				pointer->setPointerMiddleDown(false);
				pointer->setPointerRightDown(false);
			}
		}
		else if (event == EventSetFocusId)
		{
			_DownButtons = 0;
			pointer->setPointerDown(false);
			pointer->setPointerMiddleDown(false);
			pointer->setPointerRightDown(false);
		}
	}

public:
	CPointerButtonListener() : _DownButtons(0) {}
	void addToServer(CEventServer &server)
	{
		server.addListener(EventMouseDownId, this);
		server.addListener(EventMouseUpId, this);
		server.addListener(EventSetFocusId, this);
	}
	void removeFromServer(CEventServer &server)
	{
		server.removeListener(EventMouseDownId, this);
		server.removeListener(EventMouseUpId, this);
		server.removeListener(EventSetFocusId, this);
	}
};

// Minimal client-side action handlers the interface XML may invoke
class CAHProc : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		std::vector<std::string> paramList;
		splitString(params, "|", paramList);
		if (paramList.empty())
			return;
		CWidgetManager::getInstance()->runProcedure(paramList.front(), pCaller, paramList);
	}
};
REGISTER_ACTION_HANDLER(CAHProc, "proc");

class CAHEnterModal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		std::string group = getParam(params, "group");
		CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
	}
};
REGISTER_ACTION_HANDLER(CAHEnterModal, "enter_modal");

class CAHLeaveModal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHLeaveModal, "leave_modal");

class CAHSubmitQuickHelp : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) {}
};
REGISTER_ACTION_HANDLER(CAHSubmitQuickHelp, "submit_quick_help");

// ---------------------------------------------------------------------------------------------
// Paint action handlers: thin wrappers over the bridge (no second op implementation)

class CAHZpMode : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectMode) return;
		int mode = 0;
		fromString(params, mode);
		b->selectMode(mode);
	}
};
REGISTER_ACTION_HANDLER(CAHZpMode, "zp_mode");

class CAHZpTileSet : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectTileSetDelta) return;
		int d = 0;
		fromString(params, d);
		b->selectTileSetDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpTileSet, "zp_tileset");

/** Absolute tile-set select from the palette grid (ui M8) — same zpSelectTileSetAbs path. */
class CAHZpTileSetAbs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectTileSetAbs) return;
		int idx = 0;
		fromString(params, idx);
		b->selectTileSetAbs(idx);
	}
};
REGISTER_ACTION_HANDLER(CAHZpTileSetAbs, "zp_tileset_abs");

class CAHZpTogglePalette : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->togglePalette)
			b->togglePalette();
		else
			toggleTilesetPalette();
	}
};
REGISTER_ACTION_HANDLER(CAHZpTogglePalette, "zp_toggle_palette");

class CAHZpToggle256 : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleTileSize) b->toggleTileSize();
	}
};
REGISTER_ACTION_HANDLER(CAHZpToggle256, "zp_toggle_256");

class CAHZpBrush : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->brushSizeDelta) return;
		int d = 0;
		fromString(params, d);
		b->brushSizeDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpBrush, "zp_brush");

class CAHZpGroup : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->groupDelta) return;
		int d = 0;
		fromString(params, d);
		b->groupDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpGroup, "zp_group");

class CAHZpLockBorders : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleLockBorders) b->toggleLockBorders();
	}
};
REGISTER_ACTION_HANDLER(CAHZpLockBorders, "zp_lock_borders");

class CAHZpUndo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->undo) b->undo();
	}
};
REGISTER_ACTION_HANDLER(CAHZpUndo, "zp_undo");

class CAHZpRedo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->redo) b->redo();
	}
};
REGISTER_ACTION_HANDLER(CAHZpRedo, "zp_redo");

class CAHZpFill : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->fill) return;
		int rot = 0;
		fromString(params, rot);
		b->fill(rot);
	}
};
REGISTER_ACTION_HANDLER(CAHZpFill, "zp_fill");

class CAHZpSeasonNext : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->seasonNext && b->SeasonCount > 1)
			b->seasonNext();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonNext, "zp_season_next");

// Color / displace panel (M7a) — thin wrappers over bridge (same as keys)

class CAHZpColorRadius : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->colorRadiusDelta) return;
		int d = 0;
		fromString(params, d);
		b->colorRadiusDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpColorRadius, "zp_color_radius");

class CAHZpHardness : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->hardnessDelta) return;
		int d = 0;
		fromString(params, d);
		b->hardnessDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpHardness, "zp_hardness");

class CAHZpOpacity : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->opacityDelta) return;
		int d = 0;
		fromString(params, d);
		b->opacityDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpOpacity, "zp_opacity");

class CAHZpCycleMask : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->cycleBrushMask) b->cycleBrushMask();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCycleMask, "zp_cycle_mask");

class CAHZpMaskMode : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleMaskMode) b->toggleMaskMode();
	}
};
REGISTER_ACTION_HANDLER(CAHZpMaskMode, "zp_mask_mode");

class CAHZpDisplace : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->displaceIndexDelta) return;
		int d = 0;
		fromString(params, d);
		b->displaceIndexDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpDisplace, "zp_displace");

class CAHZpSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->CanSave) return;
		// Interactive flow without --save: open the overwrite/copy modal.
		// CLI --save (including startup-auto + --save): one-click direct save.
		if (b->InteractiveSave)
			openSaveDialog();
		else if (b->save)
			b->save();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSave, "zp_save");

// ---------------------------------------------------------------------------------------------
// Save modal (interactive flow only)

static bool s_SaveCopyConfirm = false;

static CViewText *findTextEarly(const char *id)
{
	return dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CCtrlTextButton *findTextButton(const char *id)
{
	return dynamic_cast<CCtrlTextButton *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CGroupEditBox *findEditBox(const char *id)
{
	return dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static void setSaveModalStatus(const std::string &msg)
{
	if (CViewText *t = findTextEarly("ui:zp:save_dialog:content:status"))
		t->setHardText(msg);
}

static void resetSaveCopyButtonLabel()
{
	s_SaveCopyConfirm = false;
	if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_copy"))
		btn->setHardText("Save copy");
}

void openSaveDialog()
{
	SPaintUIBridge *b = getPaintUIBridge();
	if (!b) return;
	resetSaveCopyButtonLabel();
	// Multi-file (M6b): Overwrite is save-all of dirty editables
	if (b->EditableFileCount > 1)
	{
		setSaveModalStatus(NLMISC::toString(
		    "Save-all: %u file(s), %u dirty (Overwrite writes each dirty file in place)",
		    b->EditableFileCount, b->DirtyFileCount));
		if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_overwrite"))
			btn->setHardText("Overwrite all");
	}
	else
	{
		setSaveModalStatus("");
		if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_overwrite"))
			btn->setHardText("Overwrite");
	}
	// Prefill edit box with <basename>_painted.max
	std::string prefill = std::string(b->EditableBasename) + "_painted.max";
	if (CGroupEditBox *eb = findEditBox("ui:zp:save_dialog:content:copy_name"))
		eb->setInputString(prefill);
	// M5c: thumbnail checkbox defaults on
	b->UpdateThumbnail = true;
	if (CCtrlBaseButton *tb = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb")))
		tb->setPushed(true);
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:save_dialog");
}

class CAHZpSaveThumbToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		// Toggle button reports new pushed state after click via getPushed
		if (CCtrlBaseButton *tb = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb")))
			b->UpdateThumbnail = tb->getPushed();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSaveThumbToggle, "zp_save_thumb_toggle");

void forceShowSaveDialogForShot()
{
	openSaveDialog();
}

// ---------------------------------------------------------------------------------------------
// Tileset palette (ui M8)

static const sint32 kPaletteCellW = 96;
static const sint32 kPaletteCellH = 92;
static const sint32 kPaletteCols = 3;
static const sint32 kPaletteThumb = 64;
static const char *kPaletteGridId = "ui:zp:tiles_palette:content:grid_host:grid";
static const char *kPaletteWinId = "ui:zp:tiles_palette";

static int s_PaletteBuiltCount = 0;
static int s_PaletteLastHighlight = -1;
static bool s_PaletteVisible = false;

static CInterfaceGroup *findGroupEl(const char *id)
{
	return dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static void setContainerActive(const char *id, bool active)
{
	if (CGroupContainer *c = dynamic_cast<CGroupContainer *>(
	        CWidgetManager::getInstance()->getElementFromId(id)))
		c->setActive(active);
	else if (CInterfaceGroup *g = findGroupEl(id))
		g->setActive(active);
}

bool isTilesetPaletteVisible()
{
	return s_PaletteVisible;
}

void setTilesetPaletteVisible(bool visible)
{
	s_PaletteVisible = visible;
	setContainerActive(kPaletteWinId, visible);
	// Panel toggle button face (if present)
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:painter:content:btn_palette")))
		btn->setPushed(visible);
}

void toggleTilesetPalette()
{
	setTilesetPaletteVisible(!s_PaletteVisible);
}

/** Spawn a palette cell under the grid (same idiom as the continent board). */
static CInterfaceGroup *spawnPaletteCell(CInterfaceGroup *parent,
                                         const std::vector<std::pair<std::string, std::string> > &params,
                                         sint32 x, sint32 y)
{
	if (!parent)
		return NULL;
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *g = NULL;
	if (!params.empty())
		g = parser->createGroupInstance("zp_tileset_cell", parent->getId(),
		                                &params[0], (uint)params.size());
	else
		g = parser->createGroupInstance("zp_tileset_cell", parent->getId(),
		                                (const std::pair<std::string, std::string> *)NULL, 0);
	if (!g)
		return NULL;
	g->setParent(parent);
	g->setParentPos(parent);
	g->setPosRef(Hotspot_TL);
	g->setParentPosRef(Hotspot_TL);
	g->setX(x);
	g->setY(y);
	g->setW(kPaletteCellW);
	g->setH(kPaletteCellH);
	g->setActive(true);
	parent->addGroup(g);
	return g;
}

/**
 * Resolve a representative 128 diffuse path for one tileset: first non-empty diffuse
 * whose CPath lookup succeeds (after resolveBankTextures remaps).
 * Also returns the bank-stored relative name (for fallback labelling) via *storedNameOut.
 */
static std::string firstResolvableDiffuse(NL3D::CTileBank *bank, int setIndex,
                                          std::string *storedNameOut = NULL)
{
	if (storedNameOut)
		storedNameOut->clear();
	if (!bank || setIndex < 0 || setIndex >= bank->getTileSetCount())
		return std::string();
	const NL3D::CTileSet *ts = bank->getTileSet(setIndex);
	if (!ts)
		return std::string();
	for (sint t = 0; t < ts->getNumTile128(); ++t)
	{
		const NL3D::CTile *pt = bank->getTile(ts->getTile128(t));
		if (!pt)
			continue;
		std::string name = pt->getRelativeFileName(NL3D::CTile::diffuse);
		if (name.empty())
			continue;
		for (size_t k = 0; k < name.size(); ++k)
			if (name[k] == '\\') name[k] = '/';
		// Prefer basename (bank stores may carry subdirs; resolveBankTextures remaps basenames)
		std::string base = CFile::getFilename(name);
		if (storedNameOut && storedNameOut->empty())
			*storedNameOut = base;
		std::string path = CPath::lookup(base, false, false);
		if (path.empty())
			path = CPath::lookup(name, false, false);
		if (!path.empty())
		{
			if (storedNameOut)
				*storedNameOut = base;
			return path;
		}
	}
	return std::string();
}

/**
 * Friendly set label: bank getName() when present; else derive from diffuse stem
 * (Ryzom tiles: y-plages-128-a-01.png → plages). smallbanks often store empty set names.
 */
static std::string tilesetDisplayName(NL3D::CTileBank *bank, int setIndex,
                                      const std::string &storedDiffuse)
{
	if (bank && setIndex >= 0 && setIndex < bank->getTileSetCount())
	{
		const NL3D::CTileSet *ts = bank->getTileSet(setIndex);
		if (ts && !ts->getName().empty())
			return ts->getName();
	}
	if (storedDiffuse.empty())
		return std::string();
	std::string base = CFile::getFilenameWithoutExtension(storedDiffuse);
	// strip leading type letter + '-' (y- / t- / …)
	if (base.size() > 2 && base[1] == '-')
		base = base.substr(2);
	// strip -128… / -256… suffix
	std::string::size_type p = base.find("-128");
	if (p == std::string::npos)
		p = base.find("-256");
	if (p != std::string::npos)
		base = base.substr(0, p);
	return base;
}

void rebuildTilesetPalette(NL3D::CTileBank *bank, const std::string &bankPath,
                           const std::string &seasonKey)
{
	CInterfaceGroup *grid = findGroupEl(kPaletteGridId);
	if (!grid)
	{
		// UI not ready / soft-fail path
		s_PaletteBuiltCount = 0;
		s_PaletteLastHighlight = -1;
		return;
	}
	grid->clearGroups();
	s_PaletteBuiltCount = 0;
	s_PaletteLastHighlight = -1;

	if (!bank)
	{
		grid->setW(kPaletteCellW);
		grid->setH(kPaletteCellH);
		return;
	}

	const sint nSets = bank->getTileSetCount();
	if (nSets <= 0)
	{
		grid->setW(kPaletteCellW);
		grid->setH(kPaletteCellH);
		return;
	}

	const int cols = kPaletteCols;
	const int rows = (nSets + cols - 1) / cols;
	grid->setW(cols * kPaletteCellW);
	grid->setH(rows * kPaletteCellH);
	grid->setOfsX(0);
	grid->setOfsY(0);

	// Ensure cache dir is on the NLGUI search path (once per rebuild is fine)
	std::string cacheDir = ZPTHUMB::tilesetPreviewCacheDir();
	if (!cacheDir.empty())
		CPath::addSearchPath(cacheDir, false, false);

	uint withPreview = 0;
	uint nameOnly = 0;
	for (sint i = 0; i < nSets; ++i)
	{
		const int col = i % cols;
		const int row = i / cols;
		const sint32 x = col * kPaletteCellW;
		const sint32 y = -(row * kPaletteCellH);

		std::string storedDiffuse;
		std::string source = firstResolvableDiffuse(bank, (int)i, &storedDiffuse);
		std::string setName = tilesetDisplayName(bank, (int)i, storedDiffuse);
		if (setName.empty())
			setName = "(unnamed)";
		// Short label: "1 name" (1-based index for the artist)
		char title[160];
		snprintf(title, sizeof(title), "%d %s", (int)(i + 1), setName.c_str());
		// Truncate very long names so the 96px cell stays readable
		if (strlen(title) > 18)
		{
			title[15] = '.';
			title[16] = '.';
			title[17] = '.';
			title[18] = 0;
		}

		std::string thumbTex;
		if (!source.empty())
		{
			std::string cached;
			if (ZPTHUMB::ensureTilesetPreview(bankPath, (int)i, seasonKey, source, cached, kPaletteThumb)
			    && !cached.empty())
			{
				thumbTex = CFile::getFilenameWithoutExtension(cached) + ".tga";
				++withPreview;
			}
		}
		if (thumbTex.empty())
			++nameOnly;

		std::vector<std::pair<std::string, std::string> > p;
		char idbuf[32], idxbuf[16];
		snprintf(idbuf, sizeof(idbuf), "ts%03d", (int)i);
		snprintf(idxbuf, sizeof(idxbuf), "%d", (int)i);
		p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
		p.push_back(std::make_pair(std::string("title"), std::string(title)));
		p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
		p.push_back(std::make_pair(std::string("thumb"),
		                           thumbTex.empty() ? std::string("w_box_blank.tga") : thumbTex));

		CInterfaceGroup *cell = spawnPaletteCell(grid, p, x, y);
		if (cell)
		{
			if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(cell->getView("thumb")))
				thumb->setActive(!thumbTex.empty());
		}
	}
	s_PaletteBuiltCount = (int)nSets;
	printf("tiles palette: %d sets (%u previews, %u name-only) season=%s cache=%s\n",
	       (int)nSets, withPreview, nameOnly,
	       seasonKey.empty() ? "auto" : seasonKey.c_str(),
	       cacheDir.c_str());
}

/** Highlight the current tileset cell (toggle_button pushed state). */
static void syncPaletteHighlight(int curTileSet, uint tileSetCount)
{
	if (s_PaletteBuiltCount <= 0)
		return;
	if (s_PaletteLastHighlight == curTileSet)
		return;
	CInterfaceGroup *grid = findGroupEl(kPaletteGridId);
	if (!grid)
		return;
	// Clear previous
	if (s_PaletteLastHighlight >= 0 && s_PaletteLastHighlight < s_PaletteBuiltCount)
	{
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "%s:ts%03d:btn", kPaletteGridId, s_PaletteLastHighlight);
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId(idbuf)))
			btn->setPushed(false);
	}
	if (tileSetCount && curTileSet >= 0 && curTileSet < (int)tileSetCount
	    && curTileSet < s_PaletteBuiltCount)
	{
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "%s:ts%03d:btn", kPaletteGridId, curTileSet);
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId(idbuf)))
			btn->setPushed(true);
		s_PaletteLastHighlight = curTileSet;
	}
	else
		s_PaletteLastHighlight = -1;
}

static void syncThumbWantFromModal(SPaintUIBridge *b)
{
	if (!b) return;
	if (CCtrlBaseButton *tb = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb")))
		b->UpdateThumbnail = tb->getPushed();
}

class CAHZpSaveOverwrite : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->saveOverwrite) return;
		syncThumbWantFromModal(b);
		// Bridge UpdateThumbnail is read by main.cpp via g_PaintCtx.WantThumbnail before save
		if (b->saveOverwrite())
		{
			setSaveModalStatus("Saved (overwrite).");
			CWidgetManager::getInstance()->disableModalWindow();
			resetSaveCopyButtonLabel();
		}
		else
			setSaveModalStatus("Overwrite failed (see stderr).");
	}
};
REGISTER_ACTION_HANDLER(CAHZpSaveOverwrite, "zp_save_overwrite");

class CAHZpSaveCopy : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->saveTo) return;

		std::string name;
		if (CGroupEditBox *eb = findEditBox("ui:zp:save_dialog:content:copy_name"))
			name = eb->getInputString();
		// Strip whitespace
		while (!name.empty() && (name[0] == ' ' || name[0] == '\t')) name.erase(0, 1);
		while (!name.empty() && (name[name.size() - 1] == ' ' || name[name.size() - 1] == '\t'))
			name.resize(name.size() - 1);
		if (name.empty())
		{
			setSaveModalStatus("Enter a file name.");
			return;
		}
		// Absolute or relative to the opened .max directory
		std::string target;
		if (!name.empty() && name[0] == '/')
			target = name;
		else
		{
			std::string dir = b->InputDir;
			if (!dir.empty() && dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
				dir += "/";
			target = dir + name;
		}
		if (toLowerAscii(CFile::getExtension(target)).empty())
			target += ".max";

		if (CFile::fileExists(target) && !s_SaveCopyConfirm)
		{
			s_SaveCopyConfirm = true;
			if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_copy"))
				btn->setHardText("Confirm overwrite");
			setSaveModalStatus("File exists — click again to overwrite.");
			return;
		}

		if (b->saveTo(target))
		{
			setSaveModalStatus("Saved copy.");
			CWidgetManager::getInstance()->disableModalWindow();
			resetSaveCopyButtonLabel();
		}
		else
		{
			setSaveModalStatus("Save copy failed (see stderr).");
			resetSaveCopyButtonLabel();
		}
	}
};
REGISTER_ACTION_HANDLER(CAHZpSaveCopy, "zp_save_copy");

class CAHZpSaveCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		resetSaveCopyButtonLabel();
		setSaveModalStatus("");
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSaveCancel, "zp_save_cancel");

// ---------------------------------------------------------------------------------------------

CEditorUI::CEditorUI()
	: _Ready(false), _Visible(true), _Driver(NULL), _TextContext(NULL),
	  _GuiListener(NULL), _PointerButtons(NULL)
{
}

CEditorUI::~CEditorUI()
{
	shutdown();
}

static std::string resolveDataDir()
{
	if (CFile::isDirectory(ZONE_PAINTER_UI_DIR))
		return ZONE_PAINTER_UI_DIR;
	return "ui";
}

static std::string resolveAssetsDir()
{
	if (CFile::isDirectory(ZONE_PAINTER_ASSETS_DIR))
		return ZONE_PAINTER_ASSETS_DIR;
	return "assets";
}

static std::string resolveFont(const std::string &hint)
{
	if (!hint.empty() && CFile::fileExists(hint))
		return hint;
	if (CFile::fileExists(ZONE_PAINTER_FONT))
		return ZONE_PAINTER_FONT;
	std::string looked = CPath::lookup(CFile::getFilename(ZONE_PAINTER_FONT), false, false, false);
	if (!looked.empty())
		return looked;
	const char *sysFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
	if (CFile::fileExists(sysFont))
		return sysFont;
	return std::string();
}

bool CEditorUI::init(UDriver *driver, const std::string &fontPathHint)
{
	shutdown();
	if (!driver)
		return false;

	_Driver = driver;

	CPath::remapExtension("png", "tga", true);

	const std::string dataDir = resolveDataDir();
	const std::string assetsDir = resolveAssetsDir();
	CPath::addSearchPath(dataDir, true, false);
	CPath::addSearchPath(assetsDir, false, false);

	CI18N::setNoResolution(true);

	std::string font = resolveFont(fontPathHint);
	if (font.empty())
	{
		fprintf(stderr, "WARNING: editor UI: no font found; NLGUI disabled\n");
		return false;
	}
	_TextContext = _Driver->createTextContext(font);
	if (!_TextContext)
	{
		fprintf(stderr, "WARNING: editor UI: createTextContext failed; NLGUI disabled\n");
		return false;
	}
	_TextContext->setKeep800x600Ratio(false);

	new CInterfaceLink::CInterfaceLinkUpdater(); // leaked once, process lifetime

	CViewRenderer::setDriver(_Driver);
	CViewRenderer::setTextContext(_TextContext);
	CViewRenderer::hwCursors = &_HwCursors;
	CViewRenderer::getInstance()->init();

	if (!CViewRenderer::getInstance()->loadTextures("zone_painter_atlas.png", "zone_painter_atlas.txt", false))
	{
		fprintf(stderr, "WARNING: editor UI: unable to load zone_painter_atlas.png/.txt from '%s'\n",
		        assetsDir.c_str());
		CViewRenderer::release();
		_Driver->deleteTextContext(_TextContext);
		_TextContext = NULL;
		return false;
	}

	IParser *parser = CWidgetManager::getInstance()->getParser();
	parser->initLUA();

	std::vector<std::string> xmlFiles;
	xmlFiles.push_back("config.xml");
	xmlFiles.push_back("widgets.xml");
	xmlFiles.push_back("main.xml");
	if (!parser->parseInterface(xmlFiles, false))
	{
		fprintf(stderr, "WARNING: editor UI: failed to parse interface XML from '%s'\n", dataDir.c_str());
		CWidgetManager::getInstance()->reset();
		CWidgetManager::getInstance()->getParser()->removeAll();
		CViewRenderer::release();
		_Driver->deleteTextContext(_TextContext);
		_TextContext = NULL;
		return false;
	}

	CWidgetManager::getInstance()->updateAllLocalisedElements();
	CWidgetManager::getInstance()->activateMasterGroup(MASTER_GROUP, true);
	CInterfaceLink::updateAllLinks();

	_GuiListener = new CEventListener();
	_GuiListener->addToServer(&_Driver->EventServer);
	_PointerButtons = new CPointerButtonListener();
	_PointerButtons->addToServer(_Driver->EventServer);

	_Ready = true;
	_Visible = true;
	printf("editor UI: ready (data=%s assets=%s)\n", dataDir.c_str(), assetsDir.c_str());
	return true;
}

void CEditorUI::shutdown()
{
	if (_GuiListener)
	{
		_GuiListener->removeFromServer();
		delete _GuiListener;
		_GuiListener = NULL;
	}
	if (_PointerButtons && _Driver)
	{
		_PointerButtons->removeFromServer(_Driver->EventServer);
		delete _PointerButtons;
		_PointerButtons = NULL;
	}
	if (_Ready)
	{
		CWidgetManager::getInstance()->reset();
		CWidgetManager::getInstance()->getParser()->removeAll();
		CViewRenderer::release();
	}
	if (_TextContext && _Driver)
	{
		_Driver->deleteTextContext(_TextContext);
		_TextContext = NULL;
	}
	_Driver = NULL;
	_Ready = false;
	_Visible = false;
	_HwCursors.clear();
}

static CViewText *findText(const char *id)
{
	return dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(id));
}

static CCtrlBaseButton *findButton(const char *id)
{
	return dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(id));
}

void CEditorUI::syncPanelFromBridge()
{
	SPaintUIBridge *b = getPaintUIBridge();
	if (!b || !b->HaveCore)
		return;

	// Mode radios
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:mode_tile"))
		btn->setPushed(b->Mode == 0);
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:mode_color"))
		btn->setPushed(b->Mode == 1);
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:mode_displace"))
		btn->setPushed(b->Mode == 2);

	// Tile set label (1-based index; unnamed sets show as (unnamed) not empty quotes)
	if (CViewText *t = findText("ui:zp:painter:content:tileset_info"))
	{
		char buf[192];
		const char *name = b->TileSetName;
		if (!name || !name[0])
			name = "(unnamed)";
		const int oneBased = b->TileSetCount ? (b->CurTileSet + 1) : 0;
		snprintf(buf, sizeof(buf), "%d/%u %s", oneBased, b->TileSetCount, name);
		t->setHardText(buf);
	}

	// 256 toggle
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:toggle_256"))
		btn->setPushed(b->Mode256);

	// Brush / group
	if (CViewText *t = findText("ui:zp:painter:content:brush_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->BrushSize);
		t->setHardText(buf);
	}
	if (CViewText *t = findText("ui:zp:painter:content:group_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->TileGroup);
		t->setHardText(buf);
	}

	// Lock borders
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:lock_borders"))
		btn->setPushed(b->LockBorders);

	// Undo depth
	if (CViewText *t = findText("ui:zp:painter:content:undo_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->UndoDepth);
		t->setHardText(buf);
	}

	// Self-instance indicator (M4b)
	if (CViewText *t = findText("ui:zp:painter:content:instance_info"))
	{
		if (b->InstanceCount > 1)
		{
			char buf[48];
			snprintf(buf, sizeof(buf), "INSTANCED x%u", b->InstanceCount);
			t->setHardText(buf);
		}
		else
			t->setHardText("");
	}

	// Season variant (M6a)
	if (CViewText *t = findText("ui:zp:painter:content:season_info"))
	{
		if (b->SeasonCount == 0)
			t->setHardText("Season: (none)");
		else
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "Season: %s",
			         b->SeasonLabel[0] ? b->SeasonLabel : "auto");
			t->setHardText(buf);
		}
	}
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:btn_season"))
		btn->setFrozen(b->SeasonCount < 2);

	// Multi-file dirty indicator (M6b): "N files, M dirty"
	if (CViewText *t = findText("ui:zp:painter:content:files_info"))
	{
		char buf[64];
		if (b->EditableFileCount <= 1)
		{
			if (b->DirtyFileCount)
				snprintf(buf, sizeof(buf), "1 file, dirty");
			else
				snprintf(buf, sizeof(buf), "1 file");
		}
		else
			snprintf(buf, sizeof(buf), "%u files, %u dirty",
			         b->EditableFileCount, b->DirtyFileCount);
		t->setHardText(buf);
	}

	// Save enabled only with --save
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:btn_save"))
		btn->setFrozen(!b->CanSave);

	// ---- Color / displace (M7a): always-visible; section labels highlight on active mode ----
	const bool colorActive = (b->Mode == 1);
	const bool displaceActive = (b->Mode == 2);
	const NLMISC::CRGBA colActive(255, 220, 140, 255);
	const NLMISC::CRGBA colIdle(255, 255, 255, 140);

	if (CViewText *t = findText("ui:zp:painter:content:lbl_color"))
		t->setColor(colorActive ? colActive : colIdle);
	if (CViewText *t = findText("ui:zp:painter:content:lbl_displace"))
		t->setColor(displaceActive ? colActive : colIdle);

	// Color radius (2-32m; SizeUp/Down keys share zp_brush when Mode==Color)
	if (CViewText *t = findText("ui:zp:painter:content:radius_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%.1f", b->ColorRadius);
		t->setHardText(buf);
	}
	if (CViewText *t = findText("ui:zp:painter:content:hard_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->ColorHardness);
		t->setHardText(buf);
	}
	if (CViewText *t = findText("ui:zp:painter:content:opac_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->ColorOpacity);
		t->setHardText(buf);
	}
	// Display-only color swatch (solid modulate on blank box)
	if (CViewBitmap *sw = dynamic_cast<CViewBitmap *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:painter:content:color_swatch")))
	{
		sw->setColor(NLMISC::CRGBA((uint8)b->ColorR, (uint8)b->ColorG, (uint8)b->ColorB, 255));
	}
	// Mask cycle button face = current mask basename or "none"
	if (CCtrlTextButton *btn = findTextButton("ui:zp:painter:content:btn_mask"))
	{
		const char *lab = b->BrushMaskLabel[0] ? b->BrushMaskLabel : "none";
		btn->setHardText(lab);
	}
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:btn_mask_mode"))
		btn->setPushed(b->BrushMaskMode);

	// Displace index 0-15
	if (CViewText *t = findText("ui:zp:painter:content:disp_info"))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->DisplaceIndex);
		t->setHardText(buf);
	}

	// Tiles palette selection highlight (ui M8) — stays in sync with keys/panel/pick
	syncPaletteHighlight(b->CurTileSet, b->TileSetCount);
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:btn_palette"))
		btn->setPushed(s_PaletteVisible);
}

void CEditorUI::update()
{
	if (!_Ready || !_Visible)
		return;
	syncPanelFromBridge();
	CWidgetManager *wm = CWidgetManager::getInstance();
	wm->sendClockTickEvent();
	wm->checkCoords();
}

void CEditorUI::draw()
{
	if (!_Ready || !_Visible)
		return;
	CWidgetManager::getInstance()->drawViews(NULL);
}

bool CEditorUI::wantsMouse() const
{
	if (!_Ready || !_Visible)
		return false;
	CWidgetManager *wm = CWidgetManager::getInstance();
	if (wm->getCapturePointerLeft() || wm->getCapturePointerRight())
		return true;
	if (wm->hasModal())
		return true;
	CViewPointerBase *pointer = wm->getPointer();
	if (!pointer)
		return false;
	sint32 x = 0, y = 0;
	pointer->getPointerPos(x, y);
	return wm->getWindowUnder(x, y) != NULL;
}

void CEditorUI::setVisible(bool visible)
{
	if (!_Ready)
		return;
	_Visible = visible;
	CWidgetManager::getInstance()->activateMasterGroup(MASTER_GROUP, _Visible);
}

void CEditorUI::toggleVisible()
{
	setVisible(!_Visible);
}

} // namespace ZPUI

/* end of file */
