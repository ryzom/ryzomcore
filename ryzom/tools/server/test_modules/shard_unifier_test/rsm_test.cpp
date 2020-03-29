/** Test file for the ring sesison manager module */

#include "nel/misc/types_nl.h"
#include "nel/misc/command.h"

#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/net/unified_network.h"

#include "src/cpptest.h"


#include "../../nelns/welcome_service/welcome_service_itf.h"
#include "game_share/ring_session_manager_itf.h"
#include "game_share/mysql_wrapper.h"
#include "shard_unifier_service/database_mapping.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;
using namespace WS;

// some globals for the test
TSessionId sessionId(0);

uint32	userId = 1;
uint32	charId = (userId*16)+0;

uint32	invitedUserId = 2;
uint32	invitedCharId = (invitedUserId*16)+0;

uint32	subscriberUserId = 3;
uint32	subscriberCharId = (subscriberUserId*16)+0;

uint32 guildId = 1;

// place holder module for welcome service
class CPHWelcomeServiceModule :
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CWelcomeServiceSkel
{
public:
	CPHWelcomeServiceModule ()
	{
		CWelcomeServiceSkel::init(this);
	}

//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		nlverify(CWelcomeServiceSkel::onDispatchMessage(sender, message));
//	}

	// ask the welcome service to welcome a user
	void welcomeUser(NLNET::IModuleProxy *sender, 
		uint32 charId, 
		const std::string &userName, 
		const NLNET::CLoginCookie &cookie, 
		const std::string &priviledge, 
		const std::string &exPriviledge, 
		WS::TUserRole mode, 
		uint32 instanceId)
	{
		CWelcomeServiceClientProxy client(sender);
		client.welcomeUserResult(this, charId>>4, true, "localhost", "");
	}
	// ask the welcome service to disconnect a user
	void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId)
	{
	}

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "RingSessionManager")
		{
			CWelcomeServiceClientProxy client(module);
			// register against the session manager
			client.registerWS(this, 1, 0, true);
		}
	}

};

NLNET_REGISTER_MODULE_FACTORY(CPHWelcomeServiceModule, "WelcomeService");


// place holder module for server edition module
class CPHServerEditionModule : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CRingSessionManagerClientSkel
{
	TModuleProxyPtr		_SessionManager;
public:

	// a set of kick reported to DSS by RSM
	set<uint32>	KickedChars;

	CPHServerEditionModule()
	{
		CRingSessionManagerClientSkel::init(this);
	}

//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		nlverify(CRingSessionManagerClientSkel::onDispatchMessage(sender, message));
//	}

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "RingSessionManager")
		{
			CRingSessionManagerProxy rsm(module);
			// register against the session manager
			std::vector < TRunningSessionInfo > runningSession;
			rsm.registerDSS(this, 1, runningSession);

			_SessionManager = module;
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		if (module == _SessionManager)
		{
			_SessionManager = NULL;
		}
	}

	// Ask the client to create a new session modules
	virtual void createSession(NLNET::IModuleProxy *sender, uint32 ownerCharId, TSessionId sessionId, RSMGR::TSessionType type)
	{
		// the shard unifier ask to start a session, be fair with him
		CRingSessionManagerProxy rsm(sender);

		TRunningSessionInfo rsi;
		rsi.setInstanceId(sessionId);
		rsi.setNbPlayingChars(0);
		rsi.setSessionId(sessionId);

		rsm.sessionCreated(this, rsi);

	}

	// Ask the client allow a helper character in the session
	virtual void addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId, WS::TUserRole enterAs, const std::string &ringAccess, bool newcomer)
//	virtual void addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId, WS::TUserRole enterAs)
	{
		
	}
	// Ask the client to close a running session
	virtual void closeSession(NLNET::IModuleProxy *sender, TSessionId sessionId)
	{
	}

	void characterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId)
	{
		KickedChars.insert(charId);
	}

	virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId)
	{
	}


	// we ask to the rsm to kick a character
	void kickACharacter(TSessionId sessionId, uint32 charId)
	{
		nlassert(_SessionManager != NULL);

		// send a kick report to the session manager
		CRingSessionManagerProxy rsm(_SessionManager);

		rsm.reportCharacterKicked(this, sessionId, charId);
	}

	// Ask the client to create a new session modules
	virtual void createSession(NLNET::IModuleProxy *sender, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type)
	{
		nlstop;
	}
	// Ask the client to allow a character in the session
	virtual void addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer)
	{
		nlstop;
	}
	// Ask the client stop hibernation for the
	// specified session. This mean to remove any
	// hibernated scenario file from the backup.
	virtual void stopHibernation(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 ownerId)
	{
		nlstop;
	}
	// Session mananger report that a character has been unkicked by the web
	virtual void characterUnkicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId)
	{
		nlstop;
	}
	// Ask to hibernate a session
	virtual void hibernateSession(NLNET::IModuleProxy *sender, TSessionId sessionId)
	{
		nlstop;
	}
	// Set the start position of a session (eg while your are uploading an animation session)
	virtual void setSessionStartParams(NLNET::IModuleProxy *sender, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason)
	{
		nlstop;
	}


};

