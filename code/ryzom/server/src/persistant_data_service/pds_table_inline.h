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
#error pds_table_inline.h MUST be included by pds_table.h
#endif

#ifndef RY_PDS_TABLE_INLINE_H
#define RY_PDS_TABLE_INLINE_H


//
// Inlines
//

/*
 * Constructor
 * Build a data accessor from a table, a row and an attribute id
 * This constructor doesn't support Class and ArrayClass attributes
 * \param table is the table peek/poke into
 * \param row is index to access
 * \param attribute is the attribute in the row
 * \param index is the array index if the attribute is an array, set to 0 for other type
 */
inline CTable::CDataAccessor::CDataAccessor(CDatabase* root, const RY_PDS::CObjectIndex& object, uint32 attribute, TEnumValue index)
{
	_IsValid = false;
	_Table = NULL;
	_Attribute = NULL;
	_Column = NULL;
	_Data = NULL;
#ifdef DEBUG_DATA_ACCESSOR
	_IsDebug = false;
#endif

	// check root and object
	if (root == NULL || !root->initialised() || !object.isValid())
	{
		return;
	}

	// init&check table
	_Table = root->getNonConstTable(object.table());
	if (_Table == NULL || !_Table->initialised())
	{
		return;
	}

	// init&check attribute
	_Attribute = _Table->getAttribute(attribute);
	if (_Attribute == NULL || !_Attribute->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, attribute '%d' not initialised", attribute);
		return;
	}

	// check attribute is not a Class or an ArrayClass
	switch (_Attribute->getMetaType())
	{
	case PDS_Type:
	case PDS_ForwardRef:
	case PDS_BackRef:
	case PDS_Set:
		// index should be 0
		if (index != 0)
			PDS_WARNING_IN(_Table)("CDataAccessor(): index in accessor is not 0, whereas attribute type is '%s', index forced to 0", getNameFromMetaType(_Attribute->getMetaType()).c_str());
		index = 0;
		break;

	case PDS_ArrayType:
	case PDS_ArrayRef:
		{
			// get index type
			const CType*	indexType = _Table->getParent()->getType(_Attribute->getIndexId());
			if (indexType == NULL)
			{
				PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, type '%d' is undefined", _Attribute->getIndexId());
				return;
			}

			// must be an enum
			if (!indexType->isIndex())
			{
				PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, type '%s' is not an enum", indexType->getName().c_str());
				return;
			}

			// check index fits enum
			if (index >= indexType->getIndexSize())
			{
				PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, index '%d' out of enum '%s' range", index, indexType->getName().c_str());
				return;
			}
		}
		break;

	default:
		// Class and ArrayClass not supported
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, '%s' not supported", getNameFromMetaType(_Attribute->getMetaType()).c_str());
		return;
		break;
	}

	// check index fits attribute number of columns
	if (index >= _Attribute->getColumns())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, index '%d' greater than attribute '%s' number of columns", index, _Attribute->getName().c_str());
		return;
	}

	// init&check column and data
	_Column = _Table->getColumn(_Attribute->getOffset() + index);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, column '%d' not initialised", _Attribute->getOffset()+index);
		return;
	}

	// check row allocated
	if (!_Table->_TableBuffer.isAllocated(object.row()))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, row is not allocated");
		return;
	}

	setupAccessor(object.row());
	_Data = _Accessor.data() + _Column->getByteOffset();

	// set as initialised
	_IsValid = true;

	// Final check to see if everything is clean
	if (!check())
	{
		invalidate();
	}
}


/*
 * Constructor
 * Build a data accessor from a table, a row and an column id
 * Please not that CDataAccessor doesn't support Class and ArrayClass attributes
 * \param table is the table peek/poke into
 * \param row is index to access
 * \param column in the row
 */
