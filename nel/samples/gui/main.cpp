// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// nl_sample_gui — a standalone NeL GUI showcase.
//
// Boots the NLGUI widget library outside the Ryzom client, parses a
// self-contained interface definition, and renders it with the legacy
// "w_" interface skin (the original beveled reference style that the
// shipped "skin_" style later replaced by name-for-name substitution).
// Interface textures are packed at build time from a ryzomcore_graphics
// checkout; no texture data lives in the code repository.

#include <nel/misc/types_nl.h>

#include <set>
#include <string>
#include <vector>

#include <nel/misc/algo.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i18n.h>
#include <nel/misc/path.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

#include <nel/gui/action_handler.h>
#include <nel/gui/db_manager.h>
#include <nel/gui/dbgroup_combo_box.h>
#include <nel/gui/event_listener.h>
#include <nel/gui/group_html.h>
#include <nel/gui/interface_link.h>
#include <nel/gui/interface_group.h>
#include <nel/gui/lua_helper.h>
#include <nel/gui/lua_ihm.h>
#include <nel/gui/lua_manager.h>
#include <nel/gui/view_pointer.h>
#include <nel/gui/view_renderer.h>
#include <nel/gui/widget_manager.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
static void sampleSegvHandler(int sig)
{
	void *frames[32];
	int n = backtrace(frames, 32);
	backtrace_symbols_fd(frames, n, STDERR_FILENO);
	_exit(128 + sig);
}
#endif

#ifndef GUI_SAMPLE_DATA_DIR
#define GUI_SAMPLE_DATA_DIR "."
#endif
#ifndef GUI_SAMPLE_ASSETS_DIR
#define GUI_SAMPLE_ASSETS_DIR "."
#endif
#ifndef GUI_SAMPLE_FONT
#define GUI_SAMPLE_FONT "basic.ttf"
#endif

using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

namespace {

UDriver *s_Driver = NULL;
UTextContext *s_TextContext = NULL;
CEventListener *s_GuiListener = NULL;
std::set<std::string> s_HwCursors;

std::string s_ScreenshotFile;
sint s_ScreenshotFrame = -1; // frame index at which to save and quit
sint s_FrameIndex = 0;
bool s_Quit = false;

// The master group defined by the sample interface definition
const char *MASTER_GROUP = "ui:sample";

class CAHSampleQuit : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		s_Quit = true;
	}
};
REGISTER_ACTION_HANDLER(CAHSampleQuit, "sample_quit");

// The "proc", "enter_modal" and "leave_modal" handlers live in the Ryzom
// client, not in NLGUI; provide the same behavior here so interface
// definitions can chain procedures and open modal windows.
class CAHProc : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		std::vector<std::string> paramList;
		splitString(params, "|", paramList);
		if (paramList.empty())
			return;
		// the whole list goes along, name included: procedure @0 refers to
		// paramList[1] (the client's handler does the same)
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

// Stacked modals: NLGUI has pushModalWindow/popModalWindow but no action
// handlers for them (the client registers its own); mirror them here.
class CAHPushModal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		std::string group = getParam(params, "group");
		CWidgetManager::getInstance()->pushModalWindow(pCaller, group);
	}
};
REGISTER_ACTION_HANDLER(CAHPushModal, "push_modal");

class CAHPopModal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
		CWidgetManager::getInstance()->popModalWindow();
	}
};
REGISTER_ACTION_HANDLER(CAHPopModal, "pop_modal");

// Open a menu group as a modal window (client convention "active_menu")
class CAHActiveMenu : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const std::string &params)
	{
		std::string menuId = getParam(params, "menu");
		if (!menuId.empty())
			CWidgetManager::getInstance()->enableModalWindow(pCaller, menuId);
	}
};
REGISTER_ACTION_HANDLER(CAHActiveMenu, "active_menu");

// The "lua" handler also lives in the Ryzom client; NLGUI itself invokes
// it for <group lua_class="...">, onclick_l="lua" and <link> lua actions.
class CAHLua : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		CLuaManager::getInstance().executeLuaScript(params, true);
	}
};
REGISTER_ACTION_HANDLER(CAHLua, "lua");

