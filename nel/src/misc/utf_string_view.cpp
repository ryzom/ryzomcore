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

#include "stdmisc.h"

// Project includes
#include <nel/misc/utf_string_view.h>

namespace NLMISC
{

std::string CUtfStringView::toUtf8()
{
	if (m_Iterator == utf8Iterator)
		return std::string((const char *)m_Str, (const char *)((ptrdiff_t)m_Str + m_Size));
	std::string res;
	res.reserve((m_Size << 1) + 1);
	for (iterator it(begin()), end(end()); it != end; ++it)
	{
		u32char c = *it;
		res += (char)c; /* TODO: Encode UTF-8 */
	}
}

u32string CUtfStringView::toUtf32()
{
	if (m_Iterator == utf32Iterator)
		return u32string((const u32char *)m_Str, (const u32char *)((ptrdiff_t)m_Str + m_Size));
	u32string res;
	res.reserve(m_Size + 1);
	for (iterator it(begin()), end(end()); it != end; ++it)
		res += *it;
}

u32char CUtfStringView::utf8Iterator(const void **addr)
{
	/* TODO: Decode UTF-8 */
	const ucchar **pp = reinterpret_cast<const ucchar **>(addr);
	ucchar c = **pp;
	++(*pp);
	return c;
}

u32char CUtfStringView::utf16Iterator(const void **addr)
{
	/* TODO: Decode UTF-16 */
	const ucchar **pp = reinterpret_cast<const ucchar **>(addr);
	ucchar c = **pp;
	++(*pp);
	return c;
}

u32char CUtfStringView::utf32Iterator(const void **addr)
{
	const u32char **pp = reinterpret_cast<const u32char **>(addr);
	u32char c = **pp;
	++(*pp);
	return c;
}

} /* namespace NLMISC */

/* end of file */
