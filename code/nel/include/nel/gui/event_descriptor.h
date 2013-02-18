// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef RZ_EVENT_DESCRIPTOR_H
#define RZ_EVENT_DESCRIPTOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/events.h"

namespace NLGUI
{

// ----------------------------------------------------------------------------
class CEventDescriptor
{
public:
	enum EEventType
	{
		key = 0,
		mouse = 1,
		system = 2
	};

	EEventType getType() const
	{
		return _EventType;
	}

protected:
	//type of the event
	EEventType _EventType;
};


/** encode key events.
  */
class CEventDescriptorKey : public CEventDescriptor
{
public:
	enum EKeyEventType
	{
		keydown = 0, // a key has been press down. The key value is stored as a TKey
		keyup,   // a key has been released. The key value is stored as a TKey
		keychar,  // a key has been stroke. The key is a ucchar
		keystring, // a string has been sent. The string is a ucstring
		unknown, // uninitialized event
	};
	CEventDescriptorKey() : _KeyEvent(unknown)
	{
		_EventType = key;
	}
	// ctrcuct from a CEventKey obj
	CEventDescriptorKey(const NLMISC::CEventKey &ev) : _KeyEvent(unknown)
	{
		_EventType = key;
		init(ev);
	}
	// get the type of the key event
	EKeyEventType getKeyEventType() const { return _KeyEvent; }
	// return the key that has been pressed. The key event type MUST be 'keydown' or 'keyup', else => assert
	NLMISC::TKey  getKey() const
	{
		nlassert(_KeyEvent == keydown || _KeyEvent == keyup);
		return _Key;
	}
	// return the char that has been pressed. The key event type MUST be 'keychar', else => assert
	ucchar getChar() const
	{
		nlassert(_KeyEvent == keychar);
		return _Char;
	}
	// return the string that has been sent. The key event type MUST be 'keystring', else => assert
	ucstring getString() const
	{
		nlassert(_KeyEvent == keystring);
		return _String;
	}
	bool getKeyCtrl() const // is CTRL pressed ?
	{
		return _CtrlState;
	}
	bool getKeyShift() const // is SHIFT (right or left) pressed ?
	{
		return _ShiftState;
	}
	bool getKeyAlt() const // is ALT (right or left) pressed ?
	{
		return _AltState;
	}
	// init from a CEventKey obj
	void init(const NLMISC::CEventKey &ev);

private:
	EKeyEventType   _KeyEvent;
	bool		    _CtrlState;
	bool			_ShiftState;
	bool			_AltState;
	union
	{
		NLMISC::TKey	_Key;
		ucchar			_Char;
	};
	ucstring		_String;
};

// ----------------------------------------------------------------------------
class CEventDescriptorMouse : public CEventDescriptor
{
public:
	enum EEventTypeExtended
	{
		mouseleftdown=0,
		mouseleftup=1,
		mouserightdown=2,
		mouserightup=3,
		mousewheel=4,		// Complementary info stored in wheel
		mousemove=5,		// Complementary info stored in x and y
		mouseleftdblclk= 6,
		mouserightdblclk= 7,
	};

	CEventDescriptorMouse()
	{
		_EventType = mouse;
		_X = _Y = _Wheel = 0;
	}

	CEventDescriptorMouse (sint32 x, sint32 y)
	{
		_X = x;
		_Y = y;
	}

	sint32 getX() const
	{
		return _X;
	}

	sint32 getY() const
	{
		return _Y;
	}

	sint32 getWheel() const
	{
		return _Wheel;
	}

	sint32 getEventTypeExtended() const
	{
		return _EventTypeExtended;
	}

	void setX (sint32 x)
	{
		_X = x;
	}

	void setY (sint32 y)
	{
		_Y = y;
	}

	void setWheel (sint32 w)
	{
		_Wheel = w;
	}

	void setEventTypeExtended (sint32 e)
	{
		_EventTypeExtended = e;
	}

protected:
	sint32 _X;
	sint32 _Y;
	sint32 _Wheel;
	sint32 _EventTypeExtended;
};


// ----------------------------------------------------------------------------
class CEventDescriptorSystem : public CEventDescriptor
{
public:
	enum EEventTypeExtended
	{
		activecalledonparent= 0,
		clocktick,
		setfocus,
		unknown
	};

	sint32 getEventTypeExtended() const
	{
		return _EventTypeExtended;
	}
	void setEventTypeExtended (sint32 e)
	{
		_EventTypeExtended = e;
	}
	CEventDescriptorSystem() : _EventTypeExtended(unknown)
	{
		_EventType = system;
	}
protected:
	sint32 _EventTypeExtended;
};

// ----------------------------------------------------------------------------
class CEventDescriptorActiveCalledOnParent : public CEventDescriptorSystem
{
public:
	bool getActive() const { return _Active; }
	void setActive(bool active) { _Active = active; }
	CEventDescriptorActiveCalledOnParent(bool active = false) : _Active(active)
	{
		setEventTypeExtended(activecalledonparent);
	}
protected:
	bool _Active;
};


// ----------------------------------------------------------------------------
class CEventDescriptorSetFocus : public CEventDescriptorSystem
{
public:
	bool hasFocus() const { return _HasFocus; }
	CEventDescriptorSetFocus(bool hasFocus = false) : _HasFocus(hasFocus)
	{
		setEventTypeExtended(setfocus);
	}
protected:
	bool _HasFocus;
};

}

#endif // RZ_EVENT_DESCRIPTOR_H

/* End of event_descriptor.h */
