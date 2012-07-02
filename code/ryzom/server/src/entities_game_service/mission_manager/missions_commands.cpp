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
#include "mission_manager/ai_alias_translator.h"
#include "nel/net/service.h"
#include "nel/misc/command.h"
#include "nel/misc/algo.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "mission_manager/mission_manager.h"
#include "primitives_parser.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_step_ai.h"
#include "mission_manager/mission_guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"

#include "admin.h"
#include "creature_manager/creature_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

NLMISC_COMMAND( forceMissionProgress,"debug command used to trigger debug commands","<user>" )
{
	if (args.empty() || args.size() > 3)
		return false;
	CEntityId id;
	id.fromString( args[0].c_str() );
	CCharacter * c = PlayerManager.getChar( id );
	if (!c)
		return true;
	CMissionEventDebug event;
	c->processMissionEvent(event);
	return true;
}

//-----------------------------------------------
// forceJournalUpdate
//-----------------------------------------------
NLMISC_COMMAND(forceJournalUpdate,"force mission journal update","<player id(id:type:crea:dyn)>")
{
	if ( args.size() != 1 )
		return false;
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar( id );
	if( !user )
	{
		log.displayNL("invalid char");
		return true;
	}
	for ( map<TAIAlias,CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
	{
		(*it).second->updateUsersJournalEntry();
	}

	CTeam * team = TeamManager.getTeam( user->getTeamId() );
	if ( team )
	{
		for ( uint i  = 0; i < team->getMissions().size(); i++ )
		{
			team->getMissions()[i]->updateUsersJournalEntry();
		}
	}
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
	if (guild)
	{
		for ( uint i  = 0; i < guild->getMissions().size(); i++ )
		{
			guild->getMissions()[i]->updateUsersJournalEntry();
		}
	}
	/*for (uint i = 0; i < MaxGuildMissionCount; i++)
	{
		/// todo guild mission
	}*/
	return true;
} // missionProgress //

//-----------------------------------------------
// simMissionEvent
//-----------------------------------------------
NLMISC_COMMAND(simMissionEvent,"simulate a mission event","<player> <event type> *[<event param>]")
{
	if ( args.empty() )
		return false;
	if ( !CMissionEvent::simMissionEvent( args,log ) )
		log.displayNL("simMissionEvent failed");
	return true;
} // simMissionEvent //

//-----------------------------------------------
// reload missions
//-----------------------------------------------
NLMISC_COMMAND(reloadMissions,"reload the mission primitives. Picked missions are erased","[bool telling if we have tio reset aliases (default : false)]")
{
	if ( args.size() > 1 )
		return true;
	CPlayerManager::TMapPlayers::const_iterator itPlayer = PlayerManager.getPlayers().begin();

	for (; itPlayer != PlayerManager.getPlayers().end(); ++itPlayer )
	{
		if ( (*itPlayer).second.Player )
		{
			CCharacter * user = (*itPlayer).second.Player->getActiveCharacter();
			if ( user )
			{
				while ( user->getMissionsBegin() != user->getMissionsEnd()  )
				{
					user->removeMission( ( *user->getMissionsBegin() ).first, mr_forced);
				}
				CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
				if ( team )
				{
					for ( uint i = 0; i < team->getMissions().size(); i++ )
					{
						team->removeMission( i, mr_forced);
					}
				}
				/// todo guild mission
			}
		}
	}

	bool reloadAliases = false;
	if ( args.size() == 1 && ( args[0] == "true" || args[0] == "1" ) )
		reloadAliases = true;
	CMissionManager::release();
	if ( reloadAliases )
	{
		log.displayNL( "please restart AI service" );
		CAIAliasTranslator::release();
	}
	if ( reloadAliases )
		CAIAliasTranslator::init();
	CMissionManager::init();
	log.displayNL( "missions reloaded" );
	return true;
} // reloadMissions



NLMISC_COMMAND(addSuccessfulMission,"add a successful mission to the player","<player > <mission alias>")
{
	if ( args.size() != 2 )
		return false;
	
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar( id );
	if (user)
	{
		TAIAlias alias;
		NLMISC::fromString(args[1], alias);
		const CHashMap< uint,CMissionTemplate* > &mts = CMissionManager::getInstance()->getMissionTemplates();
		if (mts.find(alias) == mts.end())
		{
			log.displayNL("Invalid mission");
			return true;
		}
		CMissionTemplate *mt = mts.find(alias)->second;
		if (mt)
			user->addSuccessfulMissions( *mt );
	}
	else
		log.displayNL("Invalid user");

	return true;
} // addSuccesfulMission



NLMISC_COMMAND(createMissionItem,"","")
{
	if ( args.size() != 3 )
		return false;
	string varName;

	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar(id);

	if ( !user )
	{
		log.displayNL( "invalid char" );
		return true;
	}

	uint16 quantity;
	NLMISC::fromString(args[1], quantity);

	std::vector< std::string > script;
	vector< pair<string, STRING_MANAGER::TParamType > > chatParams;
	NLMISC::splitString( args[2],":",script );

	CMissionItem item;
	item.buildFromScript( script,chatParams, varName );

	item.createItemInTempInv( user,quantity );
		
	return true;
} // createMissionItem

NLMISC_COMMAND(clearMissionDone,"Clear the list of already done missions.","<character id(id:type:crea:dyn)>")
{
	if (args.size() != 1)
		return false;
	
	CEntityId id;
	id.fromString( args[0].c_str() );
	
	CCharacter *c = PlayerManager.getChar(id);
	if( c == 0 )
	{
		log.displayNL("<clearMissionDone> unknown character '%s'", id.toString().c_str() );
		return false;
	}
	
	c->clearSuccessfullMission();
	return true;
}

NLMISC_COMMAND(clearMissionGlobalReplayTimer, "Clear the global replay timer of all mission", "")
{
	CMissionManager *mm = CMissionManager::getInstance();

	const CHashMap< uint,CMissionTemplate* >&mts = mm->getMissionTemplates();
	CHashMap< uint,CMissionTemplate* >::const_iterator first(mts.begin()), last(mts.end());
	for (; first != last; ++first)
	{
		const CMissionTemplate *mt = first->second;

		mt->resetGlobalReplayTimer();
	}

	log.displayNL("cleared %u mission global replay timer.", mts.size());
	return true;
}

NLMISC_COMMAND(displayMissionsPlayerStats, "display the missions statistic data for a player", "<character_id>")
{

	if (args.size() != 1)
	{
		log.displayNL("Syntax error: missing entity id");
		return false;
	}

	CEntityId eid(args[0]);

	if (eid == CEntityId::Unknown)
	{
		log.displayNL("Invalid entity id '%s'", args[0].c_str());
		return false;
	}

//	CCharacterManager *cm = CCharacterManager::getInstance();
	CCharacter *c = PlayerManager.getChar(eid);
	if (c == NULL)
	{
		log.displayNL("No charactere with entity id '%s'", args[0].c_str());
		return false;
	}

//	const std::vector< CMissionInstanceSolo* > & missions = c->getMissions();


	const std::map< TAIAlias, TMissionHistory > &mhs = c->getMissionHistories();
	log.displayNL("Listing %u missions history for player '%s'", mhs.size(), args[0].c_str());
	std::map< TAIAlias, TMissionHistory >::const_iterator firsth(mhs.begin()), lasth(mhs.end());
	for (; firsth != lasth; ++firsth)
	{
		const TMissionHistory &mh = firsth->second;
		bool	running = false;

		map<TAIAlias, CMission*>::iterator first(c->getMissionsBegin()), last(c->getMissionsEnd());
		for (; first != last; ++first)
		{
			CMission *mission = first->second;
			if (mission->getTemplateId() == firsth->first)
			{
				running = true;
				break;
			}
		}

		log.displayNL("Mission '%s' \t(alias %10u) : Success:%s, Status:%s, last success : %s",
			CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(firsth->first).c_str(),
			firsth->first,
			mh.Successfull ? "YES":"NO",
			running ? "RUNNING":"NOT RUNNING",
			mh.LastSuccessDate != 0 ? toString("%10u ticks ago", CTickEventHandler::getGameCycle() - mh.LastSuccessDate).c_str() : "never");
	}

	return true;
}



NLMISC_COMMAND(simAISMAction, "simulate an AI action.", "<action name> *[params]")
{
	if ( args.size() == 2 )
	{
		TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( args[1] );
		if ( alias != CAIAliasTranslator::Invalid )
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(alias);
			if ( templ != NULL )
			{
				if ( args[0] == "end_escort" )
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
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str() );
					}
				}
				else if ( args[0] == "fail" )
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
							nlwarning( "<CCAisActionMsgImp callback> *fail* mission %s  has a NULL instance ",args[1].c_str() );
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
									CMissionEventAIMsg event(args[0]);
									user->processMissionEvent( event, alias );
								}
								else
									nlwarning( "<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex() );
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str() );
					}
				}
			}
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// failMissionCategory
//-----------------------------------------------
NLMISC_COMMAND(failMissionCategory,"fail all missions of a specified category for all players","<mission_category>")
{
	if (args.size() != 1)
		return false;
	
	string sMissCat = strlwr(args[0]);

	CMissionManager *pMM = CMissionManager::getInstance();

	// For all characters connected
	const CPlayerManager::TMapPlayers &allPlayers = PlayerManager.getPlayers();
	CPlayerManager::TMapPlayers::const_iterator it = allPlayers.begin();
	while (it != allPlayers.end())
	{
		CPlayer *pPlayer = it->second.Player;
		if (pPlayer != NULL)
		{
			CCharacter *pChar = pPlayer->getActiveCharacter();
			if (pChar != NULL)
			{
				// Parse all missions to see if there is a mission that belongs to the mission_category
				std::map<TAIAlias, CMission*>::iterator itMiss = pChar->getMissionsBegin();
				while (itMiss != pChar->getMissionsEnd())
				{
					map<TAIAlias, CMission*>::iterator itNext = itMiss;
					++itNext;

					CMission *pMiss = itMiss->second;
					if (pMiss != NULL)
					{						
						CMissionTemplate *pMissTemplate;
						bool bFailed = false;
						// Check the mission template category
						pMissTemplate = pMM->getTemplate(pMiss->getTemplateId());
						if (pMissTemplate != NULL)
							if (strlwr(pMissTemplate->MissionCategory) == sMissCat)
							{
								pMiss->onFailure(true, false);
								bFailed = true;
							}

						// and the main mission template category
						if (!bFailed) // do not fail a mission twice 
						{
							pMissTemplate = pMM->getTemplate(pMiss->getMainMissionTemplateId());
							if (pMissTemplate != NULL)
								if (strlwr(pMissTemplate->MissionCategory) == sMissCat)
									pMiss->onFailure(true, false);
						}
					}
					// next mission
					itMiss = itNext;
				}
			}
		}
		// next player
		++it;
	}

	return true;
} // failMissionCategory

