// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "stdmisc.h"

#include "nel/misc/win_minimized_picture.h"

#include "nel/misc/bitmap.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// The messages are those of winuser.h and the rest is of dwmapi.h. The headers declare them only for the
// versions of the system that have them, and NeL is also built for older ones and with older headers: they are
// declared here, and the functions are taken from the library when first needed.
#ifndef WM_DWMSENDICONICTHUMBNAIL
#	define WM_DWMSENDICONICTHUMBNAIL 0x0323
#endif
#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#	define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#endif
#ifndef DWM_SIT_DISPLAYFRAME
#	define DWM_SIT_DISPLAYFRAME 0x00000001
#endif

namespace NLMISC {

namespace {

// DWMWA_FORCE_ICONIC_REPRESENTATION and DWMWA_HAS_ICONIC_BITMAP of the enumeration DWMWINDOWATTRIBUTE (an
// enumeration cannot be tested for by the preprocessor, hence the names of our own)
const DWORD s_AttributeForceIconicRepresentation = 7;
const DWORD s_AttributeHasIconicBitmap = 10;

typedef HRESULT (WINAPI *TDwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
typedef HRESULT (WINAPI *TDwmSetIconicThumbnail)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
typedef HRESULT (WINAPI *TDwmSetIconicLivePreviewBitmap)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
typedef HRESULT (WINAPI *TDwmInvalidateIconicBitmaps)(HWND hwnd);

// The functions were looked for, and found
bool s_FunctionsTried = false;
bool s_FunctionsLoaded = false;
TDwmSetWindowAttribute s_DwmSetWindowAttribute = NULL;
TDwmSetIconicThumbnail s_DwmSetIconicThumbnail = NULL;
TDwmSetIconicLivePreviewBitmap s_DwmSetIconicLivePreviewBitmap = NULL;
TDwmInvalidateIconicBitmaps s_DwmInvalidateIconicBitmaps = NULL;

} // anonymous namespace

// ***************************************************************************
CWinMinimizedPicture::CWinMinimizedPicture()
{
	m_Supplied = false;
	m_Minimized = false;
	m_Declared = false;
	m_Width = 0;
	m_Height = 0;
	m_ThumbnailWanted = false;
	m_ThumbnailMaxWidth = 0;
	m_ThumbnailMaxHeight = 0;
	m_PreviewWanted = false;
	m_OutOfDatePending = false;
	m_HandOverTime = 0;
}

// ***************************************************************************
bool CWinMinimizedPicture::loadFunctions()
{
	if (s_FunctionsTried)
		return s_FunctionsLoaded;
	s_FunctionsTried = true;

	// The library of the system, by its full path
	static const wchar_t fileName[] = L"\\dwmapi.dll";
	const UINT fileNameLength = sizeof(fileName) / sizeof(fileName[0]); // with the terminator
	wchar_t path[MAX_PATH];
	UINT length = GetSystemDirectoryW(path, MAX_PATH);
	if (length == 0)
	{
		DWORD error = GetLastError();
		nlwarning("The picture of a minimized window is not available: the directory of the system is not known (error %u)", (uint)error);
		return false;
	}
	if (length + fileNameLength > MAX_PATH)
	{
		nlwarning("The picture of a minimized window is not available: the path of the directory of the system is too long (%u characters)", (uint)length);
		return false;
	}
	memcpy(path + length, fileName, sizeof(fileName));

	HMODULE library = LoadLibraryW(path);
	if (!library)
	{
		DWORD error = GetLastError();
		nlinfo("The picture of a minimized window is not available on this system: it has no dwmapi.dll (error %u)", (uint)error);
		return false;
	}

	s_DwmSetWindowAttribute = (TDwmSetWindowAttribute)GetProcAddress(library, "DwmSetWindowAttribute");
	s_DwmSetIconicThumbnail = (TDwmSetIconicThumbnail)GetProcAddress(library, "DwmSetIconicThumbnail");
	s_DwmSetIconicLivePreviewBitmap = (TDwmSetIconicLivePreviewBitmap)GetProcAddress(library, "DwmSetIconicLivePreviewBitmap");
	s_DwmInvalidateIconicBitmaps = (TDwmInvalidateIconicBitmaps)GetProcAddress(library, "DwmInvalidateIconicBitmaps");

	const char *missing = NULL;
	if (!s_DwmSetWindowAttribute)
		missing = "DwmSetWindowAttribute";
	else if (!s_DwmSetIconicThumbnail)
		missing = "DwmSetIconicThumbnail";
	else if (!s_DwmSetIconicLivePreviewBitmap)
		missing = "DwmSetIconicLivePreviewBitmap";
	else if (!s_DwmInvalidateIconicBitmaps)
		missing = "DwmInvalidateIconicBitmaps";
	if (missing)
	{
		nlinfo("The picture of a minimized window is not available on this system: its dwmapi.dll has no %s", missing);
		s_DwmSetWindowAttribute = NULL;
		s_DwmSetIconicThumbnail = NULL;
		s_DwmSetIconicLivePreviewBitmap = NULL;
		s_DwmInvalidateIconicBitmaps = NULL;
		FreeLibrary(library);
		return false;
	}

	// the library stays loaded for as long as the process runs
	s_FunctionsLoaded = true;
	return true;
}

// ***************************************************************************
bool CWinMinimizedPicture::setSupplied(HWND wnd, bool supplied)
{
	if (supplied == m_Supplied)
		return true;

	if (supplied)
	{
		if (!loadFunctions())
			return false;
		m_Supplied = true;
		// a window that is minimized already
		if (wnd && m_Minimized)
			declarePictures(wnd);
	}
	else
	{
		m_Supplied = false;
		withdrawPictures(wnd);
	}
	return true;
}

// ***************************************************************************
void CWinMinimizedPicture::release(HWND wnd)
{
	m_Supplied = false;
	withdrawPictures(wnd);
	// the next window of the driver is another one
	m_Minimized = false;
	m_Width = 0;
	m_Height = 0;
}

// ***************************************************************************
bool CWinMinimizedPicture::setAttribute(HWND wnd, DWORD attribute, const char *name, bool value)
{
	BOOL attributeValue = value ? TRUE : FALSE;
	HRESULT hr = s_DwmSetWindowAttribute(wnd, attribute, &attributeValue, sizeof(attributeValue));
	if (FAILED(hr))
	{
		nlwarning("Minimized window: the attribute %s of the window is not set to %s (error 0x%08x)", name, value ? "true" : "false", (uint)hr);
		return false;
	}
	return true;
}

// ***************************************************************************
void CWinMinimizedPicture::declarePictures(HWND wnd)
{
	if (m_Declared)
		return;

	if (m_Width == 0 || m_Height == 0)
	{
		nldebug("Minimized window: no picture is supplied of a window that had no area (%ux%u) when it was minimized", m_Width, m_Height);
		return;
	}

	// With the first attribute the compositor shows the bitmaps of the window in place of what it kept of
	// the window itself, with the second it asks the window for them
	if (!setAttribute(wnd, s_AttributeForceIconicRepresentation, "DWMWA_FORCE_ICONIC_REPRESENTATION", true))
		return;
	if (!setAttribute(wnd, s_AttributeHasIconicBitmap, "DWMWA_HAS_ICONIC_BITMAP", true))
	{
		setAttribute(wnd, s_AttributeForceIconicRepresentation, "DWMWA_FORCE_ICONIC_REPRESENTATION", false);
		return;
	}

	m_Declared = true;
	m_ThumbnailWanted = false;
	m_PreviewWanted = false;
	m_OutOfDatePending = false;

	// what the compositor may have kept from the last time the window was minimized is out of date
	HRESULT hr = s_DwmInvalidateIconicBitmaps(wnd);
	if (FAILED(hr))
		nlwarning("Minimized window: DwmInvalidateIconicBitmaps failed (error 0x%08x)", (uint)hr);

	nldebug("Minimized window: the application supplies the pictures of the window (%ux%u before it was minimized)", m_Width, m_Height);
}

// ***************************************************************************
void CWinMinimizedPicture::withdrawPictures(HWND wnd)
{
	if (m_Declared)
	{
		m_Declared = false;
		if (wnd)
		{
			setAttribute(wnd, s_AttributeHasIconicBitmap, "DWMWA_HAS_ICONIC_BITMAP", false);
			setAttribute(wnd, s_AttributeForceIconicRepresentation, "DWMWA_FORCE_ICONIC_REPRESENTATION", false);
		}
		nldebug("Minimized window: the pictures of the window are those of the compositor again");
	}
	m_ThumbnailWanted = false;
	m_PreviewWanted = false;
	m_OutOfDatePending = false;
}

// ***************************************************************************
void CWinMinimizedPicture::processMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam, uint32 width, uint32 height)
{
	switch (message)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!m_Minimized)
			{
				m_Minimized = true;
				m_Width = width;
				m_Height = height;
				if (m_Supplied)
					declarePictures(wnd);
			}
		}
		else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
		{
			// (the other values tell of other windows)
			if (m_Minimized)
			{
				m_Minimized = false;
				withdrawPictures(wnd);
			}
		}
		break;

	case WM_DWMSENDICONICTHUMBNAIL:
	{
		// the largest width is in the high word and the largest height in the low word, the other way round
		// from most messages; a thumbnail larger than this on either side is refused
		uint32 maxWidth = HIWORD(lParam);
		uint32 maxHeight = LOWORD(lParam);
		if (!m_Declared)
		{
			nldebug("Minimized window: a thumbnail is asked of a window that is not minimized or supplies no pictures, the request is ignored");
		}
		else if (maxWidth == 0 || maxHeight == 0)
		{
			nldebug("Minimized window: a thumbnail of at most %ux%u is asked, the request is ignored", maxWidth, maxHeight);
		}
		else
		{
			m_ThumbnailWanted = true;
			m_ThumbnailMaxWidth = maxWidth;
			m_ThumbnailMaxHeight = maxHeight;
			nldebug("Minimized window: a thumbnail of at most %ux%u is wanted", maxWidth, maxHeight);
		}
		break;
	}

	case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
		if (!m_Declared)
		{
			nldebug("Minimized window: a full-size preview is asked of a window that is not minimized or supplies no pictures, the request is ignored");
		}
		else
		{
			m_PreviewWanted = true;
			nldebug("Minimized window: a full-size preview is wanted (%ux%u)", m_Width, m_Height);
		}
		break;

	case WM_DESTROY:
		// the attributes go with the window
		m_Minimized = false;
		m_Declared = false;
		m_ThumbnailWanted = false;
		m_PreviewWanted = false;
		m_OutOfDatePending = false;
		break;

	default:
		break;
	}
}

