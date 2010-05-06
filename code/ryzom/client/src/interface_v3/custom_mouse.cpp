// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "custom_mouse.h"
//
#include "nel/misc/file.h"
#include "../global.h"
#include "../input.h"
#include "input_handler_manager.h"


CCustomMouse CustomMouse;

using namespace NLMISC;
using namespace NL3D;

#ifdef NL_OS_WINDOWS

//*************************************************************************************
CCustomMouse::CCursor::CCursor() : ColorDepth(CCustomMouse::ColorDepth32),
								   OrigHeight(32),
								   HotspotScale(1.f),
								   HotspotOffsetX(0),
								   HotspotOffsetY(0),
								   HotSpotX(0),
								   HotSpotY(0),
								   Icon(0),
								   Col(CRGBA::White),
								   Rot(0)
{
}

//*************************************************************************************
CCustomMouse::CCursor::~CCursor()
{
	if (Icon)
	{
		DestroyIcon(Icon);
	}
}

//*************************************************************************************
CCustomMouse::CCustomMouse()
{
	_ColorDepth = CCustomMouse::ColorDepth32;
	_DefaultCursor = LoadCursor(NULL, IDC_ARROW);
	_AlphaBlendedCursorSupported = false;
	_AlphaBlendedCursorSupportRetrieved = false;
	_CurrCol = CRGBA::White;
	_CurrRot = 0;
	_CurrHotSpotX = 0;
	_CurrHotSpotY = 0;
}



//*************************************************************************************
bool CCustomMouse::isAlphaBlendedCursorSupported()
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

bool VerboseCursorRT12516 = true;

namespace NLMISC
{
	extern bool TempMaxVerboseResample;
}

//*************************************************************************************
void CCustomMouse::addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap)
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
	curs.Src.resize(tmpSize, tmpSize),
	// blit at top left corner
	curs.Src.blit(cursorBitmap, minX, minY, maxX - minX + 1, maxY - minY + 1, 0, 0);

	curs.OrigHeight = cursorBitmap.getHeight();
	curs.HotspotOffsetX = minX;
	curs.HotspotOffsetY = minY;
	//
	curs.HotspotScale = ClientCfg.HardwareCursorScale;
	clamp(curs.HotspotScale, 0.f, 1.f);
	// first resampling, same for all cursors
	tmpSize = (uint) (tmpSize * curs.HotspotScale);
	if (tmpSize == 0) tmpSize = 1;

	if (VerboseCursorRT12516 && ((name == "curs_stop.tga") || (name == "curs_pick_dup.tga")))
		TempMaxVerboseResample = true;

	if (TempMaxVerboseResample)
	{
		try
		{
			//nldebug("RT12516: BEFORE FIRST RESAMPLE");
			//nldebug("RT12516: %s: curs=%p curs.Src=%p curs.Src.PixelPtr=%p", name.c_str(), &curs, &(curs.Src), &curs.Src.getPixels(0)[0]);
			//nldebug("RT12516: %s: curs.Src.PixelSize=%u", name.c_str(), curs.Src.getPixels(0).size());
		}
		catch (...)
		{
			//nldebug("RT12516: An exception occurred!");
		}
	}

	// TMP for  RT 12406
/*	nlwarning("Resampling mouse %s cursor : initial size = %d x %d, new size = %d x %d",
			  name.c_str(),
			  curs.Src.getWidth(),
			  curs.Src.getHeight(),
			  tmpSize,
			  tmpSize
			 );*/
	curs.Src.resample(tmpSize, tmpSize);

	if (TempMaxVerboseResample)
	{
		try
		{
			//nldebug("RT12516: AFTER FIRST RESAMPLE");
			//nldebug("RT12516: %s: curs=%p curs.Src=%p curs.Src.PixelPtr=%p", name.c_str(), &curs, &(curs.Src), &curs.Src.getPixels(0)[0]);
			//nldebug("RT12516: %s: curs.Src.PixelSize=%u", name.c_str(), curs.Src.getPixels(0).size());
		}
		catch (...)
		{
			//nldebug("RT12516: An exception occurred!");
		}
	}

	// shrink if necessary
	if (tmpSize > destWidth || tmpSize > destHeight) // need to shrink ?
	{
		// constraint proportions
		curs.HotspotScale *= std::min(float(destWidth) / tmpSize, float(destHeight) / tmpSize);
		// TMP for  RT 12406
/*		nlwarning("Resampling mouse %s cursor : initial size = %d x %d, new size = %d x %d",
				  name.c_str(),
				  curs.Src.getWidth(),
				  curs.Src.getHeight(),
				  destWidth,
				  destHeight
				 );*/
		curs.Src.resample(destWidth, destHeight);
	}
	else
	{
		CBitmap final;
		final.resize(destWidth, destHeight);
		final.blit(&curs.Src, 0, 0);
		curs.Src.swap(final);
	}

	if (TempMaxVerboseResample)
	{
		try
		{
			//nldebug("RT12516: AFTER SECOND RESAMPLE");
			//nldebug("RT12516: %s: curs=%p curs.Src=%p curs.Src.PixelPtr=%p", name.c_str(), &curs, &(curs.Src), &curs.Src.getPixels(0)[0]);
			//nldebug("RT12516: %s: curs.Src.PixelSize=%u", name.c_str(), curs.Src.getPixels(0).size());
		}
		catch (...)
		{
			//nldebug("RT12516: An exception occurred!");
		}
	}

	if (name == _CurrName)
	{
		updateCursor();
	}
	TempMaxVerboseResample = false;
}


