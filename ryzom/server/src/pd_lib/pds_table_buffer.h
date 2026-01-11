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

#ifndef NL_PDS_TABLE_BUFFER_H
#define NL_PDS_TABLE_BUFFER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/path.h>

#include "../pd_lib/pd_utils.h"
#include "../pd_lib/pd_server_utils.h"
#include "../pd_lib/db_reference_file.h"
#include "../pd_lib/db_delta_file.h"


class IRowProcessor;

/**
 * A memory cache for table
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CTableBuffer : public CPDSLogger
{
public:

	/// Constructor
	CTableBuffer();

	/// Destructor
	~CTableBuffer();

	/// Clear all
	void				clear();

	/// Init
	void				init(uint32 tableId, uint32 rowSize, bool mapped);

	/// Setup Ref
	void				setupRef(CRefIndex& ref);




	/// Init, only for reference builder
	void				init(uint32 tableId, const std::string& refRootPath, const std::string& refPath); //CRefIndex& ref);



	/// Row Data Type
	typedef uint8*		TRowData;

	/// Map of Rows
	typedef CHashMap<RY_PDS::TRowIndex, TRowData>	TRowMap;

	/**
	 * Row Accessor, embeds any row access
	 */
	class CAccessor
	{
	public:

		/// Get row index
		RY_PDS::TRowIndex	row() const				{ return (*_MapIt).first; }

		/// Get row data
		TRowData			data()					{ return (*_MapIt).second + (_Mapped ? sizeof(CMappedHeader) : sizeof(CHeader)); }

		/// Get row data
		TRowData			data() const			{ return (*_MapIt).second + (_Mapped ? sizeof(CMappedHeader) : sizeof(CHeader)); }

		/// Default Constructor, should never be used
		explicit CAccessor()						{ }



		/// Is Row Allocated
		bool				allocated() const		{ return ((CHeader*)fullRow())->allocated(); }

		/// Is Row Mapped
		bool				mapped() const			{ return _Mapped && ((CMappedHeader*)fullRow())->mapped(); }

		/// Is Row Dirty
		bool				dirty() const			{ return ((CHeader*)fullRow())->dirty(); }

		/// Get Dirt stamp
		uint32				dirtyStamp() const		{ return ((CHeader*)fullRow())->getDirtStamp(); }

		/// Get Map Key
		uint64				key() const				{ return mapped() ? ((CMappedHeader*)fullRow())->getKey() : 0; }

		/// Get Full Row Data
		TRowData			fullRow() const			{ return (*_MapIt).second; }


		/// Equals
		bool				operator == (const CAccessor& access) const	{ return _Mapped == access._Mapped && _MapIt == access._MapIt; }


		/**
		 * Copy header info from another accessor
		 * Warning, this is only a local copy of header, data won't been updated in parent CTableBuffer
		 */
		void				copyHeader(const CAccessor& access);

	private:

		friend class CTableBuffer;

		/// Accessor in table map
		typedef TRowMap::iterator	TMapIt;
		TMapIt				_MapIt;

		bool				_Mapped;

		/// Constructor for CTableBuffer
		CAccessor(TMapIt it, bool mapped) : _MapIt(it), _Mapped(mapped)		{ }

		/// Get Full Row Data
		TRowData			fullRow()				{ return (*_MapIt).second; }
	};


	
	/**
	 * Return a pointer to straight row data
	 * If row is not loaded, data are loaded from table file
	 */
	CAccessor			getRow(RY_PDS::TRowIndex row);

	/**
	 * Return a pointer to straight new row data
	 * Row is filled with blank, not loaded from file
	 */
	CAccessor			getEmptyRow(RY_PDS::TRowIndex row);

	/**
	 * Acquire a Row
	 * Row is marked as non releasable until it is released enough time
	 */
	bool				acquireRow(CAccessor accessor);

	/**
	 * Release a Row
	 * Row is marked as purgeable, and will be purged as soon as possible, 
	 * unless it is reactivated before purge
	 */
	bool				releaseRow(CAccessor accessor);

	/**
	 * Release a Row
	 * Row is marked as purgeable, and will be purged as soon as possible, 
	 * unless it is reactivated before purge
	 */
	bool				releaseRow(RY_PDS::TRowIndex row);

	/**
	 * Release all rows
	 * To be called when client disconnects, so all loaded rows may be released without
	 * client releasing them manually
	 */
	bool				releaseAll();



	/**
	 * Mark a Row as being dirty for later delta save
	 */
	bool				dirtyRow(CAccessor accessor);



	/**
	 * Build Delta
	 */
	bool				buildDelta(const CTimestamp& starttimestamp, const CTimestamp& endtimestamp);

	/**
	 * Apply delta changes from a file
	 */
	bool				applyDeltaChanges(const std::string& filename);

	/**
	 * Flush Released Rows from memory
	 */
	void				flushReleased();

	/**
	 * Reset dirty tags
	 */
	void				resetDirty();

	/**
	 * Flush Reference files
	 */
	void				flushRefFiles();

	/**
	 * Purge all references
	 * Flush loaded reference files, and close them all.
	 */
	bool				purgeReferences();

	/**
	 * Open all reference files in reference directory to update
	 */
	bool				openAllRefFilesWrite();

	/**
	 * Open all reference files in reference directory to read
	 */
	bool				openAllRefFilesRead();

	/**
	 * Update delta ids for all references
	 */
	bool				updateDeltaIds(uint32 start, uint32 end);




	/**
	 * Allocate a row
	 * \param row is the row index to allocate
	 * Return true if succeded
	 */
	bool				allocate(RY_PDS::TRowIndex row, CAccessor& accessor);

	/**
	 * Deallocate a row
	 * \param row is the row index to deallocate
	 * Return true if succeded
	 */
	bool				deallocate(RY_PDS::TRowIndex row);

	/**
	 * Tells if a row is allocated
	 * \param row is the row to check
	 */
	bool				isAllocated(RY_PDS::TRowIndex row) const		{ return _RowMapper.allocated(row); }

	/**
	 * Get number of actually allocated rows
	 */
	uint32				numAllocated() const							{ return _RowMapper.numAllocated(); }

	/**
	 * Get Max row index used
	 */
	RY_PDS::TRowIndex	maxRowIndex() const								{ return _RowMapper.maxRowIndex(); }


	/**
	 * Get Next Unalloocated row
	 */
	RY_PDS::TRowIndex	nextUnallocatedRow() const						{ return _RowMapper.nextUnallocatedRow(); }


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
	bool				unmapRow(const RY_PDS::CObjectIndex &index, uint64 key);

	/**
	 * Get a mapped row
	 * \param key is the 64 bits row key
	 * Return a valid CObjectIndex if success
	 */
	RY_PDS::CObjectIndex	getMappedRow(uint64 key) const;

	/**
	 * Get number of actually allocated rows
	 */
	uint32				numMapped() const								{ return _RowMapper.numMapped(); }


	/**
	 * Get the number of actually loaded rows
	 */
	uint32				getLoadedRows() const							{ return (uint32)_RowMap.size(); }

	/**
	 * Get memory load
	 */
	uint32				getMemoryLoad() const							{ return (uint32)_RowMap.size()*_InternalRowSize; }



	/**
	 * Build RowMapper
	 */
	bool				buildRowMapper();

	/**
	 * Process Rows, apply external row processing after rows are loaded
	 */
	bool				processRows(IRowProcessor* processor);
	
	/**
	 * Link RowMapper
	 */
	void				linkRowMapper(CTableBuffer* link)				{ _RowMapper.link(&(link->_RowMapper)); }



	/**
	 * Get Size of Row Header (depends on whether table is mapped or not)
	 */
	uint				getHeaderSize()	const							{ return _Mapped ? sizeof(CMappedHeader) : sizeof(CHeader); }

	/**
	 * Get Internal Row Size
	 */
	uint				getInternalRowSize() const						{ return _InternalRowSize; }

	/**
	 * Get User Row Size
	 */
	uint				getRowSize() const								{ return _InternalRowSize; }



	/**
	 * Update a Row
	 * Update in reference file the whole row
	 */
	bool				updateRow(CAccessor accessor);

	/**
	 * Get current Timestamp
	 */
	static uint32		getCommonStamp()								{ return _CommonStamp; }

	/**
	 * Update common Timestamp
	 */
	static void			updateCommonStamp();




	/// \name Debug
	// @{

	/**
	 * Setup debug delta file
	 */
	bool				setupDebugDeltaFile(const std::string& filename, CDBDeltaFile& delta) const;

	/**
	 * Get Delta file Row
	 */
	uint8*				getDeltaRow(uint32& row, CDBDeltaFile& delta) const;

	// @}



