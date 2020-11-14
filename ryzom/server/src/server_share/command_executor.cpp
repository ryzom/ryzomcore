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



#include "stdpch.h"

#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "command_executor_itf.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CMDEXE;


void commandExecutor_forcelink()
{
}


class CCommandExecutor: public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav <CModuleBase> > >,
	public CCommandExecutorSkel
{
public:

	CCommandExecutor()
	{
		CCommandExecutorSkel::init(this);
	}

//	bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		if (CCommandExecutorSkel::onDispatchMessage(sender, message))
//			return true;
//
//		nlwarning("CCommandExecutor : Unknown message '%s' received", message.getName().c_str());
//
//		return false;
//	}

	std::string buildModuleManifest() const
	{
		string ret("ServiceName=");
		ret += IService::getInstance()->getServiceShortName();

		return ret;
	}


	///////////////////////////////////////////////////////////////////////////////
	///////// CCommandExecutorSkel implementation  ////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	void sendCommand(NLNET::IModuleProxy *sender, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg)
	{
		// rebuild the command line
		string cmdLine = commandName+" "+senderEId.toString();
		if (haveTarget)
		{
			cmdLine += " "+targetEId.toString();
		}
		
		cmdLine += " " + arg;
		// execute the command
		NLMISC::CCommandRegistry::getInstance().execute(cmdLine, InfoLog());
 	}

};

NLNET_REGISTER_MODULE_FACTORY(CCommandExecutor, "CommandExecutor");


