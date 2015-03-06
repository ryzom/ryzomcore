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

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/variable.h"
#include "nel/net/buf_sock.h"
#include "nel/net/buf_server.h"
#include "nel/net/net_log.h"



#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#elif defined NL_OS_UNIX
#	include <netinet/in.h>
#endif

using namespace NLMISC;
using namespace std;

NLMISC::CVariable<uint32> MaxTCPPacketSize("nel", "MaxTCPPacketSize", "Maximum size of TCP packets created by cumulating small packets", 10240, 0, true);


namespace NLNET {


NLMISC::CMutex nettrace_mutex("nettrace_mutex");


/*
 * Constructor
 */
CBufSock::CBufSock( CTcpSock *sock ) :
	Sock( sock ),
	_KnowConnected( false ),
	_LastFlushTime( 0 ),
	_TriggerTime( 20 ),
	_TriggerSize( -1 ),
	_RTSBIndex( 0 ),
	_AppId( 0 ),
	_ConnectedState( false )
{
	nlnettrace( "CBufSock::CBufSock" ); // don't define a global object

	if ( Sock == NULL )
	  {
		Sock = new CTcpSock();
	  }

#ifdef NL_DEBUG
	_FlushTrigger = FTManual;
#endif
	_LastFlushTime = CTime::getLocalTime();
}


/*
 * Destructor
 */
CBufSock::~CBufSock()
{
	nlassert (this != InvalidSockId);	// invalid bufsock

	nlnettrace( "CBufSock::~CBufSock" );

	delete Sock; // the socket disconnects automatically if needed

	// destroy the structur to be sure that other people will not access to this anymore
	AuthorizedCallback = "";
	Sock = NULL;
	_KnowConnected = false;
	_LastFlushTime = 0;
	_TriggerTime = 0;
	_TriggerSize = 0;
	_ReadyToSendBuffer.clear ();
	_RTSBIndex = 0;
	_AppId = 0;
	_ConnectedState = false;
}


/*
 * Returns a readable string from a vector of bytes, beginning from pos, limited to 'len' characters. '\0' are replaced by ' '
 */
string stringFromVectorPart( const vector<uint8>& v, uint32 pos, uint32 len )
{
	nlassertex( pos+len <= v.size(), ("pos=%u len=%u size=%u", pos, len, v.size()) );

	string s;
	if ( (! v.empty()) && (len!=0) )
	{
		// Copy contents
		s.resize( len );
		memcpy( &*s.begin(), &*v.begin()+pos, len );

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			if ( ! isprint((uint8)(*is)) || (*is) == '%' )
			{
				(*is) = '?';
			}
		}
	}

	return s;
}


/*
 * Force to send data pending in the send queue now. In the case of a non-blocking socket
 * (see CNonBlockingBufSock), if all the data could not be sent immediately,
 * the returned nbBytesRemaining value is non-zero.
 * \param nbBytesRemaining If the pointer is not NULL, the method sets the number of bytes still pending after the flush attempt.
 * \returns False if an error has occured (e.g. the remote host is disconnected).
 * To retrieve the reason of the error, call CSock::getLastError() and/or CSock::errorString()
 *
 * Note: this method works with both blocking and non-blocking sockets
 * Precond: the send queue should not contain an empty block
 */
bool CBufSock::flush( uint *nbBytesRemaining )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	//nlnettrace( "CBufSock::flush" );

	// Copy data from the send queue to _ReadyToSendBuffer
	TBlockSize netlen;
