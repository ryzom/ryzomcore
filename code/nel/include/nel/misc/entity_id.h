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

#ifndef NL_ENTITY_ID_H
#define NL_ENTITY_ID_H

#include "types_nl.h"
#include "debug.h"
#include "common.h"
#include "stream.h"

namespace NLMISC {

/**
 * Entity identifier
 * \author Sameh Chafik, Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
struct CEntityId
{
	// pseudo constants
	enum
	{
		DYNAMIC_ID_SIZE = 8,
		CREATOR_ID_SIZE = 8,
		TYPE_SIZE = 8,
		ID_SIZE = 40,
		UNKNOWN_TYPE = (1 << TYPE_SIZE)-1
	};

protected:

	// ---------------------------------------------------------------------------------
	// instantiated data

	union
	{
		struct
		{
		/// Id of the service where the entity is (variable routing info).
		uint64	DynamicId   :  DYNAMIC_ID_SIZE;
		/// Id of the service who created the entity (persistent).
		uint64	CreatorId   :  CREATOR_ID_SIZE;
		/// Type of the entity (persistent).
		uint64	Type :			TYPE_SIZE;
		/// Local entity number (persistent).
		uint64	Id :			ID_SIZE;
		} DetailedId;

		uint64 FullId;
	};

	// ---------------------------------------------------------------------------------
	// static data

	/// Counter for generation of unique entity ids
	static NLMISC::CEntityId	_NextEntityId;

	///The local num service id of the local machin.
	static uint8				_ServerId;

public:

	// ---------------------------------------------------------------------------------
	// static data

	///The maximum number that we could generate without generate an overtaking exception.
	static const uint64			MaxEntityId;

	/// Unknown CEntityId is similar as a NULL pointer.
	static const CEntityId		Unknown;


	// ---------------------------------------------------------------------------------
	// generation of new unique entity ids

	/// Set the service id for the generator
	static void					setServiceId( uint8 sid )
	{
		_NextEntityId.setDynamicId( sid );
		_NextEntityId.setCreatorId( sid );
		_ServerId = sid;
	}

	/// Generator of entity ids
	static CEntityId			getNewEntityId( uint8 type )
	{
		nlassert(_NextEntityId != Unknown ); // type may be Unknown, so isUnknownId() would return true
		NLMISC::CEntityId id = _NextEntityId++;
		id.setType( type );
		return id;
	}

	// ---------------------------------------------------------------------------------
	// constructors

	///\name Constructor
	//@{

	CEntityId ()
	{
		FullId = 0;
		DetailedId.Type = UNKNOWN_TYPE;

		/*
		DynamicId = 0;
		CreatorId = 0;
		Type = 127;
		Id = 0;
		*/
	}

	CEntityId (uint8 type, uint64 id, uint8 creator, uint8 dynamic)
	{
		DetailedId.DynamicId = dynamic;
		DetailedId.CreatorId = creator;
		DetailedId.Type = type;
		DetailedId.Id = id;
	}

	CEntityId (uint8 type, uint64 id)
	{
		DetailedId.Type = type;
		DetailedId.Id = id;
		DetailedId.CreatorId = _ServerId;
		DetailedId.DynamicId = _ServerId;
	}

	explicit CEntityId (uint64 p)
	{
		FullId = p;
		/*
		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = (p);
		*/
	}

	CEntityId (const CEntityId &a)
	{
		FullId = a.FullId;
		/*
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;
		Type = a.Type;
		Id = a.Id;
		*/
	}

	///fill from read stream.
	CEntityId (NLMISC::IStream &is)
	{
		is.serial(FullId);
		/*
		uint64 p;
		is.serial(p);

		DynamicId = (p & 0xff);
		p >>= 8;
		CreatorId = (p & 0xff);
		p >>= 8;
		Type = (p & 0xff);
		p >>= 8;
		Id = p;
		*/
	}

	explicit CEntityId (const std::string &str)
	{
		fromString(str.c_str());
	}

	explicit CEntityId (const char *str)
	{
		CEntityId ();
		fromString(str);
	}
	//@}


	// ---------------------------------------------------------------------------------
	// accessors

	/// Get the full id
	uint64 getRawId() const
	{
		return FullId;
		/*
		return (uint64)*this;
		*/
	}

	/// Get the local entity number
	uint64 getShortId() const
	{
		return DetailedId.Id;
	}

	/// Set the local entity number
	void setShortId( uint64 shortId )
	{
		DetailedId.Id = shortId;
	}

	/// Get the variable routing info
	uint8 getDynamicId() const
	{
		return DetailedId.DynamicId;
	}

	/// Set the variable routing info
	void setDynamicId( uint8 dynId )
	{
		DetailedId.DynamicId = dynId;
	}

	/// Get the persistent creator id
	uint8 getCreatorId() const
	{
		return DetailedId.CreatorId;
	}

	/// Set the persistent creator id
	void setCreatorId( uint8 creatorId )
	{
		DetailedId.CreatorId = creatorId;
	}

	/// Get the entity type
	uint8 getType() const
	{
		return (uint8)DetailedId.Type;
	}

	/// Set the entity type
	void setType( uint8 type )
	{
		DetailedId.Type = type;
	}

	/// Get the persistent part of the entity id (the dynamic part in the returned id is 0)
	uint64 getUniqueId() const
	{
		CEntityId id;
		id.FullId = FullId;
		id.DetailedId.DynamicId = 0;
		return id.FullId;
	}

	/// Test if the entity id is Unknown
	bool isUnknownId() const
	{
		return DetailedId.Type == UNKNOWN_TYPE;
	}


	// ---------------------------------------------------------------------------------
	// operators

	///\name comparison of two CEntityId.
	//@{
	bool operator == (const CEntityId &a) const
