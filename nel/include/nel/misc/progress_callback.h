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

#ifndef NL_PROGRESS_CALLBACK_H
#define NL_PROGRESS_CALLBACK_H

#include "types_nl.h"


namespace NLMISC {


/**
 * Progress callback interface
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class IProgressCallback
{
public:

	IProgressCallback ();
	virtual ~IProgressCallback () {}

	/**
	  * Call back
	  *
	  * progressValue should be 0 ~ 1
	  */
	virtual void progress (float progressValue);

	/**
	  * Push crop values
	  */
	void pushCropedValues (float min, float max);

	/**
	  * Push crop values
	  */
	void popCropedValues ();

	/**
	  * Get croped value
	  */
	float getCropedValue (float value) const;

public:

	/// Display string
	std::string		DisplayString;

private:

	class CCropedValues
	{
	public:
		CCropedValues (float min, float max)
		{
			Min = min;
			Max = max;
		}
		float	Min;
		float	Max;
	};

	std::vector<CCropedValues>	_CropedValues;
};


} // NLMISC


#endif // NL_PROGRESS_CALLBACK_H

/* End of progress_callback.h */
