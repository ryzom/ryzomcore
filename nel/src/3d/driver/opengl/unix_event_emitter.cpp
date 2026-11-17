// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl.h"
#include "unix_event_emitter.h"

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include "nel/misc/debug.h"
#include "nel/misc/utf_string_view.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

typedef bool (*x11Proc)(NL3D::IDriver *drv, XEvent *e);

static Atom XA_CLIPBOARD = 0;
static Atom XA_UTF8_STRING = 0;
static Atom XA_TARGETS = 0;
static Atom XA_NEL_SEL = 0;
static Atom XA_WM_DELETE_WINDOW = 0;

namespace NLMISC {

CUnixEventEmitter::CUnixEventEmitter ():_dpy(nullptr)
    , _win(0), _im(nullptr)
    , _ic(nullptr)
    , _driver(nullptr)
{
	_SelectionOwned = false;
	m_LostIM = false;
	m_Focus = false;
	m_FilteredKeycode = 0;
	m_FilteredTime = 0;
}

CUnixEventEmitter::~CUnixEventEmitter()
{
	closeIM();
}

void CUnixEventEmitter::init(Display *dpy, Window win, NL3D::IDriver *driver, bool ownWindow)
{
	_dpy = dpy;
	_win = win;
	_driver = driver;

	// the property changes are for the driver: the window manager states a minimized window in a property
	XSelectInput (_dpy, _win, KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask|ExposureMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|PropertyChangeMask);

	// define Atoms used by clipboard
	XA_CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False);
	XA_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
	XA_TARGETS = XInternAtom(dpy, "TARGETS", False);
	XA_NEL_SEL = XInternAtom(dpy, "NeL_SEL", False);

	// define Atom used by delete window
	XA_WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	// The property is replaced as a whole: on a window of the application this took away the protocols its
	// toolkit had set (WM_TAKE_FOCUS, _NET_WM_PING), and its owner gets the close request anyway.
	if (ownWindow)
		XSetWMProtocols(dpy, win, &XA_WM_DELETE_WINDOW, 1);

/*
	TODO: implements all useful events processing
	ButtonMotionMask|Button1MotionMask|Button2MotionMask|
	Button3MotionMask|Button4MotionMask|Button5MotionMask|KeymapStateMask|
	SubstructureNotifyMask|VisibilityChangeMask|PropertyChangeMask|
	ColormapChangeMask|OwnerGrabButtonMask
*/

	createIM();
}

#ifdef X_HAVE_UTF8_STRING
// Xlib calls this when the input method stops, for example when its server is restarted
static void inputMethodDestroyed(XIM /* im */, XPointer clientData, XPointer /* callData */)
{
	((CUnixEventEmitter *)clientData)->lostIM();
}
#endif

void CUnixEventEmitter::createIM()
{
#ifdef X_HAVE_UTF8_STRING

	m_LostIM = false;

	if (_ic != nullptr)
		return;

	// Xlib chooses the input method, and the encoding of what it gives, by the locale LC_CTYPE has while the
	// input method and its context are made. When LC_CTYPE is the C locale, this function takes the locale of
	// the environment from here to where the input context exists, and then sets LC_CTYPE back to what it
	// found, so that opening the input method changes the locale of nothing else: with the LC_CTYPE of the
	// user, tolower() and the comparisons that ignore case follow the language of the user (in a Turkish
	// locale "QUIT" and "quit" are not the same word). What LC_CTYPE is for the rest of the process is not
	// decided here. The input method keeps the locale it was opened with. The string setlocale returns does
	// not survive the next call.
	const char *ctype = setlocale(LC_CTYPE, nullptr);
	const std::string savedCtype = ctype ? ctype : "";
	if (savedCtype.empty() || savedCtype == "C" || savedCtype == "POSIX")
	{
		if (setlocale(LC_CTYPE, "") == nullptr)
			nlwarning("3D: The locale of the environment (LC_ALL, LC_CTYPE or LANG) is not installed, text input uses the C locale");
	}

	if (!XSupportsLocale())
	{
		ctype = setlocale(LC_CTYPE, nullptr);
		nlwarning("3D: Xlib does not support the locale '%s', text input uses the C locale", ctype ? ctype : "");
		setlocale(LC_CTYPE, "C");
	}

	// The input methods to try. "" is the one of the environment: the server named by XMODIFIERS (such as
	// "@im=ibus"), or without XMODIFIERS the dead keys and compose sequences of the locale, handled by Xlib.
	// The others are for a server that does not answer or cannot give the input style asked for below: a
	// modifier given here goes before the one of XMODIFIERS, so they are Xlib's own handling whatever the
	// environment says.
	static const char *const s_Modifiers[] = { "", "@im=local", "@im=" };
	uint firstModifiers = 0;

	// In the C locale the text an input method server commits is lost: the letter of a dead key never
	// arrives, which is how dead keys broke with ibus when its server was asked for whatever the locale.
	// Xlib's own handling gives them there.
	ctype = setlocale(LC_CTYPE, nullptr);
	if (ctype == nullptr || strcmp(ctype, "C") == 0 || strcmp(ctype, "POSIX") == 0)
	{
		firstModifiers = 1;

		const char *xmodifiers = getenv("XMODIFIERS");
		if (xmodifiers != nullptr && xmodifiers[0] != '\0')
			nlwarning("3D: LC_CTYPE is the C locale, the input method of XMODIFIERS (%s) is not used: its text would be lost. Set LANG or LC_CTYPE to a UTF-8 locale to use it", xmodifiers);
	}

	for (uint i = firstModifiers; i < sizeof(s_Modifiers) / sizeof(s_Modifiers[0]) && _ic == nullptr; ++i)
	{
		if (XSetLocaleModifiers(s_Modifiers[i]) == nullptr)
			continue;

		_im = XOpenIM(_dpy, nullptr, nullptr, nullptr);
		if (_im == nullptr)
			continue;

		_ic = XCreateIC(_im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, _win, XNFocusWindow, _win, NULL);
		if (_ic == nullptr)
		{
			XCloseIM(_im);
			_im = nullptr;
		}
	}

	// the modifiers the input method was opened with are those of the locale it was opened in
	const char *modifiers = XSetLocaleModifiers(nullptr);
	const std::string imModifiers = modifiers ? modifiers : "";

	// LC_CTYPE as the process had it
	if (!savedCtype.empty())
		setlocale(LC_CTYPE, savedCtype.c_str());

	if (_ic == nullptr)
	{
		nlwarning("3D: No X input method could be opened (XOpenIM or XCreateIC failed), dead keys and compose sequences do not work");
		return;
	}

	// when the input method stops, Xlib frees it and its context: lostIM() forgets them
	XIMCallback destroyCallback;
	destroyCallback.client_data = (XPointer)this;
	destroyCallback.callback = inputMethodDestroyed;
	XSetIMValues(_im, XNDestroyCallback, &destroyCallback, NULL);

	if (m_Focus)
		XSetICFocus(_ic);

	const char *imLocale = XLocaleOfIM(_im);
	nlinfo("3D: X input method opened with the locale '%s' and the modifiers '%s'", imLocale ? imLocale : "", imModifiers.c_str());
#endif
}

