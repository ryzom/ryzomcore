// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef RY_PD_SERVER_UTILS_H
#define RY_PD_SERVER_UTILS_H

/*
 * Includes
 */
#include <nel/misc/types_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/bit_set.h>
#include <nel/misc/variable.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "pd_utils.h"
#include "timestamp.h"


/**
 * CRefIndex, permanent info about the database
 * This class is not intended to be modified, only create when a valid
 * reference is ready
 * To store volatile data, see CDatabaseState.
 */
class CRefIndex
{
public:

	/// Database Id
	uint32		DatabaseId;

	/// Numeral index of the reference
	uint32		Index;

	/// Directory path of the reference, this MUST local path from root database path
	std::string	Path;

	/// Timestamp, in the form 'YYYY.MM.DD.hh.mm.ss'
	//std::string	Timestamp;
	CTimestamp	Timestamp;

	void		serial(NLMISC::IStream& s)
	{
		s.xmlPush("reference");

		s.serialCheck(NELID("RIDX');
		uint	version = s.serialVersion(0);

		s.xmlPush("database");
		s.serial(DatabaseId);
		s.xmlPop();

		s.xmlPush("index");
		s.serial(Index);
		s.xmlPop();

		s.xmlPush("path");
		s.serial(Path);
		s.xmlPop();

		s.xmlPush("timestamp");
		if (s.isReading())
		{
			std::string	ts;
			s.serial(ts);
			Timestamp.fromString(ts.c_str());
		}
		else
		{
			std::string	ts = Timestamp.toString();
			s.serial(ts);
		}
		//s.serial(Timestamp);
		s.xmlPop();

		s.xmlPop();
	}

	/// Constructor
	CRefIndex(uint32 databaseId) : DatabaseId(databaseId), Index(0)
	{
		setup();
	}

	/// Setup
	void	setup()
	{
		Path = "ref."+NLMISC::toString("%08X", Index);
		setTimestamp();
	}

	/**
	 * Set Time stamp
	 */
	void			setTimestamp();


	/**
	 * Load a Reference index file
	 */
	bool			load(const std::string& filename);

	/**
	 * Load a Reference index file
	 */
	bool			load();

	/**
	 * Save a Reference index file
	 */
	bool			save();

	/**
	 * Save a Reference index file
	 */
	bool			save(const std::string& filename);

	/**
	 * Build next Reference index file
	 */
	bool			buildNext();

	/**
	 * Get next Reference index file
	 */
	void			getNext();

	/**
	 * Get (and setup if needed) database root path
	 */
	std::string		getRootPath();

	/**
	 * Get reference path
	 */
	std::string		getPath();

	/**
	 * Set As Valid Reference
	 */
	bool			setAsValidRef();

	/**
	 * Get Nominal Root Path
	 */
	std::string		getNominalRootPath();


	/**
	 * Get Seconds update path
	 */
	std::string		getSecondsUpdatePath();

	/**
	 * Get Minutes update path
	 */
	std::string		getMinutesUpdatePath();

	/**
	 * Get Hours update path
	 */
	std::string		getHoursUpdatePath();

	/**
	 * Get Log path
	 */
	std::string		getLogPath();

private:

	/**
	 * Setup reference directory
	 */
	bool			setupDirectory();

	/**
	 * Check directory
	 */
	bool			checkDirectory(const std::string& path);
};




/**
 * CDatabaseState
 * This class contains 'volatile' state of the database
 * This class is update at each update, and written on HD at the end of update.
 * File on HD is used when database is checked at startup (and if it fails,
 * database should not be initialised unless maintenance fixes the problem)
 */
class CDatabaseState
{
public:

	/// Constructor
	CDatabaseState();

	/// Database Name
	std::string			Name;

	/// Database Id
	uint32				Id;

	/// Last Update Id
	uint32				LastUpdateId;

	/// Current Valid Index
	uint32				CurrentIndex;

	/// End Database Update Timestamp
	CTimestamp			EndTimestamp;


	/// Serial method
	void				serial(NLMISC::IStream& s);

	/// Save State
	bool				save(CRefIndex& ref);

	/// Load State
	bool				load(CRefIndex& ref, bool usePrevious = false);

	/// Load State
	bool				load(const std::string& rootpath, bool usePrevious = false);

	/// State exists in path
	static bool			exists(const std::string& rootpath);


	/// Get state filename
	static std::string	fileName()					{ return "state"; }

	/// Get state filename
	static std::string	fileName(CRefIndex& ref)	{ return ref.getRootPath()+"state"; }


private:

};







class CMixedStreamFile : public NLMISC::IStream
{
public:

	/// Constructor
	CMixedStreamFile() : NLMISC::IStream(false), _File(NULL)
	{
	}

	/// Destructor
	virtual ~CMixedStreamFile()
	{
		close();
	}

	bool				open(const char* filename, const char* mode = "rb")
	{
		if (_File != NULL)
			return false;

		_File = fopen(filename, mode);
		if (_File == NULL)
			return false;

		return true;
	}

	virtual void		close()
	{
		if (_File != NULL)
			fclose(_File);
		_File = NULL;
	}

	void				flush()
	{
		if (_File != NULL)
			fflush(_File);
	}

protected:

	/// standard FILE handler
	FILE*				_File;

	bool				readBuffer(void* buf, uint len)					{ if (_File == NULL) return false; uint32 rb = (uint32)fread(buf, 1, len, _File); _ReadBytes += rb; return rb == len; }
	bool				writeBuffer(const void* buf, uint len)			{ if (_File == NULL) return false; uint32 wb = (uint32)fwrite(buf, 1, len, _File); _WrittenBytes += wb; return wb == len; }
	bool				readBuffer(void* buf, uint len, uint& readlen)	{ if (_File == NULL) return false; readlen = (uint)fread(buf, 1, len, _File); return readlen == len; }

	static uint64		_ReadBytes;
	static uint64		_WrittenBytes;

	template<typename T>
	bool				readValue(T& v)									{ return readBuffer(&v, sizeof(v)); }

	template<typename T>
	bool				writeValue(const T& v)							{ return writeBuffer(&v, sizeof(v)); }

public:

	virtual void		serialBuffer(uint8 *buf, uint len)
	{
		if (_File == NULL)
			throw NLMISC::EStream("CMixedStreamFile not opened");

		if (isReading())
		{
			if (!readBuffer(buf, len))
				throw NLMISC::EStream("file error "+NLMISC::toString(ferror(_File)));
		}
		else
		{
			if (!writeBuffer(buf, len))
				throw NLMISC::EStream("file error "+NLMISC::toString(ferror(_File)));
		}
	}

	virtual void		serialBit(bool &bit)
	{
		if (isReading())
		{
			uint8	b;
			serial(b);
			bit = (b != 0);
		}
		else
		{
			uint8	b = bit;
			serial(b);
		}
	}
};




class CRowMapper
{
public:

	/// Constructor
	CRowMapper();

	/// Clear up mapper
	void					clear();

	/// Set link
	void					link(CRowMapper* link)				{ _Link = link; }


	/// Key
	typedef uint64	TKey;



	/**
	 * Declares a row as allocated
	 */
	bool					allocate(RY_PDS::TRowIndex row);

	/**
	 * Declares a row as deallocated
	 */
	bool					deallocate(RY_PDS::TRowIndex row);

	/**
	 * Checks if a row is allocated
	 */
	bool					allocated(RY_PDS::TRowIndex row) const;

	/**
	 * Get Number of Allocated rows
	 */
	uint32					numAllocated() const			{ return _AllocatedRows; }

	/**
	 * Get Max row index used
	 */
	RY_PDS::TRowIndex		maxRowIndex() const				{ return _RowState.size(); }

	/**
	 * Get Next Unalloocated row
	 */
	RY_PDS::TRowIndex		nextUnallocatedRow() const;



	/**
	 * Map a key to an Row
	 */
	bool					map(TKey key, const RY_PDS::CObjectIndex& index);

	/**
	 * Is Row Mapped
	 */
	bool					isMapped(TKey key);

	/**
	 * Get Row from key
	 */
	RY_PDS::CObjectIndex	get(TKey key) const;

	/**
	 * Unmap a key of an Row
	 */
	bool					unmap(TKey key);

	/**
	 * Get Number of Mapped rows
	 */
	uint32					numMapped() const				{ return (uint32)_KeyMap.size(); }



private:

	/**
	 * Bitset of allocated rows, each bit indicates if corresponding row is allocated
	 */
	typedef NLMISC::CBitSet							TRowState;
	TRowState		_RowState;

	/// Total number of allocated rows
	uint32			_AllocatedRows;

	/// Key Hash
	class CKeyHash
	{
	public:
		size_t	operator() (const TKey& key) const	{ return ((uint32)key) ^ ((uint32)(key >> 32)); }
	};

	typedef CHashMap<TKey, RY_PDS::CObjectIndex>	TKeyMap;
	TKeyMap			_KeyMap;

	/// Row Mapper link, in case of mapped table that inherit from another
	CRowMapper*		_Link;
};




/**
 * Debug/Log Interface
 */
class CPDSLogger
{
public:

	CPDSLogger() : _ParentLogger(NULL)					{ }

	void	setParentLogger(const CPDSLogger* parent)	{ _ParentLogger = parent; }


	/**
	 * Get Contextual Identifier String
	 */
	std::string	getContextualIndentifier() const;

protected:

	/**
	 * Get Internal Logger Identifier
	 * An identifier should ressemble like 'type:id', where type is the 'class'
	 * of the logger, and id is a unique identifier of the instanciated object of 'class'
	 */
	virtual std::string	getLoggerIdentifier() const = 0;

private:

	/// Parent Logger
	const CPDSLogger*		_ParentLogger;

};

#define	PDS_FULL_DEBUG_LEVEL	3
#define	PDS_NORMAL_DEBUG_LEVEL	2
#define	PDS_MINIMAL_DEBUG_LEVEL	1
#define	PDS_NO_DEBUG_LEVEL		0


#define	PDS_DEBUG_LEVEL			PDS_FULL_DEBUG_LEVEL


#define PDS_DEBUG_IN_L(obj, level)							if (level <= 0 || !RY_PDS::PDVerbose || level > RY_PDS::PDVerboseLevel) {} else NLMISC::createDebug (), NLMISC::DebugLog->setPosition( __LINE__, __FILE__ ),	NLMISC::DebugLog->display("%s:", obj->getContextualIndentifier().c_str()),		NLMISC::DebugLog->displayNL

#define PDS_DEBUG_IN(obj)									PDS_DEBUG_IN_L(obj, PDS_DEBUG_LEVEL)
#define PDS_INFO_IN(obj)									NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ),		NLMISC::InfoLog->display("%s:", obj->getContextualIndentifier().c_str()),		NLMISC::InfoLog->displayNL
#define PDS_WARNING_IN(obj)									NLMISC::createDebug (), NLMISC::WarningLog->setPosition( __LINE__, __FILE__ ),	NLMISC::WarningLog->display("%s:", obj->getContextualIndentifier().c_str()),	NLMISC::WarningLog->displayNL

#define	PDS_DEBUG_L(level)	PDS_DEBUG_IN_L(this, level)

#define	PDS_FULL_DEBUG		PDS_DEBUG_L(PDS_FULL_DEBUG_LEVEL)
#define PDS_DEBUG			PDS_DEBUG_L(PDS_DEBUG_LEVEL)
#define PDS_INFO			PDS_INFO_IN(this)
#define PDS_WARNING			PDS_WARNING_IN(this)

#define	PDS_LOG_DEBUG(level)	if (level <= 0 || !RY_PDS::PDVerbose || level > RY_PDS::PDVerboseLevel) {} else nldebug



/*
 *
 */


/*
 * Inlines
 */


/*
 * Get Contextual Identifier String
 */
inline std::string	CPDSLogger::getContextualIndentifier() const
{
	if (_ParentLogger != NULL)
		return _ParentLogger->getContextualIndentifier()+"|"+getLoggerIdentifier();
	else
		return getLoggerIdentifier();
}



/*
 * Constructor
 */
inline CRowMapper::CRowMapper()
{
	clear();
}

/*
 * Clear up mapper
 */
inline void	CRowMapper::clear()
{
	_Link = NULL;
	_AllocatedRows = 0;
	_RowState.clearAll();
	_KeyMap.clear();
}

/*
 * Declares a row as allocated
 */
inline bool	CRowMapper::allocate(RY_PDS::TRowIndex row)
{
	// check for room
	if (row >= _RowState.size())
		_RowState.resizeNoReset(row+1);

	// row already allocated?
	if (_RowState.get(row))
		return false;

	_RowState.set(row);
	++_AllocatedRows;

	return true;
}

/*
 * Declares a row as deallocated
 */
inline bool	CRowMapper::deallocate(RY_PDS::TRowIndex row)
{
	// check bit not outside bitset
	if (row >= _RowState.size())
		return false;

	// row allocated?
	if (!_RowState.get(row))
		return false;

	_RowState.clear(row);
	--_AllocatedRows;

	return true;
}

/*
 * Checks if a row is allocated
 */
inline bool	CRowMapper::allocated(RY_PDS::TRowIndex row) const
{
	return row < _RowState.size() && _RowState.get(row);
}


/*
 * Get Next Unalloocated row
 */
inline RY_PDS::TRowIndex	CRowMapper::nextUnallocatedRow() const
{
	RY_PDS::TRowIndex	row;

	// go through all rows till found a free row
	for (row=0; allocated(row); ++row)
		;

	return row;
}




/*
 * Map a key to an Row
 */
inline bool	CRowMapper::map(TKey key, const RY_PDS::CObjectIndex& index)
{
	if (_Link != NULL)
	{
		return _Link->map(key, index);
	}

	if (_KeyMap.find(key) != _KeyMap.end())
	{
		nlwarning("CRowMapper::map(): cannot map '%016"NL_I64"X' to '%d', already mapped", key, index.toString().c_str());
		return false;
	}

	_KeyMap[key] = index;

	return true;
}

/*
 * Is Row Mapped
 */
inline bool	CRowMapper::isMapped(TKey key)
{
	if (_Link != NULL)
	{
		return _Link->isMapped(key);
	}

	return (_KeyMap.find(key) != _KeyMap.end());
}

/*
 * Get Row from key
 */
inline RY_PDS::CObjectIndex	CRowMapper::get(TKey key) const
{
	if (_Link != NULL)
	{
		return _Link->get(key);
	}

	TKeyMap::const_iterator	it = _KeyMap.find(key);

	if (it == _KeyMap.end())
	{
		PDS_LOG_DEBUG(1)("CRowMapper::get(): key '%016"NL_I64"X' not mapped", key);
		return RY_PDS::CObjectIndex::null();
	}

	return (*it).second;
}

/*
 * Unmap a key of an Row
 */
inline bool	CRowMapper::unmap(TKey key)
{
	if (_Link != NULL)
	{
		return _Link->unmap(key);
	}

	TKeyMap::iterator	it = _KeyMap.find(key);

	if (it == _KeyMap.end())
	{
		nlwarning("CRowMapper::unmap(): key '%016"NL_I64"X' not mapped", key);
		return false;
	}

	_KeyMap.erase(it);

	return true;
}


#endif //RY_PDS_LIB_H



