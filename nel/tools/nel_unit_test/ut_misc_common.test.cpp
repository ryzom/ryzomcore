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
#include <cstdlib>
#include <string>
#include <vector>

#include <nel/misc/algo.h>
#include <nel/misc/common.h>
#include <nel/misc/ucstring.h>

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

#ifdef NL_OS_WINDOWS
TEST_F(CUTMiscCommonTest, launchProgramSearchesPath)
{
	// a bare program name is searched for in the system directories and PATH, and gets .exe
	EXPECT_EQ(7, NLMISC::launchProgramAndWaitForResult("cmd", "/c exit 7"));
	EXPECT_EQ(0, NLMISC::launchProgramAndWaitForResult("where.exe", "cmd"));
	EXPECT_TRUE(NLMISC::launchProgram("cmd", "/c exit 0"));
	vector<string> arguments;
	arguments.push_back("/c");
	arguments.push_back("exit 0");
	EXPECT_TRUE(NLMISC::launchProgramArray("cmd", arguments));

	// a program name with a directory is launched as is
	const char *comSpec = getenv("ComSpec");
	ASSERT_TRUE(comSpec != NULL);
	EXPECT_EQ(3, NLMISC::launchProgramAndWaitForResult(comSpec, "/c exit 3"));

	// a program that does not exist fails
	EXPECT_EQ(-1, NLMISC::launchProgramAndWaitForResult("nel_unit_test_no_such_program", "", false));
	EXPECT_FALSE(NLMISC::launchProgram("nel_unit_test_no_such_program", "", false));
	EXPECT_FALSE(NLMISC::launchProgram("C:\\nel_unit_test_no_such_directory\\no_such_program.exe", "", false));

	// a program name longer than MAX_PATH fails as well
	EXPECT_EQ(-1, NLMISC::launchProgramAndWaitForResult(string(400, 'a'), "", false));
	EXPECT_EQ(-1, NLMISC::launchProgramAndWaitForResult("C:\\" + string(400, 'a') + ".exe", "", false));
}
#endif

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

TEST_F(CUTMiscCommonTest, splitStringEmptySeparator)
{
	vector<string> list;

	// a separator splits, repeated separators give no empty string
	NLMISC::splitString("hello|bye|||bee", "|", list);
	ASSERT_EQ(3u, list.size());
	EXPECT_EQ("hello", list[0]);
	EXPECT_EQ("bye", list[1]);
	EXPECT_EQ("bee", list[2]);

	// an empty separator returns the whole string
	NLMISC::splitString("hello|bye", "", list);
	ASSERT_EQ(1u, list.size());
	EXPECT_EQ("hello|bye", list[0]);

	// an empty string with an empty separator returns nothing, and clears the list
	NLMISC::splitString("", "", list);
	EXPECT_TRUE(list.empty());
}

TEST_F(CUTMiscCommonTest, splitUCStringEmptySeparator)
{
	vector<ucstring> list;

	// a separator splits, repeated separators give no empty string
	NLMISC::splitUCString(ucstring("hello bye   bee"), ucstring(" "), list);
	ASSERT_EQ(3u, list.size());
	EXPECT_TRUE(list[0] == ucstring("hello"));
	EXPECT_TRUE(list[1] == ucstring("bye"));
	EXPECT_TRUE(list[2] == ucstring("bee"));

	// an empty separator returns the whole string
	NLMISC::splitUCString(ucstring("hello bye"), ucstring(), list);
	ASSERT_EQ(1u, list.size());
	EXPECT_TRUE(list[0] == ucstring("hello bye"));

	// an empty string with an empty separator returns nothing, and clears the list
	NLMISC::splitUCString(ucstring(), ucstring(), list);
	EXPECT_TRUE(list.empty());
}

TEST_F(CUTMiscCommonTest, explodeEmptySeparator)
{
	vector<string> list;

	// a separator splits, empty strings are kept unless skipEmpty is set
	NLMISC::explode(string("a,,b"), string(","), list);
	ASSERT_EQ(3u, list.size());
	EXPECT_EQ("a", list[0]);
	EXPECT_EQ("", list[1]);
	EXPECT_EQ("b", list[2]);
	NLMISC::explode(string("a,,b"), string(","), list, true);
	ASSERT_EQ(2u, list.size());
	EXPECT_EQ("a", list[0]);
	EXPECT_EQ("b", list[1]);

	// an empty separator returns the whole string
	NLMISC::explode(string("a,,b"), string(), list);
	ASSERT_EQ(1u, list.size());
	EXPECT_EQ("a,,b", list[0]);
	NLMISC::explode(string("a,,b"), string(), list, true);
	ASSERT_EQ(1u, list.size());
	EXPECT_EQ("a,,b", list[0]);

	// an empty string with an empty separator is one empty string, or nothing with skipEmpty,
	// as it is with a separator
	NLMISC::explode(string(), string(","), list);
	ASSERT_EQ(1u, list.size());
	EXPECT_EQ("", list[0]);
	NLMISC::explode(string(), string(), list);
	ASSERT_EQ(1u, list.size());
	EXPECT_EQ("", list[0]);
	NLMISC::explode(string(), string(","), list, true);
	EXPECT_TRUE(list.empty());
	NLMISC::explode(string(), string(), list, true);
	EXPECT_TRUE(list.empty());

	// ucstring
	vector<ucstring> uclist;
	NLMISC::explode(ucstring("a,,b"), ucstring(), uclist);
	ASSERT_EQ(1u, uclist.size());
	EXPECT_TRUE(uclist[0] == ucstring("a,,b"));
}
