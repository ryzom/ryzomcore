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

#ifndef NL_PS_SPAWN_INFO_H
#define NL_PS_SPAWN_INFO_H

#include "nel/misc/vector.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/particle_system_process.h"

namespace NL3D
{

	class CPSLocated;

	// info about the state of an emitting particle
	class CPSEmitterInfo
	{
	public:
		CPSLocated		*Loc;
		NLMISC::CVector Pos;
		NLMISC::CVector Speed;
		float			InvMass;
		float			Life;
	public:
		void setDefaults()
		{
			Loc = NULL;
			Pos = NLMISC::CVector::Null;
			Speed = NLMISC::CVector::Null;
			InvMass = 1.f;
			Life = 0.f;
		}
	};


	// info about a particle that should be spawned
	struct CPSSpawnInfo
	{
		CPSEmitterInfo	 EmitterInfo;
		NLMISC::CVector  SpawnPos;
		NLMISC::CVector  Speed;
		TPSMatrixMode	 SpeedCoordSystem;
		TAnimationTime	 LifeTime; // age of the particle when it is spawned (may not be 0 because of sub-frame accuracy)
	};

}
#endif
