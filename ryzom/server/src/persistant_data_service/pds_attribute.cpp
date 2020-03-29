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

#include "pds_attribute.h"
#include "pds_type.h"
#include "pds_table.h"
#include "pds_database.h"
#include "../pd_lib/db_description_parser.h"

#include <nel/misc/debug.h>

using namespace std;
using namespace NLMISC;


/// Destructor
CAttribute::~CAttribute()
{
	//PDS_DEBUG("delete()");
}


/*
 * Initialize attribute using a full xml node
 */
bool	CAttribute::init(CDatabase *root, CTable* parent, const CAttributeNode& attribute)
{
	// set parent logger
	setParentLogger(parent);

	_Parent = parent;
	_Root = root;

	_Name = attribute.Name;
	_Id = attribute.Id;

	_MetaType = attribute.MetaType;
	_TypeId = attribute.TypeId;

	switch (_MetaType)
	{
	case PDS_Type:
		{
			const CType*	type = _Root->getType(_TypeId);
			if (type == NULL)
			{
				PDS_WARNING("init(): unknown type '%d'", _TypeId);
				return false;
			}

			_DataType = type->getDataType();
		}
		break;

	case PDS_Class:
		_DataType = PDS_UnknownDataType;
		break;

	case PDS_BackRef:
		_DataType = PDS_Index;
		_ReferencedAttribute = attribute.Reference;
		break;

	case PDS_ForwardRef:
		_DataType = PDS_Index;
		_ReferencedAttribute = attribute.Reference;
		break;

	case PDS_ArrayType:
		{
			const CType*	index = _Root->getType(attribute.Index);
			if (index == NULL || !index->isIndex())
			{
				PDS_WARNING("init(): type '%d' unknown or not an index", attribute.Index);
				return false;
			}

			_IndexId = attribute.Index;

			const CType*	type = _Root->getType(_TypeId);
			if (type == NULL)
			{
				PDS_WARNING("init(): unknown type '%d'", _TypeId);
				return false;
			}

			_DataType = type->getDataType();
		}
		break;

	case PDS_ArrayClass:
		{
			const CType*	type = _Root->getType(attribute.Index);
			if (type == NULL || !type->isIndex())
			{
				PDS_WARNING("init(): type '%d' unknown or not an index", attribute.Index);
				return false;
			}

			_IndexId = attribute.Index;
			_DataType = PDS_UnknownDataType;
		}
		break;

	case PDS_ArrayRef:
		{
			const CType*	type = _Root->getType(attribute.Index);
			if (type == NULL || !type->isIndex())
			{
				PDS_WARNING("init(): type '%d' unknown or not an index", attribute.Index);
				return false;
			}

			_IndexId = attribute.Index;
			_DataType = PDS_Index;
			_ReferencedAttribute = attribute.Reference;
			_AllowNull = attribute.AllowNull;
		}
		break;

	case PDS_Set:
		_DataType = PDS_List;
		_ReferencedAttribute = attribute.Reference;
		break;
	}

	_Init = true;

	return true;
}

/*
 * Build columns for this attribute
 */
