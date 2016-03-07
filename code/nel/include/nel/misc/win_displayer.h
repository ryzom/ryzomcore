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

#ifndef NL_WIN_DISPLAYER_H
#define NL_WIN_DISPLAYER_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS

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

#include "displayer.h"
#include "reader_writer.h"

#include "window_displayer.h"

namespace NLMISC {


/**
 * this displayer displays on a win32 windows.
 * MT = Main Thread, DT = Display Thread
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CWinDisplayer : public NLMISC::CWindowDisplayer
{
public:

	CWinDisplayer(const char *displayerName = "");

	virtual ~CWinDisplayer ();

#ifdef NL_OS_WINDOWS
	HWND getHWnd () const { return _HWnd; }
#endif // NL_OS_WINDOWS

private:

	// called by DT only
	void	resizeLabels ();
	// called by DT only
	void	updateLabels ();

	// called by DT only
	void	open (std::string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log);
	// called by DT only
	void	clear ();
	// called by DT only
	void	display_main ();

	virtual void	setTitleBar (const std::string &titleBar);

	virtual void	getWindowPos (uint32 &x, uint32 &y, uint32 &w, uint32 &h);

	// all these variables above is used only by the DT

	HWND _HEdit, _HWnd, _HInputEdit;
	HFONT _HFont;
	HMODULE _HLibModule;

	CLog *Log;

	// the MT must set the value to true to exit the thread
	bool Exit;

	friend LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_DISPLAYER_H

/* End of win_displayer.h */
