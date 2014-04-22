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

#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/net/service.h"

#include "nel/ligo/ligo_config.h"

#include "game_share/emote_list_parser.h"
#include "game_share/generic_xml_msg_mngr.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_encyclo.h"
#include "mission_manager/mission_manager.h"
#include "creature_manager/creature_manager.h"
#include "primitives_parser.h"
#include "mission_log.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "team_manager/team_manager.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_guild.h"
#include "mission_manager/mission_team.h"
#include "egs_utils.h"
#include "egs_pd.h"
#include "guild_manager/guild_member_module.h"
#include "guild_manager/guild_manager.h"
#include "building_manager/building_manager.h"
#include "building_manager/room_instance.h"
#include "zone_manager.h"
#include "world_instances.h"
#include "mission_step_ai.h"
#include "mission_step_misc.h"
#include "server_share/stl_allocator_checker.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CCharacterDynChatBeginEnd	CharacterDynChatBeginEnd;


// Let's have a frequency of 64 cycles by default
CVariable<uint> CheckCharacterVisitPlacePeriodGC( "egs", "CheckCharacterVisitPlacePeriodGC", "Game cycle frequency for the check visit place mission routine", 64, 0, true );


/// singleton's instanciation
CMissionManager* CMissionManager::_Instance = NULL;
std::map< std::string, uint32 >	CMissionManager::_Emotes;

RY_PDS::IPDBaseData* CMissionManager::missionFactoryPD()
{
	nlerror("<MISSIONS> Invalid type entered");
	return NULL;
}

RY_PDS::IPDBaseData* CMissionManager::missionFactoryPDSolo()
{
	return new CMissionSolo;
}
RY_PDS::IPDBaseData* CMissionManager::missionFactoryPDTeam()
{
	return new CMissionTeam;
}
RY_PDS::IPDBaseData* CMissionManager::missionFactoryPDGuild()
{
	return new CMissionGuild;
}

