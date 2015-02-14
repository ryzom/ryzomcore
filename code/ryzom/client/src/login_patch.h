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

#ifndef CL_PATCH_H
#define CL_PATCH_H

#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/progress_callback.h"

#include "game_share/bnp_patch.h"

#include "login_xdelta.h"

extern NLMISC::CConfigFile ConfigFile;

class CPatchThread;
class CCheckThread;
class CScanDataThread;
class CInstallThread;


// Useful for using an external downloader (BitTorrent) use of interface with CGameDownloader from client_background_rd.exe using as install program
// Forward all messages to CRyzomControl that will forward to CGameDownloader (The engine of the install software)
class IAsyncDownloader
{
public:
	virtual ~IAsyncDownloader(){}
	virtual void addToDownloadList(const std::string &/* patchName */, const std::string &/* sourceName */, uint32 /* timestamp */, const std::string& /* extractPath */, uint32 /* sevenZipSize */, uint32 /* size */){}
	virtual void onFileInstall(const std::string& /* filename */, uint32 /* index */, uint32 /* filecount */){}
	virtual void onFileInstallFinished(){}
	virtual void onFileDownloading(const std::string& /* sourceName */, uint32 /* rate */, uint32 /* fileIndex */, uint32 /* fileCount */, uint64 /* fileSize */, uint64 /* fullSize */){}
	virtual void onFileInstalling(const std::string& /* sourceName */, uint32 /* rate */, uint32 /* fileIndex */, uint32 /* fileCount */, uint64 /* fileSize */, uint64 /* fullSize */){}
	virtual void onFileDownloadFinished(){}
	virtual void fatalError(const std::string& /* errorId */, const std::string& /* param1 */, const std::string& /* param2 */){}

};
// To define alternate log
class IPatchManagerStateListener
{
public:
	virtual void setState (bool /* bOutputToLog */, const ucstring &/* ucsNewState */){}
};

// Get Info of file to install
class CInstallThreadEntry
{
public:
	CInstallThreadEntry(){ Timestamp = 0; }
	CInstallThreadEntry(const char* patchName, const char* sourceName, uint32 timestamp, const char* extractPath, uint32 size, uint32 sZipFileSize)
		:PatchName(patchName), SourceName(sourceName), Timestamp(timestamp),Size(size),SZipFileSize(sZipFileSize),ExtractPath(extractPath) {}
	std::string PatchName;
	std::string SourceName;
	uint32 Timestamp;
	uint32 Size;
	uint32 SZipFileSize;
	std::string ExtractPath;
};


/**
 * class managing the patch system
 * \author Matthieu 'TrapII' Besson
 * \author Vianney 'Ace' Lecroart
 * \author Nevrax France
 * \date July 2004
 */
class CPatchManager
{

public:

	struct SFileToPatch
	{
		std::string			FileName;			// Dest file for the patch
		std::string			SrcFileName;		// Optional src file to which apply patch (empty unless some version of the file, possibly with
												// all patchs applied, was found in the /unpack directory due to a download by the background downloader)
		std::vector<uint32>	Patches;
		std::vector<uint32>	PatcheSizes;
		uint32				LastFileDate;		// Used during patching to set the date of the file to the Date of the last patch
		bool				LocalFileToDelete;	// Used during patching to know if we have to delete local file prior to apply patches
		bool				LocalFileExists;
		bool				Incremental;		// Used during patching to know if we have to xdelta against a ref file
		std::string			ExtractPath;		// Used during patching to extract a bnp file to a specific directory
		uint32				FinalFileSize;		// just for info, the final size after patch
		uint32				SZFileSize;			// Size of the SZ file
	};

	struct SBNPFile
	{
		std::string	Name;
		uint32		Size;
		uint32		Pos;
	};

