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

#ifndef NL_STREAM_H
#define NL_STREAM_H

#include	"types_nl.h"
#include	"ucstring.h"
#include	"class_registry.h"
#include	"common.h"

#include	<utility>
#include	<string>
#include	<vector>
#include	<deque>
#include	<list>
#include	<set>
#include	<map>

namespace	NLMISC
{


class	IStream;
class	CMemStream;


// ======================================================================================================
// ======================================================================================================
// Stream System.
// ======================================================================================================
// ======================================================================================================

// For Big/little Endian.
#  define NLMISC_BSWAP16(src)	(src) = (((src)>>8)&0xFF) | (((src)&0xFF)<<8)
#  if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
#    define NLMISC_BSWAP32(src) _asm mov eax,(src) _asm bswap eax _asm mov (src),eax
#  else
#    define NLMISC_BSWAP32(src) (src) = (((src)>>24)&0xFF) | ((((src)>>16)&0xFF)<<8) | ((((src)>>8)&0xFF)<<16) | (((src)&0xFF)<<24)
#  endif
#  define NLMISC_BSWAP64(src) (src) = (((src)>>56)&0xFF) | ((((src)>>48)&0xFF)<<8) | ((((src)>>40)&0xFF)<<16) | ((((src)>>32)&0xFF)<<24) | ((((src)>>24)&0xFF)<<32) | ((((src)>>16)&0xFF)<<40) | ((((src)>>8)&0xFF)<<48) | (((src)&0xFF)<<56)

// convert a 4 characters string to uint32
#ifdef NL_LITTLE_ENDIAN
#	define NELID(x) (uint32((x[0] << 24) | (x[1] << 16) | (x[2] << 8) | (x[3])))
#else
#	define NELID(x) (uint32((x[3] << 24) | (x[2] << 16) | (x[1] << 8) | (x[0])))
#endif

// ======================================================================================================
/**
 * Stream Exception.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
struct EStream : public Exception
{
	EStream() : Exception( "Stream Error" ) {}

	EStream( const std::string& str ) : Exception( str ) {}

	EStream( const IStream &f );

	EStream( const IStream &f, const std::string& str );

	virtual ~EStream() throw() {}

	// May Not be Filled...
	std::string	StreamName;
};

struct EOlderStream : public EStream
{
	EOlderStream() : EStream("The version in stream is older than the class" ) {}
	EOlderStream(const IStream &f) : EStream(f, "The version in stream is older than the class" ) {}
};

struct ENewerStream : public EStream
{
	ENewerStream() : EStream("The version in stream is newer than the class" ) {}
	ENewerStream(const IStream &f) : EStream(f, "The version in stream is newer than the class" ) {}
};

struct EInvalidDataStream : public EStream
{
	EInvalidDataStream() : EStream("Invalid data format" ) {}
	EInvalidDataStream(const IStream &f) : EStream(f, "Invalid data format" ) {}

	// msg must contain "%u" for the position of 'size'
	EInvalidDataStream(const char *msg, uint size);
};

struct ESeekNotSupported : public EStream
{
	ESeekNotSupported() : EStream("Seek fonctionnality is not supported" ) {}
	ESeekNotSupported(const IStream &f) : EStream(f, "Seek fonctionnality is not supported" ) {}
};

struct ENotOutputStream : public EStream
{
	ENotOutputStream() : EStream("The stream is NOT an output stream, can't write in" ) {}
	ENotOutputStream(const IStream &f) : EStream(f, "The stream is NOT an output stream, can't write in" ) {}
};
struct ENotInputStream : public EStream
{
	ENotInputStream () : EStream("The stream is NOT an input stream, cannot read in" ) {}
	ENotInputStream (const IStream &f) : EStream(f, "The stream is NOT an input stream, cannot read in" ) {}
};

/// This exception is raised when someone tries to serialize in more than there is.
struct EStreamOverflow : public EStream
{
	EStreamOverflow() : EStream( "Stream Overflow Error" ) {}

	// msg must contain "%u" for the position of 'size'
	EStreamOverflow(const char *msg, uint size);
};


class	IStreamable;


// ======================================================================================================
/**
 * A IO stream interface.
 * This is the base interface for stream objects. Differents kind of streams may be implemented,
 * by specifying serialBuffer() methods.
 *
 * \b Deriver \b Use:
 *
 * The deriver must:
 * - construct object specifying his type, see IStream(). A stream may be setup Input or Output at construction, but cannot
 * change during his life.
 * - specify serialBuffer(), to save or load pack of bytes.
 * - specify serialBit(), to save or load a bit.
 * - call resetPtrTable() when the stream reset itself (e.g.: CIFile::close() )
 *
 * Sample of streams: COutMemoryStream, CInFileStream ...
 *
 * \b Client \b Use:
 *
 * An object which can be serialized, must provide a "void serial(IStream &)" method. In this method, he can use
 * any of the IStream method to help himself like:
 * - serial() with a base type (uint32, string, char...), or even with an object which provide "void serial(IStream &)"
 * - template serial(T0&, T1&, ...) to serialize multiple object/variables in one call (up to 6).
 * - serialCont() to serialize containers.
 * - serialVersion() to check/store a version number of his class.
 * - serialPtr() to use the ptr support of IStream (see serialPtr() for more information)
 * - isReading() to know if he write in the stream, or if he read.
 *
 * The using is very simple as shown in this example:
 *
 * \code
 class A
 {
 public:
	float	x;
	uint32	y;
	Class1	a;		// this class must provide a serial() method too...
	Base	*c,*d;	// Base must derive from IStreamable
	vector<Class2>	tab;

 public:
	void	serial(IStream &f)
	{
		sint	streamver= f.serialVersion(3);
		f.serial(x,y,a);
		f.serialPtr(c);
		f.serialCont(tab);
		if(streamver>=2)
			f.serialPtr(d);
	}
 };
 \endcode
 *
 * NB: \b YOU \b CANNOT use serial with a int / uint / sint type, since those type have unspecified length.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IStream
{
public:
	/**
	 * Set the behavior of IStream regarding input stream that are older/newer than the class.
	 * If throwOnOlder==true, IStream throws a EOlderStream when needed.
	 * If throwOnNewer==true, IStream throws a ENewerStream when needed.
	 *
	 * By default, the behavior is throwOnOlder=false, throwOnNewer=true.
	 * \see serialVersion() getVersionException()
	 */
	static	void	setVersionException(bool throwOnOlder, bool throwOnNewer);
	/**
	 * Get the behavior of IStream regarding input stream that are older/newer than the class.
	 * \see serialVersion() setVersionException()
	 */
	static	void	getVersionException(bool &throwOnOlder, bool &throwOnNewer);


public:

