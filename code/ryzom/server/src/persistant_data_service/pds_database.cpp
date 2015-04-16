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

#include "../pd_lib/pds_common.h"

#include "pds_database.h"
#include "pds_table.h"
#include "pds_type.h"
#include "database_adapter.h"

#include "db_manager.h"

#include "pd_lib/pd_server_utils.h"
#include "pd_lib/reference_builder.h"
#include "pd_lib/delta_builder.h"

#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/config_file.h>
#include <nel/misc/file.h>
#include <nel/misc/variable.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>

#include <nel/net/service.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// Hosted by db_manager.cpp
extern NLMISC::CVariable<uint>	DeltaUpdateRate;

/*
 * Constructor
 */
CDatabase::CDatabase(uint32 id) : _Reference(id)
{
	clear();

	_State.Id = id;
}

/*
 * Destructor
 */
CDatabase::~CDatabase()
{
	PDS_FULL_DEBUG("delete()");

	clear();
}


/*
 * Massive Database clear
 */
void	CDatabase::clear()
{
	uint	i;

	for (i=0; i<_Types.size(); ++i)
	{
		if (_Types[i] != NULL)
			delete _Types[i];
		_Types[i] = NULL;
	}

	_Types.clear();

	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL)
			delete _Tables[i];
		_Tables[i] = NULL;
	}

	_Tables.clear();

	_Init = false;
	//_ObjectList.clear();
	_SetMap.clear();
	_State.Name.clear();
	_ServiceId.set(0xffff);
}


/*
 * Init database
 * \param xmlStream is the xml description of the database
 */
bool	CDatabase::init()
{
	PDS_DEBUG("init()");

	// clear all before doing anything
	clear();

	const CDatabaseNode&	db = _Description.getDatabaseNode();

	_State.Name = db.Name;

	uint	i;
	for (i=0; i<db.Types.size(); ++i)
	{
		CType*	type = new CType;

		if (!type->init(this, db.Types[i]))
		{
			PDS_WARNING("init(): failed to init type '%d'", i);
			return false;
		}

		if (type->getId() != i)
		{
			PDS_WARNING("init(): failed to init type '%d'", i);
			return false;
		}

		_Types.push_back(type);
	}

	for (i=0; i<db.Tables.size(); ++i)
	{
		CTable*	table = new CTable();

		if (!table->init(this, db.Tables[i]) || table->getId() != i)
		{
			PDS_WARNING("init(): failed to init table '%d'", i);
			return false;
		}

		_Tables.push_back(table);
	}

	for (i=0; i<_Tables.size(); ++i)
	{
		CTable*	table = _Tables[i];

		if (!table->buildColumns())
		{
			PDS_WARNING("init(): failed to build table '%d' columns", i);
			return false;
		}
	}

	for (i=0; i<_Tables.size(); ++i)
	{
		_Tables[i]->postInit();
	}

	_Init = true;

	if (!notifyNewReference(false))
	{
		_Init = false;
		return false;
	}

	// save description in log dir
	std::string	logDir = RY_PDS::CPDSLib::getLogDirectory(_State.Id);
	if (!CFile::isExists(logDir) || !CFile::isDirectory(logDir))
	{
		if (!CFile::createDirectoryTree(logDir))
		{
			PDS_WARNING("init(): failed to create log root directory '%s'", logDir.c_str());
		}

		if (!CFile::setRWAccess(logDir))
		{
			PDS_WARNING("init(): failed, can't set RW access to directory '%s'", logDir.c_str());
		}
	}

//	CTimestamp	initDate;
//	initDate.setToCurrent();
//	_Description.saveDescription(logDir + initDate.toString() + ".description");

	PDS_DEBUG("init() successful");

	return true;
}




/*
 * Checkup database
 */
bool	CDatabase::checkup()
{
	PDS_DEBUG("checkup()");

	if (!initialised())
	{
		PDS_WARNING("checkup(): database not initialised");
		return false;
	}

	uint	i;
	bool	dbOk = true;

	for (i=0; i<_Types.size(); ++i)
	{
		if (_Types[i] == NULL || !_Types[i]->initialised())
		{
			PDS_WARNING("checkup(): type '%d' not initialised", i);
			continue;
		}
	}

	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] == NULL || !_Tables[i]->initialised())
		{
			PDS_WARNING("checkup(): table '%d' not initialised", i);
			continue;
		}

		CTable*	table = _Tables[i];

		const std::vector<CColumn>		&columns = table->getColumns();
		const std::vector<CAttribute*>	&attributes = table->getAttributes();

		uint	j;

		// walk through attributes
		for (j=0; j<attributes.size(); ++j)
		{
			if (attributes[j] == NULL || !attributes[j]->initialised())
			{
				PDS_WARNING("checkup(): attribute '%s:%d' not initialised", table->getName().c_str(), i);
				dbOk = false;
				continue;
			}

			const CAttribute*	attribute = attributes[j];

			if (attribute->getParent() != table)
			{
				PDS_WARNING("checkup(): attribute '%s:%s' doesn't point table '%d'", table->getName().c_str(), attribute->getName().c_str(), i);
				dbOk = false;
			}

			uint	k;

			for (k=attribute->getOffset(); k<attribute->getOffset()+attribute->getColumns(); ++k)
			{
				if (k >= columns.size() || !columns[k].initialised())
				{
					PDS_WARNING("checkup(): attribute '%s:%s' points to not initialised column '%d'", table->getName().c_str(), attribute->getName().c_str(), k);
					dbOk = false;
				}
				else if (columns[k].getParent() != attribute)
				{
					PDS_WARNING("checkup(): attribute '%s:%s' points to column '%s' that doesn't point back", table->getName().c_str(), attribute->getName().c_str(), columns[k].getName().c_str());
					dbOk = false;
				}
				else
				{
					const CColumn	&column = columns[k];

				}
			}
		}

		// walk through columns
		for (j=0; j<columns.size(); ++j)
		{
			if (!columns[j].initialised())
			{
				PDS_WARNING("checkup(): column '%s:%d' not initialised", table->getName().c_str(), j);
				dbOk = false;
				continue;
			}
		}
	}

	if (dbOk)
	{
		PDS_DEBUG("checkup(): Database is correct");
	}

	PDS_DEBUG("checkup() finished");

	return true;
}

