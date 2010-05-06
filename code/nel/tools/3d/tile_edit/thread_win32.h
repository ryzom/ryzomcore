/*
	$Id: thread_win32.h,v 1.1 2000/10/19 07:53:33 corvazier Exp $

	------------------------------------------------------------------------
	ClanLib, the platform independent game SDK.

	This library is distributed under the GNU LIBRARY GENERAL PUBLIC LICENSE
	version 2. See COPYING for details.

	For a total list of contributers see CREDITS.

	------------------------------------------------------------------------
*/

#ifndef header_thread_win32
#define header_thread_win32

#include "thread.h"
#include <windows.h>

class CL_Thread_Win32 : public CL_Thread
{
public:
	CL_Thread_Win32(CL_Runnable *runnable);
	virtual ~CL_Thread_Win32();
	
	virtual void start();
	virtual void terminate();
	virtual void wait();

private:
	static unsigned long __stdcall func_proxy(void *arg);

	HANDLE thread_handle;
	DWORD thread_id;
	CL_Runnable *runnable;
};

#endif

