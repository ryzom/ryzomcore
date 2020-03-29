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

// include files
#include "nel/misc/types_nl.h"

#include "client_messages.h"

//#include "game_share/tick_event_handler.h"
//#include "game_share/msg_client_server.h"
//#include "game_share/mode_and_behaviour.h" //TEMP!!!
//#include "game_share/news_types.h"
//#include "game_share/bot_chat_types.h"
//#include "game_share/brick_types.h"
//#include "game_share/loot_harvest_state.h"
//#include "game_share/ryzom_version.h"
//#include "game_share/ryzom_mirror_properties.h"

#include "server_share/r2_variables.h"

#include "world_position_manager.h"
#include "gpm_service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


/****************************************************************\
					cbUpdateEntityPosition() 
\****************************************************************/
void cbClientPosition( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientPosition);

	CEntityId			id;
	NLMISC::TGameCycle	tick;
	msgin.serial(id, tick);

	TDataSetRow entityIndex = TheDataset.getDataSetRow( id );
	if ( !entityIndex.isValid() )
	{
		// client may send position even before it is added in system
		//nldebug( "%u: Receiving a position from client %s which is not in mirror yet", CTickEventHandler::getGameCycle(), id.toString().c_str() );
		return;
	}
	
	// entity pos (x, y, z, theta)
	sint32				x, y, z;
	float				heading;
	msgin.serial(x, y, z, heading);

	if (IsRingShard)
	{
		// make sure the move that the player is trying to make is legal
		// if the move wasn't legal then the values of 'x' and 'y' will be changed to make them legal
		bool moveWasLegal= pCGPMS->MoveChecker->checkMove(entityIndex, x, y, tick);

		// if the move wasn't legal then dispatch a message to the player
		if (!moveWasLegal)
		{
//  ***TODO ***
//			// Teleport the player back to a previous valid location
//			CMessage msgout( "IMPULSION_ID" );
//			msgout.serial( master->Id );
//			CBitMemStream bms;
//			GenericXmlMsgManager.pushNameToStream( "TP:CORRECT", bms );
//			bms.serial( x );	
//			bms.serial( y );	
//			bms.serial( z );	
//			msgout.serialMemStream( bms );
//			CUnifiedNetwork::getInstance()->send( master->Id.getDynamicId(), msgout );
//  ***TODO ***
		}

		// set the player coordinates in the ring vision universe
		pCGPMS->RingVisionUniverse->setEntityPosition(entityIndex,x,y);

		// todo: determine whether player is in water etc for real;
		bool local= false;
		bool interior= false;
		bool water= false;

		// update the player coordinates in the mirror
		CMirrorPropValue1DS<sint32>( TheDataset, entityIndex, DSPropertyPOSX )= x; 
		CMirrorPropValue1DS<sint32>( TheDataset, entityIndex, DSPropertyPOSY )= y;
		CMirrorPropValue1DS<sint32>( TheDataset, entityIndex, DSPropertyPOSZ )= (z&~7) + (local ? 1 : 0) + (interior ? 2 : 0) + (water ? 4 : 0);
		CMirrorPropValue1DS<float>( TheDataset, entityIndex, DSPropertyORIENTATION )= heading;
		CMirrorPropValue1DS<NLMISC::TGameCycle>( TheDataset, entityIndex, DSPropertyTICK_POS )= tick;

		CMirrorPropValue1DS<TYPE_CELL> cell ( TheDataset, entityIndex, DSPropertyCELL );
		uint32	cx = (uint16) ( + x/CWorldPositionManager::getCellSize() );
		uint32	cy = (uint16) ( - y/CWorldPositionManager::getCellSize() );
		cell = (cx<<16) + cy;

		// update the player position in the ring vision grid
		pCGPMS->RingVisionUniverse->setEntityPosition(entityIndex,x,y);
	}
	else
	{
/*
	// check player mode and behaviour
	CMirrorPropValueRO<MBEHAV::TMode> propMode( TheDataset, entityIndex, DSPropertyMODE );
	MBEHAV::EMode mode = (MBEHAV::EMode)(propMode().Mode);
	CMirrorPropValueRO<MBEHAV::CBehaviour> propBehaviour( TheDataset, entityIndex, DSPropertyBEHAVIOUR );
	MBEHAV::EBehaviour behaviour = (MBEHAV::EBehaviour)(propBehaviour().Behaviour);
	if (	(mode == MBEHAV::COMBAT)
		||	(mode == MBEHAV::COMBAT_FLOAT)
		||	(mode == MBEHAV::DEATH) )
	{
		H_AFTER(cbClientPosition);
		return;
	}
*/
		CWorldEntity	*player = CWorldPositionManager::getEntityPtr(entityIndex);

		if (player == NULL)
		{
			return;
		}

		if (player->X() == 0 && player->Y() == 0 && player->Z() == 0)
		{
			return;
		}

		//CWorldPositionManager::setEntityPosition(id, x, y, z, heading, tick);
		if (player->getType() == CWorldEntity::Player && player->CheckMotion && player->PosInitialised)
		{
			CWorldPositionManager::movePlayer(player, x, y, z, heading, tick);
		}
	}
} // cbClientPosition //

//----------------------------
//	CbClientArray
//----------------------------
TUnifiedCallbackItem CbClientArray[]=
{
	{ "CLIENT:POSITION",	cbClientPosition },

}; 



//-------------------------------------------------------------------------
// singleton initialisation and release

void CClientMessages::init()
{
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbClientArray, sizeof(CbClientArray)/sizeof(CbClientArray[0]) );
}

void CClientMessages::release()
{
}

