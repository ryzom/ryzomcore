// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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

#include "game_share/generic_xml_msg_mngr.h"

#include <nel/misc/stop_watch.h>
#include <nel/misc/path.h>
#include "fe_send_sub.h"
#include "game_share/simlag.h"
#include "game_share/action_factory.h"
#include "game_share/action_position.h"
#include "game_share/action_sync.h"
#include "game_share/action_disconnection.h"
#include "game_share/action_association.h"
#include "game_share/action_sint64.h"
#include "game_share/tick_event_handler.h"
#include "history.h"
#include "frontend_service.h"
#include "prio_sub.h"
#include "fe_stat.h"
#include "stream_scheduler.h"

#include "game_share/system_message.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace CLFECOMMON;

uint SendCounterRatio = 0;
uint SendCounterDelay = 1000;
extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;

// clang-format off
// The budget of the frontend and the allowances of the clients, for the QUIC streams (see stream_scheduler.h)
CVariable<uint32>	TotalOutgoingBandwidth("fs", "TotalOutgoingBandwidth", "Bytes per second the frontend may send in all, to all clients: the datagrams, which always go, and the QUIC streams, which share what is left. Not more than the link of the frontend carries: what the link cannot take in a cycle waits in front of the datagrams of the next", 5000000, 0, true);
CVariable<uint32>	StreamMinBytesPerTick("fs", "StreamMinBytesPerTick", "Bytes per cycle every client with data for its QUIC streams gets, whatever is left of TotalOutgoingBandwidth and whatever the frontend holds itself to: with the datagram it is what a client is guaranteed, and ClientLimit is how many clients the link carries of that for sure", 200, 0, true);
CVariable<uint32>	StreamMaxBytesPerTick("fs", "StreamMaxBytesPerTick", "Most bytes per cycle one client gets on its QUIC streams", 65536, 0, true);
CVariable<uint32>	StreamMinChunk("fs", "StreamMinChunk", "Smallest turn of a client on its QUIC streams when the frontend's budget is short, and the bytes per cycle a connection starts with", 8192, 0, true);
CVariable<std::string>	StreamClassWeights("fs", "StreamClassWeights", "How a client's stream data of one cycle is shared between its classes that have some waiting, as name=weight with spaces between; a class that is not named has 1", "strings=2 chat=1 login=2 gateway=2", 0, true);
CVariable<uint32>	QueueDelayTarget("fs", "QueueDelayTarget", "Milliseconds the datagrams may reach a client late before the frontend sends it less on its QUIC streams", 20, 0, true);
// clang-format on

