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

#include "nel/misc/atomic.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/net/inet_address.h"

#include "fe_receive_task.h"

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

	// Reference to the internal connection (immutable)
	void *Connection;

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