void CUnixEventEmitter::closeIM()
{
#ifdef X_HAVE_UTF8_STRING
	if (_ic)
	{
		XDestroyIC(_ic);
		_ic = nullptr;
	}

	if (_im)
	{
		XCloseIM(_im);
		_im = nullptr;
	}

	m_LostIM = false;
#endif
}

void CUnixEventEmitter::lostIM()
{
	// Xlib has freed them, or is about to: they must not be used, closed or destroyed any more
	_im = nullptr;
	_ic = nullptr;
	m_LostIM = true;
}

void CUnixEventEmitter::submitEvents(CEventServer & server, bool allWindows)
{
	while (XPending(_dpy))
	{
		XEvent	Event;
		XNextEvent(_dpy, &Event);

		// An input method takes its events first, whatever their window: the connection to an input method
		// server has a window of its own. Key presses are filtered in processMessage, which posts the key
		// before the input method takes it, and key releases are never filtered, so that every key that went
		// down for the application also goes up.
		if (Event.type != KeyPress && Event.type != KeyRelease && XFilterEvent(&Event, None))
			continue;

		// the input method stopped: open the next one that answers
		if (m_LostIM)
			createIM();

		// a keyboard mapping change is for every window, its event has none
		if (allWindows || Event.xany.window == _win || Event.type == MappingNotify)
		{
			// nlinfo("event: %d", Event.type);
			if (_driver)
			{
				// forward X events to OpenGL driver
				x11Proc proc = (x11Proc)_driver->getWindowProc();

				if (proc)
					proc(_driver, &Event);
			}
			else
			{
				processMessage (Event, &server);
			}
		}
	}

	// Dispatch sent messages
	_InternalServer.setServer (&server);
	_InternalServer.pump (allWindows);
}

static Bool isMouseMoveEvent(Display *display, XEvent *event, XPointer arg)
{
	return (event->type == MotionNotify);
}

#ifndef AltMask
#  define AltMask     (Mod1Mask)
#endif

TMouseButton getMouseButton (uint32 state)
{
	uint32 button=noButton;
	if (state&Button1Mask) button|=leftButton;
	if (state&Button2Mask) button|=middleButton;
	if (state&Button3Mask) button|=rightButton;
	if (state&ControlMask) button|=ctrlButton;
	if (state&ShiftMask)   button|=shiftButton;
	if (state&AltMask)     button|=altButton;

	return (TMouseButton)button;
}

TKeyButton getKeyButton (uint32 state)
{
	uint32 button=noKeyButton;
	if (state&ControlMask) button|=ctrlKeyButton;
	if (state&ShiftMask)   button|=shiftKeyButton;
	if (state&AltMask)     button|=altKeyButton;

	return (TKeyButton)button;
}

