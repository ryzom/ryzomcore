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

#ifndef NL_CALLBACK_CLIENT_H
#define NL_CALLBACK_CLIENT_H

#include "nel/misc/types_nl.h"

#include "callback_net_base.h"
#include "buf_client.h"

namespace NLNET {


class CInetAddress;


/**
 * Client class for layer 3
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CCallbackClient : public CCallbackNetBase, public CBufClient
{
public:

	/// Constructor
	CCallbackClient( TRecordingState rec=Off, const std::string& recfilename="", bool recordall=true, bool initPipeForDataAvailable=true );
	~CCallbackClient();

	/// Sends a message to the remote host (the second parameter isn't used)
	void	send (const CMessage &buffer, TSockId hostid = InvalidSockId, bool log = true);

	/// Force to send all data pending in the send queue. hostid must be InvalidSockId here. See comment in CCallbackNetBase.
	bool	flush (TSockId hostid = InvalidSockId, uint *nbBytesRemaining=NULL);

	/** Updates the network (call this method evenly).
	 * More info about timeout and mintime in the code of CCallbackNetBase::baseUpdate().
	 */
	void	update2 (sint32 timeout=-1, sint32 mintime=0);

	/// Updates the network (call this method evenly) (legacy)
	void	update (sint32 timeout=0);

	/// Connects to the specified host
	void	connect( const CInetAddress& addr );

	/** Returns true if the connection is still connected (changed when a disconnection
	 * event has reached the front of the receive queue, just before calling the disconnection callback
	 * if there is one)
	 */
	virtual bool	connected () const { return CBufClient::connected (); }

	virtual const CInetAddress&	hostAddress( TSockId /* hostid */ ) { return remoteAddress(); }

	/** Disconnect a connection
	 * Unlike in CCallbackClient, you can call disconnect() on a socket that is already disconnected
	 * (it will do nothing)
	 */
	void	disconnect (TSockId hostid = InvalidSockId);

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	/// Returns the sockid
	virtual TSockId	getSockId (TSockId hostid = InvalidSockId);

	uint64	getReceiveQueueSize () { return CBufClient::getReceiveQueueSize(); }
	uint64	getSendQueueSize () { return CBufClient::getSendQueueSize(); }

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufClient::displayReceiveQueueStat(log); }
	void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog, TSockId /* destid */ = InvalidSockId) { CBufClient::displaySendQueueStat(log); }

	void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog) { CBufClient::displayThreadStat(log); }

private:

	/// These function is public in the base class and put it private here because user cannot use it in layer 2
	void	send (const NLMISC::CMemStream &/* buffer */) { nlstop; }

	/// Returns true if there are messages to read
	bool	dataAvailable ();
	virtual bool getDataAvailableFlagV() const { return dataAvailableFlag(); }

	void	receive (CMessage &buffer, TSockId *hostid = NULL);

	// ---------------------------------------
#ifdef USE_MESSAGE_RECORDER
	virtual bool replaySystemCallbacks();
#endif

	bool LockDeletion;
};


} // NLNET


#endif // NL_CALLBACK_CLIENT_H

/* End of callback_client.h */
