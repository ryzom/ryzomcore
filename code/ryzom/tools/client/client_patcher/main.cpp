#include "stdpch.h"
#include "login_patch.h"
#include "client_cfg.h"
#include <locale.h>

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace NLMISC;
using namespace std;

// stuff which is defined as extern in other .cpp files
void quitCrashReport()
{
}

/// domain server version for patch
string	R2ServerVersion;
/// name of the version (used to alias many version under the same name),
/// the value is used to get the release not if not empty
string	VersionName;

string LoginLogin, LoginPassword;
uint32 LoginShardId = 0xFFFFFFFF;

// stuff which is defined in other .cpp files
extern void tmpFlagRemovedPatchCategories(NLMISC::CConfigFile &cf);

bool useUtf8 = false;
bool useEsc = false;

#ifdef NL_OS_WINDOWS
HANDLE hStdout = NULL;
sint attributes = 0;
#endif

std::string convert(const ucstring &str)
{
	if (useUtf8)
		return str.toUtf8();

	return str.toString();
}

void printError(const std::string &str)
{
	// display error in red if possible
	if (useEsc)
	{
		printf("\033[1;31mError: %s\033[0m\n", str.c_str());
	}
	else
	{
#ifdef NL_OS_WINDOWS
		if (hStdout != INVALID_HANDLE_VALUE && hStdout)
			SetConsoleTextAttribute(hStdout, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif

		printf("Error: %s\n", str.c_str());

#ifdef NL_OS_WINDOWS
		if (hStdout != INVALID_HANDLE_VALUE && hStdout)
			SetConsoleTextAttribute(hStdout, attributes);
#endif
	}
}

void printCheck(const std::string &str)
{
	// display check
	printf("%s\n", str.c_str());
}

void printDownload(const std::string &str)
{
	static char spaces[80];

	uint maxLength = 80;

	// if "COLUMNS" environnement variable is defined, use it
	if (getenv("COLUMNS"))
	{
		NLMISC::fromString(std::string(getenv("COLUMNS")), maxLength);
	}

	// only use 79 columns to not wrap
	--maxLength;

	// temporary modified string
	std::string nstr = str;

	uint length = 0;

	if (useUtf8)
	{
		ucstring ucstr;
		ucstr.fromUtf8(nstr);
		length = (uint)ucstr.length();
		if (length > maxLength)
		{
			ucstr = ucstr.luabind_substr(length - maxLength + 3);
			nstr = std::string("...") + ucstr.toUtf8();
			length = maxLength;
		}
	}
	else
	{
		length = (uint)nstr.length();
		if (length > maxLength)
		{
			nstr = std::string("...") + nstr.substr(length - maxLength + 3);
			length = maxLength;
		}
	}

	// add padding with spaces
	memset(spaces, ' ', maxLength);
	spaces[maxLength - length] = '\0';

	// display download in purple
	if (useEsc)
	{
		printf("\033[1;35m%s%s\033[0m\r", nstr.c_str(), spaces);
	}
	else
	{
#ifdef NL_OS_WINDOWS
		if (hStdout != INVALID_HANDLE_VALUE && hStdout)
			SetConsoleTextAttribute(hStdout, FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
#endif

		printf("%s%s\r", nstr.c_str(), spaces);

#ifdef NL_OS_WINDOWS
		if (hStdout != INVALID_HANDLE_VALUE && hStdout)
			SetConsoleTextAttribute(hStdout, attributes);
#endif
	}

	fflush(stdout);
}

int main(int argc, char *argv[])
{
	// init the Nel context
	CApplicationContext appContext;

	// create logs in temporary directory
	createDebug(CPath::getTemporaryDirectory().c_str(), true, true);

	// disable log display on stdout
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");

	std::string config = "client.cfg";

	// if client.cfg is not in current directory, use client.cfg from user directory
	if (!CFile::isExists(config))
		config = CPath::getApplicationDirectory("Ryzom") + config;

	// if client.cfg is not in current directory, use client_default.cfg
	if (!CFile::isExists(config))
		config = "client_default.cfg";

#ifdef RYZOM_ETC_PREFIX
	// if client_default.cfg is not in current directory, use application default directory
	if (!CFile::isExists(config))
		config = CPath::standardizePath(RYZOM_ETC_PREFIX) + config;
#endif

	if (!CFile::isExists(config))
	{
		printError(config + " not found, aborting patch.");
		return 1;
	}

	// check if console supports utf-8
	std::string lang = toLower(std::string(setlocale(LC_CTYPE, "")));
	useUtf8 = (lang.find("utf8") != string::npos || lang.find("utf-8") != string::npos);
	lang = lang.substr(0, 2);

	// check if console supports colors
	std::string term = toLower(std::string(getenv("TERM") ? getenv("TERM"):""));
	useEsc = (term.find("xterm") != string::npos || term.find("linux") != string::npos);

#ifdef NL_OS_WINDOWS
	// setup Windows console
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hStdout != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;

		if (GetConsoleScreenBufferInfo(hStdout, &consoleScreenBufferInfo))
			attributes = consoleScreenBufferInfo.wAttributes;
	}
#endif

	// load client.cfg or client_default.cfg
	ClientCfg.init(config);

	// check if PatchServer is defined
	if (ClientCfg.PatchServer.empty())
	{
		printError("PatchServer not defined in " + config);
		return 1;
	}

	// set default paths
	std::string dataPath = "./data/";
	std::string rootPath = "./";

	// use custom data path if specified
	if (!ClientCfg.DataPath.empty())
	{
		dataPath = CPath::standardizePath(ClientCfg.DataPath.front());
		string::size_type pos = dataPath.rfind('/', dataPath.length()-2);
		if (pos != string::npos)
			rootPath = dataPath.substr(0, pos+1);
	}

	std::string unpackPath = CPath::standardizePath(rootPath + "unpack");

	// check if user can write in data directory
	if (!CFile::isExists(unpackPath))
	{
		if (!CFile::createDirectoryTree(unpackPath))
		{
			printError("You don't have permission to create " + unpackPath);
			return 1;
		}
	}
	else
	{
		if (!CFile::createEmptyFile(unpackPath + "empty"))
		{
			printError("You don't have write permission in " + unpackPath);
			return 1;
		}

		CFile::deleteFile(unpackPath + "empty");
	}

	// only use PreDataPath for looking paths
	if (!ClientCfg.PreDataPath.empty())
	{
		for(uint i = 0; i < ClientCfg.PreDataPath.size(); ++i)
		{
			CPath::addSearchPath(ClientCfg.PreDataPath[i], true, false);
		}
	}

	// add more search paths if translation is not found
	if (!CPath::exists(lang + ".uxt"))
	{
		CPath::addSearchPath("patcher", true, false);
#ifdef RYZOM_SHARE_PREFIX
		CPath::addSearchPath(RYZOM_SHARE_PREFIX"/patcher", true, false);
#endif
	}

 	// load translation
	CI18N::load(lang);

	printf("Checking %s files to patch...\n", convert(CI18N::get("TheSagaOfRyzom")).c_str());

#ifdef NL_OS_UNIX
	// don't use cfg, exe and dll from Windows version
	CConfigFile::CVar var;
	var.Type = CConfigFile::CVar::T_STRING;
	std::vector<std::string> cats;
	cats.push_back("main_exedll");
	cats.push_back("main_cfg");
	var.setAsString(cats);
	ClientCfg.ConfigFile.insertVar("RemovePatchCategories", var);

	// add categories to remove
	tmpFlagRemovedPatchCategories(ClientCfg.ConfigFile);
#endif

	// initialize patch manager and set the ryzom full path, before it's used
	CPatchManager *pPM = CPatchManager::getInstance();

	// set the correct root path
	pPM->setClientRootPath(rootPath);

	// use PatchServer URL
	vector<string> patchURLs;
	pPM->init(patchURLs, ClientCfg.PatchServer, ClientCfg.PatchVersion);
	pPM->startCheckThread(true /* include background patchs */);

	ucstring state;
	vector<ucstring> log;
	bool res = false;
	bool finished = false;

	while (!finished)
	{
		nlSleep(100);

		finished = pPM->isCheckThreadEnded(res);

		if (pPM->getThreadState(state, log))
		{
			for(uint i = 0; i < log.size(); ++i)
			{
				printCheck(convert(log[i]));
			}
		}
	}

	if (!res && !pPM->getLastErrorMessage().empty())
	{
		printError(convert(CI18N::get("uiErrChecking") + " " + pPM->getLastErrorMessage()));
		return 1;
	}

	CPatchManager::SPatchInfo InfoOnPatch;

	// Check is good now ask the player if he wants to apply the patch
	pPM->getInfoToDisp(InfoOnPatch);

	// Get the list of optional categories to patch
	vector<string> vCategories;

	for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
	{
		// Ok for the moment all optional categories must be patched even if the player
		// does not want it. Because we can't detect that a continent have to be patched ingame.
		vCategories.push_back(InfoOnPatch.OptCat[i].Name);
	}

	// start patch thread
	pPM->startPatchThread(vCategories, true);

	res = false;
	finished = false;

	while (!finished)
	{
		nlSleep(100);

		finished = pPM->isPatchThreadEnded(res);

		if (pPM->getThreadState(state, log))
		{
			printDownload(convert(state));

			for(uint i = 0; i < log.size(); ++i)
			{
				printCheck(convert(log[i]));
			}
		}
	}

	if (!res && !pPM->getLastErrorMessage().empty())
	{
		printError(convert(CI18N::get("uiErrPatchApply") + " " + pPM->getLastErrorMessage()));
		return 1;
	}

	if (CPatchManager::getInstance()->mustLaunchBatFile())
	{
		std::string error;

		try
		{
			// move downloaded files to final location
			pPM->createBatchFile(pPM->getDescFile(), false, false);
			CFile::createEmptyFile("show_eula");

			if (!pPM->getLastErrorMessage().empty())
			{
				error = convert(pPM->getLastErrorMessage());
			}
		}
		catch(const EDiskFullError &)
		{
			error = convert(CI18N::get("uiPatchDiskFull"));;
		}
		catch(const EWriteError &)
		{
			error = convert(CI18N::get("uiPatchWriteError"));;
		}
		catch(const Exception &e)
		{
			error = convert(CI18N::get("uiCheckEndWithErr") + " " + e.what());
		}
		catch(...)
		{
			error = "unknown exception";
		}

		if (!error.empty())
		{
			printError(convert(CI18N::get("uiErrPatchApply")) + " " + error);
			return 1;
		}
	}

/*
	// Start Scanning
	pPM->startScanDataThread();

	// request to stop the thread
	pPM->askForStopScanDataThread();
*/

	return 0;
}