//	virtual bool operator == (const CEntityId &a) const
	{

		CEntityId testId ( FullId ^ a.FullId );
		testId.DetailedId.DynamicId = 0;
		return testId.FullId == 0;

		/*
		return (Id == a.DetailedId.Id && DetailedId.CreatorId == a.DetailedId.CreatorId && DetailedId.Type == a.DetailedId.Type);
		*/
	}
	bool operator != (const CEntityId &a) const
	{
		return !((*this) == a);
	}

	bool operator < (const CEntityId &a) const
//	virtual bool operator < (const CEntityId &a) const
	{
		return getUniqueId() < a.getUniqueId();

		/*
		if (Type < a.Type)
		{
			return true;
		}
		else if (Type == a.Type)
		{
			if (Id < a.Id)
			{
				return true;
			}
			else if (Id == a.Id)
			{
				return (CreatorId < a.CreatorId);
			}
		}
		return false;
		*/
	}

	bool operator > (const CEntityId &a) const
//	virtual bool operator > (const CEntityId &a) const
	{
		return getUniqueId() > a.getUniqueId();

		/*
		if (Type > a.Type)
		{
			return true;
		}
		else if (Type == a.Type)
		{
			if (Id > a.Id)
			{
				return true;
			}
			else if (Id == a.Id)
			{
				return (CreatorId > a.CreatorId);
			}
		}
		// lesser
		return false;
		*/
	}
	//@}

	const CEntityId &operator ++(int)
	{
		if(DetailedId.Id < MaxEntityId)
		{
			DetailedId.Id ++;
		}
		else
		{
			nlerror ("CEntityId looped (max was %" NL_I64 "d", MaxEntityId);
		}
		return *this;
	}

	const CEntityId &operator = (const CEntityId &a)
	{
		FullId = a.FullId;
		/*
		DynamicId = a.DynamicId;
		CreatorId = a.CreatorId;
		Type = a.Type;
		Id = a.Id;
		*/
		return *this;
	}

	const CEntityId &operator = (uint64 p)
	{
		FullId = p;
		/*
		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
		*/
		return *this;
	}


	// ---------------------------------------------------------------------------------
	// other methods...
	uint64 asUint64()const
	{
		return FullId;
	}

/*	operator uint64 () const
	{
		return FullId;

		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;

		return p;
	}
*/

	// ---------------------------------------------------------------------------------
	// loading, saving, serialising...

	/// Save the Id into an output stream.
	void save(NLMISC::IStream &os)
