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
#include "nel/misc/string_conversion.h"
#include "nel/misc/command.h"
#include "entity_manager/entity_manager.h"
#include "area_effect.h"
#include "range_selector.h"
#include "entity_manager/entity_base.h"

#include "egs_sheets/egs_static_ai_action.h"
#include "egs_sheets/egs_static_game_item.h"

#include "game_item_manager/game_item.h"
#include "nel/misc/entity_id.h"

using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CAreaEffect);
NL_INSTANCE_COUNTER_IMPL(CEntityRangeSelector);

CAreaEffect::CAreaEffect()
: Type(MAGICFX::UnknownSpellMode)
{
}
CAreaEffect::CAreaEffect(const CAreaEffect &areaEffect)
{
	Type = areaEffect.Type;

	switch(Type)
	{
	case MAGICFX::Bomb:
		Bomb = areaEffect.Bomb;
		break;
	case MAGICFX::Chain:
		Chain = areaEffect.Chain;
		break;
	case MAGICFX::Spray:
		Spray = areaEffect.Spray;
		break;
	default:
		nlassert(false);
	}
}

CAreaEffect::~CAreaEffect()
{
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
CAreaEffect * CAreaEffect::buildArea( CGameItemPtr &itemPtr )
{
#ifdef NL_DEBUG
	nlassert(itemPtr != NULL);
#endif
	CAreaEffect * area = NULL;
	const CStaticItem * item = itemPtr->getStaticForm();
	if ( item && item->RangeWeapon )
	{
		switch( item->RangeWeapon->AreaType)
		{
		case RANGE_WEAPON_TYPE::Missile:
			{
				CSBrickParamAreaBomb bomb;
				bomb.MinFactor = item->RangeWeapon->Missile.MinFactor;
				bomb.Radius = item->RangeWeapon->Missile.Radius;
				return buildBomb(&bomb);
			}
			break;
		case RANGE_WEAPON_TYPE::Gatlin:
			{
				CSBrickParamAreaSpray spray;
				spray.Angle = item->RangeWeapon->Gatling.Angle;
				spray.Base = item->RangeWeapon->Gatling.Base;
				spray.Height = item->RangeWeapon->Gatling.Height;
				return buildSpray(&spray);
			}
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
CAreaEffect * CAreaEffect::buildArea( const CStaticAiAction *aiAction )
{
#ifdef NL_DEBUG
	nlassert(aiAction != NULL);
#endif
	const TAiArea &areaParams = aiAction->getAreaData();
	switch( areaParams.AreaType)
	{
	case MAGICFX::Bomb:
		{
			CSBrickParamAreaBomb bomb;
			bomb.MinFactor = 1.0f;
			bomb.Radius = areaParams.AreaRange;
			bomb.MaxTarget = areaParams.BombMaxTargets;
			return buildBomb(&bomb);
		}
		break;
	case MAGICFX::Spray:
		{
			CSBrickParamAreaSpray spray;
			spray.Angle = areaParams.SprayAngle;
			spray.Base = areaParams.SprayBase;
			spray.Height = areaParams.SprayHeight;
			spray.MaxTarget = areaParams.SprayMaxTargets;
			return buildSpray(&spray);
		}
	case MAGICFX::Chain:
		{
			CSBrickParamAreaChain chain;
			chain.Factor = areaParams.ChainFadingFactor;
			chain.MaxTargets = areaParams.ChainMaxTargets;
			chain.Range = areaParams.AreaRange;
			return buildChain(&chain);
		}
		
	default:
		return NULL;
	}
	
	return NULL;
}

// ----------------------------------------------------------------------------
void CEntityRangeSelector::buildTargetList(const TDataSetRow & actorRow,const TDataSetRow & mainTargetRow, const CAreaEffect * area, ACTNATURE::TActionNature nature, bool ignoreMainTarget)
{
	_Area = area;
	CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( actorRow );
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( mainTargetRow );
	if ( !target || !actor )
		return;
	switch( area->Type )
	{
	case MAGICFX::Bomb:
		buildDisc( actor, target->getX(),target->getY(),area->Bomb.Radius, EntityMatrix, nature == ACTNATURE::FIGHT || nature == ACTNATURE::OFFENSIVE_MAGIC, ignoreMainTarget );
		break;
	case MAGICFX::Spray:
		buildCone(  actor, target,area->Spray.Height,area->Spray.MinBase, area->Spray.MaxBase, EntityMatrix, nature == ACTNATURE::FIGHT || nature == ACTNATURE::OFFENSIVE_MAGIC, ignoreMainTarget);
		break;
	case MAGICFX::Chain:
		buildChain(  actor, target,area->Chain.Range,area->Chain.MaxTargets, EntityMatrix, nature, ignoreMainTarget);
		break;
	default:
		return;
	}
	// main target must be at index 0 for compatibility with other systems
	bool found = false;
	for ( uint i = 0; i < _Entities.size() && !found; i++ )
	{
		if ( _Entities[i]->getId() == target->getId() )
		{
			_Entities[i] = _Entities[0];
			_Entities[0] = target;
			_Distances[i] = _Distances[0];
			_Distances[0] = (float) sqrt( pow( (actor->getX() - target->getX())/1000.0,2 ) + pow( (actor->getY() - target->getY() )/ 1000.0f,2 ) );
			found = true;
		}
	}

//	if ( !found ) //we not warn anymore for this pb, we never found why target disappear, perhaps it's because AIS despawn it too quickly after it's death
//	{
//		nlwarning("<CEntityRangeSelector> (actor %s on entity %s) (type %d) main target not found among %u targets where is my target ???",actor->getId().toString().c_str(), target->getId().toString().c_str(), area->Type,_Entities.size());
//	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
float CEntityRangeSelector::getFactor(uint entityIdx) const
{
	if ( !_Area || _Area->Type == MAGICFX::Spray )
	{
		return 1.0f;
	}
	if ( _Area->Type == MAGICFX::Chain )
	{
		_PrevFactor = _PrevFactor * _Area->Chain.Factor;
		return _PrevFactor;
	}
	if ( _Area->Type == MAGICFX::Bomb )
	{
		if ( entityIdx >= _Distances.size() )
		{
			nlwarning("<AREA> invalid distance index %u. count is %u. There are %u entities",entityIdx,_Distances.size(),_Entities.size());
			return 0.0f;
		}
		if ( entityIdx == 0 )
			return 1.0f;
		float dist = _Distances[entityIdx];
#if !FINAL_VERSION
		nlassert(dist <= _Area->Bomb.Radius);
#endif
		return float( 1.0 - (dist / _Area->Bomb.Radius) * ( 1.0 - _Area->Bomb.MinFactor ) );
	}
	return 0.0f;
}


//-----------------------------------------------
// displayVision
//-----------------------------------------------
NLMISC_COMMAND(displayBombRange,"display the affected entities ( all entities in a disc of the specified radius)","<caster><center entity><radius>")
{
	if ( args.size() == 3)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * source = CEntityBaseManager::getEntityBasePtr( id );
		if ( !source )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}
		id.fromString(args[1].c_str());
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( id );
		if ( !target )
		{
			log.displayNL( "invalid entity %s", args[1].c_str() );
			return true;
		}
		uint16 radius;
		NLMISC::fromString(args[2], radius);
		
		CRangeSelector selector;
		selector.buildDisc( source,target->getX(),target->getY(), radius,EntityMatrix, false );
		for ( uint i  = 0; i < selector.getEntities().size(); i++ )
		{
			log.displayNL("%s",selector.getEntities()[i]->getId().toString().c_str());	
		}
		return true;
	}
	return false;
} // displayBombRange

//-----------------------------------------------
// displayConeRange
//-----------------------------------------------
NLMISC_COMMAND(displayConeRange,"display the affected entities ( all entities in a cone of the specified values)","<source eId><target eId><height><min width><maxWidth>")
{
	if ( args.size() == 5)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * source = CEntityBaseManager::getEntityBasePtr( id );
		if ( !source )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}
		id.fromString(args[1].c_str());
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( id );
		if ( !target )
		{
			log.displayNL( "invalid entity %s", args[1].c_str() );
			return true;
		}
		float height = float(  atof(args[2].c_str() ) * 1000.0f );
		float minWidth = (float)atof(args[3].c_str());
		float maxWidth = (float)atof(args[4].c_str());

		float dist = (float) ( sqrt(pow((float)(source->getState().X - target->getState().X),2) + pow((float)(source->getState().Y - target->getState().Y),2)) / 1000.0);

		CRangeSelector selector;
		selector.buildCone(source , target, height, minWidth,  maxWidth, EntityMatrix, false);
		for ( uint i  = 0; i < selector.getEntities().size(); i++ )
		{
			log.displayNL("%s",selector.getEntities()[i]->getId().toString().c_str());	
		}
		return true;
	}
	return false;
} // displayBombRange

//-----------------------------------------------
// displayChainRange
//-----------------------------------------------
NLMISC_COMMAND(displayChainRange,"display the affected entities ( all entities in a cone of the specified values)","<source eId><target eId><max interval><max target>")
{
	if ( args.size() == 4)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * source = CEntityBaseManager::getEntityBasePtr( id );
		if ( !source )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}
		id.fromString(args[1].c_str());
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( id );
		if ( !target )
		{
			log.displayNL( "invalid entity %s", args[1].c_str() );
			return true;
		}

		uint16 range;
		NLMISC::fromString(args[2], range);

		uint max;
		NLMISC::fromString(args[3], max);
		
		CRangeSelector selector;
		selector.buildChain(source , target, range, max, EntityMatrix,ACTNATURE::NEUTRAL);
		for ( uint i  = 0; i < selector.getEntities().size(); i++ )
		{
			log.displayNL("%s at %u meters",selector.getEntities()[i]->getId().toString().c_str(), (uint)sqrt( selector.getDistances()[i] ));	
		}
		return true;
	}
	return false;
} // displayChainRange

NLMISC_COMMAND(entityDistance,"display the distance between two entities and infos about their position in the entity matrix","<eId1><eId2>")
{
	if ( args.size() == 2)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * source = CEntityBaseManager::getEntityBasePtr( id );
		if ( !source )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}
		id.fromString(args[1].c_str());
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( id );
		if ( !target )
		{
			log.displayNL( "invalid entity %s", args[1].c_str() );
			return true;
		}
		double dist = sqrt( pow(double (source->getState().X - target->getState().X)/1000.0,2) + pow(double (source->getState().Y - target->getState().Y)/1000.0,2) );
				
		
		log.displayNL("distance :%f",dist);
		log.displayNL("%s : cell = '%u,%u'",source->getId().toString().c_str(),WorldXtoMatrixX(source->getState().X),WorldYtoMatrixY(source->getState().Y));
		log.displayNL("%s : cell = '%u,%u'",target->getId().toString().c_str(),WorldXtoMatrixX(target->getState().X),WorldYtoMatrixY(target->getState().Y));


		
		return true;
	}
	return false;
} // displayBombRange

























