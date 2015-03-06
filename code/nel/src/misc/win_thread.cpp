// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"
#include "nel/misc/win_thread.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/path.h"
#ifndef NL_COMP_MINGW
#define NOMINMAX
#endif
#include <windows.h>

#include <typeinfo>

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CWinThread MainThread ((void*)GetCurrentThread (), GetCurrentThreadId());
DWORD TLSThreadPointer = 0xFFFFFFFF;

// the IThread static creator
IThread *IThread::create (IRunnable *runnable, uint32 stackSize)
{
	return new CWinThread (runnable, stackSize);
}

IThread *IThread::getCurrentThread ()
{
	// TLS alloc must have been done
	nlassert (TLSThreadPointer != 0xffffffff);

	// Get the thread pointer
	IThread *thread = (IThread*)TlsGetValue (TLSThreadPointer);

	// Return current thread
	return thread;
}

static unsigned long __stdcall ProxyFunc (void *arg)
{
	CWinThread *parent = (CWinThread *) arg;

	// TLS alloc must have been done
	nlassert (TLSThreadPointer != 0xffffffff);

	// Set the thread pointer in TLS memory
	nlverify (TlsSetValue (TLSThreadPointer, (void*)parent) != 0);

	// Run the thread
	parent->Runnable->run();

	return 0;
}

CWinThread::CWinThread (IRunnable *runnable, uint32 stackSize)
{
	_StackSize = stackSize;
	this->Runnable = runnable;
	ThreadHandle = NULL;
	_SuspendCount = -1;
	_MainThread = false;
}

namespace {
class CWinCriticalSection
{
private:
	CRITICAL_SECTION cs;
public:
	CWinCriticalSection() { InitializeCriticalSection(&cs); }
	~CWinCriticalSection() { DeleteCriticalSection(&cs); }
	inline void enter() { EnterCriticalSection(&cs); }
	inline void leave() { LeaveCriticalSection(&cs); }
};
CWinCriticalSection s_CS;
}/* anonymous namespace */

CWinThread::CWinThread (void* threadHandle, uint32 threadId)
{
	// Main thread
	_MainThread = true;
	this->Runnable = NULL;
	ThreadHandle = threadHandle;
	ThreadId = threadId;

	// TLS alloc must have been done
	TLSThreadPointer = TlsAlloc ();
	nlassert (TLSThreadPointer!=0xffffffff);

	// Set the thread pointer in TLS memory
	nlverify (TlsSetValue (TLSThreadPointer, (void*)this) != 0);

	if (GetCurrentThreadId() == threadId)
	{
		_SuspendCount = 0; // is calling thread call this itself, well, if we reach this place
						   // there are chances that it is not suspended ...
	}
	else
	{
		// initialized from another thread (very unlikely ...)
		nlassert(0); // WARNING: following code has not tested! don't know if it work fo real ...
					 // This is just a suggestion of a possible solution, should this situation one day occur ...
		// Ensure that this thread don't get deleted, or we could suspend the main thread
		s_CS.enter();
		// the 2 following statement must be executed atomicaly among the threads of the current process !
		SuspendThread(threadHandle);
		_SuspendCount = ResumeThread(threadHandle);
		s_CS.leave();
	}
}


void CWinThread::incSuspendCount()
{
	nlassert(ThreadHandle); // start was not called !!
	int newSuspendCount = ::SuspendThread(ThreadHandle) + 1;
	nlassert(newSuspendCount != 0xffffffff); // more infos with 'GetLastError'
	nlassert(newSuspendCount == _SuspendCount + 1); // is this assert fire , then 'SuspendThread' or 'ResumeThread'
													// have been called outside of this object interface! (on this thread handle ...)
	_SuspendCount = newSuspendCount;
}

void CWinThread::decSuspendCount()
{
	nlassert(ThreadHandle); // 'start' was not called !!
	nlassert(_SuspendCount > 0);
	int newSuspendCount = ::ResumeThread(ThreadHandle) - 1;
	nlassert(newSuspendCount != 0xffffffff); // more infos with 'GetLastError'
	nlassert(newSuspendCount == _SuspendCount - 1); // is this assert fire , then 'SuspendThread' or 'ResumeThread'
													// have been called outside of this object interface! (on this thread handle ...)
	_SuspendCount = newSuspendCount;
}

