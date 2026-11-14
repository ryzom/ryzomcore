// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2011  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#include "../stdopengl.h"
#include "cocoa_event_emitter.h"

namespace NLMISC
{

/// map from virtual key code to nel internal key code
static NLMISC::TKey virtualKeycodeToNelKey(unsigned short keycode)
{
	switch(keycode)
	{
		case kVK_ANSI_0:               return NLMISC::Key0;
		case kVK_ANSI_1:               return NLMISC::Key1;
		case kVK_ANSI_2:               return NLMISC::Key2;
		case kVK_ANSI_3:               return NLMISC::Key3;
		case kVK_ANSI_4:               return NLMISC::Key4;
		case kVK_ANSI_5:               return NLMISC::Key5;
		case kVK_ANSI_6:               return NLMISC::Key6;
		case kVK_ANSI_7:               return NLMISC::Key7;
		case kVK_ANSI_8:               return NLMISC::Key8;
		case kVK_ANSI_9:               return NLMISC::Key9;
		case kVK_ANSI_A:               return NLMISC::KeyA;
		case kVK_ANSI_B:               return NLMISC::KeyB;
		case kVK_ANSI_C:               return NLMISC::KeyC;
		case kVK_ANSI_D:               return NLMISC::KeyD;
		case kVK_ANSI_E:               return NLMISC::KeyE;
		case kVK_ANSI_F:               return NLMISC::KeyF;
		case kVK_ANSI_G:               return NLMISC::KeyG;
		case kVK_ANSI_H:               return NLMISC::KeyH;
		case kVK_ANSI_I:               return NLMISC::KeyI;
		case kVK_ANSI_J:               return NLMISC::KeyJ;
		case kVK_ANSI_K:               return NLMISC::KeyK;
		case kVK_ANSI_L:               return NLMISC::KeyL;
		case kVK_ANSI_M:               return NLMISC::KeyM;
		case kVK_ANSI_N:               return NLMISC::KeyN;
		case kVK_ANSI_O:               return NLMISC::KeyO;
		case kVK_ANSI_P:               return NLMISC::KeyP;
		case kVK_ANSI_Q:               return NLMISC::KeyQ;
		case kVK_ANSI_R:               return NLMISC::KeyR;
		case kVK_ANSI_S:               return NLMISC::KeyS;
		case kVK_ANSI_T:               return NLMISC::KeyT;
		case kVK_ANSI_U:               return NLMISC::KeyU;
		case kVK_ANSI_V:               return NLMISC::KeyV;
		case kVK_ANSI_W:               return NLMISC::KeyW;
		case kVK_ANSI_X:               return NLMISC::KeyX;
		case kVK_ANSI_Y:               return NLMISC::KeyY;
		case kVK_ANSI_Z:               return NLMISC::KeyZ;
		case kVK_ANSI_Equal:           return NLMISC::KeyEQUALS;
		case kVK_ANSI_Minus:           return NLMISC::KeySUBTRACT;
		case kVK_ANSI_RightBracket:    return NLMISC::KeyRBRACKET;
		case kVK_ANSI_LeftBracket:     return NLMISC::KeyLBRACKET;
		case kVK_ANSI_Quote:           return NLMISC::KeyAPOSTROPHE;
		case kVK_ANSI_Grave:           return NLMISC::KeyPARAGRAPH;
		case kVK_ANSI_Slash:           return NLMISC::KeySLASH;
		case kVK_ANSI_Backslash:       return NLMISC::KeyBACKSLASH;
		case kVK_ANSI_Comma:           return NLMISC::KeyCOMMA;
		case kVK_ANSI_Period:          return NLMISC::KeyPERIOD;
		case kVK_ANSI_Semicolon:       return NLMISC::KeySEMICOLON;
		case kVK_ANSI_KeypadDecimal:   return NLMISC::KeyDECIMAL;
		case kVK_ANSI_KeypadMultiply:  return NLMISC::KeyMULTIPLY;
		case kVK_ANSI_KeypadPlus:      return NLMISC::KeyADD;
		case kVK_ANSI_KeypadClear:     return NLMISC::KeyDELETE;
		case kVK_ANSI_KeypadDivide:    return NLMISC::KeyDIVIDE;
		case kVK_ANSI_KeypadEnter:     return NLMISC::KeyRETURN;
		case kVK_ANSI_KeypadMinus:     return NLMISC::KeySUBTRACT;
		case kVK_ANSI_KeypadEquals:    return NLMISC::KeySEPARATOR;
		case kVK_ANSI_Keypad0:         return NLMISC::KeyNUMPAD0;
		case kVK_ANSI_Keypad1:         return NLMISC::KeyNUMPAD1;
		case kVK_ANSI_Keypad2:         return NLMISC::KeyNUMPAD2;
		case kVK_ANSI_Keypad3:         return NLMISC::KeyNUMPAD3;
		case kVK_ANSI_Keypad4:         return NLMISC::KeyNUMPAD4;
		case kVK_ANSI_Keypad5:         return NLMISC::KeyNUMPAD5;
		case kVK_ANSI_Keypad6:         return NLMISC::KeyNUMPAD6;
		case kVK_ANSI_Keypad7:         return NLMISC::KeyNUMPAD7;
		case kVK_ANSI_Keypad8:         return NLMISC::KeyNUMPAD8;
		case kVK_ANSI_Keypad9:         return NLMISC::KeyNUMPAD9;
		case kVK_Return:               return NLMISC::KeyRETURN;
		case kVK_Tab:                  return NLMISC::KeyTAB;
		case kVK_Space:                return NLMISC::KeySPACE;
		case kVK_Delete:               return NLMISC::KeyBACK;
		case kVK_ForwardDelete:        return NLMISC::KeyDELETE;
		case kVK_Escape:               return NLMISC::KeyESCAPE;
		case kVK_Shift:                return NLMISC::KeySHIFT;
		case kVK_RightShift:           return NLMISC::KeyRSHIFT;
		case kVK_CapsLock:             return NLMISC::KeyCAPITAL;
		case kVK_Control:              return NLMISC::KeyCONTROL;
		case kVK_RightControl:         return NLMISC::KeyRCONTROL;
		case kVK_F1:                   return NLMISC::KeyF1;
		case kVK_F2:                   return NLMISC::KeyF2;
		case kVK_F3:                   return NLMISC::KeyF3;
		case kVK_F4:                   return NLMISC::KeyF4;
		case kVK_F5:                   return NLMISC::KeyF5;
		case kVK_F6:                   return NLMISC::KeyF6;
		case kVK_F7:                   return NLMISC::KeyF7;
		case kVK_F8:                   return NLMISC::KeyF8;
		case kVK_F9:                   return NLMISC::KeyF9;
		case kVK_F11:                  return NLMISC::KeyF11;
		case kVK_F13:                  return NLMISC::KeyF13;
		case kVK_F16:                  return NLMISC::KeyF16;
		case kVK_F14:                  return NLMISC::KeyF14;
		case kVK_F10:                  return NLMISC::KeyF10;
		case kVK_F12:                  return NLMISC::KeyF12;
		case kVK_F15:                  return NLMISC::KeyF15;
		case kVK_F17:                  return NLMISC::KeyF17;
		case kVK_F18:                  return NLMISC::KeyF18;
		case kVK_F19:                  return NLMISC::KeyF19;
		case kVK_F20:                  return NLMISC::KeyF20;
		case kVK_Home:                 return NLMISC::KeyHOME;
		case kVK_End:                  return NLMISC::KeyEND;
		case kVK_PageUp:               return NLMISC::KeyPRIOR;
		case kVK_PageDown:             return NLMISC::KeyNEXT;
		case kVK_LeftArrow:            return NLMISC::KeyLEFT;
		case kVK_RightArrow:           return NLMISC::KeyRIGHT;
		case kVK_DownArrow:            return NLMISC::KeyDOWN;
		case kVK_UpArrow:              return NLMISC::KeyUP;
		case kVK_Command:break;
		case kVK_Option:break;
		case kVK_RightOption:break;
		case kVK_Function:break;
		case kVK_VolumeUp:break;
		case kVK_VolumeDown:break;
		case kVK_Mute:break;
		case kVK_Help:break;
		case kVK_ISO_Section:break;
		case kVK_JIS_Yen:break;
		case kVK_JIS_Underscore:break;
		case kVK_JIS_KeypadComma:break;
		case kVK_JIS_Eisu:break;
		case kVK_JIS_Kana:break;
		default:break;
	}
	return NLMISC::KeyNOKEY;
}

/// the punctuation keys, in the order they are given out to the keys whose position belongs to another key
static const NLMISC::TKey s_PunctuationKeys[] = { NLMISC::KeySEMICOLON, NLMISC::KeyEQUALS, NLMISC::KeyCOMMA,
	NLMISC::KeyDASH, NLMISC::KeyPERIOD, NLMISC::KeySLASH, NLMISC::KeyTILDE, NLMISC::KeyLBRACKET,
	NLMISC::KeyBACKSLASH, NLMISC::KeyRBRACKET, NLMISC::KeyAPOSTROPHE, NLMISC::KeyPARAGRAPH, NLMISC::KeyOEM_102 };

/// the key named after a punctuation character, for a key that cannot have the key of its position
static NLMISC::TKey punctuationCharacterToNelKey(UniChar character)
{
	switch (character)
	{
		case ';':  return NLMISC::KeySEMICOLON;
		case '=':  return NLMISC::KeyEQUALS;
		case ',':  return NLMISC::KeyCOMMA;
		case '-':  return NLMISC::KeyDASH;
		case '.':  return NLMISC::KeyPERIOD;
		case '/':  return NLMISC::KeySLASH;
		case '`':  return NLMISC::KeyTILDE;
		case '[':  return NLMISC::KeyLBRACKET;
		case '\\': return NLMISC::KeyBACKSLASH;
		case ']':  return NLMISC::KeyRBRACKET;
		case '\'': return NLMISC::KeyAPOSTROPHE;
		default:break;
	}
	return NLMISC::KeyNOKEY;
}

/// Reads the key of each virtual key code from the current keyboard layout.
/// A letter key is the key of its letter (the virtual key codes are positions on a US keyboard, so on AZERTY
/// the key printed A has the code of Q). The other keys keep the key of their position, unless a letter key of
/// the layout has that key: such a key gets the key named after its character when it is free, else the first
/// punctuation key that is free. Without a layout every key has the key of its position.
void CCocoaEventEmitter::readKeyboardLayout()
{
	const uint keycodeCount = 128;

	m_LayoutKeys.clear();
	m_LayoutKeys.resize(keycodeCount, NLMISC::KeyNOKEY);

	// the character each key gives without modifiers, 0 when it gives none
	std::vector<UniChar> characters(keycodeCount, 0);

	// the keys that are given, and those among them that a letter key has
	std::vector<bool> taken(NLMISC::KeyCount, false);
	std::vector<bool> letter(NLMISC::KeyCount, false);

	TISInputSourceRef source = TISCopyCurrentKeyboardLayoutInputSource();
	const UCKeyboardLayout *layout = NULL;
	if (source)
	{
		// not every input source has this data, and the data belongs to the source
		CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);
		if (layoutData)
			layout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);
	}

	// the letters
	for (uint keycode = 0; keycode < keycodeCount; ++keycode)
	{
		if (layout)
		{
			UInt32 deadKeyState = 0;
			UniChar text[4];
			UniCharCount length = 0;
			OSStatus status = UCKeyTranslate(layout, (UInt16)keycode, kUCKeyActionDisplay, 0, LMGetKbdType(),
				kUCKeyTranslateNoDeadKeysMask, &deadKeyState, sizeof(text) / sizeof(text[0]), &length, text);
			if (status == noErr && length > 0)
				characters[keycode] = text[0];
		}

		NLMISC::TKey key = NLMISC::KeyNOKEY;
		UniChar character = characters[keycode];
		if (character >= 'a' && character <= 'z')
			key = (NLMISC::TKey)(NLMISC::KeyA + (character - 'a'));
		else if (character >= 'A' && character <= 'Z')
			key = (NLMISC::TKey)(NLMISC::KeyA + (character - 'A'));

		if (key != NLMISC::KeyNOKEY)
		{
			m_LayoutKeys[keycode] = key;
			taken[key] = true;
			letter[key] = true;
		}
	}

	if (source)
		CFRelease(source);

	// the keys of their position
	std::vector<uint> displaced;
	for (uint keycode = 0; keycode < keycodeCount; ++keycode)
	{
		if (m_LayoutKeys[keycode] != NLMISC::KeyNOKEY)
			continue;

		NLMISC::TKey key = virtualKeycodeToNelKey((unsigned short)keycode);
		if (key == NLMISC::KeyNOKEY)
			continue;

		if (!letter[key])
		{
			m_LayoutKeys[keycode] = key;
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
		NLMISC::TKey key = punctuationCharacterToNelKey(characters[displaced[i]]);
		if (key != NLMISC::KeyNOKEY && !taken[key])
		{
			m_LayoutKeys[displaced[i]] = key;
			taken[key] = true;
		}
	}

	// then the first punctuation key that is free, and when none is left the key of the position, shared
	for (uint i = 0; i < displaced.size(); ++i)
	{
		uint keycode = displaced[i];

		for (uint j = 0; j < sizeof(s_PunctuationKeys) / sizeof(s_PunctuationKeys[0]) && m_LayoutKeys[keycode] == NLMISC::KeyNOKEY; ++j)
		{
			if (!taken[s_PunctuationKeys[j]])
			{
				m_LayoutKeys[keycode] = s_PunctuationKeys[j];
				taken[s_PunctuationKeys[j]] = true;
			}
		}

		if (m_LayoutKeys[keycode] == NLMISC::KeyNOKEY)
			m_LayoutKeys[keycode] = virtualKeycodeToNelKey((unsigned short)keycode);
	}
}