namespace /* anonymous */ {

/// Bytes of a QUIC packet with a full datagram (the datagram and the packet around it)
const sint64 s_DatagramPacketBytes = 560;

/// What goes on the wire with a datagram besides its bytes: the QUIC packet around it (5 bytes of header, 4 of packet
/// number, 3 of frame, 16 of tag) and the headers of UDP and IP, counted as IPv6; for a UDP client those headers alone
const uint64 s_QuicDatagramOverhead = 76;
const uint64 s_UdpDatagramOverhead = 48;

/// Bytes granted to the streams since the service started (see displayStreamStats)
uint64 s_StreamBytesGranted = 0;

/// Bytes sent to the UDP clients since the service started, with their headers (the flush counts them)
NLMISC::CAtomicInt s_UdpEgressBytes;
uint64 s_UdpEgressTotal = 0; // as the main thread last read it, without the wrap of the counter
uint32 s_UdpEgressLast = 0;

/// What MsQuic last said of a connection and what the frontend gave it since, for streamWindowRoom
CStreamWindowState streamWindowState(CQuicUserContext *user)
{
	CStreamWindowState state;
	state.CongestionWindow = user->StatCongestionWindow.load(NLMISC::TMemoryOrderRelaxed);
	state.RttMicroseconds = user->StatRtt.load(NLMISC::TMemoryOrderRelaxed);
	state.BytesInFlight = user->StatBytesInFlight.load(NLMISC::TMemoryOrderRelaxed);
	state.Outstanding = user->StreamOutstanding.load();
	state.GrantedSinceSnapshot = (uint32)user->StreamGrantedCount.load() - (uint32)user->StatGrantedCount.load(NLMISC::TMemoryOrderRelaxed);
	return state;
}

/// Say so once when the settings of the streams cannot be meant (at start, and when one is changed)
void checkStreamConfig(const CStreamSchedulerConfig &config, uint32 maxClients, uint32 datagramBytesPerCycle)
{
	static bool s_Checked = false;
	static CStreamSchedulerConfig s_Last;
	static uint32 s_LastMaxClients = 0, s_LastDatagramBytes = 0;
	if (s_Checked && config.TotalOutgoingBandwidth == s_Last.TotalOutgoingBandwidth && config.StreamMinBytesPerTick == s_Last.StreamMinBytesPerTick
	    && config.StreamMaxBytesPerTick == s_Last.StreamMaxBytesPerTick && config.StreamMinChunk == s_Last.StreamMinChunk
	    && maxClients == s_LastMaxClients && datagramBytesPerCycle == s_LastDatagramBytes)
		return;
	s_Checked = true;
	s_Last = config;
	s_LastMaxClients = maxClients;
	s_LastDatagramBytes = datagramBytesPerCycle;

	// What the clients are guaranteed, with as many of them as the frontend takes: their datagrams, and the minimums
	// of their streams when all of them have stream data waiting. That goes whatever the budget, so the link has to
	// carry it for sure.
	const double perSecond = 1.0 / std::max(config.TickSeconds, 0.001);
	// (rounded: a cycle of a tenth of a second is not exactly that as a number)
	const uint64 datagrams = (uint64)((double)maxClients * (double)(datagramBytesPerCycle + s_QuicDatagramOverhead) * perSecond + 0.5);
	const uint64 minimums = (uint64)((double)maxClients * (double)std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick) * perSecond + 0.5);
	const uint64 guaranteed = datagrams + minimums;
	nlinfo("FESEND: ClientLimit (%u) clients need %" NL_I64 "u bytes per second (%.1f Mbit/s) of outgoing bandwidth that the link carries for sure: %" NL_I64 "u for their datagrams (%u bytes a cycle each with what the network adds) and %" NL_I64 "u for the minimums of their QUIC streams (StreamMinBytesPerTick, %u, when all of them have stream data waiting). That goes whatever the budget; TotalOutgoingBandwidth, %u, is the most the frontend sends when the link has more",
	    maxClients, guaranteed, (double)guaranteed * 8.0 / 1000000.0, datagrams, (uint32)(datagramBytesPerCycle + s_QuicDatagramOverhead), minimums,
	    std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick), config.TotalOutgoingBandwidth);
	if (guaranteed > config.TotalOutgoingBandwidth)
		nlwarning("FESEND: what ClientLimit (%u) clients are guaranteed, %" NL_I64 "u bytes per second, is above TotalOutgoingBandwidth (%u): with that many clients the frontend sends more than it is said it may",
		    maxClients, guaranteed, config.TotalOutgoingBandwidth);

	if (config.StreamMaxBytesPerTick == 0)
		nlwarning("StreamMaxBytesPerTick is 0: nothing is sent on the QUIC streams, and a client is disconnected once StreamMaxQueuedBytes of messages wait for it");
	else if (config.StreamMinBytesPerTick > config.StreamMaxBytesPerTick)
		nlwarning("StreamMinBytesPerTick (%u) is above StreamMaxBytesPerTick (%u): every client gets %u bytes per cycle on its QUIC streams whatever TotalOutgoingBandwidth leaves", config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick, config.StreamMaxBytesPerTick);
	if (config.TotalOutgoingBandwidth == 0)
		nlwarning("TotalOutgoingBandwidth is 0, which is no budget at all, not an unlimited one: the QUIC streams get StreamMinBytesPerTick per client and cycle and nothing else");
	if (config.StreamMinChunk == 0)
		nlwarning("StreamMinChunk is 0: when TotalOutgoingBandwidth is short, the turns of the clients on their QUIC streams can be of no bytes at all");
}

} /* anonymous namespace */

uint64 getStreamBytesGranted()
{
	return s_StreamBytesGranted;
}



/*
 * Init
 */