protected:

	virtual std::string	getLoggerIdentifier() const						{ return "buffer"; }

private:

	/// Initialised
	bool								_Init;

	/// Table Id
	uint32								_TableId;

	/// Row Size
	uint32								_RowSize;

	/// Internal Row Size
	uint32								_InternalRowSize;

	/// Ref Root Path
	std::string							_RefRootPath;

	/// Ref Path
	std::string							_RefPath;

	/// Rows per ref file
	uint32								_RowsPerFile;

	/// Maximum ref file size
	static uint32						_MaxRefFileSize;

	/// Row Map
	TRowMap								_RowMap;

	/// Map of purgeable rows
	typedef std::set<RY_PDS::TRowIndex>	TReleaseSet;

	/// Release Map
	TReleaseSet							_ReleaseSet;

	/// List of dirty rows
	typedef std::vector<CAccessor>		TDirtyList;

	/// Dirty List
	TDirtyList							_DirtyList;

	/// List of ref files -- a vector or pointer, to avoid realloc issues
	typedef std::vector<CDBReferenceFile*>	TRefFileMap;

	/// Reference files
	TRefFileMap							_RefFileMap;

	/// Index and Allocation Map
	CRowMapper							_RowMapper;

	/// Rows are mapped?
	bool								_Mapped;

	/// Current Delta Id
	uint32								_CurrentDeltaId;


	/// Current reference stamp
	uint32								_ReferenceStamp;

	/// Common stamp
	static uint32						_CommonStamp;


	/// Load a row from the appropriate reference file
	bool				loadRow(RY_PDS::TRowIndex row, TRowData rowData);

	/// Init Number of Rows per File
	bool				initRowsPerFile();

	/// Check reference file is ready
	bool				checkRef(uint32 refFile);

	/// Acquire a row, internal version
	bool				acquireRow(TRowMap::iterator it);

	/// Release a row, internal version
	bool				releaseRow(TRowMap::iterator it, bool forceNotAcquired = false);

	/// Update a Row -- rowdata is _InternalByteSize long, this is a full buffer row with header
	bool				updateRow(RY_PDS::TRowIndex row, const TRowData rowData, bool forceWriteToDisk);

	/// Get Reference Files list
	void				getReferenceFilesList(std::vector<std::string> &result);

	/// Process row for RowMapper
	bool				processRow(CAccessor& accessor);

	/// Header of a row
	class CHeader
	{
	public:

		enum
		{
			Dirty			= 1,
			Allocated		= 2,
		};

		/// Clear
		void	clear()								{ _Flags = 0; _Acquire = 0; _DirtStamp = 0; }

		/// GetFlags
		uint8	getFlags() const					{ return _Flags; }

		/// Is Row Dirty
		bool	dirty() const						{ return checkFlags(Dirty); }


		/// Is Row Allocated
		bool	allocated() const					{ return checkFlags(Allocated); }


		/// Set flags
		void	setFlags(uint8 flags)				{ _Flags |= flags; }

		/// Set flags
		void	clearFlags(uint8 flags)				{ _Flags &= (~flags); }

		/// Check Flags
		bool	checkFlags(uint8 testflags) const	{ return (_Flags & testflags) != 0; }



		/// Set Row As Dirty
		void	setDirty();

		/// Clear DirtStamp (set initial dirt stamp as 'no dirt stamp' value)
		void	clearDirtStamp()					{ _DirtStamp = 0; clearFlags(Dirty); }

		/// Get DirtStamp
		uint32	getDirtStamp() const				{ return _DirtStamp; }


		
		/// Acquire, return true the first time it is acquired
		bool	acquire()
		{
			++_Acquire;
			return _Acquire == 1;
		}

		/// Release, return true if row can be release effectively
		bool	unacquire()
		{
			--_Acquire;
			if (_Acquire < 0)
				_Acquire = 0;
			return !isAcquired();
		}

		/// Clear Acquire count
		void	clearAcquireCount()					{ _Acquire = 0; }

		/// Is acquired
		bool	isAcquired() const					{ return _Acquire > 0; }

	protected:

		/// Row flags
		uint8	_Flags;

		/// Acquire counter
		sint16	_Acquire;

		/// Row Dirty Timestamp
		uint32	_DirtStamp;

	};

	/// Header of a mapped row
	class CMappedHeader : public CHeader
	{
	public:

		/// Clear up header
		void			clear()						{ CHeader::clear(); _Key = 0; }

		/// Set Row Key
		void			setKey(uint64 key)			{ _Key = key; }

		/// Get Row Key
		uint64			getKey() const				{ return _Key; }

		/// Is Row Mapped?
		bool			mapped() const				{ return _Key != 0; }

	protected:

		/**
		 * Row map key
		 * The key of the row in table map, 0 means this row is not mapped
		 */
		uint64				_Key;

	};

	friend class CAccessor;
	friend class CDatabaseAdapter;
};

/**
 * A callback interface for row processing at loading
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class IRowProcessor
{
public:

	virtual bool	processRow(RY_PDS::TTableIndex table, CTableBuffer::CAccessor& accessor) = 0;
};


#include "pds_table_buffer_inline.h"


#endif // NL_PDS_TABLE_BUFFER_H

/* End of pds_table_buffer.h */
