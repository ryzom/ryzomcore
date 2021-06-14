/** \file commands.cpp
 * File with all the commands for the console.
 *
 * $Id: commands.cpp,v 1.8 2004/06/23 15:03:04 lecroart Exp $
 */




//////////////
// Includes //
//////////////
#include "stdpch.h"
// Game Share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/visual_slot_manager.h"
// Misc.
#include "nel/misc/command.h"
#include "nel/misc/i18n.h"
#include "nel/misc/bit_mem_stream.h"
// 3D Interface.
#include "nel/3d/u_scene.h"
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_play_list_manager.h"
// Client.
#include "interface_manager.h"
#include "commands.h"
#include "user_entity.h"
#include "entity_animation_manager.h"
#include "firework.h"
#include "view.h"
#include "time_client.h"
#include "entities.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "continent.h"
#include "ig_client.h"
#include "ingame_database_manager.h"
#include "client_chat_manager.h"
#include "net_manager.h"
#include "pacs_client.h"
#include "teleport.h"
#include "misc.h"
#include "sheet_manager.h"
#include "demo.h"
#include "sound_manager.h"
#include "cdb_leaf.h"
#include "debug_client.h"
#include "input_handler_manager.h"
// game share
#include "game_share/brick_types.h"
#include "game_share/player_visual_properties.h"
#include "game_share/ryzom_version.h"
#include "game_share/mode_and_behaviour.h"
#include "brick_manager.h"


////////////
// DEFINE //
////////////
//#define	_LIMITED_COMMAND_	// Disable some commands
#define _CMD_CHEAT_ON_	// Activate cheat commands
#define _CMD_TEST_ON_	// Activate test commands
#define _CMD_DEBUG_ON_	// Activate debug commands


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;
using namespace std;


/////////////
// Externs //
/////////////
extern CUserEntity					UserEntity;
extern UScene						* Scene;
extern ULandscape					* Landscape;
extern CEntityAnimationManager		* EAM;
extern CFirework					* Firework;
extern CClientChatManager			ChatMngr;
extern CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;
extern ::CNetManagerMulti			NetMngr;
extern CInterfaceManager			*IM;


//////////////
// COMMANDS //
//////////////
////////////////////
// FINAL COMMANDS //
//-----------------------------------------------
// 'emote' : play an emote.
//-----------------------------------------------
NLMISC_COMMAND(emote, "play an emote.", "<number>")
{
	const uint nbEmotes = EAM->getNbEmots();

	// Check parameters.
	if(args.size() == 1)
	{
		// Check the number is valid.
		uint num = atoi(args[0].c_str());
		if(num < nbEmotes)
		{
			// Create the message and send.
			const string msgName = "COMMAND:EMOTE";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				MBEHAV::EBehaviour behaviour = (MBEHAV::EBehaviour)(MBEHAV::EMOTE_BEGIN + num);
				out.serialEnum(behaviour);
				NetMngr.push(out);
			}
			else
				nlwarning("command 'emote': unknown message named '%s'.", msgName.c_str());

			// Command Well done.
			return true;
		}
	}

	CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 1 paramter and this must be one of the following number :"));
	CAnimationState::TAnimStateId result;
	for(uint i = 0; i<nbEmotes; ++i)
	{
		EAM->getEmot(i, result);
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("%d - %s", i, CAnimationState::getAnimationStateName(result).c_str ())));
	}

	// Command Well done.
	return true;
}// chat_mode //

//-----------------------------------------------
// 'guildSymbol' : Show/Hide the Guild Symbol.
//-----------------------------------------------
NLMISC_COMMAND(guildSymbol, "Show/Hide the Guild Symbol.", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	// Invert the state.
	GuildSymbol.Active = !GuildSymbol.Active;

	// Command Well done.
	return true;
}// chat_mode //

//-----------------------------------------------
// 'chatMode' : To specify how you will speak and who will be able to hear you.
//-----------------------------------------------
NLMISC_COMMAND(chatMode, "To specify how will you speak and who will be able to hear you.", "<mode : [say|shout|universe|team|clade]>")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	CInputHandlerManager *pIHM = CInputHandlerManager::getInstance();
	
	uint8 mode ;
	if     (args[0] == "say")
		mode = 0;
	else if(args[0] == "shout")
		mode = 1;
	else if(args[0] == "universe")
		mode = 2;
	else if(args[0] == "team")
		mode = 3;
	else if(args[0] == "clade")
		mode = 4;
	else
	{
		nlwarning("Command 'chatMode': unknown chat mode '%s'.", args[0].c_str());
		return false;
	}
	// Display debug info.
	nlinfo("Command 'chatMode': '%s' mode.", args[0].c_str());
	// Set the chat mode.
	ChatMngr.setChatMode(mode);
	// Command Well done.
	return true;
}// chat_mode //

//-----------------------------------------------
// 'tell' : send a private message to a unique player.
//-----------------------------------------------
NLMISC_COMMAND(tell, "send a private message to a unique player", "<char name><message>")
{
	// Check parameters.
	if(args.size() < 2)
		return false;

	// Compute the message.
	string message;
	for(uint i=1; i<args.size(); i++)
		message += args[i] + " ";
	// Send the message.
	string receiver = args[0];
	ChatMngr.tell(receiver, message);
	// Command Well done.
	return true;
}// tell //

//-----------------------------------------------
// 'bugReport' : to report a bug.
//-----------------------------------------------
NLMISC_COMMAND(bugReport, "call the bug report tool with dump.", "<AddScreenshot>")
{	
	const char *brname[] = { "bug_report.exe", "bug_report_r.exe", "bug_report_rd.exe", "bug_report_df.exe", "bug_report_d.exe" };

	string brn;

	for (uint i = 0; i < sizeof(brname)/sizeof(brname[0]); i++)
	{
		if (CFile::fileExists (brname[i]))
		{
			brn = brname[i];
			break;
		}
	}
	
	if (brn.empty())
	{
		log.displayNL("bug_report*.exe not found");
		return false;
	}

	string sys;

	sys = "Language "+CI18N::getCurrentLanguageName().toString() +" ";

	if (args.size()>0)
	{
		uint8 quality = atoi (args[0].c_str());
		if (quality == 0)
			quality = 80;

		CBitmap btm;
		Driver->getBuffer(btm);
		string filename = CFile::findNewFile ("screenshot.jpg");
		COFile fs(filename);
		btm.writeJPG(fs, quality);
		sys += "AttachedFile "+filename+" ";
	}

	sys += "ClientVersion "RYZOM_VERSION" ";
	
	// for now, set the same version than client one
	sys += "ShardVersion "RYZOM_VERSION" ";

	if (ClientCfg.Local)
		sys += "ShardName OFFLINE ";

	FILE *fp = fopen ("bug_report.txt", "wb");
	if (fp != NULL)
	{
		// must put \r\n each line
		fprintf (fp, "UserPosition: %.2f %.2f %.2f\r\n", View.viewPos().x, View.viewPos().y, View.viewPos().z);
		fprintf (fp, "ServerTick: %u\r\n", NetMngr.getCurrentServerTick() );
		fprintf (fp, "LocalAddress: %s\r\n", NetMngr.getAddress().asString().c_str() );
// TODO add debug information for the bug report

		fclose (fp);
		sys += "DumpFilename bug_report.txt ";
	}

	nlinfo ("Calling for bug report : '%s %s'", brn.c_str(), sys.c_str());
	
	launchProgram(brn, sys);
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////// COMMANDS after should NOT appear IN the FINAL VERSION ///////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#if !FINAL_VERSION

NLMISC_COMMAND(missionReward, "debug"," ")
{
	if (args.size() == 1)
	{
		uint8 index = atoi( args[1].c_str() );
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("BOTCHAT:COMPLETE_MISSION", out))
		{
			out.serial(index);
			NetMngr.push(out);
		}
		else
			nlwarning("<CBotChat acceptMission> : unknown message name : BOTCHAT:COMPLETE_MISSION");
		return true;
	}
	return false;
}

NLMISC_COMMAND(missionProgress, "debug"," ")
{
	if (args.size() == 1)
	{
		uint8 index = atoi( args[1].c_str() );
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("BOTCHAT:PROGRESS_MISSION", out))
		{
			out.serial(index);
			NetMngr.push(out);
		}
		else
			nlwarning("<CBotChat acceptMission> : unknown message name : BOTCHAT:PROGRESS_MISSION");
		return true;
	}
	return false;
}

/*
NLMISC_COMMAND( displayDBModifs, "display server database modification in the chat window"," ")
{
	if ( VerboseDatabase )
		CInterfaceManager::getInstance()->getChatOutput()->addTextChild(ucstring("the database is already in verbose mode"),CRGBA(255,255,255,255));
	else
	{
		CInterfaceManager::getInstance()->getChatOutput()->addTextChild(ucstring("database  is now in verbose mode"),CRGBA(255,255,255,255));
		VerboseDatabase = true;
	}
	return true;
}

NLMISC_COMMAND( hideDBModifs, "stop displaying server database modification in the chat window"," ")
{
	if ( !VerboseDatabase )
		CInterfaceManager::getInstance()->getChatOutput()->addTextChild(ucstring("the database is already not in verbose mode"),CRGBA(255,255,255,255));
	else
	{
		CInterfaceManager::getInstance()->getChatOutput()->addTextChild(ucstring("database is not in verbose mode anymore"),CRGBA(255,255,255,255));
		VerboseDatabase = false;
	}
	return true;
}
*/

