// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/misc/types_nl.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/log.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/login_cookie.h"

#include "ryzom_welcome_service.h"

#include "game_share/welcome_service_itf.h"
#include "game_share/mirror.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace WS;

extern void admin_modules_forceLink();

void foo()
{
	admin_modules_forceLink();
}


CVariable<sint> PlayerLimit(
	"ws","PlayerLimit", "Rough max number of players accepted on this shard (-1 for Unlimited)",
	5000,
	0, true );

// Forward declaration of callback cbShardOpen (see ShardOpen variable)
void	cbShardOpen(IVariable &var);

// Forward declaration of callback cbShardOpenStateFile (see ShardOpenStateFile variable)
void	cbShardOpenStateFile(IVariable &var);

// Forward declaration of callback cbUsePatchMode
void	cbUsePatchMode(IVariable &var);

// Types of open state
enum TShardOpenState
{
	ClosedForAll = 0,
	OpenOnlyForAllowed = 1,
	OpenForAll = 2
};

static bool	AllowDispatchMsgToLS = false;

/**
 * ShardOpen
 * true if shard is open to public
 * 0 means closed for all but :DEV:
 * 1 means open only for groups in config file (see OpenGroups variable) and :DEV:
 * 2 means open for all
 */
CVariable<uint>		ShardOpen("ws", "ShardOpen", "Indicates if shard is open to public (0 closed for all but :DEV:, 1 open only for groups in cfg, 2 open for all)", 2, 0, true, cbShardOpen);

/**
 * ShardOpenStateFile
 * true if shard is open to public
 */
CVariable<string>	ShardOpenStateFile("ws", "ShardOpenStateFile", "Name of the file that contains ShardOpen state", "", 0, true, cbShardOpenStateFile);

/**
 * OpenGroups
 */
CVariable<string>	OpenGroups("ws", "OpenGroups", "list of groups allowed at ShardOpen Level 1", "", 0, true);

/**
 * OpenFrontEndThreshold
 * The FS balance algorithm works like this:
 * - select the least loaded frontend
 * - if this frontend has more than the OpenFrontEndThreshold
 *   - try to open a new frontend
 *   - reselect least loaded frontend
 */
CVariable<uint>		OpenFrontEndThreshold("ws", "OpenFrontEndThreshold", "Limit number of players on all FS to decide to open a new FS", 800,	0, true );


/**
 * Use Patch mode
 */
CVariable<bool>		UsePatchMode("ws", "UsePatchMode", "Use Frontends as Patch servers (at FS startup)", true, 0, true, cbUsePatchMode );

/**
 * Use Patch mode
 */
CVariable<bool>		DontUseLS("ws", "DontUseLS", "Don't use the login service", false, 0, true);


// Shortcut to the module instance
//CWelcomeServiceMod	*CWelcomeServiceMod::_Instance = NULL;


/**
 * Using expected services and current running service instances, this class
 * reports a main "online status".
 */
class COnlineServices
{
public:

	/// Set expected instances. Ex: { "TICKS", "FS", "FS", "FS" }
	void		setExpectedInstances( CConfigFile::CVar& var )
	{
		// Reset "expected" counters (but don't clear the map, keep the running instances)
		CInstances::iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			(*ici).second.Expected = 0;
		}
		// Rebuild "expected" counters
		for ( uint i=0; i!=var.size(); ++i )
		{
			++_Instances[var.asString(i)].Expected;
		}
	}

	/// Add a service instance
	void		addInstance( const std::string& serviceName )
	{
		++_Instances[serviceName].Running;
	}

	/// Remove a service instance
	void		removeInstance( const std::string& serviceName )
	{
		CInstances::iterator ici = _Instances.find( serviceName );
		if ( ici != _Instances.end() )
		{
			--(*ici).second.Running;

			// Remove from the map only if not part of the expected list
			if ( ((*ici).second.Expected == 0) && ((*ici).second.Running == 0) )
			{
				_Instances.erase( ici );
			}
		}
		else
		{
			nlwarning( "Can't remove instance of %s", serviceName.c_str() );
		}
	}

	/// Check if all expected instances are online
	bool		getOnlineStatus() const
	{
		CInstances::const_iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			if ( ! ici->second.isOnlineAsExpected() )
				return false;
		}
		return true;
	}

	/// Display contents
	void		display( NLMISC::CLog& log = *NLMISC::DebugLog )
	{
		CInstances::const_iterator ici;
		for ( ici=_Instances.begin(); ici!=_Instances.end(); ++ici )
		{
			log.displayNL( "%s: %s (%u expected, %u running)",
				(*ici).first.c_str(),
				(*ici).second.Expected ? ((*ici).second.isOnlineAsExpected() ? "ONLINE" : "MISSING") : "OPTIONAL",
				(*ici).second.Expected, (*ici).second.Running );
		}
	}

private:

	struct TInstanceCounters
	{
		TInstanceCounters() : Expected(0), Running(0) {}

		// If not expected, count as online as well
		bool isOnlineAsExpected() const { return Running >= Expected; }

		uint	Expected;
		uint	Running;
	};

	typedef std::map< std::string, TInstanceCounters > CInstances;

	CInstances	_Instances;
};

/// Online services
COnlineServices OnlineServices;


/// Main online status
bool OnlineStatus;

