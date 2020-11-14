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

#ifndef NL_ALGO_H
#define NL_ALGO_H

#include "types_nl.h"
#include <vector>
#include <string>
#include "ucstring.h"


namespace NLMISC
{


// ***************************************************************************
// blend between 2 values
// NB: like 'toString' or 'swap', this function is intended to be specialised for other types (CRGBA ...)
template <class T, class U>
T blend(const T &v0, const T &v1, const U &blendFactor)
{
	return blendFactor * v1 + ((U) 1 - blendFactor) * v0;
}


// ***************************************************************************
// add a delta to a value until it reaches the wanted target. Value is clamped to the target
inline void incrementalBlend(float &value, float target, float absDelta)
{
	nlassert(absDelta >= 0.f);
	if (value < target)
	{
		value += absDelta;
		if (value > target) value = target;
	}
	else if (target < value)
	{
		value -= absDelta;
		if (value < target) value = target;
	}
}

// ***************************************************************************
/** bilinear of 4 values
  *  v3    v2
  *  +-----+
  *  |     |
  *  |     |
  *  +-----+
  *  v0    v1
  *
  *
  *  T
  *  ^
  *  |
  *  |
  *  +---> S
  */
template <class T, class U>
T computeBilinear(const T &v0, const T &v1, const T &v2, const T &v3, const U &s, const U &t)
{
	T h0 = t * v3 + ((U) 1 - t) * v0;
	T h1 = t * v2 + ((U) 1 - t) * v1;
	return s * h1 + ((U) 1 - s) * h0;
}

// ***************************************************************************
/** Select all points crossed by the line [(x0,y0) ; (x1,y1)]
 *  Not the same than brensenham
 */
void drawFullLine (float x0, float y0, float x1, float y1, std::vector<std::pair<sint, sint> > &result);

// ***************************************************************************
/** Select points on the line [(x0,y0) ; (x1,y1)]
 */
void drawLine (float x0, float y0, float x1, float y1, std::vector<std::pair<sint, sint> > &result);


// ***************************************************************************
/**	Search the lower_bound in a sorted array of Value, in growing order (0, 1, 2....).
 *	operator<= is used to perform the comparison.
 *	It return the first element such that array[id]<=key
 *	If not possible, 0 is returned
 *	NB: but 0 may still be a good value, so you must check whether or not 0 means "Not found", or "Id 0".
 */
template<class T>
uint		searchLowerBound(const T *array, uint arraySize, const T &key)
{
	uint	start=0;
	uint	end= arraySize;
	// find lower_bound by dichotomy
	while(end-1>start)
	{
		uint	pivot= (end+start)/2;
		// return the lower_bound, ie return first start with array[pivot]<=key
		if(array[pivot] <= key)
			start= pivot;
		else
			end= pivot;
	}

	return start;
}


// ***************************************************************************
/**	Search the lower_bound in a sorted array of Value, in growing order (0, 1, 2....).
 *	operator<= is used to perform the comparison.
 *	It returns the first element such that array[id]<=key
 *	If not possible, 0 is returned
 *	NB: but 0 may still be a good value, so you must check whether or not 0 means "Not found", or "Id 0".
 */
template<class T>
uint		searchLowerBound(const std::vector<T> &array, const T &key)
{
	uint	size= (uint)array.size();
	if(size==0)
		return 0;
	else
		return searchLowerBound(&array[0], size, key);
}


// ***************************************************************************
/** Clamp a sint in 0..255. Avoid cond jump.
 */
static inline	void fastClamp8(sint &v)
{
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	// clamp v in 0..255 (no cond jmp)
	__asm
	{
		mov		esi, v
		mov		eax, [esi]
		mov		ebx, eax
		// clamp to 0.
		add		eax, 0x80000000
		sbb		ecx, ecx
		not		ecx
		and		ebx, ecx
		// clamp to 255.
		add		eax, 0x7FFFFF00
		sbb		ecx, ecx
		or		ebx, ecx
		and		ebx, 255
		// store
		mov		[esi], ebx
	}
#else
	clamp(v, 0, 255);
#endif
}


// ***************************************************************************
/** return true if the string strIn verify the wildcard string wildCard.
 *	eg:
 *		testWildCard("azert", "*")== true
 *		testWildCard("azert", "??er*")== true
 *		testWildCard("azert", "*er*")== true
 *		testWildCard("azert", "azert*")== true
 *	Undefined result if s has some '*',
 *	return false if wildcard has some "**" or "*?"
 *	NB: case-sensitive
 */
bool		testWildCard(const char *strIn, const char *wildCard);

bool		testWildCard(const std::string &strIn, const std::string &wildCard);


// ***************************************************************************
/** From a string with some separators, build a vector of string.
 *	eg: splitString("hello|bye|||bee", "|", list) return 3 string into list: "hello", "bye" and "bee".
 */
void		splitString(const std::string &str, const std::string &separator, std::vector<std::string> &retList);

void		splitUCString(const ucstring &ucstr, const ucstring &separator, std::vector<ucstring> &retList);

// ***************************************************************************
/// In a string or ucstring, find a substr and replace it with another. return true if replaced
template<class T, class U>
bool		strFindReplace(T &str, const T &strFind, const U &strReplace)
{
	typename T::size_type pos= str.find(strFind);
	if(pos != T::npos)
	{
		str.replace(pos, strFind.size(), T(strReplace) );
		return true;
	}
	else return false;
}

template<class T, class U>
bool		strFindReplace(T &str, const char *strFind, const U &strReplace)
{
	T	tempStr(strFind);
	return strFindReplace(str, tempStr, strReplace);
}

template<class T>
T strFindReplaceAll(const T &str, const T &search, const T &replace)
{
	T ret = str;
	while (strFindReplace(ret, search, replace));
	return ret;
}

// set flags in a bit set
template <class T, class U>
inline void setFlags(T &dest, U mask, bool on)
{
	if (on)
	{
		dest = (T) (dest | (T) mask);
	}
	else
	{
		dest = (T) (dest & ~((T) mask));
	}
}


} // NLMISC


#endif // NL_ALGO_H

/* End of algo.h */
