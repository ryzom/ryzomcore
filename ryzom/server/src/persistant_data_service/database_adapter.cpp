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

#include "database_adapter.h"

#include "db_manager.h"
#include "pds_type.h"
#include "pds_table.h"

using namespace std;
using namespace NLMISC;

/*
 * Constructor
 */
CDatabaseAdapter::CDatabaseAdapter()
{
	From = NULL;
	Into = NULL;
}


/*
 * Build the adapter using 2 database description
 * \param from is the old database to adapt from
 * \param into is the new database to adapt into
 */
bool	CDatabaseAdapter::build(CDatabase* from, CDatabase* into)
{
	bool	success = true;
	uint	i;

	vector<bool>	keptTypes;
	keptTypes.resize(from->_Types.size(), false);

	// setup table for remapping
	EnumRemap.resize(from->_Types.size());

	// first check types match
	for (i=0; i<into->_Types.size(); ++i)
	{
		const CType*	intoType = into->_Types[i];
		const CType*	fromType = from->getType(intoType->getName());

		// no fromType matches, type is new
		if (fromType == NULL)
		{
			nldebug("CDatabaseAdapter::build(): added type '%s' to database '%s'", intoType->getName().c_str(), into->getName().c_str());
			continue;
		}

		// flag type has kept
		keptTypes[fromType->getId()] = true;

		if (!checkDataTypeCompatible(fromType->getDataType(), intoType->getDataType()))
		{
			success = false;
			nlwarning("CDatabaseAdapter::build(): database '%s', can't convert from type '%s' (%s) into '%s' (%s)", 
				into->getName().c_str(),
				fromType->getName().c_str(), getNameFromDataType(fromType->getDataType()).c_str(),
				intoType->getName().c_str(), getNameFromDataType(intoType->getDataType()).c_str());
		}

		// build enum remapping
		if (fromType->isEnum() && intoType->isEnum())
		{
			if (!buildEnumRemapping(fromType, intoType))
			{
				nlwarning("CDatabaseAdapter::build(): failed to build enum remapper for '%s'", intoType->getName().c_str());
				return false;
			}
		}
	}

	// display dropped types
	for (i=0; i<keptTypes.size(); ++i)
		if (!keptTypes[i])
			nldebug("CDatabaseAdapter::build(): dropped type '%s' from database '%s'", from->getType(i)->getName().c_str(), from->getName().c_str());

	TableRemap.resize(from->_Tables.size(), INVALID_TYPE_ID);

	for (i=0; i<into->_Tables.size(); ++i)
	{
		CTable*			intoTable = into->_Tables[i];
		const CTable*	fromTable = from->getTable(intoTable->getName());

		CTableAdapter	adapter;

		adapter.From = fromTable;
		adapter.Into = intoTable;

		Tables.push_back(adapter);

		if (fromTable == NULL)
		{
			nldebug("CDatabaseAdapter::build(): added table '%s' to database '%s'", intoTable->getName().c_str(), into->getName().c_str());
			continue;
		}

		// remap old table to new
		TableRemap[fromTable->getId()] = i;

		if (!buildTableAdapter(fromTable, intoTable, Tables.back()))
		{
			nlwarning("CDatabaseAdapter::build(): failed to	adapt table '%s' in database '%s'", intoTable->getName().c_str(), into->getName().c_str());
			success = false;
		}
	}

	// display dropped types
	for (i=0; i<TableRemap.size(); ++i)
		if (TableRemap[i] == INVALID_TYPE_ID)
			nldebug("CDatabaseAdapter::build(): dropped table '%s' from database '%s'", from->getTable(i)->getName().c_str(), from->getName().c_str());

	if (!success)
	{
		nlwarning("CDatabaseAdapter::build(): failed to build adapter from database '%s' into '%s'", from->getName().c_str(), into->getName().c_str());
	}
	else
	{
		From = from;
		Into = into;
	}

	return success;
}


/*
 * Build adapter for 2 tables
 */