/// Send changes of status to the LS
void reportOnlineStatus( bool newStatus )
{
	if ( newStatus != OnlineStatus && AllowDispatchMsgToLS )
	{
		if (!DontUseLS)
		{
			CMessage msgout( "OL_ST" );
			msgout.serial( newStatus );
			CUnifiedNetwork::getInstance()->send( "LS", msgout );
		}

		if (CWelcomeServiceMod::isInitialized())
		{
			// send a status report to welcome service client
			CWelcomeServiceMod::getInstance()->reportWSOpenState(newStatus);
		}

		OnlineStatus = newStatus;
	}
}



/// Set the version of the shard. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same to run correctly)
static const uint32 ServerVersion = 1;

/// Contains the correspondance between userid and the FES connection where the userid is connected.
map<uint32, TServiceId> UserIdSockAssociations;

// ubi hack
string FrontEndAddress;



enum TFESState
{
	PatchOnly,
	AcceptClientOnly
};

struct CFES
{
	CFES (TServiceId sid) : SId(sid), NbPendingUsers(0), NbUser(0), State(PatchOnly) { }

	TServiceId	SId;				// Connection to the front end
	uint32		NbPendingUsers;		// Number of not yet connected users (but rooted to this frontend)
	uint32		NbUser;				// Number of user currently connected on this front end

	TFESState	State;				// State of frontend (patching/accepting clients)
	std::string	PatchAddress;		// Address of frontend patching server

	uint32		getUsersCountHeuristic() const
	{
		return NbUser + NbPendingUsers;
	}

	void		setToAcceptClients()
	{
		if (State == AcceptClientOnly)
			return;

		// tell FS to accept client
		State = AcceptClientOnly;
		CMessage	msgOpenFES("FS_ACCEPT");
		CUnifiedNetwork::getInstance()->send(SId, msgOpenFES);

		// report state to LS
		bool	dummy;
		reportStateToLS(dummy, true);
	}

	void		reportStateToLS(bool& reportPatching, bool alive = true)
	{
		// report to LS

		bool	patching = (State == PatchOnly);
		if (alive && patching)
			reportPatching = true;

		if ( AllowDispatchMsgToLS )
		{
			if (!DontUseLS)
			{
				CMessage	msgout("REPORT_FS_STATE");
				msgout.serial(SId);
				msgout.serial(alive);
				msgout.serial(patching);
				msgout.serial(PatchAddress);
				CUnifiedNetwork::getInstance()->send("LS", msgout);
			}
		}
	}
};

list<CFES> FESList;

/*
 * Find the best front end service for a new connecting user (return NULL if there is no suitable FES).
 * Additionally, calculate totalNbUsers.
 */
CFES *findBestFES ( uint& totalNbUsers )
{
	totalNbUsers = 0;

	CFES*	best = NULL;

	for (list<CFES>::iterator it=FESList.begin(); it!=FESList.end(); ++it)
	{
		CFES &fes = *it;
		if (fes.State == AcceptClientOnly)
		{
			if (best == NULL || best->getUsersCountHeuristic() > fes.getUsersCountHeuristic())
				best = &fes;

			totalNbUsers += fes.NbUser;
		}

	}

	return best;
}

/**
 * Select a frontend in patch mode to open
 * Returns true if a new FES was open, false if no FES could be open
 */
bool	openNewFES()
{
	for (list<CFES>::iterator it=FESList.begin(); it!=FESList.end(); ++it)
	{
		if ((*it).State == PatchOnly)
		{
			nlinfo("openNewFES: ask the FS %d to accept clients", it->SId.get());

			// switch FES to AcceptClientOnly
			(*it).setToAcceptClients();
			return true;
		}
	}

	return false;
}



void displayFES ()
{
	nlinfo ("There's %d FES in the list:", FESList.size());
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		nlinfo(" > %u NbUser:%d NbPendingUser:%d", it->SId.get(), it->NbUser, it->NbPendingUsers);
	}
	nlinfo ("End of the list");
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE FRONT END SERVICE ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cbFESShardChooseShard (CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// the WS answer a user authorize
	string reason;
	CLoginCookie cookie;
	string addr;

	//
	// S09: receive "SCS" message from FES and send the "SCS" message to the LS
	//

	CMessage msgout ("SCS");

	msgin.serial (reason);
	msgout.serial (reason);

	msgin.serial (cookie);
	msgout.serial (cookie);

	if (reason.empty())
	{
		msgin.serial (addr);

		// if we set the FontEndAddress in the welcome_service.cfg we use this address
		if (FrontEndAddress.empty())
		{
			msgout.serial (addr);
		}
		else
		{
			msgout.serial (FrontEndAddress);
		}

		uint32 nbPendingUser;
		msgin.serial(nbPendingUser);

		// update the pending user count for this shard
		for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
		{
			if (it->SId == sid)
			{
				it->NbPendingUsers = nbPendingUser;
				break;
			}
		}

		/*
		// OBSOLETE: LS doesn't read patching URLs
		// build patch server list
		std::string	PatchURLS;
		for (list<CFES>::iterator it=FESList.begin(); it!=FESList.end(); ++it)
		{
			if ((*it).State == PatchOnly && !(*it).PatchAddress.empty())
			{
				if (!PatchURLS.empty())
					PatchURLS += '|';
				PatchURLS += (*it).PatchAddress;
			}
		}


		msgout.serial(PatchURLS);
		*/
	}

	if (PendingFeResponse.find(cookie) != PendingFeResponse.end())
	{
		nldebug( "ERLOG: SCS recvd from %s-%hu => sending %s to SU", serviceName.c_str(), sid.get(), cookie.toString().c_str());

		// this response is not waited by LS
		TPendingFEResponseInfo &pfri = PendingFeResponse.find(cookie)->second;

		pfri.WSMod->frontendResponse(pfri.WaiterModule, pfri.UserId, reason, cookie, addr);
		// cleanup pending record
		PendingFeResponse.erase(cookie);
	}
	else
	{
		nldebug( "ERLOG: SCS recvd from %s-%hu, but pending %s not found", serviceName.c_str(), sid.get(), cookie.toString().c_str());

		// return the result to the LS
		if (!DontUseLS)
		{
			CUnifiedNetwork::getInstance()->send ("LS", msgout);
		}
	}

}

