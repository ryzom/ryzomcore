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

#include "nel/misc/algo.h"

#include "game_share/fame.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "egs_sheets/egs_static_encyclo.h"
#include "egs_sheets/egs_sheets.h"
#include "mission_manager/mission_template.h"
#include "mission_log.h"
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_parser.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/character_encyclopedia.h"
#include "mission_item.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_guild.h"
#include "team_manager/team_manager.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "primitives_parser.h"
#include "player_manager/character_game_event.h"
#include "egs_variables.h"

using namespace std;
using namespace NLMISC;

extern CVariable<sint32> 	MissionForcedSeason;
extern CVariable<bool>		MissionPrerequisitsEnabled;
extern CPlayerManager		PlayerManager;


NL_INSTANCE_COUNTER_IMPL(CMissionTemplate);

CMissionTemplate::CMissionTemplate()
{
	// Yoyo: I prefer when classes have a constructor...
	Alias= CAIAliasTranslator::Invalid;
	_DefautltNpcGiverAlias= CAIAliasTranslator::Invalid;
	Icon= CSheetId::Unknown;
	AlreadyDone= false;
	MonoTimer= 0;
	PlayerReplayTimer= 0;
	GlobalReplayTimer= 0;
	LastSuccessDate= 0;
	EncycloAlbum= -1;
	EncycloThema= -1;
	EncycloTask= -1;
	EncycloNPC= CAIAliasTranslator::Invalid;
}

CMissionTemplate::~CMissionTemplate()
{
	for ( uint i = 0; i < Steps.size(); i++ ) 
	{
		delete Steps[i];
	}
	for ( uint i = 0; i < InitialActions.size(); i++ )
	{
		delete InitialActions[i];
	}
	for ( uint i = 0; i < FailureActions.size(); i++ )
	{
		for ( uint j = 0; j < FailureActions[i].size(); j++ )
			delete FailureActions[i][j];
	}
	for ( uint i = 0; i < OOOActions.size(); i++ )
	{
		for ( uint j = 0; j < OOOActions[i].size(); j++ )
			delete OOOActions[i][j];
	}
	for ( uint i = 0; i < CrashHandlers.size(); i++ )
	{
		for ( uint j = 0; j < CrashHandlers[i].Actions.size(); j++ )
			delete CrashHandlers[i].Actions[j];
	}
	for ( uint i = 0; i < PlayerReconnectHandlers.size(); i++ )
	{
		for ( uint j = 0; j < PlayerReconnectHandlers[i].Actions.size(); j++ )
			delete PlayerReconnectHandlers[i].Actions[j];
	}
}

void CMissionTemplate::copy(CMissionTemplate & ref)
{
	ref = * this;
	for ( uint i = 0; i < Steps.size(); i++ )
	{
		ref.Steps[i] = Steps[i]->getCopy();
	}
	for ( uint i = 0; i < InitialActions.size(); i++ )
	{
		ref.InitialActions[i] = InitialActions[i]->getCopy();
	}
	for ( uint i = 0; i < FailureActions.size(); i++ )
	{
		for ( uint j = 0; j < FailureActions[i].size(); j++ )
			ref.FailureActions[i][j] = FailureActions[i][j]->getCopy();
	}
	for ( uint i = 0; i < OOOActions.size(); i++ )
	{
		for ( uint j = 0; j < OOOActions[i].size(); j++ )
			ref.OOOActions[i][j] = OOOActions[i][j]->getCopy();
	}

}

const std::string &CMissionTemplate::getMissionName() const
{
	return CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(Alias);
}

// :NOTE: This function exists in mission_compiler_feDlg.cpp. If you change it here modify the other file.
std::string buildHashKey(std::string const& content)
{
	uint32 sum = 0;
	size_t size = content.length()/4;
	for (size_t i=0; i<size; ++i)
	{
		uint32 val = 0;
		for (int j=0; j<4; ++j)
			val += content[4*i+j]<<8*j;
		sum += val;
		if (sum&1)
			sum = sum>>1 | 0x80000000;
		else
			sum = sum>>1;
	}
	return NLMISC::toString("0x%08X", sum);
}

