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

#ifdef NL_OS_UNIX

#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "nel/misc/debug.h"
#include "unix_event_emitter.h"

namespace NLMISC {

CUnixEventEmitter::CUnixEventEmitter ():_dpy(NULL), _win(0), _PreviousKey(KeyNOKEY)
{
}

void CUnixEventEmitter::init (Display *dpy, Window win)
{
	_dpy = dpy;
	_win = win;
}

void CUnixEventEmitter::submitEvents(CEventServer & server, bool allWindows)
{
	while (XPending(_dpy))
	{
		XEvent	Event;
		XNextEvent(_dpy, &Event);
		if(Event.xany.window==_win)
		{
			// nlinfo("event: %d", Event.type);
			processMessage (Event, server);
		}
	}
}

#ifndef AltMask
# ifdef NL_OS_MAC
#  define AltMask     (8192)
# else
#  define AltMask     (Mod1Mask)
# endif
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

TKey getKey (KeySym keysym)
{
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
	case XK_0: return Key0;
	case XK_1: return Key1;
	case XK_2: return Key2;
	case XK_3: return Key3;
	case XK_4: return Key4;
	case XK_5: return Key5;
	case XK_6: return Key6;
	case XK_7: return Key7;
	case XK_8: return Key8;
	case XK_9: return Key9;
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
	//nldebug ("0x%x %d '%c'", keysym, keysym, keysym);
	break;
	}
	return KeyNOKEY;
}


#define Case(a) case(a): // nlinfo("event: "#a);

void CUnixEventEmitter::processMessage (XEvent &event, CEventServer &server)
{
	switch (event.type)
	{
	Case(ReparentNotify)
	Case(UnmapNotify)
	Case(VisibilityNotify)
		break;
	Case(ButtonPress)
	{
		//nlinfo("%d %d %d", event.xbutton.button, event.xbutton.x, event.xbutton.y);
		XWindowAttributes xwa;
		XGetWindowAttributes (_dpy, _win, &xwa);
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;
		TMouseButton button=getMouseButton(event.xbutton.state);
		switch(event.xbutton.button)
		{
		case Button1:
			server.postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(leftButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button2:
			server.postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(middleButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button3:
			server.postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(rightButton|(button&~(leftButton|middleButton|rightButton))), this));
			break;
		case Button4:
			server.postEvent(new CEventMouseWheel(fX, fY, button, true, this));
			break;
		case Button5:
			server.postEvent(new CEventMouseWheel(fX, fY, button, false, this));
			break;
		}
		break;
	}
	Case(ButtonRelease)
	{
		//nlinfo("%d %d %d", event.xbutton.button, event.xbutton.x, event.xbutton.y);
		XWindowAttributes xwa;
		XGetWindowAttributes (_dpy, _win, &xwa);
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;
		switch(event.xbutton.button)
		{
		case Button1:
			server.postEvent(new CEventMouseUp(fX, fY, leftButton, this));
			break;
		case Button2:
			server.postEvent(new CEventMouseUp(fX, fY, middleButton, this));
			break;
		case Button3:
			server.postEvent(new CEventMouseUp(fX, fY, rightButton, this));
			break;
		}
		break;
	}
	Case(MotionNotify)
	{
		XWindowAttributes xwa;
		XGetWindowAttributes (_dpy, _win, &xwa);
		float fX = (float) event.xbutton.x / (float) xwa.width;
		float fY = 1.0f - (float) event.xbutton.y / (float) xwa.height;
		if ((fX == 0.5f) && (fY == 0.5f)) break;
		TMouseButton button=getMouseButton (event.xbutton.state);
		server.postEvent (new CEventMouseMove (fX, fY, button, this));
		break;
	}
	Case(KeyPress)
	{
		char Text[1024];
		KeySym k;
		int c;
		c = XLookupString(&event.xkey, Text, 1024-1, &k, NULL);

		TKey key = getKey(XKeycodeToKeysym(_dpy, ((XKeyEvent*)&event)->keycode, 0));
		if(key == KeyNOKEY)
			key = getKey(XKeycodeToKeysym(_dpy, ((XKeyEvent*)&event)->keycode, 1));

		server.postEvent (new CEventKeyDown (key, getKeyButton(event.xbutton.state), _PreviousKey != key, this));
 		_PreviousKey = key;

		// don't send a control character when deleting
		if (key == KeyDELETE)
			c = 0;

		Text[c] = '\0';
		if(c>0)
		{
			for (int i = 0; i < c; i++)
			{
				server.postEvent (new CEventChar ((ucchar)(unsigned char)Text[i], noKeyButton, this));
			}
		}
		break;
	}
	Case (KeyRelease)
	{
		char Text[1024];
		KeySym k;
		int c;
		c = XLookupString(&event.xkey, Text, 1024-1, &k, NULL);

		TKey key = getKey(XKeycodeToKeysym(_dpy, ((XKeyEvent*)&event)->keycode, 0));
		if(key == KeyNOKEY)
			key = getKey(XKeycodeToKeysym(_dpy, ((XKeyEvent*)&event)->keycode, 1));

		server.postEvent (new CEventKeyUp (key, getKeyButton(event.xbutton.state), this));
		_PreviousKey = KeyNOKEY;
		break;
	}
	Case(FocusIn)
		return;
	Case(FocusOut)
		return;
	Case(Expose)
		break;
	Case(MappingNotify)
		XRefreshKeyboardMapping((XMappingEvent *)&event);
		break;
	Case(DestroyNotify)
		break;
	Case(ConfigureNotify)
		/* if (event.xconfigure.width==gmaxx && event.xconfigure.height==gmaxy) {
			UpdateGWin();
		} else {
			XResizeWindow(display, gwindow, gmaxx, gmaxy);
		} */
		break;
	default:
		nlinfo("UnknownEvent");
		//    XtDispatchEvent(&event);
		break;
	}
}

} // NLMISC

#endif // NL_OS_UNIX
