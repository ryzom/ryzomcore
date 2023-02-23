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

#include "stdpch.h"
#include "quic_connection.h"

#include "nel/misc/mutex.h"
#include "nel/misc/atomic.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/string_view.h"
#include "nel/misc/string_common.h"

#include "config.h"

#ifdef NL_MSQUIC_AVAILABLE
#include <msquic.h>

#ifdef NL_OS_WINDOWS
#pragma warning(push)
#pragma warning(disable : 6553) // Annotation does not apply to value type.
#include <wincrypt.h>
#pragma warning(pop)
#endif

#define MsQuic m->Api

using namespace NLMISC;
using namespace NLNET;

class CQuicConnectionImpl
{
public:
	CQuicConnectionImpl()
	    : Api(NULL)
	    , Registration(NULL)
	    , Configuration(NULL)
	    , State(CQuicConnection::Disconnected)
	    , ShutdownFlag(false)
	    , MaxSendLength(NULL)
	{
	}

	const QUIC_API_TABLE *Api;
	HQUIC Registration;
	HQUIC Configuration;
	HQUIC Connection;
	
	bool ConnectingAddrSet;
	NLNET::CInetHost ConnectingAddr;

	CMutex BufferMutex;
	NLMISC::CBufFIFO Buffer;

	CMutex StateMutex;
	CQuicConnection::TState State;

	CMutex ShutdownMutex; // Don't have fancy atomics and synchronization primitives here, so mutex everything!
	bool ShutdownFlag;

	uint32 MaxSendLength;


	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
	        connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev);
};

CQuicConnection::CQuicConnection()
#ifdef NL_CPP14
    : m(std::make_unique<CQuicConnectionImpl>())
#else
    : m(new CQuicConnectionImpl())
#endif
{
	// Open library
	QUIC_STATUS status = MsQuicOpenVersion(QUIC_API_VERSION_2, (const void **)&MsQuic);
	if (QUIC_FAILED(status))
	{
		nlwarning("MsQuicOpenVersion failed with status 0x%x", status);
		return;
	}

	// Registration, this creates the worker threads
	QUIC_REGISTRATION_CONFIG regConfig = { 0 };
	regConfig.AppName = "Ryzom Core (Client)";
	regConfig.ExecutionProfile = QUIC_EXECUTION_PROFILE_LOW_LATENCY;
	status = MsQuic->RegistrationOpen(&regConfig, &m->Registration);
	if (QUIC_FAILED(status))
	{
		nlwarning("MsQuic->RegistrationOpen failed with status 0x%x", status);
		release();
		return;
	}
}

CQuicConnection::~CQuicConnection()
{
	disconnect(true);
	release();
}

