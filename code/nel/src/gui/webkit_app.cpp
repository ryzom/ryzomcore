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

#include "nel/gui/webkit_app.h"

#include <cef_version.h>

namespace NLGUI {

	// ***************************************************************
	static bool isWebkitInitialized = false;
	bool webkitInitialize(const CefMainArgs &args, const std::string &locale, const std::string &userAgent)
	{
		if (isWebkitInitialized)
		{
			nlerror("webkitInitialize called twice");
			return false;
		}
		isWebkitInitialized = true;

		// FIXME: could use lazy loading and disable webkit in CGroupWebkit when initialize fails
		//printf(">> Cef initialize\n");

		CefSettings settings;

		// Specify the path for the sub-process webhelper executable.
		//CefString(&settings.browser_subprocess_path).FromASCII("/path/to/webhelper.exe");

		// Use own cache directory that is not in ryzom search path
		CefString(&settings.cache_path).FromASCII("cache_cef3");

		CefString(&settings.locale).FromString(locale);

		// FIXME: cannot use 'Ryzom' as useragent, because that is used by webpages to detect ingame browser
		//CefString(&settings.product_version).FromString(userAgent);

		CefRefPtr<CWebkitApp> app(new CWebkitApp);
		bool success = CefInitialize(args, settings, app.get(), NULL);

		return success;
	}

	// Automatically called from CWebkitApp destructor
	void webkitShutdown()
	{
		//printf(">> CEF shutdown\n");
		if (!isWebkitInitialized)
			return;

		isWebkitInitialized = false;
		CefShutdown();
	}

	// message from render thread
	const char kFocusedNodeChangedMessage[] = "ClientRenderer.FocusedNodeChanged";

	// shutdown Cef if it was initialized
	CWebkitApp::~CWebkitApp()
	{
		if (isWebkitInitialized)
			webkitShutdown();
	}

	// CefRenderProcessHandler
	void CWebkitApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefDOMNode> node)
	{
		//CEF_REQUIRE_RENDERER_THREAD();

		bool is_editable = (node.get() && node->IsEditable());
		if (is_editable != _LastIsEditable)
		{
			//printf(">> WebkitApp::OnFocusedNodeChanged\n");
			_LastIsEditable = is_editable;

			CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
			message->GetArgumentList()->SetBool(0, is_editable);
			browser->SendProcessMessage(PID_BROWSER, message);
		}
	}

	// return CEF version
	std::string CWebkitApp::getChromeVersion()
	{
		// Chrome/39.0.2171.95
		char buffer[256];
		sprintf(buffer, "Chrome/%d.%d.%d.%d", CHROME_VERSION_MAJOR, CHROME_VERSION_MINOR, CHROME_VERSION_BUILD, CHROME_VERSION_PATCH);

		return std::string(buffer);
	}

} // namespace

