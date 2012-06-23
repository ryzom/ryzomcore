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

#ifndef RY_PDS_DATABASE_H
#define RY_PDS_DATABASE_H

//
// NeL includes
//
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/i_xml.h>

//
// PDS includes
//
#include "../pd_lib/pds_types.h"
#include "../pd_lib/pds_common.h"
#include "../pd_lib/db_description_parser.h"

#include "../pd_lib/pd_utils.h"
#include "../pd_lib/pd_server_utils.h"
//#include "../pd_lib/pd_string_manager.h"
#include "../pd_lib/pd_string_mapper.h"

//
// stl includes
//
#include <vector>
#include <deque>

class CType;
class CTable;
class CAttribute;
class CColumn;

namespace RY_PDS
{
	class CDbMessageQueue;
};

/**
 * Definition of the database
 * This class contains tables and types definitions
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CDatabase : public CPDSLogger, public ITaskEventListener, public RY_PDS::ITableContainer
{
public:

	/**
	 * Constructor
	 */
	CDatabase(uint32 id);


	/**
	 * Destructor
	 */
	~CDatabase();


	/**
	 * Massive Database clear
	 */
	void					clear();


	/**
	 * Init database
	 */
	bool					init();

	/**
	 * Map to a service
	 */
	void					mapToService(NLNET::TServiceId serviceId)	{ _ServiceId = serviceId; }

	/**
	 * Get Mapped Service Id
	 */
	NLNET::TServiceId		getMappedService() const		{ return _ServiceId; }


	/// Initialized yet?
	bool					initialised() const			{ return _Init; }

	/**
	 * Adapt database to new description
	 * \param description is the latest xml description of the database
	 * \returns a pointer the valid database, or NULL if failed
	 */
	CDatabase*				adapt(const std::string& description);

	/**
	 * Initialise internal timestamps
	 */
	void					initTimestamps();


	/**
	 * Checkup database
	 */
	bool					checkup();


	/**
	 * Get Name
	 */
	const std::string&		getName() const				{ return _State.Name; }



	/**
	 * Get Type
	 */
	const CType*			getType(TTypeId typeId) const;

	/**
	 * Get Type
	 */
	const CType*			getType(const std::string &name) const;



	/**
	 * Get Table
	 */
	const CTable*			getTable(TTypeId tableId) const;

	/**
	 * Get Table
	 */
	const CTable*			getTable(const std::string &name) const;



	/**
	 * Get Attribute
	 */
	const CAttribute*		getAttribute(uint32 tableId, uint32 attributeId) const;

	/**
	 * Get Column
	 */
	const CColumn*			getColumn(uint32 tableId, uint32 columnId) const;




	/**
	 * Get value as a string
	 * \param path is of the form 'Table[index|key].attrib1.attrib2'
	 */
	std::string				getValue(const CLocatePath::TLocatePath &path);



	/**
	 * Allocate a row in a table
	 * \param index is the table/row to allocate
	 * Return true if succeded
	 */
	bool					allocate(const RY_PDS::CObjectIndex &index);

	/**
	 * Deallocate a row in a table
	 * \param index is the table/row to deallocate
	 * Return true if succeded
	 */
	bool					deallocate(const RY_PDS::CObjectIndex &index);

	/**
	 * Map a row in a table
	 * \param index is the table/row to allocate
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	bool					mapRow(const RY_PDS::CObjectIndex &index, uint64 key);

	/**
	 * Unmap a row in a table
	 * \param tableIndex is the table to find row
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	bool					unmapRow(RY_PDS::TTableIndex tableIndex, uint64 key);

	/**
	 * Get a mapped row
	 * \param tableIndex is the table in which the row is mapped
	 * \param key is the 64 bits row key
	 * Return a valid CObjectIndex if success
	 */
	RY_PDS::CObjectIndex	getMappedRow(RY_PDS::TTableIndex tableIndex, uint64 key) const;

	/**
	 * Search object in database using its key
	 * \param key is the 64 bits row key to search through all tables
	 * Return true if key matches at lease one object
	 */
	bool					searchObjectIndex(uint64 key, std::set<RY_PDS::CObjectIndex>& indexes) const;

	/**
	 * Release a row in a table
	 * \param index is the table/row to release
	 * Return true if succeded
	 */
	bool					release(const RY_PDS::CObjectIndex &index);

	/**
	 * Release all rows in all table
	 * Typically, the client disconnected, there is no need to keep rows
	 */
	bool					releaseAll();

	/**
	 * Tells if an object is allocated
	 * \param object is the object index to test
	 */
	bool					isAllocated(const RY_PDS::CObjectIndex &index) const;



	/**
	 * Set an item in database, located by its table, row and column.
	 * \param datasize is provided for validation check (1, 2, 4 or 8 bytes)
	 * \param dataptr points to raw data, which may be 1, 2, 4 or 8 bytes, as indicated by datasize 
	 */
	bool					set(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr);

	/**
	 * Set an object parent
	 */
	bool					setParent(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const RY_PDS::CObjectIndex &parent);


	/**
	 * Set an item from database, located by its table, row and column.
	 * \param dataptr points to raw data, which may be 1, 2, 4 or 8 bytes long, as indicated by datasize, where to store data
	 * \param datasize is the size of the dataptr buffer. At return, it is the size of actual data stored in buffer, that may have been
	 * truncated, if possible
	 * \param type is the TDataType of data stored at dataptr
	 */
	bool					get(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint& datasize, void* dataptr, TDataType& type);





	/**
	 * Get Table (non const)
	 */
	CTable*					getNonConstTable(RY_PDS::TTableIndex table);

	/**
	 * Get Object list
	 */
	RY_PDS::CSetMap&		getSetMap()			{ return _SetMap; }

	/**
	 * Get Object list
	 */
	const RY_PDS::CSetMap&	getSetMap()	const	{ return _SetMap; }

	/**
	 * Get String Manager
	 */
