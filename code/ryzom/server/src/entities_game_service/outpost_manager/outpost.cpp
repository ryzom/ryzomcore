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

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------

#include "stdpch.h"
#include "outpost.h"
#include "outpost_manager.h"
#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "player_manager/character.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_char_proxy.h"
#include "guild_manager/guild_member.h"
#include "guild_manager/guild_member_module.h"
#include "egs_globals.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_outpost.h"
#include "egs_variables.h"
#include "server_share/used_continent.h"
#include "primitives_parser.h"
#include "world_instances.h"
#include "entity_manager/entity_manager.h"
#include "outpost_version_adapter.h"
#include "player_manager/player.h"
#include "player_manager/player_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "server_share/log_outpost_gen.h"

/**

:NOTE: The behaviour of the outpost if its AIS is up but didn't load outpost primitives is weird.
:TODO: Fix that.
:TODO: Implement outpost attack cancellation (and include it in crash handling).
:TODO: Implement challenge queue.

*/

//----------------------------------------------------------------------------
// namespaces
//----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//----------------------------------------------------------------------------
// config vars
//----------------------------------------------------------------------------

static uint32 const seconds = 1;
static uint32 const minutes = 60*seconds;
static uint32 const hours = 60*minutes;
static uint32 const days = 24*hours;

CVariable<uint32> OutpostFightRoundCount("egs","OutpostFightRoundCount","number of rounds in an outpost fight", 24, 0, true);
CVariable<uint32> OutpostFightRoundTime("egs","OutpostFightRoundTime","time of a round in an outpost fight, in seconds", 5*minutes, 0, true);
CVariable<uint32> OutpostLevelDecrementTime("egs","OutpostLevelDecrementTime","time to decrement an outpost level in seconds (in peace time)", 2*days, 0, true);
CVariable<uint32> OutpostEditingConcurrencyCheckDelay("egs", "OutpostEditingConcurrencyCheckDelay", "delay in ticks used to check if 2 actions for editing an outpost are concurrent", 50, 0, true );
CVariable<uint32> OutpostClientTimersUpdatePeriod("egs", "OutpostClientTimersUpdatePeriod", "period in seconds between 2 updates of outpost timers on clients", 60, 0, true );
CVariable<bool> UseProxyMoneyForOutpostCosts("egs", "UseProxyMoneyForOutpostCosts", "If true outpost costs can be paid by player issuing command if guild has not enough money", true, 0, true );
CVariable<uint32> OutpostStateTimeOverride("egs", "OutpostStateTimeOverride", "Each state can be set to a shorter time in seconds, 0 means default computed value", true, 0, true );
CVariable<uint32> OutpostJoinPvpTimer("egs", "OutpostJoinPvpTimer", "Max time the player has to answer the JoinPvp Window, in seconds", 10, 0, true );
CVariable<uint32> NumberDayFactorGuildNeedForChallengeOutpost("egs","NumberDayFactorGuildNeedForChallengeOutpost","Nombre de 'level outpost / factor' jours d'existance que la guilde doit avoir pour pouvoir challenger un outpost",10,0,true);
CVariable<sint32> NumberDaysMinusOutpostLevelForChallenge("egs","NumberDaysMinusOutpostLevelForChallenge", "Number to substract from outpost level to get oldness required to challenge an outpost",50,0,true);

extern CPlayerManager PlayerManager;


//----------------------------------------------------------------------------
// methods COutpost
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
COutpost::COutpost()
: _State(OUTPOSTENUMS::UnknownOutpostState)
, _OwnerGuildId(0)
, _AttackerGuildId(0)
, _NextAttackSquadsA()
, _NextAttackSquadsB()
, _NextDefenseSquadsA()
, _NextDefenseSquadsB()
, _Timer0EndTime(0)
, _Timer1EndTime(0)
, _Timer2EndTime(0)
, _AISUp(false)
, _CurrentOutpostLevel(0)
, _CurrentSquadsAQueue()
, _CurrentSquadsBQueue()
, _CrashHappened(false)
, _RealChallengeTime(0)
, _ChallengeTime(0)
, _ChallengeHour(0)
, _AttackHour(0)
, _DefenseHour(0)
, _NextState(OUTPOSTENUMS::UnknownOutpostState)
, _NeedOutpostDBUpdate(true)
, _LastUpdateOfTimersForClient(0)
, _StateEndDateTickForClient(0)
, _RoundEndDateTickForClient(0)
{
//	CBankAccessor_OUTPOST::init( CDBOutpost );
	_CurrentSquadsA.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_CurrentSquadsB.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_TribeSquadsA.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_TribeSquadsB.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
}

//----------------------------------------------------------------------------
void COutpost::addOutpostDBRecipient(NLMISC::CEntityId const& player)
{
	nlassert(player!=NLMISC::CEntityId::Unknown);
	_DbGroup.addRecipient( player );
	sendOutpostDBDeltas();
}

//----------------------------------------------------------------------------
void COutpost::removeOutpostDBRecipient(NLMISC::CEntityId const& player)
{
	nlassert(player!=NLMISC::CEntityId::Unknown);
	_DbGroup.removeRecipient( player );
}

//----------------------------------------------------------------------------
void COutpost::sendOutpostDBDeltas()
{
	_DbGroup.sendDeltas( ~0, CCDBGroup::SendDeltasToRecipients );
}

//----------------------------------------------------------------------------
//void COutpost::setClientDBProp(std::string const& prop, sint64 value)
//{
//	CBankAccessor_OUTPOST::Database.setProp( prop, value );
//}
//
//void COutpost::setClientDBPropString(std::string const& prop, const ucstring &value )
//{
//	CBankAccessor_OUTPOST::Database.setPropString( prop, value );
//}
//
////----------------------------------------------------------------------------
//sint64 COutpost::getClientDBProp(std::string const& prop)
//{
//	return CBankAccessor_OUTPOST::Database.getProp( prop );
//}

//----------------------------------------------------------------------------
void COutpost::fillOutpostDB()
{
	H_AUTO(COutpost_fillOutpostDB);
	CGuild* owner = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
	CGuild* attacker = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
	uint32 const SHEET					= _Sheet.asInt();
	uint8 const LEVEL					= getStaticForm()?getStaticForm()->Level:0;
	ucstring const GUILD_NAME			= owner?owner->getName():ucstring();
	uint64 const GUILD_ICON				= owner?owner->getIcon():0;
//	uint32 const TRIBE					= isBelongingToAGuild()?0:1;
	ucstring const GUILD_NAME_ATT			= attacker?attacker->getName():ucstring();
	uint8 const STATUS					= computeStatusForClient();
	uint32 const STATE_END_DATE			= computeStateEndDateTickForClient();
//	uint32 const DISPLAY_CRASH			= _CrashHappened?1:0;
	uint32 const WARCOST				= _Form->ChallengeCost;
	uint8 const ROUND_LVL_THRESHOLD		= uint8(_CurrentOutpostLevel);
	uint8 const ROUND_LVL_MAX_ATT		= uint8(_FightData._MaxAttackLevel);
	uint8 const ROUND_LVL_MAX_DEF		= uint8(_FightData._MaxDefenseLevel);
	uint8 const ROUND_LVL_CUR			= uint8(_FightData._CurrentCombatLevel);
	uint8 const ROUND_ID_CUR			= uint8(_FightData._CurrentCombatRound);
	uint8 const ROUND_ID_MAX			= uint8(computeRoundCount());
	uint8 const TIME_RANGE_DEF_WANTED	= uint8(_DefenseHour);
	uint32 const TIME_RANGE_DEF			= computeTimeRangeDefForClient();
	uint32 const TIME_RANGE_ATT			= computeTimeRangeAttForClient();
	uint16 const TIME_RANGE_LENGTH		= uint16(computeTimeRangeLengthForClient());

	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setSHEET(_DbGroup, _Sheet);			//	setClientDBProp("OUTPOST_SELECTED:SHEET",					SHEET);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setLEVEL(_DbGroup, LEVEL);			//	 setClientDBProp("OUTPOST_SELECTED:LEVEL",					LEVEL);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().getGUILD().setNAME(_DbGroup, GUILD_NAME);	//	setClientDBPropString("OUTPOST_SELECTED:GUILD:NAME",		GUILD_NAME);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().getGUILD().setICON(_DbGroup, GUILD_ICON);	//	setClientDBProp("OUTPOST_SELECTED:GUILD:ICON",				GUILD_ICON);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().getGUILD().setTRIBE(_DbGroup, !isBelongingToAGuild());	//	setClientDBProp("OUTPOST_SELECTED:GUILD:TRIBE",				TRIBE);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().getGUILD().setNAME_ATT(_DbGroup, GUILD_NAME_ATT);		//	setClientDBPropString("OUTPOST_SELECTED:GUILD:NAME_ATT",	GUILD_NAME_ATT);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setSTATUS(_DbGroup, STATUS);		///setClientDBProp("OUTPOST_SELECTED:STATUS",					STATUS);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setSTATE_END_DATE(_DbGroup, STATE_END_DATE);		//setClientDBProp("OUTPOST_SELECTED:STATE_END_DATE",			STATE_END_DATE);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setDISPLAY_CRASH(_DbGroup, _CrashHappened);	//	setClientDBProp("OUTPOST_SELECTED:DISPLAY_CRASH",			DISPLAY_CRASH);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setWARCOST(_DbGroup, WARCOST);				//		setClientDBProp("OUTPOST_SELECTED:WARCOST",					WARCOST);
	
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_LVL_THRESHOLD(_DbGroup, ROUND_LVL_THRESHOLD);	//		setClientDBProp("OUTPOST_SELECTED:ROUND_LVL_THRESHOLD",		ROUND_LVL_THRESHOLD);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_LVL_MAX_ATT(_DbGroup, ROUND_LVL_MAX_ATT);		//		setClientDBProp("OUTPOST_SELECTED:ROUND_LVL_MAX_ATT",		ROUND_LVL_MAX_ATT);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_LVL_MAX_DEF(_DbGroup, ROUND_LVL_MAX_DEF);		//		setClientDBProp("OUTPOST_SELECTED:ROUND_LVL_MAX_DEF",		ROUND_LVL_MAX_DEF);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_LVL_CUR(_DbGroup, ROUND_LVL_CUR);				//		setClientDBProp("OUTPOST_SELECTED:ROUND_LVL_CUR",			ROUND_LVL_CUR);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_ID_CUR(_DbGroup, ROUND_ID_CUR);					//		setClientDBProp("OUTPOST_SELECTED:ROUND_ID_CUR",			ROUND_ID_CUR);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setROUND_ID_MAX(_DbGroup, ROUND_ID_MAX);					//		setClientDBProp("OUTPOST_SELECTED:ROUND_ID_MAX",			ROUND_ID_MAX);
	
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setTIME_RANGE_DEF_WANTED(_DbGroup, TIME_RANGE_DEF_WANTED);	//		setClientDBProp("OUTPOST_SELECTED:TIME_RANGE_DEF_WANTED",	TIME_RANGE_DEF_WANTED);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setTIME_RANGE_DEF(_DbGroup, TIME_RANGE_DEF);				//		setClientDBProp("OUTPOST_SELECTED:TIME_RANGE_DEF",			TIME_RANGE_DEF);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setTIME_RANGE_ATT(_DbGroup, TIME_RANGE_ATT);				//		setClientDBProp("OUTPOST_SELECTED:TIME_RANGE_ATT",			TIME_RANGE_ATT);
	CBankAccessor_OUTPOST::getOUTPOST_SELECTED().setTIME_RANGE_LENGTH(_DbGroup, TIME_RANGE_LENGTH);		//		setClientDBProp("OUTPOST_SELECTED:TIME_RANGE_LENGTH",		TIME_RANGE_LENGTH);
}

//----------------------------------------------------------------------------
void COutpost::fillCharacterOutpostDB( CCharacter * user )
{
	H_AUTO(COutpost_fillCharacterOutpostDB);

	if( user )
	{
		if( _State==OUTPOSTENUMS::AttackRound || _State==OUTPOSTENUMS::DefenseRound )
		{
//			user->_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_LVL_CUR", _FightData._CurrentCombatLevel );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_LVL_CUR(user->_PropertyDatabase, _FightData._CurrentCombatLevel);
//			user->_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_END_DATE", computeRoundEndDateTickForClient() );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_END_DATE(user->_PropertyDatabase, computeRoundEndDateTickForClient());
		}
		else
		{
//			user->_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_LVL_CUR", 0 );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_LVL_CUR(user->_PropertyDatabase, 0);
//			user->_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_END_DATE", 0 );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_END_DATE(user->_PropertyDatabase, 0);
		}
	}
}

