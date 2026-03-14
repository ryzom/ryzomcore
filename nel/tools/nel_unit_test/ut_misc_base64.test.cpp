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

#include <gtest/gtest.h>

#include <nel/misc/base64.h>

class CUTMiscBase64 : public testing::Test
{
};

TEST_F(CUTMiscBase64, testEncode)
{
	ASSERT_TRUE("" == NLMISC::base64::encode(""));

	ASSERT_TRUE("AA==" == NLMISC::base64::encode(std::string(1, '\0')));
	ASSERT_TRUE("YQ==" == NLMISC::base64::encode("a"));
	ASSERT_TRUE("YWI=" == NLMISC::base64::encode("ab"));
	ASSERT_TRUE("YWJj" == NLMISC::base64::encode("abc"));

	std::string expect = "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ==";
	std::string encoded = NLMISC::base64::encode("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}");
	ASSERT_TRUE(expect == encoded);
}

TEST_F(CUTMiscBase64, testDecode)
{
	ASSERT_TRUE("" == NLMISC::base64::decode(""));
	ASSERT_TRUE("" == NLMISC::base64::decode("="));
	ASSERT_TRUE("" == NLMISC::base64::decode("=="));
	ASSERT_TRUE("" == NLMISC::base64::decode("==="));
	ASSERT_TRUE("" == NLMISC::base64::decode("===="));

	ASSERT_TRUE(std::string(1, '\0') == NLMISC::base64::decode("AA=="));
	ASSERT_TRUE("a" == NLMISC::base64::decode("YQ=="));
	ASSERT_TRUE("ab" == NLMISC::base64::decode("YWI="));
	ASSERT_TRUE("abc" == NLMISC::base64::decode("YWJj"));

	std::string expect = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}";
	std::string decoded = NLMISC::base64::decode("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ==");
	ASSERT_TRUE(expect == decoded);
}

TEST_F(CUTMiscBase64, testDecodeNoPadding)
{
	ASSERT_TRUE(std::string(1, '\0') == NLMISC::base64::decode("AA"));
	ASSERT_TRUE("a" == NLMISC::base64::decode("YQ"));
	ASSERT_TRUE("ab" == NLMISC::base64::decode("YWI"));

	std::string expect = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#0^&*();:<>,. []{}";
	std::string decoded = NLMISC::base64::decode("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWjAxMjM0NTY3ODkhQCMwXiYqKCk7Ojw+LC4gW117fQ");
	ASSERT_TRUE(expect == decoded);
}

TEST_F(CUTMiscBase64, testDecodeInvalid)
{
	ASSERT_TRUE("" == NLMISC::base64::decode("A"));
	ASSERT_TRUE("" == NLMISC::base64::decode("A==="));
}
