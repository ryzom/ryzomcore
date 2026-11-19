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

// (this file and stream_queues.cpp are a library of their own, which the tests link too: no stdpch.h)
#include "stream_scheduler.h"

#include "nel/misc/debug.h"

#include <algorithm>

CStreamSchedulerConfig::CStreamSchedulerConfig()
    : TotalOutgoingBandwidth(5000000)
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

uint32 streamWindowRoom(const CStreamSchedulerConfig &config, const CStreamWindowState &state)
{
	const sint64 packet = (sint64)config.DatagramPacketBytes;
	const sint64 rttMs = (state.RttMicroseconds + 999) / 1000;
	const sint64 tickMs = std::max((sint64)1, (sint64)(config.TickSeconds * 1000.0));
	const sint64 datagramsInFlight = ((rttMs + tickMs - 1) / tickMs + 1) * packet;
	sint64 inFlight = datagramsInFlight + state.Outstanding + state.Outstanding / 25;
	if (state.BytesInFlight >= 0)
		inFlight = std::max(inFlight, state.BytesInFlight + state.GrantedSinceSnapshot + state.GrantedSinceSnapshot / 25);
	const sint64 room = state.CongestionWindow - inFlight - packet;
	return (uint32)std::min(std::max(room, (sint64)0), (sint64)config.StreamMaxBytesPerTick);
}

CStreamScheduler::CStreamScheduler()
    : m_Tokens(0)
    , m_Started(false)
    , m_BudgetShare(1.0)
    , m_SharedLateness(false)
    , m_SharedCyclesSinceCut(1000)
    , m_SharedCleanCycles(0)
    , m_BudgetCuts(0)
    , m_EgressRate(0)
    , m_DatagramRate(0)
    , m_MinimumCursor(0)
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

void CStreamScheduler::reports(const CStreamSchedulerConfig &config, uint32 reporting, uint32 late)
{
	if (m_SharedCyclesSinceCut < 1000)
		++m_SharedCyclesSinceCut;
	m_SharedLateness = false;

	// (too few to tell: as if nothing were late, so that a budget that was taken down comes back)
	if (reporting < (uint32)SharedMinReporting)
		late = 0;

	if (reporting != 0 && late * 2 >= reporting)
	{
		m_SharedLateness = true;
		m_SharedCleanCycles = 0;
		if (m_SharedCyclesSinceCut >= (uint32)SharedCutCycles)
		{
			// Under what it sends, which can be well under what it holds itself to when the streams have little to do
			double share = m_BudgetShare;
			double floor = 0.0;
			if (config.TotalOutgoingBandwidth != 0)
			{
				share = std::min(share, m_EgressRate / (double)config.TotalOutgoingBandwidth);
				floor = m_DatagramRate / (double)config.TotalOutgoingBandwidth;
			}
			m_BudgetShare = std::min(m_BudgetShare, std::max(share * 0.85, floor));
			m_SharedCyclesSinceCut = 0;
			++m_BudgetCuts;
		}
	}
	else if (late * 10 <= reporting)
	{
		if (m_SharedCleanCycles < 1000)
			++m_SharedCleanCycles;
		if (m_SharedCleanCycles >= (uint32)SharedCleanCycles)
			m_BudgetShare = std::min(1.0, m_BudgetShare + 0.01 * config.TickSeconds);
	}
}

