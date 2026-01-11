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


#ifndef R2_SERVER_ANIMATION_MODULE_H
#define R2_SERVER_ANIMATION_MODULE_H

#include <queue>
#include <memory>

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/smart_ptr.h"

#include "nel/net/module_manager.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "nel/ligo/primitive.h"

#include "game_share/dyn_chat.h"
#include "game_share/task_list.h"

#include "r2_modules_itf.h"
#include "game_share/r2_share_itf.h"
#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/small_string_manager.h"
#include "game_share/r2_basic_types.h"

#include "dms.h"

class CPersistentDataRecord;
class CPersistentDataRecordRyzomStore;

namespace NLLIGO
{
	class CPrimitives;
}

namespace R2
{
	class CObject;
	class CDynamicMapService;
	class CAnimationSession;
	class CRtNpc;
	class CRtGrp;

	/** Handle the animation Mode (Play Mode and DM Mode in animation session, DM/Test Mode in Edition Session).
	* Creations from RT data of Primitives that are read by AI
	* start/stop of act
	* DM functions (Kill, Heal)
	*/

	class CServerAnimationModule :
		public IServerAnimationModule,
		public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav <NLNET::CModuleBase> > >,
		public CServerAnimationItfSkel,
		public CShareServerAnimationItfSkel
	{
	public:
		//typedef uint32 TCharId;
//		typedef uint32 TUserId;
//		typedef uint32 TSessionId;
		typedef uint32 TActId;

	public:
		CServerAnimationModule();
		~CServerAnimationModule();
		void init(NLNET::IModuleSocket* gateway, CDynamicMapService* server);


		/////////////////////////////////////////////////////
		//// CModuleBase API
		/////////////////////////////////////////////////////

		virtual void onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId);
		virtual void onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId);
		virtual void onModuleUpdate();
		virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);
		virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);
		virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);
		virtual bool isImmediateDispatchingSupported() const { return false; }


		/////////////////////////////////////////////////////
		//// Start / Stop session, act, scenario
		/////////////////////////////////////////////////////

		/// schedule the start of an act (the act will begin 30 seconds after)
		void scheduleStartAct(TSessionId sessionId, uint32 actId);
		/// Called by the client when he connect to play mode in an animation session
		virtual void connectAnimationModePlay(NLNET::IModuleProxy*);
		// Called by EGS to set the start position of a player
		virtual void getStartParams(NLNET::IModuleProxy *sender, uint32 charId, TSessionId lastStoredSessionId);
		/// launch the start of a new Act (is launch by AIS or by DM bia dmc)
		void startAct(TSessionId sessionId, uint32 actId);
		void scheduleStartSession(const CAnimationMessageAnimationStart &msg);
		void scheduleStartSessionImpl(const CAnimationMessageAnimationStart &msg);
		// Remove a char from a session, remove its module
		void disconnectChar(TCharId charId);
		virtual bool getConnectedChars(TSessionId sessionId, std::vector<TCharId>& chars) const;
		virtual void setSessionStartParams(TSessionId sessionId, sint32 x, sint32 y, uint8 season);
		virtual void teleportCharacter(NLNET::IModuleProxy *ais, const NLMISC::CEntityId& eid, float x, float y, float z);
		void broadcastMsg(TSessionId sessionId, const NLNET::CMessage& msg);

		/////////////////////////////////////////////////////
		//// Data Access
		/////////////////////////////////////////////////////

		IServerEditionModule* getEditionModule() const;
		CAnimationSession* getSession(TSessionId sessionId) const;
		TSessionId getSessionIdByCharId(TCharId charId) const;
		CAnimationSession* getSessionByCharId(TCharId charId) const;
		virtual NLNET::IModule* getModule() const;
		// Called by EditionModule to add a character
		virtual void addPioneer( TSessionId sessionId, TCharId charId) ;
		// from client (the module will upload mission item description to dm that will store it to its db)
		virtual void askMissionItemsDescription(NLNET::IModuleProxy *client);
		// from client (the module will upload Acts Position Description to client)
		virtual void askActPositionDescriptions(NLNET::IModuleProxy *client);
		// from client (the module will upload Acts Position Description to client)
		virtual void askUserTriggerDescriptions(NLNET::IModuleProxy *client);
		// from client (the module will upload Acts Position Description to client)
		virtual void askIncarnatingListUpdate(NLNET::IModuleProxy *client);
		virtual void askTalkingAsListUpdate(NLNET::IModuleProxy *client);
		virtual void askUpdateScenarioHeader(NLNET::IModuleProxy *clientProxyPtr);
		virtual bool getHeaderInfo(TSessionId sessionId, TScenarioHeaderSerializer::TValueType& values) const;


		/////////////////////////////////////////////////////
		//// DM
		/////////////////////////////////////////////////////
		virtual void activateEasterEgg(class NLNET::IModuleProxy *aisControl, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string & items, float x, float y, float z, float heading, const std::string& grpControler, const std::string& name, const std::string& look);
		virtual void deactivateEasterEgg(class NLNET::IModuleProxy *aisControl, uint32 easterEggId, TSessionId scenarioId, uint32 actId);
		virtual void deactivateEasterEggsFromAct(TSessionId scenarioId, uint32 actId);
		virtual void onEasterEggLooted(class NLNET::IModuleProxy *egs, uint32 easterEggId, TSessionId scenarioId);
		virtual void dssMessage(class NLNET::IModuleProxy *ais, TSessionId sessionId, const std::string & msgType, const std::string& who, const std::string& msg);
		// a dm target a npc and want to know what et can do on it (control, kill) and execute dm action (kill, heal...)
		virtual void onDssTarget( NLNET::IModuleProxy *senderModuleProxy, const std::vector<std::string> & params);
		// EGS gives infos to d
		virtual void onCharTargetReceived( NLNET::IModuleProxy *senderModuleProxy,
				const NLMISC::CEntityId& eid, const NLMISC::CEntityId&creatureId,
				TAIAlias alias, TDataSetRow entityRowId,
				const ucstring& ucName, uint32 nameId,
				const std::vector<std::string> & params,
				bool alived);
		// EGS message to indicates that a character is ready in mirror
		virtual void characterReady(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEid);
		virtual void setScenarioPoints(NLNET::IModuleProxy *ais, TSessionId sessionId, float scenarioPoints);
		virtual void startScenarioTiming(NLNET::IModuleProxy *ais, TSessionId sessionId);
		virtual void endScenarioTiming(NLNET::IModuleProxy *ais, TSessionId sessionId);
		bool getScore(TSessionId sessionId, uint32 &score, NLMISC::TTime &timeTaken);


		/////////////////////////////////////////////////////
		//// DM
		/////////////////////////////////////////////////////

		void triggerUserTrigger(TSessionId sessionId, uint32 actId, uint32 triggerId);
		virtual void onUserTriggerTriggered(NLNET::IModuleProxy *client, uint32 actId, uint32 triggerId);

		//////////////////////////////////////////////////////
		// NPC Control & talk methods
		///////////////////////////////////////////////////////
		/*
		* stopTalk sends a message to indicate a DM has stopped talking as an NPC.
		* eid is the DM Id.
		* creatureId is the targeted npc's id.
		* entityrowId is the mirror's id of the npc
		*/
		void stopTalk(const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, TDataSetRow entityRowId);

		/*
		* stopIncarn calls a AIWrapper method that sends a message to the AIS to indicate the DM has stopped
		* controlling a npc.
		* eid is the DM Id.
		* creatureId is the targeted npc's id.
		*/
		void stopIncarn(const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId);

		/*
		* stop the control of all Npc controlled by the player(incarn or talk as) clientProxyPtr
		* (use when client Go from DM -> Player)
		*/
		virtual void stopControlNpcs(TCharId charId);

		/*
		* When a bot is despawned, this method checks if it was controlled and "talked as" by a DM.
		* If controlled, the stopControl method is called and the _ControlledEntities map is updated: if no more players
		* control the npc the map entry corresponding to the creature is deleted. If there is still one player or more that
		* controls the npc, the map entry is maintained and its eid vector is updated.
		* It does the same treatment on the _TalkedAsEntities map.
		*/
		void onBotDespawnNotification(NLMISC::CEntityId& creatureId);
		void onBotDeathNotification(NLMISC::CEntityId& creatureId);
		void onStopNpcControlNotification(NLMISC::CEntityId& creatureId);


		/////////////////////////////////////////////////////
		//// Position
		/////////////////////////////////////////////////////


		virtual void askSetUserCharActPosition( NLNET::IModuleProxy *sender, uint32 charId );
		bool getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 actIndex = 0);

		virtual bool isSessionRunning(TSessionId sessionId) const;

		virtual uint32 getCurrentAct(TSessionId sessionId) const;
		virtual bool mustReloadPosition(TSessionId sessionId, TCharId charId) const;

		virtual bool stopTestImpl(TSessionId sessionId, uint32& lastAct);

		/////////////////////////////////////////////////////
		//// Debug/test functions
		/////////////////////////////////////////////////////

		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerAnimationModule, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, loadPrimitiveFile, "load Primitive file ", "filename")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, loadPdrFile, "load AiActions from Prd", "filename")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, loadRtFile, "", "filename")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, savePdrFile, "load AiActions from Pdr", "filename")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, displayPdr, "display pdr", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, clearPdr, "clear prd", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, startTest, "run test", "the sessionId")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, stopTest, "stop a test", "the sessionId")
