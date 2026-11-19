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

// The cases of the simulation of the QUIC streams (stream_sim.h), each with what is asked of the frontend in it.
// The numbers are bounds around what the simulation gives today, with room: a test that fails says that a change to
// the scheduler, the allowance or the lateness measure made a case worse, or that the simulation changed. To see a
// case: ryzom_frontend_streams_sim <case>, with --trace for its cycles.
//
// Known and wanted better, each with a test that holds it where it is:
// - A backlog on a new connection overshoots the path once before the allowance finds its level, which makes the
//   datagrams up to 140 ms late for half a second (BacklogFindsThePath).
// - A client that stops reporting while a raise is not judged yet keeps it until the frontend probes it, and again
//   each time it is back from the probes (StalledClientDuringStartUp).

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"

#include <algorithm>
#include <string>
#include <vector>

#include "stream_sim.h"

using namespace STREAM_SIM;

namespace /* anonymous */ {

struct CRun
{
	explicit CRun(const std::string &name)
	{
		std::string description;
		EXPECT_TRUE(makeScenario(name, Config, Seconds, description)) << name;
		Sim = new CSim(Config);
		Sim->run(Seconds);
	}
	~CRun() { delete Sim; }

	const CClientResult &client(uint32 index = 0) const { return Sim->client(index); }

	CSimConfig Config;
	double Seconds;
	CSim *Sim;
};

/// All the frontend's datagrams got to the client (but the one or two on their way at the end)
void expectAllDatagrams(const CClientResult &r)
{
	EXPECT_EQ(r.DatagramsLost, 0u);
	EXPECT_EQ(r.DatagramsCancelled, 0u);
	EXPECT_EQ(r.DatagramsNotSent, 0u);
	EXPECT_EQ(r.LongestGap, 0u);
	EXPECT_GE(r.DatagramsArrived + 2, r.DatagramsWritten);
}

void expectAllRecords(const CClientResult &r)
{
	EXPECT_EQ(r.RecordSeconds.size(), (size_t)r.RecordsPushed);
}

TEST(StreamSim, SameSeedSameRun)
{
	CRun a("trickle"), b("trickle");
	EXPECT_EQ(a.client().Lateness, b.client().Lateness);
	EXPECT_EQ(a.client().RecordSeconds, b.client().RecordSeconds);
	EXPECT_EQ(a.client().DatagramsLost, b.client().DatagramsLost);
	EXPECT_EQ(a.Sim->maxEgressPerSecond(), b.Sim->maxEgressPerSecond());
	EXPECT_GT(a.client().DatagramsLost, 0u); // (the case has losses, so the seed is in it)
}

TEST(StreamSim, ClientWithoutStreamDataIsLeftAlone)
{
	CRun run("idle");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	EXPECT_EQ(r.maxLateness(), 0u);
	EXPECT_EQ(r.ReportedMax, 0u);
	EXPECT_EQ(r.Cuts, 0u);
	EXPECT_EQ(r.Raises, 0u);
	EXPECT_EQ(r.CongestionEvents, 0u);
}

// What a login sends (270 KB) arrives within seconds on any path, and no datagram is late for it
TEST(StreamSim, LoginDoesNotDelayTheDatagrams)
{
	const char *names[] = { "login", "login-5mbit", "login-fast", "login-far" };
	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
	{
		SCOPED_TRACE(names[i]);
		CRun run(names[i]);
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LE(r.maxLateness(), 5u);
		EXPECT_LT(r.maxRecordSeconds(), 4.5);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_EQ(r.CongestionEvents, 0u);
	}
}

// On a path of 1 Mbit/s the allowance passes what the path takes before the login is through, and comes back
TEST(StreamSim, LoginOnASlowPath)
{
	CRun run("login-1mbit");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	expectAllRecords(r);
	EXPECT_LE(r.maxLateness(), 90u);
	EXPECT_LE(r.lateAbove(20), 8u);
	EXPECT_LT(r.maxRecordSeconds(), 5.0);
	EXPECT_LE(r.AllowanceEnd, 12500u); // (what the path takes in a cycle)
}

// A modem: 7 KB per second, of which the datagrams take two. The allowance starts far above that and takes its first
// seconds to come down, with the datagrams late by what the bottleneck holds; from there it moves by eighths, not by
// packets (a packet per cycle is most of the path), and the datagrams are on time while the login goes through at
// what the path has left.
TEST(StreamSim, LoginOnAModem)
{
	CRun run("login-modem");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 3u);
	EXPECT_LT(r.maxRecordSeconds(), 70.0); // (270 KB: 54 s at what the path has left)
	double mean;
	uint32 worst, missing;
	run.Sim->latenessBetween(20.0, 120.0, mean, worst, missing);
	EXPECT_LE(mean, 15.0);
	EXPECT_LE(worst, 150u);
	EXPECT_EQ(missing, 0u);
	// (wanted better: the start)
	run.Sim->latenessBetween(0.0, 20.0, mean, worst, missing);
	EXPECT_LE(mean, 260.0);
}

