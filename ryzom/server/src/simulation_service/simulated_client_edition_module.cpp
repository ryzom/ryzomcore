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


#include "simulated_client_edition_module.h"

#include "simulated_dmc.h"

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

#include "r2_share/object.h"
#include "r2_share/scenario.h"
#include "r2_share/r2_messages.h"
#include <iostream>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;
using namespace R2;


NLNET_REGISTER_MODULE_FACTORY(CSimClientEditionModule, "ClientEditionModule");

CSimClientEditionModule::CSimClientEditionModule() :
	_Id( INVALID_MODULE_ID )
{	
}

CSimClientEditionModule::~CSimClientEditionModule()
{
//	delete _PropertyAccessor;
//	delete _Palette;
	delete _Factory;
	delete _Scenario;
}

void CSimClientEditionModule::init( uint id, NLNET::IModuleSocket* clientGw, CDynamicMapClient* client)
{
	_Id = id;
	_Eid = NLMISC::toString( "SimClientEditionModule%d", _Id );
//	_Palette = new CPalette();
	_Scenario = new CScenario(0);
	_Factory = new CObjectFactory(_Eid);	// CObjectFactoryClient(_Eid);	
//	_PropertyAccessor = new CPropertyAccessor(_Client);
	CObjectSerializer::Factory = _Factory;
	
	_Client = client;
//	_ClientGw = clientGw;

	// plug module to gateway
	this->plugModule( clientGw );

//	_TranslationModule = 0;
	_AdventureId = 0;
}

// when server is launched locally, it shouldn't use the client factory, 
// this class disable when local server is called
class CSerialFactoryBackup
{
public:
	CSerialFactoryBackup(CObjectFactory *newValue = NULL) : ClientFactory(CObjectSerializer::Factory)
	{
		CObjectSerializer::Factory = newValue;
	}
	~CSerialFactoryBackup()
	{
		CObjectSerializer::Factory = ClientFactory;
	}
	CObjectFactory *ClientFactory;
};


void CSimClientEditionModule::release()
{
//	delete _Palette; _Palette = 0;
	delete _Scenario;  _Scenario = 0;
	delete _Factory; _Factory = 0;
//	delete _PropertyAccessor;	_PropertyAccessor = 0;
}

void CSimClientEditionModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{
	nlassert( moduleProxy );
	std::string moduleName = moduleProxy->getModuleClassName();
	const char *szModuleName = moduleName.c_str();
	nlinfo( "SimCEM%d: onModuleUp: %s", _Id, szModuleName );
	if( !strcmp(szModuleName, "ServerEditionModule") )
	{
		_ServerEditionProxy = moduleProxy;
	}
}
	
void CSimClientEditionModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	nlassert( moduleProxy );
	nlinfo( "SimCEM%d: onModuleDown: %s", _Id, moduleProxy->getModuleClassName().c_str() );
	if (moduleProxy->getModuleClassName() == "ServerEditionModule")
	{
		_ServerEditionProxy = NULL;
	}
}