//	virtual void save(NLMISC::IStream &os)
	{
		os.serial(FullId);
		/*
		uint64 p = Id;
		p <<= 8;
		p |= (uint64)Type;
		p <<= 8;
		p |= (uint64)CreatorId;
		p <<= 8;
		p |= (uint64)DynamicId;
		os.serial(p);
		*/
	}

	/// Load the number from an input stream.
	void load(NLMISC::IStream &is)
//	virtual void load(NLMISC::IStream &is)
	{
		is.serial(FullId);
		/*
		uint64 p;
		is.serial(p);

		DynamicId = (uint64)(p & 0xff);
		p >>= 8;
		CreatorId = (uint64)(p & 0xff);
		p >>= 8;
		Type = (uint64)(p & 0xff);
		p >>= 8;
		Id = (uint64)(p);
		*/
	}


	void serial (NLMISC::IStream &f)
//	virtual void serial (NLMISC::IStream &f)
	{
		if (f.isReading ())
		{
			load (f);
		}
		else
		{
			save (f);
		}
	}


	// ---------------------------------------------------------------------------------
	// string convertions

	/// return a string in form "(a:b:c:d)" where a,b,c,d are components of entity id.
	std::string toString() const
	{
		std::string ident;
		ident.reserve(25);
		ident+='(';
		getDebugString (ident);
		ident+=')';
		return ident;
	}

	/// Read from a debug string, use the same format as toString() (id:type:creator:dynamic) in hexadecimal
	void	fromString(const char *str)
//	virtual void	fromString(const char *str)
	{
		uint64		id;
		uint		type;
		uint		creatorId;
		uint		dynamicId;

		if (sscanf(str, "(%" NL_I64 "x:%x:%x:%x)", &id, &type, &creatorId, &dynamicId) != 4)
		{
			*this = Unknown;
			return;
		}

		DetailedId.Id = id;
		DetailedId.Type = type;
		DetailedId.CreatorId = creatorId;
		DetailedId.DynamicId = dynamicId;
	}

	/// Have a debug string.
	void getDebugString(std::string &str) const
//	virtual void getDebugString(std::string &str) const
	{
		str.reserve(str.size()+24);
		char b[256];
		memset(b,0,255);
		memset(b,'0',19);
		sint n;

		uint64 x = DetailedId.Id;
		char baseTable[] = "0123456789abcdef";
		for(n = 10; n < 20; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 9] = ':';

		x = DetailedId.Type;
		for(n = 7; n < 9; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 6] = ':';

		x = DetailedId.CreatorId;
		for(n = 4; n < 6; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[19 - 3] = ':';

		x = DetailedId.DynamicId;
		for(n = 1; n < 3; n ++)
		{
			b[19 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		str += "0x";
		str += b;
	}
/*
	/// \name NLMISC::IStreamable method.
	//@{
	std::string	getClassName ()
//	virtual std::string	getClassName ()
	{
		return std::string ("<CEntityId>");
	}

	//@}
*/

//	friend std::stringstream &operator << (std::stringstream &__os, const CEntityId &__t);
};

/**
 * a generic hasher for entities
 */
/*class CEidHash
{
public:
	size_t	operator () ( const NLMISC::CEntityId & id ) const
	{
		uint64 hash64 = id.getUniqueId();
		return size_t(hash64) ^ size_t( hash64 >> 32 );
		return (uint32)id.getShortId();
	}
};*/

// Traits for hash_map using CEntityId
struct CEntityIdHashMapTraits
{
	enum { bucket_size = 4, min_buckets = 8 };
	CEntityIdHashMapTraits() { }
	size_t operator() (const NLMISC::CEntityId &id ) const
	{
		uint64 hash64 = id.getUniqueId();
#ifdef HAVE_X86_64
		return (size_t)hash64;
#else
		return (size_t)hash64 ^ (size_t)(hash64 >> 32);
#endif
		//return size_t(id.getShortId());
	}
	bool operator() (const NLMISC::CEntityId &id1, const NLMISC::CEntityId &id2) const
	{
		return id1.getShortId() < id2.getShortId();
	}
};


/*inline std::stringstream &operator << (std::stringstream &__os, const CEntityId &__t)
{
	__os << __t.toString ();
	return __os;
}*/

} // NLMISC

#endif // NL_ENTITY_ID_H

/* End of entity_id.h */