void CWinThread::suspend()
{
	if (getSuspendCount() == 0)
	{
		incSuspendCount();
	}
}

void CWinThread::resume()
{
	while (getSuspendCount() != 0)
	{
		decSuspendCount();
	}
}

void CWinThread::setPriority(TThreadPriority priority)
{
	nlassert(ThreadHandle); // 'start' was not called !!
	BOOL result = SetThreadPriority(ThreadHandle, (int)priority);
	nlassert(result);
}

void CWinThread::enablePriorityBoost(bool enabled)
{
	nlassert(ThreadHandle); // 'start' was not called !!
	SetThreadPriorityBoost(ThreadHandle, enabled ? TRUE : FALSE);
}


CWinThread::~CWinThread ()
{
	// If not the main thread
	if (_MainThread)
	{
		// Free TLS memory
		nlassert (TLSThreadPointer!=0xffffffff);
		TlsFree (TLSThreadPointer);
	}
	else
	{
		if (ThreadHandle != NULL) terminate();
	}
}

void CWinThread::start ()
{
	if (isRunning())
		throw EThread("Starting a thread that is already started, existing thread will continue running, this should not happen");	

//	ThreadHandle = (void *) ::CreateThread (NULL, _StackSize, ProxyFunc, this, 0, (DWORD *)&ThreadId);
	ThreadHandle = (void *) ::CreateThread (NULL, 0, ProxyFunc, this, 0, (DWORD *)&ThreadId);
//	nldebug("NLMISC: thread %x started for runnable '%x'", typeid( Runnable ).name());
//	OutputDebugString(toString(NL_LOC_MSG " NLMISC: thread %x started for runnable '%s'\n", ThreadId, typeid( *Runnable ).name()).c_str());
	SetThreadPriorityBoost (ThreadHandle, TRUE); // FALSE == Enable Priority Boost
	if (ThreadHandle == NULL)
	{
		throw EThread ( "Cannot create new thread" );
	}

	_SuspendCount = 0;
}

bool CWinThread::isRunning()
{
	if (ThreadHandle == NULL)
		return false;

	DWORD exitCode;
	if (!GetExitCodeThread(ThreadHandle, &exitCode))
		return false;

	return exitCode == STILL_ACTIVE;
}


void CWinThread::terminate ()
{
	TerminateThread((HANDLE)ThreadHandle, 0);
	CloseHandle((HANDLE)ThreadHandle);
	ThreadHandle = NULL;
	_SuspendCount = -1;
}

void CWinThread::wait ()
{
	if (ThreadHandle == NULL) return;

	WaitForSingleObject(ThreadHandle, INFINITE);
	CloseHandle(ThreadHandle);
	ThreadHandle = NULL;
	_SuspendCount = -1;
}

bool CWinThread::setCPUMask(uint64 cpuMask)
{
	// Thread must exist
	if (ThreadHandle == NULL)
		return false;

	// Ask the system for number of processor available for this process
	return SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD_PTR)cpuMask) != 0;
}

uint64 CWinThread::getCPUMask()
{
	// Thread must exist
	if (ThreadHandle == NULL)
		return 1;

	// Get the current process mask
	uint64 mask=IProcess::getCurrentProcess ()->getCPUMask ();

	// Get thread affinity mask
	DWORD_PTR old = SetThreadAffinityMask ((HANDLE)ThreadHandle, (DWORD_PTR)mask);
	nlassert (old != 0);
	if (old == 0)
		return 1;

	// Reset it
	SetThreadAffinityMask ((HANDLE)ThreadHandle, old);

	// Return the mask
	return (uint64)old;
}

std::string CWinThread::getUserName()
{
	char userName[512];
	DWORD size = 512;
	GetUserName (userName, &size);
	return (const char*)userName;
}

// **** Process

// The current process
CWinProcess CurrentProcess ((void*)GetCurrentProcess());

// Get the current process
IProcess *IProcess::getCurrentProcess ()
{
	return &CurrentProcess;
}

CWinProcess::CWinProcess (void *handle)
{
	// Get the current process handle
	_ProcessHandle = handle;
}

uint64 CWinProcess::getCPUMask()
{
	// Ask the system for number of processor available for this process
	DWORD_PTR processAffinityMask;
	DWORD_PTR systemAffinityMask;
	if (GetProcessAffinityMask((HANDLE)_ProcessHandle, &processAffinityMask, &systemAffinityMask))
	{
		// Return the CPU mask
		return (uint64)processAffinityMask;
	}
	else
		return 1;
}

