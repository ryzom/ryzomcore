// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Matthew LAGOE (Botanic) <cyberempires@gmail.com>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//
// Includes
//

#include "stdpch.h"

#include <sys/stat.h>

#ifndef NL_OS_WINDOWS
	#include <unistd.h>
#endif

#ifdef NL_OS_MAC
	#include "app_bundle_utils.h"
#endif

#include <memory>
#include <errno.h>

#define USE_CURL

#ifdef USE_CURL
	#include <curl/curl.h>
#endif

#include <zlib.h>

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"
#include "nel/misc/sha1.h"
#include "nel/misc/big_file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/cmd_args.h"
#include "nel/misc/seven_zip.h"
#include "nel/web/curl_certificates.h"

#include "game_share/bg_downloader_msg.h"

#include "login_patch.h"
#include "login.h"
#include "user_agent.h"


#ifndef RY_BG_DOWNLOADER
	#include "client_cfg.h"
#else
	#include "../client_background_downloader/client_background_downloader.h"
	#define __CLIENT_INSTALL_EXE__
#endif

#ifdef NL_OS_WINDOWS
	#include <direct.h>
#endif

//
// Namespaces
//


using namespace std;
using namespace NLMISC;


extern string VersionName;
extern string R2ServerVersion;

#ifdef __CLIENT_INSTALL_EXE__
	extern std::string TheTmpInstallDirectory;
	extern std::string ClientLauncherUrl;
#else
	std::string TheTmpInstallDirectory = "patch/client_install";
#endif

extern NLMISC::CCmdArgs Args;

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// CPatchManager
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

struct EPatchDownloadException : public Exception
{
	EPatchDownloadException() : Exception( "Download Error" ) {}
	EPatchDownloadException( const std::string& str ) : Exception( str ) {}
	virtual ~EPatchDownloadException() throw(){}
};


CPatchManager *CPatchManager::_Instance = NULL;

static std::string ClientRootPath;

// ****************************************************************************
CPatchManager::CPatchManager() : State("t_state"), DataScanState("t_data_scan_state")
{
	DescFilename = "ryzom_xxxxx.idx";

#ifdef NL_OS_WINDOWS
	UpdateBatchFilename = "updt_nl.bat";
	UpgradeBatchFilename = "upgd_nl.bat";
#else
	UpdateBatchFilename = "updt_nl.sh";
	UpgradeBatchFilename = "upgd_nl.sh";
#endif

	std::string rootPath;

	if (ClientCfg.getDefaultConfigLocation(rootPath))
	{
		// use same directory as client_default.cfg
		rootPath = CFile::getPath(rootPath);
	}
	else
	{
		// use current directory
		rootPath = CPath::getCurrentPath();
	}

	setClientRootPath(rootPath);

	VerboseLog = true;

	PatchThread = NULL;
	CheckThread = NULL;
	InstallThread = NULL;
	ScanDataThread = NULL;
	DownloadThread = NULL;
	Thread = NULL;

	LogSeparator = "\n";
	ValidDescFile = false;

	MustLaunchBatFile = false;

	DownloadInProgress = false;
	_AsyncDownloader = NULL;
	_StateListener = NULL;
	_StartRyzomAtEnd = true;
}

// ****************************************************************************
void CPatchManager::setClientRootPath(const std::string& clientRootPath)
{
	ClientRootPath = CPath::standardizePath(clientRootPath);
	ClientPatchPath = CPath::standardizePath(ClientRootPath + "unpack");

	// Delete the .sh file because it's not useful anymore
	std::string fullUpdateBatchFilename = ClientRootPath + UpdateBatchFilename;

	if (NLMISC::CFile::fileExists(fullUpdateBatchFilename))
		NLMISC::CFile::deleteFile(fullUpdateBatchFilename);

	WritableClientDataPath = CPath::standardizePath(ClientRootPath + "data");

#ifdef NL_OS_MAC
	ReadableClientDataPath = CPath::standardizePath(getAppBundlePath() + "/Contents/Resources/data");
#elif defined(NL_OS_UNIX)
	ReadableClientDataPath = CPath::standardizePath(getRyzomSharePrefix() + "/data");
	if (CFile::isDirectory(ReadableClientDataPath))	ReadableClientDataPath.clear();
#else
	ReadableClientDataPath.clear();
#endif

	if (ReadableClientDataPath.empty()) ReadableClientDataPath = WritableClientDataPath;
}

// ****************************************************************************
void CPatchManager::setErrorMessage(const std::string &message)
{
	_ErrorMessage = message;
}

// ****************************************************************************
void CPatchManager::forceStopCheckThread()
{
	PatchThread->StopAsked = true;
}

// ****************************************************************************
void CPatchManager::forceStopPatchThread()
{
	CheckThread->StopAsked = true;
}

// ****************************************************************************
void CPatchManager::init(const std::vector<std::string>& patchURIs, const std::string &sServerPath, const std::string &sServerVersion)
{
	uint	i;
	PatchServers.clear();

	for (i=0; i<patchURIs.size(); ++i)
	{
		PatchServers.push_back(CPatchServer(patchURIs[i]));
	}

	srand(NLMISC::CTime::getSecondsSince1970());
	UsedServer = (sint)(((double)rand() / ((double)RAND_MAX+1.0)) * (double)PatchServers.size());

	ServerPath = CPath::standardizePath (sServerPath);
	ServerVersion = sServerVersion;

	string::size_type pos = ServerPath.find ("@");
	if (pos != string::npos)
		DisplayedServerPath = "http://" + ServerPath.substr (pos+1);
	else
		DisplayedServerPath = ServerPath;

	NLMISC::CFile::createDirectory(ClientPatchPath);
	NLMISC::CFile::createDirectory(WritableClientDataPath);


	// try to read the version file from the server (that will replace the version number)
	try
	{
		CConfigFile *cf;

#ifdef RY_BG_DOWNLOADER
		cf = &theApp.ConfigFile;
#else
		cf = &ClientCfg.ConfigFile;
#endif

		// App name matches Domain on the SQL server
		std::string appName = cf->getVarPtr("Application")
			? cf->getVar("Application").asString(0)
			: "default";

		std::string versionFileName = appName + ".version";
		getServerFile(versionFileName);

		// ok, we have the file, extract version number (aka build number) and the
		// version name if present

		CIFile versionFile(ClientPatchPath + versionFileName);
		char buffer[1024];
		versionFile.getline(buffer, 1024);
		CSString line(buffer);

#ifdef NL_DEBUG
		CConfigFile::CVar *forceVersion = cf->getVarPtr("ForceVersion");

		if (forceVersion != NULL)
		{
			line = forceVersion->asString();
		}
#endif

		// Use the version specified in this file, if the file does not contain an asterisk
		if (line[0] != '*')
		{
			ServerVersion = line.firstWord(true);
			VersionName = line.firstWord(true);
		}

		// force the R2ServerVersion
		R2ServerVersion = ServerVersion;

#ifdef __CLIENT_INSTALL_EXE__
		{
			//The install program load a the url of the mini web site in the patch directory

			std::string clientLauncherUrl = "client_launcher_url.txt";
			bool ok = true;
			try
			{
				uint32 nServerVersion;
				fromString(ServerVersion, nServerVersion);
				std::string url = toString("%05u/%s", nServerVersion, clientLauncherUrl.c_str());
				// The client version is different from the server version : download new description file
				getServerFile(url.c_str(), false); // For the moment description file is not zipped
			}
			catch (...)
			{
				// fallback to patch root directory
				try
				{
					getServerFile(clientLauncherUrl.c_str(), false); // For the moment description file is not zipped
				}
				catch(...)
				{
					ok = false;
				}
			}

			if (ok)
			{
				CIFile versionFile;
				if (versionFile.open(ClientPatchPath+clientLauncherUrl) )
				{
					char buffer[1024];
					versionFile.getline(buffer, 1024);
					ClientLauncherUrl = std::string(buffer);
				}
			}
		}
#endif
	}
	catch (...)
	{
		// no version file
	}


	// retrieve the current client version, according to .idx
	readClientVersionAndDescFile();
}

// ***************************************************************************
void CPatchManager::readClientVersionAndDescFile()
{
	try
	{
		ValidDescFile = false;
		vector<string> vFiles;
		CPath::getPathContent(ClientPatchPath, false, false, true, vFiles);
		uint32 nVersion = 0xFFFFFFFF;
		uint32 nNewVersion;
		for (uint32 i = 0; i < vFiles.size(); ++i)
		{
			string sName = NLMISC::CFile::getFilename(vFiles[i]);
			string sExt = NLMISC::CFile::getExtension(sName);
			string sBase = sName.substr(0, sName.rfind('_'));
			if ((sExt == "idx") && (sBase == "ryzom"))
			{
				string val = sName.substr(sName.rfind('_')+1, 5);
				if (fromString(val, nNewVersion) && ((nNewVersion > nVersion) || (nVersion == 0xFFFFFFFF)))
					nVersion = nNewVersion;
			}
		}
		if (nVersion != 0xFFFFFFFF)
			readDescFile(nVersion);
		else
			DescFilename = "unknown";
		ValidDescFile = true;
	}
	catch(const Exception &)
	{
		nlwarning("EXCEPTION CATCH: readClientVersionAndDescFile() failed - not important");
		// Not important that there is no desc file
	}
}

// ****************************************************************************
void CPatchManager::startCheckThread(bool includeBackgroundPatch)
{
	if (CheckThread != NULL)
	{
		nlwarning ("check thread is already running");
		return;
	}
	if (Thread != NULL)
	{
		nlwarning ("a thread is already running");
		return;
	}

	_ErrorMessage.clear();

	CheckThread = new CCheckThread(includeBackgroundPatch);
	nlassert (CheckThread != NULL);

	Thread = IThread::create (CheckThread);
	nlassert (Thread != NULL);
	Thread->start ();
}

// ****************************************************************************
bool CPatchManager::isCheckThreadEnded(bool &ok)
{
	if (CheckThread == NULL)
	{
		ok = false;
		return true;
	}

	bool end = CheckThread->Ended;
	if (end)
	{
		ok = CheckThread->CheckOk;
		stopCheckThread();
	}

	return end;
}

// ****************************************************************************
void CPatchManager::stopCheckThread()
{
	if(CheckThread && Thread)
	{
		Thread->wait();
		delete Thread;
		Thread = NULL;
		delete CheckThread;
		CheckThread = NULL;
	}
}

// Return the position of the inserted/found category
sint32 updateCat(vector<CPatchManager::SPatchInfo::SCat> &rOutVec, CPatchManager::SPatchInfo::SCat &rInCat)
{
	uint32 i;
	for (i = 0; i < rOutVec.size(); ++i)
	{
		if (rOutVec[i].Name == rInCat.Name)
			break;
	}

	if (i == rOutVec.size())
	{
		rOutVec.push_back(rInCat);
	}
	else
	{
		rOutVec[i].Size += rInCat.Size;
		rOutVec[i].FinalFileSize += rInCat.FinalFileSize;
		rOutVec[i].SZipSize += rInCat.SZipSize;
	}

	return i;
}

// ****************************************************************************
void CPatchManager::getInfoToDisp(SPatchInfo &piOut)
{
	piOut.NonOptCat.clear();
	piOut.OptCat.clear();
	piOut.ReqCat.clear();

	// Convert FilesToPatch vector that must be initialized into something human readable
	uint32 i;
	for (i = 0; i < FilesToPatch.size(); ++i)
	{
		SFileToPatch &rFTP = FilesToPatch[i];
		// Find the category of the file
		sint32 nCat = -1;
		const CBNPCategorySet &rAllCats = DescFile.getCategories();
		for (uint32 j = 0; j < rAllCats.categoryCount(); ++j)
		{
			const CBNPCategory &rCat = rAllCats.getCategory(j);
			for (uint32 k = 0; k < rCat.fileCount(); ++k)
			{
				if (rCat.getFile(k) == rFTP.FileName)
				{
					nCat = j;
					break;
				}
			}

			if (nCat != -1)
				break;
		}

		if (nCat != -1)
		{
			// Add the category found, if already there just update the size
			const CBNPCategory &rCat = rAllCats.getCategory(nCat);
			string sCatName = rCat.getName();
			// Size of all patches
			uint32 nTotalPatchesSize = 0;
			for (uint32 j = 0; j < rFTP.PatcheSizes.size(); ++j)
				nTotalPatchesSize += rFTP.PatcheSizes[j];

			SPatchInfo::SCat c;
			c.Name = sCatName;
			c.Size = nTotalPatchesSize;
			c.SZipSize = rFTP.SZFileSize;
			c.FinalFileSize = rFTP.FinalFileSize;
			if (!rCat.getCatRequired().empty())
			{
				// Ensure the required cat exists
				SPatchInfo::SCat rc;
				rc.Name = rCat.getCatRequired();
				c.Req = updateCat(piOut.ReqCat, rc);
			}

			// In which category of category should we add it ?
			if (rCat.isOptional())
			{
				if (rCat.isHidden())
					updateCat(piOut.ReqCat, c);
				else
					updateCat(piOut.OptCat, c);
			}
			else
			{
				updateCat(piOut.NonOptCat, c);
			}
		}
	}

	for (i = 0; i < OptionalCat.size(); ++i)
	{
		const CBNPCategory *pCat = DescFile.getCategories().getCategory(OptionalCat[i]);
		nlassert(pCat != NULL);

		SPatchInfo::SCat c;
		c.Name = pCat->getName();

		if (!pCat->getCatRequired().empty())
		{
			// Ensure the required cat exists
			SPatchInfo::SCat rc;
			rc.Name = pCat->getCatRequired();
			c.Req = updateCat(piOut.ReqCat, rc);
		}

		updateCat(piOut.OptCat, c);
	}
}

