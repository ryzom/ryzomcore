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

#pragma message "Deprecated, don't use !"


#include "stdpch.h"
#include "gpms_interface.h"
#include "ai_instance.h"
#include "ai_mgr.h"

/*
// Include
#include "nel/net/unified_network.h"
#include "game_share/player_vision_delta.h"
#include "gpms_interface.h"
#include "ai_vision.h"
*/
#include "game_share/tick_event_handler.h"
#include "game_share/synchronised_message.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//------------------------------------------------------------------------
// static data
//------------------------------------------------------------------------

// Only for invisible group entities
//NLNET::CMessage CGPMSInterface::_updateEntityPosMsg;
//bool CGPMSInterface::_positionUpdates = false;


//------------------------------------------------------------------------
// message callbacks
//------------------------------------------------------------------------
/*
static void cbVisionDelta( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	static vector<CPlayerVisionDelta>	deltas;
	deltas.clear();

	CPlayerVisionDelta::decodeVisionDelta(msgin, deltas);

	for (uint i=0;i<deltas.size();++i)
		CAIVision::applyVisionDelta(deltas[i].PlayerId,deltas[i]);
}

static TUnifiedCallbackItem CbArray[] = 
{
	{	"VISIONS_DELTA_2",		cbVisionDelta,	}
};
*/

//------------------------------------------------------------------------
// classic init(), update() and release()
//------------------------------------------------------------------------
/*
void CGPMSInterface::init()
{
	// setup the callback array
//	CUnifiedNetwork::getInstance()->addCallbackArray( CbArray, sizeof(CbArray)/sizeof(CbArray[0]) );

	// Only for invisible group entities
	_updateEntityPosMsg.setType( "UPDATE_ENTITIES_POS" );
}
void CGPMSInterface::update()
{
	// Only for invisible group entities
	if ( _positionUpdates==true )
	{
		sendMessageViaMirror( "GPMS", _updateEntityPosMsg );
		_updateEntityPosMsg.clear();
		_updateEntityPosMsg.setType( "UPDATE_ENTITIES_POS" );
		_positionUpdates=false;
	}
}
void CGPMSInterface::release()
{
}

//------------------------------------------------------------------------
// main API
//------------------------------------------------------------------------

// add entities to the world, remove them from the world or move them in the world
void CGPMSInterface::addEntity(const NLMISC::CEntityId	&eid, const CAIPos &pos, NLMISC::CSheetId sheet)
{
	// Only for invisible group entities

	// add entities to the GPMS
	sint32 ix=(sint32)(pos.x().asDouble()*1000.0);
	sint32 iy=(sint32)(pos.y().asDouble()*1000.0);
	sint32 ih=pos.h();
	float t=pos.theta().asRadians();

	NLNET::CMessage msgout("ADD_ENTITY");
	
	
	msgout.serial( const_cast<NLMISC::CEntityId &>(eid) );

	msgout.serial( ix );
	msgout.serial( iy );
	msgout.serial( ih );
	msgout.serial( t );
	msgout.serial( sheet );
	sendMessageViaMirror( "GPMS", msgout );

	//nlinfo( "ADDTOSHARD: COORD  BOT %s ------------ %d %d %d", id().toString().c_str(), x().asInt(), y().asInt(), h() );
}
*/
/*
void CGPMSInterface::addEntity(CAIEntityId id, const CAIPos &pos, NLMISC::CSheetId sheet)
{
	addEntity(id.toEntityId(),pos,sheet);
}
*/
/*
void CGPMSInterface::removeEntity(const NLMISC::CEntityId &eid)
{
	// Only for invisible group entities

	// remove the actor from the GPMS
	CMessage msgout("REMOVE_ENTITY");
	msgout.serial( const_cast<NLMISC::CEntityId &>(eid) );
	sendMessageViaMirror( "GPMS", msgout );
}
*/
/*
void CGPMSInterface::removeEntity(CAIEntityId id)
{
	removeEntity(id.toEntityId());
}
*/
/*
void CGPMSInterface::updateEntityPos(const NLMISC::CEntityId &eid,const CAIPos &pos)
{
	// Only for invisible group entities

	sint32 ix=(sint32)(pos.x().asDouble()*1000.0);
	sint32 iy=(sint32)(pos.y().asDouble()*1000.0);
	sint32 ih=pos.h();
	float t=pos.theta().asRadians();
	NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle()-1;
	_updateEntityPosMsg.serial( const_cast<NLMISC::CEntityId &>(eid) );
	_updateEntityPosMsg.serial( ix );
	_updateEntityPosMsg.serial( iy );
	_updateEntityPosMsg.serial( ih );
	_updateEntityPosMsg.serial( t );
	_updateEntityPosMsg.serial( tick );
	_positionUpdates=true;
}
*/
/*
void CGPMSInterface::updateEntityPos(CAIEntityId id,const CAIPos &pos)
{
	updateEntityPos(id.toEntityId(),pos);
}
*/
/*
// mounting and dismounting from horseback
void CGPMSInterface::attachEntityToParent()
{
}
void CGPMSInterface::detachEntityFromParent()
{
}
*/
//---------------------------------------------------------------------------------------
// add invisible 'vision' entities to the world, remove them from the world or move them
/*
void CGPMSInterface::addVisionEntity(const NLMISC::CEntityId &eid,const CAIPos &pos)
{
	addEntity(eid,pos,NLMISC::CSheetId::Unknown);
}

void CGPMSInterface::removeVisionEntity(const NLMISC::CEntityId &eid)
{
	removeEntity(eid);
}

void CGPMSInterface::updateVisionEntityPos(const NLMISC::CEntityId &eid,const CAIPos &pos)
{
	updateEntityPos(eid,pos);
}

*/
