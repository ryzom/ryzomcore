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

#ifndef NL_STREAM_QUEUES_H
#define NL_STREAM_QUEUES_H

#include "nel/misc/types_nl.h"

#include <string>

#include <deque>
#include <memory>
#include <vector>

#include "game_share/stream_record.h"

/// One record of a stream (game_share/stream_record.h): the length, then the message. The queues of several clients
/// can hold the same record, for a message that goes to all of them (so far each gets a record of its own).
typedef std::shared_ptr<const std::vector<uint8>> TStreamRecord;

/// The record of a message (length above 0 and at most CLFECOMMON::StreamRecordLengthMax)
TStreamRecord makeStreamRecord(const uint8 *message, uint32 length);

/**
 * The records that wait for the streams of one QUIC connection of protocol level ryzomcore/4.2, one queue per class of
 * messages (a class is known by its number, see game_share/stream_record.h). The frontend takes bytes out of them as
 * the connection is allowed to send, without regard for where a record ends: the next bytes go on with the same record.
 *
 * The queues belong to the connection, not to the session on it: a connection kept for the next session (limbo mode)
 * keeps what was begun, see endSession().
 *
 * Only the main thread of the service uses it.
 */
class CStreamQueues
{
public:
	CStreamQueues();

	/// Add a record at the end of the queue of a class
	void push(uint streamClass, const TStreamRecord &record);

	/// Bytes that wait, in all classes
	uint32 pendingBytes() const { return m_PendingBytes; }

	/// Bytes that wait in one class
	uint32 pendingBytes(uint streamClass) const { return streamClass < m_Queues.size() ? m_Queues[streamClass].PendingBytes : 0; }

	/// Take the next bytes of a class out, at most maxBytes of them. Returns how many were written to dst.
	uint32 take(uint streamClass, uint8 *dst, uint32 maxBytes);

	/// Share what a client gets in a cycle between its classes: each class with bytes waiting gets a part by its
	/// weight (weights[class]; 1 without one), and what a class does not need goes to the others. shares gets a value
	/// per class; they add up to bytes, or to all that waits if that is less. Whatever the sizes of the records and of
	/// the grants, the classes get their parts: a class cannot be starved by another, and a large record in one class
	/// does not hold the others up.
	void share(uint32 bytes, const std::vector<uint32> &weights, std::vector<uint32> &shares);

	/// The session on the connection ends and the connection is kept: the records of which nothing was taken yet are
	/// dropped, a record that was begun stays (the stream cannot be cut inside a record), and each of the nbClasses
	/// classes of the frontend gets the record that marks the end of the session, before which the client drops what
	/// it reads. The marks of earlier sessions that still wait stay too: the client counts them.
	void endSession(uint nbClasses);

	/// The connection is closed: drop everything
	void clear();

	/// Once set, the connection is being closed for a fault of its streams and takes no more records
	bool Refusing;

private:
	struct CQueue
	{
		CQueue()
		    : FrontOffset(0)
		    , PendingBytes(0)
		{
		}
		std::deque<TStreamRecord> Records;
		uint32 FrontOffset; // bytes of the first record that were taken
		uint32 PendingBytes;
	};

	std::vector<CQueue> m_Queues; // by class, as far as one was used
	uint32 m_PendingBytes;
	uint32 m_ShareCursor; // the class that gets the first of the bytes a division leaves over
};

/// The weights of the classes for CStreamQueues::share, out of what an operator writes: "name=weight" for some of
/// the classes, with spaces between them (StreamClassWeights of the frontend service). A class that is not named has
/// 1, and so has one with a weight of 0. faults gets a text for each part that is not that, or names no class.
void parseStreamClassWeights(const std::string &text, const std::vector<std::string> &classNames, std::vector<uint32> &weights, std::vector<std::string> &faults);

#endif // NL_STREAM_QUEUES_H

/* end of file */