bool CMissionTemplate::build(const NLLIGO::IPrimitive* prim,CMissionGlobalParsingData & globalData, CHashMap<uint,TAIAlias> & refMissions, const std::string &missionName, TAIAlias defautltNpcGiverAlias)
{
	// init default values
	MonoTimer = 0;
	PlayerReplayTimer = 0;
	GlobalReplayTimer = 0;
	LastSuccessDate = 0;
/*	MissionStats.LastTryDate = 0;
	MissionStats.SuccessCount = 0;
	MissionStats.FailCount = 0;
	MissionStats.AbandonCount= 0;
*/	AlreadyDone = false;
	Alias = CAIAliasTranslator::Invalid;
	_DefautltNpcGiverAlias = defautltNpcGiverAlias;
	Type = MISSION_DESC::Group;

	Tags.Replayable = false;
	Tags.DoneOnce = false;
	Tags.NoList = false;
	Tags.AutoRemove = false;
	Tags.NotProposed = false;
	Tags.NonAbandonnable = false;
	Tags.NeedValidation = false;
	Tags.FailIfInventoryIsFull = false;
	Tags.HideIconOnGiverNPC = false;
	
	Prerequisits.FameId = (TStringId)0;
	Prerequisits.Title = CHARACTER_TITLE::NB_CHARACTER_TITLE;
	Prerequisits.Guild = false;
	Prerequisits.GuildGrade	 = EGSPD::CGuildGrade::Unknown;
	Prerequisits.TeamSize = 0;
	Prerequisits.Season = EGSPD::CSeason::Invalid;
	Prerequisits.CharacterMinAge = 0;
	Prerequisits.MaxPlayerID = 0;  
	
	// init parsing vars
	bool ret = true;
	string value;
	vector<string> *params;
	uint32 currentStep = 0xFFFFFFFF;
	uint32 currentAction = 0;		// Yoyo: must start at 0 for correct jumps before first step
	uint32 outOfOrderStep = 0xFFFFFFFF;
	bool setObjInOOO = false;

	CMissionSpecificParsingData missionData;
	missionData.Name = missionName;
	missionData.Template = this;
	missionData.ChatParams.push_back(std::make_pair( string("player"), STRING_MANAGER::player ) );
	missionData.ChatParams.push_back(std::make_pair( string("giver"), STRING_MANAGER::bot ) );
	missionData.ChatParams.push_back(std::make_pair( string("guild_name"), STRING_MANAGER::string_id ) );

	CConfigFile::CVar* var = NLNET::IService::getInstance()->ConfigFile.getVarPtr("DefaultMissionIcon");
	if (var)
		Icon = CSheetId(var->asString() + ".mission_icon");
	else
		Icon = CSheetId::Unknown;
	
	std::string overridenText;
	TVectorParamCheck overridenParams;
	std::string roleplayText;
	TVectorParamCheck roleplayParams;
	std::string roleplayTextOOO;
	TVectorParamCheck roleplayParamsOOO;
	string openTag;
	
	
	bool afterOOO = false;
	uint32 oooIndex = 0xFFFFFFFF;

	bool failure = false;
	uint32 failureIndex = 0xFFFFFFFF;

	bool crashHandler = false;
	uint32 crashHandlerIndex = 0xFFFFFFFF;

	bool playerReconnectHandler = false;
	uint32 playerReconnectHandlerIndex = 0xFFFFFFFF;
	
	bool oooDisplayed = true;

	bool rewardTag = false;
	bool displayNextStep = true;
	bool displayNextIconOnStepNPC = true;
		
	//get the alias
	if ( !CPrimitivesParser::getAlias(prim, Alias))
	{
		MISLOG("ERROR : no alias in mission");
		return false;
	}

	if ( Alias == 0 || Alias == CAIAliasTranslator::Invalid )
	{
		value = CPrimitivesParser::aliasToString(Alias);
		MISLOG("ERROR : invalid alias %s in  mission", value.c_str());
		return false;
	}
	
	// get the mission script
	if ( !prim->getPropertyByName("script",params) || !params)
	{
		MISLOG("ERROR : cant find mission script!!!!!!");
		return false;
	}
	
	// make a checksum
	// Temporarily save script in hashKey
	HashKey = "";
	vector<string>::iterator itParam, itParamEnd = params->end();
	for (itParam=params->begin(); itParam!=itParamEnd; ++itParam)
		HashKey += *itParam + "\n";
	HashKey = buildHashKey(HashKey);
	
	// remove comments
	for (uint i = 0; i < params->size();i++)
	{
		for ( uint j = 0; j  < (*params)[i].size(); j++ )
		{
			if( (*params)[i][j] != ' ' &&  (*params)[i][j] != '#')
				break;
			else if( (*params)[i][j] == '#')
			{
				(*params)[i] = (*params)[i].substr(0, j);
//				(*params).erase( (*params).begin() + i );
//				i--;
				break;
			}
		}
	}
	
	// add an end param at the end of the script
	params->push_back("end");
	
	// parse them
	vector<string>::iterator itParams = params->begin();
	for (uint i = 0; i < params->size();++i,++itParams)
	{
		uint32 line = i+1;
		vector<string> script;
		NLMISC::splitString( (*params)[i],":",script );
		if ( !script.empty()  )
		{
			CMissionParser::removeBlanks( script[0] );

			if (script[0].empty())
			{
				if (script.size() == 1)
				{
					// this is an empty line
					continue;
				}
				else
				{
					MISLOG("sline:%u ERROR parsing line '%s'", i+1, (*params)[i].c_str());
					ret = false;
				}
			}
			if ( rewardTag )
			{
				if ( script[0] == "/reward" )
				{
					rewardTag = false;
					openTag.clear();
				}
				else
				{
					IMissionAction * action;
					if ( failure )
						action = FailureActions[failureIndex].back();
					else if ( crashHandler )
						action = CrashHandlers[crashHandlerIndex].Actions.back();
					else 
					{
						if (afterOOO)
						{
							if( oooIndex == 0xFFFFFFFF )
							{
								MISLOG("<MISSIONS> in reward tag line %u: ooo tags are not properly enclosed", i);
								ret = false;
							}
							action = OOOActions[oooIndex].back();
						}
						else
						{
							if ( currentStep == 0xFFFFFFFF )
							{
								action = InitialActions.back();
							}
							else
							{
								nlassert(currentStep < Steps.size() );
								action = Steps[currentStep]->getActions().back();
							}
						}
					}
					if ( !action->addReward(script,missionData.Items,i) )
					{
						MISLOG("could not add reward");
						ret = false;
					}
				}
				continue;
			}

			if ( script[0] == "copy_mission" )
			{
				TAIAlias alias = parseMissionParam(i+1,script);
				if ( alias != CAIAliasTranslator::Invalid )
					globalData.CopyMissions.push_back( make_pair(Alias,alias) );
				return true;
			}
			if ( script[0] == "ref_mission" )
			{
				TAIAlias alias = parseMissionParam(i+1,script);
				if ( alias != CAIAliasTranslator::Invalid )
				{
					if ( ( refMissions.insert( make_pair(Alias,alias)) ).second == false )
					{
						MISLOGERROR1("ref mission alias %s already exists", CPrimitivesParser::aliasToString(Alias).c_str());
						return false;
					}
					return true;
				}
				else
				{
					MISLOGERROR1("referenced mission alias %s is invalid", CPrimitivesParser::aliasToString(Alias).c_str());
					return false;
				}
			}
			if ( script[0] == "solo" )
				Type = MISSION_DESC::Solo;
			else if ( script[0] == "guild" )
				Type = MISSION_DESC::Guild;
			else if ( script[0] == "decl_item" )
			{
				CMissionItem item;
				string itemName;
				if ( item.buildFromScript( script, missionData.ChatParams , itemName) )
					missionData.Items.push_back( make_pair(itemName,item) );
				else
					ret = false;
			}
			else if ( script[0] == "decl" )
				ret = parseScriptVar( i+1, script, missionData.ChatParams ) && ret;
			else if ( script[0] == "mission_title" )
				ret = parseMissionText(i+1, script,TitleText,TitleParams) && ret;
			else if ( script[0] == "mission_desc" )
				ret = parseMissionText(i+1, script,DescText,DescParams) && ret;
			else if ( script[0] == "mission_icon" )
			{
				ret = ret && (script.size() == 2);
				if (ret)
				{
					vector<string> vars;
					CMissionParser::tokenizeString( script[1], " \t", vars );
					CSheetId icon;
					if ( vars.size() == 1 )
						icon = CSheetId(vars[0] + ".mission_icon");
					if (icon != CSheetId::Unknown)
						Icon = icon;
				}
			}
			else if ( script[0] == "mission_category" )
			{
				ret = ret && (script.size() == 2);
				if (ret)
				{
					vector<string> vars;
					CMissionParser::tokenizeString( script[1], " \t", vars );
					if (vars.size() == 1)
						MissionCategory = vars[0];
				}
			}
			else if ( script[0] == "mono" )
			{
				if ( script.size() > 1)
					NLMISC::fromString(script[1], MonoTimer);
				else
					MonoTimer = MonoMissionTimout;
			}
			else if ( script[0] == "decl_price" )
				ret = parsePrice(i+1, script, missionData) && ret;
			else if ( script[0] == "player_replay_timer" )
				ret = parseReplayTimer(i+1, script, PlayerReplayTimer) && ret;
			else if ( script[0] == "global_replay_timer" )
				ret = parseReplayTimer(i+1, script, GlobalReplayTimer) && ret;
			else if ( script[0] == "once" )
				Tags.DoneOnce = true;
			else if ( script[0] == "replayable" )
				Tags.Replayable = true;
			else if ( script[0] == "need_validation" )
				Tags.NeedValidation = true;
			else if ( script[0] == "auto" )
				ret = parseMissionText(i+1, script,AutoText,AutoParams) && ret;
			else if ( script[0] == "no_list" )
				Tags.NoList = true;
			else if ( script[0] == "auto_remove" )
				Tags.AutoRemove = true;
			else if ( script[0] == "not_proposed" )
				Tags.NotProposed = true;
			else if ( script[0] == "non_abandonnable" )
				Tags.NonAbandonnable = true;
			else if ( script[0] == "fail_if_inventory_is_full" )
				Tags.FailIfInventoryIsFull = true;
			else if ( script[0] == "hide_icon_on_giver_npc" )
				Tags.HideIconOnGiverNPC = true;
			else if ( script[0] == "req_season")
				ret = parseSeason( i+1, script, Prerequisits.Season ) && ret;
			else if ( script[0] == "req_skill" )
				ret = addSkillToList( i+1, script, missionData.ChatParams ) && ret;
			else if ( script[0] == "req_mission" )
				ret = addMissionsToList( i+1, script, Prerequisits.NeededMissions ) && ret;
			else if ( script[0] == "req_mission_neg" )
				ret = addMissionsToList( i+1, script, Prerequisits.ForbiddenMissions ) && ret;
			else if ( script[0] == "req_mission_running" )
				ret = addMissionsToList( i+1, script, Prerequisits.RunningMissions ) && ret;
			else if ( script[0] == "req_mission_running_neg" )
				ret = addMissionsToList( i+1, script, Prerequisits.ForbiddenRunningMissions ) && ret;
			else if ( script[0] == "req_wear" )
				ret = parseItemList( i+1, ";",script, Prerequisits.Wear , missionData.ChatParams) && ret;
			else if ( script[0] == "req_item" )
				ret = parseItemList( i+1, ";",script, Prerequisits.Own , missionData.ChatParams) && ret;
			else if ( script[0] == "req_guild" )
				Prerequisits.Guild = true;
			else if ( script[0] == "req_title" )
				ret = parseTitlePrereq( i+1, script ) && ret;
			else if ( script[0] == "req_fame" )
				ret = parseFamePrereq(i+1, script ) && ret;
			else if ( script[0] == "req_grade" )
				ret = parseGradePrereq(i+1, script ) && ret;
			else if ( script[0] == "req_kami" )
			{
				if ( Prerequisits.KamiKaravan != CPrerequisits::None )
					MISLOGERROR("only one req_kami / req_karavan per mission");
				Prerequisits.KamiKaravan = CPrerequisits::Kami;
			}
			else if ( script[0] == "req_karavan" )
			{
				ret = ( Prerequisits.KamiKaravan == CPrerequisits::None ) && ret;
				Prerequisits.KamiKaravan = CPrerequisits::Karavan;
			}

			else if ( script[0] == "req_team_size" )
			{
				sint size;
				ret = parseInt( i+1, script,size ) && ret;
				Prerequisits.TeamSize = (uint8)size;
				if ( Prerequisits.TeamSize < 2 )
				{
					MISLOGERROR1("team size is %u. Must be >= 2", Prerequisits.TeamSize );
					ret = false;
				}
			}
			else if ( script[0] == "req_brick" )
				ret = parseBrickList( i+1, script,Prerequisits.KnownActions,missionData.ChatParams) && ret;

			// encyclopedia requesite all ritual tasks done (not the rite) and the not function
			else if (( script[0] == "req_encyclo_thema" ) || ( script[0] == "req_encyclo_thema_neg" ))
			{
				ret = (script.size() == 2);
				if (ret)
				{
					vector<string> vars;
					CMissionParser::tokenizeString( script[1]," \t",vars );
					if ( vars.size() == 2 )
					{
						if ( script[0] == "req_encyclo_thema" )
							Prerequisits.EncycloReqTaskDone = true;
						else
							Prerequisits.EncycloReqTaskDone = false;
						NLMISC::fromString(vars[0], Prerequisits.EncycloReqAlbum);
						NLMISC::fromString(vars[1], Prerequisits.EncycloReqThema);
					}
				}
			}

			// event missions can require player to belong to a particular event faction
			else if ( script[0] == "req_event_faction" )
			{
				ret = (script.size() == 2);
				if (ret)
					Prerequisits.EventFaction = CMissionParser::getNoBlankString( script[1] );
			}
			// character oldness check loading
			else if ( script[0] == "req_character_age" )
			{
				sint age;
				ret = parseInt( i+1, script,age ) && ret;
				Prerequisits.CharacterMinAge = (uint32)age;
				if ( Prerequisits.CharacterMinAge < 0 )
				{
					MISLOGERROR1("character minimum age is %d. Must be >= 0", Prerequisits.CharacterMinAge );
					ret = false;
				}
			}
			// maximum player ID check loading
			else if ( script[0] == "req_max_player_id" )
			{
				sint max_id;
				ret = parseInt( i+1, script,max_id ) && ret;
				Prerequisits.MaxPlayerID = (uint32)max_id;
				if ( Prerequisits.MaxPlayerID < 0 )
				{
					MISLOGERROR1("Maximum player ID is %u. Must be >= 0", Prerequisits.MaxPlayerID );
					ret = false;
				}
			}
			// update next step step text
			else if ( script[0] == "set_obj" )
			{
				setObjInOOO = ( outOfOrderStep != 0xFFFFFFFF );
				ret = parseMissionText( i+1, script, overridenText, overridenParams) && ret;
			}
			// update next step step roleplay text
			else if ( script[0] == "set_obj_rp" )
			{
				ret = parseMissionText( i+1, script, roleplayText, roleplayParams) && ret;
			}
			else if ( script[0] == "hide_obj" )
				displayNextStep = false;
			else if ( script[0] == "hide_icon_on_step_npc" )
				displayNextIconOnStepNPC = false;

			// parse parent mission
			else if ( script[0] == "parent" )
			{
				if ( script.size() != 2 )
				{
					MISLOGSYNTAXERROR("<mission>");
					ret = false;
				}
				else
				{
					CMissionParser::removeBlanks(script[1]);
					TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( script[1] );
					if ( alias == CAIAliasTranslator::Invalid )
					{
						MISLOGERROR1("invalid parent mission '%s'", script[1].c_str());
						ret = false;
					}
					else
					{
						globalData.ParentMissions.push_back( std::make_pair(alias,Alias) );
					}
				}
			}
			

			// check if it is an out of order begin markup
			else if ( script[0] == "ooo" )
			{
				oooIndex++;
				afterOOO = false;
				if ( !openTag.empty() )
				{
					MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
					ret = false;
				}
				outOfOrderStep = currentStep + 1;
				openTag = "ooo";
				if ( !overridenText.empty() )
					oooDisplayed = false;
			}
			else if ( script[0] == "/ooo" )
			{
				afterOOO = true;
				if ( outOfOrderStep == 0xFFFFFFFF || openTag!="ooo")
				{
					MISLOGERROR("markup '/ooo' dont close a 'ooo' markup");
					ret = false;
				}
				else
				{
					OutOfOrderSteps.push_back( std::make_pair( outOfOrderStep, currentStep ) );
					if ( outOfOrderStep >= Steps.size() || currentStep >= Steps.size() )
					{
						MISLOGERROR("error inside an OOO -> the OOO is completly broken");
						ret = false;
					}
					else
					{
						for  (uint i = outOfOrderStep; i <= currentStep; i++ )
							Steps[i]->setOOOStepIndex(oooIndex);
					}
					outOfOrderStep = 0xFFFFFFFF;
				}
				openTag.clear();
				oooDisplayed = true;
			}
			
			else if ( script[0] == "any" )
			{
				oooIndex++;
				afterOOO = false;
				if ( !openTag.empty() )
				{
					MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
					ret = false;
				}
				outOfOrderStep = currentStep + 1;
				openTag = "any";
				if ( !overridenText.empty() )
					oooDisplayed = false;
			}
			else if ( script[0] == "/any" )
			{
				afterOOO = true;
				if ( outOfOrderStep == 0xFFFFFFFF || openTag!="any")
				{
					MISLOGERROR("markup '/any' dont close a 'any' markup");
					ret = false;
				}
				else
				{
					OutOfOrderSteps.push_back( std::make_pair( outOfOrderStep, currentStep ) );
					if ( outOfOrderStep >= Steps.size() || currentStep >= Steps.size() )
					{
						MISLOGERROR("error inside an OOO -> the OOO is completly broken");
						ret = false;
					}
					else
					{
						for  (uint i = outOfOrderStep; i <= currentStep; i++ )
						{
							Steps[i]->setOOOStepIndex(oooIndex);
							Steps[i]->setAsAny();
						}
					}
					outOfOrderStep = 0xFFFFFFFF;
				}
				openTag.clear();
				oooDisplayed = true;
			}

			// jump label
			else if ( script[0] == "jump_point" )
			{
				if ( script.size() != 2 )
				{
					MISLOGSYNTAXERROR("<jump_label>");
					ret = false;
				}
				else
				{
					if ( !failure )
						JumpPoints.push_back( CJumpPoint( CMissionParser::getNoBlankString(script[1] ), currentStep,currentAction ) );
					else
					{
						MISLOGERROR("jump_point in a failure node!");
						ret = false;
					}
				}
			}

			// check for the end of a failure tag
			else if ( script[0] == "/failure" )
			{
				failure = false;
				if ( openTag!="failure")
				{
					MISLOGERROR("markup '/failure' dont close a 'failure' markup");
					ret = false;
				}
				openTag.clear();
			}
			else if (script[0] == "/crash")
			{
				crashHandler = false;
				if ( openTag!="crash")
				{
					MISLOGERROR("markup '/crash' dont close a 'crash' markup");
					ret = false;
				}
				openTag.clear();
			}
			else if (script[0] == "/player_reconnect")
			{
				playerReconnectHandler = false;
				if (openTag != "player_reconnect")
				{
					MISLOGERROR("markup '/player_reconnect' dont close a 'player_reconnect' markup");
					ret = false;
				}
				openTag.clear();
			}
			/////////////////////////////// STEPS AND ACTIONS///////////////////////////////
			else
			{
				// try to build a step
				IMissionStepTemplate * step = IMissionStepTemplateFactory::buildStep( i+1, script , globalData, missionData);
				if ( step )
				{
					afterOOO = false;
					if ( !overridenText.empty() )
					{
						// default params are kept in the overriden text if we are not in an OOO / ANY block or if the set obj was done inside such a block
						step->overrideTexts( overridenText,overridenParams, outOfOrderStep == 0xFFFFFFFF || setObjInOOO );
						overridenText.clear();
					}

					// for ooo step we duplicate roleplay text for each substep(but only one instance will be displayed)
					if( outOfOrderStep != 0xFFFFFFFF )
					{
						if ( !roleplayTextOOO.empty() )
						{
							step->setRoleplayText( roleplayTextOOO,roleplayParamsOOO );
						}
					}
					else
					{
						roleplayTextOOO.clear();
					}
					if ( !roleplayText.empty() )
					{
						step->setRoleplayText( roleplayText,roleplayParams);
						if( outOfOrderStep != 0xFFFFFFFF )
						{
							roleplayTextOOO = roleplayText;
							roleplayParamsOOO = roleplayParams;
						}
						roleplayText.clear();
					}
					
					if ( displayNextStep == false )
						step->setDisplayed( false );
					if ( displayNextIconOnStepNPC == false )
						step->setIconDisplayedOnStepNPC( false );
						/*	if ( ( oooDisplayed == false && outOfOrderStep != (currentStep + 1) ) ||
						( displayNextStep == false ) )
						step->displayed(false);
					*/
					if ( oooDisplayed == false )
						step->setAsInOverridenOOO();
					
					currentAction = 0;
					currentStep++;
					Steps.push_back(step);
					displayNextStep = true;
					displayNextIconOnStepNPC = true;
					setObjInOOO = false;
					continue;
				}
				
				// do special operations for reward blocks before parsing them as action
				else if ( script[0] == "reward" )
				{
					if ( !openTag.empty() )
					{
						MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
						ret = false;
					}
					rewardTag = true;
					openTag = "reward";
				}
			
				// try to build an action
				IMissionAction * action = IMissionActionFactory::buildAction(i+1, script, globalData, missionData );
				if ( action )
				{
					if ( failure )
					{
						if ( failureIndex >= FailureActions.size()	)
							FailureActions.resize( failureIndex + 1 );
						FailureActions[failureIndex].push_back(action);
					}
					else if ( crashHandler )
					{
						nlassert ( crashHandlerIndex < CrashHandlers.size()	);
						CrashHandlers[crashHandlerIndex].Actions.push_back(action);
					}
					else if ( playerReconnectHandler )
					{
						nlassert ( playerReconnectHandlerIndex < PlayerReconnectHandlers.size()	);
						PlayerReconnectHandlers[playerReconnectHandlerIndex].Actions.push_back(action);
					}
					else 
					{
						if (afterOOO)
						{
							if( oooIndex == 0xFFFFFFFF )
							{
								MISLOGERROR("ooo tags are not properly enclosed");
								ret = false;
							}
							if( oooIndex >= OOOActions.size() )
								OOOActions.resize( oooIndex + 1 );
							OOOActions[oooIndex].push_back( action );
						}
						else
						{
							if ( currentStep == 0xFFFFFFFF )
							{
								InitialActions.push_back(action);
							}
							else
							{
								nlassert(currentStep < Steps.size() );
								Steps[currentStep]->addAction(action);
							}
						}
						currentAction++;
					}
					// check if it is a failure markup. we put it there as it is also an action
					if ( script[0] == "failure" )
					{
						failureIndex++;
						((CMissionActionSetFailureIndex*)action)->setIndex( failureIndex );
						if ( !openTag.empty() )
						{
							MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
							ret = false;
						}
						openTag = "failure";
						failure = true;
						continue;
					}
					// check if it is a crash handler
					else if ( script[0] == "crash" )
					{
						crashHandlerIndex++;
						((CMissionActionSetCrashHandlerIndex*)action)->setIndex( crashHandlerIndex );
						if ( !openTag.empty() )
						{
							MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
							ret = false;
						}
						CrashHandlers.resize( crashHandlerIndex + 1 );
						for ( uint p = 1; p < script.size(); ++p )
							CrashHandlers[crashHandlerIndex].AIInstances.push_back(  script[p] );
						openTag = "crash";
						crashHandler = true;
						continue;
					}
					// check if it is a 'player reconnect' handler
					else if (script[0] == "player_reconnect")
					{
						playerReconnectHandlerIndex++;
						((CMissionActionSetPlayerReconnectHandlerIndex*)action)->setIndex( playerReconnectHandlerIndex );
						if ( !openTag.empty() )
						{
							MISLOGERROR1("markup '%s' not closed before another markup", openTag.c_str());
							ret = false;
						}
						PlayerReconnectHandlers.resize( playerReconnectHandlerIndex + 1 );
						openTag = "player_reconnect";
						playerReconnectHandler = true;
						continue;
					}
					// check if it is a chain mission. If So add an end label
					else if ( script[0] == "chain_mission" )
					{
						vector<string>::iterator it = itParams;
						++it;
						params->insert( it,"end" );
					}
					continue;
				}
				MISLOG("sline:%u ERROR : invalid content '%s'",i+1, (*params)[i].c_str());
				ret = false;
			}
		}
	}
	// check steps texts
	if ( !Tags.NoList)
	{
		for ( uint i = 0; i < Steps.size();i++ )
		{
			if ( !Steps[i]->checkTextConsistency() )
				ret = false;
		}
	}

	for (uint i = 0; i < JumpPoints.size();++i)
	{
		for (uint j = i+1; j < JumpPoints.size();++j)
		{
			if ( JumpPoints[i].Name == JumpPoints[j].Name )
			{
				MISLOG("ERROR jump_point '%s' exists more than once", JumpPoints[i].Name.c_str());
				ret = false;
			}
		}
		if ( std::find( missionData.Jumps.begin(), missionData.Jumps.end(),JumpPoints[i].Name  ) == missionData.Jumps.end() )
		{
			MISLOG("ERROR jump_point %s not used",JumpPoints[i].Name.c_str());
			ret = false;
		}
	}
	for (uint i = 0; i < missionData.Jumps.size();++i)
	{
		uint j = 0;
		for (; j < JumpPoints.size();++j)
		{
			if ( missionData.Jumps[i] == JumpPoints[j].Name )
				break;
		}
		if ( j ==  JumpPoints.size())
		{
			MISLOG("ERROR jump '%s' not found in jump_point list", missionData.Jumps[i].c_str());
			ret = false;
		}
	}
	
	if ( TitleText.empty() && AutoText.empty() && !Tags.NotProposed )
	{
		MISLOG("ERROR : empty title");
		ret = false;
	}
	if ( DescText.empty() && AutoText.empty() && !Tags.NotProposed)
	{
		MISLOG("ERROR : empty desc");
		ret = false;
	}
/*
	if ( Steps.empty() && ret)
	{
		string name;
		prim->getPropertyByName("name",name);

//		MISLOG("<MISSIONS> WARNING mission '%s' alias %u: No steps\n", name.c_str(), Alias);
//		ret = true;
	}*/
/*
	if ( Steps.size() > NB_STEP_PER_MISSION )
	{
		MISLOG("<MISSIONS> : MAX = %u steps in a mission. Code must be updated if larger missions are required",NB_STEP_PER_MISSION);
		ret = false;
	}
	*/
/*
	for ( uint i = 0; i < chatParams.size(); i++ )
	{
		nldebug("registered param '%s' as '%s'",chatParams[i].first.c_str(),STRING_MANAGER::paramTypeToString(chatParams[i].second).c_str() );
	}
*/
	// solve texts params
	if ( !CMissionParser::solveTextsParams( 0, TitleParams, missionData ) )
	{
		MISLOG("ERROR parsing title parameters");
		ret = false;
	}
	if ( !CMissionParser::solveTextsParams( 0, DescParams, missionData ) )
	{
		MISLOG("ERROR parsing description parameters");
		ret = false;
	}
	if ( !CMissionParser::solveTextsParams( 0, AutoParams, missionData ) )
	{
		MISLOG("ERROR parsing auto parameters parameters");
		ret = false;
	}

	for ( uint i = 0; i < InitialActions.size(); i++ )
	{
		if ( !InitialActions[i]->solveTextsParams( missionData ) )
			ret = false;
	}
	for ( uint i = 0; i < Steps.size(); i++ )
	{
		if ( !Steps[i]->solveTextsParams( missionData, this ) )
			ret = false;
	}

	for ( uint i = 0; i < OOOActions.size(); i++ )
	{
		for ( uint j = 0; j < OOOActions[i].size(); j++ )
		{
			if ( !OOOActions[i][j]->solveTextsParams( missionData ) )
				ret = false;
		}
	}

	for ( uint i = 0; i < OverloadedDescs.size(); i++ )
	{
		if ( !CMissionParser::solveTextsParams( i, OverloadedDescs[i].Params, missionData ) )
			ret = false;
	}

	return ret;
}// CMissionTemplate build

