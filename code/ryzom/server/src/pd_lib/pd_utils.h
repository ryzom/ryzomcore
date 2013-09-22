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

#ifndef RY_PDS_UTILS_H
#define RY_PDS_UTILS_H

/*
 * NeL Includes
 */
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/variable.h>


namespace RY_PDS
{

/**
 * Definition of streamable persistent data.
 * Used to fetch data from PDS to service classes
 */
typedef NLMISC::IStream		CPData;





/**
 * PD System Verbosity control
 */
extern NLMISC::CVariable<bool>	PDVerbose;

/**
 * PD System Verbosity level
 */
extern NLMISC::CVariable<sint>	PDVerboseLevel;

/**
 * PDS Resolve Unmapped rows
 * If true, PDS continue loading when it finds a row not mapped in a mapped table,
 * and keeps it unmapped.
 * Otherwise, loading fails.
 */
extern NLMISC::CVariable<bool>	ResolveUnmappedRows;

/**
 * PDS Resolve Double Mapped Rows
 * If true, PDS continue loading when it finds a row already mapped, and keeps it unmapped.
 * Otherwise, loading fails.
 * See also ResolveDoubleMappedKeepOlder
 */
extern NLMISC::CVariable<bool>	ResolveDoubleMappedRows;

/**
 * PDS Resolve Double Mapped Keep Older
 * If true, when finds a doubly mapped row at loading, keep only older row as mapped (lesser row index)
 * See also ResolveDoubleMappedRows.
 */
extern NLMISC::CVariable<bool>	ResolveDoubleMappedKeepOlder;

/**
 * PDS Resolve Unallocated rows
 * If true, when finds a reference to an unallocated row or an invalid row, force reference to null.
 */
extern NLMISC::CVariable<bool>	ResolveInvalidRow;






/**
 * Definition of Table indices
 */
typedef uint16			TTableIndex;

/**
 * Definition of Row indices
 */
typedef uint32			TRowIndex;

/**
 * Definition of Column indices
 */
typedef uint16			TColumnIndex;

/**
 * Definition of index Checksum validator
 */
typedef uint16			TIndexChecksum;


/**
 * Table & Row indices constants
 */
const TTableIndex		INVALID_TABLE_INDEX = 0xffff;
const TRowIndex			INVALID_ROW_INDEX = 0xffffffff;
const TColumnIndex		INVALID_COLUMN_INDEX = 0xffff;

/**
 * Checksum
 */
const TIndexChecksum	VALID_INDEX_CHECKSUM = 0xdead;

/**
 * Default Hashing Function
 */
/*template<typename T>
class CDefaultHash
{
public:
	size_t	operator() (const T& value) const	{ return (uint32)value; }
};*/

/**
 * Table Container Interface
 */
class ITableContainer
{
public:

	/// Get Table Index
	virtual TTableIndex		getTableIndex(const std::string& tableName) const = 0;

