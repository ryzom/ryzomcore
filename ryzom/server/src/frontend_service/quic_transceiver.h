// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_QUIC_TRANSCEIVER_H
#define NL_QUIC_TRANSCEIVER_H

#include "nel/misc/types_nl.h"

#include <memory>
#include <atomic>
#include <mutex>

#include "nel/misc/atomic.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/net/inet_address.h"

#include "game_share/stream_record.h"

#include "fe_receive_task.h"
#include "stream_queues.h"
#include "stream_scheduler.h"

/*

Beyond the realm of mere mortal code,
Lies a wondrous feat, a wizard's ode.
A QUIC implementation of the FES,
That sends and receives with utmost finesse.

The wizard's craft, a magic that transcends,
A network protocol that never ends.
A dance of data, a magical sight,
Through the air, it soars in flight.

It brings to life a world of wonder,
Where games are played, and time doth thunder.
A service that listens with a keen ear,
To every message, both far and near.

With this wizardry, comes a power unseen,
A protocol that's fast, efficient and clean.
A transceiver of QUIC, that knows no bounds,
With speeds so fast, it never slows down.

So if you seek a protocol that's rare,
A service that's fast, and knows how to care,
Look no further, for the FES is here,
A wizard's code, that's sure to bring you cheer.
  - ChatGPT

*/

// #ifdef NL_DEBUG
#define FE_DEBUG_QUIC
// #endif

class CClientHost;

class CQuicTransceiverImpl;
class CQuicTransceiver;

struct CQuicBuffer
{
	uint32 Length;
	uint8 *Buffer;
};

class CQuicUserContext;

/// A block of bytes for a stream, from when the main thread takes them out of the queues of the connection until
/// MsQuic says they were delivered (or dropped with the connection)
struct CQuicStreamSend
{
	CQuicBuffer Buffer; // Data and its length
	uint8 Data[1];
};

/// What a stream callback gets (immutable)
struct CQuicStreamContext
{
	CQuicUserContext *User;
	uint8 StreamClass; // number of the class of messages
};

/// Why the streams of a connection are of no use any more. Bytes that were taken out of the queues can then not be
/// delivered, which leaves a hole in a stream, so the main thread disconnects the client.
enum TQuicStreamFault
{
	QuicStreamNoFault = 0,
	QuicStreamOpenFailed, // A stream could not be opened or started
	QuicStreamWrongId, // A stream did not get the id of its class
	QuicStreamSendFailed, // MsQuic refused a block
	QuicStreamSendCanceled, // MsQuic dropped a block while the connection was still up
	QuicStreamAbortedByPeer, // The client refused a stream, which a client of this protocol level never does
};

// User context for quic messages
class CQuicUserContext
{
public:
	// Manual reference count
	void increaseRef()
	{
		int value = m_RefCount.fetchAdd(1, NLMISC::TMemoryOrderRelaxed) + 1;
#ifdef FE_DEBUG_QUIC
		if (DebugRefCount)
			nldebug("Reference count [%p] is now %i", this, value);
#endif
	}

	void decreaseRef()
	{
		int value = m_RefCount.fetchAdd(-1, NLMISC::TMemoryOrderRelease) - 1;
#ifdef FE_DEBUG_QUIC
		if (DebugRefCount)
			nldebug("Reference count [%p] is now %i", this, value);
#endif
		// Release order to ensure this thread is done with this object
		if (value == 0)
		{
			// Acquire order to ensure all other threads are done with this object
			std::atomic_thread_fence(std::memory_order_acquire);
			delete this;
		}
	}

public:
	CQuicUserContext();
	~CQuicUserContext();

	// Reference to QUIC context (immutable)
	CQuicTransceiver *Transceiver;

	// Handle of the connection, null once closed or given back to MsQuic (taken out under the transceiver's lock)
	void *Connection;

	// The same handle for the callbacks of the streams, which run while it is open (immutable)
	void *CallbackConnection = nullptr;

	// The callbacks get the handle as a parameter; everywhere else Connection is read, used or taken out under this
	// lock (the send thread sends through it, the main thread shuts it down, takeConnection takes it out to close it).
	// When both are taken, it is locked after the transceiver's OpenConnections lock.
	std::mutex ConnectionMutex;

	// Not a real address, just a token to identify the connection (immutable)
	// Give everyone an IPv6 in unique local network fdd5:d66b:8698::/48
	// The addresses are a sequential hash sequence, so should be unique for a very very long time
	// They should not be used for security purposes
	NLNET::CInetAddress TokenAddr;

	// Reference to the client host (game state) (owned by service main thread)
	CClientHost *ClientHost = nullptr;

	// Set if datagrams can be sent (set on connection thread, read on service main thread)
	NLMISC::CAtomicInt MaxSendLength = 0;

	// Send buffer, one being sent at a time, released as soon as it's sent out
	NLMISC::CAtomicFlag SendBusy = false;
	NLMISC::CBitMemStream SendBuffer = NLMISC::CBitMemStream(false, 512);
	CQuicBuffer SendQuicBuffer;
	NLMISC::CAtomicInt SentCount = 0;

