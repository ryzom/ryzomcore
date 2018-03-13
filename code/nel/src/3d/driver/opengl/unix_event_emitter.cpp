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

#include "stdopengl.h"
#include "unix_event_emitter.h"

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "nel/misc/debug.h"

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

CUnixEventEmitter::CUnixEventEmitter ():_dpy(NULL), _win(0), _im(NULL), _ic(NULL), _driver(NULL)
{
	_SelectionOwned = false;
}

CUnixEventEmitter::~CUnixEventEmitter()
{
	closeIM();
}

void CUnixEventEmitter::init(Display *dpy, Window win, NL3D::IDriver *driver)
{
	_dpy = dpy;
	_win = win;
	_driver = driver;

	XSelectInput (_dpy, _win, KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask|ExposureMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask);

	// define Atoms used by clipboard
	XA_CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False);
	XA_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
	XA_TARGETS = XInternAtom(dpy, "TARGETS", False);
	XA_NEL_SEL = XInternAtom(dpy, "NeL_SEL", False);

	// define Atom used by delete window
	XA_WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
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

void CUnixEventEmitter::createIM()
{
#ifdef X_HAVE_UTF8_STRING

	_im = XOpenIM(_dpy, NULL, NULL, NULL);

	if (_im == NULL)
	{
		XSetLocaleModifiers("@im=local");
 
		_im = XOpenIM(_dpy, NULL, NULL, NULL);

		if (_im == NULL)
		{
			XSetLocaleModifiers("@im=");

			_im = XOpenIM(_dpy, NULL, NULL, NULL);

			if (_im == NULL)
			{
				nlwarning("XOpenIM failed");
			}
		}
	}
 
 	if (_im)
 	{
 		_ic = XCreateIC(_im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, _win, XNFocusWindow, _win, NULL);

		if (!_ic)
		{
			nlwarning("XCreateIC failed");
		}
	}
#endif
}

void CUnixEventEmitter::closeIM()
{
#ifdef X_HAVE_UTF8_STRING
	if (_ic)
	{
		XDestroyIC(_ic);
		_ic = 0;
	}

	if (_im)
	{
		XCloseIM(_im);
		_im = 0;
	}
#endif
}

void CUnixEventEmitter::submitEvents(CEventServer & server, bool allWindows)
{
	while (XPending(_dpy))
	{
		XEvent	Event;
		XNextEvent(_dpy, &Event);
		if (allWindows || Event.xany.window == _win)
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
	case 0x26: return KeyQ;
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
		KeySym k;
		static char Text[256];
		int c = 0;

		// check if event is filtered
		bool filtered = XFilterEvent(&event, _win);

		// if key event is filtered, we shouldn't use XLookupString to retrieve KeySym
		if (!filtered)
		{
			Status status = XLookupNone;

#ifdef X_HAVE_UTF8_STRING
			if (_ic)
				c = Xutf8LookupString(_ic, &event.xkey, Text, sizeof(Text), &k, &status);
#endif

			if (status == XLookupNone)
				c = XLookupString(&event.xkey, Text, sizeof(Text), &k, NULL);
		}
		else
		{
			k = XkbKeycodeToKeysym(_dpy, keyCode, 0, 0);
		}

		// send CEventKeyDown event only if keyCode is defined
		if (keyCode)
		{
			TKey key = getKeyFromKeySym(k);
			if (key == KeyNOKEY)
				key = getKeyFromKeycode(keyCode);

			// search for key in map
			std::map<TKey, bool>::const_iterator it = _PressedKeys.find(key);

			// if key is not found or value is false, that's the first time
			bool firstTime = (it == _PressedKeys.end()) || !it->second;

			server->postEvent (new CEventKeyDown (key, getKeyButton(event.xbutton.state), firstTime, this));
			_PressedKeys[key] = true;

			// don't send a control character when deleting
			if (key == KeyDELETE) c = 0;
		}

		Text[c] = '\0';

		if (c > 0)
		{
#ifdef X_HAVE_UTF8_STRING
			ucstring ucstr;
			ucstr.fromUtf8(Text);

			CEventChar *charEvent = new CEventChar (ucstr[0], getKeyButton(event.xbutton.state), this);

			// raw if not processed by IME
			charEvent->setRaw(keyCode != 0);

			server->postEvent (charEvent);
#else
			for (int i = 0; i < c; i++)
			{
				CEventChar *charEvent = new CEventChar ((ucchar)(unsigned char)Text[i], getKeyButton(event.xbutton.state), this);

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
			KeySym k;
			// only need to get correct KeySym
			int c = XLookupString(&event.xkey, NULL, 0, &k, NULL);

			TKey key = getKeyFromKeySym(k);
			if(key == KeyNOKEY)
				key = getKeyFromKeycode(event.xkey.keycode);

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
			std::string str = _CopiedString.toString();
			XChangeProperty(req.display, req.requestor, req.property, XA_STRING, 8, PropModeReplace, (const unsigned char*)str.c_str(), str.length());
		}
		else if (req.target == XA_UTF8_STRING)
		{
			respond.xselection.property = req.property;
			std::string str = _CopiedString.toUtf8();
			XChangeProperty(req.display, req.requestor, respond.xselection.property, XA_UTF8_STRING, 8, PropModeReplace, (const unsigned char*)str.c_str(), str.length());
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
			Atom *supportedTargets = NULL;

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
			uint8 *data = NULL;

			// get selection
			if (XGetWindowProperty(_dpy, _win, XA_NEL_SEL, 0, XMaxRequestSize(_dpy), False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesLeft, (unsigned char**)&data) != Success)
				return false;

			ucstring text;
			std::string tmpData = (const char*)data;
			XFree(data);

			// convert buffer to ucstring
			if (target == XA_UTF8_STRING)
			{
				text = ucstring::makeFromUtf8(tmpData);
			}
			else if (target == XA_STRING)
			{
				text = tmpData;
			}
			else
			{
				nlwarning("Unknow format %u", (uint)target);
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
#ifdef X_HAVE_UTF8_STRING
		if (_ic) XSetICFocus(_ic);
#endif
		server->postEvent (new CEventSetFocus (true, this));
		// server->postEvent(new CEventActivate(true, this));
		break;
	case FocusOut:
		// keyboard focus
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
		break;
	case DestroyNotify:
		// XIM server has crashed
		createIM();
		break;
	case ClientMessage:
		if ((event.xclient.format == 32) && ((Atom)event.xclient.data.l[0] == XA_WM_DELETE_WINDOW))
		{
			server->postEvent(new CEventDestroyWindow(this));
		}
		break;
	default:
		//	nlinfo("UnknownEvent");
		//	XtDispatchEvent(&event);
		return false;
	}

	return true;
}

bool CUnixEventEmitter::copyTextToClipboard(const ucstring &text)
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

bool CUnixEventEmitter::pasteTextFromClipboard(ucstring &text)
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
