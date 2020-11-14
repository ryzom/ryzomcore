/**
 * \file stereo_hmd.h
 * \brief IStereoHMD
 * \date 2013-06-27 16:30GMT
 * \author Jan Boon (Kaetemi)
 * IStereoHMD
 */

/* 
 * Copyright (C) 2013  by authors
 * 
 * This file is part of NL3D.
 * NL3D is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NL3D is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NL3D.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NL3D_STEREO_HMD_H
#define NL3D_STEREO_HMD_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include <nel/3d/stereo_display.h>

namespace NL3D {

/**
 * \brief IStereoHMD
 * \date 2013-06-27 16:30GMT
 * \author Jan Boon (Kaetemi)
 * IStereoHMD
 */
class IStereoHMD : public IStereoDisplay
{
public:
	IStereoHMD();
	virtual ~IStereoHMD();
	
	/// Get the HMD orientation
	virtual NLMISC::CQuat getOrientation() const = 0;

	/// Set the GUI reference
	virtual void setInterfaceMatrix(const NLMISC::CMatrix &matrix) = 0;

	/// Get GUI center (1 = width, 1 = height, 0 = center)
	virtual void getInterface2DShift(uint cid, float &x, float &y, float distance) const = 0;
	
	/// Set the head model, eye position relative to orientation point
	virtual void setEyePosition(const NLMISC::CVector &v) = 0;
	/// Get the head model, eye position relative to orientation point
	virtual const NLMISC::CVector &getEyePosition() const = 0;

	/// Set the scale of the game in units per meter
	virtual void setScale(float s) = 0;
	
}; /* class IStereoHMD */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_HMD_H */

/* end of file */