// ****************************************************************************
// TODO : use selected categories to patch a list of files
void CPatchManager::startPatchThread(const vector<string> &CategoriesSelected, bool applyPatch)
{
	if (PatchThread != NULL)
	{
		nlwarning ("check thread is already running");
		return;
	}
	if (Thread != NULL)
	{
		nlwarning ("a thread is already running");
		return;
	}

	_ErrorMessage.clear();

	PatchThread = new CPatchThread(applyPatch);
	nlassert (PatchThread != NULL);

	// Select all the files we have to patch depending on non-optional categories and selected categories
	uint32 i, j, k;
	// Add non-optional categories
	vector<string> CatsSelected = CategoriesSelected;
	PatchThread->clear();
	const CBNPCategorySet &rAllCats = DescFile.getCategories();
	for (i = 0; i < rAllCats.categoryCount(); ++i)
	{
		const CBNPCategory &rCat = rAllCats.getCategory(i);
		if (!rCat.isOptional())
			CatsSelected.push_back(rCat.getName());
	}

	// Add all required categories
	uint32 nSize = (uint32)CatsSelected.size();
	do
	{
		nSize = (uint32)CatsSelected.size();

		for (i = 0; i < CatsSelected.size(); ++i)
		{
			const CBNPCategory *pCat = rAllCats.getCategory(CatsSelected[i]);
			if (pCat == NULL) continue;
			if (pCat->getCatRequired().empty()) continue;
			// Check if the category required is already present
			for (j = 0; j < CatsSelected.size(); ++j)
			{
				const CBNPCategory *pCat2 = rAllCats.getCategory(CatsSelected[j]);
				if (pCat2->getName() == pCat->getCatRequired())
					break;
			}
			// Not present ?
			if (j == CatsSelected.size())
			{
				CatsSelected.push_back(pCat->getCatRequired());
				break;
			}
		}
	}
	while(nSize != CatsSelected.size());

	// Select files
	for (i = 0; i < CatsSelected.size(); ++i)
	{
		// Find the category from the name
		const CBNPCategory *pCat = rAllCats.getCategory(CatsSelected[i]);
		if (pCat != NULL)
		{
			for (j = 0; j < pCat->fileCount(); ++j)
			{
				const string &rFilename = pCat->getFile(j);
				const CBNPFileSet &rFileSet = DescFile.getFiles();
				const CBNPFile *pFile = rFileSet.getFileByName(rFilename);
				if (pFile != NULL)
				{
					// Look if it's a file to patch
					for (k = 0; k < FilesToPatch.size(); ++k)
						if (FilesToPatch[k].FileName == pFile->getFileName())
							break;

					if (k < FilesToPatch.size())
					{
						FilesToPatch[k].Incremental = pCat->isIncremental();
						if (!pCat->getUnpackTo().empty())
							FilesToPatch[k].ExtractPath = CPath::standardizePath(pCat->getUnpackTo());

						PatchThread->add(FilesToPatch[k]);

						// Close opened big files
						CBigFile::getInstance().remove(FilesToPatch[k].FileName);
					}
				}
			}
		}
	}

	// Launch the thread
	Thread = IThread::create (PatchThread);
	nlassert (Thread != NULL);
	Thread->start ();
}

// ****************************************************************************
bool CPatchManager::isPatchThreadEnded (bool &ok)
{
	if (PatchThread == NULL)
	{
		ok = false;
		return true;
	}

	bool end = PatchThread->Ended;
	if (end)
	{
		ok = PatchThread->PatchOk;
		stopPatchThread();
	}

	return end;
}

// ****************************************************************************
// Called in main thread
bool CPatchManager::getThreadState (std::string &stateOut, vector<string> &stateLogOut)
{
	if ((PatchThread == NULL) && (CheckThread == NULL) && (ScanDataThread==NULL))
		return false;

	// clear output
	stateOut.clear();
	stateLogOut.clear();

	// Get access to the state
	bool	changed= false;
	{
		CSynchronized<CState>::CAccessor as(&State);
		CState	&rState= as.value();
		if (rState.StateChanged)
		{
			// and retrieve info
			changed= true;
			stateOut = rState.State;
			stateLogOut = rState.StateLog;
			// clear state
			rState.StateLog.clear();
			rState.StateChanged= false;
		}
	}

	// verbose log
	if (isVerboseLog() && !stateLogOut.empty())
		for (uint32 i = 0; i < stateLogOut.size(); ++i)
			nlinfo("%s", stateLogOut[i].c_str());

	return changed;
}

// ****************************************************************************
void CPatchManager::stopPatchThread()
{
	if(PatchThread && Thread)
	{
		Thread->wait();
		delete Thread;
		Thread = NULL;
		delete PatchThread;
		PatchThread = NULL;
	}
}

// ****************************************************************************
void CPatchManager::deleteBatchFile()
{
	deleteFile(ClientRootPath + UpdateBatchFilename, false, false);
}

// ****************************************************************************
void CPatchManager::createBatchFile(CProductDescriptionForClient &descFile, bool wantRyzomRestart, bool useBatchFile)
{
	uint nblab = 0;

	std::string content;

	// Unpack files with category ExtractPath non empty
	const CBNPCategorySet &rDescCats = descFile.getCategories();
	OptionalCat.clear();

	for (uint32 i = 0; i < rDescCats.categoryCount(); ++i)
	{
		// For all optional categories check if there is a 'file to patch' in it
		const CBNPCategory &rCat = rDescCats.getCategory(i);

		nlinfo("Category = %s", rCat.getName().c_str());

		if (!rCat.getUnpackTo().empty())
		for (uint32 j = 0; j < rCat.fileCount(); ++j)
		{
			string rFilename = ClientPatchPath + rCat.getFile(j);

			nlinfo("\tFileName = %s", rFilename.c_str());

			// Extract to patch
			vector<string> vFilenames;

			bool result = false;

			try
			{
				result = bnpUnpack(rFilename, ClientPatchPath, vFilenames);
			}
			catch(...)
			{
				throw;
			}

			if (!result)
			{
				// TODO: handle exception?
				string err = toString("Error unpacking %s", rFilename.c_str());

				throw Exception (err);
			}
			else
			{
				for (uint32 fff = 0; fff < vFilenames.size (); fff++)
				{
					// this file must be moved
					string fullDstPath = CPath::standardizePath(rCat.getUnpackTo()); // to be sure there is a / at the end
					NLMISC::CFile::createDirectoryTree(fullDstPath);

					std::string FileName = vFilenames[fff];

					bool succeeded = false;

					if (!useBatchFile)
					{
						// don't check result, because it's possible the olk file doesn't exist
						CFile::deleteFile(fullDstPath + FileName);

						// try to move it, if fails move it later in a script
						if (CFile::moveFile(fullDstPath + FileName, ClientPatchPath + FileName))
							succeeded = true;
					}

					// if we didn't succeed to delete or move the file, create a batch file anyway
					if (!succeeded)
					{
						string batchRelativeDstPath;

						// should be always true
						if (fullDstPath.compare(0, ClientRootPath.length(), ClientRootPath) == 0)
						{
							batchRelativeDstPath = fullDstPath.substr(ClientRootPath.length()) + FileName;
						}
						else
						{
							batchRelativeDstPath = fullDstPath + FileName;
						}

#ifdef NL_OS_WINDOWS
						// only fix backslashes for .bat
						batchRelativeDstPath = CPath::standardizeDosPath(batchRelativeDstPath);

						// use DSTPATH and SRCPATH variables and append filenames
						string realDstPath = toString("\"%%ROOTPATH%%\\%s\"", batchRelativeDstPath.c_str());
						string realSrcPath = toString("\"%%UNPACKPATH%%\\%s\"", FileName.c_str());

						content += toString(":loop%u\n", nblab);
						content += toString("attrib -r -a -s -h %s\n", realDstPath.c_str());
						content += toString("del %s\n", realDstPath.c_str());
						content += toString("if exist %s goto loop%u\n", realDstPath.c_str(), nblab);
						content += toString("move %s %s\n", realSrcPath.c_str(), realDstPath.c_str());
#else
						// use DSTPATH and SRCPATH variables and append filenames
						string realDstPath = toString("\"$ROOTPATH/%s\"", batchRelativeDstPath.c_str());
						string realSrcPath = toString("\"$UNPACKPATH/%s\"", FileName.c_str());

						content += toString("rm -rf %s\n", realDstPath.c_str());
						content += toString("mv %s %s\n", realSrcPath.c_str(), realDstPath.c_str());
#endif

						content += "\n";
					}

					nblab++;
				}
			}
		}
	}

	std::string patchDirectory = CPath::standardizePath(ClientRootPath + "patch");

	// Finalize batch file
	if (NLMISC::CFile::isExists(patchDirectory) && NLMISC::CFile::isDirectory(patchDirectory))
	{
		std::string patchContent;

		vector<string> vFileList;
		CPath::getPathContent (patchDirectory, false, false, true, vFileList, NULL, false);

		for(uint32 i = 0; i < vFileList.size(); ++i)
		{
			bool succeeded = false;

			if (!useBatchFile)
			{
				if (CFile::deleteFile(vFileList[i]))
					succeeded = true;
			}

			// if we didn't succeed to delete, create a batch file anyway
			if (!succeeded)
			{
#ifdef NL_OS_WINDOWS
				patchContent += toString("del \"%%ROOTPATH%%\\patch\\%s\"\n", vFileList[i].c_str());
#else
				patchContent += toString("rm -f \"$ROOTPATH/patch/%s\"\n", vFileList[i].c_str());
#endif
			}
		}

		if (!patchContent.empty())
		{
#ifdef NL_OS_WINDOWS
			content += ":looppatch\n";

			content += patchContent;

			content += "rd /Q /S \"%%ROOTPATH%%\\patch\"\n";
			content += "if exist \"%%ROOTPATH%%\\patch\" goto looppatch\n";
#else
			content += "rm -rf \"$ROOTPATH/patch\"\n";
#endif
		}
		else
		{
			CFile::deleteDirectory(patchDirectory);
		}
	}

	if (!content.empty())
	{
		deleteBatchFile();

		// batch full path
		std::string batchFilename = ClientRootPath + UpdateBatchFilename;

		// write windows .bat format else write sh format
		FILE *fp = nlfopen (batchFilename, "wt");

		if (fp == NULL)
		{
			string err = toString("Can't open file '%s' for writing: code=%d %s (error code 29)", batchFilename.c_str(), errno, strerror(errno));
			throw Exception (err);
		}
		else
		{
			nlinfo("Creating %s...", batchFilename.c_str());
		}

		string contentPrefix;

		//use bat if windows if not use sh
#ifdef NL_OS_WINDOWS
		contentPrefix += "@echo off\n";
		contentPrefix += "set RYZOM_CLIENT=%~1\n";
		contentPrefix += "set UNPACKPATH=%~2\n";
		contentPrefix += "set ROOTPATH=%~3\n";
		contentPrefix += "set STARTUPPATH=%~4\n";
		contentPrefix += toString("set UPGRADE_FILE=%%ROOTPATH%%\\%s\n", UpgradeBatchFilename.c_str());
		contentPrefix += "\n";
		contentPrefix += "set LOGIN=%~5\n";
		contentPrefix += "set PASSWORD=%~6\n";
		contentPrefix += "set SHARDID=%~7\n";
#else
		contentPrefix += "#!/bin/sh\n";
		contentPrefix += "export RYZOM_CLIENT=\"$1\"\n";
		contentPrefix += "export UNPACKPATH=\"$2\"\n";
		contentPrefix += "export ROOTPATH=\"$3\"\n";
		contentPrefix += "export STARTUPPATH=\"$4\"\n";
		contentPrefix += toString("export UPGRADE_FILE=$ROOTPATH/%s\n", UpgradeBatchFilename.c_str());
		contentPrefix += "\n";
		contentPrefix += "LOGIN=\"$5\"\n";
		contentPrefix += "PASSWORD=\"$6\"\n";
		contentPrefix += "SHARDID=\"$7\"\n";
#endif

		contentPrefix += "\n";

		string contentSuffix;

		// if we need to restart Ryzom, we need to launch it in batch
		std::string additionalParams;

		if (Args.haveLongArg("profile"))
		{
			additionalParams = "--profile " + Args.getLongArg("profile").front();
		}

#ifdef NL_OS_WINDOWS
		// launch upgrade script if present (it'll execute additional steps like moving or deleting files)
		contentSuffix += "if exist \"%UPGRADE_FILE%\" call \"%UPGRADE_FILE%\"\n";

		if (wantRyzomRestart)
		{
			// client shouldn't be in memory anymore else it couldn't be overwritten
			contentSuffix += toString("start \"\" /D \"%%STARTUPPATH%%\" \"%%RYZOM_CLIENT%%\" %s \"%%LOGIN%%\" \"%%PASSWORD%%\" \"%%SHARDID%%\"\n", additionalParams.c_str());
		}
#else
		if (wantRyzomRestart)
		{
			// wait until client not in memory anymore
			contentSuffix += toString("until ! pgrep -x \"%s\" > /dev/null; do sleep 1; done\n", CFile::getFilename(RyzomFilename).c_str());
		}

		// launch upgrade script if present (it'll execute additional steps like moving or deleting files)
		contentSuffix += "if [ -e \"$UPGRADE_FILE\" ]; then chmod +x \"$UPGRADE_FILE\" && \"$UPGRADE_FILE\"; fi\n\n";

		// be sure file is executable
		contentSuffix += "chmod +x \"$RYZOM_CLIENT\"\n\n";

		if (wantRyzomRestart)
		{
			// change to previous client directory
			contentSuffix += "cd \"$STARTUPPATH\"\n\n";

			// launch new client
#ifdef NL_OS_MAC
			// use exec command under OS X
			contentSuffix += toString("exec \"$RYZOM_CLIENT\" %s \"$LOGIN\" \"$PASSWORD\" \"$SHARDID\"\n", additionalParams.c_str());
#else
			contentSuffix += toString("\"$RYZOM_CLIENT\" %s \"$LOGIN\" \"$PASSWORD\" \"$SHARDID\" &\n", additionalParams.c_str());
#endif
		}
#endif

		// append content of script
		fputs(contentPrefix.c_str(), fp);
		fputs(content.c_str(), fp);
		fputs(contentSuffix.c_str(), fp);

		bool writeError = ferror(fp) != 0;
		bool diskFull = ferror(fp) && errno == 28 /* ENOSPC */;
		fclose(fp);
		if (diskFull)
		{
			throw NLMISC::EDiskFullError(batchFilename.c_str());
		}
		if (writeError)
		{
			throw NLMISC::EWriteError(batchFilename.c_str());
		}
	}
}

