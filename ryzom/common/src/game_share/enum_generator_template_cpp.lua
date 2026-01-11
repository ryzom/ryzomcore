-- %name1% name_type_1
-- %name2% NAME_TYPE_2
-- %name3% NameType3
-- %name4% name type 4
-- %name5% nameType5
-- %enum1% 		ENUM1,
-- %enum2% 		{ "ENUM2", ENUM2 },

if not enum_generator then
	enum_generator = {}
end
enum_generator.template_cpp = [[
/** \file %name1%.cpp
 *
 * $Id: enum_generator_template_cpp.lua,v 1.1 2005/11/07 17:16:46 vuarand Exp $
 *
 * This file is generated, do not edit it.
 */

#include "stdpch.h"

// nel
#include "nel/misc/i18n.h"
#include "nel/misc/string_conversion.h"

#include "%name1%.h"

using namespace std;
using namespace NLMISC;

namespace %name2%
{
	// The conversion table
	const CStringConversion<T%name3%>::CPair stringTable [] =
	{
%enum2%
	};

	CStringConversion<T%name3%> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  UNDEFINED);


	// convert %name4% id to %name4% name string
	const std::string& toString( T%name3% %name5% )
	{
		return conversion.toString(%name5%);
	}


	// convert %name4% name to %name4% enum value
	T%name3% fromString( const std::string& str )
	{
		return conversion.fromString(str);
	}
	// convert %name4% name to %name4% enum value
	T%name3% stringTo%name3%( const std::string& str )
	{
		return conversion.fromString(str);
	}
}; // %name2%
]]
