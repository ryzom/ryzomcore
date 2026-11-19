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

// Runs the simulation of the QUIC streams of the frontend service (stream_sim.h) from the command line and prints
// what happened: a line per case, or the cycles of one client.

#include "nel/misc/types_nl.h"
#include "nel/misc/app_context.h"
#include "nel/misc/debug.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "stream_sim.h"

using namespace STREAM_SIM;

namespace /* anonymous */ {

void usage()
{
	printf("ryzom_frontend_streams_sim [case ...] [options]\n"
	       "Simulates the QUIC streams of the frontend service in made-up time. Without a case: all of them.\n"
	       "  --list          the cases\n"
	       "  --trace         the cycles of one client instead of the summary\n"
	       "  --client N      which client (the summary of a case with many clients is of all of them)\n"
	       "  --from S --to S the seconds of the trace\n"
	       "  --seconds S     how long to run\n"
	       "  --seed N        for what is random\n"
	       "Changes to the case (to every client of it):\n"
	       "  --rate M        megabits per second of the path\n"
	       "  --delay MS      delay each way\n"
	       "  --buffer MS     what the bottleneck holds\n"
	       "  --jitter MS     --loss SHARE\n"
	       "  --datagram N    bytes of the frontend's datagram\n"
	       "  --uplink N      bytes per second of the frontend's own link (0: none)\n"
	       "  --budget N      TotalOutgoingBandwidth\n"
	       "  --target MS     QueueDelayTarget\n"
	       "  --max N         StreamMaxBytesPerTick\n"
	       "  --min N         StreamMinBytesPerTick\n"
	       "  --chunk N       StreamMinChunk\n"
	       "  --no-cancel     datagrams wait for the congestion window instead of being dropped\n"
	       "  --no-pacing     the transport sends all its window allows at once\n");
}

struct COptions
{
	COptions()
	    : Trace(false)
	    , Client(-1)
	    , From(0)
	    , To(1e9)
	    , Seconds(0)
	    , Seed(0)
	    , Rate(0)
	    , Delay(-1)
	    , Buffer(-1)
	    , Jitter(-1)
	    , Loss(-1)
	    , Datagram(0)
	    , Uplink(-1)
	    , Budget(0)
	    , Target(-1)
	    , Max(0)
	    , Min(-1)
	    , Chunk(0)
	    , NoCancel(false)
	    , NoPacing(false)
	{
	}

