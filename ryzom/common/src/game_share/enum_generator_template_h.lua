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
enum_generator.template_h = [[
/** \file %name1%.h
 * enum of %name4%
 *
 * $Id: enum_generator_template_h.lua,v 1.1 2005/11/07 17:16:46 vuarand Exp $
 *
 * This file is generated, do not edit it.
 */



#ifndef RY_%name2%_H
#define RY_%name2%_H

#include "nel/misc/types_nl.h"

namespace %name2%
{
	// This file is generated, do not edit it. Source file is (probably) %name1%.lua.
	enum T%name3%
	{
%enum1%

		UNDEFINED,
		NB_%name2% = UNDEFINED
	};


	/**
	 * get the right %name4% from the input string
	 * \param str the input string
	 * \return the T%name3% associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	T%name3% stringTo%name3%( const std::string& str );

	/**
	 * get the right %name4% from the input string
	 * \param str the input string
	 * \return the T%name3% associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	T%name3% fromString( const std::string& str );

	/**
	 * return the %name4% as a string
	 * \param %name5% %name4% to transform into a string
	 * \return the %name4% as a string
	 */
	const std::string& toString( T%name3% %name5% );

}; // %name2%

#endif // RY_%name2%_H
/* End of %name1%.h */
]]
