// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_EVENTS_H
#define NL_EVENTS_H

#include "types_nl.h"
#include "class_id.h"
#include "ucstring.h"
#include <map>
#include <list>

namespace NLMISC {

/*===================================================================*/

class IEventEmitter;

/**
 * CEvent. System event.
 * \date 2000
 */
class CEvent : public CClassId
{
public:
	/// Emitter of the event. Can be NULL if the event is posted directly to the CEventServer.
	IEventEmitter* Emitter;

	// duplicate the object
	virtual	CEvent		*clone() const =0;

	virtual ~CEvent() {}

protected:
	/** Constructor.
	  * \param emitter is the emitter of the event. Can be NULL if the event is posted directly to the CEventServer.
	  * \param classId is the classId of the event. Should be unique for each event.
	  */
	CEvent (IEventEmitter* emitter, const CClassId& classId) : CClassId (classId)
	{
		Emitter=emitter;
	}
};

// Key events
const CClassId EventKeyDownId (0x3c2643da, 0x43f802a1);
const CClassId EventKeyUpId (0x1e62e85, 0x68a35d46);
const CClassId EventCharId (0x552255fe, 0x75a2373f);
const CClassId EventStringId (0x49b5af8f, 0x7f52cd26);

// Window events
const CClassId EventActivateId (0x7da66b0a, 0x1ef74519);
const CClassId EventSetFocusId (0x17650fac, 0x19f85dde);
const CClassId EventDestroyWindowId (0x69be73fe, 0x4b07603b);
const CClassId EventCloseWindowId (0xb5cb1333, 0xd092e63a);

// Mouse events
const CClassId EventMouseMoveId (0x3dd12fdb, 0x472f548b);
const CClassId EventMouseDownId (0x35b7878, 0x5d4a0f86);
const CClassId EventMouseUpId (0xcce1f7e, 0x7ed344d7);
const CClassId EventMouseDblClkId (0x55a94cb3, 0x3e641517);
const CClassId EventMouseWheelId (0x73ac4321, 0x4c273150);

// Misc events
const CClassId EventDisplayChangeId(0x1751559, 0x25b52b3c);

// Input Mehod Editor (IME) events
const CClassId EventIME (0x261f1ede, 0x1b0a6c3a);


enum TKey
{
	KeyNOKEY          =0x00,
	Key0              ='0',
	Key1              ='1',
	Key2              ='2',
	Key3              ='3',
	Key4              ='4',
	Key5              ='5',
	Key6              ='6',
	Key7              ='7',
	Key8              ='8',
	Key9              ='9',
	KeyA              ='A',
	KeyB              ='B',
	KeyC              ='C',
	KeyD              ='D',
	KeyE              ='E',
	KeyF              ='F',
	KeyG              ='G',
	KeyH              ='H',
	KeyI              ='I',
	KeyJ              ='J',
	KeyK              ='K',
	KeyL              ='L',
	KeyM              ='M',
	KeyN              ='N',
	KeyO              ='O',
	KeyP              ='P',
	KeyQ              ='Q',
	KeyR              ='R',
	KeyS              ='S',
	KeyT              ='T',
	KeyU              ='U',
	KeyV              ='V',
	KeyW              ='W',
	KeyX              ='X',
	KeyY              ='Y',
	KeyZ              ='Z',
	KeyLBUTTON        =0x01,
	KeyRBUTTON        =0x02,
	KeyCANCEL         =0x03,
	KeyMBUTTON        =0x04,
	KeyBACK           =0x08,
	KeyTAB            =0x09,
	KeyCLEAR          =0x0C,
	KeyRETURN         =0x0D,
	KeySHIFT          =0x10,
	KeyCONTROL        =0x11,
	KeyMENU           =0x12,
	KeyPAUSE          =0x13,
	KeyCAPITAL        =0x14,
	KeyKANA           =0x15,
	KeyHANGEUL        =0x15,
	KeyHANGUL         =0x15,
	KeyJUNJA          =0x17,
	KeyFINAL          =0x18,
	KeyHANJA          =0x19,
	KeyKANJI          =0x19,
	KeyESCAPE         =0x1B,
	KeyCONVERT        =0x1C,
	KeyNONCONVERT     =0x1D,
	KeyACCEPT         =0x1E,
	KeyMODECHANGE     =0x1F,
	KeySPACE          =0x20,
	KeyPRIOR          =0x21,
	KeyNEXT           =0x22,
	KeyEND            =0x23,
	KeyHOME           =0x24,
	KeyLEFT           =0x25,
	KeyUP             =0x26,
	KeyRIGHT          =0x27,
	KeyDOWN           =0x28,
	KeySELECT         =0x29,
	KeyPRINT          =0x2A,
	KeyEXECUTE        =0x2B,
	KeySNAPSHOT       =0x2C,
	KeyINSERT         =0x2D,
	KeyDELETE         =0x2E,
	KeyHELP           =0x2F,
	KeyLWIN           =0x5B,
	KeyRWIN           =0x5C,
	KeyAPPS           =0x5D,
	KeyNUMPAD0        =0x60,
	KeyNUMPAD1        =0x61,
	KeyNUMPAD2        =0x62,
	KeyNUMPAD3        =0x63,
	KeyNUMPAD4        =0x64,
	KeyNUMPAD5        =0x65,
	KeyNUMPAD6        =0x66,
	KeyNUMPAD7        =0x67,
	KeyNUMPAD8        =0x68,
	KeyNUMPAD9        =0x69,
	KeyMULTIPLY       =0x6A,
	KeyADD            =0x6B,
	KeySEPARATOR      =0x6C,
	KeySUBTRACT       =0x6D,
	KeyDECIMAL        =0x6E,
	KeyDIVIDE         =0x6F,
	KeyF1             =0x70,
	KeyF2             =0x71,
	KeyF3             =0x72,
	KeyF4             =0x73,
	KeyF5             =0x74,
	KeyF6             =0x75,
	KeyF7             =0x76,
	KeyF8             =0x77,
	KeyF9             =0x78,
	KeyF10            =0x79,
	KeyF11            =0x7A,
	KeyF12            =0x7B,
	KeyF13            =0x7C,
	KeyF14            =0x7D,
	KeyF15            =0x7E,
	KeyF16            =0x7F,
	KeyF17            =0x80,
	KeyF18            =0x81,
	KeyF19            =0x82,
	KeyF20            =0x83,
	KeyF21            =0x84,
	KeyF22            =0x85,
	KeyF23            =0x86,
	KeyF24            =0x87,
	KeyNUMLOCK        =0x90,
	KeySCROLL         =0x91,
	KeyLSHIFT         =0xA0,
	KeyRSHIFT         =0xA1,
	KeyLCONTROL       =0xA2,
	KeyRCONTROL       =0xA3,
	KeyLMENU          =0xA4,
	KeyRMENU          =0xA5,
	KeyMUTE           =0xAD,
	KeyPLAYPAUSE      =0xB3,
	KeyVOLUMEDOWN     =0xB4,
	KeyVOLUMEUP       =0xB5,
	KeyCALC           =0xB7,
	KeySEMICOLON      =0xBA,
	KeyEQUALS         =0xBB,
	KeyCOMMA          =0xBC,
	KeyDASH           =0xBD,
	KeyPERIOD         =0xBE,
	KeySLASH          =0xBF,
	KeyTILDE          =0xC0,
	KeyLBRACKET       =0xDB,
	KeyBACKSLASH      =0xDC,
	KeyRBRACKET       =0xDD,
	KeyAPOSTROPHE     =0xDE,
	KeyPARAGRAPH      =0xDF,
	KeyOEM_102        =0xE2,
	KeyPROCESSKEY     =0xE5,
	KeyATTN           =0xF6,
	KeyCRSEL          =0xF7,
	KeyEXSEL          =0xF8,
	KeyEREOF          =0xF9,
	KeyPLAY           =0xFA,
	KeyZOOM           =0xFB,
	KeyNONAME         =0xFC,
	KeyPA1            =0xFD,
	KeyOEM_CLEAR      =0xFE,
	KeyCount          =0xFF
};

enum TMouseButton
{
	noButton      =0x0,
	leftButton    =0x1,
	middleButton  =0x2,
	rightButton   =0x4,
	ctrlButton    =0x8,
	shiftButton   =0x10,
	altButton     =0x20
};

enum TKeyButton
{
	noKeyButton     =0x0,
	ctrlKeyButton   =0x8,
	shiftKeyButton  =0x10,
	altKeyButton    =0x20
};

/**
 * CEventKey
 */
class CEventKey : public CEvent
{
public:
	CEventKey (TKeyButton button, IEventEmitter* emitter, const CClassId& classId) : CEvent (emitter, classId)
	{
		Button=button;
	}
	TKeyButton Button;

public:
	// return a TKey for its associated String (eg KeyA for "KeyA")
	static	TKey				getKeyFromString(const std::string &str);
	// return the string equivalent to the TKey (eg "KeyA" for KeyA)
	static	const std::string	&getStringFromKey(TKey k);
};

/**
 * CEventKeyDown
 * Send when a key is push down. The key type is Key and FirstTime is true if the previous key state wasn't pushed.
 */
class CEventKeyDown : public CEventKey
{
public:
	CEventKeyDown (TKey key, TKeyButton button, bool bFirstTime, IEventEmitter* emitter) : CEventKey (button, emitter, EventKeyDownId)
	{
		Key=key;
		FirstTime=bFirstTime;
	}
	TKey Key;
	bool FirstTime;

