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

#ifndef UT_MISC_STRING_COMMON
#define UT_MISC_STRING_COMMON

#include <nel/misc/string_common.h>

struct CUTMiscStringCommon : public Test::Suite
{
	CUTMiscStringCommon()
	{
		TEST_ADD(CUTMiscStringCommon::fromStringSint8);
		TEST_ADD(CUTMiscStringCommon::fromStringUint8);
		TEST_ADD(CUTMiscStringCommon::fromStringSint16);
		TEST_ADD(CUTMiscStringCommon::fromStringUint16);
		TEST_ADD(CUTMiscStringCommon::fromStringSint32);
		TEST_ADD(CUTMiscStringCommon::fromStringUint32);
		// Add a line here when adding a new test METHOD
	}

	void fromStringSint8()
	{
		bool ret;

		// tests for sint8
		sint8 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// negative value
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(ret && val == -1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("-128", val);
		TEST_ASSERT(ret && val == -128);

		// max limit
		ret = NLMISC::fromString("127", val);
		TEST_ASSERT(ret && val == 127);

		// min limit -1
		ret = NLMISC::fromString("-129", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("128", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}

	void fromStringUint8()
	{
		bool ret;

		// tests for uint8
		uint8 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("0", val);
		TEST_ASSERT(ret && val == 0);

		// max limit
		ret = NLMISC::fromString("255", val);
		TEST_ASSERT(ret && val == 255);

		// min limit -1
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("256", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}

	void fromStringSint16()
	{
		bool ret;

		// tests for sint16
		sint16 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// negative value
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(ret && val == -1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("-32768", val);
		TEST_ASSERT(ret && val == -32768);

		// max limit
		ret = NLMISC::fromString("32767", val);
		TEST_ASSERT(ret && val == 32767);

		// min limit -1
		ret = NLMISC::fromString("-32769", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("32768", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}

	void fromStringUint16()
	{
		bool ret;

		// tests for uint16
		uint16 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("0", val);
		TEST_ASSERT(ret && val == 0);

		// max limit
		ret = NLMISC::fromString("65535", val);
		TEST_ASSERT(ret && val == 65535);

		// min limit -1
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("65536", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}

	void fromStringSint32()
	{
		bool ret;

		// tests for sint32
		sint32 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// negative value
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(ret && val == -1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("-2147483648", val);
		TEST_ASSERT(ret && val == INT_MIN);

		// max limit
		ret = NLMISC::fromString("2147483647", val);
		TEST_ASSERT(ret && val == INT_MAX);

		// min limit -1
		ret = NLMISC::fromString("-2147483649", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("2147483648", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}

	void fromStringUint32()
	{
		bool ret;

		// tests for uint32
		uint32 val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1);

		// min limit
		ret = NLMISC::fromString("0", val);
		TEST_ASSERT(ret && val == 0);

		// max limit
		ret = NLMISC::fromString("4294967295", val);
		TEST_ASSERT(ret && val == 4294967295);

		// min limit -1
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(!ret && val == 0);

		// max limit +1
		ret = NLMISC::fromString("4294967296", val);
		TEST_ASSERT(!ret && val == 0);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1);
	}
};

#endif
