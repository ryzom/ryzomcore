
//#include "Core/precomp.h"

#include "thread.h"
#include "thread_win32.h"

/*************************************
 Generic static create-thread function
*************************************/

CL_Thread *CL_Thread::create(CL_Runnable *runnable)
{
	return new CL_Thread_Win32(runnable);
}

/*************************************
        class CL_Thread_Win32
*************************************/


CL_Thread_Win32::CL_Thread_Win32(CL_Runnable *runnable)
{
	this->runnable = runnable;
	thread_handle = NULL;
}

CL_Thread_Win32::~CL_Thread_Win32()
{
	if (thread_handle != NULL) terminate();
}

unsigned long CL_Thread_Win32::func_proxy(void *arg)
{
	CL_Thread_Win32 *parent = (CL_Thread_Win32 *) arg;
	parent->runnable->run();

	return 0;
}

void CL_Thread_Win32::start()
{
	thread_handle = CreateThread(NULL, 0, func_proxy, this, 0, &thread_id);
	if (thread_handle == NULL)
	{
		//throw CL_Error("Failed to create thread");
	}
}

void CL_Thread_Win32::terminate()
{
	TerminateThread(thread_handle, 0);
	CloseHandle(thread_handle);
	thread_handle = NULL;
}

void CL_Thread_Win32::wait()
{
	if (thread_handle == NULL) return;

	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);
	thread_handle = NULL;
}
