// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef NL_UNIX_EVENT_EMITTER_H
#define NL_UNIX_EVENT_EMITTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/events.h"

#include "nel/3d/driver.h"

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC) && !defined(__EMSCRIPTEN__)

#include <vector>

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
	 * \param ownWindow false for a window of the application, which keeps the window manager protocols it has
	 */
	void init(Display *dpy, Window win, NL3D::IDriver *driver = nullptr, bool ownWindow = true);

	/**
	 * sends all events to server
	 * (should call CEventServer method postEvent())
	 */
	virtual void submitEvents(CEventServer & server, bool allWindows);

	/**
	 * process input-related events (mouse and keyboard)
	 */
	bool processMessage(XEvent &event, CEventServer *server = nullptr);

	/**
	 * Copy a string to system clipboard.
	 */
        virtual bool copyTextToClipboard(const std::string &text);

	/*
	 * Paste a string from system clipboard.
	 */
	virtual bool pasteTextFromClipboard(std::string &text);

	void createIM();
	void closeIM();

	/// The input method stopped and Xlib frees it: forget it, the next events open another one
	void lostIM();

private:

	/// The key of a key event, from its keycode, the keysym looked up for it and its modifier state
	TKey getKeyFromKeyEvent(uint keycode, KeySym keysym, uint state);

	/// The character key of each keycode in a keyboard group, read from the layout when first asked for
	const std::vector<TKey> &getKeycodeKeys(uint group);

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
	std::map<uint, TKey>	m_PressedKeycodes; // key reported for each held keycode
	std::vector<TKey>		m_KeycodeKeys[4]; // character key of each keycode, per keyboard group; empty until read
	XIM						_im;
	XIC						_ic;
	bool					m_LostIM; // the input method stopped, another one is to be opened
	bool					m_Focus; // the window has the keyboard focus
	uint					m_FilteredKeycode; // last key press an input method took, 0 when the last one was not
	Time					m_FilteredTime; // and its time
	NL3D::IDriver*			_driver;
	CUnixEventServer		_InternalServer;
	std::string				_CopiedString;
	bool					_SelectionOwned;
};


} // NLMISC

#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC) && !defined(__EMSCRIPTEN__)

#endif // NL_UNIX_EVENT_EMITTER_H

/* End of unix_event_emitter.h */
