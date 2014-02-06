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

#include "db_manager.h"
#include "pds_table.h"
#include "db_manager_messages.h"

#include <nel/misc/command.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/path.h>
#include <nel/misc/hierarchical_timer.h>

#include <nel/net/service.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;




#define CHECK_DB_MGR_INIT(function, returnvalue)	\
if (!initialised())	\
{	\
	nlwarning("CDbManager not initialised, " #function "() forbidden");	\
	return returnvalue;	\
}


/*
 * Initialised yet?
 */
bool	CDbManager::initialised()
{
	return _Initialised;
}

CVariable<uint>	DeltaUpdateRate("pds", "DeltaUpdateRate", "Number of seconds between two delta updates", 10, 0, true);

/*
 * Update manager
 */
bool	CDbManager::update()
{
	H_AUTO(PDS_DbManager_update);

	CHECK_DB_MGR_INIT(update, false);

	// update stamp
	CTableBuffer::updateCommonStamp();

	TDatabaseMap::iterator	it;

	CDatabase::checkUpdateRates();


	// check evently if database need to write some delta
	TTime	tm = CTime::getLocalTime();
	if (tm >= _NextTimeDelta)
	{

		CTimestamp	starttime = _LastUpdateTime;
		CTimestamp	endtime;
		endtime.setToCurrent();

		std::vector<uint32>	ack;

		for (it=_DatabaseMap.begin(); it!=_DatabaseMap.end(); ++it)
		{
			CDatabase*	database = (*it).second;

			// generate deltas
			if (!database->buildDelta(starttime, endtime))
				nlwarning("failed to build delta for database '%d' '%s'", (*it).first, database->getName().c_str());

			// obsolete? since RBS build references and tells PDS of success/failure
			database->checkReferenceChange();

			// acknowledge last updates
			database->flushUpdates(ack);

			if (!ack.empty() && database->getMappedService().get() != 0xffff)
			{
				CMessage	msgack("PD_ACK_UPD");
				uint32		databaseId = (*it).first;
				msgack.serial(databaseId);
				msgack.serialCont(ack);
				CUnifiedNetwork::getInstance()->send(database->getMappedService(), msgack);
			}
		}

		_NextTimeDelta = tm - (tm%(DeltaUpdateRate*1000)) + (DeltaUpdateRate*1000);

		_LastUpdateTime = endtime;
	}

	CTimestamp	ts;
	ts.setToCurrent();

	// check databases require some delta packing/reference generation
	for (it=_DatabaseMap.begin(); it!=_DatabaseMap.end(); ++it)
	{
		CDatabase*	database = (*it).second;
		database->sendBuildCommands(ts);
	}

	// send messages to RBS if ready
	while (_RBSUp && !_RBSMessages.empty())
	{
		CUnifiedNetwork::getInstance()->send("RBS", *(_RBSMessages.front()));
		delete _RBSMessages.front();
		_RBSMessages.pop_front();
	}

	return true;
}

/*
 * Release manager
 */
bool	CDbManager::release()
{
	CHECK_DB_MGR_INIT(release, false);

	// release all databases
	deleteAllDatabases();

	return true;
}


// Is manager initialised
bool						CDbManager::_Initialised = false;

// Map of database
CDbManager::TDatabaseMap	CDbManager::_DatabaseMap;

// Map of services
CDbManager::TServiceMap		CDbManager::_ServiceMap;

// Next time to build delta
TTime						CDbManager::_NextTimeDelta;

// Next task
uint32						CDbManager::_TaskId = 0;

// Messages to send to RBS
std::deque<NLNET::CMessage*>	CDbManager::_RBSMessages;

// Acknowledge to wake
std::map<uint32, std::pair<ITaskEventListener*, void*> >	CDbManager::_TaskListeners;

// RBS state
bool						CDbManager::_RBSUp = false;

// Last Update timestamp
CTimestamp					CDbManager::_LastUpdateTime;


/*
 * Create a database entry
 */
CDatabase*	CDbManager::createDatabase(TDatabaseId id, CLog* log)
{
	CHECK_DB_MGR_INIT(createDatabase, NULL);

	// check db doesn't exist yet
	CDatabase*	db = getDatabase(id);
	if (db != NULL)
	{
		log->displayNL("Unable to createDatabase() %d, already exists as '%s'", id, db->getName().c_str());
		return NULL;
	}

	// create database and map it
	db = new CDatabase(id);
	_DatabaseMap[id] = db;

	return db;
}

/*
 * Delete a database entry
 */
bool	CDbManager::deleteDatabase(TDatabaseId id, CLog* log)
{
	CHECK_DB_MGR_INIT(deleteDatabase, false);

	// check db exists
	TDatabaseMap::iterator	it = _DatabaseMap.find(id);
	if (it == _DatabaseMap.end())
	{
		log->displayNL("Unable to deleteDatabase() %d, not create yet", id);
		return false;
	}

	// get database
	CDatabase*	db = (*it).second;

	// unmap it
	(*it).second = NULL;
	_DatabaseMap.erase(it);

	// delete it
	delete db;

	return true;
}




/*
 * Load a database and adapt to the description if needed
 */
CDatabase*	CDbManager::loadDatabase(TDatabaseId id, const string& description, CLog* log)
{
	CHECK_DB_MGR_INIT(loadDatabase, NULL);

	nlinfo("CDbManager::loadDatabase(): load/setup database '%d'", id);

	CDatabase*	db = getDatabase(id);

	// database not loaded yet?
	if (db == NULL)
	{
		// create a memory image
		db = createDatabase(id, log);
		if (db == NULL)
		{
			log->displayNL("failed to create database '%d'", id);
			return NULL;
		}

		// if can't load database
		if (!db->loadState())
		{
			nlinfo("CDbManager::loadDatabase(): database '%d' doesn't exist, create new", id);

			// create a new database with the new description
			if (!db->createFromScratch(description))
			{
				log->displayNL("failed to create database '%d' from scratch", id);
				return NULL;
			}

			return db;
		}
	}

	CDatabase*	adapted = db->adapt(description);
	if (adapted ==  NULL)
	{
		log->displayNL("failed to adapt database '%s' to new description", db->getName().c_str());
		return NULL;
	}

	// database changed?
	if (db != adapted)
	{
		// replace old on with new one
		_DatabaseMap[id] = adapted;
		// and delete old
		delete db;
	}

	return adapted;
}



/*
 * load a database
 */
bool	CDbManager::loadDatabase(TDatabaseId id, CLog* log)
{
	CHECK_DB_MGR_INIT(loadDatabase, false);

	// check db doesn't exist yet
	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		log->displayNL("Unable to loadDatabase() %d, not created yet", id);
		return false;
	}

	// check database not init'ed
	if (db->initialised())
	{
		log->displayNL("Unable to loadDatabase() %d, already initialised as '%s'", id, db->getName().c_str());
		return false;
	}

	return db->loadState();
}



