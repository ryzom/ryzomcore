// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2016-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef	NL_STRING_COMMON_H
#define	NL_STRING_COMMON_H

#include "types_nl.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <errno.h>

namespace NLMISC
{

// get a string and add \r before \n if necessary
std::string addSlashR (const std::string &str);
std::string removeSlashR (const std::string &str);

/**
 * \def MaxCStringSize
 *
 * The maximum size allowed for C string (zero terminated string) buffer.
 * This value is used when we have to create a standard C string buffer and we don't know exactly the final size of the string.
 */
const int MaxCStringSize = 2048;


/**
 * \def NLMISC_CONVERT_VARGS(dest,format)
 *
 * This macro converts variable arguments into C string (zero terminated string).
 * This function takes care to avoid buffer overflow.
 *
 * Example:
 *\code
	void MyFunction(const char *format, ...)
	{
		string str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);
		// str contains the result of the conversion
	}
 *\endcode
 *
 * \param _dest \c string or \c char* that contains the result of the convertion
 * \param _format format of the string, it must be the last argument before the \c '...'
 * \param _size size of the buffer that will contain the C string
 */
#define NLMISC_CONVERT_VARGS(_dest,_format,_size) \
char _cstring[_size]; \
va_list _args; \
va_start (_args, _format); \
int _res = vsnprintf (_cstring, _size-1, _format, _args); \
if (_res == -1 || _res == _size-1) \
{ \
	_cstring[_size-1] = '\0'; \
} \
va_end (_args); \
_dest = _cstring



/** sMart sprintf function. This function do a sprintf and add a zero at the end of the buffer
 * if there no enough room in the buffer.
 *
 * \param buffer a C string
 * \param count Size of the buffer
 * \param format of the string, it must be the last argument before the \c '...'
 */
sint smprintf( char *buffer, size_t count, const char *format, ... );


#ifdef NL_OS_WINDOWS

//
// These functions are needed by template system to failed the compilation if you pass bad type on nlinfo...
//

inline void _check(int /* a */) { }
inline void _check(unsigned int /* a */) { }
inline void _check(char /* a */) { }
inline void _check(unsigned char /* a */) { }
inline void _check(long /* a */) { }
inline void _check(unsigned long /* a */) { }

#ifdef NL_COMP_VC6
	inline void _check(uint8 /* a */) { }
#endif // NL_COMP_VC6

inline void _check(sint8 /* a */) { }
inline void _check(uint16 /* a */) { }
inline void _check(sint16 /* a */) { }

#ifdef NL_COMP_VC6
	inline void _check(uint32 /* a */) { }
	inline void _check(sint32 /* a */) { }
#endif // NL_COMP_VC6

inline void _check(uint64 /* a */) { }
inline void _check(sint64 /* a */) { }
inline void _check(float /* a */) { }
inline void _check(double /* a */) { }
inline void _check(const char * /* a */) { }
inline void _check(const void * /* a */) { }

#define CHECK_TYPES(__a,__b) \
inline __a(const char *fmt) { __b(fmt); } \
template<class A> __a(const char *fmt, A a) { _check(a); __b(fmt, a); } \
template<class A, class B> __a(const char *fmt, A a, B b) { _check(a); _check(b); __b(fmt, a, b); } \
template<class A, class B, class C> __a(const char *fmt, A a, B b, C c) { _check(a); _check(b); _check(c); __b(fmt, a, b, c); } \
template<class A, class B, class C, class D> __a(const char *fmt, A a, B b, C c, D d) { _check(a); _check(b); _check(c); _check(d); __b(fmt, a, b, c, d); } \
template<class A, class B, class C, class D, class E> __a(const char *fmt, A a, B b, C c, D d, E e) { _check(a); _check(b); _check(c); _check(d); _check(e); __b(fmt, a, b, c, d, e); } \
template<class A, class B, class C, class D, class E, class F> __a(const char *fmt, A a, B b, C c, D d, E e, F f) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); __b(fmt, a, b, c, d, e, f); } \
template<class A, class B, class C, class D, class E, class F, class G> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); __b(fmt, a, b, c, d, e, f, g); } \
template<class A, class B, class C, class D, class E, class F, class G, class H> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); __b(fmt, a, b, c, d, e, f, g, h); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); __b(fmt, a, b, c, d, e, f, g, h, i); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); __b(fmt, a, b, c, d, e, f, g, h, i, j); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); __b(fmt, a, b, c, d, e, f, g, h, i, j, k); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y); } \
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y, class Z> __a(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y, Z z) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); _check(z); __b(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z); }

#endif

#ifdef NL_OS_WINDOWS
	inline std::string _toString(const char *format, ...)
#else
	inline std::string toString(const char *format, ...)
#endif
{
	std::string Result;
	NLMISC_CONVERT_VARGS(Result, format, NLMISC::MaxCStringSize);
	return Result;
}

