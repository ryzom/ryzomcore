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

// Interface includes
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "action_handler_tools.h"
#include "game_share/outpost.h"
#include "nel/gui/interface_expr.h"
#include "group_map.h"
#include "../sheet_manager.h"
#include "../net_manager.h"
#include "../game_context_menu.h"
#include "../user_entity.h"
#include "../entities.h"
#include "../outpost_manager.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***************************************************************************
// ***************************************************************************
//							OUTPOST HANDLERS
// ***************************************************************************
// ***************************************************************************

uint8 getOutpostSelection()
{
	uint8 nOutpost = 0;
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SELECTION",false);
	if (pNL != NULL)
		nOutpost = (uint8)pNL->getValue32();
	return nOutpost;
}

uint32 getOutpostSheet()
{
	uint32	sheet= 0;
	uint8	outpostSel= getOutpostSelection();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GUILD:OUTPOST:O%d:SHEET", outpostSel),false);
	if (pNL != NULL)
		sheet = pNL->getValue32();
	return sheet;
}

// ***************************************************************************
// Abandon an outpost
class COutpostGiveup : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// set confirm dialog flag
		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:CONFIRM_DEL_OUTPOST")->setValueBool(true);

		// Ask if ok before.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQConfirmGiveupOutpost"),
			"outpost_do_giveup", sParams, "outpost_cancel_giveup");
	}
};
REGISTER_ACTION_HANDLER(COutpostGiveup, "outpost_giveup");

class COutpostDoGiveup : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// retrieve the sheet of this outpost
		uint32	sheet= 0;
		uint8	outpostSel;
		fromString(sParams, outpostSel);
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GUILD:OUTPOST:O%d:SHEET", outpostSel),false);
		if (pNL == NULL)
			return;
		sheet = pNL->getValue32();

		// send msg to server
		sendMsgToServer("OUTPOST:GIVEUP_OUTPOST", sheet);

		// reset confirm dialog flag
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:CONFIRM_DEL_OUTPOST")->setValueBool(false);
	}
};
REGISTER_ACTION_HANDLER(COutpostDoGiveup, "outpost_do_giveup");

class COutpostCancelGiveup : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// reset confirm dialog flag
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:CONFIRM_DEL_OUTPOST")->setValueBool(false);
	}
};
REGISTER_ACTION_HANDLER(COutpostCancelGiveup, "outpost_cancel_giveup");

// ***************************************************************************
// Set a squad To buy (list of squad given by the server)
class COutpostSetSquad : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get Squad slot Destination selected
		uint8 nSquadSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_SLOT_SELECTED")->getValue8();

		// get Squad Id to buy selected
		string sLine = getParam(sParams, "line");
		CInterfaceExprValue ievLine;
		uint8 nLine = 0;
		if (!CInterfaceExpr::eval(sLine, ievLine))
		{
			nlwarning("<COutpostBuy::execute> : Can't evaluate line");
			return;
		}
		if (ievLine.getInteger() >= 0)
			nLine = (uint8)ievLine.getInteger();

		// send msg
		sendMsgToServer("OUTPOST:SET_SQUAD", getOutpostSheet(), nSquadSlot, nLine);
	}
};
REGISTER_ACTION_HANDLER(COutpostSetSquad, "outpost_set_squad");


// ***************************************************************************
// Remove a squad To buy (list of squad given by the server)
class COutpostRemoveSquad : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get Squad slot Destination selected
		uint8 nSquadSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_SLOT_SELECTED")->getValue8();

		// send msg
		sendMsgToServer("OUTPOST:REMOVE_SQUAD", getOutpostSheet(), nSquadSlot);
	}
};
REGISTER_ACTION_HANDLER(COutpostRemoveSquad, "outpost_remove_squad");

// ***************************************************************************
// Insert a squad To buy (list of squad given by the server)
class COutpostInsertSquad : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get Squad slot Destination selected
		uint8 nSquadSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_SLOT_SELECTED")->getValue8();

		// send msg
		sendMsgToServer("OUTPOST:INSERT_SQUAD", getOutpostSheet(), nSquadSlot);

		// Then set the selected squad at this place
		CAHManager::getInstance()->runActionHandler("outpost_set_squad", pCaller, sParams);
	}
};
REGISTER_ACTION_HANDLER(COutpostInsertSquad, "outpost_insert_squad");

// ***************************************************************************
// Map helper
uint8 getOutpostSquadSpawnIndex()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// get Squad slot
	uint8 nSquadSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_SLOT_SELECTED")->getValue8();

	// So we can get the spawn index stored in the DB
	string sDBPath = "SERVER:GUILD:OUTPOST:O" + toString(getOutpostSelection()) + ":SQUADS:T";
	sDBPath += toString(nSquadSlot) + ":SPAWN";
	return NLGUI::CDBManager::getInstance()->getDbProp(sDBPath)->getValue8();
}

