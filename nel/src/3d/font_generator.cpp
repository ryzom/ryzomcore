// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"

#include "nel/3d/font_generator.h"

using namespace std;

#ifndef NL_DONT_USE_EXTERNAL_CODE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H

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

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

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

std::string CFontGenerator::getFontFileName() const
{
	return _FontFileName;
}

CFontGenerator *newCFontGenerator(const std::string &fontFileName)
{
	return new CFontGenerator(fontFileName);
}

// Freetype will call this function to get a buffer in data
static unsigned long nlFreetypeStreamIo(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
	// if count is 0, we don't need to do anything
	if (count > 0)
	{
		// get a pointer on our CIFile
		CIFile *file = (CIFile*)stream->descriptor.pointer;

		// try to seek to offset
		if (file->seek(offset, IStream::begin))
		{
			try
			{
				// try to fill buffer with data from file
				file->serialBuffer(buffer, count);
			}
			catch(const EFile &e)
			{
				nlwarning("Unable to read %u bytes from position %u of %s", (uint)count, (uint)offset, file->getStreamName().c_str());
				count = 0;
			}
		}
		else
		{
			nlwarning("Unable to seek to position %u of %s", (uint)offset, file->getStreamName().c_str());
			count = 0;
		}
	}

	return count;
}

// Freetype will call this function when it won't need to access to file anymore
static void nlFreetypeStreamClose(FT_Stream stream)
{
	if (!stream) return;

	// get a pointer on our CIFile
	CIFile *file = (CIFile*)stream->descriptor.pointer;

	if (file)
	{
		// close and delete file
		file->close();
		delete file;

		stream->descriptor.pointer = NULL;
	}

	// free Freetype stream structure
	free(stream);
}

// helper to open a font and use our functions to handle BNP files and UTF-8 filenames
static bool createFreetypeStream(const std::string &filename, FT_Open_Args &args)
{
	CIFile *file = new CIFile();

	if (!file->open(filename))
	{
		nlwarning("Unable to open %s", filename.c_str());
		return false;
	}

	args.flags = FT_OPEN_STREAM;
	args.stream = (FT_Stream)malloc(sizeof(*args.stream));

	if (args.stream == NULL)
	{
		nlwarning("Unable to allocate FT_Stream for %s", filename.c_str());

		delete file;
		return false;
	}

	args.stream->base = NULL; // only used for memory streams
	args.stream->size = file->getFileSize();
	args.stream->pos = 0;
	args.stream->descriptor.pointer = file;
	args.stream->pathname.pointer = NULL; // filename is already managed by CIFile
	args.stream->read = nlFreetypeStreamIo;
	args.stream->close = nlFreetypeStreamClose;

	return true;
}

/*
 * Constructor
 */
CFontGenerator::CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName)
{
	_UID = _FontGeneratorCounterUID;
	_FontGeneratorCounterUID++;
	_FontFileName = fontFileName;

	if (NLMISC::startsWith(fontFileName, "ui"))
	{
		std::string nameList = CI18N::get(fontFileName);
		NLMISC::explode(nameList, string("\n"), _FontFileNames, true);
		for (std::vector<std::string>::iterator it(_FontFileNames.begin()), end(_FontFileNames.end()); it != end; ++it)
		{
			*it = CPath::lookup(*it);
		}
	}
	else
	{
		_FontFileNames.push_back(fontFileName);
	}

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

	for (std::vector<std::string>::iterator it(_FontFileNames.begin()), end(_FontFileNames.end()); it != end; ++it)
	{
		FT_Open_Args args;
		const std::string &fileName = *it;

		if (!createFreetypeStream(fileName, args))
		{
			nlerror("createFreetypeStream failed with file '%s'", fileName.c_str());
		}

		FT_Face face;
		error = FT_Open_Face(_Library, &args, 0, &face);
		if (error)
		{
			nlerror("FT_New_Face() failed with file '%s': %s", fileName.c_str(), getFT2Error(error));
		}

		string fontEx = fontExFileName;
		if (fontEx.empty())
		{
			// try to see if the ex filename exists based on the fontExFileName
			fontEx = CPath::lookup(CFile::getFilenameWithoutExtension(fileName) + ".afm", false, false);
		}

		if (!fontEx.empty())
		{
			if (!createFreetypeStream(fontEx, args))
			{
				nlerror("createFreetypeStream failed with file '%s'", fileName.c_str());
				FT_Done_Face(face);
				continue;
			}

			error = FT_Attach_Stream(face, &args);
			if (error)
			{
				nlwarning("FT_Attach_File() failed with file '%s': %s", fontEx.c_str(), getFT2Error(error));
				FT_Done_Face(face);
				continue;
			}
		}

		error = FT_Select_Charmap(face, ft_encoding_unicode);
		if (error)
		{
			nlerror("FT_Select_Charmap() failed with file '%s': %s", fileName.c_str(), getFT2Error(error));
			FT_Done_Face(face);
			continue;
		}

		_Faces.push_back(face);
	}
}

