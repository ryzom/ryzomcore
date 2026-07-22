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
#include "script_api.h"
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
#include <nel/gui/db_manager.h>
#include <nel/gui/event_listener.h>
#include <nel/gui/group_container.h>
#include <nel/gui/group_menu.h>
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

// M18b Prop panel handlers
class CAHZpPropRotate : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->propRotateDelta) return;
		int d = 0;
		fromString(params, d);
		b->propRotateDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropRotate, "zp_prop_rotate");

class CAHZpPropSymmetry : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propToggleSymmetry) b->propToggleSymmetry();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropSymmetry, "zp_prop_symmetry");

class CAHZpPropPassable : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propTogglePassable) b->propTogglePassable();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropPassable, "zp_prop_passable");

class CAHZpPropUseBBox : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propToggleUseBBox) b->propToggleUseBBox();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropUseBBox, "zp_prop_usebbox");

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

// painterscript window (ui M23b)
static const char *kScriptWinId = "ui:zp:script_win";

class CAHZpScriptToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CInterfaceGroup *win = dynamic_cast<CInterfaceGroup *>(
			CWidgetManager::getInstance()->getElementFromId(kScriptWinId));
		if (win)
		{
			win->setActive(!win->getActive());
			if (win->getActive())
				CWidgetManager::getInstance()->setTopWindow(win);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptToggle, "zp_script_toggle");

void setScriptWindowVisible(bool visible)
{
	CInterfaceGroup *win = dynamic_cast<CInterfaceGroup *>(
		CWidgetManager::getInstance()->getElementFromId(kScriptWinId));
	if (win)
	{
		win->setActive(visible);
		if (visible)
			CWidgetManager::getInstance()->setTopWindow(win);
	}
}

class CAHZpScriptRun : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
			CWidgetManager::getInstance()->getElementFromId(
				"ui:zp:script_win:content:ed_frame:code"));
		if (!eb) return;
		ZPSCRIPT::runString(eb->getInputString());
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptRun, "zp_script_run");

class CAHZpScriptRecord : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		ZPSCRIPT::setRecording(!ZPSCRIPT::isRecording());
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptRecord, "zp_script_record");

class CAHZpScriptCopyRec : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
			CWidgetManager::getInstance()->getElementFromId(
				"ui:zp:script_win:content:ed_frame:code"));
		if (!eb) return;
		std::string cur = eb->getInputString();
		if (!cur.empty() && cur[cur.size() - 1] != '\n') cur += "\n";
		eb->setInputString(cur + ZPSCRIPT::recorderText());
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptCopyRec, "zp_script_copyrec");

class CAHZpScriptClear : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		ZPSCRIPT::clearOutput();
		ZPSCRIPT::clearRecorder();
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptClear, "zp_script_clear");

class CAHZpToggleBoard : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleBoard)
			b->toggleBoard();
	}
};
REGISTER_ACTION_HANDLER(CAHZpToggleBoard, "zp_toggle_board");

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

/** M14c: open season picker modal listing available seasons (toolbar season button). */
class CAHZpSeasonMenu : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || b->SeasonCount < 1)
			return;
		if (b->seasonMenuFill)
			b->seasonMenuFill(NULL); // bridge shows/hides s0..s3 buttons
		CWidgetManager::getInstance()->enableModalWindow(pCaller, "ui:zp:season_menu");
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonMenu, "zp_season_menu");

/** M14c: pick a specific season code from the menu (params = sp|su|au|wi). */
class CAHZpSeasonSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->seasonSelect)
			return;
		if (!params.empty())
			b->seasonSelect(params);
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonSelect, "zp_season_select");

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

/** Absolute displace index from the palette grid (ui M9a) — same path as [ ] keys. */
class CAHZpDisplaceAbs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->displaceIndexAbs) return;
		int idx = 0;
		fromString(params, idx);
		b->displaceIndexAbs(idx);
	}
};
REGISTER_ACTION_HANDLER(CAHZpDisplaceAbs, "zp_displace_abs");

// Color picker (ui M9b)

static const char *kColorPickerWinId = "ui:zp:color_picker";
static bool s_ColorPickerVisible = false;

static void syncColorDbFromBridge()
{
	SPaintUIBridge *b = getPaintUIBridge();
	if (!b) return;
	if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:R", false))
		n->setValue32((sint32)b->ColorR);
	if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:G", false))
		n->setValue32((sint32)b->ColorG);
	if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:B", false))
		n->setValue32((sint32)b->ColorB);
	if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:A", false))
		n->setValue32(255);
	CInterfaceLink::updateAllLinks();
}

bool isColorPickerVisible()
{
	return s_ColorPickerVisible;
}

void setColorPickerVisible(bool visible)
{
	s_ColorPickerVisible = visible;
	if (CGroupContainer *c = dynamic_cast<CGroupContainer *>(
	        CWidgetManager::getInstance()->getElementFromId(kColorPickerWinId)))
		c->setActive(visible);
	else if (CInterfaceGroup *g = dynamic_cast<CInterfaceGroup *>(
	             CWidgetManager::getInstance()->getElementFromId(kColorPickerWinId)))
		g->setActive(visible);
	if (visible)
		syncColorDbFromBridge();
}

