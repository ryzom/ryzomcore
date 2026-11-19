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

#include "stream_sim.h"

#include <algorithm>

namespace STREAM_SIM {

namespace /* anonymous */ {

/// Bytes of a packet at most, and what a packet has besides its datagram and stream data (header, packet number, frame
/// headers, tag)
const uint32 s_Mtu = 1200;
const uint32 s_PacketOverhead = 28;
/// The headers of UDP and IP, as the frontend counts them for what it sends (IPv6)
const uint32 s_WireOverhead = 48;
/// What the frontend counts for a datagram on the wire besides its bytes (CFeSendSub)
const uint32 s_QuicDatagramOverhead = s_PacketOverhead + s_WireOverhead;
/// The frontend takes a client that sent nothing for so long for lagging, probes it (a small datagram every third
/// cycle), has it back after so long if it answers, and gives it up after so long without anything from it
const TTime s_ClientLagTime = 3000000;
const TTime s_ProbeTime = 2000000;
const TTime s_ClientTimeOut = 10000000;
const uint32 s_ProbeBytes = 12;
/// The client's transport holds the acknowledgement of a lone packet back for so long
const TTime s_AckDelay = 25000;
/// The congestion window at the start, and the least it is cut to
const double s_InitialWindow = 10.0 * s_Mtu;
const double s_MinWindow = 2.0 * s_Mtu;
/// A loss is found when a packet sent this many later is acknowledged
const uint64 s_LossPackets = 3;

TTime us(double milliseconds)
{
	return (TTime)(milliseconds * 1000.0);
}

/// Add a range to those above a point up to which everything is there, and move the point on as far as they reach
void addRange(std::map<uint64, uint32> &ranges, uint64 &complete, uint64 offset, uint32 length)
{
	if (offset + length <= complete)
		return;
	uint32 &known = ranges[offset];
	known = std::max(known, length);
	for (;;)
	{
		std::map<uint64, uint32>::iterator it = ranges.begin();
		if (it == ranges.end() || it->first > complete)
			break;
		complete = std::max(complete, it->first + it->second);
		ranges.erase(it);
	}
}

} /* anonymous namespace */

CPathConfig::CPathConfig()
    : RateBytesPerSecond(250000)
    , DelayMs(20)
    , JitterMs(0)
    , BufferBytes(62500)
    , LossRate(0)
{
}

CClientConfig::CClientConfig()
    : DatagramBytes(300)
    , SendPeriodMs(100)
    , SendPhaseMs(37)
    , ClockDriftPpm(0)
{
}

CSimConfig::CSimConfig()
    : Seed(1)
    , CancelOnBlocked(true)
    , Pacing(true)
    , UplinkBytesPerSecond(0)
    , UplinkBufferMs(200)
    , TraceClient(0)
{
}

CClientResult::CClientResult()
    : DatagramsWritten(0)
    , DatagramsArrived(0)
    , DatagramsLost(0)
    , DatagramsCancelled(0)
    , DatagramsNotSent(0)
    , LongestGap(0)
    , ReportedMax(0)
    , RecordsPushed(0)
    , StreamBytesArrived(0)
    , AllowanceMin(0xFFFFFFFF)
    , AllowanceMax(0)
    , AllowanceEnd(0)
    , Cuts(0)
    , Raises(0)
    , CongestionEvents(0)
    , PacketsLost(0)
    , NoiseEnd(0)
    , Probes(0)
    , Gone(false)
{
}

uint32 CClientResult::maxLateness() const
{
	uint32 most = 0;
	for (size_t i = 0; i < Lateness.size(); ++i)
		most = std::max(most, Lateness[i]);
	return most;
}

double CClientResult::meanLateness() const
{
	double sum = 0;
	for (size_t i = 0; i < Lateness.size(); ++i)
		sum += (double)Lateness[i];
	return Lateness.empty() ? 0.0 : sum / (double)Lateness.size();
}

uint32 CClientResult::lateAbove(uint32 milliseconds) const
{
	uint32 count = 0;
	for (size_t i = 0; i < Lateness.size(); ++i)
	{
		if (Lateness[i] > milliseconds)
			++count;
	}
	return count;
}

uint32 CClientResult::latenessPercentile(double share) const
{
	if (Lateness.empty())
		return 0;
	std::vector<uint32> sorted(Lateness);
	std::sort(sorted.begin(), sorted.end());
	size_t index = (size_t)(share * (double)(sorted.size() - 1) + 0.5);
	return sorted[std::min(index, sorted.size() - 1)];
}

double CClientResult::maxRecordSeconds() const
{
	double most = 0;
	for (size_t i = 0; i < RecordSeconds.size(); ++i)
		most = std::max(most, RecordSeconds[i]);
	return most;
}

struct CSim::CClient
{
	struct CPacket
	{
		uint64 Number;
		uint32 Size;
		TTime SendTime;
		bool HasDatagram;
		uint32 DatagramNumber;
		uint16 DatagramSendTime;
		bool DatagramPathChanged;
		TTime DatagramPostTime;
		uint64 StreamOffset;
		uint32 StreamLength;
		TTime Unloaded; // what its datagram takes on the way when nothing else is on it
		bool WillArrive; // (it was not dropped on the way)
		bool Arrived;
		bool Done; // acknowledged, or taken for lost
		bool Lost;
	};

	uint32 Index;
	CClientConfig Config;
	CPathConfig Path;

	// The bottleneck: when each packet in it leaves, and its size
	std::deque<std::pair<TTime, uint32> > Queue;
	uint64 QueuedBytes;
	TTime LastDeparture;
	TTime LastArrival;

	// The transport of the frontend to this client
	std::deque<CPacket> Sent; // (Sent[i].Number == SentBase + i)
	uint64 SentBase;
	uint64 NextNumber;
	uint64 LargestAcked;
	bool AnyAcked;
	double Cwnd;
	double Ssthresh;
	uint64 BytesInFlight;
	uint64 BytesInFlightMax;
	TTime Srtt;
	bool HasRtt;
	uint64 RecoveryStart;
	TTime LastProgress;
	TTime ProbeTimeout;
	TTime LastFlush;
	bool FlushValid;
	sint64 LastSendAllowance;
	bool PacingTimer;
	bool DatagramPending;
	uint32 PendingNumber;
	uint32 PendingBytes;
	bool PendingCancel;
	uint16 PendingSendTime;
	bool PendingPathChanged;
	TTime PendingPostTime;
	uint64 StreamPosted;
	uint64 StreamNextSend;
	std::deque<std::pair<uint64, uint32> > Retransmit;
	uint64 StreamAcked;
	std::map<uint64, uint32> AckedRanges;
	uint64 StreamArrived;
	std::map<uint64, uint32> ArrivedRanges;

