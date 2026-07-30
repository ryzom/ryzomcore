/**
 * \file editor_ui.cpp
 * \brief In-engine NLGUI facade for the standalone zone painter
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
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

#include "editor_ui.h"
#include "script_api.h"
#include "startup_ui.h" // session-board drag hooks (pointer listener feeds them)
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
#include <nel/misc/time_nl.h>

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
	bool _DragCopyMod;
	virtual void operator()(const NLMISC::CEvent &event) NL_OVERRIDE
	{
		CViewPointerBase *pointer = CWidgetManager::getInstance()->getPointer();
		if (!pointer)
			return;
		if (event == EventMouseDownId)
		{
			CEventMouseDown &em = (CEventMouseDown &)event;
			_DownButtons |= em.Button & (leftButton | middleButton | rightButton);
			// Mask, not equality: Button carries OR'd modifier bits (ctrl/shift), so a
			// modifier-held left press failed an == test and never registered pointer-down.
			pointer->setPointerDown((em.Button & leftButton) != 0);
			pointer->setPointerMiddleDown((em.Button & middleButton) != 0);
			pointer->setPointerRightDown((em.Button & rightButton) != 0);
			// Arm a session-board drag from the cell under the pointer. Latch the
			// copy modifier HERE: the X11 emitter posts bare buttons on mouse-up (only
			// Windows carries modifiers there), so Ctrl/Shift must be read at down.
			// Call unconditionally: the hook itself disarms while a script executes
			// (a guard out here would leave a stale arm across a pumped script).
			if (em.Button & leftButton)
			{
				_DragCopyMod = (em.Button & (ctrlButton | shiftButton)) != 0;
				sessionBoardDragBegin();
			}
		}
		else if (event == EventMouseUpId)
		{
			CEventMouseUp &em = (CEventMouseUp &)event;
			// Drop before the button-state mirror clears (Ctrl/Shift = copy;
			// down-latched modifier OR'd with the up event's for platforms that send it)
			if (em.Button & leftButton)
				sessionBoardDragEnd(_DragCopyMod
				                    || (em.Button & (ctrlButton | shiftButton)) != 0);
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
			// The board drag arm must reset with the button state: the up event that
			// would pair with it never arrives after a focus loss (alt-tab mid-drag),
			// and a later stray up would fire a phantom drag from the stale cell.
			sessionBoardDragCancel();
		}
	}

public:
	CPointerButtonListener() : _DownButtons(0), _DragCopyMod(false) {}
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
	virtual void execute(CCtrlBase *pCaller, const std::string &params) NL_OVERRIDE
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
	virtual void execute(CCtrlBase *pCaller, const std::string &params) NL_OVERRIDE
	{
		std::string group = getParam(params, "group");
		CWidgetManager::getInstance()->enableModalWindow(pCaller, group);
	}
};
REGISTER_ACTION_HANDLER(CAHEnterModal, "enter_modal");

class CAHLeaveModal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHLeaveModal, "leave_modal");

class CAHSubmitQuickHelp : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE {}
};
REGISTER_ACTION_HANDLER(CAHSubmitQuickHelp, "submit_quick_help");

// ---------------------------------------------------------------------------------------------
// Paint action handlers: thin wrappers over the bridge (no second op implementation)

class CAHZpMode : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectMode) return;
		int mode = 0;
		fromString(params, mode);
		b->selectMode(mode);
	}
};
REGISTER_ACTION_HANDLER(CAHZpMode, "zp_mode");

// Prop panel handlers
class CAHZpPropRotate : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propToggleSymmetry) b->propToggleSymmetry();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropSymmetry, "zp_prop_symmetry");

class CAHZpPropPassable : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propTogglePassable) b->propTogglePassable();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropPassable, "zp_prop_passable");

class CAHZpPropUseBBox : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->propToggleUseBBox) b->propToggleUseBBox();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPropUseBBox, "zp_prop_usebbox");

// Patch rollout handlers
/** Sub-object level button: toggle-off - clicking the pressed level returns to
 * Object level. The level itself comes from the button's params. */
class CAHZpSubObject : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectSubObject) return;
		int level = 0;
		fromString(params, level);
		b->selectSubObject(b->SubObj == level ? 0 : level);
	}
};
REGISTER_ACTION_HANDLER(CAHZpSubObject, "zp_sub_object");

class CAHZpPatchBind : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchBind) b->patchBind();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchBind, "zp_patch_bind");

class CAHZpPatchUnbind : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchUnbind) b->patchUnbind();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchUnbind, "zp_patch_unbind");

class CAHZpPatchNoSmooth : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchNoSmooth) b->patchNoSmooth();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchNoSmooth, "zp_patch_nosmooth");

/** Selection-block checkboxes: pick filters and Lock Handles. */
class CAHZpPatchFilterVerts : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchFilterVertsToggle) b->patchFilterVertsToggle();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchFilterVerts, "zp_patch_filter_verts");

class CAHZpPatchFilterVecs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchFilterVecsToggle) b->patchFilterVecsToggle();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchFilterVecs, "zp_patch_filter_vecs");

class CAHZpPatchLockHandles : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchLockHandlesToggle) b->patchLockHandlesToggle();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchLockHandles, "zp_patch_lock_handles");

class CAHZpPatchArrows : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->arrowsToggle) b->arrowsToggle();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchArrows, "zp_patch_arrows");

