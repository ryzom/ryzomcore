// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/vectord.h>

#include <nel/net/login_client.h>
#include <nel/net/login_cookie.h>

#include <nel/3d/u_text_context.h>

#include <nel/pacs/u_move_primitive.h>

#include "snowballs_client.h"
#include "commands.h"
#include "network.h"
#include "entities.h"
#include "interface.h"
#include "graph.h"
#include "mouse_listener.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;

namespace SBCLIENT {

// -- -- random note: most of this needs to change completely anyway

//
// Variables
//

CCallbackClient *Connection = NULL;

//
// Functions
//

static void cbClientDisconnected (TSockId from, void *arg)
{
	nlwarning ("You lost the connection to the server");

	askString ("You lost the connection to the server!!!", "", 2, CRGBA(64,0,0,128));
}

static void cbAddEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	string name;
	uint8 race;
	CVector startPosition;

	msgin.serial (id, name, race, startPosition);

//	nlinfo ("%s", stringFromVector (msgin.bufferAsVector()));

//	nlinfo ("Receive add entity %u '%s' %s (%f,%f,%f)", id, name.c_str(), race==0?"penguin":"gnu", startPosition.x, startPosition.y, startPosition.z);

	nlinfo ("New player named '%s' comes in at position (%8.2f, %8.2f, %8.2f)", name.c_str(), startPosition.x, startPosition.y, startPosition.z);

	if (Self == NULL && name == Login.toUtf8())
	{
		addEntity(id, name, CEntity::Self, startPosition, startPosition);
	}
	else
	{
		addEntity(id, name, CEntity::Other, startPosition, startPosition);
	}
}

static void cbRemoveEntity (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;

	msgin.serial (id);

//	nlinfo ("Receive remove entity %u", id);

	EIT eit = findEntity (id);
	CEntity	&entity = (*eit).second;

	nlinfo ("Player named '%s' goes offline", entity.Name.c_str());

	removeEntity (id);
}

static void cbEntityPos (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	CVector position;
	float angle;
	uint32 state;

	msgin.serial (id, position, angle, state);

//	nlinfo ("(%5d) Receive entity pos %u (%f,%f,%f) %f, %u", msgin.length(), id, position.x, position.y, position.z, angle, state);

	if (Self->Id == id)
	{
		// receive my info, ignore them, i know where i am
		return;
	}

	EIT eit = findEntity (id, false);
	if (eit == Entities.end ())
	{
		nlwarning ("can't find entity %u", id);
	}
	else
	{
		CEntity	&entity = (*eit).second;

		entity.ServerPosition = position;
		entity.AuxiliaryAngle = angle;
		if (state&1)
		{
			entity.IsAiming = true;
			entity.IsWalking = false;
		}
	}
}

static void cbEntityTeleport(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// temp
	uint32 id;
	CVector position;
	msgin.serial(id, position);
	nldebug("Received entity %u teleport %f,%f,%f", id, position.x, position.y, position.z);
	EIT eit = findEntity(id, false);
	if (eit == Entities.end()) nlwarning("can't find entity %u", id);
	else
	{
		CEntity	&entity = eit->second;
		entity.ServerPosition = position;
		entity.Position = position;
		entity.MovePrimitive->setGlobalPosition(CVectorD(position.x, position.y, position.z), 0);
		if (&entity == Self) MouseListener->setPosition(position);
	}
}

static void cbHit(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 sid, eid;
	bool direct;

	msgin.serial (sid, eid, direct);

//	nlinfo ("Receive hit msg %u %u %d", sid, eid, direct);

	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	if (!entity.AnimQueue.empty())
	{
		EAnim a = entity.AnimQueue.front ();
		playAnimation (entity, HitAnim, true);
		playAnimation (entity, a);
	}
	else
	{
		playAnimation (entity, HitAnim, true);
	}
	
	removeEntity (sid);
}

static void cbSnowball (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 eid, sid;
	CVector position, target;
	float speed, deflagRadius;

	msgin.serial (sid, eid, position, target, speed, deflagRadius);
	
//	nlinfo ("Receive a snowball message");

	shotSnowball (sid, eid, position, target, speed, deflagRadius);
}

static void cbChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string line;
	msgin.serial (line);
	addLine (line);
}

static void cbIdentification (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint32 id;
	msgin.serial (id);
	
//	nlinfo ("my online id is %u", id);

//	if (Self == NULL)
//		nlerror ("Self is NULL");
//
//	if (Self->Id != id)
//	{
////		nlinfo ("remaping my entity from %u to %u", Self->Id, id);
//		
//		// copy my old entity
//		CEntity me = *Self;
//		
//		// set my new online id
//		me.Id = id;
//
//		// add my new entity in the array
//		EIT eit = (Entities.insert (make_pair (id, me))).first;
//
//		// remove my old entity
//		Entities.erase (Self->Id);
//
//		// remap Self
//		Self = &((*eit).second);
//	}

	// send to the network my entity
	std::string login_name(Login.toUtf8());
	sendAddEntity(id, login_name, 1);
}

