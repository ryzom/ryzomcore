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
#include "quic_transceiver.h"

#include "nel/misc/mutex.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/string_view.h"

#include "config.h"
#include "quic_selfsign.h"

#ifdef NL_MSQUIC_AVAILABLE
#include <msquic.h>

#ifdef NL_OS_WINDOWS
#pragma warning(push)
#pragma warning(disable : 6553) // Annotation does not apply to value type.
#include <wincrypt.h>
#pragma warning(pop)
#endif

#define MsQuic m->Api
#define null nullptr

using namespace NLMISC;
using namespace NLNET;

// This implements QUIC connection support between the Ryzom Core client and server.

// It simply runs the existing UDP-based protocol over QUIC datagrams
// i.e. we're just treating QUIC as an alternate transport.

// In the future we can use QUIC's stream support to add dedicated reliable channels
// for messages from IOS (chat, etc.) and DDS (Ryzom Ring scenario editor),
// as well as for the initial database sync message at login,
// which should improve the login experience for players
// and speed up loading of NPC dialogs.

// Advantages of using QUIC over adding a TCP connection on the side:
// - Only a single point of failure and disconnection.
// - Connection migration reduces chance of getting disconnected,
//   e.g. when switching on or off a VPN, and now we know who you are.
// - Encryption, so nobody can find out you're actually playing Ryzom.

namespace /* anonymous */ {

// This really hammers fast
class CAtomicFlagLock
{
public:
	CAtomicFlagLock(std::atomic_flag &flag)
	    : m_Flag(flag)
	{
		while (m_Flag.test_and_set(std::memory_order_acquire))
			;
	}

	~CAtomicFlagLock()
	{
		m_Flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag &m_Flag;
};

// This is a bit more relaxed
class CAtomicFlagLockYield
{
public:
	CAtomicFlagLockYield(std::atomic_flag &flag)
	    : m_Flag(flag)
	{
		while (m_Flag.test_and_set(std::memory_order_acquire))
			std::this_thread::yield();
	}

	~CAtomicFlagLockYield()
	{
		m_Flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag &m_Flag;
};

} /* anonymous namespace */

class CQuicTransceiverImpl
{
public:
	const QUIC_API_TABLE *Api = null;
	HQUIC Registration = null;
	HQUIC Configuration = null;
	HQUIC Listener = null;

	std::atomic_flag BufferMutex = ATOMIC_FLAG_INIT;
	NLMISC::CBufFIFO *Buffer = null;

	std::atomic<bool> Listening;

	// Some salt for generating the token address
	uint64 SaltA0 = (((uint64)std::random_device()()) << 32) | std::random_device()();
	uint64 SaltA1 = (((uint64)std::random_device()()) << 32) | std::random_device()();
	uint32 SaltB0 = std::random_device()();
	uint32 SaltB1 = std::random_device()();
	std::atomic<uint64> AddrA;
	std::atomic<uint32> AddrB;

	// IPv6 unique local address range to use as a token address for each connection.
	// This is used for compatibility with the existing UDP-based protocol handling
	// and the existing client address to game state pointer mapping.
	CInetAddress TokenSubnet = CInetAddress("[fdd5:d66b:8698::]:0");

	// Listener stop event wait
	std::mutex ListenerStopMutex;
	std::condition_variable ListenerStopCondition;

	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_LISTENER_CALLBACK)
#endif
	        listenerCallback(HQUIC listener, void *context, QUIC_LISTENER_EVENT *ev);
	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
	        connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev);
};

CQuicTransceiver::CQuicTransceiver(uint32 msgsize)
    : m(std::make_unique<CQuicTransceiverImpl>())
    , m_MsgSize(msgsize)
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
	regConfig.AppName = "Ryzom Core (FES)";
	regConfig.ExecutionProfile = QUIC_EXECUTION_PROFILE_LOW_LATENCY;
	status = MsQuic->RegistrationOpen(&regConfig, &m->Registration);
	if (QUIC_FAILED(status))
	{
		nlwarning("MsQuic->RegistrationOpen failed with status 0x%x", status);
		release();
		return;
	}
}

