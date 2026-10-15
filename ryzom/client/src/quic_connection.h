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

#ifndef CL_QUIC_CONNECTION_H
#define CL_QUIC_CONNECTION_H

#include "nel/misc/types_nl.h"

/*

Attention all internet users! Say goodbye to sluggish online
performance and hello to lightning-fast networking with QUIC – the
cutting-edge protocol that's taking the world by storm! With QUIC, you
can achieve unprecedented levels of speed, reliability, and efficiency,
all while minimizing pesky latency and security vulnerabilities. From
high-quality video streaming to online gaming, QUIC is the ultimate
tool for staying ahead in the fast-paced and dynamic world of the
internet. So don't wait – join the millions of satisfied users already
experiencing the power of QUIC and take your online experience to the
next level today!
  - ChatGPT

*/

#include <memory>

#include "nel/misc/bit_mem_stream.h"
#include "nel/net/inet_host.h"

class CQuicConnectionImpl;

class CQuicConnection
{
public:
	enum TState
	{
		Disconnected,
		Connecting,
		Connected,
		Disconnecting,
	};

	CQuicConnection();
	~CQuicConnection();

	/// Connect
	void connect(const NLNET::CInetHost &addr); // const CInetAddress &addr);

	/// Shutdown and close gracefully, this object can be reused immediately for a new connection even if non-blocking
	void disconnect(bool blocking = false);

	/// Update connection state
	void update();

	/// Release QUIC resources asynchronously (during update), library can be safely reused even after this call
	void release();

	/// Check if still connecting or connected
	TState state() const;

	/// Check the maximum datagram length
	uint32 maxSendLength() const;

	/// Check if the connection is in a limbo state
	inline bool limbo() const
	{
		const TState s = state();
		return s == Connecting || s == Disconnecting;
	}

	/// Check if we can send
	inline bool canSend() const
	{
		return state() == Connected && maxSendLength() > 0;
	}

	/// Check if the connection is connected
	inline bool connected() const { return state() == Connected; }

	/// Send a datagram, fancier than a telegram, but not as reliable
	// bool sendDatagram(const uint8 *buffer, uint32 size);

	/// Send a datagram, this swaps the buffer with the previous one sent
	/// Only one datagram may be in flight at a time
	bool sendDatagramSwap(NLMISC::CBitMemStream &buffer, uint32 size);

	/// Check if any datagram has been received
	bool datagramAvailable() const;

	/// Receive a datagram
	bool receiveDatagram(NLMISC::CBitMemStream &msgin);

	/// Check if quick is supported
	bool isSupported() const;

private:
	friend CQuicConnectionImpl;

	/// Internal implementation specific
	CUniquePtr<CQuicConnectionImpl> m;
};

#endif /* NL_QUIC_TRANSCEIVER_H */

/* end of file */
