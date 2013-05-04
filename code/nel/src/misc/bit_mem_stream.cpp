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

#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/bit_set.h"

#ifdef LOG_ALL_TRAFFIC
#include "nel/misc/command.h"
#endif

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifdef LOG_ALL_TRAFFIC
bool VerboseAllTraffic = false;
#endif


const char * SerialTypeToCStr [ TBMSSerialInfo::NbSerialTypes ] = { "Bool ", "Ui32N", "Ui64N", "Float", "Btfld", "Buffr" };


/*
 * Constructor
 */
CBitMemStream::CBitMemStream( bool inputStream, uint32 defaultcapacity ) :
	CMemStream( inputStream, false, defaultcapacity ),
	_FreeBits( 0 )
{
	resetBufPos();
}


/*
 * Copy constructor
 */
CBitMemStream::CBitMemStream( const CBitMemStream& other ) :
	CMemStream( other ),
	_FreeBits( other._FreeBits ),
	_DbgInfo( other._DbgInfo )
{
}

/*
 *	Exchange
 */
void CBitMemStream::swap(CBitMemStream &other)
{
	CMemStream::swap(other);
	std::swap(_FreeBits, other._FreeBits);
	_DbgInfo.swap(other._DbgInfo);
}


/*
 * Serialize a buffer
 */
void CBitMemStream::serialBuffer( uint8 *buf, uint len )
{
	_DbgInfo.addSerial( getPosInBit(), len*8, TBMSSerialInfo::Buffer );
	uint i;
	uint32 v;
	if ( isReading() )
	{
		for ( i=0; i!=len; ++i )
		{
			internalSerial( v, 8 );
			buf[i] = (uint8)v;
		}
	}
	else
	{
		for ( i=0; i!=len; ++i )
		{
			v = (uint32)buf[i];
			internalSerial( v, 8 );
		}
	}
}


/*
 * Serialize one bit
 */
void CBitMemStream::serialBit( bool& bit )
{
	_DbgInfo.addSerial( getPosInBit(), 1, TBMSSerialInfo::B );
	uint32 ubit=0;
	if ( isReading() )
	{
		internalSerial( ubit, 1 );
		bit = ( ubit!=0 );
	}
	else
	{
		ubit = bit;
		internalSerial( ubit, 1 );
	}
}


//sint32 CBitMemStream::getPosInBit ()


#ifdef LOG_ALL_TRAFFIC

void	CBitMemStream::_serialAndLog( const char *argstr, uint32& value, uint nbits )
{
	_DbgInfo.setSymbolOfNextSerialEvent( argstr );
	sint32 bitpos = getPosInBit();
	serial( value, nbits );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: %u bits at bitpos %d (%u)", this, isReading()?"I":"O", argstr, nbits, bitpos, value );
}

void	CBitMemStream::_serialAndLog( const char *argstr, uint64& value, uint nbits )
{
	_DbgInfo.setSymbolOfNextSerialEvent( argstr );
	sint32 bitpos = getPosInBit();
	serial( value, nbits );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: %u bits at bitpos %d (%u)", this, isReading()?"I":"O", argstr, nbits, bitpos, value );
}

void	CBitMemStream::_serialBitAndLog( const char *argstr, bool& bit )
{
	_DbgInfo.setSymbolOfNextSerialEvent( argstr );
	sint32 bitpos = getPosInBit();
	serialBit( bit );
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: %s: 1 bit at bitpos %d (%hu)", this, isReading()?"I":"O", argstr, bitpos, (uint16)bit );
}

NLMISC_CATEGORISED_COMMAND(nel, verboseAllTraffic, "Verbose the all-traffic logs", "" )
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
	{
		if(args[0]==string("on")||args[0]==string("ON")||args[0]==string("true")||args[0]==string("TRUE")||args[0]==string("1"))
			VerboseAllTraffic=true;
		else if(args[0]==string("off")||args[0]==string("OFF")||args[0]==string("false")||args[0]==string("FALSE")||args[0]==string("0"))
			VerboseAllTraffic=false;
	}

	nlinfo("BMS: verboseAllTraffic is %s",VerboseAllTraffic?"ON":"OFF");
	return true;
}

#endif


/*
 * Serialize only the nbits lower bits of value
 */
