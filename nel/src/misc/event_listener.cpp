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

#include "stdmisc.h"

#include "nel/misc/event_listener.h"
#include "nel/misc/event_server.h"
#include "nel/misc/events.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


/*
 * Constructor
 */
IEventListener::IEventListener() : _Hook(NULL)
{
}

// ***************************************************************************
// ***************************************************************************
// CEventListenerAsync
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CEventListenerAsync::CEventListenerAsync()
{
	_KeyArray.resize (KeyCount);
	_KeyDownArray.resize (KeyCount);
	_KeyReleaseArray.resize (KeyCount);
	reset ();
}
// ***************************************************************************
void CEventListenerAsync::addToServer (CEventServer& server)
{
	server.addListener (EventKeyDownId, this);
	server.addListener (EventSetFocusId, this);
	server.addListener (EventKeyUpId, this);
}
// ***************************************************************************
void CEventListenerAsync::removeFromServer (CEventServer& server)
{
	server.removeListener (EventKeyUpId, this);
	server.removeListener (EventKeyDownId, this);
	server.removeListener (EventSetFocusId, this);
}
// ***************************************************************************
bool CEventListenerAsync::isKeyDown (TKey key) const
{
	return _KeyArray.get(key);
}

// ***************************************************************************
bool CEventListenerAsync::isKeyPushed (TKey key, bool release)
{
	bool	ret= _KeyDownArray.get(key) && !(_KeyReleaseArray.get(key));
	if(ret && release)
	{
		_KeyReleaseArray.set(key, true);
	}
	return ret;
}

// ***************************************************************************
void CEventListenerAsync::operator ()(const CEvent& event)
{
	// Key down ?
	if (event==EventKeyDownId)
	{
		CEventKeyDown *pEvent=(CEventKeyDown*)&event;
		_KeyArray.set (pEvent->Key);
		_KeyDownArray.set (pEvent->Key);
		switch(pEvent->Key)
		{
			case KeyRCONTROL:
			case KeyLCONTROL:
				_KeyArray.set (KeyCONTROL);
				_KeyDownArray.set (KeyCONTROL);
			break;
			case KeyRSHIFT:
			case KeyLSHIFT:
				_KeyArray.set (KeySHIFT);
				_KeyDownArray.set (KeySHIFT);
			break;
			case KeyRMENU:
			case KeyLMENU:
				_KeyArray.set (KeyMENU);
				_KeyDownArray.set (KeyMENU);
			break;
			default:
			break;
		}
	}
	// Key up ?
	if (event==EventKeyUpId)
	{
		CEventKeyUp *pEvent=(CEventKeyUp*)&event;

		_KeyArray.clear (pEvent->Key);

		switch(pEvent->Key)
		{
			case KeyRCONTROL:
			case KeyLCONTROL:
				// Do not "raise up" the key, until someone has get the state of this key.
				if (!_KeyArray[KeyLCONTROL] && !_KeyArray[KeyRCONTROL])
				{
					_KeyArray.clear(KeyCONTROL);

					if(_KeyReleaseArray.get(KeyCONTROL))
					{
						_KeyDownArray.clear (KeyCONTROL);
						_KeyReleaseArray.clear (KeyCONTROL);
					}
				}
			break;
			case KeyRSHIFT:
			case KeyLSHIFT:
				if (!_KeyArray[KeyLSHIFT] && !_KeyArray[KeyRSHIFT])
				{
					_KeyArray.clear(KeySHIFT);

					if(_KeyReleaseArray.get(KeySHIFT))
					{
						_KeyDownArray.clear (KeySHIFT);
						_KeyReleaseArray.clear (KeySHIFT);
					}
				}
			break;
			case KeyRMENU:
			case KeyLMENU:
				if (!_KeyArray[KeyLMENU] && !_KeyArray[KeyRMENU])
				{
					_KeyArray.clear(KeyMENU);

					if(_KeyReleaseArray.get(KeyMENU))
					{
						_KeyDownArray.clear (KeyMENU);
						_KeyReleaseArray.clear (KeyMENU);
					}
				}
			break;
			default: break;
		}


		// Do not "raise up" the key, until someone has get the state of this key.
		if(_KeyReleaseArray.get(pEvent->Key))
		{
			_KeyDownArray.clear (pEvent->Key);
			_KeyReleaseArray.clear (pEvent->Key);
		}

	}
	// Activate false ?
	if (event==EventSetFocusId)
	{
		CEventSetFocus *pEvent=(CEventSetFocus *)&event;
		if (!pEvent->Get)
		{
			// Disactive all keys
			_KeyArray.clearAll ();
			_KeyDownArray.clearAll ();
			_KeyReleaseArray.clearAll ();
		}
	}
}


// ***************************************************************************
void CEventListenerAsync::reset ()
{
	_KeyArray.clearAll ();
	_KeyDownArray.clearAll ();
	_KeyReleaseArray.clearAll ();
}


} // NLMISC
