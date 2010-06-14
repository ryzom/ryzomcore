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
#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# define GL_GLEXT_LEGACY
# include <OpenGL/gl.h>
# include "mac/glext.h"
# include "mac/cocoa_adapter.h"
#elif defined (NL_OS_UNIX)
# include <GL/gl.h>
# include <GL/glx.h>
#endif // NL_OS_UNIX

#include "nel/misc/mouse_device.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/3d/u_driver.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

#ifdef NL_OS_WINDOWS

static bool GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_OGL(GlWndProc)
	if(message == WM_SIZE)
	{
		if (driver != NULL)
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
		if (driver != NULL)
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

#ifdef NL_DISABLE_MENU
	// disable menu (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
	if(message == WM_SYSCOMMAND && wParam == SC_KEYMENU)
		return 0;
#endif // NL_DISABLE_MENU

	// disable menu (default ALT-F4 behavior is disabled)
	if(message == WM_CLOSE)
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
#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	return NL3D::MAC::init(windowIcon, exitFunc);

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::release();

#elif defined (NL_OS_UNIX)

	XCloseDisplay(_dpy);
	_dpy = NULL;

#endif // NL_OS_UNIX

	return true;
}

// --------------------------------------------------
bool CDriverGL::setDisplay(nlWindow wnd, const GfxMode &mode, bool show, bool resizeable) throw(EBadDisplay)
{
	H_AUTO_OGL(CDriverGL_setDisplay)

	_win = EmptyWindow;
	_WindowWidth = _WindowHeight = _WindowX = _WindowY = 0;
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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::setDisplay(wnd, mode, show, resizeable);

#elif defined (NL_OS_UNIX)

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

	XSelectInput (_dpy, _win, KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask);

//	XMapWindow(_dpy, _win);

	_EventEmitter.init (_dpy, _win);

//	XEvent event;
//	XIfEvent(dpy, &event, WaitForNotify, (char *)this);

#endif // NL_OS_UNIX

	// setup window size and screen mode
	if (!setMode(mode))
		return false;

	// setup OpenGL structures
	if (!setupDisplay())
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

#elif defined(NL_OS_UNIX)

#if defined(XF86VIDMODE)

	// Store old mode in order to restore it when leaving fullscreen
	memset(&_OldScreenMode, 0, sizeof(XF86VidModeModeLine));
	XF86VidModeGetModeLine(_dpy, DefaultScreen(_dpy), &_OldDotClock, &_OldScreenMode);
	res = XF86VidModeGetViewPort(_dpy, DefaultScreen(_dpy), &_OldX, &_OldY);

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

#elif defined(NL_OS_UNIX)

#if defined(XF86VIDMODE)

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
	XF86VidModeSwitchToMode(_dpy, DefaultScreen(_dpy), &info);
	nlinfo("3D: Switching back viewport to %d,%d",_OldX, _OldY);
	res = XF86VidModeSetViewPort(_dpy, DefaultScreen(_dpy), _OldX, _OldY);

#endif // XF86VIDMODE

#endif // NL_OS_WINDOWS

	return res;
}

// --------------------------------------------------
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
		nlwarning("Fullscreen mode switch failed");
		return false;
	}

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	// TODO

#elif defined(NL_OS_UNIX)

#if defined(XF86VIDMODE)

	bool found = false;

	// Find the requested mode and use it
	XF86VidModeModeInfo **modes;
	int nmodes;
	if (XF86VidModeGetAllModeLines(_dpy, DefaultScreen(_dpy), &nmodes, &modes))
	{
		for (int i = 0; i < nmodes; i++)
		{
			nldebug("3D: Available mode - %dx%d", modes[i]->hdisplay, modes[i]->vdisplay);
			if (modes[i]->hdisplay == mode.Width && modes[i]->vdisplay == mode.Height)
			{
				if (XF86VidModeSwitchToMode(_dpy, DefaultScreen(_dpy), modes[i]))
				{
					nlinfo("3D: Switching to mode %dx%d", modes[i]->hdisplay, modes[i]->vdisplay);
					XF86VidModeSetViewPort(_dpy, DefaultScreen(_dpy), 0, 0);
					found = true;
				}
				break;
			}
		}
		XFree(modes);
	}

	if (!found)
		return false;

#endif // XF86VIDMODE

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	// TODO