inline CTable::CDataAccessor::CDataAccessor(CDatabase* root, const RY_PDS::CObjectIndex& object, RY_PDS::TColumnIndex column)
{
	_IsValid = false;
	_Table = NULL;
	_Attribute = NULL;
	_Column = NULL;
	_Data = NULL;
#ifdef DEBUG_DATA_ACCESSOR
	_IsDebug = false;
#endif

	// check root and object
	if (root == NULL || !root->initialised() || !object.isValid())
	{
		return;
	}

	// init&check table
	_Table = root->getNonConstTable(object.table());
	if (_Table == NULL || !_Table->initialised())
	{
		return;
	}

	// init&check column
	_Column = _Table->getColumn(column);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, column '%d' not initialised", column);
		return;
	}

	// init&check attribute
	_Attribute = _Column->getParent();
	if (_Attribute == NULL || !_Attribute->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, parent attribute of column '%d' not initialised", column);
		return;
	}

	if (!_Table->_TableBuffer.isAllocated(object.row()))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, row is not allocated");
		return;
	}

	setupAccessor(object.row());
	_Data = _Accessor.data() + _Column->getByteOffset();

	// set as initialised
	_IsValid = true;

	// Final check to see if everything is clean
	if (!check())
	{
		invalidate();
	}
}

/*
 * Constructor
 * Build a data accessor from a table, a data accessor and a column index
 */
inline CTable::CDataAccessor::CDataAccessor(CTable* table, CTableBuffer::CAccessor& data, RY_PDS::TColumnIndex column)
{
	_IsValid = false;
	_Table = NULL;
	_Attribute = NULL;
	_Column = NULL;
	_Data = NULL;
#ifdef DEBUG_DATA_ACCESSOR
	_IsDebug = false;
#endif

	_Table = table;

	if (_Table == NULL || !_Table->initialised())
	{
		return;
	}

	// check root and object
	if (_Table->getParent() == NULL || !_Table->getParent()->initialised())
	{
		return;
	}

	// init&check column
	_Column = _Table->getColumn(column);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, column '%d' not initialised", column);
		return;
	}

	// init&check attribute
	_Attribute = _Column->getParent();
	if (_Attribute == NULL || !_Attribute->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, parent attribute of column '%d' not initialised", column);
		return;
	}

	_Accessor = data;
	_Table->_TableBuffer.acquireRow(_Accessor);
	_Data = _Accessor.data() + _Column->getByteOffset();

	// set as initialised
	_IsValid = true;

	// Final check to see if everything is clean
	if (!check())
	{
		invalidate();
	}
}

/*
 * Constructor
 * Build a data accessor from another accessor and a column index.
 * Used to access another field in a row
 */
inline CTable::CDataAccessor::CDataAccessor(const CDataAccessor& accessor, RY_PDS::TColumnIndex column)
{
	_IsValid = false;
	_Table = NULL;
	_Attribute = NULL;
	_Column = NULL;
	_Data = NULL;
#ifdef DEBUG_DATA_ACCESSOR
	_IsDebug = accessor._IsDebug;
#endif

	if (!accessor.isValid())
		return;

	_Table = accessor._Table;
	_Accessor = accessor._Accessor;

#ifdef DEBUG_DATA_ACCESSOR
	if (!_IsDebug)
#endif
		_Table->_TableBuffer.acquireRow(_Accessor);

	_IsValid = true;

	// init&check column
	_Column = _Table->getColumn(column);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, column '%d' not initialised", column);
		return;
	}

	// init&check attribute
	_Attribute = _Column->getParent();
	if (_Attribute == NULL || !_Attribute->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, parent attribute of column '%d' not initialised", column);
		return;
	}

#ifdef DEBUG_DATA_ACCESSOR
	if (_IsDebug)
		_Data = accessor._Data - accessor._Column->getByteOffset() + _Column->getByteOffset();
	else
#endif
		_Data = _Accessor.data() + _Column->getByteOffset();

	// Final check to see if everything is clean
	if (!check())
	{
		invalidate();
	}
}



#ifdef DEBUG_DATA_ACCESSOR
/*
 * Constructor
 * Build a **debug** data accessor from row data and a column index.
 */
