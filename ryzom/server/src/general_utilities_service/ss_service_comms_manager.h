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


#ifndef SS_SERVICE_COMMS_MANAGER_H
#define SS_SERVICE_COMMS_MANAGER_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/net/message.h"

// game_share
#include "game_share/singleton_registry.h"


//-------------------------------------------------------------------------------------------------
// class CServiceCommsManager
//-------------------------------------------------------------------------------------------------

class CServiceCommsManager
{
public:
	// singleton accessor
	static CServiceCommsManager* getInstance();

public:
	// execute a command line or variable modification request
	// note that during treatment of serviceUp the '#' token is a valid
	// service id - it represents the service that has just appeared
	virtual void execute(const std::string& serviceNameWildcard,const std::string& cmdLine)=0;

	// display the state of the singleton (list the connected services etc)
	virtual void display(NLMISC::CLog* log=NLMISC::InfoLog)=0;
};


//-------------------------------------------------------------------------------------------------
#endif
