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

#ifndef NL_STREAM_SIM_H
#define NL_STREAM_SIM_H

#include "nel/misc/types_nl.h"

#include <deque>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "game_share/stream_record.h"
#include "game_share/datagram_lateness.h"

#include "stream_queues.h"
#include "stream_scheduler.h"

/*
 * A simulation of the QUIC streams of the frontend service, in made-up time: a frontend with clients, each behind a
 * path of its own, for as many seconds as asked, in a fraction of a second of real time and the same every time.
 *
 * What is real in it: the classes that decide what each client gets (CStreamScheduler, CStreamAllowance,
 * streamWindowRoom, CStreamQueues: the code the service runs), the client's measure of how late the datagrams arrive
 * (CDatagramLateness) and the byte it reports.
 *
 * What is a model:
 * - The frontend around them: a cycle every 100 ms that reads the reports of the clients, writes a datagram per client,
 *   asks the classes above, and sends the datagrams and then the stream data, as CFeSendSub does.
 * - The transport, in place of MsQuic 2.6 with its default settings: packets of at most an MTU with the datagram
 *   first and stream data behind it; a congestion window (slow start, a cut to 0.7 on a loss, growth of a packet per
 *   window after it, never above twice what was in flight); a packet written whole as long as the window has any
 *   room; pacing by its rule (what a flush may send grows with the time since the last one, by twice the window per
 *   round trip in slow start and 1.25 times after it, and what is left goes on a millisecond later); a loss found
 *   three packets later or after a time without acknowledgement; stream data sent again; a datagram that was sent so
 *   dropped at the end of a flush that did not write it; one datagram waiting at a time. Its window grows as Reno's,
 *   not as CUBIC's; no ECN, no probes, an acknowledgement per packet without delay.
 * - The frontend's own link, if one is set: all its packets leave through it in the order they were sent, at its rate.
 * - The path to each client: a bottleneck of a given rate with a queue of a given size that drops what does not fit,
 *   a delay each way, jitter, random loss. The way back is not congested.
 * - The client: it acknowledges what arrives, measures the lateness of the datagrams, and sends a datagram of its own
 *   with its report every 100 ms, except while its main thread is busy (a loading screen).
 *
 * The transport is a model of MsQuic, not MsQuic: it makes changes to the classes comparable and shows faults in
 * their logic, and its numbers stand for a real path only as far as it was compared with one.
 */

namespace STREAM_SIM {

/// Time, in microseconds from the start
typedef sint64 TTime;

/// The path from the frontend to one client
struct CPathConfig
{
	CPathConfig();

	/// What the bottleneck lets through, in bytes per second
	double RateBytesPerSecond;
	/// Delay each way, in milliseconds
	double DelayMs;
	/// Up to that many milliseconds more for each packet to the client (the order of the packets is kept)
	double JitterMs;
	/// Bytes that can wait at the bottleneck; a packet that does not fit is dropped
	uint32 BufferBytes;
	/// Share of the packets to the client that are lost for no reason
	double LossRate;
};

struct CClientConfig
{
	CClientConfig();

	CPathConfig Path;
	/// Bytes of the datagram the frontend sends it each cycle
	uint32 DatagramBytes;
	/// When the client sends its own datagrams: every so many milliseconds, from this many after the start
	double SendPeriodMs;
	double SendPhaseMs;
	/// Stretches of seconds, from and to, in which the client sends none (its transport still acknowledges)
	std::vector<std::pair<double, double> > Stalls;
};

/// Records for a client: count records of so many bytes, the first at a time, the next ones every so many seconds
/// (all at once if that is 0)
struct CTraffic
{
	CTraffic(uint32 client, double atSeconds, uint32 bytes, uint32 count = 1, double everySeconds = 0)
	    : Client(client)
	    , AtSeconds(atSeconds)
	    , Bytes(bytes)
	    , Count(count)
	    , EverySeconds(everySeconds)
	{
	}