void CFeSendSub::init( NLNET::CUdpSock *datasock, THostMap *clientmap, CHistory *history, CPrioSub *priosub )
{
	nlassert( datasock && history );

	_DataSock = datasock;

	_ClientMap = clientmap;
	_History = history;
	_PrioSub = priosub;

	_SendBuffers1.resize( MaxNbClients+1 );
	_SendBuffers2.resize( MaxNbClients+1 );
	_CurrentFillingBuffers = &_SendBuffers1;
	_CurrentFlushingBuffers = &_SendBuffers2;
	m_FillingStreamPosts = &m_StreamPosts1;
	m_FlushingStreamPosts = &m_StreamPosts2;

	// (without what declares the stream classes: the clients need not have those to have the same file)
	_MsgXmlMD5 = CGenericXmlMsgHeaderManager::getSignature("msg.xml", false);
	_DatabaseXmlMD5 = NLMISC::getMD5("database.xml");

	/*_TotalBandwidth = totalbandwith; // initialized by setTotalBandwidth() and setClientBandwitdth()
	_ClientBandwidth = clientbandwith; */
}


/*
 * Destructor
 */
CFeSendSub::~CFeSendSub()
{
	// What was never flushed
	for (TStreamPosts::iterator it = m_StreamPosts1.begin(); it != m_StreamPosts1.end(); ++it)
		CQuicTransceiver::destroyStreamSend(it->Send);
	for (TStreamPosts::iterator it = m_StreamPosts2.begin(); it != m_StreamPosts2.end(); ++it)
		CQuicTransceiver::destroyStreamSend(it->Send);
}


/*
 * Set client bandwidth per cycle in bytes
 */
void	CFeSendSub::setClientBandwidth( uint32 bytes )
{
	_ClientBitBandwidth = bytes * 8;

	// Change the bandwidth of all logged clients
	THostMap::iterator ihm;
	for ( ihm=_ClientMap->begin(); ihm!=_ClientMap->end(); ++ihm )
	{
		GETCLIENTA(ihm)->setClientBandwidth( _ClientBitBandwidth );
	}
}



/*
 * Setup headers for outgoing messages of current cycle
 */
void	CFeSendSub::prepareHeadersAndFillImpulses()
{
	//CFrontEndService::instance()->HeadWatch.start();

	TTime	ctime = CTime::getLocalTime();
	
	THostMap::iterator iclient;
	for ( iclient=_ClientMap->begin(); iclient!=_ClientMap->end(); ++iclient )
	{
		CClientHost	*client = GETCLIENTA(iclient);

		if ( SendCounterRatio > 0 && ctime - client->LastCounterTime > SendCounterDelay)
		{
			// Send several packets when checking for packet lost
			client->LastCounterTime = ctime;

			uint	i;
			for (i=0; i<SendCounterRatio; ++i)
			{
				CBitMemStream	bms;
				GenericXmlMsgHeaderMngr.pushNameToStream("DEBUG:COUNTER", bms);

				bms.serial(client->LastSentCounter);
				client->LastSentCounter++;

				//nldebug( "DebugCounter %s", CInstanceCounterManager().getInstance().displayCounter( "CAction" ).c_str() );
				CActionGeneric *ag = (CActionGeneric *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_CODE);
				ag->set (bms);

				CFrontEndService::instance()->addImpulseToClient (client->clientId(), ag, 1);
			}
		}

		if ( client->whenToSend() )
		{
			TOutBox& outbox = (*_CurrentFillingBuffers)[client->clientId()].OutBox;

			// Clear outbox
			outbox.resetBufPos();

			// Manage connection state (probe or not)
			switch (client->ConnectionState)
			{
			case CClientHost::Connected:
				{
					client->NbActionsSentAtCycle = 0;

					// 1. Fill headers
					client->setupOutBox( outbox );

#ifdef INCLUDE_FE_STATS_IN_PACKETS
					// send debug info in the message header
					uint32 value;
					value = CFrontEndService::instance()->UserLWatch.getPartialAverage();
					outbox.serialAndLog1 (value);
					value = CFrontEndService::instance()->CycleWatch.getPartialAverage();
					outbox.serialAndLog1 (value);
					value = CFrontEndService::instance()->ReceiveWatch.getPartialAverage();
					outbox.serialAndLog1 (value);
					value = CFrontEndService::instance()->SendWatch.getPartialAverage();
					outbox.serialAndLog1 (value);
					outbox.serialAndLog1 (client->PrioAmount);
					uint16 seenentities = 255 - client->NbFreeEntityItems;
					outbox.serialAndLog1 ( seenentities );
					/*float hpt = CFrontEndService::instance()->PrioSub.Prioritizer.hpThreshold();
					outbox.serialAndLog1( hpt );*/
					
					// Important: Set STAT_HEADER_SIZE if you change the debug info sent
#endif
					// 2. Fill impulse actions
					sint32 impulseFilledBits = (sint32)client->ImpulseEncoder.send( client->sendNumber(), outbox, _NbImpulseActions );

					// Impulsion flow control, takes into account:
					// - the number of bits filled (possibly exceeding the max when sending forced actions (database))
					// - the number of remaining actions not forced
					client->setImpulsionThrottle( impulseFilledBits, (sint32)client->ImpulseEncoder.maxBitSize(2), client->ImpulseEncoder.queueSize() );

				}
				break;
			case CClientHost::Synchronize:
			case CClientHost::ForceSynchronize:
				{
					// Sending of Sync is now guaranteed at this cycle (see below) => fall back to Synchronize
					if (client->ConnectionState == CClientHost::ForceSynchronize)
						client->setSynchronizeState();

					// send SYNC at defined frequency
					NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle();
					client->setFirstSentPacket( client->sendNumber()+1, tick );
					client->setupSystemHeader( outbox, SYSTEM_SYNC_CODE);

					//
					TGameCycle	sync = client->getSync();
					outbox.serialAndLog1(sync);
					TTime		stime = CTime::getLocalTime();
					outbox.serialAndLog1(stime);
					outbox.serialAndLog1(client->LastSentSync);

					outbox.serialBuffer(_MsgXmlMD5.Data, sizeof(_MsgXmlMD5.Data));
					outbox.serialBuffer(_DatabaseXmlMD5.Data, sizeof(_DatabaseXmlMD5.Data));

					nlinfo("FESEND: sent SYNC message to client %d - SYNC=%d, GameCycle=%d", client->clientId(), sync, CTickEventHandler::getGameCycle());
					break;
				}
			case CClientHost::Probe:
				// send PROBE at defined frequency
				if (ctime - client->LastProbeTime > PROBESendLatency)
				{
					client->setupSystemHeader( outbox, SYSTEM_PROBE_CODE);
					client->LastSentProbe++;
					outbox.serialAndLog1(client->LastSentProbe);
					client->LastProbeTime = ctime;
					nldebug("FESEND: sent PROBE message to client %d", client->clientId());
					client->setIdleImpulsionThrottle();
				}
				break;
			}

			//_OutputBits += outbox.getPosInBit();
		}
	}

	nlassert( ! _ClientMap->empty() );

	//CFrontEndService::instance()->HeadWatch.stop();
}


