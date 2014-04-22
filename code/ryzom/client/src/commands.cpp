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




//////////////
// Includes //
//////////////

#include "stdpch.h"

// very nice \\// :)

#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/command.h"
#include "nel/misc/i18n.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/async_file_manager.h"

#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_track.h"

#include "nel/ligo/primitive.h"

#include "game_share/player_visual_properties.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/ryzom_version.h"
#include "game_share/brick_types.h"
#include "game_share/time_weather_season/time_and_season.h"

#include "entity_animation_manager.h"
#include "ingame_database_manager.h"
#include "world_database_manager.h"
#include "string_manager_client.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/people_interraction.h"
#include "client_chat_manager.h"
#include "continent_manager.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/group_compas.h"
#include "init_main_loop.h"
#include "sheet_manager.h"
#include "sound_manager.h"
#include "nel/gui/group_editbox.h"
#include "debug_client.h"
#include "user_entity.h"
#include "time_client.h"
#include "net_manager.h"
#include "pacs_client.h"
#include "continent.h"
#include "ig_client.h"
#include "commands.h"
#include "entities.h"
#include "teleport.h"
#include "nel/misc/cdb_leaf.h"
#include "view.h"
#include "misc.h"
#include "demo.h"
#include "dummy_progress.h"
#include "interface_v3/sphrase_manager.h"
#include "interface_v3/sbrick_manager.h"
#include "interface_v3/inventory_manager.h"
#include "interface_v3/action_handler_help.h"
#include "projectile_manager.h"
#include "fx_manager.h"
#include "actions_client.h"
#include "attack_list.h"
#include "interface_v3/player_trade.h"
#include "nel/gui/ctrl_base_button.h"
#include "weather.h"
#include "forage_source_cl.h"
#include "connection.h"
#include "nel/gui/lua_object.h"
#include "nel/gui/lua_ihm.h"
#include "interface_v3/lua_ihm_ryzom.h"
#include "init.h"
#include "interface_v3/people_interraction.h"
#include "far_tp.h"
#include "zone_util.h"
#include "nel/gui/lua_manager.h"


//
// Only the define FINAL_VERSION can be defined on the project, not in this file
// to desactive some commands
//



////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern CGenericXmlMsgHeaderManager	 GenericMsgHeaderMngr;
extern CEntityAnimationManager		*EAM;
extern CClientChatManager			 ChatMngr;
extern ULandscape					*Landscape;
extern UScene						*Scene;
extern CLog							 g_log;
extern CEntityManager				EntitiesMngr;

///////////////
// Variables //
///////////////

NLLIGO::CPrimitives *LDPrim = 0;
static std::vector<UInstance> ShapeAddedByCommand; // list of shapes added with the 'shape' command



///////////////
// FUNCTIONS //
///////////////

// Function to release all things allocated for commands.
void releaseCommands()
{
	if(LDPrim)
	{
		delete LDPrim;
		LDPrim = 0;
	}
}

//////////////
// COMMANDS //
//////////////

// connect to the support chat
NLMISC_COMMAND(supportChat, "connect to the external support chat", "")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->connectYuboChat();
	return true;
}

// 'follow' : To Follow the target.
NLMISC_COMMAND(follow, "Follow the target", "")
{
	// switch
	if(UserEntity->follow())
		UserEntity->disableFollow();
	else
		// enable follow, reseting the camera rotation
		UserEntity->enableFollow(true);
	return true;
}

NLMISC_COMMAND(where, "Ask information on the position", "")
{
	// Check parameters.
	if(args.size() == 0)
	{	// Create the message and send.
		const string msgName = "COMMAND:WHERE";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			NetMngr.push(out);
		}
		else
			nlwarning("command 'where': unknown message named '%s'", msgName.c_str());
		return true;
	}
	return false;
}

NLMISC_COMMAND(who, "Display all players currently in region","[<options (GM, channel name)>]")
{
	// Check parameters.
	if(args.size() > 1)
		return false;

	CBitMemStream out;
	if(!GenericMsgHeaderMngr.pushNameToStream("DEBUG:WHO", out))
	{
		nlwarning("Unknown message name DEBUG:WHO");
		return false;
	}

	string opt;
	if ( args.size() == 1 )
	{
		opt = args[0];
	}
	out.serial(opt);
	NetMngr.push(out);
	return true;
}

NLMISC_COMMAND(afk, "Set the player as 'away from keyboard'","[<custom text>]")
{
	string customText;
	if( args.size() > 0 )
	{
		customText = args[0];
	}
	for(uint i = 1; i < args.size(); ++i )
	{
		customText += " ";
		customText += args[i];
	}

	if (UserEntity != NULL)
		UserEntity->setAFK(true,customText);
/*
	CBitMemStream out;
	if(!GenericMsgHeaderMngr.pushNameToStream("DEBUG:AFK", out))
	{
		nlwarning("Unknown message name DEBUG:AFK");
		return false;
	}
	NetMngr.push(out);
*/
	return true;
}

bool randomCheckCharset(std::string const& str)
{
	std::string::const_iterator it, itEnd = str.end();
	for (it=str.begin(); it!=itEnd; ++it)
		if (*it<'0' || *it>'9')
			return false;
	return true;
}

// returns true if a<=b
bool randomLexicographicLess(std::string a, std::string b)
{
	// Remove leading zeros
	while (a.length()>1 && a[0]=='0')
		a = a.substr(1);
	while (b.length()>1 && b[0]=='0')
		b = b.substr(1);
	// Longest is the biggest
	if (a.length()>b.length())
		return false;
	if (a.length()<b.length())
		return true;
	// Skip equal characters
	size_t i = 0;
	while (i<a.length() && a[i]==b[i])
		++i;
	// If all characters are equal a==b
	if (i==a.length())
		return false;
	// Check highest weight different character
	return a[i] < b[i];
}

bool randomFromString(std::string const& str, sint16& val, sint16 min = -32768, sint16 max = 32767)
{
	bool negative = str[0]=='-';
	std::string sAbsVal = str.substr(negative?1:0);
	// Check we have only numerical characters
	if (!randomCheckCharset(sAbsVal))
		return false;
	// Check sign
	if (negative && min>0) return false;
	if (!negative && max<0) return false;
	// Check number is not too big nor too small with a lexicographic compare
	std::string smin = NLMISC::toString(std::max<sint16>(min,-min));
	std::string smax = NLMISC::toString(std::max<sint16>(max,-max));
	bool tooSmall = false, tooBig = false;
	if (min>=0 && randomLexicographicLess(sAbsVal, smin))
		tooSmall = true;
	if (min<0 && randomLexicographicLess(smin, sAbsVal))
		tooSmall = true;
	if (max>=0 && randomLexicographicLess(smax, sAbsVal))
		tooBig = true;
	if (max<0 && randomLexicographicLess(sAbsVal, smax))
		tooBig = true;
	if (!tooSmall && !tooBig)
	{
		NLMISC::fromString(str, val);
		return true;
	}
	else
		return false;
}

NLMISC_COMMAND(random, "Roll a dice and say the result around","[<min>] <max>")
{
	// Check parameters.
	if (args.size()<1 || args.size()>2)
		return false;

	sint16 min = 1;
	sint16 max;
	if (!randomFromString(args[0], max))
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("uiRandomBadParameter");
		strFindReplace(msg, "%s", args[0] );
		pIM->displaySystemInfo(msg);
		return false;
	}
	if (args.size()==2)
	{
		if (!randomFromString(args[1], min))
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			ucstring msg = CI18N::get("uiRandomBadParameter");
			strFindReplace(msg, "%s", args[0] );
			pIM->displaySystemInfo(msg);
			return false;
		}
	}
	if (min>max)
		std::swap(min, max);

	if (UserEntity != NULL)
		UserEntity->rollDice(min, max);

	return true;
}

//-----------------------------------------------
// 'dumpShapePos' : Dump Last Added Shape Pos
//-----------------------------------------------
NLMISC_COMMAND(dumpShapePos, "Dump Last Added Shape Pos.", "")
{
	#if FINAL_VERSION
	if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;
	#endif // FINAL_VERSION

	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}

	CInterfaceManager *IM = CInterfaceManager::getInstance();
	CVector pos = ShapeAddedByCommand.back().getPos();
	IM->displaySystemInfo(ucstring(toString("Shape Pos = %f, %f, %f", pos.x, pos.y, pos.z)));
	return true;
}
//-----------------------------------------------
// 'clearShape' : Remove all shapes added with the 'shape' command
//-----------------------------------------------
NLMISC_COMMAND(clearShape, "Remove all shapes added with the 'shape' command.", "")
{
	#if FINAL_VERSION
	/*if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION
	
	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}

	if (!Scene) return false;
	for(uint k = 0; k < ShapeAddedByCommand.size(); ++k)
	{
		Scene->deleteInstance(ShapeAddedByCommand[k]);
	}
	ShapeAddedByCommand.clear();
	return true;
}

//-----------------------------------------------------
// 'setShapeX' : Set X position for last created shape
//-----------------------------------------------------
NLMISC_COMMAND(setShapeX, "Set X position for last created shape.", "<x coordinate>")
{
	#if FINAL_VERSION
	/*if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION

	if (args.size() != 1) return false;
	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}
	float coord;
	bool valid_coord;
	if (args[0][0] == '+')
		valid_coord = fromString(args[0].substr(1), coord);
	else
		valid_coord = fromString(args[0], coord);

	if (!valid_coord)
	{
		nlwarning("Can't get position");
		return false;
	}
	CVector pos = ShapeAddedByCommand.back().getPos();
	if (args[0][0] == '+')
		pos.x += coord;
	else
		pos.x = coord;
	ShapeAddedByCommand.back().setPos(pos);
	return true;
}

//-----------------------------------------------------
// 'setShapeY' : Set Y position for last created shape
//-----------------------------------------------------
NLMISC_COMMAND(setShapeY, "Set Y position for last created shape.", "<y coordinate>")
{
	#if FINAL_VERSION
	/*if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION

	if (args.size() != 1) return false;
	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}
	float coord;
	bool valid_coord;
	if (args[0][0] == '+')
		valid_coord = fromString(args[0].substr(1), coord);
	else
		valid_coord = fromString(args[0], coord);

	if (!valid_coord)
	{
		nlwarning("Can't get position");
		return false;
	}
	CVector pos = ShapeAddedByCommand.back().getPos();
	if (args[0][0] == '+')
		pos.y += coord;
	else
		pos.y = coord;
	ShapeAddedByCommand.back().setPos(pos);
	return true;
}

//-----------------------------------------------------
// 'setShapeZ' : Set Z position for last created shape
//-----------------------------------------------------
NLMISC_COMMAND(setShapeZ, "Set Z position for last created shape.", "<z coordinate>")
{
	#if FINAL_VERSION
	/*if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION

	if (args.size() != 1) return false;
	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}
	float coord;
	bool valid_coord;
	if (args[0][0] == '+')
		valid_coord = fromString(args[0].substr(1), coord);
	else
		valid_coord = fromString(args[0], coord);

	if (!valid_coord)
	{
		nlwarning("Can't get position");
		return false;
	}
	CVector pos = ShapeAddedByCommand.back().getPos();
	if (args[0][0] == '+')
		pos.z += coord;
	else
		pos.z = coord;
	ShapeAddedByCommand.back().setPos(pos);
	return true;
}


//-----------------------------------------------------
// 'setShapeDir' : Set direction angle for last created shape
//-----------------------------------------------------
NLMISC_COMMAND(setShapeDir, "Set direction angle for last created shape.", "<angle>")
{
	#if FINAL_VERSION
	/*if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION

	if (args.size() != 1) return false;
	if (ShapeAddedByCommand.empty())
	{
		nlwarning("No shape created yet");
		return false;
	}
	float angle;
	if (!fromString(args[0], angle))
	{
		nlwarning("Can't get angle");
		return false;
	}

	CMatrix dir;
	dir.identity();
	CVector vangle = CVector(sin(angle), cos(angle), 0.f);
	CVector vi = vangle^CVector(0.f, 0.f, 1.f);
	CVector vk = vi^vangle;
	dir.setRot(vi, vangle, vk, true);
	// Set Orientation : User Direction should be normalized.
	ShapeAddedByCommand.back().setRotQuat(dir.getRot());

	return true;
}


//-----------------------------------------------
// 'shape' : Add a shape in the scene.
//-----------------------------------------------
NLMISC_COMMAND(shape, "Add a shape in the scene.", "<shape file>")
{
	#if FINAL_VERSION
/*	if (!hasPrivilegeDEV() &&
		!hasPrivilegeSGM() &&
		!hasPrivilegeGM() &&
		!hasPrivilegeVG() &&
		!hasPrivilegeSG() &&
		!hasPrivilegeG() &&
		!hasPrivilegeEM() &&
		!hasPrivilegeEG())
		return true;*/
	#endif // FINAL_VERSION

	if(args.size() < 1)
	{
		nlwarning("Command 'shape': need at least 1 parameter, try '/help shape' for more details.");
		return false;
	}
	if (!Scene)
	{
		nlwarning("No scene available");
		return false;
	}
	UInstance instance = Scene->createInstance(args[0]);
	if(!instance.empty())
	{
		ShapeAddedByCommand.push_back(instance);
		// Set the position
		instance.setPos(UserEntity->pos());
		instance.setClusterSystem(UserEntity->getClusterSystem()); // for simplicity, assume it is in the same
																   // cluster system than the user
		// Compute the direction Matrix
		CMatrix dir;
		dir.identity();
		CVector vi = UserEntity->dir()^CVector(0.f, 0.f, 1.f);
		CVector vk = vi^UserEntity->dir();
		dir.setRot(vi, UserEntity->dir(), vk, true);
		// Set Orientation : User Direction should be normalized.
		instance.setRotQuat(dir.getRot());
		// if the shape is a particle system, additionnal parameters are user params
		UParticleSystemInstance psi;
		psi.cast (instance);
		if (!psi.empty())
		{
			// set each user param that is present
			for(uint k = 0; k < 4; ++k)
			{
				if (args.size() >= (k + 2))
				{
					float uparam;
					if (fromString(args[k + 1], uparam))
					{
						psi.setUserParam(k, uparam);
					}
					else
					{
						nlwarning("Cant read param %d", k);
					}
				}
			}
		}
	}
	else
	{
		nlwarning("Command 'shape': cannot find the shape %s.", args[0].c_str());
	}

	// Command Well Done
	return true;
}

NLMISC_COMMAND(bugReport, "Call the bug report tool with dump", "<AddScreenshot>")
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
		uint8 quality;
		fromString(args[0], quality);
		if (quality == 0)
			quality = 80;

		CBitmap btm;
		Driver->getBuffer(btm);
		string filename = CFile::findNewFile (getLogDirectory() + "screenshot.jpg");
		COFile fs(filename);
		btm.writeJPG(fs, quality);
		sys += "AttachedFile "+filename+" ";
	}

	sys += "ClientVersion "RYZOM_VERSION" ";

	// for now, set the same version than client one
	sys += "ShardVersion "RYZOM_VERSION" ";

	if (ClientCfg.Local)
		sys += "ShardName OFFLINE ";

	FILE *fp = fopen (std::string(getLogDirectory() + "bug_report.txt").c_str(), "wb");
	if (fp != NULL)
	{
		string res = addSlashR(getDebugInformation());

		// must put \r\n each line
		fprintf(fp, "%s", res.c_str());

//		// must put \r\n each line
//		fprintf (fp, "UserId: %u\r\n", NetMngr.getUserId());
//		fprintf (fp, "Player Name: '%s'.\r\n", UserEntity->getName().toString().c_str());
//		fprintf (fp, "UserPosition: %.2f %.2f %.2f\r\n", UserEntity->pos().x, UserEntity->pos().y, UserEntity->pos().z);
//		fprintf (fp, "ViewPosition: %.2f %.2f %.2f\r\n", View.viewPos().x,   View.viewPos().y,   View.viewPos().z);
//		time_t ts; time( &ts );
//		fprintf (fp, "LocalTime: %s\r\n", NLMISC::IDisplayer::dateToHumanString( ts ) );
//		fprintf (fp, "ServerTick: %u\r\n", NetMngr.getCurrentServerTick());
//		fprintf (fp, "ConnectState: %s\r\n", NetMngr.getConnectionStateCStr());
//		fprintf (fp, "LocalAddress: %s\r\n", NetMngr.getAddress().asString().c_str());

		fclose (fp);

		sys += "DumpFilename bug_report.txt ";
	}

	nlinfo ("Calling for bug report : '%s %s'", brn.c_str(), sys.c_str());

	launchProgram(brn, sys);

	// give some cpu to the launched application
	nlSleep (3000);

	return true;
}

//
//
// This command is use to do all admin execution commands on you
//
// For example: "/a God 1" will set you in god mode
//

NLMISC_COMMAND(a, "Execute an admin command on you","<cmd> <arg>")
{
	if(args.size() == 0)
		return false;

	CBitMemStream out;
	if (!GenericMsgHeaderMngr.pushNameToStream("COMMAND:ADMIN", out))
		return false;

	string cmd, arg;
	cmd = args[0];
	for (uint i = 1; i < args.size(); i++)
	{
		// temporary fix for utf-8
		// servers commands are not decoded so convert them to ansi
		std::string tmp = ucstring::makeFromUtf8(args[i]).toString();

		if (!arg.empty())
			arg += ' ';
		if (tmp.find(' ') != std::string::npos)
		{
			arg += "\"" + tmp + "\"";
		}
		else
		{
			arg += tmp;
		}
	}
	bool onTarget = false;
	out.serial (onTarget);
	out.serial (cmd);
	out.serial (arg);
	NetMngr.push (out);

	return true;
}