/*
 * get a database entry
 */
CDatabase*	CDbManager::getDatabase(TDatabaseId id)
{
	CHECK_DB_MGR_INIT(getDatabase, NULL);

	TDatabaseMap::iterator	it = _DatabaseMap.find(id);
	return (it == _DatabaseMap.end() ? NULL : (*it).second);
}



/*
 * Set an item in database, located by its table, row and column.
 * \param datasize is provided for validation check (1, 2, 4 or 8 bytes)
 * \param dataptr points to raw data, which may be 1, 2, 4 or 8 bytes, as indicated by datasize 
 */
bool	CDbManager::set(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr)
{
	CHECK_DB_MGR_INIT(set, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to set() value in %d, not created yet", id);
		return false;
	}

	return db->set(table, row, column, datasize, dataptr);
}

/*
 * Allocate a row in a database
 * \param id is the database id to allocate row into
 * \param table is the specified table
 * \param row is the specified row in table
 */
bool	CDbManager::allocRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row)
{
	CHECK_DB_MGR_INIT(allocRow, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to allocRow() '%d' in table '%d' in database '%d', not created yet", row, table, id);
		return false;
	}

	return db->allocate(RY_PDS::CObjectIndex(table, row));
}

/*
 * Deallocate a row in a database
 * \param id is the database id to deallocate row into
 * \param table is the specified table
 * \param row is the specified row in table
 */
