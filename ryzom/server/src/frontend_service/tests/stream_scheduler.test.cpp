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

// The frontend's stream scheduler and allowance (stream_scheduler.h), each on a model of its own: the scheduler with
// clients that take what they are given, the allowance on a path of a fixed rate that queues what it does not take
// (no congestion window; the client reports the smallest lateness of its last three datagrams, two cycles later).
// These pin down what the reviews of the allowance found; stream_sim.test.cpp runs both on a transport with a window.

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"

#include <chrono>
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

#include "stream_scheduler.h"

#define CHECK(cond) EXPECT_TRUE(cond)

namespace /* anonymous */ {

// A frontend with clients, cycle by cycle. What it sends in a cycle is measured in the next one.
struct CSim
{
	CStreamSchedulerConfig Config;
	CStreamScheduler Scheduler;
	std::vector<CStreamSchedulerClient> Clients;
	std::vector<uint64> Pending; // bytes that wait
	std::vector<uint32> Window; // what the client's own path takes per cycle
	std::vector<uint64> Total; // bytes granted so far
	std::vector<uint32> LastGrant;
	uint64 DatagramBytes; // per cycle, all clients
	uint64 LastSent;
	uint64 SentTotal;
	uint32 Ticks;
	uint32 MaxGrant;
	bool MinimumAlways;

	CSim(uint n)
	    : Clients(n)
	    , Pending(n, 0)
	    , Window(n, 65536)
	    , Total(n, 0)
	    , LastGrant(n, 0)
	    , DatagramBytes(0)
	    , LastSent(0)
	    , SentTotal(0)
	    , Ticks(0)
	    , MaxGrant(0)
	    , MinimumAlways(true)
	{
	}

