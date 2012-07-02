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

#ifndef NL_FONT_MANAGER_H
#define NL_FONT_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/texture.h"
#include "nel/3d/material.h"
#include "nel/3d/texture_font.h"

#include <map>
#include <list>
#include <functional>




namespace NL3D {

class  CFontGenerator;
struct CComputedString;

/**
 * Font manager
 * The font manager manages CMaterial pointers through a list
 * of CSmartPtr. When the user asks for the texture font representing
 * a character(font/size), it generates and stores this pointer in the list.
 * If this character has already been generated, and lies in the list,
 * it increments its reference count.
 * If the memory used by generated textures exceeds the max memory,
 * then the useless character/pointer is erased from the list.
 * Max memory is set to 0 by default, so this value should be set to non-zero
 * before generating textures to prevent immediate memory deletion.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CFontManager
{
	uint32 _MemSize;
	uint32 _MaxMemory;
	uint32 _NbChar;

	CSmartPtr<CMaterial> _MatFont;
	CSmartPtr<CTextureFont>	_TexFont;

public:

	/**
	 * Default constructor
	 */
	CFontManager()
	{
		_MemSize = 0;
		_MaxMemory = 1000000;
		_NbChar = 0;
		_MatFont = NULL;
		_TexFont = NULL;
	}


	/**
	 * define maximum memory allowed
	 * \param maximum memory
	 */
	void setMaxMemory(uint32 mem) { _MaxMemory = mem; }


	/**
	 * gives maximum memory allowed
	 * \return maximum memory
	 */
	uint32 getMaxMemory() const { return _MaxMemory; }

	/**
	 * manages fonts in memory using CSmartPtr
	 * \param character descriptor
	 * \return CSmartPtr to a font texture
	 */
	CMaterial* getFontMaterial();


	/**
	 * Compute primitive blocks and materials of each character of
	 * the string.
	 * \param s string to compute
	 * \param fontGen font generator
	 * \param color primitive blocks color
	 * \param fontSize font size
	 * \param desc display descriptor (screen size, font ratio)
	 * \param output computed string
	 * \param keep800x600Ratio true if you want that CFontManager look at Driver window size, and resize fontSize so it keeps same size...
	 */
	void computeString (const std::string& s,
						CFontGenerator *fontGen,
						const NLMISC::CRGBA &color,
						uint32 fontSize,
					    IDriver *driver,
						CComputedString& output,
						bool	keep800x600Ratio= true);

	/**
	 * Same as computeString but works with a unicode string (ucstring)
	 */
	void computeString (const ucstring &s,
						CFontGenerator *fontGen,
						const NLMISC::CRGBA &color,
						uint32 fontSize,
					    IDriver *driver,
						CComputedString &output,
						bool	keep800x600Ratio= true);

	/**
	 * Same as computeString but do not make vertex buffers and primitives
	 */
	void computeStringInfo (const ucstring &s,
							CFontGenerator *fontGen,
							const NLMISC::CRGBA &color,
							uint32 fontSize,
							IDriver *driver,
							CComputedString &output,
							bool keep800x600Ratio= true);


	/**
	 * return a string given information about the cache
	 */
	std::string getCacheInformation() const;

	void	dumpCache (const char *filename)
	{
		_TexFont->dumpTextureFont (filename);
	}

	/**
	* invalidate the texture when the text context has been modified
	*/
	void invalidate();

};



} // NL3D


#endif // NL_FONT_MANAGER_H

/* End of font_manager.h */