CQuicTransceiver::~CQuicTransceiver()
{
	stop();
	release();
}

void CQuicTransceiver::start(uint16 port)
{
	stop();

	if (!MsQuic)
	{
		nlwarning("QUIC API not available");
	}

	static const CStringView protocolName = "ryzomcore4";
	static const QUIC_BUFFER alpn = { (uint32)protocolName.size(), (uint8 *)protocolName.data() };

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
			nlwarning("MsQuic->ConfigurationOpen failed with status 0x%x", status);
			return;
		}

		// TODO: Certificate should depend on a configuration variable, if it's configured, at least for production

		// Programmatically create a self signed certificate, only valid in Windows
		// This is very useful for development servers
		uint8 certHash[20];
		PCCERT_CONTEXT schannelCert = (PCCERT_CONTEXT)FES_findOrCreateSelfSignedCertificate(certHash);
		if (schannelCert)
		{
			// Server credentials
			QUIC_CREDENTIAL_CONFIG credConfig;
			memset(&credConfig, 0, sizeof(credConfig));
			credConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT;
			credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
			credConfig.CertificateContext = (QUIC_CERTIFICATE *)schannelCert;
			status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
			if (QUIC_FAILED(status))
			{
				nlwarning("MsQuic->ConfigurationLoadCredential failed with status 0x%x", status);
				FES_freeSelfSignedCertificate((void *)schannelCert);
				schannelCert = nullptr;
			}
		}
		else
		{
#ifdef NL_OS_WINDOWS
			nlwarning("Failed to create self-signed certificate");
#endif
		}
		if (schannelCert)
		{
			// Don't need the certificate anymore (I guess? Let's hope it's been copied by MsQuic!)
			nlinfo("Self-signed certificate hash: %s", NLMISC::toHexa(certHash, 20).c_str());
			FES_freeSelfSignedCertificate((void *)schannelCert);
			schannelCert = nullptr;
		}
		else
		{
			// Either we could not create a self-signed certificate on Windows, or could not load it into the configuration
			// Try with an OpenSSL certificate
			// TODO
			nlwarning("Self signed OpenSSL certificates are not yet supported, QUIC will not work. Specify a certificate in the configuration file.");
			MsQuic->ConfigurationClose(m->Configuration);
			m->Configuration = nullptr;
			return;
		}
	}

	// Open listener listening using MSQUIC on e.g. [::]:5000 (port)
	QUIC_ADDR addr = { 0 };
	QuicAddrSetFamily(&addr, QUIC_ADDRESS_FAMILY_UNSPEC);
	QuicAddrSetPort(&addr, port);
	status = MsQuic->ListenerOpen(m->Registration, CQuicTransceiverImpl::listenerCallback, this, &m->Listener);
	if (QUIC_FAILED(status))
	{
		stop();
		nlwarning("MsQuic->ListenerOpen failed with status 0x%x", status);
		return;
	}

	// Start listening
	status = MsQuic->ListenerStart(m->Listener, &alpn, 1, &addr);
	if (QUIC_FAILED(status))
	{
		stop();
		nlwarning("MsQuic->ListenerStart failed with status 0x%x", status);
		return;
	}

	// Ok
	m->Listening.store(true, std::memory_order_release);
	nlinfo("Listening for QUIC connections on port %i", (int)port);
}

void CQuicTransceiver::stop()
{
	// Stop listening
	if (m->Listener)
	{
		if (m->Listening)
		{
			MsQuic->ListenerStop(m->Listener);
			try
			{
				// Wait for stop
				std::unique_lock<std::mutex> lock(m->ListenerStopMutex);
				m->ListenerStopCondition.wait(lock, [this] { return !m->Listening; });
			}
			catch (const std::exception &e)
			{
				nlwarning("Exception while waiting for listener stop: %s", e.what());
			}
		}
		MsQuic->ListenerClose(m->Listener);
		m->Listener = null;
	}
}

