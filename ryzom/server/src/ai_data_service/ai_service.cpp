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



//===================================================================

/*
  ***
  when a service goes down I must remember that the managers are no longer running
  in the normal way I can try to bring them back up on the remaining services

  ***
  when a service comes up and the rest of the services are over-loaded it would be
  good to be able to load ballance (if loading is fast enough)
  - tell new service to load static data and prepare to launch.
  - wait for new service to say 'ready'
  - tell old service to transfer to new service (ie: save to ram & transmit to new service)
  - * old service must continue to forward packets to the new service to ensure smooth continuation
  - old service tells me when he's down
  - new service tells me when he's up

  ***
  Need to deal with 'save's as well...

  ***
  Need init() and release() in order to register message callbacks, etc?
*/

/*
//-------------------------------------------------------------------------
// Some global variables
uint32 GlobalServicesUp=0;
NLMISC_VARIABLE(uint32, GlobalServicesUp,	"ServicesUp"); 
*/

#include "nel/misc/debug.h"

#include "ai_manager.h"
#include "ai_service.h"
#include "ai_files.h"
#include "messages.h"

//===================================================================

//---------------------------------------------------
// INSTANTIATED CLASS: Public methods

// a few read accessors
NLNET::TServiceId CAIService::id() const
{
	// compute the index of this AIService class instance in the array of
	// CAIService (this is stored in the _services array)
	return NLNET::TServiceId(this-_services);
}

bool CAIService::isUp() const
{
	return _isUp;
}


uint CAIService::powerCPU() const
{
	return _powerCPU;
}

uint CAIService::powerRAM() const
{
	return _powerRAM;
}


// assign a given manager to this service (ie open and run it)
void CAIService::assignMgr(sint mgrId)
{
	// get a pointer to the manager in question
	CAIManager *m=CAIManager::getManagerById(mgrId);
	if (m==NULL)
		return;

	// if this method has not been called by the manager's own method then ping pong
	if (!m->isAssigned())
	{
		m->assign(id());
		return;
	}

	// if the manager is assigned to a different service then bomb
	if (m->serviceId()!=id())
	{
		nlwarning("Cannot assign manager %04d (%s) to service %d as it is already assigned to service %d",
			mgrId, m->name().c_str(), id().get(), m->serviceId().get());
		return;
	}

	// if the service is up then send it a message to launch the manager
	if (isUp())
	{
		// std::string dataBuf
		// dataBuf.resize(binFileSize(id()));
		// FILE *f=fopen(binFileName(id()),"rb");
		// if (f==NULL)
		// {
		//		nlinfo("Failed to load action list file: %s (try 'aiMake')",binFileName(id()));
		//		return false;
		// }
		// if (fread(dataBuf.buffer(),1,binFileSize(id()),f) != binFileSize(id()))
		// {
		//		nlinfo("Read error in binary file: %s",binFileName(id()));
		//		return false;
		// }
		// fclose(f);
		// CMsgAIUploadActions(id(),dataBuf).send();
//		CMsgAIOpenMgrs(mgrId,m->name()).send(id()); 
		m->setIsUp(true);

		// return true;
	}
	// return false;
}


// unassign a manager currently running on this service (ie close it)
void CAIService::unassignMgr(sint mgrId)
{
	// if the manager isn't assigned to this service then bomb
	if (CAIManager::getManagerById(mgrId)->serviceId()!=id())
	{
		nlwarning("Cannot stop manager %04d (%s) on service %d as it is assigned to service %d",
			mgrId, CAIManager::getManagerById(mgrId)->name().c_str(), id().get(), CAIManager::getManagerById(mgrId)->serviceId().get());
		return;
	}

	// transfer control to the singleton to finish the work
	closeMgr(mgrId);
}


// reassign a manager currently assigned to this service to another service
void CAIService::reassignMgr(sint mgrId, NLNET::TServiceId serviceId)
{
	// get a pointerto the service
	CAIService *s=getServiceById(serviceId);
	if (s==NULL)
		return;

	// this is a very simple implementation of control transfer... should be revised later
	unassignMgr(mgrId);
	s->assignMgr(mgrId);
}



//---------------------------------------------------
// INSTANTIATED CLASS: Private methods

CAIService::CAIService()
{
	// if the following assert fails it's because a CAIService object has
	// been instantiated outside of the singleton's array
	nlassert(id().get() < maxServices());
}