	void tick()
	{
		std::vector<CStreamScheduler::CEntry> entries(Clients.size());
		for (size_t i = 0; i < Clients.size(); ++i)
		{
			entries[i].Client = &Clients[i];
			entries[i].Id = (uint32)i;
			entries[i].Pending = (uint32)std::min(Pending[i], (uint64)0x7FFFFFFF);
			entries[i].Window = Window[i];
		}
		Scheduler.schedule(Config, Config.TickSeconds, LastSent, DatagramBytes, entries);
		uint64 sent = DatagramBytes;
		for (size_t i = 0; i < Clients.size(); ++i)
		{
			const uint32 grant = entries[i].Grant;
			CHECK(grant <= entries[i].Pending);
			CHECK(grant <= entries[i].Window);
			CHECK(grant <= Config.StreamMaxBytesPerTick);
			if (entries[i].Pending != 0 && entries[i].Window != 0)
			{
				const uint32 minimum = std::min(Config.StreamMinBytesPerTick, std::min(entries[i].Pending, entries[i].Window));
				if (grant < minimum)
					MinimumAlways = false;
			}
			else
			{
				CHECK(grant == 0);
			}
			Pending[i] -= grant;
			Total[i] += grant;
			LastGrant[i] = grant;
			MaxGrant = std::max(MaxGrant, grant);
			sent += grant;
		}
		LastSent = sent;
		SentTotal += sent;
		++Ticks;
	}
};

static double jain(const std::vector<uint64> &values)
{
	double sum = 0, squares = 0;
	for (size_t i = 0; i < values.size(); ++i)
	{
		sum += (double)values[i];
		squares += (double)values[i] * (double)values[i];
	}
	return squares == 0 ? 1.0 : sum * sum / (values.size() * squares);
}

// Many clients with a lasting backlog: the frontend sends its budget, no more, and shares it evenly. (Not all of it:
// the last cycle of a round has fewer clients to serve than it could, and what it leaves is not saved up.)
TEST(StreamScheduler, KeepsTheBudgetAndSharesEvenly)
{
	CSim sim(200);
	sim.Config.TotalOutgoingBandwidth = 5000000; // 500 KB per cycle
	sim.DatagramBytes = 200 * 354;
	for (uint i = 0; i < 200; ++i)
		sim.Pending[i] = 100000000;
	for (uint t = 0; t < 300; ++t)
		sim.tick();
	const double perTick = (double)sim.SentTotal / sim.Ticks;
	printf("budget: %.0f bytes per cycle of 500000, fairness %.4f, round %u\n", perTick, jain(sim.Total), sim.Scheduler.round());
	CHECK(perTick > 500000 * 0.96 && perTick <= 500000);
	CHECK(jain(sim.Total) > 0.99);
	CHECK(sim.MinimumAlways);
	CHECK(sim.Scheduler.round() > 10);
	CHECK(sim.Scheduler.overshootDatagrams() == 0);
}

// The worked example of the design: 1000 clients at 5 MB/s, a login wave of 50 that need 60 KB each
TEST(StreamScheduler, LoginWave)
{
	CSim sim(1000);
	sim.Config.TotalOutgoingBandwidth = 5000000;
	sim.DatagramBytes = 1000 * 354;
	for (uint t = 0; t < 5; ++t)
		sim.tick(); // idle cycles first
	for (uint i = 0; i < 50; ++i)
		sim.Pending[i * 20] = 60000;
	uint doneAt = 0;
	for (uint t = 1; t <= 60 && doneAt == 0; ++t)
	{
		sim.tick();
		bool all = true;
		for (uint i = 0; i < 50; ++i)
			all = all && sim.Pending[i * 20] == 0;
		if (all)
			doneAt = t;
	}
	printf("login wave: 50 clients of 60 KB done after %u cycles\n", doneAt);
	CHECK(doneAt != 0 && doneAt <= 24);
	CHECK(sim.MinimumAlways);
}

// Clients whose own path takes little leave their part of the pool to the others
TEST(StreamScheduler, SlowClientsLeaveTheirPartToTheOthers)
{
	CSim sim(10);
	sim.Config.TotalOutgoingBandwidth = 1000000; // 100 KB per cycle
	for (uint i = 0; i < 10; ++i)
	{
		sim.Pending[i] = 100000000;
		sim.Window[i] = i == 9 ? 32768 : 1000;
	}
	for (uint t = 0; t < 100; ++t)
		sim.tick();
	printf("water filling: slow %.0f, fast %.0f bytes per cycle\n", (double)sim.Total[0] / sim.Ticks, (double)sim.Total[9] / sim.Ticks);
	CHECK(sim.Total[0] >= (uint64)sim.Ticks * 990);
	CHECK(sim.Total[9] >= (uint64)sim.Ticks * 32000);
}

// A pool that is nearly empty: rounds still complete, nobody is left out, a turn is at least StreamMinChunk
TEST(StreamScheduler, ScarcePool)
{
	CSim sim(20);
	sim.Config.TotalOutgoingBandwidth = 100000; // 10 KB per cycle
	sim.DatagramBytes = 3000; // leaves 7 KB, of which the minimums take 4 KB
	for (uint i = 0; i < 20; ++i)
		sim.Pending[i] = 100000000;
	uint32 lastRound = sim.Scheduler.round();
	uint longestRound = 0, current = 0;
	for (uint t = 0; t < 2000; ++t)
	{
		sim.tick();
		++current;
		if (sim.Scheduler.round() != lastRound)
		{
			longestRound = std::max(longestRound, current);
			current = 0;
			lastRound = sim.Scheduler.round();
		}
	}
	const double perTick = (double)sim.SentTotal / sim.Ticks;
	printf("scarce pool: %.0f bytes per cycle of 10000, longest round %u cycles, fairness %.4f\n", perTick, longestRound, jain(sim.Total));
	CHECK(perTick > 10000 * 0.97 && perTick < 10000 * 1.03);
	CHECK(sim.MinimumAlways);
	CHECK(longestRound != 0 && longestRound <= 20 * 4096 / 3000 + 4); // about clients x turn / pool
	CHECK(jain(sim.Total) > 0.98);
}

// A client that gets stream data while a round runs joins the next round, and waits no longer than that
TEST(StreamScheduler, NewcomerJoinsTheNextRound)
{
	CSim sim(30);
	sim.Config.TotalOutgoingBandwidth = 200000; // 20 KB per cycle
	for (uint i = 0; i < 29; ++i)
		sim.Pending[i] = 100000000;
	for (uint t = 0; t < 50; ++t)
		sim.tick();
	const uint32 roundAtArrival = sim.Scheduler.round();
	sim.Pending[29] = 1000000;
	uint waited = 0;
	for (uint t = 0; t < 400; ++t)
	{
		sim.tick();
		if (sim.LastGrant[29] > sim.Config.StreamMinBytesPerTick)
			break;
		CHECK(sim.LastGrant[29] == sim.Config.StreamMinBytesPerTick); // the minimum meanwhile
		++waited;
	}
	printf("newcomer: waited %u cycles, from round %u to round %u\n", waited, roundAtArrival, sim.Scheduler.round());
	CHECK(waited < 400);
	CHECK(sim.Scheduler.round() <= roundAtArrival + 2);

	// On a frontend with nothing else to send, a client is served in the cycle its data comes
	CSim idle(3);
	for (uint t = 0; t < 5; ++t)
		idle.tick();
	idle.Pending[1] = 50000;
	idle.tick();
	CHECK(idle.LastGrant[1] == 50000);
}

// Half of the clients have a small window of their own: they get as much over time as the others when the pool is
// what limits
TEST(StreamScheduler, SlowAndFastClientsGetAsMuch)
{
	CSim sim(200);
	sim.Config.TotalOutgoingBandwidth = 1500000; // 150 KB per cycle, 750 per client
	for (uint i = 0; i < 200; ++i)
	{
		sim.Pending[i] = 100000000;
		if (i % 2)
			sim.Window[i] = 2000;
	}
	for (uint t = 0; t < 3000; ++t)
		sim.tick();
	double slow = 0, fast = 0;
	for (uint i = 0; i < 200; ++i)
		(i % 2 ? slow : fast) += (double)sim.Total[i] / 100.0 / sim.Ticks;
	const double perTick = (double)sim.SentTotal / sim.Ticks;
	printf("slow and fast: %.0f and %.0f bytes per cycle each, %.0f of 150000 in all\n", slow, fast, perTick);
	CHECK(slow > fast * 0.9 && slow < fast * 1.1);
	CHECK(perTick > 150000 * 0.94 && perTick <= 150000);
}

// Datagrams above the budget: the streams get their minimums and nothing else, and it is counted
TEST(StreamScheduler, DatagramsAboveTheBudget)
{
	CSim sim(100);
	sim.Config.TotalOutgoingBandwidth = 300000; // 30 KB per cycle
	sim.DatagramBytes = 100 * 400; // 40 KB
	for (uint i = 0; i < 100; ++i)
		sim.Pending[i] = 1000000;
	for (uint t = 0; t < 50; ++t)
		sim.tick();
	CHECK(sim.MaxGrant == sim.Config.StreamMinBytesPerTick);
	CHECK(sim.MinimumAlways);
	CHECK(sim.Scheduler.overshootDatagrams() >= 49);

	// A frontend that sent far above its budget owes no more than a cycle of it: it is back within a few cycles
	CSim debt(10);
	debt.Config.TotalOutgoingBandwidth = 1000000; // 100 KB per cycle
	for (uint i = 0; i < 10; ++i)
		debt.Pending[i] = 100000000;
	debt.tick();
	debt.LastSent = 50000000; // as if 50 MB had gone out
	uint back = 0;
	for (uint t = 1; t <= 20 && back == 0; ++t)
	{
		debt.tick();
		if (debt.MaxGrant > debt.Config.StreamMinBytesPerTick && debt.LastGrant[0] + debt.LastGrant[5] > 2 * debt.Config.StreamMinBytesPerTick)
			back = t;
	}
	CHECK(back != 0 && back <= 4);
}

// The allowance of one client on a path that takes `rate` bytes per cycle. A model without congestion window: what
// the path does not take in a cycle waits in its queue, and the datagram of a cycle arrives behind what waited
// before it. The client reports two cycles later.
struct CPathSim
{
	CStreamSchedulerConfig Config;
	CStreamAllowance Allowance;
	double Rate;
	double Queue;
	uint32 SendNumber;
	std::vector<uint32> Lateness; // by datagram number
	uint32 MaxLateness;
	uint32 LateCycles;
	uint32 MaxAllowance;
	uint64 Sent;
	uint32 Changes;
	uint32 CongestionEvents;
	uint32 SendBusyDrops;
	bool Reports;
	uint32 OtherLateness; // milliseconds that every datagram is late for a reason that is not the streams

