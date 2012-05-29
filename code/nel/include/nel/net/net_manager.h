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




/**************************************************************************
********************* THIS CLASS IS DEPRECATED ****************************
**************************************************************************/

#ifdef NL_OS_WINDOWS
#	pragma message(NL_LOC_WRN "You are using a deprecated feature of NeL, consider rewriting your code with replacement feature")
#else // NL_OS_UNIX
#	warning "You are using a deprecated feature of NeL, consider rewriting your code with replacement feature"
#endif

#ifndef NL_NET_MANAGER_H
#define NL_NET_MANAGER_H

#include "nel/misc/types_nl.h"

#include <string>
#include <map>
#include <vector>

#include "nel/misc/time_nl.h"
#include "nel/misc/string_id_array.h"

#include "callback_net_base.h"
#include "naming_client.h"

namespace NLNET {

/// Callback function type for message processing
typedef void (*TNetManagerCallback) (const std::string &serviceName, TSockId from, void *arg);

/** Structure used in the second part of the map
 * If you add a client with his service name, the Name is the service name and ServiceNames is empty.
 * If you add a client with his ip, Name is the fake service name and ServiceNames[0] is the ip.
 * If you add a group (of client), Name is the name of the group and ServiceNames is names of all services in the group.
 * If you add a server, Name is the service name of the server and ServiceNames is empty.
 */
struct CBaseStruct
{
	CBaseStruct (const std::string &sn) :
		Name(sn), ConnectionCallback(NULL), ConnectionCbArg(NULL),
		DisconnectionCallback(NULL), DisconnectionCbArg(NULL), Type(Unknown)
	{ }

	/// the name used by all function to retrieve a service (in the case of group or ip, this name is a virtual
	/// name used only to find it to perform action on it
	std::string	Name;

	enum TBaseStructType { Unknown, Client, ClientWithAddr, Group, Server };

	std::vector<std::string>		ServiceNames;

	/// It could have more than one connection, in this case, the vector contains all connections
	std::vector<CCallbackNetBase*>	 NetBase;

	TNetManagerCallback				 ConnectionCallback;
	void							*ConnectionCbArg;

	TNetManagerCallback				 DisconnectionCallback;
	void							*DisconnectionCbArg;

	// autoretry is used only when Type is ClientWithAddr. If true, the CNetManager will retry to reconnect if it lost the connection
	bool							 AutoRetry;

	TBaseStructType					 Type;
};

/**
 * Layer 4
 *
 * In case of addGroup(), messages are *not* associate with id, so the message type is always sent with string.
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CNetManager
{
public:

	/** Creates the connection to the Naming Service.
	 * If the connection failed, ESocketConnectionFailed exception is generated.
	 */
	static void	init (const CInetAddress *addr, CCallbackNetBase::TRecordingState rec );

	static void release ();

	/** Sets up a server on a specific port with a specific service name (create a listen socket, register to naming service and so on)
	 * If servicePort is 0, it will be dynamically determinated by the Naming Service.
	 * If sid id 0, the service id will be dynamically determinated by the Naming Service.
	 */
	static void	addServer (const std::string &serviceName, uint16 servicePort = 0, bool external = false);

	static void	addServer (const std::string &serviceName, uint16 servicePort, NLNET::TServiceId &sid, bool external = false);

	/// Creates a connection to a specific IP and associate it this a "fake" serviceName (to enable you to send data for example)
	static void	addClient (const std::string &serviceName, const std::string &addr, bool autoRetry = true);

	/// Creates a connection to a service using the naming service and the serviceName
	static void	addClient (const std::string &serviceName);

	/// Creates connections to a group of service
	static void	addGroup (const std::string &groupName, const std::string &serviceName);

	/// Adds a callback array to a specific service connection. You can add callback only *after* adding the server, the client or the group
	static void	addCallbackArray (const std::string &serviceName, const TCallbackItem *callbackarray, NLMISC::CStringIdArray::TStringId arraysize);

	/** Call it evenly. the parameter select the timeout value in milliseconds for each update. You are absolutely certain that this
	 * function will not be returns before this amount of time you set.
	 * If you set the update timeout value higher than 0, all messages in queues will be process until the time is greater than the timeout user update().
	 * If you set the update timeout value to 0, all messages in queues will be process one time before calling the user update(). In this case, we don't nlSleep(1).
	 */
	static void	update (NLMISC::TTime timeout = 0);

	/// Sends a message to a specific serviceName
	static void	send (const std::string &serviceName, const CMessage &buffer, TSockId hostid = InvalidSockId);

	/** Sets callback for incoming connections (or NULL to disable callback)
	 * On a client, the callback will be call when the connection to the server is established (the first connection or after the server shutdown and started)
	 * On a server, the callback is called each time a new client is connected to him
	 */
	static void	setConnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg);

	/** Sets callback for disconnections (or NULL to disable callback)
		On a client, the callback will be call each time the connection to the server is lost
		On a server, the callback is called each time a client is disconnected
	 */
	static void	setDisconnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg);

	/// Returns the connection if you want to do specific calls
	static CCallbackNetBase *getNetBase (const std::string &serviceName);

	static void setUpdateTimeout (uint32 timeout);

	static void createConnection(CBaseStruct &Base, const CInetAddress &Addr, const std::string& name);

	static uint64 getBytesSent ();
	static uint64 getBytesReceived ();

	static uint64 getReceiveQueueSize ();
	static uint64 getSendQueueSize ();

private:

	typedef	std::map<std::string, CBaseStruct>	TBaseMap;
	typedef	TBaseMap::iterator					ItBaseMap;

	// Contains all the connections (client and server)
	static	TBaseMap	_BaseMap;

	static	CCallbackNetBase::TRecordingState _RecordingState;

	// used to synchonize the timeout in the update function
	static	NLMISC::TTime _NextUpdateTime;

	// Finds the service or add it if not found
	static	ItBaseMap find (const std::string &serviceName);

	friend	void RegistrationBroadcast (const std::string &name, TServiceId sid, const std::vector<CInetAddress> &addr);


	// It's a static class, you can't instanciate it
	CNetManager() { }
};


} // NLNET


#endif // NL_NET_MANAGER_H

/* End of net_manager.h */