void CSimClientEditionModule::onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
{
	nlassert( senderModuleProxy );
	std::string senderModuleName = senderModuleProxy->getModuleClassName();
	const char *szSenderName = senderModuleName.c_str();
	std::string operationName = message.getName();
	const char *szOpName = operationName.c_str();
	nlinfo( "SimCEM%d: onProcessModuleMessage: %s", _Id, szOpName );

	switch( szOpName[0] )
	{
	case 'H':
	case 'h':
		if( !stricmp(szOpName, "HELLO") )
		{
			// this is just the firewall opening message, nothing to do
			nlinfo( "%s says hello!", szSenderName );
			break;
		}
	case 'I':
	case 'i':
		if( !stricmp(szOpName, "insertNode") )
		{
			nlinfo( "insertNode ignored" );
			break;
		}
	case 'S':
	case 's':
		if( !stricmp(szOpName, "setNode") )
		{
			nlinfo( "setNode ignored" );
			break;
		}
	case 'E':
	case 'e':
		if( !stricmp(szOpName, "eraseNode") )
		{
			nlinfo( "eraseNode ignored" );
			break;
		}
		else if( !stricmp(szOpName, "EDITOR_STOP") )
		{
			nlinfo( "EDITOR_STOP received" );
			_Client->onEditionModeDisconnected();
			_Client->onTestModeConnected();		
			break;
		}
	case 'M':
	case 'm':
		if( !stricmp(szOpName, "moveNode") )
		{
			nlinfo( "moveNode ignored" );
			break;
		}
	case 'U':
	case 'u':
		if( !stricmp(szOpName, "updateScenario") )
		{
			nlinfo( "updateScenario received" );
			break;
		}
	case 'A':
	case 'a':
		if( !stricmp(szOpName, "ADV_CONN") )
		{
			nlinfo( "ADV_CONN message received" );
		
			CClientMessageAdventureUserConnection  bodyConnection;
			nlRead(message,serial,bodyConnection);
			
//			nlinfo("R2CED: user Connected as SimClient%d", bodyConnection.EditSlotId);
			nlinfo("bodyConnection received:");
			std::string edMode = (bodyConnection.Mode == 1 ? "Edition":(bodyConnection.Mode == 2 ? "Test" : "Animation"));
			nlinfo("  AdventureId: %d, EditSlotId: %d, Mode: %s", bodyConnection.AdventureId,
				bodyConnection.EditSlotId, edMode.c_str());


//			CObjectSerializer HighLevel;
//			bool MustTp;
//			bool InCache;
/*
			if (bodyConnection.Mode == 1)
			{
				_Client->onResetEditionMode();
			}

			_AdventureId = bodyConnection.AdventureId;
			CObject* data = bodyConnection.HighLevel.getData();
			if (!data && bodyConnection.InCache)
			{
				data = _Client->getComLuaModule().loadLocal("_tmp_.txt");
			}
			_Client->onEditionModeConnected(bodyConnection.EditSlotId, bodyConnection.AdventureId, data );
			if (bodyConnection.Mode == 2)
			{
				_Client->onEditionModeDisconnected();
				_Client->onTestModeConnected();
			}
*/
			break;
		}
	case 'T':
	case 't':
		if( !stricmp(szOpName, "TEST_STOP") )
		{
			nlinfo( "TEST_STOP received" );
			_Client->onTestModeDisconnected();
			break;
		}
		if( !stricmp(szOpName, "TEST_START") )
		{
			nlinfo( "TEST_START ignored" );
			break;
		}
	default:
		nlinfo( "Unknown message '%s' received from %s", szOpName, szSenderName );
		nlassert(0);
		break;
	}
}

void CSimClientEditionModule::onModuleSecurityChange(IModuleProxy *moduleProxy)
{
	nlassert( moduleProxy );
	nlinfo( "SimCEM%d: onModuleSecurityChange: %s", _Id, moduleProxy->getModuleClassName().c_str() );
}

void CSimClientEditionModule::requestCreateScenario(CObject* scenario)
{
	nlinfo( "SimCEM%d: requestCreateScenario: %x", _Id, scenario );
	CSerialFactoryBackup fb;
	CMessage message ("requestCreateScenario");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	message.serial(CObjectSerializer(scenario));
	_ServerEditionProxy->sendModuleMessage(this, message );
}

void CSimClientEditionModule::requestUploadScenario(CObject* scenario)
{
	nlinfo( "SimCEM%d: requestUploadScenario: %x", _Id, scenario );
	CSerialFactoryBackup fb;
	CMessage message ("requestUploadScenario");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	message.serial(CObjectSerializer(scenario));
	_ServerEditionProxy->sendModuleMessage(this, message );
}


void CSimClientEditionModule::requestUpdateRtScenario( CObject* scenario)
{
	nlinfo( "SimCEM%d: requestUpdateRtScenario: %x", _Id, scenario );
	CSerialFactoryBackup fb;
	CMessage message ("requestUpdateRtScenario");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	message.serial(CObjectSerializer(scenario));
	_ServerEditionProxy->sendModuleMessage(this, message );

}

void CSimClientEditionModule::requestGoTest()
{
	nlinfo( "SimCEM%d: requestGoTest", _Id );
	CSerialFactoryBackup fb;
	CMessage message ("START_TEST");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	_ServerEditionProxy->sendModuleMessage(this, message );
//	_Client->onEditionModeDisconnected(); //TODO Must Be called by admin module
//	_Client->onTestModeConnected();
}		

void CSimClientEditionModule::requestCreatePrimitives()
{
	CSerialFactoryBackup fb;
	CMessage message ("DBG_CREATE_PRIMITIVES");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	_ServerEditionProxy->sendModuleMessage(this, message );
}		

