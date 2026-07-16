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

#include <gtest/gtest.h>

#include <nel/misc/app_context.h>
#include <nel/misc/path.h>

class CSafeSingleton
{
	NL_INSTANCE_COUNTER_DECL(CSafeSingleton);
	NLMISC_SAFE_SINGLETON_DECL(CSafeSingleton);
	CSafeSingleton() { }
};

NL_INSTANCE_COUNTER_IMPL(CSafeSingleton);
NLMISC_SAFE_SINGLETON_IMPL(CSafeSingleton);

class CUnsafeSingleton
{
	NL_INSTANCE_COUNTER_DECL(CUnsafeSingleton);

	static CUnsafeSingleton *_Instance;

	CUnsafeSingleton() { }
	CUnsafeSingleton(const CUnsafeSingleton &other) { }

public:
	static CUnsafeSingleton &getInstance()
	{
		if (_Instance == nullptr)
			_Instance = new CUnsafeSingleton;
		return *_Instance;
	}
};

NL_INSTANCE_COUNTER_IMPL(CUnsafeSingleton);
CUnsafeSingleton *CUnsafeSingleton::_Instance = nullptr;

// Test suite for Singleton behavior
class CUTMiscSingletonTest : public testing::Test
{
protected:
	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
	}

	void TearDown() override
	{
	}

	friend class CDynLibTest;
};

TEST_F(CUTMiscSingletonTest, createSingleton)
{
	EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CSafeSingleton), 0);
	// createSingleton
	{
		CSafeSingleton &ss = CSafeSingleton::getInstance();

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CSafeSingleton), 1);
		EXPECT_EQ(NL_GET_INSTANCE_COUNTER(CSafeSingleton), 1);

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CUnsafeSingleton), 0);
		CUnsafeSingleton &us = CUnsafeSingleton::getInstance();

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CUnsafeSingleton), 1);
	}

	// accessSingleton
	{
		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CSafeSingleton), 1);
		CSafeSingleton &ss = CSafeSingleton::getInstance();

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CSafeSingleton), 1);

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CUnsafeSingleton), 1);
		CUnsafeSingleton &us = CUnsafeSingleton::getInstance();

		EXPECT_EQ(NL_GET_LOCAL_INSTANCE_COUNTER(CUnsafeSingleton), 1);
	}
}

/*TEST_F(CUTMiscSingletonTest, multiDllSingleton)
{
    EXPECT_TRUE(NLMISC::CCommandRegistry::getInstance().exists("aDynLibCommand") == false);

    CLibrary lib;
    if (lib.loadLibrary("dyn_lib_test", true, true, true) != true)
    {
        EXPECT_TRUE_MSG(false, "failed to reload the dll for testing singletons across dlls");
        return;
    }

    EXPECT_TRUE(NLMISC::CCommandRegistry::getInstance().isCommand("aDynLibCommand") == true);

    IDynLibTest *libTest = dynamic_cast<IDynLibTest*>(lib.getNelLibraryInterface());
    EXPECT_TRUE(libTest != NULL);

    libTest->testSingleton(this);
}*/