void CQuicTransceiver::release()
{
	// Close configuration
	if (m->Configuration)
	{
		MsQuic->ConfigurationClose(m->Configuration);
		m->Configuration = null;
	}

	// Close registration
	if (m->Registration)
	{
		MsQuic->RegistrationClose(m->Registration);
		m->Registration = null;
	}

	// Close library
	if (MsQuic)
	{
		MsQuicClose(MsQuic);
		MsQuic = null;
	}
}

CQuicUserContext::CQuicUserContext()
{
}

CQuicUserContext::~CQuicUserContext()
{
	// This should never get called before the connection is shutdown,
	// since we increase the reference when the connection gets opened,
	// and decrease the reference when the connection is shutdown.
	CQuicTransceiverImpl *m = Transceiver->m.get();
	if (Connection) // Should always be set, this is already warned for in the shutdown handling.
	{
		MsQuic->ConnectionClose((HQUIC)Connection);
		Connection = null;
	}
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_LISTENER_CALLBACK)
#endif
    CQuicTransceiverImpl::listenerCallback(HQUIC listener, void *context, QUIC_LISTENER_EVENT *ev)
{
	CQuicTransceiver *self = (CQuicTransceiver *)context;
	CQuicTransceiverImpl *m = self->m.get();
	QUIC_STATUS status = QUIC_STATUS_NOT_SUPPORTED;
	switch (ev->Type)
	{
	case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
		// Create user context
		CQuicUserContext *user = new CQuicUserContext();
		user->Transceiver = self;
		user->TokenAddr = self->generateTokenAddr(); // ev->NEW_CONNECTION.Info->RemoteAddress // Could change on migration, so don't expose it for now (OK in the future)
		user->Connection = ev->NEW_CONNECTION.Connection;
		user->increaseRef();
		// They're in.
		MsQuic->SetCallbackHandler(ev->NEW_CONNECTION.Connection, CQuicTransceiverImpl::connectionCallback, (void *)user);
		status = MsQuic->ConnectionSetConfiguration(ev->NEW_CONNECTION.Connection, m->Configuration);
		nlwarning("New QUIC connection");
		break;
	}
	case QUIC_LISTENER_EVENT_STOP_COMPLETE: {
		nlwarning("QUIC listener stopped");
		std::unique_lock<std::mutex> lock(m->ListenerStopMutex);
		m->Listening.store(false, std::memory_order_release);
		m->ListenerStopCondition.notify_all();
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	default: {
		nlwarning("Unknown event type %d", ev->Type);
		break;
	}
	}
	return status;
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_CONNECTION_CALLBACK)
#endif
    CQuicTransceiverImpl::connectionCallback(HQUIC connection, void *context, QUIC_CONNECTION_EVENT *ev)
{
	CQuicUserContext *user = (CQuicUserContext *)context;
	CQuicTransceiver *self = user->Transceiver;
	CQuicTransceiverImpl *m = self->m.get();
	QUIC_STATUS status = QUIC_STATUS_NOT_SUPPORTED;
	switch (ev->Type)
	{
	case QUIC_CONNECTION_EVENT_CONNECTED:
		nlinfo("Connected over QUIC protocol with ALPN '%s'", nlsvc(CStringView((const char *)ev->CONNECTED.NegotiatedAlpn, ev->CONNECTED.NegotiatedAlpnLength)));
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
		user->MaxSendLength = 0;
		nlinfo("Shutdown initiated by transport");
		self->shutdownReceived(user);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
		user->MaxSendLength = 0;
		nlinfo("Shutdown initiated by peer");
		self->shutdownReceived(user);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
		CQuicUserContextRelease releaseUser(user); // Hopefully we only get QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE once!
		user->MaxSendLength = 0;
		nlinfo("Shutdown complete");
		if (ev->SHUTDOWN_COMPLETE.AppCloseInProgress)
		{
			// Only applicable on client when ConnectionClose is called on the connection, so warn if this happens
			nlwarning("App close in progress flagged, connection was deleted before the context. This should never happen.");
			// Should never happen, but just in case, set to null to avoid double deletion
			user->Connection = null;
		}
		// TODO: Report to the boss (forge a disconnect datagram from this user)
		// It's detected through the datagram handling already, anyway
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
		nldebug("Datagram received");
		// YES PLEASE
		self->datagramReceived(user, ev->DATAGRAM_RECEIVED.Buffer->Buffer, ev->DATAGRAM_RECEIVED.Buffer->Length);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
		nlinfo("Datagram state changed");
		user->MaxSendLength = ev->DATAGRAM_STATE_CHANGED.SendEnabled ? ev->DATAGRAM_STATE_CHANGED.MaxSendLength : 0;
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
		if (ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_SENT
		    && (ptrdiff_t)ev->DATAGRAM_SEND_STATE_CHANGED.ClientContext == (ptrdiff_t)user->SentCount.load())
		{
			user->SendBusy.clear(); // release
		}
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_LOCAL_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_PEER_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
	case QUIC_CONNECTION_EVENT_RESUMED:
	case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
	case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE: // TODO: Match with msg.xml
	case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
		// Don't care
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
	case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS:
	default:
		// Not supported
		nlwarning("Unsupported QUIC connection event type");
		break;
	}
	return status;
}

