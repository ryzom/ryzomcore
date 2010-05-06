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

#ifndef	RY_CUSTOM_MOUSE_H
#define RY_CUSTOM_MOUSE_H

#ifdef NL_OS_WINDOWS
	#include <windows.h>
#endif

#include "nel/misc/rgba.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/common.h"

namespace NL3D
{
	class UTexture;
}

// TMP Nico : made a separate class to avoid a lot of compilation until it works
class CCustomMouse
{
public:
	enum TColorDepth { ColorDepth16 = 0, ColorDepth32, ColorDepthCount };
	CCustomMouse();
	/** Signal a change of color depth (of desktop if windowed, or video mode if fullscreen)
      * This is necessary to have the cursor built with good format
	  */
	void setColorDepth(TColorDepth colorDepth);
	// Add a new cursor (name is case unsensitive)
	void addCursor(const std::string &name, const NLMISC::CBitmap &cursorBitmap);
	// Display a cursor from its name (case unsensitive)
	void setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false);
	void updateCursor(bool forceRebuild = false);
	void release();
	bool isAlphaBlendedCursorSupported();
	// reset the cursor shape to the system arrow
	void setSystemArrow();
private:
#ifdef NL_OS_WINDOWS
	TColorDepth			_ColorDepth;
	std::string			_CurrName;
	NLMISC::CRGBA		_CurrCol;
	uint8				_CurrRot;
	uint				_CurrHotSpotX;
	uint				_CurrHotSpotY;
	//
	class CCursor
	{
	public:
		NLMISC::CBitmap Src;
		TColorDepth		ColorDepth;
		uint			OrigHeight;
		float			HotspotScale;
		uint			HotspotOffsetX;
		uint			HotspotOffsetY;
		sint			HotSpotX;
		sint			HotSpotY;
		HICON Icon;
		NLMISC::CRGBA Col;
		uint8 Rot;
	public:
		CCursor();
		~CCursor();
		CCursor& operator= (const CCursor& from)
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
			Icon = from.Icon;
			Col = from.Col;
			Rot = from.Rot;
			return *this;
		}
	};
	struct CStrCaseUnsensitiveCmp
	{
		bool operator()(const std::string &lhs, const std::string &rhs) const
		{
			return NLMISC::nlstricmp(lhs, rhs) < 0;
		}
	};
	typedef std::map<std::string, CCursor, CStrCaseUnsensitiveCmp> TIconMap;
	TIconMap _Cursors;
	HCURSOR  _DefaultCursor;
	bool	 _AlphaBlendedCursorSupported;
	bool	 _AlphaBlendedCursorSupportRetrieved;
private:
	 // build a cursor from src, src should have the same size that the hardware cursor
	 // or a assertion is thrown
	HICON buildCursor(const NLMISC::CBitmap &src, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY);
#endif // NL_OS_WINDOWS
};

extern CCustomMouse CustomMouse;

#endif
