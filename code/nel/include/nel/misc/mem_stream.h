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

#ifndef NL_MEM_STREAM_H
#define NL_MEM_STREAM_H

#include "types_nl.h"
#include "stream.h"
#include "object_vector.h"
#include "fast_mem.h"
#include "smart_ptr.h"

#include <algorithm>

namespace NLMISC
{

/// Exception class for CMemStream
struct EMemStream : public NLMISC::EStream
{
	EMemStream( const std::string& str ) : EStream( str ) {}
};


/** This class implement a copy on write behavior for memory stream buffer.
 *	The goal is to allow buffer sharing between CMemStream object, so that
 *	a CMemStream can be copied or passed by value as input/output parameter
 *	without copying the data buffer (thus making a CMemStrem copy almost free).
 *	This class reference a TMemStreamBuffer object with a smart pointer,
 *	when some code call the getBufferWrite() method to obtain write access
 *	to the memory buffer, if the ref count is more than 1, then we make a copy
 *	of the internal buffer for the current object.
 *
 *	\Author Boris Boucher
 */
class CMemStreamBuffer
{
public:
	typedef CObjectVector<uint8, false>	TBuffer;
private:
	struct TMemStreamBuffer : public CRefCount
	{
		// the buffer himself
		TBuffer	_Buffer;
	};

	typedef CSmartPtr<TMemStreamBuffer>	TMemStreamBufferPtr;

	TMemStreamBufferPtr		_SharedBuffer;

public:

	mutable	uint32	Pos;

	/// constructor, allocate a shared buffer
	CMemStreamBuffer()
	{
		_SharedBuffer = new TMemStreamBuffer;
		Pos = 0;
	}


	/// Return a read accessor to the buffer
	const TBuffer &getBuffer() const
	{
		return _SharedBuffer->_Buffer;
	}

	/** Return a write accessor to the buffer, create acopy it if more than
	 *	one CMemStreamBuffer reference then buffer.
	 */
	TBuffer &getBufferWrite()
	{
		if (_SharedBuffer->getRefCount() > 1)
		{
			// we need to duplicate the buffer
			_SharedBuffer = new TMemStreamBuffer(*_SharedBuffer);
		}
		return _SharedBuffer->_Buffer;
	}

	/// Exchange the buffer of two CMemStreamBuffer (just swap memory pointer)
	void swap(CMemStreamBuffer &other)
	{
		std::swap(_SharedBuffer, other._SharedBuffer);
		std::swap(Pos, other.Pos);
	}

};


/**
 * Memory stream.
 *
 * How output mode works:
 * The buffer size is increased by factor 2. It means the stream can be smaller than the buffer size.
 * The size of the stream is the current position in the stream (given by lengthS() which is equal
 * to getPos()), because data is always written at the end (except when using poke()).
 * About seek() particularities: see comment of the seek() method.
 *
 * buffer() ----------------------------------- getPos() ---------------- size()
 *              data already serialized out        |
 *                                              length() = lengthS()
 *
 * How input mode works:
 * The stream is exactly the buffer (the size is given by lengthR()). Data is read inside the stream,
 * at the current position (given by getPos()). If you try to read data while getPos() is equal to
 * lengthR(), you'll get an EStreamOverflow exception.
 *
 * buffer() ----------------------------------- getPos() ------------------------- size()
 *              data already serialized in                   data not read yet      |
 *                                                                               length() = lengthR()
 *
 * \seealso CBitMemStream
 * \seealso NLNET::CMessage
 * \author Olivier Cado, Vianney Lecroart
 * \author Nevrax France
 * \date 2000, 2002
 */
class CMemStream : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMemStream( bool inputStream=false, bool stringmode=false, uint32 defaultcapacity=0 ) :
		NLMISC::IStream( inputStream ), _StringMode( stringmode )
	{
		_DefaultCapacity = std::max( (uint32)defaultcapacity, (uint32)16 ); // prevent from no allocation
		_Buffer.getBufferWrite().resize (_DefaultCapacity);
		_Buffer.Pos = 0;
	}

	/// Copy constructor
	CMemStream( const CMemStream& other ) :
		IStream (other)
	{
		operator=( other );
	}