void CSimClientEditionModule::requestStopTest()
{
	nlinfo( "CSimClientEditionModule: requestStopTest" );
	CSerialFactoryBackup fb;
	CMessage message ("STOP_TEST");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	_ServerEditionProxy->sendModuleMessage(this, message );
//	_Client->onTestModeDisconnected();
}		
/*
void CSimClientEditionModule::requestSetNode(
	const std::string& instanceId, const std::string& attrName, CObject* value)	
{
	if (!attrName.empty())
	{
		CObject* instance = _Scenario->find(instanceId);
	
		if (instance)
		{			
			CObject* property = _PropertyAccessor->getPropertyValue(instance, attrName);
			if (property && property->equal(value))
			{
				nlinfo("R2Cl: Optimisation(message not send)");
				return;
			}
		}
	}

	requestSetNodeNoTest(instanceId, attrName, value);
}


void CSimClientEditionModule::requestSetNodeNoTest(
	const std::string& instanceId, const std::string& attrName, CObject* value)	
{
	
	CSerialFactoryBackup fb;
	CMessage message ("RSN");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	CMessageRequestSetNode msg(instanceId, attrName, value);
	message.serial(msg);
	_ServerEditionProxy->sendModuleMessage(this, message );
}


void CSimClientEditionModule::requestEraseNode( 
		const std::string& instanceId, const std::string& attrName, sint32 position)
{
	CSerialFactoryBackup fb;
	CMessage message ("REN");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	CMessageRequestEraseNode msg(instanceId, attrName, position);
	message.serial(msg);
	_ServerEditionProxy->sendModuleMessage(this, message );

}

void CSimClientEditionModule::requestInsertNode( 
		const std::string& instanceId, const std::string& attrName, sint32 position,
		const std::string& key, CObject* value)
{
	CSerialFactoryBackup fb;
	CMessage message ("RIN");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	CMessageRequestInsertNode msg(instanceId, attrName, position, key, value);
	message.serial(msg);
	_ServerEditionProxy->sendModuleMessage(this, message );

}


void CSimClientEditionModule::requestMoveNode( 
			const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& desInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	CSerialFactoryBackup fb;
	CMessage message ("RMN");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	CMessageRequestMoveNode msg(instanceId, attrName, position,
		desInstanceId, destAttrName, destPosition);
	message.serial(msg);
	_ServerEditionProxy->sendModuleMessage(this, message );
}
*/

void CSimClientEditionModule::requestMapConnection( uint32 scenarioId, bool mustTp)
{
	nlinfo( "CSimClientEditionModule: requestMapConnection: %d", scenarioId );
	CSerialFactoryBackup fb;
	CMessage message ("requestMapConnection");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	
	message.serial(scenarioId);
	message.serial(mustTp);
	_ServerEditionProxy->sendModuleMessage(this, message );
}

void CSimClientEditionModule::requestReconnection()
{
	if (_AdventureId != 0)
	{
		nlinfo( "CSimClientEditionModule: requestReconnection: %d", _AdventureId );
		this->requestMapConnection(_AdventureId, false);
	}	
}
/*
void CSimClientEditionModule::addPaletteElement(const std::string& attrName, CObject* paletteElement)
{
	_Palette->addPaletteElement(attrName, paletteElement);
}

CObject* CSimClientEditionModule::getPropertyValue(CObject* component, const std::string& attrName) const
{
	return _PropertyAccessor->getPropertyValue(component, attrName);
}

CObject* CSimClientEditionModule::getPropertyValue(const std::string& instanceId, const std::string& attrName) const
{
	CObject* component = _Scenario->find(instanceId);
	if (!component) return 0;
	return _PropertyAccessor->getPropertyValue(component, attrName);
}

CObject* CSimClientEditionModule::getPropertyList(CObject* component) const
{
	typedef std::list<std::string> TContainer;

	TContainer properties;

	_PropertyAccessor->getPropertyList(component, properties);
	CObject* toRet = new CObjectTable();
	TContainer::const_iterator first(properties.begin()), last(properties.end());
	for ( ; first != last; ++first)
	{
		component->add(new CObjectString(*first));
	}
	return component;
}

CObject* CSimClientEditionModule::getPaletteElement(const std::string& key)const
{
	return _Palette->getPaletteElement(key);
}

CObject* CSimClientEditionModule::newComponent(const std::string& type) const
{
	return _Factory->newComponent(type);
}

void CSimClientEditionModule::registerGenerator(CObject* classObject)
{
	_Factory->registerGenerator(classObject);
}

CPropertyAccessor& CSimClientEditionModule::getPropertyAccessor() const
{
	nlassert(_PropertyAccessor);
	return *_PropertyAccessor;
}
*/

void CSimClientEditionModule::updateScenario(CObject *scenario)
{
	nlassert(_Scenario);
	_Scenario->setHighLevel(scenario);
	std::string eid = getEid();
	_Factory->setMaxId(eid, _Scenario->getMaxId(eid));
}

void CSimClientEditionModule::setEid(const std::string & eid)
{
	_Eid = eid;
	_Factory->setPrefix(_Eid);
}
