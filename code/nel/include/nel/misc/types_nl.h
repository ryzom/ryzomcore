// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2011  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_TYPES_H
#define NL_TYPES_H

// Wrapper for the clib time function
#define nl_time time
#define nl_mktime mktime
#define nl_gmtime gmtime
#define nl_localtime localtime
#define nl_difftime difftime

// nelconfig.h inclusion, file generated by autoconf
#ifdef HAVE_NELCONFIG_H
#	include "nelconfig.h"
#endif // HAVE_NELCONFIG_H

#ifdef FINAL_VERSION
	// If the FINAL_VERSION is defined externally, check that the value is 0 or 1
#	if FINAL_VERSION != 1 && FINAL_VERSION != 0
#		error "Bad value for FINAL_VERSION, it must be 0 or 1"
#	endif
#else
	// If you want to compile in final version just put 1 instead of 0
	// WARNING: never comment this #define
#	define FINAL_VERSION 0
#endif // FINAL_VERSION

// This way we know about _HAS_TR1 and _STLPORT_VERSION
#include <string>

#if defined(HAVE_X86_64)
#	define NL_CPU_INTEL
#	define NL_CPU_X86_64
// x86_64 CPU always have SSE2 instructions
#	ifndef NL_HAS_SSE2
#		define NL_HAS_SSE2
#	endif
#elif defined(HAVE_X86)
#	define NL_CPU_INTEL
#	define NL_CPU_X86
#endif

// Operating systems definition
#ifdef _WIN32
#	define NL_OS_WINDOWS
#	define NL_LITTLE_ENDIAN
#	ifndef NL_CPU_INTEL
#		define NL_CPU_INTEL
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0500	// Minimal OS = Windows 2000 (NeL is not supported on Windows 95/98)
#	endif
#	ifdef _MSC_VER
#		define NL_COMP_VC
#		if _MSC_VER >= 1900
#			define NL_COMP_VC14
#			define NL_COMP_VC_VERSION 140
#		elif _MSC_VER >= 1800
#			define NL_COMP_VC12
#			define NL_COMP_VC_VERSION 120
#		elif _MSC_VER >= 1700
#			define NL_COMP_VC11
#			define NL_COMP_VC_VERSION 110
#		elif _MSC_VER >= 1600
#			define NL_COMP_VC10
#			define NL_COMP_VC_VERSION 100
#			ifdef _HAS_CPP0X
#				undef _HAS_CPP0X	// VC++ 2010 doesn't implement C++11 stuff we need
#			endif
#		elif _MSC_VER >= 1500
#			define NL_COMP_VC9
#			define NL_COMP_VC_VERSION 90
#		elif _MSC_VER >= 1400
#			define NL_COMP_VC8
#			define NL_COMP_VC_VERSION 80
#			undef nl_time
#			define nl_time _time32		// use the old 32 bit time function
#			undef nl_mktime
#			define nl_mktime _mktime32	// use the old 32 bit time function
#			undef nl_gmtime
#			define nl_gmtime _gmtime32	// use the old 32 bit time function
#			undef nl_localtime
#			define nl_localtime _localtime32	// use the old 32 bit time function
#			undef nl_difftime
#			define nl_difftime _difftime32	// use the old 32 bit time function
#		elif _MSC_VER >= 1310
#			define NL_COMP_VC71
#			define NL_COMP_VC_VERSION 71
#		elif _MSC_VER >= 1300
#			define NL_COMP_VC7
#			define NL_COMP_VC_VERSION 70
#		elif _MSC_VER >= 1200
#			define NL_COMP_VC6
#			define NL_COMP_VC_VERSION 60
#			define NL_COMP_NEED_PARAM_ON_METHOD
#		endif
#	elif defined(__MINGW32__)
#		define NL_COMP_MINGW
#		define NL_COMP_GCC
#		define NL_NO_ASM
#	endif
#	if defined(_HAS_TR1) && (_HAS_TR1 + 0) // VC9 TR1 feature pack or later
#		define NL_ISO_STDTR1_AVAILABLE
#		define NL_ISO_STDTR1_HEADER(header) <header>
#		define NL_ISO_STDTR1_NAMESPACE std::tr1
#	endif
#	ifdef _DEBUG
#		ifndef NL_DEBUG
#			define NL_DEBUG
#		endif
#	elif defined (NDEBUG)
#		ifndef NL_RELEASE
#			define NL_RELEASE
#		endif
#	else
#		error "Don't know the compilation mode"
#	endif
#	ifdef _WIN64
#		define NL_OS_WIN64
#		ifndef NL_NO_ASM
			// Windows 64bits platform SDK compilers doesn't support inline assembler
