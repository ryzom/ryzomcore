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

// The record framing of the QUIC streams (game_share/stream_record.h), the frontend's queues of records
// (stream_queues.h), the room for the send time in the frontend's datagrams, and the client's measure of how late
// those arrive (game_share/datagram_lateness.h).

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "game_share/stream_record.h"
#include "game_share/datagram_lateness.h"

#include "stream_queues.h"

using namespace CLFECOMMON;

namespace /* anonymous */ {

// The classes of the frontend in these tests, by number
static const uint StreamStrings = 0, StreamChat = 1, StreamLogin = 2, StreamGateway = 3;
static const uint NbClasses = 4;

#define CHECK(cond) EXPECT_TRUE(cond)

static uint32 s_Seed = 12345;
static uint32 rnd()
{
	s_Seed = s_Seed * 1664525u + 1013904223u;
	return s_Seed >> 8;
}

static std::vector<uint8> message(uint32 length, uint8 tag)
{
	std::vector<uint8> m(length);
	for (uint32 i = 0; i < length; ++i)
		m[i] = (uint8)(tag + i * 7);
	return m;
}

// What a client does with a stream (CQuicConnection::receiveStreamRecord)
struct CClientSide
{
	CStreamRecordReader Reader;
	uint32 PendingBoundaries;
	uint32 UnexpectedBoundaries;
	bool Failed;
	std::vector<std::vector<uint8> > Received;

	CClientSide()
	    : PendingBoundaries(0)
	    , UnexpectedBoundaries(0)
	    , Failed(false)
	{
	}

