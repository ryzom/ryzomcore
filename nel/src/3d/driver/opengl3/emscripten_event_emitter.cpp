// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "emscripten_event_emitter.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#include "nel/misc/debug.h"
#include "nel/misc/utf_string_view.h"

namespace NLMISC {

CEmscriptenEventEmitter::CEmscriptenEventEmitter()
    : _MouseButtons(0)
    , _TouchId(-1)
    , _Registered(false)
{
}

CEmscriptenEventEmitter::~CEmscriptenEventEmitter()
{
	release();
}

void CEmscriptenEventEmitter::init(const char *canvasSelector)
{
	release();

	_CanvasSelector = canvasSelector;

	const char *canvas = _CanvasSelector.c_str();
	emscripten_set_mousedown_callback(canvas, this, 0, mouseCallback);
	emscripten_set_mouseup_callback(canvas, this, 0, mouseCallback);
	emscripten_set_mousemove_callback(canvas, this, 0, mouseCallback);
	emscripten_set_dblclick_callback(canvas, this, 0, mouseCallback);
	emscripten_set_wheel_callback(canvas, this, 0, wheelCallback);
	emscripten_set_touchstart_callback(canvas, this, 0, touchCallback);
	emscripten_set_touchend_callback(canvas, this, 0, touchCallback);
	emscripten_set_touchmove_callback(canvas, this, 0, touchCallback);
	emscripten_set_touchcancel_callback(canvas, this, 0, touchCallback);
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, keyCallback);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, keyCallback);
	emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, focusCallback);
	emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, focusCallback);

	// Suppress the context menu so right-click reaches the application,
	// disable browser touch gestures on the canvas, and capture the pointer
	// during drags so mouseup outside the canvas is still delivered. Also
	// install a document-level paste listener that buffers pasted text for
	// the submitEvents drain (see below) so Ctrl+V/right-click-Paste land
	// as CEventString events without needing an async clipboard read.
	EM_ASM({
		var el = document.querySelector(UTF8ToString($0));
		if (el)
		{
			el.style.touchAction = 'none';
			el.addEventListener('contextmenu', function(ev) { ev.preventDefault(); });
			el.addEventListener('pointerdown', function(ev) {
				try { el.setPointerCapture(ev.pointerId); } catch (e) { }
			});
		}
		if (!window._nlPasteQueue)
		{
			window._nlPasteQueue = [];
			document.addEventListener('paste', function(ev) {
				var cd = ev.clipboardData || window.clipboardData;
				if (!cd) return;
				var s = cd.getData('text/plain') || cd.getData('text');
				if (s) window._nlPasteQueue.push(s);
			});
		}
	}, canvas);

	_Registered = true;
}

void CEmscriptenEventEmitter::release()
{
	if (_Registered)
	{
		const char *canvas = _CanvasSelector.c_str();
		emscripten_set_mousedown_callback(canvas, NULL, 0, NULL);
		emscripten_set_mouseup_callback(canvas, NULL, 0, NULL);
		emscripten_set_mousemove_callback(canvas, NULL, 0, NULL);
		emscripten_set_dblclick_callback(canvas, NULL, 0, NULL);
		emscripten_set_wheel_callback(canvas, NULL, 0, NULL);
		emscripten_set_touchstart_callback(canvas, NULL, 0, NULL);
		emscripten_set_touchend_callback(canvas, NULL, 0, NULL);
		emscripten_set_touchmove_callback(canvas, NULL, 0, NULL);
		emscripten_set_touchcancel_callback(canvas, NULL, 0, NULL);
		emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
		emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
		emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
		emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
		_Registered = false;
	}

	for (uint i = 0; i < _Events.size(); ++i)
		delete _Events[i];
	_Events.clear();

	_MouseButtons = 0;
	_TouchId = -1;
}

void CEmscriptenEventEmitter::submitEvents(CEventServer &server, bool /* allWindows */)
{
	// Drain any pasted text buffered by the document-level paste listener
	// installed in init(). Each entry becomes a CEventString the same way
	// XSelectionNotify feeds one on X11, so CGroupEditBox's existing
	// handleEventString path inserts the text at the cursor.
	while (true)
	{
		char *pasteText = (char *)EM_ASM_PTR({
			if (window._nlPasteQueue && window._nlPasteQueue.length)
			{
				var s = window._nlPasteQueue.shift();
				var len = lengthBytesUTF8(s) + 1;
				var ptr = _malloc(len);
				stringToUTF8(s, ptr, len);
				return ptr;
			}
			return 0;
		});
		if (!pasteText) break;
		server.postEvent(new CEventString(pasteText, this));
		free(pasteText);
	}

	// The HTML5 callbacks fire between frames on the same thread; hand the
	// buffered events over to the server (which takes ownership).
	for (uint i = 0; i < _Events.size(); ++i)
		server.postEvent(_Events[i]);
	_Events.clear();
}

