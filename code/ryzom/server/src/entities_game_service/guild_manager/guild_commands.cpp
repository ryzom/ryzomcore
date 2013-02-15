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

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "outpost_manager/outpost_manager.h"

#include "guild.h"
#include "guild_char_proxy.h"
#include "guild_member_module.h"
#include "guild_invitation_module.h"

#include "guild_manager.h"
#include "primitives_parser.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

// local macros
#define GET_CHAR( _id_ ) \
	CCharacter * user = PlayerManager.getChar( _id_ ); \
	if (!user) \
	{ \
		log.displayNL("<GUILD>'%s' is not a valid char. Cant process command",_id_.toString().c_str()); \
		return true; \
	} \
	if (!user->getEnterFlag()) \
	{ \
		log.displayNL("'%s' is not entered", _id_.toString().c_str()); \
		return true; \
	} \
	if (!TheDataset.isAccessible(user->getEntityRowId())) \
	{ \
		log.displayNL("'%s' is not valid in mirror", _id_.toString().c_str()); \
		return true; \
	}\


#define GET_GUILD_MODULE( _id_ ) \
	GET_CHAR( _id_ ) \
	CGuildMember *gmModule = NULL;	\
	if (!user->getModuleParent().getModule(gmModule) || static_cast<CGuild*>(gmModule->getGuild())->isProxy())	\
	{	\
		log.displayNL("<GUILD>'%s' has no valid guild module or guild is not local, cant process command",_id_.toString().c_str()); \
		return true; \
	}	\
	CGuildMemberModule * module = NULL; \
	if ( !user->getModuleParent().getModule( module ) ) \
	{ \
		log.displayNL("<GUILD>'%s' has no valid guild module Cant process command",_id_.toString().c_str()); \
		return true; \
	} \

	
#define GET_INVITATION_MODULE( _id_ ) \
	GET_CHAR( _id_ ) \
	CGuildMember *gmModule = NULL;	\
	if (!user->getModuleParent().getModule(gmModule) || static_cast<CGuild*>(gmModule->getGuild())->isProxy())	\
	{	\
		log.displayNL("<GUILD>'%s' has no valid guild module or guild is not local, cant process command",_id_.toString().c_str()); \
		return true; \
	}	\
	CGuildInvitationModule * module = NULL; \
	if ( !user->getModuleParent().getModule( module ) ) \
	{ \
		log.displayNL("<GUILD>'%s' has no valid guild invitation module Cant process command",_id_.toString().c_str()); \
		return true; \
	} \


#define GET_GUILD(onlyLocal) \
	CGuild * guild = CGuildManager::getInstance()->getGuildByName( args[0] );\
	if ( guild == NULL )\
	{\
		/* try to find the guild with an id*/ \
		uint32 shardId =0; \
		uint32 guildId =0; \
		sscanf(args[0].c_str(), "%u:%u", &shardId, &guildId); \
		guild = CGuildManager::getInstance()->getGuildFromId((shardId<<20)+guildId); \
		\
		if (guild == NULL) \
		{ \
			log.displayNL("Invalid guild '%s'",args[0].c_str());\
			return true;\
		} \
	} \
	if (onlyLocal && guild->isProxy())\
	{\
		log.displayNL("The guild '%s' is a foreign guild, operation forbidden", guild->getName().toString().c_str());\
		return true;\
	} \


//#define GET_GUILD_ARG(argIndex) \
	//CGuild * guild##argIndex = CGuildManager::getInstance()->getGuildByName( args[argIndex] );\
	//if ( guild##argIndex == NULL )\
	//{\
	//	log.displayNL("Invalid guild '%s'",args[argIndex].c_str());\
	//	return true;\
	//}

//#define GET_GUILD_OR_0 \
	//CGuild * guild = NULL;\
	//if ( args[0] != "0" )\
	//{\
	//	guild = CGuildManager::getInstance()->getGuildByName( args[0] );\
	//	if ( guild == NULL )\
	//	{\
	//		log.displayNL("Invalid guild '%s'",args[0].c_str());\
	//		return true;\
	//	}\
	//}\


