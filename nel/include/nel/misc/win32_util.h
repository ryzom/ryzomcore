// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_WIN32_UTIL_H
#define NL_WIN32_UTIL_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>

namespace NLMISC
{


	struct CWin32Util
	{
		/** replace all occurence of 'uiIdentifier' in a window with their localized versions
		  * (from CI18N)
		  */
		static void localizeWindow(HWND wnd);
		// Append all child windows of a parent window into a vector
		static void appendChildWindows(HWND parentWnd, std::vector<HWND> &childWindows);
	};


} // NLMISC


#endif // NL_OS_WINDOWS


#endif