// ****************************************************************************
void CPatchManager::executeBatchFile()
{
	// normal quit
	extern void quitCrashReport ();
	quitCrashReport ();

	bool r2Mode = false;

#ifndef RY_BG_DOWNLOADER
	r2Mode = ClientCfg.R2Mode;
#endif

	std::string batchFilename;

	std::vector<std::string> arguments;

	std::string startupPath = Args.getStartupPath();

	// 3 first parameters are Ryzom client full path, patch directory full path and client root directory full path
#ifdef NL_OS_WINDOWS
	batchFilename = CPath::standardizeDosPath(ClientRootPath);

	arguments.push_back(CPath::standardizeDosPath(RyzomFilename));
	arguments.push_back(CPath::standardizeDosPath(ClientPatchPath));
	arguments.push_back(CPath::standardizeDosPath(ClientRootPath));
	arguments.push_back(CPath::standardizeDosPath(startupPath));
#else
	batchFilename = ClientRootPath;

	arguments.push_back(RyzomFilename);
	arguments.push_back(ClientPatchPath);
	arguments.push_back(ClientRootPath);
	arguments.push_back(startupPath);
#endif

	// log parameters passed to Ryzom client
	nlinfo("Restarting Ryzom...");
	nlinfo("RyzomFilename = %s", RyzomFilename.c_str());
	nlinfo("ClientPatchPath = %s", ClientPatchPath.c_str());
	nlinfo("ClientRootPath = %s", ClientRootPath.c_str());
	nlinfo("StartupPath = %s", startupPath.c_str());

	batchFilename += UpdateBatchFilename;

	// make script executable
	CFile::setRWAccess(batchFilename);
	CFile::setExecutable(batchFilename);

	// append login, password and shard
	if (!LoginLogin.empty())
	{
		arguments.push_back(LoginLogin);

		if (!LoginPassword.empty())
		{
			// encode password in hexadecimal to avoid invalid characters on command-line
			arguments.push_back("0x" + toHexa(LoginPassword));

			if (!r2Mode)
			{
				arguments.push_back(toString(LoginShardId));
			}
		}
	}

	// launchProgram with array of strings as argument will escape arguments with spaces
	if (!launchProgramArray(batchFilename, arguments, false))
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s': code=%d %s (error code 30)", batchFilename.c_str(), errno, strerror(errno));
		throw Exception (str);
	}
}

// ****************************************************************************
void CPatchManager::reboot()
{
	onFileInstallFinished(); //In install program wait for the player to select in the gui of the install program if we want to Launch Ryzom or not
	createBatchFile(DescFile, _StartRyzomAtEnd);
	executeBatchFile();
}


// ****************************************************************************
int CPatchManager::getTotalFilesToGet()
{
	if (CheckThread != NULL)
		return CheckThread->TotalFileToCheck;

	if (PatchThread != NULL)
		return PatchThread->getNbFileToPatch();

	if (ScanDataThread != NULL)
		return ScanDataThread->TotalFileToScan;

	return 1;
}

// ****************************************************************************
int CPatchManager::getCurrentFilesToGet()
{
	if (CheckThread != NULL)
		return CheckThread->CurrentFileChecked;

	if (PatchThread != NULL)
		return PatchThread->getCurrentFilePatched();

	if (ScanDataThread != NULL)
		return ScanDataThread->CurrentFileScanned;

	return 1;
}

// ****************************************************************************
int CPatchManager::getPatchingSize()
{
	if (PatchThread != NULL)
		return PatchThread->getPatchingSize();
	return 0;
}

// ****************************************************************************
float CPatchManager::getCurrentFileProgress() const
{
	if (PatchThread != NULL)
		return PatchThread->getCurrentFileProgress();
	return 0.f;
}

// ****************************************************************************
void CPatchManager::setRWAccess (const string &filename, bool bThrowException)
{
	string s = CI18N::get("uiSetAttrib") + " " + CFile::getFilename(filename);
	setState(true, s);

	if (!NLMISC::CFile::setRWAccess(filename) && bThrowException)
	{
		s = CI18N::get("uiAttribErr") + " " + CFile::getFilename(filename) + " (" + toString(errno) + "," + strerror(errno) + ")";
		setState(true, s);
		throw Exception (s);
	}
}

// ****************************************************************************
string CPatchManager::deleteFile (const string &filename, bool bThrowException, bool bWarning)
{
	string s = CI18N::get("uiDelFile") + " " + CFile::getFilename(filename);
	setState(true, s);

	if (!NLMISC::CFile::fileExists(filename))
	{
		s = CI18N::get("uiDelNoFile");
		setState(true, s);
		return s;
	}

	if (!NLMISC::CFile::deleteFile(filename))
	{
		s = CI18N::get("uiDelErr") + " " + CFile::getFilename(filename) + " (" + toString(errno) + "," + strerror(errno) + ")";
		if(bWarning)
			setState(true, s);
		if(bThrowException)
			throw Exception (s);
		return s;
	}
	return "";
}

// ****************************************************************************
void CPatchManager::renameFile (const string &src, const string &dst)
{
	string s = CI18N::get("uiRenameFile") + " " + NLMISC::CFile::getFilename(src);
	setState(true, s);

	if (!NLMISC::CFile::moveFile(dst, src))
	{
		s = CI18N::get("uiRenameErr") + " " + src + " -> " + dst + " (" + toString(errno) + "," + strerror(errno) + ")";
		setState(true, s);
		throw Exception (s);
	}
}

// ****************************************************************************
// Take care this function is called by the thread
void CPatchManager::setState (bool bOutputToLog, const string &ucsNewState)
{
	{
		CSynchronized<CState>::CAccessor as(&State);
		CState	&rState= as.value();
		rState.State= ucsNewState;
		if(bOutputToLog)
			rState.StateLog.push_back(ucsNewState);
		rState.StateChanged= true;
	}
	if (_StateListener)
	{
		_StateListener->setState(bOutputToLog, ucsNewState);
	}
}

// ****************************************************************************
void CPatchManager::touchState ()
{
	{
		CSynchronized<CState>::CAccessor as(&State);
		as.value().StateChanged= true;
	}
}

// ****************************************************************************
string CPatchManager::getClientVersion()
{
	if (!ValidDescFile)
		return "";

	return toString(DescFile.getFiles().getVersionNumber());
}

// ****************************************************************************
void CPatchManager::readDescFile(sint32 nVersion)
{
	DescFilename = toString("ryzom_%05d.idx", nVersion);
	string srcName = ClientPatchPath + DescFilename;
	DescFile.clear();
	if (!DescFile.load(srcName))
		throw Exception ("Can't open file '%s'", srcName.c_str ());

	uint cat;

	if (ClientRootPath != "./")
	{
		// fix relative paths
		for (cat = 0; cat < DescFile.getCategories().categoryCount(); ++cat)
		{
			CBNPCategory &category = const_cast<CBNPCategory &>(DescFile.getCategories().getCategory(cat));

			std::string unpackTo = category.getUnpackTo();

			if (unpackTo.substr(0, 1) == ".")
			{
				unpackTo = CPath::makePathAbsolute(unpackTo, ClientRootPath, true);
				category.setUnpackTo(unpackTo);
			}
		}
	}

	// patch category for current platform
	std::string platformPatchCategory;

#if defined(NL_OS_WIN64)
	platformPatchCategory = "main_exedll_win64";
#elif defined(NL_OS_WINDOWS)
	platformPatchCategory = "main_exedll_win32";
#elif defined(NL_OS_MAC)
	platformPatchCategory = "main_exedll_osx";
#elif defined(NL_OS_UNIX) && defined(_LP64)
	platformPatchCategory = "main_exedll_linux64";
#else
	platformPatchCategory = "main_exedll_linux32";
#endif

	// check if we are using main_exedll or specific main_exedll_* for platform
	bool foundPlatformPatchCategory = false;

	for (cat = 0; cat < DescFile.getCategories().categoryCount(); ++cat)
	{
		CBNPCategory &category = const_cast<CBNPCategory &>(DescFile.getCategories().getCategory(cat));

		if (category.getName() == platformPatchCategory)
		{
			foundPlatformPatchCategory = true;
			break;
		}
	}

	if (foundPlatformPatchCategory)
	{
		std::set<std::string> forceRemovePatchCategories;

		// only download binaries for current platform
		forceRemovePatchCategories.insert("main_exedll");
		forceRemovePatchCategories.insert("main_exedll_win32");
		forceRemovePatchCategories.insert("main_exedll_win64");
		forceRemovePatchCategories.insert("main_exedll_linux32");
		forceRemovePatchCategories.insert("main_exedll_linux64");
		forceRemovePatchCategories.insert("main_exedll_osx");

		// remove current platform category from remove list
		forceRemovePatchCategories.erase(platformPatchCategory);

		CBNPFileSet &bnpFS = const_cast<CBNPFileSet &>(DescFile.getFiles());

		// TODO: .ref files are expected to follow platform category naming (they are in 'main' category)
		std::set<std::string>::const_iterator it;
		for(it = forceRemovePatchCategories.begin(); it != forceRemovePatchCategories.end(); ++it)
		{
			std::string name = *it;
			std::string::size_type pos = name.find("_");
			if (pos != std::string::npos)
			{
				name = name.substr(pos+1) + "_.ref";
				bnpFS.removeFile(name);
			}
		}

		for (cat = 0; cat < DescFile.getCategories().categoryCount();)
		{
			const CBNPCategory &bnpCat = DescFile.getCategories().getCategory(cat);

			if (std::find(forceRemovePatchCategories.begin(), forceRemovePatchCategories.end(),
				bnpCat.getName()) != forceRemovePatchCategories.end())
			{
				for (uint file = 0; file < bnpCat.fileCount(); ++file)
				{
					std::string fileName = bnpCat.getFile(file);
					bnpFS.removeFile(fileName);
				}
				const_cast<CBNPCategorySet &>(DescFile.getCategories()).deleteCategory(cat);
			}
			else
			{
				++cat;
			}
		}
	}
}

// ****************************************************************************
void CPatchManager::getServerFile (const std::string &name, bool bZipped, const std::string& specifyDestName, NLMISC::IProgressCallback *progress)
{
	string srcName = name;
	if (bZipped) srcName += ".ngz";

	string dstName;
	if (specifyDestName.empty())
	{
		dstName = ClientPatchPath + NLMISC::CFile::getFilename(name);
	}
	else
	{
		dstName = specifyDestName;
	}
	if (bZipped) dstName += ".ngz";

	bool	downloadSuccess = false;

	while (!downloadSuccess)
	{
		std::string	serverPath;
		std::string	serverDisplayPath;

		if (UsedServer >= 0 && !PatchServers.empty())
		{
			// first use main patch servers
			serverPath = PatchServers[UsedServer].ServerPath;
			serverDisplayPath = PatchServers[UsedServer].DisplayedServerPath;
		}
		else
		{
			// else use alternative emergency patch server
			serverPath = ServerPath;
			serverDisplayPath = DisplayedServerPath;
			UsedServer = -1;
		}

		try
		{
			string s = CI18N::get("uiLoginGetFile") + " " + NLMISC::CFile::getFilename(srcName);
			setState(true, s);

			// get the new file
			downloadFile (serverPath+srcName, dstName, progress);

			downloadSuccess = true;
		}
		catch (const EPatchDownloadException& e)
		{
			//nlwarning("EXCEPTION CATCH: getServerFile() failed - try to find an alternative: %i: %s",UsedServer,PatchServers[UsedServer].DisplayedServerPath.c_str());

			nlwarning("EXCEPTION CATCH: getServerFile() failed - try to find an alternative : %s", (serverPath+srcName).c_str());
			nlwarning("%i", UsedServer);
			if (UsedServer >= 0 && UsedServer < (int) PatchServers.size())
			{
				nlwarning("%s", PatchServers[UsedServer].DisplayedServerPath.c_str());
			}

			// if emergency patch server, this is a real issue, rethrow exception
			if (UsedServer < 0)
			{
				string s = CI18N::get("uiDLFailed");
				setState(true, s);

				throw Exception(e.what());
			}

			string s = CI18N::get("uiDLURIFailed") + " " + serverDisplayPath;
			setState(true, s);

			// this server is unavailable
			PatchServers[UsedServer].Available = false;

			sint	nextServer = (UsedServer+1) % PatchServers.size();

			while (nextServer != UsedServer && !PatchServers[nextServer].Available)
				nextServer = (nextServer+1) % PatchServers.size();

			// scanned all servers? use alternative
			if (nextServer == UsedServer)
			{
				string s = CI18N::get("uiNoMoreURI");
				setState(true, s);
				UsedServer = -1;
				nlwarning("EXCEPTION CATCH: getServerFile() failed - no alternative found");
			}
			else
			{
				UsedServer = nextServer;
				nlwarning("EXCEPTION CATCH: getServerFile() failed - trying server: %i: %s",UsedServer,PatchServers[UsedServer].DisplayedServerPath.c_str());
			}
		}
	}

	// decompress it
	if (bZipped)
		decompressFile (dstName);
}