/*
 * Initialise internal timestamps
 */
void	CDatabase::initTimestamps()
{
	// initialise update and creation timestamps to now
	_CreationTimestamp.setToCurrent();
	_MinuteUpdateTimestamp = _CreationTimestamp;
	_HourUpdateTimestamp = _CreationTimestamp;
	_DayUpdateTimestamp = _CreationTimestamp;
}



/*
 * Get value as a string
 * \param path is of the form '[tableindex|tablename].[$key|index].attrib1.attrib2'
 */
string	CDatabase::getValue(const CLocatePath::TLocatePath &path)
{
	if (path.size() < 3)
	{
		PDS_WARNING("getValue(): path is too short");
		return "";
	}

	uint	node = 0;

	// select table
	const CTable*	table;
	uint			tableId;
	if (sscanf(path[node].Name.c_str(), "%d", &tableId) == 1)
		table = getTable(tableId);
	else
		table = getTable(path[node].Name);

	if (table == NULL)
	{
		PDS_WARNING("getValue(): unable to select table '%s'", path[node].Name.c_str());
		return "";
	}

	++node;

	// select row
	RY_PDS::CObjectIndex	object;

	if (path[node].Name[0] == '$')
	{
		uint64	key;
		if (sscanf(path[node].Name.c_str()+1, "%"NL_I64"X", &key) != 1)
		{
			PDS_WARNING("getValue(): unable to select mapped row '%s'", path[node].Name.c_str());
			return "";
		}

		object = table->getMappedRow(key);
	}
	else
	{
		RY_PDS::TRowIndex	row;

		if (sscanf(path[node].Name.c_str(), "%u", &row) != 1)
		{
			PDS_WARNING("getValue(): unable to select row '%s'", path[node].Name.c_str());
			return "";
		}

		object = RY_PDS::CObjectIndex((RY_PDS::TTableIndex)table->getId(), row);
	}

	if (!object.isValid() || !isAllocated(object))
	{
		PDS_WARNING("getValue(): object '%s' not accessible", path[node].Name.c_str());
		return "";
	}

	++node;

	const CTable*	subTable = table;

	// browse through row
	while (node < path.size())
	{
		const CAttribute*	attribute = subTable->getAttribute(path[node].Name);

		if (attribute == NULL)
		{
			PDS_WARNING("getValue(): '%s' is not an attribute of '%s'", path[node].Name.c_str(), subTable->getName().c_str());
			return "";
		}

		++node;
	}

	return "";
}






/*
 * Get Table
 */
const CTable*	CDatabase::getTable(const std::string &name) const
{
	uint	i;
	for (i=0; i<_Tables.size(); ++i)
		if (_Tables[i] != NULL && _Tables[i]->getName() == name)
			return _Tables[i];

	return NULL;
}

/*
 * Get Type
 */
const CType*	CDatabase::getType(const std::string &name) const
{
	uint	i;
	for (i=0; i<_Types.size(); ++i)
		if (_Types[i] != NULL && _Types[i]->getName() == name)
			return _Types[i];

	return NULL;
}


/*
 * Get Attribute
 */
const CAttribute*	CDatabase::getAttribute(uint32 tableId, uint32 attributeId) const
{
	const CTable*	table = getTable(tableId);

	if (table == NULL)
		return NULL;

	return table->getAttribute(attributeId);
}

/*
 * Get Column
 */
const CColumn*	CDatabase::getColumn(uint32 tableId, uint32 columnId) const
{
	const CTable*	table = getTable(tableId);

	if (table == NULL)
		return NULL;

	return table->getColumn(columnId);
}




/*
 * Allocate a row in a table
 * \param index is the table/row to allocate
 * Return true if succeded
 */
bool	CDatabase::allocate(const RY_PDS::CObjectIndex &index)
{
	H_AUTO(PDS_Database_allocate);

	if (!initialised())
	{
		PDS_WARNING("allocate(): database not initialised");
		return false;
	}

	if (!index.isValid())
	{
		PDS_WARNING("allocate(): index '%s' is not valid", index.toString().c_str());
		return false;
	}

	CTable	*table = getNonConstTable(index.table());

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("allocate(): table '%d' is not initialised", index.table());
		return false;
	}

	bool	success = table->allocate(index.row());

	if (success)
		PDS_FULL_DEBUG("allocated '%s' successfully", index.toString().c_str());

	return success;
}

/*
 * Deallocate a row in a table
 * \param index is the table/row to deallocate
 * Return true if succeded
 */
bool	CDatabase::deallocate(const RY_PDS::CObjectIndex &index)
{
	H_AUTO(PDS_Database_deallocate);

	if (!initialised())
	{
		PDS_WARNING("deallocate(): database not initialised");
		return false;
	}

	if (!index.isValid())
	{
		PDS_WARNING("deallocate(): index '%s' is not valid", index.toString().c_str());
		return false;
	}

	CTable	*table = getNonConstTable(index.table());

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("deallocate(): table '%d' is not initialised", index.table());
		return false;
	}

	bool	success = table->deallocate(index.row());

	if (success)
		PDS_FULL_DEBUG("deallocated '%s' successfully", index.toString().c_str());

	return success;
}

/*
 * Tells if an object is allocated
 * \param object is the object index to test
 */