//	RY_PDS::CPDStringManager&	getStringManager()	{ return _StringManager; }

	/**
	 * Fetch data
	 */
	bool					fetch(const RY_PDS::CObjectIndex& index, RY_PDS::CPData &data, bool fetchIndex = true);




	/**
	 * Display database
	 */
	void					display(NLMISC::CLog* log = NLMISC::InfoLog, bool displayHeader = false) const;

	/**
	 * Set value with human readable parameters
	 */
	bool					set(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const std::string& type, const std::string &value);




	/**
	 * Dump database content and info of an object to xml
	 */
	void					dumpToXml(const RY_PDS::CObjectIndex& index, NLMISC::IStream& xml, sint expandDepth = -1);





	/**
	 * Get Update Queue for id
	 * May return NULL if message was already received
	 */
	RY_PDS::CDbMessageQueue*	getUpdateMessageQueue(uint32 updateId);

	/**
	 * Receive update
	 */
	void					receiveUpdate(uint32 id);

	/**
	 * Get Last Update Id
	 */
	uint32					getLastUpdateId() const			{ return _State.LastUpdateId; }

	/**
	 * Flush updates
	 */
	void					flushUpdates(std::vector<uint32>& acknowledged);



	/**
	 * Serialise SheetId String Mapper
	 */
	void					serialSheetIdStringMapper(NLMISC::IStream& f);


private:

	/// Initialised yet?
	bool						_Init;

	/// Current Database State
	CDatabaseState				_State;

	/// Service Id mapped
	NLNET::TServiceId			_ServiceId;

	/// Description
	CDBDescriptionParser		_Description;

	/// Reference index
	CRefIndex					_Reference;

	/// Types in database
	std::vector<CType*>			_Types;

	/// Tables in database
	std::vector<CTable*>		_Tables;

	/// Common object list
	RY_PDS::CSetMap				_SetMap;

	/// String manager
//	RY_PDS::CPDStringManager	_StringManager;

	/// Last Minute Update Timestamp
	CTimestamp					_MinuteUpdateTimestamp;

	/// Last Minute Update Timestamp
	CTimestamp					_HourUpdateTimestamp;

	/// Last Minute Update Timestamp
	CTimestamp					_DayUpdateTimestamp;

	/// Creation Timestamp
	CTimestamp					_CreationTimestamp;

	/// Received updates
	std::vector<uint32>			_ReceivedUpdates;

	/// SheetId String Mapper
	CPDStringMapper				_SheetIdStringMapper;



	/// FIFO of logs
	typedef std::list<RY_PDS::CUpdateLog>	TUpdateLogQueue;

	/// Log of database updates
	TUpdateLogQueue				_LogQueue;


public:

	/// \name RBS Task Event listener interface
	// @{

	/// Task ran successfully
	virtual void			taskSuccessful(void* arg);

	/// Task failed!
	virtual void			taskFailed(void* arg);

	// @}

	/// \name ITableContainer interface
	// @{

	/// Get Table Index from name
	virtual RY_PDS::TTableIndex	getTableIndex(const std::string& tableName) const;

	/// Get Table Index from name
	virtual std::string			getTableName(RY_PDS::TTableIndex index) const;

	// @}

protected:

	virtual std::string	getLoggerIdentifier() const	{ return NLMISC::toString("db:%s", (_State.Name.empty() ? "<unnamed>" : _State.Name.c_str())); }

public:




	/**
	 * Load previous database state and create a new temporary reference if needed (after a crash, for instance)
	 */
	bool					loadState();


	/**
	 * Check if reference is still the same
	 */
	bool					checkReferenceChange();


	/**
	 * Create new database from scratch, setup everything needed (references, etc.)
	 * \param description is the xml database description
	 */
	bool					createFromScratch(const std::string& description);


	/**
	 * Check if reference is up to date
	 * Returns true if reference is the latest valid database image
	 */
	bool					isReferenceUpToDate();


	/**
	 * Build a up to date reference in a temp directory.
	 * Setup 'ref' file, so that it points to new reference.
	 */
	bool					buildReference();

	/**
	 * Build the delta files and purge all dirty rows in tables
	 */
	bool					buildDelta(const CTimestamp& starttime, const CTimestamp& endtime);

	/**
	 * Flush database from released rows
	 */
	bool					flushReleased();

	/**
	 * Notify a new reference is ready
	 */
	bool					notifyNewReference(bool validateRef = true);



	/**
	 * Build index allocators
	 * One per table
	 */
	bool					buildIndexAllocators(std::vector<RY_PDS::CIndexAllocator> &allocators);

	/**
	 * Rebuild forwardrefs from backrefs
	 */
	bool					rebuildForwardRefs();

	/**
	 * Rebuild table maps
	 */
	bool					rebuildTableMaps();

	/**
	 * Reset dirty lists
	 */
	bool					resetDirtyTags();

	/**
	 * Reset and rebuild all maps, references, lists...
	 */
	bool					rebuildVolatileData();



	/**
	 * Static Check for update rate
	 */
	static void				checkUpdateRates();

	/**
	 * Send Delta/Reference build commands
	 */
	bool					sendBuildCommands(const CTimestamp& current);


private:

	friend class CDatabaseAdapter;
};


// include inlines
#include "pds_database_inline.h"

#endif //RY_PDS_DATABASE_H

