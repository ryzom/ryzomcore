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

#ifndef CL_GROUP_WEBKIT_H
#define CL_GROUP_WEBKIT_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/view_pointer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/reflect.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_texture.h"

#include "nel/gui/webkit_handler.h"

#include <string>

namespace NLGUI {

	// ***************************************************************************
	/**
	 * Chromium Embedded Framework browser impl
	 *
	 * \author Meelis Mägi
	 * \date 2015
	 */
	class CGroupWebkit : public CInterfaceGroup
	{
	public:
		DECLARE_UI_CLASS(CGroupWebkit)

		// Constructor
		CGroupWebkit(const TCtorParam &param);
		~CGroupWebkit();

		// Get Home url
		std::string	home();

		// Get current url
		std::string getURL() const { return _URL; }

		// Set window title
		void setTitle(const std::string &title);

		// Load url into browser
		void browse(const char *url);

		// Reload _URL into browser, ignoring cache
		void refresh();

		// parse html string into browser
		void parseHtml(const std::string &html);

		// Browser history
		void browseUndo();
		void browseRedo();
		void clearUndoRedo();

		// set browser zoom factor
		void browserZoomIn();
		void browserZoomOut();
		void browserZoomReset();

		// browser status bar
		void showHideStatus(bool show);
		void setStatusMessage(const std::string &msg);
		void setLoadingState(bool loading, bool undo, bool redo);
		void setTooltip(const std::string &tt);

		//
		virtual bool handleEvent(const NLGUI::CEventDescriptor& event);
		virtual void checkCoords();
		virtual void invalidateCoords();
		virtual void onInvalidateContent();
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void draw ();

		//
		bool isKeyboardCaptured() const;
		void setKeyboardCapture(bool capture);
		void setGrabKeyboard(bool grab);
		bool getGrabKeyboard() const { return _GrabKeyboard; }
		void setGrabKeyboardButtonPushed(bool pushed);

		// CefRenderHandler::OnPopupSize
		void setPopupSize(const CefRect &rect);
		// CefRenderHandler::OnPopupShow
		void setPopupVisibility(bool visible);
		// CefRenderHandler::OnPaint
		void drawIntoTexture(const CefRenderHandler::RectList &dirtyRects, const void *buffer, sint width, sint height, bool popup);
		// CefRenderHandler::GetViewRect, GetScreenInfo
		sint getTextureWidth() const { return _Texture ? _Texture->getImageWidth() : 0; };
		sint getTextureHeight() const { return _Texture ? _Texture->getImageHeight() : 0; };

		int luaBrowse(CLuaState &ls);
		int luaBrowseUndo(CLuaState &ls);
		int luaBrowseRedo(CLuaState &ls);
		int luaRefresh(CLuaState &ls);
		int luaParseHtml(CLuaState &ls);
		int luaZoomIn(CLuaState &ls);
		int luaZoomOut(CLuaState &ls);
		int luaZoomReset(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupWebkit, CInterfaceGroup)
			REFLECT_LUA_METHOD("browse", luaBrowse)
			REFLECT_LUA_METHOD("browseUndo", luaBrowseUndo)
			REFLECT_LUA_METHOD("browseRedo", luaBrowseRedo)
			REFLECT_LUA_METHOD("refresh", luaRefresh)
			REFLECT_LUA_METHOD("zoomIn", luaZoomIn)
			REFLECT_LUA_METHOD("zoomOut", luaZoomOut)
			REFLECT_LUA_METHOD("zoomReset", luaZoomReset)
			REFLECT_LUA_METHOD("parseHtml", luaParseHtml)
			REFLECT_BOOL("grab_keyboard", getGrabKeyboard, setGrabKeyboard)
			REFLECT_STRING("url", getURL, browse)
		REFLECT_EXPORT_END

	protected:
		bool handleKeyEvent(const NLGUI::CEventDescriptorKey &event);
		bool handleMouseEvent(const NLGUI::CEventDescriptorMouse &event);
		void handle();

		void setFocus(bool state);
		void releaseKeyboard();
		void captureKeyboard();

		void setProperty(const std::string &name, const std::string &value);

	private:
		CefRefPtr<CefBrowser> _Browser;
		CefRefPtr<CWebkitHandler> _BrowserHandler;
		CefKeyEvent _KeyEvent;

		// used to draw _Texture and _PopupTexture
		NL3D::UMaterial _Material;
		// main browser window content
		NL3D::UTextureMem *_Texture;
		// popup (select/combo box) content
		NL3D::UTextureMem *_PopupTexture;
		sint _PopupX;
		sint _PopupY;

		// true if mouse is over browser window
		bool _Focused;
		// if keyboard should always be captured when mouse is over window
		bool _GrabKeyboard;
		// if keyboard should be captured after gaining focus again
		bool _RestoreKeyboardCapture;
		// tracks window visibility
		bool _LastActive;

		//
		std::string _TitlePrefix;
		// url that is loaded when doing ::browse('home')
		std::string _Home;
		// currently loaded page url
		std::string _URL;

		// detect resize event
		sint32 _LastParentW;
		sint32 _LastParentH;

		// Status message from Cef
		std::string _StatusMessage;
		std::string _TooltipString;
		bool _Loading;
	};

}// namespace

#endif // CL_GROUP_WEBKIT_H

