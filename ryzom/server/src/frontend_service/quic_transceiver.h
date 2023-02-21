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

#include "nel/net/inet_address.h"

#include "fe_receive_task.h"

class CQuicTransceiverImpl;

struct CQuicUserContext
{
	CQuicTransceiver *Transceiver;

	// Not a real address, just a token to identify the connection
	// Give everyone an IPv6 in unique local network fdd5:d66b:8698::/48
	// The addresses are a sequential hash sequence, so should be unique for a very very long time
	// They should not be used for security purposes
	CInetAddress TokenAddr;
	std::vector<uint8> VTokenAddr;
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

private:
	friend CQuicTransceiverImpl;
	
	/// Internal implementation specific
	std::unique_ptr<CQuicTransceiverImpl> m;

	/// User configuration
	uint32 m_MsgSize;

	/// Received datagram
	void datagramReceived(const CQuicUserContext *user, const uint8 *buffer, uint32 length);

	/// Generates a token address to identify the connection with existing code
	CInetAddress generateTokenAddr();

	// public:
	//	/// Constructor
	//	CFEReceiveTask(uint16 firstAcceptablePort, uint16 lastAcceptablePort, uint32 msgsize);
	//
	//	/// Destructor
	//	~CFEReceiveTask();
	//
	//	/// Run
	//	virtual void run();
	//
	//	/// Set new write queue (thread-safe because mutexed)
	//	void setWriteQueue(NLMISC::CBufFIFO *writequeue);
	//
	//	/// Require exit (thread-safe because atomic assignment)
	//	void requireExit() { _ExitRequired = true; }
	//
	//	/// Return the number of rejected datagrams since the last call (thread-safe because atomic assignment)
	//	uint nbNewRejectedDatagrams()
	//	{
	//		uint nb = _NbRejectedDatagrams;
	//		_NbRejectedDatagrams = 0;
	//		return nb;
	//	}
	//
	// private:
	//	/// Datagram length
	//	uint _DatagramLength;
	//
	//	/// Received message
	//	TReceivedMessage _ReceivedMessage;
	//
	//	/// Write queue access
	//	NLMISC::CSynchronized<NLMISC::CBufFIFO *> _WriteQueue;
	//
	//	/// Number of datagrams not copied because too big
	//	volatile uint _NbRejectedDatagrams;
	//
	//	/// Exit required
	//	volatile bool _ExitRequired;
	//
	// public:
	//	/// External datagram socket
	//	NLNET::CUdpSock *DataSock;
	//
	//	/// The date of the last UPD packet recevied
	//	static volatile uint32 LastUDPPacketReceived;
};

#endif /* NL_QUIC_TRANSCEIVER_H */

/* end of file */