	/// Assignment operator
	CMemStream&		operator=( const CMemStream& other )
	{
		IStream::operator= (other);
		_Buffer = other._Buffer;
		_StringMode = other._StringMode;
		_DefaultCapacity = other._DefaultCapacity;
		return *this;
	}

	/// allocated memory exchange
	void swap(CMemStream &other);

	/// Set string mode
	void			setStringMode( bool stringmode ) { _StringMode = stringmode; }

	/// Return string mode
	bool			stringMode() const { return _StringMode; }

	/** Returns a readable string to display it to the screen. It's only for debugging purpose!
	 * Don't use it for anything else than to debugging, the string format could change in the future.
	 * \param hexFormat If true, display all bytes in hexadecimal, else display as chars (above 31, otherwise '.')
	 */
	std::string		toString( bool hexFormat=false ) const;

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit);

	/**
	 * Moves the stream pointer to a specified location.
	 *
	 * Warning: in output mode, seek(end) does not point to the end of the serialized data,
	 * but on the end of the whole allocated buffer (see size()).
	 * If you seek back and want to return to the end of the serialized data, you have to
	 * store the position (a better way is to use reserve()/poke()).
	 *
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation:
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool	seek (sint32 offset, TSeekOrigin origin) const;

	/**
	 * Get the location of the stream pointer.
	 *
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation:
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32	getPos () const
	{
		return sint32(_Buffer.Pos);
	}

	/**
	 * When writing, skip 'len' bytes and return the position of the blank space for a future poke().
	 * Warning: this has nothing to do with the semantics of reserve() in std::vector!
	 */
	sint32			reserve( uint len )
	{
		sint32 pos = sint32(_Buffer.Pos);
		if ( ! isReading() )
		{
			increaseBufferIfNecessary( len );
			_Buffer.Pos += len;
		}
		return pos;
	}

	/**
	 * When writing, fill a value previously reserved by reserve()
	 * (warning: you MUST have called reserve() with sizeof(T) before poking).
	 * Usually it's an alternative to use serialCont with a vector.
	 * Example:
	 *		uint8 counter=0;
	 *		sint32 counterPos = msgout.reserve( sizeof(counter) );
	 *		counter = serialSelectedItems( msgout );
	 *		msgout.poke( counter, counterPos );
	 */
	template <class T>
	void			poke( T value, sint32 pos )
	{
		if ( ! isReading() )
		{
			uint8 *pokeBufPos = _Buffer.getBufferWrite().getPtr() + pos;
			nlassert( pokeBufPos + sizeof(T) <= pokeBufPos+_Buffer.Pos );
			*(T*)pokeBufPos = value;
		}
	}

	/// Clears the message
	virtual void	clear()
	{
		resetPtrTable();
		_Buffer.getBufferWrite().clear();
		if (!isReading())
		{
			_Buffer.getBufferWrite().resize (_DefaultCapacity);
		}
		_Buffer.Pos = 0;
	}

	/**
	 * Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written.
	 */
	virtual uint32	length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			return lengthS();
		}
	}

	/// Returns the size of the buffer (can be greater than the length, especially in output mode)
	uint32			size() const
	{
		return _Buffer.getBuffer().size();
	}

	/** Returns a pointer to the message buffer (read only)
	 * Returns NULL if the buffer is empty
	 */
	virtual const uint8		*buffer() const
	{
		return _Buffer.getBuffer().getPtr();
	}


	/**
	 * When you fill the buffer externaly (using bufferAsVector) you have to reset the BufPos calling this method
	 *
	 * If you are using the stream only in output mode, you can use this method as a faster version
	 * of clear() *if you don't serialize pointers*.
	 */
	virtual void		resetBufPos() { _Buffer.Pos = 0; }

	/**
	 * Resize the message buffer and fill data at position 0.
	 * Input stream: the current position is set at the beginning;
	 * Output stream: the current position is set after the filled data.
	 */
	void			fill( const uint8 *srcbuf, uint32 len )
	{
		if (len == 0) return;

		_Buffer.getBufferWrite().resize( len );
		CFastMem::memcpy( _Buffer.getBufferWrite().getPtr(), srcbuf, len );
		if (isReading())
		{
			_Buffer.Pos = 0;
		}
		else
		{
			_Buffer.Pos = _Buffer.getBuffer().size();
		}
	}

	/**
	 * Resize the buffer.
	 * Warning: the position is unchanged, only the size is changed.
	 */
	void			resize (uint32 size);

	/**
	 * Resize the stream with the specified size, set the current position at the beginning
	 * of the stream and return a pointer to the stream buffer.
	 *
	 * Precondition: the stream is an input stream.
	 *
	 * Suggested usage: construct an input stream, resize and get the buffer using bufferToFillAndRead(),
	 * fill it with raw data using any filling function (warning: don't fill more than 'msgsize'
	 * bytes!), then you are ready to read, using serial(), the data you've just filled.
	 */
	virtual uint8		*bufferToFill( uint32 msgsize )
	{
#ifdef NL_DEBUG
		nlassert( isReading() );
#endif
		if ( msgsize == 0 )
			return NULL;

		_Buffer.getBufferWrite().resize( msgsize );
		_Buffer.Pos = 0;
		return _Buffer.getBufferWrite().getPtr();
	}

	/**
	 * Transforms the message from input to output or from output to input
	 *
	 * Precondition:
	 * - If the stream is in input mode, it must not be empty (nothing filled), otherwise the position
	 *   will be set to the end of the preallocated buffer (see DefaultCapacity).
	 * Postcondition:
	 * - Read->write, the position is set at the end of the stream, it is possible to add more data
	 - - Write->Read, the position is set at the beginning of the stream
	 */
	virtual void	invert()
	{
		if ( isReading() )
		{
			// In->Out: We want to write (serialize out) what we have read (serialized in)
			uint32 sizeOfReadStream = lengthR();
			resetPtrTable();
			setInOut( false );
			_Buffer.Pos = sizeOfReadStream;
		}
		else
		{
			// Out->In: We want to read (serialize in) what we have written (serialized out)
			resetPtrTable();
			setInOut( true );
			// TODO : is it necessary ?
			_Buffer.getBufferWrite().resize (_Buffer.Pos);
			_Buffer.Pos = 0;
		}
	}

	/// Force to reset the ptr table
	void			resetPtrTable() { IStream::resetPtrTable() ; }

	/// Increase the buffer size if 'len' can't enter, otherwise, do nothing
