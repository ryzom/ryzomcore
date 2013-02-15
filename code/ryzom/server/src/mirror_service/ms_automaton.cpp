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




/*
 * Description of a the cycle of the Mirror Service and of a client service:
 *
 *				Mirror Service (M)												Client Service (C)
 *
 * 1. Receive master TICK from Tick Service		    TICK
 * |  => Send TICK to local client services     ------------>	1. Receive TICK => call tick update callback
 *																|
 * 2. Receive messages from client services		MSG to forward	|
 * |  => store them in message queue		    <------------	|
 * |  Receive delta updates from remote MSs						|
 * |  => store them in pending deltas			    TOCK		|
 * |  Wait for all TOCKs from clients services  <------------	When done, send TOCK to MS
 * |
 *
 * 3. Build and send delta to remote MSs
 *
 * 3.5 Wait for all delta updates received						X: non-synchronized message callbacks
 *    Receive delta updates of same TICK from
 *    remote MSs that didn't answer before
 *
 *
 * 4. Send master TOCK to Tick Service
 * |
 * 5. Apply pending deltas to shared memory
 * |											     UMM
 * 6. Trigger processing of mirror changes		------------>	2. Execute notification callback to update
 * |  and send messages to client services						|  mirror entities and process changes
 *																|
 *																3. Receive messages and transport classes
 *																|  and execute callbacks
 *
 *																Note: non-mirrored messages and non-mirror
 *																transport classes may be received between
 *																C1. and C2. and between C3. and C1.
 *
 *
 *																X: non-synchronized message callbacks
 *
 * Parallelism and consistency: on one physical machine, the services read and write properties
 * in the same shared memory. It means changing a value by a service A is immediately reflected
 * to the service B if they are on the same physical machine. A side effect is that a value can
 * have changed between two successive readings. In most cases, this is not a problem, for exemple
 * if the property represents a colour or such an unimportant property. However, for certain kinds
 * of properties, it may be dangerous. For example, let's say _MyValue is a CMirrorPropValueRO.
 * The following code may lead to a crash: if (_MyValue() != 0) result=1.0f/_MyValue(); because
 * the value can become 0 at any time. The solution to this is to write: float value = _MyValue();
 * if (value != 0) result=1.0f/value;. A similar problem can occur when storing a dataset row (e.g.
 * for targeting purpose): the result of the test isValid() can change. User should use the
 * temporary copy way.
 */

#include "mirror_service.h"
#include "tick_proxy.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern NLMISC::CLog				_QuickLog;

static TTime BeginWaitForAllDeltaReceivedTime = 0;


/*
 * From a client service
 */
void cbMessageToForward( CMessage& msgin, const string& serviceName, TServiceId serviceId )
{
	// The assert is not enabled, because in fact some messages *can* come when the state
	// is not ExpectingLocalTocks. How is it possible? When a service sends its message
	// in a cbServiceUp callback, or in a callback to a message not send using the mirror
	// system (?). This is not a problem as the message is only buffered.
	//nlassert( CTickProxy::State == ExpectingLocalTocks );
	MSInstance->receiveMessageToForwardFromClient( msgin, serviceId );
}


/*
 * From another MS
 */
void cbRecvDelta( CMessage& msgin, const string& serviceName, TServiceId serviceId )
{
	// The assert is not enabled, because in fact some delta *can* come when the state
	// is not ExpectingLocalTocks. How is it possible? Example: There are two MS, MS1
	// has a client service. When MS1 sends its first delta packet, MS2 may not have
	// received its first Master Tick! This is not a problem as the message is only buffered.
	//nlassert( CTickProxy::State == ExpectingLocalTocks );
	MSInstance->receiveDeltaFromRemoteMS( msgin, serviceId );
}


/*
 * Sync received from master (see tick proxy)
 */
void cbOnMasterSync()
{
	MSInstance->examineDeltaUpdatesReceivedBeforeSync();
	CTickProxy::sendSyncs();
}


