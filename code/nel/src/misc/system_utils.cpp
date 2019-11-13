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
#define INITGUID
#include <ddraw.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#ifdef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#endif
#ifdef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#endif
#include <dxgi.h>
#include <initguid.h>
#include <CGuid.h>
#	include <ObjBase.h>
#	ifdef _WIN32_WINNT_WIN7
		// only supported by Windows 7 Platform SDK
#		include <ShObjIdl.h>
#		define TASKBAR_PROGRESS 1
#	endif
#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
#include "nel/misc/file.h"
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

#ifdef NL_OS_WINDOWS
static bool s_mustUninit = false;
#endif

bool CSystemUtils::init()
{
#ifdef NL_OS_WINDOWS
	// initialize COM
	if (!s_mustUninit)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr)) return false;

		s_mustUninit = true;
	}
#endif

	return true;
}

bool CSystemUtils::uninit()
{
#ifdef NL_OS_WINDOWS
	// uninitialize COM
	if (s_mustUninit)
	{
		CoUninitialize();

		s_mustUninit = false;
	}
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
		nldebug("No window has be set with CSystemUtils::setWindow(), progress bar can't be displayed");
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
	if (text.empty()) return false;

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
		OSVERSIONINFOA osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// get Windows version
		if (GetVersionExA(&osvi))
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
	uint16 klId = uint16((uintptr_t)GetKeyboardLayout(0) & 0xFFFF);
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
	LSTATUS lReturn = RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_QUERY_VALUE, &hKeyScreenSaver);
	if (lReturn == ERROR_SUCCESS)
	{
		DWORD dwType = 0L;
		DWORD dwSize = KeyMaxLength;
		unsigned char Buffer[KeyMaxLength] = {0};

		lReturn = RegQueryValueExA(hKeyScreenSaver, "SCRNSAVE.EXE", NULL, &dwType, NULL, &dwSize);
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

string CSystemUtils::getRegKey(const string &entry)
{
	string ret;
#ifdef NL_OS_WINDOWS
	HKEY hkey;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, nlUtf8ToWide(RootKey), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		DWORD	dwType	= 0L;
		DWORD	dwSize	= KeyMaxLength;
		wchar_t buffer[KeyMaxLength];

		if (RegQueryValueExW(hkey, nlUtf8ToWide(entry), NULL, &dwType, (LPBYTE)buffer, &dwSize) != ERROR_SUCCESS)
		{
			nlwarning("Can't get the reg key '%s'", entry.c_str());
		}
		else
		{
			ret = wideToUtf8(buffer);
		}

		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't get the reg key '%s'", entry.c_str());
	}
#endif
	return ret;
}