bool	CDatabase::isAllocated(const RY_PDS::CObjectIndex &index) const
{
	const CTable*	table = getTable(index.table());

	return table != NULL && table->isAllocated(index.row());
}


/*
 * Map a row in a table
 * \param index is the table/row to allocate
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CDatabase::mapRow(const RY_PDS::CObjectIndex &index, uint64 key)
{
	if (!initialised())
	{
		PDS_WARNING("mapRow(): database not initialised");
		return false;
	}

	if (!index.isValid())
	{
		PDS_WARNING("mapRow(): index '%s' is not valid", index.toString().c_str());
		return false;
	}

	CTable	*table = getNonConstTable(index.table());

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("deallocate(): table '%d' is not initialised", index.table());
		return false;
	}

	bool	success = table->mapRow(index, key);

	if (success)
		PDS_FULL_DEBUG("mapped '%016"NL_I64"X' to '%s' successfully", key, index.toString().c_str());

	return success;
}

/*
 * Unmap a row in a table
 * \param tableIndex is the table to find row
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CDatabase::unmapRow(RY_PDS::TTableIndex tableIndex, uint64 key)
{
	if (!initialised())
	{
		PDS_WARNING("unmapRow(): database not initialised");
		return false;
	}

	CTable	*table = getNonConstTable(tableIndex);

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("deallocate(): table '%d' is not initialised", tableIndex);
		return false;
	}

	bool	success = table->unmapRow(key);

	if (success)
		PDS_FULL_DEBUG("unmapped '%016"NL_I64"X' successfully", key);

	return success;
}

/**
 * Get a mapped row
 * \param tableIndex is the table in which the row is mapped
 * \param key is the 64 bits row key
 * Return a valid CObjectIndex if success
 */
RY_PDS::CObjectIndex	CDatabase::getMappedRow(RY_PDS::TTableIndex tableIndex, uint64 key) const
{
	if (!initialised())
	{
		PDS_WARNING("getMappedRow(): database not initialised");
		return RY_PDS::CObjectIndex();
	}

	const CTable	*table = getTable(tableIndex);

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("getMappedRow(): table '%d' is not initialised", tableIndex);
		return RY_PDS::CObjectIndex();
	}

	return table->getMappedRow(key);
}

/**
 * Release a row in a table
 * \param index is the table/row to release
 * Return true if succeded
 */
bool	CDatabase::release(const RY_PDS::CObjectIndex &index)
{
	if (!initialised())
	{
		PDS_WARNING("release(): database not initialised");
		return false;
	}

	if (!index.isValid())
	{
		PDS_WARNING("release(): index '%s' is not valid", index.toString().c_str());
		return false;
	}

	CTable	*table = getNonConstTable(index.table());

	if (table == NULL || !table->initialised())
	{
		PDS_WARNING("release(): table '%d' is not initialised", index.table());
		return false;
	}

	bool	success = table->release(index.row());

	if (success)
		PDS_FULL_DEBUG("released '%s' successfully", index.toString().c_str());

	return success;
}

/*
 * Release all rows in all table
 * Typically, the client disconnected, there is no need to keep rows
 */
bool	CDatabase::releaseAll()
{
	if (!initialised())
	{
		PDS_WARNING("releaseAll(): database not initialised");
		return false;
	}

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			_Tables[i]->releaseAll();
		}
	}

	return true;
}


//
// Set method
//
bool	CDatabase::set(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr)
{
	H_AUTO(PDS_Database_set);

	if (!initialised())
	{
		PDS_WARNING("set(): database not initialised");
		return false;
	}

	CTable*		_table = getNonConstTable(table);

	if (_table == NULL)
	{
		PDS_WARNING("set(): table '%d' is NULL", table);
		return false;
	}

	bool	success = _table->set(row, column, datasize, dataptr);

	if (success)
		PDS_FULL_DEBUG("set '%s' column '%d' successfully", RY_PDS::CObjectIndex(table, row).toString().c_str(), column);

	return success;
}


/*
 * Set an object parent
 */
bool	CDatabase::setParent(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const RY_PDS::CObjectIndex &parent)
{
	H_AUTO(PDS_Database_setParent);

	if (!initialised())
	{
		PDS_WARNING("set(): database not initialised");
		return false;
	}

	CTable*		_table = getNonConstTable(table);

	if (_table == NULL)
	{
		PDS_WARNING("set(): table '%d' is NULL", table);
		return false;
	}

	bool	success = _table->setParent(row, column, parent);

	if (success)
		PDS_FULL_DEBUG("set '%s' column '%d' successfully", RY_PDS::CObjectIndex(table, row).toString().c_str(), column);

	return success;
}


//
// Get method
//
bool	CDatabase::get(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint& datasize, void* dataptr, TDataType &type)
{
	if (!initialised())
	{
		PDS_WARNING("set(): database not initialised");
		return false;
	}

	CTable*		_table = getNonConstTable(table);

	if (_table == NULL)
	{
		PDS_WARNING("set(): table '%d' is NULL", table);
		return false;
	}

	bool	success = _table->get(row, column, datasize, dataptr, type);

	if (success)
		PDS_FULL_DEBUG("get '%s' column '%d' successfully", RY_PDS::CObjectIndex(table, row).toString().c_str(), column);

	return success;
}




/*
 * Display database
 */
void	CDatabase::display(NLMISC::CLog* log, bool displayHeader) const
{
	if (!initialised())
	{
		log->displayNL("Database not initialised");
	}

	log->displayNL("Database '%s'", _State.Name.c_str());
	log->displayNL("%d types, %d tables", _Types.size(), _Tables.size());

	uint	i;

	log->displayNL("     %-36s | %-15s | %-2s | %s", "TypeId/Name", "DataTypeId/Name", "Sz", "IsIndex");
	for (i=0; i<_Types.size(); ++i)
	{
		if (_Types[i] == NULL)
			log->display("** Type %d not initialised", i);
		else
			_Types[i]->display(log);
	}

	bool	first = true;

	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] == NULL)
		{
			log->display("** Table %d not initialised", i);
		}
		else
		{
			_Tables[i]->display(log, false, first);
			first = false;
		}
	}
}

