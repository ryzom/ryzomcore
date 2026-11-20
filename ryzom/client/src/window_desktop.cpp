// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdpch.h"

#include "window_desktop.h"

#include "nel/3d/u_driver.h"
#include "nel/misc/cmd_args.h"
#include "nel/misc/report.h"
#include "nel/misc/system_utils.h"

#include "init.h"

using namespace NLMISC;

namespace WindowDesktop {

namespace {

bool s_IsRequested = false;
std::string s_Requested;

/// A crash leaves its report file, without the window of the report tool on the desktop shown, nor the one of the
/// error reporting of the system or of a debugger
void keepCrashesOffDesktopShown()
{
	setReportToolEnabled(false);
#ifdef NL_OS_WINDOWS
	SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
#ifndef NL_COMP_MINGW
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#endif
}

}

void addArgs(CCmdArgs &args)
{
	// known everywhere, so that it is refused where there are no virtual desktops and is never taken for a login; its
	// value is given with '=' only, as the login and the password follow the options
	args.addLongArgWithEquals("desktop", "desktop",
	    "Show the client on this virtual desktop (Windows), by its number (1 for the first one) or the name it was given in the task view; it is shown there "
	    "without taking the focus, and the desktop shown stays as it is");
}

void init(const CCmdArgs &args)
{
	if (!args.haveLongArg("desktop"))
		return;
#ifdef NL_OS_WINDOWS
	// an empty one is asked for as well: it is no desktop, and the client stops for it once its log is there
	// (showWindow)
	std::vector<std::string> values = args.getLongArg("desktop");
	s_IsRequested = true;
	s_Requested = values.empty() ? std::string() : values.front();
	keepCrashesOffDesktopShown();
#else
	ExitClientError("%s", "The client cannot show on another virtual desktop (--desktop): virtual desktops are handled on Windows only");
#endif
}

bool isRequested()
{
	return s_IsRequested;
}

void setRequested(const std::string &desktop)
{
	s_IsRequested = true;
	s_Requested = desktop;
	keepCrashesOffDesktopShown();
}

bool mayOpen(const std::string &what)
{
	if (!s_IsRequested)
		return true;
	nlwarning("The client does not start %s, which would show on the desktop shown: the client is on another virtual desktop (--desktop)", what.c_str());
	return false;
}

const std::string &getRequested()
{
	return s_Requested;
}

void showWindow(NL3D::UDriver *driver)
{
#ifdef NL_OS_WINDOWS
	HWND window = (HWND)driver->getDisplay();
	// the client was asked to stay off the desktop shown: without the desktop it stops, before its window is shown
	if (s_Requested.empty())
	{
		ExitClientError("%s",
		    "The client cannot show its window on another virtual desktop: --desktop names none. Give its number (1 for the first one) or the name it was given in the task "
		    "view: --desktop=<desktop>");
		// ExitClientError() calls exit(), the code after it is never reached
		return;
	}
	uint number = 0;
	std::string error;
	// a desktop that is not there, or no virtual desktops at all: nothing to wait for. Another refusal may only be that
	// the window is not shown yet.
	if (!CSystemUtils::moveWindowToDesktop(window, s_Requested, number, error) && number == 0)
	{
		ExitClientError("The client cannot show its window on the virtual desktop it was asked for (--desktop): %s", error.c_str());
		// ExitClientError() calls exit(), the code after it is never reached
		return;
	}
	// off the screen, beyond the monitors on the right, until the system has moved it
	RECT rect;
	GetWindowRect(window, &rect);
	SetWindowPos(window, nullptr, GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN) + 100, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	driver->showWindow(true);
	// moved again now that it is shown, should the system not have taken the move of a window not shown yet; a window
	// there already is left as it is
	bool there = CSystemUtils::moveWindowToDesktop(window, s_Requested, number, error) && CSystemUtils::waitForWindowDesktop(window, number, 3000, error);
	SetWindowPos(window, nullptr, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	if (there)
		nlinfo("The client shows its window on the virtual desktop %u ('%s', --desktop)", number, s_Requested.c_str());
	else
		nlwarning("The system did not move the window of the client to the virtual desktop %u ('%s', --desktop): %s. It shows on the desktop shown, without taking the focus",
		    number, s_Requested.c_str(), error.c_str());
#else
	driver->showWindow();
#endif
}

}
