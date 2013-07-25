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

#ifndef NL_UNIX_EVENT_EMITTER_H
#define NL_UNIX_EVENT_EMITTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/events.h"
#include "nel/misc/game_device_events.h"

#include "nel/3d/driver.h"

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#include <X11/Xlib.h>

namespace NLMISC {


/**
 * CEventEmitter UNIX implementation
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CUnixEventEmitter : public IEventEmitter
{
public:

	/// Constructor
	CUnixEventEmitter();
	virtual ~CUnixEventEmitter();

	/**
	 * initialize CUnixEventEmitter
	 */
	void init(Display *dpy, Window win, NL3D::IDriver *driver = NULL);

	/**
	 * sends all events to server
	 * (should call CEventServer method postEvent() )
	 */
	virtual void submitEvents(CEventServer & server, bool allWindows);

	/**
	 * enable or disable mouse raw mode
	 */
	virtual void emulateMouseRawMode(bool emulate);

	/**
	 * process input-related events (mouse and keyboard)
	 */
	bool processMessage(XEvent &event, CEventServer *server = NULL);

	/**
	 * Copy a string to system clipboard.
	 */
        virtual bool copyTextToClipboard(const ucstring &text);

	/*
	 * Paste a string from system clipboard.
	 */
	virtual bool pasteTextFromClipboard(ucstring &text);

	void createIM();
	void closeIM();

private:

	// Private internal server message
	class CUnixEventServer : CEventServer
	{
		friend class CUnixEventEmitter;
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

	Display*				_dpy;
	Window					_win;
	std::map<TKey, bool>	_PressedKeys;
	XIM						_im;
	XIC						_ic;
	bool					_emulateRawMode;
	NL3D::IDriver*			_driver;
	CUnixEventServer		_InternalServer;
	ucstring				_CopiedString;
	bool					_SelectionOwned;
};


} // NLMISC

#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#endif // NL_UNIX_EVENT_EMITTER_H

/* End of unix_event_emitter.h */