NLMISC_COMMAND(save_sentences, "save sentences"," ")
{
//	CSentenceDisplayer::saveSentences();
	return true;
}

NLMISC_COMMAND(get_sheet_id, "get_sheet_id","<sheet file name>")
{
	if (args.size() != 1)
		return false;
	CSheetId id(args[0]);

	char buf[10];
	itoa( id.asInt(),buf,10 );
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(buf));
	return true;
}

NLMISC_COMMAND(get_sheet_name, "get_sheet_name","<Sheet Id>")
{
	if (args.size() != 1)
		return false;
	CSheetId id( atoi(args[0].c_str()) );

	string name = id.toString();

	
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(name));
	return true;
}

NLMISC_COMMAND(forget_all, "forget all bricks", "")
{
	// Check parameters.
	if(args.size() != 0)
	{
		return false;
	}
	char buf[100];
	for (uint i = 0;i<20;i++)
	{
		sprintf(buf,"SERVER:BRICK_FAMILY:%d:BRICKS",i);
		CCDBNodeLeaf * node= CInterfaceManager::getInstance()->getDbProp(buf);
		node->setValue64(0);
	}
	return true;

} // forget_all //



//-----------------------------------------------
// 'use_preprog_magic'
//-----------------------------------------------
NLMISC_COMMAND(use_preprog_magic, "use the specified magic preprog sentence", "<sentence id>")
{
	// Check parameters.
	if(args.size() != 1)
	{
		return false;
	}

	// Create the message for the server to execute a phrase.
	const string msgName = "SENTENCE:EXECUTE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("SENTENCE:EXECUTE", out))
	{
		uint8 phrase = atoi(args[0].c_str() );
		out.serial(phrase);

		BRICK_TYPE::EBrickType type = BRICK_TYPE::MAGIC;
		out.serialEnum( type );

		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;

} // use_preprog_magic //




//-----------------------------------------------
// 'use_preprog_combat'
//-----------------------------------------------
NLMISC_COMMAND(use_preprog_combat, "use the specified combat preprog sentence", "<sentence id>")
{
	// Check parameters.
	if(args.size() != 1)
	{
		return false;
	}

	// Create the message for the server to execute a phrase.
	const string msgName = "SENTENCE:EXECUTE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("SENTENCE:EXECUTE", out))
	{
		uint8 phrase = atoi(args[0].c_str() );
		out.serial(phrase);

		BRICK_TYPE::EBrickType type = BRICK_TYPE::COMBAT;
		out.serialEnum( type );

		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;

} // use_preprog_combat //



//-----------------------------------------------
// 'engage'
//-----------------------------------------------
NLMISC_COMMAND(engage, "engage target in combat", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "COMBAT:ENGAGE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // engage //


//-----------------------------------------------
// 'default_attack'
//-----------------------------------------------
NLMISC_COMMAND(default_attack, "use default attack on target", "")
{
	// Default attack on the current selection.
	UserEntity.attack();

	// Well Done.
	return true;
}// default_attack //

//-----------------------------------------------
// 'disengage'
//-----------------------------------------------
NLMISC_COMMAND(disengage, "disengage from combat", "")
{
	// Disengage from combat.
	UserEntity.disengage();

	// Well Done.
	return true;
}// disengage //

//-----------------------------------------------
// 'create_team'
//-----------------------------------------------
NLMISC_COMMAND(create_team, "create a new team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:CREATE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // create_team //


//-----------------------------------------------
// 'leave_team'
//-----------------------------------------------
NLMISC_COMMAND(leave_team, "leave team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:LEAVE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // leave_team //


//-----------------------------------------------
// 'join_team'
//-----------------------------------------------
NLMISC_COMMAND(join_team, "join the specified team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // join_team //


//-----------------------------------------------
// 'join_team_proposal'
//-----------------------------------------------
NLMISC_COMMAND(join_team_proposal, "propose to current target to join the team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN_PROPOSAL";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // join_team_proposal //

//-----------------------------------------------
// 'join_team_decline'
//-----------------------------------------------
NLMISC_COMMAND(join_team_decline, "decline a join team proposal", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN_PROPOSAL_DECLINE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // join_team_decline //

//-----------------------------------------------
// 'kick_teammate'
//-----------------------------------------------
NLMISC_COMMAND(kick_teammate, "kick someone from your team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:KICK";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'.", msgName.c_str());

	return true;
} // kick_teammate //

//-----------------------------------------------
// 'cancel_current_sentence'
//-----------------------------------------------
NLMISC_COMMAND(cancel_current_sentence, "cancel the sentence being executed", "")
{
	// no parameter needed

	// Create the message for the server to cancel the phrase being executed
	const string msgName = "SENTENCE:CANCEL_CURRENT";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("command : unknown message name : '%s'.", msgName.c_str());

	return true;
} // cancel_current_sentence //


//-----------------------------------------------
// 'cancel_all_sentences'
//-----------------------------------------------
NLMISC_COMMAND(cancel_all_sentences, "cancel all the sentences being executed", "")
{
	// no parameter needed

	// Create the message for the server to cancel the phrase being executed
	const string msgName = "SENTENCE:CANCEL_ALL";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("command : unknown message name : '%s'.", msgName.c_str());

	return true;
} // cancel_all_sentences //


//-----------------------------------------------
// 'ignore'
//-----------------------------------------------
NLMISC_COMMAND(ignore, "add or remove a player from the ignore list", "<player name>")
{
	// Check parameters.
	if(args.size() < 1)
	{
		return false;
	}

	// send info
	string player = args[0];
	ChatMngr.ignore( player );

	return true;

} // ignore //


/*
//-----------------------------------------------
// drop :
//-----------------------------------------------
NLMISC_COMMAND(drop,"drop an item to the ground","<id>")
{
	if( args.size() < 1 )
	{
		return false;
	}

	uint32 id = atoi( args[0].c_str() );
	CEntityId itemId(RYZOMID::object,id);
		
	sint32 x = (sint32)UserEntity.pos().x * 1000;
	sint32 y = (sint32)UserEntity.pos().y * 1000;
	sint32 z = (sint32)UserEntity.pos().z * 1000;

	CBitMemStream bms;
	string msgType = "ITEM:DROP";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( itemId );
		bms.serial( x );
		bms.serial( y );
		bms.serial( z );
		NetMngr.push( bms );
		nldebug("<drop> sending 'ITEM:DROP' message to server");
	}
	else
	{
		nlwarning("<drop> unknown message name : ITEM:DROP");
	}

	return true;
}// drop //
*/


//////////
// DEMO //

class CDummyProgress : public IProgressCallback
{
	void progress (float value) {};
};

#ifndef _LIMITED_COMMAND_
//-----------------------------------------------
// teleport :
// Teleport the User somewhere else.
//-----------------------------------------------
void teleport(const CVectorD &pos)
{
	if(!ClientCfg.Local)
	{
		// Create the message for the server to teleport the character.
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("TP:WANTED", out))
		{
			sint32 x = (sint32)(pos.x*1000.0);
			sint32 y = (sint32)(pos.y*1000.0);
			sint32 z = (sint32)(pos.z*1000.0);
			out.serial( x );
			out.serial( y );
			out.serial( z );
			// Add the message to the send list.
			NetMngr.push(out);
			// send TELEPORT
			nldebug("teleport: TELEPORT sent");
		}
		else
			nlwarning("teleport: unknown message name : 'TP:WANTED'.");
	}
	else
	{
		// Remove the selection.
		UserEntity.selection(CLFECOMMON::INVALID_SLOT);
		// Remove the target.
		UserEntity.targetSlot(CLFECOMMON::INVALID_SLOT);
		// Change the position of the entity and in Pacs.
		UserEntity.pos(pos);
		// Select the closest continent from the new position.
		CDummyProgress dummy;
		ContinentMngr.select(pos, dummy);
		// Change the position of the entity and in Pacs.
		UserEntity.pacsPos(pos);
	}
}// teleport //

//-----------------------------------------------
// 'pos' : Command To change the user position.
//-----------------------------------------------
NLMISC_COMMAND(pos, "Change the position of the user.", "<x, y, (z)> OR 1 name of 'tp.teleport_list'. or a bot name")
{
	CVectorD newPos;

	// Named destination.
	if(args.size() == 1)
	{
		string dest = args[0];
		newPos = CTeleport::getPos(NLMISC::strlwr(dest));
		if(newPos == CTeleport::Unknown)
		{
			//here we try to teleport to a bot destination
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream("TP:BOT", out))
			{
				string str = args[0];
				out.serial( str );
				nldebug("/pos: TP:BOT sent");
				NetMngr.push(out);
			}
			else
				nlwarning("/pos: unknown message name : 'TP:BOT'.");
			return true;
		}
	}
	// Teleport to anywhere.
	else if(args.size() == 2 || args.size() == 3)
	{
		newPos.x = atof(args[0].c_str());
		newPos.y = atof(args[1].c_str());
		if(args.size() == 3)
			newPos.z = atof(args[2].c_str());
		else
			newPos.z = 0.0;
	}
	// Bad argument number.
	else
		return false;

	// Teleport to the right destination.
	teleport(newPos);

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'entity' : Create an entity on the user or just remove it if Form not valid.
//-----------------------------------------------
NLMISC_COMMAND(entity, "Create an entity on the user or just remove it if Form not valid.", "<Slot> <Form>")
{
	// Check parameters.
	if(args.size() != 2)
		return false;

	// Try to create the sheet with the parameter as a string.
	CSheetId sheetId;
	if(!sheetId.build(args[1]))
	{
		// Try to create the sheet with the parameter as an int.
		sheetId = atoi(args[1].c_str());
		if(sheetId == CSheetId::Unknown)
		{
			nlwarning("Command 'entity': '%s' is not a valid form.", args[1]);
			return false;
		}
	}

	// The slot where the new entity will be.
	CLFECOMMON::TCLEntityId	slot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());

	// Debug Infos
	nldebug("Command 'entity' : AddNewEntity with form %s in the slot %d.", args[1].c_str(), slot);
	// Remove the old entity.
	EntitiesMngr.remove(slot, false);
	// Create the new entity.
	CEntityCL *entity = EntitiesMngr.create(slot, sheetId.asInt());
	if(entity)
	{
		// Update VP if Player
		if(dynamic_cast<CPlayerCL *>(entity))
		{
			SPropVisualA visualA;
			visualA.PropertySubData.Sex = ClientCfg.Sex;
			SPropVisualB visualB;
			SPropVisualC visualC;
			sint64 *prop = (sint64 *)&visualB;
			IM->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*prop);
			prop = (sint64 *)&visualC;
			IM->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPC))->setValue64(*prop);
			prop = (sint64 *)&visualA;
			IM->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
		}

		// Compute the position.
		sint64 x = (sint64)((UserEntity.pos().x+UserEntity.front().x*2.0)*1000.0);
		sint64 y = (sint64)((UserEntity.pos().y+UserEntity.front().y*2.0)*1000.0);
		sint64 z = (sint64)((UserEntity.pos().z+UserEntity.front().z*2.0)*1000.0);
		// Write the position in the DB.
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
		// Update the position.
		EntitiesMngr.updateVisualProperty(0, slot, 0);
		// Set the direction
		entity->front(UserEntity.front());
		entity->dir(UserEntity.front());

		nlinfo("entity: slot: %d \"%s\", \"%f\", \"%f\", \"%f\", \"%f\", \"%f\", \"%f\" ",slot,args[1].c_str(),
			(UserEntity.pos().x+UserEntity.front().x*2.0), (UserEntity.pos().y+UserEntity.front().y*2.0), (UserEntity.pos().z+UserEntity.front().z*2.0));
//			UserEntity.front().x, UserEntity.front().y, UserEntity.front().z );
	}
	else
		nldebug("command 'entity' : entity in slot %d removed.", slot);

	// Command well done.
	return true;
}// entity //


////////////////////
// DEBUG COMMANDS //
#include "string_manager_client.h"
#ifdef ENABLE_INCOMING_MSG_RECORDER
//-----------------------------------------------
// record
//-----------------------------------------------
NLMISC_COMMAND(record, "Start Recording", "name")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	// Warning when already recording.
	if(NetMngr.isRecording())
	{
		IM->displaySystemInfo(ucstring("Already Recording. Stop the current Record first"));
		return true;
	}

	// Save entities and DB.
	dump(args[0]);

	// On/Off record.
	if(!ClientCfg.Local)
		NetMngr.setRecordingMode(true, args[0]+"_net.rec");
	return true;
}// record //

//-----------------------------------------------
// replay
//-----------------------------------------------
NLMISC_COMMAND(replay, "replay", "name")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	// Load entities and DB.
	loadDump(args[0]);
	
	// On/Off record.
	if(ClientCfg.Local)
		NetMngr.setReplayingMode(!NetMngr.isReplaying(), args[0]+"_net.rec");
	return true;
}// replay //

//-----------------------------------------------
// stopRecord
//-----------------------------------------------
NLMISC_COMMAND(stopRecord, "Stop Recording", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;
	
	// On/Off record.
	if(!ClientCfg.Local)
		NetMngr.setRecordingMode(false);
	return true;
}// stopRecord //
#endif	// ENABLE_INCOMING_MSG_RECORDER

//-----------------------------------------------
// 'dump' : command to create a file with the current state of the client.
//-----------------------------------------------
NLMISC_COMMAND(dump, "Command to create a file with the current state of the client.", "<dump name> = default]")
{
	if(args.size() > 1)
		return false;

	string dumpName;
	if(args.size() == 1)
		dumpName = args[0];
	else
		dumpName = "default";
	
	dump(dumpName);
	return true;
}// dump //

//-----------------------------------------------
// 'loadDump' : command to load a dump file.
//-----------------------------------------------
NLMISC_COMMAND(loadDump, "Ccommand to load a dump file.", "[<dump name> = default]")
{
	if(args.size() > 1)
		return false;

	string dumpName;
	if(args.size() == 1)
		dumpName = args[0];
	else
		dumpName = "default";

	loadDump(dumpName);
	return true;
}// loadDump //


//-----------------------------------------------
// 'watchEntity' : Choose the entity to watch.
//-----------------------------------------------
NLMISC_COMMAND(sheet2idx, "Return the index ", "<sheet name> <Visual Slot Number>")
{
	if(args.size() != 2)
		return false;

	string result;
	NLMISC::CSheetId sheetId;

	if(sheetId.build(args[0]))
	{
		uint32 idx = CVisualSlotManager::getInstance()->sheet2Index(sheetId, (SLOTTYPE::EVisualSlot)atoi(args[1].c_str()));
		result = NLMISC::toString("Index = %d", idx);
	}
	else
		result = NLMISC::toString("sheet '%s' not valid.", args[0].c_str());

	IM->displaySystemInfo(ucstring(result));
	nlinfo("'sheet2idx': %s", result.c_str());
	return true;
}

//-----------------------------------------------
// 'watchEntity' : Choose the entity to watch.
//-----------------------------------------------
NLMISC_COMMAND(watchEntity, "Choose the entity to watch.", "<slot>")
{
	if(args.size() != 1)
		return false;

	// Set the new debug entity slot.
	WatchedEntitySlot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
	return true;
}// watchEntity //

//-----------------------------------------------
// 'dynstr'
//-----------------------------------------------
NLMISC_COMMAND(dynstr, "display a dyn string value", "<dyn string_id>")
{
	if (args.size() != 1)
		return false;
	
	uint dynId = atoi(args[0].c_str());
	
	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynId, result);
	
	CInterfaceManager::getInstance()->displaySystemInfo(result);
	return true;
}// dynstr //

