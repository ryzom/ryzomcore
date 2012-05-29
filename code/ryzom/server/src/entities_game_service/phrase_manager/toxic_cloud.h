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



#ifndef NL_TOXIC_CLOUD_H
#define NL_TOXIC_CLOUD_H

#include "nel/misc/vector_2f.h"
#include "nel/misc/variable.h"
#include "game_share/base_types.h"
#include "environmental_effect.h"
#include "entity_manager/entity_base.h"

/**
 * Toxic cloud
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CToxicCloud : public CEnvironmentalEffect
{
	NL_INSTANCE_COUNTER_DECL(CToxicCloud);
public:

	CToxicCloud() {}

	/// Init
	inline void		init( const NLMISC::CVector& pos, float radius, sint32 dmgPerHit, NLMISC::TGameCycle updateFrequency, NLMISC::TGameCycle lifetime=ToxicCloudDefaultLifetime) 
	{ 
		CEnvironmentalEffect::init(pos, radius,lifetime);
		_DmgPerHit = dmgPerHit;
		_UpdateFrequency = updateFrequency;
	}

	/**
	 * Spawn the toxic cloud as an entity in mirror. Return false in case of failure.
	 * \param sheet sheet id of the effect (for visual FX)
	 */
	virtual bool		spawn( const NLMISC::CSheetId &sheet );

	/// Tick update. Return false if the source's life is ended.
	virtual bool		update();

	static NLMISC::TGameCycle	ToxicCloudDefaultLifetime;

private:
	/// test target validity, returns true if target is valid
	inline bool isTargetValid(CEntityBase *entity)
	{	
#ifdef NL_DEBUG
		nlassert(entity);
#endif
		uint8 entityType = entity->getId().getType();
		switch(entityType) 
		{
		case RYZOMID::player:
			return true;

		case RYZOMID::creature:
			if (entity->getRace() == EGSPD::CPeople::MektoubMount ||  entity->getRace() == EGSPD::CPeople::MektoubPacker)
				return true;
			return false;
		
		default:
			return false;
		}
	}

	/// Dmg per hit
	sint32				_DmgPerHit;

	/// update frequency
	NLMISC::TGameCycle	_UpdateFrequency;
};


#endif // NL_TOXIC_CLOUD_H

/* End of toxic_cloud.h */