	/**
	 * Constructor.
	 * Notice that those behavior can be set at construction only.
	 * \param inputStream is the stream an Input (read) stream?
	 */
	explicit IStream(bool inputStream);

	/// Destructor.
	virtual ~IStream() {}

	/// Copy constructor
	IStream( const IStream& other );

	/// Assignment operator
	IStream&		operator=( const IStream& other );

	/// exchange
	void			swap(IStream &other);

	/// Is this stream a Read/Input stream?
	bool			isReading() const;

	// is it a xml stream ?
	bool			isXML() const { return _XML; }

	/**
	 * Template Object serialisation.
	 * \param obj any object providing a "void serial(IStream&)" method. The object doesn't have to derive from IStreamable.
	 *
	 * the VC++ error "error C2228: left of '.serial' must have class/struct/union type" means you don't provide
	 * a serial() method to your object. Or you may have use serial with a int / uint / sint type. REMEMBER YOU CANNOT
	 * do this, since those type have unspecified length.
	 */
    template<class T>
	void			serial(T &obj)  { obj.serial(*this); }

	// an utility template to unconst a type
	template <class T> static T& unconst(const T &t) { return const_cast<T&>(t);}

#define nlWriteSerial(_stream, _obj) 		\
	if ((_stream).isReading())			\
		throw NLMISC::ENotOutputStream();	\
	(_stream).serial(NLMISC::IStream::unconst(_obj));

#define nlWrite(_stream, _serialType, _obj)	\
	if ((_stream).isReading())			\
		throw NLMISC::ENotOutputStream();	\
	(_stream)._serialType(NLMISC::IStream::unconst(_obj));

#define nlReadSerial(_stream, _obj) 		\
	if (!(_stream).isReading())			\
		throw NLMISC::ENotInputStream();	\
	NLMISC::IStream::unconst(_stream).serial(_obj);

#define nlRead(_stream, _serialType, _obj)	\
	if (!(_stream).isReading())			\
		throw NLMISC::ENotInputStream();	\
	NLMISC::IStream::unconst(_stream)._serialType(_obj);

// helper macro to serialize boolean encoded as 'bool foo : 1' (they can't be referenced)
#define nlSerialBitBool(_stream, _boolean) \
	{ \
		bool tmpBool = _boolean; \
		_stream.serial(tmpBool); \
		_boolean = tmpBool; \
	}

