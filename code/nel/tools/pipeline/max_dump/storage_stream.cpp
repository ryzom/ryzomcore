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

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CStorageStream::CStorageStream(GsfInput *input) : NLMISC::IStream(true), m_Input(input)
{
	m_RootChunk.OffsetBegin = -6;
	m_RootChunk.Id = 0;
	m_RootChunk.Size = 0x80000000;
	m_CurrentChunk = &m_RootChunk;
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
	return NLMISC::IStream::getPos();
}

void CStorageStream::serialBuffer(uint8 *buf, uint len)
{
	if (!len)
		return;
	if (m_Input)
	{
		if (!gsf_input_read(m_Input, len, buf))
			throw NLMISC::EStream();
	}
	else
	{
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
	return gsf_input_eof(m_Input);
}

bool CStorageStream::enterChunk()
{
	// input logic
	if (!isChunkContainer())
		return false;
	if (endOfChunk())
		return false;
	CChunk *chunk = new CChunk();
	chunk->OffsetBegin = CStorageStream::getPos();
	serial(chunk->Id);
	serial(chunk->Size);
	chunk->Parent = m_CurrentChunk;
	{
		// temp memleak fix
		std::map<uint16, CChunk *>::iterator it = m_CurrentChunk->Children.find(chunk->Id);
		if (it != m_CurrentChunk->Children.end())
			m_CurrentChunk->Children.erase(it);
	}
	m_CurrentChunk->Children[chunk->Id] = chunk; // assuming there's one child per id...
	m_CurrentChunk = chunk;
	return true;
}

sint32 CStorageStream::leaveChunk()
{
	// input logic
	sint32 skipped = m_CurrentChunk->endOfChunk() - CStorageStream::getPos();
	if (skipped)
		CStorageStream::seek(m_CurrentChunk->endOfChunk(), begin);
	m_CurrentChunk = m_CurrentChunk->Parent;
	return skipped;
}

void CStorageStream::buildChunkIndexById()
{

}

void CStorageStream::findChunkById(uint16 id)
{

}

} /* namespace PIPELINE */

/* end of file */
