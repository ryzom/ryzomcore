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

void ucstring::toString(std::string &str) const
{
	str = nlmove(NLMISC::CUtfStringView(*this).toUtf8());
}

std::string ucstring::toUtf8() const
{
	return NLMISC::CUtfStringView(*this).toUtf8();
}

void ucstring::fromUtf8(const std::string &stringUtf8)
{
	*this = NLMISC::CUtfStringView(stringUtf8).toUtf16();
}

/* end of file */
