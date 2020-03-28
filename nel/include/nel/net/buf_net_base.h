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

#ifndef NL_BUF_NET_BASE_H
#define NL_BUF_NET_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mutex.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/thread.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

namespace NLNET {


class CBufSock;

/// Socket identifier
typedef CBufSock *TSockId;

static const TSockId InvalidSockId = (TSockId) NULL;

/// Callback function for message processing
typedef void (*TNetCallback) ( TSockId from, void *arg );

/// Storing a TNetCallback call for future call
typedef std::pair<TNetCallback,TSockId> TStoredNetCallback;

/// Synchronized FIFO buffer
typedef NLMISC::CSynchronized<NLMISC::CBufFIFO> CSynchronizedFIFO;

/// Accessor of mutexed FIFO buffer
typedef CSynchronizedFIFO::CAccessor CFifoAccessor;

/// Size of a block
typedef uint32 TBlockSize;

extern uint32 	NbNetworkTask;

#ifdef NL_OS_UNIX
/// Access to the wake-up pipe (Unix only)
enum TPipeWay { PipeRead, PipeWrite };
#endif


/**
 * Layer 1
 *
 * Base class for CBufClient and CBufServer.
 * The max block sizes for sending and receiving are controlled by setMaxSentBlockSize()
 * and setMaxExpectedBlockSize(). Their default value is the maximum number contained in a sint32,
 * that is 2^31-1 (i.e. 0x7FFFFFFF). The limit for sending is checked only in debug mode.
 *
 * \author Nevrax France
 * \date 2001
 */
class CBufNetBase
{
public:

	/// Type of incoming events (max 256)
	enum TEventType { User = 'U', Connection = 'C', Disconnection = 'D' };

	/// Destructor
	virtual ~CBufNetBase();

#ifdef NL_OS_UNIX
	/** Init the pipe for data available with an external pipe.
	 * Call it only if you set initPipeForDataAvailable to false in the constructor.
	 * Then don't call sleepUntilDataAvailable() but use select() on the pipe.
	 * The pipe will be written one byte when receiving a message.
	 */
	void	setExternalPipeForDataAvailable( int *twoPipeHandles )
	{
		_DataAvailablePipeHandle[PipeRead] = twoPipeHandles[PipeRead];
		_DataAvailablePipeHandle[PipeWrite] = twoPipeHandles[PipeWrite];
	}
#endif

	/// Sets callback for detecting a disconnection (or NULL to disable callback)
	void	setDisconnectionCallback( TNetCallback cb, void* arg ) { _DisconnectionCallback = cb; _DisconnectionCbArg = arg; }

	/// Returns the size of the receive queue (mutexed)
	uint32	getReceiveQueueSize()
	{
		CFifoAccessor syncfifo( &_RecvFifo );
		return syncfifo.value().size();
	}

	void displayReceiveQueueStat (NLMISC::CLog *log = NLMISC::InfoLog)
	{
		CFifoAccessor syncfifo( &_RecvFifo );
		syncfifo.value().displayStats(log);
	}

	/**
	 * Sets the max size of the received messages.
	 * If receiving a message bigger than the limit, the connection will be dropped.
	 *
	 * Default value: CBufNetBase::DefaultMaxExpectedBlockSize
	 * If you put a negative number as limit, the max size is reseted to the default value.
	 * Warning: you can call this method only at initialization time, before connecting (for a client)
	 * or calling init() (for a server) !
	 */
	void	setMaxExpectedBlockSize( sint32 limit )
	{
		if ( limit < 0 )
			_MaxExpectedBlockSize = DefaultMaxExpectedBlockSize;
		else
			_MaxExpectedBlockSize = (uint32)limit;
	}

	/**
	 * Sets the max size of the sent messages.
	 * Any bigger sent block will produce an assertion failure, currently.
	 *
	 * Default value: CBufNetBase::DefaultMaxSentBlockSize
	 * If you put a negative number as limit, the max size is reseted to the default value.
	 * Warning: you can call this method only at initialization time, before connecting (for a client)
	 * or calling init() (for a server) !
	 */
	void	setMaxSentBlockSize( sint32 limit )
	{
		if ( limit < 0 )
			_MaxSentBlockSize = DefaultMaxSentBlockSize;
		else
			_MaxSentBlockSize = (uint32)limit;
	}

	/// Returns the max size of the received messages (default: 2^31-1)
	uint32	maxExpectedBlockSize() const
	{
		return _MaxExpectedBlockSize;
	}

	/// Returns the max size of the sent messages (default: 2^31-1)
	uint32	maxSentBlockSize() const
	{
		return _MaxSentBlockSize;
	}

#ifdef NL_OS_UNIX
	/**
	 * Return the handle for reading the 'data available pipe'. Use it if you want to do a select on
	 * multiple CBufNetClient/CBufNetServer objects (then, don't call sleepUntilDataAvailable() on them).
	 */
	int		dataAvailablePipeReadHandle() const { return _DataAvailablePipeHandle[PipeRead]; }
#endif

	/// The value that will be used if setMaxExpectedBlockSize() is not called (or called with a negative argument)
	static uint32 DefaultMaxExpectedBlockSize;

	/// The value that will be used if setMaxSentBlockSize() is not called (or called with a negative argument)
	static uint32 DefaultMaxSentBlockSize;

protected:

	friend class NLNET::CBufSock;

#ifdef NL_OS_UNIX
	/// Constructor
	CBufNetBase( bool isDataAvailablePipeSelfManaged );
#else
	/// Constructor
	CBufNetBase();
#endif

	/// Access to the receive queue
	CSynchronizedFIFO&	receiveQueue() { return _RecvFifo; }

	/// Returns the disconnection callback
	TNetCallback		disconnectionCallback() const { return _DisconnectionCallback; }

	/// Returns the argument of the disconnection callback
	void*				argOfDisconnectionCallback() const { return _DisconnectionCbArg; }

	/// Push message into receive queue (mutexed)
	// TODO OPTIM never use this function
	void				pushMessageIntoReceiveQueue( const std::vector<uint8>& buffer );

	/// Push message into receive queue (mutexed)
	void				pushMessageIntoReceiveQueue( const uint8 *buffer, uint32 size );

	/// Sets _DataAvailable
	void				setDataAvailableFlag( bool da ) { _DataAvailable = da; }

	/// Return _DataAvailable
	bool				dataAvailableFlag() const { return _DataAvailable; }

#ifdef NL_OS_UNIX
	/// Pipe to select() on data available
	int					_DataAvailablePipeHandle [2];
#endif

private:

	/// The receive queue, protected by a mutex-like device
	CSynchronizedFIFO	_RecvFifo;

	/// Callback for disconnection
	TNetCallback		_DisconnectionCallback;

	/// Argument of the disconnection callback
	void*				_DisconnectionCbArg;

	/// Max size of received messages (limited by the user)
	uint32				_MaxExpectedBlockSize;

	/// Max size of sent messages (limited by the user)
	uint32				_MaxSentBlockSize;

	/// True if there is data available (avoids locking a mutex)
	volatile bool		_DataAvailable;

#ifdef NL_OS_UNIX
	bool _IsDataAvailablePipeSelfManaged;
#endif
};


} // NLNET


#endif // NL_BUF_NET_BASE_H

/* End of buf_net_base.h */