/** Surface Properties: one smoothing-group grid button; params = the bit "0".."31". */
class CAHZpPatchSmGroup : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		int bit = 0;
		NLMISC::fromString(params, bit);
		if (b && b->patchSmGroup) b->patchSmGroup(bit);
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchSmGroup, "zp_patch_smgroup");

class CAHZpPatchSmGroupClear : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchSmGroupClear) b->patchSmGroupClear();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchSmGroupClear, "zp_patch_smgroup_clear");

/** Tess steppers; params = "u+", "u-", "v+", "v-". */
class CAHZpPatchTess : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->patchTessDelta || params.size() < 2) return;
		const int axis = params[0] == 'v' ? 1 : 0;
		b->patchTessDelta(axis, params[1] == '-' ? -1 : 1);
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchTess, "zp_patch_tess");

class CAHZpPatchBalance : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchBalance) b->patchBalance();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchBalance, "zp_patch_balance");

class CAHZpPatchDelete : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchDelete) b->patchDelete();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchDelete, "zp_patch_delete");

/** Turn the selected patches; params "ccw" / "cw". */
class CAHZpPatchTurn : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		if (params == "cw") { if (b->patchTurnCw) b->patchTurnCw(); }
		else { if (b->patchTurnCcw) b->patchTurnCcw(); }
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchTurn, "zp_patch_turn");

class CAHZpPatchSubdivide : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchSubdivide) b->patchSubdivide();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchSubdivide, "zp_patch_subdivide");

/** Panel Weld button: pop the distance dialog, seeded with the last-used threshold. */
class CAHZpPatchWeld : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		if (CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
		        CWidgetManager::getInstance()->getElementFromId(
		            "ui:zp:weld_dialog:content:dist_frame:dist")))
			eb->setInputString(NLMISC::toString("%g", b->WeldThreshold));
		CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:weld_dialog");
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchWeld, "zp_patch_weld");

/** Weld dialog OK: parse the distance and weld the vertex selection at it. */
class CAHZpWeldOk : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->patchWeldThreshold) return;
		float t = 0.f;
		if (CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
		        CWidgetManager::getInstance()->getElementFromId(
		            "ui:zp:weld_dialog:content:dist_frame:dist")))
			NLMISC::fromString(eb->getInputString(), t);
		if (t <= 0.f)
			return; // an unparsable or zero distance welds nothing; the dialog stays up
		CWidgetManager::getInstance()->disableModalWindow();
		b->patchWeldThreshold(t);
	}
};
REGISTER_ACTION_HANDLER(CAHZpWeldOk, "zp_weld_ok");

class CAHZpWeldCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHZpWeldCancel, "zp_weld_cancel");

/** Panel Extrude button: pop the height dialog, seeded with the last-used height. */
class CAHZpPatchExtrude : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		if (CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
		        CWidgetManager::getInstance()->getElementFromId(
		            "ui:zp:extrude_dialog:content:dist_frame:dist")))
			eb->setInputString(NLMISC::toString("%g", b->ExtrudeHeight));
		CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:extrude_dialog");
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchExtrude, "zp_patch_extrude");

/** Extrude dialog OK: parse the height and extrude the face selection by it. */
class CAHZpExtrudeOk : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->patchExtrude) return;
		float h = 0.f;
		if (CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
		        CWidgetManager::getInstance()->getElementFromId(
		            "ui:zp:extrude_dialog:content:dist_frame:dist")))
			NLMISC::fromString(eb->getInputString(), h);
		if (h == 0.f)
			return; // an unparsable or zero height extrudes nothing; the dialog stays up
		CWidgetManager::getInstance()->disableModalWindow();
		b->patchExtrude(h);
	}
};
REGISTER_ACTION_HANDLER(CAHZpExtrudeOk, "zp_extrude_ok");

class CAHZpExtrudeCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHZpExtrudeCancel, "zp_extrude_cancel");

class CAHZpPatchAddQuad : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchAddQuad) b->patchAddQuad();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchAddQuad, "zp_patch_add_quad");

class CAHZpPatchDetach : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchDetach) b->patchDetach();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchDetach, "zp_patch_detach");

class CAHZpTileSet : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectTileSetDelta) return;
		int d = 0;
		fromString(params, d);
		b->selectTileSetDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpTileSet, "zp_tileset");

/** Absolute tile-set select from the palette grid (same zpSelectTileSetAbs path). */
class CAHZpTileSetAbs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->togglePalette)
			b->togglePalette();
		else
			toggleTilesetPalette();
	}
};
REGISTER_ACTION_HANDLER(CAHZpTogglePalette, "zp_toggle_palette");

// painterscript window
static const char *kScriptWinId = "ui:zp:script_win";

class CAHZpScriptToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
			CWidgetManager::getInstance()->getElementFromId(
				"ui:zp:script_win:content:ed_frame:code"));
		if (!eb) return;
		// QUEUED, not run inline: this handler executes inside EventServer::pump, and a
		// script calling painter.pumpUI() would re-enter the pump (nlassert in debug,
		// event-list UAF in release). The viewer loop runs it right after the pump.
		ZPSCRIPT::queueRunString(eb->getInputString());
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptRun, "zp_script_run");

class CAHZpScriptRecord : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		ZPSCRIPT::setRecording(!ZPSCRIPT::isRecording());
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptRecord, "zp_script_record");

class CAHZpScriptCopyRec : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		ZPSCRIPT::clearOutput();
		ZPSCRIPT::clearRecorder();
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptClear, "zp_script_clear");