NLMISC_COMMAND(serverstr, "display a server string value", "<serverstr string_id>")
{
	if (args.size() != 1)
		return false;
	
	uint dynId = atoi(args[0].c_str());
	
	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getString(dynId, result);
	
	CInterfaceManager::getInstance()->displaySystemInfo(result);
	return true;
}// serverstr //

//-----------------------------------------------
// 'verboseAnimSelection' : Enable/Disable the animation log for the current selection.
//-----------------------------------------------
NLMISC_COMMAND(verboseAnimSelection, "Enable/Disable the animation log for the current selection.", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	VerboseAnimSelection = !VerboseAnimSelection;
	if(VerboseAnimSelection)
		nlinfo("Enable VerboseAnimSelection");
	else
		nlinfo("Disable VerboseAnimSelection");

	return true;
}// verboseAnimSelection //

//-----------------------------------------------
// 'verboseAnimUser' : Enable/Disable the animation log for the user.
//-----------------------------------------------
NLMISC_COMMAND(verboseAnimUser, "Enable/Disable the animation log for the user.", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	VerboseAnimUser = !VerboseAnimUser;
	if(VerboseAnimUser)
		nlinfo("Enable VerboseAnimUser");
	else
		nlinfo("Disable VerboseAnimUser");

	return true;
}// VerboseAnimUser //

//-----------------------------------------------
// 'verboseDatabase' : Enable/Disable the log for the database.
//-----------------------------------------------
NLMISC_COMMAND(verboseDatabase, "Enable/Disable the log for the database.", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;
	
	VerboseDatabase = !VerboseDatabase;
	if(VerboseDatabase)
		nlinfo("Enable VerboseDatabase");
	else
		nlinfo("Disable VerboseDatabase");
	
	return true;
}// verboseDatabase //

//-----------------------------------------------
// 'verboseVP' : Enable/Disable the visual properties log.
//-----------------------------------------------
NLMISC_COMMAND(verboseVP, "Enable/Disable the visual properties log.", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;
	
	VerboseVP = !VerboseVP;
	if(VerboseVP)
		nlinfo("Enable verboseVP");
	else
		nlinfo("Disable verboseVP");
	
	return true;
}// verboseVP //


//-----------------------------------------------
// verbosePropertiesLoggingMode
//-----------------------------------------------
NLMISC_COMMAND(verbosePropertiesLoggingMode, "Set logging mode", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	CNetworkConnection::LoggingMode = !CNetworkConnection::LoggingMode;
	if(CNetworkConnection::LoggingMode)
		nlinfo("Enable LoggingMode");
	else
		nlinfo("Disable LoggingMode");

	return true;
}


//-----------------------------------------------
// 'where' : display player position (on the server) 
//-----------------------------------------------
NLMISC_COMMAND(where, "display player position (on the server)", " ")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:WHERE", out))
		NetMngr.push(out);
	else
		nlwarning("mainLoop : unknown message name DEBUG:WHERE");
	return true;
}// chat_mode //

