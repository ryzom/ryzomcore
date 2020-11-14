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

#ifndef NL_WIN_EVENT_EMITTER_H
#define NL_WIN_EVENT_EMITTER_H

#include "types_nl.h"
#include "event_emitter.h"
#include "events.h"


#ifdef NL_OS_WINDOWS

namespace NLMISC {

/**
 * CWinEventEmitter
 * CEventEmitter Windows implementation
 */
class CWinEventEmitter : public IEventEmitter
{
public:
	CWinEventEmitter () : _MouseEventsEnabled(true), _KeyboardEventsEnabled(true), _IMEEventsEnabled(true)
	{
		_HWnd=NULL;
		resetButtonFlagState ();
	}
	void setHWnd (HWND hWnd)
	{
		_HWnd=hWnd;
		resetButtonFlagState ();
	}

	/**
	 * sends all events to server
	 * (should call CEventServer method postEvent() )
	 * \param server
	 */
	virtual void submitEvents(CEventServer & server, bool allWindows);

	/// Build the flags of the current buttons state
	TMouseButton buildFlags() const;

	// Reset button flag state
	void resetButtonFlagState ();

	// enable / disable mouse events to be processed. The default is enabled.
	void enableMouseEvents(bool enabled = true) { _MouseEventsEnabled = enabled; }

	// enable / disable keyboard events to be processed. The default is enabled.
	void enableKeyboardEvents(bool enabled = true) { _KeyboardEventsEnabled = enabled; }

	// enable / disable other events to be processed. The default is enabled.
	void enableIMEEvents(bool enabled = true) { _IMEEventsEnabled = enabled; }

	// Test whether mouse events are enabled.
	bool areMouseEventsEnabled() const { return _MouseEventsEnabled; }

	// Test whether keyboard events are enabled.
	bool areKeyboardEventsEnabled() const { return _KeyboardEventsEnabled; }
private:


	// Private internal server message
	class CWinEventServer : CEventServer
	{
		friend class CWinEventEmitter;
	public:
		void setServer (CEventServer *server)
		{
			_Server=server;
		}
	private:
		virtual bool pumpEvent(CEvent* event)
		{
			CEventServer::pumpEvent(event);
			_Server->postEvent (event);
			return false;
		}
	private:
		CEventServer *_Server;
	};

public:
	/** Process a win32 message.
	  * Return true if the message must be trapped, false if DefWindowProc must be called afterwards
	  */
	bool processMessage (HWND hWnd, uint32 msg, WPARAM wParam, LPARAM lParam, CEventServer *server=NULL);

private:
	CWinEventServer		_InternalServer;
	HWND				_HWnd;
public:
	// private: may need to be in sync with direct input flags however...
	bool				_CtrlButton;
	bool				_ShiftButton;
	bool				_AltButton;
	bool				_MouseButtons[3];
	bool				_MouseEventsEnabled;
	bool				_KeyboardEventsEnabled;
	bool				_IMEEventsEnabled;
private:
	NLMISC::TMouseButton		getButtons() const;
};

} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_EVENT_EMITTER_H

/* End of win_event_emitter.h */