//----------------------------------------------------------------------------
bool COutpost::build(const NLLIGO::IPrimitive* prim,const std::string &filename,const std::string &dynSystem, CONTINENT::TContinent continent)
{
	_Alias = 0;
	
	_State = OUTPOSTENUMS::Peace;
	_OwnerGuildId = 0;
	_AttackerGuildId = 0;
	_CrashHappened = false;
	_Timer0EndTime = 0;
	_Timer1EndTime = 0;
	_Timer2EndTime = 0;
	_AISUp = false;
	_OwnerExpenseLimit = 0;
	_AttackerExpenseLimit = 0;
	_MoneySpentByOwner = 0;
	_MoneySpentByAttacker = 0;

	bool ret = true;
	string value;

	// parse identifier
	nlverify( prim->getPropertyByName("name",_Name) );
	
	nlverify( prim->getPropertyByName("disable_outpost", value) );
	if (value == "true")
	{
		OUTPOST_DBG("Found disabled outpost '%s', in dyn system '%s' of file '%s'", _Name.c_str(), dynSystem.c_str(), filename.c_str() );
		return false;
	}

	OUTPOST_DBG("Parsing outpost '%s', in dyn system '%s' of file '%s'", _Name.c_str(), dynSystem.c_str(), filename.c_str() );

	nlassert( continent < CONTINENT::NB_CONTINENTS ); // must be checked in calling code!!!
	_Continent = continent;
	const string continentStr = CONTINENT::toString( _Continent );

	nlverify( CPrimitivesParser::getAlias(prim, _Alias) );

	// death penalty
	string deathPenaltyFactor;
	nlverify( prim->getPropertyByName("death_penalty_factor", deathPenaltyFactor) );
	_DeathPenaltyFactor = (float)atof( deathPenaltyFactor.c_str() );
	
	// get the sheet and check it
	nlverify( prim->getPropertyByName("outpost_sheet", value ) );
	_Sheet = CSheetId( value+".outpost" );
	if ( _Sheet == CSheetId::Unknown )
	{
		OUTPOST_WRN("PRIM_ERROR : invalid sheet '%s' (the sheet is not in sheet_id.bin)", value.c_str());
		ret = false;
	}
	
	_Form = CSheets::getOutpostForm(_Sheet);
	if ( _Form == NULL )
	{
		OUTPOST_WRN("PRIM_ERROR : invalid sheet '%s' (the sheet exists but contained errors)", value.c_str());
		ret = false;
	}
	COutpost *otherOutpost = COutpostManager::getInstance().getOutpostFromSheet(_Sheet);
	if ( otherOutpost != NULL )
	{
		OUTPOST_WRN("PRIM_ERROR : the outposts %s and %s have the same sheet %s", getName().c_str(), otherOutpost->getName().c_str(), _Sheet.toString().c_str() );
	}
	
	// get the PVP type
	nlverify( prim->getPropertyByName("PVP_Type", value ) );
	_PVPType = OUTPOSTENUMS::toPVPType( value );
	if ( _PVPType == OUTPOSTENUMS::UnknownPVPType )
	{
		OUTPOST_WRN("PRIM_ERROR : invalid PVP type '%s' (the sheet exists but contained errors)", value.c_str());
		ret = false;
	}
	
	// build the outpost PVP zone
	const NLLIGO::CPrimZone * zone = dynamic_cast<const NLLIGO::CPrimZone*> ( prim );
	nlassert( zone );
	IPVPZone::buildOutpostZone( zone, this );
	CPVPManager::getInstance()->addPVPZone( this );

	setActive( false );

	vector<string> *params;

	// get default buildings
	for (uint i = 0; i < prim->getNumChildren(); ++i)
	{
		const NLLIGO::IPrimitive* outpostChildNode = NULL;
		std::string className;
		if ( prim->getChild( outpostChildNode,i ) && outpostChildNode != NULL && outpostChildNode->getPropertyByName("class", className) )
		{
			if ( className == "outpost_building" )
			{
				const CPrimPoint * buildingNode = dynamic_cast<const CPrimPoint *>(outpostChildNode);
				nlassert(buildingNode != NULL);

				std::string name;
				uint32 alias = 0;
				nlverify( buildingNode->getPropertyByName("name", name) );
				nlverify( buildingNode->getPropertyByName("sheet", value) );
				nlverify( CPrimitivesParser::getAlias(buildingNode, alias) );

				const string sheetSuffix = ".outpost_building";
				if (value.find(sheetSuffix) == string::npos)
					value += sheetSuffix;

				CSheetId buildingSheet(value);
				if (buildingSheet != CSheetId::Unknown)
				{
					if (_Buildings.size() < OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS)
					{
						_Buildings.push_back(COutpostBuilding(this, alias, buildingNode->Point, buildingSheet));
						OUTPOST_DBG( "Added building %s in outpost %s", name.c_str(), _Name.c_str() );
					}
					else
						OUTPOST_WRN( "PRIM_ERROR : an outpost cannot have more than %d buildings", OUTPOSTENUMS::OUTPOST_MAX_BUILDINGS );
				}
				else
					OUTPOST_WRN( "PRIM_ERROR : unknown outpost building sheet '%s'", value.c_str() );
			}
		}
	}

	// get default squads
	nlverify ( prim->getPropertyByName("default_squads",params) && params );
	for (size_t i = 0; i < params->size(); ++i)
	{
		string& squadName = (*params)[i];
		if ( !squadName.empty() )
		{
			COutpostSquadDescriptor squadDesc;
			if ( COutpostManager::getInstance().fillSquadDescriptor( squadName, false, continentStr, squadDesc ) )
			{
				nlassert(squadDesc.sheet()!=CSheetId::Unknown);
				nlassert(squadDesc.getStaticForm() != NULL);

				if (squadDesc.getStaticForm()->BuyPrice == 0)
				{
					_DefaultSquads.push_back( squadDesc );
				}
				else
					OUTPOST_WRN( "PRIM_ERROR : default squad '%s' is not free : price=%u", squadName.c_str(), squadDesc.getStaticForm()->BuyPrice );
			}
			else
				OUTPOST_WRN( "PRIM_ERROR : default squad '%s' not found in templates for continent %s", squadName.c_str(), continentStr.c_str() );
		}
	}
	if ( _DefaultSquads.empty() )
	{
		OUTPOST_WRN( "PRIM_ERROR : found no valid default squads in %s", _Name.c_str() );
		ret = false;
	}
	
	// get buyable squads
	nlverify ( prim->getPropertyByName("buyable_squads",params) && params );
	for (size_t i = 0; i < params->size(); ++i)
	{
		string& squadName = (*params)[i];
		if ( !squadName.empty() )
		{
			COutpostSquadDescriptor squadDesc;
			if ( COutpostManager::getInstance().fillSquadDescriptor( squadName, false, continentStr, squadDesc ) )
			{
				if (squadDesc.getStaticForm()->BuyPrice == 0)
					OUTPOST_WRN( "PRIM_ERROR : buyable squad '%s' is free", squadName.c_str() );
				_BuyableSquads.push_back( squadDesc );
			}
			else
				OUTPOST_WRN( "PRIM_ERROR : buyable squad %s not found in templates for continent %s", squadName.c_str(), continentStr.c_str() );
		}
	}

	if (_DefaultSquads.size() + _BuyableSquads.size() > OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP)
	{
		OUTPOST_WRN( "PRIM_ERROR : free and buyable squads are too many for client database : %u > %u",
			_DefaultSquads.size() + _BuyableSquads.size(),
			OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SHOP
			);
	}

	// tribe
	nlverifyex ( prim->getPropertyByName("owner_tribe",value) && (!value.empty()), ("Missing owner tribe in outpost '%s' in %s", _Name.c_str(), filename.c_str()) );
	string tribeName = value;
	
	// get tribe squads
	nlverify ( prim->getPropertyByName("tribe_squads",params) && params );
	if (!params->empty())
	{
		for (uint i = 0; i < _TribeSquadsA.size(); ++i)
		{
			string& squadName = (*params)[i % params->size()];
			if ( !squadName.empty() )
			{
				COutpostSquadDescriptor squadDesc;
				if ( COutpostManager::getInstance().fillSquadDescriptor( squadName, true, continentStr, squadDesc ) )
				{
					nlassert(squadDesc.sheet()!=CSheetId::Unknown);
					nlassert(i < _TribeSquadsA.size());
					_TribeSquadsA[i] = squadDesc;
				}
				else
				{
					OUTPOST_WRN( "PRIM_ERROR : tribe squad %s not found in templates for continent %s", squadName.c_str(), continentStr.c_str() );
					ret = false;
				}
			}
		}
	}
	else
	{
		OUTPOST_WRN( "PRIM_ERROR : tribe_squads must contain at least 1 squad!", _TribeSquadsA.size());
		ret = false;
	}

	// get tribe squads2
	vector<string> *tribe_squads_params = params; // keep a pointer on tribe_squads
	nlverify ( prim->getPropertyByName("tribe_squads2",params) && params );
	// if tribe_squads2 is empty use tribe_squads
	if (params->empty())
	{
		params = tribe_squads_params;
		OUTPOST_DBG( "tribe_squads2 is empty, tribe_squads will be used instead");
	}
	if (!params->empty())
	{
		for (uint i = 0; i < _TribeSquadsB.size(); ++i)
		{
			string& squadName = (*params)[i % params->size()];
			if ( !squadName.empty() )
			{
				COutpostSquadDescriptor squadDesc;
				if ( COutpostManager::getInstance().fillSquadDescriptor( squadName, true, continentStr, squadDesc ) )
				{
					nlassert(squadDesc.sheet()!=CSheetId::Unknown);
					nlassert(i < _TribeSquadsB.size());
					_TribeSquadsB[i] = squadDesc;
				}
				else
				{
					OUTPOST_WRN( "PRIM_ERROR : tribe squad '%s' not found in templates for tribe '%s'", squadName.c_str(), tribeName.c_str() );
					ret = false;
				}
			}
		}
	}

	
	OUTPOST_DBG( "Outpost %s has %u default squads (free), %u buyable squads, %u(A)/%u(B) tribe squads, %u default buildings",
		_Name.c_str(),
		_DefaultSquads.size(),
		_BuyableSquads.size(),
		_TribeSquadsA.size(),
		_TribeSquadsB.size(),
		_Buildings.size()
		);
	
	// spawn zones
	for ( uint i = 0; i < prim->getNumChildren(); ++i )	
	{
		const NLLIGO::IPrimitive* outpostChildNode = NULL;
		std::string className;
		if ( prim->getChild( outpostChildNode,i ) && outpostChildNode != NULL && outpostChildNode->getPropertyByName("class", className) )
		{
			if ( className == "outpost_spawn_zone" )
			{
				const CPrimPoint * spawnZoneNode = dynamic_cast<const CPrimPoint *>(outpostChildNode);
				nlassert(spawnZoneNode != NULL);

				std::string name;
				std::string radius;
				uint32 alias = 0;
				if ( spawnZoneNode->getPropertyByName("name",name) && spawnZoneNode->getPropertyByName("radius", value) && CPrimitivesParser::getAlias(spawnZoneNode, alias) )
				{
					if (_SpawnZones.size() < OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE)
					{
						_SpawnZones.push_back( COutpostSpawnZone(alias, spawnZoneNode->Point, (float)atof(radius.c_str())) );
						OUTPOST_DBG( "Added spawn zone %s in outpost %s", CPrimitivesParser::aliasToString(alias).c_str(), _Name.c_str() );
					}
					else
						OUTPOST_WRN( "PRIM_ERROR : an outpost cannot have more than %d spawn zones", OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE );
				}
			}
		}
	}

	if (_SpawnZones.empty())
	{
		OUTPOST_WRN( "PRIM_ERROR : there must be at least 1 spawn zone");
		ret = false;
	}

	if (_SpawnZones.size() > OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE)
	{
		OUTPOST_WRN( "PRIM_ERROR : spawn zones are too many for client database : %u > %u",
			_SpawnZones.size(),
			OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE
			);
	}

	OUTPOST_DBG( "Outpost %s has %u spawn zones", _Name.c_str(), _SpawnZones.size() );

	if (!ret)
	{
		OUTPOST_WRN( "PRIM_ERROR : outpost loading failed (see errors above) : %s, %s, %s", _Name.c_str(), dynSystem.c_str(), filename.c_str() );
	}

	return ret;
}

//----------------------------------------------------------------------------
bool COutpost::onBuildingSpawned(CCreature *pBuilding)
{
	for (uint i = 0; i < _Buildings.size(); ++i)
		if (pBuilding->getAlias() == _Buildings[i].getAlias())
		{
			_Buildings[i].onSpawned(pBuilding);
			return true;
		}
	return false;
}

//----------------------------------------------------------------------------
bool COutpost::canConstructBuilding(const NLMISC::CSheetId &sid, const COutpostBuilding *slot) const
{
	// By now just check that we cannot create 2 buildings of the same type (except empty)
	const CStaticOutpostBuilding *pSOB = CSheets::getOutpostBuildingForm(sid);
	if (pSOB == NULL) return false;
	if (pSOB->Type == CStaticOutpostBuilding::TypeEmpty) return true;

	// Check there is not an already constructed building of this type
	for (uint i = 0; i < _Buildings.size(); ++i)
	if (&_Buildings[i] != slot) // No check for the already present building (for upgrades)
	{
		if (_Buildings[i].getStaticData() != NULL)
			if (_Buildings[i].getStaticData()->Type == pSOB->Type)
				return false;
	}

	return true;
}

//----------------------------------------------------------------------------
void COutpost::setConstructionTime(uint32 nNbSeconds, uint32 nCurrentTime)
{
	for (uint i = 0; i < _Buildings.size(); ++i)
		_Buildings[i].setConstructionTime(nNbSeconds, nCurrentTime);
}

//----------------------------------------------------------------------------
void COutpost::initNewOutpost()
{
	_NextAttackSquadsA.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_NextAttackSquadsB.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_NextDefenseSquadsA.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	_NextDefenseSquadsB.resize(OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	
	// tribes own new outposts
	_OwnerGuildId = 0;

	// init the outpost level
	_CurrentOutpostLevel = computeTribeOutpostLevel();

	// Create default squads
	resetDefaultAttackSquads();
	resetDefaultDefenseSquads();

	OUTPOST_DBG( "Initialized the new outpost '%s'", _Name.c_str() );
}

//----------------------------------------------------------------------------
void COutpost::registerOutpostInGuilds()
{
	if (isBelongingToAGuild())
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
		if (guild == NULL)
		{
			DEBUG_STOP;
		}
		else
		{
			// register the outpost in the guild as owned
			guild->addOwnedOutpost(_Alias);
		}
	}

	if (_AttackerGuildId != 0)
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
		if (guild == NULL)
		{
			DEBUG_STOP;
		}
		else
		{
			// register the outpost in the guild as challenged
			guild->addChallengedOutpost(_Alias);
		}
	}
}

//----------------------------------------------------------------------------
bool COutpost::isBelongingToAGuild() const
{
	return (_OwnerGuildId != 0);
}

//----------------------------------------------------------------------------
uint32 COutpost::getAIInstanceNumber() const
{
	return CUsedContinent::instance().getInstanceForContinent( _Continent ); 
}

//----------------------------------------------------------------------------
NLNET::TServiceId COutpost::getAISId() const
{
	uint32 aiInstanceNumber = CUsedContinent::instance().getInstanceForContinent( _Continent );
	if ( aiInstanceNumber == INVALID_AI_INSTANCE )
	{
		OUTPOST_WRN( "AiInstanceNumber not found for continent %s", CONTINENT::toString( _Continent ).c_str() );
		return NLNET::TServiceId(0);
	}
	else
	{
		NLNET::TServiceId aisId = CWorldInstances::instance().getAISId( aiInstanceNumber );
		if ( aisId.get() == 0 )
			OUTPOST_DBG( "No online AIS for instance %u / continent %s", aiInstanceNumber, CONTINENT::toString( _Continent ).c_str() );
		return aisId;
	}
}