// This function is call when a FES accepted a new client or lost a connection to a client
void cbFESClientConnected (CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	//
	// S15: receive "CC" message from FES and send "CC" message to the "LS"
	//

	CMessage msgout ("CC");

	uint32 userid;
	msgin.serial (userid);
	msgout.serial (userid);

	uint8 con;
	msgin.serial (con);
	msgout.serial (con);

	if (!DontUseLS)
	{
		CUnifiedNetwork::getInstance()->send ("LS", msgout);
	}

	// add or remove the user number really connected on this shard
	uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if (it->SId == sid)
		{
			if (con)
			{
				(*it).NbUser++;

				// the client connected, it's no longer pending
				if ((*it).NbPendingUsers > 0)
					(*it).NbPendingUsers--;
			}
			else
			{
				if ( (*it).NbUser != 0 )
					(*it).NbUser--;
			}
		}
		totalNbOnlineUsers += (*it).NbUser;
		totalNbPendingUsers += (*it).NbPendingUsers;
	}

	if (CWelcomeServiceMod::isInitialized())
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);

	if (con)
	{
		// we know that this user is on this FES
		UserIdSockAssociations.insert (make_pair (userid, sid));
	}
	else
	{
		// remove the user
		UserIdSockAssociations.erase (userid);
	}

}

// This function is called when a FES rejected a client' cookie
void	cbFESRemovedPendingCookie(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	CLoginCookie	cookie;
	msgin.serial(cookie);
	nldebug( "ERLOG: RPC recvd from %s-%hu => %s removed", serviceName.c_str(), sid.get(), cookie.toString().c_str(), cookie.toString().c_str());


	// client' cookie rejected, no longer pending
	uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			if ((*it).NbPendingUsers > 0)
				--(*it).NbPendingUsers;
		}
		totalNbOnlineUsers += (*it).NbUser;
		totalNbPendingUsers += (*it).NbPendingUsers;
	}

	if (CWelcomeServiceMod::isInitialized())
	{
		CWelcomeServiceMod::getInstance()->pendingUserLost(cookie);
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);
	}
}

// This function is called by FES to setup its PatchAddress
void	cbFESPatchAddress(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	std::string	address;
	msgin.serial(address);

	bool		acceptClients;
	msgin.serial(acceptClients);

	nldebug("Received patch server address '%s' from service %s %d", address.c_str(), serviceName.c_str(), sid.get());

	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			nldebug("Affected patch server address '%s' to frontend %s %d", address.c_str(), serviceName.c_str(), sid.get());

			if (!UsePatchMode.get() && !acceptClients)
			{
				// not in patch mode, force fs to accept clients
				acceptClients = true;
				(*it).setToAcceptClients();
			}

			(*it).PatchAddress = address;
			(*it).State = (acceptClients ? AcceptClientOnly : PatchOnly);
			if (acceptClients)
				nldebug("Frontend %s %d reported to accept client, patching unavailable for that server", address.c_str(), serviceName.c_str(), sid.get());
			else
				nldebug("Frontend %s %d reported to be in patching mode", address.c_str(), serviceName.c_str(), sid.get());

			bool	dummy;
			(*it).reportStateToLS(dummy);
			break;
		}
	}
}

// This function is called by FES to setup the right number of players (if FES was already present before WS launching)
void	cbFESNbPlayers(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// *********** WARNING *******************
	// This version of the callback is deprecated, the system
	// now use cbFESNbPlayers2 that report the pending user count
	// as well as the number of connected players.
	// It is kept for backward compatibility only.
	// ***************************************

	uint32	nbPlayers;
	msgin.serial(nbPlayers);

	uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			nldebug("Frontend '%d' reported %d online users", sid.get(), nbPlayers);
			(*it).NbUser = nbPlayers;
			if (nbPlayers != 0 && (*it).State == PatchOnly)
			{
				nlwarning("Frontend %d is in state PatchOnly, yet reports to have online %d players, state AcceptClientOnly is forced (FS_ACCEPT message sent)");
				(*it).setToAcceptClients();
			}
		}
		totalNbOnlineUsers += (*it).NbUser;
		totalNbPendingUsers += (*it).NbPendingUsers;
	}

	if (CWelcomeServiceMod::isInitialized())
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);
}