//-----------------------------------------------
// 'who' : Display all players currently in game
//-----------------------------------------------
NLMISC_COMMAND(who, "Display all players currently in game"," ")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:WHO", out))
		NetMngr.push(out);
	else
		nlwarning("mainLoop : unknown message name DEBUG:WHO");
	return true;
}// who //

//-----------------------------------------------
// 'cmd' : Send a command to a server
//-----------------------------------------------
NLMISC_COMMAND(cmd, "Send a command to a server","<service name> <cmd>")
{
	// Check parameters.
	if(args.size() < 2)
		return false;
	
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:CMD", out))
	{
		bool addentity = false;
		string dest = args[0];
		string cmd = args[1];
		string arg;
		for (uint i = 2; i < args.size(); i++)
		{
			arg += args[i] + " ";
		}
		out.serial(addentity);
		out.serial(dest);
		out.serial(cmd);
		out.serial(arg);
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name DEBUG:CMD");
	return true;
}// cmd //

//-----------------------------------------------
// 'cmd' : Send a command to a server
//-----------------------------------------------
NLMISC_COMMAND(cmde, "Send a command to a server with entityid","<service name> <cmd>")
{
	// Check parameters.
	if(args.size() < 2)
		return false;
	
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:CMD", out))
	{
		bool addentity = true;
		string dest = args[0];
		string cmd = args[1];
		string arg;
		for (uint i = 2; i < args.size(); i++)
		{
			arg += args[i] + " ";
		}
		out.serial(addentity);
		out.serial(dest);
		out.serial(cmd);
		out.serial(arg);
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name DEBUG:CMD");
	return true;
}// cmde //

//-----------------------------------------------
// 'services' : Ask the server all services up.
//-----------------------------------------------
NLMISC_COMMAND(services, "Ask the server all services up.", "")
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:SERVICES", out))
	{
		// Add the message to the send list.
		NetMngr.push(out);
		// send TELEPORT
		nlinfo("command 'services': 'DEBUG:SERVICES' sent.");
	}
	else
		nlwarning("command 'services': unknown message named 'DEBUG:SERVICES'.");

	return true;
}

//-----------------------------------------------
// 'mode' : Command To change the mode for an entity in a slot.
//-----------------------------------------------
NLMISC_COMMAND(mode, "Command To change the mode for an entity in a slot.", "<Slot> <Mode>")
{
	// Check parameters.
	if(args.size() != 2)
	{
		// Help
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 2 paramters :"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Slot> : the slot number of the entity to change"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Mode> : the mode wanted for the entity, one of the following number :"));
		for(uint i = 0; i<MBEHAV::NUMBER_OF_MODES; ++i)
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d - %s", i, MBEHAV::modeToString((MBEHAV::EMode)i))));
	}
	// Right parameters number
	else
	{
		// Compute parameters
		CLFECOMMON::TCLEntityId slot	= (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
		MBEHAV::EMode mod				= (MBEHAV::EMode)atoi(args[1].c_str());

		// Compute the position.
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_MODE), mod);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_MODE);
		}
		// Invalid slot.
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot."));
	}
	
	// Command well done.
	return true;
}// mode //

//-----------------------------------------------
// 'behaviour' : Command To change the behaviour for an entity in a slot.
//-----------------------------------------------
NLMISC_COMMAND(behaviour, "Command To change the behaviour for an entity in a slot.", "<Slot> <Behaviour> [<Attack Intensity>] [<Impact Intensity>]")
{
	// Check parameters.
	if(args.size() < 2 || args.size() > 4)
	{
		// Help
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 2 or 3 paramters :"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Slot> : the slot number of the entity to change"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Behaviour> : the behaviour to play for the entity, one of the following number :"));
		for(uint i = 0; i<MBEHAV::EMOTE_BEGIN; ++i)
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d - %s", i, MBEHAV::behaviourToString((MBEHAV::EBehaviour)i))));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d-%d - Emotes", MBEHAV::EMOTE_BEGIN, MBEHAV::EMOTE_END)));
	}
	else
	{
		// COmpute parameters
		CLFECOMMON::TCLEntityId	slot		= (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
		MBEHAV::EBehaviour beh				= (MBEHAV::EBehaviour)atoi(args[1].c_str());
		// Make the behaviour
		MBEHAV::CBehaviour behaviour(beh);
		// Get the Power
		if(args.size() >= 3)
			behaviour.Combat.AttackIntensity = atoi(args[2].c_str());
		if(args.size() >= 4)
			behaviour.Combat.ImpactIntensity = atoi(args[3].c_str());
		
		// Compute the position.
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Cast into an uint32.
			uint32 b = behaviour;
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), b);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_BEHAVIOUR);
		}
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot."));
	}
	
	// Command well done.
	return true;
}// behaviour //

//-----------------------------------------------
// 'magic' : Command to cast a spell.
//-----------------------------------------------
NLMISC_COMMAND(magic, "Command to cast a spell.", "\n"
"<Slot> : the one who cast the spell\n"
"<type> : 0->GOOD 1->Bad 2->NEUTRAL\n"
"<success> : 0->success 1->Fail 2->Fumble\n"
"<Spell Power> : \n"
"<Impact Intensity> : \n"
"<resist> : 0->not resisted, any other->resisted.\n")
{
	// Check parameters.
	if(args.size() != 6)
	{
		// Help
//		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 2 or 3 paramters :"));
//		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Slot> : the slot number of the entity to change"));
//		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Behaviour> : the behaviour to play for the entity, one of the following number :"));
//		for(uint i = 0; i<MBEHAV::EMOTE_BEGIN; ++i)
//			CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d - %s", i, MBEHAV::behaviourToString((MBEHAV::EBehaviour)i))));
//		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d-%d - Emotes", MBEHAV::EMOTE_BEGIN, MBEHAV::EMOTE_END)));
	}
	else
	{
		// Compute parameters
		CLFECOMMON::TCLEntityId	slot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
		// Magic Type (good bad neutral)
		uint type = atoi(args[1].c_str())%3;
		MBEHAV::EBehaviour behTmp = (MBEHAV::EBehaviour)(MBEHAV::CASTING_GOOD+type);
		MBEHAV::CBehaviour castingBeh(behTmp);
		// Result
		MBEHAV::CBehaviour behaviour;
		uint result = atoi(args[2].c_str())%3;
		if     (type==0)
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_GOOD_SUCCESS    + result);
		else if(type==1)
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_BAD_SUCCESS     + result);
		else
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_NEUTRAL_SUCCESS + result);
		// Spell Power
		behaviour.Magic.SpellPower      = atoi(args[3].c_str());
		// Impact Intensity
		behaviour.Magic.ImpactIntensity = atoi(args[4].c_str());
		// Resist
		behaviour.Magic.TargetResists   = (atoi(args[5].c_str()) != 0);
		// Get the entity
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Cast into an uint32.
			uint32 b = castingBeh;
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), b);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_BEHAVIOUR);
			b = behaviour;
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), b);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+50, CLFECOMMON::PROPERTY_BEHAVIOUR);
		}
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot."));
	}
	
	// Command well done.
	return true;
}// magic //


//-----------------------------------------------
// 'target' : Set the target for an entity. Do not set the target slot to remove the target.
//-----------------------------------------------
NLMISC_COMMAND(target, "Set a target for an entity. Do not set the target slot to remove the target.", "<Slot> [<Target Slot>]")
{
	CLFECOMMON::TCLEntityId targetSlot = CLFECOMMON::INVALID_SLOT;

	// Check parameters.
	switch(args.size())
	{
	// Set the target for the entity.
	case 2:
		targetSlot = (CLFECOMMON::TCLEntityId)atoi(args[1].c_str());

	// Remove the target for the entity.
	case 1:
	{
		uint entitySlot = (uint)atoi(args[0].c_str());
		CEntityCL *entity = EntitiesMngr.entity(entitySlot);
		if(entity)
			entity->targetSlot(targetSlot);
		else
			nlwarning("command 'target': there is no entity in the slot %d.", entitySlot);
	}
	break;

	// Bad command.
	default:
		return false;
	}

	// Well done.
	return true;
}