//
//
// This command is use to do all admin execution commands on the target
//
// For example: "/b God 1" will set the target in god mod
//

NLMISC_COMMAND(b, "Execute an admin command on your target","<cmd> <arg>")
{
	if(args.size() == 0)
		return false;

	CBitMemStream out;
	if (!GenericMsgHeaderMngr.pushNameToStream("COMMAND:ADMIN", out))
		return false;

	string cmd, arg;
	cmd = args[0];
	for (uint i = 1; i < args.size(); i++)
	{
		// temporary fix for utf-8
		// servers commands are not decoded so convert them to ansi
		std::string tmp = ucstring::makeFromUtf8(args[i]).toString();

		if (!arg.empty())
			arg += ' ';
		if (tmp.find(' ') != std::string::npos)
		{
			arg += "\"" + tmp + "\"";
		}
		else
		{
			arg += tmp;
		}
	}
	bool onTarget = true;
	out.serial (onTarget);
	out.serial (cmd);
	out.serial (arg);
	NetMngr.push (out);

	return true;
}

//
//
// This command is used to do all admin execution commands on a character
//
// For example: "/c charName God 1" will set god mod on character if it's online, or keep
// command for wait character login
//

NLMISC_COMMAND(c, "Execute an admin command on character name","<Character Name> <cmd> <arg>")
{
	if(args.size() < 2)
		return false;

	CBitMemStream out;
	if (!GenericMsgHeaderMngr.pushNameToStream("COMMAND:ADMIN_OFFLINE", out))
		return false;

	string characterName, cmd, arg;

	characterName = args[0];
	cmd = args[1];
	for (uint i = 2; i < args.size(); i++)
	{
		// temporary fix for utf-8
		// servers commands are not decoded so convert them to ansi
		std::string tmp = ucstring::makeFromUtf8(args[i]).toString();

		if (!arg.empty())
			arg += ' ';
		if (tmp.find(' ') != std::string::npos)
		{
			arg += "\"" + tmp + "\"";
		}
		else
		{
			arg += tmp;
		}
	}
	out.serial (characterName);
	out.serial (cmd);
	out.serial (arg);
	NetMngr.push (out);

	return true;
}

NLMISC_COMMAND(boxes, "Show/Hide selection boxes", "[<state> : 0 to Hide, anything else to Show. Invert the current state if nothing specified.]")
{
#if FINAL_VERSION
	if (!ClientCfg.ExtendedCommands) return false;

	if( !ClientCfg.Local && !hasPrivilegeDEV() && !hasPrivilegeSGM() && !hasPrivilegeGM() )
		return true;
#endif // FINAL_VERSION

	// Invert Current State
	if(args.size() == 0)
	{
		// Invert the current value.
		ClientCfg.DrawBoxes = !ClientCfg.DrawBoxes;
		return true;
	}
	// Set Current State
	else if(args.size() == 1)
	{
		// Invert the current value.
		fromString(args[0], ClientCfg.DrawBoxes);
		return true;
	}
	// Bad parameters.
	else
		return false;
}

NLMISC_COMMAND(dump, "Command to create a file with the current state of the client", "[<dump name>]")
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
}

NLMISC_COMMAND(verbose, "Enable/Disable some Debug Information", "none or magic")
{
	// Check parameters.
	if(args.size() != 1)
	{
		// Help
		CInterfaceManager *IM = CInterfaceManager::getInstance();
		IM->displaySystemInfo(ucstring("This command need 1 parameter :"));
		IM->displaySystemInfo(ucstring("<string> :"));
		IM->displaySystemInfo(ucstring("- none(to remove all verboses)"));
		IM->displaySystemInfo(ucstring("- magic(to add debug infos about magic)"));
		IM->displaySystemInfo(ucstring("- anim (to add debug infos about animation)"));
	}
	else
	{
		std::string type = NLMISC::strlwr(args[0]);
		if     (type == "none")
			Verbose = VerboseNone;
		else if(type == "magic")
			Verbose |= VerboseMagic;
		else if(type == "anim")
			Verbose |= VerboseAnim;
		else
		{
			CInterfaceManager *IM = CInterfaceManager::getInstance();
			IM->displaySystemInfo(ucstring("This command need 1 parameter :"));
			IM->displaySystemInfo(ucstring("<string> :"));
			IM->displaySystemInfo(ucstring("- none(to remove all verboses)"));
			IM->displaySystemInfo(ucstring("- magic(to add debug infos about magic)"));
			IM->displaySystemInfo(ucstring("- anim (to add debug infos about animation)"));
		}
	}
	return true;
}

NLMISC_COMMAND(verboseAnimSelection, "Enable/Disable the animation log for the current selection", "")
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
}

NLMISC_COMMAND(verboseAnimUser, "Enable/Disable the animation log for the user", "")
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
}

NLMISC_COMMAND(verboseDatabase, "Enable/Disable the log for the database", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	bool v = NLMISC::ICDBNode::isDatabaseVerbose();
	NLMISC::ICDBNode::setVerboseDatabase( !v );

	if( !v )
		nlinfo("Enable VerboseDatabase");
	else
		nlinfo("Disable VerboseDatabase");

	return true;
}

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

NLMISC_COMMAND(logEntities, "Write the position and orientation af all entities in the vision in the file 'entities.txt'", "")
{
	// Check parameters
	if(args.size() != 0)
		return false;

	// Log entities
	EntitiesMngr.writeEntities();

	// Command well done.
	return true;
}

NLMISC_COMMAND(log, "Add/Del Positive/Negative Filters for logs", "Log System <debug, info, warning, assert>, Type <pos/neg/del/reset>, Filter <string>")
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

NLMISC_COMMAND(execScript, "Execute a script file (.cmd)","<FileName>")
{
	int size = (int)args.size();
	if (size != 1)
		return false;

	CIFile iFile;

	if (iFile.open(CPath::lookup(args[0], false)))
	{
		char line[512];
		char *buffer;
		// Read line by line and execute each line

		sint inComment= 0;
		bool eof = false;
		while (!eof)
		{
			buffer = &line[0];
			uint read = 0;
			for(;;)
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
				catch (const EFile &)
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

			// execute line
			if (strlen(line) > 0)
			{
				// if not a single comment
				if(strncmp(line, "//", 2)!=0)
				{
					if(strncmp(line, "/*", 2)==0)
						inComment++;
					if(inComment<=0)
					{
						ucstring ucline(line);
						CInterfaceManager::parseTokens(ucline);
						ICommand::execute(ucline.toUtf8(), g_log);
					}
					if(strncmp(line, "*/", 2)==0)
						inComment--;
				}
			}

			// end?
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
}


NLMISC_COMMAND(db, "Modify Database","<Property> <Value>")
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	int size = (int)args.size();
 	if (size == 2)
	{
#if !FINAL_VERSION
		// check if 2nd arg is a sheet name
		if (args[1].empty()) return false;
		sint64 value;
		if (isalpha(args[1][0]))
		{
			CSheetId sheet(args[1]);
			value = (sint64) sheet.asInt();
		}
		else
		{
			// Convert the string into an sint64.
			fromString(args[1], value);
		}

		// Set the property.
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(args[0], false);
		if(node)
			node->setValue64(value);
		else
			pIM->displaySystemInfo(toString("DB '%s' does not exist.", args[0].c_str()));
#else
		pIM->displaySystemInfo(ucstring("Can't write to DB when in Final Version."));
#endif
	}
	else if (size == 1)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(args[0], false);
		if(node)
		{
			sint64 prop = node->getValue64();
			string str = toString(prop);
			pIM->displaySystemInfo(ucstring(str));
			nlinfo("%s", str.c_str());
		}
		else
			pIM->displaySystemInfo(toString("DB '%s' does not exist.", args[0].c_str()));
		return true;
	}
	else
		return false;

	return true;
}

static bool talkInChan(uint32 nb,std::vector<std::string>args)
{
	uint32 maxChans = CChatGroup::MaxDynChanPerPlayer;
	if (nb>=maxChans)
	{
		return false;
	}
	if(args.size()>0)
	{
		std::string tmp="";
		std::vector<std::string>::const_iterator first(args.begin()),last(args.end());

		for(;first!=last;++first)
		{
			tmp = tmp + (*first);
			tmp = tmp+" ";
		}

		ucstring uctmp;
		uctmp.fromUtf8(tmp);
		PeopleInterraction.talkInDynamicChannel(nb, uctmp);
		return true;
	}
	else
	{
		ChatMngr.updateChatModeAndButton(CChatGroup::dyn_chat, nb);
	}
	return false;
}

NLMISC_COMMAND(0,"talk in 0th dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(0,args);
}

NLMISC_COMMAND(1,"talk in first dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(1,args);
}

NLMISC_COMMAND(2,"talk in 2nd dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(2,args);
}

NLMISC_COMMAND(3,"talk in 3rd dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(3,args);
}

NLMISC_COMMAND(4,"talk in 4th dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(4,args);
}

NLMISC_COMMAND(5,"talk in 5th dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(5,args);
}

NLMISC_COMMAND(6,"talk in 6th dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(6,args);
}

NLMISC_COMMAND(7,"talk in 7th dynamic chat channel","<channel_nb> <sentence>")
{
	return talkInChan(7,args);
}


NLMISC_COMMAND(setItemName, "set name of items, sbrick, etc..","<sheet_id> <name> <desc> <desc2>")
{
	if (args.size() < 2) return false;
	CSheetId id(args[0]);
	ucstring name;
	name.fromUtf8(args[1]);
	ucstring desc;
	ucstring desc2;
	if (args.size() > 2)
		desc.fromUtf8(args[2]);
	if (args.size() > 2)
		desc2.fromUtf8(args[3]);

	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	if (pSMC)
		pSMC->replaceSBrickName(id, name, desc, desc2);
	else
		return false;
	return true;
}