// Deliberately NOT execution-guarded: this is the one control that must stay live while
// a pumped script runs (cancel button; ESC via the pump is the keyboard equivalent).
class CAHZpScriptCancel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		ZPSCRIPT::requestCancel();
	}
};
REGISTER_ACTION_HANDLER(CAHZpScriptCancel, "zp_script_cancel");

class CAHZpToggleBoard : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleBoard)
			b->toggleBoard();
	}
};
REGISTER_ACTION_HANDLER(CAHZpToggleBoard, "zp_toggle_board");

class CAHZpToggle256 : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleTileSize) b->toggleTileSize();
	}
};
REGISTER_ACTION_HANDLER(CAHZpToggle256, "zp_toggle_256");

class CAHZpBrush : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleLockBorders) b->toggleLockBorders();
	}
};
REGISTER_ACTION_HANDLER(CAHZpLockBorders, "zp_lock_borders");

class CAHZpUndo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->undo) b->undo();
	}
};
REGISTER_ACTION_HANDLER(CAHZpUndo, "zp_undo");

class CAHZpRedo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->redo) b->redo();
	}
};
REGISTER_ACTION_HANDLER(CAHZpRedo, "zp_redo");

class CAHZpFill : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->seasonNext && b->SeasonCount > 1)
			b->seasonNext();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonNext, "zp_season_next");

/** Open season picker modal listing available seasons (toolbar season button). */
class CAHZpSeasonMenu : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || b->SeasonCount < 1)
			return;
		if (b->seasonMenuFill)
			b->seasonMenuFill(NULL); // bridge shows/hides s0..s3 buttons
		CWidgetManager::getInstance()->enableModalWindow(pCaller, "ui:zp:season_menu");
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonMenu, "zp_season_menu");

/** Open the pivot-point picker (toolbar pivot button, and the scene context menu). */
class CAHZpPivotMenu : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->enableModalWindow(pCaller, "ui:zp:pivot_menu");
	}
};
REGISTER_ACTION_HANDLER(CAHZpPivotMenu, "zp_pivot_menu");

/** Choose a pivot mode (params = the TPivotMode ordinal). */
class CAHZpPivotSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->selectPivotMode)
			return;
		b->selectPivotMode(atoi(params.c_str()));
	}
};
REGISTER_ACTION_HANDLER(CAHZpPivotSelect, "zp_pivot_select");

/** Drop the user pivot at the selection's centre - the only way that mode's point is set. */
class CAHZpPivotToSelection : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->userPivotToSelection)
			return;
		b->userPivotToSelection();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPivotToSelection, "zp_pivot_to_selection");

/** Open the scene context menu (right click on empty space in patch mode). */
class CAHZpSceneMenu : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		CWidgetManager::getInstance()->enableModalWindow(pCaller, "ui:zp:scene_menu");
	}
};
REGISTER_ACTION_HANDLER(CAHZpSceneMenu, "zp_scene_menu");

class CAHZpPatchElement : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchElement) b->patchElement();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchElement, "zp_patch_element");

class CAHZpPatchWeldTarget : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->weldTargetToggle) b->weldTargetToggle();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchWeldTarget, "zp_patch_weld_target");

/** Scene-menu compass: move the patch selection to the neighbor zone (params = 0..7). */
class CAHZpMoveToZone : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->moveToZoneDir)
			b->moveToZoneDir(atoi(params.c_str()));
	}
};
REGISTER_ACTION_HANDLER(CAHZpMoveToZone, "zp_move_to_zone");


class CAHZpPatchHide : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchHide) b->patchHide();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchHide, "zp_patch_hide");

class CAHZpPatchUnhide : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchUnhideAll) b->patchUnhideAll();
	}
};
REGISTER_ACTION_HANDLER(CAHZpPatchUnhide, "zp_patch_unhide");

/** Scene-menu vertex type pair: set the vertex selection Coplanar (1) or Corner (0). */
class CAHZpVertexType : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return;
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->patchVertCoplanar)
			b->patchVertCoplanar(atoi(params.c_str()));
	}
};
REGISTER_ACTION_HANDLER(CAHZpVertexType, "zp_vertex_type");

/** Pick a specific season code from the menu (params = sp|su|au|wi). */
class CAHZpSeasonSelect : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		CWidgetManager::getInstance()->disableModalWindow();
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->seasonSelect)
			return;
		if (!params.empty())
			b->seasonSelect(params);
	}
};
REGISTER_ACTION_HANDLER(CAHZpSeasonSelect, "zp_season_select");

// Color / displace panel: thin wrappers over bridge (same as keys)

class CAHZpColorRadius : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->cycleBrushMask) b->cycleBrushMask();
	}
};
REGISTER_ACTION_HANDLER(CAHZpCycleMask, "zp_cycle_mask");

class CAHZpMaskMode : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->toggleMaskMode) b->toggleMaskMode();
	}
};
REGISTER_ACTION_HANDLER(CAHZpMaskMode, "zp_mask_mode");

class CAHZpDisplace : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->displaceIndexDelta) return;
		int d = 0;
		fromString(params, d);
		b->displaceIndexDelta(d);
	}
};
REGISTER_ACTION_HANDLER(CAHZpDisplace, "zp_displace");

/** Absolute displace index from the palette grid (same path as [ ] keys). */
class CAHZpDisplaceAbs : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->displaceIndexAbs) return;
		int idx = 0;
		fromString(params, idx);
		b->displaceIndexAbs(idx);
	}
};
REGISTER_ACTION_HANDLER(CAHZpDisplaceAbs, "zp_displace_abs");

