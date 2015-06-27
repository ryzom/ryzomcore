// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdpch.h"

#include "nel/gui/group_webkit.h"
#include "nel/gui/webkit_handler.h"

#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/lua_ihm.h"

#include <cef_app.h>

using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLGUI;

NLMISC_REGISTER_OBJECT(CViewBase, CGroupWebkit, std::string, "webkit");

	CGroupWebkit::CGroupWebkit(const TCtorParam &param)
: CInterfaceGroup(param)
{
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	_Material = rVR.getDriver()->createMaterial();
	_Material.initUnlit();
	_Material.setDoubleSided();
	_Material.setZWrite(false);
	_Material.setZFunc(UMaterial::always);
	_Material.setBlend(true);
	_Material.setBlendFunc (UMaterial::srcalpha, UMaterial::invsrcalpha);
	_Material.setColor(CRGBA::White);
	_Material.setTexture(0, NULL);
	_Material.setTexture(1, NULL);
	_Material.setTexture(2, NULL);
	_Material.setTexture(3, NULL);
	_Material.setZBias(0);

	_Texture = NULL;
	_PopupTexture = NULL;
	_PopupX = 0;
	_PopupY = 0;

	_Focused = false;
	_GrabKeyboard = false;
	_RestoreKeyboardCapture = false;
	_LastActive = false;

	_TitlePrefix = "";
	_Home = "";
	_URL = "";

	_LastParentW = 0;
	_LastParentH = 0;

	_StatusMessage = "";
	_TooltipString = "";
	_Loading = false;

	CefBrowserSettings browserSettings;
	CefString(&browserSettings.default_encoding).FromASCII("UTF-8");
	browserSettings.image_shrink_standalone_to_fit = STATE_ENABLED;

	CefWindowInfo window_info;
	window_info.SetAsWindowless(0, true);

	// cef handler(s)
	_BrowserHandler = new CWebkitHandler();
	// there is no texture yet, but set window handler for other events
	_BrowserHandler->setWindow(this);

	_Browser = CefBrowserHost::CreateBrowserSync(window_info, _BrowserHandler.get(), "about:blank", browserSettings, NULL);

	// Cef message loop is depending on this
	CWidgetManager::getInstance()->registerClockMsgTarget(this);
}

// ***************************************************************************

CGroupWebkit::~CGroupWebkit()
{
	// clear texture from CefRenderHandler
	_BrowserHandler->setWindow(NULL);
	// force close browser
	_Browser->GetHost()->CloseBrowser(true);

	_Browser = NULL;
	_BrowserHandler = NULL;

	CViewRenderer &rVR = *CViewRenderer::getInstance();

	if (_Texture)
		rVR.getDriver()->deleteTextureMem(_Texture);

	if (_PopupTexture)
		rVR.getDriver()->deleteTextureMem(_PopupTexture);

	_Texture = NULL;
	_PopupTexture = NULL;
}

// ***************************************************************************

string CGroupWebkit::home ()
{
	return _Home;
}

// ***************************************************************************
void CGroupWebkit::browse(const char *url)
{
	_URL = url;

	if (_URL.empty() || _URL == "home")
		_URL = home();

	_Browser->GetMainFrame()->LoadURL(_URL);
}

// ***************************************************************************
void CGroupWebkit::browseUndo()
{
	_Browser->GoBack();
}

// ***************************************************************************
void CGroupWebkit::browseRedo()
{
	_Browser->GoForward();
}


// ***************************************************************************
void CGroupWebkit::refresh()
{
	_Browser->ReloadIgnoreCache();
}

// ***************************************************************************
void CGroupWebkit::parseHtml(const std::string &html)
{
	_Browser->GetMainFrame()->LoadString(html, ":");
}

// ***************************************************************************
void CGroupWebkit::clearUndoRedo()
{
	// FIXME: missing in cef3?
}

// ***************************************************************************
void CGroupWebkit::setProperty(const std::string &name, const std::string &value)
{
	if (name == "title_prefix")
	{
		if (!value.empty())
			_TitlePrefix = value + " - ";
		else
			_TitlePrefix = "";
	}
	else
	if (name == "url")
		_URL = value;
	else
	if (name == "home")
		_Home = value;
	else
	if (name == "grab_keyboard")
	{
		bool b;
		if (fromString(value, b))
			_GrabKeyboard = b;
	}
}