/// The key of a virtual key code in the current keyboard layout
NLMISC::TKey CCocoaEventEmitter::layoutKeycodeToNelKey(unsigned short keycode)
{
	// the layout the keys were read for: the user can change it at any time
	char identifier[256];
	identifier[0] = '\0';
	TISInputSourceRef source = TISCopyCurrentKeyboardLayoutInputSource();
	if (source)
	{
		CFStringRef sourceId = (CFStringRef)TISGetInputSourceProperty(source, kTISPropertyInputSourceID);
		if (!sourceId || !CFStringGetCString(sourceId, identifier, sizeof(identifier), kCFStringEncodingUTF8))
			identifier[0] = '\0';
		CFRelease(source);
	}

	if (m_LayoutKeys.empty() || m_LayoutId != identifier)
	{
		m_LayoutId = identifier;
		readKeyboardLayout();
	}

	if (keycode < m_LayoutKeys.size())
		return m_LayoutKeys[keycode];

	return virtualKeycodeToNelKey(keycode);
}

bool CCocoaEventEmitter::pasteTextFromClipboard(std::string &text)
{
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	NSArray *classArray = [NSArray arrayWithObject:[NSString class]];
	NSDictionary *options = [NSDictionary dictionary];

	BOOL ok = [pasteboard canReadObjectForClasses:classArray options:options];
	if (ok)
	{
		NSArray *objectsToPaste = [pasteboard readObjectsForClasses:classArray options:options];
		NSString *nstext = [objectsToPaste objectAtIndex:0];
		text = [nstext UTF8String];
		return true;
	}
	return false;
}

