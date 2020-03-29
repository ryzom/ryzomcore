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

#ifndef RING_SESSION_MANAGER_H
#define RING_SESSION_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/misc/entity_id.h"
//#include "mysql_wrapper.h"
#include "nel/net/module_builder_parts.h"
#include "game_share/ring_session_manager_itf.h"

//using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace RSMGR
{
	class CRingSessionManager : 
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CRingSessionManagerSkel
//		public CRingSessionManagerWebItf,
//		public WS::CWelcomeServiceClientSkel

	{
		// mysql ring database connection
//		MSW::CConnection _RingDb;
		// mysql nel database connection
//		MSW::CConnection _NelDb;

	
		struct TSessionServerInfo
		{
			/// Shard id 
			uint32				ShardId;
			/// Total number of player in the sessions hosted by this server
			uint32				NbTotalPlayingChars;

			typedef std::map<uint32, TRunningSessionInfo>	THostedSessions;
			/// List of session running on this server
			THostedSessions		HostedSessions;

			TSessionServerInfo()
				: NbTotalPlayingChars(0)
			{
			}
		};

		typedef std::map<TModuleProxyPtr, TSessionServerInfo>	TSessionServers;
		/// known session servers
		TSessionServers			_SessionServers;

		typedef std::map<uint32, TModuleProxyPtr>		TSessionServersIdx;
		// index of sessionId to session server 
		TSessionServersIdx		_SessionIndex;

		typedef std::map<uint32, TModuleProxyPtr>	TWelcomeServices;
		/// List of known welcome service module for each shard
		TWelcomeServices		_WelcomeServices;

		typedef std::set<uint32>		TSessionSet;
		/// List of open session temporary lock until a session server claim to host them
		/// or the owner close the session.
		TSessionSet				_TemporaryLockedSession;



		struct TPendingSessionCreateInfo
		{
			/// the session id
			uint32		SessionId;
			/// the web connection that wait the response
			TSockId		From;
		};
		typedef std::list<TPendingSessionCreateInfo>	TPendingSessions;
		/// The list of session pending create result from DSS
		TPendingSessions	_PendingSessions;

		struct TPendingJoinSession
		{
			/// The user ID
			uint32		UserId;	
			/// the session id
			uint32		SessionId;
			/// The web connection
			TSockId		From;
			/// The character id
			uint32		CharId;
		};

		typedef std::list<TPendingJoinSession>	TPendingJoins;
		/// The list of character join session pending result from WS
		TPendingJoins	_PendingJoins;
	public:
		CRingSessionManager();
		~CRingSessionManager();

		bool initModule(const TParsedCommandLine &initInfo);
//		void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message);
		void onModuleUpdate();
		void onModuleDown(IModuleProxy *proxy);
		bool closeSession(TSessionId sessionId);

		static const std::string &getInitStringHelp();

		/////////////////////////////////////////////////////////////
		//// CRingSessionManager interface impl
		/////////////////////////////////////////////////////////////

		// A edition or animation server module register in the session manager
		virtual void registerDSS(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions);

		// The session server report a session creation.
		virtual void sessionCreated(NLNET::IModuleProxy *sender, const RSMGR::TRunningSessionInfo &sessionInfo);

		// The session report an event.
		// char id is used only when the event is about a character.
		virtual void reportSessionEvent(NLNET::IModuleProxy *sender, RSMGR::TSessionEvent event,
			TSessionId sessionId, uint32 charId);

		// The session report that a DM has kicked a character from a session.
		virtual void reportCharacterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId)
		{
		}

		/////////////////////////////////////////////////////////////
		//// Welcome service client messages
		/////////////////////////////////////////////////////////////
		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error
		virtual void invokeResult(NLNET::TSockId dest, uint32 resultCode, const std::string &resultString);

		// Return the result of the session joining attempt
		// If join is ok, the shardAddr contain <ip:port> of the
		// Front end that wait for the player to come in and the.
		// participation mode for the character (editor, animator or player).
		// If ok, the web must return a page with a lua script.
		// that trigger the action handler 'on_connect_to_shard' :
		// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
		// result : 0 : ok the client can join the session
		//          1 : char not found
		//          2 : session not found
		//          3 : no session participant for this character
		//          4 : can't find session server
		//          5 : shard hosting session is not reachable
		//          6 : nel user info not found
		//          7 : ring user not found
		//          8 : welcome service rejected connection request
		virtual void joinSessionResult(NLNET::TSockId dest, uint32 result, const std::string &shardAddr,
			const std::string &participantStatus);
/*
		// Register the welcome service in the ring session manager
		virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId);

		// return for welcome user
		virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok,
			const std::string &shardAddr);

		/////////////////////////////////////////////////////////////
		//// Web callback implementation
		/////////////////////////////////////////////////////////////

		/// Connection callback : a new interface client connect
		virtual void on_CRingSessionManagerWeb_Connection(NLNET::TSockId from)	{}

		virtual void on_CRingSessionManagerWeb_Disconnection(NLNET::TSockId from);

		virtual void on_scheduleSession(NLNET::TSockId from, 
			uint32 userId, 
			const std::string &sessionType, 
			const std::string &sessionTitle, 
			uint32 plannedDate, 
			const std::string &sessionDesc, 
			const std::string &sessionLevel, 
			const std::string &accessType, 
			const std::string &ruleType, 
			const std::string &estimatedDuration);

		// Cancel a planned session
		// Return 'invokeResult' : 0 : ok, session canceled
		//                         1 : unknown user
		//                         2 : unknown session
		//						   3 : user don't own the session
		//						   4 : session not in planned state
		virtual void on_cancelSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId);
*/
		// start a planned session
		// Return 'invokeResult' : 0 : ok, session started
		//                         1 : user not found
		//                         2 : session not found
		//                         3 : session not owned by user
		//                         4 : user is already in a session
		virtual void on_startSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId);

		// Close a running session
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : session not found
		//                         2 : user don't own the session
		virtual void on_closeSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId);

		// Add a character in a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already friend
		virtual void on_addFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId);

		// Remove a character from a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend
		virtual void on_removeFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId);

		// Add a character to a user ban list. This ban the user that own the character
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char already banned by user
/*
		virtual void on_addBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId);

		// Remove a character from a user ban list.
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char not banned by user
		virtual void on_removeBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId);

		// A user invite a character to help or play in his session
		// charRole is from enum TSessionPartStatus
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : char already invited
		//                4 : char role and session type don't match (edit/editor, anim/animator)
		//                5 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
		virtual void on_inviteCharacter(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 charId,
			const std::string &charRole);

		// A user remove an invitation in a session
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : character already entered in session
		//                4 : invitation not found
		virtual void on_removeInvitedCharacter(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 charId);
*/
		// A character ask to join a session.
		virtual void on_joinSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId);
