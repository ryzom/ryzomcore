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

#include "stdmisc.h"

#include "nel/misc/progress_callback.h"
#include "nel/misc/debug.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

float IProgressCallback::getCropedValue (float value) const
{
	nlassert (_CropedValues.size ()>0);
	const CCropedValues &values = _CropedValues.back ();
	return value*(values.Max-values.Min)+values.Min;
}

IProgressCallback::IProgressCallback ()
{
	_CropedValues.push_back (CCropedValues (0, 1));
}

void IProgressCallback::pushCropedValues (float min, float max)
{
	nlassert (_CropedValues.size ()>0);
	//const CCropedValues &values = _CropedValues.back ();
	_CropedValues.push_back (CCropedValues (getCropedValue (min), getCropedValue (max)));
}

void IProgressCallback::popCropedValues ()
{
	nlassert (_CropedValues.size ()>1);
	_CropedValues.pop_back ();
}

void IProgressCallback::progress (float /* progressValue */)
{
}

} // NLMISC