#			define NL_NO_ASM
#		endif
#		undef _WIN32_WINNT
#		define _WIN32_WINNT 0x0600 // force VISTA minimal version in 64 bits
#	endif
	// define NOMINMAX to be sure that windows includes will not define min max macros, but instead, use the stl template
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#else
#	ifdef __APPLE__
#		define NL_OS_MAC
#		ifdef __BIG_ENDIAN__
#			define NL_BIG_ENDIAN
#		elif defined(__LITTLE_ENDIAN__)
#			define NL_LITTLE_ENDIAN
#		else
#			error "Cannot detect the endianness of this Mac"
#		endif
#	else
#		ifdef WORDS_BIGENDIAN
#			define NL_BIG_ENDIAN
#		else
#			define NL_LITTLE_ENDIAN
#		endif
#	endif
// these define are set the GNU/Linux and Mac OS
#	define NL_OS_UNIX
#	define NL_COMP_GCC
#endif

#if defined(_HAS_CPP0X) || defined(__GXX_EXPERIMENTAL_CXX0X__) || (defined(NL_COMP_VC_VERSION) && NL_COMP_VC_VERSION >= 110)
#	define NL_ISO_CPP0X_AVAILABLE
#endif

#if defined(NL_COMP_GCC) && (__cplusplus >= 201103L)
#	define NL_NO_EXCEPTION_SPECS
#endif

#if defined(NL_COMP_VC) && (NL_COMP_VC_VERSION >= 140)
#define nlmove(v) std::move(v)
#else
#define nlmove(v) (v)
#endif

// gcc 3.4 introduced ISO C++ with tough template rules
//
// NL_ISO_SYNTAX can be used using #if NL_ISO_SYNTAX or #if !NL_ISO_SYNTAX
//
// NL_ISO_TEMPLATE_SPEC can be used in front of an instanciated class-template member data definition,
// because sometimes MSVC++ 6 produces an error C2908 with a definition with template <>.
#if defined(NL_COMP_VC) || (defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)))
#	define NL_ISO_SYNTAX 0
#	define NL_ISO_TEMPLATE_SPEC
#else
#	define NL_ISO_SYNTAX 1
#	define NL_ISO_TEMPLATE_SPEC template <>
#endif

// gcc 4.1+ provides std::tr1
#ifdef NL_COMP_GCC
#	define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#	if GCC_VERSION > 40100
		// new libc++ bundled with clang under Mac OS X 10.9+ doesn't define __GLIBCXX__
#		ifdef __GLIBCXX__
#			define NL_ISO_STDTR1_AVAILABLE
#			define NL_ISO_STDTR1_HEADER(header) <tr1/header>
#			define NL_ISO_STDTR1_NAMESPACE std::tr1
#		else
#			define NL_ISO_STDTR1_AVAILABLE
#			define NL_ISO_STDTR1_HEADER(header) <header>
#			define NL_ISO_STDTR1_NAMESPACE std
#		endif
#	endif
	// clang define GCC version for compatibility
#	ifdef __clang__
#		define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	endif
#endif

// Remove stupid Visual C++ warnings
#ifdef NL_OS_WINDOWS
#	pragma warning (disable : 4503)			// STL: Decorated name length exceeded, name was truncated
#	pragma warning (disable : 4786)			// STL: too long identifier
#	pragma warning (disable : 4290)			// throw() not implemented warning
#	pragma warning (disable : 4250)			// inherits via dominance (informational warning).
#	pragma warning (disable : 4390)			// don't warn in empty block "if(exp) ;"
#	pragma warning (disable : 4996)			// 'vsnprintf': This function or variable may be unsafe. Consider using vsnprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
// Debug : Sept 01 2006
#	if defined(NL_COMP_VC8) || defined(NL_COMP_VC9) || defined(NL_COMP_VC10)
#		pragma warning (disable : 4005)			// don't warn on redefinitions caused by xp platform sdk
#	endif // NL_COMP_VC8 || NL_COMP_VC9
#	pragma warning (disable : 26495)		// Variable is uninitialized. Always initialize a member variable. (On purpose for performance.)
#endif // NL_OS_WINDOWS


