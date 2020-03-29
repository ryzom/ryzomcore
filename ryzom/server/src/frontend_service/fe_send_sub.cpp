// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

	_MsgXmlMD5 = NLMISC::getMD5("msg.xml");
	_DatabaseXmlMD5 = NLMISC::getMD5("database.xml");

	/*_TotalBandwidth = totalbandwith; // initialized by setTotalBandwidth() and setClientBandwitdth()
	_ClientBandwidth = clientbandwith; */
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
}


/*
 * Send the current outbox
 */
inline void	CFeSendSub::CSendBuffer::sendOutBox( NLNET::CUdpSock *datasock )
{
	if ( OutBox.length() != 0 )
	{
		sendUDP( datasock, OutBox.buffer(), OutBox.length(), &DestAddress );
		//nlinfo( "Sent %u bytes to %s", OutBox.length(), DestAddress.asString().c_str() );
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