bool	CDbManager::deallocRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row)
{
	CHECK_DB_MGR_INIT(deallocRow, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to deallocRow() '%d' in table '%d' in database '%d', not created yet", row, table, id);
		return false;
	}

	return db->deallocate(RY_PDS::CObjectIndex(table, row));
}

/*
 * Map a row in a table
 * \param index is the table/row to allocate
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CDbManager::mapRow(TDatabaseId id, const RY_PDS::CObjectIndex &index, uint64 key)
{
	CHECK_DB_MGR_INIT(mapRow, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to mapRow() '%016"NL_I64"X' to row '%d':'%d' in db '%d' , not created yet", key, index.table(), index.row(), id);
		return false;
	}

	return db->mapRow(index, key);
}

/*
 * Unmap a row in a table
 * \param tableIndex is the table to find row
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CDbManager::unmapRow(TDatabaseId id, RY_PDS::TTableIndex tableIndex, uint64 key)
{
	CHECK_DB_MGR_INIT(unmapRow, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to unmapRow() '%016"NL_I64"X' in '%d':'%d' in db '%d' , not created yet", key, tableIndex, id);
		return false;
	}

	return db->unmapRow(tableIndex, key);
}

/*
 * Release a row in a database
 * \param id is the database id to release row into
 * \param table is the specified table
 * \param row is the specified row in table
 */
bool	CDbManager::releaseRow(TDatabaseId id, RY_PDS::TTableIndex table, RY_PDS::TRowIndex row)
{
	CHECK_DB_MGR_INIT(releaseRow, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to releaseRow() '%d' in table '%d' in database '%d', not created yet", row, table, id);
		return false;
	}

	return db->release(RY_PDS::CObjectIndex(table, row));
}

/*
 * Fetch data
 */
bool	CDbManager::fetch(TDatabaseId id, RY_PDS::TTableIndex tableIndex, uint64 key, RY_PDS::CPData &data)
{
	CHECK_DB_MGR_INIT(fetch, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to fetch(), db '%d' not created yet", id);
		return false;
	}

	RY_PDS::CObjectIndex	index = db->getMappedRow(tableIndex, key);
	if (!index.isValid())
	{
		// row is not mapped
		return false;
	}

	return db->fetch(index, data);
}



/*
 * Add String in Database' string manager
 */
/*
bool	CDbManager::addString(TDatabaseId id, const NLMISC::CEntityId& eId, RY_PDS::CPDStringManager::TEntryId pdId, const ucstring& str)
{
	CHECK_DB_MGR_INIT(addString, false);

	CDatabase*	db = getDatabase(id);
	if (db == NULL)
	{
		nlwarning("Unable to addString(), db '%d' not created yet", id);
		return false;
	}

	RY_PDS::CPDStringManager&	sm = db->getStringManager();

	return sm.setString(eId, pdId, str);
}
*/





/*
 * Delete all database entries
 */
bool	CDbManager::deleteAllDatabases(CLog* log)
{
	CHECK_DB_MGR_INIT(getDatabase, false);

	CTimestamp	starttime = _LastUpdateTime;
	CTimestamp	endtime;
	endtime.setToCurrent();

	TDatabaseMap::iterator	it;
	for (it=_DatabaseMap.begin(); it!=_DatabaseMap.end(); ++it)
	{
		CDatabase*	db = (*it).second;

		if (db == NULL)
		{
			log->displayNL("Database '%d' left with as NULL", (*it).first);
		}
		else
		{
			// flush db
			if (!db->buildDelta(starttime, endtime))
				nlwarning("failed to build delta for database '%d' '%s'", (*it).first, db->getName().c_str());

			// delete it
			delete db;
		}

		// unreference it
		(*it).second = NULL;
	}

	_DatabaseMap.clear();

	return true;
}







/*
 * Parse path into TLocatePath
 */
