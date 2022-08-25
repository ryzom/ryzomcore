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



#ifndef NL_IMPULSE_ENCODER_H
#define NL_IMPULSE_ENCODER_H

#include <vector>
#include <deque>

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"

//#include "../common/action.h"
#include "game_share/action_factory.h"

extern bool verboseImpulsions;

#ifdef NL_RELEASE
#define LOG_IMPULSION_INFO ;
#define LOG_IMPULSION_DEBUG ;
#else
#define LOG_IMPULSION_INFO if (!verboseImpulsions) {} else nlinfo
#define LOG_IMPULSION_DEBUG if (!verboseImpulsions) {} else nldebug
#endif


class CClientHost;

namespace CLFECOMMON
{
	class CAction;
}

typedef void	(*TImpulseReceivedCallback)(CClientHost *, CLFECOMMON::CAction *);


// CActionFactory header + CActionGeneric header + 1/8 (the follow bit)
const sint IMPULSE_ACTION_HEADER_BITSIZE = 4*8 + 4*8 + 1;
extern uint MaxImpulseBitSizes [3];


class CClientImpulseStat
{
public:

};

const uint		IMPULSE_STAT_MEAN_TIME = 30;	// seconds
const double	IMPULSE_STAT_SMOOTH = 2.0;


class CTimedSize
{
public:

	CTimedSize()	{}
	CTimedSize(NLMISC::TTime t, uint32 s) : Time(t), Size(s)	{}

	NLMISC::TTime	Time;
	uint32			Size;

	void		serial(NLMISC::IStream& s);
};
typedef std::deque<CTimedSize>	TSendRateQueue;


/**
 * An Impulse Queue
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CImpulseQueue
{
public:
	typedef std::deque<CLFECOMMON::CActionImpulsion *>	TActionQueue;
	TActionQueue					Queue;


	/// \name Stat Part
	// @{

	TSendRateQueue					SendRateQueue;
	uint64							SentData;
	uint32							FlushTimes;

	// @}

	uint32							MaxBitSize;
	uint							TotalBitsInImpulseQueue;
	uint							FirstSent;

	uint32							Level, Channel;

public:
	CImpulseQueue() : TotalBitsInImpulseQueue(0), FirstSent(0), SentData(0), FlushTimes(0) {}

	/// Destructor
	~CImpulseQueue();

	void							setup(uint maxBitSize, uint level, uint channel) { MaxBitSize = maxBitSize; Level = level; Channel = channel; }
	void							add(CLFECOMMON::CActionImpulsion *action) { Queue.push_back(action); }

	void							flush(uint packet, CClientHost *client, std::vector<uint8> &impcounts);

	uint32							send(TActionQueue& sourceQueue, uint32 packet, NLMISC::CBitMemStream &outbox, uint32 &sentActions);

	void							reinit() { FirstSent = 0; }

	void							reset()
	{
		reinit();
		uint	i;
		for (i=0; i<Queue.size(); ++i)
		{
			CLFECOMMON::CActionFactory::getInstance()->remove(Queue[i]);
		}
		Queue.clear();
		TotalBitsInImpulseQueue = 0;
		SentData = 0;
	}

	void							removeReferences(CLFECOMMON::TCLEntityId id)
	{
		TActionQueue::iterator	it;
		for (it=Queue.begin(); it<Queue.end(); )
			if ((*it)->Slot == id)
			{
				CLFECOMMON::CActionFactory::getInstance()->remove(*it);
				it = Queue.erase(it);
			}
			else
			{
				++it;
			}
	}


	/// \name Stat Part
	// @{

	/// Count next effective sent
	void		countEffectiveSent(uint bitSent)
	{
		NLMISC::TTime	ctime = NLMISC::CTime::getLocalTime();
		flushSendRateQueue(ctime);

		if (bitSent == 0)
			return;

		//nlinfo("sent effectively %d bits in level %d, channel %d", bitSent, Level, Channel);

		SendRateQueue.push_back(CTimedSize(ctime, bitSent));
		SentData += bitSent;
	}

	/// Flush SendRateQueue
	void		flushSendRateQueue(NLMISC::TTime ctime)
	{
		while (!SendRateQueue.empty() && ctime > SendRateQueue.front().Time+(IMPULSE_STAT_MEAN_TIME*1000))
			SendRateQueue.pop_front();
	}

	/// Get Current Effective Send Rate (based on flush rate) (in bits per second)
	uint		effectiveSendRate();

	/// Total send data in this queue (based on flush rate) (in bits per second)
	uint64		totalSentData();

	/// Dump queue to xml stream
	void		dump(NLMISC::IStream& s);

	// @}


};


/**
 * The impulse encoder, used to post important actions the client must receive.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CImpulseEncoder
{
private:

	/*
	 * Warning: the numbers of queues by channels is hardcoded in the acknowledgement system
	 */

	CImpulseQueue					_Level0[1];
	CImpulseQueue					_Level1[2];
	CImpulseQueue					_Level2[4];

	CClientHost						*_ClientHost;

	//uint							_MaxBitSizes[4];

	std::vector<uint8>				_QueuedImpulses;

	typedef std::deque<CLFECOMMON::CActionImpulsion *>	TActionQueue;

	TActionQueue					_MainQueues[3];

	uint32							_TotalPackets;