//-----------------------------------------------
// 'particle' : Create a particle at the the user position
//-----------------------------------------------
NLMISC_COMMAND(particle, "Create a particule at the user position (play FireWorkA_with_sound.ps by default)", "[<filename.ps>]")
{
	string fn;
	
	// Check parameters.
	if(args.size() == 0)
	{
		fn = "FireWorkA_with_sound.ps";
	}
	else if(args.size() == 1)
	{
		fn = args[0];
	}
	else
		return false;

	UInstance *fx = Scene->createInstance(fn);

	// not found
	if(fx == NULL)
	{
		log.displayNL ("Can't create instance '%s'", fn.c_str());
		return false;
	}

	fx->setPos(UserEntity.pos());
	fx->setClusterSystem(UserEntity.skeleton()->getClusterSystem());

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'move' : Move an entity.
//-----------------------------------------------
NLMISC_COMMAND(move, "Move an entity.", "Form ID; Slot: [1-254]")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	CLFECOMMON::TCLEntityId	slot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
	{
		sint64 x = (sint64)((entity->pos().x+UserEntity.front().x*10.0)*1000.0);
		sint64 y = (sint64)((entity->pos().y+UserEntity.front().y*10.0)*1000.0);
		sint64 z = (sint64)((entity->pos().z+UserEntity.front().z*10.0)*1000.0);
		// Write the position in the DB.
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
		// Update the position.
		EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+30, slot, 0);

		x = (sint64)((entity->pos().x)*1000.0);
		y = (sint64)((entity->pos().y)*1000.0);
		z = (sint64)((entity->pos().z)*1000.0);
		// Write the position in the DB.
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
		// Update the position.
		EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+60, slot, 0);
	}
	else
		nlwarning("command 'move' : there is no entity allocated in slot %d.", slot);

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'move_to' : Move an entity to another one.
//-----------------------------------------------
NLMISC_COMMAND(move_to, "Move an entity to another one.", "<slot(from)>:[1-254], <slot(to)>:[0-254] default 0")
{
	sint64 x, y, z;

	// Check parameters.
	if(args.size() == 1)
	{
		x = (sint64)(UserEntity.pos().x*1000.0);
		y = (sint64)(UserEntity.pos().y*1000.0);
		z = (sint64)(UserEntity.pos().z*1000.0);
	}
	else if(args.size() == 2)
	{
		CLFECOMMON::TCLEntityId	slotDest = (CLFECOMMON::TCLEntityId)atoi(args[1].c_str());
		// Compute the position.
		CEntityCL *entity = EntitiesMngr.entity(slotDest);
		if(entity)
		{
			x = (sint64)(entity->pos().x*1000.0);
			y = (sint64)(entity->pos().y*1000.0);
			z = (sint64)(entity->pos().z*1000.0);
		}
		else
		{
			// Command is correct but not all the parameters are valid.
			nlwarning("command 'move_to' : there is no entity allocated for the dest in slot %d.", slotDest);
			return true;
		}
	}
	// Wrong number of parameters.
	else
		return false;

	CLFECOMMON::TCLEntityId	slot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
	// Write the position in the DB.
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
	// Update the position.
	EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+30, slot, 0);

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'setMode' : Set The Mode for an Entity without to add a stage for it.
//-----------------------------------------------
NLMISC_COMMAND(setMode, "Set The Mode for an Entity without to add a stage for it.", "<slot> <mode>")
{
	// Check parameters.
	if(args.size() != 2)
		return false;

	// Get the Slot and the Mode.
	CLFECOMMON::TCLEntityId slot = (CLFECOMMON::TCLEntityId)atoi(args[0].c_str());
	MBEHAV::EMode mod			 = (MBEHAV::EMode)atoi(args[1].c_str());

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
		entity->mode(mod);
	else
		nlwarning("command 'setMode' : there is no entity allocated in slot '%d'.", slot);

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'logEntities' : Write the position and orientation af all entities in the vision in the file 'entities.txt'.
//-----------------------------------------------
NLMISC_COMMAND(logEntities, "Write the position and orientation af all entities in the vision in the file 'entities.txt'.", "")
{
	// Check parameters
	if(args.size() != 0)
		return false;

	// Log entities
	EntitiesMngr.writeEntities();

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'log' : Command to Add/Del Positive/Negative Filters for logs.
//-----------------------------------------------
NLMISC_COMMAND(log, "Add/Del Positive/Negative Filters for logs.", "Log System <debug, info, warning, assert>, Type <pos/neg/del/reset>, Filter <string>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() < 2 || args.size() > 3)
		return false;

	CLog *logSys;
	// Debug log system.
	if     (string(args[0].c_str()) == "debug")
		logSys = DebugLog;
	// Info log system.
	else if(string(args[0].c_str()) == "info")
		logSys = InfoLog;
	// Warning log system.
	else if(string(args[0].c_str()) == "warning")
		logSys = WarningLog;
	// Assert log system.
	else if(string(args[0].c_str()) == "assert")
		logSys = AssertLog;
	// Unknown Log System -> return false.
	else
		return false;

	// Add a positive filter.
	if     (string(args[1].c_str()) == "pos")
		logSys->addPositiveFilter(args[2].c_str());
	// Add a negative filter.
	else if(string(args[1].c_str()) == "neg")
		logSys->addNegativeFilter(args[2].c_str());
	// Removes a filter by name (in both filters).
	else if(string(args[1].c_str()) == "del")
		logSys->removeFilter(args[2].c_str());
	// Reset both filters.
	else if(string(args[1].c_str()) == "reset")
		logSys->resetFilters();
	// Unknown Filter -> return false.
	else
		return false;

	// Command well done.
	return true;
}

//-----------------------------------------------
// 'db' : Command to change some variables in the database or to get it
//-----------------------------------------------
NLMISC_COMMAND(db, "Modify Database","<Property> <Value>")
{
	int size = args.size();
	if (size == 2)
	{
		// Convert the string into an sint64.
		sint64 value = atoiInt64(args[1].c_str());
		// Set the property.
		CInterfaceManager::getInstance()->getDbProp(args[0])->setValue64(value);
	}
	else if (size == 1)
	{
		sint64 prop = CInterfaceManager::getInstance()->getDbProp(args[0])->getValue64();
		string str = toString(prop);
		
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->displaySystemInfo(ucstring(str));		
		nlinfo("%s", str.c_str());
		return true;
	}
	else
		return false;

	return true;
}// db //

//-----------------------------------------------
// 'exec' : Execute a scipt file
//-----------------------------------------------
extern CLog	g_log;

NLMISC_COMMAND(exec, "Execute a scipt file","<FileName>")
{
	int size = args.size();
	if (size != 1)
		return false;

	CIFile iFile;

	if (iFile.open(CPath::lookup(args[0], false)))
	{
		char line[512];
		char *buffer;
		// Read line by line and execute each line


		bool eof = false;
		while (!eof)
		{
			buffer = &line[0];
			uint read = 0;
			while (true)
			{
				if (read == 512 -1)
				{
					*buffer = '\0';
					break;
				}

				try
				{
					// read one byte
					iFile.serialBuffer ((uint8 *)buffer, 1);
				}
				catch (EFile &)
				{
					*buffer = '\0';
					eof = true;
					break;
				}

				if (*buffer == '\n')
				{
					*buffer = '\0';
					break;
				}

				// skip '\r' char
				if (*buffer != '\r')
				{
					buffer++;
					read++;
				}
			}
			if (strlen(line) > 0)
				ICommand::execute(line, g_log);
			if (iFile.eof()) 
				eof = true;
		}
		iFile.close();
	}
	else
	{		
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("Cannot open file"));
	}
	
	return true;
}// exec //

//-----------------------------------------------
// 'vP' : Modify the Visual Property.
//-----------------------------------------------
NLMISC_COMMAND(vP, "Modify the Visual Property.",
"\n"
"<slot> of the entity to change.\n"
"<type> the property to change :\n"
"  0->CHEST\n"
"  1->LEG\n"
"  2->ARM\n"
"  3->HEAD\n"
"  4->WEAPON_R\n"
"  5->WEAPON_L\n"
"  6->FEET\n"
"  7->HAND\n"
"  8->EYES\n"
"  9->SEX (0: Male, 1: Female)\n"
"<value> for the property.\n")
{
	// Check parameters
	if(args.size() != 3)
		return false;

	// Get the database entry.
	SPropVisualA vA;
	SPropVisualB vB;
	SPropVisualC vC;
	uint slot = atoi(args[0].c_str());
	const string propNameA = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPA);
	const string propNameB = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPB);
	const string propNameC = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPC);
	vA.PropertyA = CInterfaceManager::getInstance()->getDbProp(propNameA)->getValue64();
	vB.PropertyB = CInterfaceManager::getInstance()->getDbProp(propNameB)->getValue64();
	vC.PropertyC = CInterfaceManager::getInstance()->getDbProp(propNameC)->getValue64();
	// Get the visual item index
	uint value = atoi(args[2].c_str());
	// Get the visual slot to change.
	uint type = atoi(args[1].c_str());
	switch(type)
	{
	case 0:
		vA.PropertySubData.JacketModel = value;
		break;
	case 1:
		vA.PropertySubData.TrouserModel = value;
		break;
	case 2:
		vA.PropertySubData.ArmModel = value;
		break;
	case 3:
		vA.PropertySubData.HatModel = value;
		break;
	case 4:
		vA.PropertySubData.WeaponRightHand = value;
		break;
	case 5:
		vA.PropertySubData.WeaponLeftHand = value;
		break;
	case 6:
		vB.PropertySubData.FeetModel = value;
		break;
	case 7:
		vB.PropertySubData.HandsModel = value;
		break;
	case 8:
		vC.PropertySubData.EyesColor = value;
		break;
	case 9:
		vA.PropertySubData.Sex = value;
		break;

	default:
		nlwarning("command 'vP': type not valid.");
		return false;
		break;
	}

	// Set the database.
	CInterfaceManager::getInstance()->getDbProp(propNameA)->setValue64((sint64)vA.PropertyA);
	CInterfaceManager::getInstance()->getDbProp(propNameB)->setValue64((sint64)vB.PropertyB);
	CInterfaceManager::getInstance()->getDbProp(propNameC)->setValue64((sint64)vC.PropertyC);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);

	// Done.
	return true;
}// vP //


