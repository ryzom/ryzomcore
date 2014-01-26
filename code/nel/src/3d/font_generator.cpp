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

#include "std3d.h"

#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

#include "nel/3d/font_generator.h"

using namespace std;

#ifndef NL_DONT_USE_EXTERNAL_CODE

#include <ft2build.h>
#include FT_FREETYPE_H

// for freetype 2.0
#ifdef FTERRORS_H
#undef FTERRORS_H
#endif

// for freetype 2.0.1
#ifdef __FTERRORS_H__
#undef __FTERRORS_H__
#endif

#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST  {
#define FT_ERROR_END_LIST    { 0, 0 } };

const struct
{
int          err_code;
const char*  err_msg;
} ft_errors[] =

#include FT_ERRORS_H

using namespace NLMISC;

namespace NL3D {

FT_Library	CFontGenerator::_Library = NULL;
uint		CFontGenerator::_LibraryInit = 0;
uint32		CFontGenerator::_FontGeneratorCounterUID = 1;

const char *CFontGenerator::getFT2Error(FT_Error fte)
{
	static char ukn[1024];

	for (uint32 i = 0; ft_errors[i].err_code != 0 || ft_errors[i].err_msg != 0; i++)
	{
		if (ft_errors[i].err_code == fte)
			return ft_errors[i].err_msg;
	}
	smprintf (ukn, 1024, "Unknown freetype2 error, errcode: 0x%x", fte);
	return ukn;
}



/*
 * Constructor
 */
CFontGenerator::CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName)
{
	_UID = _FontGeneratorCounterUID;
	_FontGeneratorCounterUID++;
	_FontFileName = fontFileName;

	FT_Error error;

	if (!_LibraryInit)
	{
		error = FT_Init_FreeType (&_Library);
		if (error)
		{
			nlerror ("FT_Init_FreeType() failed: %s", getFT2Error(error));
		}
	}
	++_LibraryInit;

	error = FT_New_Face (_Library, fontFileName.c_str (), 0, &_Face);
	if (error)
	{
		nlerror ("FT_New_Face() failed with file '%s': %s", fontFileName.c_str(), getFT2Error(error));
	}

	string fontEx = fontExFileName;
	if (fontEx.empty())
	{
		// try to see if the ex filename exists based on the fontExFileName
		fontEx = CPath::lookup(CFile::getFilenameWithoutExtension (fontFileName)+".afm", false, false);
	}

	if (!fontEx.empty())
	{
		error = FT_Attach_File (_Face, fontEx.c_str ());
		if (error)
		{
			nlwarning ("FT_Attach_File() failed with file '%s': %s", fontEx.c_str(), getFT2Error(error));
		}
	}

	error = FT_Select_Charmap (_Face, ft_encoding_unicode);
	if (error)
	{
		nlerror ("FT_Select_Charmap() failed with file '%s': %s", fontFileName.c_str(), getFT2Error(error));
	}
}

CFontGenerator::~CFontGenerator ()
{
	// important: destroying and creating your last font generator
	// will also reload the freetype library
	nlassert(_LibraryInit);
	--_LibraryInit;
	if (!_LibraryInit)
	{
		FT_Done_FreeType(_Library);
		_Library = NULL;
	}
}

void CFontGenerator::getSizes (ucchar c, uint32 size, uint32 &width, uint32 &height)
{
	FT_Error error;

	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	// retrieve glyph index from character code
	FT_UInt glyph_index = FT_Get_Char_Index (_Face, c);

	if (glyph_index == 0)
	{
		// no glyph available, replace with a dot
		glyph_index = FT_Get_Char_Index (_Face, ucchar('.'));
	}

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (_Face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		nlerror ("FT_Load_Glyph() failed: %s", getFT2Error(error));
	}

	// convert 24.6 fixed point into integer
	width = _Face->glyph->metrics.width >> 6;
	height = _Face->glyph->metrics.height >> 6;
}

uint8 *CFontGenerator::getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex)
{
	FT_Error error;

	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	// retrieve glyph index from character code
	FT_UInt glyph_index = FT_Get_Char_Index (_Face, c);

	if (glyph_index == 0)
	{
		// no glyph available, replace with a dot
		glyph_index = FT_Get_Char_Index (_Face, ucchar('.'));
	}

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (_Face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		nlerror ("FT_Load_Glyph() failed: %s", getFT2Error(error));
	}

	if (size == 0)
	{
		width = 0;
		height = 0;
		pitch = 0;
		left = 0;
		top = 0;
		advx = 0;
		glyphIndex = glyph_index;
		return NULL;
	}

	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph (_Face->glyph, ft_render_mode_normal);
	if (error)
	{
		nlerror ("FT_Render_Glyph() failed: %s", getFT2Error(error));
	}

	width = _Face->glyph->bitmap.width;
	height = _Face->glyph->bitmap.rows;
	pitch = _Face->glyph->bitmap.pitch;

	left = _Face->glyph->bitmap_left;
	top = _Face->glyph->bitmap_top;

	advx = _Face->glyph->advance.x >> 6;

	glyphIndex = glyph_index;

	return (uint8 *) _Face->glyph->bitmap.buffer;
}