/*
 * Fill prioritized actions into outgoing messages.
 * Query priority subsystem for properties, 
 *    retrieve them using mirror,
 *    fill client outboxes,
 *    fill history.
 */
void	CFeSendSub::fillPrioritizedActions()
{
#ifdef MEASURE_FRONTEND_TABLES
	PosSentCntFrame.setGameTick();
	PosSentCntFrame.reset( 0 );
#endif

	// We don't take _TotalBitBandwith into account currently

	/* Iterate on the clients
	 */
	THostMap::iterator icm;
	for ( icm=_ClientMap->begin(); icm!=_ClientMap->end(); ++icm )
	{
		CClientHost *destclient = GETCLIENTA(icm);

		// Do not fill if client is blocked
		if ( destclient->whenToSend() && (destclient->ConnectionState == CClientHost::Connected) )
		{
			TOutBox& outbox = (*_CurrentFillingBuffers)[destclient->clientId()].OutBox;
			_PrioSub->Prioritizer.fillOutBox( *destclient, outbox );
			_NbActions += destclient->NbActionsSentAtCycle;
			destclient->updateThrottle( outbox );
		}

		// Open/close the send buffers, depending on the state clienthost->whenToSend().
		// Note: the send buffers must not have a reference on the clienthost objects, because
		// these can be destroyed when those are still processed in the background, therefore
		// we browse the clients to enable/disable the send buffers.
		// We set the current filling buffer only, just before swapping them, so that
		// the changes will be taken into account at next flushing.

		CSendBuffer& sendbuffer = (*_CurrentFillingBuffers)[destclient->clientId()];
		sendbuffer.enableSendBuffer( destclient->whenToSend() );
		//nldebug( "%u: client %hu %s", CTickEventHandler::getGameCycle(), destclient->clientId(), destclient->whenToSend()?"OPEN":"CLOSED" );

		// Switch the send state of the client
		destclient->incSendCycle();

	}

	//nlassert( ! _ClientMap->empty() );
	//float fullbuffers = (float)_OutputBits / (float)_ClientBitBandwidth;

	CFrontEndService::instance()->SentActionsLastCycle = _NbActions + _NbImpulseActions;
	//CFrontEndService::instance()->ScannedPropsLastCycle = _NbActions + nbnotsent - _NbImpulseActions;

//	nlinfo( "FESEND: Sent %u actions (%u left ; %.2f full buffers, %.2f%%) including %u impulse (%0.2f priorities, %u per client)", _NbActions, nbnotsent, fullbuffers, fullbuffers/(float)_ClientMap->size()*100.0f, _NbImpulseActions, _PrioAmount, _NbActions / _ClientMap->size() );

	/*for ( iclient=_ClientMap->begin(); iclient!=_ClientMap->end(); ++iclient )
	{
		nlinfo( "Client %u: size %u / %u", GETCLIENTA(iclient)->clientId(), GETCLIENTA(iclient)->OutBox.length(), _ClientBandwith );
	}*/

	//CFrontEndService::instance()->FillWatch.stop();

#ifdef MEASURE_FRONTEND_TABLES
	PosSentCntFrame.commit( PosSentCntClt1 );
#endif

}