//-----------------------------------------------
// 'altLook' : Modify the Visual Property.
//-----------------------------------------------
NLMISC_COMMAND(altLook, "Modify the Alternative Look Property.",
			   "\n"
			   "<slot> of the entity to change.\n"
			   "<colorTop>\n"
			   "<colorBottom>\n"
			   "<rWeapon>\n"
			   "<lWeapon>\n"
			   "<seed>\n"
			   "<hairColor>\n"
			   "<putHelm>\n")
{
	// Check parameters
	if(args.size() != 8)
		return false;
	
	// Get the database entry.
	uint slot = atoi(args[0].c_str());
	const string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPA);
	// Get the old value (not useful since we change the whole property).
	SAltLookProp altLookProp;
	altLookProp.Summary = CInterfaceManager::getInstance()->getDbProp(propName)->getValue64();
	altLookProp.Element.ColorTop		= atoi(args[1].c_str());
	altLookProp.Element.ColorBot		= atoi(args[2].c_str());
	altLookProp.Element.WeaponRightHand	= atoi(args[3].c_str());
	altLookProp.Element.WeaponLeftHand	= atoi(args[4].c_str());
	altLookProp.Element.Seed			= atoi(args[5].c_str());
	altLookProp.Element.ColorHair		= atoi(args[6].c_str());
	altLookProp.Element.Hat				= atoi(args[7].c_str());
	
	// Set the database.
	CInterfaceManager::getInstance()->getDbProp(propName)->setValue64((sint64)altLookProp.Summary);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
	
	// Done.
	return true;
}// altLook //

//-----------------------------------------------
// 'color' : Command to color an entity.
//-----------------------------------------------
NLMISC_COMMAND(color, "Command to color an entity.",
"\n"
"<Slot>: whole number (if <0 slot will be the current selection)\n"
"<UserColor>: whole number\n"
"<Hair>: whole number\n"
"<Eyes>: whole number\n"
"[<Part>]: whole number\n"
"  default=the whole body\n"
"  0=CHEST\n"
"  1=LEG\n"
"  2=HEAD\n"
"  3=ARMS\n"
"  4=HANDS\n"
"  5=FEET\n")
{
	// Check parameters.
	if(args.size() != 4 && args.size() != 5)
		return false;

	// Witch part to dye ?
	sint part = -1;
	if(args.size() == 5)
		part = atoi(args[4].c_str());

	// Get the entity slot to dye.
	sint slotTmp = atoi(args[0].c_str());
	CLFECOMMON::TCLEntityId	slot;
	if(slotTmp >= 0)
		slot = (CLFECOMMON::TCLEntityId)slotTmp;
	else
		slot = (CLFECOMMON::TCLEntityId)UserEntity.selection();

	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
		entity->changeColors(atoi(args[1].c_str()), atoi(args[2].c_str()), atoi(args[3].c_str()), part);
	else
		nlwarning("command 'changeColors': there is no entity allocated in slot '%d'.", slot);
	
	// Command well done.
	return true;
}// color //
#endif	// #ifndef _LIMITED_COMMAND_


//-----------------------------------------------
// 'fire' : Command to launch the firework.
//-----------------------------------------------
NLMISC_COMMAND(fire, "launch the firework","")
{
	Firework->start(T1);
	return true;
}

//-----------------------------------------------
// 'save_int_cfg' : save the interface config file.
//-----------------------------------------------
NLMISC_COMMAND(save_int_cfg, "save the interface config file","")
{
	CInterfaceManager::getInstance()->saveConfig ("interface.cfg");
	return true;
}


//-----------------------------------------------
// 'harvest_deposit'
//-----------------------------------------------
NLMISC_COMMAND(harvest_deposit, "harvest a deposit", "")
{
	// no parameter needed

	// Create the message for the server
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("HARVEST:DEPOSIT", out))
	{
		NetMngr.push(out);

		// open the interface
		CInterfaceManager::getInstance()->getWindowFromId("ui:interface:harvest")->setActive(true);
	}
	else
		nlwarning("command : unknown message name : 'HARVEST:DEPOSIT'.");

	return true;
} // harvest_deposit //

//-----------------------------------------------
// 'training'
//-----------------------------------------------
NLMISC_COMMAND(training, "start a training action", "")
{
	// no parameter needed

	// Create the message for the server
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("TRAINING", out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("command : unknown message name : 'TRAINING'.");

	return true;
} // training //

//-----------------------------------------------
// 'mount' : Set the entity to mount.
//-----------------------------------------------
NLMISC_COMMAND(mount, "Set the entity to mount.","<Slot> [<Mount>]")
{
	CLFECOMMON::TCLEntityId slot;
	CLFECOMMON::TCLEntityId mount = CLFECOMMON::INVALID_SLOT;

	switch(args.size())
	{
	case 2:
		mount = (CLFECOMMON::TCLEntityId) atoi(args[1].c_str());
	case 1:
		slot  = (CLFECOMMON::TCLEntityId) atoi(args[0].c_str());
		break;

	default:
		return false;
		break;
	}

	// Set the database.
	string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	CInterfaceManager::getInstance()->getDbProp(propName)->setValue64(mount);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);

	// Command well done
	return true;
}// mount //

//-----------------------------------------------
// 'rider' : Set the rider.
//-----------------------------------------------
NLMISC_COMMAND(rider, "Set the rider.","<Slot> [<rider>]")
{
	CLFECOMMON::TCLEntityId slot;
	CLFECOMMON::TCLEntityId rider = CLFECOMMON::INVALID_SLOT;

	switch(args.size())
	{
	case 2:
		rider = (CLFECOMMON::TCLEntityId) atoi(args[1].c_str());
	case 1:
		slot  = (CLFECOMMON::TCLEntityId) atoi(args[0].c_str());
		break;

	default:
		return false;
		break;
	}

	// Set the database.
	string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	CInterfaceManager::getInstance()->getDbProp(propName)->setValue64(rider);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);

	// Command well done
	return true;
}// rider //



#ifndef _LIMITED_COMMAND_
#ifdef _CMD_CHEAT_ON_
////////////////////
// CHEAT COMMANDS //

NLMISC_COMMAND(learn_all, "learn all bricks", "all magic combat")
{
	// Check parameters.
	if(args.size() > 1)
		return false;
	uint type = 0;
	if(args.size() == 1)
	{
		if (args[0] == "magic" || args[0] == "1")
			type = 1;
		if (args[0] == "combat" || args[0] == "2")
			type = 2;
	}
	char buf[100];
	if (ClientCfg.Local)
	{
		CBrickManager * pBM = CBrickManager::getInstance();
		uint i = 0;
		for (; i < pBM->getNumberOfFamily(); i++ )
		{
			sint64 flag = 0;
			if ( pBM->getFamilySize(i) )
			{
				CBrickSheet *brick= pBM->getBrick(pBM->getBrickSheet(i, 0));
				if(brick)
				{
					if ( type == 0 || (brick->BrickType == BRICK_TYPE::COMBAT && type == 2) || (brick->BrickType == BRICK_TYPE::MAGIC && type == 1) )
					{
						for (uint j = 0; j < pBM->getFamilySize(i);j++)
						{
							flag |= (sint64)1 << (sint64)j;
						}
						sprintf(buf,"SERVER:BRICK_FAMILY:%d:BRICKS",i);
						CCDBNodeLeaf * node= CInterfaceManager::getInstance()->getDbProp(buf);
						node->setValue64(flag);
					}
				}
			}
		}
		//set all career to 25
		for (i = 0; i < ROLES::role_unknown; i++ )
		{
			string role = string("SERVER:CHARACTER_INFO:CAREER")+toString(3);
			string prop = role +string(":LEVEL");
			CCDBNodeLeaf * node= CInterfaceManager::getInstance()->getDbProp(prop);
			node->setValue64(25);
			for (uint j = 0; j < 8; j++ )
			{
				prop = role + string(":JOB") + toString(j) + string(":LEVEL");
				node= CInterfaceManager::getInstance()->getDbProp(prop);
				node->setValue64(25);
			}
		}
		// Learn all Faber Plans. For all database entries
		uint	faberPlanMId= 0;
		while(true)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CCDBNodeLeaf	*leaf= pIM->getDbProp("SERVER:FABER_PLANS:"+toString(faberPlanMId)+":KNOWN", false);
			if(!leaf)
				break;
			leaf->setValue64(0xFFFFFFFFFFFFFFFF);
			faberPlanMId++;
		}
	}
	else
	{
		// Create the message for the server to learn all bricks
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:LEARN_ALL_BRICKS", out))
		{
			// Add the message to the send list.
			NetMngr.push(out);
			// send TELEPORT
			nlinfo("command 'learn_all': learn all bricks Asked.");
		}
		else
			nlwarning("command 'learn_all': unknown message named 'CHEAT:LEARN_ALL_BRICKS'.");
	}
	return true;

} // learn_all //


NLMISC_COMMAND(learnAllFaberPlans, "learn all faber plans", "")
{
	if (ClientCfg.Local)
	{
		// Learn all Faber Plans. For all database entries
		uint	faberPlanMId= 0;
		while(true)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CCDBNodeLeaf	*leaf= pIM->getDbProp("SERVER:FABER_PLANS:"+toString(faberPlanMId)+":KNOWN", false);
			if(!leaf)
				break;
			leaf->setValue64(0xFFFFFFFFFFFFFFFF);
			faberPlanMId++;
		}
	}
	else
	{
		// Create the message for the server to learn all bricks
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:LEARN_ALL_FABER_PLANS", out))
		{
			// Add the message to the send list.
			NetMngr.push(out);
			// send TELEPORT
			nlinfo("command 'learnAllFaberPlans': learn all faber plans Asked.");
		}
		else
			nlwarning("command 'learnAllFaberPlans': unknown message named 'CHEAT:LEARN_ALL_FABER_PLANS'.");
	}
	return true;
} // learnAllFaberPlans //

