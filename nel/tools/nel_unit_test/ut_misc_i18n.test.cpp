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

#include <string>
#include <vector>

#include <nel/misc/diff_tool.h>
#include <nel/misc/i18n.h>
#include <nel/misc/ucstring.h>

using NLMISC::CI18N;
using std::string;
using std::vector;

namespace {

// What parseMarkedString reads from what makeMarkedString writes for the text
string roundTrip(ucchar openMark, ucchar closeMark, const string &text)
{
	const ucstring marked = CI18N::makeMarkedString(openMark, closeMark, ucstring::makeFromUtf8(text));
	ucstring::const_iterator it(marked.begin()), last(marked.end());
	ucstring result;
	if (!CI18N::parseMarkedString(openMark, closeMark, it, last, result))
		return "<parse error> " + marked.toUtf8();
	if (it != last)
		return "<text after the closing mark> " + marked.toUtf8();
	return result.toUtf8();
}

string removeComments(const string &text)
{
	ucstring str = ucstring::makeFromUtf8(text);
	CI18N::removeCComment(str);
	return str.toUtf8();
}

// Writes the strings as the load proxy of the Ryzom client does, for CI18N to parse them again
class CPreparedStrings : public CI18N::ILoadProxy
{
public:
	CPreparedStrings()
	{
		add("LanguageName", "Test");
	}

	void add(const string &label, const string &text)
	{
		STRING_MANAGER::TStringInfo info;
		info.Identifier = label;
		info.Text = ucstring::makeFromUtf8(text);
		info.HashValue = 0;
		m_Strings.push_back(info);
	}

	virtual void loadStringFile(const string & /* filename */, ucstring &text)
	{
		text = STRING_MANAGER::prepareStringFile(m_Strings, true);
	}

private:
	vector<STRING_MANAGER::TStringInfo> m_Strings;
};

} // namespace

TEST(CUTMiscI18N, markedStringRoundTrip)
{
	static const char *const texts[] = {
		"",
		"plain text",
		"[",
		"]",
		"[]",
		"][",
		"[[x]]",
		"\\",
		"\\\\",
		"\\[",
		"\\]",
		"ends with a backslash\\",
		"C:\\new\\temp\\data",
		"first line\nsecond line",
		"\n",
		"a\tb",
		"\b",
		"a \\n that is not a new line",
		"see http://example.ryzom.dev/ [a] // b /* c */",
		"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\xE3\x80\x8C[\xE3\x80\x8D" // Japanese, with an opening mark
	};
	for (size_t i = 0; i < sizeof(texts) / sizeof(texts[0]); ++i)
	{
		EXPECT_EQ(string(texts[i]), roundTrip('[', ']', texts[i]));
		EXPECT_EQ(string(texts[i]), roundTrip('(', ')', texts[i]));
		// the marks may be the same character
		EXPECT_EQ(string(texts[i]), roundTrip('"', '"', texts[i]));
	}
	EXPECT_EQ("say \"(this)\"", roundTrip('"', '"', "say \"(this)\""));
	EXPECT_EQ("say \"(this)\"", roundTrip('(', ')', "say \"(this)\""));
}

TEST(CUTMiscI18N, markedStringEscapes)
{
	EXPECT_EQ("[plain text]", CI18N::makeMarkedString('[', ']', ucstring("plain text")).toUtf8());
	EXPECT_EQ("[a\\nb\\tc\\dd]", CI18N::makeMarkedString('[', ']', ucstring("a\nb\tc\bd")).toUtf8());
	EXPECT_EQ("[\\[\\]\\\\]", CI18N::makeMarkedString('[', ']', ucstring("[]\\")).toUtf8());
	// other marks need no escape
	EXPECT_EQ("[(\"{}\")]", CI18N::makeMarkedString('[', ']', ucstring("(\"{}\")")).toUtf8());
}

TEST(CUTMiscI18N, removeCComment)
{
	EXPECT_EQ("a\t[text]\n", removeComments("a\t[text]\n"));
	// the end of the line goes with the comment
	EXPECT_EQ("b\t[c // d /* e */ f]", removeComments("// a\nb\t[c // d /* e */ f]// g\n"));
	EXPECT_EQ("a  b", removeComments("a /* [c] */ b"));
	// an escaped closing mark does not end the text
	EXPECT_EQ("a\t[\\] // b]\nc\t[d]\n", removeComments("a\t[\\] // b]\nc\t[d]\n"));
	EXPECT_EQ("a\t[\\\\]", removeComments("a\t[\\\\]// b\n"));
	EXPECT_EQ("a\t[\\[]", removeComments("a\t[\\[]// b\n"));
}

TEST(CUTMiscI18N, removeCCommentAtEnd)
{
	EXPECT_EQ("a", removeComments("a// b"));
	EXPECT_EQ("a", removeComments("a/* b"));
	EXPECT_EQ("a", removeComments("a/* b */"));
	EXPECT_EQ("a/", removeComments("a/"));
	EXPECT_EQ("a\t[b", removeComments("a\t[b"));
	EXPECT_EQ("a\t[b\\", removeComments("a\t[b\\"));
	EXPECT_EQ("", removeComments(""));
}

TEST(CUTMiscI18N, loadThroughProxy)
{
	CPreparedStrings strings;
	strings.add("utI18NKeyRBracket", "]");
	strings.add("utI18NKeyLBracket", "[");
	strings.add("utI18NKeyBackslash", "\\");
	strings.add("utI18NPath", "C:\\new\\temp // data");
	strings.add("utI18NClosed", "[a] // b");
	strings.add("utI18NLines", "first line\nsecond line");
	strings.add("utI18NLast", "the last text");

	CI18N::ILoadProxy *previous = CI18N::getLoadProxy();
	CI18N::setLoadProxy(&strings);
	CI18N::loadFromFilename("ut_i18n.uxt", true);
	CI18N::setLoadProxy(previous);

	EXPECT_EQ("]", CI18N::get("utI18NKeyRBracket"));
	EXPECT_EQ("[", CI18N::get("utI18NKeyLBracket"));
	EXPECT_EQ("\\", CI18N::get("utI18NKeyBackslash"));
	EXPECT_EQ("C:\\new\\temp // data", CI18N::get("utI18NPath"));
	EXPECT_EQ("[a] // b", CI18N::get("utI18NClosed"));
	EXPECT_EQ("first line\nsecond line", CI18N::get("utI18NLines"));
	EXPECT_EQ("the last text", CI18N::get("utI18NLast"));
}