TKey getKeyFromKeycode (uint keycode)
{
	// keycodes are depending on system
	switch (keycode)
	{
	case 0x0a: return Key1;
	case 0x0b: return Key2;
	case 0x0c: return Key3;
	case 0x0d: return Key4;
	case 0x0e: return Key5;
	case 0x0f: return Key6;
	case 0x10: return Key7;
	case 0x11: return Key8;
	case 0x12: return Key9;
	case 0x13: return Key0;
	case 0x14: return KeyLBRACKET;
	case 0x15: return KeyEQUALS;
	case 0x22: return KeyRBRACKET;
	case 0x23: return KeySEMICOLON;
	case 0x2f: return KeyCOMMA;
	case 0x30: return KeyTILDE;
	case 0x31: return KeyAPOSTROPHE;
	case 0x33: return KeyBACKSLASH;
	case 0x5e: return KeyOEM_102;
	case 0x5b: return KeyDECIMAL; // KPDL, on the layouts where it gives a period or a comma
	case 0x61: return KeyOEM_102; // AB11, the ro key of Japanese keyboards, as on Windows (it shares the name with LSGT on Brazilian ones)
	case 0x84: return KeyDASH; // AE13, the yen key of Japanese keyboards: no other key gives KeyDASH on X11
//	case 0x3a: return KeyCOMMA;
	case 0x3b: return KeyPERIOD;
	case 0x3c: return KeySLASH;
	case 0x3d: return KeyPARAGRAPH;
	// for non-standard keyboards, maps to QWERTY keys
	case 0x18: return KeyQ;
	case 0x19: return KeyW;
	case 0x1a: return KeyE;
	case 0x1b: return KeyR;
	case 0x1c: return KeyT;
	case 0x1d: return KeyY;
	case 0x1e: return KeyU;
	case 0x1f: return KeyI;
	case 0x20: return KeyO;
	case 0x21: return KeyP;
	case 0x26: return KeyA;
	case 0x27: return KeyS;
	case 0x28: return KeyD;
	case 0x29: return KeyF;
	case 0x2a: return KeyG;
	case 0x2b: return KeyH;
	case 0x2c: return KeyJ;
	case 0x2d: return KeyK;
	case 0x2e: return KeyL;
	case 0x34: return KeyZ;
	case 0x35: return KeyX;
	case 0x36: return KeyC;
	case 0x37: return KeyV;
	case 0x38: return KeyB;
	case 0x39: return KeyN;
	case 0x3a: return KeyM;
	default:
//	nlwarning("missing keycode 0x%x %d '%c'", keycode, keycode, keycode);
	break;
	}

	return KeyNOKEY;
}

