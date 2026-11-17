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

#ifndef NL_WIN_MINIMIZED_PICTURE_H
#define NL_WIN_MINIMIZED_PICTURE_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS // for win32 os only

#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

#include "time_nl.h"

namespace NLMISC {

class CBitmap;

/**
 * The picture of a minimized window that the application supplies to the desktop compositor.
 *
 * The desktop compositor shows a picture of a window where the user picks among the windows: the thumbnail of
 * the taskbar, and the full-size preview shown while the pointer is over that thumbnail. For a window that is
 * shown it takes the picture itself. A minimized window is not composed, and its picture is the last one taken
 * before it was minimized. A window may declare that it supplies its own pictures (the window attributes
 * DWMWA_FORCE_ICONIC_REPRESENTATION and DWMWA_HAS_ICONIC_BITMAP of dwmapi.h): it is then sent a message when a
 * picture is needed (WM_DWMSENDICONICTHUMBNAIL, WM_DWMSENDICONICLIVEPREVIEWBITMAP), answers with a bitmap
 * (DwmSetIconicThumbnail, DwmSetIconicLivePreviewBitmap), and tells when that bitmap is out of date
 * (DwmInvalidateIconicBitmaps), upon which the compositor asks again the next time it needs the picture.
 *
 * This class keeps that exchange for one window, for the 3D drivers:
 * - the window procedure passes every message of the window to processMessage(). The attributes are set when
 *   the window is minimized and cleared when it is restored, so that a window that is shown or covered keeps
 *   the picture the compositor takes of it; the requests are recorded;
 * - the application asks isPictureWanted() while the window is minimized, draws one frame when it is, and hands
 *   it over with setPicture(), which converts it, scales it down to what was asked, and gives it to the
 *   compositor;
 * - MinIntervalMs after a picture was handed over, the next isPictureWanted() tells the compositor that the
 *   picture is out of date. The compositor only asks for a new one while the thumbnail or the preview is on
 *   screen: a picture nobody looks at is not drawn.
 *
 * Nothing is done before setSupplied(true): an application that does not answer the requests keeps the
 * pictures of the compositor. The functions come from dwmapi.dll, loaded when first needed; on a system
 * without that library or without these functions, it is said once in the log and the pictures stay those of
 * the system.
 *
 * All calls are made from the thread of the window.
 */
class CWinMinimizedPicture
{
public:
	enum
	{
		/// The least time between a picture handed over and the next request for one, in milliseconds
		MinIntervalMs = 100
	};

	CWinMinimizedPicture();

	/** The application supplies the pictures of the window while it is minimized, or stops doing so.
	 *  \param wnd the window, NULL when there is none yet
	 *  \return false when asked to supply on a system that has not got the functions
	 */
	bool setSupplied(HWND wnd, bool supplied);
	bool isSupplied() const { return m_Supplied; }

	/** The driver lets go of the window. A window that lives on gets its pictures from the compositor again,
	 *  and nothing is kept of it: the application declares anew that it supplies pictures, for the next window.
	 *  \param wnd the window, NULL when there is none
	 */
	void release(HWND wnd);

	/** To call from the window procedure, with every message of the window, before the driver changes its own
	 *  record of the size of the window for the message.
	 *  \param width, height the size of the client area as the driver has it before this message, which is
	 *  the size before the window is minimized when the message tells it is
	 */
	void processMessage(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam, uint32 width, uint32 height);

	/** True when the compositor waits for a picture of the minimized window.
	 *  \param maxWidth, maxHeight the largest picture that is of use: the size the window had before it was
	 *  minimized when the full-size preview is wanted, else the largest size of the thumbnail
	 *  \param preview the full-size preview is wanted (the thumbnail may be wanted with it)
	 */
	bool isPictureWanted(HWND wnd, uint32 &maxWidth, uint32 &maxHeight, bool &preview);

	/** Hand over a picture (RGBA, first row at the top, alpha not used) for all that is wanted. It is scaled
	 *  down to the size wanted, with its aspect ratio kept, and the requests are closed. A picture without
	 *  area says that none could be drawn this time: the requests stay open, and isPictureWanted goes on
	 *  answering true.
	 *  \return true when a picture was given to the compositor
	 */
	bool setPicture(HWND wnd, const CBitmap &picture);

	/// The size the client area of the window had before it was minimized (as given to processMessage)
	void getWindowSize(uint32 &width, uint32 &height) const
	{
		width = m_Width;
		height = m_Height;
	}

private:
	// Get the functions of the compositor; false when the system has not got them (said once in the log)
	static bool loadFunctions();
	// Set the attributes of a minimized window, so that the compositor asks for its pictures
	void declarePictures(HWND wnd);
	// Clear the attributes, and forget the requests
	void withdrawPictures(HWND wnd);
	// Set one attribute of the window, false with a warning when the compositor refuses
	static bool setAttribute(HWND wnd, DWORD attribute, const char *name, bool value);
	// Make the bitmap for the compositor out of the picture, scaled down to fit into the given size.
	// NULL with a warning when the system makes none. To delete with DeleteObject
	static HBITMAP createBitmap(const CBitmap &picture, uint32 maxWidth, uint32 maxHeight, uint32 &width, uint32 &height);

private:
	// The application supplies the pictures
	bool m_Supplied;
	// The window is minimized (as told by its messages)
	bool m_Minimized;
	// The attributes are set: the compositor asks this window for its pictures
	bool m_Declared;
	// The size of the client area before the window was minimized
	uint32 m_Width;
	uint32 m_Height;
	// The requests that wait for a picture
	bool m_ThumbnailWanted;
	uint32 m_ThumbnailMaxWidth;
	uint32 m_ThumbnailMaxHeight;
	bool m_PreviewWanted;
	// A picture was handed over at m_HandOverTime, and the compositor is yet to be told that it is out of date
	bool m_OutOfDatePending;
	TTime m_HandOverTime;
};

} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_MINIMIZED_PICTURE_H

/* end of win_minimized_picture.h */
