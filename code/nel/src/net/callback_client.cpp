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

#include "nel/misc/types_nl.h"
#include "nel/net/callback_net_base.h"
#include "nel/net/callback_client.h"
#include "nel/net/net_log.h"


#ifdef USE_MESSAGE_RECORDER
#include "nel/net/message_recorder.h"
#endif


namespace NLNET {


/*
 * Constructor
 */
CCallbackClient::CCallbackClient( TRecordingState rec, const std::string& recfilename, bool recordall, bool initPipeForDataAvailable ) :
	CCallbackNetBase( rec, recfilename, recordall ), CBufClient( true, rec==Replay, initPipeForDataAvailable )
{
	LockDeletion = false;
	CBufClient::setDisconnectionCallback (_NewDisconnectionCallback, this);

	_IsAServer = false;
	_DefaultCallback = NULL;
}

CCallbackClient::~CCallbackClient()
{
	nlassert(!LockDeletion);
}

/*
 * Send a message to the remote host (pushing to its send queue)
 * Recorded : YES
 * Replayed : MAYBE
 */
void CCallbackClient::send (const CMessage &buffer, TSockId hostid, bool /* log */)
{
	nlassert (hostid == InvalidSockId);	// should always be InvalidSockId on client
	nlassert (connected ());
	nlassert (buffer.length() != 0);
	nlassert (buffer.typeIsSet());

	_BytesSent += buffer.length ();

//	if (log)
	{
//		nldebug ("LNETL3C: Client: send(%s)", buffer.toString().c_str());
//		nldebug ("send message number %u", SendNextValue);
	}

#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// Send
		CBufClient::send (buffer);

#ifdef USE_MESSAGE_RECORDER
		if ( _MR_RecordingState == Record )
		{
			// Record sent message
			_MR_Recorder.recordNext( _MR_UpdateCounter, Sending, hostid, const_cast<CMessage&>(buffer) );
		}
	}
#endif
}

/*
 * Force to send all data pending in the send queue.
 * Recorded : NO
 * Replayed : NO
 */
bool CCallbackClient::flush (TSockId hostid, uint *nbBytesRemaining)
{
	nlassert (hostid == InvalidSockId);	// should always be InvalidSockId on client

#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// Flush sending (nothing to do in replay mode)
		return CBufClient::flush( nbBytesRemaining );

#ifdef USE_MESSAGE_RECORDER
	}
	else
	{
		return true;
	}
#endif
}


/*
 * Updates the network (call this method evenly)
 * Recorded : YES (in baseUpdate())
 * Replayed : YES (in baseUpdate())
 */
void CCallbackClient::update2 ( sint32 timeout, sint32 mintime )
{
	LockDeletion = true;
//	nldebug ("L3: Client: update()");

	H_AUTO(L3UpdateClient2);

	baseUpdate2 (timeout, mintime); // first receive

#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// L1-2 Update (nothing to do in replay mode)
		CBufClient::update (); // then send

#ifdef USE_MESSAGE_RECORDER
	}
#endif

	LockDeletion = false;
}


/*
 * Updates the network (call this method evenly) (legacy)
 * Recorded : YES (in baseUpdate())
 * Replayed : YES (in baseUpdate())
 */
void CCallbackClient::update ( sint32 timeout )
{
	LockDeletion = true;
//	nldebug ("L3: Client: update()");

	H_AUTO(L3UpdateClient);

	baseUpdate (timeout); // first receive

#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// L1-2 Update (nothing to do in replay mode)
		CBufClient::update (); // then send

#ifdef USE_MESSAGE_RECORDER
	}
#endif

	LockDeletion = false;
}


/*
 * Returns true if there are messages to read
 * Recorded : NO
 * Replayed : YES
 */
bool CCallbackClient::dataAvailable ()
{
#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// Real dataAvailable()
		return CBufClient::dataAvailable ();

#ifdef USE_MESSAGE_RECORDER
	}
	else
	{
		// Simulated dataAvailable()
		return CCallbackNetBase::replayDataAvailable();
	}
#endif
}