/*
 * Take out of the stream queues of the QUIC connections what each may send this cycle
 */
void	CFeSendSub::grantStreams()
{
	CFeReceiveSub *receiveSub = CFrontEndService::instance()->receiveSub();

	CStreamSchedulerConfig config;
	config.TickSeconds = std::max(0.001, (double)CTickEventHandler::getGameTimeStep() * std::max((sint)1, CFrontEndService::instance()->gameCycleRatio()));
	config.TotalOutgoingBandwidth = TotalOutgoingBandwidth.get();
	config.StreamMinBytesPerTick = StreamMinBytesPerTick.get();
	config.StreamMaxBytesPerTick = StreamMaxBytesPerTick.get();
	config.StreamMinChunk = StreamMinChunk.get();
	config.QueueDelayTarget = QueueDelayTarget.get();
	config.DatagramPacketBytes = (uint32)s_DatagramPacketBytes;
	checkStreamConfig(config, MaxNbClients, clientBandwidth() / 8);

	// What the frontend sent since the last cycle, all of it: what MsQuic counts for the process, and the datagrams of
	// the UDP clients
	uint32 udpEgress = (uint32)s_UdpEgressBytes.load();
	s_UdpEgressTotal += (uint32)(udpEgress - s_UdpEgressLast);
	s_UdpEgressLast = udpEgress;
	uint64 quicEgress = 0;
	CQuicTransceiver::egressBytes(quicEgress);
	const uint64 egress = quicEgress + s_UdpEgressTotal;
	const TTime now = CTime::getLocalTime();
	uint64 measuredEgress = 0;
	double elapsedSeconds = config.TickSeconds;
	if (m_StreamStarted)
	{
		measuredEgress = egress >= m_StreamLastEgress ? egress - m_StreamLastEgress : 0;
		elapsedSeconds = (double)(now - m_StreamLastTime) / 1000.0;
	}
	m_StreamStarted = true;
	m_StreamLastEgress = egress;
	m_StreamLastTime = now;

	// The datagrams of this cycle, as they will go on the wire, and the clients that can get stream data, by
	// ascending id
	// Whose datagrams arrive late, of the clients that say: each client once, by its last report of the last cycles,
	// however often it reports. Half of them late at once is the link they share (CStreamScheduler::reports).
	uint32 reporting = 0, reportingLate = 0, quietReporting = 0, quietLate = 0;
	for (size_t id = 0; id < _ClientIdCont.size(); ++id)
	{
		CClientHost *client = _ClientIdCont[id];
		if (client == NULL || client->isDisconnected() || client->ConnectionState != CClientHost::Connected)
			continue;
		CQuicUserContext *user = client->QuicUser.get();
		if (!user || !user->Streams || !user->QueueDelayKnown)
			continue;
		if (user->QueueDelayReports == user->QueueDelayReportsSeen && !user->StreamAllowance.reportsLately())
			continue;
		const bool late = user->StreamAllowance.isLate(config, user->QueueDelay);
		++reporting;
		if (late)
			++reportingLate;
		if (user->StreamAllowance.quietLately())
		{
			++quietReporting;
			if (late)
				++quietLate;
		}
	}
	m_StreamScheduler.reports(config, reporting, reportingLate, quietReporting, quietLate);
	const bool sharedLateness = m_StreamScheduler.sharedLateness();
	const bool budgetShort = m_StreamScheduler.budgetShare() < 1.0;
	if (budgetShort != m_StreamBudgetShort)
	{
		m_StreamBudgetShort = budgetShort;
		if (budgetShort)
			nlwarning("FESEND: the datagrams of %u of the %u clients that report it arrive late at once, which is taken for the link of the frontend carrying less than TotalOutgoingBandwidth (%u bytes per second): the QUIC streams are held to less until it carries that again",
			    reportingLate, reporting, config.TotalOutgoingBandwidth);
		else
			nlinfo("FESEND: the QUIC streams have all of TotalOutgoingBandwidth (%u bytes per second) again", config.TotalOutgoingBandwidth);
	}

	// The weights of the classes, when they or the classes change
	const uint nbClasses = CQuicTransceiver::streamClassCount();
	if (m_StreamClassWeights.size() != nbClasses || m_StreamClassWeightsText != StreamClassWeights.get())
	{
		m_StreamClassWeightsText = StreamClassWeights.get();
		std::vector<std::string> classNames(nbClasses);
		for (uint i = 0; i < nbClasses; ++i)
			classNames[i] = CQuicTransceiver::streamClassName(i);
		std::vector<std::string> faults;
		parseStreamClassWeights(m_StreamClassWeightsText, classNames, m_StreamClassWeights, faults);
		for (size_t i = 0; i < faults.size(); ++i)
			nlwarning("FESEND: StreamClassWeights: %s", faults[i].c_str());
	}

	uint64 datagramBytes = 0;
	m_StreamEntries.clear();
	for (size_t id = 0; id < _ClientIdCont.size(); ++id)
	{
		CClientHost *client = _ClientIdCont[id];
		if (client == NULL || client->isDisconnected())
			continue;

		const CSendBuffer &sendBuffer = (*_CurrentFillingBuffers)[id];
		if (sendBuffer.SBState && sendBuffer.OutBox.length() != 0)
			datagramBytes += sendBuffer.OutBox.length() + (client->QuicUser.get() ? s_QuicDatagramOverhead : s_UdpDatagramOverhead);

		CQuicUserContext *user = client->QuicUser.get();
		if (!user || !user->Streams)
			continue;
		CStreamQueues &queues = user->StreamQueues;
		user->StreamLastGrant = 0;

		// A fault of the streams, found where they are opened, where data is sent on them, or by MsQuic: bytes that
		// were taken out of the queues did not go out, and a stream with a hole is of no use
		int fault = user->StreamFault.load(NLMISC::TMemoryOrderRelaxed);
		if (fault != QuicStreamNoFault)
		{
			if (!queues.Refusing)
			{
				nlwarning("FESEND: client %u (uid %u, %s) is disconnected, %s: messages of its QUIC streams are lost",
				    (uint)client->clientId(), client->Uid, client->address().asString().c_str(), CQuicTransceiver::streamFaultName(fault));
				queues.Refusing = true;
				queues.clear();
				receiveSub->disconnectClient(client);
			}
			continue;
		}
		if (queues.Refusing)
			continue;

		// What the path to the client takes: its allowance follows how late it says its datagrams arrive
		if (user->PathChanged.exchange(false))
			user->StreamAllowance.reset(config);
		const bool newReport = user->QueueDelayReports != user->QueueDelayReportsSeen;
		user->QueueDelayReportsSeen = user->QueueDelayReports;
		user->StreamAllowance.update(config, newReport, user->QueueDelay, user->QueueDelayAck, client->sendNumber(),
		    (uint32)user->StatCongestionEvents.load(NLMISC::TMemoryOrderRelaxed), (uint32)user->SendBusyDrops.load(NLMISC::TMemoryOrderRelaxed), sharedLateness, budgetShort);

		CStreamScheduler::CEntry entry;
		entry.Client = &user->StreamTurn;
		entry.Id = (uint32)id;
		entry.Pending = queues.pendingBytes();
		// The messages of the streams are impulses, which a client handles in its connected state only: as the
		// impulses do, they wait for it. Until the frontend knows the congestion window, the minimum is all it gives.
		user->StreamWindowLimited = false;
		if (client->ConnectionState != CClientHost::Connected)
			entry.Window = 0;
		else if (!user->StatValid)
			entry.Window = std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick);
		else
		{
			const uint32 room = streamWindowRoom(config, streamWindowState(user));
			user->StreamWindowLimited = room < user->StreamAllowance.allowance();
			entry.Window = std::min(user->StreamAllowance.allowance(), room);
		}
		m_StreamEntries.push_back(entry);
	}

	// The budget is kept every cycle, also when no client has streams: what was sent is taken off it. It is less than
	// TotalOutgoingBandwidth while the datagrams of half the clients are late at once.
	m_StreamScheduler.schedule(config, elapsedSeconds, measuredEgress, datagramBytes, m_StreamEntries);

	for (size_t i = 0; i < m_StreamEntries.size(); ++i)
	{
		const CStreamScheduler::CEntry &entry = m_StreamEntries[i];
		CClientHost *client = _ClientIdCont[entry.Id];
		CQuicUserContext *user = client->QuicUser.get();
		CStreamQueues &queues = user->StreamQueues;
		// (limited by its congestion window: that had less room than the allowance, and the client got all of it)
		user->StreamAllowance.granted(entry.Grant, entry.Pending > entry.Grant, user->StreamWindowLimited && entry.Grant == entry.Window);
		user->StreamLastGrant = entry.Grant;

		// Each class its part of the grant
		queues.share(entry.Grant, m_StreamClassWeights, m_StreamShares);
		for (uint streamClass = 0; streamClass < nbClasses && streamClass < m_StreamShares.size(); ++streamClass)
		{
			uint32 size = m_StreamShares[streamClass];
			if (size == 0)
				continue;
			CQuicStreamSend *send = CQuicTransceiver::createStreamSend(size);
			queues.take(streamClass, send->Data, size);
			// Counted here and not when it is sent, so that the next cycle knows of it whenever the flush runs
			user->StreamOutstanding.fetchAdd((int)size);
			user->StreamGrantedCount.fetchAdd((int)size);
			m_FillingStreamPosts->push_back(CStreamPost(user, (uint8)streamClass, send));
			s_StreamBytesGranted += size;
		}
	}
}


