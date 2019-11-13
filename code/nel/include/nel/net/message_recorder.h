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

#ifndef NL_MESSAGE_RECORDER_H
#define NL_MESSAGE_RECORDER_H

#include "nel/misc/types_nl.h"
#include "buf_net_base.h"
//#include "callback_net_base.h"
#include "message.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/mem_stream.h"

#include <fstream>
#include <queue>
#include <string>

namespace NLNET {


class CInetAddress;

/// Type of network events (if changed, don't forget to change EventToString() and StringToEvent()
enum TNetworkEvent { Sending, Receiving, Connecting, ConnFailing, Accepting, Disconnecting, Error };


/// TNetworkEvent -> string
std::string EventToString( TNetworkEvent e );

/// string -> TNetworkEvent
TNetworkEvent StringToEvent( std::string& s );


/*
 * TMessageRecord
 */
struct TMessageRecord
{
	/// Default constructor
	TMessageRecord( bool input = false ) : UpdateCounter(0), SockId(InvalidSockId), Message( "", input ) {}

	/// Alt. constructor
	TMessageRecord( TNetworkEvent event, TSockId sockid, CMessage& msg, sint64 updatecounter ) :
		UpdateCounter(updatecounter), Event(event), SockId(sockid), Message(msg) {}

	/// Serial to string stream
	void serial( NLMISC::CMemStream& stream )
	{
		nlassert( stream.stringMode() );

		uint32 len = 0;
		std::string s_event;
		stream.serial( UpdateCounter );
		if ( stream.isReading() )
		{
			stream.serial( s_event );
			Event = StringToEvent( s_event );
			uint32 sockId = (uint32)(size_t)SockId;
			stream.serialHex( sockId );
			stream.serial( len );
			stream.serialBuffer( Message.bufferToFill( len ), len );
		}
		else
		{
			s_event = EventToString( Event );
			stream.serial( s_event );
			uint32 sockId;
			stream.serialHex( sockId );
			SockId = (NLNET::TSockId)(size_t)sockId;
			len = Message.length();
			stream.serial( len );
			stream.serialBuffer( const_cast<uint8*>(Message.buffer()), len ); // assumes the message contains plain text
		}
	}

	//NLMISC::TTime		Time;
	sint64				UpdateCounter;
	TNetworkEvent		Event;
	TSockId				SockId;
	CMessage			Message;
};



/**
 * Message recorder.
 * The service performs sends normally. They are intercepted and the recorder
 * plays the receives back. No communication with other hosts.
 * Warning: it works only with messages as plain text.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CMessageRecorder
{
public:

	/// Constructor
	CMessageRecorder();

	/// Destructor
	~CMessageRecorder();

	/// Start recording
	bool	startRecord( const std::string& filename, bool recordall=true );

	/// Add a record
	void	recordNext( sint64 updatecounter, TNetworkEvent event, TSockId sockid, CMessage& message );

	/// Stop recording
	void	stopRecord();

	/// Start replaying
	bool	startReplay( const std::string& filename );

	/// Push the received blocks for this counter into the receive queue
	void	replayNextDataAvailable( sint64 updatecounter );

	/**
	 * Returns the event type if the counter of the next event is updatecounter,
	 * and skip it; otherwise return Error.
	 */
	TNetworkEvent	checkNextOne( sint64 updatecounter );

	/// Get the first stored connection attempt corresponding to addr
	TNetworkEvent	replayConnectionAttempt( const CInetAddress& addr );

	/// Stop playback
	void	stopReplay();

	/// Receive queue (corresponding to one update count). Use empty(), front(), pop().
	std::queue<NLNET::TMessageRecord>	ReceivedMessages;

protected:

	/// Load the next record from the file (throws EStreamOverflow)
	bool	loadNext( TMessageRecord& record );

	/// Get the next record (from the preloaded records, or from the file)
	bool	getNext( TMessageRecord& record, sint64 updatecounter );

private:

	// Input/output file
	std::fstream								_File;

	// Filename
	std::string									_Filename;

	// Preloaded records
	std::deque<TMessageRecord>					_PreloadedRecords;

	// Connection attempts
	std::deque<TMessageRecord>					_ConnectionAttempts;

	// If true, record all events including sends
	bool										_RecordAll;
};


} // NLNET


#endif // NL_MESSAGE_RECORDER_H

/* End of message_recorder.h */
