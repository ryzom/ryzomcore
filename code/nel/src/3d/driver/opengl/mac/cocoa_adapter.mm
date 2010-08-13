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
#include "nel/misc/game_device_events.h"
#include "nel/3d/driver.h"

#include "cocoa_event_emitter.h"
#include "cocoa_opengl_view.h"

// Virtual key codes are only defined here. Still do not need to link carbon.
// see: http://lists.apple.com/archives/Cocoa-dev/2009/May/msg01180.html
#include <Carbon/Carbon.h>

#import <Cocoa/Cocoa.h>

namespace NL3D { namespace MAC {

// This cocoa adapter can be used in two environments:
// First: There is no other code which creates the NSApplication object, so 
//   NeL is completely in charge of starting and setting up the application.
//   In this case, the NSAutoreleasePool needed to handle the cocoa style memory 
//   management is created by this code.
// Second: There is already a NSApplication set up. This could be the case if
//   NeL is used for example in a Qt widget. So Qt already created all the 
//   NSApplication infrastructure, so it is not set up by this code again!
//
// Thats why, the g_pool variable (containing a pointer to the NSAutoreleasePool 
// created by this code) can be used to check whether NeL created the 
// NSApplication infrastructure itself or not.
//
// WARNING:
// Currently the NSApplication infrastructure is automatically created with the
// call to createWindow(). So if for example Qt already created NSApplication,
// createWindow() must not be called. Instead, setDisplay() can be provided with
// a window handle (on Mac OS Cocoa Qt this is a NSView*). In this case, this 
// cocoa adapter will skip the NSApplication setup and embed itself into the
// provided view running in the already set up application.

static NSAutoreleasePool* g_pool           = nil;
/*
	TODO move to event emitter class
*/
static bool               g_emulateRawMode = false;
static int                g_bufferSize[2]  = { 0, 0 };

/// setup an apple style application menu (located at the top bar of the screen)
static void setupApplicationMenu()
{
	NSMenu*     menu;
	NSMenuItem* menuItem;
	NSString*   title;
	NSString*   appName;

	// get the applications name from it's process info
	appName = [[NSProcessInfo processInfo] processName];

	// create an empty menu object
	menu    = [[NSMenu alloc] initWithTitle:@""];

	// add the about menu item
	title = [@"About " stringByAppendingString:appName];
	[menu addItemWithTitle:title 
		action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

	// separator
	[menu addItem:[NSMenuItem separatorItem]];

	// add the hide application menu item
	title = [@"Hide " stringByAppendingString:appName];
	[menu addItemWithTitle:title 
		action:@selector(hide:) keyEquivalent:@"h"];

	// add the hide others menu item
	menuItem = [menu addItemWithTitle:@"Hide Others" 
		action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

	// add the show all menu item
	[menu addItemWithTitle:@"Show All" 
		action:@selector(unhideAllApplications:) keyEquivalent:@""];

	// separator
	[menu addItem:[NSMenuItem separatorItem]];

	/*
		TODO on quit send EventDestroyWindowId
	*/
	// add the quit menu item
	title = [@"Quit " stringByAppendingString:appName];
	[menu addItemWithTitle:title 
		action:@selector(terminate:) keyEquivalent:@"q"];

	// create an empty menu item and put the new menu into it as a subitem
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" 
		action:nil keyEquivalent:@""];
	[menuItem setSubmenu:menu];

	// create a menu for the application
	[NSApp setMainMenu:[[NSMenu alloc] initWithTitle:@""]];

	// attach the new menu to the applications menu
	[[NSApp mainMenu] addItem:menuItem];
}

/// set up the basic NSApplication and NSAutoreleasePool needed for Cocoa
static bool setupNSApplication()
{
	// if the pool was already created, return an error
	if(g_pool)
		return false;

	// create a pool, cocoa code would leak memory otherwise
	g_pool = [[NSAutoreleasePool alloc] init];

	// init the application object
	[NSApplication sharedApplication];
	
	// create the menu in the top screen bar
	setupApplicationMenu();

	// finish the application launching
	[NSApp finishLaunching];
	
	return true;
}

/// setup an open gl view and embed it in the provided parent view
static void setupGLView(NSView* superview)
{
	/*
		TODO use mode.Depth
		TODO NSOpenGLPFAOffScreen
	*/

	// setup opengl settings
	NSOpenGLPixelFormatAttribute att[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize,    24,
		NSOpenGLPFADepthSize,    24,
		NSOpenGLPFAAlphaSize,     8,
		NSOpenGLPFAStencilSize,   8,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFABackingStore,
		0
	};

	// put the settings into a format object
	NSOpenGLPixelFormat* format =
		[[NSOpenGLPixelFormat alloc] initWithAttributes:att];

	if(!format)
		nlerror("cannot create NSOpenGLPixelFormat");

	// create a opengl view with the created format
	NSOpenGLView* view = [[CocoaOpenGLView alloc]
		initWithFrame:NSMakeRect(0, 0, 0, 0) pixelFormat: format];

	if(!view)
		nlerror("cannot create view");

	// make the view automatically fit the super view
	[view setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];

	// put the view into the superview
	[superview addSubview:view];

	[view setFrame: [superview frame]];

	// create a opengl context for the view
	NSOpenGLContext* ctx = [view openGLContext];

	if(!ctx)
		nlerror("cannot create context");

	// free the pixel format object
	[format release];
}

void ctor()
{
}

void dtor()
{
}

bool init(uint windowIcon, emptyProc exitFunc)
{
	return true;
}

bool unInit()
{
	return true;
}

/// setup the basic cocoa app infrastructure and create a window
nlWindow createWindow(const GfxMode& mode)
{
	if(!setupNSApplication())
		nlerror("createWindow must not be called before the old window was "
			"destroyed using destroyWindow()!");

	// describe how the window should look like and behave
	unsigned int styleMask = NSTitledWindowMask | NSClosableWindowMask |
		NSMiniaturizableWindowMask | NSResizableWindowMask;

	// create a cocoa window with the size provided by the mode parameter
	NSWindow* window = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(0, 0, mode.Width, mode.Height)
		styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];

	if(!window)
		nlerror("cannot create window");

	// set the window to non transparent
	[window setOpaque:YES];

	// enable mouse move events, NeL wants them
	[window setAcceptsMouseMovedEvents:YES];

	// there are no overlapping subviews, can use the magical optimization :)
	[window useOptimizedDrawing:YES];

	// put the window to the front and make it the key window
	[window makeKeyAndOrderFront:nil];

	// this is our main window
	[window makeMainWindow];

	NSView* view = [[NSView alloc] init];

	[window setContentView: view];

	return view;
}

/// destroy the given window
bool destroyWindow(nlWindow wnd)
{
	NSView* view = (NSView*)wnd;

	// release the window
	[[view window] release];

	// release the pool
	[g_pool release];
	g_pool = nil;

	return true;
}

/// set the displays settings, if no win is provided, a new one will be created
nlWindow setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable)
{
	NSView* view = (NSView*)wnd;

	if(view == EmptyWindow)
		view = (NSView*)createWindow(mode);

	setupGLView(view);

	[[view window] makeFirstResponder:[[view subviews] lastObject]];

	return view;
}

/// switch between fullscreen and windowed mode
bool setWindowStyle(nlWindow wnd, bool fullscreen)
{
	if(wnd == EmptyWindow)
	{
		nlwarning("cannot set window style on an empty window");
		return false;		
	}
	
	NSView*          superview = (NSView*)wnd;
	NSOpenGLView*    view      = [[superview subviews] lastObject];
	
	// leave fullscreen mode, enter windowed mode
	if(!fullscreen && [superview isInFullScreenMode])
	{
		// disable manual setting of back buffer size, cocoa handles this
		// automatically as soon as the view gets resized
		CGLError error = CGLDisable(
			(CGLContextObj)[[view openGLContext] CGLContextObj],
			kCGLCESurfaceBackingSize);

		if(error != kCGLNoError)
			nlerror("cannot disable kCGLCESurfaceBackingSize (%s)",
				CGLErrorString(error));

		// pull the view back from fullscreen restoring window options
		[superview exitFullScreenModeWithOptions:nil];
		
		// let the gl view receive key events
		[[view window] makeFirstResponder:view];

		// bring the window containing the gl view to the front
		[[view window] makeKeyAndOrderFront:nil];
	}

	// enter fullscreen, leave windowed mode
	else if(fullscreen && ![superview isInFullScreenMode])
	{
		// enable manual back buffer size for mode setting in fullscreen
		CGLError error = CGLEnable(
			(CGLContextObj)[[view openGLContext] CGLContextObj],
			kCGLCESurfaceBackingSize);

		if(error != kCGLNoError)
			nlerror("cannot enable kCGLCESurfaceBackingSize (%s)",
				CGLErrorString(error));

		// put the view in fullscreen mode, hiding the dock but enabling the menubar
		// to pop up if the mouse hits the top screen border.
		// NOTE: withOptions:nil disables <CMD>+<Tab> application switching!
		[superview enterFullScreenMode:[NSScreen mainScreen] withOptions:
			[NSDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithInt:
					NSApplicationPresentationHideDock |
					NSApplicationPresentationAutoHideMenuBar],
				NSFullScreenModeApplicationPresentationOptions, nil]];

		/*
			TODO check if simply using NSView enterFullScreenMode is a good idea.
			 the context can be set to full screen as well, performance differences?
		*/

		// let the gl view receive key events
		[[view window] makeFirstResponder:view];
	}