bool CWinProcess::setCPUMask(uint64 mask)
{
	// Ask the system for number of processor available for this process
	DWORD_PTR processAffinityMask= (DWORD_PTR)mask;
	return SetProcessAffinityMask((HANDLE)_ProcessHandle, processAffinityMask)!=0;
}

// ****************************************************************************************************************
/**
 * Simple wrapper around the PSAPI library
 * \author Nicolas Vizerie
 * \author GameForge
 * \date 2007
 */

class CPSAPILib
{
public:
	typedef BOOL  (WINAPI *EnumProcessesFunPtr)(DWORD *lpidProcess, DWORD cb, DWORD *cbNeeded);
	typedef DWORD (WINAPI *GetModuleFileNameExAFunPtr)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
	typedef BOOL  (WINAPI *EnumProcessModulesFunPtr)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
	EnumProcessesFunPtr		  EnumProcesses;
	GetModuleFileNameExAFunPtr GetModuleFileNameExA;
	EnumProcessModulesFunPtr  EnumProcessModules;
public:
	CPSAPILib();
	~CPSAPILib();
	bool init();
private:
	HINSTANCE _PSAPILibHandle;
	bool	  _LoadFailed;
};

// ****************************************************************************************************************
CPSAPILib::CPSAPILib()
{
	_LoadFailed = false;
	_PSAPILibHandle     = NULL;
	EnumProcesses       = NULL;
	GetModuleFileNameExA = NULL;
	EnumProcessModules  = NULL;
}

// ****************************************************************************************************************
CPSAPILib::~CPSAPILib()
{
	if (_PSAPILibHandle)
	{
		FreeLibrary(_PSAPILibHandle);
	}
}

// ****************************************************************************************************************
bool CPSAPILib::init()
{
	//
	if (_LoadFailed) return false;
	if (!_PSAPILibHandle)
	{
		_PSAPILibHandle = LoadLibrary("psapi.dll");
		if (!_PSAPILibHandle)
		{
			nlwarning("couldn't load psapi.dll, possibly not supported by os");
			_LoadFailed = true;
			return false;
		}
		EnumProcesses		= (EnumProcessesFunPtr)		  GetProcAddress(_PSAPILibHandle, "EnumProcesses");
		GetModuleFileNameExA = (GetModuleFileNameExAFunPtr) GetProcAddress(_PSAPILibHandle, "GetModuleFileNameExA");
		EnumProcessModules  = (EnumProcessModulesFunPtr)  GetProcAddress(_PSAPILibHandle, "EnumProcessModules");
		if (!EnumProcesses ||
			!GetModuleFileNameExA ||
			!EnumProcessModules
		   )
		{
			nlwarning("Failed to import functions from psapi.dll!");
			_LoadFailed = true;
			return false;
		}
	}
	return true;
}


static CPSAPILib PSAPILib;



// ****************************************************************************************************************
bool CWinProcess::enumProcessesId(std::vector<uint32> &processesId)
{
	if (!PSAPILib.init()) return false;
	// list of processes
	std::vector<uint32> prcIds(16);
	for (;;)
	{
		DWORD cbNeeded;
		if (!PSAPILib.EnumProcesses((DWORD *) &prcIds[0], (DWORD)(prcIds.size() * sizeof(DWORD)), &cbNeeded))
		{
			nlwarning("Processes enumeration failed!");
			return false;
		}
		if (cbNeeded < prcIds.size() * sizeof(DWORD))
		{
			prcIds.resize(cbNeeded / sizeof(DWORD));
			break;
		}
		// make some more room
		prcIds.resize(prcIds.size() * 2);
	}
	processesId.swap(prcIds);
	return true;
}