//	vector<uint8> tmpbuffer;

	do
	{
		// Process each element in the send queue
		uint8 *tmpbuffer = NULL;
		uint32 size = 0;
		if (! SendFifo.empty())
		{
			SendFifo.front( tmpbuffer, size );
		}
		while ( ! SendFifo.empty() && ( (_ReadyToSendBuffer.size()==0) || (_ReadyToSendBuffer.size() +size < MaxTCPPacketSize) ) )
		{
			// Compute the size and add it into the beginning of the buffer
			netlen = htonl( (TBlockSize)size );
			uint32 oldBufferSize = _ReadyToSendBuffer.size();
			_ReadyToSendBuffer.resize (oldBufferSize+sizeof(TBlockSize)+size);
			*(TBlockSize*)&(_ReadyToSendBuffer[oldBufferSize])=netlen;
			//nldebug( "O-%u %u+L%u (0x%x)", Sock->descriptor(), oldBufferSize, size, size );


			// Append the temporary buffer to the global buffer
			CFastMem::memcpy (&_ReadyToSendBuffer[oldBufferSize+sizeof(TBlockSize)], tmpbuffer, size);
			SendFifo.pop();
			if (! SendFifo.empty())
			{
				SendFifo.front( tmpbuffer, size );
			}
		}

		// Actual sending of _ReadyToSendBuffer
		//if ( ! _ReadyToSendBuffer.empty() )
		if ( _ReadyToSendBuffer.size() != 0 )
		{
			// Send
			CSock::TSockResult res;
			TBlockSize len = _ReadyToSendBuffer.size() - _RTSBIndex;

			res = Sock->send( _ReadyToSendBuffer.getPtr()+_RTSBIndex, len, false );

			if ( res == CSock::Ok )
			{
/*				// Debug display
				switch ( _FlushTrigger )
				{
				case FTTime : LNETL1_DEBUG( "LNETL1: Time triggered flush for %s:", asString().c_str() ); break;
				case FTSize : LNETL1_DEBUG( "LNETL1: Size triggered flush for %s:", asString().c_str() ); break;
				default:	  LNETL1_DEBUG( "LNETL1: Manual flush for %s:", asString().c_str() );
				}
				_FlushTrigger = FTManual;
				LNETL1_DEBUG( "LNETL1: %s sent effectively a buffer (%d on %d B)", asString().c_str(), len, _ReadyToSendBuffer.size() );
*/

				// TODO OPTIM remove the nldebug for speed
				//commented for optimisation LNETL1_DEBUG( "LNETL1: %s sent effectively %u/%u bytes (pos %u wantedsend %u)", asString().c_str(), len, _ReadyToSendBuffer.size(), _RTSBIndex, realLen/*, stringFromVectorPart(_ReadyToSendBuffer,_RTSBIndex,len).c_str()*/ );

				if ( _RTSBIndex+len == _ReadyToSendBuffer.size() ) // for non-blocking mode
				{
					// If sending is ok, clear the global buffer
					//nldebug( "O-%u all %u bytes (%u to %u) sent", Sock->descriptor(), len, _RTSBIndex, _ReadyToSendBuffer.size() );
					_ReadyToSendBuffer.clear();
					_RTSBIndex = 0;
					if ( nbBytesRemaining )
						*nbBytesRemaining = 0;
				}
				else
				{
					// Or clear only the data that was actually sent
					nlassertex( _RTSBIndex+len < _ReadyToSendBuffer.size(), ("index=%u len=%u size=%u", _RTSBIndex, len, _ReadyToSendBuffer.size()) );
					//nldebug( "O-%u only %u B on %u (%u to %u) sent", Sock->descriptor(), len, _ReadyToSendBuffer.size()-_RTSBIndex, _RTSBIndex, _ReadyToSendBuffer.size() );
					_RTSBIndex += len;
					if ( nbBytesRemaining )
						*nbBytesRemaining = _ReadyToSendBuffer.size() - _RTSBIndex;
					if ( _ReadyToSendBuffer.size() > 20480 ) // if big, clear data already sent
					{
						uint nbcpy = _ReadyToSendBuffer.size() - _RTSBIndex;
						for (uint i = 0; i < nbcpy; i++)
						{
							_ReadyToSendBuffer[i] = _ReadyToSendBuffer[i+_RTSBIndex];
						}
						_ReadyToSendBuffer.resize(nbcpy);
						//_ReadyToSendBuffer.erase( _ReadyToSendBuffer.begin(), _ReadyToSendBuffer.begin()+_RTSBIndex );
						_RTSBIndex = 0;
						//nldebug( "O-%u Cleared data already sent, %u B remain", Sock->descriptor(), nbcpy );
					}
				}
			}
			else
			{
#ifdef NL_DEBUG
				// Can happen in a normal behavior if, for example, the other side is not connected anymore
				LNETL1_DEBUG( "LNETL1: %s failed to send effectively a buffer of %d bytes", asString().c_str(), _ReadyToSendBuffer.size() );
#endif
				// NEW: Clearing (loosing) the buffer if the sending can't be performed at all
				_ReadyToSendBuffer.clear();
				_RTSBIndex = 0;
				if ( nbBytesRemaining )
					*nbBytesRemaining = 0;
				return false;
			}
		}
		else
		{
			if ( nbBytesRemaining )
				*nbBytesRemaining = 0;
		}

	}
	while ( !SendFifo.empty() && _ReadyToSendBuffer.size()==0 );

	return true;
}


