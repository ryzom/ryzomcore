/*
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
*/

#include "cocoa_adapter.h"

#include "nel/misc/events.h"
#include "nel/3d/driver.h"

#include "cocoa_event_emitter.h"
#include "cocoa_opengl_view.h"
#include "cocoa_window.h"

// virtual key codes are only defined here. we still do not need to link carbon
// see: http://lists.apple.com/archives/Cocoa-dev/2009/May/msg01180.html
#include <Carbon/Carbon.h>

#import <Cocoa/Cocoa.h>

namespace NL3D { namespace MAC {

static NSApplication*     g_app    = 0;
static NSAutoreleasePool* g_pool   = 0;
static CocoaWindow*       g_window = 0;
static CocoaOpenGLView*   g_glview = 0;
static NSOpenGLContext*   g_glctx  = 0;

void ctor()
{
	nldebug("mac cpp bridge called");

	// create a pool, cocoa code would leak memory otherwise
	g_pool = [[NSAutoreleasePool alloc] init];

	// init the application object
	g_app	 = [NSApplication sharedApplication];
}

void dtor()
{
	nldebug("mac cpp bridge called");

	// release the pool
	[g_pool release];
}

bool init(uint windowIcon, emptyProc exitFunc)
{
	nldebug("mac cpp bridge called with %u %u", windowIcon, exitFunc);
	return true;
}

bool setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable)
{
	nldebug("mac cpp bridge called with %u %u %u %u", wnd, &mode, show, resizeable);

	// create a window
	/* TODO: NSBackingStoreBuffered ??? */
	g_window = [[CocoaWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1024, 768)
		styleMask:NSTitledWindowMask | NSResizableWindowMask |
		NSClosableWindowMask | NSMiniaturizableWindowMask
		backing:NSBackingStoreBuffered
		defer:NO];

	// setup opengl settings
	NSOpenGLPixelFormatAttribute att[] =
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		0
	};

	// put the settings into a format object
	NSOpenGLPixelFormat* format =
		[[NSOpenGLPixelFormat alloc] initWithAttributes:att];

	// create a opengl view with the created format
	g_glview = [[CocoaOpenGLView alloc]
		initWithFrame:NSMakeRect(0, 0, 1024, 768) pixelFormat: format];

	// create a opengl context for the view
	g_glctx = [g_glview openGLContext];

	// setup some stuff in the window
	[g_window setContentView:g_glview];
	[g_window makeKeyAndOrderFront:nil];
	[g_window setAcceptsMouseMovedEvents:YES];

	// make the views opengl context the currrent one
	[g_glctx makeCurrentContext];

	// tell the application that we are running now
	[g_app finishLaunching];

	// free the pixel format object
	[format release];

	return true;
}

void getWindowSize(uint32 &width, uint32 &height)
{
	NSRect rect = [g_glview bounds];
	width = rect.size.width;
	height = rect.size.height;
}

void getWindowPos(uint32 &x, uint32 &y)
{
	/*
		TODO mac os gives bottom left
	*/

	NSRect rect = [g_window frame];
	x = rect.origin.x;
	y = rect.origin.y;

	nldebug("%d %d", x, y);
}

void setWindowPos(uint32 x, uint32 y)
{
	/*
		TODO mac os gets bottom left
	*/
	[g_window setFrameOrigin:NSMakePoint(x, y)];
}

void setWindowTitle(const ucstring &title)
{
	[g_window setTitle:[NSString stringWithUTF8String:title.toUtf8().c_str()]];
}

void swapBuffers()
{
	[g_glctx flushBuffer];
}

