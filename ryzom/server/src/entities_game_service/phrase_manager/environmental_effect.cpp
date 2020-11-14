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



#include "stdpch.h"
#include "environmental_effect.h"
#include "nel/misc/variable.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;

NL_INSTANCE_COUNTER_IMPL(CEnvironmentalEffect);
NL_INSTANCE_COUNTER_IMPL(CEnvironmentalEffectManager);

NLMISC::TGameCycle								CEnvironmentalEffect::DefaultLifetime = 200; // 20 s
NL_ISO_TEMPLATE_SPEC CSimpleEntityManager<CEnvironmentalEffect>	*	CSimpleEntityManager<CEnvironmentalEffect>::_Instance = NULL;


/*
 * Despawn the effect in mirror
 */
void CEnvironmentalEffect::despawn()
{
	// Remove from mirror
	CEntityId entityId = TheDataset.getEntityId( _DataSetRow );
	Mirror.removeEntity( entityId );
}


/*
 * Tick update. Return false if the effect's life is ended.
 */
bool CEnvironmentalEffect::update()
{
	// Test effect end
	if (_TimeToLive == 0)
	{
		despawn();
		return false;
	}
	else
	{
		--_TimeToLive;
		return true;
	}
}


NLMISC_DYNVARIABLE( uint, NbEnvironmentalEffects, "Number of environmental effects" )
{
	if ( get )
		*pointer = CEnvironmentalEffectManager::getInstance()->nbEntities();
}

/// Singleton access
CEnvironmentalEffectManager *CEnvironmentalEffectManager::getInstance()
{
	return (CEnvironmentalEffectManager*)_Instance;
}

/// Release
void CEnvironmentalEffectManager::release()
{
	delete (CEnvironmentalEffectManager*)_Instance;
}