// This function is called by FES to setup the right number of players (if FES was already present before WS launching)
void	cbFESNbPlayers2(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	uint32	nbPlayers;
	uint32	nbPendingPlayers;
	msgin.serial(nbPlayers);
	msgin.serial(nbPendingPlayers);

	uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		CFES &fes = *it;
		if (fes.SId == sid)
		{
			nldebug("Frontend '%d' reported %d online users", sid.get(), nbPlayers);
			fes.NbUser = nbPlayers;
			fes.NbPendingUsers = nbPendingPlayers;
			if (nbPlayers != 0 && fes.State == PatchOnly)
			{
				nlwarning("Frontend %d is in state PatchOnly, yet reports to have online %d players, state AcceptClientOnly is forced (FS_ACCEPT message sent)");
				(*it).setToAcceptClients();
			}
		}
		totalNbOnlineUsers += fes.NbUser;
		totalNbPendingUsers += fes.NbPendingUsers;
	}

	if (CWelcomeServiceMod::isInitialized())
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);
}

/*
 * Set Shard open state
 */
void	setShardOpenState(TShardOpenState state, bool writeInVar = true)
{
	if (writeInVar)
		ShardOpen = state;

	if ( AllowDispatchMsgToLS )
	{
		if (!DontUseLS)
		{
			// send to LS current shard state
			CMessage	msgout ("SET_SHARD_OPEN");
			uint8		shardOpenState = (uint8)state;

			msgout.serial (shardOpenState);
			CUnifiedNetwork::getInstance()->send ("LS", msgout);
		}
	}
}


/*
 * Set Shard Open State
 * uint8	Open State (0 closed for all, 1 open for groups in cfg, 2 open for all)
 */
void cbSetShardOpen(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	uint8 shardOpenState;
	msgin.serial (shardOpenState);

	if (shardOpenState > OpenForAll)
	{
		shardOpenState = OpenForAll;
	}

	setShardOpenState((TShardOpenState)shardOpenState);
}

// forward declaration to callback
void	cbShardOpenStateFile(IVariable &var);

/*
 * Restore Shard Open state from config file or from file if found
 */
void cbRestoreShardOpen(CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// first restore state from config file
	CConfigFile::CVar*	var = IService::getInstance()->ConfigFile.getVarPtr("ShardOpen");
	if (var != NULL)
	{
		setShardOpenState((TShardOpenState)var->asInt());
	}

	// then restore state from state file, if it exists
	cbShardOpenStateFile(ShardOpenStateFile);
}





// a new front end connecting to me, add it
void cbFESConnection (const std::string &serviceName, TServiceId  sid, void *arg)
{
	FESList.push_back (CFES ((TServiceId)sid));
	nldebug("new FES connection: sid %u", sid.get());
	displayFES ();

	bool	dummy;
	FESList.back().reportStateToLS(dummy);

	if (!UsePatchMode.get())
	{
		FESList.back().setToAcceptClients();
	}
}


// a front end closes the connection, deconnect him
void cbFESDisconnection (const std::string &serviceName, TServiceId  sid, void *arg)
{
	nldebug("new FES disconnection: sid %u", sid.get());

	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		if ((*it).SId == sid)
		{
			// send a message to the LS to say that all players from this FES are offline
			map<uint32, TServiceId>::iterator itc = UserIdSockAssociations.begin();
			map<uint32, TServiceId>::iterator nitc = itc;
			while (itc != UserIdSockAssociations.end())
			{
				nitc++;
				if ((*itc).second == sid)
				{
					// bye bye little player
					uint32 userid = (*itc).first;
					nlinfo ("Due to a frontend crash, removed the player %d", userid);
					if (!DontUseLS)
					{
						CMessage msgout ("CC");
						msgout.serial (userid);
						uint8 con = 0;
						msgout.serial (con);
						CUnifiedNetwork::getInstance()->send ("LS", msgout);
					}
					UserIdSockAssociations.erase (itc);
				}
				itc = nitc;
			}

			bool	dummy;
			(*it).reportStateToLS(dummy, false);

			// remove the FES
			FESList.erase (it);

			break;
		}
	}

	// Update the welcome service client with the new count of connection

	uint32 totalNbOnlineUsers =0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end(); it++)
	{
		const CFES &fes = *it;
		totalNbOnlineUsers += fes.NbUser;
		totalNbPendingUsers += fes.NbPendingUsers;
	}

	if (CWelcomeServiceMod::isInitialized())
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);

	displayFES ();
}


//
void cbServiceUp (const std::string &serviceName, TServiceId  sid, void *arg)
{
	OnlineServices.addInstance( serviceName );
	bool online = OnlineServices.getOnlineStatus();
	reportOnlineStatus( online );

	// send shard id to service
	sint32 shardId;
	if (IService::getInstance()->haveArg('S'))
	{
		// use the command line param if set
		NLMISC::fromString(IService::getInstance()->getArg('S'), shardId);
	}
	else if (IService::getInstance()->ConfigFile.exists ("ShardId"))
	{
		// use the config file param if set
		shardId = IService::getInstance()->ConfigFile.getVar ("ShardId").asInt();
	}
	else
	{
		shardId = -1;
	}

	if (shardId == -1)
	{
		nlerror ("ShardId variable must be valid (>0)");
	}

	CMessage	msgout("R_SH_ID");
	msgout.serial(shardId);
	CUnifiedNetwork::getInstance()->send (sid, msgout);
}