	uint32 Client;
	double AtSeconds;
	uint32 Bytes;
	uint32 Count;
	double EverySeconds;
};

/// The path to a client becomes another one at a time. newAddress: the client is at another address from then on
/// (it changed network), which the transport tells the frontend.
struct CPathChange
{
	CPathChange(uint32 client, double atSeconds, const CPathConfig &path, bool newAddress)
	    : Client(client)
	    , AtSeconds(atSeconds)
	    , Path(path)
	    , NewAddress(newAddress)
	{
	}

	uint32 Client;
	double AtSeconds;
	CPathConfig Path;
	bool NewAddress;
};

/// The frontend's own link carries another amount from a time on (congestion in the data centre, say), which
/// nothing tells the frontend
struct CUplinkChange
{
	CUplinkChange(double atSeconds, double bytesPerSecond)
	    : AtSeconds(atSeconds)
	    , BytesPerSecond(bytesPerSecond)
	{
	}

	double AtSeconds;
	double BytesPerSecond;
};

struct CSimConfig
{
	CSimConfig();

	CStreamSchedulerConfig Scheduler;
	/// For what is random (jitter, loss): the same seed gives the same run
	uint32 Seed;
	/// The frontend's datagram is dropped when it cannot be written at once
	bool CancelOnBlocked;
	/// The transport paces what it sends
	bool Pacing;
	/// The frontend's own link, which all its packets leave through in the order they were sent before they go their
	/// own ways: what it carries in bytes per second (0: any amount, at once), and how many milliseconds of packets
	/// can wait at it before it drops them
	double UplinkBytesPerSecond;
	double UplinkBufferMs;
	/// The client whose cycles are kept in trace()
	uint32 TraceClient;
	std::vector<CClientConfig> Clients;
	std::vector<CTraffic> Traffic;
	std::vector<CPathChange> PathChanges;
	std::vector<CUplinkChange> UplinkChanges;
};

/// What happened to one client
struct CClientResult
{
	CClientResult();

	/// The frontend's datagrams: written by it, of which arrived, lost on the way, dropped by the transport because
	/// they could not be written at once, not sent because the one before still waited
	uint32 DatagramsWritten;
	uint32 DatagramsArrived;
	uint32 DatagramsLost;
	uint32 DatagramsCancelled;
	uint32 DatagramsNotSent;
	/// The most cycles in a row whose datagram did not arrive
	uint32 LongestGap;
	/// How late each datagram that arrived was, in milliseconds: its time on the way above what the empty path takes
	std::vector<uint32> Lateness;
	/// When the frontend wrote each of those, in seconds
	std::vector<float> LatenessAt;
	/// When it wrote each datagram that did not arrive (lost, dropped or not sent)
	std::vector<float> MissingAt;
	/// The most the client reported to the frontend, in milliseconds (it measures against its best datagrams of the
	/// last tens of seconds, so a lateness that never changes is not in it)
	uint32 ReportedMax;
	/// How long each record took from when the service gave it to the frontend to when the client had all of it, in
	/// seconds (only the records that arrived)
	std::vector<double> RecordSeconds;
	uint32 RecordsPushed;
	uint64 StreamBytesArrived;
	/// The allowance: its lowest and highest value, its last, how often it went down and up
	uint32 AllowanceMin;
	uint32 AllowanceMax;
	uint32 AllowanceEnd;
	uint32 Cuts;
	uint32 Raises;
	/// Times the transport cut its congestion window, and packets it lost
	uint32 CongestionEvents;
	uint32 PacketsLost;

	uint32 maxLateness() const;
	double meanLateness() const;
	/// Datagrams that arrived more than so many milliseconds late
	uint32 lateAbove(uint32 milliseconds) const;
	/// The lateness that this share of the datagrams stayed at or under (0.99: all but the worst hundredth)
	uint32 latenessPercentile(double share) const;
	double maxRecordSeconds() const;
};

/// One cycle of the traced client
struct CTickTrace
{
	double Seconds;
	uint32 Allowance;
	bool StartUp;
	uint32 Window;
	uint32 Grant;
	uint32 Pending;
	uint32 Outstanding;
	uint32 CongestionWindow;
	uint32 BytesInFlight;
	uint32 RttMs;
	/// What the client last reported, in milliseconds
	uint32 Reported;
	/// What became of the datagram of the cycle: its lateness in milliseconds, or one of the values below
	sint32 Datagram;
	enum
	{
		DatagramLost = -1,
		DatagramCancelled = -2,
		DatagramNotSent = -3,
		DatagramOnItsWay = -4
	};
};

class CSim
{
public:
	explicit CSim(const CSimConfig &config);
	~CSim();