NLNET_REGISTER_MODULE_FACTORY(CPHServerEditionModule, "ServerEditionModule");

/** place holder class for the web communication */
class CPHWeb : public RSMGR::CRingSessionManagerWebClientItf
{
public:

	vector<uint32> LastResults;

	uint32		LastSession;

	/// Disconnection callback : the connection to the server is lost
	virtual void on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId from)
	{
	}


	// Generic response to invoke.
	// result contains 0 if no error, more than 0 in case of error
	virtual void on_invokeResult(NLNET::TSockId from, uint32 userId, uint32 resultCode, const std::string &resultString)
	{
		LastResults.push_back(resultCode);
	}

	// result is : 0 : session have been created fine
	//             1 : invalid session type
	//             2 : invalid level
	//             3 : unknown user
	//             4 : not used
	//             5 : invalid access type
	//             6 : invalid rule type
	//             7 : invalid duration
	virtual void on_scheduleSessionResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString)
//	virtual void on_scheduleSessionResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint8 result)
	{
		LastResults.push_back(result);
		LastSession = sessionId;
	}

	// session info result (anim)
	virtual void on_sessionInfoResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation, const std::string &description)
	{

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
	//         15 : Session is not open
	//         16 : User banned from ring
	//         17 : Newcomer flag missmatch
	//         18 : Can't find session log to validate session access
	//         19 : Can't find scenario info to validate session access
	//         20 : Scenario is not allowed to free trial players
	virtual void on_joinSessionResult(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus)
	{
		LastResults.push_back(result);
	}

	// See joinSessionResult.
	// Adds a security code.
	virtual void on_joinSessionResultExt(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus, const CSecurityCode &securityCheckForFastDisconnection)
	{
		nlstop;
	}

	// Return the list of online shards on which the user is allowed to connect,
	// and their current dynamic attributes. Other attributes (e.g. names)
	// can be queried from the database. Offline shards are the ones in the database
	// of the same domain but not listed in the result.
	// Then the client will have to call joinShard to connect on an online shard.
	virtual void on_getShardsResult(NLNET::TSockId from, uint32 userId, const std::string &result)
	{
		nlstop;
	}


};



class CRSMTS: public Test::Suite
{
	MSW::CConnection _Conn;

	CPHServerEditionModule	*ServerEditionModule;

public:
	CRSMTS(const std::string &workingPath)
	{
		ServerEditionModule = NULL;

		// force instanciation of the module manager
		IModuleManager::getInstance();
		// create the session manager module
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		string DBHost = "borisb2";

		cr.execute("moduleManager.createModule StandardGateway gw", InfoLog());
		cr.execute(string()+"moduleManager.createModule RingSessionManager rsm web(port=49999) ring_db(host="+DBHost+" user=root password base=ring) nel_db(host="+DBHost+" user=root password base=nel) noPerm", InfoLog());
		cr.execute("rsm.plug gw", InfoLog());
		cr.execute("moduleManager.createModule ServerEditionModule sam", InfoLog());
		cr.execute("sam.plug gw", InfoLog());

		cr.execute("moduleManager.createModule WelcomeService ws", InfoLog());
		cr.execute("ws.plug gw", InfoLog());


		// check database population
		nlverify(_Conn.connect(DBHost, "root", "", "ring"));

		// check the user id
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// create a new record
			ru = CRingUser::createTransient(__FILE__, __LINE__);

			ru->setUserName("toto");
//			ru->setCurrentSession(0);

			nlverify(ru->create(_Conn));

			userId = ru->getObjectId();
		}
		else
		{
//			ru->setCurrentSession(0);
			ru->update(_Conn);
		}


		charId = (userId * 16)+0;

