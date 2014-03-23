// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#define RZ_USE_SEVENZIP 1

// 7 zip includes
#ifdef RZ_USE_SEVENZIP
	#include "seven_zip/7zCrc.h"
	#include "seven_zip/7zIn.h"
	#include "seven_zip/7zExtract.h"
	#include "seven_zip/LzmaDecode.h"
#endif

#include "game_share/bg_downloader_msg.h"

#include "login_patch.h"
#include "login.h"


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


static std::vector<std::string> ForceMainlandPatchCategories;
static std::vector<std::string> ForceRemovePatchCategories;

using namespace std;
using namespace NLMISC;


extern string VersionName;
extern string R2ServerVersion;

#ifdef __CLIENT_INSTALL_EXE__
extern std::string TheTmpInstallDirectory;
extern std::string ClientLauncherUrl;
#else
std::string TheTmpInstallDirectory ="patch/client_install";
#endif
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
	virtual ~EPatchDownloadException() throw() {}
};


CPatchManager *CPatchManager::_Instance = NULL;

#ifdef RZ_USE_SEVENZIP
/// Input stream class for 7zip archive
class CNel7ZipInStream : public _ISzInStream
{
	NLMISC::IStream		*_Stream;

public:
	/// Constructor, only allow file stream because 7zip will 'seek' in the stream
	CNel7ZipInStream(NLMISC::IStream *s)
		:	_Stream(s)
	{
		Read = readFunc;
		Seek = seekFunc;
	}

	// the read function called by 7zip to read data
	static SZ_RESULT readFunc(void *object, void *buffer, size_t size, size_t *processedSize)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			me->_Stream->serialBuffer((uint8*)buffer, (uint)size);
			*processedSize = size;
			return SZ_OK;
		}
		catch (...)
		{
			return SZE_FAIL;
		}
	}

	// the seek function called by seven zip to seek inside stream
	static SZ_RESULT seekFunc(void *object, CFileSize pos)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			bool ret= me->_Stream->seek(pos, NLMISC::IStream::begin);
			if (ret)
				return SZ_OK;
			else
				return SZE_FAIL;
		}
		catch (...)
		{
			return SZE_FAIL;
		}
	}
};
#endif

static std::string ClientRootPath;

// ****************************************************************************
CPatchManager::CPatchManager() : State("t_state"), DataScanState("t_data_scan_state")
{
	DescFilename = "ryzom_xxxxx.idx";

#ifdef NL_OS_WINDOWS
	UpdateBatchFilename = "updt_nl.bat";
#else
	UpdateBatchFilename = "updt_nl.sh";
#endif

	// use current directory by default
	setClientRootPath("./");

	VerboseLog = true;

	PatchThread = NULL;
	CheckThread = NULL;
	InstallThread = NULL;
	ScanDataThread = NULL;
	thread = NULL;

	LogSeparator = "\n";
	ValidDescFile = false;

	MustLaunchBatFile = false;

	DownloadInProgress = false;
	_AsyncDownloader = NULL;
	_StateListener = NULL;
	_StartRyzomAtEnd = true;

#ifdef NL_OS_UNIX
	// don't use cfg, exe and dll from Windows version
	ForceRemovePatchCategories.clear();
	ForceRemovePatchCategories.push_back("main_exedll");
	ForceRemovePatchCategories.push_back("main_cfg");
#endif
}

// ****************************************************************************
void CPatchManager::setClientRootPath(const std::string& clientRootPath)
{
	ClientRootPath = clientRootPath;
	ClientPatchPath = ClientRootPath + "unpack/";
	ClientDataPath = ClientRootPath + "data/";
}