/*
 * Master tick received (see tick proxy)
 */
void cbOnMasterTick()
{
	MSInstance->masterTick();
}


/*
 * Callback called when a client tocks back
 */
void cbDoNextTask()
{
	MSInstance->doNextTask();
}


/*
 * Return true if all delta updates have been received for the current tick
 */
inline bool CMirrorService::checkIfAllIncomingDeltasReceived() const
{
	return _NbDeltaUpdatesReceived == _NbExpectedDeltaUpdates;
}


/*
 * Advance state in the mirror automaton
 */
void CMirrorService::doNextTask()
{
	if ( ! _DeltaSent )
	{
		if ( CTickProxy::checkIfAllClientTocksReceived() &&
			 checkIfAllATAcknowledgesReceived() )
		{
			// Advance to step 3.
			buildAndSendAllOutgoingDeltas();
			nlassert( _DeltaSent );

			// Advance to step 4 if the required conditions are met.
			BeginWaitForAllDeltaReceivedTime = CTime::getLocalTime();
			doNextTask();
		}
	}
	else
	{
		if ( checkIfAllIncomingDeltasReceived() ) // 
		{
			// Advance to step 4.
			CTickProxy::TimeMeasures.MirrorMeasure[WaitAllReceivedDeltaDuration] = (uint16)(CTime::getLocalTime() - BeginWaitForAllDeltaReceivedTime);
			nlassert( CTickProxy::checkIfAllClientTocksReceived() );
			tockMasterAndProcessReceivedIncomingDeltas();
			nlassert( ! CTickProxy::checkIfAllClientTocksReceived() );
		}
		else /*if ( ! checkIfAllIncomingDeltasReceived() )*/
		{
			// Shouldn't be negative
			if ( _NbExpectedDeltaUpdates-_NbDeltaUpdatesReceived < 0 )
				nlwarning( "More delta updates received than expected: E%u R%u", _NbExpectedDeltaUpdates, _NbDeltaUpdatesReceived );
			//nldebug( "Delta updates remaining: %hd", _NbExpectedDeltaUpdates-_NbDeltaUpdatesReceived );
		}
	}
}


/*
 *
 */
void CMirrorService::displayAutomatonState( NLMISC::CLog *log )
{
	// TODO: add checkIfAllATAcknowledgesReceived()/_BlockedAwaitingATAck
	log->displayNL( "Tick %u, %s", CTickProxy::getGameCycle(), (CTickProxy::State == ExpectingMasterTick)?"expecting new tick":"expecting local tocks" );
	if ( CTickProxy::State == ExpectingMasterTick )
	{
		if ( _DeltaSent )
			log->displayNL( "Delta sent; Master tock sent; => Steps 5-6" );
		else
			log->displayNL( "Ready for step 1" );
	}
	else if ( ! _DeltaSent )
	{
		if ( CTickProxy::checkIfAllClientTocksReceived() )
			log->displayNL( "Delta not sent; Received all tocks => Ready for step 3" );
		else
			log->displayNL( "Delta not sent; Waiting for client tocks => Step 2" );
	}
	else
	{
		if ( checkIfAllIncomingDeltasReceived() )
			log->displayNL( "Delta sent; Received all deltas => Ready for step 4" );
		else
			log->displayNL( "Delta sent; Waiting for deltas (%hd received on %hd) => Step 3.5", _NbDeltaUpdatesReceived, _NbExpectedDeltaUpdates );
	}
}


/*
 *
 */
void CMirrorService::buildAndSendAllOutgoingDeltas()
{
	// 3. Build and send delta to remote MSs
	buildAndSendAllDeltas();
	_DeltaSent = true;

	// Apply release of client services
	applyServicesQuitting();

	// Appply pending rescans
	applyPendingRescans();
}


/*
 *
 */