NLMISC_COMMAND(setMissingDynstringText, "set text of missing dynamic string"," <name> <text>")
{
	if (args.size() < 2) return false;
	ucstring name;
	name.fromUtf8(args[0]);
	ucstring text;
	text.fromUtf8(args[1]);

	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	if (pSMC)
		pSMC->replaceDynString(name, text);
	else
		return false;
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

NLMISC_COMMAND(ah, "Launch an action handler", "<ActionHandler> <AHparam>")
{
	if (args.size() == 0)
		return false;

	if (!ClientCfg.AllowDebugLua && strlwr(args[0]) == "lua")
	{
		return false; // not allowed!!
	}

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (args.size() == 1)
	{
		CAHManager::getInstance()->runActionHandler(args[0], NULL);
	}
	else
	{
		CAHManager::getInstance()->runActionHandler(args[0], NULL, args[1]);
	}

	return true;
}

static void setDynString(uint32 strID, const std::string &value)
{
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	pSMC->receiveString(strID, ucstring(value));
	CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	bm.serial(strID);
	bm.serial(strID);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	pSMC->receiveDynString(bm);
}

// for debug purposes, insert a string in the
NLMISC_COMMAND(setDynString, "set a dynamic string","<stringID> <asciiValue>")
{
	if (args.size() != 2) return false;
	uint32 strID;
	fromString(args[0], strID);
	setDynString(strID, args[1]);
	return true;
}

class CAnimProgress : public IProgressCallback
{
public:
	// Update the progress bar
	virtual void progress (float value)
	{
		// can't do anything if no driver
		if(Driver == NULL)
			return;
		// Get croped value
		value = getCropedValue (value);
		// Set matrix
		Driver->setMatrixMode2D11();
		// Display a progress bar background
		Driver->drawQuad (PROGRESS_BAR_LEFT, 0.5f-PROGRESS_BAR_HEIGHT/2.0f, PROGRESS_BAR_LEFT+      PROGRESS_BAR_WIDTH, 0.5f+PROGRESS_BAR_HEIGHT/2.0f,
			PROGRESS_BAR_BG_COLOR);
		// Display a progress bar
		Driver->drawQuad (PROGRESS_BAR_LEFT, 0.5f-PROGRESS_BAR_HEIGHT/2.0f, PROGRESS_BAR_LEFT+value*PROGRESS_BAR_WIDTH, 0.5f+PROGRESS_BAR_HEIGHT/2.0f,
			PROGRESS_BAR_COLOR);
		if(TextContext != NULL)
		{
			// Init the Pen.
			TextContext->setKeep800x600Ratio(false);
			TextContext->setColor(CRGBA(255,255,255));
			TextContext->setFontSize(20);
			TextContext->setHotSpot(UTextContext::MiddleMiddle);

			// Display the Text.
			TextContext->printfAt(0.5f, 0.5f, _ProgressMessage.c_str());
		}
		// Display to screen.
		Driver->swapBuffers();
	}
	// New message
	void newMessage(const std::string &message) {_ProgressMessage = message;}

private:
	std::string _ProgressMessage;
};

NLMISC_COMMAND(reloadSearchPaths, "reload the search paths","")
{
	if (!args.empty()) return false;
	CPath::memoryUncompress();
	CAnimProgress progress;
	// remove all objects that may depend on an animation
	CProjectileManager::getInstance().reset();

	// Pathes
	progress.newMessage("Reloading pathes");
	progress.progress(0.0f);
	progress.pushCropedValues(0.0f, 1.0f);
	//

	addSearchPaths(progress);
	CPath::memoryCompress();
	return true;
}

NLMISC_COMMAND(reloadAnim, "reload animations","")
{
	CPath::memoryUncompress();
	CAnimProgress dummy;
	// remove all objects that may depend on an animation
	CProjectileManager::getInstance().reset();

	// Pathes
	dummy.newMessage("Pathes");
	dummy.progress(0.0f);
	dummy.pushCropedValues(0.0f, 0.5f);


	addSearchPaths(dummy);

	if (ClientCfg.UpdatePackedSheet)
	{
		for(uint i = 0; i < ClientCfg.UpdatePackedSheetPath.size(); i++)
		{
			dummy.progress((float)i/(float)ClientCfg.UpdatePackedSheetPath.size());
			dummy.pushCropedValues ((float)i/(float)ClientCfg.UpdatePackedSheetPath.size(), (float)(i+1)/(float)ClientCfg.UpdatePackedSheetPath.size());
			CPath::addSearchPath(ClientCfg.UpdatePackedSheetPath[i], true, false, &dummy);
			dummy.popCropedValues();
		}
	}


	dummy.popCropedValues();
	// Animations
	dummy.newMessage("Anims");
	dummy.progress(0.5f);
	dummy.pushCropedValues(0.5f, 1.0f);
	EAM->load(dummy, true);
	dummy.popCropedValues();
	// Reload Animations
	EntitiesMngr.reloadAnims();
	CPath::memoryCompress();
	return true;
}

NLMISC_COMMAND(reloadAttack, "reload attack", "")
{
	if (!args.empty()) return false;
	// remove all objects that may depend on an animation
	ClientSheetsStrings.memoryUncompress();
	CProjectileManager::getInstance().reset();
	EntitiesMngr.removeAllAttachedFX();
	FXMngr.reset();
	//
	std::vector<std::string> exts;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// replace attack list of creature in place (so pointers on character sheets remains valid)
	CSheetManager sheetManager;
	exts;
	// exts.push_back("creature");
	exts.push_back("race_stats");
	NLMISC::IProgressCallback progress;
	sheetManager.loadAllSheet(progress, true, false, false, true, &exts);
	//
	const CSheetManager::TEntitySheetMap &sm = SheetMngr.getSheets();
	for(CSheetManager::TEntitySheetMap::const_iterator it = sm.begin(); it != sm.end(); ++it)
	{
		if (it->second.EntitySheet && it->second.EntitySheet->Type == CEntitySheet::FAUNA)
		{
			// find matching sheet in new sheetManager
			const CEntitySheet *other = sheetManager.get(it->first);
			if (other)
			{
				// replace data in place
				((CCharacterSheet &) *it->second.EntitySheet).AttackLists = ((const CCharacterSheet &) *other).AttackLists;
			}
		}
		else if(it->second.EntitySheet && it->second.EntitySheet->Type == CEntitySheet::RACE_STATS)
		{
			// find matching sheet in new sheetManager
			const CEntitySheet *other = sheetManager.get(it->first);
			if (other)
			{
				// replace data in place
				((CRaceStatsSheet &) *it->second.EntitySheet).AttackLists = ((const CRaceStatsSheet &) *other).AttackLists;
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CAttackListManager::getInstance().release();
	// form to reload all sheets of interest
	exts.clear();
	exts.push_back("attack_list");
	exts.push_back("animation_fx_set");
	exts.push_back("id_to_string_array");
	CDummyProgress dp;
	SheetMngr.loadAllSheet(dp, true, false, true, true, &exts);
	CAttackListManager::getInstance().init();
	//
	ClientSheetsStrings.memoryCompress();
	return true;
}

NLMISC_COMMAND(reloadSky, "reload new style sky", "")
{
	if (!args.empty()) return false;
	ContinentMngr.reloadSky();
	return false;
}


NLMISC_COMMAND(missionReward, "debug"," ")
{
	if (args.size() == 1)
	{
		uint8 index;
		fromString(args[0], index);
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
		uint8 index;
		fromString(args[0], index);
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

/*
NLMISC_COMMAND(save_sentences, "save sentences"," ")
{
	CSentenceDisplayer::saveSentences();
	return true;
}
*/

NLMISC_COMMAND(getSheetId, "get_sheet_id","<sheet file name>")
{
	if (args.size() != 1)
		return false;
	CSheetId id(args[0]);

	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(toString(id.asInt())));
	return true;
}

NLMISC_COMMAND(getSheetName, "get_sheet_name","<Sheet Id>")
{
	if (args.size() != 1)
		return false;
	uint32 nId;
	fromString(args[0], nId);
	CSheetId id( nId );

	string name = id.toString();


	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(name));
	return true;
}

NLMISC_COMMAND(forgetAll, "forget all bricks", "")
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
		CCDBNodeLeaf * node= NLGUI::CDBManager::getInstance()->getDbProp(buf);
		node->setValue64(0);
	}
	return true;
}

NLMISC_COMMAND(usePreprogMagic, "use the specified magic preprog sentence", "<sentence id>")
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
		uint8 phrase;
		fromString(args[0], phrase);
		out.serial(phrase);

		BRICK_TYPE::EBrickType type = BRICK_TYPE::MAGIC;
		out.serialEnum( type );

		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(usePreprogCombat, "use the specified combat preprog sentence", "<sentence id>")
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
		uint8 phrase;
		fromString(args[0], phrase);
		out.serial(phrase);

		BRICK_TYPE::EBrickType type = BRICK_TYPE::COMBAT;
		out.serialEnum( type );

		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

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
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(defaultAttack, "use default attack on target", "")
{
	// Default attack on the current selection.
	UserEntity->attack();

	// Well Done.
	return true;
}

NLMISC_COMMAND(disengage, "disengage from combat", "")
{
	// Disengage from combat.
	UserEntity->disengage();

	// Well Done.
	return true;
}

NLMISC_COMMAND(leaveTeam, "leave team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:LEAVE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(joinTeam, "join the specified team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(joinTeamProposal, "propose to current target to join the team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN_PROPOSAL";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(joinTeamDecline, "decline a join team proposal", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:JOIN_PROPOSAL_DECLINE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(kickTeammate, "kick someone from your team", "")
{
	// Create the message for the server to execute a phrase.
	const string msgName = "TEAM:KICK";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("mainLoop : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(cancelCurrentSentence, "cancel the sentence being executed", "")
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
		nlwarning("command : unknown message name : '%s'", msgName.c_str());

	return true;
}

NLMISC_COMMAND(cancelAllPhrases, "cancel all the phrases being executed", "")
{
	// no parameter needed

	UserEntity->cancelAllPhrases();

	return true;
}


/*
NLMISC_COMMAND(drop,"drop an item to the ground","<id>")
{
	if( args.size() < 1 )
	{
		return false;
	}

	uint32 id;
	fromString(args[0], id);
	CEntityId itemId(RYZOMID::object,id);

	sint32 x = (sint32)UserEntity->pos().x * 1000;
	sint32 y = (sint32)UserEntity->pos().y * 1000;
	sint32 z = (sint32)UserEntity->pos().z * 1000;

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
	}*

	return true;
}
*/



NLMISC_COMMAND(pos, "Change the position of the user (in local only)", "<x, y, (z)> OR 1 name of 'tp.teleport_list'. or a bot name")
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
				nlwarning("/pos: unknown message name : 'TP:BOT'");
			return true;
		}
	}
	// Teleport to anywhere.
	else if(args.size() == 2 || args.size() == 3)
	{
		fromString(args[0], newPos.x);
		fromString(args[1], newPos.y);
		if(args.size() == 3)
			fromString(args[2], newPos.z);
		else
			newPos.z = 0.0;
	}
	// Bad argument number.
	else
		return false;

	/*
		CANNOT USE ProgressBar here, because it does pumpEvents(), and
		ICommand::execute() is typically called from a pumpEvents() too...
		=> cause crash
	*/

	// Fade out the Game Sound
	if(SoundMngr)
		SoundMngr->fadeOutGameSound(ClientCfg.SoundTPFade);

	// Remove the selection.
	UserEntity->selection(CLFECOMMON::INVALID_SLOT);
	// Remove the target.
	UserEntity->targetSlot(CLFECOMMON::INVALID_SLOT);
	// Change the position of the entity and in Pacs.
	UserEntity->pos(newPos);
	// Select the closest continent from the new position.
	CDummyProgress	progress;
	ContinentMngr.select(newPos, progress);
	// Teleport the User.
	UserEntity->tp(newPos);

	// First frame (for sound fade in)
	extern bool	FirstFrame;
	FirstFrame = true;

	return true;
}

NLMISC_COMMAND(removeEntity, "Remove an entity", "<Slot>")
{
	if (args.size() != 1) return false;
	uint slot;
	fromString(args[0], slot);
	EntitiesMngr.remove(slot, true);
	return true;
}


NLMISC_COMMAND(entity, "Create an entity on the user or just remove it if Form not valid", "<Slot> <Form> [posx posy posz] [relativeToPlayer]")
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	// Check parameters.
	if(args.size() != 2 && args.size() != 5 && args.size() != 6)
		return false;

	// read pos.
	CVector		entityPos;
	if(args.size()>=5)
	{
		fromString(args[2], entityPos.x);
		fromString(args[3], entityPos.y);
		fromString(args[4], entityPos.z);
		// if want pos local to UserEntity
		if(args.size()==6)
		{
			sint32 tmp;
			fromString(args[5], tmp);
			if (tmp != 0)
			{
				CMatrix	mat;
				mat.setRot(CVector::I, UserEntity->front(), CVector::K);
				mat.normalize(CMatrix::YZX);
				mat.setPos(UserEntity->pos());
				entityPos= mat * entityPos;
			}
		}
	}
	else
	{
		entityPos= UserEntity->pos()+UserEntity->front()*2.0;
	}

	// Try to create the sheet with the parameter as a string.
	CSheetId sheetId;
	if(!sheetId.buildSheetId(args[1]))
	{
		// Try to create the sheet with the parameter as an int.
		uint32 nSheetId;
		fromString(args[1], nSheetId);
		sheetId = CSheetId(nSheetId);
		if(sheetId == CSheetId::Unknown)
		{
			nlwarning("Command 'entity': '%s' is not a valid form", args[1].c_str());
			return false;
		}
	}

	// The slot where the new entity will be.
	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);

	// Debug Infos
	nldebug("Command 'entity' : AddNewEntity with form %s in the slot %d", args[1].c_str(), slot);
	// Remove the old entity.
	EntitiesMngr.remove(slot, false);
	// Create the new entity.

	TNewEntityInfo emptyEntityInfo;
	emptyEntityInfo.reset();
	CEntityCL *entity = EntitiesMngr.create(slot, sheetId.asInt(), emptyEntityInfo);
	if(entity)
	{
		sint64       *prop = 0;
		CCDBNodeLeaf *node = 0;
		// Set The property 'CLFECOMMON::PROPERTY_POSITION'.
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_POSX), false);
		if(node)
		{
			sint64 x = (sint64)(entityPos.x*1000.0);
			sint64 y = (sint64)(entityPos.y*1000.0);
			sint64 z = (sint64)(entityPos.z*1000.0);
			node->setValue64(x);
			node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_POSY), false);
			if(node)
			{
				node->setValue64(y);
				node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_POSZ), false);
				if(node)
					node->setValue64(z);
			}
		}
		// Set The property 'PROPERTY_ORIENTATION'.
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_ORIENTATION), false);
		if(node)
		{
			float dir = (float)atan2(UserEntity->front().y, UserEntity->front().x);
			prop = (sint64 *)(&dir);
			node->setValue64(*prop);
		}
		// Set Mode
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_MODE), false);
		if(node)
		{
			MBEHAV::EMode m = MBEHAV::NORMAL;
			prop = (sint64 *)&m;
			node->setValue64(*prop);
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_MODE);
		}
		// Set Visual Properties
		if(dynamic_cast<CPlayerCL *>(entity))
		{
			SPropVisualA visualA;
			visualA.PropertySubData.Sex = ClientCfg.Sex;
			SPropVisualB visualB;
			// Initialize the Visual Property C (Default parameters).
			SPropVisualC visualC;
			visualC.PropertySubData.CharacterHeight = 7;
			visualC.PropertySubData.ArmsWidth       = 7;
			visualC.PropertySubData.LegsWidth       = 7;
			visualC.PropertySubData.TorsoWidth      = 7;
			visualC.PropertySubData.BreastSize      = 7;
			// Set The Database
			prop = (sint64 *)&visualB;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*prop);
			prop = (sint64 *)&visualC;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPC))->setValue64(*prop);
			prop = (sint64 *)&visualA;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);
			// Apply Changes.
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
		}
		// Forage Source special
		if(dynamic_cast<CForageSourceCL*>(entity))
		{
			sint64	barVal;
			barVal= 32; barVal<<= 7;
			barVal+= 32; barVal<<= 7;
			barVal+= 10; barVal<<= 7;
			barVal+= 127;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_BARS))->setValue64(barVal);
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_BARS);
			// must also update position, else don't work
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_POSITION);
		}
		else
		if (dynamic_cast<CFxCL*>(entity)) // FX cl special
		{
			// must also update position, else don't work
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_POSITION);
		}
		//
		nlinfo("entity: slot: %d \"%s\", \"%f\", \"%f\", \"%f\" ",
			slot,args[1].c_str(),
			entityPos.x, entityPos.y, entityPos.z);
	}
	else
		nldebug("command 'entity' : entity in slot %d removed", slot);

	// Command well done.
	return true;
}

NLMISC_COMMAND(primPoint, "add a primitive point", "<pointName>")
{
	if(args.size() != 2)
		return false;

	if(LDPrim == 0)
	{
		LDPrim = new NLLIGO::CPrimitives;
		if(LDPrim == 0)
		{
			nlwarning("primPoint: LDPrim == 0");
			return false;
		}
	}

	if(LDPrim->RootNode == 0)
	{
		nlwarning("primPoint: LDPrim.RootNode == 0");
		return true;
	}

	NLLIGO::CPropertyString	*str	= 0;
	NLLIGO::CPrimPoint		*point	= 0;

	point = new NLLIGO::CPrimPoint;
	if(point == 0)
	{
		nlwarning("primPoint: point == 0");
		return true;
	}
	point->Point.x = (float)UserEntity->pos().x;
	point->Point.y = (float)UserEntity->pos().y;
	point->Point.z = (float)UserEntity->pos().z;

	str = new NLLIGO::CPropertyString;
	if(str == 0)
	{
		nlwarning("primPoint: str == 0 (1)");
		return true;
	}
	point->addPropertyByName("class", str);
	str->String = "reference_point";

	str = new NLLIGO::CPropertyString;
	if(str == 0)
	{
		nlwarning("primPoint: str == 0 (2)");
		return true;
	}
	point->addPropertyByName("name", str);
	str->String = args[1];

	// Add the point to the primitive.
	LDPrim->RootNode->insertChild(point);

	// Open the file.
	NLMISC::COFile file;
	if(file.open(args[0]))
	{
		// Create the XML stream
		NLMISC::COXml output;
		// Init
		if(output.init(&file, "1.0"))
		{
			LDPrim->write(output.getDocument(), args[0].c_str());
			// Flush the stream, write all the output file
			output.flush();
		}
		else
			nlwarning("primPoint: ");
		// Close the File.
		file.close();
	}
	else
		nlwarning("primPoint: cannot open/create the file '%s'", args[0].c_str());

	return true;
}

#ifdef ENABLE_INCOMING_MSG_RECORDER

NLMISC_COMMAND(record, "Start Recording", "<name>")
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

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
}

NLMISC_COMMAND(replay, "replay", "<name>")
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
}

NLMISC_COMMAND(stopRecord, "Stop Recording", "")
{
	// Check parameters.
	if(args.size() != 0)
		return false;

	// On/Off record.
	if(!ClientCfg.Local)
		NetMngr.setRecordingMode(false);
	return true;
}

#endif	// ENABLE_INCOMING_MSG_RECORDER

NLMISC_COMMAND(loadDump, "Command to load a dump file", "[<dump name>]")
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
}

NLMISC_COMMAND(sheet2idx, "Return the index of a sheet", "<sheet name> <visual slot number>")
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	if(args.size() != 2)
		return false;

	string result;
	NLMISC::CSheetId sheetId;

	if(sheetId.buildSheetId(args[0]))
	{
		uint slot;
		fromString(args[1], slot);
		uint32 idx = CVisualSlotManager::getInstance()->sheet2Index(sheetId, (SLOTTYPE::EVisualSlot)slot);
		result = NLMISC::toString("Index = %d", idx);
	}
	else
		result = NLMISC::toString("sheet '%s' not valid", args[0].c_str());

	IM->displaySystemInfo(ucstring(result));
	nlinfo("'sheet2idx': %s", result.c_str());
	return true;
}

NLMISC_COMMAND(watchEntity, "Choose the entity to watch", "<slot>")
{
	if(args.size() != 1)
		return false;

	// Set the new debug entity slot.
	fromString(args[0], WatchedEntitySlot);
	return true;
}

NLMISC_COMMAND(dynstr, "display a dyn string value", "<dyn string_id>")
{
	if (args.size() != 1)
		return false;

	uint dynId;
	fromString(args[0], dynId);

	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getDynString(dynId, result);

	CInterfaceManager::getInstance()->displaySystemInfo(result);
	return true;
}

NLMISC_COMMAND(serverstr, "display a server string value", "<serverstr string_id>")
{
	if (args.size() != 1)
		return false;

	uint dynId;
	fromString(args[0], dynId);

	ucstring result;
	STRING_MANAGER::CStringManagerClient::instance()->getString(dynId, result);

	CInterfaceManager::getInstance()->displaySystemInfo(result);
	return true;
}

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
}

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
}

NLMISC_COMMAND(askservices, "Ask the server all services up", "")
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:SERVICES", out))
	{
		// Add the message to the send list.
		NetMngr.push(out);
		nlinfo("command 'services': 'DEBUG:SERVICES' sent");
	}
	else
		nlwarning("command 'services': unknown message named 'DEBUG:SERVICES'");

	return true;
}

NLMISC_COMMAND(mode, "Change the mode for an entity in a slot", "<Slot> <Mode> [dt(tick)]")
{
	// Check parameters.
	if(args.size() < 2)
	{
		// Help
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 2 paramters :"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Slot> : the slot number of the entity to change"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Mode> : the mode wanted for the entity, one of the following number :"));
		for(uint i = 0; i<MBEHAV::NUMBER_OF_MODES; ++i)
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d - %s", i, MBEHAV::modeToString((MBEHAV::EMode)i).c_str())));
	}
	// Right parameters number
	else
	{
		// Compute parameters
		CLFECOMMON::TCLEntityId slot;
		fromString(args[0], slot);
		MBEHAV::EMode mod				= MBEHAV::stringToMode(args[1]);
		if(mod==MBEHAV::UNKNOWN_MODE)
		{
			sint32 nMode;
			fromString(args[1], nMode);
			mod = (MBEHAV::EMode)nMode;
		}

		// Compute the position.
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_MODE), mod);
			// Update the behaviour.
			sint32	dt= 10;
			if(args.size() > 2)
				fromString(args[2], dt);
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+dt, CLFECOMMON::PROPERTY_MODE);
		}
		// Invalid slot.
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot"));
	}

	// Command well done.
	return true;
}

NLMISC_COMMAND(behaviour, "Change the behaviour for an entity in a slot", "<Slot> <Behaviour> [<Attack Intensity>] [<Impact Intensity>] [<delta HP>] [dt(tick)]")
{
	// Check parameters.
	if(args.size() < 2 || args.size() > 6)
	{
		// Help
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("This command need 2 to 6 paramters :"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Slot> : the slot number of the entity to change"));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring("  <Behaviour> : the behaviour to play for the entity, one of the following number :"));
		for(uint i = 0; i<MBEHAV::EMOTE_BEGIN; ++i)
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d - %s", i, MBEHAV::behaviourToString((MBEHAV::EBehaviour)i).c_str())));
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(NLMISC::toString("    %d-%d - Emotes", MBEHAV::EMOTE_BEGIN, MBEHAV::EMOTE_END)));
	}
	else
	{
		// Compute parameters
		CLFECOMMON::TCLEntityId	slot;
		fromString(args[0], slot);
		MBEHAV::EBehaviour beh				= MBEHAV::stringToBehaviour(args[1]);
		if(beh==MBEHAV::UNKNOWN_BEHAVIOUR)
		{
			sint32 temp;
			fromString(args[1], temp);
			beh= (MBEHAV::EBehaviour)temp;
		}

		// Make the behaviour
		MBEHAV::CBehaviour behaviour(beh);
		// Get the Power
		if ( (beh == MBEHAV::PROSPECTING) || (beh == MBEHAV::PROSPECTING_END) )
		{
			if(args.size() > 2)
			{
				uint16 range;
				fromString(args[2], range);
				behaviour.ForageProspection.Range = range; // 0..127
			}
			if(args.size() > 3)
			{
				uint16 angle;
				fromString(args[3], angle);
				behaviour.ForageProspection.Angle = angle; // 0..3
			}
			if(args.size() > 4)
			{
				uint16 level;
				fromString(args[4], level);
				behaviour.ForageProspection.Level = level; // 0..4
			}
		}
		else
		{
			if(args.size() > 2)
			{
				uint16 impactIntensity;
				fromString(args[2], impactIntensity);
				behaviour.Combat.ImpactIntensity = impactIntensity;
			}
			if(args.size() > 3)
			{
				uint16 impactIntensity;
				fromString(args[3], impactIntensity);
				behaviour.Combat.ImpactIntensity = impactIntensity;
			}
			if(args.size() > 4)
				fromString(args[4], behaviour.DeltaHP);
		}
		// get the dt
		sint32	dt= 10;
		if(args.size() > 5)
			fromString(args[5], dt);

		// Compute the position.
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), behaviour);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+dt, CLFECOMMON::PROPERTY_BEHAVIOUR);
		}
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot"));
	}

	// Command well done.
	return true;
}