// ***************************************************************************
class COutpostInitSquadMap : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId(sParams));
		if (pMap == NULL)
			return;

		// Get the spawn index (from selected squad)
		uint8 nSpawnIndex = getOutpostSquadSpawnIndex();

		// *** Get the BBox of the outpost
		// init map with all xy that are in SERVER:GUILD:OUTPOST:O#x:SQUAD_SPAWN_ZONE (if 0,0 not present)
		string sDBPathSZ = "SERVER:GUILD:OUTPOST:O" + toString(getOutpostSelection()) + ":SQUAD_SPAWN_ZONE:";
		sint32 xMin = 0, xMax = 0, yMin = 0, yMax = 0;
		bool bInit = false;
		uint32 i;
		for (i = 0; i < OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE; ++i)
		{
			sint32 x = NLGUI::CDBManager::getInstance()->getDbProp(sDBPathSZ + toString(i) + ":X")->getValue32();
			sint32 y = NLGUI::CDBManager::getInstance()->getDbProp(sDBPathSZ + toString(i) + ":Y")->getValue32();
			if ((x != 0) || (y != 0))
			{
				if (!bInit)
				{
					xMin = xMax = x;
					yMin = yMax = y;
					bInit = true;
				}
				else
				{
					if (x < xMin) xMin = x;
					if (x > xMax) xMax = x;
					if (y < yMin) yMin = y;
					if (y > yMax) yMax = y;
				}
			}
		}

		// *** Find the finest map that contains the 2 points xMin,yMin and xMax,yMax
		CWorldSheet *pWorldSheet = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
		if (pWorldSheet == NULL)
			return;

		sint32 nMapFound = -1;
		for (i = 0; i < pWorldSheet->Maps.size(); ++i)
		{
			SMap &rMap = pWorldSheet->Maps[i];
			// Avoid 'world' map
			if (!rMap.ContinentName.empty())
			{
				// Is bouding box contains the 2 points ?
				if ((rMap.MinX < xMin) && (rMap.MinY < yMin) &&
					(rMap.MaxX > xMax) && (rMap.MaxY > yMax))
				{
					// Is there a map already found ?
					if (nMapFound == -1)
					{
						nMapFound = i; // No, first map found
					}
					else	// Yes we have to compare with the previous map to keep the finest
					{
						SMap &rMapFound = pWorldSheet->Maps[nMapFound];
						if ((rMap.MaxX - rMap.MinX) < (rMapFound.MaxX - rMapFound.MinX))
							nMapFound = i;
					}
				}
			}
		}

		// *** Setup the Map and zoom to outpost
		if (nMapFound != -1)
		{
			SMap &rMapFound = pWorldSheet->Maps[nMapFound];
			pMap->setMap(rMapFound.Name);
			CVector2f centerWorldCoord((xMax + xMin)/2.0f, (yMin + yMax)/2.0f);
			CVector2f centerMapCoord;
			pMap->worldToMap(centerMapCoord, centerWorldCoord);
			pMap->setScale((rMapFound.MaxX - rMapFound.MinX) / max(sint32(1), (xMax - xMin)) );
			pMap->setPlayerPos(centerMapCoord);
			pMap->centerOnPlayer();

			CRespawnPointsMsg m;
			m.NeedToReset = true;
			for (i = 0; i < OUTPOSTENUMS::OUTPOST_MAX_SPAWN_ZONE; ++i)
			{
				sint32 x = NLGUI::CDBManager::getInstance()->getDbProp(sDBPathSZ + toString(i) + ":X")->getValue32();
				sint32 y = NLGUI::CDBManager::getInstance()->getDbProp(sDBPathSZ + toString(i) + ":Y")->getValue32();
				if ((x != 0) || (y != 0))
				{
					CRespawnPointsMsg::SRespawnPoint pt;
					pt.x = x * 1000;
					pt.y = y * 1000;
					m.RespawnPoints.push_back(pt);
				}
			}
			pMap->addRespawnPoints(m);
			pMap->setRespawnSelected(nSpawnIndex);
		}
	}
};
REGISTER_ACTION_HANDLER(COutpostInitSquadMap, "outpost_init_squad_map");

// ***************************************************************************
class COutpostSquadMapSend : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId(sParams));
		if (pMap == NULL)
			return;

		// Get the spawn index (from selected squad)
		uint8 nSpawnIndex = getOutpostSquadSpawnIndex();

		// Get Spawn index selected by the user
		uint8 nSpawnSelected = (uint8)pMap->getRespawnSelected();

		if (nSpawnIndex != nSpawnSelected)
		{
			// get Squad slot
			uint8 nSquadSlot = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:SQUAD_SLOT_SELECTED")->getValue8();

			sendMsgToServer("OUTPOST:SET_SQUAD_SPAWN", getOutpostSheet(), nSquadSlot, nSpawnSelected);
		}
	}
};
REGISTER_ACTION_HANDLER(COutpostSquadMapSend, "outpost_squad_map_send");


