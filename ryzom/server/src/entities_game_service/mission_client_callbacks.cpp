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
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_queue_manager.h"
#include "egs_pd.h"
#include "egs_utils.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "team_manager/team_manager.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_log.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_member.h"


using namespace std;
using namespace NLMISC;

//----------------------------------------------------------------------------
bool checkPlayer(const CEntityId &userId, const string &sFuncName)
{
	CCharacter * user = PlayerManager.getChar( userId );
	if ( !user )
	{
		MISLOG("user:%s %s : Invalid char", userId.toString().c_str(), sFuncName.c_str());
		return false;
	}
	// check character is ready
	if (!user->getEnterFlag())
	{
		MISLOG("user:%s %s: Player not ready", userId.toString().c_str(), sFuncName.c_str());
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void cbClientBotChatChooseStaticMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	uint16 session;
	msgin.serial(userId, session);
	
	if (!checkPlayer(userId, "cbClientBotChatChooseStaticMission")) return;
	CCharacter * user = PlayerManager.getChar( userId );

	user->setAfkState(false);
	// start a bot chat
	CCreature  * c = user->startBotChat( BOTCHATTYPE::ChooseMissionFlag );
	if ( !c )
	{
		return;
	}
	user->buildMissionList(c,session);
}

//----------------------------------------------------------------------------
void cbClientBotChatNextMissionPage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	uint16 session;
	msgin.serial(userId,session);
	
	if (!checkPlayer(userId, "cbClientBotChatNextMissionPage")) return;
	CCharacter * user = PlayerManager.getChar( userId );

	user->setAfkState(false);
	user->fillMissionPage(session);
}//	cbClientBotChatNextMissionPage

//----------------------------------------------------------------------------
void cbClientBotChatPickStaticMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	if ( !MissionSystemEnabled )
		return;
	CEntityId userId;
	uint8 index;
	msgin.serial(userId);
	msgin.serial(index);
	//get the user and the asked mission
	if (!checkPlayer(userId, "cbClientBotChatPickStaticMission")) return;
	CCharacter	* user = PlayerManager.getChar( userId );
	
	// Create the debug string
	string sDebugPrefix = string("user:") + userId.toString().c_str() + " cbClientBotChatPickStaticMission :";

	// Check for user
	if ( user == NULL )
	{
		MISLOG("%s cant find user.", sDebugPrefix.c_str());
		return;
	}

	// Check that index falls in good range
	if ( index >= user->getCurrentMissionList().size() )
	{
		MISLOG("%s invalid index %u, there are %u", sDebugPrefix.c_str(), index, user->getCurrentMissionList().size() );
		return;
	}

	// Check if the user is not cheating (trying to get a mission grayed)
	if ( user->getCurrentMissionList()[index].PreReqState != MISSION_DESC::PreReqSuccess)
	{
		MISLOG("%s user choose a mission with failed prerequesit (index %u)", sDebugPrefix.c_str(), index);
		return;
	}

	CCreature * bot = CreatureManager.getCreature( user->getCurrentInterlocutor() );
	if (!bot || bot->getAlias() == CAIAliasTranslator::Invalid )
	{
		MISLOG("%s invalid interlocutor", sDebugPrefix.c_str());
		return;
	}

	// End the user bot chat (must be done before instanciateMission() because it would clear any dynchat's interlocutor open by the mission
	user->endBotChat();
	user->setAfkState(false);

	std::list< CMissionEvent* > eventList;
	CMissionManager::getInstance()->instanciateMission(user, user->getCurrentMissionList()[index].Mission,
														bot->getAlias(), eventList);
	user->processMissionEventList( eventList,true, CAIAliasTranslator::Invalid );
}

//----------------------------------------------------------------------------
void cbClientContinueMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	uint8 index;
	msgin.serial(userId);
	msgin.serial(index);

	if (!checkPlayer(userId, "cbClientContinueMission")) return;
	CCharacter	* user = PlayerManager.getChar( userId );

	if ( !user->startBotChat( BOTCHATTYPE::UnknownFlag ) )
		return;
	user->botChatMissionAdvance(index);
	user->setAfkState(false);
}

//----------------------------------------------------------------------------
void cbClientValidateMissionGift( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);

	if (!checkPlayer(userId, "cbClientContinueMission")) return;
	CCharacter	* user = PlayerManager.getChar( userId );

	user->acceptExchange(0);
	user->setAfkState(false);
}

//----------------------------------------------------------------------------
void cbClientBotChatEndGift( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);

	if (!checkPlayer(userId, "cbClientBotChatEndGift")) return;
	CCharacter* user = PlayerManager.getChar( userId );

	user->clearBotGift();
	user->setAfkState(false);
}

//----------------------------------------------------------------------------
void cbClientBotChatDynChatSend( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	TDataSetIndex index;
	uint8 choice;
	msgin.serial(userId);
	msgin.serial(index);
	msgin.serial(choice);

	if (!checkPlayer(userId, "cbClientBotChatDynChatSend")) return;
	CCharacter* user = PlayerManager.getChar( userId );

	CMissionManager::getInstance()->dynChatChoice( user, TheDataset.getCurrentDataSetRow(index) , choice);
	user->setAfkState(false);
}

//----------------------------------------------------------------------------
void cbClientAbandonMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	uint8 index;
	msgin.serial(userId,index);

	if (!checkPlayer(userId, "cbClientAbandonMission")) return;
	CCharacter * user = PlayerManager.getChar( userId );

	user->abandonMission( index );
	user->setAfkState(false);
}