/*void CDynMissionDescMsgImp::callback (const std::string &name, uint8 id)
{
//	CMissionManager::getInstance()->processDynRequestAnswer(*this);
}// CDynMissionDescMsgImp callback
*/
void CCAisActionMsgImp::callback(const std::string &name, NLNET::TServiceId id)
{
	if ( Content.size() == 2 )
	{
		MISDBG("CCAisActionMsg received. params = '%s' , '%s'", Content[0].c_str(),Content[1].c_str());
		TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( Content[0] );
		if ( alias != CAIAliasTranslator::Invalid )
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(alias);
			if ( templ != NULL )
			{
				if ( Content[1] == "end_escort" )
				{
					for ( uint i = 0; i < templ->Instances.size() ; i++)
					{
						if ( templ->Instances[i] )
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities( entities );
							for ( uint j = 0; j < entities.size() ; j++ )
							{
								CCharacter * user = PlayerManager.getChar( entities[j] );
								if( user)
								{
									CMissionEventEscort event( alias );
									user->processMissionEvent( event, alias );
								}
								else
									nlwarning( "<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex() );
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",Content[1].c_str(),Content[0].c_str() );
					}
				}
				else if ( Content[1] == "fail" )
				{
					bool exit = false;

					// get instance currently in escort step
					for ( uint i = 0; (i < templ->Instances.size()) && !exit ; ++i)
					{
						if ( templ->Instances[i] != NULL )
						{
							// check step
							for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator itStep = templ->Instances[i]->getStepsBegin(); itStep != templ->Instances[i]->getStepsEnd(); ++itStep )
							{
								nlassert( uint( (*itStep).second.getIndexInTemplate() - 1 ) < templ->Steps.size() );

								CMissionStepEscort *escortStep = dynamic_cast<CMissionStepEscort*> (templ->Steps[ (*itStep).second.getIndexInTemplate() - 1 ]);
								if ( escortStep != NULL )
								{
									templ->Instances[i]->onFailure(false);

									exit = true;
									break;
								}
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> *fail* mission %s  has a NULL instance ",Content[0].c_str() );
					}
				}
				else
				{
					for ( uint i = 0; i < templ->Instances.size() ; ++i)
					{
						if ( templ->Instances[i] )
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities( entities );
							for ( uint j = 0 ; j < entities.size() ; ++j )
							{
								CCharacter * user = PlayerManager.getChar( entities[j] );
								if( user)
								{
									CMissionEventAIMsg event(Content[1]);
									user->processMissionEvent( event, alias );
								}
								else
									nlwarning( "<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex() );
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",Content[1].c_str(),Content[0].c_str() );
					}
				}
			}
			else
				nlwarning( "<CCAisActionMsgImp callback> invalid mission %s : NULL ptr",Content[0].c_str() );
		}
		else
			nlwarning( "<CCAisActionMsgImp callback> invalid mission %s : alias not found",Content[0].c_str() );
	}
	else
		nlwarning( "<CCAisActionMsgImp callback> %u params is not supported",Content.size() );
} // CCAisActionMsgImp callback

// ****************************************************************************
void CHandledAIGroupSpawnedMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CMissionEventGroupSpawned event(this->GroupAlias);

	CCharacter * pChar = PlayerManager.getChar(this->PlayerRowId);
	if (pChar != NULL)
	{
		pChar->processMissionEvent(event, this->MissionAlias);
	}
	else
	{
		nlwarning("<CHandledAIGroupSpawnedMsgImp callback> Character not found (%s)", PlayerRowId.toString().c_str());
	}
} // CHandledAIGroupSpawnedMsgImp callback

// ****************************************************************************
void CHandledAIGroupDespawnedMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CMissionEventGroupDespawned event(this->GroupAlias);

	CCharacter * pChar = PlayerManager.getChar(this->PlayerRowId);
	if (pChar != NULL)
	{
		pChar->processMissionEvent(event, this->MissionAlias);
	}
	else
	{
		nlwarning("<CHandledAIGroupSpawnedMsgImp callback> Character not found (%s)", PlayerRowId.toString().c_str());
	}
} // CHandledAIGroupSpawnedMsgImp callback


void CMissionManager::init()
{
	nlassert(_Instance == NULL);
	_Instance = new CMissionManager();
}// CMissionManager init


void CMissionManager::release()
{
	delete _Instance;
	_Instance = NULL;
} // CMissionManager release

CMissionManager::CMissionManager()
{
	 //init the mission log system
	CConfigFile::CVar *varPtr = IService::getInstance()->ConfigFile.getVarPtr("MissionLogFile");
	if ( !varPtr )
		MissionLog.init("");
	else
		MissionLog.init( varPtr->asString() );

	// get the emote list
	EMOTE_LIST_PARSER::initEmoteList(_Emotes);

	// get the loaded primitives
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();
//	const vector<string> & primNamesVect = CPrimitivesParser::getInstance().getPrimitiveFiles();

	// parse mission validation configuration file
	loadMissionValidationFile("mission_validation.cfg");

	// parse primitive files to build the missions
	nlinfo("parsing the missions");
	bool parsingOk = true;

	CMissionGlobalParsingData globalData;

	uint countBadMission = 0;

	// parse missions
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		// parse primitive for mission. default: no npc giver alias
		if (! parsePrimForMissions(first->Primitive.RootNode, first->FileName, globalData, countBadMission, CAIAliasTranslator::Invalid) )
		{
			if ( MissionLog.getLogFile().empty() )
				nlwarning("<CMissionManager constructor> Error while building the missions in primitive number '%s'", first->FileName.c_str());
			else
				nlwarning("<CMissionManager constructor> Error while building the missions in primitive number '%s'. See %s", first->FileName.c_str(), MissionLog.getLogFile().c_str());
		}
	}
	if (countBadMission != 0)
		MISLOG("Bad missions total : %u", countBadMission);

	// build copy missions
	for ( uint i = 0; i < globalData.CopyMissions.size(); i++ )
	{
		CHashMap< uint,CMissionTemplate* >::iterator it = _MissionTemplates.find( globalData.CopyMissions[i].second );
		if ( it == _MissionTemplates.end() )
		{
			MISLOG("copy mission alias %s references bad mission alias %s, which could not be built",
				CPrimitivesParser::aliasToString(globalData.CopyMissions[i].first).c_str(),
				CPrimitivesParser::aliasToString(globalData.CopyMissions[i].second).c_str());
			_MissionTemplates.erase(globalData.CopyMissions[i].first);
		}
		else
		{
			CHashMap< uint,CMissionTemplate* >::iterator itTest = _MissionTemplates.find( globalData.CopyMissions[i].first );
			if ( itTest == _MissionTemplates.end() )
			{
				MISLOG("copy mission alias %s not found!!!", CPrimitivesParser::aliasToString(globalData.CopyMissions[i].first).c_str());
			}
			else
			{
				(*it).second->copy( *(*itTest).second );
				(*itTest).second->Alias = globalData.CopyMissions[i].first;
			}
		}
	}

	// check consistency of reference missions
	MISLOG("available reference missions:");
	uint countRef = 0;
	CHashMap< uint, TAIAlias >::iterator itRef = _RefMissions.begin();
	for (; itRef != _RefMissions.end();  )
	{
		TAIAlias refAlias = (*itRef).first;
		TAIAlias realAlias = (*itRef).second;
		if ( _MissionTemplates.erase( refAlias ) != 0 )
		{
			CHashMap< uint,CMissionTemplate* >::iterator it = _MissionTemplates.find( realAlias );
			if ( it != _MissionTemplates.end() )
			{

				std::map<TAIAlias,std::string>::iterator itName = globalData.NameMap.find( refAlias );
				if ( itName != globalData.NameMap.end() )
				{
					MISLOG("'%s' alias = %s",(*itName).second.c_str(), CPrimitivesParser::aliasToString(refAlias).c_str());
					++itRef;
					countRef++;
					continue;
				}
				else
					MISLOG("'NAME NOT FOUND' alias = %s", CPrimitivesParser::aliasToString(refAlias).c_str());

			}
			else
				MISLOG("ref mission alias %s references bad mission alias %s, which could not be built",
					CPrimitivesParser::aliasToString(refAlias).c_str(),
					CPrimitivesParser::aliasToString(realAlias).c_str());
		}
		else
			MISLOG("ref mission alias %s was not found!!!", CPrimitivesParser::aliasToString(refAlias).c_str() );

		CHashMap< uint, TAIAlias >::iterator itDel = itRef;
		++itRef;
		_RefMissions.erase( itDel );
	}

	// resolve parent missions
	MISLOG("resolving parent missions:");
	for ( uint i = 0; i<  globalData.ParentMissions.size(); i++ )
	{
		TAIAlias aliasParent = globalData.ParentMissions[i].first;
		TAIAlias aliasChild = globalData.ParentMissions[i].second;
		CMissionTemplate * templ = getTemplate( aliasParent );
		if ( !templ )
		{
			MISLOG( "invalid parent mission %u, child = %u", aliasParent,aliasChild );
		}
		else if ( !getTemplate( aliasChild ) )
		{
			MISLOG( "invalid child mission %u, child = %u", aliasChild, aliasParent );
		}
		else
		{
			templ->ChildrenMissions.push_back( aliasChild );
		}
	}

	nlinfo("end of missions parsing");
	MISLOG("available missions:");

	uint	count = 0;
	CHashMap< uint,CMissionTemplate* >::const_iterator it = _MissionTemplates.begin();
	for ( ;it!= _MissionTemplates.end();++it )
	{
		std::map<TAIAlias,std::string>::iterator itName = globalData.NameMap.find( (*it).first );
		if ( itName != globalData.NameMap.end() )
		{
			MISLOG("'%s' alias = %s",(*itName).second.c_str(), CPrimitivesParser::aliasToString((*it).first).c_str());
			count++;
		}
		else
		{
			MISLOG("'NAME NOT FOUND' alias = %s", CPrimitivesParser::aliasToString((*it).first).c_str());
		}
	}
	MISLOG("Total of %u different missions, %u reference missions (total of %u available mission)", count, countRef, count+countRef);
}// CMissionManager ctor


CMissionManager::~CMissionManager()
{
	MissionLog.release();
	CHashMap<uint,CMissionTemplate*>::iterator it = _MissionTemplates.begin();
	for ( ; it != _MissionTemplates.end(); ++it )
		delete (*it).second;
}// CMissionManager dtor

bool CMissionManager::parsePrimForMissions(const NLLIGO::IPrimitive* prim,const std::string &filename, CMissionGlobalParsingData & globalData, uint &badMissionCount, TAIAlias npcGiverAlias)
{
	string value;
	// index of the mission bring parsed
	static uint missionIndex = 0;
	// if the node is a mission parse it
	if (prim->getPropertyByName("class",value) && !nlstricmp(value.c_str(),"mission") )
	{
		missionIndex++;
		prim->getPropertyByName("name",value);

		// parse the mission and put it in our manager
		CMissionTemplate* templ = new CMissionTemplate();
		if ( !templ->build( prim, globalData, _RefMissions, value, npcGiverAlias ) )
		{
			MISLOG("Previous errors in mission %u '%s'%s in file '%s'",
				missionIndex,
				value.c_str(),
				CPrimitivesParser::aliasToString(templ->Alias).c_str(),
				filename.c_str() );
			MissionLog.Log->displayNL(" \n");
			delete templ;
			badMissionCount++;
			return false;
		}
		CHashMap<uint,CMissionTemplate*>::iterator itAlias =  _MissionTemplates.find( templ->Alias );
		if  (  itAlias != _MissionTemplates.end() )
		{

			MISLOG("<MISSIONS> Alias previously used  (mission %u, editor_name '%s'%s in file '%s'). Prev mission title is '%s'",
				missionIndex,
				value.c_str(),
				CPrimitivesParser::aliasToString(templ->Alias).c_str(),
				filename.c_str(),
				itAlias->second->getMissionName().c_str());
			delete templ;
			badMissionCount++;
			return false;
		}

		// Check if the mission is a encylopedia mission
		sint32 nAlb, nThm, nTask;
		templ->EncycloAlbum = -1;
		templ->EncycloThema = -1;
		templ->EncycloTask  = -1;
		templ->EncycloNPC  = CAIAliasTranslator::Invalid;
		if (CSheets::getEncyclopedia().isMissionPresent(value, nAlb, nThm, nTask))
		{
			templ->EncycloAlbum = nAlb;
			templ->EncycloThema = nThm;
			templ->EncycloTask  = nTask;
		}

		if (!templ->Tags.NeedValidation || isMissionValid(value, templ->HashKey))
		{
			_MissionTemplates.insert( make_pair( templ->Alias, templ ) );
			globalData.NameMap.insert( make_pair( templ->Alias, value ) );
		}
		else
		{
			nlinfo("Mission validation: mission %s is not valid and not loaded", templ->getMissionName().c_str());
			delete templ;
		}

		return true;
	}
	//this is not a mission node, so lookup recursively in the children
	else
	{
		// If this is a NPC, then it will be the default giver of its children mission
		if (prim->getPropertyByName("class",value) && !nlstricmp(value.c_str(),"npc_bot") )
		{
			// try to get its alias. Replace any previous value
			CPrimitivesParser::getAlias(prim, npcGiverAlias);
		}

		// lookup recursively in the children
		bool ok = true;
		for (uint i=0;i<prim->getNumChildren();++i)
		{
			const IPrimitive *child;
			if ( !prim->getChild(child,i) || !parsePrimForMissions(child, filename, globalData, badMissionCount, npcGiverAlias) )
				ok = false;
		}
		return ok;
	}
} // CMissionManager::parsePrimForMissions


/*
 * Warning: this must be called at the frequency of 1 game cycle
 */
void CMissionManager::tickUpdate()
{
	if( IsRingShard ) // Temporary Fix potential problem with multi shard instance Ring unification:
		return;	// Mission saved tick must be adapted for have relative value saved
	std::list< CMission* >::iterator it = _TimedMissions.begin();
	while( it != _TimedMissions.end() )
	{
		if( (*it)->getEndDate() > CTickEventHandler::getGameCycle() )
			break;
		( *it )->onFailure(false);
		it = _TimedMissions.begin();
	}

	it = _MonoMissions.begin();
	while( it != _MonoMissions.end() )
	{
		if( (*it)->getMonoEndDate() > CTickEventHandler::getGameCycle() )
			break;
		( *it )->onFailure(true);
		it = _MonoMissions.begin();
	}

	for ( map< CMission*, std::vector<CPlaceChecker> >::iterator it = _PlaceDependantMissions.begin(); it != _PlaceDependantMissions.end();)
	{
		uint i = 0;
		uint size = (uint)(*it).second.size();
		map< CMission*, std::vector<CPlaceChecker> >::iterator itBack= it;
		++itBack;
		for ( ; i < size; i++ )
		{
			CPlaceChecker & check = (*it).second[i];
			if ( check.EndDate  <= CTickEventHandler::getGameCycle() )
			{
				(*it).first->onFailure(true);
				break;
			}
		}
		it = itBack;
	}

	checkVisitPlaceMissions();

}// CMissionManager::tickUpdate


/*
 * Check the place of characters who have VisitPlace missions.
 * All CMissionEventVisitPlace are processed here, at a frequency of a few seconds,
 * because they may fail when entering VisitPlace missions if the constraints are not met.
 * Warning: this must be called at the frequency of 1 game cycle
 *
 * TODO: Enable visit place for team & guild missions
 */
void CMissionManager::checkVisitPlaceMissions()
{
	if ( !MissionSystemEnabled )
		return;

	// Warning: ++icpc not in the 'for ()'
	for ( CStepsByCharacter::iterator icpc=_CharactersForPlaceCheck.begin(); icpc!=_CharactersForPlaceCheck.end(); )
	{
		const TDataSetRow& characterRowId = (*icpc).first;
		CCharacter *character = PlayerManager.getChar( characterRowId );
		if ( character )
		{
			// Warning: this must be called at the frequency of 1 game cycle
			if ( (characterRowId.getIndex() & (CheckCharacterVisitPlacePeriodGC.get()-1)) == (CTickEventHandler::getGameCycle() & (CheckCharacterVisitPlacePeriodGC.get()-1)) )
			{
				// Iterate on all the "visit place" steps that the character has currently active
				H_AUTO(CMissionManager_CheckCharVisitPlace);
				CSteps& missionStepIds = (*icpc).second;
				for ( CSteps::iterator its=missionStepIds.begin(); its!=missionStepIds.end(); )
				{
					// Skip the step if it was cancelled
					if ( missionStepIds.isCancelled( its ) )
					{
						++its;
						continue;
					}

					const CSteps::CMissionStepId& stepId = (*its);
					const CMissionTemplate *missionTemplate = getTemplate( stepId.MissionAlias );
					BOMB_IF( ! missionTemplate, NLMISC::toString( "Invalid VisitPlace template %s", CPrimitivesParser::getInstance().aliasToString( stepId.MissionAlias ).c_str() ).c_str(), ++its; continue );
					const IMissionStepTemplate *step = missionTemplate->getStep( stepId.StepIndex );
					const CMission *missionInstance = character->getMission( stepId.MissionAlias );
					BOMB_IF( ! (step && missionInstance), NLMISC::toString( "Invalid Visit Place step or mission %s", CPrimitivesParser::getInstance().aliasToString( stepId.MissionAlias ).c_str() ).c_str(), ++its; continue );

					// Test if the iterated "visit place" steps match the current places with contraints
					bool placeProcessed = false;
					const CMissionStepVisit *stepVisit = safe_cast<const CMissionStepVisit*>(step);
					const vector<uint16>& placesUnderCharacter = character->getPlaces();
					vector<uint16> placesToCheck = placesUnderCharacter;
					placesToCheck.push_back( character->getCurrentRegion() );
					placesToCheck.push_back( character->getCurrentStable() );
					for ( vector<uint16>::const_iterator ip=placesToCheck.begin(); ip!=placesToCheck.end(); ++ip )
					{
						// Test event (calling processMissionEvent() every time would be too much CPU consuming)
						if ( stepVisit->testMatchEvent( character, missionInstance, *ip ) )
						{
							// Process the CMissionEventVisitPlace. If there are several steps that
							// are matching the event, there is no guarantee that the processed step
							// will be iterated step. However, after all matching steps are iterated,
							// all matching steps will have been processed.
							// As some code called in processMissionEvent can call removeMissionStepForPlaceCheck()
							// on any step, removeMissionStepForPlaceCheck() must not break this iteration
							// (see comment in CSteps).
							H_AUTO(CMissionManager_processEventVisitPlace);
							CMissionEventVisitPlace eventPlace( *ip );
							if ( character->processMissionEvent( eventPlace ) )
							{
								// Don't test the remaining places, continue to the next step directly
								placeProcessed = true;
								CSteps::iterator itToErase = its;
								++its;
								missionStepIds.removeIteratedStep( itToErase ); // erasing an element from CMissionStepIds does not invalidate iterators
								break;
							}
							else
							{
								nldebug( "Event VisitPlace matched but event not processed (%u-%s/%u), There is probably another condition required to validate the step.",
									stepId.MissionAlias, CPrimitivesParser::getInstance().aliasToString( stepId.MissionAlias ).c_str(), stepId.StepIndex );
							}
						}
					}
					if ( ! placeProcessed )
						++its;
				}

				// Remove the cancelled steps
				missionStepIds.removeAllCancelledSteps();

				// Are all the corresponding mission steps done?
				if ( missionStepIds.empty() )
				{
					// Remove from list if all matching steps are now done
					CStepsByCharacter::iterator icpcToRemove = icpc;
					++icpc;
					_CharactersForPlaceCheck.erase( icpcToRemove ); // erasing an element from a map does not invalidate iterators
				}
				else
				{
					// The steps are still not done; if the time of day is passed, the character can retry the next day
					++icpc;
				}
			}
			else
			{
				++icpc;
			}
		}
		else
		{
			// Remove from list if the character is not online anymore
			CStepsByCharacter::iterator icpcToRemove = icpc;
			++icpc;
			_CharactersForPlaceCheck.erase( icpcToRemove ); // erasing an element from a map does not invalidate iterators
		}
	}
}


/*
 * Add a mission step into the list of places to check for 'visit place' mission step. This method can be called from a processed event.
 */
void CMissionManager::insertMissionStepForPlaceCheck( const TDataSetRow& characterRowId, TAIAlias missionId, uint32 stepIndex0 )
{
	// All 'VisitPlace' missions are added here, and all are removed in checkVisitPlaceMissions()
	_CharactersForPlaceCheck[characterRowId].addStep( missionId, stepIndex0 );
}

/*
 * Remove a mission step from the list of places to check for 'visit place' mission step. This method can be called from a processed event.
 */
void CMissionManager::removeMissionStepForPlaceCheck( const TDataSetRow& characterRowId, TAIAlias missionId, uint32 stepIndex0 )
{
	CStepsByCharacter::iterator icpc = _CharactersForPlaceCheck.find( characterRowId );
	if ( icpc != _CharactersForPlaceCheck.end() )
	{
		CSteps& missionStepIds = (*icpc).second;
		missionStepIds.removeStep( missionId, stepIndex0 );
	}
}


/*
 * Remove the cancelled steps from the active steps (and from the cancelled steps as well)
 */
void CSteps::removeAllCancelledSteps()
{
	for ( std::vector<CMissionStepId>::const_iterator ics=_CancelledSteps.begin(); ics!=_CancelledSteps.end(); ++ics )
	{
		_Steps.erase( *ics );
	}
	_CancelledSteps.clear();
}


NL_INSTANCE_COUNTER_IMPL(CSteps);


void CMissionManager::instanciateChargeMission(TAIAlias  alias, TAIAlias giver, CGuild * guild )
{
	///TODO guild mission : initial steps on activation ( event list )
	/*

	// get the mission
	nlassert(guild);
	std::list< CMissionEvent * > eventList;
	CMissionTemplate * templ = getTemplate( alias );
	if ( !templ )
	{
		nlwarning("<MISSIONS> Invalid MissionTemplate %u for guild %u",alias,guild->getId());
		return;
	}
	if ( templ->Type  != MISSION_DESC::Guild )
	{
		nlwarning("<MISSIONS> Invalid MissionTemplate %u charge template but not a guild mission for guild %u",alias,guild->getId());
		return;
	}
	CMissionInstanceGuild * inst = new CMissionInstanceGuild( templ,guild,giver );
	inst->init(templ,guild,giver);
	guild->addMission( (CMissionInstanceGuild*)inst );
	initInstanciatedMission(inst,eventList);
	*/
}

void CMissionManager::instanciateMission(CCharacter* user,TAIAlias  alias, TAIAlias giver, std::list< CMissionEvent * > & eventList, TAIAlias mainMission)
{
	nlassert(user);

	string sDebugPrefix = "user:" + user->getId().toString() + " miss:" + CPrimitivesParser::aliasToString(alias);

	// get the template
	CMissionTemplate * templ = getTemplate( alias );
	if ( !templ )
	{
		MISDBG("%s ERROR instanciateMission : invalid mission template can't get template from alias", sDebugPrefix.c_str());
		return;
	}
	sDebugPrefix += ",'" + templ->getMissionName() + "' instanciateMission :";
	MISDBG("%s begin", sDebugPrefix.c_str());
	// test prerequisits
	if ( templ->testPrerequisits(user, true) != MISSION_DESC::PreReqSuccess )
	{
		MISDBG("%s test prerequisits fails", sDebugPrefix.c_str());
		return;
	}

	CMission* inst;
	if ( templ->Type  == MISSION_DESC::Solo )
	{
		// Solo mission
		if ( !templ->Tags.NoList && user->getMissionsCount() >= MaxSoloMissionCount )
		{
			CCharacter::sendDynamicSystemMessage(user->getId(), "MISSION_MAX_SOLO_REACHED" );
			return;
		}
		CMissionSolo * soloMission = EGS_PD_CAST<CMissionSolo*>( EGSPD::CMissionSoloPD::create( templ->Alias ) );
		if ( !soloMission )
		{
			MISDBG("%s could not create solo mission", sDebugPrefix.c_str());
			return;
		}

		soloMission->onCreation( giver );
		soloMission->setTaker(user->getEntityRowId());

		// Find a suitable client index (for non-invisible missions)
		if ( templ->Tags.NoList == false )
		{
			vector<bool> vFreePlace;
			vFreePlace.resize(MaxSoloMissionCount, true);

			std::map<TAIAlias, CMission*>::iterator it = user->getMissionsBegin();
			while (it != user->getMissionsEnd())
			{
				CMissionSolo *pMS = dynamic_cast<CMissionSolo*>(it->second);
				if (pMS != NULL)
				{
					uint8 index = pMS->getClientIndex();
					if (index < MaxSoloMissionCount)
						vFreePlace[index] = false;
				}
				++it;
			}

			uint8 idx;
			for (idx = 0; idx < MaxSoloMissionCount; ++idx)
				if (vFreePlace[idx])
					break;

			nlassert(idx<MaxSoloMissionCount);
			soloMission->setClientIndex(idx);
		}

		// Add mission
		user->addMission( soloMission );
		inst = soloMission;
	}
	else if ( templ->Type  == MISSION_DESC::Group )
	{
		// Group mission
		CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
		if ( team )
		{
			if ( !templ->Tags.NoList && team->getMissions().size() >= MaxGroupMissionCount )
			{
				CCharacter::sendDynamicSystemMessage(user->getId(), "MISSION_MAX_GROUP_REACHED" );
				return;
			}

			CMissionTeam * teamMission = EGS_PD_CAST<CMissionTeam*>( EGSPD::CMissionTeamPD::create( templ->Alias ) );
			if ( !teamMission )
			{
				MISDBG("%s could not create team mission", sDebugPrefix.c_str());
				return;
			}
			teamMission->onCreation( giver );
			teamMission->setTeam( user->getTeamId() );

			// Find a suitable client index (for non-invisible missions)
			if ( templ->Tags.NoList == false )
			{
				uint8 idx = 0;
				for ( uint i = 0; i < MaxGroupMissionCount; i++ )
				{
//					if ( ! user->_PropertyDatabase.x_getProp( NLMISC::toString( "GROUP:MISSIONS:%u:TITLE",i) ) )
					if ( ! CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(i).getTITLE(user->_PropertyDatabase))
					{
						idx = i;
						break;
					}
				}
				teamMission->setClientIndex( idx );
			}

			// Add mission
			team->addMission( teamMission );
			inst = teamMission;
		}
		else
		{
			MISDBG("%s invalid team %d", sDebugPrefix.c_str(), user->getTeamId());
			return;
		}
	}
	else if ( templ->Type  == MISSION_DESC::Guild )
	{
		/// Check to see if we can pick the mission
		CGuildMemberModule * module;
		if ( !user->getModuleParent().getModule( module ) )
		{
			MISDBG("%s user not in a guild", sDebugPrefix.c_str());
			return;
		}
		/* /// This is already checked in the prerequisites 
		if (!module->pickMission( templ->Alias ))
		{
			/// Todo : error message for the member
			return;
		}*/

		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
		if (!guild)
		{
			nlwarning( "<MISSIONS>cant find guild ID : %d", user->getGuildId() );
			return;
		}
		if ( !templ->Tags.NoList && guild->getMissions().size() >= MaxGuildMissionCount)
		{
			CCharacter::sendDynamicSystemMessage(user->getId(), "MISSION_MAX_GUILD_REACHED" );
			return;
		}

		CMissionGuild * guildMission = EGS_PD_CAST<CMissionGuild*>( EGSPD::CMissionGuildPD::create( templ->Alias ) );
		if ( !guildMission )
		{
			MISDBG("%s could not create guild mission", sDebugPrefix.c_str());
			return;
		}
		guildMission->onCreation( giver );
		guildMission->setGuild(user->getGuildId());

		// Find a suitable client index (for non-invisible missions)
		if ( templ->Tags.NoList == false )
		{
			uint8 idx = 0;
			for ( uint i = MaxGroupMissionCount; i < MaxGroupMissionCount + MaxGuildMissionCount; i++ )
			{
				if ( ! CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(i).getTITLE(user->_PropertyDatabase))
				{
					idx = i;
					break;
				}
			}
			guildMission->setClientIndex( idx );
		}

		// Add mission
		guild->addMission( guildMission );
		inst = guildMission;

		/// /!\ Do the same thing that the team missions but with the loop: for ( uint i = MaxGroupMissionCount; i < MaxGroupMissionCount + MaxGuildMissionCount; i++ )
		/// Instead of for ( uint i = 0; i < MaxGroupMissionCount; i++ ), so that we use available space for guild missions

		/*//teamMission->initBasics( giver );
		//soloMission->setTeam( user->getTeamId() );
		CGuild * guild = user->getGuild();
		if ( guild )
		{
			if ( guild->getMissions().size() >= MaxGuildMissionCount)
			{
				CCharacter::sendDynamicSystemMessage(user->getId(), "MISSION_MAX_GUILD_REACHED" );
				return;
			}
			uint16 memberIdx;
			if ( guild->getMemberIndex( user->getId(),memberIdx ) )
			{
				if ( guild->getMemberGrade( memberIdx ) < EGSPD::CGuildGrade::Officer )
				{
					inst = new CMissionInstanceGuild( templ,guild,giver );
					guild->addMission( (CMissionInstanceGuild*)inst );
				}
				else
				{
					CCharacter::sendDynamicSystemMessage( user->getId(), "GUILD_BUILDING_BAD_GRADE" );
					return;
				}
			}
			else
				return;
		}
		else
		{

		}*/
	}
	else
	{
		MISDBG("%s unimplemented mission type %u", sDebugPrefix.c_str(), templ->Type);
		return;
	}
	inst->setGiver( giver );
	inst->setMainMissionTemplateId(mainMission);

	STL_ALLOC_TEST
	initInstanciatedMission(inst, eventList);
	MISDBG("%s end (ok)", sDebugPrefix.c_str());
}// CMissionManager::instanciateMission

void CMissionManager::deInstanciateMission(CMission * mission)
{
	nlassert(mission);

	string sDebugPrefix;

	CCharacter * user = mission->getMainEntity();
	if ( user != NULL )
	{
		removeMissionDynChat(user,mission);
		user->delAllHandledAIGroup(mission);
		sDebugPrefix = "user:" + user->getId().toString();
	}
	else
	{
		MISDBG("Cannot find main entity for mission template ID %u", mission->getTemplateId());
	}

	// get the template
	CMissionTemplate * templ = getTemplate( mission->getTemplateId() );
	if (templ != NULL)
	{
		sDebugPrefix += ",'" + templ->getMissionName() + "' deinstanciateMission :";
	}
	else
	{
		MISDBG("%s ERROR deinstanciateMission : invalid mission template can't get template from alias", sDebugPrefix.c_str());
	}

	if ( mission->getEndDate() )
	{
		removeTimedMission(mission);
	}
	if ( mission->getMonoEndDate() )
	{
		removeMonoMission(mission);
	}
	if ( mission->getCrashHandlerIndex() != 0xFFFFFFFF )
	{
		CMissionManager::getInstance()->removeCrashHandlingMissions(*mission);
	}
	removeFromPlaceConstraints( mission );

	std::vector<TAIAlias> escorts;

	if ( templ )
	{
		if (user)
		{
			templ->getEscortGroups( escorts );
			for ( uint i = 0; i < escorts.size(); i++ )
				unregisterEscort( escorts[i],  user->getId() );

			if ( _SoloEscorts.find( user->getId() ) == _SoloEscorts.end() )
			{
				CTeam * team = TeamManager.getTeam( user->getTeamId() );
				if ( team )
				{
					if ( team->isFake() )
					{
						TeamManager.removeFakeTeam( user );
					}
				}
			}
		}

		for ( uint i = 0; i < templ->Instances.size(); i++ )
		{
			if ( templ->Instances[i] == mission )
			{
				templ->Instances[i] = templ->Instances.back();
				templ->Instances.pop_back();
			}
		}

		if ( templ->Tags.NoList )
		{
			vector<TDataSetRow> entities;
			mission->getEntities( entities );

			TVectorParamCheck params(1);
			string msg;
			for ( map<uint32,EGSPD::CMissionCompassPD>::const_iterator it = mission->getCompassBegin(); it != mission->getCompassEnd(); ++it )
			{

				if ( (*it).second.getPlace() == CAIAliasTranslator::Invalid )
				{
					params[0].Type = STRING_MANAGER::bot;
					CEntityId entityId = CAIAliasTranslator::getInstance()->getEntityId( (*it).second.getBotId() );
					params[0].setEIdAIAlias( entityId, (*it).second.getBotId() );
					msg = "COMPASS_BOT";
					for ( uint i = 0; i < entities.size();i++ )
					{
						uint32 text = STRING_MANAGER::sendStringToClient( entities[i],msg,params );
						PlayerManager.sendImpulseToClient( getEntityIdFromRow(entities[i]), "JOURNAL:REMOVE_COMPASS_BOT", text, TheDataset.getDataSetRow(entityId).getCompressedIndex() );
					}
				}
				else
				{
					CPlace * place = CZoneManager::getInstance().getPlaceFromAlias( (*it).second.getPlace() );
					if ( place )
					{
						params[0].Identifier = place->getName();
						params[0].Type = STRING_MANAGER::place;
						msg = "COMPASS_PLACE";
						for ( uint i = 0; i < entities.size();i++ )
						{
							uint32 text = STRING_MANAGER::sendStringToClient( entities[i],msg,params );
							PlayerManager.sendImpulseToClient( getEntityIdFromRow(entities[i]), "JOURNAL:REMOVE_COMPASS", text );
						}
					}
				}
			}
		}
	}
	else
	{
		MISDBG("Cannot find template for mission template ID %u", mission->getTemplateId());
	}

	mission->setEndDate(0);
	mission->setBeginDate(0);
	MISDBG("%s ok", sDebugPrefix.c_str());

}// CMissionManager::deInstanciateMission

void CMissionManager::missionDoneOnce( CMissionTemplate* templ )
{
	nlassert(templ);
	if ( !templ->Tags.DoneOnce )
		return;

	for ( uint i = 0; i < templ->Instances.size(); i++ )
		templ->Instances[i]->onFailure(true);
}// CMissionManager::missionDoneOnce

void CMissionManager::sendDebugJournal( CCharacter * user)
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
	uint i =0;
	while(1)
	{
		CBankAccessor_PLR::TMISSIONS::TArray &missionItem = CBankAccessor_PLR::getMISSIONS().getArray(i);
//		uint title = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:TITLE",i) );
		uint title = missionItem.getTITLE(user->_PropertyDatabase);
		if ( title == 0)
			break;
		PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), title );
