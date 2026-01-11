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

#include "pen.h"


//-----------------------------------------------
// CPen :
// Constructor.
//-----------------------------------------------
CPen::CPen()
{
	_FontSize	= 20;
	_Color		= CRGBA(255,255,255,255);
	_Shadow		= false;
}// CPen //

//-----------------------------------------------
// CPen :
// Constructor
//-----------------------------------------------
CPen::CPen(uint32 fontSize, CRGBA color, bool shadow)
{
	_FontSize	= fontSize;
	_Color		= color;
	_Shadow		= shadow;
}// CPen //


//-----------------------------------------------
// fontSize :
// Get the font size.
//-----------------------------------------------
uint32 CPen::fontSize() const
{
	return _FontSize;
}// fontSize //
//-----------------------------------------------
// fontSize :
// Set the font size.
//-----------------------------------------------
void CPen::fontSize(uint32 fs)
{
	_FontSize = fs;
}// fontSize //

//-----------------------------------------------
// color :
// Get the pen color.
//-----------------------------------------------
CRGBA CPen::color() const
{
	return _Color;
}// color //
//-----------------------------------------------
// color :
// Set the pen color.
//-----------------------------------------------
void CPen::color(CRGBA color)
{
	_Color = color;
}// color //

//-----------------------------------------------
// shadow :
// Get the shadow state.
//-----------------------------------------------
bool CPen::shadow() const
{
	return _Shadow;
}// shadow //
//-----------------------------------------------
// shadow :
// Set the shadow state.
//-----------------------------------------------
void CPen::shadow(bool s)
{
	_Shadow = s;
}// shadow //
