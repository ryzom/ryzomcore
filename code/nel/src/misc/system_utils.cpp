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
#include "nel/misc/system_utils.h"

#ifdef NL_OS_WINDOWS
	#define NOMINMAX
	#include <windows.h>

	#ifdef _WIN32_WINNT_WIN7
		// only supported by Windows 7 Platform SDK
		#include <ShObjIdl.h>
		#define TASKBAR_PROGRESS 1
	#endif
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

using namespace std;

// Key in registry
static string RootKey;
static const uint32 KeyMaxLength = 1024;

namespace NLMISC {

nlWindow CSystemUtils::s_window = EmptyWindow;

bool CSystemUtils::init()
{
#ifdef NL_OS_WINDOWS
	// initialize COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) return false;
#endif

	return true;
}

bool CSystemUtils::uninit()
{
#ifdef NL_OS_WINDOWS
	// uninitialize COM
	CoUninitialize();
#endif

	return true;
}

void CSystemUtils::setWindow(nlWindow window)
{
	s_window = window;
}

bool CSystemUtils::updateProgressBar(uint value, uint total)
{
#ifdef TASKBAR_PROGRESS
	if (s_window == NULL)
	{
		nlwarning("No window has be set with CSystemUtils::setWindow(), progress bar can't be displayed");
		return false;
	}

	ITaskbarList3 *pTaskbarList = NULL;

	// instanciate the taskbar control COM object
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskbarList));
	// error can be ignored because Windows versions before Windows 7 doesn't support it
	if (FAILED(hr) || !pTaskbarList) return false;

	if (total)
	{
		// update the taskbar progress
		hr = pTaskbarList->SetProgressValue(s_window, (ULONGLONG)value, (ULONGLONG)total);
	}
	else
	{
		// don't update anymore the progress
		hr = pTaskbarList->SetProgressState(s_window, value == 0 ? TBPF_INDETERMINATE:TBPF_NOPROGRESS);
	}

	// release the interface
	pTaskbarList->Release();

#endif // TASKBAR_PROGRESS

	return true;
}

bool CSystemUtils::copyTextToClipboard(const ucstring &text)
{
	if (!text.size()) return false;

	bool res = false;

#ifdef NL_OS_WINDOWS
	if (OpenClipboard(NULL))
	{
		// check if unicode format is supported by clipboard
		bool isUnicode = (IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE);

		// allocates a buffer to copy text in global memory
		HGLOBAL mem = GlobalAlloc(GHND|GMEM_DDESHARE, (text.size()+1) * (isUnicode ? 2:1));

		if (mem)
		{
			// create a lock on this buffer
			void *hLock = GlobalLock(mem);

			// copy text to this buffer
			if (isUnicode)
				wcscpy((wchar_t*)hLock, (const wchar_t*)text.c_str());
			else
				strcpy((char*)hLock, text.toString().c_str());

			// unlock buffer
			GlobalUnlock(mem);

			// empty clipboard
			EmptyClipboard();

			// set new data to clipboard in the right format
			SetClipboardData(isUnicode ? CF_UNICODETEXT:CF_TEXT, mem);

			res = true;
		}

		CloseClipboard();
	}
#endif

	return res;
}

bool CSystemUtils::pasteTextFromClipboard(ucstring &text)
{
	bool res = false;

#ifdef NL_OS_WINDOWS
	if (OpenClipboard(NULL))
	{
		// check if unicode format is supported by clipboard
		bool isUnicode = (IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE);

		// get data from clipboard (if not of this type, they are converted)
		// warning, this code can't be debuggued in VC++ IDE, hObj will be always NULL
		HANDLE hObj = GetClipboardData(isUnicode ? CF_UNICODETEXT:CF_TEXT);

		if (hObj)
		{
			// create a lock on clipboard data
			void *hLock = GlobalLock(hObj);

			if (hLock != NULL)
			{
				// retrieve clipboard data
				if (isUnicode)
					text = (const ucchar*)hLock;
				else
					text = (const char*)hLock;

				// unlock data
				GlobalUnlock(hObj);

				res = true;
			}
		}

		CloseClipboard();
	}
#endif

	return res;
}