// ***************************************************************************
bool CGroupWebkit::parse(xmlNodePtr cur,CInterfaceGroup *parentGroup)
{
	CInterfaceGroup::parse(cur, parentGroup);

	std::string props[] = {
		"title_prefix", "url", "home", "grab_keyboard"
	};
	uint size = sizeof(props) / sizeof(props[0]);

	CXMLAutoPtr ptr;
	for (uint i=0; i<size; ++i)
	{
		ptr = xmlGetProp(cur, (xmlChar*) props[i].c_str());
		if (ptr)
			setProperty(props[i], string(ptr));
	}

	// Tooltip position follows mouse
	setInstantContextHelp(true);
	setToolTipParent(TTMouse);
	setToolTipParentPosRef(Hotspot_TTAuto);
	setToolTipPosRef(Hotspot_TTAuto);

	return true;
}

// ***************************************************************************
bool CGroupWebkit::handleKeyEvent(const NLGUI::CEventDescriptorKey &event)
{
	// FIXME:
	//_KeyEvent.windows_key_code = ;
	//_KeyEvent.native_key_code = ;

	if (event.getKeyShift())
		_KeyEvent.modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (event.getKeyCtrl())
		_KeyEvent.modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (event.getKeyAlt())
	{
		_KeyEvent.modifiers |= EVENTFLAG_ALT_DOWN;
		_KeyEvent.is_system_key = true;
	}

	_KeyEvent.character = _KeyEvent.unmodified_character;

	if (event.getKeyEventType() == NLGUI::CEventDescriptorKey::keyup)
	{
		_KeyEvent.type = KEYEVENT_KEYUP;
		_KeyEvent.native_key_code = event.getKey();
		_KeyEvent.windows_key_code = event.getKey();

		_Browser->GetHost()->SendKeyEvent(_KeyEvent);
		return true;
	}
	if (event.getKeyEventType() == NLGUI::CEventDescriptorKey::keydown)
	{
		_KeyEvent.type = KEYEVENT_KEYDOWN;
		_KeyEvent.native_key_code = event.getKey();
		_KeyEvent.windows_key_code = event.getKey();

		_Browser->GetHost()->SendKeyEvent(_KeyEvent);
		return true;
	}
	if (event.getKeyEventType() == NLGUI::CEventDescriptorKey::keychar)
	{
		_KeyEvent.character = event.getChar();

		_KeyEvent.type = KEYEVENT_KEYUP;
		_Browser->GetHost()->SendKeyEvent(_KeyEvent);

		_KeyEvent.type = KEYEVENT_CHAR;
		_Browser->GetHost()->SendKeyEvent(_KeyEvent);

		return true;
	}
	if (event.getKeyEventType() == NLGUI::CEventDescriptorKey::keystring)
	{
		// CEF handles this itself when ctrl+v is pressed

		return true;
	}

	return false;
}

// ***************************************************************************
bool CGroupWebkit::handleMouseEvent(const NLGUI::CEventDescriptorMouse &event)
{
	// make sure event is for our group and not just in a window
	if (!isIn(event.getX(), event.getY()))
		return false;

	sint32 x = event.getX() - _XReal;
	sint32 y = _YReal - event.getY() + _HReal;

	CefMouseEvent mouse_event;
	mouse_event.x = x;
	mouse_event.y = y;

	_Browser->GetHost()->SendMouseMoveEvent(mouse_event, false);

	if (event.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousewheel)
	{
		int dx = 0;
		int dy = event.getWheel()*50;

		if (_KeyEvent.type == KEYEVENT_KEYDOWN && _KeyEvent.modifiers & EVENTFLAG_CONTROL_DOWN)
		{
			if (dy > 0)
				browserZoomIn();
			else
				browserZoomOut();
		}
		else
			_Browser->GetHost()->SendMouseWheelEvent(mouse_event, dx, dy);

		return true;
	}

	sint32 type = event.getEventTypeExtended();

	// FIXME: double click - interpreted as up/down/up/down
	if (type == NLGUI::CEventDescriptorMouse::mouseleftup ||
			type == NLGUI::CEventDescriptorMouse::mouseleftdown ||
			type == NLGUI::CEventDescriptorMouse::mouseleftdblclk)
	{
		bool up = (event.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup);

		uint8 count = (type == NLGUI::CEventDescriptorMouse::mouseleftdblclk) ? 2 : 1;

		mouse_event.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		_Browser->GetHost()->SendMouseClickEvent(mouse_event, MBT_LEFT, up, count);

		return true;
	}

	if (type == NLGUI::CEventDescriptorMouse::mouserightup ||
			type == NLGUI::CEventDescriptorMouse::mouserightdown ||
			type == NLGUI::CEventDescriptorMouse::mouserightdblclk)
	{
		bool up = (event.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup);
		uint8 count = (type == NLGUI::CEventDescriptorMouse::mouserightdblclk) ? 2 : 1;

		mouse_event.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		_Browser->GetHost()->SendMouseClickEvent(mouse_event, MBT_RIGHT, up, count);

		return true;
	}

	return false;
}

