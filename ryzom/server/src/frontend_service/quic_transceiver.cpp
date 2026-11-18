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
#include "nel/misc/variable.h"

#include "config.h"
#include "quic_selfsign.h"

#include <algorithm>
#include <climits>
#include <condition_variable>

#ifdef NL_MSQUIC_AVAILABLE
// For the bytes in flight of a connection (QUIC_PARAM_CONN_NETWORK_STATISTICS, in MsQuic 2.5 and later), which the
// frontend uses to keep room for its datagrams in the congestion window
#define QUIC_API_ENABLE_PREVIEW_FEATURES 1
#include <msquic.h>

#ifdef NL_OS_WINDOWS
#pragma warning(push)
#pragma warning(disable : 6553) // Annotation does not apply to value type.
#include <wincrypt.h>
#pragma warning(pop)
#endif

#define MsQuic m->Api
#define null nullptr

#endif /* #ifdef NL_MSQUIC_AVAILABLE */

using namespace NLMISC;
using namespace NLNET;
using namespace CLFECOMMON;

// This implements QUIC connection support between the Ryzom Core client and server.

// It runs the existing UDP-based protocol over QUIC datagrams
// i.e. we're treating QUIC as an alternate transport.

// With protocol level ryzomcore/4.2, some classes of messages go on QUIC streams instead,
// one stream per class (see game_share/stream_record.h): reliable, in order,
// and not limited by the size of the datagram.

// Advantages of using QUIC over adding a TCP connection on the side:
// - Only a single point of failure and disconnection.
// - Connection migration reduces chance of getting disconnected,
//   e.g. when switching on or off a VPN, and now we know who you are.
// - Encryption, so nobody can find out you're actually playing Ryzom.

namespace /* anonymous */ {

CVariable<bool> QuicConnection("fs", "QuicConnection", "", true, 0, true);
CVariable<std::string> QuicCertificate("fs", "QuicCertificate", "", "", 0, true);
CVariable<std::string> QuicPrivateKey("fs", "QuicPrivateKey", "", "", 0, true);
CVariable<std::string> QuicLetsEncryptLive("fs", "QuicLetsEncryptLive", "", "/home/nevrax/letsencrypt/live", 0, true);
CVariable<bool> QuicEcn("fs", "QuicEcn", "Mark the QUIC packets sent as ECN capable (read when QUIC starts)", true, 0, true);
CVariable<bool> QuicStreams("fs", "QuicStreams", "Offer protocol level ryzomcore/4.2, which sends some classes of messages on QUIC streams (read when QUIC starts)", true, 0, true);

// Protocol feature levels, corresponds to Ryzom Core release versions, keep datagram-only support to give users and server owners the option to keep bandwidth restricted
// 4.1: Only datagram support (restricted bandwidth, same behaviour as UDP)
// 4.2: Adds unidirectional streams from the server to the client, one per class of messages the server has (see game_share/stream_record.h), for the long and the ordered ones (keep bandwidth from client restricted)
// 4.3?: Add a single bidirectional stream opened by the client for the scenario editor gateway (more efficient MODULE_GATEWAY replacement)
const CStringView s_ProtocolName42 = "ryzomcore/4.2";
const CStringView s_ProtocolName41 = "ryzomcore/4.1";

// QuicEcn as the configuration was opened with, and the closed QUIC connections that had completed their handshake, and
// those of them whose path was ECN capable when they closed (see quicEcnStats)
bool s_EcnEnabled = false;
CAtomicInt s_ClosedConnections;
CAtomicInt s_EcnCapableConnections;

#ifdef FE_DEBUG_QUIC
CAtomicInt s_UserContextCount;
#endif

// The classes of messages that go on streams (see CQuicTransceiver::setStreamClasses), read by all threads once set
std::vector<std::string> s_StreamClasses;

CSynchronized<std::vector<CQuicTransceiver *>> s_QuicTransceivers;

} /* anonymous namespace */

#ifdef NL_MSQUIC_AVAILABLE

class CQuicTransceiverImpl
{
public:
	const QUIC_API_TABLE *Api = null;
	HQUIC Registration = null;
	HQUIC Configuration = null;
	HQUIC Listener = null;

	NLMISC::CAtomicFlag BufferMutex = false;
	NLMISC::CBufFIFO *Buffer = null;

	NLMISC::CAtomicBool Listening = false;

	// Protocol level ryzomcore/4.2 is offered (QuicStreams when the configuration was opened)
	bool OfferStreams = false;

	// The library refused QUIC_PARAM_CONN_NETWORK_STATISTICS (said once)
	NLMISC::CAtomicBool NoNetworkStatistics = false;

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

	// Connections that have not shutdown yet
	NLMISC::CSynchronized<std::set<CQuicUserContextPtr>> Connections;

	// Connections whose handle is not closed yet, which includes those that have shut down while a CClientHost, a limbo
	// client, a send buffer or a queued datagram still holds their context: stop() closes them, since RegistrationClose
	// waits for every connection to be closed
	NLMISC::CSynchronized<std::set<CQuicUserContext *>> OpenConnections;

	// Take the handle out of a context, to close it once (or leave it to MsQuic); null if it was already taken
	void *takeConnection(CQuicUserContext *user);

	// Take the handles out of a context and close them: the streams, then the connection
	void closeConnection(CQuicUserContext *user);

	// Store what MsQuic says about the path of a connection (connection thread)
	void updateStreamStats(CQuicUserContext *user, HQUIC connection);

	// Open the streams of a connection, in the order of their classes (under its ConnectionMutex)
	bool openStreams(CQuicUserContext *user);

	static QUIC_STATUS
#ifdef _Function_class_
	    _Function_class_(QUIC_STREAM_CALLBACK)
#endif
	        streamCallback(HQUIC stream, void *context, QUIC_STREAM_EVENT *ev);
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
	CSynchronized<std::vector<CQuicTransceiver *>>::CAccessor transceivers(&s_QuicTransceivers);

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

