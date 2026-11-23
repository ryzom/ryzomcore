// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <clocale>
#include <cmath>
#include <limits>
#include <string>

#include <nel/misc/json.h>
#include <nel/misc/string_common.h>

using NLMISC::CJsonValue;

namespace {

/// The value of a text that must parse
CJsonValue parsed(const std::string &text)
{
	CJsonValue v;
	std::string error;
	EXPECT_TRUE(v.parse(text, error)) << text << ": " << error;
	return v;
}

/// The error of a text that must not parse
std::string refused(const std::string &text)
{
	CJsonValue v(true);
	std::string error;
	EXPECT_FALSE(v.parse(text, error)) << text;
	EXPECT_TRUE(v.isNull()) << "a failed parse leaves null: " << text;
	EXPECT_FALSE(error.empty()) << text;
	return error;
}

std::string nested(uint depth, const std::string &inner)
{
	return std::string(depth, '[') + inner + std::string(depth, ']');
}

} // anonymous namespace

class CUTMiscJson : public testing::Test
{
};

// *** values ***

TEST_F(CUTMiscJson, types)
{
	EXPECT_TRUE(CJsonValue().isNull());
	EXPECT_TRUE(CJsonValue(true).isBool());
	EXPECT_TRUE(CJsonValue((sint32)-3).isNumber());
	EXPECT_TRUE(CJsonValue((uint32)3).isNumber());
	EXPECT_TRUE(CJsonValue((sint64)-3).isNumber());
	EXPECT_TRUE(CJsonValue((uint64)3).isNumber());
	EXPECT_TRUE(CJsonValue(0.5).isNumber());
	EXPECT_TRUE(CJsonValue("a").isString());
	EXPECT_TRUE(CJsonValue(std::string("a")).isString());
	EXPECT_TRUE(CJsonValue::array().isArray());
	EXPECT_TRUE(CJsonValue::object().isObject());
	EXPECT_TRUE(CJsonValue::null().isNull());
	EXPECT_EQ(CJsonValue((const char *)nullptr).getString(), "");
}

TEST_F(CUTMiscJson, gettersTakeTheDefaultForAnotherType)
{
	CJsonValue s("x");
	EXPECT_EQ(s.getBool(true), true);
	EXPECT_EQ(s.getNumber(2.5), 2.5);
	EXPECT_EQ(s.getInt(7), 7);
	EXPECT_EQ(CJsonValue(1.0).getString(), "");
	EXPECT_EQ(CJsonValue(1.0).getString("d"), "d");
	EXPECT_EQ(s.getString("d"), "x");
	EXPECT_EQ(s.size(), 0u);
}

TEST_F(CUTMiscJson, getIntRoundsTowardZeroAndSaturates)
{
	EXPECT_EQ(CJsonValue(2.9).getInt(), 2);
	EXPECT_EQ(CJsonValue(-2.9).getInt(), -2);
	EXPECT_EQ(CJsonValue(1e300).getInt(), std::numeric_limits<sint64>::max());
	EXPECT_EQ(CJsonValue(-1e300).getInt(), std::numeric_limits<sint64>::min());
	EXPECT_EQ(CJsonValue(std::numeric_limits<double>::quiet_NaN()).getInt(5), 5);
	// exact up to 2^53
	EXPECT_EQ(CJsonValue((sint64)9007199254740992LL).getInt(), 9007199254740992LL);
}

TEST_F(CUTMiscJson, arrays)
{
	CJsonValue a;
	a.push(1);
	EXPECT_TRUE(a.isArray()) << "a push turns null into an array";
	a.push("two");
	a.push(CJsonValue::object().set("k", 3));
	EXPECT_EQ(a.size(), 3u);
	EXPECT_EQ(a.at(1).getString(), "two");
	EXPECT_EQ(a.at(2).get("k").getInt(), 3);
	EXPECT_TRUE(a.at(3).isNull());
	EXPECT_TRUE(CJsonValue(1).at(0).isNull());
	EXPECT_EQ(a.items().size(), 3u);
	// an item of the array itself
	a.push(a.at(1));
	EXPECT_EQ(a.at(3).getString(), "two");
}