#ifdef NL_OS_WINDOWS
	__forceinline
#endif
	void			increaseBufferIfNecessary(uint32 len)
	{
		uint32 oldBufferSize = _Buffer.getBuffer().size();
		if (_Buffer.Pos + len > oldBufferSize)
		{
			// need to increase the buffer size
			_Buffer.getBufferWrite().resize(oldBufferSize*2 + len);
		}
	}


	template <class T> void fastSerial (T &val)
	{
#ifdef NL_LITTLE_ENDIAN
		if(isReading())
		{
			// Check that we don't read more than there is to read
			if ( lengthS()+sizeof(T) > length() ) // calls virtual length (cf. sub messages)
				throw EStreamOverflow();
			// Serialize in
			val = *(T*)(_Buffer.getBuffer().getPtr() + _Buffer.Pos);
		}
		else
		{
			increaseBufferIfNecessary (sizeof(T));
			*(T*)(_Buffer.getBufferWrite().getPtr() + _Buffer.Pos) = val;
		}
		_Buffer.Pos += sizeof (T);
#else // NL_LITTLE_ENDIAN
		IStream::serial( val );
#endif // NL_LITTLE_ENDIAN
	}

	template <class T>
	void			fastWrite( const T& value )
	{
		//nldebug( "MEMSTREAM: Writing %u-byte value in %p at pos %u", sizeof(value), this, _BufPos - _Buffer.getPtr() );
		increaseBufferIfNecessary (sizeof(T));
		*(T*)(_Buffer.getBufferWrite().getPtr() + _Buffer.Pos) = value;

		_Buffer.Pos += sizeof (T);
	}

	template <class T>
	void			fastRead( T& value )
	{
		//nldebug( "MEMSTREAM: Reading %u-byte value in %p at pos %u", sizeof(value), this, _BufPos - _Buffer.getPtr() );
		// Check that we don't read more than there is to read
		if ( lengthS()+sizeof(value) > length() ) // calls virtual length (cf. sub messages)
		{
			throw EStreamOverflow();
		}
		// Serialize in
		value = *(T*)(_Buffer.getBuffer().getPtr() + _Buffer.Pos);
		_Buffer.Pos += sizeof(value);
	}


	/// Template serialization (should take the one from IStream)
    template<class T>
	void			serial(T &obj) 				{ obj.serial(*this); }


	template<class T>
	void			serialCont(std::vector<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{IStream::serialCont(cont);}
	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) {IStream::serialCont(cont);}
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) {IStream::serialCont(cont);}
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont) {IStream::serialCont(cont);}



	template<class T0,class T1>
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
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}

	/** \name Base types serialisation, redefined for string mode
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{
	virtual void	serial(uint8 &b) ;
	virtual void	serial(sint8 &b) ;
	virtual void	serial(uint16 &b) ;
	virtual void	serial(sint16 &b) ;
	virtual void	serial(uint32 &b) ;
	virtual void	serial(sint32 &b) ;
	virtual void	serial(uint64 &b) ;
	virtual void	serial(sint64 &b) ;
	virtual void	serial(float &b) ;
	virtual void	serial(double &b) ;
	virtual void	serial(bool &b) ;
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) ;
#endif
	virtual void	serial(std::string &b) ;
	virtual void	serial(ucstring &b) ;
	//@}


	///\name String-specific methods
	//@{

	/// Input: read len bytes at most from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
	uint			serialSeparatedBufferIn( uint8 *buf, uint len );

	/// Output: writes len bytes from buf into the stream
	void			serialSeparatedBufferOut( uint8 *buf, uint len );

	/// Serialisation in hexadecimal
	virtual void	serialHex(uint32 &b);

	//@}

protected:

	/// Returns the serialized length (number of bytes written or read)
	virtual uint32			lengthS() const
	{
		return _Buffer.Pos; // not calling getPos() because virtual and not const!
	}

	/**
	 * Returns the "read" message size (number of bytes to read)
	 * Do not use directly, use length() instead in reading mode (which is virtual)
	 */
	uint32			lengthR() const
	{
		return size();
	}

	/** Get the size for this stream. return 0 by default. Only implemented for input stream that know their size.
	 *	Used internally to detect OverFlow with vector<> for instance
	 */
	virtual uint			getDbgStreamSize() const;

	CMemStreamBuffer			_Buffer;

	mutable	bool				_StringMode;

	uint32						_DefaultCapacity;
};

