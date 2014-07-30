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
#include "di_keyboard_device.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/win_event_emitter.h"
#include <dinput.h>
#include <Winuser.h>

#include "Mmsystem.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// used to do a conversion from DX key code to Nel keys enums
struct CKeyConv
{
	uint DIKey;
	TKey NelKey;
	const char *KeyName;
	bool Repeatable;
};

// this is used to build a conversion table
static const CKeyConv DIToNel[] =
{
	//
	{DIK_F1, KeyF1, "F1", true},
	{DIK_F2, KeyF2, "F2", true},
	{DIK_F3, KeyF3, "F3", true},
	{DIK_F4, KeyF4, "F4", true},
	{DIK_F5, KeyF5, "F5", true},
	{DIK_F6, KeyF6, "F6", true},
	{DIK_F7, KeyF7, "F7", true},
	{DIK_F8, KeyF8, "F8", true},
	{DIK_F9, KeyF9, "F9", true},
	{DIK_F10, KeyF10, "F10", true},
	{DIK_F11, KeyF11, "F11", true},
	{DIK_F12, KeyF12, "F12", true},
	{DIK_F13, KeyF13, "F13", true},
	{DIK_F14, KeyF14, "F14", true},
	{DIK_F15, KeyF15, "F15", true},
	//
	{DIK_NUMPAD0, KeyNUMPAD0, "NUMPAD0", true},
	{DIK_NUMPAD1, KeyNUMPAD1, "NUMPAD1", true},
	{DIK_NUMPAD2, KeyNUMPAD2, "NUMPAD2", true},
	{DIK_NUMPAD3, KeyNUMPAD3, "NUMPAD3", true},
	{DIK_NUMPAD4, KeyNUMPAD4, "NUMPAD4", true},
	{DIK_NUMPAD5, KeyNUMPAD5, "NUMPAD5", true},
	{DIK_NUMPAD6, KeyNUMPAD6, "NUMPAD6", true},
	{DIK_NUMPAD7, KeyNUMPAD7, "NUMPAD7", true},
	{DIK_NUMPAD8, KeyNUMPAD8, "NUMPAD8", true},
	{DIK_NUMPAD9, KeyNUMPAD9, "NUMPAD9", true},
	//
	{DIK_DIVIDE, KeyDIVIDE, "/", true},
	{DIK_DECIMAL, KeyDECIMAL, "NUMPAD .", true},
	//
	{DIK_LSHIFT, KeyLSHIFT, "LEFT SHIFT", false},
	{DIK_RSHIFT, KeyRSHIFT, "RIGHT SHIFT", false},
	//
	{DIK_LCONTROL, KeyLCONTROL, "LEFT CONTROL", false},
	{DIK_RCONTROL, KeyRCONTROL, "RIGHT CONTROL", false},
	//
	{DIK_LMENU, KeyLMENU, "ALT", false},
	{DIK_RMENU, KeyRMENU, "ALT GR", false},
	//
	{DIK_UP, KeyUP, "UP", true},
	{DIK_PRIOR, KeyPRIOR, "PRIOR", true},
	{DIK_LEFT, KeyLEFT, "LEFT", true},
	{DIK_RIGHT, KeyRIGHT, "RIGHT", true},
	{DIK_END, KeyEND, "END", true},
	{DIK_DOWN, KeyDOWN, "DOWN", true},
	{DIK_NEXT, KeyNEXT, "NEXT", true},
	{DIK_INSERT, KeyINSERT, "INSERT", true},
	{DIK_DELETE, KeyDELETE, "DELETE", true},
	{DIK_HOME, KeyHOME, "HOME", true},
	{DIK_LWIN, KeyLWIN, "LEFT WIN", false},
	{DIK_RWIN, KeyRWIN, "RIGHT WIN", false},
	{DIK_APPS, KeyAPPS, "APPS", false},
	{DIK_BACK, KeyBACK, "BACK", true},
	//
	{DIK_SYSRQ, KeySNAPSHOT, "SNAPSHOT", false},
	{DIK_SCROLL, KeySCROLL, "SCROLL", false},
	{DIK_PAUSE, KeyPAUSE, "PAUSE", false},
	//
	{DIK_NUMLOCK, KeyNUMLOCK, "NUMLOCK", false},
	//
	{DIK_NUMPADENTER, KeyRETURN, "RETURN", true},
	//{DIK_NUMPADENTER, KeyRETURN, "ENTER", true},
	//
	{DIK_CONVERT, KeyCONVERT, "CONVERT", false},
	{DIK_NOCONVERT, KeyNONCONVERT, "NOCONVERT", true},
	//
	{DIK_KANA, KeyKANA, "KANA", false},
	{DIK_KANJI, KeyKANJI, "KANJI", false},
};