// ****************************************************************************
void CPatchManager::downloadFileWithCurl (const string &source, const string &dest, NLMISC::IProgressCallback *progress)
{
	DownloadInProgress = true;
	try
	{
#ifdef USE_CURL
		string s = CI18N::get("uiDLWithCurl") + " " + CFile::getFilename(dest);
		setState(true, s);

		// user agent = nel_launcher

		CURL *curl;
		CURLcode res;

		string sTranslate = CI18N::get("uiLoginGetFile") + " " + NLMISC::CFile::getFilename (source);
		setState(true, sTranslate);
		CurrentFile = NLMISC::CFile::getFilename (source);

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		if(curl == NULL)
		{
			// file not found, delete local file
			throw Exception ("curl init failed");
		}

		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadProgressFunc);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void *) progress);
		curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
		if (source.length() > 8 && (source[4] == 's' || source[4] == 'S')) // 01234 https
		{
			NLWEB::CCurlCertificates::addCertificateFile("cacert.pem");
			NLWEB::CCurlCertificates::useCertificates(curl);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		}

		// create the local file
		if (NLMISC::CFile::fileExists(dest))
		{
			setRWAccess(dest, false);
			NLMISC::CFile::deleteFile(dest.c_str());
		}

		FILE *fp = nlfopen (dest, "wb");

		if (fp == NULL)
		{
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
			throw Exception ("Can't open file '%s' for writing: code=%d %s (error code 37)", dest.c_str (), errno, strerror(errno));
		}

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

		//CurrentFilesToGet++;

		res = curl_easy_perform(curl);

		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);

		long r;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r);

		curl_easy_cleanup(curl);

		bool diskFull = ferror(fp) && errno == 28 /*ENOSPC*/;

		fclose(fp);
		curl_global_cleanup();

		CurrentFile.clear();

		if (diskFull)
		{
			NLMISC::CFile::deleteFile(dest.c_str());
			throw NLMISC::EDiskFullError(dest);
		}

		if(CURLE_WRITE_ERROR == res)
		{
			// file not found, delete local file
			NLMISC::CFile::deleteFile(dest.c_str());
			throw NLMISC::EWriteError(dest);
		}

		if(CURLE_FTP_COULDNT_RETR_FILE == res)
		{
			// file not found, delete local file
			NLMISC::CFile::deleteFile(dest.c_str());
			throw EPatchDownloadException (NLMISC::toString("curl download failed: (ec %d %d)", res, r));
		}

		if(CURLE_OK != res)
		{
			NLMISC::CFile::deleteFile(dest.c_str());
			throw EPatchDownloadException (NLMISC::toString("curl download failed: (ec %d %d)", res, r));
		}

		if(r == 404)
		{
			// file not found, delete it
			NLMISC::CFile::deleteFile(dest.c_str());
			throw EPatchDownloadException (NLMISC::toString("curl download failed: (ec %d %d)", res, r));
		}

		if(r < 200 || r >= 300)
		{
			// file not found, delete it
			NLMISC::CFile::deleteFile(dest.c_str());
			throw EPatchDownloadException (NLMISC::toString("curl download failed: (ec %d %d)", res, r));
		}

#else
		throw Exception("USE_CURL is not defined, no curl method");
#endif
	}
	catch(...)
	{
		DownloadInProgress = false;
		throw;
	}
	if (progress)
	{
		// set progress to 1
		validateProgress((void *) progress, 1, 1, 0, 0);
	}
	DownloadInProgress = false;
}

// ****************************************************************************
void CPatchManager::downloadFile (const string &source, const string &dest, NLMISC::IProgressCallback *progress)
{
	// For the moment use only curl
	const string sourceLower = toLowerAscii(source.substr(0, 6));

	if (startsWith(sourceLower, "http:")
		|| startsWith(sourceLower, "https:")
		|| startsWith(sourceLower, "ftp:")
		|| startsWith(sourceLower, "file:"))
	{
		nldebug("Download patch file %s", source.c_str());
		downloadFileWithCurl(source, dest, progress);
	}
	else
	{
		if (!NLMISC::CFile::copyFile(dest, source, false, progress))
		{
			if (errno == 28)
			{
				throw NLMISC::EDiskFullError(dest);
			}
			throw Exception ("cannot copy file %s to %s", source.c_str(), dest.c_str());
		}
	}
}

// ****************************************************************************
// TODO : Review this uncompress routine to uncompress in a temp file before overwriting destination file

void CPatchManager::decompressFile (const string &filename)
{
	string sTranslate = CI18N::get("uiDecompressing") + " " + NLMISC::CFile::getFilename(filename);
	setState(true, sTranslate);

	//if(isVerboseLog()) nlinfo("Calling gzopen('%s','rb')", filename.c_str());
	gzFile gz = gzopen (filename.c_str (), "rb");
	if (gz == NULL)
	{
		string err = toString("Can't open compressed file '%s' : ", filename.c_str());
		if(errno == 0)
		{
			// gzerror
			int gzerrno;
			const char *gzerr = gzerror (gz, &gzerrno);
			err += toString("code=%d %s", gzerrno, gzerr);
		}
		else
		{
			err += toString("code=%d %s", errno, strerror (errno));
		}
		err += " (error code 31)";
		deleteFile (filename);
		throw Exception (err);
	}

	string dest = filename.substr(0, filename.size ()-4);
	setRWAccess(dest, false);
	//if(isVerboseLog()) nlinfo("Calling nlfopen('%s','wb')", dest.c_str());
	FILE *fp = nlfopen (dest, "wb");
	if (fp == NULL)
	{
		string err = toString("Can't open file '%s' : code=%d %s, (error code 32)", dest.c_str(), errno, strerror(errno));

		gzclose(gz);
		deleteFile (filename);
		throw Exception (err);
	}

	//if(isVerboseLog()) nlinfo("Entering the while loop decompression");

	uint32 currentSize = 0;
	uint8 buffer[10000];
	while (!gzeof(gz))
	{
		//if(isVerboseLog()) nlinfo("Calling gzread");
		int res = gzread (gz, buffer, 10000);
		//if(isVerboseLog()) nlinfo("gzread returns %d", res);
		if (res == -1)
		{
			int gzerrno;
			const char *gzerr = gzerror (gz, &gzerrno);
			gzclose(gz);
			fclose(fp);
			//deleteFile (filename);
			throw Exception ("Can't read compressed file '%s' (after %d bytes) : code=%d %s, (error code 33)", filename.c_str(), currentSize, gzerrno, gzerr);
		}

		currentSize += res;

		//if(isVerboseLog()) nlinfo("Calling fwrite for %d bytes", res);
		int res2 = (int)fwrite (buffer, 1, res, fp);
		//if(isVerboseLog()) nlinfo("fwrite returns %d", res2);
		if (res2 != res)
		{
			bool diskFull = (errno == 28 /* ENOSPC */);
			string err = toString("Can't write file '%s' : code=%d %s (error code 34)", dest.c_str(), errno, strerror(errno));

			gzclose(gz);
			fclose(fp);
			deleteFile (filename);
			if (diskFull)
			{
				throw NLMISC::EDiskFullError(err);
			}
			else
			{
				throw Exception (err);
			}
		}
	}

	//if(isVerboseLog()) nlinfo("Exiting the while loop decompression");

	//if(isVerboseLog()) nlinfo("Calling gzclose");
	gzclose(gz);
	//if(isVerboseLog()) nlinfo("Calling fclose");
	fclose(fp);
	deleteFile(filename);

	//if(isVerboseLog()) nlinfo("Exiting the decompressing file");
}

// ****************************************************************************
void CPatchManager::applyDate (const string &sFilename, uint32 nDate)
{
	// change the file time
	if(nDate != 0)
	{
		setRWAccess(sFilename, false);
		string s = CI18N::get("uiChangeDate") + " " + NLMISC::CFile::getFilename(sFilename) + " " + timestampToHumanReadable(NLMISC::CFile::getFileModificationDate (sFilename)) +
						" -> " + timestampToHumanReadable(nDate);
		setState(true,s);

		if (!NLMISC::CFile::setFileModificationDate(sFilename, nDate))
		{
			int err = NLMISC::getLastError();
			s = CI18N::get("uiChgDateErr") + " " + CFile::getFilename(sFilename) + " (" + toString(err) + ", " + formatErrorMessage(err) + ")";
			setState(true,s);
		}
		s = CI18N::get("uiNowDate") + " " + CFile::getFilename(sFilename) + " " + timestampToHumanReadable(NLMISC::CFile::getFileModificationDate (sFilename));
		setState(true,s);
	}
}

// ****************************************************************************
// Get all the patches that need to be applied to a file from the description of this file given by the server
void CPatchManager::getPatchFromDesc(SFileToPatch &ftpOut, const CBNPFile &fIn, bool forceCheckSumTest)
{
	uint32 j;
	const CBNPFile rFile = fIn;
	const string &rFilename = rFile.getFileName();
	string sFilePath;
	bool inPatchDir = false;
	// first see if there's a version that was downloaded from the background downloader
	if (NLMISC::CFile::fileExists(ClientPatchPath + rFilename + ".tmp"))
	{
		sFilePath = ClientPatchPath + rFilename + ".tmp";
		ftpOut.SrcFileName = sFilePath;
		inPatchDir = true;
	}

	bool needUnpack = false;
	const CBNPCategory *cat = DescFile.getCategories().getCategoryFromFile(rFilename);
	if (cat)
	{
		needUnpack = !cat->getUnpackTo().empty();
	}
	else
	{
		nlwarning("Can't find file category for %s:", rFilename.c_str());
	}

	// Does the BNP exists ?
	// following lines added by Sadge to ensure that the correct file is patched
	// Only look in data path if the file should not be unpack (otherwise it should only remains in the "unpack" directory)
	if (!needUnpack)
	{
		if (sFilePath.empty())
		{
			if (NLMISC::CFile::fileExists(WritableClientDataPath + rFilename))
			{
				// if file exists in writable directory, use it
				sFilePath = WritableClientDataPath + rFilename;
			}
			else if (NLMISC::CFile::fileExists(ReadableClientDataPath + rFilename))
			{
				// if file exists in readable directory, use it
				sFilePath = ReadableClientDataPath + rFilename;
			}
		}
	}

	if (sFilePath.empty() && NLMISC::CFile::fileExists(ClientPatchPath + rFilename))
	{
		sFilePath = ClientPatchPath + rFilename;
	}

// following lines removed by Sadge to ensure that the correct file is patched
//	string sFilePath = CPath::lookup(rFilename, false, false);
//	if (sFilePath.empty())
//	{
//		if (NLMISC::CFile::fileExists(ClientPatchPath + rFilename))
//			sFilePath = ClientPatchPath + rFilename;
//	}

	// if file not found anywhere
	if (sFilePath.empty())
	{
		ftpOut.FileName = rFilename;
		ftpOut.LocalFileToDelete = false;
		ftpOut.LocalFileExists = false;
		// It happens some time (maybe a bug) that the versionCount is 0... =>
		// it happens if the BNP file is empty (8 bytes)
		ftpOut.FinalFileSize = EmptyBnpFileSize;
		// BNP does not exists : get all the patches version
		for (j = 0; j < rFile.versionCount(); ++j)
		{
			ftpOut.Patches.push_back(rFile.getVersion(j).getVersionNumber());
			ftpOut.PatcheSizes.push_back(rFile.getVersion(j).getPatchSize());
			ftpOut.LastFileDate = rFile.getVersion(j).getTimeStamp();
			ftpOut.FinalFileSize = rFile.getVersion(j).getFileSize();
			ftpOut.SZFileSize = rFile.getVersion(j).get7ZFileSize();
		}
	}
	else
	{
		// The local BNP file exists : find its version
		uint32 nLocalSize = NLMISC::CFile::getFileSize(sFilePath);
		uint32 nLocalTime = NLMISC::CFile::getFileModificationDate(sFilePath);
		// From the couple time, size look the version of the file
		uint32 nVersionFound = 0xFFFFFFFF;
		// If forceChecksum is wanted (slow), then don't do the test with filesize/date
		if(!forceCheckSumTest)
		{
			for (j = 0; j < rFile.versionCount(); ++j)
			{
				const CBNPFileVersion &rVersion = rFile.getVersion(j);
				uint32 nServerSize = rVersion.getFileSize();
				uint32 nServerTime = rVersion.getTimeStamp();
				// Does the time and size match a version ?
				if ((nServerSize == nLocalSize) && (abs((sint32)(nServerTime - nLocalTime)) <= 2) )
				{
					nVersionFound = rVersion.getVersionNumber();
					// break; // ace -> get the last good version (if more than one version of the same file exists)
				}
			}
		}

		// If the version cannot be found with size and time try with sha1
		if (nVersionFound == 0xFFFFFFFF)
		{
			string sTranslate = CI18N::get("uiCheckInt") + " " + rFilename;
			setState(true, sTranslate);
			CHashKey hkLocalSHA1 = getSHA1(sFilePath);
			for (j = 0; j < rFile.versionCount(); ++j)
			{
				const CBNPFileVersion &rVersion = rFile.getVersion(j);
				CHashKey hkServerSHA1 = rVersion.getHashKey();
				// Does the sha1 match a version ?
				if (hkServerSHA1 == hkLocalSHA1)
				{
					nVersionFound = rVersion.getVersionNumber();
					applyDate(sFilePath, rVersion.getTimeStamp());
					// break; // ace -> same as above
				}
			}
		}

		// No version available found
		if (nVersionFound == 0xFFFFFFFF)
		{
			string sTranslate = CI18N::get("uiNoVersionFound");
			setState(true, sTranslate);
			// Get all patches from beginning (first patch is reference file)
			ftpOut.FileName = rFilename;
			ftpOut.LocalFileToDelete = true;
			ftpOut.LocalFileExists = !inPatchDir;
			// It happens some time (maybe a bug) that the versionCount is 0... =>
			// it happens if the BNP file is empty (8 bytes)
			ftpOut.FinalFileSize = EmptyBnpFileSize;
			// Get all the patches version
			for (j = 0; j < rFile.versionCount(); ++j)
			{
				ftpOut.Patches.push_back(rFile.getVersion(j).getVersionNumber());
				ftpOut.PatcheSizes.push_back(rFile.getVersion(j).getPatchSize());
				ftpOut.LastFileDate = rFile.getVersion(j).getTimeStamp();
				ftpOut.FinalFileSize = rFile.getVersion(j).getFileSize();
				ftpOut.SZFileSize = rFile.getVersion(j).get7ZFileSize();
			}
		}
		else // A version of the file has been found
		{
			string sTranslate = CI18N::get("uiVersionFound") + " " + toString(nVersionFound);
			setState(true, sTranslate);
			// Get All patches from this version !
			ftpOut.FileName = rFilename;
			ftpOut.LocalFileToDelete = false;
			ftpOut.LocalFileExists = !inPatchDir;
			// Go to the version
			for (j = 0; j < rFile.versionCount(); ++j)
				if (rFile.getVersion(j).getVersionNumber() == nVersionFound)
					break;

			nlassert(j != rFile.versionCount()); // Not normal if we cant find the version we found previously

			// Point on the next version
			j++;
			// If there are newer versions
			if (j != rFile.versionCount())
			{
				// Add all version until the last one
				for (; j < rFile.versionCount(); ++j)
				{
					ftpOut.Patches.push_back(rFile.getVersion(j).getVersionNumber());
					ftpOut.PatcheSizes.push_back(rFile.getVersion(j).getPatchSize());
					ftpOut.LastFileDate = rFile.getVersion(j).getTimeStamp();
					ftpOut.SZFileSize = rFile.getVersion(j).get7ZFileSize();
				}
			}
			// Else this file is up to date !

			// For info, get its final file size
			ftpOut.FinalFileSize= rFile.getVersion(rFile.versionCount()-1).getFileSize();
		}
	} // end of else local BNP file exists
}

