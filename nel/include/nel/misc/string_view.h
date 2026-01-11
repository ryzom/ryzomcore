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

#ifndef NLMISC_STRING_VIEW_H
#define NLMISC_STRING_VIEW_H

#include <nel/misc/types_nl.h>
#include <string>

#ifdef NL_CPP14
using namespace std::string_literals;
#ifdef NL_CPP17
#include <string_view>
using namespace std::string_view_literals;
#endif
#endif

#ifdef NL_CPP14
/// Obtain an std::string from a string literal.
#define nlstr(strLit) (strLit##s)
#else
/// Obtain an std::string from a string literal.
#define nlstr(strLit) (std::string(strLit))
#endif

#ifdef NL_CPP17
/// Obtain a string view from a string literal.
#define nlsv(strLit) (strLit##sv)
/// Obtain an std::string from a string view.
#define nlsvs(strView) (std::string(strView))
#else
/// Obtain a string view from a string literal.
#define nlsv(strLit) (CStringView(strLit, ::strlen(strLit)))
/// Obtain an std::string from a string view.
#define nlsvs(strView) (std::string(strView.data(), strView.size()))
#endif

/// Obtain a temporary C-string from a string view. Use directly in argument, do not store.
#define nlsvc(strView) ((strView.data()[strView.size()]) ? nlsvs(strView).c_str() : strView.data())

namespace NLMISC {

/// String view literals allow bypassing allocation and strlen calls.
/// CStringView is a 100% drop-in replacement for (const char *str, size_t len) tuples. It's a non-owning reference.
/// Always use `CStringView` where previously `const std::string &` would have been used. It avoids accidental copy.
/// Gotcha: CStringView doesn't need to end with \0, so there's no guarantee with functions that expect \0 terminated strings, 
/// use the `nlsvc` macro to get a temporary C-string from a CStringView.
/// Use the `nlsv` macro to get a CStringView from a string literal.
/// Use the `nlstr` macro to get an std::string from a string literal.
/// Use the `nlsvs` macro to get an std::string from a CStringView.
#ifdef NL_CPP17
typedef std::string_view CStringView;
#else
class CStringView
{
public:
	CStringView(const std::string &str) : m_Str(&str[0]), m_Len(str.size()) {}
	CStringView(const char *const str, const size_t len) : m_Str(str), m_Len(len) {}
	CStringView(const char *const str) : m_Str(str), m_Len(sizeof(str)) {}

	inline const char *data() const { return m_Str; }
	inline size_t length() const { return m_Len; }
	inline size_t size() const { return m_Len; }

	inline CStringView substr(const size_t offset, const size_t count = -1) { return CStringView(m_Str + offset, std::min(m_Len - offset, count)); }

	inline bool operator==(const CStringView o) { if (m_Len != o.m_Len) return false; return memcmp(m_Str, o.m_Str, m_Len) == 0; }
	inline bool operator!=(const CStringView o) { if (m_Len != o.m_Len) return true; return memcmp(m_Str, o.m_Str, m_Len) != 0; }

	struct const_iterator
	{
	public:
		const_iterator() : m_Addr(NULL) { }

		inline void operator++()
		{
			++m_Addr;
		}
		inline void operator+=(ptrdiff_t v)
		{
			m_Addr += v;
		}
		inline void operator--()
		{
			--m_Addr;
		}
		inline void operator-=(ptrdiff_t v)
		{
			m_Addr -= v;
		}
		inline bool operator!=(const const_iterator &o) const { return m_Addr != o.m_Addr; }
		inline bool operator==(const const_iterator &o) const { return m_Addr == o.m_Addr; }
		inline const char &operator*() const { return *m_Addr; }

	private:
		friend class CStringView;
		inline const_iterator(const char *addr) : m_Addr(addr) {}
		const char *m_Addr;

	};

	typedef const_iterator iterator;

	iterator begin() const { return iterator(m_Str); }
	inline iterator end() const { return iterator(m_Str + m_Len); }

private:
	const char *m_Str;
	size_t m_Len;

};
#endif

}

#endif /* #ifndef NLMISC_STRING_VIEW_H */

/* end of file */
