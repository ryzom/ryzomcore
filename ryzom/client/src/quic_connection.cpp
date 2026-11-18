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
#include "client_cfg.h"

#include "game_share/stream_record.h"

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
using namespace CLFECOMMON;

namespace /* anonymous */ {

// Protocol feature levels, corresponds to Ryzom Core release versions, keep datagram-only support to give users and server owners the option to keep bandwidth restricted
// 4.1: Only datagram support (restricted bandwidth, same behaviour as UDP)
// 4.2: Adds unidirectional streams from the server to the client, one per class of messages the server has (see game_share/stream_record.h), for the long and the ordered ones (keep bandwidth from client restricted)
// 4.3?: Add a single bidirectional stream opened by the client for the scenario editor gateway (more efficient MODULE_GATEWAY replacement)
const CStringView s_ProtocolName42 = "ryzomcore/4.2";
const CStringView s_ProtocolName41 = "ryzomcore/4.1";

/// What the client lets the server have in flight on one stream. MsQuic gives credit back as the data is read, which
/// the stream callback does at once, and raises the window by itself for a server that fills it.
const uint32 s_StreamRecvWindow = 256 * 1024; // (a power of two)

/// Name and code of a QUIC status, for logs and error messages
std::string quicStatusText(QUIC_STATUS status)
{
	// Compared one by one, as some statuses are not constant expressions (HRESULT_FROM_WIN32) and some share a value on some platforms
#define CL_QUIC_STATUS_NAME(s) \
	if (status == (s)) return toString("%s (0x%x)", #s, (uint32)status);
	CL_QUIC_STATUS_NAME(QUIC_STATUS_OUT_OF_MEMORY)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_INVALID_PARAMETER)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_INVALID_STATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_NOT_SUPPORTED)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_HANDSHAKE_FAILURE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_ABORTED)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_INVALID_ADDRESS)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CONNECTION_TIMEOUT)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CONNECTION_IDLE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_UNREACHABLE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_INTERNAL_ERROR)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CONNECTION_REFUSED)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_PROTOCOL_ERROR)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_VER_NEG_ERROR)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_TLS_ERROR)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_USER_CANCELED)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_ALPN_NEG_FAILURE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CLOSE_NOTIFY)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_BAD_CERTIFICATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_UNSUPPORTED_CERTIFICATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_REVOKED_CERTIFICATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_EXPIRED_CERTIFICATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_UNKNOWN_CERTIFICATE)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_REQUIRED_CERTIFICATE)
#ifdef QUIC_STATUS_CERT_EXPIRED
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CERT_EXPIRED)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CERT_UNTRUSTED_ROOT)
	CL_QUIC_STATUS_NAME(QUIC_STATUS_CERT_NO_CERT)
#endif
#undef CL_QUIC_STATUS_NAME
	uint32 alert = (uint32)status - (uint32)QUIC_STATUS_TLS_ALERT(0);
	if (alert < 256)
		return toString("TLS alert %u (0x%x)", alert, (uint32)status);
	return toString("0x%x", (uint32)status);
}

/// What a QUIC status that ended a connection attempt means for the user
CQuicConnection::TFailure quicStatusFailure(QUIC_STATUS status)
{
	if (status == QUIC_STATUS_CONNECTION_IDLE // The handshake timed out
	    || status == QUIC_STATUS_CONNECTION_TIMEOUT)
		return CQuicConnection::NoResponse;
	if (status == QUIC_STATUS_UNREACHABLE) // ICMP unreachable
		return CQuicConnection::Unreachable;
	if (status == QUIC_STATUS_CONNECTION_REFUSED)
		return CQuicConnection::Refused;
	if (status == QUIC_STATUS_ALPN_NEG_FAILURE
	    || status == QUIC_STATUS_VER_NEG_ERROR)
		return CQuicConnection::ProtocolMismatch;
#ifdef QUIC_STATUS_CERT_EXPIRED
	if (status == QUIC_STATUS_CERT_EXPIRED
	    || status == QUIC_STATUS_CERT_UNTRUSTED_ROOT
	    || status == QUIC_STATUS_CERT_NO_CERT)
		return CQuicConnection::CertificateRejected;
#endif
	// A certificate that fails validation ends the handshake with a TLS alert
	uint32 alert = (uint32)status - (uint32)QUIC_STATUS_TLS_ALERT(0);
	switch (alert)
	{
	case 42: // bad_certificate
	case 43: // unsupported_certificate
	case 44: // certificate_revoked
	case 45: // certificate_expired
	case 46: // certificate_unknown
	case 48: // unknown_ca
	case 116: // certificate_required
		return CQuicConnection::CertificateRejected;
	case 120: // no_application_protocol
		return CQuicConnection::ProtocolMismatch;
	default:
		break;
	}
#ifdef NL_OS_WINDOWS
	// Certificate chain errors (CERT_E_*) from the Windows certificate validation
	if (((uint32)status & 0xFFFF0000) == 0x800B0000)
		return CQuicConnection::CertificateRejected;
#endif
	return CQuicConnection::OtherFailure;
}

} /* anonymous namespace */