void CEmscriptenEventEmitter::postEvent(CEvent *e)
{
	_Events.push_back(e);
}

bool CEmscriptenEventEmitter::normalizePos(long targetX, long targetY, float &fX, float &fY)
{
	// targetX/targetY are in CSS pixels relative to the canvas element
	double w, h;
	if (emscripten_get_element_css_size(_CanvasSelector.c_str(), &w, &h) != EMSCRIPTEN_RESULT_SUCCESS || w <= 0.0 || h <= 0.0)
	{
		int iw, ih;
		if (emscripten_get_canvas_element_size(_CanvasSelector.c_str(), &iw, &ih) != EMSCRIPTEN_RESULT_SUCCESS || iw <= 0 || ih <= 0)
			return false;
		w = iw;
		h = ih;
	}
	fX = (float)((double)targetX / w);
	fY = 1.0f - (float)((double)targetY / h);
	clamp(fX, 0.f, 1.f);
	clamp(fY, 0.f, 1.f);
	return true;
}

static uint modifierFlags(bool ctrl, bool shift, bool alt)
{
	uint flags = 0;
	if (ctrl) flags |= ctrlButton;
	if (shift) flags |= shiftButton;
	if (alt) flags |= altButton;
	return flags;
}

static TKey domKeyCodeToNelKey(unsigned long keyCode)
{
	switch (keyCode)
	{
	case 59: return KeySEMICOLON; // Firefox ';'
	case 61: return KeyEQUALS; // Firefox '='
	case 173: return KeyDASH; // Firefox '-'
	case 224: return KeyLWIN; // Firefox meta
	default: break;
	}
	// DOM keyCode values match the Windows virtual key codes TKey is based on
	if (keyCode > 0 && keyCode < KeyCount)
		return (TKey)keyCode;
	return (TKey)0;
}

EM_BOOL CEmscriptenEventEmitter::mouseCallback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
	CEmscriptenEventEmitter *emitter = (CEmscriptenEventEmitter *)userData;

	float fX, fY;
	if (!emitter->normalizePos(e->targetX, e->targetY, fX, fY))
		return EM_FALSE;

	uint mods = modifierFlags(e->ctrlKey, e->shiftKey, e->altKey);

	uint button = 0;
	switch (e->button)
	{
	case 0: button = leftButton; break;
	case 1: button = middleButton; break;
	case 2: button = rightButton; break;
	default: break;
	}

	switch (eventType)
	{
	case EMSCRIPTEN_EVENT_MOUSEMOVE:
		emitter->postEvent(new CEventMouseMove(fX, fY, (TMouseButton)(emitter->_MouseButtons | mods), emitter));
		break;
	case EMSCRIPTEN_EVENT_MOUSEDOWN:
		if (!button)
			return EM_FALSE;
		emitter->_MouseButtons |= button;
		emitter->postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(button | mods), emitter));
		break;
	case EMSCRIPTEN_EVENT_MOUSEUP:
		if (!button)
			return EM_FALSE;
		emitter->_MouseButtons &= ~button;
		emitter->postEvent(new CEventMouseUp(fX, fY, (TMouseButton)(button | mods), emitter));
		break;
	case EMSCRIPTEN_EVENT_DBLCLICK:
		if (!button)
			return EM_FALSE;
		emitter->postEvent(new CEventMouseDblClk(fX, fY, (TMouseButton)(button | mods), emitter));
		break;
	default:
		return EM_FALSE;
	}

	return EM_TRUE;
}

EM_BOOL CEmscriptenEventEmitter::wheelCallback(int /* eventType */, const EmscriptenWheelEvent *e, void *userData)
{
	CEmscriptenEventEmitter *emitter = (CEmscriptenEventEmitter *)userData;

	if (e->deltaY == 0.0)
		return EM_FALSE;

	float fX, fY;
	if (!emitter->normalizePos(e->mouse.targetX, e->mouse.targetY, fX, fY))
		return EM_FALSE;

	uint mods = modifierFlags(e->mouse.ctrlKey, e->mouse.shiftKey, e->mouse.altKey);
	emitter->postEvent(new CEventMouseWheel(fX, fY, (TMouseButton)(emitter->_MouseButtons | mods), e->deltaY < 0.0, emitter));

	return EM_TRUE;
}

