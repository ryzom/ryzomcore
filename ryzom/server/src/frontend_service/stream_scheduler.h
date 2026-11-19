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

/// What the transport says of one connection, for streamWindowRoom
struct CStreamWindowState
{
	/// Its congestion window, in bytes
	sint64 CongestionWindow;
	/// Its round trip time, in microseconds
	sint64 RttMicroseconds;
	/// Bytes it had sent and not got acknowledged when it was last asked, -1 if it does not tell
	sint64 BytesInFlight;
	/// Bytes of stream data given to it that it has not completed (acknowledged) yet
	sint64 Outstanding;
	/// Of those, the bytes given since it was last asked
	sint64 GrantedSinceSnapshot;
};

/**
 * Bytes the streams of a connection may take this cycle as far as its congestion window goes, which is what the window
 * leaves beside its datagrams: the next datagram must never wait for the window.
 * The window is taken by the datagrams in flight (one per cycle within the round trip time) and by what was given to
 * the streams and is not acknowledged yet, with the headers of its packets (about 4%). That is counted from the
 * frontend's own numbers. If the transport said that more was in flight when it was last asked (datagrams whose
 * acknowledgement is late, data sent again after a loss), its number counts, with what was given to the streams since.
 */
uint32 streamWindowRoom(const CStreamSchedulerConfig &config, const CStreamWindowState &state);

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

	/// Once per cycle, before schedule(). Of the clients whose report on the lateness of their datagrams arrived since
	/// the last cycle: how many there are, and how many of them say their datagrams are late (above QueueDelayTarget).
	///
	/// The datagrams of half the clients late at once is not the doing of the path of each of them: it is what they
	/// share, the link of the frontend (or something behind it, in the data centre), which carries less than
	/// TotalOutgoingBandwidth says. Nothing else tells the frontend, and the allowances cannot help: what one client
	/// gives up goes to the next. So the frontend then holds itself to less than its budget: to 85% of what it sends
	/// (not to less than its datagrams, which go whatever the budget), again after as many cycles as a report takes
	/// to show it, and after a second of cycles without it, it goes back up by 1% of the budget per second.
	/// The caller counts each client once, by its last report of the last cycles (CStreamAllowance::reportsLately,
	/// isLate): taking the budget down, or holding it down, takes half of the clients that report. While it holds itself to less, the minimum of StreamMinBytesPerTick is given out of the budget like the
	/// rest, to as many clients as it reaches, in turns: on a link that has no room, every byte of stream data is in
	/// front of somebody's datagram.
	void reports(const CStreamSchedulerConfig &config, uint32 reporting, uint32 late);
	/// The last reports() found the datagrams of half the clients late: a client is not held to account for it
	bool sharedLateness() const { return m_SharedLateness; }
	/// Bytes per second the frontend holds itself to now (TotalOutgoingBandwidth or less)
	uint32 budget(const CStreamSchedulerConfig &config) const { return (uint32)((double)config.TotalOutgoingBandwidth * m_BudgetShare); }
	/// The part of TotalOutgoingBandwidth that is (1: all of it)
	double budgetShare() const { return m_BudgetShare; }
	/// How often it took its budget down
	uint64 budgetCuts() const { return m_BudgetCuts; }

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
	/// Cycles in which the datagrams alone were above what a cycle may send, and in which the minimums pushed it above
	uint64 overshootDatagrams() const { return m_OvershootDatagrams; }
	uint64 overshootMinimums() const { return m_OvershootMinimums; }
	// @}