/* Sets the time flush trigger (in millisecond). When this time is elapsed,
 * all data in the send queue is automatically sent (-1 to disable this trigger)
 */
void CBufSock::setTimeFlushTrigger( sint32 ms )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	_TriggerTime = ms;
	_LastFlushTime = CTime::getLocalTime();
}


/*
 * Update the network sending (call this method evenly). Returns false if an error occured.
 */
bool CBufSock::update()
{
	nlassert (this != InvalidSockId);	// invalid bufsock
//	nlnettrace( "CBufSock::update-BEGIN" );
	// Time trigger

	if ( _TriggerTime != -1 )
	{
		TTime now = CTime::getLocalTime();
		if ( (sint32)(now-_LastFlushTime) >= _TriggerTime )
		{
#ifdef NL_DEBUG
			_FlushTrigger = FTTime;
#endif
			if ( flush() )
			{
				_LastFlushTime = now;
//				nlnettrace ( "CBufSock::update-END time 1" );
				return true;
			}
			else
			{
//				nlnettrace ( "CBufSock::update-END time 0" );
				return false;
			}
		}
	}
	// Size trigger
	if ( _TriggerSize != -1 )
	{
		if ( (sint32)SendFifo.size() > _TriggerSize )
		{
#ifdef NL_DEBUG
			_FlushTrigger = FTSize;
#endif
//			nlnettrace( "CBufSock::update-END size" );
			return flush();
		}
	}
//	nlnettrace( "CBufSock::update-END nosend" );
	return true;
}


/*
 * Connects to the specified addr; set connectedstate to true if no connection advertising is needed
 * Precond: not connected
 */
void CBufSock::connect( const CInetAddress& addr, bool nodelay, bool connectedstate )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlassert( ! Sock->connected() );

	Sock->connect( addr );
	_ConnectedState = connectedstate;
	_KnowConnected = connectedstate;
	if ( nodelay )
	{
		Sock->setNoDelay( true );
	}
	_ReadyToSendBuffer.clear();
	_RTSBIndex = 0;
}


/*
 * Disconnects; set connectedstate to false if no disconnection advertising is needed
 */
void CBufSock::disconnect( bool connectedstate )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	Sock->disconnect();
	_ConnectedState = connectedstate;
	_KnowConnected = connectedstate;
}


/*
 * Returns a string with the characteristics of the object
 */
string CBufSock::asString() const
{
//	stringstream ss;
	string str;
	if (this == InvalidSockId) // tricky
		str = "<null>";
	else
	{
		// if it crashs here, it means that the CBufSock was deleted and you try to access to the virtual table that is empty
		// because the object is destroyed.
		str += typeStr();
		str += NLMISC::toStringPtr(this) + " (socket ";

		if (Sock == NULL)
			str += "<null>";
		else
			str += NLMISC::toString(Sock->descriptor());

		str += ")";
	}
	return str;
}


