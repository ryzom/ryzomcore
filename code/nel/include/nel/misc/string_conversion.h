// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_STRING_CONVERSION_H
#define NL_STRING_CONVERSION_H

#include "types_nl.h"
#include "common.h"

#include <map>

namespace NLMISC
{



// a predicate use to do unsensitive string comparison (with operator <)
struct CUnsensitiveStrLessPred
{
	bool operator()(const std::string &lhs, const std::string &rhs) const
	{
		return nlstricmp(lhs, rhs) < 0;
	}
};


/** This class allow simple mapping between string and other type (such as integral types or enum)
  * In fact this primarily intended to make a string / enum correspondance
  * Example of use :
  *
  * // An enumerated type
  * enum TMyType { Foo = 0, Bar, FooBar, Unknown };
  *
  * // The conversion table
  * static const CStringConversion<TMyType>::CPair stringTable [] =
  * {
  *   { "Foo", Foo },
  *   { "Bar", Bar },
  *   { "FooBar", FooBar }
  * };
  *
  * // The helper object for conversion (instance of this class)
  *	static const CStringConversion conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);
  *
  * // Some conversions request
  * TMyType value1 = conversion.fromString("foo");  // returns 'foo'
  * value1 = conversion.fromString("Foo");          // returns 'foo' (this is case unsensitive by default)
  * std::string str = conversion.toString(Bar)      // returns "Bar"
  *
  * NB : Please note that that helpers macros are provided to build such a table in an easy way
  *      \see NL_BEGIN_STRING_CONVERSION_TABLE
  *      \see NL_END_STRING_CONVERSION_TABLE
  *
  *
  * NB: by default this class behaves in a case unsensitive way. To change this, just change the 'Pred' template parameter
  *     to std::less<std::string>
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */

template <class DestType, class Pred = CUnsensitiveStrLessPred>
class CStringConversion
{
public:
	typedef DestType TDestType;
	typedef Pred     TPred;
	struct CPair
	{
		const char *Str;
		TDestType   Value;
	};
public:
	// init from pairs of string / value
	CStringConversion(const CPair *pairs, uint numPairs, const DestType &notFoundValue);

	// add a pair in the array
	void insert(const char *str, TDestType value);

	// From a string, retrieve the associated value, or the 'notFoundValue' provided to the ctor of this object if the string wasn't found
	const TDestType &fromString(const std::string &str) const;

	// From a value, retrieve the associated string, or an empty string if not found
	const std::string &toString(const TDestType &value) const;

	// nb of pairs in the map
	inline uint16 getNbPairs() const { return (uint16)_String2DestType.size(); }

	// Check a value against the list a value, return true if the value exist in the container
	bool isValid(const DestType &value) const;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	typedef std::map<std::string, TDestType, TPred> TString2DestType;
	typedef std::map<TDestType, std::string>        TDestType2String;
private:
	TString2DestType _String2DestType;
	TDestType2String _DestType2String;
	TDestType        _NotFoundValue;
};

/** This macro helps building a string conversion table
  * Example of use :
  *
  * // The enumerated type for which a conversion should be defined
  * enum TMyType { Foo = 0, Bar, FooBar, Unknown };
  *
  * // The conversion table
  * NL_BEGIN_STRING_CONVERSION_TABLE(TMyType)
  *	  NL_STRING_CONVERSION_TABLE_ENTRY(Foo)
  *	  NL_STRING_CONVERSION_TABLE_ENTRY(Bar)
  *	  NL_STRING_CONVERSION_TABLE_ENTRY(FooBar)
  * NL_END_STRING_CONVERSION_TABLE(TMyType, myConversionTable, Unknown)
  *
  * // Now, we can use the 'myConversionTable' intance
  *
  * std::string str = myConversionTable.toString(Bar)      // returns "Bar"
  *
  */
#define NL_BEGIN_STRING_CONVERSION_TABLE(__type)                                               \
static const NLMISC::CStringConversion<__type>::CPair __type##_nl_string_conversion_table[] =          \
{
#define NL_END_STRING_CONVERSION_TABLE(__type, __tableName, __defaultValue)                    \
};                                                                                             \
NLMISC::CStringConversion<__type>                                                                \
__tableName(__type##_nl_string_conversion_table, sizeof(__type##_nl_string_conversion_table)   \
			/ sizeof(__type##_nl_string_conversion_table[0]),  __defaultValue);
#define NL_STRING_CONVERSION_TABLE_ENTRY(val) { #val, val},






//////////////////////////////////////
// CStringConversion implementation //
//////////////////////////////////////

//=================================================================================================================
/** CStringConversion ctor
  */
template <class DestType, class Pred>
CStringConversion<DestType, Pred>::CStringConversion(const CPair *pairs, uint numPairs, const DestType &notFoundValue)
{
	for(uint k = 0; k < numPairs; ++k)
	{
		_String2DestType[pairs[k].Str] = pairs[k].Value;
		_DestType2String[pairs[k].Value] = pairs[k].Str;
	}
	_NotFoundValue = notFoundValue;
}

//
template <class DestType, class Pred>
void CStringConversion<DestType, Pred>::insert(const char *str, TDestType value)
{
	_String2DestType[str] = value;
	_DestType2String[value] = str;
}


//=================================================================================================================
template <class DestType, class Pred>
const DestType &CStringConversion<DestType, Pred>::fromString(const std::string &str) const
{
	typename TString2DestType::const_iterator it = _String2DestType.find(str);
	if (it == _String2DestType.end())
	{
		return _NotFoundValue;
	}
	else
	{
		return it->second;
	}
}

//=================================================================================================================
template <class DestType, class Pred>
const std::string &CStringConversion<DestType, Pred>::toString(const DestType &value) const
{
	typename TDestType2String::const_iterator it = _DestType2String.find(value);
	if (it == _DestType2String.end())
	{
		static std::string emptyString;
		return emptyString;
	}
	else
	{
		return it->second;
	}
}


//=================================================================================================================
template <class DestType, class Pred>
bool CStringConversion<DestType, Pred>::isValid(const DestType &value) const
{
	typename TDestType2String::const_iterator it = _DestType2String.find(value);

	return it != _DestType2String.end();
}


} // NLMISC








#endif