private:
	enum
	{
		/// Fewer clients than this that report do not tell a shared link from paths of their own
		SharedMinReporting = 8,
		/// Cycles between two cuts of the budget: what a report takes to show the cut before
		SharedCutCycles = 6,
		/// Cycles without lateness before the budget goes up again
		SharedCleanCycles = 10
	};

	sint64 m_Tokens;
	bool m_Started;
	/// The part of TotalOutgoingBandwidth the frontend holds itself to
	double m_BudgetShare;
	bool m_SharedLateness;
	uint32 m_SharedCyclesSinceCut;
	uint32 m_SharedCleanCycles;
	uint64 m_BudgetCuts;
	/// Bytes per second it sent lately, as measured, and of its datagrams in the last cycle
	double m_EgressRate;
	double m_DatagramRate;
	/// Where the minimums start in a cycle that has not enough for all (an id)
	uint32 m_MinimumCursor;
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
 * than a packet), and only while the report shows next to no lateness at all; by a packet once it was cut (by an
 * eighth where a packet is more than that, on a slow path). One cycle
 * at the allowance, or datagrams sent before it, are no evidence that the path takes more.
 *
 * It is halved when a report shows lateness, or the connection could not send a datagram, and the client used its
 * allowance shortly before: one of the last cycles gave it more than half of it, which is as much as
 * a cut takes away. A client that got less than that because its congestion window had no room for more has an
 * allowance far above what its path takes: that allowance goes to a quarter. If it got nothing at all lately with
 * stream data waiting and no room in its window, the window is full of what it got before, and the cut is for that.
 * A client that got little because the budget of the frontend is short, or that had no more to send than the little
 * it got, is not held up by its stream data, and a cut would change nothing for it: its allowance
 * stays, whatever happens to its datagrams. After a cut, the queue that stood at the cut takes time
 * to drain: the reports on datagrams of after the cut are compared with each other, lateness that comes down is that
 * queue and does not cut again, lateness that does not come down does; and the datagrams not sent up to the first of
 * those reports are of what was sent before the cut.
 *
 * Congestion that the connection saw (a packet lost) does not cut it: the congestion window answers for that, which
 * takes the room of the stream data with it, and a loss is not always congestion. A report that comes with it is no
 * reason for a raise.
 *
 * An allowance says how much stream data a cycle may give at once. Where the congestion window of the connection is
 * what holds a client back, cycle after cycle, the allowance is no longer that: the window then lets a block through
 * every few cycles, as large as the window, and the datagrams behind it are late while those between are on time,
 * which the client's smallest of three does not show. So an allowance of more than one and a half times what got
 * through per cycle, over ten cycles of which the window limited eight, comes down to one and a quarter times that
 * (to a quarter of itself at most at once, and not again before ten cycles have passed since).
 *
 * Lateness is counted from what the client reports when it gets no stream data: the jitter of its path and a clock
 * that gains on the frontend's read as lateness all the time, and are not of the streams. The streams may add
 * QueueDelayTarget to that.
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

	/// What the client says its connection carries towards it, in bytes per second (0: it does not say, and the
	/// frontend finds out). Told when the client logs in. The allowance then starts at half of that (not above
	/// four times StreamMinChunk) and stays under nine tenths of it: it does not have to pass what the path takes to
	/// find it. It is the client's word about its own path: lateness and the rest still cut an allowance that is
	/// under it. A client that says more than it has starts higher than its path takes, which is its own datagrams
	/// late until the allowance has come down.
	void declare(uint32 bytesPerSecond);
	uint32 declared() const { return m_Declared; }

	/// Once per cycle, before the scheduler.
	/// newReport: the client reported its lateness since the last call; lateness: the report, in milliseconds;
	/// reportNumber: the number of the frontend's datagram the client had received last when it reported.
	/// sendNumber: the number of the frontend's datagram of this cycle.
	/// congestionEvents, sendBusyDrops: counters of the connection since it opened.
	/// sharedLateness: the datagrams of half the clients are late (CStreamScheduler::sharedLateness), which is not
	/// this client's doing: its allowance is not cut for what it reports or loses meanwhile, and not raised.
	void update(const CStreamSchedulerConfig &config, bool newReport, uint32 lateness, uint32 reportNumber, uint32 sendNumber, uint32 congestionEvents, uint32 sendBusyDrops, bool sharedLateness = false);

	/// Once per cycle, after the scheduler: what the client got, and whether more waited for it than that.
	/// windowLimited: what it got is all that the congestion window of its connection had room for, and less than
	/// its allowance. (Not: it got less because the budget of the frontend was short.)
	void granted(uint32 bytes, bool wantedMore, bool windowLimited = false);

	/// A new session starts on the connection: its datagrams may be numbered from another point, and what was kept
	/// by number is of no use. The caller drops a report of the session before as well.
	void newSession();

	/// Bytes per cycle
	uint32 allowance() const { return m_Allowance; }
	/// The client reported the lateness of its datagrams in one of the last cycles (as of the last update())
	bool reportsLately() const { return m_Started && m_CyclesWithoutReport < (uint32)ReportCycles; }
	/// A report of this lateness is late for this client: more than QueueDelayTarget above the noise of its path
	bool isLate(const CStreamSchedulerConfig &config, uint32 lateness) const
	{
		const uint32 floor = noise();
		return lateness > floor && lateness - floor > config.QueueDelayTarget;
	}
	/// The lateness, in milliseconds, that this client reports when it gets no stream data: what its path and its
	/// clock do by themselves (jitter, a clock that runs fast). Lateness is counted from there.
	uint32 noise() const { return std::max(m_NoiseNow, m_NoiseBefore); }
	/// Growing by quarters
	bool startUp() const { return m_StartUp; }

