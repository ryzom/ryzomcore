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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

//nel
#include "nel/misc/command.h"
#include "nel/net/unified_network.h"

// game share
#include "game_share/utils.h"

// local
#include "gus_utils.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;


//-----------------------------------------------------------------------------
// Local globals
//-----------------------------------------------------------------------------

static uint32 RemoteControlId=0;


//-----------------------------------------------------------------------------
// Utility Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(utils,on,"run a command ON a different service","(<service name>|<service id>) <command line>")
{
	CNLSmartLogOverride logOverride(&log);

//	if (RemoteControlId!=0)
//	{
//		nlinfo("Forwarding command to remote control slave module: %d",RemoteControlId);
//		GUSNET::sendModuleMessage(msg,RemoteControlId,NULL);
//		return true;
//	}

	if (args.size()<2)
		return false;

	// split up the command line ...
	NLMISC::CSString rawArgs=		NLMISC::CSString(rawCommandString).tailFromFirstWord();
	NLMISC::CSString service=		rawArgs.strtok(" \t");	// eg '*'
	NLMISC::CSString commandLine=	rawArgs;				// eg 'help all'

	// check whether the service id is numeric 
	if (NLMISC::toString("%d",service.atosi())==service)
	{
		// execute the command by numeric service id
		executeRemoteCommand(NLNET::TServiceId(service.atosi()),commandLine);
	}
	else
	{
		// execute the command by service name
		executeRemoteCommand(service.c_str(),commandLine);
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,connect,"add a layer5 service connection to the given service","<short name> <address>[:<port>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	NLNET::CUnifiedNetwork::getInstance()->addService(args[0], NLNET::CInetAddress(args[1]));

	return true;
}

/*
NLMISC_CATEGORISED_COMMAND(utils,remoteControl,"set the target for the 'on' command to be a remote RCSLAVE module instead of the local shard (use gusnetDisplay to view remote module list)","<remote module id> | END")
{
	CNLSmartLogOverride logOverride(&log);

	switch (args.size())
	{
	case 0:
		if (RemoteControlId==0)
		{
			nlinfo("Remote control currently off - the 'on' command will operate on the local shard");
		}
		else
		{
			nlinfo("Remote control current active - remote control slave module id = %d",RemoteControlId);
		}
		return true;

	case 1:
		break;

	default:
		return false;
	}

	// check whether we have an 'end' or 0 id
	if (NLMISC::CSString(args[0])=="end" || args[0]=="0")
	{
		nlinfo("Ending remote control");
		RemoteControlId= 0;
		return true;
	}

	// lookup the id and make sure it's valid
	uint32 id= atoi(args[0].c_str());
	GUSNET::TRemoteModulePtr ptr= GUSNET::getInstance()->lookupRemoteModule(id);
	if (ptr==NULL || ptr->getName()!="RCSLAVE")
	{
		nlinfo("You need to supply the id of an RCSLAVE module");
		return true;
	}

	// setup the id
	RemoteControlId= id;

	return true;
}
*/

//-----------------------------------------------------------------------------
