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

#ifdef NL_OS_WINDOWS
# include <windowsx.h>
#elif defined(NL_OS_MAC)
#elif defined (NL_OS_UNIX)
# include <GL/gl.h>
# include <GL/glx.h>
# ifdef XRANDR
#  include <X11/extensions/Xrandr.h>
# endif
# include <X11/Xatom.h>
# define _NET_WM_STATE_REMOVE	0
# define _NET_WM_STATE_ADD		1
#endif // NL_OS_UNIX

#include "nel/misc/mouse_device.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/3d/u_driver.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

#ifdef NL_OS_WINDOWS

bool GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_OGL(GlWndProc)

	if (!driver)
		return false;

	if(message == WM_SIZE)
	{
		if (!driver->_FullScreen)
		{
			RECT rect;
			GetClientRect (driver->_win, &rect);

			// Setup gl viewport
			driver->_WindowWidth = rect.right-rect.left;
			driver->_WindowHeight = rect.bottom-rect.top;
		}
	}
	else if(message == WM_MOVE)
	{
		if (!driver->_FullScreen)
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
	H_AUTO_OGL(DriverGL_WndProc)
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

bool GlWndProc(CDriverGL *driver, NSEvent* e)
{
	H_AUTO_OGL(GlWndProc)

	if(!driver)
		return false;
	
	// NSLog(@"NSEvent in GlWndProc %@", e);

	switch([e type])
	{
		/* TODO handle window move, resize, activate, close, etc. */
		default:
			return driver->_EventEmitter.processMessage(e);
	}

	return false;
}

#elif defined (NL_OS_UNIX)

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
		nlwarning("Expose event");
		break;

		case ConfigureNotify:

		if (!driver->_FullScreen && driver->_WndActive)
		{
			// first time setting decoration sizes
			if ((driver->_DecorationWidth == -1) || (driver->_DecorationWidth == 0))
			{
				driver->_DecorationWidth = e.xconfigure.x - driver->_WindowX;
				driver->_DecorationHeight = e.xconfigure.y - driver->_WindowY;

				nlwarning("Decoration size x = %d, y = %d", driver->_DecorationWidth, driver->_DecorationHeight);
			}

			driver->_WindowWidth = e.xconfigure.width;
			driver->_WindowHeight = e.xconfigure.height;
			driver->_WindowX = e.xconfigure.x - driver->_DecorationWidth;
			driver->_WindowY = e.xconfigure.y - driver->_DecorationHeight;

			XConfigureEvent event = e.xconfigure;

			nlwarning("Configure x = %d, y = %d, width = %d, height = %d, send event = %d", event.x, event.y, event.width, event.height, event.send_event);
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
bool CDriverGL::init (uint windowIcon, emptyProc exitFunc)
{
	H_AUTO_OGL(CDriverGL_init)

	ExitFunc = exitFunc;

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
		wc.hCursor			= LoadCursorA(NULL, IDC_ARROW);
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

#elif defined (NL_OS_UNIX)

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

#ifdef XRANDR
	_OldSizeID = 0;
	sint xrandr_major, xrandr_minor;
	if (XRRQueryVersion(_dpy, &xrandr_major, &xrandr_minor))
	{
		_xrandr_version = xrandr_major * 100 + xrandr_minor;
		nlinfo("3D: XRandR %d.%d found", xrandr_major, xrandr_minor);
	}

#endif

	_xvidmode_version = 0;

#ifdef XF86VIDMODE
	sint event = 0, error = -1, vm_major = 0, vm_minor = 0;
	if (XF86VidModeQueryExtension(_dpy, &event, &error) && XF86VidModeQueryVersion(_dpy, &vm_major, &vm_minor))
	{
		_xvidmode_version = vm_major * 100 + vm_minor;
		nlinfo("3D: XF86VidMode %d.%d found", vm_major, vm_minor);
	}
#endif

#endif
	return true;
}

// ***************************************************************************
bool CDriverGL::unInit()
{
	H_AUTO_OGL(CDriverGL_unInit)

	if (_FullScreen)
	{
		restoreScreenMode();
		showCursor(true);

		_FullScreen = false;
	}

#ifdef NL_OS_WINDOWS

	// Off-screen rendering ?
	if (_PBuffer)
	{
		nwglReleasePbufferDCARB(_PBuffer, _hDC);
		nwglDestroyPbufferARB(_PBuffer);
		_PBuffer = NULL;
	}

	if (_Registered && !UnregisterClassW(L"NLClass", GetModuleHandle(NULL)))
	{
		nlwarning("Can't unregister NLClass");
	}

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
		winIconSmall = bitmaps[smallIndex].getHICON(smallWidth, smallHeight, 32);

	if (bigIndex > -1)
		winIconBig = bitmaps[bigIndex].getHICON(bigWidth, bigHeight, 32);

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
			// get bitmap width and height
			uint width = bitmaps[i].getWidth();
			uint height = bitmaps[i].getHeight();

			// icon_data position for bitmap
			uint pos = (uint)icon_data.size();

			// extend icon_data size for bitmap
			icon_data.resize(pos + 2 + width*height);

			// set bitmap width and height
			icon_data[pos++] = width;
			icon_data[pos++] = height;

			// convert RGBA to ARGB
			CObjectVector<uint8> pixels = bitmaps[i].getPixels();
			for(uint j = 0; j < pixels.size(); j+=4)
				icon_data[pos++] = pixels[j] << 16 | pixels[j+1] << 8 | pixels[j+2] | pixels[j+3] << 24;
		}
	}

	Atom _NET_WM_ICON = XInternAtom(_dpy, "_NET_WM_ICON", False);

	if (!icon_data.empty())
	{
		// change window icon
		XChangeProperty(_dpy, _win, _NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (const unsigned char *) &icon_data[0], icon_data.size());
	}
	else
	{
		// delete window icon if no bitmap is available
		XDeleteProperty(_dpy, _win, _NET_WM_ICON);
	}

#endif // NL_OS_WINDOWS
}