void	CBitMemStream::internalSerial( uint32& value, uint nbits, bool resetvalue )
{
	if ( nbits == 0 )
		return;
	if ( nbits > 32 )
		throw EMemStream (string("trying to serial ")+NLMISC::toString(nbits)+string(" bits"));

	if ( isReading() )
	{
		const uint8 *buffer = _Buffer.getBuffer().getPtr();
		// Check that we don't read more than there is to read
		uint32 pib = getPosInBit();
		uint32 len = ((uint32)lengthR());
		if ( pib + nbits > len * 8 )
		{
			//displayStream( "Stream Overflow" );
			throw EStreamOverflow( "CBitMemStream overflow: Read past %u bytes", len );
		}

		if ( resetvalue )
		{
			value = 0;
		}

		// Clear high-order bits after _FreeBits
		uint8 v = *(buffer + _Buffer.Pos) & ((1 << _FreeBits) - 1);

		if ( nbits > _FreeBits )
		{
			//nldebug( "Reading byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			value |= (v << (nbits-_FreeBits));
//			++_BufPos;
			++_Buffer.Pos;
			uint readbits = _FreeBits;
			//displayByteBits( *_BufPos, 8, readbits-1, false );
			_FreeBits = 8;
			internalSerial( value, nbits - readbits, false ); // read without resetting value
		}
		else
		{
			//nlinfo( "Reading last byte %u from %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			//displayByteBits( *_BufPos, 8, _FreeBits-1, false );
			value |= (v >> (_FreeBits-nbits));
			//displayByteBits( *_BufPos, 8, _FreeBits-1, false );
			_FreeBits -= nbits;
			if ( _FreeBits == 0 )
			{
				_FreeBits = 8;
//				++_BufPos;
				++_Buffer.Pos;
			}
		}
	}
	else
	{
		uint8 *buffer = _Buffer.getBufferWrite().getPtr();
		// Clear high-order bits after nbits
		//displayDwordBits( value, 32, nbits-1, false );

		//uint32 mask = (-1 >> (32-nbits)); // does not work
		uint32 v;
		if ( nbits != 32 ) // arg of shl/sal/shr/sal ranges from 0 to 31
		{
			uint32 mask = (1 << nbits) - 1;
			v = value & mask;
		}
		else
		{
			v = value;
		}

#ifdef NL_DEBUG
		// Check that the current byte is prepared
		nlassert( ! ((_FreeBits == 8) && (*(buffer+_Buffer.Pos) != 0)) );
#endif

		// Set
		if ( nbits > _FreeBits )
		{
			// Longer than the room in the current byte
			//nldebug( "Writing byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			//displayDwordBits( value, 32, nbits-1, false );
//			*_BufPos |= (v >> (nbits - _FreeBits));
			*(buffer+_Buffer.Pos) |= (v >> (nbits - _FreeBits));
			uint filledbits = _FreeBits;
			//displayByteBits( *_BufPos, 8, filledbits-1, false );
			prepareNextByte();
			internalSerial( v, nbits - filledbits );
		}
		else
		{
			// Shorter or equal
			//nldebug( "Writing last byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
			//displayByteBits( *_BufPos, 8, 7, false );
//			*_BufPos |= (v << (_FreeBits-nbits));
			*(buffer+_Buffer.Pos) |= (v << (_FreeBits-nbits));
			//displayByteBits( *_BufPos, 8, _FreeBits-1, false );
			_FreeBits -= nbits;
			if ( _FreeBits == 0 )
			{
				prepareNextByte();
			}
		}
	}
}


/*
 * Same as CMemStream::reserve()
 */
sint32	CBitMemStream::reserve( uint byteLen )
{
	sint32 p = getPos();
	reserveBits( byteLen * 8 );
	return p;
}


/*
 * In a output bit stream, serialize nbits bits (no matter their value).
 * Works even if the number of bits to add is larger than 64. See also poke() and pokeBits().
 */
void	CBitMemStream::reserveBits( uint nbits )
{
#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Reserving %u bits at bitpos %d", this, isReading()?"I":"O", nbits, getPosInBit() );
#endif

	uint32 v = 0;
	while ( nbits > 32 )
	{
		serial( v, 32 );
		nbits -= 32;
	}
	if ( nbits != 0 )
		serial( v, nbits );
}


/*
 * Helper for poke(), to write a value inside an output stream (works because reserveBits sets to 0)
 */
