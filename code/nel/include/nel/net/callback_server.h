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

#ifndef NL_CALLBACK_SERVER_H
#define NL_CALLBACK_SERVER_H

#include "nel/misc/types_nl.h"

#include "callback_net_base.h"
#include "buf_server.h"


namespace NLNET {


/**
 * Server class for layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackServer : public CCallbackNetBase, public CBufServer
{
public:

	/// Constructor
	CCallbackServer( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true, bool initPipeForDataAvailable=true );

	/// Sends a message to the specified host
	void	send (const CMessage &buffer, TSockId hostid, bool log = true);

	/// Force to send all data pending in the send queue. See comment in CCallbackNetBase.
	bool	flush (TSockId destid, uint *nbBytesRemaining=NULL) { nlassert( destid != InvalidSockId ); return CBufServer::flush(destid, nbBytesRemaining); }

	/** Updates the network (call this method evenly).
	 * More info about timeout and mintime in the code of CCallbackNetBase::baseUpdate().
	 */
	void	update2 (sint32 timeout=-1, sint32 mintime=0);

	/// Updates the network (call this method evenly) (legacy)
	void	update (sint32 timeout=0);

	/// Sets callback for incoming connections (or NULL to disable callback)
	void	setConnectionCallback (TNetCallback cb, void *arg) { _ConnectionCallback = cb; _ConnectionCbArg = arg; }

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	/// Returns true if the connection is still connected. on server, we always "connected"
	bool	connected () const { return true; }

	/** Disconnect a connection
	 * Set hostid to InvalidSockId to disconnect all connections.
	 * If hostid is not InvalidSockId and the socket is not connected, the method does nothing.
	 * Before disconnecting, any pending data is actually sent.
	 */
	void	disconnect (TSockId hostid);

	/// Returns the address of the specified host
	const CInetAddress& hostAddress (TSockId hostid) { nlassert(hostid!=InvalidSockId); return CBufServer::hostAddress (hostid); }

	/// Returns the sockid (cf. CCallbackClient)
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId);

	uint64	getReceiveQueueSize () { return CBufServer::getReceiveQueueSize(); }
	uint64	getSendQueueSize () { return CBufServer::getSendQueueSize(0); }

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufServer::displayReceiveQueueStat(log); }
	void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId) { CBufServer::displaySendQueueStat(log, destid); }

	void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufServer::displayThreadStat(log); }

private:

	/// This function is public in the base class and put it private here because user cannot use it in layer 2
	void			send (const NLMISC::CMemStream &/* buffer */, TSockId /* hostid */) { nlstop; }

	bool			dataAvailable ();
	virtual bool	getDataAvailableFlagV() const { return dataAvailableFlag(); }

	void			receive (CMessage &buffer, TSockId *hostid);

	void			sendAllMyAssociations (TSockId to);

	TNetCallback	_ConnectionCallback;
	void			*_ConnectionCbArg;

	friend void		cbsNewConnection (TSockId from, void *data);

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	void						noticeConnection( TSockId hostid );
	virtual						bool replaySystemCallbacks();
	std::vector<CBufSock*>		_MR_Connections;
	std::map<TSockId,TSockId>	_MR_SockIds; // first=sockid in file; second=CBufSock*
#endif
	// ---------------------------------------

};


} // NLNET


#endif // NL_CALLBACK_SERVER_H

/* End of callback_server.h */
