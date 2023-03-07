// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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
#include "nel/misc/ucstring.h"
#include "nel/misc/utf_string_view.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{

ucstring toLower (const ucstring &str)
{
	return ucstring::makeFromUtf8(toLower(str.toUtf8()));
}

// ***************************************************************************

void toLower (ucchar *str)
{
	while (*str)
	{
		*str = toLower(*str);
		++str;
	}
}

// ***************************************************************************

ucchar toLower (ucchar c)
{
	if ((c & 0xF800) == 0xD800)
		return c;
	std::string tmpc, tmpr;
	CUtfStringView::append(tmpc, c);
	ptrdiff_t i = 0;
	appendToLower(tmpr, tmpc, i);
	ucstring res = CUtfStringView(tmpr).toUtf16();
	nlassert(res.size() == 1);
	return res[0];
}

// ***************************************************************************

ucstring toUpper (const ucstring &str)
{
	return ucstring::makeFromUtf8(toUpper(str.toUtf8()));
}

// ***************************************************************************

void toUpper (ucchar *str)
{
	while (*str)
	{
		*str = toUpper(*str);
		++str;
	}
}

// ***************************************************************************

ucchar toUpper (ucchar c)
{
	if ((c & 0xF800) == 0xD800)
		return c;
	std::string tmpc, tmpr;
	CUtfStringView::append(tmpc, c);
	ptrdiff_t i = 0;
	appendToUpper(tmpr, tmpc, i);
	ucstring res = CUtfStringView(tmpr).toUtf16();
	nlassert(res.size() == 1);
	return res[0];
}

} // NLMISC
