#include "stdpch.h"
#include "login_patch.h"
#include "client_cfg.h"
#include <locale.h>

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

std::string convert(const ucstring &str)
{
	if (useUtf8)
		return str.toUtf8();

	return str.toString();
}

void printError(const std::string &str)
{
	// display error in red
	printf("\033[22;31mError: %s\033[0m\n", str.c_str());
}

void printProgress(const std::string &str)
{
	// display progress in green
	printf("\033[F\033[22;31mError: %d\033[0m\033[[1J\n", str.c_str());
}

int main(int argc, char *argv[])
{
	// init the Nel context
	CApplicationContext appContext;

	createDebug();

	// disable log display on stdout
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");

	// if client_default.cfg is not in current directory, use application default directory
	if (!CFile::isExists("client_default.cfg"))
	{
		std::string currentPath = CFile::getApplicationDirectory("Ryzom");

		if (!CFile::isExists(currentPath)) CFile::createDirectory(currentPath);

		CPath::setCurrentPath(currentPath);
	}

	std::string lang;

	char *locale = setlocale(LC_CTYPE, NULL);

	if (locale)
		lang = toLower(std::string(locale));
	useUtf8 = (lang.find("utf8") != string::npos || lang.find("utf-8") != string::npos);

	// create or load client.cfg
	ClientCfg.init("client.cfg");

	// check if PatchServer is defined
	if (ClientCfg.PatchServer.empty())
	{
		printError("PatchServer not defined in client_default.cfg or client.cfg");
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

	// load translation
	CI18N::load(ClientCfg.LanguageCode);

	printf("Checking %s files to patch...\n", convert(CI18N::get("TheSagaOfRyzom")).c_str());

	printf("Using language: %s\n", convert(CI18N::get("LanguageName")).c_str());

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
				printf("%s\n", convert(log[i]).c_str());
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
			printProgress(convert(state));

			for(uint i = 0; i < log.size(); ++i)
			{
				printf("%s\n", convert(log[i]).c_str());
			}
		}
	}

	if (!res && !pPM->getLastErrorMessage().empty())
	{
		printError(convert(CI18N::get("uiErrPatching") + " " + pPM->getLastErrorMessage()));
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