class CQuicConnectionImpl
{
public:
	// The desired state change
	enum TDesiredStateChange
	{
		NoChange,
		Connect,
		Disconnect,
		Release,
	};

	CQuicConnectionImpl(CQuicConnection *self)
	    : Self(self)
	    , Api(NULL)
	    , Registration(NULL)
	    , Configuration(NULL)
	    , Connection(NULL)
	    , DesiredStateChange(NoChange)
	    , Buffer("CQuicConnectionImpl::Buffer")
		, BufferWrites(0)
	    , BufferReads(0)
	    , State(CQuicConnection::Disconnected)
		, ConnectedFlag(false)
		, ShuttingDownFlag(false)
		, ShutdownFlag(false)
	    , ReceiveEnable(0)
		, MaxSendLength(0)
		, SendBusy(false)
		, SentCount(0)
		, Failure(CQuicConnection::NoFailure)
		, Established(false)
		, TransportStatus(0)
		, PeerShutdown(false)
		, StreamsNegotiated(false)
	{
		for (uint i = 0; i < StreamClassMax; ++i)
			Streams[i].Impl = this;
	}

	CQuicConnection *Self;
	const QUIC_API_TABLE *Api;
	HQUIC Registration;
	HQUIC Configuration;
	HQUIC Connection;

	// This value is set by connect and disconnect,
	// and checked from the update call. (main thread only)
	TDesiredStateChange DesiredStateChange;
	CInetHost DesiredAddr;

	// This is the FIFO buffer for all datagrams that are received.
	// It is filled by the receive callback (any thread), and emptied by the receiveDatagram call (main thread only).
	CSynchronizedFIFO Buffer; // (any thread)
	CAtomicInt BufferWrites; // (any thread)
	int BufferReads; // If BufferReads == BufferWrites, then there's no more datagrams (main thread only)

	// Current state of the connection
	CQuicConnection::TState State; // Updated by the update call (main thread only)
	CAtomicFlag ConnectedFlag; // Set by the callback thread, checked by update (main)
	CAtomicFlag ShuttingDownFlag; // Set by the callback thread, checked by the update call (main thread), reset by update on complete disconnection
	CAtomicFlag ShutdownFlag; // Set by the callback thread, checked by the update call (main thread), reset by update on complete disconnection

	bool ReceiveEnable; // Set when datagrams may be received, otherwise they'll be ignored, this is to avoid stray messages after disconnecting (set and accessed by main thread only)
	CAtomicInt MaxSendLength; // Set by the callback thread, used to check if we can send

	// Send buffer, one being sent at a time, released as soon as it's sent out
	CAtomicFlag SendBusy;
	CBitMemStream SendBuffer;
	QUIC_BUFFER SendQuicBuffer;
	CAtomicInt SentCount;

	// Why the last connection ended before it was established (main thread only)
	CQuicConnection::TFailure Failure;
	std::string FailureText;
	CInetHost Address; // Address of the last connection attempt (main thread only)
	bool Established; // Set once the connection reached the Connected state (main thread only)
	CAtomicInt TransportStatus; // Status of a shutdown initiated by the transport, set by the callback thread
	CAtomicBool PeerShutdown; // Set by the callback thread when the server shuts the connection down

	// One of the streams the server can open (protocol level ryzomcore/4.2)
	struct CStream
	{
		CStream()
		    : Impl(NULL)
		    , IncomingWrites(0)
		    , IncomingReads(0)
		    , Ended(false)
		    , PendingBoundaries(0)
		    , Failed(false)
		{
		}

		CQuicConnectionImpl *Impl; // (immutable)

		// The bytes received and not yet taken by the main thread (under StreamMutex).
		// Filled by the stream callback (any thread), emptied by receiveStreamRecord (main thread only).
		std::vector<uint8> Incoming;
		CAtomicInt IncomingWrites; // (any thread)
		int IncomingReads; // If IncomingReads == IncomingWrites, Incoming has nothing new (main thread only)
		CAtomicBool Ended; // Set by the callback thread when the server ends the stream, which it never does