// ****************************************************************************************************************
bool CWinProcess::enumProcessModules(uint32 processId, std::vector<std::string> &moduleNames)
{
	if (!PSAPILib.init()) return false;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, (DWORD) processId);
	if (!hProcess) return false;
	// list of modules
	std::vector<HMODULE> prcModules(2);
	for (;;)
	{
		DWORD cbNeeded;
		if (!PSAPILib.EnumProcessModules(hProcess, (HMODULE *) &prcModules[0], (DWORD)(prcModules.size() * sizeof(HMODULE)), &cbNeeded))
		{
			//nlwarning("Processe modules enumeration failed!");
			return false;
		}
		if (cbNeeded < prcModules.size() * sizeof(HMODULE))
		{
			prcModules.resize(cbNeeded / sizeof(HMODULE));
			break;
		}
		// make some more room
		prcModules.resize(prcModules.size() * 2);
	}
	moduleNames.clear();
	std::vector<std::string> resultModuleNames;
	char moduleName[MAX_PATH + 1];
	for (uint m = 0; m < prcModules.size(); ++m)
	{
		if (PSAPILib.GetModuleFileNameExA(hProcess, prcModules[m], moduleName, MAX_PATH))
		{
			moduleNames.push_back(moduleName);
		}
	}
	CloseHandle(hProcess);
	return true;
}

// ****************************************************************************************************************
uint32 CWinProcess::getProcessIdFromModuleFilename(const std::string &moduleFileName)
{
	std::vector<uint32> processesId;
	if (!enumProcessesId(processesId)) return false;
	std::vector<std::string> moduleNames;
	for (uint prc = 0; prc < processesId.size(); ++prc)
	{
		if (enumProcessModules(processesId[prc], moduleNames))
		{
			for (uint m = 0; m < moduleNames.size(); ++m)
			{
				if (nlstricmp(CFile::getFilename(moduleNames[m]), moduleFileName) == 0)
				{
					return processesId[prc];
				}
			}
		}
	}
	return 0;
}

// ****************************************************************************************************************
bool CWinProcess::terminateProcess(uint32 processId, uint exitCode)
{
	if (!processId) return false;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD) processId);
	if (!hProcess) return false;
	BOOL ok = TerminateProcess(hProcess, (UINT) exitCode);
	CloseHandle(hProcess);
	return ok != FALSE;
}

// ****************************************************************************************************************
bool CWinProcess::terminateProcessFromModuleName(const std::string &moduleName, uint exitCode)
{
	return terminateProcess(getProcessIdFromModuleFilename(moduleName), exitCode);
}


///////////////////
// CProcessWatch //
///////////////////


/*

//  I didn't use and test that code, eventually, but maybe useful in the future

class CProcessWatchTask : public IRunnable
{
public:
	HANDLE HProcess;
public:
	CProcessWatchTask(HANDLE hProcess) : HProcess(hProcess)
	{
	}
	virtual void run()
	{
		WaitForSingleObject(HProcess, INFINITE);
	}
};

class CProcessWatchImpl
{
public:
	bool Launched;
	IThread				*WatchThread;
	CProcessWatchTask	*WatchTask;
public:
	CProcessWatchImpl() : Launched(false), WatchThread(NULL), WatchTask(NULL)
	{
	}
	~CProcessWatchImpl()
	{
		reset();
	}
	void reset()
	{
		if (WatchThread)
		{
			if (WatchThread->isRunning())
			{
				WatchThread->terminate();
			}
			delete WatchTask;
			delete WatchThread;
			WatchTask = NULL;
			WatchThread = NULL;
			Launched = false;
		}
	}
	bool launch(const std::string &programName, const std::string &arguments)
	{
		if (isRunning()) return false;
		PROCESS_INFORMATION processInfo;
		STARTUPINFO startupInfo = {0};
		startupInfo.cb = sizeof(STARTUPINFO);
		if (CreateProcess(programName.c_str(), const_cast<LPTSTR>(arguments.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
		{
			WatchTask = new CProcessWatchTask(processInfo.hProcess);
			WatchThread = IThread::create(WatchTask);
			WatchThread->start();
			Launched = true;
			return true;
		}
		return false;
	}
	bool isRunning()
	{
		if (!Launched) return false;
		nlassert(WatchThread);
		nlassert(WatchTask);
		if (WatchThread->isRunning()) return true;
		reset();
		return false;
	}
};


CProcessWatch::CProcessWatch()
{
	_PImpl = new CProcessWatchImpl;
}

CProcessWatch::~CProcessWatch()
{
	delete _PImpl;
}

bool CProcessWatch::launch(const std::string &programName, const std::string &arguments)
{
	return _PImpl->launch(programName, arguments);
}

bool CProcessWatch::isRunning() const
{
	return _PImpl->isRunning();
}

*/
} // NLMISC

#endif // NL_OS_WINDOWS