void toggleColorPicker()
{
	setColorPickerVisible(!s_ColorPickerVisible);
}

void forceShowColorPickerForShot()
{
	setColorPickerVisible(true);
}

class CAHZpOpenColorPicker : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		// Toggle: second click on the swatch closes it
		toggleColorPicker();
	}
};
REGISTER_ACTION_HANDLER(CAHZpOpenColorPicker, "zp_open_color_picker");

class CAHZpColorPicked : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->setBrushColor) return;
		sint32 r = 255, g = 255, bl = 255;
		if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:R", false))
			r = n->getValue32();
		if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:G", false))
			g = n->getValue32();
		if (CCDBNodeLeaf *n = CDBManager::getInstance()->getDbProp("UI:TEMP:COL:B", false))
			bl = n->getValue32();
		if (r < 0) r = 0; if (r > 255) r = 255;
		if (g < 0) g = 0; if (g > 255) g = 255;
		if (bl < 0) bl = 0; if (bl > 255) bl = 255;
		b->setBrushColor((int)r, (int)g, (int)bl);
	}
};
REGISTER_ACTION_HANDLER(CAHZpColorPicked, "zp_color_picked");

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
// Tileset + Displace palette (ui M8 / M9a)

static const sint32 kPaletteCellW = 96;
static const sint32 kPaletteCellH = 92;
static const sint32 kPaletteCols = 3;
static const sint32 kPaletteThumb = 64;
static const sint32 kPaletteHeaderH = 18;
static const sint32 kPaletteSectionGap = 10;
static const char *kPaletteBodyId = "ui:zp:tiles_palette:content:grid_host:body";
static const char *kPaletteGridId = "ui:zp:tiles_palette:content:grid_host:body:grid";
static const char *kPaletteDispGridId = "ui:zp:tiles_palette:content:grid_host:body:disp_grid";
static const char *kPaletteLblTilesId = "ui:zp:tiles_palette:content:grid_host:body:lbl_tiles";
static const char *kPaletteLblDispId = "ui:zp:tiles_palette:content:grid_host:body:lbl_disp";
static const char *kPaletteWinId = "ui:zp:tiles_palette";

static int s_PaletteBuiltCount = 0;
static int s_PaletteLastHighlight = -1;
static int s_DisplaceBuiltCount = 0;
static int s_DisplaceLastHighlight = -1;
static int s_DisplacePaletteTileset = -1;
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
	// Panel toggle button face (Tiles rollout, M22)
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId(
	            "ui:zp:roll_tiles:content:btn_palette")))
		btn->setPushed(visible);
}

void toggleTilesetPalette()
{
	setTilesetPaletteVisible(!s_PaletteVisible);
}

void scrollPaletteToDisplaceSection()
{
	CInterfaceGroup *body = findGroupEl(kPaletteBodyId);
	if (!body)
		return;
	CInterfaceElement *lbl = CWidgetManager::getInstance()->getElementFromId(kPaletteLblDispId);
	if (!lbl)
		return;
	const sint32 want = -lbl->getY() - 8;
	if (want > 0)
		body->setOfsY(want);
}