/*
NLMISC_COMMAND(magic, "Cast a spell", "\n"
"<Slot> : the one who cast the spell\n"
"<type> : 0->GOOD 1->Bad 2->NEUTRAL\n"
"<success> : 0->success 1->Fail 2->Fumble\n"
"<Spell Power> : \n"
"<Impact Intensity> : \n"
"<resist> : 0->not resisted, any other->resisted.\n")
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

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
		CLFECOMMON::TCLEntityId	slot;
		fromString(args[0], slot);
		// Magic Type (good bad neutral)
		uint type;
		fromString(args[1], type);
		type %= 3;
		MBEHAV::EBehaviour behTmp = (MBEHAV::EBehaviour)(MBEHAV::CASTING_GOOD+type);
		MBEHAV::CBehaviour castingBeh(behTmp);
		// Result
		MBEHAV::CBehaviour behaviour;
		uint result;
		fromString(args[2], result);
		result %= %3;
		if     (type==0)
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_GOOD_SUCCESS    + result);
		else if(type==1)
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_BAD_SUCCESS     + result);
		else
			behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::END_CASTING_NEUTRAL_SUCCESS + result);
		uint16 spellPower, impactIntensity;
		// Spell Power
		fromString(args[3], spellPower);
		behaviour.Magic.SpellPower      = spellPower;
		// Impact Intensity
		fromString(args[4], impactIntensity);
		behaviour.Magic.ImpactIntensity = impactIntensity;
		// Resist
		bool targetResists;
		fromString(args[5], targetResists);
		behaviour.Magic.TargetResists   = targetResists;
		// Get the entity
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), castingBeh);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_BEHAVIOUR);
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), behaviour);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+50, CLFECOMMON::PROPERTY_BEHAVIOUR);
		}
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot"));
	}

	// Command well done.
	return true;
}
*/
NLMISC_COMMAND(spell, "Cast a spell", "\n"
			   "<Slot> : the one who cast the spell\n"
			   "<type> : 0->OFF 1->CUR 2->MIX\n"
			   "<success> : 0->Fail 1->Fumble 2->Success 3->Link\n"
			   "<Resist> : 0->Resist 1->Not Resist\n"
			   "<Spell Id> : \n"
			   "<Intensity> : [0, 5]\n")
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
		CLFECOMMON::TCLEntityId	slot;
		fromString(args[0], slot);
		// Magic Type (good bad neutral)
		uint type;
		fromString(args[1], type);
		type %= 3;
		MBEHAV::EBehaviour behTmp = (MBEHAV::EBehaviour)(MBEHAV::CAST_OFF+type);
		MBEHAV::CBehaviour castingBeh(behTmp);
		// Result
		MBEHAV::CBehaviour behaviour;
		uint result;
		fromString(args[2], result);
		result %= 4;
		behaviour.Behaviour = (MBEHAV::EBehaviour)(MBEHAV::CAST_OFF_FAIL+type*4+result);
		// Spell Power
		uint16 spellMode;
		fromString(args[3], spellMode);
		behaviour.Spell.SpellMode      = spellMode;
		// Impact Intensity
		uint16 spellId;
		fromString(args[4], spellId);
		behaviour.Spell.SpellId        = spellId;
		// Resist
		uint16 spellIntensity;
		fromString(args[5], spellIntensity);
		behaviour.Spell.SpellIntensity = spellIntensity;
		// Get the entity
		CEntityCL *entity = EntitiesMngr.entity(slot);
		if(entity)
		{
			uint64 beha = castingBeh;
			sint64 beha2 = *((sint64 *)(&beha));
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), beha2);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_BEHAVIOUR);
			beha  = behaviour;
			beha2 = *((sint64 *)(&beha));
			// Write the behaviour in the DB.
			IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P" + toString(CLFECOMMON::PROPERTY_BEHAVIOUR), beha2);
			// Update the behaviour.
			entity->updateVisualProperty(NetMngr.getCurrentServerTick()+50, CLFECOMMON::PROPERTY_BEHAVIOUR);
		}
		else
			CInterfaceManager::getInstance()->displaySystemInfo(ucstring("There is no entity in the given slot"));
	}

	// Command well done.
	return true;
}

NLMISC_COMMAND(settarget, "Set a target for an entity. Do not set the target slot to remove the target", "<Slot> [<Target Slot>]")
{
	CLFECOMMON::TCLEntityId targetSlot = CLFECOMMON::INVALID_SLOT;

	// Check parameters.
	switch(args.size())
	{
	// Set the target for the entity.
	case 2:
		fromString(args[1], targetSlot);

	// Remove the target for the entity.
	case 1:
	{
		uint entitySlot;
		fromString(args[0], entitySlot);
		CEntityCL *entity = EntitiesMngr.entity(entitySlot);
		if(entity)
			entity->targetSlot(targetSlot);
		else
			nlwarning("command 'settarget': there is no entity in the slot %d", entitySlot);
	}
	break;

	// Bad command.
	default:
		return false;
	}

	// Well done.
	return true;
}

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

	UInstance fx = Scene->createInstance(fn);

	// not found
	if(fx.empty())
	{
		log.displayNL ("Can't create instance '%s'", fn.c_str());
		return false;
	}

	fx.setPos(UserEntity->pos());
	fx.setClusterSystem(UserEntity->skeleton()->getClusterSystem());

	// Command well done.
	return true;
}

NLMISC_COMMAND(move, "Move an entity", "Slot: [1-254]")
{
	// Check parameters.
	if(args.size() != 1)
		return false;

	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
	{
		sint64 x = (sint64)((entity->pos().x+UserEntity->front().x*10.0)*1000.0);
		sint64 y = (sint64)((entity->pos().y+UserEntity->front().y*10.0)*1000.0);
		sint64 z = (sint64)((entity->pos().z+UserEntity->front().z*10.0)*1000.0);
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
		nlwarning("command 'move' : there is no entity allocated in slot %d", slot);

	// Command well done.
	return true;
}

NLMISC_COMMAND(moveRel, "Move an entity, specifying delta pos from current", "Slot: [1-254] dx(m) dy(m) [dt(tick)] [predictedIV(tick)]")
{
	// Check parameters.
	if(args.size() <3)
		return false;

	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
	{
		float	dx, dy;
		fromString(args[1], dx);
		fromString(args[2], dy);
		sint32	dt= 10;
		if(args.size()>=4)
			fromString(args[3], dt);
		sint32	pi= 0;
		if(args.size()>=5)
			fromString(args[4], pi);
		sint64 x = (sint64)((entity->pos().x+dx)*1000.0);
		sint64 y = (sint64)((entity->pos().y+dy)*1000.0);
		sint64 z = (sint64)((entity->pos().z+0)*1000.0);
		// Write the position in the DB.
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
		// Update the position.
		EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+dt, slot, 0, pi);
	}
	else
		nlwarning("command 'move' : there is no entity allocated in slot %d", slot);

	// Command well done.
	return true;
}

NLMISC_COMMAND(orient, "Orient an entity", "Slot: [1-254] orient(degree) [dt(tick)]")
{
	// Check parameters.
	if(args.size() < 2)
		return false;

	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
	{
		sint32	rot;
		fromString(args[1], rot);
		sint32	dt= 10;
		if(args.size()> 2)
			fromString(args[2], dt);
		// Write the position in the DB.
		float	fRot= (float)(rot*Pi/180.f);
		uint64	val= *(uint32*)(&fRot);
		IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P"+toString(CLFECOMMON::PROPERTY_ORIENTATION), val);
		// Update the position.
		EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+dt, slot, CLFECOMMON::PROPERTY_ORIENTATION);
	}
	else
		nlwarning("command 'move' : there is no entity allocated in slot %d", slot);

	// Command well done.
	return true;
}

NLMISC_COMMAND(moveTo, "Move an entity to another one", "<slot(from)>:[1-254], <slot(to)>:[0-254] default 0")
{
	sint64 x, y, z;

	// Check parameters.
	if(args.size() == 1)
	{
		x = (sint64)(UserEntity->pos().x*1000.0);
		y = (sint64)(UserEntity->pos().y*1000.0);
		z = (sint64)(UserEntity->pos().z*1000.0);
	}
	else if(args.size() == 2)
	{
		CLFECOMMON::TCLEntityId	slotDest;
		fromString(args[1], slotDest);
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
			nlwarning("command 'move_to' : there is no entity allocated for the dest in slot %d", slotDest);
			return true;
		}
	}
	// Wrong number of parameters.
	else
		return false;

	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);
	// Write the position in the DB.
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P0", x);
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P1", y);
	IngameDbMngr.setProp("Entities:E" + toString(slot) + ":P2", z);
	// Update the position.
	EntitiesMngr.updateVisualProperty(NetMngr.getCurrentServerTick()+30, slot, 0);

	// Command well done.
	return true;
}

NLMISC_COMMAND(setMode, "Set The Mode for an Entity without to add a stage for it", "<slot> <mode>")
{
	// Check parameters.
	if(args.size() != 2)
		return false;

	// Get the Slot and the Mode.
	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);
	sint32 nMode;
	fromString(args[1], nMode);
	MBEHAV::EMode mod = (MBEHAV::EMode)nMode;

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
		entity->mode(mod);
	else
		nlwarning("command 'setMode' : there is no entity allocated in slot '%d'", slot);

	// Command well done.
	return true;
}
NLMISC_COMMAND(paintTarget, "Modify the target color",
			   "\n"
			   "<color> color for the target (0-7)\n")
{
	// Check parameters
	if(args.size() != 1)
		return false;
	// Get the entity slot
	CLFECOMMON::TCLEntityId slot = UserEntity->selection();
	if(slot == CLFECOMMON::INVALID_SLOT)
		return true;
	//
	SPropVisualA vA;
	SPropVisualB vB;
	SPropVisualC vC;
	const string propNameA = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPA);
	const string propNameB = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPB);
	const string propNameC = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPC);
	vA.PropertyA = NLGUI::CDBManager::getInstance()->getDbProp(propNameA)->getValue64();
	vB.PropertyB = NLGUI::CDBManager::getInstance()->getDbProp(propNameB)->getValue64();
	vC.PropertyC = NLGUI::CDBManager::getInstance()->getDbProp(propNameC)->getValue64();

	// Get the visual item index
	uint value;
	fromString(args[0], value);
	// Change color
	vA.PropertySubData.JacketColor = value;
	vA.PropertySubData.TrouserColor = value;
	vA.PropertySubData.ArmColor = value;
	vA.PropertySubData.HatColor = value;
	vB.PropertySubData.HandsColor = value;
	vB.PropertySubData.FeetColor = value;

	// Set the database.
	NLGUI::CDBManager::getInstance()->getDbProp(propNameA)->setValue64((sint64)vA.PropertyA);
	NLGUI::CDBManager::getInstance()->getDbProp(propNameB)->setValue64((sint64)vB.PropertyB);
	NLGUI::CDBManager::getInstance()->getDbProp(propNameC)->setValue64((sint64)vC.PropertyC);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);

	// Done.
	return true;
}

NLMISC_COMMAND(playAnim, "Try to play the animation to the target", "<anim name>")
{
	// Check parameters
	if(args.size() != 1)
		return false;

	CLFECOMMON::TCLEntityId slot = UserEntity->selection();
	if(slot == CLFECOMMON::INVALID_SLOT)
		return true;
	if(EAM == 0)
		return true;
	NL3D::UAnimationSet *animset = EAM->getAnimationSet();
	if(animset == 0)
		return true;
	uint animId = animset->getAnimationIdByName(args[0]);
	if(animId == UAnimationSet::NotFound)
	{
		nlwarning("anim not found %s", args[0].c_str());
		return true;
	}
	CEntityCL *selection = EntitiesMngr.entity(slot);
	CCharacterCL *character = dynamic_cast<CCharacterCL *>(selection);
	if(character)
		character->setAnim(CAnimationStateSheet::Idle, (TAnimStateKey)CAnimation::UnknownAnim, animId);
	return true;
}

NLMISC_COMMAND(vP, "Modify the Visual Property",
"\n"
"<slot> of the entity to change.\n"
"<type> the property to change :\n"
"  0->CHEST (0~511)\n"
"  1->LEG (0~255)\n"
"  2->ARM (0~255)\n"
"  3->HEAD (0~127)\n"
"  4->WEAPON_R (0~2047)\n"
"  5->WEAPON_L (0~255)\n"
"  6->FEET (0~511)\n"
"  7->HAND (0~511)\n"
"  8->EYES COLOR (0~7)\n"
"  9->SEX (0: Male, 1: Female)\n"
" 10->TATOO (0~31)\n"
" 11->CHEST COLOR (0~7)\n"
" 12->LEG COLOR (0~7)\n"
" 13->ARM COLOR (0~7)\n"
" 14->HAIR COLOR (0~7)\n"
" 15->HAND COLOR (0~7)\n"
" 16->FEET COLOR (0~7)\n"
" 17->MORPH 1 (0~7)\n"
" 18->MORPH 2 (0~7)\n"
" 19->MORPH 3 (0~7)\n"
" 20->MORPH 4 (0~7)\n"
" 21->MORPH 5 (0~7)\n"
" 22->MORPH 6 (0~7)\n"
" 23->MORPH 7 (0~7)\n"
" 24->CHARACTER HEIGHT (0~15)\n"
" 25->TORSO WIDTH (0~15)\n"
" 26->ARMS WIDTH (0~15)\n"
" 27->LEGS WIDTH (0~15)\n"
" 28->BREASTS SIZE (0~15)\n"
"<value> for the property.\n")
{
	// Check parameters
	if(args.size() != 3)
		return false;

	// Get the database entry.
	SPropVisualA vA;
	SPropVisualB vB;
	SPropVisualC vC;
	uint slot;
	fromString(args[0], slot);
	const string propNameA = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPA);
	const string propNameB = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPB);
	const string propNameC = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPC);
	vA.PropertyA = NLGUI::CDBManager::getInstance()->getDbProp(propNameA)->getValue64();
	vB.PropertyB = NLGUI::CDBManager::getInstance()->getDbProp(propNameB)->getValue64();
	vC.PropertyC = NLGUI::CDBManager::getInstance()->getDbProp(propNameC)->getValue64();
	// Get the visual item index
	uint value;
	fromString(args[2], value);
	// Get the visual slot to change.
	uint type;
	fromString(args[1], type);
	// if .sitem visual slot, try translate .sitem to VSIndex
	if(type<=7)
	{
		SLOTTYPE::EVisualSlot	vslot= SLOTTYPE::HIDDEN_SLOT;
		switch(type)
		{
		case 0: vslot= SLOTTYPE::CHEST_SLOT; break;
		case 1: vslot= SLOTTYPE::LEGS_SLOT; break;
		case 2: vslot= SLOTTYPE::ARMS_SLOT; break;
		case 3: vslot= SLOTTYPE::HEAD_SLOT; break;
		case 4: vslot= SLOTTYPE::RIGHT_HAND_SLOT; break;
		case 5: vslot= SLOTTYPE::LEFT_HAND_SLOT; break;
		case 6: vslot= SLOTTYPE::FEET_SLOT; break;
		case 7: vslot= SLOTTYPE::HANDS_SLOT; break;
		default: break;
		}
		if(vslot!=SLOTTYPE::HIDDEN_SLOT && value==0)
		{
			sint	vsIndex= SheetMngr.getVSIndex(args[2], vslot);
			// succed!
			if(vsIndex!=-1)
				value= vsIndex;
		}
	}
	// setup
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
	case 10:
		vC.PropertySubData.Tattoo = value;
		break;
	case 11:
		vA.PropertySubData.JacketColor = value;
		break;
	case 12:
		vA.PropertySubData.TrouserColor = value;
		break;
	case 13:
		vA.PropertySubData.ArmColor = value;
		break;
	case 14:
		vA.PropertySubData.HatColor = value;
		break;
	case 15:
		vB.PropertySubData.HandsColor = value;
		break;
	case 16:
		vB.PropertySubData.FeetColor = value;
		break;
	case 17:
		vC.PropertySubData.MorphTarget1 = value;
		break;
	case 18:
		vC.PropertySubData.MorphTarget2 = value;
		break;
	case 19:
		vC.PropertySubData.MorphTarget3 = value;
		break;
	case 20:
		vC.PropertySubData.MorphTarget4 = value;
		break;
	case 21:
		vC.PropertySubData.MorphTarget5 = value;
		break;
	case 22:
		vC.PropertySubData.MorphTarget6 = value;
		break;
	case 23:
		vC.PropertySubData.MorphTarget7 = value;
		break;
	case 24:
		vC.PropertySubData.CharacterHeight = value;
		break;
	case 25:
		vC.PropertySubData.TorsoWidth = value;
		break;
	case 26:
		vC.PropertySubData.ArmsWidth = value;
		break;
	case 27:
		vC.PropertySubData.LegsWidth = value;
		break;
	case 28:
		vC.PropertySubData.BreastSize = value;
		break;

	default:
		nlwarning("command 'vP': type not valid");
		return false;
		break;
	}

	// Set the database.
	NLGUI::CDBManager::getInstance()->getDbProp(propNameA)->setValue64((sint64)vA.PropertyA);
	NLGUI::CDBManager::getInstance()->getDbProp(propNameB)->setValue64((sint64)vB.PropertyB);
	NLGUI::CDBManager::getInstance()->getDbProp(propNameC)->setValue64((sint64)vC.PropertyC);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);

	// Done.
	return true;
}

