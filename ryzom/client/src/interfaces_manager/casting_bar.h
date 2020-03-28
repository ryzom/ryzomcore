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



#ifndef CL_CASTING_BAR_H
#define CL_CASTING_BAR_H

#include "nel/misc/types_nl.h"
#include "progress_bar.h"



/**
 * class for casting bar : a progress bar but with auto modification of the position every  'x' 1/10s
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CCastingBar : public CProgressBar
{
public:
	/// Constructor
	CCastingBar(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint range)
		:CProgressBar(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, range)
	{
		_Running = false;
		_LastChangePosTime = ryzomGetLocalTime ();
		_AutoHide = true;
	}

	/**
	 * start the control
	 */
	inline void start()
	{
		_Running = true;
		_LastChangePosTime = ryzomGetLocalTime ();
	}

	/**
	 * stop the control
	 */
	inline void stop() { _Running = false; }

	/// display the control
	virtual void display();

	/// set the auto hide mode (hide the control when the pos == 0
	inline void autoHide( bool autoHide ) { _AutoHide = autoHide; }

private:
	/// true : the control is active, false : the control is stopped (no modification of the bar)
	bool	_Running;

	/// the time of the last modif of pos
	mutable NLMISC::TTime	_LastChangePosTime;

	/// time
	mutable NLMISC::TTime	_Time;

	/// auto hide mode
	bool	_AutoHide;

};


#endif // CL_CASTING_BAR_H

/* End of casting_bar.h */
