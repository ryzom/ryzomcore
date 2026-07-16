// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <gtest/gtest.h>

#include <limits>
#include <nel/misc/string_common.h>

class CUTMiscStringCommonTest : public testing::Test
{
};

TEST_F(CUTMiscStringCommonTest, fromStringSint8)
{
	bool ret;

	// tests for sint8
	sint8 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("-128", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint8>::min());

	// max limit
	ret = NLMISC::fromString("127", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint8>::max());

	// min limit -1
	ret = NLMISC::fromString("-129", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("128", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringUint8)
{
	bool ret;

	// tests for uint8
	uint8 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("0", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint8>::min());

	// max limit
	ret = NLMISC::fromString("255", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint8>::max());

	// min limit -1
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("256", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringSint16)
{
	bool ret;

	// tests for sint16
	sint16 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("-32768", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint16>::min());

	// max limit
	ret = NLMISC::fromString("32767", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint16>::max());

	// min limit -1
	ret = NLMISC::fromString("-32769", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("32768", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringUint16)
{
	bool ret;

	// tests for uint16
	uint16 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("0", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint16>::min());

	// max limit
	ret = NLMISC::fromString("65535", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint16>::max());

	// min limit -1
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("65536", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringSint32)
{
	bool ret;

	// tests for sint32
	sint32 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("-2147483648", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint32>::min());

	// max limit
	ret = NLMISC::fromString("2147483647", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint32>::max());

	// min limit -1
	ret = NLMISC::fromString("-2147483649", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("2147483648", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringUint32)
{
	bool ret;

	// tests for uint32
	uint32 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("0", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint32>::min());

	// max limit
	ret = NLMISC::fromString("4294967295", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint32>::max());

	// min limit -1
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(!ret && val == 0);

	// max limit +1
	ret = NLMISC::fromString("4294967296", val);
	EXPECT_TRUE(!ret && val == 0);

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringSint64)
{
	bool ret;

	// tests for sint64
	sint64 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("-9223372036854775808", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint64>::min());

	// max limit
	ret = NLMISC::fromString("9223372036854775807", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<sint64>::max());

	// min limit -1, unable to compare with minimum value because no lower type
	ret = NLMISC::fromString("-9223372036854775809", val);
	// with GCC, it returns min, with VC++ it returns max
	EXPECT_TRUE(ret && (val == std::numeric_limits<sint64>::max() || std::numeric_limits<sint64>::min()));

	// max limit +1, unable to compare with maximum value because no higher type
	ret = NLMISC::fromString("9223372036854775808", val);
	// with GCC, it returns max with VC++ it returns min
	EXPECT_TRUE(ret && (val == std::numeric_limits<sint64>::min() || std::numeric_limits<sint64>::max()));

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringUint64)
{
	bool ret;

	// tests for uint64
	uint64 val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1);

	// min limit
	ret = NLMISC::fromString("0", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint64>::min());

	// max limit
	ret = NLMISC::fromString("18446744073709551615", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint64>::max());

	// min limit -1, unable to compare with minimum value because no lower type
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<uint64>::max());

	// max limit +1, unable to compare with maximum value because no higher type
	ret = NLMISC::fromString("18446744073709551616", val);
	// with GCC, it returns max with VC++ it returns min
	EXPECT_TRUE(ret && (val == std::numeric_limits<uint64>::min() || val == std::numeric_limits<uint64>::max()));

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1);
}

TEST_F(CUTMiscStringCommonTest, fromStringFloat)
{
	bool ret;

	// tests for float
	float val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1.f);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1.f);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0.f);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1.f);

	// min limit
	ret = NLMISC::fromString("-3.4028235e+038", val);
	EXPECT_TRUE(ret && val == -std::numeric_limits<float>::max());

	// min limit towards 0
	ret = NLMISC::fromString("1.1754944e-038", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<float>::min());

	// max limit
	ret = NLMISC::fromString("3.4028235e+038", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<float>::max());

	// min limit -1
	ret = NLMISC::fromString("-3.4028235e+048", val);
	EXPECT_TRUE(ret && val == -std::numeric_limits<float>::infinity());

	// min limit towards 0 -1
	ret = NLMISC::fromString("1.1754944e-048", val);
	EXPECT_TRUE(ret && val == 0.f);

	// max limit +1
	ret = NLMISC::fromString("3.4028235e+048", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<float>::infinity());

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1.2f);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1.f);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10.f);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10.f);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1.f);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1.f);
}

TEST_F(CUTMiscStringCommonTest, fromStringDouble)
{
	bool ret;

	// tests for double
	double val;

	// positive value
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val == 1.0);

	// negative value
	ret = NLMISC::fromString("-1", val);
	EXPECT_TRUE(ret && val == -1.0);

	// bad character
	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && val == 0);

	// right character and bad character
	ret = NLMISC::fromString("1a", val);
	EXPECT_TRUE(ret && val == 1.0);

	// min limit
	ret = NLMISC::fromString("-1.7976931348623158e+308", val);
	EXPECT_TRUE(ret && val == -std::numeric_limits<double>::max());

	// min limit towards 0
	ret = NLMISC::fromString("2.2250738585072014e-308", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<double>::min());

	// max limit
	ret = NLMISC::fromString("1.7976931348623158e+308", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<double>::max());

	// min limit -1
	ret = NLMISC::fromString("3e-408", val);
	EXPECT_TRUE(ret && val == 0.0);

	// max limit +1
	ret = NLMISC::fromString("2e+308", val);
	EXPECT_TRUE(ret && val == std::numeric_limits<double>::infinity());

	// with period
	ret = NLMISC::fromString("1.2", val);
	EXPECT_TRUE(ret && val == 1.2);

	// with coma
	ret = NLMISC::fromString("1,2", val);
	EXPECT_TRUE(ret && val == 1.0);

	// with spaces before
	ret = NLMISC::fromString("  10", val);
	EXPECT_TRUE(ret && val == 10.0);

	// with spaces after
	ret = NLMISC::fromString("10  ", val);
	EXPECT_TRUE(ret && val == 10.0);

	// with 0s before
	ret = NLMISC::fromString("001", val);
	EXPECT_TRUE(ret && val == 1.0);

	// with + before
	ret = NLMISC::fromString("+1", val);
	EXPECT_TRUE(ret && val == 1.0);
}

TEST_F(CUTMiscStringCommonTest, fromStringBool)
{
	bool ret;

	// tests for bool
	bool val;

	// true values
	ret = NLMISC::fromString("1", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("t", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("y", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("T", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("Y", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("true", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("yes", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("YES", val);
	EXPECT_TRUE(ret && val);

	ret = NLMISC::fromString("True", val);
	EXPECT_TRUE(ret && val);

	// false values
	ret = NLMISC::fromString("0", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("f", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("n", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("F", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("N", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("false", val);
	EXPECT_TRUE(ret && !val);

	ret = NLMISC::fromString("no", val);
	EXPECT_TRUE(ret && !val);

	// wrong values
	ret = NLMISC::fromString("foo", val);
	EXPECT_TRUE(!ret && !val);

	ret = NLMISC::fromString("a", val);
	EXPECT_TRUE(!ret && !val);

	ret = NLMISC::fromString("Yesss", val);
	EXPECT_TRUE(!ret && !val);

	ret = NLMISC::fromString("nope", val);
	EXPECT_TRUE(!ret && !val);
}
