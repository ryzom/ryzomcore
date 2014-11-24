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
#include "driver_opengl.h"
#include "driver_opengl_extension.h"
#include "driver_opengl_vertex_buffer_hard.h"

// by default, we disable the windows menu keys (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU

#ifdef NL_OS_MAC
#	import "mac/cocoa_window_delegate.h"
#	import "mac/cocoa_application_delegate.h"
#	import <OpenGL/OpenGL.h>
#elif defined (NL_OS_UNIX)
#	ifdef HAVE_XRANDR
#		include <X11/extensions/Xrandr.h>
#	endif // HAVE_XRANDR
#	ifdef HAVE_XRENDER
#		include <X11/extensions/Xrender.h>
#	endif // HAVE_XRENDER
#	include <X11/Xatom.h>
#	define _NET_WM_STATE_REMOVE	0
#	define _NET_WM_STATE_ADD	1
#endif // NL_OS_UNIX

#include "nel/3d/u_driver.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

#ifdef NL_OS_WINDOWS

bool GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_OGL(GlWndProc)

	if (!driver)
		return false;

	if(message == WM_SIZE)
	{
		if (driver->_CurrentMode.Windowed)
		{
			RECT rect;
			GetClientRect (driver->_win, &rect);

			// Setup gl viewport
			driver->_CurrentMode.Width = (uint16)(rect.right-rect.left);
			driver->_CurrentMode.Height = (uint16)(rect.bottom-rect.top);
		}
	}
	else if(message == WM_MOVE)
	{
		if (driver->_CurrentMode.Windowed)
		{
			RECT rect;
			GetWindowRect (hWnd, &rect);
			driver->_WindowX = rect.left;
			driver->_WindowY = rect.top;
		}
	}
	else if (message == WM_ACTIVATE)
	{
		WORD fActive = LOWORD(wParam);
		if (fActive == WA_INACTIVE)
		{
			driver->_WndActive = false;
		}
		else
		{
			driver->_WndActive = true;
		}
	}

	bool trapMessage = false;
	if (driver->_EventEmitter.getNumEmitters() > 0)
	{
		CWinEventEmitter *we = NLMISC::safe_cast<CWinEventEmitter *>(driver->_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		we->setHWnd(hWnd);
		trapMessage = we->processMessage (hWnd, message, wParam, lParam);
	}
	return trapMessage;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_OGL(DriverGL_WndProc);

	// Get the driver pointer..
	CDriverGL *pDriver=(CDriverGL*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
	bool trapMessage = false;
	if (pDriver != NULL)
	{
		trapMessage = GlWndProc (pDriver, hWnd, message, wParam, lParam);
	}

	// we don't want Windows to erase background
	if (message == WM_ERASEBKGND)
	{
		return TRUE;
	}

	if (message == WM_SYSCOMMAND)
	{
		switch (wParam)
		{
#ifdef NL_DISABLE_MENU
			// disable menu (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
			case SC_KEYMENU:
#endif // NL_DISABLE_MENU

			// Screensaver Trying To Start?
			case SC_SCREENSAVE:

			// Monitor Trying To Enter Powersave?
			case SC_MONITORPOWER:

			// Prevent From Happening
			return 0;

			default:
			break;
		}
	}

	// disable menu (default ALT-F4 behavior is disabled)
	if (message == WM_CLOSE)
	{
		if(pDriver && pDriver->ExitFunc)
		{
			pDriver->ExitFunc();
		}
		else
		{
#ifndef NL_DISABLE_MENU
			// if we don't disable menu, alt F4 make a direct exit else we discard the message
			exit(0);
#endif // NL_DISABLE_MENU
		}
		return 0;
	}

	return trapMessage ? 0 : DefWindowProcW(hWnd, message, wParam, lParam);
}

#elif defined (NL_OS_MAC)

bool GlWndProc(CDriverGL *driver, const void* e)
{
	H_AUTO_OGL(GlWndProc)

	if(!driver)
		return false;

	NSEvent* event = [NSEvent eventWithEventRef:e];
	// NSLog(@"NSEvent in GlWndProc %@", event);

	return driver->_EventEmitter.processMessage(event);
}

#elif defined (NL_OS_UNIX)

static Atom XA_WM_STATE = 0;
static Atom XA_WM_STATE_FULLSCREEN = 0;
static Atom XA_WM_ICON = 0;
static Atom XA_WM_WINDOW_TYPE = 0;
static Atom XA_WM_WINDOW_TYPE_NORMAL = 0;
static Atom XA_FRAME_EXTENTS = 0;

sint nelXErrorsHandler(Display *dpy, XErrorEvent *e)
{
	char buf[80];
	XGetErrorText(dpy, e->error_code, buf, sizeof(buf));
	nlwarning("3D: XError: %s", buf);
	return 1;
}

bool GlWndProc(CDriverGL *driver, XEvent &e)
{
	H_AUTO_OGL(GlWndProc)

	if (!driver)
		return false;

	// nlinfo("3D: glop %d %d", e.type, e.xmap.window);

	// disable menu (default ALT-F4 behavior is disabled)
	switch(e.type)
	{
		case DestroyNotify:

		if(driver && driver->ExitFunc)
		{
			driver->ExitFunc();
		}
		else
		{
#ifndef NL_DISABLE_MENU
			// if we don't disable menu, alt F4 make a direct exit else we discard the message
			exit(0);
#endif // NL_DISABLE_MENU
		}
		break;

		case MapNotify:
		driver->_WndActive = true;
		break;

		case UnmapNotify:
		driver->_WndActive = false;
		break;

		case Expose:
//		nlwarning("Expose event");
		break;

		case ConfigureNotify:

		if (driver->_CurrentMode.Windowed && driver->_WndActive)
		{
			// first time setting decoration sizes
			if ((driver->_DecorationWidth == -1) || (driver->_DecorationWidth == 0))
			{
				Atom type_return = 0;
				int format_return = 0;
				unsigned long nitems_return = 0;
				unsigned long bytes_after_return = 0;
				long *data = NULL;

				int status = XGetWindowProperty(driver->_dpy, driver->_win, XA_FRAME_EXTENTS, 0, 4, False, XA_CARDINAL, &type_return, &format_return, &nitems_return, &bytes_after_return, (unsigned char**)&data);

				// succeeded to retrieve decoration size
				if (status == Success && type_return == XA_CARDINAL && format_return == 32 && nitems_return == 4 && data)
				{
					driver->_DecorationWidth = data[0];
					driver->_DecorationHeight = data[2];
				}
				else
				{
					// use difference between current position and previous one (set by application)
					driver->_DecorationWidth = e.xconfigure.x - driver->_WindowX;
					driver->_DecorationHeight = e.xconfigure.y - driver->_WindowY;
				}

				// don't allow negative decoration sizes
				if (driver->_DecorationWidth < 0) driver->_DecorationWidth = 0;
				if (driver->_DecorationHeight < 0) driver->_DecorationHeight = 0;
			}

			driver->_CurrentMode.Width = e.xconfigure.width;
			driver->_CurrentMode.Height = e.xconfigure.height;
			driver->_WindowX = e.xconfigure.x - driver->_DecorationWidth;
			driver->_WindowY = e.xconfigure.y - driver->_DecorationHeight;
		}

		break;

		default:

		// Process the message by the emitter
		return driver->_EventEmitter.processMessage(e);
	}

	return true;
}

#endif // NL_OS_UNIX

// ***************************************************************************
bool CDriverGL::init (uintptr_t windowIcon, emptyProc exitFunc)
{
	H_AUTO_OGL(CDriverGL_init)

	ExitFunc = exitFunc;

	createCursors();

#ifdef NL_OS_WINDOWS
	WNDCLASSW		wc;

	if (!_Registered)
	{
		memset(&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= (HICON)windowIcon;
		wc.hCursor			= _DefaultCursor;
		wc.hbrBackground	= WHITE_BRUSH;
		wc.lpszClassName	= L"NLClass";
		wc.lpszMenuName		= NULL;
		if ( !RegisterClassW(&wc) )
		{
			return false;
		}
		_Registered=1;
	}

	// Backup monitor color parameters
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		_NeedToRestaureGammaRamp = GetDeviceGammaRamp (dc, _GammaRampBackuped) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);
	}
	else
	{
		nlwarning ("(CDriverGL::init): can't create DC");
	}

	// ati specific : try to retrieve driver version
	retrieveATIDriverVersion();
#elif defined(NL_OS_MAC)

	// nothing to do
	nlunreferenced(windowIcon);

#elif defined (NL_OS_UNIX)

	nlunreferenced(windowIcon);

	_dpy = XOpenDisplay(NULL);

	if (_dpy == NULL)
	{
		nlerror ("XOpenDisplay failed on '%s'", getenv("DISPLAY"));
	}
	else
	{
		nldebug("3D: XOpenDisplay on '%s' OK", getenv("DISPLAY"));
	}

	_xrandr_version = 0;

#ifdef HAVE_XRANDR
	_OldSizeID = 0;
	sint xrandr_major, xrandr_minor;
	if (XRRQueryVersion(_dpy, &xrandr_major, &xrandr_minor))
	{
		_xrandr_version = xrandr_major * 100 + xrandr_minor;
		nlinfo("3D: XRandR %d.%d found", xrandr_major, xrandr_minor);
	}
#endif // HAVE_XRANDR

	_xvidmode_version = 0;

#ifdef XF86VIDMODE
	sint event = 0, error = -1, vm_major = 0, vm_minor = 0;
	if (XF86VidModeQueryExtension(_dpy, &event, &error) && XF86VidModeQueryVersion(_dpy, &vm_major, &vm_minor))
	{
		_xvidmode_version = vm_major * 100 + vm_minor;
		nlinfo("3D: XF86VidMode %d.%d found", vm_major, vm_minor);
	}
#endif

	_xrender_version = 0;

#ifdef HAVE_XRENDER
	sint xrender_major, xrender_event, xrender_error;
	if (XQueryExtension(_dpy, "RENDER", &xrender_major, &xrender_event, &xrender_error) &&
		XRenderQueryExtension(_dpy, &xrender_event, &xrender_error))
	{
		sint xrender_minor = 0;
		XRenderQueryVersion(_dpy, &xrender_major, &xrender_minor);
		_xrender_version = xrender_major * 100 + xrender_minor;
		nlinfo("3D: XRender %d.%d found", xrender_major, xrender_minor);
	}
#endif // HAVE_XRENDER

	nldebug("3D: Available X Extensions:");

	if (DebugLog)
	{
		// list all supported extensions
		sint nextensions = 0;
		char **extensions = XListExtensions(_dpy, &nextensions);

		for(sint i = 0; i < nextensions; ++i)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(NLMISC::toString("%s ", extensions[i]).c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}

		DebugLog->displayRaw("\n");

		XFreeExtensionList(extensions);
	}

	// set default X errors handler
	XSetErrorHandler(nelXErrorsHandler);

	// define Atoms
	XA_WM_STATE = XInternAtom(_dpy, "_NET_WM_STATE", False);
	XA_WM_STATE_FULLSCREEN = XInternAtom(_dpy, "_NET_WM_STATE_FULLSCREEN", False);
	XA_WM_ICON = XInternAtom(_dpy, "_NET_WM_ICON", False);
	XA_WM_WINDOW_TYPE = XInternAtom(_dpy, "_NET_WM_WINDOW_TYPE", False);
	XA_WM_WINDOW_TYPE_NORMAL = XInternAtom(_dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
	XA_FRAME_EXTENTS = XInternAtom(_dpy, "_NET_FRAME_EXTENTS", False);

#endif

	return true;
}

// ***************************************************************************
bool CDriverGL::unInit()
{
	H_AUTO_OGL(CDriverGL_unInit)

	if (!_CurrentMode.Windowed)
	{
		restoreScreenMode();
		showCursor(true);

		_CurrentMode.Windowed = true;
	}

#ifdef NL_OS_WINDOWS

#ifndef USE_OPENGLES
	// Off-screen rendering ?
	if (_PBuffer)
	{
		nwglReleasePbufferDCARB(_PBuffer, _hDC);
		nwglDestroyPbufferARB(_PBuffer);
		_PBuffer = NULL;
	}
#endif

	if (_Registered && !UnregisterClassW(L"NLClass", GetModuleHandle(NULL)))
	{
		nlwarning("Can't unregister NLClass");
	}
	_Registered = 0;

	// Restaure monitor color parameters
	if (_NeedToRestaureGammaRamp)
	{
		HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
		if (dc)
		{
			if (!SetDeviceGammaRamp (dc, _GammaRampBackuped))
				nlwarning ("(CDriverGL::release): SetDeviceGammaRamp failed");

			// Release the DC
			ReleaseDC (NULL, dc);
		}
		else
		{
			nlwarning ("(CDriverGL::release): can't create DC");
		}
	}

#elif defined(NL_OS_MAC)

	// nothing to do

#elif defined (NL_OS_UNIX)

	// restore default X errors handler
	XSetErrorHandler(NULL);

	XCloseDisplay(_dpy);
	_dpy = NULL;

#endif // NL_OS_UNIX

	return true;
}

void CDriverGL::setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	if (_win == EmptyWindow)
		return;

#if defined(NL_OS_WINDOWS)

	static HICON winIconBig = NULL;
	static HICON winIconSmall = NULL;

	if (winIconBig)
	{
		DestroyIcon(winIconBig);
		winIconBig = NULL;
	}

	if (winIconSmall)
	{
		DestroyIcon(winIconSmall);
		winIconSmall = NULL;
	}

	sint smallIndex = -1;
	uint smallWidth = GetSystemMetrics(SM_CXSMICON);
	uint smallHeight = GetSystemMetrics(SM_CYSMICON);

	sint bigIndex = -1;
	uint bigWidth = GetSystemMetrics(SM_CXICON);
	uint bigHeight = GetSystemMetrics(SM_CYICON);

	// find icons with the exact size
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 &&	bitmaps[i].getWidth() == smallWidth &&	bitmaps[i].getHeight() == smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() == bigWidth && bitmaps[i].getHeight() == bigHeight)
			bigIndex = i;
	}

	// find icons with taller size (we will resize them)
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 && bitmaps[i].getWidth() >= smallWidth && bitmaps[i].getHeight() >= smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() >= bigWidth && bitmaps[i].getHeight() >= bigHeight)
			bigIndex = i;
	}

	if (smallIndex > -1)
		convertBitmapToIcon(bitmaps[smallIndex], winIconSmall, smallWidth, smallHeight, 32);

	if (bigIndex > -1)
		convertBitmapToIcon(bitmaps[bigIndex], winIconBig, bigWidth, bigHeight, 32);

	if (winIconBig)
	{
		SendMessage(_win, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_win, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconBig);
	}
	else
	{
		SendMessage(_win, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_win, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconSmall);
	}