// ***************************************************************************
bool CGroupWebkit::handleEvent(const NLGUI::CEventDescriptor& event)
{
	// FIXME: subscribe to keyboard event to know shift/alt/ctrl state when we dont have keyboard focus
	//
	if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) event;
		if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
		{
			handle();
		}
	}

	if (event.getType() == NLGUI::CEventDescriptor::key)
	{
		if (handleKeyEvent((const NLGUI::CEventDescriptorKey &)event))
			return true;
	}

	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		if (handleMouseEvent((const NLGUI::CEventDescriptorMouse &)event))
			return true;
	}

	if (CInterfaceGroup::handleEvent(event)) return true;

	return false;
}

// ***************************************************************************
bool CGroupWebkit::isKeyboardCaptured() const
{
	return (CWidgetManager::getInstance()->getCaptureKeyboard() == this);
}

// ***************************************************************************
void CGroupWebkit::releaseKeyboard()
{
	setGrabKeyboardButtonPushed(false);

	if (! isKeyboardCaptured())
		return;

	_KeyEvent.modifiers = EVENTFLAG_NONE;
	CWidgetManager::getInstance()->resetCaptureKeyboard();
}

// ***************************************************************************
void CGroupWebkit::captureKeyboard()
{
	setGrabKeyboardButtonPushed(true);

	if (isKeyboardCaptured())
		return;

	CWidgetManager::getInstance()->setCaptureKeyboard(this);
}

// ***************************************************************************
void CGroupWebkit::setKeyboardCapture(bool capture)
{
	_RestoreKeyboardCapture = _GrabKeyboard || capture;

	// ignore release request if window still has mouse over it
	if (capture || (_GrabKeyboard && _Focused))
		captureKeyboard();
	else
		releaseKeyboard();
}

// ***************************************************************************
void CGroupWebkit::setGrabKeyboard(bool grab)
{
	_GrabKeyboard = grab;
	setKeyboardCapture(grab);
}

// ***************************************************************************
void CGroupWebkit::setGrabKeyboardButtonPushed(bool pushed)
{
	CInterfaceGroup* group = getParentContainer();
	if (group)
	{
		CCtrlBaseButton	*btn = dynamic_cast<CCtrlBaseButton *>(group->getCtrl("grab_keyboard"));
		if (btn)
			btn->setPushed(pushed);
	}
}

// ***************************************************************************
void CGroupWebkit::setFocus(bool state)
{
	if (!_Active)
		return;

	if (state && ! _Focused)
	{
		_Focused = true;
		_Browser->GetHost()->SendFocusEvent(true);

		if (_RestoreKeyboardCapture)
			captureKeyboard();
	}
	else
	if (! state && _Focused)
	{
		_Focused = false;
		_Browser->GetHost()->SendFocusEvent(false);

		if (isKeyboardCaptured())
			releaseKeyboard();
	}

	setGrabKeyboardButtonPushed(_Focused && _RestoreKeyboardCapture);
}

// ***************************************************************************
void CGroupWebkit::setTitle(const std::string &title)
{
	CInterfaceElement *parent = getParent();
	if (parent)
	{
		if ((parent = parent->getParent()))
		{
			CGroupContainer *container = dynamic_cast<CGroupContainer*>(parent);
			if (container)
			{
				container->setUCTitle(ucstring(_TitlePrefix + title));
			}
		}
	}
}

