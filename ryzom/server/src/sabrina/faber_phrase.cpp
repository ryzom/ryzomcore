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
#include "faber_phrase.h"
#include "game_share/brick_families.h"
#include "s_phrase_factory.h"
#include "entity_manager.h"
#include "phrase_manager.h"
#include "phrase_utilities_functions.h"
#include "game_share/entity_structure/statistic.h"


DEFAULT_SPHRASE_FACTORY( CFaberPhrase, BRICK_TYPE::FABER );

using namespace std;
using namespace NLMISC;

//-----------------------------------------------
// ctor
//-----------------------------------------------
CFaberPhrase::CFaberPhrase()
{
	_FaberAction = 0;
	_SabrinaCost = 0;
	_SabrinaCredit = 0;
	_StaminaCost = 0;
	_HPCost = 0;
	_FaberTime = 0;
	_RootFaberPlan = 0;
	_Tool = 0;
	_MBOQuality = 0;;
	_MBODurability = 0;
	_MBOMBOWeight = 0.0f;
	_MBODmg = 0;
	_MBOSpeed = 0.0f;
	_MBORange = 0;
	_MBOProtection = 0;
	_MBOSapLoad = 0;
	_IsStatic = true;
}


//-----------------------------------------------
// CFaberPhrase build
//-----------------------------------------------
bool CFaberPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks )
{
	// we are sure there is at least one brick and that there are non NULL;
	nlassert( !bricks.empty() );

	_ActorRowId = actorRowId;

	// compute cost and credit and parse other params
	for ( uint i = 0; i < bricks.size(); ++i )
	{
		const CStaticBrick & brick = *bricks[i];

		if ( brick.SabrinaValue < 0 )
			_SabrinaCredit -= brick.SabrinaValue;
		else
			_SabrinaCost += brick.SabrinaValue;
		
		switch( brick.Family )
		{
			case BRICK_FAMILIES::RootFaber:
				_RootFaberPlan = ( const CStaticItem* ) &brick;
				break;
			case BRICK_FAMILIES::FARawMaterial:
				_Mps.push_back( ( const CStaticItem* ) &brick );
				break;
			default:
			for ( uint i=0 ; i<brick.Params.size() ; ++i)
			{
				switch(brick.Params[i]->id())
				{	
					case TBrickParam::SAP:
						return false;
					case TBrickParam::HP:
						INFOLOG("HP: %i",((CSBrickParamHp *)brick.Params[i])->Hp);
						_HPCost += ((CSBrickParamHp *)brick.Params[i])->Hp;
						break;
					case TBrickParam::STA:
						INFOLOG("STA: %i",((CSBrickParamSta *)brick.Params[i])->Sta);
						_StaminaCost += ((CSBrickParamSta *)brick.Params[i])->Sta;
						break;
					default:
						// unused param ?
						break;
				}
			}
			break;
		}
	}

	// build the craft action
	_FaberAction = IFaberActionFactory::buildAction( actorRowId, bricks, this );
	if ( !_FaberAction )
	{
		nlwarning( "<CFaberPhrase build> could not build action for root brick %s", bricks[0]->SheetId.toString().c_str() );
		return false;
	}
	
	return true;
}// CFaberPhrase build


//-----------------------------------------------
// CFaberPhrase evaluate
//-----------------------------------------------
bool CFaberPhrase::evaluate(CEvalReturnInfos *msg)
{
	// update state
	_State = CSPhrase::Evaluated;
	return true;
}// CFaberPhrase evaluate


//-----------------------------------------------
// CFaberPhrase validate
//-----------------------------------------------
bool CFaberPhrase::validate()
{
	// entities cant craft if in combat
	TDataSetRow entityRowId = CPhraseManager::getInstance()->getEntityEngagedMeleeBy( _ActorRowId );
	if (entityRowId.isValid())
	{
		///\todo alain : send message
		return false;
	}
	CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	const sint32 hp = entity->getScores()._PhysicalScores[ SCORES::hit_points ].Current;
	if ( hp < _HPCost  )
	{
		///\todo alain : send message
		return false;
	}
	const sint32 sta = entity->getScores()._PhysicalScores[ SCORES::stamina ].Current;
	if ( sta < _StaminaCost  )
	{
		///\todo alain : send message
		return false;
	}
	if (hp <= 0	||	entity->getMode()==MBEHAV::DEATH)
	{
		///\todo alain : send message
		return false;
	}

	/// todo alain : test if on mount

	// update state
	if (_State == Evaluated)
		_State = Validated;
	else if (_State == ExecutionInProgress)
		_State = SecondValidated;
	return true;
}// CFaberPhrase validate


