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

/// The settings of both (the first six are the variables of the frontend service of the same names)
struct CStreamSchedulerConfig
{
	CStreamSchedulerConfig();

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
	/// Seconds between two cycles
	double TickSeconds;
	/// Bytes of a QUIC packet with a full datagram, by which an allowance grows once it has found its level
	uint32 DatagramPacketBytes;
	/// How many datagrams the lateness that a client reports covers (it reports the smallest of its last ones): a
	/// report says what a change of the allowance did once it covers only datagrams that were sent after the change
	uint32 ReportDatagrams;
	/// Seconds without stream data for a client after which its allowance looks for its level again, from where it is
	double IdleSecondsToStartUp;
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
 * It starts at StreamMinChunk. The client reports, in each of its datagrams, how late the frontend's datagrams reach
 * it: the smallest lateness of the last ReportDatagrams of them, with the number of the last one. The allowance keeps
 * what it gave in each of the last cycles, by the number of that cycle's datagram, and judges a report by the cycles
 * that could have held up the datagrams it covers.
 *
 * It is raised when a report shows no lateness for datagrams that were all sent while the client was held at the
 * allowance (it got all of it and had more to send), after the last change: by a quarter at first (but not by less
 * than a packet), and only while the report shows next to no lateness at all; by a packet once it was cut. One cycle
 * at the allowance, or datagrams sent before it, are no evidence that the path takes more.
 *
 * It is halved when a report shows lateness, or the connection saw congestion or could not send a datagram, and the
 * client used its allowance shortly before: one of the last cycles gave it more than half of it, which is as much as
 * a cut takes away. A client that uses less is not held up by its stream data, and a cut would change nothing for
 * it: its allowance stays, whatever happens to its datagrams. After a cut, the queue that stood at the cut takes time
 * to drain: the reports on datagrams of after the cut are compared with each other, lateness that comes down is that
 * queue and does not cut again, lateness that does not come down does; and the congestion seen up to the first of
 * those reports is of what was sent before the cut.
 *
 * What this cannot tell apart: datagrams that are late for another reason, while the client does use its allowance.
 * The allowance is then cut as if the streams were the cause, down to the minimum if it lasts.
 *
 * After IdleSecondsToStartUp without stream data, an allowance that was cut grows by quarters again, from where it
 * stands, until the next cut.
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
	/// sendNumber: the number of the frontend's datagram of this cycle.
	/// congestionEvents, sendBusyDrops: counters of the connection since it opened.
	void update(const CStreamSchedulerConfig &config, bool newReport, uint32 lateness, uint32 reportNumber, uint32 sendNumber, uint32 congestionEvents, uint32 sendBusyDrops);

	/// Once per cycle, after the scheduler: what the client got, and whether more waited for it than that
	void granted(uint32 bytes, bool wantedMore);

	/// A new session starts on the connection: its datagrams may be numbered from another point, and what was kept
	/// by number is of no use. The caller drops a report of the session before as well.
	void newSession();

	/// Bytes per cycle
	uint32 allowance() const { return m_Allowance; }
	/// Growing by quarters
	bool startUp() const { return m_StartUp; }

private:
	enum
	{
		/// Cycles kept, which is how old a report can be and still be judged
		NbCycles = 32,
		/// Cycles before a datagram in which stream data can have held it up
		CutCycles = 10
	};

	struct CCycle
	{
		uint32 Number; // of the datagram of the cycle
		uint32 Granted; // bytes of stream data given after that datagram
		bool Held; // the client got all of its allowance and had more to send
		bool Valid;
	};

	/// One of the cycles of these numbers gave the client more than half of the allowance as it is now
	bool usedHalfIn(uint32 firstNumber, uint32 lastNumber) const;
	/// The client was held at the allowance in every cycle of these numbers
	bool heldIn(uint32 firstNumber, uint32 lastNumber) const;
	void cut(uint32 sendNumber);

	CCycle m_Cycles[NbCycles];
	uint32 m_CurrentNumber;

	uint32 m_Allowance;
	bool m_StartUp;
	bool m_Started;
	/// The number of the datagram of the cycle of the last change: the datagrams after it show what the change did
	uint32 m_LastChangeNumber;
	/// The counters of the connection as of the last report that was judged for them
	uint32 m_CongestionEvents;
	uint32 m_SendBusyDrops;
	uint32 m_CyclesWithoutReport;
	/// Cycles in a row without stream data for the client, and how many of them make it grow by quarters again
	uint32 m_IdleCycles;
	uint32 m_IdleCyclesToStartUp;
	/// The allowance was cut and the lateness has not been back under the target since: the smallest lateness
	/// reported on datagrams of after the cut, which a report must stay under for the queue to count as draining
	bool m_InRecovery;
	uint32 m_RecoveryLateness;
	/// Congestion and datagrams not sent are not counted until the first report on datagrams of after the last cut
	/// or reset: they are of what was sent before the cut, or of the validation of the new path
	bool m_IgnoreEvents;
	/// Cycles left after a reset in which they are not counted either (a validation can take longer than a report)
	uint32 m_SettleCycles;
	/// The number of the last report that was judged
	bool m_ReportJudged;
	uint32 m_LastReportNumber;
};

#endif // NL_STREAM_SCHEDULER_H

/* end of file */
