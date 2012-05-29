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

#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/entity_id.h>

#include <string.h>

#include "pds_table.h"
#include "pds_database.h"
//#include "db_file_stream.h"
#include "../pd_lib/db_delta_file.h"
#include "../pd_lib/db_description_parser.h"

using namespace std;
using namespace NLMISC;

/// Aligne a value on a boundary (if boundary is not a power of 2, rounded to next power of 2 boundary)
inline uint32		alignOnBoundary(uint32 value, uint32 boundary)
{
	// round to power of 2 boundary
	boundary = raiseToNextPowerOf2(boundary);

	// align
	return (value+boundary-1)&(~(boundary-1));
}


/*
 * Constructor
 */
CTable::CTable()
{
	clear();
}

/*
 * Destructor
 */
CTable::~CTable()
{
	//PDS_DEBUG("delete()");
	clear();
}

/*
 * Clear table
 */
void	CTable::clear()
{
	_Init = false;
	uint	i;

	// clear table buffer
	_TableBuffer.clear();

	// delete columns
	_Columns.clear();

	// delete attributes
	for (i=0; i<_Attributes.size(); ++i)
	{
		delete _Attributes[i];
		_Attributes[i] = NULL;
	}
	_Attributes.clear();

	_Parent = NULL;
	_Name = "";
	_Id = INVALID_TYPE_ID;
	_Key = INVALID_TYPE_ID;
	_Mapped = false;
	_RowSize = 0;
	_EmptyRow.clear();
}




/*
 * Init table
 */
bool	CTable::init(CDatabase *database, const CTableNode& table)
{
	// first a good clean up
	clear();

	// set parent logger
	setParentLogger(database);
	_TableBuffer.setParentLogger(this);

	_Parent = database;

	_Name = table.Name;
	_Id = table.Id;
	_Inheritance = table.Inherit;
	_Mapped = (table.Mapped != -1);
	_Key = table.Key;

	uint	i;

	for (i=0; i<table.Attributes.size(); ++i)
	{
		CAttribute*		attribute = new CAttribute();

		if (!attribute->init(database, this, table.Attributes[i]) || attribute->getId() != i)
			return false;

		_Attributes.push_back(attribute);
	}

	return true;
}


/*
 * Build columns
 */
bool	CTable::buildColumns()
{
	// do not compute twice
	if (!_Columns.empty())
		return true;

	uint	i;

	// build columns from attributes
	for (i=0; i<_Attributes.size(); ++i)
		if (!_Attributes[i]->buildColumns())
			return false;

	uint32	columnStart = 0;

	for (i=0; i<_Columns.size(); ++i)
	{
		_Columns[i].setByteOffset(columnStart);
		columnStart += _Columns[i].getByteSize();
	}

	_RowSize = columnStart;

	_TableBuffer.init(_Id, _RowSize, _Mapped);

	_EmptyRow.clear();
	_EmptyRow.resize(_RowSize, 0);

	for (i=0; i<_Columns.size(); ++i)
	{
		TDataType	type = _Columns[i].getDataType();
		void*		data = &_EmptyRow[_Columns[i].getByteOffset()];

		if (type == PDS_Index)
		{
			*(RY_PDS::CObjectIndex*)data = RY_PDS::CObjectIndex::null();
		}
		else if (type == PDS_List)
		{
			memset(data, 0, _Columns[i].getByteSize());
		}
	}

	_Init = true;

	return true;
}

/*
 * Post Init, called once all tables have been initialised
 */
bool	CTable::postInit()
{
	CTable*		root = getRootTable();

	if (root != this)
		_TableBuffer.linkRowMapper(&(root->_TableBuffer));

	uint	i;
	for (i=0; i<_Attributes.size(); ++i)
		if (!_Attributes[i]->computeBackRefKey())
			return false;

	return true;
}






/*
 * Display table
 */
void	CTable::display(NLMISC::CLog* log, bool expanded, bool displayHeader) const
{
	if (!initialised())
	{
		log->displayNL("** Table not initialised");
		return;
	}

	if (displayHeader)
	{
		log->displayNL("--------------------------------------------------------------------------------");
		log->displayNL("%-10s %-3s %-32s | %-3s | %-3s | %-4s | %-8s %-8s %8s |", "", "Id", "Name", "Inh", "Att", "Cols", "Allocs", "Loaded", "LoadedSz");
	}

	log->displayNL("%-10s %-3d %-32s | %-3d | %-3d | %-4d | %-8d %-8d %6dkb |", "Table", _Id, _Name.c_str(), _Inheritance, _Attributes.size(), _Columns.size(), _TableBuffer.numAllocated(), _TableBuffer.getLoadedRows(), (_TableBuffer.getMemoryLoad()+1023)/1024);

	uint	i;
	if (expanded)
	{
		log->displayNL("--------------------------------------------------------------------------------");
		log->displayNL("%-10s %-3s %-32s | %-10s | %-4s %-4s |", "", "Id", "Name", "MetaType", "From", "NCol");
		for (i=0; i<_Attributes.size(); ++i)
		{
			const CAttribute*	attrib = _Attributes[i];
			if (attrib == NULL || !attrib->initialised())
				log->displayNL("** Attribute %d not initialised", i);
			else
				log->displayNL("%-10s %-3d %-32s | %-10s | %-4d %-4d |", "Attribute", attrib->getId(), attrib->getName().c_str(), getNameFromMetaType(attrib->getMetaType()).c_str(), attrib->getOffset(), attrib->getColumns());
		}

		log->displayNL("--------------------------------------------------------------------------------");
		log->displayNL("%-10s %-3s %-64s | %-10s %-10s | %-10s", "", "Id", "Name", "MetaType", "DataType", "RowSz/Offs");
		for (i=0; i<_Columns.size(); ++i)
		{
			const CColumn&	column = _Columns[i];
			if (!column.initialised())
				log->displayNL("** Column %d not initialised", i);
			else
				log->displayNL("%-10s %-3d %-64s | %-10s %-10s | %1db at %-3d", "Column", column.getId(), column.getName().c_str(), getNameFromMetaType(column.getMetaType()).c_str(), getNameFromDataType(column.getDataType()).c_str(), column.getByteSize(), column.getByteOffset());
		}
	}
}


/*
 * Display row
 */
void	CTable::displayRow(RY_PDS::TRowIndex row, NLMISC::CLog* log, bool displayHeader)
{
	if (!initialised())
	{
		log->displayNL("** Table not initialised");
		return;
	}

	CTableBuffer::CAccessor	rowaccess = _TableBuffer.getRow(row);

	string	flagstr = toString("%s%s%s", 
		(rowaccess.allocated() ? "allocated" : "free"), 
		(rowaccess.mapped() ? ", mapped" : ""),
		(rowaccess.dirty() ? ", dirty" : ""));

	log->displayNL("row %d: %d bytes, flags=[%s] (map=%016"NL_I64"X, dirtstamp=%08X)", row, _RowSize, flagstr.c_str(), (rowaccess.mapped() ? rowaccess.key() : (uint64)0), rowaccess.dirtyStamp());

	if (displayHeader)
	{
		log->displayNL("%-10s %-3s %-64s | %-10s %-10s | %-9s | %-32s", "", "Id", "Name", "MetaType", "DataType", "Sz/Offs", "Value");
	}

	uint	i;
	for (i=0; i<_Columns.size(); ++i)
	{
		if (!_Columns[i].initialised())
		{
			log->displayNL("** Column %d not initialised", i);
			continue;
		}

		string			value;
		CDataAccessor	accessor(this, rowaccess, i);
		const CColumn	&col = _Columns[i];

		if (!accessor.isValid())
			value = "unaccessible value";
		else
			value = accessor.valueAsString();

		log->displayNL("%-10s %-3d %-64s | %-10s %-10s | %1db at %-3d | %-32s", 
			"Column", col.getId(), col.getName().c_str(),
			getNameFromMetaType(col.getMetaType()).c_str(), getNameFromDataType(col.getDataType()).c_str(),
			col.getByteSize(), col.getByteOffset(),
			value.c_str());
	}

	_TableBuffer.releaseRow(rowaccess);
}

