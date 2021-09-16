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