	struct SPatchInfo
	{
		struct SCat
		{
			std::string Name;
			uint32		Size;	// Patch size for this category (all files to patch)
			sint32		Req;	// -1 or a value in Required cat (used only for optcat)
			uint32	SZipSize;	// SevenZipSize
			uint32	FinalFileSize; // uncompressed SevenZipSize
			SCat() { Size = 0; Req = -1; SZipSize = 0; FinalFileSize=0;}
		};

		std::vector<SCat>	NonOptCat;
		std::vector<SCat>	ReqCat;
		std::vector<SCat>	OptCat;


		void clear()
		{
			NonOptCat.clear();
			ReqCat.clear();
			OptCat.clear();
		}

		// return bitfield of available patchs, indexed by BGDownloader::TDownloadID
		uint32 getAvailablePatchsBitfield() const;
	};

public:

	/// Singleton method : Get the unique patch manager instance
	static CPatchManager* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CPatchManager();
		return _Instance;
	}

	// serverPath is the name where to find all patches, files description and so on
	// serverVersion is a string describing the version of the description file
	void init(const std::vector<std::string>& patchURIs, const std::string &serverPath, const std::string &serverVersion);

	/// Get the version with the Description file (must be done after init)
	std::string getClientVersion ();

	bool isVerboseLog() { return VerboseLog; }

	void setVerboseLog(bool b) { VerboseLog = b; }

	// Get the string information about what the threads are doing
	// Return true if the state has changed
	bool getThreadState (ucstring &state, std::vector<ucstring> &stateLog);

	/** Get the error message (filled after a patch of check)
	  * May be empty if the cause of error is unknown or unhandled
	  */
	const ucstring &getLastErrorMessage() { return _ErrorMessage; }

	// ---------------------
	// First Part : Checking
	// ---------------------

	// start the check of bnp (download description file DescFilename, look at the version of bnps)
	// init must have been done
	void startCheckThread(bool includeBackgroundPatch);

	// if the checkThread ended and is ok then the getDesc* methods can be called
	bool isCheckThreadEnded(bool &ok);

	// Get all the optional categories to display for patching
	void getInfoToDisp(SPatchInfo &piOut);

	void forceStopCheckThread();
	// Used by installation software
	void startInstallThread(const std::vector<CInstallThreadEntry>& entries);
	// Used by installation software
	void startDownloadThread(const std::vector<CInstallThreadEntry>& entries);
	// ----------------------------------------------
	// Second Part : Downloading and Applying patches
	// ----------------------------------------------

	/** start to patch with a set of files
      * \param commitPatch When true, the patched version of the file will replace the original file, else the patched version
	  *                    will be left in the /unpack directory and the previous version in /data will remain unchanged.
	  *					   The background downloader should set this to false.
	  *
	  */
	void startPatchThread(const std::vector<std::string> &CategoriesSelected, bool commitPatch);

	// Return true if patch has ended
	// Ok is set to true if the patch is ended and the patch process is ok (if the patch is already ended ok is false)
	bool isPatchThreadEnded (bool &ok);


	void forceStopPatchThread();

	bool mustLaunchBatFile() const { return MustLaunchBatFile; }
	void createBatchFile(CProductDescriptionForClient &descFile, bool wantRyzomRestart = true, bool useBatchFile = true);
	void executeBatchFile();
	void deleteBatchFile();
	void reboot();

	// TODO : modify it for the second part ...
	// Maybe we can't get patchs size ... currently, it only returns 1!

	int getTotalFilesToGet();
	int getCurrentFilesToGet();
	int getPatchingSize();

	// Return % of current file that has been downloaded/patched so far
	float getCurrentFileProgress() const;

	bool isDownloadInProgress() { return DownloadInProgress; }

	// ----------------------------------------------
	// ScanData Part : optional task which verify all data
	// ----------------------------------------------

	// start the full check of BNP
	void startScanDataThread();

	// if the scan data thread has ended
	bool isScanDataThreadEnded(bool &ok);

	// ask to stop the Scan Data thread. NB: for security, the thread will continue to the current scanned file,
	// then stop. Hence, you must still wait isScanDataThreadEnded() return true
	void askForStopScanDataThread();

	// If the scan thread is not running, touch the files with error, and return the number of corrupted files
	uint applyScanDataResult();

	// get the current info Log for data Scan (true if some change from last get, else text is not filled)
	bool getDataScanLog(ucstring &text);

	CProductDescriptionForClient &getDescFile() { return DescFile; }

	NLMISC::IThread *getCurrThread() const { return thread; }
	// set an external state listener (enable to log) infos
	void setStateListener(IPatchManagerStateListener* stateListener);

	// set an external downloader (useful for installer using BitTorrent protocole)
	void setAsyncDownloader(IAsyncDownloader* asyncDownloader);
	// get the external downloader (useful for installer using BitTorrent protocole or NULL)
	IAsyncDownloader*  getAsyncDownloader() const;

	// By default the name used is the name of the current executable.
	// But for external torrent downloader we must set "client_ryzom_rd.exe"
	void setRyzomFilename(const std::string& ryzomFilename) { RyzomFilename = ryzomFilename; }

	// Used when client data is not located in current directory
	void setClientRootPath(const std::string& clientRootPath);

	// Used by installation software to download a file only if necessary
	static bool download(const std::string& patchPath, const std::string& sourcePath,
									  const std::string& tmpDirectory, uint32 timestamp);
	// Used by installation software to create install.bat file (not used)
	static bool extract(const std::string& patchPath,
							const std::vector<std::string>& sourceFilename,
							const std::vector<std::string>& extractPath,
							const std::string& updateBatchFilename,
							const std::string& exectName,
							void (*stopFun)() );
	// Installation software must be able to indicate if ryzom must be stated at end of installation
	void setStartRyzomAtEnd(bool startAtEnd){ _StartRyzomAtEnd = startAtEnd; }
	// Forward message to installation software if needed
	void fatalError(const std::string& errorId, const std::string& param1, const std::string& param2);

	const std::string &	getServerVersion () { return ServerVersion; }