/** Spawn a palette cell under the grid (same idiom as the continent board). */
static CInterfaceGroup *spawnPaletteCell(CInterfaceGroup *parent,
                                         const char *templateName,
                                         const std::vector<std::pair<std::string, std::string> > &params,
                                         sint32 x, sint32 y)
{
	if (!parent || !templateName)
		return NULL;
	IParser *parser = CWidgetManager::getInstance()->getParser();
	CInterfaceGroup *g = NULL;
	if (!params.empty())
		g = parser->createGroupInstance(templateName, parent->getId(),
		                                &params[0], (uint)params.size());
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

/**
 * Resolve a displacement-map source for paint sub-index 0-15 via a tileset's
 * _DisplacementBitmap → bank.getDisplacementMap. Also returns bank-global map id.
 */
static std::string resolveDisplaceSource(NL3D::CTileBank *bank, int tileset, int subIdx,
                                         int *mapIdOut = NULL, std::string *storedNameOut = NULL)
{
	if (mapIdOut) *mapIdOut = -1;
	if (storedNameOut) storedNameOut->clear();
	if (!bank || subIdx < 0 || subIdx > 15)
		return std::string();
	if (bank->getTileSetCount() <= 0)
		return std::string();
	if (tileset < 0 || tileset >= bank->getTileSetCount())
		tileset = 0;
	const NL3D::CTileSet *ts = bank->getTileSet(tileset);
	if (!ts)
		return std::string();
	const uint mapId = ts->getDisplacementTile((NL3D::CTileSet::TDisplacement)subIdx);
	if (mapIdOut) *mapIdOut = (int)mapId;
	if (mapId >= bank->getDisplacementMapCount())
		return std::string();
	const char *name = bank->getDisplacementMap(mapId);
	if (!name || !*name)
		return std::string();
	std::string stored = name;
	for (size_t k = 0; k < stored.size(); ++k)
		if (stored[k] == '\\') stored[k] = '/';
	std::string base = CFile::getFilename(stored);
	if (storedNameOut)
		*storedNameOut = base;
	// Basename first (resolveBankTextures remaps + CPath search on tiles/displace dirs)
	std::string path = CPath::lookup(base, false, false);
	if (path.empty())
		path = CPath::lookup(stored, false, false);
	return path;
}

void rebuildTilesetPalette(NL3D::CTileBank *bank, const std::string &bankPath,
                           const std::string &seasonKey, int tilesetForDisplace)
{
	CInterfaceGroup *body = findGroupEl(kPaletteBodyId);
	CInterfaceGroup *grid = findGroupEl(kPaletteGridId);
	CInterfaceGroup *dispGrid = findGroupEl(kPaletteDispGridId);
	if (!grid)
	{
		// UI not ready / soft-fail path
		s_PaletteBuiltCount = 0;
		s_PaletteLastHighlight = -1;
		s_DisplaceBuiltCount = 0;
		s_DisplaceLastHighlight = -1;
		s_DisplacePaletteTileset = -1;
		return;
	}
	grid->clearGroups();
	if (dispGrid)
		dispGrid->clearGroups();
	s_PaletteBuiltCount = 0;
	s_PaletteLastHighlight = -1;
	s_DisplaceBuiltCount = 0;
	s_DisplaceLastHighlight = -1;
	s_DisplacePaletteTileset = -1;

	const int cols = kPaletteCols;
	sint32 tsH = kPaletteCellH;
	sint32 dispH = kPaletteCellH;
	sint nSets = 0;

	// Cache dirs on the NLGUI search path. CPath::addSearchPath (non-alternative) only
	// indexes files present *at call time* — previews written after that call are
	// invisible to createTexture/lookup, so cells render empty. We re-index after each
	// ensure* (CPath::addSearchFile) so cold-cache first paint still resolves thumbs.
	std::string tsCacheDir = ZPTHUMB::tilesetPreviewCacheDir();
	if (!tsCacheDir.empty())
		CPath::addSearchPath(tsCacheDir, false, false);
	std::string dispCacheDir = ZPTHUMB::displacePreviewCacheDir();
	if (!dispCacheDir.empty())
		CPath::addSearchPath(dispCacheDir, false, false);

	// ---- Tilesets ----
	uint withPreview = 0;
	uint nameOnly = 0;
	if (bank)
	{
		nSets = bank->getTileSetCount();
		if (nSets > 0)
		{
			const int rows = (nSets + cols - 1) / cols;
			tsH = rows * kPaletteCellH;
			grid->setW(cols * kPaletteCellW);
			grid->setH(tsH);
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
				char title[160];
				snprintf(title, sizeof(title), "%d %s", (int)(i + 1), setName.c_str());
				if (strlen(title) > 16)
				{
					title[13] = '.';
					title[14] = '.';
					title[15] = '.';
					title[16] = 0;
				}

				std::string thumbTex;
				if (!source.empty())
				{
					std::string cached;
					if (ZPTHUMB::ensureTilesetPreview(bankPath, (int)i, seasonKey, source, cached, kPaletteThumb)
					    && !cached.empty())
					{
						// Register newly written TGA so CPath::lookup finds it this frame.
						CPath::addSearchFile(cached);
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

				CInterfaceGroup *cell = spawnPaletteCell(grid, "zp_tileset_cell", p, x, y);
				if (cell)
				{
					if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(cell->getView("thumb")))
						thumb->setActive(!thumbTex.empty());
				}
			}
			s_PaletteBuiltCount = (int)nSets;
		}
		else
		{
			grid->setW(kPaletteCellW);
			grid->setH(kPaletteCellH);
		}
	}
	else
	{
		grid->setW(kPaletteCellW);
		grid->setH(kPaletteCellH);
	}

	// ---- Displace (paint indices 0-15 via tilesetForDisplace mapping) ----
	uint dispPreview = 0;
	uint dispNameOnly = 0;
	uint bankMapCount = bank ? bank->getDisplacementMapCount() : 0;
	int useTs = tilesetForDisplace;
	if (bank && bank->getTileSetCount() > 0)
	{
		if (useTs < 0 || useTs >= bank->getTileSetCount())
			useTs = 0;
	}
	else
		useTs = 0;

	if (dispGrid && bank)
	{
		const int nDisp = 16; // paint sub-noise 0..15
		const int drows = (nDisp + cols - 1) / cols;
		dispH = drows * kPaletteCellH;
		dispGrid->setW(cols * kPaletteCellW);
		dispGrid->setH(dispH);

		for (int i = 0; i < nDisp; ++i)
		{
			const int col = i % cols;
			const int row = i / cols;
			const sint32 x = col * kPaletteCellW;
			const sint32 y = -(row * kPaletteCellH);

			int mapId = -1;
			std::string storedName;
			std::string source = resolveDisplaceSource(bank, useTs, i, &mapId, &storedName);

			char title[160];
			// EmptyDisplacementMap is the bank sentinel for "no noise" (tile_bank.cpp setEmpty).
			// Also treat unresolved / missing filenames as empty for the label.
			bool isEmptyMap = storedName.empty();
			if (!isEmptyMap)
			{
				std::string stem = CFile::getFilenameWithoutExtension(storedName);
				// case-insensitive match on the magic sentinel name
				std::string lower = stem;
				for (size_t k = 0; k < lower.size(); ++k)
					if (lower[k] >= 'A' && lower[k] <= 'Z')
						lower[k] = (char)(lower[k] - 'A' + 'a');
				if (lower == "emptydisplacementmap")
					isEmptyMap = true;
			}
			if (!isEmptyMap)
			{
				// "3 falaisen…" — index + short stem
				std::string stem = CFile::getFilenameWithoutExtension(storedName);
				snprintf(title, sizeof(title), "%d %s", i, stem.c_str());
			}
			else
				snprintf(title, sizeof(title), "%d (empty)", i);
			if (strlen(title) > 16)
			{
				title[13] = '.';
				title[14] = '.';
				title[15] = '.';
				title[16] = 0;
			}

			std::string thumbTex;
			if (!source.empty() && mapId >= 0)
			{
				std::string cached;
				if (ZPTHUMB::ensureDisplacePreview(bankPath, mapId, source, cached, kPaletteThumb)
				    && !cached.empty())
				{
					// Register newly written TGA so CPath::lookup finds it this frame.
					// Without this, cold-cache first paint leaves displace cells empty
					// (review M10c: previews on disk, UI wells blank).
					CPath::addSearchFile(cached);
					thumbTex = CFile::getFilenameWithoutExtension(cached) + ".tga";
					++dispPreview;
				}
			}
			if (thumbTex.empty())
				++dispNameOnly;

			std::vector<std::pair<std::string, std::string> > p;
			char idbuf[32], idxbuf[16];
			snprintf(idbuf, sizeof(idbuf), "dp%03d", i);
			snprintf(idxbuf, sizeof(idxbuf), "%d", i);
			p.push_back(std::make_pair(std::string("id"), std::string(idbuf)));
			p.push_back(std::make_pair(std::string("title"), std::string(title)));
			p.push_back(std::make_pair(std::string("idx"), std::string(idxbuf)));
			p.push_back(std::make_pair(std::string("thumb"),
			                           thumbTex.empty() ? std::string("w_box_blank.tga") : thumbTex));

			CInterfaceGroup *cell = spawnPaletteCell(dispGrid, "zp_displace_cell", p, x, y);
			if (cell)
			{
				if (CViewBitmap *thumb = dynamic_cast<CViewBitmap *>(cell->getView("thumb")))
					thumb->setActive(!thumbTex.empty());
			}
		}
		s_DisplaceBuiltCount = nDisp;
		s_DisplacePaletteTileset = useTs;
	}
	else if (dispGrid)
	{
		dispGrid->setW(kPaletteCellW);
		dispGrid->setH(kPaletteCellH);
	}

	// ---- Layout body: Tilesets header + grid, then Displace header + grid ----
	const sint32 yTilesLbl = -2;
	const sint32 yTsGrid = -(kPaletteHeaderH);
	const sint32 yDispLbl = yTsGrid - tsH - kPaletteSectionGap;
	const sint32 yDispGrid = yDispLbl - kPaletteHeaderH;
	const sint32 bodyH = -yDispGrid + dispH + 8;

	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(kPaletteLblTilesId))
	{
		el->setY(yTilesLbl);
		el->setX(4);
	}
	grid->setX(0);
	grid->setY(yTsGrid);
	if (CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(kPaletteLblDispId))
	{
		el->setY(yDispLbl);
		el->setX(4);
	}
	if (dispGrid)
	{
		dispGrid->setX(0);
		dispGrid->setY(yDispGrid);
	}
	if (body)
	{
		body->setW(cols * kPaletteCellW);
		body->setH(bodyH > 0 ? bodyH : kPaletteCellH);
		body->setOfsX(0);
		body->setOfsY(0);
	}

	printf("tiles palette: %d sets (%u previews, %u name-only) season=%s cache=%s\n",
	       (int)nSets, withPreview, nameOnly,
	       seasonKey.empty() ? "auto" : seasonKey.c_str(),
	       tsCacheDir.c_str());
	printf("displace palette: 16 indices via tileset %d (%u previews, %u name-only); "
	       "bank maps=%u cache=%s\n",
	       useTs, dispPreview, dispNameOnly, bankMapCount, dispCacheDir.c_str());
	// Report which files the bank actually carries (basename list, truncated)
	if (bank && bankMapCount > 0)
	{
		printf("displace bank maps:");
		uint listed = 0;
		for (uint m = 0; m < bankMapCount && listed < 24; ++m)
		{
			const char *nm = bank->getDisplacementMap(m);
			if (!nm || !*nm)
				continue;
			printf(" %s", CFile::getFilename(nm).c_str());
			++listed;
		}
		if (bankMapCount > listed)
			printf(" …(+%u)", bankMapCount - listed);
		printf("\n");
	}
}

