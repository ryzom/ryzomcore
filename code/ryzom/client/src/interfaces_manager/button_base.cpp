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

#include "button_base.h"


//-----------------------------------------------
// CButtonBase :
// Constructor.
//-----------------------------------------------
CButtonBase::CButtonBase()
{
	init(0, 0, 0, CRGBA(255,0,0), CRGBA(0,255,0), CRGBA(100,100,100));
}// CButtonBase //
//-----------------------------------------------
// CButtonBase :
// Constructor.
//-----------------------------------------------
CButtonBase::CButtonBase(uint tOn, uint tOff, uint tDisable)
{
	init(tOn, tOff, tDisable, CRGBA(255,0,0), CRGBA(0,255,0), CRGBA(100,100,100));
}// CButtonBase //
//-----------------------------------------------
// CButtonBase :
// Constructor.
//-----------------------------------------------
CButtonBase::CButtonBase(const CRGBA &on, const CRGBA &off, const CRGBA &disable)
{
	init(0, 0, 0, on, off, disable);
}// CButtonBase //
//-----------------------------------------------
// CButtonBase :
// Constructor.
//-----------------------------------------------
CButtonBase::CButtonBase(uint tOn, uint tOff, uint tDisable, const CRGBA &on, const CRGBA &off, const CRGBA &disable)
{
	init(tOn, tOff, tDisable, on, off, disable);
}// CButtonBase //

//-----------------------------------------------
// init :
// Initialize the class(only 1 function for all constructor -> easier).
//-----------------------------------------------
void CButtonBase::init(uint tOn, uint tOff, uint tDisable, const CRGBA &on, const CRGBA &off, const CRGBA &disable)
{
	_On				= false;
	_Enable			= true;
	_TextureOn		= tOn;
	_TextureOff		= tOff;
	_TextureDisable	= tDisable;
	_ColorOn		= on;
	_ColorOff		= off;
	_ColorDisable	= disable;
	_BGModeOn		= BG_none;
	_BGModeOff		= BG_none;
	_BGModeDisable	= BG_none;

}// init //


//-----------------------------------------------
// textureOn :
// Change texture when On.
//-----------------------------------------------
void CButtonBase::textureOn(uint texture)
{
	_TextureOn = texture;
}// textureOn //
//-----------------------------------------------
// textureOff :
// Change texture when Off.
//-----------------------------------------------
void CButtonBase::textureOff(uint texture)
{
	_TextureOff = texture;
}// textureOff //
//-----------------------------------------------
// textureDisable :
// Change texture when Disable.
//-----------------------------------------------
void CButtonBase::textureDisable(uint texture)
{
	_TextureDisable = texture;
}// textureDisable //


//-----------------------------------------------
// colorOn :
// Change color when On.
//-----------------------------------------------
void CButtonBase::colorOn(const CRGBA &color)
{
	_ColorOn = color;
}// colorOn //
//-----------------------------------------------
// colorOff :
// Change color when Off.
//-----------------------------------------------
void CButtonBase::colorOff(const CRGBA &color)
{
	_ColorOff = color;
}// colorOff //
//-----------------------------------------------
// colorDisable :
// Change color when Disable.
//-----------------------------------------------
void CButtonBase::colorDisable(const CRGBA &color)
{
	_ColorDisable = color;
}// colorDisable //


//-----------------------------------------------
// push :
// Push the button.
//-----------------------------------------------
void CButtonBase::select()
{
	_On = true;
}// push //

//-----------------------------------------------
// unPush :
// Un-push the button.
//-----------------------------------------------
void CButtonBase::unSelect()
{
	_On = false;
}// unPush //


//-----------------------------------------------
// enable :
// Get the state of the button(Enable/Disable).
//-----------------------------------------------
bool CButtonBase::enable()
{
	return _Enable;
}// enable //

//-----------------------------------------------
// enable :
// Enable or Disable the Button.
//-----------------------------------------------
void CButtonBase::enable(bool e)
{
	_Enable = e;
}// enable //