//----------------------------------------------------------------------------
// Sets the guild and resend data to AIS. 
// The case when _OwnerGuildId==ownerGuild only resends data (see resendDynamicDataToAIS()).
void COutpost::setOwnerGuild( EGSPD::TGuildId ownerGuild )
{
	// clear the attacker if the new owner was the attacker
	if (ownerGuild != 0 && ownerGuild == _AttackerGuildId)
		setAttackerGuild(0);
	
	EGSPD::TGuildId oldOwnerGuildId = _OwnerGuildId;
	
	_OwnerGuildId = ownerGuild;
	OUTPOST_DBG( "Outpost %s is now owned by 0x%x", _Name.c_str(), _OwnerGuildId );
	
	if (ownerGuild != oldOwnerGuildId)
	{
		CGuild* oldOwner = CGuildManager::getInstance()->getGuildFromId(oldOwnerGuildId);
		if (oldOwner != NULL)
		{
			// unregister old owner
			oldOwner->removeOwnedOutpost(_Alias);
		}

		// reset the outpost buildings
		for (vector<COutpostBuilding>::iterator it = _Buildings.begin(); it != _Buildings.end(); ++it)
		{
			(*it).construct( (*it).getDefaultSheet() );
		}
	}

	CGuild* owner = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
	if (owner != NULL)
	{
		// register new owner
		owner->addOwnedOutpost(_Alias);
	}
	
	// Send to AIS
	CSetOutpostOwner ownerParams;
	ownerParams.Outpost = getAlias();
	ownerParams.Owner = _OwnerGuildId;
	sendOutpostMessage( "OUTPOST_OWNER", ownerParams );
}

//----------------------------------------------------------------------------
// Sets the guild and resend data to AIS. 
// The case when _Owner==attackerGuild only resends data (see resendDynamicDataToAIS()).
void COutpost::setAttackerGuild( EGSPD::TGuildId attackerGuild )
{
	// clear the owner if the new attacker was the owner
	if (attackerGuild != 0 && attackerGuild == _OwnerGuildId)
		setOwnerGuild(0);

	EGSPD::TGuildId oldAttackerGuildId = _AttackerGuildId;

	_AttackerGuildId = attackerGuild;
	OUTPOST_DBG( "Outpost %s is now attacked by 0x%x", _Name.c_str(), _AttackerGuildId );

	if (attackerGuild != oldAttackerGuildId)
	{
		CGuild* oldAttacker = CGuildManager::getInstance()->getGuildFromId(oldAttackerGuildId);
		if (oldAttacker != NULL)
		{
			// unregister old challenger
			oldAttacker->removeChallengedOutpost(_Alias);
		}
	}

	CGuild* attacker = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
	if (attacker != NULL)
	{
		// register new challenger
		attacker->addChallengedOutpost(_Alias);
	}
	
	// Send to AIS
	CSetOutpostAttacker attackerParams;
	attackerParams.Outpost = getAlias();
	attackerParams.Attacker = _AttackerGuildId;
	sendOutpostMessage( "OUTPOST_ATTACKER", attackerParams );
}

//----------------------------------------------------------------------------
void COutpost::setState(OUTPOSTENUMS::TOutpostState state)
{
	if( state != _State )
	{
		switch( state )
		{
			case OUTPOSTENUMS::Peace :
			{	
				clearBanishment();
			}
			break;
			case OUTPOSTENUMS::AttackRound :
			{
				broadcastMessage(OwnerGuild, AttackRounds);
				broadcastMessage(AttackerGuild, AttackRounds);
			}
			break;
			case OUTPOSTENUMS::DefenseRound :
			{	
				broadcastMessage(OwnerGuild, DefenseRounds);
				broadcastMessage(AttackerGuild, DefenseRounds);
			}
			break;
			case OUTPOSTENUMS::WarDeclaration :
			{
				broadcastMessage(OwnerGuild, WarDeclared);
				broadcastMessage(AttackerGuild, WarDeclared);
			}
			default:
				break;
		}
	}

	OUTPOST_DBG( "Outpost %s: [%s] -> [%s]", _Name.c_str(), OUTPOSTENUMS::toString( _State ).c_str(), OUTPOSTENUMS::toString( state ).c_str() );
	_State = state;
	
	// Send to AIS
	COutpostSetStateMsg params;
	params.Outpost = getAlias();
	params.State = _State;
	sendOutpostMessage("OUTPOST_STATE", params);
}

//----------------------------------------------------------------------------
std::string COutpost::getStateName() const
{
	if (_State != OUTPOSTENUMS::UnknownOutpostState)
		return "OUTPOST_STATE_" + OUTPOSTENUMS::toString( _State );
	else
		return string();
}

//----------------------------------------------------------------------------
COutpost::TChallengeOutpostErrors COutpost::challengeOutpost( CGuild *attackerGuild, bool simulate )
{
	OUTPOST_DBG( "Outpost %s: Challenged by %s", _Name.c_str(), attackerGuild->getName().toString().c_str() );

	nlassert( attackerGuild->getId() != 0 );

	if (!attackerGuild->canAddOutpost())
		return COutpost::TooManyGuildOutposts;

	// validate guild attacker
	bool guildAttackerValid = false;

	const CStaticOutpost * outpostForm = CSheets::getOutpostForm(getSheet());
	if(outpostForm == 0)
		return COutpost::InvalidOutpost;

	sint32 guildMemberLevel = (sint32)outpostForm->Level - (sint32)NumberDaysMinusOutpostLevelForChallenge;
	if(guildMemberLevel < 1) guildMemberLevel = 1;

	for ( std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::const_iterator it = attackerGuild->getMembersBegin(); it != attackerGuild->getMembersEnd(); ++it )
	{
		CCharacter * member = PlayerManager.getChar((*it).first);
		if(member)
		{
			if(member->getSkillValue(member->getBestSkill()) >= guildMemberLevel)
			{
				CGuildMember* member = EGS_PD_CAST<CGuildMember*> ( (*it).second );
				EGS_PD_AST(member);

				if( outpostForm->Level/NumberDayFactorGuildNeedForChallengeOutpost < ((CTickEventHandler::getGameCycle() - member->getEnterTime()) / (days/CTickEventHandler::getGameTimeStep())) )
				{
					guildAttackerValid = true;
					break;
				}
			}
		}
	}
	if(!guildAttackerValid)
	{
		return COutpost::BadGuildMemberLevel;
	}

	if (!simulate)
	{
		setAttackerGuild(attackerGuild->getId());
		eventTriggered(OUTPOSTENUMS::Challenged);
	}

	log_Outpost_Challenge(_Name, 
		this->_OwnerGuildId ? CGuildManager::getInstance()->getGuildFromId(this->_OwnerGuildId)->getName().toUtf8() : "TRIBES_OWNED",
		attackerGuild->getName().toUtf8());

	return COutpost::NoError;
}

//----------------------------------------------------------------------------
void COutpost::ownerGuildVanished()
{
	OUTPOST_DBG( "Outpost %s: Owner guild vanished", _Name.c_str() );

	eventTriggered(OUTPOSTENUMS::OwnerVanished);
}

//----------------------------------------------------------------------------
void COutpost::attackerGuildVanished()
{
	OUTPOST_DBG( "Outpost %s: Attacker guild vanished", _Name.c_str() );

	eventTriggered(OUTPOSTENUMS::AttackerVanished);
}

//----------------------------------------------------------------------------
void COutpost::giveupAttack()
{
	OUTPOST_DBG( "Outpost %s: Attacker gave up", _Name.c_str() );

	eventTriggered(OUTPOSTENUMS::AttackerGiveUp);
}

//----------------------------------------------------------------------------
void COutpost::giveupOwnership()
{
	OUTPOST_DBG( "Outpost %s: Owner gave up", _Name.c_str() );

	eventTriggered(OUTPOSTENUMS::OwnerGiveUp);
}

//----------------------------------------------------------------------------
// Squads used to defend the outpost during the attack
void COutpost::resetDefaultAttackSquads()
{
	if (isBelongingToAGuild())
	{
		for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
		{
			TAIAlias spawnZone = getRandomSpawnZone();
			if (spawnZone != CAIAliasTranslator::Invalid)
				setNextAttackSquadA(i, COutpostSquadData(_Alias, _DefaultSquads[0], spawnZone));
			else
				OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
		}
		for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
		{
			TAIAlias spawnZone = getRandomSpawnZone();
			if (spawnZone != CAIAliasTranslator::Invalid)
				setNextAttackSquadB(i, COutpostSquadData(_Alias, _DefaultSquads[0], spawnZone));
			else
				OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
		}
	}
	else
	{
		for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
		{
			nlassert(i<_TribeSquadsA.size());
			TAIAlias spawnZone = getRandomSpawnZone();
			if (spawnZone != CAIAliasTranslator::Invalid)
				setNextAttackSquadA(i, COutpostSquadData(_Alias, _TribeSquadsA[i], spawnZone));
			else
				OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
		}
		for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
		{
			nlassert(i<_TribeSquadsB.size());
			TAIAlias spawnZone = getRandomSpawnZone();
			if (spawnZone != CAIAliasTranslator::Invalid)
				setNextAttackSquadB(i, COutpostSquadData(_Alias, _TribeSquadsB[i], spawnZone));
			else
				OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
		}
	}
}

//----------------------------------------------------------------------------
// Squads to attack during the defense (counter-attack)
void COutpost::resetDefaultDefenseSquads()
{
	for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
	{
		TAIAlias spawnZone = getRandomSpawnZone();
		if (spawnZone != CAIAliasTranslator::Invalid)
			setNextDefenseSquadA(i, COutpostSquadData(0, _DefaultSquads[0], spawnZone));
		else
			OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
	}
	for (uint32 i=0; i<OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS; ++i)
	{
		TAIAlias spawnZone = getRandomSpawnZone();
		if (spawnZone != CAIAliasTranslator::Invalid)
			setNextDefenseSquadB(i, COutpostSquadData(0, _DefaultSquads[0], spawnZone));
		else
			OUTPOST_WRN( "Outpost %s: invalid spawn zone", _Name.c_str() );
	}
}

//----------------------------------------------------------------------------
void COutpost::aieventSquadCreated(uint32 createOrder, uint32 groupId)
{
	NLMISC::CSmartPtr<COutpostSquad> createdSquad = COutpostSquad::getSquadFromCreateOrder(createOrder);
	if (createdSquad != NULL)
	{
		createdSquad->created(createOrder, groupId);
		askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
	}
	else
	{
		OUTPOST_WRN("Outpost %s: A squad was created but no pending creation request was found", _Name.c_str());
	}
}

//----------------------------------------------------------------------------
// Spawning squads must be active
void COutpost::aieventSquadSpawned(uint32 groupId)
{
	vector<COutpostSquadPtr>::iterator it, itEnd;
	itEnd = _CurrentSquadsA.end();
	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
//			++_FightData._SpawnedSquadsA;
			(*it)->spawned();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	itEnd = _CurrentSquadsB.end();
	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
//			++_FightData._SpawnedSquadsB;
			(*it)->spawned();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	OUTPOST_WRN("Outpost %s: A squad spawned (with id 0x%08x) but we don't have it registered", _Name.c_str(), groupId);
}

//----------------------------------------------------------------------------
// Despawning squads can be in any container
void COutpost::aieventSquadDespawned(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: squad 0x%08x despawned", _Name.c_str(), groupId );

	vector<COutpostSquadPtr>::iterator it, itEnd;
	itEnd = _CurrentSquadsA.end();
	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
			(*it)->despawned();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	itEnd = _CurrentSquadsB.end();
	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
			(*it)->despawned();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	OUTPOST_WRN("A squad despawned (with id 0x%08x) but we don't have it registered", groupId);
}

//----------------------------------------------------------------------------
// Dying squads can be either active (last member is leader) or zombie (leader already died)
void COutpost::aieventSquadDied(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: squad 0x%8x dead", _Name.c_str(), groupId );
	vector<COutpostSquadPtr>::iterator it, itEnd;
	itEnd = _CurrentSquadsA.end();
	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
			++_FightData._KilledSquads;			
			(*it)->died();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	itEnd = _CurrentSquadsB.end();
	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
			++_FightData._KilledSquads;			
			(*it)->died();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	OUTPOST_WRN("A squad died (with id 0x%08x) but we don't have it registered", groupId);
}