// Array that contains all callback that could comes from the server
static TCallbackItem ClientCallbackArray[] =
{
	{ "ADD_ENTITY", cbAddEntity },
	{ "REMOVE_ENTITY", cbRemoveEntity },
	{ "ENTITY_POS", cbEntityPos },
	{ "ENTITY_TP", cbEntityTeleport },
	{ "HIT", cbHit },
	{ "CHAT", cbChat },
	{ "SNOWBALL", cbSnowball },
	{ "IDENTIFICATION", cbIdentification },
};


bool	isOnline ()
{
	return Connection != NULL && Connection->connected ();
}

void	sendAddEntity (uint32 id, string &name, uint8 race)
{
	if (!isOnline ()) return;

	CMessage msgout("ADD_ENTITY");
	msgout.serial(id, name, race);
	Connection->send(msgout);
}

void	sendChatLine (string Line)
{
	if (!isOnline ()) return;

	CMessage msgout ("CHAT");
	string s;
	if (Self != NULL)
	{
		string line = Self->Name + string("> ") + Line;
		msgout.serial (line);
	}
	else
	{
		string line = string("Unknown> ") + Line;
		msgout.serial (line);
	}

	Connection->send (msgout);
}

void	sendEntityPos (CEntity &entity)
{
	if (!isOnline ()) return;

	// is aiming? is launching etc...
	uint32 state = 0;
	state |= (entity.IsAiming?1:0);

	CMessage msgout ("ENTITY_POS");
	msgout.serial (entity.Id, entity.Position, entity.Angle, state);

	Connection->send (msgout);
	
//	nlinfo("(%5d) Sending pos to network (%f,%f,%f, %f), %u", msgout.length(), entity.Position.x, entity.Position.y, entity.Position.z, entity.Angle, state);
}

void	sendSnowBall (uint32 eid, const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, float deflagRadius)
{
	if (!isOnline ()) return;

	CMessage msgout ("SNOWBALL");
	msgout.serial (eid, const_cast<CVector &>(position), const_cast<CVector &>(target), speed, deflagRadius);
	Connection->send (msgout);

//	nlinfo("Sending snowball to network (%f,%f,%f) to (%f,%f,%f) with %f %f", position.x, position.y, position.z, target.x, target.y, target.z, speed, deflagRadius);
}


TTime LastPosSended;

void	initNetwork(const std::string &lc, const std::string &addr)
{
	// if lc and addr is valid, directly connect to the fs
	Connection = new CCallbackClient;
	Connection->addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	Connection->setDisconnectionCallback (cbClientDisconnected, NULL);

	string fsaddr;
	if (addr.empty())
		fsaddr = ConfigFile->getVar("FSHost").asString ();
	else
		fsaddr = addr;

	if(fsaddr.find (":") == string::npos)
	{
		fsaddr += ":37000";
	}

	CLoginCookie loginCookie;
	if (!lc.empty ())
	{
		loginCookie.setFromString (lc);
	}

	string res = CLoginClient::connectToShard (loginCookie, fsaddr, *Connection);
	if (!res.empty ())
	{
		string err = string ("Connection to shard failed: ") + res;
		askString (err, "", 2, CRGBA(64,0,0,128));
	}
	else
	{
		// we remove all offline entities
		removeAllEntitiesExceptUs ();
		
		askString ("You are online!!!", "", 2, CRGBA(0,64,0,128));
		// now we have to wait the identification message to know my id
	}

	LastPosSended = 0;
}

void	updateNetwork()
{
	if (Connection != NULL)
	{
		Connection->update ();

		sint64 newBytesDownloaded = (sint64) Connection->newBytesDownloaded ();
		sint64 newBytesUploaded = (sint64) Connection->newBytesUploaded ();

		TextContext->setHotSpot (UTextContext::MiddleTop);
		TextContext->setColor (CRGBA(255, 255, 255, 255));
		TextContext->setFontSize (14);
		TextContext->printfAt (0.5f, 0.99f, "d:%"NL_I64"u u:%"NL_I64"u / d:%"NL_I64"u u:%"NL_I64"u / d:%"NL_I64"u u:%"NL_I64"u",
			Connection->bytesDownloaded (), Connection->bytesUploaded (),
			Connection->getBytesReceived (),Connection->getBytesSent (),
			newBytesDownloaded, newBytesUploaded);

		DownloadGraph.addValue ((float)newBytesDownloaded);
		UploadGraph.addValue ((float)newBytesUploaded);

		if (isOnline () && Self != NULL)
		{
			static float oldAngle = 0.0f;
			
			if ((Self->Angle != oldAngle || Self->IsAiming || Self->IsWalking) && CTime::getLocalTime () > LastPosSended + 100)
			{
				sendEntityPos (*Self);

				LastPosSended = CTime::getLocalTime ();

				oldAngle = Self->Angle;
			}
		}
	}
}

void	releaseNetwork()
{
	if (Connection != NULL)
	{
		if (Connection->connected ())
			Connection->disconnect ();

		delete Connection;
		Connection = NULL;
	}
}

} /* namespace SBCLIENT */

/* end of file */