#ifdef NL_OS_WINDOWS
	CHECK_TYPES(std::string toString, return _toString)
#endif // NL_OS_WINDOWS

template<class T> std::string toStringPtr(const T *val) { return toString("%p", val); }

template<class T> std::string toStringEnum(const T &val) { return toString("%u", (uint32)val); }

/**
 * Template Object toString.
 * \param obj any object providing a "std::string toString()" method. The object doesn't have to derive from anything.
 *
 * the VC++ error "error C2228: left of '.toString' must have class/struct/union type" means you don't provide
 * a toString() method to your object.
 */
template<class T> std::string toString(const T &obj)
{
	return obj.toString();
}

//inline std::string toString(const char *val) { return toString("%s", val); }
inline std::string toString(const uint8 &val) { return toString("%hu", (uint16)val); }
inline std::string toString(const sint8 &val) { return toString("%hd", (sint16)val); }
inline std::string toString(const uint16 &val) { return toString("%hu", val); }
inline std::string toString(const sint16 &val) { return toString("%hd", val); }
inline std::string toString(const uint32 &val) { return toString("%u", val); }
inline std::string toString(const sint32 &val) { return toString("%d", val); }
inline std::string toString(const uint64 &val) { return toString("%" NL_I64 "u", val); }
inline std::string toString(const sint64 &val) { return toString("%" NL_I64 "d", val); }

#ifdef NL_OS_WINDOWS
inline std::string toString(const wchar_t &val) { return toString(reinterpret_cast<const uint16 &>(val)); }
#endif

#ifdef NL_COMP_GCC
#	if GCC_VERSION == 40102

// error fix for size_t? gcc 4.1.2 requested this type instead of size_t ...
inline std::string toString(const long unsigned int &val)
{
#ifdef _LP64
	return toString((uint64)val);
#else
	return toString((uint32)val);
#endif
}

#	endif
#endif

#if (SIZEOF_SIZE_T) == 8
inline std::string toString(const size_t &val) { return toString("%" NL_I64 "u", val); }
#else
#ifdef NL_OS_MAC
inline std::string toString(const size_t &val) { return toString("%u", val); }
#endif
#endif

inline std::string toString(const float &val) { return toString("%f", val); }
inline std::string toString(const double &val) { return toString("%lf", val); }
inline std::string toString(const bool &val) { return val ? "1":"0"; }
inline std::string toString(const std::string &val) { return val; }

// stl vectors of bool use bit reference and not real bools, so define the operator for bit reference

#ifdef NL_COMP_VC6
inline std::string toString(const uint &val) { return toString("%u", val); }
inline std::string toString(const sint &val) { return toString("%d", val); }
#endif // NL_COMP_VC6

template<class T>
bool fromString(const std::string &str, T &obj)
{
	return obj.fromString(str);
}

inline bool fromString(const std::string &str, uint32 &val) { if (str.find('-') != std::string::npos) { val = 0; return false; } char *end; unsigned long v; errno = 0; v = strtoul(str.c_str(), &end, 10); if (errno || v > UINT_MAX || end == str.c_str()) { val = 0; return false; } else { val = (uint32)v; return true; } }
inline bool fromString(const std::string &str, sint32 &val) { char *end; long v; errno = 0; v = strtol(str.c_str(), &end, 10); if (errno || v > INT_MAX || v < INT_MIN || end == str.c_str()) { val = 0; return false; } else { val = (sint32)v; return true; } }
inline bool fromString(const std::string &str, uint8 &val) { char *end; long v; errno = 0; v = strtol(str.c_str(), &end, 10); if (errno || v > UCHAR_MAX || v < 0 || end == str.c_str()) { val = 0; return false; } else { val = (uint8)v; return true; } }
inline bool fromString(const std::string &str, sint8 &val) { char *end; long v; errno = 0; v = strtol(str.c_str(), &end, 10); if (errno || v > SCHAR_MAX || v < SCHAR_MIN || end == str.c_str()) { val = 0; return false; } else { val = (sint8)v; return true; } }
inline bool fromString(const std::string &str, uint16 &val) { char *end; long v; errno = 0; v = strtol(str.c_str(), &end, 10); if (errno || v > USHRT_MAX || v < 0 || end == str.c_str()) { val = 0; return false; } else { val = (uint16)v; return true; } }
inline bool fromString(const std::string &str, sint16 &val) { char *end; long v; errno = 0; v = strtol(str.c_str(), &end, 10); if (errno || v > SHRT_MAX || v < SHRT_MIN || end == str.c_str()) { val = 0; return false; } else { val = (sint16)v; return true; } }
inline bool fromString(const std::string &str, uint64 &val) { bool ret = sscanf(str.c_str(), "%" NL_I64 "u", &val) == 1; if (!ret) val = 0; return ret; }
inline bool fromString(const std::string &str, sint64 &val) { bool ret = sscanf(str.c_str(), "%" NL_I64 "d", &val) == 1; if (!ret) val = 0; return ret; }
inline bool fromString(const std::string &str, float &val) { bool ret = sscanf(str.c_str(), "%f", &val) == 1; if (!ret) val = 0.0f; return ret; }
inline bool fromString(const std::string &str, double &val) { bool ret = sscanf(str.c_str(), "%lf", &val) == 1; if (!ret) val = 0.0; return ret; }