TKey getKeyFromKeySym (KeySym keysym)
{
//	nlwarning("0x%x %d '%c'", keysym, keysym, keysym);
	switch (keysym)
	{
	case XK_BackSpace: return KeyBACK;
	case XK_Tab: return KeyTAB;
	case XK_ISO_Left_Tab: return KeyTAB; // Shift+Tab
	case XK_Return: return KeyRETURN;
	case XK_Sys_Req: return KeySNAPSHOT;
	case XK_Scroll_Lock: return KeySCROLL;
	case XK_Pause: return KeyPAUSE;
	case XK_Escape: return KeyESCAPE;
	case XK_Delete: return KeyDELETE;
	case XK_Home: return KeyHOME;
	case XK_Left: return KeyLEFT;
	case XK_Up: return KeyUP;
	case XK_Right: return KeyRIGHT;
	case XK_Down: return KeyDOWN;
	case XK_Page_Up: return KeyPRIOR;
	case XK_Page_Down: return KeyNEXT;
	case XK_End: return KeyEND;
	case XK_Print: return KeyPRINT;
	case XK_Insert: return KeyINSERT;
	case XK_Num_Lock: return KeyNUMLOCK;
	case XK_KP_0: return KeyNUMPAD0;
	case XK_KP_1: return KeyNUMPAD1;
	case XK_KP_2: return KeyNUMPAD2;
	case XK_KP_3: return KeyNUMPAD3;
	case XK_KP_4: return KeyNUMPAD4;
	case XK_KP_5: return KeyNUMPAD5;
	case XK_KP_6: return KeyNUMPAD6;
	case XK_KP_7: return KeyNUMPAD7;
	case XK_KP_8: return KeyNUMPAD8;
	case XK_KP_9: return KeyNUMPAD9;
	case XK_KP_Enter: return KeyRETURN;
	case XK_KP_Home: return KeyHOME;
	case XK_KP_Left: return KeyLEFT;
	case XK_KP_Up: return KeyUP;
	case XK_KP_Right: return KeyRIGHT;
	case XK_KP_Down: return KeyDOWN;
	case XK_KP_Page_Up: return KeyPRIOR;
	case XK_KP_Page_Down: return KeyNEXT;
	case XK_KP_End: return KeyEND;
	case XK_KP_Begin: return KeyCANCEL;
	case XK_KP_Insert: return KeyINSERT;
	case XK_KP_Delete: return KeyDELETE;
	case XK_KP_Multiply: return KeyMULTIPLY;
	case XK_KP_Add: return KeyADD;
	case XK_KP_Subtract: return KeySUBTRACT;
	case XK_KP_Decimal: return KeyDECIMAL;
	case XK_KP_Separator: return KeyDECIMAL; // the decimal comma of the keypad, as on Windows
//	case XK_period: return KeyDECIMAL;
	case XK_KP_Divide: return KeyDIVIDE;
	case XK_F1: return KeyF1;
	case XK_F2: return KeyF2;
	case XK_F3: return KeyF3;
	case XK_F4: return KeyF4;
	case XK_F5: return KeyF5;
	case XK_F6: return KeyF6;
	case XK_F7: return KeyF7;
	case XK_F8: return KeyF8;
	case XK_F9: return KeyF9;
	case XK_F10: return KeyF10;
	case XK_F11: return KeyF11;
	case XK_F12: return KeyF12;
	case XK_Shift_L: return KeySHIFT;
	case XK_Shift_R: return KeySHIFT;
	case XK_Control_L: return KeyCONTROL;
	case XK_Control_R: return KeyCONTROL;
	case XK_Caps_Lock: return KeyCAPITAL;
	case XK_Super_L: return KeyLWIN;
	case XK_Super_R: return KeyRWIN;
	case XK_Mode_switch: return KeyMENU;
	case XK_ISO_Level3_Shift: return KeyMENU;
	case XK_Menu: return KeyAPPS;
	// Japanese keyboards; without these, Zenkaku_Hankaku falls back to its keycode, the apostrophe's
	case XK_Zenkaku_Hankaku: return KeyKANJI;
	case XK_Kanji: return KeyKANJI;
	case XK_Henkan_Mode: return KeyCONVERT;
	case XK_Muhenkan: return KeyNONCONVERT;
	case XK_Hiragana_Katakana: return KeyKANA;
	case XK_Alt_L: return KeyMENU;
	case XK_Alt_R: return KeyMENU;
	case XK_space: return KeySPACE;
	case XK_A:
	case XK_a: return KeyA;
	case XK_B:
	case XK_b: return KeyB;
	case XK_C:
	case XK_c: return KeyC;
	case XK_D:
	case XK_d: return KeyD;
	case XK_E:
	case XK_e: return KeyE;
	case XK_F:
	case XK_f: return KeyF;
	case XK_G:
	case XK_g: return KeyG;
	case XK_H:
	case XK_h: return KeyH;
	case XK_I:
	case XK_i: return KeyI;
	case XK_J:
	case XK_j: return KeyJ;
	case XK_K:
	case XK_k: return KeyK;
	case XK_L:
	case XK_l: return KeyL;
	case XK_M:
	case XK_m: return KeyM;
	case XK_N:
	case XK_n: return KeyN;
	case XK_O:
	case XK_o: return KeyO;
	case XK_P:
	case XK_p: return KeyP;
	case XK_Q:
	case XK_q: return KeyQ;
	case XK_R:
	case XK_r: return KeyR;
	case XK_S:
	case XK_s: return KeyS;
	case XK_T:
	case XK_t: return KeyT;
	case XK_U:
	case XK_u: return KeyU;
	case XK_V:
	case XK_v: return KeyV;
	case XK_W:
	case XK_w: return KeyW;
	case XK_X:
	case XK_x: return KeyX;
	case XK_Y:
	case XK_y: return KeyY;
	case XK_Z:
	case XK_z: return KeyZ;
	default:
	// other keys don't need to be processed here
	break;
	}
	return KeyNOKEY;
}

// the keys of the character block of the keyboard besides the letters and the digits, in the order they are
// given out to the keys whose position belongs to another key
static const TKey s_PunctuationKeys[] = { KeySEMICOLON, KeyEQUALS, KeyCOMMA, KeyDASH, KeyPERIOD, KeySLASH, KeyTILDE,
	KeyLBRACKET, KeyBACKSLASH, KeyRBRACKET, KeyAPOSTROPHE, KeyPARAGRAPH, KeyOEM_102 };

// whether a key is one of the character keys: a letter, a digit or punctuation. Which TKey each of these keys
// has depends on the keyboard layout, and each has one TKey whatever the modifiers.
static bool isCharacterKey(TKey key)
{
	if ((key >= KeyA && key <= KeyZ) || (key >= Key0 && key <= Key9))
		return true;

	for (uint i = 0; i < sizeof(s_PunctuationKeys) / sizeof(s_PunctuationKeys[0]); ++i)
	{
		if (s_PunctuationKeys[i] == key)
			return true;
	}

	return false;
}

// the key named after a punctuation character, for a key that cannot have the key of its position
static TKey getPunctuationKeyFromKeySym(KeySym keysym)
{
	switch (keysym)
	{
	case XK_semicolon: return KeySEMICOLON;
	case XK_equal: return KeyEQUALS;
	case XK_comma: return KeyCOMMA;
	case XK_minus: return KeyDASH;
	case XK_period: return KeyPERIOD;
	case XK_slash: return KeySLASH;
	case XK_grave: return KeyTILDE;
	case XK_bracketleft: return KeyLBRACKET;
	case XK_backslash: return KeyBACKSLASH;
	case XK_bracketright: return KeyRBRACKET;
	case XK_apostrophe: return KeyAPOSTROPHE;
	default: break;
	}
	return KeyNOKEY;
}