	void feed(const uint8 *data, uint32 size)
	{
		Reader.push(data, size);
		for (;;)
		{
			const uint8 *d;
			uint32 l;
			CStreamRecordReader::TResult res = Reader.next(d, l, StreamRecordClientMaxBytes);
			if (res == CStreamRecordReader::NoRecord)
				return;
			if (res == CStreamRecordReader::BadLength)
			{
				Failed = true;
				return;
			}
			if (res == CStreamRecordReader::Boundary)
			{
				if (PendingBoundaries)
					--PendingBoundaries;
				else
					++UnexpectedBoundaries;
				continue;
			}
			if (PendingBoundaries)
				continue;
			Received.push_back(std::vector<uint8>(d, d + l));
		}
	}
};

TEST(StreamRecord, LengthsAndStreamIds)
{
	const uint32 lengths[] = { 1, 2, 127, 128, 129, 16383, 16384, 2097151, 2097152, StreamRecordLengthMax };
	const uint sizes[] = { 1, 1, 1, 2, 2, 2, 3, 3, 4, 4 };
	for (uint i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i)
	{
		uint8 buf[StreamRecordLengthMaxBytes];
		uint n = writeStreamRecordLength(buf, lengths[i]);
		CHECK(n == sizes[i]);
		// read it back through a reader, with a limit above it: the record is not complete, the length is accepted
		CStreamRecordReader reader;
		reader.push(buf, n);
		const uint8 *d;
		uint32 l;
		CHECK(reader.next(d, l, StreamRecordLengthMax) == CStreamRecordReader::NoRecord);
		// and with a limit just below it: refused, with the length read
		CStreamRecordReader reader2;
		reader2.push(buf, n);
		CHECK(reader2.next(d, l, lengths[i] - 1) == CStreamRecordReader::BadLength);
		CHECK(l == lengths[i]);
		CHECK(reader2.next(d, l, lengths[i] - 1) == CStreamRecordReader::BadLength); // stays so
	}

	// a length that does not end within 4 bytes
	{
		const uint8 bad[5] = { 0x80, 0x80, 0x80, 0x80, 0x01 };
		CStreamRecordReader reader;
		reader.push(bad, 3);
		const uint8 *d;
		uint32 l;
		CHECK(reader.next(d, l, StreamRecordClientMaxBytes) == CStreamRecordReader::NoRecord);
		reader.push(bad + 3, 2);
		CHECK(reader.next(d, l, StreamRecordClientMaxBytes) == CStreamRecordReader::BadLength);
		CHECK(l == 0);
	}

	// stream ids
	CHECK(streamIdOfClass(StreamStrings) == 3 && streamIdOfClass(StreamChat) == 7 && streamIdOfClass(StreamLogin) == 11 && streamIdOfClass(StreamGateway) == 15);
	CHECK(streamClassOfId(3) == StreamStrings && streamClassOfId(15) == StreamGateway);
	CHECK(streamClassOfId(19) == 4 && streamClassOfId(63) == 15 && streamClassOfId(67) == NoStreamClass);
	CHECK(streamClassOfId(0) == NoStreamClass && streamClassOfId(2) == NoStreamClass && streamClassOfId(1) == NoStreamClass);
	CHECK(streamIdOfClass(StreamClassMax - 1) == 63);
}

// Records of many sizes through the queues and a reader, cut at random places on both sides
TEST(StreamQueues, RoundTripThroughTheReader)
{
	CStreamQueues queues;
	std::vector<std::vector<uint8> > sent;
	const uint32 sizes[] = { 1, 5, 126, 127, 128, 129, 300, 16383, 16384, 70000, 200000, 3, 1 };
	for (uint round = 0; round < 6; ++round)
	{
		for (uint i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i)
		{
			std::vector<uint8> m = message(sizes[i], (uint8)(round * 16 + i));
			queues.push(StreamStrings, makeStreamRecord(&m[0], (uint32)m.size()));
			sent.push_back(m);
		}
	}
	uint32 pending = queues.pendingBytes();
	CHECK(pending == queues.pendingBytes(StreamStrings));
	CHECK(queues.pendingBytes(StreamChat) == 0);

	CClientSide client;
	std::vector<uint8> wire;
	uint32 total = 0;
	while (queues.pendingBytes() != 0)
	{
		uint32 want = 1 + rnd() % 3000;
		std::vector<uint8> block(want);
		uint32 got = queues.take(StreamStrings, &block[0], want);
		CHECK(got != 0 && got <= want);
		total += got;
		// the network cuts it differently again
		uint32 pos = 0;
		while (pos < got)
		{
			uint32 part = std::min(got - pos, 1 + rnd() % 1400);
			client.feed(&block[pos], part);
			pos += part;
		}
	}
	CHECK(total == pending);
	CHECK(!client.Failed);
	CHECK(client.UnexpectedBoundaries == 0);
	CHECK(client.Reader.bufferedBytes() == 0);
	CHECK(client.Received.size() == sent.size());
	bool same = client.Received.size() == sent.size();
	for (size_t i = 0; same && i < sent.size(); ++i)
		same = (client.Received[i] == sent[i]);
	CHECK(same);

	uint8 one;
	CHECK(queues.take(StreamStrings, &one, 1) == 0);
	CHECK(queues.take(StreamClassMax - 1, &one, 1) == 0 && queues.pendingBytes(StreamClassMax - 1) == 0); // never used
}

// The end of a session on a connection that is kept: startedBytes of the first record were sent when it ends
void sessionEnd(uint32 startedBytes)
{
	CStreamQueues queues;
	CClientSide client;

	// The old session: three records, of which the first is 1000 bytes
	std::vector<uint8> a = message(1000, 1), b = message(50, 2), c = message(70, 3);
	queues.push(StreamStrings, makeStreamRecord(&a[0], (uint32)a.size()));
	queues.push(StreamStrings, makeStreamRecord(&b[0], (uint32)b.size()));
	queues.push(StreamStrings, makeStreamRecord(&c[0], (uint32)c.size()));
	std::vector<uint8> other = message(20, 9);
	queues.push(StreamGateway, makeStreamRecord(&other[0], (uint32)other.size()));

	std::vector<uint8> block(4096);
	if (startedBytes)
	{
		uint32 got = queues.take(StreamStrings, &block[0], startedBytes);
		CHECK(got == startedBytes);
		client.feed(&block[0], got);
	}
	size_t receivedBefore = client.Received.size();

	// QUIT: the frontend ends the session, the client too (in either order, the stream bytes come later)
	queues.endSession(NbClasses);
	++client.PendingBoundaries;

	// what stays: the rest of a record that was begun, and the mark; in the other classes, the mark alone
	uint32 recordSize = 2 + 1000; // (a length of 1000 takes 2 bytes)
	uint32 rest = 0;
	if (startedBytes != 0 && startedBytes < recordSize)
		rest = recordSize - startedBytes;
	else if (startedBytes > recordSize)
		rest = (1 + 50) - (startedBytes - recordSize); // the second record was begun
	CHECK(queues.pendingBytes(StreamStrings) == rest + 1);
	CHECK(queues.pendingBytes(StreamGateway) == 1);
	CHECK(queues.pendingBytes(StreamChat) == 1);
	CHECK(queues.pendingBytes(StreamLogin) == 1);
	CHECK(queues.pendingBytes() == rest + 4);

	// The new session
	std::vector<uint8> d = message(33, 4), f = message(400, 5);
	queues.push(StreamStrings, makeStreamRecord(&d[0], (uint32)d.size()));
	queues.push(StreamStrings, makeStreamRecord(&f[0], (uint32)f.size()));

	while (queues.pendingBytes(StreamStrings) != 0)
	{
		uint32 got = queues.take(StreamStrings, &block[0], 1 + rnd() % 300);
		client.feed(&block[0], got);
	}
	CHECK(!client.Failed);
	CHECK(client.PendingBoundaries == 0);
	CHECK(client.UnexpectedBoundaries == 0);
	// Before the end, the client got the records that were complete; after it, those of the new session only
	CHECK(client.Received.size() == receivedBefore + 2);
	if (client.Received.size() == receivedBefore + 2)
	{
		CHECK(client.Received[receivedBefore] == d);
		CHECK(client.Received[receivedBefore + 1] == f);
	}
	if (startedBytes > recordSize)
	{
		CHECK(receivedBefore == 1 && client.Received[0] == a);
	}
	else
	{
		CHECK(receivedBefore == (startedBytes == recordSize ? 1u : 0u));
	}

	// Two sessions end before the client reads on: it waits for both marks
	queues.endSession(NbClasses);
	++client.PendingBoundaries;
	std::vector<uint8> g = message(10, 6);
	queues.push(StreamStrings, makeStreamRecord(&g[0], (uint32)g.size()));
	queues.endSession(NbClasses);
	++client.PendingBoundaries;
	std::vector<uint8> h = message(11, 7);
	queues.push(StreamStrings, makeStreamRecord(&h[0], (uint32)h.size()));
	size_t before = client.Received.size();
	while (queues.pendingBytes(StreamStrings) != 0)
	{
		uint32 got = queues.take(StreamStrings, &block[0], 1 + rnd() % 7);
		client.feed(&block[0], got);
	}
	CHECK(client.PendingBoundaries == 0 && client.UnexpectedBoundaries == 0 && !client.Failed);
	CHECK(client.Received.size() == before + 1 && client.Received.back() == h);

	queues.clear();
	CHECK(queues.pendingBytes() == 0 && queues.pendingBytes(StreamGateway) == 0);

	// A connection that never sent anything gets the marks all the same
	CStreamQueues fresh;
	fresh.endSession(NbClasses);
	CHECK(fresh.pendingBytes() == NbClasses && fresh.pendingBytes(StreamLogin) == 1 && fresh.pendingBytes(NbClasses) == 0);
}

// The frontend leaves room for the send time in the header and writes it when the datagram goes
TEST(DatagramHeader, SendTimeIsPokedIntoItsRoom)
{
	const uint16 times[] = { 0x0000, 0x0001, 0x8000, 0xBEEF, 0xFFFF, 0x00FF, 0xFF00 };
	for (uint i = 0; i < sizeof(times) / sizeof(times[0]); ++i)
	{
		NLMISC::CBitMemStream out;
		uint32 number = 0xA1B2C3D4;
		bool systemMode = (i & 1) != 0;
		bool pathChanged = (i & 2) != 0;
		uint8 code = 0x5A;
		uint32 after = 0x11223344;
		out.serial(number);
		out.serialBit(systemMode);
		CHECK((uint)out.getPosInBit() == DatagramSendTimeBitPos);
		out.reserveBits(DatagramSendTimeBits);
		out.serialBit(pathChanged);
		out.serial(code);
		out.serial(after);
		out.poke(times[i], DatagramSendTimeBitPos, DatagramSendTimeBits);

		NLMISC::CBitMemStream in(true);
		memcpy(in.bufferToFill(out.length()), out.buffer(), out.length());
		uint32 number2 = 0, after2 = 0;
		bool systemMode2 = false, pathChanged2 = false;
		uint16 sendTime2 = 0;
		uint8 code2 = 0;
		in.serial(number2);
		in.serial(systemMode2);
		in.serial(sendTime2);
		in.serial(pathChanged2);
		in.serial(code2);
		in.serial(after2);
		CHECK(number2 == number && systemMode2 == systemMode && sendTime2 == times[i] && pathChanged2 == pathChanged && code2 == code && after2 == after);
	}
}

// The client's measure of how late the datagrams arrive
TEST(DatagramLateness, Measure)
{
	CDatagramLateness l;
	CHECK(l.report() == 0 && l.last() == 0);

	// The frontend's clock and the client's have nothing in common; both wrap during the test
	uint32 serverTime = 65536u * 7 - 3000; // its low 16 bits wrap after 3 s
	const uint32 offset = 0xFFFFE000u; // the client's clock wraps after about 8 s
	uint32 travel = 30;

	// A steady path: nothing is late
	for (uint i = 0; i < 100; ++i, serverTime += 100)
	{
		l.add((uint16)serverTime, serverTime + offset + travel, false);
		CHECK(l.last() == 0 && l.report() == 0);
	}

	// One datagram held up for 80 ms: it is late, the report (the smallest of three) is not
	l.add((uint16)serverTime, serverTime + offset + travel + 80, false);
	serverTime += 100;
	CHECK(l.last() == 80 && l.report() == 0);
	l.add((uint16)serverTime, serverTime + offset + travel, false);
	serverTime += 100;
	CHECK(l.last() == 0 && l.report() == 0);

	// A queue: three in a row
	const uint32 late[] = { 50, 70, 60 };
	for (uint i = 0; i < 3; ++i, serverTime += 100)
		l.add((uint16)serverTime, serverTime + offset + travel + late[i], false);
	CHECK(l.last() == 60 && l.report() == 50);

	// More than the frontend decides on: reported in steps of 8 ms up to a little over a second
	for (uint i = 0; i < 3; ++i, serverTime += 100)
		l.add((uint16)serverTime, serverTime + offset + travel + 400 + i, false);
	CHECK(l.last() == 402 && l.report() == 400);
	CHECK(decodeDatagramLateness(encodeDatagramLateness(l.report())) == 400);
	for (uint i = 0; i < 3; ++i, serverTime += 100)
		l.add((uint16)serverTime, serverTime + offset + travel + 5000, false);
	CHECK(l.report() == 2000);
	CHECK(decodeDatagramLateness(encodeDatagramLateness(l.report())) == DatagramLatenessMax);
	// the report in its byte: exact where the frontend decides, never above the real value, never going down as
	// the lateness goes up
	{
		uint32 previous = 0;
		for (uint32 ms = 0; ms <= 3000; ++ms)
		{
			const uint32 decoded = decodeDatagramLateness(encodeDatagramLateness(ms));
			CHECK(decoded <= ms && decoded >= previous);
			CHECK(ms >= 128 || decoded == ms);
			CHECK(ms > DatagramLatenessMax || ms - decoded < 8);
			previous = decoded;
		}
		CHECK(previous == DatagramLatenessMax && DatagramLatenessMax == 1144);
	}

	// A datagram that travels faster than any before becomes the measure
	l.add((uint16)serverTime, serverTime + offset + travel - 10, false);
	serverTime += 100;
	CHECK(l.last() == 0);
	l.add((uint16)serverTime, serverTime + offset + travel, false);
	serverTime += 100;
	CHECK(l.last() == 10);
	travel -= 10;

	// The path gets 40 ms longer for good: late at first, and no more once the periods of the shorter path are over
	travel += 40;
	uint lateCount = 0, steps = 0;
	for (; steps < 600; ++steps, serverTime += 100) // a minute
	{
		l.add((uint16)serverTime, serverTime + offset + travel, false);
		if (l.last() != 0)
		{
			CHECK(l.last() == 40);
			++lateCount;
		}
	}
	CHECK(l.last() == 0);
	CHECK(lateCount >= 300 && lateCount <= 400); // between 30 and 40 seconds

	// The frontend says that the path changed: the longer path is the measure at once
	travel += 100;
	l.add((uint16)serverTime, serverTime + offset + travel, true);
	serverTime += 100;
	CHECK(l.last() == 0);
	l.add((uint16)serverTime, serverTime + offset + travel + 5, false);
	serverTime += 100;
	CHECK(l.last() == 5);

	// Nothing for two minutes (a loading screen that does not read the network counts no datagram late either)
	serverTime += 120000;
	l.add((uint16)serverTime, serverTime + offset + travel + 20, false);
	CHECK(l.last() == 0);

	// A client clock that runs 200 ppm fast: the lateness it adds stays small
	l.reset();
	uint maxLate = 0;
	for (uint i = 0; i < 3000; ++i, serverTime += 100) // five minutes
	{
		uint32 drift = (i * 100) / 5000; // 1 ms every 5 s
		l.add((uint16)serverTime, serverTime + offset + travel + drift, false);
		if (l.last() > maxLate)
			maxLate = l.last();
	}
	CHECK(maxLate <= 8);
}

TEST(StreamQueues, SessionEnd)
{
	sessionEnd(0); // nothing of the session was begun
	sessionEnd(1); // inside the length of the first record
	sessionEnd(2); // its length was sent, nothing of the message
	sessionEnd(500); // inside the first record
	sessionEnd(1002); // exactly the first record
	sessionEnd(1010); // inside the second record
}

// What a client gets in a cycle is shared between its classes by their weights, whatever the sizes of the records
TEST(StreamQueues, ClassesShareAGrant)
{
	std::vector<uint8> big = message(200000, 1);
	std::vector<uint8> small = message(40, 2);
	std::vector<uint32> weights;
	weights.push_back(2); // strings
	weights.push_back(1); // chat
	weights.push_back(2); // login
	std::vector<uint32> shares;
	std::vector<uint8> scratch(70000);

	// All three have more than they get: 2 to 1 to 2, to the byte over many cycles of an odd size
	{
		CStreamQueues queues;
		for (uint c = 0; c < 3; ++c)
			queues.push(c, makeStreamRecord(&big[0], (uint32)big.size()));
		uint64 total[3] = { 0, 0, 0 };
		for (uint t = 0; t < 1000; ++t)
		{
			queues.share(101, weights, shares);
			ASSERT_EQ(shares.size(), (size_t)3);
			EXPECT_EQ(shares[0] + shares[1] + shares[2], 101u);
			for (uint c = 0; c < 3; ++c)
			{
				EXPECT_EQ(queues.take(c, &scratch[0], shares[c]), shares[c]);
				total[c] += shares[c];
			}
		}
		EXPECT_NEAR((double)total[0], 101000.0 * 2 / 5, 400.0);
		EXPECT_NEAR((double)total[1], 101000.0 * 1 / 5, 400.0);
		EXPECT_NEAR((double)total[2], 101000.0 * 2 / 5, 400.0);
	}

	// A class that wants less than its part leaves the rest to the others, and one that has nothing gets nothing
	{
		CStreamQueues queues;
		queues.push(0, makeStreamRecord(&big[0], (uint32)big.size()));
		queues.push(1, makeStreamRecord(&small[0], (uint32)small.size()));
		queues.share(1000, weights, shares);
		const uint32 smallRecord = queues.pendingBytes(1);
		ASSERT_GE(shares.size(), (size_t)2);
		EXPECT_EQ(shares[1], smallRecord);
		EXPECT_EQ(shares[0], 1000 - smallRecord);
		if (shares.size() > 2)
			EXPECT_EQ(shares[2], 0u);
	}

	// More than all that waits: all of it
	{
		CStreamQueues queues;
		queues.push(0, makeStreamRecord(&small[0], (uint32)small.size()));
		queues.push(2, makeStreamRecord(&small[0], (uint32)small.size()));
		const uint32 pending = queues.pendingBytes();
		queues.share(60000, weights, shares);
		ASSERT_EQ(shares.size(), (size_t)3);
		EXPECT_EQ(shares[0] + shares[1] + shares[2], pending);
		EXPECT_EQ(shares[0], queues.pendingBytes(0));
		EXPECT_EQ(shares[2], queues.pendingBytes(2));
	}

	// Without weights every class has 1; nothing waits, nothing is shared
	{
		CStreamQueues queues;
		queues.share(500, std::vector<uint32>(), shares);
		EXPECT_TRUE(shares.empty());
		for (uint c = 0; c < 2; ++c)
			queues.push(c, makeStreamRecord(&big[0], (uint32)big.size()));
		queues.share(500, std::vector<uint32>(), shares);
		ASSERT_EQ(shares.size(), (size_t)2);
		EXPECT_EQ(shares[0], 250u);
		EXPECT_EQ(shares[1], 250u);
	}
}

TEST(StreamQueues, ClassWeightsAsWritten)
{
	std::vector<std::string> names;
	names.push_back("strings");
	names.push_back("login");
	std::vector<uint32> weights;
	std::vector<std::string> faults;
	parseStreamClassWeights("strings=2 chat=1 login=3 gateway=2", names, weights, faults);
	ASSERT_EQ(weights.size(), (size_t)2);
	EXPECT_EQ(weights[0], 2u);
	EXPECT_EQ(weights[1], 3u);
	EXPECT_EQ(faults.size(), (size_t)2); // (two classes that this msg.xml does not have)
	parseStreamClassWeights("", names, weights, faults);
	EXPECT_EQ(weights[0], 1u);
	EXPECT_EQ(weights[1], 1u);
	EXPECT_TRUE(faults.empty());
	parseStreamClassWeights("login=0  strings login=x =4", names, weights, faults);
	EXPECT_EQ(weights[0], 1u);
	EXPECT_EQ(weights[1], 1u);
	EXPECT_EQ(faults.size(), (size_t)3);
}

} /* anonymous namespace */

/* end of file */
