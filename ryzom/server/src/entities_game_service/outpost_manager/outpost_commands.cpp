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

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_member_module.h"
#include "outpost_manager/outpost.h"
#include "outpost_manager/outpost_manager.h"
#include "primitives_parser.h"
#include "admin.h"


//----------------------------------------------------------------------------
// namespaces
//----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

static uint32 const seconds = 1;
static uint32 const minutes = 60*seconds;
static uint32 const hours = 60*minutes;
static uint32 const days = 24*hours;

//----------------------------------------------------------------------------
// helpers
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// get an outpost from its sheet or alias
static inline
CSmartPtr<COutpost> getOutpostFromString(const std::string & outpostString, CLog & log)
{
	CSmartPtr<COutpost> outpost;

	TAIAlias outpostAlias = CPrimitivesParser::aliasFromString(outpostString);
	outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost == NULL)
	{
		CSheetId outpostSheet(outpostString);
		outpost = COutpostManager::getInstance().getOutpostFromSheet(outpostSheet);
	}

	if (outpost == NULL)
	{
		log.displayNL("Unknown outpost : %s", outpostString.c_str());
	}

	return outpost;
}


//----------------------------------------------------------------------------
// commands
//
// NOTE: do not forget to register commands in admin.cpp if you want to
// make them accessible from the client
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostChallengeByGuild, "Challenges an outpost", "<outpost_id> <guild_name>")
{
	if (args.size()!=2)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	CGuild * guild = CGuildManager::getInstance()->getGuildByName( args[1] );
	if ( guild == NULL )
	{
		log.displayNL("Invalid guild '%s'", args[1].c_str());
		return true;
	}

	outpost->challengeOutpost( guild );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostGetCurrentTime, "", "")
{
	log.displayNL("Current time is %d", CTime::getSecondsSince1970());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSimulateTimer0End, "", "<outpost_id> [<absolute end time> | +<time to end>]")
{
	if (args.size()<1)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 endTime = 1;
	if (args.size()>1)
	{
		NLMISC::fromString(args[1], endTime);
		if (args[1].find('+')==0)
			endTime += CTime::getSecondsSince1970();
	}
	if (endTime==0) endTime = 1;
	
	outpost->simulateTimer0End(endTime);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSimulateTimer1End, "", "<outpost_id> [<absolute end time> | +<time to end>]")
{
	if (args.size()<1)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 endTime = 1;
	if (args.size()>1)
	{
		NLMISC::fromString(args[1], endTime);
		if (args[1].find('+')==0)
			endTime += CTime::getSecondsSince1970();
	}
	if (endTime==0) endTime = 1;
	
	outpost->simulateTimer1End(endTime);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSimulateTimer2End, "", "<outpost_id> [<absolute end time> | +<time to end>]")
{
	if (args.size()<1)
		return false;
	
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 endTime = 1;
	if (args.size()>1)
	{
		NLMISC::fromString(args[1], endTime);
		if (args[1].find('+')==0)
			endTime += CTime::getSecondsSince1970();
	}
	if (endTime==0) endTime = 1;
	
	outpost->simulateTimer2End(endTime);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetFightData, "", "<outpost_id> <current_round> [<current_level>] [<max_attack_level>]  [<max_defense_level>]")
{
	if (args.size() < 2 || args.size() > 5)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	if (args.size() > 1)
	{
		uint32 currentRound;
		NLMISC::fromString(args[1], currentRound);
		if (currentRound > 0 && currentRound <= outpost->computeRoundCount())
		{
			outpost->_FightData._CurrentCombatRound = currentRound - 1;
		}
		else
			log.displayNL("Invalid current round %u (round count = %u)", currentRound, outpost->computeRoundCount());
	}

	if (args.size() > 2)
	{
		uint32 currentLevel;
		NLMISC::fromString(args[2], currentLevel);
		if (currentLevel > 0 && currentLevel <= outpost->computeRoundCount())
		{
			outpost->_FightData._CurrentCombatLevel = currentLevel - 1;
		}
		else
			log.displayNL("Invalid current level %u (round count = %u)", currentLevel, outpost->computeRoundCount());
	}

	if (args.size() > 3)
	{
		uint32 maxAttackLevel;
		NLMISC::fromString(args[3], maxAttackLevel);
		if (maxAttackLevel <= outpost->computeRoundCount())
		{
			outpost->_FightData._MaxAttackLevel = maxAttackLevel;
		}
		else
			log.displayNL("Invalid max attack level %u (round count = %u)", maxAttackLevel, outpost->computeRoundCount());
	}

	if (args.size() > 4)
	{
		uint32 maxDefenseLevel;
		NLMISC::fromString(args[4], maxDefenseLevel);
		if (maxDefenseLevel <= outpost->computeRoundCount())
		{
			outpost->_FightData._MaxDefenseLevel = maxDefenseLevel;
		}
		else
			log.displayNL("Invalid max defense level %u (round count = %u)", maxDefenseLevel, outpost->computeRoundCount());
	}

	outpost->askGuildDBUpdate(COutpostGuildDBUpdater::OUTPOST_PROPERTIES);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setOutpostLevel, "Set the outpost level", "<outpost id><level>" )
{
	if (args.size() != 2)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 level;
	NLMISC::fromString(args[1], level);
	outpost->setOutpostCurrentLevel(level);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostDumpOutpostList, "", "")
{
	if (args.size() != 0)
		return false;

	COutpostManager::getInstance().dumpOutpostList(log);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostDumpOutpost, "", "<outpost_id>")
{
	if (args.size() != 1)
		return false;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	outpost->dumpOutpost(log);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostAccelerateConstruction, "set all current construction to a certain amount of time (default 30s)", "<time_left_in_second>")
{
	if (args.size() > 1) return false;

	uint nNbSecondLeft = 30;
	if (args.size() == 1)
		NLMISC::fromString(args[0], nNbSecondLeft);

	COutpostManager::getInstance().setConstructionTime(nNbSecondLeft);
	
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostPlayerOutpostInfos, "get the outpost infos of a player", "<player_id>")
{
	if (args.size() != 1) return false;
	
	GET_CHARACTER
		
	string str;
	uint32 outpostAlias = c->getOutpostAlias();
	OUTPOSTENUMS::TPVPSide side = c->getOutpostSide();
	if( outpostAlias != 0 )
	{
		str += CPrimitivesParser::aliasToString(outpostAlias);
		str += " - ";
		str += OUTPOSTENUMS::toString(side);
	}
	else
	{
		str = "no outpost";
	}
	
	log.displayNL("Player %s outpost : %s", c->getId().toString().c_str(), str.c_str());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostChallengeOutpost, "Challenges an outpost", "<player_id> <outpost_id>")
{
	if (args.size()!=2)
		return false;

	GET_CHARACTER

	CGuildMemberModule * module;
	if ( !c->getModuleParent().getModule( module ) )
		return true;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[1], log);
	if (outpost == NULL)
		return true;

	module->challengeOutpost(outpost->getSheet(), false);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostGiveupOutpost, "Giveup an outpost, letting its ownership to the tribe", "<player_id> <outpost_id>")
{
	if (args.size()!=2)
		return false;
	
	GET_CHARACTER

	CGuildMemberModule * module;
	if ( !c->getModuleParent().getModule( module ) )
		return true;

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[1], log);
	if (outpost == NULL)
		return true;
	
	module->giveupOutpost(outpost->getSheet());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostDisplayGuildOutposts, "Display the outposts owned or being attacked by the player guild and their status.", "<player_id>" )
{
	if ( args.size() < 1 )
		return false;
	
	GET_CHARACTER

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( c->getGuildId() );
	if (guild == NULL)
		return true;

	vector<TAIAlias> ownedOutposts;
	vector<TAIAlias> challengedOutposts;
	guild->getOwnedOutposts(ownedOutposts);
	guild->getChallengedOutposts(challengedOutposts);

	std::string text;
	if (!ownedOutposts.empty())
	{
		text += "Outposts owned by the guild are:\n";
		for (uint i = 0; i < ownedOutposts.size(); ++i)
		{
			CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( ownedOutposts[i] );
			text += "  ";
			if (outpost != NULL)
				text += outpost->toString();
			else
				text += "cannot find outpost " + CPrimitivesParser::aliasToString( ownedOutposts[i] );
			text += "\n";
		}
	}
	else
	{
		text += "Guild does not own any outpost.\n";
	}

	if (!challengedOutposts.empty())
	{
		text += "Outposts attacked by the guild are:\n";
		for (uint i = 0; i < challengedOutposts.size(); ++i)
		{
			CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( challengedOutposts[i] );
			text += "  ";
			if (outpost != NULL)
				text += outpost->toString();
			else
				text += "cannot find outpost " + CPrimitivesParser::aliasToString( challengedOutposts[i] );
			text += "\n";
		}
	}
	else
	{
		text += "Guild does not attack any outpost.\n";
	}

	SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
	params[0].Literal = text;
	CCharacter::sendDynamicSystemMessage( c->getId(), "LITERAL", params );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostForceOpenGuildInventory, "", "<player_id>" )
{
	if ( args.size() < 1 )
		return false;
	
	GET_CHARACTER
	
	PlayerManager.sendImpulseToClient(eid, "GUILD:OPEN_INVENTORY");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostForceCloseGuildInventory, "", "<player_id>" )
{
	if ( args.size() < 1 )
		return false;
	
	GET_CHARACTER
	
	PlayerManager.sendImpulseToClient(eid, "GUILD:CLOSE_INVENTORY");
	
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetPlayerPvpSide, "changes the outpost infos of a player", "<player_id> [<outpost_id> attacker|defender]")
{
	if (args.size() != 3 && args.size() != 1) return false;
	
	GET_CHARACTER
	
	if (args.size()==3)
	{
		CSmartPtr<COutpost> outpost = getOutpostFromString(args[1], log);
		if (outpost == NULL)
			return true;

		OUTPOSTENUMS::TPVPSide side = OUTPOSTENUMS::UnknownPVPSide;
		if (args[2]=="attacker")
			side = OUTPOSTENUMS::OutpostAttacker;
		else if (args[2]=="defender")
			side = OUTPOSTENUMS::OutpostOwner;
		else
		{
			log.displayNL("Invalid side specified");
			return false;
		}
		
		c->setOutpostAlias(outpost->getAlias());
		c->setOutpostSide(side);
		OUTPOST_INF("Player %s outpost side set to %s %s", c->getId().toString().c_str(), CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(), side?"attacker":"defender");
	}
	else
	{
		uint32 outpostAlias = c->getOutpostAlias();
		OUTPOSTENUMS::TPVPSide side = c->getOutpostSide();
		log.displayNL("Player %s outpost side set to %s %s", c->getId().toString().c_str(), CPrimitivesParser::aliasToString(outpostAlias).c_str(), OUTPOSTENUMS::toString(side).c_str());
	}
	
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetOutpostToPlayer, "set the player's guild as the owner of the outpost", "<player_id> <outpost_id>")
{
	if (args.size() != 2) return false;
	
	GET_CHARACTER
		
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[1], log);
	if (outpost == NULL)
		return true;
	
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( c->getGuildId() );
	if (guild != NULL)
	{
		if (!guild->canAddOutpost())
		{
			log.displayNL("the guild '%s' cannot get one more outpost", guild->getName().toUtf8().c_str());
			return true;
		}
		if (outpost->getState() != OUTPOSTENUMS::Peace)
			outpost->giveupAttack();
		outpost->setOwnerGuild( c->getGuildId() );
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetOutpostOwner, "set the owner of the outpost", "<outpost_id> guild|tribe [<guild_name>]")
{
	if (args.size() < 2 || args.size() > 3)
		return false;
	
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	const string & ownerType = args[1];
	EGSPD::TGuildId ownerId;
	if (ownerType == "tribe")
	{
		ownerId = 0;
	}
	else if (ownerType == "guild")
	{
		if (args.size() < 3)
			return false;

		const string & guildName = args[2];
		CGuild * guild = CGuildManager::getInstance()->getGuildByName(guildName);
		if (guild == NULL)
		{
			log.displayNL("unknown guild : '%s'", guildName.c_str());
			return true;
		}
		ownerId = guild->getId();

		if (!guild->canAddOutpost())
		{
			log.displayNL("the guild '%s' cannot get one more outpost", guild->getName().toUtf8().c_str());
			return true;
		}
	}
	else
	{
		log.displayNL("invalid owner type : '%s' (it must be 'guild' or 'tribe')", ownerType.c_str());
		return true;
	}

	if (outpost->getState() != OUTPOSTENUMS::Peace)
		outpost->giveupAttack();
	outpost->setOwnerGuild(ownerId);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSelectOutpost, "select an outpost to show it info", "<player_id> <outpost_id>")
{
	if (args.size() != 2) return false;
	
	GET_CHARACTER

	// remove previously selected outpost if any
	TAIAlias outpostAlias = c->getSelectedOutpost();
	CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost != NULL)
		outpost->removeOutpostDBRecipient(c->getId());

	// select the wanted outpost
	outpost = getOutpostFromString(args[1], log);
	if (outpost == NULL)
		return true;

	c->setSelectedOutpost(outpost->getAlias());
	outpost->addOutpostDBRecipient(c->getId());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostUnselectOutpost, "unselect an outpost", "<player_id>")
{
	if (args.size() != 1) return false;
	
	GET_CHARACTER

	TAIAlias outpostAlias = c->getSelectedOutpost();
	CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost)
		outpost->removeOutpostDBRecipient(c->getId());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetAttackDefenseHour, "Set attack and defense time of an outpost", "<outpost_id> <attack hour (0-23)> <defense hour(0-23)>")
{
	if (args.size() != 3) return false;
	
	// select the wanted outpost
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 attackHour;
	NLMISC::fromString(args[1], attackHour);
	uint32 defenseHour;
	NLMISC::fromString(args[2], defenseHour);

	if(attackHour > 23)
	{
		log.displayNL("attack hour must be between 0-23");
		return true;
	}

	if(defenseHour > 23)
	{
		log.displayNL("defense hour must be between 0-23");
		return true;
	}

	outpost->timeSetAttackHour(attackHour);
	outpost->timeSetDefenseHour(defenseHour);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetAttackDefenseDate, "Set attack and defense date of an outpost", "<outpost_id> <Nb days to add at attack/defense date>")
{
	if (args.size() != 2) return false;
	
	// select the wanted outpost
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return true;

	uint32 nbDaysAdd;
	NLMISC::fromString(args[1], nbDaysAdd);

	outpost->setRealChallengeTime( outpost->getRealChallengeTime() + nbDaysAdd*days );
	outpost->setChallengeTime( (outpost->getRealChallengeTime()/hours + 1)*hours );
	outpost->setChallengeHour( (outpost->getChallengeTime()%days)/hours );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostSetState, "Set outpost state (Peace/WarDeclaration/AttackBefore/AttackRound/AttackAfter/DefenseBefore/DefenseRound/DefenseAfter)", "<outpost_id> <State>")
{
	if (args.size() != 2) return false;
	
	// select the wanted outpost
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return false;
		
	OUTPOSTENUMS::TOutpostState state = OUTPOSTENUMS::toOutpostState(args[1]);
	if(state == OUTPOSTENUMS::UnknownOutpostState)
	{
		log.displayNL("invalid state : '%s'", args[1].c_str());
		return true;
	}

	outpost->setState(state);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setMemberEntryDate, "Set guild member entry date", "<eid> <entryCycle>")
{
	if (args.size() != 2) return false;
	
	GET_CHARACTER

	uint32 cycleEntryDate;
	NLMISC::fromString(args[1], cycleEntryDate);

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( c->getGuildId() );
	if (guild == NULL)
	{
		log.displayNL("Guild Id '%d' not found for character '%s'", c->getGuildId(), eid.toString().c_str());
		return true;
	}

	if(guild->getMemberFromEId(eid) == NULL)
	{
		log.displayNL("GuildMember not found for guild Id '%d' for character '%s'", c->getGuildId(), eid.toString().c_str());
		return true;
	}

	guild->getMemberFromEId(eid)->setEnterTime(cycleEntryDate);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostUnbanPlayer, "Unban player for an outpost", "<outpost_id> <eid> [<all|atk|def>]")
{
	if( args.size() < 2 || args.size() > 3 ) return false;

	CEntityId eid(args[1]);

	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return false;

	if(args.size() == 2 || args[2] == string("all") )
	{
		outpost->unBanishPlayerForDefense(eid);
		outpost->unBanishPlayerForAttack(eid);
	}
	else if(args[2] == string("atk"))
	{
		outpost->unBanishPlayerForAttack(eid);
	}
	else
	{
		outpost->unBanishPlayerForDefense(eid);
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostUnbanGuild, "Unban guild for an outpost", "<outpost_id> <guild_name> [<all|atk|def>]")
{
	if( args.size() < 2 || args.size() > 3 ) return false;
	
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return false;
	
	CGuild * guild = CGuildManager::getInstance()->getGuildByName(args[1]);
	if (guild == NULL)
	{
		log.displayNL("unknown guild : '%s'", args[1].c_str());
		return true;
	}

	if(args.size() == 2 || args[2] == string("all") )
	{
		outpost->unBanishGuildForDefense(guild->getId());
		outpost->unBanishGuildForAttack(guild->getId());
	}
	else if(args[2] == string("atk"))
	{
		outpost->unBanishGuildForAttack(guild->getId());
	}
	else
	{
		outpost->unBanishGuildForDefense(guild->getId());
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostBanPlayer, "Ban player for an outpost", "<outpost_id> <eid> [<all|atk|def>]")
{
	if( args.size() < 2 || args.size() > 3 ) return false;
	
	CEntityId eid(args[1]);
	
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return false;
	
	if(args.size() == 2 || args[2] == string("all") )
	{
		outpost->banishPlayerForDefense(eid);
		outpost->banishPlayerForAttack(eid);
	}
	else if(args[2] == string("atk"))
	{
		outpost->banishPlayerForAttack(eid);
	}
	else
	{
		outpost->banishPlayerForDefense(eid);
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(outpostBanGuild, "Ban guild for an outpost", "<outpost_id> <guild_name> [<all|atk|def>]")
{
	if( args.size() < 2 || args.size() > 3 ) return false;
	
	CSmartPtr<COutpost> outpost = getOutpostFromString(args[0], log);
	if (outpost == NULL)
		return false;
	
	CGuild * guild = CGuildManager::getInstance()->getGuildByName(args[1]);
	if (guild == NULL)
	{
		log.displayNL("unknown guild : '%s'", args[1].c_str());
		return true;
	}
	
	if(args.size() == 2 || args[2] == string("all") )
	{
		outpost->banishGuildForDefense(guild->getId());
		outpost->banishGuildForAttack(guild->getId());
	}
	else if(args[2] == string("atk"))
	{
		outpost->banishGuildForAttack(guild->getId());
	}
	else
	{
		outpost->banishGuildForDefense(guild->getId());
	}
	return true;
}