NLMISC_COMMAND(altLook, "Modify the Alternative Look Property",
			   "\n"
			   "<slot> of the entity to change.\n"
			   "<colorTop>\n"
			   "<colorBottom>\n"
			   "<rWeapon>\n"
			   "<lWeapon>\n"
			   "<seed>\n"
			   "<hairColor>\n"
			   "<putHelm>\n"
			   "[<colorGlove>]\n"
			   "[<colorBoot>]\n"
			   "[<colorArm>]\n")
{
	// Check parameters
	if(args.size() < 8 || args.size() > 11)
		return false;

	// Get the database entry.
	uint slot;
	fromString(args[0], slot);
	const string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_VPA);
	// Get the old value (not useful since we change the whole property).
	SAltLookProp altLookProp;
	altLookProp.Summary = NLGUI::CDBManager::getInstance()->getDbProp(propName)->getValue64();
	uint32 colorTop, colorBot, weaponRightHand, weaponLeftHand, seed, colorHair, hat;
	fromString(args[1], colorTop);
	fromString(args[2], colorBot);
	fromString(args[3], weaponRightHand);
	fromString(args[4], weaponLeftHand);
	fromString(args[5], seed);
	fromString(args[6], colorHair);
	fromString(args[7], hat);
	altLookProp.Element.ColorTop		= colorTop;
	altLookProp.Element.ColorBot		= colorBot;
	altLookProp.Element.WeaponRightHand	= weaponRightHand;
	altLookProp.Element.WeaponLeftHand	= weaponLeftHand;
	altLookProp.Element.Seed			= seed;
	altLookProp.Element.ColorHair		= colorHair;
	altLookProp.Element.Hat				= hat;
	// New colours
	if(args.size() == 11)
	{
		uint32 colorGlove, colorBoot, colorArm;
		fromString(args[8], colorGlove);
		fromString(args[9], colorBoot);
		fromString(args[10], colorArm);
		altLookProp.Element.ColorGlove		= colorGlove;
		altLookProp.Element.ColorBoot		= colorBoot;
		altLookProp.Element.ColorArm		= colorArm;
	}
	// Old Colours
	else
	{
		altLookProp.Element.ColorGlove		= altLookProp.Element.ColorTop;
		altLookProp.Element.ColorArm		= altLookProp.Element.ColorTop;
		altLookProp.Element.ColorBoot		= altLookProp.Element.ColorBot;
	}

	// Set the database.
	NLGUI::CDBManager::getInstance()->getDbProp(propName)->setValue64((sint64)altLookProp.Summary);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);

	// Done.
	return true;
}

NLMISC_COMMAND(color, "Command to color an entity",
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
		fromString(args[4], part);

	// Get the entity slot to dye.
	sint slotTmp;
	fromString(args[0], slotTmp);
	CLFECOMMON::TCLEntityId	slot;
	if(slotTmp >= 0)
		slot = (CLFECOMMON::TCLEntityId)slotTmp;
	else
		slot = (CLFECOMMON::TCLEntityId)UserEntity->selection();

	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(entity)
	{
		sint color, hair, eyes;
		fromString(args[1], color);
		fromString(args[2], hair);
		fromString(args[3], eyes);
		entity->changeColors(color, hair, eyes, part);
	}
	else
		nlwarning("command 'changeColors': there is no entity allocated in slot '%d'", slot);

	// Command well done.
	return true;
}

NLMISC_COMMAND(saveIntCfg, "save the interface config file","")
{
	CInterfaceManager::getInstance()->saveConfig ("save/interface.icfg");
	return true;
}

NLMISC_COMMAND(loadIntCfg, "load the interface config file","")
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->loadConfig ("save/interface.icfg");
	// reset the compass target
 	CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass"));
	if (gc && gc->isSavedTargetValid())
	{
		gc->setTarget(gc->getSavedTarget());
	}
	return true;
}

NLMISC_COMMAND(harvestDeposit, "harvest a deposit", "")
{
	// no parameter needed

	// Create the message for the server
/*	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("HARVEST:DEPOSIT", out))
	{
		uint16 skill = SKILLS::digging;

		out.serial(skill);

		NetMngr.push(out);

		// open the interface
		// CWidgetManager::getInstance()->getWindowFromId("ui:interface:harvest")->setActive(true);
	}
	else
		nlwarning("command : unknown message name : 'HARVEST:DEPOSIT'");
*/
	return true;
}

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
		nlwarning("command : unknown message name : 'TRAINING'");

	return true;
}

NLMISC_COMMAND(testMount, "Set the entity to mount","<Slot> <Mount>")
{
	CLFECOMMON::TCLEntityId slot;
	CLFECOMMON::TCLEntityId mount = CLFECOMMON::INVALID_SLOT;

	switch(args.size())
	{
	case 2:
		fromString(args[1], mount);
	case 1:
		fromString(args[0], slot);
		break;

	default:
		return false;
		break;
	}

	// Set the database.
	string propName = toString("SERVER:Entities:E%d:P%d", mount, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	NLGUI::CDBManager::getInstance()->getDbProp(propName)->setValue64(slot);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, mount, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	// Set the database.
	propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	NLGUI::CDBManager::getInstance()->getDbProp(propName)->setValue64(mount);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	return true;
}

NLMISC_COMMAND(mount, "Set the entity to mount","<Slot> [<Mount>]")
{
	CLFECOMMON::TCLEntityId slot;
	CLFECOMMON::TCLEntityId mount = CLFECOMMON::INVALID_SLOT;

	switch(args.size())
	{
	case 2:
		fromString(args[1], mount);
	case 1:
		fromString(args[0], slot);
		break;

	default:
		return false;
		break;
	}

	// Set the database.
	string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	NLGUI::CDBManager::getInstance()->getDbProp(propName)->setValue64(mount);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);

	// Command well done
	return true;
}

NLMISC_COMMAND(rider, "Set the rider","<Slot> [<rider>]")
{
	CLFECOMMON::TCLEntityId slot;
	CLFECOMMON::TCLEntityId rider = CLFECOMMON::INVALID_SLOT;

	switch(args.size())
	{
	case 2:
		fromString(args[1], rider);
	case 1:
		fromString(args[0], slot);
		break;

	default:
		return false;
		break;
	}

	// Set the database.
	string propName = toString("SERVER:Entities:E%d:P%d", slot, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	NLGUI::CDBManager::getInstance()->getDbProp(propName)->setValue64(rider);
	// Force to update properties.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);

	// Command well done
	return true;
}

NLMISC_COMMAND(disbandConvoy, "disband current beasts convoy", "")
{
	// no parameter needed

	// Create the message for the server
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("ANIMALS:DISBAND_CONVOY", out))
	{
		NetMngr.push(out);
	}
	else
		nlwarning("command : unknown message name : 'ANIMALS:DISBAND_CONVOY'");

	return true;
}

NLMISC_COMMAND(learnAllBrick, "learn all bricks (only in local mode)", "")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	i=0;
	for(;;)
	{
		CCDBNodeLeaf * node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:BRICK_FAMILY:%d:BRICKS", i), false);
		if(node)
			node->setValue64(SINT64_CONSTANT(0xFFFFFFFFFFFFFFFF));
		else
			break;
		i++;
	}
	return true;
}

NLMISC_COMMAND(learnBrick, "learn a specified brick (only in local mode)", "<brick number or name>")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	if(args.size()<1)
		return false;

	// translate to brick sheet id
	CSheetId	brickSheetId;
	uint		testId;
	fromString(args[0], testId);
	if(testId!=0)
	{
		brickSheetId= CSheetId(testId);
	}
	else
	{
		string	str= args[0];
		if(str.find(".sbrick")==string::npos)
			str+= ".sbrick";
		brickSheetId.buildSheetId(str);
	}

	// get the brick sheet
	CSBrickSheet	*brick= pBM->getBrick(brickSheetId);
	if(!brick)
	{
		pIM->displaySystemInfo(toString("brick '%s' not found", args[0].c_str()));
		return false;
	}

	// force learn it.
	CCDBNodeLeaf * node= pBM->getKnownBrickBitFieldDB(brick->BrickFamily);
	if(node)
	{
		uint64	flags= node->getValue64();
		flags|= uint64(1)<<(brick->IndexInFamily-1);
		node->setValue64(flags);
	}
	return true;
}

NLMISC_COMMAND(learnPhrase, "learn all bricks of a specified phrase (only in local mode)", "<phrase sheetId or name>")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	if(args.size()<1)
		return false;

	// translate to brick sheet id
	CSheetId	phraseSheetId;
	uint		testId;
	fromString(args[0], testId);
	if(testId!=0)
	{
		phraseSheetId= CSheetId(testId);
	}
	else
	{
		string	str= args[0];
		if(str.find(".sphrase")==string::npos)
			str+= ".sphrase";
		phraseSheetId.buildSheetId(str);
	}

	// get the brick sheet
	CSPhraseCom		phrase;
	pPM->buildPhraseFromSheet(phrase, phraseSheetId.asInt());
	if(phrase.empty())
	{
		pIM->displaySystemInfo(toString("phrase '%s' not found", args[0].c_str()));
		return false;
	}

	// For all bricks of this phrase
	for(uint i=0;i<phrase.Bricks.size();i++)
	{
		CSBrickSheet	*brick= pBM->getBrick(phrase.Bricks[i]);
		if(brick)
		{
			// force learn it.
			CCDBNodeLeaf * node= pBM->getKnownBrickBitFieldDB(brick->BrickFamily);
			if(node)
			{
				uint64	flags= node->getValue64();
				flags|= uint64(1)<<(brick->IndexInFamily-1);
				node->setValue64(flags);
			}
		}
	}

	return true;
}

/*NLMISC_COMMAND(xp, "To gain XP in a given Skill","<Amount Xp> <Skill> [<Speciality>]")
{
	// Check parameters.
	if( args.size() < 2 || args.size() > 3 )
		return false;

	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CHEAT:XP", out))
	{
		uint32 xp;
		fromString(args[0], xp);
		string skill = args[1];
		string speciality;
		if( args.size() == 3 )
			speciality = args[2];
		out.serial( xp );
		out.serial( skill );
		out.serial( speciality );
		// Add the message to the send list.
		NetMngr.push(out);
		// send CHEAT:XP
		nlinfo("command 'xp': CHEAT:XP pushed");
	}
	else
		nlwarning("command 'xp': unknown message named 'CHEAT:XP'");

	// Done.
	return true;
}*/

NLMISC_COMMAND(money, "To earn Money (only in local mode)","<very big seed> [<big seed>] [<medium seed>] [<small seed>]")
{
	if (args.size() != 1) return false;
	uint64 money;
	fromString(args[0], money);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:MONEY")->setValue64(money);
	return true;
/*
	sint32 a = 0;
	sint32 b = 0;
	sint32 c = 0;
	sint32 d = 0;

	// Check parameters.
	switch(args.size())
	{
	case 4:
		fromString(args[3], d);
	case 3:
		fromString(args[2], c);
	case 2:
		fromString(args[1], b);
	case 1:
		fromString(args[0], a);
		break;
	default:
		return false;
	}

	CInterfaceManager *im = CInterfaceManager::getInstance();
	string ls = im->getDefine("money_1");
	string ms = im->getDefine("money_2");
	string bs = im->getDefine("money_3");
	string vbs = im->getDefine("money_4");
	NLGUI::CDBManager::getInstance()->getDbProp(ls + ":QUANTITY")->setValue32(a);
	NLGUI::CDBManager::getInstance()->getDbProp(ms + ":QUANTITY")->setValue32(b);
	NLGUI::CDBManager::getInstance()->getDbProp(bs + ":QUANTITY")->setValue32(c);
	NLGUI::CDBManager::getInstance()->getDbProp(vbs + ":QUANTITY")->setValue32(d);
	return true;
*/
}
/*
NLMISC_COMMAND( createPerso, "create a new character", "Parameters:\n-Character name\n-Race( Fyros, Tryker...)\n-gender(Male, Female)\n-Role( MeleeFighter, RangeFighter, AttackCaster, BufferCaster, HealerCaster...)\n-Level (1-25 (but more accepted)>" )
{
	// Check parameters.
	if(args.size() < 5) return false;

	// read params
	string characterName = args[0];
	EGSPD::CPeople::TPeople race = EGSPD::CPeople::fromString( args[1] );
	if( race == EGSPD::CPeople::EndPeople ) return false;

	GSGENDER::EGender gender = GSGENDER::stringToEnum( args[2] );
	if( gender == GSGENDER::unknown ) return false;

	ROLES::ERole role = ROLES::toRoleId( args[3] );
	if( role == ROLES::role_unknown ) return false;

	uint16 level;
	fromString(args[4], level);

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
}
*/
/*
NLMISC_COMMAND( add_role, "add role to character", "<Role( MeleeFighter, RangeFighter, AttackCaster, BufferCaster, HealerCaster...), Level (1-25 (but more accepted))>" )
{
	// Check parameters.
	if(args.size() < 2) return false;

	ROLES::ERole role = ROLES::toRoleId( args[0] );
	if( role == ROLES::role_unknown ) return false;

	uint16 level;
	fromString(args[1], level);

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
}
*/

NLMISC_COMMAND(test, "", "")
{
	sint64 x, y, z;
	CLFECOMMON::TCLEntityId	entSlot = UserEntity->selection();
	CEntityCL *entPtr = EntitiesMngr.entity(entSlot);
	if(entPtr)
	{
		if(entPtr->skeleton())
		{
			if(entPtr->skeleton()->getLastClippedState())
			{
				NLMISC::CMatrix mat = entPtr->skeleton()->getLastWorldMatrixComputed();
				NLMISC::CVectorD newPos = entPtr->pos() + mat.getJ()*0.5f;
				x = (sint64)(newPos.x*1000.0);
				y = (sint64)(newPos.y*1000.0);
				z = (sint64)(newPos.z*1000.0);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX), x);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY), y);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ), z);
				entPtr->updateVisualProperty(NetMngr.getCurrentServerTick(), CLFECOMMON::PROPERTY_POSITION);

				x = (sint64)(entPtr->pos().x*1000.0);
				y = (sint64)(entPtr->pos().y*1000.0);
				z = (sint64)(entPtr->pos().z*1000.0);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSX), x);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSY), y);
				IngameDbMngr.setProp("Entities:E" + toString("%d", entSlot) + ":P" + toString("%d", CLFECOMMON::PROPERTY_POSZ), z);
				entPtr->updateVisualProperty(NetMngr.getCurrentServerTick()+5, CLFECOMMON::PROPERTY_POSITION);
			}
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
	if(!sheetId.buildSheetId(args[0]))																				\
	{																												\
		/* Try to create the sheet with the parameter as an int. */													\
		uint32 nSheetId;																							\
		fromString(args[0], nSheetId);																				\
		sheetId = CSheetId(nSheetId);																				\
		if(sheetId == CSheetId::Unknown)																			\
		{																											\
			nlwarning("Command '" #commandName "': '%s' is not a valid form.", args[0].c_str());					\
			return false;																							\
		}																											\
	}																												\
																													\
	/* Get the new distance. */																						\
	float dist;																										\
	fromString(args[1], dist);																						\
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



// Change the parent of an entity. 'parent slot' not defined remove the current parent.
NLMISC_COMMAND(parent, "Change the parent of an entity.", "<slot> [<parent slot>]")
{
	CLFECOMMON::TCLEntityId parentSlot = CLFECOMMON::INVALID_SLOT;

	// Check parameters.
	switch(args.size())
	{
	// Set the target for the entity.
	case 2:
		fromString(args[1], parentSlot);

	// Remove the target for the entity.
	case 1:
	{
		uint entitySlot;
		fromString(args[0], entitySlot);
		CEntityCL *entity = EntitiesMngr.entity(entitySlot);
		if(entity)
		{
			entity->parent(parentSlot);
			entity->pos(CVectorD::Null);
		}
		else
			nlwarning("command 'parent': there is no entity in the slot %d", entitySlot);
	}
	break;

	// Bad command.
	default:
		return false;
	}

	// Well done.
	return true;
}

NLMISC_COMMAND(displayInventoryCounter, "display the Inventory counter to compare with db counter", "")
{

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint	srvVal= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:COUNTER")->getValue32();
	uint	locVal= pIM->getLocalSyncActionCounter() ;
	srvVal&= pIM->getLocalSyncActionCounterMask();
	locVal&= pIM->getLocalSyncActionCounterMask();

	pIM->displaySystemInfo(ucstring( "ServerCounter: " + toString(srvVal) + "/ LocalCounter: " + toString(locVal)) );

	// Well done.
	return true;
}


NLMISC_COMMAND(displayActionCounter, "display the action counters", "")
{

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	// next
	uint	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_NEXT)->getValue32();
	uint	locVal= pPM->getPhraseNextExecuteCounter() ;
	srvVal&= PHRASE_EXECUTE_COUNTER_MASK;
	locVal&= PHRASE_EXECUTE_COUNTER_MASK;

	pIM->displaySystemInfo(ucstring( "NextCounter: " + toString(srvVal) + "/ LocalCounter: " + toString(locVal)) );

	// cycle
	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_CYCLE)->getValue32();
	locVal= pPM->getPhraseCycleExecuteCounter() ;
	srvVal&= PHRASE_EXECUTE_COUNTER_MASK;
	locVal&= PHRASE_EXECUTE_COUNTER_MASK;

	pIM->displaySystemInfo(ucstring( "CycleCounter: " + toString(srvVal) + "/ LocalCounter: " + toString(locVal)) );

	return true;
}


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

NLMISC_COMMAND( reconnect, "Reconnect to the same shard (self Far TP)", "")
{
	// If the server is up, the egs will begin the quit sequence (shortened only if we are in edition or animation mode).
	// If the server is down or frozen, a second /reconnect will be necessary to make the client reconnect
	// but if you reconnect before letting the EGS save the character file, the previous saved file will be loaded.
	switch ( LoginSM.getCurrentState() )
	{
	case CLoginStateMachine::st_ingame:
		LoginSM.pushEvent( CLoginStateMachine::ev_connect );
		break;
	case CLoginStateMachine::st_leave_shard:
		FarTP.onServerQuitOk();
		break;
	default:
		log.displayNL( "Can't reconnect from LoginSM state %u", (uint)LoginSM.getCurrentState() );
	}

	return true;
}