#elif defined (NL_OS_UNIX)

	if (_visual_info == NULL)
		return false;

	nlWindow root = RootWindow(_dpy, DefaultScreen(_dpy));

	XSetWindowAttributes attr;
	attr.background_pixel = BlackPixel(_dpy, DefaultScreen(_dpy));
	attr.colormap = XCreateColormap(_dpy, root, _visual_info->visual, AllocNone);

#ifdef XF86VIDMODE
	// If we're going to attempt fullscreen, we need to set redirect to True,
	// This basically places the window with no borders in the top left
	// corner of the screen.
	if (mode.Windowed)
	{
		attr.override_redirect = False;
	}
	else
	{
		attr.override_redirect = True;
	}
#else
	attr.override_redirect = False;
#endif

	int attr_flags = CWOverrideRedirect | CWBackPixel | CWColormap;

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

#ifdef NL_OS_WINDOWS

	// Then delete.
	// wglMakeCurrent(NULL,NULL);

	if (_hDC)
		wglMakeCurrent(_hDC, NULL);

	if (_hRC)
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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	// TODO

#elif defined (NL_OS_UNIX)

	if (_ctx)
	{
		glXDestroyContext(_dpy, _ctx);
		_ctx = NULL;
	}

	if (_win && _DestroyWindow)
		XDestroyWindow(_dpy, _win);

	// Ungrab the keyboard (probably not necessary);
//	XUnmapWindow(_dpy, _win);
	XSync(_dpy, True);
	XUngrabKeyboard(_dpy, CurrentTime);

#endif

	_win = EmptyWindow;

	return true;
}

CDriverGL::EWindowStyle CDriverGL::getWindowStyle() const
{
	H_AUTO_OGL(CDriverGL_getWindowStyle)

	if (_FullScreen)
		return EWSFullscreen;

	return EWSWindowed;
}

bool CDriverGL::setWindowStyle(EWindowStyle windowStyle)
{
	H_AUTO_OGL(CDriverGL_setWindowStyle)

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

#elif defined(NL_OS_UNIX)

	// x11 fullscreen is not working on mac os x
#if !defined(NL_OS_MAC)
	// Toggle fullscreen
	if (windowStyle != getWindowStyle())
	{
		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = _win;
		xev.xclient.message_type = XInternAtom(_dpy, "_NET_WM_STATE", False);
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = windowStyle == EWSFullscreen ? 1:0;
		xev.xclient.data.l[1] = XInternAtom(_dpy, "_NET_WM_STATE_FULLSCREEN", False);
		xev.xclient.data.l[2] = 0;
		XSendEvent(_dpy, DefaultRootWindow(_dpy), False, SubstructureNotifyMask, &xev);
	}

#endif

#endif // NL_OS_WINDOWS

	return true;
}

// --------------------------------------------------
bool CDriverGL::setMode(const GfxMode& mode)
{
	H_AUTO_OGL(CDriverGL_setMode)

	if (!setScreenMode(mode))
		return false;

	// when changing window style, it's possible system change window size too
	setWindowStyle(mode.Windowed ? EWSWindowed:EWSFullscreen);

	_WindowWidth = mode.Width;
	_WindowHeight = mode.Height;

	if (!mode.Windowed)
		_Depth = mode.Depth;

	_FullScreen = !mode.Windowed;

#if defined(NL_OS_MAC)

#if defined(NL_MAC_NATIVE)
	NL3D::MAC::setMode(mode);
#else
	// X11 under Mac OS can't use fullscreen
	_FullScreen = false;
#endif // NL_MAC_NATIVE

#endif // NL_OS_MAC

	setWindowSize(mode.Width, mode.Height);

	return true;
}

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
	getMacModes(modes);
#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

#elif defined (NL_OS_UNIX)

