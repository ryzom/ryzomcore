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

#include	"stdpch.h"

#include "script_compiler.h"

#include "ai_grp_npc.h"
#include "group_profile.h"
#include "ai_generic_fight.h"
#include "server_share/msg_brick_service.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

#include "ai_script_data_manager.h"

#include "nf_helpers.h"

#include "nel/misc/md5.h"

using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;

//----------------------------------------------------------------------------
/** @page code

@subsection copyDynEnergy_sff_
Copy energy values from an index to another, on all groups matching the
specified request. Valid index values are integers from 0 to 3.

Request is of the form
"[family-<family_name>] [cellZone-<cellzone_name>]". Names can contain
wild cards (? and *).

Arguments: s(Request), f(IndexSrc), f(IndexDst) ->
@param[in] Request is a request of the form "[family-<family_name>] [cellZone-<cellzone_name>]"
@param[in] IndexSrc is a an index number in energy system
@param[in] IndexDst is a an index number in energy system

@code
()copyDynEnergy("family-tribu*", SourceIndex , DestinationIndex); // Copy the dyn energy of groups defined by family-tribu* from slot SourceIndex to slot DestinationIndex
@endcode

*/
// none
void copyDynEnergy_sff_(CStateInstance* entity, CScriptStack& stack)
{
	size_t const destIndex = (int)(float)stack.top();
	stack.pop();
	size_t const srcIndex = (int)(float)stack.top();
	stack.pop();
	string Request = stack.top();
	stack.pop();
	
	vector<string> args;
	{
		CStringSeparator sep(Request," ");
		while (sep.hasNext())
			args.push_back(sep.get());
	}
	
	CDoOnFamilyCopyDynEnergy command(srcIndex, destIndex);
	doOnFamily(args, &command);
}

//----------------------------------------------------------------------------
/** @page code

@subsection setDynEnergy_sff_
Sets energy values on an index, on all groups matching the specified request.
Valid index values are integers from 0 to 3. Valid values range from 0 to 1.

Request is of the form
"[family-<family_name>] [cellZone-<cellzone_name>]". Names can contain
wild cards (? and *).

Arguments: s(Request), f(Index), f(Value) ->
@param[in] Request is a request of the form "[family-<family_name>] [cellZone-<cellzone_name>]"
@param[in] Index is a an index number in energy system
@param[in] Value is a an energy value for energy system

@code
()setDynEnergy("family-tribu*", Index, Energy); // Sets the dyn energy of all groups defined by family-tribu* to Energy in slot Index
@endcode

*/
// none
void setDynEnergy_sff_(CStateInstance* entity, CScriptStack& stack)
{
	float const value = stack.top();
	stack.pop();
	size_t const index = (int)(float)stack.top();
	stack.pop();
	string Request = stack.top();
	stack.pop();
	
	vector<string> args;
	{
		CStringSeparator sep(Request, " ");
		while (sep.hasNext())
			args.push_back(sep.get());
	}
	
	CDoOnFamilySetDynEnergy command(index, value);
	doOnFamily(args, &command);
}

//----------------------------------------------------------------------------
/** @page code

@subsection clamp_fff_f
Clamps a value between lim1 and lim2. Low bound is the min value of
(lim1,lim2), up bound the max value. If input value is out of
[low_bound;up_bound], this function returns the nearest limit of value.

Arguments: f(value),f(lim1),f(lim2) -> f(clamped_value)
@param[in] value is the value to clamp
@param[in] lim1 is a clamp limit
@param[in] lim2 is a clamp limit
@param[out] clamped_value is the clamped value

@code
(percentage)clamp(ratio, 0, 1);
@endcode

*/
// none
void clamp_fff_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float lim2 = stack.top();
	stack.pop();
	float lim1 = stack.top();
	stack.pop();
	float& value = stack.top();
	
	// Order limits
	if (lim1>lim2)
		std::swap(lim1, lim2);
	// Clamp value
	if (value<lim1)
		value = lim1;
	if (value>lim2)
		value = lim2;
}