	CPathSim(double rate)
	    : Rate(rate)
	    , Queue(0)
	    , SendNumber(0)
	    , MaxLateness(0)
	    , LateCycles(0)
	    , MaxAllowance(0)
	    , Sent(0)
	    , Changes(0)
	    , CongestionEvents(0)
	    , SendBusyDrops(0)
	    , Reports(true)
	    , OtherLateness(0)
	{
		Allowance.reset(Config);
	}

	// pending: stream bytes that wait for the client. Returns what it got.
	uint32 tick(uint64 pending)
	{
		++SendNumber;
		// this cycle's datagram arrives behind the queue
		const uint32 late = (uint32)std::min(Queue / Rate * 100.0 + OtherLateness, 1143.0);
		Lateness.push_back(late);
		MaxLateness = std::max(MaxLateness, late);
		if (late > Config.QueueDelayTarget)
			++LateCycles;

		const uint32 before = Allowance.allowance();
		const bool report = Reports && SendNumber > 2;
		const uint32 reportNumber = report ? SendNumber - 2 : 0;
		uint32 reported = 0;
		if (report)
		{
			// what the client reports: the smallest lateness of the last three datagrams it has
			reported = 1143;
			for (uint32 n = reportNumber; n >= 1 && n + 3 > reportNumber; --n)
				reported = std::min(reported, Lateness[n - 1]);
		}
		Allowance.update(Config, report, reported, reportNumber, SendNumber, CongestionEvents, SendBusyDrops);
		if (Allowance.allowance() != before)
			++Changes;
		MaxAllowance = std::max(MaxAllowance, Allowance.allowance());

		const uint32 grant = (uint32)std::min((uint64)Allowance.allowance(), pending);
		Allowance.granted(grant, pending > grant);
		Sent += grant;
		Queue = std::max(0.0, Queue + 500 + grant - Rate);
		return grant;
	}
};

// 270 KB at `rate` bytes per cycle after `idle` cycles without stream data. Returns the cycles the transfer took.
static uint transferAfterIdle(double rate, uint idle, uint32 reportDatagrams, uint32 *maxLateness = nullptr, uint32 *lateCycles = nullptr)
{
	CPathSim path(rate);
	path.Config.ReportDatagrams = reportDatagrams;
	for (uint t = 0; t < idle; ++t)
		path.tick(0);
	uint64 pending = 270000;
	uint ticks = 0;
	while (pending != 0 && ticks < 2000)
	{
		pending -= path.tick(pending);
		++ticks;
	}
	if (maxLateness)
		*maxLateness = path.MaxLateness;
	if (lateCycles)
		*lateCycles = path.LateCycles;
	return ticks;
}

TEST(StreamAllowance, IdleStretches)
{
	// A client that was idle (a login: synchronising, the character selection) grows as one that starts
	for (uint32 span = 1; span <= 3; span += 2)
	{
		uint32 maxLate = 0, lateCycles = 0;
		const uint fromStart = transferAfterIdle(25000, 0, span, &maxLate, &lateCycles);
		printf("allowance, reports judged after %u datagrams: 270 KB at 2 Mbit/s in %u cycles (at most %u ms late, %u cycles above the target)", span, fromStart, maxLate, lateCycles);
		const uint idles[] = { 10, 50, 300, 3000 };
		for (uint i = 0; i < 4; ++i)
		{
			const uint afterIdle = transferAfterIdle(25000, idles[i], span);
			printf(", after %u idle cycles %u", idles[i], afterIdle);
			CHECK(afterIdle <= fromStart + 3);
		}
		printf("; on a fast path %u cycles\n", transferAfterIdle(1000000, 0, span));
	}

	for (uint32 span = 1; span <= 3; span += 2)
	{
		const double rates[] = { 12500, 25000, 62500 }; // 1, 2 and 5 Mbit/s
		for (uint r = 0; r < 3; ++r)
		{
			CPathSim path(rates[r]);
			path.Config.ReportDatagrams = span;
			for (uint t = 0; t < 3000; ++t)
				path.tick(100000000);
			printf("lasting backlog, reports judged after %u datagrams, path of %.0f bytes per cycle: %.0f sent per cycle, %u of 3000 cycles above the target, at most %u ms late\n",
			    span, rates[r], (double)path.Sent / 3000, path.LateCycles, path.MaxLateness);
		}
	}

	// Losses while no stream data flows do not cut the allowance: a burst after each of them is as fast as the first
	{
		CPathSim path(1000000);
		uint first = 0, last = 0;
		for (uint episode = 0; episode < 8; ++episode)
		{
			for (uint t = 0; t < 300; ++t)
				path.tick(0);
			++path.CongestionEvents; // a datagram was lost
			for (uint t = 0; t < 300; ++t)
				path.tick(0);
			uint64 pending = 60000;
			uint ticks = 0;
			while (pending != 0 && ticks < 2000)
			{
				pending -= path.tick(pending);
				++ticks;
			}
			if (episode == 0)
				first = ticks;
			last = ticks;
		}
		printf("allowance, a loss while idle before each burst of 60 KB: first burst %u cycles, eighth %u\n", first, last);
		CHECK(last <= first);
	}

	// An allowance that was cut grows by packets; after ten seconds without stream data it grows by quarters again,
	// from where it stands
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 40; ++t)
			path.tick(100000000);
		++path.SendBusyDrops;
		for (uint t = 0; t < 6; ++t)
			path.tick(100000000);
		CHECK(!path.Allowance.startUp());
		const uint32 cut = path.Allowance.allowance();