// --------------------------------------------------
bool CDriverGL::setDisplay(nlWindow wnd, const GfxMode &mode, bool show, bool resizeable) throw(EBadDisplay)
{
	H_AUTO_OGL(CDriverGL_setDisplay)

	_win = EmptyWindow;
	_WindowWidth = _WindowHeight = _WindowX = _WindowY = 0;
	_WindowVisible = false;
	_FullScreen = false;
	_Resizable = resizeable;
	_OffScreen = mode.OffScreen;
	_DestroyWindow = false;

#ifdef NL_OS_WINDOWS

	// Init pointers
	_PBuffer = NULL;
	_hRC = NULL;
	_hDC = NULL;

	// Driver caps.
	//=============
	// Retrieve the WGL extensions before init the driver.
	int						pf;

	// Offscreen mode ?
	if (_OffScreen)
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

		_Depth=uint8(GetDeviceCaps(tempHDC,BITSPIXEL));

		// ---
		memset(&_pfd,0,sizeof(_pfd));
		_pfd.nSize        = sizeof(_pfd);
		_pfd.nVersion     = 1;
		_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		_pfd.iPixelType   = PFD_TYPE_RGBA;
		_pfd.cColorBits   = (char)_Depth;

		// Choose best suited Depth Buffer.
		if(_Depth<=16)
		{
			_pfd.cDepthBits   = 16;
		}
		else
		{
			_pfd.cDepthBits = 24;
			_pfd.cAlphaBits	= 8;
		}
		_pfd.iLayerType	  = PFD_MAIN_PLANE;
		pf=ChoosePixelFormat(tempHDC,&_pfd);
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
		registerWGlExtensions (_Extensions, tempHDC);

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

		_WindowWidth = width;
		_WindowHeight = height;

		/* The next step is to create a device context for the newly created pbuffer. To do this,
			call the the function: */
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
		_Depth = uint8(GetDeviceCaps (_hDC, BITSPIXEL));

		// Destroy the temp gl context
		if (!wglDeleteContext (tempGLRC))
		{
			DWORD error = GetLastError ();
			nlwarning ("CDriverGL::setDisplay: wglDeleteContext failed: 0x%x", error);
		}

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
			wglDeleteContext (_hRC);
			nwglReleasePbufferDCARB( _PBuffer, _hDC );
			nwglDestroyPbufferARB( _PBuffer );
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


		_hDC=GetDC(_win);
		wglMakeCurrent(_hDC,NULL);

		_Depth=uint8(GetDeviceCaps(_hDC,BITSPIXEL));
		// ---
		memset(&_pfd,0,sizeof(_pfd));
		_pfd.nSize        = sizeof(_pfd);
		_pfd.nVersion     = 1;
		_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		_pfd.iPixelType   = PFD_TYPE_RGBA;
		_pfd.cColorBits   = (char)_Depth;
		// Choose best suited Depth Buffer.
		if(_Depth<=16)
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
		pf=ChoosePixelFormat(_hDC,&_pfd);
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

	}

	/// release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}

	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter;

	// setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true /*must delete*/); // the main emitter

	/// try to get direct input
	try
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::CDIEventEmitter::create(GetModuleHandle(NULL), _win, we);
		if (diee)
		{
			_EventEmitter.addEmitter(diee, true);
		}
	}
	catch(EDirectInput &e)
	{
		nlinfo(e.what());
	}

