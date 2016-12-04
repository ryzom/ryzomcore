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

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
# include <X11/Xatom.h>
# ifdef HAVE_XRENDER
#  include <X11/extensions/Xrender.h>
# endif // HAVE_XRENDER
# ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
# endif // HAVE_XCURSOR
#endif // defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

#include "nel/3d/u_driver.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// *************************************************************************************
CDriverGL::CCursor::CCursor() : ColorDepth(CDriverGL::ColorDepth32),
								OrigHeight(32),
								HotspotScale(1.f),
								HotspotOffsetX(0),
								HotspotOffsetY(0),
								HotSpotX(0),
								HotSpotY(0),
								Cursor(EmptyCursor),
								Col(CRGBA::White),
								Rot(0)
{
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	Dpy = NULL;
#endif
}

// *************************************************************************************
CDriverGL::CCursor::~CCursor()
{
	reset();
}

// *************************************************************************************
void CDriverGL::CCursor::reset()
{
	if (Cursor != EmptyCursor)
	{
#ifdef NL_OS_WINDOWS
		DestroyIcon(Cursor);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
		XFreeCursor(Dpy, Cursor);
		XSync(Dpy, False);
#endif
	}
}

// *************************************************************************************
CDriverGL::CCursor& CDriverGL::CCursor::operator= (const CDriverGL::CCursor& from)
{
	if (&from == this)
		return *this;
	Src = from.Src; // requires more than a surface copy
	OrigHeight = from.OrigHeight;
	HotspotScale = from.HotspotScale;
	HotspotOffsetX = from.HotspotOffsetX;
	HotspotOffsetY = from.HotspotOffsetY;
	HotSpotX = from.HotSpotX;
	HotSpotY = from.HotSpotY;
	Cursor = from.Cursor;
	Col = from.Col;
	Rot = from.Rot;
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	Dpy = from.Dpy;
#endif
	return *this;
}

// *************************************************************************************
bool CDriverGL::isAlphaBlendedCursorSupported()
{
	if (!_AlphaBlendedCursorSupportRetrieved)
	{
#ifdef NL_OS_WINDOWS
		// Support starts with windows 2000 (not only from XP as seen in most docs)
		// NB : Additionnaly, could query D3D caps to know if
		// color hardware cursor is supported, not only emulated,
		// but can't be sure that using the win32 api 'SetCursor' uses the same resources
		// So far, seems to be supported on any modern card used by the game anyway ...
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&osvi))
		{
			_AlphaBlendedCursorSupported = (osvi.dwMajorVersion	>= 5);
		}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

		_AlphaBlendedCursorSupported = false;

#ifdef HAVE_XCURSOR
		if (!_AlphaBlendedCursorSupported && XcursorSupportsARGB(_dpy))
			_AlphaBlendedCursorSupported = true;
#endif // HAVE_XCURSOR

		if (!_AlphaBlendedCursorSupported && _xrender_version > 0)
			_AlphaBlendedCursorSupported = true;

#endif

		_AlphaBlendedCursorSupportRetrieved = true;
	}

	return _AlphaBlendedCursorSupported;
}