		// check the character
		CCharacterPtr character = CCharacter::load(_Conn, charId, __FILE__, __LINE__);
		if (character == NULL)
		{
			// create a character
			character = CCharacter::createTransient(__FILE__, __LINE__);

			character->setObjectId(charId);
			character->setCharName("toto_char");
			character->setUserId(userId);
			character->setGuildId(0);
			character->setBestCombatLevel(100);

			nlverify(character->create(_Conn));

		}

		// check the invited user id
		ru = CRingUser::load(_Conn, invitedUserId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// create a new record
			ru = CRingUser::createTransient(__FILE__, __LINE__);

			ru->setUserName("invited");
//			ru->setCurrentSession(0);

			nlverify(ru->create(_Conn));

			invitedUserId = ru->getObjectId();
		}
		else
		{
//			ru->setCurrentSession(0);
			ru->update(_Conn);
		}

		// check the subscriber user id
		ru = CRingUser::load(_Conn, subscriberUserId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// create a new record
			ru = CRingUser::createTransient(__FILE__, __LINE__);

			ru->setUserName("subscriber");
//			ru->setCurrentSession(0);

			nlverify(ru->create(_Conn));

			invitedUserId = ru->getObjectId();
		}
		else
		{
//			ru->setCurrentSession(0);
			ru->update(_Conn);
		}


		invitedCharId = (invitedUserId * 16)+0;

		// check the character
		character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		if (character == NULL)
		{
			// create a character
			character = CCharacter::createTransient(__FILE__, __LINE__);

			character->setObjectId(invitedCharId);
			character->setCharName("invited_char");
			character->setUserId(invitedUserId);
			character->setGuildId(0);
			character->setBestCombatLevel(100);

			nlverify(character->create(_Conn));

		}

		// check the character
		subscriberCharId = (subscriberUserId * 16)+0;
		character = CCharacter::load(_Conn, subscriberCharId, __FILE__, __LINE__);
		if (character == NULL)
		{
			// create a character
			character = CCharacter::createTransient(__FILE__, __LINE__);

			character->setObjectId(subscriberCharId);
			character->setCharName("subscriber_char");
			character->setUserId(subscriberUserId);
			character->setGuildId(0);
			character->setBestCombatLevel(100);

			nlverify(character->create(_Conn));
		}


		// check the guild used for the test
		CGuildPtr guild = CGuild::load(_Conn, guildId, __FILE__, __LINE__);
		if (guild == 0)
		{
			// create guild
			guild = CGuild::createTransient(__FILE__, __LINE__);
			guild->setGuildName("A test guild");
			guild->setObjectId(guildId);
			nlverify(guild->create(_Conn));
		}

		// add the tests
		TEST_ADD(CRSMTS::scheduleSession);
		TEST_ADD(CRSMTS::inviteChar);
		TEST_ADD(CRSMTS::startSession);
		TEST_ADD(CRSMTS::removeInvitedCharacter);
		TEST_ADD(CRSMTS::joinSession);
		TEST_ADD(CRSMTS::updateSessionInfo);
		TEST_ADD(CRSMTS::subscribeInSession);
		TEST_ADD(CRSMTS::unsubscribeInSession);
		TEST_ADD(CRSMTS::inviteGuild);
		TEST_ADD(CRSMTS::kickCharacterWithDSS);
		TEST_ADD(CRSMTS::unkickCharacter);
		TEST_ADD(CRSMTS::kickCharacterWithWeb);
		