// A client that says what its connection carries. The frontend does not have to pass what the path takes to find
// it: on a modem the first seconds are as the rest, and a backlog at 2 Mbit/s goes through with no datagram late and
// no cut at all. A client that says more than it has is treated as one that says nothing.
TEST(StreamSim, ClientThatSaysWhatItsConnectionCarries)
{
	{
		SCOPED_TRACE("modem");
		CRun run("login-modem-said");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_EQ(r.CongestionEvents, 0u);
		EXPECT_LE(r.maxLateness(), 150u);
		EXPECT_LE(r.meanLateness(), 10.0);
		EXPECT_LT(r.maxRecordSeconds(), 70.0);
		double mean;
		uint32 worst, missing;
		run.Sim->latenessBetween(0.0, 20.0, mean, worst, missing);
		EXPECT_LE(mean, 25.0);
	}
	{
		SCOPED_TRACE("2 Mbit/s");
		CRun run("backlog-said");
		CRun reference("backlog");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LE(r.maxLateness(), 5u);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_LT(r.maxRecordSeconds(), reference.client().maxRecordSeconds());
	}
	{
		SCOPED_TRACE("2 Mbit/s, says 16");
		CRun run("backlog-said-more");
		CRun reference("backlog");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LE(r.maxLateness(), reference.client().maxLateness() + 20);
		EXPECT_LE(r.lateAbove(20), reference.client().lateAbove(20) + 5);
		EXPECT_LT(r.maxRecordSeconds(), reference.client().maxRecordSeconds() + 3.0);
	}
}

// Megabytes for one client: the allowance finds what the path takes and stays near it
TEST(StreamSim, BacklogFindsThePath)
{
	{
		SCOPED_TRACE("2 Mbit/s");
		CRun run("backlog");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LT(r.maxRecordSeconds(), 40.0); // (6 MB: 24 s at the rate of the path)
		EXPECT_LE(r.maxLateness(), 160u); // the overshoot at the start
		EXPECT_LE(r.lateAbove(20), 30u);
		EXPECT_LE(r.latenessPercentile(0.95), 20u);
		EXPECT_LE(r.Cuts, 8u);
		EXPECT_LE(r.CongestionEvents, 1u);
		EXPECT_LE(r.AllowanceEnd, 25000u);
		EXPECT_GE(r.AllowanceEnd, 12500u);
	}
	{
		SCOPED_TRACE("1 Mbit/s");
		CRun run("backlog-1mbit");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LT(r.maxRecordSeconds(), 40.0); // (3 MB: 24 s at the rate of the path)
		EXPECT_LE(r.maxLateness(), 90u);
		EXPECT_LE(r.latenessPercentile(0.95), 25u);
		EXPECT_EQ(r.CongestionEvents, 0u);
		EXPECT_LE(r.AllowanceEnd, 12500u);
		EXPECT_GE(r.AllowanceEnd, 6250u);
	}
}