//		PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:DETAIL_TEXT",i) ) );
		PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), missionItem.getDETAIL_TEXT(user->_PropertyDatabase) );

//		params[0].Int = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:DELAY",i) );
//		params[0].Int = missionItem.getDELAY(user->_PropertyDatabase);
//		PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), "SOLE_INT",params );

//		params[0].Int = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:STEP_STATES",i) );
//		params[0].Int = missionItem.getSTEP_STATES(user->_PropertyDatabase);
//		PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), "SOLE_INT",params );

		uint j = 0;
		while(1)
		{
//			uint text = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:GOALS:%u:TEXT",i,j ) );
			uint text = missionItem.getGOALS().getArray(j).getTEXT(user->_PropertyDatabase);
			if ( text == 0)
				break;
			PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), text );
			j++;
		}
		j = 0;
		while(1)
		{
			if (j >= 8)
			{
				nlwarning("Invalid access to CBD entry MISSIONS:%u:TARGET%u : TARGET is out of 8 range", i, j);
				break;
			}
//			uint text = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:TITLE",i,j ) );
			uint text = missionItem.getTARGET(j).getTITLE(user->_PropertyDatabase);
			if ( text == 0)
				break;
			PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), text );

//			params[0].Int = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:X",i,j) );
			params[0].Int = missionItem.getTARGET(j).getX(user->_PropertyDatabase);
			PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), "SOLE_INT",params );