	/// \name Streams of protocol level ryzomcore/4.2, one per class of messages (game_share/stream_record.h)
	// @{

	// The connection is of that protocol level (set on the connection thread when it connects, before any datagram of the
	// client comes in)
	NLMISC::CAtomicBool Streams = false;

	// The records that wait for the streams (service main thread only)
	CStreamQueues StreamQueues;

	// What the path to the client takes of stream data per cycle, and where the client is in the turns of the frontend
	// (service main thread only, see CFeSendSub::grantStreams)
	CStreamAllowance StreamAllowance;
	CStreamSchedulerClient StreamTurn;
	uint32 StreamLastGrant = 0; // bytes it got in the last cycle

	// Bytes taken out of the queues that MsQuic has not completed yet, which is when they were acknowledged
	// (added by the main thread when it takes them, subtracted on the connection thread, or where a block is dropped)
	NLMISC::CAtomicInt StreamOutstanding = 0;

	// Set where the fault happens (any thread), acted upon by the main thread
	NLMISC::CAtomicInt StreamFault = QuicStreamNoFault;

	// The connection is shutting down (set on the connection thread): what fails from then on is no fault
	NLMISC::CAtomicBool ShuttingDown = false;

	// Datagrams still to send with the bit that says the path to the client changed (set on the connection thread when
	// the address of the client changes, counted down by the main thread): the client then measures the lateness of
	// its datagrams against the new path alone
	NLMISC::CAtomicInt PathChangedDatagrams = 0;

	// What the client last reported: how late the datagrams reach it, in milliseconds (decoded, at most
	// CLFECOMMON::DatagramLatenessMax), and the number of the datagram it had received last then (service main thread
	// only)
	bool QueueDelayKnown = false;
	uint16 QueueDelay = 0;
	uint32 QueueDelayAck = 0;
	/// The congestion window left the streams less room in this cycle than the allowance (CFeSendSub::grantStreams)
	bool StreamWindowLimited = false;
	uint32 QueueDelayReports = 0; // how many reports came, and how many of them the allowance has seen
	uint32 QueueDelayReportsSeen = 0;

	// What MsQuic last said about the path, for the room the streams may take in the congestion window
	// (stored on the connection thread, read by the main thread; each value on its own, they need not match exactly)
	NLMISC::CAtomicBool StatValid = false;
	NLMISC::CAtomicInt StatCongestionWindow = 0; // bytes
	NLMISC::CAtomicInt StatRtt = 0; // microseconds
	NLMISC::CAtomicInt StatBytesInFlight = -1; // bytes sent and not acknowledged, -1 if this MsQuic does not tell
	NLMISC::CAtomicInt StatGrantedCount = 0; // StreamGrantedCount when MsQuic was asked
	NLMISC::CAtomicInt StatCongestionEvents = 0; // times the connection saw congestion (loss or marks), since it opened

	// Datagrams of the frontend that were not sent: MsQuic dropped them (it could not write them at once, or they no
	// longer fit), or the one before had not left yet (counted where that happens; the main thread cuts the allowance
	// of the streams on it)
	NLMISC::CAtomicInt SendBusyDrops = 0;

	// Datagrams in a row that MsQuic dropped because it could not write them at once (connection thread): after a few,
	// the next one waits for room instead (see CQuicTransceiver::sendDatagramSwap)
	NLMISC::CAtomicInt DatagramCancels = 0;

	// The client is reached over another path, of which the allowance of the streams knows nothing yet (set on the
	// connection thread, taken by the main thread)
	NLMISC::CAtomicBool PathChanged = false;

	// Bytes taken out of the queues since the connection opened, modulo 2^32 (added by the main thread): with
	// StatGrantedCount, what was taken since MsQuic was last asked, which its bytes in flight do not count yet
	NLMISC::CAtomicInt StreamGrantedCount = 0;

	// Handles of the streams, in the order of the classes, null until opened and once taken out to be closed (under
	// ConnectionMutex; opened and used by whoever flushes: the send thread, or the main thread when it flushes itself)
	void *StreamHandles[CLFECOMMON::StreamClassMax] = {};
	bool StreamsOpened = false;
	CQuicStreamContext StreamContexts[CLFECOMMON::StreamClassMax];

	// @}

#ifdef FE_DEBUG_QUIC
	// After shutdown, enable ref count debugging
	NLMISC::CAtomicBool DebugRefCount = false;
#endif

private:
	NLMISC::CAtomicInt m_RefCount = 0;
};

// Utility to decrease reference count, does not increase count
// This is to ensure that the reference count is decreased even when exceptions get thrown
class CQuicUserContextRelease
{
public:
	CQuicUserContextRelease(CQuicUserContext *user)
	    : m_User(user)
	{
	}

	~CQuicUserContextRelease()
	{
		if (m_User)
			m_User->decreaseRef();
	}

	CQuicUserContextRelease(const CQuicUserContextRelease &) = delete;
	CQuicUserContextRelease &operator=(const CQuicUserContextRelease &) = delete;

private:
	CQuicUserContext *m_User;
};