//-----------------------------------------------
// 'god' : To be invulnerable.
//-----------------------------------------------
NLMISC_COMMAND(god, "To be invulnerable","")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	// Create the message for the server to teleport the character.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:GOD", out))
	{
		// Add the message to the send list.
		NetMngr.push(out);
		// send TELEPORT
		nlinfo("command 'god': God Mode Asked.");
	}
	else
		nlwarning("command 'god': unknown message named 'CHEAT:GOD'.");

	// Done.
	return true;
}// god //

//-----------------------------------------------
// 'createItem' : Create an item in the hand of the user.
//-----------------------------------------------
NLMISC_COMMAND(createItem, "Create an item in the hand of the user.","<Form> [<Quantity>] [<Quality>]")
{
	CSheetId sheetId;
	uint16 quantity = 1;
	uint16 quality = 1;

	// Check parameters.
	switch(args.size())
	{
	case 3:
		quality = atoi(args[2].c_str());

	case 2:
		quantity = atoi(args[1].c_str());

	case 1:
	// Try to create the sheet with the parameter as a string.
	if(!sheetId.build(args[0]))
	{
		// Try to create the sheet with the parameter as an int.
		sheetId = atoi(args[0].c_str());
	}
	break;

	// Bad number of parameters.
	default:
		nlwarning("Command 'createItem': invalid number of parameter.");
		return false;
	}

	// Check Sheet Id.
	if(sheetId == CSheetId::Unknown)
	{
		nlwarning("Command 'createItem': '%s' is not a valid form.", args[0].c_str());
		return false;
	}

	// Create the message for the server to teleport the character.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:CREATE_ITEM_IN_BAG", out))
	{
		uint32 sheet = sheetId.asInt();
		out.serial(sheet);
		out.serial(quantity);
		out.serial(quality);
		// Add the message to the send list.
		NetMngr.push(out);
		// send TELEPORT
		nlinfo("Command 'createItem': CREATE_ITEM_IN_BAG sent");
	}
	else
		nlwarning("Command 'createItem': unknown message named 'CHEAT:CREATE_ITEM_IN_BAG'.");

	// Command well done.
	return true;
}// createItem //

//-----------------------------------------------
// 'xp' : To gain XP in a given Skill.
//-----------------------------------------------
NLMISC_COMMAND(xp, "To gain XP in a given Skill.","<Skill> <Amount>")
{
	// Check parameters.
	if(args.size() != 2)
		return false;

	// Create the message for the server to teleport the character.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:XP", out))
	{
		uint32 xp = atoi(args[1].c_str());
		string skill = args[0];
		out.serial( skill );
		out.serial(xp);
		// Add the message to the send list.
		NetMngr.push(out);
		// send CHEAT:XP
		nlinfo("command 'xp': CHEAT:XP pushed.");
	}
	else
		nlwarning("command 'xp': unknown message named 'CHEAT:XP'.");

	// Done.
	return true;
}// xp //

//-----------------------------------------------
// 'money' : To earn Money.
// \todo 
//-----------------------------------------------
NLMISC_COMMAND(money, "To earn Money.","<very big seed> [<big seed>] [<medium seed>] [<small seed>]")
{
	sint32 a = 0;
	sint32 b = 0;
	sint32 c = 0;
	sint32 d = 0;

	// Check parameters.
	switch(args.size())
	{
	case 4:
		d = atoi(args[3].c_str());
	case 3:
		c = atoi(args[2].c_str());
	case 2:
		b = atoi(args[1].c_str());
	case 1:
		a = atoi(args[0].c_str());
		break;
	default:
		return false;
	}

	// Create the message for the server to teleport the character.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:MONEY", out))
	{
		out.serial(a);
		out.serial(b);
		out.serial(c);
		out.serial(d);
		// Add the message to the send list.
		NetMngr.push(out);
		// send CHEAT:MONEY
		nlinfo("command 'money': CHEAT:MONEY pushed.");
	}
	else
		nlwarning("command 'money': unknown message named 'CHEAT:MONEY'.");

	// Done.
	return true;
}// money //

//-----------------------------------------------
// 'setTime' : To set Ryzom time on server.
//-----------------------------------------------
NLMISC_COMMAND(setTime, "To set Ryzom time on server.","<Time>")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	// Create the message for the server to set ryzom time.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:SET_TIME", out))
	{
		float time = (float)( atof(args[0].c_str()) );
		out.serial(time);
		// Add the message to the send list.
		NetMngr.push(out);
		// send CHEAT:SET_TIME
		nlinfo("command 'setTime': CHEAT:SET_TIME pushed.");
	}
	else
		nlwarning("command 'setTime': unknown message named 'CHEAT:SET_TIME'.");

	// Done.
	return true;
}// setTime //

//-----------------------------------------------
// 'setDay' : To set Ryzom day on server.
//-----------------------------------------------
NLMISC_COMMAND(setDay, "To set Ryzom day on server.","<Day>")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	// Create the message for the server to set ryzom time.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:SET_DAY", out))
	{
		uint32 Day = (uint32) atoi(args[0].c_str());
		out.serial(Day);
		// Add the message to the send list.
		NetMngr.push(out);
		// send CHEAT:SET_DAY
		nlinfo("command 'setDay': CHEAT:SET_DAY pushed.");
	}
	else
		nlwarning("command 'setDay': unknown message named 'CHEAT:SET_DAY'.");

	// Done.
	return true;
}// setTime //

//-----------------------------------------------
// createPerso :
//-----------------------------------------------
NLMISC_COMMAND( createPerso, "create a new character", "Parameters:\n-Character name\n-Race( Fyros, Tryker...)\n-gender(Male, Female)\n-Role( MeleeFighter, RangeFighter, AttackCaster, BufferCaster, HealerCaster...)\n-Level (1-25 (but more accepted)>" )
{
	// Check parameters.
	if(args.size() < 5) return false;

	// read params
	string characterName = args[0];
	EGSPD::CPeople::TPeople race = EGSPD::CPeople::fromString( args[1] );
	if( race == EGSPD::CPeople::unknown ) return false;

	GSGENDER::EGender gender = GSGENDER::stringToEnum( args[2] );
	if( gender == GSGENDER::unknown ) return false;

	ROLES::ERole role = ROLES::toRoleId( args[3] );
	if( role == ROLES::role_unknown ) return false;

	uint16 level = atoi( args[4].c_str() );

	CBitMemStream bms;
	string msgType = "CHEAT:CREATE_CHARACTER";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( characterName );
		bms.serialEnum( race );
		bms.serialEnum( gender );
		bms.serialEnum( role );
		bms.serial( level );
		NetMngr.push( bms );
		nldebug("<create_perso> sending 'CHEAT:CREATE_CHARACTER' message to server");
	}
	else
	{
		nlwarning("<create_perso> unknown message name : CHEAT:CREATE_CHARACTER");
	}
	return true;
} // createPerso //

/*
//-----------------------------------------------
// add_role :
//-----------------------------------------------
NLMISC_COMMAND( add_role, "add role to character", "<Role( MeleeFighter, RangeFighter, AttackCaster, BufferCaster, HealerCaster...), Level (1-25 (but more accepted))>" )
{
	// Check parameters.
	if(args.size() < 2) return false;

	ROLES::ERole role = ROLES::toRoleId( args[0] );
	if( role == ROLES::role_unknown ) return false;

	uint16 level = atoi( args[1].c_str() );

	CBitMemStream bms;
	string msgType = "CHEAT:ADD_ROLE";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serialEnum( role );
		bms.serial( level );
		NetMngr.push( bms );
		nldebug("<add_role> sending 'CHEAT:ADD_ROLE' message to server");
	}
	else
	{
		nlwarning("<add_role> unknown message name : CHEAT:ADD_ROLE");
	}
	return true;
} // add_role //
*/

//-----------------------------------------------
// learn_bricks :
//-----------------------------------------------
NLMISC_COMMAND( learn_bricks, "learn brick for role and level", "<Role( MeleeFighter, RangeFighter, AttackCaster, BufferCaster, HealerCaster...), Level (1-25 (but more accepted))>" )
{
	// Check parameters.
	if(args.size() < 2) return false;

	ROLES::ERole role = ROLES::toRoleId( args[0] );
	if( role == ROLES::role_unknown ) return false;

	uint16 level = atoi( args[1].c_str() );

	CBitMemStream bms;
	string msgType = "CHEAT:LEARN_BRICK";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serialEnum( role );
		bms.serial( level );
		NetMngr.push( bms );
		nldebug("<learn_bricks> sending 'CHEAT:LEARN_BRICK' message to server");
	}
	else
	{
		nlwarning("<learn_bricks> unknown message name : CHEAT:LEARN_BRICK");
	}
	return true;
} // learn_bricks


#endif	// _CMD_CHEAT_ON_