	/** \name Base type serialization.
	 * Those method are a specialization of template method "void serial(T&)".
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


	/// Template enum serialisation. Serialized as a sint32.
    template<class T>
	void			serialEnum(T &em)
	{
		sint32	i;
		if(isReading())
		{
			serial(i);
			em = (T)i;
		}
		else
		{
			i = em;
			serial(i);
		}
	}
	/// Template short enum serialisation. Serialized as a uint8 (with checking).
    template<class T>
	void			serialShortEnum(T &em)
	{
		uint8	i;
		if(isReading())
		{
			serial(i);
			em = (T)i;
		}
		else
		{
			nlassert(em < 0xff);
			i = uint8(em);
			serial(i);
		}
	}

	/// Serial memstream, bitmemstream...
	virtual void serialMemStream( CMemStream &b );

	/** \name BitField serialisation.
	 * Unlike other serial method, The reading bitfield is returned!! If !this->isReading(), bf is returned.
	 *
	 * MUST use it simply like this:   a= serialBitFieldX(a);		// where X== 8, 16 or 32.
	 *
	 * NB: Performance warning: the data is stored as an uint8, uint16 or uint32, according to the method you use.
	 */
	//@{
	/// Serialisation of bitfield <=8 bits.
	uint8			serialBitField8(uint8  bf);
	/// Serialisation of bitfield <=16 bits.
	uint16			serialBitField16(uint16  bf);
	/// Serialisation of bitfield <=32 bits.
	uint32			serialBitField32(uint32  bf);
	//@}


	/** \name Multiple serialisation.
	 * Template for easy multiple serialisation.
	 */
	//@{
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
	//@}


