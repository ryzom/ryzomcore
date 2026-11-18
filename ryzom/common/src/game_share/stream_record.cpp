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
#include "stream_record.h"

#include <cstring>

namespace CLFECOMMON {

uint writeStreamRecordLength(uint8 *dst, uint32 length)
{
	nlassert(length <= StreamRecordLengthMax);
	uint size = 0;
	do
	{
		uint8 byte = (uint8)(length & 0x7F);
		length >>= 7;
		if (length != 0)
			byte |= 0x80;
		dst[size++] = byte;
	} while (length != 0);
	return size;
}

CStreamRecordReader::CStreamRecordReader()
    : m_Pos(0)
{
}

void CStreamRecordReader::clear()
{
	std::vector<uint8>().swap(m_Buffer);
	m_Pos = 0;
}

void CStreamRecordReader::push(const uint8 *data, uint32 size)
{
	// Drop what was taken out: all of it once nothing is left, else once it is the larger part
	if (m_Pos == m_Buffer.size())
	{
		if (m_Buffer.capacity() > 1024 * 1024)
			std::vector<uint8>().swap(m_Buffer); // the memory of a large record is given back
		else
			m_Buffer.clear();
		m_Pos = 0;
	}
	else if (m_Pos >= 65536 && m_Pos >= m_Buffer.size() - m_Pos)
	{
		m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + m_Pos);
		m_Pos = 0;
	}
	m_Buffer.insert(m_Buffer.end(), data, data + size);
}

CStreamRecordReader::TResult CStreamRecordReader::next(const uint8 *&data, uint32 &length, uint32 maxLength)
{
	data = nullptr;
	length = 0;

	// The length
	size_t available = m_Buffer.size() - m_Pos;
	uint32 value = 0;
	uint prefix = 0;
	for (;;)
	{
		if (prefix == StreamRecordLengthMaxBytes)
			return BadLength; // its last byte announces another one
		if (prefix == available)
			return NoRecord;
		uint8 byte = m_Buffer[m_Pos + prefix];
		value |= (uint32)(byte & 0x7F) << (7 * prefix);
		++prefix;
		if (!(byte & 0x80))
			break;
	}
	if (value > maxLength)
	{
		length = value;
		return BadLength;
	}

	if (value == 0)
	{
		m_Pos += prefix;
		return Boundary;
	}

	if (available - prefix < value)
		return NoRecord;

	data = &m_Buffer[m_Pos + prefix];
	length = value;
	m_Pos += prefix + value;
	return Record;
}

} // CLFECOMMON

/* end of file */