//*************************************************************************************
void CCustomMouse::release()
{
	if (!isAlphaBlendedCursorSupported()) return;
	nlassert(Driver);
	HWND drvWnd = (HWND) Driver->getDisplay();
	if (drvWnd)
	{
		SetClassLongPtr(drvWnd, GCLP_HCURSOR, 0);
	}
	_Cursors.clear();
}

//*************************************************************************************
void CCustomMouse::setColorDepth(TColorDepth colorDepth)
{
	if (colorDepth == _ColorDepth) return;
	_ColorDepth = colorDepth;
	updateCursor(true);
}

//*************************************************************************************
void CCustomMouse::updateCursor(bool forceRebuild)
{
	if (!Driver) return;
	setCursor(_CurrName, _CurrCol, _CurrRot, _CurrHotSpotX, _CurrHotSpotY, forceRebuild);
}

//*************************************************************************************
void CCustomMouse::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
{
	if (!isAlphaBlendedCursorSupported()) return;
	_CurrName = name;
	_CurrCol = col;
	_CurrRot = rot;
	_CurrHotSpotX = hotSpotX;
	_CurrHotSpotY = hotSpotY;
	//
	if (rot >	3) rot = 3; // same than 'CViewRenderer::drawRotFlipBitmapTiled
	TIconMap::iterator it = _Cursors.find(name);
	HCURSOR cursorHandle = _DefaultCursor;
	if (it != _Cursors.end())
	{
		// Update cursor if modified or not already built
		CCursor &curs = it->second;
		hotSpotX = (sint) (curs.HotspotScale * (hotSpotX - curs.HotspotOffsetX));
		hotSpotY = (sint) (curs.HotspotScale * ((curs.OrigHeight - hotSpotY) - curs.HotspotOffsetY));
		if (curs.Icon == 0 ||
			curs.HotSpotX != hotSpotX ||
			curs.HotSpotY != hotSpotY ||
			curs.Col != col ||
			curs.Rot != rot ||
			curs.ColorDepth != _ColorDepth ||
			forceRebuild
		   )
		{
			if (curs.Icon != 0)
			{
				DestroyIcon(curs.Icon);
			}
			curs.Icon = buildCursor(curs.Src, col, rot, hotSpotX, hotSpotY);
			curs.Col = col;
			curs.Rot = rot;
			curs.HotSpotX = hotSpotX;
			curs.HotSpotY = hotSpotY;
			curs.ColorDepth = _ColorDepth;
		}
		cursorHandle = curs.Icon ? (HCURSOR) curs.Icon : _DefaultCursor;
	}
	if (IsSystemCursorInClientArea() || IsSystemCursorCaptured() || forceRebuild)
	{
		if (CInputHandlerManager::getInstance()->hasFocus())
		{
			::SetCursor(cursorHandle);
			HWND drvWnd = (HWND) Driver->getDisplay();
			if (drvWnd)
			{
				SetClassLongPtr(drvWnd, GCLP_HCURSOR, (LONG_PTR) cursorHandle); // set default mouse icon to the last one
			}
		}
	}

}

