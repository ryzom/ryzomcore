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

#ifndef RY_PDS_TYPES_H
#define RY_PDS_TYPES_H

#include <nel/misc/types_nl.h>
#include <nel/misc/entity_id.h>


/**
 * Possible datatypes
 */
enum TDataType
{
	PDS_FirstDataType,

	PDS_bool = PDS_FirstDataType,
	PDS_char,
	PDS_ucchar,
	PDS_uint8,
	PDS_uint16,
	PDS_uint32,
	PDS_uint64,
	PDS_sint8,
	PDS_sint16,
	PDS_sint32,
	PDS_sint64,
	PDS_float,
	PDS_double,
	PDS_CSheetId,
	PDS_CNodeId,
	PDS_CEntityId,
	PDS_enum,
	PDS_dimension,

	PDS_StrictDataType = PDS_dimension,

	PDS_Index,
	PDS_List,

	PDS_OverLastDataType,
	PDS_UnknownDataType = PDS_OverLastDataType
};

/**
 * Metatypes
 * The kind of data an attribute handles
 */
enum TMetaType
{
	PDS_FirstMetaType,

	PDS_Type = PDS_FirstMetaType,
	PDS_Class,
	PDS_BackRef,
	PDS_ForwardRef,
	PDS_ArrayType,
	PDS_ArrayClass,
	PDS_ArrayRef,
	PDS_Set,

	PDS_OverLastMetaType,
	PDS_UnknownMetaType = PDS_OverLastMetaType
};

/**
 * Id of type/class
 */
typedef uint32		TTypeId;
const TTypeId		INVALID_TYPE_ID = 0xffffffff;

/**
 * Type for builtin indexes
 */
typedef uint32		TIndexValue;	// only for handling, storage assumes its own type (see dimension)
const TIndexValue	INVALID_INDEX_VALUE = 0xffffffff;

/**
 * Type for builtin enums
 */
typedef TIndexValue	TEnumValue;
const TEnumValue	INVALID_ENUM_VALUE = INVALID_INDEX_VALUE;
const uint32		ENUM_BYTE_SIZE = 4;
#define INVALID_ENUM_NAME "InvalidEnumName"



/**
 * Display function for all datatypes
 */
template<typename T>
std::string	dataTypeAsString(const T& a)							{ return "'"+NLMISC::toString(a)+"'"; }
inline std::string	dataTypeAsString(const bool &a)					{ return "'"+NLMISC::toString((uint)a)+"' "+std::string(a ? "true" : "false"); }
inline std::string	dataTypeAsString(const char &a)					{ return std::string("'")+a+std::string("' '")+NLMISC::toString((uint8)a)+"'"; }
inline std::string	dataTypeAsString(const NLMISC::CEntityId &a)	{ return a.toString(); }


/**
 * Get the TDataType from a name
 */
inline TDataType	getDataTypeFromName(const std::string &name)
{
#define CHECK_DATATYPE(type)	if (name == #type) return PDS_##type;

	CHECK_DATATYPE(bool)
	CHECK_DATATYPE(char)
	CHECK_DATATYPE(ucchar)
	CHECK_DATATYPE(uint8)
	CHECK_DATATYPE(uint16)
	CHECK_DATATYPE(uint32)
	CHECK_DATATYPE(uint64)
	CHECK_DATATYPE(sint8)
	CHECK_DATATYPE(sint16)
	CHECK_DATATYPE(sint32)
	CHECK_DATATYPE(sint64)
	CHECK_DATATYPE(float)
	CHECK_DATATYPE(double)
	CHECK_DATATYPE(CSheetId)
	CHECK_DATATYPE(CEntityId)
	CHECK_DATATYPE(enum)
	CHECK_DATATYPE(dimension)
	CHECK_DATATYPE(Index)
	CHECK_DATATYPE(List)

	return PDS_UnknownDataType;
}

/**
 * Get the name of a TDataType
 */
inline std::string	getNameFromDataType(TDataType type)
{
#define CHECK_DATATYPE_NAME(type)	case PDS_##type: return #type; break;

	switch (type)
	{
		CHECK_DATATYPE_NAME(bool)
		CHECK_DATATYPE_NAME(char)
		CHECK_DATATYPE_NAME(ucchar)
		CHECK_DATATYPE_NAME(uint8)
		CHECK_DATATYPE_NAME(uint16)
		CHECK_DATATYPE_NAME(uint32)
		CHECK_DATATYPE_NAME(uint64)
		CHECK_DATATYPE_NAME(sint8)
		CHECK_DATATYPE_NAME(sint16)
		CHECK_DATATYPE_NAME(sint32)
		CHECK_DATATYPE_NAME(sint64)
		CHECK_DATATYPE_NAME(float)
		CHECK_DATATYPE_NAME(double)
		CHECK_DATATYPE_NAME(CSheetId)
		CHECK_DATATYPE_NAME(CEntityId)
		CHECK_DATATYPE_NAME(enum)
		CHECK_DATATYPE_NAME(dimension)
		CHECK_DATATYPE_NAME(Index)
		CHECK_DATATYPE_NAME(List)
	default:
		return "UnknownDataType";
		break;
	}

	return "UnknownDataType";
}


