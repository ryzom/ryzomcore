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

#ifndef NL_PS_DIRECTION_H
#define NL_PS_DIRECTION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

namespace NL3D {


/** This class is an interface for object for a particle system that need a direction (forces, emitter...)
 *  Some of these object support global value as a direction (see CParticleSystem::setGlobalVectorValue), so
 *  calls to setDir then have no effect.
 */

class CPSDirection
{
public :
	virtual ~CPSDirection() {}
	virtual void setDir(const NLMISC::CVector &v) = 0;
	virtual NLMISC::CVector getDir(void) const = 0;
	virtual bool supportGlobalVectorValue() const { return false; }
	/** The direction is taken from a global vector defined in the particle system
	  * NULL or an empty string as a name disable the use of a global value
	  */
	virtual void				enableGlobalVectorValue(const std::string &/* name */) {}
	virtual std::string         getGlobalVectorValueName() const { return ""; }
};


} // NL3D


#endif // NL_PS_DIRECTION_H

/* End of ps_direction.h */
