#include "stdpch.h"
#include "login_patch.h"
#include "client_cfg.h"

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

int main(int argc, char *argv[])
{
	// init the Nel context
	CApplicationContext *appContext = new CApplicationContext;

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

	ClientCfg.init("client.cfg");

	std::string dataPath = "./data/";
	std::string rootPath = "./";
	
	if (!ClientCfg.DataPath.empty())
	{
		dataPath = CPath::standardizePath(ClientCfg.DataPath[0]);
		string::size_type pos = dataPath.rfind('/', dataPath.length()-2);
		if (pos != string::npos)
			rootPath = dataPath.substr(0, pos+1);
	}

	// add .bnp containing translations
	CPath::addSearchBigFile(dataPath + "gamedev.bnp", true, false);

	// load translation
	CI18N::load(ClientCfg.LanguageCode);

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

	pPM->setClientRootPath(rootPath);

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
				printf("%s\n", log[i].toUtf8().c_str());
			}
		}

		if (!res)
		{
			ucstring errMsg = CI18N::get("uiErrChecking");
			if (!pPM->getLastErrorMessage().empty())
			{
				ucstring errMsg = pPM->getLastErrorMessage();
				printf("Error: %s\n", errMsg.toUtf8().c_str());
			}
		}
	}

	CPatchManager::SPatchInfo InfoOnPatch;

	// Check is good now ask the player if he wants to apply the patch
	pPM->getInfoToDisp(InfoOnPatch);

	uint AvailablePatchs = InfoOnPatch.getAvailablePatchsBitfield();

	// Get the list of optional categories to patch
	vector<string> vCategories;

	for(uint i = 0; i < InfoOnPatch.OptCat.size(); i++)
	{
		// Ok for the moment all optional categories must be patched even if the player
		// does not want it. Because we cant detect that a continent have to be patched ingame.
		vCategories.push_back(InfoOnPatch.OptCat[i].Name);
	}

	pPM->startPatchThread(vCategories, true);

	res = false;
	finished = false;

	while (!finished)
	{
		nlSleep(100);

		finished = pPM->isPatchThreadEnded(res);

		if (pPM->getThreadState(state, log))
		{
			printf("%s\n", state.toUtf8().c_str());

			for(uint i = 0; i < log.size(); ++i)
			{
				printf("%s\n", log[i].toUtf8().c_str());
			}
		}

		if (!res)
		{
			ucstring errMsg = CI18N::get("uiErrChecking");
			if (!pPM->getLastErrorMessage().empty())
			{
				ucstring errMsg = pPM->getLastErrorMessage();
				printf("Error: %s\n", errMsg.toUtf8().c_str());
			}
		}
	}

	if (CPatchManager::getInstance()->mustLaunchBatFile())
	{
		// move downloaded files to final location
		pPM->createBatchFile(pPM->getDescFile(), false, false);
		CFile::createEmptyFile("show_eula");
	}

/*
	// Start Scanning
	pPM->startScanDataThread();

	// request to stop the thread
	pPM->askForStopScanDataThread();
*/

	delete appContext;

	return 0;
}