/*
 * Dump database content and info of an object to xml
 */
void	CDatabase::dumpToXml(const RY_PDS::CObjectIndex& index, NLMISC::IStream& xml, sint expandDepth)
{
	if (xml.isReading() || !initialised())
		return;

	CTable*	table = getNonConstTable(index.table());

	if (table != NULL)
		table->dumpToXml(index.row(), xml, expandDepth);
}



/*
 * Set value with human readable parameters
 */
bool	CDatabase::set(RY_PDS::TTableIndex table, RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const std::string& type, const std::string &value)
{
	TDataType	datatype = getDataTypeFromName(type);

	if (!checkDataType(datatype))
		return false;

	switch (datatype)
	{
	case PDS_bool:
	case PDS_char:
	case PDS_uint8:
	case PDS_sint8:
		{
			uint8	data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_ucchar:
	case PDS_uint16:
	case PDS_sint16:
		{
			uint16	data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_uint32:
	case PDS_sint32:
	case PDS_enum:
	case PDS_CSheetId:
	case PDS_CNodeId:
		{
			uint32	data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_uint64:
	case PDS_sint64:
		{
			uint64	data;
			sscanf(value.c_str(), "%016"NL_I64"d", &data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_CEntityId:
		{
			CEntityId	data;
			data.fromString(value.c_str());
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_float:
		{
			float data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_double:
		{
			double data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_Index:
		{
			RY_PDS::CObjectIndex	data;
			data.fromString(value.c_str());
			return set(table, row, column, sizeof(data), &data);
		}
		break;

	case PDS_dimension:
		{
			uint32	data;
			NLMISC::fromString(value, data);
			return set(table, row, column, sizeof(data), &data);
		}
		break;
	}

	return false;
}


/*
 * Fetch data
 */
bool	CDatabase::fetch(const RY_PDS::CObjectIndex& index, RY_PDS::CPData &data, bool fetchIndex)
{
	H_AUTO(PDS_Database_fetch);

	if (!initialised())
	{
		PDS_WARNING("set(): database not initialised");
		return false;
	}

	CTable*		table = getNonConstTable(index.table());

	if (table == NULL)
	{
		PDS_WARNING("fetch(): unable to get table '%d'", index.table());
		return false;
	}

	bool	success = table->fetch(index.row(), data, fetchIndex);

	if (success)
		PDS_FULL_DEBUG("fetch '%s' successfully", index.toString().c_str());

	return success;
}




/*
 * Build index allocators
 * One per table
 */
bool	CDatabase::buildIndexAllocators(std::vector<RY_PDS::CIndexAllocator> &allocators)
{
	PDS_DEBUG("buildIndexAllocators()");

	if (!initialised())
	{
		PDS_WARNING("buildIndexAllocators(): database not initialised");
		return false;
	}

	allocators.clear();
	allocators.resize(_Tables.size());

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
		if (_Tables[i] != NULL && _Tables[i]->initialised())
			_Tables[i]->buildIndexAllocator(allocators[i]);

	return true;
}






/*
 * Rebuild forwardrefs from backrefs
 */
bool	CDatabase::rebuildForwardRefs()
{
	if (!initialised())
	{
		PDS_WARNING("rebuildForwardRefs(): database not initialised");
		return false;
	}

	// clear up object list
	_SetMap.clear();

	// the rebuild forwardrefs
	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			if (!_Tables[i]->resetForwardRefs())
			{
				PDS_WARNING("rebuildForwardRefs(): failed to resetForwardRefs(), abort");
				return false;
			}
		}
	}

	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			if (!_Tables[i]->rebuildForwardRefs())
			{
				PDS_WARNING("rebuildForwardRefs(): failed to rebuildForwardRefs(), abort");
				return false;
			}
		}
	}

	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			if (!_Tables[i]->fixForwardRefs())
			{
				PDS_WARNING("rebuildForwardRefs(): failed to fixForwardRefs(), abort");
				return false;
			}
		}
	}

	return true;
}


/*
 * Rebuild table maps
 */
bool	CDatabase::rebuildTableMaps()
{
	if (!initialised())
	{
		PDS_WARNING("rebuildTableMaps(): database not initialised");
		return false;
	}

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			_Tables[i]->buildRowMapper();
		}
	}

	return true;
}

/*
 * Reset dirty lists
 */
bool	CDatabase::resetDirtyTags()
{
	if (!initialised())
	{
		PDS_WARNING("resetDirtyLists(): database not initialised");
		return false;
	}

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			_Tables[i]->resetDirtyTags();
		}
	}

	return true;
}

/*
 * Reset and rebuild all maps, references, lists...
 */
bool	CDatabase::rebuildVolatileData()
{
	PDS_DEBUG("rebuildVolatileData()");

	if (!initialised())
	{
		PDS_WARNING("rebuildVolatileData(): database not initialised");
		return false;
	}

	// force database to notify a new reference is up to date
	notifyNewReference();

	if (!rebuildTableMaps())
	{
		PDS_WARNING("rebuildVolatileData(): failed to rebuildTableMaps()");
		return false;
	}

	if (!rebuildForwardRefs())
	{
		PDS_WARNING("rebuildVolatileData(): failed to rebuildForwardRefs()");
		return false;
	}

	/*
	if (!resetDirtyTags())
	{
		PDS_WARNING("rebuildVolatileData(): failed to resetDirtyTags()");
		return false;
	}
	*/

	// preload data from reference files
	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] != NULL && _Tables[i]->initialised())
		{
			if (!_Tables[i]->preloadRefFiles())
			{
				PDS_WARNING("rebuildVolatileData(): failed to preloadRefFiles() for table '%d'", i);
				return false;
			}
		}
	}

	// load string manager