bool	CDatabaseAdapter::buildTableAdapter(const CTable* from, const CTable* into, CTableAdapter& adapter)
{
	bool	success = true;

	vector<bool>	keptColumns;
	keptColumns.resize(from->_Columns.size(), false);

	uint	i;
	for (i=0; i<into->_Columns.size(); ++i)
	{
		// get 'into' column path
		const CColumn*	intoColumn = &(into->_Columns[i]);
		std::string		intoColumnName = intoColumn->getName();
		CLocatePath		intoColumnPath;
		if (!CDbManager::parsePath(intoColumnName, intoColumnPath))
		{
			success = false;
			nlwarning("CDatabaseAdapter::buildTableAdapter(): failed to build CLocatePath for column '%s' in table '%s'", intoColumnName.c_str(), into->getName().c_str());
			continue;
		}

		// try to get 'from' column from path
		const CColumn*	fromColumn = from->getColumn(intoColumnPath, false);

		if (!intoColumnPath.end())
		{
			nldebug("CDatabaseAdapter::buildTableAdapter(): get column '%s' incomplete in table '%s', attribute dropped?", intoColumnName.c_str(), into->getName().c_str());
			continue;
		}

		CColumnAdapter	coladapter;

		coladapter.From = fromColumn;
		coladapter.Into = intoColumn;

		adapter.Columns.push_back(coladapter);

		if (fromColumn == NULL)
		{
			nldebug("CDatabaseAdapter::buildTableAdapter(): added column '%s' to table '%s'", intoColumnName.c_str(), into->getName().c_str());
			continue;
		}

		// check column types are compatible
		if (!checkDataTypeCompatible(fromColumn->getDataType(), intoColumn->getDataType()))
		{
			success = false;
			nlwarning("CDatabaseAdapter::buildTableAdapter(): type in column '%s' of table '%s' incompatible (forbidden cast from '%s' to '%s')",
				intoColumn->getName().c_str(),
				into->getName().c_str(),
				getNameFromDataType(fromColumn->getDataType()).c_str(),
				getNameFromDataType(intoColumn->getDataType()).c_str());
		}

		keptColumns[coladapter.From->getId()] = true;
	}

	// display dropped types
	for (i=0; i<keptColumns.size(); ++i)
		if (!keptColumns[i])
			nldebug("CDatabaseAdapter::buildTableAdapter(): dropped column '%s' from table '%s'", from->getColumn(i)->getName().c_str(), from->getName().c_str());

	return success;
}



/*
 * Build Enum Remapping
 */
bool	CDatabaseAdapter::buildEnumRemapping(const CType* fromType, const CType* intoType)
{
	const std::map<std::string, TEnumValue>&	oldmap = fromType->getEnumMap();
	vector<vector<string> >						oldmapease;

	// build map ease, back remapping for old map...
	std::map<std::string, TEnumValue>::const_iterator	it;
	for (it=oldmap.begin(); it!=oldmap.end(); ++it)
	{
		const string	&str = (*it).first;
		TEnumValue		val = (*it).second;
		if (val >= oldmapease.size())
			oldmapease.resize(val+1);
		// map string to value
		oldmapease[val].push_back(str);
	}

	TEnumRemap&	remap = EnumRemap[fromType->getId()];
	remap.resize(oldmapease.size(), INVALID_ENUM_VALUE);

	// and remap old values to new ones
	TEnumValue	value;
	for (value=0; value<oldmapease.size(); ++value)
	{
		if (oldmapease[value].empty())
		{
			nlwarning("CDatabaseAdapter::buildEnumRemapping(): internal error, type '%s', has undefined value '%d'", intoType->getName().c_str(), value);
			return false;
		}

		TEnumValue	mapvalue = INVALID_ENUM_VALUE;
		uint		teststr;
		for (teststr=0; teststr<oldmapease[value].size(); ++teststr)
		{
			TEnumValue	v = intoType->getIndexValue(oldmapease[value][teststr]);
			if (v == INVALID_ENUM_VALUE)
				continue;	// string is no longer mapped

			if (mapvalue != INVALID_ENUM_VALUE && v != mapvalue)
			{
				nlwarning("CDatabaseAdapter::buildEnumRemapping(): in type '%s', value '%s' does not belong anymore to the same group", intoType->getName().c_str(), oldmapease[value][teststr].c_str());
				return false;
			}

			mapvalue = v;
		}

		// remap 'value' to 'mapvalue'
		// if mapvalue == INVALID_ENUM_VALUE, value does no longer exist
		remap[value] = mapvalue;
	}

	return true;
}