///========================================================================
const CKeyConv	*CDIKeyboard::DIKeyToNelKeyTab[CDIKeyboard::NumKeys];

///========================================================================
CDIKeyboard::CDIKeyboard(CWinEventEmitter *we, HWND hwnd)
:	_Keyboard(NULL),
	_WE(we),
	ShiftPressed(false),
	CtrlPressed(false),
	AltPressed(false),
	_CapsLockToggle(true),
	_hWnd(hwnd),
	_RepeatDelay(250),
	_RepeatPeriod(200),
	_FirstPressDate(-1),
	_LastDIKeyPressed(0)
{
	if (::GetKeyboardState((PBYTE) _VKKeyState) == FALSE)
	{
		std::fill(_VKKeyState, _VKKeyState + NumKeys, 0);
	}
	//	test whether the user toggle its keyboard with shift or not..
	HKEY hKey;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, "Keyboard Layout", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD type = REG_DWORD;
		DWORD value;
		DWORD size = sizeof(DWORD);
		if (::RegQueryValueEx(hKey, "Attributes", NULL, &type, (LPBYTE) &value, &size) == ERROR_SUCCESS)
		{
			_CapsLockToggle = (value & (1 << 16)) == 0;
		}
		::RegCloseKey(hKey);
	}
	// get repeat delay and period
	int keybDelay;
	if (::SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &keybDelay, 0) != 0)
	{
		_RepeatDelay = 250 + 250 * keybDelay;
	}
	DWORD keybSpeed;
	if (::SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &keybSpeed, 0) != 0)
	{
		_RepeatPeriod = (uint) (1000.f / (keybSpeed * (27.5f / 31.f) + 2.5f));
	}
	// get keyboard layout
	_KBLayout = ::GetKeyboardLayout(0);

	_RepetitionDisabled.resize(NumKeys);
	_RepetitionDisabled.clearAll();
}

///========================================================================
void	CDIKeyboard::updateVKKeyState(uint diKey, bool pressed, TKey &keyValue, TKey &charValue)
{
	bool extKey;
	bool repeatable;
	keyValue = DIKeyToNelKey(diKey, extKey, repeatable);
	//
	if (keyValue == 0)
	{
		charValue = keyValue;
		return;
	}
	//
	if (pressed)
	{
		// check for toggle key
		switch (keyValue)
		{
			case KeyPAUSE:
			case KeyKANA:
			case KeyKANJI:
				_VKKeyState[keyValue] ^= 0x01; // toggle first bit
			break;
			case KeyCAPITAL:
				if (_CapsLockToggle)
				{
					_VKKeyState[keyValue] ^= 0x01;
					//toggleCapsLock(false);
				}
				else
				{
					if ((_VKKeyState[keyValue] & 0x01) == 0)
					{
						_VKKeyState[keyValue] |= 0x01;
						//toggleCapsLock(false);
					}
				}
			break;
			case KeyNUMLOCK:
				_VKKeyState[keyValue] ^= 0x01;
				//setNumLock((_VKKeyState[keyValue] & 0x01) != 0);
			break;
			case KeySCROLL:
				_VKKeyState[keyValue] ^= 0x01;
				//toggleScrollLock();
			break;

		}

		_VKKeyState[keyValue] |= 0x80;
	}
	else
	{
		_VKKeyState[keyValue] &= ~0x80;
	}
	//
	switch (keyValue)
	{
		case KeyLSHIFT: charValue = KeySHIFT; break;
		case KeyRSHIFT: charValue = KeySHIFT; break;
		case KeyLCONTROL: charValue = KeyCONTROL; break;
		case KeyRCONTROL: charValue = KeyCONTROL; break;
		case KeyLMENU: charValue = KeyMENU; break;
		case KeyRMENU: charValue = KeyMENU; break;
		default: charValue = keyValue; break;
	}
	//
	if (charValue == KeySHIFT && !_CapsLockToggle)
	{
		if (_VKKeyState[KeyCAPITAL] & 0x01)
		{
			_VKKeyState[KeyCAPITAL] &= ~0x01;
			//toggleCapsLock(true);
		}
	}
	//
	if (charValue != keyValue)
	{
		_VKKeyState[charValue] = _VKKeyState[keyValue];
	}
	//
	updateCtrlAltShiftValues();
}