// ***************************************************************************
bool CWinMinimizedPicture::isPictureWanted(HWND wnd, uint32 &maxWidth, uint32 &maxHeight, bool &preview)
{
	if (!m_Declared)
		return false;

	// The picture handed over is out of date once the interval has passed. The compositor then asks for a new
	// one if the thumbnail or the preview is on screen, and else when it next shows one
	if (m_OutOfDatePending)
	{
		TTime elapsed = CTime::getLocalTime() - m_HandOverTime;
		if (elapsed < 0 || elapsed >= (TTime)MinIntervalMs)
		{
			m_OutOfDatePending = false;
			HRESULT hr = s_DwmInvalidateIconicBitmaps(wnd);
			if (FAILED(hr))
				nlwarning("Minimized window: DwmInvalidateIconicBitmaps failed (error 0x%08x)", (uint)hr);
		}
	}

	if (m_PreviewWanted)
	{
		preview = true;
		maxWidth = m_Width;
		maxHeight = m_Height;
		return true;
	}
	if (m_ThumbnailWanted)
	{
		preview = false;
		maxWidth = m_ThumbnailMaxWidth;
		maxHeight = m_ThumbnailMaxHeight;
		return true;
	}
	return false;
}

// ***************************************************************************
HBITMAP CWinMinimizedPicture::createBitmap(const CBitmap &picture, uint32 maxWidth, uint32 maxHeight, uint32 &width, uint32 &height)
{
	// The picture as it is when it fits, else the largest one of its aspect ratio that does
	width = picture.getWidth();
	height = picture.getHeight();
	if (width > maxWidth || height > maxHeight)
	{
		if ((uint64)width * maxHeight > (uint64)height * maxWidth)
		{
			// the width is the side that decides
			height = (uint32)((uint64)height * maxWidth / width);
			width = maxWidth;
		}
		else
		{
			width = (uint32)((uint64)width * maxHeight / height);
			height = maxHeight;
		}
		if (width == 0) width = 1;
		if (height == 0) height = 1;
	}

	CBitmap scaled;
	const CBitmap *source = &picture;
	if (width != picture.getWidth() || height != picture.getHeight())
	{
		scaled = picture;
		scaled.resample((sint32)width, (sint32)height);
		source = &scaled;
	}

	// The compositor wants a bitmap with a color depth of 32 bits. It is made the way the sample of the
	// system for these bitmaps makes it: a bitmap of our own memory whose first row is the top one (negative
	// height), four bytes a pixel in the order blue, green, red, alpha, the colors multiplied by the alpha
	BITMAPINFO info;
	memset(&info, 0, sizeof(info));
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = (LONG)width;
	info.bmiHeader.biHeight = -(LONG)height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	void *bits = NULL;
	HBITMAP bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!bitmap || !bits)
	{
		DWORD error = GetLastError();
		nlwarning("Minimized window: CreateDIBSection made no bitmap of %ux%u (error %u)", width, height, (uint)error);
		if (bitmap)
			DeleteObject(bitmap);
		return NULL;
	}

	// The alpha of a frame buffer is whatever the blending of the frame left in it: the picture is opaque
	const uint8 *src = source->getPixels().getPtr();
	uint8 *dst = (uint8 *)bits;
	const uint32 count = width * height;
	for (uint32 i = 0; i < count; ++i)
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = 255;
		src += 4;
		dst += 4;
	}

	return bitmap;
}