private:

	// Methods used by patch & check threads
	// -------------------------------------

	friend class CCheckThread;
	friend class CPatchThread;
	friend class CScanDataThread;
	friend class CInstallThread;
	friend class CDownloadThread;
	friend class CPatchThreadDownloadProgress;

	// Set the thread state (called by threads to let us know what they are doing)
	void setState (bool bOutputToLog, const ucstring &ucsState);
	void touchState();

	/// Get the version of the server given during init()


	/// Read the description file (throw exception if it doesn't exist)
	void				readDescFile(sint32 nVersion);

	/// Read the description of the highest client version file found
	void				readClientVersionAndDescFile();

	void		setRWAccess (const std::string &filename, bool bThrowException=true);

	std::string deleteFile (const std::string &filename, bool bThrowException=true, bool bWarning=true);

	// Rename src to dst throw an exception if we cant
	void renameFile (const std::string &src, const std::string &dst);

	// Accessors used by the patch thread
	const std::string &getDescFilename() { return DescFilename; }
	const std::string &getUpdateBatchFilename() { return UpdateBatchFilename; }
	const std::string &getRyzomFilename() { return RyzomFilename; }

	CPatchThread *getPatchThread() { return PatchThread; }

	// Get a file from the server and decompress it if zipped
	void getServerFile (const std::string &name, bool bZipped = false, const std::string& destName="", NLMISC::IProgressCallback *progress = NULL);
	void downloadFileWithCurl (const std::string &source, const std::string &dest, NLMISC::IProgressCallback *progress = NULL);
	void downloadFile (const std::string &source, const std::string &dest, NLMISC::IProgressCallback *progress = NULL);
	// Decompress zipped file override destination file
	void decompressFile (const std::string &filename);
	void applyDate (const std::string &sFilename, uint32 nDate);

	void getPatchFromDesc(SFileToPatch &ftpOut, const CBNPFile &fIn, bool forceCheckSumTest);

	bool readBNPHeader(const std::string &Filename, std::vector<SBNPFile> &FilesOut);
	bool bnpUnpack(const std::string &srcBigfile, const std::string &dstPath, std::vector<std::string> &vFilenames);

	// stop the threads (called when knowing the thread ended)
	void stopCheckThread();
	void stopPatchThread();
	void stopScanDataThread();

	// add a file to the scan data log
	void addDataScanLogCorruptedFile(const SFileToPatch &ftp);
	void clearDataScanLog();
	static void getCorruptedFileInfo(const SFileToPatch &ftp, ucstring &sTranslate);

	// utility func to decompress a monofile 7zip archive
	static bool unpack7Zip(const std::string &sevenZipFile, const std::string &destFileName);
	// utility func to decompress a single LZMA packed file
	static bool unpackLZMA(const std::string &sevenZipFile, const std::string &destFileName);
	static bool downloadAndUnpack(const std::string& patchPath, const std::string& sourceFilename, const std::string& extractPath, const std::string& tmpDirectory, uint32 timestamp);
	// Forward message to Installation Software
	void onFileInstallFinished();
	void onFileDownloadFinished();
	void onFileDownloading(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize);
	void onFileInstalling(const std::string& sourceName, uint32 rate, uint32 fileIndex, uint32 fileCount, uint64 fileSize, uint64 fullSize);

