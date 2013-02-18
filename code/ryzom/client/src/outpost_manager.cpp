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

#include "outpost_manager.h"
#include "net_manager.h"
#include "interface_v3/interface_manager.h"
#include "nel/gui/group_container.h"
#include "nel/misc/bit_mem_stream.h"
#include "game_share/generic_xml_msg_mngr.h"

COutpostManager		OutpostManager;

using namespace NLMISC;


// ***************************************************************************
COutpostManager::COutpostManager()
{
	_EndTickForPvpJoinProposal= 0;
}


// ***************************************************************************
void	COutpostManager::startPvpJoinProposal(bool playerGuildInConflict, bool playerGuildIsAttacker,
							 uint32 ownerGuildNameId, uint32 attackerGuildNameId, uint32 declTimer)
{
	// reset counter that force player to be neutral (eg: 10 seconds)
	_EndTickForPvpJoinProposal= NetMngr.getCurrentServerTick() + declTimer;

	// setup TEMP DB
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:PVP_PROPOSAL_PLAYER_GUILD_IN_CONFLICT");
	if(node)	node->setValueBool(playerGuildInConflict);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:PVP_PROPOSAL_PLAYER_GUILD_IS_ATTACKER");
	if(node)	node->setValueBool(playerGuildIsAttacker);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:PVP_PROPOSAL_ATTACKER");
	if(node)	node->setValue32(attackerGuildNameId);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:PVP_PROPOSAL_DEFENDER");
	if(node)	node->setValue32(ownerGuildNameId);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:PVP_PROPOSAL_TICK_END");
	if(node)	node->setValue32(_EndTickForPvpJoinProposal);

	// open Popup
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_outpost_proposal"));
	if (pGC)
	{
		pGC->setActive(true);
		CWidgetManager::getInstance()->setTopWindow(pGC);
		pGC->updateCoords();
		pGC->updateCoords();
		pGC->center();
		pGC->enableBlink(2);
	}

}

// ***************************************************************************
void	COutpostManager::endPvpJoinProposal(bool bNeutral, OUTPOSTENUMS::TPVPSide pvpSide)
{
	// send msg
	string sMsg= "OUTPOST:SIDE_CHOSEN";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %s sent", sMsg.c_str(), bNeutral, OUTPOSTENUMS::toString(pvpSide).c_str());
		out.serial(bNeutral);
		uint8 sideAsInt = (uint8)pvpSide;
		out.serial(sideAsInt);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}

	// Abort any timer
	_EndTickForPvpJoinProposal= 0;
}

// ***************************************************************************
void	COutpostManager::update()
{
	// *** If there is a join proposal running
	if(_EndTickForPvpJoinProposal!=0)
	{
		if(_EndTickForPvpJoinProposal<NetMngr.getCurrentServerTick())
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();

			// Force the neutral choose
			CAHManager::getInstance()->runActionHandler("outpost_pvp_join", NULL, "neutral");

			// close the window
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_outpost_proposal"));
			if (pGC)
				pGC->setActive(false);

			// stop timer
			_EndTickForPvpJoinProposal= 0;
		}
	}
}