//----------------------------------------------------------------------------
void cbClientGroupAbandonMission( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	uint8 index;
	msgin.serial(userId,index);

	if (!checkPlayer(userId, "cbClientGroupAbandonMission")) return;
	CCharacter * user = PlayerManager.getChar( userId );

	user->setAfkState(false);

	// We check if it's a guild or team mission
	if (index < MaxGroupMissionCount)
	{
		// Team
		CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
		if ( !team )
		{
			MISLOG("user:%s cbClientGroupAbandonMission : Invalid team", userId.toString().c_str());
			return;
		}

		if ( team->getLeader() != userId )
		{
			CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "REQ_LEADER_TO_ABANDON_MISSION" );
			return;
		}

		if ( index >=  team->getMissions().size() )
		{
			MISLOG("user:%s cbClientGroupAbandonMission : Invalid group mission %u ( count %u )", 
							userId.toString().c_str(), index, team->getMissions().size());
			return;
		}

		CMissionTeam* mission = team->getMissions()[index];
		nlassert(mission);

		if ( mission->getFinished() == false )
		{
			CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
			if ( !templ )
			{
				MISLOG("user:%s cbClientGroupAbandonMission : invalid group mission alias %u", 
					userId.toString().c_str(), mission->getTemplateId());
				return;
			}
			if ( templ->Tags.NonAbandonnable )
			{
				MISLOG("user:%s cbClientGroupAbandonMission : group mission alias %u is not abandonnable but user tries to abandon it", 
					userId.toString().c_str(), mission->getTemplateId());
				return;
			}
			set<CEntityId> excluded;
			excluded.insert( userId );

			team->sendDynamicMessageToMembers( "ABANDON_GROUP_MISSION",TVectorParamCheck(), excluded );
		}
		team->removeMission( index, mr_abandon );
	}
	else
	{
		// Guild
		// We set the correct index
		index = MaxGroupMissionCount - index;

		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
		if ( !guild )
		{
			MISLOG("user:%s cbClientGroupAbandonMission : Invalid team", userId.toString().c_str());
			return;
		}
		if ( guild->getLeader()->getIngameEId() != userId )
		{
			CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "REQ_LEADER_TO_ABANDON_MISSION" );
			return;
		}

		if ( index >=  guild->getMissions().size() )
		{
			MISLOG("user:%s cbClientGroupAbandonMission : Invalid group mission %u ( count %u )", 
				userId.toString().c_str(), index, guild->getMissions().size());
			return;
		}

		CMissionGuild* mission = guild->getMissions()[index];
		nlassert(mission);

		if ( mission->getFinished() == false )
		{
			CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
			if ( !templ )
			{
				MISLOG("user:%s cbClientGroupAbandonMission : invalid group mission alias %u", 
					userId.toString().c_str(), mission->getTemplateId());
				return;
			}
			if ( templ->Tags.NonAbandonnable )
			{
				MISLOG("user:%s cbClientGroupAbandonMission : group mission alias %u is not abandonnable but user tries to abandon it", 
					userId.toString().c_str(), mission->getTemplateId());
				return;
			}
			set<CEntityId> excluded;
			excluded.insert( userId );

			guild->sendDynamicMessageToMembers( "ABANDON_GROUP_MISSION",TVectorParamCheck(), excluded );
		}
		guild->removeMission( index, mr_abandon );
	}
}

//----------------------------------------------------------------------------
void cbClientMissionWake( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	
	uint8 missionIndex;
	msgin.serial(missionIndex);
	
	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user || !user->getEnterFlag() )
		return;
	
	user->setAfkState(false);
	
	CMission* mission = NULL;
	CMissionTemplate * templ = NULL;
	for ( map<TAIAlias, CMission*>::iterator it =  user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
	{
		mission = (*it).second;
		if ( mission && mission->getClientIndex() == missionIndex )
		{
			templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
			if ( templ->Tags.NoList ) // skip invisible missions
				continue;
			break;
		}
	}
	if ( !mission )
	{
		MISDBG( "user:%s abandonMission : ERROR : invalid mission index %u sent ",userId.toString().c_str(), missionIndex);
		return;
	}
	if ( mission->getFinished() == false )
	{
		if ( !templ )
		{
			MISDBG( "user:%s abandonMission : ERROR : invalid mission alias %u", userId.toString().c_str(), mission->getTemplateId() );
			return;
		}
	}
	
	CMissionQueueManager::getInstance()->playerWakesUp (userId, templ->Alias );
}

//----------------------------------------------------------------------------
void cbClientMissionGroupWake( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId userId;
	msgin.serial(userId);
	
	uint8 missionGroupIndex;
	msgin.serial(missionGroupIndex);
	
	CCharacter * user = PlayerManager.getChar(userId);
	if ( !user || !user->getEnterFlag() )
		return;
	
	user->setAfkState(false);
	
	CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
	if ( !team )
	{
		MISLOG("user:%s cbClientMissionGroupWake : Invalid team", userId.toString().c_str());
		return;
	}
	if ( team->getLeader() != userId )
	{
		return;
	}
	
	if ( missionGroupIndex >=  team->getMissions().size() )
	{
		MISLOG("user:%s cbClientMissionGroupWake : Invalid group mission %u ( count %u )", 
			userId.toString().c_str(), missionGroupIndex, team->getMissions().size());
		return;
	}
	
	
	CMissionTeam* mission = team->getMissions()[missionGroupIndex];
	nlassert(mission);
	
	if ( mission->getFinished() == false )
	{
		CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
		if ( !templ )
		{
			MISLOG("user:%s cbClientGroupAbandonMission : invalid group mission alias %u", 
				userId.toString().c_str(), mission->getTemplateId());
			return;
		}
		
		CMissionQueueManager::getInstance()->playerWakesUp (userId, templ->Alias );
	}	
}