void	CBitMemStream::serialPoke( uint32 value, uint nbits )
{
	uint32 v;
	if ( nbits != 32 ) // arg of shl/sal/shr/sal ranges from 0 to 31
	{
		uint32 mask = (1 << nbits) - 1;
		v = value & mask;
	}
	else
	{
		v = value;
	}

	uint8 *buffer = _Buffer.getBufferWrite().getPtr();

	// Set
	if ( nbits > _FreeBits )
	{
		// Longer than the room in the current byte
		//nldebug( "Writing byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
		//displayDwordBits( value, 32, nbits-1, false );
//		*_BufPos |= (v >> (nbits - _FreeBits));
		*(buffer + _Buffer.Pos) |= (v >> (nbits - _FreeBits));
		uint filledbits = _FreeBits;
		//displayByteBits( *_BufPos, 8, filledbits-1, false );
		pointNextByte(); // do not set next byte to 0!
//		nlassert( _BufPos < _Buffer.getPtr() + _Buffer.size() );
		nlassert( _Buffer.Pos < _Buffer.getBuffer().size() );
		serialPoke( v, nbits - filledbits );
	}
	else
	{
		// Shorter or equal
		//nldebug( "Writing last byte %u into %u free bits (%u remaining bits)", lengthS(), _FreeBits, nbits );
		//displayByteBits( *_BufPos, 8, 7, false );
//		*_BufPos |= (v << (_FreeBits-nbits));
		*(buffer + _Buffer.Pos) |= (v << (_FreeBits-nbits));
		//displayByteBits( *_BufPos, 8, _FreeBits-1, false );
		_FreeBits -= nbits;
		if ( _FreeBits == 0 )
		{
			pointNextByte(); // do not set next byte to 0!
//			nlassert( _BufPos < _Buffer.getPtr() + _Buffer.size() );
			nlassert( _Buffer.Pos < _Buffer.getBuffer().size() );
		}
	}
}


/*
 * Rewrite the nbbits lowest bits of a value at the specified position bitpos of the current output bit stream.
 *
 * Preconditions:
 * - bitpos+nbbits <= the current length in bit of the stream.
 * - The bits poked must have been reserved by reserve() (i.e. set to 0)
 */
void	CBitMemStream::poke( uint32 value, uint bitpos, uint nbits )
{
#ifdef NL_DEBUG
	nlassert( (nbits <= 32) && (nbits != 0) );
	nlassert( ! isReading() );
	nlassert( bitpos+nbits <= (uint)getPosInBit() );
#endif

	// Save the current pointers of the stream, and make them point to the required position
	uint savedFreeBits = _FreeBits;
	uint bytepos = bitpos >> 3;
	_FreeBits = 8 - (bitpos - (bytepos << 3));
//	uint8 *savedBufPos = _BufPos;
	uint32 savedBufPos = _Buffer.Pos;
//	_BufPos = _Buffer.getPtr() + bytepos;
	_Buffer.Pos = bytepos;

	// Serial
	_DbgInfo.addPoke( bitpos, nbits, TBMSSerialInfo::U );
	serialPoke( value, nbits );

	// Restore the current pointers
	_FreeBits = savedFreeBits;
//	_BufPos = savedBufPos;
	_Buffer.Pos = savedBufPos;
}


/* Rewrite the bitfield at the specified position bitpos of the current output bit stream.
 * The size of the bitfield is *not* written into stream (unlike serialCont()).
 * Precondition: bitpos+bitfield.size() <= the current length in bit of the stream. See also reserveBits().
 */
void	CBitMemStream::pokeBits( const CBitSet& bitfield, uint bitpos )
{
#ifdef NL_DEBUG
	nlassert( ! isReading() );
	nlassert( bitpos+bitfield.size() <= (uint)getPosInBit() );
#endif

	// Save the current pointers of the stream, and make them point to the required position
	uint savedFreeBits = _FreeBits;
	uint bytepos = bitpos >> 3;
	_FreeBits = 8 - (bitpos - (bytepos << 3));
//	uint8 *savedBufPos = _BufPos;
	uint32 savedBufPos = _Buffer.Pos;
//	_BufPos = _Buffer.getPtr() + bytepos;
	_Buffer.Pos = bytepos;

	// Serial
	_DbgInfo.addPoke( bitpos, bitfield.size(), TBMSSerialInfo::BF );
	const vector<uint32>& uintVec = bitfield.getVector();
	if ( ! uintVec.empty() )
	{
		uint len = bitfield.size();
		uint i = 0;
		while ( len > 32 )
		{
			serialPoke( uintVec[i], 32 );
			len -= 32;
			++i;
		}
		if ( len != 0 )
			serialPoke( uintVec[i], len );
	}

	// Restore the current pointers
	_FreeBits = savedFreeBits;
//	_BufPos = savedBufPos;
	_Buffer.Pos = savedBufPos;
}


