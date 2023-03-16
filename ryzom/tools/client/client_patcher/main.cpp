#include "stdpch.h"
#include "login_patch.h"
#include "client_cfg.h"
#include "user_agent.h"

#include "nel/misc/cmd_args.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace NLMISC;
using namespace std;

// simplified implementation to not depend on client_cfg.cpp
CClientConfig::CClientConfig()
{
}

void CClientConfig::serial(NLMISC::IStream &/* f */)
{
}

bool CClientConfig::getDefaultConfigLocation(std::string&/* p_name */) const
{
	return false;
}

CClientConfig ClientCfg;

// stuff which is defined as extern in other .cpp files
void quitCrashReport()
{
}

void stopSoundMngr()
{
}

/// domain server version for patch
string	R2ServerVersion;
/// name of the version (used to alias many version under the same name),
/// the value is used to get the release not if not empty
string	VersionName;

string LoginLogin, LoginPassword;
uint32 LoginShardId = 0xFFFFFFFF;

CCmdArgs Args;

bool useEsc = false;

#ifdef NL_OS_WINDOWS
HANDLE hStdout = NULL;
sint attributes = 0;
#endif

std::string convert(const ucstring &str)
{
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

	uint length = (uint)nstr.length();

	if (length > maxLength)
	{
		nstr = std::string("...") + nstr.substr(length - maxLength + 3);
		length = maxLength;
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

// hardcoded english translations to not depend on external files
struct CClientPatcherTranslations : public NLMISC::CI18N::ILoadProxy
{
	virtual void loadStringFile(const std::string &filename, ucstring &text)
	{
		text.fromUtf8(
			"TheSagaOfRyzom	[Ryzom]\n"
			"uiErrPatchApply	[Error: Patch process ended but the patch has not been successfully applied.]\n"
			"uiErrChecking	[Error: Patch files failed - checking.]\n"
			"uiByte	[B]\n"
			"uiKb	[KiB]\n"
			"uiMb	[MiB]\n"
			"uiLoginGetFile	[Getting File:]\n"
			"uiDLWithCurl	[Downloading File With Curl:]\n"
			"uiDecompressing	[Decompressing File:]\n"
			"uiCheckInt	[Checking Integrity:]\n"
			"uiNoVersionFound	[No Version Found]\n"
			"uiVersionFound	[Version Found:]\n"
			"uiApplyingDelta	[Applying Delta:]\n"
			"uiClientVersion	[Client Version]\n"
			"uiServerVersion	[Server Version]\n"
			"uiCheckingFile	[Checking File]\n"
			"uiNeededPatches	[Required Patches:]\n"
			"uiCheckInBNP	[Checking inside BNP:]\n"
			"uiSHA1Diff	[Force BNP Unpacking: checksums do not correspond:]\n"
			"uiCheckEndNoErr	[Checking file ended with no errors]\n"
			"uiCheckEndWithErr	[Checking file ended with errors:]\n"
			"uiPatchEndNoErr	[Patching file ended with no errors]\n"
			"uiPatchEndWithErr	[Patch failed!]\n"
			"uiPatchDiskFull	[Disk full!]\n"
			"uiPatchWriteError	[Disk write error! (disk full?)]\n"
			"uiProcessing	[Processing file:]\n"
			"uiUnpack	[BNP Unpacking:]\n"
			"uiUnpackErrHead	[Cannot read bnp header:]\n"
			"uiChangeDate	[Changing the mod date:]\n"
			"uiChgDateErr	[Cannot change file time:]\n"
			"uiNowDate	[Now the date is:]\n"
			"uiSetAttrib	[Set file attributes:]\n"
			"uiAttribErr	[Cannot have read/write access:]\n"
			"uiDelFile	[Delete file:]\n"
			"uiDelErr	[Cannot delete file:]\n"
			"uiDelNoFile	[Delete file (no file)]\n"
			"uiRenameFile	[Rename File:]\n"
			"uiRenameErr	[Cannot rename file:]\n"
		);
	}
};

// hardcoded URL to not depend on external files
static const std::string DefaultPatchUrl = RYZOM_CLIENT_PATCH_URL; // "https://cdn.ryzom.dev/open/patch";
static const std::string DefaultAppName = RYZOM_CLIENT_APP_NAME; // "default"

int main(int argc, char *argv[])
{
	// init the Nel context
	CApplicationContext appContext;

	Args.setVersion(getDisplayVersion());
	Args.setDescription("Ryzom client");
	Args.addArg("p", "patch", "patch", "Name of the file to use tp xdelta the source file");
	Args.addArg("s", "source", "source", "Name of source file to xdelta with patch file");
	Args.addArg("d", "destination", "destination", "Name of destination operation (patch or unpack)");
	Args.addArg("u", "unpack", "unpack", "Name of bnp file to unpack");
	Args.addArg("", "url", "PatchUrl", "Patch server url, ie '" RYZOM_CLIENT_PATCH_URL "'");
	Args.addArg("", "app", "Application", "Patch application name for version file, ie '" RYZOM_CLIENT_APP_NAME "' requests " RYZOM_CLIENT_APP_NAME ".version from PatchUrl");

	if (!Args.parse(argc, argv)) return 1;

	if (Args.haveArg("p") && Args.haveArg("s") && Args.haveArg("d"))
	{
		string	patchName = Args.getArg("p").front();
		string	sourceName = Args.getArg("s").front();
		string	destinationName = Args.getArg("d").front();

		std::string errorMsg;
		CXDeltaPatch::TApplyResult ar = CXDeltaPatch::apply(patchName, sourceName, destinationName, errorMsg);
		nlinfo("%s", errorMsg.c_str());
		return ar;
	}

	if (Args.haveArg("u") && Args.haveArg("d"))
	{
		string	bnpName = Args.getArg("u").front();
		string	destinationName = Args.getArg("d").front();
		vector<string> vFilenames;
		if (CPatchManager::getInstance()->bnpUnpack(bnpName, destinationName, vFilenames))
			return 0;
		return 1;
	}

	// create logs in temporary directory
	createDebug(CPath::getTemporaryDirectory().c_str(), true, true);

	// disable log display on stdout
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");

	// check if console supports colors
	std::string term = toLowerAscii(std::string(getenv("TERM") ? getenv("TERM"):""));
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

	// allocate translations proxy
	CClientPatcherTranslations *trans = new CClientPatcherTranslations();

	// use proxy
	CI18N::setLoadProxy(trans);

 	// load english translations
	CI18N::load("en");

	// now translations are read, we don't need it anymore
	delete trans;

	// create minimal client.cfg file in memory for patcher
	{
		CConfigFile::CVar patchUrl;
		patchUrl.forceAsString(DefaultPatchUrl);
		if (Args.haveLongArg("url") && !Args.getLongArg("url").empty())
			patchUrl.forceAsString(Args.getLongArg("url").front());

		CConfigFile::CVar appName;
		appName.forceAsString(DefaultAppName);
		if (Args.haveLongArg("app") && !Args.getLongArg("app").empty())
			appName.forceAsString(Args.getLongArg("app").front());

		ClientCfg.ConfigFile.insertVar("PatchUrl", patchUrl);
		ClientCfg.ConfigFile.insertVar("Application", appName);
	}

	//
	Args.displayVersion();
	printf("\n");
	printf("Checking %s files to patch...\n", convert(CI18N::get("TheSagaOfRyzom")).c_str());
	printf("Using '%s/%s.version'\n", ClientCfg.ConfigFile.getVar("PatchUrl").asString().c_str(),
		ClientCfg.ConfigFile.getVar("Application").asString().c_str());

	// FIXME: On Ryzom Core 4, the patch url and version are retrieved from the login server
	// using StartupHost, StartupPage, and Application[0], as different domains 
	// may be running on the same patch series at different versions.
	// The version file on the patch server is a dummy containing just "*",
	// since this file gets cached by the CDN.
	
	// defaults to the current build in case version file is not used
	int versionNumbers[] = { RYZOM_VERSION_RC, 0, 0 };
	VersionName = NLMISC::toString(versionNumbers[2]);

	// initialize patch manager and set the ryzom full path, before it's used
	CPatchManager *pPM = CPatchManager::getInstance();

	// use PatchUrl
	vector<string> patchURLs;
	pPM->init(patchURLs, ClientCfg.ConfigFile.getVar("PatchUrl").asString(), VersionName);
	pPM->startCheckThread(true /* include background patchs */);

	string state;
	vector<string> log;
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
			// batch file will not be created
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

	// upgd_nl.sh will normally take care of the permissions
	//
	// for linux/macOS (no-op on windows)
	// Set for current executable (might be 'dev' version),
	// and also 'ryzom_client_patcher' directly (from patched files)
	CFile::setExecutable(Args.getProgramPath() + Args.getProgramName());
	CFile::setExecutable("ryzom_client_patcher");
	// other
	CFile::setExecutable("crash_report");
	CFile::setExecutable("ryzom_client");
	CFile::setExecutable("ryzom_installer_qt");
	CFile::setExecutable("ryzom_configuration_qt");

	return 0;
}
