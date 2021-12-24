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

#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/variable.h>
#include <nel/misc/o_xml.h>

#include "impulse_encoder.h"
#include "client_host.h"
#include "game_share/action_factory.h"
#include "game_share/tick_event_handler.h"

using namespace CLFECOMMON;
using namespace NLMISC;

TImpulseReceivedCallback	CImpulseEncoder::_Callbacks[256];

uint MaxImpulseBitSizes [3];

void cbImpulsionByteSize0Changed( IVariable& var );
void cbImpulsionByteSize1Changed( IVariable& var );
void cbImpulsionByteSize2Changed( IVariable& var );

CVariable<uint> ImpulsionByteSize0( "fe", "ImpulsionByteSize0", "Size of impulsion channel 0", 20, 0, true, cbImpulsionByteSize0Changed, true );
CVariable<uint> ImpulsionByteSize1( "fe", "ImpulsionByteSize1", "Size of impulsion channel 1", 200, 0, true, cbImpulsionByteSize1Changed, true );
CVariable<uint> ImpulsionByteSize2( "fe", "ImpulsionByteSize2", "Size of impulsion channel 0", 200, 0, true, cbImpulsionByteSize2Changed, true );

void cbImpulsionByteSize0Changed( IVariable& var )
{
	// -IMPULSE_ACTION_HEADER_BITSIZE not needed, as actions know their own size
	// and handle it right
	MaxImpulseBitSizes[0] = (ImpulsionByteSize0.get())*8; // - IMPULSE_ACTION_HEADER_BITSIZE;
}

void cbImpulsionByteSize1Changed( IVariable& var )
{
	MaxImpulseBitSizes[1] = (ImpulsionByteSize1.get())*8; // - IMPULSE_ACTION_HEADER_BITSIZE;
}

void cbImpulsionByteSize2Changed( IVariable& var )
{
	MaxImpulseBitSizes[2] = (ImpulsionByteSize2.get())*8; // - IMPULSE_ACTION_HEADER_BITSIZE;
}


bool verboseImpulsions = false;

//
uint32		CImpulseQueue::send( TActionQueue& sourceQueue, uint32 packet, NLMISC::CBitMemStream &outbox, uint32 &sentActions)
{
	bool	wasFlushed = false;
	{
		H_AUTO(ImpulseQueueSend1);

		// If the current sending impulse queue is empty (flushed), fill it with actions from the source queue
		if ( Queue.empty() )
		{
			wasFlushed = true;
			FirstSent = packet;
			uint actSize;
			while ( (!sourceQueue.empty()) && // stop if there are no more actions in the source queue
					( 
						// stop if the max size is reached
						(TotalBitsInImpulseQueue + (actSize=CActionFactory::getInstance()->size(sourceQueue.front())) < MaxBitSize) ||
						// continue if the action can exceed the total size (only one!)
						(sourceQueue.front()->AllowExceedingMaxSize && Queue.empty())
					)
				  )
			{
				// Move action from the source queue to the current sending impulse queue
				Queue.push_back( sourceQueue.front() );
				//nlinfo("add action '%d' size '%d' in queue %d:%d", sourceQueue.front()->Code, actSize, Level, Channel);
				TotalBitsInImpulseQueue += actSize;
				sourceQueue.pop_front();
			}
		}
	}

	uint	i;

	bool	impulsionFollowBit = true;
	uint32	serialised = 0;

	{
		H_AUTO(ImpulseQueueSend2);
		// Send all the current sending impulse queue
		for ( i=0; i<Queue.size(); ++i )
		{
			// serial channel swap bit
			outbox.serialBitAndLog(impulsionFollowBit);

			// serial marked actions
			CActionFactory::getInstance()->pack( Queue[i], outbox, false );
	#ifdef NL_DEBUG
			if ( verboseImpulsions )
				nldebug( "Sending effectively impulsion %p (len=%u)", Queue[i], CActionFactory::getInstance()->size(Queue[i]) );
	#endif
			serialised += (1+CActionFactory::getInstance()->size(Queue[i]));
			++sentActions;
		}

		if ( ! Queue.empty() )
			LOG_IMPULSION_DEBUG("FEIMP: %s %d actions (%u bits) to send on Channel %d at Level %d", (wasFlushed) ? "sent new" : "resent", Queue.size(), serialised+1, Channel, Level);
	}

	{
		H_AUTO(ImpulseQueueSend3);
		bool	impulsionEndBit = false;
		outbox.serialBitAndLog(impulsionEndBit);
	}

	//nlinfo("serialised %d bits", TotalBitsInImpulseQueue);

	return serialised+1;
}

