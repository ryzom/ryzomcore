/**
 * \file storage_stream.cpp
 * \brief CStorageStream
 * \date 2012-08-16 22:06GMT
 * \author Jan Boon (Kaetemi)
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

// 3rd Party includes
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-output-stdio.h>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

// #define NL_DEBUG_STORAGE_STREAM

namespace PIPELINE {
namespace MAX {

CStorageStream::CStorageStream(GsfInput *input) : NLMISC::IStream(true), m_Input(input), m_Output(NULL), m_Is64Bit(false)
{
	m_Chunks.reserve(64);
	m_Chunks.resize(1);
	m_Chunks[0].OffsetBegin = -6;
	m_Chunks[0].Id = 0;
	m_Chunks[0].Size = 0x80000000;
}

CStorageStream::CStorageStream(GsfOutput *output) : NLMISC::IStream(true), m_Input(NULL), m_Output(output), m_Is64Bit(false)
{
	m_Chunks.reserve(64);
	m_Chunks.resize(1);
	m_Chunks[0].OffsetBegin = -6;
	m_Chunks[0].Id = 1; // in write mode, id flags a container
	m_Chunks[0].Size = 0;
}

CStorageStream::~CStorageStream()
{

}

bool CStorageStream::seek(sint32 offset, NLMISC::IStream::TSeekOrigin origin) const
{
	if (m_Input)
	{
		switch (origin)
		{
		case begin:
			return gsf_input_seek(m_Input, offset, G_SEEK_SET);
		case current:
			return gsf_input_seek(m_Input, offset, G_SEEK_CUR);
		case end:
			return gsf_input_seek(m_Input, offset, G_SEEK_END);
		}
	}
	else if (m_Output)
	{
		switch (origin)
		{
		case begin:
			return gsf_output_seek(m_Output, offset, G_SEEK_SET);
		case current:
			return gsf_output_seek(m_Output, offset, G_SEEK_CUR);
		case end:
			return gsf_output_seek(m_Output, offset, G_SEEK_END);
		}
	}
	return NLMISC::IStream::seek(offset, origin);
}

sint32 CStorageStream::getPos() const
{
	if (m_Input)
	{
		gsf_off_t res = gsf_input_tell(m_Input);
		if (res < 2147483647L) // exception when larger
			return (sint32)res;
	}
	else if (m_Output)
	{
		gsf_off_t res = gsf_output_tell(m_Output);
		if (res < 2147483647L) // exception when larger
			return (sint32)res;
	}
	return NLMISC::IStream::getPos();
}

void CStorageStream::serialBuffer(uint8 *buf, uint len)
{
	if (!len)
	{
#ifdef NL_DEBUG_STORAGE_STREAM
		nldebug("Serial 0 size buffer");
#endif
		return;
	}
	if (m_Input)
	{
		if (!gsf_input_read(m_Input, len, buf))
		{
#ifdef NL_DEBUG_STORAGE_STREAM
			nldebug("Cannot read from input, throw exception");
#endif
			throw NLMISC::EStream();
		}
	}
	else if (m_Output)
	{
		if (!gsf_output_write(m_Output, len, buf))
		{
#ifdef NL_DEBUG_STORAGE_STREAM
			nldebug("Cannot write to output, throw exception");
#endif
			throw NLMISC::EStream();
		}
	}
	else
	{
#ifdef NL_DEBUG_STORAGE_STREAM
		nldebug("No input or output, should not happen, throw exception");
#endif
		throw NLMISC::EStream();
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
	if (m_Input)
	{
		return gsf_input_eof(m_Input);
	}
	else
	{
#ifdef NL_DEBUG_STORAGE_STREAM
		nldebug("No input, this function cannot output, throw exception");
#endif
		throw NLMISC::EStream();
	}
}

bool CStorageStream::enterChunk()
{
	if (m_Input)
	{
		// input logic
		if (!isChunkContainer())
		{
#ifdef NL_DEBUG_STORAGE_STREAM
			nldebug("Current chunk is not a container, cannot enter");
#endif
			return false;
		}
		if (endOfChunk())
		{
#ifdef NL_DEBUG_STORAGE_STREAM
			nldebug("End of chunk, cannot enter");
#endif
			return false;
		}
		m_Chunks.resize(m_Chunks.size() + 1);
		CChunk *chunk = currentChunk();
		chunk->OffsetBegin = CStorageStream::getPos();
		serial(chunk->Id);
		serial(chunk->Size);
		chunk->HeaderSize = 6;
		if (chunk->Size == 0)
		{
			// this is a 64bit chunk
			uint64 size64;
			serial(size64);
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
		return true;
	}
	else
	{
#ifdef NL_DEBUG_STORAGE_STREAM
		nldebug("No input, this function cannot output, throw exception");
#endif
		throw NLMISC::EStream();
	}
}

bool CStorageStream::enterChunk(uint16 id)
{
	if (m_Output)
	{
		if (m_Is64Bit)
			throw NLMISC::EStream("64bit chunks not supported");

		// current chunk is a container
		currentChunk()->Id = 1; // in write mode, id flags a container

		// enter the new chunk
		m_Chunks.resize(m_Chunks.size() + 1);
		CChunk *chunk = currentChunk();
		chunk->Id = 0; // don't know if it's a container
		uint32 sizeDummy = 0xFFFFFFFF;
		chunk->OffsetBegin = getPos(); // store current pos

		// write header
		serial(id); // write the id
		serial(sizeDummy); // write 32 bit size placeholder
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

sint32 CStorageStream::leaveChunk()
{
	if (m_Input)
	{
		// input logic
		sint32 skipped = currentChunk()->endOfChunk() - CStorageStream::getPos();
		if (skipped)
		{
			CStorageStream::seek(currentChunk()->endOfChunk(), begin);
#ifdef NL_DEBUG_STORAGE_STREAM
			nldebug("Skipped %i bytes in the current chunk", skipped);
#endif
		}
		m_Chunks.resize(m_Chunks.size() - 1);
		return skipped;
	}
	else if (m_Output)
	{
		sint32 pos = getPos();
		sint32 sizeWithHeader = pos - currentChunk()->OffsetBegin;
		sint32 sizePos = currentChunk()->OffsetBegin + 2;
		seek(sizePos, begin); // hopefully this correctly overwrites!!!
		uint32 sizeField = (uint32)sizeWithHeader | (uint32)currentChunk()->Id << 31; // add container flag
		serial(sizeField);
		seek(pos, begin);
		m_Chunks.resize(m_Chunks.size() - 1);
		return sizeWithHeader;
	}
	else
	{
#ifdef NL_DEBUG_STORAGE_STREAM
		nldebug("No input or output, should not happen, throw exception");
#endif
		throw NLMISC::EStream();
	}
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
