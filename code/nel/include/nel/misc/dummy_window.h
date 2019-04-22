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

#ifndef NL_DUMMY_WINDOW_H
#define NL_DUMMY_WINDOW_H


#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS // for win32 os only

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINDOWS
#	define _WIN32_WINDOWS 0x0500
#endif
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#	define WINVER 0x0500
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>


namespace NLMISC
{

/** A simple invisible win32 window, with an optional message handling function.
  * Possible uses include :
  * - Creating an icon in the tray (require a window to be passed)
  * - Creating a message queue (in order to use an IPC mechanism such as WM_COPYDATA)
  * - etc.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2007
  */
class CDummyWindow
{
public:
	CDummyWindow();
	/** Init a dummy window, with an optional message handling procedure
	  * \return true on success
	  */
	bool init(HINSTANCE hInstance, WNDPROC winProc = NULL);
	// release this window
	void release();
	~CDummyWindow();
	// Get this window handle
	HWND getWnd() const { return _HWnd; }
private:
	HWND _HWnd;
};


} // NLMISC


#endif // NL_OS_WINDOWS


#endif