inline CTable::CDataAccessor::CDataAccessor(CTable* table, uint8* data, RY_PDS::TColumnIndex column)
{
	_IsValid = false;
	_Table = NULL;
	_Attribute = NULL;
	_Column = NULL;
	_Data = NULL;
	_IsDebug = true;

	_Table = table;

	if (_Table == NULL || !_Table->initialised())
	{
		return;
	}

	// check root and object
	if (_Table->getParent() == NULL || !_Table->getParent()->initialised())
	{
		return;
	}

	// init&check column
	_Column = _Table->getColumn(column);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, column '%d' not initialised", column);
		return;
	}

	// init&check attribute
	_Attribute = _Column->getParent();
	if (_Attribute == NULL || !_Attribute->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor(): failed to create accessor, parent attribute of column '%d' not initialised", column);
		return;
	}

	_Data = data + _Column->getByteOffset();

	_IsValid = true;
}
#endif




/*
 * Destructor
 */
inline CTable::CDataAccessor::~CDataAccessor()
{
#ifdef DEBUG_DATA_ACCESSOR
	if (_IsValid && !_IsDebug)
		_Table->_TableBuffer.releaseRow(_Accessor);
#else
	if (_IsValid)
		_Table->_TableBuffer.releaseRow(_Accessor);
#endif
}


/*
 * Setup Row Accessor
 */
inline bool	CTable::CDataAccessor::setupAccessor(RY_PDS::TRowIndex row)
{
	_Accessor = _Table->_TableBuffer.getRow(row);
	return true;
}


/*
 * Seek to array index
 * \param seek index in the array previously setup
 * Return true if success, false if not and accessor is invalidated
 */
inline bool	CTable::CDataAccessor::seek(TEnumValue index)
{
	if (!isValid())
		return false;

	// invalidate
	_IsValid = false;

	if (_Attribute->getMetaType() != PDS_ArrayType && _Attribute->getMetaType() != PDS_ArrayRef)
	{
		return false;
	}

	// get index type
	const CType*	indexType = _Table->getParent()->getType(_Attribute->getIndexId());
	if (indexType == NULL)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to seek in array, type '%d' is undefined", _Attribute->getIndexId());
		return false;
	}

	// must be an enum
	if (!indexType->isIndex())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to seek in array, type '%s' is not an enum", indexType->getName().c_str());
		return false;
	}

	// check index fits enum
	if (index >= indexType->getIndexSize())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to seek in array, index '%d' out of enum '%s' range", index, indexType->getName().c_str());
		return false;
	}

	// check index fits attribute number of columns
	if (index >= _Attribute->getColumns())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to seek in array, index '%d' greater than attribute '%s' number of columns", index, _Attribute->getName().c_str());
		return false;
	}

	// init&check column and data
	_Column = _Table->getColumn(_Attribute->getOffset() + index);
	if (_Column == NULL || !_Column->initialised())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to seek in array, column '%d' not initialised", _Attribute->getOffset()+index);
		return false;
	}

	_Data = _Accessor.data() + _Column->getByteOffset();

	// revalidate
	_IsValid = true;

	// Final check to see if everything is clean
	if (!check())
	{
		invalidate();
	}

	return _IsValid;
}

/*
 * Seek to array index
 * \param backref is the back reference associated the current forward reference
 * Seek doesn't apply to ArrayClass attributes
 * Return true if success, false if not and accessor is invalidated
 */
inline bool	CTable::CDataAccessor::seek(CDataAccessor& backref)
{
	if (!isValid() || !backref.isValid())
		return false;

	uint32	key;
	if (!backref.getBackRefKey(key))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): failed to get key from object '%s'", backref.toString().c_str());
		return false;
	}

	// seek to good column in ArrayRef
	if (!seek(key))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::seek(): unable to seek to key '%d' of '%s'", key, backref.toString().c_str());
		return false;
	}

	return true;
}

/*
 * Invalidate accessor
 */
inline void	CTable::CDataAccessor::invalidate()
{
#ifdef DEBUG_DATA_ACCESSOR
	if (_IsValid && !_IsDebug)
	{
		_IsValid = false;
		_Table->_TableBuffer.releaseRow(_Accessor);
	}
#else
	if (_IsValid)
	{
		_IsValid = false;
		_Table->_TableBuffer.releaseRow(_Accessor);
	}
#endif
}


/*
 * Check type of an index
 */
