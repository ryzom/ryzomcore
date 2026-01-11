// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_LOGIN_SERVER_H
#define NL_LOGIN_SERVER_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>

#include "callback_server.h"
#include "login_cookie.h"

namespace NLMISC
{
	class CConfigFile;
}

namespace NLNET
{

/// Callback function type called when a new client is identified (with the login password procedure)
typedef void (*TNewClientCallback) (TSockId from, const CLoginCookie &cookie);

/// Callback function type called when a new cookie is acceptable (aka as 'a player can connect with this cookie')
typedef void (*TNewCookieCallback) (const CLoginCookie &cookie);

/// Callback function type called when a client need to be disconnected (double login...)
typedef void (*TDisconnectClientCallback) (uint32 userId, const std::string &reqServiceName);

class CUdpSock;
class IDisplayer;

/** This class is the server part of the Login System. It is used in the Front End Service.
 * At the beginning, it connects to the WS. When a new player comes in and is authenticated, a
 * callback is called to warn the user code that a new player is here.
 * Example:
 * \code
 * \endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CLoginServer {
public:

	/// Create the connection to the Welcome Service and install callbacks to the callback server (for a TCP connection)
	/// init() will try to find the ListenAddress in the config file and it will be used to say to the client
	/// the address to connect to this frontend (using the login system). You can modify this in real time in
	/// the config file or with the ls_listen_address command
	/// The ListenAddress must be in the form of "itsalive.nevrax.org:38000" (ip+port)
	static void init (CCallbackServer &server, TNewClientCallback ncl);

	/// Create the connection to the Welcome Service for an UDP connection
	/// the dc will be call when the Welcome Service decides to disconnect a player (double login...)
	static void init (CUdpSock &server, TDisconnectClientCallback dc);

	/// Create the connection to the Welcome Service for a connection
	/// the dc will be call when the Welcome Service decides to disconnect a player (double login...)
	static void init (const std::string &listenAddr, TDisconnectClientCallback dc);

	/// Add a callback to be warned when a new cookie become acceptable
	static void addNewCookieCallback(TNewCookieCallback newCookieCb);

	/// Used only in UDP, check if the cookie is valid. return empty string if valid, reason otherwise
	static std::string isValidCookie (const CLoginCookie &lc, std::string &userName, std::string &userPriv, std::string &userExtended, uint32 &instanceId, uint32 &charSlot);

	/// Call this method when a user is disconnected or the server disconnect the user.
	/// This method will warn the login system that the user is not here anymore
	static void clientDisconnected (uint32 userId);

	/// Call this method to retrieve the listen address
	static const std::string &getListenAddress();

	/// Return true if we are in 'dev' mode
	static bool acceptsInvalidCookie();

	/// Set the actual listen address
	static void setListenAddress(const std::string &la);

	/// Return the number of pending client connection.
	static uint32 getNbPendingUsers();

	/// Refresh the list of pending cookies to remove outdated one
	/// (i.e. cookies for users that never connect)
	static void refreshPendingList();

private:

	/// This function is used by init() to create the connection to the Welcome Service
	static void connectToWS ();

	// called by other init()
	static void init (const std::string &listenAddress);

};


} // NLNET

#endif // NL_LOGIN_SERVER_H

/* End of login_server.h */
