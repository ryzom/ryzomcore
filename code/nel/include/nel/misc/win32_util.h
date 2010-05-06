#ifndef NL_WIN32_UTIL_H
#define NL_WIN32_UTIL_H


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
