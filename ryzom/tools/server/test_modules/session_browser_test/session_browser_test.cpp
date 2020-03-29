
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"
#include "nel/net/service.h"
#include "nel/net/module_manager.h"

#include "src/cpptest.h"

#include "game_share/mysql_wrapper.h"


#include "client/session_browser.h"


#include <cstdlib>
#include <mysql.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;


extern void session_browser_force_link();

void foo()
{
	session_browser_force_link();
}


const nbUserField = 4;
const char *userData[][nbUserField] =
{
	{	"10000", "test_user1", "DF01A8C0|0674E006|00002710", "cs_offline"},
	{	"10001", "test_user2", "DF01A8C0|0674E006|00002711", "cs_online"},
	{	"268435455", "bad_user", "DF01A8C0|0674E006|FFFFFFFF", "cs_online"},
};


class CSessionBrowserTS
	:	public Test::Suite,
		public CSessionBrowser
{
	string			_WorkingPath;
//	string			_BaseDir;
//	string			_CurrentPath;
//	string			_CurrentPathToRestore;

	bool			_ConnFailed;
	bool			_DisconnectionReceived;
	bool			_SessionReceived;
	std::vector < TSessionDesc >	_Sessions;

	MSW::CConnection _RingDb;

	IService		_Service;


public:

	CSessionBrowserTS(const std::string &workingPath)
	{
		_WorkingPath = workingPath;

		NLMISC::createDebug();


		// add a variable
		CConfigFile::CVar mainlandNames;
		mainlandNames.Type = CConfigFile::CVar::T_STRING;
		mainlandNames.StrValues.resize(9);
		mainlandNames.setAsString("101", 0);
		mainlandNames.setAsString("Aniro", 1);
		mainlandNames.setAsString("ani", 2);
		mainlandNames.setAsString("102", 3);
		mainlandNames.setAsString("Leanon", 4);
		mainlandNames.setAsString("lea", 5);
		mainlandNames.setAsString("103", 6);
		mainlandNames.setAsString("Arispotle", 7);
		mainlandNames.setAsString("ari", 8);

		IService::getInstance()->ConfigFile.insertVar("HomeMainlandNames", mainlandNames);

		// check database population
		nlverify(_RingDb.connect("borisb2", "root", "", "ring"));


		CCommandRegistry &cr = CCommandRegistry::getInstance();
		cr.execute("moduleManager.createModule StandardGateway gw", InfoLog());
		cr.execute("moduleManager.createModule SessionBrowserServerMod sbs suAddr=localhost:49999 listenPort=1234 ring_db(host=borisb2 user=ring password= base=ring)", InfoLog());
		cr.execute("suc.plug gw", InfoLog());


		// fill the database with some users

		// cleanup the previously created users
		CSString query;
		query << "DELETE FROM ring_users WHERE user_id >= 10000";
		_RingDb.query(query);

		for (uint i=0; i<sizeofarray(userData); ++i)
		{
			CSString query;
			query << "INSERT INTO ring_users (user_id, user_name, cookie, current_status)";
			query << " VALUES (";
			for (uint j=0; j<nbUserField; ++j)
			{
				query << "'" << userData[i][j] <<"'";
				if (j != nbUserField-1)
					query << ", ";
			}
			query << ")";
			_RingDb.query(query);
		}

		// fill the database with some character
		const nbCharField = 9;
		const char *charData[][nbCharField] =
		{
			//	char_id, char_name, user_id, guil_id, best_combat_level, home_mainland_session_id, race, civilisation, cult
			{	"160000", "test_char1", "10000", "0", "20", "102", "r_fyros", "c_fyros", "c_neutral" },
			{	"160016", "test_char2", "10001", "0", "20", "102", "r_fyros", "c_fyros", "c_neutral" },
			{	"4294967280", "bad_char", "268435455", "0", "20", "102", "r_fyros", "c_fyros", "c_neutral" },
		};

		// cleanup the previously created users
		query = "DELETE FROM characters WHERE char_id >= 160000";
		_RingDb.query(query);

		for (uint i=0; i<sizeofarray(charData); ++i)
		{
			CSString query;
			query << "INSERT INTO characters (char_id, char_name, user_id, guild_id, best_combat_level, home_mainland_session_id, race, civilisation, cult)";
			query << " VALUES (";
			for (uint j=0; j<nbCharField; ++j)
			{
				query << "'" << charData[i][j] <<"'";
				if (j != nbCharField-1)
					query << ", ";
			}
			query << ")";
			_RingDb.query(query);
		}



#define RACE_FILTER_ALL "rf_neutral,rf_matis,rf_fyros,rf_tryker,rf_zorai"
#define RELIG_ALL "rf_neutral,rf_kami,rf_karavan"

		// fill the database with some guilds
		const uint nbSessionField = 9;
		const char *sessionData[][nbSessionField] =
		{
			//  session_id	session_type	title	owner		level	state		host_shard_id	race_filter	religion_filter
			{	"100000",	"st_anim",		"ts_1", "16000",	"sl_a", "ss_open",	"100",			RACE_FILTER_ALL , RELIG_ALL },
			{	"100001",	"st_anim",		"ts_2", "16001",	"sl_b", "ss_open",	"100",			RACE_FILTER_ALL , RELIG_ALL },
			{	"100002",	"st_anim",		"ts_2", "16001",	"sl_b", "ss_open",	"100",			"rf_matis" , RELIG_ALL },
		};

		// cleanup the previously created sessions
		query = "DELETE FROM sessions WHERE session_id >= 100000";
		_RingDb.query(query);


		for (uint i=0; i<sizeofarray(sessionData); ++i)
		{
			CSString query;
			query << "INSERT INTO sessions (session_id, session_type, title, owner, level, state, host_shard_id, race_filter, religion_filter, guild_filter)";
			query << " VALUES (";
			for (uint j=0; j<nbSessionField; ++j)
			{
				query << "'" << sessionData[i][j] <<"'";
				if (j != nbSessionField-1)
					query << ", ";
			}
			query << ", 'gf_any_player')";
			_RingDb.query(query);
		}


		// add the tests methods
		TEST_ADD(CSessionBrowserTS::failAuth);
		TEST_ADD(CSessionBrowserTS::listSession);
		TEST_ADD(CSessionBrowserTS::failListSession_disconnected);

	}

	void setup()
	{
		_ConnFailed = false;
		_DisconnectionReceived = false;
		_SessionReceived = false;
		_Sessions.clear();
	}

	~CSessionBrowserTS()
	{
		CPath::setCurrentPath(_WorkingPath.c_str());

//		delete _Context;
	}

	void updateNetwork()
	{
		for (uint i=0; i<50; ++i)
		{
			nlSleep(100);
			update();
			IModuleManager::getInstance().updateModules();
		}
	}

	void connectAndAuth(uint32 userIndex)
	{
		CLoginCookie cookie;
		cookie.setFromString(userData[userIndex][2]);
		setAuthInfo(cookie);
		connectItf(CInetAddress("localhost:1234"));
	}

	void failListSession_disconnected()
	{
		connectAndAuth(2);

		getSessionList(0xffffffff);

		// update the network 
		updateNetwork();

		TEST_ASSERT(_DisconnectionReceived);
		TEST_ASSERT(!_SessionReceived);
		TEST_ASSERT(_Sessions.size() == 0);
	}

	void failAuth()
	{
		connectItf(CInetAddress("localhost:1234"));

		_Sessions.clear();
		_SessionReceived = false;

		getSessionList(160016);

		// update the network 
		updateNetwork();

		TEST_ASSERT(_DisconnectionReceived);
		TEST_ASSERT(!_SessionReceived);
	}


	void listSession()
	{
		connectAndAuth(1);

		getSessionList(160016);

		// update the network 
		updateNetwork();

		TEST_ASSERT(_DisconnectionReceived);
		TEST_ASSERT(_SessionReceived);
		TEST_ASSERT(_Sessions.size() == 1);
	}

	///////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////

	/// Disconnection callback : the connection to the server is lost
	virtual void on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId from)
	{
		nlstop;
	}


	// Generic response to invoke.
	// result contains 0 if no error, more than 0 in case of error
	virtual void on_invokeResult(NLNET::TSockId from, uint32 userId, uint32 resultCode, const std::string &resultString)
	{
		// forward the response to the appropriate client

		nlstop;
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
		nlstop;
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
	virtual void on_joinSessionResult(NLNET::TSockId from, uint32 userId, uint32 result, const std::string &shardAddr, const std::string &participantStatus)
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


	///////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////
	/// Disconnection callback : the connection to the server is lost
	virtual void on_CSessionBrowserServerWebClient_Disconnection(NLNET::TSockId from)
	{
		nldebug("Lost connection with server");

		_DisconnectionReceived = true;
	}

	// Return the list of available session
	virtual void on_sessionList(NLNET::TSockId from, uint32 charId, const std::vector < TSessionDesc > &sessions)
	{
		_SessionReceived = true;
		_Sessions = sessions;
	}

	///////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////
	virtual void on_connectionFailed() 
	{
		_ConnFailed = true;
	}

//	// The connection has been closed.
//	virtual void on_connectionClosed()
//	{
//		nlstop;
//	}

};

NLMISC::CApplicationContext	*Context;

auto_ptr<Test::Suite> intRegisterTestSuite(const std::string &workingPath)
{

	Context = new CApplicationContext();
	// init a new nel instance
	INelContext::getInstance();
	// init the module manager
	IModuleManager::getInstance();
	return static_cast<Test::Suite*>(new CSessionBrowserTS(workingPath));
}

NL_LIB_EXPORT_SYMBOL(registerTestSuite, void, intRegisterTestSuite);