		CStreamRecordReader Reader; // The bytes taken, cut into records (main thread only)
		uint32 PendingBoundaries; // Sessions that ended whose mark has not been read yet (main thread only)
		bool Failed; // The stream cannot be read any further (main thread only)
	};
	CStream Streams[StreamClassMax]; // by stream number, whether or not the server has opened it
	CMutex StreamMutex;
	CAtomicBool StreamsNegotiated; // Set by the callback thread when the connection is of protocol level 4.2

	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
	        connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev);
	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_STREAM_CALLBACK)
#endif
	        streamCallback(HQUIC stream, void *context, QUIC_STREAM_EVENT *ev);

	void init(); // Delayed init
	void update();
	void connect();
	void shutdown();
	void close();
	void release();
	void clearStreams(); // Nothing received, no session ended (main thread only)

	/// Record why the connection could not be established (main thread only)
	void setFailure(CQuicConnection::TFailure failure, const std::string &text)
	{
		Failure = failure;
		FailureText = text;
	}

	/// Received datagram (from quic threads)
	void datagramReceived(const uint8 *buffer, uint32 length);
};

CQuicConnection::CQuicConnection()
#ifdef NL_CPP14
    : m(std::make_unique<CQuicConnectionImpl>(this))
#else
    : m(new CQuicConnectionImpl(this))
#endif
{
}

CQuicConnection::~CQuicConnection()
{
	disconnect(true);
	release();
}