// Standard include

#include <string>
#include <exception>

// Setup extern asm functions.

#ifndef NL_NO_ASM							// If NL_NO_ASM is externely defined, don't override it.
#	ifndef NL_CPU_INTEL						// If not on an Intel compatible plateforme (BeOS, 0x86 Linux, Windows)
#		define NL_NO_ASM						// Don't use extern ASM. Full C++ code.
#	endif // NL_CPU_INTEL
#endif // NL_NO_ASM


// Define this if you want to use GTK for gtk_displayer

//#define NL_USE_GTK
//#undef NL_USE_GTK

// Define this if you want to remove all assert, debug code...
// You should never need to define this since it's always good to have assert, even in release mode

//#define NL_NO_DEBUG
#undef NL_NO_DEBUG


// Standard types

/*
 * correct numeric types:	sint8, uint8, sint16, uint16, sint32, uint32, sint64, uint64, sint, uint
 * correct char types:		char, string, ucchar, ucstring
 * correct misc types:		void, bool, float, double
 *
 */

/**
 * \typedef uint8
 * An unsigned 8 bits integer (use char only as \b character and not as integer)
 **/

/**
 * \typedef sint8
 * An signed 8 bits integer (use char only as \b character and not as integer)
 */

/**
 * \typedef uint16
 * An unsigned 16 bits integer (don't use short)
 **/

/**
 * \typedef sint16
 * An signed 16 bits integer (don't use short)
 */

/**
 * \typedef uint32
 * An unsigned 32 bits integer (don't use int or long)
 **/

/**
 * \typedef sint32
 * An signed 32 bits integer (don't use int or long)
 */

/**
 * \typedef uint64
 * An unsigned 64 bits integer (don't use long long or __int64)
 **/

/**
 * \typedef sint64
 * An signed 64 bits integer (don't use long long or __int64)
 */

/**
 * \typedef uint
 * An unsigned integer, at least 32 bits (used only for internal loops or speedy purpose, processor dependent)
 **/

/**
 * \typedef sint
 * An signed integer at least 32 bits (used only for internal loops or speedy purpose, processor dependent)
 */

/**
 * \def NL_I64
 * Used to display a int64 in a platform independent way with printf like functions.
 \code
 sint64 myint64 = SINT64_CONSTANT(0x123456781234);
 printf("This is a 64 bits int: %" NL_I64 "u", myint64);
 \endcode
 */

#ifdef NL_OS_WINDOWS

typedef	signed		__int8		sint8;
typedef	unsigned	__int8		uint8;
typedef	signed		__int16		sint16;
typedef	unsigned	__int16		uint16;
typedef	signed		__int32		sint32;
typedef	unsigned	__int32		uint32;
typedef	signed		__int64		sint64;
typedef	unsigned	__int64		uint64;

typedef				int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#define	NL_I64 "I64"

#elif defined (NL_OS_UNIX)

#include <sys/types.h>
#include <stdint.h>
#include <climits>

typedef	int8_t		sint8;
typedef	uint8_t		uint8;
typedef	int16_t		sint16;
typedef	uint16_t	uint16;
typedef	int32_t		sint32;
typedef	uint32_t	uint32;
typedef	int64_t		sint64;
typedef	uint64_t	uint64;

typedef				int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

// used for macro PRI*64
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#if defined(__PRI_64_LENGTH_MODIFIER__)
#	define NL_I64 __PRI_64_LENGTH_MODIFIER__
#elif defined(__PRI64_PREFIX)
#	define NL_I64 __PRI64_PREFIX
#else
#	ifdef _LP64
#		define	NL_I64 "l"
#	else
#		define	NL_I64 "ll"
#	endif // _LP64
#endif

#endif // NL_OS_UNIX


