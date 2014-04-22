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

#ifndef CL_BG_DOWNLOADER_ACCESS
#define CL_BG_DOWNLOADER_ACCESS

#include "nel/misc/singleton.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/inter_window_msg_queue.h"
#include "nel/misc/co_task.h"
//
#include "game_share/bg_downloader_msg.h"
#include "game_share/bnp_patch.h"

// communication with bg downloader
class CBGDownloaderAccess : public NLMISC::CSingleton<CBGDownloaderAccess>
{
public:
	CBGDownloaderAccess();
	~CBGDownloaderAccess();
	// Init the background downloader access.
	void init();
	void release();
	// jobs
	void						startTask(const BGDownloader::CTaskDesc &taskDesc, const std::string &commandLine, bool showDownloader);
	bool						isTaskEnded(BGDownloader::TTaskResult &result, ucstring &errorMsg) const;
	// The following flag will be true after a 'patch' task has been completed successfully
	bool						getPatchCompletionFlag(bool clearFlag);
	//
	BGDownloader::TTaskResult	getLastTaskResult() const { return _TaskResult; }
	bool						mustLaunchBatFile() const { return _MustLaunchBatFile; }
	void						reboot();
	// Valid after a task has ended. If this was a check task, then contain a bitfield indexed by
	// BGDownloader::TDownloadID that indicate the part that need to be patched
	uint32						getAvailablePatchs() const { return _AvailablePatchs; }
	void						resumeBackgroundDownload();
	//
	void update(); // call this at each frame to update the download process
	// Get last displayed message by the background downloader
	const ucstring				&getCurrentMessage() const { return _CurrentMessage; }
	uint32						getCurrentFilesToGet() const { return _CurrentFilesToGet; }
	uint32						getTotalFilesToGet() const { return _TotalFilesToGet; }
	//
	uint32						getPatchingSize() const { return _PatchingSize; }
	uint32						getTotalSize() const { return _TotalSize; }
	float						getCurrentFileProgress() const { return _CurrentFileProgress; }

	void						showDownloader();
	void						hideDownloader();

	// download thread
	BGDownloader::TThreadPriority getDownloadThreadPriority() const { return _DownloadThreadPriority; }
	bool						isDownloaderUIFrozen() const { return _FrozenUI; }
	void						requestDownloadThreadPriority(BGDownloader::TThreadPriority newPriority, bool freezeUI);

	const ucstring			    &getLastErrorMessage() const { return _ErrorMsg; }

private:
	enum TState { State_Idle, State_Patching, State_Finished };
	TState							_State;

	ucstring						_CurrentMessage;
#ifdef NL_OS_WINDOWS
	NLMISC::CInterWindowMsgQueue	_DownloaderMsgQueue;
#endif
	ucstring						_ErrorMsg;
	std::string						_CommandLine;
	BGDownloader::TTaskResult		_TaskResult;
	uint32							_AvailablePatchs;
	BGDownloader::CTaskDesc			_WantedTask;
	uint32							_CurrentFilesToGet;
	uint32							_TotalFilesToGet;
	uint32							_PatchingSize;
	uint32							_TotalSize;
	float							_CurrentFileProgress;
	CProductDescriptionForClient	_DescFile;
	BGDownloader::TThreadPriority	_DownloadThreadPriority;
	void							*_RyzomInstPIDPtr;
	bool							_ShowDownloader;
	bool							_FrozenUI;
	bool							_MustLaunchBatFile;
	bool							_PatchCompletionFlag;
	bool							_WaitBalloonDisplay;
	// The download task, implemented as a coroutine
	class CDownloadCoTask : public NLMISC::CCoTask
	{
	public:
		CBGDownloaderAccess			*Parent;
		BGDownloader::TDownloadID	DownloadID;
	public:
		virtual void run();
		void restartDownloader();
		void createDownloaderProcess();
		void waitMsg(BGDownloader::TMsgType wantedMsgType, NLMISC::CMemStream &msg, NLMISC::TTime timeOutInMs = 6000000);
		void sendMsg(NLMISC::CMemStream &msg);
		void sendSimpleMsg(BGDownloader::TMsgType msgType);
		bool defaultMessageHandling(BGDownloader::TMsgType msgType, NLMISC::CMemStream &msg);
		void setDownloaderMode(BGDownloader::TDownloaderMode mode);
		void setDownloaderVerbosity(bool verbose);
		bool isDownloaderProcessRunning();
		BGDownloader::CTaskDesc getDownloaderState();
		void doTask(const BGDownloader::CTaskDesc &taskDesc); // return value may indicate 'idle' or 'want reboot'
		void stopTask();
		void startTask(const BGDownloader::CTaskDesc &taskDesc);
		void waitIdle();
		void checkDownloaderAlive();
		void yieldDownload(); // same as yield, but also handle the isTerminationRequested() case
		void shutdownDownloader();
		void getTaskResult(BGDownloader::TTaskResult &result,
						   uint32 &availablePatchs,
						   bool &mustLaunchBatFile,
						   ucstring &errorMsg
						  );
		void getDescFile();
		BGDownloader::TDownloaderMode getDownloaderMode();
	};
	CDownloadCoTask *_DownloadCoTask;
	friend class CDownloadCoTask;
private:
	void setDownloaderVerbosity(bool verbose);
	void clearCurrentMessage();
	void sendSimpleMsg(BGDownloader::TMsgType msgType);
	void resetDownloadTask();
};


bool isBGDownloadEnabled();


// helpers to pause / unpause downloader & retore its previous state
void pauseBGDownloader();
void unpauseBGDownloader();


#endif
