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

#ifndef UT_MISC_TYPES
#define UT_MISC_TYPES

#include <nel/misc/variable.h>

class CUTMiscTypes: public Test::Suite
{
public:
	CUTMiscTypes ()
	{
		TEST_ADD(CUTMiscTypes::basicTypes)
	}

	void basicTypes()
	{
		// this doesn't work on 64bit architectures
		//Test_ASSERT(sizeof(uint) == sizeof(void*));

		TEST_ASSERT(sizeof(sint8) == 1);
		TEST_ASSERT(sizeof(uint8) == 1);
		TEST_ASSERT(sizeof(sint16) == 2);
		TEST_ASSERT(sizeof(uint16) == 2);
		TEST_ASSERT(sizeof(sint32) == 4);
		TEST_ASSERT(sizeof(uint32) == 4);
		TEST_ASSERT(sizeof(sint64) == 8);
		TEST_ASSERT(sizeof(uint64) == 8);

		TEST_ASSERT(sizeof(sint) >= 4);
		TEST_ASSERT(sizeof(uint) >= 4);
	}
};

#endif
