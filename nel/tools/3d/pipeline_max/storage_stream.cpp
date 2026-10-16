/**
 * \file storage_stream.cpp
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 * CStorageStream
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "storage_stream.h"

// STL includes
#include <cstring>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// #define NL_DEBUG_STORAGE

namespace PIPELINE {
namespace MAX {

CStorageStream::CStorageStream(const uint8 *data, size_t size)
    : NLMISC::IStream(true), m_ReadData(data), m_ReadSize(size), m_Pos(0)
{

}

CStorageStream::CStorageStream(const std::vector<uint8> &data)
    : NLMISC::IStream(true), m_ReadData(data.empty() ? nullptr : &data[0]), m_ReadSize(data.size()), m_Pos(0)
{

}

CStorageStream::CStorageStream()
    : NLMISC::IStream(false), m_ReadData(nullptr)
    , m_ReadSize(0), m_Pos(0)
{

}

CStorageStream::~CStorageStream()
{

}

bool CStorageStream::seek(sint32 offset, NLMISC::IStream::TSeekOrigin origin) const
{
	// Size limit for the cursor: the buffer we can seek within. In write mode the cursor may only
	// land inside the already-written span (CStorageChunks back-patches sizes it has written, then
	// returns to the end) — seeking past the end is not how the chunk layer grows the stream
	// (serialBuffer does that).
	size_t limit = isReading() ? m_ReadSize : m_WriteBuffer.size();
	sint64 target;
	switch (origin)
	{
	case begin:
		target = (sint64)offset;
		break;
	case current:
		target = (sint64)m_Pos + (sint64)offset;
		break;
	case end:
		target = (sint64)limit + (sint64)offset;
		break;
	default:
		return false;
	}
	if (target < 0 || target > (sint64)limit)
		return false;
	m_Pos = (size_t)target;
	return true;
}

sint32 CStorageStream::getPos() const
{
	return (sint32)m_Pos;
}

void CStorageStream::serialBuffer(uint8 *buf, uint len)
{
	if (!len)
		return;
	if (isReading())
	{
		if (m_Pos + len > m_ReadSize)
		{
#ifdef NL_DEBUG_STORAGE
			nldebug("Read past end of storage stream, throw exception");
#endif
			throw NLMISC::EStream();
		}
		memcpy(buf, m_ReadData + m_Pos, len);
		m_Pos += len;
	}
	else
	{
		if (m_Pos + len > m_WriteBuffer.size())
			m_WriteBuffer.resize(m_Pos + len);
		memcpy(&m_WriteBuffer[m_Pos], buf, len);
		m_Pos += len;
	}
}

void CStorageStream::serialBit(bool &bit)
{
	uint8 var = (uint8)bit;
	serial(var);
	bit = (bool)var;
}

bool CStorageStream::eof()
{
	if (isReading())
		return m_Pos >= m_ReadSize;
	return false;
}

sint32 CStorageStream::size()
{
	return (sint32)(isReading() ? m_ReadSize : m_WriteBuffer.size());
}

void CStorageStream::swapBuffer(std::vector<uint8> &out)
{
	nlassert(!isReading());
	out.swap(m_WriteBuffer);
	m_WriteBuffer.clear();
	m_Pos = 0;
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