	transceivers.value().push_back(this);
}

CQuicTransceiver::~CQuicTransceiver()
{
	CSynchronized<std::vector<CQuicTransceiver *>>::CAccessor transceivers(&s_QuicTransceivers);

	stop();
	release();

	transceivers.value().erase(std::find(transceivers.value().begin(), transceivers.value().end(), this));
}

void CQuicTransceiver::reloadCert()
{
	nldebug("Reload QUIC certificates [%p]", this);

	if (!MsQuic)
	{
		nldebug("QUIC API not available, skip [%p]", this);
		return;
	}

	if (!m->Configuration)
	{
		nldebug("QUIC not configured yet, skip [%p]", this);
		return;
	}

	if (!QuicCertificate.get().empty() && !QuicPrivateKey.get().empty())
	{
		// Server credentials
		nlinfo("Using certificate %s with key %s", QuicCertificate.get().c_str(), QuicPrivateKey.get().c_str());
		std::string certificate = NLMISC::utf8ToMbcs(QuicCertificate.get());
		std::string privateKey = NLMISC::utf8ToMbcs(QuicPrivateKey.get());
		QUIC_CREDENTIAL_CONFIG credConfig;
		QUIC_CERTIFICATE_FILE certFile;
		memset(&credConfig, 0, sizeof(credConfig));
		credConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
		credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
		credConfig.CertificateFile = &certFile;
		certFile.CertificateFile = certificate.c_str();
		certFile.PrivateKeyFile = privateKey.c_str();
		QUIC_STATUS status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
		if (QUIC_FAILED(status))
		{
			nlinfo("MsQuic->ConfigurationLoadCredential failed with status 0x%x", status);
		}
		else
		{
			nlinfo("QUIC certificate reloaded");
		}
	}
	else
	{
		nlinfo("No certificate configured for QUIC");
	}
}

bool CQuicTransceiver::isConfigEnabled() const
{
	return QuicConnection.get();
}