	virtual	CEvent			*clone() const {return new CEventKeyDown(*this);}
};

/**
 * CEventKeyUp
 */
class CEventKeyUp : public CEventKey
{
public:
	CEventKeyUp (TKey key, TKeyButton button, IEventEmitter* emitter) : CEventKey (button, emitter, EventKeyUpId)
	{
		Key=key;
	}
	TKey Key;

	virtual	CEvent			*clone() const {return new CEventKeyUp(*this);}
};

/**
 * CEventChar
 */
class CEventChar : public CEventKey
{
public:
	CEventChar (u32char c, TKeyButton button, IEventEmitter* emitter) : CEventKey (button, emitter, EventCharId), _Raw(true)
	{
		Char=c;
	}
	u32char Char;

	virtual	CEvent			*clone() const {return new CEventChar(*this);}
	void					setRaw( bool raw ) { _Raw = raw; }
	bool					isRaw() const { return _Raw; }

private:
	bool	_Raw; // true if raw, false if composed by an IME

};

/**
 * CEventString
 */
class CEventString : public CEventKey
{
public:
	CEventString (const std::string &str, IEventEmitter* emitter) : CEventKey (noKeyButton, emitter, EventStringId)
	{
		String = str;
	}
	std::string String;

	virtual	CEvent			*clone() const {return new CEventString(*this);}
};

/**
 * CEventMouse.
 * Base for mouse events.
 */
class CEventMouse : public CEvent
{
public:
	float X,Y;
	TMouseButton Button;