//
void	CImpulseQueue::flush(uint packet, CClientHost *client, std::vector<uint8> &impcounts)
{
	if (packet < FirstSent)
		return;

//	LOG_IMPULSION_DEBUG("FEIMP: flush %d actions from Channel %d at Level %d", QueueMark, Channel, Level);

	// Call acknowledge callback for each action in the impulse queue, then clear the queue
	for ( uint i=0; i<Queue.size(); ++i )
	{
		CAction	*action = Queue[i];

#ifdef TRACE_SHARD_MESSAGES
		// Measure the latency between adding and acknowledgement
		LOG_IMPULSION_INFO( "FEIMP: Ack impulsion %p (latency: %u cycles)", Queue[i], CTickEventHandler::getGameCycle()-action->GameCycle );
#endif
		// call associated callback
		if (CImpulseEncoder::_Callbacks[action->Code] != NULL)
			CImpulseEncoder::_Callbacks[action->Code] (client, action);
		--impcounts[action->Slot];
		CActionFactory::getInstance()->remove(action);
	}
	Queue.clear();

	// stat part
	countEffectiveSent(TotalBitsInImpulseQueue);

	TotalBitsInImpulseQueue = 0;
	++FlushTimes;
}


// Get Current Effective Send Rate (based on flush rate) (in bits per second)
uint	CImpulseQueue::effectiveSendRate()
{
	flushSendRateQueue(CTime::getLocalTime());

	uint	totalBitSent = 0;
	uint	i;
	for (i=0; i<SendRateQueue.size(); ++i)
		totalBitSent += SendRateQueue[i].Size;

	return totalBitSent / IMPULSE_STAT_MEAN_TIME;
}

// Get Current Effective Send Rate (based on flush rate) (in bits per second)
uint64	CImpulseQueue::totalSentData()
{
	return SentData;
}





/*
 * Constructor
 */
CImpulseEncoder::CImpulseEncoder()
{
	uint	i;

	for (i=0; i<1; ++i)		_Level0[i].setup(maxBitSize(0), 0, i);
	for (i=0; i<2; ++i)		_Level1[i].setup(maxBitSize(1), 1, i);
	for (i=0; i<4; ++i)		_Level2[i].setup(maxBitSize(2), 2, i);

	_QueuedImpulses.resize(256, 0);

	_TotalPackets = 0;
}


//
void	CImpulseEncoder::add(CActionImpulsion *action, uint level)
{
	nlassert(level < 3);

#ifdef TRACE_SHARD_MESSAGES
	// Set the timestamp
	action->GameCycle = CTickEventHandler::getGameCycle();
#endif

	++_QueuedImpulses[action->Slot];

	countAddedAction(action, level);

/*
	CImpulseQueue	*table = NULL;
	switch (level)
	{
	case 0:	table = _Level0; break;
	case 1:	table = _Level1; break;
	case 2:	table = _Level2; break;
	}

	if (channel == -1)
	{
		uint	i, min = -1;
		channel = 0;
		uint nbQueues = (uint)(1<<level);
		for (i=0; i<nbQueues; ++i)
		{
			if (table[i].Queue.size() < min)
			{
				min = table[i].Queue.size();
				channel = i;
			}
		}
	}

	nlassert(channel >= 0);
	nlassertex(channel < (1<<level), ("channel=%d level=%d", channel, level) );

	table[channel].add(action);
*/

	//nlassert(channel == -1);

	_MainQueues[level].push_back(action);

	LOG_IMPULSION_DEBUG("FEIMP: added 1 action (%d) at Level %d", action->Code, level);
}