/*
 * Read bitfield.size() bits from the input stream to fill the bitfield.
 * It means you have to know the size and to resize the bitfield yourself.
 */
void	CBitMemStream::readBits( NLMISC::CBitSet& bitfield )
{
#ifdef NL_DEBUG
	nlassert( isReading() );
#endif
	uint len = bitfield.size();
	if ( len != 0 )
	{
#ifdef LOG_ALL_TRAFFIC
		if ( VerboseAllTraffic )
			nldebug( "TRAFFIC/%p/%s: Reading %u bits bitfield at bitpos %d", this, isReading()?"I":"O", len, getPosInBit() );
#endif
		uint i = 0;
		uint32 v;
		while ( len > 32 )
		{
			serial( v, 32 );
			//nldebug( "Bitfield: Read %u at %d", v, _BufPos-_Buffer.getPtr()-4 );
			bitfield.setUint( v, i );
			len -= 32;
			++i;
		}
		serial( v, len );
		//nldebug( "Bitfield: Read %u at %d", v, _BufPos-_Buffer.getPtr()-4 );
		bitfield.setUint( v, i );
	}
}


/*
 * Serial float
 */
void	CBitMemStream::serial(float &b)
{
	_DbgInfo.addSerial( getPosInBit(), sizeof(b)*8, TBMSSerialInfo::F );
	uint32 uf=0;
	if ( isReading() )
	{
		internalSerial( uf, sizeof(b)*8 );
		memcpy(&b, &uf, sizeof(b));
	}
	else
	{
		memcpy(&uf, &b, sizeof(b));
		internalSerial( uf, sizeof(b)*8 );
	}
}


/*
 * Serial string
 */
void	CBitMemStream::serial(std::string &b)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

	uint32 len=0;

	// Serialize length
	if ( isReading() )
	{
		serial( len );
		if (len > length()-(uint32)getPos())
			throw NLMISC::EInvalidDataStream( "BMS: Trying to read a string of %u bytes, past stream size", len );
		b.resize( len );
	}
	else
	{
		len = (uint32)b.size();
		if (len>1000000)
			throw NLMISC::EInvalidDataStream( "BMS: Trying to write a string of %u bytes", len );
		serial( len );
	}

	// Serialize buffer
	if ( len != 0 )
	{
		serialBuffer( (uint8*)(&*b.begin()), len );
	}

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: String (size 32+%u*8 bits) at bitpos %d", this, isReading()?"I":"O", len, bitpos );
#endif
}


/*
 * Serial string
 */
inline	void		CBitMemStream::serial(ucstring &b)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

	if ( _StringMode )
	{
		uint32	len=0;
		// Read/Write the length.
		if(isReading())
		{
			serial(len);
			if (len > (uint32)(sint32(length())-sint32(getPos())))
				throw NLMISC::EInvalidDataStream( "BMS: Trying to read an ucstring of %u bytes, past stream size", len );
			b.resize(len);
		}
		else
		{
			len= (uint32)b.size();
			if (len>1000000)
				throw NLMISC::EInvalidDataStream( "BMS: Trying to write an ucstring of %u bytes", len );
			serial(len);
		}
		// Read/Write the string.
		for(uint i=0;i!=len;++i)
			serialBuffer( (uint8*)&b[i], sizeof(b[i]) );

		char sep = SEPARATOR;
		serialBuffer( (uint8*)&sep, 1 );
	}
	else
	{
		IStream::serial( b );
	}

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Ucstring at bitpos %d", this, isReading()?"I":"O", bitpos );
#endif

}


/*
 * Append the contents of a bitmemstream at the end of our bitmemstream
 */
void	CBitMemStream::append( const CBitMemStream& newBits )
{
	nlassert ( !isReading() );
	serialBuffer( const_cast<uint8*>(newBits.buffer()), newBits.getPos() );

	uint nbRemainingBits = 8 - newBits._FreeBits;
	_DbgInfo.addSerial( getPosInBit(), nbRemainingBits, TBMSSerialInfo::Buffer );
	uint32 lastByte = (uint32)(*(newBits.buffer() + newBits.getPos())) >> newBits._FreeBits;
	internalSerial( lastByte, nbRemainingBits );
}


/*
 * Serial bitmemstream
 */
