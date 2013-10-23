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

#ifndef RY_PDS_TABLE_H
#define RY_PDS_TABLE_H

//
// NeL includes
//
#include <nel/misc/types_nl.h>
#include <nel/misc/i_xml.h>

//
// STL includes
//
#include <map>

//
// PDS includes
//
#include "pds_database.h"
#include "pds_type.h"
#include "pds_attribute.h"
#include "pds_column.h"

//
// PDS Lib includes
//
#include "../pd_lib/pd_lib.h"
#include "../pd_lib/pds_common.h"
#include "../pd_lib/pds_types.h"
#include "../pd_lib/pd_server_utils.h"
#include "../pd_lib/db_reference_file.h"
#include "../pd_lib/pds_table_buffer.h"

class IDbFileStream;
class CDatabase;
class CTableNode;

//#define DEBUG_DATA_ACCESSOR


/**
 * A table able to update values into it
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CTable : public IRowProcessor, public CPDSLogger
{
public:

	/**
	 * Constructor
	 */
	CTable();

	/**
	 * Destructor
	 */
	~CTable();

	/**
	 * Clear table
	 */
	void				clear();

	/**
	 * Init table
	 * \return true iff success
	 */
	bool				init(CDatabase *database, const CTableNode& table);

	/// Initialized yet?
	bool				initialised() const			{ return _Init; }

	/**
	 * Post Init, called once all tables have been initialised
	 */
	bool				postInit();

	/**
	 * Build columns
	 */
	bool				buildColumns();



	/**
	 * Get parent Database
	 */
	const CDatabase*	getParent() const							{ return _Parent; }

	/**
	 * Get parent Database
	 */
	CDatabase*			getParent() 								{ return _Parent; }

	/**
	 * Get name
	 */
	const std::string&	getName() const								{ return _Name; }

	/**
	 * Get Id
	 */
	TTypeId				getId() const								{ return _Id; }

	/**
	 * Get Class Key
	 */
	uint32				getKey() const								{ return _Key; }

	/**
	 * Get inherited table
	 */
	TTypeId				getInheritTable() const						{ return _Inheritance; }

	/**
	 * Get Attributes
	 */
	const std::vector<CAttribute*>&	getAttributes() const			{ return _Attributes; }

	/**
	 * Get Attribute
	 */
	const CAttribute*	getAttribute(uint32 attribute) const		{ return attribute < _Attributes.size() ? _Attributes[attribute] : NULL; }

	/**
	 * Get Attribute
	 */
	const CAttribute*	getAttribute(const std::string &name) const;

	/**
	 * Get Columns
	 */
	const std::vector<CColumn>&		getColumns() const				{ return _Columns; }

	/**
	 * Get Column
	 */
	const CColumn*		getColumn(uint32 column) const				{ return column < _Columns.size() && _Columns[column].initialised() ? &_Columns[column] : NULL; }

	/**
	 * Get Column
	 */
	const CColumn*		getColumn(CLocatePath &path, bool verbose = true) const;




	/// \name Row Management
	// @{

	/**
	 * Allocate a row in a table
	 * \param row is the row to allocate
	 * Return true if succeded
	 */
	bool				allocate(RY_PDS::TRowIndex row, bool acquireRow = false);

	/**
	 * Deallocate a row in a table
	 * \param row is the row to deallocate
	 * Return true if succeded
	 */
	bool				deallocate(RY_PDS::TRowIndex row);

	/**
	 * Tells if a row is allocated
	 * \param row is the row to check
	 */
	bool				isAllocated(RY_PDS::TRowIndex row) const;

	/**
	 * Is Table Mapped
	 */
	bool				isMapped() const		{ return _Mapped; }

	/**
	 * Map a row
	 * \param index is the index to map
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	bool				mapRow(const RY_PDS::CObjectIndex &index, uint64 key);

	/**
	 * Unmap a row in a table
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	bool				unmapRow(uint64 key);

	/**
	 * Release a row in table
	 * \param row is the row to release
	 * Return true if succeded
	 */
	bool				release(RY_PDS::TRowIndex row);

	/**
	 * Release all rows in table
	 */
	bool				releaseAll();

	/**
	 * Get a mapped row
	 * \param key is the 64 bits row key
	 * Return a valid CObjectIndex if success
	 */
	RY_PDS::CObjectIndex	getMappedRow(uint64 key) const;


	/**
	 * Get the next unallocated row index in table
	 */
	RY_PDS::TRowIndex	nextUnallocatedRow() const					{ return _TableBuffer.nextUnallocatedRow(); }


	/**
	 * Get memory load
	 */
	uint32				getMemoryLoad() const						{ return _TableBuffer.getMemoryLoad(); }

	// @}




	/**
	 * Set value
	 */
	bool				set(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr);

	/**
	 * Set Parent
	 */
	bool				setParent(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const RY_PDS::CObjectIndex& parent);

	/**
	 * Get value
	 */
	bool				get(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint& datasize, void* dataptr, TDataType& type);



	/**
	 * Fetch a row into stream
	 * Fetch the whole object arborescence (i.e. children linked to this object)
	 * \param row is the row to fetch
	 * \param data is the stream store data into
	 */
	bool				fetch(RY_PDS::TRowIndex row, RY_PDS::CPData &data, bool fetchIndex = true);


	/**
	 * Build the index allocator for this table
	 */
	bool				buildIndexAllocator(RY_PDS::CIndexAllocator& alloc);




