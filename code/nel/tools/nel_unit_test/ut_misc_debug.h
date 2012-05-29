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

#ifndef UT_MISC_DEBUG
#define UT_MISC_DEBUG

#include <nel/misc/debug.h>
#include <nel/misc/dynloadlib.h>

class CMiscUnitTestNelLibrary : public NLMISC::INelLibrary { 
	void onLibraryLoaded(bool firstTime) { } 
	void onLibraryUnloaded(bool lastTime) { }  
};
NLMISC_DECL_PURE_LIB(CMiscUnitTestNelLibrary);

// Test suite for CInstanceCounter
class CFoo1
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo1);
};
NL_INSTANCE_COUNTER_IMPL(CFoo1);

class CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo2);
};

NL_INSTANCE_COUNTER_IMPL(CFoo2);

class CFoo3 : public CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo3);
};

NL_INSTANCE_COUNTER_IMPL(CFoo3);


class CUTMiscDebug : public Test::Suite
{
public:
	CUTMiscDebug()
	{
		TEST_ADD(CUTMiscDebug::testInstanceCounter)
		TEST_ADD(CUTMiscDebug::testInstanceCounterOutput)
	}
	
private:
	void testInstanceCounter()
	{
		sint32 n;
		{
			CFoo1	foo1;

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 1);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 1);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 1);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -1);

		{
			CFoo1	foo1;
			CFoo1	other(foo1);

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 1);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -2);

		{
			CFoo1	foo1;
			CFoo1	other;

			foo1 = other;

			sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);

			NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

			n = NL_GET_INSTANCE_COUNTER(CFoo1);
			TEST_ASSERT(n == 2);
			n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
			TEST_ASSERT(n == 0);
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -2);

		CFoo1	*foo1s[10];
		CFoo2	*foo2s[10];
		CFoo3	*foo3s[10];
		for (uint i=0; i<10; ++i)
		{
			foo1s[i] = new CFoo1;
			foo2s[i] = new CFoo2;
			foo3s[i] = new CFoo3;

		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == 8);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == 10);

		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 20);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == 0);

		for (uint i=0; i<5; ++i)
		{
			delete foo1s[i];
			delete foo2s[i];
			delete foo3s[i];
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 5);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -5);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 10);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == -10);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 5);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == -5);
		for (uint i=5; i<10; ++i)
		{
			delete foo1s[i];
			delete foo2s[i];
			delete foo3s[i];
		}
		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		TEST_ASSERT(n == -10);
		n = NL_GET_INSTANCE_COUNTER(CFoo2);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
		TEST_ASSERT(n == -20);
		n = NL_GET_INSTANCE_COUNTER(CFoo3);
		TEST_ASSERT(n == 0);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
		TEST_ASSERT(n == -10);
	}

	void testInstanceCounterOutput()
	{
		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

		CFoo1	*foo1s[10];
		CFoo2	*foo2s[10];
		CFoo3	*foo3s[10];
		for (uint i=0; i<10; ++i)
		{
			foo1s[i] = new CFoo1;
			foo2s[i] = new CFoo2;
			foo3s[i] = new CFoo3;

		}

		string ref = "Listing 3 Instance counters :\n"
					 "  Class 'CFoo1               ', \t        10 instances, \t        10 delta\n"
					 "  Class 'CFoo2               ', \t        20 instances, \t        20 delta\n"
					 "  Class 'CFoo3               ', \t        10 instances, \t        10 delta\n";

		string ret = NLMISC::CInstanceCounterManager::getInstance().displayCounters();

		nlinfo("%s", ref.c_str());
		nlinfo("%s", ret.c_str());
		TEST_ASSERT(ref == ret);
	}
};

#endif
