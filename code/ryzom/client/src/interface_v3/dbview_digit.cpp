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

#include "dbview_digit.h"
#include "interface_manager.h"
#include "game_share/xml_auto_ptr.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CDBViewDigit, std::string, "digit");

// ***************************************************************************
CDBViewDigit::CDBViewDigit(const TCtorParam &param)
: CViewBase(param)
{
	_NumDigit= 2;
	_WSpace= -1;
	_DivBase= 1;
}


// ***************************************************************************
bool CDBViewDigit::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if(!CViewBase::parse(cur, parentGroup))
		return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	// link to the db
	CXMLAutoPtr ptr;
	ptr = xmlGetProp (cur, (xmlChar*)"value");
	if ( ptr )
		_Number.link ( ptr );
	else
	{
		nlinfo ("no value in %s", _Id.c_str());
		return false;
	}

	// read options
	ptr = xmlGetProp (cur, (xmlChar*)"numdigit");
	if(ptr)	fromString((const char*)ptr, _NumDigit);
	clamp(_NumDigit, 1, 10);

	ptr = xmlGetProp (cur, (xmlChar*)"wspace");
	if(ptr)	fromString((const char*)ptr, _WSpace);

	ptr= (char*) xmlGetProp( cur, (xmlChar*)"color" );
	_Color = CRGBA(255,255,255,255);
	if (ptr)
		_Color = convertColor (ptr);

	// compute window size. Remove one space.
	sint32	wDigit= rVR.getFigurTextureW();
	sint32	hDigit= rVR.getFigurTextureH();
	setW((wDigit+_WSpace)*_NumDigit - _WSpace);
	setH(hDigit);

	// some init
	// For _NumDigit=2; set the divBase to 100, etc...
	_DivBase= 1;
	for(uint i= 0;i<(uint)_NumDigit;i++)
	{
		_DivBase*= 10;
	}

	// init cache.
	_Cache= -1;

	return true;
}


// ***************************************************************************
void CDBViewDigit::updateCoords()
{
	CViewBase::updateCoords();
}


// ***************************************************************************
void CDBViewDigit::draw ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();
	sint32	wDigit= rVR.getFigurTextureW();
	sint32	hDigit= rVR.getFigurTextureH();

	// change bitmap ids
	sint32	val= _Number.getSInt32();
	val= max(val, sint32(0));
	if(_Cache!=val)
	{
		_Cache= val;
		// clamp the value to max possible (eg:99)
		if(val>(sint32)_DivBase-1)
			val=(sint32)_DivBase-1;
		// compute each digit id
		uint	divisor= _DivBase/10;
		for(sint i=0;i<_NumDigit;i++)
		{
			sint digitVal= (val/divisor)%10;
			// set the digit text id
			_DigitId[i]= rVR.getFigurTextureId(digitVal);
			// next divisor
			divisor/= 10;
		}
	}

	// Display bitmaps
	sint32	x= _XReal;
	sint32	y= _YReal;
	for(sint i=0;i<_NumDigit;i++)
	{
		rVR.drawRotFlipBitmap (	_RenderLayer, x, y, wDigit, hDigit, 0, false, _DigitId[i], _Color );
		// next digit
		x+= wDigit+_WSpace;
	}

}
