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
// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "game_share/string_manager_sender.h"
#include "game_share/brick_families.h"
#include "game_share/ecosystem.h"
#include "game_share/people.h"
#include "game_share/roles.h"
//#include "game_share/jobs.h"
#include "game_share/skills.h"
#include "game_share/scores.h"
#include "game_share/characteristics.h"
#include "game_share/damage_types.h"
#include "game_share/power_types.h"
#include "input_output_service.h"
#include "string_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace STRING_MANAGER;


NLMISC_COMMAND(smReload, "reload all translation files in string manager", "")
{
	SM->reload(&log);

	log.displayNL("reload done.");

	return true;
}

NLMISC_COMMAND(smReloadEventFactions, "reload event factions", "[<filename>]")
{
	if (args.size() > 1)
		return false;

	if (args.size() == 1)
		SM->reloadEventFactions(&log, args[0]);
	else
		SM->reloadEventFactions(&log);

	log.displayNL("reload done.");

	return true;
}

NLMISC_COMMAND(smClearCache, "clear the string cache in string manager", "")
{
	SM->clearCache(&log);
	return true;
}

NLMISC_COMMAND(smString, "display a string from the string manager <string_id>", "")
{
	if (args.size() != 1)
	{
		return false;
	}

	uint32 stringId;
	NLMISC::fromString(args[0], stringId);

	const ucstring &str = SM->getString(stringId);

	log.displayNL("String id %u = [%s]", stringId, str.toString().c_str());

	return true;
}