inline bool	CTable::CDataAccessor::checkType(const RY_PDS::CObjectIndex &object) const
{
	if (!isValid())
		return false;

	// type checking only for references
	if (_Column->getMetaType() != PDS_BackRef &&
		_Column->getMetaType() != PDS_ForwardRef &&
		_Column->getMetaType() != PDS_Set)
		return false;

	if (!object.isChecksumValid())
		return false;

	// null is kind of void*
	if (object.isNull())
		return true;

	// get table id of object
	TTypeId	id = object.table();

	do
	{
		// if id matches, ok
		if (id == _Column->getTypeId())
			return true;

		// get inherited table
		const CTable*	table = _Table->getParent()->getTable(id);
		if (table == NULL)
			return false;
		id = table->getInheritTable();
	}
	while (id != INVALID_TYPE_ID);

	return false;
}


/*
 * Get data as CObjectIndex
 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an CObjectIndex)
 */
inline bool	CTable::CDataAccessor::getIndex(RY_PDS::CObjectIndex &index) const
{
	if (!isValid())
		return false;

	// check column&attribute define a reference (BackRef, ForwardRef or ArrayRef)
	if (_Attribute->getDataType() != PDS_Index ||
		_Column->getDataType() != PDS_Index ||
		_Column->getByteSize() != getStandardByteSize(PDS_Index))
		return false;

	// load index
	index = *((RY_PDS::CObjectIndex*)_Data);

	return true;
}

/*
 * Set data as CObjectIndex
 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an CObjectIndex)
 */
inline bool	CTable::CDataAccessor::setIndex(const RY_PDS::CObjectIndex &index)
{
	if (!isValid())
		return false;

	// check column&attribute define a reference (BackRef, ForwardRef or ArrayRef)
	if (_Attribute->getDataType() != PDS_Index ||
		_Column->getDataType() != PDS_Index ||
		_Column->getByteSize() != getStandardByteSize(PDS_Index))
		return false;

	if (!checkType(index))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::setIndex(): failed, '%s' is not of attribute '%s' type", index.toString().c_str(), _Attribute->getName().c_str());
		return false;
	}

	// store index
	*((RY_PDS::CObjectIndex*)_Data) = index;

	// always dirty row, even for forwardrefs
	// if not, forward refs may be lost, and reload
	// will generate a huge memory load, because all
	// rows will have to be reloaded (since all may be
	// different from what is stored in reference
	dirtyRow();

	return true;
}

/*
 * Get data as a Set
 * Return an accessor on a set, which is valid only if not issue occured
 * Thus, you are able to modify the list own your own, add/remove items...
 */
inline RY_PDS::CSetMap::CAccessor	CTable::CDataAccessor::getSet()
{
#ifdef DEBUG_DATA_ACCESSOR
	if (!isValid() || _IsDebug)
		return RY_PDS::CSetMap::CAccessor();
#else
	if (!isValid())
		return RY_PDS::CSetMap::CAccessor();
#endif

	// check column&attribute define a list (Set only)
	if (_Attribute->getMetaType() != PDS_Set ||
		_Attribute->getDataType() != PDS_List ||
		_Column->getDataType() != PDS_List ||
		_Column->getByteSize() != getStandardByteSize(PDS_List))
		return RY_PDS::CSetMap::CAccessor();

	return _Table->getParent()->getSetMap().get(getColumnIndex());
}

/*
 * Get data as a Set
 * Return an accessor on a set, which is valid only if not issue occured
 */
inline const RY_PDS::CSetMap::CAccessor	CTable::CDataAccessor::getSet() const
{
#ifdef DEBUG_DATA_ACCESSOR
	if (!isValid() || _IsDebug)
		return RY_PDS::CSetMap::CAccessor();
#else
	if (!isValid())
		return RY_PDS::CSetMap::CAccessor();
#endif

	// check column&attribute define a list (Set only)
	if (_Attribute->getMetaType() != PDS_Set ||
		_Attribute->getDataType() != PDS_List ||
		_Column->getDataType() != PDS_List ||
		_Column->getByteSize() != getStandardByteSize(PDS_List))
		return RY_PDS::CSetMap::CAccessor();

	return _Table->getParent()->getSetMap().get(getColumnIndex());
}

