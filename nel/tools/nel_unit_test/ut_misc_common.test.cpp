// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
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

#include <cstdint>
#include <string>
#include <vector>

#include <nel/misc/common.h>

using std::string;
using std::vector;

class CUTMiscCommonTest : public testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(CUTMiscCommonTest, bytesToHumanReadableUnits)
{
	vector<string> units;
	string res;

	// no unit, returns an empty string
	res = NLMISC::bytesToHumanReadableUnits(0, units);
	EXPECT_TRUE(res.empty());

	// support bytes
	units.push_back("B");

	// 0 bytes
	res = NLMISC::bytesToHumanReadableUnits(0, units);
	EXPECT_EQ(res, "0 B");

	// 1000 bytes in B
	res = NLMISC::bytesToHumanReadableUnits(1000, units);
	EXPECT_EQ(res, "1000 B");

	// 1024 bytes in B
	res = NLMISC::bytesToHumanReadableUnits(1024, units);
	EXPECT_EQ(res, "1024 B");

	// support kibibytes
	units.push_back("KiB");

	// 1000 bytes in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1000, units);
	EXPECT_EQ(res, "1000 B");

	// 1024 bytes in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1024, units);
	EXPECT_EQ(res, "1024 B");

	// 1 MB in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1000 * 1000, units);
	EXPECT_EQ(res, "976 KiB");

	// 1 MiB in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1024 * 1024, units);
	EXPECT_EQ(res, "1024 KiB");

	// 1 GB in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1000 * 1000 * 1000, units);
	EXPECT_EQ(res, "976562 KiB");

	// 1 GiB in B or KiB
	res = NLMISC::bytesToHumanReadableUnits(1024 * 1024 * 1024, units);
	EXPECT_EQ(res, "1048576 KiB");

	// support mebibytes
	units.push_back("MiB");

	// 1 GB in B, KiB or MiB
	res = NLMISC::bytesToHumanReadableUnits(1000 * 1000 * 1000, units);
	EXPECT_EQ(res, "953 MiB");

	// 1 GiB in B, KiB or MiB
	res = NLMISC::bytesToHumanReadableUnits(1024 * 1024 * 1024, units);
	EXPECT_EQ(res, "1024 MiB");
}

TEST_F(CUTMiscCommonTest, humanReadableToBytes)
{
	uint64 bytes = 0;

	// kiB is a wrong unit
	bytes = NLMISC::humanReadableToBytes("1kiB");
	EXPECT_EQ(bytes, 1);

	// 1 kibibyte
	bytes = NLMISC::humanReadableToBytes("1KiB");
	EXPECT_EQ(bytes, 1024);

	// 1 mebibyte
	bytes = NLMISC::humanReadableToBytes("1MiB");
	EXPECT_EQ(bytes, 1024 * 1024);

	// 1 kilobyte
	bytes = NLMISC::humanReadableToBytes("1KB");
	EXPECT_EQ(bytes, 1000);

	// 1 kilobyte
	bytes = NLMISC::humanReadableToBytes("1kB");
	EXPECT_EQ(bytes, 1000);

	// 1 megabyte
	bytes = NLMISC::humanReadableToBytes("1MB");
	EXPECT_EQ(bytes, 1000 * 1000);

	// 1 byte
	bytes = NLMISC::humanReadableToBytes("1B");
	EXPECT_EQ(bytes, 1);

	// 1 byte
	bytes = NLMISC::humanReadableToBytes("1");
	EXPECT_EQ(bytes, 1);

	// kiB is a wrong unit
	bytes = NLMISC::humanReadableToBytes("1 kiB");
	EXPECT_EQ(bytes, 1);

	// 1 kibibyte
	bytes = NLMISC::humanReadableToBytes("1 KiB");
	EXPECT_EQ(bytes, 1024);

	// 1 mebibyte
	bytes = NLMISC::humanReadableToBytes("1 MiB");
	EXPECT_EQ(bytes, 1024 * 1024);

	// 1 kilobyte
	bytes = NLMISC::humanReadableToBytes("1 KB");
	EXPECT_EQ(bytes, 1000);

	// 1 kilobyte
	bytes = NLMISC::humanReadableToBytes("1 kB");
	EXPECT_EQ(bytes, 1000);

	// 1 megabyte
	bytes = NLMISC::humanReadableToBytes("1 MB");
	EXPECT_EQ(bytes, 1000 * 1000);

	// 1 byte
	bytes = NLMISC::humanReadableToBytes("1 B");
	EXPECT_EQ(bytes, 1);

	// not a number
	bytes = NLMISC::humanReadableToBytes("AB");
	EXPECT_EQ(bytes, 0);

	// not a positive number
	bytes = NLMISC::humanReadableToBytes("-1 B");
	EXPECT_EQ(bytes, 0);
}

TEST_F(CUTMiscCommonTest, encodeURIComponent)
{
	EXPECT_EQ("%00", NLMISC::encodeURIComponent(std::string("\x00", 1)));
	EXPECT_EQ("%0A", NLMISC::encodeURIComponent(std::string("\x0A", 1)));
	EXPECT_EQ("%A0", NLMISC::encodeURIComponent(std::string("\xA0", 1)));
	EXPECT_EQ("a%20b", NLMISC::encodeURIComponent("a b"));
	EXPECT_EQ("a%2Bb", NLMISC::encodeURIComponent("a+b"));
}

TEST_F(CUTMiscCommonTest, decodeURIComponent)
{
	EXPECT_EQ(std::string("\x00", 1), NLMISC::decodeURIComponent(std::string("\x00", 1)));
	EXPECT_EQ(std::string("\x0A", 1), NLMISC::decodeURIComponent(std::string("\x0A", 1)));
	EXPECT_EQ(std::string("\xA0", 1), NLMISC::decodeURIComponent(std::string("\xA0", 1)));
	EXPECT_EQ("a b", NLMISC::decodeURIComponent("a%20b"));
	EXPECT_EQ("a+b", NLMISC::decodeURIComponent("a%2Bb"));

	EXPECT_EQ("a%A", NLMISC::decodeURIComponent("a%A"));
	EXPECT_EQ("a%AX", NLMISC::decodeURIComponent("a%AX"));
}
