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
#include "mission_step_template.h"
#include "mission_manager/mission_template.h"
#include "mission_log.h"
#include "zone_manager.h"
#include "mission_manager/ai_alias_translator.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "mission_manager/mission_parser.h"
#include "nel/misc/algo.h"
#include "game_share/fame.h"
#include "game_share/pvp_clan.h"

using namespace std;
using namespace NLMISC;


/***************************************************************************************************
Steps linked with kill events
	-kill_fauna
	-kill_species ( from the GSPeople::EPeople enum )
	-kill_npc
	-kill_player
***************************************************************************************************/

// ----------------------------------------------------------------------------
class CMissionStepKillFauna : public IMissionStepTemplate
{
	struct CSubStep
	{
		string		Dynamic;
		CSheetId	Sheet;
		uint16		Quantity;
	};
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		_Place = 0xFFFF;
		bool ret = true;
		if (script.size() < 2 || script.size() > 3)
		{
			MISLOGSYNTAXERROR("<creature> <quantity> *[; <creature> <quantity>] [: <place>]");
			return false;
		}
		else
		{
			std::vector< std::string > subs;
			NLMISC::splitString( script[1],";", subs );
			_SubSteps.reserve( subs.size() );
			for ( uint i = 0; i < subs.size(); i++ )
			{
				CSubStep subStep;

				std::vector< std::string > args;
				CMissionParser::tokenizeString( subs[i]," \t", args );

				//// Dynamic Mission Args : #dynamic# <quantity>
				if ((args.size() == 2) && (args[0] == "#dynamic#"))
				{
					subStep.Dynamic = missionData.Name;
					subStep.Quantity = atoi(args[1].c_str());
				}
				////
				else
				{
					if ( args.size() != 2 )
					{
						MISLOGSYNTAXERROR("<creature> <quantity> *[; <creature> <quantity>] [: <place>]");
						return false;
					}
					missionData.ChatParams.push_back( make_pair(args[0],STRING_MANAGER::creature_model) );
				
					subStep.Dynamic = "";
					subStep.Sheet = CSheetId( args[0] + ".creature");
					if ( subStep.Sheet == CSheetId::Unknown )
					{
						ret = false;
						MISLOGERROR1("invalid sheet '%s'", args[0].c_str());
					}
					NLMISC::fromString(args[1], subStep.Quantity);
				}
				_SubSteps.push_back(subStep);
			}
			if ( script.size() == 3 )
			{
				string placeStr = CMissionParser::getNoBlankString( script[2] );
				CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
				missionData.ChatParams.push_back( make_pair(placeStr,STRING_MANAGER::place) );
				if ( !place )
				{
					ret = false;
					MISLOGERROR1("invalid place '%s'",script[2].c_str());
				}
				else
					_Place = place->getId();
			}
			return ret;
		}
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		string webAppUrl;
		bool ret = true;
		_User = PlayerManager.getChar(getEntityIdFromRow(userRow));