// ****************************************************************************
bool CPatchManager::bnpUnpack(const string &srcBigfile, const string &dstPath, vector<string> &vFilenames)
{
	nldebug("bnpUnpack: srcBigfile=%s", srcBigfile.c_str());
	string SourceName, DestPath;

	// following lines added by Sadge to ensure that the correct file gets patched
//	if		(NLMISC::CFile::fileExists(ClientDataPath + srcBigfile))	SourceName = ClientDataPath + srcBigfile;
//	else if (NLMISC::CFile::fileExists(srcBigfile))						SourceName = ClientDataPath + srcBigfile;
	SourceName= srcBigfile;

// following lines removed by Sadge to ensure that the correct file gets patched
//	SourceName = CPath::lookup(srcBigfile, false, false);
//	if (SourceName.empty())
//		SourceName = ClientPatchPath + srcBigfile;

	if (dstPath.empty())
		DestPath = ClientRootPath;
	else
		DestPath = CPath::standardizePath (dstPath);

	string s = CI18N::get("uiUnpack") + " " + NLMISC::CFile::getFilename(SourceName);
	setState(true,s);

	// Read Header of the BNP File
	CBigFile::BNP bnpFile;
	bnpFile.BigFileName = SourceName;

	if (!bnpFile.readHeader())
	{
		string s = CI18N::get("uiUnpackErrHead") + " " + CFile::getFilename(SourceName);
		setState(true,s);
		return false;
	}

	// Unpack
	if (!bnpFile.unpack(DestPath))
		return false;

	// Return names
	{
		vFilenames.clear();
		for (uint32 i = 0; i < bnpFile.SFiles.size(); ++i)
			vFilenames.push_back(bnpFile.SFiles[i].Name);
	}

	return true;
}


// ****************************************************************************
int CPatchManager::downloadProgressFunc(void *foo, double t, double d, double ultotal, double ulnow)
{
	if (d != t)
	{
		// In the case of progress = 1, don't update because, this will be called in case of error to signal the end of the download, though
		// no download actually occurred. Instead, we set progress to 1.f at the end of downloadWithCurl if everything went fine
		return validateProgress(foo, t, d, ultotal, ulnow);
	}
	return 0;
}

// ****************************************************************************
int CPatchManager::validateProgress(void *foo, double t, double d, double /* ultotal */, double /* ulnow */)
{
	static std::vector<std::string> units;

	if (units.empty())
	{
		units.push_back(CI18N::get("uiByte"));
		units.push_back(CI18N::get("uiKb"));
		units.push_back(CI18N::get("uiMb"));
	}

	CPatchManager *pPM = CPatchManager::getInstance();
	double pour1 = t!=0.0?d*100.0/t:0.0;
	string sTranslate = CI18N::get("uiLoginGetFile") + toString(" %s : %s / %s (%.02f %%)", NLMISC::CFile::getFilename(pPM->CurrentFile).c_str(),
		NLMISC::bytesToHumanReadableUnits((uint64)d, units).c_str(), NLMISC::bytesToHumanReadableUnits((uint64)t, units).c_str(), pour1);
	pPM->setState(false, sTranslate);
	if (foo)
	{
		((NLMISC::IProgressCallback *) foo)->progress((float) (t != 0 ? d / t : 0));
	}
	return 0;
}

// ****************************************************************************
void CPatchManager::MyPatchingCB::progress(float f)
{
	CPatchManager *pPM = CPatchManager::getInstance();
	double p = 100.0*f;
	string sTranslate = CI18N::get("uiApplyingDelta") + toString(" %s (%.02f %%)", CFile::getFilename(patchFilename).c_str(), p);
	pPM->setState(false, sTranslate);
}

// ***************************************************************************
void CPatchManager::startScanDataThread()
{
	if (ScanDataThread != NULL)
	{
		nlwarning ("scan data thread is already running");
		return;
	}
	if (Thread != NULL)
	{
		nlwarning ("a thread is already running");
		return;
	}

	_ErrorMessage.clear();

	// Reset result
	clearDataScanLog();

	// Read now the client version and Desc File.
	readClientVersionAndDescFile();

	// start thread
	ScanDataThread = new CScanDataThread();
	nlassert (ScanDataThread != NULL);

	Thread = IThread::create (ScanDataThread);
	nlassert (Thread != NULL);
	Thread->start ();
}

// ****************************************************************************
bool CPatchManager::isScanDataThreadEnded(bool &ok)
{
	if (ScanDataThread == NULL)
	{
		ok = false;
		return true;
	}

	bool end = ScanDataThread->Ended;
	if (end)
	{
		ok = ScanDataThread->CheckOk;
		stopScanDataThread();
	}

	return end;
}

// ****************************************************************************
void CPatchManager::stopScanDataThread()
{
	if(ScanDataThread && Thread)
	{
		Thread->wait();
		delete Thread;
		Thread = NULL;
		delete ScanDataThread;
		ScanDataThread = NULL;
	}
}

// ***************************************************************************
void CPatchManager::askForStopScanDataThread()
{
	if(!ScanDataThread)
		return;

	ScanDataThread->AskForCancel= true;
}

// ***************************************************************************
uint CPatchManager::applyScanDataResult()
{
	// if still running, abort
	if(ScanDataThread)
		return 0;

	uint numError= 0;

	{
		TSyncDataScanState::CAccessor	ac(&DataScanState);
		CDataScanState	&val= ac.value();
		numError= (uint)val.FilesWithScanDataError.size();

		// Touch the files with error (will be reloaded at next patch)
		for(uint i=0;i<numError;i++)
		{
			SFileToPatch	&ftp= val.FilesWithScanDataError[i];

			// if the file was not found (just loggued for information)
			if(!ftp.LocalFileExists)
				continue;

			// get file path
			// following lines added by Sadge to ensure that the correct file gets patched
			string sFilePath;
			if (NLMISC::CFile::fileExists(WritableClientDataPath + ftp.FileName)) sFilePath = WritableClientDataPath + ftp.FileName;
			if (sFilePath.empty() && NLMISC::CFile::fileExists(ReadableClientDataPath + ftp.FileName)) sFilePath = ReadableClientDataPath + ftp.FileName;
			if (sFilePath.empty() && NLMISC::CFile::fileExists(ClientPatchPath + ftp.FileName))	sFilePath = ClientPatchPath + ftp.FileName;

// following lines removed by Sadge to ensure that the correct file gets patched
//			string sFilePath = CPath::lookup(ftp.FileName, false, false);
//			if (sFilePath.empty())
//			{
//				if (NLMISC::CFile::fileExists(ClientPatchPath + ftp.FileName))
//					sFilePath = ClientPatchPath + ftp.FileName;
//			}

			// Reset to a dummy date, so the patch will be fully/checked/patched next time
			if(!sFilePath.empty())
				applyDate(sFilePath, DefaultResetDate);
			else
				nlwarning("File '%s' Not Found. should exist...", ftp.FileName.c_str());
		}
	}

	return numError;
}

// ***************************************************************************
bool CPatchManager::getDataScanLog(string &text)
{
	text.clear();
	bool	changed= false;
	{
		TSyncDataScanState::CAccessor	ac(&DataScanState);
		CDataScanState	&val= ac.value();
		changed= val.Changed;
		// if changed, build the log
		if(changed)
		{
			for(uint i=0;i<val.FilesWithScanDataError.size();i++)
			{
				string	str;
				getCorruptedFileInfo(val.FilesWithScanDataError[i], str);
				text+= str + "\n";
			}
		}
		// then reset
		val.Changed= false;
	}

	return changed;
}

// ***************************************************************************
void CPatchManager::addDataScanLogCorruptedFile(const SFileToPatch &ftp)
{
	{
		TSyncDataScanState::CAccessor	ac(&DataScanState);
		CDataScanState	&val= ac.value();
		val.FilesWithScanDataError.push_back(ftp);
		val.Changed= true;
	}
}

// ***************************************************************************
void CPatchManager::clearDataScanLog()
{
	{
		TSyncDataScanState::CAccessor	ac(&DataScanState);
		CDataScanState	&val= ac.value();
		val.FilesWithScanDataError.clear();
		val.Changed= true;
	}
}

// ***************************************************************************
void CPatchManager::getCorruptedFileInfo(const SFileToPatch &ftp, string &sTranslate)
{
	sTranslate = CI18N::get("uiCorruptedFile") + " " + ftp.FileName + " (" +
		toString("%.1f ", (float)ftp.FinalFileSize/1000000.f) + CI18N::get("uiMb") + ")";
}


// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// CCheckThread
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
CCheckThread::CCheckThread(bool includeBackgroundPatch)
{
	Ended = false;
	CheckOk = false;
	TotalFileToCheck = 0;
	CurrentFileChecked = 0;
	IncludeBackgroundPatch = includeBackgroundPatch;
	StopAsked = false;
}

