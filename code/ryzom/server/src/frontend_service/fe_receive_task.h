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



#ifndef NL_FE_RECEIVE_TASK_H
#define NL_FE_RECEIVE_TASK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/thread.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/mutex.h"

//#define MEASURE_RECEIVE_TASK

#include "nel/net/udp_sock.h"

#include <vector>


const uint32 MsgHeaderSize = 1;


/**
 * Placeholder for received messages
 */
struct TReceivedMessage
{
	/// Type of incoming events (see also NLNET::CBufNetBase::TEventType)
	enum TEventType { User = 'U', RemoveClient = 'D' };

	/// Constructor
	TReceivedMessage();

	/// Resize data
	void				resizeData( uint32 datasize )	{ _Data.resize( MsgHeaderSize + datasize ); }

	/// Return a vector containing the address info
	void				addressToVector();

	/// Set address with address info from specified vector
	void				vectorToAddress();

	/// Set "disconnection" message for the current AddrFrom
	void				setTypeEvent( TEventType t )	{ *_Data.begin() = (uint8)t; }

	/// Return the event type
	TEventType			eventType() const				{ return (TEventType)(*_Data.begin()); }

	/// Return a pointer to user data for writing
	uint8				*userDataW()					{ return &*_Data.begin() + MsgHeaderSize; }

	/// Return a pointer to user data for reading
	const uint8			*userDataR() const				{ return &*_Data.begin() + MsgHeaderSize; }

	/// Return the size of user data
	uint32				userSize()						{ return (uint32)_Data.size() - MsgHeaderSize; }

	/// Return the data vector (event type header byte + user data)
	std::vector<uint8>&	data()							{ return _Data; }

private:

	/// One byte for event type (header), followed by user data
	std::vector<uint8>	_Data;

public:
	
	/// Address of sender as CInetAddress
	NLNET::CInetAddress	AddrFrom;

	/// Placeholder vector for address info
	std::vector<uint8>	VAddrFrom;
};


/**
 * Front-end receive task
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CFEReceiveTask : public NLMISC::IRunnable
{
public:

	/// Constructor
	CFEReceiveTask( uint16 firstAcceptablePort, uint16 lastAcceptablePort, uint32 msgsize );

	/// Destructor
	~CFEReceiveTask();

	/// Run
	virtual void	run();

	/// Set new write queue (thread-safe because mutexed)
	void			setWriteQueue( NLMISC::CBufFIFO *writequeue );

	/// Require exit (thread-safe because atomic assignment)
	void			requireExit() { _ExitRequired = true; }

	/// Return the number of rejected datagrams since the last call (thread-safe because atomic assignment)
	uint			nbNewRejectedDatagrams()	{ uint nb=_NbRejectedDatagrams; _NbRejectedDatagrams=0; return nb; }

private:

	/// Datagram length
	uint										_DatagramLength;

	/// Received message
	TReceivedMessage							_ReceivedMessage;

	/// Write queue access
	NLMISC::CSynchronized<NLMISC::CBufFIFO*>	_WriteQueue;

	/// Number of datagrams not copied because too big
	volatile uint								_NbRejectedDatagrams;

	/// Exit required
	volatile bool								_ExitRequired;

public:
	
	/// External datagram socket
	NLNET::CUdpSock								*DataSock;

	/// The date of the last UPD packet recevied
	static volatile	uint32						LastUDPPacketReceived;

};


#endif // NL_FE_RECEIVE_TASK_H

/* End of fe_receive_task.h */
