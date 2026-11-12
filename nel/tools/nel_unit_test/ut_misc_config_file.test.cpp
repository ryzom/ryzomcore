#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include <nel/misc/config_file.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/mem_displayer.h>

#ifndef NEL_UNIT_BASE
#define NEL_UNIT_BASE ""
#endif // NEL_UNIT_BASE

using std::string;

using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::StartsWith;
using ::testing::StrEq;

class CUTMiscConfigFileTest : public testing::Test
{
protected:
	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
	}

	void TearDown() override
	{
	}
};

TEST_F(CUTMiscConfigFileTest, configWithInclude)
{
	NLMISC::CConfigFile configFile;

	ASSERT_NO_THROW({ configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_include.cfg"); });

	ASSERT_THAT(configFile.loaded(), IsTrue());
	EXPECT_THAT(configFile.getVarPtr("CfgWithInclude"), NotNull());
	EXPECT_THAT(configFile.getVar("CfgWithInclude").asString(0), StrEq("ok"));
	EXPECT_THAT(configFile.getVarPtr("IncludedCfg"), NotNull());
	EXPECT_THAT(configFile.getVar("IncludedCfg").asString(0), StrEq("ok"));
}

TEST_F(CUTMiscConfigFileTest, configWithOptional)
{
	NLMISC::CConfigFile configFile;

	ASSERT_NO_THROW({ configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_optional.cfg"); });

	ASSERT_THAT(configFile.loaded(), IsTrue());
	EXPECT_THAT(configFile.getVarPtr("CfgWithInclude"), NotNull());
	EXPECT_THAT(configFile.getVar("CfgWithInclude").asString(0), StrEq("ok"));
	EXPECT_THAT(configFile.getVarPtr("IncludedCfg"), NotNull());
	EXPECT_THAT(configFile.getVar("IncludedCfg").asString(0), StrEq("ok"));
}

TEST_F(CUTMiscConfigFileTest, configWithDefine)
{
	NLMISC::CConfigFile configFile;

	ASSERT_NO_THROW({ configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_define.cfg"); });

	ASSERT_THAT(configFile.loaded(), IsTrue());
	EXPECT_THAT(configFile.getVarPtr("CfgReadableVar"), NotNull());
	EXPECT_THAT(configFile.getVarPtr("CfgNotToBeFound"), IsNull());
	EXPECT_THAT(configFile.getVarPtr("CfgInvisible"), IsNull());
	EXPECT_THAT(configFile.getVarPtr("CfgMustExist"), NotNull());
}

TEST_F(CUTMiscConfigFileTest, configWithBadTest)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	NLMISC::CConfigFile configFile;

	string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_bad_test.cfg", false);

	ASSERT_NO_THROW({ configFile.load(fullName); });

	EXPECT_THAT(configFile.getVarPtr("ASimpleVar"), NotNull());

	auto &warningLogs = warnings.lockStrings();
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(6) : Error unrecognized preprocessor command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(9) : Error found '#endif' without matching #if\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(12) : Error parsing include file command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(13) : Error parsing include file command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(14) : Error parsing optional file command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(15) : Error parsing optional file command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(16) : Error parsing #define command\n")));
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(17) : Error parsing #ifdef command\n")));
	EXPECT_THAT(warningLogs, Contains(StartsWith("Preprocess: Missing 1 closing #endif after parsing")));
}

TEST_F(CUTMiscConfigFileTest, configIncludeAndOptional)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	NLMISC::CConfigFile configFile;

	string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_include_and_optional.cfg", false);

	ASSERT_NO_THROW({ configFile.load(fullName); });

	auto &warningLogs = warnings.lockStrings();
	EXPECT_THAT(warningLogs, Contains(StrEq(string("Preprocess: In file ") + fullName + "(2) : Cannot include file 'a_missing_file.cfg'\n")));
}

TEST_F(CUTMiscConfigFileTest, reportErrorInSubFiles)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	NLMISC::CConfigFile configFile;

	string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_error_main.cfg", false);
	string subfullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_error.cfg", false);

	EXPECT_THROW({ configFile.load(fullName); }, NLMISC::EParseError);

	// check that we have error report with correct filename and line number
	auto &warningLogs = warnings.lockStrings();
	EXPECT_THAT(warningLogs, Contains(StrEq(string("CF: Parsing error in file ") + subfullName + " line 18, look in 'debug_cfg_with_error_main.cfg' for a preprocessed version of the config file\n")));
}

namespace {

// writes a config file in the current directory, where the other tests write
void writeConfigFile(const string &fileName, const string &content)
{
	FILE *fp = NLMISC::nlfopen(fileName, "wb");
	ASSERT_NE(fp, nullptr);
	ASSERT_EQ(fwrite(content.data(), 1, content.size(), fp), content.size());
	fclose(fp);
}

} // namespace

// Strings and names have no maximum length. The parser had a buffer of 1024 characters for them.
TEST_F(CUTMiscConfigFileTest, longTokens)
{
	const string fileName = "__cfg_long_tokens.cfg";

	const string str1023(1023, 'a');
	const string str1024(1024, 'b');
	const string str5000(5000, 'c');
	// longer than the read buffer of the lexer (16384)
	const string str100000(100000, 'd');
	const string name1023 = "N" + string(1022, 'e');
	const string name1024 = "N" + string(1023, 'f');
	const string name5000 = "N" + string(4999, 'g');
	const string half600a(600, 'h');
	const string half600b(600, 'i');
	const string str1020(1020, 'j');

	string content;
	content += "Str1023 = \"" + str1023 + "\";\n";
	content += "Str1024 = \"" + str1024 + "\";\n";
	content += "Str5000 = \"" + str5000 + "\";\n";
	content += "Str100000 = \"" + str100000 + "\";\n";
	content += name1023 + " = 1023;\n";
	content += name1024 + " = 1024;\n";
	content += name5000 + " = \"" + str5000 + "\";\n";
	// concatenations that end above 1024 characters
	content += "CatStr = \"" + half600a + "\" + \"" + half600b + "\";\n";
	content += "CatInt = \"" + str1020 + "\" + 123456;\n";
	content += "CatReal = \"" + str1020 + "\" + 1.5;\n";
	content += "CatThree = \"" + str1023 + "\" + \"" + str1024 + "\" + \"" + str5000 + "\";\n";
	// references to long strings, by short and by long names
	content += "RefStr5000 = Str5000;\n";
	content += "RefName5000 = " + name5000 + ";\n";
	content += "RefCat = Str1023 + Str1024;\n";
	content += "RefInt = " + name1024 + " + 1;\n";
	content += "Array = { \"" + str1024 + "\", \"" + str5000 + "\", Str1023 };\n";
	content += "Array += { \"" + str5000 + "\" };\n";
	writeConfigFile(fileName, content);

	NLMISC::CConfigFile configFile;

	ASSERT_NO_THROW({ configFile.load(fileName); });

	// a second parse of the same file gives the same values
	for (int pass = 0; pass < 2; ++pass)
	{
		SCOPED_TRACE(pass);

		ASSERT_THAT(configFile.getVarPtr("Str1023"), NotNull());
		EXPECT_EQ(configFile.getVar("Str1023").asString(), str1023);
		ASSERT_THAT(configFile.getVarPtr("Str1024"), NotNull());
		EXPECT_EQ(configFile.getVar("Str1024").asString(), str1024);
		ASSERT_THAT(configFile.getVarPtr("Str5000"), NotNull());
		EXPECT_EQ(configFile.getVar("Str5000").asString(), str5000);
		ASSERT_THAT(configFile.getVarPtr("Str100000"), NotNull());
		EXPECT_EQ(configFile.getVar("Str100000").asString(), str100000);

		ASSERT_THAT(configFile.getVarPtr(name1023), NotNull());
		EXPECT_EQ(configFile.getVar(name1023).asInt(), 1023);
		ASSERT_THAT(configFile.getVarPtr(name1024), NotNull());
		EXPECT_EQ(configFile.getVar(name1024).asInt(), 1024);
		ASSERT_THAT(configFile.getVarPtr(name5000), NotNull());
		EXPECT_EQ(configFile.getVar(name5000).asString(), str5000);

		ASSERT_THAT(configFile.getVarPtr("CatStr"), NotNull());
		EXPECT_EQ(configFile.getVar("CatStr").asString(), half600a + half600b);
		ASSERT_THAT(configFile.getVarPtr("CatInt"), NotNull());
		EXPECT_EQ(configFile.getVar("CatInt").asString(), str1020 + "123456");
		ASSERT_THAT(configFile.getVarPtr("CatReal"), NotNull());
		EXPECT_EQ(configFile.getVar("CatReal").asString(), str1020 + "1.500000");
		ASSERT_THAT(configFile.getVarPtr("CatThree"), NotNull());
		EXPECT_EQ(configFile.getVar("CatThree").asString(), str1023 + str1024 + str5000);

		ASSERT_THAT(configFile.getVarPtr("RefStr5000"), NotNull());
		EXPECT_EQ(configFile.getVar("RefStr5000").asString(), str5000);
		ASSERT_THAT(configFile.getVarPtr("RefName5000"), NotNull());
		EXPECT_EQ(configFile.getVar("RefName5000").asString(), str5000);
		ASSERT_THAT(configFile.getVarPtr("RefCat"), NotNull());
		EXPECT_EQ(configFile.getVar("RefCat").asString(), str1023 + str1024);
		ASSERT_THAT(configFile.getVarPtr("RefInt"), NotNull());
		EXPECT_EQ(configFile.getVar("RefInt").asInt(), 1025);

		ASSERT_THAT(configFile.getVarPtr("Array"), NotNull());
		ASSERT_EQ(configFile.getVar("Array").size(), 4u);
		EXPECT_EQ(configFile.getVar("Array").asString(0), str1024);
		EXPECT_EQ(configFile.getVar("Array").asString(1), str5000);
		EXPECT_EQ(configFile.getVar("Array").asString(2), str1023);
		EXPECT_EQ(configFile.getVar("Array").asString(3), str5000);

		if (pass == 0)
		{
			ASSERT_NO_THROW({ configFile.reparse(); });
		}
	}

	NLMISC::CFile::deleteFile(fileName);
}

// A parse error after long tokens is reported with its line, and the next file parses normally
TEST_F(CUTMiscConfigFileTest, parseErrorAfterLongTokens)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	const string badFileName = "__cfg_long_tokens_error.cfg";
	const string goodFileName = "__cfg_long_tokens_good.cfg";
	const string str5000(5000, 'c');

	writeConfigFile(badFileName, "Str5000 = \"" + str5000 + "\";\nCat = Str5000 + \"" + str5000 + "\";\nBroken = = \"" + str5000 + "\";\n");
	writeConfigFile(goodFileName, "Str5000 = \"" + str5000 + "\";\nRef = Str5000;\n");

	const string badFullName = NLMISC::CPath::getFullPath(badFileName, false);

	{
		NLMISC::CConfigFile configFile;
		EXPECT_THROW({ configFile.load(badFullName); }, NLMISC::EParseError);
	}
	{
		auto &warningLogs = warnings.lockStrings();
		EXPECT_THAT(warningLogs, Contains(StrEq(string("CF: Parsing error in file ") + badFullName + " line 3, look in 'debug_" + badFileName + "' for a preprocessed version of the config file\n")));
		warnings.unlockStrings();
	}
	{
		NLMISC::CConfigFile configFile;
		ASSERT_NO_THROW({ configFile.load(goodFileName); });
		ASSERT_THAT(configFile.getVarPtr("Ref"), NotNull());
		EXPECT_EQ(configFile.getVar("Ref").asString(), str5000);
	}

	NLMISC::CFile::deleteFile(badFileName);
	NLMISC::CFile::deleteFile(goodFileName);
	NLMISC::CFile::deleteFile("debug_" + badFileName);
}