void	CBitMemStream::serialMemStream(CMemStream &b)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

	uint32 len=0;

	// Serialize length
	if ( isReading() )
	{
		// fill b with data from this
		serial (len);
		if (len > length()-getPos())
			throw NLMISC::EInvalidDataStream( "BMS: Trying to read a BMS of %u bytes, past stream size", len );

		serialBuffer (b.bufferToFill (len), len);
		b.resetBufPos ();
	}
	else
	{
		// fill this with data from b
		len = b.length();
		// Accept to write a big sized BMS

		serial( len );
		serialBuffer( (uint8*) b.buffer (), len );
	}

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Sub-bitmemstream (size 32+%u*8 bits) at bitpos %d", this, isReading()?"I":"O", len, bitpos );
#endif
}

/*
 * Specialisation of serialCont() for vector<bool>
 */
void CBitMemStream::serialCont(std::vector<bool> &cont)
{
#ifdef LOG_ALL_TRAFFIC
	sint32 bitpos = getPosInBit();
#endif

	sint32	len=0;
	if(isReading())
	{
		serial(len);
		if (len/8 > (sint32)(length()-getPos()))
		{
			throw NLMISC::EInvalidDataStream( "BMS: Trying to read a vec<bool> of %u bytes, past stream size", len/8 );
		}
		// special version for vector: adjust good size.
		contReset(cont);
		cont.reserve(len);

		for(sint i=0;i<len;i++)
		{
			bool	v;
			serialBit(v);
			cont.insert(cont.end(), v);
		}
	}
	else
	{
		len= (sint32)cont.size();
		serial(len);

		std::vector<bool>::iterator it= cont.begin();
		for(sint i=0;i<len;i++, it++)
		{
			bool b = *it;
			serialBit( b );
		}
	}

#ifdef LOG_ALL_TRAFFIC
	if ( VerboseAllTraffic )
		nldebug( "TRAFFIC/%p/%s: Container (header: 32 bits) at bitpos %d", this, isReading()?"I":"O", bitpos );
#endif

}


/*
 * Display the bits of the stream just before the current pos
 */
void	CBitMemStream::displayLastBits( sint nbits, sint bitpos, NLMISC::CLog *log )
{
	if ( bitpos == -1 )
		bitpos = getPosInBit();
	displayBitStream( *this, max(bitpos-nbits, 0), bitpos-1, log );
}


/*
 * Display a part of a bitmemstream
 */
void	displayBitStream( const CBitMemStream& msg, sint beginbitpos, sint endbitpos, NLMISC::CLog *log )
{
	sint beginpos = beginbitpos/8;
	sint endpos = endbitpos/8;
	nlinfo( "BMS: beginpos %d endpos %d beginbitpos %d endbitpos %d", beginpos, endpos, beginbitpos, endbitpos );
	displayByteBits( *(msg.buffer()+beginpos), 8, 8-(beginbitpos-beginpos*8), true, log );
	const uint8 *p;
	for ( p=msg.buffer()+beginpos+1; p<msg.buffer()+endpos-1; ++p )
	{
		displayByteBits( *p, 8, 0, false, log );
	}
	if ( endpos > beginpos )
	{
		displayByteBits( *(msg.buffer()+endpos), 8, 0, false, log );
	}
}


/*
 * Returns the stream as a string with 0 and 1.
 */