/*
 * Build new database content
 */
bool	CDatabaseAdapter::buildContent()
{
	uint	table;

	for (table=0; table<Tables.size(); ++table)
	{
		CTableAdapter&		adapter = Tables[table];

		CTable*				from = const_cast<CTable*>(adapter.From);
		CTable*				into = adapter.Into;

		CTableBuffer&		frombuffer = from->_TableBuffer;
		CTableBuffer&		intobuffer = into->_TableBuffer;

		RY_PDS::TRowIndex	row;
		for (row=0; row<from->_TableBuffer.maxRowIndex(); ++row)
		{
			if (!frombuffer.isAllocated(row))
				continue;

			// get source accessor
			const CTableBuffer::CAccessor	fromrow = frombuffer.getRow(row);

			// get destination accessor
			CTableBuffer::CAccessor			intorow = intobuffer.getEmptyRow(row);

			// copy header
			// WARNING: intobuffer is not updated!!
			// -> key map is not set
			// -> allocated flag is not set
			intorow.copyHeader(fromrow);

			// reset destination row buffer
			into->resetRow(intorow.data());

			uint	column;
			for (column=0; column<adapter.Columns.size(); ++column)
			{
				// adapt column content
				CColumnAdapter&	coladapter = adapter.Columns[column];

				if (!buildColumnContent(coladapter, fromrow.data(), intorow.data()))
				{
					nlwarning("CDatabaseAdapter::buildContent(): failed to build content of column '%s' of '%s:%d'", coladapter.From->getName().c_str(), from->getName().c_str(), row);
					return false;
				}
			}

			/// \todo add some checks here

			// write row to ref file
			intobuffer.updateRow(intorow);
			// then release it (row is released from memory as it is clean)
			intobuffer.releaseRow(intorow);

			// release row from memory, as row was kept clean
			frombuffer.releaseRow(fromrow);

		}
	}

	return true;
}


/*
 * Build column content
 */
bool	CDatabaseAdapter::buildColumnContent(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	// nothing to adapt from, column left as is, assumed column had been reset before
	if (adapter.From == NULL)
		return true;

	const uint8*	fromObject = from + adapter.From->getByteOffset();
	uint8*			intoObject = into + adapter.Into->getByteOffset();

	// should never happen!
	if (!checkDataTypeCompatible(adapter.From->getDataType(), adapter.Into->getDataType()))
		return false;

	switch (adapter.Into->getDataType())
	{
	case PDS_bool:		return adaptBool(adapter, fromObject, intoObject);		break;
	case PDS_char:		return adaptChar(adapter, fromObject, intoObject);		break;
	case PDS_ucchar:	return adaptUcchar(adapter, fromObject, intoObject);	break;
	case PDS_uint8:		return adaptUint8(adapter, fromObject, intoObject);		break;
	case PDS_uint16:	return adaptUint16(adapter, fromObject, intoObject);	break;
	case PDS_uint32:	return adaptUint32(adapter, fromObject, intoObject);	break;
	case PDS_uint64:	return adaptUint64(adapter, fromObject, intoObject);	break;
	case PDS_sint8:		return adaptSint8(adapter, fromObject, intoObject);		break;
	case PDS_sint16:	return adaptSint16(adapter, fromObject, intoObject);	break;
	case PDS_sint32:	return adaptSint32(adapter, fromObject, intoObject);	break;
	case PDS_sint64:	return adaptSint64(adapter, fromObject, intoObject);	break;
	case PDS_float:		return adaptFloat(adapter, fromObject, intoObject);		break;
	case PDS_double:	return adaptDouble(adapter, fromObject, intoObject);	break;
	case PDS_CSheetId:	return adaptCSheetId(adapter, fromObject, intoObject);	break;

	/// \todo Uncomment this when *nodeid* type ready
	//case PDS_CNodeId:	return adaptCNodeId(adapter, fromObject, intoObject);	break;

	case PDS_CEntityId:	return adaptCEntityId(adapter, fromObject, intoObject);	break;
	case PDS_enum:		return adaptEnum(adapter, fromObject, intoObject);		break;
	case PDS_dimension:	return adaptDimension(adapter, fromObject, intoObject);	break;
	case PDS_Index:		return adaptIndex(adapter, fromObject, intoObject);		break;
	case PDS_List:		return adaptList(adapter, fromObject, intoObject);		break;
	}

	return false;
}