bool CSystemUtils::setRegKey(const string &valueName, const string &value)
{
	bool res = false;
#ifdef NL_OS_WINDOWS
	HKEY hkey;
	DWORD dwDisp;

	if (RegCreateKeyExW(HKEY_CURRENT_USER, nlUtf8ToWide(RootKey), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
	{
		// we must use the real Unicode string size in bytes
		std::wstring wvalue = nlUtf8ToWide(value);
		if (RegSetValueExW(hkey, nlUtf8ToWide(valueName), 0, REG_SZ, (const BYTE *)wvalue.c_str(), (wvalue.size() + 1) * sizeof(WCHAR)) == ERROR_SUCCESS)
			res = true;
		RegCloseKey(hkey);
	}
	else
	{
		nlwarning("Can't set the reg key '%s' '%s'", valueName.c_str(), value.c_str());
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

/// Detect whether the current process is a windowed application. Return true if definitely yes, false if unknown
bool CSystemUtils::detectWindowedApplication()
{
#ifdef NL_OS_WINDOWS
	if (GetConsoleWindow() == NULL)
		return true;
#endif
	return false;
}

#ifdef NL_OS_WINDOWS
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }
#endif

typedef HRESULT (WINAPI* LPDIRECTDRAWCREATE)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI* LPCREATEDXGIFACTORY)(REFIID, void**);

static std::string FormatError(HRESULT hr)
{
	return NLMISC::toString("%s (0x%x)", formatErrorMessage(hr).c_str(), hr);
}

struct SAdapter
{
	uint id;
	std::string name;
	sint memory;
	GUID guid;
	HMONITOR hMonitor;
	bool found;

	SAdapter()
	{
		id = 0;
		memory = -1;
		guid = GUID_NULL;
		hMonitor = NULL;
		found = false;
	}
};

static std::list<SAdapter> s_dxgiAdapters;

static void EnumerateUsingDXGI(IDXGIFactory *pDXGIFactory)
{
	nlassert(pDXGIFactory != NULL);

	for(uint index = 0; ; ++index)
	{
		IDXGIAdapter *pAdapter = NULL;
		HRESULT hr = pDXGIFactory->EnumAdapters(index, &pAdapter);
		// DXGIERR_NOT_FOUND is expected when the end of the list is hit
		if (FAILED(hr)) break;

		DXGI_ADAPTER_DESC desc;
		memset(&desc, 0, sizeof(DXGI_ADAPTER_DESC));

		if (SUCCEEDED(pAdapter->GetDesc(&desc)))
		{
			SAdapter adapter;
			adapter.id = index;
			adapter.name = wideToUtf8(desc.Description);
			adapter.memory = desc.DedicatedVideoMemory / 1024;
			adapter.found = true;

			nldebug("DXGI Adapter: %u - %s - DedicatedVideoMemory: %d KiB", index, adapter.name.c_str(), adapter.memory);

			s_dxgiAdapters.push_back(adapter);
		}

		SAFE_RELEASE(pAdapter);
	}
}

BOOL WINAPI DDEnumCallbackEx(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	SAdapter * pAdapter = (SAdapter*)lpContext;

	if (pAdapter->hMonitor == hm)
	{
		pAdapter->name = lpDriverDescription;
		pAdapter->guid = *lpGUID;
		pAdapter->found = true;
	}

	return TRUE;
}

#endif

sint CSystemUtils::getTotalVideoMemory()
{
	sint res = -1;

#if defined(NL_OS_WINDOWS)
	// using DXGI
	HINSTANCE hDXGI = LoadLibraryA("dxgi.dll");

	if (hDXGI)
	{

		// We prefer the use of DXGI 1.1
		LPCREATEDXGIFACTORY pCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory1");

		if (!pCreateDXGIFactory)
		{
			pCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(hDXGI, "CreateDXGIFactory");
		}

		if (pCreateDXGIFactory)
		{
			IDXGIFactory *pDXGIFactory = NULL;
			HRESULT hr = pCreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&pDXGIFactory);

			if (SUCCEEDED(hr))
			{
				EnumerateUsingDXGI(pDXGIFactory);

				SAFE_RELEASE(pDXGIFactory);

				if (!s_dxgiAdapters.empty())
				{
					// TODO: determine what adapter is used by NeL
					res = s_dxgiAdapters.front().memory;
				}
				else
				{
					nlwarning("Unable to find an DXGI adapter");
				}
			}
			else
			{
				nlwarning("Unable to create DXGI factory");
			}
		}
		else
		{
			nlwarning("dxgi.dll missing entry-point");
		}

		FreeLibrary(hDXGI);
	}

	if (res == -1)
	{
		// using DirectDraw
		HMODULE hInstDDraw = LoadLibraryA("ddraw.dll");

		if (hInstDDraw)
		{
			SAdapter adapter;
			adapter.hMonitor = MonitorFromWindow(s_window, MONITOR_DEFAULTTONULL);

			LPDIRECTDRAWENUMERATEEXA pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(hInstDDraw, "DirectDrawEnumerateExA");
			LPDIRECTDRAWCREATE pDDCreate = (LPDIRECTDRAWCREATE)GetProcAddress(hInstDDraw, "DirectDrawCreate");

			if (pDirectDrawEnumerateEx && pDDCreate)
			{
				HRESULT hr = pDirectDrawEnumerateEx(DDEnumCallbackEx, (VOID*)&adapter, DDENUM_ATTACHEDSECONDARYDEVICES);

				if (SUCCEEDED(hr) && adapter.found)
				{
					LPDIRECTDRAW pDDraw = NULL;
					hr = pDDCreate(&adapter.guid, &pDDraw, NULL);

					if (SUCCEEDED(hr))
					{
						LPDIRECTDRAW7 pDDraw7 = NULL;
						hr = pDDraw->QueryInterface(IID_IDirectDraw7, (VOID**)&pDDraw7);

						if (SUCCEEDED(hr))
						{
							DDSCAPS2 ddscaps;
							memset(&ddscaps, 0, sizeof(DDSCAPS2));
							ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

							DWORD pdwAvailableVidMem;
							hr = pDDraw7->GetAvailableVidMem(&ddscaps, &pdwAvailableVidMem, NULL);

 							if (SUCCEEDED(hr))
							{
								res = (sint)pdwAvailableVidMem / 1024;
								nlinfo("DirectDraw Adapter: %s - DedicatedVideoMemory: %d KiB", adapter.name.c_str(), adapter.memory);
							}
							else
							{
								nlwarning("Unable to get DirectDraw available video memory: %s", FormatError(hr).c_str());
							}

							SAFE_RELEASE(pDDraw7);
						}
						else
						{
							nlwarning("Unable to query IDirectDraw7 interface: %s", FormatError(hr).c_str());
						}
					}
					else
					{
						nlwarning("Unable to call DirectDrawCreate: %s", FormatError(hr).c_str());
					}
				}
				else
				{
					nlwarning("Unable to enumerate DirectDraw adapters (%s): %s", (adapter.found ? "found":"not found"), FormatError(hr).c_str());
				}
			}
			else
			{
				nlwarning("Unable to get pointer on DirectDraw functions (DirectDrawEnumerateExA %p, DirectDrawCreate %p)", pDirectDrawEnumerateEx, pDDCreate);
			}

			FreeLibrary(hInstDDraw);
		}
		else
		{
			nlwarning("Unable to load ddraw.dll");
		}
	}
#elif defined(NL_OS_MAC)
	// the right method is using OpenGL
#else
	if (res == -1)
	{
		// use nvidia-smi
		std::string command = "nvidia-smi -q -d MEMORY";

		std::string out = getCommandOutput(command);

		if (out.empty())
		{
			nlwarning("Unable to launch %s", command.c_str());
		}
		else
		{
			std::vector<std::string> lines;
			explode(out, std::string("\n"), lines, true);
	
			// process each line
			for(uint i = 0; i < lines.size(); ++i)
			{
				//        Total                   : 62 MB

				std::string line = lines[i];
	
				// find Total line
				std::string::size_type pos = line.find("Total");
				if (pos == std::string::npos) continue;
				pos += 6;

				// find separator
				pos = line.find(':', pos);
				if (pos == std::string::npos) continue;
				pos += 2;

				// find units
				std::string::size_type posUnits = line.find(' ', pos);
				if (posUnits == std::string::npos) continue;
				++posUnits;

				// found device ID
				std::string memory = line.substr(pos, posUnits-pos-1);
				std::string units = line.substr(posUnits);

				// convert video memory to sint
				if (NLMISC::fromString(memory, res))
				{
					if (units == "MB")
					{
						res *= 1024;
					}
					else if (units == "GB")
					{
						res *= 1024 * 1024;
					}
					else
					{
						// reset to use other methods
						res = -1;

						nlwarning("nvidia-smi reported %d %s as wrong video memory units", res, units.c_str());
						break;
					}

					nlinfo("nvidia-smi reported %d KiB of video memory", res);
				}
				else
				{
					// reset to use other methods
					res = -1;
				}

				break;
			}
		}
	}

	if (res == -1)
	{
		// under Linux, no method is really reliable...
		NLMISC::CIFile file;
	
		std::string logFile = "/var/log/Xorg.0.log";

		// parse last Xorg.0.log
		if (file.open(logFile, true))
		{
			char buffer[256];

			while(!file.eof())
			{
				file.getline(buffer, 256);
			
				if (buffer[0] == '\0') break;

				std::string line(buffer);

				// nvidia driver
				std::string::size_type pos = line.find(") NVIDIA(");

				if (pos != std::string::npos)
				{
					// [    20.883] (--) NVIDIA(0): Memory: 2097152 kBytes
					// [    28.515] (--) NVIDIA(0): Memory: 262144 kBytes
					pos = line.find("Memory: ", pos);

					// found memory line
					if (pos == std::string::npos) continue;
					pos += 8;

					std::string::size_type posUnits = line.find(" kBytes", pos);

					// found units in KiB
					if (posUnits == std::string::npos) continue;

					std::string videoMemory = line.substr(pos, posUnits-pos);
					
					if (!NLMISC::fromString(videoMemory, res)) continue;

					nlinfo("Xorg NVIDIA driver reported %d KiB of video memory", res);
					break;
				}

				// intel driver
				pos = line.find(") intel(");

				if (pos != std::string::npos)
				{
					// (**) intel(0): VideoRam: 131072 KB
					pos = line.find("VideoRam: ", pos);

					// found memory line
					if (pos == std::string::npos) continue;
					pos += 10;

					std::string::size_type posUnits = line.find(" KB", pos);

					// found units in KiB
					if (posUnits == std::string::npos) continue;

					std::string videoMemory = line.substr(pos, posUnits-pos);
					
					if (!NLMISC::fromString(videoMemory, res)) continue;

					nlinfo("Xorg Intel driver reported %d KiB of video memory", res);
					break;
				}

				// TODO: other drivers: fglrx (ATI), radeon (ATI)
			}

			file.close();
		}
	}

	if (res == -1)
	{
		// use lspci
		std::string command = "lspci";

		std::string out = getCommandOutput(command);

		if (out.empty())
		{
			nlwarning("Unable to launch %s", command.c_str());
		}
		else
		{
			std::vector<std::string> lines;
			std::string deviceId;

			explode(out, std::string("\n"), lines, true);
	
			// process each line
			for(uint i = 0; i < lines.size(); ++i)
			{
				std::string line = lines[i];
	
				if (line.find("VGA") == std::string::npos &&
					line.find("3D") == std::string::npos &&
					line.find("2D") == std::string::npos)
					continue;

				std::string::size_type pos = line.find(' ');
			
				if (pos == std::string::npos) continue;

				// found device ID
				deviceId = line.substr(0, pos);
				break;
			}

			if (deviceId.empty())
			{
				nlwarning("Unable to find a 3D device with lspci");
			}
			else
			{
				command = "lspci -v -s " + deviceId;

				out = getCommandOutput(command);

				if (out.empty())
				{
					nlwarning("Unable to launch %s", command.c_str());
				}
				else
				{
					explode(out, std::string("\n"), lines, true);

					// process each line
					for(uint i = 0; i < lines.size(); ++i)
					{
						std::string line = lines[i];
	
						// look for a size
						std::string::size_type pos0 = line.find("[size=");
						if (pos0 == std::string::npos) continue;

						// move to first digit
						pos0 += 6;

						// end of the size
						std::string::size_type pos1 = line.find("]", pos0);
						if (pos1 == std::string::npos) continue;

						sint units;

						if (line.substr(pos1-1, 1) == "M")
						{
							// size in MiB
							units = 1024;
							--pos1;
						}
						else if (line.substr(pos1-1, 1) == "K")
						{
							// size in KiB
							units = 1;
							--pos1;
						}
						else
						{
							// size in B
							units = 0;
						}

						// extract the size
						std::string sizeStr = line.substr(pos0, pos1-pos0);

						// convert size to integer with right units
						sint tmpSize;
						if (!NLMISC::fromString(sizeStr, tmpSize)) continue;

						tmpSize *= units;

						// take the higher size (up to 256 MiB apparently)
						if (tmpSize > res) res = tmpSize;
					}

					nlinfo("lspci reported %d KiB of video memory", res);
				}
			}
		}
	}
#endif

	return res;
}

} // NLMISC