// Input
#define readnumber(dest,thetype,digits,convfunc) \
	char number_as_cstring [digits+1]; \
	uint realdigits = serialSeparatedBufferIn( (uint8*)number_as_cstring, digits ); \
	number_as_cstring[realdigits] = '\0'; \
	dest = (thetype)convfunc( number_as_cstring );

// Output
#define writenumber(src,format,digits) \
	char number_as_cstring [digits+1]; \
	sprintf( number_as_cstring, format, src ); \
	serialSeparatedBufferOut( (uint8*)number_as_cstring, (uint)strlen(number_as_cstring) );

/*
 * atoihex
 */
inline uint32 atoihex( const char* ident )
{
	uint32 number;
	sscanf( ident, "%x", &number );
	return number;
}

inline uint32 atoui( const char *ident)
{
	return (uint32) strtoul (ident, NULL, 10);
}

static const char SEPARATOR = ' ';
static const int SEP_SIZE = 1; // the code is easier to read with that

//
// inline serial functions
//


// ======================================================================================================
inline	void		CMemStream::serial(uint8 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint8, 3, atoi ); // 255
		}
		else
		{
			writenumber( (uint16)b,"%hu", 3 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint8 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint8, 4, atoi ); // -128
		}
		else
		{
			writenumber( (sint16)b, "%hd", 4 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint16 &b)
{
	if ( _StringMode )
	{
		// No byte swapping in text mode
		if ( isReading() )
		{
			readnumber( b, uint16, 5, atoi ); // 65535
		}
		else
		{
			writenumber( b, "%hu", 5 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint16 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint16, 6, atoi ); // -32768
		}
		else
		{
			writenumber( b, "%hd", 6 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint32 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint32, 10, atoui ); // 4294967295
		}
		else
		{
			writenumber( b, "%u", 10 );
		}
	}
	else
	{
		fastSerial (b);
	}
}


// ======================================================================================================
inline	void		CMemStream::serial(sint32 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint32, 11, atoi ); // -2147483648
		}
		else
		{
			writenumber( b, "%d", 11 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(uint64 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint64, 20, atoiInt64 ); // 18446744073709551615
		}
		else
		{
			writenumber( b, "%" NL_I64 "u", 20 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(sint64 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, sint64, 20, atoiInt64 ); // -9223372036854775808
		}
		else
		{
			writenumber( b, "%" NL_I64 "d", 20 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(float &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, float, 128, atof ); // ?
		}
		else
		{
			writenumber( (double)b, "%f", 128 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(double &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, double, 128, atof ); //
		}
		else
		{
			writenumber( b, "%f", 128 );
		}
	}
	else
	{
		fastSerial (b);
	}
}

// ======================================================================================================
inline	void		CMemStream::serial(bool &b)
{
	if ( _StringMode )
	{
		serialBit(b);
	}
	else
	{
		fastSerial (b);
	}
}


#ifndef NL_OS_CYGWIN
// ======================================================================================================
inline	void		CMemStream::serial(char &b)
{
	if ( _StringMode )
	{
		char buff [2];
		if ( isReading() )
		{
			serialBuffer( (uint8*)buff, 2 );
			b = buff[0];
		}
		else
		{
			buff[0] = b;
			buff[1] = SEPARATOR;
			serialBuffer( (uint8*)buff, 2 );
		}
	}
	else
	{
		fastSerial (b);
	}
}
#endif

// ======================================================================================================
inline	void		CMemStream::serial(std::string &b)
{
	if ( _StringMode )
	{
		uint32	len=0;
		// Read/Write the length.
		if(isReading())
		{
			serial(len);
			checkStreamSize((uint)len);
			/*if (len>1000000)
				throw NLMISC::EInvalidDataStream( "CMemStream/str: Trying to read a string of %u bytes", len );
			*/
			b.resize(len);
		}
		else
		{
			len= (uint32)b.size();
			if (len>1000000)
				throw NLMISC::EInvalidDataStream( "CMemStream/str: Trying to write a string of %u bytes", len );
			serial(len);
		}

		// Read/Write the string.
		for(uint i=0;i!=len;++i)
			serialBuffer( (uint8*)&(b[i]), sizeof(b[i]) );

		char sep = SEPARATOR;
		serialBuffer( (uint8*)&sep, 1 );
	}
	else
	{
		if (isReading())
		{
			if (!isXML())
			{
				uint32	len=0;
				fastSerial(len);
				checkStreamSize((uint)len);
				/*
				if (len>1000000)
					throw NLMISC::EInvalidDataStream( "CMemStream: Trying to read a string of %u bytes", len );
				*/
				b.resize(len);
				if (len > 0)
				{
					// can serial all in a single call to serialBuffer, since sizeof(char) == 1
					serialBuffer((uint8 *) &b[0], len);
				}
			}
			else
			{
				IStream::serial( b );
			}
		}
		else
		{
			IStream::serial( b );
		}
	}
}


// ======================================================================================================
inline	void		CMemStream::serial(ucstring &b)
{
	if ( _StringMode )
	{
		uint32	len=0;
		// Read/Write the length.
		if(isReading())
		{
			serial(len);
			checkStreamSize((uint)len);
			/*
			if (len>1000000)
				throw NLMISC::EInvalidDataStream( "CMemStream/str: Trying to read an ucstring of %u bytes", len );
			*/
			b.resize(len);
		}
		else
		{
			len= (uint32)b.size();
			if (len>1000000)
				throw NLMISC::EInvalidDataStream( "CMemStream/str: Trying to write an ucstring of %u bytes", len );
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
}



/*
 * Serialisation in hexadecimal
 */
inline	void	CMemStream::serialHex(uint32 &b)
{
	if ( _StringMode )
	{
		if ( isReading() )
		{
			readnumber( b, uint32, 10, atoihex ); // 4294967295
		}
		else
		{
			writenumber( b, "%x", 10 );
		}
	}
	else
	{
		IStream::serial( b );
	}
}

}

#endif // NL_MEM_STREAM_H

/* End of mem_stream.h */