// Color picker

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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		toggleColorPicker();
	}
};
REGISTER_ACTION_HANDLER(CAHZpOpenColorPicker, "zp_open_color_picker");

class CAHZpColorPicked : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b || !b->CanSave) return;
		// Board sessions: toolbar SAVE is one-click save-all (each dirty file in
		// place, one-time .bak, per-file thumbnails always). Per-file save-as with custom
		// options lives on the board cells. Legacy interactive single-file keeps the
		// modal (no board to reach save-as from); CLI --save keeps direct save.
		if (b->InteractiveSave && b->BoardSession)
		{
			if (b->saveOverwrite)
				b->saveOverwrite();
		}
		else if (b->InteractiveSave)
			openSaveDialog();
		else if (b->save)
			b->save();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSave, "zp_save");

// ---------------------------------------------------------------------------------------------
// Save modal (interactive flow only)

static bool s_SaveCopyConfirm = false;
// The resolved target the armed confirm applies to: retyping the name must re-arm,
// or a confirm meant for file A silently overwrites file B.
static std::string s_SaveCopyTarget;

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
	s_SaveCopyTarget.clear();
	if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_copy"))
		btn->setHardText("Save copy");
}

// Which editable file the save dialog targets; empty = legacy whole-session form
// (single-file interactive without a board). The board cell "Save as…" sets it.
static std::string s_SaveDialogFile;