/*
 * Swap read buffers
 */
void CFeSendSub::swapSendBuffers()
{
	// Synchronization is done at module-level (in frontend_service.cpp)

	// OBSOLETE: now the enabling of the send buffers depends on clienthost->whenToSend(), see below
	/*// Before swapping, but after the previous flushing, enable the buffers of the new clients
	// in the flushing buffers
	typename TBuffersToEnable::iterator ib;
	for ( ib=_BuffersToEnable.begin(); ib!=_BuffersToEnable.end(); ++ib )
	{
		enableSendBuffer( *ib );
	}
	_BuffersToEnable.clear();*/

	// The flush of the flushing buffers has ended (the caller waits for it), so they can now receive the addresses that
	// were set or unset in the filling buffers only. They are enabled again when filled.
	for (std::vector<TClientId>::const_iterator it = m_PendingSendBufferIds.begin(); it != m_PendingSendBufferIds.end(); ++it)
	{
		const CSendBuffer &filling = (*_CurrentFillingBuffers)[*it];
		(*_CurrentFlushingBuffers)[*it].setAddress(&filling.DestAddress, filling.QuicUser.get(), false);
	}
	m_PendingSendBufferIds.clear();

	// Swap the buffers
	if ( _CurrentFillingBuffers == &_SendBuffers1 )
	{
		_CurrentFillingBuffers = &_SendBuffers2;
		_CurrentFlushingBuffers = &_SendBuffers1;
	}
	else
	{
		_CurrentFillingBuffers = &_SendBuffers1;
		_CurrentFlushingBuffers = &_SendBuffers2;
	}

	// and the stream data with them (the flush emptied its list)
	std::swap(m_FillingStreamPosts, m_FlushingStreamPosts);
}