//
uint32	CImpulseEncoder::send(uint32 packet, NLMISC::CBitMemStream &outbox, uint32 &sentActions)
{
	//LOG_IMPULSION_DEBUG("FEIMP: send() packet %d", packet);

	uint32	serialised = 0;
	sentActions = 0;

	// Send one queue for each level
	serialised += _Level0[0].send( _MainQueues[0], packet, outbox, sentActions );
	serialised += _Level1[packet&1].send( _MainQueues[1], packet, outbox, sentActions );
	serialised += _Level2[packet&3].send( _MainQueues[2], packet, outbox, sentActions );

	++_TotalPackets;

	return serialised;
}

//
void	CImpulseEncoder::ack(uint32 packet)
{
	//LOG_IMPULSION_DEBUG("FEIMP: ack() packet %d", packet);

	_Level0[0].flush(packet, _ClientHost, _QueuedImpulses);
	_Level1[packet&1].flush(packet, _ClientHost, _QueuedImpulses);
	_Level2[packet&3].flush(packet, _ClientHost, _QueuedImpulses);
}

//
uint	CImpulseEncoder::queueSize(uint level) const
{
	nlassert(level < 3);

	uint				channel, 
						maxChannel = (1<<level);
	const CImpulseQueue	*channels;

	switch (level)
	{
	case 0: channels = _Level0; break;
	case 1: channels = _Level1; break;
	case 2: channels = _Level2; break;
	}

	uint	size = (uint)_MainQueues[level].size();

	for (channel=0; channel<maxChannel; ++channel)
		size += (uint)channels[channel].Queue.size();

	return size;
}

//
uint	CImpulseEncoder::queueSize() const
{
	uint	level, 
			size = 0;

	for (level=0; level<3; ++level)
		size += queueSize(level);

	return size;
}

// Get Enqueued actions size in bits for a specified level
uint	CImpulseEncoder::getEnqueuedSize(uint level) const
{
	nlassert(level < 3);
	const TActionQueue&	queue = _MainQueues[level];

	uint	bitSize = 0;
	uint	i;
	for (i=0; i<queue.size(); ++i)
		bitSize += CActionFactory::getInstance()->size(queue[i]);

	return bitSize;
}


//
void	CImpulseEncoder::reset()
{
	_MainQueues[0].clear();
	_MainQueues[1].clear();
	_MainQueues[2].clear();

	uint	i;
	for (i=0; i<1; ++i)		_Level0[i].reset();
	for (i=0; i<2; ++i)		_Level1[i].reset();
	for (i=0; i<4; ++i)		_Level2[i].reset();
}

//
void	CImpulseEncoder::unmarkAll()
{
	uint	i;
	for (i=0; i<1; ++i)		_Level0[i].reinit();
	for (i=0; i<2; ++i)		_Level1[i].reinit();
	for (i=0; i<4; ++i)		_Level2[i].reinit();
}

//
void	CImpulseEncoder::setReceivedCallback(TImpulseReceivedCallback cb, sint actionCode)
{
	if (actionCode == -1)
	{
		uint	i;
		for (i=0; i<256; ++i)
			_Callbacks[i] = cb;
	}
	else
	{
		nlassert(actionCode >= 0);
		nlassert(actionCode < 256);

		_Callbacks[actionCode] = cb;
	}
}