// The character key of each keycode, for one group (layout) of the keyboard.
// A letter key is the key of its letter. The other keys are the key of their position (getKeyFromKeycode),
// which for a letter position means that the layout has no key for that letter (Cyrillic, Greek). A key whose
// position belongs to another key gets a key of its own: without that, on AZERTY the comma was KeyM like the
// m key, and on BEPO seven keys shared their key with a letter.
const std::vector<TKey> &CUnixEventEmitter::getKeycodeKeys(uint group)
{
	std::vector<TKey> &keys = m_KeycodeKeys[group & 3];
	if (!keys.empty())
		return keys;

	keys.resize(256, KeyNOKEY);

	int minKeycode = 8, maxKeycode = 255;
	XDisplayKeycodes(_dpy, &minKeycode, &maxKeycode);
	if (minKeycode < 0)
		minKeycode = 0;
	if (maxKeycode > 255)
		maxKeycode = 255;

	// the keys that are given, and those among them that a letter key has
	std::vector<bool> taken(KeyCount, false);
	std::vector<bool> letter(KeyCount, false);
	std::vector<KeySym> keysyms(256, NoSymbol);

	// the letters
	for (int keycode = minKeycode; keycode <= maxKeycode; ++keycode)
	{
		KeySym keysym = XkbKeycodeToKeysym(_dpy, (KeyCode)keycode, group & 3, 0);

		// a key with fewer groups than the keyboard has
		if (keysym == NoSymbol && (group & 3) != 0)
			keysym = XkbKeycodeToKeysym(_dpy, (KeyCode)keycode, 0, 0);

		keysyms[keycode] = keysym;

		TKey key = getKeyFromKeySym(keysym);
		if (key >= KeyA && key <= KeyZ)
		{
			keys[keycode] = key;
			taken[key] = true;
			letter[key] = true;
		}
	}

	// the keys of their position
	std::vector<int> displaced;
	for (int keycode = minKeycode; keycode <= maxKeycode; ++keycode)
	{
		if (keys[keycode] != KeyNOKEY || keysyms[keycode] == NoSymbol)
			continue;

		TKey key = getKeyFromKeycode(keycode);
		if (key == KeyNOKEY)
			continue;

		if (!isCharacterKey(key))
		{
			// the decimal key of the keypad: nothing else is at its position
			keys[keycode] = key;
		}
		else if (getKeyFromKeySym(keysyms[keycode]) != KeyNOKEY)
		{
			// no character key in this layout (Zenkaku_Hankaku): its keysym names it, its position stays free
		}
		else if (!letter[key])
		{
			// two positions have the same key where a keyboard has only one of them (LSGT and AB11)
			keys[keycode] = key;
			taken[key] = true;
		}
		else
		{
			displaced.push_back(keycode);
		}
	}

	// the keys whose position is taken: the key named after their character when it is free
	for (uint i = 0; i < displaced.size(); ++i)
	{
		TKey key = getPunctuationKeyFromKeySym(keysyms[displaced[i]]);
		if (key != KeyNOKEY && !taken[key])
		{
			keys[displaced[i]] = key;
			taken[key] = true;
		}
	}

	// then the first punctuation key that is free
	for (uint i = 0; i < displaced.size(); ++i)
	{
		int keycode = displaced[i];

		for (uint j = 0; j < sizeof(s_PunctuationKeys) / sizeof(s_PunctuationKeys[0]) && keys[keycode] == KeyNOKEY; ++j)
		{
			if (!taken[s_PunctuationKeys[j]])
			{
				keys[keycode] = s_PunctuationKeys[j];
				taken[s_PunctuationKeys[j]] = true;
			}
		}

		const char *keysymName = XKeysymToString(keysyms[keycode]);
		if (keys[keycode] == KeyNOKEY)
		{
			// more character keys than there are keys for them: shared, as all of these were before
			keys[keycode] = getKeyFromKeycode(keycode);
			nlwarning("3D: No key is left for X keycode %d (%s) of keyboard group %u, it shares %s with another key", keycode, keysymName ? keysymName : "", group & 3, CEventKey::getStringFromKey(keys[keycode]).c_str());
		}
		else
		{
			nldebug("3D: X keycode %d (%s) of keyboard group %u is %s, as %s is another key of this layout", keycode, keysymName ? keysymName : "", group & 3, CEventKey::getStringFromKey(keys[keycode]).c_str(), CEventKey::getStringFromKey(getKeyFromKeycode(keycode)).c_str());
		}
	}

	return keys;
}

