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


#include "dynamic_scenario_service.h"

#include <sstream>
#include "game_share/dms.h"
#include "game_share/ai_wrapper.h"
#include "game_share/server_animation_module.h"
#include "game_share/server_edition_module.h"

#include "game_share/r2_ligo_config.h"
#include "game_share/ring_access.h"

#include "nel/net/unified_network.h"
#include "nel/net/module_manager.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/misc/entity_id.h"

#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"
//#include "game_share/module_security.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/chat_group.h"
#include "game_share/singleton_registry.h"

using namespace NLNET;
using namespace NLMISC;
using namespace R2;


// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

namespace R2
{
	// The ligo config
	extern NLLIGO::CLigoConfig * LigoConfigPtr;
	CR2LigoConfig R2LigoConfig;
}

void CDynamicScenarioService::forwardToStringManagerModule (CMessage &msgin)
{
	TDataSetRow senderId;
	CChatGroup::TGroupType groupType= CChatGroup::universe;
	std::string id_;
	TSessionId scenarioId;
	msgin.serial(senderId);
	msgin.serialEnum(groupType);

	msgin.serial(id_);
	msgin.serial(scenarioId);
	_Dms->translateAndForwardRequested(senderId,groupType,id_,scenarioId);
}


static  void cbForwardToStringManagerModule(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	nldebug("forwardToStringManagerModule");
	CDynamicScenarioService::instance().forwardToStringManagerModule(msgin);
}
static void cbForwardToStringManagerModuleWithArg(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	nldebug("forwardToStringManagerModuleWithArg");
	IServerAnimationModule *imodule = CDynamicMapService::getInstance()->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);
	nlassert(module);
	module->onProcessModuleMessage(0,msgin);
}

void CDynamicScenarioService::forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence)
{
	_Dms->forwardIncarnChat(id,senderId,sentence);
	nldebug("Forwarding dyn chat \"%s\" to dms",sentence.c_str());
}

static void cbDynChatForward(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{

	TChanID id;
	TDataSetRow sender;
	ucstring ucsentence;
	std::string sentence;
	CChatGroup::TGroupType groupType = CChatGroup::say;
	msgin.serial(id);
	msgin.serial(sender);
	msgin.serial(ucsentence);
	sentence = ucsentence.toString();
	CDynamicScenarioService::instance().forwardIncarnChat(id,sender,sentence);
	nldebug("forwarding dyn chat \"%s\"",sentence.c_str());
}

static void cbSessionAck(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	CDynamicMapService* dms = CDynamicMapService::getInstance();
	IServerAnimationModule *imodule = dms->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);
	module->onProcessModuleMessage(0, msgin);
}

static void cbDssStartAct(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	CDynamicScenarioService& service = CDynamicScenarioService::instance();
	CDynamicMapService* dms = CDynamicMapService::getInstance();
	IServerAnimationModule*  imodule= dms->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);
	TSessionId sessionId;

	uint32 actId;

	msgin.serial(sessionId);
	msgin.serial(actId);
	module->scheduleStartAct(sessionId, actId);
}

static void cbExecCommandResult(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// treat the rely message sent back from a service whom we asked to execute a command
	NLMISC::InfoLog->displayNL("EXEC_COMMAND_RESULT' Received from: %3d: %s", sid.get() ,serviceName.c_str());

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

static void cbBotDespawnNotification(NLNET::CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	TAIAlias alias;
	CEntityId creatureId;
	msgin.serial(alias);
	msgin.serial(creatureId);
	CDynamicMapService* dms = CDynamicMapService::getInstance();
	IServerAnimationModule*  imodule= dms->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);

	module->onBotDespawnNotification(creatureId);

}

static void cbBotDeathNotification(NLNET::CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	TAIAlias alias;
	CEntityId creatureId;
	msgin.serial(alias);
	msgin.serial(creatureId);
	CDynamicMapService* dms = CDynamicMapService::getInstance();
	IServerAnimationModule*  imodule= dms->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);

	module->onBotDeathNotification(creatureId);

}

