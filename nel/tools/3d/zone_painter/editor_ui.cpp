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

#include <cstdio>
#include <cstring>

#include <nel/misc/algo.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i18n.h>
#include <nel/misc/path.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/event_listener.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/interface_link.h>
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

class CAHZpSave : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		SPaintUIBridge *b = getPaintUIBridge();
		if (b && b->save) b->save();
	}
};
REGISTER_ACTION_HANDLER(CAHZpSave, "zp_save");

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

	// Save enabled only with --save
	if (CCtrlBaseButton *btn = findButton("ui:zp:painter:content:btn_save"))
		btn->setFrozen(!b->CanSave);
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
