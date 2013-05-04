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




#ifndef NL_DUP_PS_H
#define NL_DUP_PS_H


namespace NL3D
{
	class CParticleSystemProcess;
	class CPSLocatedBindable;
}


/** For now there is no duplication method in particle system.
  * These are needed for edition, though..
  * These functions provide a way to do this (by serializing the system in a memory stream).
  * NB this is slow
  * These may be removed, but this provide a easy way to accomplish our goal
  */

/** temp : duplicate a process of a particle system.
  * return NULL if the copy failed
  */
NL3D::CParticleSystemProcess	*DupPSLocated(const NL3D::CParticleSystemProcess *in);
	
/** temp : duplicate a located bindable of a particle system
  * return NULL if the copy failed
  */
NL3D::CPSLocatedBindable	*DupPSLocatedBindable(NL3D::CPSLocatedBindable *in);





#endif