//----------------------------------------------------------------------------
// CSR/DEBUG COMMANDS
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpGuildList, "dump guild list", "[local]")
{
	if ( args.size() > 1 )
		return false;

	bool onlyLocal = false;
	if (args.size() == 1)
		onlyLocal = args[0] == "local";


	CGuildManager::getInstance()->dumpGuilds(onlyLocal, log );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpGuild, "dump a guild", "<guildName|<shardId>:<guildId>")
{
	if (args.size() != 1)
		return false;

	GET_GUILD(false);

	guild->dumpGuildInfos( log );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildAddMember, "add a new member to a guild", "<guildName|<shardId>:<guildId> <member eid>")
{
	if (args.size() != 2)
		return false;

	GET_GUILD(true);

	CEntityId eId;
	eId.fromString( args[1].c_str() );
	GET_CHAR( eId );

	CGuildCharProxy proxy(user);

	CGuildMemberModule * module;
	if ( proxy.getModule(module) )
	{
		log.displayNL("%s already is a member of a guild", args[1].c_str() );
		return true;
	}

	// message for all guild members about the new member
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
		guild->sendMessageToGuildMembers("GUILD_JOIN", params);
	}

	proxy.setGuildId( guild->getId() );

	// message for the new member
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::string_id);
		params[0].StringId = guild->getNameId();
		proxy.sendSystemMessage("GUILD_YOU_JOIN", params);
	}

	CGuildMember * member = guild->newMember( proxy.getId() );
	BOMB_IF( !member, "created null guild member!", return true );

	// ask the client to open it's guild interface
	PlayerManager.sendImpulseToClient( proxy.getId(),"GUILD:OPEN_GUILD_WINDOW" );

//	module = new CGuildMemberModule( proxy, member );
//	BOMB_IF( !module, "created null guild member module!", return true );

//	member->setMemberGrade( EGSPD::CGuildGrade::Member );
//	guild->setMemberOnline( member, proxy.getId().getDynamicId() );

	log.displayNL("%s now is a member of guild '%s'", args[1].c_str(), args[0].c_str() );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildSetGrade, "set the grade of a guild member", "<guildName|<shardId>:<guildId> <member eid> <grade = Member/Officer/HighOfficer/Leader>")
{
	if (args.size() != 3)
		return false;

	GET_GUILD(true);


	CEntityId eId;
	eId.fromString( args[1].c_str() );
	EGSPD::CGuildGrade::TGuildGrade grade = EGSPD::CGuildGrade::fromString( args[2] );

	CGuildMember * member = guild->getMemberFromEId( eId );
	if ( !member )
	{
		log.displayNL("%s is not a member of guild '%s'", args[1].c_str(), args[0].c_str() );
		return true;
	}

	guild->setMemberGrade( member, grade, &log );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildSetLeader, "set the leader of a guild", "<guildName|<shardId>:<guildId> <member eid>")
{
	if (args.size() != 2)
		return false;

	GET_GUILD(true);

	CEntityId eId;
	eId.fromString( args[1].c_str() );

	CGuildMember * member = guild->getMemberFromEId( eId );
	if ( !member )
	{
		log.displayNL("%s is not a member of guild '%s'", args[1].c_str(), args[0].c_str() );
		return true;
	}

	if (member->getGrade() == EGSPD::CGuildGrade::Leader)
	{
		log.displayNL("%s already is leader of guild '%s'", args[1].c_str(), args[0].c_str() );
		return true;
	}

	CGuildMember * leader = guild->getLeader();
	if (leader)
	{
		guild->setMemberGrade( leader, EGSPD::CGuildGrade::Member, &log );
	}

	guild->setMemberGrade( member, EGSPD::CGuildGrade::Leader, &log );

	return true;
}