static void cbStopNpcControlNotification(NLNET::CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	TAIAlias alias;
	CEntityId creatureId;
	msgin.serial(alias);
	msgin.serial(creatureId);
	CDynamicMapService* dms = CDynamicMapService::getInstance();
	IServerAnimationModule*  imodule= dms->getAnimationModule();
	CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);

	module->onStopNpcControlNotification(creatureId);

}

TUnifiedCallbackItem CbArray[]=
{
	{"translateAndForward", cbForwardToStringManagerModule},
	{"DYN_CHAT:FORWARD", cbDynChatForward},
	{"DSS_START_ACT", cbDssStartAct},
	{"translateAndForwardArg", cbForwardToStringManagerModuleWithArg},
	{"SESSION_ACK", cbSessionAck},
	{"EXEC_COMMAND_RESULT",	cbExecCommandResult},
	{"BOT_DESPAWN_NOTIFICATION", cbBotDespawnNotification},
	{"BOT_DEATH_NOTIFICATION", cbBotDeathNotification},
	{"BOT_STOPCCONTROL_NOTIFICATION", cbStopNpcControlNotification},
	{"", NULL}
};


void cbServiceUp(const std::string &serviceName, TServiceId serviceId, void *)
{

	CDynamicScenarioService& instance = CDynamicScenarioService::instance();
	nlinfo( "DSS: %s Service up", serviceName.c_str() );
	if ( instance.getR2Sbm())
	{
		instance.getR2Sbm()->onServiceUp(serviceName, serviceId);
	}
	if( serviceName == "AIS" )
	{
		nlinfo( "DSS: AI server up" );
		CDynamicMapService *dms = CDynamicMapService::getInstance();
		if( dms )
		{
			IServerAnimationModule *imodule = dms->getAnimationModule();
			CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);
			if( module )
			{
				nlinfo( "DSS: notifying ServerAnimModule that AI server is up" );
				module->onServiceUp( serviceName, serviceId );
			}

			{

				IServerEditionModule*imodule = dms->getEditionModule();
				CServerEditionModule* module = safe_cast<CServerEditionModule*>(imodule);
				if( module )
				{
					nlinfo( "DSS: notifying ServerEditionModule that '%s' server is up", serviceName.c_str() );
					module->onServiceUp( serviceName, serviceId );
				}
			}

		}
	}

	if( serviceName == "BS" )
	{
		instance.setBsUp(true);
		nlinfo( "DSS: %s server up", serviceName.c_str() );
		CDynamicMapService *dms = CDynamicMapService::getInstance();
		if( dms )
		{
			IServerEditionModule*imodule = dms->getEditionModule();
			CServerEditionModule* module = safe_cast<CServerEditionModule*>(imodule);
			if( module )
			{
				nlinfo( "DSS: notifying ServerEditionModule that '%s' server is up", serviceName.c_str() );
				module->onServiceUp( serviceName, serviceId );
			}
		}
	}
}

void cbServiceDown(const std::string &serviceName, TServiceId serviceId, void *)
{
	nlinfo( "DSS: %s Service down", serviceName.c_str() );

	CDynamicScenarioService& instance = CDynamicScenarioService::instance();
	nlinfo( "DSS: %s Service up", serviceName.c_str() );
	if ( instance.getR2Sbm())
	{
		instance.getR2Sbm()->onServiceDown(serviceName, serviceId);
	}
	if( serviceName == "AIS" )
	{
		nlinfo( "DSS: AI server down" );
		CDynamicMapService* dms = CDynamicMapService::getInstance();
		if( dms )
		{
			{

				IServerEditionModule*imodule = dms->getEditionModule();
				CServerEditionModule* module = safe_cast<CServerEditionModule*>(imodule);
				if( module )
				{
					nlinfo( "DSS: notifying ServerEditionModule that '%s' server is down", serviceName.c_str() );
					module->onServiceDown( serviceName, serviceId );
				}
			}

			IServerAnimationModule *imodule = dms->getAnimationModule();
			CServerAnimationModule* module = safe_cast<CServerAnimationModule*>(imodule);
			if( module )
			{
				nlinfo( "DSS: notifying ServerAnimModule that AI server is down" );
//				module->onServiceDown( serviceName, serviceId );
			}
		}
	}

	if( serviceName == "BS" )
	{
		instance.setBsUp(false);
		nlinfo( "DSS: %s server up", serviceName.c_str() );
		CDynamicMapService *dms = CDynamicMapService::getInstance();
		if( dms )
		{
			IServerEditionModule*imodule = dms->getEditionModule();
			CServerEditionModule* module = safe_cast<CServerEditionModule*>(imodule);
			if( module )
			{
				nlinfo( "DSS: notifying ServerEditionModule that '%s' server is down", serviceName.c_str() );
//				module->onServiceDown( serviceName, serviceId );
			}
		}
	}
}

