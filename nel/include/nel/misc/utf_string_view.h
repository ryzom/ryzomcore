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

#ifndef NLMISC_UTF_STRING_VIEW_H
#define NLMISC_UTF_STRING_VIEW_H

#include <nel/misc/types_nl.h>
#include <nel/misc/ucstring.h>
#include <string>

namespace NLMISC {

/// String view for UTF-8 and UTF-32 iteration as 32-bit codepoints.
/// This string view keeps the string as a reference, it does not make a copy.
/// Only use this for iterating a string's codepoints.
/// String must be NUL terminated, but its size may specify a substring.
class CUtfStringView
{
public:
	inline CUtfStringView() : m_Str(NULL), m_Size(0), m_Iterator(utf32Iterator) {}

	inline CUtfStringView(const char *utf8Str) : m_Str(utf8Str), m_Size(strlen(utf8Str)), m_Iterator(utf8Iterator) {}
	inline CUtfStringView(const char *utf8Str, size_t len): m_Str(utf8Str), m_Size(len), m_Iterator(utf8Iterator)
	{
		nlassert(len <= strlen(utf8Str));
	}
#if defined(NL_OS_WINDOWS)
	inline CUtfStringView(const wchar_t *utf16Str) : m_Str(utf16Str), m_Size(wcslen(utf16Str)), m_Iterator(utf16Iterator) {}
	inline CUtfStringView(const wchar_t *utf16Str, size_t len): m_Str(utf16Str), m_Size(len), m_Iterator(utf16Iterator)
	{
		nlassert(len <= wcslen(utf16Str));
	}
#else
	inline CUtfStringView(const wchar_t *utf32Str) : m_Str(utf32Str), m_Size(wcslen(utf32Str)), m_Iterator(utf32Iterator) {}
	inline CUtfStringView(const wchar_t *utf32Str, size_t len): m_Str(utf32Str), m_Size(len), m_Iterator(utf32Iterator)
	{
		nlassert(len <= wcslen(utf32Str));
	}
#endif

	inline CUtfStringView(const std::string &utf8Str) : m_Str(utf8Str.c_str()), m_Size(utf8Str.size()), m_Iterator(utf8Iterator) {}
	inline CUtfStringView(const ucstring &utf16Str) : m_Str(utf16Str.c_str()), m_Size(utf16Str.size() << 1), m_Iterator(utf16Iterator) {}
	inline CUtfStringView(const u32string &utf32Str) : m_Str(utf32Str.c_str()), m_Size(utf32Str.size() << 2), m_Iterator(utf32Iterator) {}

	std::string toUtf8(bool reEncode = false) const; // Makes a copy
	ucstring toUtf16(bool reEncode = false) const; // Makes a copy
	u32string toUtf32() const; // Makes a copy

	std::wstring toWide() const; // Platform dependent, UTF-16 or UTF-32. Makes a copy.
	std::string toAscii() const; // Returns only values 0-127, 7-bit ASCII. Makes a copy.

	inline bool isUtf8() const { return m_Iterator == utf8Iterator; }
	inline bool isUtf16() const { return m_Iterator == utf16Iterator; }
	inline bool isUtf32() const { return m_Iterator == utf32Iterator; }

	struct const_iterator
	{
	public:
		inline void operator++()
		{ 
			m_Char = m_View.m_Iterator(&m_Addr);
			if ((ptrdiff_t)m_Addr > ((ptrdiff_t)m_View.m_Str + m_View.m_Size))
			{
				m_Addr = 0;
				m_Char = 0;
			}
		}
		inline void operator+=(ptrdiff_t a)
		{
			while (m_Addr)
			{
				++(*this);
			}
		}
		inline bool operator!=(const const_iterator &o) const { return m_Addr != o.m_Addr; }
		inline bool operator==(const const_iterator &o) const { return m_Addr == o.m_Addr; }
		inline const u32char &operator*() const {  return m_Char; }
		const_iterator() : m_View(*(CUtfStringView *)NULL), m_Addr(NULL), m_Char(0) { }

		const_iterator &operator=(const const_iterator &other) 
		{  
			if(this == &other) return *this;
			this->~const_iterator();
			return *new(this) const_iterator(other);
		}
	private:
		friend class CUtfStringView;
		inline const_iterator(const CUtfStringView &view, const void *addr) : m_View(view), m_Addr(addr), m_Char(addr ? view.m_Iterator(&m_Addr) : 0) { }
		const CUtfStringView &m_View;
		const void *m_Addr; // Next address
		u32char m_Char;
	};

	typedef const_iterator iterator;

	iterator begin() const { return iterator(*this, m_Str); }
	inline iterator end() const { return iterator(*this, NULL); }

	/// Largest possible number of characters in this string.
	/// Number of actual characters may be less or equal.
	inline size_t largestSize() const { return m_Size; }

	inline bool empty() const { return !m_Size; }
	const void *ptr() const { return m_Str; }

	inline CUtfStringView substr(const iterator &begin, const iterator &end) const
	{
		return CUtfStringView(begin.m_Addr, (ptrdiff_t)end.m_Addr - (ptrdiff_t)begin.m_Addr, m_Iterator);
	}
	
	inline bool endsWith(char c) { nlassert(c < 0x80); return *((char *)m_Str + m_Size - 1) == c; }

	CUtfStringView &operator=(const CUtfStringView &other) 
	{  
		if(this == &other) return *this;
		this->~CUtfStringView();
		return *new(this) CUtfStringView(other);
	}

private:
	typedef u32char (*TIterator)(const void **addr);
	static u32char utf8Iterator(const void **addr);
	static u32char utf16Iterator(const void **addr);
	static u32char utf32Iterator(const void **addr);

	inline CUtfStringView(const void *str, size_t len, TIterator it) : m_Str(str), m_Size(len), m_Iterator(it) { }

	const void *const m_Str; // String
	const size_t m_Size; // Size in bytes
	const TIterator m_Iterator;
	
}; /* class CUtfStringView */

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_STREAMED_PACKAGE_PROVIDER_H */

/* end of file */
