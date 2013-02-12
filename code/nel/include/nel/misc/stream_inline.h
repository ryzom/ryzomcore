// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_STREAM_INLINE_H
#define NL_STREAM_INLINE_H

#include "types_nl.h"
#include "debug.h"

#include <cstdio>


namespace	NLMISC
{


// ======================================================================================================
// ======================================================================================================
// IBasicStream Inline Implementation.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	IStream::IStream(bool inputStream)
{
	_InputStream= inputStream;
	_XML = false;
	resetPtrTable();
}


// ======================================================================================================
inline	bool		IStream::isReading() const
{
	return _InputStream;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
inline	void		IStream::serial(uint8 &b)
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(sint8 &b)
{
	serialBuffer((uint8 *)&b, 1);
}

// ======================================================================================================
inline	void		IStream::serial(uint16 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 2);
#else // NL_LITTLE_ENDIAN
	uint16	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 2);
		NLMISC_BSWAP16(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP16(v);
		serialBuffer((uint8 *)&v, 2);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(sint16 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 2);
#else // NL_LITTLE_ENDIAN
	uint16	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 2);
		NLMISC_BSWAP16(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP16(v);
		serialBuffer((uint8 *)&v, 2);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(uint32 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 4);
#else // NL_LITTLE_ENDIAN
	uint32	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 4);
		NLMISC_BSWAP32(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP32(v);
		serialBuffer((uint8 *)&v, 4);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(sint32 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 4);
#else // NL_LITTLE_ENDIAN
	uint32	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 4);
		NLMISC_BSWAP32(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP32(v);
		serialBuffer((uint8 *)&v, 4);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(uint64 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 8);
#else // NL_LITTLE_ENDIAN
	uint64	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 8);
		NLMISC_BSWAP64(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP64(v);
		serialBuffer((uint8 *)&v, 8);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(sint64 &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 8);
#else // NL_LITTLE_ENDIAN
	uint64	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 8);
		NLMISC_BSWAP64(v);
		b=v;
	}
	else
	{
		v=b;
		NLMISC_BSWAP64(v);
		serialBuffer((uint8 *)&v, 8);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(float &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 4);
#else // NL_LITTLE_ENDIAN
	uint32	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 4);
		NLMISC_BSWAP32(v);
		b=*((float*)&v);
	}
	else
	{
		v=*((uint32*)&b);
		NLMISC_BSWAP32(v);
		serialBuffer((uint8 *)&v, 4);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(double &b)
{
#ifdef NL_LITTLE_ENDIAN
	serialBuffer((uint8 *)&b, 8);
#else // NL_LITTLE_ENDIAN
	uint64	v;
	if(isReading())
	{
		serialBuffer((uint8 *)&v, 8);
		NLMISC_BSWAP64(v);
		b=*((double*)&v);
	}
	else
	{
		v=*((uint64*)&b);
		NLMISC_BSWAP64(v);
		serialBuffer((uint8 *)&v, 8);
	}
#endif // NL_LITTLE_ENDIAN
}

// ======================================================================================================
inline	void		IStream::serial(bool &b)
{
	serialBit(b);
}

#ifndef NL_OS_CYGWIN
// ======================================================================================================
inline	void		IStream::serial(char &b)
{
	serialBuffer((uint8 *)&b, 1);
}
#endif

// ======================================================================================================
inline	void		IStream::serial(std::string &b)
{
	uint32	len=0;
	// Read/Write the length.
	if(isReading())
	{
		serial(len);

		// check stream holds enough bytes (avoid STL to crash on resize)
		checkStreamSize(len);

		b.resize(len);
	}
	else
	{
		len= uint32(b.size());
		if (len>1000000)
			throw NLMISC::EInvalidDataStream( "IStream: Trying to write a string of %u bytes", len );
		serial(len);
	}

/*
	// Read/Write the string.
	for(sint i=0;i<len;i++)
		serial(b[i]);
*/

	if (len > 0)
		serialBuffer((uint8*)(&(b[0])), len);
}


// ======================================================================================================
inline	void		IStream::serial(ucstring &b)
{
	uint32	len=0;
	// Read/Write the length.
	if(isReading())
	{
		serial(len);

		// check stream holds enough bytes (avoid STL to crash on resize)
		checkStreamSize(len);

		b.resize(len);
	}
	else
	{
		len= uint32(b.size());
		if (len>1000000)
			throw NLMISC::EInvalidDataStream( "IStream: Trying to write an ucstring of %u bytes", len );
		serial(len);
	}
	// Read/Write the string.
	for(uint i=0;i!=len;++i)
		serial(b[i]);
}



// ======================================================================================================
inline uint8			IStream::serialBitField8(uint8  bf)
{
	serial(bf);
	return bf;
}
// ======================================================================================================
inline uint16			IStream::serialBitField16(uint16  bf)
{
	serial(bf);
	return bf;
}
// ======================================================================================================
inline uint32			IStream::serialBitField32(uint32  bf)
{
	serial(bf);
	return bf;
}


}


#endif // NL_STREAM_INLINE_H

/* End of stream_inline.h */