CFontGenerator::~CFontGenerator ()
{
	for (std::vector<FT_Face>::iterator it(_Faces.begin()), end(_Faces.end()); it != end; ++it)
		FT_Done_Face(*it);
	_Faces.clear();

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

void CFontGenerator::getSizes (u32char c, uint32 size, uint32 &width, uint32 &height)
{
	FT_Error error;
	FT_Face face;
	FT_UInt glyph_index = 0;

	if (!_Faces.size())
	{
		nlerror("No faces loaded");
		return;
	}

	for (std::vector<FT_Face>::iterator it(_Faces.begin()), end(_Faces.end()); it != end; ++it)
	{
		face = *it;
		error = FT_Set_Pixel_Sizes(face, size, size);
		if (error)
		{
			nlerror("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
			continue;
		}

		// retrieve glyph index from character code
		glyph_index = FT_Get_Char_Index(face, c);

		// found glyph
		if (glyph_index)
			break;
	}

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		// use fallback for glyph/character errors (composite char limit for example)
		nlwarning ("FT_Load_Glyph() failed: %s", getFT2Error(error));

		error = FT_Load_Glyph(face, 0, FT_LOAD_DEFAULT);
		if (error)
		{
			nlerror("FT_Load_Glyph() fallback failed: %s", getFT2Error(error));
		}
	}

	// convert 24.6 fixed point into integer
	width = face->glyph->metrics.width >> 6;
	height = face->glyph->metrics.height >> 6;
}

uint8 *CFontGenerator::getBitmap (u32char c, uint32 size, bool embolden, bool oblique, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex)
{
	FT_Error error;
	FT_Face face;
	FT_UInt glyph_index = 0;

	if (!_Faces.size())
	{
		nlerror("No faces loaded");
		return NULL;
	}

	for (std::vector<FT_Face>::iterator it(_Faces.begin()), end(_Faces.end()); it != end; ++it)
	{
		face = *it;
		error = FT_Set_Pixel_Sizes(face, size, size);
		if (error)
		{
			nlerror("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
			continue;
		}

		// retrieve glyph index from character code
		glyph_index = FT_Get_Char_Index(face, c);

		// found glyph
		if (glyph_index)
			break;
	}

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		// use fallback for glyph/character errors (composite char limit for example)
		nlwarning ("FT_Load_Glyph() failed: %s", getFT2Error(error));

		error = FT_Load_Glyph (face, 0, FT_LOAD_DEFAULT);
		if (error)
		{
			nlerror("FT_Load_Glyph() fallback failed: %s", getFT2Error(error));
		}
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

	if (embolden)
	{
		FT_GlyphSlot_Embolden(face->glyph);
	}

	if (oblique)
	{
		FT_GlyphSlot_Oblique(face->glyph);
	}

	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph (face->glyph, ft_render_mode_normal);
	if (error)
	{
		nlerror ("FT_Render_Glyph() failed: %s", getFT2Error(error));
	}

	width = face->glyph->bitmap.width;
	height = face->glyph->bitmap.rows;
	pitch = face->glyph->bitmap.pitch;

	left = face->glyph->bitmap_left;
	top = face->glyph->bitmap_top;

	advx = face->glyph->advance.x >> 6;

	glyphIndex = glyph_index;

	return (uint8 *) face->glyph->bitmap.buffer;
}



void CFontGenerator::getKerning (u32char left, u32char right, sint32 &kernx)
{
	if (!FT_HAS_KERNING(_Faces[0]))
	{
		kernx = 0;
	}
	else
	{
		// This is currently not used...

		FT_Vector  kerning;
		FT_Error error = FT_Get_Kerning (_Faces[0], left, right, ft_kerning_default, &kerning);
		if (error)
		{
			nlerror ("FT_Get_Kerning() failed: %s", getFT2Error(error));
		}
		kernx = kerning.x;
	}
}



uint32	 CFontGenerator::getCharIndex (u32char c)
{
	// This is currently not used...

	uint32 ret = FT_Get_Char_Index(_Faces[0], c);

	if (ret == 0)
	{
		// no glyph available, replace with a dot
		ret = FT_Get_Char_Index (_Faces[0], u32char('.'));
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
}

CFontGenerator::~CFontGenerator ()
{
	DeleteObject (Dib);

	DeleteDC (hdcDib);
}

void CFontGenerator::getSizes (u32char c, uint32 size, uint32 &width, uint32 &height)
{
}

HFONT hFont = NULL;
uint32 CurrentFontSize = 0;

uint8 *CFontGenerator::getBitmap (u32char c, uint32 size, bool embolden, bool oblique, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex)
{
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

	const u32char cc = /*(char)*/ c;

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




void CFontGenerator::getKerning (u32char left, u32char right, sint32 &kernx)
{
}



uint32	 CFontGenerator::getCharIndex (u32char c)
{
	return 0;
}


} // NL3D

#endif // NL_DONT_USE_EXTERNAL_CODE

