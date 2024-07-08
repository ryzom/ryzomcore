#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include <nel/misc/config_file.h>
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
	NLMISC::CApplicationContext context;

	void SetUp() override
	{
		context = NLMISC::CApplicationContext();

		NLMISC::createDebug(NULL);
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