//	if (!_StringManager.load(_Reference.getPath()))
//	{
//		PDS_WARNING("rebuildVolatileData(): failed to load string manager");
//	}

	return true;
}





/*
 * Adapt database to new description
 * \param description is the latest xml description of the database
 * \returns true is adaptation succeded
 */
CDatabase*	CDatabase::adapt(const string& description)
{
	PDS_DEBUG("adapt()");

	if (!initialised())
	{
		PDS_WARNING("adapt(): failed, database not initialised");
		return NULL;
	}

	// get 'From' HashKey
	CHashKey	hash1 = _Description.getHashKey();
	// get 'Into' HashKey
	CHashKey	hash2 = getSHA1((const uint8*)(description.c_str()), (uint32)description.size());

	// same hash, ok go on
	if (hash1 == hash2)
		return this;

	// build a clean reference if needed
	if (!isReferenceUpToDate())
	{
		if (!buildReference())
		{
			PDS_WARNING("adapt(): failed to buildReference()");
			return NULL;
		}
	}

	// backup old reference
	if (!_Reference.save(_Reference.getPath()+"ref"))
	{
		PDS_WARNING("adapt(): failed to backup reference index");
	}

	// create a new destination database
	CDatabase*	into = new CDatabase(_State.Id);

	if (!into->createFromScratch(description))
	{
		PDS_WARNING("adapt(): failed to create new reference");
		delete into;
		return NULL;
	}

	CDatabaseAdapter	adapter;

	if (!adapter.build(this, into))
	{
		PDS_WARNING("adapt(): failed to build() adapter");
		delete into;
		return NULL;
	}

	if (!adapter.buildContent())
	{
		PDS_WARNING("adapt(): adapter failed to buildContent()");
		delete into;
		return NULL;
	}

	// rebuild volatile data
	// that is all non persistant data that can be found from persistant data
	if (!into->rebuildVolatileData())
	{
		PDS_WARNING("adapt(): failed to rebuildVolatileData()");
		delete into;
		return NULL;
	}

	// init timestamps as database is ready to run
	into->initTimestamps();

	return into;
}












/*
 * Load previous database state
 */
bool	CDatabase::loadState()
{
	PDS_DEBUG("loadState()");

	if (initialised())
	{
		PDS_WARNING("loadState(): cannot loadState(), database is already initialised");
		return false;
	}

	// check directory exists
	std::string	directory = _Reference.getNominalRootPath();
	if (!CFile::isExists(directory) || !CFile::isDirectory(directory))
	{
		return false;
	}

	// load database current state (or previous state if current state corrupted)
	if (!_State.load(_Reference) && !_State.load(_Reference, true))
	{
		return false;
	}

	// get last valid reference
	if (!_Reference.load())
	{
		return false;
	}

	if (_Reference.Index != _State.CurrentIndex)
	{
		PDS_WARNING("loadState(): failed, Reference and State files have different current index", (_Reference.getPath()+"description.xml").c_str());
		return false;
	}

	// load description
	if (!_Description.loadDescriptionFile(_Reference.getPath()+"description.xml"))
	{
		PDS_WARNING("loadState(): failed to load description file '%s'", (_Reference.getPath()+"description.xml").c_str());
		return false;
	}

	// and init!
	if (!init())
	{
		PDS_WARNING("loadState(): failed to init db '%d'", _State.Id);
		return false;
	}

	PDS_DEBUG("loadState(): database '%d' init ok", _State.Id);

	// check reference is up to date
	if (!isReferenceUpToDate())
	{
		if (!buildReference())
		{
			PDS_WARNING("loadState(): failed to buildReference()");
			return false;
		}
	}

	// rebuild volatile data
	// that is all non persistant data that can be found from persistant data
	if (!rebuildVolatileData())
	{
		PDS_WARNING("loadState(): failed to rebuildVolatileData()");
		return false;
	}

	// init timestamps as database is ready to run
	initTimestamps();

	return true;
}


/*
 * Check if reference is still the same
 * Reinit reference if reference changed
 */
bool	CDatabase::checkReferenceChange()
{
	CRefIndex	ref(_State.Id);

	// load current reference
	if (!ref.load())
	{
		PDS_WARNING("checkReferenceChange(): failed to load reference for database '%d', no change assumed", _State.Id);
		return false;
	}

	// check current database reference and current up to date reference
	if (ref.Index != _Reference.Index)
	{
		// check nothing went wrong
		if (ref.Index < _Reference.Index || ref.Timestamp <= _Reference.Timestamp)
		{
			PDS_WARNING("checkReferenceChange(): index changed (new index %d), but is lower than previous index (%d), no change assumed", ref.Index, _Reference.Index);
			return false;
		}

		// affect new reference
		_Reference = ref;

		notifyNewReference(false);
		return true;
	}

	return false;
}


/*
 * Check if reference is up to date
 * Returns true if reference is the latest valid database image
 */
