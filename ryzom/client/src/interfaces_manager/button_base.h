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



#ifndef NL_BUTTON_BASE_H
#define NL_BUTTON_BASE_H


/////////////
// Include //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"


///////////
// Using //
///////////
using NLMISC::CRGBA;


/**
 * Class to be the basis of a button.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CButtonBase
{
public:
	enum TBG	// BG = Background.
	{
		BG_none = 0,	// NO BG.
		BG_plain,		// BG is only made with 1 color (RGBA).
		BG_stretch		// BG is a stretched Bitmap and 1 color (RGBA).
	};

private:
	inline void init(uint tOn, uint tOff, uint tDisable, const CRGBA &on, const CRGBA &off, const CRGBA &disable);

protected:
	bool	_On;
	bool	_Enable;
	CRGBA	_ColorOn;
	CRGBA	_ColorOff;
	CRGBA	_ColorDisable;
	uint	_TextureOn;
	uint	_TextureOff;
	uint	_TextureDisable;
	TBG		_BGModeOn;
	TBG		_BGModeOff;
	TBG		_BGModeDisable;

public:
	/// Constructor
	CButtonBase();
	CButtonBase(uint tOn, uint tOff, uint tDisable);
	CButtonBase(const CRGBA &on, const CRGBA &off, const CRGBA &disable);
	CButtonBase(uint tOn, uint tOff, uint tDisable, const CRGBA &on, const CRGBA &off, const CRGBA &disable);

	/// \name accessors for writing
	// @{
	/// Change color when On.
	void colorOn(const CRGBA &color);
	/// Change color when Off.
	void colorOff(const CRGBA &color);
	/// Change color when Disable.
	void colorDisable(const CRGBA &color);

	/// Change texture when On.
	void textureOn(uint texture);
	/// Change texture when Off.
	void textureOff(uint texture);
	/// Change texture when Disable.
	void textureDisable(uint texture);

	/// Change the mode to display the background of the button when On.
	void bgModeOn(const TBG &mode) {_BGModeOn = mode;}
	/// Change the mode to display the background of the button when Off.
	void bgModeOff(const TBG &mode) {_BGModeOff = mode;}
	/// Change the mode to display the background of the button when Disable.
	void bgModeDisable(const TBG &mode) {_BGModeDisable = mode;}
	// @}

	/// \name accessors for reading
	// @{
	/// get color when On.
	inline const CRGBA &colorOn() const { return _ColorOn; }
	/// get color when Off.
	inline const CRGBA &colorOff() const { return _ColorOff; }
	/// get color when Disable.
	inline const CRGBA &colorDisable() const { return _ColorDisable; }

	/// get texture when On.
	inline uint textureOn() const { return _TextureOn; }
	/// get texture when Off.
	inline uint textureOff() const { return _TextureOff; }
	/// get texture when Disable.
	inline uint textureDisable() const { return _TextureDisable; }

	/// get the background display mode when On.
	inline const TBG & bgModeOn() const { return _BGModeOn; }
	/// get the background display mode when Off.
	inline const TBG & bgModeOff() const { return _BGModeOff; }
	/// get the background display mode when Disable.
	inline const TBG & bgModeDisable() const { return _BGModeDisable; }
	// @}


	/// Get the state of the button(Enable/Disable).
	bool enable();
	/// Enable or Disable the Button.
	void enable(bool e);

	/// return true if the button is selected
	bool isSelected() const { return _On; }

	/// Select the button.
	virtual void select();

	/// Un-Select the button.
	virtual void unSelect();
};


#endif // NL_BUTTON_BASE_H

/* End of button_base.h */
