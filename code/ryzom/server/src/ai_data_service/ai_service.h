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



#ifndef RYAI_AI_SERVICE_H
#define RYAI_AI_SERVICE_H

#define RYAI_AI_SERVICE_MAX_SERVICES 256


#include "nel/misc/types_nl.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

#include <list>

 
/*
  ---------------------------------------------------------------------------

  This class defines both the singleton that manages the ai services and the
  class type of the ai services themselves. The singleton interface can be found
  at the end of the class

  ---------------------------------------------------------------------------
*/

class CAIService
{

//===================================================================
//  ***            START OF THE INSTANTIATED CLASS               ***
//===================================================================


public:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Public methods

	// a few read accessors
	NLNET::TServiceId id() const;
	bool isUp() const;

	uint powerCPU() const;
	uint powerRAM() const;

	// assign a given manager to this service (ie open and run it)
	void assignMgr(sint mgrId);

	// unassign a manager currently running on this service (ie close it)
	void unassignMgr(sint mgrId);

	// reassign a manager currently assigned to this service to another service
	void reassignMgr(sint mgrId, NLNET::TServiceId serviceId);


private:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Private methods

	CAIService();	// may only be instantiated by the singleton


private:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Private data

	bool _isUp;				// flag says the service is up

	uint _powerCPU;			// maximum CPU weight allowed on service
	uint _powerRAM;			// maximum RAM weight allowed on service

	uint _unusedPowerCPU;	// remaining CPU capacity (after counting weight of running managers)
	uint _unusedPowerRAM;	// remaining RAM capacity (after counting weight of running managers)

	uint _dataSentCount;	// number of data packets sent to service
	uint _dataAckCount;		// number of data acknowledges received from service

	// vector of buffers of data that are waiting to be sent to the service
	std::list<NLNET::CMessage> _dataToSend;

//===================================================================
//  *** END OF THE INSTANTIATED CLASS *** START OF THE SINGLETON ***
//===================================================================


public:
	//---------------------------------------------------
	// SINGLETON: Public methods

	// get the number of valid handles (ie the maximum number of managers allowed)
	static uint maxServices() { return RYAI_AI_SERVICE_MAX_SERVICES; }

	// get the number of allocated managers
	static uint numServices();

	// get a pointer to the manager with given handle (0..maxManagers-1)
	static CAIService *getServiceById(NLNET::TServiceId id);

	// get a pointer to the manager with given index (0..numManagers-1)
	static CAIService *getServiceByIdx(uint idx);

	// select a service to assign the manager to and assign it
	// if no services are available then the manager is queued until one
	// becomes available
	// managers opened via this interface are queued back up and re-launched
	// if their service goes down
	static void openMgr(sint mgrId);

	// close a manager (on whichever service its running)
	static void closeMgr(sint mgrId);

	// update routine called by service_main update every net loop
	static void update();

private:
	//---------------------------------------------------
	// SINGLETON: Private methods


private:
	//---------------------------------------------------
	// SINGLETON: Private data
	static class CAIService _services[RYAI_AI_SERVICE_MAX_SERVICES];


//===================================================================
//  ***                 END OF THE SINGLETON                     ***
//===================================================================

};


#endif