bool	CDatabase::isReferenceUpToDate()
{
	PDS_DEBUG("isReferenceUpToDate()");

	if (!initialised())
	{
		PDS_WARNING("isReferenceUpToDate(): failed, database is not initialised");
		return false;
	}

	vector<string>	files;

	NLMISC::CPath::getPathContent(_Reference.getRootPath()+"hours", false, false, true, files);
	NLMISC::CPath::getPathContent(_Reference.getRootPath()+"minutes", false, false, true, files);
	NLMISC::CPath::getPathContent(_Reference.getRootPath()+"seconds", false, false, true, files);

	bool	upToDate = true;

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		uint32		tableId;
		CTimestamp	timestamp;

		bool		deltaFile = CDBDeltaFile::isDeltaFileName(files[i], tableId, timestamp);
//		bool		stringLogFile = RY_PDS::CPDStringManager::isLogFileName(files[i], timestamp);

		if (deltaFile /*|| stringLogFile*/)
		{
			if (timestamp > _State.EndTimestamp)
			{
				CFile::moveFile((files[i]+".disabled").c_str(), files[i].c_str());
				continue;
			}

			// compare file timestamp (in name) to current reference timestamp
			if (_Reference.Timestamp <= timestamp && timestamp <= _State.EndTimestamp)
				upToDate = false;
		}
	}

	if (upToDate)
		PDS_DEBUG("isReferenceUpToDate(): database is up to date");

	return upToDate;
}

/*
 * Build a up to date reference
 */
bool	CDatabase::buildReference()
{
	PDS_DEBUG("buildReference()");

	CRefIndex	prev = _Reference;
	CRefIndex	next(_State.Id);

	if (!next.buildNext())
	{
		PDS_WARNING("buildReference(): failed to build next ref");
		return false;
	}

	PDS_DEBUG("buildReference(): building from reference '%08X' to reference '%08X'", prev.Index, next.Index);

	// build reference but clamp to last valid state (
	next.Timestamp = _State.EndTimestamp;

	_Reference = next;

	// setup reference
	// this will release all references owned by the internal table buffers
	// so we can copy/modify reference files without problem

	if (!CReferenceBuilder::build(prev, next))
	{
		PDS_WARNING("buildReference(): failed to build next reference");
		return false;
	}

	notifyNewReference();

	PDS_DEBUG("buildReference(): new reference is up to date");

	return true;
}





/*
 * Create new database from scratch, setup everything needed (references, etc.)
 * \param description is the xml database description
 */
bool	CDatabase::createFromScratch(const string& description)
{
	PDS_DEBUG("createFromScratch()");

	// In order to create a new database from scratch, directory must no exist
	if (CFile::isExists(_Reference.getNominalRootPath()))
	{
		PDS_WARNING("createFromScratch(): failed, directory '%s' already exists for database '%d', manual cleanup is needed to proceed", _Reference.getRootPath().c_str(), _Reference.DatabaseId);
		return false;
	}

	// build a brand new reference
	if (!_Reference.buildNext())
	{
		PDS_WARNING("createFromScratch(): failed, unable to init reference");
		return false;
	}

	if (initialised())
	{
		PDS_WARNING("createFromScratch(): failed, database is initialised");
		return false;
	}

	if (!_Description.loadDescription((uint8*)description.c_str()))
	{
		PDS_WARNING("createFromScratch(): failed to load description");
		return false;
	}

	if (!init())
	{
		PDS_WARNING("createFromScratch(): failed to init() database");
		return false;
	}

//	// save description
//	if (!_Description.saveDescription(_Reference.getPath() + "description.xml"))
//	{
//		PDS_WARNING("createFromScratch(): failed to save description");
//		return false;
//	}

	// init timestamps as database is ready to run
	initTimestamps();

	// a new reference is valid
	notifyNewReference();

	return true;
}

/*
 * Build the delta files and purge all dirty rows in tables
 */
bool	CDatabase::buildDelta(const CTimestamp& starttime, const CTimestamp& endtime)
{
	H_AUTO(PDS_Database_buildDelta);

	if (!initialised())
	{
		PDS_WARNING("buildDelta(): failed, database is not initialised");
		return false;
	}

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (!_Tables[i]->buildDelta(starttime, endtime))
		{
			PDS_WARNING("buildDelta(): failed to buildDelta() for table '%d' '%s'", i, _Tables[i]->getName().c_str());
			return false;
		}
	}

//	std::string	logDir = RY_PDS::CPDSLib::getLogDirectory(_State.Id);
//	if (!CFile::isExists(logDir) || !CFile::isDirectory(logDir))
//	{
//		if (!CFile::createDirectoryTree(logDir))
//		{
//			PDS_WARNING("buildDelta(): failed to create log root directory '%s'", logDir.c_str());
//		}
//
//		if (!CFile::setRWAccess(logDir))
//		{
//			PDS_WARNING("buildDelta(): failed, can't set RW access to directory '%s'", logDir.c_str());
//		}
//	}

//	// save string manager logs
//	if (!_StringManager.logEmpty())
//	{
//		COFile		smf;
//		COXml		smxml;
//		std::string	smfilename = logDir + endtime.toString()+".string_log";
//		if (!smf.open(smfilename) || !smxml.init(&smf) || !_StringManager.storeLog(smxml))
//		{
//			PDS_WARNING("buildDelta(): failed to build string manager log file '%s'", smfilename.c_str());
//		}
//	}

	// save straight logs
	if (!_LogQueue.empty())
	{
//		std::string	logfilename = logDir + endtime.toString()+"_0000.pd_log";
//		COFile		logf;
//		if (logf.open(logfilename))
//		{
//			try
//			{
//				logf.serialCont(_LogQueue);
//			}
//			catch (const Exception& e)
//			{
//				PDS_WARNING("buildDelta(): exception occured while saving straight log : %s", e.what());
//			}
//		}
//		else
//		{
//			PDS_WARNING("buildDelta(): failed to build log file '%s'", logfilename.c_str());
//		}
//
		_LogQueue.clear();
	}

	// State file swapping
	std::string	statePath = _Reference.getRootPath();
	std::string	stateName = CDatabaseState::fileName();
	if (CFile::fileExists(statePath+stateName) &&
		!CFile::copyFile(statePath+"previous_"+stateName, statePath+stateName, false))
	{
		PDS_WARNING("buildDelta(): failed copy state file to backup previous_state");
	}

	// setup state timestamp
	_State.EndTimestamp = endtime;

	_State.save(_Reference);

	return true;
}

