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

#include "stddirect3d.h"
#include "driver_direct3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// *************************************************************************************
CDriverD3D::CCursor::CCursor() : ColorDepth(CDriverD3D::ColorDepth32),
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
}

// *************************************************************************************
CDriverD3D::CCursor::~CCursor()
{
	reset();
}

// *************************************************************************************
void CDriverD3D::CCursor::reset()
{
	if (Cursor != EmptyCursor)
	{
		DestroyIcon(Cursor);
	}
}

// *************************************************************************************
CDriverD3D::CCursor& CDriverD3D::CCursor::operator= (const CDriverD3D::CCursor& from)
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
	return *this;
}

// *************************************************************************************
bool CDriverD3D::isAlphaBlendedCursorSupported()
{
	if (!_AlphaBlendedCursorSupportRetrieved)
	{
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

		_AlphaBlendedCursorSupportRetrieved = true;
	}

	return _AlphaBlendedCursorSupported;
}

// *************************************************************************************
void CDriverD3D::addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap)
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

	uint destWidth = GetSystemMetrics(SM_CXCURSOR);
	uint destHeight = GetSystemMetrics(SM_CYCURSOR);

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
void CDriverD3D::createCursors()
{
	_DefaultCursor = LoadCursor(NULL, IDC_ARROW);
}

// *************************************************************************************
void CDriverD3D::releaseCursors()
{
	SetClassLongPtr(_HWnd, GCLP_HCURSOR, 0);

	_Cursors.clear();
}

// *************************************************************************************
void CDriverD3D::updateCursor(bool forceRebuild)
{
	setCursor(_CurrName, _CurrCol, _CurrRot, _CurrHotSpotX, _CurrHotSpotY, forceRebuild);
}

// *************************************************************************************
void CDriverD3D::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
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
		}
		cursorHandle = curs.Cursor ? curs.Cursor : _DefaultCursor;
	}

	if (isSystemCursorInClientArea() || isSystemCursorCaptured() || forceRebuild)
	{
//		if (CInputHandlerManager::getInstance()->hasFocus())
		{
			::SetCursor(cursorHandle);
			SetClassLongPtr(_HWnd, GCLP_HCURSOR, (LONG_PTR) cursorHandle); // set default mouse icon to the last one
		}
	}

}

// *************************************************************************************
void CDriverD3D::setCursorScale(float scale)
{
	_CursorScale = scale;
}

// *************************************************************************************
nlCursor CDriverD3D::buildCursor(const CBitmap &src, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY)
{
	nlassert(isAlphaBlendedCursorSupported());

	uint mouseW = GetSystemMetrics(SM_CXCURSOR);
	uint mouseH = GetSystemMetrics(SM_CYCURSOR);

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
	nlCursor result = NULL;
	convertBitmapToCursor(rotSrc, result, mouseW, mouseH, _ColorDepth == ColorDepth16 ? 16:32, col, hotSpotX, hotSpotY);
	return result;
}


// *************************************************************************************
void CDriverD3D::setSystemArrow()
{
	H_AUTO_D3D(CDriverD3D_setSystemArrow);

	if (isSystemCursorInClientArea() || isSystemCursorCaptured())
	{
		SetCursor(_DefaultCursor);
	}

	// set default mouse icon to the default one
	SetClassLongPtr(_HWnd, GCLP_HCURSOR, (LONG_PTR) _DefaultCursor);
}

// ***************************************************************************
void CDriverD3D::showCursor(bool b)
{
	H_AUTO_D3D(CDriverD3D_showCursor);

	if (_HWnd == EmptyWindow)
		return;

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
}

// ***************************************************************************
void CDriverD3D::setMousePos(float x, float y)
{
	H_AUTO_D3D(CDriverD3D_setMousePos);

	if (_HWnd == EmptyWindow || !_WindowFocus)
		return;

	// convert position size from float to pixels
	sint x1 = (sint)((float)_CurrentMode.Width*x);
	sint y1 = (sint)((float)_CurrentMode.Height*(1.0f-y));

	// NeL window coordinate to MSWindows coordinates
	POINT pt;
	pt.x = x1;
	pt.y = y1;
	ClientToScreen (_HWnd, &pt);
	SetCursorPos(pt.x, pt.y);
}

// ***************************************************************************
void CDriverD3D::setCapture (bool b)
{
	H_AUTO_D3D(CDriverD3D_setCapture);

	if (b && isSystemCursorInClientArea() && !isSystemCursorCaptured())
	{
		SetCapture(_HWnd);
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
}

// ***************************************************************************
bool CDriverD3D::isSystemCursorInClientArea()
{
	if (_FullScreen /* || !IsMouseCursorHardware() */)
	{
		return IsWindowVisible(_HWnd) != FALSE;
	}
	else
	{
		POINT cursPos;
		// the mouse should be in the client area of the window
		if (!GetCursorPos(&cursPos))
		{
			return false;
		}
		HWND wnd = WindowFromPoint(cursPos);
		if (wnd != _HWnd)
		{
			return false; // not the same window
		}
		// want that the mouse be in the client area
		RECT clientRect;
		if (!GetClientRect(_HWnd, &clientRect))
		{
			return false;
		}
		POINT tl, br;
		tl.x = clientRect.left;
		tl.y = clientRect.top;
		br.x = clientRect.right;
		br.y = clientRect.bottom;
		if (!ClientToScreen(_HWnd, &tl))
		{
			return false;
		}
		if (!ClientToScreen(_HWnd, &br))
		{
			return false;
		}
		if ((cursPos.x < tl.x) || (cursPos.x >= br.x) || (cursPos.y < tl.y) || (cursPos.y >= br.y))
		{
			return false;
		}
	}

	return true;
}

// ***************************************************************************
bool CDriverD3D::isSystemCursorCaptured()
{
	H_AUTO_D3D(CDriverD3D_isSystemCursorCaptured);

	return GetCapture() == _HWnd;
}

bool CDriverD3D::convertBitmapToCursor(const NLMISC::CBitmap &bitmap, nlCursor &cursor, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY)
{
	return convertBitmapToIcon(bitmap, cursor, iconWidth, iconHeight, iconDepth, col, hotSpotX, hotSpotY, true);
}

} // NL3D
