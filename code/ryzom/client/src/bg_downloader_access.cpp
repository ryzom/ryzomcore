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

#include "stdpch.h"
#include "bg_downloader_access.h"
#include "global.h"
#include "login_patch.h"
//
#include "nel/misc/shared_memory.h"
#include "nel/misc/i18n.h"
#include "nel/misc/win_thread.h"
#include "nel/misc/big_file.h"
//
#include "game_share/bg_downloader_msg.h"


#ifdef NL_OS_WINDOWS

#include "nel/misc/win_thread.h"

using namespace BGDownloader;
using namespace NLMISC;

extern HINSTANCE HInstance;

// exception used by the download coroutine to signal an error
class EDownloadException : public NLMISC::Exception
{
public:
	EDownloadException(const std::string &reason) : Exception(reason) {}
};

class EDownloadTerminationRequested : public NLMISC::Exception
{
public:
	EDownloadTerminationRequested() : Exception("Download termination requested") {}
};

class EWaitMessageTimeoutException : public NLMISC::Exception
{
public:
	EWaitMessageTimeoutException() : Exception("Download timeout message") {}
};

//=====================================================
CBGDownloaderAccess::CBGDownloaderAccess()
{
	_DownloadCoTask = new CDownloadCoTask;
	_DownloadCoTask->Parent = this;
	_State = State_Idle;
	_TaskResult = BGDownloader::TaskResult_Unknown;
	_AvailablePatchs = 0;
	clearCurrentMessage();
	_MustLaunchBatFile = false;
	_ShowDownloader = false;
	_DownloadThreadPriority = ThreadPriority_Low;
	_FrozenUI = false;
	_PatchCompletionFlag = false;
	_RyzomInstPIDPtr = NULL;
	_WaitBalloonDisplay = false;
	_CurrentFileProgress = 0.f;
}

//=====================================================
void CBGDownloaderAccess::clearCurrentMessage()
{
	_CurrentMessage = ucstring();
	_CurrentFilesToGet = 0;
	_TotalFilesToGet = 0;
	_PatchingSize = 0;
	_TotalSize = 0;
	_CurrentFileProgress = 0.f;
}

//=====================================================
CBGDownloaderAccess::~CBGDownloaderAccess()
{
	delete _DownloadCoTask;
}

//=====================================================
void CBGDownloaderAccess::init()
{
	release();
}

//=====================================================
void CBGDownloaderAccess::release()
{
	_DownloaderMsgQueue.release();
	if (_DownloadCoTask->isStarted())
	{
		_DownloadCoTask->requestTerminate();
		while (_DownloadCoTask->isFinished())
		{
		}
	}
	_State = State_Idle;
	if (_RyzomInstPIDPtr)
	{
		NLMISC::CSharedMemory::closeSharedMemory(_RyzomInstPIDPtr);
		_RyzomInstPIDPtr = NULL;
	}
}


//=====================================================
void CBGDownloaderAccess::update()
{
	/*nlwarning("In msg queue = %d, out msg queue = %d",
			  (int) _DownloaderMsgQueue.getReceiveQueueSize(),
		      (int) _DownloaderMsgQueue.getSendQueueSize());*/
	switch (_State)
	{
		case State_Idle:
			nlassert(!_DownloadCoTask->isStarted());
		break;
		case State_Finished:
			if (_WaitBalloonDisplay)
			{
				nlassert(_DownloadCoTask->isStarted());
				_DownloadCoTask->resume();
			}
			else
			{
				if (_DownloadCoTask->isStarted())
				{
					resetDownloadTask();
				}
			}
		break;
		case State_Patching:
			// Avoid task collision here : this may happen during the tp main loop, and
			// would lead to a freeze
			if (NLMISC::CCoTask::getCurrentTask() == NULL)
			{
				_DownloadCoTask->resume();
			}
			if (_State == State_Finished && !_WaitBalloonDisplay)
			{
				resetDownloadTask();
			}
		break;
		default:
			nlassert(0);
		break;
	}
}

//=====================================================
void CBGDownloaderAccess::resetDownloadTask()
{
	delete _DownloadCoTask;
	_DownloadCoTask = new CDownloadCoTask;
	_DownloadCoTask->Parent = this;
	nlassert(!_DownloadCoTask->isStarted());
}

