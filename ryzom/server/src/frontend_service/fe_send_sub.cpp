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

#include "game_share/system_message.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace CLFECOMMON;

uint SendCounterRatio = 0;
uint SendCounterDelay = 1000;
extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;

CVariable<uint32> StreamMinBytesPerTick("fs", "StreamMinBytesPerTick", "Bytes per cycle a QUIC connection may send on its streams before the frontend knows its congestion window", 200, 0, true);

namespace /* anonymous */ {

/// Bytes per cycle a connection may send on its streams when its congestion window has the room.
/// A fixed value for now: it becomes the allowance that follows how late the datagrams reach the client.
const sint64 s_StreamBytesPerTick = 1500;

/// Bytes of a QUIC packet with a full datagram (the datagram and the packet around it)
const sint64 s_DatagramPacketBytes = 560;

/// Bytes granted to the streams since the service started (see displayStreamStats)
uint64 s_StreamBytesGranted = 0;

/// Bytes the streams of a connection may take this cycle, which is what its congestion window leaves beside its
/// datagrams: the next datagram must never wait for the window.
/// The window is taken by the datagrams in flight (one per cycle within the round trip time) and by what was given to
/// the streams and is not acknowledged yet, with the headers of its packets (about 4%). That is counted from the
/// frontend's own numbers. If MsQuic said that more was in flight when it was last asked (datagrams whose
/// acknowledgement is late, data sent again after a loss), its number counts, with what was given to the streams since.
uint32 streamAllowance(CQuicUserContext *user)
{
	if (!user->StatValid)
		return StreamMinBytesPerTick.get();

	sint64 congestionWindow = user->StatCongestionWindow.load(NLMISC::TMemoryOrderRelaxed);
	sint64 rttMs = ((sint64)user->StatRtt.load(NLMISC::TMemoryOrderRelaxed) + 999) / 1000;
	sint64 tickMs = std::max((sint64)1, (sint64)(CTickEventHandler::getGameTimeStep() * 1000.0));
	sint64 datagramsInFlight = ((rttMs + tickMs - 1) / tickMs + 1) * s_DatagramPacketBytes;
	sint64 outstanding = user->StreamOutstanding.load();
	sint64 inFlight = datagramsInFlight + outstanding + outstanding / 25;
	sint64 bytesInFlight = user->StatBytesInFlight.load(NLMISC::TMemoryOrderRelaxed);
	if (bytesInFlight >= 0)
	{
		sint64 grantedSince = (uint32)user->StreamGrantedCount.load() - (uint32)user->StatGrantedCount.load(NLMISC::TMemoryOrderRelaxed);
		inFlight = std::max(inFlight, bytesInFlight + grantedSince + grantedSince / 25);
	}
	sint64 room = congestionWindow - inFlight - s_DatagramPacketBytes;
	return (uint32)std::min(std::max(room, (sint64)0), s_StreamBytesPerTick);
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

	_MsgXmlMD5 = NLMISC::getMD5("msg.xml");
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

	THostMap::iterator icm;
	for (icm = _ClientMap->begin(); icm != _ClientMap->end(); ++icm)
	{
		CClientHost *client = GETCLIENTA(icm);
		CQuicUserContext *user = client->QuicUser.get();
		if (!user || !user->Streams)
			continue;
		CStreamQueues &queues = user->StreamQueues;

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

		// The messages of the streams are impulses, which a client handles in its connected state only: as the
		// impulses do, they wait for it
		if (queues.Refusing || client->ConnectionState != CClientHost::Connected || queues.pendingBytes() == 0)
			continue;

		// The classes in their order (only one class has messages so far, they get shares of the allowance once
		// there are more)
		uint32 allowance = streamAllowance(user);
		for (uint streamClass = 0; streamClass < (uint)CLFECOMMON::NbStreamClasses && allowance != 0; ++streamClass)
		{
			uint32 size = std::min(allowance, queues.pendingBytes((CLFECOMMON::TStreamClass)streamClass));
			if (size == 0)
				continue;
			CQuicStreamSend *send = CQuicTransceiver::createStreamSend(size);
			queues.take((CLFECOMMON::TStreamClass)streamClass, send->Data, size);
			// Counted here and not when it is sent, so that the next cycle knows of it whenever the flush runs
			user->StreamOutstanding.fetchAdd((int)size);
			user->StreamGrantedCount.fetchAdd((int)size);
			m_FillingStreamPosts->push_back(CStreamPost(user, (uint8)streamClass, send));
			allowance -= size;
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
			QuicUser->Transceiver->sendDatagramSwap(QuicUser.get(), OutBox);
		}
		else
		{
			sendUDP(datasock, OutBox.buffer(), OutBox.length(), &DestAddress);
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