bool	CDbManager::parsePath(const string &strPath, CLocatePath &lpath)
{
	CLocatePath::TLocatePath	&path = lpath.FullPath;

	lpath.Pos = 0;
	path.clear();

	if (strPath.empty())
	{
		nlwarning("CDbManager::parsePath(): empty path");
		return false;
	}

	// explode path into nodes formed like 'a_name' or 'an_array[a_key]' or 'a_set<a_key>'

	vector<string>	nodes;
	explode(strPath, string("."), nodes, false);

	uint	i;
	for (i=0; i<nodes.size(); ++i)
	{
		string								&node = nodes[i];
		CLocatePath::CLocateAttributeNode	anode;

		if (node.empty())
			return false;

		anode.Set = false;
		anode.Array = false;

		string::size_type pos = node.find_first_of("[<");

		if (pos != string::npos)
		{
			if (node[pos] == '[')
				anode.Array = true;
			else
				anode.Set = true;

			anode.Name = node.substr(0, pos);

			string::size_type end = node.find((anode.Array ? ']' : '>'), pos);
			if (end == string::npos)
				return false;

			anode.Key = node.substr(pos+1, end-pos-1);
		}
		else
		{
			anode.Name = node;
		}

		path.push_back(anode);
	}

	return true;
}


/*
 * Locate a column using a path
 */
CTable::CDataAccessor	CDbManager::locate(CLocatePath &path)
{
	CHECK_DB_MGR_INIT(getDatabase, CTable::CDataAccessor());

	if (path.end())
		return CTable::CDataAccessor();

	TDatabaseId		id;
	NLMISC::fromString(path.node().Name, id);
	if (!path.next())
		return CTable::CDataAccessor();

	CDatabase*		db = getDatabase(id);
	if (db == NULL)
		return CTable::CDataAccessor();

	CTable*	table = const_cast<CTable*>(db->getTable(path.node().Name));
	if (table == NULL)
		return CTable::CDataAccessor();

	path.next();

	return table->getAccessor(path);
}






/*
 * Constructor
 */
CDbManager::CDbManager()
{
}



/*
 * Init manager
 */
bool	CDbManager::init()
{
	nlinfo("CDbManager::init(): initialise database engine");

	// initial type checking
	RY_PDS::CPDSLib::checkInternalTypes();

	initDbManagerMessages();

	uint	i;
	for (i=0; i<256; ++i)
		_ServiceMap[i] = INVALID_DATABASE_ID;

	string	rootPath = RY_PDS::CPDSLib::getPDSRootDirectory();

	if (!CFile::isDirectory(rootPath))
	{
		if (!CFile::createDirectoryTree(rootPath))
		{
			nlwarning("CDbManager::init(): failure, can't create root path '%s', can't start.", rootPath.c_str());
			return false;
		}

		if (!CFile::setRWAccess(rootPath))
		{
			nlwarning("CDbManager::init(): failure, can't set RW access to path '%s', can't start.", rootPath.c_str());
			return false;
		}
	}

	_Initialised = true;

	vector<string>	databases;
	NLMISC::CPath::getPathContent(rootPath, false, true, false, databases);

	for (i=0; i<databases.size(); ++i)
	{
		string&	db = databases[i];
		// init database in directory db

		bool	inited = false;

		nldebug("CDbManager::init(): found directory '%s' in root database path, try to load database", db.c_str());

		try
		{
			if (CDatabaseState::exists(db))
			{
				TTime	starttime = CTime::getLocalTime();

				CDatabaseState	state;
				if (state.load(db) && createDatabase(state.Id) && loadDatabase(state.Id))
				{
					TTime	totaltime = CTime::getLocalTime()-starttime;
					nlinfo("CDbManager::init(): database '%d' initialised in %d ms", state.Id, (uint32)totaltime);
					inited = true;
				}
				else
				{
					nlwarning("CDbManager::init(): failed to initialise database '%d', state file in directory '%s' may be corrupted ", state.Id, db.c_str());
					deleteDatabase(state.Id);
				}
			}
		}
		catch (const Exception&)
		{
		}

		if (!inited)
		{
			nlwarning("CDbManager::init(): couldn't init database in directory '%s', database is skipped.", db.c_str());
		}
	}

	nlinfo("CDbManager::init(): engine initialised successfully");

	_LastUpdateTime.setToCurrent();

	return true;
}



