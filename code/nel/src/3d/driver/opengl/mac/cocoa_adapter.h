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
 *
 * btw: we cannot simply use a c++ class here, because then NSWindow* and friends
 * would be members, but then we would need to add obj-c code here using an
 * include or a forward declaration. this again would break compiling cpp files
 * including this one (eg. driver_opengl.cpp)
 */

namespace NL3D { namespace MAC {

/// mac specific stuff while calling CDriverGL::CDriverGL()
void ctor();

/// mac specific stuff while calling CDriverGL::~CDriverGL()
void dtor();

/// mac specific stuff while calling CDriverGL::init()
bool init(uint windowIcon = 0, emptyProc exitFunc = 0);

/// mac specific stuff while calling CDriverGL::unInit()
bool unInit();

/// mac specific stuff while calling CDriverGL::createWindow()
nlWindow createWindow(const GfxMode& mode);

/// mac specific stuff while calling CDriverGL::destroyWindow()
bool destroyWindow(nlWindow wnd);

/// mac specific stuff while calling CDriverGL::setDisplay()
nlWindow setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable);

/// mac specific stuff while calling CDriverGL::setWindowStyle()
bool setWindowStyle(nlWindow wnd, bool fullscreen);

/// mac specific stuff while calling CDriverGL::getCurrentScreenMode()
void getCurrentScreenMode(nlWindow wnd, GfxMode& mode);

/// mac specific stuff while calling CDriverGL::getWindowSize()
void getWindowSize(nlWindow wnd, uint32 &width, uint32 &height);

/// mac specific stuff while calling CDriverGL::setWindowSize()
void setWindowSize(nlWindow wnd, uint32 width, uint32 height);

/// mac specific stuff while calling CDriverGL::getWindowPos()
void getWindowPos(nlWindow wnd, sint32 &x, sint32 &y);

/// mac specific stuff while calling CDriverGL::setWindowPos()
void setWindowPos(nlWindow wnd, sint32 x, sint32 y);

/// mac specific stuff while calling CDriverGL::setWindowTitle()
void setWindowTitle(nlWindow wnd, const ucstring& title);

/// mac specific stuff while calling CDriverGL::showWindow()
void showWindow(bool show);

/// mac specific stuff while calling CDriverGL::activate()
bool activate(nlWindow wnd);

/// mac specific stuff while calling CDriverGL::swapBuffers()
void swapBuffers(nlWindow wnd);

/// mac specific stuff while calling CDriverGL::setCapture()
void setCapture(bool capture);

/// mac specific stuff while calling CDriverGL::showCursor()
void showCursor(bool show);

/// mac specific stuff while calling CDriverGL::setMousePos()
void setMousePos(nlWindow wnd, float x, float y);

/// mac specific stuff while calling CCocoaEventEmitter::submitEvents()
void submitEvents(NLMISC::CEventServer& server, 
	bool allWindows, NLMISC::CCocoaEventEmitter* eventEmitter);

/// mac specific stuff while calling CCocoaEventEmitter::emulateMouseRawMode()
void emulateMouseRawMode(bool enable);

}}

#endif
