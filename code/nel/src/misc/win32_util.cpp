#include "stdmisc.h"
#include "nel/misc/win32_util.h"
#include "nel/misc/i18n.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>

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