/*
 * Adaptation code
 * Yes, nasty macroing. Don't blame.
 */

#define ADAPT_SWITCH	\
	switch (adapter.From->getDataType())	\
	{	\
		default: break;

#define ADAPT_END		\
	}

// simple data cast
#define	ADAPT_DATA(typefrom, typeinto)	case PDS_##typefrom: (*(typeinto*)into) = (typeinto)(*(typefrom*)from); return true;

bool	CDatabaseAdapter::adaptBool(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(bool, bool)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptChar(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(char, char)
		ADAPT_DATA(ucchar, char)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptUcchar(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(char, ucchar)
		ADAPT_DATA(ucchar, ucchar)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptUint8(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, uint8)
		ADAPT_DATA(uint16, uint8)
		ADAPT_DATA(uint32, uint8)
		ADAPT_DATA(uint64, uint8)
		ADAPT_DATA(sint8, uint8)
		ADAPT_DATA(sint16, uint8)
		ADAPT_DATA(sint32, uint8)
		ADAPT_DATA(sint64, uint8)
		ADAPT_DATA(float, uint8)
		ADAPT_DATA(double, uint8)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptUint16(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, uint16)
		ADAPT_DATA(uint16, uint16)
		ADAPT_DATA(uint32, uint16)
		ADAPT_DATA(uint64, uint16)
		ADAPT_DATA(sint8, uint16)
		ADAPT_DATA(sint16, uint16)
		ADAPT_DATA(sint32, uint16)
		ADAPT_DATA(sint64, uint16)
		ADAPT_DATA(float, uint16)
		ADAPT_DATA(double, uint16)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptUint32(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, uint32)
		ADAPT_DATA(uint16, uint32)
		ADAPT_DATA(uint32, uint32)
		ADAPT_DATA(uint64, uint32)
		ADAPT_DATA(sint8, uint32)
		ADAPT_DATA(sint16, uint32)
		ADAPT_DATA(sint32, uint32)
		ADAPT_DATA(sint64, uint32)
		ADAPT_DATA(float, uint32)
		ADAPT_DATA(double, uint32)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptUint64(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, uint64)
		ADAPT_DATA(uint16, uint64)
		ADAPT_DATA(uint32, uint64)
		ADAPT_DATA(uint64, uint64)
		ADAPT_DATA(sint8, uint64)
		ADAPT_DATA(sint16, uint64)
		ADAPT_DATA(sint32, uint64)
		ADAPT_DATA(sint64, uint64)
		ADAPT_DATA(float, uint64)
		ADAPT_DATA(double, uint64)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptSint8(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, sint8)
		ADAPT_DATA(uint16, sint8)
		ADAPT_DATA(uint32, sint8)
		ADAPT_DATA(uint64, sint8)
		ADAPT_DATA(sint8, sint8)
		ADAPT_DATA(sint16, sint8)
		ADAPT_DATA(sint32, sint8)
		ADAPT_DATA(sint64, sint8)
		ADAPT_DATA(float, sint8)
		ADAPT_DATA(double, sint8)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptSint16(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, sint16)
		ADAPT_DATA(uint16, sint16)
		ADAPT_DATA(uint32, sint16)
		ADAPT_DATA(uint64, sint16)
		ADAPT_DATA(sint8, sint16)
		ADAPT_DATA(sint16, sint16)
		ADAPT_DATA(sint32, sint16)
		ADAPT_DATA(sint64, sint16)
		ADAPT_DATA(float, sint16)
		ADAPT_DATA(double, sint16)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptSint32(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, sint32)
		ADAPT_DATA(uint16, sint32)
		ADAPT_DATA(uint32, sint32)
		ADAPT_DATA(uint64, sint32)
		ADAPT_DATA(sint8, sint32)
		ADAPT_DATA(sint16, sint32)
		ADAPT_DATA(sint32, sint32)
		ADAPT_DATA(sint64, sint32)
		ADAPT_DATA(float, sint32)
		ADAPT_DATA(double, sint32)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptSint64(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, sint64)
		ADAPT_DATA(uint16, sint64)
		ADAPT_DATA(uint32, sint64)
		ADAPT_DATA(uint64, sint64)
		ADAPT_DATA(sint8, sint64)
		ADAPT_DATA(sint16, sint64)
		ADAPT_DATA(sint32, sint64)
		ADAPT_DATA(sint64, sint64)
		ADAPT_DATA(float, sint64)
		ADAPT_DATA(double, sint64)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptFloat(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, float)
		ADAPT_DATA(uint16, float)
		ADAPT_DATA(uint32, float)
		ADAPT_DATA(uint64, float)
		ADAPT_DATA(sint8, float)
		ADAPT_DATA(sint16, float)
		ADAPT_DATA(sint32, float)
		ADAPT_DATA(sint64, float)
		ADAPT_DATA(float, float)
		ADAPT_DATA(double, float)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptDouble(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(uint8, double)
		ADAPT_DATA(uint16, double)
		ADAPT_DATA(uint32, double)
		ADAPT_DATA(uint64, double)
		ADAPT_DATA(sint8, double)
		ADAPT_DATA(sint16, double)
		ADAPT_DATA(sint32, double)
		ADAPT_DATA(sint64, double)
		ADAPT_DATA(float, double)
		ADAPT_DATA(double, double)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptCSheetId(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(CSheetId, CSheetId)
	ADAPT_END
	return false;
}

/*
bool	CDatabaseAdapter::adaptCNodeId(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(CNodeId, CNodeId)
	ADAPT_END
	return false;
}
*/

bool	CDatabaseAdapter::adaptCEntityId(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	ADAPT_SWITCH
		ADAPT_DATA(CEntityId, CEntityId)
	ADAPT_END
	return false;
}

bool	CDatabaseAdapter::adaptEnum(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	const CType*	fromtype = From->getType(adapter.From->getTypeId());
	const CType*	intotype = Into->getType(adapter.Into->getTypeId());

	// fast remapping
	*(TEnumValue*)into = EnumRemap[fromtype->getId()][*(TEnumValue*)from];

	return true;
}

bool	CDatabaseAdapter::adaptDimension(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	const CType*	fromtype = From->getType(adapter.From->getTypeId());
	const CType*	intotype = Into->getType(adapter.Into->getTypeId());

	if (!fromtype->isDimension() || !intotype->isDimension())
		return false;

	uint32			value;

	if (fromtype->getByteSize() == 1)		value = *(uint8*)from;
	else if (fromtype->getByteSize() == 2)	value = *(uint16*)from;
	else if (fromtype->getByteSize() == 4)	value = *(uint32*)from;

	if (value >= intotype->getIndexSize())
		value = INVALID_INDEX_VALUE;

	if (intotype->getByteSize() == 1)		*(uint8*)into = (uint8)value;
	if (intotype->getByteSize() == 2)		*(uint16*)into = (uint16)value;
	if (intotype->getByteSize() == 4)		*(uint32*)into = (uint32)value;

	return true;
}

bool	CDatabaseAdapter::adaptIndex(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	RY_PDS::CObjectIndex	index = *(RY_PDS::CObjectIndex*)from;

	if (!index.isValid())
	{
		index = RY_PDS::CObjectIndex::null();
	}
	else
	{
		TTypeId	table = TableRemap[index.table()];

		// does table still exist?
		if (table == INVALID_TYPE_ID)
		{
			// no? remap object to null
			index = RY_PDS::CObjectIndex::null();
		}
		else
		{
			// yes? remap table in object
			index = RY_PDS::CObjectIndex((RY_PDS::TTableIndex)table, index.row());
		}
	}

	*(RY_PDS::CObjectIndex*)into = index;

	return true;
}

bool	CDatabaseAdapter::adaptList(const CColumnAdapter& adapter, const uint8* from, uint8* into)
{
	// list are always good, since they are rebuilt from backref
	return true;
}

