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



#ifndef NL_FE_RECEIVE_SUB_H
#define NL_FE_RECEIVE_SUB_H

#include "nel/misc/types_nl.h"
#include "nel/misc/buf_fifo.h"

#include "nel/net/login_cookie.h"

#include "fe_types.h"
#include "client_host.h"
#include "fe_receive_task.h"
#include "client_id_lookup.h"

#include <list>


extern bool verbosePacketLost;

#ifdef NL_RELEASE
#define LOG_PACKET_LOST ;
#else
#define LOG_PACKET_LOST if (!verbosePacketLost) {} else nldebug
#endif



class CHistory;
class CVision;
class CVisionData;


/// Type of remove list
typedef std::list< std::pair<TClientId,uint8> > TClientsToRemove;

/// Types of message invalidity (used for THackingDesc::Reasons bitfield)
enum TBadMessageFormatType { InsufficientSize=1, NotSystemLoginCode=2, BadCookie=4, BadSystemCode=8, HackedSizeInBuffer=16, AccessClosed=32, IrrelevantSystemMessage=64, MalformedAction=128, UnknownExceptionType=256, UnknownFormatType=512, UnauthorizedCharacterSlot=1024 };

/// Return the string for the message invalidity reasons
std::string getBadMessageString( uint32 reasons );

/// Hacking description
struct THackingDesc
{
	THackingDesc() : InvalidMsgCounter(1), UserId(0), Reasons(0) {}	

	/// Number of invalid messages received
	uint32	InvalidMsgCounter;

	/// UserId if known
	TUid	UserId;

	/// Reason(s) of the invalidities (bitfield: see TBadMessageFormatType)
	uint32	Reasons;
};

/// Type of address set with counter
typedef CHashMap<NLNET::CInetAddress,THackingDesc,CInetAddressHashMapTraits> THackingAddrSet;

/// Type of map of ip -> user id
typedef std::map<uint32,TUid> TAutoUidMap;


