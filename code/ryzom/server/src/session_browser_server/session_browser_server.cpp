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

#include <set>

#include "session_browser_server.h"
#include "nel/misc/command.h"


#include "game_share/singleton_registry.h"
#include "game_share/ring_session_manager_itf.h"
#include "game_share/character_sync_itf.h"
#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"
#include "game_share/shard_names.h"
#include "game_share/r2_share_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;
using namespace MSW;
using namespace CHARSYNC;
using namespace R2;


CVariable<string>	MaxRingPoints("sbs", "MaxRingPoints", "A string that contains the maximum ring points in each ecosystem (e.g A3:D8:F5:J9:L2:R4)", "A1:D9:F9:J9:L9:R9", 0, true);

#ifndef UNIT_TEST_ENV
// force admin module and command executor to link in
extern void admin_modules_forceLink();

void foo()
{
	admin_modules_forceLink();
}
#else
void session_browser_force_link()
{
}
#endif

#define	CHECK_AUTH_WITH_USERID(from, userId)	\
{ \
	const uint32 *puserId = _ClientAuths.getB(from); \
	if (puserId == NULL || *puserId != userId) \
	{ \
		nlwarning("Unauthorised access from %s, with user %u, disconnect", from->getTcpSock()->remoteAddr().asString().c_str(), userId); \
		CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); \
		return; \
	} \
} \

#define	CHECK_AUTH_WITH_CHARID(from, charId)	\
{ \
	uint32 userId = charId >> 4; \
	CHECK_AUTH_WITH_USERID(from, userId); \
} \