#ifdef NL_OS_WINDOWS
inline bool fromString(const std::string &str, wchar_t &val) { return fromString(str, reinterpret_cast<uint16 &>(val)); }
#endif

/// Fast string to bool, reliably defined for strings starting with 0, 1, t, T, f, F, y, Y, n, N, and empty strings, anything else is undefined.
///  - Kaetemi
inline bool toBool(const char *str) { return str[0] == '1' || (str[0] & 0xD2) == 0x50; }
inline bool toBool(const std::string &str) { return toBool(str.c_str()); } // Safe because first byte may be null

bool fromString(const std::string &str, bool &val);

inline bool fromString(const std::string &str, std::string &val) { val = str; return true; }

// stl vectors of bool use bit reference and not real bools, so define the operator for bit reference

#ifdef NL_COMP_VC6
inline bool fromString(const std::string &str, uint &val) { return sscanf(str.c_str(), "%u", &val) == 1; }
inline bool fromString(const std::string &str, sint &val) { return sscanf(str.c_str(), "%d", &val) == 1; }
#endif // NL_COMP_VC6

inline bool startsWith(const char *str, const char *prefix)
{
	for (int i = 0;; ++i)
	{
		if (str[i] != prefix[i] || str[i] == '\0')
		{
			return prefix[i] == '\0';
		}
	}
}

inline bool startsWith(const std::string &str, const char *prefix) { return startsWith(str.c_str(), prefix); }
inline bool startsWith(const std::string &str, const std::string &prefix) { return startsWith(str.c_str(), prefix.c_str()); }

inline bool endsWith(const char *str, size_t strLen, const char *suffix, size_t suffixLen)
{
	if (strLen < suffixLen)
		return false;
	int minLen = strLen < suffixLen ? strLen : suffixLen;
	for (int i = 1; i <= minLen; ++i)
		if (str[strLen - i] != suffix[suffixLen - i])
			return false;
	return true;
}

inline bool endsWith(const char *str, const char *suffix) { return endsWith(str, strlen(str), suffix, strlen(suffix)); }
inline bool endsWith(const std::string &str, const char *suffix) { return endsWith(str.c_str(), str.size(), suffix, strlen(suffix)); }
inline bool endsWith(const std::string &str, const std::string &suffix) { return endsWith(str.c_str(), str.size(), suffix.c_str(), suffix.size()); }

// ****************************************************************************
// Conversion of strings between
// - UTF-8
// - Local codepage (UTF-8 on Linux)
// - TString (Local codepage or wide on Windows)
// - Wide (wchar_t *, UTF-16 on Windows, UTF-32 on Linux)
// ****************************************************************************

// Convert local codepage to UTF-8
// On Windows, the local codepage is undetermined
// On Linux, the local codepage is always UTF-8 (no-op)
std::string mbcsToUtf8(const char *str, size_t len = 0);
std::string mbcsToUtf8(const std::string &str);

// Convert wide codepage to UTF-8
// On Windows, the wide codepage is UTF-16
// On Linux, the wide codepage is UTF-32
std::string wideToUtf8(const wchar_t *str, size_t len = 0);
std::string wideToUtf8(const std::wstring &str);

// Convert UTF-8 to wide character set
std::wstring utf8ToWide(const char *str, size_t len = 0);
std::wstring utf8ToWide(const std::string &str);

// Convert UTF-8 to local multibyte character set
std::string utf8ToMbcs(const char *str, size_t len = 0);
std::string utf8ToMbcs(const std::string &str);

// Convert wide to local multibyte character set
std::string wideToMbcs(const wchar_t *str, size_t len = 0);
std::string wideToMbcs(const std::wstring &str);

// Convert local multibyte to wide character set
std::wstring mbcsToWide(const char *str, size_t len = 0);
std::wstring mbcsToWide(const std::string &str);

inline const char *asCStr(const char *str) { return str; }
inline const char *asCStr(const std::string &str) { return str.c_str(); }
inline const wchar_t *asCStr(const wchar_t *str) { return str; }
inline const wchar_t *asCStr(const std::wstring &str) { return str.c_str(); }