// ****************************************************************************
void CCheckThread::run ()
{
	nlwarning("CCheckThread::start");
	StopAsked = false;
	CPatchManager *pPM = CPatchManager::getInstance();
	pPM->MustLaunchBatFile = false;
	try
	{
		nlwarning("CCheckThread : get version");
		uint32 i, j, k;
		// Check if the client version is the same as the server version
		string sClientVersion = pPM->getClientVersion();
		string sServerVersion = pPM->getServerVersion();
		string sTranslate = CI18N::get("uiClientVersion") + " (" + sClientVersion + ") ";
		sTranslate += CI18N::get("uiServerVersion") + " (" + sServerVersion + ")";
		pPM->setState(true, sTranslate);

		if (sServerVersion.empty())
		{
			// No need to patch
			CheckOk = true;
			Ended = true;
			return;
		}

		sint32 nServerVersion, nClientVersion;
		fromString(sServerVersion, nServerVersion);
		fromString(sClientVersion, nClientVersion);

		if (nClientVersion != nServerVersion)
		{
			// first, try in the version subdirectory
			try
			{
				pPM->DescFilename = toString("%05d/ryzom_%05d.idx", nServerVersion, nServerVersion);
				// The client version is different from the server version : download new description file
				pPM->getServerFile(pPM->DescFilename, false); // For the moment description file is not zipped
			}
			catch (...)
			{
				// fallback to patch root directory
				pPM->DescFilename = toString("ryzom_%05d.idx", nServerVersion);
				// The client version is different from the server version : download new description file
				pPM->getServerFile(pPM->DescFilename, false); // For the moment description file is not zipped
			}
		}

		nlwarning("CCheckThread : read description files");

		// Read the description file
		pPM->readDescFile(nServerVersion);

		nlwarning("CCheckThread : check files");

		// For all bnp in the description file get all patches to apply
		// depending on the version of the client bnp files
		const CBNPFileSet &rDescFiles = pPM->DescFile.getFiles();
		pPM->FilesToPatch.clear();
		TotalFileToCheck = rDescFiles.fileCount();
		for (i = 0; i < rDescFiles.fileCount(); ++i)
		{
			CPatchManager::SFileToPatch ftp;
			sTranslate = CI18N::get("uiCheckingFile") + " " + rDescFiles.getFile(i).getFileName();
			pPM->setState(true, sTranslate);
			// get list of patch to apply to this file. don't to a full checksum test if possible
			nlwarning(rDescFiles.getFile(i).getFileName().c_str());
			pPM->getPatchFromDesc(ftp, rDescFiles.getFile(i), false);
			// add the file if there are some patches to apply, or if an already patched version was found in the unpack directory
			if (!ftp.Patches.empty() || (IncludeBackgroundPatch && !ftp.SrcFileName.empty()))
			{
				pPM->FilesToPatch.push_back(ftp);
				sTranslate = CI18N::get("uiNeededPatches") + " " + toString (ftp.Patches.size());
				pPM->setState(true, sTranslate);
			}
			CurrentFileChecked = i;
		}

		// Here we got all the files to patch in FilesToPatch and all the versions that must be obtained
		// Now we have to get the optional categories
		const CBNPCategorySet &rDescCats = pPM->DescFile.getCategories();
		pPM->OptionalCat.clear();
		for (i = 0; i < rDescCats.categoryCount(); ++i)
		{
			// For all optional categories check if there is a 'file to patch' in it
			const CBNPCategory &rCat = rDescCats.getCategory(i);
			if (rCat.isOptional())
			for (j = 0; j < rCat.fileCount(); ++j)
			{
				const string &rFilename = rCat.getFile(j);
				bool bAdded = false;
				for (k = 0; k < pPM->FilesToPatch.size(); ++k)
				{
					if (stricmp(pPM->FilesToPatch[k].FileName.c_str(), rFilename.c_str()) == 0)
					{
						pPM->OptionalCat.push_back(rCat.getName());
						bAdded = true;
						break;
					}
				}
				if (bAdded)
					break;
			}
		}

		// For all categories that required an optional category if the cat required is present the category that
		// reference it must be present
		for (i = 0; i < rDescCats.categoryCount(); ++i)
		{
			// For all optional categories check if there is a 'file to patch' in it
			const CBNPCategory &rCat = rDescCats.getCategory(i);
			if (rCat.isOptional() && !rCat.getCatRequired().empty())
			{
				// Does the rCat is already present ?
				bool bFound = false;
				for (j = 0; j < pPM->OptionalCat.size(); ++j)
				{
					if (rCat.getName() == pPM->OptionalCat[j])
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
				{
					// rCat is not present but perhaps its required cat is present
					const string &sCatReq = rCat.getCatRequired();
					for (j = 0; j < pPM->OptionalCat.size(); ++j)
					{
						if (sCatReq == pPM->OptionalCat[j])
						{
							// Required Cat present -> Add the category rCat
							pPM->OptionalCat.push_back(rCat.getName());
							break;
						}
					}
				}
			}
		}


		// Delete categories optional cat that are hidden
		for (i = 0; i < rDescCats.categoryCount(); ++i)
		{
			// For all optional categories check if there is a 'file to patch' in it
			const CBNPCategory &rCat = rDescCats.getCategory(i);
			if (rCat.isOptional() && rCat.isHidden())
			{
				// Does the rCat is present ?
				for (j = 0; j < pPM->OptionalCat.size(); ++j)
				{
					if (rCat.getName() == pPM->OptionalCat[j])
					{
						// remove it
						/*#ifdef RY_BG_DOWNLOADER
							// fixme : strange stlport link error for the background downloader when using erase ...
							std::copy(pPM->OptionalCat.begin() + j + 1, pPM->OptionalCat.end(), pPM->OptionalCat.begin()+j);
							pPM->OptionalCat.resize(pPM->OptionalCat.size() - 1);
						#else*/
							pPM->OptionalCat.erase(pPM->OptionalCat.begin()+j);
						//#endif
						break;
					}
				}
			}
		}

		// Get all extract to category and check files inside the bnp with real files
		for (i = 0; i < rDescCats.categoryCount(); ++i)
		{
			// For all optional categories check if there is a 'file to patch' in it
			const CBNPCategory &rCat = rDescCats.getCategory(i);
			if (!rCat.getUnpackTo().empty())
			{
				for (j = 0; j < rCat.fileCount(); ++j)
				{
					string sBNPFilename = pPM->ClientPatchPath + rCat.getFile(j);

					sTranslate = CI18N::get("uiCheckInBNP") + " " + rCat.getFile(j);
					pPM->setState(true, sTranslate);
					CBigFile::BNP bnpFile;
					bnpFile.BigFileName = sBNPFilename;

					if (bnpFile.readHeader())
					{
						// read the file inside the bnp and calculate the sha1
						FILE *bnp = nlfopen (sBNPFilename, "rb");
						if (bnp != NULL)
						{
							for (uint32 k = 0; k < bnpFile.SFiles.size(); ++k)
							{
								const CBigFile::SBNPFile &rBNPFile = bnpFile.SFiles[k];
								// Is the real file exists ?
								string sRealFilename = rCat.getUnpackTo() + rBNPFile.Name;
								if (NLMISC::CFile::fileExists(sRealFilename))
								{
									// Yes compare the sha1 with the sha1 of the BNP File
									CHashKey sha1BNPFile;
									nlfseek64 (bnp, rBNPFile.Pos, SEEK_SET);
									uint8 *pPtr = new uint8[rBNPFile.Size];
									if (fread (pPtr, rBNPFile.Size, 1, bnp) != 1)
									{
										delete [] pPtr;
										break;
									}

									sha1BNPFile = getSHA1(pPtr, rBNPFile.Size);
									delete [] pPtr;
									CHashKey sha1RealFile = getSHA1(sRealFilename, true);
									if ( ! (sha1RealFile == sha1BNPFile))
									{
										sTranslate = CI18N::get("uiSHA1Diff") + " " + rBNPFile.Name;
										pPM->setState(true, sTranslate);
										pPM->MustLaunchBatFile = true;
									}
								}
								else
								{
									// File dest do not exist
									sTranslate = CI18N::get("uiSHA1Diff") + " " + rBNPFile.Name;
									pPM->setState(true, sTranslate);
									pPM->MustLaunchBatFile = true;
								}

							}
							fclose (bnp);
						}
						if (StopAsked)
						{
							StopAsked = false;
							return;
						}
					}
				}
			}
		}

		sTranslate = CI18N::get("uiCheckEndNoErr");
		pPM->setState(true, sTranslate);
		CheckOk = true;
		Ended = true;
	}
	catch (const NLMISC::EDiskFullError &)
	{
		// more explicit message for this common error case
		nlwarning("EXCEPTION CATCH: disk full");
		pPM->setState(true, CI18N::get("uiCheckEndWithErr"));
		pPM->setErrorMessage(CI18N::get("uiPatchDiskFull"));
		CheckOk = false;
		Ended = true;
	}
	catch (const Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CCheckThread::run() failed");
		string sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
		pPM->setState(true, CI18N::get("uiCheckEndWithErr"));
		pPM->setErrorMessage(sTranslate);
		CheckOk = false;
		Ended = true;
	}
}

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// CPatchThread
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
CPatchThread::CPatchThread(bool commitPatch)
{
	Ended = false;
	PatchOk = false;
	CurrentFilePatched = 0;
	PatchSizeProgress = 0;
	_CommitPatch = commitPatch;
	StopAsked = false;
}

// ****************************************************************************
void CPatchThread::clear()
{
	AllFilesToPatch.clear();
}

// ****************************************************************************
// trap : optimize this if needed
void CPatchThread::add(const CPatchManager::SFileToPatch &ftp)
{
	for (uint32 i = 0; i < AllFilesToPatch.size(); ++i)
	{
		if (AllFilesToPatch[i].FileName == ftp.FileName)
			return;
	}
	AllFilesToPatch.push_back(ftp);
}


// ****************************************************************************
void CPatchThread::run()
{
	StopAsked = false;
	CPatchManager *pPM = CPatchManager::getInstance();
	bool bErr = false;
	uint32 i;

	string sServerVersion = pPM->getServerVersion();
	PatchSizeProgress = 0;

	if (sServerVersion.empty())
	{
		// No need to patch
		PatchOk = true;
		Ended = true;
		return;
	}

	// Patch all the files
	// If at least one file has been patched relaunch the client

	CurrentFilePatched = 0.f;

	string sTranslate;
	try
	{
		// First do all ref files
		// ----------------------

		for (i = 0; i < AllFilesToPatch.size(); ++i)
		{
			CPatchManager::SFileToPatch &rFTP = AllFilesToPatch[i];
			string ext = NLMISC::CFile::getExtension(rFTP.FileName);
			if (ext == "ref")
			{
				float oldCurrentFilePatched = CurrentFilePatched;
				processFile (rFTP);
				pPM->MustLaunchBatFile = true;
				CurrentFilePatched = oldCurrentFilePatched + 1.f;
			}
			if (StopAsked)
			{
				StopAsked = false;
				return;
			}
		}

		// Second do all bnp files
		// -----------------------

		for (i = 0; i < AllFilesToPatch.size(); ++i)
		{
			CPatchManager::SFileToPatch &rFTP = AllFilesToPatch[i];

			string ext = NLMISC::CFile::getExtension(rFTP.FileName);
			if (ext == "bnp" || ext == "snp")
			{
				float oldCurrentFilePatched = CurrentFilePatched;
				processFile (rFTP);
				pPM->MustLaunchBatFile = true;
				CurrentFilePatched = oldCurrentFilePatched + 1.f;
			}
			if (StopAsked)
			{
				StopAsked = false;
				return;
			}
		}

	}
	catch (const NLMISC::EDiskFullError &)
	{
		// more explicit message for this common error case
		nlwarning("EXCEPTION CATCH: CPatchThread::run() Disk Full");
		pPM->setState(true, CI18N::get("uiPatchEndWithErr"));
		sTranslate = CI18N::get("uiPatchDiskFull");
		bErr = true;
	}
	catch(const Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CPatchThread::run() failed");
		pPM->setState(true, string(e.what()));
		sTranslate = CI18N::get("uiPatchEndWithErr");
		bErr = true;
	}


	// Unpack all files with the UnpackTo flag
	// ---------------------------------------
	// To do that we create a batch file that will copy all files (unpacked to patch directory)
	// to the directory we want (the UnpackTo string)

	// Recreate batch file
	if (!pPM->MustLaunchBatFile && !pPM->getAsyncDownloader())
	{
		pPM->deleteFile(pPM->UpdateBatchFilename, false, false);
	}

	if (!bErr)
	{
		sTranslate = CI18N::get("uiPatchEndNoErr");
	}
	else
	{
		// Set a more explicit error message
		pPM->setErrorMessage(sTranslate);
	}

	PatchOk = !bErr;
	Ended = true;
}


class CPatchThreadDownloadProgress : public NLMISC::IProgressCallback
{
public:
	CPatchThread *PatchThread;
	float Scale;
	float Bias;
	uint CurrentFilePatched;
	CPatchThreadDownloadProgress() : PatchThread(NULL),
									 Scale(1.f),
									 Bias(0.f),
									 CurrentFilePatched(0)
	{
	}
	virtual void progress (float progressValue)
	{
		clamp(progressValue, 0.f, 1.f);
		nlassert(PatchThread);
		//PatchThread->CurrentFilePatched = std::max(PatchThread->CurrentFilePatched, (float) this->CurrentFilePatched + Bias + Scale * progressValue);
		PatchThread->CurrentFilePatched = this->CurrentFilePatched + Bias + Scale * progressValue;
		CPatchManager::getInstance()->touchState();
	}
};

void stopSoundMngr();

// ****************************************************************************
void CPatchThread::processFile (CPatchManager::SFileToPatch &rFTP)
{
	CPatchManager *pPM = CPatchManager::getInstance();

	// Source File Name (in writable or readable directory)
	string SourceName;

	// Destination File Name (in writable directory)
	string DestinationName;

	if (NLMISC::startsWith(rFTP.FileName, "sound"))
	{
		// Stop sound playback
		stopSoundMngr();
	}

	if (rFTP.ExtractPath.empty())
	{
		DestinationName = pPM->WritableClientDataPath + rFTP.FileName;

		if (rFTP.LocalFileExists)
		{
			// following lines added by Sadge to ensure that the correct file gets patched
			SourceName.clear();

			if (NLMISC::CFile::fileExists(pPM->WritableClientDataPath + rFTP.FileName))
			{
				SourceName = pPM->WritableClientDataPath + rFTP.FileName;
			}
			else if (NLMISC::CFile::fileExists(pPM->ReadableClientDataPath + rFTP.FileName))
			{
				SourceName = pPM->ReadableClientDataPath + rFTP.FileName;
			}

			// version from previous download
			if (SourceName.empty())	throw Exception (std::string("ERROR: Failed to find file: ")+rFTP.FileName);

// following lines removed by Sadge to ensure that the correct file gets patched
//			SourceName = CPath::lookup(rFTP.FileName); // exception if file do not exists
		}
		else
		{
			// note : if file was background downloaded, we have :
			// rFTP.LocalFileExists = false
			// rFTP.SrcFileName = "unpack/filename.bnp.tmp"
			SourceName = DestinationName;
		}
	}
	else
	{
		SourceName = pPM->ClientPatchPath + rFTP.FileName;
		DestinationName = SourceName;
	}

	if (rFTP.LocalFileToDelete)
	{
		// corrupted or invalid file ? ....
		NLMISC::CFile::deleteFile(SourceName);
		rFTP.LocalFileExists = false;
	}

	string sTranslate;
	sTranslate = CI18N::get("uiProcessing") + " " + rFTP.FileName;
	pPM->setState(true, sTranslate);

	if (pPM->getAsyncDownloader())
	{
		if (!rFTP.ExtractPath.empty())
		{
			std::string info = rFTP.FileName;
		}
		string PatchName = toString("%05d/%s%s", rFTP.Patches[rFTP.Patches.size() - 1], rFTP.FileName.c_str(), ".lzma");
		pPM->getAsyncDownloader()->addToDownloadList(PatchName, SourceName, rFTP.LastFileDate, rFTP.ExtractPath, rFTP.SZFileSize, rFTP.FinalFileSize );
		return;
	}


	std::string tmpSourceName = rFTP.SrcFileName.empty() ? SourceName : rFTP.SrcFileName; // source is same than destination, or possibly
																						 // a patch from a previous background downloader session
	if (rFTP.LocalFileToDelete)
	{
		// corrupted or invalid file ? ....
		rFTP.LocalFileExists = false;
		NLMISC::CFile::deleteFile(tmpSourceName);
	}

	string OutFilename;
	bool usePatchFile = true;
	bool haveAllreadyTryiedDownloadingOfFile = false;

	// compute the total size of patch to download
	uint32 totalPatchSize = 0;
	if (rFTP.Incremental)
	{
		for (uint i=0; i<rFTP.PatcheSizes.size(); ++i)
			totalPatchSize += rFTP.PatcheSizes[i];
	}
	else if (!rFTP.PatcheSizes.empty())
	{
		totalPatchSize = rFTP.PatcheSizes.back();
	}


	CPatchThreadDownloadProgress progress;
	progress.PatchThread = this;
	progress.CurrentFilePatched = (uint) floorf(CurrentFilePatched);

	// look for the source file, if not present or invalid (not matching
	// a known version) or if total patch size greater than half the
	// uncompressed final size or if total patch size is greater
	// than lzma compressed file, then load the lzma file
	if ((rFTP.Incremental && !rFTP.LocalFileExists)		// incremental and no base file, we need a complete file
		|| totalPatchSize > rFTP.FinalFileSize / 2		// patch is too big regarding the final file, patch applying time will be slow
		|| rFTP.SZFileSize < totalPatchSize)			// lzma is smaller than patch !
	{
		breakable
		{
			usePatchFile = false;
			// compute the seven zip filename
			string lzmaFile = rFTP.FileName+".lzma";

			// download the 7zip file
			try
			{
				// first, try in the file version subfolfer
				try
				{
					progress.Scale = 1.f;
					progress.Bias = 0.f;
					if (!rFTP.Patches.empty())
					{
						pPM->getServerFile(toString("%05u/", rFTP.Patches.back())+lzmaFile, false, "", &progress);
					}
					// else -> file comes from a previous download (with .tmp extension, and is up to date)
					// the remaining code will just rename it with good name and exit
				}
				catch (const NLMISC::EWriteError &)
				{
					// this is a local error, rethrow ...
					throw;
				}
				catch(...)
				{
					// failed with version subfolder, try in the root patch directory
					pPM->getServerFile(lzmaFile, false, "", &progress);
				}
			}
			catch (const NLMISC::EWriteError &)
			{
				// this is a local error, rethrow ...
				throw;
			}
			catch (...)
			{
				// can not load the 7zip file, use normal patching
				usePatchFile = true;
				haveAllreadyTryiedDownloadingOfFile = true;
				break;
			}

			OutFilename = pPM->ClientPatchPath + NLMISC::CFile::getFilename(rFTP.FileName);
			// try to unpack the file
			try
			{
				if (!unpackLZMA(pPM->ClientPatchPath+lzmaFile, OutFilename+".tmp"))
				{
					// fallback to standard patch method
					usePatchFile = true;
					haveAllreadyTryiedDownloadingOfFile = true;
					break;
				}
			}
			catch (const NLMISC::EWriteError&)
			{
				throw;
			}
			catch (...)
			{
				nlwarning("Failed to unpack lzma file %s", (pPM->ClientPatchPath+lzmaFile).c_str());
				// fallback to standard patch method
				usePatchFile = true;
				haveAllreadyTryiedDownloadingOfFile = true;
				break;
			}

			if (rFTP.LocalFileExists)
				pPM->deleteFile(SourceName);

			pPM->deleteFile(pPM->ClientPatchPath+lzmaFile);	// delete the archive file
			pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
			if (_CommitPatch)
			{
				pPM->renameFile(OutFilename+".tmp", DestinationName);
			}
		}
	}
	if (usePatchFile)
	{
		uint32 currentPatchedSize = 0;
		for (uint32 j = 0; j < rFTP.Patches.size(); ++j)
		{
			// Get the patch
			// first, try in the patch subdirectory
			string PatchName;
			try
			{
				PatchName = toString("%05d/", rFTP.Patches[j]) + rFTP.FileName.substr(0, rFTP.FileName.size()-4) + toString("_%05d", rFTP.Patches[j]) + ".patch";
				sTranslate = CI18N::get("uiLoginGetFile") + " " + PatchName;
				pPM->setState(true, sTranslate);
				progress.Scale = 1.f;
				progress.Bias = totalPatchSize != 0 ? (float) currentPatchedSize / totalPatchSize : 0.f;
				progress.Scale = totalPatchSize != 0 ? (float) rFTP.PatcheSizes[j] / totalPatchSize : 1.f;
				pPM->getServerFile(PatchName, false, "", &progress);
				// remove the subfolder name
				PatchName = NLMISC::CFile::getFilename(PatchName);
			}
			catch (const NLMISC::EWriteError &)
			{
				throw;
			}
			catch (...)
			{
				// fallback to patch root directory
				PatchName = rFTP.FileName.substr(0, rFTP.FileName.size()-4);
				PatchName += toString("_%05d", rFTP.Patches[j]) + ".patch";
				sTranslate = CI18N::get("uiLoginGetFile") + " " + PatchName;
				pPM->setState(true, sTranslate);
				pPM->getServerFile(PatchName, false, "", &progress);
			}

			// Apply the patch
			// If the patches are not to be applied on the last version
			string SourceNameXD = tmpSourceName;
			if (!rFTP.Incremental)
			{
				// Some note about non incremental patches :
				// Usually, files such as '.exe' or '.dll' do not work well with incremental patch, so it is better
				// to apply a single patch from a refrence version to them. (so here we necessarily got
				// 'rFTP.Patches.size() == 1' !)
				// The reference version itself is incrementally patched, however, and may be rebuilt from time to time
				// when the delta to the reference has become too big (but is most of the time < to the sum of equivallent delta patchs)
				// The non-incremental final patch (from ref file to final bnp), is guaranteed to be done last
				// after the reference file has been updated

				// Find the reference file to apply the patch
				SourceNameXD = rFTP.FileName;
				SourceNameXD = SourceNameXD.substr(0, SourceNameXD.rfind('.'));
				SourceNameXD += "_.ref";

				if (!_CommitPatch)
				{
					// works
					std::string tmpRefFile = SourceNameXD + ".tmp";
					if (!NLMISC::CFile::fileExists(pPM->ClientPatchPath + tmpRefFile))
					{
						// Not found in the patch directory -> version in data directory should be good, or would have been
						// detected by the check thread else.
					}
					else
					{
						SourceNameXD = tmpRefFile; // good ref file download by bg downloader
					}
				}

				// following lines added by Sadge to ensure that the correct file gets patched
				//			string SourceNameXDFull;
				//			if (NLMISC::CFile::fileExists(pPM->ClientDataPath + SourceNameXD))	SourceNameXDFull = pPM->ClientDataPath + SourceNameXD;

				// following lines removed by Sadge to ensure that the correct file gets patched
				//			string SourceNameXDFull = CPath::lookup(SourceNameXD, false, false);
				//			if (SourceNameXDFull.empty())
				//				SourceNameXDFull = pPM->ClientDataPath + SourceNameXD;
				//			SourceNameXD = SourceNameXDFull;
				if (CFile::fileExists(pPM->WritableClientDataPath + SourceNameXD))
				{
					SourceNameXD = pPM->WritableClientDataPath + SourceNameXD;
				}
				else if (CFile::fileExists(pPM->ReadableClientDataPath + SourceNameXD))
				{
					SourceNameXD = pPM->ReadableClientDataPath + SourceNameXD;
				}
			}

			PatchName = pPM->ClientPatchPath + PatchName;

			string OutFilename = pPM->ClientPatchPath + rFTP.FileName + ".tmp__" + toString(j);

			sTranslate = CI18N::get("uiApplyingDelta") + " " + CFile::getFilename(PatchName);
			pPM->setState(true, sTranslate);

			bool deltaPatchResult = xDeltaPatch(PatchName, SourceNameXD, OutFilename);

			if (!deltaPatchResult && !haveAllreadyTryiedDownloadingOfFile) // Patch failed, try to download and apply lzma
			{
				breakable
				{
					// compute the seven zip filename
					string lzmaFile = rFTP.FileName+".lzma";

					// download the 7zip file
					try
					{
						// first, try in the file version subfolfer
						try
						{
							progress.Scale = 1.f;
							progress.Bias = 0.f;
							if (!rFTP.Patches.empty())
							{
								pPM->getServerFile(toString("%05u/", rFTP.Patches.back())+lzmaFile, false, "", &progress);
							}
							// else -> file comes from a previous download (with .tmp extension, and is up to date)
							// the remaining code will just rename it with good name and exit
						}
						catch (const NLMISC::EWriteError &)
						{
							// this is a local error, rethrow ...
							throw;
						}
						catch(...)
						{
							// failed with version subfolder, try in the root patch directory
							pPM->getServerFile(lzmaFile, false, "", &progress);
						}
					}
					catch (const NLMISC::EWriteError &)
					{
						// this is a local error, rethrow ...
						throw;
					}
					catch (...)
					{
						break;
					}

					OutFilename = pPM->ClientPatchPath + NLMISC::CFile::getFilename(rFTP.FileName);
					// try to unpack the file
					try
					{
						if (!unpackLZMA(pPM->ClientPatchPath+lzmaFile, OutFilename+".tmp"))
						{
							break;
						}
					}
					catch (const NLMISC::EWriteError&)
					{
						throw;
					}
					catch (...)
					{
						nlwarning("Failed to unpack lzma file %s", (pPM->ClientPatchPath+lzmaFile).c_str());
						break;
					}

					if (rFTP.LocalFileExists)
						pPM->deleteFile(SourceName);

					pPM->deleteFile(pPM->ClientPatchPath+lzmaFile);	// delete the archive file
					pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
					if (_CommitPatch)
					{
						pPM->renameFile(OutFilename+".tmp", DestinationName);
					}
				}
			}
			else
			{
				if (rFTP.LocalFileExists)
					pPM->deleteFile(SourceName);
				pPM->deleteFile(PatchName);

				if (j > 0)
				{
					pPM->deleteFile(SourceNameXD, false, false); // File can exists if bad BNP loading
				}
				tmpSourceName = OutFilename;
				PatchSizeProgress += rFTP.PatcheSizes[j];
				currentPatchedSize += rFTP.PatcheSizes[j];
			}
		}
			if (tmpSourceName != DestinationName)
			{
				pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
				if (!_CommitPatch)
				{
					// let the patch in the unpack directory
					pPM->renameFile(tmpSourceName, pPM->ClientPatchPath + rFTP.FileName + ".tmp");
				}
				else
				{
					pPM->renameFile(tmpSourceName, DestinationName);
				}
			}
	}
	else
	{
		PatchSizeProgress += totalPatchSize;
	}

	// If all patches applied with success so file size should be ok
	// We just have to change file date to match the last patch applied
	pPM->applyDate(DestinationName, rFTP.LastFileDate);
	//progress.progress(1.f);
}

// ****************************************************************************
bool CPatchThread::xDeltaPatch(const string &patch, const string &src, const string &out)
{
	// Internal xdelta

	CPatchManager::MyPatchingCB patchingCB;

	patchingCB.patchFilename = patch;
	patchingCB.srcFilename = src;

	CPatchManager *pPM = CPatchManager::getInstance();
	pPM->deleteFile(out, false, false);

	std::string errorMsg;
	CXDeltaPatch::TApplyResult ar = CXDeltaPatch::apply(patch, src, out, errorMsg, &patchingCB);
	if (ar != CXDeltaPatch::ApplyResult_Ok)
	{
		switch(ar)
		{
			case CXDeltaPatch::ApplyResult_WriteError:
				throw NLMISC::EWriteError(out);
			break;
			case CXDeltaPatch::ApplyResult_DiskFull:
				throw NLMISC::EDiskFullError(out);
			break;
			default:
			{
				nlinfo("Error applying %s to %s giving %s", patch.c_str(), src.c_str(), out.c_str());
				return false;
			}
			break;
		}
	} else
		return true;


	// Launching xdelta
/*
	// Start the child process.
	string strCmdLine = "xdelta patch " + patch + " " + src + " " + out;
*/
}


// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// CScanDataThread
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
CScanDataThread::CScanDataThread()
{
	AskForCancel= false;
	Ended = false;
	CheckOk = false;
	TotalFileToScan = 1;
	CurrentFileScanned = 1;
}

// ****************************************************************************
void CScanDataThread::run ()
{
	CPatchManager *pPM = CPatchManager::getInstance();
	try
	{
		uint32 i;
		// Check if the client version is the same as the server version
		string sClientVersion = pPM->getClientVersion();
		string sTranslate = CI18N::get("uiClientVersion") + " (" + sClientVersion + ") ";
		pPM->setState(true, sTranslate);

		// For all bnp in the description file get all patches to apply
		// depending on the version of the client bnp files
		const CBNPFileSet &rDescFiles = pPM->DescFile.getFiles();
		TotalFileToScan = rDescFiles.fileCount();
		for (i = 0; i < rDescFiles.fileCount(); ++i)
		{
			sTranslate = CI18N::get("uiCheckingFile") + " " + rDescFiles.getFile(i).getFileName();
			pPM->setState(true, sTranslate);

			// get list of file to apply to this patch, performing a full checksum test (slow...)
			CPatchManager::SFileToPatch ftp;
			pPM->getPatchFromDesc(ftp, rDescFiles.getFile(i), false);
			// if the file has been found but don't correspond to any local version (SHA1)
			// or if the file has not been found (or the .bnp is very very buggy so that addSearchFile() failed)
			if( (ftp.LocalFileExists && ftp.LocalFileToDelete) ||
				(!ftp.LocalFileExists && !ftp.LocalFileToDelete) )
			{
				pPM->addDataScanLogCorruptedFile(ftp);
				CPatchManager::getCorruptedFileInfo(ftp, sTranslate);
				pPM->setState(true, sTranslate);
			}
			CurrentFileScanned = i;

			// if the user ask to cancel the thread, stop now
			if(AskForCancel)
				break;
		}

		sTranslate = CI18N::get("uiCheckEndNoErr");
		pPM->setState(true, sTranslate);
		CheckOk = true;
		Ended = true;
	}
	catch (const Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CScanDataThread::run() failed");
		string sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
		pPM->setState(true, sTranslate);
		CheckOk = false;
		Ended = true;
	}
}


// ****************************************************************************
uint32 CPatchManager::SPatchInfo::getAvailablePatchsBitfield() const
{
	// About the test (until a patch enum is added, we use the 'optional' flag)
	// Non optional -> must patch it (will be for RoS)
	// Optional -> Will be for Mainland
	// Required : stands for 'bnp' required by the Optional bnps !! so ignore only RoS is wanted

	uint32 result = 0;
	if (!NonOptCat.empty())
	{
		result |= (1 << BGDownloader::DownloadID_RoS);
	}
	if (!OptCat.empty() || !ReqCat.empty())
	{
		result |= (1 << BGDownloader::DownloadID_MainLand);
	}
	return result;
}

// ***************************************************************************
void CPatchManager::setAsyncDownloader(IAsyncDownloader* asyncDownloader)
{
	_AsyncDownloader = asyncDownloader;
}
// ***************************************************************************
IAsyncDownloader* CPatchManager::getAsyncDownloader() const
{
	return _AsyncDownloader;
}
// **************************************************************************
// ****************************************************************************
void CPatchManager::startInstallThread(const std::vector<CInstallThreadEntry>& entries)
{
	InstallThread = new CInstallThread(entries);
	Thread = IThread::create (InstallThread);
	nlassert (Thread != NULL);
	Thread->start ();
}

void CPatchManager::startDownloadThread(const std::vector<CInstallThreadEntry>& entries)
{
	DownloadThread = new CDownloadThread(entries);
	Thread = IThread::create (DownloadThread);
	nlassert (Thread != NULL);
	Thread->start ();
}


void CPatchManager::onFileInstallFinished()
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->onFileInstallFinished();
	}
}