		if ( event.Type == CMissionEvent::Kill )
		{
			CMissionEventKill & eventSpe = (CMissionEventKill&)event;
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			CSheetId faunaSheet;

			//// Dynamic Mission Args
			if (_SubSteps[subStepIndex].Dynamic.empty()) {
				faunaSheet = _SubSteps[subStepIndex].Sheet;
			}
			else
			{
				vector<string> params = _User->getCustomMissionParams(_SubSteps[subStepIndex].Dynamic);
				if (params.size() < 2)
				{
					LOGMISSIONSTEPERROR("kill_fauna : invalid dynamic creature");
					return 0;
				}
				else
				{
					webAppUrl = params[0];
					faunaSheet = CSheetId(params[1]);
					
					if (params.size() > 2) {
						string placeStr = CMissionParser::getNoBlankString( params[2] );
						CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
						if ( !place )
						{
							ret = false;
							LOGMISSIONSTEPERROR("kill_fauna : invalid place "+params[2]);
						}
						else
							_Place = place->getId();
					}
				}
				////
			}

			if ( !c )
			{
				LOGMISSIONSTEPERROR("kill_fauna : invalid creature " + toString(event.TargetEntity.getIndex()));
			}
			else if ( faunaSheet == c->getType() )
			{
				if ( _Place != 0xFFFF )
				{
					float gooDistance;
					const CPlace * stable = NULL;
					std::vector<const CPlace *> places;
					const CRegion * region = NULL;
					const CContinent * continent = NULL;
					if ( !CZoneManager::getInstance().getPlace( c->getState().X, c->getState().Y, gooDistance, &stable, places, &region , &continent ) )
						return 0;

					if ( region && region->getId() == _Place )
					{
						if (!webAppUrl.empty())
							_User->validateDynamicMissionStep(webAppUrl);
						LOGMISSIONSTEPSUCCESS("kill_fauna");
						return 1;
					}
					
					for ( uint i = 0; i < places.size(); i++ )
					{
						if ( places[i] && places[i]->getId() == _Place )
						{
							if (!webAppUrl.empty())
								_User->validateDynamicMissionStep(webAppUrl);
							LOGMISSIONSTEPSUCCESS("kill_fauna");
							return 1;
						}
					}
					return 0;
				}
				else
				{
					if (!webAppUrl.empty())
						_User->validateDynamicMissionStep(webAppUrl);
					LOGMISSIONSTEPSUCCESS("kill_fauna");
					return 1;
				}
			}
		}

		return 0;
	}

	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
			ret[i] = _SubSteps[i].Quantity;
	}

	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepTextReact = "MIS_NEED_REACTIVATION";
		static const std::string stepText = "MIS_KILL_FAUNA_";
		static const std::string stepTextLoc = "MIS_KILL_FAUNA_LOC_";
		nlassert( _SubSteps.size() == subStepStates.size() );
		CSheetId faunaSheet;
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				if (_SubSteps[i].Dynamic.empty())
				{
					faunaSheet = _SubSteps[i].Sheet;
				}
				else
				{
					//// Dynamic Mission Args
					vector<string> params = _User->getCustomMissionParams(_SubSteps[i].Dynamic);
					if (params.size() < 2)
					{
						faunaSheet = CSheetId::Unknown;
					}
					else
					{
						faunaSheet = CSheetId(params[1]);
					}

					if ((_Place == 0xFFFF) && (params.size() > 2))
					{
						string placeStr = CMissionParser::getNoBlankString( params[2] );
						CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
						if ( !place )
						{
							MISLOG("sline:%u ERROR : kill_fauna : Invalid place %u", _SourceLine, _Place);
						}
						else
							_Place = place->getId();
					}
					////
				}
				
				nbSubSteps++;
				if (faunaSheet != CSheetId::Unknown)
				{
					retParams.push_back(STRING_MANAGER::TParam());
					retParams.back().Type = STRING_MANAGER::creature_model;
					retParams.back().SheetId = faunaSheet;
					
					retParams.push_back(STRING_MANAGER::TParam());
					retParams.back().Type = STRING_MANAGER::integer;
					retParams.back().Int = subStepStates[i];
				}
			}
		}
		if (faunaSheet != CSheetId::Unknown)
		{
			if ( _Place != 0xFFFF )
			{
				STRING_MANAGER::TParam param;
				param.Type = STRING_MANAGER::place;
				CPlace * place = CZoneManager::getInstance().getPlaceFromId(_Place);
				if ( !place )
				{
					MISLOG("sline:%u ERROR : kill_fauna : Invalid place %u", _SourceLine, _Place);
				}
				else
				{
					param.Identifier = place->getName();
					retParams.push_back(param);
				}
				textPtr = &stepTextLoc;
			}
			else
				textPtr = &stepText;
		}
		else
			textPtr = &stepTextReact;
	}

	std::vector< CSubStep > _SubSteps;
	uint16					_Place;

	MISSION_STEP_GETNEWPTR(CMissionStepKillFauna)
};
MISSION_REGISTER_STEP(CMissionStepKillFauna,"kill_fauna");


