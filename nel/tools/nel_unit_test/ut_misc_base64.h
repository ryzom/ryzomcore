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

#ifndef UT_MISC_BASE64
#define UT_MISC_BASE64

#include <nel/misc/base64.h>

struct CUTMiscBase64 : public Test::Suite
{
	CUTMiscBase64()
	{
		TEST_ADD(CUTMiscBase64::testEncode);
		TEST_ADD(CUTMiscBase64::testDecode);
		TEST_ADD(CUTMiscBase64::testDecodeNoPadding);
		TEST_ADD(CUTMiscBase64::testDecodeInvalid);
	}

	void testEncode()
	{
		TEST_ASSERT("" == NLMISC::base64::encode(""));

		TEST_ASSERT("AA==" == NLMISC::base64::encode(std::string(1, '\0')));
		TEST_ASSERT("YQ==" == NLMISC::base64::encode("a"));
		TEST_ASSERT("YWI=" == NLMISC::base64::encode("ab"));
		TEST_ASSERT("YWJj" == NLMISC::base64::encode("abc"));

		std::string expect = "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ==";
		std::string encoded = NLMISC::base64::encode("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}");
		TEST_ASSERT(expect == encoded);
	}

	void testDecode()
	{
		TEST_ASSERT("" == NLMISC::base64::decode(""));
		TEST_ASSERT("" == NLMISC::base64::decode("="));
		TEST_ASSERT("" == NLMISC::base64::decode("=="));
		TEST_ASSERT("" == NLMISC::base64::decode("==="));
		TEST_ASSERT("" == NLMISC::base64::decode("===="));

		TEST_ASSERT(std::string(1, '\0') == NLMISC::base64::decode("AA=="));
		TEST_ASSERT("a" == NLMISC::base64::decode("YQ=="));
		TEST_ASSERT("ab" == NLMISC::base64::decode("YWI="));
		TEST_ASSERT("abc" == NLMISC::base64::decode("YWJj"));

		std::string expect = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}";
		std::string decoded = NLMISC::base64::decode("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ==");
		TEST_ASSERT(expect == decoded);
	}

	void testDecodeNoPadding()
	{
		TEST_ASSERT(std::string(1, '\0') == NLMISC::base64::decode("AA"));
		TEST_ASSERT("a" == NLMISC::base64::decode("YQ"));
		TEST_ASSERT("ab" == NLMISC::base64::decode("YWI"));

		std::string expect = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}";
		std::string decoded = NLMISC::base64::decode("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ");
		TEST_ASSERT(expect == decoded);
	}

	void testDecodeInvalid()
	{
		TEST_ASSERT("" == NLMISC::base64::decode("A"));
		TEST_ASSERT("" == NLMISC::base64::decode("A==="));
	}
};

#endif