void CQuicConnection::connect(const NLNET::CInetHost &addr)
{
	disconnect(false);

	if (!MsQuic)
	{
		nlwarning("QUIC API not available");
	}

	if (!addr.isValid())
	{
		return;
	}

	{
		CUnlockableAutoMutex<CMutex> lock(m->StateMutex);
		if (m->State != Disconnected)
		{
			lock.unlock();
			m->ConnectingAddr = addr;
			m->ConnectingAddrSet = true;
			return; // Try again in update()
		}
		m->State = Connecting;
	}

	static const char *protocolName = "ryzomcore4";
	static const QUIC_BUFFER alpn = { sizeof(protocolName) - 1, (uint8_t *)protocolName };

	// Configuration, initialized in start, but destroyed on release only (may attempt more than once)
	QUIC_STATUS status = QUIC_STATUS_SUCCESS;
	if (!m->Configuration)
	{
		QUIC_SETTINGS settings = { 0 };
		settings.DatagramReceiveEnabled = TRUE;
		settings.IsSet.DatagramReceiveEnabled = TRUE;
		settings.MigrationEnabled = TRUE;
		settings.IsSet.MigrationEnabled = TRUE;
		settings.PeerBidiStreamCount = 0;
		settings.IsSet.PeerBidiStreamCount = TRUE;
		settings.PeerUnidiStreamCount = 0; // TODO: Configured from msg.xml
		settings.IsSet.PeerUnidiStreamCount = TRUE;
		// settings.SendBufferingEnabled = TRUE;
		// settings.IsSet.SendBufferingEnabled = TRUE;
		// settings.GreaseQuicBitEnabled = TRUE;
		// settings.IsSet.GreaseQuicBitEnabled = TRUE;
		// settings.MinimumMtu = m_MsgSize + size of QUIC header; // Probably violates QUIC protocol if we do this, also no need
		// settings.IsSet.MinimumMtu = TRUE;
		status = MsQuic->ConfigurationOpen(m->Registration, &alpn, 1, &settings, sizeof(settings), NULL, &m->Configuration);
		if (QUIC_FAILED(status))
		{
			m->ShutdownFlag = true;
			disconnect(false);
			nlwarning("MsQuic->ConfigurationOpen failed with status 0x%x", status);
			return;
		}

		// Load credentials for client, client doesn't need a certificate
		QUIC_CREDENTIAL_CONFIG credConfig;
		memset(&credConfig, 0, sizeof(credConfig));
		credConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT;
		credConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
		status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
		if (QUIC_FAILED(status))
		{
			m->ShutdownFlag = true;
			disconnect(false);
			MsQuic->ConfigurationClose(m->Configuration);
			m->Configuration = nullptr;
			nlwarning("MsQuic->ConfigurationLoadCredential failed with status 0x%x", status);
			return;
		}
	}

	// Connect to the specified address
	status = MsQuic->ConnectionOpen(m->Registration, CQuicConnectionImpl::connectionCallback, (void *)this, &m->Connection);
	if (QUIC_FAILED(status))
	{
		m->ShutdownFlag = true;
		disconnect(false);
		nlwarning("MsQuic->ConnectionOpen failed with status 0x%x", status);
		return;
	}

	// Start the connection
	status = MsQuic->ConnectionStart(m->Connection, m->Configuration, QUIC_ADDRESS_FAMILY_UNSPEC, nlUtf8ToMbcs(addr.hostname()), addr.port());
	if (QUIC_FAILED(status))
	{
		m->ShutdownFlag = true;
		disconnect(false);
		nlwarning("MsQuic->ConnectionStart to %s failed with status 0x%x", addr.toStringLong().c_str(), status);
		return;
	}

	// Check
	update();
}