// ----------------------------------------------------------------------------
class CMissionStepKillRace : public IMissionStepTemplate
{
	struct CSubStep
	{
		EGSPD::CPeople::TPeople	Race;
		uint16				Quantity;
	};
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		_Place = 0xFFFF;
		bool ret = true;
		if ( script.size() < 2 || script.size() > 3)
		{
			MISLOGSYNTAXERROR("<race> <quantity> *[; <race> <quantity>] [:<place>]");
			return false;
		}
		else
		{
			std::vector< std::string > subs;
			NLMISC::splitString( script[1],";", subs );
			_SubSteps.reserve(subs.size());
			for ( uint i = 0; i < subs.size(); i++ )
			{
				std::vector< std::string > args;
				CMissionParser::tokenizeString( subs[i]," \t", args );
				if ( args.size() != 2 )
				{
					MISLOGSYNTAXERROR("<race> <quantity> *[; <race> <quantity>] [:<place>]");
					return false;
				}
				missionData.ChatParams.push_back( make_pair(args[0],STRING_MANAGER::race) );
				CSubStep subStep;
				subStep.Race = EGSPD::CPeople::fromString( args[0] );
				if ( subStep.Race == EGSPD::CPeople::EndPeople )
				{
					ret = false;
					MISLOGERROR1("invalid race '%s'", args[0].c_str());
				}
				NLMISC::fromString(args[1], subStep.Quantity);
				_SubSteps.push_back( subStep );
			}
			if ( script.size() == 3 )
			{
				string placeStr = CMissionParser::getNoBlankString( script[2] );
				missionData.ChatParams.push_back( make_pair(placeStr,STRING_MANAGER::place) );
				CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
				if ( !place )
				{
					ret = false;
					MISLOGERROR1("invalid place '%s'", script[2].c_str());
				}
				else
					_Place = place->getId();
			}
			return ret;
		}
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Kill )
		{
			CMissionEventKill & eventSpe = (CMissionEventKill&)event;
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( !c )
			{
				LOGMISSIONSTEPERROR("kill_race : invalid creature " + toString(event.TargetEntity.getIndex()));
			}
			else if ( _SubSteps[subStepIndex].Race == c->getRace() )
			{
				if ( _Place != 0xFFFF )
				{
					float gooDistance;
					const CPlace * stable = NULL;
					std::vector<const CPlace *> places;
					const CRegion * region = NULL;
					const CContinent * continent = NULL;
					if ( !CZoneManager::getInstance().getPlace( c->getState().X, c->getState().Y, gooDistance, &stable, places, &region , &continent ) )
						return 0;
					if ( continent && continent->getId() == _Place )
					{
						LOGMISSIONSTEPSUCCESS("kill_race");
						return 1;
					}
					if ( region && region->getId() == _Place )
					{
						LOGMISSIONSTEPSUCCESS("kill_race");
						return 1;
					}
					for ( uint i = 0; i < places.size(); i++ )
					{
						if ( places[i] && places[i]->getId() == _Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_race");
							return 1;
						}
					}
					return 0;
				}
				else
				{
					LOGMISSIONSTEPSUCCESS("kill_race");
					return 1;
				}
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = _SubSteps[i].Quantity;
		}
	}

	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepTextReact = "MIS_NEED_REACTIVATION";
		static const std::string stepText = "MIS_KILL_RACE_";
		static const std::string stepTextLoc = "MIS_KILL_RACE_LOC_";

		nlassert( _SubSteps.size() == subStepStates.size() );
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::race;
				retParams.back().Enum = (uint)_SubSteps[i].Race;
			
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = subStepStates[i];
			}
		}
		if ( _Place != 0xFFFF )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::place;
			CPlace * place = CZoneManager::getInstance().getPlaceFromId(_Place);
			if ( !place )
			{
				MISLOG("sline:%u ERROR : kill_race : Invalid place %u", _SourceLine, _Place);
			}
			else
			{
				param.Identifier = place->getName();
				retParams.push_back(param);
			}
			textPtr = &stepTextLoc;
		}
		else
			textPtr = &stepText;
	}
	
	std::vector< CSubStep > _SubSteps;
	uint16					_Place;

	MISSION_STEP_GETNEWPTR(CMissionStepKillRace)
};
MISSION_REGISTER_STEP(CMissionStepKillRace,"kill_race");


