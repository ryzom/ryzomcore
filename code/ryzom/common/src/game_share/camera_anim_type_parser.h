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

#ifndef RY_CAMERAANIMTYPEPARSER_H
#define RY_CAMERAANIMTYPEPARSER_H

#include "nel/misc/vector.h"
#include "nel/misc/stream.h"
#include <math.h>

namespace NLMISC
{

/// Function that serializes durations (in seconds)
/// Duration is converted to 1/100 second and cannot be greater than 2^12-1 = 4095
/// Result is serialized in 12 bits
inline void serialDuration(NLMISC::CBitMemStream &f, float& duration)
{
	static uint32 maxVal = (uint32)pow((float)2, 12) - 1;

	if (f.isReading())
	{
		uint32 d = 0;

		f.serial(d, 12);

		duration = (float)d / 100.f;
	}
	else
	{
		uint32 d = (uint32)(duration * 100);
		if (d > maxVal)
			d = maxVal;

		f.serial(d, 12);
	}
}

/// Function that serializes distances (in meters)
/// Distance is converted to centimeters and cannot be greater than 2^16-1 = 65535
/// Result is serialized in 2 bytes
inline void serialDistance(NLMISC::CBitMemStream &f, float& distance)
{
	static uint32 maxVal = (uint32)pow((float)2, 16) - 1;

	if (f.isReading())
	{
		uint32 d = 0;

		f.serial(d, 16);

		distance = (float)d / 100.f;
	}
	else
	{
		uint32 d = (uint32)(distance * 100);
		if (d > maxVal)
			d = maxVal;

		f.serial(d, 16);
	}
}

/// Function that serializes speeds (in m/s)
/// Speed is converted to cm/s and cannot be greater than 2^12-1 = 4095
/// Result is serialized in 12 bits
inline void serialSpeed(NLMISC::CBitMemStream &f, float& speed)
{
	static uint32 maxVal = (uint32)pow((float)2, 12) - 1;

	if (f.isReading())
	{
		uint32 d = 0;

		f.serial(d, 12);

		speed = (float)d / 100.f;
	}
	else
	{
		uint32 d = (uint32)(speed * 100);
		if (d > maxVal)
			d = maxVal;

		f.serial(d, 12);
	}
}

/// Serializes the difference between 2 positions
/// The difference is converted in cm and cannot be greater than 2^15-1 = 32767
/// Result is serialized in 3 x 2 bytes = 6 bytes
inline void serialPositionDifference(NLMISC::CBitMemStream &f, NLMISC::CVector& diffPos)
{
	if (f.isReading())
	{
		sint16 x = 0;
		sint16 y = 0;
		sint16 z = 0;

		f.serial(x);
		f.serial(y);
		f.serial(z);

		diffPos.x = (float)(x) / 100.f;
		diffPos.y = (float)(y) / 100.f;
		diffPos.z = (float)(z) / 100.f;
	}
	else
	{
		sint16 x = (sint16)(diffPos.x * 100.f);
		sint16 y = (sint16)(diffPos.y * 100.f);
		sint16 z = (sint16)(diffPos.z * 100.f);

		f.serial(x);
		f.serial(y);
		f.serial(z);
	}
}

}

#endif /* RY_CAMERAANIMTYPEPARSER_H */
