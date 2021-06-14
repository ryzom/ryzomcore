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

#include "client_message.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"

#include "ai.h"
#include "ai_player.h"
#include "nel/misc/entity_id.h"

using namespace NLMISC;
using namespace NLNET;

// a big bad global var !
extern CAIEntityPhysical	*TempSpeaker;
extern CBotPlayer			*TempPlayer;


void cbClientFollowTarget( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	msgin.serial( eId );

	if	(eId.getType()!=RYZOMID::player)
		return;

	// then the entity ID
	CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(eId));

	if	(!entityPhys)
		return;

	CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);
	player->setFollowMode(true);

	if	(	((CAIEntityPhysical*)player->getTarget())
		||	((CAIEntityPhysical*)player->getVisualTarget())	)
	{
		// generate the follow event
		CSpawnBotNpc *bnpc = dynamic_cast<CSpawnBotNpc *>(((CAIEntityPhysical*)player->getTarget()));
		if	(!bnpc)
			bnpc = dynamic_cast<CSpawnBotNpc *>(((CAIEntityPhysical*)player->getVisualTarget()));
		
		if	(!bnpc)
			return;

		// set the temporary speaker
		TempSpeaker = bnpc;
		TempPlayer = player;

		{
			CGroupNpc	&grpNpc = bnpc->getPersistent().grp();
			// generate en event on this bot group
//			grpNpc.getEventContainer().EventPlayerTargetNpc.processStateEvent(&grpNpc);
			grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerTargetNpc);
			
			// if player is in follow mode, then generate an suplementary event
			if	(player->getFollowMode())
				grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerFollowNpc);
//				grpNpc.getEventContainer().EventPlayerFollowNpc.processStateEvent(&grpNpc);
		}

		// reset the temp speaker
		TempSpeaker = NULL;
		TempPlayer = NULL;
	}

}

void cbClientNoFollowTarget( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	CEntityId eId;
	msgin.serial( eId );

	if	(eId.getType()!=RYZOMID::player)
		return;

	// then the entity ID
	CAIEntityPhysical	*const	entityPhys=CAIS::instance().getEntityPhysical(TheDataset.getDataSetRow(eId));

	if	(!entityPhys)
		return;

	CBotPlayer	*const	player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);
	player->setFollowMode(false);
}


//----------------------------
//	CbClientArray
//----------------------------
TUnifiedCallbackItem CbClientArray[]=
{
	{ "CLIENT:TARGET:FOLLOW",			cbClientFollowTarget },
	{ "CLIENT:TARGET:NO_FOLLOW",		cbClientNoFollowTarget },
}; 

void CAIClientMessages::init()
{
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbClientArray, sizeof(CbClientArray)/sizeof(CbClientArray[0]) );
}

void CAIClientMessages::release()
{
}

#include "event_reaction_include.h"