// ***************************************************************************
bool CWinMinimizedPicture::setPicture(HWND wnd, const CBitmap &picture)
{
	if (!m_Declared || !wnd || (!m_ThumbnailWanted && !m_PreviewWanted))
		return false;

	// No picture could be drawn this time: the requests stay open for a later frame
	if (picture.getWidth() == 0 || picture.getHeight() == 0)
	{
		nldebug("Minimized window: no picture was drawn, the requests stay open");
		return false;
	}

	// From here the requests are closed whatever comes of them: a picture that the system refuses is not
	// tried again, the compositor asks anew when the window is next minimized
	bool thumbnail = m_ThumbnailWanted;
	bool preview = m_PreviewWanted;
	m_ThumbnailWanted = false;
	m_PreviewWanted = false;

	if (picture.getPixelFormat() != CBitmap::RGBA)
	{
		nlwarning("Minimized window: the picture is not in the RGBA format, the compositor is given none");
		return false;
	}

	bool handedOver = false;

	if (thumbnail)
	{
		uint32 width, height;
		HBITMAP bitmap = createBitmap(picture, m_ThumbnailMaxWidth, m_ThumbnailMaxHeight, width, height);
		if (bitmap)
		{
			HRESULT hr = s_DwmSetIconicThumbnail(wnd, bitmap, 0);
			if (FAILED(hr))
			{
				nlwarning("Minimized window: DwmSetIconicThumbnail failed for a bitmap of %ux%u (error 0x%08x)", width, height, (uint)hr);
			}
			else
			{
				handedOver = true;
				nldebug("Minimized window: a thumbnail of %ux%u is handed over", width, height);
			}
			// the compositor works on a copy
			DeleteObject(bitmap);
		}
	}

	if (preview)
	{
		// The preview is the client area of the window, no larger than it was. The compositor draws the frame
		// of the window around it when the window has one
		uint32 width, height;
		HBITMAP bitmap = createBitmap(picture, m_Width, m_Height, width, height);
		if (bitmap)
		{
			DWORD flags = ((GetWindowLongW(wnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION) ? DWM_SIT_DISPLAYFRAME : 0;
			HRESULT hr = s_DwmSetIconicLivePreviewBitmap(wnd, bitmap, NULL, flags);
			if (FAILED(hr))
			{
				nlwarning("Minimized window: DwmSetIconicLivePreviewBitmap failed for a bitmap of %ux%u (error 0x%08x)", width, height, (uint)hr);
			}
			else
			{
				handedOver = true;
				nldebug("Minimized window: a full-size preview of %ux%u is handed over", width, height);
			}
			DeleteObject(bitmap);
		}
	}

	if (handedOver)
	{
		// isPictureWanted tells the compositor that this picture is out of date, MinIntervalMs from now
		m_OutOfDatePending = true;
		m_HandOverTime = CTime::getLocalTime();
	}

	return handedOver;
}

} // NLMISC

#endif // NL_OS_WINDOWS

/* end of win_minimized_picture.cpp */