// -------------------------------------------------------------------------------------------
static void getDHMS(uint32 nSecondsIN, uint32 &nDays, uint32 &nHours, uint32 &nMinutes, uint32 &nSeconds)
{
	uint32 seconds = nSecondsIN;
	nMinutes = uint32(seconds/60);
	nHours = uint32(nMinutes/60);
	nDays = uint32(nHours/24);
	seconds = seconds%60;
	nMinutes = nMinutes%60;
	nHours = nHours%24;
	if (seconds > 0)
	{
		++nMinutes;
		if (nMinutes == 60)
		{
			--nMinutes;
			++nHours;
			if (nHours == 24)
			{
				--nHours;
				++nDays;
			}
		}
	}
	nSeconds = seconds;
}

// -------------------------------------------------------------------------------------------
void	CMissionTemplate::addMissionPrerequisitInfo(CPrerequisitInfos &prereqInfos, const char *reqText, CCharacter *user, const std::vector<std::string> &missionReqOr, bool testAutoForbidden, set<string> &addedPrereqTexts) const
{
	if(!user)
		return;

	CPrerequisitDesc prereqDesc;
	
	// Must test for auto-forbidden mission?
	if(testAutoForbidden)
	{
		// If only one of the forbidden mission is this, then display a single message
		for(uint j=0;j<missionReqOr.size();j++)
		{
			TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( missionReqOr[j] );
			const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
			if ( templ == this )
			{
				// only if not already done
				if (addedPrereqTexts.find("MISSION_PREREQ_CAN_NO_LONGER_TAKE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_CAN_NO_LONGER_TAKE",TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_CAN_NO_LONGER_TAKE");
				}
				
				// and do nothing else
				return;
			}
		}
	}

	// Add a line for all missions that failed (ORed)
	bool	mandatory= true;
	for(uint j=0;j<missionReqOr.size();j++)
	{
		TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( missionReqOr[j] );
		const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
		// just skip line in case of error (btw should not be here)
		if ( !templ )
			continue;
			
		// Get the mission title
		TDataSetRow giverRow = TDataSetRow();
		// try to get the giver id
		if(_DefautltNpcGiverAlias != CAIAliasTranslator::Invalid)
		{
			NLMISC::CEntityId giverEid= CAIAliasTranslator::getInstance()->getEntityId(templ->getDefaultNpcGiver());
			giverRow= TheDataset.getDataSetRow(giverEid);
		}
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::dyn_string_id);
		params[0].StringId = templ->sendTitleText(user->getEntityRowId(), giverRow);
		
		// Add the prerequesit string
		prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), reqText, params);
		prereqDesc.IsMandatory = mandatory;	// First is mandatory, other are OR
		mandatory= false;
		prereqDesc.Validated = false;
		prereqInfos.Prerequisits.push_back(prereqDesc);
	}
}