// A bottleneck that holds 30 ms of packets drops what a cycle sends at once above that. The datagrams go first and
// stay on time; the stream data gets through at what the queue takes per cycle, which is well under the path.
TEST(StreamSim, ShallowBottleneckKeepsTheDatagrams)
{
	CRun run("backlog-shallow");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	EXPECT_LE(r.maxLateness(), 5u);
	EXPECT_GE(r.StreamBytesArrived, (uint64)2500000);
	EXPECT_GE(r.AllowanceMin, 2048u);
}

// A string now and then on a path that loses a packet in a hundred: the losses are not the streams' doing
TEST(StreamSim, StrayLossesDoNotCutATrickle)
{
	CRun run("trickle");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_EQ(r.Cuts, 0u);
	EXPECT_EQ(r.AllowanceMin, run.Config.Scheduler.StreamMinChunk);
	EXPECT_GT(r.CongestionEvents, 10u);
	EXPECT_LE(r.maxLateness(), 5u);
	EXPECT_LT(r.maxRecordSeconds(), 1.0);
	EXPECT_LE(r.LongestGap, 2u);
}

// A message of 30 KB every few seconds, with and without jitter on the path: jitter under the lateness the frontend
// allows (15 ms against 20) is not lateness
TEST(StreamSim, MessagesNowAndThen)
{
	{
		SCOPED_TRACE("1 Mbit/s");
		CRun run("messages");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_LE(r.maxLateness(), 20u);
		EXPECT_LT(r.maxRecordSeconds(), 1.0);
	}
	{
		SCOPED_TRACE("jitter");
		CRun run("jitter");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_LE(r.maxLateness(), 20u); // (15 ms of it is the jitter)
		EXPECT_LT(r.maxRecordSeconds(), 1.0);
	}
}

// Jitter of twice the lateness the frontend allows. The smallest of three datagrams is above the target one time in
// eight, with or without stream data: the frontend learns that from the reports that come without stream data before
// them, and counts lateness from there. The messages arrive as they do without the jitter, and a backlog gets what the
// path takes, with the streams adding no more than the target to what the jitter does.
TEST(StreamSim, JitterAboveTheTarget)
{
	{
		SCOPED_TRACE("messages");
		CRun run("jitter-wifi");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_LE(r.maxLateness(), 45u); // (40 ms of it is the jitter)
		EXPECT_LT(r.maxRecordSeconds(), 1.0);
		EXPECT_GE(r.NoiseEnd, 20u);
		EXPECT_LE(r.NoiseEnd, 40u);
	}
	{
		SCOPED_TRACE("backlog");
		CRun run("backlog-wifi");
		const CClientResult &r = run.client();
		expectAllDatagrams(r);
		expectAllRecords(r);
		EXPECT_LE(r.maxLateness(), 85u);
		EXPECT_LE(r.Cuts, 6u);
		EXPECT_LT(r.maxRecordSeconds(), 42.0); // (32 s without the jitter)
	}
	{
		// (a path without jitter has no noise, and is judged as before)
		CRun run("backlog");
		EXPECT_LE(run.client().NoiseEnd, 2u);
	}
}

// A path that loses a packet in a hundred, with stream data: the losses are not the doing of the streams, the
// congestion window of the connection answers for them, and the allowance is not cut for them. (It is not raised on a
// report that comes with one either, so a backlog gets less than the path takes.)
TEST(StreamSim, LossesWithStreamData)
{
	{
		SCOPED_TRACE("messages");
		CRun run("messages-lossy");
		const CClientResult &r = run.client();
		expectAllRecords(r);
		EXPECT_EQ(r.Cuts, 0u);
		EXPECT_LE(r.maxLateness(), 10u);
		EXPECT_LE(r.LongestGap, 2u);
		EXPECT_LT(r.maxRecordSeconds(), 1.2);
	}
	{
		SCOPED_TRACE("backlog");
		// (The allowance does come down, without a cut: to what the congestion window of the connection, which the
		// losses keep small, lets through.)
		CRun run("backlog-lossy");
		const CClientResult &r = run.client();
		EXPECT_GE(r.AllowanceMin, 4096u);
		EXPECT_LE(r.maxLateness(), 5u);
		EXPECT_LE(r.LongestGap, 2u);
		EXPECT_GE(r.StreamBytesArrived, (uint64)4500000); // (of 6 MB in a minute)
	}
}