void CQuicTransceiver::start(uint16 port)
{
	stop();

	if (!QuicConnection.get())
	{
		nlinfo("QUIC listener disabled");
		return;
	}

	if (!MsQuic)
	{
		nlwarning("QUIC API not available");
		return;
	}

	nldebug("Configure QUIC at port %i", (int)port);
	// The protocol levels (see s_ProtocolName42), the preferred one first: a connection gets the first of them that its
	// client offers. A level is a configured behaviour, not a fallback: a connection of level 4.1 gets everything in
	// datagrams, and one of level 4.2 never gets a message of a stream class in a datagram.
	static const QUIC_BUFFER alpnLevels[2] = {
		{ (uint32)s_ProtocolName42.size(), (uint8 *)s_ProtocolName42.data() },
		{ (uint32)s_ProtocolName41.size(), (uint8 *)s_ProtocolName41.data() },
	};
	if (!m->Configuration)
		m->OfferStreams = QuicStreams.get(); // The listener offers what the configuration was opened with
	const QUIC_BUFFER *alpn = m->OfferStreams ? &alpnLevels[0] : &alpnLevels[1];
	const uint32 alpnCount = m->OfferStreams ? 2 : 1;

	// Configuration, initialized in start, but destroyed on release only (may attempt more than once)
	QUIC_STATUS status = QUIC_STATUS_SUCCESS;
	if (!m->Configuration) // Might need a configuration per supported version... or just the highest supported :)
	{
		QUIC_SETTINGS settings = { 0 };
		settings.DatagramReceiveEnabled = TRUE;
		settings.IsSet.DatagramReceiveEnabled = TRUE;
		settings.MigrationEnabled = TRUE;
		settings.IsSet.MigrationEnabled = TRUE;
		settings.PeerBidiStreamCount = 0;
		settings.IsSet.PeerBidiStreamCount = TRUE;
		settings.PeerUnidiStreamCount = 0; // The client opens no stream
		settings.IsSet.PeerUnidiStreamCount = TRUE;
		// Without its send buffer, MsQuic completes what was sent on a stream once the client acknowledged it, not as
		// soon as it copied it: StreamOutstanding then says what is still on its way, which the frontend keeps below what
		// the congestion window leaves beside the datagrams (see CFeSendSub::grantStreams)
		settings.SendBufferingEnabled = FALSE;
		settings.IsSet.SendBufferingEnabled = TRUE;
		// settings.GreaseQuicBitEnabled = TRUE;
		// settings.IsSet.GreaseQuicBitEnabled = TRUE;
		settings.MinimumMtu = 576;
		settings.IsSet.MinimumMtu = TRUE;
		// A bottleneck whose queue management marks (fq_codel, CAKE, PIE, L4S) then signals congestion without dropping
		// packets. MsQuic marks every new path for 3 PTOs (about 3 s before an RTT is measured, the handshake included),
		// and keeps marking only if the client's ACKs echo the marks correctly. A client or a path that does not echo
		// them then gets unmarked packets; a path that drops marked packets delays the handshake by those 3 PTOs.
		settings.EcnEnabled = QuicEcn.get();
		settings.IsSet.EcnEnabled = TRUE;
		status = MsQuic->ConfigurationOpen(m->Registration, alpn, alpnCount, &settings, sizeof(settings), NULL, &m->Configuration);
		if (QUIC_FAILED(status))
		{
			nlwarning("MsQuic->ConfigurationOpen failed with status 0x%x", status);
			return;
		}
		s_EcnEnabled = settings.EcnEnabled;

		if (QuicCertificate.get().empty() || QuicPrivateKey.get().empty())
		{
			// Attempt to auto configure certificate from well-known locations
			const NLNET::CInetHost &listenHost = NLNET::CLoginServer::getListenHost();
			std::string localHostName = listenHost.hostname();
			if (localHostName.empty() || NLNET::CIPv6Address(localHostName).isValid()) // IP address...
				localHostName = NLNET::CInetHost::localHostName();
			if (!localHostName.empty() && !NLNET::CIPv6Address(localHostName).isValid()) // If not empty and not an IP address
			{
				std::string fullchain = QuicLetsEncryptLive.get() + "/" + localHostName + "/fullchain.pem";
				std::string privkey = QuicLetsEncryptLive.get() + "/" + localHostName + "/privkey.pem";
				nldebug("Check if %s and %s exist", fullchain.c_str(), privkey.c_str());
				if (CFile::fileExists(fullchain) && CFile::fileExists(privkey))
				{
					nldebug("They exist");
					QuicCertificate.set(fullchain);
					QuicPrivateKey.set(privkey);
				}
				else
				{
					nldebug("They don't exist");
				}
			}
		}

		// Certificate depends on a configuration variable, if it's configured, at least for production
		bool liveCert = false;
		if (!QuicCertificate.get().empty() && !QuicPrivateKey.get().empty())
		{
			// Server credentials
			nlinfo("Using certificate %s with key %s", QuicCertificate.get().c_str(), QuicPrivateKey.get().c_str());
			std::string certificate = NLMISC::utf8ToMbcs(QuicCertificate.get());
			std::string privateKey = NLMISC::utf8ToMbcs(QuicPrivateKey.get());
			QUIC_CREDENTIAL_CONFIG credConfig;
			QUIC_CERTIFICATE_FILE certFile;
			memset(&credConfig, 0, sizeof(credConfig));
			credConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
			credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
			credConfig.CertificateFile = &certFile;
			certFile.CertificateFile = certificate.c_str();
			certFile.PrivateKeyFile = privateKey.c_str();
			status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
			if (QUIC_FAILED(status))
			{
				nlwarning("MsQuic->ConfigurationLoadCredential failed with status 0x%x, try a self-signed", status);
			}
			else
			{
				// TODO: Flag to reload somehow when the files change!
				liveCert = true;
			}
		}
		else
		{
			nlwarning("No certificate configured for QUIC on hostname %s, try a self-signed", NLNET::CLoginServer::getListenHost().hostname().c_str());
		}

		if (!liveCert)
		{
			// Programmatically create a self signed certificate, only valid in Windows
			// This is very useful for development servers
			uint8 certHash[20];
			void *certContext = FES_findOrCreateSelfSignedCertificate(certHash); // PCCERT_CONTEXT
			if (certContext)
			{
				// Server credentials
				QUIC_CREDENTIAL_CONFIG credConfig;
				memset(&credConfig, 0, sizeof(credConfig));
				credConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_CONTEXT;
				credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
				credConfig.CertificateContext = (QUIC_CERTIFICATE *)certContext;
				status = MsQuic->ConfigurationLoadCredential(m->Configuration, &credConfig);
				if (QUIC_FAILED(status))
				{
					nlwarning("MsQuic->ConfigurationLoadCredential failed with status 0x%x", status);
					FES_freeSelfSignedCertificate((void *)certContext);
					certContext = nullptr;
				}
			}
			else
			{
#ifdef NL_OS_WINDOWS
				nlwarning("Failed to create self-signed certificate");
#endif
			}
			if (certContext)
			{
				// Don't need the certificate anymore (I guess? Let's hope it's been copied by MsQuic!)
				nlinfo("Self-signed certificate hash: %s", NLMISC::toHexa(certHash, 20).c_str());
				FES_freeSelfSignedCertificate((void *)certContext);
				certContext = nullptr;
			}
			else
			{
				// Either we could not create a self-signed certificate on Windows, or could not load it into the configuration
				// Try with an OpenSSL certificate
				// TODO
				nlwarning("Self signed OpenSSL certificate generation is not yet supported, QUIC will not work. Specify a certificate in the configuration file");
				MsQuic->ConfigurationClose(m->Configuration);
				m->Configuration = nullptr;
				return;
			}
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
	{
		std::unique_lock<std::mutex> lock(m->ListenerStopMutex); // Lock m->Listening
		status = MsQuic->ListenerStart(m->Listener, alpn, alpnCount, &addr);
		if (!QUIC_FAILED(status))
		{
			// Ok
			m->Listening.store(true, NLMISC::TMemoryOrderRelaxed); // Released by lock
			nlinfo("Listening for QUIC connections on port %i, protocol level %s", (int)port, m->OfferStreams ? "ryzomcore/4.2 or ryzomcore/4.1" : "ryzomcore/4.1");
		}
	}

	// Failed
	if (QUIC_FAILED(status))
	{
		stop();
		nlwarning("MsQuic->ListenerStart failed with status 0x%x", status);
		return;
	}
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
				nldebug("Wait for QUIC listener stop");
				std::unique_lock<std::mutex> lock(m->ListenerStopMutex);
				m->ListenerStopCondition.wait(lock, [this] { return !m->Listening.load(NLMISC::TMemoryOrderRelaxed); /* Aqcuired by lock */ });
				nldebug("Stop wait OK");
			}
			catch (const std::exception &e)
			{
				nlwarning("Exception while waiting for listener stop: %s", e.what());
			}
		}
		nldebug("Close listener");
		MsQuic->ListenerClose(m->Listener);
		m->Listener = null;
		nldebug("Listener closed");
	}

	// TODO: Graceful shutdown.
	// First, close all listeners (close QUIC listener, and ignore new UDP addresses in the UDP receiver)
	// Then, shutdown or disconnect all existing connections (send disconnect through Ryzom datagram, then shutdown all QUIC)
	// Once all connections are gone, proceed with closing down the sockets entirely
	// If this takes more than one second, force close all QUIC connetions

	// Shutdown all connections
	for (;;) // Doesn't need to loop, but just in case...
	{
		std::set<CQuicUserContextPtr> connectionsCopy;
		{
			NLMISC::CSynchronized<std::set<CQuicUserContextPtr>>::CAccessor connections(&m->Connections);
			connectionsCopy = connections.value();
		}
		if (connectionsCopy.empty())
			break;

		nldebug("Shutdown %i connections", (int)connectionsCopy.size());
		for (const CQuicUserContextPtr &user : connectionsCopy)
		{
			shutdown(user.get());
		}
		// Wait for connections to be empty, lousy spin loop
		// (the lock is released while sleeping: SHUTDOWN_COMPLETE needs it to erase each connection)
		nldebug("Wait for shutdown");
		for (;;)
		{
			{
				NLMISC::CSynchronized<std::set<CQuicUserContextPtr>>::CAccessor connections(&m->Connections);
				if (connections.value().empty())
					break;
			}
			nlSleep(1);
		}
		nldebug("Shutdown ok");
		// Forcefully close all connections
		for (const CQuicUserContextPtr &user : connectionsCopy)
		{
			m->closeConnection(user.get());
		}
		nldebug("Closed all connections");
		connectionsCopy.clear();
		nldebug("Cleared references. There may be leftovers in CClientHost, etc");
	}

	// Close the connections that had shut down before but whose context is still held (see OpenConnections), so that
	// RegistrationClose does not wait for them, and a context released later no longer reaches this transceiver
	std::vector<void *> leftovers;
	std::vector<void *> leftoverStreams;
	{
		NLMISC::CSynchronized<std::set<CQuicUserContext *>>::CAccessor openConnections(&m->OpenConnections);
		for (CQuicUserContext *user : openConnections.value())
		{
			std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
			leftovers.push_back(user->Connection);
			user->Connection = null;
			for (void *&stream : user->StreamHandles)
			{
				if (stream)
					leftoverStreams.push_back(stream);
				stream = null;
			}
		}
		openConnections.value().clear();
	}
	nldebug("Close %i connections that had already shut down", (int)leftovers.size());
	for (void *stream : leftoverStreams)
	{
		MsQuic->StreamClose((HQUIC)stream);
	}
	for (void *connection : leftovers)
	{
		if (connection)
			MsQuic->ConnectionClose((HQUIC)connection);
	}

	// Clear queue
	nldebug("Clear current write queue");
	clearQueue(m->Buffer);
	nldebug("Cleared");
}

