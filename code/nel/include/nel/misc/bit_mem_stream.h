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

#ifndef NL_BIT_MEM_STREAM_H
#define NL_BIT_MEM_STREAM_H

#include "types_nl.h"
#include "mem_stream.h"


namespace NLMISC {


/* In debugging stage, should be defined. In stable stage, undefine it!
 * Works along with the verboseAllTraffic command
 */
#ifndef NL_NO_DEBUG
#	ifdef NL_OS_WINDOWS
#		define LOG_ALL_TRAFFIC
#	else
#		undef LOG_ALL_TRAFFIC
#	endif
#endif

#ifdef LOG_ALL_TRAFFIC

extern bool VerboseAllTraffic;

#define serialAndLog1( v ) \
	_serialAndLog( #v, v );

#define serialAndLog2( v, s ) \
	_serialAndLog( #v, v, s );

#define serialBitAndLog( v ) \
	_serialBitAndLog( #v, v );

#define	serialAdaptAndLog( argstr, b, type ) \
	uint32 ub=0; \
	if ( isReading() ) \
	{ \
		_serialAndLog( argstr, ub, sizeof(type)*8 ); \
		b = (type)ub; \
	} \
	else \
	{ \
		ub = (uint32)b; \
		_serialAndLog( argstr, ub, sizeof(type)*8 ); \
	}

#ifdef NL_LITTLE_ENDIAN

#define	serialAdapt64AndLog( argstr, b ) \
	_serialAndLog( argstr, *((uint32*)(&b)), 32 ); \
	_serialAndLog( argstr, *((uint32*)(&b)+1), 32 );

#else

#define	serialAdapt64AndLog( argstr, b ) \
	serialAndLog( argstr, *((uint32*)(&b)+1), 32); \
	serialAndLog( argstr, *((uint32*)(&b)), 32);

#endif


#else

#define serialAndLog1 serial
#define serialAndLog2 serial
#define serialBitAndLog serialBit


#endif

#define	serialAdapt( b, type ) \
	uint32 ub=0; \
	if ( isReading() ) \
	{ \
		serial( ub, sizeof(type)*8 ); \
		b = (type)ub; \
	} \
	else \
	{ \
		ub = (uint32)b; \
		serial( ub, sizeof(type)*8 ); \
	}

#ifdef NL_LITTLE_ENDIAN

#define	serialAdapt64( b ) \
	serial( *((uint32*)(&b)), 32); \
	serial( *((uint32*)(&b)+1), 32);

#else

#define	serialAdapt64( b ) \
	serial( *((uint32*)(&b)+1), 32); \
	serial( *((uint32*)(&b)), 32);

#endif


class CBitSet;


/*
 * Item of CBMSDbgInfo
 */
struct TBMSSerialInfo
{
	enum TSerialType { B, U, U64, F, BF, Buffer, NbSerialTypes };

	TBMSSerialInfo( uint32 bitpos, uint32 bitsize, TSerialType type, const char *symbol )
	{
		nlassert( bitpos < 800000 );
		BitPos = bitpos;
		BitSize = bitsize;
		Type = type;
		Symbol = symbol;
	}

	uint32		BitPos;
	uint32		BitSize;
	TSerialType	Type;
	const char	*Symbol;
};

extern const char * SerialTypeToCStr [ TBMSSerialInfo::NbSerialTypes ];


typedef std::vector< TBMSSerialInfo > TBMSSerialInfoList;


/*
 * Data members struct for CBMSDbgInfo
 */
struct TBMSDbgInfoData
{
	/// Constructor
	TBMSDbgInfoData() : List(), CurrentBrowsedItem(0), NextSymbol(NULL), AddEventIsEnabled(true) {}

	/// Vector of serial items
	TBMSSerialInfoList				List;

	/// Current browsed item in the list (valid only from beginEventBrowsing() until clear() or addSerial()/addPoke())
	uint32							CurrentBrowsedItem;

	/// Symbol of next event
	const char						*NextSymbol;

	/// Flag to enable/disable addSerial() and addPoke() (because CBitMemStream::getSerialItem() must not add events in the list)
	bool							AddEventIsEnabled;
};

class CBitMemStream;

/*
 * Debug details about what was serialised (sizeof is only one pointer if not explicitely initialised)
 */
class CBMSDbgInfo
{
public:

#ifdef NL_DEBUG
	/// Constructor
	CBMSDbgInfo() : _DbgData(NULL) { init(); }
#else
	/// Constructor
	CBMSDbgInfo() {}
#endif

#ifdef NL_DEBUG
	/// Copy constructor
	CBMSDbgInfo( const CBMSDbgInfo& src ) : _DbgData(NULL)
	{
		init();
		operator=( src );
	}

	/// Operator=
	CBMSDbgInfo&	operator=( const CBMSDbgInfo& src )
	{
		*_DbgData = *src._DbgData;
		return *this;
	}

	/// Destructor
	~CBMSDbgInfo()
	{
		delete _DbgData;
		_DbgData = NULL;
	}

#endif
	void swap(CBMSDbgInfo &other)
	{
#ifdef NL_DEBUG
		std::swap(_DbgData, other._DbgData);
#else
		nlunreferenced(other);
#endif
	}

	/// Add a serial event at the end
	void	addSerial( uint32 bitpos, uint32 size, TBMSSerialInfo::TSerialType type )
	{
#ifdef NL_DEBUG
		if ( ! _DbgData->AddEventIsEnabled )
		{
			_DbgData->NextSymbol = NULL;
			return;
		}

		TBMSSerialInfo serialItem( bitpos, size, type, _DbgData->NextSymbol );
		_DbgData->List.push_back( serialItem );
		_DbgData->NextSymbol = NULL;
#else
		nlunreferenced(bitpos);
		nlunreferenced(size);
		nlunreferenced(type);
#endif
	}

	/// Add a serial event in the middle
	void	addPoke( uint32 bitpos, uint32 size, TBMSSerialInfo::TSerialType type )
	{
#ifdef NL_DEBUG
		if ( ! _DbgData->AddEventIsEnabled )
		{
			_DbgData->NextSymbol = NULL;
			return;
		}

		TBMSSerialInfo serialItem( bitpos, size, type, _DbgData->NextSymbol );

		/// Find where to add it
		bool found = false;
		TBMSSerialInfoList::iterator itl;
		for ( itl=_DbgData->List.begin(); itl!=_DbgData->List.end(); ++itl )
		{
			if ( (*itl).BitPos == bitpos )
			{
				// Found, replace reserved by poked
				(*itl) = serialItem;
				found = true;
				break;
			}
		}
		if ( ! found )
		{
			nlwarning( "Missing reserve() corresponding to poke()" );
		}
		_DbgData->NextSymbol = NULL;
#else
		nlunreferenced(bitpos);
		nlunreferenced(size);
		nlunreferenced(type);
#endif
	}

	/// Set the symbol for the next event that will be added (optional)
	void	setSymbolOfNextSerialEvent( const char *symbol )
	{
#ifdef NL_DEBUG
		_DbgData->NextSymbol = symbol;
#else
		nlunreferenced(symbol);
#endif
	}

	/// Clear
	void	clear()
	{
#ifdef NL_DEBUG
		_DbgData->List.clear();
#endif
	}

	/// Begin a browsing session of serial events, addSerial()/addPoke() is now disabled
	void		beginEventBrowsing()
	{
#ifdef NL_DEBUG
		_DbgData->CurrentBrowsedItem = 0;
		_DbgData->AddEventIsEnabled = false;
#endif
	}

	/// End a browsing session of serial events, and reenable addSerial()/addPoke()
	void		endEventBrowsing()
	{
#ifdef NL_DEBUG
		_DbgData->AddEventIsEnabled = true;
#endif
	}

	/// Return an eventId of serial event, or "" (and eventId -1) if nothing found at the specified bitpos
	std::string	getEventIdAtBitPos( uint32 bitpos, sint32 *eventId )
	{
#ifdef NL_DEBUG
		if ( _DbgData->CurrentBrowsedItem < _DbgData->List.size() )
		{
			if ( bitpos == _DbgData->List[_DbgData->CurrentBrowsedItem].BitPos ) // works only with a vector!
			{
				*eventId = (sint32)_DbgData->CurrentBrowsedItem;
				++_DbgData->CurrentBrowsedItem;
				return toString( "(%u)", _DbgData->CurrentBrowsedItem - 1 );
			}
			//nlassert( bitpos < (*_List)[_CurrentBrowsedItem].BitPos ); // occurs if stream overflow
		}
#else
		nlunreferenced(bitpos);
#endif
		*eventId = -1;
		return std::string();
	}

	/// Return full info about a serial event, or "" if eventId is -1
	std::string getEventLegendAtBitPos( CBitMemStream& bms, sint32 eventId );

private:

#ifdef NL_DEBUG
	/// Explicit init
	void	init()
	{
		_DbgData = new TBMSDbgInfoData();
	}

	/// List of serials
	TBMSDbgInfoData			*_DbgData;
#endif
};


/**
 * Bit-oriented memory stream
 *
 * How output mode works:
 * In CMemStream, _BufPos points to the end of the stream, where to write new data.
 * In CBitMemStream, _BufPos points to the last byte of the stream, where to write new data.
 * Then _FreeBits tells the number of bits not used in this byte (i.e. (8-_FreeBits) is the
 * number of bits already filled inside this byte).
 * The minimum buffer size is 1: when nothing has been written yet, the position is at beginning
 * and _FreeBits is 8.
 *
 * How input mode works:
 * Same as in CMemStream, but some bits may be free in the last byte (the information about
 * how many is not stored in the CBitMemStream object, the reader needs to know the format of
 * the data it reads).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001, 2003
 */
class CBitMemStream : public CMemStream
{
public:

	/// Constructor
	CBitMemStream( bool inputStream=false, uint32 defaultcapacity=32 );

	/// Copy constructor
	CBitMemStream( const CBitMemStream& other );

	/// Assignment operator
	CBitMemStream&	operator=( const CBitMemStream& other )
	{
		CMemStream::operator=( other );
		_FreeBits = other._FreeBits;
		_DbgInfo = other._DbgInfo;
		return *this;
	}

	// Echange status and memory data
	void swap(CBitMemStream &other);

	/**
	 * Set the position at the beginning. In output mode, the method ensures the buffer
	 * contains at least one blank byte to write to.
	 *
	 * If you are using the stream only in output mode, you can use this method as a faster version
	 * of clear() *if you don't serialize pointers*.
	 */
	virtual void		resetBufPos()
	{
		// This is ensured in CMemStream::CMemStream() and CMemStream::clear()
		//if ( (!isReading()) && _Buffer.empty() )
		///	_Buffer.resize( 8 ); // at least 8 bytes
		nlassert( ! ((!isReading()) && _Buffer.getBuffer().empty()) );

		CMemStream::resetBufPos();
		if ( !isReading() )
//			*_BufPos = 0; // partial prepareNextByte()
			*(_Buffer.getBufferWrite().getPtr()+_Buffer.Pos) = 0;
		_FreeBits = 8;
		_DbgInfo.clear();
	}

	/** Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written
	 * (the last byte may not be full, it may have free bits, see
	 * also getPosInBit()).
	 */
	virtual uint32	length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			if ( _FreeBits == 8 )
				return lengthS();
			else
				return lengthS() + 1;
		}
	}

	/// Transforms the message from input to output or from output to input
	virtual void	invert()
	{
		if ( ! isReading() )
		{
//			++_BufPos; // write->read: extend to keep the last byte inside the payload
			++_Buffer.Pos; // write->read: extend to keep the last byte inside the payload
		}
		CMemStream::invert();
		if ( ! isReading() )
		{
#ifdef NL_DEBUG
//			nlassert( _BufPos == _Buffer.getPtr()+_Buffer.size() );
			nlassert( _Buffer.Pos == _Buffer.getBuffer().size() );
#endif
//			--_BufPos; // read->write: set the position on the last byte, not at the end as in CMemStream::invert()
			--(_Buffer.Pos); // read->write: set the position on the last byte, not at the end as in CMemStream::invert()
		}
		// Keep the same _FreeBits
	}

	/// Clears the message
	virtual void	clear()
	{
		CMemStream::clear();
		resetBufPos();
	}

	/// Returns the number of bit from the beginning of the buffer (in bit)
	sint32	getPosInBit() const
	{
//		return (_BufPos - _Buffer.getPtr() + 1)*8 - _FreeBits;
		return (_Buffer.Pos + 1)*8 - _FreeBits;
	}

	/// Returns the stream as a string with 0 and 1.
	void			displayStream( const char *title="", CLog *log = NLMISC::DebugLog );

	/// See doc in CMemStream::fill()
	void			fill( const uint8 *srcbuf, uint32 len )
	{
		_FreeBits = 8;
		_DbgInfo.clear();
		CMemStream::fill( srcbuf, len );
	}

	/// See doc in CMemStream::bufferToFill()
	virtual uint8		*bufferToFill( uint32 msgsize )
	{
		_FreeBits = 8;
		_DbgInfo.clear();
		return CMemStream::bufferToFill( msgsize );
	}

	/// Append the contents of a bitmemstream at the end of our bitmemstream (precondition: !isReading())
	void			append( const CBitMemStream& newBits );

	/// Serialize a buffer
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Serialize one bit
	virtual void	serialBit( bool& bit );

#ifdef LOG_ALL_TRAFFIC
	void			_serialAndLog( const char *argstr, uint32& value, uint nbits );
	void			_serialAndLog( const char *argstr, uint64& value, uint nbits );
	void			_serialBitAndLog( const char *argstr, bool& bit );
#endif

	/**
	 * Serialize only the nbits lower bits of value (nbits range: [1..32])
	 * When using this method, always leave resetvalue to true.
	 */
	void			serial( uint32& value, uint nbits, bool resetvalue=true )
	{
		_DbgInfo.addSerial( getPosInBit(), nbits, TBMSSerialInfo::U );
		internalSerial( value, nbits, resetvalue );
	}

	/**
	 * Serialize only the nbits lower bits of 64-bit value (nbits range: [1..64])
	 */
	void			serial( uint64& value, uint nbits )
	{
		_DbgInfo.addSerial( getPosInBit(), nbits, TBMSSerialInfo::U64 );
		internalSerial( value, nbits );
	}

	/**
	 * Same as CMemStream::reserve(). Warning, the return value is a byte pos (not bitpos)!
	 * Consider using reserveBits() instead.
	 */
	sint32			reserve( uint byteLen );

	/**
	 * In a output bit stream, serialize nbits bits (no matter their value).
	 * Works even if the number of bits to add is larger than 64. See also poke() and pokeBits().
	 */
	void			reserveBits( uint nbits );

	/*
	 * Rewrite the nbbits lowest bits of a value at the specified position bitpos of the current output bit stream.
	 *
	 * Preconditions:
	 * - bitpos+nbbits <= the current length in bit of the stream.
	 * - The bits poked must have been reserved by reserveBits() (i.e. set to 0)
	 */
	void			poke( uint32 value, uint bitpos, uint nbits );

	/**
	 * Rewrite the bitfield at the specified position bitpos of the current output bit stream.
	 * The size of the bitfield is *not* written into the stream (unlike serialCont()).
	 * Precondition: bitpos+bitfield.size() <= the current length in bit of the stream. See also reserveBits().
	 */
	void			pokeBits( const NLMISC::CBitSet& bitfield, uint bitpos );

	/**
	 * Read bitfield.size() bits from the input stream to fill the bitfield.
	 * It means you have to know the size and to resize the bitfield yourself.
	 */
	void			readBits( NLMISC::CBitSet& bitfield );

	/// Display the bits of the stream just before the specified bitpos (or current pos if -1)
	void			displayLastBits( sint nbits, sint bitpos=-1, NLMISC::CLog *log=NLMISC::DebugLog );

	/// Return a string showing the serial item
	std::string		getSerialItem( const TBMSSerialInfo& serialItem );

	/// Template serialisation (should take the one from IStream)
    template<class T>
	void			serial(T &obj)							{ obj.serial(*this); }

	// CMemStream::serialCont() will call CBitMemStream's virtual serialBuffer()
	template<class T>
	void			serialCont(std::vector<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{CMemStream::serialCont(cont);}

	/*template<class T0,class T1>
	void			serial(T0 &a, T1 &b)
	{ serial(a); serial(b);}
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c)
	{ serial(a); serial(b); serial(c);}
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d)
	{ serial(a); serial(b); serial(c); serial(d);}
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e)
	{ serial(a); serial(b); serial(c); serial(d); serial(e);}
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f)
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}*/

	/** \name Base type serialisation.
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{


/*
#define	serialAdapt64( b, type ) \
	uint32 ubl=0, ubh=0; \
	if ( isReading() ) \
	{ \
		serial( ubh, sizeof(uint32)*8 ); \
		serial( ubl, sizeof(uint32)*8 ); \
		b = (((type)ubh)<<32)+ubl; \
	} \
	else \
	{ \
		ubh = (uint32)(b>>32); \
		ubl = (uint32)(b); \
		serial( ubh, sizeof(uint32)*8 ); \
		serial( ubl, sizeof(uint32)*8 ); \
	}
*/

#ifdef LOG_ALL_TRAFFIC
	void			_serialAndLog(const char *argstr, uint8 &b) { serialAdaptAndLog( argstr, b, uint8 ); }
	void			_serialAndLog(const char *argstr, sint8 &b) { serialAdaptAndLog( argstr, b, sint8 ); }
	void			_serialAndLog(const char *argstr, uint16 &b) { serialAdaptAndLog( argstr, b, uint16 ); }
	void			_serialAndLog(const char *argstr, sint16 &b) { serialAdaptAndLog( argstr, b, sint16 ); }
	void			_serialAndLog(const char *argstr, uint32 &b) { serialAdaptAndLog( argstr, b, uint32 ); }
	void			_serialAndLog(const char *argstr, sint32 &b) { serialAdaptAndLog( argstr, b, sint32 ); }
	void			_serialAndLog(const char *argstr, uint64 &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, sint64 &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, float &b);
	void			_serialAndLog(const char *argstr, double &b) { serialAdapt64AndLog( argstr, b ); }
	void			_serialAndLog(const char *argstr, bool &b) { _serialBitAndLog( argstr, b ); }
#ifndef NL_OS_CYGWIN
	virtual void	_serialAndLog(const char *argstr, char &b) { serialAdaptAndLog( argstr, b, char ); }
#endif
#endif

	virtual void	serial(uint8 &b) { serialAdapt( b, uint8 ); }
	virtual void	serial(sint8 &b) { serialAdapt( b, sint8 ); }
	virtual void	serial(uint16 &b) { serialAdapt( b, uint16 ); }
	virtual void	serial(sint16 &b) { serialAdapt( b, sint16 ); }
	virtual void	serial(uint32 &b) { serialAdapt( b, uint32 ); }
	virtual void	serial(sint32 &b) { serialAdapt( b, sint32 ); }
	virtual void	serial(uint64 &b) { serialAdapt64( b ); }
	virtual void	serial(sint64 &b) { serialAdapt64( b ); }
	virtual void	serial(float &b);
	virtual void	serial(double &b) { serialAdapt64( b ); }
	virtual void	serial(bool &b) { serialBit( b ); }
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) { serialAdapt( b, char ); }
#endif

	virtual void	serial(std::string &b);
	virtual void	serial(ucstring &b);

	virtual void	serial(CBitMemStream &b) { serialMemStream(b); }
	virtual void	serialMemStream(CMemStream &b);


	//@}

	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont);

protected:

	/**
	 * Helper for serial(uint32,uint)
	 */
	void			internalSerial( uint32& value, uint nbits, bool resetvalue=true );

	/**
	 * Helper for serial(uint64,uint)
	 */
	void			internalSerial( uint64& value, uint nbits )
	{
		if ( nbits > 32 )
		{
			if ( isReading() )
			{
				// Reset and read MSD
				uint32 msd = 0;
				internalSerial( msd, nbits-32 );
				value = (uint64)msd << 32;
				// Reset and read LSD
				internalSerial( (uint32&)value, 32 );
			}
			else
			{
				// Write MSD
				uint32 msd = (uint32)(value >> 32);
				internalSerial( msd, nbits-32 );
				// Write LSD
				internalSerial( (uint32&)value, 32 );
			}
		}
		else
		{
			if ( isReading() )
			{
				// Reset MSB (=0 is faster than value&=0xFFFFFFFF)
				value = 0;
			}
			// Read or write LSB
			internalSerial( (uint32&)value, nbits );
		}
	}

	/**
	 * Prepare next byte for writing.
	 *
	 * Preconditions:
	 * - See the preconditions of increaseBufferIfNecessary() and pointNextByte()
	 *
	 * Postconditions:
	 * - See the postconditions of increaseBufferIfNecessary() and pointNextByte()
	 * - The new pointed byte is 0
	 */
	void			prepareNextByte()
	{
		pointNextByte();
		increaseBufferIfNecessary();
//		*_BufPos = 0;
		*(_Buffer.getBufferWrite().getPtr() + _Buffer.Pos) = 0;
	}

	/**
	 * Point the beginning of the byte after _BufPos
	 *
	 * Preconditions
	 * - The last written byte, at pos _BufPos, is fully written (but _FreeBits may not be updated yet)
	 *
	 * Postconditions
 	 * - The pos was incremented by 1, _FreeBits is 8
	 */
	void			pointNextByte()
	{
#ifdef NL_DEBUG
		nlassert( !isReading() );
#endif
		_FreeBits = 8;
//		++_BufPos;
		++_Buffer.Pos;
	}

	/**
	 * Increase the size of the buffer if necessary (outpout bit stream)
	 *
	 * Preconditions:
	 * - The stream is in output mode (!isReading())
	 * - resetBufPos() must have been called since construction
	 * - getPos() <= _Buffer.size()
	 *
	 * Postconditions:
	 * - getPos() < _Buffer.size()
	 */
	void			increaseBufferIfNecessary()
	{
#ifdef NL_DEBUG
//		nlassert( (!isReading()) && (!_Buffer.empty()) );
		nlassert( (!isReading()) && (!_Buffer.getBuffer().empty()) );
//		nlassert( _BufPos <= _Buffer.getPtr() + _Buffer.size() );
		nlassert( _Buffer.Pos <= _Buffer.getBuffer().size() );
#endif
//		uint32 bytepos = _BufPos - _Buffer.getPtr();
//		uint32 bytepos = _BufPos;
//		if ( bytepos == _Buffer.size() )
		if ( _Buffer.Pos == _Buffer.getBuffer().size() )
		{
//			_Buffer.resize( bytepos * 2 );
			_Buffer.getBufferWrite().resize( _Buffer.Pos * 2 );
//			_BufPos = _Buffer.getPtr() + bytepos; // don't change the pos but update pointer (needed because the buffer may have moved when reallocating)
		}
	}

	/**
	* Helper for poke(), to write a value inside an output stream (works because reserveBits sets to 0)
	* Warning: if _FreeBits == 8, increments _BufPos.
	*/
	void			serialPoke( uint32 value, uint nbits );

	/// Number of bits unused at the current pos. If 8, means the current pos if full and we need to increment the pos!
	uint			_FreeBits; // From 8 downto 1

	/// Debug details about what was serialised
	CBMSDbgInfo		_DbgInfo;
};


/// Display a part of a bitmemstream
void	displayBitStream( const CBitMemStream& msg, sint beginbitpos, sint endbitpos, NLMISC::CLog *log=NLMISC::DebugLog );


/*
 * Return full info about a serial event, or "" if eventId is -1
 */
inline std::string CBMSDbgInfo::getEventLegendAtBitPos( CBitMemStream& bms, sint32 eventId )
{
#ifdef NL_DEBUG
	if ( eventId != -1 )
	{
		nlassert( eventId < (sint32)_DbgData->List.size() );
		TBMSSerialInfo& serialItem = _DbgData->List[eventId]; // works only with a vector!
		return toString( "(%d) BitPos %3u Type %s BitSize %2u Value %s %s\n",
					eventId, serialItem.BitPos, SerialTypeToCStr[serialItem.Type], serialItem.BitSize,
					bms.getSerialItem( serialItem ).c_str(), (serialItem.Symbol!=NULL)?serialItem.Symbol:"" );
	}
#else
	nlunreferenced(bms);
	nlunreferenced(eventId);
#endif

	return std::string();
}


} // NLMISC


#endif // NL_BIT_MEM_STREAM_H

/* End of bit_mem_stream.h */