/*
 * Dump Delta file content
 */
void	CTable::dumpDeltaFileContent(const std::string& filename, NLMISC::CLog* log) const
{
#ifdef DEBUG_DATA_ACCESSOR
	if (!initialised())
	{
		PDS_WARNING("dumpDeltaFileContent(): table not initialised");
		return;
	}

	CDBDeltaFile	delta;

	if (!_TableBuffer.setupDebugDeltaFile(filename, delta))
	{
		return;
	}

	uint8*	data;
	uint32	row;

	log->displayNL("%-10s %-3s %-64s | %-10s %-10s | %-9s | %-32s", "", "Id", "Name", "MetaType", "DataType", "Sz/Offs", "Value");

	while ((data = _TableBuffer.getDeltaRow(row, delta)) != NULL)
	{
		log->displayNL("row %d", row);

		CTable*	table = const_cast<CTable*>(this);
		CDataAccessor	accessor(table, data, 0);

		uint	i;
		for (i=0; i<_Columns.size(); ++i)
		{
			if (!_Columns[i].initialised())
			{
				log->displayNL("** Column %d not initialised", i);
				continue;
			}

			string			value;
			CDataAccessor	accessor(accessor, i);
			const CColumn	&col = _Columns[i];

			if (!accessor.isValid())
				value = "unaccessible value";
			else
				value = accessor.valueAsString();

			log->displayNL("%-10s %-3d %-64s | %-10s %-10s | %1db at %-3d | %-32s", 
				"Column", col.getId(), col.getName().c_str(),
				CType::getNameFromMetaType(col.getMetaType()).c_str(), CType::getNameFromDataType(col.getDataType()).c_str(),
				col.getByteSize(), col.getByteOffset(),
				value.c_str());
		}
	}
#endif
}



/*
 * Rebuild forwardrefs from backrefs
 */
bool	CTable::rebuildForwardRefs()
{
	if (!initialised())
	{
		PDS_WARNING("rebuildForwardRefs(): table not initialised");
		return false;
	}

	if (!fillRefInfo())
	{
		PDS_WARNING("rebuildForwardRefs(): failed to fillRefInfo()");
		return false;
	}

	if (BackRefInfo.empty() && ForwardRefInfo.empty())
		return true;

	if (!_TableBuffer.processRows(this))
	{
		PDS_WARNING("rebuildForwardRefs(): failed to processRows()");
		return false;
	}

	return true;
}

/*
 * Reset forwardrefs
 */
bool	CTable::resetForwardRefs()
{
	if (!initialised())
	{
		PDS_WARNING("resetForwardRefs(): table not initialised");
		return false;
	}

	return true;
}

/*
 * Reset table map
 */
bool	CTable::resetTableMap()
{
	if (!initialised())
	{
		PDS_WARNING("resetTableMap(): table not initialised");
		return false;
	}

	return true;
}

/*
 * Rebuild table map
 */
bool	CTable::rebuildTableMap()
{
	if (!initialised())
	{
		PDS_WARNING("rebuildTableMap(): table not initialised");
		return false;
	}

	if (!_Mapped)
		return true;

	return true;
}


/*
 * Allocate a row in a table
 * \param row is the row to allocate
 * Return true if succeded
 */
bool	CTable::allocate(RY_PDS::TRowIndex row, bool acquireRow)
{
	CTableBuffer::CAccessor	accessor;

	if (!_TableBuffer.allocate(row, accessor))
		return false;

	if (!accessor.allocated())
		return false;			// should not happen

	resetRow(accessor.data());

	// lock row if required
	if (acquireRow)
		_TableBuffer.acquireRow(accessor);

	return true;
}

/*
 * Deallocate a row in a table
 * \param row is the row to deallocate
 * Return true if succeded
 */
bool	CTable::deallocate(RY_PDS::TRowIndex row)
{
	return _TableBuffer.deallocate(row);
}

/*
 * Map a row
 * \param row is the row to allocate
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CTable::mapRow(const RY_PDS::CObjectIndex &index, uint64 key)
{
	if (!initialised())
	{
		PDS_WARNING("mapRow(): failed, table is not initialised");
		return false;
	}

	if (!_Mapped)
	{
		PDS_WARNING("mapRow(): failed, table is not mapped");
		return false;
	}

	if (!_TableBuffer.mapRow(index, key))
	{
		PDS_WARNING("mapRow(): failed to map '%s' to '%016"NL_I64"X'", index.toString().c_str(), key);
		return false;
	}

	PDS_FULL_DEBUG("Mapped '%s' to key '%016"NL_I64"X'", index.toString().c_str(), key);

	return true;
}

/*
 * Unmap a row in a table
 * \param tableIndex is the table to find row
 * \param key is the 64 bits row key
 * Return true if succeded
 */
bool	CTable::unmapRow(uint64 key)
{
	if (!initialised())
	{
		PDS_WARNING("unmapRow(): failed, table is not initialised");
		return false;
	}

	if (!_Mapped)
	{
		PDS_WARNING("unmapRow(): failed, table is not mapped");
		return false;
	}

	// get index
	RY_PDS::CObjectIndex	index = _TableBuffer.getMappedRow(key);

	if (index.isNull())
	{
		PDS_WARNING("unmapRow(): failed, inherited table is not initialised");
		return false;
	}

	if (!_TableBuffer.unmapRow(index, key))
	{
		PDS_WARNING("mapRow(): failed to unmap '%s' to '%016"NL_I64"X'", index.toString().c_str(), key);
		return false;
	}

	PDS_FULL_DEBUG("Unmapped '%s' of key '%016"NL_I64"X'", index.toString().c_str(), key);

	return true;
}

/*
 * Get a mapped row
 * \param key is the 64 bits row key
 * Return a valid TRowIndex if success
 */
RY_PDS::CObjectIndex	CTable::getMappedRow(uint64 key) const
{
	if (!initialised())
	{
		PDS_WARNING("getMappedRow(): failed, table is not initialised");
		return RY_PDS::CObjectIndex::null();
	}

	return _TableBuffer.getMappedRow(key);
}

/*
 * Release a row in a table
 * \param row is the row to release
 * \param timestamp is the current timestamp
 * Return true if succeded
 */
bool	CTable::release(RY_PDS::TRowIndex row)
{
	if (!initialised())
	{
		PDS_WARNING("release(): failed, table is not initialised");
		return false;
	}

	return _TableBuffer.releaseRow(row);
}

/*
 * Release all rows in table
 */
bool	CTable::releaseAll()
{
	return _TableBuffer.releaseAll();
}






/*
 * Set value
 */