	/** \name standard STL containers serialisation.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>, map<>, multimap<>
	 * Support up to sint32 length containers.
	 * \see serialContPtr() serialContPolyPtr()
	 */
	template<class T, class Allocator>
	void			serialCont(std::vector<T, Allocator> &cont)	{serialVector(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 	{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 	{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 		{serialSTLCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 	{serialSTLCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 			{serialMap(cont);}
	template<class K, class T, class H>
	void			serialCont(CHashMap<K, T, H> &cont) 			{serialMap(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{serialMultimap(cont);}

	/** \name standard STL containers serialisation.
	 * Thse variants suppose contained type is a NeL smart pointer.
	 * Known Supported containers: map<>
	 * Support up to sint32 length containers.
	 * \see serialCont() serialContPtr() serialContPolyPtr()
	 */
	template<class K, class T>
	void	serialPtrCont(std::map<K, T> &cont)	{serialPtrMap(cont);}


	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) ;
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) ;
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont) ;


	/** \name standard STL containers serialisation. Elements must be pointers on a base type (uint...) or on a
	 * object providing "void serial(IStream&)" method.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>
	 * Support up to sint32 length containers.
	 * \see serialCont() serialContPolyPtr()
	 */
	template<class T, class Allocator>
	void			serialContPtr(std::vector<T, Allocator> &cont) 	{serialVectorPtr(cont);}
	template<class T>
	void			serialContPtr(std::list<T> &cont) 	{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::deque<T> &cont) 	{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::set<T> &cont) 			{serialSTLContPtr(cont);}
	template<class T>
	void			serialContPtr(std::multiset<T> &cont) 	{serialSTLContPtr(cont);}


	/** \name standard STL containers serialisation. Elements must be pointers on a IStreamable object.
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>
	 * Support up to sint32 length containers.
	 * \see serialCont() serialContPtr()
	 */
	template<class T, class Allocator>
	void			serialContPolyPtr(std::vector<T, Allocator> &cont) 	{serialVectorPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::list<T> &cont) 	{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::deque<T> &cont) 	{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::set<T> &cont) 			{serialSTLContPolyPtr(cont);}
	template<class T>
	void			serialContPolyPtr(std::multiset<T> &cont) 	{serialSTLContPolyPtr(cont);}
	template<class K, class T>
	void			serialContPolyPtr(std::map<K, T> &cont) 	{serialMapPolyPtr(cont);}


	/**
	 * Serialize Non Polymorphic Objet Ptr.
	 * Works with NULL pointers. If the same object is found mutliple time in the stream, ONLY ONE instance is written!
	 * NB: The ptr is serialised as a uint64 (64 bit compliant).
	 * \param ptr a pointer on a base type or an object.
	 * \see resetPtrTable()
	 */
	template<class T>
	void			serialPtr(T* &ptr)
	{
		uint64	node;

		// Open the node header
		xmlPushBegin ("PTR");

		xmlSetAttrib ("id");

		if(isReading())
		{
			serial(node);

			// Close the header
			xmlPushEnd ();

			if(node==0)
				ptr=NULL;
			else
			{
				ItIdMap	it;
				it= _IdMap.find(node);

				// Test if object already created/read.
				if( it==_IdMap.end() )
				{
					// Construct object.
					ptr= new T;
					if(ptr==NULL)
						throw EStream();

					// Insert the node.
					_IdMap.insert( ValueIdMap(node, ptr) );

					// Read the object!
					serial(*ptr);
				}
				else
					ptr= static_cast<T*>(it->second);
			}
		}
		else
		{
			if(ptr==NULL)
			{
				node= 0;
				serial(node);

				// Close the header
				xmlPushEnd ();
			}
			else
			{
				ItIdMap	it;
				it = _IdMap.find((uint64)ptr);

				// Test if object has been already written
				if( it==_IdMap.end() )
				{
					// Not yet written

					// Get the next available ID
					node = _NextSerialPtrId++;

					// Serial the id
					serial(node);

					// Insert the pointer in the map with the id
					_IdMap.insert( ValueIdMap((uint64)ptr, (void*)node) );

					// Close the header
					xmlPushEnd ();

					// Write the object
					serial(*ptr);
				}
				else
				{
					// Write only the object id
					node = (uint64)(it->second);

					serial(node);

					// Close the header
					xmlPushEnd ();
				}
			}
		}

		// Close the node
		xmlPop ();
	}


	/**
	 * Serialize Polymorphic Objet Ptr.
	 * Works with NULL pointers. If the same object is found mutliple time in the stream, ONLY ONE instance is written!
	 * NB: The ptr is serialised as a uint64 (64 bit compliant).
	 * \param ptr a pointer on a IStreamable object.
	 * \see resetPtrTable()
	 */
	template<class T>
	void			serialPolyPtr(T* &ptr)
	{ IStreamable *p=ptr; serialIStreamable(p); ptr= static_cast<T*>(p);}


	/**
	 * Serialize a version number.
	 * Each object should store/read first a version number, using this method.
	 * Then he can use the streamVersion returned to see how he should serialise himself.
	 *
	 * NB: Version Number is read/store as a uint8, or uint32 if too bigger..
	 * \param currentVersion the current version of the class, provided by user.
	 * \return the version of the stream. If the stream is an Output stream, currentVersion is returned.
	 * \see setVersionException() getVersionException()
	 */
	uint			serialVersion(uint currentVersion) ;


	/**
	 * Serialize a check value.
	 * An object can stream a check value to check integrity or format of filed or streamed data.
	 * Just call serial check with a const value. Write will serial the value. Read will
	 * check the value is the same. If it is not, it will throw EInvalidDataStream exception.
	 *
	 * NB: The type of the value must implement an operator == and must be serializable.
	 * \param value the value used to the check.
	 * \see EInvalidDataStream
	 */
	template<class T>
	void			serialCheck(const T& value)
	{
		// Open a node
		xmlPush ("CHECK");

		if (isReading())
		{
			T read;
			serial (read);
			if (read!=value)
				throw EInvalidDataStream(*this);
		}
		else
		{
			serial (const_cast<T&>(value));
		}

		// Close the node
		xmlPop ();
	}

	/// \name Seek fonctionnality

	/**
	 * Parameters for seek().
	 * begin seek from the beginning of the stream.
	 * current seek from the current location of the stream pointer.
	 * end seek from the end of the stream.
	 */
	enum TSeekOrigin { begin, current, end };

	/**
	 * Moves the stream pointer to a specified location.
	 *
	 * NB: If the stream doesn't support the seek fonctionnality, it throw ESeekNotSupported.
	 * Default implementation:
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool		seek (sint32 offset, TSeekOrigin origin) const;


	/**
	 * Get the location of the stream pointer.
	 *
	 * NB: If the stream doesn't support the seek fonctionnality, it throw ESeekNotSupported.
	 * Default implementation:
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32		getPos () const;


	/** Get a name for this stream. maybe a fileName if FileStream.
	 *	Default is to return "".
	 */
	virtual std::string		getStreamName() const;

	/** \name XML user interface
	  *
	  * Those functions are used to add information in your stream to structure it like
	  * a XML document. Exemple of a serial sequence :
	  \code
		// Start the opening of a new node named Identity
		stream.xmlPush ("Identity")

			// Serial some infos
			stream.serial (name);
			stream.serial (pseudo);

			// Open a new node header named Address
			stream.xmlPushBegin ("Address");

					// Set a property name
					stream.xmlSetAttrib ("Street")

					// Serial the property
					stream.serial ("Street");

				// Close the new node header
				stream.xmlPushEnd ();

				// Serial in this node
				stream.serial (cityName);

			// Close the address node
			stream.xmlPop ();

			// Add a comment
			stream.xmlComment ("Hello");

		// Close the identity node
		stream.xmlPop ();
      \endcode
	  *
	  * The result will be an xml document structured like this:
	  *
	  \code
		<Identity>
			Corvazier Hulud
			<Address Street="rue du Faubourg Saint Antoine">
				Paris
			<\Address>
			<!-- Hello -->
		<\Identity>
	  \endcode
	  *
	  * Node header serials are the serialisations done between xmlPushBegin() and xmlPushEnd() call. There is some restrictions on them:
	  * Node header serials are only available for basic types (numbers and strings).
	  * xmlSetAttrib() must be called before node header serial.
	  *
	  * Note that XML documents only have ONE root node, so all serialisation must be done between a xmlPush() and a xmlPop() call.
	  *
	  * When a xml input stream will try to open a node, it will scan all currrent child nodes to find the good one.
	  * So input xml stream can skip unknown node.
	  */

	/**
	  * xmlSerial() serial a values into a node.
	  */
	template<class T>
	void xmlSerial (T& value0, const char *nodeName)
	{
		// Open the node
		xmlPush (nodeName);

		// Serial the value
		serial (value0);

		// Close the node
		xmlPop ();
	}
	template<class T>
	void xmlSerial (T& value0, T& value1, const char *nodeName)
	{
		// Open the node
		xmlPush (nodeName);

		// Serial the values
		serial (value0, value1);

		// Close the node
		xmlPop ();
	}
	template<class T>
	void xmlSerial (T& value0, T& value1, T& value2, const char *nodeName)
	{
		// Open the node
		xmlPush (nodeName);

		// Serial the values
		serial (value0, value1, value2);

		// Close the node
		xmlPop ();
	}
	template<class T>
	void xmlSerial (T& value0, T& value1, T& value2, T& value3, const char *nodeName)
	{
		// Open the node
		xmlPush (nodeName);

		// Serial the values
		serial (value0, value1, value2, value3);

		// Close the node
		xmlPop ();
	}

	/**
	  * xmlPush() open recurcively a new node. You must call xmlPop to close this node.
	  *
	  * \name is the name of the node to open
	  * \return true if you can open the node, false if the stream is between a xmlPushBegin() and a xmlPushEnd() call.
	  */
	bool xmlPush (const char *name)
	{
		// XML Mode ?
		if (_XML)
		{
			// Open the header
			bool res=xmlPushBeginInternal (name);
			if (res)
				// close the header
				xmlPushEndInternal ();
			// Return the result
			return res;
		}

		// Return ok
		return true;
	}

	/**
	  * xmlPushBegin() open recurcively a new node and open its header. You must call xmlPushEnd() to close the header and xmlPop() to close this node.
	  *
	  * \name is the name of the node to open
	  * \return true if you can open the node header, false if the stream is between a xmlPushBegin() and a xmlPushEnd() call.
	  */
	bool xmlPushBegin (const char *name)
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlPushBeginInternal (name);
		}

		// Return ok
		return true;
	}

	/**
	  * xmlPushEnd() close the node header.
	  *
	  * \return true if you can close the node header, false if no node header have been opened with xmlPushBegin().
	  */
	bool xmlPushEnd ()
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlPushEndInternal ();
		}

		// Return ok
		return true;
	}

	/**
	  * xmlPop() close the node.
	  *
	  * \return true if you can close the node, false if the node can't be closed (its header is still opened) or if there is no node to close.
	  */
	bool xmlPop ()
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlPopInternal ();
		}

