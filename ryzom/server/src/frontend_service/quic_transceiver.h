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
		m_RefCount.fetch_add(1, std::memory_order_relaxed);
	}

	void decreaseRef()
	{
		if (m_RefCount.fetch_sub(1, std::memory_order_release) == 1)
		{
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
	NLMISC::CAtomicInt MaxSendLength;

	// Send buffer, one being sent at a time, released as soon as it's sent out
	NLMISC::CAtomicFlag SendBusy;
	NLMISC::CBitMemStream SendBuffer = NLMISC::CBitMemStream(false, 512);
	CQuicBuffer SendQuicBuffer;
	NLMISC::CAtomicInt SentCount;

private:
	std::atomic_int m_RefCount = 0;
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

private:
	CQuicUserContext *m_User;
};

class CQuicTransceiver
{
public:
	CQuicTransceiver(uint32 msgsize);
	~CQuicTransceiver();

	/// Start listening
	void start(uint16 port);

	/// Stop listening
	void stop();

	/// Release. Instance is useless after this call
	void release();

	/// Set new write queue for incoming messages (thread-safe because mutexed)
	NLMISC::CBufFIFO *swapWriteQueue(NLMISC::CBufFIFO *writeQueue);

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