		// five seconds idle: still by packets (the first idle cycles may still bring the report on the last busy ones)
		for (uint t = 0; t < 10; ++t)
			path.tick(0);
		uint32 before = path.Allowance.allowance();
		for (uint t = 0; t < 40; ++t)
			path.tick(0);
		CHECK(!path.Allowance.startUp() && path.Allowance.allowance() == before);
		for (uint t = 0; t < 10; ++t)
			path.tick(100000000);
		CHECK(!path.Allowance.startUp());
		CHECK(path.Allowance.allowance() > before && path.Allowance.allowance() <= before + 2 * 560);

		// ten seconds idle: by quarters, from the same allowance
		for (uint t = 0; t < 10; ++t)
			path.tick(0);
		before = path.Allowance.allowance();
		for (uint t = 0; t < 90; ++t)
			path.tick(0);
		CHECK(path.Allowance.startUp() && path.Allowance.allowance() == before);
		for (uint t = 0; t < 10; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.startUp());
		CHECK(path.Allowance.allowance() >= before + before / 4);
		printf("allowance after a cut: %u, by packets to %u in a second, after ten idle seconds by quarters to %u in a second\n", cut, before, path.Allowance.allowance());
	}

	// With a minimum of 0, an allowance that was cut to nothing still grows again
	{
		CPathSim path(1000000);
		path.Config.StreamMinBytesPerTick = 0;
		for (uint t = 0; t < 200; ++t)
		{
			++path.SendBusyDrops;
			path.tick(100000000);
		}
		CHECK(path.Allowance.allowance() >= 1 && path.Allowance.allowance() < 100);
		for (uint t = 0; t < 400; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() > 10000);
	}

	// A new session on the connection, with datagram numbers that start over: the allowance goes on from there
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 30; ++t)
			path.tick(100000000);
		const uint32 allowance = path.Allowance.allowance();
		path.SendNumber = 0;
		path.Lateness.clear();
		path.Allowance.newSession();
		for (uint t = 0; t < 30; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() > allowance);
	}
}