/*
 * Constructor
 */
CNonBlockingBufSock::CNonBlockingBufSock( CTcpSock *sock, uint32 maxExpectedBlockSize ) :
	CBufSock( sock ),
	_MaxExpectedBlockSize( maxExpectedBlockSize ),
	_NowReadingBuffer( false ),
	_BytesRead( 0 ),
	_Length( 0 )
{
	nlnettrace( "CNonBlockingBufSock::CNonBlockingBufSock" );
}


/*
 * Constructor with an existing socket (created by an accept())
 */
CServerBufSock::CServerBufSock( CTcpSock *sock ) :
	CNonBlockingBufSock( sock ),
	_Advertised( false ),
	_OwnerTask( NULL )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlnettrace( "CServerBufSock::CServerBufSock" );
}


// In Receive Threads:


/*
 * Receives a part of a message (nonblocking socket only)
 */
bool CNonBlockingBufSock::receivePart( uint32 nbExtraBytes )
{
	nlassert (this != InvalidSockId);	// invalid bufsock
	nlnettrace( "CNonBlockingBufSock::receivePart" );

	TBlockSize actuallen;
	if ( ! _NowReadingBuffer )
	{
		// Receiving length prefix
		actuallen = sizeof(_Length)-_BytesRead;
		CSock :: TSockResult ret = Sock->receive( (uint8*)(&_Length)+_BytesRead, actuallen, false );
		if (ret == CSock::ConnectionClosed)
		{
			LNETL1_DEBUG( "LNETL1: Connection %s closed", asString().c_str() );
			return false;
		}
		else if (ret == CSock::Error)
		{
			LNETL1_DEBUG( "LNETL1: Socket error for %s", asString().c_str() );
			Sock->disconnect();
			return false;
		}

		_BytesRead += actuallen;
		if ( _BytesRead == sizeof(_Length ) )
		{
			if ( _Length != 0 )
			{
				_Length = ntohl( _Length );
				//nldebug( "I-%u L%u (0x%x) a%u", Sock->descriptor(), _Length, _Length, actuallen );

				// Test size limit
				if ( _Length > _MaxExpectedBlockSize )
				{
					nlwarning( "LNETL1: Socket %s received header length %u exceeding max expected %u... Disconnecting", asString().c_str(), _Length, _MaxExpectedBlockSize );
					throw ESocket( toString( "Received length %u exceeding max expected %u from %s", _Length, _MaxExpectedBlockSize, Sock->remoteAddr().asString().c_str() ).c_str(), false );
				}

				_NowReadingBuffer = true;
				_ReceiveBuffer.resize( _Length + nbExtraBytes );
			}
			else
			{
				nlwarning( "LNETL1: Socket %s received null length in block header", asString().c_str() );
			}
			_BytesRead = 0;
		}
	}

	if ( _NowReadingBuffer )
	{
		// Receiving payload buffer
		actuallen = _Length-_BytesRead;
		Sock->receive( &*_ReceiveBuffer.begin()+_BytesRead, actuallen );
		_BytesRead += actuallen;

		if ( _BytesRead == _Length )
		{
#ifdef NL_DEBUG
			LNETL1_DEBUG( "LNETL1: %s received buffer (%u bytes): [%s]", asString().c_str(), _ReceiveBuffer.size(), stringFromVector(_ReceiveBuffer).c_str() );
#endif
			_NowReadingBuffer = false;
			//nldebug( "I-%u all %u B on %u", Sock->descriptor(), actuallen );
			_BytesRead = 0;
			return true;
		}
		//else
		//{
		//	nldebug( "I-%u only %u B on %u", actuallen, Sock->descriptor(), _Length-(_BytesRead-actuallen) );
		//}
	}

	return false;
}


} // NLNET