///========================================================================
void	CDIKeyboard::updateCtrlAltShiftValues()
{
	ShiftPressed = (_VKKeyState[KeySHIFT] & 0x80) != 0;
	CtrlPressed = (_VKKeyState[KeyCONTROL] & 0x80) != 0;
	AltPressed = (_VKKeyState[KeyMENU] & 0x80) != 0;
}

///========================================================================
CDIKeyboard::~CDIKeyboard()
{
	if (_Keyboard)
	{
		_Keyboard->Unacquire();
		_Keyboard->Release();
	}
}

///========================================================================
CDIKeyboard *CDIKeyboard::createKeyboardDevice(IDirectInput8 *di8,
											   HWND hwnd,
											   CDIEventEmitter *diEventEmitter,
											   CWinEventEmitter *we
											  ) throw(EDirectInput)
{
	std::auto_ptr<CDIKeyboard> kb(new CDIKeyboard(we, hwnd));
	kb->_DIEventEmitter = diEventEmitter;
	HRESULT result = di8->CreateDevice(GUID_SysKeyboard, &kb->_Keyboard, NULL);
	if (result != DI_OK) throw EDirectInputNoKeyboard();
	result = kb->_Keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (result != DI_OK) throw EDirectInputCooperativeLevelFailed();
	result = kb->_Keyboard->SetDataFormat(&c_dfDIKeyboard);
	kb->setBufferSize(16);
	kb->_Keyboard->Acquire();

	// Enable win32 keyboard messages only if hardware mouse in normal mode
	if (kb->_WE)
		kb->_WE->enableKeyboardEvents(false);

	return kb.release();
}

///========================================================================
void CDIKeyboard::poll(CInputDeviceServer *dev)
{
	nlassert(_Keyboard);
	nlassert(_KeyboardBufferSize > 0);
	static std::vector<DIDEVICEOBJECTDATA> datas;
	datas.resize(_KeyboardBufferSize);
	DWORD numElements = _KeyboardBufferSize;
	HRESULT result = _Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
	if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST)
	{
		result = _Keyboard->Acquire();
		if (result != DI_OK) return;
		// get device state
		::GetKeyboardState((unsigned char *) _VKKeyState);
		_LastDIKeyPressed = 0;
		updateCtrlAltShiftValues();
		result = _Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
		if (result != DI_OK) return;
	}
	else if (result != DI_OK)
	{
		return;
	}

	_PollTime = (uint32) CTime::getLocalTime();


	// process each message in the list
	for	(uint k = 0; k < numElements; ++k)
	{
		CDIEvent *die = new CDIEvent;
		die->Emitter = this;
		die->Datas = datas[k];
		dev->submitEvent(die);
	}
}

///========================================================================
void	CDIKeyboard::transitionOccured(CEventServer *server, const IInputDeviceEvent *nextMessage)
{
	repeatKey(buildDateFromEvent(nextMessage), server);
}

///========================================================================
TKeyButton CDIKeyboard::buildKeyButtonsFlags() const
{
	return (TKeyButton) ( (ShiftPressed   ? shiftKeyButton : 0)
						  | (CtrlPressed  ? ctrlKeyButton   : 0)
						  | (AltPressed   ? altKeyButton  : 0)
						);
}

///========================================================================
void CDIKeyboard::keyTriggered(bool pressed, uint dikey, CEventServer *server, uint32 date)
{
	#if 0
		const uint numPairs = sizeof(DIToNel) / sizeof(CKeyConv);
		for (uint k = 0; k < numPairs; ++k)
		{
			if (DIToNel[k].DIKey == key)
			{
				nlinfo(DIToNel[k].KeyName);
			}
		}
	#endif


	TKey keyValue, charValue;
	updateVKKeyState(dikey, pressed, keyValue, charValue);
	if (keyValue == 0) return;

	CEventKey *ek;
	if (pressed )
	{
		ek = new CEventKeyDown(keyValue, buildKeyButtonsFlags(), true, _DIEventEmitter);
	}
	else
	{
		ek = new CEventKeyUp(keyValue, buildKeyButtonsFlags(), _DIEventEmitter);
	}
	server->postEvent(ek);

	if (pressed)
	{
		if (_RepetitionDisabled[(uint) keyValue] == false)
		{
			_LastEmitDate = _FirstPressDate = date;
			_LastDIKeyPressed = dikey;
		}
		else // not a repeatable key
		{
			_LastDIKeyPressed = 0;
			return;
		}
	}
	else
	{
		// key released ?
		if (dikey == _LastDIKeyPressed)
		{
			_LastDIKeyPressed = 0;
		}

		if (_RepetitionDisabled[(uint) keyValue] == true)
		{
			return;
		}
	}

	// first char event (if repetition not disabled)
	if (keyValue >= KeyNUMPAD0 && keyValue <= KeyNUMPAD9 || keyValue == KeyDECIMAL)
	{
		if ((_VKKeyState[KeyNUMLOCK] & 0x01) != 0)
		{
			sendUnicode(charValue, dikey, server, pressed);
		}
	}
	else
	{
		sendUnicode(charValue, dikey, server, pressed);
	}

	_FirstPressDate  = (uint32) NLMISC::CTime::getLocalTime(); // can't use the time stamp, because we can't not sure it matches the local time.
	                                                  // time stamp is used for evenrts sorting only
}

