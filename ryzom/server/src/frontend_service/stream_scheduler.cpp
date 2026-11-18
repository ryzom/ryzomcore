// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "stream_scheduler.h"

#include <algorithm>

CStreamSchedulerConfig::CStreamSchedulerConfig()
    : TotalOutgoingBandwidth(5000000)
    , StreamBurstBytes(0)
    , StreamMinBytesPerTick(200)
    , StreamMaxBytesPerTick(65536)
    , StreamMinChunk(4096)
    , QueueDelayTarget(20)
    , TickSeconds(0.1)
    , DatagramPacketBytes(560)
    , ReportDatagrams(3)
    , IdleSecondsToStartUp(10.0)
{
}

CStreamScheduler::CStreamScheduler()
    : m_Tokens(0)
    , m_Started(false)
    , m_Round(1)
    , m_RoundDone(false)
    , m_Cursor(0)
    , m_LastPool(0)
    , m_LastPoolLeft(0)
    , m_LastGranted(0)
    , m_LastGrantedMinimum(0)
    , m_LastWaiting(0)
    , m_LastServed(0)
    , m_RoundTicks(0)
    , m_OvershootDatagrams(0)
    , m_OvershootMinimums(0)
{
}

void CStreamScheduler::schedule(const CStreamSchedulerConfig &config, double elapsedSeconds, uint64 measuredEgress, uint64 datagramBytes, std::vector<CEntry> &entries)
{
	const uint32 minBytes = std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick);
	const uint32 maxBytes = config.StreamMaxBytesPerTick;

	// The budget. What was really sent since the last cycle is taken off, whatever it was (datagrams, stream data,
	// what was sent again, acknowledgements): the frontend owes what it sent above its budget and may not save up
	// more than the burst. Then this cycle's share is added, so that a cycle that comes late does not lose it.
	const sint64 tickBudget = (sint64)((double)config.TotalOutgoingBandwidth * config.TickSeconds);
	const sint64 burst = config.StreamBurstBytes != 0 ? (sint64)config.StreamBurstBytes : tickBudget;
	const double elapsed = std::min(std::max(elapsedSeconds, 0.0), 2.0 * config.TickSeconds);
	if (!m_Started)
	{
		m_Started = true;
		m_Tokens = 0;
	}
	m_Tokens = std::min(std::max(m_Tokens - (sint64)measuredEgress, -burst), burst) + (sint64)((double)config.TotalOutgoingBandwidth * elapsed);

	// What is left for the streams once this cycle's datagrams are out
	sint64 pool = m_Tokens - (sint64)datagramBytes;
	m_LastPool = pool;
	if (pool < 0)
		++m_OvershootDatagrams;

	// A client whose stream data starts now joins the round after the one that runs (when none runs, that is this
	// cycle's: the round is counted on below)
	for (size_t i = 0; i < entries.size(); ++i)
	{
		CEntry &entry = entries[i];
		entry.Grant = 0;
		if (entry.Pending != 0 && !entry.Client->HadPending)
		{
			entry.Client->LastRound = m_Round;
			entry.Client->TurnLeft = 0;
		}
	}
	if (m_RoundDone)
	{
		++m_Round;
		m_RoundDone = false;
		m_RoundTicks = 0;
	}
	++m_RoundTicks;

	// 1. The minimum, for every client with stream data that its own path lets through. It is given even when the
	// budget has nothing left: a client is never without progress because others are served.
	uint64 grantedMinimum = 0;
	for (size_t i = 0; i < entries.size(); ++i)
	{
		CEntry &entry = entries[i];
		if (entry.Pending != 0 && entry.Window != 0)
		{
			entry.Grant = std::min(minBytes, std::min(entry.Pending, entry.Window));
			pool -= entry.Grant;
			grantedMinimum += entry.Grant;
		}
	}
	if (m_LastPool >= 0 && pool < 0)
		++m_OvershootMinimums;

	// 2. The turns of this round: the clients that wait for more and did not have their turn yet
	uint64 grantedTurns = 0;
	uint32 served = 0;
	m_Waiting.clear();
	for (size_t i = 0; i < entries.size(); ++i)
	{
		const CEntry &entry = entries[i];
		if (entry.Pending > entry.Grant && entry.Window > entry.Grant && (sint32)(m_Round - entry.Client->LastRound) > 0)
			m_Waiting.push_back((uint32)i);
	}
	const size_t nbWaiting = m_Waiting.size();
	if (nbWaiting == 0)
	{
		// Nobody waits for a turn: the round is over, and with it the turns that a client's own path had cut short
		m_RoundDone = true;
		for (size_t i = 0; i < entries.size(); ++i)
			entries[i].Client->TurnLeft = 0;
	}
	else if (pool > 0)
	{
		// What each of them can take now
		m_Wants.resize(nbWaiting);
		for (size_t k = 0; k < nbWaiting; ++k)
		{
			const CEntry &entry = entries[m_Waiting[k]];
			uint32 want = std::min(entry.Pending, std::min(entry.Window, maxBytes));
			m_Wants[k] = want > entry.Grant ? want - entry.Grant : 0;
		}

		// The size of a turn: the level up to which all of them can be filled out of the pool (those that can take
		// less leave their part to the others), but not below StreamMinChunk, and not above the most a client gets
		uint32 turn = maxBytes;
		{
			std::vector<uint32> sorted(m_Wants);
			std::sort(sorted.begin(), sorted.end());
			sint64 left = pool;
			for (size_t k = 0; k < nbWaiting; ++k)
			{
				const sint64 others = (sint64)(nbWaiting - k);
				if ((sint64)sorted[k] * others <= left)
				{
					left -= sorted[k];
				}
				else
				{
					turn = (uint32)std::min(left / others, (sint64)maxBytes);
					break;
				}
			}
		}
		turn = std::min(maxBytes, std::max(turn, config.StreamMinChunk));

		// By ascending id, from where the last cycle stopped
		size_t start = 0;
		while (start < nbWaiting && entries[m_Waiting[start]].Id < m_Cursor)
			++start;
		if (start == nbWaiting)
			start = 0;

		bool done = true;
		for (size_t n = 0; n < nbWaiting; ++n)
		{
			const size_t k = (start + n) % nbWaiting;
			CEntry &entry = entries[m_Waiting[k]];
			CStreamSchedulerClient *client = entry.Client;
			const uint32 want = m_Wants[k];

			if (client->TurnLeft == 0)
				client->TurnLeft = turn;
			const uint32 rest = entry.Pending - entry.Grant;
			const uint32 given = (uint32)std::min((sint64)std::min(client->TurnLeft, want), pool);
			entry.Grant += given;
			pool -= given;
			client->TurnLeft -= given;
			grantedTurns += given;
			if (given != 0)
				++served;

			if (client->TurnLeft == 0 || given == rest)
			{
				// Its turn is over, or it has nothing left to send
				client->LastRound = m_Round;
				client->TurnLeft = 0;
			}
			else if (given < want)
			{
				// The pool ran out inside its turn: it goes on from here in the next cycle
				done = false;
				m_Cursor = entry.Id;
				break;
			}
			// Else its own path took no more: its turn stays open for the cycles to come

			if (pool <= 0)
			{
				m_Cursor = entry.Id + 1;
				if (n + 1 < nbWaiting)
					done = false;
				break;
			}
		}

		if (done)
		{
			// Everybody that waited was served in one cycle without the pool running out: the round is over
			m_RoundDone = true;
			for (size_t i = 0; i < entries.size(); ++i)
				entries[i].Client->TurnLeft = 0;
			for (size_t k = 0; k < nbWaiting; ++k)
				entries[m_Waiting[k]].Client->LastRound = m_Round;
		}
	}

	for (size_t i = 0; i < entries.size(); ++i)
		entries[i].Client->HadPending = entries[i].Pending > entries[i].Grant;

	m_LastPoolLeft = pool;
	m_LastGrantedMinimum = grantedMinimum;
	m_LastGranted = grantedMinimum + grantedTurns;
	m_LastWaiting = (uint32)nbWaiting;
	m_LastServed = served;
}