// A client whose clock gains 150 parts in a million on the frontend's: what it measures against its best datagrams of
// the last 40 s is 6 ms too much, all the time. That is noise of the same kind, and the login is as fast as with a
// clock that keeps time.
TEST(StreamSim, ClockThatRunsFast)
{
	CRun run("login-drift");
	CRun reference("login");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	expectAllRecords(r);
	EXPECT_EQ(r.maxLateness(), 0u);
	EXPECT_LE(r.ReportedMax, 8u);
	EXPECT_GE(r.NoiseEnd, 3u);
	EXPECT_EQ(r.Cuts, 0u);
	EXPECT_LE(r.maxRecordSeconds(), reference.client().maxRecordSeconds() + 0.15);
}

// The path lets nothing through for 4 s: the frontend probes the client, the transport probes the path, and both
// are back when the path is. For 15 s: the frontend gives the client up.
TEST(StreamSim, PathThatGoesDark)
{
	{
		SCOPED_TRACE("4 s");
		CRun run("dark");
		const CClientResult &r = run.client();
		expectAllRecords(r);
		EXPECT_FALSE(r.Gone);
		EXPECT_GE(r.Probes, 1u);
		EXPECT_LT(r.maxRecordSeconds(), 45.0);
		double mean;
		uint32 worst, missing;
		run.Sim->latenessBetween(20.0, 60.0, mean, worst, missing);
		EXPECT_EQ(missing, 0u);
		EXPECT_LE(mean, 5.0);
		EXPECT_LE(worst, 110u);
		run.Sim->latenessBetween(0.0, 9.9, mean, worst, missing);
		EXPECT_EQ(missing, 0u);
	}
	{
		SCOPED_TRACE("15 s");
		CRun run("dark-long");
		EXPECT_TRUE(run.client().Gone);
	}
}

// The path goes from 5 to 1 Mbit/s under a full allowance: datagrams are late and lost for a moment, and the
// allowance comes down to what the path takes now
TEST(StreamSim, PathThatDropsInSpeed)
{
	CRun run("pathdrop");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_LE(r.LongestGap, 12u);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 20u);
	EXPECT_LE(r.maxLateness(), 260u); // (the bottleneck holds 250 ms)
	EXPECT_LE(r.lateAbove(50), 60u);
	EXPECT_LE(r.AllowanceEnd, 13500u); // (12500 is what the path takes now)
	EXPECT_GE(r.AllowanceEnd, 6250u);
}

// The client changes network: nothing goes to the new address until it is validated (datagrams are dropped for it),
// and the allowance starts over on the new path
TEST(StreamSim, NewAddress)
{
	CRun run("newaddress");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_GE(r.DatagramsCancelled, 1u);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 4u);
	EXPECT_LE(r.maxLateness(), 170u);
	EXPECT_LE(r.lateAbove(50), 16u);
	EXPECT_LT(r.maxRecordSeconds(), 25.0);
	// (after the change: back near what the path takes, from the start)
	EXPECT_GE(r.AllowanceEnd, 12500u);
	EXPECT_LE(r.AllowanceEnd, 25000u);
}

// A client that sends nothing for 10 s (a loading screen) while its allowance is at its level. The frontend takes it
// for lagging after 3 s and probes it, and stream data waits while it does. Nothing is late or missing for it.
TEST(StreamSim, StalledClient)
{
	CRun run("stall");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	expectAllRecords(r);
	EXPECT_GE(r.Probes, 1u);
	EXPECT_FALSE(r.Gone);
	double mean;
	uint32 worst, missing;
	run.Sim->latenessBetween(14.0, 27.0, mean, worst, missing);
	EXPECT_LE(worst, 20u);
	EXPECT_EQ(missing, 0u);
	EXPECT_LT(r.maxRecordSeconds(), 45.0);
}