	CEventMouse (float x, float y, TMouseButton button, IEventEmitter* emitter, const CClassId& classId) : CEvent (emitter, classId)
	{
		X = x;
		Y = y;
		Button = button;
	}
};


/**
 * CEventMouseDown
 * Send when a single mouse button is pushed down. The Button value should have only ONE flag set between leftButton, rightButton and middleButton.
 * X and Y have the new mouse position in window coordinate system.
 */
class CEventMouseDown : public CEventMouse
{
public:
	CEventMouseDown (float x, float y, TMouseButton button, IEventEmitter* emitter) : CEventMouse (x, y, button, emitter, EventMouseDownId)
	{}

	virtual	CEvent			*clone() const {return new CEventMouseDown(*this);}
};


/**
 * CEventMouseUp
 * Send when a single mouse button is pushed down. The Button value should have only ONE flag set between leftButton, rightButton and middleButton.
 * X and Y have the new mouse position in window coordinate system.
 */
class CEventMouseUp : public CEventMouse
{
public:
	CEventMouseUp (float x, float y, TMouseButton button, IEventEmitter* emitter) : CEventMouse (x, y, button, emitter, EventMouseUpId)
	{}

	virtual	CEvent			*clone() const {return new CEventMouseUp(*this);}
};


/**
 * CEventMouseMove
 * Button have the state of the three mouse and SHIFT CTRL and ALT system keys. When the flag is set, the button is pushed.
 * X and Y have the new mouse position in window coordinate system.
 */
class CEventMouseMove : public CEventMouse
{
public:
	CEventMouseMove (float x, float y, TMouseButton button, IEventEmitter* emitter) : CEventMouse (x, y, button, emitter, EventMouseMoveId)
	{}

