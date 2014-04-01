/**
 * \file stereo_ng_hmd.h
 * \brief IStereoNGHMD
 * \date 2014-04-01 10:53GMT
 * \author Jan Boon (Kaetemi)
 * IStereoNGHMD
 */

/* 
 * Copyright (C) 2014  by authors
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

#ifndef NL3D_STEREO_NG_HMD_H
#define NL3D_STEREO_NG_HMD_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include <nel/3d/stereo_hmd.h>

namespace NL3D {

/**
 * \brief IStereoNGHMD
 * \date 2014-04-01 10:53GMT
 * \author Jan Boon (Kaetemi)
 * IStereoNGHMD
 */
class IStereoNGHMD : public IStereoHMD
{
public:
	IStereoNGHMD();
	virtual ~IStereoNGHMD();
	
	/// Kill the player
	virtual void killUser() const = 0;
	
}; /* class IStereoNGHMD */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_NG_HMD_H */

/* end of file */