TEST_F(CUTMiscJson, objectsKeepTheOrderTheyWereBuiltIn)
{
	CJsonValue o;
	o.set("b", 1).set("a", 2).set("c", 3);
	EXPECT_TRUE(o.isObject()) << "a set turns null into an object";
	EXPECT_EQ(o.toString(), "{\"b\":1,\"a\":2,\"c\":3}");
	// a key set again keeps its place
	o.set("a", 20);
	EXPECT_EQ(o.toString(), "{\"b\":1,\"a\":20,\"c\":3}");
	EXPECT_TRUE(o.has("c"));
	EXPECT_FALSE(o.has("d"));
	EXPECT_TRUE(o.get("d").isNull());
	EXPECT_TRUE(CJsonValue(1).get("a").isNull());
	EXPECT_FALSE(CJsonValue(1).has("a"));
	o.member("d") = "x";
	EXPECT_EQ(o.get("d").getString(), "x");
	EXPECT_TRUE(o.erase("b"));
	EXPECT_FALSE(o.erase("b"));
	EXPECT_EQ(o.toString(), "{\"a\":20,\"c\":3,\"d\":\"x\"}");
	EXPECT_EQ(o.members().size(), 3u);
}

TEST_F(CUTMiscJson, setTakesAPartOfTheSameObject)
{
	// adding a member moves the others; the value is copied before
	CJsonValue o = CJsonValue::object();
	o.set("a", CJsonValue::array());
	o.member("a").push("deep");
	for (uint i = 0; i < 40; ++i)
		o.set(NLMISC::toString("k%u", i), o.get("a"));
	EXPECT_EQ(o.get("k39").at(0).getString(), "deep");
	o = o.get("k3");
	EXPECT_TRUE(o.isArray());
	EXPECT_EQ(o.at(0).getString(), "deep");
}

TEST_F(CUTMiscJson, largeObjects)
{
	// past the size from which an object keeps an index of its keys
	CJsonValue o = CJsonValue::object();
	const uint count = 2000;
	for (uint i = 0; i < count; ++i)
		o.set(NLMISC::toString("key%u", i), i);
	EXPECT_EQ(o.size(), count);
	for (uint i = 0; i < count; ++i)
		ASSERT_EQ(o.get(NLMISC::toString("key%u", i)).getInt(), (sint64)i);
	EXPECT_FALSE(o.has("key2000"));

	// a key set again keeps its place
	o.set("key5", "five");
	EXPECT_EQ(o.members()[5].first, "key5");
	EXPECT_EQ(o.members()[5].second.getString(), "five");
	EXPECT_EQ(o.size(), count);

	// an erase moves the members after it down
	EXPECT_TRUE(o.erase("key0"));
	EXPECT_TRUE(o.erase("key1000"));
	EXPECT_FALSE(o.has("key0"));
	EXPECT_EQ(o.size(), count - 2);
	EXPECT_EQ(o.get("key1").getInt(), 1);
	EXPECT_EQ(o.get("key1999").getInt(), 1999);
	EXPECT_EQ(o.members()[0].first, "key1");
	o.set("key0", 0);
	EXPECT_EQ(o.members().back().first, "key0");
	EXPECT_EQ(o.get("key0").getInt(), 0);
	EXPECT_EQ(o.get("key1001").getInt(), 1001);

	// copies are separate
	CJsonValue copy = o;
	copy.set("key1", "changed");
	copy.erase("key2");
	EXPECT_EQ(o.get("key1").getInt(), 1);
	EXPECT_EQ(o.get("key2").getInt(), 2);
	EXPECT_EQ(copy.get("key1").getString(), "changed");
	EXPECT_FALSE(copy.has("key2"));
	EXPECT_EQ(copy.get("key3").getInt(), 3);
	CJsonValue assigned;
	assigned = copy;
	EXPECT_EQ(assigned.get("key1").getString(), "changed");
	EXPECT_EQ(assigned.get("key1999").getInt(), 1999);

	// and survive the round trip
	CJsonValue back = parsed(o.toString());
	EXPECT_EQ(back.toString(), o.toString());
	EXPECT_EQ(back.get("key1500").getInt(), 1500);
}

TEST_F(CUTMiscJson, swapAndMove)
{
	CJsonValue a = CJsonValue::object().set("x", 1);
	CJsonValue b = CJsonValue::array();
	b.push(2);
	a.swap(b);
	EXPECT_TRUE(a.isArray());
	EXPECT_EQ(a.at(0).getInt(), 2);
	EXPECT_EQ(b.get("x").getInt(), 1);
	CJsonValue c = std::move(b);
	EXPECT_EQ(c.get("x").getInt(), 1);
	CJsonValue d;
	d = std::move(c);
	EXPECT_EQ(d.get("x").getInt(), 1);
}

