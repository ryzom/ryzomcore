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



// include files
#include "nel/misc/types_nl.h"
#include "nel/misc/vectord.h"
#include "game_share/tick_event_handler.h"

#include "messages.h"
#include "mirrors.h"
#include "actor_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace AGS_TEST;



//-------------------------------------------------------------------------
// the callback table

/*
static void cbEngageFight( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );
static void cbUpdateFightBehaviour( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );
static void cbUpdateFightPosition( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );
static void cbUpdateFightEnd( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );
static void cbAckVisionZone( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );
static void cbAIVision( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId );

TUnifiedCallbackItem CbArray[] = 
{

	{	"ENGAGE_FIGHT",			cbEngageFight,	},
	{	"U_FIGHT_BEHAVIOUR",	cbUpdateFightBehaviour,	},
	{	"U_FIGHT_POS",			cbUpdateFightPosition,	},
	{	"U_END_FIGHT",			cbUpdateFightEnd,		},
	{	"U_FIGHT_END",			cbUpdateFightEnd,		},

	{	"ACK_VISION_ZONE",		cbAckVisionZone,		},
	{	"AGENT_VISION",			cbAIVision,				},
};
*/

//-------------------------------------------------------------------------
// singleton initialisation and release

void CMessages::init()
{
	// setup the callback array
	//CUnifiedNetwork::getInstance()->addCallbackArray( CbArray, sizeof(CbArray)/sizeof(CbArray[0]) );
}

void CMessages::release()
{
}

//--------------------------------------------------------------------------
// incoming message callbacks

/*
static void cbEngageFight( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CEntityId aggressor, target;
	msgin.serial(aggressor);
	msgin.serial(target);
	CActor *actor=CActorManager::getActor(target);

	if (actor)
	{
		nlinfo("Responding to ENGAGE_FIGHT: %s %s",aggressor.toString().c_str(),target.toString().c_str());
		actor->doFight(aggressor);
	}
	else
		nlinfo("Ignoring ENGAGE_FIGHT: %s %s",aggressor.toString().c_str(),target.toString().c_str());

}

static void cbUpdateFightBehaviour( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	nlinfo("Received U_FIGHT_BEHAVIOUR - ** Message not treated **");
}


static void cbUpdateFightPosition( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	// forwarding position updates from CMS to GPMS
//	nlinfo("Received U_FIGHT_POS: length=%i pos=%i",msgin.length(),msgin.getPos() );

	// this vector contais the body of the output messsage

	CMessage	msgout("UPDATE_ENTITIES_POS");
	CMessage	msgoutCMS("U_FIGHT_POS");
	bool CMSChanges=false;
	bool GPMSChanges=false;

	// parse the input message
	TGameCycle	cycle = CTickEventHandler::getGameCycle();
	while (msgin.length()>(unsigned)msgin.getPos())
	{
		CEntityId id;
		CVectorD pos;
		float		theta;
		msgin.serial(id,pos, theta);

		// make sure that the actor is one of ours and doesn't belong to another service
		CActor	*pactor = CActorManager::getActor(id);
		if (pactor!=0)
		{
			// check pos
			if (pactor->testPositionInPatat(pos))
			{
				sint32 x=(sint32)(pos.x*1000.0);
				sint32 y=(sint32)(pos.y*1000.0);
				sint32 z=(sint32)(pos.z*1000.0);
				//float angle=pactor->getAngle();

				msgout.serial(id,x,y,z,theta,cycle);
				GPMSChanges=true;
			}
			else
			{
				pos=CVectorD(CMirrors::x(id),CMirrors::y(id),CMirrors::z(id));
				sint32 x = CMirrors::x(id);
				sint32 y = CMirrors::y(id);
				sint32 z = CMirrors::z(id);
				float angle=pactor->getAngle();

				msgoutCMS.serial(id,x,y,z,angle,cycle);
				CMSChanges=true;
			}
		}

	}

	// create and send the output message
	if (GPMSChanges) sendMessageViaMirror( "GPMS", msgout );
	if (CMSChanges)  sendMessageViaMirror( "CMS", msgoutCMS );
}


static void cbUpdateFightEnd( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	// dealing with end of fight
//	nlinfo("Received U_FIGHT_END: length=%i pos=%i",msgin.length(),msgin.getPos() );

	while (msgin.length()>(unsigned)msgin.getPos())
	{
		// get actor id
		CEntityId	id;
		msgin.serial(id);

		// set actor to wandering mode
		CActor	*actor = CActorManager::getActor(id);
		if (actor)
		{
			nlinfo("Received U_FIGHT_END for actor: %s (%s)",id.toString().c_str(), actor->getName().c_str());
			actor->resetActivity();
			// at combat end, recover position from mirror (get the last fight position)
			actor->setPos(CMirrors::x(actor->getSid()), CMirrors::y(actor->getSid()), CMirrors::z(actor->getSid()));
//			actor->doWander();
		}
		else
		{
			nlinfo("Received U_FIGHT_END for unknown actor: %s",id.toString().c_str());
		}
	}
}



//
static void cbAIVision( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CMoveManager::processAIVision(msgin);
}

//
static void cbAckVisionZone( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	CActorManager::addVisionService(serviceId);
}
*/