/*
 * Map Service Id
 */
bool	CDbManager::mapService(TServiceId serviceId, TDatabaseId databaseId)
{
	if (serviceId.get() > 256 || _ServiceMap[serviceId.get()] != INVALID_DATABASE_ID)
	{
		nlwarning("CDbManager::mapService(): failed, serviceId '%hu' not valid or service already mapped", serviceId.get());
		return false;
	}

	_ServiceMap[serviceId.get()] = databaseId;

	CDatabase*	database = getDatabase(databaseId);
	if (database != NULL)
		database->mapToService(serviceId);

	return true;
}

/*
 * Unmap Service Id
 */
bool	CDbManager::unmapService(NLNET::TServiceId serviceId)
{
	if (serviceId.get() > 256 || _ServiceMap[serviceId.get()] == INVALID_DATABASE_ID)
		return false;

	TDatabaseId	id = _ServiceMap[serviceId.get()];
	_ServiceMap[serviceId.get()] = INVALID_DATABASE_ID;

	CDatabase*	db = getDatabase(id);

	if (db != NULL)
	{
		db->mapToService(TServiceId(0xffff));
		db->releaseAll();
	}

	return true;
}



/*
 * Add RBS Task
 */
NLNET::CMessage&	CDbManager::addTask(const std::string& msg, ITaskEventListener* listener, void* arg)
{
	NLNET::CMessage*	msgrbs = new NLNET::CMessage(msg);
	_RBSMessages.push_back(msgrbs);

	uint32	id = nextTaskId();
	msgrbs->serial(id);

	// add listener to task listeners
	if (listener != NULL)
		_TaskListeners[id] = std::make_pair<ITaskEventListener*, void*>(listener, arg);

	return *msgrbs;
}


/*
 * Notify RBS task success report
 */
void	CDbManager::notifyRBSSuccess(uint32 taskId)
{
	std::map<uint32, std::pair<ITaskEventListener*, void*> >::iterator	it = _TaskListeners.find(taskId);
	if (it == _TaskListeners.end())
		return;

	// call listener success method
	ITaskEventListener*	listener = (*it).second.first;
	void*				arg = (*it).second.second;

	listener->taskSuccessful(arg);

	// and remove task
	_TaskListeners.erase(it);
}

/*
 * Notify RBS task failure report
 */
void	CDbManager::notifyRBSFailure(uint32 taskId)
{
	std::map<uint32, std::pair<ITaskEventListener*, void*> >::iterator	it = _TaskListeners.find(taskId);
	if (it == _TaskListeners.end())
		return;

	// call listener failure method
	ITaskEventListener*	listener = (*it).second.first;
	void*				arg = (*it).second.second;

	listener->taskFailed(arg);

	// and remove task
	_TaskListeners.erase(it);
}












/*
 * Utility commands
 */





//
NLMISC_COMMAND(createDatabase, "create a database using a given id", "<databaseId>")
{
	if (args.size() != 1)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);
	return CDbManager::createDatabase(databaseId, &log) != NULL;
}

//
NLMISC_COMMAND(deleteDatabase, "delete a database using a given id", "<databaseId>")
{
	if (args.size() != 1)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);
	return CDbManager::deleteDatabase(databaseId, &log);
}

//
NLMISC_COMMAND(loadDatabase, "load a database using a given id", "<databaseId>")
{
	if (args.size() != 1)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);
	return CDbManager::loadDatabase(databaseId, &log);
}

//
NLMISC_COMMAND(displayDatabase, "display database info", "<databaseId>")
{
	if (args.size() != 1)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	CDatabase *database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
		return false;

	database->display(&log);

	return true;
}

//
NLMISC_COMMAND(displayTable, "display table info", "<databaseId> <tableName>")
{
	if (args.size() != 2)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&	tableName = args[1];

	CDatabase*			database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);
	if (table == NULL)
		return false;

	table->display(&log, true, true);

	return true;
}

//
NLMISC_COMMAND(dumpDeltaFileContent, "duump the content of a delta file", "<databaseId> <tableName> <filename>")
{
	if (args.size() != 3)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&	tableName = args[1];

	CDatabase*			database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);
	if (table == NULL)
		return false;

	table->dumpDeltaFileContent(args[2], &log);

	return true;
}