///========================================================================
void CDIKeyboard::submit(IInputDeviceEvent *deviceEvent, CEventServer *server)
{
	CDIEvent *die = safe_cast<CDIEvent *>(deviceEvent);
	bool pressed = (die->Datas.dwData & 0x80) != 0;
	keyTriggered(pressed, (uint) die->Datas.dwOfs, server, die->Datas.dwTimeStamp);
}

///========================================================================
TMouseButton	CDIKeyboard::buildKeyboardButtonFlags() const
{
	nlassert(_Keyboard);
	return _DIEventEmitter->buildKeyboardButtonFlags();
}

///========================================================================
bool	CDIKeyboard::setBufferSize(uint size)
{
	nlassert(size > 0);
	nlassert(_Keyboard);
	_Keyboard->Unacquire();
	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = size;
	HRESULT					r = _Keyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	if (r != DI_OK)			return false;
	_KeyboardBufferSize = size;
	return true;
}

///========================================================================
uint	CDIKeyboard::getBufferSize() const
{
	return _KeyboardBufferSize;
}

///========================================================================
TKey CDIKeyboard::DIKeyToNelKey(uint diKey, bool &extKey, bool &repeatable)
{
	// some key are not handled by MapVirtualKeyEx so we need to convert them ourselves
	static bool tableBuilt = false;

	if (!tableBuilt)
	{
		uint k;
		for (k = 0; k < NumKeys; ++k)
		{
			DIKeyToNelKeyTab[k] = NULL; // set as not a valid key by default
		}
		const uint numPairs = sizeof(DIToNel) / sizeof(CKeyConv);
		for (k = 0; k < numPairs; ++k)
		{
			DIKeyToNelKeyTab[DIToNel[k].DIKey] = &DIToNel[k];
		}
		tableBuilt = true;
	}


	//
	if (DIKeyToNelKeyTab[diKey] != NULL)
	{
		const CKeyConv &keyConv = *DIKeyToNelKeyTab[diKey];
		extKey     = true;
		repeatable = keyConv.Repeatable;
		return keyConv.NelKey;
	}



	// try doing the conversion using MapVirtualKey
	TKey key = (TKey) ::MapVirtualKeyEx(diKey, 1, _KBLayout);
	extKey = false;
	return key;
}

///========================================================================
void	CDIKeyboard::sendUnicode(TKey vkey, uint dikey, CEventServer *server, bool pressed)
{
	uint8 oldShift = _VKKeyState[KeySHIFT];
	/// If caps lock is off when pressing shift, we must disable shift, to get no minuscule letters when it is pressed and capslocks is on.
	if (!_CapsLockToggle && _VKKeyState[KeyCAPITAL] & 0x01)
	{
		_VKKeyState[KeySHIFT] = 0;
	}
	// 'ToUnicode??' is supported since NT4.0 only
	// Check if there's support


	static bool init = false;
	static bool toUnicodeSupported = false;
	if (!init)
	{
		init = true;
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (::GetVersionEx (&osvi))
		{
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				if (osvi.dwMajorVersion >= 4)
				{
					toUnicodeSupported = true;
				}
			}
		}
	}


	if (toUnicodeSupported)
	{
		const uint maxNumKeys = 8;
		WCHAR keyUnicodes[maxNumKeys];
		int res = ::ToUnicodeEx(vkey, dikey | (pressed ? 0 : (1 << 15)), (unsigned char *) _VKKeyState, keyUnicodes, maxNumKeys, 0, _KBLayout);
		//
		_VKKeyState[KeySHIFT] = oldShift;
		//
		for (sint k = 0; k < res; ++k)
		{
			CEventChar *evc = new CEventChar((ucchar) keyUnicodes[k], buildKeyButtonsFlags(), _DIEventEmitter);
			server->postEvent(evc);
		}
	}
	else
	{
		unsigned char buf[2];
		int res = ::ToAsciiEx(vkey, dikey | (pressed ? 0 : (1 << 15)), (unsigned char *) _VKKeyState, (LPWORD) buf, 0, _KBLayout);
		for (sint k = 0; k < res; ++k)
		{
			CEventChar *evc = new CEventChar((ucchar) buf[k], buildKeyButtonsFlags(), _DIEventEmitter);
			server->postEvent(evc);
		}
	}
}