// The cases the second review of the allowance found
TEST(StreamAllowance, EvidenceForRaisesAndCuts)
{
	// Small messages now and then, and a datagram lost every minute, for an hour: the streams are not the cause of
	// those losses, and a string of 300 bytes cannot hold a datagram up. Nothing is cut, and nothing is raised.
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 36000; ++t)
		{
			if (t % 600 == 299)
				++path.CongestionEvents;
			path.tick(t % 50 == 0 ? 300 : 0);
		}
		CHECK(path.Allowance.allowance() == path.Config.StreamMinChunk);
		CHECK(path.Changes == 0);
		uint64 pending = 270000;
		uint ticks = 0;
		while (pending != 0 && ticks < 2000)
		{
			pending -= path.tick(pending);
			++ticks;
		}
		printf("allowance after an hour of small strings and a loss per minute: %u; 270 KB then in %u cycles\n", path.Config.StreamMinChunk, ticks);
		CHECK(ticks <= 36);
	}

	// The cases the third review found. A steady trickle of stream data, far under the allowance, for an hour, with
	// a datagram lost every minute and five datagrams 40 ms late every two minutes: a cut would take nothing away
	// from this client, and nothing is cut
	{
		const uint32 sizes[] = { 100, 600, 2000 };
		const uint periods[] = { 1, 10, 50 };
		for (uint i = 0; i < 3; ++i)
		{
			CPathSim path(1000000);
			for (uint t = 0; t < 36000; ++t)
			{
				if (t % 600 == 299)
					++path.CongestionEvents;
				path.OtherLateness = (t % 1200 >= 700 && t % 1200 < 705) ? 40 : 0;
				path.tick(t % periods[i] == 0 ? sizes[i] : 0);
			}
			CHECK(path.Allowance.allowance() == path.Config.StreamMinChunk);
			CHECK(path.Changes == 0);
		}
	}

	// One message of 600 bytes, and 30 ms of lateness from another cause from then on; and small strings through 40
	// seconds of such lateness: nothing is cut
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 300; ++t)
			path.tick(0);
		path.tick(600);
		path.OtherLateness = 30;
		for (uint t = 0; t < 600; ++t)
			path.tick(0);
		CHECK(path.Changes == 0);

		CPathSim strings(1000000);
		for (uint t = 0; t < 1200; ++t)
		{
			strings.OtherLateness = (t >= 300 && t < 700) ? 30 : 0;
			strings.tick(t % 50 == 0 ? 2000 : 0);
		}
		CHECK(strings.Changes == 0);
	}

	// A lasting backlog on paths of every speed: no episode cuts twice
	{
		uint paths = 0, closeCuts = 0, cutsInAll = 0;
		for (double rate = 4000; rate <= 70000; rate += 600)
		{
			CPathSim path(rate);
			uint32 lastAllowance = 0;
			uint lastCut = 0;
			bool cutBefore = false;
			for (uint t = 0; t < 1500; ++t)
			{
				path.tick(100000000);
				const uint32 allowance = path.Allowance.allowance();
				if (t != 0 && allowance < lastAllowance)
				{
					if (cutBefore && t - lastCut <= 8)
						++closeCuts;
					++cutsInAll;
					cutBefore = true;
					lastCut = t;
				}
				lastAllowance = allowance;
			}
			++paths;
		}
		printf("lasting backlog on %u paths from 4000 to 70000 bytes per cycle: %u cuts, %u of them within 8 cycles of the one before\n", paths, cutsInAll, closeCuts);
		CHECK(closeCuts == 0);
	}

	// A path that drops to a fifth of what it took: the queue is deep, and the allowance comes down to what the path
	// takes now, not to the minimum
	{
		CPathSim path(62500);
		for (uint t = 0; t < 400; ++t)
			path.tick(100000000);
		const uint32 before = path.Allowance.allowance();
		path.Rate = 12500;
		uint32 lowest = before;
		for (uint t = 0; t < 600; ++t)
		{
			path.tick(100000000);
			lowest = std::min(lowest, path.Allowance.allowance());
		}
		printf("path from 62500 to 12500 bytes per cycle: allowance %u before, never under %u after, %u at the end\n", before, lowest, path.Allowance.allowance());
		CHECK(lowest >= 2000);
		CHECK(path.Allowance.allowance() < 12500 * 1.3); // (it moves around what the path takes)
	}

	// A message of 30 KB every three seconds on a path of 1 Mbit/s: a cycle or two at the allowance are no evidence
	// that the path takes more, so the allowance stops growing where a message no longer holds it for three cycles
	{
		CPathSim path(12500);
		uint32 worstAfterStart = 0;
		for (uint message = 0; message < 60; ++message)
		{
			uint64 pending = 30000;
			for (uint t = 0; t < 30; ++t)
			{
				pending -= path.tick(pending);
				if (message >= 10)
					worstAfterStart = std::max(worstAfterStart, path.Lateness.back());
			}
			CHECK(pending == 0);
		}
		printf("allowance with a 30 KB message every 3 s at 1 Mbit/s: settles at %u, datagrams at most %u ms late once settled (%u in all)\n", path.Allowance.allowance(), worstAfterStart, path.MaxLateness);
		CHECK(path.Allowance.allowance() < 12500);
		CHECK(worstAfterStart <= 20);
	}

	// One byte more than the allowance once a second is no evidence either
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 600; ++t)
			path.tick(t % 10 == 0 ? path.Allowance.allowance() + 1 : 0);
		CHECK(path.Allowance.allowance() == path.Config.StreamMinChunk);
	}

	// A lasting backlog on slow paths: one episode of lateness does not cut twice while the queue drains
	{
		const double rates[] = { 12500, 40000 };
		for (uint r = 0; r < 2; ++r)
		{
			CPathSim path(rates[r]);
			uint32 lowest = 0xFFFFFFFF, lastAllowance = 0;
			uint cuts = 0, closeCuts = 0, lastCut = 0;
			for (uint t = 0; t < 3000; ++t)
			{
				path.tick(100000000);
				const uint32 allowance = path.Allowance.allowance();
				if (t != 0 && allowance < lastAllowance)
				{
					if (cuts != 0 && t - lastCut <= 8)
						++closeCuts;
					++cuts;
					lastCut = t;
				}
				lastAllowance = allowance;
				if (t > 100)
					lowest = std::min(lowest, allowance);
			}
			printf("lasting backlog on a path of %.0f bytes per cycle: %u cuts, %u of them within 8 cycles of the one before, allowance never under %u after the start, %.0f sent per cycle\n",
			    rates[r], cuts, closeCuts, lowest, (double)path.Sent / 3000);
			CHECK((double)path.Sent / 3000 > rates[r] * 0.7);
			CHECK(closeCuts == 0);
		}
	}

	// With a minimum of 0: an allowance cut to nothing, then ten idle seconds, still grows (a quarter of 1 is 0)
	{
		CPathSim path(1000000);
		path.Config.StreamMinBytesPerTick = 0;
		for (uint t = 0; t < 400; ++t)
		{
			++path.SendBusyDrops;
			path.tick(100000000);
		}
		const uint32 low = path.Allowance.allowance();
		for (uint t = 0; t < 110; ++t)
			path.tick(0);
		CHECK(path.Allowance.startUp());
		for (uint t = 0; t < 400; ++t)
			path.tick(100000000);
		printf("allowance cut to %u with a minimum of 0, after ten idle seconds and 40 s of backlog: %u\n", low, path.Allowance.allowance());
		CHECK(low < 1000);
		CHECK(path.Allowance.allowance() > 20000);
	}
}