#elif defined(NL_OS_MAC)

	// nothing to do, on Mac OS X, only windows representing a file have icons

#elif defined(NL_OS_UNIX)

	std::vector<long> icon_data;

	if (!bitmaps.empty())
	{
		// process each bitmap
		for(uint i = 0; i < bitmaps.size(); ++i)
		{
			convertBitmapToIcon(bitmaps[i], icon_data);
		}
	}

	if (!icon_data.empty())
	{
		// change window icon
		XChangeProperty(_dpy, _win, XA_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (const unsigned char *) &icon_data[0], icon_data.size());
	}
	else
	{
		// delete window icon if no bitmap is available
		XDeleteProperty(_dpy, _win, XA_WM_ICON);
	}

#endif // NL_OS_WINDOWS
}

// --------------------------------------------------
bool CDriverGL::setDisplay(nlWindow wnd, const GfxMode &mode, bool show, bool resizeable) throw(EBadDisplay)
{
	H_AUTO_OGL(CDriverGL_setDisplay)

	_win = EmptyWindow;

	_CurrentMode = mode;

	_WindowVisible = false;
	_Resizable = resizeable;
	_DestroyWindow = false;

#ifdef NL_OS_WINDOWS

	// Init pointers
#ifndef USE_OPENGLES
	_PBuffer = NULL;
	_hRC = NULL;
	_hDC = NULL;
#endif

	// Driver caps.
	//=============
	// Retrieve the WGL extensions before init the driver.

	// Offscreen mode ?
	if (_CurrentMode.OffScreen)
	{
#if 0
		if (!createWindow(mode))
			return false;

		// resize the window
		RECT rc;
		SetRect (&rc, 0, 0, width, height);
		AdjustWindowRectEx (&rc, GetWindowStyle (_win), GetMenu (_win) != NULL, GetWindowExStyle (_win));
		SetWindowPos (_win, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

		// Get the
		HDC tempHDC = GetDC(tmpHWND);

		_CurrentMode.Depth = uint8(GetDeviceCaps(tempHDC,BITSPIXEL));

		// ---
		memset(&_pfd,0,sizeof(_pfd));
		_pfd.nSize        = sizeof(_pfd);
		_pfd.nVersion     = 1;
		_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		_pfd.iPixelType   = PFD_TYPE_RGBA;
		_pfd.cColorBits   = _CurrentMode.Depth;

		// Choose best suited Depth Buffer.
		if(_CurrentMode.Depth<=16)
		{
			_pfd.cDepthBits   = 16;
		}
		else
		{
			_pfd.cDepthBits = 24;
			_pfd.cAlphaBits	= 8;
		}
		_pfd.iLayerType	  = PFD_MAIN_PLANE;
		int pf=ChoosePixelFormat(tempHDC,&_pfd);
		if (!pf)
		{
			nlwarning ("CDriverGL::setDisplay: ChoosePixelFormat failed");
			DestroyWindow (tmpHWND);
			return false;
		}
		if ( !SetPixelFormat(tempHDC,pf,&_pfd) )
		{
			nlwarning ("CDriverGL::setDisplay: SetPixelFormat failed");
			DestroyWindow (tmpHWND);
			return false;
		}

		// Create gl context
		HGLRC tempGLRC = wglCreateContext(tempHDC);
		if (tempGLRC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreateContext failed: 0x%x", error);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Make the context current
		if (!wglMakeCurrent(tempHDC,tempGLRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglMakeCurrent failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Register WGL functions
#ifdef USE_OPENGLES
		registerEGlExtensions (_Extensions, tempHDC);
#else
		registerWGlExtensions (_Extensions, tempHDC);
#endif

		HDC hdc = wglGetCurrentDC ();

		if (hdc == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglGetCurrentDC failed: 0x%x", error);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Get ready to query for a suitable pixel format that meets our
		// minimum requirements.
		int iattributes[2*20];
		float fattributes[2*20];
		int niattribs = 0;

		// Attribute arrays must be "0" terminated - for simplicity, first
		// just zero-out the array then fill from left to right.
		for ( int a = 0; a < 2*20; a++ )
		{
			iattributes[a] = 0;
			fattributes[a] = 0;
		}

		// Since we are trying to create a pbuffer, the pixel format we
		// request (and subsequently use) must be "buffer capable".
		iattributes[2*niattribs ] = WGL_DRAW_TO_PBUFFER_ARB;
		iattributes[2*niattribs+1] = true;
		niattribs++;

		// We require a minimum of 24-bit depth.
		iattributes[2*niattribs ] = WGL_DEPTH_BITS_ARB;
		iattributes[2*niattribs+1] = 24;
		niattribs++;

		// We require a minimum of 8-bits for each R, G, B, and A.
		iattributes[2*niattribs ] = WGL_RED_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_GREEN_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_BLUE_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;
		iattributes[2*niattribs ] = WGL_ALPHA_BITS_ARB;
		iattributes[2*niattribs+1] = 8;
		niattribs++;

		// Now obtain a list of pixel formats that meet these minimum
		// requirements.
		int pformat[20];
		unsigned int nformats;
		if ( !nwglChoosePixelFormatARB ( hdc, iattributes, fattributes,
			20, pformat, &nformats ) )
		{
			nlwarning ( "pbuffer creation error: Couldn't find a suitable pixel format." );
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			return false;
		}

		/* After determining a compatible pixel format, the next step is to create a pbuffer of the
			chosen format. Fortunately this step is fairly easy, as you merely select one of the formats
			returned in the list in step #2 and call the function: */
		int iattributes2[1] = {0};
		// int iattributes2[] = {WGL_PBUFFER_LARGEST_ARB, 1, 0};
		_PBuffer = nwglCreatePbufferARB( hdc, pformat[0], width, height, iattributes2 );
		if (_PBuffer == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreatePbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);

			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		/* After creating a pbuffer, you may use this functions to determine the dimensions of the pbuffer actually created. */
		if ( !nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width ) )
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglQueryPbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		if ( !nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height ) )
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglQueryPbufferARB failed: 0x%x", error);
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		_CurrentMode.Width = width;
		_CurrentMode.Height = height;

		/* The next step is to create a device context for the newly created pbuffer. To do this,
			call to the function: */
		_hDC = nwglGetPbufferDCARB( _PBuffer );
		if (_hDC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglGetPbufferDCARB failed: 0x%x", error);
			nwglDestroyPbufferARB( _PBuffer );

			wglDeleteContext (tempGLRC);

			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		/* The final step of pbuffer creation is to create an OpenGL rendering context and
			associate it with the handle for the pbuffer's device context created in step #4. This is done as follows */
		_hRC = wglCreateContext( _hDC );
		if (_hRC == NULL)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglCreateContext failed: 0x%x", error);
			nwglReleasePbufferDCARB( _PBuffer, _hDC );
			nwglDestroyPbufferARB( _PBuffer );
			wglDeleteContext (tempGLRC);
			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}

		// Get the depth
		_CurrentMode.Depth = uint8(GetDeviceCaps (_hDC, BITSPIXEL));

		// Destroy the temp gl context
#ifdef USE_OPENGLES
		if (!eglDestroyContext(_EglDisplay, _EglContext);)
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglDeleteContext failed: 0x%x", error);
		}
#else
		if (!wglDeleteContext (tempGLRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglDeleteContext failed: 0x%x", error);
		}
#endif

		// Destroy the temp windows
		if (!DestroyWindow (tmpHWND))
			nlwarning ("CDriverGL::setDisplay: DestroyWindow failed");

		/* After a pbuffer has been successfully created you can use it for off-screen rendering. To do
			so, you'll first need to bind the pbuffer, or more precisely, make its GL rendering context
			the current context that will interpret all OpenGL commands and state changes. */
		if (!wglMakeCurrent(_hDC,_hRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglMakeCurrent failed: 0x%x", error);

#ifdef USE_OPENGLES
			eglDestroyContext(_EglDisplay, _EglContext);
#else
			wglDeleteContext (_hRC);
			nwglReleasePbufferDCARB( _PBuffer, _hDC );
			nwglDestroyPbufferARB( _PBuffer );
#endif

			DestroyWindow (tmpHWND);
			_PBuffer = NULL;
			_win = EmptyWindow;
			_hRC = NULL;
			_hDC = NULL;
			return false;
		}
#endif
	}
	else
	{
		if (wnd == EmptyWindow)
		{
			if (!createWindow(mode))
				return false;
		}
		else
		{
			_win = wnd;
		}

		// associate OpenGL driver to window
		SetWindowLongPtr(_win, GWLP_USERDATA, (LONG_PTR)this);

		createContext();
	}

	/// release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}

	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter;

	// setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true /*must delete*/); // the main emitter

#elif defined(NL_OS_MAC)

	if (wnd == EmptyWindow)
	{
		if (!createWindow(mode))
			return false;
	}
	else
	{
		_win = wnd;

		/* The NSView* extracted from a QWidget using winId() has bounds set to
		 * (QWidget::x(), QWidget::y(), QWidget::width(), QWidget::height()).
		 * This causes cocoa to draw at an offset of x(), y() leaving an unhandled
		 * border in the NSView. The code below fixes this by translating the
		 * coordinate system of the NSView back to 0,0.
		 * In my opinion this is an error in Qt since QWidget::x/y() are relative to
		 * parent and [NSView bounds.origin] is relative to it's own coordinate
		 * system. This are incompatible notations. Qt should handle the conversion.
		 * Fixes: #1013 Viewport size when embedding NeL Cocoa view in Qt
		 *   (http://dev.ryzom.com/issues/1013)
		 */
		NSView* view = (NSView*)wnd;
		if(view.frame.origin.x != 0 || view.frame.origin.y != 0) {
			[view setBoundsOrigin:view.frame.origin];
		}
	}

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

	// intially set height/width, further updates through CocoaOpenGLView
	_CurrentMode.Height = mode.Height;
	_CurrentMode.Width = mode.Width;

	// create a opengl view with the created format
	_glView = [[CocoaOpenGLView alloc]
		initWithFrame:NSMakeRect(0, 0, mode.Width, mode.Height)
		pixelFormat:format];

	if(!_glView)
		nlerror("cannot create view");

	// tell the view about the driver so the view is able to update "window" size
	[_glView setDriver:this];

	// make the view automatically fit the super view
	[_glView setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];

	// put the open gl view into the dummy view contained in the window
	[containerView() addSubview:_glView];

	// adjust size
	[_glView setFrame: [containerView() frame]];

	// create a opengl context for the view
	_ctx = [_glView openGLContext];

	if(!_ctx)
		nlerror("cannot create context");

	// free the pixel format object
	[format release];

	// let the open gl view handle the input
	[[containerView() window] makeFirstResponder:_glView];

	// prevents scrambled content in the view before first swap
	[_ctx flushBuffer];
	[_glView display];

	_EventEmitter.init(this, _glView, _DestroyWindow);

#elif defined(NL_OS_UNIX)

	static int sAttribList16bpp[] =
	{
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 16,
		GLX_RED_SIZE, 4,
		GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4,
		GLX_ALPHA_SIZE, 4,
		GLX_STENCIL_SIZE, 8,
		None
	};

	static int sAttribList24bpp[] =
	{
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_STENCIL_SIZE, 8,
		None
	};

	// first try 24bpp and if that fails 16bpp
	XVisualInfo *visual_info = glXChooseVisual (_dpy, DefaultScreen(_dpy), sAttribList24bpp);
	if (visual_info == NULL)
		visual_info = glXChooseVisual(_dpy, DefaultScreen(_dpy), sAttribList16bpp);
	if(visual_info == NULL)
	{
		nlerror("glXChooseVisual() failed");
	}

	_visual_info = visual_info;

	_ctx = glXCreateContext (_dpy, visual_info, None, GL_TRUE);
	if (_ctx == NULL)
	{
		nlerror("glXCreateContext() failed");
	}

	if (wnd == EmptyWindow)
	{
		if (!createWindow(mode))
			return false;
	}
	else
	{
		_win = wnd;

		XSetWindowAttributes attr;
		attr.background_pixel = BlackPixel(_dpy, DefaultScreen(_dpy));
		attr.override_redirect = False;

		int attr_flags = CWOverrideRedirect | CWBackPixel;

		XChangeWindowAttributes(_dpy, _win, attr_flags, &attr);
	}

	glXMakeCurrent (_dpy, _win, _ctx);
//	XMapRaised (_dpy, _win);

//	XMapWindow(_dpy, _win);

	_EventEmitter.init (_dpy, _win, this);

//	XEvent event;
//	XIfEvent(dpy, &event, WaitForNotify, (char *)this);

#endif // NL_OS_UNIX

	// setup OpenGL structures
	if (!setupDisplay())
		return false;

	// setup window size and screen mode
	if (!setMode(mode))
		return false;

	if (show || !_CurrentMode.Windowed)
		showWindow(true);

	return true;
}

// --------------------------------------------------
bool CDriverGL::saveScreenMode()
{
	H_AUTO_OGL(CDriverGL_saveScreenMode)

	bool res = true;

#if defined(NL_OS_WINDOWS)

	// don't need to save it because Windows will use default desktop resolution

#elif defined(NL_OS_MAC)

	// no need to store because the screen mode is never really changed

#elif defined(NL_OS_UNIX)

	int screen = DefaultScreen(_dpy);
	res = false;

#ifdef HAVE_XRANDR

	if (!res && _xrandr_version > 0)
	{
		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, RootWindow(_dpy, screen));

		if (screen_config)
		{
			Rotation saved_rotation;
			_OldSizeID = XRRConfigCurrentConfiguration(screen_config, &saved_rotation);
			nlinfo("3D: current XRandR mode %d", _OldSizeID);
			XRRFreeScreenConfigInfo(screen_config);

			res = true;
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#if defined(XF86VIDMODE)

	if (!res && _xvidmode_version > 0)
	{
		// Store old mode in order to restore it when leaving fullscreen
		memset(&_OldScreenMode, 0, sizeof(XF86VidModeModeLine));
		XF86VidModeGetModeLine(_dpy, screen, &_OldDotClock, &_OldScreenMode);
		res = XF86VidModeGetViewPort(_dpy, screen, &_OldX, &_OldY);
	}

#endif // XF86VIDMODE

#endif // NL_OS_WINDOWS

	return res;
}

// --------------------------------------------------
bool CDriverGL::restoreScreenMode()
{
	H_AUTO_OGL(CDriverGL_restoreScreenMode)

	bool res = false;

#if defined(NL_OS_WINDOWS)

	res = (ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL);

#elif defined(NL_OS_MAC)

	// no need to restore because the screen mode was never really changed
	res = true;

#elif defined(NL_OS_UNIX)

	int screen = DefaultScreen(_dpy);

#ifdef HAVE_XRANDR

	if (!res && _xrandr_version > 0)
	{
		Window root = RootWindow(_dpy, screen);

		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, root);

		if (screen_config)
		{
			Rotation saved_rotation;
			SizeID size = XRRConfigCurrentConfiguration(screen_config, &saved_rotation);
			if (XRRSetScreenConfig(_dpy, screen_config, root, _OldSizeID, saved_rotation, CurrentTime) == RRSetConfigSuccess)
			{
				nlinfo("3D: Switching back to XRandR mode %d", _OldSizeID);
				res = true;
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#if defined(XF86VIDMODE)

	if (!res && _xvidmode_version > 0)
	{
		XF86VidModeModeInfo info;
		nlinfo("3D: Switching back to original mode");

		// This is UGLY
		info.dotclock = _OldDotClock;
		info.hdisplay = _OldScreenMode.hdisplay;
		info.hsyncstart = _OldScreenMode.hsyncstart;
		info.hsyncend = _OldScreenMode.hsyncend;
		info.htotal = _OldScreenMode.htotal;
		info.vdisplay = _OldScreenMode.vdisplay;
		info.vsyncstart = _OldScreenMode.vsyncstart;
		info.vsyncend = _OldScreenMode.vsyncend;
		info.vtotal = _OldScreenMode.vtotal;
		info.flags = _OldScreenMode.flags;
		info.privsize = _OldScreenMode.privsize;
		info.c_private = _OldScreenMode.c_private;

		nlinfo("3D: Switching back mode to %dx%d", info.hdisplay, info.vdisplay);
		XF86VidModeSwitchToMode(_dpy, screen, &info);
		nlinfo("3D: Switching back viewport to %d,%d",_OldX, _OldY);
		res = XF86VidModeSetViewPort(_dpy, screen, _OldX, _OldY);
	}

#endif // XF86VIDMODE

#endif // NL_OS_WINDOWS

	return res;
}

// ***************************************************************************

#ifdef XF86VIDMODE
static sint modeInfoToFrequency(XF86VidModeModeInfo *info)
{
	return (info->htotal && info->vtotal) ? (1000 * info->dotclock / (info->htotal * info->vtotal)) : 0;
}
#endif // XF86VIDMODE

// ***************************************************************************

bool CDriverGL::setScreenMode(const GfxMode &mode)
{
	H_AUTO_OGL(CDriverGL_setScreenMode)

	if (mode.Windowed)
	{
		// if fullscreen, switch back to desktop screen mode
		if (!_CurrentMode.Windowed)
			restoreScreenMode();

		return true;
	}

	// save previous screen mode only if switching from windowed to fullscreen
	if (_CurrentMode.Windowed)
		saveScreenMode();

	// if switching exactly to the same screen mode, doesn't change it
	GfxMode previousMode;
	if (getCurrentScreenMode(previousMode)
		&& mode.Width == previousMode.Width
		&& mode.Height == previousMode.Height
		&& mode.Depth == previousMode.Depth
		&& mode.Frequency == previousMode.Frequency)
		return true;

#if defined(NL_OS_WINDOWS)

	DEVMODE devMode;
	memset(&devMode, 0, sizeof(DEVMODE));
	devMode.dmSize        = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;
	devMode.dmFields      = DM_PELSWIDTH | DM_PELSHEIGHT;
	devMode.dmPelsWidth   = mode.Width;
	devMode.dmPelsHeight  = mode.Height;

	if(mode.Depth > 0)
	{
		devMode.dmBitsPerPel  = mode.Depth;
		devMode.dmFields     |= DM_BITSPERPEL;
	}

	if(mode.Frequency > 0)
	{
		devMode.dmDisplayFrequency  = mode.Frequency;
		devMode.dmFields           |= DM_DISPLAYFREQUENCY;
	}

	if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		nlwarning("3D: Fullscreen mode switch failed");
		return false;
	}

#elif defined(NL_OS_MAC)

	// no need to do anything here, on mac os, the screen mode is never changed

#elif defined(NL_OS_UNIX)

	bool found = false;

#ifdef HAVE_XRANDR

	if (!found && _xrandr_version > 0)
	{
		int screen = DefaultScreen(_dpy);
		Window root = RootWindow(_dpy, screen);

		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, root);

		if (screen_config)
		{
			Rotation saved_rotation;
			SizeID cur_size = XRRConfigCurrentConfiguration(screen_config, &saved_rotation);

			sint nsizes;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);
			sint size = -1;

			for (sint i = 0; i < nsizes; ++i)
			{
				if (sizes[i].width == mode.Width && sizes[i].height == mode.Height)
				{
					size = i;
					break;
				}
			}

			if (size > -1)
			{
				if (XRRSetScreenConfig(_dpy, screen_config, root, size, saved_rotation, CurrentTime) == RRSetConfigSuccess)
				{
					nlinfo("3D: Switching to XRandR mode %d: %dx%d", size, sizes[size].width, sizes[size].height);
					found = true;
				}
				else
				{
					nlwarning("3D: XRRSetScreenConfig failed for mode %d: %dx%d", size, sizes[size].width, sizes[size].height);
				}
			}
			else
			{
				nlwarning("3D: No corresponding screen mode");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#if defined(XF86VIDMODE)

	if (!found && _xvidmode_version > 0)
	{
		// Find the requested mode and use it
		XF86VidModeModeInfo **modes;
		int nmodes;
		if (XF86VidModeGetAllModeLines(_dpy, DefaultScreen(_dpy), &nmodes, &modes))
		{
			for (int i = 0; i < nmodes; i++)
			{
				const uint16 freq = modeInfoToFrequency(modes[i]);

				nldebug("3D: Available mode - %dx%d %d Hz", modes[i]->hdisplay, modes[i]->vdisplay, (int)freq);
				if (modes[i]->hdisplay == mode.Width && modes[i]->vdisplay == mode.Height /* && freq == mode.Frequency */)
				{
					if (XF86VidModeSwitchToMode(_dpy, DefaultScreen(_dpy), modes[i]))
					{
						nlinfo("3D: XF86VidMode Switching to mode %dx%d", modes[i]->hdisplay, modes[i]->vdisplay);
						XF86VidModeSetViewPort(_dpy, DefaultScreen(_dpy), 0, 0);
						found = true;
					}
					break;
				}
			}
			XFree(modes);
		}
	}

#endif // XF86VIDMODE

	if (!found)
		return false;

#endif // NL_OS_WINDOWS

	return true;
}

// ***************************************************************************
bool CDriverGL::createWindow(const GfxMode &mode)
{
	H_AUTO_OGL(CDriverGL_createWindow)

	nlWindow window = EmptyWindow;

#ifdef NL_OS_WINDOWS

	// create the OpenGL window
	window = CreateWindowW(L"NLClass", L"NeL Window", WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, mode.Width, mode.Height, HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);

	if (window == EmptyWindow)
	{
		DWORD res = GetLastError();
		nlwarning("CreateWindow failed: %u", res);
		return false;
	}

#elif defined(NL_OS_MAC)

	// describe how the window should look like and behave
	unsigned int styleMask = NSTitledWindowMask | NSClosableWindowMask |
		NSMiniaturizableWindowMask | NSResizableWindowMask;

	// create a cocoa window with the size provided by the mode parameter
	NSWindow* cocoa_window = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(0, 0, mode.Width, mode.Height)
		styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];

	if(!cocoa_window)
	{
		nlerror("cannot create cocoa window");
		return false;
	}

	// create an application delegate
	CocoaApplicationDelegate* appDelegate =
		[[CocoaApplicationDelegate alloc] initWithDriver:this];

	// set the application delegate, this will handle window/app close events
	[NSApp setDelegate:appDelegate];

	// bind the close button of the window to applicationShouldTerminate
	id closeButton = [cocoa_window standardWindowButton:NSWindowCloseButton];
	[closeButton setAction:@selector(applicationShouldTerminate:)];
	[closeButton setTarget:appDelegate];

	// set the delegate which will handle window move events
	[cocoa_window setDelegate:[[CocoaWindowDelegate alloc] initWithDriver:this]];

	// set the window to non transparent
	[cocoa_window setOpaque:YES];

	// enable mouse move events, NeL wants them
	[cocoa_window setAcceptsMouseMovedEvents:YES];

	// there are no overlapping subviews, can use the magical optimization :)
	[cocoa_window useOptimizedDrawing:YES];

	// put the window to the front and make it the key window
	[cocoa_window makeKeyAndOrderFront:nil];

	// this is our main window
	[cocoa_window makeMainWindow];

	// create a dummy view which works like the window on other platforms
	// the open gl view will be created as subview of this one.
	window = [[NSView alloc]
		initWithFrame:NSMakeRect(0, 0, mode.Width, mode.Height)];

	[cocoa_window setContentView: (NSView*)window];

	if(window == EmptyWindow)
	{
		nldebug("cannot create cocoa view for cocoa window");
		return false;
	}

#elif defined (NL_OS_UNIX)

	if (_visual_info == NULL)
		return false;

	nlWindow root = RootWindow(_dpy, DefaultScreen(_dpy));

	XSetWindowAttributes attr;
	attr.background_pixel = BlackPixel(_dpy, DefaultScreen(_dpy));
	attr.colormap = XCreateColormap(_dpy, root, _visual_info->visual, AllocNone);
	int attr_flags = CWBackPixel | CWColormap;

	window = XCreateWindow (_dpy, root, 0, 0, mode.Width, mode.Height, 0, _visual_info->depth, InputOutput, _visual_info->visual, attr_flags, &attr);

	if (window == EmptyWindow)
	{
		nlerror("3D: XCreateWindow() failed");
		return false;
	}

	// normal window type
	XChangeProperty(_dpy, window, XA_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&XA_WM_WINDOW_TYPE_NORMAL, 1);

	// set WM hints
	XWMHints *wm_hints = XAllocWMHints();

	if (wm_hints)
	{
		wm_hints->flags = StateHint | InputHint;
		wm_hints->initial_state = NormalState;
		wm_hints->input = True;

		XSetWMHints(_dpy, window, wm_hints);
		XFree(wm_hints);
	}
	else
	{
		nlwarning("3D: Couldn't allocate XWMHints");
	}

	// set class hints
	XClassHint *class_hints = XAllocClassHint();

	if (class_hints)
	{
		class_hints->res_name = (char*)"NeL";
		class_hints->res_class = (char*)"nel";

		XSetClassHint(_dpy, window, class_hints);
		XFree(class_hints);
	}
	else
	{
		nlwarning("3D: Couldn't allocate XClassHint");
	}

#endif // NL_OS_UNIX

	_win = window;

	_CurrentMode.Width = mode.Width;
	_CurrentMode.Height = mode.Height;

	// Must destroy this window
	_DestroyWindow = true;

	setWindowTitle(ucstring("NeL window"));

	createCursors();

	return true;
}

// ***************************************************************************

bool CDriverGL::destroyWindow()
{
	H_AUTO_OGL(CDriverGL_destroyWindow)

	releaseCursors();

	// make sure window icons are deleted
	std::vector<NLMISC::CBitmap> bitmaps;
	setWindowIcon(bitmaps);

#ifdef USE_OPENGLES

	if (_EglDisplay && _EglContext)
	{
		eglMakeCurrent(_EglDisplay, _EglSurface, _EglSurface, _EglContext);

		if (_DestroyWindow)
		{
			eglDestroyContext(_EglDisplay, _EglContext);
		}
	}

#elif defined(NL_OS_WINDOWS)

	// Then delete.
	// wglMakeCurrent(NULL,NULL);

	if (_hDC)
		wglMakeCurrent(_hDC, NULL);

	if (_DestroyWindow && _hRC)
	{
		wglDeleteContext(_hRC);
		_hRC = NULL;
	}

	if (_win && _hDC)
	{
		ReleaseDC(_win, _hDC);
		_hDC = NULL;
	}

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

	if (_DestroyWindow && _ctx)
		glXDestroyContext(_dpy, _ctx);

	_ctx = NULL;

#endif

#ifdef NL_OS_WINDOWS

	// don't destroy window if it hasn't been created by our driver
	if (_win && _DestroyWindow)
		DestroyWindow(_win);

#elif defined(NL_OS_MAC)

	if (_DestroyWindow)
	{
		[[containerView() window] release];
		[containerView() release];
		[_glView release];
	}

	_ctx = nil;

#elif defined (NL_OS_UNIX)

	_EventEmitter.closeIM();

	if (_DestroyWindow && _win)
		XDestroyWindow(_dpy, _win);

	// Ungrab the keyboard (probably not necessary);
//	XUnmapWindow(_dpy, _win);
	XSync(_dpy, True);
	XUngrabKeyboard(_dpy, CurrentTime);

#endif

	_win = EmptyWindow;

	return true;
}

// ***************************************************************************

CDriverGL::EWindowStyle CDriverGL::getWindowStyle() const
{
	H_AUTO_OGL(CDriverGL_getWindowStyle)

	if (!_CurrentMode.Windowed)
		return EWSFullscreen;

	return EWSWindowed;
}

// ***************************************************************************

bool CDriverGL::setWindowStyle(EWindowStyle windowStyle)
{
	H_AUTO_OGL(CDriverGL_setWindowStyle)

	// don't change window style, if we did not create the window
	if (_win == EmptyWindow || !_DestroyWindow)
		return true;

	if (getWindowStyle() == windowStyle)
		return true;

#if defined(NL_OS_WINDOWS)

	// get current style
	LONG dwStyle = GetWindowLong(_win, GWL_STYLE);

	// prepare new style
	LONG dwNewStyle = WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	// get window current state
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);

	bool isMaximized = GetWindowPlacement(_win, &wndpl) && (wndpl.showCmd == SW_SHOWMAXIMIZED);
	bool isVisible = false;

	if (windowStyle == EWSWindowed && !_CurrentMode.OffScreen)
	{
		dwNewStyle |= WS_OVERLAPPEDWINDOW;

		// if we can't resize window, remove maximize box and resize anchors
		if (!_Resizable) dwNewStyle ^= WS_MAXIMIZEBOX|WS_THICKFRAME;

		isVisible = (dwStyle & WS_VISIBLE) != 0;
	}
	else if (windowStyle == EWSFullscreen)
	{
		dwNewStyle |= WS_POPUP;
		isVisible = true;
	}

	if (isVisible)
		dwNewStyle |= WS_VISIBLE;

	if (dwStyle != dwNewStyle)
		SetWindowLong(_win, GWL_STYLE, dwNewStyle);

//	if (windowStyle == EWSMaximized && isVisible && !isMaximized)
//		ShowWindow(_hWnd, SW_SHOWMAXIMIZED);
//	else if (isMaximized && isVisible)
//		ShowWindow(_hWnd, SW_RESTORE);

#elif defined(NL_OS_MAC)

	// leave fullscreen mode, enter windowed mode
	if(windowStyle == EWSWindowed && [containerView() isInFullScreenMode])
	{
		// disable manual setting of back buffer size, cocoa handles this
		// automatically as soon as the view gets resized
		CGLError error = CGLDisable((CGLContextObj)[_ctx CGLContextObj],
			kCGLCESurfaceBackingSize);

		if(error != kCGLNoError)
			nlerror("cannot disable kCGLCESurfaceBackingSize (%s)",
				CGLErrorString(error));

		// pull the view back from fullscreen restoring window options
		[containerView() exitFullScreenModeWithOptions:nil];

		// let the gl view receive key events
		[[containerView() window] makeFirstResponder:_glView];

		// bring the window containing the gl view to the front
		[[containerView() window] makeKeyAndOrderFront:nil];
	}

	// enter fullscreen, leave windowed mode
	else if(windowStyle == EWSFullscreen && ![containerView() isInFullScreenMode])
	{
		// enable manual back buffer size for mode setting in fullscreen
		CGLError error = CGLEnable((CGLContextObj)[_ctx CGLContextObj],
			kCGLCESurfaceBackingSize);

		if(error != kCGLNoError)
			nlerror("cannot enable kCGLCESurfaceBackingSize (%s)",
				CGLErrorString(error));

		// put the view in fullscreen mode, hiding the dock but enabling the menubar
		// to pop up if the mouse hits the top screen border.
		// NOTE: withOptions:nil disables <CMD>+<Tab> application switching!
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
		[containerView() enterFullScreenMode:[NSScreen mainScreen] withOptions:
			[NSDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithInt:
					NSApplicationPresentationHideDock |
					NSApplicationPresentationAutoHideMenuBar],
				NSFullScreenModeApplicationPresentationOptions, nil]];
#endif
		/*
			TODO check if simply using NSView enterFullScreenMode is a good idea.
			 the context can be set to full screen as well, performance differences?
		*/

		// let the gl view receive key events
		[[containerView() window] makeFirstResponder:_glView];
	}

#elif defined(NL_OS_UNIX)

	XWindowAttributes attr;
	XGetWindowAttributes(_dpy, _win, &attr);

	// if window is mapped use events else properties
	if (attr.map_state != IsUnmapped)
	{
		// Toggle fullscreen
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.display = _dpy;
		xev.xclient.window = _win;
		xev.xclient.message_type = XA_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = windowStyle == EWSFullscreen ? _NET_WM_STATE_ADD:_NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = XA_WM_STATE_FULLSCREEN;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1; // 1 for Application, 2 for Page or Taskbar, 0 for old source
		xev.xclient.data.l[4] = 0;
		if (!XSendEvent(_dpy, XDefaultRootWindow(_dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
		{
			nlwarning("3D: Failed to toggle to fullscreen");
			return false;
		}
	}
	else
	{
		if (windowStyle == EWSFullscreen)
		{
			// set state property to fullscreen
			XChangeProperty(_dpy, _win, XA_WM_STATE, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&XA_WM_STATE_FULLSCREEN, 1);
		}
		else
		{
			// delete state property
			XDeleteProperty(_dpy, _win, XA_WM_STATE);
		}
	}

#endif // NL_OS_WINDOWS

	_CurrentMode.Windowed = (windowStyle == EWSWindowed);

	return true;
}

// --------------------------------------------------
bool CDriverGL::setMode(const GfxMode& mode)
{
	H_AUTO_OGL(CDriverGL_setMode);

	// don't modify window or screen if managed by a 3rd party library
	if (!_DestroyWindow)
		return true;

#if defined(NL_OS_WINDOWS)
	// save relative cursor
	POINT cursorPos;
	cursorPos.x = 0;
	cursorPos.y = 0;

	BOOL cursorPosOk = isSystemCursorInClientArea()
		&& GetCursorPos(&cursorPos)
		&& ScreenToClient(_win, &cursorPos);
	sint curX = (sint)cursorPos.x * (sint)mode.Width;
	sint curY = (sint)cursorPos.y * (sint)mode.Height;
#endif

	if (!setScreenMode(mode))
		return false;

	// when changing window style, it's possible system change window size too
	setWindowStyle(mode.Windowed ? EWSWindowed : EWSFullscreen);

	if (!mode.Windowed)
		_CurrentMode.Depth = mode.Depth;

	setWindowSize(mode.Width, mode.Height);
	setWindowPos(_WindowX, _WindowY);

	switch (_CurrentMode.Depth)
	{
		case 16: _ColorDepth = ColorDepth16; break;
		case 24:
		case 32: _ColorDepth = ColorDepth32; break;
	}

#if defined(NL_OS_WINDOWS)
	// restore relative cursor
	if (cursorPosOk)
	{
		cursorPos.x = curX / (sint)mode.Width;
		cursorPos.y = curY / (sint)mode.Height;
		ClientToScreen(_win, &cursorPos);
		SetCursorPos(cursorPos.x, cursorPos.y);
	}
#endif

	// set color depth for custom cursor
	updateCursor(true);

	return true;
}

#if defined(NL_OS_MAC) && defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6


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

#elif defined(NL_OS_MAC)

long GetDictionaryLong(CFDictionaryRef theDict, const void* key)
{
	long value = 0;
	CFNumberRef numRef;
	numRef = (CFNumberRef)CFDictionaryGetValue(theDict, key);
	if (numRef != NULL)
		CFNumberGetValue(numRef, kCFNumberLongType, &value);
	return value;
}

// some macros to make code more readable.
#define GetModeWidth(mode) GetDictionaryLong((mode), kCGDisplayWidth)
#define GetModeHeight(mode) GetDictionaryLong((mode), kCGDisplayHeight)
#define GetModeBitsPerPixel(mode) GetDictionaryLong((mode), kCGDisplayBitsPerPixel)

#endif // defined(NL_OS_MAC)

// --------------------------------------------------
bool CDriverGL::getModes(std::vector<GfxMode> &modes)
{
	H_AUTO_OGL(CDriverGL_getModes)

#ifdef NL_OS_WINDOWS
	sint modeIndex = 0;
	DEVMODE devMode;
	while (EnumDisplaySettings (NULL, modeIndex, &devMode))
	{
		// Keep only 16 and 32 bits
		if ((devMode.dmBitsPerPel == 16 ) || (devMode.dmBitsPerPel == 32))
		{
			// Add this mode
			GfxMode mode;
			mode.Width = (uint16)devMode.dmPelsWidth;
			mode.Height = (uint16)devMode.dmPelsHeight;
			mode.Depth = (uint8)devMode.dmBitsPerPel;
			mode.Frequency = devMode.dmDisplayFrequency;
			modes.push_back (mode);
		}

		// Mode index
		modeIndex++;
	}

#elif defined(NL_OS_MAC)

	static const CGDisplayCount kMaxDisplays = 16;
	CGDirectDisplayID display[kMaxDisplays];
	CGDisplayCount numDisplays;

	CGDisplayErr err = CGGetActiveDisplayList(kMaxDisplays, display, &numDisplays);
	if(err != CGDisplayNoErr)
	{
		nlwarning("Cannot get displays (%d)", err);
		return false;
	}

	// nldebug("3D: %d displays found", (int)numDisplays);

	for (CGDisplayCount i = 0; i < numDisplays; ++i)
	{
		CGDirectDisplayID dspy = display[i];

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
		CFArrayRef modeList = CGDisplayCopyAllDisplayModes(dspy, NULL);
#else
		CFArrayRef modeList = CGDisplayAvailableModes(dspy);
#endif

		if (modeList == NULL)
		{
			nlwarning("Display is invalid");
			continue;
		}

		for (CFIndex j = 0; j < CFArrayGetCount(modeList); ++j)
		{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
			CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeList, j);
			uint8 bpp = bppFromDisplayMode(mode);
#else
			CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(modeList, j);
			uint8 bpp = (uint8)GetModeBitsPerPixel(mode);
#endif

			if (bpp >= 16)
			{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
				uint16 w = CGDisplayModeGetWidth(mode);
				uint16 h = CGDisplayModeGetHeight(mode);
#else
				uint16 w = (uint16)GetModeWidth(mode);
				uint16 h = (uint16)GetModeHeight(mode);
#endif

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

				// nldebug(" Display 0x%x: Mode %dx%d, %d BPP", dspy, w, h, bpp);
			}
		}
	}

#elif defined (NL_OS_UNIX)

	bool found = false;
	int screen = DefaultScreen(_dpy);

#if defined(HAVE_XRANDR)
	if (!found && _xrandr_version >= 100)
	{
		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, RootWindow(_dpy, screen));

		if (screen_config)
		{
			// retrieve the list of resolutions
			int nsizes = 0;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);

			if (nsizes > 0)
			{
//				nldebug("3D: %d available XRandR modes:", nsizes);
				for (sint i = 0; i < nsizes; ++i)
				{
					// Add this mode
					GfxMode mode;
					mode.Width = sizes[i].width;
					mode.Height = sizes[i].height;
					mode.Frequency = 0;
					modes.push_back(mode);
//					nldebug("3D:   Mode %d: %dx%d", i, mode.Width, mode.Height);
				}

				found = true;
			}
			else
			{
				nlwarning("3D: No XRandR modes available");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}
#endif

#ifdef XF86VIDMODE
	if (!found && _xvidmode_version > 0)
	{
		int nmodes;
		XF86VidModeModeInfo **ms;
		if (XF86VidModeGetAllModeLines(_dpy, screen, &nmodes, &ms))
		{
//			nlinfo("3D: %d available XF86VidMode modes:", nmodes);
			for (int j = 0; j < nmodes; j++)
			{
				// Add this mode
				GfxMode mode;
				mode.Width = (uint16)ms[j]->hdisplay;
				mode.Height = (uint16)ms[j]->vdisplay;
				mode.Frequency = modeInfoToFrequency(ms[j]);
//				nlinfo("3D:   Mode %d: %dx%d, %d Hz", j, mode.Width, mode.Height, mode.Frequency);
				modes.push_back (mode);
			}
			XFree(ms);
		}
		else
		{
			nlwarning("3D: XF86VidModeGetAllModeLines failed");
		}
	}
#endif // XF86VIDMODE

	if (!found)
	{
		// Add current screen mode
		GfxMode mode;
		mode.Width = DisplayWidth(_dpy, screen);
		mode.Height = DisplayHeight(_dpy, screen);
		mode.Frequency = 0;
		modes.push_back(mode);
	}

#endif
	return true;
}

// --------------------------------------------------
bool CDriverGL::getCurrentScreenMode(GfxMode &mode)
{
	H_AUTO_OGL(CDriverGL_getCurrentScreenMode)

#ifdef NL_OS_WINDOWS

	DEVMODE	devmode;
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

	mode.Windowed = _CurrentMode.Windowed;
	mode.OffScreen = false;
	mode.Depth = (uint8)devmode.dmBitsPerPel;
	mode.Frequency = devmode.dmDisplayFrequency;
	mode.Width = (uint16)devmode.dmPelsWidth;
	mode.Height = (uint16)devmode.dmPelsHeight;
	mode.AntiAlias = _CurrentMode.AntiAlias;

#elif defined(NL_OS_MAC)

	// the sceen with the menu bar
	NSScreen* screen = [[NSScreen screens] objectAtIndex:0];

	mode.OffScreen = false;
	mode.Frequency = 0;
	mode.Depth     = NSBitsPerPixelFromDepth([screen depth]);

	// in fullscreen mode
	if([containerView() isInFullScreenMode])
	{
		// return the size of the back buffer (like having switched monitor mode)
		mode.Windowed  = false;
		mode.Width     = _backBufferWidth;
		mode.Height    = _backBufferHeight;
	}

	// in windowed mode
	else
	{
		// return the size of the screen with menu bar
		mode.Windowed  = true;
		mode.Width     = (uint16)[screen frame].size.width;
		mode.Height    = (uint16)[screen frame].size.height;
	}

#elif defined(NL_OS_UNIX)

	bool found = false;
	int screen = DefaultScreen(_dpy);

#ifdef HAVE_XRANDR

	if (!found && _xrandr_version > 0)
	{
		XRRScreenConfiguration *screen_config = XRRGetScreenInfo(_dpy, RootWindow(_dpy, screen));

		if (screen_config)
		{
			int nsizes;
			XRRScreenSize *sizes = XRRConfigSizes(screen_config, &nsizes);
			if (nsizes > 0)
			{
				Rotation cur_rotation;
				SizeID size = XRRConfigCurrentConfiguration(screen_config, &cur_rotation);

				mode.Windowed = _CurrentMode.Windowed;
				mode.OffScreen = false;
				mode.Depth = (uint) DefaultDepth(_dpy, screen);
				mode.Frequency = 0;
				mode.Width = sizes[size].width;
				mode.Height = sizes[size].height;

				found = true;

				nlinfo("3D: Current XRandR mode %d: %dx%d, %dbit", size, mode.Width, mode.Height, mode.Depth);
			}
			else
			{
				nlwarning("3D: No XRandR modes available");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif // HAVE_XRANDR

#ifdef XF86VIDMODE

	if (!found && _xvidmode_version > 0)
	{
		sint pixelClock;
		XF86VidModeModeLine xmode;

		if (XF86VidModeGetModeLine(_dpy, screen, &pixelClock, &xmode))
		{
			mode.Windowed = _CurrentMode.Windowed;
			mode.OffScreen = false;
			mode.Depth = (uint) DefaultDepth(_dpy, screen);
			mode.Frequency = 1000 * pixelClock / (xmode.htotal * xmode.vtotal) ;
			mode.Width = xmode.hdisplay;
			mode.Height = xmode.vdisplay;
			nlinfo("3D: Current XF86VidMode mode: %dx%d, %d Hz, %dbit", mode.Width, mode.Height, mode.Frequency, mode.Depth);

			found = true;
		}
		else
		{
			nlwarning("3D: XF86VidModeGetModeLine failed, cannot get current video mode");
		}
	}

#endif // XF86VidMode

	if (!found)
	{
		mode.Windowed = _CurrentMode.Windowed;
		mode.OffScreen = _CurrentMode.OffScreen;
		mode.Depth = (uint) DefaultDepth(_dpy, screen);
		mode.Frequency = 0;
		mode.Width = DisplayWidth(_dpy, screen);
		mode.Height = DisplayHeight(_dpy, screen);

		found = true;

		nldebug("Current mode: %dx%d, %d Hz, %dbit", mode.Width, mode.Height, mode.Frequency, mode.Depth);
	}

#endif // NL_OS_UNIX

	return true;
}

// --------------------------------------------------
void CDriverGL::setWindowTitle(const ucstring &title)
{
	H_AUTO_OGL(CDriverGL_setWindowTitle)

	if (_win == EmptyWindow)
		return;

#ifdef NL_OS_WINDOWS

	SetWindowTextW(_win, (WCHAR*)title.c_str());

#elif defined(NL_OS_MAC)

	[[containerView() window] setTitle:
		[NSString stringWithUTF8String:title.toUtf8().c_str()]];

#elif defined (NL_OS_UNIX)

#ifdef X_HAVE_UTF8_STRING
	// UTF8 properties
	Xutf8SetWMProperties (_dpy, _win, (char*)title.toUtf8().c_str(), (char*)title.toUtf8().c_str(), NULL, 0, NULL, NULL, NULL);
#else
	// standard properties
	XTextProperty text_property;
	if (XStringListToTextProperty((char**)&title.toUtf8().c_str(), 1, &text_property) != 0)
	{
		XSetWMProperties (_dpy, _win, &text_property, &text_property,  NULL, 0, NULL, NULL, NULL);
	}
	else
	{
		nlwarning("3D: Can't convert title to TextProperty");
	}
#endif

#endif // NL_OS_WINDOWS
}

// ***************************************************************************
void CDriverGL::setWindowPos(sint32 x, sint32 y)
{
	H_AUTO_OGL(CDriverGL_setWindowPos)

	_WindowX = x;
	_WindowY = y;

	if (_win == EmptyWindow || !_CurrentMode.Windowed)
		return;

#ifdef NL_OS_WINDOWS

	// save relative cursor
	POINT cursorPos;
	BOOL cursorPosOk = isSystemCursorInClientArea()
		&& GetCursorPos(&cursorPos)
		&& ScreenToClient(_win, &cursorPos);

	SetWindowPos(_win, NULL, x, y, 0, 0, /*SWP_NOZORDER | SWP_NOACTIVATE |*/ SWP_NOSIZE);

	if (cursorPosOk)
	{
		ClientToScreen(_win, &cursorPos);
		SetCursorPos(cursorPos.x, cursorPos.y);
	}

#elif defined(NL_OS_MAC)
	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [[containerView() window] frame];

	// convert y from NeL coordinates to cocoa coordinates
	y = screenRect.size.height - y;

	// tell cocoa to move the window
	[[containerView() window] setFrameTopLeftPoint:NSMakePoint(x, y)];

#elif defined (NL_OS_UNIX)

	if (_CurrentMode.Windowed)
	{
		// first time requesting decoration sizes
		if (_WindowX && _WindowY && !_DecorationWidth && !_DecorationHeight && _WndActive)
		{
			_DecorationWidth = -1;
			_DecorationHeight = -1;
		}

		XMoveWindow(_dpy, _win, x, y);
	}

#endif // NL_OS_WINDOWS
}

// ***************************************************************************
void CDriverGL::showWindow(bool show)
{
	H_AUTO_OGL(CDriverGL_showWindow)

	// don't change window visibility, if we didn't create the window
	if (_win == EmptyWindow || !_DestroyWindow)
		return;

	_WindowVisible = show;

#ifdef NL_OS_WINDOWS

	ShowWindow (_win, show ? SW_SHOW:SW_HIDE);

#elif defined(NL_OS_MAC)

	// TODO: Missing Mac Implementation for showWindow

#elif defined (NL_OS_UNIX)

	if (show)
	{
		XMapRaised(_dpy, _win);

		// fix window position if windows manager want to impose them
		setWindowPos(_WindowX, _WindowY);
	}
	else
	{
		XUnmapWindow(_dpy, _win);
	}

#endif // NL_OS_WINDOWS
}

// --------------------------------------------------
emptyProc CDriverGL::getWindowProc()
{
	H_AUTO_OGL(CDriverGL_getWindowProc)

	return (emptyProc)GlWndProc;
}

// --------------------------------------------------
bool CDriverGL::createContext()
{
#ifdef USE_OPENGLES
	uint samples = 0;

	if (_CurrentMode.AntiAlias > -1)
	{
		if (_CurrentMode.AntiAlias == 0)
		{
			samples = 4;
		}
		else
		{
			samples = _CurrentMode.AntiAlias;
		}
	}

	EGLint attribList[] =
	{
		EGL_RED_SIZE,		8,
		EGL_GREEN_SIZE,		8,
		EGL_BLUE_SIZE,		8,
		EGL_ALPHA_SIZE,		8,
		EGL_DEPTH_SIZE,		16,
		EGL_STENCIL_SIZE,	8,
//		EGL_SAMPLE_BUFFERS,	_CurrentMode.AntiAlias > -1 ? 1:0,
//		EGL_SAMPLES,		samples,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	// Get Display
	_EglDisplay = EGL_NO_DISPLAY; // eglGetDisplay(_hDC);

	if (_EglDisplay == EGL_NO_DISPLAY)
	{
		_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

		if (_EglDisplay == EGL_NO_DISPLAY)
		{
			nlwarning("3D: failed to get display 0x%x", eglGetError());
			return false;
		}
	}

	// Initialize EGL
	EGLint majorVersion;
	EGLint minorVersion;

	if (!eglInitialize(_EglDisplay, &majorVersion, &minorVersion))
	{
		return EGL_FALSE;
	}

	const char *extensions = eglQueryString(_EglDisplay, EGL_EXTENSIONS);


	// Get configs
	EGLint numConfigs;

	if (!eglGetConfigs(_EglDisplay, NULL, 0, &numConfigs))
	{
		return false;
	}

	// Choose config
	EGLConfig config = NULL;

	if (!eglChooseConfig(_EglDisplay, attribList, &config, 1, &numConfigs))
	{
		return false;
	}

	// Create a surface
	_EglSurface = eglCreateWindowSurface(_EglDisplay, config, (EGLNativeWindowType)_win, NULL);

	if (_EglSurface == EGL_NO_SURFACE)
	{
		return false;
	}

	// Create a GL context
	EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE
	};

	_EglContext = eglCreateContext(_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);

	if (_EglContext == EGL_NO_CONTEXT)
	{
		return false;
	}

	// Make the context current
	if (!eglMakeCurrent(_EglDisplay, _EglSurface, _EglSurface, _EglContext))
	{
		return false;
	}
#elif defined(NL_OS_WINDOWS)
	_hDC = GetDC(_win);
	_CurrentMode.Depth = uint8(GetDeviceCaps(_hDC,BITSPIXEL));

	wglMakeCurrent(_hDC,NULL);
	// ---
	memset(&_pfd,0,sizeof(_pfd));
	_pfd.nSize        = sizeof(_pfd);
	_pfd.nVersion     = 1;
	_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType   = PFD_TYPE_RGBA;
	_pfd.cColorBits   = _CurrentMode.Depth;
	// Choose best suited Depth Buffer.
	if(_CurrentMode.Depth <= 16)
	{
		_pfd.cDepthBits   = 16;
	}
	else
	{
		_pfd.cDepthBits = 24;
		_pfd.cAlphaBits	= 8;
		_pfd.cStencilBits = 8;
	}
	_pfd.iLayerType	  = PFD_MAIN_PLANE;
	int pf=ChoosePixelFormat(_hDC,&_pfd);
	if (!pf)
	{
		return false;
	}

	if ( !SetPixelFormat(_hDC,pf,&_pfd) )
	{
		return false;
	}
	_hRC=wglCreateContext(_hDC);

	wglMakeCurrent(_hDC,_hRC);
#endif

	return true;
}

// --------------------------------------------------
bool CDriverGL::activate()
{
	H_AUTO_OGL(CDriverGL_activate);

	if (_win == EmptyWindow)
		return false;

#ifdef USE_OPENGLES

	EGLContext ctx = eglGetCurrentContext();

	if (ctx != _EglContext)
	{
		// Make the context current
		if (!eglMakeCurrent(_EglDisplay, _EglSurface, _EglSurface, _EglContext))
		{
			return false;
		}
	}

#elif defined(NL_OS_WINDOWS)

	HGLRC hglrc = wglGetCurrentContext();

	if (hglrc != _hRC)
		wglMakeCurrent(_hDC, _hRC);

#elif defined(NL_OS_MAC)

	if([NSOpenGLContext currentContext] != _ctx)
		[_ctx makeCurrentContext];

#elif defined (NL_OS_UNIX)

	GLXContext nctx = glXGetCurrentContext();

	if (nctx != NULL && nctx != _ctx)
		glXMakeCurrent(_dpy, _win, _ctx);

#endif // USE_OPENGLES

	return true;
}

// --------------------------------------------------
IDriver::TMessageBoxId CDriverGL::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, TMessageBoxIcon icon)
{
	H_AUTO_OGL(CDriverGL_systemMessageBox)
#ifdef NL_OS_WINDOWS
	switch (::MessageBox (NULL, message, title, ((type==retryCancelType)?MB_RETRYCANCEL:
										(type==yesNoCancelType)?MB_YESNOCANCEL:
										(type==okCancelType)?MB_OKCANCEL:
										(type==abortRetryIgnoreType)?MB_ABORTRETRYIGNORE:
										(type==yesNoType)?MB_YESNO|MB_ICONQUESTION:MB_OK)|

										((icon==handIcon)?MB_ICONHAND:
										(icon==questionIcon)?MB_ICONQUESTION:
										(icon==exclamationIcon)?MB_ICONEXCLAMATION:
										(icon==asteriskIcon)?MB_ICONASTERISK:
										(icon==warningIcon)?MB_ICONWARNING:
										(icon==errorIcon)?MB_ICONERROR:
										(icon==informationIcon)?MB_ICONINFORMATION:
										(icon==stopIcon)?MB_ICONSTOP:0)))
										{
											case IDOK:
												return okId;
											case IDCANCEL:
												return cancelId;
											case IDABORT:
												return abortId;
											case IDRETRY:
												return retryId;
											case IDIGNORE:
												return ignoreId;
											case IDYES:
												return yesId;
											case IDNO:
												return noId;
										}
	nlstop;
#else // NL_OS_WINDOWS
	// Call the console version!
	IDriver::systemMessageBox (message, title, type, icon);
#endif // NL_OS_WINDOWS
	return okId;
}

void CDriverGL::getWindowSize(uint32 &width, uint32 &height)
{
	H_AUTO_OGL(CDriverGL_getWindowSize)

	if (_CurrentMode.OffScreen)
	{
#ifdef NL_OS_WINDOWS

#ifndef USE_OPENGLES
		if (_PBuffer)
		{
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width );
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height );
		}
#endif

#endif
	}
	else
	{
#ifdef NL_OS_MAC
		if([containerView() isInFullScreenMode])
		{
			width = _backBufferWidth;
			height = _backBufferHeight;
			return;
		}
#endif
		width = _CurrentMode.Width;
		height = _CurrentMode.Height;
	}
}

void CDriverGL::setWindowSize(uint32 width, uint32 height)
{
	H_AUTO_OGL(CDriverGL_setWindowSize)

	if (_win == EmptyWindow)
		return;

#if defined(NL_OS_WINDOWS)

	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, width, height);
	AdjustWindowRectEx(&rc, GetWindowStyle(_win), GetMenu(_win) != NULL, GetWindowExStyle(_win));
	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
	// set position to (0, 0) if fullscreen
	if (_CurrentMode.Windowed)
		flags |= SWP_NOMOVE;
	SetWindowPos(_win, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, flags);

	// init window width and height
	RECT clientRect;
	GetClientRect(_win, &clientRect);
	_CurrentMode.Width = uint16(clientRect.right-clientRect.left);
	_CurrentMode.Height = uint16(clientRect.bottom-clientRect.top);
	GetWindowRect(_win, &clientRect);
	_WindowX = clientRect.left;
	_WindowY = clientRect.top;

#elif defined(NL_OS_MAC)

	// for fullscreen mode, adjust the back buffer size to desired resolution
	if([containerView() isInFullScreenMode])
	{
		// disable and re-enable fullscreen
		// fixes #1062 (http://dev.ryzom.com/issues/1062)
		setWindowStyle(EWSWindowed);
		setWindowStyle(EWSFullscreen);

		// set the back buffer manually to match the desired rendering resolution
		GLint dim[2]   = { width, height };
		CGLError error = CGLSetParameter(
			(CGLContextObj)[_ctx CGLContextObj],
			kCGLCPSurfaceBackingSize, dim);

		if(error != kCGLNoError)
			nlerror("cannot set kCGLCPSurfaceBackingSize parameter (%s)",
				CGLErrorString(error));

		_backBufferHeight = height;
		_backBufferWidth  = width;
	}
	else
	{
		// only change the window size if the driver created the window itself
		if(_DestroyWindow)
		{
			// get the windows current frame
			NSRect rect = [[containerView() window] frame];

			// convert the desired content size to window size
			rect = [[containerView() window] frameRectForContentRect:
				NSMakeRect(rect.origin.x, rect.origin.y, width, height)];

			// update window dimensions
			[[containerView() window] setFrame:rect display:YES];
		}
	}

#elif defined(NL_OS_UNIX)

	if (!_Resizable)
	{
		// Update WM hints (disallow resizing)
		XSizeHints *size_hints = XAllocSizeHints();

		size_hints->flags = PMinSize | PMaxSize;
		size_hints->min_width = width;
		size_hints->min_height = height;
		size_hints->max_width = width;
		size_hints->max_height = height;

		XSetWMNormalHints(_dpy, _win, size_hints);
		XFree(size_hints);
	}
	else
	{
//		XSetWMNormalHints(_dpy, _win, StdHints);
	}

	if (width != _CurrentMode.Width || height != _CurrentMode.Height)
	{
		// resize the window
		XResizeWindow(_dpy, _win, width, height);

		_CurrentMode.Width = width;
		_CurrentMode.Height = height;
	}

#endif // NL_OS_WINDOWS
}

void CDriverGL::getWindowPos(sint32 &x, sint32 &y)
{
	H_AUTO_OGL(CDriverGL_getWindowPos)

	// Off-screen rendering ?
	if (_CurrentMode.OffScreen)
	{
		x = y = 0;
	}
	else
	{
		x = _WindowX;
		y = _WindowY;
	}
}

// --------------------------------------------------
bool CDriverGL::isActive()
{
	H_AUTO_OGL(CDriverGL_isActive)

	if (_win == EmptyWindow)
		return false;

	bool res = true;

#ifdef NL_OS_WINDOWS

	res = (IsWindow(_win) != FALSE);

#elif defined(NL_OS_MAC)
	// TODO: Missing Mac Implementation for isActive (always true if a window is set)
#elif defined (NL_OS_UNIX)

	// check if our window is still active
	XWindowAttributes attr;
	Status status = XGetWindowAttributes(_dpy, _win, &attr);

	res = (status == 1);

#endif // NL_OS_UNIX

	return res;
}

// ***************************************************************************
bool CDriverGL::setMonitorColorProperties (const CMonitorColorProperties &properties)
{
	H_AUTO_OGL(CDriverGL_setMonitorColorProperties )

#ifdef NL_OS_WINDOWS

	// Get a DC
	HDC dc = CreateDC ("DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		// The ramp
		WORD ramp[256*3];

		// For each composant
		uint c;
		for( c=0; c<3; c++ )
		{
			uint i;
			for( i=0; i<256; i++ )
			{
				// Floating value
				float value = (float)i / 256;

				// Contrast
				value = (float) max (0.0f, (value-0.5f) * (float) pow (3.f, properties.Contrast[c]) + 0.5f );

				// Gamma
				value = (float) pow (value, (properties.Gamma[c]>0) ? 1 - 3 * properties.Gamma[c] / 4 : 1 - properties.Gamma[c] );

				// Luminosity
				value = value + properties.Luminosity[c] / 2.f;
				ramp[i+(c<<8)] = (WORD)min ((int)65535, max (0, (int)(value * 65535)));
			}
		}

		// Set the ramp
		bool result = SetDeviceGammaRamp (dc, ramp) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);

		// Returns result
		return result;
	}
	else
	{
		nlwarning ("(CDriverGL::setMonitorColorProperties): can't create DC");
	}

#elif defined(NL_OS_MAC)
	// TODO for Mac: implement CDriverGL::setMonitorColorProperties
	nlwarning ("CDriverGL::setMonitorColorProperties not implemented");

#elif defined (NL_OS_UNIX)
	// TODO for Linux: implement CDriverGL::setMonitorColorProperties
	nlwarning ("CDriverGL::setMonitorColorProperties not implemented");

#endif

	return false;
}

#ifdef NL_OS_MAC
void CDriverGL::setupApplicationMenu()
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
#endif

bool CDriverGL::copyTextToClipboard(const ucstring &text)
{
	return _EventEmitter.copyTextToClipboard(text);
}

bool CDriverGL::pasteTextFromClipboard(ucstring &text)
{
	return _EventEmitter.pasteTextFromClipboard(text);
}

#ifdef NL_OS_WINDOWS

bool CDriverGL::convertBitmapToIcon(const NLMISC::CBitmap &bitmap, HICON &icon, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY, bool cursor)
{
	CBitmap src = bitmap;

	// resample bitmap if necessary
	if (src.getWidth() != iconWidth || src.getHeight() != iconHeight)
	{
		src.resample(iconWidth, iconHeight);
	}
	CBitmap colorBm;
	colorBm.resize(iconWidth, iconHeight, CBitmap::RGBA);
	const CRGBA *srcColorPtr = (CRGBA *) &(src.getPixels()[0]);
	const CRGBA *srcColorPtrLast = srcColorPtr + (iconWidth * iconHeight);
	CRGBA *destColorPtr = (CRGBA *) &(colorBm.getPixels()[0]);
	static uint8 alphaThreshold = 127;
	do
	{
		destColorPtr->modulateFromColor(*srcColorPtr, col);
		std::swap(destColorPtr->R, destColorPtr->B);
		++ srcColorPtr;
		++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);
	//
	HBITMAP colorHbm = NULL;
	HBITMAP maskHbm = NULL;
	//
	if (iconDepth == 16)
	{
		std::vector<uint16> colorBm16(iconWidth * iconHeight);
		const CRGBA *src32 = (const CRGBA *) &colorBm.getPixels(0)[0];

		for (uint k = 0; k < colorBm16.size(); ++k)
		{
			colorBm16[k] = ((uint16)(src32[k].R&0xf8)>>3) | ((uint16)(src32[k].G&0xfc)<<3) | ((uint16)(src32[k].B & 0xf8)<<8);
		}

		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 16, &colorBm16[0]);
		std::vector<uint8> bitMask((iconWidth * iconHeight + 7) / 8, 0);

		for (uint k = 0;k < colorBm16.size(); ++k)
		{
			if (src32[k].A <= alphaThreshold)
			{
				bitMask[k / 8] |= (0x80 >> (k & 7));
			}
		}

		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 1, &bitMask[0]);
	}
	else
	{
		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
	}

	ICONINFO iconInfo;
	iconInfo.fIcon = cursor ? FALSE:TRUE;
	iconInfo.xHotspot = (DWORD) hotSpotX;
	iconInfo.yHotspot = (DWORD) hotSpotY;
	iconInfo.hbmMask = maskHbm;
	iconInfo.hbmColor = colorHbm;

	if (colorHbm && maskHbm)
	{
		icon = CreateIconIndirect(&iconInfo);
	}

	//
	if (colorHbm) DeleteObject(colorHbm);
	if (maskHbm) DeleteObject(maskHbm);

	return true;
}

#elif defined(NL_OS_MAC)

#elif defined(NL_OS_UNIX)

bool CDriverGL::convertBitmapToIcon(const NLMISC::CBitmap &bitmap, std::vector<long> &icon)
{
	// get bitmap width and height
	uint width = bitmap.getWidth();
	uint height = bitmap.getHeight();

	// icon position for bitmap
	uint pos = (uint)icon.size();

	// extend icon_data size for bitmap
	icon.resize(pos + 2 + width*height);

	// set bitmap width and height
	icon[pos++] = width;
	icon[pos++] = height;

	// convert RGBA to ARGB
	CObjectVector<uint8> pixels = bitmap.getPixels();
	for(uint j = 0; j < pixels.size(); j+=4)
		icon[pos++] = pixels[j] << 16 | pixels[j+1] << 8 | pixels[j+2] | pixels[j+3] << 24;

	return true;
}

#endif

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
