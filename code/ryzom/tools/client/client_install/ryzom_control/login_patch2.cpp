/** \file patch.cpp
 *
 * $Id: login_patch2.cpp,v 1.3 2007/06/07 13:34:32 boucher Exp $
 */

/* Copyright, 2004 Nevrax Ltd.
 *
 * This file is part of NEVRAX NELNS.
 * NEVRAX NELNS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NELNS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NELNS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

//
// Includes
//

#include "stdpch.h"

#include <memory>
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

// 7 zip includes
extern "C"
{
#include "seven_zip/7zCrc.h"
#include "seven_zip/7zIn.h"
#include "seven_zip/7zExtract.h"
#include "seven_zip/LzmaDecode.h"
};
#include "login_patch2.h"
#include "login.h"
#include "client_cfg.h"

#include <direct.h>
//#include <sys/utime.h>

//
// Namespaces
//

using namespace std;
using namespace NLMISC;


extern string VersionName;
extern string R2ServerVersion;


#ifdef __CLIENT_INSTALL_EXE__
extern std::string CfgApplication;
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

	// the read function called by 7sip to read data
	static  SZ_RESULT readFunc(void *object, void *buffer, size_t size, size_t *processedSize)
	{
		try
		{
			CNel7ZipInStream *me = (CNel7ZipInStream*)object;
			me->_Stream->serialBuffer((uint8*)buffer, size);
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



// ****************************************************************************
CPatchManager::CPatchManager() : State("t_state"), DataScanState("t_data_scan_state")
{
	DescFilename = "ryzom_xxxxx.idx";

	char filename[1024];
	GetModuleFileName(GetModuleHandle(NULL), filename, 1024);
	RyzomFilename = CFile::getFilename(filename);

	UpdateBatchFilename = "updt_nl.bat";

	ClientPatchPath = "./unpack/";
	ClientDataPath = "./data/";

	VerboseLog = true;

	PatchThread = NULL;
	CheckThread = NULL;
	InstallThread = NULL;
	ScanDataThread = NULL;
	thread = NULL;

	LogSeparator = "\n";
	ValidDescFile = false;

	MustLaunchBatFile = false;

	_AsyncDownloader = NULL;
	_StartRyzomAtEnd = true;
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

	srand(time(NULL));
	UsedServer = rand() * PatchServers.size() / (RAND_MAX+1);

	ServerPath = CPath::standardizePath (sServerPath);
	ServerVersion = sServerVersion;

	uint pos = ServerPath.find ("@");
	if (pos != string::npos)
		DisplayedServerPath = "http://" + ServerPath.substr (pos+1);
	else
		DisplayedServerPath = ServerPath;

	NLMISC::CFile::createDirectory(ClientPatchPath);
	NLMISC::CFile::createDirectory(ClientDataPath);

	// try to read the version file from the server (that will replace the version number)
	try
	{
		string versionFileName;

#ifdef __CLIENT_INSTALL_EXE__
		versionFileName = CfgApplication+".version";
#else
		versionFileName = ClientCfg.ConfigFile.getVar("Application").asString(0)+".version";		
#endif
		
		

		getServerFile(versionFileName);

		// ok, we have the file, extract version number (aka build number) and the 
		// version name if present
		CIFile versionFile(ClientPatchPath+versionFileName);
		char buffer[1024];
		versionFile.getline(buffer, 1024);
		CSString line(buffer);

		ServerVersion = line.firstWord(true);
		//ServerVersion = "00134";
		VersionName = line.firstWord(true);

		// force the R2ServerVersion
		R2ServerVersion = ServerVersion;

		#ifdef __CLIENT_INSTALL_EXE__
		{
		
			std::string clientLauncherUrl = "client_launcher_url.txt";
			bool ok = true;
			try
			{
				std::string url = toString("%05u/%s", atoi(ServerVersion.c_str()), clientLauncherUrl.c_str());
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

void CPatchManager::readClientVersionAndDescFile()
{
	try
	{
		ValidDescFile = false;
		vector<string> vFiles;
		CPath::getPathContent(ClientPatchPath, false, false, true, vFiles);
		uint32 nVersion = 0xFFFFFFFF;
		for (uint32 i = 0; i < vFiles.size(); ++i)
		{
			string sName = CFile::getFilename(vFiles[i]);
			string sExt = CFile::getExtension(sName);
			string sBase = sName.substr(0, sName.rfind('_'));
			if ((sExt == "idx") && (sBase == "ryzom"))
			{
				string val = sName.substr(sName.rfind('_')+1, 5);
				uint32 nNewVersion = atoi(val.c_str());
				if ((nNewVersion > nVersion) || (nVersion == 0xFFFFFFFF))
					nVersion = nNewVersion;
			}
		}
		if (nVersion != 0xFFFFFFFF)
			readDescFile(nVersion);
		else
			DescFilename = "unknown";
		ValidDescFile = true;
	}
	catch(Exception &)
	{
		nlwarning("EXCEPTION CATCH: readClientVersionAndDescFile() failed - not important");
		// Not important that there is no desc file
	}
}

// ****************************************************************************
void CPatchManager::startCheckThread()
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

	CheckThread = new CCheckThread();
	nlassert (CheckThread != NULL);

	thread = IThread::create (CheckThread);
	nlassert (thread != NULL);
	thread->start ();
}
// ***************************************************************************
void CPatchManager::setAsyncDownloader(IAsyncDownloader* asyncDownloader)
{
	_AsyncDownloader = asyncDownloader;
}
// ***************************************************************************
IAsyncDownloader*  CPatchManager::getAsyncDownloader() const
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

	// Convertir le vecteur FilesToPatch qui doit etre initialise en un truc comprehensible par l'homme
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
void CPatchManager::startPatchThread(const vector<string> &CategoriesSelected)
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

	PatchThread = new CPatchThread();
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
	uint32 nSize = CatsSelected.size();
	do
	{	
		nSize = CatsSelected.size();

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
					// Look if its a file to patch
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
void CPatchManager::reboot()
{
	onFileInstallFinished();

	uint nblab = 0;
	deleteFile(UpdateBatchFilename, false, false);
	FILE *fp = fopen (UpdateBatchFilename.c_str(), "wt");
	if (fp == 0)
	{
		string err = toString("Can't open file '%s' for writing: code=%d %s (error code 29)", UpdateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (err);
	}
	fprintf(fp, "@echo off\n");

	// Unpack files with category ExtractPath non empty
	const CBNPCategorySet &rDescCats = DescFile.getCategories();
	OptionalCat.clear();
	for (uint32 i = 0; i < rDescCats.categoryCount(); ++i)
	{
		// For all optional categories check if there is a 'file to patch' in it
		const CBNPCategory &rCat = rDescCats.getCategory(i);
		if (!rCat.getUnpackTo().empty())
		for (uint32 j = 0; j < rCat.fileCount(); ++j)
		{
			string rFilename = ClientPatchPath + rCat.getFile(j);
			// Extract to patch
			vector<string> vFilenames;
			if (!bnpUnpack(rFilename, ClientPatchPath, vFilenames))
			{
				/*
				string err = toString("Error unpacking %s", rFilename.c_str());
				throw Exception (err);
				*/
			}
			else
			{
				for (uint32 fff = 0; fff < vFilenames.size (); fff++)
				{
					string SrcPath = CPath::standardizeDosPath(ClientPatchPath);
					string SrcName = SrcPath + vFilenames[fff];
					string DstPath = CPath::standardizeDosPath(rCat.getUnpackTo());
					string DstName = DstPath + vFilenames[fff];
					CFile::createDirectoryTree(rCat.getUnpackTo());
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

	// Finalize batch file
	if (CFile::isExists("patch") && CFile::isDirectory("patch"))
	{
		fprintf(fp, ":looppatch\n");

		vector<string> vFileList;
		CPath::getPathContent ("patch", false, false, true, vFileList, NULL, false);
		for(uint32 i = 0; i < vFileList.size(); ++i)
			fprintf(fp, "del %s\n", CPath::standardizeDosPath(vFileList[i]).c_str());

		fprintf(fp, "rd patch\n");
		fprintf(fp, "if exist patch goto looppatch\n");
	}

	//fprintf(fp, "ping 127.0.0.1 -n 7 -w 1000 > nul\n"); // wait	

	if (_StartRyzomAtEnd)
	{
		fprintf(fp, "start %s %%1 %%2 %%3\n", RyzomFilename.c_str());
	}
	fclose(fp);

	// normal quit
	/*
	extern void quitCrashReport ();
	quitCrashReport ();
	*/
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
	strCmdLine = UpdateBatchFilename;
	/*
	if (ClientCfg.R2Mode)
	{
		//strCmdLine = UpdateBatchFilename + " " + LoginLogin + " " + LoginPassword;
	}
	else
	{
	//	strCmdLine = UpdateBatchFilename + " " + LoginLogin + " " + LoginPassword + " " + toString(LoginShardId);
	}

*/
	

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
		string str = toString("Can't execute '%s': code=%d %s (error code 30)", UpdateBatchFilename.c_str(), errno, strerror(errno));
		throw Exception (str);
	}
	
	// Close process and thread handles. 
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );
	