// ***************************************************************************
class COutpostSelectSquadCapital : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &params)
	{
		uint32	capital;
		fromString(params, capital);
		sendMsgToServer("OUTPOST:SET_SQUAD_CAPITAL", getOutpostSheet(), capital);
	}
};
REGISTER_ACTION_HANDLER(COutpostSelectSquadCapital, "outpost_select_squad_capital");


// ***************************************************************************
static sint		localToGmt(sint hour)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:OUTPOST:TIME_ZONE", false);
	if(node)
		hour-= node->getValue32();
	hour+= 48;
	hour%=24;

	return hour;
}
static sint		gmtToLocal(sint hour)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:OUTPOST:TIME_ZONE", false);
	if(node)
		hour+= node->getValue32();
	hour+= 48;
	hour%=24;

	return hour;
}


// ***************************************************************************
class CAHOutpostSelectDefPeriod : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// The user change the defense period (edited in LOCAL TimeZone)
		sint defPeriod;
		fromString(sParams, defPeriod);

		// Then send request to Server
		sendMsgToServer("OUTPOST:SET_DEF_PERIOD", getOutpostSheet(), uint8(localToGmt(defPeriod)));
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostSelectDefPeriod, "outpost_select_def_period");

// ***************************************************************************
class CAHOutpostSelectAttPeriod : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// The user change the attack period (edited in LOCAL TimeZone)
		sint attPeriod;
		fromString(sParams, attPeriod);

		// Store in local DB (in LOCAL for consistency)
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ATTACK_PERIOD", false);
		if(node)
			node->setValue32(localToGmt(attPeriod));

		// Nead to resend a Declare War Start (because wanted Att Hour changed)
		CAHManager::getInstance()->runActionHandler("outpost_declare_war_start", pCaller);
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostSelectAttPeriod, "outpost_select_att_period");

// ***************************************************************************
class CAHOutpostDeclareWarStart : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// read current outpost sheet
		uint32	outpostSheet= 0;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:OUTPOST_SELECTED:SHEET", false);
		if(node)
			outpostSheet= node->getValue32();

		// read wanted GMT attack period
		uint8	wantedAttHour= 0;
		node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ATTACK_PERIOD", false);
		if(node)
			wantedAttHour= (uint8)node->getValue32();

		// send a DECLARE_WAR_START message to server
		sendMsgToServer("OUTPOST:DECLARE_WAR_START", outpostSheet, wantedAttHour);
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostDeclareWarStart, "outpost_declare_war_start");

// ***************************************************************************
class CAHOutpostDeclareWarValidate : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// read current outpost sheet
		uint32	outpostSheet= 0;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:OUTPOST_SELECTED:SHEET", false);
		if(node)
			outpostSheet= node->getValue32();

		// read wanted GMT attack period
		uint8	wantedAttHour= 0;
		node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ATTACK_PERIOD", false);
		if(node)
			wantedAttHour= (uint8)node->getValue32();

		// read result Att Period Time (NB: for final server check: ensure that the user will get what it sees)
		uint32	startAttackTime= 0;
		node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_TIME_RANGE_ATT", false);
		if(node)
			startAttackTime= node->getValue32();

		// send a DECLARE_WAR_VALIDATE message to server
		string	sMsg= "OUTPOST:DECLARE_WAR_VALIDATE";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			//nlinfo("impulseCallBack : %s %d %d %d sent", sMsg.c_str(), outpostSheet, wantedAttHour, startAttackTime);
			out.serial(outpostSheet);
			out.serial(wantedAttHour);
			out.serial(startAttackTime);
			NetMngr.push(out);
		}
		else
		{
			nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
		}

	}
};
REGISTER_ACTION_HANDLER(CAHOutpostDeclareWarValidate, "outpost_declare_war_validate");

// ***************************************************************************
class CAHOutpostSelectFromBC : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Copy SERVER DB to Local selection
		uint32	outpostSheet= 0;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:OUTPOST");
		if(node)	outpostSheet= node->getValue32();
		node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:BOT_SELECTION");
		if(node)	node->setValue32(outpostSheet);

		// Send a msg to server
		if(outpostSheet)
		{
			string	sMsg= "OUTPOST:SELECT";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
			{
				//nlinfo("impulseCallBack : %s %d sent", sMsg.c_str(), outpostSheet);
				out.serial(outpostSheet);
				NetMngr.push(out);
			}
			else
			{
				nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostSelectFromBC, "outpost_select_from_bc");


// ***************************************************************************
class CAHOutpostUnSelect : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Called when the Outpost State window (the one opened from BotChat) is closed

		// Send a msg to server
		sendMsgToServer("OUTPOST:UNSELECT");
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostUnSelect, "outpost_unselect");


