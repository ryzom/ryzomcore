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

#include "stdmisc.h"

#include "nel/misc/mem_stream.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

void CMemStream::swap(CMemStream &other)
{
	IStream::swap(other);
	_Buffer.swap(other._Buffer);
	std::swap(_StringMode, other._StringMode);
	std::swap(_DefaultCapacity, other._DefaultCapacity);
}


/*
 * serial (inherited from IStream)
 */
void CMemStream::serialBuffer(uint8 *buf, uint len)
{
	// commented for optimum performance
//	nlassert (len > 0);

	if (len == 0)
		return;

	nlassert (buf != NULL);

	if ( isReading() )
	{
		// Check that we don't read more than there is to read
		//checkStreamSize(len);

		uint32 pos = lengthS();
		uint32 total = length();
		if ( pos+len > total ) // calls virtual length (cf. sub messages)
		{
			throw EStreamOverflow( "CMemStream serialBuffer overflow: Read past %u bytes", total );
		}

		// Serialize in
		CFastMem::memcpy( buf, _Buffer.getBuffer().getPtr()+_Buffer.Pos, len );
		_Buffer.Pos += len;
	}
	else
	{
		// Serialize out

		increaseBufferIfNecessary (len);
		CFastMem::memcpy( _Buffer.getBufferWrite().getPtr()+_Buffer.Pos, buf, len );
		_Buffer.Pos += len;

	}
}

/*
 * serialBit (inherited from IStream)
 */
void CMemStream::serialBit(bool &bit)
{
	uint8 u;
	if ( isReading() )
	{
		serial( u );
		bit = (u!=0);
	}
	else
	{
		u = (uint8)bit;
		serial( u );
	}
}


/*
 * seek (inherited from IStream)
 *
 * Warning: in output mode, seek(end) does not point to the end of the serialized data,
 * but on the end of the whole allocated buffer (see size()).
 * If you seek back and want to return to the end of the serialized data, you have to
 * store the position (a better way is to use reserve()/poke()).
 *
 * Possible enhancement:
 * In output mode, keep another pointer to track the end of serialized data.
 * When serializing, increment the pointer if its value exceeds its previous value
 * (to prevent from an "inside serial" to increment it).
 * Then a seek(end) would get back to the pointer.
 */
bool CMemStream::seek (sint32 offset, TSeekOrigin origin) const
{
	switch (origin)
	{
	case begin:
		if (offset > (sint)length())
			return false;
		if (offset < 0)
			return false;
		_Buffer.Pos = offset;
		break;
	case current:
		if (getPos ()+offset > (sint)length())
			return false;
		if (getPos ()+offset < 0)
			return false;
		_Buffer.Pos += offset;
		break;
	case end:
		if (offset < -(sint)length())
			return false;
		if (offset > 0)
			return false;
		_Buffer.Pos = _Buffer.getBuffer().size()+offset;
		break;
	}
	return true;
}


/*
 * Resize the buffer.
 * Warning: the position is unchanged, only the size is changed.
 */
void CMemStream::resize (uint32 size)
{
	if (size == length()) return;
	// need to increase the buffer size
	_Buffer.getBufferWrite().resize(size);
}


/*
 * Input: read from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
 */
uint CMemStream::serialSeparatedBufferIn( uint8 *buf, uint len )
{
	nlassert( _StringMode && isReading() );

	// Check that we don't read more than there is to read
	if ( ( _Buffer.Pos == _Buffer.getBuffer().size() ) || // we are at the end
		 ( ( lengthS()+len+SEP_SIZE > length() ) && (_Buffer.getBuffer()[_Buffer.getBuffer().size()-1] != SEPARATOR ) ) ) // we are before the end // calls virtual length (cf. sub messages)
	{
		throw EStreamOverflow();
	}
	// Serialize in
	uint32 i = 0;
	const uint8	*pos = _Buffer.getBuffer().getPtr()+_Buffer.Pos;
	while ( (i<len) && (*pos) != SEPARATOR )
	{
		*(buf+i) = *pos;
		i++;
		++pos;
		++_Buffer.Pos;
	}
	// Exceeds len
	if ( (*pos) != SEPARATOR )
	{
		throw EStreamOverflow();
	}
	_Buffer.Pos += SEP_SIZE;

	return i;
}


/*
 * Output: writes len bytes from buf into the stream
 */
void CMemStream::serialSeparatedBufferOut( uint8 *buf, uint len )
{
	nlassert( _StringMode && (!isReading()) );

	// Serialize out
	uint32 oldBufferSize = _Buffer.getBuffer().size();
	if (_Buffer.Pos + (len + SEP_SIZE) > oldBufferSize)
	{
		// need to increase the buffer size
		_Buffer.getBufferWrite().resize(oldBufferSize*2 + len + SEP_SIZE);
	}

	CFastMem::memcpy( _Buffer.getBufferWrite().getPtr()+_Buffer.Pos, buf, len );
	_Buffer.Pos += len;
	*(_Buffer.getBufferWrite().getPtr()+_Buffer.Pos) = SEPARATOR;
	_Buffer.Pos += SEP_SIZE;

}

/* Returns a readable string to display it to the screen. It's only for debugging purpose!
 * Don't use it for anything else than to debugging, the string format could change in the future.
 * \param hexFormat If true, display all bytes in hexadecimal, else display as chars (above 31, otherwise '.')
 */
std::string		CMemStream::toString( bool hexFormat ) const
{
	std::string s;
	uint32 len = length();
	if ( hexFormat )
	{
		for ( uint i=0; i!=len; ++i )
			s += NLMISC::toString( "%2X ", buffer()[i] );
	}
	else
	{
		for ( uint i=0; i!=len; ++i )
			s += NLMISC::toString( "%c", (buffer()[i]>31) ? buffer()[i] : '.' );
	}
	return s;
}


// ***************************************************************************
uint			CMemStream::getDbgStreamSize() const
{
	if(isReading())
		return length();
	else
		return 0;
}


}