// ----------------------------------------------------------------------------
class CMissionStepKillNpc : public IMissionStepTemplate
{
	struct CSubStep
	{
		string		Dynamic;
		TAIAlias	Alias;
//		NLMISC::TStringId	NpcName;
	};
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() < 2 || script.size() > 3)
		{
			MISLOGSYNTAXERROR("<npc_name>*[; <npc_name>]");
			return false;
		}
		else
		{
			std::vector< std::string > subs;
			NLMISC::splitString( script[1],";", subs );
			_SubSteps.reserve(subs.size());
			for ( uint i = 0; i < subs.size(); i++ )
			{
				CSubStep subStep;
				//// Dynamic Mission Args : #dynamic#
				if (trim(subs[i]) == "#dynamic#") {
					subStep.Dynamic = missionData.Name;
				} 
				////
				else 
				{
					subStep.Alias = CAIAliasTranslator::Invalid;
					if ( !CMissionParser::parseBotName(subs[i],subStep.Alias,missionData) )
						ret = false;
				}
				_SubSteps.push_back( subStep );
			}
			return ret;
		}
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		string webAppUrl;
		CCharacter * user = PlayerManager.getChar(getEntityIdFromRow(userRow));

		if ( event.Type == CMissionEvent::Kill )
		{
			CMissionEventKill & eventSpe = (CMissionEventKill&)event;
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( !c )
			{
				LOGMISSIONSTEPERROR("kill_npc : invalid creature " + toString(event.TargetEntity.getIndex()));
			}
			else
			{
				if (_SubSteps[subStepIndex].Dynamic.empty())
				{
					if ( _SubSteps[subStepIndex].Alias != CAIAliasTranslator::Invalid )
					{
						if ( _SubSteps[subStepIndex].Alias == c->getAlias() )
						{
							LOGMISSIONSTEPSUCCESS("kill_npc");
							return 1;
						}
					}
					else if (  event.TargetEntity == giverRow )
					{
						LOGMISSIONSTEPSUCCESS("kill_npc");
						return 1;
					}
				}
				else
				{
					//// Dynamic Mission Args								
					vector<string> params = user->getCustomMissionParams(_SubSteps[subStepIndex].Dynamic);
					if (params.size() < 2) {
						LOGMISSIONSTEPERROR("kill_npc : invalid dynamic npc");
						return 0;
					}
					else
					{
						webAppUrl = params[0];
						string name;
						
						CAIAliasTranslator::getInstance()->getNPCNameFromAlias(c->getAlias(), name);
						if ( name == params[1] )
						{
							user->validateDynamicMissionStep(webAppUrl);
							LOGMISSIONSTEPSUCCESS("kill_npc");
							return 1;
						}
					}
					////
				}
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = 1;
		}
	}

	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepTextReact = "MIS_NEED_REACTIVATION";
		static const std::string stepText = "MIS_KILL_NPC_";
		textPtr = &stepText;
		nlassert( _SubSteps.size() == subStepStates.size() );
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::bot;
				if (_SubSteps[i].Dynamic.empty())
				{
					if ( _SubSteps[i].Alias != CAIAliasTranslator::Invalid )
						retParams.back().Int = _SubSteps[i].Alias;
					else
						retParams.back().Identifier = "giver";
				}
				else
				{
					vector<string> params = _User->getCustomMissionParams(_SubSteps[i].Dynamic);
					if (params.size() < 2)
					{
						nlinfo("kill_npc : invalid dynamic npc");
						textPtr = &stepTextReact;
						return;
					}
					else
					{
						vector<TAIAlias> aliases;
						CAIAliasTranslator::getInstance()->getNPCAliasesFromName( params[1] , aliases );
						if ( aliases.empty() )
						{
							retParams.back().Int = CAIAliasTranslator::Invalid;
							return;
						}
						retParams.back().Int = aliases[0];
					}
				}
			}
		}
	}
	
	// We don't define getInvolvedBot() here, because 1) we don't want an icon on bots to kill,
	// and 2) at the moment only one bot can be returned

	std::vector< CSubStep > _SubSteps;

	MISSION_STEP_GETNEWPTR(CMissionStepKillNpc)
};
MISSION_REGISTER_STEP(CMissionStepKillNpc,"kill_npc");