///========================================================================
void	CDIKeyboard::repeatKey(uint32 currentDate, CEventServer *server)
{
	if (_LastDIKeyPressed == 0 || _LastDIKeyPressed == DIK_INSERT) return;
	bool extKey;
	bool repeatable;
	TKey vkey = DIKeyToNelKey(_LastDIKeyPressed, extKey, repeatable);
	if (vkey == 0) return;
	if (currentDate - _FirstPressDate < _RepeatDelay) return;

	sint32 firstDate = _LastEmitDate - (_FirstPressDate + _RepeatDelay);
	sint32 lastDate = currentDate - (_FirstPressDate + _RepeatDelay);
	if (firstDate < 0) firstDate = 0;

	if (lastDate < firstDate) return;

	uint numRep = (uint) ((lastDate + _RepeatPeriod - 1) / _RepeatPeriod  - (firstDate + _RepeatPeriod - 1) / _RepeatPeriod);
	//numRep = std::min(16u, numRep); // too much repetitions don't make sense...
	if ((sint) numRep < 0) return; // 50 days loop..
	numRep = 1; // fix : for now it seems better to limit the number of repetition to 1 per frame (it can be greater than 1 only if framerate is slow, but its not very useable)


	// numpad case
	if (vkey >= KeyNUMPAD0 && vkey <= KeyNUMPAD9 || vkey == KeyDECIMAL)
	{
		// check whether numlock is activated
		if ((_VKKeyState[KeyNUMLOCK] & 0x01) != 0)
		{
			for (uint k = 0; k < numRep; ++k)
			{
				sendUnicode(vkey, _LastDIKeyPressed, server, true);
			}
		}
		else
		{
			// arrow, home, end.. events
			for (uint k = 0; k < numRep; ++k)
			{
				CEventKey *ek = new CEventKeyDown(vkey, buildKeyButtonsFlags(), false, _DIEventEmitter);
				server->postEvent(ek);
			}
		}
	}
	else
	{
		for (uint k = 0; k < numRep; ++k)
		{
			// if it is an extended key, repetition won't be managed by sendUnicode
			if (extKey && repeatable)
			{
				CEventKey *ek = new CEventKeyDown(vkey, buildKeyButtonsFlags(), false, _DIEventEmitter);
				server->postEvent(ek);
			}
			else
			{
				sendUnicode(vkey, _LastDIKeyPressed, server, true);
			}
		}
	}

	_LastEmitDate = currentDate;
}

///========================================================================
uint32	CDIKeyboard::buildDateFromEvent(const IInputDeviceEvent *deviceEvent)
{
	if (deviceEvent)
	{
		const CDIEvent *die = safe_cast<const CDIEvent *>(deviceEvent);
		return (uint32) die->Datas.dwData;
	}
	else
	{
		return _PollTime;
	}
}

///========================================================================
void CDIKeyboard::disableRepetition(const TKey *keyTab, uint numKey)
{
	_RepetitionDisabled.clearAll();
	for (uint k = 0; k < numKey; ++k)
	{
		_RepetitionDisabled.set((sint) keyTab[k]);
	}

	if (_LastDIKeyPressed != 0)
	{
		bool extKey;
		bool repeatable;
		TKey key = DIKeyToNelKey(_LastDIKeyPressed, extKey, repeatable);
		if (_RepetitionDisabled[(uint) key])
		{
			// disable this key repetition
			_LastDIKeyPressed = 0;
		}
	}
}

///========================================================================
uint CDIKeyboard::getNumDisabledRepetition() const
{
	uint numKey = 0;
	for (uint k = 0; k < NumKeys; ++k)
	{
		if (_RepetitionDisabled[k]) ++numKey;
	}
	return numKey;
}

///========================================================================
void CDIKeyboard::getDisabledRepetitions(TKey *destTab) const
{
	for (uint k = 0; k < NumKeys; ++k)
	{
		if (_RepetitionDisabled[k]) *destTab++ = (TKey) k;
	}
}



} // NLMISC

#endif // NL_OS_WINDOWS