/*
  TODO: this function has to be moved to a more central place to handle key
        mapping on mac x11 as well
*/
NLMISC::TKey virtualKeycodeToNelKey(unsigned short keycode)
{
	switch(keycode)
	{
		case kVK_ANSI_0:               return NLMISC::Key0;
		case kVK_ANSI_1:               return NLMISC::Key1;
		case kVK_ANSI_2:               return NLMISC::Key2;
		case kVK_ANSI_3:               return NLMISC::Key3;
		case kVK_ANSI_4:               return NLMISC::Key4;
		case kVK_ANSI_5:               return NLMISC::Key6;
		case kVK_ANSI_6:               return NLMISC::Key5;
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

/*
  TODO: this function has to be moved to a more central place to handle key
        mapping on mac x11 as well
*/
NLMISC::TKeyButton modifierFlagsToNelKeyButton(unsigned int modifierFlags)
{
	unsigned int buttons = 0;
	if (modifierFlags & NSControlKeyMask)   buttons |= NLMISC::ctrlKeyButton;
	if (modifierFlags & NSShiftKeyMask)     buttons |= NLMISC::shiftKeyButton;
	if (modifierFlags & NSAlternateKeyMask) buttons |= NLMISC::altKeyButton;
	return (NLMISC::TKeyButton)buttons;
}

bool isTextKeyEvent(NSEvent* event)
{
	// if there are no characters provided with this event, is is not a text event
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
			it always returns false, even for π é ...
	*/
	// > 127 but not printable
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

void submitEvents(NLMISC::CEventServer& server,
	bool allWindows, NLMISC::CCocoaEventEmitter* eventEmitter)
{
	// cocoa style memory cleanup
	[g_pool release];
	g_pool = [[NSAutoreleasePool alloc] init];

	// we break if there was no event to handle
	/* TODO maximum? */
	while(true)
	{
		// get the next event to handle
		NSEvent* event = [g_app nextEventMatchingMask:NSAnyEventMask
			untilDate:nil /*[NSDate distantFuture]*/
			inMode:NSDefaultRunLoopMode dequeue:YES];

		// stop, if there was no event
		if(!event)
			break;

		// NSLog(@"%@", event);

		uint32 width, height;
		/* TODO cache? */
		getWindowSize(width, height);

		// get the mouse position in nel style (relative)
		float mouseX = event.locationInWindow.x / (float)width;
		float mouseY = event.locationInWindow.y / (float)height;

		switch(event.type)
		{
		case NSLeftMouseDown:
			server.postEvent(new NLMISC::CEventMouseDown(
				mouseX, mouseY, NLMISC::leftButton /* modifiers */, eventEmitter));
		break;
		case NSLeftMouseUp:
			server.postEvent(new NLMISC::CEventMouseUp(
				mouseX, mouseY, NLMISC::leftButton /* modifiers */, eventEmitter));
		break;
		case NSRightMouseDown:
			server.postEvent(new NLMISC::CEventMouseDown(
				mouseX, mouseY, NLMISC::rightButton /* modifiers */, eventEmitter));
		break;
		case NSRightMouseUp:
			server.postEvent(new NLMISC::CEventMouseUp(
				mouseX, mouseY, NLMISC::rightButton /* modifiers */, eventEmitter));
		break;
		case NSMouseMoved:
			server.postEvent(new NLMISC::CEventMouseMove(
				mouseX, mouseY, (NLMISC::TMouseButton)0 /* modifiers */, eventEmitter));
		break;
		case NSLeftMouseDragged:
			server.postEvent(new NLMISC::CEventMouseMove(
				mouseX, mouseY, NLMISC::leftButton /* modifiers */, eventEmitter));
		break;
		case NSRightMouseDragged:break;
			server.postEvent(new NLMISC::CEventMouseMove(
				mouseX, mouseY, NLMISC::rightButton /* modifiers */, eventEmitter));
		case NSMouseEntered:break;
		case NSMouseExited:break;
		case NSKeyDown:
		 	/*
		 		TODO dead keys
					http://developer.apple.com/mac/library/documentation/Carbon/Reference/
						Unicode_Utilities_Ref/Reference/reference.html#//apple_ref/c/func/
						UCKeyTranslate
		 	*/
			
			// push the key press event to the new event server
			server.postEvent(new NLMISC::CEventKeyDown(
				virtualKeycodeToNelKey([event keyCode]), 
				modifierFlagsToNelKeyButton([event modifierFlags]), 
				[event isARepeat] == NO, 
				eventEmitter));
			
			if(isTextKeyEvent(event)) 
			{
				ucstring ucstr;

				// get the string associated with the key press event
				ucstr.fromUtf8([[event characters] UTF8String]);

				// push to event server
				server.postEvent(new NLMISC::CEventChar(
					ucstr[0], 
					NLMISC::noKeyButton, 
					eventEmitter));
			}
		break;
		case NSKeyUp:
			server.postEvent(new NLMISC::CEventKeyUp(
				virtualKeycodeToNelKey([event keyCode]), 
				modifierFlagsToNelKeyButton([event modifierFlags]), 
				eventEmitter));
		break;
		case NSFlagsChanged:break;
		case NSAppKitDefined:break;
		case NSSystemDefined:break;
		case NSApplicationDefined:break;
		case NSPeriodic:break;
		case NSCursorUpdate:break;
		case NSScrollWheel:break;
		case NSTabletPoint:break;
		case NSTabletProximity:break;
		case NSOtherMouseDown:break;
		case NSOtherMouseUp:break;
		case NSOtherMouseDragged:break;
		case NSEventTypeGesture:break;
		case NSEventTypeMagnify:break;
		case NSEventTypeSwipe:break;
		case NSEventTypeRotate:break;
		case NSEventTypeBeginGesture:break;
		case NSEventTypeEndGesture:break;
		default:
			nlwarning("Unknown event type. dropping.");
			// NSLog(@"%@", event);
		break;
		}

		[g_app sendEvent:event];
		[g_app updateWindows];
	}
}

}}
