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



#ifndef NL_SCROLL_BAR_H
#define NL_SCROLL_BAR_H

#include "nel/misc/types_nl.h"
#include "control.h"
#include "nel/misc/rgba.h"



class CScrollableControl;

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CScrollBar : public CControl
{
public:

	///default  Constructor
	CScrollBar(uint id);

	/// constructor
	CScrollBar(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, bool vertical = true,CScrollableControl *ctrl = NULL);

	/// destructor
	virtual ~CScrollBar() {}

	/// Display the control
	virtual void display();

	/// Manage the click of the mouse for control
	virtual void click(float x, float y, bool &taken);

	/// set the number of unit
	void setNbUnit(uint32 nbUnits) { _NbUnits = nbUnits; }

	/// set the number of unit displayed
	void setNbDisplayedUnit(uint32 nbUnits) { _NbDisplayedUnit = nbUnits; }

	/**
	 * set the texture of the button 'up' when 'On'
	 * \param uint textureId
	 */
	void setUpArrowTextureOn( uint textureId) { _UpTextureOn = textureId; }

	/**
	 * set the texture of the button 'up' when 'Off'
	 * \param uint textureId
	 */
	void setUpArrowTextureOff( uint textureId) { _UpTextureOff = textureId; }

	/**
	 * set the texture of the button 'down' when 'On'
	 * \param uint textureId
	 */
	void setDownArrowTextureOn( uint textureId) { _DownTextureOn = textureId; }

	/**
	 * set the texture of the button 'down' when 'Off'
	 * \param uint textureId
	 */
	void setDownArrowTextureOff( uint textureId) { _DownTextureOff = textureId; }

	/**
	 * set the texture of the button 'left' when 'On'
	 * \param uint textureId
	 */
	void setLeftArrowTextureOn( uint textureId) { _LeftTextureOn = textureId; }

	/**
	 * set the texture of the button 'left' when 'Off'
	 * \param uint textureId
	 */
	void setLeftArrowTextureOff( uint textureId) { _LeftTextureOff = textureId; }

	/**
	 * set the texture of the button 'right' when 'On'
	 * \param uint textureId
	 */
	void setRightArrowTextureOn( uint textureId) { _RightTextureOn = textureId; }

	/**
	 * set the texture of the button 'right' when 'Off'
	 * \param uint textureId
	 */
	void setRightArrowTextureOff( uint textureId) { _RightTextureOff = textureId; }


	/// Change color when On.
	void colorOn(const NLMISC::CRGBA &color);

	/// Change color when Disable.
	void colorDisable(const NLMISC::CRGBA &color);

	/// Change texture when On.
	void textureOn(uint32 texture);

	/// Change texture when Disable.
	void textureDisable(uint32 texture);

	/// Get the state of the scroll bar (Enable/Disable).
	bool enable();

	/// Enable or Disable the scroll bar.
	void enable(bool e);

	/**
	* get the display size of the scroll bar (width if Vertical, Height if horizontal)
	*/
	float size() const;

private:
	/// init function
	void init();

// attributes
private:
	/// tells if the control, is vertical or horizontal
	bool				_Vertical;

	/// the scroll bar size (in units)
	uint32				_NbUnits;

	/// the current position of the slider
	uint32				_CurrentUnit;

	/// the number of displayed unit
	uint32				_NbDisplayedUnit;

	/// texture of the upper arrow 'on'
	uint				_UpTextureOn;
	/// texture of the upper arrow 'off'
	uint				_UpTextureOff;

	/// texture of the down arrow 'on'
	uint				_DownTextureOn;
	/// texture of the down arrow 'off'
	uint				_DownTextureOff;

	/// texture of the left arrow 'on'
	uint				_LeftTextureOn;
	/// texture of the left arrow 'off'
	uint				_LeftTextureOff;

	/// texture of the right arrow 'on'
	uint				_RightTextureOn;
	/// texture of the right arrow 'off'
	uint				_RightTextureOff;


	/// color of the scroll bar
	NLMISC::CRGBA		_ColorOn;
	/// color of the scroll bar when disabled
	NLMISC::CRGBA		_ColorDisable;
	/// texture of the scroll bar when disabled
	uint32				_TextureOn;
	/// texture of the scroll bar when disabled
	uint32				_TextureDisable;

	/// state of the scroll bar (enable/disable)
	bool				_Enable;

	/// the height (if vertical, width if horizontal) of the button, in proportion of th window (0-1)
	float				_ButtonDisplaySize;
};


#endif // NL_SCROLL_BAR_H

/* End of scroll_bar.h */
