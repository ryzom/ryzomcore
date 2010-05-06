#ifndef UT_MISC_CONFIG_FILE
#define UT_MISC_CONFIG_FILE

#include <nel/misc/path.h>

#ifndef NEL_UNIT_BASE
#define NEL_UNIT_BASE ""
#endif // NEL_UNIT_BASE

// Test suite for CConfigFile class
class CUTMiscConfigFile : public Test::Suite
{
	string		_WorkingPath;
	string		_OldPath;
public:
	CUTMiscConfigFile ()
	{
		TEST_ADD(CUTMiscConfigFile::configWithInclude);
		TEST_ADD(CUTMiscConfigFile::configWithOptional);
		TEST_ADD(CUTMiscConfigFile::configWithDefine);
		TEST_ADD(CUTMiscConfigFile::configWithBadTest);
		TEST_ADD(CUTMiscConfigFile::configIncludeAndOptional);
		TEST_ADD(CUTMiscConfigFile::reportErrorInSubFiles);
	}

	void setup()
	{
		_OldPath = CPath::getCurrentPath();
		CPath::setCurrentPath(_WorkingPath.c_str());
	}

	void tear_down()
	{
		CPath::setCurrentPath(_OldPath.c_str());
	}
	
	void configWithInclude()
	{
		CConfigFile configFile;

		TEST_THROWS_NOTHING(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_include.cfg"));

		TEST_ASSERT(configFile.loaded());
		TEST_ASSERT(configFile.getVarPtr("CfgWithInclude") != NULL);
		TEST_ASSERT(configFile.getVar("CfgWithInclude").asString(0) == "ok");
		TEST_ASSERT(configFile.getVarPtr("IncludedCfg") != NULL);
		TEST_ASSERT(configFile.getVar("IncludedCfg").asString(0) == "ok");
	}

	void configWithOptional()
	{
		CConfigFile configFile;

		TEST_THROWS_NOTHING(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_optional.cfg"));

		TEST_ASSERT(configFile.loaded());
		TEST_ASSERT(configFile.getVarPtr("CfgWithInclude") != NULL);
		TEST_ASSERT(configFile.getVar("CfgWithInclude").asString(0) == "ok");
		TEST_ASSERT(configFile.getVarPtr("IncludedCfg") != NULL);
		TEST_ASSERT(configFile.getVar("IncludedCfg").asString(0) == "ok");
	}


	void configWithDefine()
	{
		CConfigFile configFile;

		TEST_THROWS_NOTHING(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_define.cfg"));

		TEST_ASSERT(configFile.loaded());
		TEST_ASSERT(configFile.getVarPtr("CfgReadableVar") != NULL);
		TEST_ASSERT(configFile.getVarPtr("CfgNotToBeFound") == NULL);
		TEST_ASSERT(configFile.getVarPtr("CfgInvisible") == NULL);
		TEST_ASSERT(configFile.getVarPtr("CfgMustExist") != NULL);
	}

	class  CMyDisplayer : public IDisplayer
	{
	public:
		vector<string>	Lines;
	
		virtual void doDisplay( const CLog::TDisplayInfo& args, const char *message)
		{
			Lines.push_back(message);
		}
	};

	void configWithBadTest()
	{
		// override the warning channel to get the error on unclosed if
		CMyDisplayer warnings;
		CLog logger;
		logger.addDisplayer(&warnings);
		CNLWarningOverride	override(&logger);

		CConfigFile configFile;

		string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_bad_test.cfg", false);

		TEST_THROWS_NOTHING(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_bad_test.cfg"));

		TEST_ASSERT(configFile.getVarPtr("ASimpleVar") != NULL);

		// check that we have the warnings
		TEST_ASSERT(warnings.Lines.size() == 13);
		TEST_ASSERT(warnings.Lines[0].find(string("Preprocess: In file ")+fullName+"(6) : Error unrecognized preprocessor command") != string::npos);
		TEST_ASSERT(warnings.Lines[1].find(string("Preprocess: In file ")+fullName+"(9) : Error found '#endif' without matching #if") != string::npos);

		// skip the I18N warning from parseMarkedString
		TEST_ASSERT(warnings.Lines[3].find(string("Preprocess: In file ")+fullName+"(12) : Error parsing include file command") != string::npos);
		// skip the I18N warning from parseMarkedString
		TEST_ASSERT(warnings.Lines[5].find(string("Preprocess: In file ")+fullName+"(13) : Error parsing include file command") != string::npos);
		// skip the I18N warning from parseMarkedString
		TEST_ASSERT(warnings.Lines[7].find(string("Preprocess: In file ")+fullName+"(14) : Error parsing optional file command") != string::npos);
		// skip the I18N warning from parseMarkedString
		TEST_ASSERT(warnings.Lines[9].find(string("Preprocess: In file ")+fullName+"(15) : Error parsing optional file command") != string::npos);
		TEST_ASSERT(warnings.Lines[10].find(string("Preprocess: In file ")+fullName+"(16) : Error parsing #define command") != string::npos);
		TEST_ASSERT(warnings.Lines[11].find(string("Preprocess: In file ")+fullName+"(17) : Error parsing #ifdef command") != string::npos);
		
		TEST_ASSERT(warnings.Lines[12].find("Preprocess: Missing 1 closing #endif after parsing") != string::npos);
	}

	void configIncludeAndOptional()
	{
		CMyDisplayer warnings;
		CLog logger;
		logger.addDisplayer(&warnings);
		CNLWarningOverride	override(&logger);

		CConfigFile configFile;

		string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_include_and_optional.cfg", false);


		TEST_THROWS_NOTHING(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_include_and_optional.cfg"));

		// check that we have the warnings only for the 'include' command
		TEST_ASSERT(warnings.Lines.size() == 1);
		TEST_ASSERT(warnings.Lines[0].find(string("Preprocess: In file ")+fullName+"(2) : Cannot include file 'a_missing_file.cfg'") != string::npos);
	}

	void reportErrorInSubFiles()
	{
		CMyDisplayer warnings;
		CLog logger;
		logger.addDisplayer(&warnings);
		CNLWarningOverride	override(&logger);

		CConfigFile configFile;

		string fullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_error_main.cfg", false);
		string subfullName = NLMISC::CPath::getFullPath(NEL_UNIT_BASE "ut_misc_files/cfg_with_error.cfg", false);


		TEST_THROWS(configFile.load(NEL_UNIT_BASE "ut_misc_files/cfg_with_error_main.cfg"), EParseError);

		// check that we have error report with correct filename and line number
		TEST_ASSERT(warnings.Lines.size() == 1);
		// the first error is in the subfile
		TEST_ASSERT(warnings.Lines[0].find(string("CF: Parsing error in file ")+subfullName+" line 18") != string::npos);
	}
};

#endif
