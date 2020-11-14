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

#ifndef NL_DB_MANAGER_H
#define NL_DB_MANAGER_H

//
// NeL includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/log.h>
#include <nel/misc/time_nl.h>

#include <nel/net/message.h>

//
// stl includes
//

#include <map>

//
// PDS includes
//

#include "pds_database.h"
#include "pds_table.h"



//
// basic typedefs
//

typedef uint32		TDatabaseId;
const TDatabaseId	INVALID_DATABASE_ID = 0xffffffff;

extern NLMISC::CVariable<uint>	DeltaUpdateRate;

/**
 * Handle management of multiple databases
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDbManager
{
public:

	/// \name Basic manager implementation
	// @{

	/**
	 * Init manager
	 * Load all previously loaded databases
	 */
	static bool				init();

	/**
	 * Initialised yet?
	 */
	static bool				initialised();

	/**
	 * Update manager
	 */
	static bool				update();

	/**
	 * Release manager
	 */
	static bool				release();

	// @}




	/// \name User Database manipulation
	// @{

	/**
	 * Remap ServiceId to DatabaseId
	 */
	static TDatabaseId		getDatabaseId(NLNET::TServiceId serviceId)	
	{ 
		if (serviceId.get() > 256) 
			return INVALID_DATABASE_ID; 
		return _ServiceMap[serviceId.get()];
	}

	/**
	 * Create a database entry
	 */
	static CDatabase*		createDatabase(TDatabaseId id, NLMISC::CLog* log = NLMISC::InfoLog);

	/**
	 * Delete a database entry
	 */
	static bool				deleteDatabase(TDatabaseId id, NLMISC::CLog* log = NLMISC::InfoLog);



	/**
	 * Load a database and adapt to the description if needed
	 */
	static CDatabase*		loadDatabase(TDatabaseId id, const std::string& description, NLMISC::CLog* log = NLMISC::InfoLog);



	/**
	 * load a database
	 */
	static bool				loadDatabase(TDatabaseId id, NLMISC::CLog* log = NLMISC::InfoLog);

	/**
	 * load a database entry
	 */
	static CDatabase*		getDatabase(TDatabaseId id);

	/**
	 * Allocate a row in a database
	 * \param id is the database id to allocate row into
	 * \param table is the specified table
	 * \param row is the specified row in table
	 */
	static bool				allocRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row);

	/**
	 * Deallocate a row in a database
	 * \param id is the database id to deallocate row into
	 * \param table is the specified table
	 * \param row is the specified row in table
	 */
	static bool				deallocRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row);

	/**
	 * Map a row in a table
	 * \param index is the table/row to allocate
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	static bool				mapRow(TDatabaseId id, const RY_PDS::CObjectIndex &index, uint64 key);

	/**
	 * Unmap a row in a table
	 * \param tableIndex is the table to find row
	 * \param key is the 64 bits row key
	 * Return true if succeded
	 */
	static bool				unmapRow(TDatabaseId id, RY_PDS::TTableIndex tableIndex, uint64 key);

	/**
	 * Release a row in a database
	 * \param id is the database id to release row
	 * \param table is the specified table
	 * \param row is the specified row in table
	 */
	static bool				releaseRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row);

	/**
	 * Set an item in database, located by its table, row and column.
	 * \param datasize is provided for validation check (1, 2, 4 or 8 bytes)
	 * \param dataptr points to raw data, which may be 1, 2, 4 or 8 bytes, as indicated by datasize 
	 */
	static bool				set(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr);

	/**
	 * Fetch data
	 */
	static bool				fetch(TDatabaseId id, RY_PDS::TTableIndex tableIndex, uint64 key, RY_PDS::CPData &data);


	/**
	 * Add String in Database' string manager
	 */
	//static bool				addString(TDatabaseId id, const NLMISC::CEntityId& eId, RY_PDS::CPDStringManager::TEntryId pdId, const ucstring& str);

	// @}




	/// \name System Database manipulation
	// @{

	/**
	 * Delete all database entries
	 */
	static bool				deleteAllDatabases(NLMISC::CLog* log = NLMISC::InfoLog);

	// @}




	/**
	 * Parse path into TLocatePath
	 */
	static bool				parsePath(const std::string &strPath, CLocatePath &path);

	/**
	 * Locate a column using a path
	 */
	static CTable::CDataAccessor	locate(CLocatePath &path);



	/**
	 * Map Service Id
	 */
	static bool				mapService(NLNET::TServiceId serviceId, TDatabaseId databaseId);

	/**
	 * Unmap Service Id
	 */
	static bool				unmapService(NLNET::TServiceId serviceId);



	/// \name RBS Connection Management
	// @{

	/**
	 * Add RBS Task
	 * TaskId is automatically added to the message to be sent
	 */
	static NLNET::CMessage&	addTask(const std::string& msg, ITaskEventListener* listener, void* arg);

	/**
	 * Notify RBS task success report
	 */
	static void				notifyRBSSuccess(uint32 taskId);

	/**
	 * Notify RBS task failure report
	 */
	static void				notifyRBSFailure(uint32 taskId);

	/**
	 * Set RBS Up
	 */
	static void				RBSUp()
	{
		_RBSUp = true;
	}

	/**
	 * Set RBS Down
	 */
	static void				RBSDown()
	{
		_RBSUp = false;
		nlwarning("CDbManager::RBSDown(): RBS down, please call maintenance. Data corruption may appear.");
	}

	/**
	 * Get Next RBS Task Id
	 */
	static uint32			nextTaskId()			{ return _TaskId++; }

	// @}


private:

	/// Is manager initialised
	static bool										_Initialised;


	/// Type Map of database
	typedef std::map<TDatabaseId, CDatabase*>		TDatabaseMap;

	/// Map of database
	static TDatabaseMap								_DatabaseMap;

	/// Map from service to database
	typedef TDatabaseId								TServiceMap[256];

	/// Map of services
	static TServiceMap								_ServiceMap;

	/// Next time to build delta
	static NLMISC::TTime							_NextTimeDelta;

	/// Last Update timestamp
	static CTimestamp								_LastUpdateTime;

	/// Next task
	static uint32									_TaskId;

	/// Messages to send to RBS
	static std::deque<NLNET::CMessage*>				_RBSMessages;

	/// Acknowledge to wake
	static std::map<uint32, std::pair<ITaskEventListener*, void*> >	_TaskListeners;

	/// RBS state
	static bool										_RBSUp;



	/// Private constructor, class is singleton
	CDbManager();

};


#endif // NL_DB_MANAGER_H

/* End of db_manager.h */