bool	CTable::set(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint datasize, const void* dataptr)
{
	RY_PDS::CObjectIndex	object((RY_PDS::TTableIndex)_Id, row);

	// get an accessor on data
	CDataAccessor	accessor(_Parent, object, column);

	if (!accessor.isValid())
	{
		PDS_WARNING("set(): failed to get accessor on '%s'", accessor.getColumnIndex().toString().c_str());
		return false;
	}

	switch (accessor.column()->getMetaType())
	{

	case PDS_Type:
		// for simple type, easy money
		return accessor.setValue(dataptr, datasize);
		break;

	case PDS_BackRef:
		{
			// first unlink previous parent
			if (!unlink(accessor, object))
			{
				PDS_WARNING("set(): unable to unlink previous parent at '%s'", accessor.getColumnIndex().toString().c_str());
			}

			// then link new parent
			if (datasize != getStandardByteSize(PDS_Index))
			{
				PDS_WARNING("set(): unable to link new at '%s', provided bytesize is not standard", accessor.getColumnIndex().toString().c_str());
				return false;
			}

			// get parent index
			RY_PDS::CObjectIndex	parent = *(RY_PDS::CObjectIndex*)dataptr;

			// check checksum is valid
			if (!parent.isChecksumValid())
			{
				PDS_WARNING("set(): unable to link new parent '%s' at '%s', parent checksum is invalid", parent.toString().c_str(), accessor.getColumnIndex().toString().c_str());
				return false;
			}

			// if parent is invalid, then no need to link
			if (!parent.isValid())
				return true;

			// and link!
			if (!link(accessor, parent, object))
			{
				PDS_WARNING("set(): unable to link new parent '%s' at '%s'", parent.toString().c_str(), accessor.getColumnIndex().toString().c_str());
				return false;
			}
		}
		break;

	// other types MUST not be directly set
	default:
		PDS_WARNING("set(): column '%d' doesn't support set on '%s'", column, getNameFromMetaType(accessor.column()->getMetaType()).c_str());
		return false;
		break;
	}

	return true;
}

/**
 * Set Parent
 */
bool	CTable::setParent(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, const RY_PDS::CObjectIndex& parent)
{
	RY_PDS::CObjectIndex	object((RY_PDS::TTableIndex)_Id, row);

	// get an accessor on data
	CDataAccessor	accessor(_Parent, object, column);

	if (!accessor.isValid())
	{
		PDS_WARNING("set(): failed to get accessor on '%s'", accessor.getColumnIndex().toString().c_str());
		return false;
	}

	switch (accessor.column()->getMetaType())
	{

	case PDS_BackRef:
		{
			// first unlink previous parent
			if (!unlink(accessor, object))
			{
				PDS_WARNING("set(): unable to unlink previous parent at '%s'", accessor.getColumnIndex().toString().c_str());
			}

			// check checksum is valid
			if (!parent.isChecksumValid())
			{
				PDS_WARNING("set(): unable to link new parent '%s' at '%s', parent checksum is invalid", parent.toString().c_str(), accessor.getColumnIndex().toString().c_str());
				return false;
			}

			// if parent is invalid, then no need to link
			if (!parent.isValid())
				return true;

			// and link!
			if (!link(accessor, parent, object))
			{
				PDS_WARNING("set(): unable to link new parent '%s' at '%s'", parent.toString().c_str(), accessor.getColumnIndex().toString().c_str());
				return false;
			}
		}
		break;

	// other types MUST not be directly set
	default:
		PDS_WARNING("setParent(): column '%d' doesn't support set on '%s'", column, getNameFromMetaType(accessor.column()->getMetaType()).c_str());
		return false;
		break;
	}

	return true;
}


/**
 * link a BackRef to a parent
 * This method will perfom a full linking of child and new parent
 * \param backref is an accessor on the BackRef
 * \param parent is an index on the parent to link
 */
bool	CTable::link(CDataAccessor &backref, const RY_PDS::CObjectIndex &parent, const RY_PDS::CObjectIndex &child)
{
	// check ref is valid
	if (!backref.isValid())
	{
		PDS_WARNING("link(): failed, accessor is not valid");
		return false;
	}

	// check really a BackRef
	if (backref.attribute()->getMetaType() != PDS_BackRef || backref.column()->getMetaType() != PDS_BackRef)
	{
		PDS_WARNING("link(): failed, accessor is not a BackRef");
		return false;
	}

	if (!parent.isValid())
	{
		PDS_WARNING("link(): failed, parent has not a valid index");
		return false;
	}

	// link parent to child
	uint32			forwardrefid = backref.attribute()->getReferencedAttribute();
	CDataAccessor	parentref(_Parent, parent, forwardrefid, 0);

	if (!parentref.isValid())
	{
		PDS_WARNING("link(): failed, parent '%s' is not valid", parentref.toString().c_str());
		return false;
	}

	// check parent and child can really be linked
	// that is cross references match
	if (parentref.attribute()->getReferencedAttribute() != backref.attribute()->getId() ||
		backref.attribute()->getReferencedAttribute() != parentref.attribute()->getId())
	{
		PDS_WARNING("link(): failed, child '%s' and parent '%s' are not bound to be linked", backref.toString().c_str(), parentref.toString().c_str());
		return false;
	}

	forwardLink(backref, parentref, child);

	// set no ref index, with checksum validation
	return backref.setIndex(parent);
}



/*
 * link a ForwardRef to a child
 * This method will only link parent to child
 * \param forwardref is an accessor on the BackRef
 * \param child is an index on the child to link
 */
bool	CTable::forwardLink(CDataAccessor &backref, CDataAccessor &forwardref, const RY_PDS::CObjectIndex &child)
{
	if (!backref.isValid() || !forwardref.isValid())
	{
		PDS_WARNING("forwardLink(): failed, accessor is not valid");
		return false;
	}

	if (!child.isValid())
	{
		PDS_WARNING("forwardLink(): failed, child '%s' index is invalid", child.toString().c_str());
		return false;
	}

	switch (forwardref.attribute()->getMetaType())
	{
	case PDS_ArrayRef:
		// ArrayRef and ForwardRef set code is a bit shared as ArrayRef will only do
		// a little seek to the good column and set the index just like a ForwardRef
		// actually, datatypes are the same!
		if (!forwardref.seek(backref))
		{
			PDS_WARNING("forwardLink(): unable to seek to key of '%s'", child.toString().c_str());
			return false;
		}

	case PDS_ForwardRef:
		{
			RY_PDS::CObjectIndex	prevChild;

			// check no previous child
			if (!forwardref.getIndex(prevChild))
			{
				PDS_WARNING("forwardLink(): unable to access previous child of '%s'", forwardref.toString().c_str());
			}
			else if (prevChild.isValid())
			{
				PDS_WARNING("forwardLink(): '%s' has a previous child '%s'", forwardref.toString().c_str(), prevChild.toString().c_str());
			}

			// set child
			return forwardref.setIndex(child);
		}
		break;

	case PDS_Set:
		{
			if (!forwardref.checkType(child))
			{
				PDS_WARNING("forwardLink(): failed, '%s' is not of attribute '%s' type", child.toString().c_str(), forwardref.attribute()->getName().c_str());
				return false;
			}

			RY_PDS::CSetMap::CAccessor	setaccess = forwardref.getSet();

			if (!setaccess.isValid())
			{
				PDS_WARNING("forwardLink(): failed, parent '%s' set access is invalid", forwardref.toString().c_str());
				return false;
			}

			setaccess.add(child);
			// CHECK
			if (!setaccess.belongsTo(child))
			{
				PDS_WARNING("forwardLink(): failed, child '%s' doesn't belong to parent '%s' though it just had been added", child.toString().c_str(), forwardref.toString().c_str());
				return false;
			}
			// CHECK
			return true;
		}
		break;

	default:
		PDS_WARNING("forwardLink(): failed, can't link '%s'", getNameFromMetaType(forwardref.attribute()->getMetaType()).c_str());
		break;
	}

	return false;
}