// ***************************************************************************
void CGroupWebkit::draw ()
{
	if (!_Active)
		return;

	CRGBA color(CRGBA::White);
	/* // CtrlQuad::draw

	if (getModulateGlobalColor())
		color.modulateFromColor(CRGBA::White, CWidgetManager::getInstance()->getGlobalColorForContent());
	else
	{
		color = CRGBA::White;
		color.A = (uint8)(((sint32)color.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
	}
	*/

	// get parent container content alpha value
	CInterfaceGroup *gr = getParent();
	while (gr)
	{
		if (gr->isGroupContainer())
		{
			CGroupContainer *gc = static_cast<CGroupContainer *>(gr);
			color.A = gc->getCurrentContainerAlpha();
			break;
		}
		gr = gr->getParent();
	}

	CViewRenderer &rVR = *CViewRenderer::getInstance();
	if (_Texture)
	{
		_Material.setTexture(0, _Texture);
		rVR.drawCustom(_XReal, _YReal, _WReal, _HReal, color, _Material);
	}

	if (_PopupTexture)
	{
		// calculate bottom-left corner for popup texture inside window
		sint popx = _XReal + _PopupX;
		sint popy = _YReal + _HReal - _PopupY - _PopupTexture->getImageHeight();

		_Material.setTexture(0, _PopupTexture);
		rVR.drawCustom(popx, popy, _PopupTexture->getImageWidth(), _PopupTexture->getImageHeight(),
				color, _Material);
	}

	CInterfaceGroup::draw();
}

static void copyBGRAtoRGBA(
		// destination buffer and it's size
		uint8 *dst, sint dstw, sint dsth,
		// source buffer and it's size
		const uint8 *src, sint srcw, sint srch,
		// rectangle to be copied
		const CefRect &rect)
{
	uint bytePerPixel = 4;

	// make sure we dont overflow either buffers
	sint xmax = std::min(dstw, std::min(rect.x + rect.width, rect.x + srcw));
	sint ymax = std::min(dsth, std::min(rect.y + rect.height, rect.y + srch));

	for(sint x = rect.x; x < xmax; ++x)
	{
		for(sint y = rect.y; y < ymax; ++y)
		{
			sint pos = (x + y * srcw) * bytePerPixel;

			// RGBA <- BGRA
			dst[pos + 0] = src[pos + 2];
			dst[pos + 1] = src[pos + 1];
			dst[pos + 2] = src[pos];
			dst[pos + 3] = src[pos + 3];
		}
	}
}

// ***************************************************************************
void CGroupWebkit::drawIntoTexture(const CefRenderHandler::RectList &dirtyRects, const void *buffer, sint width, sint height, bool popup)
{
	// this can be optimized by copying full buffer into texture using memcpy
	// and when drawing, setting up vertex buffer with BGRA format and use driver->renderRawQuads() to draw it

	UTextureMem *tex = NULL;
	if (popup)
		tex = _PopupTexture;
	else
		tex = _Texture;

	if (tex)
	{
		// dirtyRects should contain only single rect (largest region to update)
		if (dirtyRects.size() == 1)
		{
			const CefRect &rect = dirtyRects[0];
			copyBGRAtoRGBA((uint8 *)tex->getPointer(), tex->getImageWidth(), tex->getImageHeight(),
					(uint8 *)buffer, width, height,
					rect);
		}
		else
		{
			CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
			for(; i != dirtyRects.end(); ++i)
			{
				const CefRect &rect = *i;
				copyBGRAtoRGBA((uint8 *)tex->getPointer(), tex->getImageWidth(), tex->getImageHeight(),
						(uint8 *)buffer, width, height,
						rect);
			}
		}

		tex->touch();
	}
	else
		nldebug("missing %s texture\n", popup ? "main" : "popup");
}

// ***************************************************************************
void CGroupWebkit::setPopupVisibility(bool visible)
{
	// hide popup by making sure texture is not set
	if (_PopupTexture && !visible)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.getDriver()->deleteTextureMem(_PopupTexture);
		_PopupTexture = NULL;
	}
}

// ***************************************************************************
void CGroupWebkit::setPopupSize(const CefRect &rect)
{
	// always get rid of old popup
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	if (_PopupTexture)
	{
		rVR.getDriver()->deleteTextureMem(_PopupTexture);
		_PopupTexture = NULL;
	}

	if (rect.width == 0 || rect.height == 0)
		return;

	// setup new popup with new texture
	_PopupTexture = rVR.getDriver()->createTextureMem(rect.width, rect.height, CBitmap::RGBA);
	_PopupX = rect.x;
	_PopupY = rect.y;
}

