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

#include <string>

#include <nel/misc/variable.h>

using std::string;

class CUTMiscVariableTest : public testing::Test
{
protected:
	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
		NLMISC::createDebug(nullptr);
	}

	void TearDown() override
	{
	}
};

TEST_F(CUTMiscVariableTest, declareVar)
{
	auto &command_registry = NLMISC::CCommandRegistry::getInstance();
	{
		NLMISC::CVariable<string> myLocalVar("test", "myLocalVar", "no help", "");

		EXPECT_EQ(myLocalVar.get(), string(""));
		ASSERT_TRUE(command_registry.execute("myLocalVar foo", (*NLMISC::InfoLog)));
		EXPECT_EQ(myLocalVar.get(), string("foo"));
	}

	EXPECT_FALSE(command_registry.execute("myLocalVar foo", (*NLMISC::InfoLog)));
}
