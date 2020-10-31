// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLMAX_STRING_COMMON_H
#define NLMAX_STRING_COMMON_H

#include <nel/misc/ucstring.h>
#include <nel/misc/string_common.h>

#if (MAX_VERSION_MAJOR < 15)
#define GET_OBJECT_NAME_CONST
#define NOTIFY_REF_PARAMS Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message
#define NOTIFY_REF_PROPAGATE , BOOL propagate
#define nl_p_end end
#else
#define GET_OBJECT_NAME_CONST const
#define NOTIFY_REF_PARAMS const Interval &changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message, BOOL propagate
#define nl_p_end p_end
#endif

static TSTR MaxTStrFromUtf8(const std::string &src)
{
	TSTR dst;
#if (MAX_VERSION_MAJOR < 15)
	dst = nlUtf8ToTStr(src);
#else
	dst.FromUTF8(src.c_str());
#endif
	return dst;
}

static std::string MaxTStrToUtf8(const TSTR& src)
{
#if (MAX_VERSION_MAJOR < 15)
	return NLMISC::tStrToUtf8(src.data());
#else
	return src.ToUTF8().data();
#endif
}

static std::string MCharStrToUtf8(const MCHAR *src)
{
	return NLMISC::tStrToUtf8(src);
}

#endif /* #ifndef NLMAX_STRING_COMMON_H */

/* end of file */
