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

#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


// ======================================================================================================
// ======================================================================================================
// EStream.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
EStream::EStream( const IStream &f ) : Exception( "In Stream: " + f.getStreamName() + string(": Stream Error") )
{
	StreamName= f.getStreamName();
}

EStream::EStream( const IStream &f, const std::string& str )
 : Exception( "In Stream: " + f.getStreamName() + ": " + str )
{
	StreamName= f.getStreamName();
}

EInvalidDataStream::EInvalidDataStream(const char *msg, uint size)
: EStream( NLMISC::toString( msg, size ) )
{}


EStreamOverflow::EStreamOverflow( const char *msg, uint size )
 : EStream( NLMISC::toString( msg, size ) )
{}


// ======================================================================================================
// ======================================================================================================
// IStream.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
bool	IStream::_ThrowOnOlder=false;
bool	IStream::_ThrowOnNewer=true;


// ======================================================================================================
void	IStream::setVersionException(bool throwOnOlder, bool throwOnNewer)
{
	_ThrowOnOlder=throwOnOlder;
	_ThrowOnNewer=throwOnNewer;
}

// ======================================================================================================
void	IStream::getVersionException(bool &throwOnOlder, bool &throwOnNewer)
{
	throwOnOlder=_ThrowOnOlder;
	throwOnNewer=_ThrowOnNewer;
}



/*
 * Copy constructor
 */
IStream::IStream( const IStream& other )
{
	operator=( other );

	// By default, mode _XML is off
	_XML = false;
}


/*
 * Assignment operator
 */
IStream& IStream::operator=( const IStream& other )
{
	_InputStream = other._InputStream;
	resetPtrTable();
	return *this;
}