// *************************************************************************************
void CDriverGL::addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap)
{
	if (!isAlphaBlendedCursorSupported()) return;

	nlassert(cursorBitmap.getWidth() != 0);
	nlassert(cursorBitmap.getHeight() != 0);

	// find used part base on alpha, to avoid too much shrinking
	const CRGBA *pixels = (const CRGBA *) &cursorBitmap.getPixels()[0];
	uint minX, maxX, minY, maxY;
	uint width = cursorBitmap.getWidth();
	uint height = cursorBitmap.getHeight();
	//
	minX = 0;
	for (uint x = 0; x < width; ++x)
	{
		bool stop = false;
		minX = x;
		for (uint y = 0; y < height; ++y)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}
	//
	maxX = width - 1;
	for (sint x = width - 1; x >= 0; --x)
	{
		bool stop = false;
		maxX = (uint) x;
		for (uint y = 0; y < height; ++y)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}
	//
	minY = 0;
	for (uint y = 0; y < height; ++y)
	{
		bool stop = false;
		minY = y;
		for (uint x = 0; x < width; ++x)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}
	//
	maxY = height - 1;
	for (sint y = height - 1; y >= 0; --y)
	{
		bool stop = false;
		maxY = (uint) y;
		for (uint x = 0; x < width; ++x)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}
	//
	CCursor &curs = _Cursors[name];
	curs = CCursor(); // erase possible previous cursor

	uint destWidth = 32, destHeight = 32;
	getBestCursorSize(width, height, destWidth, destHeight);

	// build a square bitmap
	uint tmpSize = std::max(maxX - minX + 1, maxY - minY + 1);
	curs.Src.resize(tmpSize, tmpSize);
	// blit at top left corner
	curs.Src.blit(cursorBitmap, minX, minY, maxX - minX + 1, maxY - minY + 1, 0, 0);

	curs.OrigHeight = cursorBitmap.getHeight();
	curs.HotspotOffsetX = minX;
	curs.HotspotOffsetY = minY;
	//
	curs.HotspotScale = _CursorScale;
	clamp(curs.HotspotScale, 0.f, 1.f);
	// first resampling, same for all cursors
	tmpSize = (uint) (tmpSize * curs.HotspotScale);
	if (tmpSize == 0) tmpSize = 1;

	if (curs.HotspotScale < 1.f)
	{
		curs.Src.resample(tmpSize, tmpSize);
	}

	// shrink if necessary
	if (tmpSize > destWidth || tmpSize > destHeight) // need to shrink ?
	{
		// constraint proportions
		curs.HotspotScale *= std::min(float(destWidth) / tmpSize, float(destHeight) / tmpSize);
		curs.Src.resample(destWidth, destHeight);
	}
	else
	{
		CBitmap final;
		final.resize(destWidth, destHeight);
		final.blit(&curs.Src, 0, 0);
		curs.Src.swap(final);
	}

	if (name == _CurrName)
	{
		updateCursor();
	}
}

// *************************************************************************************
void CDriverGL::createCursors()
{
#ifdef NL_OS_WINDOWS
	_DefaultCursor = LoadCursor(NULL, IDC_ARROW);
	_BlankCursor = NULL;
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	_DefaultCursor = None;

	if (_dpy && _win && _BlankCursor == EmptyCursor)
	{
		// create blank cursor
		char bm_no_data[] = { 0,0,0,0,0,0,0,0 };
		Pixmap pixmap_no_data = XCreateBitmapFromData (_dpy, _win, bm_no_data, 8, 8);
		XColor black;
		memset(&black, 0, sizeof (XColor));
		black.flags = DoRed | DoGreen | DoBlue;
		_BlankCursor = XCreatePixmapCursor (_dpy, pixmap_no_data, pixmap_no_data, &black, &black, 0, 0);
		XFreePixmap(_dpy, pixmap_no_data);
	}
#endif
}

// *************************************************************************************
void CDriverGL::releaseCursors()
{
#ifdef NL_OS_WINDOWS
	SetClassLongPtr(_win, GCLP_HCURSOR, 0);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	XUndefineCursor(_dpy, _win);
	XFreeCursor(_dpy, _BlankCursor);
#endif

	_Cursors.clear();
}

// *************************************************************************************
void CDriverGL::updateCursor(bool forceRebuild)
{
	setCursor(_CurrName, _CurrCol, _CurrRot, _CurrHotSpotX, _CurrHotSpotY, forceRebuild);
}

