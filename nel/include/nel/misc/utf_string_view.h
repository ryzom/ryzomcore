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

#include <string>

#include <nel/misc/string_view.h>
#include <nel/misc/ucstring.h>

namespace NLMISC {

class IStream;

/// String view for UTF-8 and UTF-32 iteration as 32-bit codepoints.
/// This string view keeps the string as a reference, it does not make a copy.
/// Only use this for iterating a string's codepoints.
/// String must be NUL terminated, but its size may specify a substring.
class CUtfStringView
{
public:
	inline CUtfStringView() : m_Str(NULL), m_Size(0), m_Iterator(utf32Iterator) {}

	inline CUtfStringView(const std::string &utf8Str) : m_Str(utf8Str.c_str()), m_Size(utf8Str.size()), m_Iterator(utf8Iterator) {}
	inline CUtfStringView(const char *utf8Str) : m_Str(utf8Str), m_Size(strlen(utf8Str)), m_Iterator(utf8Iterator) {}
	inline CUtfStringView(const char *utf8Str, size_t len): m_Str(utf8Str), m_Size(len), m_Iterator(utf8Iterator)
	{
		nlassert(len <= strlen(utf8Str));
	}

	inline CUtfStringView(CStringView utf8Str) : m_Str(utf8Str.data()), m_Size(utf8Str.size()), m_Iterator(utf8Iterator) {}

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

	inline CUtfStringView(const ucstring &utf16Str) : m_Str(utf16Str.c_str()), m_Size(utf16Str.size() << 1), m_Iterator(utf16Iterator) {}
	inline CUtfStringView(const ucchar *utf16Str) : m_Str(utf16Str), m_Size(strlen((const char *)utf16Str) & (ptrdiff_t)(-2)), m_Iterator(utf16Iterator) {}
	inline CUtfStringView(const ::u32string &utf32Str) : m_Str(utf32Str.c_str()), m_Size(utf32Str.size() << 2), m_Iterator(utf32Iterator) {}

	std::string toUtf8(bool reEncode = false) const; // Makes a copy
	ucstring toUtf16(bool reEncode = false) const; // Makes a copy
	::u32string toUtf32() const; // Makes a copy

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
			if ((ptrdiff_t)m_Next > ((ptrdiff_t)m_View.m_Str + m_View.m_Size))
				m_Next = (void *)((ptrdiff_t)m_View.m_Str + m_View.m_Size);
			m_Addr = m_Next;
			m_Char = m_View.m_Iterator(&m_Next);
		}
		inline void operator+=(ptrdiff_t a)
		{
			while ((ptrdiff_t)m_Addr < ((ptrdiff_t)m_View.m_Str + m_View.m_Size))
			{
				++(*this);
			}
		}
		inline bool operator!=(const const_iterator &o) const { return m_Addr != o.m_Addr; }
		inline bool operator==(const const_iterator &o) const { return m_Addr == o.m_Addr; }
		inline const u32char &operator*() const { return m_Char; }
		const_iterator() : m_View(*(CUtfStringView *)NULL), m_Addr(NULL), m_Char(0) { }

		const_iterator &operator=(const const_iterator &other)
		{
			if (this == &other) return *this;
			this->~const_iterator();
			return *new(this) const_iterator(other);
		}

		const void *ptr() const { return m_Addr; }
	private:
		friend class CUtfStringView;
		inline const_iterator(const CUtfStringView &view, const void *addr) : m_View(view), m_Addr(addr), m_Next(addr), m_Char(view.m_Iterator(&m_Next))
		{
			if ((ptrdiff_t)m_Next > ((ptrdiff_t)m_View.m_Str + m_View.m_Size))
				m_Next = (void *)((ptrdiff_t)m_View.m_Str + m_View.m_Size);
		}
		inline const_iterator(const CUtfStringView &view) : m_View(view), m_Addr((char *)view.m_Str + view.m_Size), m_Next((char *)view.m_Str + view.m_Size + 1), m_Char(0) { }
		const CUtfStringView &m_View;
		const void *m_Addr; // Current address
		const void *m_Next; // Next address
		u32char m_Char;
	};

	typedef const_iterator iterator;

	iterator begin() const { return iterator(*this, m_Str); }
	inline iterator end() const { return iterator(*this); }

	/// Largest possible number of characters in this string.
	/// Number of actual characters may be less or equal.
	inline size_t largestSize() const { return m_Size; }

	inline bool empty() const { return !m_Size; }
	const void *ptr() const { return m_Str; }

	size_t count() const; //< Slow count of UTF-32 codepoints
	ptrdiff_t offset(ptrdiff_t i); const //< Get byte offset by utf-32 codepoint index

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

	static void append(std::string &str, u32char c);

	/// Encode or decode a single UTF-8 character in a stream (also useful for packing unsigned 20-bit integers)
	static void append(IStream &s, u32char c);
	static u32char get(IStream &s);

	/// Get an UTF-8 string from an undefined ASCII-based codepage, without attempting to convert non-7-bit characters
	static std::string fromAscii(const std::string &str);

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

#endif /* #ifndef NLMISC_UTF_STRING_VIEW_H */

/* end of file */