//
void	CImpulseEncoder::removeEntityReferences(CLFECOMMON::TCLEntityId id)
{
	if (hasEntityReferences(id))
	{
		TActionQueue::iterator	it;
		for (it=_MainQueues[0].begin(); it<_MainQueues[0].end(); )
			if ((*it)->Slot == id)
				CLFECOMMON::CActionFactory::getInstance()->remove(*it), it = _MainQueues[0].erase(it);
			else
				++it;
		for (it=_MainQueues[1].begin(); it<_MainQueues[1].end(); )
			if ((*it)->Slot == id)
				CLFECOMMON::CActionFactory::getInstance()->remove(*it), it = _MainQueues[1].erase(it);
			else
				++it;
		for (it=_MainQueues[2].begin(); it<_MainQueues[2].end(); )
			if ((*it)->Slot == id)
				CLFECOMMON::CActionFactory::getInstance()->remove(*it), it = _MainQueues[2].erase(it);
			else
				++it;

		uint	i;
		for (i=0; i<1; ++i)		_Level0[i].removeReferences(id);
		for (i=0; i<2; ++i)		_Level1[i].removeReferences(id);
		for (i=0; i<4; ++i)		_Level2[i].removeReferences(id);
		_QueuedImpulses[id] = 0;
	}
}


// Dump stats to XML stream
void	CImpulseEncoder::dump(NLMISC::IStream& s)
{
	s.xmlPushBegin("ImpulseStat");

	s.xmlSetAttrib("id");
	uint32		id = _ClientHost->clientId();
	s.serial(id);

	s.xmlSetAttrib("eid");
	CEntityId	eid = _ClientHost->eId();
	s.serial(eid);

	s.xmlSetAttrib("totalPacketSent");
	s.serial(_TotalPackets);

	s.xmlPushEnd();

	s.xmlPushBegin("Level0");
	s.xmlSetAttrib("efficiency");
	float	efficiency = efficiencyRatio(0);
	s.serial(efficiency);
	s.xmlSetAttrib("stillEnqueued");
	uint32	enqueuedSize = getEnqueuedSize(0);
	s.serial(enqueuedSize);
	s.xmlSetAttrib("addRate");
	uint32	addRate = effectiveAddRate(0);
	s.serial(addRate);
	s.xmlPushEnd();
	_Level0[0].dump(s);
	s.xmlPop();

	s.xmlPushBegin("Level1");
	s.xmlSetAttrib("efficiency");
	efficiency = efficiencyRatio(1);
	s.serial(efficiency);
	s.xmlSetAttrib("stillEnqueued");
	enqueuedSize = getEnqueuedSize(1);
	s.serial(enqueuedSize);
	s.xmlSetAttrib("addRate");
	addRate = effectiveAddRate(1);
	s.serial(addRate);
	s.xmlPushEnd();
	_Level1[0].dump(s);
	_Level1[1].dump(s);
	s.xmlPop();

	s.xmlPushBegin("Level2");
	s.xmlSetAttrib("efficiency");
	efficiency = efficiencyRatio(2);
	s.serial(efficiency);
	s.xmlSetAttrib("stillEnqueued");
	enqueuedSize = getEnqueuedSize(2);
	s.serial(enqueuedSize);
	s.xmlSetAttrib("addRate");
	addRate = effectiveAddRate(2);
	s.serial(addRate);
	s.xmlPushEnd();
	_Level2[0].dump(s);
	_Level2[1].dump(s);
	_Level2[2].dump(s);
	_Level2[3].dump(s);
	s.xmlPop();

	s.xmlPop();
}

// Count Added action
void	CImpulseEncoder::countAddedAction(CLFECOMMON::CActionImpulsion *action, uint level)
{
	TSendRateQueue&	queue = _AddRateQueues[level];

	NLMISC::TTime	ctime = NLMISC::CTime::getLocalTime();
	while (!queue.empty() && ctime > queue.front().Time+(IMPULSE_STAT_MEAN_TIME*1000))
		queue.pop_front();

	queue.push_back(CTimedSize(ctime, CActionFactory::getInstance()->size(action)));
}


// Effective Add Rate
uint	CImpulseEncoder::effectiveAddRate(uint level)
{
	TSendRateQueue&	queue = _AddRateQueues[level];

	NLMISC::TTime	ctime = NLMISC::CTime::getLocalTime();
	while (!queue.empty() && ctime > queue.front().Time+(IMPULSE_STAT_MEAN_TIME*1000))
		queue.pop_front();

	uint	totalBitAdded = 0;
	uint	i;
	for (i=0; i<queue.size(); ++i)
		totalBitAdded += queue[i].Size;

	return totalBitAdded / IMPULSE_STAT_MEAN_TIME;
}

