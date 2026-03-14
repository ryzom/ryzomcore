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

#include <nel/net/message.h>
#include <nel/misc/common.h>

using std::string;

class CUTNetMessageTest : public testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(CUTNetMessageTest, lockSubMessageWithLongName)
{
	NLNET::CMessage master("BIG");

	// serial some stuff
	for (uint8 i = 0; i < 10; ++i)
	{
		master.serial(i);
	}

	uint32 sizes[4];

	// serial 4 sub messages
	for (uint i = 0; i < 4; ++i)
	{
		NLNET::CMessage sub(NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));

		for (uint8 j = 0; j < i * 4; ++j)
		{
			sub.serial(j);
		}

		string s("A VERY LONG MESSAGE THAT COULD BE A PROBLEM TO HANDLE");
		sub.serial(s);

		sizes[i] = sub.length();

		master.serialMessage(sub);
	}

	// invert the message
	master.invert();

	// now, unpack and check

	// read the first master data
	for (uint8 i = 0; i < 10; ++i)
	{
		uint8 b;
		master.serial(b);

		ASSERT_EQ(b, i);
	}

	// unpack each sub message
	for (uint i = 0; i < 4; ++i)
	{
		uint32 subSize;
		master.serial(subSize);

		master.lockSubMessage(subSize);
		ASSERT_EQ(subSize, sizes[i]);

		ASSERT_EQ(master.getName(), NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));
		ASSERT_EQ(master.length(), sizes[i]);

		for (uint8 j = 0; j < i * 4; ++j)
		{
			uint8 b;
			master.serial(b);
			ASSERT_EQ(b, j);
		}

		string s;
		master.serial(s);
		ASSERT_EQ(s, "A VERY LONG MESSAGE THAT COULD BE A PROBLEM TO HANDLE");

		ASSERT_EQ(master.getPos(), master.length());

		master.unlockSubMessage();
	}

	// rewind the message
	master.seek(master.getHeaderSize(), NLMISC::IStream::begin);

	// read the first master data
	for (uint8 i = 0; i < 10; ++i)
	{
		uint8 b;
		master.serial(b);

		ASSERT_EQ(b, i);
	}

	// assign from each sub message
	for (uint i = 0; i < 4; ++i)
	{
		uint32 subSize;
		master.serial(subSize);

		master.lockSubMessage(subSize);

		ASSERT_EQ(subSize, sizes[i]);

		ASSERT_EQ(master.getName(), NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));
		ASSERT_EQ(master.length(), sizes[i]);

		NLNET::CMessage sub;
		sub.assignFromSubMessage(master);

		for (uint8 j = 0; j < i * 4; ++j)
		{
			uint8 b;
			sub.serial(b);
			ASSERT_EQ(b, j);
		}

		string s;
		sub.serial(s);
		ASSERT_EQ(s, "A VERY LONG MESSAGE THAT COULD BE A PROBLEM TO HANDLE");

		ASSERT_EQ(sub.getPos(), sub.length());

		master.unlockSubMessage();
	}
}

TEST_F(CUTNetMessageTest, lockSubMessage)
{
	NLNET::CMessage master("BIG");

	// serial some stuff
	for (uint8 i = 0; i < 10; ++i)
	{
		master.serial(i);
	}

	sint32 sizes[4];

	// serial 4 sub messages
	for (uint i = 0; i < 4; ++i)
	{
		NLNET::CMessage sub(NLMISC::toString("SUB_%u", i));

		for (uint8 j = 0; j < i * 4; ++j)
		{
			sub.serial(j);
		}

		string s("A MESSAGE");
		sub.serial(s);

		sizes[i] = sub.length();

		master.serialMessage(sub);
	}

	// invert the message
	master.invert();

	// now, unpack and check

	// read the first master data
	for (uint8 i = 0; i < 10; ++i)
	{
		uint8 b;
		master.serial(b);

		ASSERT_EQ(b, i);
	}

	// unpack each sub message
	for (uint i = 0; i < 4; ++i)
	{
		uint32 subSize;
		master.serial(subSize);

		master.lockSubMessage(subSize);
		ASSERT_EQ(subSize, sizes[i]);

		ASSERT_EQ(master.getName(), NLMISC::toString("SUB_%u", i));
		ASSERT_EQ(master.length(), sizes[i]);

		for (uint8 j = 0; j < i * 4; ++j)
		{
			uint8 b;
			master.serial(b);
			ASSERT_EQ(b, j);
		}

		string s;
		master.serial(s);
		ASSERT_EQ(s, "A MESSAGE");

		ASSERT_EQ(master.getPos(), master.length());

		master.unlockSubMessage();
	}

	// rewind the message
	master.seek(master.getHeaderSize(), NLMISC::IStream::begin);

	// read the first master data
	for (uint8 i = 0; i < 10; ++i)
	{
		uint8 b;
		master.serial(b);

		ASSERT_EQ(b, i);
	}

	// assign from each sub message
	for (uint i = 0; i < 4; ++i)
	{
		uint32 subSize;
		master.serial(subSize);

		master.lockSubMessage(subSize);

		ASSERT_EQ(subSize, sizes[i]);

		ASSERT_EQ(master.getName(), NLMISC::toString("SUB_%u", i));
		ASSERT_EQ(master.length(), sizes[i]);

		NLNET::CMessage sub;
		sub.assignFromSubMessage(master);

		for (uint8 j = 0; j < i * 4; ++j)
		{
			uint8 b;
			sub.serial(b);
			ASSERT_EQ(b, j);
		}

		string s;
		sub.serial(s);
		ASSERT_EQ(s, "A MESSAGE");

		ASSERT_EQ(sub.getPos(), sub.length());

		master.unlockSubMessage();
	}
}

TEST_F(CUTNetMessageTest, messageSwap)
{
	NLNET::CMessage msg2;

	string s;
	{
		NLNET::CMessage msg1;
		msg1.setType("NAME", NLNET::CMessage::Request);

		s = "foo1";
		msg1.serial(s);
		s = "foo2";
		msg1.serial(s);
		s = "";

		msg2.swap(msg1);

		// check that ms1 is empty now
		ASSERT_EQ(msg1.length(), 0u);
		ASSERT_FALSE(msg1.typeIsSet());
	}

	ASSERT_FALSE(msg2.isReading());
	msg2.invert();
	ASSERT_TRUE(msg2.typeIsSet());
	ASSERT_EQ(msg2.getName(), "NAME");
	ASSERT_EQ(msg2.getType(), NLNET::CMessage::Request);
	msg2.serial(s);
	ASSERT_EQ(s, "foo1");
	msg2.serial(s);
	ASSERT_EQ(s, "foo2");
}
