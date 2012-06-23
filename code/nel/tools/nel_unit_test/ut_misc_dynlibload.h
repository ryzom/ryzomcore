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

#ifndef UT_MISC_DYNLIBLOAD
#define UT_MISC_DYNLIBLOAD

#include <nel/misc/dynloadlib.h>

class CUTMiscDynLibLoad : public Test::Suite
{
public:
	CUTMiscDynLibLoad ()
	{
		TEST_ADD(CUTMiscDynLibLoad ::libraryNameDecoration)
	}

	void libraryNameDecoration()
	{
		string libName = "libmylib_with_dll_so_some_very_bad_rd_df_tag_inside_df";
		string fileName = "some/path/to/add/difficulties/"+NLMISC::CLibrary::makeLibName(libName);
		string cleanedName = NLMISC::CLibrary::cleanLibName(fileName);

		TEST_ASSERT(cleanedName == libName);
	}
};

#endif