/**
 * Get the TMetaType from a name
 */
inline TMetaType	getMetaTypeFromName(const std::string &name)
{
#define CHECK_METATYPE(typestr, type)	if (name == #typestr) return PDS_##type;

	CHECK_METATYPE(type, Type)
	CHECK_METATYPE(class, Class)
	CHECK_METATYPE(backref, BackRef)
	CHECK_METATYPE(forwardref, ForwardRef)
	CHECK_METATYPE(arraytype, ArrayType)
	CHECK_METATYPE(arrayclass, ArrayClass)
	CHECK_METATYPE(arrayref, ArrayRef)
	CHECK_METATYPE(set, Set)

	return PDS_UnknownMetaType;
}

/**
 * Get the name of a TMetaType
 */
inline std::string	getNameFromMetaType(TMetaType type)
{
#define CHECK_METATYPE_NAME(typestr, type)	case PDS_##type: return #typestr; break;

	switch (type)
	{
		CHECK_METATYPE_NAME(type, Type)
		CHECK_METATYPE_NAME(class, Class)
		CHECK_METATYPE_NAME(backref, BackRef)
		CHECK_METATYPE_NAME(forwardref, ForwardRef)
		CHECK_METATYPE_NAME(arraytype, ArrayType)
		CHECK_METATYPE_NAME(arrayclass, ArrayClass)
		CHECK_METATYPE_NAME(arrayref, ArrayRef)
		CHECK_METATYPE_NAME(set, Set)
	default:
		return "UnknownMetaType";
		break;
	}

	return "UnknownDataType";
}


/**
 * Get standard byte size
 */
inline uint32		getStandardByteSize(TDataType dataType)
{
#define CHECK_DATATYPE_SIZE(type, size)		case PDS_##type: return size; break;

	switch (dataType)
	{
		CHECK_DATATYPE_SIZE(bool, 1)
		CHECK_DATATYPE_SIZE(char, 1)
		CHECK_DATATYPE_SIZE(ucchar, 2)
		CHECK_DATATYPE_SIZE(uint8, 1)
		CHECK_DATATYPE_SIZE(uint16, 2)
		CHECK_DATATYPE_SIZE(uint32, 4)
		CHECK_DATATYPE_SIZE(uint64, 8)
		CHECK_DATATYPE_SIZE(sint8, 1)
		CHECK_DATATYPE_SIZE(sint16, 2)
		CHECK_DATATYPE_SIZE(sint32, 4)
		CHECK_DATATYPE_SIZE(sint64, 8)
		CHECK_DATATYPE_SIZE(float, 4)
		CHECK_DATATYPE_SIZE(double, 8)
		CHECK_DATATYPE_SIZE(CSheetId, 4)
		CHECK_DATATYPE_SIZE(CEntityId, 8)
		CHECK_DATATYPE_SIZE(enum, ENUM_BYTE_SIZE)
		CHECK_DATATYPE_SIZE(Index, 8)
		CHECK_DATATYPE_SIZE(List, 4)

		CHECK_DATATYPE_SIZE(dimension, ENUM_BYTE_SIZE)

	default:
		nlwarning("PDS::CType::getStandardByteSize(): dataType %d is not standard, used byte size is set to 0", dataType);
		return 0;
		break;
	}

	// something went wrong
	return 0;
}

/**
 * Get strict standard byte size
 * Will fail for PDS_List and PDS_Index datatypes
 */
