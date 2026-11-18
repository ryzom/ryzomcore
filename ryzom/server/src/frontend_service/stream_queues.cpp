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

#include "stdpch.h"
#include "stream_queues.h"

#include <algorithm>
#include <cstring>

using namespace CLFECOMMON;

TStreamRecord makeStreamRecord(const uint8 *message, uint32 length)
{
	nlassert(length != 0 && length <= StreamRecordLengthMax);
	uint8 prefix[StreamRecordLengthMaxBytes];
	uint prefixSize = writeStreamRecordLength(prefix, length);
	std::shared_ptr<std::vector<uint8>> record = std::make_shared<std::vector<uint8>>();
	record->reserve(prefixSize + length);
	record->insert(record->end(), prefix, prefix + prefixSize);
	record->insert(record->end(), message, message + length);
	return record;
}

namespace /* anonymous */ {

/// The record of length 0, which marks the end of a session
const TStreamRecord &boundaryRecord()
{
	static const TStreamRecord s_Record = std::make_shared<const std::vector<uint8>>(1, (uint8)0);
	return s_Record;
}

} /* anonymous namespace */

CStreamQueues::CStreamQueues()
    : Refusing(false)
    , m_PendingBytes(0)
{
}

void CStreamQueues::push(TStreamClass streamClass, const TStreamRecord &record)
{
	nlassert((uint)streamClass < (uint)NbStreamClasses);
	CQueue &queue = m_Queues[streamClass];
	queue.Records.push_back(record);
	queue.PendingBytes += (uint32)record->size();
	m_PendingBytes += (uint32)record->size();
}

uint32 CStreamQueues::take(TStreamClass streamClass, uint8 *dst, uint32 maxBytes)
{
	nlassert((uint)streamClass < (uint)NbStreamClasses);
	CQueue &queue = m_Queues[streamClass];
	uint32 taken = 0;
	while (taken < maxBytes && !queue.Records.empty())
	{
		const std::vector<uint8> &record = *queue.Records.front();
		uint32 size = std::min(maxBytes - taken, (uint32)record.size() - queue.FrontOffset);
		memcpy(dst + taken, &record[queue.FrontOffset], size);
		taken += size;
		queue.FrontOffset += size;
		if (queue.FrontOffset == record.size())
		{
			queue.Records.pop_front();
			queue.FrontOffset = 0;
		}
	}
	queue.PendingBytes -= taken;
	m_PendingBytes -= taken;
	return taken;
}

void CStreamQueues::endSession()
{
	for (uint i = 0; i < (uint)NbStreamClasses; ++i)
	{
		CQueue &queue = m_Queues[i];

		// A record that was begun stays, for what is left of it, and so do the marks of the sessions that ended
		// before this one and were not sent yet: the client waits for each of them
		std::deque<TStreamRecord> kept;
		uint32 keptBytes = 0;
		for (size_t r = 0; r < queue.Records.size(); ++r)
		{
			const TStreamRecord &record = queue.Records[r];
			bool begun = (r == 0 && queue.FrontOffset != 0);
			if (begun || record->size() == 1) // (the mark is the only record of one byte)
			{
				kept.push_back(record);
				keptBytes += (uint32)record->size() - (begun ? queue.FrontOffset : 0);
			}
		}
		queue.Records.swap(kept);
		m_PendingBytes -= queue.PendingBytes - keptBytes;
		queue.PendingBytes = keptBytes;

		push((TStreamClass)i, boundaryRecord());
	}
}

void CStreamQueues::clear()
{
	for (uint i = 0; i < (uint)NbStreamClasses; ++i)
	{
		CQueue &queue = m_Queues[i];
		std::deque<TStreamRecord>().swap(queue.Records);
		queue.FrontOffset = 0;
		queue.PendingBytes = 0;
	}
	m_PendingBytes = 0;
}

/* end of file */