#endif // !FINAL_VERSION

struct CItemSheetSort
{
	const CItemSheet		*IS;
	CSheetId				 ID;
	ITEMFAMILY::EItemFamily  Family;
};

static inline bool operator < (const CItemSheetSort &lhs, const CItemSheetSort &rhs)
{
	return lhs.Family < rhs.Family;
}

NLMISC_COMMAND(dumpItems, "Sort items by category & display their sheet ids", "")
{
	std::vector<CItemSheetSort> isVect;
	const CSheetManager::TEntitySheetMap &sheets = SheetMngr.getSheets();
	for(CSheetManager::TEntitySheetMap::const_iterator it = sheets.begin(); it != sheets.end(); ++it)
	{
		const CEntitySheet *es = it->second.EntitySheet;
		if (es && es->type() == CEntitySheet::ITEM)
		{
			CItemSheetSort iss;
			iss.IS = static_cast<const CItemSheet *>(es);
			iss.ID = it->first;
			iss.Family = iss.IS->Family;
			isVect.push_back(iss);
		}
	}
	//
	// sort items
	std::sort(isVect.begin(), isVect.end());
	//
	for(std::vector<CItemSheetSort>::iterator itemIt = isVect.begin(); itemIt != isVect.end(); ++itemIt)
	{
		std::string info;
		info = "FAMILY: ";
		info += ITEMFAMILY::toString(itemIt->Family);
		info += ";  Name = ";
		info += itemIt->IS->Id.toString();
		info += "; Sheet ID = ";
		info += toString(itemIt->ID.asInt());
		nlwarning(info.c_str());
	}
	return true;
}

NLMISC_COMMAND(dumpVisualSlots, "dump the visual slots", "")
{
	if (!args.empty()) return false;
	SheetMngr.dumpVisualSlots();
	SheetMngr.dumpVisualSlotsIndex();
	return true;
}

NLMISC_COMMAND(skillToInt, "Convert a skill to an int", "")
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->displaySystemInfo(ucstring(toString((uint) SKILLS::toSkill(args[0]))));
	return true;
}

NLMISC_COMMAND(browse, "Browse a HTML document with the internal help web browser.", "")
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CAHManager::getInstance()->runActionHandler("browse", NULL, "name=ui:interface:help_browser:content:html|url="+args[0]);
	return true;
}

NLMISC_COMMAND(openRingWindow, "Browse the main page in the ring web browser.", "")
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CAHManager::getInstance()->runActionHandler("browse", NULL, "name=ui:interface:r2ed_web_admin:content:admin_web_page|url="+RingMainURL);
	return true;
}

NLMISC_COMMAND(browseRingAdmin, "Browse a HTML document with the ring web browser.", "")
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CAHManager::getInstance()->runActionHandler("browse", NULL, "name=ui:interface:r2ed_web_admin:content:admin_web_page|url="+args[0]);
	return true;
}

NLMISC_COMMAND(GUCreate, "create a guild", "<guild name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:CREATE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUQuit, "quit a guild", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:QUIT";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GULeaveLeadership, "abandon leadership of a guild", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:ABANDON_LEADERSHIP";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}
NLMISC_COMMAND(GULeaveOfficerTitle, "abandon officer title", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:ABANDON_OFFICER_TITLE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUNameOfficer, "name an officer", "<player name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:NAME_OFFICER";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUDismissOfficer, "dismiss an officer", "<player name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:DISMISS_OFFICER";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUKick, "kick a member", "<player name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:KICK_MEMBER";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}


NLMISC_COMMAND(GUAccept, "accept an invitation", "")
{
	CAHManager::getInstance()->runActionHandler("accept_guild_invitation",NULL);
	return true;
}

NLMISC_COMMAND(GURefuse, "refuse an invitation", "")
{
	CAHManager::getInstance()->runActionHandler("refuse_guild_invitation",NULL);
	return true;
}

NLMISC_COMMAND(GUFriend, "invite a player to become a friend of the guild", "<player name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:FRIEND_INVITATION";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUFriendAccept, "accept to be a friend of a guild that invited you", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:ACCEPT_FRIEND_INVITATION";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUFriendRefuse, "refuse to be a friend of a guild that invited you", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:REFUSE_FRIEND_INVITATION";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUSetSuccessor, "set the successor of the guild leader", "<player name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:SET_SUCCESSOR";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUInfos, "get information on a guild", "<guild name>")
{
	if (args.size() != 1) return false;
	const string msgName = "GUILD:GET_INFOS";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		string buf = args[0];
		out.serial( buf );
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(GUJournal, "get the guild journal", "")
{
	if (args.size() != 0) return false;
	const string msgName = "GUILD:GET_LOG";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(buildingTeleport, "teleport to a building", "building index")
{
	if (args.size() != 1) return false;
	uint16 index;
	fromString(args[0], index);
	const string msgName = "GUILD:TELEPORT";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		out.serial(index);
		NetMngr.push(out);
	}
	return true;
}

NLMISC_COMMAND(logFaberMpCompatibles, "log all MP compatibles for faber the item", "sheetid")
{
	if (args.size() != 1) return false;
	uint32 sheetId;
	fromString(args[0], sheetId);
	CSBrickManager	*pBM= CSBrickManager::getInstance();

	// get the faber plan
	CSBrickSheet	*brick= pBM->getBrick(CSheetId(sheetId));
	// get the origin of the item built
	CItemSheet		*itemBuilt= NULL;
	if(brick)
		itemBuilt= dynamic_cast<CItemSheet*>(SheetMngr.get(brick->FaberPlan.ItemBuilt));
	if(brick && itemBuilt)
	{

		// build array of MP sheetId
		std::vector<CItemSheet*>	mps;
		mps.reserve(100);
		const CSheetManager::TEntitySheetMap &sheetMap= SheetMngr.getSheets();
		CSheetManager::TEntitySheetMap::const_iterator	it;
		for(it= sheetMap.begin(); it!=sheetMap.end(); it++)
		{
			CItemSheet	*mp= const_cast<CItemSheet*>( dynamic_cast<const CItemSheet*>(it->second.EntitySheet) );
			if(mp && mp->Family == ITEMFAMILY::RAW_MATERIAL)
				mps.push_back(mp);
		}

		// header
		uint	numMpSlots= (uint)brick->FaberPlan.ItemPartMps.size();
		nlinfo("**********  FABERLOG  **********");
		nlinfo("  ItemBuilt Origin: %s", ITEM_ORIGIN::enumToString(itemBuilt->ItemOrigin).c_str() );
		nlinfo("  NumMPSlot: %d", numMpSlots);

		// Parse All Slots.
		for(uint i=0;i<numMpSlots;i++)
		{
			CSBrickSheet::CFaberPlan::CItemPartMP		&mpSlot= brick->FaberPlan.ItemPartMps[i];
			nlinfo("  MPSlot %d", i);
			nlinfo("  Quantity: %d", mpSlot.Quantity);
			nlinfo("  TypeReq: %s", RM_FABER_TYPE::toString(mpSlot.FaberTypeFilter).c_str() );
			nlinfo("    List Of Compatibles MPs:");

			for(uint i=0;i<mps.size();i++)
			{
				CItemSheet	*itemSheet= mps[i];
				bool	ok= true;
				// check faber type filter
				if( mpSlot.FaberTypeFilter!=RM_FABER_TYPE::Unknown && !itemSheet->canBuildItemPart(mpSlot.FaberTypeFilter, itemBuilt->ItemOrigin))
					ok= false;

				if(ok)
				{
					nlinfo("      %s", itemSheet->Id.toString().c_str() );
				}
			}

		}
	}

	return true;
}

NLMISC_COMMAND(debugItemInfo, "simulate a ItemInfo received from server", "itemSlotId version [enchant]")
{
	CItemInfos itemInfo;

	if (args.size() < 2 || args.size() > 3) return false;
	bool	enchant= false;
	if(args.size()==3)
		fromString(args[2], enchant);

	fromString(args[0], itemInfo.slotId);
	fromString(args[1], itemInfo.versionInfo);

	itemInfo.CurrentDamage= 10;
	itemInfo.MaxDamage= 15;
	itemInfo.DodgeModifier= 5;
	itemInfo.ParryModifier= -10;
	itemInfo.AdversaryDodgeModifier= 666;
	itemInfo.AdversaryParryModifier= 333;
	itemInfo.HpBuff= 12;
	itemInfo.SapBuff= -14;
	itemInfo.StaBuff= 0;
	itemInfo.FocusBuff= 1;
	itemInfo.MagicProtection[0]= PROTECTION_TYPE::Electricity;
	itemInfo.MagicProtectionFactor[0]= 43;
	itemInfo.MagicProtection[1]= PROTECTION_TYPE::Shockwave;
	itemInfo.MagicProtectionFactor[1]= 21;
	itemInfo.MagicProtection[2]= PROTECTION_TYPE::Rot;
	itemInfo.MagicProtectionFactor[2]= 100;
	itemInfo.DesertMagicResistance= 133;
	itemInfo.ForestMagicResistance= 500;
	itemInfo.PrimaryRootMagicResistance= 341;
	itemInfo.Hp= 66;
	itemInfo.HpMax= 100;
	itemInfo.Range= 169;
	itemInfo.SapLoadCurrent= 6;
	itemInfo.SapLoadMax= 30;
	itemInfo.HitRate= 8;
	itemInfo.ProtectionFactor= 0.25;
	itemInfo.MaxSlashingProtection= 38;
	itemInfo.MaxPiercingProtection= 48;
	itemInfo.MaxBluntProtection= 58;
	itemInfo.WearEquipmentMalus= 0.31f;

	if(enchant)
	{
		itemInfo.Enchantment.Name="pipoSort";
		itemInfo.Enchantment.Bricks.resize(3);
		itemInfo.Enchantment.Bricks[0]= CSheetId("bmpa01.sbrick");
		itemInfo.Enchantment.Bricks[1]= CSheetId("bmlchea01.sbrick");
		itemInfo.Enchantment.Bricks[2]= CSheetId("bmlchmh00005.sbrick");
	}

	switch(rand()%4)
	{
	case 0:
		break;
	case 1:
		{
			itemInfo.CastingSpeedFactor[1]= 1.0f;
			itemInfo.MagicPowerFactor[1]= 0.2f;
		}
		break;
	case 2:
		{
			itemInfo.CastingSpeedFactor[0]= 0.4f;
			itemInfo.MagicPowerFactor[0]= 0.2f;
			itemInfo.CastingSpeedFactor[2]= 0.8f;
			itemInfo.MagicPowerFactor[2]= 0.3f;
		}
		break;
	case 3:
		{
			itemInfo.CastingSpeedFactor[0]= 0.3f;
			itemInfo.MagicPowerFactor[0]= 0.3f;
			itemInfo.CastingSpeedFactor[1]= 0.3f;
			itemInfo.MagicPowerFactor[1]= 0.3f;
			itemInfo.CastingSpeedFactor[2]= 0.3f;
			itemInfo.MagicPowerFactor[2]= 0.3f;
			itemInfo.CastingSpeedFactor[3]= 0.3f;
			itemInfo.MagicPowerFactor[3]= 0.3f;
		}
		break;
	};


	getInventory().onReceiveItemInfo(itemInfo);

	return true;
}

NLMISC_COMMAND(debugItemInfoWaiters, "log ItemInfoWaiters", "")
{
	getInventory().debugItemInfoWaiters();

	return true;
}

NLMISC_COMMAND(debugInfoWindows, "log info windows sheetId", "")
{
	CInterfaceHelp::debugOpenedInfoWindows();

	return true;
}

NLMISC_COMMAND(getSkillValue, "get a skill value by its name", "skill_name")
{
	if (args.size() != 1) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	skillId= (uint) SKILLS::toSkill(args[0]);
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:SKILL", skillId), false);
	if(node)
	{
		pIM->displaySystemInfo(ucstring(toString(node->getValue32())));
	}

	return true;
}

NLMISC_COMMAND(setSkillValue, "set a skill value by its name", "skill_name value")
{
	if (args.size() != 2) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	skillId= (uint) SKILLS::toSkill(args[0]);
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:SKILL", skillId), false);
	if(node)
	{
		sint32 value;
		fromString(args[1], value);
		node->setValue32(value);
	}

	return true;
}

NLMISC_COMMAND(getBaseSkillValue, "get a baseskill value by its name", "skill_name")
{
	if (args.size() != 1) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	skillId= (uint) SKILLS::toSkill(args[0]);
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:BaseSKILL", skillId), false);
	if(node)
	{
		pIM->displaySystemInfo(ucstring(toString(node->getValue32())));
	}

	return true;
}

NLMISC_COMMAND(setBaseSkillValue, "set a baseskill value by its name", "skill_name value")
{
	if (args.size() != 2) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	skillId= (uint) SKILLS::toSkill(args[0]);
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:BaseSKILL", skillId), false);
	if(node)
	{
		sint32 value;
		fromString(args[1], value);
		node->setValue32(value);
	}

	return true;
}

NLMISC_COMMAND(setAllSkillValue, "set all Skill and baseskill to the given value", "value")
{
	if (args.size() != 1) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	uint	value;
	fromString(args[0], value);
	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		CCDBNodeLeaf	*node;
		node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:BaseSKILL", i), false);
		if(node)
			node->setValue32(value);
		node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:SKILL", i), false);
		if(node)
			node->setValue32(value);
	}

	return true;
}

NLMISC_COMMAND(setEntityName, "set a entity name id", "entitySlot nameId")
{
	if (args.size() != 2) return false;
	uint slot;
	fromString(args[0], slot);

	CCharacterCL	*entity= dynamic_cast<CCharacterCL*>(EntitiesMngr.entity(slot));
	if(entity)
	{
		uint32 nameId;
		fromString(args[1], nameId);
		entity->debugSetNameId(nameId);
	}
	return true;
}

NLMISC_COMMAND(reloadWeather, "reload the weather sheets", "")
{
	if (!args.empty()) return false;
	ContinentMngr.reloadWeather();
	return true;
}


// Common method to reload sheets (hope it works later...)
template<class T>
bool	reloadSheets(string filter, string wildcardFilter)
{
	CSheetManager sheetManager;
	std::vector<std::string> filters;
	filters.push_back(filter);
	ClientSheetsStrings.memoryUncompress();
	NLMISC::IProgressCallback progress;
	sheetManager.loadAllSheetNoPackedSheet(progress, filters, wildcardFilter);
	ClientSheetsStrings.memoryCompress();
	// copy sheets into current sheet manager (because numerous ptr are kept on the previous sheets in various places)
	const CSheetManager::TEntitySheetMap &sheetMap= sheetManager.getSheets();
	CSheetManager::TEntitySheetMap::const_iterator	it;
	for(it=sheetMap.begin();it!=sheetMap.end();it++)
	{
		T *dest = dynamic_cast<T*>(SheetMngr.get(it->first));
		if (dest)
		{
			const T *src = dynamic_cast<T*>(it->second.EntitySheet);
			if (src)
			{
				*dest = *src;
			}
		}
	}
	return true;
}

std::string	extendWildcard(const std::string &in)
{
	string	out;
	// append * at begin if not present (or if enp
	if(in.empty() || in[0]!='*')
		out= '*';
	out+= in;
	// append .* at end if no . found
	if(in.find('.')==string::npos)
		out+= ".*";
	return out;
}

// macros to reload Sheets
#define CMD_RELOAD_SHEET(_cmd_name, _filter, _type)		\
NLMISC_COMMAND(_cmd_name, #_cmd_name, "")				\
{														\
	if (args.size()>1) return false;	\
	string		wildcardFilter;			\
	if (args.size()>=1)					\
		wildcardFilter= extendWildcard(args[0]);		\
	return reloadSheets<_type>(_filter, wildcardFilter);				\
}
// Important ones
CMD_RELOAD_SHEET(reloadCreature, "creature", CCharacterSheet)
CMD_RELOAD_SHEET(reloadSbrick, "sbrick", CSBrickSheet)
CMD_RELOAD_SHEET(reloadSphrase, "sphrase", CSPhraseSheet)
CMD_RELOAD_SHEET(reloadSitem, "sitem", CItemSheet)
// Not tested ones
/*
CMD_RELOAD_SHEET(reloadPlayer, "player", CPlayerSheet)
CMD_RELOAD_SHEET(reloadFx, "fx", CFXSheet)
CMD_RELOAD_SHEET(reloadBuilding, "building", CBuildingSheet)
CMD_RELOAD_SHEET(reloadDeath_impact, "death_impact", CPactSheet)
CMD_RELOAD_SHEET(reloadMission, "mission", CMissionSheet)
CMD_RELOAD_SHEET(reloadRace_stats, "race_stats", CRaceStatsSheet)
CMD_RELOAD_SHEET(reloadLight_cycle, "light_cycle", CLightCycleSheet)
CMD_RELOAD_SHEET(reloadContinent, "continent", CContinentSheet)
CMD_RELOAD_SHEET(reloadWorld, "world", CWorldSheet)
CMD_RELOAD_SHEET(reloadMission_icon, "mission_icon", CMissionIconSheet)
CMD_RELOAD_SHEET(reloadSkill_tree, "skill_tree", CSkillsTreeSheet)
CMD_RELOAD_SHEET(reloadTitles, "titles", CUnblockTitlesSheet)
CMD_RELOAD_SHEET(reloadSucces_chances_table, "succes_chances_table", CSuccessTableSheet)
CMD_RELOAD_SHEET(reloadAutomaton_list, "automaton_list", CAutomatonListSheet)
CMD_RELOAD_SHEET(reloadAnimset_list, "animset_list", CAnimationSetListSheet)
CMD_RELOAD_SHEET(reloadAnimation_fx, "animation_fx", CAnimationFXSheet)
CMD_RELOAD_SHEET(reloadEmot, "emot", CEmotListSheet)
CMD_RELOAD_SHEET(reloadForage_source, "forage_source", CForageSourceSheet)
CMD_RELOAD_SHEET(reloadText_emotes, "text_emotes", CTextEmotListSheet)
*/

NLMISC_COMMAND(vprop, "Flush the Visual Property (local only). you must write to the DB before (but if you give the value in the 3rd arg)", "slot propId [val]")
{
	if(args.size()!=2 && args.size()!=3) return false;
	uint	slot;
	fromString(args[0], slot);
	uint	propId;
	fromString(args[1], propId);

	// set value in the DB?
	if(args.size()==3)
	{
		sint64 val= 0;
		fromString(args[2], val);
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", slot, propId), false);
		if(node)
			node->setValue64(val);
	}

	EntitiesMngr.updateVisualProperty(0, slot, propId);

	return true;
}

NLMISC_COMMAND(dataSetId, "Set the UID of an entity", "slot uid")
{
	if(args.size()!=2) return false;
	uint	slot;
	fromString(args[0], slot);
	uint	uid;
	fromString(args[1], uid);

	CEntityCL	*entity= EntitiesMngr.entity(slot);
	if(!entity)
		return false;

	entity->dataSetId(uid);

	return true;
}

NLMISC_COMMAND(forceDisplayFXBBoxes, "Force to display bboxes of all fxs", "0=off, 1=off")
{
	if (args.size() != 1) return false;
	bool on;
	fromString(args[0], on);
	UParticleSystemInstance::forceDisplayBBox(on);
	return true;
}

NLMISC_COMMAND(dumpVillages, "Dump villages loading zones in a bitmap", "filename>")
{
	if (args.size() != 1) return false;
	ContinentMngr.cur()->dumpVillagesLoadingZones(args[0]);
	return true;
}

NLMISC_COMMAND(dumpFogDayMap, "Dump fog day map", "filename>")
{
	if (args.size() != 1) return false;
	ContinentMngr.cur()->dumpFogMap(CFogMapBuild::Day, args[0]);
	return true;
}


NLMISC_COMMAND(dumpFogDepthMap, "Dump fog depth map", "filename>")
{
	if (args.size() != 1) return false;
	ContinentMngr.cur()->dumpFogMap(CFogMapBuild::Depth, args[0]);
	return true;
}

NLMISC_COMMAND(dumpFogDistMap, "Dump fog depth map", "filename>")
{
	if (args.size() != 1) return false;
	ContinentMngr.cur()->dumpFogMap(CFogMapBuild::Distance, args[0]);
	return true;
}

NLMISC_COMMAND(dumpRainMap, "Dump fog rain map", "filename>")
{
	if (args.size() != 1) return false;
	CRGBA colorLookup[256];
	for(uint8 k = 1; k < 255; ++k)
	{
		colorLookup[k] = CRGBA(k, k, k, 1);
	}
	colorLookup[0] = CRGBA::Red;
	colorLookup[255] = CRGBA::Blue;
	ContinentMngr.cur()->dumpFogMap(CFogMapBuild::NoPrecipitation, args[0], CContinent::ChannelR, colorLookup);
	return true;
}


NLMISC_COMMAND(stick_log, "", "<slot>")
{
	if(args.size()!=1)
		return false;
	CLFECOMMON::TCLEntityId	slot;
	fromString(args[0], slot);

	// Compute the position.
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if(!entity)
		return false;

	USkeleton	*skel= entity->skeleton();
	if(skel)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		nlinfo("Skel Log: %s", skel->getShapeName().c_str());

		std::vector<UTransform>	sticks;
		skel->getStickedObjects(sticks);

		nlinfo("StickedModels: %d", sticks.size());
		pIM->displaySystemInfo(ucstring(toString("StickedModels: %d", sticks.size())));

		for(uint i=0;i<sticks.size();i++)
		{
			UInstance	inst;
			inst.cast(sticks[i]);
			if(!inst.empty())
			{
				string	str= toString("  %d: %X. %s", i, inst.getObjectPtr(), inst.getShapeName().c_str());
				nlinfo(str.c_str());
				pIM->displaySystemInfo(str);
			}
			else
			{
				string	str= toString("  %d: %X. NOT a TransformShape", i, sticks[i].getObjectPtr());
				nlinfo(str.c_str());
				pIM->displaySystemInfo(str);
			}
		}
	}

	return true;
}

NLMISC_COMMAND(print_sys, "", "<cat> <str>")
{
	if(args.size()<1)
		return false;
	string	cat= args[0];
	string	str;
	for (uint i = 1; i < args.size(); i++)
	{
		str += args[i] + " ";
	}

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->displaySystemInfo(str, cat);

	return true;
}

NLMISC_COMMAND(fillAllInfoVersion, "", "<version>")
{
	if(args.size()!=1)
		return false;

	uint	i,j;
	uint	ver;
	fromString(args[0], ver);
	CInventoryManager	&im= getInventory();

	// BAG
	for(i=0;i<MAX_BAGINV_ENTRIES;i++)
		im.getServerBagItem(i).setInfoVersion(ver);

	// PACK_ANIMAL
	for(j=0;j<MAX_PACK_ANIMAL;j++)
	{
		for(i=0;i<MAX_ANIMALINV_ENTRIES;i++)
			im.getServerPAItem(j,i).setInfoVersion(ver);
	}

	// EXCHANGE
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	for(i=0;i<CPlayerTrade::NumTradeSlot;i++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:EXCHANGE:GIVE:%d:INFO_VERSION", i), false);
		if(node)
			node->setValue32(ver);
		node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:EXCHANGE:RECEIVE:%d:INFO_VERSION", i), false);
		if(node)
			node->setValue32(ver);
	}

	return true;
}

NLMISC_COMMAND(fullFillInventory, "", "dbstring sheetName")
{
	if(args.size()!=2)
		return false;

	// read value
	sint64	value;
	if (isalpha(args[1][0]))
	{
		CSheetId sheet(args[1]);
		value = (sint64) sheet.asInt();
	}
	else
	{
		// Convert the string into an sint64.
		fromString(args[1], value);
	}

	// read db dest
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeBranch	*nb= NLGUI::CDBManager::getInstance()->getDbBranch(args[0]);
	if(!nb)
		return false;

	uint	num= nb->getNbNodes();
	for(uint i=0;i<num;i++)
	{
		CCDBNodeLeaf	*nl;
		nl= NLGUI::CDBManager::getInstance()->getDbProp(args[0]+":"+toString(i)+":SHEET", false);
		if(nl)
		{
			nl->setValue64(value);
			nl= NLGUI::CDBManager::getInstance()->getDbProp(args[0]+":"+toString(i)+":QUALITY", false);
			if(nl)
				nl->setValue64(i);
			nl= NLGUI::CDBManager::getInstance()->getDbProp(args[0]+":"+toString(i)+":PREREQUISIT_VALID", false);
			if(nl)
				nl->setValue64(1);
		}
	}

	return true;
}

NLMISC_COMMAND(fillAllItemPreReq, "", "dbstring value")
{
	if(args.size()!=2)
		return false;

	// read value
	sint32	value;
	fromString(args[1], value);
	string	dbBase= args[0];

	// write prop for all elements of the branch
	uint	index= 0;
	for(;;)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:PREREQUISIT_VALID", dbBase.c_str(), index), false);
		if(!node)
			break;
		node->setValue32(value);
		index++;
	}

	return true;
}