// #ifdef NL_ENABLE_FORCE_INLINE
#	ifdef NL_COMP_VC
#		define NL_FORCE_INLINE __forceinline
#	elif defined(NL_COMP_GCC)
#		define NL_FORCE_INLINE inline __attribute__((always_inline))
#	else
#		define NL_FORCE_INLINE inline
#	endif
// #else
// #	define NL_FORCE_INLINE inline
// #endif


#ifdef NL_COMP_VC
#define NL_ALIGN(nb) __declspec(align(nb))
#else
#define NL_ALIGN(nb) __attribute__((aligned(nb)))
#endif

#ifdef NL_OS_WINDOWS
#include <stdlib.h>
#include <intrin.h>
#include <malloc.h>
#define aligned_malloc(size, alignment) _aligned_malloc(size, alignment)
#define aligned_free(ptr) _aligned_free(ptr)
#elif defined(NL_OS_MAC)
#include <stdlib.h>
// under Mac OS X, malloc is already aligned for SSE and Altivec (16 bytes alignment)
#define aligned_malloc(size, alignment) malloc(size)
#define aligned_free(ptr) free(ptr)
#else
#include <malloc.h>
#define aligned_malloc(size, alignment) memalign(alignment, size)
#define aligned_free(ptr) free(ptr)
#endif /* NL_COMP_ */


#ifdef NL_HAS_SSE2

#define NL_DEFAULT_MEMORY_ALIGNMENT 16
#define NL_ALIGN_SSE2 NL_ALIGN(NL_DEFAULT_MEMORY_ALIGNMENT)

#ifdef NL_CPU_X86_64
// on x86_64, new and delete are already aligned on 16 bytes
#elif (defined(NL_COMP_VC) && defined(NL_DEBUG))
// don't use aligned memory if debugging with VC++ in 32 bits
#else
// use aligned memory in all other cases
#define NL_USE_ALIGNED_MEMORY_OPERATORS
#endif

#ifdef NL_USE_ALIGNED_MEMORY_OPERATORS

#ifdef NL_NO_EXCEPTION_SPECS
extern void *operator new(size_t size);
extern void *operator new[](size_t size);
extern void operator delete(void *p) noexcept;
extern void operator delete[](void *p) noexcept;
#else
extern void *operator new(size_t size) throw(std::bad_alloc);
extern void *operator new[](size_t size) throw(std::bad_alloc);
extern void operator delete(void *p) throw();
extern void operator delete[](void *p) throw();
#endif /* NL_NO_EXCEPTION_SPECS */

#endif /* NL_USE_ALIGNED_MEMORY_OPERATORS */

#else /* NL_HAS_SSE2 */

#define NL_DEFAULT_MEMORY_ALIGNMENT 4
#define NL_ALIGN_SSE2

#endif /* NL_HAS_SSE2 */


// CHashMap, CHashSet and CHashMultiMap definitions
#if defined(_STLPORT_VERSION) // STLport detected
#	include <hash_map>
#	include <hash_set>
#	ifdef _STLP_HASH_MAP
#		define CHashMap ::std::hash_map
#		define CHashSet ::std::hash_set
#		define CHashMultiMap ::std::hash_multimap
#	endif // _STLP_HASH_MAP
#	define CUniquePtr ::std::auto_ptr
#	define CUniquePtrMove
#elif defined(NL_ISO_CPP0X_AVAILABLE) || (defined(NL_COMP_VC) && (NL_COMP_VC_VERSION >= 100))
#	include <unordered_map>
#	include <unordered_set>
#	define CHashMap ::std::unordered_map
#	define CHashSet ::std::unordered_set
#	define CHashMultiMap ::std::unordered_multimap
#	define CUniquePtr ::std::unique_ptr
#	define CUniquePtrMove ::std::move
#elif defined(NL_ISO_STDTR1_AVAILABLE) // use std::tr1 for CHash* classes, if available (gcc 4.1+ and VC9 with TR1 feature pack)
#	include NL_ISO_STDTR1_HEADER(unordered_map)
#	include NL_ISO_STDTR1_HEADER(unordered_set)
#	define CHashMap NL_ISO_STDTR1_NAMESPACE::unordered_map
#	define CHashSet NL_ISO_STDTR1_NAMESPACE::unordered_set
#	define CHashMultiMap NL_ISO_STDTR1_NAMESPACE::unordered_multimap
#	define CUniquePtr ::std::auto_ptr
#	define CUniquePtrMove
#	define NL_OVERRIDE
#elif defined(NL_COMP_VC) && (NL_COMP_VC_VERSION >= 70 && NL_COMP_VC_VERSION <= 90) // VC7 through 9
#	include <hash_map>
#	include <hash_set>
#	define CHashMap stdext::hash_map
#	define CHashSet stdext::hash_set
#	define CHashMultiMap stdext::hash_multimap
#	define CUniquePtr ::std::auto_ptr
#	define CUniquePtrMove
#	define NL_OVERRIDE
#elif defined(NL_COMP_GCC) // GCC4
#	include <ext/hash_map>
#	include <ext/hash_set>
#	define CHashMap ::__gnu_cxx::hash_map
#	define CHashSet ::__gnu_cxx::hash_set
#	define CHashMultiMap ::__gnu_cxx::hash_multimap
#	define CUniquePtr ::std::auto_ptr
#	define CUniquePtrMove