#elif defined(NL_OS_MAC)

	if (wnd == EmptyWindow)
	{
		if (!createWindow(mode))
			return false;
	}
	else
	{
		_win = wnd;
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

	// create a opengl view with the created format
	_glView = [[CocoaOpenGLView alloc]
		initWithFrame:NSMakeRect(0, 0, 0, 0) pixelFormat: format];

	if(!_glView)
		nlerror("cannot create view");

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
	
	[_ctx flushBuffer];
	[containerView() display]; 

	_EventEmitter.init(this, _glView);

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

	if (show || _FullScreen)
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

	// hide window (hack to avoid black window bug)
	if (_win)
		XUnmapWindow(_dpy, _win);

	int screen = DefaultScreen(_dpy);
	res = false;

#ifdef XRANDR

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

#endif // XRANDR

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

	// hide window (hack to avoid black window bug)
	if (_win)
		XUnmapWindow(_dpy, _win);

	int screen = DefaultScreen(_dpy);

#ifdef XRANDR

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

#endif // XRANDR

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
		if (_FullScreen)
			restoreScreenMode();

		return true;
	}

	// save previous screen mode only if switching from windowed to fullscreen
	if (!_FullScreen)
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

#ifdef XRANDR

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

			if (size > -1 && XRRSetScreenConfig(_dpy, screen_config, root, size, saved_rotation, CurrentTime) == RRSetConfigSuccess)
			{
				nlinfo("3D: Switching to XRandR mode %d: %dx%d", size, sizes[size].width, sizes[size].height);
				found = true;
			}
			else
			{
				nlwarning("3D: No corresponding screen mode or XRRSetScreenConfig failed");
			}

			XRRFreeScreenConfigInfo(screen_config);
		}
		else
		{
			nlwarning("3D: XRRGetScreenInfo failed");
		}
	}

#endif

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
	window = [[NSView alloc] init];

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

#endif // NL_OS_UNIX

	_win = window;

	_WindowWidth = mode.Width;
	_WindowHeight = mode.Height;

	// Must destroy this window
	_DestroyWindow = true;

	setWindowTitle(ucstring("NeL window"));

	return true;
}

// ***************************************************************************

bool CDriverGL::destroyWindow()
{
	H_AUTO_OGL(CDriverGL_destroyWindow)

	// make sure window icons are deleted
	std::vector<NLMISC::CBitmap> bitmaps;
	setWindowIcon(bitmaps);

#ifdef NL_OS_WINDOWS

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

		// don't destroy window if it hasn't been created by our driver
		if (_DestroyWindow)
			DestroyWindow(_win);
	}

#elif defined(NL_OS_MAC)

	if(_DestroyWindow)
	{
		[containerView() release];
		[[containerView() window] release];
	}
	
	_ctx = nil;

#elif defined (NL_OS_UNIX)

	if (_DestroyWindow)
	{
		if (_ctx)
			glXDestroyContext(_dpy, _ctx);

		if (_win)
			XDestroyWindow(_dpy, _win);
	}

	_ctx = NULL;

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

	if (_FullScreen)
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

	if (windowStyle == EWSWindowed && !_OffScreen)
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
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
		[containerView() enterFullScreenMode:[NSScreen mainScreen] withOptions:
			[NSDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithInt:
					NSApplicationPresentationHideDock |
					NSApplicationPresentationAutoHideMenuBar],
				NSFullScreenModeApplicationPresentationOptions, nil]];
