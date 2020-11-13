// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLMISC_DEEP_PTR_H
#define NLMISC_DEEP_PTR_H

#include <nel/misc/types_nl.h>

namespace NLMISC {

/// Pointer template with deep copy and move semantics
template<class T>
class CDeepPtr
{
public:
	NL_FORCE_INLINE CDeepPtr() : m(NULL) { } //< Null
	NL_FORCE_INLINE ~CDeepPtr() { delete m; }

	NL_FORCE_INLINE CDeepPtr(const CDeepPtr &p) : m(p.m ? new T(*p) : NULL) { } //< Copy operator
	NL_FORCE_INLINE CDeepPtr &operator=(const CDeepPtr &p) { if (p.m) { if (!m) m = new T(*p); else *m = *p; } else { delete m; m = NULL; } return *this; } //< Copy operator

#ifdef NL_CPP14
	NL_FORCE_INLINE CDeepPtr(CDeepPtr &&p) noexcept : m(p.m) { p.m = NULL; } //< Move operator
	NL_FORCE_INLINE CDeepPtr &operator=(CDeepPtr &&p) noexcept { delete m; m = p.m; p.m = NULL; return *this; } //< Move operator
#endif

	NL_FORCE_INLINE CDeepPtr(T *p) : m(p) { } //< Initializer
	NL_FORCE_INLINE CDeepPtr &operator=(T *p) { delete m; m = p; return *this; } //< Initializer

	NL_FORCE_INLINE bool operator==(const CDeepPtr &p) const { return /* (m == p.m) || */ (m && p.m && *m == *p); }
	NL_FORCE_INLINE bool operator!=(const CDeepPtr &p) const { return !(*this == p); }

	NL_FORCE_INLINE bool operator==(const T *p) const { return (m == p) || (m && p && *m == *p); }
	NL_FORCE_INLINE bool operator!=(const T *p) const { return !(*this == p); }

	NL_FORCE_INLINE bool operator==(const T &p) const { return (m == &p) || (m && *m == p); }
	NL_FORCE_INLINE bool operator!=(const T &p) const { return !(*this == p); }

	NL_FORCE_INLINE bool operator==(long int p) const { return (*this == (const T *)(ptrdiff_t)p); } //< == NULL
	NL_FORCE_INLINE bool operator!=(long int p) const { return (*this != (const T *)(ptrdiff_t)p); } //< != NULL

	NL_FORCE_INLINE bool operator==(int p) const { return (*this == (const T *)(ptrdiff_t)p); } //< == 0
	NL_FORCE_INLINE bool operator!=(int p) const { return (*this != (const T *)(ptrdiff_t)p); } //< != 0

#ifdef NL_CPP14
	NL_FORCE_INLINE bool operator==(nullptr_t p) const { return (*this == (const T *)(ptrdiff_t)p); } //< == nullptr
	NL_FORCE_INLINE bool operator!=(nullptr_t p) const { return (*this != (const T *)(ptrdiff_t)p); } //< != nullptr
#endif

	NL_FORCE_INLINE T &operator*() { return *m; }
	NL_FORCE_INLINE const T &operator*() const { return *m; }
	NL_FORCE_INLINE T *operator->() { return m; }
	NL_FORCE_INLINE const T *operator->() const { return m; }

	NL_FORCE_INLINE operator bool() const { return m; }
	NL_FORCE_INLINE bool operator!() const { return !m; }

	NL_FORCE_INLINE T *ptr() { return m; }
	NL_FORCE_INLINE const T *ptr() const { return m; }

private:
	T *m;

};

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_DEEP_PTR_H */

/* end of file */
