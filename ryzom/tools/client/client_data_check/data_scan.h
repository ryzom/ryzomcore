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
#include "../game_share/bnp_patch.h"
#include <vector>

class CScanDataThread;

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
		std::string			FileName;
		std::vector<uint32>	Patches;
		std::vector<uint32>	PatcheSizes;
		uint32				LastFileDate;		// Used during patching to set the date of the file to the Date of the last patch
		bool				LocalFileToDelete;	// Used during patching to know if we have to delete local file prior to apply patches
		bool				LocalFileExists;
		bool				Incremental;		// Used during patching to know if we have to xdelta against a ref file
		std::string			ExtractPath;		// Used during patching to extract a bnp file to a specific directory
		uint32				FinalFileSize;		// just for info, the final size after patch
	};

public:

	/// Singleton method : Get the unique patch manager instance
	static CPatchManager* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CPatchManager();
		return _Instance;
	}
	
	// init
	void init();

	/// Get the version with the Description file (must be done after init)
	std::string getClientVersion ();

	bool isVerboseLog() { return VerboseLog; }
	
	void setVerboseLog(bool b) { VerboseLog = b; }

	// Get the string information about what the threads are doing 
	// Return true if the state has changed
	bool getThreadState (ucstring &state, std::vector<ucstring> &stateLog);

	// TODO : Revoir ca pour la seconde partie ...
	// On a peut etre pas les informations de taille des patches ... voir avec daniel
	// pour l'instant c'est un fake qui retourne 1 !
	
	int getTotalFilesToGet();
	int getCurrentFilesToGet();

	// ----------------------------------------------
	// ScanData Part : optional task wich verify all data
	// ----------------------------------------------
	
	// start the full check of BNP
	void startScanDataThread();

	// if the scan data thread has ended
	bool isScanDataThreadEnded(bool &ok);

	// ask to stop the Scan Data thread. NB: for security, the thread will continue to the current scanned file, 
	// then stop. Hence, you must still wait isScanDataThreadEnded() return true
	void askForStopScanDataThread();

	// get the current info Log for data Scan (true if some change from last get, else text is not filled)
	bool getDataScanLog(ucstring &text);

private:

	// Methods used by patch & check threads
	// -------------------------------------

	friend class CScanDataThread;

	// Set the thread state (called by threads to let us know what they are doing)
	void setState (bool bOutputToLog, const ucstring &ucsState);
	
	/// Read the description file (throw exception if it doesn't exists)
	void				readDescFile(sint32 nVersion);

	/// Read the description of the highest client version file found
	void				readClientVersionAndDescFile();

	/// get all the patchs toapply from the desc and current files present
	void getPatchFromDesc(SFileToPatch &ftpOut, const CBNPFile &fIn, bool forceCheckSumTest);

	// stop the threads (called when knowing the thread ended)
	void stopScanDataThread();

	// add a file to the scan data log
	void addDataScanLogCorruptedFile(const SFileToPatch &ftp);
	void clearDataScanLog();
	static void getCorruptedFileInfo(const SFileToPatch &ftp, ucstring &sTranslate);
	
private:

	/// Constructor
	CPatchManager();

	/// The singleton's instance
	static CPatchManager* _Instance;

	/// Description file
	std::string						DescFilename;
	bool							ValidDescFile;
	CProductDescriptionForClient	DescFile;
	std::vector<SFileToPatch>		FilesToPatch;

	// Threads
	CScanDataThread	*ScanDataThread;
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

	// Where the client get all delta and desc file
	std::string ClientPatchPath;
	std::string ClientDataPath;

	/// Output usefull information for debugging in the log file
	bool VerboseLog;

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
	// The file size of a an empty BNP
	enum	{EmptyBnpFileSize= 8};
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