void CMirrorService::tockMasterAndProcessReceivedIncomingDeltas()
{
	// 4. Send master TOCK to Tick Service
	CTickProxy::sendMasterTock();

	// 5. Apply pending deltas to shared memory
	applyPendingDeltas();

	TTime BeforeSendUMM = CTime::getLocalTime();

	// 6. Trigger processing of mirror changes and send messages to client services
	tellLocalServicesAndSendMessages();

	// Reset incoming delta received counter and outgoing delta sent flag
	resetDeltaUpdatesReceived();
	resetDeltaSent();

	CTickProxy::setEndOfTick();

	CTickProxy::TimeMeasures.MirrorMeasure[PrevSendUMMDuration] = (uint16)(CTime::getLocalTime() - BeforeSendUMM);
}


/*
 * Send ticks to local client services
 */
void CMirrorService::masterTick()
{
	// 1. Send TICK to local client services
	CTickProxy::sendTicks();

	doNextTask();
}


/*
 * 
 */
void CMirrorService::pushMessageToLocalQueue( std::vector<TMessageCarrier>& msgQueue, TServiceId senderId, NLMISC::CMemStream& msg )
{
	TMessageCarrier mc( true );
	msgQueue.push_back( mc );
	msgQueue.back().SenderId = senderId;
	msgQueue.back().Msg = msg;
	//nldebug( "Received msg from %s for local %s", servStr(senderId).c_str(), servStr(destId).c_str() );
}


/*
 *
 */
void CMirrorService::pushMessageToLocalQueueFromMessage( std::vector<TMessageCarrier>& msgQueue, NLNET::CMessage& msgin )
{
	TMessageCarrier mc( true );
	msgQueue.push_back( mc );
	msgin.serial( msgQueue.back().SenderId );
	msgin.serialMemStream( msgQueue.back().Msg );
}


/*
 *
 */
void CMirrorService::receiveMessageToForwardFromClient( CMessage& msgin, TServiceId senderId )
{
	H_AUTO(receiveMessageToForwardFromClient);

	uint8 counter;
	msgin.serial( counter ); // there are no multiple messages, but there can be multiple destinations
	if ( counter == MSG_BROADCAST )
	{
		CMemStream ms = msgin.extractStreamFromPos( msgin.getHeaderSize() + sizeof(counter) );
#ifdef NL_DEBUG
		string name = msgin.readTypeAtCurrentPos(); // warning: the pos is updated in msgin
		nldebug( "MSG: Broadcasting message from %s (msg %s, %u b)", servStr(senderId).c_str(), name.c_str(), ms.length() );
#endif
		pushMessageToLocalQueue( _MessagesToBroadcastLocally, senderId, ms );
		_RemoteMSList.pushMessageToRemoteQueue( DEST_MSG_BROADCAST, senderId, ms );
	}
	else
	{
		for ( uint i=0; i!=(uint)counter; ++i )
		{
			TServiceId destId;
			msgin.serial( destId );
			TClientServices::iterator ics = _ClientServices.find( destId );
			if ( ics != _ClientServices.end() )
			{
				// Destination service is local => buffer the message (which is found in the stream after the list of destinations)
				CMemStream m = msgin.extractStreamFromPos( msgin.getHeaderSize() + sizeof(counter) + counter*sizeof(destId) );
				pushMessageToLocalQueue( GET_CLIENT_SERVICE_INFO(ics).Messages, senderId, m );
			}
			else
			{
				// Destination service is remote => find the corresponding remote MS and buffer the message
				CMemStream m = msgin.extractStreamFromPos( msgin.getHeaderSize() + sizeof(counter) + counter*sizeof(destId) );
				_RemoteMSList.pushMessageToRemoteQueue( (TServiceId)destId, (TServiceId)senderId, m );
			}
		}
	}
}


/*
 *
 */
