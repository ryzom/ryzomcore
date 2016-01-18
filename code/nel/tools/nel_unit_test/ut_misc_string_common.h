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
		TEST_ADD(CUTMiscStringCommon::fromStringSint64);
		TEST_ADD(CUTMiscStringCommon::fromStringUint64);
		TEST_ADD(CUTMiscStringCommon::fromStringFloat);
		TEST_ADD(CUTMiscStringCommon::fromStringDouble);
		TEST_ADD(CUTMiscStringCommon::fromStringBool);

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

	void fromStringSint64()
	{
		bool ret;

		// tests for sint64
		sint64 val;

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
		ret = NLMISC::fromString("-9223372036854775808", val);
		TEST_ASSERT(ret && val == LLONG_MIN);

		// max limit
		ret = NLMISC::fromString("9223372036854775807", val);
		TEST_ASSERT(ret && val == LLONG_MAX);

		// min limit -1
		ret = NLMISC::fromString("-9223372036854775809", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(LLONG_MIN == val);

		// max limit +1
		ret = NLMISC::fromString("9223372036854775808", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(LLONG_MAX == val);

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

	void fromStringUint64()
	{
		bool ret;

		// tests for uint64
		uint64 val;

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
		ret = NLMISC::fromString("18446744073709551615", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(ULLONG_MAX == val);

		// min limit -1
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(ULLONG_MAX == val);

		// max limit +1
		ret = NLMISC::fromString("18446744073709551616", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(ULLONG_MAX == val);

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

	void fromStringFloat()
	{
		bool ret;

		// tests for float
		float val;

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
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(INT_MIN == val);

		// max limit
		ret = NLMISC::fromString("2147483647", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(INT_MAX == val);

		// min limit -1
		ret = NLMISC::fromString("-2147483649", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(INT_MIN == val);

		// max limit +1
		ret = NLMISC::fromString("2147483648", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(INT_MAX == val);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(1.2f == val);

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

	void fromStringDouble()
	{
		bool ret;

		// tests for double
		double val;

		// positive value
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(ret && val == 1.0);

		// negative value
		ret = NLMISC::fromString("-1", val);
		TEST_ASSERT(ret && val == -1.0);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT(!ret && val == 0);

		// right character and bad character
		ret = NLMISC::fromString("1a", val);
		TEST_ASSERT(ret && val == 1.0);

		// min limit
		ret = NLMISC::fromString("2.2250738585072014e-308", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(DBL_MIN == val);

		// max limit
		ret = NLMISC::fromString("1.7976931348623158e+308", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(DBL_MAX == val);

		// min limit -1
		ret = NLMISC::fromString("3e-408", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(0 == val);

		// max limit +1
		ret = NLMISC::fromString("2e+308", val);
		TEST_ASSERT_MSG(ret, "should succeed");
		TEST_ASSERT(INFINITY == val);

		// with period
		ret = NLMISC::fromString("1.2", val);
		TEST_ASSERT(ret && val == 1.2);

		// with coma
		ret = NLMISC::fromString("1,2", val);
		TEST_ASSERT(ret && val == 1.0);

		// with spaces before
		ret = NLMISC::fromString("  10", val);
		TEST_ASSERT(ret && val == 10.0);

		// with spaces after
		ret = NLMISC::fromString("10  ", val);
		TEST_ASSERT(ret && val == 10.0);

		// with 0s before
		ret = NLMISC::fromString("001", val);
		TEST_ASSERT(ret && val == 1.0);

		// with + before
		ret = NLMISC::fromString("+1", val);
		TEST_ASSERT(ret && val == 1.0);
	}

	void fromStringBool()
	{
		bool ret;

		// tests for bool
		bool val;

		// true value
		val = false;
		ret = NLMISC::fromString("1", val);
		TEST_ASSERT(val);
		TEST_ASSERT_MSG(ret, "should succeed");

		val = false;
		NLMISC::fromString("t", val);
		TEST_ASSERT(val);

		val = false;
		NLMISC::fromString("y", val);
		TEST_ASSERT(val);

		val = false;
		NLMISC::fromString("T", val);
		TEST_ASSERT(val);

		val = false;
		NLMISC::fromString("Y", val);
		TEST_ASSERT(val);

		val = true;
		ret = NLMISC::fromString("0", val);
		TEST_ASSERT(!val);
		TEST_ASSERT_MSG(ret, "should succeed");

		val = true;
		NLMISC::fromString("f", val);
		TEST_ASSERT(!val);

		val = true;
		NLMISC::fromString("n", val);
		TEST_ASSERT(!val);

		val = true;
		NLMISC::fromString("F", val);
		TEST_ASSERT(!val);

		val = true;
		NLMISC::fromString("N", val);
		TEST_ASSERT(!val);

		// bad character
		ret = NLMISC::fromString("a", val);
		TEST_ASSERT_MSG(!ret, "should not succeed");

		val = true;
		NLMISC::fromString("a", val);
		TEST_ASSERT_MSG(val, "should not modify the value");

		val = false;
		NLMISC::fromString("a", val);
		TEST_ASSERT_MSG(!val, "should not modify the value");
	}
};

#endif