EM_BOOL CEmscriptenEventEmitter::keyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
	CEmscriptenEventEmitter *emitter = (CEmscriptenEventEmitter *)userData;

	uint mods = modifierFlags(e->ctrlKey, e->shiftKey, e->altKey);
	TKey key = domKeyCodeToNelKey(e->keyCode);

	if (eventType == EMSCRIPTEN_EVENT_KEYDOWN)
	{
		if (key)
			emitter->postEvent(new CEventKeyDown(key, (TKeyButton)mods, !e->repeat, emitter));

		// Synthesize the character event (matching what XLookupString provides
		// on X11: printable characters plus the usual control characters)
		if (!e->ctrlKey && !e->metaKey)
		{
			u32char c = 0;
			CUtfStringView view(e->key);
			CUtfStringView::const_iterator it = view.begin();
			if (it != view.end())
			{
				u32char first = *it;
				++it;
				if (it == view.end())
					c = first; // single printable character
			}
			if (!c)
			{
				if (!strcmp(e->key, "Enter")) c = '\r';
				else if (!strcmp(e->key, "Backspace")) c = '\b';
				else if (!strcmp(e->key, "Tab")) c = '\t';
				else if (!strcmp(e->key, "Escape")) c = 27;
			}
			if (c)
				emitter->postEvent(new CEventChar(c, (TKeyButton)mods, emitter));
		}
	}
	else if (eventType == EMSCRIPTEN_EVENT_KEYUP)
	{
		if (key)
			emitter->postEvent(new CEventKeyUp(key, (TKeyButton)mods, emitter));
	}
	else
	{
		return EM_FALSE;
	}

	// Keep browser shortcuts (Ctrl/Cmd combos) and the function keys
	// (F5 refresh, F12 devtools, ...) working; consume everything else so
	// Tab, Backspace, arrows and Space don't scroll or navigate the page.
	if (e->ctrlKey || e->metaKey)
		return EM_FALSE;
	if (key >= KeyF1 && key <= KeyF24)
		return EM_FALSE;
	return EM_TRUE;
}

EM_BOOL CEmscriptenEventEmitter::touchCallback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
	CEmscriptenEventEmitter *emitter = (CEmscriptenEventEmitter *)userData;

	uint mods = modifierFlags(e->ctrlKey, e->shiftKey, e->altKey);

	// Map the first touch to the left mouse button
	for (int i = 0; i < e->numTouches; ++i)
	{
		const EmscriptenTouchPoint &touch = e->touches[i];
		if (!touch.isChanged)
			continue;

		float fX, fY;
		if (!emitter->normalizePos(touch.targetX, touch.targetY, fX, fY))
			continue;

		switch (eventType)
		{
		case EMSCRIPTEN_EVENT_TOUCHSTART:
			if (emitter->_TouchId < 0)
			{
				emitter->_TouchId = touch.identifier;
				emitter->_MouseButtons |= leftButton;
				emitter->postEvent(new CEventMouseMove(fX, fY, (TMouseButton)mods, emitter));
				emitter->postEvent(new CEventMouseDown(fX, fY, (TMouseButton)(leftButton | mods), emitter));
			}
			break;
		case EMSCRIPTEN_EVENT_TOUCHMOVE:
			if (emitter->_TouchId == touch.identifier)
				emitter->postEvent(new CEventMouseMove(fX, fY, (TMouseButton)(leftButton | mods), emitter));
			break;
		case EMSCRIPTEN_EVENT_TOUCHEND:
		case EMSCRIPTEN_EVENT_TOUCHCANCEL:
			if (emitter->_TouchId == touch.identifier)
			{
				emitter->_TouchId = -1;
				emitter->_MouseButtons &= ~(uint)leftButton;
				emitter->postEvent(new CEventMouseMove(fX, fY, (TMouseButton)(leftButton | mods), emitter));
				emitter->postEvent(new CEventMouseUp(fX, fY, (TMouseButton)(leftButton | mods), emitter));
			}
			break;
		default:
			return EM_FALSE;
		}
	}

	return EM_TRUE;
}

EM_BOOL CEmscriptenEventEmitter::focusCallback(int eventType, const EmscriptenFocusEvent * /* e */, void *userData)
{
	CEmscriptenEventEmitter *emitter = (CEmscriptenEventEmitter *)userData;

	if (eventType == EMSCRIPTEN_EVENT_FOCUS)
	{
		emitter->postEvent(new CEventSetFocus(true, emitter));
	}
	else if (eventType == EMSCRIPTEN_EVENT_BLUR)
	{
		// Release held state so nothing stays stuck while unfocused
		emitter->_MouseButtons = 0;
		emitter->_TouchId = -1;
		emitter->postEvent(new CEventSetFocus(false, emitter));
	}

	return EM_FALSE;
}

} // NLMISC

#endif // __EMSCRIPTEN__

/* End of emscripten_event_emitter.cpp */