//-----------------------------------------------
// CFaberPhrase update
//-----------------------------------------------
bool  CFaberPhrase::update()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	
	// if the sentence execution delay time has ended, apply sentence effects
	if ( _State == SecondValidated && _ExecutionEndDate <= time && _NbWaitingRequests == 0)
	{
		apply();
	}
	else if ( _State == Latent /*&& _LatencyEndDate <= time*/ )
	{
		INFOLOG("Latency ended");
		_State = CSPhrase::LatencyEnded;
	}
	
	return true;
}// CFaberPhrase update


//-----------------------------------------------
// CFaberPhrase execute
//-----------------------------------------------
void  CFaberPhrase::execute()
{
	if( _NbWaitingRequests != 0)
		return;
	
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_State = CSPhrase::ExecutionInProgress;
	_ExecutionEndDate  = time + _FaberTime ;
}// CFaberPhrase execute


//-----------------------------------------------
// CFaberPhrase apply
//-----------------------------------------------
void CFaberPhrase::apply()
{
	_State = CSPhrase::Latent;

	///\todo behaviour

	// spend energies
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( _ActorRowId );
	if (entity == NULL)
	{
		nlwarning("<CFaberPhrase::apply> Invalid entity Id %s", TheDataset.getEntityId(_ActorRowId).toString().c_str() );		
		return;
	}
	RY_GAME_SHARE::SCharacteristicsAndScores &sta = entity->getScores()._PhysicalScores[SCORES::stamina];
	if ( sta.Current != 0)
	{
		sta.Current = sta.Current - _StaminaCost;
		if (sta.Current < 0)
			sta.Current = 0;
	}

	RY_GAME_SHARE::SCharacteristicsAndScores &hp = entity->getScores()._PhysicalScores[SCORES::hit_points];
	if ( hp.Current != 0)
	{
		hp.Current = hp.Current - _HPCost;
		if (hp.Current < 0)
			hp.Current = 0;
	}

	// apply action of the sentence
	_FaberAction->apply(this);
}//CFaberPhrase apply


//-----------------------------------------------
// CFaberPhrase systemCraftItem:
//-----------------------------------------------
CGameItemPtr CFaberPhrase::systemCraftItem( const NLMISC::CSheetId& sheet, const std::vector< NLMISC::CSheetId >& Mp )
{
	std::vector< const CStaticBrick* > bricks;
	_RootFaberPlan = CSheets::getForm( sheet );

	CGameItemPtr craftedItem = 0;
	
	if( _RootFaberPlan && _RootFaberPlan->Faber )
	{
		if( ((const CStaticBrick * )_RootFaberPlan)->Family == BRICK_FAMILIES::RootFaber )
		{
			bricks.push_back( (const CStaticBrick*)_RootFaberPlan );
			
			for( vector< NLMISC::CSheetId >::const_iterator it = Mp.begin(); it != Mp.end(); ++it )
			{
				const CStaticItem * mp = CSheets::getForm( (*it) );
				if( mp == 0 )
				{
					nlwarning("<CFaberPhrase::systemCraftItem> Can't found form for Mp %s for craft %s item", (*it).toString().c_str(), sheet.toString().c_str() );
					return 0;
				}
				bricks.push_back( (const CStaticBrick*) mp );
				_Mps.push_back( mp );
			}

			// Check quantity of gived Mps
			if( _RootFaberPlan->Faber->NeededMps.size() > _Mps.size() )
			{
				nlwarning("<CFaberPhrase::systemCraftItem> Not enought gived RM for crafting %s (gived Mp %d, Needed Mp %d)", sheet.toString().c_str(), _Mps.size(), _RootFaberPlan->Faber->NeededMps.size() );
				return 0;
			}

			// build the craft action
			TDataSetRow row; //build an invalide dataset index
			_FaberAction = IFaberActionFactory::buildAction( row, bricks, this );
			if ( !_FaberAction )
			{
				nlwarning( "<CFaberPhrase build> could not build action for root faber %s", _RootFaberPlan->SheetId.toString().c_str() );
				return 0;
			}
			_FaberAction->systemApply(this);
			craftedItem = _CraftedItem;
			_CraftedItem = 0;
		}	
	}
	return craftedItem;
}