/**
 * Front-end Receive Subsystem
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CFeReceiveSub
{
public:

	/// Constructor
	CFeReceiveSub() :
		EntityToClient(),
		_ReceiveTask(NULL),
		_ReceiveThread(NULL),
		_ClientMap(),
		_ClientIdCont(NULL),
		_Queue1(),
		_Queue2(),
		_CurrentReadQueue(NULL),
		_CurrentInMsg(),
		_RcvCounter(0),
		_RcvBytes(0),
		_PrevRcvBytes(0),
		_ClientIdPool(),
		_History( NULL ),
		_RemovedClientEntities(),
		_AccessOpen(true),
		ConnectionStatLog(NULL),
		_ConnectionStatDisp(NULL)
		{}

	/// Init
	void				init( uint16 firstAcceptableFrontendPort, uint16 lastAcceptableFrontendPort, uint32 dgrammaxlength, CHistory *history, TClientIdCont *clientidcont );

	/// Update
	void				update();

	/// Release
	void				release();

	/// Add client
	CClientHost			*addClient( const NLNET::CInetAddress& addrfrom, TUid userId, const std::string &userName, const std::string &userPriv, const std::string & userExtended, const std::string & languageId, const NLNET::CLoginCookie &cookie, uint32 instanceId, uint8 authorisedCharSlot, bool sendCLConnect=true );

	/// Add to the list of clients which will be removed by addr at the three cycles later (leaving the time to send an impulsion to the client)
	void				addToRemoveList( TClientId clientid ) { _ClientsToRemove.push_back( std::make_pair(clientid,3) ); }

	/// Remove from the list of clients
	void				removeFromRemoveList( TClientId clientid );

	/// Return the number of connecged client
	uint32				getNbClient();

	// Remove a client by iterator on THostMap (see also byId)
	//void				removeClientByAddr( THostMap::iterator iclient );

	void				setClientInLimboMode( CClientHost *client );

	CClientHost			*exitFromLimboMode( const CLimboClient& client );
	
	void				removeClientFromMap( CClientHost *client );
	
	/// Remove a client by clientid
	/// all remove are supposed to be a client crashed. it s false only on the case we receive really the client message
	void				removeClientById( TClientId clientid, bool crashed = true );

	/// After a client has been removed from the tables, free its identifier
	void				freeIdsOfRemovedClients();

	THostMap&			clientMap()					{ return _ClientMap; }
	TClientIdCont&		clientIdCont()				{ return *_ClientIdCont; }
	CClientHost*		getClientHost(TClientId id)	{ return (*_ClientIdCont)[id]; }
	NLNET::CUdpSock		*dataSock()					{ return _ReceiveTask->DataSock; }


	// Swap receive queues (to avoid high contention between the receive thread and the reading thread)
	void				swapReadQueues();

	// Read incoming data from the current read queue
	void				readIncomingData();

	/// Open or close the access for new clients
	void				openAccess( bool b ) { _AccessOpen = b; }

	/// Return the current access state
	bool				accessOpen() const { return _AccessOpen; }

	/// Find a client host by Uid (slow)
	CClientHost			*findClientHostByUid( TUid uid, bool exitFromLimbo=false );

	/// Remove client in tables
	void				removeClientLinks( CClientHost *clienthost );

	/// Delete a client object
	void				deleteClient( CClientHost *clienthost );

	///
	bool				acceptUserIdConnection( TUid userId );
	
	/// Do not accept the current message (hacking detection)
	void				rejectReceivedMessage( TBadMessageFormatType bmft, TUid userId=0 );

	// Entity to client
	CClientIdLookup		EntityToClient;

	NLMISC::CLog		*ConnectionStatLog;

	std::map<TUid,CLimboClient>	LimboClients;

protected:

	/** Process current incoming message (handles client addition/removal/identification,
	 * then calls handleReceivedMsg() if not removal)
	 */
	void	handleIncomingMsg();

	/// Process current received message (called by handleIncomingMsg())
	void	handleReceivedMsg( CClientHost *clienthost );

	/// Compute stats about received datagrams
	void	computeStats( CClientHost *clienthost, uint32 currentcounter, uint32 currentsize, bool updateAcknowledge );

	/// Display lost statistics
	void	displayDatagramStats( NLMISC::TTime lastdisplay );

	/// Utility method to remove a client
	void	doRemoveClient( CClientHost *client, bool crashed );

private:

	/// Receive socket from the clients
	CFEReceiveTask		*_ReceiveTask;

	/// Receive thread
	NLMISC::IThread		*_ReceiveThread;

	/// Client map by address
	THostMap			_ClientMap;

	/// Client map by id (belonging to the send subsystem)
	TClientIdCont		*_ClientIdCont;

	/// First queue
	NLMISC::CBufFIFO	_Queue1;

	/// Second queue
	NLMISC::CBufFIFO	_Queue2;

	/// Current read queue
	NLMISC::CBufFIFO	*_CurrentReadQueue;

	/// Current incoming message
	TReceivedMessage	*_CurrentInMsg;

	/// Number of messages received (stat)
	volatile uint32		_RcvCounter;

	/// Receive bytes in UDP (stat)
	volatile uint32		_RcvBytes;

	/// Previous RcvBytes (stat)
	volatile uint32		_PrevRcvBytes;

	/// Client Id pool
	CClientIdPool		_ClientIdPool;

	/// Packet History access
	CHistory			*_History;

	/// Clients to remove (iterator and game cycle left to wait before removing) (used to temporize removal when an impulsion needs to be sent before removal)
	TClientsToRemove			_ClientsToRemove;

	/// Clients to free (not a TEntityIndex because they are not set for client who do not see anyone)
	std::vector<CClientHost*>	_RemovedClientEntities;

	/// Set of addresses of unidentified clients
	THackingAddrSet				_UnidentifiedFlyingClients;

	/// Do clients have the right to connect? (does not affect clients already connected)
	bool						_AccessOpen;

	NLMISC::CFileDisplayer		*_ConnectionStatDisp;

	/// Persistent map to auto-allocate a dev user id
	TAutoUidMap					_AutoUidMap;
};


#endif // NL_FE_RECEIVE_SUB_H

/* End of fe_receive_sub.h */