CStreamAllowance::CStreamAllowance()
    : m_CurrentNumber(0)
    , m_Allowance(0)
    , m_StartUp(true)
    , m_Started(false)
    , m_LastChangeNumber(0)
    , m_CongestionEvents(0)
    , m_SendBusyDrops(0)
    , m_CyclesWithoutReport(0)
    , m_IdleCycles(0)
    , m_IdleCyclesToStartUp(0)
    , m_InRecovery(false)
    , m_RecoveryLateness(0)
    , m_IgnoreEvents(false)
{
	for (uint i = 0; i < (uint)NbCycles; ++i)
		m_Cycles[i].Valid = false;
}

void CStreamAllowance::reset(const CStreamSchedulerConfig &config)
{
	m_Allowance = config.StreamMinChunk;
	m_StartUp = true;
	m_InRecovery = false;
	m_IdleCycles = 0;
	// What the connection sees while its new path is validated (datagrams that cannot go yet) is not of the streams
	m_IgnoreEvents = true;
	newSession();
}

void CStreamAllowance::newSession()
{
	m_Started = false;
	m_CyclesWithoutReport = 0;
	for (uint i = 0; i < (uint)NbCycles; ++i)
		m_Cycles[i].Valid = false;
}

uint64 CStreamAllowance::grantedIn(uint32 firstNumber, uint32 lastNumber) const
{
	uint64 granted = 0;
	for (uint32 number = firstNumber; (sint32)(lastNumber - number) >= 0; ++number)
	{
		const CCycle &cycle = m_Cycles[number % NbCycles];
		if (cycle.Valid && cycle.Number == number)
			granted += cycle.Granted;
	}
	return granted;
}