// ***************************************************************************
void CGroupWebkit::invalidateCoords()
{
	if (!_Active && _LastActive)
	{
		_Browser->GetHost()->SetWindowVisibility(false);
		_LastActive = false;
	}
	else
	if (_Active && !_LastActive)
	{
		_Browser->GetHost()->SetWindowVisibility(true);
		_LastActive = true;
	}
	CInterfaceGroup::invalidateCoords();
}

// ***************************************************************************
void CGroupWebkit::checkCoords()
{
	if (!_Active)
	{
		if (_LastActive)
			_Browser->GetHost()->SetWindowVisibility(false);

		_LastActive = false;
		releaseKeyboard();

		return;
	}

	// called on each frame
	if (_Parent)
	{
		// switch focus only when not being resized
		if (CWidgetManager::getInstance()->getCapturePointerLeft() == NULL)
		{
			CViewPointerBase *mousePointer = CWidgetManager::getInstance()->getPointer();
			if (mousePointer)
			{
				if (isIn(mousePointer->getX(), mousePointer->getY()))
				{
					if (!_Focused)
					{
						// get top-most window and see if any of it belongs to us
						CInterfaceGroup *ig = CWidgetManager::getInstance()->getCurrentWindowUnder();
						while(ig)
						{
							if (ig == _Parent->getRootWindow())
							{
								setFocus(true);
								break;
							}
							ig = ig->getParent();
						}
					}
				}
				else
				if (_Focused)
					setFocus(false);
			}
		}

		// resize event
		sint parentWidth = std::min(_Parent->getMaxWReal(), _Parent->getWReal());
		sint parentHeight = std::min(_Parent->getMaxHReal(), _Parent->getHReal());
		if (_LastParentW != (sint)parentWidth || _LastParentH != (sint)parentHeight)
		{
			CCtrlBase *pCB = CWidgetManager::getInstance()->getCapturePointerLeft();
			if (pCB != NULL)
			{
				CCtrlResizer *pCR = dynamic_cast<CCtrlResizer *>(pCB);
				if (pCR != NULL)
				{
					// resize in progress
				}
				else
				{
					_LastParentW = parentWidth;
					_LastParentH = parentHeight;
					invalidateContent();
				}
			}
			else
			{
				_LastParentW = parentWidth;
				_LastParentH = parentHeight;
				invalidateContent();
			}
		}
	}
	CInterfaceGroup::checkCoords();
}

// ***************************************************************************
void CGroupWebkit::onInvalidateContent()
{
	if (!_Active)
		return;

	CViewRenderer &rVR = *CViewRenderer::getInstance();

	if (_Texture)
	{
		rVR.getDriver()->deleteTextureMem(_Texture);
		_Texture = NULL;
	}
	else
	{
		// must be first call as there is no texture yet
		browse(_URL.c_str());
	}

	if (_WReal > 0 && _HReal > 0)
	{
		// create texture same size as window
		_Texture = rVR.getDriver()->createTextureMem(_WReal, _HReal, CBitmap::RGBA);
		_Texture->setWrapS(NL3D::UTexture::Clamp);
		_Texture->setWrapT(NL3D::UTexture::Clamp);

		_Browser->GetHost()->WasResized();
	}
}

// ***************************************************************************
void CGroupWebkit::handle()
{
	// FIXME: auto hide status bar on timeout
	//const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();
	// times.thisFrameMs / 1000.0f;

	CefDoMessageLoopWork();
}

// ***************************************************************************
void CGroupWebkit::showHideStatus(bool show)
{
	CInterfaceGroup* group = getParentContainer();
	if (group)
	{
		CInterfaceGroup *pGroup = dynamic_cast<CInterfaceGroup*>(group->getGroup("status_bar"));
		if (pGroup)
			pGroup->setActive(show);
	}
}

// ***************************************************************************
void CGroupWebkit::setStatusMessage(const std::string &msg)
{
	if (_StatusMessage == msg)
		return;

	_StatusMessage = msg;
	showHideStatus(! _StatusMessage.empty());

	CInterfaceGroup* group = getParentContainer();
	if (group)
	{
		CViewText *pView = dynamic_cast<CViewText*>(group->getView("status"));
		if (pView)
			pView->setText(ucstring(msg));
	}
}

