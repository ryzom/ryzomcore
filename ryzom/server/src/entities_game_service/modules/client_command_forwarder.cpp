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

#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/command_executor_itf.h"

#include "client_command_forwarder.h"



using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CMDEXE;



class CClientCommandForwader : public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav <CModuleBase> > >,
	public IClientCommandForwader
{
	typedef map<std::string, TModuleProxyPtr>	TCommandExecutors;

	TCommandExecutors	_CommandExecutors;

public:

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "CommandExecutor")
		{
			TParsedCommandLine pcl;
			if (!pcl.parseParamList(module->getModuleManifest()))
				return;

			const TParsedCommandLine *serviceName = pcl.getParam("ServiceName");
			if (serviceName == NULL)
				return;

			_CommandExecutors[serviceName->ParamValue] = module;
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "CommandExecutor")
		{
			// remove it from the list
			TCommandExecutors::iterator first(_CommandExecutors.begin()), last(_CommandExecutors.end());
			for (; first != last; ++first)
			{
				if (first->second == module)
				{
					_CommandExecutors.erase(first);
					break;
				}
			}
		}

	}

	bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
	{
		nlwarning("CRingSessionManager : Unknown message '%s' received", message.getName().c_str());

		return false;
	}


	///////////////////////////////////////////////////////////////////////////////
	///////// IClientCommandForwader implementation  //////////////////////////////
	///////////////////////////////////////////////////////////////////////////////


	void sendCommand(const std::string &service, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg)
	{
		TCommandExecutors::iterator it(_CommandExecutors.find(service));

		if (it == _CommandExecutors.end())
			return;

		CCommandExecutorProxy cep(it->second);

		cep.sendCommand(this, commandName, senderEId, haveTarget, targetEId, arg);
	}

};

NLNET_REGISTER_MODULE_FACTORY(CClientCommandForwader, "ClientCommandForwader");

