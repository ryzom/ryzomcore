// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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

#ifndef NL_EMSCRIPTEN_EVENT_EMITTER_H
#define NL_EMSCRIPTEN_EVENT_EMITTER_H

#include "nel/misc/types_nl.h"

#ifdef __EMSCRIPTEN__

#include "nel/misc/event_emitter.h"
#include "nel/misc/events.h"

#include <emscripten/html5.h>

#include <string>
#include <vector>

namespace NLMISC {

/**
 * CEventEmitter Emscripten implementation.
 * Registers HTML5 callbacks on the canvas (mouse, touch) and window
 * (keyboard, focus), buffers the resulting NeL events and posts them
 * to the event server on submitEvents().
 * Touch input is mapped to left mouse button events (first touch only).
 */
class CEmscriptenEventEmitter : public IEventEmitter
{
public:
	CEmscriptenEventEmitter();
	virtual ~CEmscriptenEventEmitter();

	/// Register HTML5 callbacks on the given canvas selector (e.g. "#canvas").
	void init(const char *canvasSelector);

	/// Unregister the HTML5 callbacks and drop pending events.
	void release();

	virtual void submitEvents(CEventServer &server, bool allWindows);

private:
	static EM_BOOL mouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData);
	static EM_BOOL wheelCallback(int eventType, const EmscriptenWheelEvent *e, void *userData);
	static EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData);
	static EM_BOOL touchCallback(int eventType, const EmscriptenTouchEvent *e, void *userData);
	static EM_BOOL focusCallback(int eventType, const EmscriptenFocusEvent *e, void *userData);

	void postEvent(CEvent *e);
	bool normalizePos(long targetX, long targetY, float &fX, float &fY);

	std::string _CanvasSelector;
	std::vector<CEvent *> _Events; // buffered events, flushed by submitEvents
	uint _MouseButtons; // currently held buttons (left/middle/right mask)
	sint32 _TouchId; // identifier of the touch acting as the mouse, -1 if none
	bool _Registered;
};

} // NLMISC

#endif // __EMSCRIPTEN__

#endif // NL_EMSCRIPTEN_EVENT_EMITTER_H

/* End of emscripten_event_emitter.h */