/**
 * Explorer-style cell selection fill (M10e).
 * Color matches NeL CGroupTree default col_select (255 128 128 128) — the w_ skin's
 * list-row selection tone. Applied as a modulated blank.tga button face behind the
 * texture well + label (same mechanism as the color swatch; not setPushed brick chrome).
 */
static const CRGBA kCellSelFill(255, 128, 128, 128);
static const CRGBA kCellSelHover(255, 128, 128, 64);
static const CRGBA kCellSelNone(0, 0, 0, 0);

static void setCellSelFill(CInterfaceGroup *cell, bool selected)
{
	if (!cell)
		return;
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(cell->getCtrl("btn")))
	{
		const CRGBA c = selected ? kCellSelFill : kCellSelNone;
		btn->setColor(c);
		btn->setColorPushed(c);
		btn->setColorOver(selected ? kCellSelFill : kCellSelHover);
	}
}

static void setCellSelFillByBtnPath(const char *btnPath, bool selected)
{
	if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId(btnPath)))
	{
		const CRGBA c = selected ? kCellSelFill : kCellSelNone;
		btn->setColor(c);
		btn->setColorPushed(c);
		btn->setColorOver(selected ? kCellSelFill : kCellSelHover);
	}
}

/** Highlight the current tileset cell (flat blank.tga fill via setColor, M10e). */
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
		setCellSelFillByBtnPath(idbuf, false);
	}
	if (tileSetCount && curTileSet >= 0 && curTileSet < (int)tileSetCount
	    && curTileSet < s_PaletteBuiltCount)
	{
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "%s:ts%03d:btn", kPaletteGridId, curTileSet);
		setCellSelFillByBtnPath(idbuf, true);
		s_PaletteLastHighlight = curTileSet;
	}
	else
		s_PaletteLastHighlight = -1;
}