void		CBitMemStream::displayStream( const char *title, CLog *log )
{
//	nlassert( (_BufPos >= _Buffer.getPtr()) && (_BufPos <= _Buffer.getPtr() + _Buffer.size()) );
	nlassert( _Buffer.Pos <= _Buffer.getBuffer().size() );

	// Display title and information
	string s = (isReading()?string("I"):string("O")) + string("BMS ") + string(title) + ": ";
	string sLegend;
//	if ( _BufPos == _Buffer.getPtr() )
	if ( _Buffer.Pos == 0 )
	{
		log->displayNL( (s + "Empty").c_str() );
		return;
	}

//	s += NLMISC::toString( "BitPos=%d Pos=%u FreeBits=%u Size=%u ", getPosInBit(), (uint32)(_BufPos-_Buffer.getPtr()), _FreeBits, _Buffer.size() );
	s += NLMISC::toString( "BitPos=%d Pos=%u FreeBits=%u Size=%u ", getPosInBit(), _Buffer.Pos, _FreeBits, _Buffer.getBuffer().size() );
	log->displayNL( s.c_str() );
	s.clear();

	// Display bitstream (output: until _BufPos/_FreeBits; input: whole buffer)
	_DbgInfo.beginEventBrowsing();
	sint32 eventId;
	uint32 bitpos = 0;
	const uint8 *p;
//	uint8 *endPos = isReading() ? (_Buffer.getPtr() + _Buffer.size()) : (_BufPos+1);
	const uint8 *endPos = isReading() ? (_Buffer.getBuffer().getPtr() + _Buffer.getBuffer().size()) : (_Buffer.getBuffer().getPtr() + _Buffer.Pos+1);
//	for ( p=_Buffer.getPtr(); p!=endPos; ++p )
	for ( p=_Buffer.getBuffer().getPtr(); p!=endPos; ++p )
	{
		sint i;
		for ( i=7; i!=-1; --i )
		{
			//bitpos = (p-_Buffer.getPtr())*8 + (7-i);
			if ( bitpos == (uint32)getPosInBit() )
				s += "<P>"; // display the current position
			s += _DbgInfo.getEventIdAtBitPos( bitpos, &eventId );
			s += ( ((*p) >> i) & 1 ) ? '1' : '0';
			sLegend += _DbgInfo.getEventLegendAtBitPos( *this, eventId );
			++bitpos;
		}

		s += ' '; // a blank char between each byte
		if ( bitpos % 64 == 0 ) // limit to 8 bytes per line
		{
			log->displayRawNL( s.c_str() );
			s.clear();
		}
	}
	if ( bitpos % 64 != 0 )
		log->displayRawNL( s.c_str() );
	_DbgInfo.endEventBrowsing();

	// Display legend
	string::size_type lineStart = 0;
	string::size_type crp = sLegend.find( '\n', lineStart );
	while ( crp != string::npos )
	{
		log->displayRawNL( sLegend.substr( lineStart, crp-lineStart ).c_str() );
		lineStart = crp + 1;
		crp = sLegend.find( '\n', lineStart );
	}
	// sLegend ends with a '\n'
}


/*
 * Return a string showing the serial item
 */
std::string		CBitMemStream::getSerialItem( const TBMSSerialInfo& serialItem )
{
	// Save the current pointers of the stream, and make them point to the required position
	uint savedFreeBits = _FreeBits;
	uint bytepos = serialItem.BitPos >> 3;
	_FreeBits = 8 - (serialItem.BitPos - (bytepos << 3));
//	uint8 *savedBufPos = _BufPos;
	uint32 savedBufPos = _Buffer.Pos;
//	_BufPos = _Buffer.getPtr() + bytepos;
	_Buffer.Pos = bytepos;

	bool wasOutput = false;;
	if ( ! isReading() )
	{
		setInOut( true ); // lighter than invert()
		wasOutput = true;
	}

	// Read and format string
	string s;
	if ( getPosInBit() + serialItem.BitSize > lengthR() * 8 )
	{
		s = "<Stream Overflow>";
	}
	else
	{
		switch ( serialItem.Type )
		{
		case TBMSSerialInfo::B:
			{
				bool b;
				serialBit( b );
				s = NLMISC::toString( "%s", b?"TRUE":"FALSE" );
				break;
			}
		case TBMSSerialInfo::U: // no distinction with signed int!
			{
				uint32 u;
				serial( u, serialItem.BitSize );
				s = NLMISC::toString( "%u", u );
				break;
			}
		case TBMSSerialInfo::U64: // no distinction with signed int64!
			{
				uint64 u;
				serial( u, serialItem.BitSize );
				s = NLMISC::toString( "%"NL_I64"u", u );
				break;
			}
		case TBMSSerialInfo::F: // what about double?
			{
				float f;
				serial( f );
				s = NLMISC::toString( "%g", f );
				break;
			}
		case TBMSSerialInfo::BF:
			{
				CBitSet bs;
				bs.resize( serialItem.BitSize );
				readBits( bs );
				s = bs.toString();
				break;
			}
		case TBMSSerialInfo::Buffer:
			{
				uint32 len = serialItem.BitSize / 8;
				s.resize( len + 2 );
				if ( len != 0 )
				{
					serialBuffer( &((uint8&)(s[1])), len );
					string::size_type p;
					for ( p=1; p!=len+1; ++p )
					{
						if ( ! isalnum(s[p]) )
							s[p] = '?'; // remove end-of-c_string
					}
				}
				s[0] = '[';
				s[len+1] = ']';
				break;
			}
		default:
		  break;
		}
	}

	// Restore the current pointers
	if ( wasOutput )
	{
		setInOut( false );
	}

	_FreeBits = savedFreeBits;
//	_BufPos = savedBufPos;
	_Buffer.Pos = savedBufPos;

	return s;
}


} // NLMISC