//			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, stopTest, "stop a test", "the sessionId")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, displayMissionItems, "display Mission items of a session", "the sessionId")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, displayUserTriggers, "display UserTrigger of a session", "the sessionId")
			NLMISC_COMMAND_HANDLER_ADD(CServerAnimationModule, triggerUserTrigger, "trigger a user trigger", "the sessionId, the trigger act, the trigger Id");


		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(loadPrimitiveFile);
		NLMISC_CLASS_COMMAND_DECL(loadPdrFile);
		NLMISC_CLASS_COMMAND_DECL(savePdrFile);
		NLMISC_CLASS_COMMAND_DECL(displayPdr);
		NLMISC_CLASS_COMMAND_DECL(clearPdr);
		NLMISC_CLASS_COMMAND_DECL(loadRtFile);
		NLMISC_CLASS_COMMAND_DECL(startTest);
		NLMISC_CLASS_COMMAND_DECL(stopTest);
		NLMISC_CLASS_COMMAND_DECL(displayMissionItems);
		NLMISC_CLASS_COMMAND_DECL(displayUserTriggers);
		NLMISC_CLASS_COMMAND_DECL(triggerUserTrigger);

	private:
		struct COwnedCreatureInfo
		{
			std::vector<NLMISC::CEntityId> PlayerIds;
			TDataSetRow CreatureRowId;
			uint32 CreatatureAlias;
		};

		//private types
		typedef std::deque<CAnimationSession*> TSessionsQ;
		//map <scenarioId,CAnimationSession*>
		typedef std::map<TSessionId, CAnimationSession*> TSessions;
		//map <userId,scenarioId>
		typedef std::map<TCharId, TSessionId> TCharSessions;
		//map <userId,ClientAnimationModule>

		//key: controlled creature id; value: player id
		typedef std::map<NLMISC::CEntityId, COwnedCreatureInfo> TOwnedEntities;

	private: // private functions


		////////////////////////////
		// Talk & control methods
		////////////////////////////
		/*
		* If the creature is already controlled by a player, updates its eid vector by adding the id of the new controlling
		* player. If the creature is not already controlled by any player, this method adds an entry in the
		* _ControlledEntities map.
		*/
		bool setIncarningPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid, TDataSetRow entityRowId, TAIAlias alias);

		/*
		* If the creature is already "talked as" by a player, updates its eid vector by adding the id of the new talking
		* player. If the creature is not already "talked as"by any player, this method adds an entry in the
		* _TalkedAsEntities map.
		*/
		bool setTalkingAsPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid, TDataSetRow entityRowId, TAIAlias alias);

		/*
		* Checks whether the player is already talking as the targeted npc or not.
		*/
		bool isTalkingAs(const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid) const;

		/*
		* Checks whether a creature is already controlled by the player or not.
		*/
		bool isIncarnedByPlayer(const NLMISC::CEntityId& creatureId, const NLMISC::	CEntityId& eid) const;

		/*
		* Deletes an eid from the eid vector of a creature (in the _TalkedAsEntities map).If the vector is empty, the
		* whole map entry is deleted.
		*/
		void removeTalkingAsPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid);

		/*
		* Deletes an eid from the eid vector of a creature (in the _ControlledEntities map).If the vector is empty, the
		* whole map entry is deleted.
		*/
		void removeIncarningPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid);

		bool doMakeAnimationSession(CAnimationSession* animSession);
		void requestReleaseChannels(TSessionId sessionId);
		void requestLoadTable(CAnimationSession* session);
		void requestUnloadTable(TSessionId sessionId);
		TSessionId getScenarioId(TCharId charId);
		void broadcastMessage(CAnimationSession* session, const NLNET::CMessage& msg);
		NLMISC::CEntityId getEid(NLNET::TModuleProxyPtr senderModuleProxy);
		// message from  Server Admin Module for starting Animation Session (calle queueSession)

		//called by the DM to change the weather
		void setWeatherValue(TSessionId instanceId, uint16 weatherValue);
		//called by the DM to change  the season
		void setSeasonValue(TSessionId instanceId, uint8 seasonValue);



		/// Queue the creation of an animation session (+ output Debug)
		bool queueSession(CAnimationSession* session, bool runTest = true);
		/// for queueSession implementation
		bool makeAnimationSession(CAnimationSession* animSession, bool runTest);

		/// Create primitives that will be send to AIS from RTData
		bool translateActToPrimitive(CInstanceMap& components, CAnimationSession* animSession, CObject* act,
			uint32 actId, NLLIGO::CPrimitives& primDoc);
		// Create an Actuib  (for translateActToPrimitive implementation)
		virtual NLLIGO::IPrimitive* getAction(CObject* object, const std::string& prefix,TSessionId scenarioId);
		// Create an Event  (for translateActToPrimitive implementation)
		virtual NLLIGO::IPrimitive* getEvent(CObject* object,CInstanceMap& components, const std::string& prefix,TSessionId scenarioId);


		/// Queue the creation of an animation session (one creation is done by
		void startTest(TSessionId sessionId, CPersistentDataRecord& pdr);
		/// start a test mode (send message to AIS)
		bool startTest(CAnimationSession* session, bool runTest = true);
		/// stop a test (send message to AIS to unload Primitives)
		virtual bool stopTest(TSessionId sessionId, uint32& lastAct);


		/// stop the act
		void stopAct(TSessionId sessionId);


		// Update client the list of controlled by dm
		void updateTalkingAsList(TSessionId sessionId, uint32 charId);
		void updateIncarningList(TSessionId sessionId, uint32 charId);
		// Update client animation properties
		void updateAnimationProperties(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CEntityId & eid, CRtNpc * rtNpc, CRtGrp * rtGrp);
		CRtNpc* getNpcByAlias(TSessionId sessionId, TAIAlias alias) const;

	private: // privates members
		CDynamicMapService* _Server;

		// Proxy to string manager
		NLNET::TModuleProxyPtr	_StringManagerProxy;

		// Proxy to Character Controler
		NLNET::TModuleProxyPtr	_CharacterControlProxy;

		// Proxy to IOS ring interface
		NLNET::TModuleProxyPtr	_IOSRingProxy;

		//Map of SessionId, Animation Session
		TSessions		_Sessions;

		// Map of UserId, SessionId
		TCharSessions	_CharSessions;

		// liste of emote Map of emote name and animation name


		// Only one session can be performed by update
		TSessionsQ	_QueuedSessions;
		bool		_ReadyForNextSession;

		// Task that are waiting to be executed (like starting an act in 30 seconds...)
		CTaskList<NLMISC::TTime> _Tasks;

		CEmoteBehavior _Emotes;


		/*
		* Map containing:
		* -key: CEntityId corresponding to a controlled creature Id
		* -value: a COwnedCreatureInfo structure. This structure contains a TDataSetRow corresponding to the creature's id
		* in the mirror, and a vector of CEntityId (eids) corresponding to the players respectively controlling or "talking
		* as" the npc.
		*/
		TOwnedEntities _IncarnedEntities;

		TOwnedEntities _TalkedAsEntities;

		TOwnedEntities _TargetedEntities;



	};
}  // namespace R2

#endif	// R2_SERVER_ANIMATION_MODULE_H

