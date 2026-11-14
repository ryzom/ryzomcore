// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <cstring>
#include <vector>

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/system_info.h>
#include <nel/misc/thread.h>

namespace {

// allocates and frees blocks of many sizes until told to stop
class CHeapUser : public NLMISC::IRunnable
{
public:
	CHeapUser()
	    : Stop(false)
	{
	}

	virtual void run() NL_OVERRIDE
	{
		std::vector<char *> blocks(256, (char *)NULL);
		uint i = 0;
		while (!Stop)
		{
			const uint slot = i % (uint)blocks.size();
			delete[] blocks[slot];
			const size_t size = 16 + (i * 7919) % 20000;
			blocks[slot] = new char[size];
			blocks[slot][0] = (char)i;
			++i;
		}
		for (uint j = 0; j < blocks.size(); ++j)
			delete[] blocks[j];
	}

	volatile bool Stop;
};

} // namespace

TEST(CUTMiscSystemInfo, allocatedSystemMemoryFollowsTheProcess)
{
	const uint64 before = NLMISC::CSystemInfo::getAllocatedSystemMemory();
	EXPECT_GT(before, (uint64)0);

	// memory that is written to is memory the process uses, whatever the system counts
	const size_t size = 64 * 1024 * 1024;
	// volatile: the block must really be allocated and written, whatever the optimiser sees
	char *volatile block = new char[size];
	memset(block, 1, size);
	const uint64 with = NLMISC::CSystemInfo::getAllocatedSystemMemory();
	EXPECT_GT(with, before + size / 2);
	EXPECT_EQ(block[size - 1], 1);
	delete[] block;
}

// The memory is asked for by the main thread of a service while its other threads allocate and free. It was
// counted by walking the heaps, which could go astray on a heap in use and never come back.
TEST(CUTMiscSystemInfo, allocatedSystemMemoryWhileOtherThreadsAllocate)
{
	const uint threadCount = 4;
	CHeapUser users[threadCount];
	NLMISC::IThread *threads[threadCount];
	for (uint i = 0; i < threadCount; ++i)
	{
		threads[i] = NLMISC::IThread::create(&users[i]);
		threads[i]->start();
	}

	for (uint i = 0; i < 2000; ++i)
		EXPECT_GT(NLMISC::CSystemInfo::getAllocatedSystemMemory(), (uint64)0);

	for (uint i = 0; i < threadCount; ++i)
		users[i].Stop = true;
	for (uint i = 0; i < threadCount; ++i)
	{
		threads[i]->wait();
		delete threads[i];
	}
}
