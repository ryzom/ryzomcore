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

#ifndef UT_MISC_VARIABLE
#define UT_MISC_VARIABLE

#include <nel/misc/variable.h>

class CUTMiscVariable : public Test::Suite
{
public:
	CUTMiscVariable ()
	{
		TEST_ADD(CUTMiscVariable ::declareVar)
	}

	void declareVar()
	{
		{
			NLMISC::CVariable<std::string> myLocalVar("test", "myLocalVar", "no help", "");

			TEST_ASSERT(myLocalVar.get() == string(""));
			TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute("myLocalVar foo", (*NLMISC::InfoLog)));
			TEST_ASSERT(myLocalVar.get() == string("foo"));
		}

		TEST_ASSERT(!NLMISC::CCommandRegistry::getInstance().execute("myLocalVar foo", (*NLMISC::InfoLog)));
	}
};

#endif