void CQuicTransceiver::release()
{
	// Close configuration
	nldebug("Closing configuration");
	if (m->Configuration)
	{
		MsQuic->ConfigurationClose(m->Configuration);
		m->Configuration = null;
	}
	nldebug("Configuration closed");

	// Close registration
	nldebug("Closing registration");
	if (m->Registration)
	{
		MsQuic->RegistrationClose(m->Registration);
		m->Registration = null;
	}
	nldebug("Registration closed");

	// Close library
	nldebug("Closing library");
	if (MsQuic)
	{
		MsQuicClose(MsQuic);
		MsQuic = null;
	}
	nldebug("Library closed");
}

CQuicUserContext::CQuicUserContext()
{
#ifdef FE_DEBUG_QUIC
	nldebug("Create QUIC user context, total %i", (int)(++s_UserContextCount));
#endif
	for (uint i = 0; i < StreamClassMax; ++i)
	{
		StreamContexts[i].User = this;
		StreamContexts[i].StreamClass = (uint8)i;
	}
}

CQuicUserContext::~CQuicUserContext()
{
#ifdef FE_DEBUG_QUIC
	nldebug("Destroy QUIC user context, total %i", (int)(--s_UserContextCount));
#endif

	// This should never get called before the connection is shutdown,
	// since we increase the reference when the connection gets opened,
	// and decrease the reference when the connection is shutdown.
	// Once the transceiver has stopped, it has closed the handle already and may be gone: it is not touched then.
	bool hasConnection;
	{
		// (released before takeConnection, which locks OpenConnections first)
		std::unique_lock<std::mutex> connectionLock(ConnectionMutex);
		hasConnection = Connection != null;
	}
	if (hasConnection)
	{
		// (the stream handles are closed with it: they are only set while the context has its connection)
		CQuicTransceiverImpl *m = Transceiver->m.get();
		m->closeConnection(this);
	}
}

void CQuicTransceiverImpl::closeConnection(CQuicUserContext *user)
{
	CQuicTransceiverImpl *m = this;

	// Once the connection is taken out, nobody opens or uses a stream of it any more (see sendStream)
	void *connection = takeConnection(user);
	void *streams[StreamClassMax];
	{
		std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
		for (uint i = 0; i < StreamClassMax; ++i)
		{
			streams[i] = user->StreamHandles[i];
			user->StreamHandles[i] = null;
		}
	}

	// Closing waits for the callbacks, which complete (as canceled) the blocks that were still to be sent: not under
	// the lock. The streams go before their connection.
	for (uint i = 0; i < StreamClassMax; ++i)
	{
		if (streams[i])
			MsQuic->StreamClose((HQUIC)streams[i]);
	}
	if (connection)
		MsQuic->ConnectionClose((HQUIC)connection);
}