/*
 * Unlink a BackRef
 * \param ref is an accessor on the BackRef
 * \param child is a remember of the child index
 */
bool	CTable::unlink(CDataAccessor &backref, const RY_PDS::CObjectIndex &child)
{
	// check ref is valid
	if (!backref.isValid())
	{
		PDS_WARNING("unlink(): failed, accessor is not valid");
		return false;
	}

	// check really a BackRef
	if (backref.attribute()->getMetaType() != PDS_BackRef || backref.column()->getMetaType() != PDS_BackRef)
	{
		PDS_WARNING("unlink(): failed, accessor is not a BackRef");
		return false;
	}

	RY_PDS::CObjectIndex	parent;

	// unlink parent
	if (!backref.getIndex(parent))
	{
		PDS_WARNING("unlink(): unable to get parent index");
	}
	else if (!parent.isChecksumValid())
	{
		PDS_WARNING("unlink(): parent index has invalid checksum");
	}
	else if (parent.isValid())
	{
		// only unlink if parent exists
		// because link will perform unlink before, and child may have no parent yet

		uint32			forwardrefid = backref.attribute()->getReferencedAttribute();
		CDataAccessor	parentref(_Parent, parent, forwardrefid, 0);

		forwardUnlink(backref, parentref, child);
	}

	// set no ref index, with checksum validation
	return backref.setIndex(RY_PDS::CObjectIndex::null());
}



/*
 * unlink a ForwardRef to a child
 * This method will only unlink parent to child
 * \param forwardref is an accessor on the BackRef
 * \param child is an index on the child to unlink
 */
bool	CTable::forwardUnlink(CDataAccessor &backref, CDataAccessor &forwardref, const RY_PDS::CObjectIndex &child)
{
	if (!forwardref.isValid())
	{
		PDS_WARNING("forwardUnlink(): failed, accessor is not valid");
		return false;
	}

	if (!child.isValid())
	{
		PDS_WARNING("forwardUnlink(): failed, child '%s' index is invalid", child.toString().c_str());
		return false;
	}

	switch (forwardref.attribute()->getMetaType())
	{
	case PDS_ArrayRef:
		// ArrayRef and ForwardRef set code is a bit shared as ArrayRef will only do
		// a little seek to the good column and set the index just like a ForwardRef
		// actually, datatypes are the same!
		if (!forwardref.seek(backref))
		{
			PDS_WARNING("forwardLink(): unable to seek to key of '%s'", child.toString().c_str());
			return false;
		}

	case PDS_ForwardRef:
		{
			RY_PDS::CObjectIndex	prevChild;

			// check child is the one we want to unlink
			if (!forwardref.getIndex(prevChild))
			{
				PDS_WARNING("forwardUnlink(): unable to access previous child of '%s'", forwardref.toString().c_str());
			}
			else if (!prevChild.isValid())
			{
				PDS_WARNING("forwardUnlink(): '%s' has no previous child '%s'", prevChild.toString().c_str(), forwardref.toString().c_str());
			}
			else if (prevChild != child)
			{
				PDS_WARNING("forwardUnlink(): failed, '%s' was not linked to '%s' but to '%s'", forwardref.toString().c_str(), child.toString().c_str(), prevChild.toString().c_str());
				return false;
			}

			// set child
			return forwardref.setIndex(RY_PDS::CObjectIndex::null());
		}
		break;

	case PDS_Set:
		{
			RY_PDS::CSetMap::CAccessor	setaccess = forwardref.getSet();

			if (!setaccess.isValid())
			{
				PDS_WARNING("forwardUnlink(): failed, parent '%s' set access is invalid", forwardref.toString().c_str());
				return false;
			}

			if (!setaccess.belongsTo(child))
			{
				PDS_WARNING("forwardUnlink(): failed, child '%s' doesn't belong to parent '%s'", child.toString().c_str(), forwardref.toString().c_str());
				return false;
			}
			setaccess.erase(child);
			if (setaccess.belongsTo(child))
			{
				PDS_WARNING("forwardUnlink(): failed, child '%s' still belongs to parent '%s' though it had been deleted", child.toString().c_str(), forwardref.toString().c_str());
				return false;
			}
			return true;
		}
		break;

	default:
		PDS_WARNING("forwardUnlink(): failed, can't unlink '%s'", getNameFromMetaType(forwardref.attribute()->getMetaType()).c_str());
		break;
	}

	return false;
}



/*
 * Set value
 */
bool	CTable::get(RY_PDS::TRowIndex row, RY_PDS::TColumnIndex column, uint& datasize, void* dataptr, TDataType& type)
{
	RY_PDS::CObjectIndex	object((RY_PDS::TTableIndex)_Id, row);

	// get an accessor on data
	CDataAccessor	accessor(_Parent, object, column);

	if (!accessor.isValid())
	{
		PDS_WARNING("get(): failed to get accessor on '%s'", accessor.getColumnIndex().toString().c_str());
		return false;
	}

	// get datatype
	type = accessor.column()->getDataType();

	switch (accessor.column()->getMetaType())
	{

	case PDS_Type:
		// for simple type, easy money
		return accessor.getValue(dataptr, datasize);
		break;

	case PDS_BackRef:
	case PDS_ForwardRef:
		{
			if (getStandardByteSize(type) > datasize)
			{
				PDS_WARNING("get(): databuffer too narrow to store column '%d' Index", column);
				return false;
			}
			datasize = sizeof(RY_PDS::CObjectIndex);
			return accessor.getIndex(*(RY_PDS::CObjectIndex*)dataptr);
		}
		break;

	case PDS_Set:
		{
			PDS_WARNING("get(): not supported for sets");
			return false;
		}
		break;

	// other types MUST not be directly set
	default:
		PDS_WARNING("get(): column '%d' doesn't support get on '%s'", column, getNameFromMetaType(accessor.column()->getMetaType()).c_str());
		return false;
		break;
	}

	return true;
}





/*
 * Perform column integrity check
 */
bool	CTable::CDataAccessor::check() const
{
	if (!isValid())
		return false;

#ifdef DEBUG_DATA_ACCESSOR
	if (_IsDebug)
		return true;
#endif


	switch (_Column->getMetaType())
	{
	case PDS_Type:

		return checkAsTypeAccessor();

		break;

	case PDS_BackRef:

		if (_Attribute->getMetaType() != PDS_BackRef)
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::check(): column '%s' MetaType incoherent with attribute's definition", _Column->getName().c_str());
			return false;
		}

		return checkAsRefAccessor();

		break;

	case PDS_ForwardRef:

		if (_Attribute->getMetaType() != PDS_ForwardRef &&
			_Attribute->getMetaType() != PDS_ArrayRef)
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::check(): column '%s' MetaType incoherent with attribute's definition", _Column->getName().c_str());
			return false;
		}

		return checkAsRefAccessor();

		break;

	case PDS_Set:

		return checkAsSetAccessor();

		break;
	}

	return false;
}