//			params[0].Int = (uint32)user->_PropertyDatabase.getProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:Y",i,j) );
			params[0].Int = missionItem.getTARGET(j).getY(user->_PropertyDatabase);
			PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), "SOLE_INT",params );
			j++;
		}
		i++;
	}

}// CMissionManager::sendDebugJournal

void CMissionManager::addDynChat( CMission * instance, CMissionStepDynChat * step, uint32 stepIndex )
{
	nlassert(instance);
	nlassert(step);
	CMissionTemplate* templ = getTemplate( instance->getTemplateId() );
	nlassert(templ);

	nlassert( stepIndex < templ->Steps.size() );

	CCharacter * user = instance->getMainEntity();
	if ( !user )
	{
		nlwarning("<MISSIONS> invalid user in mission instance %s",	CPrimitivesParser::aliasToString(instance->getTemplateId()).c_str() );
		return;
	}

	uint32 botAlias = step->Bot;
	if (botAlias==CAIAliasTranslator::Invalid)
		botAlias = instance->getGiver();

	CCreature * bot = NULL;
	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell = mirrorCell;
	if ( cell <= -2 )
	{
		IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
		if ( !room )
		{
			nlwarning("<MISSIONS>%s in cell %d. invalid room",user->getId().toString().c_str(),cell );
			return;
		}

		const uint size = (uint)room->getBots().size();
		for ( uint i = 0; i < size; i++ )
		{
			CCreature * c = CreatureManager.getCreature( room->getBots()[i] );
			if ( c && c->getAlias() == botAlias )
			{
				bot = c;
				break;
			}
		}
	}
	else
	{
		bot = CreatureManager.getCreature( CAIAliasTranslator::getInstance()->getEntityId( botAlias ) );
	}

	if ( !bot )
	{
		nlwarning("<MISSIONS>%s invalid bot %u",user->getId().toString().c_str(),step->Bot );
		return;
	}

	// build the dyn chat
	CDynChat dynChat;
	dynChat.Bot = bot->getEntityRowId();
	dynChat.StepIndex = stepIndex;
	dynChat.Mission = instance;


	// Remove any previous dyn chat that is not from that bot
	// we first close all the concerned dyn chat interface on the client,then send EndDynchat event and finally remove the concerned entries from the map
	// we have to do it in three passes because processMissionEvent can remove dyn chat entries. And we'd better avoid dereferencement on invalid iterators...
	nlassert(user);
	vector<TAIAlias> aliases;
	// close all interfaces
	// TODO nico : maybe this could be factored with 'removeAllUserChats' by using a predicate
	CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
	for (; it!= _DynChats.end() && (*it).first == user->getEntityRowId(); ++it )
	{
		if (it->second.Bot != dynChat.Bot)
		{
			// tell client to close the dyn chat interface
			// but sometime, mission giver is false, we must not close (delete) current dynChat mission.
			if( it->second.Mission->getTemplateId() != instance->getTemplateId() )
			{
				TDataSetIndex botId = it->second.Bot.getCompressedIndex();
				closeDynChat(user, it->second.Bot);
				// store the mission alias for future event processing
				aliases.push_back(it->second.Mission->getTemplateId());
			}
			else
			{
				nlwarning("Mission DynChat Bug: try to close a dynChat corresponding to actual mission running, mission allias %d, mission stepIndex %d, pre existing dynChat Bot %d, new dyn chat bot %d", instance->getTemplateId(), stepIndex, it->second.Bot.getCompressedIndex(), botAlias );
			}
		}
	}

	// For each concern mission, send an end dyn chat event to the player, specifying the mission
	CMissionEventEndDynChat event;
	const uint eventCount = (uint)aliases.size();
	for ( uint i = 0; i < eventCount; ++i )
	{
		user->processMissionEvent(event,aliases[i]);
	}
	// erase the user entries
	it = _DynChats.find(user->getEntityRowId());
	while (it!= _DynChats.end() && (*it).first == user->getEntityRowId())
	{
		CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator tmpIt = it;
		++ it;
		if (tmpIt->second.Bot != dynChat.Bot)
		{
			if( tmpIt->second.Mission->getTemplateId() != instance->getTemplateId() )
			{
				// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
				CharacterDynChatBeginEnd.DynChatEnd.push_back( dynChat.Bot );
				_DynChats.erase(tmpIt);
			}
			else
			{
				nlwarning("Mission DynChat Bug: try to erase a dynChat corresponding to actual mission running, mission allias %d, mission stepIndex %d, pre existing dynChat Bot %d, new dyn chat bot %d", instance->getTemplateId(), stepIndex, tmpIt->second.Bot.getCompressedIndex(), botAlias );
			}
		}
	}

	// if the player has a dynchat with this bot, make the new dyn chat sleep ( will be activated after first dyn chat )
	// NB nico : the guideline is now to have only a single possible dynchat at a time, let this code
	// as a fool keeper.
	it = _DynChats.find( user->getEntityRowId() );
	bool openChat = true;
	while ( it!= _DynChats.end() && (*it).first == user->getEntityRowId() )
	{
		CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator tmpIt = it;
		++ it;
		if ( (*tmpIt).second.Bot == bot->getEntityRowId() )
		{
			if ( (*tmpIt).second.StepIndex == dynChat.StepIndex )
			{
				if( tmpIt->second.Mission->getTemplateId() != instance->getTemplateId() )
				{
					// if the same dynchat is added twice replace the previous one
					_DynChats.erase(tmpIt);
				}
				else
				{
					nlwarning("Mission DynChat Bug: try to erase a dynChat added twice but corresponding to actual mission running, mission allias %d, mission stepIndex %d, pre existing dynChat Bot %d, new dyn chat bot %d", instance->getTemplateId(), stepIndex, tmpIt->second.Bot.getCompressedIndex(), botAlias );
				}
			}
			else
			{
				openChat = false;
			}
			break;
		}
	}

	_DynChats.insert( make_pair( user->getEntityRowId(), dynChat ) );
	CEntityId giverId = CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() );
	if ( giverId == CEntityId::Unknown )
	{
		nlwarning("<MISSIONS>%s invalid giver %u",user->getId().toString().c_str(),step->Bot );
		return;
	}
	if ( openChat )
	{
		openDynChat( user,bot,step, giverId );
	}
}// CMissionManager::addDynChat

