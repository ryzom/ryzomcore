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

#ifndef UT_MISC_STREAM
#define UT_MISC_STREAM

#include <nel/misc/stream.h>
#include <nel/misc/bit_mem_stream.h>

// The following line is known to crash in a Ryzom service
NLMISC::CBitMemStream globalBms( false, 2048 ); // global to avoid reallocation

// Test suite for stream based classes
// ! not complete at all at time of writing !
class CUTMiscStream: public Test::Suite
{
public:
	CUTMiscStream ()
	{
		TEST_ADD(CUTMiscStream::constAndStream);
		TEST_ADD(CUTMiscStream::memStreamSwap);
		TEST_ADD(CUTMiscStream::copyOnWrite);
		TEST_ADD(CUTMiscStream::preallocatedBitStream);
	}

	void preallocatedBitStream()
	{
		NLMISC::CBitMemStream localBms( false, 2048 ); // global to avoid reallocation
	}


	void copyOnWrite()
	{
		// test the copy on write strategy in the mem stream (and derived) class.
		// The point is to be able to copy a mem stream (e.g a NLNET::CMessage) 
		// but to do not copy the stream buffer.
		// If more than one stream use the same buffer, any attempt to 
		// modifye the buffer content while lead to a buffer duplication

		NLMISC::CMemStream s1;
		NLMISC::CMemStream s2;
		NLMISC::CMemStream s3;


		uint32 i = 1;
		s1.serial(i);

		s2 = s1;
		s3 = s2;

		TEST_ASSERT(s1.buffer() == s2.buffer());
		TEST_ASSERT(s1.buffer() == s3.buffer());

		// change s1
		s1.serial(i);
		TEST_ASSERT(s1.buffer() != s2.buffer());
		TEST_ASSERT(s2.buffer() == s3.buffer());

		s2.invert();
		s3 = s2;

		TEST_ASSERT(s2.buffer() == s3.buffer());

		s2.serial(i);

		TEST_ASSERT(s2.buffer() == s3.buffer());


	}

	enum TEnum
	{
		e_a,
		e_b,
		e_c,
		e_d
	};

	void constAndStream()
	{
		// check that we can serialize with const stream or const object


		NLMISC::CMemStream s1;
		NLMISC::IStream &is1 = s1;

		const string str("toto");
		const uint32 i(1234546);
		const TEnum	e(e_a);
		string str2("titi");
		uint32 i2(123456);
		TEnum	e2(e_b);

		// no need for const cast any more
		nlWriteSerial(s1, str);
		nlWriteSerial(s1, i);
		nlWrite(s1, serialEnum, e);
		nlWriteSerial(is1, str);
		nlWriteSerial(is1, i);
		nlWrite(is1, serialEnum, i);
		// this work as well
		s1.serial(str2);
		s1.serial(i2);
		s1.serialEnum(e2);

		is1.serial(str2);
		is1.serial(i2);
		is1.serialEnum(e2);

		const NLMISC::CMemStream &s2 = s1;
		const NLMISC::IStream &is2 = s2;

		string str3;
		uint32 i3;
		TEnum e3(e_c);
		// cant write in a const stream
		TEST_THROWS(nlReadSerial(s2, str3), NLMISC::ENotInputStream);
		TEST_THROWS(nlReadSerial(s2, i3), NLMISC::ENotInputStream);
		TEST_THROWS(nlRead(s2, serialEnum, e3), NLMISC::ENotInputStream);
		TEST_THROWS(nlReadSerial(is2, str3), NLMISC::ENotInputStream);
		TEST_THROWS(nlReadSerial(is2, i3), NLMISC::ENotInputStream);
		TEST_THROWS(nlRead(is2, serialEnum, e3), NLMISC::ENotInputStream);


		s1.invert();

		nlReadSerial(s2, str3);
		nlReadSerial(s2, i3);
		nlRead(s2, serialEnum, e3);
		nlReadSerial(is2, str3);
		nlReadSerial(is2, i3);
		nlRead(is2, serialEnum, e3);


		// cant read a const value
		TEST_THROWS(nlWriteSerial(s1, str), NLMISC::ENotOutputStream);
		TEST_THROWS(nlWriteSerial(s1, i), NLMISC::ENotOutputStream);
		TEST_THROWS(nlWrite(s1, serialEnum, e), NLMISC::ENotOutputStream);
		TEST_THROWS(nlWriteSerial(is1, str), NLMISC::ENotOutputStream);
		TEST_THROWS(nlWriteSerial(is1, i), NLMISC::ENotOutputStream);
		TEST_THROWS(nlWrite(is1, serialEnum, e), NLMISC::ENotOutputStream);

	}
	
	void memStreamSwap()
	{
		NLMISC::CMemStream ms2;
			
		string s;
		{
			NLMISC::CMemStream ms1;

			s = "foo1";
			ms1.serial(s);
			s = "foo2";
			ms1.serial(s);
			s = "";

			ms2.swap(ms1);

			// check that ms1 is empty now
			TEST_ASSERT(ms1.length() == 0);
		}

		TEST_ASSERT(!ms2.isReading());
		ms2.invert();
		ms2.serial(s);
		TEST_ASSERT(s == "foo1");
		ms2.serial(s);
		TEST_ASSERT(s == "foo2");
	}
};

#endif
