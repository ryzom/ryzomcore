// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SYSTEM_UTILS_H
#define NL_SYSTEM_UTILS_H

#include "types_nl.h"
#include "ucstring.h"

#include <string>
#include <vector>

namespace NLMISC
{

/*
 * Operating system miscellaneous functions (all methods and variables should be static)
 * \author Kervala
 * \date 2010
 */
class CSystemUtils
{
	static nlWindow s_window;
public:

	/// Initialize data which needs it before using them.
	static bool init();

	/// Uninitialize data when they won't be used anymore.
	static bool uninit();

	/// Set the window which will be used by some functions.
	static void setWindow(nlWindow window);

	/// Create/update a progress bar with an appearance depending on system.
	static bool updateProgressBar(uint value, uint total);

	/// Copy a string to system clipboard.
	static bool copyTextToClipboard(const std::string &text);

	/// Paste a string from system clipboard.
	static bool pasteTextFromClipboard(std::string &text);

	/// Check if system supports unicode.
	static bool supportUnicode();

	/// Check if keyboard layout is AZERTY.
	static bool isAzertyKeyboard();

	/// Check if screensaver is enabled.
	static bool isScreensaverEnabled();

	/// Enable or disable screeensaver.
	static bool enableScreensaver(bool screensaver);

	/// Get the ROOT registry key used by getRegKey and setRegKey.
	static std::string getRootKey();

	/// Set the ROOT registry key used by getRegKey and setRegKey.
	static void setRootKey(const std::string &root);

	/// Read a value from registry.
	static std::string getRegKey(const std::string &entry);

	/// Write a value to registry.
	static bool setRegKey(const std::string &valueName, const std::string &value);

	/// Get desktop current color depth without using UDriver.
	static uint getCurrentColorDepth();

	/// Detect whether the current process is a windowed application. Return true if definitely yes, false if unknown
	static bool detectWindowedApplication();

	static sint getTotalVideoMemory();

	/// A function that is called when the session of the user ends, see setEndSessionCallback
	typedef void (*TEndSessionCallback)();

	/** Set the function that is called when the session of the user ends: the system shuts down or restarts, or
	 *  the user logs off. nullptr removes it. The system does not wait for the application then, it ends the
	 *  process after a few seconds: the function asks nothing, saves what must be saved, and returns or ends the
	 *  process itself. It is called in the thread that pumps the events of the window.
	 *  - Windows: at WM_ENDSESSION of a window whose messages a CWinEventEmitter processes. The system may end
	 *    the process as soon as the function returns. The message is sent, not posted: it comes whenever that
	 *    thread takes messages, in the pump and also in a modal dialog or in a call that waits for a window.
	 *  - Unix with X11: after SIGTERM or SIGHUP (unless the signal is ignored when the function is set), at the
	 *    next pollEndSession, which the event emitters of the drivers call when they are pumped. An application
	 *    that sets a function and has no such emitter calls pollEndSession itself. The process goes on when the
	 *    function returns. When no poll comes within five seconds of the signal (the application hangs, or does
	 *    not pump its events), the signal ends the process as it would without a function; the alarm of the
	 *    process (SIGALRM) is used for that while a function is set. A signal that still waits for a poll when
	 *    the function is removed is raised again, for the action it had before.
	 *  - Elsewhere the function is not called.
	 */
	static void setEndSessionCallback(TEndSessionCallback callback);

	/// The session of the user ends: call the function set with setEndSessionCallback, if any. For the code that
	/// receives the notice of the system.
	static void endSession();

	/// Call endSession when a signal that ends the session came since the last call, see setEndSessionCallback.
	static void pollEndSession();

	/// The virtual desktops of the session (Windows 10 and later), in the order of the task view: the name the user
	/// gave each one, empty for a desktop that has its default name ("Desktop 2"...), and the number of the one shown
	/// (0 when unknown). False with the error elsewhere.
	static bool getVirtualDesktops(std::vector<std::string> &names, uint &shown, std::string &error);

	/// Moves a window of this process to a virtual desktop of the session: desktop is its number (1 for the first one)
	/// or the name the user gave it, and number gets its number once the desktop is known, also when the move then
	/// fails (0 when there is no such desktop: the move cannot work later either). A window not shown yet goes there a moment after it
	/// is shown, and is on the desktop shown until then (see waitForWindowDesktop). Show it without activating it, as
	/// activating a window on a desktop that is not shown switches the screen to that desktop. A tool window has no
	/// desktop of its own: it needs WS_EX_APPWINDOW as well.
	static bool moveWindowToDesktop(nlWindow window, const std::string &desktop, uint &number, std::string &error);

	/// Waits until a window of this process, moved to virtual desktop number and shown, is there and, when that desktop
	/// is not the one shown, hidden from the screen by the system: timeout milliseconds at most. The messages of this
	/// thread are handled meanwhile, as the system may need the window to answer. Show the window off the screen and
	/// wait here before putting it in its place, and it never appears on the desktop shown.
	static bool waitForWindowDesktop(nlWindow window, uint number, uint timeout, std::string &error);

	/// The number of the virtual desktop a window of this process is on: 0 before the window was first shown, or
	/// for a window on every desktop. False with the error when the system does not tell, or lists no desktops.
	static bool getWindowDesktop(nlWindow window, uint &number, std::string &error);
};

} // NLMISC

#endif // NL_SYSTEM_UTILS_H