void CMissionManager::switchDynChatSpeaker(CCharacter * user, const NLMISC::CEntityId & successorId)
{
	CCharacter * leader = PlayerManager.getChar( successorId );
	if ( !leader )
	{
		nlwarning("<MISSIONS>invalid successorId %s", successorId.toString().c_str());
		return;
	}
	CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
	// give all dynchat to the successor
	bool open = true;
	while ( it!= _DynChats.end() && (*it).first == user->getEntityRowId() )
	{
		CMission* instance = (*it).second.Mission;
		nlassert(instance);
		CMissionTemplate* templ = getTemplate( instance->getTemplateId() );
		nlassert(templ);
		if ( ( templ->Type == MISSION_DESC::Group ) ||
			 ( templ->Type == MISSION_DESC::Guild ) )

		{
			CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator itOld = it;
			++it;
			closeDynChat( user,(*itOld).second.Bot );
			CCreature * bot = CreatureManager.getCreature( (*itOld).second.Bot );
			if ( bot )
			{
				_DynChats.insert( make_pair(leader->getEntityRowId(), (*itOld).second ) );
				if (open)
				{
					nlassert( (*itOld).second.StepIndex < templ->Steps.size() );
					IMissionStepTemplate * stepTempl = templ->Steps[ (*itOld).second.StepIndex ];
					CMissionStepDynChat * chatStep = dynamic_cast< CMissionStepDynChat* >( stepTempl );
					nlassert( chatStep );
					CEntityId giverId = CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() );
					if ( giverId == CEntityId::Unknown )
						nlwarning("<DYNCHAT>'%s' invalid giver %u",user->getId().toString().c_str(), instance->getGiver() );
					openDynChat( user,bot, chatStep, giverId );
					open = false;
				}
				// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
				CharacterDynChatBeginEnd.DynChatEnd.push_back( (*itOld).second.Bot );
				_DynChats.erase(itOld);
			}
		}
		else
			++it;
	}

}// CMissionManager::switchDynChatSpeaker


void CMissionManager::removeAllUserDynChat(CCharacter * user)
{
	// we first close all the concerned dyn chat interface on the client,then send EndDynchat event and finally remove the concerned entries from the map
	// we have to do it in three passes because processMissionEvent can remove dyn chat entries. And we'd better avoid dereferencement on invalid iterators...
	nlassert(user);
	vector<TAIAlias> aliases;
	// close all interfaces
	CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
	for (; it!= _DynChats.end() && (*it).first == user->getEntityRowId(); ++it )
	{
		// tell client to close the dyn chat interface
		TDataSetIndex botId = (*it).second.Bot.getCompressedIndex();
		closeDynChat( user,(*it).second.Bot );
		// store the mission alias for future event processing
		aliases.push_back((*it).second.Mission->getTemplateId());
	}

	// The dynchats may have been erased in removeMissionDynChat called by processMissionEvent()
	it = _DynChats.find( user->getEntityRowId() );
	for (; it!= _DynChats.end() && (*it).first == user->getEntityRowId(); ++it )
	{
		// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
		CharacterDynChatBeginEnd.DynChatEnd.push_back( (*it).second.Bot );
	}

	// erase the user entries
	_DynChats.erase(user->getEntityRowId());

	// For each concerned mission, send an end dyn chat event to the player, specifying the mission
	// This must be after _DynChats.erase(user->getEntityRowId()), otherwise the event (e.g. jump
	// back) can trigger a openDynChat() on the same bot that would lead to a reentrance bug.
	CMissionEventEndDynChat event;
	const uint eventCount = (uint)aliases.size();
	for ( uint i = 0; i < eventCount; ++i )
		user->processMissionEvent(event,aliases[i] );
}

void CMissionManager::removeMissionDynChat(CCharacter * user, CMission * instance)
{
	nlassert(user);
	nlassert(instance);
	std::set<TDataSetRow> bots;

	CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
	while ( it != _DynChats.end() && (*it).first == user->getEntityRowId()  )
	{
		CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator itold = it;
		++it;
		if( (*itold).second.Mission == instance )
		{
			bots.insert( (*itold).second.Bot );
			// we dont send events because if we are here, it means that the mission is removed
			// inform client
			closeDynChat( user,(*itold).second.Bot );

			// erase the dyn chat
			_DynChats.erase( itold );

			// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
			CharacterDynChatBeginEnd.DynChatEnd.push_back( (*itold).second.Bot );
		}
	}

	// open the next waiting dyn chat for this user ( a user and a bot can share only 1 dyn chat )
	it = _DynChats.find( user->getEntityRowId() );
	while ( it != _DynChats.end() && (*it).first == user->getEntityRowId()  )
	{
		if ( bots.find( (*it).second.Bot ) != bots.end())
		{
			CCreature * bot = CreatureManager.getCreature( (*it).second.Bot );
			if ( bot )
			{
				CMission * instance = (*it).second.Mission;
				CMissionTemplate* templ = getTemplate( instance->getTemplateId() );
				nlassert(templ);
				nlassert( (*it).second.StepIndex < templ->Steps.size() );
				CMissionStepDynChat * step = dynamic_cast<CMissionStepDynChat *> ( templ->Steps[ (*it).second.StepIndex ] );
				nlassert( step );
				CEntityId giverId = CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() );
				if ( giverId == CEntityId::Unknown )
				{
					nlwarning("<DYNCHAT> invalid giver '%u'",instance->getGiver() );
					return;
				}
				openDynChat( user,bot, step, giverId );
			}
			bots.erase( (*it).second.Bot );
		}
		++it;
	}
}// CMissionManager::removeMissionDynChat


void CMissionManager::dynChatChoice( CCharacter * user, const TDataSetRow & botRow,uint8 choice )
{
	CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
    while ( it!= _DynChats.end() && (*it).first == user->getEntityRowId() )
    {
        bool reStart=false;
        if ( (*it).second.Bot == botRow )
        {
/*
			// tell client to close the dyn chat interface
			// but sometime, mission giver is false, we must not close (delete) current dynChat mission.
			if( it->second.Mission->getTemplateId() != instance->getTemplateId() )
*/

            CMission * inst = (*it).second.Mission;
            TAIAlias missionAlias = inst->getTemplateId();
            CMissionTemplate* templ = getTemplate( inst->getTemplateId() );
            nlassert(templ);
            uint index = (*it).second.StepIndex;
            nlassert ( index < templ->Steps.size() );
            CMissionStepDynChat * dynChat = dynamic_cast<CMissionStepDynChat *>( templ->Steps[index] );
            nlassert( dynChat );
            if ( choice >= dynChat->Answers.size() )
            {
                nlwarning("MISSIONS: Invalid answer index %u for user %s on mission alias %s",
					choice,
					user->getId().toString().c_str(),
					CPrimitivesParser::aliasToString(missionAlias).c_str());
                return;
            }
            const std::string & jump = dynChat->Answers[choice].Jump;
            uint i = 0;
            uint nbJumpPoints = (uint)templ->JumpPoints.size();
            bool updateJournal = false;
            for (; i < nbJumpPoints; i++ )
            {
                if ( templ->JumpPoints[i].Name == jump )
                {
                    // inform client
                    closeDynChat( user, botRow );

					std::list< CMissionEvent * > eventList;
                    inst->jump( templ->JumpPoints[i].Step,templ->JumpPoints[i].Action,eventList );

					// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
					CharacterDynChatBeginEnd.DynChatEnd.push_back( botRow );

// HERE "it" IS INVALID, NEED TO FIND IT AGAIN TO ERASE IT

					it = _DynChats.end(); // to be sure

					{
						/* We need to find the mission that we are working on as we invalidated the iterator
						 * in the jump()
						 */
						CHashMultiMap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator itToLookForMission = _DynChats.find( user->getEntityRowId() );
						while (itToLookForMission != _DynChats.end() && (*itToLookForMission).first == user->getEntityRowId())
						{
							if ( (*itToLookForMission).second.Bot == botRow )
							{
								CMission * missionIterated = (*itToLookForMission).second.Mission;
								if (missionIterated && inst  == missionIterated)
								{
									_DynChats.erase(itToLookForMission);
									break;
								}
							}
							++itToLookForMission;
						}
					}

//            _DynChats.erase(it);

// END PATCH



                    reStart = true;
                    switch ( inst->getProcessingState() )
                    {
	                    case CMission::Failed:
							inst->setProcessingState( CMission::Normal );
							inst->onFailure( true );
						break;
						case CMission::ActionFailed:
							inst->setProcessingState( CMission::Normal );
							inst->onFailure( false );
						break;
		                case CMission::Complete:
			                inst->setProcessingState( CMission::Normal );
				            inst->forceSuccess();
                        break;
					    case CMission::InJump:
						    inst->setProcessingState(CMission::Normal);
							updateJournal = true;
                        break;
						case CMission::Normal:
							updateJournal = true;
                        break;
                    }

					// Process events whatever happened (even if mission fails or complete)
					while( !eventList.empty() )
					{
						CMissionEvent::TMissionEventType t = eventList.front()->Type; // Backup the type
						user->processMissionEvent( *eventList.front() );
						// after processMissionEvent the event pointer is deleted ONLY if its an "AddMission Event"
						if ( t != CMissionEvent::AddMission ) // special case to avoid recursive call in processMissionEvent
							delete eventList.front();
						eventList.pop_front();
					}

                    break; // stop we found the jump point selected
                }
            }
            if (updateJournal)
                inst->updateUsersJournalEntry();

			if (i != nbJumpPoints) // Found the jump point so no more dynchat to process
                break;

            if ( i == nbJumpPoints )
            {
                nlwarning("MISSIONS: Invalid jump point %s in mission alias %s",
					dynChat->Answers[choice].Jump.c_str(),
					CPrimitivesParser::aliasToString(missionAlias).c_str());
                return;
            }
        }
        if (reStart)
            it=_DynChats.find( user->getEntityRowId() );
        else
            ++it;
    }

    // open the next waiting dyn chat for this user
    CCreature * bot = CreatureManager.getCreature( botRow );
    if ( bot )
    {
        it = _DynChats.find( user->getEntityRowId() );
        while ( it != _DynChats.end() && (*it).first == user->getEntityRowId()  )
        {
            if ( botRow == (*it).second.Bot )
            {
                CMission * instance = (*it).second.Mission;
                CMissionTemplate* templ = getTemplate( instance->getTemplateId() );
                nlassert(templ);
                nlassert( (*it).second.StepIndex < templ->Steps.size() );
                CMissionStepDynChat * step = dynamic_cast<CMissionStepDynChat *> ( templ->Steps[ (*it).second.StepIndex ] );
                nlassert( step );
                CEntityId giverId = CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() );
                if ( giverId == CEntityId::Unknown )
                {
                    nlwarning("<DYNCHAT> invalid giver '%u'",instance->getGiver() );
                    return;
                }
                openDynChat( user,bot, step,giverId );
                break;
            }
            ++it;
        }
    }
}// CMissionManager::dynChatChoice