/*
 * Read the next message in the receive queue
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackClient::receive (CMessage &buffer, TSockId *hostid)
{
//	nlassert (connected ());
	*hostid = InvalidSockId;

#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
#endif

		// Receive
		CBufClient::receive (buffer);

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
#ifdef NL_BIG_ENDIAN
		uint32 val = NLMISC_BSWAP32(*(uint32*)buffer.buffer ());
#else
		uint32 val = *(uint32*)buffer.buffer ();
#endif

#ifdef USE_MESSAGE_RECORDER
		if ( _MR_RecordingState == Record )
		{
			// Record received message
			_MR_Recorder.recordNext( _MR_UpdateCounter, Receiving, *hostid, const_cast<CMessage&>(buffer) );
		}
	}
	else
	{
		// Retrieve received message loaded by dataAvailable()
		buffer = _MR_Recorder.ReceivedMessages.front().Message;
		_MR_Recorder.ReceivedMessages.pop();
	}
#endif

	buffer.readType ();
}

/*
 *
 */
TSockId	CCallbackClient::getSockId (TSockId hostid)
{
	nlassert (hostid == InvalidSockId);

	return id ();
}


/*
 * Connect to the specified host
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackClient::connect( const CInetAddress& addr )
{
#ifdef USE_MESSAGE_RECORDER
	if ( _MR_RecordingState != Replay )
	{
		try
		{
#endif

			// Connect
			CBufClient::connect( addr );

#ifdef USE_MESSAGE_RECORDER
			if ( _MR_RecordingState == Record )
			{
				// Record connection
				CMessage addrmsg;
				addrmsg.serial( const_cast<CInetAddress&>(addr) );
				_MR_Recorder.recordNext( _MR_UpdateCounter, Connecting, _BufSock, addrmsg );
			}
		}
		catch (const ESocketConnectionFailed&)
		{
			if ( _MR_RecordingState == Record )
			{
				// Record connection
				CMessage addrmsg;
				addrmsg.serial( const_cast<CInetAddress&>(addr) );
				_MR_Recorder.recordNext( _MR_UpdateCounter, ConnFailing, _BufSock, addrmsg );
			}
			throw;
		}
	}
	else
	{
		// Check the connection : failure or not
		TNetworkEvent event = _MR_Recorder.replayConnectionAttempt( addr );
		switch ( event )
		{
		case Connecting :
			// Set the remote address
			nlassert( ! _BufSock->Sock->connected() );
			_BufSock->connect( addr, _NoDelay, true );
			_PrevBytesDownloaded = 0;
			_PrevBytesUploaded = 0;
			/*_PrevBytesReceived = 0;
			_PrevBytesSent = 0;*/
			break;
		case ConnFailing :
			throw ESocketConnectionFailed( addr );
			//break;
		default :
			nlwarning( "LNETL3C: No connection event in replay data, at update #%"NL_I64"u", _MR_UpdateCounter );
		}
	}
#endif
}


/*
 * Disconnect a connection
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackClient::disconnect( TSockId hostid )
{
	nlassert (hostid == InvalidSockId);	// should always be InvalidSockId on client

	// Disconnect only if connected (same as physically connected for the client)
	if ( _BufSock->connectedState() )
	{

#ifdef USE_MESSAGE_RECORDER
		if ( _MR_RecordingState != Replay )
		{
#endif

			// Disconnect
			CBufClient::disconnect ();

#ifdef USE_MESSAGE_RECORDER
		}
		else
		{
			// Read (skip) disconnection in the file
			if ( ! (_MR_Recorder.checkNextOne( _MR_UpdateCounter ) == Disconnecting) )
			{
				nlwarning( "LNETL3C: No disconnection event in the replay data, at update #%"NL_I64"u", _MR_UpdateCounter );
			}
		}
		// Record or replay disconnection (because disconnect() in the client does not push a disc. event)
		noticeDisconnection( _BufSock );
#endif
	}
}


#ifdef USE_MESSAGE_RECORDER


/*
 * replay connection and disconnection callbacks, client version
 */
bool CCallbackClient::replaySystemCallbacks()
{
	do
	{
		if ( _MR_Recorder.ReceivedMessages.empty() )
		{
			return false;
		}
		else
		{
			switch( _MR_Recorder.ReceivedMessages.front().Event )
			{
			case Receiving:
				return true;

			case Disconnecting:
				LNETL3_DEBUG( "LNETL3C: Disconnection event" );
				_BufSock->setConnectedState( false );

				// Call callback if needed
				if ( disconnectionCallback() != NULL )
				{
					disconnectionCallback()( id(), argOfDisconnectionCallback() );
				}
				break;

			default:
				nlerror( "LNETL3C: Invalid system event type in client receive queue" );
			}
			// Extract system event
			_MR_Recorder.ReceivedMessages.pop();
		}
	}
	while ( true );
}


#endif // USE_MESSAGE_RECORDER


} // NLNET