/*
 * Send the current outbox
 */
inline void	CFeSendSub::CSendBuffer::sendOutBox( NLNET::CUdpSock *datasock )
{
	if ( OutBox.length() != 0 )
	{
		if (QuicUser.get())
		{
			// Protocol level ryzomcore/4.2: the time the datagram is sent goes in the room its header has for it
			// (CClientHost::setupLatenessHeader)
			if (QuicUser->Streams)
				OutBox.poke((uint32)(uint16)CTime::getLocalTime(), CLFECOMMON::DatagramSendTimeBitPos, CLFECOMMON::DatagramSendTimeBits);
			QuicUser->Transceiver->sendDatagramSwap(QuicUser.get(), OutBox);
		}
		else
		{
			sendUDP(datasock, OutBox.buffer(), OutBox.length(), &DestAddress);
			s_UdpEgressBytes.fetchAdd((int)(OutBox.length() + s_UdpDatagramOverhead));
			//nlinfo( "Sent %u bytes to %s", OutBox.length(), DestAddress.asString().c_str() );
		}
	}

#ifdef MEASURE_SENDING
		static sint32 loopcount = 0;
		++loopcount;
		static TTime lastdisplay = CTime::getLocalTime();
		TTime tn = CTime::getLocalTime();
		uint32 diff = (uint32)(tn - lastdisplay);
		if ( diff > 2000 )
		{
			nlinfo("Sends by second: %.1f => LoopTime = %.2f ms LoopCount = %u Diff = %u ms Size=%u",(float)loopcount * 1000.0f / (float)diff, (float)diff / loopcount, loopcount, diff, OutBox.length());
			loopcount = 0;
			lastdisplay = tn;
		}
#endif
}