/*
 * Get data as simple type value
 * \param dataptr is the destination databuffer
 * \param datasize is the destination buffer size
 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an simple type)
 * If datasize doesn't match, data are loaded anyway, but shrinked or truncated, and a warning is issued
 */
inline bool	CTable::CDataAccessor::getValue(void* dataptr, uint32 datasize) const
{
	if (!isValid())
		return false;

	if ((_Attribute->getMetaType() != PDS_Type &&
		 _Attribute->getMetaType() != PDS_Class &&
		 _Attribute->getMetaType() != PDS_ArrayType &&
		 _Attribute->getMetaType() != PDS_ArrayClass) ||
		_Column->getMetaType() != PDS_Type || 
		!checkStrictDataType(_Column->getDataType()))
		return false;

	if (_Column->getByteSize() == datasize)
	{
		memcpy(dataptr, _Data, datasize);
	}
	else if (_Column->getByteSize() < datasize)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getValue(): attribute '%s' bytesize is less than desired, load is zero padded", _Attribute->getName().c_str());
		memset(dataptr, 0, datasize);
		memcpy(dataptr, _Data, _Column->getByteSize());
	}
	else
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getValue(): attribute '%s' bytesize is more than desired, load is truncated", _Attribute->getName().c_str());
		memcpy(dataptr, _Data, datasize);
	}

	return true;
}

/*
 * Set data as simple type value
 * \param dataptr is the source databuffer
 * \param datasize is the source buffer size
 * Return false if something went wrong (mainly, CDataAccessor is not valid, or pointed data is not an simple type)
 * If datasize doesn't match, data are stored anyway, but shrinked or truncated, and a warning is issued
 */
inline bool	CTable::CDataAccessor::setValue(const void* dataptr, uint32 datasize)
{
	if (!isValid())
		return false;

	if ((_Attribute->getMetaType() != PDS_Type && _Attribute->getMetaType() != PDS_Class && _Attribute->getMetaType() != PDS_ArrayType && _Attribute->getMetaType() != PDS_ArrayClass) ||
		_Column->getMetaType()!=PDS_Type || 
		!checkStrictDataType(_Column->getDataType()))
		return false;

	if (_Column->getByteSize() == datasize)
	{
		memcpy(_Data, dataptr, datasize);
	}
	else if (_Column->getByteSize() < datasize)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getValue(): attribute '%s' bytesize is less than desired, store is truncated", _Attribute->getName().c_str());
		memcpy(_Data, dataptr, _Column->getByteSize());
	}
	else
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getValue(): attribute '%s' bytesize is more than desired, store is zero padded", _Attribute->getName().c_str());
		memset(_Data, 0, _Column->getByteSize());
		memcpy(_Data, dataptr, datasize);
	}

	// mark row as modified
	dirtyRow();

	return true;
}

/*
 * Get as enum/dimension
 */
inline bool	CTable::CDataAccessor::getAsIndexType(uint32& value) const
{
	if (!isValid())
		return false;

	if (_Column->getDataType() != PDS_enum && _Column->getDataType() != PDS_dimension)
		return false;

	switch (_Column->getByteSize())
	{
	case 1:		value = *(uint8*)_Data;		break;
	case 2:		value = *(uint16*)_Data;	break;
	case 4:		value = *(uint32*)_Data;	break;
	default:	return false;				break;
	}

	return true;
}

/*
 * Set as enum/dimension
 */
inline bool	CTable::CDataAccessor::setAsIndexType(uint32 value)
{
	if (!isValid())
		return false;

	if (_Column->getDataType() != PDS_enum && _Column->getDataType() != PDS_dimension)
		return false;

	// check value fits index width
	const CType*	type = _Table->getParent()->getType(_Column->getTypeId());
	if (type == NULL)
		return false;

	if (value >= type->getIndexSize())
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::setAsIndexType(): value '%d' is beyond type '%s' limit", value, type->getName().c_str());
		return false;
	}

	switch (_Column->getByteSize())
	{
	case 1:		*(uint8*)_Data = (uint8)value;		break;
	case 2:		*(uint16*)_Data = (uint16)value;	break;
	case 4:		*(uint32*)_Data = (uint32)value;	break;
	default:	return false;						break;
	}

	return true;
}