//
void cbServiceDown (const std::string &serviceName, TServiceId  sid, void *arg)
{
	OnlineServices.removeInstance( serviceName );
	bool online = OnlineServices.getOnlineStatus();
	reportOnlineStatus( online );
}


// Callback Array for message from FES
TUnifiedCallbackItem FESCallbackArray[] =
{
	{ "SCS",				cbFESShardChooseShard },
	{ "CC",					cbFESClientConnected },
	{ "RPC",				cbFESRemovedPendingCookie },
	{ "FEPA",				cbFESPatchAddress },
	{ "NBPLAYERS",			cbFESNbPlayers },
	{ "NBPLAYERS2",			cbFESNbPlayers2 },

	{ "SET_SHARD_OPEN",		cbSetShardOpen },
	{ "RESTORE_SHARD_OPEN",	cbRestoreShardOpen },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// CONNECTION TO THE LOGIN SERVICE ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cbLSChooseShard (CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// the LS warns me that a new client want to come in my shard

	nldebug( "ERLOG: CS recvd from %s-%hu", serviceName.c_str(), sid.get());

	//
	// S07: receive the "CS" message from LS and send the "CS" message to the selected FES
	//

	CLoginCookie cookie;
	msgin.serial (cookie);
	string userName, userPriv, userExtended;
	msgin.serial (userName);

	try
	{
		msgin.serial (userPriv);
	}
	catch (const Exception &)
	{
		nlwarning ("LS didn't give me the user privilege for user '%s', set to empty", userName.c_str());
	}

	try
	{
		msgin.serial (userExtended);
	}
	catch (const Exception &)
	{
		nlwarning ("LS didn't give me the extended data for user '%s', set to empty", userName.c_str());
	}


	string ret = lsChooseShard(userName, cookie, userPriv, userExtended, WS::TUserRole::ur_player, 0xffffffff, std::numeric_limits<uint32>::max());

	if (!ret.empty())
	{
		// send back an error message to LS
		CMessage msgout ("SCS");
		msgout.serial (ret);
		msgout.serial (cookie);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
	}
}

//void cbLSChooseShard (CMessage &msgin, const std::string &serviceName, uint16 sid)
std::string lsChooseShard (const std::string &userName,
					const CLoginCookie &cookie,
					const std::string &userPriv,
					const std::string &userExtended,
					WS::TUserRole userRole,
					uint32 instanceId,
					uint32 charSlot)
{
	// the LS warns me that a new client want to come in my shard

	//
	// S07: receive the "CS" message from LS and send the "CS" message to the selected FES
	//

/*
	uint totalNbUsers;
	CFES *best = findBestFES( totalNbUsers );
	if (best == NULL)
	{
		// answer the LS that we can't accept the user
		CMessage msgout ("SCS");
		string reason = "No front-end server available";
		msgout.serial (reason);
		msgout.serial (cookie);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
		return;
	}
*/

	uint	totalNbUsers;
	CFES*	best = findBestFES( totalNbUsers );

	// could not find a good FES or best FES has more players than balance limit
	if (best == NULL || best->getUsersCountHeuristic() >= OpenFrontEndThreshold)
	{
		// open a new frontend
		openNewFES();

		// reselect best FES (will return newly open FES, or previous if no more FES available)
		best = findBestFES(totalNbUsers);

		// check there is a FES available
		if (best == NULL)
		{
			// answer the LS that we can't accept the user
			return "No front-end server available";
		}
	}


	bool	authorizeUser = false;
	bool	forceAuthorize = false;

	if (userPriv == ":DEV:")
	{
		// devs have all privileges
		authorizeUser = true;
		forceAuthorize = true;
	}
	else if (ShardOpen != ClosedForAll)
	{
		const std::string&	allowedGroups = OpenGroups;
		bool				userInOpenGroups = (!userPriv.empty() && !allowedGroups.empty() && allowedGroups.find(userPriv) != std::string::npos);

		// open for all or user is privileged
		authorizeUser = (ShardOpen == OpenForAll || userInOpenGroups);
		// let authorized users to force access even if limit is reached
		forceAuthorize = userInOpenGroups;
	}

	bool	shardLimitReached = ( (PlayerLimit.get() != -1) && (totalNbUsers >= (uint)PlayerLimit.get()) );

	if (!forceAuthorize && (!authorizeUser || shardLimitReached))
	{
		// answer the LS that we can't accept the user
		CMessage msgout ("SCS");
		string reason;
		if (shardLimitReached)
			return "The shard is currently full, please try again in 5 minutes.";
		else
			return "The shard is closed.";
	}


	CMessage msgout ("CS");
	msgout.serial (const_cast<CLoginCookie&>(cookie));
	msgout.serial (const_cast<string&>(userName), const_cast<string&>(userPriv), const_cast<string&>(userExtended));
	msgout.serial (instanceId);
	msgout.serial (charSlot);

	CUnifiedNetwork::getInstance()->send (best->SId, msgout);
	best->NbPendingUsers++;

	// Update counts
	uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
	for (list<CFES>::iterator it=FESList.begin(); it!=FESList.end(); ++it)
	{
		totalNbOnlineUsers += (*it).NbUser;
		totalNbPendingUsers += (*it).NbPendingUsers;
	}
	if (CWelcomeServiceMod::isInitialized())
		CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);

	return "";
}