// -------------------------------------------------------------------------------------------
uint32 CMissionTemplate::testPrerequisits( CCharacter * user, CPrerequisitInfos &prereqInfos, bool logOnFail, bool fillPrereqInfos ) const
{
	// NB if fillPrereqInfos is false then return as soon as a test fails, otherwise make all tests (except cases when mission should not appear on client side)

	// keep prereq texts, just to be sure we won't show twice the same message
	set<string> addedPrereqTexts;

	uint32 returnValue = MISSION_DESC::PreReqSuccess;
	CPrerequisitDesc prereqDesc;
	prereqInfos.Prerequisits.clear();

	 // get the real mission alias ( to take ref Mission into account )
	CMissionTemplate * mt = CMissionManager::getInstance()->getTemplate( Alias );
	nlassert(mt);
	TAIAlias alias = mt->Alias;

	string sDebugPrefix = "user:" + user->getId().toString() + " miss:" + CPrimitivesParser::aliasToString(alias);
	sDebugPrefix += ",'" + mt->getMissionName() + "' testPrerequisits :";	

	/// todo guild mission
	//CGuild * guild = user->getGuild();
	CGuild * guild = NULL;

	CTeam * team = TeamManager.getRealTeam(user->getTeamId());
	// check doneOnce flag
	if ( MissionPrerequisitsEnabled && Tags.DoneOnce && AlreadyDone)
	{
		if (logOnFail)
			MISDBG("%s Mission already done (once mission)", sDebugPrefix.c_str());

		if (fillPrereqInfos)
		{
			prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
			prereqDesc.IsMandatory = true;
			prereqInfos.Prerequisits.push_back(prereqDesc);

			addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
		}
			
		// always stop tests here
		return MISSION_DESC::PreReqFailAlreadyDone;
	}

	// test global replay timer
	if (MissionPrerequisitsEnabled && GlobalReplayTimer != 0 && LastSuccessDate != 0 && LastSuccessDate+GlobalReplayTimer > CTickEventHandler::getGameCycle())
	{
		if (logOnFail)
			MISDBG("%s Global replay timer not elapsed", sDebugPrefix.c_str());
		
		if (!fillPrereqInfos)
			return MISSION_DESC::PreReqFail;

		uint32 days, hours, minutes, seconds;
		getDHMS(uint32((LastSuccessDate+GlobalReplayTimer - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep()),
				days, hours, minutes, seconds);

		SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Int = days;
		params[1].Int = hours;
		params[2].Int = minutes;
		prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TIMER", params);
		prereqDesc.IsMandatory = true;
		prereqDesc.Validated = false;
		prereqInfos.Prerequisits.push_back(prereqDesc);

		returnValue = MISSION_DESC::PreReqFail;
		logOnFail = false;
	}

	// test player replay timer
	const TGameCycle lastSuccess = user->getMissionLastSuccess(*mt);
	if (MissionPrerequisitsEnabled && PlayerReplayTimer != 0 && lastSuccess != 0 && lastSuccess+PlayerReplayTimer > CTickEventHandler::getGameCycle())
	{
		if (logOnFail)
			MISDBG("%s Player replay timer not elapsed", sDebugPrefix.c_str());
		
		if (!fillPrereqInfos)
			return MISSION_DESC::PreReqFail;

		uint32 days, hours, minutes, seconds;
		getDHMS(uint32((lastSuccess+PlayerReplayTimer - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep()),
			days, hours, minutes, seconds);

		SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Int = days;
		params[1].Int = hours;
		params[2].Int = minutes;
		prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TIMER", params);
		prereqDesc.IsMandatory = true;
		prereqDesc.Validated = false;
		prereqInfos.Prerequisits.push_back(prereqDesc);

		returnValue = MISSION_DESC::PreReqFail;
		logOnFail = false;
	}

	// special tests for team missions
	if ( Type == MISSION_DESC::Group )
	{
		CTeam * team = TeamManager.getRealTeam(user->getTeamId());
		if ( ! team )
		{
			if (logOnFail)
				MISDBG("%s No team", sDebugPrefix.c_str());
			
			if (!fillPrereqInfos)
				return MISSION_DESC::PreReqFail;

			prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TEAM", TVectorParamCheck());
			prereqDesc.IsMandatory = true;
			prereqDesc.Validated = false;
			prereqInfos.Prerequisits.push_back(prereqDesc);

			addedPrereqTexts.insert("MISSION_PREREQ_TEAM");

			returnValue = MISSION_DESC::PreReqFail;
			logOnFail = false;
		}
		else
		{
			if ( team->getTeamSize() < 2 )
			{
				if (logOnFail)
					MISDBG("%s Team size < 2");
				
				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFail;

				if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TEAM", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_TEAM");
				}
				returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;
			}
			if ( team->getLeader() != user->getId() )
			{
				if (logOnFail)
					MISDBG("%s Not leader", sDebugPrefix.c_str());
				
				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFail;

				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TEAM_LEADER", TVectorParamCheck());
				prereqDesc.IsMandatory = true;
				prereqDesc.Validated = false;
				prereqInfos.Prerequisits.push_back(prereqDesc);

				returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;
			}
			// check if the mission is already picked
			for ( uint j  = 0 ; j < team->getMissions().size(); j++ )
			{
				if  ((team->getMissions()[j]->getTemplateId() == alias) || 
					 (team->getMissions()[j]->getMainMissionTemplateId() == alias))
				{
					if (logOnFail)
						MISDBG("%s The team already own this mission", sDebugPrefix.c_str());
					
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
					{
						prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
						prereqDesc.IsMandatory = true;
						prereqDesc.Validated = false;
						prereqInfos.Prerequisits.push_back(prereqDesc);
						addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
					}
					
					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
			}
		}
	}
	else if ( Type == MISSION_DESC::Guild )
	{
		/// todo guild mission
		
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
		if ( guild == NULL )
		{
			MISDBG("No guild");
			return MISSION_DESC::PreReqFail;

			/*if (logOnFail)
				MISDBG("%s No guild", sDebugPrefix.c_str());

			if (!fillPrereqInfos)
				return MISSION_DESC::PreReqFail;

			prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TEAM", TVectorParamCheck());
			prereqDesc.IsMandatory = true;
			prereqDesc.Validated = false;
			prereqInfos.Prerequisits.push_back(prereqDesc);

			addedPrereqTexts.insert("MISSION_PREREQ_TEAM");

			returnValue = MISSION_DESC::PreReqFail;
			logOnFail = false;*/
		}
		// check if the mission is already picked
		for ( uint j  = 0 ; j < guild->getMissions().size(); j++ )
		{
			if  ( guild->getMissions()[j]->getTemplateId() == alias ||
					guild->getMissions()[j]->getMainMissionTemplateId() == alias)
			{
				MISDBG("The guild already own this mission");
				return MISSION_DESC::PreReqFail;

				/*if (logOnFail)
					MISDBG("%s The guild already own this mission", sDebugPrefix.c_str());

				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFail;

				if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
				}

				returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;*/
			}
		}
		// check non replayable missions
		if( !Tags.Replayable )
		{
			if (guild->isMissionSuccessfull(alias))
				//		if ( std::find(guild->getSuccessfulMissions().begin(),guild->getSuccessfulMissions().end(), alias) != guild->getSuccessfulMissions().end() )
			{
				MISDBG("mission non replayable");
				return MISSION_DESC::PreReqFail;

				/*if (logOnFail)
					MISDBG("%s Guild mission already done and not replayable", sDebugPrefix.c_str());

				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFailAlreadyDone;

				if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
				}

				returnValue = MISSION_DESC::PreReqFailAlreadyDone;
				logOnFail = false;*/
			}
		}
	}
	else
	{
		// check if the mission is already picked
		for ( map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
		{
			CMission *pMission = it->second;
			// If the mission is the same or if the main mission (in case of a chain) is the same 
			// dont let player get this mission twice
			if  ((pMission->getTemplateId() == alias) || (pMission->getMainMissionTemplateId() == alias))
			{
				if (logOnFail)
					MISDBG("%s The player already own this mission", sDebugPrefix.c_str());
				
				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFailRunning;
				
				if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
				}
				
				returnValue = MISSION_DESC::PreReqFailRunning;
				logOnFail = false;
			}
		}
		// check non replayable missions
		if(MissionPrerequisitsEnabled && !Tags.Replayable )
		{
			if (user->isMissionSuccessfull(*mt))
			{
				if (logOnFail)
					MISDBG("%s Solo mission already done and not replayable", sDebugPrefix.c_str());

				if (!fillPrereqInfos)
					return MISSION_DESC::PreReqFailAlreadyDone;
				
				if (addedPrereqTexts.find("MISSION_PREREQ_ALREADY_DONE") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ALREADY_DONE", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqDesc.Validated = false;
					prereqInfos.Prerequisits.push_back(prereqDesc);
					addedPrereqTexts.insert("MISSION_PREREQ_ALREADY_DONE");
				}
				
				returnValue = MISSION_DESC::PreReqFailAlreadyDone;
				logOnFail = false;
			}
		}
	}


	// check needed missions
	if (MissionPrerequisitsEnabled)
	{
		for (uint i = 0; i < Prerequisits.NeededMissions.size(); i++ )
		{
			uint j = 0;
			for (; j < Prerequisits.NeededMissions[i].Missions.size(); j++ )
			{
				TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( Prerequisits.NeededMissions[i].Missions[j] );
				const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
				if ( !templ )
				{
					if (logOnFail)
						MISDBG("%s ERROR invalid mission alias %u", sDebugPrefix.c_str(), alias2);
					
					// always stop tests here
					return MISSION_DESC::PreReqFail;
				}
				if ( templ->Type == MISSION_DESC::Guild )
				{
					/// todo guild mission
					CGuild* guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
					if  ( ! guild )
					{
						MISDBG("Require needed mission at line %u (guild mission but player has no guild)", Prerequisits.NeededMissions[i].Line);
						return MISSION_DESC::PreReqFail;
					}
					if (guild->isMissionSuccessfull(templ->Alias))
						break;
				}
				else if ( templ->Type == MISSION_DESC::Solo )
				{
					if (user->isMissionSuccessfull(*templ))
						break;
				}
			}
			if ( j ==  Prerequisits.NeededMissions[i].Missions.size() )
			{
				if (logOnFail)
					MISDBG("%s Require needed mission at line %u", sDebugPrefix.c_str(), Prerequisits.NeededMissions[i].Line);
				
				if (fillPrereqInfos)
				{
					// Add all mission requirement to the info
					addMissionPrerequisitInfo(prereqInfos, "MISSION_PREREQ_MISSION_DONE", 
						user, Prerequisits.NeededMissions[i].Missions, false, addedPrereqTexts);
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
						returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				else
					return MISSION_DESC::PreReqFail;
			}
		}
			
		// check forbidden missions
		for (uint i = 0; i < Prerequisits.ForbiddenMissions.size(); i++ )
		{
			uint j = 0;
			for (; j < Prerequisits.ForbiddenMissions[i].Missions.size(); j++ )
			{
				TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( Prerequisits.ForbiddenMissions[i].Missions[j] );
				const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
				if ( !templ )
				{
					if (logOnFail)
						MISDBG("%s Cant Find Alias %s", sDebugPrefix.c_str(), CPrimitivesParser::aliasToString(alias2).c_str());
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}
				}
				else if ( templ->Type == MISSION_DESC::Guild )
				{
					/// todo guild mission
					CGuild* guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
					if  ( !guild )
						break;
					if (!guild->isMissionSuccessfull(templ->Alias))
						break;
					/*if (!guild || !guild->isMissionSuccessfull(templ->Alias))
						break;*/
				}
				else if ( templ->Type == MISSION_DESC::Solo )
				{
					if (!user->isMissionSuccessfull(*templ))
						break;
				}
			}
			if ( j ==  Prerequisits.ForbiddenMissions[i].Missions.size() )
			{
				if (logOnFail)
					MISDBG("%s Require forbidden mission at line %u", sDebugPrefix.c_str(), Prerequisits.ForbiddenMissions[i].Line);
				
				if (fillPrereqInfos)
				{
					// Add all mission requirement to the info. If there is a mission requirement that is the current one,
					// display a single message "MISSION_PREREQ_CAN_NO_LONGER_TAKE" only one time
					addMissionPrerequisitInfo(prereqInfos, "MISSION_PREREQ_MISSION_NOTDONE", 
						user, Prerequisits.ForbiddenMissions[i].Missions, true, addedPrereqTexts);
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
						returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				else
					return MISSION_DESC::PreReqFail;
			}
		}
	}			
	
	// check needed running missions
	for (uint i = 0; i < Prerequisits.RunningMissions.size(); i++ )
	{
		uint j = 0;
		for (; j < Prerequisits.RunningMissions[i].Missions.size(); j++ )
		{
			TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( Prerequisits.RunningMissions[i].Missions[j] );
			const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
			if ( !templ )
			{
				if (logOnFail)
					MISDBG("%s Cant Find Alias %s", sDebugPrefix.c_str(), CPrimitivesParser::aliasToString(alias2).c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
			}
			if ( templ->Type == MISSION_DESC::Guild )
			{

				/// todo guild mission
				CGuild* guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
				if ( !guild )
				{
					MISDBG("Require running mission at line %u (guild mission but player has no guild)", Prerequisits.RunningMissions[i].Line );
					return MISSION_DESC::PreReqFail;
				}
				uint k=0;

				for ( ; k < guild->getMissions().size(); k++ )
				{
					if  ( guild->getMissions()[k]->getTemplateId() == templ->Alias )
						break;
				}
				if (k != guild->getMissions().size())
					break;
			}
			else if ( templ->Type == MISSION_DESC::Solo )
			{
				map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin();
				for ( ; it != user->getMissionsEnd(); ++it )
				{
					if  ( (*it).second->getTemplateId() == templ->Alias )
						break;
				}
				if ( it != user->getMissionsEnd() )
					break;
			}
			else if ( templ->Type == MISSION_DESC::Group)
			{
				if (!team)
				{
					if (logOnFail)
						MISDBG("%s Require running mission at line %u (team mission but player has no team)", sDebugPrefix.c_str(), Prerequisits.RunningMissions[i].Line );
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}
				}
				else
				{

					uint k = 0;
					for ( ; k < team->getMissions().size(); k++ )
					{
						if  ( team->getMissions()[k]->getTemplateId() == templ->Alias )
							break;
					}
					if (k != team->getMissions().size())
						break;
				}
			}
		}
		if ( j == Prerequisits.RunningMissions[i].Missions.size() )
		{
			if (logOnFail)
				MISDBG("%s Require running mission at line %u", sDebugPrefix.c_str(), Prerequisits.RunningMissions[i].Line );
			
			if (fillPrereqInfos)
			{
				// Add all mission requirement to the info
				addMissionPrerequisitInfo(prereqInfos, "MISSION_PREREQ_MISSION_RUNNING", 
					user, Prerequisits.RunningMissions[i].Missions, false, addedPrereqTexts);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
					returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;
			}
			else
				return MISSION_DESC::PreReqFail;
		}
	}
	
	// check forbidden running missions
	for (uint i = 0; i < Prerequisits.ForbiddenRunningMissions.size(); i++ )
	{
		prereqDesc.Validated = true;
		
		uint j = 0;
		bool ok = true;
		for (; j < Prerequisits.ForbiddenRunningMissions[i].Missions.size(); j++ )
		{
			TAIAlias alias2 = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( Prerequisits.ForbiddenRunningMissions[i].Missions[j] );
			const CMissionTemplate* templ = CMissionManager::getInstance()->getTemplate( alias2 );
			if ( !templ )
			{
				if (logOnFail)
					MISDBG("%s Cant Find Alias %s", sDebugPrefix.c_str(), CPrimitivesParser::aliasToString(alias2).c_str());
				
				if (fillPrereqInfos)
				{
					// Add all mission requirement to the info. If there is a mission requirement that is the current one,
					// display a single message "MISSION_PREREQ_CAN_NO_LONGER_TAKE" only one time
					addMissionPrerequisitInfo(prereqInfos, "MISSION_PREREQ_MISSION_NOTRUNNING", 
						user, Prerequisits.ForbiddenRunningMissions[i].Missions, true, addedPrereqTexts);
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
						returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				else
					return MISSION_DESC::PreReqFail;
			}

			if ( templ->Type == MISSION_DESC::Guild )
			{
				/// todo guild mission
				CGuild* guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
				if(!guild)
					break;
				uint k=0;

				for ( ; k < guild->getMissions().size(); k++ )
				{
					if  ( guild->getMissions()[k]->getTemplateId() == templ->Alias )
						break;
				}
				if (k == guild->getMissions().size())
					break;
			}
			else if ( templ->Type == MISSION_DESC::Solo )
			{
				map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin();
				for ( ; it != user->getMissionsEnd(); ++it )
				{
					if  ( (*it).second->getTemplateId() == templ->Alias )
						break;
				}
				if ( it == user->getMissionsEnd() )
					break;
			}
			else if ( templ->Type == MISSION_DESC::Group)
			{
				if (!team)
					break;
				uint k=0;
				for ( ; k < team->getMissions().size(); k++ )
				{
					if  ( team->getMissions()[k]->getTemplateId() == templ->Alias )
						break;
				}

				if (k == team->getMissions().size())
					break;
			}
		}
		if ( j == Prerequisits.ForbiddenRunningMissions[i].Missions.size() )
		{
			if (logOnFail)
				MISDBG("%s Require forbidden running mission at line %u", sDebugPrefix.c_str(), Prerequisits.ForbiddenRunningMissions[i].Line);
						
			if (fillPrereqInfos)
			{
				// Add all mission requirement to the info. If there is a mission requirement that is the current one,
				// display a single message "MISSION_PREREQ_CAN_NO_LONGER_TAKE" only one time
				addMissionPrerequisitInfo(prereqInfos, "MISSION_PREREQ_MISSION_NOTRUNNING", 
					user, Prerequisits.ForbiddenRunningMissions[i].Missions, true, addedPrereqTexts);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
					returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;
			}
			else
				return MISSION_DESC::PreReqFail;
		}
	}
	
	if (MissionPrerequisitsEnabled)
	{
		// check mission unique instance ("mono")
		if( MonoTimer && !Instances.empty() )
		{
			if (logOnFail)
				MISDBG("%s This mission is mono instance and already running somewhere", sDebugPrefix.c_str());
			
			if (fillPrereqInfos)
			{
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_MONO",TVectorParamCheck());
				prereqDesc.IsMandatory = true;
				prereqDesc.Validated = false;
				prereqInfos.Prerequisits.push_back(prereqDesc);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
					returnValue = MISSION_DESC::PreReqFail;
				logOnFail = false;
			}
			else
				return MISSION_DESC::PreReqFail;		
		}
		
		
		// test skills
		for ( uint i = 0; i < Prerequisits.Skills.size(); i++ )
		{
			uint j = 0;
			bool matchPrerequisit = false;
			for ( ; j < Prerequisits.Skills[i].size(); j++ )
			{
				sint32 val = user->getBestChildSkillValue(Prerequisits.Skills[i][j].Skill);
				if ( val >= Prerequisits.Skills[i][j].Min && val <= Prerequisits.Skills[i][j].Max )
				{
					matchPrerequisit = true;

					if (fillPrereqInfos)
						prereqDesc.Validated = true;
					else
						break;
				}
				else
				{
					if (fillPrereqInfos)
						prereqDesc.Validated = false;
				}

				if (fillPrereqInfos)
				{
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::skill, STRING_MANAGER::integer, STRING_MANAGER::integer);
					params[0].Enum = Prerequisits.Skills[i][j].Skill;
					params[1].Int = Prerequisits.Skills[i][j].Min;
					params[2].Int = Prerequisits.Skills[i][j].Max;
					if (j == 0)
						prereqDesc.IsMandatory = true;
					else
						prereqDesc.IsMandatory = false;

					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_SKILL_LEVEL", params);
					prereqInfos.Prerequisits.push_back(prereqDesc);
				}
			}

			if ( j ==  Prerequisits.Skills[i].size() && !matchPrerequisit )
			{
				if (logOnFail)
					MISDBG("%s Require skill at %uth skill line", sDebugPrefix.c_str(), i+1);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
			}
		}
		
		
		// check worn items
		for ( uint i = 0; i < Prerequisits.Wear.size(); i++ )
		{
			if( !user->doesWear( Prerequisits.Wear[i] ) )
			{
				if (logOnFail)
					MISDBG("%s Require wear item at %uth wear line", sDebugPrefix.c_str(), i+1);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			else
			{
				prereqDesc.Validated = true;
			}

			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
				params[0].SheetId = Prerequisits.Wear[i];
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_NEED_ITEM_WORN", params);
				prereqDesc.IsMandatory = true;

				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}
		// check item in bag
		for ( uint i = 0; i < Prerequisits.Own.size(); i++ )
		{
	//		const std::vector<CGameItemPtr> & equip = user->_Inventory[INVENTORIES::bag]->getChildren();
			const CInventoryPtr equip = user->getInventory(INVENTORIES::bag);
			uint j = 0;
	//		for (; j< equip.size(); j++)
			for (; j< equip->getSlotCount(); j++)
			{
	//			if(  equip[j] != NULL && equip[j]->getSheetId() == Prerequisits.Own[i] )
				if(  equip->getItem(j) != NULL && equip->getItem(j)->getSheetId() == Prerequisits.Own[i] )
					break;
			}
			if ( j == equip->getSlotCount() )
			{
				if (logOnFail)
					MISDBG("%s Require bag item at %uth req_item line", sDebugPrefix.c_str(), i);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}

				prereqDesc.Validated = false;
			}
			else
			{
				prereqDesc.Validated = true;
			}
			
			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
				params[0].SheetId = Prerequisits.Own[i];
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_NEED_ITEM_BAG", params);
				prereqDesc.IsMandatory = true;
				
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}
		// check title
		if ( Prerequisits.Title != CHARACTER_TITLE::NB_CHARACTER_TITLE )
		{
			if ( user->getTitle() !=  Prerequisits.Title )
			{
				if (logOnFail)
					MISDBG("%s Require title '%s' and user is '%s'", sDebugPrefix.c_str(), CHARACTER_TITLE::toString(Prerequisits.Title).c_str(), CHARACTER_TITLE::toString(user->getTitle()).c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			else
			{
				prereqDesc.Validated = true;
			}

			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::title);
				params[0].Identifier = CHARACTER_TITLE::toString(Prerequisits.Title);
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TITLE", params);
				prereqDesc.IsMandatory = true;
				
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}
		
		// check fame
		if ( Prerequisits.FameId )
		{
			sint32 fame = CFameInterface::getInstance().getFame(user->getId(), Prerequisits.FameId);
			if ( fame < Prerequisits.FameMin )
			{
				if (logOnFail)
					MISDBG("%s Require fame '%s' of %d and user has %d", sDebugPrefix.c_str(), CStringMapper::unmap(Prerequisits.FameId).c_str(), Prerequisits.FameMin, fame);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			else
			{
				prereqDesc.Validated = true;
			}

			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::faction, STRING_MANAGER::integer);
				params[0].Enum = CFameInterface::getInstance().getFactionIndex(Prerequisits.FameId);

				if ( float(Prerequisits.FameMin)/FameAbsoluteMax*100 - sint32(float(Prerequisits.FameMin)/FameAbsoluteMax*100 != 0.0f) )
					params[1].Int = 1 + sint32(float(Prerequisits.FameMin)/FameAbsoluteMax*100);
				else
					params[1].Int = sint32(float(Prerequisits.FameMin)/FameAbsoluteMax*100);

				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_FAME_FACTION", params);
				prereqDesc.IsMandatory = true;
				
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}

		// check kami / karavan
		if ( Prerequisits.KamiKaravan != CPrerequisits::None )
		{
			prereqDesc.Validated = true;

			NLMISC::TStringId strId = CStringMapper::map( "kami" );
			sint32 fameKami = CFameInterface::getInstance().getFame( user->getId(), strId);

			strId = CStringMapper::map( "karavan" );
			sint32 fameKaravan = CFameInterface::getInstance().getFame( user->getId(), strId);

			if ( Prerequisits.KamiKaravan == CPrerequisits::Kami && fameKaravan >= fameKami )
			{
				if (logOnFail)
					MISDBG("%s Require Kami", sDebugPrefix.c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			if ( Prerequisits.KamiKaravan == CPrerequisits::Karavan && fameKaravan <= fameKami )
			{
				if (logOnFail)
					MISDBG("%s Require Karavan", sDebugPrefix.c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			
			if (fillPrereqInfos)
			{
				if (Prerequisits.KamiKaravan == CPrerequisits::Karavan)
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_KARAVAN", TVectorParamCheck());
				else
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_KAMI", TVectorParamCheck());
				prereqDesc.IsMandatory = true;
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}

		// check guild and guild grade
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
		if (guild != NULL)
		{
			if (Prerequisits.GuildGrade != EGSPD::CGuildGrade::Unknown)
			{
				bool preReqFail = false;
				EGSPD::CGuildMemberPD * memberPD = guild->getMembers( user->getId() );
				if (memberPD != NULL)
				{
					if (memberPD->getGrade() > Prerequisits.GuildGrade)
					{
						if (logOnFail)
						{
							MISDBG("%s Require guild grade '%s' but player only has grade '%s'",
								sDebugPrefix.c_str(),
								EGSPD::CGuildGrade::toString(Prerequisits.GuildGrade).c_str(),
								EGSPD::CGuildGrade::toString(memberPD->getGrade()).c_str()
								);
						}
						preReqFail = true;
					}
				}
				else
				{
					if (logOnFail)
					{
						MISDBG("%s Invalid guild member %s he's not in his guild %u",
							sDebugPrefix.c_str(),
							user->getId().toString().c_str(),
							user->getGuildId()
							);
					}
					preReqFail = true;
				}

				if (preReqFail)
				{
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						/*if (addedPrereqTexts.find("GUILD_BUILDING_BAD_GRADE") == addedPrereqTexts.end())
						{
							prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "GUILD_BUILDING_BAD_GRADE", TVectorParamCheck());
							prereqDesc.IsMandatory = true;
							prereqDesc.Validated = false;
							prereqInfos.Prerequisits.push_back(prereqDesc);
							addedPrereqTexts.insert("GUILD_BUILDING_BAD_GRADE");
						}*/

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}

					prereqDesc.Validated = false;
				}
			}
		}
		else
		{
			if (Prerequisits.Guild || Prerequisits.GuildGrade != EGSPD::CGuildGrade::Unknown)
			{
				if (logOnFail)
					MISDBG("%s Require guild and player has no guild", sDebugPrefix.c_str());

				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}

				prereqDesc.Validated = false;
			}
		}

		// check team prerequisits
		if( Prerequisits.TeamSize )
		{
			prereqDesc.Validated = true;

			CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
			if (team == NULL)
			{
				if (logOnFail)
					MISDBG("%s Require team size of %d and player has no team", sDebugPrefix.c_str(), Prerequisits.TeamSize);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}

				prereqDesc.Validated = false;
			}
			else 
			{
				if (team->getTeamSize() < (uint16) Prerequisits.TeamSize )
				{
					if (logOnFail)
						MISDBG("%s Require team size of %d and player of team %d has team size of %d", sDebugPrefix.c_str(), 
							Prerequisits.TeamSize, user->getTeamId(), team->getTeamSize());
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}
					prereqDesc.Validated = false;
				}
			}

			if (fillPrereqInfos)
			{
				if (addedPrereqTexts.find("MISSION_PREREQ_TEAM_SIZE") == addedPrereqTexts.end())
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = (sint32) Prerequisits.TeamSize;
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_TEAM_SIZE", params);
					prereqDesc.IsMandatory = true;
					prereqInfos.Prerequisits.push_back(prereqDesc);
				}
			}
		}

		// check season prerequisits
		if (Prerequisits.Season != EGSPD::CSeason::Invalid)
		{
			prereqDesc.Validated = true;
			CRyzomTime::ESeason season = (MissionForcedSeason.get()==-1) ? CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason() : (CRyzomTime::ESeason)(uint)MissionForcedSeason.get();
			if (Prerequisits.Season != season)
			{
				if (logOnFail)
					MISDBG("%s Require season %s and ryzom season is %s", sDebugPrefix.c_str(), 
						EGSPD::CSeason::toString(Prerequisits.Season).c_str(),
						EGSPD::CSeason::toString(season).c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}

				prereqDesc.Validated = false;
			}

			if (fillPrereqInfos)
			{
				switch(Prerequisits.Season)
				{
				case EGSPD::CSeason::Spring:
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_SEASON_SPRING", TVectorParamCheck());
					break;
				case EGSPD::CSeason::Summer:
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_SEASON_SUMMER", TVectorParamCheck());
					break;
				case EGSPD::CSeason::Autumn:
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_SEASON_AUTUMN", TVectorParamCheck());
					break;
				case EGSPD::CSeason::Winter:
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_SEASON_WINTER", TVectorParamCheck());
					break;
				default:;
				};
				prereqDesc.IsMandatory = true;
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}

		// check known bricks
		for ( uint i = 0; i < Prerequisits.KnownActions.size(); i++ )
		{
			prereqDesc.Validated = true;

			std::set< CSheetId >::const_iterator it = user->getKnownBricks().find( Prerequisits.KnownActions[i]  );
			if ( it ==  user->getKnownBricks().end() )
			{
				if (logOnFail)
					MISDBG("%s Require bricks, player do not have %s", sDebugPrefix.c_str(), Prerequisits.KnownActions[i].toString().c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}

			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::sbrick);
				params[0].SheetId = Prerequisits.KnownActions[i];
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ACTION", params);
				prereqDesc.IsMandatory = true;
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}

		// check encyclopedia
		if (Prerequisits.EncycloReqAlbum != -1)
		{
			prereqDesc.Validated = true;

			CCharacterEncyclopedia &rEncy = user->getEncyclopedia();

			if (Prerequisits.EncycloReqTaskDone)
			{
				if (! rEncy.isAllTaskDone(Prerequisits.EncycloReqAlbum, Prerequisits.EncycloReqThema))
				{
					if (logOnFail)
						MISDBG("%s Require Encyclopedia all task done album %d thema %d", sDebugPrefix.c_str(), 
							Prerequisits.EncycloReqAlbum, Prerequisits.EncycloReqThema);
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}
					prereqDesc.Validated = false;
				}

				if (fillPrereqInfos)
				{
					const CStaticEncycloAlbum *album = CSheets::getEncyclopedia().getAlbum(Prerequisits.EncycloReqAlbum);
					const CStaticEncycloThema *thema = CSheets::getEncyclopedia().getThema(Prerequisits.EncycloReqAlbum, Prerequisits.EncycloReqThema);
					if (album && thema)
					{
						SM_STATIC_PARAMS_2(params, STRING_MANAGER::dyn_string_id, STRING_MANAGER::dyn_string_id);
						params[0].StringId = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), album->Title, TVectorParamCheck());
						params[1].StringId = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), thema->Title, TVectorParamCheck());
						prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_ENCYCLO_ALL_DONE", params);
						prereqDesc.IsMandatory = true;
						prereqInfos.Prerequisits.push_back(prereqDesc);
					}
				}
			}
			else
			{
				if (! rEncy.isAtLeastOneTaskNotDone(Prerequisits.EncycloReqAlbum, Prerequisits.EncycloReqThema))
				{
					if (logOnFail)
						MISDBG("%s Require Encyclopedia all at least one task not done album %d thema %d", sDebugPrefix.c_str(), 
							Prerequisits.EncycloReqAlbum, Prerequisits.EncycloReqThema);
					
					if (returnValue == MISSION_DESC::PreReqSuccess)
					{
						if (!fillPrereqInfos)
							return MISSION_DESC::PreReqFail;

						returnValue = MISSION_DESC::PreReqFail;
						logOnFail = false;
					}
					
					if (fillPrereqInfos)
					{
						if (addedPrereqTexts.find("MISSION_PREREQ_CAN_NO_LONGER_TAKE") == addedPrereqTexts.end())
						{
							prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_CAN_NO_LONGER_TAKE",TVectorParamCheck());
							prereqDesc.IsMandatory = true;
							prereqDesc.Validated = false;
							prereqInfos.Prerequisits.push_back(prereqDesc);
							addedPrereqTexts.insert("MISSION_PREREQ_CAN_NO_LONGER_TAKE");
						}
					}
				}
			}
		}

		// check event faction
		if ( !Prerequisits.EventFaction.empty() )
		{
			prereqDesc.Validated = true;
			
			if (user->getGameEvent().getEventFaction() != Prerequisits.EventFaction)
			{
				if (logOnFail)
					MISDBG("%s Require event faction %s", sDebugPrefix.c_str(), Prerequisits.EventFaction.c_str());
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						return MISSION_DESC::PreReqFail;

					prereqDesc.Validated = false;
					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
					prereqDesc.Validated = false;
				}
			}

			if (fillPrereqInfos)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::event_faction);
				params[0].Identifier = Prerequisits.EventFaction;
				prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_EVENT_FACTION", params);
				prereqDesc.IsMandatory = true;
				prereqDesc.Validated = true;
				prereqInfos.Prerequisits.push_back(prereqDesc);
			}
		}

		// check character minimum oldness
		if( Prerequisits.CharacterMinAge > 0 )
		{
			prereqDesc.Validated = true;

			uint32 minimumAge   = Prerequisits.CharacterMinAge * 24 * 3600; // oldness required is given in days
			uint32 characterAge = NLMISC::CTime::getSecondsSince1970() - user->getFirstConnectedTime();
			nlwarning("%s Require character age of %d days (%ds) and current character age is %d", sDebugPrefix.c_str(), 
						Prerequisits.CharacterMinAge, minimumAge, characterAge);

			if (characterAge < minimumAge)
			{
				if (logOnFail)
					MISDBG("%s Require character age of %d days (%ds) and current character age is %d", sDebugPrefix.c_str(), 
						Prerequisits.CharacterMinAge, minimumAge, characterAge);
					
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						{return MISSION_DESC::PreReqFail;}

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			if (fillPrereqInfos)
			{
				if (addedPrereqTexts.find("MISSION_PREREQ_CHARACTER_MIN_AGE") == addedPrereqTexts.end())
				{
					SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::integer);
					params[0].Int = (uint32) (characterAge / (24 * 3600) );
					params[1].Int = (uint32) Prerequisits.CharacterMinAge;
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_CHARACTER_MIN_AGE", params);
					prereqDesc.IsMandatory = true;
					prereqInfos.Prerequisits.push_back(prereqDesc);
				}
			}
		}

		// check maximum player ID
		if (Prerequisits.MaxPlayerID > 0)
		{
			prereqDesc.Validated = true;

			uint32 playerId = PlayerManager.getPlayerId(user->getId());
			nlwarning("%s Require player ID of %d and player's ID is %d", sDebugPrefix.c_str(), 
						Prerequisits.MaxPlayerID, playerId);

			if (playerId > Prerequisits.MaxPlayerID )
			{
				if (logOnFail)
					MISDBG("%s Require player ID of %d and player's ID is %d", sDebugPrefix.c_str(), 
						Prerequisits.MaxPlayerID, playerId);
				
				if (returnValue == MISSION_DESC::PreReqSuccess)
				{
					if (!fillPrereqInfos)
						{return MISSION_DESC::PreReqFail;}

					returnValue = MISSION_DESC::PreReqFail;
					logOnFail = false;
				}
				prereqDesc.Validated = false;
			}
			if (fillPrereqInfos)
			{
				if (addedPrereqTexts.find("MISSION_PREREQ_MAX_PLAYER_ID") == addedPrereqTexts.end())
				{
					prereqDesc.Description = STRING_MANAGER::sendStringToClient(user->getEntityRowId(), "MISSION_PREREQ_MAX_PLAYER_ID", TVectorParamCheck());
					prereqDesc.IsMandatory = true;
					prereqInfos.Prerequisits.push_back(prereqDesc);
				}
			}
		}
	}
	
	if (returnValue == MISSION_DESC::PreReqSuccess)
	{
		MISDBG("%s ok", sDebugPrefix.c_str());
	}
	
	return returnValue;
}// CMissionTemplate testPrerequisits