//----------------------------------------------------------------------------
// Dying squads must be active
void COutpost::aieventSquadLeaderDied(uint32 groupId)
{
	OUTPOST_DBG( "Outpost %s: squad leader 0x%8x dead", _Name.c_str(), groupId );
	vector<COutpostSquadPtr>::iterator it, itEnd;
	itEnd = _CurrentSquadsA.end();
	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{

//			++_FightData._KilledSquads;			
			(*it)->leaderDied();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	itEnd = _CurrentSquadsB.end();
	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
	{
		if (!it->isNull() && (*it)->getGroupId()==groupId)
		{
//			++_FightData._KilledSquads;			
			(*it)->leaderDied();
			askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
			return;
		}
	}
	OUTPOST_WRN("A squad leader died (with id 0x%08x) but we don't have its squad registered", groupId);
}

//----------------------------------------------------------------------------
void COutpost::aisUp()
{
	OUTPOST_DBG( "Outpost %s: resending dynamic data to AIS", _Name.c_str() );
	
	nlassert(!_AISUp);
	_AISUp = true;

	setOwnerGuild( _OwnerGuildId );
	setAttackerGuild( _AttackerGuildId );

	setState( _State );
	
//	vector<COutpostSquadPtr>::iterator it, itEnd;
//	itEnd = _CurrentSquadsA.end();
//	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
//		if (!it->isNull())
//			(*it)->AISUp();
//	itEnd = _CurrentSquadsB.end();
//	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
//		if (!it->isNull())
//			(*it)->AISUp();
	
	eventTriggered(OUTPOSTENUMS::EventAisUp);

	// buildings
	for (uint i = 0; i < _Buildings.size(); ++i)
		_Buildings[i].onAISUp();
}

//----------------------------------------------------------------------------
void COutpost::aisDown()
{
	OUTPOST_DBG( "Outpost %s: resetting dynamic data of AIS", _Name.c_str() );
	
	nlassert(_AISUp);
	_AISUp = false;
	
	vector<COutpostSquadPtr>::iterator it, itEnd;
	itEnd = _CurrentSquadsA.end();
	for (it=_CurrentSquadsA.begin(); it!=itEnd; ++it)
		if (!it->isNull())
		{
			(*it)->AISDown();
			(*it) = NULL;
		}
	itEnd = _CurrentSquadsB.end();
	for (it=_CurrentSquadsB.begin(); it!=itEnd; ++it)
		if (!it->isNull())
		{
			(*it)->AISDown();
			(*it) = NULL;
		}
	
	eventTriggered(OUTPOSTENUMS::EventAisDown);
	
	_CurrentSquadsA.clear();
	_CurrentSquadsB.clear();

	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
}

//----------------------------------------------------------------------------
void COutpost::updateOutpost(uint32 currentTime)
{
	// Transitions
	if (_Timer0EndTime!=0 && currentTime>_Timer0EndTime)
	{
		_Timer0EndTime = 0;
		eventTriggered(OUTPOSTENUMS::Timer0End);
	}
	if (_Timer1EndTime!=0 && currentTime>_Timer1EndTime)
	{
		_Timer1EndTime = 0;
		eventTriggered(OUTPOSTENUMS::Timer1End);
	}
	if (_Timer2EndTime!=0 && currentTime>_Timer2EndTime)
	{
		_Timer2EndTime = 0;
		eventTriggered(OUTPOSTENUMS::Timer2End);
	}
	// States
	{
		switch (_State)
		{
		case OUTPOSTENUMS::Peace:
		case OUTPOSTENUMS::WarDeclaration:
		case OUTPOSTENUMS::AttackBefore:
		case OUTPOSTENUMS::AttackRound:
		case OUTPOSTENUMS::AttackAfter:
		case OUTPOSTENUMS::DefenseBefore:
		case OUTPOSTENUMS::DefenseAfter:
		case OUTPOSTENUMS::DefenseRound:
			break;
		default:
			nlerror("Undefined state in outpost");
		}
	}
	
	// Child updates
	// Squads
	
	for (vector<COutpostSquadPtr>::iterator its=_CurrentSquadsA.begin(); its!=_CurrentSquadsA.end(); ++its)
	{
		if (!its->isNull() && !(*its)->updateSquad(currentTime) )
		{
			(*its) = NULL;
			OUTPOST_DBG( "Outpost %s: Squad A removed", _Name.c_str() );
		}
	}
	for (vector<COutpostSquadPtr>::iterator its=_CurrentSquadsB.begin(); its!=_CurrentSquadsB.end(); ++its)
	{
		if (!its->isNull() && !(*its)->updateSquad(currentTime))
		{
			(*its) = NULL;
			OUTPOST_DBG( "Outpost %s: Squad B removed", _Name.c_str() );
		}
	}

	// update buildings
	for (uint i = 0; i < _Buildings.size(); ++i)
		_Buildings[i].update(currentTime);

	bool updateClientTimers = false;

	// State machine stuff
	if (_NextState!=OUTPOSTENUMS::UnknownOutpostState)
	{
		OUTPOST_DBG("Outpost %s: Switching from state '%s' to '%s'", _Name.c_str(), OUTPOSTENUMS::toString(_State).c_str(), OUTPOSTENUMS::toString(_NextState).c_str());
		eventTriggered(OUTPOSTENUMS::EndOfState);
		setState(_NextState);
		_NextState = OUTPOSTENUMS::UnknownOutpostState;
		eventTriggered(OUTPOSTENUMS::StartOfState);
		updateClientTimers = true;
	}

	// check if it is time to update client timers
	if (_LastUpdateOfTimersForClient + OutpostClientTimersUpdatePeriod.get() < CTime::getSecondsSince1970())
		updateClientTimers = true;

	if (updateClientTimers)
	{
		updateTimersForClient();
		askOutpostDBUpdate();
		askGuildDBUpdate(COutpostGuildDBUpdater::STATE_END_DATE);
	}

	// update the outpost database if needed
	if (_NeedOutpostDBUpdate)
	{
		fillOutpostDB();
		sendOutpostDBDeltas();
		_NeedOutpostDBUpdate = false;
	}
}

//----------------------------------------------------------------------------
void COutpost::simulateTimer0End(uint32 endTime)
{
	OUTPOST_DBG( "Outpost %s: Timer 0 end sim", _Name.c_str() );
	if (_Timer0EndTime!=0)
		_Timer0EndTime = endTime;
	else
		OUTPOST_WRN("Cannot simulate timer 0 end in outpost %s, timer is not running.", _Name.c_str());
}

//----------------------------------------------------------------------------
void COutpost::simulateTimer1End(uint32 endTime)
{
	OUTPOST_DBG( "Outpost %s: Timer 1 end sim", _Name.c_str() );
	if (_Timer1EndTime!=0)
		_Timer1EndTime = endTime;
	else
		OUTPOST_WRN("Cannot simulate timer 1 end in outpost %s, timer is not running.", _Name.c_str());
}

//----------------------------------------------------------------------------
void COutpost::simulateTimer2End(uint32 endTime)
{
	OUTPOST_DBG( "Outpost %s: Timer 2 end sim", _Name.c_str() );
	if (_Timer2EndTime!=0)
		_Timer2EndTime = endTime;
	else
		OUTPOST_WRN("Cannot simulate timer 2 end in outpost %s, timer is not running.", _Name.c_str());
}

//----------------------------------------------------------------------------
bool COutpost::setSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, uint32 shopSquadIndex)
{
	COutpostSquadData * squad = getSquadFromSlot(side, squadSlot);
	if (squad == NULL)
		return false;

	COutpostSquadDescriptor squadDesc;
	if (!convertShopSquadIndex(shopSquadIndex, squadDesc))
		return false;

	squad->setSquadDescriptor(squadDesc);
	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_TRAINING);

	return true;
}

//----------------------------------------------------------------------------
bool COutpost::setSquadSpawnZone(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, uint32 spawnZoneIndex)
{
	COutpostSquadData * squad = getSquadFromSlot(side, squadSlot);
	if (squad == NULL)
		return false;

	TAIAlias spawnZoneAlias;
	if (!convertSpawnZoneIndex(spawnZoneIndex, spawnZoneAlias))
		return false;

	squad->setSpawnZone(spawnZoneAlias);
	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_TRAINING);

	return true;
}

//----------------------------------------------------------------------------
bool COutpost::insertDefaultSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot)
{
	vector<COutpostSquadData> * squads;
	uint32 squadIndex;
	if (!getSquadFromSlot(side, squadSlot, squads, squadIndex))
		return false;

	// right shift slots
	if (squads->size() >= 2)
	{
		uint32 i = (uint32)squads->size()-2;
		while (i >= squadIndex)
		{
			(*squads)[i+1] = (*squads)[i];

			if (i == 0)
				break;
			--i;
		}
	}

	// set a default squad
	(*squads)[squadIndex].setSquadDescriptor(_DefaultSquads[0]);
	(*squads)[squadIndex].setSpawnZone(getRandomSpawnZone());

	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_TRAINING);

	return true;
}

//----------------------------------------------------------------------------
bool COutpost::removeSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot)
{
	vector<COutpostSquadData> * squads;
	uint32 squadIndex;
	if (!getSquadFromSlot(side, squadSlot, squads, squadIndex))
		return false;

	// left shift slots
	while (squadIndex < squads->size()-1)
	{
		(*squads)[squadIndex] = (*squads)[squadIndex+1];
		++squadIndex;
	}

	// set a default squad
	(*squads)[squadIndex].setSquadDescriptor(_DefaultSquads[0]);
	(*squads)[squadIndex].setSpawnZone(getRandomSpawnZone());

	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_TRAINING);

	return true;
}

//----------------------------------------------------------------------------
void COutpost::setOwnerExpenseLimit(uint32 expenseLimit)
{
	_OwnerExpenseLimit = expenseLimit;
	askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
}

//----------------------------------------------------------------------------
void COutpost::setAttackerExpenseLimit(uint32 expenseLimit)
{
	_AttackerExpenseLimit = expenseLimit;
	askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
}

//----------------------------------------------------------------------------
uint32 COutpost::getChallengeCost() const
{
	return _Form->ChallengeCost;
}

//----------------------------------------------------------------------------
TAIAlias COutpost::getRandomSpawnZone() const
{
	// choose a random spawn zone
	sint32 randomCount = RandomGenerator.rand((uint16)_SpawnZones.size() - 1);
	nlassert(randomCount >= 0 && randomCount < (sint32)_SpawnZones.size());
	return _SpawnZones[randomCount].alias();
}