// Every action handler run is also forwarded to the quick-help event
// system (the client records progress with it); provide the sink so each
// click doesn't warn into the log
class CAHSubmitQuickHelp : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string & /* params */)
	{
	}
};
REGISTER_ACTION_HANDLER(CAHSubmitQuickHelp, "submit_quick_help");

// Links and buttons inside group_html fire these handlers; they live in
// the client, not in NLGUI. Minimal versions: resolve the html group from
// the "name" param and drive it.
CGroupHTML *getHtmlGroup(const std::string &params)
{
	std::string name = IActionHandler::getParam(params, "name");
	return dynamic_cast<CGroupHTML *>(CWidgetManager::getInstance()->getElementFromId(name));
}

class CAHBrowse : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		CGroupHTML *html = getHtmlGroup(params);
		if (html)
			html->browse(getParam(params, "url").c_str());
	}
};
REGISTER_ACTION_HANDLER(CAHBrowse, "browse");

class CAHBrowseUndo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		CGroupHTML *html = getHtmlGroup(params);
		if (html)
			html->browseUndo();
	}
};
REGISTER_ACTION_HANDLER(CAHBrowseUndo, "browse_undo");

class CAHBrowseRedo : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		CGroupHTML *html = getHtmlGroup(params);
		if (html)
			html->browseRedo();
	}
};
REGISTER_ACTION_HANDLER(CAHBrowseRedo, "browse_redo");

// The widget manager keeps the pointer position up to date, but nothing in
// NLGUI feeds it the mouse button state — the client mirrors its driver
// events into the pointer in its input handler; do the same here so data
// scripts can poll drag gestures
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
CPointerButtonListener *s_PointerButtonListener = NULL;

// NLGUI's Lua bindings leave widget lookup to the embedder (the client
// registers its own getUI); data scripts need it to reach widgets by id
int luaGetUI(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "getUI", 1);
	CLuaIHM::check(ls, ls.isString(1), "getUI() requires a string");
	std::string id;
	ls.toString(1, id);
	CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId(id);
	if (element)
		CLuaIHM::pushUIOnStack(ls, element);
	else
		ls.pushNil();
	return 1;
}

// Mouse polling for data scripts, same names and signatures as the client
// exposes (getMousePos/getMouseDown); coordinates are interface pixels,
// the space widget x_real/y_real reflected properties live in
int luaGetMousePos(CLuaState &ls)
{
	sint32 x = -1, y = -1;
	CViewPointerBase *pointer = CWidgetManager::getInstance()->getPointer();
	if (pointer)
		pointer->getPointerPos(x, y);
	ls.push(x);
	ls.push(y);
	return 2;
}

int luaGetMouseDown(CLuaState &ls)
{
	bool down = false;
	sint32 x = -1, y = -1;
	CViewPointerBase *pointer = CWidgetManager::getInstance()->getPointer();
	if (pointer)
		down = pointer->getPointerDown(x, y);
	ls.push(down);
	ls.push(x);
	ls.push(y);
	return 3;
}

void renderOneFrame()
{
	s_Driver->EventServer.pump();

	s_Driver->clearBuffers(CRGBA(90, 102, 116));

	CWidgetManager *wm = CWidgetManager::getInstance();
	wm->sendClockTickEvent();
	wm->checkCoords();
	wm->drawViews(NULL);

	if (!s_ScreenshotFile.empty() && s_FrameIndex == s_ScreenshotFrame)
	{
		CBitmap bitmap;
		s_Driver->getBuffer(bitmap);
		COFile f(s_ScreenshotFile);
		bitmap.writePNG(f, 24);
		nlinfo("Saved screenshot: %s", s_ScreenshotFile.c_str());
		s_Quit = true;
	}
	++s_FrameIndex;

	s_Driver->swapBuffers();
}

} // namespace