// ****************************************************************************
void CPatchManager::setErrorMessage(const ucstring &message)
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
	NLMISC::CFile::createDirectory(ClientDataPath);


	// try to read the version file from the server (that will replace the version number)
	try
	{
		CConfigFile *cf;
		#ifdef RY_BG_DOWNLOADER
			cf = &theApp.ConfigFile;
		#else
			cf = &ClientCfg.ConfigFile;
		#endif
		std::string appName = "ryzom_live";
		if (cf->getVarPtr("Application"))
		{
			appName = cf->getVar("Application").asString(0);
		}
		std::string versionFileName = appName + ".version";
		getServerFile(versionFileName);

		// ok, we have the file, extract version number (aka build number) and the
		// version name if present

		CIFile versionFile(ClientPatchPath+versionFileName);
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

		ServerVersion = line.firstWord(true);
		VersionName = line.firstWord(true);

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
	if (thread != NULL)
	{
		nlwarning ("a thread is already running");
		return;
	}

	_ErrorMessage.clear();

	CheckThread = new CCheckThread(includeBackgroundPatch);
	nlassert (CheckThread != NULL);

	thread = IThread::create (CheckThread);
	nlassert (thread != NULL);
	thread->start ();
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
	if(CheckThread && thread)
	{
		thread->wait();
		delete thread;
		thread = NULL;
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
	if (thread != NULL)
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
	thread = IThread::create (PatchThread);
	nlassert (thread != NULL);
	thread->start ();
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
bool CPatchManager::getThreadState (ucstring &stateOut, vector<ucstring> &stateLogOut)
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
			nlinfo("%s", stateLogOut[i].toString().c_str());

	return changed;
}

// ****************************************************************************
void CPatchManager::stopPatchThread()
{
	if(PatchThread && thread)
	{
		thread->wait();
		delete thread;
		thread = NULL;
		delete PatchThread;
		PatchThread = NULL;
	}
}

// ****************************************************************************
void CPatchManager::deleteBatchFile()
{
	deleteFile(UpdateBatchFilename, false, false);
}

// ****************************************************************************
void CPatchManager::createBatchFile(CProductDescriptionForClient &descFile, bool wantRyzomRestart, bool useBatchFile)
{
	uint nblab = 0;

	FILE *fp = NULL;

	if (useBatchFile)
	{
		deleteBatchFile();
		fp = fopen (UpdateBatchFilename.c_str(), "wt");
		if (fp == 0)
		{
			string err = toString("Can't open file '%s' for writing: code=%d %s (error code 29)", UpdateBatchFilename.c_str(), errno, strerror(errno));
			throw Exception (err);
		}
		//use bat if windows if not use sh
#ifdef NL_OS_WINDOWS
		fprintf(fp, "@echo off\n");
#else
		fprintf(fp, "#!/bin/sh\n");
#endif
	}

	// Unpack files with category ExtractPath non empty
	const CBNPCategorySet &rDescCats = descFile.getCategories();
	OptionalCat.clear();
	for (uint32 i = 0; i < rDescCats.categoryCount(); ++i)
	{
		// For all optional categories check if there is a 'file to patch' in it
		const CBNPCategory &rCat = rDescCats.getCategory(i);
		nlwarning("Category = %s", rCat.getName().c_str());
		if (!rCat.getUnpackTo().empty())
		for (uint32 j = 0; j < rCat.fileCount(); ++j)
		{
			string rFilename = ClientPatchPath + rCat.getFile(j);
			nlwarning("\tFileName = %s", rFilename.c_str());
			// Extract to patch
			vector<string> vFilenames;
			bool result = false;
			try
			{
				result = bnpUnpack(rFilename, ClientPatchPath, vFilenames);
			}
			catch(...)
			{
				if (useBatchFile)
				{
					fclose(fp);
				}

				throw;
			}
			if (!result)
			{
//:TODO: handle exception?
				string err = toString("Error unpacking %s", rFilename.c_str());

				if (useBatchFile)
				{
					fclose(fp);
				}

				throw Exception (err);
			}
			else
			{
				for (uint32 fff = 0; fff < vFilenames.size (); fff++)
				{
					string SrcPath = ClientPatchPath;
					string DstPath = rCat.getUnpackTo();
					NLMISC::CFile::createDirectoryTree(DstPath);
					// this file must be moved

					if (useBatchFile)
					{
						#ifdef NL_OS_WINDOWS
							SrcPath = CPath::standardizeDosPath(SrcPath);
							DstPath = CPath::standardizeDosPath(DstPath);
						#elif NL_OS_MAC
							//no patcher on mac yet
						#else
							SrcPath = CPath::standardizePath(SrcPath);
							DstPath = CPath::standardizePath(DstPath);
						#endif
					}

					std::string SrcName = SrcPath + vFilenames[fff];
					std::string DstName = DstPath + vFilenames[fff];

					if (useBatchFile)
					{
						// write windows .bat format else write sh format
#ifdef NL_OS_WINDOWS
						fprintf(fp, ":loop%u\n", nblab);
						fprintf(fp, "attrib -r -a -s -h %s\n", DstName.c_str());
						fprintf(fp, "del %s\n", DstName.c_str());
						fprintf(fp, "if exist %s goto loop%u\n", DstName.c_str(), nblab);
						fprintf(fp, "move %s %s\n", SrcName.c_str(), DstPath.c_str());
#else
						fprintf(fp, "rm -rf %s\n", DstName.c_str());
						fprintf(fp, "mv %s %s\n", SrcName.c_str(), DstPath.c_str());
#endif
					}
					else
					{
						deleteFile(DstName);
						CFile::moveFile(DstName.c_str(), SrcName.c_str());
					}

					nblab++;
				}
			}
		}
	}

	// Finalize batch file
	if (NLMISC::CFile::isExists("patch") && NLMISC::CFile::isDirectory("patch"))
	{
		#ifdef NL_OS_WINDOWS
		if (useBatchFile)
		{
			fprintf(fp, ":looppatch\n");
		}
		#endif
		
		vector<string> vFileList;
		CPath::getPathContent ("patch", false, false, true, vFileList, NULL, false);
		for(uint32 i = 0; i < vFileList.size(); ++i)
		{
			if (useBatchFile)
			{
				#ifdef NL_OS_WINDOWS
					fprintf(fp, "del %s\n", CPath::standardizeDosPath(vFileList[i]).c_str());
				#elif NL_OS_MAC
					//no patcher on MAC yet
				#else
					fprintf(fp, "rm -f %s\n", CPath::standardizePath(vFileList[i]).c_str());
				#endif
			}
			else
			{
				CFile::deleteFile(vFileList[i]);
			}
		}

		if (useBatchFile)
		{
			#ifdef NL_OS_WINDOWS
				fprintf(fp, "rd /Q /S patch\n");
				fprintf(fp, "if exist patch goto looppatch\n");
			#elif NL_OS_MAC
				//no patcher on mac yet
			#else
				fprintf(fp, "rm -rf patch\n");
			#endif
		}
		else
		{
			CFile::deleteDirectory("patch");
		}
	}

	if (useBatchFile)
	{
		if (wantRyzomRestart)
		{
			#ifdef NL_OS_WINDOWS
			fprintf(fp, "start %s %%1 %%2 %%3\n", RyzomFilename.c_str());
			#else
			fprintf(fp, "%s $1 $2 $3\n", RyzomFilename.c_str());
			#endif
		}

		bool writeError = ferror(fp) != 0;
		bool diskFull = ferror(fp) && errno == 28 /* ENOSPC */;
		fclose(fp);
		if (diskFull)
		{
			throw NLMISC::EDiskFullError(UpdateBatchFilename.c_str());
		}
		if (writeError)
		{
			throw NLMISC::EWriteError(UpdateBatchFilename.c_str());
		}
	}

}

// ****************************************************************************
void CPatchManager::executeBatchFile()
{
	// normal quit
	extern void quitCrashReport ();
	quitCrashReport ();

#ifdef NL_OS_WINDOWS
	// Launch the batch file
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // SW_SHOW

	si.cb = sizeof(si);

	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process.
	string strCmdLine;
	bool r2Mode = false;
	#ifndef RY_BG_DOWNLOADER
		r2Mode = ClientCfg.R2Mode;
	#endif
	if (r2Mode)
	{
		strCmdLine = UpdateBatchFilename + " " + LoginLogin + " " + LoginPassword;
	}
	else
	{
		strCmdLine = UpdateBatchFilename + " " + LoginLogin + " " + LoginPassword + " " + toString(LoginShardId);
	}
	if( !CreateProcess( NULL, // No module name (use command line).
		(char*)strCmdLine.c_str(), // Command line.
		NULL,				// Process handle not inheritable.
		NULL,				// Thread handle not inheritable.
		FALSE,				// Set handle inheritance to FALSE.
		0,					// No creation flags.
		NULL,				// Use parent's environment block.
		NULL,				// Use parent's starting directory.
		&si,				// Pointer to STARTUPINFO structure.
		&pi )				// Pointer to PROCESS_INFORMATION structure.
		)
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s': code=%d %s (error code 30)", UpdateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (str);
	}
	// Close process and thread handles.
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );

#else
	// Start the child process.
	bool r2Mode = false;
	#ifndef RY_BG_DOWNLOADER
		r2Mode = ClientCfg.R2Mode;
	#endif
	string strCmdLine;

	strCmdLine = "./" + UpdateBatchFilename;

	chmod(strCmdLine.c_str(), S_IRWXU);
	if (r2Mode)
	{
		if (execl(strCmdLine.c_str(), strCmdLine.c_str(), LoginLogin.c_str(), LoginPassword.c_str(), (char *) NULL) == -1)
		{
			int errsv = errno;
			nlerror("Execl Error: %d %s", errsv, strCmdLine.c_str(), (char *) NULL);
		}
		else
		{
			nlinfo("Ran batch file r2Mode Success");
		}
	}
	else
	{
		if (execl(strCmdLine.c_str(), strCmdLine.c_str(), LoginLogin.c_str(), LoginPassword.c_str(), toString(LoginShardId).c_str(), (char *) NULL) == -1)
		{
			int errsv = errno;
			nlerror("Execl r2mode Error: %d %s", errsv, strCmdLine.c_str());
		}
		else
		{
			nlinfo("Ran batch file Success");
		}
	}
#endif

//	exit(0);
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
void CPatchManager::setRWAccess (const string &filename)
{
	ucstring s = CI18N::get("uiSetAttrib") + " " + filename;
	setState(true, s);

	if (!NLMISC::CFile::setRWAccess(filename))
	{
		s = CI18N::get("uiAttribErr") + " " + filename + " (" + toString(errno) + "," + strerror(errno) + ")";
		setState(true, s);
		throw Exception (s.toString());
	}
}

// ****************************************************************************
string CPatchManager::deleteFile (const string &filename, bool bThrowException, bool bWarning)
{
	ucstring s = CI18N::get("uiDelFile") + " " + filename;
	setState(true, s);

	if (!NLMISC::CFile::fileExists(filename))
	{
		s = CI18N::get("uiDelNoFile");
		setState(true, s);
		return s.toString();
	}

	if (!NLMISC::CFile::deleteFile(filename))
	{
		s = CI18N::get("uiDelErr") + " " + filename + " (" + toString(errno) + "," + strerror(errno) + ")";
		if(bWarning)
			setState(true, s);
		if(bThrowException)
			throw Exception (s.toString());
		return s.toString();
	}
	return "";
}

// ****************************************************************************
void CPatchManager::renameFile (const string &src, const string &dst)
{
	ucstring s = CI18N::get("uiRenameFile") + " " + NLMISC::CFile::getFilename(src);
	setState(true, s);

	if (!NLMISC::CFile::moveFile(dst.c_str(), src.c_str()))
	{
		s = CI18N::get("uiRenameErr") + " " + src + " -> " + dst + " (" + toString(errno) + "," + strerror(errno) + ")";
		setState(true, s);
		throw Exception (s.toString());
	}
}