static void openSaveDialogCommon(SPaintUIBridge *b, const std::string &prefillBase)
{
	resetSaveCopyButtonLabel();
	// prefillBase may be a caller-provided basename that still carries its ".max"
	// extension (e.g. the per-file board "Save as..." path passes the file-identity
	// basename verbatim) or an already-stripped stem (EditableBasename). Strip either
	// way so the suggested copy name is never "name.max_painted.max".
	std::string prefill = CFile::getFilenameWithoutExtension(prefillBase) + "_painted.max";
	if (CGroupEditBox *eb = findEditBox("ui:zp:save_dialog:content:copy_frame:copy_name"))
		eb->setInputString(prefill);
	// Thumbnail checkbox defaults on; --no-thumbnail hides the row entirely
	// (the kill-switch drops every SI write, a live checked box would be a lie).
	b->UpdateThumbnail = !b->ThumbnailsDisabled;
	if (CCtrlBaseButton *tb = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb:box")))
		tb->setPushed(b->UpdateThumbnail);
	if (CInterfaceGroup *row = dynamic_cast<CInterfaceGroup *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb")))
		row->setActive(!b->ThumbnailsDisabled);
	CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:save_dialog");
}

void openSaveDialog()
{
	SPaintUIBridge *b = getPaintUIBridge();
	if (!b) return;
	s_SaveDialogFile.clear();
	// Multi-file: Overwrite is save-all of dirty editables
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
	openSaveDialogCommon(b, std::string(b->EditableBasename));
}

void openSaveDialogForFile(const std::string &basename)
{
	SPaintUIBridge *b = getPaintUIBridge();
	if (!b) return;
	s_SaveDialogFile = basename;
	setSaveModalStatus("Save '" + basename + "'");
	if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_overwrite"))
		btn->setHardText("Overwrite");
	openSaveDialogCommon(b, basename);
}

class CAHZpSaveThumbToggle : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		if (b->ThumbnailsDisabled) return; // row hidden; keep the forced-off state
		CCtrlBaseButton *box = dynamic_cast<CCtrlBaseButton *>(
			CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb:box"));
		// The zp_checkbox_row LABEL fires this same handler but does not flip the box's
		// visual state: read-only handling made caption clicks dead (no per-frame sync
		// writes this modal's box back like the bridge-backed rows get). Box click: the
		// toggle already flipped, read it. Label click: flip state, write the box.
		if (box && pCaller == box)
			b->UpdateThumbnail = box->getPushed();
		else
		{
			b->UpdateThumbnail = !b->UpdateThumbnail;
			if (box)
				box->setPushed(b->UpdateThumbnail);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHZpSaveThumbToggle, "zp_save_thumb_toggle");

void forceShowSaveDialogForShot()
{
	openSaveDialog();
}

// ---------------------------------------------------------------------------------------------
// Tileset + Displace palette

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
	// indexes files present *at call time*; previews written after that call are
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
				// "3 falaisen…" = index + short stem
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
					// Without this, cold-cache first paint leaves displace cells empty.
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
 * Explorer-style cell selection fill.
 * Color matches NeL CGroupTree default col_select (255 128 128 128), the w_ skin's
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

/** Highlight the current tileset cell (flat blank.tga fill via setColor). */
static void syncPaletteHighlight(int curTileSet, uint tileSetCount)
{
	if (s_PaletteBuiltCount <= 0)
		return;
	if (s_PaletteLastHighlight == curTileSet)
		return;
	CInterfaceGroup *grid = findGroupEl(kPaletteGridId);
	if (!grid)
		return;
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

/** Highlight the current displace cell (flat blank.tga fill via setColor). */
static void syncDisplaceHighlight(uint displaceIndex)
{
	if (s_DisplaceBuiltCount <= 0)
		return;
	const int cur = (int)(displaceIndex % 16);
	if (s_DisplaceLastHighlight == cur)
		return;
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
 * is visible. Does not pin it to the top so Tilesets stay reachable; skips when
 * ZONE_PAINTER_PALETTE_SHOT is set (headless shots want ofs 0).
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
	if (b->ThumbnailsDisabled)
	{
		b->UpdateThumbnail = false; // hidden row's stale pushed state must not resurrect it
		return;
	}
	if (CCtrlBaseButton *tb = dynamic_cast<CCtrlBaseButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:save_dialog:content:update_thumb:box")))
		b->UpdateThumbnail = tb->getPushed();
}

class CAHZpSaveOverwrite : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		syncThumbWantFromModal(b);
		// File-bound form (board "Save as…") overwrites that ONE file
		bool ok;
		if (!s_SaveDialogFile.empty() && b->saveFileOverwrite)
			ok = b->saveFileOverwrite(s_SaveDialogFile, b->UpdateThumbnail);
		else if (b->saveOverwrite)
			ok = b->saveOverwrite();
		else
			return;
		if (ok)
		{
			setSaveModalStatus("Saved (overwrite).");
			s_SaveDialogFile.clear(); // drop the per-file binding; a reopened dialog must rebind
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		SPaintUIBridge *b = getPaintUIBridge();
		if (!b) return;
		// Bound form needs saveFileCopy; plain form needs saveTo. Never fall through to a
		// null (or the wrong) callback when only one side is wired.
		if (!s_SaveDialogFile.empty())
		{
			if (!b->saveFileCopy) return;
		}
		else if (!b->saveTo)
		{
			return;
		}

		std::string name;
		if (CGroupEditBox *eb = findEditBox("ui:zp:save_dialog:content:copy_frame:copy_name"))
			name = eb->getInputString();
		while (!name.empty() && (name[0] == ' ' || name[0] == '\t')) name.erase(0, 1);
		while (!name.empty() && (name[name.size() - 1] == ' ' || name[name.size() - 1] == '\t'))
			name.resize(name.size() - 1);
		if (name.empty())
		{
			setSaveModalStatus("Enter a file name.");
			return;
		}
		// Absolute, or relative to the saved FILE's own directory. For the bound form
		// that is the bound file's dir (same base saveFileCopy resolves against; the
		// exists-check below must agree with the actual write target), else InputDir.
		// isAbsolutePath covers Windows drive-letter paths (a bare '/' check does not).
		std::string target;
		if (CPath::isAbsolutePath(name))
			target = name;
		else
		{
			std::string dir = b->InputDir;
			if (!s_SaveDialogFile.empty() && b->fileDir)
			{
				std::string d = b->fileDir(s_SaveDialogFile);
				if (!d.empty())
					dir = d;
			}
			if (!dir.empty() && dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
				dir += "/";
			target = dir + name;
		}
		// getExtension searches the full path for the last '.'; a directory component
		// like "user.name" would look like an extension and skip appending ".max".
		// Only the filename matters for the auto-extension rule.
		if (toLowerAscii(CFile::getExtension(CFile::getFilename(target))).empty())
			target += ".max";

		if (CFile::fileExists(target) && (!s_SaveCopyConfirm || target != s_SaveCopyTarget))
		{
			s_SaveCopyConfirm = true;
			s_SaveCopyTarget = target;
			if (CCtrlTextButton *btn = findTextButton("ui:zp:save_dialog:content:btn_copy"))
				btn->setHardText("Confirm overwrite");
			setSaveModalStatus("File exists. Click again to overwrite.");
			return;
		}

		syncThumbWantFromModal(b);
		// File-bound form (board "Save as…") copies that ONE file, checkbox honored.
		// Pass the RESOLVED absolute target: the exists-check above ran on it, so the
		// write must hit the same path (a relative name re-resolved elsewhere may not).
		bool ok;
		if (!s_SaveDialogFile.empty())
			ok = b->saveFileCopy(s_SaveDialogFile, target, b->UpdateThumbnail);
		else
			ok = b->saveTo(target);
		if (ok)
		{
			setSaveModalStatus("Saved copy.");
			s_SaveDialogFile.clear(); // drop the per-file binding; a reopened dialog must rebind
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
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */) NL_OVERRIDE
	{
		if (ZPSCRIPT::isExecuting()) return; // pumped script: UI locked (CANCEL only)
		resetSaveCopyButtonLabel();
		setSaveModalStatus("");
		s_SaveDialogFile.clear(); // drop the per-file binding
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

	// Software cursor: CViewPointer draws the shape from the atlas as ordinary GUI
	// geometry, so the resize / pick shapes look and behave the same on every platform -
	// including WebGL, where the driver has no native cursor to hand a bitmap to. The
	// system cursor is hidden so the two do not draw on top of each other; setVisible()
	// and shutdown() put it back, or hiding the GUI would leave no cursor at all.
	CViewPointer::setHWMouse(false);
	_Driver->showCursor(false);

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
	// Hand the pointer back to the system before the GUI stops drawing one.
	if (_Ready && _Driver)
		_Driver->showCursor(true);
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
	// Painterscript window sync: generation counters, not text length. A same-length
	// content change (CLEAR + same-length line in one frame) would leave stale text.
	{
		static uint lastRec = (uint)-1, lastOut = (uint)-1;
		if (ZPSCRIPT::recorderGeneration() != lastRec)
		{
			lastRec = ZPSCRIPT::recorderGeneration();
			if (CViewText *t = dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(
					"ui:zp:script_win:content:rec_frame:rec_text")))
				t->setHardText(ZPSCRIPT::recorderText());
		}
		if (ZPSCRIPT::outputGeneration() != lastOut)
		{
			lastOut = ZPSCRIPT::outputGeneration();
			if (CViewText *t = dynamic_cast<CViewText *>(CWidgetManager::getInstance()->getElementFromId(
					"ui:zp:script_win:content:out_frame:out_text")))
				t->setHardText(ZPSCRIPT::outputText());
		}
		if (CCtrlBaseButton *btn = dynamic_cast<CCtrlBaseButton *>(CWidgetManager::getInstance()->getElementFromId(
				"ui:zp:script_win:content:btn_rec")))
			btn->setPushed(ZPSCRIPT::isRecording());
	}

	SPaintUIBridge *b = getPaintUIBridge();
	if (!b || !b->HaveCore)
		return;

	// rollout containers under the painter tree list.
	// Paths are top-level ids (ui:zp:roll_*); tree attach keeps short ids under root.
	// Content widgets live at :content:<id>. Toolbar holds mode/season/undo.
	static const char *kRollSession = "ui:zp:roll_session";
	static const char *kRollTiles = "ui:zp:roll_tiles";
	static const char *kRollBrush = "ui:zp:roll_brush";
	static const char *kRollFill = "ui:zp:roll_fill";
	static const char *kRollDisplace = "ui:zp:roll_displace";
	static const char *kRollProperties = "ui:zp:roll_properties";
	static const char *kRollPatch = "ui:zp:roll_patch";
	static const char *kPatchC = "ui:zp:roll_patch:content";
	static const char *kSessionC = "ui:zp:roll_session:content";
	static const char *kTilesC = "ui:zp:roll_tiles:content";
	static const char *kBrushC = "ui:zp:roll_brush:content";
	static const char *kFillC = "ui:zp:roll_fill:content";
	static const char *kDispC = "ui:zp:roll_displace:content";
	static const char *kPropC = "ui:zp:roll_properties:content";
	static const char *kPainterWin = "ui:zp:painter";
	static const char *kToolbar = "ui:zp:toolbar";
	static const char *kBtnPalette = "ui:zp:roll_tiles:content:btn_palette";
	static const char *kBtnPaletteDisp = "ui:zp:roll_displace:content:btn_palette_disp";

	if (CInterfaceGroup *tb = findGroupEl(kToolbar))
	{
		if (!tb->getActive())
			tb->setActive(true);
	}

	// Mode radios on TOOLBAR; buttons live under header_closed
	// (gestionsets closed-bar idiom), not content
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_tile"))
		btn->setPushed(b->Mode == 0);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_color"))
		btn->setPushed(b->Mode == 1);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_displace"))
		btn->setPushed(b->Mode == 2);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_prop"))
		btn->setPushed(b->Mode == 3);
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:mode_patch"))
		btn->setPushed(b->Mode == 4);

	// Mode-gated rollout VISIBILITY (not collapse).
	// Non-applicable rollouts are fully hidden. Open/collapsed is remembered per
	// rollout across mode switches (setActive does not touch isOpen).
	// Tile → Session+Tiles+Brush+Fill
	// Color → Session+Brush+Fill
	// Displace → Session+Displace+Brush+Fill
	// Prop → Session+Properties
	// Patch → Session+Patch
	const bool tileActive = (b->Mode == 0);
	const bool colorActive = (b->Mode == 1);
	const bool displaceActive = (b->Mode == 2);
	const bool propActive = (b->Mode == 3);
	const bool patchActive = (b->Mode == 4);
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
	if (CInterfaceGroup *g = findGroupEl(kRollPatch))
		g->setActive(patchActive);

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
				kRollSession, kRollTiles, kRollBrush, kRollFill, kRollDisplace, kRollProperties,
				kRollPatch
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

	// Hide Tiles palette window while Prop is active; restore when leaving if
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
				// P key / stray open while still in Prop: keep suppressed
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
	// (tree-list scroll when open rollouts exceed max H); no mode-based setH.
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

	if (CCtrlBaseButton *btn = findButton((std::string(kTilesC) + ":toggle_256").c_str()))
		btn->setPushed(b->Mode256);

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

	// Season face on TOOLBAR: shows current season; frozen when <2 seasons
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:toolbar:header_closed:btn_season")))
	{
		const char *lab = b->SeasonLabel[0] ? b->SeasonLabel : "auto";
		// Uppercase face (SPRING / SUMMER / …)
		std::string up = NLMISC::toUpperAscii(lab);
		btn->setHardText(up);
		btn->setFrozen(b->SeasonCount < 1);
	}
	// Pivot face on TOOLBAR: only a transform has a pivot, so the button is shown with patch
	// mode rather than sitting inert next to the paint tools.
	if (CCtrlTextButton *btn = dynamic_cast<CCtrlTextButton *>(
	        CWidgetManager::getInstance()->getElementFromId("ui:zp:toolbar:header_closed:btn_pivot")))
	{
		btn->setActive(b->Mode == 4);
		btn->setHardText(b->PivotLabel[0] ? b->PivotLabel : "SEL");
	}
	if (CCtrlBaseButton *btn = findButton("ui:zp:toolbar:header_closed:btn_save"))
		btn->setFrozen(!b->CanSave);

	// Multi-file dirty indicator: "N files, M dirty"
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
	// Clickable color swatch: button face modulated to the brush color
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
	// btn_mask_mode is a zp_checkbox_row; canonical box is :box
	if (CCtrlBaseButton *btn = findButton((std::string(kBrushC) + ":btn_mask_mode:box").c_str()))
		btn->setPushed(b->BrushMaskMode);

	if (CViewText *t = findText((std::string(kDispC) + ":disp_info").c_str()))
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", b->DisplaceIndex);
		t->setHardText(buf);
	}

	if (patchActive)
	{
		// Level buttons: pressed = current level (legacy toggle-off handled by the AH).
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":lvl_vert").c_str()))
			btn->setPushed(b->SubObj == 1);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":lvl_edge").c_str()))
			btn->setPushed(b->SubObj == 2);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":lvl_patch").c_str()))
			btn->setPushed(b->SubObj == 3);
		// Selection readout: the current level's count, singular/plural like the legacy line.
		if (CViewText *t = findText((std::string(kPatchC) + ":sel_info").c_str()))
		{
			char buf[64];
			if (b->SubObj == 1)
			{
				if (b->PatchSelTans)
					snprintf(buf, sizeof(buf), "%u Handle%s (%u Vert%s) Selected",
					         b->PatchSelTans, b->PatchSelTans == 1 ? "" : "s",
					         b->PatchSelVerts, b->PatchSelVerts == 1 ? "ex" : "ices");
				else if (b->PatchSelVerts)
					snprintf(buf, sizeof(buf), "%u Vert%s Selected",
					         b->PatchSelVerts, b->PatchSelVerts == 1 ? "ex" : "ices");
				else
					snprintf(buf, sizeof(buf), "No Vertex Selected");
			}
			else if (b->SubObj == 2)
				snprintf(buf, sizeof(buf), b->PatchSelEdges == 1 ? "%u Edge Selected"
				                                                 : "%u Edges Selected", b->PatchSelEdges);
			else if (b->SubObj == 3)
				snprintf(buf, sizeof(buf), b->PatchSelFaces == 1 ? "%u Patch Selected"
				                                                 : "%u Patches Selected", b->PatchSelFaces);
			else
				snprintf(buf, sizeof(buf), "Object Level");
			// One zone -> its name; several -> the count. A cross-file selection is easy
			// to make by accident and otherwise invisible in the readout.
			std::string line = buf;
			const bool haveSel = (b->SubObj == 1 && (b->PatchSelVerts || b->PatchSelTans))
				|| (b->SubObj == 2 && b->PatchSelEdges)
				|| (b->SubObj == 3 && b->PatchSelFaces);
			if (haveSel && b->PatchSelZones == 1 && b->PatchSelZoneName[0])
				line += std::string(" - ") + b->PatchSelZoneName;
			else if (haveSel && b->PatchSelZones > 1)
			{
				char zbuf[32];
				snprintf(zbuf, sizeof(zbuf), " - %u zones", b->PatchSelZones);
				line += zbuf;
			}
			t->setHardText(line);
		}
		// Selection block: the pick filters live at vertex level, and the two boxes obey
		// the you-cannot-filter-both-away rule - unchecking one freezes the other.
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":filter_verts:box").c_str()))
		{
			btn->setPushed(b->FilterVerts);
			btn->setFrozen(b->SubObj != 1 || !b->FilterVecs);
		}
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":filter_vecs:box").c_str()))
		{
			btn->setPushed(b->FilterVecs);
			btn->setFrozen(b->SubObj != 1 || !b->FilterVerts);
		}
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":lock_handles:box").c_str()))
		{
			btn->setPushed(b->LockHandles);
			btn->setFrozen(b->SubObj != 1);
		}
		// Orientation arrows: a display toggle, live at every level.
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":show_arrows:box").c_str()))
			btn->setPushed(b->ShowArrows);
		// Bind/Unbind live at vertex level, Delete at patch level, No smooth at edge level.
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_bind").c_str()))
			btn->setFrozen(b->SubObj != 1 || !b->PatchSelVerts);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_unbind").c_str()))
			btn->setFrozen(b->SubObj != 1 || !b->PatchSelVerts);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_weld").c_str()))
			btn->setFrozen(b->SubObj != 1 || b->PatchSelVerts < 2);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_weld_target").c_str()))
		{
			btn->setFrozen(b->SubObj != 1);
			btn->setPushed(b->WeldTargetArmed);
		}
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_delete").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_turn_ccw").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_turn_cw").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_subdivide").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_add_quad").c_str()))
			btn->setFrozen(b->SubObj != 2 || !b->PatchSelEdges);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_detach").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_extrude").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_element").c_str()))
			btn->setFrozen(b->SubObj != 3 || !b->PatchSelFaces);
		// Hide takes the current level's selection (vertex/edge/patch); Unhide All is live
		// whenever anything is hidden, whatever the level.
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_hide").c_str()))
		{
			const bool haveSel = (b->SubObj == 1 && b->PatchSelVerts)
				|| (b->SubObj == 2 && b->PatchSelEdges)
				|| (b->SubObj == 3 && b->PatchSelFaces);
			btn->setFrozen(!haveSel);
		}
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_unhide").c_str()))
			btn->setFrozen(!b->HiddenCount);
		// Scene-menu compass: a direction is live when an editable board neighbor sits there
		// (patch level with a face selection; the mask is empty otherwise).
		{
			static const char *kMvIds[8] = { "mv_n", "mv_ne", "mv_e", "mv_se",
			                                 "mv_s", "mv_sw", "mv_w", "mv_nw" };
			for (int d = 0; d < 8; ++d)
				if (CCtrlBaseButton *btn = findButton(
						(std::string("ui:zp:scene_menu:content:") + kMvIds[d]).c_str()))
					btn->setFrozen(!(b->MoveDirMask & (1u << d)));
		}
		// Scene-menu vertex type pair: checked = the WHOLE selection carries that type
		// (mixed shows neither); live at vertex level with a vertex selection.
		{
			const bool vtOff = b->SubObj != 1 || !b->PatchSelVerts;
			if (CCtrlBaseButton *btn = findButton("ui:zp:scene_menu:content:vt_coplanar"))
			{
				btn->setFrozen(vtOff);
				btn->setPushed(!vtOff && b->VertCoplanar == 1);
			}
			if (CCtrlBaseButton *btn = findButton("ui:zp:scene_menu:content:vt_corner"))
			{
				btn->setFrozen(vtOff);
				btn->setPushed(!vtOff && b->VertCoplanar == 0);
			}
		}
		if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":no_smooth:box").c_str()))
		{
			btn->setFrozen(b->SubObj != 2 || !b->PatchSelEdges);
			btn->setPushed(b->PatchNoSmooth == 1);
		}
		if (CViewText *t = findText((std::string(kPatchC) + ":no_smooth_state").c_str()))
			t->setHardText(b->SubObj == 2 && b->PatchSelEdges
			                   ? (b->PatchNoSmooth == 2 ? "(mixed)" : "")
			                   : "");
		// Surface Properties: grid pushed = bit on ALL selected patches; frozen outside
		// patch level or without a face selection. A mixed bit shows unpushed - the click
		// still resolves the tri-state (off or mixed sets, all clears).
		{
			const bool surfOff = b->SubObj != 3 || !b->PatchSelFaces;
			for (int g = 0; g < 32; ++g)
			{
				char nm[160];
				snprintf(nm, sizeof(nm), "%s:sg_%d", kPatchC, g);
				if (CCtrlBaseButton *btn = findButton(nm))
				{
					btn->setPushed(((b->SmGroupAll >> g) & 1) != 0);
					btn->setFrozen(surfOff);
				}
			}
			if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":btn_sg_clear").c_str()))
				btn->setFrozen(surfOff || !b->SmGroupAny);
			if (CViewText *t = findText((std::string(kPatchC) + ":tess_info").c_str()))
			{
				char buf[32];
				if (b->TessU > 0)
					snprintf(buf, sizeof(buf), "Tess %dx%d", 1 << b->TessU, 1 << b->TessV);
				else
					snprintf(buf, sizeof(buf), "Tess -");
				t->setHardText(buf);
			}
			static const char *kSteppers[5] = { "tess_u_dn", "tess_u_up", "tess_v_dn",
			                                    "tess_v_up", "btn_balance" };
			for (int k = 0; k < 5; ++k)
				if (CCtrlBaseButton *btn = findButton((std::string(kPatchC) + ":" + kSteppers[k]).c_str()))
					btn->setFrozen(surfOff);
		}
		// Same live status line the HUD shows (op results land in g_PropStatusMsg).
		if (CViewText *t = findText((std::string(kPatchC) + ":patch_status").c_str()))
			t->setHardText(b->PropStatus);
	}

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
				         b->PropDirty ? " dirty" : "");
			t->setHardText(buf);
		}
		if (CViewText *t = findText((std::string(kPropC) + ":prop_rot").c_str()))
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", b->PropRotate);
			t->setHardText(buf);
		}
		// prop_* are zp_checkbox_row groups; bridge drives :box (+ freeze :lbl)
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

	syncPaletteHighlight(b->CurTileSet, b->TileSetCount);
	syncDisplaceHighlight(b->DisplaceIndex);
	// Track close-via-X: container may deactivate without going through togglePalette
	{
		CInterfaceElement *el = CWidgetManager::getInstance()->getElementFromId(kPaletteWinId);
		if (el)
			s_PaletteVisible = el->getActive();
	}
	if (CCtrlBaseButton *btn = findButton(kBtnPalette))
		btn->setPushed(s_PaletteVisible);
	if (CCtrlBaseButton *btn = findButton(kBtnPaletteDisp))
		btn->setPushed(s_PaletteVisible);
	emphasizeDisplaceSection(displaceActive);
}

