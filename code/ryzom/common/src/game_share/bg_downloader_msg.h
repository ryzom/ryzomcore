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

#ifndef RY_BG_DOWNLOADER_MSG_H
#define RY_BG_DOWNLOADER_MSG_H


namespace BGDownloader // Ryzom background downloader
{

enum TMsgType
{
	// from background downloader to client
	BGD_UpdateStatusString = 0, // Sent by background downloader to reflect an update of its status string in the client
	BGD_State,					// The downloader gives its state to the client
	BGD_Mode,					// The downloader gives its mode to the client
	BGD_TaskResult,				// Return the result of the last task after a 'GetTaskResult' request
								// will be followed by a uint32 bitfield giving the name of the available downloads
								// (as indexed by TDownloadID)
								// (or 0 else)
	BGD_DescFile,				// The downloader send the desc file to the client
	BGD_Error,					// The downloader signal a download problem to the client
	BGD_Priority,

	// from client to background downloader
	CL_GetMode,				// Client ask its mode to the downloader
	CL_GetState,			// Client ask its state to the downloader
	CL_GetTaskResult,		// Client ask the result of the last task
	CL_Stop,				// Client ask the downloader to stop (interrupt) its current task, then remain in 'idle' state
	CL_StartTask,			// Client ask the downloader to start a new specific task, the downloader should be in 'idle' state
	CL_SetMode,				// Put the downloader in 'slave' mode or autonomous, 1 param is 'TDownloaderMode'
	CL_SetVerbose,			// Followed by 'true' or 'false'
							// In verbose mode, the downloader will send a status string
	CL_Shutdown,			// Client wants the downloader to be shutdown. Happens before a reboot of both client & downloader
	CL_GetDescFile,			// Client ask the description file that is updated when the last "CheckPatch" succeeded
	CL_Show,
	CL_Hide,
	CL_SetPriority,

	// probe / ack for init
	CL_Probe,
	BGD_Ack,
	UnknownMessageType
};


enum TDownloaderState
{
	DLState_Idle = 0,
	DLState_CheckPatch,			// both slave & autonomous modes
	DLState_GetPatch,			// autonomous mode only
	DLState_GetAndApplyPatch,	// slave mode only
	DLState_Count
};


enum TDownloaderMode
{
	DownloaderMode_Autonomous = 0, // for future standalone downloader
	DownloaderMode_Slave,	// In this mode, the downloader won't take the initiative
							// to start a download task. Client has authority on the tasks to start
	DownloaderMode_Unknown
};

enum TDownloadID
{
	DownloadID_RoS		= 0,			 // ruin of silans
	DownloadID_MainLand = 1,			 // mainland
	DownloadID_Count,
	DownloadID_Unknown = DownloadID_Count
};


enum TThreadPriority
{
	ThreadPriority_DownloaderError = -1,
	ThreadPriority_Paused = 0,
	ThreadPriority_Low,
	ThreadPriority_Normal,
	ThreadPriority_Count,
	ThreadPriority_Unknown = ThreadPriority_Count
};


// description of current task of the bg downloader
class CTaskDesc
{
public:
	TDownloaderState   State;
	uint32			   DownloadBitfield;
public:
	CTaskDesc(TDownloaderState state = DLState_Idle, uint32 downloadBitfield = 0) : State(state), DownloadBitfield(downloadBitfield) {}
	bool operator == (const CTaskDesc &rhs) const
	{
		return State == rhs.State && DownloadBitfield == rhs.DownloadBitfield;
	}
	bool operator != (const CTaskDesc &rhs) const { return !(*this == rhs); }
	void serial(NLMISC::IStream &s)
	{
		s.serialVersion(0);
		s.serialEnum(State);
		s.serialEnum(DownloadBitfield);
	}
};


enum TTaskResult
{
	TaskResult_Success = 0,
	TaskResult_Error,
	TaskResult_Interrupted,
	TaskResult_Unknown
};

// windows ids for inter window communication
const int ClientWndID = 0xc6e526b;
const int DownloaderWndID = 0x5ce37c22;

const uint RYZOM_PID_SHM_ID = 0x6b833f31;

// name of the background downloader system-wide mutex

extern const char *DownloaderMutexName;


// get patch written size in megabytes
ucstring getWrittenSize(uint32 nSize);

std::string toString(TMsgType msgType);

}

#endif
