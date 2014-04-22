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

#include "creature_manager/creature_manager.h"
#include "creature_manager/creature.h"
#include "shop_type/shop_type_manager.h"

#include "nel/misc/hierarchical_timer.h"

#include "entity_manager/entity_callbacks.h"
#include "mission_manager/ai_alias_translator.h"
#include "team_manager/team_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "mission_manager/mission_manager.h"
#include "building_manager/building_manager.h"
#include "outpost_manager/outpost_manager.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"

#include "egs_variables.h"
#include "primitives_parser.h"

#include "egs_dynamic_sheet_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;



void genNpcDescCb (CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid)
{
	CGenNpcDescMsgImp	msg;
	msg.serial(msgin);
	
	msg.callback(serviceName, sid);
}

TUnifiedCallbackItem	GenNpcDescCbTable[] =
{
	{	"TGenNpcDescMsg", genNpcDescCb},
};


//--------------------------------------------------------------
//				CNpcBotDescription ::callback()  
//--------------------------------------------------------------
//void CNpcBotDescriptionImp::callback(const string &serviceName, uint8 sid)
void CGenNpcDescMsgImp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	H_AUTO(CGenNpcDescMsgImp_callback );
	
	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<CNpcBotDescriptionImp::callback> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}
	CEntityId	Eid= getEntityIdFromRow(_EntityIndex);
	if ( Eid.isUnknownId() )
	{
		nlwarning( "Received CNpcBotDescription with E%u that has no entity id, skipping it", _EntityIndex.getIndex() );
		nldebug( "Reason: %s", TheDataset.explainIsAccessible(_EntityIndex).c_str() );
#ifdef NL_DEBUG
//		nlstop;
#endif
		return;
	}
	CCreature *creature = CreatureManager.getCreature( Eid );
	if( creature == 0 )
	{
		creature = new CCreature();
		if ( creature )
		{
			creature->setAIGroupAlias( _GrpAlias );
			creature->setUserModelId(_UserModelId);
			creature->setCustomLootTableId(_CustomLootTableId);
			creature->setPrimAlias(_PrimAlias);
			if ( _GrpAlias != CAIAliasTranslator::Invalid )
			{
				CreatureManager.addNpcToGroup( _GrpAlias, _Alias ); // called every time the callback is received
			}
			creature->setId(Eid);
			creature->mirrorizeEntityState( false,	_EntityIndex ); // import the position
			creature->addPropertiesToMirror( _EntityIndex, false ); // init properties + import the sheetid from the mirror
			creature->setServerSheet();

			if( creature->getType() == CSheetId::Unknown )
			{
				nlwarning("<CNpcBotDescriptionImp::callback> Npc Eid %s GrpAlias %s Alias %s have invalide sheet, not spawned in EGS", 
					Eid.toString().c_str(), 
					CPrimitivesParser::aliasToString(_GrpAlias).c_str(), 
					CPrimitivesParser::aliasToString(_Alias).c_str() );
				delete creature;
				return;
			}
			else
			{
				creature->loadSheetCreature( _EntityIndex );
				
				//if the creature has a user model and if the user model's script contains parse errors, change
				//the creature's name to <usermodelId:ERROR> 
				if (!_UserModelId.empty() && CDynamicSheetManager::getInstance()->scriptErrors(_PrimAlias, _UserModelId) == true)
				{
					TDataSetRow row = creature->getEntityRowId();
					ucstring name;
					name.fromUtf8("<"+ _UserModelId + ":ERROR>");
					NLNET::CMessage	msgout("CHARACTER_NAME");
					msgout.serial(row);
					msgout.serial(name);
					sendMessageViaMirror("IOS", msgout);
				}
				CreatureManager.addCreature( Eid, creature );

				// set the items
				creature->setItems( _RightHandItem, _RightHandItemQuality, _LeftHandItem, _LeftHandItemQuality );
			}
		}
	}
//	else
//		CreatureManager.addUnaffectedDescription( *this );

	if (creature != NULL)
	{
		if (!creature->getCustomLootTableId().empty())
		{
			creature->getContextualProperty().directAccessForStructMembers().lootable(true);
		}
		creature->setBotDescription( *this );
		CAIAliasTranslator::getInstance()->updateAssociation(_Alias,Eid);

		if (_BuildingBot)
			COutpostManager::getInstance().onBuildingSpawned(creature);
	}
}