/*
 * Check an accessor as a PDS_Type accessor
 */
bool	CTable::CDataAccessor::checkAsTypeAccessor() const
{
	// check attribute
	if (_Attribute->getMetaType() != PDS_Type &&
		_Attribute->getMetaType() != PDS_Class &&
		_Attribute->getMetaType() != PDS_ArrayType &&
		_Attribute->getMetaType() != PDS_ArrayClass)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): column '%s' MetaType incoherent with attribute's definition", _Column->getName().c_str());
		return false;
	}

	if (!checkStrictDataType(_Column->getDataType()))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): column '%s' has not a strict valid datatype ('%s')", _Column->getName().c_str(), getNameFromDataType(_Column->getDataType()).c_str());
		return false;
	}

	if (_Column->getDataType() == PDS_enum || _Column->getDataType() == PDS_dimension)
	{
		const CType*	index = _Table->getParent()->getType(_Column->getTypeId());
		if (index == NULL)
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): couldn't find column '%s' original type in database", _Column->getName().c_str());
			return false;
		}

		if (!index->isIndex())
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): column '%s' original type is not an enum, whereas column says so", _Column->getName().c_str());
			return false;
		}

		uint32	value;

		if (!getAsIndexType(value))
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): couldn't getAsIndexValue() column '%s'", _Column->getName().c_str());
			return false;
		}

		if (value >= index->getIndexSize())
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsTypeAccessor(): column '%s' is out of enum '%s' range", _Column->getName().c_str(), index->getName().c_str());
			return false;
		}
	}

	return true;
}

/*
 * Check an accessor as a PDS_BackRef or PDS_ForwardRef accessor
 */
bool	CTable::CDataAccessor::checkAsRefAccessor() const
{
	if (_Column->getDataType() != PDS_Index)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsRefAccessor(): column '%s' DataType must be Index, found '%s'", _Column->getName().c_str(), getNameFromDataType(_Column->getDataType()).c_str());
		return false;
	}

	RY_PDS::CObjectIndex	ref;
	if (!getIndex(ref))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsRefAccessor(): can't get column '%s' Index", _Column->getName().c_str());
		return false;
	}

	if (!checkType(ref))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsRefAccessor(): column '%s' contains object '%s' of invalid type", _Column->getName().c_str(), ref.toString().c_str());
		return false;
	}

	if (!ref.isNull() && !_Table->getParent()->isAllocated(ref))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsRefAccessor(): column '%s' points to '%s' which is not allocated", _Column->getName().c_str(), ref.toString().c_str());
		return false;
	}

	return true;
}

/*
 * Check an accessor as a PDS_Set accessor
 */
bool	CTable::CDataAccessor::checkAsSetAccessor() const
{
	if (_Column->getDataType() != PDS_List)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsSetAccessor(): column '%s' DataType must be List, found '%s'", _Column->getName().c_str(), getNameFromDataType(_Column->getDataType()).c_str());
		return false;
	}

	RY_PDS::CSetMap::CAccessor	setaccess = getSet();

	if (!setaccess.isValid())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::checkAsSetAccessor(): set accessor to '%s' is invalid", toString().c_str());
		return false;
	}

	const RY_PDS::TIndexList&	list = setaccess.get();
	bool						success = true;

	uint	i;
	for (i=0; i<list.size(); ++i)
	{
		RY_PDS::CObjectIndex	index = list[i];

		if (!index.isValid())
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsSetAccessor(): column '%s' points to invalid object '%s'", _Column->getName().c_str(), index.toString().c_str());
			success = false;
			continue;
		}

		if (!_Table->getParent()->isAllocated(index))
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsSetAccessor(): column '%s' points to unallocated object '%s'", _Column->getName().c_str(), index.toString().c_str());
			success = false;
			continue;
		}

		if (!checkType(index))
		{
			PDS_WARNING_IN(_Table)("CDataAccessor::checkAsSetAccessor(): column '%s' points to object '%s' which is not of expected type", _Column->getName().c_str(), index.toString().c_str());
			success = false;
			continue;
		}
	}

	return success;
}





/*
 * Fetch a row into stream
 * Fetch the whole object arborescence (i.e. children linked to this object)
 * \param row is the row to fetch
 * \param data is the stream store data into
 */
bool	CTable::fetch(RY_PDS::TRowIndex row, RY_PDS::CPData &data, bool fetchIndex)
{
	uint	i;

	RY_PDS::TTableIndex		table = (RY_PDS::TTableIndex)_Id;
	RY_PDS::CObjectIndex	index(table, row);

	if (fetchIndex)
	{
		data.serial(table, row);
	}

	CDataAccessor	rowaccessor(_Parent, index, 0);

	for (i=0; i<_Columns.size(); ++i)
	{
		if (!_Columns[i].initialised())
		{
			PDS_WARNING("fetch(): failed, column '%d' not initialised", i);
			return false;
		}

		if (_Columns[i].getMetaType() == PDS_BackRef)
		{
			// backref are not sent, implicitely deduced
			continue;
		}

		CDataAccessor	accessor(rowaccessor, i);

		if (!accessor.isValid())
		{
			PDS_WARNING("fetch(): failed, can't create accessor on column '%d' for object '%s'", i, index.toString().c_str());
			return false;
		}

		if (!accessor.fetch(data))
			return false;

	}

	// lock row
	rowaccessor.acquire();

	return true;
}




/**
 * Fetch column data into stream
 */
bool	CTable::CDataAccessor::fetch(RY_PDS::CPData &data)
{
	if (!isValid())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor:fetch(): failed, fetch invalid accessor '%s'", toString().c_str());
		return false;
	}

	switch (_Column->getDataType())
	{
	case PDS_bool:
	case PDS_sint8:
	case PDS_uint8:
	case PDS_char:
		data.serial(*(uint8*)_Data);
		break;

	case PDS_ucchar:
	case PDS_uint16:
	case PDS_sint16:
		data.serial(*(uint16*)_Data);
		break;

	case PDS_uint32:
	case PDS_sint32:
	case PDS_float:
	case PDS_CSheetId:
	case PDS_CNodeId:
	case PDS_enum:
		data.serial(*(uint32*)_Data);
		break;

	case PDS_uint64:
	case PDS_sint64:
	case PDS_double:
	case PDS_CEntityId:
		data.serial(*(uint64*)_Data);
		break;

	case PDS_Index:
		{
			// check this is a forward ref
			// only forward ref are sent, backref are implicitely deduced
			if (_Column->getMetaType() == PDS_BackRef)
			{
				PDS_WARNING_IN(_Table)("CDataAccessor:fetch(): failed, try to serialize '%s' as backref", toString().c_str());
				return false;
			}

			RY_PDS::CObjectIndex	child = *(RY_PDS::CObjectIndex*)_Data;

			if (!child.isChecksumValid())
			{
				PDS_WARNING_IN(_Table)("CDataAccessor:fetch(): failed, '%s' points to invalid object '%s'", toString().c_str(), child.toString().c_str());
				return false;
			}

			if (child.isNull())
			{
				// send object id
				RY_PDS::TTableIndex		tid = RY_PDS::INVALID_TABLE_INDEX;
				RY_PDS::TRowIndex		rid = RY_PDS::INVALID_ROW_INDEX;
				data.serial(tid, rid);

				return true;
			}

			// send object and hierarchy
			return _Table->getParent()->fetch(child, data);
		}
		break;

	case PDS_List:
		{
			RY_PDS::CSetMap::CAccessor	setaccess = getSet();

			if (!setaccess.isValid())
			{
				PDS_WARNING_IN(_Table)("CDataAccessor:fetch(): failed, object '%s' set access is invalid", toString().c_str());
				return false;
			}

			const RY_PDS::TIndexList&	list = setaccess.get();

			uint	i;
			for (i=0; i<list.size(); ++i)
			{
				RY_PDS::CObjectIndex	child = list[i];

				// fetch object index
				RY_PDS::TTableIndex		tid = child.table();
				RY_PDS::TRowIndex		rid = child.row();
				data.serial(tid, rid);

				// fetch object key, for inset creation
				CDataAccessor			keyaccess(_Table->getParent(), child, _Attribute->getBackRefKey(), 0);
				if (!keyaccess.isValid())
				{
					PDS_WARNING_IN(_Table)("CDataAccessor:fetch(): failed, '%s' points to invalid object '%s'", toString().c_str(), child.toString().c_str());
					return false;
				}
				keyaccess.fetch(data);

				// force table not to fetch index, because we already done it
				_Table->getParent()->fetch(child, data, false);
			}

			RY_PDS::TTableIndex		tid = RY_PDS::INVALID_TABLE_INDEX;
			RY_PDS::TRowIndex		rid = RY_PDS::INVALID_ROW_INDEX;
			data.serial(tid, rid);

		}
		break;

	case PDS_dimension:
		if (_Column->getByteSize() == 1)
		{
			data.serial(*(uint8*)_Data);
		}
		else if (_Column->getByteSize() == 2)
		{
			data.serial(*(uint16*)_Data);
		}
		else
		{
			data.serial(*(uint32*)_Data);
		}
		break;
	}

	return true;
}