#endif // AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
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
		xev.xclient.message_type = XInternAtom(_dpy, "_NET_WM_STATE", False);
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = windowStyle == EWSFullscreen ? _NET_WM_STATE_ADD:_NET_WM_STATE_REMOVE;
		xev.xclient.data.l[1] = XInternAtom(_dpy, "_NET_WM_STATE_FULLSCREEN", False);
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1; // 1 for Application, 2 for Page or Taskbar, 0 for old source
		xev.xclient.data.l[4] = 0;
		if (!XSendEvent(_dpy, DefaultRootWindow(_dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
		{
			nlwarning("3D: Failed to toggle to fullscreen");
			return false;
		}
	}
	else
	{
		Atom _NET_WM_STATE = XInternAtom(_dpy, "_NET_WM_STATE", False);

		if (windowStyle == EWSFullscreen)
		{
			Atom _NET_WM_STATE_FULLSCREEN = XInternAtom(_dpy, "_NET_WM_STATE_FULLSCREEN", False);

			// set state property to fullscreen
			XChangeProperty(_dpy, _win, _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&_NET_WM_STATE_FULLSCREEN, 1);
		}
		else
		{
			// delete state property
			XDeleteProperty(_dpy, _win, _NET_WM_STATE);
		}
	}

	// show window (hack to avoid black window bug)
	if (_WindowVisible)
		XMapRaised(_dpy, _win);

#endif // NL_OS_WINDOWS

	_FullScreen = (windowStyle == EWSFullscreen);

	return true;
}

// --------------------------------------------------
bool CDriverGL::setMode(const GfxMode& mode)
{
	H_AUTO_OGL(CDriverGL_setMode)

	if (!setScreenMode(mode))
		return false;

	// when changing window style, it's possible system change window size too
	setWindowStyle(mode.Windowed ? EWSWindowed : EWSFullscreen);

	if (!mode.Windowed)
		_Depth = mode.Depth;

	setWindowSize(mode.Width, mode.Height);
	setWindowPos(_WindowX, _WindowY);

	return true;
}

#if defined(NL_OS_MAC) && defined(AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)

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

#elif defined(NL_OS_MAC) && !defined(AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER)

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

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
		CFArrayRef modeList = CGDisplayCopyAllDisplayModes(dspy, NULL);
#else
		CFArrayRef modeList = CGDisplayAvailableModes(dspy);
#endif // AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER

		if (modeList == NULL)
		{
			nlwarning("Display is invalid");
			continue;
		}

		for (CFIndex j = 0; j < CFArrayGetCount(modeList); ++j)
		{
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
			CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeList, j);
			uint8 bpp = bppFromDisplayMode(mode);
#else
			CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(modeList, j);
			uint8 bpp = (uint8)GetModeBitsPerPixel(mode);
#endif // AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER

			if (bpp >= 16)
			{
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
				uint16 w = CGDisplayModeGetWidth(mode);
				uint16 h = CGDisplayModeGetHeight(mode);
#else
				uint16 w = (uint16)GetModeWidth(mode); 
				uint16 h = (uint16)GetModeHeight(mode); 
#endif // AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER

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

#if defined(XRANDR)
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

	mode.Windowed = !_FullScreen;
	mode.OffScreen = false;
	mode.Depth = (uint8)devmode.dmBitsPerPel;
	mode.Frequency = devmode.dmDisplayFrequency,
	mode.Width = (uint16)devmode.dmPelsWidth;
	mode.Height = (uint16)devmode.dmPelsHeight;
	mode.AntiAlias = _AntiAliasing;

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

#ifdef XRANDR

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

				mode.Windowed = !_FullScreen;
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

#endif // XRANDR

#ifdef XF86VIDMODE

	if (!found && _xvidmode_version > 0)
	{
		sint pixelClock;
		XF86VidModeModeLine xmode;

		if (XF86VidModeGetModeLine(_dpy, screen, &pixelClock, &xmode))
		{
			mode.Windowed = !_FullScreen;
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
		mode.Windowed = !_FullScreen;
		mode.OffScreen = _OffScreen;
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
	Xutf8SetWMProperties (_dpy, _win, (char*)title.toUtf8().c_str(), (char*)title.toUtf8().c_str(), NULL, 0, NULL, NULL, NULL);
#else
	XTextProperty text_property;
	XStringListToTextProperty((char**)&title.toUtf8().c_str(), 1, &text_property);
	XSetWMProperties (_dpy, _win, &text_property, &text_property,  0, 0, NULL, 0, 0);
#endif

#endif // NL_OS_WINDOWS
}

// ***************************************************************************
void CDriverGL::setWindowPos(sint32 x, sint32 y)
{
	H_AUTO_OGL(CDriverGL_setWindowPos)

	_WindowX = x;
	_WindowY = y;

	if (_win == EmptyWindow || _FullScreen)
		return;

#ifdef NL_OS_WINDOWS

	SetWindowPos(_win, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

#elif defined(NL_OS_MAC)

	nldebug("setting window pos to %d %d", x, y);

	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [[containerView() window] frame];

	// convert y from NeL coordinates to cocoa coordinates
	y = screenRect.size.height - y;

	// tell cocoa to move the window
	[[containerView() window] setFrameTopLeftPoint:NSMakePoint(x, y)];

#elif defined (NL_OS_UNIX)

	// first time requesting decoration sizes
	if (_WindowX && _WindowY && !_DecorationWidth && !_DecorationHeight && _WndActive)
	{
		_DecorationWidth = -1;
		_DecorationHeight = -1;
	}

	XMoveWindow(_dpy, _win, x, y);

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

# warning "OpenGL Driver: Missing Mac Implementation for showWindow"

#elif defined (NL_OS_UNIX)

	if (show)
	{
//		XMapWindow(_dpy, _win);
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
bool CDriverGL::activate()
{
	H_AUTO_OGL(CDriverGL_activate)

	if (_win == EmptyWindow)
		return false;

#ifdef NL_OS_WINDOWS

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

#endif // NL_OS_WINDOWS

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

// --------------------------------------------------
void CDriverGL::showCursor(bool b)
{
	H_AUTO_OGL(CDriverGL_showCursor)

	if (_win == EmptyWindow)
		return;

#ifdef NL_OS_WINDOWS

	if (b)
	{
		while (ShowCursor(b) < 0)
			;
	}
	else
	{
		while (ShowCursor(b) >= 0)
			;
	}

#elif defined(NL_OS_MAC)

	// Mac OS manages a show/hide counter for the cursor, so hiding the cursor
	// twice requires two calls to "show" to make the cursor visible again.
	// Since other platforms seem to not do this, the functionality is masked here
	// by only calling hide if the cursor is visible and only calling show if
	// the cursor was hidden.

	CGDisplayErr error  = kCGErrorSuccess;
	static bool visible = true;

	if(b && !visible)
	{
		error = CGDisplayShowCursor(kCGDirectMainDisplay);
		visible = true;
	}
	else if(!b && visible)
	{
		error = CGDisplayHideCursor(kCGDirectMainDisplay);
		visible = false;
	}

	if(error != kCGErrorSuccess)
		nlerror("cannot show / hide cursor");

#elif defined (NL_OS_UNIX)

	if (b)
	{
		if (_cursor != None)
		{
			XFreeCursor(_dpy, _cursor);
			_cursor = None;
		}
		XUndefineCursor(_dpy, _win);
	}
	else
	{
		if (_cursor == None)
		{
			char bm_no_data[] = { 0,0,0,0, 0,0,0,0 };
			Pixmap pixmap_no_data = XCreateBitmapFromData (_dpy, _win, bm_no_data, 8, 8);
			XColor black;
			memset(&black, 0, sizeof (XColor));
			black.flags = DoRed | DoGreen | DoBlue;
			_cursor = XCreatePixmapCursor (_dpy, pixmap_no_data, pixmap_no_data, &black, &black, 0, 0);
			XFreePixmap(_dpy, pixmap_no_data);
		}
		XDefineCursor(_dpy, _win, _cursor);
	}

#endif // NL_OS_UNIX
}

// --------------------------------------------------
void CDriverGL::setMousePos(float x, float y)
{
	H_AUTO_OGL(CDriverGL_setMousePos)

	if (_win == EmptyWindow)
		return;

	sint x1 = (sint)((float)_WindowWidth*x);
	sint y1 = (sint)((float)_WindowHeight*(1.0f-y));

#ifdef NL_OS_WINDOWS

	// NeL window coordinate to MSWindows coordinates
	POINT pt;
	pt.x = x1;
	pt.y = y1;
	ClientToScreen (_win, &pt);
	SetCursorPos(pt.x, pt.y);

#elif defined(NL_OS_MAC)

	// CG wants absolute coordinates related to first screen's top left

	// get the first screen's (conaints menubar) rect (this is not mainScreen)
	NSRect firstScreenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect;
	if([containerView() isInFullScreenMode])
		windowRect = [[[containerView() window] screen] frame];
	else
		windowRect = [[containerView() window] frame];

	// get the view's rect for height and width
	NSRect viewRect = [containerView() frame];

	// set the cursor position
	CGDisplayErr error = CGDisplayMoveCursorToPoint(
		kCGDirectMainDisplay, CGPointMake(
			windowRect.origin.x + (viewRect.size.width * x), 
			firstScreenRect.size.height - windowRect.origin.y - 
				viewRect.size.height + ((1.0 - y) * viewRect.size.height)));

	if(error != kCGErrorSuccess)
		nlerror("cannot set mouse position");

#elif defined (NL_OS_UNIX)

	XWarpPointer (_dpy, None, _win, None, None, None, None, x1, y1);

#endif // NL_OS_UNIX
}

void CDriverGL::getWindowSize(uint32 &width, uint32 &height)
{
	H_AUTO_OGL(CDriverGL_getWindowSize)

#ifdef NL_OS_MAC

	// TODO set them in windowproc, so no special impl is needed here 
	
	// A cocoa fullscreen view stays at the native resolution of the display.
	// When changing the rendering resolution, the size of the back buffer gets
	// changed, but the view still stays at full resolution. So the scaling of
	// the image from the rendered resolution to the view's resolution is done
	// by cocoa automatically while flushing buffers.
	// That's why, in fullscreen mode, return the resolution of the back buffer,
	// not the one from the window.

	// in fullscreen mode
	if([containerView() isInFullScreenMode])
	{
		// use the size stored in setWindowSize()
		width = _backBufferWidth;
		height = _backBufferHeight;
	}

	// in windowed mode
	else
	{
		// use the size of the view
		NSRect rect = [containerView() frame];
		width = rect.size.width;
		height = rect.size.height;
	}

#else // NL_OS_MAC

	// Off-screen rendering ?
	if (_OffScreen)
	{
#ifdef NL_OS_WINDOWS
		if (_PBuffer)
		{
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width );
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height );
		}
#endif
	}
	else
	{
		width = _WindowWidth;
		height = _WindowHeight;
	}

#endif // NL_OS_MAC
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
	if (!_FullScreen)
		flags |= SWP_NOMOVE;
	SetWindowPos(_win, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, flags);

	// init window width and height
	RECT clientRect;
	GetClientRect(_win, &clientRect);
	_WindowWidth = clientRect.right-clientRect.left;
	_WindowHeight = clientRect.bottom-clientRect.top;
	GetWindowRect(_win, &clientRect);
	_WindowX = clientRect.left;
	_WindowY = clientRect.top;

#elif defined(NL_OS_MAC)

	// for fullscreen mode, adjust the back buffer size to the desired resolution
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

		_backBufferWidth = width;
		_backBufferHeight = height;
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

	if (width != _WindowWidth || height != _WindowHeight)
	{
		// resize the window
		XResizeWindow(_dpy, _win, width, height);

		_WindowWidth = width;
		_WindowHeight = height;
	}

	// Update WM hints (allow resizing)
	XSizeHints size_hints;
	size_hints.flags = 0;

	if (!_Resizable || _FullScreen)
	{
		size_hints.flags |= PMinSize | PMaxSize;
		size_hints.min_width = width;
		size_hints.min_height = height;
		size_hints.max_width = width;
		size_hints.max_height = height;
	}

	XSetWMNormalHints(_dpy, _win, &size_hints);

#endif // NL_OS_WINDOWS
}

void CDriverGL::getWindowPos(sint32 &x, sint32 &y)
{
	H_AUTO_OGL(CDriverGL_getWindowPos)

#ifdef NL_OS_MAC
	// TODO set them in window proc so no special impl is needed here

	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [[containerView() window] frame];

	// simply return x
	x = windowRect.origin.x;

	// map y from cocoa to NeL coordinates before returning
	y = screenRect.size.height - windowRect.size.height - windowRect.origin.y;

#else // NL_OS_MAC

	// Off-screen rendering ?
	if (_OffScreen)
	{
		x = y = 0;
	}
	else
	{
		if (_win)
		{
			x = _WindowX;
			y = _WindowY;
		}
	}

#endif // NL_OS_MAC
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
# warning "OpenGL Driver: Missing Mac Implementation for isActive (always true if a window is set)"
#elif defined (NL_OS_UNIX)

#endif // NL_OS_UNIX

	return res;
}

void CDriverGL::setCapture (bool b)
{
	H_AUTO_OGL(CDriverGL_setCapture )

#ifdef NL_OS_WINDOWS

	if (b)
	{
		RECT client;
		GetClientRect (_win, &client);
		POINT pt1,pt2;
		pt1.x = client.left;
		pt1.y = client.top;
		ClientToScreen (_win, &pt1);
		pt2.x = client.right;
		pt2.y = client.bottom;
		ClientToScreen (_win, &pt2);
		client.bottom = pt2.y;
		client.top = pt1.y;
		client.left = pt1.x;
		client.right = pt2.x;
		ClipCursor (&client);
	}
	else
		ClipCursor (NULL);

	/*
	if (b)
		SetCapture (_hWnd);
	else
		ReleaseCapture ();
	*/

#elif defined(NL_OS_MAC)

	// no need to capture

#elif defined (NL_OS_UNIX)

	/*
		TODO x11 funtion: setCapture
	*/

	if(b) // capture the cursor.
	{
		XGrabPointer(_dpy, _win, True, 0, GrabModeAsync, GrabModeAsync, _win, None, CurrentTime);
	}
	else // release the cursor.
	{
		XUngrabPointer(_dpy, CurrentTime);
	}

#endif // NL_OS_UNIX
}

// ***************************************************************************
NLMISC::IMouseDevice* CDriverGL::enableLowLevelMouse(bool enable, bool exclusive)
{
	H_AUTO_OGL(CDriverGL_enableLowLevelMouse)

	NLMISC::IMouseDevice *res = NULL;

#ifdef NL_OS_WINDOWS

	NLMISC::CDIEventEmitter *diee = NULL;

	if (_EventEmitter.getNumEmitters() > 1)
		diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));

	if (enable)
	{
		try
		{
			if (diee)
				res = diee->getMouseDevice(exclusive);
		}
		catch (EDirectInput &)
		{
		}
	}
	else
	{
		if (diee)
			diee->releaseMouse();
	}

#elif defined(NL_OS_MAC)
#elif defined (NL_OS_UNIX)
#endif

	return res;
}

// ***************************************************************************
NLMISC::IKeyboardDevice* CDriverGL::enableLowLevelKeyboard(bool enable)
{
	H_AUTO_OGL(CDriverGL_enableLowLevelKeyboard)

	NLMISC::IKeyboardDevice *res = NULL;

#ifdef NL_OS_WINDOWS

	NLMISC::CDIEventEmitter *diee = NULL;

	if (_EventEmitter.getNumEmitters() > 1)
		diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));

	if (enable)
	{
		try
		{
			if (diee)
				res = diee->getKeyboardDevice();
		}
		catch (EDirectInput &)
		{
		}
	}
	else
	{
		if (diee)
			diee->releaseKeyboard();
	}

#elif defined(NL_OS_MAC)
#elif defined (NL_OS_UNIX)
#endif

	return res;
}

// ***************************************************************************
NLMISC::IInputDeviceManager* CDriverGL::getLowLevelInputDeviceManager()
{
	H_AUTO_OGL(CDriverGL_getLowLevelInputDeviceManager)

	NLMISC::IInputDeviceManager *res = NULL;

#ifdef NL_OS_WINDOWS

	if (_EventEmitter.getNumEmitters() > 1)
		res = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));

#elif defined(NL_OS_MAC)
#elif defined (NL_OS_UNIX)
#endif

	return res;
}

// ***************************************************************************
uint CDriverGL::getDoubleClickDelay(bool hardwareMouse)
{
	H_AUTO_OGL(CDriverGL_getDoubleClickDelay)

	uint res = 250;

#ifdef NL_OS_WINDOWS

	NLMISC::IMouseDevice *md = NULL;

	if (_EventEmitter.getNumEmitters() >= 2)
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));
		if (diee->isMouseCreated())
		{
			try
			{
				md = diee->getMouseDevice(hardwareMouse);
			}
			catch (EDirectInput &)
			{
				// could not get device ..
			}
		}
	}

	if (md)
	{
		res = md->getDoubleClickDelay();
	}
	else
	{
		// try to read the good value from windows
		res = ::GetDoubleClickTime();
	}

#elif defined(NL_OS_MAC)
# warning "OpenGL Driver: Missing Mac Implementation for getDoubleClickDelay"
	nlwarning("OpenGL Driver: Missing Mac Implementation for getDoubleClickDelay");

#elif defined (NL_OS_UNIX)

	// TODO for Linux

#endif

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

} // NL3D