private:
	enum
	{
		/// Cycles kept, which is how old a report can be and still be judged
		NbCycles = 32,
		/// Cycles before a datagram in which stream data can have held it up
		CutCycles = 10,
		/// Cycles before a datagram without any stream data, given or waiting, for its lateness to be of the path alone
		QuietCycles = 20,
		/// Cycles over which the allowance is compared with what the congestion window let through
		FollowCycles = 10,
		/// Reports without stream data before them that make a stretch of the noise, the cycles after which a
		/// stretch ends in any case, and the most that counts as noise
		NoiseReports = 150,
		NoiseCycles = 600,
		NoiseMax = 100,
		/// Cycles without a report after which a client does not count as one that reports
		ReportCycles = 5
	};

	struct CCycle
	{
		uint32 Number; // of the datagram of the cycle
		uint32 Granted; // bytes of stream data given after that datagram
		bool Held; // the client got all of its allowance and had more to send
		bool Backlog; // it had stream data to send, whether it got any or not
		bool WindowLimited; // and what it got was all that the congestion window of its connection had room for
		bool Valid;
	};

	/// In one of the cycles of these numbers the client got stream data or had some waiting
	bool busyIn(uint32 firstNumber, uint32 lastNumber) const;
	/// One of the cycles of these numbers gave the client more than half of the allowance as it is now.
	/// (The numbers come from the client: only the cycles that are kept are looked at, however far apart they are.)
	bool usedHalfIn(uint32 firstNumber, uint32 lastNumber) const;
	/// The client was held at the allowance in every cycle of these numbers
	bool heldIn(uint32 firstNumber, uint32 lastNumber) const;
	/// The first of the cycles before a datagram whose stream data a cut can be for: CutCycles before it, and not
	/// before the last cut, which was for the cycles before it
	uint32 firstCutCycle(uint32 number) const;
	/// The most that one of the cycles of these numbers gave the client while its congestion window held it (0: none)
	uint32 heldBackGrantIn(uint32 firstNumber, uint32 lastNumber) const;
	/// In one of the cycles of these numbers the client had stream data waiting and its congestion window no room
	bool waitingIn(uint32 firstNumber, uint32 lastNumber) const;
	/// Bring an allowance that the congestion window keeps the client far under down to what gets through
	void followWindow(const CStreamSchedulerConfig &config, uint32 sendNumber);
	/// What the allowance is cut to for the stream data of the cycles of these numbers (0: not at all)
	uint32 cutFor(uint32 firstNumber, uint32 lastNumber) const;
	void cut(uint32 sendNumber, uint32 allowance);

	CCycle m_Cycles[NbCycles];
	uint32 m_CurrentNumber;

	/// What a connection starts with, and the most the client's word allows
	uint32 startAllowance(const CStreamSchedulerConfig &config) const;
	uint32 ceiling(const CStreamSchedulerConfig &config) const;

	uint32 m_Declared;
	uint32 m_Allowance;
	bool m_StartUp;
	bool m_Started;
	/// The number of the datagram of the cycle of the last change: the datagrams after it show what the change did
	uint32 m_LastChangeNumber;
	bool m_LastChangeWasCut;
	/// The most lateness reported on datagrams with no stream data in the cycles before them: in the stretch of such
	/// reports that runs, and in the one before it
	uint32 m_NoiseNow;
	uint32 m_NoiseBefore;
	uint32 m_NoiseCount;
	uint32 m_NoiseCycles;
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
