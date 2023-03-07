// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef NL_COCOA_EVENT_EMITTER_H
#define NL_COCOA_EVENT_EMITTER_H

#include "nel/misc/event_emitter.h"
#include "nel/misc/event_server.h"
#include "nel/misc/events.h"
#include "nel/misc/game_device_events.h"
#include "nel/3d/driver.h"
#import  "cocoa_opengl_view.h"

#include <Carbon/Carbon.h>
#import  <Cocoa/Cocoa.h>

namespace NLMISC 
{

class CCocoaEventEmitter : public IEventEmitter
{
	bool             _emulateRawMode;
	bool             _setToEmulateRawMode;
	bool             _eventLoop;
	NL3D::IDriver*   _driver;
	CocoaOpenGLView* _glView;

	// TODO like internal server in unix event emitter... review!
	CEventServer*    _server;

public:
	CCocoaEventEmitter() : 
		_emulateRawMode(false), 
		_setToEmulateRawMode(false),
		_driver(NULL),
		_glView(nil),
		_server(NULL) { }

	void         init(NL3D::IDriver*, CocoaOpenGLView*, bool eventLoop);
	bool         processMessage(NSEvent* event, CEventServer* server = NULL);
	virtual void submitEvents(CEventServer& server, bool allWindows);
	virtual void emulateMouseRawMode(bool enable);
	bool         handleQuitRequest();

	virtual bool copyTextToClipboard(const ucstring &text);
	virtual bool pasteTextFromClipboard(ucstring &text);
};

}

#endif
