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

#include "stdnet.h"

#include "nel/net/message.h"

/*#ifdef MESSAGES_PLAIN_TEXT
#pragma message( "CMessage: compiling messages as plain text" )
#else
#pragma message( "CMessage: compiling messages as binary" )
#endif*/

namespace NLNET
{

bool CMessage::_DefaultStringMode = false;

const char *LockedSubMessageError = "a sub message is forbidden";

#define FormatLong 1
#define FormatShort 0


/*
 * Constructor by name
 */
CMessage::CMessage (const std::string &name, bool inputStream, TStreamFormat streamformat, uint32 defaultCapacity) :
	NLMISC::CMemStream (inputStream, false, defaultCapacity),
	_Type(OneWay), _SubMessagePosR(0), _LengthR(0), _HeaderSize(0xFFFFFFFF), _TypeSet (false)
{
	init( name, streamformat );
}


/*
 * Utility method
 */
void CMessage::init( const std::string &name, TStreamFormat streamformat )
{
	if ( streamformat == UseDefault )
	{
		setStringMode( _DefaultStringMode );
	}
	else
	{
		setStringMode( streamformat == String );
	}

	if (!name.empty())
		setType (name);
}


/*
 * Constructor with copy from CMemStream
 */
CMessage::CMessage (NLMISC::CMemStream &memstr) :
	NLMISC::CMemStream( memstr ),
	_Type(OneWay), _SubMessagePosR(0), _LengthR(0), _HeaderSize(0xFFFFFFFF), _TypeSet (false)
{
	sint32 pos = getPos();
	bool reading = isReading();
	if ( reading ) // force input mode to read the type
		readType(); // sets _TypeSet, _HeaderSize and _LengthR
	else
		invert(); // calls readType()
	if ( ! reading )
		invert(); // set output mode back if necessary
	seek( pos, begin ); // sets the same position as the one in the memstream
}


/*
 * Copy constructor
 */
CMessage::CMessage (const CMessage &other)
	:	CMemStream(),
		_TypeSet(false)
{
	operator= (other);
}

/*
 * Assignment operator
 */
CMessage &CMessage::operator= (const CMessage &other)
{
//	nlassertex( (!other.isReading()) || (!other.hasLockedSubMessage()), ("Storing %s", LockedSubMessageError) );
	nlassertex( (!isReading()) || (!hasLockedSubMessage()), ("Assigning %s", LockedSubMessageError) );
	if ( other.hasLockedSubMessage() )
	{
		assignFromSubMessage(other);
	}
	else
	{

		CMemStream::operator= (other);
		_Type = other._Type;
		_TypeSet = other._TypeSet;
		_Name = other._Name;
		_HeaderSize = other._HeaderSize;
		_SubMessagePosR = other._SubMessagePosR;
		_LengthR = other._LengthR;
	}

	return *this;
}

void CMessage::swap(CMessage &other)
{
	nlassert( !hasLockedSubMessage() );
	CMemStream::swap(other);
	_Name.swap(other._Name);
	std::swap(_SubMessagePosR, other._SubMessagePosR);
	std::swap(_LengthR, other._LengthR);
	std::swap(_HeaderSize, other._HeaderSize);
	std::swap(_TypeSet, other._TypeSet);
	std::swap(_Type, other._Type);
}


/**
 * Similar to operator=, but makes the current message contain *only* the locked sub message in msgin
 * or the whole msgin if it is not locked
 *
 * Preconditions:
 * - msgin is an input message (isReading())
 * - The current message is blank (new or reset with clear())
 *
 * Postconditions:
 * - If msgin has been locked using lockSubMessage(), the current message contains only the locked
 *   sub message in msgin, otherwise the current message is exactly msgin
 * - The current message is an input message, it is not locked
 */
void CMessage::assignFromSubMessage( const CMessage& msgin )
{
	nlassert( msgin.isReading() );
	nlassert( ! _TypeSet );
	if ( ! isReading() )
		invert();

	if ( msgin.hasLockedSubMessage() )
	{
		fill( msgin.buffer(), msgin._LengthR );
		readType();
		seek( msgin.getPos(), IStream::begin );
	}
	else
	{
		operator=( msgin );
	}
}


/*
 * Sets the message type as a string and put it in the buffer if we are in writing mode
 */
void CMessage::setType (const std::string &name, TMessageType type)
{
	// check if we already do a setType ()
	nlassert (!_TypeSet);
	// don't accept empty string
	nlassert (!name.empty ());

	_Name = name;
	_Type = type;

	if (!isReading ())
	{
		// check if they don't already serial some stuffs
		nlassert (length () == 0);

		// if we can send the id instead of the string, "just do it" (c)nike!
		//NLMISC::CStringIdArray::TStringId id = _SIDA->getId (name);

		// Force binary mode for header
		bool msgmode = _StringMode;
		_StringMode = false;

		// debug features, we number all packet to be sure that they are all sent and received
		// \todo remove this debug feature when ok
		// this value will be fill after in the callback function
		uint32 zeroValue = 123;
		serial (zeroValue);


		TFormat format;
		format.LongFormat = FormatLong;
		format.StringMode = msgmode;
		format.MessageType = _Type;
		//nldebug( "OUT format = %hu", (uint16)format );
		serial (format);

		// End of binary header
		_StringMode = msgmode;

		serial ((std::string&)name);

		_HeaderSize = getPos ();
	}

	_TypeSet = true;
}


/*
 * Warning: MUST be of the same size than previous name!
 * Output message only.
 */
void CMessage::changeType (const std::string &name)
{
	sint32 prevPos = getPos();
	seek( sizeof(uint32)+sizeof(uint8), begin );
	serial ((std::string&)name);
	seek( prevPos, begin );
}


/*
 * Returns the size, in byte of the header that contains the type name of the message or the type number
 */
uint32 CMessage::getHeaderSize () const
{
	nlassert (_HeaderSize != 0xFFFFFFFF);
	nlassert(!hasLockedSubMessage());
	return _HeaderSize;
}


/*
 * The message was filled with an CMemStream, Now, we'll get the message type on this buffer
 */
void CMessage::readType ()
{
	nlassert (isReading ());

	// debug features, we number all packet to be sure that they are all sent and received
	// \todo remove this debug feature when ok

	// we remove the message from the message
	resetSubMessageInternals();
	const uint HeaderSize = 4;
	seek (HeaderSize, begin);
//		uint32 zeroValue;
//		serial (zeroValue);

	// Force binary mode for header
	_StringMode = false;

	TFormat format;
	serial (format);
	//nldebug( "IN format = %hu", (uint16)format );

	// Set mode for the following of the buffer
	_StringMode = format.StringMode;

	std::string name;
	serial (name);
	setType (name, TMessageType(format.MessageType));
	_HeaderSize = getPos();
}


/*
 * Get the message name (input message only) and advance the current pos
 */
std::string CMessage::readTypeAtCurrentPos() const
{
	nlassert( isReading() );

	const uint HeaderSize = 4;
	seek( HeaderSize, current );

	bool sm = _StringMode;
	_StringMode = false;

	TFormat format;
	nlRead(*this, serial, format );
	bool LongFormat = format.LongFormat;
	_StringMode = format.StringMode;
	_Type = TMessageType(format.MessageType);

	if ( LongFormat )
	{
		std::string name;
		nlRead(*this, serial, name );
		_StringMode = sm;
		return name;
	}
	else
		nlerror( "Id not supported" );


	_StringMode = sm;
	return "";
}


// Returns true if the message type was already set
bool CMessage::typeIsSet () const
{
	return _TypeSet;
}

// Clear the message. With this function, you can reuse a message to create another message
void CMessage::clear ()
{
	nlassertex( (!isReading()) || (!hasLockedSubMessage()), ("Clearing %s", LockedSubMessageError) );

	CMemStream::clear ();
	_TypeSet = false;
	_SubMessagePosR = 0;
	_LengthR = 0;
}

/*
 * Returns the type name in string if available. Be sure that the message have the name of the message type
 */
std::string CMessage::getName () const
{
	if ( hasLockedSubMessage() )
	{
		CMessage& notconstMsg = const_cast<CMessage&>(*this);
		sint32 savedPos = notconstMsg.getPos();
		uint32 subPosSaved = _SubMessagePosR;
		uint32 lenthRSaved = _LengthR;
		const_cast<uint32&>(_SubMessagePosR) = 0;
//		const_cast<uint32&>(_LengthR) = _Buffer.size();
		const_cast<uint32&>(_LengthR) = _Buffer.getBuffer().size();
		notconstMsg.seek( subPosSaved, begin ); // not really const... but removing the const from getName() would need too many const changes elsewhere
		std::string name = notconstMsg.readTypeAtCurrentPos();
		notconstMsg.seek( subPosSaved+savedPos, begin );
		const_cast<uint32&>(_SubMessagePosR) = subPosSaved;
		const_cast<uint32&>(_LengthR) = lenthRSaved;
		return name;
	}
	else
	{
		nlassert (_TypeSet);
		return _Name;
	}
}

CMessage::TMessageType CMessage::getType() const
{
	if ( hasLockedSubMessage() )
	{
		CMessage& notconstMsg = const_cast<CMessage&>(*this);
		sint32 savedPos = notconstMsg.getPos();
		uint32 subPosSaved = _SubMessagePosR;
		uint32 lenthRSaved = _LengthR;
		const_cast<uint32&>(_SubMessagePosR) = 0;
//		const_cast<uint32&>(_LengthR) = _Buffer.size();
		const_cast<uint32&>(_LengthR) = _Buffer.getBuffer().size();
		notconstMsg.seek( subPosSaved, begin ); // not really const... but removing the const from getName() would need too many const changes elsewhere
		notconstMsg.readTypeAtCurrentPos();
		notconstMsg.seek( subPosSaved+savedPos, begin );
		const_cast<uint32&>(_SubMessagePosR) = subPosSaved;
		const_cast<uint32&>(_LengthR) = lenthRSaved;
		return _Type;
	}
	else
	{
		nlassert (_TypeSet);
		return _Type;
	}
}


/* Returns a readable string to display it to the screen. It's only for debugging purpose!
 * Don't use it for anything else than to debugging, the string format could change in the future.
 * \param hexFormat If true, display all bytes in hexadecimal
 * \param textFormat If true, display all bytes as chars (above 31, otherwise '.')
 */
std::string CMessage::toString( bool hexFormat, bool textFormat ) const
{
	//nlassert (_TypeSet);
	std::string s = "('" + _Name + "')";
	if ( hexFormat )
		s += " " + CMemStream::toString( true );
	if ( textFormat )
		s += " " + CMemStream::toString( false );
	return s;
}


/*
 * Return an input stream containing the stream beginning in the message at the specified pos
 */
NLMISC::CMemStream	CMessage::extractStreamFromPos( sint32 pos )
{
	NLMISC::CMemStream msg( true );
	sint32 len = length() - pos;
	memcpy( msg.bufferToFill( len ), buffer() + pos, len );
	return msg;
}


/*
 * Encapsulate/decapsulate another message inside the current message
 */
void	CMessage::serialMessage( CMessage& msg )
{
	if ( isReading() )
	{
		// Init 'msg' with the contents serialised from 'this'
		uint32 len;
		serial( len );
		if ( ! msg.isReading() )
			msg.invert();
		serialBuffer( msg.bufferToFill( len ), len );
		msg.readType();
		msg.invert();
		msg.seek( 0, CMemStream::end );
	}
	else
	{
		// Store into 'this' the contents of 'msg'
		uint32 len = msg.length();
		serial( len );
		serialBuffer( const_cast<uint8*>(msg.buffer()), msg.length() );
	}
}

}