bool CCocoaEventEmitter::copyTextToClipboard(const std::string &text)
{
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard clearContents];
	NSArray *copiedObjects = [NSArray arrayWithObject:[NSString stringWithUTF8String:text.c_str()]];
	[pasteboard writeObjects:copiedObjects];
	return true;
}

/// convert modifier key state to nel internal modifier key state
static NLMISC::TKeyButton modifierFlagsToNelKeyButton(unsigned int modifierFlags)
{
	unsigned int buttons = 0;
	if (modifierFlags & NSControlKeyMask)   buttons |= NLMISC::ctrlKeyButton;
	if (modifierFlags & NSShiftKeyMask)     buttons |= NLMISC::shiftKeyButton;
	if (modifierFlags & NSAlternateKeyMask) buttons |= NLMISC::altKeyButton;
	return (NLMISC::TKeyButton)buttons;
}

/// the button of a mouse drag or release, noButton for the other events
static NLMISC::TMouseButton draggedOrReleasedButton(NSEventType type)
{
	switch (type)
	{
	case NSLeftMouseDragged:
	case NSLeftMouseUp:
		return NLMISC::leftButton;
	case NSRightMouseDragged:
	case NSRightMouseUp:
		return NLMISC::rightButton;
	default:
		return NLMISC::noButton;
	}
}