NLMISC_COMMAND(eventMusic, "", "music")
{
	if(args.size()<1 && args.size()>3)
		return false;

	string	fileName= args[0].c_str();
	bool	loop= false;
	if(args.size() > 1)
		fromString(args[1], loop);
	uint	fadeTime= 1000;
	if(args.size() > 2)
		fromString(args[2], fadeTime);

	if(SoundMngr)
		SoundMngr->playEventMusic(fileName, fadeTime, loop);

	return true;
}


NLMISC_COMMAND(setLightHour, "force the light hour, (negative value to reset to auto)", "<hour>")
{
	if( args.size()!=1 )
		return false;

	float hour;
	fromString(args[0], hour);
	if( hour < LightCycleManager.getLightDesc().NumHours )
	{
		// check for privileges if this is the final build
		#if FINAL_VERSION
				// test that user has privilege
				if (hasPrivilegeDEV() ||
					hasPrivilegeSGM() ||
					hasPrivilegeGM() ||
					hasPrivilegeVG() ||
					hasPrivilegeSG() ||
					hasPrivilegeG() ||
					hasPrivilegeEM() ||
					hasPrivilegeEG())
		#endif
		{
				ForcedDayNightCycleHour = hour;
				return true;
		}
	}
	return false;
}



NLMISC_COMMAND(jobAnim, "set the job anim specialisation of an entity", "eid number")
{
	if(args.size()!=2)
		return false;

	uint	eid;
	fromString(args[0], eid);
	uint	jas;
	fromString(args[1], jas);
	CCharacterCL	*ent= dynamic_cast<CCharacterCL*>(EntitiesMngr.entity(eid));
	if(ent)
	{
		ent->setAnimJobSpecialisation(jas);
	}

	return true;
}

NLMISC_COMMAND(startLogStageChange, "start to log the change of stages of watched entity", "")
{
	// stop first any log
	EntitiesMngr.stopLogStageChange();

	// start the log
	EntitiesMngr.startLogStageChange(NetMngr.getCurrentClientTick(), T1);

	return true;
}

NLMISC_COMMAND(stopLogStageChange, "stop to log the change of watched entity stages", "")
{
	EntitiesMngr.stopLogStageChange();
	return true;
}

NLMISC_COMMAND(testReceiveMissionInfo, "emulate a dummy receive of mission info", "")
{
	CBotChatManager::getInstance()->debugLocalReceiveMissionInfo();
	return true;
}

// command to dump the ui, no indentation full name
NLMISC_COMMAND(dumpUIIndent, "Debug only : Dump the ui hierarchy in the output debug window", "")
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->dumpUI(true);
	return true;
}

// command to dump the ui, no indentation full name
NLMISC_COMMAND(dumpUIFull, "Debug only : Dump the ui hierarchy in the output debug window", "")
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->dumpUI(false);
	return true;
}

// command to dump coordinates of a UI, for debug
NLMISC_COMMAND(dumpUICoords, "Debug only : dump all coords info of an UI", "uiid")
{
	if(args.size()!=1)
		return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceElement	*el= CWidgetManager::getInstance()->getElementFromId(args[0]);
	if(!el)
	{
		pIM->displaySystemInfo(toString("dumpUICoords: '%s' does not exist", args[0].c_str()));
	}
	else
	{
		pIM->displaySystemInfo(toString("dumpUICoords: **** '%s'", args[0].c_str()));
		pIM->displaySystemInfo(toString("    active= %d", uint(el->getActive()) ));
		pIM->displaySystemInfo(toString("    x= %d", el->getX() ));
		pIM->displaySystemInfo(toString("    y= %d", el->getY() ));
		pIM->displaySystemInfo(toString("    w= %d", el->getW() ));
		pIM->displaySystemInfo(toString("    h= %d", el->getH() ));
		pIM->displaySystemInfo(toString("    xreal= %d", el->getXReal() ));
		pIM->displaySystemInfo(toString("    yreal= %d", el->getYReal() ));
		pIM->displaySystemInfo(toString("    wreal= %d", el->getWReal() ));
		pIM->displaySystemInfo(toString("    hreal= %d", el->getHReal() ));
		pIM->displaySystemInfo(toString("    parent= '%s'", el->getParent()?el->getParent()->getId().c_str():""));
		pIM->displaySystemInfo(toString("    parentpos= '%s'", el->getParentPos()?el->getParentPos()->getId().c_str():""));
		pIM->displaySystemInfo(toString("    parentsize= '%s'", el->getParentSize()?el->getParentSize()->getId().c_str():""));

		// SizeRef
		string	sr;
		switch(el->getSizeRef())
		{
		case 1: sr= "w"; break;
		case 2: sr= "h"; break;
		case 3: sr= "wh"; break;
		default:break;
		}
		pIM->displaySystemInfo(toString("    sizeref= '%s'", sr.c_str()));

		// PosRef
		string pr;
		THotSpot	hsParent= el->getParentPosRef();
		THotSpot	hsSelf= el->getPosRef();
		// parent
		if(hsParent & Hotspot_Bx)	pr+= "B";
		else if(hsParent & Hotspot_Mx)	pr+= "M";
		else if(hsParent & Hotspot_Tx)	pr+= "T";
		else pr+= "?";
		if(hsParent & Hotspot_xL)	pr+= "L";
		else if(hsParent & Hotspot_xM)	pr+= "M";
		else if(hsParent & Hotspot_xR)	pr+= "R";
		else pr+= "?";
		pr+=" ";
		// self
		if(hsSelf & Hotspot_Bx)	pr+= "B";
		else if(hsSelf & Hotspot_Mx)	pr+= "M";
		else if(hsSelf & Hotspot_Tx)	pr+= "T";
		else pr+= "?";
		if(hsSelf & Hotspot_xL)	pr+= "L";
		else if(hsSelf & Hotspot_xM)	pr+= "M";
		else if(hsSelf & Hotspot_xR)	pr+= "R";
		else pr+= "?";
		pIM->displaySystemInfo(toString("    posref= '%s'", pr.c_str()));

		pIM->displaySystemInfo(string("dumpUICoords: **** END"));
	}

	return true;
}

// Command to clear the dump of done Files opened and Async File Manager done files (for debug)
NLMISC_COMMAND(clearDumpFiles, "clear the CAsyncFileManager and CIFile Debug list of opened files", "")
{
	CIFile::clearDump();
	CAsyncFileManager::getInstance().clearDump();

	return true;
}

#endif // FINAL_VERSION


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////// COMMANDS before should NOT appear IN the FINAL VERSION //////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

NLMISC_COMMAND(reloadFogMaps, "Force to reload all the fog maps", "<>")
{
	if (!args.empty()) return false;
	ContinentMngr.cur()->reloadFogMap();
	return true;
}

// dump the names of all loaded sounds
NLMISC_COMMAND(dumpSounds, "Dump names of all loaded sound", "<>")
{
	if (!args.empty()) return false;
	std::vector<NLMISC::CSheetId> sounds;
	extern CSoundManager	*SoundMngr;
	if (!SoundMngr) return false;
	if (!SoundMngr->getMixer()) return false;
	SoundMngr->getMixer()->getSoundNames(sounds);
	for(uint k = 0; k < sounds.size(); ++k)
	{
		nlinfo(sounds[k].toString()/*NLMISC::CStringMapper::unmap(sounds[k])*/.c_str());
	}
	return true;
}

// ***************************************************************************
// LUA
// ***************************************************************************
const char	*LUADebugNotEnabledMsg= "Lua Commands are available only if you add 'AllowDebugLua= 1;' in your client.cfg";

NLMISC_COMMAND(luaReload, "reload all .lua script files", "")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.AllowDebugLua)
	{
		CWidgetManager::getInstance()->getParser()->reloadAllLuaFileScripts();
		return true;
	}
	else
	{
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(LUADebugNotEnabledMsg));
		return false;
	}
}

NLMISC_COMMAND(luaScript, "Execute a lua script", "direct_script_code")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.AllowDebugLua)
	{
		if(args.size()<1)
			return false;

		// Concat list of string in one script
		string	script;
		for(uint i=0;i<args.size();i++)
		{
			script+= args[i] + " ";
		}

		// not smallScript because suppose var can change a lot
		CLuaManager::getInstance().executeLuaScript(script, false);

		return true;
	}
	else
	{
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(LUADebugNotEnabledMsg));
		return false;
	}
}

NLMISC_COMMAND(luaInfo, "Dump some information on LUA state", "detaillevel from 0 to 2")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.AllowDebugLua)
	{
		if(args.size()!=1)
			return false;

		uint	detail;
		fromString(args[0], detail);

		pIM->dumpLuaState(detail);
		return true;
	}
	else
	{
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(LUADebugNotEnabledMsg));
		return false;
	}
}

NLMISC_COMMAND(luaObject, "Dump the content of a lua object", "<table name> [maxDepth = 20, 0 for no limits]")
{
	if (args.empty()) return false;
	if (args.size() > 2) return false;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if (!ClientCfg.AllowDebugLua)
	{
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(LUADebugNotEnabledMsg));
		return false;
	}
	CLuaState *luaState = CLuaManager::getInstance().getLuaState();
	if (!luaState) return false;
	CLuaStackChecker lsc(luaState);
	// get the table
	static const char *inspectedTable = "_____inspected_table";
	try
	{
		// make a reference to the table to be inspected (is this this a primitive type, just make a copy)
		luaState->executeScript(std::string(inspectedTable) + " = " + args[0]);
	}
	catch(const ELuaError &e)
	{
		CLuaIHMRyzom::debugInfo(e.what());
		return false;
	}
	luaState->pushGlobalTable();
	CLuaObject env;
	env.pop(*luaState);
	uint maxDepth;
	if (args.size() > 1)
		fromString(args[1], maxDepth);
	else
		maxDepth = 20;
	//CLuaIHM::debugInfo(env[inspectedTable].toStringRecurse(0, maxDepth));
	env[inspectedTable].dump();
	env.eraseValue(inspectedTable);
	return true;
}




