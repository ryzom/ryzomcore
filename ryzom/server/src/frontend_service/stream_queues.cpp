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

// (this file and stream_scheduler.cpp are a library of their own, which the tests link too: no stdpch.h)
#include "stream_queues.h"

#include "nel/misc/debug.h"
#include "nel/misc/string_common.h"
#include "nel/misc/common.h"
#include "nel/misc/algo.h"

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
    , m_ShareCursor(0)
{
}

void CStreamQueues::push(uint streamClass, const TStreamRecord &record)
{
	nlassert(streamClass < StreamClassMax);
	if (m_Queues.size() <= streamClass)
		m_Queues.resize(streamClass + 1);
	CQueue &queue = m_Queues[streamClass];
	queue.Records.push_back(record);
	queue.PendingBytes += (uint32)record->size();
	m_PendingBytes += (uint32)record->size();
}

uint32 CStreamQueues::take(uint streamClass, uint8 *dst, uint32 maxBytes)
{
	if (streamClass >= m_Queues.size())
		return 0;
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

void CStreamQueues::endSession(uint nbClasses)
{
	nlassert(nbClasses <= StreamClassMax);
	if (m_Queues.size() < nbClasses)
		m_Queues.resize(nbClasses);
	for (uint i = 0; i < (uint)m_Queues.size(); ++i)
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

		push(i, boundaryRecord());
	}
}

void CStreamQueues::share(uint32 bytes, const std::vector<uint32> &weights, std::vector<uint32> &shares)
{
	const size_t nbClasses = m_Queues.size();
	shares.assign(nbClasses, 0);
	uint32 left = std::min(bytes, m_PendingBytes);
	while (left != 0)
	{
		// The classes that still want some, and their weights
		uint64 sumWeights = 0;
		for (size_t c = 0; c < nbClasses; ++c)
		{
			if (m_Queues[c].PendingBytes > shares[c])
				sumWeights += (c < weights.size() && weights[c] != 0) ? weights[c] : 1;
		}
		if (sumWeights == 0)
			break;

		// A class that wants no more than its part has it, and what it leaves is shared again
		bool satisfied = false;
		uint32 given = 0;
		for (size_t c = 0; c < nbClasses; ++c)
		{
			const uint32 want = m_Queues[c].PendingBytes - shares[c];
			if (want == 0)
				continue;
			const uint32 weight = (c < weights.size() && weights[c] != 0) ? weights[c] : 1;
			const uint32 part = (uint32)((uint64)left * weight / sumWeights);
			if (part >= want)
			{
				shares[c] += want;
				given += want;
				satisfied = true;
			}
		}
		if (satisfied)
		{
			left -= given;
			continue;
		}

		// All of them want more than their part: each gets it, and the bytes that the division leaves over go one by
		// one from where they started the last time
		for (size_t c = 0; c < nbClasses; ++c)
		{
			if (m_Queues[c].PendingBytes <= shares[c])
				continue;
			const uint32 weight = (c < weights.size() && weights[c] != 0) ? weights[c] : 1;
			const uint32 part = (uint32)((uint64)left * weight / sumWeights);
			shares[c] += part;
			given += part;
		}
		left -= given;
		for (size_t n = 0; n < nbClasses && left != 0; ++n)
		{
			const size_t c = (m_ShareCursor + n) % nbClasses;
			if (m_Queues[c].PendingBytes > shares[c])
			{
				++shares[c];
				--left;
			}
		}
		m_ShareCursor = (uint32)((m_ShareCursor + 1) % nbClasses);
		break;
	}
}

void parseStreamClassWeights(const std::string &text, const std::vector<std::string> &classNames, std::vector<uint32> &weights, std::vector<std::string> &faults)
{
	weights.assign(classNames.size(), 1);
	faults.clear();
	std::vector<std::string> parts;
	NLMISC::splitString(text, " ", parts);
	for (size_t i = 0; i < parts.size(); ++i)
	{
		const std::string &part = parts[i];
		if (part.empty())
			continue;
		const std::string::size_type equals = part.find('=');
		uint32 weight = 0;
		if (equals == std::string::npos || equals == 0 || !NLMISC::fromString(part.substr(equals + 1), weight))
		{
			faults.push_back(NLMISC::toString("'%s' is not a class and its weight, as in strings=2", part.c_str()));
			continue;
		}
		const std::string name = part.substr(0, equals);
		size_t c = 0;
		while (c < classNames.size() && classNames[c] != name)
			++c;
		if (c == classNames.size())
		{
			faults.push_back(NLMISC::toString("'%s' is not a stream class of msg.xml", name.c_str()));
			continue;
		}
		weights[c] = weight != 0 ? weight : 1;
	}
}

void CStreamQueues::clear()
{
	std::vector<CQueue>().swap(m_Queues);
	m_PendingBytes = 0;
}

/* end of file */