private:

	static int downloadProgressFunc(void *foo, double t, double d, double ultotal, double ulnow);
	static int validateProgress(void *foo, double t, double d, double ultotal, double ulnow);

	class MyPatchingCB : public CXDeltaPatch::ICallBack
	{
	public:
		std::string patchFilename;
		std::string srcFilename;

		virtual void progress(float f);
	};
	friend class CPatchManager::MyPatchingCB;

private:

	/// Constructor
	CPatchManager();

	/// The singleton's instance
	static CPatchManager* _Instance;

	class CPatchServer
	{
	public:

		CPatchServer(const std::string& serverPath)
		{
			ServerPath = serverPath;
			Available = true;

			if (ServerPath.size()>0 && ServerPath[ServerPath.size()-1] != '/')
				ServerPath += '/';

			std::string::size_type pos = ServerPath.find ("@");
			if (pos != std::string::npos)
				DisplayedServerPath = "http://" + ServerPath.substr (pos+1);
			else
				DisplayedServerPath = ServerPath;
		}

		std::string	ServerPath;
		std::string	DisplayedServerPath;
		bool		Available;
	};

	// Patch Servers
	std::vector<CPatchServer>	PatchServers;
	sint						UsedServer;

	// Server desc
	std::string ServerPath;
	std::string DisplayedServerPath;
	std::string ServerVersion;

	bool DownloadInProgress;

	/// Description file
	std::string						DescFilename;
	bool							ValidDescFile;
	CProductDescriptionForClient	DescFile;
	std::vector<SFileToPatch>		FilesToPatch;
	std::vector<std::string>		OptionalCat;

	ucstring						_ErrorMessage;

	// Threads
	CPatchThread	*PatchThread;
	CCheckThread	*CheckThread;
	CScanDataThread	*ScanDataThread;
	CInstallThread	*InstallThread;
	NLMISC::IThread	*thread;

	// State
	struct CState
	{
		ucstring					State;
		std::vector<ucstring>		StateLog;
		bool						StateChanged;
		CState()
		{
			StateChanged= false;
		}
	};
	NLMISC::CSynchronized<CState>	State;
	std::string LogSeparator;
	std::string CurrentFile;


	// Useful pathes and names

	/// Now deprecated : the launcher is the client ryzom
	std::string RyzomFilename;
	std::string UpdateBatchFilename;

	// Where the client get all delta and desc file
	std::string ClientPatchPath;
	std::string ClientDataPath;

	/// Output useful information for debugging in the log file
	bool VerboseLog;

	bool MustLaunchBatFile; // And then relaunch ryzom

	// For Data Scan, list of files with checksum error => should delete them
	struct CDataScanState
	{
		std::vector<SFileToPatch>		FilesWithScanDataError;
		bool							Changed;
		CDataScanState()
		{
			Changed= false;
		}
	};
	typedef	NLMISC::CSynchronized<CDataScanState>	TSyncDataScanState;
	TSyncDataScanState					DataScanState;
	// The date to apply when file is buggy: use default of year 2001, just to have a coherent date
	enum	{DefaultResetDate= 31 * 366 * 24 * 3600};
	// The file size of a an empty BNP
	enum	{EmptyBnpFileSize= 8};
	// Use an external downloader: (BitTorrent)
	IAsyncDownloader* _AsyncDownloader;
	IPatchManagerStateListener* _StateListener;// Alternate log
	bool _StartRyzomAtEnd;
