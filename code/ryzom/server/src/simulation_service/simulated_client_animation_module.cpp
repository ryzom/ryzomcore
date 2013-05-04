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


#include "simulated_client_animation_module.h"


#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"

#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive_utils.h"

#include "game_share/utils.h"
//#include "game_share/dyn_chat.h"

#include "r2_share/object.h"
#include "r2_share/scenario.h"
#include "r2_share/r2_messages.h"
#include <iostream>


using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;
using namespace R2;



NLNET_REGISTER_MODULE_FACTORY(CSimClientAnimationModule, "ClientAnimationModule");


void CSimClientAnimationModule::init(NLNET::IModuleSocket* clientGw)	//, CDynamicMapClient* client)
{

//	_Client = client;
	this->plugModule(clientGw);
}

void CSimClientAnimationModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{
	if (moduleProxy->getModuleClassName() == "ServerAnimationModule")
	{
		_ServerAnimationProxy = moduleProxy;
	}
	
}
	
void CSimClientAnimationModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	std::string moduleClassName = moduleProxy->getModuleClassName();
	if ( moduleClassName == "ServerAnimationModule")
	{
		_ServerAnimationProxy = NULL;
	}
}

void CSimClientAnimationModule::onProcessModuleMessage(IModuleProxy *senderModule, const CMessage &message)
{
	std::string operationName = message.getName();
	if(operationName=="stringTable")
	{
		nlwarning("received string table!");
		uint32 nb,i;
		nlRead(message, serial, nb);
		i=nb;
		nlwarning("%d entries! ",nb);
		while(i)
		{
			std::string localId;
			std::string value;
			nlRead(message, serial,localId);
			nlRead(message,serial,value);
			nlwarning("{ %s , %s}",localId.c_str(),value.c_str());
			i--;
		}
		return;
	}

	if(operationName == "stringValue")
	{
		std::string localId;
		std::string value;
		nlRead(message,serial,localId);
		nlRead(message,serial,value);
		nlwarning("received {%s , %s}",localId.c_str(),value.c_str());
		return;
	}
	
	if (operationName == "idList")
	{
		uint32 nb,i;
		nlRead(message,serial,nb);
		i=nb;
		nlwarning("%d entries! ",nb);
		while(i)
		{
			std::string localId;
			nlRead(message,serial,localId);
			nlwarning("{ %s }",localId.c_str());
			i--;
		}
		return;
	}
	if (operationName == "HELLO")
	{
		// this is just the firewall opening message, nothing to do
		return;
	}
	
	if (operationName == "NPC_APROP")
	{
		uint32 modes;
		nlRead(message,serial,modes);
//		_Client->onNpcAnimationTargeted(modes);
		return;
	}

	nlwarning("CSimClientAdminModule : receive unknown message '%s'", message.getName().c_str());
}

void CSimClientAnimationModule::onModuleSecurityChange(IModuleProxy *moduleProxy)
{
}

void CSimClientAnimationModule::requestTalkAs(const std::string& npcname)
{
	CMessage msg("talk_as");
	msg.serial(static_cast<std::string>(npcname));
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}



void CSimClientAnimationModule::requestStringTable()
{
	CMessage msg("requestStringTable");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CSimClientAnimationModule::requestSetStringValue(std::string& id,std::string& value )
{
	CMessage msg("requestSetValue");
	msg.serial(id);
	msg.serial(value);
	_ServerAnimationProxy->sendModuleMessage(this,msg);
}

void CSimClientAnimationModule::requestStartAct(uint32 actId)
{
	CMessage message ("requestStartAct");
	message.serial(actId);
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}		

void CSimClientAnimationModule::requestSetWeather(uint16 weatherValue)
{
	CMessage message ("requestSetWeather");
	message.serial(weatherValue);
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CSimClientAnimationModule::requestStopAct()
{
	CMessage message ("requestStopAct");
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CSimClientAnimationModule::requestStopTalkAs()
{
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	

	CMessage msg("stopTalk");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CSimClientAnimationModule::requestStringValue(std::string& localId )
{
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	CMessage msg("requestStringValue");
	msg.serial(localId);
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CSimClientAnimationModule::requestTpPosition(float x, float y)
{
	CMessage message ("requestTpPosition");
	message.serial(x);
	message.serial(y);
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CSimClientAnimationModule::requestDespawnEntity(const std::string& npcRtId)
{
	CMessage message ("requestTpPosition");
	std::string npcRtIdStr = npcRtId;
	message.serial(npcRtIdStr);
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CSimClientAnimationModule::requestDespawnGrp(const std::string& groupRtId)
{
	CMessage message ("requestDespawnGrp");
	std::string groupRtIdStr = groupRtId;
	message.serial(groupRtIdStr);
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CSimClientAnimationModule::requestIdList()
{
	BOMB_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);	
	CMessage msg("requestIdList");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

bool CSimClientAnimationModule::requestSetPioneerRight(const TPioneerRight& right)
{
	if (_ServerAnimationProxy != NULL)	
	{
		CShareServerAnimationItfProxy serverAnimationModule(_ServerAnimationProxy);
		serverAnimationModule.setPioneerRight(this, right);
	}
	return true;
}