//----------------------------------------------------------------------------
/** @page code

@subsection min_ff_f
Returns the lowest of two values.

Arguments: f(value1),f(value2) -> f(min_value)
@param[in] value1 is a value to compare
@param[in] value2 is a value to compare
@param[out] min_value is the min value

@code
(shortest)min(left, right);
@endcode

*/
// none
void min_ff_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float value2 = stack.top();
	stack.pop();
	float& value = stack.top();
	// Min value
	value = std::min(value, value2);
}

//----------------------------------------------------------------------------
/** @page code

@subsection max_ff_f
Returns the highest of two values.

Arguments: f(value1),f(value2) -> f(max_value)
@param[in] value1 is a value to compare
@param[in] value2 is a value to compare
@param[out] max_value is the max value

@code
(longest)max(left, right);
@endcode

*/
// none
void max_ff_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float value2 = stack.top();
	stack.pop();
	float& value = stack.top();
	// Min value
	value = std::max(value, value2);
}

//----------------------------------------------------------------------------
/** @page code

@subsection rndm_ff_f
Returns a random value in interval [min;max[. max must be higher than min.

Arguments: f(min),f(max) -> f(random_value)
@param[in] min is a lower bound of the possible returned values
@param[in] max is a upper bound of the possible returned values
@param[out] random_value is a random value

@code
(val)rndm(0, 1);
@endcode

*/
// none
void rndm_ff_f(CStateInstance* entity, CScriptStack& stack)
{
	float max = stack.top();
	stack.pop();
	float min = stack.top();
	
	// Min value
	static uint32 const maxLimit = ((uint32)~0U)>>1;
	double const rval = (double)CAIS::rand32(maxLimit)/(double)maxLimit; // [0-1[
	float const value = (float)(rval * (max-min) + min);

	// Set rets
	stack.top() = value;
}

