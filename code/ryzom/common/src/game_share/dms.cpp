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

#include "stdpch.h"

#include "dms.h"
#include "nel/misc/debug.h"
#include "user_connection_mgr.h"
#include "server_animation_module.h"
#include "server_admin_module.h"
#include "server_edition_module.h"
#include "string_mgr_module.h"

#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/ring_access.h"

#include "nel/net/module_common.h"
#include "nel/net/module_manager.h"
#include "nel/net/module.h"
#include "nel/net/inet_address.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_message.h"

#include "game_share/module_security.h"

#include "server_admin_module.h"
#include "server_edition_module.h"
#include "server_animation_module.h"


using namespace R2;
using namespace NLNET;
using namespace NLMISC;
using namespace std;

extern	CVariable<uint32> TimeBeforeDisconnectionAfterKick;

// If true load hibernating session and saved session at startup
extern	CVariable<bool> DssUseBs;
// 1 minutes
extern	CVariable<uint32> TimeBeforeAutoSaveTimeEditionSession;
// 5 minutes
extern	CVariable<uint32> TimeBeforeStopTestInEditionSessionWithNoPlayer;
// 15 minutes
extern	CVariable<uint32> TimeBeforeAutoHibernateEditionSessionWithNoPlayer;
// 7 days
extern	CVariable<uint32> TimeBeforeCloseHibernatingEditionSession;

extern	CVariable<uint32> TimeBeforeAutoCloseAnimationSessionWithNoPlayer;




CDynamicMapService* CDynamicMapService::_Instance=0;

CDynamicMapService::CDynamicMapService(	NLMISC::CConfigFile& confFile, NLNET::IModuleSocket * clientGateway)
:_ConfigFile(confFile)
{
//	_UserConnectionMgr = new CUserConnectionMgr(this);

	_UseNetwork = false;
	nlassert(!_Instance);
	_Instance = this;

	_AnimationModule = 0;
	_EditionModule = 0;
	_AdminModule = 0;
	_StringMgrModule = 0;

	IModuleManager &mm = IModuleManager::getInstance();
	{
		IModule *imodule = mm.createModule("ServerAnimationModule", "ServerAnimationModule", "");
		nlassert(imodule != NULL);
		_AnimationModule = safe_cast<CServerAnimationModule* >(imodule);
		_AnimationModule->init(clientGateway, this);
	}
	{
		IModule *imodule = mm.createModule("ServerAdminModule", "ServerAdminModule", "");
		nlassert(imodule != NULL);
		_AdminModule = safe_cast<CServerAdminModule* >(imodule);
		_AdminModule->init(clientGateway, this);
	}
	{
		IModule *imodule = mm.createModule("ServerEditionModule", "ServerEditionModule", "");
		nlassert(imodule != NULL);
		_EditionModule = safe_cast<CServerEditionModule* >(imodule);
		_EditionModule->init(clientGateway, this);
	}
	{
		IModule *imodule = mm.createModule("StringManagerModule", "StringManagerModule", "");
		nlassert(imodule != NULL);
		_StringMgrModule = safe_cast<CStringManagerModule* >(imodule);
		_StringMgrModule->init(clientGateway,this);
	}


}

CDynamicMapService::~CDynamicMapService()
{
	IModuleManager &mm = IModuleManager::getInstance();
//	_EditionModule->saveToDb();
	mm.deleteModule(_EditionModule);
	_EditionModule = 0;
	mm.deleteModule(_AnimationModule);
	_AnimationModule = 0;
	mm.deleteModule(_StringMgrModule);
	_StringMgrModule = 0;
	mm.deleteModule(_AdminModule);
	_AdminModule = 0;
	_Instance = 0;
}

CDynamicMapService* CDynamicMapService::getInstance()
{
	return _Instance;
}

void CDynamicMapService::init()
{
	_UseNetwork = false;

	DssUseBs = 0;

	TimeBeforeAutoSaveTimeEditionSession = 0;

	TimeBeforeStopTestInEditionSessionWithNoPlayer = 0;

	TimeBeforeAutoHibernateEditionSessionWithNoPlayer = 0;

	TimeBeforeCloseHibernatingEditionSession = 0;

	TimeBeforeAutoCloseAnimationSessionWithNoPlayer = 0;

}

void CDynamicMapService::init2()
{
	_UseNetwork = true;
}

void CDynamicMapService::translateAndForwardRequested(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId)
{
	_StringMgrModule->translateAndForward(senderId,groupType,id,sessionId);
}

void CDynamicMapService::forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence)
{
	_StringMgrModule->forwardIncarnChat(id, senderId,sentence);
}

IServerAnimationModule* CDynamicMapService::getAnimationModule() const { return _AnimationModule;}

IServerEditionModule* CDynamicMapService::getEditionModule() const { return _EditionModule;}

IServerAdminModule* CDynamicMapService::getAdminModule() const { return _AdminModule;}

std::string CDynamicMapService::getValue(TSessionId sessionId, const std::string& msg) const
{
	if (_StringMgrModule)
	{
		return _StringMgrModule->getValue(sessionId.asInt(), msg);
	}
	return msg;
}

namespace R2
{

	bool getCharInfo(NLNET::IModuleProxy *senderModuleProxy, uint32 & charId, NLMISC::CEntityId & clientEid, std::string & userPriv, std::string &extendedPriv)
	{
		const NLNET::TSecurityData *securityData = senderModuleProxy->findSecurityData(rmst_client_info);

		if (!CDynamicMapService::getInstance()->useNetwork())
		{
	//		userId = 999<;
			charId = 999<<4;

			clientEid = CEntityId::Unknown;
			clientEid.setShortId(charId);
			clientEid.setDynamicId(0x86); //FrontendId
			clientEid.setType(RYZOMID::player);

			userPriv =":DEV:";
			extendedPriv = "";
			return true;
		}

		if (securityData != NULL)
		{
			const struct TClientInfo* clientInfo= static_cast<const struct TClientInfo *>(securityData); //safe_cast ???
			//userId = clientInfo->UserId;
			charId = uint32(clientInfo->ClientEid.getShortId());
			clientEid = clientInfo->ClientEid;
			userPriv = clientInfo->UserPriv;
			extendedPriv = clientInfo->ExtendedPriv;
			return true;
		}
		else
		{
			return false;
		}
	}


	// same getCharInfo but return true only if client has been allowad by su to connect
	bool checkSecurityInfo(NLNET::IModuleProxy *senderModuleProxy, uint32 & charId, NLMISC::CEntityId & clientEid, std::string & userPriv, std::string &extendedPriv)
	{
		bool ok =  getCharInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
		if (!ok )
		{
			nlwarning("Warning: Security issues: a client '%s' without security data try to connect... ", senderModuleProxy->getModuleName().c_str() );
			return false;
		}
		IServerEditionModule* edition = CDynamicMapService::getInstance()->getEditionModule();
		if (!edition)
		{
			return false;
		}
		ok = edition->isClientAuthorized(charId);
		if (!ok)
		{
			nlwarning( "Warning: A client '%s' '%s' %u '%s' try to send messages, without begin allowed (he was disconnected)",
				senderModuleProxy->getModuleName().c_str(), clientEid.toString().c_str(),  charId, userPriv.c_str());

			edition->disconnectChar(charId);
			edition->returnToPreviousSession(charId);

			return false;
		}
		return true;
	}


} // namespace R2


