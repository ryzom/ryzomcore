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
};

} // NLMISC

#endif // NL_SYSTEM_UTILS_H