// The same while the allowance is above what the path takes: it cannot be judged without reports and stays there,
// for the 3 s until the frontend probes the client and again each time the client is back from the probes. The
// congestion window is all that holds the stream data back then. (Wanted better; see the top of the file.)
TEST(StreamSim, StalledClientDuringStartUp)
{
	CRun run("stall-startup");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_GE(r.Probes, 1u);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 3u);
	EXPECT_LE(r.maxLateness(), 200u);
	EXPECT_LE(r.meanLateness(), 20.0);
	EXPECT_LT(r.maxRecordSeconds(), 25.0);
}

// 300 clients log in one after another, well within the budget: everyone is served, nobody's datagrams are late
TEST(StreamSim, LoginWave)
{
	CRun run("wave");
	EXPECT_EQ(run.Sim->clientsDone(), 300u);
	EXPECT_LE(run.Sim->maxLateness(), 5u);
	double worst = 0;
	for (uint32 i = 0; i < run.Sim->clientCount(); ++i)
	{
		const CClientResult &r = run.client(i);
		EXPECT_EQ(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 0u) << "client " << i;
		worst = std::max(worst, r.maxRecordSeconds());
	}
	EXPECT_LT(worst, 2.5);
}

// The same with a budget that the datagrams alone pass: every client still gets its minimum each cycle, and the
// datagrams are not held back for it
TEST(StreamSim, LoginWaveAboveTheBudget)
{
	CRun run("wave-tight");
	EXPECT_EQ(run.Sim->clientsDone(), 300u);
	EXPECT_LE(run.Sim->maxLateness(), 5u);
	EXPECT_GT(run.Sim->scheduler().overshootDatagrams(), (uint64)0);
	for (uint32 i = 0; i < run.Sim->clientCount(); ++i)
	{
		const CClientResult &r = run.client(i);
		EXPECT_EQ(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 0u) << "client " << i;
		EXPECT_EQ(r.Cuts, 0u) << "client " << i;
	}
}

// The frontend's own link. It sends the datagrams of all its clients at once and its link spreads them over the
// cycle by their place in that order: a frontend without streams whose datagrams fill its link has the last of them
// a cycle late. That is the measure. With stream data up to a budget of all that the link carries, the datagrams stay
// under it, and near where they are without any stream data: the frontend sends no more in a cycle than the link
// takes in one, whatever it did not send before.
TEST(StreamSim, OwnLinkOfTheFrontend)
{
	CRun udp("uplink-udp");
	EXPECT_GE(udp.Sim->maxLateness(), 95u);
	EXPECT_LE(udp.Sim->maxLateness(), 100u);
	EXPECT_EQ(udp.client(0).maxLateness(), 0u);
	// (a place in the order is always as late: the clients do not report it)
	EXPECT_EQ(udp.client(udp.Sim->clientCount() - 1).ReportedMax, 0u);

	CRun idle("uplink-idle");
	EXPECT_LT(idle.Sim->maxLateness(), 50u);

	const char *names[] = { "uplink-streams-90", "uplink-streams", "uplink-wave" };
	for (size_t n = 0; n < sizeof(names) / sizeof(names[0]); ++n)
	{
		SCOPED_TRACE(names[n]);
		const bool full = n != 0;
		CRun streams(names[n]);
		EXPECT_EQ(streams.Sim->uplinkDrops(), 0u);
		EXPECT_LE(streams.Sim->maxLateness(), idle.Sim->maxLateness() + (full ? 20u : 5u));
		EXPECT_LT(streams.Sim->maxLateness(), udp.Sim->latenessPercentile(0.75));
		EXPECT_LE(streams.Sim->latenessPercentile(0.5), idle.Sim->latenessPercentile(0.5) + (full ? 12u : 3u));
		EXPECT_LE(streams.Sim->maxEgressPerSecond(), (uint64)(streams.Config.UplinkBytesPerSecond * 1.03));
		uint32 cuts = 0;
		for (uint32 i = 0; i < streams.Sim->clientCount(); ++i)
		{
			const CClientResult &r = streams.client(i);
			EXPECT_EQ(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 0u) << "client " << i;
			EXPECT_EQ(r.CongestionEvents, 0u) << "client " << i;
			cuts += r.Cuts;
		}
		// (no client is held back for what waits at the frontend's link, which is none of its doing)
		EXPECT_EQ(cuts, 0u);
		if (n != 2)
			EXPECT_GE(streams.Sim->meanEgressPerSecond(), 0.95 * (double)streams.Config.Scheduler.TotalOutgoingBandwidth);
	}
}