		// Return ok
		return true;
	}

	/**
	  * xmlSetAttrib() set the name of the next node header attribute serialised.
	  *
	  * \param name is the name of the node header attribute serialised.
	  * \return true if the attribute name have been set, false if the node header is not open (the call is not between xmlPushBegin and xmlPushEnd)
	  */
	bool xmlSetAttrib (const char *name)
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlSetAttribInternal (name);
		}

		// Return ok
		return true;
	}

	/**
	  * xmlBreakLine() insert a break line in the XML stream.
	  *
	  * \return true if the break line is added, return false if no node is opened.
	  */
	bool xmlBreakLine ()
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlBreakLineInternal ();
		}

		// Return ok
		return true;
	}

	/**
	  * xmlComment() insert a comment line in the XML stream.
	  *
	  * \return true if the comment is added, return false if no node is opened.
	  */
	bool xmlComment (const char *comment)
	{
		// XML Mode ?
		if (_XML)
		{
			return xmlCommentInternal (comment);
		}

		// Return ok
		return true;
	}

protected:

	/// \name XML implementation interface

	/** Set the XML mode
	  * \param on is true to enable XML mode else false
	  */
	void setXMLMode (bool on);

	/// xmlPushBegin implementation
	virtual bool		xmlPushBeginInternal (const char * /* name */) { return true; };

	/// xmlPushEnd implementation
	virtual bool		xmlPushEndInternal () { return true; };

	/// xmlPop implementation
	virtual bool		xmlPopInternal () { return true; };

	/// xmlBreakLine implementation
	virtual bool		xmlSetAttribInternal (const char * /* name */) { return true; };

	/// xmlBreakLine implementation
	virtual bool		xmlBreakLineInternal () { return true; };

	/// xmlComment implementation
	virtual	bool		xmlCommentInternal (const char * /* comment */) { return true; };

	/**
	 * for Deriver: reset the PtrTable in the stream.
	 * If Derived stream provide reset()-like methods, they must call this method in their reset() methods.
	 * For example, CFile::close() must call it, so it will work correctly with next serialPtr()
	 */
	void				resetPtrTable();

	/**
	 * Change, in live, the state of the inputStream. This could be useful in certain case.
	 * The deriver which would want to do such a thing must call this method, and implement his own behavior.
	 * In certain case, it should call resetPtrTable() if he want to reset the stream ptr info (maybe always)...
	 */
	void				setInOut(bool inputStream);

	/** Get the size for this stream. return 0 by default. Only implemented for input stream that know their size.
	 *	Used internally to detect OverFlow with vector<> for instance
	 */
	virtual uint			getDbgStreamSize() const {return 0;}

	/**
	 * Elementarily check at least n bytes can be serialized from this stream (or throw EStreamOverflow)
	 */
	void				checkStreamSize(uint numBytes) const
	{
		uint			ssize = getDbgStreamSize();
		if (ssize > 0 && ssize < numBytes)
			throw EStreamOverflow("stream does not contain at least %u bytes for check", numBytes);
	}

