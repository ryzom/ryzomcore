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

#ifndef CL_WINDOW_DESKTOP_H
#define CL_WINDOW_DESKTOP_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NLMISC {
class CCmdArgs;
}

namespace NL3D {
class UDriver;
}

/// The virtual desktop the client shows its window on (--desktop, Windows), without activating it, as activating a
/// window on a desktop that is not shown switches the screen to that desktop. So several clients can run on a second
/// desktop while the first one stays free. There is no splash screen then.
namespace WindowDesktop {

void addArgs(NLMISC::CCmdArgs &args);

/// Reads the option
void init(const NLMISC::CCmdArgs &args);

/// Whether the option asks for a desktop: the window of the client is then never activated, and stays in a window
bool isRequested();

/// The desktop the option asks for, its number or its name
const std::string &getRequested();

/// A desktop asked for while the client runs (a control tool moved the window there): as with the option, the window
/// is never activated from then on, and stays out of a full screen mode
void setRequested(const std::string &desktop);

/// Shows the window of the client on the desktop of the option, without activating it. The system moves a window
/// there a moment after it is shown, and it is on the desktop shown until then: it is shown off the screen, and put
/// in its place once it is there. When the desktop cannot be had, it shows on the desktop shown, still without
/// taking the focus, and the log says why.
void showWindow(NL3D::UDriver *driver);

}

#endif // CL_WINDOW_DESKTOP_H
