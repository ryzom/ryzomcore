// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2015-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef	NL_COMMON_H
#define	NL_COMMON_H

#include "types_nl.h"

#include <cctype>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <cfloat>
#include <cstdarg>
#include <cstdlib>
#include <algorithm>

#ifdef NL_OS_WINDOWS
#	include <process.h>
#	include <intrin.h>
#else
#	include <unistd.h>
#	include <sys/types.h>
#endif

#if defined(NL_CPU_INTEL) && defined(NL_COMP_GCC)
#include <x86intrin.h>
#endif

#include "string_common.h"

#ifdef NL_OS_WINDOWS
	struct nameHWND__;
	typedef struct HWND__ *HWND;
	typedef HWND nlWindow;
	#define EmptyWindow NULL
#elif defined(NL_OS_MAC)
	// TODO This should be NSView*, but then we would need to include Cocoa.h
	//   and compile with "-x objective-c++" ... everything including this file.
	typedef void* nlWindow;
	#define EmptyWindow NULL
#elif defined(NL_OS_UNIX)
	typedef int nlWindow;
	#define EmptyWindow 0
#endif

/// This namespace contains all miscellaneous classes used by other modules
namespace	NLMISC
{

/** Read the time stamp counter. Supports only Intel architectures for now
  */
#ifdef NL_CPU_INTEL

inline uint64 rdtsc()
{
#if defined(NL_COMP_GCC) && !defined(CLANG_VERSION) && (GCC_VERSION <= 40405)
// for GCC versions that don't implement __rdtsc()
#ifdef NL_CPU_X86_64
	uint64 low, high;
	__asm__ volatile("rdtsc" : "=a" (low), "=d" (high));
	return low | (high << 32);
#else
	uint64 ticks;
	__asm__ volatile("rdtsc" : "=A" (ticks));
	return ticks;
#endif
#else
	return uint64(__rdtsc());
#endif
}

#endif	// NL_CPU_INTEL


/** breakable statement, used to allow break call inside parenthesis.
 */
#define		breakable	\
	switch(1) case 1: default:


/** Pi constant in double format.
 */
const double Pi = 3.1415926535897932384626433832795;


// retrieve size of a static array
#define sizeofarray(v) (sizeof(v) / sizeof((v)[0]))

/** Return a float random inside the interval [0,mod]
 */
inline float frand(float mod)
{
	double	r = (double) rand();
	r/= (double) RAND_MAX;
	return (float)(r * mod);
}


/** Return -1 if f<0, 0 if f==0, 1 if f>1
 */
inline sint fsgn(double f)
{
	if(f<0)
		return -1;
	else if(f>0)
		return 1;
	else
		return 0;
}


/** Return the square of a number
 */
template<class T>	inline T sqr(const T &v)
{
	return v * v;
}


/** Force v to be inside the interval [min,max]. Warning: implicit cast are made if T,U or V are different.
 */
template<class T, class U, class V>	inline void clamp(T &v, const U &min, const V &max)
{
	v = (v < min) ? min : v;
	v = (v > max) ? max : v;
}


/** MIN/MAX extended functions.
 */
template<class T>	inline T minof(const T& a,  const T& b,  const T& c)
	{return std::min(std::min(a,b),c);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::min(minof(a,b,c),d);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::min(minof(a,b,c,d),e);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c)
	{return std::max(std::max(a,b),c);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::max(maxof(a,b,c),d);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::max(maxof(a,b,c,d),e);}

/** \c contReset take a container like std::vector or std::deque and put his size to 0 like \c clear() but free all buffers.
 * This function is useful because \c resize(), \c clear(), \c erase() or \c reserve() methods never realloc when the array size come down.
 * \param a is the container to reset.
 */
template<class T>	inline void contReset (T& a)
{
	a.~T();
	new (&a) T;
}

/** Return the value maximized to the next power of 2 of v.
 * Example:
 *   raiseToNextPowerOf2(8) is 8
 *   raiseToNextPowerOf2(5) is 8
 */
uint			raiseToNextPowerOf2 (uint v);

/** Return the power of 2 of v.
 * Example:
 *   getPowerOf2(8) is 3
 *   getPowerOf2(5) is 3
 */
uint			getPowerOf2 (uint v);

/** Return \c true if the value is a power of 2.
 */
bool			isPowerOf2 (sint32 v);


/** Converts from degrees to radians
 */
inline float	degToRad( float deg )
{
	return deg * (float)Pi / 180.0f;
}


/** Converts from radians to degrees
 */
inline float	radToDeg( float rad )
{
	return rad * 180.0f / (float)Pi;
}


/** Return true if double is a valid value (not inf nor nan)
 */
inline double	isValidDouble (double v)
{
#ifdef NL_OS_WINDOWS
	return _finite(v) && !_isnan(v);
#else
#ifdef _STLPORT_VERSION
	return !isnan(v) && !isinf(v);
#else
	return !std::isnan(v) && !std::isinf(v);
#endif
#endif
}


/** Convert a string in lower case.
 * \param str a string to transform to lower case
 */

std::string toLower ( const char *str ); // UTF-8
std::string	toLower ( const std::string &str ); // UTF-8
void		toLower ( char *str ); // Ascii only
char		toLower ( const char ch );	// convert only one character

/** Convert a string in upper case.
 * \param a string to transform to upper case
 */

std::string toUpper ( const char *str ); // UTF-8
std::string	toUpper ( const std::string &str); // UTF-8
void		toUpper ( char *str); // Ascii only

/** Convert a single character in UTF-8 to upper or lowercase.
* \param res Character is appended in UTF-8 into this string.
* \param src Character is sourced from this UTF-8 string.
* \param i Index in `str`, incremented by the number of bytes read.
*/
void appendToLower(std::string &res, const char *str, ptrdiff_t &i);
void appendToLower(std::string &res, const std::string &str, ptrdiff_t &i);
void appendToUpper(std::string &res, const char *str, ptrdiff_t &i);
void appendToUpper(std::string &res, const std::string &str, ptrdiff_t &i);
void appendToTitle(std::string &res, const char *str, ptrdiff_t &i);
void appendToTitle(std::string &res, const std::string &str, ptrdiff_t &i);

/** UTF-8 case insensitive compare */
int compareCaseInsensitive(const char *a, const char *b);
int compareCaseInsensitive(const char *a, size_t lenA, const char *b, size_t lenB);
inline int compareCaseInsensitive(const std::string &a, const std::string &b) { return compareCaseInsensitive(&a[0], a.size(), &b[0], b.size()); }
inline bool ltCaseInsensitive(const std::string &a, const std::string &b) { return compareCaseInsensitive(&a[0], a.size(), &b[0], b.size()) < 0; }
std::string	toCaseInsensitive(const char *str); // UTF-8, case-insensitive toLower
std::string	toCaseInsensitive(const std::string &str); // UTF-8, case-insensitive toLower

/** ASCII to lowercase. Useful for internal identifiers.
* Characters outside of the 7-bit ASCII space, and control characters, are replaced.
*/
std::string toLowerAscii(const std::string &str, char replacement);
void toLowerAscii(char *str, char replacement);

/** ASCII to uppercase. Useful for internal identifiers.
* Characters outside of the 7-bit ASCII space, and control characters, are replaced.
*/
std::string toUpperAscii(const std::string &str, char replacement);
void toUpperAscii(char *str, char replacement);

/** ASCII to lowercase. Useful for internal identifiers.
* Characters outside of the 7-bit ASCII space are not affected.
*/
std::string toLowerAscii(const std::string &str);
void toLowerAscii(char *str);

/** ASCII to uppercase. Useful for internal identifiers.
* Characters outside of the 7-bit ASCII space are not affected.
*/
std::string toUpperAscii(const std::string &str);
void toUpperAscii(char *str);

/**
 *  Convert to an hexadecimal std::string
 */
std::string toHexa(const uint8 &b);
std::string toHexa(const uint8 *data, uint size);
std::string toHexa(const std::string &str);
std::string toHexa(const char *str);

/**
*  Convert from an hexadecimal std::string
*/
bool fromHexa(const std::string &hexa, uint8 &b);
bool fromHexa(const std::string &hexa, uint8 *data);
bool fromHexa(const std::string &hexa, std::string &str);
bool fromHexa(const char *hexa, uint8 &b);
bool fromHexa(const char *hexa, uint8 *data);
bool fromHexa(const char *hexa, std::string &str);
bool fromHexa(const char hexa, uint8 &b);

// Remove all the characters <= 32 (tab, space, new line, return, vertical tab etc..) at the beginning and at the end of a string
template <class T> T trim (const T &str)
{
	typename T::size_type start = 0;
	const typename T::size_type size = str.size();
	while (start < size && str[start] <= 32)
		start++;
	typename T::size_type end = size;
	while (end > start && str[end-1] <= 32)
		end--;
	return str.substr (start, end-start);
}

// remove spaces at the end of the string
template <class T> T trimRightWhiteSpaces (const T &str)
{
	typename T::size_type end = str.size();
	while (end > 0 && str[end-1] == ' ')
		end--;
	return str.substr (0, end);
}

// remove spaces and tabs at the begin and end of the string
template <class T> T trimSeparators (const T &str)
{
	typename T::size_type start = 0;
	typename T::size_type size = str.size();
	while (start < size && (str[start] == ' ' || str[start] == '\t'))
		start++;
	typename T::size_type end = size;
	while (end > start && (str[end-1] == ' ' || str[end-1] == '\t'))
		end--;
	return str.substr (start, end-start);
}

// if both first and last char are quotes (' or "), then remove them
template <class T> T trimQuotes (const T &str)
{
	typename T::size_type size = str.size();
	if (size == 0)
		return str;
	if (str[0] != str[size-1])
		return str;
	if (str[0] != '"' && str[0] != '\'')
		return str;
	return str.substr(1, size - 2);
}

// encode/decode uri component using %AB hex encoding
std::string encodeURIComponent(const std::string &in);
std::string decodeURIComponent(const std::string &in);

//////////////////////////////////////////////////////////////////////////
// ****  DEPRECATED *****: PLEASE DON'T USE THESE METHODS BUT FUNCTIONS ABOVE toLower() and toUpper()
//////////////////////////////////////////////////////////////////////////
inline std::string		&strlwr ( std::string &str )		{ str = toLower(str); return str; }
inline std::string		 strlwr ( const std::string &str )	{ return toLower(str); }
inline char			*strlwr ( char *str )				{ toLower(str); return str; }
inline std::string		&strupr ( std::string &str )		{ str = toUpper(str); return str; }
inline std::string		 strupr ( const std::string &str )	{ return toUpper(str); }
inline char			*strupr ( char *str )				{ toUpper(str); return str; }


/** Compare 2 C-Style strings without regard to case
  * \return 0 if strings are equal, < 0 if lhs < rhs, > 0 if lhs > rhs
  *
  * On Windows,   use stricmp
  * On GNU/Linux, create stricmp using strcasecmp and use stricmp
  */
#ifndef NL_OS_WINDOWS
inline int stricmp(const char *lhs, const char *rhs) { return strcasecmp(lhs, rhs); }
inline int strnicmp(const char *lhs, const char *rhs, size_t n) { return strncasecmp(lhs, rhs, n); }
#endif

inline sint nlstricmp(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }
inline sint nlstricmp(const std::string &lhs, const std::string &rhs) { return stricmp(lhs.c_str(), rhs.c_str()); }
inline sint nlstricmp(const std::string &lhs, const char *rhs) { return stricmp(lhs.c_str(),rhs); }
inline sint nlstricmp(const char *lhs, const std::string &rhs) { return stricmp(lhs,rhs.c_str()); }

#if (NL_COMP_VC_VERSION <= 90)
inline float nlroundf(float x)
{
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}
#define roundf(x) NLMISC::nlroundf(x)
#endif

// Wrapper for fopen to be able to open files with an UTF-8 filename
FILE *nlfopen(const std::string &filename, const std::string &mode);

/** Signed 64 bit fseek. Same interface as fseek
  */
int nlfseek64(FILE *stream, sint64 offset, int origin);

// Retrieve position in a file, same interface as ftell
sint64 nlftell64(FILE *stream);

/**
 * Base class for all NeL exception.
 * It enables to construct simple string at the ctor.
 */
class Exception : public std::exception
{
protected:
	std::string	_Reason;
public:
	Exception();
	Exception(const std::string &reason);
	Exception(const char *format, ...);
	virtual ~Exception() NL_OVERRIDE {}
	virtual const char	*what() const throw() NL_OVERRIDE;
};


/**
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms );


/// Returns Process Id (note: on Linux, Process Id is the same as the Thread Id)
#ifdef NL_OS_WINDOWS
#	define getpid _getpid
#endif

/// Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
size_t getThreadId();

/// Returns a readable string from a vector of bytes. unprintable char are replaced by '?'
std::string stringFromVector( const std::vector<uint8>& v, bool limited = true );


/// Convert a string into an sint64 (same as atoi() function but for 64 bits intergers)
sint64 atoiInt64 (const char *ident, sint64 base = 10);

/// Convert an sint64 into a string (same as itoa() function but for 64 bits intergers)
void itoaInt64 (sint64 number, char *str, sint64 base = 10);


/// Convert a number in bytes into a string that is easily readable by an human, for example 105123 -> "102kb"
std::string bytesToHumanReadable (const std::string &bytes);
std::string bytesToHumanReadable (uint64 bytes);

/// Convert a number in bytes into a string that is easily readable by an human, for example 105123 -> "102kb"
/// Using units array as string: 0 => B, 1 => KiB, 2 => MiB, 3 => GiB, etc...
std::string bytesToHumanReadableUnits (uint64 bytes, const std::vector<std::string> &units);

/// Convert a human readable into a bytes,  for example "102kb" -> 105123
uint32 humanReadableToBytes (const std::string &str);

/// Convert a time into a string that is easily readable by an human, for example 3600 -> "1h"
std::string secondsToHumanReadable (uint32 time);

/// Convert a UNIX timestamp to a formatted date in ISO format
std::string timestampToHumanReadable(uint32 timestamp);

/// Get a bytes or time in string format and convert it in seconds or bytes
uint32 fromHumanReadable (const std::string &str);

/// Add digit grouping seperator to if value >= 10 000. Assumes input is numerical string.
std::string formatThousands(const std::string& s);

/// This function executes a program in the background and returns instantly (used for example to launch services in AES).
/// The program will be launched in the current directory
bool launchProgram (const std::string &programName, const std::string &arguments, bool log = true);

/// Same but with an array of strings for arguments
bool launchProgramArray (const std::string &programName, const std::vector<std::string> &arguments, bool log = true);

/// This function executes a program and wait for result (used for example for crash report).
/// The program will be launched in the current directory
sint launchProgramAndWaitForResult (const std::string &programName, const std::string &arguments, bool log = true);

/// This function executes a program and returns output as a string
std::string getCommandOutput(const std::string &command);

/// This function replace all environment variables in a string by their content.
/// Environment variables names can use both Windows (%NAME%) and UNIX syntax ($NAME)
/// Authorized characters in names are A-Z, a-z, 0-9 and _
std::string expandEnvironmentVariables(const std::string &s);

/// Functions to convert a string with arguments to array or array to string (will espace strings with spaces)
bool explodeArguments(const std::string &str, std::vector<std::string> &args);
std::string joinArguments(const std::vector<std::string> &args);

/// Escape an argument to not evaluate environment variables or special cases
std::string escapeArgument(const std::string &arg);

/// This function kills a program using his pid (on unix, it uses the kill() POSIX function)
bool killProgram(uint32 pid);

/// This function kills a program using his pid with abort signal (on unix, it uses the kill() POSIX function)
bool abortProgram(uint32 pid);

/** Returns a string corresponding to the class T in string format.
 * Example:
 *  string num = toString (1234); // num = "1234";
 */
/*acetemplate<class T> std::string toString (const T &t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}
*/

/** Returns a string corresponding to the format and parameter (like printf).
 * Example:
 *  string hexnum = toString ("%x", 255); // hexnum = "ff";
 */
/*#ifdef NL_OS_WINDOWS
inline std::string _toString (const char *format, ...)
#else
inline std::string toString (const char *format, ...)
#endif
{
	std::string Result;
	NLMISC_CONVERT_VARGS (Result, format, NLMISC::MaxCStringSize);
	return Result;
}

#ifdef NL_OS_WINDOWS
CHECK_TYPES(std::string toString, return _toString)
#endif // NL_OS_WINDOWS



#ifdef NL_OS_UNIX
inline std::string toString (const uint8 &t)
{
	std::stringstream ss;
	ss << (unsigned int)t;
	return ss.str();
}

inline std::string toString (const sint8 &t)
{
	std::stringstream ss;
	ss << (unsigned int)t;
	return ss.str();
}
#endif // NL_OS_UNIX
*/

/** Explode a string (or ucstring) into a vector of string with *sep* as separator. If sep can be more than 1 char, in this case,
 * we find the entire sep to separator (it s not a set of possible separator)
 *
 * \param skipEmpty if true, we don't put in the res vector empty string
 */
template <class T> void explode (const T &src, const T &sep, std::vector<T> &res, bool skipEmpty = false)
{
	std::string::size_type oldpos = 0, pos;

	res.clear ();

	do
	{
		pos = src.find (sep, oldpos);
		T s;
		if(pos == std::string::npos)
			s = src.substr (oldpos);
		else
			s = src.substr (oldpos, (pos-oldpos));

		if (!skipEmpty || !s.empty())
			res.push_back (s);

		oldpos = pos+sep.size();
	}
	while(pos != std::string::npos);
}

/** Join a string (or ucstring) from a vector of strings with *sep* as separator. If sep can be more than 1 char, in this case,
* we find the entire sep to separator (it s not a set of possible separator)
*/
template <class T, class U> void join(const std::vector<T>& strings, const U& separator, T &res)
{
	res.clear();

	for (uint i = 0, len = strings.size(); i<len; ++i)
	{
		// add in separators before all but the first string
		if (!res.empty()) res += separator;

		// append next string
		res += strings[i];
	}
}

/* All the code above is used to add our types (uint8, ...) in the stringstream (used by the toString() function).
 * So we can use stringstream operator << and >> with all NeL simple types (except for ucchar and ucstring)
 */
/*
#ifdef NL_OS_WINDOWS

#if _MSC_VER < 1300	// visual or older (on visual .NET, we don't need to do that)

#define NLMISC_ADD_BASIC_ISTREAM_OPERATOR(__type,__casttype) \
template <class _CharT, class _Traits> \
std::basic_istream<_CharT, _Traits>& __cdecl \
operator>>(std::basic_istream<_CharT, _Traits>& __is, __type& __z) \
{ \
	__casttype __z2 = (__casttype) __z; \
	__is.operator>>(__z2); \
	__z = (__type) __z2; \
	return __is; \
} \
 \
template <class _CharT, class _Traits> \
std::basic_ostream<_CharT, _Traits>& __cdecl \
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const __type& __z) \
{ \
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp; \
	__tmp << (__casttype) __z; \
	return __os << __tmp.str(); \
}

NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint8, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint8, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint16, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint16, signed int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(uint32, unsigned int);
NLMISC_ADD_BASIC_ISTREAM_OPERATOR(sint32, signed int);

#endif // _MSC_VER < 1300


template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __cdecl
operator>>(std::basic_istream<_CharT, _Traits>& __is, uint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = 0;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __cdecl
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const uint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __res;

	if (__z == 0)
	{
		__res << '0';
	}
	else
	{
		std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;
		uint64	__z2 = __z;
		while (__z2 != 0)
		{
			__tmp << (char)((__z2%10)+'0');
			__z2 /= 10;
		}

		uint __s = __tmp.str().size();
		for (uint i = 0; i < __s; i++)
			__res << __tmp.str()[__s - 1 - i];
	}
	return __os << __res.str();
}

template <class _CharT, class _Traits>
std::basic_istream<_CharT, _Traits>& __cdecl
operator>>(std::basic_istream<_CharT, _Traits>& __is, sint64& __z)
{
	__z = 0;
	bool neg = false;
	char c;
	do
	{
		__is >> c;
	}
	while (isspace(c));

	if (c == '-')
	{
		neg = true;
		__is >> c;
	}

	while (isdigit(c))
	{
		__z *= 10;
		__z += c-'0';
		__is >> c;
		if (__is.fail())
			break;
	}

	if (neg) __z = -__z;

	return __is;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>& __cdecl
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const sint64& __z)
{
	std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __res;

	if (__z == 0)
	{
		__res << '0';
	}
	else
	{
		sint64	__z2 = __z;

		if (__z2 < 0)
		{
			__res << '-';
		}

		std::basic_ostringstream<_CharT, _Traits, std::allocator<_CharT> > __tmp;
		while (__z2 != 0)
		{
			if (__z2 < 0)
			{
				__tmp << (char)((-(__z2%10))+'0');
			}
			else
			{
				__tmp << (char)((__z2%10)+'0');
			}
			__z2 /= 10;
		}

		uint __s = __tmp.str().size();
		for (uint i = 0; i < __s; i++)
			__res << __tmp.str()[__s - 1 - i];
	}
	return __os << __res.str();
}

#endif // NL_OS_WINDOWS
*/

class CLog;

/// Display the bits (with 0 and 1) composing a byte (from right to left)
void displayByteBits( uint8 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log );

/// Display the bits (with 0 and 1) composing a number (uint32) (from right to left)
void displayDwordBits( uint32 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log );

/// this wrapping is due to a visual bug when calling isprint with big value
/// example of crash with VC6 SP4:	int a = isprint(0x40e208);
#ifdef NL_OS_WINDOWS
inline int nlisprint(int c)
{
	if(c>255||c<0) return 0;
	return isprint(c);
}
#else
#define nlisprint isprint
#endif

// Open an url in a browser
bool openURL (const std::string &url);

// Open a document
bool openDoc (const std::string &document);

// AntiBug method that return an epsilon if x==0, else x
inline float	favoid0(float x)
{
	if(x==0)	return 0.00001f;
	return x;
}
inline double	davoid0(double x)
{
	if(x==0)	return 0.00001;
	return x;
}
// AntiBug method that return 1 if x==0, else x
template<class T>
inline T		iavoid0(T x)
{
	if(x==0)	return 1;
	return x;
}

// Helper to convert in memory between types of different sizes
union C64BitsParts
{
	// unsigned
	uint64 u64[1];
	uint32 u32[2];
	uint16 u16[4];
	uint8 u8[8];

	// signed
	sint64 i64[1];
	sint32 i32[2];
	sint16 i16[4];
	sint8 i8[8];

	// floats
	double d[1];
	float f[2];
};

} // NLMISC

#endif	// NL_COMMON_H