/// whether an event is one of the mouse events NeL is told about
static bool isMouseEvent(NSEventType type)
{
	switch (type)
	{
	case NSLeftMouseDown:
	case NSLeftMouseUp:
	case NSRightMouseDown:
	case NSRightMouseUp:
	case NSMouseMoved:
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSScrollWheel:
		return true;
	default:
		return false;
	}
}

/// check whether a given event represents input text
static bool isTextKeyEvent(NSEvent* event)
{
	// if there are no characters provided with this event, it is not a text event
	if([[event characters] length] == 0)
		return false;

	NLMISC::TKey nelKey = virtualKeycodeToNelKey([event keyCode]);

	// ryzom ui wants to have "escape key string" to leave text box
	if(nelKey == NLMISC::KeyESCAPE)
		return true;

	// ryzom ui wants to have "return key string" to submit text box (send chat)
	if(nelKey == NLMISC::KeyRETURN)
		return true;

	// get the character reported by cocoa
	unsigned int character = [[event characters] characterAtIndex:0];

	// printable ascii characters
	if(isprint(character))
		return true;

	/*
		TODO check why iswprint(character) does not solve it.
			it always returns false, even for non-ASCII characters
	*/
	// characters > 127 but not printable
	if( nelKey == NLMISC::KeyF1    || nelKey == NLMISC::KeyF2    ||
			nelKey == NLMISC::KeyF3    || nelKey == NLMISC::KeyF4    ||
			nelKey == NLMISC::KeyF5    || nelKey == NLMISC::KeyF6    ||
			nelKey == NLMISC::KeyF7    || nelKey == NLMISC::KeyF8    ||
			nelKey == NLMISC::KeyF9    || nelKey == NLMISC::KeyF10   ||
			nelKey == NLMISC::KeyF11   || nelKey == NLMISC::KeyF12   ||
			nelKey == NLMISC::KeyF13   || nelKey == NLMISC::KeyF14   ||
			nelKey == NLMISC::KeyF15   || nelKey == NLMISC::KeyF16   ||
			nelKey == NLMISC::KeyF17   || nelKey == NLMISC::KeyF18   ||
			nelKey == NLMISC::KeyF19   || nelKey == NLMISC::KeyF20   ||
			nelKey == NLMISC::KeyUP    || nelKey == NLMISC::KeyDOWN  ||
			nelKey == NLMISC::KeyLEFT  || nelKey == NLMISC::KeyRIGHT ||
			nelKey == NLMISC::KeyHOME  || nelKey == NLMISC::KeyEND   ||
			nelKey == NLMISC::KeyPRIOR || nelKey == NLMISC::KeyNEXT  ||
			nelKey == NLMISC::KeyDELETE)
		return false;

	// all the fancy wide characters
	if(character > 127)
		return true;

	return false;
}