/*
 * Get value as string
 */
string	CTable::CDataAccessor::valueAsString(bool expandSet) const
{
	if (!isValid())
		return string("");

	std::string		value;

	switch (_Column->getDataType())
	{
	case PDS_bool:
		value = dataTypeAsString(*(bool*)_Data);
		break;

	case PDS_char:
		value = dataTypeAsString(*(char*)_Data);
		break;

	case PDS_uint8:
		value = dataTypeAsString(*(uint8*)_Data);
		break;

	case PDS_ucchar:
	case PDS_uint16:
		value = dataTypeAsString(*(uint16*)_Data);
		break;

	case PDS_CSheetId:
	case PDS_CNodeId:
	case PDS_uint32:
		value = dataTypeAsString(*(uint32*)_Data);
		break;

	case PDS_uint64:
		value = dataTypeAsString(*(uint64*)_Data);
		break;

	case PDS_sint8:
		value = dataTypeAsString(*(sint8*)_Data);
		break;

	case PDS_sint16:
		value = dataTypeAsString(*(sint16*)_Data);
		break;

	case PDS_sint32:
		value = dataTypeAsString(*(sint32*)_Data);
		break;

	case PDS_sint64:
		value = dataTypeAsString(*(sint64*)_Data);
		break;

	case PDS_float:
		value = dataTypeAsString(*(float*)_Data);
		break;

	case PDS_double:
		value = dataTypeAsString(*(double*)_Data);
		break;

	case PDS_CEntityId:
		value = dataTypeAsString(*(CEntityId*)_Data);
		break;

	case PDS_enum:
		{
			value = "'"+NLMISC::toString(*(uint32*)_Data)+"'";
			const CType*	type = _Table->getParent()->getType(_Column->getTypeId());
			if (type == NULL)
			{
				value += " <undisplayable type>";
			}
			else
			{
				value += " "+type->getName();
				if (type->isEnum())
				{
					value += " "+type->getIndexName(*(uint32*)_Data);
				}
				else
				{
					value += " not enum";
				}
			}

		}
		break;

	case PDS_dimension:
		{
			if (_Column->getByteSize() == 1)
				value = "'"+NLMISC::toString(*(uint8*)_Data)+"'";
			else if (_Column->getByteSize() == 2)
				value = "'"+NLMISC::toString(*(uint16*)_Data)+"'";
			else if (_Column->getByteSize() == 4)
				value = "'"+NLMISC::toString(*(uint32*)_Data)+"'";
			else
				value = "<undisplayable>";

			const CType*	type = _Table->getParent()->getType(_Column->getTypeId());
			if (type == NULL)
			{
				value += " <undisplayable type>";
			}
			else
			{
				value += " "+type->getName();
				if (type->isDimension())
				{
					value += " "+type->getIndexName(*(uint32*)_Data);
				}
				else
				{
					value += " not dimension";
				}
			}

		}
		break;

	case PDS_List:
		{
#ifdef DEBUG_DATA_ACCESSOR
			if (_IsDebug)
				value = "undisplayable set";
			else
			{
				const RY_PDS::TIndexList&	list = getSet().get();
				if (list.empty())
				{
					value = "Empty list";
				}
				else if (expandSet)
				{
					uint	i;
					value = "";
					for (i=0; i<list.size(); ++i)
						value += (list[i].toString()+" ");
				}
				else
				{
					value = NLMISC::toString(list.size())+" object(s)";
				}
			}
#else
			const RY_PDS::TIndexList&	list = getSet().get();
			if (list.empty())
			{
				value = "Empty list";
			}
			else
			{
				value = NLMISC::toString(list.size())+" object(s)";
			}
#endif
		}
		break;

	case PDS_Index:
		value = "'"+((RY_PDS::CObjectIndex*)_Data)->toString()+"'";
		break;

	default:
		value = "undisplayable value";
		break;
	}
	return value;
}



/*
 * Build the index allocator for this table
 */
bool	CTable::buildIndexAllocator(RY_PDS::CIndexAllocator& allocat)
{
	allocat.clear();

	uint	row;
	for (row=0; row<_TableBuffer.maxRowIndex(); ++row)
	{
		if (isAllocated(row))
		{
			allocat.forceAllocated(row);
		}
	}

	return true;
}



/*
 * Clear dirty list
 */
bool	CTable::clearDirtyList()
{
	if (!initialised())
	{
		PDS_WARNING("clearDirtyList(): table not initialised");
		return false;
	}

	_TableBuffer.resetDirty();

	return true;
}



/*
 * Reset dirty tags
 * Reset all rows so no one is marked as being dirty.
 * This method fixes broken list issues
 */
bool	CTable::resetDirtyTags()
{
	/// \todo fill here
	nlstop;

	if (!initialised())
	{
		PDS_WARNING("clearDirtyList(): table not initialised");
		return false;
	}

	return true;
}


/*
 * Preload reference files
 */
bool	CTable::preloadRefFiles()
{
	if (!_TableBuffer.openAllRefFilesRead())
		return false;



	return true;
}




/*
 * Notify new Reference, do necessary job...
 */
bool	CTable::notifyNewReference(CRefIndex& newref)
{
	if (!initialised())
		return false;

	_TableBuffer.setupRef(newref);

	return true;
}




/*
 * Apply delta changes from a file
 */
bool	CTable::applyDeltaChanges(const string& filename)
{
	if (!initialised())
	{
		PDS_WARNING("buildDelta(): failed, table not initialised");
		return false;
	}

	return _TableBuffer.applyDeltaChanges(filename);
}


/*
 * Reset row to initial value
 */