uint32 CMissionTemplate::sendTitleText( const TDataSetRow & userRow, const TDataSetRow & giver ) const
{
	TVectorParamCheck params(1 + TitleParams.size() );
	std::copy( TitleParams.begin(),TitleParams.end(), params.begin() + 1  );
	params[0].Type = STRING_MANAGER::bot;
	params[0].setEIdAIAlias( getEntityIdFromRow( giver ), CAIAliasTranslator::getInstance()->getAIAlias(getEntityIdFromRow( giver )) ); 
	CMissionParser::solveEntitiesNames(params,userRow,params[0].getEId());	
	return STRING_MANAGER::sendStringToClient( userRow, TitleText,params );
}// CMissionTemplate sendTitleText

uint32 CMissionTemplate::sendAutoText( const TDataSetRow & userRow,const NLMISC::CEntityId & giver) const
{
	TVectorParamCheck params = AutoParams;
	CMissionParser::solveEntitiesNames(params,userRow,giver);	
	return STRING_MANAGER::sendStringToClient( userRow, AutoText,params );
}// CMissionTemplate::sendAutoText

uint32 CMissionTemplate::sendDescText( const TDataSetRow & userRow, const TDataSetRow & giver, uint32 descIndex) const
{
	CEntityId id = getEntityIdFromRow( giver );
	
	TVectorParamCheck params;
	const TVectorParamCheck* addParams = NULL;
	const string * txt = NULL;
	if ( descIndex == 0xFFFFFFFF )
	{
		txt = &DescText;
		addParams = &DescParams;
	}
	else
	{
		if  ( descIndex >= OverloadedDescs.size() )
		{
			nlwarning("<MISSIONS> Invalid descIndex %u, size is  %u",descIndex,OverloadedDescs.size() );
			txt = &DescText;
			addParams = &DescParams;
		}
		else
		{
			txt = &(OverloadedDescs[descIndex].Text);
			addParams = &(OverloadedDescs[descIndex].Params);
		}
	}
	params.reserve(1 + (*addParams).size() );		
	params.push_back(STRING_MANAGER::TParam(STRING_MANAGER::entity));
	params.back().setEIdAIAlias(id, CAIAliasTranslator::getInstance()->getAIAlias(id));
	params.insert(params.end(), (*addParams).begin(), (*addParams).end());


	CMissionParser::solveEntitiesNames(params,userRow,id);
	return STRING_MANAGER::sendStringToClient( userRow,*txt,params );
}// CMissionTemplate sendDetailsText