//
NLMISC_COMMAND(displayRow, "display row values", "<databaseId> [<tableName> <rowId> | <CObjectIndex>]")
{
	if (args.size() != 2 && args.size() != 3)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	CDatabase*			database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
		return false;

	CTable*		table = NULL;
	RY_PDS::TRowIndex	rowId;

	if (args.size() == 3)
	{
		const CTable*	ctable = database->getTable(args[1]);
		if (ctable != NULL)
		{
			RY_PDS::TTableIndex	tableIndex = (RY_PDS::TTableIndex)ctable->getId();
			table = database->getNonConstTable(tableIndex);
			NLMISC::fromString(args[2], rowId);
		}
	}
	else
	{
		RY_PDS::CObjectIndex	index;
		index.fromString(args[1].c_str());

		table = database->getNonConstTable(index.table());
		rowId = index.row();
	}

	if (table == NULL)
		return false;

	table->displayRow(rowId, &log, true);

	return true;
}

//
NLMISC_COMMAND(allocRow, "allocate a row in a table of a given database", "<databaseId> <tableName> <rowId>")
{
	if (args.size() != 3)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&	tableName = args[1];
	RY_PDS::TRowIndex	rowId;
	NLMISC::fromString(args[2], rowId);

	CDatabase*			database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);
	if (table == NULL)
		return false;

	return database->allocate(RY_PDS::CObjectIndex((RY_PDS::TTableIndex)table->getId(), rowId));
}

//
NLMISC_COMMAND(deallocRow, "deallocate a row in a table of a given database", "<databaseId> <tableName> <rowId>")
{
	if (args.size() != 3)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&	tableName = args[1];
	RY_PDS::TRowIndex	rowId;
	NLMISC::fromString(args[2], rowId);

	CDatabase*			database = CDbManager::getDatabase(databaseId);

	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);

	if (table == NULL)
		return false;

	return database->deallocate(RY_PDS::CObjectIndex((RY_PDS::TTableIndex)table->getId(), rowId));
}


//
NLMISC_COMMAND(mapRow, "map a row in a table of a given database with a 64bits key", "<databaseId> <tableName> <rowId> <key64>")
{
	if (args.size() != 4)
		return false;

	TDatabaseId databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&	tableName = args[1];
	RY_PDS::TRowIndex	rowId;
	NLMISC::fromString(args[2], rowId);
	uint64				key;
	sscanf(args[3].c_str(), "%"NL_I64"X", &key);

	CDatabase*			database = CDbManager::getDatabase(databaseId);

	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);

	if (table == NULL)
		return false;

	return database->mapRow(RY_PDS::CObjectIndex((RY_PDS::TTableIndex)table->getId(), rowId), key);
}

//
NLMISC_COMMAND(unmapRow, "unmap a row in a table of a given database with a 64bits key", "<databaseId> <tableName> <key64>")
{
	if (args.size() != 3)
		return false;

	TDatabaseId			databaseId;
	NLMISC::fromString(args[0], databaseId);
	const std::string&	tableName = args[1];
	uint64				key;
	sscanf(args[2].c_str(), "%"NL_I64"X", &key);

	CDatabase*			database = CDbManager::getDatabase(databaseId);

	if (database == NULL)
		return false;

	const CTable*		table = database->getTable(tableName);

	if (table == NULL)
		return false;

	return database->unmapRow((RY_PDS::TTableIndex)table->getId(), key);
}

//
NLMISC_COMMAND(setValue, "set a value in table", "<databaseId> <tableName> <rowId> <columnId> <type> <value>")
{
	if (args.size() != 6)
		return false;

	TDatabaseId				databaseId;
	NLMISC::fromString(args[0], databaseId);

	const std::string&		tableName = args[1];

	RY_PDS::TRowIndex		rowId;
	NLMISC::fromString(args[2], rowId);

	RY_PDS::TColumnIndex	colId;
	NLMISC::fromString(args[3], colId);

	const std::string&		type = args[4];
	const std::string&		value = args[5];

	CDatabase*				database = CDbManager::getDatabase(databaseId);

	if (database == NULL)
		return false;

	const CTable*			table = database->getTable(tableName);

	if (table == NULL)
		return false;

	return database->set((RY_PDS::TTableIndex)table->getId(), rowId, colId, type, value);
}