bool CSystemUtils::supportUnicode()
{
	static bool init = false;
	static bool unicodeSupported = false;
	if (!init)
	{
		init = true;
#ifdef NL_OS_WINDOWS
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// get Windows version
		if (GetVersionEx(&osvi))
		{
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				// unicode is supported since Windows NT 4.0
				if (osvi.dwMajorVersion >= 4)
				{
					unicodeSupported = true;
				}
			}
		}
#else
		unicodeSupported = true;
#endif
	}
	return unicodeSupported;
}

bool CSystemUtils::isAzertyKeyboard()
{
#ifdef NL_OS_WINDOWS
	uint16 klId = uint16((uint32)GetKeyboardLayout(0) & 0xFFFF);
	// 0x040c is French, 0x080c is Belgian
	if (klId == 0x040c || klId == 0x080c)
		return true;
#endif
	return false;
}

bool CSystemUtils::isScreensaverEnabled()
{
	bool res = false;
#ifdef NL_OS_WINDOWS
//	old code, is not working anymore
//	BOOL bRetValue;
//	SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &bRetValue, 0);
//	res = (bRetValue == TRUE);
	HKEY hKeyScreenSaver = NULL;
	LSTATUS lReturn = RegOpenKeyExA(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_QUERY_VALUE, &hKeyScreenSaver);
	if (lReturn == ERROR_SUCCESS)
	{
		DWORD dwType = 0L;
		DWORD dwSize = KeyMaxLength;
		unsigned char Buffer[KeyMaxLength] = {0};

		lReturn = RegQueryValueExA(hKeyScreenSaver, TEXT("SCRNSAVE.EXE"), NULL, &dwType, NULL, &dwSize);
		// if SCRNSAVE.EXE is present, check also if it's empty
		if (lReturn == ERROR_SUCCESS)
			res = (Buffer[0] != '\0');
	}
	RegCloseKey(hKeyScreenSaver);
#endif
	return res;
}

bool CSystemUtils::enableScreensaver(bool screensaver)
{
	bool res = false;
#ifdef NL_OS_WINDOWS
	res = (SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, screensaver ? TRUE:FALSE, NULL, 0) == TRUE);
#endif
	return res;
}

std::string CSystemUtils::getRootKey()
{
	return RootKey;
}

void CSystemUtils::setRootKey(const std::string &root)
{
	RootKey = root;
}

string CSystemUtils::getRegKey(const string &Entry)
{
	string ret;
#ifdef NL_OS_WINDOWS
	HKEY hkey;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, RootKey.c_str(), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		DWORD	dwType	= 0L;
		DWORD	dwSize	= KeyMaxLength;
		unsigned char	Buffer[KeyMaxLength];

		if(RegQueryValueEx(hkey, Entry.c_str(), NULL, &dwType, Buffer, &dwSize) != ERROR_SUCCESS)
		{
			nlwarning("Can't get the reg key '%s'", Entry.c_str());
		}
		else
		{
			ret = (char*)Buffer;
		}
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't get the reg key '%s'", Entry.c_str());
	}
#endif
	return ret;
}

bool CSystemUtils::setRegKey(const string &ValueName, const string &Value)
{
	bool res = false;
#ifdef NL_OS_WINDOWS
	HKEY hkey;
	DWORD dwDisp;

	if (RegCreateKeyExA(HKEY_CURRENT_USER, RootKey.c_str(), 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
	{
		if (RegSetValueExA(hkey, ValueName.c_str(), 0L, REG_SZ, (const BYTE *)Value.c_str(), (DWORD)(Value.size())+1) == ERROR_SUCCESS)
			res = true;
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't set the reg key '%s' '%s'", ValueName.c_str(), Value.c_str());
	}
#endif

	return res;
}

uint CSystemUtils::getCurrentColorDepth()
{
	uint depth = 0;

#ifdef NL_OS_WINDOWS
	HWND desktopWnd = GetDesktopWindow();
	if (desktopWnd)
	{
		HDC desktopDC = GetWindowDC(desktopWnd);
		if (desktopDC)
		{
			depth = (uint) GetDeviceCaps(desktopDC, BITSPIXEL);
			ReleaseDC(desktopWnd, desktopDC);
		}
	}
#else
	depth = 24; // temporary fix for compilation under Linux
/*
	Display *display = XOpenDisplay(NULL);
	if (display)
	{
		depth = (uint) DefaultDepth(display, DefaultScreen(display));
		XCloseDisplay(display);
	}
*/
#endif
	return depth;
}

} // NLMISC