#if defined(NL_OS_WINDOWS)
#define nlUtf8ToMbcs(str) (NLMISC::utf8ToMbcs(str).c_str())
#define nlMbcsToUtf8(str) (NLMISC::mbcsToUtf8(str).c_str())
#else
#define nlUtf8ToMbcs(str) (NLMISC::asCStr(str))
#define nlMbcsToUtf8(str) (NLMISC::asCStr(str))
#endif
#define nlWideToUtf8(str) (NLMISC::wideToUtf8(str).c_str())
#define nlUtf8ToWide(str) (NLMISC::utf8ToWide(str).c_str())
#define nlWideToMbcs(str) (NLMISC::wideToMbcs(str).c_str())
#define nlMbcsToWide(str) (NLMISC::mbcsToWide(str).c_str())

// On Windows, tstring is either local multibyte or utf-16 wide
// On Linux, tstring is always utf-8

#if defined(NL_OS_WINDOWS) && (defined(UNICODE) || defined(_UNICODE))
typedef std::wstring tstring;
typedef wchar_t tchar;
inline std::string tStrToUtf8(const tchar *str) { return wideToUtf8((const wchar_t *)str); }
inline std::string tStrToUtf8(const tstring &str) { return wideToUtf8((const std::wstring &)str); }
inline std::wstring tStrToWide(const tchar *str) { return (const wchar_t *)str; }
inline std::wstring tStrToWide(const tstring &str) { return (const std::wstring &)str; }
inline std::string tStrToMbcs(const tchar *str) { return wideToMbcs((const wchar_t *)str); }
inline std::string tStrToMbcs(const tstring &str) { return wideToMbcs((const std::wstring &)str); }
#define nlTStrToUtf8(str) (NLMISC::tStrToUtf8(str).c_str())
#define nlTStrToWide(str) ((const wchar_t *)NLMISC::asCStr(str))
#define nlTStrToMbcs(str) (NLMISC::tStrToMbcs(str).c_str())
inline tstring utf8ToTStr(const char *str) {return (const tstring &)utf8ToWide(str); }
inline tstring utf8ToTStr(const std::string &str) { return (const tstring &)utf8ToWide(str); }
inline tstring wideToTStr(const wchar_t *str) { return (const tchar *)str; }
inline tstring wideToTStr(const std::wstring &str) { return (const tstring &)str; }
inline tstring mbcsToTStr(const char *str) { return (const tstring &)mbcsToWide(str); }
inline tstring mbcsToTStr(const std::string &str) { return (const tstring &)mbcsToWide(str); }
#define nlUtf8ToTStr(str) (NLMISC::utf8ToTStr(str).c_str())
#define nlWideToTStr(str) ((const tchar *)NLMISC::asCStr(str))
#define nlMbcsToTStr(str) (NLMISC::mbcsToTStr(str).c_str())
#else
typedef std::string tstring;
typedef char tchar;
inline std::string tStrToUtf8(const tchar *str) { return mbcsToUtf8((const char *)str); }
inline std::string tStrToUtf8(const tstring &str) { return mbcsToUtf8((const std::string &)str); }
inline std::wstring tStrToWide(const tchar *str) { return mbcsToWide((const char *)str); }
inline std::wstring tStrToWide(const tstring &str) { return mbcsToWide((const std::string &)str); }
inline std::string tStrToMbcs(const tchar *str) { return (const char *)str; }
inline std::string tStrToMbcs(const tstring &str) { return (const std::string &)str; }
#if defined(NL_OS_WINDOWS)
#define nlTStrToUtf8(str) (NLMISC::tStrToUtf8(str).c_str())
#else
#define nlTStrToUtf8(str) ((const char *)NLMISC::asCStr(str))
#endif
#define nlTStrToWide(str) (NLMISC::tStrToWide(str).c_str())
#define nlTStrToMbcs(str) ((const char *)NLMISC::asCStr(str))
inline tstring utf8ToTStr(const char *str) { return (const tstring &)utf8ToMbcs(str); }
inline tstring utf8ToTStr(const std::string &str) { return (const tstring &)utf8ToMbcs(str); }
inline tstring wideToTStr(const wchar_t *str) { return (const tstring &)wideToMbcs(str); }
inline tstring wideToTStr(const std::wstring &str) { return (const tstring &)wideToMbcs(str); }
inline tstring mbcsToTStr(const char *str) { return (const tchar *)str; }
inline tstring mbcsToTStr(const std::string &str) { return (const tstring &)str; }
#if defined(NL_OS_WINDOWS)
#define nlUtf8ToTStr(str) (NLMISC::utf8ToTStr(str).c_str())
#else
#define nlUtf8ToTStr(str) ((const tchar *)NLMISC::asCStr(str))
#endif
#define nlWideToTStr(str) (NLMISC::wideToTStr(str).c_str())
#define nlMbcsToTStr(str) ((const tchar *)NLMISC::asCStr(str))
#endif

} // NLMISC

#endif	// NL_STRING_COMMON_H