//
NLMISC_COMMAND(set, "set a value in table", "<locatepath> [<type>] <value>")
{
	if (args.size() != 2 && args.size() != 3)
		return false;

	CLocatePath			path;

	if (!CDbManager::parsePath(args[0], path))
		return false;

	CTable::CDataAccessor	accessor = CDbManager::locate(path);

	if (!accessor.isValid())
		return false;

	CDatabase*			database = const_cast<CDatabase*>(accessor.table()->getParent());
	const CTable*		table = accessor.table();

	const std::string	value = (args.size() == 3 ? args[2] : args[1]);
	const std::string	type = (args.size() == 2 ? getNameFromDataType(accessor.column()->getDataType()) : args[1]);

	return database->set((RY_PDS::TTableIndex)table->getId(), accessor.row(), (RY_PDS::TColumnIndex)accessor.column()->getId(), type, value);
}

//
NLMISC_COMMAND(get, "get a value in table", "<locatepath>")
{
	if (args.size() != 1)
		return false;

	CLocatePath			path;

	if (!CDbManager::parsePath(args[0], path))
		return false;

	CTable::CDataAccessor	accessor = CDbManager::locate(path);

	log.displayNL("%s = '%s'", args[0].c_str(), accessor.valueAsString(1).c_str());

	return true;
}

//
//NLMISC_COMMAND(displayStringManager, "display the content of a string manager", "<databaseId>")
//{
//	if (args.size() != 1)
//		return false;
//
//	TDatabaseId				databaseId;
//	NLMISC::fromString(args[0], databaseId);
//	CDatabase*				database = CDbManager::getDatabase(databaseId);
//
//	if (database == NULL)
//		return false;
//
//	database->getStringManager().display(&log);
//
//	return true;
//}


//
NLMISC_COMMAND(dumpToXml, "dump the content of an object into an xml file", "<databaseId> <objectIndex|entityId|key64> <xmlfilename> [sint expandDepth=-1(infinite depth)]")
{
	if (args.size() < 3 || args.size() > 4)
		return false;

	TDatabaseId				databaseId;
	NLMISC::fromString(args[0], databaseId);

	CDatabase*				database = CDbManager::getDatabase(databaseId);

	if (database == NULL)
		return false;

	RY_PDS::CObjectIndex	index;

	index.fromString(args[1].c_str(), database);
	if (!index.isValid())
	{
		uint64				key;
		NLMISC::CEntityId	id;
		id.fromString(args[1].c_str());

		if (id == NLMISC::CEntityId::Unknown)
		{
			if (sscanf(args[1].c_str(), "%"NL_I64"u", &key) != 1)
			{
				log.displayNL("id '%s' is not recognized as an EntityId, an ObjectIndex nor a 64 bits raw key", args[1].c_str());
				return false;
			}
		}
		else
		{
			key = id.getRawId();
		}

		std::set<RY_PDS::CObjectIndex>	indexes;
		if (!database->searchObjectIndex(key, indexes))
		{
			log.displayNL("no object matching key %s found", args[1].c_str());
			return false;
		}

		if (indexes.size() > 1)
		{
			log.displayNL("%d objects match key '%s', please select the correct ObjectIndex below", indexes.size(), args[1].c_str());
			
			std::set<RY_PDS::CObjectIndex>::iterator	it;
			for (it=indexes.begin(); it!=indexes.end(); ++it)
				log.displayNL("%s", it->toString(database).c_str());
			return true;
		}

		index = *(indexes.begin());
	}

	COFile	ofile;
	if (!ofile.open(args[2]))
		return false;

	COXml	oxml;
	if (!oxml.init(&ofile))
		return false;

	sint	expandDepth = -1;
	if (args.size() == 4)
	{
		NLMISC::fromString(args[3], expandDepth);
	}

	database->dumpToXml(index, oxml, expandDepth);

	return true;
}

