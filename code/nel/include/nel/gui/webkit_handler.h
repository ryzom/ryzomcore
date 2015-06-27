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

#ifndef CL_WEBKIT_HANDLER_H
#define CL_WEBKIT_HANDLER_H

#include <cef_client.h>
#include <wrapper/cef_helpers.h>

namespace NLGUI {

	class CGroupWebkit;

	// ***************************************************************************
	/**
	 * Cef browser client
	 *
	 * \author Meelis Mägi
	 * \date 2015
	 */
	class CWebkitHandler: public CefClient,
		public CefLifeSpanHandler,
		public CefLoadHandler,
		public CefDisplayHandler,
		public CefJSDialogHandler,
		public CefRenderHandler
	{
	public:
		CWebkitHandler();

		void setWindow(CGroupWebkit *win);

		// CefClient
		virtual CefRefPtr<CefRenderHandler> GetRenderHandler() { return this; }
		virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler(){ return this; }
		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() { return this; }
		virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() { return this; }
		virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() { return this; }

		// CefClient
		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
				CefProcessId source_process,
				CefRefPtr<CefProcessMessage> message) OVERRIDE;

		// CefLifeSpanHandler
		virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefFrame> frame,
				const CefString& target_url,
				const CefString& target_frame_name,
				const CefPopupFeatures& popupFeatures,
				CefWindowInfo& windowInfo,
				CefRefPtr<CefClient>& client,
				CefBrowserSettings& settings,
				bool* no_javascript_access) OVERRIDE;

		// CefLoadHandler
		void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
				bool isLoading,
				bool canGoBack,
				bool canGoForward) OVERRIDE;

		// CefLoadHandler
		void OnLoadError(CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefFrame> frame,
				ErrorCode errorCode,
				const CefString &errorText,
				const CefString &failedUrl) OVERRIDE;

		// CefDisplayHandler
		virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;

		// CefDisplayHandler
		virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) OVERRIDE;

		// CefDisplayHandler
		virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) OVERRIDE;

		// CefDisplayHandler
		virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
				const CefString& message,
				const CefString& source,
				int line) OVERRIDE;

		// CefJSDialogHandler
		virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
				const CefString& origin_url,
				const CefString& accept_lang,
				JSDialogType dialog_type,
				const CefString& message_text,
				const CefString& default_prompt_text,
				CefRefPtr<CefJSDialogCallback> callback,
				bool& suppress_message) OVERRIDE;

		//CefJSDialogHandler
		virtual void OnResetDialogState(CefRefPtr<CefBrowser> browser) OVERRIDE;

		// CefRenderHandler
		virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) OVERRIDE;
		virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
		virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
		virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;
		virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) OVERRIDE;
		virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info) OVERRIDE;

		// called from action handler after ui dialog is closed
		static void jsDialogContinue(bool success, const std::string &input);

	private:
		CGroupWebkit *_Window;

		static CefRefPtr<CefJSDialogCallback> _JSDialogCallback;

		IMPLEMENT_REFCOUNTING(CWebkitHandler);
	};

}// namespace

#endif // CL_WEBKIT_HANDLER_H