//--------------------------------------------------------------
//				CFaunaBotDescription ::callback()  
//--------------------------------------------------------------
void CFaunaBotDescriptionImp::callback(const string &, NLNET::TServiceId sid)
{
	H_AUTO(CFaunaBotDescriptionImpCallback);

	if ( Bots.size() != GrpAlias.size() )
	{
		nlwarning("<CFaunaBotDescription callback> the two vectors do not have the same size!");
	}
	// for each bot, set its new group alias
	for ( uint i = 0; i < Bots.size(); i++ )
	{
		CCreature * c = CreatureManager.getCreature( Bots[i] );
		if ( c )
		{
			c->setAIGroupAlias( GrpAlias[i] );
		}
		else
		{
			CreatureManager.addUnaffectedFaunaGroup(  Bots[i], GrpAlias[i] ) ;
		}
	}
}

//--------------------------------------------------------------
//				CCreatureCompleteHealImp ::callback()  
//--------------------------------------------------------------
void CCreatureCompleteHealImp::callback(const string &, NLNET::TServiceId sid)
{
	H_AUTO(CCreatureCompleteHealImp);
	
	// for each creature, restore full HP
	for ( uint i = 0; i < Entities.size(); ++i )
	{
		CCreature * c = CreatureManager.getCreature( Entities[i] );
		if ( c )
		{
			c->changeCurrentHp( c->maxHp() - c->currentHp() );
		}
	}
}

//--------------------------------------------------------------
//				CChangeCreatureMaxHPImp ::callback()  
//--------------------------------------------------------------
void CChangeCreatureMaxHPImp::callback(const string &, NLNET::TServiceId sid)
{
	H_AUTO(CChangeCreatureMaxHPImp);
	
	// for each creature, restore full HP
	for ( uint i = 0; i < Entities.size(); ++i )
	{
		CCreature * c = CreatureManager.getCreature( Entities[i] );
		if ( c )
		{
			c->getScores()._PhysicalScores[SCORES::hit_points].Max = MaxHp[i];
			if (SetFull[i] != 0)
				c->changeCurrentHp( c->maxHp() - c->currentHp() );
		}
	}
}



//--------------------------------------------------------------
//				CChangeCreatureHPImp ::callback()  
//--------------------------------------------------------------
void CChangeCreatureHPImp::callback(const string &, NLNET::TServiceId sid)
{
	H_AUTO(CChangeCreatureHPImp);

	uint16 size = (uint16)Entities.size();
	if (Entities.size() != DeltaHp.size() )
	{
		nlwarning("Entities.size() != DeltaHp.size()");

		size = (uint16)min(Entities.size(),DeltaHp.size());
	}
	
	// for each creature, change HP
	for ( uint i = 0; i < size; ++i )
	{
		CCreature * c = CreatureManager.getCreature( Entities[i] );
		if ( c )
		{
			if( c->currentHp()+DeltaHp[i] > c->maxHp() )
			{
				// clamp hp
				c->changeCurrentHp( c->maxHp() - c->currentHp() );
			}
			else
			{
				c->changeCurrentHp( DeltaHp[i] );
			}
		}
	}
}


//--------------------------------------------------------------
//				CCreatureSetUrlImp ::callback()  
//--------------------------------------------------------------
void CCreatureSetUrlImp::callback(const string &, NLNET::TServiceId sid)
{
	H_AUTO(CCreatureSetUrlImp);

	// for each creature set url
	for ( uint i = 0; i < Entities.size(); ++i )
	{
		CCreature * c = CreatureManager.getCreature( Entities[i] );
		if ( c )
		{
			uint32 program = c->getBotChatProgram();
			if(!(program & (1<<BOTCHATTYPE::WebPageFlag)))
			{
				if(program != 0)
				{
					return;
				}
				program |= 1 << BOTCHATTYPE::WebPageFlag;
				c->setBotChatProgram(program);
			}
			
			const string &wp = c->getWebPage();
			if(Url == "*") {
				(string &)wp = "";
				program &= ~(1 << BOTCHATTYPE::WebPageFlag);
				c->setBotChatProgram(program);
				return;
			}
			else
				(string &)wp = Url;

			const string &wpn = c->getWebPageName();
			(string &)wpn = ActionName;

			return;
		}
	}
}


//---------------------------------------------------
// Constructor
//
//---------------------------------------------------
CCreatureManager::CCreatureManager()
{
	_SlideUpdate = 0;
	_StartCreatureRegen = 0;
}


//---------------------------------------------------
// Constructor
//
//---------------------------------------------------
CCreatureManager::~CCreatureManager()
{
//	CShopTypeManager::release();
}


//---------------------------------------------------
// addCreatureCallback: Add callback for creature management
//
//---------------------------------------------------
void CCreatureManager::addCreatureCallback()
{
/*	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
*/	
}