	// What the frontend keeps of the connection (CQuicUserContext)
	CStreamQueues Queues;
	CStreamAllowance Allowance;
	CStreamSchedulerClient Turn;
	uint32 SendNumber;
	bool StatValid;
	sint64 StatCwnd;
	sint64 StatRtt;
	sint64 StatBytesInFlight;
	uint32 StatGrantedCount;
	uint32 GrantedCount;
	uint32 QueueDelay;
	uint32 QueueDelayAck;
	uint32 Reports;
	uint32 ReportsSeen;
	uint32 SendBusyDrops;
	uint32 DatagramCancels;
	uint32 CongestionEvents;
	bool PathChanged;
	uint32 PathChangedDatagrams;
	uint32 LastAllowance;
	uint32 StatCongestionEvents;
	// The blocks of stream data it gave the transport that are not all acknowledged: where each ends, and its size
	std::deque<std::pair<uint64, uint32> > Blocks;
	uint64 Outstanding;
	TTime LastReceive;
	bool Probe;
	TTime ProbeUntil;
	uint32 ProbeCycle;
	bool Gone;
	bool WritesDatagram;
	uint32 CycleDatagramBytes;
	// The transport: when the path of a new address is validated
	TTime ValidatedAt;
	// The client's transport: the packet whose acknowledgement it holds back
	bool AckWaiting;
	uint64 AckWaitingNumber;
	TTime AckWaitingSince;

	// The client
	CDatagramLateness Lateness;
	uint32 LastReceivedNumber;
	bool AnyReceived;

	// The records the services gave: where each ends in the stream, and when it was given
	std::deque<std::pair<uint64, TTime> > Records;
	uint64 StreamPushed;

	CClientResult Result;

