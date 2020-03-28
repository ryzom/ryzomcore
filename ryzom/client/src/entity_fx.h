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



#ifndef CL_ENTITY_FX_H
#define CL_ENTITY_FX_H

#include <string>
#include <map>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/3d/u_particle_system_instance.h"

/**
 * CEntityFX class contained datas for instanciate an FX
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CEntityFx
{
public:

	struct SStatusFx
	{
		uint32 AlwaysStarted : 1;
		uint32 InfinityLoop : 1;

		uint32 Started : 1;

		SStatusFx( void )
		{
			AlwaysStarted = 0;
			InfinityLoop = 0;
			Started = 0;
		}
	};

	/**
	 * Constructor
	 * \param FxName name of Fx, must be unique
	 * \param InstanceName name of instance (shape), used for instanciate FX
	 * \param Position initial position of Fx
	 * \param Rotation initial rotation matrix of Fx
	 */
	 CEntityFx( const std::string& FxName, const std::string& InstanceName, const NLMISC::CVector& Position, const NLMISC::CQuat& Rotation );

	/**
	 * Destructor
	 */
	 ~CEntityFx();

	/// Get Status of Fx
	inline SStatusFx& getStatus( void ) { return _StatusFx; }

	/// Set Status of Fx
	inline void setStatus( const SStatusFx& Status ) { _StatusFx = Status; }

	/// alwaysStarted return true if Fx must be always started or set it
	inline bool alwaysStarted( void ) { return _StatusFx.AlwaysStarted; }
	inline void alwaysStarted( bool b ) { _StatusFx.AlwaysStarted = b; }

	/// infinityLoop return true if Fx is must be in InfinityLoop or set it
	inline bool infinityLoop( void ) { return _StatusFx.InfinityLoop; }
	inline void infinityLoop( bool b ) { _StatusFx.InfinityLoop = b; }

	/// started return true if Fx is Started or set it
	inline bool started( void ) { return _StatusFx.Started; }
	inline void started( bool b ) { _StatusFx.Started = b; }

	/// startFx
	void startFx( void );

	/// isTerminated return true if fx is finish
	bool isTerminated( void );

	/// deleteInstance
	void deleteInstance( void);

private:
	std::string _FxName;
	std::string _InstanceName;
	NLMISC::CVector _Position;
	NLMISC::CQuat _Rotation;
	SStatusFx _StatusFx;

	NL3D::UParticleSystemInstance _FxInstance;
};


typedef std::map< std::string, CEntityFx *> TMapEntityFx;
extern TMapEntityFx EntityFx;

void newFx( const std::string& FxName, const std::string& InstanceName, const NLMISC::CVector& Position, const NLMISC::CQuat& Rotation );
void deleteFx( const std::string& FxName );
void startFx( const std::string& FxName );
void manageFxEntities( void );

#endif // CL_ENTITY_FX_H

/* End of entity_fx.h */