public:

	/**
	 * A data accessor, that handles embedded access to an object' value
	 */
	class CDataAccessor
	{
	public:

		/**
		 * Default constructor, this accessor is invalid
		 */
#ifdef DEBUG_DATA_ACCESSOR
		CDataAccessor() : _IsValid(false), _Table(NULL), _Attribute(NULL), _Column(NULL), _Data(NULL), _IsDebug(false)	{}
#else
		CDataAccessor() : _IsValid(false), _Table(NULL), _Attribute(NULL), _Column(NULL), _Data(NULL)	{}
#endif

		/**
		 * Destructor
		 */
		~CDataAccessor();

		/// Is accessor valid?
		bool						isValid() const						{ return _IsValid; }

		/**
		 * Get data as CObjectIndex
		 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an CObjectIndex)
		 */
		bool						getIndex(RY_PDS::CObjectIndex &index) const;

		/**
		 * Set data as CObjectIndex
		 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an CObjectIndex)
		 */
		bool						setIndex(const RY_PDS::CObjectIndex &index);

		/**
		 * Get data as a Set
		 * Return an accessor on a set, which is valid only if not issue occured
		 * Thus, you are able to modify the list own your own, add/remove items...
		 */
		RY_PDS::CSetMap::CAccessor	getSet();

		/**
		 * Get data as a Set
		 * Return an accessor on a set, which is valid only if not issue occured
		 */
		const RY_PDS::CSetMap::CAccessor	getSet() const;

		/**
		 * Get data as simple type value
		 * \param dataptr is the destination databuffer
		 * \param datasize is the destination buffer size
		 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an simple type)
		 * If datasize doesn't match, data are loaded anyway, but shrinked or truncated, and a warning is issued
		 */
		bool						getValue(void* dataptr, uint32 datasize) const;

		/**
		 * Set data as simple type value
		 * \param dataptr is the source databuffer
		 * \param datasize is the source buffer size
		 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an simple type)
		 * If datasize doesn't match, data are stored anyway, but shrinked or truncated, and a warning is issued
		 */
		bool						setValue(const void* dataptr, uint32 datasize);

		/**
		 * Get as enum/dimension
		 */
		bool						getAsIndexType(uint32& value) const;

		/**
		 * Set as enum/dimension
		 */
		bool						setAsIndexType(uint32 value);

		/**
		 * Get Back Ref Key value
		 */
		bool						getBackRefKey(uint32& key);

		/**
		 * Fetch column data into stream
		 */
		bool						fetch(RY_PDS::CPData &data);


		/**
		 * Ask Parent Table to release row
		 */
		//bool						releaseRow();

		/**
		 * Acquire row
		 */
		void						acquire();

		/**
		 * Unacquire row
		 */
		void						unacquire();


		/**
		 * toString()
		 */
		std::string					toString() const		{ return isValid() ? (_Table->getName()+":"+NLMISC::toString(_Accessor.row())+":"+_Column->getName()) : "<invalid>"; }


		/**
		 * Get table
		 */
		CTable*						table()					{ return isValid() ? _Table : NULL; }

		/**
		 * Get attribute
		 */
		const CAttribute*			attribute() const		{ return isValid() ? _Attribute : NULL; }

		/**
		 * Get column
		 */
		const CColumn*				column() const			{ return isValid() ? _Column : NULL; }

		/**
		 * Get Row
		 */
		RY_PDS::TRowIndex			row() const				{ return isValid() ? _Accessor.row() : RY_PDS::INVALID_ROW_INDEX; }

		/**
		 * Perform column integrity check
		 */
		bool						check() const;

		/**
		 * Get value as string
		 */
		std::string					valueAsString(bool expandSet = false) const;

		/**
		 * Get Column Index
		 * Return the Column Index of the column accessed
		 */
		RY_PDS::CColumnIndex		getColumnIndex() const
		{
			return isValid() ?
				RY_PDS::CColumnIndex((RY_PDS::TTableIndex)_Table->getId(), _Accessor.row(), (RY_PDS::TColumnIndex)_Column->getId()) :
				RY_PDS::CColumnIndex();
		}

		/**
		 * Get Object Index
		 * Return the Object Index of the whole row accessed
		 */
		RY_PDS::CObjectIndex		getObjectIndex() const
		{
			return isValid() ?
				RY_PDS::CObjectIndex((RY_PDS::TTableIndex)_Table->getId(), _Accessor.row()) :
				RY_PDS::CObjectIndex();
		}


		/**
		 * Dump accessor content and info to xml
		 */
		void						dumpToXml(NLMISC::IStream& xml, sint expandDepth = -1);

	private:

		/**
		 * Constructor
		 * Build a data accessor from a table, a row and an attribute id
		 * This constructor doesn't support Class and ArrayClass attributes
		 * \param object is the CObjectIndex of the row to peek/poke
		 * \param attribute is the attribute in the row
		 * \param index is the array index if the attribute is an array, set to 0 for other type
		 */
		CDataAccessor(CDatabase* root, const RY_PDS::CObjectIndex& object, uint32 attribute, TEnumValue arrayIndex);

		/** 
		 * Constructor
		 * Build a data accessor from a table, a row and an column id
		 * This constructor supports Class and ArrayClass attributes
		 * \param table is the table peek/poke into
		 * \param row is index to access
		 * \param column in the row
		 */
		CDataAccessor(CDatabase* root, const RY_PDS::CObjectIndex& object, RY_PDS::TColumnIndex column);

		/**
		 * Constructor
		 * Build a data accessor from a table, a data accessor and a column index
		 */
		CDataAccessor(CTable* table, CTableBuffer::CAccessor& data, RY_PDS::TColumnIndex column);

		/**
		 * Constructor
		 * Build a data accessor from another accessor and a column index.
		 * Used to access another field in a row
		 */
		CDataAccessor(const CDataAccessor& accessor, RY_PDS::TColumnIndex column);


#ifdef DEBUG_DATA_ACCESSOR
		/**
		 * Constructor
		 * Build a **debug** data accessor from row data and a column index.
		 */
		CDataAccessor(CTable* table, uint8* data, RY_PDS::TColumnIndex column);
#endif


		/**
		 * Seek to array index
		 * \param seek index in the array previously setup
		 * Seek doesn't apply to ArrayClass attributes
		 * Return true if success, false if not and accessor is invalidated
		 */
		bool						seek(TEnumValue index);

		/**
		 * Seek to array index
		 * \param backref is the back reference associated the current forward reference
		 * Seek doesn't apply to ArrayClass attributes
		 * Return true if success, false if not and accessor is invalidated
		 */
		bool						seek(CDataAccessor& backref);

		/**
		 * Check type of an index
		 */
		bool						checkType(const RY_PDS::CObjectIndex &object) const;

		/**
		 * Check an accessor as a PDS_Type accessor
		 */
		bool						checkAsTypeAccessor() const;

		/**
		 * Check an accessor as a PDS_BackRef or PDS_ForwardRef accessor
		 */
		bool						checkAsRefAccessor() const;

		/**
		 * Check an accessor as a PDS_Set accessor
		 */
		bool						checkAsSetAccessor() const;

		/**
		 * Dirty row
		 * Mark the whole row as modified, and to be stored at next delta backup time
		 */
		bool						dirtyRow();

		/**
		 * Setup Row Accessor
		 */
		bool						setupAccessor(RY_PDS::TRowIndex row);

		/**
		 * Invalidate accessor
		 */
		void						invalidate();

		/// Table to look into
		CTable*						_Table;

		/// Attribute
		const CAttribute*			_Attribute;

		/// Column
		const CColumn*				_Column;

		/// Row data accessor
		CTableBuffer::CAccessor		_Accessor;

		/// Pointer to object data (start pointer, in case of array)
		uint8*						_Data;

		/// Is valid
		bool						_IsValid;

#ifdef DEBUG_DATA_ACCESSOR
		/// Is debug
		bool						_IsDebug;
#endif

		friend class CTable;
	};