void cbFailed (CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// I can't connect to the Login Service, just nlerror ();
	string reason;
	msgin.serial (reason);
	nlerror (reason.c_str());
}


bool disconnectClient(uint32 userId)
{
	map<uint32, TServiceId>::iterator it = UserIdSockAssociations.find (userId);
	if (it == UserIdSockAssociations.end ())
	{
		nlinfo ("Login service ask to disconnect user %d, he is not connected here, so ignoring", userId);
		return false;
	}
	else
	{
		CMessage msgout ("DC");
		msgout.serial (userId);
		CUnifiedNetwork::getInstance()->send (it->second, msgout);

		return true;
	}
}

void cbLSDisconnectClient (CMessage &msgin, const std::string &serviceName, TServiceId  sid)
{
	// the LS tells me that i have to disconnect a client

	uint32 userid;
	msgin.serial (userid);

	disconnectClient(userid);

}

// connection to the LS, send the identification message
void cbLSConnection (const std::string &serviceName, TServiceId  sid, void *arg)
{
	sint32 shardId;

	if (IService::getInstance()->haveArg('S'))
	{
		// use the command line param if set
		NLMISC::fromString(IService::getInstance()->getArg('S'), shardId);
	}
	else if (IService::getInstance()->ConfigFile.exists ("ShardId"))
	{
		// use the config file param if set
		shardId = IService::getInstance()->ConfigFile.getVar ("ShardId").asInt();
	}
	else
	{
		shardId = -1;
	}

	if (shardId == -1)
	{
		nlerror ("ShardId variable must be valid (>0)");
	}

	CMessage	msgout ("WS_IDENT");
	msgout.serial (shardId);
	CUnifiedNetwork::getInstance()->send (sid, msgout);

	nlinfo ("Connected to %s-%hu and sent identification with shardId '%d'", serviceName.c_str(), sid.get(), shardId);

	// send state to LS
	setShardOpenState((TShardOpenState)(ShardOpen.get()), false);

	//
	if (!DontUseLS)
	{
		CMessage	msgrpn("REPORT_NO_PATCH");
		CUnifiedNetwork::getInstance()->send("LS", msgrpn);
	}

	bool	reportPatching = false;
	list<CFES>::iterator	itfs;
	for (itfs=FESList.begin(); itfs!=FESList.end(); ++itfs)
		(*itfs).reportStateToLS(reportPatching);
}


// Callback for detection of config file change about "ExpectedServices"
void cbUpdateExpectedServices( CConfigFile::CVar& var )
{
	OnlineServices.setExpectedInstances( var );
}


/*
 * ShardOpen update functions/callbacks etc.
 */

/**
 * updateShardOpenFromFile()
 * Update ShardOpen from a file.
 * Read a line of text in the file, converts it to int (atoi), then casts into bool for ShardOpen.
 */
void	updateShardOpenFromFile(const std::string& filename)
{
	CIFile	f;

	if (!f.open(filename))
	{
		nlwarning("Failed to update ShardOpen from file '%s', couldn't open file", filename.c_str());
		return;
	}

	try
	{
		char	readBuffer[256];
		f.getline(readBuffer, 256);
		sint state;
		NLMISC::fromString(std::string(readBuffer), state);

		setShardOpenState((TShardOpenState)state);

		nlinfo("Updated ShardOpen state to '%u' from file '%s'", ShardOpen.get(), filename.c_str());
	}
	catch (const Exception& e)
	{
		nlwarning("Failed to update ShardOpen from file '%s', exception raised while getline() '%s'", filename.c_str(), e.what());
	}
}

std::string	ShardOpenStateFileName;

/**
 * cbShardOpen()
 * Callback for ShardOpen
 */
void	cbShardOpen(IVariable &var)
{
	setShardOpenState((TShardOpenState)(ShardOpen.get()), false);
}


/**
 * cbShardOpenStateFile()
 * Callback for ShardOpenStateFile
 */
void	cbShardOpenStateFile(IVariable &var)
{
	// remove previous file change callback
	if (!ShardOpenStateFileName.empty())
	{
		CFile::removeFileChangeCallback(ShardOpenStateFileName);
		nlinfo("Removed callback for ShardOpenStateFileName file '%s'", ShardOpenStateFileName.c_str());
	}

	ShardOpenStateFileName = var.toString();

	if (!ShardOpenStateFileName.empty())
	{
		// set new callback for the file
		CFile::addFileChangeCallback(ShardOpenStateFileName, updateShardOpenFromFile);
		nlinfo("Set callback for ShardOpenStateFileName file '%s'", ShardOpenStateFileName.c_str());

		// and update state from file...
		updateShardOpenFromFile(ShardOpenStateFileName);
	}
}

/**
 * cbUsePatchMode()
 * Callback for UsePatchMode
 */
void	cbUsePatchMode(IVariable &var)
{
	// if patch mode not set, set all fs in patching mode to accept clients now
	if (!UsePatchMode.get())
	{
		nlinfo("UsePatchMode disabled, switch all patching servers to actual frontends");

		list<CFES>::iterator	it;

		for (it=FESList.begin(); it!=FESList.end(); ++it)
		{
			if ((*it).State == PatchOnly)
			{
				(*it).setToAcceptClients();
			}
		}
	}
}


// Callback Array for message from LS
TUnifiedCallbackItem LSCallbackArray[] =
{
	{ "CS", cbLSChooseShard },
	{ "DC", cbLSDisconnectClient },
	{ "FAILED", cbFailed },
};

