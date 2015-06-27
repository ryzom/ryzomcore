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

#ifndef CL_WEBKIT_APP_H
#define CL_WEBKIT_APP_H

#include "nel/misc/types_nl.h"
#include <cef_app.h>

namespace NLGUI
{

	extern const char kFocusedNodeChangedMessage[];

	// ***************************************************************************
	/**
	 * Cef client app
	 *
	 * \author Meelis Mägi
	 * \date 2015
	 */
	class CWebkitApp: public CefApp,
		public CefRenderProcessHandler
	{
	public:
		CWebkitApp(): _LastIsEditable(false)
		{
		}

		~CWebkitApp();

		// CefApp
		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
		{
			return this;
		}

		// CefRenderProcessHandler
		virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefFrame> frame,
				CefRefPtr<CefDOMNode> node) OVERRIDE;

		// Chrome/39.0.2171.95
		static std::string getChromeVersion();

	private:
		bool _LastIsEditable;

		IMPLEMENT_REFCOUNTING(CWebkitApp);
	};

	// Initializes CEF3 for main thread
	bool webkitInitialize(const CefMainArgs &args, const std::string &locale, const std::string &userAgent);
	void webkitShutdown();

} // namespace

#endif // CL_WEBKIT_APP_H

