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

#ifndef NL_FONT_GENERATOR_H
#define NL_FONT_GENERATOR_H

//#define NL_DONT_USE_EXTERNAL_CODE
#undef NL_DONT_USE_EXTERNAL_CODE

#ifndef NL_DONT_USE_EXTERNAL_CODE

// forward declarations to avoid including freetype.h in headers
typedef int FT_Error;
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_* FT_Face;

#else // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_DONT_USE_EXTERNAL_CODE

#include "nel/misc/types_nl.h"
#include <string>


namespace NL3D {


/**
 * Generate bitmap based on a true type font (using freetype2)
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CFontGenerator
{
public:

	/** Constructor
	 * \param fontFileName path+filename (ex: "c:\winnt\fonts\arial.ttf")
	 */
	CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName = "");

	virtual ~CFontGenerator ();

	/** generate and return a bitmap
	 * \param c the unicode char
	 * \param size size of the generated font in ??? format
	 * \param embolden set embolden style (bold)
	 * \param oblique set oblique style (slanted, italic)
	 * \param width width of the generated bitmap, this value is set by this function
	 * \param height height of the generated bitmap, this value is set by this function
	 * \param pitch pitch of the generated bitmap (+ or - the number of bytes per row), this value is set by this function
	 */
	uint8	*getBitmap (ucchar c, uint32 size, bool embolden, bool oblique, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex);

	/** returns the width and height of a character using a specific size and
	 *
	 * \warning this function is not very fast (but faster than getBitmap()) because it has to load the char before.
	 */
	void	getSizes (ucchar c, uint32 size, uint32 &width, uint32 &height);

	void	getKerning (ucchar left, ucchar right, sint32 &kernx);

	uint32	getCharIndex (ucchar c);

	uint32	getUID() { return _UID; }

	std::string getFontFileName() const;

private:

	static uint32 _FontGeneratorCounterUID;
	uint32			_UID;
	std::string		_FontFileName;

#ifndef NL_DONT_USE_EXTERNAL_CODE
	const char			*getFT2Error(FT_Error fte);

	static FT_Library	_Library;
	static uint			_LibraryInit;

	FT_Face				_Face;
#else // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_DONT_USE_EXTERNAL_CODE
};

} // NL3D


#endif // NL_FONT_GENERATOR_H

/* End of font_generator.h */
