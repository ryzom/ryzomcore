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

#ifndef NL_PDS_COLUMN_H
#define NL_PDS_COLUMN_H

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
class CAttribute;
class CDatabase;

/**
 * A column definition.
 * Defines the kind of data stored at this place and the type of data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CColumn : public CPDSLogger
{
public:

	/// Constructor
	CColumn() :
		_Init(false),
		_MetaType(PDS_UnknownMetaType),
		_DataType(PDS_UnknownDataType),
		_TypeId(INVALID_TYPE_ID),
		//_ReferencedAttribute(0),
		_Id(0),
		_ByteSize(0),
		_ByteOffset(0),
		_Root(NULL),
		_Parent(NULL)
	{
	}

	/// Destructor
	~CColumn();

	/**
	 * Initialize column
	 */
	bool				init(CAttribute* parent, CDatabase *root);

	/// Initialized yet?
	bool				initialised() const			{ return _Init; }

	/// Get meta type of column
	TMetaType			getMetaType() const			{ return _MetaType; }

	/// Get data type of column
	TDataType			getDataType() const			{ return _DataType; }

	/// Get Class descriptor
	TTypeId				getTypeId() const			{ return _TypeId; }

	/// Get C++ descriptor
	//uint32				getReferencedAttribute() const	{ return _ReferencedAttribute; }

	/// Get Row order
	uint32				getId() const				{ return _Id; }

	/// Get Column byte size
	uint32				getByteSize() const			{ return _ByteSize; }

	/// Get Byte Offset
	uint32				getByteOffset() const		{ return _ByteOffset; }

	/// Get column name
	const std::string&	getName() const				{ return _Name; }

	/// Get parent attribute
	const CAttribute*	getParent() const			{ return _Parent; }

	/// Get Root
	const CDatabase*	getRoot() const				{ return _Root; }


	/// Set byte offset
	void				setByteOffset(uint32 offset)	{ _ByteOffset = offset; }

/*
	/// Critical message
	void				warning(const std::string &msg) const;

	/// Debug message
	void				debug(const std::string &msg) const;
*/

protected:

	virtual std::string	getLoggerIdentifier() const	{ return NLMISC::toString("col:%s", (_Name.empty() ? "<unnamed>" : _Name.c_str())); }


private:

	/// Initialized yet?
	bool								_Init;

	/// Root Database
	CDatabase*							_Root;

	/// Parent attribute
	CAttribute*							_Parent;

	/// Name of the column
	std::string							_Name;

	/// Id of column in row
	uint32								_Id;


	/// Metatype of column, can only be PDS_Type, PDS_BackRef, PDS_ForwardRef, PDS_Set
	TMetaType							_MetaType;

	/// Data type of attribute
	TDataType							_DataType;

	/// Type descriptor of the column
	TTypeId								_TypeId;


	/// Back/Forward referenced attribute in the parent class
	//uint32								_ReferencedAttribute;


	/// Column size in byte
	uint32								_ByteSize;

	/// Byte offset from row start
	uint32								_ByteOffset;


	friend class CAttribute;
};



#endif // NL_PDS_COLUMN_H

/* End of pds_column.h */
