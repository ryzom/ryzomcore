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

#include "stdnet.h"

#include "nel/net/buf_net_base.h"

using namespace NLMISC;
using namespace std;


namespace NLNET {

uint32 	NbNetworkTask = 0;

// The value that will be used if setMaxExpectedBlockSize() is not called (or called with a negative argument)
uint32 CBufNetBase::DefaultMaxExpectedBlockSize = 1048576; // 10 M unless changed by a NeL variable

// The value that will be used if setMaxSentBlockSize() is not called (or called with a negative argument)
uint32 CBufNetBase::DefaultMaxSentBlockSize = 1048576; // 10 M unless changed by a NeL variable


/***************************************************************************************************
 * User main thread
 **************************************************************************************************/


/*
 * Constructor
 */
#ifdef NL_OS_UNIX
CBufNetBase::CBufNetBase( bool isDataAvailablePipeSelfManaged ) :
#else
CBufNetBase::CBufNetBase() :
#endif
	_RecvFifo("CBufNetBase::_RecvFifo"),
	_DisconnectionCallback( NULL ),
	_DisconnectionCbArg( NULL ),
	_MaxExpectedBlockSize( DefaultMaxExpectedBlockSize ),
	_MaxSentBlockSize( DefaultMaxSentBlockSize ),
	_DataAvailable( false )
{
	// Debug info for mutexes
#ifdef MUTEX_DEBUG
	initAcquireTimeMap();
#endif
#ifdef NL_OS_UNIX
	_IsDataAvailablePipeSelfManaged = isDataAvailablePipeSelfManaged;
	if ( _IsDataAvailablePipeSelfManaged )
	{
		if ( ::pipe( _DataAvailablePipeHandle ) != 0 )
			nlwarning( "Unable to create D.A. pipe" );
	}
#endif
}


/*
 * Destructor
 */
CBufNetBase::~CBufNetBase()
{
#ifdef NL_OS_UNIX
	if ( _IsDataAvailablePipeSelfManaged )
	{
		::close( _DataAvailablePipeHandle[PipeRead] );
		::close( _DataAvailablePipeHandle[PipeWrite] );
	}
#endif
}


/*
 * Push message into receive queue (mutexed)
 * TODO OPTIM never use this function
 */
void	CBufNetBase::pushMessageIntoReceiveQueue( const std::vector<uint8>& buffer )
{
	//sint32 mbsize;
	{
		//nldebug( "BNB: Acquiring the receive queue... ");
		CFifoAccessor recvfifo( &_RecvFifo );
		//nldebug( "BNB: Acquired, pushing the received buffer... ");
		recvfifo.value().push( buffer );
		//nldebug( "BNB: Pushed, releasing the receive queue..." );
		//mbsize = recvfifo.value().size() / 1048576;
		setDataAvailableFlag( true );
	}
#ifdef NL_OS_UNIX
	// Wake-up main thread (outside the critical section of CFifoAccessor, to allow main thread to be
	// read the fifo; if the main thread sees the Data Available flag is true but the pipe not written
	// yet, it will block on read()).
	uint8 b=0;
	if ( write( _DataAvailablePipeHandle[PipeWrite], &b, 1 ) == -1 )
	{
		nlwarning( "LNETL1: Write pipe failed in pushMessageIntoReceiveQueue" );
	}
	//nldebug( "Pipe: 1 byte written (%p)", this );
#endif
	//nldebug( "BNB: Released." );
	//if ( mbsize > 1 )
	//{
	//	nlwarning( "The receive queue size exceeds %d MB", mbsize );
	//}
}

/*
 * Push message into receive queue (mutexed)
 */
void	CBufNetBase::pushMessageIntoReceiveQueue( const uint8 *buffer, uint32 size )
{
	//sint32 mbsize;
	{
		//nldebug( "BNB: Acquiring the receive queue... ");
		CFifoAccessor recvfifo( &_RecvFifo );
		//nldebug( "BNB: Acquired, pushing the received buffer... ");
		recvfifo.value().push( buffer, size );
		//nldebug( "BNB: Pushed, releasing the receive queue..." );
		//mbsize = recvfifo.value().size() / 1048576;
		setDataAvailableFlag( true );
#ifdef NL_OS_UNIX
		// Wake-up main thread
		uint8 b=0;
		if ( write( _DataAvailablePipeHandle[PipeWrite], &b, 1 ) == -1 )
		{
			nlwarning( "LNETL1: Write pipe failed in pushMessageIntoReceiveQueue" );
		}
		nldebug( "Pipe: 1 byte written" );
#endif
	}
	//nldebug( "BNB: Released." );
	/*if ( mbsize > 1 )
	{
		nlwarning( "The receive queue size exceeds %d MB", mbsize );
	}*/
}



NLMISC_CATEGORISED_VARIABLE(nel, uint32, NbNetworkTask, "Number of server and client thread");

} // NLNET