	CPacket &packet(uint64 number) { return Sent[(size_t)(number - SentBase)]; }
	bool has(uint64 number) const { return number >= SentBase && number - SentBase < Sent.size(); }
};

CSim::CSim(const CSimConfig &config)
    : m_Config(config)
    , m_Sequence(0)
    , m_Now(0)
    , m_RandomState(config.Seed ? config.Seed : 1)
    , m_LastTick(0)
    , m_Ticked(false)
    , m_EgressSinceTick(0)
    , m_EgressTotal(0)
    , m_EgressThisSecond(0)
    , m_EgressSecondStart(0)
    , m_MaxEgressPerSecond(0)
    , m_UplinkRate(config.UplinkBytesPerSecond)
    , m_BudgetMin(0xFFFFFFFF)
    , m_UplinkLastDeparture(0)
    , m_UplinkDrops(0)
{
	for (uint32 i = 0; i < (uint32)m_Config.Clients.size(); ++i)
	{
		CClient *c = new CClient();
		c->Index = i;
		c->Config = m_Config.Clients[i];
		c->Path = c->Config.Path;
		c->QueuedBytes = 0;
		c->LastDeparture = 0;
		c->LastArrival = 0;
		c->SentBase = 0;
		c->NextNumber = 0;
		c->LargestAcked = 0;
		c->AnyAcked = false;
		c->Cwnd = s_InitialWindow;
		c->Ssthresh = 1e18;
		c->BytesInFlight = 0;
		c->BytesInFlightMax = (uint64)(s_InitialWindow / 2);
		c->Srtt = 0;
		c->HasRtt = false;
		c->RecoveryStart = 0;
		c->LastProgress = 0;
		c->ProbeTimeout = 500000;
		c->LastFlush = 0;
		c->FlushValid = false;
		c->LastSendAllowance = 0;
		c->PacingTimer = false;
		c->DatagramPending = false;
		c->PendingNumber = 0;
		c->PendingBytes = 0;
		c->PendingCancel = false;
		c->PendingSendTime = 0;
		c->PendingPathChanged = false;
		c->PendingPostTime = 0;
		c->StreamPosted = 0;
		c->StreamNextSend = 0;
		c->StreamAcked = 0;
		c->StreamArrived = 0;
		c->SendNumber = 0;
		c->StatValid = false;
		c->StatCwnd = 0;
		c->StatRtt = 0;
		c->StatBytesInFlight = -1;
		c->StatGrantedCount = 0;
		c->GrantedCount = 0;
		c->QueueDelay = 0;
		c->QueueDelayAck = 0;
		c->Reports = 0;
		c->ReportsSeen = 0;
		c->SendBusyDrops = 0;
		c->DatagramCancels = 0;
		c->CongestionEvents = 0;
		c->PathChanged = false;
		c->PathChangedDatagrams = 0;
		c->LastAllowance = 0;
		c->StatCongestionEvents = 0;
		c->Outstanding = 0;
		c->LastReceive = 0;
		c->Probe = false;
		c->ProbeUntil = 0;
		c->ProbeCycle = 0;
		c->Gone = false;
		c->WritesDatagram = false;
		c->CycleDatagramBytes = 0;
		c->ValidatedAt = 0;
		c->AckWaiting = false;
		c->AckWaitingNumber = 0;
		c->AckWaitingSince = 0;
		c->LastReceivedNumber = 0;
		c->AnyReceived = false;
		c->StreamPushed = 0;
		c->Allowance.reset(m_Config.Scheduler);
		m_Clients.push_back(c);
		post(us(c->Config.SendPhaseMs), EventClientSends, i);
	}
	for (size_t i = 0; i < m_Config.Traffic.size(); ++i)
	{
		const CTraffic &traffic = m_Config.Traffic[i];
		if (traffic.Client < m_Clients.size() && traffic.Count != 0 && traffic.Bytes != 0)
			post((TTime)(traffic.AtSeconds * 1e6), EventTraffic, traffic.Client, i, traffic.Count);
	}
	for (size_t i = 0; i < m_Config.PathChanges.size(); ++i)
	{
		const CPathChange &change = m_Config.PathChanges[i];
		if (change.Client < m_Clients.size())
			post((TTime)(change.AtSeconds * 1e6), EventPathChange, change.Client, i);
	}
	for (size_t i = 0; i < m_Config.UplinkChanges.size(); ++i)
		post((TTime)(m_Config.UplinkChanges[i].AtSeconds * 1e6), EventUplinkChange, 0, i);
	post(0, EventTick, 0);
}

CSim::~CSim()
{
	for (size_t i = 0; i < m_Clients.size(); ++i)
		delete m_Clients[i];
}

const CClientResult &CSim::client(uint32 index) const
{
	return m_Clients[index]->Result;
}

double CSim::meanEgressPerSecond() const
{
	return m_Now > 0 ? (double)m_EgressTotal / ((double)m_Now / 1e6) : 0.0;
}

uint32 CSim::clientsDone() const
{
	uint32 done = 0;
	for (size_t i = 0; i < m_Clients.size(); ++i)
	{
		if (m_Clients[i]->Records.empty())
			++done;
	}
	return done;
}

uint32 CSim::maxLateness() const
{
	uint32 most = 0;
	for (size_t i = 0; i < m_Clients.size(); ++i)
		most = std::max(most, m_Clients[i]->Result.maxLateness());
	return most;
}

uint32 CSim::latenessPercentile(double share) const
{
	std::vector<uint32> all;
	for (size_t i = 0; i < m_Clients.size(); ++i)
		all.insert(all.end(), m_Clients[i]->Result.Lateness.begin(), m_Clients[i]->Result.Lateness.end());
	if (all.empty())
		return 0;
	std::sort(all.begin(), all.end());
	size_t index = (size_t)(share * (double)(all.size() - 1) + 0.5);
	return all[std::min(index, all.size() - 1)];
}

void CSim::latenessBetween(double fromSeconds, double toSeconds, double &mean, uint32 &worst, uint32 &missing) const
{
	double sum = 0;
	uint64 count = 0;
	worst = 0;
	missing = 0;
	for (size_t i = 0; i < m_Clients.size(); ++i)
	{
		const CClientResult &r = m_Clients[i]->Result;
		for (size_t k = 0; k < r.Lateness.size(); ++k)
		{
			if (r.LatenessAt[k] >= fromSeconds && r.LatenessAt[k] < toSeconds)
			{
				sum += (double)r.Lateness[k];
				++count;
				worst = std::max(worst, r.Lateness[k]);
			}
		}
		for (size_t k = 0; k < r.MissingAt.size(); ++k)
		{
			if (r.MissingAt[k] >= fromSeconds && r.MissingAt[k] < toSeconds)
				++missing;
		}
	}
	mean = count ? sum / (double)count : 0.0;
}

void CSim::post(TTime time, uint32 type, uint32 client, uint64 a, uint64 b)
{
	CEvent event;
	event.Time = time;
	event.Sequence = m_Sequence++;
	event.Type = type;
	event.Client = client;
	event.A = a;
	event.B = b;
	m_Events.push(event);
}

double CSim::random()
{
	// (xorshift: the same on every platform)
	m_RandomState ^= m_RandomState << 13;
	m_RandomState ^= m_RandomState >> 17;
	m_RandomState ^= m_RandomState << 5;
	return (double)(m_RandomState & 0xFFFFFF) / (double)0x1000000;
}

void CSim::run(double seconds)
{
	const TTime end = m_Now + (TTime)(seconds * 1e6);
	while (!m_Events.empty() && m_Events.top().Time <= end)
	{
		const CEvent event = m_Events.top();
		m_Events.pop();
		m_Now = event.Time;
		if (event.Type == EventTick)
		{
			tick();
			continue;
		}
		if (event.Type == EventUplinkChange)
		{
			// (what waits at the link leaves it as it was worked out when it was sent)
			m_UplinkRate = m_Config.UplinkChanges[(size_t)event.A].BytesPerSecond;
			continue;
		}
		if (event.Client >= m_Clients.size())
			continue;
		CClient &c = *m_Clients[event.Client];
		switch (event.Type)
		{
		case EventPacketArrives:
			packetArrives(c, event.A);
			break;
		case EventAckArrives:
			ackArrives(c, event.A, (TTime)event.B);
			break;
		case EventAckTimer:
			if (c.AckWaiting && c.AckWaitingNumber == event.A)
			{
				c.AckWaiting = false;
				post(m_Now + us(c.Path.DelayMs), EventAckArrives, c.Index, event.A, (uint64)(m_Now - c.AckWaitingSince));
			}
			break;
		case EventClientSends:
			clientSends(c);
			break;
		case EventPacing:
			c.PacingTimer = false;
			flush(c);
			break;
		case EventReportArrives:
			// (the frontend reads it at its next cycle)
			c.QueueDelay = CLFECOMMON::decodeDatagramLateness((uint8)event.B);
			c.QueueDelayAck = (uint32)event.A;
			c.LastReceive = m_Now;
			++c.Reports;
			break;
		case EventTraffic: {
			const CTraffic &traffic = m_Config.Traffic[(size_t)event.A];
			if (m_Scratch.size() < traffic.Bytes)
				m_Scratch.resize(traffic.Bytes, 0x55);
			const uint64 count = traffic.EverySeconds > 0 ? 1 : event.B;
			for (uint64 i = 0; i < count; ++i)
			{
				TStreamRecord record = makeStreamRecord(&m_Scratch[0], traffic.Bytes);
				c.Queues.push(0, record);
				c.StreamPushed += record->size();
				c.Records.push_back(std::make_pair(c.StreamPushed, m_Now));
				++c.Result.RecordsPushed;
			}
			if (traffic.EverySeconds > 0 && event.B > 1)
				post(m_Now + (TTime)(traffic.EverySeconds * 1e6), EventTraffic, event.Client, event.A, event.B - 1);
			break;
		}
		case EventPathChange: {
			const CPathChange &change = m_Config.PathChanges[(size_t)event.A];
			c.Path = change.Path;
			if (change.NewAddress)
			{
				// The transport starts over on a new address (window, round trip time), sends nothing there until
				// the path is validated, and tells the frontend
				c.Cwnd = s_InitialWindow;
				c.Ssthresh = 1e18;
				c.BytesInFlightMax = (uint64)(s_InitialWindow / 2);
				c.Srtt = 333000;
				c.ValidatedAt = m_Now + 2 * us(c.Path.DelayMs) + 2000;
				post(c.ValidatedAt, EventPacing, c.Index);
				c.PathChanged = true;
				c.PathChangedDatagrams = 5;
			}
			break;
		}
		default:
			break;
		}
	}
	m_Now = end;
	m_MaxEgressPerSecond = std::max(m_MaxEgressPerSecond, m_EgressThisSecond);
	for (size_t i = 0; i < m_Clients.size(); ++i)
	{
		m_Clients[i]->Result.AllowanceEnd = m_Clients[i]->Allowance.allowance();
		m_Clients[i]->Result.NoiseEnd = m_Clients[i]->Allowance.noise();
	}
}

void CSim::datagramFate(CClient &c, uint32 datagramNumber, sint32 fate)
{
	if (c.Index != m_Config.TraceClient)
		return;
	std::map<uint32, size_t>::const_iterator it = m_TraceRows.find(datagramNumber);
	if (it != m_TraceRows.end())
		m_Trace[it->second].Datagram = fate;
}

void CSim::snapshot(CClient &c)
{
	// What the frontend reads from MsQuic when a datagram is written and when a block of stream data is acknowledged
	// to its end (CQuicTransceiver::updateStreamStats)
	c.StatCongestionEvents = c.CongestionEvents;
	c.StatCwnd = (sint64)c.Cwnd;
	c.StatRtt = c.Srtt;
	c.StatBytesInFlight = (sint64)c.BytesInFlight;
	c.StatGrantedCount = c.GrantedCount;
	c.StatValid = true;
}

uint32 CSim::sendPacket(CClient &c, bool withDatagram, uint64 streamOffset, uint32 streamLength)
{
	CClient::CPacket p;
	p.Number = c.NextNumber++;
	p.Size = s_PacketOverhead + (withDatagram ? c.PendingBytes : 0) + streamLength;
	p.SendTime = m_Now;
	p.HasDatagram = withDatagram;
	p.DatagramNumber = c.PendingNumber;
	p.DatagramSendTime = c.PendingSendTime;
	p.DatagramPathChanged = c.PendingPathChanged;
	p.DatagramPostTime = c.PendingPostTime;
	p.StreamOffset = streamOffset;
	p.StreamLength = streamLength;
	p.Arrived = false;
	p.Done = false;
	p.Lost = false;
	p.Unloaded = 0;
	if (withDatagram)
	{
		const double alone = (double)(s_PacketOverhead + c.PendingBytes);
		p.Unloaded = us(c.Path.DelayMs) + (TTime)(alone / c.Path.RateBytesPerSecond * 1e6);
		if (m_Config.UplinkBytesPerSecond > 0)
			p.Unloaded += (TTime)((alone + (double)s_WireOverhead) / m_Config.UplinkBytesPerSecond * 1e6);
	}

	if (c.BytesInFlight == 0)
		c.LastProgress = m_Now;
	c.BytesInFlight += p.Size;
	c.BytesInFlightMax = std::max(c.BytesInFlightMax, c.BytesInFlight);

	// What the frontend sends, as it counts it
	const uint64 wire = p.Size + s_WireOverhead;
	while (m_Now - m_EgressSecondStart >= 1000000)
	{
		m_MaxEgressPerSecond = std::max(m_MaxEgressPerSecond, m_EgressThisSecond);
		m_EgressThisSecond = 0;
		m_EgressSecondStart += 1000000;
	}
	m_EgressSinceTick += wire;
	m_EgressTotal += wire;
	m_EgressThisSecond += wire;

	// The frontend's own link: the packet leaves it behind all that was sent before it, to whichever client
	TTime enter = m_Now;
	bool dropped = false;
	if (m_UplinkRate > 0)
	{
		const TTime start = std::max(m_Now, m_UplinkLastDeparture);
		if (start - m_Now > us(m_Config.UplinkBufferMs))
		{
			dropped = true;
			++m_UplinkDrops;
		}
		else
		{
			enter = start + (TTime)((double)wire / m_UplinkRate * 1e6);
			m_UplinkLastDeparture = enter;
		}
	}

	// The bottleneck of the client: what has left it by the time the packet gets there, and whether it still fits
	// (the packets of a client get there in the order they were sent, so this can be worked out now)
	while (!c.Queue.empty() && c.Queue.front().first <= enter)
	{
		c.QueuedBytes -= c.Queue.front().second;
		c.Queue.pop_front();
	}
	p.WillArrive = !dropped && (c.QueuedBytes + p.Size <= c.Path.BufferBytes) && !(c.Path.LossRate > 0 && random() < c.Path.LossRate);
	if (p.WillArrive)
	{
		const TTime departure = std::max(enter, c.LastDeparture) + (TTime)((double)p.Size / c.Path.RateBytesPerSecond * 1e6);
		c.LastDeparture = departure;
		c.Queue.push_back(std::make_pair(departure, p.Size));
		c.QueuedBytes += p.Size;
		TTime arrival = departure + us(c.Path.DelayMs);
		if (c.Path.JitterMs > 0)
			arrival += (TTime)(random() * c.Path.JitterMs * 1000.0);
		if (arrival <= c.LastArrival)
			arrival = c.LastArrival + 1;
		c.LastArrival = arrival;
		post(arrival, EventPacketArrives, c.Index, p.Number);
	}
	c.Sent.push_back(p);

	if (withDatagram)
	{
		c.DatagramPending = false;
		c.DatagramCancels = 0;
		snapshot(c);
	}
	return p.Size;
}

void CSim::flush(CClient &c)
{
	// (with nothing to send there is no flush, and the pacing does not hear of it)
	if (!c.DatagramPending && c.Retransmit.empty() && c.StreamNextSend >= c.StreamPosted)
		return;

	// What may be sent now: what the congestion window leaves, and with pacing no more than the time since the last
	// flush allows on top of what that one left (MsQuic: CubicCongestionControlGetSendAllowance)
	// (On a new address nothing goes until the path is validated.)
	const bool validated = m_Now >= c.ValidatedAt;
	sint64 allowance = validated ? std::max((sint64)0, (sint64)c.Cwnd - (sint64)c.BytesInFlight) : 0;
	if (allowance > 0 && m_Config.Pacing && c.FlushValid && c.HasRtt && c.Srtt >= 1000)
	{
		const double estimated = c.Cwnd < c.Ssthresh ? std::min(2.0 * c.Cwnd, c.Ssthresh) : 1.25 * c.Cwnd;
		const sint64 paced = c.LastSendAllowance + (sint64)(estimated * (double)(m_Now - c.LastFlush) / (double)c.Srtt);
		allowance = std::min(allowance, paced);
		c.LastSendAllowance = allowance;
	}
	c.LastFlush = m_Now;
	c.FlushValid = true;

	for (;;)
	{
		const bool hasStream = !c.Retransmit.empty() || c.StreamNextSend < c.StreamPosted;
		if (!c.DatagramPending && !hasStream)
			break;
		if (allowance <= 0)
		{
			// The window has room and the pacing does not: it goes on a millisecond later
			if (validated && (sint64)c.Cwnd > (sint64)c.BytesInFlight && !c.PacingTimer)
			{
				c.PacingTimer = true;
				post(m_Now + 1000, EventPacing, c.Index);
			}
			break;
		}

		// A packet is written whole once anything is allowed: the datagram, and stream data behind it, what was lost
		// first and then what is new
		const bool withDatagram = c.DatagramPending;
		const uint32 used = s_PacketOverhead + (withDatagram ? c.PendingBytes : 0);
		const uint32 space = s_Mtu > used ? s_Mtu - used : 0;
		uint64 streamOffset = 0;
		uint32 streamLength = 0;
		if (hasStream && space >= 16)
		{
			if (!c.Retransmit.empty())
			{
				std::pair<uint64, uint32> &range = c.Retransmit.front();
				streamOffset = range.first;
				streamLength = std::min(range.second, space);
				range.first += streamLength;
				range.second -= streamLength;
				if (range.second == 0)
					c.Retransmit.pop_front();
			}
			else
			{
				streamOffset = c.StreamNextSend;
				streamLength = (uint32)std::min((uint64)space, c.StreamPosted - c.StreamNextSend);
				c.StreamNextSend += streamLength;
			}
		}
		if (!withDatagram && streamLength == 0)
			break;
		const sint64 size = (sint64)sendPacket(c, withDatagram, streamOffset, streamLength);
		allowance -= size;
		c.LastSendAllowance = std::max((sint64)0, c.LastSendAllowance - size);
	}

	// A datagram that this flush did not write is dropped if it was sent so, whatever kept it (MsQuic does that at the
	// end of QuicSendFlush); else it waits
	if (c.DatagramPending && c.PendingCancel)
	{
		c.DatagramPending = false;
		++c.SendBusyDrops;
		++c.DatagramCancels;
		++c.Result.DatagramsCancelled;
		c.Result.MissingAt.push_back((float)((double)m_Now / 1e6));
		datagramFate(c, c.PendingNumber, CTickTrace::DatagramCancelled);
	}
}

void CSim::packetArrives(CClient &c, uint64 number)
{
	if (!c.has(number))
		return;
	CClient::CPacket &p = c.packet(number);
	p.Arrived = true;

	if (p.HasDatagram)
	{
		// The client: when it arrived, on its own clock in milliseconds
		const uint32 clientClock = (uint32)((double)m_Now * (1.0 + c.Config.ClockDriftPpm * 1e-6) / 1000.0) + 123456789u;
		c.Lateness.add(p.DatagramSendTime, clientClock, p.DatagramPathChanged);
		if (!c.AnyReceived || p.DatagramNumber > c.LastReceivedNumber)
		{
			if (p.DatagramNumber > c.LastReceivedNumber + 1)
				c.Result.LongestGap = std::max(c.Result.LongestGap, p.DatagramNumber - c.LastReceivedNumber - 1);
			c.LastReceivedNumber = p.DatagramNumber;
		}
		c.AnyReceived = true;

		// How late it is in truth: its time on the way above what the path, as it was when it was sent, takes for it
		// alone
		const TTime late = std::max((TTime)0, m_Now - p.DatagramPostTime - p.Unloaded);
		const uint32 lateMs = (uint32)(late / 1000);
		c.Result.Lateness.push_back(lateMs);
		c.Result.LatenessAt.push_back((float)((double)p.DatagramPostTime / 1e6));
		++c.Result.DatagramsArrived;
		if (p.Lost)
		{
			// (the transport took it for lost too early)
			--c.Result.DatagramsLost;
			const float at = (float)((double)p.DatagramPostTime / 1e6);
			for (size_t k = c.Result.MissingAt.size(); k-- != 0;)
			{
				if (c.Result.MissingAt[k] == at)
				{
					c.Result.MissingAt.erase(c.Result.MissingAt.begin() + k);
					break;
				}
			}
		}
		datagramFate(c, p.DatagramNumber, (sint32)lateMs);
	}

	if (p.StreamLength != 0)
	{
		addRange(c.ArrivedRanges, c.StreamArrived, p.StreamOffset, p.StreamLength);
		c.Result.StreamBytesArrived = c.StreamArrived;
		while (!c.Records.empty() && c.Records.front().first <= c.StreamArrived)
		{
			c.Result.RecordSeconds.push_back((double)(m_Now - c.Records.front().second) / 1e6);
			c.Records.pop_front();
		}
	}

	// The client's transport acknowledges every second packet at once, and a lone one after a while
	if (c.AckWaiting)
	{
		c.AckWaiting = false;
		post(m_Now + us(c.Path.DelayMs), EventAckArrives, c.Index, c.AckWaitingNumber, (uint64)(m_Now - c.AckWaitingSince));
		post(m_Now + us(c.Path.DelayMs), EventAckArrives, c.Index, number, 0);
	}
	else
	{
		c.AckWaiting = true;
		c.AckWaitingNumber = number;
		c.AckWaitingSince = m_Now;
		post(m_Now + s_AckDelay, EventAckTimer, c.Index, number);
	}
}

void CSim::packetLost(CClient &c, uint64 number)
{
	CClient::CPacket &p = c.packet(number);
	p.Done = true;
	p.Lost = true;
	c.BytesInFlight -= p.Size;
	++c.Result.PacketsLost;
	if (p.HasDatagram)
	{
		++c.Result.DatagramsLost;
		c.Result.MissingAt.push_back((float)((double)p.DatagramPostTime / 1e6));
		datagramFate(c, p.DatagramNumber, CTickTrace::DatagramLost);
	}
	if (p.StreamLength != 0 && p.StreamOffset + p.StreamLength > c.StreamAcked)
		c.Retransmit.push_back(std::make_pair(p.StreamOffset, p.StreamLength));

	// The window is cut once for the losses of one flight
	if (number >= c.RecoveryStart)
	{
		c.Cwnd = std::max(c.Cwnd * 0.7, s_MinWindow);
		c.Ssthresh = c.Cwnd;
		c.RecoveryStart = c.NextNumber;
		++c.CongestionEvents;
		++c.Result.CongestionEvents;
	}
}

void CSim::findLosses(CClient &c, uint64 ackedNumber)
{
	if (!c.AnyAcked || ackedNumber > c.LargestAcked)
		c.LargestAcked = ackedNumber;
	c.AnyAcked = true;
	// A packet of before the last one acknowledged is lost when more than three packets were sent between them, or
	// when it was sent more than a round trip and an eighth ago
	const TTime timeThreshold = c.Srtt + c.Srtt / 8;
	for (size_t i = 0; i < c.Sent.size(); ++i)
	{
		CClient::CPacket &p = c.Sent[i];
		if (p.Number >= c.LargestAcked)
			break;
		if (!p.Done && (p.Number + s_LossPackets < c.LargestAcked || p.SendTime + timeThreshold <= m_Now))
			packetLost(c, p.Number);
	}
}

void CSim::ackArrives(CClient &c, uint64 number, TTime ackDelay)
{
	if (!c.has(number))
		return;
	CClient::CPacket &p = c.packet(number);
	c.LastProgress = m_Now;
	c.ProbeTimeout = std::max((TTime)200000, 2 * c.Srtt + s_AckDelay);
	if (!p.Done)
	{
		p.Done = true;
		c.BytesInFlight -= p.Size;

		// (the client says how long it held the acknowledgement back, which is not of the round trip)
		const TTime sample = std::max((TTime)1, m_Now - p.SendTime - ackDelay);
		c.Srtt = c.HasRtt ? (7 * c.Srtt + sample) / 8 : sample;
		c.HasRtt = true;

		// Slow start, then a packet per window, not for the packets of before the last cut; never above twice what
		// was in flight
		if (number >= c.RecoveryStart)
		{
			if (c.Cwnd < c.Ssthresh)
				c.Cwnd += p.Size;
			else
				c.Cwnd += (double)s_Mtu * (double)p.Size / c.Cwnd;
		}
		c.Cwnd = std::min(c.Cwnd, std::max(2.0 * (double)c.BytesInFlightMax, s_InitialWindow));
	}
	if (p.StreamLength != 0)
	{
		// (a packet that was taken for lost and arrived all the same still acknowledges its stream data, as long as
		// it is remembered)
		addRange(c.AckedRanges, c.StreamAcked, p.StreamOffset, p.StreamLength);
		// The frontend hears of it block by block: one is outstanding until all of it is acknowledged
		bool completed = false;
		while (!c.Blocks.empty() && c.Blocks.front().first <= c.StreamAcked)
		{
			c.Outstanding -= c.Blocks.front().second;
			c.Blocks.pop_front();
			completed = true;
		}
		if (completed)
			snapshot(c);
	}

	findLosses(c, number);

	// What is settled at the front is forgotten
	while (!c.Sent.empty() && c.Sent.front().Done && (c.Sent.front().Arrived || !c.Sent.front().WillArrive))
	{
		c.Sent.pop_front();
		++c.SentBase;
	}

	flush(c);
}

void CSim::clientSends(CClient &c)
{
	post(m_Now + us(c.Config.SendPeriodMs), EventClientSends, c.Index);
	const double now = (double)m_Now / 1e6;
	for (size_t i = 0; i < c.Config.Stalls.size(); ++i)
	{
		if (now >= c.Config.Stalls[i].first && now < c.Config.Stalls[i].second)
			return;
	}
	if (!c.AnyReceived)
		return;
	// (on a path that lets nothing through, nothing gets back either)
	if (c.Path.LossRate >= 1.0)
		return;
	// Its datagram, with the number of the last one it got and its report in its byte
	c.Result.ReportedMax = std::max(c.Result.ReportedMax, CLFECOMMON::decodeDatagramLateness(CLFECOMMON::encodeDatagramLateness(c.Lateness.report())));
	post(m_Now + us(c.Path.DelayMs), EventReportArrives, c.Index, c.LastReceivedNumber, CLFECOMMON::encodeDatagramLateness(c.Lateness.report()));
}

void CSim::tick()
{
	const CStreamSchedulerConfig &config = m_Config.Scheduler;
	const double elapsed = m_Ticked ? (double)(m_Now - m_LastTick) / 1e6 : config.TickSeconds;
	const uint64 measured = m_Ticked ? m_EgressSinceTick : 0;
	m_EgressSinceTick = 0;
	m_Ticked = true;
	m_LastTick = m_Now;

	uint64 datagramBytes = 0;
	uint32 reporting = 0, reportingLate = 0;
	const bool sharedLateness = m_Scheduler.sharedLateness(); // (as of the cycle before)
	m_Entries.clear();
	for (size_t i = 0; i < m_Clients.size(); ++i)
	{
		CClient &c = *m_Clients[i];

		// The transport: nothing acknowledged for a while with packets in flight. It sends a probe, a packet that goes
		// whatever the window says, and waits twice as long for the next one. The acknowledgement of a probe shows
		// what of before it was lost.
		if (c.BytesInFlight != 0 && m_Now - c.LastProgress > c.ProbeTimeout)
		{
			sendPacket(c, false, 0, 0);
			c.ProbeTimeout = std::min(c.ProbeTimeout * 2, (TTime)8000000);
			c.LastProgress = m_Now;
		}

		// A client that is gone gets nothing any more
		if (c.Gone)
		{
			c.WritesDatagram = false;
			CStreamScheduler::CEntry gone;
			gone.Client = &c.Turn;
			gone.Id = (uint32)i;
			m_Entries.push_back(gone);
			continue;
		}

		// The state of the client, as CFrontEndService::monitorClients keeps it
		if (!c.Probe)
		{
			if (m_Now - c.LastReceive > s_ClientLagTime)
			{
				c.Probe = true;
				c.ProbeUntil = m_Now + s_ProbeTime;
				c.ProbeCycle = 0;
				++c.Result.Probes;
			}
		}
		else if (m_Now >= c.ProbeUntil && c.Path.LossRate < 1.0)
		{
			// (it answered the probes)
			c.Probe = false;
			c.LastReceive = m_Now;
		}
		else if (m_Now - c.LastReceive > s_ClientTimeOut)
		{
			c.Gone = true;
			c.Result.Gone = true;
			c.Queues.clear();
			c.WritesDatagram = false;
			CStreamScheduler::CEntry gone;
			gone.Client = &c.Turn;
			gone.Id = (uint32)i;
			m_Entries.push_back(gone);
			continue;
		}

		// The datagram of this cycle; for a client that is probed, a probe every third cycle
		c.WritesDatagram = !c.Probe || (c.ProbeCycle++ % 3) == 0;
		if (c.WritesDatagram)
		{
			++c.SendNumber;
			++c.Result.DatagramsWritten;
			c.CycleDatagramBytes = c.Probe ? s_ProbeBytes : c.Config.DatagramBytes;
			datagramBytes += c.CycleDatagramBytes + s_QuicDatagramOverhead;
		}

		// The allowance of the connection, as CFeSendSub::grantStreams asks it
		if (c.PathChanged)
		{
			c.PathChanged = false;
			c.Allowance.reset(config);
		}
		const bool newReport = c.Reports != c.ReportsSeen;
		c.ReportsSeen = c.Reports;
		if (newReport)
		{
			++reporting;
			if (c.QueueDelay > config.QueueDelayTarget)
				++reportingLate;
		}
		c.Allowance.update(config, newReport, c.QueueDelay, c.QueueDelayAck, c.SendNumber, c.StatCongestionEvents, c.SendBusyDrops, sharedLateness);
		const uint32 allowance = c.Allowance.allowance();
		if (c.LastAllowance != 0)
		{
			if (allowance < c.LastAllowance)
				++c.Result.Cuts;
			else if (allowance > c.LastAllowance)
				++c.Result.Raises;
		}
		c.LastAllowance = allowance;
		c.Result.AllowanceMin = std::min(c.Result.AllowanceMin, allowance);
		c.Result.AllowanceMax = std::max(c.Result.AllowanceMax, allowance);

		CStreamScheduler::CEntry entry;
		entry.Client = &c.Turn;
		entry.Id = (uint32)i;
		entry.Pending = c.Queues.pendingBytes();
		if (c.Probe)
		{
			// (stream data waits for the connected state, as the impulses do)
			entry.Window = 0;
		}
		else if (!c.StatValid)
		{
			entry.Window = std::min(config.StreamMinBytesPerTick, config.StreamMaxBytesPerTick);
		}
		else
		{
			CStreamWindowState state;
			state.CongestionWindow = c.StatCwnd;
			state.RttMicroseconds = c.StatRtt;
			state.BytesInFlight = c.StatBytesInFlight;
			state.Outstanding = (sint64)c.Outstanding;
			state.GrantedSinceSnapshot = (uint32)(c.GrantedCount - c.StatGrantedCount);
			entry.Window = std::min(allowance, streamWindowRoom(config, state));
		}
		m_Entries.push_back(entry);
	}

	m_Scheduler.reports(config, reporting, reportingLate);
	m_Scheduler.schedule(config, elapsed, measured, datagramBytes, m_Entries);
	m_BudgetMin = std::min(m_BudgetMin, m_Scheduler.budget(config));

	// The trace of one client, before this cycle sends
	if (m_Config.TraceClient < m_Clients.size())
	{
		const CClient &c = *m_Clients[m_Config.TraceClient];
		const CStreamScheduler::CEntry &entry = m_Entries[m_Config.TraceClient];
		CTickTrace t;
		t.Seconds = (double)m_Now / 1e6;
		t.Allowance = c.Allowance.allowance();
		t.StartUp = c.Allowance.startUp();
		t.Probe = c.Probe;
		t.Window = entry.Window;
		t.Grant = entry.Grant;
		t.Pending = entry.Pending;
		t.Outstanding = (uint32)c.Outstanding;
		t.CongestionWindow = (uint32)c.Cwnd;
		t.BytesInFlight = (uint32)c.BytesInFlight;
		t.RttMs = (uint32)(c.Srtt / 1000);
		t.Reported = c.QueueDelay;
		t.Datagram = c.WritesDatagram ? (sint32)CTickTrace::DatagramOnItsWay : (sint32)CTickTrace::NoDatagram;
		if (c.WritesDatagram)
			m_TraceRows[c.SendNumber] = m_Trace.size();
		m_Trace.push_back(t);
	}

	// First the datagrams of all the clients
	for (size_t i = 0; i < m_Clients.size(); ++i)
	{
		CClient &c = *m_Clients[i];
		if (!c.WritesDatagram)
			continue;
		if (c.DatagramPending)
		{
			// The one before has not left: this one is not sent
			++c.SendBusyDrops;
			++c.Result.DatagramsNotSent;
			c.Result.MissingAt.push_back((float)((double)m_Now / 1e6));
			datagramFate(c, c.SendNumber, CTickTrace::DatagramNotSent);
			continue;
		}
		c.DatagramPending = true;
		c.PendingNumber = c.SendNumber;
		c.PendingBytes = c.CycleDatagramBytes;
		c.PendingCancel = m_Config.CancelOnBlocked && c.DatagramCancels < 3;
		c.PendingSendTime = (uint16)(m_Now / 1000);
		c.PendingPathChanged = false;
		if (c.PathChangedDatagrams != 0)
		{
			--c.PathChangedDatagrams;
			c.PendingPathChanged = true;
		}
		c.PendingPostTime = m_Now;
		flush(c);
	}

	// Then the stream data
	for (size_t i = 0; i < m_Entries.size(); ++i)
	{
		const CStreamScheduler::CEntry &entry = m_Entries[i];
		CClient &c = *m_Clients[entry.Id];
		c.Allowance.granted(entry.Grant, entry.Pending > entry.Grant);
		if (entry.Grant == 0)
			continue;
		if (m_Scratch.size() < entry.Grant)
			m_Scratch.resize(entry.Grant, 0x55);
		const uint32 taken = c.Queues.take(0, &m_Scratch[0], entry.Grant);
		c.StreamPosted += taken;
		c.GrantedCount += taken;
		if (taken != 0)
		{
			c.Blocks.push_back(std::make_pair(c.StreamPosted, taken));
			c.Outstanding += taken;
		}
		flush(c);
	}

	post(m_Now + (TTime)(config.TickSeconds * 1e6), EventTick, 0);
}

namespace /* anonymous */ {

CPathConfig path(double megabitsPerSecond, double delayMs, double bufferMs)
{
	CPathConfig p;
	p.RateBytesPerSecond = megabitsPerSecond * 125000.0;
	p.DelayMs = delayMs;
	p.BufferBytes = (uint32)(p.RateBytesPerSecond * bufferMs / 1000.0);
	return p;
}

void oneClient(CSimConfig &config, const CPathConfig &p)
{
	CClientConfig client;
	client.Path = p;
	config.Clients.push_back(client);
}

struct CScenario
{
	const char *Name;
	const char *Description;
};

const CScenario s_Scenarios[] = {
	{ "idle", "one client at 2 Mbit/s, no stream data" },
	{ "login", "one client at 2 Mbit/s, 270 KB of records at once" },
	{ "login-1mbit", "the same at 1 Mbit/s" },
	{ "login-5mbit", "the same at 5 Mbit/s" },
	{ "login-fast", "the same at 100 Mbit/s" },
	{ "login-far", "the same at 2 Mbit/s with 100 ms each way" },
	{ "backlog", "one client at 2 Mbit/s, 6 MB at once" },
	{ "backlog-1mbit", "one client at 1 Mbit/s, 3 MB at once" },
	{ "backlog-shallow", "one client at 2 Mbit/s whose bottleneck holds 30 ms, 6 MB at once" },
	{ "trickle", "one client at 2 Mbit/s that loses a packet in a hundred, a string of 300 bytes twice a second" },
	{ "messages", "one client at 1 Mbit/s, a message of 30 KB every 3 s" },
	{ "messages-lossy", "the same on a path that loses a packet in a hundred" },
	{ "jitter", "one client at 2 Mbit/s with up to 15 ms of jitter, a message of 30 KB every 3 s" },
	{ "jitter-wifi", "the same with up to 40 ms of jitter, twice the lateness the frontend allows" },
	{ "backlog-wifi", "one client at 2 Mbit/s with up to 40 ms of jitter, 6 MB at once" },
	{ "backlog-lossy", "one client at 2 Mbit/s that loses a packet in a hundred, 6 MB at once" },
	{ "login-drift", "one client at 2 Mbit/s whose clock runs 150 parts per million fast, 270 KB after 45 s" },
	{ "dark", "one client at 2 Mbit/s whose path lets nothing through for 4 s in the middle of 6 MB" },
	{ "dark-long", "the same for 15 s: the frontend gives the client up" },
	{ "pathdrop", "one client whose path goes from 5 to 1 Mbit/s in the middle of 6 MB" },
	{ "newaddress", "one client at 2 Mbit/s that changes network in the middle of 3 MB" },
	{ "stall", "one client at 2 Mbit/s that sends nothing for 10 s in the middle of 6 MB (a loading screen): the frontend probes it" },
	{ "stall-startup", "the same from 4 s after the start of 3 MB, while its allowance still looks for its level" },
	{ "wave", "300 clients on paths of 1 to 20 Mbit/s, 60 KB each, one more every 50 ms" },
	{ "wave-tight", "the same with a frontend that may send 500 KB per second in all" },
	{ "uplink-idle", "300 clients, no stream data, behind a link of the frontend of 2.5 MB/s" },
	{ "uplink-udp", "300 clients, no stream data, behind a link that their datagrams alone fill" },
	{ "uplink-streams", "300 clients with 1 MB each, a frontend that may send 2.5 MB/s behind a link of 2.5 MB/s" },
	{ "uplink-streams-90", "the same with a frontend that may send 90% of what its link carries" },
	{ "uplink-short", "the same with a link that carries 2 MB/s, a fifth less than the frontend may send" },
	{ "uplink-dip", "the same with a link of 2.5 MB/s that carries 1.5 MB/s from 20 s to 40 s" },
	{ "uplink-dip-udp", "the dip without stream data: what it does to the datagrams alone, which take 1.13 MB/s" },
	{ "uplink-starved", "a link that carries 1 MB/s from 20 s to 40 s, less than the datagrams alone, with stream data" },
	{ "uplink-starved-udp", "the same without stream data" },
	{ "uplink-wave", "300 clients behind that link without stream data until 10 s, then 1 MB each at once" },
};

} /* anonymous namespace */

std::vector<std::string> scenarioNames()
{
	std::vector<std::string> names;
	for (size_t i = 0; i < sizeof(s_Scenarios) / sizeof(s_Scenarios[0]); ++i)
		names.push_back(s_Scenarios[i].Name);
	return names;
}

bool makeScenario(const std::string &name, CSimConfig &config, double &seconds, std::string &description)
{
	config = CSimConfig();
	seconds = 30;
	description.clear();
	for (size_t i = 0; i < sizeof(s_Scenarios) / sizeof(s_Scenarios[0]); ++i)
	{
		if (name == s_Scenarios[i].Name)
			description = s_Scenarios[i].Description;
	}
	if (description.empty())
		return false;

	// (270 KB as 180 records of 1500 bytes: the database of a login, about)
	if (name == "idle")
	{
		oneClient(config, path(2, 20, 250));
	}
	else if (name == "login")
	{
		oneClient(config, path(2, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 180));
	}
	else if (name == "login-1mbit")
	{
		oneClient(config, path(1, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 180));
	}
	else if (name == "login-5mbit")
	{
		oneClient(config, path(5, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 180));
	}
	else if (name == "login-fast")
	{
		oneClient(config, path(100, 5, 50));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 180));
	}
	else if (name == "login-far")
	{
		oneClient(config, path(2, 100, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 180));
	}
	else if (name == "backlog")
	{
		oneClient(config, path(2, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 4000));
		seconds = 60;
	}
	else if (name == "backlog-1mbit")
	{
		oneClient(config, path(1, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 2000));
		seconds = 60;
	}
	else if (name == "backlog-shallow")
	{
		oneClient(config, path(2, 20, 30));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 4000));
		seconds = 60;
	}
	else if (name == "trickle")
	{
		CPathConfig p = path(2, 20, 250);
		p.LossRate = 0.01;
		oneClient(config, p);
		config.Traffic.push_back(CTraffic(0, 1.0, 300, 600, 0.5));
		seconds = 310;
	}
	else if (name == "messages")
	{
		oneClient(config, path(1, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 30000, 40, 3.0));
		seconds = 125;
	}
	else if (name == "messages-lossy")
	{
		CPathConfig p = path(1, 20, 250);
		p.LossRate = 0.01;
		oneClient(config, p);
		config.Traffic.push_back(CTraffic(0, 1.0, 30000, 40, 3.0));
		seconds = 125;
	}
	else if (name == "jitter-wifi")
	{
		CPathConfig p = path(2, 20, 250);
		p.JitterMs = 40;
		oneClient(config, p);
		config.Traffic.push_back(CTraffic(0, 1.0, 30000, 40, 3.0));
		seconds = 125;
	}
	else if (name == "backlog-wifi" || name == "backlog-lossy")
	{
		CPathConfig p = path(2, 20, 250);
		if (name == "backlog-wifi")
			p.JitterMs = 40;
		else
			p.LossRate = 0.01;
		oneClient(config, p);
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 4000));
		seconds = 60;
	}
	else if (name == "login-drift")
	{
		CClientConfig client;
		client.Path = path(2, 20, 250);
		client.ClockDriftPpm = 150;
		config.Clients.push_back(client);
		config.Traffic.push_back(CTraffic(0, 45.0, 1500, 180));
		seconds = 60;
	}
	else if (name == "dark" || name == "dark-long")
	{
		oneClient(config, path(2, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 4000));
		CPathConfig nothing = path(2, 20, 250);
		nothing.LossRate = 1.0;
		config.PathChanges.push_back(CPathChange(0, 10.0, nothing, false));
		config.PathChanges.push_back(CPathChange(0, name == "dark" ? 14.0 : 25.0, path(2, 20, 250), false));
		seconds = 60;
	}
	else if (name == "jitter")
	{
		CPathConfig p = path(2, 20, 250);
		p.JitterMs = 15;
		oneClient(config, p);
		config.Traffic.push_back(CTraffic(0, 1.0, 30000, 40, 3.0));
		seconds = 125;
	}
	else if (name == "pathdrop")
	{
		oneClient(config, path(5, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 4000));
		config.PathChanges.push_back(CPathChange(0, 6.0, path(1, 20, 250), false));
		seconds = 60;
	}
	else if (name == "newaddress")
	{
		oneClient(config, path(2, 20, 250));
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, 2000));
		config.PathChanges.push_back(CPathChange(0, 8.0, path(2, 60, 250), true));
		seconds = 40;
	}
	else if (name == "stall" || name == "stall-startup")
	{
		const bool startUp = name == "stall-startup";
		CClientConfig client;
		client.Path = path(2, 20, 250);
		client.Stalls.push_back(startUp ? std::make_pair(5.0, 15.0) : std::make_pair(15.0, 25.0));
		config.Clients.push_back(client);
		config.Traffic.push_back(CTraffic(0, 1.0, 1500, startUp ? 2000 : 4000));
		seconds = startUp ? 40 : 60;
	}
	else if (name == "wave" || name == "wave-tight")
	{
		for (uint32 i = 0; i < 300; ++i)
		{
			CClientConfig client;
			client.Path = path(1.0 + (double)(i % 20), 10.0 + (double)(i % 7) * 10.0, 250);
			client.SendPhaseMs = (double)(i % 100);
			config.Clients.push_back(client);
			config.Traffic.push_back(CTraffic(i, 1.0 + 0.05 * (double)i, 1500, 40));
		}
		if (name == "wave-tight")
			config.Scheduler.TotalOutgoingBandwidth = 500000;
		seconds = 60;
	}
	else if (name.compare(0, 7, "uplink-") == 0)
	{
		// The frontend sends the datagrams of all its clients at once, and its link spreads them over the cycle by
		// their place in that order. uplink-udp is the measure for the others: a frontend without streams whose
		// datagrams fill its link, where the last client gets its datagram a cycle late.
		const uint32 nbClients = 300;
		const bool wave = name == "uplink-wave";
		const bool dip = name == "uplink-dip" || name == "uplink-dip-udp";
		const bool starved = name == "uplink-starved" || name == "uplink-starved-udp";
		const bool streams = name == "uplink-streams" || name == "uplink-streams-90" || wave || name == "uplink-short" || name == "uplink-dip" || name == "uplink-starved";
		for (uint32 i = 0; i < nbClients; ++i)
		{
			CClientConfig client;
			client.Path = path(2.0 + (double)(i % 19), 10.0 + (double)(i % 7) * 10.0, 250);
			client.SendPhaseMs = (double)(i % 100);
			config.Clients.push_back(client);
			if (streams)
				config.Traffic.push_back(CTraffic(i, wave ? 10.0 : 1.0, 1500, 700));
		}
		config.TraceClient = nbClients - 1;
		config.Scheduler.TotalOutgoingBandwidth = 2500000;
		config.UplinkBytesPerSecond = 2500000;
		if (name == "uplink-udp")
			config.UplinkBytesPerSecond = (double)(nbClients * (config.Clients[0].DatagramBytes + s_PacketOverhead + s_WireOverhead)) / config.Scheduler.TickSeconds;
		else if (name == "uplink-streams-90")
			config.Scheduler.TotalOutgoingBandwidth = 2250000;
		else if (name == "uplink-short")
			config.UplinkBytesPerSecond = 2000000;
		if (dip || starved)
		{
			config.UplinkChanges.push_back(CUplinkChange(20.0, dip ? 1500000 : 1000000));
			config.UplinkChanges.push_back(CUplinkChange(40.0, 2500000));
		}
		seconds = 60;
	}
	return true;
}

} // STREAM_SIM

/* end of file */