public:
	//@{
	/** Method to be specified by the Deriver.
	 * \warning Do not call these methods from outside, unless you really know what you are doing.
	 * Using them instead of serial() can lead to communication problems between different platforms !
	 */
	virtual void		serialBuffer(uint8 *buf, uint len) =0;
	virtual void		serialBit(bool &bit) =0;
	//@}

	/// This method first serializes the size of the buffer and after the buffer itself, it enables
	/// the possibility to serial with a serialCont() on the other side.
	virtual void		serialBufferWithSize(uint8 *buf, uint32 len)
	{
		serial (len);
		serialBuffer (buf, len);
	}

private:
	bool	_InputStream;
	static	bool	_ThrowOnOlder;
	static	bool	_ThrowOnNewer;

	// Ptr registry. We store 64 bit Id, to be compatible with future 64+ bits pointers.
	uint32								_NextSerialPtrId;
	CHashMap<uint64, void*>		_IdMap;
	typedef CHashMap<uint64, void*>::iterator	ItIdMap;
	typedef CHashMap<uint64, void*>::value_type	ValueIdMap;

	// Ptr serialization.
	void			serialIStreamable(IStreamable* &ptr) ;



private:
	/**
	 * standard STL containers serialization. Don't work with map<> and multimap<>.
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLen(T &cont, sint32 len)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			for(sint i=0;i<len;i++)
			{
				xmlPush ("ELM");

				__value_type v;
				serial(v);
				(void)cont.insert(cont.end(), v);

				xmlPop ();
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("ELM");

				serial(const_cast<__value_type&>(*it));

				xmlPop ();
			}
		}
	}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.
	 * Support up to sint32 length containers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(a base type (uint...), or an object providing "void serial(IStream&)" method.)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: vector<>, list<>, deque<>, set<>, multiset<>.
	 * \param cont a STL container (vector<>, set<> ...).
	 */
	template<class T>
	void			serialSTLCont(T &cont)
	{
		// Open a node header
		xmlPushBegin ("CONTAINER");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
		}

		// Close the header
		xmlPushEnd ();

		serialSTLContLen(cont, len);

		// Close the node
		xmlPop ();
	}


