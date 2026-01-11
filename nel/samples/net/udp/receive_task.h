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

#ifndef NL_RECEIVE_TASK_H
#define NL_RECEIVE_TASK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/thread.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/mutex.h"

#include "nel/net/udp_sock.h"

#include <vector>


const uint32 MsgHeaderSize = 1+8;	// 8 for the date


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

	void				setDate()	{ *(sint64*)&(*(_Data.begin()+1)) = NLMISC::CTime::getLocalTime(); }

	sint64				getDate()	{ return *(sint64*)&(*(_Data.begin()+1)); }
	
	/// Return the event type
	TEventType			eventType() const				{ return (TEventType)(*_Data.begin()); }

	/// Return a pointer to user data for writing
	uint8				*userDataW()					{ return &*_Data.begin() + MsgHeaderSize; }

	/// Return a pointer to user data for reading
	const uint8			*userDataR() const				{ return &*_Data.begin() + MsgHeaderSize; }

	/// Return the size of user data
	uint32				userSize()						{ return (uint32)_Data.size() - MsgHeaderSize; }

	/// Return the data vector (event type header byte + user data)
	std::vector<uint8>&		data()						{ return _Data; }

private:

	/// One byte for event type (header), followed by user data
	std::vector<uint8>		_Data;

public:
	
	/// Address of sender as CInetAddress
	NLNET::CInetAddress	AddrFrom;

	/// Placeholder vector for address info
	std::vector<uint8>		VAddrFrom;
};


/**
 * receive task
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CReceiveTask : public NLMISC::IRunnable
{
public:

	/// Constructor
	CReceiveTask( uint16 port, uint32 msgsize );

	/// Destructor
	~CReceiveTask();

	/// Run
	virtual void	run();

	/// Set new write queue
	void			setWriteQueue( NLMISC::CBufFIFO *writequeue );

	/// Require exit
	void			requireExit() { _ExitRequired = true; }

private:

	/// Datagram length
	uint										_DatagramLength;

	/// Received message
	TReceivedMessage							_ReceivedMessage;

	/// Write queue access
	NLMISC::CSynchronized<NLMISC::CBufFIFO*>	_WriteQueue;

	/// Exit required
	volatile bool								_ExitRequired;

public:
	
	/// External datagram socket
	NLNET::CUdpSock								*DataSock;

};


#endif // NL_RECEIVE_TASK_H

/* End of receive_task.h */
