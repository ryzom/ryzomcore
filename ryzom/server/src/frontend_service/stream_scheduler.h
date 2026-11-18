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

#ifndef NL_STREAM_SCHEDULER_H
#define NL_STREAM_SCHEDULER_H

#include "nel/misc/types_nl.h"

#include <vector>

/*
 * How much each client gets to send on its QUIC streams in a cycle (protocol level ryzomcore/4.2).
 *
 * Two things limit it, and each has its class here:
 * - what the frontend as a whole may send (CStreamScheduler): a budget in bytes per second for everything that leaves
 *   it, datagrams first, of which the streams of all clients share what is left;
 * - what the path to one client takes without holding up its datagrams (CStreamAllowance): an allowance that follows
 *   how late the client says its datagrams arrive.
 *
 * Neither knows of MsQuic or of the service: the frontend gives them numbers once per cycle (CFeSendSub::grantStreams).
 * Both are used by the main thread of the service only.
 */

/// The settings of both (the variables of the frontend service of the same names)
struct CStreamSchedulerConfig
{
	CStreamSchedulerConfig();

	/// Seconds between two cycles
	double TickSeconds;
	/// Bytes per second the frontend may send in all: datagrams, streams, and what QUIC adds
	uint32 TotalOutgoingBandwidth;
	/// How far the frontend may run ahead of or behind that, in bytes (0: what one cycle may send)
	uint32 StreamBurstBytes;
	/// Bytes per cycle every client with stream data gets, whatever the budget
	uint32 StreamMinBytesPerTick;
	/// Most bytes per cycle one client gets
	uint32 StreamMaxBytesPerTick;
	/// Smallest turn of a client when the budget is short, and the allowance a connection starts with
	uint32 StreamMinChunk;
	/// Lateness of the datagrams, in milliseconds, above which the allowance of a client is cut
	uint32 QueueDelayTarget;
	/// Bytes of a QUIC packet with a full datagram, by which an allowance grows once it has found its level
	uint32 DatagramPacketBytes;
	/// How many datagrams the lateness that a client reports covers (it reports the smallest of its last ones): a
	/// report says what a change of the allowance did once it covers only datagrams that were sent after the change
	uint32 ReportDatagrams;
};

/**
 * What the scheduler remembers of one client between cycles. It belongs to the connection.
 */
struct CStreamSchedulerClient
{
	CStreamSchedulerClient()
	    : LastRound(0)
	    , TurnLeft(0)
	    , HadPending(false)
	{
	}

	/// The round in which the client had its turn last (or joined)
	uint32 LastRound;
	/// What is left of a turn that the budget or the client's own allowance cut short
	uint32 TurnLeft;
	/// Stream data waited for it after the last cycle
	bool HadPending;
};

/**
 * Shares what the frontend may send among the streams of its clients.
 *
 * Every client with stream data gets a small minimum each cycle, whatever the budget. What the budget leaves beyond
 * the datagrams and the minimums goes out in turns: in a round every client that waits gets one turn, of a size that
 * lets the budget of the cycle go round (but not below StreamMinChunk, so that a long message is done in a few turns),
 * and then waits for the next round at the minimum. A client that gets stream data while a round runs joins the next
 * one. A turn that the budget cuts short goes on in the next cycle; a turn that the client's own allowance cuts short
 * stays open for the cycles to come.
 */
class CStreamScheduler
{
public:
	/// One client in one cycle
	struct CEntry
	{
		CEntry()
		    : Client(nullptr)
		    , Id(0)
		    , Pending(0)
		    , Window(0)
		    , Grant(0)
		{
		}

		/// What the scheduler remembers of it
		CStreamSchedulerClient *Client;
		/// The turns go by ascending id, from where the last cycle stopped
		uint32 Id;
		/// Bytes of stream data that wait for it
		uint32 Pending;
		/// Most bytes its own path takes this cycle (0 for a client that cannot be sent anything now)
		uint32 Window;
		/// Out: the bytes it gets
		uint32 Grant;
	};

	CStreamScheduler();

	/// One cycle. elapsedSeconds: since the cycle before. measuredEgress: bytes the frontend sent since then, all of
	/// them. datagramBytes: bytes of the datagrams of this cycle, with what QUIC and the network add to them.
	/// entries: every client that could get stream data, by ascending id; their Grant is set.
	void schedule(const CStreamSchedulerConfig &config, double elapsedSeconds, uint64 measuredEgress, uint64 datagramBytes, std::vector<CEntry> &entries);

