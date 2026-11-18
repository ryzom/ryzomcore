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

#ifndef RY_STREAM_RECORD_H
#define RY_STREAM_RECORD_H

#include "nel/misc/types_nl.h"

#include <vector>

/*
 * Protocol level ryzomcore/4.2: besides its datagrams, the frontend sends some classes of messages on reliable QUIC
 * streams, one unidirectional stream per class, opened by the frontend.
 *
 * The classes are the frontend's own. Its msg.xml declares them (`stream_class` elements, in the order of their
 * streams) and says which messages belong to each (the `stream` attribute of a branch or a leaf). The client knows
 * none of that: it lets the frontend open up to StreamClassMax streams and treats them all alike, so a frontend can
 * add a class, or move a message to another class or back to the datagrams, without a new client program or protocol
 * level (the client's copy of msg.xml has to follow, as for any change to that file: the frontend sends its signature
 * to the clients). The level fixes the framing below, the mark of the end of a session, and StreamClassMax.
 *
 * The messages of one stream reach their handlers in the order the frontend sent them. In one update, the client
 * goes through the streams in the order the frontend opened them, so what the messages of other streams refer to
 * (the strings) belongs in the first class.
 *
 * A stream is a sequence of records. A record is a length (unsigned LEB128, at most 4 bytes) followed by that many
 * bytes, which are the message as an impulse carries it: the message name bits of msg.xml, then the fields.
 *
 * A record of length 0 is no message (a message is never empty, it starts with its name bits). It marks the end of a
 * session on a connection that is kept for the next session: the frontend sends one in every stream when the session
 * ends, and the client drops the records it reads before it.
 *
 * The datagrams of the level carry what the frontend needs to keep the streams from holding them up:
 * - Every datagram of the frontend has, after its number (32 bits) and its system bit, the time the frontend sent it
 *   (16 bits: its clock in milliseconds, modulo 2^16) and one bit that says that the path to the client changed.
 * - Every normal (not system) datagram of the client has, after its acknowledgement bits, how late the datagrams of
 *   the frontend arrive (8 bits: milliseconds, at most 255; see CDatagramLateness in the client).
 */

namespace CLFECOMMON {

/// Most streams a client of protocol level ryzomcore/4.2 lets the frontend open, which is the most classes of
/// messages a frontend can have. A class is known by its number, from 0, in the order the frontend declares them.
const uint StreamClassMax = 16;

/// The number of no class: a message that goes by impulse, a stream that is not one of a class
const uint8 NoStreamClass = 0xFF;

/// QUIC stream id of a class: the unidirectional streams a server opens are 3, 7, 11, 15 and so on, in the order it
/// opens them
inline uint64 streamIdOfClass(uint streamClass)
{
	return 3 + 4 * (uint64)streamClass;
}

/// Class of a QUIC stream id, NoStreamClass if it is not the stream of a class
inline uint8 streamClassOfId(uint64 streamId)
{
	if ((streamId & 3) != 3 || (streamId >> 2) >= (uint64)StreamClassMax)
		return NoStreamClass;
	return (uint8)(streamId >> 2);
}

/// Where the send time is in a datagram of the frontend, in bits from its start, and how many bits it is
const uint DatagramSendTimeBitPos = 33;
const uint DatagramSendTimeBits = 16;

/// Most bytes the length of a record takes
const uint StreamRecordLengthMaxBytes = 4;

/// Largest length that fits those bytes
const uint32 StreamRecordLengthMax = (1 << (7 * StreamRecordLengthMaxBytes)) - 1;

/// Largest record a client accepts (above what a frontend sends, see its StreamRecordMaxBytes)
const uint32 StreamRecordClientMaxBytes = 16 * 1024 * 1024;

/// Write the length of a record (at most StreamRecordLengthMax) to dst, which has room for StreamRecordLengthMaxBytes.
/// Returns the number of bytes written.
uint writeStreamRecordLength(uint8 *dst, uint32 length);

/**
 * Takes the bytes of one stream as they arrive and gives the records back one at a time.
 * Not thread-safe.
 */
class CStreamRecordReader
{
public:
	enum TResult
	{
		/// The next record is not complete yet
		NoRecord,
		/// A message
		Record,
		/// The record of length 0 that marks the end of a session
		Boundary,
		/// The length does not end within StreamRecordLengthMaxBytes or is above the limit: the stream cannot be read
		/// any further, every later call returns this again
		BadLength
	};

	CStreamRecordReader();

	/// Drop everything, for a new stream
	void clear();

	/// Add bytes received on the stream
	void push(const uint8 *data, uint32 size);

	/// Take the next record out. For a Record, data and length are its bytes, valid until the next call of push() or
	/// clear(). For BadLength, length is the length read if it was above maxLength, 0 if the length did not end.
	TResult next(const uint8 *&data, uint32 &length, uint32 maxLength);

	/// Bytes received and not taken out yet
	uint32 bufferedBytes() const { return (uint32)(m_Buffer.size() - m_Pos); }

private:
	std::vector<uint8> m_Buffer;
	size_t m_Pos; // start of the next record in m_Buffer
};

} // CLFECOMMON

#endif // RY_STREAM_RECORD_H

/* end of file */