bool	CTable::resetRow(uint8* rowData)
{
	// copy row pattern
	memcpy(rowData, &(_EmptyRow[0]), _RowSize);

	return true;
}



/*
 * Build the delta file and purge all dirty rows in this table
 */
bool	CTable::buildDelta(const CTimestamp& starttime, const CTimestamp& endtime)
{
	H_AUTO(PDS_Table_buildDelta);

	if (!initialised())
	{
		PDS_WARNING("buildDelta(): failed, table not initialised");
		return false;
	}

	return _TableBuffer.buildDelta(starttime, endtime);
}

/*
 * Flush table from released rows
 */
bool	CTable::flushReleased()
{
	if (!initialised())
	{
		PDS_WARNING("flushReleased(): failed, table not initialised");
		return false;
	}

	_TableBuffer.flushReleased();
	return true;
}



/*
 * Build RowMapper
 */
bool	CTable::buildRowMapper()
{
	if (!initialised())
	{
		PDS_WARNING("buildRowMapper(): failed, table not initialised");
		return false;
	}

	return _TableBuffer.buildRowMapper();
}

/*
 * The process row callback, fix forwardrefs from backrefs
 */
bool	CTable::processRow(RY_PDS::TTableIndex table, CTableBuffer::CAccessor& accessor)
{
	if (table != _Id)
	{
		PDS_WARNING("processRow(): try to process a row from another table! (id '%d')", table);
		return false;
	}

	RY_PDS::CObjectIndex	child = RY_PDS::CObjectIndex(table, accessor.row());

	uint	i;
	for (i=0; i<BackRefInfo.size(); ++i)
	{
		// get backref accessor
		CBackRefFiller&			bref = BackRefInfo[i];
		CDataAccessor			backref(this, accessor, (RY_PDS::TColumnIndex)bref.Column->getId());
		RY_PDS::CObjectIndex	parent;

		if (!backref.isValid() || !backref.getIndex(parent))
		{
			PDS_WARNING("processRow(): failed to get BackRef index");
			return false;
		}

		// parent not set, do nothing
		if (!parent.isValid())
		{
			continue;
		}

		// get forwardref accessor
		CDataAccessor			forwardref(_Parent, parent, bref.Referenced->getId(), 0);

		switch (bref.Referenced->getMetaType())
		{
		case PDS_Set:
			processBackRefToSet(forwardref, child);
			break;

		case PDS_ArrayRef:
			// seek to pos in array
			if (!forwardref.seek(backref))
			{
				PDS_WARNING("processRow(): failed to seek into '%s'", forwardref.toString().c_str());
				return false;
			}

		case PDS_ForwardRef:
			if (!processBackRefToForwardRef(forwardref, child))
			{
				PDS_WARNING("processRow(): failed to processBackRefToForwardRef('%s', '%s')", forwardref.toString().c_str(), child.toString().c_str());
				return false;
			}
			break;

		default:
			break;
		}
	}

	for (i=0; i<ForwardRefInfo.size(); ++i)
	{
		CAutoForwardRefFiller&	fref = ForwardRefInfo[i];

		CDataAccessor			fwdref(this, accessor, (RY_PDS::TColumnIndex)fref.Column->getId());
		RY_PDS::CObjectIndex	child;

		if (!fwdref.isValid() || !fwdref.getIndex(child))
		{
			PDS_WARNING("processRow(): failed to get ForwardRef index");
			return false;
		}

		// should not be null and valid
		if (child.isValid() && !child.isNull())
			continue;

		// if potentially broken link
		// add to list of rows to fix, and leave
		BrokenForwardRefs.push_back(accessor.row());
		break;
	}

	return true;
}

/*
 * Process Back Reference to a set
 */
bool	CTable::processBackRefToSet(CDataAccessor& parent, RY_PDS::CObjectIndex child)
{
	parent.getSet().add(child);
	return true;
}

/*
 * Process Back Reference to a forward ref
 */
bool	CTable::processBackRefToForwardRef(CDataAccessor& parent, RY_PDS::CObjectIndex child)
{
	RY_PDS::CObjectIndex	checkchild;
	if (!parent.getIndex(checkchild))
	{
		PDS_WARNING("processBackRefToForwardRef(): failed to access to parent '%s'", parent.toString().c_str());
		return false;
	}

	if (checkchild != child)
		parent.setIndex(child);

	return true;
}


/*
 * Fill up Backward and Forward References information
 */
bool	CTable::fillRefInfo()
{
	uint	i;
	for (i=0; i<_Columns.size(); ++i)
	{
		CColumn&	column = _Columns[i];

		// generate summary of backward reference
		if (column.getMetaType() == PDS_BackRef)
		{
			CTable*	parent = _Parent->getNonConstTable((RY_PDS::TTableIndex)(column.getTypeId()));

			if (parent == NULL)
				return false;

			const CAttribute*	referenced = parent->getAttribute(column.getParent()->getReferencedAttribute());

			if (referenced == NULL)
				return false;

			CBackRefFiller		bref;

			bref.Column = &column;
			bref.Referenced = referenced;
			bref.ParentTable = parent;

			BackRefInfo.push_back(bref);
		}
		// generate summary of forward reference
		else if (column.getMetaType() == PDS_ForwardRef)
		{
			const CAttribute*	parent = column.getParent();

			// check column in array of ref that are allowed to contain null
			if (parent->getMetaType() == PDS_ArrayRef &&
				!parent->allowNull())
			{
				CAutoForwardRefFiller	fref;

				fref.Column = &column;
				ForwardRefInfo.push_back(fref);
			}
		}
	}

	return true;
}


/*
 * Fix broken forward refs
 */
bool	CTable::fixForwardRefs()
{
	if (BrokenForwardRefs.empty())
		return true;

	uint	i;
	for (i=0; i<BrokenForwardRefs.size(); ++i)
	{
		RY_PDS::TRowIndex		row = BrokenForwardRefs[i];

		if (!fixRowForwardRefs(row))
		{
			PDS_WARNING("fixForwardRefs(): failed to fix forward refs in row '%d' in '%s'", row, _Name.c_str());
		}
	}

	// clean up...
	BrokenForwardRefs.clear();

	return true;
}

/*
 * Fix row broken forward refs
 */