void CFontGenerator::getKerning (ucchar left, ucchar right, sint32 &kernx)
{
	if (!FT_HAS_KERNING(_Face))
	{
		kernx = 0;
	}
	else
	{
		FT_Vector  kerning;
		FT_Error error = FT_Get_Kerning (_Face, left, right, ft_kerning_default, &kerning);
		if (error)
		{
			nlerror ("FT_Get_Kerning() failed: %s", getFT2Error(error));
		}
		kernx = kerning.x;
	}
}



uint32	 CFontGenerator::getCharIndex (ucchar c)
{
	uint32 ret = FT_Get_Char_Index(_Face, c);

	if (ret == 0)
	{
		// no glyph available, replace with a dot
		ret = FT_Get_Char_Index (_Face, ucchar('.'));
	}

	return ret;
}

} // NL3D

#else // NL_DONT_USE_EXTERNAL_CODE

#define NOMINMAX
#include <windows.h>

using namespace NLMISC;

namespace NL3D {

HBITMAP Dib;
uint32 *Buffer;
HDC hdcDib;
int Width = 100;
int Height = 100;

/*
 * Constructor
 */
CFontGenerator::CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName)
{

//	HWND win=winHack;
//	WindowHandle = win;
//	Format = format;
//	RECT rect;
//	BOOL ret = GetClientRect( WindowHandle, &rect);
//	assert (ret);

//	uint f = AddFontResource ("");
//	nlassert (f);

	BITMAPINFO info;
	info.bmiHeader.biWidth = Width;
	info.bmiHeader.biHeight= -Height;
	info.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 4*Width*Height;
	info.bmiHeader.biXPelsPerMeter = 1;
	info.bmiHeader.biYPelsPerMeter = 1;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;

	HDC hdc = GetDC (NULL);
	nlassert (hdc);
	Dib = CreateDIBSection (hdc, &info, DIB_RGB_COLORS, (void**)&Buffer, NULL, NULL);

	hdcDib = CreateCompatibleDC (hdc);
	nlassert (hdcDib);


	ReleaseDC (NULL, hdc);

	SetTextAlign (hdcDib, TA_TOP | TA_LEFT | TA_NOUPDATECP);
	SetBkColor (hdcDib, RGB (0,0,0));
	SetTextColor (hdcDib, RGB (255, 255, 255));

	/*FT_Error error;

	if (!_LibraryInit)
	{
		error = FT_Init_FreeType (&_Library);
		if (error)
		{
			nlerror ("FT_Init_FreeType() failed: %s", getFT2Error(error));
		}
		_LibraryInit = true;
	}

	error = FT_New_Face (_Library, fontFileName.c_str (), 0, &_Face);
	if (error)
	{
		nlerror ("FT_New_Face() failed with file '%s': %s", fontFileName.c_str(), getFT2Error(error));
	}

	if (fontExFileName != "")
	{
		error = FT_Attach_File (_Face, fontExFileName.c_str ());
		if (error)
		{
			nlwarning ("FT_Attach_File() failed with file '%s': %s", fontExFileName.c_str(), getFT2Error(error));
		}
	}*/
}

CFontGenerator::~CFontGenerator ()
{
	DeleteObject (Dib);

	DeleteDC (hdcDib);
}

void CFontGenerator::getSizes (ucchar c, uint32 size, uint32 &width, uint32 &height)
{
/*	FT_Error error;

	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	// retrieve glyph index from character code
	FT_UInt glyph_index = FT_Get_Char_Index (_Face, c);

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (_Face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		nlerror ("FT_Load_Glyph() failed: %s", getFT2Error(error));
	}

	// convert 24.6 fixed point into integer
	width = _Face->glyph->metrics.width >> 6;
	height = _Face->glyph->metrics.height >> 6;
	*/
}

HFONT hFont = NULL;
uint32 CurrentFontSize = 0;