	/// Get Table Name
	virtual std::string		getTableName(TTableIndex tableIndex) const = 0;
};



/**
 * Definition of Object indices
 * An object is pointed by a table index and a row index
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CObjectIndex
{
public:

	/// Constructor of invalid index
	explicit CObjectIndex(bool validateChecksum = false) : _Row(INVALID_ROW_INDEX), _Table(INVALID_TABLE_INDEX), _Checksum((TIndexChecksum)~VALID_INDEX_CHECKSUM)
	{
		if (validateChecksum)
			validate();
		else
			invalidate();
	}

	/// Constructor
	CObjectIndex(TTableIndex table, TRowIndex row) : _Row(row), _Table(table)		{ validate(); }

	/// Constructor
	CObjectIndex(const CObjectIndex &index)											{ *this = index; }





	/// Cast operator
	operator uint64 () const														{ return _FullIndex; }

	/// Get Table index
	TTableIndex		table() const													{ return _Table; }

	/// Get Row index
	TRowIndex		row() const														{ return _Row; }




	/// Copy
	CObjectIndex	& operator = (const CObjectIndex &index)						{ _FullIndex = index._FullIndex; return *this; }

	/// Equal?
	bool			operator == (const CObjectIndex &index) const					{ return _FullIndex == index._FullIndex; }

	/// Not equal?
	bool			operator != (const CObjectIndex &index) const					{ return !(*this == index); }



	/// Return null
	static CObjectIndex		null()
	{
		return CObjectIndex(true);
	}



	/// Checksum valid?
	bool			isChecksumValid() const
	{
		return getChecksum() == VALID_INDEX_CHECKSUM;
	}

	/// Is it Null Ptr?
	bool			isNull() const
	{
		return _Table == INVALID_TABLE_INDEX && _Row == INVALID_ROW_INDEX && isChecksumValid();
	}

	/// Validity check
	bool			isValid() const
	{
		return _Table != INVALID_TABLE_INDEX && _Row != INVALID_ROW_INDEX && isChecksumValid();
	}




	/// transform to string
	std::string		toString(const ITableContainer* container = NULL) const
	{
		if (isValid())
		{
			if (container != NULL)
			{
				return NLMISC::toString("(%s:%u)", container->getTableName(_Table).c_str(), _Row);
			}
			else
			{
				return NLMISC::toString("(%u:%u)", _Table, _Row);
			}
		}
		else if (isNull())
		{
			return "<Null>";
		}
		else
		{
			return NLMISC::toString("(%u:%u <invalid>)", _Table, _Row);
		}
	}

	/// get from string
	void			fromString(const char* str, const ITableContainer* container = NULL)
	{
		uint		table;
		uint		row;
		uint		valid;

		if (sscanf(str, "(%d:%d:%d)", &table, &row, &valid) == 3)
		{
			_Table = (TTableIndex)table;
			_Row = (TRowIndex)row;

			if (valid != 0)
				validate();
			else
				invalidate();

			return;
		}
		else if (sscanf(str, "(%d:%d)", &table, &row) == 2)
		{
			_Table = (TTableIndex)table;
			_Row = (TRowIndex)row;
			validate();

			return;
		}
		else if (container != NULL && parseIndexWithName(str, container))
		{
			validate();
			return;
		}

		*this = CObjectIndex::null();
		return;
	}

	void			serial(NLMISC::IStream& f)
	{
		f.serial(_FullIndex);
	}

private:

	bool			parseIndexWithName(const char* str, const ITableContainer* container)
	{
		TRowIndex	row;
		TTableIndex	table;

		if (*(str++) != '(')
			return false;

		std::string	tableName;

		while (*str != '\0' && *str != ':')
			tableName += *(str++);

		if (*(str++) != ':')
			return false;

		table = container->getTableIndex(tableName);
		if (table == INVALID_TABLE_INDEX)
			return false;

		NLMISC::fromString(std::string(str), row);

		while (*str >= '0' && *str <= '9')
			++str;

		if (*str != ')')
			return false;

		_Row = row;
		_Table = table;

		return true;
	}


#ifdef NL_OS_WINDOWS

	#define PDS_ROW_LO_WORD		0
	#define PDS_ROW_HI_WORD		1
	#define PDS_TABLE_WORD		2
	#define PDS_CHECKSUM_WORD	3

#else

	#define PDS_ROW_LO_WORD		0
	#define PDS_ROW_HI_WORD		1
	#define PDS_TABLE_WORD		2
	#define PDS_CHECKSUM_WORD	3

#endif

	union
	{
		struct
		{
			TRowIndex			_Row;
			TTableIndex			_Table;
			TIndexChecksum		_Checksum;
		};
		uint64					_FullIndex;
		uint16					_Words[4];
	};

	/// Compute partial checksum of the index
	TIndexChecksum		getPartialChecksum() const
	{
		return _Words[PDS_ROW_LO_WORD] ^ _Words[PDS_ROW_HI_WORD] ^ _Words[PDS_TABLE_WORD];
	}

	/// Get checksum of the index
	TIndexChecksum		getChecksum() const
	{
		return getPartialChecksum() ^ _Words[PDS_CHECKSUM_WORD];
	}

	/// Force validation of an index
	void				validate()
	{
		_Checksum = getPartialChecksum() ^ VALID_INDEX_CHECKSUM;
	}

	/// Force invalidation of an index
	void				invalidate()
	{
		_Checksum = getPartialChecksum() ^ (~VALID_INDEX_CHECKSUM);
	}
};



/**
 * Definition of column index
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CColumnIndex
{
public:

	/// Constructor
	explicit CColumnIndex(TTableIndex table = INVALID_TABLE_INDEX, TRowIndex row = INVALID_ROW_INDEX, TColumnIndex column = INVALID_COLUMN_INDEX)
		: _Table(table), _Column(column), _Row(row)
	{
	}

	/// Constructor
	explicit CColumnIndex(const CObjectIndex& object, TColumnIndex column)
		: _Table(INVALID_TABLE_INDEX), _Column(INVALID_COLUMN_INDEX), _Row(INVALID_ROW_INDEX)
	{
		if (!object.isValid())
			return;

		_Table = object.table();
		_Row = object.row();
		_Column = column;
	}


	/// Get Table Index
	TTableIndex		table() const					{ return _Table; }
	/// Get Row Index
	TRowIndex		row() const						{ return _Row; }
	/// Get Column Index
	TColumnIndex	column() const					{ return _Column; }

	/// Test if is null
	bool			isNull() const					{ return _Table == INVALID_TABLE_INDEX && _Row == INVALID_ROW_INDEX && _Column == INVALID_COLUMN_INDEX; }


	/// Cast to 64 bits
	operator uint64() const							{ return _FullIndex; }

	/// 32 bits hash key
	uint32			hash() const					{ return (uint32) ((_Table<<2) ^ (_Row<<1) ^ _Column); }

	/// Operator <
	bool			operator < (const CColumnIndex& b) const	{ return _FullIndex < b._FullIndex; }

	/// transform to string
	std::string		toString(const ITableContainer* container = NULL) const
	{
		if (isNull())
		{
			return "<Null>";
		}
		else if (container != NULL)
		{
			return NLMISC::toString("(%s:%u:%u)", container->getTableName(_Table).c_str(), _Row, _Column);
		}
		else
		{
			return NLMISC::toString("(%u:%u:%u)", _Table, _Row, _Column);
		}
	}

private:

	union
	{
		struct
		{
			/// Table Index
			TTableIndex		_Table;

			/// Column Index in row
			TColumnIndex	_Column;

			/// Row Index in table
			TRowIndex		_Row;
		};

		/// Full 64 bits index
		uint64				_FullIndex;
	};
};

/**
 * Definition of set of object indexes
 * This is actually a index on the first element of a list contained in each database
 */
typedef std::vector<CObjectIndex>		TIndexList;


/**
 * A container of index lists
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */

//#define DEBUG_SETMAP_ACCESSOR

struct CColumnIndexHashMapTraits
{
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
	CColumnIndexHashMapTraits() { }
	size_t operator() (const CColumnIndex &id) const
		{
			return id.hash();
		}
};

class CSetMap
{
public:

	/// Hash Key
	/*class CKeyHash
	{
	public:
		size_t	operator() (const CColumnIndex& key) const	{ return (uint32)(key.hash()); }
	};*/

	/// Map of lists
	typedef CHashMap<CColumnIndex, TIndexList, CColumnIndexHashMapTraits>			TListMap;

	/// An Accessor on a list
	class CAccessor
	{
	public:

		/// Constructor, only builds invalid accessor
		CAccessor() : _Valid(false)										{ }

		/// Is Valid
		bool				isValid() const								{ return _Valid; }

		/// Get whole list
		const TIndexList&	get() const									{ nlassert(isValid()); return (*_It).second; }

		/// Test if object belongs to list
		bool				belongsTo(const CObjectIndex& test) const
		{
			nlassert(isValid());
			return std::find((*_It).second.begin(), (*_It).second.end(), test) != (*_It).second.end();
		}

		/// Add index to list
		void				add(const CObjectIndex& index)
		{
			nlassert(isValid());
			if (!belongsTo(index))
			{
				(*_It).second.push_back(index);
			}
		}

		/// Remove index from list
		void				erase(const CObjectIndex& index)
		{
			nlassert(isValid());

			TIndexList&                    iList = (*_It).second;
			TIndexList::iterator     itr;
			for (itr=iList.begin(); itr!=iList.end(); )
				itr = ((*itr == index) ? iList.erase(itr) : (itr+1));

			// gcc can't resolve this:
			//(*_It).second.erase(std::remove((*_It).second.begin(), (*_It).second.end(), index), (*_It).second.end());
		}

		/// Dump list
		void				display() const
		{
			if (!isValid())
			{
				nlinfo("<can't display set>");
				return;
			}

#ifdef DEBUG_SETMAP_ACCESSOR
			nlinfo("Set '%s' content:", _Debug.toString().c_str());
#else
			nlinfo("Set content:");
#endif
			TIndexList::const_iterator	it;
			for (it=(*_It).second.begin(); it!=(*_It).second.end(); ++it)
				nlinfo("%s", (*it).toString().c_str());
		}

