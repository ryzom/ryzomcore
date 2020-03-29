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

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/algo.h"
#include "nel/net/unified_network.h"

// game share
#include "game_share/singleton_registry.h"

// local
#include "gus_utils.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// private routines for layer 5 service tracking
	//-----------------------------------------------------------------------------

	typedef std::map<NLNET::TServiceId,CSString> TServiceMap;
	static TServiceMap ServiceMap;

	static void cbServiceUp(const std::string& serviceName, NLNET::TServiceId sid, void *)
	{
		ServiceMap[sid]= serviceName;
	}

	static void cbServiceDown(const std::string& serviceName, NLNET::TServiceId sid, void *)
	{
		ServiceMap.erase(sid);
	}


	//-----------------------------------------------------------------------------
	// executeRemoteCommand
	//-----------------------------------------------------------------------------

	void executeRemoteCommand(NLNET::TServiceId sid,const CSString& cmdLine)
	{
		// make sure the service we're trying to talk to is up
		DROP_IF(ServiceMap.find(sid)==ServiceMap.end(),"Failed to send command to unknown remote service: "+NLMISC::toString(sid.get()),return);

		// display a little debug message...
		nldebug("Sending command to service: %s(%d): %s",
			ServiceMap[sid].c_str(),
			sid.get(),
			cmdLine.c_str());

		// compose and send the command execution message
		CMessage msgout("EXEC_COMMAND");
		msgout.serial(const_cast<CSString&>(cmdLine));
		CUnifiedNetwork::getInstance()->send(sid,msgout);
	}

	void executeRemoteCommand(const char* serviceName,const CSString& cmdLine)
	{
		uint32 count=0;

		// iterate over all of the services that are up right now
		for (TServiceMap::iterator it= ServiceMap.begin(); it!=ServiceMap.end(); ++it)
		{
			// check whether the srvice matches the srevice name (with wildcards) that we supplied
			if (NLMISC::testWildCard(it->second,serviceName),NLMISC::testWildCard(it->second.toUpper(),CSString(serviceName).toUpper()))
			{
				// delegate command execution to executeRemoteCommand(uint16 sid,cmdLine)
				executeRemoteCommand(it->first,cmdLine);
				++count;
			}
		}
		if (count!=1)
		{
			nldebug("Command sent to %d services",count);
		}
	}

	static void cbExecCommandResult(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid)
	{
		// treat the rely message sent back from a service whom we asked to execute a command
		NLMISC::InfoLog->displayNL("EXEC_COMMAND_RESULT' Received from: %3d: %s",sid.get(),serviceName.c_str());

		// retrieve the text from the input message
		CSString txt;
		msgin.serial(txt);

		// divide the text into lines because NeL doesn't like long texts
		CVectorSString lines;
		txt.splitLines(lines);

		// display the lines of text
		for (uint32 i=0;i<lines.size();++i)
		{
			NLMISC::InfoLog->displayNL("%s",lines[i].c_str());
		}
	}


	//-----------------------------------------------------------------------------
	// code for registering callbacks at service init time
	//-----------------------------------------------------------------------------

	// automatically register the callbacks for message management at service init
	static class CRegisterGusUtilsMessageCallbacks: public IServiceSingleton
	{
	public:
		void init()
		{
			TUnifiedCallbackItem cbArray[] = 
			{
				{	"EXEC_COMMAND_RESULT",	cbExecCommandResult,	},
			};
			CUnifiedNetwork::getInstance()->addCallbackArray( cbArray, sizeof(cbArray)/sizeof(cbArray[0]) );

			CUnifiedNetwork::getInstance()->setServiceUpCallback("*",cbServiceUp);
			CUnifiedNetwork::getInstance()->setServiceDownCallback("*",cbServiceDown);
		}
	}
	_RegisterGusUtilsMessageCallbacks;

}

//-----------------------------------------------------------------------------
