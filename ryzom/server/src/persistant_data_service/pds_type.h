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

#ifndef NL_PDS_TYPE_H
#define NL_PDS_TYPE_H

//
// NeL includes
//
#include <nel/misc/types_nl.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/log.h>

//
// STL includes
//
#include <map>
#include <vector>

//
// PDS includes
//
#include "../pd_lib/pd_server_utils.h"
#include "../pd_lib/pds_types.h"

class IDbFileStream;
class CDatabase;
class CTypeNode;

/**
 * A database Type
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CType : public CPDSLogger
{
public:

	/**
	 * Constructor
	 */
	CType();

	/**
	 * Destructor
	 */
	~CType();

	/**
	 * Clear
	 */
	void		clear();


	/**
	 * Init
	 */
	bool				init(CDatabase *database, const CTypeNode& type);

	/// Initialized yet?
	bool				initialised() const			{ return _Init; }


	/**
	 * Get type name
	 */
	const std::string&	getName() const							{ return _Name; }

	/**
	 * Get type id
	 */
	TTypeId				getId() const							{ return _Id; }

	/**
	 * Get datatype
	 */
	TDataType			getDataType() const						{ return _DataType; }

	/**
	 * Get byte size
	 */
	uint32				getByteSize() const						{ return _ByteSize; }

	/**
	 * Is type an index
	 */
	bool				isIndex() const							{ return isEnum() || isDimension(); }

	/**
	 * Is type an enum
	 */
	bool				isEnum() const							{ return _DataType == PDS_enum; }

	/**
	 * Is type a dimension
	 */
	bool				isDimension() const						{ return _DataType == PDS_dimension; }

	/**
	 * Add enum value
	 */
	bool				addEnum(const std::string &enumName, TEnumValue value);

	/**
	 * Get enum value from its name
	 */
	TEnumValue			getIndexValue(const std::string &name, bool verbose = true) const;

	/**
	 * Get enum name from its value
	 */
	std::string			getIndexName(TEnumValue value, bool verbose = true) const;

	/**
	 * Get Index size
	 */
	TEnumValue			getIndexSize() const;


	/**
	 * Get Enum Map
	 * You should check type is really an enum (no check performed)
	 */
	const std::map<std::string, TEnumValue>&	getEnumMap() const		{ return _EnumValueTable; }



protected:

	virtual std::string	getLoggerIdentifier() const	{ return NLMISC::toString("typ:%s", (_Name.empty() ? "<unnamed>" : _Name.c_str())); }

private:

	/// Initialised yet?
	bool				_Init;

	/// Parent database
	CDatabase*			_Parent;

	/// Name
	std::string			_Name;

	/// Id
	TTypeId				_Id;

	/// Data type
	TDataType			_DataType;

	/// Byte size
	uint32				_ByteSize;

	typedef std::map<std::string, TEnumValue>	TEnumValueTable;

	/**
	 * Enum to value conversion table
	 * Please notice that different enum may have the same value
	 */
	TEnumValueTable		_EnumValueTable;


	typedef std::vector<std::string>			TValueEnumTable;

	/**
	 * Value to enum conversion table
	 * Please notice that some enum may not be mapped, as they might share the same value with other
	 * Only 'last' enum is kept in this table
	 */
	TValueEnumTable		_ValueEnumTable;

	/// Index Size
	uint32				_IndexSize;

public:

	/// Display
	void				display(NLMISC::CLog *log = NLMISC::InfoLog) const;

};



#endif // NL_PDS_TYPE_H

/* End of pds_type.h */