bool	CTable::fixRowForwardRefs(RY_PDS::TRowIndex row)
{
	// get row and examine all forward refs
	CTableBuffer::CAccessor	accessor = _TableBuffer.getRow(row);

	uint	j;
	for (j=0; j<ForwardRefInfo.size(); ++j)
	{
		CAutoForwardRefFiller	&colInfo = ForwardRefInfo[j];
		CDataAccessor			forwardref(this, accessor, (RY_PDS::TColumnIndex)colInfo.Column->getId());

		RY_PDS::CObjectIndex	index;

		if (!forwardref.isValid() || !forwardref.getIndex(index))
		{
			PDS_WARNING("fixRowForwardRefs(): failed to get forward ref '%s', data left as is", forwardref.toString().c_str());
			continue;
		}

		// was ref fixed?
		if (index.isValid())
			continue;

		// allocate new row
		CTable	*childTable = _Parent->getNonConstTable((RY_PDS::TTableIndex)(colInfo.Column->getTypeId()));
		if (childTable == NULL || !childTable->initialised())
		{
			PDS_WARNING("fixRowForwardRefs(): failed to get child table '%d'", colInfo.Column->getTypeId());
			continue;
		}

		RY_PDS::TRowIndex	alloc = childTable->nextUnallocatedRow();
		if (!childTable->allocate(alloc))
		{
			PDS_WARNING("fixRowForwardRefs(): failed to get allocate free row in table '%s'", childTable->getName().c_str());
			continue;
		}

		// get an accessor on row key
		CTableBuffer::CAccessor	allocAccess = childTable->_TableBuffer.getRow(alloc);
		CDataAccessor			keyAccess(childTable, allocAccess, (RY_PDS::TColumnIndex)(childTable->getAttribute(childTable->getKey())->getOffset()));

		if (!keyAccess.isValid())
		{
			PDS_WARNING("fixRowForwardRefs(): failed to get key access of row '%s:%d'", childTable->getName().c_str(), alloc);
			childTable->_TableBuffer.releaseRow(allocAccess);
			continue;
		}

		// compute key
		TEnumValue				keyValue = forwardref.column()->getId()-forwardref.attribute()->getOffset();

		// set new row key
		if (!keyAccess.setAsIndexType(keyValue))
		{
			PDS_WARNING("fixRowForwardRefs(): failed to set key '%s'", keyAccess.toString().c_str());
			childTable->_TableBuffer.releaseRow(allocAccess);
			continue;
		}

		CDataAccessor			backref(keyAccess, (RY_PDS::TColumnIndex)(childTable->getAttribute(forwardref.attribute()->getReferencedAttribute())->getOffset()));

		// set back&forward links
		forwardref.setIndex(backref.getObjectIndex());
		backref.setIndex(forwardref.getObjectIndex());

		// fixup links in new child
		if (!childTable->fixRowForwardRefs(alloc))
		{
			PDS_WARNING("fixRowForwardRefs(): failed to set fix forward refs in newly allocated row '%s'", backref.getObjectIndex().toString().c_str());
			childTable->_TableBuffer.releaseRow(allocAccess);
			continue;
		}

		childTable->_TableBuffer.releaseRow(allocAccess);
	}

	_TableBuffer.releaseRow(accessor);

	return true;
}





/*
 * Dump accessor content and info to xml
 */
void	CTable::CDataAccessor::dumpToXml(NLMISC::IStream& xml, sint expandDepth)
{
	if (xml.isReading())
		return;

	xml.xmlPushBegin("value");

	xml.xmlSetAttrib("valid");
	bool	valid = isValid();
	xml.serial(valid);

	bool	closeTag = true;

	if (valid)
	{
		xml.xmlSetAttrib("name");
		std::string		columnName = _Column->getName();
		xml.serial(columnName);

		xml.xmlSetAttrib("type");
		std::string		typeName = getNameFromDataType(_Column->getDataType());
		xml.serial(typeName);

		std::string		value;

		xml.xmlSetAttrib("value");

		switch (_Column->getDataType())
		{
		case PDS_bool:
			xml.serial(*(bool*)_Data);
			break;

		case PDS_char:
			xml.serial(*(char*)_Data);
			break;

		case PDS_uint8:
			xml.serial(*(uint8*)_Data);
			break;

		case PDS_ucchar:
		case PDS_uint16:
			xml.serial(*(uint16*)_Data);
			break;

		case PDS_CSheetId:
		case PDS_CNodeId:
		case PDS_uint32:
			xml.serial(*(uint32*)_Data);
			break;

		case PDS_uint64:
			xml.serial(*(uint64*)_Data);
			break;

		case PDS_sint8:
			xml.serial(*(sint8*)_Data);
			break;

		case PDS_sint16:
			xml.serial(*(sint16*)_Data);
			break;

		case PDS_sint32:
			xml.serial(*(sint32*)_Data);
			break;

		case PDS_sint64:
			xml.serial(*(sint64*)_Data);
			break;

		case PDS_float:
			xml.serial(*(float*)_Data);
			break;

		case PDS_double:
			xml.serial(*(double*)_Data);
			break;

		case PDS_CEntityId:
			{
				std::string	id = ((CEntityId*)_Data)->toString();
				xml.serial(id);
			}
			break;

		case PDS_enum:
			{
				std::string		value;
				const CType*	type = _Table->getParent()->getType(_Column->getTypeId());
				if (type == NULL || !type->isEnum())
				{
					xml.serial(*(uint32*)_Data);
				}
				else
				{
					std::string	name = type->getIndexName(*(uint32*)_Data);
					xml.serial(name);
				}

			}
			break;

		case PDS_dimension:
			{
				if (_Column->getByteSize() == 1)
				{
					xml.serial(*(uint8*)_Data);
				}
				else if (_Column->getByteSize() == 2)
				{
					xml.serial(*(uint16*)_Data);
				}
				else if (_Column->getByteSize() == 4)
				{
					xml.serial(*(uint32*)_Data);
				}
				else
				{
					std::string	unknown = "non displayable";
					xml.serial(unknown);
				}
			}
			break;

		case PDS_List:
			{
				std::string	value = "list";
				xml.serial(value);
				xml.xmlPushEnd();
				closeTag = false;

				if (expandDepth != 0)
				{
					const RY_PDS::TIndexList&	list = getSet().get();
					if (!list.empty())
					{
						uint	i;
						for (i=0; i<list.size(); ++i)
							_Table->_Parent->dumpToXml(list[i], xml, expandDepth-1);
					}
				}
			}
			break;

		case PDS_Index:
			{
				std::string	value = ((RY_PDS::CObjectIndex*)_Data)->toString(_Table->getParent());
				xml.serial(value);
				xml.xmlPushEnd();
				closeTag = false;

				if (expandDepth != 0 && _Attribute->getMetaType() != PDS_BackRef)
					_Table->_Parent->dumpToXml(*(RY_PDS::CObjectIndex*)_Data, xml, expandDepth-1);
			}
			break;

		default:
			{
				std::string	unknown = "non displayable";
				xml.serial(unknown);
			}
			break;
		}
	}

	if (closeTag)
		xml.xmlPushEnd();

	xml.xmlPop();
}


/*
 * Dump accessor content and info to xml
 */
void	CTable::dumpToXml(RY_PDS::TRowIndex row, NLMISC::IStream& xml, sint expandDepth)
{
	if (xml.isReading())
		return;

	xml.xmlPushBegin("object");

	xml.xmlSetAttrib("valid");
	bool	valid = initialised();
	xml.serial(valid);

	if (initialised())
	{
		xml.xmlSetAttrib("name");
		xml.serial(_Name);

		xml.xmlSetAttrib("index");
		RY_PDS::CObjectIndex	index((RY_PDS::TTableIndex)_Id, (RY_PDS::TRowIndex)row);
		std::string				indexName = index.toString(_Parent);
		xml.serial(indexName);

		CTableBuffer::CAccessor	rowaccess = _TableBuffer.getRow(row);

		bool	rowAllocated = rowaccess.allocated();
		xml.xmlSetAttrib("allocated");
		xml.serial(rowAllocated);

		xml.xmlPushEnd();

		if (rowAllocated)
		{
			uint	i;
			for (i=0; i<_Columns.size(); ++i)
			{
				CDataAccessor	accessor(this, rowaccess, i);
				const CColumn	&col = _Columns[i];

				accessor.dumpToXml(xml, expandDepth);
			}
		}

		_TableBuffer.releaseRow(rowaccess);
	}
	else
	{
		xml.xmlPushEnd();
	}

	xml.xmlPop();
}