void CQuicConnectionImpl::init()
{
	CQuicConnectionImpl *const m = this;

	if (MsQuic)
	{
		return;
	}

	// TestAndSet has acquire semantics, clear has release semantics, so we use clear to flag the events
	m->ConnectedFlag.testAndSet();
	m->ShuttingDownFlag.testAndSet();
	m->ShutdownFlag.testAndSet();
	m->SendBusy.clear(); // release order

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

void CQuicConnection::connect(const NLNET::CInetHost &addr)
{
	m->DesiredStateChange = CQuicConnectionImpl::Connect;
	m->DesiredAddr = addr;
	update();
}

void CQuicConnectionImpl::connect()
{
	CQuicConnectionImpl *const m = this;

	init();

	nlassert(m->State == CQuicConnection::Disconnected);
	nlassert(m->DesiredStateChange == Connect);

	NLNET::CInetHost addr = m->DesiredAddr;

	// Reset state
	m->DesiredStateChange = NoChange;
	m->ConnectedFlag.testAndSet();
	m->ShuttingDownFlag.testAndSet();
	m->ShutdownFlag.testAndSet();
	m->MaxSendLength = 0;
	m->SendBusy.clear(); // Each connection starts with nothing in flight, the previous one is closed
	m->BufferReads = m->BufferWrites;
	m->ReceiveEnable = true;
	m->setFailure(CQuicConnection::NoFailure, std::string());
	m->Address = addr;
	m->Established = false;
	m->TransportStatus.store(QUIC_STATUS_SUCCESS);
	m->PeerShutdown.store(false);

	// Clear FIFO
	{
		CFifoAccessor access(&m->Buffer);
		CBufFIFO *fifo = &access.value();
		fifo->clear();
	}

	// A new connection has new streams
	m->StreamsNegotiated.store(false);
	clearStreams();

	if (!MsQuic)
	{
		nlwarning("QUIC API not available");
		m->setFailure(CQuicConnection::OtherFailure, "QUIC library not available");
		return;
	}

	if (!addr.isValid())
	{
		m->setFailure(CQuicConnection::OtherFailure, "invalid address");
		return;
	}

	// The protocol levels offered (see s_ProtocolName42), of which the server takes the first one of its own list.
	// A level is a configured behaviour: a connection of level 4.1 gets everything in datagrams, as before.
	static const QUIC_BUFFER alpn[2] = {
		{ (uint32)s_ProtocolName42.size(), (uint8 *)s_ProtocolName42.data() },
		{ (uint32)s_ProtocolName41.size(), (uint8 *)s_ProtocolName41.data() },
	};

	// Configuration, initialized in start, but destroyed on release only (may attempt more than once)
	QUIC_STATUS status = QUIC_STATUS_SUCCESS;
	if (!m->Configuration)
	{
		const bool offerStreams = ClientCfg.QuicStreams;
		QUIC_SETTINGS settings = { 0 };
		settings.DatagramReceiveEnabled = TRUE;
		settings.IsSet.DatagramReceiveEnabled = TRUE;
		settings.MigrationEnabled = TRUE;
		settings.IsSet.MigrationEnabled = TRUE;
		settings.PeerBidiStreamCount = 0;
		settings.IsSet.PeerBidiStreamCount = TRUE;
		// How many streams a server of level 4.2 may open is a constant of the level; how many it opens and what for
		// is its own choice. A server that opens more ends the connection (MsQuic refuses the stream).
		settings.PeerUnidiStreamCount = offerStreams ? (uint16)StreamClassMax : 0;
		settings.IsSet.PeerUnidiStreamCount = TRUE;
		if (offerStreams)
		{
			settings.StreamRecvWindowUnidiDefault = s_StreamRecvWindow;
			settings.IsSet.StreamRecvWindowUnidiDefault = TRUE;
		}
		// settings.SendBufferingEnabled = TRUE;
		// settings.IsSet.SendBufferingEnabled = TRUE;
		// settings.GreaseQuicBitEnabled = TRUE;
		// settings.IsSet.GreaseQuicBitEnabled = TRUE;
		settings.MinimumMtu = 576;
		settings.IsSet.MinimumMtu = TRUE;
		status = MsQuic->ConfigurationOpen(m->Registration, offerStreams ? &alpn[0] : &alpn[1], offerStreams ? 2 : 1, &settings, sizeof(settings), NULL, &m->Configuration);
		if (QUIC_FAILED(status))
		{
			close();
			nlwarning("MsQuic->ConfigurationOpen failed with status 0x%x", status);
			m->setFailure(CQuicConnection::OtherFailure, "ConfigurationOpen: " + quicStatusText(status));
			return;
		}

		// Load credentials for client, client doesn't need a certificate
		QUIC_CREDENTIAL_CONFIG credConfig;
		memset(&credConfig, 0, sizeof(credConfig));
		credConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT;
		if (!ClientCfg.QuicCertValidation)
		{
			credConfig.Flags |= QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION; // Don't care for development
		}
		credConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
		status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
		if (QUIC_FAILED(status))
		{
			close();
			MsQuic->ConfigurationClose(m->Configuration);
			m->Configuration = nullptr;
			nlwarning("MsQuic->ConfigurationLoadCredential failed with status 0x%x", status);
			m->setFailure(CQuicConnection::OtherFailure, "ConfigurationLoadCredential: " + quicStatusText(status));
			return;
		}
	}

	// Connect to the specified address
	status = MsQuic->ConnectionOpen(m->Registration, CQuicConnectionImpl::connectionCallback, (void *)this, &m->Connection);
	if (QUIC_FAILED(status))
	{
		close();
		nlwarning("MsQuic->ConnectionOpen failed with status 0x%x", status);
		m->setFailure(CQuicConnection::OtherFailure, "ConnectionOpen: " + quicStatusText(status));
		return;
	}

	// Start the connection (NOTE: The hostname lookup is a bit redundant here, but it's okay I suppose...)
	status = MsQuic->ConnectionStart(m->Connection, m->Configuration, QUIC_ADDRESS_FAMILY_UNSPEC, nlUtf8ToMbcs(addr.hostname()), addr.port());
	if (QUIC_FAILED(status))
	{
		close();
		nlwarning("MsQuic->ConnectionStart to %s failed with status 0x%x", addr.toStringLong().c_str(), status);
		m->setFailure(CQuicConnection::OtherFailure, "ConnectionStart: " + quicStatusText(status));
		return;
	}

	// Check
	m->State = CQuicConnection::Connecting;
	update();
}

void CQuicConnection::disconnect(bool blocking)
{
	m->DesiredStateChange = CQuicConnectionImpl::Disconnect;
	m->ReceiveEnable = false;
	if (MsQuic && blocking)
	{
		try
		{
			while (m->State != CQuicConnection::Disconnected)
			{
				nlSleep(1);
				update();
			}
			return;
		}
		catch (const std::exception &e)
		{
			m->close(); // Not very safe safety fallback
			if (INelContext::isContextInitialised())
			{
				nlwarning("Exception while waiting for connection shutdown: %s", e.what());
			}
		}
	}
}

void CQuicConnectionImpl::shutdown()
{
	CQuicConnectionImpl *const m = this;
	if (m->Connection)
	{
		MsQuic->ConnectionShutdown(m->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
	}
	update();
}

void CQuicConnectionImpl::close()
{
	CQuicConnectionImpl *const m = this;
	if (m->Connection)
	{
		MsQuic->ConnectionClose(m->Connection);
		m->Connection = NULL;
	}
}

// All the state updates and switches are done here
void CQuicConnectionImpl::update()
{
	CQuicConnectionImpl *const m = this;

	// Asynchronous release
	if (!m->Connection
	    && m->DesiredStateChange == CQuicConnectionImpl::Release)
	{
		nlassert(m->State == CQuicConnection::Disconnected);
		m->release();
		m->DesiredStateChange = CQuicConnectionImpl::NoChange;
		update();
		return;
	}

	// Early exit
	if (m->State == CQuicConnection::Disconnected
	    && m->DesiredStateChange != CQuicConnectionImpl::Connect)
	{
		nlassert(!m->Connection);
		return;
	}

	// Simple case, initial state, user wants to connect and we're disconnected
	if (m->DesiredStateChange == Connect
	    && m->State == CQuicConnection::Disconnected)
	{
		nlassert(!m->Connection);
		connect();
		return;
	}

	// Also simple case, a handshake still in progress is abandoned the same way rather than waited out
	if (m->DesiredStateChange != NoChange // Disconnect or Connect (reconnecting)
	    && (m->State == CQuicConnection::Connected || m->State == CQuicConnection::Connecting))
	{
		m->State = CQuicConnection::Disconnecting;
		shutdown();
		return;
	}

	// Connected!
	if (!m->ConnectedFlag.testAndSet())
	{
		if (m->State == CQuicConnection::Connecting)
		{
			m->State = CQuicConnection::Connected;
			m->Established = true;
			// We're good!
		}
		else if (m->State != CQuicConnection::Disconnecting) // The handshake may complete while it is being abandoned
		{
			nlwarning("Connection is connected, but state is %d", m->State);
		}
		update();
		return;
	}

	// Shutting down, this is just a limbo state
	if (!m->ShuttingDownFlag.testAndSet())
	{
		if (m->State == CQuicConnection::Connected
		    || m->State == CQuicConnection::Connecting)
		{
			m->State = CQuicConnection::Disconnecting;
		}
		update();
		return;
	}

	// Shutdown complete, now close!
	if (!m->ShutdownFlag.testAndSet())
	{
		nlassert(m->State != CQuicConnection::Disconnected);
		nlassert(m->Connection);
		close();
		if (!m->Established)
		{
			// Ended before it was established, which is no failure if it was abandoned on request
			QUIC_STATUS transportStatus = (QUIC_STATUS)m->TransportStatus.load();
			if (m->PeerShutdown.load())
				m->setFailure(CQuicConnection::Refused, "shut down by the server");
			else if (transportStatus != QUIC_STATUS_SUCCESS)
				m->setFailure(quicStatusFailure(transportStatus), quicStatusText(transportStatus));
			if (m->Failure != CQuicConnection::NoFailure)
				nlwarning("QUIC connection to %s could not be established: %s", m->Address.toStringLong().c_str(), m->FailureText.c_str());
		}
		m->State = CQuicConnection::Disconnected;
		update();
		return;
	}
}

void CQuicConnection::update()
{
	m->update();
}

void CQuicConnectionImpl::release()
{
	CQuicConnectionImpl *const m = this;

	// Clear FIFO
	{
		CFifoAccessor access(&m->Buffer);
		CBufFIFO *fifo = &access.value();
		fifo->clear();
	}

	m->SendBuffer.clear();
	clearStreams();

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

void CQuicConnection::release()
{
	m->DesiredStateChange = CQuicConnectionImpl::Release;
	m->ReceiveEnable = false;
	update();
}

void CQuicConnectionImpl::clearStreams()
{
	CQuicConnectionImpl *const m = this;
	CAutoMutex<> lock(m->StreamMutex);
	for (uint i = 0; i < StreamClassMax; ++i)
	{
		CStream &stream = m->Streams[i];
		std::vector<uint8>().swap(stream.Incoming);
		stream.IncomingReads = stream.IncomingWrites;
		stream.Ended.store(false);
		stream.Reader.clear();
		stream.PendingBoundaries = 0;
		stream.Failed = false;
	}
}

CQuicConnection::TState CQuicConnection::state() const
{
	return m->State;
}

uint32 CQuicConnection::maxSendLength() const
{
	return m->MaxSendLength;
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
    CQuicConnectionImpl::connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev)
{
	CQuicConnectionImpl *const m = (CQuicConnectionImpl *)context;
	QUIC_STATUS status = QUIC_STATUS_NOT_SUPPORTED;
	switch (ev->Type)
	{
	case QUIC_CONNECTION_EVENT_CONNECTED: {
		CStringView negotiatedAlpn((const char *)ev->CONNECTED.NegotiatedAlpn, ev->CONNECTED.NegotiatedAlpnLength);
		m->StreamsNegotiated.store(negotiatedAlpn == s_ProtocolName42); // Before the flag, which releases it
		m->ConnectedFlag.clear();
		nlinfo("Connected over QUIC protocol with ALPN '%s'", nlsvc(negotiatedAlpn));
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT: {
		m->TransportStatus.store((int)ev->SHUTDOWN_INITIATED_BY_TRANSPORT.Status); // Before the flag, which releases it
		m->ShuttingDownFlag.clear();
		m->MaxSendLength = 0;
		if (INelContext::isContextInitialised())
		{
			nlwarning("Shutdown initiated by transport, error code %" NL_I64 "u, status %s", (uint64)ev->SHUTDOWN_INITIATED_BY_TRANSPORT.ErrorCode, quicStatusText(ev->SHUTDOWN_INITIATED_BY_TRANSPORT.Status).c_str());
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER: {
		if (INelContext::isContextInitialised())
		{
			nlinfo("Shutdown initiated by peer, error code %" NL_I64 "u", (uint64)ev->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
		}
		m->PeerShutdown.store(true); // Before the flag, which releases it
		m->ShuttingDownFlag.clear();
		m->MaxSendLength = 0;
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
		if (INelContext::isContextInitialised())
		{
			nlinfo("Shutdown complete");
		}
		m->ShutdownFlag.clear();
		m->MaxSendLength = 0;
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
		// nldebug("Datagram received");
		// YES PLEASE
		m->datagramReceived(ev->DATAGRAM_RECEIVED.Buffer->Buffer, ev->DATAGRAM_RECEIVED.Buffer->Length);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
		m->MaxSendLength = ev->DATAGRAM_STATE_CHANGED.SendEnabled ? ev->DATAGRAM_STATE_CHANGED.MaxSendLength : 0;
		if (INelContext::isContextInitialised())
		{
			nlinfo("Datagram state changed, max send length is now %i", (int)m->MaxSendLength);
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
		// Canceled when it was still queued at shutdown, or no longer fits, and then it is never sent
		if ((ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_SENT
		        || ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_CANCELED)
		    && (ptrdiff_t)ev->DATAGRAM_SEND_STATE_CHANGED.ClientContext == (ptrdiff_t)m->SentCount.load())
		{
			m->SendBusy.clear(); // release
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_LOCAL_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_PEER_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
	case QUIC_CONNECTION_EVENT_RESUMED:
	case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
	case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE: // The client opens no stream
	case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
		// Don't care
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED: {
		// The server opens a stream for a class of its messages. The stream id gives the number of the stream.
		HQUIC stream = ev->PEER_STREAM_STARTED.Stream;
		QUIC_UINT62 streamId = 0;
		uint32 streamIdSize = sizeof(streamId);
		uint8 streamNumber = NoStreamClass;
		if ((ev->PEER_STREAM_STARTED.Flags & QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL)
		    && QUIC_SUCCEEDED(MsQuic->GetParam(stream, QUIC_PARAM_STREAM_ID, &streamIdSize, &streamId)))
		{
			streamNumber = streamClassOfId(streamId);
		}
		if (streamNumber == NoStreamClass)
		{
			// MsQuic lets the server open no other stream than those the client allows, so this is not reached.
			// The stream is refused, MsQuic closes it.
			if (INelContext::isContextInitialised())
			{
				nlwarning("The server opened QUIC stream %" NL_I64 "u, which is none of the streams of messages", (uint64)streamId);
			}
			break;
		}
		MsQuic->SetCallbackHandler(stream, (void *)CQuicConnectionImpl::streamCallback, (void *)&m->Streams[streamNumber]);
		if (INelContext::isContextInitialised())
		{
			nlinfo("The server opened QUIC stream %u for messages", (uint)streamNumber);
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS:
	default:
		// Not supported
		nlwarning("Unsupported QUIC connection event type");
		break;
	}
	return status;
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_STREAM_CALLBACK)
#endif
    CQuicConnectionImpl::streamCallback(HQUIC stream, void *context, QUIC_STREAM_EVENT *ev)
{
	CStream &s = *(CStream *)context;
	CQuicConnectionImpl *const m = s.Impl;
	switch (ev->Type)
	{
	case QUIC_STREAM_EVENT_RECEIVE: {
		{
			CAutoMutex<> lock(m->StreamMutex);
			for (uint32 i = 0; i < ev->RECEIVE.BufferCount; ++i)
			{
				const QUIC_BUFFER &buffer = ev->RECEIVE.Buffers[i];
				s.Incoming.insert(s.Incoming.end(), buffer.Buffer, buffer.Buffer + buffer.Length);
			}
		}
		if (ev->RECEIVE.Flags & QUIC_RECEIVE_FLAG_FIN)
			s.Ended.store(true); // Before the count, which releases it
		++s.IncomingWrites;
		// All of it is taken: MsQuic gives the server the credit back
		return QUIC_STATUS_SUCCESS;
	}
	case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
	case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
		// The server keeps its streams open for as long as the connection: without one of them, messages are missing
		s.Ended.store(true); // Before the count, which releases it
		++s.IncomingWrites;
		return QUIC_STATUS_SUCCESS;
	case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
		// The stream was accepted, so its handle is the client's to close, which is allowed from this event
		MsQuic->StreamClose(stream);
		return QUIC_STATUS_SUCCESS;
	default:
		return QUIC_STATUS_SUCCESS;
	}
}

// bool CQuicConnection::sendDatagram(const uint8 *buffer, uint32 size)
//{
//	if (m->Connection && m->State && CQuicConnection::Connected && size <= m->MaxSendLength.load())
//	{
//		QUIC_BUFFER *buf = new QUIC_BUFFER(); // wow leak :)
//		uint8 *copy = new uint8[size];
//		memcpy(copy, buffer, size);
//		buf->Buffer = copy; // (uint8 *)buffer;
//		buf->Length = size;
//		QUIC_STATUS status = MsQuic->DatagramSend(m->Connection, buf, 1, QUIC_SEND_FLAG_NONE, this);
//		if (QUIC_FAILED(status))
//		{
//			nlwarning("DatagramSend failed with %d", status);
//			return false;
//		}
//		return true;
//	}
//	return false;
// }

bool CQuicConnection::sendDatagramSwap(NLMISC::CBitMemStream &buffer, uint32 size)
{
	if (m->Connection && m->State == CQuicConnection::Connected && size <= m->MaxSendLength.load() && size <= buffer.length())
	{
		if (m->SendBusy.testAndSet())
		{
			// Already busy
			return false;
		}

		// Swap buffers
		++m->SentCount;
		m->SendBuffer.swap(buffer);
		m->SendQuicBuffer.Buffer = (uint8 *)m->SendBuffer.buffer();
		m->SendQuicBuffer.Length = size;
		QUIC_STATUS status = MsQuic->DatagramSend(m->Connection, &m->SendQuicBuffer, 1, QUIC_SEND_FLAG_NONE, (void *)(ptrdiff_t)m->SentCount.load());
		if (QUIC_FAILED(status))
		{
			m->SendBusy.clear();
			nlwarning("DatagramSend failed with %d", status);
			return false;
		}
		return true;
	}
	return false;
}

bool CQuicConnection::datagramAvailable() const
{
	// CFifoAccessor access(&m->Buffer);
	// return !access.value().empty();
	return m->ReceiveEnable && m->BufferWrites != m->BufferReads;
}

bool CQuicConnection::receiveDatagram(NLMISC::CBitMemStream &msgin)
{
	if (!m->ReceiveEnable)
		return false;

	int writes = m->BufferWrites;
	int reads = m->BufferReads;
	if (writes != reads)
	{
		CFifoAccessor access(&m->Buffer); // This block is mutex'd now
		CBufFIFO *fifo = &access.value();
		if (fifo->empty())
		{
			m->BufferReads = writes; // Update value (not a real counter)
			return false;
		}
		uint8 *buffer;
		uint32 size;
		fifo->front(buffer, size);
		msgin.clear();
		memcpy(msgin.bufferToFill(size), buffer, size);
		fifo->pop();
		if (fifo->empty())
		{
			m->BufferReads = writes; // Update value (not a real counter)
		}
		return true;
	}
	return false;
}

bool CQuicConnection::streams() const
{
	return m->StreamsNegotiated.load();
}

void CQuicConnection::endStreamSession()
{
	// The server marks the end in every stream it has, whenever it opens them (the others never carry anything)
	for (uint i = 0; i < StreamClassMax; ++i)
		++m->Streams[i].PendingBoundaries;
}

CQuicConnection::TStreamRecord CQuicConnection::receiveStreamRecord(uint streamNumber, NLMISC::CBitMemStream &msgin)
{
	nlassert(streamNumber < StreamClassMax);
	CQuicConnectionImpl::CStream &s = m->Streams[streamNumber];

	if (!m->ReceiveEnable)
		return NoStreamRecord;
	if (s.Failed)
		return StreamError;

	// Take what has arrived (the end of the stream is looked at first: what came before it is then taken below)
	const bool ended = s.Ended.load();
	int writes = s.IncomingWrites;
	if (ended || writes != s.IncomingReads)
	{
		CAutoMutex<> lock(m->StreamMutex);
		if (!s.Incoming.empty())
		{
			s.Reader.push(&s.Incoming[0], (uint32)s.Incoming.size());
			if (s.Incoming.capacity() > 1024 * 1024)
				std::vector<uint8>().swap(s.Incoming); // the memory of a large record is given back
			else
				s.Incoming.clear();
		}
		s.IncomingReads = writes; // Update value (not a real counter)
	}

	for (;;)
	{
		const uint8 *data;
		uint32 length;
		switch (s.Reader.next(data, length, StreamRecordClientMaxBytes))
		{
		case CStreamRecordReader::Record:
			if (s.PendingBoundaries != 0)
				continue; // Of a session that has ended
			msgin.clear();
			memcpy(msgin.bufferToFill(length), data, length);
			return StreamRecord;
		case CStreamRecordReader::Boundary:
			if (s.PendingBoundaries != 0)
				--s.PendingBoundaries;
			else
				nlwarning("QUIC stream %u marks the end of a session that the client did not end", streamNumber);
			continue;
		case CStreamRecordReader::BadLength:
			s.Failed = true;
			if (length != 0)
				nlwarning("QUIC stream %u announces a message of %u bytes, more than the %u a client accepts: the stream cannot be read any further", streamNumber, length, StreamRecordClientMaxBytes);
			else
				nlwarning("QUIC stream %u has a message length that does not end: the stream cannot be read any further", streamNumber);
			return StreamError;
		case CStreamRecordReader::NoRecord:
		default:
			if (ended)
			{
				s.Failed = true;
				nlwarning("The server ended QUIC stream %u (%u bytes of an incomplete message left)", streamNumber, s.Reader.bufferedBytes());
				return StreamError;
			}
			return NoStreamRecord;
		}
	}
}

bool CQuicConnection::isSupported() const
{
	return true;
}

CQuicConnection::TFailure CQuicConnection::failure() const
{
	return m->Failure;
}

const std::string &CQuicConnection::failureText() const
{
	return m->FailureText;
}

const NLNET::CInetHost &CQuicConnection::address() const
{
	return m->Address;
}

void CQuicConnectionImpl::datagramReceived(const uint8 *buffer, uint32 length)
{
	CQuicConnectionImpl *const m = this;
	CFifoAccessor access(&m->Buffer); // This block is mutex'd now
	CBufFIFO *fifo = &access.value();
	fifo->push(buffer, length);
	++m->BufferWrites;
}

#else

using namespace NLMISC;
using namespace NLNET;

class CQuicConnectionImpl
{
public:
	int Dummy;
	std::string FailureText;
	CInetHost Address;
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

void CQuicConnection::update()
{
}

void CQuicConnection::release()
{
}

CQuicConnection::TState CQuicConnection::state() const
{
	return Disconnected;
}

uint32 CQuicConnection::maxSendLength() const
{
	return 0;
}

// bool CQuicConnection::sendDatagram(const uint8 *buffer, uint32 size)
//{
// }

bool CQuicConnection::sendDatagramSwap(NLMISC::CBitMemStream &buffer, uint32 size)
{
	return false;
}

bool CQuicConnection::datagramAvailable() const
{
	return false;
}

bool CQuicConnection::receiveDatagram(NLMISC::CBitMemStream &msgin)
{
	return false;
}

bool CQuicConnection::streams() const
{
	return false;
}

void CQuicConnection::endStreamSession()
{
}

CQuicConnection::TStreamRecord CQuicConnection::receiveStreamRecord(uint streamNumber, NLMISC::CBitMemStream &msgin)
{
	return NoStreamRecord;
}

bool CQuicConnection::isSupported() const
{
	return false;
}

CQuicConnection::TFailure CQuicConnection::failure() const
{
	return NoFailure;
}

const std::string &CQuicConnection::failureText() const
{
	return m->FailureText;
}

const NLNET::CInetHost &CQuicConnection::address() const
{
	return m->Address;
}

#endif

/* end of file */