	/// \name What the last cycle did, for statistics
	// @{
	/// Bytes the last cycle could send in all, datagrams and streams (what was sent before it is taken off, what it
	/// sent itself is not yet)
	sint64 tokens() const { return m_Tokens; }
	/// What the budget left for the streams in the last cycle, before the minimums (can be below 0)
	sint64 lastPool() const { return m_LastPool; }
	/// What it left unused
	sint64 lastPoolLeft() const { return m_LastPoolLeft; }
	/// Bytes granted in the last cycle, of which as minimums
	uint64 lastGranted() const { return m_LastGranted; }
	uint64 lastGrantedMinimum() const { return m_LastGrantedMinimum; }
	/// Clients that waited for a turn in the last cycle, and of them those that got bytes of one
	uint32 lastWaiting() const { return m_LastWaiting; }
	uint32 lastServed() const { return m_LastServed; }
	/// The round that runs, and how many cycles it has run
	uint32 round() const { return m_Round; }
	uint32 roundTicks() const { return m_RoundTicks; }
	/// Cycles in which the datagrams alone were above the budget, and in which the minimums pushed it above
	uint64 overshootDatagrams() const { return m_OvershootDatagrams; }
	uint64 overshootMinimums() const { return m_OvershootMinimums; }
	// @}

private:
	sint64 m_Tokens;
	bool m_Started;
	uint32 m_Round;
	bool m_RoundDone;
	uint32 m_Cursor;

	sint64 m_LastPool;
	sint64 m_LastPoolLeft;
	uint64 m_LastGranted;
	uint64 m_LastGrantedMinimum;
	uint32 m_LastWaiting;
	uint32 m_LastServed;
	uint32 m_RoundTicks;
	uint64 m_OvershootDatagrams;
	uint64 m_OvershootMinimums;

	// (kept between cycles for its memory)
	std::vector<uint32> m_Wants;
	std::vector<uint32> m_Waiting;
};

/**
 * What the path to one client takes of stream data per cycle without holding up its datagrams.
 *
 * It starts at StreamMinChunk and grows while the client reports that its datagrams arrive on time and the allowance
 * is what holds the client back: by a quarter at a time at first, then by one packet at a time. It is halved when the
 * datagrams arrive late, when the connection sees congestion, or when a datagram could not be sent because the one
 * before had not left, provided stream data went to the client since the last change (else the streams are not the
 * cause). It changes at most once per report that covers only datagrams sent after the change before, so that each
 * change is judged by its own effect.
 *
 * Cycles in which the client had nothing more to send than it got say nothing about its path: they count neither for
 * nor against growth, so a client that was idle grows as one that starts.
 */
class CStreamAllowance
{
public:
	CStreamAllowance();

	/// Start over, for a new connection or a new path
	void reset(const CStreamSchedulerConfig &config);

	/// Once per cycle, before the scheduler.
	/// newReport: the client reported its lateness since the last call; lateness: the report, in milliseconds;
	/// reportNumber: the number of the frontend's datagram the client had received last when it reported.
	/// sendNumber: the number of the frontend's last datagram to the client.
	/// congestionEvents, sendBusyDrops: counters of the connection since it opened.
	void update(const CStreamSchedulerConfig &config, bool newReport, uint32 lateness, uint32 reportNumber, uint32 sendNumber, uint32 congestionEvents, uint32 sendBusyDrops);

	/// Once per cycle, after the scheduler: what the client got, and whether more waited for it than that
	void granted(uint32 bytes, bool wantedMore);

	/// A new session starts on the connection: its datagrams may be numbered from another point
	void newSession() { m_Started = false; }

	/// Bytes per cycle
	uint32 allowance() const { return m_Allowance; }
	/// Still looking for its level
	bool startUp() const { return m_StartUp; }

private:
	uint32 m_Allowance;
	bool m_StartUp;
	bool m_Started;
	uint32 m_LastChangeNumber;
	uint32 m_CongestionEvents;
	uint32 m_SendBusyDrops;
	/// Bytes the client got since the last change
	uint64 m_GrantedSinceChange;
	/// Since the last report that was looked at, in the cycles in which the client wanted more than it got: what it
	/// got, and what its allowance let it have
	uint64 m_GateGranted;
	uint64 m_GateAllowed;
	uint32 m_CyclesWithoutReport;
	/// The allowance was just cut: congestion seen up to the next report belongs to what was sent before the cut
	bool m_Recovering;
	/// The last change was a cut (after a raise, a report of lateness is acted on at once)
	bool m_LastChangeWasCut;
};

#endif // NL_STREAM_SCHEDULER_H

/* end of file */