		TEST_ADD(CRSMTS::cancelSession);
		TEST_ADD(CRSMTS::closeSession);
		TEST_ADD(CRSMTS::addFriendCharacter);
		TEST_ADD(CRSMTS::setKnownUserComment);
		TEST_ADD(CRSMTS::removeFriendCharacter);
		TEST_ADD(CRSMTS::addBannedCharacter);
		TEST_ADD(CRSMTS::removeBannedCharacter);
		TEST_ADD(CRSMTS::addJournalEntry);
		TEST_ADD(CRSMTS::setPlayerRating);

		
		TEST_ADD(CRSMTS::dssDown);


	}

	~CRSMTS()
	{
		// delete the session manager module
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		cr.execute("moduleManager.deleteModule rsm", InfoLog());
		cr.execute("moduleManager.deleteModule sam", InfoLog());
		cr.execute("moduleManager.deleteModule gw", InfoLog());

		// remove the object created for the test
		if (userId != 1)
		{
			CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
			if (ru != NULL)
				ru->remove(_Conn);
			ru = CRingUser::load(_Conn, invitedUserId, __FILE__, __LINE__);
			if (ru != NULL)
				ru->remove(_Conn);
			ru = CRingUser::load(_Conn, subscriberUserId, __FILE__, __LINE__);
			if (ru != NULL)
				ru->remove(_Conn);

			CGuildPtr guild = CGuild::load(_Conn, guildId, __FILE__, __LINE__);
			if (guild != NULL)
				guild->remove(_Conn);
		}
		_Conn.closeConn();

		ServerEditionModule = NULL;
	}

	void dssDown()
	{
		// this test validate the correction for the dss down rsm crash

		// unplug the dss and see what happen (normaly nothing)
		CCommandRegistry &cr = CCommandRegistry::getInstance();
		cr.execute("moduleManager.deleteModule sam", InfoLog());

	}

	void setup()
	{
		ServerEditionModule = dynamic_cast<CPHServerEditionModule*>(IModuleManager::getInstance().getLocalModule("sam"));
	}

	void tear_down()
	{
		ServerEditionModule = NULL;
	}

	void setPlayerRating()
	{
		// TODO : update this code
//		TEST_ASSERT(userId != 0);
//		TEST_ASSERT(charId != 0);
//		TEST_ASSERT(sessionId != 0);
//
//		// add an entry in the session journal
//
//		CPHWeb	web;
//		web.connectItf(NLNET::CInetAddress("localhost", 49999));
//
//		// 1st, cleanup the session ratings
//		CSessionPtr session = CSession::load(_Conn, sessionId, __FILE__, __LINE__);
//		TEST_ASSERT(session != NULL);
//		TEST_ASSERT(session->loadPlayerRatings(_Conn, __FILE__, __LINE__));
//		while (!session->getPlayerRatings().empty())
//		{
//			TEST_ASSERT(session->getPlayerRatingsByIndex(session->getPlayerRatings().size()-1)->remove(_Conn));
//		}
//		// then close the session
//		session->setState(TSessionState::ss_closed);
//		session->update(_Conn);
//
//		// add the journal entry
//		web.setPlayerRating(charId, sessionId, 2, "a rating comment");
//
//		updateNetwork(web);
//
//		TEST_ASSERT(web.LastResults.size() == 1);
//		TEST_ASSERT(web.LastResults.back() == 0);
//
//		// chek the session
//		TEST_ASSERT(session->getPlayerRatings().size() == 1);
//		CPlayerRatingPtr pr = session->getPlayerRatingsByIndex(0);
//		TEST_ASSERT(pr->getAuthor() == charId);
//		TEST_ASSERT(pr->getRating() == 2);
//		TEST_ASSERT(pr->getComments() == "a rating comment");
	}

	void addJournalEntry()
	{
		TEST_ASSERT(userId != 0);
		TEST_ASSERT(charId != 0);
		TEST_ASSERT(sessionId != 0);

		// add an entry in the session journal

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// 1st, cleanup the session journal
		CSessionPtr session = CSession::load(_Conn, sessionId, __FILE__, __LINE__);
		TEST_ASSERT(session != NULL);
		TEST_ASSERT(session->loadJournalEntries(_Conn, __FILE__, __LINE__));
		while (!session->getJournalEntries().empty())
		{
			TEST_ASSERT(session->getJournalEntriesByIndex(session->getJournalEntries().size()-1)->remove(_Conn));
		}
		// set the character in 'sps_animatig' mode in the session
		TEST_ASSERT(session->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		uint i=0;
		for (uint i=0; i<session->getSessionParticipants().size(); ++i)
		{
			if (session->getSessionParticipantsByIndex(i)->getCharId() == charId)
			{
				// we found the participation, update it
				session->getSessionParticipantsByIndex(i)->setStatus(TSessionPartStatus::sps_animating);

				session->getSessionParticipantsByIndex(i)->update(_Conn);
			}
		}

		// add the journal entry
		web.addJournalEntry(charId, sessionId, "jet_credits", "a journal credits text");

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		// check the session
		TEST_ASSERT(session->getJournalEntries().size() == 1);
		CJournalEntryPtr je = session->getJournalEntriesByIndex(0);
		TEST_ASSERT(je->getType() == TJournalEntryType::jet_credits);
		TEST_ASSERT(je->getAuthor() == charId);
		TEST_ASSERT(je->getText() == "a journal credits text");
	}

	void removeBannedCharacter()
	{
		TEST_ASSERT(userId != 0);
		TEST_ASSERT(invitedCharId != 0);
		// add a friend char to the user (need the previous test to have worked)

		// 1st, load the user and clean the friend list
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);
		const vector<CKnownUserPtr> &kus = ru->getKnownUsers();

		// check starting state
		TEST_ASSERT(ru->getKnownUsers().size() == 1);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getTargetCharacter() == invitedCharId);

		// clear the cache
		ru = CRingUserPtr();
		NOPE::CPersistentCache::getInstance().clearCache();

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// add the friend
		web.removeBannedCharacter(userId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(ru->getKnownUsers().size() == 0);
	}


	void addBannedCharacter()
	{
		TEST_ASSERT(userId != 0);
		TEST_ASSERT(invitedCharId != 0);
		// add a friend char to the user (need the previous test to have worked)

		// 1st, load the user and clean the friend list
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);
		const vector<CKnownUserPtr> &kus = ru->getKnownUsers();

		// check starting state
		TEST_ASSERT(ru->getKnownUsers().size() == 0);

		// clear the cache
		ru = CRingUserPtr();
		NOPE::CPersistentCache::getInstance().clearCache();

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// add the friend
		web.addBannedCharacter(userId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(ru->getKnownUsers().size() == 1);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getTargetCharacter() == invitedCharId);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getRelation() == TKnownUserRelation::rt_banned);
	}

	void removeFriendCharacter()
	{
		TEST_ASSERT(userId != 0);
		TEST_ASSERT(invitedCharId != 0);
		// add a friend char to the user (need the previus test to have worked)

		// 1st, load the user and clean the friend list
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);
		const vector<CKnownUserPtr> &kus = ru->getKnownUsers();

		// check starting state
		TEST_ASSERT(ru->getKnownUsers().size() == 1);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getTargetCharacter() == invitedCharId);

		// clear the cache
		ru = CRingUserPtr();
		NOPE::CPersistentCache::getInstance().clearCache();

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// add the friend
		web.removeFriendCharacter(userId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(ru->getKnownUsers().size() == 0);
	}

	void setKnownUserComment()
	{
		// set a comment on a known user
		
		// 1st, load the user and clean the comment (if any)
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);
		const vector<CKnownUserPtr> &kus = ru->getKnownUsers();

		// loop until we found the know user we are looking for
		for (uint i=0; i<kus.size(); ++i)
		{
			CKnownUserPtr ku = ru->getKnownUsersByIndex(i);

			TEST_ASSERT(ku != NULL);

			if (ku->getTargetCharacter() == invitedCharId)
			{
				// ok, it's this one
				ku->setComments("");
				break;
			}
		}

		// create the web caller
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// set the comment
		web.setKnownCharacterComments(userId, invitedCharId, "rt_friend", "the comment");

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		TEST_ASSERT(ru->loadKnownUsers(_Conn, __FILE__, __LINE__));

		// look for the friend
		for (uint i=0; i<ru->getKnownUsers().size(); ++i)
		{
			CKnownUserPtr ku = ru->getKnownUsersByIndex(i);
			if (ku->getTargetCharacter() == invitedCharId)
			{
				TEST_ASSERT(ku->getComments() == "the comment");
				break;
			}
		}
	}

	void addFriendCharacter()
	{
		TEST_ASSERT(userId != 0);
		TEST_ASSERT(invitedCharId != 0);
		// add a friend char to the user

		// 1st, load the user and clean the friend list
		CRingUserPtr ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		TEST_ASSERT(ru != NULL);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);
		const vector<CKnownUserPtr> &kus = ru->getKnownUsers();

		// loop until container is empty
		while (!kus.empty())
		{
			CKnownUserPtr ku = ru->getKnownUsersByIndex(kus.size() -1);

			// remove this one
			ku->remove(_Conn);
		}

		// clear the cache
		ru = CRingUserPtr();
		NOPE::CPersistentCache::getInstance().clearCache();

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// add the friend
		web.addFriendCharacter(userId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		ru = CRingUser::load(_Conn, userId, __FILE__, __LINE__);
		ru->loadKnownUsers(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(ru->getKnownUsers().size() == 1);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getTargetCharacter() == invitedCharId);
		TEST_ASSERT(ru->getKnownUsersByIndex(0)->getRelation() == TKnownUserRelation::rt_friend);

	}

	void closeSession()
	{
		TEST_ASSERT(sessionId != 0);
		// close a running session
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.closeSession(charId, sessionId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);
	}

	void cancelSession()
	{
		// try to cancel the session
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// ask to create an editing session
		web.scheduleSession(
			charId,
			TSessionType::st_edit,
			"Session Title",
//			NLMISC::CTime::getSecondsSince1970(),
			"Session Description",
			R2::TSessionLevel::sl_a,
//			TAccessType::at_public,
			TRuleType::rt_liberal,
			TEstimatedDuration::et_medium,
			0,
			TAnimMode::am_dm,
			TRaceFilter(TRaceFilterEnum::rf_fyros),
			TReligionFilter(TReligionFilterEnum::rf_kami),
			TGuildFilter::gf_any_player,
			TShardFilter(TShardFilterEnum::sf_shard00),
			TLevelFilter(TLevelFilterEnum::lf_a),
			"fr",
			TSessionOrientation::so_other,
			false,
			true);

		// update the network
		updateNetwork(web);

		TSessionId sessionId = web.LastSession;

		web.cancelSession(charId, sessionId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 2);
		TEST_ASSERT(web.LastResults.back() == 0);

		// ask to create an animation session
		web.scheduleSession(
			charId,
			TSessionType::st_anim,
			"Session Title",
//			NLMISC::CTime::getSecondsSince1970(),
			"Session Description",
			R2::TSessionLevel::sl_a,
//			TAccessType::at_public,
			TRuleType::rt_liberal,
			TEstimatedDuration::et_medium,
			0,
			TAnimMode::am_dm,
			TRaceFilter(TRaceFilterEnum::rf_fyros),
			TReligionFilter(TReligionFilterEnum::rf_kami),
			TGuildFilter::gf_any_player,
			TShardFilter(TShardFilterEnum::sf_shard00),
			TLevelFilter(TLevelFilterEnum::lf_a),
			"fr",
			TSessionOrientation::so_other,
			false,
			true);


		// update the network
		updateNetwork(web);

		sessionId = web.LastSession;

		web.cancelSession(charId, sessionId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 4);
		TEST_ASSERT(web.LastResults.back() == 0);

	
	}

	void removeInvitedGuild()
	{
		TEST_ASSERT(sessionId != 0);
		TEST_ASSERT(guildId != 0);

		// cleanup the invitation
		CGuildPtr guild = CGuild::load(_Conn, guildId, __FILE__, __LINE__);
		TEST_ASSERT(guild != NULL);
		guild->loadInvites(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(guild->getInvites().size() == 1);
		TEST_ASSERT(guild->getInvites().back()->getSessionId() == sessionId);

		// invite a guild in the session
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.removeInvitedGuild(charId, sessionId, guildId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		TEST_ASSERT(guild->getInvites().size() == 0);
	}

	void kickCharacterWithWeb()
	{
		TEST_ASSERT(sessionId != 0);
		TEST_ASSERT(ServerEditionModule != NULL);
		TEST_ASSERT(invitedCharId != 0);

		CCharacterPtr character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		TEST_ASSERT(character->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == false);

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.kickCharacter(charId, sessionId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == true);

		TEST_ASSERT(ServerEditionModule->KickedChars.find(invitedCharId) != ServerEditionModule->KickedChars.end());
	}


	void unkickCharacter()
	{
		TEST_ASSERT(sessionId != 0);
		TEST_ASSERT(ServerEditionModule != NULL);
		TEST_ASSERT(invitedCharId != 0);

		CCharacterPtr character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		TEST_ASSERT(character->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == true);

		ServerEditionModule->KickedChars.clear();

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.unkickCharacter(charId, sessionId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == false);

		TEST_ASSERT(ServerEditionModule->KickedChars.empty());
	}


	void kickCharacterWithDSS()
	{
		TEST_ASSERT(sessionId != 0);
		TEST_ASSERT(ServerEditionModule != NULL);
		TEST_ASSERT(invitedCharId != 0);

		CCharacterPtr character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		TEST_ASSERT(character->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == false);

		// kick a character from the session from DSS
		ServerEditionModule->kickACharacter(sessionId, invitedCharId);

		for (uint i=0; i<10; ++i)
		{
			IModuleManager::getInstance().updateModules();

			nlSleep(10);
		}


		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getKicked() == true);
	}

	void inviteGuild()
	{
		TEST_ASSERT(sessionId != 0);
		TEST_ASSERT(guildId != 0);

		// cleanup the invitation
		CGuildPtr guild = CGuild::load(_Conn, guildId, __FILE__, __LINE__);
		TEST_ASSERT(guild != NULL);
		guild->loadInvites(_Conn, __FILE__, __LINE__);
		while (!guild->getInvites().empty())
		{
			guild->getInvitesByIndex(guild->getInvites().size()-1)->remove(_Conn);
		}

		// invite a guild in the session
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.inviteGuild(charId, sessionId, guildId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		guild = CGuild::load(_Conn, guildId, __FILE__, __LINE__);
		TEST_ASSERT(guild != NULL);
		guild->loadInvites(_Conn, __FILE__, __LINE__);
		TEST_ASSERT(guild->getInvites().size() == 1);
		TEST_ASSERT(guild->getInvites().back()->getSessionId() == sessionId);
	}

	void unsubscribeInSession()
	{
		// try to unsubscribe a character from a session
		TEST_ASSERT(sessionId != 0);
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// unsubscribe from the session
		web.unsubscribeSession(subscriberCharId, sessionId);
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		// check the subscribed slot count
		CSessionPtr session = CSession::load(_Conn, sessionId, __FILE__, __LINE__);
		TEST_ASSERT(session != NULL);
		TEST_ASSERT(session->getSubscriptionSlots() == 5);
		TEST_ASSERT(session->getReservedSlots() == 0);

		// check the invitation
		TEST_ASSERT(session->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		const vector<CSessionParticipantPtr> &parts = session->getSessionParticipants();
		uint i;
		for (i=0; i<parts.size(); ++i)
		{
			if (parts[i]->getCharId() == subscriberCharId)
				break;
		}
		TEST_ASSERT(i == parts.size());
	}
	void subscribeInSession()
	{
		// try to subscribe a character in a session
		TEST_ASSERT(sessionId != 0);
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// subscribe in the session, must fail because no slot available
		web.subscribeSession(subscriberCharId, sessionId);
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 6);

		// update the session info, allowing some place for subscriber
		web.updateSessionInfo(charId, 
			sessionId, 
			"updated title", 
			NLMISC::CTime::getSecondsSince1970()+24*12*60*60, 
			"updated description", 
			R2::TSessionLevel::sl_b, 
//			TAccessType::at_public, 
			TEstimatedDuration::et_long,
			5,
			TRaceFilter(TRaceFilterEnum::rf_fyros),
			TReligionFilter(TReligionFilterEnum::rf_kami),
			TGuildFilter::gf_any_player,
			TShardFilter(TShardFilterEnum::sf_shard00),
			TLevelFilter(TLevelFilterEnum::lf_a),
			false,
			true,
			"fr",
			TSessionOrientation::so_other);

		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 2);
		TEST_ASSERT(web.LastResults.back() == 0);

		// retry to subscribe in the session
		web.subscribeSession(subscriberCharId, sessionId);
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 3);
		TEST_ASSERT(web.LastResults.back() == 0);

		// check the subscribed slot count
		CSessionPtr session = CSession::load(_Conn, sessionId, __FILE__, __LINE__);
		TEST_ASSERT(session != NULL);
		TEST_ASSERT(session->getSubscriptionSlots() == 5);
		TEST_ASSERT(session->getReservedSlots() == 1);

		// check the invitation
		TEST_ASSERT(session->loadSessionParticipants(_Conn, __FILE__, __LINE__));
		const vector<CSessionParticipantPtr> &parts = session->getSessionParticipants();
		uint i;
		for (i=0; i<parts.size(); ++i)
		{
			if (parts[i]->getCharId() == subscriberCharId)
				break;
		}
		TEST_ASSERT(i != parts.size());

	}

	void updateSessionInfo()
	{
		TEST_ASSERT(sessionId != 0);
		// try to update the session info
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.updateSessionInfo(charId, 
			sessionId, 
			"updated title", 
			NLMISC::CTime::getSecondsSince1970()+24*12*60*60, 
			"updated description", 
			R2::TSessionLevel::sl_b, 
//			TAccessType::at_public, 
			TEstimatedDuration::et_long,
			0,
			TRaceFilter(TRaceFilterEnum::rf_fyros),
			TReligionFilter(TReligionFilterEnum::rf_kami),
			TGuildFilter::gf_any_player,
			TShardFilter(TShardFilterEnum::sf_shard00),
			TLevelFilter(TLevelFilterEnum::lf_a),
			false,
			true,
			"fr",
			TSessionOrientation::so_other);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);
	}

	void joinSession()
	{
		TEST_ASSERT(sessionId != 0);
		// try to join the session with the two character	
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// join for the first character
		web.joinSession(charId, sessionId, "borisb2");
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		// re-invite the second character
		web.inviteCharacter(charId, sessionId, invitedCharId, TSessionPartStatus(TSessionPartStatus::sps_edit_invited).toString());
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 2);
		TEST_ASSERT(web.LastResults.back() == 0);


		// join for the second character
		web.joinSession(invitedCharId, sessionId, "borisb2");
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 3);
		TEST_ASSERT(web.LastResults.back() == 0);
	}

	void removeInvitedCharacter()
	{
		TEST_ASSERT(sessionId != 0);

		// remove the invitation for a character
		CCharacterPtr character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		character->loadSessionParticipants(_Conn, __FILE__, __LINE__);
		TEST_ASSERT(character->getSessionParticipants().size() == 1);
		TEST_ASSERT(character->getSessionParticipantsByIndex(0)->getSessionId() == sessionId);

		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.removeInvitedCharacter(charId, sessionId, invitedCharId);

		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		TEST_ASSERT(character->getSessionParticipants().size() == 0);
	}


	void startSession()
	{
		TEST_ASSERT(sessionId != 0);
		// start a session
		CPHWeb	web;
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.startSession(charId, sessionId);

		updateNetwork(web);

		// check the result
		TEST_ASSERT(!web.LastResults.empty());
		TEST_ASSERT(web.LastResults.back() == 0);
	}

	void inviteChar()
	{
		TEST_ASSERT(sessionId != 0);
		// invite a character in a scheduled session
		CPHWeb	web;

		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		web.inviteCharacter(charId, sessionId, charId, TSessionPartStatus(TSessionPartStatus::sps_edit_invited).toString());
		// update the network
		updateNetwork(web);

		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		// cleanup the database for other test
		CCharacterPtr character = CCharacter::load(_Conn, invitedCharId, __FILE__, __LINE__);
		TEST_ASSERT(character != NULL);

		character->loadSessionParticipants(_Conn, __FILE__, __LINE__);
		// delete any existing participation
		
		while (!character->getSessionParticipants().empty())
		{
			character->getSessionParticipantsByIndex(character->getSessionParticipants().size()-1)->remove(_Conn);
		}

		// invite for the second character
		web.inviteCharacter(charId, sessionId, invitedCharId, TSessionPartStatus(TSessionPartStatus::sps_edit_invited).toString());
		updateNetwork(web);
		// check the result
		TEST_ASSERT(web.LastResults.size() == 2);
		TEST_ASSERT(web.LastResults.back() == 0);
	}

	void scheduleSession()
	{
		// schedule a session from the web
		CPHWeb	web;

		// connect the web to the session manager module
		web.connectItf(NLNET::CInetAddress("localhost", 49999));

		// ask to create a session
		web.scheduleSession(
			charId,
			TSessionType::st_edit,
			"Session Title",
//			NLMISC::CTime::getSecondsSince1970(),
			"Session Description",
			R2::TSessionLevel::sl_a,
//			TAccessType::at_public,
			TRuleType::rt_liberal,
			TEstimatedDuration::et_medium,
			0,
			TAnimMode::am_dm,
			TRaceFilter(TRaceFilterEnum::rf_fyros),
			TReligionFilter(TReligionFilterEnum::rf_kami),
			TGuildFilter::gf_any_player,
			TShardFilter(TShardFilterEnum::sf_shard00),
			TLevelFilter(TLevelFilterEnum::lf_a),
			"fr",
			TSessionOrientation::so_other,
			false,
			true);


		// update the network
		updateNetwork(web);
		
		// check that the session have been scheduled
		TEST_ASSERT(web.LastResults.size() == 1);
		TEST_ASSERT(web.LastResults.back() == 0);

		// store the session ID for later
		sessionId = web.LastSession;
	}

	void updateNetwork(CPHWeb &web)
	{
		for (uint i=0; i<10; ++i)
		{
			web.update();
			IModuleManager::getInstance().updateModules();

			nlSleep(10);
		}
	}

};


Test::Suite *createCRSMTS(const std::string &workingPath)
{
	return new CRSMTS(workingPath);
}
