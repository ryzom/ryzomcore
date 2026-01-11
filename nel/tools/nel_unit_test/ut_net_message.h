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

#ifndef UT_NET_MESSAGE
#define UT_NET_MESSAGE

class CUTNetMessage: public Test::Suite
{
public:
	CUTNetMessage ()
	{
		TEST_ADD(CUTNetMessage::messageSwap);
		TEST_ADD(CUTNetMessage::lockSubMEssage);
		TEST_ADD(CUTNetMessage::lockSubMEssageWithLongName);

	}

	void lockSubMEssageWithLongName()
	{
		NLNET::CMessage master("BIG");

		// serial some stuff
		for (uint8 i=0; i<10; ++i)
		{
			master.serial(i);
		}

		uint32 sizes[4];

		// serial 4 sub messages
		for (uint i=0; i<4; ++i)
		{
			NLNET::CMessage sub(NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));

			for (uint8 j=0; j<i*4; ++j)
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
		for (uint8 i=0; i<10; ++i)
		{
			uint8 b;
			master.serial(b);

			TEST_ASSERT(b == i);
		}

		// unpack each sub message
		for (uint i=0; i<4; ++i)
		{
			uint32 subSize;
			master.serial(subSize);

			master.lockSubMessage(subSize);
			TEST_ASSERT(subSize == sizes[i]);

			TEST_ASSERT(master.getName() == NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));
			TEST_ASSERT(master.length() == sizes[i]);

			for (uint8 j=0; j<i*4; ++j)
			{
				uint8 b;
				master.serial(b);
				TEST_ASSERT(b == j);
			}

			string s;
			master.serial(s);
			TEST_ASSERT(s == "A VERY LONG MESSAGE THAT COULD BE A PROBLEM TO HANDLE");

			TEST_ASSERT(master.getPos() == master.length());

			master.unlockSubMessage();
		}

		// rewind the message
		master.seek(master.getHeaderSize(), NLMISC::IStream::begin);

		// read the first master data
		for (uint8 i=0; i<10; ++i)
		{
			uint8 b;
			master.serial(b);

			TEST_ASSERT(b == i);
		}

		// assign from each sub message
		for (uint i=0; i<4; ++i)
		{
			uint32 subSize;
			master.serial(subSize);

			master.lockSubMessage(subSize);

			TEST_ASSERT(subSize == sizes[i]);

			TEST_ASSERT(master.getName() == NLMISC::toString("A_VERY_LONG_SUB_MESSAGE_NAME_%u", i));
			TEST_ASSERT(master.length() == sizes[i]);

			NLNET::CMessage sub;
			sub.assignFromSubMessage(master);

			for (uint8 j=0; j<i*4; ++j)
			{
				uint8 b;
				sub.serial(b);
				TEST_ASSERT(b == j);
			}

			string s;
			sub.serial(s);
			TEST_ASSERT(s == "A VERY LONG MESSAGE THAT COULD BE A PROBLEM TO HANDLE");

			TEST_ASSERT(sub.getPos() == sub.length());

			master.unlockSubMessage();
		}

	}

	void lockSubMEssage()
	{
		NLNET::CMessage master("BIG");

		// serial some stuff
		for (uint8 i=0; i<10; ++i)
		{
			master.serial(i);
		}

		sint32 sizes[4];

		// serial 4 sub messages
		for (uint i=0; i<4; ++i)
		{
			NLNET::CMessage sub(NLMISC::toString("SUB_%u", i));

			for (uint8 j=0; j<i*4; ++j)
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
		for (uint8 i=0; i<10; ++i)
		{
			uint8 b;
			master.serial(b);

			TEST_ASSERT(b == i);
		}

		// unpack each sub message
		for (uint i=0; i<4; ++i)
		{
			uint32 subSize;
			master.serial(subSize);

			master.lockSubMessage(subSize);
			TEST_ASSERT(subSize == sizes[i]);

			TEST_ASSERT(master.getName() == NLMISC::toString("SUB_%u", i));
			TEST_ASSERT(master.length() == sizes[i]);

			for (uint8 j=0; j<i*4; ++j)
			{
				uint8 b;
				master.serial(b);
				TEST_ASSERT(b == j);
			}

			string s;
			master.serial(s);
			TEST_ASSERT(s == "A MESSAGE");

			TEST_ASSERT(master.getPos() == master.length());

			master.unlockSubMessage();
		}

		// rewind the message
		master.seek(master.getHeaderSize(), NLMISC::IStream::begin);

		// read the first master data
		for (uint8 i=0; i<10; ++i)
		{
			uint8 b;
			master.serial(b);

			TEST_ASSERT(b == i);
		}

		// assign from each sub message
		for (uint i=0; i<4; ++i)
		{
			uint32 subSize;
			master.serial(subSize);

			master.lockSubMessage(subSize);

			TEST_ASSERT(subSize == sizes[i]);

			TEST_ASSERT(master.getName() == NLMISC::toString("SUB_%u", i));
			TEST_ASSERT(master.length() == sizes[i]);

			NLNET::CMessage sub;
			sub.assignFromSubMessage(master);

			for (uint8 j=0; j<i*4; ++j)
			{
				uint8 b;
				sub.serial(b);
				TEST_ASSERT(b == j);
			}

			string s;
			sub.serial(s);
			TEST_ASSERT(s == "A MESSAGE");

			TEST_ASSERT(sub.getPos() == sub.length());

			master.unlockSubMessage();
		}

	}
	
	void messageSwap()
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
			TEST_ASSERT(msg1.length() == 0);
			TEST_ASSERT(!msg1.typeIsSet());
		}

		TEST_ASSERT(!msg2.isReading());
		msg2.invert();
		TEST_ASSERT(msg2.typeIsSet());
		TEST_ASSERT(msg2.getName() == "NAME");
		TEST_ASSERT(msg2.getType() == NLNET::CMessage::Request);
		msg2.serial(s);
		TEST_ASSERT(s == "foo1");
		msg2.serial(s);
		TEST_ASSERT(s == "foo2");
	}
};

#endif