TEST(StreamAllowance, GrowthCutsAndBounds)
{
	// 2 Mbit/s: 25 KB per cycle. 270 KB to send.
	{
		CPathSim path(25000);
		uint64 pending = 270000;
		uint ticks = 0;
		while (pending != 0 && ticks < 500)
		{
			pending -= path.tick(pending);
			++ticks;
		}
		printf("allowance at 2 Mbit/s: 270 KB in %u cycles, datagrams at most %u ms late, %u cycles above the target, allowance up to %u\n", ticks, path.MaxLateness, path.LateCycles, path.MaxAllowance);
		CHECK(pending == 0 && ticks <= 40);
		CHECK(path.MaxLateness <= 60);
		CHECK(path.LateCycles <= 6);
	}

	// The same path with a lasting backlog: the allowance settles around what the path takes
	{
		CPathSim path(25000);
		for (uint t = 0; t < 3000; ++t)
			path.tick(100000000);
		const double perTick = (double)path.Sent / 3000;
		printf("allowance at 2 Mbit/s, lasting: %.0f bytes per cycle, %u of 3000 cycles above the target, at most %u ms\n", perTick, path.LateCycles, path.MaxLateness);
		CHECK(perTick > 25000 * 0.6 && perTick < 24500);
		CHECK(path.LateCycles < 3000 / 5);
		CHECK(path.MaxLateness <= 130);
	}

	// A fast path: the allowance reaches the most a client gets, and stays
	{
		CPathSim path(1000000);
		uint reached = 0;
		for (uint t = 1; t <= 400; ++t)
		{
			path.tick(100000000);
			if (reached == 0 && path.Allowance.allowance() == path.Config.StreamMaxBytesPerTick)
				reached = t;
		}
		printf("allowance on a fast path: at the maximum after %u cycles\n", reached);
		CHECK(reached != 0 && reached <= 70);
		CHECK(path.MaxLateness == 0);
		CHECK(path.Allowance.allowance() == path.Config.StreamMaxBytesPerTick);
	}

	// A client that does not use what it is allowed gets no more
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 200; ++t)
			path.tick(300);
		CHECK(path.Allowance.allowance() == path.Config.StreamMinChunk);
	}

	// One change per report that covers only datagrams sent after the change before: with reports two cycles behind
	// that cover three datagrams, one change in five cycles
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 30; ++t)
			path.tick(100000000);
		CHECK(path.Changes >= 5 && path.Changes <= 6);
	}

	// No reports (the client's main thread is busy): nothing changes; and the first report after that does not raise
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 20; ++t)
			path.tick(100000000);
		const uint32 allowance = path.Allowance.allowance();
		path.Reports = false;
		for (uint t = 0; t < 50; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() == allowance);
		path.Reports = true;
		path.tick(100000000);
		CHECK(path.Allowance.allowance() == allowance);
		for (uint t = 0; t < 10; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() > allowance);
	}

	// A datagram that could not be sent halves it once. Congestion that the connection saw does not: its congestion
	// window answers for a loss, which takes the room for stream data with it, and a loss is not always congestion.
	{
		CPathSim path(1000000);
		for (uint t = 0; t < 40; ++t)
			path.tick(100000000);
		uint32 allowance = path.Allowance.allowance();
		path.CongestionEvents = 1;
		for (uint t = 0; t < 3; ++t) // (the next report on a datagram sent after the last change is at most that far)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() >= allowance);
		for (uint t = 0; t < 20; ++t)
			path.tick(100000000);
		allowance = path.Allowance.allowance();
		path.SendBusyDrops = 3;
		// (with the next report on datagrams that were all sent after the last change, which is a few cycles away
		// while the allowance still grows)
		for (uint t = 0; t < 8 && path.Allowance.allowance() == allowance; ++t)
			path.tick(100000000);
		EXPECT_EQ(path.Allowance.allowance(), allowance / 2);
		EXPECT_FALSE(path.Allowance.startUp());
		for (uint t = 0; t < 20; ++t)
			path.tick(100000000);
		CHECK(path.Allowance.allowance() > allowance / 2); // and it grows again, by packets

		// never below the minimum
		for (uint t = 0; t < 100; ++t)
		{
			++path.SendBusyDrops;
			path.tick(100000000);
		}
		CHECK(path.Allowance.allowance() == path.Config.StreamMinBytesPerTick);

		path.Allowance.reset(path.Config);
		CHECK(path.Allowance.allowance() == path.Config.StreamMinChunk && path.Allowance.startUp());
	}
}