//===================================================================
//  *** END OF THE INSTANTIATED CLASS *** START OF THE SINGLETON ***
//===================================================================


//---------------------------------------------------
// SINGLETON: Data

class CAIService CAIService::_services[RYAI_AI_SERVICE_MAX_SERVICES];




//---------------------------------------------------
// SINGLETON: Public methods

// get the number of allocated managers
uint CAIService::numServices()
{
	uint count=0;
	for (uint i=0;i<maxServices();i++)
		if (_services[i].isUp())
			count++;
	return count;
}


// get a pointer to the manager with given handle (0..maxManagers-1)
CAIService *CAIService::getServiceById(NLNET::TServiceId id)
{
	if (id.get() >= maxServices())
	{
		nlwarning("CAIService::getServiceById(id): id %d not in range 0..%d",id.get(),maxServices()-1);
		return NULL;
	}
	return &(_services[NLNET::TServiceId8(id).get()]);
}


// get a pointer to the manager with given index (0..numManagers-1)
CAIService *CAIService::getServiceByIdx(uint idx)
{
	uint count=0;
	for (uint i=0;i<maxServices();i++)
		if (_services[i].isUp())
		{
			if (idx==count)
				return &(_services[i]);
			count++;
		}
	nlwarning("CAIService::getServiceByIdx(idx): idx (%d)>=maxServices (%d)",idx,count);
	return NULL;
}


// select a service to assign the manager to and assign it
// if no services are available then the manager is queued until one
// becomes available
// managers opened via this interface are queued back up and re-launched
// if their service goes down
void CAIService::openMgr(sint mgrId)
{
	// get a pointer to the manager in question
	CAIManager *m=CAIManager::getManagerById(mgrId);
	if (m==NULL)
		return;
	m->setIsOpen(true);
}


// close a manager (on whichever service its running)
void CAIService::closeMgr(sint mgrId)
{
	// get a pointer to the manager in question
	CAIManager *m=CAIManager::getManagerById(mgrId);
	if (m==NULL)
		return;

	// if the manager is flagged as up and running on a service then shut it down
	if (m->isUp())
	{
		CAIService *s=getServiceById(m->serviceId());
		if (s!=NULL && s->isUp())
		{
			// send a message to the service to stop the manager
			CMsgAICloseMgrs(mgrId).send(m->serviceId());
		}
		// clear the manager's isUp() flag
		m->setIsUp(false);
	}

	// if the manager is assigned to a service then transfer control to the manager's close()
	if (m->isAssigned())
		m->close();
}

// update routine called by service_main update every net loop
void CAIService::update()
{
	#define IN_TRANSIT_PACKET_LIMIT 1

	for (uint i=0;i<maxServices();i++)
	{
		CAIService &s=_services[i];
		if (s.isUp())
		{
			// see whether we have enough spare capacity to start sending data about a new manager
			// if so look through the managers for any that are awaiting assignment
			if (s._dataSentCount+s._dataToSend.size()-s._dataAckCount<IN_TRANSIT_PACKET_LIMIT)
			{
				// look for the heaviest manager that doesn't blow my quotas
				uint best=~0u;
				uint bestScore=0;
				for (uint j=0;j<CAIManager::maxManagers();j++)
				{
					CAIManager *m=CAIManager::getManagerById(j);
					if (m->isOpen() && !m->isAssigned())
						if (m->weightRAM()<=s._unusedPowerRAM && m->weightCPU()<=s._unusedPowerCPU)
							if (m->weightRAM()+m->weightCPU()/2>=bestScore)
							{
								best=j;
								bestScore=m->weightRAM()+m->weightCPU()/2;
							}
				}
				// if we found a manager then go ahead and assign it
				if (best!=~0u)
					s.assignMgr(best);
			}

			// if we have data waiting to be sent then send it
			while (!s._dataToSend.empty() && s._dataSentCount-s._dataAckCount<IN_TRANSIT_PACKET_LIMIT)
			{
				NLNET::CUnifiedNetwork::getInstance()->send( s.id(), *(s._dataToSend.begin()) );
				s._dataToSend.pop_front();
				s._dataSentCount++;
			}
		}
	}

	#undef IN_TRANSIT_PACKET_LIMIT
}


//===================================================================