bool CStreamAllowance::heldIn(uint32 firstNumber, uint32 lastNumber) const
{
	for (uint32 number = firstNumber; (sint32)(lastNumber - number) >= 0; ++number)
	{
		const CCycle &cycle = m_Cycles[number % NbCycles];
		if (!cycle.Valid || cycle.Number != number || !cycle.Held)
			return false;
	}
	return true;
}

void CStreamAllowance::cut(uint32 sendNumber, bool late, uint32 lateness)
{
	m_Allowance = m_Allowance / 2; // (held above the minimum by the caller)
	m_StartUp = false;
	m_LastChangeNumber = sendNumber;
	m_InRecovery = true;
	m_RecoveryLateness = late ? lateness : 0xFFFFFFFF;
	m_IgnoreEvents = true;
}

void CStreamAllowance::update(const CStreamSchedulerConfig &config, bool newReport, uint32 lateness, uint32 reportNumber, uint32 sendNumber, uint32 congestionEvents, uint32 sendBusyDrops)
{
	if (!m_Started)
	{
		m_Started = true;
		if (m_Allowance == 0)
			m_Allowance = config.StreamMinChunk;
		m_LastChangeNumber = sendNumber;
		m_CongestionEvents = congestionEvents;
		m_SendBusyDrops = sendBusyDrops;
	}
	m_CurrentNumber = sendNumber;

	// (for granted(), which counts the idle cycles)
	m_IdleCyclesToStartUp = (uint32)std::max(1.0, config.IdleSecondsToStartUp / std::max(config.TickSeconds, 0.001));

	// A client that has not reported for a while (its main thread is busy: a loading screen) says nothing about what
	// its path takes now: its next report may cut the allowance, not raise it
	const bool mayRaise = m_CyclesWithoutReport < 5;
	if (newReport)
		m_CyclesWithoutReport = 0;
	else if (m_CyclesWithoutReport < 1000)
		++m_CyclesWithoutReport;

	if (newReport)
	{
		const uint32 reportDatagrams = std::max(config.ReportDatagrams, (uint32)1);
		const sint32 sinceChange = (sint32)(reportNumber - m_LastChangeNumber);
		// The report covers only datagrams that were sent after the last change
		const bool covers = sinceChange >= (sint32)reportDatagrams;
		const bool late = lateness > config.QueueDelayTarget;

		// Congestion and datagrams not sent, since the last report that covered: counted once, with such a report
		bool events = false;
		bool eventsIgnored = false;
		if (covers)
		{
			events = (congestionEvents != m_CongestionEvents) || (sendBusyDrops != m_SendBusyDrops);
			m_CongestionEvents = congestionEvents;
			m_SendBusyDrops = sendBusyDrops;
			if (m_IgnoreEvents)
			{
				eventsIgnored = events;
				events = false;
				m_IgnoreEvents = false;
			}
		}

		bool doCut = false;
		if (late)
		{
			if (m_InRecovery)
			{
				// After a cut: the queue that stood then drains, and the lateness comes down with it. Lateness that
				// does not come down, on datagrams that were all sent after the cut, is more than that queue.
				if (covers)
				{
					if (lateness < m_RecoveryLateness)
						m_RecoveryLateness = lateness;
					else
						doCut = true;
				}
			}
			else if (sinceChange >= 1 && grantedIn(reportNumber - CutCycles, reportNumber - 1) >= config.DatagramPacketBytes)
			{
				// Stream data went out shortly before the datagrams that are late, a packet of it or more (less
				// cannot hold a datagram up). After a raise this does not wait for a report on datagrams of after
				// it only: the last of them is, and every cycle at the raised allowance makes the queue longer.
				doCut = true;
			}
		}
		else if (covers)
		{
			m_InRecovery = false;
		}

		// Congestion or a datagram not sent while stream data was going out
		if (!doCut && events && grantedIn(sendNumber - CutCycles, sendNumber - 1) >= config.DatagramPacketBytes)
			doCut = true;

		if (doCut)
		{
			cut(sendNumber, late, lateness);
		}
		else if (covers && !late && !events && !eventsIgnored && mayRaise && heldIn(reportNumber - reportDatagrams, reportNumber - 1))
		{
			// The datagrams of the report were all sent behind cycles in which the client was held at this allowance,
			// and none of them is late
			if (m_StartUp && lateness * 2 < config.QueueDelayTarget)
			{
				// (a quarter of a small allowance is less than the step of an allowance that grows by packets)
				m_Allowance = (uint32)std::min((uint64)m_Allowance + std::max(m_Allowance / 4, config.DatagramPacketBytes), (uint64)config.StreamMaxBytesPerTick);
			}
			else
			{
				m_StartUp = false;
				m_Allowance = (uint32)std::min((uint64)m_Allowance + config.DatagramPacketBytes, (uint64)config.StreamMaxBytesPerTick);
			}
			m_LastChangeNumber = sendNumber;
		}
	}

	// Between the minimum every client gets and the most one gets, and not 0 (unless the most is 0, which stops the
	// streams)
	const uint32 floor = std::max(std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick), (uint32)1);
	m_Allowance = std::min(std::max(m_Allowance, floor), config.StreamMaxBytesPerTick);
}

void CStreamAllowance::granted(uint32 bytes, bool wantedMore)
{
	// After long enough without stream data, an allowance that was cut grows by quarters again, from where it stands
	if (bytes == 0 && !wantedMore)
	{
		if (m_IdleCycles < m_IdleCyclesToStartUp && ++m_IdleCycles == m_IdleCyclesToStartUp)
		{
			m_StartUp = true;
			m_InRecovery = false;
		}
	}
	else
	{
		m_IdleCycles = 0;
	}

	// What this cycle gave, behind its datagram
	CCycle &cycle = m_Cycles[m_CurrentNumber % NbCycles];
	cycle.Number = m_CurrentNumber;
	cycle.Granted = bytes;
	cycle.Held = wantedMore && (uint64)bytes * 10 >= (uint64)m_Allowance * 9;
	cycle.Valid = true;
}

/* end of file */