// GC allowed only in Dev version
#if !FINAL_VERSION
NLMISC_COMMAND(luaGC, "Force a garbage collector of lua", "")
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if(ClientCfg.AllowDebugLua)
	{
		CLuaManager::getInstance().forceGarbageCollect();
		return true;
	}
	else
	{
		pIM->displaySystemInfo( LuaHelperStuff::formatLuaErrorSysInfo(LUADebugNotEnabledMsg));
		return false;
	}
}
#endif


// ***************************************************************************
// CUserCommand
// ***************************************************************************

std::map<std::string, CUserCommand*> CUserCommand::CommandMap;

// release memory
void CUserCommand::release()
{
	std::map<std::string, CUserCommand*>::iterator it = CommandMap.begin();
	while( it != CommandMap.end() )
		delete (*it++).second;
	CommandMap.clear();
}

// ***************************************************************************

CUserCommand::CUserCommand(const string &commandName, const ucstring &help, const ucstring &argsHelp)
		: ICommand("user", commandName.c_str(), toString(help).c_str(), toString(argsHelp).c_str())
{
	CommandName  = commandName;
}

// ***************************************************************************

void CUserCommand::addMode (const string &action, uint numArg, bool infiniteAgr, const std::vector<string> &keywords)
{
	CMode *mode;
	if (!infiniteAgr)
		mode = &(FixedArgModes[numArg]);
	else
		mode = &InfiniteMode;
	mode->Action = action;
	mode->KeywordsCount = numArg;
	mode->Keywords = keywords;
}

// ***************************************************************************

bool CUserCommand::execute(const std::string &/* rawCommandString */, const std::vector<std::string> &args, NLMISC::CLog &/* log */, bool /* quiet */, bool /* human */)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Find the good keyword table
	CMode *mode = NULL;
	if (FixedArgModes.find ((uint)args.size()) != FixedArgModes.end())
		mode = &(FixedArgModes[(uint)args.size()]);
	else
		if (!InfiniteMode.Keywords.empty() && (args.size() >= InfiniteMode.KeywordsCount))
			mode = &InfiniteMode;

	if (mode)
	{
		// Build the final string
		static string finalArgs;
		finalArgs = "";
		uint i;
		uint index = 0;
		const vector<string> &keywords = mode->Keywords;
		for (i=0; i<keywords.size(); i++)
		{
			if ((keywords[i] == "$") || (keywords[i] == "+"))
			{
				if ((uint)index >= args.size())
				{
					// Not enough arguments
					pIM->displaySystemInfo (ucstring(CommandName+" : ")+CI18N::get ("uiCommandWrongArgumentCount"));
					return false;
				}
				else
				{
					if (keywords[i] == "$")
						finalArgs += /*ucstring(*/args[index++]/*).toUtf8()*/;
					else
					{
						while (index<args.size())
						{
							finalArgs += (index > 0) ? " " : "";
							// If arg contains spaces then put it in quotes
							if (string::npos != args[index].find(" "))
							{
								finalArgs += "\"" + args[index++] + "\"";
							}
							else 
							{
								finalArgs += args[index++];
							}
						}
					}
				}
			}
			else
			{
				finalArgs += keywords[i];
			}
		}

		// Run the action handler
		CAHManager::getInstance()->runActionHandler (mode->Action, CWidgetManager::getInstance()->getOldCaptureKeyboard(), finalArgs);
	}
	else
	{
		// Not enough argument
		pIM->displaySystemInfo (ucstring(CommandName+" : ")+CI18N::get ("uiCommandWrongArgumentCount"));
		return false;
	}
	return true;
}

// ***************************************************************************

void CUserCommand::createCommand (const char *name, const char *action, const char *ptrParams)
{
	// Parse the params
	std::vector<string> keywords;

	// Get the help and the argument help
	uint countArgument = 0;
	bool infiniteArgument = false;
	string params = ptrParams;
	string::size_type pos = 0;
	while (pos < params.size())
	{
		// Token ?
		string::size_type last;
		switch (params[pos])
		{
		case '|':
		case '=':
		case '$':
		case '+':
			if ((params[pos] == '$') || (params[pos] == '+'))
				countArgument++;
			if (params[pos] == '+')
				infiniteArgument = true;
			last = pos+1;
			break;
		default:
			last = params.find_first_of ("|=$+", pos);
			if (last == string::npos)
				last = params.size();
			break;
		}

		// Add a keyword
		keywords.push_back (params.substr (pos, last-pos));
		pos = last;
	}

	// Find action name
	ucstring help;
	const CBaseAction *ab = Actions.getBaseAction (::CAction::CName (action, ptrParams));
	if (ab)
		help = CI18N::get(ab->LocalizedName);

	// Build a argument help
	ucstring argsHelp;

	if (ab)
	{
		// Look for each arguments
		uint i;
		for (i=0; i<keywords.size(); i++)
		{
			// Look for a '$'
			if ((keywords[i] == "$") || (keywords[i] == "+"))
			{
				// Have a "=" ?
				if ((i > 1) && (keywords[i-1]=="="))
				{
					// Argument
					bool bFound = false;
					for (uint j=0; j<ab->Parameters.size(); j++)
					{
						// Argument found
						if (ab->Parameters[j].Name == keywords[i-2])
						{
							// Add the string
							if (!argsHelp.empty())
								argsHelp += " ";
							argsHelp += ucstring("<") + CI18N::get(ab->Parameters[j].LocalizedName) + ucstring(">");
							bFound = true;
						}
					}
					// Not found ? Warning
					if (!bFound)
					{
						nlwarning ("Argument %s not found in command %s using action %s", keywords[i-2].c_str(), name, action);
					}
				}
			}
		}
	}

	// Ugly : never deleted, but who cares ?
	// Command exist ?
	CUserCommand *currentCommand;
	if (CommandMap.find (name) != CommandMap.end())
		currentCommand = CommandMap[name];
	else
	{
		currentCommand = new CUserCommand (name, help, argsHelp);
		CommandMap[name] = currentCommand;
	}

	// Add keywords
	currentCommand->addMode (action, countArgument, infiniteArgument, keywords);
}

NLMISC_COMMAND(doAssert, "Create an assert", "")
{
	nlassert(0);
	return true;
}

NLMISC_COMMAND(dumpPosAsPrim, "ld helper : add current position to pos.primitive with the given comment", "")
{
	if (!EntitiesMngr.entity(0)) return false;
	std::string comment;
	for(uint k = 0; k < args.size(); ++k)
	{
		if (k != 0) comment += " ";
		comment += args[k];
	}
	std::string srcFile;
	const std::string path = "save/pos.primitive";
	if (CFile::fileExists(path))
	{
		try
		{
			uint32 fileSize = CFile::getFileSize(path);
			srcFile.resize(fileSize);
			CIFile stream;
			stream.open(path);
			stream.serialBuffer((uint8 *) &srcFile[0], fileSize);
		}
		catch(const NLMISC::EStream &e)
		{
			nlinfo(e.what());
			srcFile.clear();
		}
	}
	std::string newPrim =
	NLMISC::toString("<CHILD TYPE=\"CPrimPoint\">            \n\
						   <PT X=\"%f\" Y=\"%f\" Z=\"%f\"/>  \n\
								<PROPERTY TYPE=\"string\">   \n\
									<NAME>class</NAME>       \n\
									<STRING>plot</STRING>    \n\
							   </PROPERTY>                   \n\
							   <PROPERTY TYPE=\"string\">    \n\
									<NAME>name</NAME>        \n\
									<STRING>%s</STRING>      \n\
							   </PROPERTY>                   \n\
						  </CHILD>\n", (float) EntitiesMngr.entity(0)->pos().x, (float) EntitiesMngr.entity(0)->pos().y, (float) EntitiesMngr.entity(0)->pos().z, comment.c_str());

	// try to append result to current file
	const std::string LAST_CHILD_MARKER = "</CHILD>";
	std::string::size_type insertPos = srcFile.rfind(LAST_CHILD_MARKER);
	if (insertPos != std::string::npos)
	{
		insertPos += LAST_CHILD_MARKER.size();
		srcFile.insert(insertPos, "\n" + newPrim);
	}
	else
	{
		srcFile.clear();
	}
	if (srcFile.empty())
	{
		srcFile = "<?xml version=\"1.0\"?>                        \n\
					    <PRIMITIVES VERSION=\"1\">                \n\
                             <ROOT_PRIMITIVE TYPE=\"CPrimNode\">  \n"
				   + newPrim
				   + "    </ROOT_PRIMITIVE> \n\
					  </PRIMITIVES>\n";
	}

	// write result
	try
	{
		COFile stream;
		stream.open(path);
		stream.serialBuffer((uint8 *) &srcFile[0], (uint)srcFile.size());
	}
	catch(const NLMISC::EStream &e)
	{
		nlinfo(e.what());
	}
	return true;
}

NLMISC_COMMAND(clear, "clear content of current char window", "<chat window caller id>")
{
	if (args.size() > 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->flushDebugWindow();
	CChatWindow *cw;
	if (args.size() == 1)
	{
		cw = getChatWndMgr().getChatWindowFromCaller(dynamic_cast<CCtrlBase *>(CWidgetManager::getInstance()->getElementFromId(args[0])));
	}
	else
	{
		// if no chat window (or enclosed control) id is given then see if a chat window called this command
		cw = CChatWindow::getChatWindowLaunchingCommand();
	}
	if (cw && cw->getContainer())
	{
		CGroupList *gl = dynamic_cast<CGroupList *>(cw->getContainer()->getGroup("text_list"));
		if (gl)
		{
			gl->deleteAllChildren();
		}
	}
	return true;
}

NLMISC_COMMAND(dumpAllLoadedZones, "dump all loaded zones", "")
{
	if (!args.empty()) return false;
	if (Landscape)
	{
		std::vector<std::string>	loadedZones;
		Landscape->getAllZoneLoaded(loadedZones);
		for(uint k = 0; k < loadedZones.size(); ++k)
		{
			nlwarning(loadedZones[k].c_str());
		}
	}
	else
	{
		nlwarning("Landscape has no loaded zones ");
	}
	return true;
}


NLMISC_COMMAND(tickToDate, "convert a tick value into a readable ryzom time", "")
{
	if (args.size() != 1) return false;
	CRyzomTime rt;
	uint32 tick;
	fromString(args[0], tick);
	rt.updateRyzomClock(tick);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	float ryTime = rt.getRyzomTime();
	std::string readableDate = toString("Day = %d, hour = %d:%d", rt.getRyzomDay(), (int) floorf(ryTime), (int) floorf(60.f * fmodf(ryTime, 1.f)));
	im->displaySystemInfo(ucstring(readableDate));
	return true;
}


NLMISC_COMMAND(dumpShapeMaxDist, "dump max dist for shapes", "")
{
/*
	std::set<std::string> shapeSet;
	typedef CHashMultiMap<float, std::string> TShapeMap;
	TShapeMap shapes;
	const CSheetManager::TEntitySheetMap &sheets = SheetMngr.getSheets();
	std::vector<const CCharacterSheet::CEquipment*> equipList;
	for (CSheetManager::TEntitySheetMap::const_iterator it = sheets.begin(); it != sheets.end(); ++it)
	{
		CCharacterSheet *cs = dynamic_cast<CCharacterSheet *>(SheetMngr.get(it->first));
		if (cs)
		{
			equipList.clear();
			cs->getWholeEquipmentList(equipList);
			for (uint k = 0; k < equipList.size(); ++k)
			{
				std::string item = toLower(equipList[k]->getItem());
				if (!item.empty())
				{

					string ext = CFile::getExtension(item);
					if (ext == "shape")
					{
						if (!shapeSet.count(item))
						{
							UInstance inst = Scene->createInstance(item);
							if (!inst.empty())
							{
								shapes.insert(make_pair(inst.getDistMax(), item));
								Scene->deleteInstance(inst);
							}
							shapeSet.insert(item);
						}
					}
				}
			}
		}
	}
	for (TShapeMap::iterator it = shapes.begin(); it != shapes.end(); ++it)
	{
		nlwarning("Dist = %f, shape = %s", it->first, it->second.c_str());
	}
*/
	return true;
}


NLMISC_COMMAND(dumpContinentCorners, "dump max dist for shapes", "")
{
	if (!args.empty()) return false;
	if (!ContinentMngr.cur()) return false;

	CVector2f posMin;
	CVector2f posMax;
	getPosFromZoneName(ContinentMngr.cur()->ZoneMin, posMin);
	getPosFromZoneName(ContinentMngr.cur()->ZoneMax, posMax);
	if (posMin.x > posMax.x) std::swap(posMin.x, posMax.x);
	if (posMin.y > posMax.y) std::swap(posMin.y, posMax.y);
	posMax.x += 160.f;
	posMax.y += 160.f;
	nlwarning("min = (%f, %f), max = (%f, %f)", posMin.x, posMin.y, posMax.x, posMax.y);
	return true;
}


#if !FINAL_VERSION
NLMISC_COMMAND(setMission, "locally set a mission text for test", "<mission index><text>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 2) return false;
	uint index;
	fromString(args[0], index);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	static sint32 strID = 10000; // any abitrary string id will do in local
	if (index >= 30) return false;
	if (index < 15)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:MISSIONS:%d:TITLE", (int) index))->setValue32(strID);
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:MISSIONS:%d:FINISHED", (int) index))->setValue32(0);
	}
	else
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GROUP:MISSIONS:%d:TITLE", (int) index - 15))->setValue32(strID);
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GROUP:MISSIONS:%d:FINISHED", (int) index - 15))->setValue32(0);
	}
	setDynString(strID++, args[1]);
	return true;
}

static bool setMissionStep(uint missionIndex, uint stepIndex, uint32 strID)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (stepIndex >= 30) return false;
	if (stepIndex >= 20) return false;
	if (missionIndex < 15)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:MISSIONS:%d:GOALS:%d:TEXT", (int) missionIndex, (int) stepIndex))->setValue32(strID);
	}
	else
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GROUP:MISSIONS:%d:GOALS:%d:TEXT", (int) (missionIndex - 15), (int) stepIndex))->setValue32(strID);
	}
	return true;
}


// add a new step in a mission, the mission must already exist
NLMISC_COMMAND(setMissionStep, "locally set a mission step for test", "<mission index><step index><text>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 3) return false;
	uint missionIndex;
	fromString(args[0], missionIndex);
	uint stepIndex;
	fromString(args[1], stepIndex);

	static sint32 strID = 20000; // any abitrary string id will do in local
	if (!setMissionStep(missionIndex, stepIndex, strID)) return false;
	setDynString(strID++, args[2]);
	return true;
}

// add a newstepin a mission, the mission must already exist
NLMISC_COMMAND(clearMissionStep, "locally set a mission step for test", "<mission index><step index><text>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 2) return false;
	uint missionIndex;
	fromString(args[0], missionIndex);
	uint stepIndex;
	fromString(args[1], stepIndex);
	return setMissionStep(missionIndex, stepIndex, 0);
}


#endif


#if !FINAL_VERSION
static bool debugSetMissionState(uint index, sint32 /* state */)
{
	if (index >= 30) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (index < 15)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:MISSIONS:%d:TITLE", (int) index))->setValue32(0);
	}
	else
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GROUP:MISSIONS:%d:TITLE", (int) index - 15))->setValue32(0);
	}
	return true;
}
#endif


#if !FINAL_VERSION
NLMISC_COMMAND(clearMission, "clear the content of a mission", "<mission index>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 1) return false;
	uint index;
	fromString(args[0], index);
	return debugSetMissionState(index, 0);

}
#endif

#if !FINAL_VERSION
NLMISC_COMMAND(finishMission, "clear the content of a mission", "<mission index>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 1) return false;
	uint index;
	fromString(args[0], index);
	return debugSetMissionState(index, 1);
}
#endif

#if !FINAL_VERSION
NLMISC_COMMAND(failMission, "clear the content of a mission", "<mission index>")
{
	if (!ClientCfg.Local) return false;
	if (args.size() != 1) return false;
	uint index;
	fromString(args[0], index);
	return debugSetMissionState(index, 2);
}
#endif


// ***************************************************************************



NLMISC_COMMAND(em, "emote command", "<emote phrase>")
{
	if (args.size() < 1) return false;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if( pIM )
	{
		string emotePhrase;
		if( args.size() > 0 )
		{
			emotePhrase = args[0];
		}
		for(uint i = 1; i < args.size(); ++i )
		{
			emotePhrase += " ";
			emotePhrase += args[i];
		}
		CAHManager::getInstance()->runActionHandler("emote", NULL, "nb=0|behav=255|custom_phrase="+emotePhrase);
		return true;
	}
	return false;
}




NLMISC_COMMAND(guildmotd, "Set or see the guild message of the day","<msg of the day>")
{
	CBitMemStream out;
	if (!GenericMsgHeaderMngr.pushNameToStream("COMMAND:GUILDMOTD", out))
		return false;

	string gmotd;
	if( args.size() > 0 )
	{
		gmotd = args[0];
	}
	for(uint i = 1; i < args.size(); ++i )
	{
		gmotd += " ";
		gmotd += args[i];
	}

	out.serial (gmotd);
	NetMngr.push (out);

	return true;
}


NLMISC_COMMAND(time, "Shows information about the current time", "")
{
	const uint8 size = 50;
	char cs_local[size];
	char cs_utc[size];
	time_t date;
	time(&date);
	struct tm *tm;
	tm = localtime(&date);
	strftime(cs_local, size, "%X", tm);
	tm = gmtime(&date);
	strftime(cs_utc, size, "%X", tm);

	ucstring msg = CI18N::get("uiCurrentLocalAndUtcTime");
	strFindReplace(msg, "%local", cs_local);
	strFindReplace(msg, "%utc", cs_utc);
	CInterfaceManager::getInstance()->displaySystemInfo(msg, "AROUND");
	return true;
}
