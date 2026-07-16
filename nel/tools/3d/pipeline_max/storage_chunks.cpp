/**
 * \file storage_chunks.cpp
 * \brief CStorageChunks
 * \date 2012-08-18 09:20GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7
 * \author Claude Sonnet 5
 * CStorageChunks
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
#include "storage_chunks.h"

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

// #define NL_DEBUG_STORAGE

namespace PIPELINE {
namespace MAX {

CStorageChunks::CStorageChunks(NLMISC::IStream &stream, sint64 size) : m_Stream(stream), m_Is64Bit(false)
{
	if (size >= 2147483647L)
		throw NLMISC::EStream("64bit chunks not supported");
	m_Chunks.reserve(64);
	m_Chunks.resize(1);
	m_Chunks[0].HeaderSize = 0;
	m_Chunks[0].OffsetBegin = stream.getPos();
	if (stream.isReading())
	{
		m_Chunks[0].Id = 0;
		m_Chunks[0].Size = 0x80000000 | (uint32)(size);
	}
	else
	{
		m_Chunks[0].Id = 1;
		m_Chunks[0].Size = 0;
	}
}

CStorageChunks::~CStorageChunks()
{
#ifdef NL_DEBUG_STORAGE
	if (m_Chunks.size() != 1)
		nldebug("Not all chunks were closed");
#endif
}

bool CStorageChunks::enterChunk()
{
	if (m_Stream.isReading())
	{
		// input logic
		if (!isChunkContainer())
		{
#ifdef NL_DEBUG_STORAGE
			nldebug("Current chunk is not a container, cannot enter");
#endif
			return false;
		}
		if (endOfChunk())
		{
#ifdef NL_DEBUG_STORAGE
			nldebug("End of chunk, cannot enter");
#endif
			return false;
		}
		m_Chunks.resize(m_Chunks.size() + 1);
		CChunk *chunk = currentChunk();
		chunk->OffsetBegin = m_Stream.getPos();
		m_Stream.serial(chunk->Id);
		m_Stream.serial(chunk->Size);
		chunk->HeaderSize = 6;
		if (chunk->Size == 0)
		{
			// this is a 64bit chunk
			uint64 size64;
			m_Stream.serial(size64);
			chunk->HeaderSize += 8;
			bool iscont = (size64 & 0x8000000000000000) == 0x8000000000000000;
			size64 &= 0x7FFFFFFFFFFFFFFF;
			if (size64 >= 2147483647L)
				throw NLMISC::EStream("64bit chunks not supported");
			// downgrade to 32 bit chunk
			chunk->Size = (uint32)size64;
			if (iscont) chunk->Size |= 0x80000000;
			m_Is64Bit = true; // it's true
		}
#ifdef NL_DEBUG_STORAGE
		nldebug("Entered reading chunk of size %i", chunk->Size);
#endif
		return true;
	}
	else
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("No input, this function cannot output, throw exception");
#endif
		throw NLMISC::EStream();
	}
}

bool CStorageChunks::enterChunk(uint16 id, bool container, bool as64Bit)
{
	if (!m_Stream.isReading())
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("Writing, enter chunk");
#endif

		// enter the new chunk
		m_Chunks.resize(m_Chunks.size() + 1);
		CChunk *chunk = currentChunk();
		chunk->Id = container ? 1 : 0; // reused: bit indicates container-flag for leaveChunk
		chunk->OffsetBegin = m_Stream.getPos(); // store current pos
		chunk->HeaderSize = as64Bit ? 14 : 6; // per-chunk width, patched in leaveChunk

		// write header
		m_Stream.serial(id); // write the id
		if (as64Bit)
		{
			uint32 zero = 0;
			uint64 sizeDummy64 = 0xFFFFFFFFFFFFFFFFULL;
			m_Stream.serial(zero); // marker: size32==0 means read the size64 next
			m_Stream.serial(sizeDummy64); // 8-byte size placeholder
		}
		else
		{
			uint32 sizeDummy = 0xFFFFFFFF;
			m_Stream.serial(sizeDummy); // 4-byte size placeholder
		}
		return true;
	}
	else // input or exception
	{
		while (enterChunk())
		{
			if (getChunkId() == id)
				return true;
			leaveChunk(); // skip data
		}
		return false;
	}
}

sint32 CStorageChunks::leaveChunk()
{
	if (m_Stream.isReading())
	{
		// input logic
		sint32 skipped = currentChunk()->endOfChunk() - m_Stream.getPos();
		if (skipped)
		{
			m_Stream.seek(currentChunk()->endOfChunk(), NLMISC::IStream::begin);
#ifdef NL_DEBUG_STORAGE
			nldebug("Skipped %i bytes in the current chunk", skipped);
#endif
		}
		m_Chunks.resize(m_Chunks.size() - 1);
		return skipped;
	}
	else
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("Writing, leave chunk");
#endif
		sint32 pos = m_Stream.getPos();
		sint32 sizeWithHeader = pos - currentChunk()->OffsetBegin;
		bool isContainer = currentChunk()->Id != 0;
		if (currentChunk()->HeaderSize == 14)
		{
			// 64-bit header: id(2) + zero32(4) + size64(8)
			sint32 sizePos = currentChunk()->OffsetBegin + 6;
			m_Stream.seek(sizePos, NLMISC::IStream::begin);
			uint64 sizeField64 = (uint64)sizeWithHeader | (isContainer ? 0x8000000000000000ULL : 0);
			m_Stream.serial(sizeField64);
			m_Stream.seek(pos, NLMISC::IStream::begin);
		}
		else
		{
			// 32-bit header: id(2) + size32(4)
			sint32 sizePos = currentChunk()->OffsetBegin + 2;
			m_Stream.seek(sizePos, NLMISC::IStream::begin);
			uint32 sizeField = (uint32)sizeWithHeader | (isContainer ? 0x80000000u : 0);
			m_Stream.serial(sizeField);
			m_Stream.seek(pos, NLMISC::IStream::begin);
		}
		m_Chunks.resize(m_Chunks.size() - 1);
#ifdef NL_DEBUG_STORAGE
		nldebug("Size: %i", sizeWithHeader);
#endif
		return sizeWithHeader;
	}
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