// sm_test TEST_ITEM fyros_2h_axe_lvl_01_05.item
NLMISC_COMMAND(smTest, "Send a test dyn string to a client (look at first phrase in phrase_en.txt)", "<client_name> <PHRASE NAME> [<param>]")
{
	if (args.size() < 2)
		return false;

	CCharacterInfos *ci = IOS->getCharInfos(args[0]);
	if (ci == NULL)
	{
		// try to find a valid client
		
		log.displayNL("Unknown client '%s' !", args[0].c_str());
		return false;
	}

	uint dynId;
	TVectorParamCheck	params;
	TParam					p;

	if (args[1] == "TEST_SELF")
	{
		if (args.size() != 2)
			return false;
		
		dynId = sendStringToClient(ci->DataSetIndex, "TEST_SELF", params, &IosLocalSender);
	}
	else
	{
		if (args.size() != 3)
			return false;

		if (args[1] == "TEST_ITEM")
		{
			// try to retreive the sheet id
			NLMISC::CSheetId sid(args[2]);
			if (sid != NLMISC::CSheetId::Unknown)
			{
				p.Type = STRING_MANAGER::item;
				p.SheetId = sid;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_ITEM", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown item %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_PLAYER")
		{
			// try to retreive the player infos
			CCharacterInfos *other = IOS->getCharInfos(args[2]);
			if (other)
			{
				p.Type = STRING_MANAGER::player;
				p.setEId( other->EntityId );
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_PLAYER", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown player name '%s'", args[2].c_str());
			}
		}
		else if (args[1] == "TEST_ENTITY")
		{
			// try to retreive the player infos
//			CCharacterInfos *other = IOS->getCharInfos(args[2]);
			CEntityId	eid(args[2]);
			if (eid != CEntityId::Unknown)
			{
				p.Type = STRING_MANAGER::entity;
				p.setEId( eid );

				// for some obscure reason, GCC 3.2.3 fail internaly to compile the push_back !
				// so, I replaced the push_back with a resize + affectation.
				params.resize(1);
				params[0] = p;
//				params.push_back(p);

				dynId = sendStringToClient(ci->DataSetIndex, "TEST_ENTITY", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Can't make entity ID from '%s'", args[2].c_str());
			}
		}
		else if (args[1] == "TEST_SBRICK")
		{
			NLMISC::CSheetId sid(args[2]);
//			BRICK_FAMILIES::TBrickFamily brick = BRICK_FAMILIES::toSBrickFamily(args[2]);
//			if (brick != BRICK_FAMILIES::Unknown)
			if (sid != CSheetId::Unknown )
			{
				p.Type = STRING_MANAGER::sbrick;
				p.SheetId = sid;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_SBRICK", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown sbrick '%s'", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_ECO")
		{
			ECOSYSTEM::EECosystem eco = ECOSYSTEM::stringToEcosystem(args[2]);
			if (eco != ECOSYSTEM::unknown)
			{
				p.Type = STRING_MANAGER::ecosystem;
				p.Enum = eco;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_ECO", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown ecosystem %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_CREATURE_MODEL")
		{
			std::string sheetName = args[2];
			CSheetId sid(sheetName);
//			EGSPD::CPeople::TPeople people = EGSPD::CPeople::fromString(args[1]);
			if (sid != CSheetId::Unknown)
			{
				p.Type = STRING_MANAGER::creature_model;
				p.SheetId = sid;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_CREATURE_MODEL", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown creature model '%s'", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_RACE")
		{
			//RACES::TRace race = RACES::toRaceId(args[1]);
			EGSPD::CPeople::TPeople people = EGSPD::CPeople::fromString(args[2]);
			if (people != EGSPD::CPeople::EndPeople)
			{
				p.Type = STRING_MANAGER::race;
				p.Enum = people;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_RACE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown race %s", args[2].c_str());
				return true;
			}
		}
/*		else if (args[1] == "TEST_CAREER")
		{
			ROLES::ERole career = ROLES::toRoleId(args[2]);
			if (career != ROLES::role_unknown)
			{
				p.Type = STRING_MANAGER::career;
				p.Enum = career;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_CAREER", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown career %s", args[2].c_str());
				return true;
			}
		}
*/
/*		else if (args[1] == "TEST_JOB")
		{
			JOBS::TJob job = JOBS::toJob(args[2]);
			if (job != JOBS::Unknown)
			{
				p.Type = STRING_MANAGER::job;
				p.Enum = job;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_JOB", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown job %s", args[2].c_str());
				return true;
			}
		}
*/
		else if (args[1] == "TEST_SKILL")
		{
			SKILLS::ESkills skill = SKILLS::toSkill(args[2]);
			if (skill != SKILLS::unknown)
			{
				p.Type = STRING_MANAGER::skill;
				p.Enum = skill;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_SKILL", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown skill %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_BODYPART")
		{
			BODY::TBodyPart bodyPart = BODY::toBodyPart(args[2]);
			if (bodyPart != BODY::UnknownBodyPart)
			{
				p.Type = STRING_MANAGER::body_part;
				p.Enum = bodyPart;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_BODYPART", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown body part %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_SCORE")
		{
			SCORES::TScores score = SCORES::toScore(args[2]);
			if (score != SCORES::unknown)
			{
				p.Type = STRING_MANAGER::score;
				p.Enum = score;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_SCORE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown score %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_CHARAC")
		{
			CHARACTERISTICS::TCharacteristics charac = CHARACTERISTICS::toCharacteristic(args[2]);
			if (charac != CHARACTERISTICS::Unknown)
			{
				p.Type = STRING_MANAGER::characteristic;
				p.Enum = charac;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_CHARAC", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown characteristic %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_DAMAGE_TYPE")
		{
			DMGTYPE::EDamageType damageType = DMGTYPE::stringToDamageType(args[2]);
			if (damageType != DMGTYPE::UNDEFINED)
			{
				p.Type = STRING_MANAGER::damage_type;
				p.Enum = damageType;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_DAMAGE_TYPE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown damage type %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_SPHRASE")
		{
			NLMISC::CSheetId sid(args[2]);
			if (sid != NLMISC::CSheetId::Unknown)
			{
				p.Type = STRING_MANAGER::sphrase;
				p.SheetId = sid;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_SPHRASE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown sphrase Id %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_DYN_STRING")
		{
			uint32 id;
			NLMISC::fromString(args[2], id);
			p.Type = STRING_MANAGER::dyn_string_id;
			p.StringId = id;
			params.push_back(p);
			dynId = sendStringToClient(ci->DataSetIndex, "TEST_DYN_STRING", params, &IosLocalSender);
		}
		else if (args[1] == "TEST_STRING")
		{
			uint32 id;
			NLMISC::fromString(args[2], id);
			p.Type = STRING_MANAGER::string_id;
			p.StringId = id;
			params.push_back(p);
			dynId = sendStringToClient(ci->DataSetIndex, "TEST_STRING", params, &IosLocalSender);
		}
		else if (args[1] == "TEST_BOT_NAME")
		{
			if (args.size() < 3)
			{
				log.displayNL("Mission bot name as parameter !");
				return true;
			}
			else
			{
				p.Type = STRING_MANAGER::bot_name;
				p.Identifier = args[2];
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_BOT_NAME", params, &IosLocalSender);
			}
		}
		else if (args[1] == "TEST_POWER_TYPE")
		{
			POWERS::TPowerType powerType = POWERS::toPowerType(args[2]);
			if (powerType != POWERS::UnknownType)
			{
				p.Type = STRING_MANAGER::power_type;
				p.Enum = powerType;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_POWER_TYPE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown power type %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_LITERAL")
		{
			ucstring str = args[2];
			p.Type = STRING_MANAGER::literal;
			p.Literal = str;
			params.push_back(p);
			dynId = sendStringToClient(ci->DataSetIndex, "TEST_LITERAL", params, &IosLocalSender);
		}
		else if (args[1] == "TEST_TITLE")
		{
			ucstring str = args[2];
			p.Type = STRING_MANAGER::title;
			p.Identifier = str.toString();
			params.push_back(p);
			dynId = sendStringToClient(ci->DataSetIndex, "TEST_TITLE", params, &IosLocalSender);
		}
		else if (args[1] == "TEST_EVENT_FACTION")
		{
			ucstring str = args[2];
			p.Type = STRING_MANAGER::event_faction;
			p.Identifier = str.toString();
			params.push_back(p);
			dynId = sendStringToClient(ci->DataSetIndex, "TEST_EVENT_FACTION", params, &IosLocalSender);
		}
		else if (args[1] == "TEST_CLASSIFICATION_TYPE")
		{
			EGSPD::CClassificationType::TClassificationType type = EGSPD::CClassificationType::fromString(args[2]);
			if (type != EGSPD::CClassificationType::Unknown)
			{
				p.Type = STRING_MANAGER::classification_type;
				p.Enum = type;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_CLASSIFICATION_TYPE", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown classification type %s", args[2].c_str());
				return true;
			}
		}
		else if (args[1] == "TEST_OUTPOST_WORD")
		{
			NLMISC::CSheetId sid(args[2]);
			if (sid != CSheetId::Unknown )
			{
				p.Type = STRING_MANAGER::outpost;
				p.SheetId = sid;
				params.push_back(p);
				dynId = sendStringToClient(ci->DataSetIndex, "TEST_OUTPOST_WORD", params, &IosLocalSender);
			}
			else
			{
				log.displayNL("Unknown sbrick '%s'", args[2].c_str());
				return true;
			}
		}
		else
		{
			log.displayNL("Unknow parameter %s", args[2].c_str());
			return false;
		}
	}

	log.displayNL("Dyn string send to client %s with ID = %u", args[0].c_str(), dynId);
	
	return true;
}


NLMISC_COMMAND(sysCmd, "Run preprogrammed system command(s)","commands are in the SystemCmd config variable")
{
	CConfigFile::CVar *sysCmds = IService::getInstance()->ConfigFile.getVarPtr("SystemCmd");
	if (sysCmds != NULL)
	{
		for (uint i=0; i<sysCmds->size(); ++i)
		{
			string cmd = sysCmds->asString(i);

			log.displayNL("Invoking system command '%s'...", cmd.c_str());
			int ret = system(cmd.c_str());
			log.displayNL(" command returned %d", ret);
		}
	}
	else
	{
		log.displayNL("Variable SystemCmd not found !");
	}

	return true;
}


//-----------------------------------------------
//	'mute'
//
//-----------------------------------------------
NLMISC_COMMAND(mute,"Mute or unmute a player. the player can be muted for a fixed period of time","<player>[<delay>]")
{
	if( args.size() == 0 )
	{
		return false;
	}
	sint32 delay = -1;
	if( args.size() > 1 )
	{
		NLMISC::fromString(args[1], delay);
	}

	CCharacterInfos * charInfos = IOS->getCharInfos( args[0] );
	if( charInfos != NULL )
	{
		try
		{
			IOS->getChatManager().getClient(charInfos->DataSetIndex).mute( delay );
		}
		catch(const Exception &e)
		{
			nlwarning("<mite> %s",e.what());
		}
	}
	else
	{
		nlwarning("(mute) No infos about the character %s",args[0].c_str());
	}
		
	return true;

} // mode //



//-----------------------------------------------
//	'remove'
//
//-----------------------------------------------
NLMISC_COMMAND(remove,"remove an entity from","<entity name>")
{
	if( args.size() < 1 )
	{
		return false;
	}

	CCharacterInfos * charInfos = IOS->getCharInfos( args[0] );
	if( charInfos )
	{
		IOS->removeEntity( charInfos->DataSetIndex);
	}
	else
	{
		nlwarning("<remove> Entity %s is not in the IOS",args[0].c_str());
		return false;
	}
	return true;
}


/*
NLMISC_COMMAND(genImpulsion,"generate a fake impulsion, used to debug the CActionGeneric on the FS","<nb_uint8>")
{
	if( args.size() != 1 )
	{
		return false;
	}

	CEntityId id;
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( id );
	
	CBitMemStream stream;

	uint count;
	NLMISC::fromString(args[0], count);
	uint8 val = 0xAC;
	for (uint i = 0; i < count; i++)
		stream.serial (val);

	//vector<uint8> &v = stream.bufferAsVector();
	msgout.serialBufferWithSize ((uint8*)stream.buffer(), stream.length());

	//const uint16 frontEndId = _Id.DynamicId;
	sendMessageViaMirror ("FS", msgout);

	return true;
}
*/


NLMISC_COMMAND(display,"display","")
{
	IOS->display(log);
	return true;
}

NLMISC_COMMAND( displayChatClients, "Display the list of clients", "" )
{
	IOS->getChatManager().displayChatClients(log);
	return true;
}

NLMISC_COMMAND( displayChatGroups, "Display the list of chat groups, optionally, display universe chat group and/or player audience groups", "[universe] [player]" )
{
	bool displayUniverse = false;
	bool playerAudience = false;
	vector<string> params = args;
	while (!params.empty())
	{
		if (params[0] == "universe")
			displayUniverse = true;
		else if (params[0] == "player")
			playerAudience = true;
		else
		{
			log.displayNL("syntax error: invalid parameter '%s'", params[0].c_str());
			return false;
		}
		params.erase(params.begin());
	}

	IOS->getChatManager().displayChatGroups(log, displayUniverse, playerAudience);
	return true;
}

NLMISC_COMMAND( displayChatAudience, "Display the current chat dynamic audience for a player, optionally, force the update", "<playerId> [update]" )
{
	bool update = false;
	if (args.size() < 1)
		return false;

	CEntityId eid = CEntityId(args[0]);

	if (args.size() >= 2 && args[1] == "update")
		update = true;
	IOS->getChatManager().displayChatAudience(log, eid, update);
	return true;
}


//-----------------------------------------------
//	'showChat'
//
//-----------------------------------------------
NLMISC_COMMAND(showChat,"show or hide chat messages","")
{
	ShowChat = !ShowChat;
	
	return true;

} // showChat //