class CWelcomeService : public IService
{

public:

	/// Init the service, load the universal time.
	void init ()
	{
		string FrontendServiceName = ConfigFile.getVar ("FrontendServiceName").asString();

		try { FrontEndAddress = ConfigFile.getVar ("FrontEndAddress").asString(); } catch(const Exception &) { }

		nlinfo ("Waiting frontend services named '%s'", FrontendServiceName.c_str());

		CUnifiedNetwork::getInstance()->setServiceUpCallback(FrontendServiceName, cbFESConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback(FrontendServiceName, cbFESDisconnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbServiceUp, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback("*", cbServiceDown, NULL);

		// add a connection to the LS
		string LSAddr;
		if (haveArg('T'))
		{
			// use the command line param if set
			LSAddr = getArg('T');
		}
		else if (ConfigFile.exists ("LSHost"))
		{
			// use the config file param if set
			LSAddr = ConfigFile.getVar("LSHost").asString();
		}

		if (haveArg('S'))
		{
			// use the command line param if set
			uint shardId;
			NLMISC::fromString(IService::getInstance()->getArg('S'), shardId);

			nlinfo("Using shard id %u from command line '%s'", shardId, IService::getInstance()->getArg('S').c_str());
			anticipateShardId(shardId);
		}
		else if (ConfigFile.exists ("ShardId"))
		{
			// use the config file param if set
			uint shardId = IService::getInstance()->ConfigFile.getVar ("ShardId").asInt();

			nlinfo("Using shard id %u from config file '%s'", shardId, IService::getInstance()->ConfigFile.getVar ("ShardId").asString().c_str());
			anticipateShardId(shardId);
		}

		// the config file must have a valid address where the login service is
		nlassert(!LSAddr.empty());

		// add default port if not set by the config file
		if (LSAddr.find (":") == string::npos)
			LSAddr += ":49999";

		AllowDispatchMsgToLS = true;

		if (ConfigFile.getVarPtr("DontUseLSService") == NULL
			|| !ConfigFile.getVar("DontUseLSService").asBool())
		{
			// We are using NeL Login Service
			CUnifiedNetwork::getInstance()->addCallbackArray(LSCallbackArray, sizeof(LSCallbackArray)/sizeof(LSCallbackArray[0]));
			if (!DontUseLS)
			{
				CUnifiedNetwork::getInstance()->setServiceUpCallback("LS", cbLSConnection, NULL);
				CUnifiedNetwork::getInstance()->addService("LS", LSAddr);
			}
		}
		// List of expected service instances
		ConfigFile.setCallback( "ExpectedServices", cbUpdateExpectedServices );
		cbUpdateExpectedServices( ConfigFile.getVar( "ExpectedServices" ) );


		/*
		 * read config variable ShardOpenStateFile to update
		 *
		 */
		cbShardOpenStateFile(ShardOpenStateFile);

//		// create a welcome service module (for SU comm)
//		IModuleManager::getInstance().createModule("WelcomeService", "ws", "");
//		// plug the module in the default gateway
//		NLMISC::CCommandRegistry::getInstance().execute("ws.plug wg", InfoLog());
	}

	bool			update ()
	{
		// update the service status

		removeStatusTag("DEV_ONLY");
		removeStatusTag("RESTRICTED");
		removeStatusTag("Open");

		if (ShardOpen == 0)
			addStatusTag("DEV_ONLY");
		else if (ShardOpen == 1)
			addStatusTag("RESTRICTED");
		else if (ShardOpen == 2)
			addStatusTag("Open");

		return true;
	}

	CMirror				Mirror;
};


static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s;
	s= "welcome_service";

	if (theService->haveLongArg("wsname"))
	{
		s+= "_"+theService->getLongArg("wsname");
	}

	if (theService->haveLongArg("fullwsname"))
	{
		s= theService->getLongArg("fullwsname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s;
	s= "WS";

	if (theService->haveLongArg("shortwsname"))
	{
		s= theService->getLongArg("shortwsname");
	}

	return s.c_str();
}

// Service instantiation
NLNET_SERVICE_MAIN( CWelcomeService, getShortServiceName(scn), getCompleteServiceName(scn), 0, FESCallbackArray, "", "");


// welcome service module
//class CWelcomeServiceMod :
//	public CEmptyModuleCommBehav<CEmptyModuleServiceBehav<CEmptySocketBehav<CModuleBase> > >,
//	public WS::CWelcomeServiceSkel
//{
//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		if (CWelcomeServiceSkel::onDispatchMessage(sender, message))
//			return;
//
//		nlwarning("Unknown message '%s' received by '%s'",
//			message.getName().c_str(),
//			getModuleName().c_str());
//	}
//
//
//	////// CWelcomeServiceSkel implementation
//
//	// ask the welcome service to welcome a user
//	virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 userId, const std::string &userName, const CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
//	{
//		string ret = lsChooseShard(userName,
//			cookie,
//			priviledge,
//			exPriviledge,
//			mode,
//			instanceId);
//
//		if (!ret.empty())
//		{
//			// TODO : correct this
//			string fsAddr;
//			CWelcomeServiceClientProxy wsc(sender);
//			wsc.welcomeUserResult(this, userId, ret.empty(), fsAddr);
//		}
//	}
//
//	// ask the welcome service to disconnect a user
//	virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId)
//	{
//		nlstop;
//	}
//
//};

namespace WS
{

	void CWelcomeServiceMod::onModuleUp(IModuleProxy *proxy)
	{
		if (proxy->getModuleClassName() == "RingSessionManager")
		{
			if (_RingSessionManager != NULL)
			{
				nlwarning("WelcomeServiceMod::onModuleUp : receiving module up for RingSessionManager '%s', but already have it as '%s', replacing it",
					proxy->getModuleName().c_str(),
					_RingSessionManager->getModuleName().c_str());
			}
			// store this module as the ring session manager
			_RingSessionManager = proxy;

			// say hello to our new friend (transmit fixed session id if set in config file)
			nlinfo("Registering welcome service module into session manager '%s'", proxy->getModuleName().c_str());
			uint32 sessionId = 0;
			CConfigFile::CVar *varFixedSessionId = IService::getInstance()->ConfigFile.getVarPtr( "FixedSessionId" );
			if ( varFixedSessionId )
				sessionId = varFixedSessionId->asInt();
			CWelcomeServiceClientProxy wscp(proxy);
			wscp.registerWS(this, IService::getInstance()->getShardId(), sessionId, OnlineServices.getOnlineStatus());

			// Send counts
			uint32 totalNbOnlineUsers = 0, totalNbPendingUsers = 0;
			for (list<CFES>::iterator it=FESList.begin(); it!=FESList.end(); ++it)
			{
				totalNbOnlineUsers += (*it).NbUser;
				totalNbPendingUsers += (*it).NbPendingUsers;
			}
			CWelcomeServiceMod::getInstance()->updateConnectedPlayerCount(totalNbOnlineUsers, totalNbPendingUsers);
		}
		else if (proxy->getModuleClassName() == "LoginService")
		{
			_LoginService = proxy;
		}
	}

	void CWelcomeServiceMod::onModuleDown(IModuleProxy *proxy)
	{
		if (_RingSessionManager == proxy)
		{
			// remove this module as the ring session manager
			_RingSessionManager = NULL;
		}
		else if (_LoginService == proxy)
			_LoginService = NULL;
	}


	void CWelcomeServiceMod::welcomeUser(NLNET::IModuleProxy *sender, uint32 charId, const std::string &userName, const CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
	{
		nldebug( "ERLOG: welcomeUser(%u,%s,%s,%s,%s,%u,%u)", charId, userName.c_str(), cookie.toString().c_str(), priviledge.c_str(), exPriviledge.c_str(), (uint)mode.getValue(), instanceId );
		string ret = lsChooseShard(userName,
									cookie,
									priviledge,
									exPriviledge,
									mode,
									instanceId,
									charId & 0xF);

		uint32 userId = charId >> 4;
		if (!ret.empty())
		{
			nldebug( "ERLOG: lsChooseShard returned an error => welcomeUserResult");
			// TODO : correct this
			string fsAddr;
			CWelcomeServiceClientProxy wsc(sender);
			wsc.welcomeUserResult(this, userId, false, fsAddr, ret);
		}
		else
		{
			nldebug( "ERLOG: lsChooseShard OK => adding to pending");
			TPendingFEResponseInfo pfri;
			pfri.WSMod = this;
			pfri.UserId = userId;
			pfri.WaiterModule = sender;
			PendingFeResponse.insert(make_pair(cookie, pfri));
		}
	}

	void CWelcomeServiceMod::pendingUserLost(const NLNET::CLoginCookie &cookie)
	{
		if (!_LoginService)
			return;

		CLoginServiceProxy ls(_LoginService);

		ls.pendingUserLost(this, cookie);
	}


	// register the module
	NLNET_REGISTER_MODULE_FACTORY(CWelcomeServiceMod, "WelcomeService");

} // namespace WS


//
// Variables
//

NLMISC_DYNVARIABLE(uint32, OnlineUsersNumber, "number of connected users on this shard")
{
	// we can only read the value
	if (get)
	{
		uint32 nbusers = 0;
		for (list<CFES>::iterator it = FESList.begin(); it != FESList.end (); it++)
		{
			nbusers += (*it).NbUser;
		}
		*pointer = nbusers;
	}
}


//
// Commands
//


NLMISC_COMMAND (frontends, "displays the list of all registered front ends", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered front end :", FESList.size());
	for (list<CFES>::iterator it = FESList.begin(); it != FESList.end (); it++)
	{
//		log.displayNL ("> FE %u: nb estimated users: %u nb users: %u, nb pending users : %u",
		log.displayNL ("> FE %u: nb users: %u, nb pending users : %u",
			it->SId.get(),
			it->NbUser,
			it->NbPendingUsers);
	}
	log.displayNL ("End ot the list");

	return true;
}

NLMISC_COMMAND (users, "displays the list of all registered users", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered users :", UserIdSockAssociations.size());
	for (map<uint32, TServiceId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
	{
		log.displayNL ("> %u SId=%u", (*it).first, (*it).second.get());
	}
	log.displayNL ("End ot the list");

	return true;
}

NLMISC_COMMAND( displayOnlineServices, "Display the online service instances", "" )
{
	OnlineServices.display( log );
	return true;
}

NLMISC_VARIABLE( bool, OnlineStatus, "Main online status of the shard" );