#	ifdef XF86VIDMODE
	int nmodes;
	XF86VidModeModeInfo **ms;
	Bool ok = XF86VidModeGetAllModeLines(_dpy, DefaultScreen(_dpy), &nmodes, &ms);
	if(ok)
	{
		nldebug("3D: %d available modes:", nmodes);
		for (int j = 0; j < nmodes; j++)
		{
			// Add this mode
			GfxMode mode;
			mode.Width = (uint16)ms[j]->hdisplay;
			mode.Height = (uint16)ms[j]->vdisplay;
			mode.Frequency = 1000 * ms[j]->dotclock / (ms[j]->htotal * ms[j]->vtotal);
			nldebug("3D:   Mode %d: %dx%d, %d Hz", j, ms[j]->hdisplay,ms[j]->vdisplay, 1000 * ms[j]->dotclock / (ms[j]->htotal * ms[j]->vtotal));
			modes.push_back (mode);
		}
		XFree(ms);
	}
	else
	{
		nlwarning("XF86VidModeGetAllModeLines returns 0, cannot get available video mode");
		return false;
	}
#	endif

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
	
#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::getCurrentScreenMode(mode);

#elif defined(NL_OS_MAC)
	/*
		TODO this is just a hack to get the ryzom client running on mac os x x11.
			the implementation below relies on the vidmode extension which is not
			availeble on mac os x's x11. for that reason the color depth value is 
			hard coded here.
		FIXME replace this hack by native cocoa color depth retrieval
	*/
	nlwarning("FIXME: returning hardcoded color depth of 24bit");
	mode.Depth= 24;

#elif defined(NL_OS_UNIX)

#	ifdef XF86VIDMODE
	sint pixelClock;
	XF86VidModeModeLine xmode;

	if (!XF86VidModeGetModeLine(_dpy, DefaultScreen(_dpy), &pixelClock, &xmode))
	{
		nlwarning("XF86VidModeGetModeLine returns 0, cannot get current video mode");
		return false;
	}

	// x11 fullscreen is not working on mac os x
#if !defined(NL_OS_MAC)
	mode.Windowed = !_FullScreen;
#else
	mode.Windowed = true;
#endif

	mode.OffScreen = false;
	mode.Depth = (uint) DefaultDepth(_dpy, DefaultScreen(_dpy));
	mode.Frequency = 1000 * pixelClock / (xmode.htotal * xmode.vtotal) ;
	mode.Width = xmode.hdisplay;
	mode.Height = xmode.vdisplay;

	nldebug("Current mode : %dx%d, %d Hz, %dbit", mode.Width, mode.Height, mode.Frequency, mode.Depth);
#	endif

#endif
	return true;
}

