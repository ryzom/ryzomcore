// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
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

#ifndef UT_MISC_BIT_SET
#define UT_MISC_BIT_SET

#include <nel/misc/bit_set.h>
#include <nel/misc/mem_stream.h>

struct CUTMiscBitSet : public Test::Suite
{
	CUTMiscBitSet()
	{
		TEST_ADD(CUTMiscBitSet::testSerialRoundTrip);
		TEST_ADD(CUTMiscBitSet::testSerialWithoutLastWord);
		TEST_ADD(CUTMiscBitSet::testSerialInvalid);
	}

	// read a bitset from the stream CBitSet::serial() expects: version, number of bits, then the bits in 32 bit words
	void read(NLMISC::CBitSet &bitSet, uint32 numBits, std::vector<uint32> words)
	{
		NLMISC::CMemStream out;
		uint8 version = 0;
		out.serial(version);
		out.serial(numBits);
		out.serialCont(words);

		NLMISC::CMemStream in(true);
		in.fill(out.buffer(), out.length());
		in.serial(bitSet);
	}

	void roundTrip(uint numBits)
	{
		NLMISC::CBitSet written(numBits);
		for (uint i = 0; i < numBits; i += 3)
			written.set(i);
		if (numBits > 0)
			written.set(numBits - 1);

		NLMISC::CMemStream out;
		out.serial(written);

		NLMISC::CMemStream in(true);
		in.fill(out.buffer(), out.length());
		NLMISC::CBitSet read;
		in.serial(read);

		TEST_ASSERT(read == written);
	}

	void testSerialRoundTrip()
	{
		roundTrip(0);
		roundTrip(1);
		roundTrip(32);
		roundTrip(33);
		roundTrip(512);
		roundTrip(1024);
	}

	void testSerialWithoutLastWord()
	{
		// serial() used to write only the full words, such streams must still load
		NLMISC::CBitSet bitSet;
		read(bitSet, 33, std::vector<uint32>(1, 0x80000001));

		TEST_ASSERT(bitSet.size() == 33);
		TEST_ASSERT(bitSet.get(0));
		TEST_ASSERT(!bitSet.get(1));
		TEST_ASSERT(bitSet.get(31));
		TEST_ASSERT(!bitSet.get(32));
	}

	void testSerialInvalid()
	{
		NLMISC::CBitSet bitSet;

		// less words than bits
		TEST_THROWS(read(bitSet, 1024, std::vector<uint32>()), NLMISC::EInvalidDataStream);
		TEST_THROWS(read(bitSet, 64, std::vector<uint32>(1, 0xffffffff)), NLMISC::EInvalidDataStream);
		// more words than bits
		TEST_THROWS(read(bitSet, 32, std::vector<uint32>(3, 0xffffffff)), NLMISC::EInvalidDataStream);
		// more bits than a bitset can hold; with the last one the word count wraps to 0
		TEST_THROWS(read(bitSet, 0xffffffe0, std::vector<uint32>()), NLMISC::EInvalidDataStream);
		TEST_THROWS(read(bitSet, 0xffffffff, std::vector<uint32>()), NLMISC::EInvalidDataStream);
	}
};

#endif