void CQuicConnection::disconnect(bool blocking)
{
	// Stop connection
	if (m->Connection)
	{
		MsQuic->ConnectionShutdown(m->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
		if (blocking)
		{
			try
			{
				for (;;) // Spin wait because we don't have fancy mechanisms when supporting legacy code base
				{
					CAutoMutex<CMutex> lock(m->ShutdownMutex);
					if (m->ShutdownFlag)
						break;
					nlSleep(1);
				}
			}
			catch (const std::exception &e)
			{
				nlwarning("Exception while waiting for connection shutdown: %s", e.what());
			}
		}
	}
	m->ConnectingAddr.clear();
	m->ConnectingAddrSet = false;

	// Check
	update();
}

void CQuicConnection::update()
{
	bool shutdownFlag;
	{
		CAutoMutex<CMutex> lock(m->ShutdownMutex);
		shutdownFlag = m->ShutdownFlag;
	}
	if (shutdownFlag)
	{
		{
			CAutoMutex<CMutex> lock(m->StateMutex);
			m->State = Disconnected;
		}
		if (m->Connection)
		{
			MsQuic->ConnectionClose(m->Connection);
			m->Connection = NULL;
		}
		{
			CAutoMutex<CMutex> lock(m->ShutdownMutex);
			shutdownFlag = false;
		}
	}
	if (m->ConnectingAddrSet)
	{
		CInetHost addr = m->ConnectingAddr;
		m->ConnectingAddr.clear();
		m->ConnectingAddrSet = false;
		connect(addr);
	}
}

void CQuicConnection::release()
{
	// Close configuration
	if (m->Configuration)
	{
		MsQuic->ConfigurationClose(m->Configuration);
		m->Configuration = NULL;
	}

	// Close registration
	if (m->Registration)
	{
		MsQuic->RegistrationClose(m->Registration);
		m->Registration = NULL;
	}

	// Close library
	if (MsQuic)
	{
		MsQuicClose(MsQuic);
		MsQuic = NULL;
	}
}

CQuicConnection::TState CQuicConnection::state() const
{
	CAutoMutex<CMutex> lock(m->StateMutex);
	return m->State;
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
    CQuicConnectionImpl::connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev)
{
	CQuicConnection *self = (CQuicConnection *)context;
	CQuicConnectionImpl *m = self->m.get();
	QUIC_STATUS status = QUIC_STATUS_NOT_SUPPORTED;
	switch (ev->Type)
	{
	case QUIC_CONNECTION_EVENT_CONNECTED: {
		nlinfo("Connected");
		nlassert(CStringView((const char *)ev->CONNECTED.NegotiatedAlpn, ev->CONNECTED.NegotiatedAlpnLength) == "ryzomcore4");
		// MsQuic->ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL); // What does this even do?
		{
			CAutoMutex<CMutex> lock(m->StateMutex);
			m->State = CQuicConnection::Connected;
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT: {
		nlinfo("Shutdown initiated by transport");
		{
			CAutoMutex<CMutex> lock(m->StateMutex);
			m->State = CQuicConnection::Disconnecting;
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER: {
		nlinfo("Shutdown initiated by peer");
		{
			CAutoMutex<CMutex> lock(m->StateMutex);
			m->State = CQuicConnection::Disconnecting;
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
		nlinfo("Shutdown complete");
		{
			CAutoMutex<CMutex> lock(m->StateMutex);
			m->State = CQuicConnection::Disconnecting;
		}
		{
			CAutoMutex<CMutex> lock(m->ShutdownMutex);
			m->ShutdownFlag = true;
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
		nlinfo("Datagram received");
		// YES PLEASE
		self->datagramReceived(ev->DATAGRAM_RECEIVED.Buffer->Buffer, ev->DATAGRAM_RECEIVED.Buffer->Length);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
		nlinfo("Datagram state changed");
		m->MaxSendLength = ev->DATAGRAM_STATE_CHANGED.SendEnabled ? ev->DATAGRAM_STATE_CHANGED.MaxSendLength : 0;
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_LOCAL_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_PEER_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
	case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
	case QUIC_CONNECTION_EVENT_RESUMED:
	case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
	case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE: // TODO: Match with msg.xml
		// Don't care
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
	case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS:
	case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
		// Not supported
		break;
	}
	return status;
}

void CQuicConnection::sendDatagram(const uint8 *buffer, uint32 size)
{
	if (m->Connection)
	{
		QUIC_BUFFER buf;
		buf.Buffer = (uint8 *)buffer;
		buf.Length = size;
		MsQuic->DatagramSend(m->Connection, &buf, 1, QUIC_SEND_FLAG_NONE, this);
	}
}

bool CQuicConnection::datagramAvailable()
{
	CAutoMutex<CMutex> lock(m->BufferMutex);
	return !m->Buffer.empty();
}

bool CQuicConnection::receiveDatagram(NLMISC::CBitMemStream &msgin)
{
	CAutoMutex<CMutex> lock(m->BufferMutex);
	if (!m->Buffer.empty())
		return false;
	uint8 *buffer;
	uint32 size;
	m->Buffer.front(buffer, size);
	msgin.clear();
	memcpy(msgin.bufferToFill(size), buffer, size);
	return true;
}

void CQuicConnection::datagramReceived(const uint8 *buffer, uint32 length)
{
	CAutoMutex<CMutex> lock(m->BufferMutex);
	m->Buffer.push(buffer, length);
}

#else

using namespace NLMISC;
using namespace NLNET;

class CQuicConnectionImpl
{
public:
	int Dummy;
};

CQuicConnection::CQuicConnection()
#ifdef NL_CPP14
    : m(std::make_unique<CQuicConnectionImpl>())
#else
    : m(new CQuicConnectionImpl())
#endif
{
}

CQuicConnection::~CQuicConnection()
{
}

void CQuicConnection::connect(const NLNET::CInetHost &addr)
{
}

void CQuicConnection::disconnect(bool blocking)
{
}

void CQuicConnection::release()
{
}

CQuicConnection::TState CQuicConnection::state() const
{
	return Disconnected;
}

void CQuicConnection::sendDatagram(const uint8 *buffer, uint32 size)
{
}

bool CQuicConnection::datagramAvailable()
{
	return false;
}

bool CQuicConnection::receiveDatagram(NLMISC::CBitMemStream &msgin)
{
	return false;
}

void CQuicConnection::datagramReceived(const uint8 *buffer, uint32 length)
{
}

#endif

/* end of file */