// The session browser module
class CSessionBrowserServerMod 
	:	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CSessionBrowserServerWebItf,
		public CRingSessionManagerWebClientItf
{

	typedef uint32	TUserId;

	// Ring database connection
	MSW::CConnection		_RingDB;

	/// Address of the SU server
	CInetAddress	_ServerAddress;

	// date of last SU connection attempt
	time_t		_LastConnAttempt;

	/// flag stating the connection state with SU
	bool		_ServerConnected;

//	typedef map<TSockId, CLoginCookie>	TClientAuths;
	typedef CTwinMap<TSockId, TUserId>	TClientAuths;
	/// client auth info
	TClientAuths	_ClientAuths;
	
	// Proxy to ServerEditionModule
	NLNET::TModuleProxyPtr	_ServerEditionProxy;

public:
	CSessionBrowserServerMod()
		:	_LastConnAttempt(0),
			_ServerConnected(false)
	{
	}

	bool initModule(const TParsedCommandLine &initInfo)
	{
		// recall base class
		bool ret = CModuleBase::initModule(initInfo);

		const TParsedCommandLine *sa = initInfo.getParam("suAddr");
		BOMB_IF(sa == NULL, "Failed to find param 'suAddr' in init param", return false);

		_ServerAddress = CInetAddress(sa->ParamValue);
		nldebug("Initializing module, SU at %s", _ServerAddress.asString().c_str());
		BOMB_IF(!_ServerAddress.isValid(), "Invalid server address in '"<<sa->ParamValue<<"'", return false);

		const TParsedCommandLine *lp = initInfo.getParam("listenPort");
		BOMB_IF(lp == NULL, "Failed to find param 'listenPort' in init param", return false);

		uint16 port;
		NLMISC::fromString(lp->ParamValue, port);
		BOMB_IF(port == 0, "Invalid listen port '"<<lp->ParamValue<<"'", return false);

		// open the server
		openItf(port);

		// init ring db
		const TParsedCommandLine *initRingDb = initInfo.getParam("ring_db");
		if (initRingDb  == NULL)
		{
			nlwarning("RSM : missing ring db connection information");
			return false;
		}
		
		// connect to the database
		if (!_RingDB.connect(*initRingDb))
		{
			nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
			return false;
		}
		
		CShardNames::getInstance().init(IService::getInstance()->ConfigFile);

		return ret;
	}

	void onModuleUpdate()
	{
		time_t now = CTime::getSecondsSince1970();
		// check connection with the SU server
		if (!CRingSessionManagerWebClientItf::_CallbackClient->connected() && now - _LastConnAttempt > 5)
		{
			// time to try a connection
			_LastConnAttempt = now;

			try
			{
				nldebug("Connecting to SU at %s...", _ServerAddress.asString().c_str());
				CRingSessionManagerWebClientItf::connectItf(_ServerAddress);
			}
			catch(...)
			{
				nldebug("Connection failed");
				// failed to connect :(
			}
			_ServerConnected = CRingSessionManagerWebClientItf::_CallbackClient->connected();
		}
		else
		{
			// just update the interfaces
			CRingSessionManagerWebClientItf::update();
			CSessionBrowserServerWebItf::update();

		}
	}
	 
	void onModuleUp(NLNET::IModuleProxy *moduleProxy)
	{
		std::string moduleName = moduleProxy->getModuleClassName();
		if (moduleName == "ServerEditionModule")
		{
			_ServerEditionProxy = moduleProxy;
		}
	}

	void onModuleDown(NLNET::IModuleProxy *moduleProxy)
	{
		std::string moduleName = moduleProxy->getModuleClassName();
		if (moduleName == "ServerEditionModule")
		{
			_ServerEditionProxy = NULL;
		}
	}


	///////////////////////////////////////////////////////////////////////
	// CRingSessionManagerWebItf implementation
	///////////////////////////////////////////////////////////////////////

	/// Connection callback : a new interface client connect
	virtual void on_CRingSessionManagerWeb_Connection(NLNET::TSockId from)
	{
		if (!_ServerConnected)
		{
			// the server is out, disconnect the client
			CSessionBrowserServerWebItf::_CallbackServer->disconnect(from);
		}
	}
	/// Disconnection callback : one of the interface client disconnect
	virtual void on_CRingSessionManagerWeb_Disconnection(NLNET::TSockId from)
	{
		// TODO : remove any pending request for this client

		// erase the free trial info (if any)
		const TUserId *puid = _ClientAuths.getB(from);
		if (puid != NULL)
		{
			// erase auth info
			_ClientAuths.removeWithA(from);
		}
	}
	virtual void on_setSessionStartParams(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string& initialIslandLocation, const std::string & initialEntryPoint, const std::string& initialSeason)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::setSessionStartParams(charId, sessionId, initialIslandLocation, initialEntryPoint, initialSeason);			
	}


	// Create or schedule a new session (edit or anim)
	virtual void on_scheduleSession(NLNET::TSockId from, uint32 charId, const TSessionType &sessionType, const std::string &sessionTitle, const std::string &sessionDesc, const TSessionLevel &sessionLevel, /*const TAccessType &accessType, */const TRuleType &ruleType, const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TAnimMode &animMode, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, const std::string &language, const TSessionOrientation &orientation, bool subscriptionClosed, bool autoInvite)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::scheduleSession(charId, sessionType, sessionTitle, sessionDesc, sessionLevel, /*accessType, */ruleType, estimatedDuration, subscriptionSlot, animMode, raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, language, orientation, subscriptionClosed, autoInvite);
	}

	// get session info
	virtual void on_getSessionInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::getSessionInfo(charId, sessionId);
	}


	// Update the information of a planned or running session
	// Return 'invokeResult' : 0 : ok, session updated
	//                         1 : unknown character
	//                         2 : unknown session
	//                         3 : char don't own the session
	//                         4 : session is closed, no update allowed
	virtual void on_updateSessionInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &sessionTitle, uint32 plannedDate, const std::string &sessionDesc, const R2::TSessionLevel &sessionLevel, /*const TAccessType &accessType, */const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::updateSessionInfo(charId, sessionId, sessionTitle, plannedDate, sessionDesc, sessionLevel, /*accessType, */estimatedDuration, subscriptionSlot, raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, subscriptionClosed, autoInvite, language, orientation);
	}

	// Cancel a plannified session
	// Return 'invokeResult' : 0 : ok, session canceled
	//                         1 : unknown char
	//                         2 : unknown session
	//                         3 : char don't own the session
	//                         4 : session not in planned state
	virtual void on_cancelSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::cancelSession(charId, sessionId);
	}

	// start a planned session
	// Return 'invokeResult' : 0 : ok, session started
	//                         1 : char not found
	//                         2 : session not found
	//                         3 : session not own by user
	//                         4 : user is already have a running session of this type
	//                         5 : session server failure
	virtual void on_startSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::startSession(charId, sessionId);
	}

	// Close a running session
	// Return 'invokeResult' : 0 : ok, session closed (or about to close)
	//                         1 : session not found
	//                         2 : char don't own the session
	//                         3 : session not open
	//                         4 : failed to close the session, internal error
	virtual void on_closeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::closeSession(charId, sessionId);
	}

	// Close the current edit session of the character
	// Return 'invokeResult' : 0 : ok, session closed (or about to close)
	//                         1 : char not found
	//                         2 : failed to close the session, internal error
	virtual void on_closeEditSession(NLNET::TSockId from, uint32 charId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::closeEditSession(charId);
	}

	// Hibernat an edit session (if open)
	// Return 'invokeResult' : 0 : ok
	//                         1 : not ok
	virtual void on_hibernateEditSession(NLNET::TSockId from, uint32 charId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::hibernateEditSession(charId);
	}

	// Add a character in a user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char already friend
	virtual void on_addFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::addFriendCharacter(userId, friendCharId);
	}

	// Repove a character from a user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char not friend
	virtual void on_removeFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::removeFriendCharacter(userId, friendCharId);
	}

	// Add a character to a user ban list. This ban the user that own the character
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : banned char not found
	//                         3 : char already banned by user
	virtual void on_addBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::addBannedCharacter(userId, bannedCharId);
	}

	// Remove a character from a user ban list.
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : banned char not found
	//                         3 : char not banned by user
	virtual void on_removeBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::removeBannedCharacter(userId, bannedCharId);
	}

	// Add a character in a DM user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char already DM friend
	virtual void on_addFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::addFriendDMCharacter(userId, friendDMCharId);
	}

	// Remove a character from a DM user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char not friend
	virtual void on_removeFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::removeFriendDMCharacter(userId, friendDMCharId);
	}

	// Set the comment associated to a known character entry
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : known character entry not found
	//                         3 : character relation don't match the set comments relation
	//                         4 : internal error
	virtual void on_setKnownCharacterComments(NLNET::TSockId from, uint32 userId, uint32 charId, const std::string &relation, const std::string &comments)
	{
		CHECK_AUTH_WITH_USERID(from, userId);
		CRingSessionManagerWebClientItf::setKnownCharacterComments(userId, charId, relation, comments);
	}

	// A user invite a character to help or play in his session
	// charRole is from enum TSessionPartStatus
	// invokeReturn : 0 : ok, character invited
	//                1 : char not found
	//                2 : session not found
	//                3 : invited char not found
	//                4 : char not own the session
	//                5 : char already invited
	//                6 : char role and session type don't match (edit/editor, anim/animator)
	//                7 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
	//                8 : database failure
	//               11 : owner char is not animator in the session
	virtual void on_inviteCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 invitedCharId, const TSessionPartStatus &charRole)
	{
		CHECK_AUTH_WITH_CHARID(from, ownerCharId);
		CRingSessionManagerWebClientItf::inviteCharacter(ownerCharId, sessionId, invitedCharId, charRole);
	}

	// A user remove an invitation in a session
	// invokeReturn : 0 : ok, character invited
	//                1 : removed char not found
	//                2 : session not found
	//                3 : character already entered in session
	//                4 : invitation not found
	//                5 : owner char don't own the session
	virtual void on_removeInvitedCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 removedCharId)
	{
		CHECK_AUTH_WITH_CHARID(from, ownerCharId);
		CRingSessionManagerWebClientItf::removeInvitedCharacter(ownerCharId, sessionId, removedCharId);
	}

	// A character subscribe to a public animation session
	// invokeReturn : 0 : ok, subscription accepted
	//                1 : char not found
	//                2 : session not found
	//                3 : character already subscribed to or invited in the session
	//                4 : session not public
	//                5 : character banned
	//                6 : no place left, session is full
	//                7 : session owner not found
	//                8 : internal error
	virtual void on_subscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::subscribeSession(charId, sessionId);
	}

	// A character unsubscribe to a public animation session
	// The character must not join the session in order to unsubscribe
	// invokeReturn : 0 : ok, unsubscription accepted
	//                1 : char not found
	//                2 : session not found
	//                3 : character has not subscribed in the session
	virtual void on_unsubscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::unsubscribeSession(charId, sessionId);
	}

	// A character asks to join (or enter) a running session.
	// It must have been subscribed or invited to the session to be allowed
	virtual void on_joinSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &clientApplication)
	{
		nldebug("joinSession for char %u", charId);
		CHECK_AUTH_WITH_CHARID(from, charId);
		nldebug("  forwarded to SU");
		CRingSessionManagerWebClientItf::joinSession(charId, sessionId, clientApplication);
	}

	// A character asks to join a shard.
	// charId: userId << 4 & charSlot
	// If ingame, charSlot in [0..14], otherwise 15 if outgame
	// The actual shard will be chosen according to current load.
	// Will return a joinSessionResult.
	virtual void on_joinMainland(NLNET::TSockId from, uint32 charId, const std::string &clientApplication)
	{
		nldebug("joinMainland for char %u", charId);
		CHECK_AUTH_WITH_CHARID(from, charId);
		nldebug("  forwarded to SU");
		CRingSessionManagerWebClientItf::joinMainland(charId, clientApplication);
	}

	// Ask to join the edit session for the specified character.
	// If the edit session do not exist, then the SU
	// create it before internally calling the join session request
	// the SU.
	// Return joinSessionResult.
	virtual void on_joinEditSession(NLNET::TSockId from, uint32 charId, const std::string &clientApplication)
	{
		nldebug("joinEditSession for char %u", charId);
		CHECK_AUTH_WITH_CHARID(from, charId);
		nldebug("  forwarded to SU");
		CRingSessionManagerWebClientItf::joinEditSession(charId, clientApplication);
	}

	// Request to have the list of accessible shards with their attributes.
	// This is a dev feature only.
	virtual void on_getShards(NLNET::TSockId from, uint32 charId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::getShards(charId);
	}

	// Kick a character from a session
	// charId must be the owner of a DM in the session
	// invokeReturn : 0 : ok, character kicked
	//                1 : char not found
	//                2 : session not found
	//                3 : kicked character has no participation in the session
	//                4 : internal error
	//                5 : owner char don't own the session
	virtual void on_kickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 kickedCharId)
	{
		CHECK_AUTH_WITH_CHARID(from, ownerCharId);
		CRingSessionManagerWebClientItf::kickCharacter(ownerCharId, sessionId, kickedCharId);
	}

	// Unkick a character from a session
	// charId must be the owner of a DM in the session
	// invokeReturn : 0 : ok, character kicked
	//                1 : char not found
	//                2 : session not found
	//                3 : kicked character has no participation in the session
	//                4 : internal error
	//                5 : owner char don't own the session
	virtual void on_unkickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 unkickedCharId)
	{
		CHECK_AUTH_WITH_CHARID(from, ownerCharId);
		CRingSessionManagerWebClientItf::unkickCharacter(ownerCharId, sessionId, unkickedCharId);
	}

	// A user invite a guild to play in a session
	// invokeReturn : 0 : ok, guild invited
	//                1 : guild not found
	//                2 : char not found
	//                3 : session not found
	//                4 : guild already invited
	//                5 : char don't own the session
	virtual void on_inviteGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::inviteGuild(charId, sessionId, guildId);
	}

	// Remove a guild invitation in a session
	// invokeReturn : 0 : ok, guild invited
	//                1 : guild not found
	//                2 : char not found
	//                3 : session not found
	//                4 : guild not invited
	//                5 : char don't own the session
	virtual void on_removeInvitedGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::removeInvitedGuild(charId, sessionId, guildId);
	}

	// Set the additionnal scenario info
	// playType is the enumerated type TPlayType
	// invokeReturn : 0 : ok, info setted
	//                1 : scenario not found
	//                2 : char not owner of session
	//                3 : char not found
	virtual void on_setScenarioInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &title, uint32 numPlayer, const std::string &playType)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::setScenarioInfo(charId, sessionId, title, numPlayer, playType);
	}

	// Add an entry in the session journal
	// invokeReturn : 0 : ok, entry added
	//                1 : scenario not found
	//                2 : user can't post in this journal
	virtual void on_addJournalEntry(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &entryType, const std::string &text)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::addJournalEntry(charId, sessionId, entryType, text);
	}

	// Set the rating for a scenario
	// invokeReturn : 0 : ok, rating set
	//                1 : scenario not found
	//                2 : char is not found
	//                3 : char is not a participant of session
	//                4 : no session log for the session
	//                5 : char is banned from session
	//                6 : session not found
	//                7 : scenario not found
	//                8 : internal error
	virtual void on_setPlayerRating(NLNET::TSockId from, uint32 charId, TSessionId sessionId, 
		uint32 rateFun, 
		uint32 rateDifficulty, 
		uint32 rateAccessibility, 
		uint32 rateOriginality, 
		uint32 rateDirection)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);
		CRingSessionManagerWebClientItf::setPlayerRating(charId, sessionId, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection);
	}


	///////////////////////////////////////////////////////////////////////
	// CSessionBrowserServerWebItf implementation
	///////////////////////////////////////////////////////////////////////
	/// Connection callback : a new interface client connect
	virtual void on_CSessionBrowserServerWeb_Connection(NLNET::TSockId from)
	{

	}

	/// Disconnection callback : one of the interface client disconnect
	virtual void on_CSessionBrowserServerWeb_Disconnection(NLNET::TSockId from)
	{
		// remove auth info
		const TUserId *puid = _ClientAuths.getB(from);
		if (puid != NULL)
		{
			_ClientAuths.removeWithA(from);
		}
	}

	// The client send it's cookie information to
	// authenticate himself.
	// The cookie value is checked against the value stored.
	// in the database.
	// Furthermore, the server will check
	// evenly the database to see if the user is still
	// online and still have the same cookie
	virtual void on_authenticate(NLNET::TSockId from, uint32 userId, const NLNET::CLoginCookie &cookie)
	{
		nldebug("on_authenticate : connection %s : user %u send cookie %s",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				userId,
				cookie.toString().c_str());
		// check with the database
		CSString query;
		query << "SELECT cookie, current_status FROM ring_users WHERE user_id = "<<userId;

		if (!_RingDB.query(query))
		{
			nlinfo("Client from '%s' submited auth cookie '%s' for user %u, but DB request failed",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				cookie.toString().c_str(),
				userId);

			// close the connection
			_CallbackServer->disconnect(from);

			return;
		}
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());
		if (result->getNumRows() != 1)
		{
			nlinfo("Client from '%s' submited auth cookie '%s' for user %u, but DB return %u row instead of 1",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				cookie.toString().c_str(),
				userId,
				result->getNumRows());

			// close the connection
			_CallbackServer->disconnect(from);

			return;
		}

		result->fetchRow();
		string cookieStr;
		string connStatus;
		result->getField(0, cookieStr);
		result->getField(1, connStatus);
		CLoginCookie realCookie;
		realCookie.setFromString(cookieStr);
		if ((!realCookie.isValid()) || (realCookie != cookie))
		{
			nlinfo("Client from '%s' submited auth cookie '%s' for user %u, but cookie in DB is %s",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				cookie.toString().c_str(),
				userId,
				realCookie.toString().c_str());

			// close the connection
			_CallbackServer->disconnect(from);

			return;
		}
		if (connStatus != "cs_online")
		{	
			nlinfo("Client from '%s' submited auth cookie '%s' for user %u, but user is not online",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				cookie.toString().c_str(),
				userId);

			// close the connection
			_CallbackServer->disconnect(from);

			return;
		}

		// ok, all check are done
		// store the auth info
		if (_ClientAuths.getA(userId) != NULL)
		{
			nldebug("on_authorized : client %u is already authenticated with connection %s, ignore new auth",
					userId, from->getTcpSock()->remoteAddr().asString().c_str());
			/// auth already done with another connection, refuse the new one
			return;
		}
		_ClientAuths.add(from, userId);
	}
	
	// Ask for the list of session that are available
	// for the requesting character.
	virtual void on_getSessionList(NLNET::TSockId from, uint32 charId)
	{
		nldebug("on_getSessionList : getting sessions for char %u (new version)", charId); 
		CHECK_AUTH_WITH_CHARID(from, charId);

		// check for unit test
		if (charId == 0xfffffff0)
		{
			// do not respond to the client
			return;
		}

		// verify session at which the player can connect
		std::set<uint32> allowedSessions;
		std::set<uint32> kickedSessions;

		{

			CSString query;
			query << "SELECT session_id, kicked";
			query << " FROM session_participant WHERE char_id = "<< charId;
			BOMB_IF(!_RingDB.query(query), "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);
			auto_ptr<CStoreResult> result(_RingDB.storeResult());	
			uint32 row = result->getNumRows();
			uint32 index = 0;

			for (; index != row; ++index)
			{
				uint32 session_id;
				uint32 kicked;
				
				result->fetchRow();
				result->getField(0, session_id);
				result->getField(1, kicked);
				if (kicked == 0)
				{
					allowedSessions.insert(session_id);
				}
				else
				{
					kickedSessions.insert(session_id);
				}
			}
		}


		// read the character info
		CSString query;
		//					0			1					2					3		4			5		6
		query << "SELECT guild_id, best_combat_level, home_mainland_session_id, race, civilisation, cult, newcomer";
		query << " FROM characters WHERE char_id = "<<charId;

		BOMB_IF(!_RingDB.query(query), "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);
		auto_ptr<CStoreResult> result(_RingDB.storeResult());
		BOMB_IF(result->getNumRows() != 1, "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);
		result->fetchRow();
		uint32 guildId;
		uint32 bestCombatLevel;
		uint32 homeMainlandSessionId;
		bool newcomer;
		CHARSYNC::TRace race;
		CHARSYNC::TCivilisation civ;
		CHARSYNC::TCult cult;
		string s;

		result->getField(0, guildId);
		result->getField(1, bestCombatLevel);
		result->getField(2, homeMainlandSessionId);
		result->getField(3, s);
		race = TRace(s);
		result->getField(4, s);
		civ = TCivilisation(s);
		result->getField(5, s);
		cult = TCult(s);
		result->getField(6, newcomer);
		cult = TCult(s);

		uint32 shardIndex = CShardNames::getInstance().getShardIndex((TSessionId)homeMainlandSessionId);
		STOP_IF(shardIndex == 0xffffffff, "Invalid shard name");

		// issue the request in the session table (only Animation Sessions)
		//                     0                    1					2          
		query = "SELECT home_mainland_session_id, guild_id, sessions.session_id,";
		//			3				4         5						6					7
		query << " sessions.title, sessions.owner, start_date, sessions.description, sessions.orientation,";
		// 						8			9				10			11		12		13				14
		query << " sessions.anim_mode, char_name, sessions.level, session_type, lang, access_type, subscription_closed";
		//					15				16					17						18						19					20
		query << ", COUNT(rate_fun), AVG(rate_fun), AVG(rate_difficulty), AVG(rate_accessibility), AVG(rate_originality), AVG(rate_direction)";
		//					21							22
		query << ", scenario.rrp_total, scenario.allow_free_trial";
		query << " FROM sessions LEFT JOIN characters ON sessions.owner = char_id";
		query << " LEFT JOIN session_log ON sessions.session_id = session_log.id";
		query << " LEFT JOIN player_rating ON session_log.scenario_id = player_rating.scenario_id";
		query << " LEFT JOIN scenario ON session_log.scenario_id = scenario.id";
		
		query << " WHERE session_type = 'st_anim'";

		query << " AND state = 'ss_open'";
		query << " AND sessions.newcomer = "<<newcomer;

		// apply guild filter
		query << " AND (guild_filter = 'gf_any_player' OR guild_id = "<<guildId<<")";
		// apply civilisation filter
		if ((civ == TCivilisation::c_neutral) || (civ == TCivilisation::invalid_val))
			query << " AND (FIND_IN_SET('rf_"<<race.toString().substr(2)<<"', race_filter) > 0) ";
		else
			query << " AND (FIND_IN_SET('rf_"<<civ.toString().substr(2)<<"', race_filter) > 0) ";

		// apply religion filter
		query << " AND (FIND_IN_SET('rf_"<<cult.toString().substr(2)<<"', religion_filter) > 0) ";
		// apply shard filter
		query << " AND (FIND_IN_SET('sf_shard"<<toString("%02u", shardIndex)<<"', shard_filter) > 0) ";
		// apply combat level filter
//		const char levelLeter[]="abcdeeeeeeee";
		TSessionLevel combatLevel;
		combatLevel.fromCharacterLevel(bestCombatLevel);
		query << " AND (FIND_IN_SET('lf_"<<combatLevel.toString().substr(3)<<"', level_filter) > 0) ";
//		query << " AND level = 'sl_"<<levelLeter[min(uint32(sizeofarray(levelLeter)-1), (bestCombatLevel-1)/50)]<<"'";
		// apply closed subscription filter
//		query << " AND subscription_closed = 0";

		// A final group by clause to group the player rating for each session record
		query << " GROUP BY sessions.session_id";


		BOMB_IF(!_RingDB.query(query), "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);
		auto_ptr<CStoreResult> result2(_RingDB.storeResult());
//		BOMB_IF(result->getNumFields() != 1, "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);

		// parse the result set and build the return vector
		vector<TSessionDesc>	sessionDescs;
		sessionDescs.reserve(result2->getNumRows());

		nlinfo("---- sessions ----");
		for (uint i=0; i<result2->getNumRows(); ++i)
		{
			result2->fetchRow();
			TSessionDesc sd;
			uint32 homeSessionId;
//			uint32 guild;
			uint32 sessionNum;
			TAccessType accessType;
			bool	subscriptionClosed;
			bool	autoInvite;
			uint32 nbRating, rateFun, rateDiff, rateAcc, rateOri, rateDir;
			uint32 rrpTotal;
			bool	allowFreeTrial;
			string s;

			// read the data
			result2->getField(2, sessionNum);
			TSessionId sessionId(sessionNum);

//			nlinfo("Session: %d",sessionNum);
			sd.setSessionId(sessionId);
			result2->getField(0, homeSessionId);
			result2->getField(9, s);
			s = CShardNames::getInstance().makeFullName(s, homeSessionId);
//			nlinfo("- Owner: %s",s.c_str());
			sd.setOwnerName(s);
			result2->getField(3, s);
//			nlinfo("- Title: %s",s.c_str());
			sd.setTitle(s);
			result2->getField(6, s);
			sd.setDescription(s);
			result2->getField(7, s);
			sd.setOrientation(TSessionOrientation(s));
//			nlinfo("- Orientation: %s",s.c_str());
			result2->getField(8, s);
			sd.setAnimMode(TAnimMode(s));
			result2->getField(10, s);
			sd.setSessionLevel(TSessionLevel(s));
			result2->getField(12, s);
//			nlinfo("- Language: %s",s.c_str());
			sd.setLanguage(s);
			result2->getField(13, s);
			accessType = TAccessType(s);
			autoInvite = (accessType == TAccessType::at_public);
			result2->getField(14, s);
			NLMISC::fromString(s, subscriptionClosed);

			result2->getField(15, nbRating);
			sd.setNbRating(nbRating);
			result2->getField(16, rateFun);
			sd.setRateFun(rateFun);
			result2->getField(17, rateDiff);
			sd.setRateDifficulty(rateDiff);
			result2->getField(18, rateAcc);
			sd.setRateAccessibility(rateAcc);
			result2->getField(19, rateOri);
			sd.setRateOriginality(rateOri);
			result2->getField(20, rateDir);
			sd.setRateDirection(rateDir);
			result2->getField(21, rrpTotal);
			sd.setScenarioRRPTotal(rrpTotal);
			result2->getField(22, allowFreeTrial);
			sd.setAllowFreeTrial(allowFreeTrial);
			

			uint32 startDate;
			result2->getDateField(5, startDate);
			sd.setLaunchDate(startDate);
			
			sd.setRequesterCharInvited(allowedSessions.find(sessionNum) != allowedSessions.end() || accessType == TAccessType::at_public || autoInvite);
			sd.setRequesterCharKicked(kickedSessions.find(sessionNum) != kickedSessions.end());
			
			// Calculate number of connected players
			// TODO : do this a bit faster
			query =  "SELECT COUNT(*) FROM ring_users";
			query << " WHERE current_status='cs_online'";
			query << " AND current_session="<< sd.getSessionId();
			BOMB_IF(!_RingDB.query(query), "error", CSessionBrowserServerWebItf::_CallbackServer->disconnect(from); return);
			auto_ptr<CStoreResult> result3(_RingDB.storeResult());
			BOMB_IF(result2->getNumRows()<1,"Expected 1 result from SQL nbPlayers request but retrieved none",return);
			uint32 nbPlayers;
			result3->fetchRow();
			result3->getField(0, nbPlayers);
			sd.setNbConnectedPlayer(nbPlayers);

			// check for closed subscription
			if (!subscriptionClosed || allowedSessions.find(sessionNum) != allowedSessions.end())
			{
				// only display session with subscription open or where the player is invited
				sessionDescs.push_back(sd);
			}
		}

		sessionList(from, charId, sessionDescs);
	}

	// Ask for the list of players that are available
	// for the requesting session.
	virtual void on_getCharList(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);

		// do the 'request of the death' !
		CSString query;
		//                             0                                                                                                      1
		query << "SELECT characters.char_id, (characters.current_session = "<<sessionId<<" AND ring_users.current_status = 'cs_online') AS connected,";
		//                       2                     3                4
		query << " characters.char_name, characters.guild_id, guilds.guild_name,";
		//                              5                     6                   7
		query << " characters.best_combat_level, characters.race, characters.civilisation, ";
		//                     8                          9                   10
		query << " characters.cult, session_participant.status, characters.home_mainland_session_id, ";
		//                    11
		query << " session_participant.kicked";
		query << " FROM sessions, session_participant, characters, ring_users LEFT JOIN guilds ON characters.guild_id = guilds.guild_id";
		query << " WHERE sessions.session_id = "<<sessionId;
		query << " AND sessions.session_id = session_participant.session_id";
		query << " AND characters.char_id = session_participant.char_id";
		query << " AND ring_users.user_id = characters.user_id";

		BOMB_IF(!_RingDB.query(query), "getCharList : error executing request in database", charList(from, charId, sessionId, vector <TCharDesc>()); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		vector <TCharDesc> charDescs;
		charDescs.resize(result->getNumRows());

		for (uint i=0; i<result->getNumRows(); ++i)
		{
			result->fetchRow();
			TCharDesc &cd = charDescs[i];

			uint32	charId;
			uint8	connected;
			string	charName;
			uint32	guildId;
			string	guildName;
			uint32	bestCombatLevel;
			uint32	shardId;
			uint32	kicked;
//			TRace	race;
//			TCivilisation civ;
//			TCult	cult;
//			TSessionPartStatus partStatus;
			string	temp;

			// read the database field
			result->getField(0, charId);
			result->getField(1, connected);
			result->getField(2, charName);
			result->getField(3, guildId);
			if (guildId != 0)
				result->getField(4, guildName);
			result->getField(5, bestCombatLevel);
			TSessionLevel level;
			level.fromCharacterLevel(bestCombatLevel);
			result->getField(6, temp);
			TRace race(temp);
			result->getField(7, temp);
			TCivilisation civ(temp);
			result->getField(8, temp);
			TCult cult(temp);
			result->getField(9, temp);
			TSessionPartStatus partStatus(temp);
			result->getField(10, shardId);
			result->getField(11, kicked);


			cd.setCharId(charId);
			cd.setConnected(connected != 0);
			cd.setCharName(charName);
			cd.setGuildName(guildName);
			cd.setLevel(level);
			cd.setRace(race);
			cd.setCivilisation(civ);
			cd.setCult(cult);
			cd.setPartStatus(partStatus);
			cd.setShardId(shardId);
			cd.setKicked(kicked != 0);
		}

		charList(from, charId, sessionId, charDescs);
	}

	// Invite a player in a session given it's name
	// This method make a certain number of asumption :
	// The sessionId is deducted from the current session id of
	// the requester character.
	// The invited char id is deducted from the name by using.
	// the full name rules for shard resolution.
	// Return invoke_result with the following error codes :
	//   0   : no error
	//   100 : unknown player
	//   101 : player already invited
	//   102 : no current session
	virtual void on_inviteCharacterByName(NLNET::TSockId from, uint32 charId, std::string invitedCharName)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);

		nldebug ("SBS : inviteCharacterByName : char %u invite character '%s' in his session",
			charId, invitedCharName.c_str());

		// we first need to retrieve the host character infos
		CSString query;
		query << "SELECT home_mainland_session_id, current_session FROM characters WHERE char_id = "<<charId;
		BOMB_IF(!_RingDB.query(query), "invitedCharacterByName : failed request in database", invokeResult(from, charId >> 4, 103, "Database request failed"); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		BOMB_IF (result->getNumRows() == 0, "invitedCharacterByName : can't find char "<<charId<<" in the characters table", invokeResult(from, charId >> 4, 100, "Owner requester character"); return);
		result->fetchRow();
		uint32 homeSessionId;
		result->getField(0, homeSessionId);
		uint32 sessionNum;
		result->getField(1, sessionNum);
		TSessionId sessionId(sessionNum);

		// Use the shard names singleton to resolve the user name
		TSessionId invitedCharHome;
		string shortName;
		CShardNames::getInstance().parseRelativeName(TSessionId(homeSessionId), invitedCharName, shortName, invitedCharHome);

		// request the database to retrieve the invited character Id
		query="";
		query << "SELECT char_id FROM characters WHERE char_name = '"<<shortName<<"' AND home_mainland_session_id = "<<invitedCharHome.asInt();

		BOMB_IF(!_RingDB.query(query), "invitedCharacterByName : failed request 2 in database", invokeResult(from, charId >> 4, 103, "Database request failed"); return);
		result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		BOMB_IF (result->getNumRows() == 0, "invitedCharacterByName : can't find invited char '"<<shortName<<"' from shard "<<invitedCharHome.asInt()<<" in the characters table", invokeResult(from, charId >> 4, 104, "Invited char not found"); return);
		result->fetchRow();
		uint32 invitedCharId;
		result->getField(0, invitedCharId);

		// do the real invitation with the SU
		CRingSessionManagerWebClientItf::inviteCharacter(charId, sessionId, invitedCharId, TSessionPartStatus::sps_play_invited);

		// the return is done with SU return
	}

	// Ask for character existing rating for the current session scenario
	// return playerRatings.
	virtual void on_getMyRatings(NLNET::TSockId from, uint32 charId, uint32 sessionId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);

		// retrieve the rating (if any) sets by this player regarding a scenario in the
		// specified session

		CSString query;
		//					0			1					2				3					4
		query << "SELECT rate_fun, rate_difficulty, rate_accessibility, rate_originality, rate_direction";
		query << " FROM sessions";
		query << " LEFT JOIN characters ON char_id = author";
		query << " LEFT JOIN session_log ON sessions.session_id = session_log.id";
		query << " LEFT JOIN player_rating ON player_rating.scenario_id = session_log.scenario_id";

		query << " WHERE char_id = "<<charId<<" AND session_log.id = "<<sessionId;

		BOMB_IF(!_RingDB.query(query), "getRingRatings : failed request in database", playerRatings(from, charId, false, 0,0,0,0,0); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		if (result->getNumRows() == 0)
		{
			// no rating from this character
			playerRatings(from, charId, false, 0,0,0,0,0);
			return;
		}

		result->fetchRow();
		uint32 rateFun, rateDiff, rateAcc, rateOri, rateDir;
		result->getField(0, rateFun);
		result->getField(1, rateDiff);
		result->getField(2, rateAcc);
		result->getField(3, rateOri);
		result->getField(4, rateDir);

		// send the result back to the client
		playerRatings(from, charId, true, rateFun, rateDiff, rateAcc, rateOri, rateDir);
	}

	// Ask the average scores of a session
	// return sessionAverageScores.
	virtual void on_getSessionAverageScores(NLNET::TSockId from, uint32 sessionId)
	{
		CSString query;
		//					0				1						2						3						4						5			6
		query << "SELECT COUNT(rate_fun), AVG(rate_fun), AVG(rate_difficulty), AVG(rate_accessibility), AVG(rate_originality), AVG(rate_direction), rrp_scored";
		query << " FROM sessions";
		query << " LEFT JOIN session_log ON sessions.session_id = session_log.id";
		query << " LEFT JOIN player_rating ON player_rating.scenario_id = session_log.scenario_id";

		query << " WHERE session_log.id = "<<sessionId;
		query << " GROUP BY session_log.id";

		BOMB_IF(!_RingDB.query(query), "getScessionAverageScores : failed request in database", sessionAverageScores(from, false, 0,0,0,0,0, 0); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		if (result->getNumRows() == 0)
		{
			sessionAverageScores(from, false, 0,0,0,0,0, 0);
			return;	
		}
		
		result->fetchRow();

		uint32 ratesNb;
		result->getField(0, ratesNb);
		if (ratesNb == 0)
		{
			// no rating from this scenario
			sessionAverageScores(from, false, 0,0,0,0,0, 0);
			return;
		}

		uint32 rateFun, rateDiff, rateAcc, rateOri, rateDir, rrpScored;
		result->getField(1, rateFun);
		result->getField(2, rateDiff);
		result->getField(3, rateAcc);
		result->getField(4, rateOri);
		result->getField(5, rateDir);
		result->getField(6, rrpScored);
		
		// send the result back to the client
		sessionAverageScores(from, true, rateFun, rateDiff, rateAcc, rateOri, rateDir, rrpScored);
	}

	// Ask for average scores of a scenario
	// return scessionAverageScores.
	virtual void on_getScenarioAverageScores(NLNET::TSockId from, const std::string &md5)
	{
		CSString query;
		//					0				1					2						3						4						5				6
		query << "SELECT COUNT(rate_fun), AVG(rate_fun), AVG(rate_difficulty), AVG(rate_accessibility), AVG(rate_originality), AVG(rate_direction), rrp_total";
		query << " FROM scenario";
		query << " LEFT JOIN player_rating ON player_rating.scenario_id = scenario.id";

		query << " WHERE scenario.md5 = '"<< MSW::escapeString(md5, _RingDB) << "'";
		query << " GROUP BY scenario.id";

		BOMB_IF(!_RingDB.query(query), "getScenarioAverageScores : failed request in database", scenarioAverageScores(from, false, 0,0,0,0,0,0); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());
		
		if (result->getNumRows() == 0)
		{
			scenarioAverageScores(from, false, 0,0,0,0,0, 0);
			return;	
		}

		result->fetchRow();

		uint32 ratesNb;
		result->getField(0, ratesNb);
		if (ratesNb == 0)
		{
			// no rating from this scenario
			scenarioAverageScores(from, false, 0,0,0,0,0, 0);
			return;
		}

		uint32 rateFun, rateDiff, rateAcc, rateOri, rateDir, rrpTotal;
		result->getField(1, rateFun);
		result->getField(2, rateDiff);
		result->getField(3, rateAcc);
		result->getField(4, rateOri);
		result->getField(5, rateDir);
		result->getField(6, rrpTotal);
		
		// send the result back to the client
		scenarioAverageScores(from, true, rateFun, rateDiff, rateAcc, rateOri, rateDir, rrpTotal);
	}


	// Ask for the author rating, the AM rating and the Masterless rating
	// for the requesting character.
	virtual void on_getRingRatings(NLNET::TSockId from, uint32 charId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);

		CSString query;
		query << "SELECT rrp_am, rrp_masterless, rrp_author FROM characters WHERE char_id = "<<charId;
		BOMB_IF(!_RingDB.query(query), "getRingRatings : failed request in database", ringRatings(from, charId, 0, 0, 0); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		BOMB_IF (result->getNumRows() == 0, "getRingRatings : can't find char "<<charId<<" in the characters table", ringRatings(from, charId, 0, 0, 0); return);
		result->fetchRow();
		uint32 rrpAm, rrpMasterless, rrpAuthor;
		result->getField(0, rrpAm);
		result->getField(1, rrpMasterless);
		result->getField(2, rrpAuthor);

		// send the result back to the client
		ringRatings(from, charId, rrpAuthor, rrpAm, rrpMasterless);
	}

	// Ask for ring points of the character
	virtual void on_getRingPoints(NLNET::TSockId from, uint32 charId)
	{
		CHECK_AUTH_WITH_CHARID(from, charId);

		CSString query;
		query << "SELECT ring_access FROM characters WHERE char_id = "<<charId;
		BOMB_IF(!_RingDB.query(query), "on_getRingPoints: failed request in database", ringPoints(from, charId, "", MaxRingPoints); return);
		auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

		BOMB_IF (result->getNumRows() == 0, "on_getRingPoints : can't find char "<<charId<<" in the characters table", ringPoints(from, charId, "", MaxRingPoints); return);
		result->fetchRow();
		string ringAccess;
		result->getField(0, ringAccess);

		// send the result back to the client
		ringPoints(from, charId, ringAccess, MaxRingPoints);
	}




	// Send the Footer message of a multi-part message that will be forwarded to DSS via sbs
	void on_forwardToDss(NLNET::TSockId from, uint32 charId, const NLNET::CMessage& msg)
	{		
		CHECK_AUTH_WITH_CHARID(from, charId);
		BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);	

		NLNET::CMessage message(msg);		
		/*
		if (!message.isReading())
		{
			message.invert();
		}
		*/
		_ServerEditionProxy->sendModuleMessage(this, message);
		
		
	}



	///////////////////////////////////////////////////////////////////////
	// CRingSessionManagerWebClientItf implementation
	///////////////////////////////////////////////////////////////////////
	/// Disconnection callback : the connection to the server is lost
	virtual void on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId from)
	{
		// we have lost conn with the SU, we need to disconnect all
		// connected clients, discard all pending request, and 
		// wait for reconnection to the SU

		// disconnect all clients
		CSessionBrowserServerWebItf::_CallbackServer->disconnect(InvalidSockId);

		_ServerConnected = false;

		// TODO : remove all pending request

	}


	// Generic response to invoke.
	// result contains 0 if no error, more than 0 in case of error
	virtual void on_invokeResult(NLNET::TSockId from, uint32 userId, uint32 resultCode, const std::string &resultString)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(userId);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it invoke result", userId);
			return;
		}

		invokeResult(*psock, userId, resultCode, resultString);
	}

	// result is : 0 : session have been created fine
	//             1 : invalid session type
	//             2 : invalid level
	//             3 : unknown character
	//             4 : not used
	//             5 : invalid access type
	//             6 : invalid rule type
	//             7 : invalid duration
	virtual void on_scheduleSessionResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(charId >> 4);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it invoke result", charId>>4);
			return;
		}

		scheduleSessionResult(*psock, charId, sessionId, result, resultString);
	}

	//
	virtual void on_sessionInfoResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const RSMGR::TRaceFilter &raceFilter, const RSMGR::TReligionFilter &religionFilter, 
		const RSMGR::TGuildFilter &guildFilter, const RSMGR::TShardFilter &shardFilter, const RSMGR::TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation, const std::string &description)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(charId >> 4);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it invoke result", charId>>4);
			return;
		}

		sessionInfoResult(*psock, charId, sessionId, raceFilter, religionFilter, guildFilter, 
			shardFilter, levelFilter, subscriptionClosed, autoInvite, language, orientation, description);
	}

	// Return the result of the session joining attempt
	// If join is ok, the shardAddr contain <ip:port> of the
	// Front end that waits for the player to come in and the.
	// participation mode for the character (editor, animator or player).
	// If ok, the web must return a page with a lua script.
	// that trigger the action handler 'on_connect_to_shard' :
	// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
	// result : 0 : ok the client can join the session
	//          1 : char not found
	//          2 : session not found
	//          3 : no session participant for this character (not used for a mainland shard)
	//          4 : can't find session server (not used for a mainland shard)
	//          5 : shard hosting session is not reachable
	//          6 : nel user info not found
	//          7 : ring user not found
	//          8 : welcome service rejected connection request
	//          9 : session service shutdown (not used for a mainland shard)
	//         10 : no mainland shard found (joinMainland only)
	//         11 : internal error
	//         12 : failed to request for access permission
	//         13 : can't find access permission for user and domain
	//         14 : Welcome service is closed for you
	//		   15 : ??
	virtual void on_joinSessionResult(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(userId);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it joinSessionResult", userId);
			return;
		}

		joinSessionResult(*psock, userId, sessionId, result, shardAddr, participantStatus);
	}

	// See joinSessionResult.
	// Adds a security code.
	virtual void on_joinSessionResultExt(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus, const CSecurityCode& securityCheckForFastDisconnection)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(userId);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it joinSessionResultExt", userId);
			return;
		}

		joinSessionResultExt(*psock, userId, sessionId, result, shardAddr, participantStatus, securityCheckForFastDisconnection);
	}

	// Return the list of online shards on which the user is allowed to connect,
	// and their current dynamic attributes. Other attributes (e.g. names)
	// can be queried from the database. Offline shards are the ones in the database
	// of the same domain but not listed in the result.
	// Then the client will have to call joinShard to connect on an online shard.
	virtual void on_getShardsResult(NLNET::TSockId from, uint32 userId, const std::string &result)
	{
		// forward to the appropriate client
		const TSockId *psock = _ClientAuths.getA(userId);
		if (psock == NULL)
		{
			nlwarning("Can't find user %u to send it invoke result", userId);
			return;
		}

		getShardsResult(*psock, userId, result);

	}

};

NLNET_REGISTER_MODULE_FACTORY(CSessionBrowserServerMod, "SessionBrowserServerMod");



NLNET::TUnifiedCallbackItem cbArraySU[] = 
{ 
	{"", NULL}
};

// declare the serice
NLNET_SERVICE_MAIN(CSessionBrowserServer, "SBS", "session_browser_server", 0, cbArraySU, "", "");



void CSessionBrowserServer::init()
{
	CSingletonRegistry::getInstance()->init();
}

bool CSessionBrowserServer::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	
	return true;
}

void CSessionBrowserServer::release()
{
	CSingletonRegistry::getInstance()->release();
}