	private:

		friend class CSetMap;

		/// Internal pointer to list
		TListMap::iterator	_It;

		/// Is Valid
		bool				_Valid;

#ifdef DEBUG_SETMAP_ACCESSOR

		CColumnIndex		_Debug;

		/// constructor for CIndexListMap
		CAccessor(TListMap::iterator it, const CColumnIndex& index) : _It(it), _Valid(true), _Debug(index)		{ }

#else

		/// constructor for CIndexListMap
		CAccessor(TListMap::iterator it, const CColumnIndex& index) : _It(it), _Valid(true)						{ }

#endif

	};


	/// Get the list associated to this column
	CAccessor				get(const CColumnIndex& index)
	{
		TListMap::iterator	it = _Map.find(index);
		if (it == _Map.end())
		{
			it = _Map.insert(TListMap::value_type(index, TIndexList())).first;
		}
		return CAccessor(it, index);
	}

	/// Clear up whole map
	void					clear()										{ _Map.clear(); }

private:

	friend class CAccessor;

	TListMap		_Map;

};





/**
 * Base class for Row accessible objects.
 * Contains a Row index, used to represent the object in the database
 * Classes that derive from this class are assumed to know their own Table index.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class IPDBaseData
{
public:

	/// Constructor
	IPDBaseData() : __BaseRow(INVALID_ROW_INDEX), __BaseTable(INVALID_TABLE_INDEX) {}

	/// Get Row index
	TRowIndex	getRow() const	{ return __BaseRow; }

	/// Get Table index
	TTableIndex	getTable() const	{ return __BaseTable; }

protected:

	TRowIndex	__BaseRow;
	TTableIndex	__BaseTable;

	friend class CPDSLib;
};



/**
 * A class that allocates and deallocate indices in PDS database.
 * The allocator is able to init itself from a PDS command
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CIndexAllocator
{
public:

	CIndexAllocator() : _NextIndex(0)	{ }

	TRowIndex			allocate()
	{
		// no index in stack, get next
		if (_FreeIndices.empty())
			return _NextIndex++;

		// pop a free index
		TRowIndex	index = _FreeIndices.front();
		_FreeIndices.pop_front();
		return index;
	}

	void				deallocate(TRowIndex index)
	{
		// push index on stack
		_FreeIndices.push_back(index);
	}

	void				forceAllocated(TRowIndex index)
	{
		for (; _NextIndex<index; ++_NextIndex)
			_FreeIndices.push_back(_NextIndex);

		// remove index from free if was there
		std::deque<TRowIndex>::iterator	it = std::find(_FreeIndices.begin(), _FreeIndices.end(), index);
		if (it != _FreeIndices.end())
			_FreeIndices.erase(it);

		if (_NextIndex < index+1)
			_NextIndex = index+1;
	}

	void				clear()
	{
		_NextIndex = 0;
		_FreeIndices.clear();
	}

	void				serial(NLMISC::IStream& f)
	{
		f.serialCheck(NELID("IALC');
		f.serialVersion(0);

		f.serial(_NextIndex);
		f.serialCont(_FreeIndices);
	}

private:

	/// Next free index
	TRowIndex						_NextIndex;

	/// Free items
	std::deque<TRowIndex>			_FreeIndices;
};






typedef IPDBaseData*	(*TPDFactory)();
typedef void			(*TPDFetch)(IPDBaseData*, CPData&);
typedef void			(*TPDFetchFailure)(uint64 key);



/**
 * Get Value as String
 */
template<typename T>
std::string			pdsToString(const T& value)					{ return NLMISC::toString(value); }

inline std::string	pdsToString(const bool& value)				{ return value ? std::string("true") : std::string("false"); }
inline std::string	pdsToString(const CObjectIndex& value)		{ return value.toString(); }
inline std::string	pdsToString(const CColumnIndex& value)		{ return value.toString(); }
inline std::string	pdsToString(const NLMISC::CSheetId& value)	{ return value.toString(); }
inline std::string	pdsToString(const NLMISC::CEntityId& value)	{ return value.toString(); }


}; // RY_PDS

#endif //RY_PDS_UTILS_H