/*
 * Flush database from released rows
 */
bool	CDatabase::flushReleased()
{
	if (!initialised())
	{
		PDS_WARNING("flushReleased(): failed, database is not initialised");
		return false;
	}

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (!_Tables[i]->flushReleased())
		{
			PDS_WARNING("flushReleased(): failed to flushReleased() for table '%d' '%s'", i, _Tables[i]->getName().c_str());
			return false;
		}
	}

	return true;
}



/*
 * Notify a new reference is ready
 */
bool	CDatabase::notifyNewReference(bool validateRef)
{
	if (!initialised())
	{
		PDS_WARNING("notifyNewReference(): failed, database is not initialised");
		return false;
	}

	if (validateRef)
		_Reference.setAsValidRef();

	_State.CurrentIndex = _Reference.Index;

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
		_Tables[i]->notifyNewReference(_Reference);

	return true;
}




/*
 * Receive update
 */
void	CDatabase::receiveUpdate(uint32 id)
{
	// add to acknowledged updates
	_ReceivedUpdates.push_back(id);

	_State.LastUpdateId = id;
}

/*
 * Flush updates
 */
void	CDatabase::flushUpdates(std::vector<uint32>& acknowledged)
{
	// copy acknowledge and flush
	acknowledged = _ReceivedUpdates;
	_ReceivedUpdates.clear();
}


/**
 * Get Update Queue for id
 * May return NULL if message was already received
 */
RY_PDS::CDbMessageQueue*	CDatabase::getUpdateMessageQueue(uint32 updateId)
{
	if (updateId != 0 && updateId <= _State.LastUpdateId)
		return NULL;

	_LogQueue.push_back(RY_PDS::CUpdateLog());

	RY_PDS::CUpdateLog&	ulog = _LogQueue.back();

	ulog.UpdateId = updateId;
	ulog.createUpdates();
	ulog.StartStamp.setToCurrent();
	ulog.EndStamp.setToCurrent();

	return ulog.getUpdates();
}




CVariable<uint>	MinuteUpdateRate("pds", "MinuteUpdateRate", "Number of seconds between two minute updates", 60, 0, true);
CVariable<uint>	HourUpdateRate("pds", "HourUpdateRate", "Number of seconds between two hour updates", 3600, 0, true);
CVariable<uint>	ReferenceUpdateRate("pds", "ReferenceUpdateRate", "Number of seconds between two reference builds", 86400, 0, true);

CVariable<uint>	KeepSecondsAtMinuteUpdate("pds", "KeepSecondsAtMinuteUpdate", "Number of seconds to keep (in number of minute updates)", 2, 0, true);
CVariable<uint>	KeepMinutesAtHourUpdate("pds", "KeepMinutesAtHourUpdate", "Number of minutes to keep (in number of hours updates)", 2, 0, true);
CVariable<uint>	KeepHoursAtReferenceUpdate("pds", "KeepHoursAtReferenceUpdate", "Number of hours to keep (in number of daily updates)", 2, 0, true);


/*
 * Send Delta/Reference build commands
 */
bool	CDatabase::sendBuildCommands(const CTimestamp& current)
{
	checkUpdateRates();

	while (current - _MinuteUpdateTimestamp >= MinuteUpdateRate)
	{
		CTimestamp	start = _MinuteUpdateTimestamp;
		CTimestamp	end = start + MinuteUpdateRate;
		CTimestamp	keep = end - MinuteUpdateRate*KeepSecondsAtMinuteUpdate;

		string					outputPath = _Reference.getMinutesUpdatePath();
		string					hoursUpdatePath = _Reference.getHoursUpdatePath();
		string					minutesUpdatePath = _Reference.getMinutesUpdatePath();
		string					secondsUpdatePath = _Reference.getSecondsUpdatePath();
		string					mintimestamp = start.toString();
		string					maxtimestamp = end.toString();
		CDeltaBuilder::TDelta	type = CDeltaBuilder::Minute;
		string					keeptimestamp = keep.toString();

		CMessage&				msgdelta = CDbManager::addTask("RB_GEN_DELTA", NULL, NULL);
		msgdelta.serial(outputPath);
		msgdelta.serial(hoursUpdatePath);
		msgdelta.serial(minutesUpdatePath);
		msgdelta.serial(secondsUpdatePath);
		msgdelta.serial(mintimestamp);
		msgdelta.serial(maxtimestamp);
		msgdelta.serialEnum(type);
		msgdelta.serial(keeptimestamp);

		_MinuteUpdateTimestamp = end;
	}


	while (current - _HourUpdateTimestamp >= HourUpdateRate)
	{
		CTimestamp	start = _HourUpdateTimestamp;
		CTimestamp	end = start + HourUpdateRate;
		CTimestamp	keep = end - HourUpdateRate*KeepMinutesAtHourUpdate;

		string					outputPath = _Reference.getHoursUpdatePath();
		string					hoursUpdatePath = _Reference.getHoursUpdatePath();
		string					minutesUpdatePath = _Reference.getMinutesUpdatePath();
		string					secondsUpdatePath = _Reference.getSecondsUpdatePath();
		string					mintimestamp = start.toString();
		string					maxtimestamp = end.toString();
		CDeltaBuilder::TDelta	type = CDeltaBuilder::Hour;
		string					keeptimestamp = keep.toString();

		CMessage&				msgdelta = CDbManager::addTask("RB_GEN_DELTA", NULL, NULL);
		msgdelta.serial(outputPath);
		msgdelta.serial(hoursUpdatePath);
		msgdelta.serial(minutesUpdatePath);
		msgdelta.serial(secondsUpdatePath);
		msgdelta.serial(mintimestamp);
		msgdelta.serial(maxtimestamp);
		msgdelta.serialEnum(type);
		msgdelta.serial(keeptimestamp);

		_HourUpdateTimestamp = end;
	}

	while (current - _DayUpdateTimestamp >= ReferenceUpdateRate)
	{
		CTimestamp	start = _DayUpdateTimestamp;
		CTimestamp	end = start + ReferenceUpdateRate;
		CTimestamp	keep = end - ReferenceUpdateRate*KeepHoursAtReferenceUpdate;

		CRefIndex*	next = new CRefIndex(_State.Id);
		*next = _Reference;
		next->buildNext();

		string		rootRefPath = _Reference.getRootPath();
		string		previousReferencePath = _Reference.getPath();
		string		nextReferencePath = next->getPath();

		string		logUpdatePath = _Reference.getLogPath();
		string		hoursUpdatePath = _Reference.getHoursUpdatePath();
		string		minutesUpdatePath = _Reference.getMinutesUpdatePath();
		string		secondsUpdatePath = _Reference.getSecondsUpdatePath();

		string		mintimestamp = start.toString();
		string		maxtimestamp = end.toString();
		string		keeptimestamp = keep.toString();

		CMessage&	msgref = CDbManager::addTask("RB_GEN_REF", this, (void*)next);
		msgref.serial(rootRefPath);
		msgref.serial(previousReferencePath);
		msgref.serial(nextReferencePath);
		msgref.serial(hoursUpdatePath);
		msgref.serial(minutesUpdatePath);
		msgref.serial(secondsUpdatePath);
		msgref.serial(logUpdatePath);
		msgref.serial(mintimestamp);
		msgref.serial(maxtimestamp);
		msgref.serial(keeptimestamp);

		_DayUpdateTimestamp = end;
	}

	return true;
}