namespace __gnu_cxx {

template<> struct hash<std::string>
{
	size_t operator()(const std::string &s) const
	{
		return __stl_hash_string(s.c_str());
	}
};

template<> struct hash<uint64>
{
	size_t operator()(const uint64 x) const
	{
		return x;
	}
};

} // END NAMESPACE __GNU_CXX

#else
#	pragma error("You need to update your compiler")
#endif // _STLPORT_VERSION

/**
 * \typedef ucchar
 * An Unicode character (16 bits)
 */
typedef	uint16	ucchar;

#ifndef NL_OVERRIDE
#define NL_OVERRIDE override
#endif

#if defined(NL_OS_WINDOWS) && (defined(UNICODE) || defined(_UNICODE))
#define nltmain wmain
#define nltWinMain wWinMain
#else
#define nltmain main
#if defined(NL_OS_WINDOWS)
#define nltWinMain WinMain
#endif
#endif

// To define a 64bits constant; ie: UINT64_CONSTANT(0x123456781234)
#ifdef NL_COMP_VC
#	if (NL_COMP_VC_VERSION >= 100)
#		define INT64_CONSTANT(c)		(c##LL)
#		define SINT64_CONSTANT(c)	(c##LL)
#		define UINT64_CONSTANT(c)	(c##ULL)
#	elif (NL_COMP_VC_VERSION >= 80)
#		define INT64_CONSTANT(c)	(c##LL)
#		define SINT64_CONSTANT(c)	(c##LL)
#		define UINT64_CONSTANT(c)	(c##LL)
#	else
#		define INT64_CONSTANT(c)	(c)
#		define SINT64_CONSTANT(c)	(c)
#		define UINT64_CONSTANT(c)	(c)
#	endif
#else
#	define INT64_CONSTANT(c)		(c##LL)
#	define SINT64_CONSTANT(c)	(c##LL)
#	define UINT64_CONSTANT(c)	(c##ULL)
#endif

// Define a macro to write template function according to compiler weakness
#ifdef NL_COMP_NEED_PARAM_ON_METHOD
#	define NL_TMPL_PARAM_ON_METHOD_1(p1)	<p1>
#	define NL_TMPL_PARAM_ON_METHOD_2(p1, p2)	<p1, p2>
#else
#	define NL_TMPL_PARAM_ON_METHOD_1(p1)
#	define NL_TMPL_PARAM_ON_METHOD_2(p1, p2)
#endif

#if !defined(MAX_PATH) && !defined(NL_OS_WINDOWS)
#	define MAX_PATH 255
#endif

#ifdef NL_DEBUG
const std::string nlMode("NL_DEBUG");
#else
const std::string nlMode("NL_RELEASE");
#endif

// Sanity checks
#if defined (NL_DEBUG) && defined (NL_RELEASE)
#	error "NeL cannot be configured for debug and release in the same time"
#endif
#if !defined (NL_DEBUG) && !defined (NL_RELEASE)
#	error "NeL must be configured for debug or release"
#endif
#ifdef NL_RELEASE_DEBUG
#	error "NL_RELEASE_DEBUG doesn't exist anymore, please remove it"
#endif
#ifdef NL_DEBUG_FAST
#	error "NL_DEBUG_FAST doesn't exist anymore, please remove it"
#endif

#endif // NL_TYPES_H