// ----------------------------------------------------------------------------
class CMissionStepKillGroup : public IMissionStepTemplate
{
	struct CSubStep
	{
		TAIAlias	Alias;
	};
	
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<group_name>");
			return false;
		}

		std::vector< std::string > subs;
		NLMISC::splitString( script[1],";", subs );
		
		if ( subs.size() != 1 )
		{
			MISLOGSYNTAXERROR("<group_name>");
			return false;
		}

		_SubSteps.reserve(1);
		
		{
 			CMissionParser::removeBlanks( subs[0] );
			std::vector< TAIAlias > aliases;
			CAIAliasTranslator::getInstance()->getGroupAliasesFromName( subs[0],aliases );
			if ( aliases.empty() )
			{
				ret = false;
				MISLOGERROR1("invalid group %s", subs[0].c_str());
			}

			for ( uint j = 0; j < aliases.size(); j++)
			{
				CSubStep subStep;
				subStep.Alias = aliases[j];
				_SubSteps.push_back( subStep );

			}
			Target = subs[0];
		}
		return ret;
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::KillGroup )
		{
			CMissionEventKillGroup & eventSpe = (CMissionEventKillGroup&)event;
			{
				if ( _SubSteps[subStepIndex].Alias == eventSpe.Alias )
				{
					LOGMISSIONSTEPSUCCESS("kill_group");
					return 1;
				}
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = 1;
		}
	}
	
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		/// overloaded text in script

		static const std::string stepText = "MIS_KILL_GROUP";
		textPtr = &stepText;
		retParams.push_back(STRING_MANAGER::TParam(STRING_MANAGER::bot_name));
		retParams.back().Type = STRING_MANAGER::bot_name;
		retParams.back().Identifier = Target;

	}

	// See comment in CMissionStepKillNpc about getInvolvedBot()

	bool checkTextConsistency()
	{
		return true;
	}
	
	std::vector< CSubStep > _SubSteps;
	std::string Target;

	MISSION_STEP_GETNEWPTR(CMissionStepKillGroup)
};
MISSION_REGISTER_STEP(CMissionStepKillGroup,"kill_group");


// ----------------------------------------------------------------------------
class CMissionStepKillFaction : public IMissionStepTemplate
{
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		_Place = 0xFFFF;
		bool ret = true;
		if ( script.size() < 2 || script.size() > 3)
		{
			MISLOGSYNTAXERROR("<faction> <quantity> [: <place>]");
			return false;
		}
	
		std::vector< std::string > args;
		CMissionParser::tokenizeString( script[1]," \t", args );
		if ( args.size() != 2 )
		{
			MISLOGSYNTAXERROR("<faction> <quantity> [: <place>]");
			return false;
		}

		missionData.ChatParams.push_back( make_pair(args[0], STRING_MANAGER::faction) );
		_Faction = CStaticFames::getInstance().getFactionIndex(args[0]);
		if ( _Faction == CStaticFames::INVALID_FACTION_INDEX )
		{
			ret = false;
			MISLOGERROR1("invalid faction '%s'", args[0].c_str());
		}
		NLMISC::fromString(args[1], _Quantity);

