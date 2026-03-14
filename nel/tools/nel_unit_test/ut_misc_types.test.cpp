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

#include <nel/misc/types_nl.h>

class CUTMiscTypesTest : public testing::Test
{
};

TEST_F(CUTMiscTypesTest, basicTypes)
{
	// this doesn't work on 64bit architectures
	// Test_ASSERT(sizeof(uint) == sizeof(void*));

	EXPECT_EQ(sizeof(sint8), 1);
	EXPECT_EQ(sizeof(uint8), 1);
	EXPECT_EQ(sizeof(sint16), 2);
	EXPECT_EQ(sizeof(uint16), 2);
	EXPECT_EQ(sizeof(sint32), 4);
	EXPECT_EQ(sizeof(uint32), 4);
	EXPECT_EQ(sizeof(sint64), 8);
	EXPECT_EQ(sizeof(uint64), 8);

	EXPECT_GE(sizeof(sint), 4);
	EXPECT_GE(sizeof(uint), 4);
}
