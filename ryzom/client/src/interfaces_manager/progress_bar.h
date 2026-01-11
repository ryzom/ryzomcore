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



#ifndef NL_PROGRESS_BAR_H
#define NL_PROGRESS_BAR_H


/////////////
// Include //
/////////////
// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/rgba.h"
// Client
#include "control.h"
#include "pen.h"



/** class for progress bar controls
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CProgressBar : public CControl, public CPen
{
public:

	/// Constructor
	CProgressBar(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint range);

	/**
	 * set the range for the control
	 * \param uint32 range the new range
	 * \return uint32 the old range value
	 */
	uint32 setRange( uint32 range);

	/**
	 * get the range value of the progress control
	 * \return uint32 the range value
	 */
	inline uint32 getRange() const { return _Range; }

	/**
	 * set the step value
	 * \param uint32 step the new step value
	 * \return uint32 the old step value
	 */
	uint32 setStep( uint32 step);

	/**
	 * set the step value
	 * \param uint32 step the new step value
	 * \return uint32 the old step value
	 */
	inline uint32 getStep() const { return _StepInc; }

	/**
	 * set the position of the progress bar (from 0 to range)
	 * \param uint32 pos the new position
	 * \return uint32 the previous position of the progress bar control
	 */
	uint32 setPos( uint32 pos);

	/**
	 * get the position of the progress bar (from 0 to range)
	 * \return uint32 the position of the progress bar control
	 */
	inline uint32 getPos() const { return _CurrentPos; }

	/**
	 * Advances the current position for the progress bar control by the specified value
	 * \param uint32 offset the value added to current position
	 * \return uint32 the old position of the progress bar
	 */
	inline uint32 offsetPos( uint32 offset)
	{
		uint32 old = _CurrentPos;
		_CurrentPos += offset;
		if (_CurrentPos > _Range)
			_CurrentPos = _Range;
		return old;
	}

	/**
	 * Advances the current position for the progress bar control by the step increment
	 * \return uint32 the previous position of the progress bar control
	 */
	inline uint32 stepIt()
	{
		uint32 old = _CurrentPos;
		_CurrentPos += _StepInc;
		if (_CurrentPos > _Range)
			_CurrentPos = _Range;
		return old;
	}


	/**
	 * set the texture of the background
	 * \param uint texture the new texture for the background
	 */
	void setBackgroundTexture(uint texture) { _BackgroundTexture = texture; }

	/**
	 * set the color of the background
	 * \param CRGBA& color the new color for the background
	 */
	void setBackgroundColor(NLMISC::CRGBA &color) { _BackgroundColor = color; }

	/**
	 * set the texture of the progress bar
	 * \param uint texture the new texture for the progress bar
	 */
	void setProgressBarTexture(uint texture) { _ProgressBarTexture = texture; }

	/**
	 * set the color of the progress bar
	 * \param CRGBA& color the new color for the progress bar
	 */
	void setProgressBarColor(NLMISC::CRGBA &color) { _ProgressBarColor = color; }

	/// display the control
	virtual void display();

	/**
	 * set the smooth mode of the control
	 * \param bool smooth the smooth mode (true = smooth mode on)
	 */
	void smooth(bool smooth) { _Smooth = smooth; }

	/**
	 * get the smooth mode of the control
	 * \return bool the smooth mode (true = smooth mode on)
	 */
	bool smooth() const { return _Smooth; }

	/**
	 * set the smooth fill rate in milliseconds
	 * \param uint32 the smooth fill rate in milliseconds
	 */
	void smoothFillRate( uint32 rate) { _SmoothFillRate = rate; }


	/**
	 * get the smooth fill rate in milliseconds
	 * \return the smooth fill rate in milliseconds
	 */
	uint32 smoothFillRate() const { return _SmoothFillRate; }

	/**
	 * set the text displayed in the control
	 * \param std::string &text the new text
	 */
	inline void setText( const std::string &text) { _Text = text; }

private:
	/// the init method
	void init(uint32 range);

protected:

	/// the current 'position'
	uint32			_CurrentPos;

	/// the range of the bar (always start at 0)
	uint32			_Range;

	/// the step increment
	uint32			_StepInc;

	/// temporary position, used when smooth filling the control, only for internal use
	uint32			_TempPos;

// textures and colors
	/// background texture
	uint			_BackgroundTexture;
	/// background color
	NLMISC::CRGBA	_BackgroundColor;

	/// progress bar texture
	uint			_ProgressBarTexture;
	/// progress bar color
	NLMISC::CRGBA	_ProgressBarColor;

	/// text to display in the control
	std::string		_Text;

	/// the smooth mode (smooth transition beetween two positions true = ON) (default = false)
	bool			_Smooth;

	/// if in smooth mode, set the interval in ms (milliseconds) beetween the addition/soustraction of two units to the bar (defualt = 200ms = 5units/second)
	uint32			_SmoothFillRate;

	/// last update time for smooth fill
	mutable NLMISC::TTime	_LastUpdateSmooth;
};


#endif // NL_PROGRESS_BAR_H

/* End of progress_bar.h */