/*
void CMissionManager::dynChatChoice( CCharacter * user, const TDataSetRow & botRow,uint8 choice )
{
	std::hash_multimap<TDataSetRow,CDynChat,TDataSetRow::CHashCode>::iterator it = _DynChats.find( user->getEntityRowId() );
	for (; it!= _DynChats.end() && (*it).first == user->getEntityRowId(); ++it )
	{
		if ( (*it).second.Bot == botRow );
		{
			CMission * inst = (*it).second.Mission;
			TAIAlias missionAlias = inst->getTemplateId();
			CMissionTemplate* templ = getTemplate( missionAlias );
			nlassert(templ);
			uint index = (*it).second.StepIndex;
			nlassert ( index < templ->Steps.size() );
			CMissionStepDynChat * dynChat = dynamic_cast<CMissionStepDynChat *>( templ->Steps[index] );
			nlassert( dynChat );
			if ( choice >= dynChat->Answers.size() )
			{
				nlwarning("<CMissionManager::dynChatChoice> Invalid answer index %u for user %s", choice, user->getId().toString().c_str());
				return;
			}
			const std::string & jump = dynChat->Answers[choice].Jump;
			uint i = 0;
			uint nbJumpPoints = templ->JumpPoints.size();
			bool updateJournal = false;
			for (; i < nbJumpPoints; i++ )
			{
				if ( templ->JumpPoints[i].Name == jump )
				{
					// inform client
					closeDynChat( user, botRow );

					std::list< CMissionEvent * > eventList;
					inst->jump( templ->JumpPoints[i].Step,templ->JumpPoints[i].Action,eventList );
					switch ( inst->getProcessingState() )
					{
					case CMission::Failed:
						inst->setProcessingState( CMission::Normal );
						inst->onFailure( true );
						break;
					case CMission::Complete:
						inst->setProcessingState( CMission::Normal );
						inst->forceSuccess();
						break;
					case CMission::InJump:
						inst->setProcessingState(CMission::Normal);
						updateJournal = true;
						// no break : we want to proceed events
					case CMission::Normal:
						while( !eventList.empty() )
						{
							user->processMissionEvent( *eventList.front() );
							delete eventList.front();
							eventList.pop_front();
						}
						updateJournal = true;
						break;
					}
					break;
				}
			}
			if ( updateJournal )
				inst->updateUsersJournalEntry();
			if ( i == nbJumpPoints )
			{
				nlwarning("<MISSIONS>Invalid jump point %s in mission %u",dynChat->Answers[choice].Jump.c_str(), missionAlias);
				return;
			}

			// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
			CharacterDynChatBeginEnd.DynChatEnd.push_back( botRow );

			_DynChats.erase(it);
			break;
		}
	}
	// open the next waiting dyn chat for this user
	CCreature * bot = CreatureManager.getCreature( botRow );
	if ( bot )
	{
		it = _DynChats.find( user->getEntityRowId() );
		while ( it != _DynChats.end() && (*it).first == user->getEntityRowId()  )
		{
			if ( botRow == (*it).second.Bot )
			{
				CMission * instance = (*it).second.Mission;
				CMissionTemplate* templ = getTemplate( instance->getTemplateId() );
				nlassert(templ);
				nlassert( (*it).second.StepIndex < templ->Steps.size() );
				CMissionStepDynChat * step = dynamic_cast<CMissionStepDynChat *> ( templ->Steps[ (*it).second.StepIndex ] );
				nlassert( step );
				CEntityId giverId = CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() );
				if ( giverId == CEntityId::Unknown )
				{
					nlwarning("<DYNCHAT> invalid giver '%u'",instance->getGiver() );
					return;
				}
				openDynChat( user,bot, step,giverId );
				break;
			}
			++it;
		}
	}
}// CMissionManager::dynChatChoice
*/


/*
 * Must be called for each openDynChat()
 */
inline void CMissionManager::closeDynChat( CCharacter * user, const TDataSetRow & botRow )
{
	user->staticActionInProgress(false);
	user->setCurrentInterlocutor(CEntityId::Unknown);
	CMessage msgout( "IMPULSION_ID" );
	CBitMemStream bms;
	msgout.serial( (CEntityId&)user->getId() );
	if ( ! GenericMsgManager.pushNameToStream( "BOTCHAT:DYNCHAT_CLOSE", bms) )
	{
		nlwarning("<CCharacter closeDynChat> Msg name BOTCHAT:DYNCHAT_CLOSE not found");
		return;
	}

	// Send to client
	TDataSetIndex index = botRow.getCompressedIndex();
	bms.serial( index );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(user->getId().getDynamicId()), msgout );
}// CMissionManager::closeDynChat


/*
 * closeDynChat() must be called for each openDynChat()
 */
inline void CMissionManager::openDynChat( CCharacter * user,const CCreature * bot,CMissionStepDynChat * dynChat, const NLMISC::CEntityId & giverId)
{
	// the user is now in static mode
	user->staticActionInProgress(true,STATIC_ACT_TYPES::BotChat);
	user->setCurrentInterlocutor(bot->getId());
	nlassert(user);
	nlassert(bot);
	nlassert(dynChat);

	// build the message params
	CMirrorPropValue<TYPE_NAME_STRING_ID> nameId( TheDataset, bot->getEntityRowId(), DSPropertyNAME_STRING_ID );
	uint32 name = nameId;

	TDataSetIndex index = bot->getEntityRowId().getCompressedIndex();

	std::vector<uint32> dynStrings(dynChat->Answers.size() + 1);

	TVectorParamCheck params = dynChat->Params;
	CMissionParser::solveEntitiesNames( params,user->getEntityRowId(),giverId );
	dynStrings[0] = STRING_MANAGER::sendStringToClient(user->getEntityRowId(),dynChat->PhraseId, params );
	for ( uint i = 1; i < dynStrings.size(); i++)
	{
		params = dynChat->Answers[i-1].Params;
		CMissionParser::solveEntitiesNames( params,user->getEntityRowId(),giverId );
		dynStrings[i] = STRING_MANAGER::sendStringToClient(user->getEntityRowId(),dynChat->Answers[i-1].PhraseId, params );
	}

	// Send to client
	CMessage msgout( "IMPULSION_ID" );
	CBitMemStream bms;
	msgout.serial( (CEntityId&)user->getId() );
	if ( ! GenericMsgManager.pushNameToStream( "BOTCHAT:DYNCHAT_OPEN", bms) )
	{
		nlwarning("<CCharacter open dyn chat> Msg name BOTCHAT:DYNCHAT_OPEN not found");
		return;
	}
	bms.serial(index);
	bms.serial(name);
	bms.serialCont(dynStrings);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(user->getId().getDynamicId()), msgout );

	// Send to AIS (to stop the bot). Important: there must be the same number of items pushed in DynChatEnd that in DynChatStart for the bot to resume.
	CharacterDynChatBeginEnd.DynChatStart.push_back( bot->getEntityRowId() );
}

inline void CMissionManager::initInstanciatedMission(CMission * inst, std::list< CMissionEvent * > & eventList)
{
	nlassert(inst);
	CMissionTemplate* templ = getTemplate( inst->getTemplateId() );
	nlassert(templ);

	// tag the last mission instantiation (for global repeat timer)
//	templ->MissionStats.LastTryDate = CTickEventHandler::getGameCycle();
//	templ->LastTryDate = CTickEventHandler::getGameCycle();
	templ->Instances.push_back(inst);

	if ( templ->MonoTimer )
	{
		inst->setMonoEndDate( CTickEventHandler::getGameCycle() + templ->MonoTimer );
		addMonoMission(inst);
	}

	if(inst->getProcessingState() == CMission::Normal)
		inst->setProcessingState(CMission::Init);
	for ( uint i = 0; i < templ->InitialActions.size() && inst->getProcessingState() == CMission::Init; i++ )
	{
		templ->InitialActions[i]->launch( inst, eventList );
		if(inst->getProcessingState() == CMission::Normal)
			inst->setProcessingState(CMission::Init);
	}

	switch ( inst->getProcessingState() )
	{
	case CMission::Failed:
		MISDBG("mission %s. Fail instruction encountered in an initial jump", CPrimitivesParser::aliasToString(templ->Alias).c_str() );
		inst->setProcessingState( CMission::Normal );
		inst->onFailure( true );
		return;
	case CMission::Complete:
		MISDBG("mission %s. End instruction encountered in an initial jump", CPrimitivesParser::aliasToString(templ->Alias).c_str() );
		inst->forceSuccess();
		return;
	case CMission::Normal:
	case CMission::Init:
		inst->setProcessingState(CMission::Normal);
		if ( templ->Type == MISSION_DESC::Guild )
		{
			/// todo guild mission
			/*
			CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild *>(inst);
			nlassert( mission );
			{

				CGuild * guild = mission->getGuild();
				while( !eventList.empty() )
				{
					guild->processGuildSpecificEvent( *eventList.front() );
					delete eventList.front();
					eventList.pop_front();
				}
			}
			*/
		}
		else
		{
			CCharacter * user = inst->getMainEntity();
			if (user)
			{
				while( !eventList.empty() )
				{
					if ( eventList.front()->Type != CMissionEvent::AddMission )
					{
						user->processMissionEvent(* eventList.front() );
						delete eventList.front();
					}
					else
					{
						user->processMissionEvent(* eventList.front() );
					}
					eventList.pop_front();
				}
			}
		}
		break;
	}

	/// activate the steps that should now be active
	inst->activateInitialSteps( eventList );
}

void CMissionManager::checkPlaceConstraints ( CMission* mission)
{
	nlassert(mission);
	CCharacter * user = mission->getMainEntity();
	if ( !user )
		return;


	/// check outside constraints
	for ( map<TAIAlias, EGSPD::CMissionOutsidePlacePD>::iterator it = mission->getOutsidePlacesBegin(); it != mission->getOutsidePlacesEnd(); ++it )
	{
		const uint size = (uint)user->getPlaces().size();
		for ( uint i = 0; i < size; i++ )
		{
			if ( (*it).second.getAlias() == user->getPlaces()[i] )
			{
				CPlace* place  = CZoneManager::getInstance().getPlaceFromId( user->getPlaces()[i] );
				if ( place )
				{
					std::vector<CPlaceChecker> & vect = _PlaceDependantMissions[ mission ];
					CPlaceChecker checker;
					checker.PlaceAlias =  place->getAlias();
					checker.EndDate = (*it).second.getDelay() + CTickEventHandler::getGameCycle();
					vect.push_back(checker);

					vector<TDataSetRow> entities;
					mission->getEntities( entities );

					SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::integer );
					params[0].Identifier = place->getName();
					params[1].Int = (*it).second.getDelay() / 10;
					for ( uint i = 0; i < entities.size(); i++ )
					{
						PHRASE_UTILITIES::sendDynamicSystemMessage(entities[i],"MIS_OUTSIDE_WARN", params);
					}
				}
			}
		}
	}
	/// check inside constraints
	map<TAIAlias, EGSPD::CMissionInsidePlacePD>::iterator itIn = mission->getInsidePlacesBegin();
	for (; itIn != mission->getInsidePlacesEnd(); ++itIn )
	{
		const uint size = (uint)user->getPlaces().size();
		uint i = 0;
		for (; i < size; i++ )
		{
			if ( (*itIn).second.getAlias() == user->getPlaces()[i] )
				break;
		}
		if ( i== size )
		{
			CPlace* place  = CZoneManager::getInstance().getPlaceFromAlias( (*itIn).second.getAlias() );
			if ( place )
			{
				std::vector<CPlaceChecker> & vect = _PlaceDependantMissions[ mission ];
				CPlaceChecker checker;
				checker.PlaceAlias = place->getAlias();
				checker.EndDate = (*itIn).second.getDelay() + CTickEventHandler::getGameCycle();
				vect.push_back(checker);

				vector<TDataSetRow> entities;
				mission->getEntities( entities );

				SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::integer );
				params[0].Identifier = place->getName();
				params[1].Int = (*itIn).second.getDelay() / 10;
				for ( uint i = 0; i < entities.size(); i++ )
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage(entities[i],"MIS_INSIDE_WARN", params);
				}
			}
		}
	}
}


