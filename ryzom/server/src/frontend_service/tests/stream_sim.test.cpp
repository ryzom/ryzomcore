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
// Known and wanted better, with the bound that holds it where it is:
// - A backlog on a new connection overshoots the path once before the allowance finds its level, which makes the
//   datagrams up to 140 ms late for half a second (BacklogFindsThePath).
// - A client that stops reporting just then (StalledClientDuringStartUp) keeps that allowance for as long as it is
//   silent, with datagrams late all the while.

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"

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
		EXPECT_EQ(r.CongestionEvents, 0u);
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
	EXPECT_EQ(r.AllowanceMin, 4096u);
	EXPECT_GT(r.CongestionEvents, 10u);
	EXPECT_LE(r.maxLateness(), 5u);
	EXPECT_LT(r.maxRecordSeconds(), 1.0);
	EXPECT_LE(r.LongestGap, 2u);
}

// A message of 30 KB every few seconds, with and without jitter on the path: jitter is not lateness
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
	EXPECT_LE(r.AllowanceEnd, 12500u);
	EXPECT_GE(r.AllowanceEnd, 6250u);
}

// The client changes network: the allowance starts over on the new path
TEST(StreamSim, NewAddress)
{
	CRun run("newaddress");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 3u);
	EXPECT_LE(r.maxLateness(), 170u);
	EXPECT_LE(r.lateAbove(50), 16u);
	EXPECT_LT(r.maxRecordSeconds(), 25.0);
}

// A client that sends nothing for 10 s (a loading screen) while its allowance is at its level: the allowance stays
// where it is, and nothing is late for it
TEST(StreamSim, StalledClient)
{
	CRun run("stall");
	CRun reference("backlog");
	const CClientResult &r = run.client();
	expectAllDatagrams(r);
	expectAllRecords(r);
	EXPECT_LE(r.maxLateness(), reference.client().maxLateness());
	EXPECT_LE(r.lateAbove(20), reference.client().lateAbove(20));
	EXPECT_LT(r.maxRecordSeconds(), 40.0);
}

// The same while the allowance is above what the path takes: it cannot be judged without reports and stays there,
// and the congestion window is all that holds the stream data back. (Wanted better; see the top of the file.)
TEST(StreamSim, StalledClientDuringStartUp)
{
	CRun run("stall-startup");
	const CClientResult &r = run.client();
	expectAllRecords(r);
	EXPECT_LE(r.DatagramsLost + r.DatagramsCancelled + r.DatagramsNotSent, 3u);
	EXPECT_LE(r.maxLateness(), 200u);
	EXPECT_LE(r.meanLateness(), 35.0);
	EXPECT_LT(r.maxRecordSeconds(), 20.0);
}

// 300 clients log in one after another: everyone is served, nobody's datagrams are late, the budget holds
TEST(StreamSim, LoginWave)
{
	CRun run("wave");
	EXPECT_EQ(run.Sim->clientsDone(), 300u);
	EXPECT_LE(run.Sim->maxLateness(), 5u);
	EXPECT_LE(run.Sim->maxEgressPerSecond(), (uint64)run.Config.Scheduler.TotalOutgoingBandwidth);
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
// a cycle late. That is the measure: with stream data up to a budget of nine tenths of the link, the datagrams are
// no later than they are without any, and far from that.
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

	CRun streams("uplink-streams-90");
	EXPECT_EQ(streams.Sim->uplinkDrops(), 0u);
	EXPECT_LE(streams.Sim->latenessPercentile(0.5), idle.Sim->latenessPercentile(0.5) + 3);
	EXPECT_LE(streams.Sim->latenessPercentile(0.99), idle.Sim->latenessPercentile(0.99) + 15);
	EXPECT_LE(streams.Sim->latenessPercentile(0.99), udp.Sim->latenessPercentile(0.99));
	EXPECT_LE(streams.Sim->maxEgressPerSecond(), (uint64)(streams.Config.UplinkBytesPerSecond * 1.01));
	EXPECT_GE(streams.Sim->meanEgressPerSecond(), 0.95 * (double)streams.Config.Scheduler.TotalOutgoingBandwidth);
	for (uint32 i = 0; i < streams.Sim->clientCount(); ++i)
		EXPECT_EQ(streams.client(i).DatagramsLost + streams.client(i).DatagramsCancelled + streams.client(i).DatagramsNotSent, 0u) << "client " << i;
}

} /* anonymous namespace */

/* end of file */
