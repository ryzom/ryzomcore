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

#include "simulated_dmc.h"

#include "nel/net/unified_network.h"
#include "nel/net/module_manager.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"

#include "r2_share/object.h"
#include "r2_share/scenario.h"
#include "r2_share/r2_messages.h"

#include "com_lua_module.h"
//#include "property_accessor.h"
//#include "palette.h"

// #include "client_admin_module.h"
#include "simulated_client_animation_module.h"
#include "simulated_client_edition_module.h"

#include "simulation_service.h"	// for onReceiveStopTest

#include <iostream>
#include <sstream>
#include <assert.h>


using namespace R2;
using namespace NLNET;
using namespace NLMISC;

CDynamicMapClient::CDynamicMapClient( const std::string &/*eid*/, NLNET::IModuleSocket *clientGateway, lua_State * /*luaState*/)
{
	_ComLua = NULL;
	_SocketGateway = clientGateway;

/*
	if (!luaState)
	{
		loadFeatures(); // NB nico : when used by the client,  features are loaded externally for now ...		
	}
*/	

}

void CDynamicMapClient::release()
{
	delete _ComLua;
	_ComLua = NULL;
	_EditionModule->release();
		//new CComLuaModule(this, luaState);
}

void CDynamicMapClient::init( uint id, lua_State *luaState )
{
	_Id = id;

	IModuleManager &mm = IModuleManager::getInstance();
/*
	//Admin
	IModule *clientAdminModuleInterface = mm.createModule("ClientAdminModule", "ClientAdminModule", "");
	nlassert(clientAdminModuleInterface != NULL);
	_AdminModule = safe_cast<CClientAdminModule* >(clientAdminModuleInterface);
	_AdminModule->init(clientGateway);
*/
	std::string sSCEM = NLMISC::toString( "SimClientEditionModule%d", id );
	std::string sSCAnM = NLMISC::toString( "SimClientAnimationModule%d", id );

	//Edition
	IModule *clientEditionModuleInterface = mm.createModule( "ClientEditionModule", sSCEM, "" );
	nlassert( clientEditionModuleInterface != NULL );

	_EditionModule = safe_cast<CSimClientEditionModule* >(clientEditionModuleInterface);
	_EditionModule->init( _Id, _SocketGateway, this);


	//Animation
	IModule *clientAnimationModuleInterface = mm.createModule( "ClientAnimationModule", sSCAnM, "" );
	nlassert( clientAnimationModuleInterface != NULL );

	_AnimationModule = safe_cast<CSimClientAnimationModule *>(clientAnimationModuleInterface);
	_AnimationModule->init( _SocketGateway );	//, this);

//	nlassert(luaState != 0);
	_ComLua = new CComLuaModule(this, luaState);	
//	_EditionModule->init();
}

CDynamicMapClient::~CDynamicMapClient()
{
	delete _ComLua;	
	IModuleManager &mm = IModuleManager::getInstance();
//	mm.deleteModule(_AdminModule);
	mm.deleteModule(_AnimationModule);
	mm.deleteModule(_EditionModule);
}

bool CDynamicMapClient::isSEMConnected()
{
	if( !_EditionModule )
		return false;
	
	return _EditionModule->isSEMConnected();
}

void CDynamicMapClient::save(const std::string& filename)
{
	std::string name = "";
	name += filename;
	std::ostringstream out2;
	NLMISC::COFile out(name.c_str());
	CObject* scenario =  _EditionModule->getCurrentScenario()->getHighLevel();
	if (scenario)
	{
		out2 <<"scenario = "<< *scenario ;
		std::string tmp=out2.str();
		out.serialBuffer((uint8*)tmp.c_str(),tmp.size());
		return;
	}
	nlwarning("can't save: no scenario yet");

}

void CDynamicMapClient::saveRtData(const std::string& filename)
{
	std::string name = "";
	name += filename;
	std::ostringstream out2;
	NLMISC::COFile out(name.c_str());
	CObject* scenario =  _EditionModule->getCurrentScenario()->getRtData();
	if (scenario)
	{
		out2 <<"scenario = "<< *scenario ;
		std::string tmp=out2.str();
		out.serialBuffer((uint8*)tmp.c_str(),tmp.size());
		return;
	}
	nlwarning("can't save: no scenario yet");

}
/*
void CDynamicMapClient::requestTalkAs(const std::string& npcname)
{
	_AnimationModule->requestTalkAs(npcname);
}
*/
bool  CDynamicMapClient::load(const std::string& filename)
{
	return _ComLua->load(filename);
}