//	exit(0);		
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
void CPatchManager::setRWAccess (const string &filename)
{
	ucstring s = CI18N::get("uiSetAttrib") + filename;
	setState(true, s);

	if (!NLMISC::CFile::setRWAccess(filename))
	{
		s = CI18N::get("uiAttribErr") + filename + " (" + toString(errno) + "," + strerror(errno) + ")";
		setState(true, s);
		throw Exception (s.toString());
	}
}

// ****************************************************************************
string CPatchManager::deleteFile (const string &filename, bool bThrowException, bool bWarning)
{
	ucstring s = CI18N::get("uiDelFile") + filename;
	setState(true, s);

	if (!NLMISC::CFile::fileExists(filename))
	{
		s = CI18N::get("uiDelNoFile");
		setState(true, s);
		return s.toString();
	}

	if (!NLMISC::CFile::deleteFile(filename))
	{
		s = CI18N::get("uiDelErr") + filename + " (" + toString(errno) + "," + strerror(errno) + ")";
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
	ucstring s = CI18N::get("uiRenameFile") + src + " -> " + dst;
	setState(true, s);

	if (!NLMISC::CFile::moveFile(dst.c_str(), src.c_str()))
	{
		s = CI18N::get("uiRenameErr") + src + " -> " + dst + " (" + toString(errno) + "," + strerror(errno) + ")";
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
string CPatchManager::getClientVersion()
{
	if (!ValidDescFile)
		return "";

	return toString("%05d", DescFile.getFiles().getVersionNumber());
}

// ****************************************************************************
void CPatchManager::readDescFile(sint32 nVersion)
{
	DescFilename = toString("ryzom_%05d.idx", nVersion);
	string srcName = ClientPatchPath + DescFilename;
	DescFile.clear();
	if (!DescFile.load(srcName))
		throw Exception ("Can't open file '%s'", srcName.c_str ());
}

// ****************************************************************************
void CPatchManager::getServerFile (const std::string &name, bool bZipped, const std::string& specifyDestName)
{
	string srcName = name;
	if (bZipped) srcName += ".ngz";

	string dstName;
	if (specifyDestName.empty())
	{
		dstName = ClientPatchPath + CFile::getFilename(name);
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
			ucstring s = CI18N::get("uiLoginGetFile") + CFile::getFilename(serverPath+srcName);
			setState(true, s);

			// get the new file
			downloadFile (serverPath+srcName, dstName);

			downloadSuccess = true;
		}
		catch (EPatchDownloadException& e)
		{
			nlwarning("EXCEPTION CATCH: getServerFile() failed - try to find an alternative: %i: %s",UsedServer,PatchServers[UsedServer].DisplayedServerPath.c_str());

			// if emergency patch server, this is a real issue, rethrow exception
			if (UsedServer < 0)
			{
				ucstring s = CI18N::get("uiDLFailed");
				setState(true, s);
				
				throw Exception(e.what());
			}

			ucstring s = CI18N::get("uiDLURIFailed") + serverDisplayPath;
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
void CPatchManager::downloadFileWithCurl (const string &source, const string &dest)
{
#ifdef USE_CURL
	ucstring s = CI18N::get("uiDLWithCurl") + dest;
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
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadProgressFunc);
	curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
	//create Directory
	CFile::createDirectoryTree( CFile::getPath(dest) );
	// create the local file
	setRWAccess(dest);
	FILE *fp = fopen (dest.c_str(), "wb");
	if (fp == NULL)
	{
		throw Exception ("Can't open file '%s' for writing: code=%d %s (error code 37)", dest.c_str (), errno, strerror(errno));
	}
	curl_easy_setopt(curl, CURLOPT_FILE, fp);

	//CurrentFilesToGet++;

	res = curl_easy_perform(curl);

	long r;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r);

	curl_easy_cleanup(curl);

	fclose(fp);
	curl_global_cleanup();

	CurrentFile = "";

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

// ****************************************************************************
void CPatchManager::downloadFile (const string &source, const string &dest)
{
	// For the moment use only curl
	string sHeadHttp = toLower(source.substr(0,5));
	string sHeadFtp = toLower(source.substr(0,4));
	string sHeadFile = toLower(source.substr(0,5));

	if ((sHeadHttp == "http:") || (sHeadFtp == "ftp:") || (sHeadFile == "file:"))
	{
		downloadFileWithCurl(source, dest);
	}
	else
	{
		if (!CFile::copyFile(dest.c_str(), source.c_str()))
			throw Exception ("cannot copy file %s to %s", source.c_str(), dest.c_str());
	}
}

// ****************************************************************************
// TODO : revoir cette routine de decompression pour decompresser dans un fichier temporaire 
// TODO : avant d'ecraser le fichier destination

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
		int res2 = fwrite (buffer, 1, res, fp);
		//if(isVerboseLog()) nlinfo("fwrite returns %d", res2);
		if (res2 != res)
		{
			string err = toString("Can't write file '%s' : code=%d %s (error code 34)", dest.c_str(), errno, strerror(errno));

			gzclose(gz);
			fclose(fp);
			deleteFile (filename);
			throw Exception (err);
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
		ucstring s = CI18N::get("uiChangeDate") + sFilename + " " + toString(CFile::getFileModificationDate (sFilename)) + 
						" -> " + toString(nDate);
	//	setState(true,s);

		if (!CFile::setFileModificationDate(sFilename, nDate))
//		if (_utime (sFilename.c_str (), &utb) == -1)
		{
			int err = NLMISC::getLastError();
			s = CI18N::get("uiChgDateErr") + sFilename + " (" + toString(err) + ", " + NLMISC::formatErrorMessage(err) + ")";
	//		setState(true,s);
		}
		s = CI18N::get("uiNowDate") + sFilename + " " + toString(CFile::getFileModificationDate (sFilename));
	//	setState(true,s);
	}
}

// ****************************************************************************
// Get all the patches that need to be applied to a file from the description of this file given by the server
void CPatchManager::getPatchFromDesc(SFileToPatch &ftpOut, const CBNPFile &fIn, bool forceCheckSumTest)
{
	uint32 j;
	const CBNPFile rFile = fIn;
	const string &rFilename = rFile.getFileName();
	// Does the BNP exists ?
	// following lines added by Sadge to ensure that the correct file is patched
	string sFilePath;
	if (NLMISC::CFile::fileExists(ClientDataPath + rFilename))		sFilePath = ClientDataPath + rFilename;
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
		uint32 nLocalSize = CFile::getFileSize(sFilePath);
		uint32 nLocalTime = CFile::getFileModificationDate(sFilePath);
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
			ftpOut.LocalFileExists = true;
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
			ftpOut.LocalFileExists = true;
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
	uint32 nFileSize = CFile::getFileSize (SourceName);
	nlfseek64 (f, nFileSize-sizeof(uint32), SEEK_SET);
	uint32 nOffsetFromBegining;
	fread (&nOffsetFromBegining, sizeof(uint32), 1, f);
	if (nlfseek64 (f, nOffsetFromBegining, SEEK_SET) != 0)
		return false;
	
	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
		return false;
	for (uint32 i = 0; i < nNbFile; ++i)
	{
		uint8 nStringSize;
		char sName[256];
		if (fread (&nStringSize, 1, 1, f) != 1)
			return false;
		if (fread (sName, 1, nStringSize, f) != nStringSize)
			return false;
		sName[nStringSize] = 0;
		SBNPFile tmpBNPFile;
		tmpBNPFile.Name = sName;
		if (fread (&tmpBNPFile.Size, sizeof(uint32), 1, f) != 1)
			return false;
		if (fread (&tmpBNPFile.Pos, sizeof(uint32), 1, f) != 1)
			return false;
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
		DestPath = "./";
	else
		DestPath = CPath::standardizePath (dstPath);

	ucstring s = CI18N::get("uiUnpack") + SourceName + " -> " + dstPath;
	setState(true,s);

	// Read Header of the BNP File
	vector<CPatchManager::SBNPFile> Files;
	if (!readBNPHeader(SourceName, Files))
	{
		ucstring s = CI18N::get("uiUnpackErrHead") + SourceName;
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
				fread (ptr, rBNPFile.Size, 1, bnp);
				fwrite (ptr, rBNPFile.Size, 1, out);
				fclose (out);
				delete [] ptr;
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
	CPatchManager *pPM = CPatchManager::getInstance();
	double pour1 = t!=0.0?d*100.0/t:0.0;
	double pour2 = ultotal!=0.0?ulnow*100.0/ultotal:0.0;
	ucstring sTranslate = CI18N::get("uiLoginGetFile") + toString(" %s : %s / %s (%5.02f %%)", pPM->CurrentFile.c_str(), 
		NLMISC::bytesToHumanReadable((uint32)d).c_str(), NLMISC::bytesToHumanReadable((uint32)t).c_str(), pour1);
	pPM->setState(false, sTranslate);
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
		numError= val.FilesWithScanDataError.size();

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
	sTranslate = CI18N::get("uiCorruptedFile") + ftp.FileName + " (" +
		toString("%.1f ", (float)ftp.FinalFileSize/1000000.f) + CI18N::get("uiMb") + ")";
}

bool CPatchManager::unpack7Zip(const std::string &sevenZipFile, const std::string &destFileName)
{
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
	size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
	
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
	processedSize = fwrite(outBuffer + offset, 1, outSizeProcessed, outputHandle);
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

	// delete install file
	if (NLMISC::CFile::fileExists(sourceFullname))
	{
		uint32 t = CFile::getFileModificationDate(sourceFullname);	
		if (t == timestamp)
		{	//we didn't downl oad
			return true;
		}
		pPM->deleteFile(sourceFullname);
	}

	std::string extension = "";
	if (patchFullname.size() >= zsStrLength	&& patchFullname.substr(patchFullname.size() - zsStrLength) == zsStr)
	{
		extension = zsStr;
	}

	std::string patchName = tmpDirectory + NLMISC::CFile::getFilename(sourceFullname) + extension + std::string(".tmp");

	if (NLMISC::CFile::fileExists(patchName))
	{
		pPM->deleteFile(patchName);
	}

	CFile::createDirectoryTree( CFile::getPath(patchName) );
	CFile::createDirectoryTree( CFile::getPath(sourceFullname) );

	// download

	try  {
			pPM->getServerFile(patchFullname, false, patchName); 	
	} catch ( const std::exception& e)
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
		pPM->renameFile(patchName, sourceFullname);
	}			
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
	nlassert(sourcePath.size() == extractPath.size());
	CPatchManager *pPM = CPatchManager::getInstance();

	bool ok = false;
	for (uint32 j = 0; j < extractPath.size() && !ok; ++j)
	{
		if (!extractPath[j].empty())
		{	ok = true;
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
				std::string sourcePath = CFile::getPath(sourceFilename[j]);
				for (uint32 fff = 0; fff < vFilenames.size (); fff++)
				{
					string SrcPath = CPath::standardizeDosPath(sourcePath);
					string SrcName = SrcPath + vFilenames[fff];
					string DstPath = CPath::standardizeDosPath(extractPath[j]);
					string DstName = DstPath + vFilenames[fff];
					CFile::createDirectoryTree(extractPath[j]);
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

/*
	// Finalize batch file
	if (CFile::isExists("patch") && CFile::isDirectory("patch"))
	{
		fprintf(fp, ":looppatch\n");

		vector<string> vFileList;
		CPath::getPathContent ("patch", false, false, true, vFileList, NULL, false);
		for(uint32 i = 0; i < vFileList.size(); ++i)
			fprintf(fp, "del %s\n", CPath::standardizeDosPath(vFileList[i]).c_str());

		fprintf(fp, "rd patch\n");
		fprintf(fp, "if exist patch goto looppatch\n");
	}*/

	fprintf(fp, "start %s %%1 %%2 %%3\n", execName.c_str());
	fclose(fp);

	if (stopFun)
	{
		stopFun();
	}
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
	/*
	if (ClientCfg.R2Mode)
	{
		//strCmdLine = updateBatchFilename + " " + LoginLogin + " " + LoginPassword;
	}
	else
	{
	//	strCmdLine = updateBatchFilename + " " + LoginLogin + " " + LoginPassword + " " + toString(LoginShardId);
	}

*/
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



	return true;	
}


void CPatchManager::setStateListener(IPatchManagerStateListener* stateListener)
{
	_StateListener = stateListener;
}
bool CPatchManager::unpackLZMA(const std::string &lzmaFile, const std::string &destFileName)
{
	nldebug("unpackLZMA : decompression the lzma file '%s' into output file '%s", lzmaFile.c_str(), destFileName.c_str());
	CIFile inStream(lzmaFile);
	uint32 inSize = inStream.getFileSize();
	std::auto_ptr<uint8> inBuffer = std::auto_ptr<uint8>(new uint8[inSize]);
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
		Byte b;
		if (pos >= inBuffer.get()+inSize)
		{
			nlassert(false);
			return false;
		}
		fileSize |= ((UInt64)*pos++) << (8 * i);
    }

	size_t outProcessed = 0;
	size_t inProcessed = 0;
	// allocate the output buffer
	auto_ptr<uint8> outBuffer = auto_ptr<uint8>(new uint8[fileSize]);
	// decompress the file in memory
	ret = LzmaDecode(&state, (unsigned char*) pos, inSize-(pos-inBuffer.get()), &inProcessed, (unsigned char*)outBuffer.get(), fileSize, &outProcessed);
	if (ret != 0 || outProcessed != fileSize)
	{
		nlwarning("Failed to decode lzma file '%s'", lzmaFile.c_str());
		return false;
	}

	// store on output buffer
	COFile outStream(destFileName);
	outStream.serialBuffer(outBuffer.get(), fileSize);
	
	return true;
}


void CPatchManager::fatalError(const std::string& errorId, const std::string& param1, const std::string& param2)
{
	if (_AsyncDownloader)
	{
		_AsyncDownloader->fatalError(errorId, param1, param2);
	}
}

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// CCheckThread
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
CCheckThread::CCheckThread()
{
	Ended = false;
	CheckOk = false;
	TotalFileToCheck = 1;
	CurrentFileChecked = 1;
}

// ****************************************************************************
void CCheckThread::run ()
{
	CPatchManager *pPM = CPatchManager::getInstance();
	pPM->MustLaunchBatFile = false;
	try
	{
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

		if (sClientVersion != sServerVersion)
		{
			// first, try in the version subdirectory
			try
			{
				pPM->DescFilename = toString("%05u/ryzom_%05d.idx", atoi(sServerVersion.c_str()), atoi(sServerVersion.c_str()));
				// The client version is different from the server version : download new description file
				pPM->getServerFile(pPM->DescFilename, false); // For the moment description file is not zipped
			}
			catch (...)
			{
				// fallback to patch root directory
				pPM->DescFilename = toString("ryzom_%05d.idx", atoi(sServerVersion.c_str()));
				// The client version is different from the server version : download new description file
				pPM->getServerFile(pPM->DescFilename, false); // For the moment description file is not zipped
			}
		}

		// Read the description file
		pPM->readDescFile(atoi(sServerVersion.c_str()));

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
			pPM->getPatchFromDesc(ftp, rDescFiles.getFile(i), false);
			if (ftp.Patches.size() > 0)
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
				bool bFound = false;
				for (j = 0; j < pPM->OptionalCat.size(); ++j)
				{
					if (rCat.getName() == pPM->OptionalCat[j])
					{
						// remove it
						pPM->OptionalCat.erase(pPM->OptionalCat.begin()+j);
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
								// Is the real file exists  ?
								string sRealFilename = rCat.getUnpackTo() + rBNPFile.Name;
								if (NLMISC::CFile::fileExists(sRealFilename))
								{
									// Yes compare the sha1 with the sha1 of the BNP File
									CHashKey sha1BNPFile;
									nlfseek64 (bnp, rBNPFile.Pos, SEEK_SET);
									uint8 *pPtr = new uint8[rBNPFile.Size];
									fread (pPtr, rBNPFile.Size, 1, bnp);
									sha1BNPFile = getSHA1(pPtr, rBNPFile.Size);
									delete [] pPtr;
									CHashKey sha1RealFile = getSHA1(sRealFilename);
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
						
					}
				}
			}
		}

		sTranslate = CI18N::get("uiCheckEndNoErr");
		pPM->setState(true, sTranslate);
		CheckOk = true;
		Ended = true;
	}
	catch (Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CCheckThread::run() failed");
		ucstring sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
		pPM->setState(true, sTranslate);
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
CPatchThread::CPatchThread()
{
	Ended = false;
	PatchOk = false;
	CurrentFilePatched = 0;
	PatchSizeProgress = 0;
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

	CurrentFilePatched = 0;

	try 
	{

		// First do all ref files
		// ----------------------

		for (i = 0; i < AllFilesToPatch.size(); ++i)
		{
			CPatchManager::SFileToPatch &rFTP = AllFilesToPatch[i];
			string ext = rFTP.FileName.substr(rFTP.FileName.rfind('.'), rFTP.FileName.size());
			if (ext == ".ref")
			{
				processFile (rFTP);
				pPM->MustLaunchBatFile = true;
				CurrentFilePatched++;
			}
		}

		// Second do all bnp files
		// -----------------------

		for (i = 0; i < AllFilesToPatch.size(); ++i)
		{
			CPatchManager::SFileToPatch &rFTP = AllFilesToPatch[i];
			string ext = rFTP.FileName.substr(rFTP.FileName.rfind('.'), rFTP.FileName.size());
			if (ext == ".bnp")
			{
				processFile (rFTP);
				pPM->MustLaunchBatFile = true;
				CurrentFilePatched++;
			}
		}

	}
	catch(Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CPatchThread::run() failed");
		pPM->setState(true, ucstring(e.what()));
		bErr = true;
	}

	// Unpack all files with the UnpackTo flag
	// ---------------------------------------
	// To do that we create a batch file that will copy all files (unpacked to patch directory) 
	// to the directory we want (the UnpackTo string)

	// Recreate batch file
	// If asyncDownloader (download by bittorent its not the  
	if (!pPM->MustLaunchBatFile && !pPM->getAsyncDownloader())
	{
		pPM->deleteFile(pPM->UpdateBatchFilename, false, false);
	}

	ucstring sTranslate;
	if (!bErr)
		sTranslate = CI18N::get("uiPatchEndNoErr");
	else
		sTranslate = CI18N::get("uiPatchEndWithErr");

	pPM->setState(true, sTranslate);
	PatchOk = !bErr;
	Ended = true;
}

// ****************************************************************************
void CPatchThread::processFile (const CPatchManager::SFileToPatch &rFTP)
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
			if (SourceName.empty())	throw Exception (std::string("ERROR: Failed to find file: ")+rFTP.FileName);

// following lines removed by Sadge to ensure that the correct file gets patched
//			SourceName = CPath::lookup(rFTP.FileName); // exception if file do not exists
		}
		else
			SourceName = pPM->ClientDataPath + rFTP.FileName;
	}
	else
	{
		SourceName = pPM->ClientPatchPath + rFTP.FileName;
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
		pPM->getAsyncDownloader()->addToDownloadList(PatchName, SourceName, rFTP.LastFileDate, rFTP.ExtractPath, rFTP.SZFileSize,  rFTP.FinalFileSize );							
	}
	else
	{
	string OutFilename;
	bool usePatchFile = true;

	// compute the total size of patch to download
	uint32 totalPatchSize = 0;
	for (uint i=0; i<rFTP.PatcheSizes.size(); ++i)
		totalPatchSize += rFTP.PatcheSizes[i];

	// look for the source file, if not present or invalid (not matching 
	// a known version) or if total patch size greater than half the 
	// uncompressed final size then load the 7 zip file
	if (!rFTP.LocalFileExists || totalPatchSize > rFTP.FinalFileSize / 2)
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
					pPM->getServerFile(toString("%05u/", rFTP.Patches.back())+lzmaFile);
				}
				catch(...)
				{
					// failed with version subfolder, try in the root patch directory
					pPM->getServerFile(lzmaFile);
				}
			}
			catch (...)
			{
				// can not load the 7zip file, use normal patching
				usePatchFile = true;
				break;
			}

			OutFilename = pPM->ClientPatchPath + CFile::getFilename(rFTP.FileName);
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
			pPM->renameFile(OutFilename+".tmp", SourceName);
		}
	}
	
	if (usePatchFile)
	{
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
				pPM->getServerFile(PatchName);
				// remove the subfolder name
				PatchName = CFile::getFilename(PatchName);
			}
			catch (...)
			{
				// fallback to patch root directory
				PatchName = rFTP.FileName.substr(0, rFTP.FileName.size()-4);
				PatchName += toString("_%05d", rFTP.Patches[j]) + ".patch";
				sTranslate = CI18N::get("uiLoginGetFile") + " " + PatchName;
				pPM->setState(true, sTranslate);
				pPM->getServerFile(PatchName);
			}
			
			// Apply the patch
			// If the patches are not to be applied on the last version
			string SourceNameXD = SourceName;
			if (!rFTP.Incremental)
			{
				// Find the reference file to apply the patch
				SourceNameXD = rFTP.FileName;
				SourceNameXD = SourceNameXD.substr(0, SourceNameXD.rfind('.'));
				SourceNameXD += "_.ref";
				
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
			
			OutFilename = pPM->ClientPatchPath + rFTP.FileName + ".tmp";
			
			sTranslate = CI18N::get("uiApplyingDelta") + " " + PatchName;
			pPM->setState(true, sTranslate);
			xDeltaPatch(PatchName, SourceNameXD, OutFilename);
			
			pPM->deleteFile(PatchName);
			PatchSizeProgress += rFTP.PatcheSizes[j];

			if (rFTP.LocalFileExists)
				pPM->deleteFile(SourceName);
		
			// rename tmp file into final file (in unpack folder)
			pPM->deleteFile(SourceName, false, false); // File can exists if bad BNP loading
			pPM->renameFile(OutFilename, SourceName);
		}			
	}

	// If all patches applied with success so file size should be ok
	// We just have to change file date to match the last patch applied
	pPM->applyDate(SourceName, rFTP.LastFileDate);
}
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
/*
	if (!CXDeltaPatch::apply(patch, src, out, &patchingCB))
	{
		string str = toString("Error applying %s to %s giving %s", patch.c_str(), src.c_str(), out.c_str());
		throw Exception (str);
	}
	*/


	// Launching xdelta.exe 
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
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
	{
		// error occurs during the launch
		string str = toString("Can't execute '%s'", strCmdLine.c_str());
		throw Exception (str);
	}

	// Wait for the process to terminate
	DWORD dwTimeout = 1000 * 300; // 5 min = 300 s
	DWORD nRetWait =  WaitForSingleObject(pi.hProcess, dwTimeout);

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
			pPM->getPatchFromDesc(ftp, rDescFiles.getFile(i), true);
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
	catch (Exception &e)
	{
		nlwarning("EXCEPTION CATCH: CScanDataThread::run() failed");
		ucstring sTranslate = CI18N::get("uiCheckEndWithErr") + " " + e.what();
		pPM->setState(true, sTranslate);
		CheckOk = false;
		Ended = true;
	}
}