	virtual	CEvent			*clone() const {return new CEventMouseMove(*this);}
};


/**
 * CEventMouseDblClk
 * Send when a single mouse button is double clicked. The Button value should have only ONE flag set between leftButton, rightButton and middleButton.
 * X and Y have the new mouse position in window coordinate system.
 */
class CEventMouseDblClk : public CEventMouse
{
public:
	CEventMouseDblClk (float x, float y, TMouseButton button, IEventEmitter* emitter) : CEventMouse (x, y, button, emitter, EventMouseDblClkId)
	{}

	virtual	CEvent			*clone() const {return new CEventMouseDblClk(*this);}
};


/**
 * CEventMouseWheel
 * Send when the mouse wheel is actioned.
 * Button have the state of the three mouse and SHIFT CTRL and ALT system keys. When the flag is set, the button is pushed.
 * X and Y have the new mouse position in window coordinate system.
 * If Direction is true, the wheel was moved forward and if it is false, backward.
 */
class CEventMouseWheel : public CEventMouse
{
public:
	bool	Direction;
	CEventMouseWheel (float x, float y, TMouseButton button, bool direction, IEventEmitter* emitter) : CEventMouse (x, y, button, emitter, EventMouseWheelId)
	{
		Direction=direction;
	}

	virtual	CEvent			*clone() const {return new CEventMouseWheel(*this);}
};


/**
 * CEventActivate. Called when window is actived / disactived.
 */
class CEventActivate : public CEvent
{
public:
	/**
	  * True if window is actived, false if it is disactived.
	  */
	bool Activate;

	/**
	  * Create an activate event. Notify the activation disactivation of a window.
	  * \param activate is True if window is actived, false if it is disactived.
	  */
	CEventActivate (bool activate, IEventEmitter* emitter) : CEvent (emitter, EventActivateId)
	{
		Activate = activate;
	}

	virtual	CEvent			*clone() const {return new CEventActivate(*this);}
};


/**
 * CEventSetFocus. Called when window lost / get keyboard focus.
 */
class CEventSetFocus : public CEvent
{
public:
	/**
	  * True if window get the focus, false if it lost it.
	  */
	bool Get;

	/**
	  * Create focus event. Notify get and lost of the keyboard focus of a window.
	  * \param activate is True if window get the focus, false if it lost it.
	  */
	CEventSetFocus (bool get, IEventEmitter* emitter) : CEvent (emitter, EventSetFocusId)
	{
		Get = get;
	}

	virtual	CEvent			*clone() const {return new CEventSetFocus(*this);}
};


/**
 * CEventDestroyWindow
 */
class CEventDestroyWindow : public CEvent
{
public:
	CEventDestroyWindow (IEventEmitter* emitter) : CEvent (emitter, EventDestroyWindowId)
	{
	}

	virtual	CEvent			*clone() const {return new CEventDestroyWindow(*this);}
};

/**
 * CEventCloseWindow
 */
class CEventCloseWindow : public CEvent
{
public:
	CEventCloseWindow (IEventEmitter* emitter) : CEvent (emitter, EventCloseWindowId)
	{
	}

	virtual	CEvent			*clone() const {return new CEventCloseWindow(*this);}
};

/**
 * CEventIME
 */
class CEventIME : public CEvent
{
public:
	CEventIME (uint32 msg, uint32 wParam, uint32 lParam, IEventEmitter* emitter) : CEvent (emitter, EventIME), EventMessage(msg), WParam(wParam), LParam(lParam)
	{}

	uint32	EventMessage;
	uint32	WParam, LParam;

	virtual CEvent			*clone() const {return new CEventIME(*this);}
};

/**
 * CEventDisplayChange : Called user has changed the desktop resolution
 */
class CEventDisplayChange : public CEvent
{
public:
	uint Width;
	uint Height;
	uint BitDepth;

	/**
	  * Create focus event. Notify get and lost of the keyboard focus of a window.
	  * \param activate is True if window get the focus, false if it lost it.
	  */
	CEventDisplayChange(uint width, uint height, uint bitDepth, IEventEmitter* emitter) : CEvent (emitter, EventDisplayChangeId)
	{
		Width    = width;
		Height   = height;
		BitDepth = bitDepth;
	}

	virtual	CEvent			*clone() const {return new CEventDisplayChange(*this);}
};


} // NLMISC


#endif // NL_EVENTS_H

/* End of events.h */