//*************************************************************************************
HICON CCustomMouse::buildCursor(const CBitmap &src, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY)
{
	nlassert(isAlphaBlendedCursorSupported());
	uint mouseW = GetSystemMetrics(SM_CXCURSOR);
	uint mouseH = GetSystemMetrics(SM_CYCURSOR);
	nlassert(src.getWidth() == mouseW);
	nlassert(src.getHeight() == mouseH);
	HICON result = 0;
	CBitmap rotSrc = src;
	if (rot > 3) rot = 3; // mimic behavior of 'CViewRenderer::drawRotFlipBitmapTiled' (why not rot & 3 ??? ...)
	switch(rot)
	{
		case 0: break;
		case 1: rotSrc.rot90CW(); break;
		case 2: rotSrc.rot90CW(); rotSrc.rot90CW(); break;
		case 3: rotSrc.rot90CCW(); break;
	}
	CBitmap colorBm;
	colorBm.resize(mouseW, mouseH, CBitmap::RGBA);
	const CRGBA *srcColorPtr = (CRGBA *) &(rotSrc.getPixels()[0]);
	const CRGBA *srcColorPtrLast = srcColorPtr + (mouseW * mouseH);
	CRGBA *destColorPtr = (CRGBA *) &(colorBm.getPixels()[0]);
	static volatile uint8 alphaThreshold = 127;
	do
	{
			destColorPtr->modulateFromColor(*srcColorPtr, col);
			std::swap(destColorPtr->R, destColorPtr->B);
			++ srcColorPtr;
			++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);
	//
	HBITMAP colorHbm = 0;
	HBITMAP maskHbm = 0;
	//
	if (_ColorDepth == ColorDepth16)
	{
		std::vector<uint16> colorBm16(colorBm.getWidth() * colorBm.getHeight());
		const CRGBA *src32 = (const CRGBA *) &colorBm.getPixels(0)[0];
		for (uint k = 0;k < colorBm16.size(); ++k)
		{
			colorBm16[k] = ((uint16)(src32[k].R&0xf8)>>3) | ((uint16)(src32[k].G&0xfc)<<3) | ((uint16)(src32[k].B & 0xf8)<<8);
		}
		colorHbm = CreateBitmap(mouseW, mouseH, 1, 16, &colorBm16[0]);
		std::vector<uint8> bitMask((colorBm.getWidth() * colorBm.getHeight() + 7) / 8, 0);
		for (uint k = 0;k < colorBm16.size(); ++k)
		{
			if (src32[k].A <= 120)
			{
				bitMask[k / 8] |= (0x80 >> (k & 7));
			}
		}
		maskHbm = CreateBitmap(mouseW, mouseH, 1, 1, &bitMask[0]);
	}
	else
	{
		colorHbm = CreateBitmap(mouseW, mouseH, 1, 32, &colorBm.getPixels(0)[0]);
		maskHbm = CreateBitmap(mouseW, mouseH, 1, 32, &colorBm.getPixels(0)[0]);
	}
	ICONINFO iconInfo;
	iconInfo.fIcon = FALSE;
	iconInfo.xHotspot = (DWORD) hotSpotX;
	iconInfo.yHotspot = (DWORD) hotSpotY;
	iconInfo.hbmMask = maskHbm;
	iconInfo.hbmColor = colorHbm;
	if (colorHbm && maskHbm)
	{
		result = CreateIconIndirect(&iconInfo);
	}
	//
	if (colorHbm) DeleteObject(colorHbm);
	if (maskHbm) DeleteObject(maskHbm);
	return result;
}


//*************************************************************************************
void CCustomMouse::setSystemArrow()
{
	extern HINSTANCE HInstance;
	HCURSOR arrow = LoadCursor(NULL, IDC_ARROW);
	if (IsSystemCursorInClientArea() || IsSystemCursorCaptured())
	{
		::SetCursor(arrow);
	}
	HWND drvWnd = (HWND) Driver->getDisplay();
	if (drvWnd)
	{
		SetClassLongPtr(drvWnd, GCLP_HCURSOR, (LONG_PTR) arrow); // set default mouse icon to the last one
	}
}

#else

// not implemented yet for other OS

//*************************************************************************************
CCustomMouse::CCustomMouse()
{
	// NOT IMPLEMENTED
}

//*************************************************************************************
void CCustomMouse::setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild)
{
	// NOT IMPLEMENTED
}

//*************************************************************************************
void CCustomMouse::release()
{
	// NOT IMPLEMENTED
}

//*************************************************************************************
bool CCustomMouse::isAlphaBlendedCursorSupported()
{
	return false;
}

//*************************************************************************************
void CCustomMouse::setSystemArrow()
{
	//
}

void CCustomMouse::addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap)
{
	// TODO for Linux
}

//*************************************************************************************
void CCustomMouse::setColorDepth(TColorDepth colorDepth)
{
	// TODO for Linux
}

//*************************************************************************************
void CCustomMouse::updateCursor(bool forceRebuild)
{
	// TODO for Linux
}

#endif // NL_OS_WINDOWS





