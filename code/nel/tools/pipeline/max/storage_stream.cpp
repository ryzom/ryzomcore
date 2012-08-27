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

// using namespace std;
// using namespace NLMISC;

// #define NL_DEBUG_STORAGE

namespace PIPELINE {
namespace MAX {

CStorageStream::CStorageStream(GsfInput *input) : NLMISC::IStream(true), m_Input(input), m_Output(NULL)
{

}

CStorageStream::CStorageStream(GsfOutput *output) : NLMISC::IStream(false), m_Input(NULL), m_Output(output)
{

}

CStorageStream::~CStorageStream()
{
	if (m_Output)
	{
		gsf_output_seek(m_Output, 0, G_SEEK_END);
	}
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
#ifdef NL_DEBUG_STORAGE
		nldebug("Serial 0 size buffer");
#endif
		return;
	}
	if (m_Input)
	{
		if (!gsf_input_read(m_Input, len, buf))
		{
#ifdef NL_DEBUG_STORAGE
			nldebug("Cannot read from input, throw exception");
#endif
			throw NLMISC::EStream();
		}
	}
	else if (m_Output)
	{
		if (!gsf_output_write(m_Output, len, buf))
		{
			nlwarning("Cannot write %i bytes to output at pos %i, throw exception", len, getPos());
			throw NLMISC::EStream();
		}
	}
	else
	{
#ifdef NL_DEBUG_STORAGE
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
#ifdef NL_DEBUG_STORAGE
		nldebug("No input, this function cannot output, throw exception");
#endif
		throw NLMISC::EStream();
	}
}

sint32 CStorageStream::size()
{
	if (m_Input)
	{
		gsf_off_t res = gsf_input_size(m_Input);
		if (res < 2147483647L) // exception when larger
			return (sint32)res;
	}
	else
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("No input, this function cannot output, throw exception");
#endif
		throw NLMISC::EStream();
	}
	throw NLMISC::EStream();
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