CObject *CDynamicMapClient::loadScenario(const std::string& filename)
{
	return _ComLua->loadLocal(filename);
}

//void CDynamicMapClient::loadFeatures()
//{
//	_ComLua->loadFeatures();
//}

void CDynamicMapClient::show() const
{
	std::stringstream ss;
	std::string s;

	CObject* highLevel= getCurrentScenario()->getHighLevel();
	if (highLevel) { highLevel->serialize(ss); }
	else { ss << "No HL" << std::endl; }
	

	while ( std::getline(ss, s))
	{
		nlinfo("%s", s.c_str());
	}

	//nlinfo("%s", ss.str().c_str());
}

void CDynamicMapClient::testConnectionAsCreator()
{
	nlassert(0);
	_EditionModule->requestMapConnection( 1, true );
}

void CDynamicMapClient::updateScenario(CObject* scenario)
{
	_EditionModule->updateScenario(scenario);
}


void CDynamicMapClient::doFile(const std::string& filename)
{
	_ComLua->doFile(filename);
}

void CDynamicMapClient::runLuaScript(const std::string& filename)
{
	std::string errorMsg;
	_ComLua->runLuaScript(filename, errorMsg);
}

void CDynamicMapClient::addPaletteElement(const std::string& attrName, CObject* paletteElement)
{
//	_EditionModule->addPaletteElement(attrName, paletteElement);
	nlassert( "unimplemented method reached in sim dmc: addPaletteElement" );
}

CObject* CDynamicMapClient::getPropertyValue(CObject* component,  const std::string& attrName) const
{
//	return _EditionModule->getPropertyValue(component,  attrName);
	nlassert( "unimplemented method reached in sim dmc: getPropertyValue" );
	return NULL;
}

CObject* CDynamicMapClient::getPropertyList(CObject* component) const
{
//	return _EditionModule->getPropertyList(component);
	nlassert( "unimplemented method reached in sim dmc: getPropertyList" );
	return NULL;
}

CObject* CDynamicMapClient::getPaletteElement(const std::string& key)const
{
//	return _EditionModule->getPaletteElement(key);
	nlassert( "unimplemented method reached in sim dmc: getPaletteElement" );
	return NULL;
}


CObject* CDynamicMapClient::newComponent(const std::string& type) const
{
//	return _EditionModule->newComponent(type);
	nlassert( "unimplemented method reached in sim dmc: newComponent" );
	return NULL;
}

void CDynamicMapClient::registerGenerator(CObject* classObject)
{
//	_EditionModule->registerGenerator(classObject);
	nlassert( "unimplemented method reached in sim dmc: registerGenerator" );
}

void CDynamicMapClient::requestTranslateFeatures()
{
	_ComLua->callTranslateFeatures(_EditionModule->getCurrentScenario()->getHighLevel());	
}

void CDynamicMapClient::requestGoTest()
{
	_EditionModule->requestGoTest();
}

void CDynamicMapClient::requestStartAct( uint actId )
{
	_AnimationModule->requestStartAct( actId );
}

void CDynamicMapClient::requestStopTest()
{
	_EditionModule->requestStopTest();
}

void CDynamicMapClient::requestUpdateRtScenario(CObject* scenario)
{
	_EditionModule->requestUpdateRtScenario(scenario);
}

void CDynamicMapClient::requestCreateScenario(CObject* scenario)
{
	_EditionModule->requestCreateScenario(scenario);
}

void CDynamicMapClient::requestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value)
{
//	_EditionModule->requestInsertNode(instanceId, name, position ,key, value);
	nlassert( "unimplemented method reached in sim dmc: requestInsertNode" );
}

void CDynamicMapClient::requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
//	_EditionModule->requestSetNode(instanceId, attrName, value);
	nlassert( "unimplemented method reached in sim dmc: requestSetNode" );
}

void CDynamicMapClient::requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
//	_EditionModule->requestEraseNode(instanceId, attrName, position);
	nlassert( "unimplemented method reached in sim dmc: requestEraseNode" );
}