void CMirrorService::receiveDeltaFromRemoteMS( CMessage& msgin, TServiceId senderMSId )
{
	H_AUTO(receiveDeltaFromRemoteMS);

	// 2b. Receive delta updates from remote MSs => store them in pending deltas
	_PendingDeltas.push_back( make_pair( senderMSId, msgin ) ); // warning: msgin must be copied because below we advance the reading position of msgin
	//nldebug( "Received delta from %s", servStr(senderMSId).c_str() );

	// Get the "wait for" mode of this remote MS delta
	bool waitForIt;
	msgin.serial( waitForIt );
	if ( waitForIt ) // we assume it does not change during the lifetime of a particular remote MS
	{
		// Check if the sender is a new remote MS (search in the "delta update expectation list")
		std::list< TServiceId >::const_iterator it = find( _RemoteMSToWaitForDelta.begin(), _RemoteMSToWaitForDelta.end(), senderMSId );
		if ( it == _RemoteMSToWaitForDelta.end() )
		{
			_RemoteMSToWaitForDelta.push_back( senderMSId ); // _RemoteMSToWaitForDelta will not contain remote MSes for which we don't have to wait
			++_NbExpectedDeltaUpdates;
		}
	}

	TGameCycle gamecycle;
	msgin.serial( gamecycle );
	//nldebug( "%u: Rcvd Delta %u", CTickProxy::getGameCycle(), gamecycle );
	_QuickLog.displayNL( "TCK-%u: Rcvd Delta %u from MS-%hu (NbExpectedDelta=%hd)", CTickProxy::getGameCycle(), gamecycle, senderMSId.get(), _NbExpectedDeltaUpdates );

	// Count this delta update for the wait test only if its tick is the current tick or +1 (see explanation below)
	// Explanation: possibles cases:
	//
	//                  MS1                  MS2
	//                   |                    |
	//             Tick1>|              Tick1>|
	//             Delta>|>Delta <====> Delta>|>Delta
	//                   |>Tock1              |>Tock1
	//                   |                    |
	//                   |              Tick2>|
	//             Delta>|       <-----       |>Delta
	//             Tick2>|>Delta -----> Delta>|>Tock2
	//                   |>Tock2              |
	//
	if ( CTickProxy::State == ExpectingMasterTick )
	{
		// If sync not received yet, defer possible incrementation of _NbDeltaUpdatesReceived
		// because we can't compare now the gamecycle of the delta update with the current gamecycle
		// (see examineDeltaUpdatesReceivedBeforeSync() called when receiving the sync)
		if ( CTickProxy::getGameCycle() == 0 )
		{
			if ( waitForIt )
				_GameCyclesOfDeltaReceivedBeforeSync.push_back( gamecycle );
			return;
		}
		// Accept +1 only if received by advance (before the tick update)
		// Forbid >+1
		// Discard <=
		else if ( gamecycle > CTickProxy::getGameCycle() )
		{
			nlassert( gamecycle == CTickProxy::getGameCycle() + 1 );
			if ( waitForIt )
				++_NbDeltaUpdatesReceived;
		}
	}
	else
	{
		// Accept == if received in the same tick
		// Forbid >
		// Discard <
		if ( gamecycle >= CTickProxy::getGameCycle() )
		{
			nlassert( gamecycle == CTickProxy::getGameCycle() );
			if ( waitForIt )
				++_NbDeltaUpdatesReceived;
		}
	}

	doNextTask();
}


/*
 * Increment _NbDeltaUpdatesReceived for the number of times we have received a VALID delta update before sync
 * (valid = its tick = the current tick + 1)
 */
void CMirrorService::examineDeltaUpdatesReceivedBeforeSync()
{
	if ( ! _GameCyclesOfDeltaReceivedBeforeSync.empty() )
	{
		for ( vector<TGameCycle>::const_iterator ig=_GameCyclesOfDeltaReceivedBeforeSync.begin(); ig!=_GameCyclesOfDeltaReceivedBeforeSync.end(); ++ig )
		{
			if ( (*ig) > CTickProxy::getGameCycle() )
				++_NbDeltaUpdatesReceived;
		}
		_GameCyclesOfDeltaReceivedBeforeSync.clear();

		doNextTask();
	}
}


/*
 *
 */