//----------------------------------------------------------------------------
/** @page code

@subsection floor_f_f
Returns the highest integer lower than or equal to input value.

Arguments: f(value) -> f(floored_value)
@param[in] value is a real value
@param[out] floored_value is an integer

@code
(count)floor(value);
@endcode

*/
// none
void floor_f_f(CStateInstance* entity, CScriptStack& stack)
{
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = floorf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection ceil_f_f
Returns the lowest integer higher than or equal to input value.

Arguments: f(value) -> f(ceiled_value)
@param[in] value is a real value
@param[out] ceiled_value is an integer

@code
(count)ceil(value);
@endcode

*/
// none
void ceil_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = ceilf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection round_f_f
Returns the integer closest to input value.

Arguments: f(value) -> f(rounded_value)
@param[in] value is a real value
@param[out] ceiled_value is an integer

@code
(count)round(value);
@endcode

*/
// none
void round_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = floorf(value+0.5f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection abs_f_f
Returns the absolute value of the input value.

Arguments: f(value) -> f(rounded_value)
@param[in] value is a real value
@param[out] absolute_value is a positive value

@code
(dist)abs(diff);
@endcode

*/
// none
void abs_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = fabsf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection sin_f_f
Returns the sinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is sin(x)

@code
(y)sin(x);
@endcode

*/
// none
void sin_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = sinf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection asin_f_f
Returns the arcsinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is asin(x)

@code
(y)asin(x);
@endcode

*/
// none
void asin_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = asinf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection sinh_f_f
Returns the hyperbolic sinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is sinh(x)

@code
(y)sinh(x);
@endcode

*/
// none
void sinh_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = sinhf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection cos_f_f
Returns the cosinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is cos(x)

@code
(y)cos(x);
@endcode

*/
// none
void cos_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = cosf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection acos_f_f
Returns the arccosinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is acos(x)

@code
(y)acos(x);
@endcode

*/
// none
void acos_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = acosf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection cosh_f_f
Returns the hyperbolic cosinus of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is cosh(x)

@code
(y)cosh(x);
@endcode

*/
// none
void cosh_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = coshf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection tan_f_f
Returns the tangent of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is tan(x)

@code
(y)tan(x);
@endcode

*/
// none
void tan_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = tanf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection atan_f_f
Returns the arctangent of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is atan(x)

@code
(y)atan(x);
@endcode

*/
// none
void atan_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = atanf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection tanh_f_f
Returns the hyperbolic tangent of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is tanh(x)

@code
(y)tanh(x);
@endcode

*/
// none
void tanh_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = tanhf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection sqrt_f_f
Returns the square root of the input value.

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is sqrt(x)

@code
(y)sqrt(x);
@endcode

*/
// none
void sqrt_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = sqrtf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection exp_f_f
Returns the exponent of the input value (ie e^x).

Arguments: f(x) -> f(y)
@param[in] x is a real value
@param[out] y is exp(x)

@code
(y)exp(x);
@endcode

*/
// none
void exp_f_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float& value = stack.top();
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	value = expf(value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection pow_ff_f
Returns the base^exponent.

Arguments: f(x) -> f(y)
@param[in] pow is the base
@param[in] exponent is the exponent
@param[out] ret is base^exponent

@code
(max)exp(2, nbits);
@endcode

*/
// none
void pow_ff_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	float const exponent = stack.top();
	stack.pop();
	float const base = stack.top();
	
	// Min value
	// :FIXME: When used with a conformant standard library use std:: equivalent
	float const value = powf(base,exponent);
	// Set rets
	stack.top() = value;
}

//----------------------------------------------------------------------------
/** @page code

@subsection md5sum_s_s
Returns the md5 sum of a string.

Arguments: s(string) -> s(string)
@param[in] string is a string
@param[out] md5sum is a string

@code
($sum)strlen($str);
@endcode

*/
// none
void md5sum_s_s(CStateInstance* entity, CScriptStack& stack)
{
	std::string str = (std::string)stack.top();

	std::string value = NLMISC::getMD5((uint8*)&str[0], (uint32)str.size() ).toString();
	nlinfo(value.c_str());
	stack.top() = value;
}

//----------------------------------------------------------------------------
/** @page code

@subsection strlen_s_f
Returns the length of a string.

Arguments: s(string) -> f(length)
@param[in] string is a string
@param[out] length is the length of the input string

@code
(length)strlen($str);
@endcode

*/
// none
void strlen_s_f(CStateInstance* entity, CScriptStack& stack)
{
	stack.top() = (float)((string&)stack.top()).length();
}

//----------------------------------------------------------------------------
/** @page code

@subsection substr_sff_s
Returns a substring of the input string.

Arguments: s(string),f(start),f(length) -> s(substring)
@param[in] string is a string
@param[in] start is the first character to copy
@param[in] length is the length of the returned string
@param[out] substring is the substring of string starting at character @e start and @e length characters long

@code
(length)substr($str);
@endcode

*/
// none
void substr_sff_s(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	int length = (int)(float)stack.top();
	stack.pop();
	int start = (int)(float)stack.top();
	stack.pop();
	
	BOMB_IF( ((string&)stack.top()).length() < (uint)(start + length), "String too short for substr_sff_s operation", return );
	stack.top() = ((string&)stack.top()).substr(start, length);
}

//----------------------------------------------------------------------------
/** @page code

@subsection strtof_s_f
Converts a string to a float.

Arguments: s(string) -> f(value)
@param[in] string is a string
@param[out] value is the converted value

@code
(val)strtof($str);
@endcode

*/
// none
void strtof_s_f(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	std::string str = (std::string)stack.top();
	stack.pop();
	char* pos = NULL;
	float f = 0.0;
	if(str.length() > 0)
		f = (float)strtod(str.c_str(), &pos);
	else
		BOMB("strtof_s_f try to convert an empty string !", return);
	stack.push(f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection strtof_s_ff
Converts a string to a float.

Arguments: s(string) -> f(value),f(isfloat)
@param[in] string is a string
@param[out] value is the converted value
@param[out] isfloat is 1 if the input string contained a value, otherwise 0

@code
(val, isfloat)strtof($str);
@endcode

*/
// none
void strtof_s_ff(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	std::string str = (std::string)stack.top();
	stack.pop();
	char* pos = NULL;
	float f = 0.0;
	if(str.length() > 0)
		f = (float)strtod(str.c_str(), &pos);
	else
		BOMB("strtof_s_ff try to convert an empty string !", return);
	float success = str.c_str()!=pos?1.f:0.f;
	stack.push(success);
	stack.push(f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection strtof_s_fff
Converts a string to a float.

Arguments: s(string) -> f(value),f(isfloat),f(isfull)
@param[in] string is a string
@param[out] value is the converted value
@param[out] isfloat is 1 if the input string contained a value, otherwise 0
@param[out] isfull is 1 if the input string only contained the converted value, otherwise 0

@code
(val, isfloat, isfull)strtof($str);
@endcode

*/
// none
void strtof_s_fff(CStateInstance* entity, CScriptStack& stack)
{
	// Get args
	std::string str = (std::string)stack.top();
	stack.pop();
	char* pos = NULL;
	float f = 0.0;
	if(str.length() > 0)
		f = (float)strtod(str.c_str(), &pos);
	else
		BOMB("strtof_s_fff try to convert an empty string !", return);
	float success = str.c_str()!=pos?1.f:0.f;
	float full = pos[0]=='\0'?1.f:0.f;
	stack.push(full);
	stack.push(success);
	stack.push(f);
}

//----------------------------------------------------------------------------
/** @page code

@subsection createNamedEntity_s_
Creates a named entity in current AIS. A named entity is an entity containing
only 4 fields (name, state, param1, param2) and that can appear on web admin.

Arguments: s(name) ->
@param[in] name is a the name of the named entity to create

@code
()createNamedEntity("Invasion");
@endcode

*/
// none
void createNamedEntity_s_(CStateInstance* entity, CScriptStack& stack)
{
	CNamedEntityManager::getInstance()->create((std::string)stack.top());
	stack.pop();
}

//----------------------------------------------------------------------------
/** @page code

@subsection setNamedEntityProp_sss_
Sets a property of an existing named entity. Valid property names are:
- state
- param1
- param2
Name of the entity cannot be changed. This function does not trigger
listeners associated with the entity property.

Arguments: s(name),s(prop),s(content) ->
@param[in] name is a the name of the named entity to modify
@param[in] prop is a the property of the named entity to modify
@param[in] content is a the value to set

@code
()setNamedEntityProp("Invasion", "state", "Active");
@endcode

*/
// none
void setNamedEntityProp_sss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	CNamedEntityManager::getInstance()->get(name).set(prop, value, false);

}

//----------------------------------------------------------------------------
/** @page code

@subsection setNamedEntityPropCb_sss_
@sa @ref setNamedEntityProp_sss_

This function does trigger listeners associated with the entity property.

Arguments: s(name),s(prop),s(content) ->
@param[in] name is a the name of the named entity to modify
@param[in] prop is a the property of the named entity to modify
@param[in] content is a the value to set

@code
()setNamedEntityPropCb("Invasion", "state", "Active");
@endcode

*/
// none
void setNamedEntityPropCb_sss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	stack.pop();
	CNamedEntityManager::getInstance()->get(name).set(prop, value, true);

}

//----------------------------------------------------------------------------
// getNamedEntityProp_ss_s
// Arguments: s(name) -> 
/** @page code

@subsection getNamedEntityProp_ss_s
Returns the content of a named entity property. Valid property names are:
- state
- param1
- param2
Name of the entity cannot be retrieved, because it must be known to access the
entity.

Arguments: s(name),s(prop) -> s(content)
@param[in] name is a the name of the named entity to modify
@param[in] prop is a the property of the named entity to modify
@param[out] content is a the content of the specified field

@code
($state)getNamedEntityProp("Invasion", "state");
@endcode

*/
// none
void getNamedEntityProp_ss_s(CStateInstance* entity, CScriptStack& stack)
{
	std::string prop = (std::string)stack.top();
	stack.pop();
	std::string name = (std::string)stack.top();
	
	stack.top() = CNamedEntityManager::getInstance()->get(name).get(prop);
}

//----------------------------------------------------------------------------
/** @page code

@subsection destroyNamedEntity_s_
Destroys a named entity.

Arguments: s(name) ->
@param[in] name is a the name of the named entity to destroy

@code
()destroyNamedEntity("Invasion");
@endcode

*/
// none
void destroyNamedEntity_s_(CStateInstance* entity, CScriptStack& stack)
{
	CNamedEntityManager::getInstance()->destroy((std::string)stack.top());
	stack.pop();
}

//----------------------------------------------------------------------------
/** @page code

@subsection setSimplePhrase_ss_
Creates a phrase ID of the form <em>phraseName(){[phraseContent]}</em>.

Arguments: s(phraseName),s(phraseContent) ->
@param[in] phraseName is a the id of the phrase to define
@param[in] phraseContent is the text associated with the phrase

@code
()setSimplePhrase("HELLO", "Hi, how are you?"); // equivalent to "HELLO(){[Hi, how are you?]}"
@endcode

*/
// none
void setSimplePhrase_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string phraseContent = (std::string)stack.top();
	stack.pop();
	std::string phraseName = (std::string)stack.top();
	stack.pop();
	
	std::string phraseContent2;
	phraseContent2 += phraseName;
	phraseContent2 += "(){[";
	phraseContent2 += phraseContent;
	phraseContent2 += "]}";

	ucstring ucPhraseContent;
	ucPhraseContent.fromUtf8(phraseContent2); // utf-8 version
	//ucPhraseContent = phraseContent2; // iso-8859-1 version
	
	NLNET::CMessage	msgout("SET_PHRASE");
	msgout.serial(phraseName);
	msgout.serial(ucPhraseContent);
	sendMessageViaMirror("IOS", msgout);
}

void setSimplePhrase_sss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string lang = (std::string)stack.top();
	stack.pop();
	std::string phraseContent = (std::string)stack.top();
	stack.pop();
	std::string phraseName = (std::string)stack.top();
	stack.pop();
	
	std::string phraseContent2;
	phraseContent2 += phraseName;
	phraseContent2 += "(){[";
	phraseContent2 += phraseContent;
	phraseContent2 += "]}";

	ucstring ucPhraseContent;
	ucPhraseContent.fromUtf8(phraseContent2); // utf-8 version
	//ucPhraseContent = phraseContent2; // iso-8859-1 version
	
	NLNET::CMessage	msgout("SET_PHRASE_LANG");
	msgout.serial(phraseName);
	msgout.serial(ucPhraseContent);
	msgout.serial(lang);
	sendMessageViaMirror("IOS", msgout);
}


//----------------------------------------------------------------------------
/** @page code

@subsection dataGetVar_s_s
Returns the content of a script data variable. Data variable name is composed
of a file name and a variable name separated with ':', like in
"file:variable".

Arguments: s(name) -> s(value)
@param[in] name is a the name of the data variable
@param[out] value is a the content of the data variable

@code
($state)dataGetVar("Fyros:Patrol1State");
@endcode

*/
// none
void dataGetVar_s_s(CStateInstance* entity, CScriptStack& stack)
{
	std::string varId = (std::string)stack.top();
	
	stack.top() = CAIScriptDataManager::getInstance()->getVar_s(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection dataGetVar_s_f
Returns the content of a script data variable. Data variable name is composed
of a file name and a variable name separated with ':', like in
"file:variable".

Arguments: s(name) -> f(value)
@param[in] name is a the name of the data variable
@param[out] value is a the content of the data variable

@code
(nbPatrol)dataGetVar("Fyros:PatrolCount");
@endcode

*/
// none
void dataGetVar_s_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string varId = (std::string)stack.top();
	
	stack.top() = CAIScriptDataManager::getInstance()->getVar_f(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection dataSetVar_ss_
Changes the content of a script data variable. Data variable name is composed
of a file name and a variable name separated with ':', like in
"file:variable".

Arguments: s(name),s(value) -> 
@param[in] name is a the name of the data variable
@param[in] value is a the content of the data variable

@code
()dataSetVar("Fyros:Patrol1State", "Active");
@endcode

*/
// none
void dataSetVar_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	CAIScriptDataManager::getInstance()->setVar(varId, value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection dataSetVar_sf_
Changes the content of a script data variable. Data variable name is composed
of a file name and a variable name separated with ':', like in
"file:variable".

Arguments: s(name),s(value) -> 
@param[in] name is a the name of the data variable
@param[in] value is a the content of the data variable

@code
()dataSetVar("Fyros:PatrolCount", nbPatrol);
@endcode

*/
// none
void dataSetVar_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float value = (float)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	CAIScriptDataManager::getInstance()->setVar(varId, value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection dataSave__
Save all previously written script data variables to file. This must be done
explicitly, otherwise modified variables are not saved. This is primarily
necessary to save CPU, because writing operations can take time. This also
permit to ensure data integrity if a crash occurs between the writing of two
related variables.

Arguments:  -> 

@code
()dataSave();
@endcode

*/
// none
void dataSave__(CStateInstance* entity, CScriptStack& stack)
{
//	CAIScriptDataManager::getInstance()->save();
}

//----------------------------------------------------------------------------
/** @page code

@subsection setZoneState_sf_

Arguments:  -> 

arg0: is the zone name id

arg1:
if zone is not pvp
	arg1 is interpreted as a boolean (0 - inactive, 1 - active)
if zone is a pvp zone
	arg1 is interpreted as 
		0 - inactive
		1 - active with faction point rewards
		2 - active without faction point rewards

@code
()setZoneState("toto", 1.0);
@endcode

*/
// none
void setZoneState_sf_(CStateInstance* entity, CScriptStack& stack)
{
	uint32 state = (uint32)((float)stack.top());
	stack.pop();
	std::string zoneName = (std::string)stack.top();
	stack.pop();
	
	NLNET::CMessage	msgout("SET_ZONE_STATE");
	msgout.serial(zoneName);
	msgout.serial(state);
	sendMessageViaMirror("EGS", msgout);
}


//////////////////////////////////////////////////////////////////////////////
// Undocumented methods                                                     //
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// break__
// Arguments: ->
// /!\ This function should not be documented as it's not intended to be used
// by level designers (it immediatly stops the AIS on release versions).
// none
void break__(CStateInstance* entity, CScriptStack& stack)
{
	//	_asm int 3;
	nlassert(false);
}

void getName_c_s(CStateInstance* entity, CScriptStack& stack)
{
	IScriptContext* ctx = (IScriptContext*)stack.top();
	CGroup* group = dynamic_cast<CGroup*>(ctx);
	if (group)
		stack.top() = group->getName();
	else
		stack.top() = string();
}

void context__c(CStateInstance* entity, CScriptStack& stack)
{
	stack.push(entity);
}

//----------------------------------------------------------------------------
// setActivityVa
// Arguments: v(Activity,...) ->
// (null), bandit, escorted, guard, guard_escorted, normal, no_change
/*
void	setActivityVa	(CStateInstance	*entity, CScriptStack	&stack)
{
	string inSig = stack.top();
	stack.pop();
	string outSig = stack.top();
	stack.pop();
	
	// Pop input args
	std::deque<size_t> params;
	for (string::size_type i=0; i<inSig.length(); ++i)
	{
		params.push_front(stack.top());
		stack.pop();
	}
	vector<size_t> inParams(params.begin(), params.end());
	params.clear();
	vector<size_t> outParams(pOutSig->length());
	
//////////////////////////////////////////////////////////////////////////////
	
	// Content
	
//////////////////////////////////////////////////////////////////////////////
	
	// Push output args
	params.assign(outParams.begin(), outParams.end());
	for (string::size_type i=0; i<outSig.length(); ++i)
	{
		stack.push(params.front());
		params.pop_front();
	}
}
*/

void warning_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string msg = (std::string)stack.top();
	stack.pop();
	nlwarning("%s", msg.c_str());
}

void info_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string msg = (std::string)stack.top();
	stack.pop();
	nlinfo("%s", msg.c_str());
}

void debug_s_(CStateInstance* entity, CScriptStack& stack)
{
	std::string msg = (std::string)stack.top();
	stack.pop();
	nldebug("%s", msg.c_str());
}

void warning_f_(CStateInstance* entity, CScriptStack& stack)
{
	float val = (float)stack.top();
	stack.pop();
	nlwarning("%f", val);
}

void info_f_(CStateInstance* entity, CScriptStack& stack)
{
	float val = (float)stack.top();
	stack.pop();
	nlinfo("%f", val);
}

void debug_f_(CStateInstance* entity, CScriptStack& stack)
{
	float val = (float)stack.top();
	stack.pop();
	nldebug("%f", val);
}

/****************************************************************************/

std::map<std::string, FScrptNativeFunc> nfGetStaticNativeFunctions()
{
	std::map<std::string, FScrptNativeFunc> functions;
	
#define REGISTER_NATIVE_FUNC(cont, func) cont.insert(std::make_pair(std::string(#func), &func))
	
	REGISTER_NATIVE_FUNC(functions, copyDynEnergy_sff_);
	REGISTER_NATIVE_FUNC(functions, setDynEnergy_sff_);
	REGISTER_NATIVE_FUNC(functions, clamp_fff_f);
	REGISTER_NATIVE_FUNC(functions, min_ff_f);
	REGISTER_NATIVE_FUNC(functions, max_ff_f);
	REGISTER_NATIVE_FUNC(functions, rndm_ff_f);
	REGISTER_NATIVE_FUNC(functions, floor_f_f);
	REGISTER_NATIVE_FUNC(functions, ceil_f_f);
	REGISTER_NATIVE_FUNC(functions, round_f_f);
	REGISTER_NATIVE_FUNC(functions, abs_f_f);
	REGISTER_NATIVE_FUNC(functions, sin_f_f);
	REGISTER_NATIVE_FUNC(functions, asin_f_f);
	REGISTER_NATIVE_FUNC(functions, sinh_f_f);
	REGISTER_NATIVE_FUNC(functions, cos_f_f);
	REGISTER_NATIVE_FUNC(functions, acos_f_f);
	REGISTER_NATIVE_FUNC(functions, cosh_f_f);
	REGISTER_NATIVE_FUNC(functions, tan_f_f);
	REGISTER_NATIVE_FUNC(functions, atan_f_f);
	REGISTER_NATIVE_FUNC(functions, tanh_f_f);
	REGISTER_NATIVE_FUNC(functions, sqrt_f_f);
	REGISTER_NATIVE_FUNC(functions, exp_f_f);
	REGISTER_NATIVE_FUNC(functions, pow_ff_f);
	REGISTER_NATIVE_FUNC(functions, md5sum_s_s);
	REGISTER_NATIVE_FUNC(functions, strlen_s_f);
	REGISTER_NATIVE_FUNC(functions, substr_sff_s);
	REGISTER_NATIVE_FUNC(functions, strtof_s_f);
	REGISTER_NATIVE_FUNC(functions, strtof_s_ff);
	REGISTER_NATIVE_FUNC(functions, strtof_s_fff);
	REGISTER_NATIVE_FUNC(functions, createNamedEntity_s_);
	REGISTER_NATIVE_FUNC(functions, setNamedEntityProp_sss_);
	REGISTER_NATIVE_FUNC(functions, setNamedEntityPropCb_sss_);
	REGISTER_NATIVE_FUNC(functions, getNamedEntityProp_ss_s);
	REGISTER_NATIVE_FUNC(functions, destroyNamedEntity_s_);
	REGISTER_NATIVE_FUNC(functions, setSimplePhrase_ss_);
	REGISTER_NATIVE_FUNC(functions, setSimplePhrase_sss_);
	REGISTER_NATIVE_FUNC(functions, dataGetVar_s_s);
	REGISTER_NATIVE_FUNC(functions, dataGetVar_s_f);
	REGISTER_NATIVE_FUNC(functions, dataSetVar_ss_);
	REGISTER_NATIVE_FUNC(functions, dataSetVar_sf_);
	REGISTER_NATIVE_FUNC(functions, dataSave__);
	REGISTER_NATIVE_FUNC(functions, setZoneState_sf_);
	REGISTER_NATIVE_FUNC(functions, break__);
	REGISTER_NATIVE_FUNC(functions, getName_c_s);
	REGISTER_NATIVE_FUNC(functions, context__c);
	REGISTER_NATIVE_FUNC(functions, warning_s_);
	REGISTER_NATIVE_FUNC(functions, info_s_);
	REGISTER_NATIVE_FUNC(functions, debug_s_);
	REGISTER_NATIVE_FUNC(functions, warning_f_);
	REGISTER_NATIVE_FUNC(functions, info_f_);
	REGISTER_NATIVE_FUNC(functions, debug_f_);
	
#undef REGISTER_NATIVE_FUNC
	
	return functions;
}
