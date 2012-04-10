/**
 * \file u_group_controller.h
 * \brief UGroupController
 * \date 2012-04-10 12:49GMT
 * \author Jan Boon (Kaetemi)
 * UGroupController
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NLSOUND_U_GROUP_CONTROLLER_H
#define NLSOUND_U_GROUP_CONTROLLER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace NLSOUND {

/**
 * \brief UGroupController
 * \date 2012-04-10 12:49GMT
 * \author Jan Boon (Kaetemi)
 * UGroupController
 */
class UGroupController
{
	virtual void setDevGain(float gain) = 0;
	virtual float getDevGain() = 0;

	virtual void setUserGain(float gain) = 0;
	virtual float getUserGain() = 0;

protected:
	virtual ~UGroupController();

}; /* class UGroupController */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_U_GROUP_CONTROLLER_H */

/* end of file */