void CEditorUI::update()
{
	if (!_Ready || !_Visible)
		return;
	syncPanelFromBridge();
	CWidgetManager *wm = CWidgetManager::getInstance();
	// NLGUI does not write its own frame clock; without this, frameDiffMs
	// stays 0 and every time-driven behavior (caret blink, rollover fades,
	// interface animations) stalls silently.
	{
		CWidgetManager::SInterfaceTimes times = wm->getInterfaceTimes();
		sint64 nowMs = (sint64)NLMISC::CTime::getLocalTime();
		times.lastFrameMs = times.thisFrameMs;
		times.thisFrameMs = nowMs;
		times.frameDiffMs = times.lastFrameMs ? (nowMs - times.lastFrameMs) : 0;
		wm->updateInterfaceTimes(times);
	}
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
	// The software cursor is a GUI view: with the GUI hidden nothing draws it, so the
	// system cursor has to come back or the viewport is left with no pointer at all.
	if (_Driver)
		_Driver->showCursor(!_Visible);
}

void CEditorUI::toggleVisible()
{
	setVisible(!_Visible);
}

} // namespace ZPUI

/**
 * Open the scene context menu from the 3D view.
 *
 * Called by the viewer's right-click path rather than by an action handler: the click lands on
 * the scene, which is not a widget, so there is no caller control to hang a handler on. The
 * modal is opened with a NULL caller, which is what puts it at the pointer.
 */
void zpOpenSceneMenu()
{
	if (ZPSCRIPT::isExecuting())
		return;
	NLGUI::CWidgetManager::getInstance()->enableModalWindow(NULL, "ui:zp:scene_menu");
}


/* end of file */
