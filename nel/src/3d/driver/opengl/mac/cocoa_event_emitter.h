// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <string.h>
#include "nel/misc/event_emitter.h"
#include "nel/misc/event_server.h"
#include "nel/misc/events.h"
#include "nel/3d/driver.h"
#import  "cocoa_opengl_view.h"

#include <Carbon/Carbon.h>
#import  <Cocoa/Cocoa.h>

namespace NLMISC
{

class CCocoaEventEmitter : public IEventEmitter
{
	bool             _eventLoop;
	NL3D::IDriver*   _driver;
	CocoaOpenGLView* _glView;

	// TODO like internal server in unix event emitter... review!
	CEventServer*    _server;

	// buttons pressed in the view and not released yet
	uint             m_ButtonsDown;

	// while the mouse is relative, its moves add their deltas to NeL's position and the cursor stays where it is
	bool             m_MouseRelative;
	// NeL's position of the mouse in the view
	float            m_MouseX;
	float            m_MouseY;

public:
	CCocoaEventEmitter() :
		_driver(NULL),
		_glView(nil),
		_server(NULL),
		m_ButtonsDown(0),
		m_MouseRelative(false),
		m_MouseX(0.5f),
		m_MouseY(0.5f) { }

	void         init(NL3D::IDriver*, CocoaOpenGLView*, bool eventLoop);
	bool         processMessage(NSEvent* event, CEventServer* server = NULL);
	virtual void submitEvents(CEventServer& server, bool allWindows);
	bool         handleQuitRequest();

	/// the window got or lost the keyboard focus
	void         postFocusEvent(bool focused);

	/// see m_MouseRelative, the driver decides when the mouse is relative
	void         setMouseRelative(bool relative) { m_MouseRelative = relative; }
	bool         isMouseRelative() const { return m_MouseRelative; }

	/// NeL's position of the mouse, from the last mouse event or the last position the driver set
	void         setMousePos(float x, float y) { m_MouseX = x; m_MouseY = y; }
	void         getMousePos(float &x, float &y) const { x = m_MouseX; y = m_MouseY; }

	virtual bool copyTextToClipboard(const std::string &text);
	virtual bool pasteTextFromClipboard(std::string &text);
};

}

#endif