///////////////////
// TEST COMMANDS //
//-----------------------------------------------
// 'test' :
//-----------------------------------------------
NLMISC_COMMAND(test, "", "")
{
	sint64 x, y, z;

	MBEHAV::EMode mod = MBEHAV::MOUNT_NORMAL;
	MBEHAV::EMode mod2 = MBEHAV::NORMAL;
	const NLMISC::TGameCycle cst = NetMngr.getCurrentServerTick();
	NLMISC::TGameCycle gc;

	CLFECOMMON::TCLEntityId	entSlot = (CLFECOMMON::TCLEntityId)1;
	CEntityCL *entPtr = EntitiesMngr.entity(entSlot);
	if(entPtr)
	{
		CLFECOMMON::TCLEntityId	mountSlot = (CLFECOMMON::TCLEntityId)2;
		// Compute the position.
		CEntityCL *mountPtr = EntitiesMngr.entity(mountSlot);
		if(mountPtr)
		{
			//
			gc = (NLMISC::TGameCycle)(cst+10);
			//
			x = (sint64)((mountPtr->pos().x+2)*1000.0);
			y = (sint64)((mountPtr->pos().y)*1000.0);
			z = (sint64)((mountPtr->pos().z)*1000.0);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX),              x);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY),              y);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ),              z);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_MODE),              mod);			// Mode
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID), mountSlot);	// Mount
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_POSITION);
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_MODE);
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
			//
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_MODE), mod);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_RIDER_ENTITY_ID), entSlot);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_MODE);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);

/*
			//
			gc = (NLMISC::TGameCycle)(cst+30);
			//
			x = (sint64)((mountPtr->pos().x+21)*1000.0);
			y = (sint64)((mountPtr->pos().y+20)*1000.0);
			z = (sint64)((mountPtr->pos().z)*1000.0);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX), x);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY), y);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ), z);
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_MODE),              mod2);			// Mode
			IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID), CLFECOMMON::INVALID_SLOT);	// Mount
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_POSITION);
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_MODE);
			entPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
			//
			x = (sint64)((mountPtr->pos().x+20)*1000.0);
			y = (sint64)((mountPtr->pos().y+20)*1000.0);
			z = (sint64)((mountPtr->pos().z)*1000.0);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX), x);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY), y);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ), z);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_MODE), mod2);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_RIDER_ENTITY_ID), CLFECOMMON::INVALID_SLOT);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_POSITION);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_MODE);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);


			//
			gc = (NLMISC::TGameCycle)(cst+50);
			//
			x = (sint64)((mountPtr->pos().x)*1000.0);
			y = (sint64)((mountPtr->pos().y)*1000.0);
			z = (sint64)((mountPtr->pos().z)*1000.0);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX), x);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY), y);
			IngameDbMngr.setProp("Entities:E" + toString("%d", mountSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ), z);
			mountPtr->updateVisualProperty(gc, CLFECOMMON::PROPERTY_POSITION);
*/
		}
	}

	return true;
}


//-----------------------------------------------
/// Macro to set the new dist to front(back or side) for a given sheet.
/// commandName  : Name of the command.
/// variableName : Variable Name to change.
//-----------------------------------------------
#define DIST_TO_COMMAND(commandName, variableName)																	\
	/* Check Parameters */																							\
	if(args.size() != 2)																							\
	{																												\
		nlwarning("Command '" #commandName "': need 2 parameters, try '/help " #commandName "' for more details.");	\
		return false;																								\
	}																												\
																													\
	/* Try to create the sheet with the parameter as a string. */													\
	CSheetId sheetId;																								\
	if(!sheetId.build(args[0]))																						\
	{																												\
		/* Try to create the sheet with the parameter as an int. */													\
		sheetId = atoi(args[0].c_str());																			\
		if(sheetId == CSheetId::Unknown)																			\
		{																											\
			nlwarning("Command '" #commandName "': '%s' is not a valid form.", args[0]);							\
			return false;																							\
		}																											\
	}																												\
																													\
	/* Get the new distance. */																						\
	float dist = (float)atof(args[1].c_str());																		\
	if(dist < 0)																									\
	{																												\
		nlwarning("Command '" #commandName "': distance < 0, this is not good.");									\
		return false;																								\
	}																												\
																													\
	CCharacterSheet *ch = dynamic_cast<CCharacterSheet *>(SheetMngr.get(sheetId));									\
	if(ch == 0)																										\
	{																												\
		nlwarning("Command '" #commandName "': cannot find the character for the given sheet.");					\
		return false;																								\
	}																												\
																													\
	/* Set the new distance for this sheet. */																		\
	ch->variableName = dist;																						\
																													\
	/* Well Done */																									\
	return true;																									\

//-----------------------------------------------
// 'dist2front' : Change the distance to the front for a given sheet.
//-----------------------------------------------
NLMISC_COMMAND(dist2front, "Change the distance to the front for a given sheet.", "<form> <dist>")
{
	DIST_TO_COMMAND(dist2front, DistToFront);
}

//-----------------------------------------------
// 'dist2back' : Change the distance to the back for a given sheet.
//-----------------------------------------------
NLMISC_COMMAND(dist2back, "Change the distance to the back for a given sheet.", "<form> <dist>")
{
	DIST_TO_COMMAND(dist2back, DistToBack);
}

//-----------------------------------------------
// 'dist2side' : Change the distance to the side for a given sheet.
//-----------------------------------------------
NLMISC_COMMAND(dist2side, "Change the distance to the side for a given sheet.", "<form> <dist>")
{
	DIST_TO_COMMAND(dist2side, DistToSide);
}

//-----------------------------------------------
// 'shape' : Add a shape in the scene.
//-----------------------------------------------
NLMISC_COMMAND(shape, "Add a shape in the scene.", "<shape file>")
{
	if(args.size() != 1)
	{
		nlwarning("Command 'shape': need 1 parameter, try '/help shape' for more details.");
		return false;
	}

	UInstance *instance = Scene->createInstance(args[0]);
	if(instance)
	{
		// Set the position
		instance->setPos(UserEntity.pos());
		// Compute the direction Matrix
		CMatrix dir;
		dir.identity();
		CVector vi = UserEntity.dir()^CVector(0.f, 0.f, 1.f);
		CVector vk = vi^UserEntity.dir();
		dir.setRot(vi, UserEntity.dir(), vk, true);
		// Set Orientation : User Direction should be normalized.
		instance->setRotQuat(dir.getRot());
	}
	else
		nlwarning("Command 'shape': cannot find the shape %s.", args[0].c_str());

	// Command Well Done
	return true;
}

//-----------------------------------------------
// 'parent' : Change the parent of an entity. 'parent slot' not defined remove the current parent.
//-----------------------------------------------
NLMISC_COMMAND(parent, "Change the parent of an entity.", "<slot> [<parent slot>]")
{
	CLFECOMMON::TCLEntityId parentSlot = CLFECOMMON::INVALID_SLOT;

	// Check parameters.
	switch(args.size())
	{
	// Set the target for the entity.
	case 2:
		parentSlot = (CLFECOMMON::TCLEntityId)atoi(args[1].c_str());

	// Remove the target for the entity.
	case 1:
	{
		uint entitySlot = (uint)atoi(args[0].c_str());
		CEntityCL *entity = EntitiesMngr.entity(entitySlot);
		if(entity)
		{
			entity->parent(parentSlot);
			entity->pos(CVectorD::Null);
		}
		else
			nlwarning("command 'parent': there is no entity in the slot %d.", entitySlot);
	}
	break;

	// Bad command.
	default:
		return false;
	}

	// Well done.
	return true;
}

//-----------------------------------------------
// 'displaySentenceCounter' : display the sentence counter to compare with db counter
//-----------------------------------------------
NLMISC_COMMAND(displaySentenceCounter, "display the sentence counter to compare with db counter.", "")
{

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CBrickManager		*pBM= CBrickManager::getInstance();

	uint	srvVal= pIM->getDbProp("SERVER:BUILDING_SENTENCE:COUNTER")->getValue32();
	uint	locVal= pIM->getLocalSyncActionCounter() ;
	srvVal&= pIM->getLocalSyncActionCounterMask();
	locVal&= pIM->getLocalSyncActionCounterMask();

	pIM->displaySystemInfo(ucstring( "ServerCounter: " + toString(srvVal) + "/ LocalCounter: " + toString(locVal)) );

	// Well done.
	return true;
} // displaySentenceCounter //


#if defined(NL_OS_WINDOWS)
NLMISC_COMMAND (url, "launch a browser to the specified url", "<url>")
{
	if (args.size () != 1)
		return false;
	
	HINSTANCE result = ShellExecute(NULL, "open", args[0].c_str(), NULL,NULL, SW_SHOW);
	if ((sint32)result > 32)
		return true;
	else
	{
		log.displayNL ("ShellExecute failed %d", (uint32)result);
		return false;
	}
}
#endif

#if defined(NL_OS_WINDOWS)

NLMISC_COMMAND (igurl, "launch a *in game* browser to the specified url", "<url>")
{
	const char *webname[] = { "web.exe", "web_r.exe", "web_rd.exe", "web_df.exe", "web_d.exe" };

	if (args.size () != 1)
		return false;
	
	string wn;
	
	for (uint i = 0; i < sizeof(webname)/sizeof(webname[0]); i++)
	{
		if (CFile::fileExists (webname[i]))
		{
			wn = webname[i];
			break;
		}
	}

	if (wn.empty())
	{
		log.displayNL("web*.exe not found");
		return false;
	}

	uint32 x, y, w, h;
	Driver->getWindowPos(x, y);
	Driver->getWindowSize(w, h);
	
	x += w/2;
	y += h/2;
	
	w = (uint32)((float)w * 0.8f);
	h = (uint32)((float)h * 0.8f);

	string params;

	params = toString (x);
	params += " " + toString (y);
	params += " " + toString (w);
	params += " " + toString (h);
	params += " " + args[0];
	
	nlinfo ("launching web with : %s", params.c_str());

	launchProgram(wn, params);
	
	return true;
}
#endif


#endif	// _LIMITED_COMMAND_

#endif // FINAL_VERSION
