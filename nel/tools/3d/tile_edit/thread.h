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

//! component="System"

#ifndef header_thread
#define header_thread

class CL_Runnable
//: Thread callback interface.
// When a thread is created, it will call run() in its attached CL_Runnable interface.
{
public:
	virtual void run()=0;
	// Called when a thread is run.
};

class CL_Thread
{
public:
	static CL_Thread *create(CL_Runnable *runnable);
	// Create a thread that uses the CL_Runnable callback interface.
	//!param: runnable - Class to call when thread is started.

	static CL_Thread *create(int (*func)(void*), void* value);
	// Create a thread that calls the function specified, with the value specified.
	//!param: func - Function that gets called at thread start.
	//!param: value - Value passed to the function at thread start.
	
	virtual ~CL_Thread () {;}
	
	virtual void start()=0;
	// Starts the thread.

	virtual void terminate()=0;
	// Terminate the thread. (use with caution under win98)

	virtual void wait()=0;
	// Wait until the thread finishes its execution.
};

#endif