//-----------------------------------------------
// characterMissionDump
//-----------------------------------------------
NLMISC_COMMAND(characterMissionDump,"dump character missions","<character_id>")
{
	if (args.size() != 1)
		return false;

	GET_CHARACTER

	std::string text;
	uint i = 0;
	for ( map<TAIAlias, CMission*>::iterator it = c->getMissionsBegin(); it != c->getMissionsEnd(); ++it )
	{
		log.displayNL("- %2d: Mission '%s' (alias %10u) Bot Giver Alias: %10u", i++,
			CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId((*it).first).c_str(),
			(*it).first,
			(*it).second->getGiver());
	}

	return true;
}

//-----------------------------------------------
// removeMission
//-----------------------------------------------
NLMISC_COMMAND(removeMission,"Remove mission of character","<character_id> <mission alias>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER

	TAIAlias missionAlias;
	NLMISC::fromString(args[1], missionAlias);
	c->removeMission(missionAlias, 0);
	c->removeMissionFromHistories(missionAlias);

	log.displayNL("Mission '%s' \t(alias %10u)  removed from character %s",
	CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias).c_str(),
	missionAlias, 
	args[0].c_str());

	return true;
}

//-----------------------------------------------
// removeMission
//-----------------------------------------------
NLMISC_COMMAND(addMission,"Add mission to character","<character_id> <Mission giver Alias> <mission alias>")
{
	if (args.size() != 3)
		return false;

	GET_CHARACTER
	
	TAIAlias giverAlias;
	NLMISC::fromString(args[1], giverAlias);	

	TAIAlias missionAlias;
	NLMISC::fromString(args[2], missionAlias);	

	c->endBotChat();
	c->setAfkState(false);
	
	std::list< CMissionEvent* > eventList;
	CMissionManager::getInstance()->instanciateMission(c, missionAlias,	giverAlias, eventList);
	c->processMissionEventList( eventList,true, CAIAliasTranslator::Invalid );

	log.displayNL("Mission '%s' \t(alias %10u)  added to character %s",
		CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias).c_str(),
		missionAlias, 
		args[0].c_str());

	return true;
}
