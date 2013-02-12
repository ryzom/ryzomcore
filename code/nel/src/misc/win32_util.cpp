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

#include "stdmisc.h"
#include "nel/misc/win32_util.h"
#include "nel/misc/i18n.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


//*************************************************************************************
void CWin32Util::localizeWindow(HWND wnd)
{
	if (!wnd) return;
	int textLength = GetWindowTextLength(wnd);
	if (textLength > 0)
	{
		std::vector<char> str(textLength + 1);
		GetWindowText(wnd, &str[0], textLength + 1);
		std::string winText(str.begin(), str.end() - 1);
		if (CI18N::hasTranslation(winText))
		{
			SetWindowTextW(wnd, (const WCHAR *) CI18N::get(winText).c_str());
		}
	}
	HWND currSon = GetWindow(wnd, GW_CHILD);
	if (currSon)
	{
		HWND lastSon = GetWindow(currSon, GW_HWNDLAST);
		for(;;)
		{
			localizeWindow(currSon);
			if (currSon == lastSon) break;
			currSon = GetWindow(currSon, GW_HWNDNEXT);
		}
	}
}

//*************************************************************************************
void CWin32Util::appendChildWindows(HWND parentWnd, std::vector<HWND> &childWindows)
{
	if (!parentWnd) return;
	HWND currSon = GetWindow(parentWnd, GW_CHILD);
	if (currSon)
	{
		HWND lastSon = GetWindow(currSon, GW_HWNDLAST);
		for(;;)
		{
			childWindows.push_back(currSon);
			appendChildWindows(currSon, childWindows);
			if (currSon == lastSon) break;
			currSon = GetWindow(currSon, GW_HWNDNEXT);
		}
	}
}





} // NLMISC


#endif