// --------------------------------------------------
void CDriverGL::setWindowTitle(const ucstring &title)
{
#ifdef NL_OS_WINDOWS

	SetWindowTextW(_win, (WCHAR*)title.c_str());

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::setWindowTitle(title);

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
	_WindowX = x;
	_WindowY = y;

#ifdef NL_OS_WINDOWS

	SetWindowPos(_win, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::setWindowPos(x, y);

#elif defined (NL_OS_UNIX)

	XMoveWindow(_dpy, _win, x, y);

#endif // NL_OS_WINDOWS
}

// ***************************************************************************
void CDriverGL::showWindow(bool show)
{
#ifdef NL_OS_WINDOWS
	ShowWindow (_win, show ? SW_SHOW:SW_HIDE);
#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

#elif defined (NL_OS_UNIX)

	if (show)
	{
		XMapWindow(_dpy, _win);
//		XMapRaised(_dpy, _win);
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
#ifdef NL_OS_WINDOWS
	return (emptyProc)GlWndProc;
#else // NL_OS_WINDOWS
	return NULL;
#endif // NL_OS_WINDOWS
}

// --------------------------------------------------
bool CDriverGL::activate()
{
	H_AUTO_OGL(CDriverGL_activate)
#ifdef NL_OS_WINDOWS

	HGLRC hglrc = wglGetCurrentContext();

	if (hglrc != _hRC)
		wglMakeCurrent(_hDC, _hRC);

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

# warning "OpenGL Driver: Temporary Mac Implementation"
	nlwarning("OpenGL Driver: Temporary Mac Implementation");

	// already done in setDisplay, not needed here - unclean! FIXME

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::showCursor(b);

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

#ifdef NL_OS_WINDOWS

	if (_win)
	{
		// NeL window coordinate to MSWindows coordinates
		POINT pt;
		pt.x = (sint)((float)(_WindowWidth)*x);
		pt.y = (sint)((float)(_WindowHeight)*(1.0f-y));
		ClientToScreen (_win, &pt);
		SetCursorPos(pt.x, pt.y);
	}

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::setMousePos(x, y);

#elif defined (NL_OS_UNIX)

	XWindowAttributes xwa;
	XGetWindowAttributes (_dpy, _win, &xwa);
	int x1 = (int)(x * (float) xwa.width);
	int y1 = (int)((1.0f - y) * (float) xwa.height);
	XWarpPointer (_dpy, None, _win, None, None, None, None, x1, y1);

#endif // NL_OS_UNIX
}

void CDriverGL::getWindowSize(uint32 &width, uint32 &height)
{
	H_AUTO_OGL(CDriverGL_getWindowSize)

#ifdef NL_OS_WINDOWS

	// Off-screen rendering ?
	if (_OffScreen)
	{
		if (_PBuffer)
		{
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&width );
			nwglQueryPbufferARB( _PBuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&height );
		}
	}
	else
	{
		if (_win)
		{
			width = (uint32)_WindowWidth;
			height = (uint32)_WindowHeight;
		}
	}

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::getWindowSize(width, height);

#elif defined (NL_OS_UNIX)

	XWindowAttributes xwa;
	XGetWindowAttributes (_dpy, _win, &xwa);
	width = (uint32) xwa.width;
	height = (uint32) xwa.height;

#endif // NL_OS_UNIX
}

void CDriverGL::setWindowSize(uint32 width, uint32 height)
{
	H_AUTO_OGL(CDriverGL_setWindowSize)

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

#elif defined(NL_OS_UNIX) && !defined(NL_MAC_NATIVE)

	// set position to (0, 0) if fullscreen
	if (_FullScreen)
	{
		// move and resize the window
		XMoveResizeWindow(_dpy, _win, 0, 0, width, height);
	}
	else
	{
		// resize the window
		XResizeWindow(_dpy, _win, width, height);
	}

	if (!_Resizable)
	{
		// Update WM hints (update size and allow resizing)
		XSizeHints size_hints;

		size_hints.flags = PMinSize | PMaxSize;
		size_hints.min_width = width;
		size_hints.min_height = height;
		size_hints.max_width = width;
		size_hints.max_height = height;

		XSetWMNormalHints(_dpy, _win, &size_hints);
	}

//	XMapWindow(_dpy, _win);

	_WindowWidth = width;
	_WindowHeight = height;

#endif // NL_OS_WINDOWS
}

void CDriverGL::getWindowPos(sint32 &x, sint32 &y)
{
	H_AUTO_OGL(CDriverGL_getWindowPos)

#ifdef NL_OS_WINDOWS

	// Off-screen rendering ?
	if (_OffScreen)
	{
		if (_PBuffer)
		{
			x = y = 0;
		}
	}
	else
	{
		if (_win)
		{
			x = _WindowX;
			y = _WindowY;
		}
	}

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::getWindowPos(x, y);

#elif defined (NL_OS_UNIX)

	int screen = DefaultScreen(_dpy);

#if 0
	// Display size is a member of display structure
	int display_width = DisplayWidth(_dpy, screen);
	int display_height = DisplayHeight(_dpy, screen);
#endif

	int xtmp = 0, ytmp = 0;
	unsigned int width = 0, height = 0;
	unsigned int border_width = 0;
	unsigned int depth = 0;

	// Get geometry information about root window
	if (!XGetGeometry(_dpy, RootWindow(_dpy, screen), (Window*)&_win, &xtmp, &ytmp, &width, &height, &border_width, &depth))
	{
		nlwarning("can't get root window geometry");
	}

	x = xtmp;
	y = ytmp;

#endif // NL_OS_UNIX
}

// --------------------------------------------------
bool CDriverGL::isActive()
{
	H_AUTO_OGL(CDriverGL_isActive)

	bool res = true;

#ifdef NL_OS_WINDOWS

	res = (IsWindow(_win) != FALSE);

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	// nlwarning("OpenGL Driver: Missing Mac Implementation");

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)

	NL3D::MAC::setCapture(b);

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

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

#elif defined(NL_OS_MAC) && defined(NL_MAC_NATIVE)
# warning "OpenGL Driver: Missing Mac Implementation"
	nlwarning("OpenGL Driver: Missing Mac Implementation");

#elif defined (NL_OS_UNIX)

	// TODO for Linux: implement CDriverGL::setMonitorColorProperties
	nlwarning ("CDriverGL::setMonitorColorProperties not implemented");

#endif

	return false;
}

} // NL3D