void IStream::swap(IStream &other)
{
	std::swap(_InputStream, other._InputStream);
	std::swap(_NextSerialPtrId, other._NextSerialPtrId);
	_IdMap.swap(other._IdMap);
	std::swap(_XML, other._XML);
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void			IStream::serialIStreamable(IStreamable* &ptr)
{
	uint64	node=0;

	// Open a node
	xmlPushBegin ("POLYPTR");

	if(isReading())
	{
		// First attribute name
		xmlSetAttrib ("id");

		serial(node);

		if(node==0)
		{
			ptr=NULL;

			// Close the node header
			xmlPushEnd ();
		}
		else
		{
			ItIdMap	it;
			it= _IdMap.find(node);

			// Test if object already created/read.
			if( it==_IdMap.end() )
			{
				// Read the class name.
				string	className;

				// Second attribute name
				xmlSetAttrib ("class");

				serial(className);

				// Close the node header
				xmlPushEnd ();

				// Construct object.
				ptr= dynamic_cast<IStreamable*> (CClassRegistry::create(className));
				if(ptr==NULL)
				#ifdef NL_DEBUG
					throw EUnregisteredClass(className);
				#else
					throw EUnregisteredClass();
				#endif


				#ifdef NL_DEBUG
					nlassert(CClassRegistry::checkObject(ptr));
				#endif

				// Insert the node.
				_IdMap.insert( ValueIdMap(node, ptr) );

				// Read the object!
				ptr->serial(*this);
			}
			else
			{
				ptr= static_cast<IStreamable*>(it->second);

				// Close the node header
				xmlPushEnd ();
			}
		}
	}
	else
	{
		if(ptr==NULL)
		{
			node= 0;

			// First attribute name
			xmlSetAttrib ("id");

			serial(node);

			// Close the node header
			xmlPushEnd ();
		}
		else
		{
			// Assume that prt size is an int size
			//#ifdef NL_DEBUG
			//	nlassert(sizeof(uint) == sizeof(void *));
			//#endif

			ItIdMap	it;
			it = _IdMap.find((uint64)/*(uint)*/ptr);

			// Test if object has been already written
			if( it==_IdMap.end() )
			{
				// Not yet written

				// Get the next available ID
				node = _NextSerialPtrId++;

				// Serial the id
				xmlSetAttrib ("id");
				serial(node);

				// Insert the pointer in the map with the id
				_IdMap.insert( ValueIdMap((uint64)/*(uint)*/ptr, (void*)/*(uint)*/node) );

				#ifdef NL_DEBUG
					nlassert(CClassRegistry::checkObject(ptr));
				#endif

				// Write the class name.
				string	className=ptr->getClassName();

				// Second attribute name
				xmlSetAttrib ("class");
				serial(className);

				// Close the node header
				xmlPushEnd ();

				// Write the object!
				ptr->serial(*this);
			}
			else
			{
				// Write only the object id
				xmlSetAttrib ("id");
				node = (uint64)/*(uint)*/(it->second);
				serial(node);
				xmlPushEnd ();
			}
		}
	}

	// Close the node
	xmlPop ();
}
// ======================================================================================================
void			IStream::resetPtrTable()
{
	_IdMap.clear();
	_NextSerialPtrId = 1;		// Start at 1 because 0 is the NULL pointer
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
uint IStream::serialVersion(uint currentVersion)
{
	uint8	b=0;
	uint32	v=0;
	uint	streamVersion;

	// Open the node
	xmlPush ("VERSION");

	if(isReading())
	{
		serial(b);
		if(b==0xFF)
			serial(v);
		else
			v=b;
		streamVersion=v;

		// Exception test.
		if(_ThrowOnOlder && streamVersion < currentVersion)
			throw EOlderStream(*this);
		if(_ThrowOnNewer && streamVersion > currentVersion)
			throw ENewerStream(*this);
	}
	else
	{
		v= streamVersion=currentVersion;
		if(v>=0xFF)
		{
			b=0xFF;
			serial(b);
			serial(v);
		}
		else
		{
			b= (uint8)v;
			serial(b);
		}
	}

	// Close the node
	xmlPop ();

	return streamVersion;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================

// ======================================================================================================
void			IStream::serialCont(vector<uint8> &cont)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);

		// check stream holds enough bytes (avoid STL to crash on resize)
		checkStreamSize(len);

		// one block serial
		cont.resize(len);
		if (len != 0)
			serialBuffer( (uint8*)&(*cont.begin()) , len);
	}
	else
	{
		len= (sint32)cont.size();
		serial(len);
		if (len != 0)
			serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
}
// ======================================================================================================
void			IStream::serialCont(vector<sint8> &cont)
{
	sint32	len=0;
	if(isReading())
	{
		serial(len);

		// check stream holds enough bytes (avoid STL to crash on resize)
		checkStreamSize(len);

		// one block serial
		cont.resize(len);
		if (len != 0)
			serialBuffer( (uint8*)&(*cont.begin()) , len);
	}
	else
	{
		len= (sint32)cont.size();
		serial(len);
		if (len != 0)
			serialBuffer( (uint8*)&(*cont.begin()) ,  len);
	}
}
// ======================================================================================================
void			IStream::serialCont(vector<bool> &cont)
{
	sint32	len=0;
	vector<uint8>	vec;

	if(isReading())
	{
		serial(len);

		// check stream holds enough bytes (avoid STL to crash on resize)
		checkStreamSize(len/8);

		// One Block Serial
		cont.resize(len);

		if (len != 0)
		{
			// read as uint8*.
			sint	lb= (len+7)/8;
			vec.resize(lb);
			serialBuffer( (uint8*)&(*vec.begin()) ,  lb);
			for(sint i=0;i<len;i++)
			{
				uint	bit= (vec[i>>3]>>(i&7)) & 1;
				cont[i]= bit?true:false;
			}
		}
	}
	else
	{
		len= (sint32)cont.size();
		serial(len);

		if (len != 0)
		{
			// write as uint8*.
			sint	lb= (len+7)/8;
			vec.resize(lb);
			fill_n(vec.begin(), lb, 0);
			for(sint i=0;i<len;i++)
			{
				uint	bit= cont[i]?1:0;
				vec[i>>3]|= bit<<(i&7);
			}
			serialBuffer( (uint8*)&(*vec.begin()) ,  lb);
		}
	}

}
// ======================================================================================================
bool			IStream::seek (sint32 offset, TSeekOrigin origin) const
{
	throw ESeekNotSupported(*this);
}
// ======================================================================================================
sint32			IStream::getPos () const
{
	throw ESeekNotSupported(*this);
}

// ======================================================================================================
void			IStream::setInOut(bool inputStream)
{
	_InputStream= inputStream;
}


// ======================================================================================================
string			IStream::getStreamName() const
{
	return "";
}


// ======================================================================================================
void			IStream::setXMLMode (bool on)
{
	_XML = on;
}


/*
 * Serial memstream, bitmemstream...
 */
void	IStream::serialMemStream( CMemStream &b )
{
	uint32 len=0;

	// Serialize length
	if ( isReading() )
	{
		// fill b with data from this
		serial (len);
		serialBuffer (b.bufferToFill (len), len);
		b.resetBufPos ();
	}
	else
	{
		// fill this with data from b
		len = b.length();

		serial( len );
		serialBuffer( (uint8*) b.buffer (), len );
	}
}


}