protected:

	/**
	 * special version for serializing a vector.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVector(T &cont)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("VECTOR");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);

			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			// Open a node header
			xmlPushEnd ();

			// special version for vector: adjut good size.
			contReset(cont);
			cont.resize (len);

			// Read the vector
			for(sint i=0;i<len;i++)
			{
				xmlPush ("ELM");

				serial(cont[i]);

				xmlPop ();
			}
		}
		else
		{
			len = (sint32)cont.size();
			serial(len);

			// Close the node header
			xmlPushEnd ();

			// Write the vector
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("ELM");

				serial(const_cast<__value_type&>(*it));

				xmlPop ();
			}
		}

		// Close the node
		xmlPop ();
	}


private:
	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.  Ptr version.
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLenPtr(T &cont, sint32 len)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			for(sint i=0;i<len;i++)
			{
				__value_type	v;
				serialPtr(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serialPtr(const_cast<__value_type&>(*it));
			}
		}
	}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>.  Ptr version.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialSTLContPtr(T &cont)
	{
		// Open a node header
		xmlPushBegin ("CONTAINER");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		// Close the node header
		xmlPushEnd ();

		serialSTLContLenPtr(cont, len);

		// Close the node
		xmlPop ();
	}


	/**
	 * special version for serializing a vector.  Ptr version.
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVectorPtr(T &cont)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("VECTOR");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			// special version for vector: adjut good size.
			contReset(cont);

			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			cont.reserve(len);
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
		}

		// Close the node header
		xmlPushEnd ();

		serialSTLContLenPtr(cont, len);

		// Close the node
		xmlPop ();
	}


private:
	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>. PolyPtr version
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialSTLContLenPolyPtr(T &cont, sint32 len)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			for(sint i=0;i<len;i++)
			{
				__value_type	v=NULL;
				serialPolyPtr(v);
				cont.insert(cont.end(), v);
			}
		}
		else
		{
			__iterator		it= cont.begin();
			for(sint i=0;i<len;i++, it++)
			{
				serialPolyPtr(const_cast<__value_type&>(*it));
			}
		}
	}

	/**
	 * Map serialisation. PolyPtr version
	 * Support up to sint32 length containers. serialize just len  element of the container.
	 */
	template<class T>
	void			serialMapContLenPolyPtr(T &cont, sint32 len)
	{
		typedef typename T::key_type __key_type;
		typedef typename T::data_type __data_type;
		typedef typename T::iterator __iterator;

		if(isReading())
		{
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);
			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++)
			{
				__key_type k;

				xmlPush ("KEY");
				serial ( k );
				xmlPop ();

				xmlPush ("ELM");
				__data_type	v=NULL;
				v.serialPolyPtr(*this);
				cont[k] = v;
				xmlPop ();
			}
		}
		else
		{
			__iterator		it= cont.begin();

			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("KEY");
				serial( const_cast<__key_type&>((*it).first) );
				xmlPop ();

				xmlPush ("ELM");
				__data_type	v= const_cast<__data_type&>(it->second);
				v.serialPolyPtr(*this);
				xmlPop ();
			}
		}
	}


	/**
	 * standard STL containers serialisation. Don't work with map<> and multimap<>. PolyPtr version
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialSTLContPolyPtr(T &cont)
	{
		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialSTLContLenPolyPtr(cont, len);
	}


	/**
	 * special version for serializing a vector. PolyPtr version
	 * Support up to sint32 length containers.
	 */
	template<class T>
	void			serialVectorPolyPtr(T &cont)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("VECTOR");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			// special version for vector: adjut good size.
			contReset(cont);

			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			cont.reserve(len);
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
		}

		// Close the node header
		xmlPushEnd ();

		serialSTLContLenPolyPtr(cont, len);

		// Close the node
		xmlPop ();
	}

	/**
	 * special version for serializing a map. PolyPtr version
	 * Support up to sint32 length containers.
	 */
	template<class K, class T>
	void			serialMapPolyPtr(std::map<K, T> &cont)
	{
		// Open a node header
		xmlPushBegin ("MAP");
		// Attrib size
		xmlSetAttrib ("size");

		sint32	len=0;
		if(isReading())
		{
			serial(len);
			cont.clear();
		}
		else
		{
			len= cont.size();
			serial(len);
		}

		serialMapContLenPolyPtr(cont, len);

		// Close the node
		xmlPop ();
	}