// *************************************************************************************
void CDriverGL::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
{
	// don't update cursor if it's hidden or if custom cursors are not suppported
	if (!isAlphaBlendedCursorSupported() || _CurrName == "none") return;

	_CurrName = name;
	_CurrCol = col;
	_CurrRot = rot;
	_CurrHotSpotX = hotSpotX;
	_CurrHotSpotY = hotSpotY;

	// cursor has to be changed next time
	if (_CurrName.empty()) return;

	if (rot >	3) rot = 3; // same than 'CViewRenderer::drawRotFlipBitmapTiled

	TCursorMap::iterator it = _Cursors.find(name);

	nlCursor cursorHandle = _DefaultCursor;

	if (it != _Cursors.end())
	{
		// Update cursor if modified or not already built
		CCursor &curs = it->second;
		hotSpotX = (sint) (curs.HotspotScale * (hotSpotX - curs.HotspotOffsetX));
		hotSpotY = (sint) (curs.HotspotScale * ((curs.OrigHeight - hotSpotY) - curs.HotspotOffsetY));
		if (curs.Cursor == EmptyCursor ||
			curs.HotSpotX != hotSpotX ||
			curs.HotSpotY != hotSpotY ||
			curs.Col != col ||
			curs.Rot != rot ||
			curs.ColorDepth != _ColorDepth ||
			forceRebuild
		   )
		{
			curs.reset();
			curs.Cursor = buildCursor(curs.Src, col, rot, hotSpotX, hotSpotY);
			curs.Col = col;
			curs.Rot = rot;
			curs.HotSpotX = hotSpotX;
			curs.HotSpotY = hotSpotY;
			curs.ColorDepth = _ColorDepth;
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
			curs.Dpy = _dpy;
#endif
		}
		cursorHandle = curs.Cursor ? curs.Cursor : _DefaultCursor;
	}

	if (isSystemCursorInClientArea() || isSystemCursorCaptured() || forceRebuild)
	{
//		if (CInputHandlerManager::getInstance()->hasFocus())
#ifdef NL_OS_WINDOWS
		{
			::SetCursor(cursorHandle);
			SetClassLongPtr(_win, GCLP_HCURSOR, (LONG_PTR) cursorHandle); // set default mouse icon to the last one
		}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
		if (cursorHandle == _DefaultCursor)
		{
			XUndefineCursor(_dpy, _win);
		}
		else
		{
			XDefineCursor(_dpy, _win, cursorHandle);
		}
#endif
	}

}

// *************************************************************************************
void CDriverGL::setCursorScale(float scale)
{
	_CursorScale = scale;
}

// *************************************************************************************
nlCursor CDriverGL::buildCursor(const CBitmap &src, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY)
{
	nlassert(isAlphaBlendedCursorSupported());

	uint mouseW = 32, mouseH = 32;
	getBestCursorSize(src.getWidth(), src.getHeight(), mouseW, mouseH);

	CBitmap rotSrc = src;
	if (rot > 3) rot = 3; // mimic behavior of 'CViewRenderer::drawRotFlipBitmapTiled' (why not rot & 3 ??? ...)
	switch(rot)
	{
		case 0: break;
		case 1: rotSrc.rot90CW(); break;
		case 2: rotSrc.rot90CW(); rotSrc.rot90CW(); break;
		case 3: rotSrc.rot90CCW(); break;
	}

	// create a cursor from bitmap
	nlCursor result = EmptyCursor;
	convertBitmapToCursor(rotSrc, result, mouseW, mouseH, _ColorDepth == ColorDepth16 ? 16:32, col, hotSpotX, hotSpotY);
	return result;
}


// *************************************************************************************
void CDriverGL::setSystemArrow()
{
	H_AUTO_OGL(CDriverGL_setSystemArrow);

#ifdef NL_OS_WINDOWS
	if (isSystemCursorInClientArea() || isSystemCursorCaptured())
	{
		SetCursor(_DefaultCursor);
	}

	// set default mouse icon to the default one
	SetClassLongPtr(_win, GCLP_HCURSOR, (LONG_PTR) _DefaultCursor);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	XUndefineCursor(_dpy, _win);
#endif
}