// Regular reference counting
class CQuicUserContextPtr
{
public:
	CQuicUserContextPtr(CQuicUserContext *user)
	    : m_User(user)
	{
		if (m_User)
			m_User->increaseRef();
	}

	~CQuicUserContextPtr()
	{
		if (m_User)
			m_User->decreaseRef();
	}

	CQuicUserContextPtr(const CQuicUserContextPtr &other)
	    : m_User(other.m_User)
	{
		if (m_User)
			m_User->increaseRef();
	}

	CQuicUserContextPtr &operator=(CQuicUserContext *user)
	{
		if (m_User)
			m_User->decreaseRef();
		m_User = user;
		if (m_User)
			m_User->increaseRef();
		return *this;
	}

	CQuicUserContextPtr &operator=(const CQuicUserContextPtr &other)
	{
		if (m_User)
			m_User->decreaseRef();
		m_User = other.m_User;
		if (m_User)
			m_User->increaseRef();
		return *this;
	}

	CQuicUserContext *operator->() const
	{
		return m_User;
	}

	CQuicUserContext *get() const
	{
		return m_User;
	}

	// compare
	bool operator==(const CQuicUserContextPtr &other) const
	{
		return m_User == other.m_User;
	}

	bool operator!=(const CQuicUserContextPtr &other) const
	{
		return m_User != other.m_User;
	}

	bool operator==(const CQuicUserContext *other) const
	{
		return m_User == other;
	}

	bool operator!=(const CQuicUserContext *other) const
	{
		return m_User != other;
	}

	// compare less
	bool operator<(const CQuicUserContextPtr &other) const
	{
		return m_User < other.m_User;
	}

	bool operator<(const CQuicUserContext *other) const
	{
		return m_User < other;
	}

private:
	CQuicUserContext *m_User;
};

class CQuicTransceiver
{
public:
	CQuicTransceiver(uint32 msgsize);
	~CQuicTransceiver();

	/// Reload certificate
	void reloadCert();

	/// Check if QUIC is enabled in the config
	bool isConfigEnabled() const;

	/// Start listening
	void start(uint16 port);

	/// Stop listening
	void stop();

	/// Release. Instance is useless after this call
	void release();

	/// Set new write queue for incoming messages (thread-safe because mutexed)
	NLMISC::CBufFIFO *swapWriteQueue(NLMISC::CBufFIFO *writeQueue);

	/// Clear a write queue safely
	void clearQueue(NLMISC::CBufFIFO *writeQueue);

	/// Check if still listening
	bool listening();

	/// Send a datagram, fancier than a telegram, but not as reliable
	// void sendDatagram(CQuicUserContext *user, const uint8 *buffer, uint32 size);

	/// Send a datagram, this swaps the buffer with the previous one sent
	/// Only one datagram may be in flight at a time
	bool sendDatagramSwap(CQuicUserContext *user, NLMISC::CBitMemStream &buffer);

	/// Shutdown a connection
	void shutdown(CQuicUserContext *user);

	/// The classes of messages that go on streams, in the order of their streams (those msg.xml declares): a connection
	/// of protocol level ryzomcore/4.2 gets one stream per class. Set once, when the service starts, before its first
	/// cycle: nothing reads them before a message is queued for a stream, which the cycles do.
	static void setStreamClasses(const std::vector<std::string> &names);

	/// How many classes there are (at most CLFECOMMON::StreamClassMax)
	static uint streamClassCount();

	/// Name of a class, for logs
	static const char *streamClassName(uint streamClass);

	/// Allocate a block of bytes for a stream
	static CQuicStreamSend *createStreamSend(uint32 length);

	/// Free a block that was not given to sendStream()
	static void destroyStreamSend(CQuicStreamSend *send);

	/// Send a block on the stream of a class, after what was sent on it before. The block is taken in all cases: when it
	/// cannot be sent, it is dropped and the connection gets a fault (CQuicUserContext::StreamFault), unless it is
	/// shutting down. The first call opens the streams of the connection. Called by whoever flushes.
	void sendStream(CQuicUserContext *user, uint8 streamClass, CQuicStreamSend *send);

	/// Name of a fault, for logs
	static const char *streamFaultName(int fault);

	/// Bytes that MsQuic has sent since the process started, for all connections and with all that QUIC adds
	/// (acknowledgements, data sent again, handshakes), and the UDP and IP headers of its packets counted as IPv6.
	/// False if there is no MsQuic to ask.
	static bool egressBytes(uint64 &bytes);

private:
	friend CQuicTransceiverImpl;
	friend CQuicUserContext;

	/// Internal implementation specific
	std::unique_ptr<CQuicTransceiverImpl> m;

	/// User configuration
	uint32 m_MsgSize;

	/// Received datagram
	void datagramReceived(CQuicUserContext *user, const uint8 *buffer, uint32 length);

	/// Received shutdown
	void shutdownReceived(CQuicUserContext *user);

	/// Generates a token address to identify the connection with existing code
	NLNET::CInetAddress generateTokenAddr();
};

#endif /* NL_QUIC_TRANSCEIVER_H */

/* end of file */