	bool Trace;
	sint32 Client;
	double From, To, Seconds;
	uint32 Seed;
	double Rate, Delay, Buffer, Jitter, Loss;
	uint32 Datagram;
	double Uplink;
	uint32 Budget;
	sint32 Target;
	uint32 Max;
	sint32 Min;
	uint32 Chunk;
	bool NoCancel;
	bool NoPacing;
};

void applyPath(const COptions &options, CPathConfig &path)
{
	const double bufferMs = (double)path.BufferBytes / path.RateBytesPerSecond * 1000.0;
	if (options.Rate > 0)
		path.RateBytesPerSecond = options.Rate * 125000.0;
	if (options.Delay >= 0)
		path.DelayMs = options.Delay;
	if (options.Jitter >= 0)
		path.JitterMs = options.Jitter;
	if (options.Loss >= 0)
		path.LossRate = options.Loss;
	path.BufferBytes = (uint32)(path.RateBytesPerSecond * (options.Buffer >= 0 ? options.Buffer : bufferMs) / 1000.0);
}

void apply(const COptions &options, CSimConfig &config, double &seconds)
{
	for (size_t i = 0; i < config.Clients.size(); ++i)
	{
		applyPath(options, config.Clients[i].Path);
		if (options.Datagram)
			config.Clients[i].DatagramBytes = options.Datagram;
	}
	for (size_t i = 0; i < config.PathChanges.size(); ++i)
	{
		COptions pathOptions(options);
		pathOptions.Rate = 0; // (the change of the case stays what it is)
		pathOptions.Delay = -1;
		applyPath(pathOptions, config.PathChanges[i].Path);
	}
	if (options.Seconds > 0)
		seconds = options.Seconds;
	if (options.Seed)
		config.Seed = options.Seed;
	if (options.Uplink >= 0)
		config.UplinkBytesPerSecond = options.Uplink;
	if (options.Budget)
		config.Scheduler.TotalOutgoingBandwidth = options.Budget;
	if (options.Target >= 0)
		config.Scheduler.QueueDelayTarget = (uint32)options.Target;
	if (options.Max)
		config.Scheduler.StreamMaxBytesPerTick = options.Max;
	if (options.Min >= 0)
		config.Scheduler.StreamMinBytesPerTick = (uint32)options.Min;
	if (options.Chunk)
		config.Scheduler.StreamMinChunk = options.Chunk;
	if (options.NoCancel)
		config.CancelOnBlocked = false;
	if (options.NoPacing)
		config.Pacing = false;
	if (options.Client >= 0 && (size_t)options.Client < config.Clients.size())
		config.TraceClient = (uint32)options.Client;
}

/// A line per case. Datagrams: how late they were in milliseconds over all the clients (mean, all but the worst
/// hundredth, the worst), how many were more than 20 ms late, the most a client reported, how many were lost on the way, dropped by the transport, not sent
/// because the one before waited, and the most in a row that did not arrive. Records: how many arrived, and how long
/// they took in seconds (mean, worst); clients that got all of theirs. Allowance: lowest, highest, highest at the end,
/// times it went down and up. Transport: times a window was cut, packets lost. Frontend: kilobytes per second it sent
/// over the run and in its busiest second.
void printSummaryHeader()
{
	printf("%-18s %4s | %5s %5s %5s %5s %5s %5s %5s %5s %4s | %6s %6s %6s %4s | %6s %6s %6s %5s %5s | %5s %5s | %7s %7s\n",
	    "case", "cl", "late", "p99", "max", ">20ms", "rep", "lost", "drop", "unsnt", "gap",
	    "rec", "mean s", "max s", "done",
	    "al.min", "al.max", "al.end", "cuts", "rais", "cong", "pkt.l", "kB/s", "peak");
}

void printSummary(const std::string &name, const CSim &sim)
{
	const uint32 nb = sim.clientCount();
	double lateSum = 0, recordSum = 0, recordMax = 0;
	uint32 lost = 0, cancelled = 0, notSent = 0, gap = 0, reported = 0, cuts = 0, raises = 0, congestion = 0, packetsLost = 0;
	uint32 allowanceMin = 0xFFFFFFFF, allowanceMax = 0, allowanceEnd = 0;
	uint64 lateCount = 0, records = 0;
	uint32 lateAbove = 0;
	for (uint32 i = 0; i < nb; ++i)
	{
		const CClientResult &r = sim.client(i);
		lateSum += r.meanLateness() * (double)r.Lateness.size();
		lateCount += r.Lateness.size();
		lateAbove += r.lateAbove(20);
		lost += r.DatagramsLost;
		cancelled += r.DatagramsCancelled;
		notSent += r.DatagramsNotSent;
		gap = std::max(gap, r.LongestGap);
		reported = std::max(reported, r.ReportedMax);
		for (size_t k = 0; k < r.RecordSeconds.size(); ++k)
			recordSum += r.RecordSeconds[k];
		recordMax = std::max(recordMax, r.maxRecordSeconds());
		records += r.RecordSeconds.size();
		allowanceMin = std::min(allowanceMin, r.AllowanceMin);
		allowanceMax = std::max(allowanceMax, r.AllowanceMax);
		allowanceEnd = std::max(allowanceEnd, r.AllowanceEnd);
		cuts += r.Cuts;
		raises += r.Raises;
		congestion += r.CongestionEvents;
		packetsLost += r.PacketsLost;
	}
	printf("%-18s %4u | %5.1f %5u %5u %5u %5u %5u %5u %5u %4u | %6u %6.2f %6.2f %4u | %6u %6u %6u %5u %5u | %5u %5u | %7.1f %7.1f\n",
	    name.c_str(), nb,
	    lateCount ? lateSum / (double)lateCount : 0.0, sim.latenessPercentile(0.99), sim.maxLateness(), lateAbove, reported,
	    lost, cancelled, notSent, gap,
	    (uint32)records, records ? recordSum / (double)records : 0.0, recordMax, sim.clientsDone(),
	    allowanceMin, allowanceMax, allowanceEnd, cuts, raises, congestion, packetsLost,
	    sim.meanEgressPerSecond() / 1000.0, (double)sim.maxEgressPerSecond() / 1000.0);
}

void printTrace(const CSim &sim, const COptions &options)
{
	printf("%8s %7s %2s %7s %7s %8s %8s %8s %8s %5s %5s %s\n",
	    "s", "allow", "su", "window", "grant", "pending", "outst", "cwnd", "flight", "rtt", "rep", "datagram");
	const std::vector<CTickTrace> &trace = sim.trace();
	for (size_t i = 0; i < trace.size(); ++i)
	{
		const CTickTrace &t = trace[i];
		if (t.Seconds < options.From || t.Seconds > options.To)
			continue;
		char datagram[32];
		switch (t.Datagram)
		{
		case CTickTrace::DatagramLost:
			strcpy(datagram, "lost");
			break;
		case CTickTrace::DatagramCancelled:
			strcpy(datagram, "cancelled");
			break;
		case CTickTrace::DatagramNotSent:
			strcpy(datagram, "not sent");
			break;
		case CTickTrace::DatagramOnItsWay:
			strcpy(datagram, "-");
			break;
		default:
			sprintf(datagram, "%d", t.Datagram);
			break;
		}
		printf("%8.1f %7u %2s %7u %7u %8u %8u %8u %8u %5u %5u %s\n",
		    t.Seconds, t.Allowance, t.StartUp ? "su" : "", t.Window, t.Grant, t.Pending, t.Outstanding,
		    t.CongestionWindow, t.BytesInFlight, t.RttMs, t.Reported, datagram);
	}
}

} /* anonymous namespace */

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext context;
	NLMISC::createDebug(nullptr);

	COptions options;
	std::vector<std::string> names;
	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];
		const char *value = i + 1 < argc ? argv[i + 1] : nullptr;
		if (arg == "--help" || arg == "-h")
		{
			usage();
			return 0;
		}
		else if (arg == "--list")
		{
			const std::vector<std::string> all = scenarioNames();
			for (size_t k = 0; k < all.size(); ++k)
			{
				CSimConfig config;
				double seconds;
				std::string description;
				makeScenario(all[k], config, seconds, description);
				printf("%-18s %s (%.0f s)\n", all[k].c_str(), description.c_str(), seconds);
			}
			return 0;
		}
		else if (arg == "--trace")
			options.Trace = true;
		else if (arg == "--no-cancel")
			options.NoCancel = true;
		else if (arg == "--no-pacing")
			options.NoPacing = true;
		else if (arg.compare(0, 2, "--") == 0)
		{
			if (!value)
			{
				printf("%s wants a value\n", arg.c_str());
				return 2;
			}
			++i;
			const double number = atof(value);
			if (arg == "--client")
				options.Client = (sint32)number;
			else if (arg == "--from")
				options.From = number;
			else if (arg == "--to")
				options.To = number;
			else if (arg == "--seconds")
				options.Seconds = number;
			else if (arg == "--seed")
				options.Seed = (uint32)number;
			else if (arg == "--rate")
				options.Rate = number;
			else if (arg == "--delay")
				options.Delay = number;
			else if (arg == "--buffer")
				options.Buffer = number;
			else if (arg == "--jitter")
				options.Jitter = number;
			else if (arg == "--loss")
				options.Loss = number;
			else if (arg == "--datagram")
				options.Datagram = (uint32)number;
			else if (arg == "--uplink")
				options.Uplink = number;
			else if (arg == "--budget")
				options.Budget = (uint32)number;
			else if (arg == "--target")
				options.Target = (sint32)number;
			else if (arg == "--max")
				options.Max = (uint32)number;
			else if (arg == "--min")
				options.Min = (sint32)number;
			else if (arg == "--chunk")
				options.Chunk = (uint32)number;
			else
			{
				printf("Unknown option %s\n", arg.c_str());
				usage();
				return 2;
			}
		}
		else
			names.push_back(arg);
	}
	if (names.empty())
		names = scenarioNames();

	if (!options.Trace)
		printSummaryHeader();
	for (size_t k = 0; k < names.size(); ++k)
	{
		CSimConfig config;
		double seconds;
		std::string description;
		if (!makeScenario(names[k], config, seconds, description))
		{
			printf("No case named %s (--list has them)\n", names[k].c_str());
			return 2;
		}
		apply(options, config, seconds);
		CSim sim(config);
		sim.run(seconds);
		if (options.Trace)
		{
			printf("%s: %s, client %u\n", names[k].c_str(), description.c_str(), config.TraceClient);
			printTrace(sim, options);
		}
		else
			printSummary(names[k], sim);
	}
	return 0;
}

/* end of file */