		if ( script.size() == 3 )
		{
			string placeStr = CMissionParser::getNoBlankString( script[2] );
			CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
			missionData.ChatParams.push_back( make_pair(placeStr,STRING_MANAGER::place) );
			if ( !place )
			{
				ret = false;
				MISLOGERROR1("invalid place '%s'", script[2].c_str());
			}
			else
				_Place = place->getId();
		}
		return ret;
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Kill )
		{
			CMissionEventKill & eventSpe = (CMissionEventKill&)event;
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( !c )
			{
				LOGMISSIONSTEPERROR("kill_faction : invalid creature " + toString(event.TargetEntity.getIndex()));
			}
			else if ( c->getForm()->getFaction() == _Faction )
			{
				if ( _Place != 0xFFFF )
				{
					float gooDistance;
					const CPlace * stable = NULL;
					std::vector<const CPlace *> places;
					const CRegion * region = NULL;
					const CContinent * continent = NULL;
					if ( !CZoneManager::getInstance().getPlace( c->getState().X, c->getState().Y, gooDistance, &stable, places, &region , &continent ) )
						return 0;
					if ( continent && continent->getId() == _Place )
					{
						LOGMISSIONSTEPSUCCESS("kill_faction");
						return 1;
					}
					if ( region && region->getId() == _Place )
					{
						LOGMISSIONSTEPSUCCESS("kill_faction");
						return 1;
					}
					for ( uint i = 0; i < places.size(); i++ )
					{
						if ( places[i] && places[i]->getId() == _Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_faction");
							return 1;
						}
					}
					return 0;
				}
				else
				{
					LOGMISSIONSTEPSUCCESS("kill_faction");
					return 1;
				}
			}
		}
		return 0;
	}

	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( 1 );
		ret.back() = _Quantity;
	}

	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = "MIS_KILL_FACTION";
		static const std::string stepTextLoc = "MIS_KILL_FACTION_LOC";
		nlassert( subStepStates.size() == 1);

		nbSubSteps = 1;
		retParams.push_back(STRING_MANAGER::TParam(STRING_MANAGER::faction));
		retParams.back().Enum = _Faction;
		retParams.push_back(STRING_MANAGER::TParam(STRING_MANAGER::integer, sint32(_Quantity)));
				
		if ( _Place != 0xFFFF )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::place;
			CPlace * place = CZoneManager::getInstance().getPlaceFromId(_Place);
			if ( !place )
			{
				MISLOG("sline:%u ERROR : kill_faction : invalid place %u", _SourceLine, _Place);
			}
			else
			{
				param.Identifier = place->getName();
				retParams.push_back(param);
			}
			textPtr = &stepTextLoc;
		}
		else
			textPtr = &stepText;
		
	}

	/// Faction as defined in CStaticFame
	uint32		_Faction;
	uint16		_Quantity;
	uint16		_Place;

	MISSION_STEP_GETNEWPTR(CMissionStepKillFaction)
};
MISSION_REGISTER_STEP(CMissionStepKillFaction,"kill_faction");


// ----------------------------------------------------------------------------
class CMissionStepKillByName : public IMissionStepTemplate
{
	bool				GroupKill;
	std::set<TAIAlias>	Aliases;
	string				TargetName;
	uint16				Quantity;
	uint16				Place;
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;

		GroupKill = false;
		Place = 0xFFFF;
		if ( script.size() != 2 && script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<npc_name> <quantity>");
			return false;
		}
		std::vector< std::string > args;
		CMissionParser::tokenizeString( script[1]," \t", args );
		if ( args.size() != 2 )
		{
			MISLOGSYNTAXERROR("<npc_name> <quantity>");
			return false;
		}

		TargetName = args[0];
		CAIAliasTranslator::getInstance()->getNPCAliasesFromNameInSet( args[0], Aliases );
		if ( Aliases.empty() )
		{
			// try to look for a group alias
			GroupKill = true;
			vector<TAIAlias> va;
			CAIAliasTranslator::getInstance()->getGroupAliasesFromName( args[0], va );
			if ( va.empty() )
			{
				MISLOGERROR1("invalid name '%s'",args[0].c_str());
				return false;
			}

			Aliases.insert(va.begin(), va.end());
		}
		NLMISC::fromString(args[1], Quantity);
		if ( Quantity == 0 )
		{
			MISLOGERROR("invalid quantity 0");
			return false;
		}

		missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::bot ) );

		if ( script.size() == 3 )
		{
			string placeStr = CMissionParser::getNoBlankString( script[2] );
			CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
			missionData.ChatParams.push_back( make_pair(placeStr,STRING_MANAGER::place) );
			if ( !place )
			{
				MISLOGERROR1("invalid place '%s'", script[2].c_str());
				return false;
			}
			Place = place->getId();
		}
		return true;
	}
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::Kill )
		{
			CCreature * c = CreatureManager.getCreature( event.TargetEntity );
			if ( !c )
			{
				LOGMISSIONSTEPERROR("kill_npc_by_name : invalid creature " + toString(event.TargetEntity.getIndex()));
			}
			else
			{
				if ( (GroupKill && Aliases.find(c->getAIGroupAlias()) != Aliases.end())
					|| Aliases.find( c->getAlias() ) != Aliases.end() )
				{
					if ( Place != 0xFFFF )
					{
						float gooDistance;
						const CPlace * stable = NULL;
						std::vector<const CPlace *> places;
						const CRegion * region = NULL;
						const CContinent * continent = NULL;
						if ( !CZoneManager::getInstance().getPlace( c->getState().X, c->getState().Y, gooDistance, &stable, places, &region , &continent ) )
							return 0;
						if ( continent && continent->getId() == Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_npc_by_name");
							return 1;
						}
						if ( region && region->getId() == Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_npc_by_name");
							return 1;
						}
						for ( uint i = 0; i < places.size(); i++ )
						{
							if ( places[i] && places[i]->getId() == Place )
							{
								LOGMISSIONSTEPSUCCESS("kill_npc_by_name");
								return 1;
							}
						}
						return 0;
					}
					else
					{
						LOGMISSIONSTEPSUCCESS("kill_npc_by_name");
						return 1;
					}
				}
			}
		}
		return 0;
	}
	
	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( 1 );
		ret[0] = Quantity;
	}

	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepTextNoLoc = "MIS_KILL_NPC_BY_NAME";
		static const std::string stepTextLoc = "MIS_KILL_NPC_BY_NAME_LOC";
		if ( Place != 0xFFFF )
			retParams.resize(3);
		else
			retParams.resize(2);
		retParams[0].Type = STRING_MANAGER::bot_name;
		retParams[0].Identifier = TargetName;
		retParams[1].Type = STRING_MANAGER::integer;
		nlassert( subStepStates.size() == 1 );
		retParams[1].Int = subStepStates[0];
		if ( Place != 0xFFFF )
		{
			CPlace * place = CZoneManager::getInstance().getPlaceFromId( Place );
			if ( !place )
			{
				MISLOG("sline:%u ERROR : kill_npc_by_name : invalid place %u", _SourceLine, Place);
			}
			else
			{
				retParams[2].Type = STRING_MANAGER::place;
				retParams[2].Identifier = place->getName();
				textPtr = &stepTextLoc;
			}
		}
		else
			textPtr = &stepTextNoLoc;

	}

	MISSION_STEP_GETNEWPTR(CMissionStepKillByName)
};
MISSION_REGISTER_STEP(CMissionStepKillByName,"kill_npc_by_name");