protected:
	friend class CImpulseQueue;

	static TImpulseReceivedCallback	_Callbacks[256];

	TSendRateQueue					_AddRateQueues[3];

public:

	/// Constructor
	CImpulseEncoder();

	/// Destructor
	~CImpulseEncoder();

	/// Return the number of bits that the specified level can manage (biggest action it can manage) (no bound check)
	static uint						maxBitSize( uint level ) { return MaxImpulseBitSizes[level]; }

	/// Return the number of bits that can be sent at most using all levels
	static uint						maxBitSizeTotal() { return MaxImpulseBitSizes[0]+MaxImpulseBitSizes[1]+MaxImpulseBitSizes[2] - - (IMPULSE_ACTION_HEADER_BITSIZE*3/8); }

	///
	void							setClientHost(CClientHost *clientHost) { _ClientHost = clientHost; }

	/// Adds an action at a given level of priority.
	void							add(CLFECOMMON::CActionImpulsion *action, uint level);

	/// Packs/sends impulse on the given outbox, using the previously added actions
	uint32							send(uint32 packet, NLMISC::CBitMemStream &outbox, uint32 &sentActions);

	/// Acknowledges a message and flushes involved queues (only positive acks).
	void							ack(uint32 packet);

	/// Reinits the whole queues
	void							reset();

	/// Unmarks all queues (act as if the awaiting actions were cleared up and readded at once)
	void							unmarkAll();

	/// Gets the total number of messages still to be sent
	uint							queueSize() const;

	/// Gets the number of messages still to be sent at a given level
	uint							queueSize(uint level) const;

	/// Sets a callback for a specific type of action (-1 for all types of action)
	static void						setReceivedCallback(TImpulseReceivedCallback cb, sint actionCode = -1);

	/// Gets the maximum bit size that can be sent through a channel (specific for a given level)
	//uint							getMaxBitSize(uint level) const { return _MaxBitSizes[level]; }

	/// Returns true if the encoder has actions referencing a given entity (by its client entity id)
	bool							hasEntityReferences(CLFECOMMON::TCLEntityId id) const
	{
		return (_QueuedImpulses[id] != 0);
	}

	/// Removes all actions referencing a given entity (if there are some)
	void							removeEntityReferences(CLFECOMMON::TCLEntityId id);

	/// \name Stats part
	// @{

	/// Get Enqueued actions size in bits for a specified level
	uint							getEnqueuedSize(uint level) const;

	/// Dump stats to XML stream
	void							dump(NLMISC::IStream& s);

	/// Count Added action
	void							countAddedAction(CLFECOMMON::CActionImpulsion *action, uint level);

	/// Effective Add Rate
	uint							effectiveAddRate(uint level);

	/// Effective Send Rate (cumulated send of each channel)
	uint							effectiveSendRate(uint level);

	/// Efficiency Ratio
	float							efficiencyRatio(uint level);

	/// Get least efficiency
	float							leastEfficiencyRatio();

	/// Get biggest queue size
	uint							biggestQueueSize();

	// @}
};


#endif // NL_IMPULSE_ENCODER_H

/* End of impulse_encoder.h */