inline uint32		getStrictStandardByteSize(TDataType dataType)
{
	switch (dataType)
	{
		CHECK_DATATYPE_SIZE(bool, 1)
		CHECK_DATATYPE_SIZE(char, 1)
		CHECK_DATATYPE_SIZE(ucchar, 2)
		CHECK_DATATYPE_SIZE(uint8, 1)
		CHECK_DATATYPE_SIZE(uint16, 2)
		CHECK_DATATYPE_SIZE(uint32, 4)
		CHECK_DATATYPE_SIZE(uint64, 8)
		CHECK_DATATYPE_SIZE(sint8, 1)
		CHECK_DATATYPE_SIZE(sint16, 2)
		CHECK_DATATYPE_SIZE(sint32, 4)
		CHECK_DATATYPE_SIZE(sint64, 8)
		CHECK_DATATYPE_SIZE(float, 4)
		CHECK_DATATYPE_SIZE(double, 8)
		CHECK_DATATYPE_SIZE(CSheetId, 4)
		CHECK_DATATYPE_SIZE(CEntityId, 8)
		CHECK_DATATYPE_SIZE(enum, ENUM_BYTE_SIZE)

		CHECK_DATATYPE_SIZE(dimension, ENUM_BYTE_SIZE)

	default:
		nlwarning("PDS::CType::getStandardByteSize(): dataType %d is not standard, used byte size is set to 0", dataType);
		return 0;
		break;
	}

	// something went wrong
	return 0;
}

/**
 * Check TDataType
 */
inline bool			checkDataType(TDataType dataType)
{
	return dataType >= PDS_FirstDataType && dataType < PDS_OverLastDataType;
}

/**
 * Check TMetaType
 */
inline bool			checkMetaType(TMetaType metaType)
{
	return metaType >= PDS_FirstMetaType && metaType < PDS_OverLastMetaType;
}




//
inline bool	isIntegerType(TDataType type)
{
#define CHECK_IS_INTEGER_TYPE(type)		case PDS_##type: return true; break;

	switch (type)
	{
		CHECK_IS_INTEGER_TYPE(uint8)
		CHECK_IS_INTEGER_TYPE(sint8)
		CHECK_IS_INTEGER_TYPE(uint16)
		CHECK_IS_INTEGER_TYPE(sint16)
		CHECK_IS_INTEGER_TYPE(uint32)
		CHECK_IS_INTEGER_TYPE(sint32)
		CHECK_IS_INTEGER_TYPE(uint64)
		CHECK_IS_INTEGER_TYPE(sint64)
		CHECK_IS_INTEGER_TYPE(dimension)
		default:
			return false;
			break;
	}

	return false;
}

inline bool	isFloatType(TDataType type)
{
#define CHECK_IS_FLOAT_TYPE(type)		case PDS_##type: return true; break;

	switch (type)
	{
		CHECK_IS_FLOAT_TYPE(float)
		CHECK_IS_FLOAT_TYPE(double)
		default:
			return false;
			break;
	}

	return false;
}

/**
 * Check Strict Datatype
 */
inline bool			checkStrictDataType(TDataType dataType)
{
	return dataType >= PDS_FirstDataType && dataType <= PDS_StrictDataType;
}


/**
 * Check Datatype compatibility
 */
inline bool			checkDataTypeCompatible(TDataType from, TDataType into)
{
	// check from and into are known datatypes
	if (!checkDataType(from) || !checkDataType(into))
		return false;

	// the trivial case
	if (from == into)
		return true;

	/*
	 * Conversion rules
	 *
	 * bool -> bool
	 *
	 * char, ucchar -> char, ucchar
	 *
	 * uint8, uint16, uint32, uint64
	 *		-> uint8, uint16, uint32, uint64 (clamp value to max of type)
	 *		-> sint8, sint16, sint32, sint64 (clamp value to max of type)
	 *		-> float, double (accuracy discarded)
	 *
	 * sint8, sint16, sint32, sint64
	 *		-> uint8, uint16, uint32, uint64 (negative value clamped to 0, max clamped)
	 *		-> sint8, sint16, sint32, sint64 (clamp value to max of type, max clamped)
	 *		-> float, double (accuracy discarded)
	 *
	 * float, double
	 *		-> uint8, uint16, uint32, uint64 (negative value clamped to 0, max clamped)
	 *		-> sint8, sint16, sint32, sint64 (min and max clamped)
	 *		-> float, double (min and max clamped)
	 *
	 * CSheetId -> CSheetId
	 *
	 * CEntityId -> CEntityId
	 *
	 * enum -> enum
	 *
	 * dimension -> dimension
	 *
	 * Index -> Index
	 *
	 * List -> List
	 */

	switch (from)
	{
	case PDS_char:
	case PDS_ucchar:
		return into == PDS_char || into == PDS_ucchar;
		break;

	case PDS_uint8:
	case PDS_uint16:
	case PDS_uint32:
	case PDS_uint64:
		return isIntegerType(into) || isFloatType(into);
		break;

	case PDS_sint8:
	case PDS_sint16:
	case PDS_sint32:
	case PDS_sint64:
		return isIntegerType(into) || isFloatType(into);
		break;

	case PDS_float:
	case PDS_double:
		return isIntegerType(into) || isFloatType(into);
		break;
	default:
		return false;
		break;
	}

	return false;
}


#endif //RY_PDS_TYPES_H