/*
 * Get Back Ref Key value
 */
inline bool	CTable::CDataAccessor::getBackRefKey(uint32& key)
{
	// basic check
	if (!isValid() || _Attribute->getMetaType() != PDS_BackRef)
		return false;

	// get key associated to the backref -- may not exist if
	// backref is forwarded wy a simple ForwardRef (not a ArrayRef nor a Set)
	uint				keyAttribId = _Attribute->getBackRefKey();
	if (keyAttribId == INVALID_TYPE_ID)
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getBackRefKey(): failed");
		return false;
	}

	// get a data accessor on the key
	const CAttribute*	keyAttrib = _Table->getAttribute(keyAttribId);
	CDataAccessor		keyAccessor(*this, (RY_PDS::TColumnIndex)keyAttrib->getOffset());

	// and retrieve key value
	if (!keyAccessor.getAsIndexType(key))
	{
		PDS_WARNING_IN(_Table)("CDataAccessor::getBackRefKey(): failed");
		return false;
	}

	return true;
}


/*
 * Dirty row
 */
inline bool	CTable::CDataAccessor::dirtyRow()
{
	if (!isValid())
		return false;

	return _Table->_TableBuffer.dirtyRow(_Accessor);
}




/*
 * Acquire row
 */
inline void	CTable::CDataAccessor::acquire()
{
	if (isValid())
	{
		_Table->_TableBuffer.acquireRow(_Accessor);
	}
}

/*
 * Unacquire row
 */
inline void	CTable::CDataAccessor::unacquire()
{
	if (isValid())
	{
		_Table->_TableBuffer.releaseRow(_Accessor);
	}
}





/*
 * Get Root Inheritance Table
 */
inline CTable*	CTable::getRootTable()
{
	CTable	*root = this;

	while (root->_Inheritance != INVALID_TYPE_ID)
	{
		root = _Parent->getNonConstTable((RY_PDS::TTableIndex)root->_Inheritance);

		if (root == NULL)
		{
			PDS_WARNING("getRootTable(): inherited table is not initialised");
			return NULL;
		}
	}

	return root;
}

/*
 * Get Root Inheritance Table
 */
inline const CTable*	CTable::getRootTable() const
{
	const CTable	*root = this;

	while (root->_Inheritance != INVALID_TYPE_ID)
	{
		root = _Parent->getTable((RY_PDS::TTableIndex)root->_Inheritance);

		if (root == NULL)
		{
			PDS_WARNING("getRootTable(): inherited table is not initialised");
			return NULL;
		}
	}

	return root;
}


/*
 * Tells if a row is allocated
 * \param row is the row to check
 */
inline bool	CTable::isAllocated(RY_PDS::TRowIndex row) const
{
	return _TableBuffer.isAllocated(row);
}

/*
 * Get Attribute
 */
inline const CAttribute*	CTable::getAttribute(const std::string &name) const
{
	if (!initialised())
	{
		PDS_WARNING("getAttribute(): table not initialised");
		return NULL;
	}

	uint	i;
	for (i=0; i<_Attributes.size(); ++i)
		if (_Attributes[i] != NULL && _Attributes[i]->initialised() && _Attributes[i]->getName() == name)
			return _Attributes[i];

	return NULL;
}



/*
 * Get accessor on data from a path
 */
inline CTable::CDataAccessor	CTable::getAccessor(CLocatePath &path)
{
	if (path.end())
		return CDataAccessor();

	RY_PDS::TRowIndex	row;
	NLMISC::fromString(path.node().Name, row);
	path.next();

	const CColumn*	column = getColumn(path);

	if (column == NULL)
		return CDataAccessor();

	return CDataAccessor(_Parent, RY_PDS::CObjectIndex((RY_PDS::TTableIndex)_Id, row), (RY_PDS::TColumnIndex)column->getId());
}

/*
 * Get Column
 */
