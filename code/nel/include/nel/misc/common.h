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
	uint64 ticks;
#	ifdef NL_OS_WINDOWS
#	ifdef NL_NO_ASM
		ticks = uint64(__rdtsc());
#	else
		// We should use the intrinsic code now. ticks = uint64(__rdtsc());
		__asm	rdtsc
		__asm	mov		DWORD PTR [ticks], eax
		__asm	mov		DWORD PTR [ticks + 4], edx
#	endif // NL_NO_ASM
#	else
		__asm__ volatile(".byte 0x0f, 0x31" : "=a" (ticks.low), "=d" (ticks.high));
#	endif // NL_OS_WINDOWS
	return ticks;
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

std::string	toLower ( const std::string &str );
void		toLower ( char *str );
char		toLower ( const char ch );	// convert only one character

/** Convert a string in upper case.
 * \param a string to transform to upper case
 */

std::string	toUpper ( const std::string &str);
void		toUpper ( char *str);

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

/** Signed 64 bit fseek. Same interface as fseek
  */
int		nlfseek64( FILE *stream, sint64 offset, int origin );

// Retrieve position in a file, same interface as ftell
sint64  nlftell64(FILE *stream);

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
	virtual ~Exception() throw() {}
	virtual const char	*what() const throw();
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

/// Convert a human readable into a bytes,  for example "102kb" -> 105123
uint32 humanReadableToBytes (const std::string &str);

/// Convert a time into a string that is easily readable by an human, for example 3600 -> "1h"
std::string secondsToHumanReadable (uint32 time);


/// Get a bytes or time in string format and convert it in seconds or bytes
uint32 fromHumanReadable (const std::string &str);

/// Add digit grouping seperator to if value >= 10 000. Assumes input is numerical string.
std::string formatThousands(const std::string& s);

/// This function executes a program in the background and returns instantly (used for example to launch services in AES).
/// The program will be launched in the current directory
bool launchProgram (const std::string &programName, const std::string &arguments);

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
bool openURL (const char *url);

// Open a document
bool openDoc (const char *document);

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


} // NLMISC

#endif	// NL_COMMON_H
