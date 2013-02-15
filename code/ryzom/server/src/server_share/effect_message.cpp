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
#include "effect_message.h"
#include "effect_manager.h"



//--------------------------------------------------------------
//				CAddEffectsMessage::callback()  
//--------------------------------------------------------------
void CAddEffectsMessage::callback(const std::string &name, NLNET::TServiceId sid)
{
	std::vector< TDataSetRow >::const_iterator	itEntities;
	std::vector< uint32 >::const_iterator		itEffectId = EffectIds.begin();
	std::vector< uint16 >::const_iterator		itFamilies = Families.begin();
	std::vector< TDataSetRow >::const_iterator	itCreators =  Creators.begin();

	if ( Entities.size() != EffectIds.size() || EffectIds.size() != Families.size() || Creators.size() != EffectIds.size() )
	{
		nlwarning("<CAddEffectsMessage::callback> ERROR : different size for the vectors (%u entities), (%u effects), (%u families)",Entities.size(), EffectIds.size(), Families.size());
		return;
	}

	const std::vector< TDataSetRow >::const_iterator itEntitiesEnd = Entities.end();

	for (itEntities = Entities.begin() ; itEntities != itEntitiesEnd ; ++itEntities )
	{
		CBasicEffect effect( (EFFECT_FAMILIES::TEffectFamily) (*itFamilies), *itCreators ,*itEntities ,*itEffectId);
		CEffectManager::addEffect( *itEntities, effect );

		++itEffectId;
		++itFamilies;
		++itCreators;
	}

} // CAddEffectsMessage::callback //



//--------------------------------------------------------------
//				CRemoveEffectsMessage::callback()  
//--------------------------------------------------------------
void CRemoveEffectsMessage::callback(const std::string &name, NLNET::TServiceId sid)
{
	std::vector< TDataSetRow >::const_iterator	itEntities;
	std::vector< uint32 >::const_iterator		itEffectId = EffectIds.begin();

	if ( Entities.size() != EffectIds.size() )
	{
		nlwarning("<CRemoveEffectsMessage::callback> ERROR : different size for the vectors (%u entities), (%u effects id)",Entities.size(), EffectIds.size() );
		return;
	}

	const std::vector< TDataSetRow >::const_iterator itEntitiesEnd = Entities.end();

	for (itEntities = Entities.begin() ; itEntities != itEntitiesEnd ; ++itEntities )
	{
		CEffectManager::removeEffect( *itEntities, *itEffectId );

		++itEffectId;
	}
} // CRemoveEffectsMessage::callback //