void CCocoaEventEmitter::init(
		NL3D::IDriver* driver, CocoaOpenGLView* glView, bool eventLoop)
{
	_driver    = driver;
	_glView    = glView;
	_eventLoop = eventLoop;
}

bool CCocoaEventEmitter::processMessage(NSEvent* event, CEventServer* server)
{
	if(!server && !_server)
	{
		// nlerror("no server to post events to");
		nldebug("no server to post events to");
		return true;
	}

	if(!server)
		server = _server;

	NSRect  viewRect = [_glView frame];
	NSPoint mousePos = [_glView convertPoint:event.locationInWindow fromView:nil];

	mousePos.x /= (float)viewRect.size.width;
	mousePos.y /= (float)viewRect.size.height;

	// a relative mouse leaves the cursor where it is: each move adds its delta to NeL's position, kept in
	// the view, which all the mouse events report (the drags with the other buttons are not reported,
	// their moves still count)
	if (m_MouseRelative)
	{
		if (event.type == NSMouseMoved || event.type == NSLeftMouseDragged || event.type == NSRightMouseDragged || event.type == NSOtherMouseDragged)
		{
			// deltaY is positive downwards, NeL's y upwards
			m_MouseX += (float)(event.deltaX / viewRect.size.width);
			m_MouseY -= (float)(event.deltaY / viewRect.size.height);
			clamp(m_MouseX, 0.f, 1.f);
			clamp(m_MouseY, 0.f, 1.f);
		}

		mousePos = NSMakePoint(m_MouseX, m_MouseY);
	}

	// if the mouse event was placed outside the view, don't tell NeL :)
	// except the drags and the release of a button pressed in the view: Cocoa sends them to the window
	// of the press wherever the cursor is, and without the release NeL would keep the button held
	if (!m_MouseRelative && (mousePos.x < 0.0 || mousePos.x > 1.0 || mousePos.y < 0.0 || mousePos.y > 1.0) && event.type != NSKeyDown && event.type != NSKeyUp && !(m_ButtonsDown & draggedOrReleasedButton(event.type)))
	{
		return false;
	}

	// where a relative mouse starts from
	if (isMouseEvent(event.type))
	{
		m_MouseX = (float)mousePos.x;
		m_MouseY = (float)mousePos.y;
	}

	// convert the modifiers for nel to pass them with the events
	NLMISC::TKeyButton modifiers = modifierFlagsToNelKeyButton([event modifierFlags]);

	switch(event.type)
	{
	case NSLeftMouseDown:
	{
		m_ButtonsDown |= NLMISC::leftButton;
		server->postEvent(new NLMISC::CEventMouseDown(mousePos.x, mousePos.y, (NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), this));
	}
	break;
	case NSLeftMouseUp:
	{
		m_ButtonsDown &= ~NLMISC::leftButton;
		server->postEvent(new NLMISC::CEventMouseUp(
			mousePos.x, mousePos.y,
			(NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), this));
		break;
	}
	case NSRightMouseDown:
	{
		m_ButtonsDown |= NLMISC::rightButton;
		server->postEvent(new NLMISC::CEventMouseDown(
			mousePos.x, mousePos.y,
			(NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), this));
		break;
	}
	case NSRightMouseUp:
	{
		m_ButtonsDown &= ~NLMISC::rightButton;
		server->postEvent(new NLMISC::CEventMouseUp(
			mousePos.x, mousePos.y,
			(NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), this));
		break;
	}
	case NSMouseMoved:
	{
		NLMISC::CEvent* nelEvent;

		nelEvent = new NLMISC::CEventMouseMove(mousePos.x, mousePos.y, (NLMISC::TMouseButton)modifiers, this);

		server->postEvent(nelEvent);
		break;
	}
	case NSLeftMouseDragged:
	{
		NLMISC::CEvent* nelEvent;

		nelEvent = new NLMISC::CEventMouseMove(mousePos.x, mousePos.y, (NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), this);

		server->postEvent(nelEvent);
		break;
	}
	case NSRightMouseDragged:
	{
		NLMISC::CEvent* nelEvent;

		nelEvent = new NLMISC::CEventMouseMove(mousePos.x, mousePos.y, (NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), this);

		server->postEvent(nelEvent);
		break;
	}
	case NSMouseEntered:break;
	case NSMouseExited:break;
	case NSKeyDown:
	{
		// the key of the current keyboard layout, which the release reports too
		unsigned short keycode = [event keyCode];
		NLMISC::TKey key = layoutKeycodeToNelKey(keycode);
		m_PressedKeycodes[keycode] = key;

		// push the key press event to the event server
		server->postEvent(new NLMISC::CEventKeyDown(
			key,
			modifiers,
			[event isARepeat] == NO, this));

		// if this was a text event
		if(isTextKeyEvent(event))
		{
			ucstring ucstr;

			// get the string associated with the key press event
			ucstr.fromUtf8([[event characters] UTF8String]);

			// push the text event to event server as well
			server->postEvent(new NLMISC::CEventChar(
				ucstr[0], modifiers, this));
		}
		break;
	}
	case NSKeyUp:
	{
		// the key its press reported, also when the keyboard layout changed in between
		unsigned short keycode = [event keyCode];
		NLMISC::TKey key;
		std::map<unsigned short, NLMISC::TKey>::iterator it = m_PressedKeycodes.find(keycode);
		if (it != m_PressedKeycodes.end())
		{
			key = it->second;
			m_PressedKeycodes.erase(it);
		}
		else
		{
			key = layoutKeycodeToNelKey(keycode);
		}

		// push the key release event to the event server
		server->postEvent(new NLMISC::CEventKeyUp(
			key,
			modifiers, this));
		break;
	}
	case NSFlagsChanged:break;
	case NSAppKitDefined:break;
	case NSSystemDefined:break;
	case NSApplicationDefined:break;
	case NSPeriodic:break;
	case NSCursorUpdate:break;
	case NSScrollWheel:
	{
		if(fabs(event.deltaY) > 0.1)
			server->postEvent(new NLMISC::CEventMouseWheel(
				mousePos.x, mousePos.y, (NLMISC::TMouseButton)modifiers,
				(event.deltaY > 0), this));

		break;
	}
	case NSTabletPoint:break;
	case NSTabletProximity:break;
	case NSOtherMouseDown:break;
	case NSOtherMouseUp:break;
	case NSOtherMouseDragged:break;
#ifdef NL_MAC_VERSION_10_6_UP
	case NSEventTypeGesture:break;
	case NSEventTypeMagnify:break;
	case NSEventTypeSwipe:break;
	case NSEventTypeRotate:break;
	case NSEventTypeBeginGesture:break;
	case NSEventTypeEndGesture:break;
#endif
	default:
	{
		nlwarning("Unknown event type. dropping.");
		NSLog(@"%@", event);
		break;
	}
	}

	return true;
}

