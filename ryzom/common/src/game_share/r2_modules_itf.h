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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef R2_MODULES_ITF
#define R2_MODULES_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/far_position.h"

#include "game_share/r2_share_itf.h"

#include "game_share/r2_types.h"

#include "game_share/base_types.h"

#include "game_share/misc_const.h"

namespace R2
{

	class TR2SbmSessionInfo;


	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerEditionItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CServerEditionItfSkel>	TInterceptor;
	protected:
		CServerEditionItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerEditionItfSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CServerEditionItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CServerEditionItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerEditionItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CServerEditionItfSkel	*_LocalModuleSkel;


	public:
		CServerEditionItfProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "ServerEditionItf");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CServerEditionItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CServerEditionItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}





	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerAnimationItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CServerAnimationItfSkel>	TInterceptor;
	protected:
		CServerAnimationItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerAnimationItfSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CServerAnimationItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void getStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void askSetUserCharActPosition_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void activateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void dssMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setScenarioPoints_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void startScenarioTiming_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void endScenarioTiming_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void deactivateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onEasterEggLooted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onCharTargetReceived_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void characterReady_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CServerAnimationItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Ask for the position, season and adventure mode of a connecting character
		// The reply will either give a new position or tell to load the last stored one
		// Used by the EGS of a Ring shard to send the start position to a connecting client
		virtual void getStartParams(NLNET::IModuleProxy *sender, uint32 charId, TSessionId lastStoredSessionId) =0;
		// Ask to server animation module to re-send usre char entry point
		// The reply call CCharacterControlItf::setUserCharActPosition
		virtual void askSetUserCharActPosition(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// AIS Message to activate a scenario generated easter egg
		virtual void activateEasterEgg(NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string &name, const std::string &look) =0;
		// AIS Message to make the dss send a message
		virtual void dssMessage(NLNET::IModuleProxy *sender, TSessionId sessionId, const std::string &mode, const std::string &who, const std::string &msg) =0;
		// AIS Message to make the dss set the scenario points
		virtual void setScenarioPoints(NLNET::IModuleProxy *sender, TSessionId sessionId, float scenarioPoints) =0;
		// AIS Message to make the dss start scenario timing
		virtual void startScenarioTiming(NLNET::IModuleProxy *sender, TSessionId sessionId) =0;
		// AIS Message to make the dss end scenario timing
		virtual void endScenarioTiming(NLNET::IModuleProxy *sender, TSessionId sessionId) =0;
		// AIS Message to activate a scenario generated easter egg
		virtual void deactivateEasterEgg(NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId) =0;
		// EGS Message to indicates that an easter egg is looted
		virtual void onEasterEggLooted(NLNET::IModuleProxy *sender, uint32 eggId, TSessionId scenarioId) =0;
		// EGS message to indicates info of the target of a player
		virtual void onCharTargetReceived(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, uint32 creatureAlias, TDataSetRow creatureRowId, const ucstring &name, uint32 nameId, const std::vector<std::string> &params, bool alived) =0;
		// AIS message to ask the dss to teleport a character to a position
		virtual void teleportCharacter(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &player, float x, float y, float z) =0;
		// EGS message to indicates that a character is ready in mirror
		virtual void characterReady(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEid) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerAnimationItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CServerAnimationItfSkel	*_LocalModuleSkel;


	public:
		CServerAnimationItfProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "ServerAnimationModule");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CServerAnimationItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CServerAnimationItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// Ask for the position, season and adventure mode of a connecting character
		// The reply will either give a new position or tell to load the last stored one
		// Used by the EGS of a Ring shard to send the start position to a connecting client
		void getStartParams(NLNET::IModule *sender, uint32 charId, TSessionId lastStoredSessionId);
		// Ask to server animation module to re-send usre char entry point
		// The reply call CCharacterControlItf::setUserCharActPosition
		void askSetUserCharActPosition(NLNET::IModule *sender, uint32 charId);
		// AIS Message to activate a scenario generated easter egg
		void activateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string &name, const std::string &look);
		// AIS Message to make the dss send a message
		void dssMessage(NLNET::IModule *sender, TSessionId sessionId, const std::string &mode, const std::string &who, const std::string &msg);
		// AIS Message to make the dss set the scenario points
		void setScenarioPoints(NLNET::IModule *sender, TSessionId sessionId, float scenarioPoints);
		// AIS Message to make the dss start scenario timing
		void startScenarioTiming(NLNET::IModule *sender, TSessionId sessionId);
		// AIS Message to make the dss end scenario timing
		void endScenarioTiming(NLNET::IModule *sender, TSessionId sessionId);
		// AIS Message to activate a scenario generated easter egg
		void deactivateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId);
		// EGS Message to indicates that an easter egg is looted
		void onEasterEggLooted(NLNET::IModule *sender, uint32 eggId, TSessionId scenarioId);
		// EGS message to indicates info of the target of a player
		void onCharTargetReceived(NLNET::IModule *sender, const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, uint32 creatureAlias, TDataSetRow creatureRowId, const ucstring &name, uint32 nameId, const std::vector<std::string> &params, bool alived);
		// AIS message to ask the dss to teleport a character to a position
		void teleportCharacter(NLNET::IModule *sender, const NLMISC::CEntityId &player, float x, float y, float z);
		// EGS message to indicates that a character is ready in mirror
		void characterReady(NLNET::IModule *sender, const NLMISC::CEntityId &charEid);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_getStartParams(NLNET::CMessage &__message, uint32 charId, TSessionId lastStoredSessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_askSetUserCharActPosition(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_activateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string &name, const std::string &look);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_dssMessage(NLNET::CMessage &__message, TSessionId sessionId, const std::string &mode, const std::string &who, const std::string &msg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setScenarioPoints(NLNET::CMessage &__message, TSessionId sessionId, float scenarioPoints);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startScenarioTiming(NLNET::CMessage &__message, TSessionId sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_endScenarioTiming(NLNET::CMessage &__message, TSessionId sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_deactivateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 actId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onEasterEggLooted(NLNET::CMessage &__message, uint32 eggId, TSessionId scenarioId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onCharTargetReceived(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, uint32 creatureAlias, TDataSetRow creatureRowId, const ucstring &name, uint32 nameId, const std::vector<std::string> &params, bool alived);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportCharacter(NLNET::CMessage &__message, const NLMISC::CEntityId &player, float x, float y, float z);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_characterReady(NLNET::CMessage &__message, const NLMISC::CEntityId &charEid);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharacterControlItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CCharacterControlItfSkel>	TInterceptor;
	protected:
		CCharacterControlItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CCharacterControlItfSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CCharacterControlItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void setUserCharStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void charJoinAnimSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void charLeaveAnimSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setUserCharActPosition_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void animSessionStarted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void animSessionEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void scenarioEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void sendItemDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void activateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void deactivateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void deactivateEasterEggs_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void sendCharTargetToDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onTpPositionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void disconnectChar_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void returnToPreviousSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setPioneerRight_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportCharacterToNpc_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setUserCharCurrentSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportLinkedSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportUnlinkedSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void giveRewardMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportNpcControl_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportStopNpcControl_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void subscribeCharacterInRingUniverse_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void unsubscribeCharacterInRingUniverse_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CCharacterControlItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The reply of CServerAnimationItf::getStartParams. If reloadPos is true,
		// the character will start from his current saved pos, otherwise the character
		// will start at farPos. In all cases farPos is the respawn point to set.
		virtual void setUserCharStartParams(NLNET::IModuleProxy *sender, uint32 charId, const CFarPosition &farPos, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role) =0;
		// A character enter an anim session as player
		virtual void charJoinAnimSession(NLNET::IModuleProxy *sender, uint32 charId, uint32 sessionId) =0;
		// A character leave an anim session as player
		virtual void charLeaveAnimSession(NLNET::IModuleProxy *sender, uint32 charId, uint32 sessionId) =0;
		// The reply of CServerAnimationItf::startAct telling to teleport user
		virtual void setUserCharActPosition(NLNET::IModuleProxy *sender, uint32 charId, const CFarPosition &farPos, uint8 season) =0;
		// A DSS to EGS signal that an anim session is started
		virtual void animSessionStarted(NLNET::IModuleProxy *sender, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo) =0;
		// A DSS to EGS signal that an anim session is ended
		virtual void animSessionEnded(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken) =0;
		// A DSS Message to signal that a session is ended
		virtual void scenarioEnded(NLNET::IModuleProxy *sender, TSessionId sessionId) =0;
		// A DSS Message to register mission item of a scenario
		virtual void sendItemDescription(NLNET::IModuleProxy *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem) =0;
		// AIS Message to activate a scenario generated easter egg
		virtual void activateEasterEgg(NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector<R2::TItemAndQuantity> &items, const CFarPosition &pos, const std::string &name, const std::string &look) =0;
		// AIS Message to deactivate a scenario generated easter egg
		virtual void deactivateEasterEgg(NLNET::IModuleProxy *sender, uint32 easterEggId, TSessionId scenarioId) =0;
		// AIS Message to deactivate a multiple easterEgg scenario generated easter egg
		virtual void deactivateEasterEggs(NLNET::IModuleProxy *sender, const std::set<uint32> &items, TSessionId scenarioId) =0;
		// DSS message to ask info of the target of a player
		virtual void sendCharTargetToDss(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &eid, const std::vector<std::string> &params) =0;
		// DSS message to ask the tp of a pioneer
		virtual void onTpPositionAsked(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos &teleportInfos) =0;
		// DSS message to ask to disconnect a char
		virtual void disconnectChar(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// DSS message to ask the egs to return a player to mainland
		virtual void returnToPreviousSession(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// DSS message to ask the egs to set DM righ (aggro, visible, god)
		virtual void setPioneerRight(NLNET::IModuleProxy *sender, uint32 charId, bool isDM) =0;
		// DSS message to ask the egs to teleport a character to another character
		virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, uint32 sourceId, uint32 destId, uint8 season) =0;
		// DSS message to ask the egs to teleport a character to a npc
		virtual void teleportCharacterToNpc(NLNET::IModuleProxy *sender, uint32 sourceId, const NLMISC::CEntityId &destEid, uint8 season) =0;
		// DSS message to update the respawn point
		virtual void setUserCharCurrentSession(NLNET::IModuleProxy *sender, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role) =0;
		// DSS message to indicates to the egs that session may be linked
		virtual void reportLinkedSession(NLNET::IModuleProxy *sender, TSessionId editionSession, TSessionId animationSession) =0;
		// DSS message to indicates to the egs that linked session are no more linked
		virtual void reportUnlinkedSession(NLNET::IModuleProxy *sender, TSessionId editionSession, TSessionId animationSession) =0;
		// AIS Message to give some reward to the player
		virtual void giveRewardMessage(NLNET::IModuleProxy *sender, TDataSetRow characterRowId, TDataSetRow creatureRowId, const std::string &rewardText, const std::string &rareRewardText, const std::string &inventoryFullText, const std::string &notEnoughPointsText) =0;
		// AIS Message to indicates that a bot is being controled
		virtual void reportNpcControl(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid) =0;
		// AIS Message to indicates that a bot is stop being controled
		virtual void reportStopNpcControl(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid) =0;
		// DSS ask to put a character in the ring universe channel
		// This is for editors and animator characters only
		virtual void subscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// DSS ask to remove a character from the ring universe channel
		// This is for editors and animator characters only
		virtual void unsubscribeCharacterInRingUniverse(NLNET::IModuleProxy *sender, uint32 charId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharacterControlItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CCharacterControlItfSkel	*_LocalModuleSkel;


	public:
		CCharacterControlItfProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CCharacterControlItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CCharacterControlItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The reply of CServerAnimationItf::getStartParams. If reloadPos is true,
		// the character will start from his current saved pos, otherwise the character
		// will start at farPos. In all cases farPos is the respawn point to set.
		void setUserCharStartParams(NLNET::IModule *sender, uint32 charId, const CFarPosition &farPos, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role);
		// A character enter an anim session as player
		void charJoinAnimSession(NLNET::IModule *sender, uint32 charId, uint32 sessionId);
		// A character leave an anim session as player
		void charLeaveAnimSession(NLNET::IModule *sender, uint32 charId, uint32 sessionId);
		// The reply of CServerAnimationItf::startAct telling to teleport user
		void setUserCharActPosition(NLNET::IModule *sender, uint32 charId, const CFarPosition &farPos, uint8 season);
		// A DSS to EGS signal that an anim session is started
		void animSessionStarted(NLNET::IModule *sender, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo);
		// A DSS to EGS signal that an anim session is ended
		void animSessionEnded(NLNET::IModule *sender, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken);
		// A DSS Message to signal that a session is ended
		void scenarioEnded(NLNET::IModule *sender, TSessionId sessionId);
		// A DSS Message to register mission item of a scenario
		void sendItemDescription(NLNET::IModule *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem);
		// AIS Message to activate a scenario generated easter egg
		void activateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector<R2::TItemAndQuantity> &items, const CFarPosition &pos, const std::string &name, const std::string &look);
		// AIS Message to deactivate a scenario generated easter egg
		void deactivateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId);
		// AIS Message to deactivate a multiple easterEgg scenario generated easter egg
		void deactivateEasterEggs(NLNET::IModule *sender, const std::set<uint32> &items, TSessionId scenarioId);
		// DSS message to ask info of the target of a player
		void sendCharTargetToDss(NLNET::IModule *sender, const NLMISC::CEntityId &eid, const std::vector<std::string> &params);
		// DSS message to ask the tp of a pioneer
		void onTpPositionAsked(NLNET::IModule *sender, const NLMISC::CEntityId &eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos &teleportInfos);
		// DSS message to ask to disconnect a char
		void disconnectChar(NLNET::IModule *sender, uint32 charId);
		// DSS message to ask the egs to return a player to mainland
		void returnToPreviousSession(NLNET::IModule *sender, uint32 charId);
		// DSS message to ask the egs to set DM righ (aggro, visible, god)
		void setPioneerRight(NLNET::IModule *sender, uint32 charId, bool isDM);
		// DSS message to ask the egs to teleport a character to another character
		void teleportOneCharacterToAnother(NLNET::IModule *sender, uint32 sourceId, uint32 destId, uint8 season);
		// DSS message to ask the egs to teleport a character to a npc
		void teleportCharacterToNpc(NLNET::IModule *sender, uint32 sourceId, const NLMISC::CEntityId &destEid, uint8 season);
		// DSS message to update the respawn point
		void setUserCharCurrentSession(NLNET::IModule *sender, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role);
		// DSS message to indicates to the egs that session may be linked
		void reportLinkedSession(NLNET::IModule *sender, TSessionId editionSession, TSessionId animationSession);
		// DSS message to indicates to the egs that linked session are no more linked
		void reportUnlinkedSession(NLNET::IModule *sender, TSessionId editionSession, TSessionId animationSession);
		// AIS Message to give some reward to the player
		void giveRewardMessage(NLNET::IModule *sender, TDataSetRow characterRowId, TDataSetRow creatureRowId, const std::string &rewardText, const std::string &rareRewardText, const std::string &inventoryFullText, const std::string &notEnoughPointsText);
		// AIS Message to indicates that a bot is being controled
		void reportNpcControl(NLNET::IModule *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid);
		// AIS Message to indicates that a bot is stop being controled
		void reportStopNpcControl(NLNET::IModule *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid);
		// DSS ask to put a character in the ring universe channel
		// This is for editors and animator characters only
		void subscribeCharacterInRingUniverse(NLNET::IModule *sender, uint32 charId);
		// DSS ask to remove a character from the ring universe channel
		// This is for editors and animator characters only
		void unsubscribeCharacterInRingUniverse(NLNET::IModule *sender, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setUserCharStartParams(NLNET::CMessage &__message, uint32 charId, const CFarPosition &farPos, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_charJoinAnimSession(NLNET::CMessage &__message, uint32 charId, uint32 sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_charLeaveAnimSession(NLNET::CMessage &__message, uint32 charId, uint32 sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setUserCharActPosition(NLNET::CMessage &__message, uint32 charId, const CFarPosition &farPos, uint8 season);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_animSessionStarted(NLNET::CMessage &__message, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_animSessionEnded(NLNET::CMessage &__message, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_scenarioEnded(NLNET::CMessage &__message, TSessionId sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sendItemDescription(NLNET::CMessage &__message, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_activateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector<R2::TItemAndQuantity> &items, const CFarPosition &pos, const std::string &name, const std::string &look);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_deactivateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_deactivateEasterEggs(NLNET::CMessage &__message, const std::set<uint32> &items, TSessionId scenarioId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sendCharTargetToDss(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, const std::vector<std::string> &params);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onTpPositionAsked(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos &teleportInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_disconnectChar(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_returnToPreviousSession(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setPioneerRight(NLNET::CMessage &__message, uint32 charId, bool isDM);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, uint32 sourceId, uint32 destId, uint8 season);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportCharacterToNpc(NLNET::CMessage &__message, uint32 sourceId, const NLMISC::CEntityId &destEid, uint8 season);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setUserCharCurrentSession(NLNET::CMessage &__message, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportLinkedSession(NLNET::CMessage &__message, TSessionId editionSession, TSessionId animationSession);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportUnlinkedSession(NLNET::CMessage &__message, TSessionId editionSession, TSessionId animationSession);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_giveRewardMessage(NLNET::CMessage &__message, TDataSetRow characterRowId, TDataSetRow creatureRowId, const std::string &rewardText, const std::string &rareRewardText, const std::string &inventoryFullText, const std::string &notEnoughPointsText);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportNpcControl(NLNET::CMessage &__message, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportStopNpcControl(NLNET::CMessage &__message, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_subscribeCharacterInRingUniverse(NLNET::CMessage &__message, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_unsubscribeCharacterInRingUniverse(NLNET::CMessage &__message, uint32 charId);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAisControlItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CAisControlItfSkel>	TInterceptor;
	protected:
		CAisControlItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CAisControlItfSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CAisControlItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CAisControlItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAisControlItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CAisControlItfSkel	*_LocalModuleSkel;


	public:
		CAisControlItfProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CAisControlItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CAisControlItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}





	};
	// Info about a connected character, used for block tranfert
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TR2SbmSessionInfo
	{
	protected:
		// The SessionId
		TSessionId	_SessionId;
		// the date of last disconnection of the last character
		uint32	_DateEmpty;
	public:
		// The SessionId
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

				_SessionId = value;

		}
			// the date of last disconnection of the last character
		uint32 getDateEmpty() const
		{
			return _DateEmpty;
		}

		void setDateEmpty(uint32 value)
		{

				_DateEmpty = value;

		}

		bool operator == (const TR2SbmSessionInfo &other) const
		{
			return _SessionId == other._SessionId
				&& _DateEmpty == other._DateEmpty;
		}


		// constructor
		TR2SbmSessionInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_SessionId);
			s.serial(_DateEmpty);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CR2SessionBackupModuleItfSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CR2SessionBackupModuleItfSkel>	TInterceptor;
	protected:
		CR2SessionBackupModuleItfSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CR2SessionBackupModuleItfSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CR2SessionBackupModuleItfSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void reportDeletedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportHibernatedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportSavedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void registerDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CR2SessionBackupModuleItfSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// DSS message to report session backup that have been deleteed
		virtual void reportDeletedSessions(NLNET::IModuleProxy *sender, const std::vector<TSessionId> &sessionIds) =0;
		// DSS message to report session backup that have been hibernated
		virtual void reportHibernatedSessions(NLNET::IModuleProxy *sender, const std::vector<TSessionId> &sessionIds) =0;
		// DSS message to report session backup that have been deleteed
		virtual void reportSavedSessions(NLNET::IModuleProxy *sender, const std::vector< TR2SbmSessionInfo > &sessionInfos) =0;
		// DSS message to register itself to the R2SBM
		virtual void registerDss(NLNET::IModuleProxy *sender, TShardId shardId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CR2SessionBackupModuleItfProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CR2SessionBackupModuleItfSkel	*_LocalModuleSkel;


	public:
		CR2SessionBackupModuleItfProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CR2SessionBackupModuleItfSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CR2SessionBackupModuleItfProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// DSS message to report session backup that have been deleteed
		void reportDeletedSessions(NLNET::IModule *sender, const std::vector<TSessionId> &sessionIds);
		// DSS message to report session backup that have been hibernated
		void reportHibernatedSessions(NLNET::IModule *sender, const std::vector<TSessionId> &sessionIds);
		// DSS message to report session backup that have been deleteed
		void reportSavedSessions(NLNET::IModule *sender, const std::vector< TR2SbmSessionInfo > &sessionInfos);
		// DSS message to register itself to the R2SBM
		void registerDss(NLNET::IModule *sender, TShardId shardId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportDeletedSessions(NLNET::CMessage &__message, const std::vector<TSessionId> &sessionIds);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportHibernatedSessions(NLNET::CMessage &__message, const std::vector<TSessionId> &sessionIds);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportSavedSessions(NLNET::CMessage &__message, const std::vector< TR2SbmSessionInfo > &sessionInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerDss(NLNET::CMessage &__message, TShardId shardId);




	};

}

#endif