// ***************************************************************************
void CGroupWebkit::setTooltip(const std::string &tt)
{
	if (_TooltipString == tt)
		return;

	_TooltipString = tt;
	setDefaultContextHelp(tt);
}

// ***************************************************************************
void CGroupWebkit::setLoadingState(bool loading, bool undo, bool redo)
{
	if (_Loading == loading)
		return;

	_Loading = loading;

	CInterfaceGroup* group = getParentContainer();
	if (group)
	{
		CViewBitmap *pView = dynamic_cast<CViewBitmap*>(group->getView("loading"));
		if (pView)
		{
			if (loading)
			{
				pView->setTexture("w_online.tga");
				setTitle(_TitlePrefix + CI18N::get("uiPleaseWait").toString());
				setStatusMessage(CI18N::get("uiPleaseWait").toString());
			}
			else
				pView->setTexture("w_offline.tga");
		}

		CCtrlBaseButton	*btnRefresh = dynamic_cast<CCtrlBaseButton *>(group->getCtrl("browse_refresh"));
		if (btnRefresh)
			btnRefresh->setFrozen(loading);

		CCtrlBaseButton	*btnUndo = dynamic_cast<CCtrlBaseButton *>(group->getCtrl("browse_undo"));
		if (btnUndo)
			btnUndo->setFrozen(!undo);

		CCtrlBaseButton	*btnRedo = dynamic_cast<CCtrlBaseButton *>(group->getCtrl("browse_redo"));
		if (btnRedo)
			btnRedo->setFrozen(!redo);
	}
}

// ***************************************************************************
void CGroupWebkit::browserZoomIn()
{
	_Browser->GetHost()->SetZoomLevel(_Browser->GetHost()->GetZoomLevel() + 0.5);
}

// ***************************************************************************
void CGroupWebkit::browserZoomOut()
{
	_Browser->GetHost()->SetZoomLevel(_Browser->GetHost()->GetZoomLevel() - 0.5);
}

// ***************************************************************************
void CGroupWebkit::browserZoomReset()
{
	_Browser->GetHost()->SetZoomLevel(0.0f);
}

// ***************************************************************************
int CGroupWebkit::luaBrowse(CLuaState &ls)
{
	const char *funcName = "browse";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	browse(ls.toString(1));
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaBrowseUndo(CLuaState &ls)
{
	const char *funcName = "browseUndo";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	browseUndo();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaBrowseRedo(CLuaState &ls)
{
	const char *funcName = "browseRedo";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	browseRedo();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaRefresh(CLuaState &ls)
{
	const char *funcName = "refresh";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	refresh();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaZoomIn(CLuaState &ls)
{
	const char *funcName = "zoomIn";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	browserZoomIn();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaZoomOut(CLuaState &ls)
{
	const char *funcName = "zoomOut";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	browserZoomOut();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaZoomReset(CLuaState &ls)
{
	const char *funcName = "zoomReset";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	browserZoomReset();
	return 0;
}

// ***************************************************************************
int CGroupWebkit::luaParseHtml(CLuaState &ls)
{
	const char *funcName = "parseHtml";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	std::string html = ls.toString(1);

	parseHtml(html);

	return 0;
}

// ***************************************************************************
class CHandlerWebkitJSDialog : public IActionHandler
{
	void execute(CCtrlBase *pCaller, const std::string &sParams)
	{
		std::string btn = getParam(sParams, "button");
		bool success = (btn == "ok");
		std::string input;

		// get dialog window
		CInterfaceGroup* group = pCaller->getRootWindow();
		if (group)
		{
			// if there is edit box, then it was a prompt dialog
			CGroupEditBox* eb = dynamic_cast<CGroupEditBox*>(group->getGroup("prompt:eb"));
			if (eb)
				input = eb->getInputStringAsUtf8();
		}

		CWebkitHandler::jsDialogContinue(success, input);
		CWidgetManager::getInstance()->popModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerWebkitJSDialog, "webkit_jsdialog");

// ***************************************************************************
class CHandlerWebkitJSDialogCancel : public IActionHandler
{
	void execute(CCtrlBase *pCaller, const std::string &sParams)
	{
		// execute js dialog callback when modal is closed by ESC key for example
		CWebkitHandler::jsDialogContinue(false, "");
		//CWidgetManager::getInstance()->popModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerWebkitJSDialogCancel, "webkit_jsdialog_cancel");