//=====================================================
void CBGDownloaderAccess::startTask(const BGDownloader::CTaskDesc &taskDesc, const std::string &commandLine, bool showDownloader)
{
	nlassert(_State != State_Patching); // patching already started
	nlassert(!_DownloadCoTask->isStarted());
	_CurrentFilesToGet = 0;
	_TotalFilesToGet = 0;
	_PatchingSize = 0;
	_TotalSize = 0;
	_CommandLine = commandLine;
	_State = State_Patching;
	_TaskResult = BGDownloader::TaskResult_Unknown;
	_WantedTask = taskDesc;
	_ErrorMsg.clear();
	_ShowDownloader = showDownloader;
}

//=====================================================
bool CBGDownloaderAccess::isTaskEnded(BGDownloader::TTaskResult &result, ucstring &errorMsg) const
{
	if (_State == State_Finished)
	{
		result = _TaskResult;
		errorMsg = _ErrorMsg;
		return true;
	}
	return false;
}

//=====================================================
void CBGDownloaderAccess::resumeBackgroundDownload()
{
	nlassert(0); // not handled yet
	//_DownloadCoTask->setDownloaderMode(DownloaderMode_Autonomous);
}

////////////////////////
// DOWNLOAD COROUTINE //
////////////////////////


//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::run()
{
	Parent->_ErrorMsg = "";
	try
	{
		// don't restart downloader before
		if (Parent->_WantedTask.State == DLState_GetAndApplyPatch)
		{
			// this is a continuation of the check task, so ensure
			// that the downloader is still running and in slave mode
			if (!isDownloaderProcessRunning() && getDownloaderMode() != DownloaderMode_Slave)
			{
				throw EDownloadException(CI18N::get("uiBGD_DownloaderStopped").toUtf8());
			}
		}
		else
		{
			// should be executed only once for the check
			restartDownloader();
		}
		setDownloaderVerbosity(true);
		setDownloaderMode(DownloaderMode_Slave);
		doTask(Parent->_WantedTask);
		setDownloaderVerbosity(false);
		// clear display info, else they would remain for a brief time when arriving on patch screen
		Parent->clearCurrentMessage();
		//
		getTaskResult(Parent->_TaskResult, Parent->_AvailablePatchs, Parent->_MustLaunchBatFile, Parent->_ErrorMsg);

		Parent->_PatchCompletionFlag = false;
		if (Parent->_TaskResult == BGDownloader::TaskResult_Success)
		{
			if (Parent->_WantedTask == DLState_CheckPatch)
			{
				getDescFile();
				//
				if (!Parent->_AvailablePatchs)
				{
					Parent->_WaitBalloonDisplay = true;
					Parent->_State = State_Finished;
					// because the downloader may pop a notification balloon here, wait some more before shutting it down

					sleep(2500); // at this time, may last longer because the client starts a loading phase
					Parent->_WaitBalloonDisplay = false;
					// no patch, just stop the downloader
					shutdownDownloader();
				}

				bool rosPatch = (Parent->_AvailablePatchs & (1 << BGDownloader::DownloadID_RoS)) != 0;
				bool mainlandPatch = (Parent->_AvailablePatchs & (1 << BGDownloader::DownloadID_MainLand)) != 0;

				if (Parent->_MustLaunchBatFile && !rosPatch) // let be superstitious, should always be true ...
				{
					// no patch but must rebbot ? may happen only if some 'unpacked' file needed by RoS have been deleted
					shutdownDownloader();
				}
				else
				if (mainlandPatch)
				{
					// if mainland patch isn't completed yet, remove the searchpaths
					// to the .bnp that are going to be patched

					//sint64 startTime = NLMISC::CTime::getLocalTime();
					const CBNPCategorySet &bnpCatSet = Parent->_DescFile.getCategories();
					std::vector<std::string> bigFiles;
					for (uint catIndex = 0; catIndex < bnpCatSet.categoryCount(); ++catIndex)
					{
						const CBNPCategory	&cat = bnpCatSet.getCategory(catIndex);
						if (cat.isOptional()) // NB : 'optional' flag meaning has changed : it now means 'Mainland Patch'
											  // until an enum is added
						{
							for (uint f = 0; f < cat.fileCount(); ++f)
							{
								bigFiles.push_back(cat.getFile(f));
							}
						}
					}
					NLMISC::CPath::removeBigFiles(bigFiles);
					//sint64 endTime = NLMISC::CTime::getLocalTime();
					//nlinfo("%.2f s to remove paths", (endTime - startTime) / 1000.f);
				}
			}

			if (Parent->_WantedTask.State == DLState_GetAndApplyPatch)
			{
				Parent->_PatchCompletionFlag = true;
				if (Parent->_WantedTask.DownloadBitfield & (1 << BGDownloader::DownloadID_RoS))
				{
					if (Parent->_MustLaunchBatFile) // let be superstitious, should always be true ...
					{
						Parent->_WaitBalloonDisplay = true;
						Parent->_State = State_Finished;
						// because the downloader may pop a notification balloon here, wait some more before shutting it down

						sleep(5000);
						Parent->_WaitBalloonDisplay = false;
						// because a reboot is required, just stop now ...
						shutdownDownloader();
					}
				}
				else
				if (Parent->_WantedTask.DownloadBitfield & (1 << BGDownloader::DownloadID_MainLand))
				{
					bool memoryCompressed = CPath::isMemoryCompressed();
					if (memoryCompressed)
					{
						CPath::memoryUncompress();
					}
					// and redo 'addSearchPath' on data, because of the new bnp files that have just been added
					NLMISC::CPath::addSearchPath("data/", true, false, NULL);
					if (memoryCompressed)
					{
						CPath::memoryCompress();
					}
					Parent->_WaitBalloonDisplay = true;
					Parent->_State = State_Finished;
					// because the downloader may pop a notification balloon here, wait some more before shutting it down
					sleep(5000);
					Parent->_WaitBalloonDisplay = false;
					// stop after the mainland download (hardcoded, but should suffice for now)
					shutdownDownloader();
				}
			}
		}
	}
	catch(const EDownloadException &e)
	{
		//shutdownDownloader();
		Parent->_TaskResult = TaskResult_Error;
		Parent->_ErrorMsg.fromUtf8(e.what());
		Parent->_DownloadThreadPriority = ThreadPriority_DownloaderError;
	}
	catch(const EDownloadTerminationRequested &e)
	{
		shutdownDownloader();
		Parent->_TaskResult = TaskResult_Error;
		Parent->_ErrorMsg = ucstring(e.what());
		Parent->_DownloadThreadPriority = ThreadPriority_DownloaderError;
	}
	catch(const NLMISC::EStream &e)
	{
		shutdownDownloader();
		Parent->_TaskResult = TaskResult_Error;
		nlwarning("BG DOWNLOADER PROTOCOL ERROR ! Stream error");
		Parent->_ErrorMsg = CI18N::get("uiBGD_ProtocolError") + ucstring(" : ") + ucstring(e.what());
		Parent->_DownloadThreadPriority = ThreadPriority_DownloaderError;
	}
	catch (const EWaitMessageTimeoutException &e)
	{
		shutdownDownloader();
		Parent->_TaskResult = TaskResult_Error;
		nlwarning("BG DOWNLOADER PROTOCOL ERROR ! Message timeout");
		Parent->_ErrorMsg = CI18N::get("uiBGD_ProtocolError") + ucstring(" : ") + ucstring(e.what());
		Parent->_DownloadThreadPriority = ThreadPriority_DownloaderError;
	}
	Parent->_State = State_Finished;
}