uint8 *CFontGenerator::getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex)
{
/*	FT_Error error;

	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	// retrieve glyph index from character code
	FT_UInt glyph_index = FT_Get_Char_Index (_Face, c);

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (_Face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		nlerror ("FT_Load_Glyph() failed: %s", getFT2Error(error));
	}

	if (size == 0)
	{
		width = 0;
		height = 0;
		pitch = 0;
		left = 0;
		top = 0;
		advx = 0;
		glyphIndex = glyph_index;
		return NULL;
	}

	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph (_Face->glyph, ft_render_mode_normal);
	if (error)
	{
		nlerror ("FT_Render_Glyph() failed: %s", getFT2Error(error));
	}

	width = _Face->glyph->bitmap.width;
	height = _Face->glyph->bitmap.rows;
	pitch = _Face->glyph->bitmap.pitch;

	left = _Face->glyph->bitmap_left;
	top = _Face->glyph->bitmap_top;

	advx = _Face->glyph->advance.x >> 6;

	glyphIndex = glyph_index;

	return (uint8 *) _Face->glyph->bitmap.buffer;
*/

	if (size == 0)
	{
		width = 0;
		height = 0;
		pitch = 0;
		left = 0;
		top = 0;
		advx = 0;
		glyphIndex = 0;
		return NULL;
	}

	// Create the font
	if (hFont ==  NULL || CurrentFontSize != size)
	{
		if (hFont !=  NULL)
		{
			DeleteObject (hFont);
		}

		hFont=CreateFont
		(
			size,					// logical height of font
			0,					// logical average character width
			0,					// angle of escapement
			0,					// base-line orientation angle
			FW_DONTCARE, //FW_NORMAL,			// font weight
			FALSE,				// italic attribute flag
			FALSE,				// underline attribute flag
			FALSE,				// strikeout attribute flag
			DEFAULT_CHARSET,	// character set identifier
			OUT_DEVICE_PRECIS,  // output precision
			CLIP_DEFAULT_PRECIS,	// clipping precision
			DEFAULT_QUALITY,        // output quality
			DEFAULT_PITCH|FF_DONTCARE,  // pitch and family
			"Arial Unicode MS Normal"         // pointer to typeface name string
		);
		nlassert (hFont);

		CurrentFontSize = size;
	}

	SelectObject (hdcDib, hFont);
	SelectObject (hdcDib, Dib);

	const ucchar cc = /*(char)*/ c;

	// prevent outputing white glyph if char is not available in font
	DWORD glyphIndex;
	if (GetGlyphIndicesW(hdcDib, &cc, 1, &glyphIndex, GGI_MARK_NONEXISTING_GLYPHS) == 1);
	{
		if (glyphIndex == 0xffff)
		{
			// thee char is unsupported, replace with a dot
			cc = '.';
		}
	}

	RECT rect;
	rect.bottom = Height;
	rect.top = 0;
	rect.left = 0;
	rect.right = Width;

	int res = DrawTextW (hdcDib, &cc, 1, &rect, DT_LEFT | DT_TOP);

	POINT point;
	point.y = res;

	int w = res;
//	BOOL rey = GetCharWidth32 (hdcDib, (uint8) cc,  (uint8) cc, &w);
	BOOL rey = GetCharWidth32 (hdcDib, cc,  cc, &w);
	nlassert (rey);
	point.x = w;

//	ABC abc;
//	BOOL rey = GetCharABCWidths (hdcDib, (uint8) cc, (uint8) cc, &abc);
//	nlassert (rey);
//	point.x = abc.abcA;

	SIZE s;
	GetTextExtentPoint32W (hdcDib, &cc, 1, &s);

	BOOL ret = LPtoDP (hdcDib, &point, 1);
	nlassert (ret);

	static uint8 buf[100*100];

	sint32 _top = 0, _left = point.x;
	sint32 right = 0, bottom = point.y;
	bool emptyLine;
	for (sint y = 0; y < point.y; y++)
	{
		emptyLine = true;
		for (sint x = 0; x < point.x; x++)
		{
			buf[y*100+x] = (uint8) Buffer[y*100+x];
			if (buf[y*100+x])
			{
				emptyLine = false;
				if (x < _left)
					_left = x;
				if (x > right)
					right = x;
			}

//			printf (buf[y*100+x]?"*":".");
		}
		if (!emptyLine)
		{
			if (_top == 0)
				_top = y;

			bottom = y;
		}

//		printf ("\n");
	}
	width = right - _left + 1;
	if (right - _left + 1 < 0) width = 0;
	height = bottom - _top + 1;
	if (bottom - _top + 1 < 0) height = 0;
	pitch = 100;
	advx = point.x;

	WORD ag = 0;
	/*uint t = GetGlyphIndices (hdcDib, &cc, 1, &ag, 0);
	if (t == GDI_ERROR)
	{
		nlstop;
	}
*/
	glyphIndex = ag;

	top = -_top;
	left = -_left;

/*	{
		for (sint y = _top; y < _top + height; y++)
		{
			for (sint x = _left; x < _left + width; x++)
			{
				printf (buf[y*100+x]?"*":".");
			}
			printf ("\n");
		}
		printf ("w: %d h: %d s: %d a: %d l: %d t: %d", width, height, size, advx, left, top);
		getchar();
	}
*/
	_top = _left = top = left = 0;
	top = s.cy;
	width = s.cx;
	height = s.cy;

	return buf + _top * 100 + _left;
}




void CFontGenerator::getKerning (ucchar left, ucchar right, sint32 &kernx)
{
/*	if (!FT_HAS_KERNING(_Face))
	{
		kernx = 0;
	}
	else
	{
		FT_Vector  kerning;
		FT_Error error = FT_Get_Kerning (_Face, left, right, ft_kerning_default, &kerning);
		if (error)
		{
			nlerror ("FT_Get_Kerning() failed: %s", getFT2Error(error));
		}
		kernx = kerning.x;
	}*/
}



uint32	 CFontGenerator::getCharIndex (ucchar c)
{
//	return FT_Get_Char_Index (_Face, c);
	return 0;
}


} // NL3D

#endif // NL_DONT_USE_EXTERNAL_CODE

