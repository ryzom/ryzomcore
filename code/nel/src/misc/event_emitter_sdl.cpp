/*

Copyright (c) 2014, Jan BOON
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdmisc.h"

#include "nel/misc/events.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/event_emitter_sdl.h"
#include "nel/misc/event_server.h"

#include <SDL.h>

namespace NLMISC {

void CEventEmitterSDL::submitEvents(CEventServer &server, bool allWindows)
{
	// ISSUE: SDL2 pumps from all windows, so we cannot use this accross all platforms at the moment...

    SDL_Event event;
	while (SDL_PollEvent(&event))
		processEvent(event, server);
}

static inline TKey getKey(SDL_Keycode keycode)
{
	switch (keycode)
	{
		case SDLK_0: return Key0;
		case SDLK_1: return Key1;
		case SDLK_2: return Key2;
		case SDLK_3: return Key3;
		case SDLK_4: return Key4;
		case SDLK_5: return Key5;
		case SDLK_6: return Key6;
		case SDLK_7: return Key7;
		case SDLK_8: return Key8;
		case SDLK_9: return Key9;
		case SDLK_a: return KeyA;
		case SDLK_b: return KeyB;
		case SDLK_c: return KeyC;
		case SDLK_d: return KeyD;
		case SDLK_e: return KeyE;
		case SDLK_f: return KeyF;
		case SDLK_g: return KeyG;
		case SDLK_h: return KeyH;
		case SDLK_i: return KeyI;
		case SDLK_j: return KeyJ;
		case SDLK_k: return KeyK;
		case SDLK_l: return KeyL;
		case SDLK_m: return KeyM;
		case SDLK_n: return KeyN;
		case SDLK_o: return KeyO;
		case SDLK_p: return KeyP;
		case SDLK_q: return KeyQ;
		case SDLK_r: return KeyR;
		case SDLK_s: return KeyS;
		case SDLK_t: return KeyT;
		case SDLK_u: return KeyU;
		case SDLK_v: return KeyV;
		case SDLK_w: return KeyW;
		case SDLK_x: return KeyX;
		case SDLK_y: return KeyY;
		case SDLK_z: return KeyZ;
		case SDLK_a: return KeyA;
		case SDLK_CANCEL: return KeyCANCEL;
		case SDLK_TAB: return KeyTAB;
		case SDLK_CLEAR: return KeyCLEAR;
		case SDLK_RETURN: return KeyRETURN;
		case SDLK_MENU: return KeyMENU;
		case SDLK_PAUSE: return KeyPAUSE;
		case SDLK_ESCAPE: return KeyESCAPE;
		case SDLK_MODE: return KeyMODECHANGE;
		case SDLK_SPACE: return KeySPACE;
		case SDLK_PRIOR: return KeyPRIOR;
		case SDLK_END: return KeyEND;
		case SDLK_HOME: return KeyHOME;
		case SDLK_LEFT: return KeyLEFT;
		case SDLK_UP: return KeyUP;
		case SDLK_RIGHT: return KeyRIGHT;
		case SDLK_DOWN: return KeyDOWN;
		case SDLK_SELECT: return KeySELECT;
		case SDLK_PRINTSCREEN: return KeyPRINT;
		case SDLK_EXECUTE: return KeyEXECUTE;
		case SDLK_INSERT: return KeyINSERT;
		case SDLK_DELETE: return KeyDELETE;
		case SDLK_HELP: return KeyHELP;
		case SDLK_LGUI: return KeyLWIN;
		case SDLK_RGUI: return KeyRWIN;
		case SDLK_KP_0: return KeyNUMPAD0;
		case SDLK_KP_1: return KeyNUMPAD1;
		case SDLK_KP_2: return KeyNUMPAD2;
		case SDLK_KP_3: return KeyNUMPAD3;
		case SDLK_KP_4: return KeyNUMPAD4;
		case SDLK_KP_5: return KeyNUMPAD5;
		case SDLK_KP_6: return KeyNUMPAD6;
		case SDLK_KP_7: return KeyNUMPAD7;
		case SDLK_KP_8: return KeyNUMPAD8;
		case SDLK_KP_9: return KeyNUMPAD9;
		case SDLK_KP_MULTIPLY: return KeyMULTIPLY;
		case SDLK_KP_PLUS: return KeyADD;
		case SDLK_SEPARATOR: return KeySEPARATOR;
		case SDLK_KP_MINUS: return KeySUBTRACT;
		case SDLK_KP_DECIMAL: return KeyDECIMAL;
		case SDLK_KP_DIVIDE: return KeyDIVIDE;
		case SDLK_F1: return KeyF1;
		case SDLK_F2: return KeyF2;
		case SDLK_F3: return KeyF3;
		case SDLK_F4: return KeyF4;
		case SDLK_F5: return KeyF5;
		case SDLK_F6: return KeyF6;
		case SDLK_F7: return KeyF7;
		case SDLK_F8: return KeyF8;
		case SDLK_F9: return KeyF9;
		case SDLK_F10: return KeyF10;
		case SDLK_F11: return KeyF11;
		case SDLK_F12: return KeyF12;
		case SDLK_F13: return KeyF13;
		case SDLK_F14: return KeyF14;
		case SDLK_F15: return KeyF15;
		case SDLK_F16: return KeyF16;
		case SDLK_F17: return KeyF17;
		case SDLK_F18: return KeyF18;
		case SDLK_F19: return KeyF19;
		case SDLK_F20: return KeyF20;
		case SDLK_F21: return KeyF21;
		case SDLK_F22: return KeyF22;
		case SDLK_F23: return KeyF23;
		case SDLK_F24: return KeyF24;
		case SDLK_NUMLOCKCLEAR: return KeyNUMLOCK;
		case SDLK_LSHIFT: return KeyLSHIFT;
		case SDLK_RSHIFT: return KeyRSHIFT;
		case SDLK_LCTRL: return KeyLCONTROL;
		case SDLK_RCTRL: return KeyRCONTROL;
		case SDLK_SEMICOLON: return KeySEMICOLON;
		case SDLK_EQUALS: return KeyEQUALS;
		case SDLK_COMMA: return KeyCOMMA;
		case SDLK_MINUS: return KeyDASH;
		case SDLK_PERIOD: return KeyPERIOD;
		case SDLK_SLASH: return KeySLASH;
		case SDLK_LEFTBRACKET: return KeyLBRACKET;
		case SDLK_BACKSLASH: return KeyBACKSLASH;
		case SDLK_RIGHTBRACKET: return KeyRBRACKET;
		case SDLK_QUOTE: return KeyAPOSTROPHE;
		case SDLK_CRSL: return KeyCRSEL;
		case SDLK_EXSEL: return KeyEXSEL;
		default: nldebug("SDL2: Key unhandled: %i (%#x)\n", (uint32)keycode, (uint32)keycode); return KeyNOKEY;
	}
}

static inline TKeyButton getKeyButton(SDL_Keymod keymod)
{
	TKeyButton res = 0;
	if (keymod & KMOD_CTRL)
		res |= ctrlKeyButton;
	if (keymod & KMOD_SHIFT)
		res |= shiftKeyButton;
	if (keymod & KMOD_ALT)
		res |= altKeyButton;
	return res;
}

inline void cacheKeyButton(SDL_Keymod keymod)
{
	TMouseButton res = 0;
	if (keymod & KMOD_CTRL)
		res |= ctrlButton;
	if (keymod & KMOD_SHIFT)
		res |= shiftButton;
	if (keymod & KMOD_ALT)
		res |= altButton;
	m_KeyButtonCache = res;
}

inline void getMouseCoords(uint32 windowId, sint32 x, sint32 y, float &fx, float &fy)
{
	SDL_Window *window = SDL_GetWindowFromID(windowId);
	sint32 w, h;
	SDL_GetWindowSize(window, w, h);
	fx = (float)x / (float)w;
	fx = (float)y / (float)h;
	m_MouseXCache = fx;
	m_MouseYCache = fy;
}

static inline TMouseButton getMouseButton(TMouseButton keyButton, uint32 state)
{
	TMouseButton res = 0;
	if (state & SDL_BUTTON_LMASK)
		res |= leftButton;
	if (state & SDL_BUTTON_MMASK)
		res |= middleButton;
	if (state & SDL_BUTTON_RMASK)
		res |= rightButton;
	return keyButton | res;
}

inline void cacheMouseButton(uint32 state)
{
	TMouseButton res = 0;
	if (state & SDL_BUTTON_LMASK)
		res |= leftButton;
	if (state & SDL_BUTTON_MMASK)
		res |= middleButton;
	if (state & SDL_BUTTON_RMASK)
		res |= rightButton;
	m_MouseButtonCache = res;
}

static inline TMouseButton getMouseButton(TMouseButton keyButton, uint8 button)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT: return keyButton | leftButton;
		case SDL_BUTTON_MIDDLE: return keyButton | middleButton;
		case SDL_BUTTON_RIGHT: return keyButton | rightButton;
		default: return 0;
	}
}

void CEventEmitterSDL::processEvent(const SDL_Event &event, CEventServer &server)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
		{
			TKey key = getKey(event.key.keysim.sym);
			cacheKeyButton(event.key.keysim.mod);
			if (key) server.postEvent(new CEventKeyDown(key, getKeyButton(event.key.keysim.mod), !event.key.repeat, this));
			break;
		}
		case SDL_KEYUP:
		{
			TKey key = getKey(event.key.keysim.sym);
			cacheKeyButton(event.key.keysim.mod);
			if (key) server.postEvent(new CEventKeyUp(key, getKeyButton(event.key.keysim.mod), !event.key.repeat, this));
			break;
		}
		case SDL_TEXTEDITING:
		{
			server.postEvent(new CEventTextEditing(event.edit.text, sizeof(event.edit.text), event.edit.start, event.edit.length, this));
			break;
		}
		case SDL_TEXTINPUT:
		{
			server.postEvent(new CEventTextInput(event.text.text, sizeof(event.text.text), this));
			break;
		}
		case SDL_MOUSEMOTION:
		{
			float x, y;
			getMouseCoords(event.motion.windowID, event.motion.x, event.motion.y, f, x);
			cacheMouseButton(event.button.state);
			server.postEvent(new CEventMouseMove(x, y, getMouseButton(m_KeyButtonCache, event.motion.state), this));
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			TMouseButton button = getMouseButton(m_KeyButtonCache, event.button.button);
			if (button)
			{
				cacheMouseButton(event.button.state);
				float x, y;
				getMouseCoords(event.button.windowID, event.button.x, event.button.y, f, x);
				server.postEvent(new CEventMouseDown(x, y, button, this));
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			TMouseButton button = getMouseButton(m_KeyButtonCache, event.button.button);
			if (button)
			{
				cacheMouseButton(event.button.state);
				float x, y;
				getMouseCoords(event.button.windowID, event.button.x, event.button.y, f, x);
				server.postEvent(new CEventMouseUp(x, y, button, this));
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			TMouseButton button = m_KeyButtonCache | m_MouseButtonCache;
			float x = m_MouseXCache;
			float y = m_MouseYCache;
			server.postEvent(new CEventMouseWheel(x, y, button, event.wheel.y >= 0, this));
			break;
		}
	}
}

} // NLMISC