/*
 * Task ran successfully
 */
void	CDatabase::taskSuccessful(void* arg)
{
	// when reference is up to date, notify new reference
	CRefIndex*	ref = (CRefIndex*)arg;

	_Reference = *ref;
	notifyNewReference(true);

	delete ref;
}

/*
 * Task failed!
 */
void	CDatabase::taskFailed(void* arg)
{
	// ok, nothing to do in this case...
}

/*
 * Static Check for update rates
 */
void	CDatabase::checkUpdateRates()
{
	uint	deltaRate = DeltaUpdateRate;
	uint	minuteRate = MinuteUpdateRate;

	// minute rate must be a multiple of delta rate
	if ((minuteRate % deltaRate) != 0)
	{
		minuteRate = deltaRate*(minuteRate/deltaRate + 1);
		nlwarning("CDatabase::checkUpdateRate(): MinuteUpdateRate is not a multiple of DeltaUpdateRate! Rounded to %d seconds", minuteRate);
		MinuteUpdateRate = minuteRate;
	}

	uint	hourRate = HourUpdateRate;

	// hour rate must be a multiple of minute rate
	if ((hourRate % minuteRate) != 0)
	{
		hourRate = minuteRate*(hourRate/minuteRate + 1);
		nlwarning("CDatabase::checkUpdateRate(): HourUpdateRate is not a multiple of MinuteUpdateRate! Rounded to %d seconds", hourRate);
		HourUpdateRate = hourRate;
	}

	uint	referenceRate = ReferenceUpdateRate;

	// reference rate must be a multiple of hour rate
	if ((referenceRate % hourRate) != 0)
	{
		referenceRate = hourRate*(referenceRate/hourRate + 1);
		nlwarning("CDatabase::checkUpdateRate(): ReferenceUpdateRate is not a multiple of HourUpdateRate! Rounded to %d seconds", referenceRate);
		ReferenceUpdateRate = referenceRate;
	}
}



/*
 * Serialise SheetId String Mapper
 */
void	CDatabase::serialSheetIdStringMapper(NLMISC::IStream& f)
{
	// serial mapper
	_SheetIdStringMapper.serial(f);

	if (f.isReading())
	{
		// if mapper is read, save it now in reference path
		std::string		refPath = _Reference.getPath();

		COFile	ofile;
		if (!ofile.open(refPath+"sheetid_map.bin"))
		{
			PDS_WARNING("serialSheetIdStringMapper(): failed to open reference sheetid_map file '%s' for save", (refPath+"sheetid_map.bin").c_str());
			return;
		}

		_SheetIdStringMapper.serial(ofile);
	}
}



/*
 * Get Table Index from name
 */
RY_PDS::TTableIndex	CDatabase::getTableIndex(const std::string& tableName) const
{
	const CTable*	table = getTable(tableName);
	return (table == NULL) ? RY_PDS::INVALID_TABLE_INDEX : (RY_PDS::TTableIndex)table->getId();
}

/*
 * Get Table Index from name
 */
std::string	CDatabase::getTableName(RY_PDS::TTableIndex index) const
{
	if (index >= _Tables.size() || _Tables[index] == NULL)
		return "invalid";

	return _Tables[index]->getName();
}




/*
 * Search object in database using its key
 * \param key is the 64 bits row key to search through all tables
 */
bool	CDatabase::searchObjectIndex(uint64 key, std::set<RY_PDS::CObjectIndex>& indexes) const
{
	indexes.clear();

	uint	i;
	for (i=0; i<_Tables.size(); ++i)
	{
		if (_Tables[i] == NULL || !_Tables[i]->initialised() || !_Tables[i]->isMapped())
			continue;

		RY_PDS::CObjectIndex	index = _Tables[i]->getMappedRow(key);

		if (index.isValid())
			indexes.insert(index);
	}

	return !indexes.empty();
}