public:
	// used by threads to signal error at the end of execution
	void setErrorMessage(const ucstring &message);
};

/**
 *
 * \author Matthieu 'TrapII' Besson
 * \author Vianney 'Ace' Lecroart
 * \author Nevrax France
 * \date July 2004
 */
class CCheckThread : public NLMISC::IRunnable
{

public:

	CCheckThread(bool includeBackgroundPatch);

public:

	bool		Ended;					// true if the thread have ended
	bool		CheckOk;				// true if the check is good
	bool		StopAsked;

	int TotalFileToCheck;
	int CurrentFileChecked;
	bool IncludeBackgroundPatch;

private:

	void run();

};

/**
 * class managing the patch getting files from server and applying patches
 * \author Matthieu 'TrapII' Besson
 * \author Vianney 'Ace' Lecroart
 * \author Nevrax France
 * \date July 2004
 */
class CPatchThread : public NLMISC::IRunnable
{

public:

	bool		Ended;					// true if the thread have ended the patch
	bool		PatchOk;				// true if the patch was good
	bool		StopAsked;

public:

	CPatchThread(bool commitPatch);

	// Clear the list of files to patch
	void clear();

	// Add files to be patched (avoid multiple time the same file)
	void add(const CPatchManager::SFileToPatch &ftp);

	int getNbFileToPatch() { return (int)AllFilesToPatch.size(); }
	int getCurrentFilePatched() { return (int) floorf(CurrentFilePatched); }
	int getPatchingSize() { return PatchSizeProgress; }
	float getCurrentFileProgress() const { return fmodf(CurrentFilePatched, 1.f); }

private:

	std::vector<CPatchManager::SFileToPatch>	AllFilesToPatch;
	float CurrentFilePatched;
	int	  PatchSizeProgress;
	bool _CommitPatch;


	friend class CPatchThreadDownloadProgress;
private:

	void run();
	void processFile (CPatchManager::SFileToPatch &rFTP);
	void xDeltaPatch(const std::string &patch, const std::string &src, const std::string &out);

};
/**
* This thread peforms the installation of file that have been downloaded via asynchrous download (BitTorrent)
* It unzip, delete obsolete file, extract file from bnp
*/


class CInstallThread : public NLMISC::IRunnable
{
public:
	CInstallThread(const std::vector<CInstallThreadEntry> entries):_Entries(entries){}
	void run();
	std::vector<CInstallThreadEntry> _Entries;
};

class CDownloadThread : public NLMISC::IRunnable
{
public:
	CDownloadThread(const std::vector<CInstallThreadEntry> entries):_Entries(entries){}
	void run();
	std::vector<CInstallThreadEntry> _Entries;
};

/**
 *	This thread perform a checksum check on all existing files registered in the local .idx
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date July 2004
 */
class CScanDataThread : public NLMISC::IRunnable
{

public:

	CScanDataThread();

public:

	// Written by MainThread, read by thread
	bool		AskForCancel;			// true if the main thread ask to cancel the task

	// Written by thread, read by Main Thread
	bool		Ended;					// true if the thread have ended
	bool		CheckOk;				// true if the check is good
	int			TotalFileToScan;
	int			CurrentFileScanned;

private:

	void run();

};


#endif // CL_PATCH_H

/* End of login_patch.h */