void CMirrorService::applyPendingDeltas()
{
	H_AUTO(applyPendingDeltas);

	TTime BeforeApplyPendingDelta = CTime::getLocalTime();

	// Binding counters (possibly with removals)
	for ( TSDataSetsMS::iterator ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		GET_SDATASET(ids).processReceivedBindingCounters();
	}

	// Deltas
	vector< pair<TServiceId,CMessage> >::iterator ipd;
	for ( ipd=_PendingDeltas.begin(); ipd!=_PendingDeltas.end(); ++ipd )
	{
//		processReceivedDelta( const_cast<CMessage&>((*ipd).second), (*ipd).first );
		processReceivedDelta( ipd->second, ipd->first );
	}
	//if ( ! _PendingDeltas.empty() )
	//	nldebug( "Applied %u pending deltas", _PendingDeltas.size() );
	_PendingDeltas.clear();

	CTickProxy::TimeMeasures.MirrorMeasure[PrevApplyReceivedDeltaDuration] = (uint16)(CTime::getLocalTime() - BeforeApplyPendingDelta);
}


/*
 *
 */
void CMirrorService::tellLocalServicesAndSendMessages()
{
	H_AUTO(tellUMM)
	TClientServices::iterator ics;
	for ( ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	{
		//H_BEFORE(tellHeaderAndBroadcast);
		CMessage msgout( "UMM" ); // Update Mirror and Messages
		sint32 nbBufPos = msgout.reserve( sizeof(uint32) );
		uint32 nbBroadcastedMsgs = 0;
		vector<TMessageCarrier>::const_iterator im;
		for ( im=_MessagesToBroadcastLocally.begin(); im!=_MessagesToBroadcastLocally.end(); ++im )
		{
			if ( (*im).SenderId != (*ics).first ) // don't send to sender
			{
				serialToMessageFromLocalQueue( msgout, (*im) );
				++nbBroadcastedMsgs;
			}
		}
		//H_AFTER(tellHeaderAndBroadcast);
		for ( im=GET_CLIENT_SERVICE_INFO(ics).Messages.begin(); im!=GET_CLIENT_SERVICE_INFO(ics).Messages.end(); ++im )
		{
			serialToMessageFromLocalQueue( msgout, (*im) );
		}
		uint32 nbMsgs = nbBroadcastedMsgs + (uint32)GET_CLIENT_SERVICE_INFO(ics).Messages.size();
		msgout.poke( nbMsgs, nbBufPos );
		//H_BEFORE(tellUMMSend);
		CUnifiedNetwork::getInstance()->send( (*ics).first, msgout );
		//H_AFTER(tellUMMSend);
		//if ( nbMsgs != 0 )
		//	nldebug( "Sent UMM to %s with %u messages", servStr((*ics).first).c_str(), nbMsgs );
		//H_AUTO(clearMessages);
		GET_CLIENT_SERVICE_INFO(ics).Messages.clear();
	}
	_MessagesToBroadcastLocally.clear();
}


/*
 *
 */
void CMirrorService::serialToMessageFromLocalQueue( CMessage& msgout, const TMessageCarrier& srcMsgInQueue )
{
	//H_AUTO(serialToMessageFromLocalQueue);
	msgout.serial( const_cast<TServiceId&>(srcMsgInQueue.SenderId) );

	// Not using serialMemStream because the receiver will read the size and pass the entire message to the appropriate callback
	sint32 msgSize = (sint32)srcMsgInQueue.Msg.length();
	msgout.serial( msgSize );
	msgout.serialBuffer( const_cast<uint8*>(srcMsgInQueue.Msg.buffer()), srcMsgInQueue.Msg.length() );
}


NLMISC_COMMAND( displayAutomatonState, "Display the current state of the MS automaton", "" )
{
	MSInstance->displayAutomatonState( &log );
	return true;
}

NLMISC_COMMAND( doNextTask, "Update automaton state", "" )
{
	MSInstance->doNextTask();
	return true;
}