// ***************************************************************************
class CAHOutpostPVPJoin : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &Params)
	{
		// parse AH
		bool	bNeutral= true;
		OUTPOSTENUMS::TPVPSide side = OUTPOSTENUMS::UnknownPVPSide;
		if(Params=="attack")
		{
			bNeutral= false;
			side= OUTPOSTENUMS::OutpostAttacker;
		}
		else if(Params=="defend")
		{
			bNeutral= false;
			side= OUTPOSTENUMS::OutpostOwner;
		}

		// validate proposal
		OutpostManager.endPvpJoinProposal(bNeutral, side);
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostPVPJoin, "outpost_pvp_join");

// ***************************************************************************
class CAHOutpostBanPlayer : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Ask if ok before.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQConfirmOutpostBanPlayer"),
			"outpost_do_ban_player", sParams, "");
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostBanPlayer, "outpost_ban_player");

// ***************************************************************************
class CAHOutpostBanGuild : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Ask if ok before.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQConfirmOutpostBanGuild"),
			"outpost_do_ban_guild", sParams, "");
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostBanGuild, "outpost_ban_guild");

// ***************************************************************************
// Used also in game_context_menu
void	outpostTestUserCanBan(CCDBNodeLeaf *dbBanRight, bool &okForBanPlayer, bool &okForBanGuild)
{
	okForBanPlayer= false;
	okForBanGuild= false;

	/** A player can ban a player in a PVP outpost if:
	 *	- The player has right to ban AND his guild attack/owns the outpost the player attacks (all this is in RIGHT_BANNISH)
	 *	- The player and the target are in the same war of the outpost, and in same alliance.
	 *	- The player and the target are not in same guild
	 */
	if(dbBanRight && dbBanRight->getValueBool() && UserEntity)
	{
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
		if(selection && selection->isPlayer())
		{
			// in same outpost War and Side
			if( selection->getOutpostId() == UserEntity->getOutpostId() &&
				selection->getOutpostSide() == UserEntity->getOutpostSide() )
			{
				// not in same guild
				if( selection->getGuildNameID() != UserEntity->getGuildNameID() )
				{
					okForBanPlayer= true;
					okForBanGuild= selection->getGuildNameID()!=0;
				}
			}
		}
	}
}

// ***************************************************************************
class CAHOutpostDoBanPlayer : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*dbBan = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:PVP_OUTPOST:RIGHT_TO_BANISH", false);

		// Re-Test if can ban the selection
		bool	okForBanPlayer= false;
		bool	okForBanGuild= false;
		outpostTestUserCanBan(dbBan, okForBanPlayer, okForBanGuild);

		// if still ok to ban player
		if(okForBanPlayer)
		{
			sendMsgToServer("OUTPOST:BANISH_PLAYER");
		}
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostDoBanPlayer, "outpost_do_ban_player");

// ***************************************************************************
class CAHOutpostDoBanGuild : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*dbBan = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:PVP_OUTPOST:RIGHT_TO_BANISH", false);

		// Re-Test if can ban the selection
		bool	okForBanPlayer= false;
		bool	okForBanGuild= false;
		outpostTestUserCanBan(dbBan, okForBanPlayer, okForBanGuild);

		// if still ok to ban player's guild
		if(okForBanGuild)
		{
			sendMsgToServer("OUTPOST:BANISH_GUILD");
		}
	}
};
REGISTER_ACTION_HANDLER(CAHOutpostDoBanGuild, "outpost_do_ban_guild");

// ***************************************************************************
class CAHOutpostUpdateTimeZoneAuto : public IActionHandler
{
public:
	void	execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*dbTZ = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:OUTPOST:TIME_ZONE", false);
		if(!dbTZ)
			return;

		// *** must update time zone, in case of user changed it during play
		tzset();

		// *** Get the difference of time => timezone
		time_t	tGmt=0, tLocal=0;
		struct tm *timeTm;
		// get current time
		time_t ltime;
		time( &ltime );
		// convert into GMT time
		timeTm = gmtime( &ltime );
		if(timeTm)
			tGmt= mktime(timeTm);
		// convert into local time
		timeTm = localtime( &ltime );
		timeTm->tm_isdst = 0;		// reset daylight saving time flag
		if(timeTm)
			tLocal= mktime(timeTm);

		// Make the difference and hence compute the time zone
		double	tzSec= difftime(tLocal, tGmt);
		sint	tzHour= (sint)floor(tzSec / 3600);
		clamp(tzHour, -12, 12);

		// set the result
		dbTZ->setValue32(tzHour);

	}
};
REGISTER_ACTION_HANDLER(CAHOutpostUpdateTimeZoneAuto, "outpost_update_time_zone_auto");

