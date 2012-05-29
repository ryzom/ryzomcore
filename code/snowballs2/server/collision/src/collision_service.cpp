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

#include "collision_service.h"
#include <nel/3d/u_instance_group.h>

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

using namespace SBSERVICE;
using namespace NLMISC;
using namespace NLNET;
using namespace NLPACS;
using namespace std;


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                               VARIABLES                                ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

CCollisionService::CClientMap CCollisionService::_Clients;
NLPACS::URetrieverBank *CCollisionService::_RetrieverBank;
NLPACS::UGlobalRetriever *CCollisionService::_GlobalRetriever;
NLPACS::UMoveContainer *CCollisionService::_MoveContainer;
CCollisionService::CMovePrimitiveVector CCollisionService::_StaticMovePrimitives;
TTime CCollisionService::_LastTime, CCollisionService::_NewTime, CCollisionService::_DiffTime;
double CCollisionService::_DiffTimeSeconds;
float CCollisionService::_DiffTimeFloat;


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            BASIC FUNCTIONS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

void CCollisionService::commandStart()
{

}

void CCollisionService::init()
{
	// set down callback
	CUnifiedNetwork::getInstance()->setServiceDownCallback("*", cbDown);

	// set the data path
	CPath::addSearchPath(ConfigFile.getVar("DataPath").asString(), true, false);

	// init the global retriever, the retriever bank, and the move container
	_RetrieverBank = URetrieverBank::createRetrieverBank(ConfigFile.getVar("RetrieverBankName").asString().c_str());
	_GlobalRetriever = UGlobalRetriever::createGlobalRetriever(ConfigFile.getVar("GlobalRetrieverName").asString().c_str(), _RetrieverBank);
	_MoveContainer = UMoveContainer::createMoveContainer(_GlobalRetriever, 100, 100, 6.0);

	// some silly snowballs specific code to load static instance groups, redo
	CConfigFile::CVar igv = ConfigFile.getVar("InstanceGroups");
	for (uint i = 0; i < igv.size(); ++i)
	{
		NL3D::UInstanceGroup *ig = NL3D::UInstanceGroup::createInstanceGroup(igv.asString(i));
		if (ig == NULL) nlwarning("Instance group '%s' not found", igv.asString(i).c_str());
		else
		{
			for (uint i = 0; i < ig->getNumInstance(); ++i)
			{
				UMovePrimitive *primitive = _MoveContainer->addCollisionablePrimitive(0, 1);
				primitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
				primitive->setReactionType(UMovePrimitive::DoNothing);
				primitive->setTriggerType(UMovePrimitive::NotATrigger);
				primitive->setCollisionMask(2);
				primitive->setOcclusionMask(1);
				primitive->setObstacle(true);

				string name = ig->getShapeName(i);
				float rad;
					 if (strlwr(name) == "pi_po_igloo_a")		rad = 4.5f;
				else if (strlwr(name) == "pi_po_snowman_a")		rad = 1.0f;
				else if (strlwr(name) == "pi_po_pinetree_a")	rad = 2.0f;
				else if (strlwr(name) == "pi_po_tree_a")		rad = 2.0f;
				else if (strlwr(name) == "pi_po_pingoo_stat_a")	rad = 1.0f;
				else if (strlwr(name) == "pi_po_gnu_stat_a")	rad = 1.0f;
				else
				{
					rad = 2.0f;
					nlwarning ("Instance name '%s' doesn't have a good radius for collision", name.c_str());
				}

				primitive->setRadius(rad);
				primitive->setHeight(6.0f);

				primitive->insertInWorldImage(0);
				CVector	pos = ig->getInstancePos(i);
				primitive->setGlobalPosition(CVectorD(pos.x, pos.y, pos.z - 1.5f), 0);
				_StaticMovePrimitives.push_back(primitive);
			}
		}
		delete ig;
	}

	_NewTime = CTime::getLocalTime();
}

