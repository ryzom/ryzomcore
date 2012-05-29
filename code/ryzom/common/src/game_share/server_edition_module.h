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

#ifndef R2_SERVER_EDITION_MODULE_H
#define R2_SERVER_EDITION_MODULE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/twin_map.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sstring.h"


#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "game_share/ring_session_manager_itf.h"
#include "game_share/task_list.h"
#include "game_share/r2_share_itf.h"
#include "game_share/scenario.h"
#include "game_share/r2_types.h"
#include "r2_modules_itf.h"

#include "dms.h"

class CFarPosition;

namespace NLMISC
{
	struct CEntityId;
}

namespace NLNET
{
	class CMessage;
}

namespace R2
{
	class CObject;
	class CDynamicMapService;
	class CScenario;
	class CEditionSession;
	class CIdRecycle;
	class CKeysHolder;

	typedef uint32 TCharId;
	/**  Handle the edition session.
	Communicates with the client when he is editing a Scenario.
	Holds the references on clients that are allowed to connect themself.
	Holds the references on clients that are connected.
	Holds Editions Sessions.

	*/
	class CServerEditionModule : public IServerEditionModule,
		public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav <NLNET::CModuleBase> > >,
		public RSMGR::CRingSessionManagerClientSkel,
		public R2::CShareServerEditionItfSkel,
		public CServerEditionItfSkel
	{
	public:

		typedef uint32 TCharId;
		typedef uint32 TUserId;
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TAiInstanceId;

		struct THibernatingSession
		{
			THibernatingSession():HibernationDate(0){PositionX = 0; PositionY = 0; Orient = 0; Season=0;}

			THibernatingSession(uint32 hibernationDate, RSMGR::TSessionType sessionType, double x, double y, double orient, uint8 season )
				:HibernationDate(hibernationDate), SessionType(sessionType) { PositionX = x; PositionY = y; Orient = orient; Season = season; }

			~THibernatingSession()
			{

			}
		public:
			uint32 HibernationDate; //seconds since 1970
			RSMGR::TSessionType SessionType;
			CTaskList<NLMISC::TTime> Tasks;
			// In order that getStartPos works without
			double PositionX,PositionY, Orient;
			uint8 Season;
		};

		/// Infos on the connected pionner (useful for tp a player)
		class CPioneerInfo
		{
		public:
			CPioneerInfo();
			~CPioneerInfo();
			NLMISC::CEntityId EntityId;
			bool              WaitingTp; // Client is waiting the tp command + the season after he joined a session
			std::string		RingAccess;
			bool	EgsReloadPos;
			TCharMode Mode;
			bool Newcomer;
			NLNET::CMessage* WaitingMsg;
		};


		typedef std::map<TSessionId, THibernatingSession> THibernatingSessions;
		typedef std::map<TCharId, std::string> TOverrideRingAccess;
	public:

		CServerEditionModule();
		~CServerEditionModule();

		void init(NLNET::IModuleSocket* gateway, CDynamicMapService* server);

		/////////////////////////////////////////////////////
		//// CModuleBase API
		///////////////////////////////////////////////
		virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);
		virtual void onModuleUpdate();
		virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);
		virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);
		virtual void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);
		virtual bool isImmediateDispatchingSupported() const { return false; }
		virtual void onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId);
		virtual void onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId);


		/////////////////////////////////////////////////////
		//// RingSessionManagerClient callbacks
		/////////////////////////////////////////////////////
		// Ask the client to create a new session modules
		virtual void createSession(NLNET::IModuleProxy *sender, TCharId ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type);
		// Ask the client allow a character in the session
		virtual void addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId, const WS::TUserRole &connectedAs, const std::string &ringAccess, bool newcomer);
		// Ask the client to close a running session
		virtual void closeSession(NLNET::IModuleProxy *sender, TSessionId sessionId);
		// Ask the client stop the hibernation for the
		// specified character. This mean to remove any
		// hibernated scenario file from the backup.
		virtual void stopHibernation(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId ownerId);
		// Ask the client to hibernate a running session
		virtual void hibernateSession(NLNET::IModuleProxy *sender, TSessionId sessionId);
		// Specify the start param of a session
		virtual void setSessionStartParams(NLNET::IModuleProxy *sender, TCharId charId, TSessionId sessionId, const std::string& initialIslandLocation, const std::string& initialEntryPoint, const std::string& initialSeason);
		void setSessionStartParams(TSessionId, sint32 x, sint32 y, uint8 season);


		// Ask the client to close a running session
		// destroy the current scenario without closing the session
		virtual void resetSession(NLNET::IModuleProxy *sender, TSessionId sessionId, bool reconnect);
		// Session manager report that a character has been kicked by the web
		virtual void characterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId);
		virtual void characterUnkicked(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId);

		// Session manager report that a character must be teleport to another location
		virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId sourceCharId, TCharId destCharId);
		virtual void teleportWhileUploadingScenario(NLNET::IModuleProxy *sender, const std::string& island, const std::string& entryPoint, const std::string& season);

		/////////////////////////////////////////////////////
		//// Connection, Upload, Start (messages from Client)
		/////////////////////////////////////////////////////
		// a Client ask to start a scenario (the ask is broadcast to all connected users)
		virtual void startingScenario(NLNET::IModuleProxy *sender);
		// a Client is uploading a rtScenario in order to start the test session
		virtual void startScenario(NLNET::IModuleProxy *sender, bool ok, const TScenarioHeaderSerializer& header, const CObjectSerializerServer &data, uint32 startingAct);
		// Call by the client after a connection to a scenario in edition sesssion (is fallowed by a tp)
		virtual void advConnACK(NLNET::IModuleProxy *sender);
		// Call by the client in order to download its current scenario (and tp)
		virtual void onMapConnectionAsked(NLNET::IModuleProxy * clientEditionProxy, TSessionId scenarioId, bool  updateHighLevel = true, bool mustTp = false, TUserRole role = TUserRole::ur_editor);
		//  Call by the client in order to update the real time tree of the current session.
		virtual void rtScenarioUpdateRequested(NLNET::IModuleProxy *senderModuleProxy, TCharId charId, CObject* rtScenario);
		// Call by client that is developer to generate a primitive use for
		virtual void onMessageReceivedCreatePrimitives(NLNET::IModuleProxy *senderModuleProxy, TCharId charId);
		// Call by the client in order to stop the test and comme back to edition mode
		virtual void stopTestRequested(NLNET::IModuleProxy *senderModuleProxy, TCharId charId);
		// a client message to validate a file waiting to be saved
		virtual void saveScenarioFile(NLNET::IModuleProxy *sender, const std::string &md5, const TScenarioHeaderSerializer &header);
		// a client message to validate a file waiting to be loaded
		virtual void loadScenarioFile(NLNET::IModuleProxy *sender, const std::string &md5, const std::string &signature);
		// test if a session is hibernating if true return the "start position" of the session


		virtual void saveUserComponentFile(NLNET::IModuleProxy *sender, const std::string &md5, const TScenarioHeaderSerializer &header);
		virtual void loadUserComponentFile(NLNET::IModuleProxy *senderModuleProxy, const std::string &md5, const std::string &signature);

		/////////////////////////////////////////////////////
		//// Message forwarded by the SBS
		/////////////////////////////////////////////////////
		// then the header of the multipart message
		virtual void multiPartMsgHead(NLNET::IModuleProxy *sbs, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size);
		//send a part of a multi-pat message
		virtual void multiPartMsgBody(NLNET::IModuleProxy *sbs, uint32 charId, uint32 partId, const std::vector<uint8> &data);
		//send a footer of a mutlipart Message
		virtual void multiPartMsgFoot(NLNET::IModuleProxy *sbs, uint32 charId);
		// simulate the SBS. (Some message can be send to SBS that forward to DSS)
		virtual void forwardToDss(NLNET::IModuleProxy *senderModuleProxy, uint32 charId, const NLNET::CMessage& msg);

		/////////////////////////////////////////////////////
		//// Data tree modification (messages from Client)
		/////////////////////////////////////////////////////
		virtual void onScenarioUploadAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
			const CObjectSerializerServer& hlScenario, bool mustBroadCast);

		virtual void onNodeSetAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
			const std::string&  instanceId, const std::string & attrName, const CObjectSerializerServer& value2);

		virtual void onNodeInsertAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
			const std::string&  instanceId, const std::string & attrName, sint32 position,
			const std::string& key, const CObjectSerializerServer& value2);

		virtual void onNodeEraseAsked(NLNET::IModuleProxy *senderModuleProxy,  uint32 messageId,
			const std::string& instanceId, const std::string& attrName, sint32 position);

		virtual void onNodeMoveAsked(NLNET::IModuleProxy *senderModuleProxy,  uint32 messageId,
			const std::string& instanceId1, const std::string& attrName1, sint32 position1,
			const std::string& instanceId2, const std::string& attrName2, sint32 position2);


		/////////////////////////////////////////////////////
		//// Position
		/////////////////////////////////////////////////////
		// getEditing position (use AdminModule::getPosition for having a position in editing and animation mode)
		// Tp the user (senderModuleProxy) to the entry point of its scenario
		virtual void tpToEntryPoint(NLNET::IModuleProxy *senderModuleProxy, uint32 actId);
		// Set current Act( when go test start from this act and when reco restart in this act)
		virtual void setStartingAct(  NLNET::IModuleProxy *senderModuleProxy, uint32 actId);
		// The client senderModuleProxy ask to be tp at x,y (the season is correctly setted)
		virtual void onTpPositionAsked( NLNET::IModuleProxy *senderModuleProxy, float x, float y, float z);
		// Ask the teleporatation of entitiy clientEid
		virtual void tpPosition(  NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CEntityId & clientEid, float x, float y, float z, uint8 season, const R2::TR2TpInfos& tpInfos);
		void getTpContext(TCharId charId, std::string& tpCancelTextId, R2::TTeleportContext& tpContext);

		void returnToPreviousSession(TCharId charId);


		/////////////////////////////////////////////////////
		//// Handle key policies
		/////////////////////////////////////////////////////
		// eg of conf
		// setDefaultKey(0, "PROD");
		//	addKeyPolicy(0, "DEFAULT", "EMPTY", "REFUSE");
		//	addKeyPolicy(0, "EMPTY", "EMPTY", "REFUSE"); // Deprecated (scenario older than 22 juin 2006)
		//	addKeyPolicy(0, "PROD", "PROD:098085b10aa27157d57587464e3b2390 ", "TEST");
		//	addKeyPolicy(0, "DEV", "DEV:26289fe9502f4db1fa3229e4fb2d8803", "TEST"); // accept current dev key
		//	addKeyPolicy(0, "DEVOBSOLETE", "DEVOBSOLETE:26289fe9502f4db1fa3229e4fb2d8803", "REFUSE");  // refuse old dev key
		// reset all policies
		virtual void resetKeyPolicies(NLNET::IModuleProxy *sender);
		// set the default key to use e.g. "DEV01" for dev "PROD01" for prod
		virtual void setDefaultKey(NLNET::IModuleProxy *sender, const std::string & defaultKeyName);
		// Add a key policy
		//
		//Key name is the name of the key, the special values can be "DEFAULT" (rules for all key that are not listed, "EMPTY" rules for scenario that are previous the key system,
		//The Private key value must be "NAMEOFTHEKEY:Md5OfARandomGeneratedFile"
		//policy must be
		virtual void addKeyPolicy(NLNET::IModuleProxy *sender, const std::string & keyName, const std::string & privateKeyValue, const std::string& policy);
		// The client update the ring points needed by the current scenario
		// ringAcess the ring points of the current scenario
		// ok true if the Pioneer has enought points false otherwise
		virtual void onScenarioRingAccessUpdated(NLNET::IModuleProxy *client, bool ok,const std::string & ringAccess, const std::string& errMsg);


		/////////////////////////////////////////////////////
		//// Management of userComponent (Not finished)
		/////////////////////////////////////////////////////
		// The client ask add a new UserComponent (if not present ask the user to upload)
		virtual void onUserComponentRegistered(NLNET::IModuleProxy *sender, const NLMISC::CHashKeyMD5& md5 );
		// The client was asked to upload an user component (that was not present on the server during the call of onUserComponentRegister)
		virtual void  onUserComponentUploaded(NLNET::IModuleProxy *senderModuleProxy, CUserComponent* component);
		// The client was asked to upload an user component (that was not present on the server during the call of onUserComponentRegister)
		virtual void  onUserComponentDownloading(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CHashKeyMD5& md5);
		// Gets an User Component by its md5
		CUserComponent* getUserComponent( const NLMISC::CHashKeyMD5& md5) const;
		virtual void onCharModeUpdateAsked(NLNET::IModuleProxy *sender, TCharMode mode);

		/////////////////////////////////////////////////////
		//// IServerEditionModule virtuals (ahem, some of them)
		/////////////////////////////////////////////////////


		virtual void updateCharPioneerRight(TCharId charId);
		virtual void characterReady(TCharId charId);

		/////////////////////////////////////////////////////
		//// Task && Callback implementation
		/////////////////////////////////////////////////////
		CEditionSession* getSession(TSessionId sessionId) const;
		TSessionId getSessionIdByCharId(TCharId charId) const;
		// implement private task CTaskConnectPlayer
		void connectChar(TSessionId sessionId, TCharId charId, TUserRole userRole, const std::string& ringAccess, bool newcomer);
		// implement private task CTaskTryConnectPlayer
		void  tryCharConnection(TCharId charId);
		// Remove a character from a session (exectuted by kick player if player was not going back to mainland)
		// implement private task CKickPlayerIfStillConnected
		bool removeCharacterFromSessionImpl(TSessionId sessionId, TCharId charId, std::string& outMsg);
		// verify Ring access of a char or reset
		void verifyRingAccess(TSessionId sessionId, TCharId charId);
		//implement private Callback COverrideRingAccessCallback
		void swapOverrideRingAccess(TOverrideRingAccess& access);
		// save a session -> session go to Backup service)
		bool saveSessionImpl(TSessionId sessionId, std::string& msg);

		// wakeUp a session
		void wakeUpSessionImpl(CEditionSession* session);

		void getStartParams(uint32 charId, TSessionId lastStoredSessionId);
		void getStartParamsImpl(uint32 charId, TSessionId lastStoredSessionId);
		// wakup a session (load session from BS), start animation, connect waiting clients
		bool wakeUpSession(TSessionId sessionId, TCharId ownerId, std::string& msg);
		// The file has been loadded from file (or no file exist)
		void setLoaded(TSessionId sessionId);
		// update the vision of every person in the current session
		void updateScenarioVision(TSessionId sessionId);


		/////////////////////////////////////////////////////
		//// Debug / Command functions
		/////////////////////////////////////////////////////

		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerEditionModule, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, listSessions, "display the list of session", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, hibernateSession, "Hibernate a session", "<sesionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, wakeUpSession, "Wake up an hibernating session", "<sesionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, displaySession, "display a session", "<sessionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, dumpSession, "dump infos on a session", "<sesionId>")

			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, closeSession, "close a session", "<sessionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, resetSession, "reset a session", "<sessionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, reconnectSession, "recconnect char to a session", "<sessionId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, listPioneers, "display the list of pionnieers", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, kickPioneer, "Kick a player out of a session", "<sesionId> <charId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, unkickPioneer, "Unkick a player from a session", "<sesionId> <charId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, teleportOneCharacterToAnother, "Teleport a player to another player in the same session", "<sesionId> <charId> <targetCharId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, displayPioneer, "display a pionnieer", "<charId>")


			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, addCharacterInSession, "simulates the msg addCharacterInSessuion", "<sessionId> <charId> ")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, createSession, "simulates the msg CreateSession", "<ownerCharId> <sessionId> <charId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, listAllowedPioneers, "list allowed Pioneers","no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, listConnectedPioneers, "list connected Pioneers", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, setCharMode, "set the mode of a char mode are 'editor' 'tester' 'player' 'dm' )", "<charId> <mode>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, displayCharRingAccess, "display the RingAccess of a character", "<charId>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, setCharRingAccess, "change the ring access of a character", "<charId> <mode>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, setSessionStartParams, "simulate a SU message", "<userId> <sessionId> <initialIslandLocation> <initialEntryPoint> <initialSeason>")



			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, setCharOverrideRingAccess, "Set the overrigde of ring access (for dev only)", "<charId> <mode>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, displayCharOverrideRingAccess, "Set the overrigde of ring access (for dev only)", "<charId> <mode>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, listCharOverrideRingAccess, "list the override of ring access", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, removeCharOverrideRingAccess, "remove the override of ring access", "no args")

			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, resetKeyPolicies, "reset all key policies", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, setDefaultKey, "set the default key use eg 'DEV'", "<theKey>")
			NLMISC_COMMAND_HANDLER_ADD(CServerEditionModule, addKeyPolicy, "add a key Policy(eg 'DEV DEV:01234567890123 Accept' or 'DEFAULT DEFAULT Refuse' or 'EMPTY EMPTY Refuse'  )", "<keyName> <keyValue> <policy>")

		NLMISC_COMMAND_HANDLER_TABLE_END

		// Debugs functions for displaying infos on sessions
		NLMISC_CLASS_COMMAND_DECL(listSessions);
		NLMISC_CLASS_COMMAND_DECL(displaySession);
		NLMISC_CLASS_COMMAND_DECL(hibernateSession);
		NLMISC_CLASS_COMMAND_DECL(wakeUpSession);
		NLMISC_CLASS_COMMAND_DECL(closeSession);
		NLMISC_CLASS_COMMAND_DECL(resetSession);
		NLMISC_CLASS_COMMAND_DECL(reconnectSession);
		NLMISC_CLASS_COMMAND_DECL(setSessionStartParams);
		NLMISC_CLASS_COMMAND_DECL(dumpSession);


	// Debugs functions for displaying infos on Dev that have overridden access
		NLMISC_CLASS_COMMAND_DECL(listCharOverrideRingAccess);
		NLMISC_CLASS_COMMAND_DECL(displayCharOverrideRingAccess);
		NLMISC_CLASS_COMMAND_DECL(setCharOverrideRingAccess);
		NLMISC_CLASS_COMMAND_DECL(removeCharOverrideRingAccess);


		// Debugs functions for displaying infos on connected Pioneer
		NLMISC_CLASS_COMMAND_DECL(listPioneers);
		NLMISC_CLASS_COMMAND_DECL(displayPioneer);
		NLMISC_CLASS_COMMAND_DECL(kickPioneer);
		NLMISC_CLASS_COMMAND_DECL(unkickPioneer);
		NLMISC_CLASS_COMMAND_DECL(teleportOneCharacterToAnother);

		// Debugs functions for displaying infos Pioneers that are allowed to connect
		NLMISC_CLASS_COMMAND_DECL(listAllowedPioneers);
		NLMISC_CLASS_COMMAND_DECL(listConnectedPioneers);

		// Emulate SU messages
		NLMISC_CLASS_COMMAND_DECL(addCharacterInSession);
		NLMISC_CLASS_COMMAND_DECL(createSession);

		// Debug function to test the mode of a player
		NLMISC_CLASS_COMMAND_DECL(setCharMode);
		NLMISC_CLASS_COMMAND_DECL(displayCharRingAccess);
		NLMISC_CLASS_COMMAND_DECL(setCharRingAccess);

		NLMISC_CLASS_COMMAND_DECL(resetKeyPolicies);
		NLMISC_CLASS_COMMAND_DECL(setDefaultKey);
		NLMISC_CLASS_COMMAND_DECL(addKeyPolicy);



	// privates methodes
	private:
		// Check if a scenario is ok (not to much element)
		bool checkScenario(CObject* scenario);

		// Backup current Secenarios (obsolete)
		void saveToDb();

		// register a session
		void registerSession(TSessionId sessionId,  CEditionSession* session);
		// return a 0 if not linked
		TSessionId  getLinkedSessionId(TSessionId sessionId) const;

		/////////////////////////////////////////////////////
		//// Communication
		/////////////////////////////////////////////////////
		// broadcast a message to all user connected to the scenario
		void broadcastToCharConnected(const NLNET::CMessage & msg, CEditionSession *session);
		// fill a vector of module proxy for broadcasting message related to a scenario
		void fillBroadcastVector(CEditionSession *session, std::vector<NLNET::TModuleProxyPtr> &broadcastList);
		// send as fillBroadcastVector but remove the sender from the list
		void fillBroadcastVectorNoSender(CEditionSession *session, std::vector<NLNET::TModuleProxyPtr> &broadcastList, NLNET::IModuleProxy *senderModuleProxy);
		// send message to all users connected to the sender scenario
		void replyToAll(NLNET::IModuleProxy *sender, const NLNET::CMessage & msg);

		// indicate to the BS to save scenarios;
		void release();
		void updateSessionListFile();
		std::string getSessionFilename(TSessionId sessionId, TCharId charId) const;
		std::string getOverrideRingAccessFilename() const;

		void updateRSMGR();
		bool isInitialized() const;

			// disconnect a player by its char Id
		virtual void disconnectChar(TCharId charId);


		bool isClientAuthorized(TCharId charId) const;


		virtual bool isSessionHibernating(TSessionId sessionId, RSMGR::TSessionType& sessionType, double& x, double& y, double& orient, uint8& season);

		// stop the session
		void stopTest(TSessionId sessionId);
		// Simulate autoConnection without SU
		virtual void createSessionWithoutSu(TCharId charId, NLMISC::CEntityId clientEid);
		// Simulate Set Start AiInstance by SU
		virtual void simulateSU(NLMISC::CEntityId clientEid);

		bool getIsCharRoS(TCharId charId) const;


		/////////////////////////////////////////////////////
		//// Data Access
		/////////////////////////////////////////////////////

		// get a scenario by the sessionId
		CScenario* getScenarioById(TSessionId sessionId) const;
		// get the current session (available from the moment when the client module is connected)
		TUserRole getRoleByCharId(TCharId charId) const;

		// get the session that is about to be joined by a client, or NULL if not found
		TPioneersSessionsAllowed * getSessionAllowedForChar(TCharId charId) const;
		// get the scenario use by the user

		CScenario*  getScenarioByCharId(TCharId charId) const;
		CEditionSession* getSessionByCharId(TCharId charId) const;

		IServerAnimationModule* getAnimationModule() const;
		const NLNET::TModuleProxyPtr * getClientProxyPtr(TCharId charId) const;


		bool getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 locationIndex = 0);
		bool isEditingSession(TSessionId sessionId) const;
		// return the linked session (eg an animation session linked to an edition session ) or NULL
		TSessionId getLinkedSession(CEditionSession* session) const;

		// Remove a module from an vector of Module (use the remove the sender from all vectors)
		std::vector<NLNET::TModuleProxyPtr> removeSender(const std::vector<NLNET::TModuleProxyPtr>& allModule, NLNET::IModuleProxy *senderModuleProxy );


		/////////////////////////////////////////////////////
		//// Helper function (use to implement NLMISC_COMMAND, module message, or task)
		/////////////////////////////////////////////////////
		// implementation of kick ( send a warning message to client to disconnect himself otherwise he will be kicked 30 seconds later)
		bool kickPioneerImpl(TSessionId sessionId, TCharId charId,  std::string& outMsg);
		bool unkickPioneerImpl(TSessionId sessionId, TCharId charId,  std::string& outMsg);
		// implementaion of teleport (teleport a character to another character if they are in the same session (current whether is correctly set)
		bool teleportOneCharacterToAnotherImpl(TSessionId sessionId, TCharId source, TCharId target, std::string& outMsg);
		// hibernate a session (save to files)
		bool hibernateSessionImpl(TSessionId sessionId, std::string& msg);
		// close a session
		bool closeSessionImpl(TSessionId sessionId, std::string& msg);

		CServerEditionModule::CPioneerInfo* getPioneerInfo(TCharId charId);


	private: // private types


		/// Map of charId, SessionId (Session in witch the user is hold).
		typedef std::map<TCharId, TSessionId> TPioneersSessions;

		/// Map of UserId,TSession Allowed (user that are allowed to connect, and their role)
		typedef std::map<TCharId, TPioneersSessionsAllowed>   TPioneersSessionsAlloweds;
		typedef std::map<TCharId, NLMISC::TTime>   TKicked;



		class CUserComponentRefCounter: public NLMISC::CRefCount
		{
		public:
			void registerSession(TSessionId sessionId){_Sessions.insert(sessionId);}

			void unregisterSession(TSessionId sessionId){_Sessions.erase(sessionId);}

			void updateComponent(CUserComponent* component)
			{
				_Component.reset( component );
			}

			CUserComponent*get() const
			{
				return _Component.get();
			}

		private:
			std::set<TSessionId> _Sessions;

			std::auto_ptr<CUserComponent> _Component;
		};

		struct CHashKeyMD5Less : public std::binary_function<NLMISC::CHashKeyMD5, NLMISC::CHashKeyMD5, bool>
		{
			bool operator()(const NLMISC::CHashKeyMD5& x, const NLMISC::CHashKeyMD5& y) const { return x.operator<(y); }
		};


		//typedef std::map<NLMISC::CHashKeyMD5, NLMISC::CRefPtr<CUserComponentRefCounter>, CHashKeyMD5Less>  TUserComponentRefCounter;
		typedef std::map<NLMISC::CHashKeyMD5, NLMISC::CRefPtr<CUserComponentRefCounter> >  TUserComponentRefCounter;

		typedef std::map<TCharId, CPioneerInfo> TPionnerInfos;

		typedef std::map<TSessionId, CEditionSession*> TSessions;


		struct TOwnerInfo
		{
		public:
			TSessionId EditionSessionId;
			TSessionId AnimationSessionId;
			TAiInstanceId AiInstanceId;

			TOwnerInfo(TSessionId editionSessionId, TSessionId animationSessionId, TAiInstanceId aiInstanceId)
				:EditionSessionId(editionSessionId),AnimationSessionId(animationSessionId), AiInstanceId(aiInstanceId){}
		};

		typedef std::map<TCharId, TOwnerInfo> TOwnerInfos;
		typedef std::map<TSessionId, TSessionId> TRemappedSessionIds; //Remapped SessionId (eg an animation session started from an edition session)



	private: // private members


		// Name of the version (read from a file and send to the player)
		NLMISC::CSString _VersionName;

		//Map of sessionId CEditionSession( a scenario contains, edition tree, connected users)
		TSessions _Sessions;


		/// The service
		CDynamicMapService*		_Server;

		/// Map of UserId, SessionId (Session in witch the user is hold).
		TPioneersSessions _PioneersSessions;

	    /// Map of UserId,TSession Allowed (user that are allowed to connect, and their role)
		TPioneersSessionsAlloweds _PioneersSessionsAllowed; // charId/sessionId of pioneers allowed to connect

		/// Module proxy to SessionManager (SU)
		NLNET::TModuleProxyPtr	_SessionManager;

		/// Module proxy to CServerAnimationModule (DSS)
		NLNET::TModuleProxyPtr	_ServerAnimationProxy;

		/// Module proxy to CServerAdminModule (DSS)
		NLNET::TModuleProxyPtr	_ServerAdminProxy;

		// Proxy to Character Controler
		NLNET::TModuleProxyPtr	_CharacterControlProxy;

		// Proxy to The module that handle save on the BS
		NLNET::TModuleProxyPtr	_R2SessionBackupModule;

		// Proxy to IOS ring interface
		NLNET::TModuleProxyPtr	_IOSRingProxy;


		/// Twin map TUserId <=> Proxy to client edition module
		NLMISC::CTwinMap<TCharId, NLNET::TModuleProxyPtr> _ClientsEditionModule;

		/// Infos on the connected pionner (useful for tp a player)
		TPionnerInfos		_PioneersInfo;

		TUserComponentRefCounter _UserComponentRefCounter;



		TOwnerInfos	_OwnerInfos;
		TRemappedSessionIds _RemappedSessionIds;


		THibernatingSessions _HibernatingSessions;
		typedef std::set< std::pair<TSessionId, TCharId> > TClosedSessions;
		TClosedSessions _ClosedSessions;
		bool _WaitingForBS;
		bool _MustUpdateHibernatingFileList;

		std::deque<CTask<NLMISC::TTime>*> _ConnectionsTask;
		CTaskList<NLMISC::TTime> _Tasks; //Execute only when bs is ok on if not client
		CTaskList<NLMISC::TTime> _RingAccessTasks; // Remove people that do not belong here
		TOverrideRingAccess _OverrideRingAccess; //Ring access for dev
		bool _MustUpdateOverrideRingAcess;
		std::auto_ptr<CIdRecycle> _IdRecycle;

		TKicked _Kicked;
		//
		bool _BsGoingUp;
		CKeysHolder* _KeysHolder;
	};

} // namespace R2

#endif	// R2_SERVER_EDITION_MODULE_H