void CPatchManager::onFileDownloadFinished()
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->onFileDownloadFinished();
	}
}


void CPatchManager::onFileDownloading(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize)
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->onFileDownloading(sourceName, rate, fileIndex, fileCount, fileSize, fullSize);
	}
}

void CPatchManager::onFileInstalling(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize)
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->onFileInstalling(sourceName, rate, fileIndex, fileCount, fileSize, fullSize);
	}
}

bool CPatchManager::download(const std::string& patchFullname, const std::string& sourceFullname,
									 const std::string& tmpDirectory, uint32 timestamp)
{
	CPatchManager *pPM = CPatchManager::getInstance();

	static std::string zsStr = ".lzma";
	static std::string::size_type zsStrLength = zsStr.size();

	// we delete file if present and diferent timestamp
	// if the file is the same then indicates its not necessary to download it a second time
	if (NLMISC::CFile::fileExists(sourceFullname))
	{
		uint32 t = NLMISC::CFile::getFileModificationDate(sourceFullname);
		if (t == timestamp)
		{	//we didn't downl oad
			return true;
		}
		pPM->deleteFile(sourceFullname);
	}
	// file will be save to a .tmp file
	std::string extension;
	if (patchFullname.size() >= zsStrLength	&& patchFullname.substr(patchFullname.size() - zsStrLength) == zsStr)
	{
		extension = zsStr;
	}
	// remove tmp file if exist
	std::string patchName = tmpDirectory + NLMISC::CFile::getFilename(sourceFullname) + extension + std::string(".tmp");
	if (NLMISC::CFile::fileExists(patchName))
	{
		pPM->deleteFile(patchName);
	}
	// creates directory tree if necessary
	NLMISC::CFile::createDirectoryTree( NLMISC::CFile::getPath(patchName) );
	NLMISC::CFile::createDirectoryTree( NLMISC::CFile::getPath(sourceFullname) );

	// try to download
	try
	{
		pPM->getServerFile(patchFullname, false, patchName);
	}
	catch ( const std::exception& e)
	{
		nlwarning("%s", e.what());
		pPM->setState(true, string(e.what()) );
		return false;
	}

	// install
	if (!NLMISC::CFile::fileExists(patchName))
	{
		return false;
	}
	// if file is compressed unpack then commit (rename)
	if (patchName.size() >= zsStrLength
		&& patchName.substr(patchName.size() - zsStrLength) == zsStr)
	{
		std::string outFilename = patchName.substr(0, patchName.size() - zsStrLength);
		unpack7Zip(patchName, outFilename);
		pPM->deleteFile(patchName);
		pPM->renameFile(outFilename, sourceFullname);
	}
	else
	{
		// file is not compressed so rename the .tmp file to final name
		pPM->renameFile(patchName, sourceFullname);
	}
	// apply modification date
	pPM->applyDate(sourceFullname, timestamp);

	return true;
}