bool CCollisionService::update()
{
	_LastTime = _NewTime;
	_NewTime = CTime::getLocalTime();
	_DiffTime = _NewTime - _LastTime;
	_DiffTimeSeconds = (double)_DiffTime / 1000.0;
	_DiffTimeFloat = (float)_DiffTimeSeconds;

	for (CClientMap::iterator it = _Clients.begin(); it != _Clients.end(); it++)
		for (CEntityMap::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
	{
		CEntity &entity = it2->second;

		if (entity.Moving)
		{
			// nldebug("entity.Moving");
			CVector movement = entity.NewClientPosition - entity.OldClientPosition;
			entity.Distance = movement.norm();
			if (entity.Distance == 0.0f)
			{
				entity.Moving = false;
				entity.Retry = false;
			}
			CVectorD speed = CVectorD(movement) / _DiffTimeSeconds;
			entity.MovePrimitive->move(speed, 0);
		}
		//  else nldebug("!entity.Moving");
	}

	// apparently this thingy does the collision checks
	_MoveContainer->evalCollision(_DiffTimeSeconds, 0);
	
	// check all wrong stuff (use some different way maybe)
	for (CClientMap::iterator it = _Clients.begin(); it != _Clients.end(); it++)
		for (CEntityMap::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
	{
		CEntity &entity = it2->second;

		if (entity.Moving)
		{
			UGlobalPosition globalPosition;
			entity.MovePrimitive->getGlobalPosition(globalPosition, 0);
			CVector serverPosition = _GlobalRetriever->getGlobalPosition(globalPosition);
			//nlinfo("clientPosition: %f %f %f", entity.NewClientPosition.x, entity.NewClientPosition.y, entity.NewClientPosition.z);
			//nlinfo("serverPosition: %f %f %f", serverPosition.x, serverPosition.y, serverPosition.z);
			
			// Allow the difference between the external client position and the 
			// local server position to be up to half of the traveled distance 
			// plus the entity's height or radius, twice (using Retry).
			float allowedDifference = entity.Distance * 0.5f;
			bool move = false;
			if (abs(entity.NewClientPosition.z - serverPosition.z) > 1.0f + allowedDifference)
			{
				move = true;
			}
			else
			{
				// fake server position has same z as client position if ok
				serverPosition.z = entity.NewClientPosition.z;
				if ((serverPosition - entity.NewClientPosition).norm() > entity.MovePrimitive->getRadius() + allowedDifference)
					move = true;
			}

			if (move) // Entity moved incorrectly.
			{				
				if (entity.Retry) // If second try.
				{
					nldebug("entity.Retry");
					msgPosition(it2->first, serverPosition);
				}
				else nldebug("!entity.Retry");
				entity.Retry = !entity.Retry; // Else the entity gets one more chance.
			}

			// The difference to the new position must be from local server position.
			entity.OldClientPosition = serverPosition;
		}
	}
	
	msgUpdate();
	return true;
}

void CCollisionService::release()
{

}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            OTHER FUNCTIONS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

void CCollisionService::sendMessage(CMessage &msgout)
{
	CUnifiedNetwork *instance = CUnifiedNetwork::getInstance();
	for (CClientMap::iterator it = _Clients.begin(); it != _Clients.end(); it++)
		CUnifiedNetwork::getInstance()->send(TServiceId(it->first), msgout);
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            MESSAGE SENDERS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   msgUpdate
 *             Send CLS_UPDATE message to registered services
 * 
 * Arguments:
 ****************************************************************************/
void CCollisionService::msgUpdate()
{
	if (!_Clients.size()) return;

	static CMessage msgout("CLS_UPDATE");
	sendMessage(msgout);
	//nldebug("Sent CLS_UPDATE to %u services", _Clients.size());
}

/****************************************************************************
 * Function:   msgPosition
 *             Send CLS_POSITION message to registered services
 * 
 * Arguments:
 *             - id:        entity id
 *             - position:  new position
 ****************************************************************************/
void CCollisionService::msgPosition(uint32 id, CVector position)
{
	if (!_Clients.size()) return;

	CMessage msgout("CLS_POSITION");
	msgout.serial(id, position);
	sendMessage(msgout);
	nldebug("Sent CLS_POSITION %u %f %f %f to %u services", id, position.x, position.y, position.z, _Clients.size());
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                           MESSAGE CALLBACKS                            ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   cbAdd
 *             Receives an "ADD" message.
 ****************************************************************************/
void CCollisionService::cbAdd(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	uint32 id;
	CVector position;
	float radius;
	msgin.serial(id, position, radius);
	nldebug("Received ADD %u %f %f %f %f", id, position.x, position.y, position.z, radius);
	
	// Do something with it
	CEntity &entity = _Clients[sid.get()][id] = CEntity();
	entity.OldClientPosition = position;
	entity.NewClientPosition = position;
	entity.MovePrimitive = _MoveContainer->addCollisionablePrimitive(0, 1);
	entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
	entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
	entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
	entity.MovePrimitive->setCollisionMask(3);
	entity.MovePrimitive->setOcclusionMask(2);
	entity.MovePrimitive->setObstacle(true);
	entity.MovePrimitive->setRadius(radius);
	entity.MovePrimitive->setHeight(1.8f);
	entity.MovePrimitive->insertInWorldImage(0);
	entity.MovePrimitive->setGlobalPosition(position, 0);	
}

/****************************************************************************
 * Function:   cbMove
 *             Receives a "MOVE" message.
 ****************************************************************************/
void CCollisionService::cbMove(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	uint32 id;
	CVector position;
	msgin.serial(id, position);
	// nldebug("Received MOVE %u %f %f %f", id, position.x, position.y, position.z);
	
	// Do something with it
	CEntity &entity = _Clients[sid.get()][id];
	entity.NewClientPosition = position;
	if (entity.OldClientPosition != position)
		entity.Moving = true;
}

/****************************************************************************
 * Function:   cbRemove
 *             Receives a "REMOVE" message.
 ****************************************************************************/
void CCollisionService::cbRemove(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	uint32 id;
	msgin.serial(id);
	nldebug("Received REMOVE %u", id);
	
	// Do something with it
	if (_Clients[sid.get()].find(id) == _Clients[sid.get()].end())
	{
		nlwarning("Unknown entity %u", id);
		return;
	}
	CEntity &entity = _Clients[sid.get()][id];
	_MoveContainer->removePrimitive(entity.MovePrimitive);
	_Clients[sid.get()].erase(id);
}

/****************************************************************************
 * Function:   cbRegister
 *             Receives a "REGISTER" message.
 ****************************************************************************/
void CCollisionService::cbRegister(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	nldebug("Received REGISTER %s %s", serviceName.c_str(), sid.toString().c_str());

	// Do something with it
	_Clients[sid.get()] = CEntityMap();
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                           NETWORK CALLBACKS                            ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   cbDown
 *             Called when a registered service goes down
 ****************************************************************************/
void CCollisionService::cbDown(const string &serviceName, TServiceId sid, void *arg)
{
	CClientMap::iterator it = _Clients.find(sid.get());
	if (it != _Clients.end())
	{
		// remove all entities
		for (CEntityMap::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
			_MoveContainer->removePrimitive(it2->second.MovePrimitive);
		// erase the client
		_Clients.erase(it);
	}
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                         SERVICE CONFIGURATION                          ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * CallbackArray
 *
 * It define the functions to call when receiving a specific message
 ****************************************************************************/
TUnifiedCallbackItem CallbackArray[] =
{
	{ "REGISTER", CCollisionService::cbRegister },
	{ "ADD", CCollisionService::cbAdd },
	{ "MOVE", CCollisionService::cbMove },
	{ "REMOVE", CCollisionService::cbRemove },
};

/****************************************************************************
 * SNOWBALLS COLLISION SERVICE MAIN Function
 *
 * This call create a main function for the world_service:
 *
 *    - based on the service class CCollisionService inherited from IService
 *    - having the short name "CLS"
 *    - having the long name "collision_service"
 *    - listening on an automatically allocated port (0) by the naming service
 *    - and callback actions set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN(CCollisionService, "CLS", "collision_service", 0, CallbackArray, "", "")

/* end of file */