// The number of the datagram a report is on is the client's word. One far ahead of what the frontend sent made the
// allowance look through two thousand million cycles, on the main thread of the service.
TEST(StreamAllowance, ReportOnADatagramThatWasNotSent)
{
	CStreamSchedulerConfig config;
	CStreamAllowance allowance;
	allowance.reset(config);
	uint32 number = 100;
	// Stream data at the allowance, and reports without lateness on the datagram of two cycles before
	for (uint i = 0; i < 10; ++i, ++number)
	{
		allowance.update(config, true, 0, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	// The datagrams are late: a cut, and then reports that stay late with no stream data any more
	const uint32 before = allowance.allowance();
	allowance.update(config, true, 1144, number - 2, number, 0, 0);
	allowance.granted(0, false);
	++number;
	const uint32 afterCut = allowance.allowance();
	EXPECT_LT(afterCut, before);
	for (uint i = 0; i < 6; ++i, ++number)
	{
		allowance.update(config, true, 1144, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.allowance(), afterCut);

	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	const uint32 wild[] = { 0x7FFFFFF0u, 0x40000000u, 0x80000000u, 0xC0000000u, 33u, 1000u };
	for (size_t i = 0; i < sizeof(wild) / sizeof(wild[0]); ++i, ++number)
	{
		// (ahead of what was sent, and as far behind it)
		allowance.update(config, true, 1144, number + wild[i], number, 0, 0);
		allowance.granted(0, false);
		++number;
		allowance.update(config, true, 1144, number - wild[i], number, 0, 0);
		allowance.granted(0, false);
		++number;
		allowance.update(config, true, 0, number + wild[i], number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	EXPECT_LT(seconds, 0.2);
	// (none of them was a reason to raise it either)
	EXPECT_LE(allowance.allowance(), afterCut);
}

// A cut is for the stream data of the cycles before it. Lateness that comes back after it, with no stream data since,
// is not a reason to cut again for those same cycles.
TEST(StreamAllowance, NoSecondCutForWhatWasSentBeforeTheFirst)
{
	CStreamSchedulerConfig config;
	CStreamAllowance allowance;
	allowance.reset(config);
	uint32 number = 100;
	for (uint i = 0; i < 6; ++i, ++number)
	{
		allowance.update(config, true, 0, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	const uint32 before = allowance.allowance();
	allowance.update(config, true, 30, number - 2, number, 0, 0);
	allowance.granted(0, false);
	++number;
	const uint32 afterCut = allowance.allowance();
	EXPECT_LT(afterCut, before);
	// The lateness goes away on the datagrams of after the cut: that episode is over
	for (uint i = 0; i < 5; ++i, ++number)
	{
		allowance.update(config, true, 0, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	// Late again, for something else: the client got no stream data since the cut
	for (uint i = 0; i < 4; ++i, ++number)
	{
		allowance.update(config, true, 30, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.allowance(), afterCut);
}

// A frontend with clients that always have more to send, and what its clients report
struct CBudgetRun
{
	CStreamSchedulerConfig Config;
	CStreamScheduler Scheduler;
	std::vector<CStreamSchedulerClient> Clients;
	std::vector<uint32> TimesServed;
	uint64 DatagramBytes; // per cycle
	uint64 LastSent;
	uint64 LastGranted;

	explicit CBudgetRun(uint n)
	    : Clients(n)
	    , TimesServed(n, 0)
	    , DatagramBytes(0)
	    , LastSent(0)
	    , LastGranted(0)
	{
	}

	void cycle(uint32 reporting, uint32 late)
	{
		std::vector<CStreamScheduler::CEntry> entries(Clients.size());
		for (size_t i = 0; i < Clients.size(); ++i)
		{
			entries[i].Client = &Clients[i];
			entries[i].Id = (uint32)i;
			entries[i].Pending = 1000000;
			entries[i].Window = 65536;
		}
		Scheduler.reports(Config, reporting, late);
		Scheduler.schedule(Config, Config.TickSeconds, LastSent, DatagramBytes, entries);
		LastGranted = 0;
		for (size_t i = 0; i < Clients.size(); ++i)
		{
			LastGranted += entries[i].Grant;
			if (entries[i].Grant != 0)
				++TimesServed[i];
		}
		LastSent = DatagramBytes + LastGranted;
	}
};

// The datagrams of half the clients late at once: the link of the frontend carries less than its budget says, and
// the frontend holds itself to less
TEST(StreamScheduler, SharedLatenessTakesTheBudgetDown)
{
	CBudgetRun run(20);
	run.Config.TotalOutgoingBandwidth = 1000000; // 100 KB per cycle
	run.DatagramBytes = 40000;
	for (uint t = 0; t < 50; ++t)
		run.cycle(20, 0);
	EXPECT_EQ(run.Scheduler.budget(run.Config), 1000000u);
	EXPECT_FALSE(run.Scheduler.sharedLateness());
	EXPECT_GT(run.LastGranted, (uint64)50000);

	// Some of them late is of their own paths
	for (uint t = 0; t < 50; ++t)
		run.cycle(20, 9);
	EXPECT_EQ(run.Scheduler.budget(run.Config), 1000000u);
	EXPECT_FALSE(run.Scheduler.sharedLateness());

	// Too few that report do not tell
	for (uint t = 0; t < 50; ++t)
		run.cycle(7, 7);
	EXPECT_EQ(run.Scheduler.budget(run.Config), 1000000u);
	EXPECT_FALSE(run.Scheduler.sharedLateness());

	// Half of them: down to 85% of what it sends, and again when the cut has had time to show
	run.cycle(20, 10);
	EXPECT_TRUE(run.Scheduler.sharedLateness());
	const uint32 first = run.Scheduler.budget(run.Config);
	EXPECT_GT(first, 800000u);
	EXPECT_LT(first, 870000u);
	for (uint t = 0; t < 5; ++t)
		run.cycle(20, 10);
	EXPECT_EQ(run.Scheduler.budget(run.Config), first);
	run.cycle(20, 10);
	EXPECT_LT(run.Scheduler.budget(run.Config), first);
	EXPECT_EQ(run.Scheduler.budgetCuts(), (uint64)2);

	// For as long as it lasts: not under what the datagrams take, which go whatever the budget, and then the streams
	// get nothing, their minimums included
	for (uint t = 0; t < 300; ++t)
		run.cycle(20, 20);
	EXPECT_GE(run.Scheduler.budget(run.Config), 400000u);
	EXPECT_LE(run.Scheduler.budget(run.Config), 401000u);
	EXPECT_EQ(run.LastGranted, (uint64)0);

	// On time again: after a second it comes back, by 1% of the budget per second
	for (uint t = 0; t < 9; ++t)
		run.cycle(20, 0);
	EXPECT_LE(run.Scheduler.budget(run.Config), 401000u);
	for (uint t = 0; t < 101; ++t)
		run.cycle(20, 1);
	EXPECT_GT(run.Scheduler.budget(run.Config), 495000u);
	EXPECT_LT(run.Scheduler.budget(run.Config), 505000u);
	// (what there is goes to the clients in turns, the minimums too: nobody is left out)
	std::fill(run.TimesServed.begin(), run.TimesServed.end(), 0);
	for (uint t = 0; t < 100; ++t)
		run.cycle(20, 1);
	for (size_t i = 0; i < run.TimesServed.size(); ++i)
		EXPECT_GT(run.TimesServed[i], 10u) << "client " << i;
	EXPECT_LE(run.LastSent, (uint64)run.Scheduler.budget(run.Config) / 10 + 4096);
	for (uint t = 0; t < 5000 && run.Scheduler.budget(run.Config) < 1000000u; ++t)
		run.cycle(20, 0);
	EXPECT_EQ(run.Scheduler.budget(run.Config), 1000000u);
	EXPECT_LT(run.Scheduler.budgetCuts(), (uint64)60);
}

// While the datagrams of half the clients are late, that is not held against any one of them
TEST(StreamAllowance, SharedLatenessIsNotTheClientsDoing)
{
	CStreamSchedulerConfig config;
	for (uint shared = 0; shared < 2; ++shared)
	{
		CStreamAllowance allowance;
		allowance.reset(config);
		uint32 number = 100;
		for (uint i = 0; i < 10; ++i, ++number)
		{
			allowance.update(config, true, 0, number - 2, number, 0, 0);
			allowance.granted(allowance.allowance(), true);
		}
		const uint32 before = allowance.allowance();
		for (uint i = 0; i < 10; ++i, ++number)
		{
			allowance.update(config, true, 150, number - 2, number, i, i, shared != 0);
			allowance.granted(allowance.allowance(), true);
		}
		if (shared)
			EXPECT_EQ(allowance.allowance(), before);
		else
			EXPECT_LT(allowance.allowance(), before);
	}
}

// Congestion that the connection saw is not a reason to raise the allowance on the report that comes with it, and
// not one to cut it either, however much stream data went out
TEST(StreamAllowance, LossAloneDoesNotCut)
{
	CStreamSchedulerConfig config;
	CStreamAllowance allowance;
	allowance.reset(config);
	uint32 number = 100;
	// (past the cycles after a reset in which nothing of the kind counts anyway)
	for (uint i = 0; i < 40; ++i, ++number)
	{
		allowance.update(config, true, 10, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	const uint32 before = allowance.allowance();
	for (uint i = 0; i < 40; ++i, ++number)
	{
		allowance.update(config, true, 0, number - 2, number, 1 + i, 0);
		allowance.granted(allowance.allowance(), true);
	}
	EXPECT_EQ(allowance.allowance(), before);
	// Without it the allowance goes up again
	for (uint i = 0; i < 10; ++i, ++number)
	{
		allowance.update(config, true, 0, number - 2, number, 40, 0);
		allowance.granted(allowance.allowance(), true);
	}
	EXPECT_GT(allowance.allowance(), before);
}

// What a client reports while it gets no stream data is the noise of its path and of its clock, and lateness is
// counted from there
TEST(StreamAllowance, NoiseOfThePathIsNotLateness)
{
	CStreamSchedulerConfig config;
	CStreamAllowance allowance;
	allowance.reset(config);
	uint32 number = 100;
	// No stream data, and reports of up to 30 ms
	for (uint i = 0; i < 30; ++i, ++number)
	{
		allowance.update(config, true, i % 31, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.noise(), 29u);

	// Stream data at the allowance, and reports within the target of that: no cut, and the allowance grows by
	// quarters where the reports are next to the noise
	const uint32 start = allowance.allowance();
	for (uint i = 0; i < 12; ++i, ++number)
	{
		allowance.update(config, true, 30, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	EXPECT_GE(allowance.allowance(), start + start / 4);
	EXPECT_TRUE(allowance.startUp());
	// (the first of those reports were on datagrams of before the stream data and count as noise; the others do not)
	EXPECT_EQ(allowance.noise(), 30u);
	uint32 before = allowance.allowance();
	for (uint i = 0; i < 12; ++i, ++number)
	{
		allowance.update(config, true, 45, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	EXPECT_GE(allowance.allowance(), before);

	// More than the target above it: a cut
	before = allowance.allowance();
	for (uint i = 0; i < 4; ++i, ++number)
	{
		allowance.update(config, true, 55, number - 2, number, 0, 0);
		allowance.granted(allowance.allowance(), true);
	}
	EXPECT_LT(allowance.allowance(), before);

	// What counts as noise has a limit, and a new path starts without any
	for (uint i = 0; i < 40; ++i, ++number)
	{
		allowance.update(config, true, 900, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.noise(), 100u);
	allowance.reset(config);
	EXPECT_EQ(allowance.noise(), 0u);

	// It is the most of the last two stretches of such reports: jitter that is gone is forgotten after them
	for (uint i = 0; i < 100; ++i, ++number)
	{
		allowance.update(config, true, 35, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.noise(), 35u);
	for (uint i = 0; i < 100; ++i, ++number)
	{
		allowance.update(config, true, 2, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.noise(), 35u);
	for (uint i = 0; i < 150; ++i, ++number)
	{
		allowance.update(config, true, 2, number - 2, number, 0, 0);
		allowance.granted(0, false);
	}
	EXPECT_EQ(allowance.noise(), 2u);
}

// An allowance far above what the path takes, with the congestion window of the connection holding the client back:
// half of the allowance would take nothing away, and no cycle of the last ten gave half of it. It is cut all the same,
// for what the client got or for what is still on its way.
TEST(StreamAllowance, CutWhereSomethingElseHoldsTheClient)
{
	CStreamSchedulerConfig config;
	for (uint variant = 0; variant < 3; ++variant)
	{
		SCOPED_TRACE(variant);
		CStreamAllowance allowance;
		allowance.reset(config);
		uint32 number = 100;
		for (uint i = 0; i < 60; ++i, ++number)
		{
			allowance.update(config, true, 0, number - 2, number, 0, 0);
			allowance.granted(allowance.allowance(), true);
		}
		ASSERT_GT(allowance.allowance(), 40000u);

		// Twelve cycles in which the client has stream data waiting and gets: nothing (its window is full of what it
		// got before); a tenth of its allowance or so; or, without more waiting, a little
		for (uint i = 0; i < 12; ++i, ++number)
		{
			allowance.update(config, true, 0, number - 2, number, 0, 0);
			if (variant == 0)
				allowance.granted(0, true);
			else if (variant == 1)
				allowance.granted(4000, true);
			else
				allowance.granted(300, false);
		}
		// (what was still owed a raise at the start of them got it)
		const uint32 before = allowance.allowance();

		// Its datagrams are late
		allowance.update(config, true, 150, number - 2, number, 0, 0);
		if (variant == 0)
			EXPECT_EQ(allowance.allowance(), before / 2); // for what is on its way: it did use its allowance then
		else if (variant == 1)
			EXPECT_EQ(allowance.allowance(), before / 4); // towards what it gets, a quarter at once
		else
			EXPECT_EQ(allowance.allowance(), before); // a trickle is not what holds its datagrams up
	}
}

} /* anonymous namespace */

/* end of file */