//---------------------------------------------------
// addCreature :
//
//---------------------------------------------------
void CCreatureManager::addCreature( const CEntityId& Id, CCreature * creature )
{
	H_AUTO(CCreatureManagerAddCreature);

	// if we have loaded a totem bot object, we must update pointers
	NLMISC::CSString sheetIdName = creature->getType().toString();
	
	if ( creature->isSpire() )
	{
		CPVPFactionRewardManager::getInstance().addBotObject( creature );
	}

	TMapCreatures::iterator itCreature = _Creatures.find( Id );
	if( itCreature == _Creatures.end() )
	{
		_Creatures.insert( make_pair( Id, creature ) );

		TDataSetRow	row=TheDataset.getDataSetRow(Id);

		for (list< CGenNpcDescMsgImp >::iterator it = _UnaffectedDescription.begin(); it!= _UnaffectedDescription.end(); ++it )
		{
			if ( it->getEntityIndex()==row )
			{
				string name;
				it->callback(name, NLNET::TServiceId(0));
				_UnaffectedDescription.erase(it);
				break;
			}
		}
		for ( uint i = 0; i < _UnaffectedFaunaGroups.size(); i++ )
		{
			if ( _UnaffectedFaunaGroups[i].EntityIndex == row )
			{
				creature->setAIGroupAlias( _UnaffectedFaunaGroups[i].GroupAlias );
				_UnaffectedFaunaGroups[i] = _UnaffectedFaunaGroups.back();
				_UnaffectedFaunaGroups.pop_back();
				break;
			}
		}
	}
	else
	{
		nlwarning("(EGS)<CCreatureManager::addCreature> : The creature %s has already been added", Id.toString().c_str() );
	}

} // addCreature //

//---------------------------------------------------
// getCreature :
//---------------------------------------------------
CCreature * CCreatureManager::getCreature( const CEntityId& Id )
{
	H_AUTO(getCreature);
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		return (*it).second;
	}
	else
	{
		return 0;
	}

} // getCreature //

//---------------------------------------------------
// removeCreature
//
//---------------------------------------------------
void CCreatureManager::removeCreature( const CEntityId& Id )
{
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		delete (*it).second;
		_Creatures.erase( it );
//		nlinfo("(EGS)<CCreatureManager::removeCreature> creature %s removed", Id.toString().c_str());
	}
	else
	{
		nlwarning("(EGS)<CCreatureManager::removeCreature> : Creature %s doesn't exist", Id.toString().c_str());
	}
} // removeCreature //

//---------------------------------------------------
// agsDisconnect :
//---------------------------------------------------
void CCreatureManager::agsDisconnect( NLNET::TServiceId serviceId )
{
	TMapCreatures::iterator it = _Creatures.begin();
	while( it != _Creatures.end() )
	{
		if( (*it).first.getDynamicId() == serviceId.get() )
		{
			TMapCreatures::iterator itTmp = it;
			++it,
			delete (*itTmp).second;
			_Creatures.erase( itTmp );
		}
		else
		{
			++it;
		}
	}
} // agsDisconnect //


//---------------------------------------------------
// GPMS connexion
//---------------------------------------------------
void CCreatureManager::gpmsConnexion()
{
	// nothing to do for now
}

//---------------------------------------------------
// getType :
//
//---------------------------------------------------
CSheetId CCreatureManager::getType( const CEntityId& Id )
{
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		return (*it).second->getType();
	}
	else
	{
		throw ECreature(Id);
	}
} // getType //

//---------------------------------------------------
// setValue :
//
//---------------------------------------------------
void CCreatureManager::setValue( const CEntityId& Id, const string& var, const string& value )
{
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		(*it).second->setValue( var, value );
	}
	else
	{
		nlwarning("(EGS)<CCreatureManager::setValue> : Creature %s doesn't exist", Id.toString().c_str());
	}
} // setValue //

//---------------------------------------------------
// modifyValue :
//
//---------------------------------------------------
void CCreatureManager::modifyValue( const CEntityId& Id, const string& var, const string& value )
{
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		(*it).second->modifyValue( var, value );
	}
	else
	{
		nlwarning("(EGS)<CCreatureManager::modifyValue> : Creature %s doesn't exist", Id.toString().c_str());
	}
} // setValue //

//---------------------------------------------------
// getValue :
//
//---------------------------------------------------
string CCreatureManager::getValue( const CEntityId& Id, const string& var )
{
	TMapCreatures::iterator it = _Creatures.find( Id );
	if( it != _Creatures.end() )
	{
		string value;
		(*it).second->getValue( var, value );
		return value;
	}
	else
	{
		throw ECreature( Id );
	}
} // getValue //