void CMissionManager::enterPlace( CMission* mission,uint32 placeAlias,uint16 placeId )
{
	nlassert( mission );
	/// check inside constraints
	for ( map<TAIAlias, EGSPD::CMissionInsidePlacePD>::iterator it = mission->getInsidePlacesBegin(); it != mission->getInsidePlacesEnd(); ++it )
	{
		if ( (*it).second.getAlias() == placeAlias )
		{
			// user enters a place where he must stay. Remove an entry for our mission that are palce dependent
			map< CMission*, std::vector<CPlaceChecker> >::iterator it = _PlaceDependantMissions.find( mission );
			if ( it != _PlaceDependantMissions.end() )
			{
				vector<CPlaceChecker> & checks = (*it).second;
				for ( uint i = 0;  i < checks.size();)
				{
					if ( checks[i].PlaceAlias == placeAlias )
					{
						checks[i] = checks.back();
						checks.pop_back();
					}
					else
						i++;
				}
			}
		}
	}
	// check outside constraints
	for ( map<TAIAlias, EGSPD::CMissionOutsidePlacePD>::iterator it = mission->getOutsidePlacesBegin(); it != mission->getOutsidePlacesEnd(); ++it )
	{
		if ( (*it).second.getAlias() == placeAlias )
		{
			std::vector<CPlaceChecker> & vect = _PlaceDependantMissions[ mission ];
			CPlaceChecker checker;
			checker.PlaceAlias = placeAlias;
			checker.EndDate = (*it).second.getDelay() + CTickEventHandler::getGameCycle();
			vect.push_back(checker);

			vector<TDataSetRow> entities;
			mission->getEntities( entities );
			CPlace* place  = CZoneManager::getInstance().getPlaceFromId( placeId );
			if ( place )
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::integer );
				params[0].Identifier = place->getName();
				params[1].Int = (*it).second.getDelay() / 10;
				for ( uint i = 0; i < entities.size(); i++ )
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage(entities[i],"MIS_OUTSIDE_WARN", params);
				}
			}
		}
	}
}

void CMissionManager::leavePlace( CMission* mission,uint32 placeAlias,uint16 placeId )
{
	nlassert( mission );
	/// check inside constraints
	for ( map<TAIAlias, EGSPD::CMissionInsidePlacePD>::iterator it = mission->getInsidePlacesBegin(); it != mission->getInsidePlacesEnd(); ++it )
	{
		if ( (*it).second.getAlias() == placeAlias )
		{
			if ( (*it).second.getAlias() == placeAlias )
			{
				std::vector<CPlaceChecker> & vect = _PlaceDependantMissions[ mission ];
				CPlaceChecker checker;
				checker.PlaceAlias = placeAlias;
				checker.EndDate = (*it).second.getDelay() + CTickEventHandler::getGameCycle();
				vect.push_back(checker);

				vector<TDataSetRow> entities;
				mission->getEntities( entities );
				CPlace* place  = CZoneManager::getInstance().getPlaceFromId( placeId );
				if ( place )
				{
					SM_STATIC_PARAMS_2(params, STRING_MANAGER::place, STRING_MANAGER::integer );
					params[0].Identifier = place->getName();
					params[1].Int = (*it).second.getDelay() / 10;
					for ( uint i = 0; i < entities.size(); i++ )
					{
						PHRASE_UTILITIES::sendDynamicSystemMessage(entities[i],"MIS_INSIDE_WARN", params);
					}
				}
			}
		}
	}
	// check outside constraints
	for ( map<TAIAlias, EGSPD::CMissionOutsidePlacePD>::iterator it = mission->getOutsidePlacesBegin(); it != mission->getOutsidePlacesEnd(); ++it )
	{
		// user enters a place where he must stay. Remove an entry for our mission that are place dependent
		map< CMission*, std::vector<CPlaceChecker> >::iterator itp = _PlaceDependantMissions.find( mission );
		if ( itp != _PlaceDependantMissions.end() )
		{
			for ( uint i = 0;  i < (*itp).second.size();)
			{
				if ( (*itp).second[i].PlaceAlias == placeAlias )
				{
					(*itp).second[i] = (*itp).second.back();
					(*itp).second.pop_back();
				}
				else
					i++;
			}
		}
	}
}

void CMissionManager::cleanPlaceConstraint( CMission* mission,TAIAlias placeAlias )
{
	nlassert(mission);
	map< CMission*, std::vector<CPlaceChecker> >::iterator it = _PlaceDependantMissions.find( mission );
	if ( it != _PlaceDependantMissions.end() )
	{
		for ( uint i = 0; i < (*it).second.size();)
		{
			if ( (*it).second[i].PlaceAlias == placeAlias )
			{
				(*it).second[i] = (*it).second.back();
				(*it).second.pop_back();
			}
			else
				i++;
		}
	}
}

void CMissionManager::removePlayerReconnectHandlingMissions( CMission & mission )
{
	std::vector< CMission* >::iterator it = std::find( _PlayerReconnectHandlingMissions.begin(), _PlayerReconnectHandlingMissions.end(), &mission );
	if ( it != _PlayerReconnectHandlingMissions.end() )
	{
		*it = _PlayerReconnectHandlingMissions.back();
		_PlayerReconnectHandlingMissions.pop_back();
	}
}

void CMissionManager::addPlayerReconnectHandlingMissions( CMission & mission )
{
	std::vector< CMission* >::iterator it = std::find( _PlayerReconnectHandlingMissions.begin(), _PlayerReconnectHandlingMissions.end(), &mission );
	if (it == _PlayerReconnectHandlingMissions.end())
	{
		_PlayerReconnectHandlingMissions.push_back(&mission);
	}
}


void CMissionManager::removeCrashHandlingMissions( CMission & mission )
{
	std::vector< CMission* >::iterator it = std::find( _CrashHandlingMissions.begin(), _CrashHandlingMissions.end(), &mission );
	if ( it != _CrashHandlingMissions.end() )
	{
		*it = _CrashHandlingMissions.back();
		_CrashHandlingMissions.pop_back();
	}
}

void CMissionManager::addCrashHandlingMissions( CMission & mission )
{
	std::vector< CMission* >::iterator it = std::find( _CrashHandlingMissions.begin(), _CrashHandlingMissions.end(), &mission );
	if ( it == _CrashHandlingMissions.end() )
	{
		_CrashHandlingMissions.push_back(&mission);
	}
}


void CMissionManager::applyAICrashConsequences( NLNET::TServiceId aiServiceId )
{
	string name;
	if (CWorldInstances::instance().getAIInstanceNameFromeServiceId(aiServiceId,name))
	{
		const uint size = (uint)_CrashHandlingMissions.size();
		for ( uint i = 0; i < size; i++ )
		{
			if ( _CrashHandlingMissions[i] )
				_CrashHandlingMissions[i]->applyCrashHandler(false, name);
		}
	}
	else
	{
		nlinfo("<MISSIONS> Invalid AI instance service %u (AI crashed before registering)",aiServiceId.get());
	}
}

void CMissionManager::registerEscort( TAIAlias group, TAIAlias mission,  const NLMISC::CEntityId & user )
{
	if ( _EscortGroups.insert( std::make_pair((uint)group,mission) ).second == false )
		nlwarning("<MISSIONS> group %u already registered",group);
	if (  user != NLMISC::CEntityId::Unknown )
		_SoloEscorts.insert( make_pair( user, group ) );
}

void CMissionManager::unregisterEscort( TAIAlias group, const NLMISC::CEntityId & user )
{
	_EscortGroups.erase( (uint)group );
	if (  user != NLMISC::CEntityId::Unknown )
	{
		std::pair<CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator, CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator > bounds;
		bounds = _SoloEscorts.equal_range( user );
		for ( CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator it = bounds.first; it != bounds.second; ++it )
		{
			if ( (*it).second == group )
			{
				_SoloEscorts.erase(it);
				return;
			}
		}
	}
}

void CMissionManager::updateEscortTeam( const NLMISC::CEntityId & user )
{
	std::pair<CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator, CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator > bounds;
	bounds = _SoloEscorts.equal_range( user );

	CSetEscortTeamId msg;
	for ( CHashMultiMap<NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator it = bounds.first; it != bounds.second; ++it )
		msg.Groups.push_back( (*it).second );
	if ( !msg.Groups.empty() )
	{
		CCharacter* c = PlayerManager.getChar( user );
		if ( !c )
		{
			nlwarning("<MISSIONS> Invalid user %u",user.toString().c_str() );
			return;
		}
		CTeam * team = TeamManager.getRealTeam(	c->getTeamId() );
		if ( !team )
			TeamManager.addFakeTeam( c );
		msg.TeamId = c->getTeamId();

		CMirrorPropValueRO<uint32>	in(TheDataset, c->getEntityRowId(), DSPropertyAI_INSTANCE);
		msg.InstanceNumber = in;
		CWorldInstances::instance().msgToAIInstance(in, msg);
	}
}

void CMissionManager::checkEscortFailure( TAIAlias group , bool groupWiped)
{
	CHashMap<uint,TAIAlias>::iterator it = _EscortGroups.find( (uint) group);
	if ( it != _EscortGroups.end() )
	{
		const CMissionTemplate * templ = getTemplate( (*it).second );
		if ( templ )
		{
			for ( uint i = 0; i < templ->Instances.size(); i++ )
			{
				templ->Instances[i]->checkEscortFailure(groupWiped);
			}
		}
		else
			nlwarning("<MISSIONS> Invalid mission %u",(*it).second);
	}
}

bool CMissionManager::isMissionValid(std::string const& missionName, std::string const& hashKey)
{
	TMissionStatesContainer::iterator it = _MissionStates.find(missionName);

	bool allMissionValids = _ValidMissionStates.find("All")!=_ValidMissionStates.end();
	bool missionHasState = it!=_MissionStates.end();
	bool missionHasGoodHashKey = missionHasState && it->second.hashKey==hashKey;
	bool missionStateIsValid = missionHasState && _ValidMissionStates.find(it->second.state)!=_ValidMissionStates.end();

	// check that we want all missions or that mission has a validation state, has a correct hash key and that its state is one of the valid states
	return allMissionValids || (missionHasState && missionHasGoodHashKey && missionStateIsValid);
}

void CMissionManager::loadMissionValidationFile(std::string const& filename)
{
	using namespace std;

	// load valid states from global service config file
	CConfigFile::CVar* var = IService::getInstance()->ConfigFile.getVarPtr("ValidMissionStates");
	if (var)
	{
		for (uint i=0; i<var->size(); ++i)
			_ValidMissionStates.insert(var->asString(i));
	}

	// load the configuration file
	CConfigFile cf;
	string pathName = CPath::lookup(filename, false);

	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no mission will be valid", filename.c_str());
		return;
	}
	cf.load(pathName);

	set<string> authorizedStates;

	// get the variable
	var = cf.getVarPtr("AuthorizedMissionStates");
	if (var)
	{
		for (uint i=0; i<var->size(); ++i)
			authorizedStates.insert(var->asString(i));
	}
	int missionStatesFields = 3;
	var = cf.getVarPtr("MissionStatesFields");
	if (var)
	{
		missionStatesFields = var->asInt();
	}
	else
	{
		nlwarning("Mission validation file does not contain MissionStatesFields variable. Parsing may fail and corrupt data.");
	}
	var = cf.getVarPtr("MissionStates");
	if (var)
	{
		for (uint i=0; i<var->size()/missionStatesFields; ++i)
		{
			string mission = var->asString(i*missionStatesFields);
			string state = var->asString(i*missionStatesFields+1);
			string hashKey = var->asString(i*missionStatesFields+2);

			if (authorizedStates.empty() || authorizedStates.find(state)!=authorizedStates.end())
				_MissionStates.insert(make_pair(mission, CMissionState(mission, state, hashKey)));
		}
	}
}