private:

	/**
	 * STL map<> and multimap<> serialisation.
	 * Support up to sint32 length containers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(must be a std::pair<>)
	 *	\li typedef key_type;		(must be the type of the key)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: map<>, multimap<>.
	 * \param cont a STL map<> or multimap<> container.
	 */
	template<class T>
	void			serialMultimap(T &cont)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::key_type __key_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("MULTIMAP");

		// Attrib size
		xmlSetAttrib ("size");

		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);

			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);

			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++)
			{
				__value_type v;

				xmlPush ("KEY");

				serial ( const_cast<__key_type&>(v.first) );

				xmlPop ();


				xmlPush ("ELM");

				serial (v.second);

				xmlPop ();

				cont.insert(cont.end(), v);
			}
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
			__iterator		it= cont.begin();

			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("KEY");

				serial( const_cast<__key_type&>((*it).first) );

				xmlPop ();

				xmlPush ("ELM");

				serial((*it).second);

				xmlPop ();
			}
		}

		// Close the node
		xmlPop ();
	}


	/**
	 * STL map<>
	 * Support up to sint32 length containers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(must be a std::pair<>)
	 *	\li typedef key_type;		(must be the type of the key)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: map<>
	 * \param cont a STL map<> container.
	 */
	template<class T>
	void			serialMap(T &cont)
	{
		typedef typename T::value_type __value_type;
		typedef typename T::key_type __key_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("MAP");
		// Attrib size
		xmlSetAttrib ("size");

		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);
			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++)
			{
				// MALKAV 05/07/02 : prevent a copy of the value, copy the key instead
				__key_type k;

				xmlPush ("KEY");
				serial ( k );
				xmlPop ();

				xmlPush ("ELM");
				serial (cont[k]);
				xmlPop ();
			}
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
			__iterator		it= cont.begin();

			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("KEY");
				serial( const_cast<__key_type&>((*it).first) );
				xmlPop ();

				xmlPush ("ELM");
				serial((*it).second);
				xmlPop ();
			}
		}

		// Close the node
		xmlPop ();
	}

	/**
	 * STL map<>
	 * Support up to sint32 length containers. Container must contain NeL smart pointers.
	 *
	 * the object T must provide:
	 *	\li typedef iterator;		(providing operator++() and operator*())
	 *	\li typedef value_type;		(must be a std::pair<>)
	 *	\li typedef key_type;		(must be the type of the key)
	 *	\li void clear();
	 *	\li size_type size() const;
	 *	\li iterator begin();
	 *	\li iterator end();
	 *	\li iterator insert(iterator it, const value_type& x);
	 *
	 * Known Supported containers: map<>
	 * \param cont a STL map<> container.
	 */
	template<class T>
	void			serialPtrMap(T &cont)
	{
		typedef typename T::value_type::second_type __ptr_type;
		typedef typename __ptr_type::element_type __value_type;
		typedef typename T::key_type __key_type;
		typedef typename T::iterator __iterator;

		// Open a node header
		xmlPushBegin ("MAP");
		// Attrib size
		xmlSetAttrib ("size");

		sint32	len;
		if(isReading())
		{
			cont.clear();
			serial(len);
			// check stream holds enough bytes (avoid STL to crash on resize)
			checkStreamSize(len);
			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++)
			{
				// MALKAV 05/07/02 : prevent a copy of the value, copy the key instead
				__key_type k;

				xmlPush ("KEY");
				serial ( k );
				xmlPop ();

				xmlPush ("ELM");
				cont[k] = __ptr_type(new __value_type());
				serial (*cont[k]);
				xmlPop ();
			}
		}
		else
		{
			len= (sint32)cont.size();
			serial(len);
			__iterator		it= cont.begin();

			// Close the node header
			xmlPushEnd ();

			for(sint i=0;i<len;i++, it++)
			{
				xmlPush ("KEY");
				serial( const_cast<__key_type&>((*it).first) );
				xmlPop ();

				xmlPush ("ELM");
				serial(*(*it).second);
				xmlPop ();
			}
		}

		// Close the node
		xmlPop ();
	}

	// Mode XML
	bool	_XML;
};


// ======================================================================================================
// ======================================================================================================
// Handle for streaming Polymorphic classes.
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
/**
 * An Object Streamable interface. Any polymorphic class which want to use serial() in a polymorphic way, must derive
 * from this interface.
 * \author Lionel Berenguier
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IStreamable : public IClassable
{
public:
	virtual void		serial(IStream	&f) =0;
};


} // NLMISC.


// Inline Implementation.
#include "stream_inline.h"


#endif // NL_STREAM_H

/* End of stream.h */
