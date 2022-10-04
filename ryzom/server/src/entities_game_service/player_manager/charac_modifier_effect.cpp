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
// net
#include "nel/net/message.h"
//
#include "charac_modifier_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"
#include "entity_manager/entity_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//		CCharacteristicModifierEffect::isTimeToUpdate()
//--------------------------------------------------------------
bool CCharacteristicModifierEffect::isTimeToUpdate()
{
	return true;
} // isTimeToUpdate //

//--------------------------------------------------------------
//		CCharacteristicModifierEffect::update()
//--------------------------------------------------------------
bool CCharacteristicModifierEffect::update( uint32 & updateFlag )
{
	if (_AffectedCharac >= CHARACTERISTICS::NUM_CHARACTERISTICS)
		return true;

	if (!TheDataset.isAccessible(_TargetRowId))
		return true;

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
		return true;

	entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Modifier = _Modifier + entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Modifier;
	if( entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Modifier >= entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Base )
	{
		entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Modifier = entity->getCharacteristics()._PhysicalCharacteristics[_AffectedCharac].Base - 1;
	}

	return (_EndDate <= CTickEventHandler::getGameCycle());
} // update //

//--------------------------------------------------------------
//		CCharacteristicModifierEffect::removed()
//--------------------------------------------------------------
void CCharacteristicModifierEffect::removed()
{
	// send messages to clients
	PHRASE_UTILITIES::sendEffectStandardEndMessages(_CreatorRowId, _TargetRowId, _EffectName);
} // removed //