// the key of a key event
TKey CUnixEventEmitter::getKeyFromKeyEvent(uint keycode, KeySym keysym, uint state)
{
	// the keys that are no characters follow their keysym, which follows the modifiers: the keypad with Num Lock,
	// Shift+Tab
	TKey key = getKeyFromKeySym(keysym);
	if (key != KeyNOKEY && !isCharacterKey(key))
		return key;

	// a character key is the same key at all its levels, the one the layout of the group gives its keycode
	if (keycode < 256)
	{
		TKey layoutKey = getKeycodeKeys(XkbGroupForCoreState(state))[keycode];
		if (layoutKey != KeyNOKEY)
			return layoutKey;
	}

	// a letter from a key that has none at its first level
	if (key != KeyNOKEY)
		return key;

	// the keys that give an unknown keysym with modifiers (Ctrl+Pause gives Break): by the keysym of their first level
	return getKeyFromKeySym(XkbKeycodeToKeysym(_dpy, keycode, 0, 0));
}

// check if the next pressed key is the same
static bool keyRepeat(Display *display, XEvent *event)
{
	XEvent peekevent;

	if (XPending(display))
	{
		XPeekEvent(display, &peekevent);

		if ((peekevent.type == KeyPress) &&
			(peekevent.xkey.keycode == event->xkey.keycode) &&
			((peekevent.xkey.time-event->xkey.time) < 2))
			return true;
	}

	return false;
}