// ----------------------------------------------------------------------------
class CMissionStepKillPlayer : public IMissionStepTemplate
{
	struct CSubStep
	{
		sint32	Clan;
		sint32	MinLevel;
		sint32	MaxLevel;
		uint16	Quantity;
	};


	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
	{
		_SourceLine = line;
		_Place = 0xFFFF;

		if ( script.size() < 2 || script.size() > 3)
		{
			MISLOGSYNTAXERROR("<clan_name> <min_level> <max_level> <quantity> *[; <clan_name> <min_level> <max_level> <quantity>] [:<place>]");
			return false;
		}
		std::vector< std::string > subs;
		NLMISC::splitString( script[1],";", subs );
		for ( uint i = 0; i < subs.size(); i++ )
		{
			std::vector< std::string > args;
			CMissionParser::tokenizeString( subs[i]," \t", args );
			if ( args.size() != 4 )
			{
				MISLOGSYNTAXERROR("<clan_name> <min_level> <max_level> <quantity> *[; <clan_name> <min_level> <max_level> <quantity>] [:<place>]");
				return false;
			}
			CSubStep subStep;
			subStep.Clan = PVP_CLAN::getFactionIndex(PVP_CLAN::fromString(args[0]));
			if ( subStep.Clan == CStaticFames::INVALID_FACTION_INDEX )
			{
				MISLOGERROR("invalid faction name (Allowed = kami/karavan/tryker/matis/fyros/zorai)");
				return false;
			}
//			missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::clan ) );
			subStep.MinLevel = atoi( args[1].c_str() ) * kFameMultipler;
			subStep.MaxLevel = atoi( args[2].c_str() ) * kFameMultipler;
			if ( subStep.MinLevel >= subStep.MaxLevel )
			{
				MISLOGERROR("min_level >= max_level");
				return false;
			}
			subStep.Quantity = (uint16) atoi( args[3].c_str() );
			if ( subStep.Quantity == 0 )
			{
				MISLOGERROR("invalid quantity 0");
				return false;
			}
			_SubSteps.push_back( subStep );
		}

		if ( script.size() == 3 )
		{
			string placeStr = CMissionParser::getNoBlankString( script[2] );
			missionData.ChatParams.push_back( make_pair(placeStr,STRING_MANAGER::place) );
			CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
			if ( !place )
			{
				MISLOGERROR1("invalid place '%s'", script[2].c_str());
				return false;
			}
			else {_Place = place->getId();}
		}
		return true;
	}


	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
	{
		if ( event.Type == CMissionEvent::KillPlayer )
		{
			CCharacter * victim = PlayerManager.getChar( event.TargetEntity );

			if ( !victim )
			{
				LOGMISSIONSTEPERROR("kill_player : invalid victim " + toString(event.TargetEntity.getIndex()));
			}
			else
			{
				sint32 victimFame = CFameInterface::getInstance().getFameIndexed(victim->getId(), _SubSteps[subStepIndex].Clan);
				if ( (victimFame > _SubSteps[subStepIndex].MinLevel) && 
					 (victimFame < _SubSteps[subStepIndex].MaxLevel) )
				{
					if ( _Place != 0xFFFF )
					{
						float gooDistance;
						const CPlace * stable = NULL;
						std::vector<const CPlace *> places;
						const CRegion * region = NULL;
						const CContinent * continent = NULL;
						if ( !CZoneManager::getInstance().getPlace( victim->getState().X, victim->getState().Y, gooDistance, &stable, places, &region , &continent ) )
							return 0;
						if ( continent && continent->getId() == _Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_player");
							return 1;
						}
						if ( region && region->getId() == _Place )
						{
							LOGMISSIONSTEPSUCCESS("kill_player");
							return 1;
						}
						for ( uint i = 0; i < places.size(); i++ )
						{
							if ( places[i] && places[i]->getId() == _Place )
							{
								LOGMISSIONSTEPSUCCESS("kill_player");
								return 1;
							}
						}
						return 0;
					}
					else
					{
						LOGMISSIONSTEPSUCCESS("kill_player");
						return 1;
					}
				}
			}
		}
		return 0;
	}
	

	void getInitState( std::vector<uint32>& ret )
	{
		ret.clear();
		ret.resize( _SubSteps.size() );
		for ( uint i = 0; i < _SubSteps.size(); i++ )
		{
			ret[i] = _SubSteps[i].Quantity;
		}
	}


	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{
		static const std::string stepText = "MIS_KILL_PLAYER";
		static const std::string stepTextLoc = "MIS_KILL_PLAYER_LOC";
		if ( _Place != 0xFFFF )
			retParams.resize(3);
		else
			retParams.resize(2);
		textPtr = &stepText;
		nlassert( _SubSteps.size() == subStepStates.size() );
		for ( uint i  = 0; i < subStepStates.size(); i++ )
		{
			if( subStepStates[i] != 0 )
			{
				nbSubSteps++;
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::faction;
				retParams.back().Enum = _SubSteps[i].Clan;
	
				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = _SubSteps[i].MinLevel;

				retParams.push_back(STRING_MANAGER::TParam());
				retParams.back().Type = STRING_MANAGER::integer;
				retParams.back().Int = _SubSteps[i].MaxLevel;
			}
		}
		/*
		if ( _Place != 0xFFFF )
		{
			STRING_MANAGER::TParam param;
			param.Type = STRING_MANAGER::place;
			CPlace * place = CZoneManager::getInstance().getPlaceFromId(_Place);
			if ( !place )
			{
				MISLOG("sline:%u ERROR : kill_player : Invalid place %u", _SourceLine, _Place);
			}
			else
			{
				param.Identifier = place->getName();
				retParams.push_back(param);
			}
			textPtr = &stepTextLoc;
		}
		else
		*/
		textPtr = &stepText;
	}

	std::vector< CSubStep >	_SubSteps;
	uint16					_Place;

	MISSION_STEP_GETNEWPTR(CMissionStepKillPlayer)
};
MISSION_REGISTER_STEP(CMissionStepKillPlayer, "kill_player");