//=====================================================
bool CBGDownloaderAccess::getPatchCompletionFlag(bool clearFlag)
{
	bool flag = _PatchCompletionFlag;
	if (clearFlag)
	{
		_PatchCompletionFlag = false;
	}
	return flag;
}

//=====================================================
bool CBGDownloaderAccess::CDownloadCoTask::isDownloaderProcessRunning()
{
	// the downloader creates a system-wide mutex, so if present, assume that the downloader is running
	//
	HANDLE mutex = CreateMutex (NULL, FALSE, BGDownloader::DownloaderMutexName);
	if (mutex)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(mutex);
			return true;
		}
		CloseHandle(mutex);
	}
	return false;
}

// launch the process and wait until completion
#if defined (NL_DEBUG)
	static const char *BGDownloaderName = "client_background_downloader_d.exe";
#elif defined (NL_RELEASE)
	static const char *BGDownloaderName = "client_background_downloader_r.exe";
#else
#	error "Unknown bg downloader exe name";
#endif


//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::createDownloaderProcess()
{
	bool manualLaunch = false;
	CConfigFile::CVar *manualLaunchVar = ClientCfg.ConfigFile.getVarPtr("BackgroundDownloaderManualLaunch");
	if (manualLaunchVar)
	{
		manualLaunch = (manualLaunchVar->asInt() != 0);
	}
	if (!manualLaunch)
	{
		BOOL ok = NLMISC::launchProgram(BGDownloaderName, Parent->_CommandLine);
		if (!ok)
		{
			throw EDownloadException(CI18N::get("uiBGD_LaunchError").toUtf8());
		}
	}
	else
	{
		nlwarning(Parent->_CommandLine.c_str());
	}
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::restartDownloader()
{
	// if another ryzom client is running, try stopping it first
	for (;;)
	{
		void *ryzomInstPIDPtr = NLMISC::CSharedMemory::accessSharedMemory(NLMISC::toSharedMemId(RYZOM_PID_SHM_ID));
		if (!ryzomInstPIDPtr) break; // only instance running is us
		// try to terminate the other instance
		uint count = 0;
		while (*(DWORD *) ryzomInstPIDPtr == 0)
		{
			// the other process didn't write its pid into shared mem yet (shared memory is initially filled with zero's)
			// so wait a bit (very unlikely case !!)
			sleep(100);
			++ count;
			if (count == 50)
			{
				nlwarning("CBGDownloaderAccess::CDownloadCoTask : detected shared memory segment, with NULL pid");
				// some problem here ...
				throw EDownloadException(CI18N::get("uiBGD_MultipleRyzomInstance").toUtf8());
			}
		}
		bool ok = NLMISC::CWinProcess::terminateProcess(*(DWORD *) ryzomInstPIDPtr);
		CSharedMemory::closeSharedMemory(ryzomInstPIDPtr);
		if (!ok)
		{
			nlwarning("CBGDownloaderAccess::CDownloadCoTask : detected shared memory segment, with good pid, but couldn't stop the process");
			// couldn't stop the other client ...
			throw EDownloadException(CI18N::get("uiBGD_MultipleRyzomInstance").toUtf8());
		}
	}
	// write our pid into shared mem
	Parent->_RyzomInstPIDPtr = CSharedMemory::createSharedMemory(NLMISC::toSharedMemId(RYZOM_PID_SHM_ID), sizeof(uint32));
	if (!Parent->_RyzomInstPIDPtr)
	{
		// really, really bad luck ...
		throw EDownloadException(CI18N::get("uiBGD_MultipleRyzomInstance").toUtf8());
	}
	*(uint32 *) Parent->_RyzomInstPIDPtr = (uint32) GetCurrentProcessId();

	HWND hWnd = Driver->getDisplay();

	// for safety, stop any running downloader
	if (isDownloaderProcessRunning())
	{
		Parent->_DownloaderMsgQueue.init(hWnd, BGDownloader::ClientWndID, BGDownloader::DownloaderWndID);
		sleep(200);
		shutdownDownloader();
		Parent->_DownloaderMsgQueue.release();
		sleep(200);
	}
	uint tryCounter = 1;
	for (;;)
	{
		nlwarning("Launching downloader: try number %u", tryCounter++);
		// now we can create the message queue because we are sure that it will reach the good app
		Parent->_DownloaderMsgQueue.init(HInstance, BGDownloader::ClientWndID, BGDownloader::DownloaderWndID);
		sleep(200);

		Parent->_CurrentMessage = CI18N::get("uiBGD_Launching");

		#ifndef BG_DOWNLOADER_MANUAL_LAUNCH
			createDownloaderProcess();
		#endif
		while (!Parent->_DownloaderMsgQueue.connected())
		{
			yieldDownload();
		}
		// try probe/ack test for safety
		bool ok = false;
		int tryIndex = 1;
		uint32 waitTime = 500;
		const uint32 totalTries = 7;
		while (waitTime <= 32000)
		{
			Parent->_CurrentMessage.fromUtf8(toString(CI18N::get("uiBGD_HandShaking").toUtf8().c_str(), tryIndex, totalTries));

			sendSimpleMsg(CL_Probe);
			NLMISC::CMemStream dummyMsg;
			try
			{
				waitMsg(BGD_Ack, dummyMsg, waitTime /* max milliseconds */);
				ok = true;
				break;
			}
			catch (const EWaitMessageTimeoutException &)
			{
				// no-op, just continue the loop for another try
			}
			waitTime *= 2;
			++ tryIndex;
		}
		if (ok) break;
		if (isDownloaderProcessRunning())
		{
			shutdownDownloader();
			sleep(200);
		}
		Parent->_DownloaderMsgQueue.release();
		sleep(200);
	}
	nlwarning("Downloader launched successfully");
	if (Parent->_ShowDownloader)
	{
		Parent->showDownloader();
	}
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::yieldDownload()
{
	yield();
	if (isTerminationRequested())
	{
		throw EDownloadTerminationRequested();
	}
}

//=====================================================
void CBGDownloaderAccess::sendSimpleMsg(BGDownloader::TMsgType msgType)
{
	CMemStream outMsg(false /* output stream */);
	outMsg.serialEnum(msgType);
	_DownloaderMsgQueue.sendMessage(outMsg);
}

//=====================================================
void CBGDownloaderAccess::showDownloader()
{
	sendSimpleMsg(CL_Show);
}

//=====================================================
void CBGDownloaderAccess::hideDownloader()
{
	sendSimpleMsg(CL_Hide);
}


//=====================================================
CTaskDesc CBGDownloaderAccess::CDownloadCoTask::getDownloaderState()
{
	sendSimpleMsg(CL_GetState);
	CMemStream inMsg(true /* input stream */);
	waitMsg(BGD_State, inMsg);
	CTaskDesc result;
	inMsg.serial(result);
	return result;
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::getDescFile()
{
	sendSimpleMsg(CL_GetDescFile);
	CMemStream inMsg(true /* input stream */);
	waitMsg(BGD_DescFile, inMsg);
	inMsg.serial(Parent->_DescFile);
}


//=====================================================
TDownloaderMode CBGDownloaderAccess::CDownloadCoTask::getDownloaderMode()
{
	sendSimpleMsg(CL_GetMode);
	CMemStream inMsg(true /* input stream */);
	waitMsg(BGD_Mode, inMsg);
	TDownloaderMode mode = DownloaderMode_Unknown;
	inMsg.serialEnum(mode);
	return mode;
}

//=====================================================
void  CBGDownloaderAccess::CDownloadCoTask::getTaskResult(TTaskResult &result,
														  uint32 &availablePatchs,
														  bool &mustLaunchBatFile,
														  ucstring &errorMsg
														 )
{
	sendSimpleMsg(CL_GetTaskResult);
	CMemStream inMsg(true /* input stream */);
	waitMsg(BGD_TaskResult, inMsg);
	inMsg.serialEnum(result);
	inMsg.serial(availablePatchs);
	inMsg.serial(mustLaunchBatFile);
	inMsg.serial(errorMsg);
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::doTask(const CTaskDesc &taskDesc)
{
	CTaskDesc downloaderTask = getDownloaderState();
	if (downloaderTask.State != DLState_Idle && downloaderTask != taskDesc)
	{
		// if not the wanted task, ask to stop current task, and wait until completion
		stopTask();
		startTask(taskDesc);
	}
	else
	if (downloaderTask.State == DLState_Idle)
	{
		startTask(taskDesc);
	}
	// else, good task already started, just wait
	waitIdle();
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::startTask(const CTaskDesc &taskDesc)
{
	CMemStream outMsg(false /* output stream */);
	TMsgType msgType = CL_StartTask;
	outMsg.serialEnum(msgType);
	CTaskDesc mutableTaskDesc = taskDesc;
	outMsg.serial(mutableTaskDesc);
	sendMsg(outMsg);
	waitIdle();
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::stopTask()
{
	sendSimpleMsg(CL_Stop);
	waitIdle();
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::shutdownDownloader()
{
	const uint32 SHUTDOWN_TIMEOUT = 6; // seconds until we consider the downloader is hung, and try to shut it down
	sendSimpleMsg(CL_Shutdown);
	uint32 startTime = NLMISC::CTime::getSecondsSince1970();
	setDownloaderVerbosity(false);
	while (isDownloaderProcessRunning())
	{
		Parent->_CurrentMessage = CI18N::get("uiBGD_ShuttingDown");
		yield();
		if ((NLMISC::CTime::getSecondsSince1970() - startTime) >= SHUTDOWN_TIMEOUT)
		{
			Parent->_CurrentMessage = CI18N::get("uiBGD_ForciblyShutdown");
			yield();
			if (!CWinProcess::terminateProcessFromModuleName(BGDownloaderName))
			{
				Parent->_CurrentMessage = CI18N::get("uiBGD_ShutdownFailed");
			}
			return;
		}
	}
	CWinProcess::terminateProcessFromModuleName(BGDownloaderName); // for safety
	Parent->_CurrentMessage = ucstring();
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::setDownloaderMode(TDownloaderMode mode)
{
	CMemStream outMsg(false /* output stream */);
	TMsgType msgType = CL_SetMode;
	outMsg.serialEnum(msgType);
	outMsg.serialEnum(mode);
	sendMsg(outMsg);
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::setDownloaderVerbosity(bool verbose)
{
	CMemStream outMsg(false /* output stream */);
	TMsgType msgType = CL_SetVerbose;
	outMsg.serialEnum(msgType);
	outMsg.serial(verbose);
	sendMsg(outMsg);
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::waitIdle()
{
	for (;;)
	{
		CTaskDesc downloaderTask = getDownloaderState();
		if (downloaderTask.State == DLState_Idle) break;
	}
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::waitMsg(BGDownloader::TMsgType wantedMsgType, NLMISC::CMemStream &msg, NLMISC::TTime timeOutInMs)
{
	NLMISC::TTime startTime = NLMISC::CTime::getLocalTime();
	BGDownloader::TMsgType msgType = (BGDownloader::TMsgType) ~0;
	for (;;)
	{
		while (!Parent->_DownloaderMsgQueue.pumpMessage(msg))
		{
			#ifdef NL_DEBUG
			CConfigFile::CVar *manualLaunchVar = ClientCfg.ConfigFile.getVarPtr("BackgroundDownloaderManualLaunch");
			if (!manualLaunchVar || manualLaunchVar->asInt() == 0)
			#endif
			{
				if (NLMISC::CTime::getLocalTime() - startTime > timeOutInMs)
				{
					nlwarning("Time out exceeded while waiting for message of type %d", (int) wantedMsgType);
					#ifdef NL_DEBUG
						//nlassert(0);
					#endif
					throw EWaitMessageTimeoutException();
				}
			}
			checkDownloaderAlive();
			yieldDownload();
		}
		nlassert (msg.isReading());
		msg.serialEnum(msgType);
		if (msgType == wantedMsgType) return;
		if (!defaultMessageHandling(msgType, msg))
		{
			//nlwarning("##CLIENT RCV message of type : %s", toString(msg).c_str());
			break;
		}
	}
	if (msgType != wantedMsgType)
	{
		nlwarning("BG DOWNLOADER PROTOCOL ERROR ! Bad message type received. Expected type is '%d', received type is '%d'", (int) wantedMsgType, (int) msgType);
		throw EDownloadException(CI18N::get("uiBGD_ProtocolError").toUtf8());
	}
}


//=====================================================
bool CBGDownloaderAccess::CDownloadCoTask::defaultMessageHandling(BGDownloader::TMsgType msgType, NLMISC::CMemStream &msg)
{
	nlassert(msg.isReading());
	switch(msgType)
	{
		case BGD_Priority:
		{
			BGDownloader::TThreadPriority tp;
			msg.serialEnum(tp);
			if (tp != Parent->_DownloadThreadPriority)
			{
				Parent->_DownloadThreadPriority = tp; // TMP, for debug
			}
			return true;
		}
		break;
		case BGD_UpdateStatusString:
		{
			msg.serial(Parent->_CurrentMessage);
			msg.serial(Parent->_CurrentFilesToGet);
			msg.serial(Parent->_TotalFilesToGet);
			msg.serial(Parent->_PatchingSize);
			msg.serial(Parent->_TotalSize);
			msg.serial(Parent->_CurrentFileProgress);
			//nlinfo(Parent->_CurrentMessage.toString().c_str());
			return true;
		}
		break;
		case BGD_Error:
		{
			Parent->_TaskResult = TaskResult_Error;
			ucstring errorMsg;
			msg.serial(errorMsg);
			throw EDownloadException(errorMsg.toUtf8());
		}
		break;
		case BGD_Ack:
		{
			// NO-OP
			return true;
		}
		break;
	}
	return false;
}


//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::checkDownloaderAlive()
{
	if (!Parent->_DownloaderMsgQueue.connected() || !isDownloaderProcessRunning())
	{
		throw EDownloadException(CI18N::get("uiBGD_DownloaderDisconnected").toUtf8());
	}
}


//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::sendMsg(NLMISC::CMemStream &msg)
{
	Parent->_DownloaderMsgQueue.sendMessage(msg);
}

//=====================================================
void CBGDownloaderAccess::CDownloadCoTask::sendSimpleMsg(BGDownloader::TMsgType msgType)
{
	//nlwarning("##CLIENT SEND simple message of type : %s", toString(msgType).c_str());
	Parent->sendSimpleMsg(msgType);
}

//=====================================================
void CBGDownloaderAccess::reboot()
{
	CPatchManager *pm = CPatchManager::getInstance();
	pm->createBatchFile(_DescFile);
	pm->executeBatchFile();
}

//=====================================================
void CBGDownloaderAccess::requestDownloadThreadPriority(BGDownloader::TThreadPriority newPriority, bool freezeUI)
{
	nlassert((uint) newPriority < BGDownloader::ThreadPriority_Count);
	CMemStream outMsg(false /* output stream */);
	TMsgType msgType = CL_SetPriority;
	outMsg.serialEnum(msgType);
	outMsg.serialEnum(newPriority);
	outMsg.serial(freezeUI);
	_DownloaderMsgQueue.sendMessage(outMsg);
	_FrozenUI = freezeUI;
}



#else

//=====================================================
CBGDownloaderAccess::CBGDownloaderAccess()
{
	// TODO for Linux
}

//=====================================================
CBGDownloaderAccess::~CBGDownloaderAccess()
{
	// TODO for Linux
}

//=====================================================
bool CBGDownloaderAccess::getPatchCompletionFlag(bool clearFlag)
{
	// TODO for Linux
	return false;
}

//=====================================================
void CBGDownloaderAccess::startTask(const BGDownloader::CTaskDesc &taskDesc, const std::string &commandLine, bool showDownloader)
{
	// TODO for Linux
}

//=====================================================
bool CBGDownloaderAccess::isTaskEnded(BGDownloader::TTaskResult &result, ucstring &errorMsg) const
{
	// TODO for Linux
	return false;
}

//=====================================================
void CBGDownloaderAccess::update()
{
	// TODO for Linux
}

//=====================================================
void CBGDownloaderAccess::release()
{
	// TODO for Linux
}

//=====================================================
void CBGDownloaderAccess::requestDownloadThreadPriority(BGDownloader::TThreadPriority newPriority, bool freezeUI)
{
	// TODO for Linux
}

//=====================================================
void CBGDownloaderAccess::reboot()
{
}

// not supported on other os
//#error IMPLEMENT ME PLEASE !!

#endif


bool isBGDownloadEnabled()
{
	return ClientCfg.BackgroundDownloader && !ClientCfg.Local && ClientCfg.PatchWanted;
}


static BGDownloader::TThreadPriority DownloaderPriorityBackup = BGDownloader::ThreadPriority_Low;
static bool DownloaderUIFrozen = false;
static bool DownloaderPaused = false;


// ------------------------------------------------------------------------------------------------
void pauseBGDownloader()
{
	if (isBGDownloadEnabled() && !DownloaderPaused)
	{
		CBGDownloaderAccess &downloader = CBGDownloaderAccess::getInstance();
		DownloaderPriorityBackup = downloader.getDownloadThreadPriority();
		DownloaderUIFrozen       = downloader.isDownloaderUIFrozen();
		downloader.requestDownloadThreadPriority(BGDownloader::ThreadPriority_Paused, true);
		DownloaderPaused = true;
	}
}

// ------------------------------------------------------------------------------------------------
void unpauseBGDownloader()
{
	if (isBGDownloadEnabled() && DownloaderPaused)
	{
		if (DownloaderPriorityBackup != BGDownloader::ThreadPriority_DownloaderError)
		{
			CBGDownloaderAccess::getInstance().requestDownloadThreadPriority(DownloaderPriorityBackup, DownloaderUIFrozen);
		}
		DownloaderPaused = false;
	}
}