/*
void CMissionTemplate::updateMissionStats(TMissionResult result)
{
	switch (result)
	{
	case mr_success:
		MissionStats.SuccessCount++;
		break;
	case mr_fail:
		MissionStats.FailCount++;
		break;
	case mr_abandon:
		MissionStats.AbandonCount++;
		break;
	case mr_ignore:
		break;
	default:
		nlstop;
	}
}
*/

bool CMissionTemplate::parseMissionText( uint32 line, const std::vector< std::string > & script, std::string & textId, TVectorParamCheck & textParams )
{
	if( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<phrase_id>*[;param]");
		return false;
	}
	return CMissionParser::parseParamText( line, script[1], textId, textParams );
}// CMissionTemplate parseDescription

bool CMissionTemplate::parseScriptVar(uint32 line,  const std::vector< std::string > & script, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > >& chatParams )
{
	if( script.size() != 3 )
	{
		MISLOGSYNTAXERROR( "<type> : <name>");
		return false;
	}
	STRING_MANAGER::TParamType type = STRING_MANAGER::stringToParamType( CMissionParser::getNoBlankString(script[1]) );
	if ( type == STRING_MANAGER::NB_PARAM_TYPES )
	{
		MISLOGERROR1("invalid type '%s'", CMissionParser::getNoBlankString(script[1]).c_str() );
		return false;
	}
	// convert creature into creature model
	if( type == STRING_MANAGER::creature )
		type = STRING_MANAGER::creature_model;
	chatParams.push_back( make_pair( CMissionParser::getNoBlankString(script[2]), type ) );
	return true;
}// CMissionTemplate parseScriptVar