void *CQuicTransceiverImpl::takeConnection(CQuicUserContext *user)
{
	NLMISC::CSynchronized<std::set<CQuicUserContext *>>::CAccessor openConnections(&OpenConnections);
	void *connection;
	{
		// Once taken out, the send thread and shutdown() no longer use the handle
		std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
		connection = user->Connection;
		user->Connection = null;
	}
	openConnections.value().erase(user);
	return connection;
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
		user->CallbackConnection = ev->NEW_CONNECTION.Connection;
		CQuicUserContextPtr userPtr = user;
		{
			NLMISC::CSynchronized<std::set<CQuicUserContextPtr>>::CAccessor connections(&m->Connections);
			connections.value().insert(userPtr);
		}
		{
			NLMISC::CSynchronized<std::set<CQuicUserContext *>>::CAccessor openConnections(&m->OpenConnections);
			openConnections.value().insert(user);
		}
		// user->increaseRef();
		// They're in.
		MsQuic->SetCallbackHandler(ev->NEW_CONNECTION.Connection, (void *)CQuicTransceiverImpl::connectionCallback, (void *)user);
		status = MsQuic->ConnectionSetConfiguration(ev->NEW_CONNECTION.Connection, m->Configuration);
		nldebug("New QUIC connection");
		if (QUIC_FAILED(status))
		{
			nlwarning("MsQuic->ConnectionSetConfiguration failed with status 0x%x", status);
			// Returning the failure makes MsQuic refuse the connection and take the handle back,
			// and it then never reports QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE, so the user context is released here.
			// The handle must not be closed (MsQuic asserts on that), and no further event may reach the context.
			MsQuic->SetCallbackHandler(ev->NEW_CONNECTION.Connection, null, null);
			m->takeConnection(user);
			NLMISC::CSynchronized<std::set<CQuicUserContextPtr>>::CAccessor connections(&m->Connections);
			connections.value().erase(userPtr);
		}
		break;
	}
	case QUIC_LISTENER_EVENT_STOP_COMPLETE: {
		nldebug("QUIC listener stopped");
		std::unique_lock<std::mutex> lock(m->ListenerStopMutex);
		m->Listening.store(false, NLMISC::TMemoryOrderRelaxed); // Released by lock
		m->ListenerStopCondition.notify_all();
		nldebug("QUIC listener stop notified");
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
	case QUIC_CONNECTION_EVENT_CONNECTED: {
		CStringView negotiatedAlpn((const char *)ev->CONNECTED.NegotiatedAlpn, ev->CONNECTED.NegotiatedAlpnLength);
		user->Streams = (negotiatedAlpn == s_ProtocolName42);
		nlinfo("Connected over QUIC protocol with ALPN '%s'", nlsvc(negotiatedAlpn));
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
		user->ShuttingDown = true;
		user->MaxSendLength = 0;
		nlinfo("Shutdown initiated by transport");
		self->shutdownReceived(user);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
		user->ShuttingDown = true;
		user->MaxSendLength = 0;
		nlinfo("Shutdown initiated by peer");
		self->shutdownReceived(user);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
		// CQuicUserContextRelease releaseUser(user); // Hopefully we only get QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE once!
		CQuicUserContextPtr userPtr = user;
		{
			NLMISC::CSynchronized<std::set<CQuicUserContextPtr>>::CAccessor connections(&m->Connections);
			connections.value().erase(userPtr);
		}
#ifdef FE_DEBUG_QUIC
		user->DebugRefCount = true;
#endif
		user->ShuttingDown = true;
		user->MaxSendLength = 0;
		if (ev->SHUTDOWN_COMPLETE.HandshakeCompleted)
		{
			++s_ClosedConnections;
			QUIC_STATISTICS_V2 stats = {};
			uint32 statsSize = sizeof(stats);
			QUIC_STATUS statsStatus = MsQuic->GetParam(connection, QUIC_PARAM_CONN_STATISTICS_V2, &statsSize, &stats);
			if (QUIC_SUCCEEDED(statsStatus))
			{
				if (stats.EcnCapable)
					++s_EcnCapableConnections;
				nlinfo("Shutdown complete (ECN %s, %u congestion events, %u of them from ECN)", stats.EcnCapable ? "capable" : "not capable", stats.SendCongestionCount, stats.SendEcnCongestionCount);
			}
			else
			{
				nlwarning("Shutdown complete, but the connection statistics could not be read (status 0x%x)", statsStatus);
			}
		}
		else
		{
			nlinfo("Shutdown complete, before the handshake completed");
		}
		if (ev->SHUTDOWN_COMPLETE.AppCloseInProgress)
		{
			// Only applicable on client when ConnectionClose is called on the connection, so warn if this happens
			nlwarning("App close in progress flagged, connection was deleted before the context. This should never happen.");
			// Should never happen, but just in case, take the handle out so it is not closed twice
			m->takeConnection(user);
		}
		// TODO: Report to the boss (forge a disconnect datagram from this user)
		// It's detected through the datagram handling already, anyway
		status = QUIC_STATUS_SUCCESS;
		break;
	}
	case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
		// nldebug("Datagram received");
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
		// Canceled when it was still queued at shutdown, no longer fits, or could not be written at once on a
		// connection with streams (see sendDatagramSwap), and then it is never sent
		if ((ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_SENT
		        || ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_CANCELED)
		    && (ptrdiff_t)ev->DATAGRAM_SEND_STATE_CHANGED.ClientContext == (ptrdiff_t)user->SentCount.load())
		{
			// A datagram that was not sent while the connection is up: the allowance of the streams is cut on it
			if (ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_CANCELED && user->Streams && !user->ShuttingDown)
				user->SendBusyDrops.fetchAdd(1);
			user->SendBusy.clear(); // release
		}
		// Once per datagram, while its packet is being built (which this snapshot does not count yet)
		if (ev->DATAGRAM_SEND_STATE_CHANGED.State == QUIC_DATAGRAM_SEND_SENT && user->Streams)
			m->updateStreamStats(user, connection);
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_PEER_ADDRESS_CHANGED:
		// The client is reached over another path (it changed network): the next datagrams tell it, and the allowance
		// of its streams starts over
		user->PathChangedDatagrams.store(5);
		user->PathChanged = true;
		status = QUIC_STATUS_SUCCESS;
		break;
	case QUIC_CONNECTION_EVENT_LOCAL_ADDRESS_CHANGED:
	case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
	case QUIC_CONNECTION_EVENT_RESUMED:
	case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
	case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE: // The streams are opened without asking: a client of level 4.2 allows them
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

void CQuicTransceiverImpl::updateStreamStats(CQuicUserContext *user, HQUIC connection)
{
	CQuicTransceiverImpl *m = this;

	// What the main thread took out of the queues before this point; MsQuic may not count all of it yet, which errs
	// on the careful side (read first, so that nothing taken later is thought to be counted)
	int grantedCount = user->StreamGrantedCount.load();

	// Inside a callback of the connection or of one of its streams, these are copies without a round trip to its thread.
	// The window, the round trip time and the bytes in flight; then the count of congestion events, which only the
	// older statistics have (they have the first two as well, for a library without the newer ones).
	bool networkStatistics = false;
#ifdef QUIC_PARAM_CONN_NETWORK_STATISTICS
	if (!NoNetworkStatistics.load(NLMISC::TMemoryOrderRelaxed))
	{
		QUIC_NETWORK_STATISTICS stats = {};
		uint32 statsSize = sizeof(stats);
		QUIC_STATUS status = MsQuic->GetParam(connection, QUIC_PARAM_CONN_NETWORK_STATISTICS, &statsSize, &stats);
		if (QUIC_SUCCEEDED(status))
		{
			user->StatCongestionWindow.store((int)std::min(stats.CongestionWindow, (uint32)INT_MAX), NLMISC::TMemoryOrderRelaxed);
			user->StatRtt.store((int)std::min(stats.SmoothedRTT, (uint64)INT_MAX), NLMISC::TMemoryOrderRelaxed);
			user->StatBytesInFlight.store((int)std::min(stats.BytesInFlight, (uint32)INT_MAX), NLMISC::TMemoryOrderRelaxed);
			user->StatGrantedCount.store(grantedCount, NLMISC::TMemoryOrderRelaxed);
			user->StatValid = true;
			networkStatistics = true;
		}
		// The library is older than the headers the frontend was built with (the parameter came with MsQuic 2.5)
		else if (!NoNetworkStatistics.exchange(true))
			nlwarning("This MsQuic library does not tell the bytes in flight of a connection (status 0x%x): the room the QUIC streams leave to the datagrams is worked out from what the frontend gave to the streams alone", status);
	}
#endif
	QUIC_STATISTICS_V2 stats = {};
	uint32 statsSize = sizeof(stats);
	if (QUIC_SUCCEEDED(MsQuic->GetParam(connection, QUIC_PARAM_CONN_STATISTICS_V2, &statsSize, &stats)))
	{
		user->StatCongestionEvents.store((int)(stats.SendCongestionCount + stats.SendEcnCongestionCount), NLMISC::TMemoryOrderRelaxed);
		if (networkStatistics)
			return;
		user->StatCongestionWindow.store((int)std::min(stats.SendCongestionWindow, (uint32)INT_MAX), NLMISC::TMemoryOrderRelaxed);
		user->StatRtt.store((int)std::min(stats.Rtt, (uint32)INT_MAX), NLMISC::TMemoryOrderRelaxed);
		user->StatValid = true;
	}
}

QUIC_STATUS
#ifdef _Function_class_
_Function_class_(QUIC_STREAM_CALLBACK)
#endif
    CQuicTransceiverImpl::streamCallback(HQUIC stream, void *context, QUIC_STREAM_EVENT *ev)
{
	// The context outlives the stream: it closes the handle before it goes (closeConnection)
	CQuicStreamContext *streamContext = (CQuicStreamContext *)context;
	CQuicUserContext *user = streamContext->User;
	switch (ev->Type)
	{
	case QUIC_STREAM_EVENT_START_COMPLETE:
		if (QUIC_FAILED(ev->START_COMPLETE.Status))
		{
			// For one, the client allows fewer streams than the protocol level says
			if (!user->ShuttingDown)
			{
				nlwarning("The QUIC stream of the messages of class '%s' could not be started (status 0x%x)", CQuicTransceiver::streamClassName(streamContext->StreamClass), (uint32)ev->START_COMPLETE.Status);
				user->StreamFault.store(QuicStreamOpenFailed);
			}
		}
		else if (ev->START_COMPLETE.ID != streamIdOfClass(streamContext->StreamClass))
		{
			nlwarning("The QUIC stream of the messages of class '%s' got id %" NL_I64 "u instead of %" NL_I64 "u", CQuicTransceiver::streamClassName(streamContext->StreamClass), (uint64)ev->START_COMPLETE.ID, streamIdOfClass(streamContext->StreamClass));
			user->StreamFault.store(QuicStreamWrongId);
		}
		break;
	case QUIC_STREAM_EVENT_SEND_COMPLETE: {
		// The client acknowledged the block and all before it, or the block is dropped with its stream
		CQuicStreamSend *send = (CQuicStreamSend *)ev->SEND_COMPLETE.ClientContext;
		user->StreamOutstanding.fetchAdd(-(int)send->Buffer.Length);
		if (ev->SEND_COMPLETE.Canceled && !user->ShuttingDown)
			user->StreamFault.store(QuicStreamSendCanceled);
		CQuicTransceiver::destroyStreamSend(send);
		// While the streams send, this keeps the window and the bytes in flight up to date between the datagrams, and
		// when a datagram waits (they are those of before the acknowledgement that completed the block)
		if (!ev->SEND_COMPLETE.Canceled)
			user->Transceiver->m->updateStreamStats(user, (HQUIC)user->CallbackConnection);
		break;
	}
	case QUIC_STREAM_EVENT_PEER_RECEIVE_ABORTED:
		// A client of this protocol level reads its streams to the end of the connection
		if (!user->ShuttingDown)
		{
			nlwarning("FEHACK: The client refused the QUIC stream of the messages of class '%s' (error code %" NL_I64 "u)", CQuicTransceiver::streamClassName(streamContext->StreamClass), (uint64)ev->PEER_RECEIVE_ABORTED.ErrorCode);
			user->StreamFault.store(QuicStreamAbortedByPeer);
		}
		break;
	default:
		// (the handle of a stream that has shut down is closed with its connection)
		break;
	}
	return QUIC_STATUS_SUCCESS;
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
	return m->Listening.load(NLMISC::TMemoryOrderAcquire);
}

void CQuicTransceiver::datagramReceived(CQuicUserContext *user, const uint8 *buffer, uint32 length)
{
	// Increase reference for FIFO copy
	user->increaseRef();

	// Locked block
	{
		NLMISC::CAtomicLockYield lock(m->BufferMutex);
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
		NLMISC::CAtomicLockYield lock(m->BufferMutex);
		static const uint8 removeEvent = TReceivedMessage::RemoveClient;
		static_assert(MsgHeaderSize == sizeof(removeEvent));
		m->Buffer->push(&removeEvent, MsgHeaderSize);
		m->Buffer->push((uint8 *)&user, sizeof(user)); // Pointer
	}
}

NLMISC::CBufFIFO *CQuicTransceiver::swapWriteQueue(NLMISC::CBufFIFO *writeQueue)
{
	NLMISC::CAtomicLockSpin lock(m->BufferMutex);
	CBufFIFO *previous = m->Buffer;
	m->Buffer = writeQueue;
	return previous;
}

void CQuicTransceiver::clearQueue(NLMISC::CBufFIFO *queue)
{
	NLMISC::CAtomicLockFast lock(m->BufferMutex);
	int count = 0;
	while (!queue->empty())
	{
		// Data, don't care
		queue->pop();
		nlassert(!queue->empty());
		uint8 *buffer;
		uint32 size;
		// User ptr, need to decrease ref
		queue->front(buffer, size);
		CQuicUserContext *user;
		nlassert(size == sizeof(user));
		memcpy(&user, buffer, size);
		// Decrease ref count after pop
		CQuicUserContextRelease releaseUser(user);
		queue->pop();
		++count;
	}
	nldebug("Cleared %i messages from queue [%p]", count, queue);
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
	if (buffer.length() <= user->MaxSendLength.load())
	{
		if (user->SendBusy.testAndSet())
		{
			// Already busy: the datagram of the cycle before has not left, this one is not sent
			user->SendBusyDrops.fetchAdd(1);
			return false;
		}

		// The handle may be taken out to be closed (takeConnection) while this thread sends
		std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
		if (!user->Connection)
		{
			user->SendBusy.clear();
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
		// On a connection with streams, the datagram is handled before the stream data that waits in the connection's
		// queue of work (in a packet, MsQuic puts the datagram before stream data anyway). And a datagram that cannot
		// be written at once, because the congestion window is full or the path is being validated, is dropped: it
		// would go out late, with a state that is old by then, and the datagram of the next cycle would find this one
		// still waiting and not be sent at all. The protocol of the game takes a lost datagram in its stride.
		QUIC_SEND_FLAGS flags = QUIC_SEND_FLAG_NONE;
		if (user->Streams)
		{
			flags = QUIC_SEND_FLAG_PRIORITY_WORK;
#ifdef QUIC_PARAM_CONN_NETWORK_STATISTICS // (headers of MsQuic 2.5 and later, which is where the flag below came)
			flags |= QUIC_SEND_FLAG_CANCEL_ON_BLOCKED;
#endif
		}
		QUIC_STATUS status = MsQuic->DatagramSend((HQUIC)user->Connection, (QUIC_BUFFER *)(&user->SendQuicBuffer), 1, flags, (void *)(ptrdiff_t)user->SentCount.load());
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
	// (once the transceiver has stopped, the context has no handle any more)
	std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
	if (!user->Connection)
		return;
	CQuicTransceiverImpl *m = user->Transceiver->m.get();
	user->ShuttingDown = true; // (MsQuic tells about a shutdown the frontend asks for only once it is complete)
	MsQuic->ConnectionShutdown((HQUIC)user->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
}

bool CQuicTransceiverImpl::openStreams(CQuicUserContext *user)
{
	CQuicTransceiverImpl *m = this;

	// One per class the frontend has, all at once and in the order of the classes, which gives each the stream id of
	// its class. A stream is announced to the client at once, and fails to start if the client does not allow it.
	user->StreamsOpened = true;
	const uint nbClasses = CQuicTransceiver::streamClassCount();
	for (uint i = 0; i < nbClasses; ++i)
	{
		HQUIC stream = null;
		QUIC_STATUS status = MsQuic->StreamOpen((HQUIC)user->Connection, QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, CQuicTransceiverImpl::streamCallback, (void *)&user->StreamContexts[i], &stream);
		if (QUIC_FAILED(status))
		{
			// The streams are opened on a connection that has connected, so these two say that it has closed, by the
			// client or by the transport: the event that tells so comes after
			if (status == QUIC_STATUS_ABORTED || status == QUIC_STATUS_INVALID_STATE)
				user->ShuttingDown = true;
			else if (!user->ShuttingDown)
				nlwarning("MsQuic->StreamOpen failed with status 0x%x", status);
			return false;
		}
		user->StreamHandles[i] = stream;
		status = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_IMMEDIATE | QUIC_STREAM_START_FLAG_FAIL_BLOCKED);
		if (QUIC_FAILED(status))
		{
			if (status == QUIC_STATUS_ABORTED)
				user->ShuttingDown = true;
			else if (!user->ShuttingDown)
				nlwarning("MsQuic->StreamStart failed with status 0x%x", status);
			return false;
		}
	}
	return true;
}

void CQuicTransceiver::sendStream(CQuicUserContext *user, uint8 streamClass, CQuicStreamSend *send)
{
	nlassert(streamClass < StreamClassMax);
	int fault = QuicStreamNoFault;
	{
		// The handles may be taken out to be closed (closeConnection) while this thread sends
		std::unique_lock<std::mutex> connectionLock(user->ConnectionMutex);
		if (!user->Connection)
		{
			// Closed: the block goes with the connection
		}
		else if (user->StreamFault.load() != QuicStreamNoFault)
		{
			// A block before this one was lost: the stream has a hole, the main thread disconnects the client
		}
		else if (!user->StreamsOpened && !m->openStreams(user))
		{
			fault = QuicStreamOpenFailed;
		}
		else if (!user->StreamHandles[streamClass])
		{
			fault = QuicStreamOpenFailed; // (opening failed at a stream before this one)
		}
		else
		{
			static_assert(sizeof(CQuicBuffer) == sizeof(QUIC_BUFFER));
			QUIC_STATUS status = MsQuic->StreamSend((HQUIC)user->StreamHandles[streamClass], (QUIC_BUFFER *)(&send->Buffer), 1, QUIC_SEND_FLAG_NONE, (void *)send);
			if (!QUIC_FAILED(status))
				return; // The block is MsQuic's until it completes it (streamCallback)
			// Aborted: the connection is closing, or the client refused the stream, and then the event of that has
			// stored the fault already (QuicStreamAbortedByPeer)
			if (status == QUIC_STATUS_ABORTED)
				user->ShuttingDown = true;
			else if (!user->ShuttingDown)
				nlwarning("MsQuic->StreamSend failed with status 0x%x", status);
			fault = QuicStreamSendFailed;
		}
	}

	// Not sent
	if (fault != QuicStreamNoFault && !user->ShuttingDown)
		user->StreamFault.store(fault);
	user->StreamOutstanding.fetchAdd(-(int)send->Buffer.Length);
	destroyStreamSend(send);
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

void CQuicTransceiver::reloadCert()
{
}

bool CQuicTransceiver::isConfigEnabled() const
{
	return false;
}

void CQuicTransceiver::start(uint16 port)
{
	nlwarning("QUIC not supported, no listener started");
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

void CQuicTransceiver::clearQueue(NLMISC::CBufFIFO *writeQueue)
{
}

bool CQuicTransceiver::sendDatagramSwap(CQuicUserContext *user, NLMISC::CBitMemStream &buffer)
{
	return false;
}

void CQuicTransceiver::shutdown(CQuicUserContext *user)
{
}

void CQuicTransceiver::sendStream(CQuicUserContext *user, uint8 streamClass, CQuicStreamSend *send)
{
	user->StreamOutstanding.fetchAdd(-(int)send->Buffer.Length);
	destroyStreamSend(send);
}

#endif

void CQuicTransceiver::setStreamClasses(const std::vector<std::string> &names)
{
	nlassert(names.size() <= StreamClassMax);
	s_StreamClasses = names;
}

uint CQuicTransceiver::streamClassCount()
{
	return (uint)s_StreamClasses.size();
}

const char *CQuicTransceiver::streamClassName(uint streamClass)
{
	return streamClass < s_StreamClasses.size() ? s_StreamClasses[streamClass].c_str() : "unknown";
}

CQuicStreamSend *CQuicTransceiver::createStreamSend(uint32 length)
{
	CQuicStreamSend *send = (CQuicStreamSend *)malloc(offsetof(CQuicStreamSend, Data) + length);
	nlassert(send);
	send->Buffer.Length = length;
	send->Buffer.Buffer = send->Data;
	return send;
}

void CQuicTransceiver::destroyStreamSend(CQuicStreamSend *send)
{
	free(send);
}

bool CQuicTransceiver::egressBytes(uint64 &bytes)
{
#ifdef NL_MSQUIC_AVAILABLE
	// The counters are those of the process: any transceiver's library handle will do
	const QUIC_API_TABLE *api = null;
	{
		CSynchronized<std::vector<CQuicTransceiver *>>::CAccessor transceivers(&s_QuicTransceivers);
		for (CQuicTransceiver *transceiver : transceivers.value())
		{
			if (transceiver->m->Api)
			{
				api = transceiver->m->Api;
				break;
			}
		}
		if (api)
		{
			// (asked while the list is held: a transceiver holds it while it closes its library handle)
			uint64 counters[QUIC_PERF_COUNTER_MAX] = {};
			uint32 size = sizeof(counters);
			QUIC_STATUS status = api->GetParam(null, QUIC_PARAM_GLOBAL_PERF_COUNTERS, &size, counters);
			if (QUIC_SUCCEEDED(status))
			{
				// What MsQuic counts is the payload of its UDP datagrams. The headers of UDP and IP are counted as
				// IPv6 (48 bytes), and a flush that goes out in several batches counts its datagrams more than
				// once: both err to the side of sending less than the budget.
				bytes = counters[QUIC_PERF_COUNTER_UDP_SEND_BYTES] + 48 * counters[QUIC_PERF_COUNTER_UDP_SEND];
				return true;
			}
		}
	}
#endif
	bytes = 0;
	return false;
}

const char *CQuicTransceiver::streamFaultName(int fault)
{
	switch (fault)
	{
	case QuicStreamNoFault: return "no fault";
	case QuicStreamOpenFailed: return "a stream could not be opened";
	case QuicStreamWrongId: return "a stream did not get the id of its class";
	case QuicStreamSendFailed: return "data could not be sent on a stream";
	case QuicStreamSendCanceled: return "data sent on a stream was dropped";
	case QuicStreamAbortedByPeer: return "the client refused a stream";
	default: return "unknown fault";
	}
}

NLMISC_CATEGORISED_COMMAND(fs, quicEcnStats, "Display how many of the closed QUIC connections that completed their handshake were ECN capable when they closed", "")
{
	if (args.size() != 0)
		return false;

	log.displayNL("ECN %s: %u of %u closed QUIC connections were ECN capable when they closed", s_EcnEnabled ? "enabled" : "disabled", (uint)s_EcnCapableConnections.load(), (uint)s_ClosedConnections.load());
	return true;
}

NLMISC_CATEGORISED_COMMAND(fs, quicCertReload, "Reload QUIC certificates", "")
{
	CSynchronized<std::vector<CQuicTransceiver *>>::CAccessor transceivers(&s_QuicTransceivers);
	for (CQuicTransceiver *transceiver : transceivers.value())
	{
		transceiver->reloadCert();
	}
	return true;
}

/* end of file */
