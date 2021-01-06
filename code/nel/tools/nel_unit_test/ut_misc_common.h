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

#ifndef UT_MISC_COMMON
#define UT_MISC_COMMON

#include <limits>
#include <nel/misc/common.h>

struct CUTMiscCommon : public Test::Suite
{
	CUTMiscCommon()
	{
		TEST_ADD(CUTMiscCommon::bytesToHumanReadableUnits);
		TEST_ADD(CUTMiscCommon::humanReadableToBytes);
		TEST_ADD(CUTMiscCommon::encodeURIComponent);
		TEST_ADD(CUTMiscCommon::decodeURIComponent);

		// Add a line here when adding a new test METHOD
	}

	void bytesToHumanReadableUnits()
	{
		std::vector<std::string> units;

		std::string res;

		// no unit, returns an empty string
		res = NLMISC::bytesToHumanReadableUnits(0, units);
		TEST_ASSERT(res.empty());

		// support bytes
		units.push_back("B");

		// 0 bytes
		res = NLMISC::bytesToHumanReadableUnits(0, units);
		TEST_ASSERT(res == "0 B");

		// 1000 bytes in B
		res = NLMISC::bytesToHumanReadableUnits(1000, units);
		TEST_ASSERT(res == "1000 B");

		// 1024 bytes in B
		res = NLMISC::bytesToHumanReadableUnits(1024, units);
		TEST_ASSERT(res == "1024 B");

		// support kibibytes
		units.push_back("KiB");

		// 1000 bytes in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1000, units);
		TEST_ASSERT(res == "1000 B");

		// 1024 bytes in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1024, units);
		TEST_ASSERT(res == "1024 B");

		// 1 MB in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1000 * 1000, units);
		TEST_ASSERT(res == "976 KiB");

		// 1 MiB in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1024 * 1024, units);
		TEST_ASSERT(res == "1024 KiB");

		// 1 GB in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1000 * 1000 * 1000, units);
		TEST_ASSERT(res == "976562 KiB");

		// 1 GiB in B or KiB
		res = NLMISC::bytesToHumanReadableUnits(1024 * 1024 * 1024, units);
		TEST_ASSERT(res == "1048576 KiB");

		// support mebibytes
		units.push_back("MiB");

		// 1 GB in B, KiB or MiB
		res = NLMISC::bytesToHumanReadableUnits(1000 * 1000 * 1000, units);
		TEST_ASSERT(res == "953 MiB");

		// 1 GiB in B, KiB or MiB
		res = NLMISC::bytesToHumanReadableUnits(1024 * 1024 * 1024, units);
		TEST_ASSERT(res == "1024 MiB");
	}

	void humanReadableToBytes()
	{
		uint64 bytes = 0;

		// kiB is a wrong unit
		bytes = NLMISC::humanReadableToBytes("1kiB");
		TEST_ASSERT(bytes == 1);

		// 1 kibibyte
		bytes = NLMISC::humanReadableToBytes("1KiB");
		TEST_ASSERT(bytes == 1024);

		// 1 mebibyte
		bytes = NLMISC::humanReadableToBytes("1MiB");
		TEST_ASSERT(bytes == 1024*1024);

		// 1 kilobyte
		bytes = NLMISC::humanReadableToBytes("1KB");
		TEST_ASSERT(bytes == 1000);

		// 1 kilobyte
		bytes = NLMISC::humanReadableToBytes("1kB");
		TEST_ASSERT(bytes == 1000);

		// 1 megabyte
		bytes = NLMISC::humanReadableToBytes("1MB");
		TEST_ASSERT(bytes == 1000*1000);

		// 1 byte
		bytes = NLMISC::humanReadableToBytes("1B");
		TEST_ASSERT(bytes == 1);

		// 1 byte
		bytes = NLMISC::humanReadableToBytes("1");
		TEST_ASSERT(bytes == 1);

		// kiB is a wrong unit
		bytes = NLMISC::humanReadableToBytes("1 kiB");
		TEST_ASSERT(bytes == 1);

		// 1 kibibyte
		bytes = NLMISC::humanReadableToBytes("1 KiB");
		TEST_ASSERT(bytes == 1024);

		// 1 mebibyte
		bytes = NLMISC::humanReadableToBytes("1 MiB");
		TEST_ASSERT(bytes == 1024*1024);

		// 1 kilobyte
		bytes = NLMISC::humanReadableToBytes("1 KB");
		TEST_ASSERT(bytes == 1000);

		// 1 kilobyte
		bytes = NLMISC::humanReadableToBytes("1 kB");
		TEST_ASSERT(bytes == 1000);

		// 1 megabyte
		bytes = NLMISC::humanReadableToBytes("1 MB");
		TEST_ASSERT(bytes == 1000*1000);

		// 1 byte
		bytes = NLMISC::humanReadableToBytes("1 B");
		TEST_ASSERT(bytes == 1);

		// not a number
		bytes = NLMISC::humanReadableToBytes("AB");
		TEST_ASSERT(bytes == 0);

		// not a positive number
		bytes = NLMISC::humanReadableToBytes("-1 B");
		TEST_ASSERT(bytes == 0);
	}

	void encodeURIComponent()
	{
		TEST_ASSERT("%00" == NLMISC::encodeURIComponent(std::string("\x00", 1)));
		TEST_ASSERT("%0A" == NLMISC::encodeURIComponent(std::string("\x0A", 1)));
		TEST_ASSERT("%A0" == NLMISC::encodeURIComponent(std::string("\xA0", 1)));
		TEST_ASSERT("a%20b" == NLMISC::encodeURIComponent("a b"));
		TEST_ASSERT("a%2Bb" == NLMISC::encodeURIComponent("a+b"));
	}

	void decodeURIComponent()
	{
		TEST_ASSERT(std::string("\x00", 1) == NLMISC::decodeURIComponent(std::string("\x00", 1)));
		TEST_ASSERT(std::string("\x0A", 1) == NLMISC::decodeURIComponent(std::string("\x0A", 1)));
		TEST_ASSERT(std::string("\xA0", 1) == NLMISC::decodeURIComponent(std::string("\xA0", 1)));
		TEST_ASSERT("a b" == NLMISC::decodeURIComponent("a%20b"));
		TEST_ASSERT("a+b" == NLMISC::decodeURIComponent("a%2Bb"));

		TEST_ASSERT("a%A" == NLMISC::decodeURIComponent("a%A"));
		TEST_ASSERT("a%AX" == NLMISC::decodeURIComponent("a%AX"));
	}
};

#endif
