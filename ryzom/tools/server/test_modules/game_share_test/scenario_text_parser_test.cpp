// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "nel/misc/types_nl.h"
#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/scenario_text_parser.h"

#include <gtest/gtest.h>

#include <string>

using namespace R2;

namespace {

std::string serializeScenario(const CObject *scenario)
{
	std::string out = "scenario = ";
	scenario->serialize(out);
	return out;
}

CObject::TSmartPtr parseOk(const std::string &text)
{
	std::string error;
	CObject::TSmartPtr result = CScenarioTextParser::parse(text, nullptr, error);
	EXPECT_TRUE(result != nullptr) << error << "\n"
	                               << text;
	return result;
}

void expectRefused(const std::string &text)
{
	std::string error;
	CObject::TSmartPtr result = CScenarioTextParser::parse(text, nullptr, error);
	EXPECT_TRUE(result == nullptr) << "accepted: " << text;
	EXPECT_FALSE(error.empty());
}

CObject::TSmartPtr buildScenario()
{
	CObjectTable *root = new CObjectTable();
	CObject::TSmartPtr keep(root);
	root->add("Class", std::string("Scenario"));
	root->add("Count", (sint64)42);
	root->add("Negative", (sint64)-7);
	root->add("Ratio", 1.5);
	root->add("Small", 0.125);
	root->add("Brackets", std::string("a[b]c]]d[[e"));
	root->add("Quotes", std::string("say \"hi\" 'there' \\ and\nnewline\ttab"));
	root->add("Empty", std::string(""));
	root->add("Ref", CObject::TSmartPtr(new CObjectRefId("Client1_12")));
	CObjectTable *acts = new CObjectTable();
	root->add("Acts", CObject::TSmartPtr(acts));
	for (sint64 i = 0; i < 3; ++i)
	{
		CObjectTable *act = new CObjectTable();
		act->add("Class", std::string("Act"));
		act->add("Index", i);
		acts->add(CObject::TSmartPtr(act));
	}
	root->add("EmptyTable", CObject::TSmartPtr(new CObjectTable()));
	return keep;
}

} // namespace

TEST(ScenarioTextParser, RoundTripThroughTheWriter)
{
	CObject::TSmartPtr original = buildScenario();
	std::string text = serializeScenario(original);

	CObject::TSmartPtr parsed = parseOk(text);
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ(text, serializeScenario(parsed));

	EXPECT_TRUE(parsed->isTable());
	EXPECT_EQ("Scenario", parsed->toString("Class"));
	EXPECT_TRUE(parsed->isInteger("Count"));
	EXPECT_EQ(42, parsed->toInteger("Count"));
	EXPECT_EQ(-7, parsed->toInteger("Negative"));
	EXPECT_TRUE(parsed->isNumber("Ratio"));
	EXPECT_FALSE(parsed->isInteger("Ratio"));
	EXPECT_DOUBLE_EQ(1.5, parsed->toNumber("Ratio"));
	EXPECT_DOUBLE_EQ(0.125, parsed->toNumber("Small"));
	EXPECT_EQ("a[b]c]]d[[e", parsed->toString("Brackets"));
	EXPECT_EQ("say \"hi\" 'there' \\ and\nnewline\ttab", parsed->toString("Quotes"));
	EXPECT_EQ("", parsed->toString("Empty"));
	EXPECT_TRUE(parsed->getAttr("Ref")->isRefId());
	EXPECT_EQ("Client1_12", parsed->toString("Ref"));
	EXPECT_TRUE(parsed->isTable("Acts"));
	EXPECT_EQ(3u, parsed->getAttr("Acts")->getSize());
	EXPECT_EQ(2, parsed->getAttr("Acts")->getValueAtPos(2)->toInteger("Index"));
	EXPECT_TRUE(parsed->isTable("EmptyTable"));
	EXPECT_EQ(0u, parsed->getAttr("EmptyTable")->getSize());
}

TEST(ScenarioTextParser, HeaderCommentsAndLineEndings)
{
	std::string text = "---- Header\r\n"
	                   "-- Version = '1'\r\n"
	                   "-- Signature = 'abc'\r\n"
	                   "-- HeaderMD5 = '00'\r\n"
	                   "-- BodyMD5 = '11'\r\n"
	                   "-- Name = 'My scenario'\r\n"
	                   "---- /Header\r\n"
	                   "\r\n"
	                   "scenario = {\r\n"
	                   "\tClass = [[Scenario]], -- a line comment\r\n"
	                   "\t--[[ a block\r\n"
	                   "\tcomment ]] Title = [[Hello]],\r\n"
	                   "\tLast = 1,\r\n"
	                   "}\r\n"
	                   "-- trailing comment\r\n";
	CObject::TSmartPtr parsed = parseOk(text);
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ("Scenario", parsed->toString("Class"));
	EXPECT_EQ("Hello", parsed->toString("Title"));
	EXPECT_EQ(1, parsed->toInteger("Last"));
}

