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

#include "messages.h"
//#include "actor_manager.h"

using namespace NLMISC;
using namespace NLNET;


//-------------------------------------------------------------------------
// singleton initialisation and release

void CMessages::init()
{
	// incoming messages
	TRANSPORT_CLASS_REGISTER( CMsgAIServiceUp );
	TRANSPORT_CLASS_REGISTER( CMsgAIManagerUp );
	TRANSPORT_CLASS_REGISTER( CMsgAIFeedback );

	TRANSPORT_CLASS_REGISTER( CMsgAICloseMgrs );
	TRANSPORT_CLASS_REGISTER( CMsgAIBackupMgrs );
}

void CMessages::release()
{
}


//-------------------------------------------------------------------------
// incoming message callbacks

void CMsgAIServiceUp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	nlinfo("AI Service Up: %02d (CPU rating: %f  RAMrating %f)", sid.get(), ProcessorAllocation,RamAllocation);
	for (uint i=0;i<ManagersRunning.size();i++)
		nlinfo("-- Running Manager: %04d",ManagersRunning[i]);
}

void CMsgAIManagerUp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	nlinfo("Service %d: Manager Up: %04d",sid.get(), MgrId);
}

void CMsgAIFeedback::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	nlinfo("AI Manager %d: %s",Message.c_str());
}

//-------------------------------------------------------------------------
// stub outgoing message callbacks

void CMsgAICloseMgrs::callback(const std::string &serviceName, NLNET::TServiceId sid) {}
void CMsgAIBackupMgrs::callback(const std::string &serviceName, NLNET::TServiceId sid) {}