// ****************************************************************************
// Take care this function is called by the thread
void CPatchManager::setState (bool bOutputToLog, const ucstring &ucsNewState)
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

			if (unpackTo.substr(0, 2) == "./")
			{
				unpackTo = ClientRootPath + unpackTo.substr(2);
				category.setUnpackTo(unpackTo);
			}
		}
	}

	// tmp for debug : flag some categories as 'Mainland'
	for (cat = 0; cat < DescFile.getCategories().categoryCount(); ++cat)
	{
		if (std::find(ForceMainlandPatchCategories.begin(), ForceMainlandPatchCategories.end(),
			DescFile.getCategories().getCategory(cat).getName()) != ForceMainlandPatchCategories.end())
		{
			const_cast<CBNPCategory &>(DescFile.getCategories().getCategory(cat)).setOptional(true);
		}
	}

	CBNPFileSet &bnpFS = const_cast<CBNPFileSet &>(DescFile.getFiles());

	for(cat = 0; cat < DescFile.getCategories().categoryCount();)
	{
		const CBNPCategory &bnpCat = DescFile.getCategories().getCategory(cat);

		if (std::find(ForceRemovePatchCategories.begin(), ForceRemovePatchCategories.end(),
			bnpCat.getName()) != ForceRemovePatchCategories.end())
		{
			for(uint file = 0; file < bnpCat.fileCount(); ++file)
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

		if (UsedServer >= 0 && PatchServers.size() > 0)
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
			ucstring s = CI18N::get("uiLoginGetFile") + " " + NLMISC::CFile::getFilename(srcName);
			setState(true, s);

			// get the new file
			downloadFile (serverPath+srcName, dstName, progress);

			downloadSuccess = true;
		}
		catch (const EPatchDownloadException& e)
		{
			//nlwarning("EXCEPTION CATCH: getServerFile() failed - try to find an alternative: %i: %s",UsedServer,PatchServers[UsedServer].DisplayedServerPath.c_str());

			nlwarning("EXCEPTION CATCH: getServerFile() failed - try to find an alternative :");
			nlwarning("%i", UsedServer);
			if (UsedServer >= 0 && UsedServer < (int) PatchServers.size())
			{
				nlwarning("%s", PatchServers[UsedServer].DisplayedServerPath.c_str());
			}

			// if emergency patch server, this is a real issue, rethrow exception
			if (UsedServer < 0)
			{
				ucstring s = CI18N::get("uiDLFailed");
				setState(true, s);

				throw Exception(e.what());
			}

			ucstring s = CI18N::get("uiDLURIFailed") + " " + serverDisplayPath;
			setState(true, s);

			// this server is unavailable
			PatchServers[UsedServer].Available = false;

			sint	nextServer = (UsedServer+1) % PatchServers.size();

			while (nextServer != UsedServer && !PatchServers[nextServer].Available)
				nextServer = (nextServer+1) % PatchServers.size();

			// scanned all servers? use alternative
			if (nextServer == UsedServer)
			{
				ucstring s = CI18N::get("uiNoMoreURI");
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
		ucstring s = CI18N::get("uiDLWithCurl") + " " + dest;
		setState(true, s);

		// user agent = nel_launcher

		CURL *curl;
		CURLcode res;

		ucstring sTranslate = CI18N::get("uiLoginGetFile") + " " + NLMISC::CFile::getFilename (source);
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

		// create the local file
		if (NLMISC::CFile::fileExists(dest))
		{
			setRWAccess(dest);
			NLMISC::CFile::deleteFile(dest.c_str());
		}
		FILE *fp = fopen (dest.c_str(), "wb");
		if (fp == NULL)
		{
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
			throw Exception ("Can't open file '%s' for writing: code=%d %s (error code 37)", dest.c_str (), errno, strerror(errno));
		}
		curl_easy_setopt(curl, CURLOPT_FILE, fp);

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
	const string sHeadHttp = toLower(source.substr(0,5));
	const string sHeadFtp = toLower(source.substr(0,4));
	const string sHeadFile = toLower(source.substr(0,5));

	if ((sHeadHttp == "http:") || (sHeadFtp == "ftp:") || (sHeadFile == "file:"))
	{
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
	ucstring sTranslate = CI18N::get("uiDecompressing") + " " + NLMISC::CFile::getFilename(filename);
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
	setRWAccess(dest);
	//if(isVerboseLog()) nlinfo("Calling fopen('%s','wb')", dest.c_str());
	FILE *fp = fopen (dest.c_str(), "wb");
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
//		_utimbuf utb;
//		utb.actime = utb.modtime = nDate;
		setRWAccess(sFilename);
		ucstring s = CI18N::get("uiChangeDate") + " " + NLMISC::CFile::getFilename(sFilename) + " " + toString(NLMISC::CFile::getFileModificationDate (sFilename)) +
						" -> " + toString(nDate);
		setState(true,s);

		if (!NLMISC::CFile::setFileModificationDate(sFilename, nDate))
//		if (_utime (sFilename.c_str (), &utb) == -1)
		{
			int err = NLMISC::getLastError();
			s = CI18N::get("uiChgDateErr") + " " + sFilename + " (" + toString(err) + ", " + formatErrorMessage(err) + ")";
			setState(true,s);
		}
		s = CI18N::get("uiNowDate") + " " + sFilename + " " + toString(NLMISC::CFile::getFileModificationDate (sFilename));
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
		if (sFilePath.empty() && NLMISC::CFile::fileExists(ClientDataPath + rFilename))		sFilePath = ClientDataPath + rFilename;
	}
	if (sFilePath.empty() && NLMISC::CFile::fileExists(ClientPatchPath + rFilename))		sFilePath = ClientPatchPath + rFilename;

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
			ucstring sTranslate = CI18N::get("uiCheckInt") + " " + rFilename;
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
			ucstring sTranslate = CI18N::get("uiNoVersionFound");
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
			ucstring sTranslate = CI18N::get("uiVersionFound") + " " + toString(nVersionFound);
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
bool CPatchManager::readBNPHeader(const string &SourceName, vector<SBNPFile> &Files)
{
	/* *****
		If the BNP Header format change, please modify 'EmptyBnpFileSize' as needed
	***** */
	FILE *f = fopen (SourceName.c_str(), "rb");
	if (f == NULL) return false;

	nlfseek64 (f, 0, SEEK_END);
	uint32 nFileSize = NLMISC::CFile::getFileSize (SourceName);
	nlfseek64 (f, nFileSize-sizeof(uint32), SEEK_SET);

	uint32 nOffsetFromBeginning;
	if (fread (&nOffsetFromBeginning, sizeof(uint32), 1, f) != 1)
	{
		fclose(f);
		return false;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

	if (nlfseek64 (f, nOffsetFromBeginning, SEEK_SET) != 0)
	{
		fclose(f);
		return false;
	}

	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
	{
		fclose(f);
		return false;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nNbFile);
#endif

	for (uint32 i = 0; i < nNbFile; ++i)
	{
		uint8 nStringSize;
		if (fread (&nStringSize, 1, 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		char sName[256];
		if (fread (sName, 1, nStringSize, f) != nStringSize)
		{
			fclose(f);
			return false;
		}
		sName[nStringSize] = 0;

		SBNPFile tmpBNPFile;
		tmpBNPFile.Name = sName;
		if (fread (&tmpBNPFile.Size, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(tmpBNPFile.Size);
#endif

		if (fread (&tmpBNPFile.Pos, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(tmpBNPFile.Pos);
#endif

		Files.push_back (tmpBNPFile);
	}
	fclose (f);
	return true;
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

	ucstring s = CI18N::get("uiUnpack") + " " + NLMISC::CFile::getFilename(SourceName);
	setState(true,s);

	// Read Header of the BNP File
	vector<CPatchManager::SBNPFile> Files;
	if (!readBNPHeader(SourceName, Files))
	{
		ucstring s = CI18N::get("uiUnpackErrHead") + " " + SourceName;
		setState(true,s);
		return false;
	}

	// Unpack
	{
		FILE *bnp = fopen (SourceName.c_str(), "rb");
		FILE *out;
		if (bnp == NULL)
			return false;

		for (uint32 i = 0; i < Files.size(); ++i)
		{
			CPatchManager::SBNPFile &rBNPFile = Files[i];
			string filename = DestPath + rBNPFile.Name;
			out = fopen (filename.c_str(), "wb");
			if (out != NULL)
			{
				nlfseek64 (bnp, rBNPFile.Pos, SEEK_SET);
				uint8 *ptr = new uint8[rBNPFile.Size];

				if (fread (ptr, rBNPFile.Size, 1, bnp) != 1)
				{
					fclose(out);
					return false;
				}

				bool writeError = fwrite (ptr, rBNPFile.Size, 1, out) != 1;
				if (writeError)
				{
					nlwarning("errno = %d", errno);
				}
				bool diskFull = ferror(out) && errno == 28 /* ENOSPC*/;
				fclose (out);
				delete [] ptr;
				if (diskFull)
				{
					throw NLMISC::EDiskFullError(filename);
				}
				if (writeError)
				{
					throw NLMISC::EWriteError(filename);
				}
			}
		}
		fclose (bnp);
	}

	// Return names
	{
		vFilenames.clear();
		for (uint32 i = 0; i < Files.size(); ++i)
			vFilenames.push_back(Files[i].Name);
	}

	return true;
}


// ****************************************************************************
int CPatchManager::downloadProgressFunc(void *foo, double t, double d, double ultotal, double ulnow)
{
	if (d != t)
	{
		// In the case of progress = 1, don't update because, this will be called in case of error to signal the end of the download, though
		// no download actually occured. Instead, we set progress to 1.f at the end of downloadWithCurl if everything went fine
		return validateProgress(foo, t, d, ultotal, ulnow);
	}
	return 0;
}

// ****************************************************************************
int CPatchManager::validateProgress(void *foo, double t, double d, double /* ultotal */, double /* ulnow */)
{
	CPatchManager *pPM = CPatchManager::getInstance();
	double pour1 = t!=0.0?d*100.0/t:0.0;
	ucstring sTranslate = CI18N::get("uiLoginGetFile") + toString(" %s : %s / %s (%5.02f %%)", NLMISC::CFile::getFilename(pPM->CurrentFile).c_str(),
		NLMISC::bytesToHumanReadable((uint64)d).c_str(), NLMISC::bytesToHumanReadable((uint64)t).c_str(), pour1);
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
	ucstring sTranslate = CI18N::get("uiApplyingDelta") + toString(" %s (%5.02f %%)", patchFilename.c_str(), p);
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
	if (thread != NULL)
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

	thread = IThread::create (ScanDataThread);
	nlassert (thread != NULL);
	thread->start ();
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
	if(ScanDataThread && thread)
	{
		thread->wait();
		delete thread;
		thread = NULL;
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

	uint	numError= 0;
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
			if (NLMISC::CFile::fileExists(ClientDataPath + ftp.FileName))	sFilePath = ClientDataPath + ftp.FileName;
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
bool CPatchManager::getDataScanLog(ucstring &text)
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
				ucstring	str;
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
void CPatchManager::getCorruptedFileInfo(const SFileToPatch &ftp, ucstring &sTranslate)
{
	sTranslate = CI18N::get("uiCorruptedFile") + " " + ftp.FileName + " (" +
		toString("%.1f ", (float)ftp.FinalFileSize/1000000.f) + CI18N::get("uiMb") + ")";
}

bool CPatchManager::unpack7Zip(const std::string &sevenZipFile, const std::string &destFileName)
{
#ifdef RZ_USE_SEVENZIP
	nlinfo("Uncompressing 7zip archive '%s' to '%s'",
		sevenZipFile.c_str(),
		destFileName.c_str());

	// init seven zip
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	CArchiveDatabaseEx db;

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	InitCrcTable();
	SzArDbExInit(&db);

	// unpack the file using the 7zip API

	CIFile input(sevenZipFile);
	CNel7ZipInStream	inStr(&input);

	SZ_RESULT res = SzArchiveOpen(&inStr, &db, &allocImp, &allocTempImp);
	if (res != SZ_OK)
	{
		nlerror("Failed to open archive file %s", sevenZipFile.c_str());
		return false;
	}

	if (db.Database.NumFiles != 1)
	{
		nlerror("Seven zip archive with more than 1 file are unsupported");
		return false;
	}

	UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0; /* it can have any value before first call (if outBuffer = 0) */

	size_t offset;
	size_t outSizeProcessed = 0;

	// get the first file
	CFileItem *f = db.Database.Files;
	res = SzExtract(&inStr, &db, 0,
		&blockIndex, &outBuffer, &outBufferSize,
		&offset, &outSizeProcessed,
		&allocImp, &allocTempImp);

	// write the extracted file
	FILE *outputHandle;
	UInt32 processedSize;
	char *fileName = f->Name;
	size_t nameLen = strlen(f->Name);
	for (; nameLen > 0; nameLen--)
	{
		if (f->Name[nameLen - 1] == '/')
		{
			fileName = f->Name + nameLen;
			break;
		}
	}

	outputHandle = fopen(destFileName.c_str(), "wb+");
	if (outputHandle == 0)
	{
		nlerror("Can not open output file '%s'", destFileName.c_str());
		return false;
	}
	processedSize = (UInt32)fwrite(outBuffer + offset, 1, outSizeProcessed, outputHandle);
	if (processedSize != outSizeProcessed)
	{
		nlerror("Failed to write %u char to output file '%s'", outSizeProcessed-processedSize, destFileName.c_str());
		return false;
	}
	fclose(outputHandle);
	allocImp.Free(outBuffer);

	// free 7z context
	SzArDbExFree(&db, allocImp.Free);

	// ok, all is fine, file is unpacked
	return true;
#else
	return false;
#endif
}

bool CPatchManager::unpackLZMA(const std::string &lzmaFile, const std::string &destFileName)
{
#ifdef RZ_USE_SEVENZIP
	nldebug("unpackLZMA : decompression the lzma file '%s' into output file '%s", lzmaFile.c_str(), destFileName.c_str());
	CIFile inStream(lzmaFile);
	uint32 inSize = inStream.getFileSize();
	auto_ptr<uint8> inBuffer = auto_ptr<uint8>(new uint8[inSize]);
	inStream.serialBuffer(inBuffer.get(), inSize);

	CLzmaDecoderState state;

	uint8 *pos = inBuffer.get();
	// read the lzma properties
	int ret = LzmaDecodeProperties(&state.Properties, (unsigned char*) pos, LZMA_PROPERTIES_SIZE);
	if (ret != 0)
	{
		nlwarning("Failed to decode lzma properies in file '%s'!", lzmaFile.c_str());
		return false;
	}

	if (inSize < LZMA_PROPERTIES_SIZE + 8)
	{
		nlwarning("Invalid file size, too small file '%s'", lzmaFile.c_str());
		return false;
	}

	// alloc the probs, making sure they are deleted in function exit
	size_t nbProb = LzmaGetNumProbs(&state.Properties);
	auto_ptr<CProb> probs = auto_ptr<CProb>(new CProb[nbProb]);
	state.Probs = probs.get();

	pos += LZMA_PROPERTIES_SIZE;

	// read the output file size
	size_t fileSize = 0;
	for (int i = 0; i < 8; i++)
	{
		//Byte b;
		if (pos >= inBuffer.get()+inSize)
		{
			nlassert(false);
			return false;
		}
		fileSize |= ((UInt64)*pos++) << (8 * i);
	}

	SizeT outProcessed = 0;
	SizeT inProcessed = 0;
	// allocate the output buffer
	auto_ptr<uint8> outBuffer = auto_ptr<uint8>(new uint8[fileSize]);
	// decompress the file in memory
	ret = LzmaDecode(&state, (unsigned char*) pos, (SizeT)(inSize-(pos-inBuffer.get())), &inProcessed, (unsigned char*)outBuffer.get(), (SizeT)fileSize, &outProcessed);
	if (ret != 0 || outProcessed != fileSize)
	{
		nlwarning("Failed to decode lzma file '%s'", lzmaFile.c_str());
		return false;
	}

	// store on output buffer
	COFile outStream(destFileName);
	outStream.serialBuffer(outBuffer.get(), (uint)fileSize);

	return true;
#else
	return false;
#endif
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
		ucstring sTranslate = CI18N::get("uiClientVersion") + " (" + sClientVersion + ") ";
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

#ifdef NL_OS_UNIX
		string sClientNewVersion = ClientCfg.BuildName;

		sint32 nClientNewVersion;
		fromString(sClientNewVersion, nClientNewVersion);

		// servers files are not compatible with current client, use last client version
		if (nClientNewVersion && nServerVersion > nClientNewVersion)
		{
			nServerVersion = nClientNewVersion;
		}
#endif

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
			if (ftp.Patches.size() > 0 || (IncludeBackgroundPatch && !ftp.SrcFileName.empty()))
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
					vector<CPatchManager::SBNPFile> vFiles;
					if (pPM->readBNPHeader(sBNPFilename, vFiles))
					{
						// read the file inside the bnp and calculate the sha1
						FILE *bnp = fopen (sBNPFilename.c_str(), "rb");
						if (bnp != NULL)
						{
							for (uint32 k = 0; k < vFiles.size(); ++k)
							{
								CPatchManager::SBNPFile &rBNPFile = vFiles[k];
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
		ucstring sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
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

	ucstring sTranslate;
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
			if (ext == "bnp")
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
		pPM->setState(true, ucstring(e.what()));
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



// ****************************************************************************
void CPatchThread::processFile (CPatchManager::SFileToPatch &rFTP)
{
	CPatchManager *pPM = CPatchManager::getInstance();
	// Source File Name
	string SourceName;
	if (rFTP.ExtractPath.empty())
	{
		if (rFTP.LocalFileExists)
		{
			// following lines added by Sadge to ensure that the correct file gets patched
			SourceName.clear();
			if (NLMISC::CFile::fileExists(pPM->ClientDataPath + rFTP.FileName))		SourceName = pPM->ClientDataPath + rFTP.FileName;
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
			SourceName = pPM->ClientDataPath + rFTP.FileName;
		}
	}
	else
	{
		SourceName = pPM->ClientPatchPath + rFTP.FileName;
	}

	if (rFTP.LocalFileToDelete)
	{
		// corrupted or invalid file ? ....
		NLMISC::CFile::deleteFile(SourceName);
		rFTP.LocalFileExists = false;
	}

	ucstring sTranslate;
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

	// compute the total size of patch to download
	uint32 totalPatchSize = 0;
	if (rFTP.Incremental)
	{
		for (uint i=0; i<rFTP.PatcheSizes.size(); ++i)
			totalPatchSize += rFTP.PatcheSizes[i];
	}
	else
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
				break;
			}

			OutFilename = pPM->ClientPatchPath + NLMISC::CFile::getFilename(rFTP.FileName);
			// try to unpack the file
			try
			{
				if (!CPatchManager::unpackLZMA(pPM->ClientPatchPath+lzmaFile, OutFilename+".tmp"))
				{
					// fallback to standard patch method
					usePatchFile = true;
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
				break;
			}

			if (rFTP.LocalFileExists)
				pPM->deleteFile(SourceName);

			pPM->deleteFile(pPM->ClientPatchPath+lzmaFile);	// delete the archive file
			pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
			if (_CommitPatch)
			{
				pPM->renameFile(OutFilename+".tmp", SourceName);
			}
		}
	}
	if (usePatchFile && !rFTP.Patches.empty())
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
				// when the delta to the reference has become to big (but is most of the time < to the sum of equivallent delta patchs)
				// The non-incremental final patch (from ref file to final bnp), is guaranteed to be done last
				// after the reference file has been updated

				// Find the reference file to apply the patch
				SourceNameXD = rFTP.FileName;
				SourceNameXD = SourceNameXD.substr(0, SourceNameXD.rfind('.'));
				SourceNameXD += "_.ref";

				std::string refPath;
				if (_CommitPatch)
				{
					refPath = pPM->ClientDataPath;
				}
				else
				{
					// works
					refPath = pPM->ClientPatchPath;
					std::string tmpRefFile = SourceNameXD + ".tmp";
					if (!NLMISC::CFile::fileExists(pPM->ClientPatchPath + tmpRefFile))
					{
						// Not found in the patch directory -> version in data directory should be good, or would have been
						// detected by the check thread else.
						refPath = pPM->ClientDataPath;
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
				SourceNameXD = pPM->ClientDataPath + SourceNameXD;
			}

			PatchName = pPM->ClientPatchPath + PatchName;

			string OutFilename = pPM->ClientPatchPath + rFTP.FileName + ".tmp__" + toString(j);

			sTranslate = CI18N::get("uiApplyingDelta") + " " + PatchName;
			pPM->setState(true, sTranslate);

			xDeltaPatch(PatchName, SourceNameXD, OutFilename);

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
		if (tmpSourceName != SourceName)
		{
			pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
			if (!_CommitPatch)
			{
				// let the patch in the unpack directory
				pPM->renameFile(tmpSourceName, pPM->ClientPatchPath + rFTP.FileName + ".tmp");
			}
			else
			{
				pPM->renameFile(tmpSourceName, SourceName);
			}
		}
	}
	else
	{
		PatchSizeProgress += totalPatchSize;
	}
	// If all patches applied with success so file size should be ok
	// We just have to change file date to match the last patch applied
	pPM->applyDate(SourceName, rFTP.LastFileDate);
	//progress.progress(1.f);
}

// ****************************************************************************
void CPatchThread::xDeltaPatch(const string &patch, const string &src, const string &out)
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
				std::string str = toString("Error applying %s to %s giving %s", patch.c_str(), src.c_str(), out.c_str());
				throw Exception (str);
			}
			break;
		}
	}


	// Launching xdelta
/*
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);

	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process.
	string strCmdLine = "xdelta patch " + patch + " " + src + " " + out;

	if( !CreateProcess( NULL, // No module name (use command line).
		(char*)strCmdLine.c_str(), // Command line.
		NULL,							// Process handle not inheritable.
		NULL,							// Thread handle not inheritable.
		FALSE,						// Set handle inheritance to FALSE.
		0,								// No creation flags.
		NULL,							// Use parent's environment block.
		NULL,							// Use parent's starting directory.
		&si,							// Pointer to STARTUPINFO structure.
		&pi )							// Pointer to PROCESS_INFORMATION structure.
		)
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s'", strCmdLine.c_str());
		throw Exception (str);
	}

	// Wait for the process to terminate
	DWORD dwTimeout = 1000 * 300; // 5 min = 300 s
	DWORD nRetWait = WaitForSingleObject(pi.hProcess, dwTimeout);

	if (nRetWait == WAIT_TIMEOUT)
	{
		string str = toString("Time Out After %d s", dwTimeout/1000);
		throw Exception (str);
	}

	// Close process and thread handles.
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
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
		ucstring sTranslate = CI18N::get("uiClientVersion") + " (" + sClientVersion + ") ";
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
		ucstring sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
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
	CInstallThread* installThread = new CInstallThread(entries);
	thread = IThread::create (installThread);
	nlassert (thread != NULL);
	thread->start ();
}

void CPatchManager::startDownloadThread(const std::vector<CInstallThreadEntry>& entries)
{
	CDownloadThread* downloadThread = new CDownloadThread(entries);
	thread = IThread::create (downloadThread);
	nlassert (thread != NULL);
	thread->start ();
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
		pPM->setState(true, ucstring(e.what()) );
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
		CPatchManager::unpack7Zip(patchName, outFilename);
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
	FILE *fp = fopen (updateBatchFilename.c_str(), "wt");
	if (fp == 0)
	{
		string err = toString("Can't open file '%s' for writing: code=%d %s (error code 29)", updateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (err);
	}
	fprintf(fp, "@echo off\n");
	fprintf(fp, "ping 127.0.0.1 -n 7 -w 1000 > nul\n"); // wait


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

					fprintf(fp, ":loop%u\n", nblab);
					fprintf(fp, "attrib -r -a -s -h %s\n", DstName.c_str());
					fprintf(fp, "del %s\n", DstName.c_str());
					fprintf(fp, "if exist %s goto loop%u\n", DstName.c_str(), nblab);
					fprintf(fp, "move %s %s\n", SrcName.c_str(), DstPath.c_str());
					nblab++;
				}
			}
		}
	}


	fprintf(fp, "start %s %%1 %%2 %%3\n", execName.c_str());
	fclose(fp);

	if (stopFun)
	{
		stopFun();
	}

#ifdef NL_OS_WINDOWS
	// normal quit
	// Launch the batch file
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // SW_SHOW

	si.cb = sizeof(si);

	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process.
	string strCmdLine;
	strCmdLine = updateBatchFilename;
	//onFileInstallFinished();

	if( !CreateProcess( NULL, // No module name (use command line).
		(LPSTR)strCmdLine.c_str(), // Command line.
		NULL,				// Process handle not inheritable.
		NULL,				// Thread handle not inheritable.
		FALSE,				// Set handle inheritance to FALSE.
		0,					// No creation flags.
		NULL,				// Use parent's environment block.
		NULL,				// Use parent's starting directory.
		&si,				// Pointer to STARTUPINFO structure.
		&pi )				// Pointer to PROCESS_INFORMATION structure.
		)
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s': code=%d %s (error code 30)", updateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (str);
	}
#else
	// TODO for Linux and Mac OS
#endif
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
					NLMISC::CFile::moveFile(finalFile.c_str(), tmpFile.c_str());

					pPM->applyDate(finalFile, _Entries[first].Timestamp);
				}
				catch ( const std::exception& e)
				{
					nlwarning("%s", e.what());
					pPM->setState(true, ucstring(e.what()) );
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

					if ( CPatchManager::unpackLZMA(patchName, localOutFilename) )
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
				pPM->setState(true, ucstring(e.what()) );
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

