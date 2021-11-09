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

#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/thread.h"
#include "nel/misc/sha1.h"
#include "nel/misc/big_file.h"
#include "nel/misc/i18n.h"

#include "data_scan.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


// ***************************************************************************
static	ucstring	dummyI18N(const std::string &s)
{
	return s;
}


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

// ****************************************************************************
CPatchManager::CPatchManager() : State("t_state"), DataScanState("t_data_scan_state")
{
	DescFilename = "ryzom_xxxxx.idx";

	ClientPatchPath = "./unpack/";
	ClientDataPath = "./data/";


	VerboseLog = true;

	ScanDataThread = NULL;
	thread = NULL;

	ValidDescFile = false;
}

// ****************************************************************************
void CPatchManager::init()
{
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
		// Not important that there is no desc file
	}
}

// ****************************************************************************
// Called in main thread
bool CPatchManager::getThreadState (ucstring &stateOut, vector<ucstring> &stateLogOut)
{
	if (ScanDataThread==NULL)
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
int CPatchManager::getTotalFilesToGet()
{
	if (ScanDataThread != NULL)
		return ScanDataThread->TotalFileToScan;
	
	return 1;
}

// ****************************************************************************
int CPatchManager::getCurrentFilesToGet()
{
	if (ScanDataThread != NULL)
		return ScanDataThread->CurrentFileScanned;
	
	return 1;
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
// Get all the patches that need to be applied to a file from the description of this file given by the server
void CPatchManager::getPatchFromDesc(SFileToPatch &ftpOut, const CBNPFile &fIn, bool forceCheckSumTest)
{
	uint32 j;
	const CBNPFile rFile = fIn;
	const string &rFilename = rFile.getFileName();
	// Does the BNP exists ?
	string sFilePath = CPath::lookup(rFilename);
	if (sFilePath.empty())
	{
		if (NLMISC::CFile::fileExists(ClientPatchPath + rFilename))
			sFilePath = ClientPatchPath + rFilename;
	}

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
			ucstring sTranslate = dummyI18N("Checking Integrity :") + " " + rFilename;
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
					// break; // ace -> same as above
				}
			}
		}
		
		// No version available found
		if (nVersionFound == 0xFFFFFFFF)
		{
			ucstring sTranslate = dummyI18N("No Version Found");
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
			}
		}
		else // A version of the file has been found
		{
			ucstring sTranslate = dummyI18N("Version Found :") + " " + toString(nVersionFound);
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
				}
			}
			// Else this file is up to date !

			// For info, get its final file size
			ftpOut.FinalFileSize= rFile.getVersion(rFile.versionCount()-1).getFileSize();
		}
	} // end of else local BNP file exists 
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
	sTranslate = dummyI18N("Corrupted File: ") + ftp.FileName + " (" +
		toString("%.1f ", (float)ftp.FinalFileSize/1000000.f) + dummyI18N("Mb") + ")";
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
		ucstring sTranslate = dummyI18N("Client Version") + " (" + sClientVersion + ") ";
		pPM->setState(true, sTranslate);
		
		// For all bnp in the description file get all patches to apply 
		// depending on the version of the client bnp files
		const CBNPFileSet &rDescFiles = pPM->DescFile.getFiles();
		TotalFileToScan = rDescFiles.fileCount();
		for (i = 0; i < rDescFiles.fileCount(); ++i)
		{
			sTranslate = dummyI18N("Checking File") + " " + rDescFiles.getFile(i).getFileName();
			pPM->setState(true, sTranslate);

			// get list of file to apply to this patch, performing a full checksum test (slow...)
			CPatchManager::SFileToPatch ftp;
			pPM->getPatchFromDesc(ftp, rDescFiles.getFile(i), true);
			// if the file has been found but don't correspond to any local version (SHA1)
			if (ftp.LocalFileExists && ftp.LocalFileToDelete)
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

		sTranslate = dummyI18N("Checking file ended with no error");
		pPM->setState(true, sTranslate);
		CheckOk = true;
		Ended = true;
	}
	catch (Exception &e)
	{
		ucstring sTranslate = dummyI18N("Checking file ended with errors :") + " " + e.what();
		pPM->setState(true, sTranslate);
		CheckOk = false;
		Ended = true;
	}
}