bool CMissionTemplate::addSkillToList( uint32 line, const std::vector< std::string > & script, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams )
{
	if( script.size() != 2)
	{
		MISLOGSYNTAXERROR( "<skill_name> [<min_level> [<max_level>]] *[;<skill_name> [<min_level> [<max_level>]]  ]");
		return false;
	}
	std::vector< std::string > entries;
	splitString( script[1],";",entries );
	Prerequisits.Skills.resize( Prerequisits.Skills.size() + 1 );
	bool ret = true;
	for ( uint i = 0; i < entries.size(); i++ )
	{
		std::vector< std::string > args;
		CMissionParser::tokenizeString( entries[i]," \t",args );
	
		if (args.empty())
		{
			MISLOGERROR("empty skill list !");
			return false;
		}
		CSkillPrereq skill;
		CMissionParser::removeBlanks(args[0]);
		skill.Skill = SKILLS::toSkill( args[0] );
		if ( skill.Skill == SKILLS::unknown )
		{
			MISLOGERROR1("invalid skill '%s'", args[0].c_str());
			ret = false;
		}
	
		chatParams.push_back( std::make_pair( args[0],STRING_MANAGER::skill )  );
		skill.Min = 1;
		skill.Max = 0x7FFFFFFF;
		if ( args.size() > 1 )
			NLMISC::fromString(args[1], skill.Min);
		if ( args.size() == 3 )
			NLMISC::fromString(args[2], skill.Max);
		if ( args.size() >= 4 )
		{
			MISLOGSYNTAXERROR( "<skill_name> [<min_level> [<max_level>]] *[;<skill_name> [<min_level> [<max_level>]]  ]");
			ret = false;
		}
		Prerequisits.Skills.back().push_back( skill );
	}
	return ret;
}// CMissionTemplate parseSkillPrereq

