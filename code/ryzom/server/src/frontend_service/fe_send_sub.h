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



#ifndef NL_FE_SEND_SUB_H
#define NL_FE_SEND_SUB_H

#include "nel/misc/types_nl.h"
#include <nel/misc/md5.h>

#include "fe_receive_sub.h"

class CPrioSub;


const uint	SYNCSendLatency = 100;		// number of ms between sends of SYNC system message
const uint	PROBESendLatency = 300;		// number of ms between sends of PROBE system message


/**
 * Front-end Send Subsystem
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CFeSendSub
{
public:

	/*
	 * Buffer and address
	 */
	class CSendBuffer
	{
	public:
		typedef bool TSBState;

		/// Destination address
		NLNET::CInetAddress		DestAddress;

		/// Used (connected) or not
		volatile TSBState		SBState;

		/// Output buffer
		TOutBox					OutBox;

		/// Default constructor
		CSendBuffer() : SBState(false), OutBox(false, 512) {} // prealloc 512 bytes to avoid bitmemstream reallocation

		/// Set the new address. state should be SBReady or SBNotReady only.
		void	setAddress( const NLNET::CInetAddress *addr, TSBState state )
		{
			// Copy the address (just a link would not work)
			DestAddress = *addr;
			SBState = state;
		}

		/// Enable or disable the current address
		void	enableSendBuffer( TSBState state )
		{
			SBState = state;
		}

		/// Send the current outbox
		void	sendOutBox( NLNET::CUdpSock *datasock );
	};

	/// Vector for send buffers indexed by TClientId
	typedef std::vector<CSendBuffer> TSendBuffers;

	/// Set of ids of buffers to enable in the flushing buffers before the next swapping
	typedef std::set<TClientId> TBuffersToEnable;

	/// Constructor
	CFeSendSub() :
		_DataSock( NULL ),
		_ClientIdCont(),
//		_UIActions(),
		_History( NULL ),
		_PrioSub( NULL ),
		_TotalBitBandwidth(0xFFFFFFFF),
		_ClientBitBandwidth(150*8),
		//_OutputBits(0),
		_SendCounter(0),
		_NbActions(0),
		_NbImpulseActions(0)
		{}

	/// Init
	void	init( NLNET::CUdpSock *datasock, THostMap *clientmap, CHistory *history, CPrioSub *priosub );

	/// Update
	void	update();

	/// Set client bandwidth per cycle in bytes
	void	setClientBandwidth( uint32 bytes );

	/// Set total bandwidth per cycle in bytes (limited to 512 MB) (currently not used!)
	void	setTotalBandwidth( uint32 bytes )
	{
		if ( bytes < 536870912 ) // 512 MB
			_TotalBitBandwidth = bytes*8;
		else
			_TotalBitBandwidth = 0xFFFFFFFF; // prevent from overflow
	}

	uint32 clientBandwidth() const { return _ClientBitBandwidth; }
	
	uint32 totalBandwidth() const { return _TotalBitBandwidth; }

	TClientIdCont&			clientIdCont() { return _ClientIdCont; }

	volatile uint32			&sendCounter() { return _SendCounter; }

	/// Set the address for send buffer, to match a connected client
	void					setSendBufferAddress( TClientId id, const NLNET::CInetAddress *addr )
	{
		// We set the address for both, but we can't enable the buffer yet
		((*_CurrentFillingBuffers)[id]).setAddress( addr, false /*true*/ );
		((*_CurrentFlushingBuffers)[id]).setAddress( addr, false );
		//_BuffersToEnable.insert( id ); // OBSOLETE: now it is enabled/disabled in CFeSendSub::fillPrioritizedActions
	}

	/// Unset a send buffer, when a client disconnects
	void					disableSendBuffer( TClientId id )
	{
		// We can disable both, because no problem if the flushing thread sees the state of a buffer change to unused
		((*_CurrentFillingBuffers)[id]).enableSendBuffer( false );
		((*_CurrentFlushingBuffers)[id]).enableSendBuffer( false );

		// But we must remove the id for _BuffersToEnable in the case when there was
		// a connection and then a disconnection for the same client in the same cycle
		// (maybe, not necessary to manage, because unlikely to happen)
		//_BuffersToEnable.erase( id ); // OBSOLETE
	}

	/// Enable a send buffer, the first time when the buffer is ready to be flushed out
	void					enableSendBuffer( TClientId id )
	{
		// In the filling one, we enabled the buffer as soon as the client connected
		((*_CurrentFlushingBuffers)[id]).enableSendBuffer( true );
	}

	/// Access the outbox (for filling)
	TOutBox&				outBox( TClientId id )
	{
		return (*_CurrentFillingBuffers)[id].OutBox;
	}		

	/// Call before a send cycle (even if there is no client)
	void					prepareSendCycle()
	{
		_NbActions = 0;
		_NbImpulseActions = 0;
	}

	/// Setup headers for outgoing messages of current cycle (only if there are clients)
	void					prepareHeadersAndFillImpulses();

	/// Fill prioritized actions into outgoing messages (only if there are clients)
	void					fillPrioritizedActions();

	/// Swap send buffers
	void					swapSendBuffers();

	/// Send outgoing messages
	void					flushMessages();

private:

	/// Socket access
	NLNET::CUdpSock			*_DataSock;

	/** Client id container (insert()/erase() are done by the receive subsystem who manages the clients).
	 * For a vector, there are as many elements as possibles clients, non-attributed elements are set to NULL.
	 */
	TClientIdCont			_ClientIdCont;

	/// Urgent-important actions
//	CUrgentImportantActions	_UIActions;

	// Client map access
	THostMap				*_ClientMap;

	/// Packet History access
	CHistory				*_History;

	// Priority access
	CPrioSub				*_PrioSub;

	/** Max total bandwidth (bytes per update)
	 * Warning: uint32 for bits => limitation is 512 KB ((2^32-1)/8))
	 * see setTotalBitBandwidth
	 */
	uint32					_TotalBitBandwidth;

	/// Max bandwidth per client (bytes per update)
	uint32					_ClientBitBandwidth;

	/// Number of bits stored for the current cyle
	//uint32					_OutputBits;

	/// Number of messages effectively sent (flushed)
	volatile uint32			_SendCounter;

	/// Number of actions stored for the current cycle (important + prioritized)
	uint32					_NbActions;

	/// Number of important actions stored for the current cycle
	uint32					_NbImpulseActions;

	TSendBuffers			_SendBuffers1, _SendBuffers2;
	TSendBuffers			*_CurrentFillingBuffers, *_CurrentFlushingBuffers;

	/// MD5 hash keys of msg.xml and database.xml
	NLMISC::CHashKeyMD5		_MsgXmlMD5;
	NLMISC::CHashKeyMD5		_DatabaseXmlMD5;

	//TBuffersToEnable		_BuffersToEnable; // OBSOLETE
};


#endif // NL_FE_SEND_SUB_H

/* End of fe_send_sub.h */

