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

#ifndef NL_PDS_ATTRIBUTE_H
#define NL_PDS_ATTRIBUTE_H

//
// NeL includes
//
#include "nel/misc/types_nl.h"
#include <nel/misc/i_xml.h>

//
// PDS includes
//
#include "../pd_lib/pd_server_utils.h"
#include "../pd_lib/pds_types.h"

class IDbFileStream;
class CTable;
class CDatabase;
class CAttributeNode;

/**
 * An attribute of a table/class
 * It is different from a column, as it may hold many columns (as in an array or a simple class)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CAttribute : public CPDSLogger
{
public:

	/// Constructor
	CAttribute() : 
		_MetaType(PDS_UnknownMetaType),
		_DataType(PDS_UnknownDataType),
		_TypeId(INVALID_TYPE_ID),
		_ReferencedAttribute(0),
		_AllowNull(false),
		_Key(INVALID_TYPE_ID),
		_IndexId(0),
		_Id(0),
		_Offset(0),
		_Columns(0),
		_Init(false),
		_Root(NULL),
		_Parent(NULL)
	{
	}

	/// Destructor
	~CAttribute();

	/**
	 * Initialize attribute using a full xml node
	 */
	bool				init(CDatabase *root, CTable* parent, const CAttributeNode& attribute);

	/// Initialized yet?
	bool				initialised() const			{ return _Init; }

	/**
	 * Build columns for this attribute
	 */
	bool				buildColumns();

	/**
	 * Compute back reference key
	 */
	bool				computeBackRefKey();


	/**
	 * Set column offset and size
	 */
	void				setAttributeColumns(uint32 offset, uint32 size)		{ _Offset = offset; _Columns = size; }


	/// Get meta type of column
	TMetaType			getMetaType() const			{ return _MetaType; }

	/// Get data type of column
	TDataType			getDataType() const			{ return _DataType; }

	/// Get type/class id
	TTypeId				getTypeId() const			{ return _TypeId; }

	/// Get referenced attribute in backward/forward reference
	uint32				getReferencedAttribute() const	{ return _ReferencedAttribute; }

	/// Get associated key
	uint32				getBackRefKey() const		{ return _Key; }

	/// Is ArrayRef has null pointers allowed?
	bool				allowNull() const			{ return _AllowNull; }

	/// Get index type
	uint32				getIndexId() const			{ return _IndexId; }

	/// Get Row order
	uint32				getId() const				{ return _Id; }

	/// Get column offset in table
	uint32				getOffset() const			{ return _Offset; }

	/// Get column number
	uint32				getColumns() const			{ return _Columns; }

	/// Get column name
	const std::string&	getName() const				{ return _Name; }

	/// Get parent table
	const CTable*		getParent() const			{ return _Parent; }

	/// Get Root
	const CDatabase*	getRoot() const				{ return _Root; }

protected:

	virtual std::string	getLoggerIdentifier() const	{ return NLMISC::toString("attr:%s", (_Name.empty() ? "<unnamed>" : _Name.c_str())); }


private:

	/// Initialised yet?
	bool								_Init;

	/// Root Database
	CDatabase*							_Root;

	/// Parent table
	CTable*								_Parent;

	/// Name of the attribute
	std::string							_Name;

	/// Id of the attribute in class
	uint32								_Id;


	/// Metatype of the attribute
	TMetaType							_MetaType;

	/// Data type of the attribute, unused when meta type is an array, a set or a simple class
	TDataType							_DataType;

	/// Type descriptor of the attribute (type or class)
	TTypeId								_TypeId;

	/// Back/Forward referenced attribute in the parent class
	uint32								_ReferencedAttribute;

	/// Allow null reference, only ArrayRef
	bool								_AllowNull;

	/// Key associated to a backreference, 
	uint32								_Key;

	/// Index type for array
	TTypeId								_IndexId;

	/// Start column of the attribute
	uint32								_Offset;

	/// Number of columns in the attribute
	uint32								_Columns;
};

#endif // NL_PDS_ATTRIBUTE_H

/* End of pds_attribute.h */