	return true;
}

/// get the current mode of the screen
void getCurrentScreenMode(nlWindow wnd, GfxMode& mode)
{
	NSView*       superview = (NSView*)wnd;
	NSOpenGLView* view      = [[superview subviews] lastObject];
	
	// the sceen with the menu bar
	NSScreen* screen = [[NSScreen screens] objectAtIndex:0];

	mode.OffScreen = false;
	mode.Frequency = 0;
	mode.Depth     = NSBitsPerPixelFromDepth([screen depth]);

	// in fullscreen mode
	if([superview isInFullScreenMode])
	{
		// return the size of the back buffer (like having switched monitor mode)
		mode.Windowed  = false;
		mode.Width     = (uint16)g_bufferSize[0];
		mode.Height    = (uint16)g_bufferSize[1];
	}
	
	// in windowed mode
	else
	{
		// return the size of the screen with menu bar
		mode.Windowed  = true;
		mode.Width     = (uint16)[screen frame].size.width;
		mode.Height    = (uint16)[screen frame].size.height;
	}
}

/// helper to extract bits per pixel value from screen mode, only 16 or 32 bits
static int bppFromDisplayMode(CGDisplayModeRef mode)
{
	CFStringRef pixelEncoding = CGDisplayModeCopyPixelEncoding(mode);
	
	if(CFStringCompare(pixelEncoding, CFSTR(IO32BitDirectPixels), 
			kCFCompareCaseInsensitive) == kCFCompareEqualTo) 
		return 32;
	
	else if(CFStringCompare(pixelEncoding, CFSTR(IO16BitDirectPixels), 
			kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		return 16;
	
	return 0;
}

/// get the list of available screen modes
bool getModes(std::vector<GfxMode> &modes)
{
	static const CGDisplayCount kMaxDisplays = 16;
	CGDirectDisplayID display[kMaxDisplays];
	CGDisplayCount numDisplays;

	CGDisplayErr err = CGGetActiveDisplayList(kMaxDisplays, display, &numDisplays);
	if(err != CGDisplayNoErr)
	{
		nlwarning("Cannot get displays (%d)", err);
		return false;
	}

	nldebug("3D: %d displays found", (int)numDisplays);

	for (CGDisplayCount i = 0; i < numDisplays; ++i)
	{
		CGDirectDisplayID dspy = display[i];
		CFArrayRef modeList = CGDisplayCopyAllDisplayModes(dspy, NULL);
		
		if (modeList == NULL)
		{
			nlwarning("Display is invalid");
			continue;
		}

		for (CFIndex j = 0; j < CFArrayGetCount(modeList); ++j)
		{
			CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeList, j);
			uint8 bpp = bppFromDisplayMode(mode);
			
			if (bpp >= 16)
			{
				uint16 w = CGDisplayModeGetWidth(mode);
				uint16 h = CGDisplayModeGetHeight(mode);

				// Add this mode
				GfxMode mode;
				mode.Width  = w;
				mode.Height = h;
				mode.Depth  = bpp;

				// Frequency stays at 0 because on mac cocoa, display resolution
				//   is never really changed. if rendering res < display res,
				//   cocoa interpolates and keeps the display at it's original res.
				mode.Frequency = 0;
				modes.push_back (mode);

				nldebug(" Display 0x%x: Mode %dx%d, %d BPP", dspy, w, h, bpp);
			}
		}
	}
	
	return true;
}

/// get the size of the window's content area
void getWindowSize(nlWindow wnd, uint32 &width, uint32 &height)
{
	NSView*          superview = (NSView*)wnd;
	NSOpenGLView*    view      = [[superview subviews] lastObject];
	
	// A cocoa fullscreen view stays at the native resolution of the display.
	// When changing the rendering resolution, the size of the back buffer gets
	// changed, but the view still stays at full resolution. So the scaling of
	// the image from the rendered resolution to the view's resolution is done
	// by cocoa automatically while flushing buffers.
	// That's why, in fullscreen mode, return the resolution of the back buffer,
	// not the one from the window.

	// in fullscreen mode
	if([superview isInFullScreenMode])
	{
		// use the size stored in setWindowSize()
		width = g_bufferSize[0];
		height = g_bufferSize[1];
	}

	// in windowed mode
	else
	{
		// use the size of the view
		NSRect rect = [view frame];
		width = rect.size.width;
		height = rect.size.height;
	}
}

/// set the size of the window's content area
void setWindowSize(nlWindow wnd, uint32 width, uint32 height)
{
	NSView*       superview = (NSView*)wnd;
	NSOpenGLView* view      = [[superview subviews] lastObject];
	
	// for fullscreen mode, adjust the back buffer size to the desired resolution
	if([superview isInFullScreenMode])
	{
		// set the back buffer manually to match the desired rendering resolution
		GLint dim[2]   = { width, height };
		CGLError error = CGLSetParameter(
			(CGLContextObj)[[view openGLContext] CGLContextObj],
			kCGLCPSurfaceBackingSize, dim);

		if(error != kCGLNoError)
			nlerror("cannot set kCGLCPSurfaceBackingSize parameter (%s)",
				CGLErrorString(error));
	}
	else
	{
		// there is only a pool if NeL created the window itself
		// else, the window is not NeL's, so it must not be changed
		if(g_pool)
		{
			NSWindow* window = [view window];

			// get the windows current frame
			NSRect rect = [window frame];

			// convert the desired content size to window size
			rect = [window frameRectForContentRect:
				NSMakeRect(rect.origin.x, rect.origin.y, width, height)];

			// update window dimensions
			[window setFrame:rect display:YES];
		}
	}
	
	// store the size
	g_bufferSize[0] = width;
	g_bufferSize[1] = height;
}

/// get the position of the window
void getWindowPos(nlWindow wnd, sint32 &x, sint32 &y)
{
	NSView*       superview = (NSView*)wnd;
	NSOpenGLView* view      = [[superview subviews] lastObject];
	NSWindow*     window    = [view window];
	
	// for IDriver conformity
	if([superview isInFullScreenMode])
	{
		x = y = 0;
		return;
	}
	
	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [window frame];

	// simply return x
	x = windowRect.origin.x;

	// map y from cocoa to NeL coordinates before returning
	y = screenRect.size.height - windowRect.size.height - windowRect.origin.y;
}

/// set the position of the window
void setWindowPos(nlWindow wnd, sint32 x, sint32 y)
{
	NSView*   superview = (NSView*)wnd;
	NSWindow* window    = [superview window];
	
	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [window frame];

	// convert y from NeL coordinates to cocoa coordinates
	y = screenRect.size.height - y;

	// tell cocoa to move the window
	[window setFrameTopLeftPoint:NSMakePoint(x, y)];
}

/// set the windows title (not the title of the application)
void setWindowTitle(nlWindow wnd, const ucstring& title)
{
	NSView*   superview = (NSView*)wnd;
	NSWindow* window    = [superview window];

	// well... set the title of the window
	[window setTitle:[NSString stringWithUTF8String:title.toUtf8().c_str()]];
}

void showWindow(bool show)
{
	nldebug("show: %d - implement me!", show);
}

/// make the opengl context the current one
bool activate(nlWindow wnd)
{
	NSView*          superview = (NSView*)wnd;
	NSOpenGLView*    view      = [[superview subviews] lastObject];
	NSOpenGLContext* ctx       = [view openGLContext];

	// if our context is not the current one, make it the current
	if([NSOpenGLContext currentContext] != ctx)
		[ctx makeCurrentContext];
	
	return true;
}

/// flush current back buffer to screen
void swapBuffers(nlWindow wnd)
{
	NSView*          superview = (NSView*)wnd;
	NSOpenGLView*    view      = [[superview subviews] lastObject];
	NSOpenGLContext* ctx       = [view openGLContext];
	
	// make cocoa draw buffer contents to the view
	[ctx flushBuffer];
	[view display];
}

void setCapture(bool capture)
{
	// no need to capture
}

/// show or hide the mouse cursor
void showCursor(bool show)
{
	// Mac OS manages a show/hide counter for the cursor, so hiding the cursor
	// twice requires two calls to "show" to make the cursor visible again.
	// Since other platforms seem to not do this, the functionality is masked here
	// by only calling hide if the cursor is visible and only calling show if
	// the cursor was hidden.

	CGDisplayErr error  = kCGErrorSuccess;
	static bool visible = true;

	if(show && !visible)
	{
		error = CGDisplayShowCursor(kCGDirectMainDisplay);
		visible = true;
	}
	else if(!show && visible)
	{
		error = CGDisplayHideCursor(kCGDirectMainDisplay);
		visible = false;
	}

	if(error != kCGErrorSuccess)
		nlerror("cannot show / hide cursor");
}

/// set the mouse position
void setMousePos(nlWindow wnd, float x, float y)
{
	NSView*          superview = (NSView*)wnd;
	NSOpenGLView*    view      = [[superview subviews] lastObject];
	NSWindow*        window    = [view window];
	
	// CG wants absolute coordinates related to first screen's top left

	// get the first screen's (conaints menubar) rect (this is not mainScreen)
	NSRect firstScreenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect;
	if([superview isInFullScreenMode])
		windowRect = [[window screen] frame];
	else
		windowRect = [window frame];

	// get the gl view's rect for height and width
	NSRect viewRect = [view frame];

	// set the cursor position
	CGDisplayErr error = CGDisplayMoveCursorToPoint(
		kCGDirectMainDisplay, CGPointMake(
			windowRect.origin.x + (viewRect.size.width * x), 
			firstScreenRect.size.height - windowRect.origin.y - 
				viewRect.size.height + ((1.0 - y) * viewRect.size.height)));

	if(error != kCGErrorSuccess)
		nlerror("cannot set mouse position");
}

/*
  TODO: this function has to be moved to a more central place to handle key
        mapping on mac x11 as well
*/
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

/*
  TODO: this function has to be moved to a more central place to handle key
        mapping on mac x11 as well
*/
/// convert modifier key state to nel internal modifier key state
static NLMISC::TKeyButton modifierFlagsToNelKeyButton(unsigned int modifierFlags)
{
	unsigned int buttons = 0;
	if (modifierFlags & NSControlKeyMask)   buttons |= NLMISC::ctrlKeyButton;
	if (modifierFlags & NSShiftKeyMask)     buttons |= NLMISC::shiftKeyButton;
	if (modifierFlags & NSAlternateKeyMask) buttons |= NLMISC::altKeyButton;
	return (NLMISC::TKeyButton)buttons;
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
			it always returns false, even for π, é, ...
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

/// switch between raw mode emulation, see IEventEmitter::emulateMouseRawMode()
void emulateMouseRawMode(bool enable)
{
	g_emulateRawMode = enable;
}

/// submit events provided by the application to an event server 
void submitEvents(NLMISC::CEventServer& server,
	bool allWindows, NLMISC::CCocoaEventEmitter* eventEmitter)
{
	// if there is a pool, NeL needs to clean it up
	// otherwise, other code must have created it (for example Qt)
	if(g_pool)
	{
		// cocoa style memory cleanup
		[g_pool release];
		g_pool = [[NSAutoreleasePool alloc] init];
	}
	
	// break if there was no event to handle
	/* TODO maximum number of events processed in one update? */
	while(true)
	{
		// get the next event to handle
		NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
			untilDate:nil /*[NSDate distantFuture]*/
			inMode:NSDefaultRunLoopMode dequeue:YES];

		// stop, if there was no event
		if(!event)
			break;

		NSView* glView   = [[[[event window] contentView] subviews] lastObject];
		NSRect  viewRect = [glView frame];

		// TODO this code assumes, that the view fills the window
		// convert the mouse position to NeL style (relative)
		float mouseX = event.locationInWindow.x / (float)viewRect.size.width;
		float mouseY = event.locationInWindow.y / (float)viewRect.size.height;

		// if the mouse event was placed outside the view, don't tell NeL :)
		if((mouseX < 0.0 || mouseX > 1.0 || mouseY < 0.0 || mouseY > 1.0) && 
				event.type != NSKeyDown && event.type != NSKeyUp)
		{
			[NSApp sendEvent:event];
			continue;
		}

		// convert the modifiers for nel to pass them with the events
		NLMISC::TKeyButton modifiers = 
			modifierFlagsToNelKeyButton([event modifierFlags]);

		switch(event.type)
		{
		case NSLeftMouseDown:
		{
			server.postEvent(new NLMISC::CEventMouseDown(
				mouseX, mouseY, 
				(NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), 
				eventEmitter));
		}
		break;
		case NSLeftMouseUp:
		{
			server.postEvent(new NLMISC::CEventMouseUp(
				mouseX, mouseY, 
				(NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), 
				eventEmitter));
			break;
		}
		case NSRightMouseDown:
		{
			server.postEvent(new NLMISC::CEventMouseDown(
				mouseX, mouseY, 
				(NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), 
				eventEmitter));
			break;
		}
		case NSRightMouseUp:
		{
			server.postEvent(new NLMISC::CEventMouseUp(
				mouseX, mouseY, 
				(NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), 
				eventEmitter));
			break;
		}
		case NSMouseMoved:
		{
			NLMISC::CEvent* nelEvent;

			// when emulating raw mode, send the delta in a CGDMouseMove event
			if(g_emulateRawMode)
				nelEvent = new NLMISC::CGDMouseMove(
					eventEmitter, NULL /* no mouse device */, event.deltaX, -event.deltaY);

			// normally send position in a CEventMouseMove
			else
				nelEvent = new NLMISC::CEventMouseMove(
					mouseX, mouseY, (NLMISC::TMouseButton)modifiers, eventEmitter);

			server.postEvent(nelEvent);
			break;
		}
		case NSLeftMouseDragged:
		{
			NLMISC::CEvent* nelEvent;

			// when emulating raw mode, send the delta in a CGDMouseMove event
			if(g_emulateRawMode)
				nelEvent = new NLMISC::CGDMouseMove(
					eventEmitter, NULL /* no mouse device */, event.deltaX, -event.deltaY);

			// normally send position in a CEventMouseMove
			else
				nelEvent = new NLMISC::CEventMouseMove(mouseX, mouseY,
					(NLMISC::TMouseButton)(NLMISC::leftButton | modifiers), 
					eventEmitter);

			server.postEvent(nelEvent);
			break;
		}
		case NSRightMouseDragged:
		{
			NLMISC::CEvent* nelEvent;

			// when emulating raw mode, send the delta in a CGDMouseMove event
			if(g_emulateRawMode)
				nelEvent = new NLMISC::CGDMouseMove(
					eventEmitter, NULL /* no mouse device */, event.deltaX, -event.deltaY);

			// normally send position in a CEventMouseMove
			else
				nelEvent = new NLMISC::CEventMouseMove(mouseX, mouseY,
					(NLMISC::TMouseButton)(NLMISC::rightButton | modifiers), 
					eventEmitter);

			server.postEvent(nelEvent);
			break;
		}
		case NSMouseEntered:break;
		case NSMouseExited:break;
		case NSKeyDown:
		{
			// push the key press event to the event server
			server.postEvent(new NLMISC::CEventKeyDown(
				virtualKeycodeToNelKey([event keyCode]),
				modifierFlagsToNelKeyButton([event modifierFlags]),
				[event isARepeat] == NO,
				eventEmitter));

			// if this was a text event
			if(isTextKeyEvent(event))
			{
				ucstring ucstr;

				// get the string associated with the key press event
				ucstr.fromUtf8([[event characters] UTF8String]);

				// push the text event to event server as well
				server.postEvent(new NLMISC::CEventChar(
					ucstr[0],
					NLMISC::noKeyButton,
					eventEmitter));
			}
			break;
		}
		case NSKeyUp:
		{
			// push the key release event to the event server
			server.postEvent(new NLMISC::CEventKeyUp(
				virtualKeycodeToNelKey([event keyCode]),
				modifierFlagsToNelKeyButton([event modifierFlags]),
				eventEmitter));
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
				server.postEvent(new NLMISC::CEventMouseWheel(
					mouseX, mouseY, (NLMISC::TMouseButton)modifiers, 
					(event.deltaY > 0), eventEmitter));

			break;
		}
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
		{
			nlwarning("Unknown event type. dropping.");
			// NSLog(@"%@", event);
			break;
		}
		}

		[NSApp sendEvent:event];
	}
}

}}