//----------------------------------------------------------------------------
bool COutpost::leavePVP(CCharacter * user, IPVP::TEndType type)
{
	nlassert(user);

	switch (type)
	{
	case IPVP::LeavePVPZone:
		user->getPVPInterface().reset();
		_Users.erase( user->getEntityRowId() );
		break;
	case IPVP::Disconnect:
		_Users.erase( user->getEntityRowId() );
		user->startOutpostLeavingTimer();
		break;
	case IPVP::EnterPVPZone:
	case IPVP::Teleport:
		user->startOutpostLeavingTimer();
		break;

	default:
		// ignore all other events
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
void COutpost::addPlayer(CCharacter * user)
{
	nlassert(user);

	_Users.insert( user->getEntityRowId() );
}

//----------------------------------------------------------------------------
bool COutpost::isCharacterInConflict(CCharacter *user) const
{
	H_AUTO(COutpost_isPlayerInConflict);
	BOMB_IF( ! user, "<COutpost::isCharacterInConflict> user is null", return false );

	// when outpost war is started, everybody can participate
	return ( _State > OUTPOSTENUMS::Peace);
}

//----------------------------------------------------------------------------
//	getPVPRelation
//
//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation COutpost::getPVPRelation( CCharacter * user, CEntityBase * target ) const
{
	H_AUTO(COutpost_getPVPRelation);
	BOMB_IF( ! (user && target), "ErrorCUHTT", return PVP_RELATION::Unknown );
	
	if( IsRingShard )
		return PVP_RELATION::Neutral;


	bool targetSafe = false;
	bool actorSafe = false;

	if( target->getOutpostAlias() == 0 )
	{
		return PVP_RELATION::Neutral;
	}
	
	CCharacter * pTarget = dynamic_cast<CCharacter*>(target);
	if (pTarget == 0)
		return PVP_RELATION::Unknown;

	if (CPVPManager2::getInstance()->inSafeZone(pTarget->getPosition()))
	{
		if (pTarget->getSafeInPvPSafeZone())
			targetSafe = true;
	}

	if( CPVPManager2::getInstance()->inSafeZone(user->getPosition()))
	{
		if( user->getSafeInPvPSafeZone())
			actorSafe = true;
	}

	// One is safe but not other => NeutralPVP
	if ((targetSafe && !actorSafe) || (actorSafe && !targetSafe)) {
		return PVP_RELATION::NeutralPVP;
	}

	if( user->getOutpostAlias() == target->getOutpostAlias() )
	{
		if( user->getOutpostSide() != target->getOutpostSide() )
		{
			if (!targetSafe && !actorSafe)
			{
				CPVPManager2::getInstance()->setPVPOutpostEnemyReminder( true );
				return PVP_RELATION::Ennemy;
			}
		}
		else
		{
			CPVPManager2::getInstance()->setPVPOutpostAllyReminder( true );
			return PVP_RELATION::Ally;
		}
	}

	return PVP_RELATION::NeutralPVP;

} // getPVPRelation //

//----------------------------------------------------------------------------
bool COutpost::isPlayerBanishedForAttack( CEntityId& id ) const
{
	set<CEntityId>::const_iterator it = _AttackBanishedPlayers.find( id );
	if( it != _AttackBanishedPlayers.end() )
	{
		return true;
	}
	else
	{
		// test if guild is banished
		CCharacter * character = PlayerManager.getChar( id );
		if( character )
		{
			return isGuildBanishedForAttack( character->getGuildId() );
		}
		return false;
	}

}

//----------------------------------------------------------------------------
bool COutpost::isPlayerBanishedForDefense( CEntityId& id ) const
{
	set<CEntityId>::const_iterator it = _DefenseBanishedPlayers.find( id );
	if( it != _DefenseBanishedPlayers.end() )
	{
		return true;
	}
	else
	{
		// test if guild is banished
		CCharacter * character = PlayerManager.getChar( id );
		if( character )
		{
			return isGuildBanishedForDefense( character->getGuildId() );
		}
		return false;
	}
	
}

//----------------------------------------------------------------------------
bool COutpost::isGuildBanishedForAttack( uint32 guildId ) const
{
	set<uint32>::const_iterator it = _AttackBanishedGuilds.find( guildId );
	if( it != _AttackBanishedGuilds.end() )
		return true;
	else
		return false;

}

//----------------------------------------------------------------------------
bool COutpost::isGuildBanishedForDefense( uint32 guildId ) const
{
	set<uint32>::const_iterator it = _DefenseBanishedGuilds.find( guildId );
	if( it != _DefenseBanishedGuilds.end() )
		return true;
	else
		return false;
	
}

//-----------------------------------------------------------------------------
void COutpost::banishPlayerForDefense( CEntityId& id )
{
	set<CEntityId>::const_iterator it = _DefenseBanishedPlayers.find( id );
	if( it != _DefenseBanishedPlayers.end() )
		return;
	else
		_DefenseBanishedPlayers.insert( id );

	CCharacter * character = PlayerManager.getChar( id );
	if( character && character->getEnterFlag() )
	{
		// remove player from outpost conflict
		CCharacter::sendDynamicSystemMessage( character->getEntityRowId(), "OUTPOST_PLAYER_BANISHED_DEFENSE" );
		character->setOutpostAlias( 0 );
	}
}

//-----------------------------------------------------------------------------
void COutpost::banishPlayerForAttack( CEntityId& id )
{
	set<CEntityId>::const_iterator it = _AttackBanishedPlayers.find( id );
	if( it != _AttackBanishedPlayers.end() )
		return;
	else
		_AttackBanishedPlayers.insert( id );

	CCharacter * character = PlayerManager.getChar( id );
	if( character && character->getEnterFlag() )
	{
		// remove player from outpost conflict
		CCharacter::sendDynamicSystemMessage( character->getEntityRowId(), "OUTPOST_PLAYER_BANISHED_ATTACK" );
		character->setOutpostAlias( 0 );
	}
}

//----------------------------------------------------------------------------
void COutpost::banishGuildForAttack( uint32 guildId )
{
	if( guildId == 0 )
	{
		OUTPOST_WRN("<COutpost::banishGuildForAttack> guild id 0 is not a valid guild id");
		return;
	}

	set<uint32>::const_iterator it = _AttackBanishedGuilds.find( guildId );
	if( it != _AttackBanishedGuilds.end() )
		return;
	else
		_AttackBanishedGuilds.insert( guildId );

	// each members of guild are removed from outpost conflict
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( guildId );
	if( guild )
	{
		map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it;
		for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
		{
			CCharacter * c = PlayerManager.getChar( (*it).first );
			if( c && c->getEnterFlag() )
			{
				if(c->getOutpostSide() == OUTPOSTENUMS::OutpostAttacker)
				{
					CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "OUTPOST_GUILD_BANISHED_ATTACK" );
					c->setOutpostAlias( 0 );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void COutpost::banishGuildForDefense( uint32 guildId )
{
	if( guildId == 0 )
	{
		OUTPOST_WRN("<COutpost::banishGuild> guild id 0 is not a valid guild id");
		return;
	}
	
	set<uint32>::const_iterator it = _DefenseBanishedGuilds.find( guildId );
	if( it != _DefenseBanishedGuilds.end() )
		return;
	else
		_DefenseBanishedGuilds.insert( guildId );
	
	// each members of guild are removed from outpost conflict
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( guildId );
	if( guild )
	{
		map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it;
		for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
		{
			CCharacter * c = PlayerManager.getChar( (*it).first );
			if( c && c->getEnterFlag() )
			{
				if(c->getOutpostSide() == OUTPOSTENUMS::OutpostOwner)
				{
					CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "OUTPOST_GUILD_BANISHED_DEFENSE" );
					c->setOutpostAlias( 0 );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void COutpost::unBanishPlayerForAttack( NLMISC::CEntityId& id )
{
	set<CEntityId>::iterator it = _AttackBanishedPlayers.find( id );
	if( it != _AttackBanishedPlayers.end() )
		_AttackBanishedPlayers.erase( it );
}

//----------------------------------------------------------------------------
void COutpost::unBanishPlayerForDefense( NLMISC::CEntityId& id )
{
	set<CEntityId>::iterator it = _DefenseBanishedPlayers.find( id );
	if( it != _DefenseBanishedPlayers.end() )
		_DefenseBanishedPlayers.erase( it );
}

//----------------------------------------------------------------------------
void COutpost::unBanishGuildForAttack( uint32 guildId )
{
	set<uint32>::iterator it = _AttackBanishedGuilds.find( guildId );
	if( it != _AttackBanishedGuilds.end() )
		_AttackBanishedGuilds.erase( it );
}

//----------------------------------------------------------------------------
void COutpost::unBanishGuildForDefense( uint32 guildId )
{
	set<uint32>::iterator it = _DefenseBanishedGuilds.find( guildId );
	if( it != _DefenseBanishedGuilds.end() )
		_DefenseBanishedGuilds.erase( it );
}

//----------------------------------------------------------------------------
void COutpost::clearBanishment()
{
	_AttackBanishedPlayers.clear();
	_DefenseBanishedPlayers.clear();
	_AttackBanishedGuilds.clear();
	_DefenseBanishedGuilds.clear();
}

//----------------------------------------------------------------------------
void COutpost::preStore() const
{
	nlassert(_NextAttackSquadsA.size()==OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_NextAttackSquadsB.size()==OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_NextDefenseSquadsA.size()==OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_NextDefenseSquadsB.size()==OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_CurrentSquadsAQueue.size()<=OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(_CurrentSquadsBQueue.size()<=OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
}

//----------------------------------------------------------------------------
void COutpost::preLoad()
{
	// Loading must be done before receiving AIS up event
	nlassert(!_AISUp);
}

//----------------------------------------------------------------------------
void COutpost::postLoad()
{
	_Form = CSheets::getOutpostForm(_Sheet);
	nlassert(_Form);
	OUTPOST_DBG( "Loaded outpost %s owned by 0x%x", CPrimitivesParser::aliasToString( _Alias ).c_str(), _OwnerGuildId );

	// Set PVP on/off
	switch ( _State )
	{
	case OUTPOSTENUMS::Peace:
		actionSetPVPActive( false );
		break;
	default:
		actionSetPVPActive( true );
	}

	// update the tribe outpost level
	if (!isBelongingToAGuild())
		_CurrentOutpostLevel = computeTribeOutpostLevel();

	// check squad vectors and resize them if necessary
	{
		TAIAlias spawnZone = getRandomSpawnZone();
		nlassert(_DefaultSquads.size() > 0);
		nlassert(spawnZone != CAIAliasTranslator::Invalid);

		const uint newSize = OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS;
		if (_NextAttackSquadsA.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_NextAttackSquadsA.resize(newSize,	COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
		if (_NextAttackSquadsB.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_NextAttackSquadsB.resize(newSize,	COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
		if (_NextDefenseSquadsA.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_NextDefenseSquadsA.resize(newSize,	COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
		if (_NextDefenseSquadsB.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_NextDefenseSquadsB.resize(newSize,	COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
		if (_CurrentSquadsAQueue.size() != 0 && _CurrentSquadsAQueue.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_CurrentSquadsAQueue.resize(newSize, COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
		if (_CurrentSquadsBQueue.size() != 0 && _CurrentSquadsAQueue.size() != OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
			_CurrentSquadsBQueue.resize(newSize, COutpostSquadData(0, _DefaultSquads[0], spawnZone) );
	}

	// process all squads
	for ( vector<COutpostSquadData>::iterator it=_NextAttackSquadsA.begin(); it!=_NextAttackSquadsA.end(); ++it )
		postLoadSquad(*it);
	for ( vector<COutpostSquadData>::iterator it=_NextAttackSquadsB.begin(); it!=_NextAttackSquadsB.end(); ++it )
		postLoadSquad(*it);
	for ( vector<COutpostSquadData>::iterator it=_NextDefenseSquadsA.begin(); it!=_NextDefenseSquadsA.end(); ++it )
		postLoadSquad(*it);
	for ( vector<COutpostSquadData>::iterator it=_NextDefenseSquadsB.begin(); it!=_NextDefenseSquadsB.end(); ++it )
		postLoadSquad(*it);
	for ( vector<COutpostSquadData>::iterator it=_CurrentSquadsAQueue.begin(); it!=_CurrentSquadsAQueue.end(); ++it )
		postLoadSquad(*it);
	for ( vector<COutpostSquadData>::iterator it=_CurrentSquadsBQueue.begin(); it!=_CurrentSquadsBQueue.end(); ++it )
		postLoadSquad(*it);

	if ( _AISUp )
	{
		// Reset dynamic data, because when we get the AIS up event resendDynamicDataToAIS() will be called
		aisDown();
	}
}

//----------------------------------------------------------------------------
void COutpost::postLoadSquad(COutpostSquadData & squadData)
{
	// set the outpost alias
	squadData.setOutpostAlias(_Alias);

	// check that the squad spawn zone is still valid
	std::vector<COutpostSpawnZone>::const_iterator it;
	std::vector<COutpostSpawnZone>::const_iterator itEnd = _SpawnZones.end();
	for (it = _SpawnZones.begin(); it != itEnd; ++it)
	{
		if ((*it).alias() == squadData.getSpawnZone())
			break;
	}
	// if not, set a random spawn zone
	if (it == itEnd)
		squadData.setSpawnZone( getRandomSpawnZone() );
}

//----------------------------------------------------------------------------
bool COutpost::getSpawnZoneIndex(TAIAlias spawnZoneAlias, uint32 & spawnZoneIndex) const
{
	for (uint i = 0; i < _SpawnZones.size(); i++)
	{
		if (_SpawnZones[i].alias() == spawnZoneAlias)
		{
			spawnZoneIndex = i;
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
bool COutpost::convertShopSquadIndex(uint32 shopSquadIndex, COutpostSquadDescriptor & squadDesc) const
{
	if (shopSquadIndex < _DefaultSquads.size())
	{
		squadDesc = _DefaultSquads[shopSquadIndex];
		return true;
	}
	else
	{
		shopSquadIndex -= (uint32)_DefaultSquads.size();
		if (shopSquadIndex < _BuyableSquads.size())
		{
			squadDesc = _BuyableSquads[shopSquadIndex];
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool COutpost::convertSpawnZoneIndex(uint32 spawnZoneIndex, TAIAlias & spawnZoneAlias) const
{
	if (spawnZoneIndex >= _SpawnZones.size())
		return false;
	
	spawnZoneAlias = _SpawnZones[spawnZoneIndex].alias();
	return true;
}

//----------------------------------------------------------------------------
void COutpost::askGuildDBUpdate(COutpostGuildDBUpdater::TDBPropSet dbPropSet) const
{
	COutpostManager::getInstance().askOutpostGuildDBUpdate(_Alias, dbPropSet);
}

//----------------------------------------------------------------------------
void COutpost::askOutpostDBUpdate()
{
	_NeedOutpostDBUpdate = true;
}

//----------------------------------------------------------------------------
void COutpost::setNextAttackSquadA(uint32 index, COutpostSquadData const& nextSquad)
{
	nlassert(index < _NextAttackSquadsA.size());
	nlassert(index < OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(nextSquad.getSquadDescriptor().sheet()!=CSheetId::Unknown);
	_NextAttackSquadsA[index] = nextSquad;
}	

//----------------------------------------------------------------------------
void COutpost::setNextAttackSquadB(uint32 index, COutpostSquadData const& nextSquad)
{
	nlassert(index < _NextAttackSquadsA.size());
	nlassert(index < OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS);
	nlassert(nextSquad.getSquadDescriptor().sheet()!=CSheetId::Unknown);
	_NextAttackSquadsB[index] = nextSquad;
}	

//----------------------------------------------------------------------------
void COutpost::setNextDefenseSquadA(uint32 index, COutpostSquadData const& nextSquad)
{
	nlassert(index < _NextDefenseSquadsA.size());
	_NextDefenseSquadsA[index] = nextSquad;
}	

//----------------------------------------------------------------------------
void COutpost::setNextDefenseSquadB(uint32 index, COutpostSquadData const& nextSquad)
{
	nlassert(index < _NextDefenseSquadsA.size());
	_NextDefenseSquadsB[index] = nextSquad;
}	

//----------------------------------------------------------------------------
bool COutpost::createSquad(COutpostSquadPtr& squad, COutpostSquadData const& squadData, CGuildCharProxy* leader, CGuild* originGuild, OUTPOSTENUMS::TPVPSide side)
{
	squad = COutpostSquadPtr(new COutpostSquad(_Alias, squadData.getSquadDescriptor(), squadData.getSpawnZone(), side));
	OUTPOST_DBG( "Outpost %s: squad recruited", _Name.c_str() );
	return true;
}

//----------------------------------------------------------------------------
bool COutpost::getSquadFromSlot(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, std::vector<COutpostSquadData> * &squads, uint32 & squadIndex)
{
	if (squadSlot >= OUTPOSTENUMS::OUTPOST_MAX_SQUAD_TRAINING)
		return false;

	vector<COutpostSquadData> * squadsA;
	vector<COutpostSquadData> * squadsB;
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		squadsA = &_NextAttackSquadsA;
		squadsB = &_NextAttackSquadsB;
	}
	else if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		squadsA = &_NextDefenseSquadsA;
		squadsB = &_NextDefenseSquadsB;
	}
	else
	{
		nlstop;
		return false;
	}

	if (squadSlot < OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS)
	{
		squads = squadsA;
		squadIndex = squadSlot;
	}
	else
	{
		squads = squadsB;
		squadIndex = squadSlot - OUTPOSTENUMS::OUTPOST_NB_SQUAD_SLOTS;
	}

	nlassert(squadIndex < squads->size());
	if (squadIndex >= squads->size())
		return false;

	return true;
}

//----------------------------------------------------------------------------
COutpostSquadData * COutpost::getSquadFromSlot(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot)
{
	vector<COutpostSquadData> * squads;
	uint32 squadIndex;
	if (!getSquadFromSlot(side, squadSlot, squads, squadIndex))
		return NULL;

	nlassert(squadIndex < squads->size());
	return &(*squads)[squadIndex];
}

//----------------------------------------------------------------------------
void COutpost::actionPostNextState(OUTPOSTENUMS::TOutpostState state)
{
	_NextState = state;
}

//----------------------------------------------------------------------------
void COutpost::actionSetTimer0(uint32 seconds)
{
	_Timer0EndTime = CTime::getSecondsSince1970() + seconds;
}

//----------------------------------------------------------------------------
void COutpost::actionSetTimer0End(uint32 seconds)
{
	_Timer0EndTime = seconds;
}

//----------------------------------------------------------------------------
void COutpost::actionSetTimer1(uint32 seconds)
{
	_Timer1EndTime = CTime::getSecondsSince1970() + seconds;
}

//----------------------------------------------------------------------------
void COutpost::actionSetTimer2(uint32 seconds)
{
	_Timer2EndTime = CTime::getSecondsSince1970() + seconds;
}

//----------------------------------------------------------------------------
void COutpost::actionStopTimer0()
{
	_Timer0EndTime = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionStopTimer1()
{
	_Timer1EndTime = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionStopTimer2()
{
	_Timer2EndTime = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionSetPVPActive(bool active)
{
	this->CPVPOutpostZone::setActive(active);
}

//----------------------------------------------------------------------------
void COutpost::actionBuySquadsA(uint32 squadCount, OUTPOSTENUMS::TPVPSide side)
{
	nlassert(squadCount <= _CurrentSquadsAQueue.size());
	nlassert(!_DefaultSquads.empty());

	EGSPD::TGuildId guildId;
	uint32 * expenseLimit;
	uint32 * spentMoney;
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		guildId = _OwnerGuildId;
		expenseLimit = &_OwnerExpenseLimit;
		spentMoney = &_MoneySpentByOwner;
	}
	else if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		guildId = _AttackerGuildId;
		expenseLimit = &_AttackerExpenseLimit;
		spentMoney = &_MoneySpentByAttacker;
	}
	else
	{
		nlstop;
		return;
	}

	// tribes do not have to pay
	if (guildId == 0)
	{
		if (guildId != _OwnerGuildId)
			nlerror("a tribe cannot be the outpost attacker : _OwnerGuildId=%u, _AttackerGuildId=%u", _OwnerGuildId, _AttackerGuildId);
		return;
	}

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if (guild == NULL)
	{
		OUTPOST_WRN("cannot find guild %u", guildId);
		DEBUG_STOP;
		return;
	}

	uint32 nbReplacedSquads = 0;
	for (uint i = 0; i < squadCount; ++i)
	{
		uint32 squadPrice = _CurrentSquadsAQueue[i].getSquadDescriptor().getStaticForm()->BuyPrice;
		if (squadPrice == 0)
			continue;

		if (squadPrice > (*expenseLimit) || squadPrice > guild->getMoney())
		{
			// if the guild cannot pay the squad, replace it with the default one (free)
			_CurrentSquadsAQueue[i].setSquadDescriptor(_DefaultSquads[0]);
			++nbReplacedSquads;
			continue;
		}

		guild->spendMoney(squadPrice);

		(*spentMoney) += squadPrice;
		(*expenseLimit) -= squadPrice;
	}

	if (nbReplacedSquads > 0)
	{
		// send a message to the guild members
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = nbReplacedSquads;
		guild->sendMessageToGuildMembers("OUTPOST_CANNOT_BUY_SQUADS_A", params);
	}
}

//----------------------------------------------------------------------------
void COutpost::actionBuySquadsB(uint32 squadIndex, OUTPOSTENUMS::TPVPSide side)
{
	nlassert(squadIndex < _CurrentSquadsBQueue.size());
	nlassert(!_DefaultSquads.empty());

	EGSPD::TGuildId guildId;
	uint32 * expenseLimit;
	uint32 * spentMoney;
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		guildId = _OwnerGuildId;
		expenseLimit = &_OwnerExpenseLimit;
		spentMoney = &_MoneySpentByOwner;
	}
	else if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		guildId = _AttackerGuildId;
		expenseLimit = &_AttackerExpenseLimit;
		spentMoney = &_MoneySpentByAttacker;
	}
	else
	{
		nlstop;
		return;
	}

	// tribes do not have to pay
	if (guildId == 0)
	{
		if (guildId != _OwnerGuildId)
			nlerror("a tribe cannot be the outpost attacker : _OwnerGuildId=%u, _AttackerGuildId=%u", _OwnerGuildId, _AttackerGuildId);
		return;
	}

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if (guild == NULL)
	{
		OUTPOST_WRN("cannot find guild %u", guildId);
		DEBUG_STOP;
		return;
	}

	uint32 squadPrice = _CurrentSquadsBQueue[squadIndex].getSquadDescriptor().getStaticForm()->BuyPrice;
	if (squadPrice == 0)
		return;

	if (squadPrice > (*expenseLimit) || squadPrice > guild->getMoney())
	{
		// if the guild cannot pay the squad, replace it with the default one (free)
		_CurrentSquadsBQueue[squadIndex].setSquadDescriptor(_DefaultSquads[0]);
		// send a message to the guild members
		guild->sendMessageToGuildMembers("OUTPOST_CANNOT_BUY_SQUADS_B");
		return;
	}

	guild->spendMoney(squadPrice);

	(*spentMoney) += squadPrice;
	(*expenseLimit) -= squadPrice;
}

//----------------------------------------------------------------------------
void COutpost::actionPayBackAliveSquads(OUTPOSTENUMS::TPVPSide side)
{
	EGSPD::TGuildId guildId;
	uint32 * expenseLimit;
	uint32 * spentMoney;
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		guildId = _OwnerGuildId;
		expenseLimit = &_OwnerExpenseLimit;
		spentMoney = &_MoneySpentByOwner;
	}
	else if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		guildId = _AttackerGuildId;
		expenseLimit = &_AttackerExpenseLimit;
		spentMoney = &_MoneySpentByAttacker;
	}
	else
	{
		nlstop;
		return;
	}

	// tribes do not get money
	if (guildId == 0)
	{
		if (guildId != _OwnerGuildId)
			nlerror("a tribe cannot be the outpost attacker : _OwnerGuildId=%u, _AttackerGuildId=%u", _OwnerGuildId, _AttackerGuildId);
		return;
	}

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if (guild == NULL)
	{
		OUTPOST_WRN("cannot find guild %u", guildId);
		DEBUG_STOP;
		return;
	}

	// pay back alive squads A
	for (uint i = 0; i < _CurrentSquadsA.size(); ++i)
	{
		const COutpostSquadPtr & squad = _CurrentSquadsA[i];
		if (squad.isNull() || squad->isDead())
			continue;

		nlassert(i < _CurrentSquadsAQueue.size());
		uint32 squadPrice = _CurrentSquadsAQueue[i].getSquadDescriptor().getStaticForm()->BuyPrice;
		if (squadPrice == 0)
			continue;

		guild->addMoney(squadPrice);

		if (squadPrice > (*spentMoney))
		{
			OUTPOST_WRN("error while paying back squad A %u (%u dappers), spent money = %u", i, squadPrice, (*spentMoney));
			DEBUG_STOP;
			(*spentMoney) = 0;
		}
		else
		{
			(*spentMoney) -= squadPrice;
		}
		(*expenseLimit) += squadPrice;
	}

	// pay back alive squads B
	for (uint i = 0; i < _CurrentSquadsB.size(); ++i)
	{
		const COutpostSquadPtr & squad = _CurrentSquadsB[i];
		if (squad.isNull() || squad->isDead())
			continue;

		nlassert(i < _CurrentSquadsBQueue.size());
		uint32 squadPrice = _CurrentSquadsBQueue[i].getSquadDescriptor().getStaticForm()->BuyPrice;
		if (squadPrice == 0)
			continue;

		guild->addMoney(squadPrice);

		if (squadPrice > (*spentMoney))
		{
			OUTPOST_WRN("error while paying back squad B %u (%u dappers), spent money = %u", i, squadPrice, (*spentMoney));
			DEBUG_STOP;
			(*spentMoney) = 0;
		}
		else
		{
			(*spentMoney) -= squadPrice;
		}
		(*expenseLimit) += squadPrice;
	}
}

//----------------------------------------------------------------------------
void COutpost::actionSpawnSquadsA(uint32 squadCount, OUTPOSTENUMS::TPVPSide side)
{
	nlassert(squadCount<=_CurrentSquadsA.size() && squadCount<=_CurrentSquadsAQueue.size());
	const bool isTribeSquad = (side == OUTPOSTENUMS::OutpostOwner && !isBelongingToAGuild());
	for (size_t i=0; i<squadCount; ++i)
	{
		nlassert(_CurrentSquadsA[i].isNull());
		// set a random spawn zone each time a tribe squad spawns
		if (isTribeSquad)
			_CurrentSquadsAQueue[i].setSpawnZone( getRandomSpawnZone() );
		createSquad(_CurrentSquadsA[i], _CurrentSquadsAQueue[i], NULL, NULL, side);
		++_FightData._SpawnedSquadsA;
	}
}

//----------------------------------------------------------------------------
void COutpost::actionSpawnSquadsB(uint32 squadIndex, OUTPOSTENUMS::TPVPSide side)
{
	nlassert(squadIndex<_CurrentSquadsB.size() && squadIndex<_CurrentSquadsBQueue.size());
	nlassert(_CurrentSquadsB[squadIndex].isNull());
	const bool isTribeSquad = (side == OUTPOSTENUMS::OutpostOwner && !isBelongingToAGuild());
	// set a random spawn zone each time a tribe squad spawns
	if (isTribeSquad)
		_CurrentSquadsBQueue[squadIndex].setSpawnZone( getRandomSpawnZone() );
	createSquad(_CurrentSquadsB[squadIndex], _CurrentSquadsBQueue[squadIndex], NULL, NULL, side);
	++_FightData._SpawnedSquadsB;
}

//----------------------------------------------------------------------------
void COutpost::actionDespawnAllSquads()
{
	std::vector<COutpostSquadPtr>::iterator it, itEnd;
	for (it=_CurrentSquadsA.begin(), itEnd=_CurrentSquadsA.end(); it!=itEnd; ++it)
	{
		COutpostSquadPtr& ptr = *it;
		if (!ptr.isNull())
		{
			ptr = NULL;
			--_FightData._SpawnedSquadsA;
		}
	}
	for (it=_CurrentSquadsB.begin(), itEnd=_CurrentSquadsB.end(); it!=itEnd; ++it)
	{
		COutpostSquadPtr& ptr = *it;
		if (!ptr.isNull())
		{
			ptr = NULL;
			--_FightData._SpawnedSquadsB;
		}
	}

	askGuildDBUpdate(COutpostGuildDBUpdater::SQUAD_SPAWNED);
}

//----------------------------------------------------------------------------
void COutpost::actionChangeOwner()
{
	EGSPD::TGuildId newOwner = _AttackerGuildId;
	EGSPD::TGuildId newAttacker = _OwnerGuildId;
	
	setOwnerGuild(newOwner);
	setAttackerGuild(newAttacker);
	
	std::swap(_NextAttackSquadsA, _NextDefenseSquadsA);
	std::swap(_NextAttackSquadsB, _NextDefenseSquadsB);
	std::swap(_OwnerExpenseLimit, _AttackerExpenseLimit);
}

//----------------------------------------------------------------------------
void COutpost::actionCancelOutpostChallenge()
{
	// :TODO: Implement this
	OUTPOST_WRN("Outpost challenge cancelation not yet implemented.");
}

//----------------------------------------------------------------------------
void COutpost::actionResetOwnerExpenseLimit()
{
	_OwnerExpenseLimit = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionResetAttackerExpenseLimit()
{
	_AttackerExpenseLimit = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionResetMoneySpent()
{
	_MoneySpentByOwner = 0;
	_MoneySpentByAttacker = 0;
}

//----------------------------------------------------------------------------
void COutpost::actionPayBackMoneySpent()
{
	if (_MoneySpentByOwner == 0 && _MoneySpentByAttacker == 0)
		return;

	CGuild * ownerGuild = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
	if (ownerGuild != NULL)
	{
		ownerGuild->addMoney(_MoneySpentByOwner);
		OUTPOST_INF("%u dappers have been paid back to the owner guild '%s' (id=%u) of the outpost %s",
			_MoneySpentByOwner,
			ownerGuild->getName().toUtf8().c_str(),
			ownerGuild->getId(),
			_Sheet.toString().c_str()
			);
	}

	CGuild * attackerGuild = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
	if (attackerGuild != NULL)
	{
		attackerGuild->addMoney(_MoneySpentByAttacker);
		OUTPOST_INF("%u dappers have been paid back to the attacker guild '%s' (id=%u) of the outpost %s",
			_MoneySpentByAttacker,
			attackerGuild->getName().toUtf8().c_str(),
			attackerGuild->getId(),
			_Sheet.toString().c_str()
			);
	}

	_OwnerExpenseLimit += _MoneySpentByOwner;
	_MoneySpentByOwner = 0;
	_MoneySpentByAttacker = 0;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeRoundCount() const
{
	return std::min(OutpostFightRoundCount.get(), OUTPOSTENUMS::OUTPOST_MAX_SQUAD_SPAWNED);
}

//----------------------------------------------------------------------------
uint32 COutpost::computeRoundTime() const
{
	return OutpostFightRoundTime.get();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeFightTime() const
{
	return computeRoundTime() * computeRoundCount();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeLevelDecrementTime() const
{
	return OutpostLevelDecrementTime.get();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeSpawnDelay(uint32 roundLevel) const
{
	return (computeRoundTime()-60) / (computeSquadCountB(roundLevel)+1);
}

//----------------------------------------------------------------------------
uint32 COutpost::computeSquadCountA(uint32 roundLevel) const
{
	return (uint32)ceil((float)(roundLevel+1)/2.f);
}

//----------------------------------------------------------------------------
uint32 COutpost::computeSquadCountB(uint32 roundLevel) const
{
	return (uint32)floor((float)(roundLevel+1)/2.f);
}

//----------------------------------------------------------------------------
uint32 COutpost::computeChallengeTime() const
{
	return _ChallengeTime - _RealChallengeTime + 24*hours;
}
/*
//----------------------------------------------------------------------------
uint32 COutpost::computeChallengeEndTime() const
{
	return _ChallengeTime - (_ChallengeTime%hours) + 25*hours;
}
*/
//----------------------------------------------------------------------------
uint32 COutpost::s_computeAttackHour(uint32 challengeHour, uint32 attackHour)
{
	if (attackHour == (challengeHour+23)%24)
		return (challengeHour+22)%24;
	else
		return attackHour;
}
/*
//----------------------------------------------------------------------------
uint32 COutpost::computeAttackHour() const
{
	if (_AttackHour == (_ChallengeHour+23)%24)
		return (_ChallengeHour+22)%24;
	else
		return _AttackHour;
}
*/
//----------------------------------------------------------------------------
uint32 COutpost::computeDefenseHour() const
{
	if (_DefenseHour == (_ChallengeHour+23)%24)
		return (_ChallengeHour+22)%24;
	else
		return _DefenseHour;
}

//----------------------------------------------------------------------------
uint32 COutpost::s_computeTimeBeforeAttack(uint32 challengeHour, uint32 attackHour)
{
	return ((24+s_computeAttackHour(challengeHour, attackHour)-challengeHour)%24)*hours;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeBeforeAttack() const
{
	return s_computeTimeBeforeAttack(_ChallengeHour, _AttackHour);
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeAfterAttack() const
{
	return 24*hours - computeFightTime() - computeTimeBeforeAttack();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeBeforeDefense() const
{
	return ((24+computeDefenseHour()-_ChallengeHour)%24)*hours;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeAfterDefense() const
{
	return 24*hours - computeFightTime() - computeTimeBeforeDefense();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeMinimumTimeToNextFight() const
{
	// Returns 0 if undefined
	switch (_State) {
	case OUTPOSTENUMS::Peace:			return 0;
	case OUTPOSTENUMS::WarDeclaration:	return _Timer0EndTime - CTime::getSecondsSince1970();
	case OUTPOSTENUMS::AttackBefore:	return _Timer0EndTime - CTime::getSecondsSince1970();
	case OUTPOSTENUMS::AttackRound:		return 0;
	case OUTPOSTENUMS::AttackAfter:		return _Timer0EndTime - NLMISC::CTime::getSecondsSince1970();
	case OUTPOSTENUMS::DefenseBefore:	return _Timer0EndTime - CTime::getSecondsSince1970();
	case OUTPOSTENUMS::DefenseRound:	return 0;
	case OUTPOSTENUMS::DefenseAfter:	return 0;
	}
	return 0;
}

//----------------------------------------------------------------------------
uint8 COutpost::computeStatusForClient() const
{
	// Yoyo: there is a little difference on the client: For AttackAfter, Use DefenseBefore if this one will happens
	if(_State==OUTPOSTENUMS::AttackAfter)
	{
		// only if there will be a defense
		if (isBelongingToAGuild() && _FightData._MaxAttackLevel > _CurrentOutpostLevel)
			return OUTPOSTENUMS::DefenseBefore;
	}

	// common case
	return _State;
}

//----------------------------------------------------------------------------
void COutpost::updateTimersForClient()
{
	// update state end date
	{
		uint32 stateEndDate = 0;
		switch (_State)
		{
		case OUTPOSTENUMS::AttackRound:
		case OUTPOSTENUMS::DefenseRound:
			stateEndDate = (_Timer0EndTime - CTime::getSecondsSince1970() + (computeRoundCount()-_FightData._CurrentCombatRound-1)*computeRoundTime())*10 + CTickEventHandler::getGameCycle();
			break;
		case OUTPOSTENUMS::WarDeclaration:
		case OUTPOSTENUMS::AttackBefore:
		case OUTPOSTENUMS::DefenseBefore:
		case OUTPOSTENUMS::DefenseAfter:
			stateEndDate = (_Timer0EndTime - CTime::getSecondsSince1970())*10 + CTickEventHandler::getGameCycle();
			break;
		case OUTPOSTENUMS::AttackAfter: // We merge AttackAfter and DefenseBefore if there is a defense period
			if (isBelongingToAGuild() && _FightData._MaxAttackLevel > _CurrentOutpostLevel)
				stateEndDate = (_Timer0EndTime - CTime::getSecondsSince1970() + computeTimeBeforeDefense())*10 + CTickEventHandler::getGameCycle();
			else
				stateEndDate = (_Timer0EndTime - CTime::getSecondsSince1970())*10 + CTickEventHandler::getGameCycle();
			break;
		}
		_StateEndDateTickForClient = stateEndDate;
	}

	// update round end date
	{
		uint32 roundEndDate = 0;
		switch (_State)
		{
		case OUTPOSTENUMS::AttackRound:
		case OUTPOSTENUMS::DefenseRound:
			 roundEndDate = (_Timer0EndTime - CTime::getSecondsSince1970())*10 + CTickEventHandler::getGameCycle();
			 break;
		}
		_RoundEndDateTickForClient = roundEndDate;
	}

	// set last update
	_LastUpdateOfTimersForClient = CTime::getSecondsSince1970();
}

//----------------------------------------------------------------------------
uint32 COutpost::computeStateEndDateTickForClient() const
{
	return _StateEndDateTickForClient;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeRoundEndDateTickForClient() const
{
	return _RoundEndDateTickForClient;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeRangeAttForClient() const
{
	uint32 time = _Timer0EndTime;
	switch (_State) {
	case OUTPOSTENUMS::Peace:
		return 0;
	case OUTPOSTENUMS::WarDeclaration:
		time += computeTimeBeforeAttack();
	case OUTPOSTENUMS::AttackBefore:
		return time;
	case OUTPOSTENUMS::AttackRound:
	case OUTPOSTENUMS::AttackAfter:
	case OUTPOSTENUMS::DefenseBefore:
	case OUTPOSTENUMS::DefenseRound:
	case OUTPOSTENUMS::DefenseAfter:
		return 0;
	}
	return 0;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeRangeDefForClient() const
{
	uint32 time = _Timer0EndTime;
	switch (_State) {
	case OUTPOSTENUMS::Peace:
		return 0;
	case OUTPOSTENUMS::WarDeclaration:
		time += computeTimeBeforeAttack();
	case OUTPOSTENUMS::AttackBefore:
		time += computeRoundTime() * computeRoundCount();
	case OUTPOSTENUMS::AttackRound:
		time += computeTimeAfterAttack();
	case OUTPOSTENUMS::AttackAfter:
		time += computeTimeBeforeDefense();
	case OUTPOSTENUMS::DefenseBefore:
		return time;
	case OUTPOSTENUMS::DefenseRound:
	case OUTPOSTENUMS::DefenseAfter:
		return 0;
	}
	return 0;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTimeRangeLengthForClient() const
{
	return computeRoundCount()*computeRoundTime()/minutes;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeTribeOutpostLevel() const
{
	return _Form->MinimumTribeRoundLevel;
}

//----------------------------------------------------------------------------
uint32 COutpost::computeGuildMinimumOutpostLevel() const
{
	return _Form->MinimumGuildRoundLevel;
}

//----------------------------------------------------------------------------
uint32 COutpost::s_computeEstimatedAttackTimeForClient(uint32 hour)
{
	uint32 realChallengeTime = CTime::getSecondsSince1970();
	uint32 challengeTime = (realChallengeTime/hours + 1)*hours; // Aligned on next hour
	uint32 challengeHour = (challengeTime%days)/hours; // Aligned on next hour
	return challengeTime + 24*hours + s_computeTimeBeforeAttack(challengeHour, hour); // Aligned on next hour
}

//----------------------------------------------------------------------------
void COutpost::timeSetAttackHour(uint32 val)
{
	_AttackHour = val;
}

//----------------------------------------------------------------------------
void COutpost::timeSetDefenseHour(uint32 val)
{
	_DefenseHour = val;
	askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
}

//----------------------------------------------------------------------------
bool COutpost::submitEditingAccess(OUTPOSTENUMS::TPVPSide side, NLMISC::CEntityId playerId, TEditingAccessType accessType)
{
	nlassert(accessType >= 0 && accessType < NbEditingAccessType);

	CEditingAccess * lastEditingAccess = NULL;
	if (side == OUTPOSTENUMS::OutpostOwner)
	{
		lastEditingAccess = &_LastOwnerEditingAccess[accessType];
	}
	else if (side == OUTPOSTENUMS::OutpostAttacker)
	{
		lastEditingAccess = &_LastAttackerEditingAccess[accessType];
	}
	else
	{
		nlstop;
		return false;
	}

	const TGameCycle currentTick = CTickEventHandler::getGameCycle();

	// not ok if another player has edited recently
	if (lastEditingAccess->PlayerId != CEntityId::Unknown && playerId != lastEditingAccess->PlayerId)
	{
		if (lastEditingAccess->Tick + OutpostEditingConcurrencyCheckDelay.get() > currentTick)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
			params[0].setEId(lastEditingAccess->PlayerId);
			CCharacter::sendDynamicSystemMessage(playerId, "OUTPOST_EDITING_CONCURRENCY_ISSUE", params);
			return false;
		}
	}

	// ok update the last access
	lastEditingAccess->PlayerId = playerId;
	lastEditingAccess->Tick = currentTick;

	return true;
}

//----------------------------------------------------------------------------
std::string COutpost::getErrorString(TChallengeOutpostErrors error)
{
	switch (error)
	{
	case NoError:				return "OUTPOST_ERROR_NONE";
	case InvalidUser:			return "OUTPOST_ERROR_INVALID_USER";
	case InvalidOutpost:		return "OUTPOST_ERROR_INVALID_OUTPOST";
	case NoGuildModule:			return "OUTPOST_ERROR_NO_GUILD_MODULE";
	case BadGuildGrade:			return "OUTPOST_ERROR_BAD_GUILD_GRADE";
	case BadGuildMemberLevel:	return "OUTPOST_ERROR_BAD_MEMBER_LEVEL";
	case NotEnoughMoney:		return "OUTPOST_ERROR_NOT_ENOUGH_MONEY";
	case AlreadyAttacked:		return "OUTPOST_ERROR_ALREADY_ATTACKED";
	case AlreadyOwned:			return "OUTPOST_ERROR_ALREADY_OWNED";
	case TimePeriodEstimationChanged:	return "OUTPOST_ERROR_TIME_PERIOD_ESTIMATION_CHANGED";
	case TooManyGuildOutposts:	return "OUTPOST_ERROR_TOO_MANY_GUILD_OUTPOSTS";
	case UnknownError:			return "OUTPOST_ERROR_UNKNOWN";
	}
	return "OUTPOST_ERROR_UNKNOWN";
}

//----------------------------------------------------------------------------
std::string COutpost::getBroadcastString(TBroadcastMessage message) const
{
	switch (message)
	{
	case RoundNearEnd:		return "ROUND_NEAR_END";
	case RoundLost:			return "ROUND_LOST";
	case RoundWon:			return "ROUND_WON";
	case LastRoundLost:		return "LAST_ROUND_LOST";
	case LastRoundWon:		return "LAST_ROUND_WON";
	case AttackFailed:		return "ATTACK_FAILED";
	case AttackSucceeded:	return "ATTACK_SUCCEEDED";
	case DefenseFailed:		return "DEFENSE_FAILED";
	case DefenseSucceeded:	return "DEFENSE_SUCEEDED";
	case AttackRounds:		return "ATTACK_ROUNDS";
	case DefenseRounds:		return "DEFENSE_ROUNDS";
	case WarDeclared:		return "WAR_DECLARED";
	}
	return "UNKNOWN";
}

//----------------------------------------------------------------------------
void COutpost::broadcastMessageMsg(vector<TDataSetRow> const& audience, string const& message, TVectorParamCheck const& params) const
{
	vector<TDataSetRow>::const_iterator it, itEnd;
	for (it=audience.begin(), itEnd=audience.end(); it!=itEnd; ++it)
		PHRASE_UTILITIES::sendDynamicSystemMessage(*it, message, params);
}

void COutpost::broadcastMessagePopup(vector<TDataSetRow> const& audience, string const& message, TVectorParamCheck const& params, TVectorParamCheck const& paramsTitle) const
{
	string title = message + "_TITLE";
	vector<TDataSetRow>::const_iterator it, itEnd;
	for (it=audience.begin(), itEnd=audience.end(); it!=itEnd; ++it)
	{
		uint32 msgId = STRING_MANAGER::sendStringToClient(*it, message, params);
		uint32 titleId = STRING_MANAGER::sendStringToClient(*it, title, paramsTitle);
		CEntityId eid = TheDataset.getEntityId(*it);
		PlayerManager.sendImpulseToClient(eid, "USER:POPUP", titleId, msgId);
	}
}

void COutpost::broadcastMessage(TBroadcastAudience audience, TBroadcastMessage message) const
{
	vector<TDataSetRow> dest;
	CGuild* guild = NULL;
	set<TDataSetRow>::const_iterator itUser, itUserEnd;
	uint32 side = ~0;
	switch (audience)
	{
	case OwnerGuild:
		side = 0;
		guild = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
		if (guild != NULL)
		{
			std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it;
			for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
			{
				CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
				CGuildMemberModule * module = NULL;
				// add member to recipients if he is online
				if (member != NULL && member->getReferencingModule(module))
					dest.push_back(TheDataset.getDataSetRow(member->getIngameEId()));
			}
		}
		break;
	case AttackerGuild:
		side = 1;
		guild = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
		if (guild != NULL)
		{
			std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it;
			for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
			{
				CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
				CGuildMemberModule * module = NULL;
				// add member to recipients if he is online
				if (member != NULL && member->getReferencingModule(module))
					dest.push_back(TheDataset.getDataSetRow(member->getIngameEId()));
			}
		}
		break;
	case OwnerFighters:
		side = 0;
		for (itUser=_Users.begin(), itUserEnd=_Users.end(); itUser!=itUserEnd; ++itUser)
		{
			if (PlayerManager.getChar(*itUser) && PlayerManager.getChar(*itUser)->getOutpostSide()==OUTPOSTENUMS::OutpostOwner)
				dest.push_back(*itUser);
		}
		break;
	case AttackerFighters:
		side = 1;
		for (itUser=_Users.begin(), itUserEnd=_Users.end(); itUser!=itUserEnd; ++itUser)
		{
			if (PlayerManager.getChar(*itUser) && PlayerManager.getChar(*itUser)->getOutpostSide()==OUTPOSTENUMS::OutpostAttacker)
				dest.push_back(*itUser);
		}
		break;
	}
	if (dest.empty())
		return;
	switch (message)
	{
	case AttackSucceeded:
	{
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::outpost, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].SheetId = getSheet();
		params[1].Int = side;
		params[2].Int = isBelongingToAGuild()?1:0;
		broadcastMessagePopup(dest, "OUTPOST_BCP_"+getBroadcastString(message), params, params);
		break;
	}
	case AttackFailed:
	case DefenseFailed:
	case DefenseSucceeded:
	case AttackRounds:
	case DefenseRounds:
	case WarDeclared:
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::outpost, STRING_MANAGER::integer);
		params[0].SheetId = getSheet();
		params[1].Int = side;
		broadcastMessagePopup(dest, "OUTPOST_BCP_"+getBroadcastString(message), params, params);
		break;
	}
	default:
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::outpost, STRING_MANAGER::integer);
		params[0].SheetId = getSheet();
		params[1].Int = side;
		broadcastMessageMsg(dest, "OUTPOST_BCM_"+getBroadcastString(message), params);
		break;
	}
	}
}

//----------------------------------------------------------------------------
void COutpost::dumpOutpost(NLMISC::CLog & log) const
{
	string ownerName;
	string attackerName;

	if (isBelongingToAGuild())
	{
		CGuild * ownerGuild = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
		if (ownerGuild != NULL)
			ownerName = ownerGuild->getName().toUtf8();
		else
			ownerName = "unknown guild";
	}
	else
	{
		ownerName = "tribe";
	}

	if (_AttackerGuildId != 0)
	{
		CGuild * attackerGuild = CGuildManager::getInstance()->getGuildFromId(_AttackerGuildId);
		if (attackerGuild != NULL)
			attackerName = attackerGuild->getName().toUtf8();
		else
			attackerName = "unknown guild";
	}

	log.displayNL("__BEGIN_OUTPOST_DUMP__");

	// static data (from primitives and sheets)
	log.displayNL("_STATIC_DATA_");
	log.displayNL("Alias: %s",							CPrimitivesParser::aliasToString(_Alias).c_str());
	log.displayNL("Name: '%s'",							_Name.c_str());
	log.displayNL("Sheet: '%s'",						_Sheet.toString().c_str());
	log.displayNL("Continent: '%s'",					CONTINENT::toString(_Continent).c_str());
	log.displayNL("PVPType: '%s'",						OUTPOSTENUMS::toString(_PVPType).c_str());
	if (_Form != NULL)
	{
		log.displayNL("ChallengeCost: %u",				_Form->ChallengeCost);
		log.displayNL("MaxSpawnSquadCount: %u",			_Form->MaxSpawnSquadCount);
		log.displayNL("Level: %u",						_Form->Level);
		log.displayNL("MinimumTribeRoundLevel: %u",		_Form->MinimumTribeRoundLevel);
		log.displayNL("MinimumGuildRoundLevel: %u",		_Form->MinimumGuildRoundLevel);
	}
	else
	{
		log.displayNL("ERROR: _Form is NULL!");
	}

	log.displayNL("SpawnZones (size=%u):", _SpawnZones.size());
	for (uint i = 0; i < _SpawnZones.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _SpawnZones[i].toString().c_str());
	}

	log.displayNL("TribeSquadsA (size=%u):", _TribeSquadsA.size());
	for (uint i = 0; i < _TribeSquadsA.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _TribeSquadsA[i].toString().c_str());
	}
	log.displayNL("TribeSquadsB (size=%u):", _TribeSquadsB.size());

	for (uint i = 0; i < _TribeSquadsB.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _TribeSquadsB[i].toString().c_str());
	}

	log.displayNL("DefaultSquadsA (size=%u):", _DefaultSquads.size());
	for (uint i = 0; i < _DefaultSquads.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _DefaultSquads[i].toString().c_str());
	}

	log.displayNL("BuyableSquads (size=%u):", _BuyableSquads.size());
	for (uint i = 0; i < _BuyableSquads.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _BuyableSquads[i].toString().c_str());
	}

	// dynamic data
	log.displayNL("_DYNAMIC_DATA_");
	log.displayNL("State: '%s'",						OUTPOSTENUMS::toString(_State).c_str());
	log.displayNL("NextState: '%s'",					OUTPOSTENUMS::toString(_NextState).c_str());
	log.displayNL("Owner: id=%u '%s'",					_OwnerGuildId, ownerName.c_str());
	log.displayNL("Attacker: id=%u '%s'",				_AttackerGuildId, attackerName.c_str());
	log.displayNL("Timer0EndTime: %u",					_Timer0EndTime);
	log.displayNL("Timer1EndTime: %u",					_Timer1EndTime);
	log.displayNL("Timer2EndTime: %u",					_Timer2EndTime);
	log.displayNL("AISUp: %s",							_AISUp ? "true":"false");
	log.displayNL("CurrentOutpostLevel: %u",			_CurrentOutpostLevel);
	log.displayNL("OwnerExpenseLimit: %u",				_OwnerExpenseLimit);
	log.displayNL("AttackerExpenseLimit: %u",			_AttackerExpenseLimit);
	log.displayNL("MoneySpentByOwner: %u",				_MoneySpentByOwner);
	log.displayNL("MoneySpentByAttacker: %u",			_MoneySpentByAttacker);
	log.displayNL("CrashHappened: %s",					_CrashHappened ? "true":"false");
	log.displayNL("RealChallengeTime: %u",				_RealChallengeTime);
	log.displayNL("ChallengeTime: %u",					_ChallengeTime);
	log.displayNL("ChallengeHour: %u",					_ChallengeHour);
	log.displayNL("AttackHour: %u",						_AttackHour);
	log.displayNL("DefenseHour: %u",					_DefenseHour);

	log.displayNL("FightData:");
	log.displayNL("    SpawnedSquadsA: %u",				_FightData._SpawnedSquadsA);
	log.displayNL("    SpawnedSquadsB: %u",				_FightData._SpawnedSquadsB);
	log.displayNL("    KilledSquads: %u",				_FightData._KilledSquads);
	log.displayNL("    CurrentCombatLevel: %u",			_FightData._CurrentCombatLevel);
	log.displayNL("    CurrentCombatRound: %u",			_FightData._CurrentCombatRound);
	log.displayNL("    MaxAttackLevel: %u",				_FightData._MaxAttackLevel);
	log.displayNL("    MaxDefenseLevel: %u",			_FightData._MaxDefenseLevel);

	log.displayNL("Buildings (size=%u):", _Buildings.size());
	for (uint i = 0; i < _Buildings.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _Buildings[i].toString().c_str());
	}

	log.display("BanishedGuilds for attack (size=%u): ", _AttackBanishedGuilds.size());
	for (std::set<uint32>::const_iterator it = _AttackBanishedGuilds.begin(); it != _AttackBanishedGuilds.end(); ++it)
	{
		if (it != _AttackBanishedGuilds.begin())
			log.display(", ");
		log.display("%u", (*it));
	}
	log.displayNL("");

	log.display("BanishedGuilds for defense (size=%u): ", _DefenseBanishedGuilds.size());
	for (std::set<uint32>::const_iterator it = _DefenseBanishedGuilds.begin(); it != _DefenseBanishedGuilds.end(); ++it)
	{
		if (it != _DefenseBanishedGuilds.begin())
			log.display(", ");
		log.display("%u", (*it));
	}
	log.displayNL("");

	log.display("BanishedPlayers for attack(size=%u): ", _AttackBanishedPlayers.size());
	for (std::set<CEntityId>::const_iterator it = _AttackBanishedPlayers.begin(); it != _AttackBanishedPlayers.end(); ++it)
	{
		if (it != _AttackBanishedPlayers.begin())
			log.display(", ");
		log.display("%s", (*it).toString().c_str());
	}
	log.displayNL("");

	log.display("BanishedPlayers for defense(size=%u): ", _DefenseBanishedPlayers.size());
	for (std::set<CEntityId>::const_iterator it = _DefenseBanishedPlayers.begin(); it != _DefenseBanishedPlayers.end(); ++it)
	{
		if (it != _DefenseBanishedPlayers.begin())
			log.display(", ");
		log.display("%s", (*it).toString().c_str());
	}
	log.displayNL("");

	log.displayNL("NextAttackSquadsA (size=%u):", _NextAttackSquadsA.size());
	for (uint i = 0; i < _NextAttackSquadsA.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _NextAttackSquadsA[i].toString().c_str());
	}

	log.displayNL("NextAttackSquadsB (size=%u):", _NextAttackSquadsB.size());
	for (uint i = 0; i < _NextAttackSquadsB.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _NextAttackSquadsB[i].toString().c_str());
	}

	log.displayNL("NextDefenseSquadsA (size=%u):", _NextDefenseSquadsA.size());
	for (uint i = 0; i < _NextDefenseSquadsA.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _NextDefenseSquadsA[i].toString().c_str());
	}

	log.displayNL("NextDefenseSquadsB (size=%u):", _NextDefenseSquadsB.size());
	for (uint i = 0; i < _NextDefenseSquadsB.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _NextDefenseSquadsB[i].toString().c_str());
	}

	log.displayNL("CurrentSquadsAQueue (size=%u):", _CurrentSquadsAQueue.size());
	for (uint i = 0; i < _CurrentSquadsAQueue.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _CurrentSquadsAQueue[i].toString().c_str());
	}

	log.displayNL("CurrentSquadsBQueue (size=%u):", _CurrentSquadsBQueue.size());
	for (uint i = 0; i < _CurrentSquadsBQueue.size(); ++i)
	{
		log.displayNL("    %u: [%s]", i, _CurrentSquadsBQueue[i].toString().c_str());
	}

	log.displayNL("CurrentSquadsA (size=%u):", _CurrentSquadsA.size());
	for (uint i = 0; i < _CurrentSquadsA.size(); ++i)
	{
		if (_CurrentSquadsA[i] != NULL)
			log.displayNL("    %u: [%s]", i, _CurrentSquadsA[i]->toString().c_str());
		else
			log.displayNL("    %u: NULL", i);
	}

	log.displayNL("CurrentSquadsB (size=%u):", _CurrentSquadsB.size());
	for (uint i = 0; i < _CurrentSquadsB.size(); ++i)
	{
		if (_CurrentSquadsB[i] != NULL)
			log.displayNL("    %u: [%s]", i, _CurrentSquadsB[i]->toString().c_str());
		else
			log.displayNL("    %u: NULL", i);
	}

	const uint lastOwnerEditingAccessSize = sizeof(_LastOwnerEditingAccess) / sizeof(_LastOwnerEditingAccess[0]);
	log.displayNL("LastOwnerEditingAccess (size=%u):", lastOwnerEditingAccessSize);
	for (uint i = 0; i < lastOwnerEditingAccessSize; i++)
	{
		log.displayNL("    %u: [Player: %s, Tick: %u]", i, _LastOwnerEditingAccess[i].PlayerId.toString().c_str(), _LastOwnerEditingAccess[i].Tick);
	}

	const uint lastAttackerEditingAccessSize = sizeof(_LastAttackerEditingAccess) / sizeof(_LastAttackerEditingAccess[0]);
	log.displayNL("LastAttackerEditingAccess (size=%u):", lastAttackerEditingAccessSize);
	for (uint i = 0; i < lastAttackerEditingAccessSize; i++)
	{
		log.displayNL("    %u: [Player: %s, Tick: %u]", i, _LastAttackerEditingAccess[i].PlayerId.toString().c_str(), _LastAttackerEditingAccess[i].Tick);
	}

	log.displayNL("__END_OUTPOST_DUMP__");
}

//----------------------------------------------------------------------------
std::string COutpost::toString() const
{
	string ownerName;
	if (isBelongingToAGuild())
	{
		CGuild * ownerGuild = CGuildManager::getInstance()->getGuildFromId(_OwnerGuildId);
		if (ownerGuild != NULL)
			ownerName = "'" + ownerGuild->getName().toUtf8() + "'";
		else
			ownerName = NLMISC::toString("unknown guild (id=%u)", _OwnerGuildId);
	}
	else
	{
		ownerName = "tribe";
	}

	string desc;
	desc = NLMISC::toString("Alias: %s, Name: '%s', Sheet: '%s', State: '%s', Level: %d, Owner: %s",
		CPrimitivesParser::aliasToString( _Alias ).c_str(),
		_Name.c_str(),
		_Sheet.toString().c_str(),
		OUTPOSTENUMS::toString( _State ).c_str(),
		_CurrentOutpostLevel,
		ownerName.c_str()
		);

	return desc;
}

//----------------------------------------------------------------------------

#define PERSISTENT_MACROS_AUTO_UNDEF

//----------------------------------------------------------------------------
// Persistent data for COutpost
//----------------------------------------------------------------------------

#define PERSISTENT_CLASS COutpost

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COutpostApply);\
	uint32 version = ~0u;\
	preLoad();\

#define PERSISTENT_POST_APPLY\
	if (version != ~0u)\
		COutpostVersionAdapter::getInstance()->adaptOutpostFromVersion(*this, version);\
	postLoad();\

//#define PERSISTENT_POST_STORE\
//	postStore();

#define PERSISTENT_DATA\
	PROP2(VERSION, uint32, COutpostVersionAdapter::getInstance()->currentVersionNumber(), version = val)\
	\
	/*PROP2(_PVPType, string, OUTPOSTENUMS::toString( _PVPType );, _PVPType = OUTPOSTENUMS::toPVPType( val ); )*/\
	PROP2(_State, string, OUTPOSTENUMS::toString( _State );, _State = OUTPOSTENUMS::toOutpostState( val ); )\
	PROP2(_OwnerGuildId, uint32, _OwnerGuildId & ((1<<20)-1), _OwnerGuildId = val != 0 ? ((val & ((1<<20)-1)) | (IService::getInstance()->getShardId()<<20)) : 0)\
	PROP2(_AttackerGuildId, uint32, _AttackerGuildId & ((1<<20)-1), _AttackerGuildId = val != 0 ? ((val & ((1<<20)-1)) | (IService::getInstance()->getShardId()<<20)) : 0)\
	STRUCT_VECT(_NextAttackSquadsA)\
	STRUCT_VECT(_NextAttackSquadsB)\
	STRUCT_VECT(_NextDefenseSquadsA)\
	STRUCT_VECT(_NextDefenseSquadsB)\
	STRUCT_VECT(_CurrentSquadsAQueue)\
	STRUCT_VECT(_CurrentSquadsBQueue)\
	/*STRUCT_SMRTPTR_VECT(COutpostSquad,_CurrentSquadsA)*/\
	/*STRUCT_SMRTPTR_VECT(COutpostSquad,_CurrentSquadsB)*/\
	PROP(bool, _CrashHappened)\
	PROP(uint32, _Timer0EndTime)\
	PROP(uint32, _Timer1EndTime)\
	PROP(uint32, _Timer2EndTime)\
	PROP(bool, _AISUp)\
	PROP(uint32, _CurrentOutpostLevel)\
	PROP(uint32, _RealChallengeTime)\
	PROP(uint32, _ChallengeTime)\
	PROP(uint32, _ChallengeHour)\
	PROP(uint32, _AttackHour)\
	PROP(uint32, _DefenseHour)\
	PROP(uint32, _OwnerExpenseLimit)\
	PROP(uint32, _AttackerExpenseLimit)\
	PROP(uint32, _MoneySpentByOwner)\
	PROP(uint32, _MoneySpentByAttacker)\
	NPROP(_MaxAttackLevel, uint32, _FightData._MaxAttackLevel)\
	NPROP(_MaxDefenseLevel, uint32, _FightData._MaxDefenseLevel)\
	\
	LSTRUCT_MAP2(_Buildings, TAIAlias,\
		VECT_LOGIC(_Buildings),\
		_Buildings[i].getAlias(),\
		_Buildings[i].store(pdr),\
		{\
			std::vector< COutpostBuilding >::iterator it;\
			std::vector< COutpostBuilding >::iterator itEnd=_Buildings.end();\
			for (it=_Buildings.begin(); it!=itEnd; ++it)\
				if (it->getAlias()==key)\
					break;\
			if (it!=itEnd)\
				it->apply(pdr);\
			else\
				( nlwarning("Saved building %s cannot be loaded.", CPrimitivesParser::aliasToString(key).c_str()) );\
		}\
	)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