/*
bool CMissionManager::dumpMissionStat(CLog &log, TAIAlias missionAlias)
{
	std::hash_map< uint,CMissionTemplate* >::iterator it(_MissionTemplates.find(missionAlias));

	if (it == _MissionTemplates.end())
	{
		log.displayNL("WRN : can't find mission for alias '%u'", missionAlias);
		return false;
	}

	CMissionTemplate *tpl = it->second;

	TGameCycle delay = CTickEventHandler::getGameCycle() - tpl->MissionStats.LastTryDate;
	string delayStr;
	if (tpl->MissionStats.LastTryDate == 0)
	{
		delayStr = "never tryed";
	}
	else
	{
		const uint oneMinute= 10*60;
		const uint oneHour= oneMinute *60;
		const uint oneDay = oneHour *24;
		const uint oneWeek = oneDay *7;
		const uint oneMonth = oneDay *30;
		const uint oneYear = oneDay *365;
		uint years = delay / oneYear;
		uint months = (delay - years * oneYear) / oneMonth;
		uint weeks = (delay - years * oneYear - months * oneMonth) / oneWeek;
		uint days = (delay - years * oneYear - months * oneMonth - weeks * oneWeek) / oneDay;
		uint hours = (delay - years * oneYear - months * oneMonth - weeks * oneWeek - days * oneDay) / oneHour;
		uint minutes = (delay - years * oneYear - months * oneMonth - weeks * oneWeek - days * oneDay - hours * oneHour) / oneMinute;

		if (years)
			goto display_months;
		else if (months)
			goto display_weeks;
		else if (weeks)
			goto display_days;
		else if (days)
			goto display_hours;
		else if (hours || minutes)
			goto display_minutes;

		// only display ticks
		delayStr = toString("%u ticks ", delay) + delayStr;

display_minutes:
		if (minutes)
			delayStr = toString("%u mn ", minutes) + delayStr;
display_hours:
		if (hours)
			delayStr = toString("%u h ", hours) + delayStr;
display_days:
		if (days)
			delayStr = toString("%u day ", days) + delayStr;
display_weeks:
		if (weeks)
			delayStr = toString("%u week ", weeks) + delayStr;
display_months:
		if (months)
			delayStr = toString("%u month ", months) + delayStr;
//display_years:
		if (years)
			delayStr = toString("%u year ", years) + delayStr;

		delayStr += "ago";
	}

	log.displayNL("Mission '%s' \t(Alias=%10u): %10u success, %10u failure, %10u abandon (%8u try), last try at %10u (%s)",
		CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias).c_str(),
		missionAlias,
		tpl->MissionStats.SuccessCount,
		tpl->MissionStats.FailCount,
		tpl->MissionStats.AbandonCount,
		tpl->MissionStats.SuccessCount+tpl->MissionStats.FailCount+tpl->MissionStats.AbandonCount,
		tpl->MissionStats.LastTryDate,
		delayStr.c_str()
		);

	return true;
}


////////////////////////////////////////
// functor for orderby clause in command displayMissionStats

struct TOrderByTry : public std::binary_function<const pair<TAIAlias, TMissionStats*>, const pair<TAIAlias, TMissionStats*>, bool>
{
	bool operator()(const pair<TAIAlias, TMissionStats*> ms1, const pair<TAIAlias, TMissionStats*> ms2) const
	{
		return ms1.second->AbandonCount+ms1.second->FailCount+ms1.second->SuccessCount < ms2.second->AbandonCount+ms2.second->FailCount+ms2.second->SuccessCount;
	}
};

struct TOrderBySuccess : public std::binary_function<const pair<TAIAlias, TMissionStats*>, const pair<TAIAlias, TMissionStats*>, bool>
{
	bool operator()(const pair<TAIAlias, TMissionStats*> ms1, const pair<TAIAlias, TMissionStats*> ms2) const
	{
		return ms1.second->SuccessCount < ms2.second->SuccessCount;
	}
};

struct TOrderByFail : public std::binary_function<const pair<TAIAlias, TMissionStats*>, const pair<TAIAlias, TMissionStats*>, bool>
{
	bool operator()(const pair<TAIAlias, TMissionStats*> ms1, const pair<TAIAlias, TMissionStats*> ms2) const
	{
		return ms1.second->FailCount < ms2.second->FailCount;
	}
};

struct TOrderByAbandon : public std::binary_function<const pair<TAIAlias, TMissionStats*>, const pair<TAIAlias, TMissionStats*>, bool>
{
	bool operator()(const pair<TAIAlias, TMissionStats*> ms1, const pair<TAIAlias, TMissionStats*> ms2) const
	{
		return ms1.second->AbandonCount< ms2.second->AbandonCount;
	}
};

struct TOrderByLastTry : public std::binary_function<const pair<TAIAlias, TMissionStats*>, const pair<TAIAlias, TMissionStats*>, bool>
{
	bool operator()(const pair<TAIAlias, TMissionStats*> ms1, const pair<TAIAlias, TMissionStats*> ms2) const
	{
		return ms1.second->LastTryDate < ms2.second->LastTryDate;
	}
};

NLMISC_COMMAND(displayMissionStats,
			   "Display statistical data for one or all the missions",
			   "[<mission_name>] | [orderby try|success|fail|abandon|lastTry [desc]] [limit <count> [startat <index>]")
{
	enum TOrderBy
	{
		ob_none,
		ob_try,
		ob_success,
		ob_fail,
		ob_abandon,
		ob_lastTry
	};

	TOrderBy	order = ob_none;
	uint		limit = UINT_MAX;
	bool		desc = false;
	uint		start = 0;

	if (args.size() == 1)
	{
		// this must be a single mission stat
		TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[0]);
		if (alias == CAIAliasTranslator::Invalid)
		{
			log.displayNL("Invalid mission name '%s'", args[0].c_str());
			return false;
		}

		return CMissionManager::getInstance()->dumpMissionStat(log, alias);
	}
	else
	{
		// mutiple mission stats
		vector<string> params = args;

		// 1st check the params
		if (params.size() != 0)
		{
			// first param must be 'orderby' or limit
			if (params[0] != "orderby" && params[0] != "limit")
			{
				log.displayNL("Syntaxe error in parameters, expression must begin with 'orderby' or 'limit'");
				return false;
			}

			while (!params.empty())
			{
				if (params[0] == "orderby")
				{
					params.erase(params.begin());
					if (params.empty())
					{
						log.displayNL("Syntax error : orderby must be followed by an order name");
						return false;
					}
					else
					{
						if (params.front() == "try")
							order = ob_try;
						else if (params.front() == "success")
							order = ob_success;
						else if (params.front() == "fail")
							order = ob_fail;
						else if (params.front() == "abandon")
							order = ob_abandon;
						else if (params.front() == "lastTry")
							order = ob_lastTry;
						else
						{
							log.displayNL("Syntax error : order name '%s' invalid", params.front().c_str());
							return false;
						}
					}
				}
				else if (params[0] == "limit")
				{
					params.erase(params.begin());
					if (params.empty())
					{
						log.displayNL("Synatax error: limit must be followed by a number of row");
						return false;
					}
					limit = atoui(params.front().c_str());
				}
				else if (params[0] == "desc")
				{
					if (order == ob_none)
					{
						log.displayNL("Syntax error: desc can only apear after an orderby clause");
						return false;
					}
					desc = true;
				}
				else if (params[0] == "startat")
				{
					if (limit == UINT_MAX)
					{
						log.displayNL("Syntax error: startat can only apear after limit clause");
						return false;
					}
					params.erase(params.begin());
					if (params.empty())
					{
						log.displayNL("Syntax error: startat must be followed by the mission row to start from");
						return false;
					}
					start = atoui(params.front().c_str());
				}
				else
				{
					log.displayNL("Syntax error: unknown parameter '%s'", params.front().c_str());
					return false;
				}

				params.erase(params.begin());
			}
		}

		// 2nd, gather the stat data in the appropiate container
		vector<pair<TAIAlias, TMissionStats*> > stats;

		CMissionManager &mm = *(CMissionManager::getInstance());

		std::hash_map< uint,CMissionTemplate* >::iterator first(mm._MissionTemplates.begin()), last(mm._MissionTemplates.end());

		for (; first !=last; ++first)
		{
			stats.push_back(make_pair(first->first, &(first->second->MissionStats)));
		}

		// 3rd, order by if needed
		switch(order)
		{
		case ob_try:
			std::sort(stats.begin(), stats.end(), TOrderByTry());
			break;
		case ob_success:
			std::sort(stats.begin(), stats.end(), TOrderBySuccess());
			break;
		case ob_fail:
			std::sort(stats.begin(), stats.end(), TOrderByFail());
			break;
		case ob_abandon:
			std::sort(stats.begin(), stats.end(), TOrderByAbandon());
			break;
		case ob_lastTry:
			std::sort(stats.begin(), stats.end(), TOrderByLastTry());
			break;
		}

		// 4th, display the result
		log.displayNL("Listing %u missions starting at pos %u (on a total of %u missions) : ", min(limit, stats.size()), min(start, stats.size()), stats.size());
		if (desc)
		{
			for (uint i=min(start, stats.size()); i<min(start+limit, stats.size()); ++i)
			{
				mm.dumpMissionStat(log, stats[stats.size()-1-i].first);
			}
		}
		else
		{
			for (uint i=min(start, stats.size()); i<min(limit+start, stats.size()); ++i)
			{
				mm.dumpMissionStat(log, stats[i].first);
			}
		}

		return true;
	}
}


*/

NLMISC_COMMAND(displayDynChats, "", "")
{
	if (args.size() != 0)
		return false;

	const CHashMultiMap<TDataSetRow,CMissionManager::CDynChat,TDataSetRow::CHashCode> & dynChats = CMissionManager::getInstance()->_DynChats;
	CHashMultiMap<TDataSetRow,CMissionManager::CDynChat,TDataSetRow::CHashCode>::const_iterator it;
	for (it = dynChats.begin(); it != dynChats.end(); ++it)
	{
		const TDataSetRow & playerRowId = (*it).first;
		const CMissionManager::CDynChat & dynChat = (*it).second;
		log.displayNL("DYNCHAT: player = %s, mission = %s, step index = %u, bot = %s",
			playerRowId.toString().c_str(),
			CPrimitivesParser::aliasToString( dynChat.Mission->getTemplateId() ).c_str(),
			dynChat.StepIndex,
			dynChat.Bot.toString().c_str()
			);
	}

	return true;
}