/** Highlight the current displace cell (flat blank.tga fill via setColor, M10e). */
static void syncDisplaceHighlight(uint displaceIndex)
{
	if (s_DisplaceBuiltCount <= 0)
		return;
	const int cur = (int)(displaceIndex % 16);
	if (s_DisplaceLastHighlight == cur)
		return;
	// Clear previous
	if (s_DisplaceLastHighlight >= 0 && s_DisplaceLastHighlight < s_DisplaceBuiltCount)
	{
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "%s:dp%03d:btn", kPaletteDispGridId, s_DisplaceLastHighlight);
		setCellSelFillByBtnPath(idbuf, false);
	}
	if (cur >= 0 && cur < s_DisplaceBuiltCount)
	{
		char idbuf[96];
		snprintf(idbuf, sizeof(idbuf), "%s:dp%03d:btn", kPaletteDispGridId, cur);
		setCellSelFillByBtnPath(idbuf, true);
		s_DisplaceLastHighlight = cur;
	}
	else
		s_DisplaceLastHighlight = -1;
}

/**
 * When Displace mode is active, gently scroll the palette so the Displace header
 * is visible (optional M9a nicety). Does not pin it to the top so Tilesets stay
 * reachable; skips when ZONE_PAINTER_PALETTE_SHOT is set (headless shots want ofs 0
 * or an explicit displace focus via ZONE_PAINTER_SHOT_MODE alone is fine).
 */
