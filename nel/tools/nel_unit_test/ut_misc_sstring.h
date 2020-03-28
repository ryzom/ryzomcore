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

#ifndef UT_MISC_SSTRING
#define UT_MISC_SSTRING

#include <nel/misc/sstring.h>

struct CUTMiscSString : public Test::Suite
{
	CUTMiscSString()
	{
		TEST_ADD(CUTMiscSString::testStrtok);
		// Add a line here when adding a new test METHOD
	}

	void testStrtok()
	{
		NLMISC::CSString testLine("  a=b  c   (a=e b=c) \t\t  c(a=e b=c) toto(bimbo(foo(bar(a=b))))");

		NLMISC::CSString part;
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "a=b");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "c");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "(a=e b=c)");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "c");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "(a=e b=c)");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "toto");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "(bimbo(foo(bar(a=b))))");
		part = part.stripBlockDelimiters();
		NLMISC::CSString part2 = part.strtok(" \t=", true, false);
		TEST_ASSERT(part2 == "bimbo");
		part2 = part.strtok(" \t=", true, false);
		TEST_ASSERT(part2 == "(foo(bar(a=b)))");
	}
};

#endif