bool CCocoaEventEmitter::handleQuitRequest()
{
	if(_server)
	{
		// the close box and Cmd+Q ask to close the window, which is still there: a request, as on Windows
		// and X11, and the application decides what to do
		_server->postEvent(new CEventCloseWindow(this));
		return true;
	}

	return false;
}

void CCocoaEventEmitter::postFocusEvent(bool focused)
{
	// a button released while another window has the focus is never seen
	if (!focused)
		m_ButtonsDown = 0;

	if (_server)
		_server->postEvent(new CEventSetFocus(focused, this));
}

typedef bool (*cocoaProc)(NL3D::IDriver*, const void* e);

void CCocoaEventEmitter::submitEvents(CEventServer& server, bool /* allWins */)
{
	// break if there was no event to handle
	// if running embedded in e.g. qt, _eventLoop will be false
	while(_eventLoop)
	{
		// get the next event to handle
		NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
			untilDate:nil /*[NSDate distantFuture]*/
			inMode:NSDefaultRunLoopMode dequeue:YES];

		// stop, if there was no event
		if(!event)
			break;

		// if there is a driver set up, forward event to it's windowProc
		if(_driver)
		{
			cocoaProc proc = (cocoaProc)_driver->getWindowProc();

			if(proc)
				proc(_driver, [event eventRef]);
		}
		// without driver, just process the event
		else
		{
			processMessage(event, &server);
		}

		@try
		{
			// forward the event to the cocoa application
			[NSApp sendEvent:event];
		}
		@catch(NSException *e)
		{
			nlwarning("Exception when sending event: %s", [[e reason] UTF8String]);
		}
	}

	_server = &server;
}

}