// The frontend's link carries less than its budget says, which nothing tells it but the datagrams of all its clients
// arriving late at once. It then holds itself to less, and the datagrams are as they are on a frontend without
// streams behind the same link: that is the measure, case by case.
TEST(StreamSim, LinkThatCarriesLessThanTheBudget)
{
	double mean, udpMean;
	uint32 worst, missing, udpWorst, udpMissing;
	{
		SCOPED_TRACE("a fifth less, all the time");
		CRun run("uplink-short");
		CRun idle("uplink-idle");
		run.Sim->latenessBetween(15.0, 60.0, mean, worst, missing);
		EXPECT_LE(mean, idle.Sim->latenessPercentile(0.5) * 1.25 + 8.0); // (the link is slower by a quarter)
		EXPECT_LE(worst, 100u);
		EXPECT_EQ(missing, 0u);
		EXPECT_LT(run.Sim->budgetMin(), 2000000u);
		// (and the streams still get what the link has left: more than half of it)
		EXPECT_GE(run.Sim->meanEgressPerSecond(), 1700000.0);
	}
	{
		SCOPED_TRACE("down to 1.5 MB/s for 20 s");
		CRun run("uplink-dip");
		CRun udp("uplink-dip-udp");
		// Once it has found the level, a few seconds in
		run.Sim->latenessBetween(28.0, 40.0, mean, worst, missing);
		udp.Sim->latenessBetween(28.0, 40.0, udpMean, udpWorst, udpMissing);
		EXPECT_LE(mean, udpMean + 3.0);
		EXPECT_LE(worst, udpWorst + 10);
		EXPECT_EQ(missing, 0u);
		// On the way there
		run.Sim->latenessBetween(20.0, 28.0, mean, worst, missing);
		EXPECT_LE(mean, 90.0);
		EXPECT_LE(missing, 1500u); // (of 24000)
		// And after it
		run.Sim->latenessBetween(42.0, 60.0, mean, worst, missing);
		udp.Sim->latenessBetween(42.0, 60.0, udpMean, udpWorst, udpMissing);
		EXPECT_LE(mean, udpMean + 3.0);
		EXPECT_LE(worst, udpWorst + 10);
		EXPECT_EQ(missing, 0u);
		EXPECT_LT(run.Sim->budgetMin(), 1500000u);
		EXPECT_GT(run.Sim->budgetEnd(), run.Sim->budgetMin() + 300000u);
		EXPECT_LE(run.Sim->scheduler().budgetCuts(), (uint64)12);
	}
	{
		SCOPED_TRACE("less than the datagrams alone take");
		CRun run("uplink-starved");
		CRun udp("uplink-starved-udp");
		run.Sim->latenessBetween(28.0, 40.0, mean, worst, missing);
		udp.Sim->latenessBetween(28.0, 40.0, udpMean, udpWorst, udpMissing);
		EXPECT_LE(mean, udpMean + 10.0);
		EXPECT_LE(missing, udpMissing + udpMissing / 10 + 50);
		run.Sim->latenessBetween(45.0, 60.0, mean, worst, missing);
		udp.Sim->latenessBetween(45.0, 60.0, udpMean, udpWorst, udpMissing);
		EXPECT_LE(mean, udpMean + 3.0);
		EXPECT_LE(missing, udpMissing + 20);
	}
	{
		SCOPED_TRACE("a link that carries the budget");
		const char *names[] = { "uplink-streams", "uplink-streams-90", "uplink-wave", "wave", "wave-tight" };
		for (size_t n = 0; n < sizeof(names) / sizeof(names[0]); ++n)
		{
			CRun run(names[n]);
			EXPECT_EQ(run.Sim->scheduler().budgetCuts(), (uint64)0) << names[n];
		}
	}
}

} /* anonymous namespace */

/* end of file */