int main(int argc, char **argv)
{
	CApplicationContext applicationContext;

#if defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
	signal(SIGSEGV, sampleSegvHandler);
#endif

	CCmdArgs args;
	args.setDescription("NeL GUI widget showcase (legacy w_ interface skin)");
	args.addArg("d", "data", "path", "Extra data directory to add to the search path");
	args.addArg("", "screenshot", "file.png", "Save a screenshot and exit");
	args.addArg("", "frames", "count", "Frame to capture for --screenshot (default 8)");
	args.addArg("", "size", "WxH", "Window size (default 1280x720)");
	args.addArg("", "xml", "file.xml", "Extra interface XML file(s) to parse after the sample set");
	args.addArg("", "font-probe", "", "Print renderer font metrics per size and exit");
	if (!args.parse(argc, argv))
		return EXIT_FAILURE;

	// Showcase window positions are laid out for 1280x720
	uint width = 1280, height = 720;
	if (args.haveLongArg("size"))
	{
		std::string s = args.getLongArg("size").front();
		std::string::size_type x = s.find('x');
		if (x != std::string::npos)
		{
			fromString(s.substr(0, x), width);
			fromString(s.substr(x + 1), height);
		}
	}
	if (args.haveLongArg("screenshot"))
	{
		s_ScreenshotFile = args.getLongArg("screenshot").front();
		s_ScreenshotFrame = 8;
		if (args.haveLongArg("frames"))
			fromString(args.getLongArg("frames").front(), s_ScreenshotFrame);
	}

	// Loose-file textures (e.g. the color picker palette) are png on disk
	// while interface definitions use the canonical .tga names; remap before
	// the search paths are indexed
	CPath::remapExtension("png", "tga", true);

	// Search paths: sample interface definitions + generated atlas + font
	CPath::addSearchPath(GUI_SAMPLE_DATA_DIR, true, false);
	CPath::addSearchPath(GUI_SAMPLE_ASSETS_DIR, false, false);
	if (args.haveArg("d"))
	{
		std::vector<std::string> dirs = args.getArg("d");
		for (uint i = 0; i < dirs.size(); ++i)
			CPath::addSearchPath(dirs[i], true, false);
	}

	// Render literal text; the sample does not ship translation tables
	CI18N::setNoResolution(true);

#ifdef __EMSCRIPTEN__
	s_Driver = UDriver::createDriver(0, UDriver::OpenGlEs3);
#else
	s_Driver = UDriver::createDriver(0, false, 0);
#endif
	nlassert(s_Driver);
	s_Driver->setDisplay(UDriver::CMode(width, height, 32, true));
	s_Driver->setWindowTitle(ucstring("NeL GUI sample"));

	std::string font = GUI_SAMPLE_FONT;
	if (!CFile::fileExists(font))
		font = CPath::lookup(CFile::getFilename(font));
	s_TextContext = s_Driver->createTextContext(font);
	nlassert(s_TextContext);
	// fontsize means pixels: without this, sizes scale by windowHeight/600
	// (the client disables the ratio the same way in resetTextContext)
	s_TextContext->setKeep800x600Ratio(false);

	if (args.haveLongArg("font-probe"))
	{
		// Print the metrics the UI renderer actually uses, per font size
		for (uint32 fs = 8; fs <= 16; ++fs)
		{
			s_TextContext->setFontSize(fs);
			UTextContext::CStringInfo caps = s_TextContext->getStringInfo(ucstring("HEADER"));
			UTextContext::CStringInfo mixed = s_TextContext->getStringInfo(ucstring("Sliders, gauges and links gjpqy"));
			printf("fontsize %2u: caps h=%4.1f  mixed h=%4.1f (below baseline %4.1f)\n",
				fs, caps.StringHeight, mixed.StringHeight, -mixed.StringLine);
		}
		s_Driver->deleteTextContext(s_TextContext);
		s_Driver->release();
		return EXIT_SUCCESS;
	}

	// Pump triggered <link> updates whenever database observers flush
	// (the client creates the same updater in its interface manager)
	new CInterfaceLink::CInterfaceLinkUpdater(); // leaked once, process lifetime

	CViewRenderer::setDriver(s_Driver);
	CViewRenderer::setTextContext(s_TextContext);
	CViewRenderer::hwCursors = &s_HwCursors;
	CViewRenderer::getInstance()->init();

	if (!CViewRenderer::getInstance()->loadTextures("gui_sample_atlas.png", "gui_sample_atlas.txt", false))
	{
		nlwarning("Unable to load the interface atlas (gui_sample_atlas.png/.txt)");
		return EXIT_FAILURE;
	}

	// Bring up the interface Lua state before parsing: initLUA registers the
	// stock NLGUI API (setOnDraw, nltime, ...) — the parser does NOT call it
	// itself, the embedder must (the client does the same) — and the sample
	// adds its own entry points on top of it
	IParser *parser = CWidgetManager::getInstance()->getParser();
	parser->initLUA();
	CLuaManager::getInstance().getLuaState()->registerFunc("getUI", luaGetUI);
	CLuaManager::getInstance().getLuaState()->registerFunc("getMousePos", luaGetMousePos);
	CLuaManager::getInstance().getLuaState()->registerFunc("getMouseDown", luaGetMouseDown);

	// Parse the sample interface definition
	std::vector<std::string> xmlFiles;
	xmlFiles.push_back("config_sample.xml");
	xmlFiles.push_back("widgets_sample.xml");
	xmlFiles.push_back("main_sample.xml");
	xmlFiles.push_back("minesweeper_sample.xml");
	xmlFiles.push_back("console_sample.xml");
	xmlFiles.push_back("inventory_sample.xml");
	xmlFiles.push_back("help_sample.xml");
	if (args.haveLongArg("xml"))
	{
		std::vector<std::string> extra = args.getLongArg("xml");
		for (uint i = 0; i < extra.size(); ++i)
			xmlFiles.push_back(extra[i]);
	}

	nlinfo("Atlas loaded");

	if (!parser->parseInterface(xmlFiles, false))
	{
		nlwarning("Unable to parse the sample interface definition");
		return EXIT_FAILURE;
	}

	nlinfo("Interface parsed");

	// The combo box machinery resolves its dropdown menus through these
	// application-provided ids (the client points them at ui:interface:*)
	CDBGroupComboBox::selectMenu = "ui:sample:combo_box_select_menu";
	CDBGroupComboBox::selectMenuOut = "ui:sample:combo_box_select_menu";
	CDBGroupComboBox::measureMenu = "ui:sample:combo_box_measure_menu";

	CWidgetManager::getInstance()->updateAllLocalisedElements();
	nlinfo("Localised elements updated");
	CWidgetManager::getInstance()->activateMasterGroup(MASTER_GROUP, true);
	// select_number::parse writes min into its DB leaf, wiping the sample default
	if (NLMISC::CCDBNodeLeaf *sel = CDBManager::getInstance()->getDbProp("UI:TEMP:SELNUM", false))
		sel->setValue32(3);
	CInterfaceLink::updateAllLinks(); // initial evaluation; afterwards links run on database changes
	nlinfo("Master group activated");

	// Route driver events into the widget manager
	s_GuiListener = new CEventListener();
	s_GuiListener->addToServer(&s_Driver->EventServer);
	s_PointerButtonListener = new CPointerButtonListener();
	s_PointerButtonListener->addToServer(s_Driver->EventServer);
	s_Driver->AsyncListener.addToServer(s_Driver->EventServer);

	nlinfo("Entering main loop");

#ifdef __EMSCRIPTEN__
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(renderOneFrame, 0, 1);
#else
	while (!s_Quit && s_Driver->isActive())
	{
		renderOneFrame();
		if (s_Driver->AsyncListener.isKeyPushed(KeyESCAPE))
			s_Quit = true;
		nlSleep(10); // ~60 fps cap; vsync is not available everywhere
	}
#endif

	s_GuiListener->removeFromServer();
	delete s_GuiListener;
	s_PointerButtonListener->removeFromServer(s_Driver->EventServer);
	delete s_PointerButtonListener;

	CWidgetManager::getInstance()->reset();
	CWidgetManager::getInstance()->getParser()->removeAll();
	CViewRenderer::release();

	s_Driver->deleteTextContext(s_TextContext);
	s_Driver->release();

	return EXIT_SUCCESS;
}