	/// Go on for so many seconds
	void run(double seconds);

	uint32 clientCount() const { return (uint32)m_Clients.size(); }
	const CClientResult &client(uint32 index) const;
	const std::vector<CTickTrace> &trace() const { return m_Trace; }

	/// \name The frontend as a whole
	// @{
	/// The most bytes it sent in any one second, and what it sent per second over the whole run
	uint64 maxEgressPerSecond() const { return m_MaxEgressPerSecond; }
	double meanEgressPerSecond() const;
	/// Clients whose records have all arrived
	uint32 clientsDone() const;
	/// Packets its own link dropped
	uint32 uplinkDrops() const { return m_UplinkDrops; }
	/// Over all the clients: the most a datagram was late, and what this share of the datagrams stayed at or under
	uint32 maxLateness() const;
	uint32 latenessPercentile(double share) const;
	/// The same of the datagrams the frontend wrote in a stretch of seconds, with how many of them did not arrive
	/// (the lateness of each is counted from what the path takes when all of it is empty at the rates of the start)
	void latenessBetween(double fromSeconds, double toSeconds, double &mean, uint32 &worst, uint32 &missing) const;
	const CStreamScheduler &scheduler() const { return m_Scheduler; }
	// @}

private:
	struct CClient;
	struct CEvent
	{
		TTime Time;
		uint64 Sequence;
		uint32 Type;
		uint32 Client;
		uint64 A;
		uint64 B;
		bool operator<(const CEvent &other) const
		{
			// (the earliest first out of a queue that gives the greatest)
			return Time != other.Time ? Time > other.Time : Sequence > other.Sequence;
		}
	};
	enum
	{
		EventTick,
		EventPacketArrives,
		EventAckArrives,
		EventClientSends,
		EventReportArrives,
		EventTraffic,
		EventPathChange,
		EventPacing,
		EventUplinkChange
	};

	void post(TTime time, uint32 type, uint32 client, uint64 a = 0, uint64 b = 0);
	double random();

	void tick();
	void flush(CClient &client);
	uint32 sendPacket(CClient &client, bool withDatagram, uint64 streamOffset, uint32 streamLength);
	void packetArrives(CClient &client, uint64 number);
	void ackArrives(CClient &client, uint64 number);
	void packetLost(CClient &client, uint64 number);
	void findLosses(CClient &client, uint64 ackedNumber);
	void clientSends(CClient &client);
	void snapshot(CClient &client);
	void datagramFate(CClient &client, uint32 datagramNumber, sint32 fate);

	CSimConfig m_Config;
	std::vector<CClient *> m_Clients;
	std::priority_queue<CEvent> m_Events;
	uint64 m_Sequence;
	TTime m_Now;
	uint32 m_RandomState;

	CStreamScheduler m_Scheduler;
	std::vector<CStreamScheduler::CEntry> m_Entries;
	std::vector<uint8> m_Scratch;
	TTime m_LastTick;
	bool m_Ticked;
	uint64 m_EgressSinceTick;
	uint64 m_EgressTotal;
	uint64 m_EgressThisSecond;
	TTime m_EgressSecondStart;
	uint64 m_MaxEgressPerSecond;
	uint32 m_TickCount;
	double m_UplinkRate;
	TTime m_UplinkLastDeparture;
	uint32 m_UplinkDrops;

	std::vector<CTickTrace> m_Trace;
	uint32 m_TraceFirstNumber;
};

/// The named cases, for the tests and for the program that prints them. False if there is none of that name.
bool makeScenario(const std::string &name, CSimConfig &config, double &seconds, std::string &description);
std::vector<std::string> scenarioNames();

} // STREAM_SIM

#endif // NL_STREAM_SIM_H

/* end of file */
