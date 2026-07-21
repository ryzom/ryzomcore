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

#include <nel/misc/algo.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i18n.h>
#include <nel/misc/path.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/event_listener.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/interface_link.h>
#include <nel/gui/view_pointer.h>
#include <nel/gui/view_renderer.h>
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

// ---------------------------------------------------------------------------------------------
// Pointer button mirror (from nel/samples/gui): NLGUI keeps pointer position up to date but
// nothing feeds button-down state; embedders must mirror driver mouse events into the pointer.

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

// Minimal client-side action handlers the interface XML may invoke (proc / modals).
// Full painter ops land as zp_* handlers in M1c.

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
	// Exe-relative fallback: <exe>/ui then ./ui
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
	// Last resort: system font used by the HUD
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

	// Loose-file textures: png on disk, .tga names in interface XML (sample contract)
	CPath::remapExtension("png", "tga", true);

	const std::string dataDir = resolveDataDir();
	const std::string assetsDir = resolveAssetsDir();
	CPath::addSearchPath(dataDir, true, false);
	CPath::addSearchPath(assetsDir, false, false);

	// Render literal hardtext; this tool does not ship translation tables
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
	// fontsize means pixels (client disables the height-ratio the same way)
	_TextContext->setKeep800x600Ratio(false);

	// Pump triggered <link> updates whenever database observers flush
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

	// Route driver events into the widget manager (before paint/nav so wantsMouse sees them)
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

void CEditorUI::update()
{
	if (!_Ready || !_Visible)
		return;
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
