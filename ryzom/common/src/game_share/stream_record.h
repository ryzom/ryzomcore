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
 * streams, one unidirectional stream per class, opened by the frontend. The classes and their order, and with them the
 * stream ids, are constants of the protocol level. msg.xml says which messages belong to a class (the `stream`
 * attribute of a branch or a leaf); only the frontend reads that.
 *
 * A stream is a sequence of records. A record is a length (unsigned LEB128, at most 4 bytes) followed by that many
 * bytes, which are the message as an impulse carries it: the message name bits of msg.xml, then the fields.
 *
 * A record of length 0 is no message (a message is never empty, it starts with its name bits). It marks the end of a
 * session on a connection that is kept for the next session: the frontend sends one in every stream when the session
 * ends, and the client drops the records it reads before it.
 */

namespace CLFECOMMON {

/// Message classes of protocol level ryzomcore/4.2, in the order the frontend opens their streams
enum TStreamClass
{
	StreamStrings = 0,
	StreamChat,
	StreamLogin,
	StreamGateway,
	NbStreamClasses,
	NoStreamClass = 0xFF
};

/// Name of a class in msg.xml, nullptr if it is none
const char *streamClassName(TStreamClass streamClass);

/// Class of a name of msg.xml, NoStreamClass if no class has that name
TStreamClass streamClassFromName(const char *name);

/// QUIC stream id of a class: the unidirectional streams a server opens are 3, 7, 11, 15, in the order it opens them
inline uint64 streamIdOfClass(TStreamClass streamClass)
{
	return 3 + 4 * (uint64)streamClass;
}

/// Class of a QUIC stream id, NoStreamClass if it is not the stream of a class
inline TStreamClass streamClassOfId(uint64 streamId)
{
	if ((streamId & 3) != 3 || (streamId >> 2) >= (uint64)NbStreamClasses)
		return NoStreamClass;
	return (TStreamClass)(streamId >> 2);
}

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