void CStreamScheduler::schedule(const CStreamSchedulerConfig &config, double elapsedSeconds, uint64 measuredEgress, uint64 datagramBytes, std::vector<CEntry> &entries)
{
	const uint32 minBytes = std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick);
	const uint32 maxBytes = config.StreamMaxBytesPerTick;

	// The budget. What was really sent since the last cycle is taken off, whatever it was (datagrams, stream data,
	// what was sent again, acknowledgements). The frontend owes what it sent above its budget, up to what one cycle
	// may send. It saves nothing up: what a cycle did not send cannot be sent in that cycle any more, and sent on top
	// of a later one it is more than the link takes in that one, where it waits in front of the datagrams that
	// follow. Then this cycle's share is added, by the time that passed, so that a cycle that comes late has all of
	// it.
	// (the budget is what reports() left of TotalOutgoingBandwidth)
	const double bandwidth = (double)config.TotalOutgoingBandwidth * m_BudgetShare;
	const sint64 tickBudget = (sint64)(bandwidth * config.TickSeconds);
	const double elapsed = std::min(std::max(elapsedSeconds, 0.0), 2.0 * config.TickSeconds);
	if (!m_Started)
	{
		m_Started = true;
		m_Tokens = 0;
	}
	m_Tokens = std::min(std::max(m_Tokens - (sint64)measuredEgress, -tickBudget), (sint64)0) + (sint64)(bandwidth * elapsed);
	m_DatagramRate = (double)datagramBytes / std::max(config.TickSeconds, 0.001);
	if (elapsedSeconds > 0.0)
		m_EgressRate += ((double)measuredEgress / std::max(elapsedSeconds, 0.5 * config.TickSeconds) - m_EgressRate) * 0.25;

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
	// (Unless the frontend holds itself to less than its budget because its link has no room: then the minimums come
	// out of what is left like everything else, from where they stopped the cycle before.)
	uint64 grantedMinimum = 0;
	const bool linkIsShort = m_BudgetShare < 1.0;
	size_t firstMinimum = 0;
	if (linkIsShort)
	{
		while (firstMinimum < entries.size() && entries[firstMinimum].Id < m_MinimumCursor)
			++firstMinimum;
		if (firstMinimum == entries.size())
			firstMinimum = 0;
	}
	for (size_t n = 0; n < entries.size(); ++n)
	{
		CEntry &entry = entries[(firstMinimum + n) % entries.size()];
		if (entry.Pending != 0 && entry.Window != 0)
		{
			if (linkIsShort && pool <= 0)
			{
				m_MinimumCursor = entry.Id;
				break;
			}
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
    , m_LastChangeWasCut(false)
    , m_CongestionEvents(0)
    , m_SendBusyDrops(0)
    , m_CyclesWithoutReport(0)
    , m_IdleCycles(0)
    , m_IdleCyclesToStartUp(0)
    , m_InRecovery(false)
    , m_RecoveryLateness(0)
    , m_IgnoreEvents(false)
    , m_SettleCycles(0)
    , m_ReportJudged(false)
    , m_LastReportNumber(0)
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
	// What the connection sees while its new path is validated (datagrams that cannot go yet) is not of the streams:
	// not until the first report on datagrams of after this, and not in the first seconds
	m_IgnoreEvents = true;
	m_SettleCycles = (uint32)std::max(1.0, 3.0 / std::max(config.TickSeconds, 0.001));
	newSession();
}

void CStreamAllowance::newSession()
{
	m_Started = false;
	m_LastChangeWasCut = false;
	m_CyclesWithoutReport = 0;
	m_InRecovery = false;
	m_ReportJudged = false;
	for (uint i = 0; i < (uint)NbCycles; ++i)
		m_Cycles[i].Valid = false;
}

bool CStreamAllowance::usedHalfIn(uint32 firstNumber, uint32 lastNumber) const
{
	// (cycles further back than those that are kept cannot be found)
	if ((sint32)(lastNumber - firstNumber) >= (sint32)NbCycles)
		firstNumber = lastNumber - ((uint32)NbCycles - 1);
	for (uint32 number = firstNumber; (sint32)(lastNumber - number) >= 0; ++number)
	{
		const CCycle &cycle = m_Cycles[number % NbCycles];
		if (cycle.Valid && cycle.Number == number && (uint64)cycle.Granted * 2 > m_Allowance)
			return true;
	}
	return false;
}

bool CStreamAllowance::heldIn(uint32 firstNumber, uint32 lastNumber) const
{
	// (more cycles than are kept: some of them are not known to have been held)
	if ((sint32)(lastNumber - firstNumber) >= (sint32)NbCycles)
		return false;
	for (uint32 number = firstNumber; (sint32)(lastNumber - number) >= 0; ++number)
	{
		const CCycle &cycle = m_Cycles[number % NbCycles];
		if (!cycle.Valid || cycle.Number != number || !cycle.Held)
			return false;
	}
	return true;
}

uint32 CStreamAllowance::firstCutCycle(uint32 number) const
{
	const uint32 first = number - (uint32)CutCycles;
	if (m_LastChangeWasCut && (sint32)(m_LastChangeNumber - first) > 0)
		return m_LastChangeNumber;
	return first;
}

void CStreamAllowance::cut(uint32 sendNumber)
{
	m_Allowance = m_Allowance / 2; // (held above the minimum by the caller)
	m_StartUp = false;
	m_LastChangeNumber = sendNumber;
	m_LastChangeWasCut = true;
	m_InRecovery = true;
	m_RecoveryLateness = 0xFFFFFFFF; // (the first report on datagrams of after the cut sets it)
	m_IgnoreEvents = true;
}

void CStreamAllowance::update(const CStreamSchedulerConfig &config, bool newReport, uint32 lateness, uint32 reportNumber, uint32 sendNumber, uint32 congestionEvents, uint32 sendBusyDrops, bool sharedLateness)
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

	// A report on a datagram that was not sent yet is no report: the number is the client's word
	if (newReport && (sint32)(sendNumber - reportNumber) < 0)
		newReport = false;

	// (for granted(), which counts the idle cycles)
	m_IdleCyclesToStartUp = (uint32)std::max(1.0, config.IdleSecondsToStartUp / std::max(config.TickSeconds, 0.001));

	// A client that has not reported for a while (its main thread is busy: a loading screen) says nothing about what
	// its path takes now: its next report may cut the allowance, not raise it
	const bool mayRaise = m_CyclesWithoutReport < 5;
	if (newReport)
		m_CyclesWithoutReport = 0;
	else if (m_CyclesWithoutReport < 1000)
		++m_CyclesWithoutReport;

	// (the cycles after a new path in which what the connection sees is not counted)
	if (m_SettleCycles != 0)
		--m_SettleCycles;

	// A report is judged once: a client that got no new datagram reports the same again
	if (newReport && (!m_ReportJudged || reportNumber != m_LastReportNumber))
	{
		m_ReportJudged = true;
		m_LastReportNumber = reportNumber;

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
			if (m_IgnoreEvents || m_SettleCycles != 0)
			{
				eventsIgnored = events;
				events = false;
				m_IgnoreEvents = false;
			}
		}

		// A cut only helps where it takes something away: where a cycle gave more than half the allowance, which
		// is what the allowance is after the cut. A client that used less than that is not held up by its stream
		// data, whatever makes its datagrams late or its connection lose packets, and its allowance stays.
		bool doCut = false;
		if (sharedLateness)
		{
			// What is late or lost now is of the link all the clients share, where the frontend answers for all of
			// them: nothing of it is held against this client, and nothing is made of it either
			events = false;
			eventsIgnored = true;
		}
		else if (late)
		{
			if (m_InRecovery)
			{
				// After a cut: the queue that stood then drains, and the lateness comes down with it. The reports
				// on datagrams that were all sent after the cut are compared with each other (the report that
				// caused the cut was of datagrams from before the queue was at its longest): lateness that does
				// not come down, with stream data still going out, is more than that queue.
				if (covers)
				{
					if (lateness < m_RecoveryLateness)
						m_RecoveryLateness = lateness;
					else if (usedHalfIn(m_LastChangeNumber, reportNumber - 1))
						doCut = true;
				}
			}
			else if (usedHalfIn(firstCutCycle(reportNumber), reportNumber - 1))
			{
				// Stream data went out shortly before the datagrams that are late. This does not wait for a report
				// on datagrams of after the last raise only: every cycle at an allowance that is too high makes the
				// queue longer.
				doCut = true;
			}
		}
		else if (covers)
		{
			m_InRecovery = false;
		}

		// Congestion or a datagram not sent while stream data was going out
		if (!doCut && events && usedHalfIn(firstCutCycle(sendNumber), sendNumber - 1))
			doCut = true;

		if (doCut)
		{
			cut(sendNumber);
		}
		else if (covers && !late && !events && !eventsIgnored && mayRaise && heldIn(reportNumber - reportDatagrams, reportNumber - 1))
		{
			// The datagrams of the report were all sent behind cycles in which the client was held at this allowance,
			// and none of them is late
			if (m_StartUp && lateness * 5 <= config.QueueDelayTarget)
			{
				// By a quarter while nothing shows at all. (A quarter of a small allowance is less than the step
				// of an allowance that grows by packets.)
				m_Allowance = (uint32)std::min((uint64)m_Allowance + std::max(m_Allowance / 4, config.DatagramPacketBytes), (uint64)config.StreamMaxBytesPerTick);
			}
			else
			{
				m_StartUp = false;
				m_Allowance = (uint32)std::min((uint64)m_Allowance + config.DatagramPacketBytes, (uint64)config.StreamMaxBytesPerTick);
			}
			m_LastChangeNumber = sendNumber;
			m_LastChangeWasCut = false;
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