void CDynamicMapClient::requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
//	_EditionModule->requestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
	nlassert( "unimplemented method reached in sim dmc: requestMoveNode" );
}

void CDynamicMapClient::nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	getCurrentScenario()->setNode(instanceId, attrName,value);
}

void CDynamicMapClient::nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	getCurrentScenario()->eraseNode(instanceId, attrName, position);
}

void CDynamicMapClient::nodeInserted(const std::string& instanceId, const std::string& attrName, sint32 position,
	const std::string& key, CObject* value)
{
	getCurrentScenario()->insertNode(instanceId, attrName, position, key,value);
}

void CDynamicMapClient::nodeMoved(const std::string& instanceId, const std::string& attrName, sint32 position,
	const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	getCurrentScenario()->moveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}

void CDynamicMapClient::scenarioUpdated(CObject* highLevel)
{
	updateScenario(highLevel);
}

CObject* CDynamicMapClient::find(const std::string& instanceId)
{
	nlassert(_EditionModule->getCurrentScenario());
	return _EditionModule->getCurrentScenario()->find(instanceId);
}

const CObject *CDynamicMapClient::getHighLevel() const
{
	nlassert(getCurrentScenario());
	return getCurrentScenario()->getHighLevel();
}
/*
CPropertyAccessor &CDynamicMapClient::getPropertyAccessor() const
{	
	return _EditionModule->getPropertyAccessor();
}
*/

void CDynamicMapClient::disconnect()
{
	//delete _Scenario;//??
	//_Scenario=new CScenario(0);
//	_ComModule->requestDisconnect(_Eid);
}

	

CScenario* CDynamicMapClient::getCurrentScenario() const
{
	return _EditionModule->getCurrentScenario();
}




CSimClientEditionModule& CDynamicMapClient::getEditionModule() const
{
	nlassert(_EditionModule);
	return *_EditionModule;
}

CSimClientAnimationModule& CDynamicMapClient::getAnimationModule() const
{
	nlassert(_AnimationModule);
	return *_AnimationModule;
}
/*

CClientAdminModule& CDynamicMapClient::getAdminModule() const
{
	nlassert(_AdminModule);
	return *_AdminModule;
}
*/

void CDynamicMapClient::requestUploadScenario(CObject* scenario)
{
	_EditionModule->requestUploadScenario(scenario);
}

void CDynamicMapClient::onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel)
{
	nlinfo( "DMC %d: onEditionModeConnected user %d, adventure %d", _Id, userSlotId, adventureId );
	_EditionModule->setEid(NLMISC::toString("SimClientEditionModule%d", userSlotId));	
	scenarioUpdated(highLevel);
}

void CDynamicMapClient::onEditionModeDisconnected()
{
}

void CDynamicMapClient::onResetEditionMode()
{
	// 
}


void CDynamicMapClient::onTestModeConnected()
{
	_AnimationModule->requestStartAct(1);
}

void CDynamicMapClient::onTestModeDisconnected()
{
	CSimulationService::getSS().onReceiveStopTest( _Id );
}

CObject* CDynamicMapClient::getCurrentScenarioHighLevel()
{
	return _EditionModule->getCurrentScenario()->getHighLevel();
}

/*
void CDynamicMapClient::onNpcAnimationTargeted(uint32 mode)
{
	nlinfo("R2Cl: DSS_TARGET");

	if (mode & CAnimationProp::TalkAs)
	{
		nlinfo("R2Cl: /a dssTaget TALK_AS");
	}
	if (mode & CAnimationProp::DespawnNpc)
	{
		nlinfo("R2Cl: /a dssTaget DESPAWN_NPC");
	}
	if (mode & CAnimationProp::AddAggro)
	{
		nlinfo("R2Cl: /a dssTaget AGGRO_RANGE_BIG");
		nlinfo("R2Cl: /a dssTaget AGGRO_RANGE_NORMAL");
	}
	if (mode & CAnimationProp::RemoveAggro)
	{
		nlinfo("R2Cl: /a dssTaget AGGRO_RANGE_SMALL");
		nlinfo("R2Cl: /a dssTaget AGGRO_RANGE_NONE");
	}
	if (mode & CAnimationProp::AddHp)
	{
			nlinfo("R2Cl: /a dssTaget ADD_HP");
	}

			
}
*/