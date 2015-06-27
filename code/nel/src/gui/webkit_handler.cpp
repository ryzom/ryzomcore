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

#include "nel/gui/webkit_app.h"
#include "nel/gui/webkit_handler.h"
#include "nel/gui/group_webkit.h"
#include "nel/gui/view_text.h"
#include "nel/gui/group_editbox.h"

// gui window id for javascript dialog
#define WIN_WEBKIT_JS_ALERT "ui:interface:webkit_js_alert"
#define WIN_WEBKIT_JS_CONFIRM "ui:interface:webkit_js_confirm"
#define WIN_WEBKIT_JS_PROMPT "ui:interface:webkit_js_prompt"

namespace NLGUI {

	// global pointer for last javscript dialog callback
	CefRefPtr<CefJSDialogCallback> CWebkitHandler::_JSDialogCallback = NULL;

	CWebkitHandler::CWebkitHandler():
		_Window(NULL)
	{
	}

	void CWebkitHandler::setWindow(CGroupWebkit *win)
	{
		_Window = win;
	}

	// CefClient
	bool CWebkitHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message)
	{
		CEF_REQUIRE_UI_THREAD();

		// message from render thread
		//printf(">> OnProcessMessageReceived: process id:%d, name: %s\n",
		//		source_process, message->GetName().ToString().c_str());

		std::string name = message->GetName();
		if (name == kFocusedNodeChangedMessage)
		{
			bool is_editable = message->GetArgumentList()->GetBool(0);
			if (_Window)
				_Window->setKeyboardCapture(is_editable);
			return true;
		}

		return false;
	}

	// CefLifeSpanHandler
	bool CWebkitHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString& target_url,
			const CefString& target_frame_name,
			const CefPopupFeatures& popupFeatures,
			CefWindowInfo& windowInfo,
			CefRefPtr<CefClient>& client,
			CefBrowserSettings& settings,
			bool* no_javascript_access)
	{
		CEF_REQUIRE_IO_THREAD();

		// FIXME: inject new browser using template xml, set url and set active, centered
		std::string url(target_url);

		// load popup into main window
		if (target_url.size() > 0)
			browser->GetMainFrame()->LoadURL(target_url);

		// block popup
		return true;
	}

	// CefDisplayHandler
	void CWebkitHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setTitle(title.ToString());
	}

	// CefDisplayHandler
	bool CWebkitHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setTooltip(text.ToString());

		return true;
	}

	// CefRenderHandler
	bool CWebkitHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
	{
		CEF_REQUIRE_UI_THREAD();

		CefRect rect = CefRect(0, 0, 0, 0);
		if (_Window)
		{
			rect.width = _Window->getTextureWidth();
			rect.height = _Window->getTextureHeight();
		}

		screen_info = CefScreenInfo(
				1.0f, // scale factor
				32, // depth
				8, // bits per component
				false, // monochrome
				rect, // rect - maximum size
				rect // available rect - fullscreen maximum size
				);

		return true;
	}

	// CefRenderHandler
	bool CWebkitHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
	{
		CEF_REQUIRE_UI_THREAD();

		rect = CefRect(0, 0, 0, 0);
		if (_Window)
		{
			rect.width = _Window->getTextureWidth();
			rect.height = _Window->getTextureHeight();
		}

		return true;
	}

	// CefRenderHandler
	void CWebkitHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setPopupVisibility(show);
	}

	// CefRenderHandler
	void CWebkitHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setPopupSize(rect);
	}

	// CefRenderHandler
	void CWebkitHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->drawIntoTexture(dirtyRects, buffer, width, height, type == PET_POPUP);
	}

	// CefRenderHandler
	void CWebkitHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
			CefCursorHandle cursor,
			CursorType type,
			const CefCursorInfo& custom_cursor_info)
	{
		//printf(">> OnCursorChange: %d\n", type);
		// FIXME: type=CT_CUSTOM, custom_cursor_info has cursor image
		std::string cursorTexture;
		switch(type)
		{
			case CT_CROSS:
				cursorTexture = "curs_default.tga";
				break;
			case CT_HAND:
				cursorTexture = "curs_pick.tga";
				break;
			case CT_IBEAM:
				cursorTexture = "curs_pick_dup.tga";
				break;
			case CT_WAIT:
				cursorTexture = "curs_rotate.tga";
				break;
			case CT_HELP:
				cursorTexture = "curs_help.tga";
				break;
			default:
				cursorTexture = "curs_default.tga";
		}

		// FIXME: works on login screen, but not ingame
		CViewPointer *vp = dynamic_cast<CViewPointer *>(CWidgetManager::getInstance()->getPointer());
		if(vp)
		{
			//printf(">> set cursor to [%s]\n", cursorTexture.c_str());
			vp->setStringMode(false);
			vp->setCursor(cursorTexture);
		}
		else
			printf(">> error: cursor pointer\n");
	}

	// CefLoadHandler
	void CWebkitHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
			bool isLoading,
			bool canGoBack,
			bool canGoForward)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setLoadingState(isLoading, canGoBack, canGoForward);
	}

	// CefLoadHandler
	void CWebkitHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			ErrorCode errorCode,
			const CefString &errorText,
			const CefString &failedUrl)
	{
		CEF_REQUIRE_UI_THREAD();

		std::stringstream ss;
		ss << "<html><body bgcolor=\"white\">"
			"<h2>Failed to load URL " << std::string(failedUrl) << " with error " << std::string(errorText) << " (" << errorCode << ")</h2>"
			"</body></html>";
		frame->LoadString(ss.str(), failedUrl);
	}

	// CefDisplayHandler
	void CWebkitHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_Window)
			_Window->setStatusMessage(value.ToString());
	}

	// CefDisplayHandler
	bool CWebkitHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
			const CefString& message,
			const CefString& source,
			int line)
	{
		// Log a console message...
		printf("[%d] %s: %s\n", line, source.ToString().c_str(), message.ToString().c_str());

		return false;
	}

	// CefJSDialogHandler
	bool CWebkitHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
			const CefString& origin_url,
			const CefString& accept_lang,
			JSDialogType dialog_type,
			const CefString& message_text,
			const CefString& default_prompt_text,
			CefRefPtr<CefJSDialogCallback> callback,
			bool& suppress_message)
	{
		CEF_REQUIRE_UI_THREAD();

		CInterfaceGroup* group;
		_JSDialogCallback = NULL;
		switch(dialog_type)
		{
			case JSDIALOGTYPE_ALERT:
				group = CWidgetManager::getInstance()->getWindowFromId(WIN_WEBKIT_JS_ALERT);
				if (group)
				{
					CViewText* text = dynamic_cast<CViewText*>(group->getView("text"));
					if (text)
						text->setText(ucstring(message_text.ToString()));

					_JSDialogCallback = callback;
					CWidgetManager::getInstance()->enableModalWindow(NULL, WIN_WEBKIT_JS_ALERT);
				}
				else
					nldebug("window (%s) for javascript alert dialog not found", WIN_WEBKIT_JS_ALERT);
				break;
			case JSDIALOGTYPE_CONFIRM:
				group = CWidgetManager::getInstance()->getWindowFromId(WIN_WEBKIT_JS_CONFIRM);
				if (group)
				{
					CViewText* text = dynamic_cast<CViewText*>(group->getView("text"));
					if (text)
						text->setText(ucstring(message_text.ToString()));

					_JSDialogCallback = callback;
					CWidgetManager::getInstance()->enableModalWindow(NULL, WIN_WEBKIT_JS_CONFIRM);
				}
				else
					nldebug("window (%s) for javascript confirm dialog not found", WIN_WEBKIT_JS_CONFIRM);
				break;
			case JSDIALOGTYPE_PROMPT:
				group = CWidgetManager::getInstance()->getWindowFromId(WIN_WEBKIT_JS_PROMPT);
				if (group)
				{
					CViewText* text = dynamic_cast<CViewText*>(group->getView("text"));
					if (text)
						text->setText(ucstring(message_text.ToString()));

					CGroupEditBox* eb = dynamic_cast<CGroupEditBox*>(group->getGroup("prompt:eb"));
					if (eb)
						eb->setInputStringAsUtf8(default_prompt_text.ToString());

					_JSDialogCallback = callback;
					CWidgetManager::getInstance()->enableModalWindow(NULL, WIN_WEBKIT_JS_PROMPT);
				}
				else
					nldebug("window (%s) for javascript prompt dialog not found", WIN_WEBKIT_JS_PROMPT);
				break;
			default:
				// ignore dialog
				break;
		}

		return _JSDialogCallback != NULL;
	}

	// CefJSDialogHandler
	void CWebkitHandler::OnResetDialogState(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();

		if (_JSDialogCallback)
		{
			CWidgetManager::getInstance()->popModalWindow();
			_JSDialogCallback = NULL;
		}
	}

	// static
	void CWebkitHandler::jsDialogContinue(bool success, const std::string &input)
	{
		if (_JSDialogCallback)
		{
			_JSDialogCallback->Continue(success, input);

			_JSDialogCallback = NULL;
		}
	}

}// namespace