bool CUnixEventEmitter::processMessage (XEvent &event, CEventServer *server)
{
	if (!server)
		server=&_InternalServer;

	XWindowAttributes xwa;
	XGetWindowAttributes (_dpy, _win, &xwa);

	switch (event.type)
	{
	case ButtonPress:
	{
		//nlinfo("%d %d %d", event.xbutton.button, event.xbutton.x, event.xbutton.y);
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;
		TMouseButton button=getMouseButton(event.xbutton.state);
		switch(event.xbutton.button)
		{
		case Button1:
			server->postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(leftButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button2:
			server->postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(middleButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button3:
			server->postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(rightButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button4:
			server->postEvent(new CEventMouseWheel(fX, fY, button, true, this));
			break;
		case Button5:
			server->postEvent(new CEventMouseWheel(fX, fY, button, false, this));
			break;
		}
		break;
	}
	case ButtonRelease:
	{
		//nlinfo("%d %d %d", event.xbutton.button, event.xbutton.x, event.xbutton.y);
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;
		switch(event.xbutton.button)
		{
		case Button1:
			server->postEvent(new CEventMouseUp(fX, fY, leftButton, this));
			break;
		case Button2:
			server->postEvent(new CEventMouseUp(fX, fY, middleButton, this));
			break;
		case Button3:
			server->postEvent(new CEventMouseUp(fX, fY, rightButton, this));
			break;
		}
		break;
	}
	case MotionNotify:
	{
		TMouseButton button=getMouseButton (event.xbutton.state);

		// get the relative mouse position
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;

		// post a normal mouse move event to the event server
		server->postEvent (new CEventMouseMove (fX, fY, button, this));

		break;
	}
	case KeyPress:
	{
		// save keycode because XFilterEvent could set it to 0
		uint keyCode = event.xkey.keycode;
		KeySym k = NoSymbol;
		char Text[256];
		// the text of the key: Text, or longText when an input method commits more than Text holds
		std::vector<char> longText;
		const char *text = Text;
		int c = 0;

		// check if event is filtered
		bool filtered = XFilterEvent(&event, _win);

		// if key event is filtered, we shouldn't use XLookupString to retrieve KeySym
		if (!filtered)
		{
			Status status = XLookupNone;

#ifdef X_HAVE_UTF8_STRING
			if (_ic)
			{
				c = Xutf8LookupString(_ic, &event.xkey, Text, sizeof(Text), &k, &status);

				if (status == XBufferOverflow && c > 0)
				{
					// nothing was written, c is the size the text needs
					longText.resize(c);
					c = Xutf8LookupString(_ic, &event.xkey, &longText[0], (int)longText.size(), &k, &status);
					text = &longText[0];
				}

				// no text with these two, and a text that still does not fit is dropped
				if (status == XLookupKeySym || status == XBufferOverflow)
					c = 0;
			}
#endif

			if (status == XLookupNone)
			{
				c = XLookupString(&event.xkey, Text, sizeof(Text), &k, nullptr);
				text = Text;
			}
		}
		else
		{
			// the keysym the key has with the modifiers held (the keypad follows Num Lock), as a lookup gives it
			unsigned int unusedModifiers = 0;
			if (!XkbLookupKeySym(_dpy, (KeyCode)keyCode, event.xkey.state, &unusedModifiers, &k))
				k = XkbKeycodeToKeysym(_dpy, keyCode, 0, 0);
		}

		// send CEventKeyDown event only if keyCode is defined
		if (keyCode)
		{
			TKey key = getKeyFromKeyEvent(keyCode, k, event.xkey.state);

			// An input method server gives back the key presses it does not use: the same press, with the
			// same time, comes a second time and is not filtered then. Its key down was posted the first time.
			bool givenBack = !filtered && keyCode == m_FilteredKeycode && event.xkey.time == m_FilteredTime;
			m_FilteredKeycode = filtered ? keyCode : 0;
			m_FilteredTime = event.xkey.time;

			if (!givenBack)
			{
				// search for key in map
				std::map<TKey, bool>::const_iterator it = _PressedKeys.find(key);

				// if key is not found or value is false, that's the first time
				bool firstTime = (it == _PressedKeys.end()) || !it->second;

				server->postEvent(new CEventKeyDown(key, getKeyButton(event.xbutton.state), firstTime, this));
				_PressedKeys[key] = true;

				// its release reports the same key, whatever the modifiers are by then
				m_PressedKeycodes[keyCode] = key;
			}

			// don't send a control character when deleting
			if (key == KeyDELETE) c = 0;
		}

		if (c > 0)
		{
#ifdef X_HAVE_UTF8_STRING
			// One event for each character: an input method commits whole words. The text is taken by its
			// length, as it is not terminated and Xlib gives a NUL byte for Ctrl+Space, Ctrl+2 and Ctrl+@, which
			// is posted as the character it is, as before.
			::u32string ucstr = NLMISC::CUtfStringView(std::string(text, (size_t)c)).toUtf32();

			for (size_t i = 0; i < ucstr.size(); ++i)
			{
				CEventChar *charEvent = new CEventChar(ucstr[i], getKeyButton(event.xbutton.state), this);

				// raw if not processed by IME
				charEvent->setRaw(keyCode != 0);

				server->postEvent(charEvent);
			}
#else
			// FIXME: Convert locale to UTF-32
			for (int i = 0; i < c; i++)
			{
				CEventChar *charEvent = new CEventChar ((u32char)(unsigned char)text[i], getKeyButton(event.xbutton.state), this);

				// raw if not processed by IME
				charEvent->setRaw(keyCode != 0);

				server->postEvent (charEvent);
			}
#endif
		}
		break;
	}
	case KeyRelease:
	{
		if (!keyRepeat(_dpy, &event))
		{
			// the key its press reported: looked up again, the keysym follows the modifiers held now,
			// and a Shift key that gives Caps_Lock at its second level would be released as Caps Lock
			TKey key;
			std::map<uint, TKey>::iterator it = m_PressedKeycodes.find(event.xkey.keycode);
			if (it != m_PressedKeycodes.end())
			{
				key = it->second;
				m_PressedKeycodes.erase(it);
			}
			else
			{
				// pressed before the window had the focus: the key without modifiers
				key = getKeyFromKeyEvent(event.xkey.keycode, XkbKeycodeToKeysym(_dpy, event.xkey.keycode, 0, 0), event.xkey.state);
			}

			server->postEvent (new CEventKeyUp (key, getKeyButton(event.xbutton.state), this));
			_PressedKeys[key] = false;
		}
		break;
	}
	case SelectionRequest:
	{
		XEvent respond;
		XSelectionRequestEvent req = event.xselectionrequest;

		respond.xselection.type= SelectionNotify;
		respond.xselection.display= req.display;
		respond.xselection.requestor= req.requestor;
		respond.xselection.selection=req.selection;
		respond.xselection.target= req.target;
		respond.xselection.time = req.time;
		respond.xselection.property = req.property;

		if (req.property == None)
		{
			respond.xselection.property = req.target;
		}
		if (req.target == XA_TARGETS)
		{
			Atom targets[] =
			{
				XA_TARGETS,
				XA_STRING,
				XA_UTF8_STRING
			};

			respond.xselection.property = req.property;

			XChangeProperty(req.display, req.requestor, req.property, XA_ATOM, 32, PropModeReplace, (unsigned char *)targets, 3 /* number of element */);
		}
		else if (req.target == XA_STRING)
		{
			respond.xselection.property = req.property;
			std::string str = _CopiedString; // NLMISC::CUtfStringView(_CopiedString).toAscii(); // FIXME: Convert UTF-8 to local
			XChangeProperty(req.display, req.requestor, req.property, XA_STRING, 8, PropModeReplace, (const unsigned char*)str.c_str(), str.length());
		}
		else if (req.target == XA_UTF8_STRING)
		{
			respond.xselection.property = req.property;
			XChangeProperty(req.display, req.requestor, respond.xselection.property, XA_UTF8_STRING, 8, PropModeReplace, (const unsigned char*)_CopiedString.c_str(), _CopiedString.length());
		}
		else
		{
			// Note: Calling XGetAtomName with arbitrary value crash the client, maybe req.target have been sanitized by X11 server
			respond.xselection.property = None;
		}

		XSendEvent (_dpy, req.requestor, 0, 0, &respond);

		break;
	}
	case SelectionClear:
		_SelectionOwned = false;
		_CopiedString.clear();
		break;
	case SelectionNotify:
	{
		Atom target = event.xselection.target;

		Atom actualType = 0;
		int actualFormat = 0;
		unsigned long nitems = 0, bytesLeft = 0;

		// some applications are sending ATOM and other TARGETS
		if (target == XA_TARGETS || target == XA_ATOM)
		{
			Atom *supportedTargets = nullptr;

			// list NeL selection properties
			if (XGetWindowProperty(_dpy, _win, XA_NEL_SEL, 0, XMaxRequestSize(_dpy), False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesLeft, (unsigned char**)&supportedTargets) != Success)
				return false;

			if (bytesLeft > 0)
			{
				nlwarning("Paste: Supported TARGETS list too long.");
			}

			Atom bestTarget = 0;
			sint bestTargetElect = 0;

			// Elect best type
			for (uint i=0; i < nitems; i++)
			{
				// nlwarning(" - Type=%s (%u)", XGetAtomName(_dpy, supportedTargets[i]), (uint)supportedTargets[i]);
				if (supportedTargets[i] == XA_UTF8_STRING )
				{
					if (bestTargetElect < 2)
					{
						bestTarget = XA_UTF8_STRING;
						bestTargetElect = 2;
					}
				}
				else if (supportedTargets[i] == XA_STRING )
				{
					if (bestTargetElect < 1)
					{
						bestTarget = XA_STRING;
						bestTargetElect = 1;
					}
				}
			}

			XFree(supportedTargets);

			if (!bestTargetElect)
			{
				nlwarning("Paste buffer is not a text buffer.");
				return false;
			}

			// request string conversion
			XConvertSelection(_dpy, XA_CLIPBOARD, bestTarget, XA_NEL_SEL, _win, CurrentTime);
		}
		else if (target == XA_UTF8_STRING || target == XA_STRING)
		{
			uint8 *data = nullptr;

			// get selection
			if (XGetWindowProperty(_dpy, _win, XA_NEL_SEL, 0, XMaxRequestSize(_dpy), False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesLeft, (unsigned char**)&data) != Success)
				return false;

			std::string text = (const char*)data;
			XFree(data);

			// convert buffer to ucstring
			if (target == XA_UTF8_STRING)
			{
				// OK
			}
			else if (target == XA_STRING)
			{
				// FIXME: Convert local to UTF-8
				// text = NLMISC::CUtfStringView(text).toAscii();
			}
			else
			{
				nlwarning("Unknown format %u", (uint)target);
			}

			// sent string event to event server
			server->postEvent (new CEventString (text, this));
		}
		else
		{
			nlwarning("Unknow target %u", (uint)target);
		}

		break;
	}
	case FocusIn:
		// keyboard focus
		m_Focus = true;
#ifdef X_HAVE_UTF8_STRING
		if (_ic) XSetICFocus(_ic);
#endif
		server->postEvent (new CEventSetFocus (true, this));
		// server->postEvent(new CEventActivate(true, this));
		break;
	case FocusOut:
		// keyboard focus
		m_Focus = false;
#ifdef X_HAVE_UTF8_STRING
		if (_ic) XUnsetICFocus(_ic);
#endif
		server->postEvent (new CEventSetFocus (false, this));
		// server->postEvent(new CEventActivate(false, this));
		break;
	case KeymapNotify:
		break;
	case MappingNotify:
		// update keymap
		XRefreshKeyboardMapping((XMappingEvent *)&event);

		// the keys of the layout are read again when the next key needs them
		for (uint i = 0; i < sizeof(m_KeycodeKeys) / sizeof(m_KeycodeKeys[0]); ++i)
			m_KeycodeKeys[i].clear();
		break;
	case DestroyNotify:
		// XIM server has crashed
		createIM();
		break;
	case ClientMessage:
		if ((event.xclient.format == 32) && ((Atom)event.xclient.data.l[0] == XA_WM_DELETE_WINDOW))
		{
			// the window manager asks to close the window (close box, Alt+F4): a request, as WM_CLOSE on
			// Windows. The window is still there and the application decides what to do.
			server->postEvent(new CEventCloseWindow(this));
		}
		break;
	default:
		//	nlinfo("UnknownEvent");
		//	XtDispatchEvent(&event);
		return false;
	}

	return true;
}

bool CUnixEventEmitter::copyTextToClipboard(const std::string &text)
{
	_CopiedString = text;

	// NeL window is the owner of clipboard
	XSetSelectionOwner(_dpy,  XA_CLIPBOARD, _win, CurrentTime);

	// check we are owning the clipboard
	if (XGetSelectionOwner(_dpy, XA_CLIPBOARD) != _win)
	{
		nlwarning("Can't aquire selection");
		return false;
	}

	_SelectionOwned = true;

	return true;
}

bool CUnixEventEmitter::pasteTextFromClipboard(std::string &text)
{
	// check if we own the selection
	if (_SelectionOwned)
	{
		text = _CopiedString;
		return true;
	}

	// check if there is a data in clipboard
	if (XGetSelectionOwner(_dpy, XA_CLIPBOARD) == None)
		return false;

	// request supported methods
	XConvertSelection(_dpy, XA_CLIPBOARD, XA_TARGETS, XA_NEL_SEL, _win, CurrentTime);

	// don't return result now
	return false;
}

} // NLMISC

#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
