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


#ifndef R2_DMS_H
#define R2_DMS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/net/service.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "game_share/dyn_chat.h"
#include "game_share/chat_group.h"
#include "game_share/base_types.h"

#include "game_share/r2_types.h"

#include <list>
#include <map>
#include <vector>

namespace NLNET
{
	class IModuleSocket;
	class IModuleProxy;
	class IModule;
}

namespace RSMGR
{
	struct TSessionType;
}

namespace NLMISC
{
	struct CEntityId;
}


namespace R2
{


	class CObject;
	class CUserConnectionMgr;
	class CObjectId;
	class CScenario;
	class CServerAnimationModule;
	class CServerAdminModule;
	class CServerEditionModule;
	class CStringManagerModule;
	class CAnimationMessageAnimationStart;




	class IServerEditionModule
	{
	public:
		typedef uint32 TCharId;
		// typedef CSessionId TSessionId; 	// typedef uint32 TSessionId;

	public:
		virtual ~IServerEditionModule(){}
		virtual void createSessionWithoutSu(uint32 charId, NLMISC::CEntityId clientEid) = 0;
		virtual TPioneersSessionsAllowed * getSessionAllowedForChar(TCharId charId) const = 0;
		virtual CScenario* getScenarioById(TSessionId sessionId) const = 0;
		// getEditing position (use AdminModule::getPosition for having a position in editing and animation mode)
		virtual	bool getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 locationIndex = 0) = 0;
		virtual TSessionId getSessionIdByCharId(TCharId charId) const = 0;
		virtual bool isClientAuthorized(uint32 charId) const = 0;
		virtual void disconnectChar(uint32 charId) = 0;
		virtual const NLNET::TModuleProxyPtr * getClientProxyPtr(TCharId charId) const = 0;
		virtual void tpToEntryPoint(NLNET::IModuleProxy *senderModuleProxy, uint32 actId) = 0;
		// wakup a session (load session from BS), start animation, connect waiting clients
		virtual bool wakeUpSession(TSessionId sessionId, TCharId ownerId,  std::string& msg) = 0;

		virtual bool isSessionHibernating(TSessionId sessionId, RSMGR::TSessionType& sessionType, double& x, double& y, double& orient, uint8& season) = 0;
		virtual bool isEditingSession(TSessionId sessionId) const = 0;
		virtual TSessionId  getLinkedSessionId(TSessionId sessionId) const = 0;
		virtual void getStartParams(uint32 charId, TSessionId lastStoredSessionId) = 0;
		virtual void updateCharPioneerRight(TCharId charId) = 0;
		virtual void characterReady(TCharId charId) =0;
		virtual void returnToPreviousSession(TCharId charId) = 0;


	};

	class IServerAnimationModule
	{
	public:
		typedef uint32 TCharId;
	//	typedef uint32 TSessionId;

	public:
		virtual ~IServerAnimationModule() {}
		virtual TSessionId getSessionIdByCharId(TCharId charId) const = 0;
		virtual	bool getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 locationIndex = 0) = 0;
		virtual void scheduleStartSession(const CAnimationMessageAnimationStart &msg) = 0;
		virtual void addPioneer( TSessionId sessionId, TCharId charId) = 0;
		virtual bool stopTest(TSessionId sessionId, uint32 & lastAct) = 0;
		virtual void disconnectChar(uint32 charId) = 0;
		virtual bool getConnectedChars(TSessionId sessionId, std::vector<TCharId>& chars) const = 0;
		virtual NLNET::IModule* getModule() const = 0;

		virtual bool isSessionRunning(TSessionId sessionId) const = 0;
		virtual uint32 getCurrentAct(TSessionId sessionId) const = 0;
		virtual void stopControlNpcs(TCharId charId) = 0;
		virtual void setSessionStartParams(TSessionId sessionId, sint32 x, sint32 y, uint8 season) = 0;
		virtual bool mustReloadPosition(TSessionId sessionId, TCharId charId) const = 0;
		virtual bool getHeaderInfo(TSessionId sessionId, TScenarioHeaderSerializer::TValueType& values) const = 0;
		virtual bool getScore(TSessionId sessionId, uint32 &score, NLMISC::TTime &timeTaken) = 0;
	};

	class IServerAdminModule
	{
	public:
			virtual ~IServerAdminModule() {}

			virtual bool getPosition(TSessionId sessionId, double&x, double&y, double& orient, uint8& season, uint32 locationIndex = 0) = 0;

			virtual TSessionId getSessionIdByCharId(uint32 charId) const = 0;

	};
	//-----------------------------------------------------------------------------
	// class CDynamicMapService
	//-----------------------------------------------------------------------------

	class CDynamicMapService
	{
	public:
		CDynamicMapService(	NLMISC::CConfigFile& confFile, NLNET::IModuleSocket * gateway);

		virtual ~CDynamicMapService();

		static CDynamicMapService* getInstance();

		virtual void init();

		virtual void init2();
		bool useNetwork() const { return _UseNetwork;}
		virtual void translateAndForwardRequested(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId);

		void forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence);

		IServerAnimationModule* getAnimationModule() const;

		IServerEditionModule* getEditionModule() const;

		IServerAdminModule* getAdminModule() const;

	std::string getValue(TSessionId sessionId, const std::string& msg) const;

	private:
		static CDynamicMapService* _Instance;
	//	CUserConnectionMgr* _UserConnectionMgr; will be session manager
		CServerAnimationModule* _AnimationModule;
		CServerAdminModule* _AdminModule;
		CServerEditionModule* _EditionModule;
		CStringManagerModule* _StringMgrModule;

		NLMISC::CConfigFile& _ConfigFile;
		bool _UseNetwork;

	};

	// get the security info of the module
	bool getCharInfo(NLNET::IModuleProxy *senderModuleProxy, uint32 & charId, NLMISC::CEntityId & clientEid, std::string & userPriv, std::string &extendedPriv);

	// get the security info of the module and verify that the su has allowed the client to connect to the dss
	bool checkSecurityInfo(NLNET::IModuleProxy *senderModuleProxy, uint32 & charId, NLMISC::CEntityId & clientEid, std::string & userPriv, std::string &extendedPriv);

} // namespace R2

#endif //R2_DMS_H
