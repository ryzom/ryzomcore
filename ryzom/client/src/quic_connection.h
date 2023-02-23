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
		Connected
	};

	CQuicConnection();
	~CQuicConnection();

	/// Connect
	void connect(const NLNET::CInetHost &addr); // const CInetAddress &addr);

	/// Shutdown and close gracefully, this object can be reused immediately for a new connection
	void disconnect();

	/// Release. Instance is useless after this call
	void release();

	/// Check if still connecting or connected
	TState state() const;

	/// Send a datagram, fancier than a telegram, but not as reliable
	void sendDatagram(const uint8 *buffer, uint32 size);

	/// Check if any datagram has been received
	bool datagramAvailable();

	/// Receive a datagram
	bool receiveDatagram(NLMISC::CBitMemStream &msgin);

private:
	friend CQuicConnectionImpl;

	/// Internal implementation specific
	CUniquePtr<CQuicConnectionImpl> m;

	/// Received datagram
	void datagramReceived(const uint8 *buffer, uint32 length);
};

#endif /* NL_QUIC_TRANSCEIVER_H */

/* end of file */
