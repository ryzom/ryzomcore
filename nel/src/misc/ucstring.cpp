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

#include "stdmisc.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/utf_string_view.h"

void ucstring::toString(std::string &str) const
{
	str.resize(size());
	for (uint i = 0; i < str.size(); i++)
	{
		if (operator[](i) > 255)
			str[i] = '?';
		else
			str[i] = (char)operator[](i);
	}
}

std::string ucstring::toUtf8() const
{
	return NLMISC::CUtfStringView(*this).toUtf8();
}

void ucstring::fromUtf8(const std::string &stringUtf8)
{
	*this = NLMISC::CUtfStringView(stringUtf8).toUtf16();
}

void ucstring::rawCopy(const std::string &str)
{
	// We need to convert the char into 8bits unsigned int before promotion to 16 bits
	// otherwise, as char are signed on some compiler (MSCV for ex), the sign bit is extended to 16 bits.
	resize(str.size());
	std::string::const_iterator first(str.begin()), last(str.end());
	iterator dest(begin());
	for (; first != last; ++first, ++dest)
	{
		*dest = uint8(*first);
	}
}

/* end of file */