bool CMissionTemplate::parseItemList(uint32 line,  const std::string & separator, const std::vector< std::string > & script, std::vector< NLMISC::CSheetId > & ret, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR1("<item>*[%s<item>]", separator.c_str());
		return false;
	}
	else
	{
		std::vector< std::string > args;
		NLMISC::splitString(script[1],separator,args);
		for ( uint j = 0; j < args.size(); j++ )
		{
			CMissionParser::removeBlanks(args[j]);
			CSheetId sheet( args[j] + ".sitem" );
			if ( sheet == CSheetId::Unknown)
			{
				MISLOGERROR1("invalid item '%s'", (args[j] + ".sitem").c_str() );
				return false;
			}
			else
			{
				ret.push_back( sheet );
				chatParams.push_back( std::make_pair( args[j], STRING_MANAGER::item )  );
			}
		}
		return true;
	}
}// CMissionTemplate parseItemList

bool CMissionTemplate::parseTitlePrereq(uint32 line,  const std::vector< std::string > & script )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<title>");
		return false;
	}
	else
	{
		string str = CMissionParser::getNoBlankString( script[1] );
		Prerequisits.Title = CHARACTER_TITLE::toCharacterTitle( str );
		if ( Prerequisits.Title == CHARACTER_TITLE::NB_CHARACTER_TITLE )
		{
			MISLOGERROR1("invalid title %s", script[1].c_str());
			return false;
		}
		return true;
	}
}// CMissionTemplate parseTitlePrereq

bool CMissionTemplate::parseFamePrereq(uint32 line, const std::vector< std::string > & script )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<fame_id> <fame_mini>");
		return false;
	}
	else
	{
		vector<string> vars;
		CMissionParser::tokenizeString( script[1]," \t",vars );
		if ( vars.size() != 2 )
		{
			MISLOGSYNTAXERROR("<fame_id> <fame_mini>");
			return false;
		}
		if (  CStaticFames::getInstance().getFactionIndex( vars[0] ) == CStaticFames::INVALID_FACTION_INDEX )
		{
			MISLOGERROR1("invalid fame '%s'", vars[0].c_str());
			return false;
		}
		Prerequisits.FameId = CStringMapper::map( vars[0] );
		NLMISC::fromString(vars[1], Prerequisits.FameMin);
		return true;
	}
}// CMissionTemplate parseFamePrereq

bool CMissionTemplate::parseGradePrereq(uint32 line, const std::vector< std::string > & script )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<grade>");
		return false;
	}
	else
	{
		string str = CMissionParser::getNoBlankString(script[1]);
		Prerequisits.GuildGrade = EGSPD::CGuildGrade::fromString( str );
		if ( Prerequisits.GuildGrade == EGSPD::CGuildGrade::Unknown )
		{
			MISLOGERROR1("invalid grade '%s'", str.c_str());
			return false;
		}
	}
	return true;
}// CMissionTemplate parseGradePrereq

bool CMissionTemplate::parseInt(uint32 line, const std::vector< std::string > & script, int & ret )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<int>");
		return false;
	}
	else
	{
		NLMISC::fromString(script[1], ret);
		return true;
	}
}// CMissionTemplate parseInt

bool CMissionTemplate::parseBrickList(uint32 line, const std::vector< std::string > & script, std::vector< NLMISC::CSheetId > & ret, std::vector< std::pair< std::string, STRING_MANAGER::TParamType > > & chatParams )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<sbrick>*[ <action>]");
		return false;
	}
	std::vector< std::string > args;
	NLMISC::splitString(script[1],";",args);
	for ( uint j = 0; j < args.size(); j++ )
	{
		CMissionParser::removeBlanks(args[j]);
		CSheetId sheet( args[j] + ".sbrick" );
		if ( sheet == CSheetId::Unknown)
		{
			MISLOGERROR1("invalid sbrick '%s'", (args[j] + ".sbrick").c_str() );
			return false;
		}
		else
		{
			ret.push_back( sheet );
			chatParams.push_back( std::make_pair( args[j], STRING_MANAGER::sbrick )  );
		}
	}
	return true;
}// CMissionTemplate::parseActionList

bool CMissionTemplate::parseReplayTimer(uint32 line,  const std::vector< std::string > & script , TGameCycle &replayTimer)
{
	if (script.size() != 2)
	{
		MISLOGSYNTAXERROR("<ticks_delay>");
		return false;
	}

	replayTimer = atoui(script[1].c_str());
	return true;
}


bool CMissionTemplate::addMissionsToList( uint32 line, const std::vector< std::string > & script, std::vector< CPrerequisits::TMissionReq > & ret)
{
	ret.reserve( ret.size() + 1 );

	vector<string> missions;
	if ( !CMissionParser::parseStringList(line, ";", script, missions ) )
		return false;

	ret.push_back(CPrerequisits::TMissionReq(line, missions));
	bool ok = true;
	for (uint i = 0;i < ret.back().Missions.size(); i++)
	{
		if ( CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( ret.back().Missions[i] ) == CAIAliasTranslator::Invalid )
		{
			MISLOGERROR1("invalid mission '%s' can't get alias", ret.back().Missions[i].c_str());
			ok = false;
		}
	}
	return ok;
}// CMissionTemplate::addMissionsToList

bool CMissionTemplate::parseSeason(uint32 line, const std::vector<std::string> &script, EGSPD::CSeason::TSeason &ret)
{
	if (script.size() != 2)
	{
		MISLOGSYNTAXERROR("<season>");
		return false;
	}
	string season = script[1];
	CMissionParser::removeBlanks(season);
	ret = EGSPD::CSeason::fromString(season);
	if ( ret == EGSPD::CSeason::EndSeason || ret == EGSPD::CSeason::Invalid )
	{
		MISLOGERROR2("invalid season name '%s' (extracted from '%s')", season.c_str(), script[1].c_str());
		return false;
	}
	return true;
}


TAIAlias CMissionTemplate::parseMissionParam(uint32 line,  const std::vector< std::string > & script )
{
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<mission_name>");
		return CAIAliasTranslator::Invalid;
	}
	TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( CMissionParser::getNoBlankString(script[1]) );
	if ( alias == CAIAliasTranslator::Invalid )
	{
		MISLOGERROR1("invalid mission name '%s'", script[1].c_str());
		return CAIAliasTranslator::Invalid;
	}
	return alias;	
}// CMissionTemplate::parseMissionParam

bool CMissionTemplate::parsePrice(uint32 line,  const std::vector< std::string > & script, CMissionSpecificParsingData& data )
{
	bool ret = true;
	if ( script.size() != 3 )
	{
		MISLOGSYNTAXERROR("<var_name> : <item> <quality> <factor> *[;<item> <quality> <factor>]");
		return false;
	}
	pair<string,uint> entry;
	entry.first = CMissionParser::getNoBlankString( script[1] );
	entry.second = 0;

	vector<string> multiArgs;
	CMissionParser::tokenizeString( script[2],";",multiArgs );
	for ( uint i = 0; i < multiArgs.size(); i++ )
	{
		vector<string> args;
		CMissionParser::tokenizeString( multiArgs[i]," \t",args );
		if ( !CMissionParser::addItemPrice(line, args,entry.second ) )
			ret = false;
	}
	data.Integers.insert( entry );
	return ret;
}