/*
		// A user invite a guild to play in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                3 : session not found
		//                4 : guild already invited
		//                5 : user don't own the session
		virtual void on_inviteGuild(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 guildId);

		// Remove a guild invitation in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                3 : session not found
		//                4 : guild not invited
		//                5 : user don't own the session
		virtual void on_removeInvitedGuild(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 guildId);

		// Set the additional scenario info
		// playType is the enumerated type TPlayType
		// invokeReturn : 0 : ok, info setted
		//                1 : scenario not found
		//                2 : user not owner of session
		virtual void on_setScenarioInfo(NLNET::TSockId from, uint32 userId, TSessionId sessionId,
			const std::string &title, const std::string &journal, const std::string &credits, uint32 numPlayer,
			const std::string &playType);

		// Add an entry in the session journal
		// invokeReturn : 0 : ok, entry added
		//                1 : scenario not found
		//                2 : user can't post in this journal
		virtual void on_addJournalEntry(NLNET::TSockId from, uint32 userId, TSessionId sessionId,
			const std::string &entryType, const std::string &text);

		// Set the rating of a terminated (closed) session
		// invokeReturn : 0 : ok, rating added
		//                1 : scenario not found
		//                2 : char is not found
		//                3 : char is not a participant of session
		//                4 : session is not closed
		//                5 : char has already rated this session
		virtual void on_setPlayerRating(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 rating,
			const std::string &comments);

		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CRingSessionManager, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, dump, "dump the session manager internal state", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, forceSessionCleanup, "force a database synchronisation with current running session", "no param");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(forceSessionCleanup);

		NLMISC_CLASS_COMMAND_DECL(dump);
*/
	}; 
}	// namespace RSMGR

#endif