// ****************************************************************************
void CDownloadThread::run()
{
	CPatchManager *pPM = CPatchManager::getInstance();
	
	std::string patchPath = CPath::standardizePath (std::string("./")+TheTmpInstallDirectory)+std::string("patch/");


	static bool _FirstTime = true;
	static uint64 _FullSize = 0;
	static uint64 _CurrentSize = 0;
	static uint32 _Start;
	if (_FirstTime)
	{
		for (unsigned int first = 0,last = _Entries.size() ; first != last; ++first)
		{
			_FullSize += _Entries[first].SZipFileSize;
		}
		_Start = NLMISC::CTime::getSecondsSince1970();
		_FirstTime = false;
	}


	for (unsigned int first = 0,last = _Entries.size() ; first != last; ++first)
	{
		std::string patchName = CPath::standardizePath (_Entries[first].PatchName, false);
		std::string sourceName = CPath::standardizePath (_Entries[first].SourceName, false);
		uint32 lastFileDate = _Entries[first].Timestamp;						
	
		_CurrentSize += _Entries[first].SZipFileSize;

		uint32 rate = 0;

		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			long long size = _CurrentSize;
			rate = uint32(_CurrentSize / dt);
		}
		/*
		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			long long size = _Entries[first].SZipFileSize;
			rate = uint32(size / dt);
			_Start = NLMISC::CTime::getSecondsSince1970();
		}
		*/
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
			if (CFile::fileExists(finalFile))
			{
				unsigned long long fz = CFile::getFileSize(finalFile);
				uint32 timestamp = _Entries[first].Timestamp;
				uint32 timestamp2 = CFile::getFileModificationDate(finalFile);
				if ( fz == _Entries[first].SZipFileSize && timestamp == timestamp2)
				{
					toDownload = false;
				}
				else
				{
					CFile::deleteFile(finalFile);
				}			
			}
			if (toDownload)
			{
				if (CFile::fileExists(tmpFile))
				{
					CFile::deleteFile(tmpFile);
				}
				std::string path = CFile::getPath(tmpFile);
				CFile::createDirectoryTree(path);
				try  {
					pPM->getServerFile(patchName, false, tmpFile); 	
					CFile::moveFile(finalFile.c_str(), tmpFile.c_str());
					pPM->applyDate(finalFile, _Entries[first].Timestamp);
				} catch ( const std::exception& e)
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
	pPM->onFileDownloadFinished();

}

void CInstallThread::run()
{
	std::string patchPath = CPath::standardizePath (std::string("./")+TheTmpInstallDirectory)+std::string("patch/");
	CPatchManager *pPM = CPatchManager::getInstance();

	std::set<std::string> allowed;
	
	unsigned int first, last;

	for (first = 0,last = _Entries.size() ; first != last; ++first)
	{
		std::string correct = CPath::standardizePath (patchPath + _Entries[first].PatchName, false);
		allowed.insert(correct);
	}


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
	if (_FirstTime)
	{
		for (unsigned int first = 0,last = _Entries.size() ; first != last; ++first)
		{
			_FullSize += _Entries[first].Size;
		}
		_Start = NLMISC::CTime::getSecondsSince1970();
		_FirstTime = false;
	}


	for (first = 0,last = _Entries.size() ; first != last; ++first)
	{
		std::string patchName = CPath::standardizePath (patchPath +_Entries[first].PatchName, false);
		std::string sourceName = CPath::standardizePath (_Entries[first].SourceName, false);
		uint32 lastFileDate = _Entries[first].Timestamp;

		
		_CurrentSize += _Entries[first].Size;
		uint32 rate = 0;
		
		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			long long size = _CurrentSize / dt;
			rate = uint32 (size);
		}
		/*
		uint32 dt = NLMISC::CTime::getSecondsSince1970() - _Start;
		if (dt)
		{
			long long size = _Entries[first].Size;
			rate = uint32(size / dt);
			_Start = NLMISC::CTime::getSecondsSince1970();
		}
	*/	

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
				if (patchName.size() >= zsStrLength
					&& patchName.substr(patchName.size() - zsStrLength) == zsStr)
				{
					std::string outFilename = patchName.substr(0, patchName.size() - zsStrLength);
					std::string localOutFilename =  CPath::standardizeDosPath(outFilename);
		
					if ( CPatchManager::unpackLZMA(patchName, localOutFilename) )
					{
						pPM->deleteFile(patchName);
						pPM->renameFile(outFilename, sourceName);
						pPM->applyDate(sourceName, lastFileDate);
					}
					else
					{
						throw std::exception("Can not unpack");
					}
					
				}
				else
				{
					pPM->renameFile(patchName, sourceName);
					pPM->applyDate(sourceName, lastFileDate);
				}						
			} catch ( const std::exception& e)
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
		pPM->onFileInstalling(sourceName, rate, first+1, last, _CurrentSize, _FullSize);

	}
	// extract bnp

	{
		std::string install = CPath::standardizePath  (std::string("./")+TheTmpInstallDirectory);

		std::vector<std::string> vFiles;
		CPath::getPathContent(install, true, false, true, vFiles);	
		for (uint32 i = 0; i < vFiles.size(); ++i)
		{
			NLMISC::CFile::deleteFile(vFiles[i]);
		}


		
		do {
			vFiles.clear();
			CPath::getPathContent(install, true, true, false, vFiles);				
			for (uint32 i = 0; i < vFiles.size(); ++i)
			{
				NLMISC::CFile::deleteDirectory(vFiles[i]);
			}
		} while ( !vFiles.empty() );
		NLMISC::CFile::deleteDirectory(install);

		if (NLMISC::CFile::fileExists("libtorrent_logs"))
		{
			do {
				vFiles.clear();
				CPath::getPathContent("libtorrent_logs", true, true, false, vFiles);				
				for (uint32 i = 0; i < vFiles.size(); ++i)
				{
					NLMISC::CFile::deleteDirectory(vFiles[i]);
				}
			} while ( !vFiles.empty() );
			NLMISC::CFile::deleteDirectory("libtorrent_logs");
		}

		
	}



	pPM->reboot(); // do not reboot run a .bat 
}

