/*
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
*/

#ifndef NL_DRIVER_OPENGL_MAC_COCOA_ADAPTER_H
#define NL_DRIVER_OPENGL_MAC_COCOA_ADAPTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_server.h"
#include "nel/3d/driver.h"

#include "cocoa_event_emitter.h"

/*
 * this cocoa adapter is a helper to call functions executing obj-c code
 * from driver_opengl.cpp
 * 
 * please see this as a temporary solution... there is some stuff concerning 
 * driver refactoring going on anyway as far as i know
 *
 * this can as well be seen as a preparation to pull platform specific code
 * out of driver_opengl.cpp ;)
 */

namespace NL3D { namespace MAC {

/// mac specific stuff todo while calling CDriverGL::CDriverGL()
void ctor();

/// mac specific stuff todo while calling CDriverGL::~CDriverGL()
void dtor();

/// mac specific stuff todo while calling CDriverGL::init()
bool init(uint windowIcon = 0, emptyProc exitFunc = 0);

/// mac specific stuff todo while calling CDriverGL::setDisplay()
bool setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable);

/// mac specific stuff todo while calling CDriverGL::swapBuffers()
void swapBuffers();

/// mac specific stuff todo while calling CCocoaEventEmitter::submitEvents()
void submitEvents(NLMISC::CEventServer& server, 
	bool allWindows, NLMISC::CCocoaEventEmitter* eventEmitter);

}}

#endif
