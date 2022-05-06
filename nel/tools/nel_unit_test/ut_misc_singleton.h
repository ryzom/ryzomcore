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

#ifndef UT_MISC_SINGLETON
#define UT_MISC_SINGLETON

#include <nel/misc/command.h>

class CSafeSingleton
{
	NL_INSTANCE_COUNTER_DECL(CSafeSingleton);
	NLMISC_SAFE_SINGLETON_DECL(CSafeSingleton);
	CSafeSingleton() {}
};

NL_INSTANCE_COUNTER_IMPL(CSafeSingleton);
NLMISC_SAFE_SINGLETON_IMPL(CSafeSingleton);


class CUnsafeSingleton
{
	NL_INSTANCE_COUNTER_DECL(CUnsafeSingleton);

	static CUnsafeSingleton	*_Instance;

	CUnsafeSingleton() {}
	CUnsafeSingleton(const CUnsafeSingleton &other) {}

public:

	static CUnsafeSingleton &getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CUnsafeSingleton;
		return *_Instance;
	}
};

NL_INSTANCE_COUNTER_IMPL(CUnsafeSingleton);
CUnsafeSingleton	*CUnsafeSingleton::_Instance = NULL;


// Test suite for Singleton behavior
class CUTMiscSingleton : public Test::Suite
{
	std::string WorkingPath;
	std::string OldPath;
public:
	CUTMiscSingleton()
	{
		TEST_ADD(CUTMiscSingleton::createSingleton);
		TEST_ADD(CUTMiscSingleton::accessSingleton);
		//TEST_ADD(CUTMiscSingleton::multiDllSingleton);
	}

	void setup()
	{
		OldPath = NLMISC::CPath::getCurrentPath();

		NLMISC::CPath::setCurrentPath(WorkingPath.c_str());
	}

	void tear_down()
	{
		NLMISC::CPath::setCurrentPath(OldPath.c_str());
	}
	
	void createSingleton()
	{
		TEST_ASSERT(NLMISC::CInstanceCounterManager::getInstance().getInstanceCounter("CSafeSingleton") == 0);
		CSafeSingleton &ss = CSafeSingleton::getInstance();
	
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CSafeSingleton) == 1);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CUnsafeSingleton) == 0);
		CUnsafeSingleton &us = CUnsafeSingleton::getInstance();
	
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CUnsafeSingleton) == 1);
	}

	void accessSingleton()
	{
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CSafeSingleton) == 1);
		CSafeSingleton &ss = CSafeSingleton::getInstance();
	
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CSafeSingleton) == 1);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CUnsafeSingleton) == 1);
		CUnsafeSingleton &us = CUnsafeSingleton::getInstance();
	
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CUnsafeSingleton) == 1);
	}

	/*void multiDllSingleton()
	{
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().exists("aDynLibCommand") == false);

		CLibrary lib;
		if (lib.loadLibrary("dyn_lib_test", true, true, true) != true)
		{
			TEST_ASSERT_MSG(false, "failed to reload the dll for testing singletons across dlls");
			return;
		}

		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().isCommand("aDynLibCommand") == true);

		IDynLibTest *libTest = dynamic_cast<IDynLibTest*>(lib.getNelLibraryInterface());
		TEST_ASSERT(libTest != NULL);

		libTest->testSingleton(this);
	}*/

	void assertmentWrapper(Test::Source src)
	{
		assertment(src);
	}

	bool continue_after_failureWrapper()
	{
		return continue_after_failure();
	}

	friend class CDynLibTest;
};

#endif