inline const CColumn*	CTable::getColumn(CLocatePath &path, bool verbose) const
{
	const CTable*	table = this;
	uint			currentColumn = 0;

	while (!path.end())
	{
		CLocatePath::CLocateAttributeNode	&node = path.node();
		path.next();

		const CAttribute*	attr = table->getAttribute(node.Name);
		if (attr == NULL)
		{
			if (verbose)
				PDS_WARNING("getColumn(): can't get attribute '%s'", node.Name.c_str());
			return NULL;
		}

		currentColumn += attr->getOffset();

		switch (attr->getMetaType())
		{
		case PDS_Type:
		case PDS_ForwardRef:
		case PDS_BackRef:
		case PDS_Set:
			return getColumn(currentColumn);
			break;

		case PDS_ArrayType:
		case PDS_ArrayRef:
			{
				const CType*	index = _Parent->getType(attr->getIndexId());
				if (index == NULL)
				{
					// this is a real error!
					PDS_WARNING("getColumn(): can't get index '%d' of attribute '%s'", attr->getIndexId(), node.Name.c_str());
					return NULL;
				}

				TEnumValue		indexValue;
				if (sscanf(node.Key.c_str(), "%d", &indexValue) != 1)
				{
					if (!index->isIndex())
					{
						PDS_WARNING("getColumn(): can't get index value of '%s' in attribute '%s'", node.Key.c_str(), node.Name.c_str());
						return NULL;
					}

					indexValue = index->getIndexValue(node.Key, verbose);

					if (indexValue == INVALID_ENUM_VALUE)
					{
						if (verbose)
							PDS_WARNING("getColumn(): can't get index value of '%s' in attribute '%s', unknown to enum '%s'", node.Key.c_str(), node.Name.c_str(), index->getName().c_str());
						return NULL;
					}
				}

				if (indexValue >= attr->getColumns())
				{
					if (verbose)
						PDS_WARNING("getColumn(): index '%s' is out of '%s' attribute bounds", node.Key.c_str(), node.Name.c_str());
					return NULL;
				}

				currentColumn += indexValue;
				return getColumn(currentColumn);
			}
			break;

		case PDS_Class:
			{
				table = _Parent->getTable(attr->getTypeId());
				if (table == NULL || !table->initialised())
				{
					PDS_WARNING("getColumn(): unable to locate table '%d'", attr->getTypeId());
					return NULL;
				}
			}
			break;

		case PDS_ArrayClass:
			{
				table = _Parent->getTable(attr->getTypeId());
				if (table == NULL || !table->initialised())
				{
					PDS_WARNING("getColumn(): unable to locate table '%d'", attr->getTypeId());
					return NULL;
				}

				const CType*	index = _Parent->getType(attr->getIndexId());
				if (index == NULL)
				{
					PDS_WARNING("getColumn(): can't get index '%d' of attribute '%s'", attr->getIndexId(), node.Name.c_str());
					return NULL;
				}

				TEnumValue		indexValue;
				if (sscanf(node.Key.c_str(), "%d", &indexValue) != 1)
				{
					if (!index->isIndex())
					{
						PDS_WARNING("getColumn(): can't get index value of '%s' in attribute '%s'", node.Key.c_str(), node.Name.c_str());
						return NULL;
					}

					indexValue = index->getIndexValue(node.Key, verbose);

					if (indexValue == INVALID_ENUM_VALUE)
					{
						if (verbose)
							PDS_WARNING("getColumn(): can't get index value of '%s' in attribute '%s', unknown to enum '%s'", node.Key.c_str(), node.Name.c_str(), index->getName().c_str());
						return NULL;
					}
				}

				indexValue *= (TEnumValue)table->getColumns().size();

				if (indexValue >= attr->getColumns())
				{
					if (verbose)
						PDS_WARNING("getColumn(): index '%s' is out of '%s' attribute bounds", node.Key.c_str(), node.Name.c_str());
					return NULL;
				}

				currentColumn += indexValue;
			}
			break;
		}
	}

	// locate was not complete
	return NULL;
}



#endif //RY_PDS_TABLE_INLINE_H