// Effective Send Rate (cumulated send of each channel)
uint	CImpulseEncoder::effectiveSendRate(uint level)
{
	CImpulseQueue	*channels;

	switch (level)
	{
	case 0: channels = _Level0; break;
	case 1: channels = _Level1; break;
	case 2: channels = _Level2; break;
	}

	uint	rate = 0;

	uint	channel, maxChannel = (1<<level);
	for (channel=0; channel<maxChannel; ++channel)
		rate += channels[channel].effectiveSendRate();

	return rate;
}


// Efficiency Ratio
float	CImpulseEncoder::efficiencyRatio(uint level)
{
	uint	addRate = effectiveAddRate(level);
	uint	sendRate = effectiveSendRate(level);

	return (addRate != 0) ? ((float)sendRate/(float)addRate) : 0.0f;
}

// Get least efficiency
float	CImpulseEncoder::leastEfficiencyRatio()
{
	float	l0 = efficiencyRatio(0);
	float	l1 = efficiencyRatio(1);
	float	l2 = efficiencyRatio(2);
	float	less = 1000.0f;

	if (l0 > 0 && l0 < less)	less = l0;
	if (l1 > 0 && l1 < less)	less = l1;
	if (l2 > 0 && l2 < less)	less = l2;

	return less;
}

// Get biggest queue size
uint	CImpulseEncoder::biggestQueueSize()
{
	uint	l0 = getEnqueuedSize(0);
	uint	l1 = getEnqueuedSize(1);
	uint	l2 = getEnqueuedSize(2);
	uint	more = 0;

	if (l0 > more)	more = l0;
	if (l1 > more)	more = l1;
	if (l2 > more)	more = l2;

	return more;
}



//
void	CImpulseQueue::dump(NLMISC::IStream& s)
{
	s.xmlPushBegin("QueueStat");

	s.xmlSetAttrib("channel");
	s.serial(Channel);

	s.xmlSetAttrib("maxBitSize");
	s.serial(MaxBitSize);

	s.xmlSetAttrib("sendRate");
	uint32	sendRate = effectiveSendRate();
	s.serial(sendRate);

	s.xmlSetAttrib("sentData");
	uint64	sentData = totalSentData();
	s.serial(sentData);

	s.xmlSetAttrib("effectiveSend");
	uint32	effectiveSend = FlushTimes;
	s.serial(effectiveSend);

	s.xmlPushEnd();

	s.xmlPush("SendQueue");
	for (uint i=0; i<SendRateQueue.size(); ++i)
		s.serial(SendRateQueue[i]);
	s.xmlPop();

	s.xmlPop();
}



void	CTimedSize::serial(NLMISC::IStream& s)
{
	s.xmlPushBegin("TimedSize");

	s.xmlSetAttrib("time");
	NLMISC::TTime	dtime = NLMISC::CTime::getLocalTime()-Time;
	s.serial(dtime);

	s.xmlSetAttrib("size");
	s.serial(Size);

	s.xmlPushEnd();
	s.xmlPop();
}


/*
 * Destructor
 */
CImpulseQueue::~CImpulseQueue()
{
	for ( TActionQueue::iterator it=Queue.begin(); it!=Queue.end(); ++it )
	{
		CLFECOMMON::CActionFactory::getInstance()->remove( *it );
	}
}


/*
 * Destructor
 */
CImpulseEncoder::~CImpulseEncoder()
{
	for ( uint i=0; i!=3; ++i )
	{
		TActionQueue& queue = _MainQueues[i];

		for ( TActionQueue::iterator it=queue.begin(); it!=queue.end(); ++it )
		{
			CActionFactory::getInstance()->remove( *it );
		}
	}
}