TEST(ScenarioTextParser, StringForms)
{
	CObject::TSmartPtr parsed = parseOk(
	    "scenario = {\n"
	    "\tQuoted = \"a\\\"b\\n\\65\\x41\\\\\",\n"
	    "\tSingle = 'it\\'s',\n"
	    "\tJoined = [[one]]..\"[\"..[[two]] .. ']',\n"
	    "\tLeadingNewline = [[\nkept]],\n"
	    "\tRef = r2.RefId(\"Client2_3\"),\n"
	    "\tTrue = true, False = false,\n"
	    "\tExp = 1e3, Neg = -0.5, Padded = 007\n"
	    "}");
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ("a\"b\nAA\\", parsed->toString("Quoted"));
	EXPECT_EQ("it's", parsed->toString("Single"));
	EXPECT_EQ("one[two]", parsed->toString("Joined"));
	EXPECT_EQ("kept", parsed->toString("LeadingNewline"));
	EXPECT_TRUE(parsed->getAttr("Ref")->isRefId());
	EXPECT_EQ("Client2_3", parsed->toString("Ref"));
	EXPECT_EQ(1, parsed->toInteger("True"));
	EXPECT_EQ(0, parsed->toInteger("False"));
	EXPECT_DOUBLE_EQ(1000.0, parsed->toNumber("Exp"));
	EXPECT_DOUBLE_EQ(-0.5, parsed->toNumber("Neg"));
	EXPECT_EQ(7, parsed->toInteger("Padded"));
}

TEST(ScenarioTextParser, SingleQuoteAdjacentStringsAreNotConcatenated)
{
	// 'it''s' is two strings with nothing between them, which is not a value
	expectRefused("scenario = { Single = 'it''s' }");
}

TEST(ScenarioTextParser, TrailingCommaAndEmptyTable)
{
	CObject::TSmartPtr parsed = parseOk("scenario = { A = 1, B = { }, C = { 1, 2, 3, }, }");
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ(3u, parsed->getAttr("C")->getSize());
	EXPECT_EQ(0u, parsed->getAttr("B")->getSize());
}

TEST(ScenarioTextParser, RefusesCode)
{
	expectRefused("scenario = {}\nos.remove('x')");
	expectRefused("scenario = { A = os.remove('x') }");
	expectRefused("scenario = { A = [[x]] .. os.getenv('HOME') }");
	expectRefused("scenario = { A = function() end }");
	expectRefused("scenario = { A = 1 + 2 }");
	expectRefused("scenario = { A = nil }");
	expectRefused("scenario = { [\"A\"] = 1 }");
	expectRefused("scenario = { A = r2.RefId(os.getenv('HOME')) }");
	expectRefused("scenario = { A = r2.Other([[x]]) }");
	expectRefused("scenario = { A = print }");
	expectRefused("print('x') scenario = {}");
	expectRefused("foo = {}");
	expectRefused("scenario = {}; scenario = {}");
}

TEST(ScenarioTextParser, RefusesMalformedText)
{
	expectRefused("");
	expectRefused("scenario");
	expectRefused("scenario = ");
	expectRefused("scenario = {");
	expectRefused("scenario = { A = }");
	expectRefused("scenario = { A = 1 B = 2 }");
	expectRefused("scenario = { , }");
	expectRefused("scenario = { A = [[x }");
	expectRefused("scenario = { A = \"x }");
	expectRefused("scenario = { A = \"x\ny\" }");
	expectRefused("scenario = { A = [=[x]=] }");
	expectRefused("scenario = { A = 1.2.3 }");
	expectRefused("scenario = { A = 12abc }");
	expectRefused("scenario = { A = - }");
	expectRefused("scenario = { A = \"\\q\" }");
	expectRefused("scenario = { A = \"\\999\" }");
	expectRefused("scenario = { A = [[x]] .. }");
	expectRefused("scenario = { A == 1 }");
}

TEST(ScenarioTextParser, DepthIsCapped)
{
	std::string deep = "scenario = ";
	for (uint32 i = 0; i < 100; ++i)
		deep += "{ A = ";
	deep += "1";
	for (uint32 i = 0; i < 100; ++i)
		deep += " }";
	EXPECT_TRUE(parseOk(deep) != nullptr);

	std::string tooDeep = "scenario = ";
	for (uint32 i = 0; i < CScenarioTextParser::MaxDepth + 10; ++i)
		tooDeep += "{ A = ";
	tooDeep += "1";
	for (uint32 i = 0; i < CScenarioTextParser::MaxDepth + 10; ++i)
		tooDeep += " }";
	expectRefused(tooDeep);
}

TEST(ScenarioTextParser, ErrorsCarryAPosition)
{
	std::string error;
	CObject::TSmartPtr result = CScenarioTextParser::parse("scenario = {\n\tA = 1,\n\tB = os.x()\n}", nullptr, error);
	EXPECT_TRUE(result == nullptr);
	EXPECT_NE(std::string::npos, error.find("line 3"));
}