static void emphasizeDisplaceSection(bool displaceActive)
{
	if (!s_PaletteVisible || !displaceActive)
		return;
	// Headless palette shots: leave scroll alone so both sections are reproducible.
	const char *palShot = getenv("ZONE_PAINTER_PALETTE_SHOT");
	if (palShot && palShot[0] && palShot[0] != '0')
		return;
	CInterfaceGroup *body = findGroupEl(kPaletteBodyId);
	if (!body)
		return;
	CInterfaceElement *lbl = CWidgetManager::getInstance()->getElementFromId(kPaletteLblDispId);
	if (!lbl)
		return;
	// Only nudge if the Displace label is currently below the visible viewport (~400px).
	const sint32 lblY = lbl->getY(); // negative from body top
	const sint32 ofs = body->getOfsY();
	const sint32 viewH = 400;
	// Visible range in body coords: [-ofs, -ofs-viewH]
	const sint32 visTop = -ofs;
	const sint32 visBot = -ofs - viewH;
	if (lblY < visBot + 40)
	{
		// Scroll just enough that the header is ~1/3 down the viewport
		const sint32 want = -lblY - viewH / 3;
		if (want > 0)
			body->setOfsY(want);
	}
	(void)visTop;
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
	// painterscript window sync (ui M23b): cheap length-compare before setHardText
	{
		static size_t lastRec = (size_t)-1, lastOut = (size_t)-1;
		const std::string &rec = ZPSCRIPT::recorderText();
		const std::string &out = ZPSCRIPT::outputText();
		if (rec.size() != lastRec)
		{
			lastRec = rec.size();
			if (CViewText *t = dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(
					"ui:zp:script_win:content:rec_frame:rec_text")))
				t->setHardText(rec);
		}
		if (out.size() != lastOut)
		{
			lastOut = out.size();
			if (CViewText *t = dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(
					"ui:zp:script_win:content:out_frame:out_text")))
				t->setHardText(out);
		}
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(
				"ui:zp:script_win:content:btn_rec")))
			btn->setPushed(ZPSCRIPT::isRecording());
	}

	SPaintUIBridge *b = getPaintUIBridge();
	if (!b || !b->HaveCore)
		return;

	// M22: Max-style rollout containers under the painter tree list.
	// Paths are top-level ids (ui:zp:roll_*) — tree attach keeps short ids under root.
	// Content widgets live at :content:<id>. Toolbar holds mode/season/undo (M14c).
	static const char *kRollSession = "ui:zp:roll_session";
	static const char *kRollTiles = "ui:zp:roll_tiles";
	static const char *kRollBrush = "ui:zp:roll_brush";
	static const char *kRollFill = "ui:zp:roll_fill";
	static const char *kRollDisplace = "ui:zp:roll_displace";
	static const char *kRollProperties = "ui:zp:roll_properties";
	static const char *kSessionC = "ui:zp:roll_session:content";
	static const char *kTilesC = "ui:zp:roll_tiles:content";
	static const char *kBrushC = "ui:zp:roll_brush:content";
	static const char *kFillC = "ui:zp:roll_fill:content";
	static const char *kDispC = "ui:zp:roll_displace:content";
	static const char *kPropC = "ui:zp:roll_properties:content";
	static const char *kPainterWin = "ui:zp:painter";
	static const char *kToolbar = "ui:zp:toolbar";
	static const char *kBtnPalette = "ui:zp:roll_tiles:content:btn_palette";

	// Ensure toolbar is visible once the viewer panel is live
	if (CInterfaceGroup *tb = findGroupEl(kToolbar))
	{
		if (!tb->getActive())
			tb->setActive(true);
	}

	// Mode radios on TOOLBAR (M14c + M18a PROP); panel radios removed
	// M15: buttons live under header_closed (gestionsets closed-bar idiom), not content
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_tile"))
		btn->setPushed(b->Mode == 0);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_color"))
		btn->setPushed(b->Mode == 1);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_displace"))
		btn->setPushed(b->Mode == 2);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_prop"))
		btn->setPushed(b->Mode == 3);

	// ---- M22: mode-gated rollout VISIBILITY (not collapse) ----
	// Non-applicable rollouts are fully hidden. Open/collapsed is remembered per
	// rollout across mode switches (setActive does not touch isOpen).
	//   Tile → Session+Tiles+Brush+Fill
	//   Color → Session+Brush+Fill
	//   Displace → Session+Displace+Brush+Fill
	//   Prop → Session+Properties
	const bool tileActive = (b->Mode == 0);
	const bool colorActive = (b->Mode == 1);
	const bool displaceActive = (b->Mode == 2);
	const bool propActive = (b->Mode == 3);
	const bool brushActive = tileActive || colorActive || displaceActive;
	const bool fillActive = brushActive; // paint-common fill tools
	if (CInterfaceGroup *g = findGroupEl(kRollSession))
		g->setActive(true);
	if (CInterfaceGroup *g = findGroupEl(kRollTiles))
		g->setActive(tileActive);
	if (CInterfaceGroup *g = findGroupEl(kRollBrush))
		g->setActive(brushActive);
	if (CInterfaceGroup *g = findGroupEl(kRollFill))
		g->setActive(fillActive);
	if (CInterfaceGroup *g = findGroupEl(kRollDisplace))
		g->setActive(displaceActive);
	if (CInterfaceGroup *g = findGroupEl(kRollProperties))
		g->setActive(propActive);

	// ZONE_PAINTER_ROLLOUTS=all: force-open every rollout once (full-column shot).
	// ZONE_PAINTER_ROLLOUTS=collapsed: close all except Session (header/arrow crop).
	// Does not change mode visibility; combine with SHOT_MODE for a mode's column.
	{
		static bool s_RolloutsEnvDone = false;
		const char *ra = getenv("ZONE_PAINTER_ROLLOUTS");
		if (ra && !s_RolloutsEnvDone)
		{
			s_RolloutsEnvDone = true;
			static const char *kAllRolls[] = {
				kRollSession, kRollTiles, kRollBrush, kRollFill, kRollDisplace, kRollProperties
			};
			const bool forceOpen = (strcmp(ra, "all") == 0 || strcmp(ra, "1") == 0);
			const bool forceCollapse = (strcmp(ra, "collapsed") == 0 || strcmp(ra, "closed") == 0);
			if (forceOpen || forceCollapse)
			{
				for (uint i = 0; i < sizeof(kAllRolls) / sizeof(kAllRolls[0]); ++i)
				{
					if (CGroupContainer *gc = dynamic_cast<CGroupContainer *>(
					        CWidgetManager::getInstance()->getElementFromId(kAllRolls[i])))
					{
						if (forceOpen)
						{
							if (!gc->isOpen())
								gc->open();
						}
						else if (forceCollapse)
						{
							// Keep Session open so the panel isn't an empty bar.
							const bool isSession = (strcmp(kAllRolls[i], kRollSession) == 0);
							if (isSession)
							{
								if (!gc->isOpen())
									gc->open();
							}
							else if (gc->isOpen())
								gc->close();
						}
					}
				}
			}
		}
	}

	// M20a: hide Tiles palette window while Prop is active; restore when leaving if
	// it was open (paint palette is meaningless over property edit).
	{
		static bool s_PropPaletteGate = false;
		static bool s_RestorePaletteAfterProp = false;
		if (propActive)
		{
			if (!s_PropPaletteGate)
			{
				s_PropPaletteGate = true;
				s_RestorePaletteAfterProp = s_PaletteVisible;
				if (s_PaletteVisible)
					setTilesetPaletteVisible(false);
			}
			else if (s_PaletteVisible)
			{
				// P key / stray open while still in Prop — keep suppressed
				setTilesetPaletteVisible(false);
			}
		}
		else if (s_PropPaletteGate)
		{
			s_PropPaletteGate = false;
			if (s_RestorePaletteAfterProp)
			{
				s_RestorePaletteAfterProp = false;
				setTilesetPaletteVisible(true);
			}
		}
	}

	// Parent painter: keep open/active. Height is owned by pop_min/max + resizer
	// (tree-list scroll when open rollouts exceed max H) — no mode-based setH.
	if (CInterfaceGroup *win = findGroupEl(kPainterWin))
	{
		if (!win->getActive())
			win->setActive(true);
		if (CGroupContainer *gc = dynamic_cast<CGroupContainer *>(win))
		{
			if (!gc->isOpen())
				gc->setOpen(true);
			gc->invalidateCoords();
		}
		else
			win->invalidateCoords();
	}

	// Tile set label (1-based index; unnamed sets show as (unnamed) not empty quotes)
	{
		char buf[192];
		const char *name = b->TileSetName;
		if (!name || !name[0])
			name = "(unnamed)";
		const int oneBased = b->TileSetCount ? (b->CurTileSet + 1) : 0;
		snprintf(buf, sizeof(buf), "%d/%u %s", oneBased, b->TileSetCount, name);
		if (CViewText *t = findText((std::string(kTilesC) + ":tileset_info").c_str()))
			t->setHardText(buf);
	}

	// 256 toggle
	if (CCtrlBaseButton *btn = findButton((std::string(kTilesC) + ":toggle_256").c_str()))
		btn->setPushed(b->Mode256);

	// Brush size — single readout in shared Brush rollout (M22)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->BrushSize);
		if (CViewText *t = findText((std::string(kBrushC) + ":brush_info").c_str()))
			t->setHardText(buf);
	}
	if (CViewText *t = findText((std::string(kTilesC) + ":group_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->TileGroup);
		t->setHardText(buf);
	}

	// Lock borders (Fill rollout); zp_checkbox_row :box
	if (CCtrlBaseButton *btn = findButton((std::string(kFillC) + ":lock_borders:box").c_str()))
		btn->setPushed(b->LockBorders);

	// Self-instance indicator (M4b) — Session rollout
	if (CViewText *t = findText((std::string(kSessionC) + ":instance_info").c_str()))
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

	// Season face on TOOLBAR (M14c): shows current season; frozen when <2 seasons
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:toolbar:header_closed:btn_season")))
	{
		const char *lab = b->SeasonLabel[0] ? b->SeasonLabel : "auto";
		// Uppercase face (SPRING / SUMMER / …)
		std::string up = NLMISC::toUpperAscii(lab);
		btn->setHardText(up);
		btn->setFrozen(b->SeasonCount < 1);
	}
	// Toolbar SAVE frozen when save unavailable
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:btn_save"))
		btn->setFrozen(!b->CanSave);

	// Multi-file dirty indicator (M6b): "N files, M dirty" — Session rollout
	if (CViewText *t = findText((std::string(kSessionC) + ":files_info").c_str()))
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

	// Color radius / hardness / opacity (shared Brush rollout)
	if (CViewText *t = findText((std::string(kBrushC) + ":radius_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%.1f", b->ColorRadius);
		t->setHardText(buf);
	}
	if (CViewText *t = findText((std::string(kBrushC) + ":hard_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->ColorHardness);
		t->setHardText(buf);
	}
	if (CViewText *t = findText((std::string(kBrushC) + ":opac_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->ColorOpacity);
		t->setHardText(buf);
	}
	// Clickable color swatch (M9b): button face modulated to the brush color
	{
		const NLMISC::CRGBA col((uint8)b->ColorR, (uint8)b->ColorG, (uint8)b->ColorB, 255);
		if (CCtrlBaseButton *sw = dynamic_cast<CCtrlBaseButton *>(
		        CWidgetManager::getInstance()->getElementFromId(
		            (std::string(kBrushC) + ":color_swatch").c_str())))
		{
			sw->setColor(col);
			sw->setColorPushed(col);
			sw->setColorOver(col);
		}
	}
	// RGB readout next to swatch
	if (CViewText *t = findText((std::string(kBrushC) + ":color_rgb").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u %u %u", b->ColorR, b->ColorG, b->ColorB);
		t->setHardText(buf);
	}
	// Track color-picker close-via-X
	{
		CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(kColorPickerWinId);
		if (el)
			s_ColorPickerVisible = el->getActive();
	}
	// Mask cycle button face = current mask basename or "none"
	if (CCtrlTextButton *btn = findTextButton((std::string(kBrushC) + ":btn_mask").c_str()))
	{
		const char *lab = b->BrushMaskLabel[0] ? b->BrushMaskLabel : "none";
		btn->setHardText(lab);
	}
	// M21a: btn_mask_mode is a zp_checkbox_row; canonical box is :box
	if (CCtrlBaseButton *btn = findButton((std::string(kBrushC) + ":btn_mask_mode:box").c_str()))
		btn->setPushed(b->BrushMaskMode);

	// Displace index 0-15
	if (CViewText *t = findText((std::string(kDispC) + ":disp_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->DisplaceIndex);
		t->setHardText(buf);
	}

	// Properties rollout (M18b / M22)
	if (propActive)
	{
		if (CViewText *t = findText((std::string(kPropC) + ":prop_name").c_str()))
			t->setHardText(b->PropHaveSelection
			                   ? (b->PropZoneName[0] ? b->PropZoneName : "(unnamed)")
			                   : "(no selection)");
		if (CViewText *t = findText((std::string(kPropC) + ":prop_file").c_str()))
			t->setHardText(b->PropHaveSelection ? b->PropFileBasename : "");
		if (CViewText *t = findText((std::string(kPropC) + ":prop_fp").c_str()))
			t->setHardText(b->PropHaveSelection ? b->PropFootprint : "");
		if (CViewText *t = findText((std::string(kPropC) + ":prop_meta").c_str()))
		{
			char buf[96];
			if (!b->PropHaveSelection)
				buf[0] = 0;
			else
				snprintf(buf, sizeof(buf), "%s%s",
				         b->PropEditable ? "editable" : "read-only",
				         b->PropDirty ? "  dirty" : "");
			t->setHardText(buf);
		}
		if (CViewText *t = findText((std::string(kPropC) + ":prop_rot").c_str()))
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", b->PropRotate);
			t->setHardText(buf);
		}
		// M21a: prop_* are zp_checkbox_row groups; bridge drives :box (+ freeze :lbl)
		if (CCtrlBaseButton *btn = findButton((std::string(kPropC) + ":prop_sym:box").c_str()))
			btn->setPushed(b->PropSymmetry);
		if (CCtrlBaseButton *btn = findButton((std::string(kPropC) + ":prop_pass:box").c_str()))
			btn->setPushed(b->PropPassable);
		if (CCtrlBaseButton *btn = findButton((std::string(kPropC) + ":prop_bbox:box").c_str()))
			btn->setPushed(b->PropUseBBox);
		if (CViewText *t = findText((std::string(kPropC) + ":prop_status").c_str()))
			t->setHardText(b->PropStatus);
		// Freeze steppers + checkbox rows when no selection / read-only
		const bool fr = !b->PropHaveSelection || !b->PropEditable;
		if (CCtrlBaseButton *btn = findButton((std::string(kPropC) + ":rot_down").c_str()))
			btn->setFrozen(fr);
		if (CCtrlBaseButton *btn = findButton((std::string(kPropC) + ":rot_up").c_str()))
			btn->setFrozen(fr);
		// Box + label hit both frozen; caption text dimmed so disabled is obvious
		const char *propRows[] = { "prop_sym", "prop_pass", "prop_bbox" };
		for (uint i = 0; i < sizeof(propRows) / sizeof(propRows[0]); ++i)
		{
			const std::string base = std::string(kPropC) + ":" + propRows[i];
			if (CCtrlBaseButton *btn = findButton((base + ":box").c_str()))
				btn->setFrozen(fr);
			if (CCtrlBaseButton *btn = findButton((base + ":lbl").c_str()))
				btn->setFrozen(fr);
			if (CViewText *t = findText((base + ":t").c_str()))
				t->setColor(fr ? CRGBA(160, 160, 160, 140) : CRGBA(255, 255, 255, 220));
		}
	}

	// Tiles palette selection highlight (ui M8) — stays in sync with keys/panel/pick
	syncPaletteHighlight(b->CurTileSet, b->TileSetCount);
	// Displace palette highlight (ui M9a)
	syncDisplaceHighlight(b->DisplaceIndex);
	// Track close-via-X: container may deactivate without going through togglePalette
	{
		CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(kPaletteWinId);
		if (el)
			s_PaletteVisible = el->getActive();
	}
	if (CCtrlBaseButton *btn = findButton(kBtnPalette))
		btn->setPushed(s_PaletteVisible);
	// Optional: auto-scroll palette toward Displace when that mode is active
	emphasizeDisplaceSection(displaceActive);
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