CInetAddress CQuicTransceiver::generateTokenAddr()
{
	// This generates a random address from a deterministic pseudo-random sequence
	uint64 addrA = m->AddrA++;
	uint32 addrB = m->AddrB++;
	addrA = NLMISC::wangHash64(addrA ^ m->SaltA0) ^ m->SaltA1;
	addrB = NLMISC::wangHash(addrB ^ m->SaltB0) ^ m->SaltB1;
	sockaddr_in6 sa;
	m->TokenSubnet.toSockAddrInet6(&sa);
	// Keep the first 6 bytes of sa.sin6_addr.s6_addr
	// Then 2 bytes from addrB
	// Then 8 bytes from addrA
	memcpy(&sa.sin6_addr.s6_addr[6], &addrB, 2);
	memcpy(&sa.sin6_addr.s6_addr[8], &addrA, 8);
	// And the remaining 2 bytes from addrB are to set the port
	sa.sin6_port = htons(((uint16 *)&addrB)[1]);
	CInetAddress res(false);
	res.fromSockAddrInet6(&sa);
	return res;
}

bool CQuicTransceiver::listening()
{
	return m->Listening.load(std::memory_order_acquire);
}

void CQuicTransceiver::datagramReceived(CQuicUserContext *user, const uint8 *buffer, uint32 length)
{
	// Increase reference for FIFO copy
	user->increaseRef();

	// Locked block
	{
		CAtomicFlagLockYield lock(m->BufferMutex);
		static const uint8 userEvent = TReceivedMessage::User;
		static_assert(MsgHeaderSize == sizeof(userEvent));
		m->Buffer->push(&userEvent, MsgHeaderSize, buffer, length);
		m->Buffer->push((uint8 *)&user, sizeof(user)); // Pointer
	}
}

void CQuicTransceiver::shutdownReceived(CQuicUserContext *user)
{
	// Increase reference for FIFO copy
	user->increaseRef();

	// Locked block
	{
		CAtomicFlagLockYield lock(m->BufferMutex);
		static const uint8 removeEvent = TReceivedMessage::RemoveClient;
		static_assert(MsgHeaderSize == sizeof(removeEvent));
		m->Buffer->push(&removeEvent, MsgHeaderSize);
		m->Buffer->push((uint8 *)&user, sizeof(user)); // Pointer
	}
}

NLMISC::CBufFIFO *CQuicTransceiver::swapWriteQueue(NLMISC::CBufFIFO *writeQueue)
{
	CAtomicFlagLockYield lock(m->BufferMutex);
	CBufFIFO *previous = m->Buffer;
	m->Buffer = writeQueue;
	return previous;
}