protected:

	virtual std::string	getLoggerIdentifier() const		{ return NLMISC::toString("tab:%s", (_Name.empty() ? "<unnamed>" : _Name.c_str())); }

public:

	/**
	 * Get accessor on data from a path
	 */
	CDataAccessor		getAccessor(CLocatePath &path);



	/// Display
	void				display(NLMISC::CLog *log = NLMISC::InfoLog, bool expanded = false, bool displayHeader = false) const;

	/// Display row
	void				displayRow(RY_PDS::TRowIndex row, NLMISC::CLog *log = NLMISC::InfoLog, bool displayHeader = false);

	/// Display row
	void				displayValue(RY_PDS::TRowIndex row, NLMISC::CLog *log = NLMISC::InfoLog);

	/// Dump Delta file content
	void				dumpDeltaFileContent(const std::string& filename, NLMISC::CLog *log = NLMISC::InfoLog) const;



	/**
	 * Dump table content and info at row to xml
	 */
	void				dumpToXml(RY_PDS::TRowIndex row, NLMISC::IStream& xml, sint expandDepth = -1);



	/// Rebuild forwardrefs from backrefs
	bool				rebuildForwardRefs();

	/// Reset forwardrefs
	bool				resetForwardRefs();

	/// Reset table map
	bool				resetTableMap();

	/// Rebuild table map
	bool				rebuildTableMap();

	/**
	 * Reset dirty tags
	 * Reset all rows so no one is marked as being dirty.
	 * This method fixes broken list issues
	 */
	bool				resetDirtyTags();

	/**
	 * Preload reference files
	 */
	bool				preloadRefFiles();



	/**
	 * Notify new Reference
	 * Internal table reference is reset and table is flushed from its released rows
	 */
	bool				notifyNewReference(CRefIndex& newref);

	/**
	 * Apply delta changes from a file
	 */
	bool				applyDeltaChanges(const std::string& filename);

	/**
	 * Build the delta file and purge all dirty rows in this table
	 */
	bool				buildDelta(const CTimestamp& starttime, const CTimestamp& endtime);

	/**
	 * Flush table from released rows
	 */
	bool				flushReleased();

	/**
	 * Build RowMapper
	 */
	bool				buildRowMapper();

	/**
	 * Process Row
	 */
	virtual bool		processRow(RY_PDS::TTableIndex table, CTableBuffer::CAccessor& accessor);

	/**
	 * Fix broken forward refs
	 */
	bool				fixForwardRefs();

	/**
	 * Fix row broken forward refs
	 * For each forward ref that should not contain null index, allocate a new row in
	 * the child table, then setup back and forward references
	 */
	bool				fixRowForwardRefs(RY_PDS::TRowIndex row);