//----------------------------------------------------------------------------
// TEST COMMANDS
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildMOTD,"Set the guild message of the day","<userId><message of the day>")
{
	if ( args.size() != 2 && args.size() != 1 )
		return false;

	CEntityId eId;
	eId.fromString(args[0].c_str());
	
	GET_CHAR(eId);
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
	if( guild )
	{
		string motd;
		if(args.size() == 2)
			motd = args[1];
		guild->setMOTD(motd, eId);
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildCreate,"create a new guild","<userId><name><description><icon><guild creator bot>")
{
	if ( args.size() != 5 )
		return false;
	CEntityId eId;
	eId.fromString(args[0].c_str());
	ucstring name = args[1];
	ucstring description = args[2];
	uint64 icon = NLMISC::atoiInt64( args[3].c_str() );
	
	// get the character and build a proxy from it
	GET_CHAR(eId);
	CGuildCharProxy proxy(user);


	CEntityId botId;
	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getNPCAliasesFromName( args[4], aliases );
	if ( !aliases.empty() )
		botId = aliases[0];
	user->setCurrentInterlocutor( botId );
	CGuildManager::getInstance()->createGuild(proxy,name,icon,description);
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildJoinInvitation,"guild Join Invitation","<user><target>")
{
	if ( args.size() != 2 )
		return false;
	CEntityId eId1;
	eId1.fromString(args[0].c_str());
	CEntityId eId2;
	eId2.fromString(args[1].c_str());
	CCharacter * firstChar = PlayerManager.getChar( eId1 );
	if ( !firstChar )
	{ 
		log.displayNL("<GUILD>'%s' is not a valid char. Cant process command",eId1.toString().c_str());
		return true; 
	}
	firstChar->setTarget( eId2 );
	GET_GUILD_MODULE(eId1);
	module->inviteTargetInGuild();
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildAcceptJoinInvitation,"Accept guild Join Invitation","<user>")
{
	if ( args.size() != 1 )
		return false;
	CEntityId eId;
	eId.fromString(args[0].c_str());
	GET_INVITATION_MODULE(eId);
	module->accept();
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildRefuseJoinInvitation,"Refuse guild Join Invitation","<user>")
{
	if ( args.size() != 1 )
		return false;
	CEntityId eId;
	eId.fromString(args[0].c_str());
	GET_INVITATION_MODULE(eId);
	module->refuse();
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( guildDB, "Display or set the value of a property in the guild database", "<guildName|<shardId>:<guildId> <db_entry> [<value>]" )
{
	if ( args.size() != 2 && args.size() != 3 )
		return false;
	GET_GUILD(true);
	try
	{
		if ( args.size() == 3 )
		{
			sint64 val = NLMISC::atoiInt64( args[2].c_str() );
//			guild->setClientDBProp( args[1],val );
			guild->_DbGroup.Database.x_setProp(args[1], val);
		}
//		sint64 val = guild->getClientDBProp( args[1] );
		sint64 val = guild->_DbGroup.Database.x_getProp(args[1]);
		log.displayNL( "property value is '%"NL_I64"d'", val );
	}
	catch (const CCDBSynchronised::ECDBNotFound &e)
	{
		log.displayNL( "exception %s", e.what() );
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(testGuildSetLeader,"set the leader of a guild","<userId> <index> <clientSession>")
{
	if ( args.size() != 3 )
		return true;
	CEntityId eId;
	eId.fromString(args[0].c_str());
	uint16 index;
	NLMISC::fromString(args[1], index);
	uint8 session;
	NLMISC::fromString(args[2], session);
	GET_GUILD_MODULE(eId);
	module->setLeader( index,session );
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(testGuildSetGrade,"set the grade of a member","<userId> <index> <clientSession> <grade>")
{
	if ( args.size() != 4 )
		return false;
	CEntityId eId;
	eId.fromString(args[0].c_str());
	uint16 index;
	NLMISC::fromString(args[1], index);
	uint8 session;
	NLMISC::fromString(args[2], session);
	EGSPD::CGuildGrade::TGuildGrade grade = EGSPD::CGuildGrade::fromString( args[4] );
	GET_GUILD_MODULE(eId);
	module->setGrade(index,session, (EGSPD::CGuildGrade::TGuildGrade) grade);
	log.displayNL("Command Executed");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( dumpGuildInventory, "Display the content of a guild inventory (DEBUG)", "<guildName|<shardId>:<guildId>" )
{
	if ( args.empty() )
		return false;
	GET_GUILD(false);

	guild->getInventory()->dumpInventory( log );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( importGuildFile, "Import a guild file into the server", "<filename> [<guildNumber>|highest]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;

	log.displayNL("Importing guild file '%s'...", args[0].c_str());
	log.displayNL("Current directory is '%s'", CPath::getCurrentPath().c_str());

	if (!CFile::isExists(args[0]))
	{
		log.displayNL("Can't find the file '%s', retry with a relative or full path or with the correct name !", args[0].c_str());
		return true;
	}

	uint32 guildId = 0;

	// check the second arg if any for forced guild id
	if (args.size() == 2)
	{
		if (args[1] == "highest")
			guildId = CGuildManager::getInstance()->getFreeGuildId();
		else
		{
			NLMISC::fromString(args[1], guildId);
			if (guildId == 0)
			{
				log.displayNL("Invalid specific guildId, must be a number or 'highest', found '%s'", args[1].c_str());
				return true;
			}
		}
	}

	// ok, the file is available.
	const uint len = (uint)strlen("guild_XXXXX.ext");
	string file = CFile::getFilename(args[0]);
	CGuild *guild = NULL;

	static CPersistentDataRecord	pdr;
	pdr.clear();
	
	// check if we have a guild file
	if ( file.size() == len &&
		file.find( "guild_" ) == 0 &&
		file.find( "bin" ) == len - 3 )
	{
		// load a binary file
		string idStr = file.substr(6,5);
		uint32 id;
		NLMISC::fromString(idStr, id);

		if (guildId != 0)
			id = guildId;
		else
			guildId = id;

		if ( id > 0)
		{
			// this is a guild file. We can load it
			pdr.readFromBinFile(args[0].c_str());

			guildId = id;
		}
		else
		{
			log.displayNL("Invalid guild ID, can't import the file  you must provide a guilde id directly");
			return true;
		}
	}
	else
	{
		// check if we have a guild file
		if ( file.size() == len &&
			file.find( "guild_" ) == 0 &&
			(file.find( "txt" ) == (len - 3) || file.find( "xml" ) == (len - 3))  )
		{
			// Load a text file
			string idStr = file.substr(6,5);
			uint32 id;
			NLMISC::fromString(idStr, id);

			if (guildId != 0)
				id = guildId;
			else
				guildId = id;

			if ( id > 0)
			{
				// this is a guild file. We can load it
				pdr.readFromTxtFile(args[0].c_str());
				guildId = id;
			}
			else
			{
				log.displayNL("Invalid guild ID, can't import the file  you must provide a guilde id directly");
				return true;
			}
		}
	}

	// ok, we have a new guild loaded in the pdr

	// check that the id is free
	if (CGuildManager::getInstance()->getGuildFromId(guildId) != NULL)
	{
		log.displayNL("ERROR : The guild number %u is already existing, can't overwrite it, you must provide guild id", guildId);
		return true;
	}

	guild = EGS_PD_CAST<CGuild*>( EGSPD::CGuildPD::create(guildId) );
	EGS_PD_AST(guild);
	guild->initNonPDMembers();
	guild->apply( pdr );

	// add the guild to the container
	CGuildManager::getInstance()->_Container->setGuilds( guild );

	CGuildManager::getInstance()->checkMemberConsistency(guild);
	CGuildManager::getInstance()->registerGuildAfterLoading(guild);


	// compute the highest valid guild id ( 0 if none )
//	CGuildManager::getInstance()->_HighestGuildId = max(CGuildManager::getInstance()->_HighestGuildId, guildId);

	// for a member list update
	guild->updateMembersStringIds();

	log.displayNL("Guild file '%s' effectively imported as guild number %u", args[0].c_str(), guildId);


	return true;
}


#undef GET_CHAR
#undef GET_GUILD_MODULE
#undef GET_INVITATION_MODULE
#undef GET_GUILD
//#undef GET_GUILD_OR_0
//#undef GET_GUILD_ARG