bool CPatchManager::extract(const std::string& patchPath,
							const std::vector<std::string>& sourceFilename,
							const std::vector<std::string>& extractPath,
							const std::string& updateBatchFilename,
							const std::string& execName,
							void (*stopFun)() )
{
	nlassert(sourceFilename.size() == extractPath.size());
	CPatchManager *pPM = CPatchManager::getInstance();

	bool ok = false;
	for (uint32 j = 0; j < extractPath.size() && !ok; ++j)
	{
		if (!extractPath[j].empty())
		{
			ok = true;
		}
	}

	if (!ok)
	{
		// nothing to extract
		return false;
	}

	// extract
	uint nblab = 0;
	pPM->deleteFile(updateBatchFilename, false, false);

	FILE *fp = nlfopen (updateBatchFilename, "wt");

	if (fp == 0)
	{
		string err = toString("Can't open file '%s' for writing: code=%d %s (error code 29)", updateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (err);
	}

#ifdef NL_OS_WINDOWS
	fprintf(fp, "@echo off\n");
	fprintf(fp, "ping 127.0.0.1 -n 7 -w 1000 > nul\n"); // wait
#else
	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "sleep 7\n"); // wait
#endif

	// Unpack files with category ExtractPath non empty
	for (uint32 j = 0; j < sourceFilename.size(); ++j)
	{
		if (!extractPath[j].empty())
		{
			string rFilename = sourceFilename[j];
			// Extract to patch
			vector<string> vFilenames;
			if (!pPM->bnpUnpack(rFilename, patchPath, vFilenames))
			{
				/*
				string err = toString("Error unpacking %s", rFilename.c_str());
				throw Exception (err);
				*/
			}
			else
			{
				std::string sourcePath = NLMISC::CFile::getPath(sourceFilename[j]);
				for (uint32 fff = 0; fff < vFilenames.size (); fff++)
				{
					string SrcPath = CPath::standardizeDosPath(sourcePath);
					string SrcName = SrcPath + vFilenames[fff];
					string DstPath = CPath::standardizeDosPath(extractPath[j]);
					string DstName = DstPath + vFilenames[fff];
					NLMISC::CFile::createDirectoryTree(extractPath[j]);

					// this file must be moved
#ifdef NL_OS_WINDOWS
					fprintf(fp, ":loop%u\n", nblab);
					fprintf(fp, "attrib -r -a -s -h %s\n", DstName.c_str());
					fprintf(fp, "del %s\n", DstName.c_str());
					fprintf(fp, "if exist %s goto loop%u\n", DstName.c_str(), nblab);
					fprintf(fp, "move %s %s\n", SrcName.c_str(), DstPath.c_str());
#else
					// TODO: for Linux and OS X
#endif

					nblab++;

				}
			}
		}
	}


#ifdef NL_OS_WINDOWS
	fprintf(fp, "start %s %%1 %%2 %%3\n", execName.c_str());
#else
	// TODO: for Linux and OS X
#endif

	fclose(fp);

	if (stopFun)
	{
		stopFun();
	}

	if (!launchProgram(updateBatchFilename, "", false))
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s': code=%d %s (error code 30)", updateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (str);
	}

	return true;
}


void CPatchManager::setStateListener(IPatchManagerStateListener* stateListener)
{
	_StateListener = stateListener;
}


void CPatchManager::fatalError(const std::string& errorId, const std::string& param1, const std::string& param2)
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->fatalError(errorId, param1, param2);
	}
}


// ****************************************************************************
void CDownloadThread::run()
{
	CPatchManager *pPM = CPatchManager::getInstance();

	std::string patchPath = CPath::standardizePath (ClientRootPath+TheTmpInstallDirectory)+"patch/";


	static bool _FirstTime = true;
	static uint64 _FullSize = 0;
	static uint64 _CurrentSize = 0;
	static uint32 _Start;

	// At first launch calculat the amount of data need to download
	if (_FirstTime)
	{
		for (uint first = 0,last = (uint)_Entries.size() ; first != last; ++first)
		{
			_FullSize += _Entries[first].SZipFileSize;
		}
		_Start = NLMISC::CTime::getSecondsSince1970();
		_FirstTime = false;
	}


	for (uint first = 0,last = (uint)_Entries.size() ; first != last; ++first)
	{
		std::string patchName = CPath::standardizePath (_Entries[first].PatchName, false);
		std::string sourceName = CPath::standardizePath (_Entries[first].SourceName, false);

		_CurrentSize += _Entries[first].SZipFileSize;

		uint32 rate = 0;
		// Calculate the time since last update
		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			rate = uint32(_CurrentSize / dt);
		}
		// update Gui
		pPM->onFileDownloading(sourceName, rate, first+1, last, _CurrentSize, _FullSize);

		std::string finalFile =patchPath+ patchName;
		std::string tmpFile = finalFile + std::string(".part");


		bool toDownload = true;
		static volatile bool simulateDownload = false;
		if (simulateDownload)
		{
			nlSleep(100);
		}
		else
		{
			// Do not download if file are identicaly (check size and modification date)
			if (NLMISC::CFile::fileExists(finalFile))
			{
				uint64 fz = NLMISC::CFile::getFileSize(finalFile);
				uint32 timestamp = _Entries[first].Timestamp;
				uint32 timestamp2 = NLMISC::CFile::getFileModificationDate(finalFile);
				if ( fz == _Entries[first].SZipFileSize && timestamp == timestamp2)
				{
					toDownload = false;
				}
				else
				{
					NLMISC::CFile::deleteFile(finalFile);
				}
			}

			// file need to be download
			if (toDownload)
			{
				// delete tmp file if exist
				if (NLMISC::CFile::fileExists(tmpFile))
				{
					NLMISC::CFile::deleteFile(tmpFile);
				}
				std::string path = NLMISC::CFile::getPath(tmpFile);
				// create directory tree
				NLMISC::CFile::createDirectoryTree(path);
				// Try to download, rename, applyDate and send error msg to gui in case of error
				try
				{
					pPM->getServerFile(patchName, false, tmpFile);
					NLMISC::CFile::moveFile(finalFile, tmpFile);

					pPM->applyDate(finalFile, _Entries[first].Timestamp);
				}
				catch ( const std::exception& e)
				{
					nlwarning("%s", e.what());
					pPM->setState(true, string(e.what()) );
					pPM->fatalError("uiCanNotDownload", patchName.c_str(), "");
				}
				catch (...)
				{
					pPM->fatalError("uiCanNotDownload", patchName.c_str(), "");
				}
			}
		}

	}
	// message to gui to indicates that all file are downloaded
	pPM->onFileDownloadFinished();

}

void CInstallThread::run()
{
	std::string patchPath = CPath::standardizePath (ClientRootPath+TheTmpInstallDirectory)+"patch/";
	CPatchManager *pPM = CPatchManager::getInstance();

	std::set<std::string> allowed;

	uint first, last;

	for (first = 0,last = (uint)_Entries.size() ; first != last; ++first)
	{
		std::string correct = CPath::standardizePath (patchPath + _Entries[first].PatchName, false);
		allowed.insert(correct);
	}
	// Delete file from tmp directory that are not "allowed" (torrent protocol can download partial file that are not asked)
	std::vector<std::string> vFiles;
	CPath::getPathContent(patchPath , true, false, true, vFiles);
	for (uint32 i = 0; i < vFiles.size(); ++i)
	{
		std::string sName2 = CPath::standardizePath (vFiles[i], false);
		if (allowed.find(sName2) == allowed.end() )
		{
			pPM->deleteFile(sName2 , false, false);
		}
	}

	static bool _FirstTime = true;
	static uint64 _FullSize = 0;
	static uint64 _CurrentSize = 0;
	static uint32 _Start;

	// calculate size of data to download in order to know the install speed
	if (_FirstTime)
	{
		for (uint first = 0,last = (uint)_Entries.size() ; first != last; ++first)
		{
			_FullSize += _Entries[first].Size;
		}
		_Start = NLMISC::CTime::getSecondsSince1970();
		_FirstTime = false;
	}


	for (first = 0,last = (uint)_Entries.size() ; first != last; ++first)
	{
		std::string patchName = CPath::standardizePath (patchPath +_Entries[first].PatchName, false);
		std::string sourceName = CPath::standardizePath (_Entries[first].SourceName, false);
		uint32 lastFileDate = _Entries[first].Timestamp;


		_CurrentSize += _Entries[first].Size;
		uint32 rate = 0;
		// calcule the install speed
		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			uint64 size = _CurrentSize / dt;
			rate = uint32 (size);
		}

		// File can exists if bad BNP loading
		if (NLMISC::CFile::fileExists(sourceName))
		{
			pPM->deleteFile(sourceName, false, false);
		}

		if (NLMISC::CFile::fileExists(patchName))
		{
			static std::string zsStr = ".lzma";
			static std::string::size_type zsStrLength = zsStr.size();
			try
			{
				// if file is compressed unpack the file (decompress, rename tempory file, apply modification date)
				if (patchName.size() >= zsStrLength
					&& patchName.substr(patchName.size() - zsStrLength) == zsStr)
				{
					std::string outFilename = patchName.substr(0, patchName.size() - zsStrLength);
					std::string localOutFilename = CPath::standardizeDosPath(outFilename);

					if ( unpackLZMA(patchName, localOutFilename) )
					{
						pPM->deleteFile(patchName);
						pPM->renameFile(outFilename, sourceName);
						pPM->applyDate(sourceName, lastFileDate);
					}
					else
					{
						throw NLMISC::Exception("Can not unpack");
					}

				}
				else
				{
					// if file is uncompressed rename tempory file and apply modification date)
					pPM->renameFile(patchName, sourceName);
					pPM->applyDate(sourceName, lastFileDate);
				}
			}
			catch ( const std::exception& e)
			{
				nlwarning("%s", e.what());
				pPM->setState(true, string(e.what()) );
				pPM->fatalError("uiCanNotInstall", patchName.c_str(), "");
				return;

			}
			catch (...)
			{
				pPM->fatalError("uiCanNotInstall", patchName.c_str(), "");
				return;
			}
		}
		// update gui
		pPM->onFileInstalling(sourceName, rate, first+1, last, _CurrentSize, _FullSize);

	}
	// extract bnp

	{
		// remove date from tmp directory (because install is finished)
		std::string install = CPath::standardizePath (ClientRootPath+TheTmpInstallDirectory);

		std::vector<std::string> vFiles;
		// Delete all classic file from tmp directory
		CPath::getPathContent(install, true, false, true, vFiles);
		for (uint32 i = 0; i < vFiles.size(); ++i)
		{
			NLMISC::CFile::deleteFile(vFiles[i]);
		}
		// Delete all directory from tmp directory
		do
		{
			vFiles.clear();
			CPath::getPathContent(install, true, true, false, vFiles);
			for (uint32 i = 0; i < vFiles.size(); ++i)
			{
				NLMISC::CFile::deleteDirectory(vFiles[i]);
			}
		}
		while ( !vFiles.empty() );
		// delete tmp directory
		NLMISC::CFile::deleteDirectory(install);
		// delete libtorrent_logs directory if exist (not activate)
		if (NLMISC::CFile::fileExists("libtorrent_logs"))
		{
			do
			{
				vFiles.clear();
				CPath::getPathContent("libtorrent_logs", true, true, false, vFiles);
				for (uint32 i = 0; i < vFiles.size(); ++i)
				{
					NLMISC::CFile::deleteDirectory(vFiles[i]);
				}
			}
			while ( !vFiles.empty() );
			NLMISC::CFile::deleteDirectory("libtorrent_logs");
		}

	}

	pPM->reboot(); // do not reboot just run the extract .bat
}