// *** writing ***

TEST_F(CUTMiscJson, writeNumbers)
{
	EXPECT_EQ(CJsonValue(0).toString(), "0");
	EXPECT_EQ(CJsonValue(-12).toString(), "-12");
	EXPECT_EQ(CJsonValue((uint32)4294967295u).toString(), "4294967295");
	EXPECT_EQ(CJsonValue(0.5).toString(), "0.5");
	EXPECT_EQ(CJsonValue(0.1).toString(), "0.1");
	EXPECT_EQ(CJsonValue(1e300).toString(), "1e+300");
	// the shortest text that reads back the same
	double third = 1.0 / 3.0;
	EXPECT_EQ(parsed(CJsonValue(third).toString()).getNumber(), third);
	// JSON has no NaN or infinity
	EXPECT_EQ(CJsonValue(std::numeric_limits<double>::quiet_NaN()).toString(), "null");
	EXPECT_EQ(CJsonValue(std::numeric_limits<double>::infinity()).toString(), "null");
	EXPECT_EQ(CJsonValue(-std::numeric_limits<double>::infinity()).toString(), "null");
}

TEST_F(CUTMiscJson, writeStrings)
{
	EXPECT_EQ(CJsonValue("a\"b\\c").toString(), "\"a\\\"b\\\\c\"");
	EXPECT_EQ(CJsonValue("\n\r\t\b\f").toString(), "\"\\n\\r\\t\\b\\f\"");
	EXPECT_EQ(CJsonValue(std::string("\x01\x1f\x7f", 3)).toString(), "\"\\u0001\\u001f\\u007f\"");
	EXPECT_EQ(CJsonValue(std::string("a\0b", 3)).toString(), "\"a\\u0000b\"");
	// valid UTF-8 as it is
	EXPECT_EQ(CJsonValue("\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80").toString(), "\"\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\"");
	// one U+FFFD per byte that is not UTF-8: a stray continuation, a cut sequence, an overlong form, a surrogate
	EXPECT_EQ(CJsonValue("\x80").toString(), "\"\xEF\xBF\xBD\"");
	EXPECT_EQ(CJsonValue("a\xE2\x82").toString(), "\"a\xEF\xBF\xBD\xEF\xBF\xBD\"");
	EXPECT_EQ(CJsonValue("\xC0\xAF").toString(), "\"\xEF\xBF\xBD\xEF\xBF\xBD\"");
	EXPECT_EQ(CJsonValue("\xED\xA0\x80").toString(), "\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"");
	EXPECT_EQ(CJsonValue("\xF4\x90\x80\x80").toString(), "\"\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\"");
	// a key is a string too
	EXPECT_EQ(CJsonValue::object().set("k\"", 1).toString(), "{\"k\\\"\":1}");
	std::string out;
	CJsonValue::writeString(out, "x");
	EXPECT_EQ(out, "\"x\"");
}

TEST_F(CUTMiscJson, writeStructures)
{
	CJsonValue v = CJsonValue::object();
	v.set("n", CJsonValue());
	v.set("t", true);
	v.set("f", false);
	CJsonValue a = CJsonValue::array();
	a.push(1);
	a.push(CJsonValue::array());
	v.set("a", a);
	v.set("o", CJsonValue::object());
	EXPECT_EQ(v.toString(), "{\"n\":null,\"t\":true,\"f\":false,\"a\":[1,[]],\"o\":{}}");
	std::string out = "x";
	v.write(out);
	EXPECT_EQ(out, "x" + v.toString());
}

// *** parsing ***

TEST_F(CUTMiscJson, parseLiteralsAndSpace)
{
	EXPECT_TRUE(parsed("null").isNull());
	EXPECT_EQ(parsed("true").getBool(), true);
	EXPECT_EQ(parsed(" \t\r\nfalse \n").getBool(true), false);
	refused("nul");
	refused("True");
	refused("");
	refused("   ");
	refused("1 2");
	refused("{} x");
}

TEST_F(CUTMiscJson, parseNumbers)
{
	EXPECT_EQ(parsed("0").getNumber(), 0.0);
	EXPECT_EQ(parsed("-0").getNumber(), 0.0);
	EXPECT_EQ(parsed("12").getInt(), 12);
	EXPECT_EQ(parsed("-12.5").getNumber(), -12.5);
	EXPECT_EQ(parsed("1e3").getNumber(), 1000.0);
	EXPECT_EQ(parsed("1E+3").getNumber(), 1000.0);
	EXPECT_EQ(parsed("25e-1").getNumber(), 2.5);
	EXPECT_EQ(parsed("1.7976931348623157e308").getNumber(), std::numeric_limits<double>::max());
	refused("01");
	refused("1.");
	refused(".5");
	refused("-");
	refused("+1");
	refused("1e");
	refused("1e+");
	refused("0x10");
	refused("NaN");
	refused("Infinity");
}

TEST_F(CUTMiscJson, parseRefusesNumbersThatOverflow)
{
	// an infinity would reach the code as a number
	std::string error = refused("1e999");
	EXPECT_NE(error.find("range"), std::string::npos) << error;
	EXPECT_NE(error.find("offset 0"), std::string::npos) << error;
	refused("-1e999");
	refused("1.8e308");
	refused("{\"offset\": 1e400}");
	refused("[1, 2, -1e309]");
	// what underflows is a number
	EXPECT_EQ(parsed("1e-999").getNumber(), 0.0);
	EXPECT_TRUE(parsed("4.9e-324").isNumber());
}

TEST_F(CUTMiscJson, parseStrings)
{
	EXPECT_EQ(parsed("\"\"").getString(), "");
	EXPECT_EQ(parsed("\"abc\"").getString(), "abc");
	EXPECT_EQ(parsed("\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"").getString(), "\" \\ / \b \f \n \r \t");
	EXPECT_EQ(parsed("\"\\u0041\\u00e9\\u20AC\"").getString(), "A\xC3\xA9\xE2\x82\xAC");
	EXPECT_EQ(parsed("\"\\u0000\"").getString(), std::string(1, '\0'));
	// a surrogate pair is one character of four bytes
	EXPECT_EQ(parsed("\"\\ud83d\\ude00\"").getString(), "\xF0\x9F\x98\x80");
	// raw UTF-8 passes
	EXPECT_EQ(parsed("\"\xC3\xA9\"").getString(), "\xC3\xA9");
	refused("\"abc");
	refused("\"a\\\"");
	refused("\"\\x\"");
	refused("\"\\u12\"");
	refused("\"\\u12g4\"");
	refused("\"a\nb\"");
	refused(std::string("\"a\x01\"", 4));
	refused("'a'");
}

TEST_F(CUTMiscJson, parseReadsALoneSurrogateAsReplacement)
{
	// a message with one is still read, so its request can be answered
	const std::string fffd = "\xEF\xBF\xBD";
	EXPECT_EQ(parsed("\"\\ud83d\"").getString(), fffd);
	EXPECT_EQ(parsed("\"\\ude00\"").getString(), fffd);
	EXPECT_EQ(parsed("\"a\\ud83db\"").getString(), "a" + fffd + "b");
	// a high half followed by another escape: the other escape is read as itself
	EXPECT_EQ(parsed("\"\\ud83d\\u0041\"").getString(), fffd + "A");
	EXPECT_EQ(parsed("\"\\ud83d\\ud83d\\ude00\"").getString(), fffd + "\xF0\x9F\x98\x80");
	EXPECT_EQ(parsed("\"\\ud83d\\n\"").getString(), fffd + "\n");
	// what is no escape stays an error
	refused("\"\\ud83d\\u00zz\"");
	// written back it is valid UTF-8
	EXPECT_EQ(parsed("\"\\udc00\"").toString(), "\"" + fffd + "\"");
	CJsonValue rpc = parsed("{\"jsonrpc\":\"2.0\",\"id\":7,\"params\":{\"text\":\"\\ud800\"}}");
	EXPECT_EQ(rpc.get("id").getInt(), 7);
}

TEST_F(CUTMiscJson, parseStructures)
{
	CJsonValue v = parsed(" { \"a\" : [ 1 , { \"b\" : null } , [ ] ] , \"c\" : { } , \"d\" : \"e\" } ");
	EXPECT_EQ(v.toString(), "{\"a\":[1,{\"b\":null},[]],\"c\":{},\"d\":\"e\"}");
	EXPECT_TRUE(v.get("a").at(1).has("b"));
	EXPECT_TRUE(parsed("[]").isArray());
	EXPECT_TRUE(parsed("{}").isObject());
	refused("[1,]");
	refused("[1 2]");
	refused("[,1]");
	refused("{\"a\":1,}");
	refused("{\"a\" 1}");
	refused("{a:1}");
	refused("{\"a\":}");
	refused("{1:1}");
	refused("[");
	refused("{");
	refused("{\"a\":1");
	refused("]");
}

TEST_F(CUTMiscJson, parseRepeatedKeysKeepTheLastValueInTheFirstPlace)
{
	EXPECT_EQ(parsed("{\"a\":1,\"b\":2,\"a\":{\"x\":3}}").toString(), "{\"a\":{\"x\":3},\"b\":2}");
	// in a large object too
	std::string text = "{";
	for (uint i = 0; i < 100; ++i)
		text += NLMISC::toString("\"k%u\":%u,", i, i);
	text += "\"k10\":\"again\"}";
	CJsonValue v = parsed(text);
	EXPECT_EQ(v.size(), 100u);
	EXPECT_EQ(v.members()[10].second.getString(), "again");
	EXPECT_EQ(v.get("k10").getString(), "again");
	EXPECT_EQ(v.get("k99").getInt(), 99);
}

TEST_F(CUTMiscJson, parseDepthIsBounded)
{
	// 256 levels below the top are read, the next is refused, so a hostile text cannot exhaust the stack
	EXPECT_TRUE(parsed(nested(256, "0")).isArray());
	std::string error = refused(nested(257, "0"));
	EXPECT_NE(error.find("deep"), std::string::npos) << error;
	refused(nested(100000, ""));
	std::string objects;
	for (uint i = 0; i < 300; ++i)
		objects += "{\"a\":";
	objects += "1";
	objects += std::string(300, '}');
	refused(objects);
}

TEST_F(CUTMiscJson, parseReplacesTheValue)
{
	CJsonValue v = CJsonValue::object().set("old", 1);
	std::string error;
	EXPECT_TRUE(v.parse("[1]", error));
	EXPECT_TRUE(v.isArray());
	EXPECT_EQ(v.size(), 1u);
	// the text may be a part of the value itself
	v = CJsonValue("{\"x\":2}");
	EXPECT_TRUE(v.parse(v.getString(), error));
	EXPECT_EQ(v.get("x").getInt(), 2);
	// a failure leaves null and says where
	EXPECT_FALSE(v.parse("[1,", error));
	EXPECT_TRUE(v.isNull());
	EXPECT_NE(error.find("offset 3"), std::string::npos) << error;
}

TEST_F(CUTMiscJson, roundTrip)
{
	std::string text = "{\"s\":\"t\\u00e9xt \\\"q\\\"\\n\",\"n\":[0,-1,2.5,1e+300,123456789012345],\"b\":[true,false,null],\"o\":{\"deep\":{\"er\":[[]]}}}";
	CJsonValue v = parsed(text);
	CJsonValue again = parsed(v.toString());
	EXPECT_EQ(again.toString(), v.toString());
	EXPECT_EQ(v.get("s").getString(), "t\xC3\xA9xt \"q\"\n");
	EXPECT_EQ(v.get("n").at(4).getInt(), 123456789012345LL);
}

TEST_F(CUTMiscJson, numbersIgnoreTheDecimalCommaOfTheLocale)
{
	const char *saved = setlocale(LC_NUMERIC, nullptr);
	std::string previous = saved != nullptr ? saved : "C";
	const char *names[] = { "de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8", "German_Germany.1252", "de-DE" };
	bool set = false;
	for (uint i = 0; i < sizeof(names) / sizeof(names[0]) && !set; ++i)
		set = setlocale(LC_NUMERIC, names[i]) != nullptr;
	if (!set || localeconv()->decimal_point[0] != ',')
	{
		setlocale(LC_NUMERIC, previous.c_str());
		GTEST_SKIP() << "no locale with a decimal comma on this system";
	}
	std::string written = CJsonValue(2.5).toString();
	double read = parsed("2.5").getNumber();
	setlocale(LC_NUMERIC, previous.c_str());
	EXPECT_EQ(written, "2.5");
	EXPECT_EQ(read, 2.5);
}
