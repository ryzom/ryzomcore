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
		TEST_ADD(CUTMiscCommon::humanReadableToBytes);

		// Add a line here when adding a new test METHOD
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

		// 1 Megabyte
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
};

#endif
