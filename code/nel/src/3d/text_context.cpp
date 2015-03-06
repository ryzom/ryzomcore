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

#include "nel/3d/text_context.h"
#include "nel/3d/font_generator.h"

namespace NL3D {

// ------------------------------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------------------------------
CTextContext::CTextContext()
{
	_Driver = NULL;
	_FontManager = NULL;
	_FontGen = NULL;

	_FontSize = 12;

	_Color = NLMISC::CRGBA(0,0,0);

	_HotSpot = CComputedString::BottomLeft;

	_ScaleX = 1.0f;
	_ScaleZ = 1.0f;

	_Shaded = false;
	_ShadeOutline = false;
	_ShadeExtent = 0.001f;
	_ShadeColor = NLMISC::CRGBA(0,0,0);

	_Keep800x600Ratio= true;

	_CacheNbFreePlaces = 0;
}

// ------------------------------------------------------------------------------------------------
CTextContext::~CTextContext()
{
	if (_FontGen)
		delete _FontGen;
	if (_FontManager)
		_FontManager->invalidate();
}

// ------------------------------------------------------------------------------------------------
uint32 CTextContext::textPush (const char *format, ...)
{
	nlassert(_FontGen);

	// convert the string.
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	if (_CacheNbFreePlaces == 0)
	{
		CComputedString csTmp;

		_CacheStrings.push_back (csTmp);
		if (_CacheFreePlaces.size() == 0)
			_CacheFreePlaces.resize (1);
		_CacheFreePlaces[0] = (uint32)_CacheStrings.size()-1;
		_CacheNbFreePlaces = 1;
	}

	// compute the string.
	uint32 index = _CacheFreePlaces[_CacheNbFreePlaces-1];
	CComputedString &strToFill = _CacheStrings[index];
	_FontManager->computeString (str, _FontGen, _Color, _FontSize, _Driver, strToFill, _Keep800x600Ratio);

	_CacheNbFreePlaces--;

	return index;
}

// ------------------------------------------------------------------------------------------------
uint32 CTextContext::textPush (const ucstring &str)
{
	nlassert(_FontGen);

	if (_CacheNbFreePlaces == 0)
	{
		CComputedString csTmp;

		_CacheStrings.push_back (csTmp);
		if (_CacheFreePlaces.size() == 0)
			_CacheFreePlaces.resize (1);
		_CacheFreePlaces[0] = (uint32)_CacheStrings.size()-1;
		_CacheNbFreePlaces = 1;
	}

	// compute the string.
	uint32 index = _CacheFreePlaces[_CacheNbFreePlaces-1];
	nlassert (index < _CacheStrings.size());
	CComputedString &strToFill = _CacheStrings[index];
	_FontManager->computeString (str, _FontGen, _Color
		, _FontSize, _Driver, strToFill, _Keep800x600Ratio);

	_CacheNbFreePlaces--;

	return index;
}

// ------------------------------------------------------------------------------------------------
void CTextContext::erase (uint32 i)
{
	nlassertex ((i < _CacheStrings.size()), ("try to erase an unknown text"));
	_CacheStrings[i].LetterColors.clear();
	if (_CacheFreePlaces.size() == _CacheNbFreePlaces)
	{
		_CacheFreePlaces.push_back (i);
	}
	else
	{
		_CacheFreePlaces[_CacheNbFreePlaces] = i;
	}
	_CacheNbFreePlaces++;
}


// ------------------------------------------------------------------------------------------------
void CTextContext::clear ()
{
	_CacheFreePlaces.clear();
	_CacheNbFreePlaces = 0;
	_CacheStrings.clear();
}


// ------------------------------------------------------------------------------------------------
void CTextContext::setFontGenerator(const std::string &fontFileName, const std::string &fontExFileName)
{
	CFontGenerator *oldFontGen = _FontGen;
	_FontGen = new NL3D::CFontGenerator(fontFileName, fontExFileName);
	if (oldFontGen) delete oldFontGen;
}

// ------------------------------------------------------------------------------------------------
void CTextContext::setLetterColors(CLetterColors * letterColors, uint index)
{
	if(/*index>=0 &&*/ index<_CacheStrings.size())
	{
		_CacheStrings[index].LetterColors.clear();
		_CacheStrings[index].LetterColors = *letterColors;
	}
}

// ------------------------------------------------------------------------------------------------
bool CTextContext::isSameLetterColors(CLetterColors * letterColors, uint index)
{
	if(/*index>=0 &&*/ index<_CacheStrings.size())
	{
		CLetterColors & strLetterColors = _CacheStrings[index].LetterColors;
		return strLetterColors.isSameLetterColors(letterColors);
	}

	return false;
}

} // NL3D