//---------------------------------------------------
// tickUpdate :
//---------------------------------------------------
void CCreatureManager::tickUpdate()
{
#if 0
	H_AUTO(CreatureManagerUpdate);

	TMapCreatures::iterator it, itend = _Creatures.end();

//	uint32 endCreatureRegen = _StartCreatureRegen + ( _Creatures.size() + NbTickForRegenCreature - 1 ) / NbTickForRegenCreature;

	static uint8 timeMask = 0;
	timeMask++;

//	uint32 creatureCounter = 0;
	for ( it = _Creatures.begin() ; it != itend ; ++it )
	{
		uint8 localTimeMask = uint8(((int)(*it).second)>>8) + timeMask;
		if ((localTimeMask&3) && (*it).second->getSEffects().empty())
			continue;

		// toremove
		H_AUTO(tickup_4);

		const TDataSetRow &rowid = (*it).second->getEntityRowId();
		if( TheDataset.isDataSetRowStillValid( rowid ) )
		{
			/*if( creatureCounter >= _StartCreatureRegen && creatureCounter < endCreatureRegen )
			{
				(*it).second->tickUpdate( true, localTimeMask );
			}
			else
			{*/
				(*it).second->tickUpdate( false, localTimeMask );
				//}
		}
		//++creatureCounter;
	}

/*	_StartCreatureRegen = endCreatureRegen;
	if( endCreatureRegen >= _Creatures.size() )
	{
		_StartCreatureRegen = 0;
	}
*/
#endif
} // tickUpdate //

//---------------------------------------------------
// dumpUnaffectedFaunaDesc
//---------------------------------------------------
void CCreatureManager::dumpUnaffectedFaunaGroups(NLMISC::CLog & log)
{
	for (uint i = 0; i < _UnaffectedFaunaGroups.size(); i++ )
	{
		log.displayNL("row %d, group %d",_UnaffectedFaunaGroups[i].EntityIndex.getIndex(),_UnaffectedFaunaGroups[i].GroupAlias);
	}
} // dumpUnaffectedFaunaDesc

//---------------------------------------------------
// removeNpcFromGroup
//---------------------------------------------------
void CCreatureManager::removeNpcFromGroup( TAIAlias groupAlias, TAIAlias npcAlias )
{
	CHashMap< unsigned int,CNPCGroup >::iterator it = _NpcGroups.find( groupAlias );
	if ( it == _NpcGroups.end() )
	{
		nlwarning("<CCreatureManager removeNpcFromGroup> Invalid NPC group %s", CPrimitivesParser::aliasToString(groupAlias).c_str());
	}
	else
	{
		CNPCGroup & group = (*it).second;

		group.Members.erase( npcAlias );
		const bool groupWiped = (group.Members.empty());

		CMissionManager::getInstance()->checkEscortFailure(groupAlias, groupWiped);

		if (groupWiped)
		{
			// group is wiped, time to trigger mission event
			vector<uint16> processedTeams;
			for ( uint i = 0; i < group.TeamKillers.size(); i++ )
			{
				CTeam * team = TeamManager.getRealTeam( group.TeamKillers[i] );
				if ( team )
				{
					CCharacter * user = PlayerManager.getChar( team->getLeader() );
					if ( user )
					{
						CMissionEventKillGroup event(groupAlias,CMissionEvent::NoSolo);
						if ( user->processMissionEvent( event ) )
						{
							// the event has been processed for this team. We add the team to the processed team
							processedTeams.push_back( group.TeamKillers[i] );
						}
					}
				}
			}
			for ( uint i = 0; i < group.PlayerKillers.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( group.PlayerKillers[i] );
				if ( user )
				{
					if ( std::find( processedTeams.begin(),processedTeams.end(),user->getTeamId() ) == processedTeams.end() )
					{
						CMissionEventKillGroup event( groupAlias,CMissionEvent::NoGroup );
						user->processMissionEvent( event );
					}
				}
			}
			_NpcGroups.erase(it);
		}
		else
			CMissionManager::getInstance()->checkEscortFailure(groupAlias,false);
	}
} // removeNpcFromGroup


//----------------------------------------------------------------------------
void CCreatureManager::fixAltar()
{
	for( TMapCreatures::iterator it = _Creatures.begin(); it != _Creatures.end(); ++it )
	{
		CCreature * c = (*it).second;
		if( c )
		{
			if( c->getAltarForNeutral() )
			{
				c->clearAltarFameRestriction();
				c->clearAltarFameRestrictionValue();
			}
		}
	}
}

//--------------------------------------------------------------
//				CNpcBotDescription ::callback()  
//--------------------------------------------------------------
void CAIGainAggroMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CCreature * creature = dynamic_cast< CCreature *>( CEntityBaseManager::getEntityBasePtr( TargetRowId ) );
	if( creature )
	{
//		creature->getCreatureOpponent().storeAggressor(PlayerRowId,0);
		creature->addAggressivenessAgainstPlayerCharacter( PlayerRowId );
	}
}
