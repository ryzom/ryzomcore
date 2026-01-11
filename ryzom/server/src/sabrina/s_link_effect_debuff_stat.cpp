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
#include "s_link_effect_debuff_stat.h"
#include "s_link_effect_dot.h"
#include "entity_manager.h"
#include "game_share/entity_structure/statistic.h"



using namespace std;
using namespace NLMISC;
using namespace RY_GAME_SHARE;

extern CRandom RandomGenerator;


bool CSLinkEffectDebuffStat::update(uint32 & updateFlag)
{
	if ( CSLinkEffectOffensive::update(updateFlag) )
		return true;
	
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDebuffStat update> Invalid target %u",_CreatorRowId.getIndex() );
		return true;
	}
	
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectDebuffStat update> Invalid target %u",_TargetRowId.getIndex() );
		return true;
	}

	if ( (_Family & updateFlag) == 0)
	{
		switch( _Type )
		{
		case STAT_TYPES::Skill:
			///\todo nico
			break;
		case STAT_TYPES::Score:
			///\todo nico
			break;
		case STAT_TYPES::Speed:
			target->getPhysScores().SpeedVariationModifier += _Value;
			break;
		default:
			nlwarning("<CSLinkEffectDebuffStat update> invalid stat type %d", _Type);
			return true;
		}
	}
	updateFlag |= _Family;
	return false;
}