void CDynamicScenarioService::init()
{
	_Dms = NULL;

	_R2Sbm = NULL;
	_Mode = Dss;
	if (haveLongArg("r2sbm"))
	{
		_Mode = R2Sbm;
	} else if (haveLongArg("NoSu"))
	{
		_Mode = DssWithNoSu;
	}

	// initialize callbacks for service up / down
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbServiceUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, NULL);

	CSingletonRegistry::getInstance()->init();

	// preset the shard ID
	if (ConfigFile.getVarPtr("ShardId") != NULL)
	{
		anticipateShardId(ConfigFile.getVarPtr("ShardId")->asInt());
	}
	else
	{
		nlwarning("No variable ShardId in config file, this could result in miss registered DSS into SU because of late WS shard id message");
	}

	setVersion (RYZOM_VERSION);

	// setup the update systems
	setUpdateTimeout(100);

	if (_Mode != R2Sbm)
	{
		CAiWrapper::setInstance(new CAiWrapperServer);

		LigoConfigPtr=&R2LigoConfig;
		// Init ligo
		if (!R2LigoConfig.readPrimitiveClass ("world_editor_classes.xml", false))
		{
			// Should be in l:\leveldesign\world_editor_files
			nlerror ("Can't load ligo primitive config file world_editor_classes.xml");
		}
		R2LigoConfig.updateDynamicAliasBitCount(16);

		// have ligo library register its own class types for its class factory
		NLLIGO::Register();

		IModuleManager &mm = IModuleManager::getInstance();
		IModule * serverGw = mm.createModule("StandardGateway", "serverGw", "");
		nlassert(serverGw != NULL);

		NLNET::IModuleSocket *socketServerGw = mm.getModuleSocket("serverGw");
		nlassert(socketServerGw != NULL);
		_Dms = new CDynamicMapService(ConfigFile, socketServerGw);
		_Dms->init2();

		// open the server gw with a layer 3 transport
		CCommandRegistry::getInstance().execute("serverGw.transportAdd L5Transport l5", *InfoLog);
		CCommandRegistry::getInstance().execute("serverGw.transportCmd l5(open)", *InfoLog);

		CSheetId::init(0);
		CRingAccess::getInstance().init();
	}
	else
	{
		CR2SessionBackupModule_WantToBeLinked();
		nldebug("Launching the module '%s'",RSMGR::CR2SessionBackupModuleClassName);
	}
}

bool CDynamicScenarioService::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	return true;
}

void CDynamicScenarioService::release()
{


	CSheetId::uninit();
	delete _Dms;
	_Dms = NULL;
	IModuleManager &mm = IModuleManager::getInstance();
	IModule * clientGw = mm.getLocalModule("serverGw");
	nlassert(clientGw != NULL);
	mm.deleteModule(clientGw);

	CSingletonRegistry::getInstance()->release();

}

//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------

static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s = "dynamic_scenario_service";


	if (theService->haveLongArg("r2sbm"))
	{
		s += std::string("_r2sbm");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s = "DSS";

	if (theService->haveLongArg("r2sbm"))
	{
		s = std::string("R2SBM");
	}
	return s.c_str();
}

NLNET_SERVICE_MAIN( CDynamicScenarioService, getShortServiceName(scn), getCompleteServiceName(scn), 0, CbArray, "", "" );

