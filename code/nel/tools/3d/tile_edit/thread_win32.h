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

/*
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