// void CQuicTransceiver::sendDatagram(CQuicUserContext *user, const uint8 *buffer, uint32 size)
//{
//	QUIC_BUFFER *buf = new QUIC_BUFFER(); // wow leak :)
//	uint8 *copy = new uint8[size];
//	memcpy(copy, buffer, size);
//	buf->Buffer = copy; // (uint8 *)buffer;
//	buf->Length = size;
//	QUIC_STATUS status = MsQuic->DatagramSend((HQUIC)user->Connection, buf, 1, QUIC_SEND_FLAG_NONE, (void *)user);
//	if (QUIC_FAILED(status))
//	{
//		nlwarning("MsQuic->ConnectionSendDatagram failed with status %d", status);
//	}
// }

bool CQuicTransceiver::sendDatagramSwap(CQuicUserContext *user, NLMISC::CBitMemStream &buffer)
{
	if (buffer.size() <= user->MaxSendLength.load())
	{
		if (user->SendBusy.testAndSet())
		{
			// Already busy
			return false;
		}

		// Swap buffers
		++user->SentCount;
		user->SendBuffer.swap(buffer);
		static_assert(sizeof(CQuicBuffer) == sizeof(QUIC_BUFFER));
		static_assert(offsetof(CQuicBuffer, Buffer) == offsetof(QUIC_BUFFER, Buffer));
		static_assert(offsetof(CQuicBuffer, Length) == offsetof(QUIC_BUFFER, Length));
		user->SendQuicBuffer.Buffer = (uint8 *)user->SendBuffer.buffer();
		user->SendQuicBuffer.Length = user->SendBuffer.length();
		QUIC_STATUS status = MsQuic->DatagramSend((HQUIC)user->Connection, (QUIC_BUFFER *)(&user->SendQuicBuffer), 1, QUIC_SEND_FLAG_NONE, (void *)(ptrdiff_t)user->SentCount.load());
		if (QUIC_FAILED(status))
		{
			user->SendBusy.clear();
			nlwarning("DatagramSend failed with %d", status);
			return false;
		}
		return true;
	}
	return false;
}

void CQuicTransceiver::shutdown(CQuicUserContext *user)
{
	// Gracefully shuts down a connection (assume this is called from the main service thread only)
	// Assuming we can call shutdown as many times as we like...
	CQuicTransceiverImpl *m = user->Transceiver->m.get();
	MsQuic->ConnectionShutdown((HQUIC)user->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
}

#else

#define null nullptr

using namespace NLMISC;
using namespace NLNET;

class CQuicTransceiverImpl
{
public:
	NLMISC::CBufFIFO *Buffer = null;
};

CQuicTransceiver::CQuicTransceiver(uint32 msgsize)
    : m(std::make_unique<CQuicTransceiverImpl>())
    , m_MsgSize(msgsize)
{
}

CQuicTransceiver::~CQuicTransceiver()
{
}

void CQuicTransceiver::start(uint16 port)
{
}

void CQuicTransceiver::stop()
{
}

CQuicUserContext::CQuicUserContext()
{
}

CQuicUserContext::~CQuicUserContext()
{
}

CInetAddress CQuicTransceiver::generateTokenAddr()
{
	return CInetAddress(false);
}

bool CQuicTransceiver::listening()
{
	return false;
}

void CQuicTransceiver::datagramReceived(CQuicUserContext *user, const uint8 *buffer, uint32 length)
{
	// LOCK
	// m->Buffer->push(buffer, length);
	// m->Buffer->push(user->VTokenAddr);
}

NLMISC::CBufFIFO *CQuicTransceiver::swapWriteQueue(NLMISC::CBufFIFO *writeQueue)
{
	// LOCK
	CBufFIFO *previous = m->Buffer;
	m->Buffer = writeQueue;
	return previous;
}

void CQuicTransceiver::sendDatagram(CQuicUserContext *user, const uint8 *buffer, uint32 size)
{
}

void CQuicTransceiver::shutdown(CQuicUserContext *user)
{
}

#endif

/* end of file */
