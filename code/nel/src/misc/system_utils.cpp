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

using namespace std;

namespace NLMISC {

void *CSystemUtils::s_window = NULL;

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

void CSystemUtils::setWindow(void *window)
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
		hr = pTaskbarList->SetProgressValue((HWND)s_window, (ULONGLONG)value, (ULONGLONG)total);
	}
	else
	{
		// don't update anymore the progress
		hr = pTaskbarList->SetProgressState((HWND)s_window, value == 0 ? TBPF_INDETERMINATE:TBPF_NOPROGRESS);
	}

	// release the interface
	pTaskbarList->Release();

#endif // TASKBAR_PROGRESS

	return true;
}

} // NLMISC