TEST(ScenarioTextParser, NodeCountIsCapped)
{
	std::string many = "scenario = {";
	many.reserve(many.size() + 2 * (CScenarioTextParser::MaxNodes + 1) + 1);
	for (uint32 i = 0; i < CScenarioTextParser::MaxNodes + 1; ++i)
		many += "0,";
	many += "}";
	expectRefused(many);
}

TEST(ScenarioTextParser, DuplicateKeysAreRefused)
{
	expectRefused("scenario = { A = 1, A = 2 }");
	expectRefused("scenario = { A = 1, B = { C = 1, C = 2 } }");
	// bare values have no key and may repeat
	EXPECT_TRUE(parseOk("scenario = { 1, 1, 1 }") != nullptr);
}

TEST(ScenarioTextParser, KeysAttributeIsPlainData)
{
	// the old loader re-sorted a table from a "Keys" child, which the writer no longer produces
	CObject::TSmartPtr parsed = parseOk("scenario = { Keys = { [[A]], [[A]] }, A = 1 }");
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ(1, parsed->toInteger("A"));
	EXPECT_EQ(2u, parsed->getAttr("Keys")->getSize());
	std::string out;
	parsed->serialize(out);
	EXPECT_FALSE(out.empty());
}

TEST(ScenarioTextParser, ByteOrderMarkAndHeaderCarriageReturn)
{
	EXPECT_TRUE(parseOk("\xEF\xBB\xBFscenario = {}") != nullptr);
	// a lone carriage return does not end a comment here; it did in Lua, which ran the rest
	CObject::TSmartPtr parsed = parseOk("-- Name = 'x\rrunCommand(\"quit\")'\nscenario = { A = 1 }");
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_EQ(1u, parsed->getSize());
}

TEST(ScenarioTextParser, IntegerLimits)
{
	CObject::TSmartPtr parsed = parseOk(
	    "scenario = {\n"
	    "\tMin = -9223372036854775808,\n"
	    "\tMax = 9223372036854775807,\n"
	    "\tMid = 922337203685477580,\n"
	    "\tBig = 9999999999999999999,\n"
	    "\tTwo = 2,\n"
	    "}");
	ASSERT_TRUE(parsed != nullptr);
	EXPECT_TRUE(parsed->isInteger("Min"));
	EXPECT_EQ((sint64)(-9223372036854775807LL - 1), parsed->toInteger("Min"));
	EXPECT_TRUE(parsed->isInteger("Max"));
	EXPECT_EQ((sint64)9223372036854775807LL, parsed->toInteger("Max"));
	// integers above 2^53 must not pass through a double on the way out either
	std::string maxText;
	parsed->getAttr("Max")->serialize(maxText);
	EXPECT_EQ("9223372036854775807", maxText);
	EXPECT_EQ((sint64)922337203685477580LL, parsed->toInteger("Mid"));
	EXPECT_FALSE(parsed->isInteger("Big"));
	EXPECT_DOUBLE_EQ(1e19, parsed->toNumber("Big"));
	EXPECT_TRUE(parsed->isInteger("Two"));

	// a double the writer rounds to a whole number comes back as an integer, as it did from Lua
	CObjectTable *root = new CObjectTable();
	CObject::TSmartPtr keep(root);
	root->add("Two", 2.0);
	CObject::TSmartPtr again = parseOk(serializeScenario(root));
	ASSERT_TRUE(again != nullptr);
	EXPECT_TRUE(again->isInteger("Two"));
	EXPECT_DOUBLE_EQ(2.0, again->toNumber("Two"));
}

#ifdef RYZOM_EXAMPLES_DIR
TEST(ScenarioTextParser, ShippedExampleScenarios)
{
	const char *files[] = { "Example_01_-_A_Hearty_Meal.r2", "Plight_Of_Trigio.r2" };
	for (uint i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		std::string path = std::string(RYZOM_EXAMPLES_DIR) + "/" + files[i];
		CScenarioValidator validator;
		CScenarioValidator::TValues values;
		std::string md5, signature;
		ASSERT_TRUE(validator.setScenarioToLoad(path, values, md5, signature, false)) << path;
		std::string filename, body;
		validator.applyLoad(filename, body, values);
		ASSERT_FALSE(body.empty()) << path;

		CObject::TSmartPtr parsed = parseOk(body);
		ASSERT_TRUE(parsed != nullptr) << path;
		EXPECT_EQ("Scenario", parsed->toString("Class")) << path;
		EXPECT_FALSE(parsed->toString("InstanceId").empty()) << path;
		EXPECT_TRUE(parsed->isTable("Acts")) << path;
		EXPECT_LT(0u, parsed->getAttr("Acts")->getSize()) << path;

		// writing it back and reading it again is stable
		std::string text = serializeScenario(parsed);
		CObject::TSmartPtr again = parseOk(text);
		ASSERT_TRUE(again != nullptr) << path;
		EXPECT_EQ(text, serializeScenario(again)) << path;
	}
}
#endif