/*
 * Send outgoing messages
 * This can be executed by a background thread
 */
void	CFeSendSub::flushMessages()
{

	//CFrontEndService::instance()->SndtWatch.start();

#ifdef MEASURE_SENDING
	TTicks before = CTime::getPerformanceTime();
#endif

	TSendBuffers::iterator isb;
	for ( isb=_CurrentFlushingBuffers->begin(); isb!=_CurrentFlushingBuffers->end(); ++isb )
	{
		if ( (*isb).SBState )
		{
			try
			{
				(*isb).sendOutBox( _DataSock );
				++_SendCounter;
				//nldebug( "%u: SENDING NOW %u bytes to %s", CTickEventHandler::getGameCycle(), (*isb).OutBox.length(), (*isb).DestAddress.asString().c_str() );
			}
			catch (const ESocket&)
			{
				nlwarning( "Could not send data to client %u", isb-_CurrentFlushingBuffers->begin() );
			}
		}
	}

	// Second pass, after the datagrams of all the connections: the stream data of this cycle. It is sent whether or not
	// its connection sent a datagram, and the list is emptied, so that a flush outside of the cycle sends nothing twice.
	for (TStreamPosts::iterator isp = m_FlushingStreamPosts->begin(); isp != m_FlushingStreamPosts->end(); ++isp)
	{
		isp->User->Transceiver->sendStream(isp->User.get(), isp->StreamClass, isp->Send);
	}
	m_FlushingStreamPosts->clear();

#ifdef MEASURE_SENDING
	TTicks after = CTime::getPerformanceTime();
	static TTicks sum = 0;
	sum += (after-before);
	static uint counter = 0;
	++counter;
	if ( counter == 20 )
	{
		nlinfo( "Sending time: %.3f ms", CTime::ticksToSecond( sum/20 ) * 1000.0 );
		sum = 0;
		counter = 0;
	}
#endif

}



/*
 * Update
 * Deprecated: replaced by modules (see initModules() in frontend_service.cpp)
 */
/*void CFeSendSub::update()
{
	if ( _ClientMap->empty() )
	{
		return;
	}

	prepareHeaders();

	fillImpulses();

	fillUIActions();

	fillPrioritizedActions();

	flushMessages();

	_PrioSub->endCycle(); // disables the priority marked to

	nlinfo( "FETIME: Time of sending: Headers=%u Fill=%u Send=%u EndCycle=%u",
		CFrontEndService::instance()->HeadWatch.getDuration(),
		CFrontEndService::instance()->FillWatch.getDuration(),
		CFrontEndService::instance()->SndtWatch.getDuration(),
		CFrontEndService::instance()->EndCWatch.getDuration() );
}*/


NLMISC_VARIABLE( uint, SendCounterRatio, "Set the check counter send ratio, e.g. the number of counter sent at a time (0 means don't send)");
NLMISC_VARIABLE( uint, SendCounterDelay, "Set the check counter send delay, e.g. the time between 2 counter send rafale (in ms)");