// ***************************************************************************
void CDriverGL::showCursor(bool b)
{
	H_AUTO_OGL(CDriverGL_showCursor);

	if (_win == EmptyWindow)
		return;

#ifdef NL_OS_WINDOWS

	if (b)
	{
		// update current hardware icon to avoid to have the plain arrow
		updateCursor(true);

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

	if (!b)
	{
		XDefineCursor(_dpy, _win, _BlankCursor);
		_CurrName = "none";
	}
	else
	{
		_CurrName.clear();
	}

	// update current hardware icon to avoid to have the plain arrow
	updateCursor(true);

#endif // NL_OS_UNIX
}

// ***************************************************************************
void CDriverGL::setMousePos(float x, float y)
{
	H_AUTO_OGL(CDriverGL_setMousePos)

	if (_win == EmptyWindow)
		return;

	sint x1 = (sint)((float)_CurrentMode.Width*x);
	sint y1 = (sint)((float)_CurrentMode.Height*(1.0f-y));

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

// ***************************************************************************
void CDriverGL::setCapture (bool b)
{
	H_AUTO_OGL(CDriverGL_setCapture);

#ifdef NL_OS_WINDOWS

	if (b && isSystemCursorInClientArea() && !isSystemCursorCaptured())
	{
		SetCapture(_win);
	}
	else if (!b && isSystemCursorCaptured())
	{
		// if hardware mouse and not in client area, then force to update its aspect by updating its pos
		if (!isSystemCursorInClientArea())
		{
			// force update
			showCursor(true);
		}

		ReleaseCapture();
	}

#elif defined(NL_OS_MAC)

	// no need to capture
	_MouseCaptured = b;

#elif defined (NL_OS_UNIX)

	if(b /* && isSystemCursorInClientArea() && !isSystemCursorCaptured()*/) // capture the cursor.
	{
		// capture the cursor
		XGrabPointer(_dpy, _win, True, 0, GrabModeAsync, GrabModeAsync, _win, None, CurrentTime);
		_MouseCaptured = true;
	}
	else if (!b/* && isSystemCursorCaptured()*/)
	{
		// release the cursor
		XUngrabPointer(_dpy, CurrentTime);
		_MouseCaptured = false;
	}

#endif // NL_OS_UNIX
}

// ***************************************************************************
bool CDriverGL::isSystemCursorInClientArea()
{
	if (!_CurrentMode.Windowed)
	{
#ifdef NL_OS_WINDOWS
		return IsWindowVisible(_win) != FALSE;
#endif
	}
	else
	{
#ifdef NL_OS_WINDOWS
		POINT cursPos;
		// the mouse should be in the client area of the window
		if (!GetCursorPos(&cursPos))
		{
			return false;
		}
		HWND wnd = WindowFromPoint(cursPos);
		if (wnd != _win)
		{
			return false; // not the same window
		}
		// want that the mouse be in the client area
		RECT clientRect;
		if (!GetClientRect(_win, &clientRect))
		{
			return false;
		}
		POINT tl, br;
		tl.x = clientRect.left;
		tl.y = clientRect.top;
		br.x = clientRect.right;
		br.y = clientRect.bottom;
		if (!ClientToScreen(_win, &tl))
		{
			return false;
		}
		if (!ClientToScreen(_win, &br))
		{
			return false;
		}
		if ((cursPos.x < tl.x) || (cursPos.x >= br.x) || (cursPos.y < tl.y) || (cursPos.y >= br.y))
		{
			return false;
		}
#endif
	}

	return true;
}

// ***************************************************************************
bool CDriverGL::isSystemCursorCaptured()
{
	H_AUTO_OGL(CDriverGL_isSystemCursorCaptured);

#ifdef NL_OS_WINDOWS
	return GetCapture() == _win;
#else
	return _MouseCaptured;
#endif
}

bool CDriverGL::getBestCursorSize(uint srcWidth, uint srcHeight, uint &dstWidth, uint &dstHeight)
{
#ifdef NL_OS_WINDOWS

	// Windows provides default size for cursors
	dstWidth = (uint)GetSystemMetrics(SM_CXCURSOR);
	dstHeight = (uint)GetSystemMetrics(SM_CYCURSOR);

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

	Status status = XQueryBestCursor(_dpy, _win, srcWidth, srcHeight, &dstWidth, &dstHeight);

	if (!status)
	{
		nlwarning("XQueryBestCursor failed");
	}

#endif

	return true;
}

bool CDriverGL::convertBitmapToCursor(const NLMISC::CBitmap &bitmap, nlCursor &cursor, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY)
{
#if defined(NL_OS_WINDOWS)

	return convertBitmapToIcon(bitmap, cursor, iconWidth, iconHeight, iconDepth, col, hotSpotX, hotSpotY, true);

#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)

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

	do
	{
		// colorize icon
		destColorPtr->modulateFromColor(*srcColorPtr, col);

		// X11 wants BGRA pixels : swap red and blue channels
		std::swap(destColorPtr->R, destColorPtr->B);

		// premultiplied alpha
		if (destColorPtr->A < 255)
		{
			destColorPtr->R = (destColorPtr->R * destColorPtr->A) / 255;
			destColorPtr->G = (destColorPtr->G * destColorPtr->A) / 255;
			destColorPtr->B = (destColorPtr->B * destColorPtr->A) / 255;
		}

		++ srcColorPtr;
		++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);

#ifdef HAVE_XCURSOR

	if (XcursorSupportsARGB(_dpy))
	{
		XcursorImage *image = XcursorImageCreate(iconWidth, iconHeight);

		if (!image)
		{
			nlwarning("Failed to create a XcusorImage with size %ux%u", iconWidth, iconHeight);
		}
		else
		{
			image->xhot = (uint)hotSpotX;
			image->yhot = (uint)hotSpotY;

			memcpy(image->pixels, &colorBm.getPixels(0)[0], colorBm.getSize()*4);

			cursor = XcursorImageLoadCursor(_dpy, image);

			XcursorImageDestroy(image);
		}
	}

#endif // HAVE_XCURSOR

#ifdef HAVE_XRENDER

	if (_xrender_version > 0)
	{
		// use malloc() because X will free() data itself
		CRGBA *src32 = (CRGBA*)malloc(colorBm.getSize()*4);
		memcpy(src32, &colorBm.getPixels(0)[0], colorBm.getSize()*4);

		uint size = iconWidth * iconHeight;

		sint screen = DefaultScreen(_dpy);
		Visual *visual = DefaultVisual(_dpy, screen);

		if (!visual)
		{
			nlwarning("Failed to get a default visual for screen %d", screen);
			return false;
		}

		// Create the icon image
		XImage* image = XCreateImage(_dpy, visual, 32, ZPixmap, 0, (char*)src32, iconWidth, iconHeight, 32, 0);

		if (!image)
		{
			nlwarning("Failed to set the window's icon");
			return false;
		}

		// Create the icon pixmap
		Pixmap pixmap = XCreatePixmap(_dpy, _win, iconWidth, iconHeight, 32 /* defDepth */);

		if (!pixmap)
		{
			nlwarning("Failed to create a pixmap %ux%ux%d", iconWidth, iconHeight, 32);
			return false;
		}

		// Create the icon graphic contest
		GC gc = XCreateGC(_dpy, pixmap, 0, NULL);

		if (!gc)
		{
			nlwarning("Failed to create a GC");
			return false;
		}

		sint res = XPutImage(_dpy, pixmap, gc, image, 0, 0, 0, 0, iconWidth, iconHeight);

		if (res)
		{
			nlwarning("XPutImage failed with code %d", res);
		}

		if (!XFreeGC(_dpy, gc))
		{
			nlwarning("XFreeGC failed");
		}

		if (image->data)
		{
			free(image->data);
			image->data = NULL;
		}

		XDestroyImage(image);

		XRenderPictFormat *format = XRenderFindStandardFormat(_dpy, PictStandardARGB32);

		if (!format)
		{
			nlwarning("Failed to find a standard format");
			return false;
		}

		Picture picture = XRenderCreatePicture(_dpy, pixmap, format, 0, 0);

		if (!picture)
		{
			nlwarning("Failed to create picture");
			return false;
		}

		cursor = XRenderCreateCursor(_dpy, picture, (uint)hotSpotX, (uint)hotSpotY);

		if (!cursor)
		{
			nlwarning("Failed to create cursor");
			return false;
		}

		XRenderFreePicture(_dpy, picture);

		if (!XFreePixmap(_dpy, pixmap))
		{
			nlwarning("XFreePixmap failed");
		}

		return true;
	}

#endif // HAVE_XRENDER

	return false;

#else

	return false;

#endif
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