bool	CAttribute::buildColumns()
{
	vector<CColumn>&	columns = _Parent->_Columns;

	_Offset = (uint32)columns.size();

	switch (_MetaType)
	{
	case PDS_Type:
		{
			CColumn	column;
			column._Parent = this;
			column._Root = _Root;
			column._Id = (uint32)columns.size();
			column._Name = _Name;
			column._MetaType = PDS_Type;
			column._DataType = _DataType;
			column._TypeId = _TypeId;
			const CType*	type = _Root->getType(_TypeId);
			if (type == NULL)
			{
				PDS_WARNING("init(): unknown type '%d'", _TypeId);
				return false;
			}
			column._ByteSize = type->getByteSize();
			column._Init = true;
			columns.push_back(column);
		}
		break;

	case PDS_Class:
		{
			CTable*	sub = const_cast<CTable*>(_Root->getTable(_TypeId));

			if (sub == NULL || !sub->buildColumns())
				return false;

			uint	i;
			for (i=0; i<sub->_Columns.size(); ++i)
			{
				CColumn	column = sub->_Columns[i];
				column._Parent = this;
				column._Root = _Root;
				column._Id = (uint32)columns.size();
				column._Name = _Name+"."+column._Name;
				column._Init = true;
				columns.push_back(column);
			}
		}
		break;

	case PDS_BackRef:
	case PDS_ForwardRef:
	case PDS_Set:
		{
			CColumn	column;
			column._Parent = this;
			column._Root = _Root;
			column._Id = (uint32)columns.size();
			column._Name = _Name;
			column._MetaType = _MetaType;
			column._DataType = _DataType;
			column._TypeId = _TypeId;
			column._ByteSize = getStandardByteSize(column._DataType);
			column._Init = true;
			columns.push_back(column);
		}
		break;

	case PDS_ArrayType:
		{
			const CType*	type = _Root->getType(_TypeId);
			const CType*	index = _Root->getType(_IndexId);

			if (type == NULL)
			{
				PDS_WARNING("init(): unknown type '%d'", _TypeId);
				return false;
			}

			if (index == NULL || !index->isIndex())
			{
				PDS_WARNING("buildColumns(): type '%d' unknown or not an index", _IndexId);
				return false;
			}

			uint	i;
			for (i=0; i<index->getIndexSize(); ++i)
			{
				CColumn	column;
				column._Parent = this;
				column._Root = _Root;
				column._Id = (uint32)columns.size();
				column._Name = _Name+"["+index->getIndexName(i)+"]";
				column._MetaType = PDS_Type;
				column._DataType = _DataType;
				column._TypeId = _TypeId;
				column._ByteSize = type->getByteSize();
				column._Init = true;
				columns.push_back(column);
			}
		}
		break;

	case PDS_ArrayClass:
		{
			CTable*			sub = const_cast<CTable*>(_Root->getTable(_TypeId));
			const CType*	index = _Root->getType(_IndexId);

			if (sub == NULL || sub == NULL || !sub->buildColumns())
			{
				PDS_WARNING("buildColumns(): unknown table '%d' or failed to build its columns", _TypeId);
				return false;
			}

			if (index == NULL || index == NULL || !index->isIndex())
			{
				PDS_WARNING("buildColumns(): type '%d' unknown or not an index", _IndexId);
				return false;
			}

			uint	i;
			for (i=0; i<index->getIndexSize(); ++i)
			{
				uint	j;
				for (j=0; j<sub->_Columns.size(); ++j)
				{
					CColumn	column = sub->_Columns[j];
					column._Parent = this;
					column._Root = _Root;
					column._Id = (uint32)columns.size();
					column._Name = _Name+"["+index->getIndexName(i)+"]"+"."+column._Name;
					column._Init = true;
					columns.push_back(column);
				}
			}
		}
		break;

	case PDS_ArrayRef:
		{
			const CType*	index = _Root->getType(_IndexId);

			if (index == NULL || !index->isIndex())
			{
				PDS_WARNING("buildColumns(): type '%d' unknown or not an index", _IndexId);
				return false;
			}

			uint	i;
			for (i=0; i<index->getIndexSize(); ++i)
			{
				CColumn	column;
				column._Parent = this;
				column._Root = _Root;
				column._Id = (uint32)columns.size();
				column._Name = _Name+"["+index->getIndexName(i)+"]";;
				column._MetaType = PDS_ForwardRef;
				column._DataType = _DataType;
				column._TypeId = _TypeId;
				column._ByteSize = getStandardByteSize(column._DataType);
				column._Init = true;
				columns.push_back(column);
			}
		}
		break;

	default:
		PDS_WARNING("buildColumns(): attribute '%s' metatype is unknown", _Name.c_str());
		return false;
		break;
	}

	_Columns = (uint32)columns.size() - _Offset;

	return true;
}


/*
 * Compute back reference key
 */
bool	CAttribute::computeBackRefKey()
{
	if (_MetaType == PDS_BackRef)
	{
		const CTable*		parentTable = _Root->getTable(_TypeId);
		const CAttribute*	parentAttribute = parentTable->getAttribute(_ReferencedAttribute);

		// should check parentAttribute is a forward reference of any kind

		const CTable*		childTable = _Root->getTable(parentAttribute->getTypeId());

		_Key = childTable->getKey();
	}
	else if (_MetaType == PDS_ArrayRef || _MetaType == PDS_Set)
	{
		const CTable*		childTable = _Root->getTable(_TypeId);
		const CAttribute*	childAttribute = childTable->getAttribute(_ReferencedAttribute);

		if (childAttribute == NULL)
		{
			PDS_WARNING("computeBackRefKey(): failed, child backref not initialised");
			return false;
		}

		if (childAttribute->getMetaType() != PDS_BackRef)
		{
			PDS_WARNING("computeBackRefKey(): failed, child backref actually not a backref");
			return false;
		}

		_Key = childTable->getKey();
		if (_Key == INVALID_TYPE_ID)
		{
			PDS_WARNING("computeBackRefKey(): failed, child key is invalid");
			return false;
		}

		const CAttribute*	keyAttribute = childTable->getAttribute(_Key);

		if (keyAttribute == NULL)
		{
			PDS_WARNING("computeBackRefKey(): failed, child key '%d' attribute cannot be found", _Key);
			return false;
		}

		if (_Key != childAttribute->getBackRefKey())
		{
			PDS_WARNING("computeBackRefKey(): failed, child backref key '%d' mismatch child key '%d'", childAttribute->getBackRefKey(), _Key);
			return false;
		}
	}

	return true;
}