private:

	/// Initialised yet?
	bool							_Init;


	/// Name of the table
	std::string						_Name;

	/// Id of the table
	TTypeId							_Id;

	/// Inheritance of the table
	TTypeId							_Inheritance;

	/// Parent database
	CDatabase*						_Parent;

	/// Attributes of the table
	std::vector<CAttribute*>		_Attributes;

	/// Columns of the table
	std::vector<CColumn>			_Columns;


	/// Key Attribute
	TTypeId							_Key;

	/// Table buffer
	CTableBuffer					_TableBuffer;

	/// Size of a row 
	uint32							_RowSize;

	/// Table is mapped
	bool							_Mapped;


	/// Empty Row pattern
	std::vector<uint8>				_EmptyRow;


private:

	friend class CDataAccessor;
	friend class CAttribute;
	friend class CDatabaseAdapter;

	/**
	 * Reset row to initial value
	 */
	bool				resetRow(uint8* rowData);


	/**
	 * Get Root Inheritance Table
	 */
	CTable*				getRootTable();

	/**
	 * Get Root Inheritance Table
	 */
	const CTable*		getRootTable() const;



	/**
	 * link a BackRef to a parent
	 * This method will perfom a full linking of child and new parent
	 * \param backref is an accessor on the BackRef
	 * \param parent is an index on the parent to link
	 * \param child is a remember of the child index
	 */
	bool				link(CDataAccessor &backref, const RY_PDS::CObjectIndex &parent, const RY_PDS::CObjectIndex &child);

	/**
	 * link a ForwardRef to a child
	 * This method will only link parent to child
	 * \param forwardref is an accessor on the BackRef
	 * \param child is an index on the child to link
	 */
	bool				forwardLink(CDataAccessor &backref, CDataAccessor &forwardref, const RY_PDS::CObjectIndex &child);


	/**
	 * Unlink a BackRef
	 * This method will perfom a full unlinking of child and current parent
	 * \param backref is an accessor on the BackRef
	 * \param child is a remember of the child index
	 */
	bool				unlink(CDataAccessor &backref, const RY_PDS::CObjectIndex &child);

	/**
	 * unlink a ForwardRef to a child
	 * This method will only unlink parent of child
	 * \param forwardref is an accessor on the BackRef
	 * \param child is an index on the child to link
	 */
	bool				forwardUnlink(CDataAccessor &backref, CDataAccessor &forwardref, const RY_PDS::CObjectIndex &child);

	/**
	 * Clear dirty list
	 * Reset dirty rows in the table. Only works properly if list is not broken
	 */
	bool				clearDirtyList();



	struct CBackRefFiller
	{
		const CColumn*			Column;
		const CAttribute*		Referenced;
		CTable*					ParentTable;
	};

	std::vector<CBackRefFiller>	BackRefInfo;

	struct CAutoForwardRefFiller
	{
		const CColumn*			Column;
	};

	std::vector<CAutoForwardRefFiller>	ForwardRefInfo;

	std::vector<RY_PDS::TRowIndex>	BrokenForwardRefs;


	/**
	 * Fill up Backward and Forward References information
	 */
	bool				fillRefInfo();

	/**
	 * Process Back Reference to a set
	 */
	bool				processBackRefToSet(CDataAccessor& parent, RY_PDS::CObjectIndex child);

	/**
	 * Process Back Reference to a set
	 */
	bool				processBackRefToForwardRef(CDataAccessor& parent, RY_PDS::CObjectIndex child);
};


// include inlines
#include "pds_table_inline.h"


#endif //RY_PDS_TABLE_H

