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

#ifdef NL_OS_UNIX

#include "nel/misc/p_thread.h"


namespace NLMISC {


/*
 * The IThread static creator
 */
IThread *IThread::create( IRunnable *runnable, uint32 stackSize)
{
	return new CPThread( runnable, stackSize );
}


CPThread CurrentThread(NULL, 0);

/*
 * Get the current thread
 */
IThread *IThread::getCurrentThread ()
{
	return &CurrentThread;
}

/*
 * Thread beginning
 */
static void *ProxyFunc( void *arg )
{
	CPThread *parent = (CPThread*)arg;

	// Allow to terminate the thread without cancellation point
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	// Run the code of the thread
	parent->Runnable->run();

	// Allow some clean
//	pthread_exit(0);
	return NULL;
}



/*
 * Constructor
 */
CPThread::CPThread(IRunnable *runnable, uint32 stackSize)
	:	Runnable(runnable),
		_State(0),
		_StackSize(stackSize)
{}


/*
 * Destructor
 */
CPThread::~CPThread()
{
	if(_State == 1)
		terminate(); // force the end of the thread if not already ended

	if(_State > 0)
		pthread_detach(_ThreadHandle); // free allocated resources only if it was created
}

/*
 * start
 */
void CPThread::start()
{
	pthread_attr_t tattr;
	pthread_t tid;
	int ret;

	/* initialized with default attributes */
	ret = pthread_attr_init(&tattr);

	/* setting the size of the stack also */
	ret = pthread_attr_setstacksize(&tattr, _StackSize);

	if(pthread_create(&_ThreadHandle, _StackSize != 0 ? &tattr : 0, ProxyFunc, this) != 0)
	{
		throw EThread("Cannot start new thread");
	}
	_State = 1;
}

bool CPThread::isRunning()
{
	// TODO : need a real implementation here that check thread status
	return _State == 1;
}

/*
 * terminate
 */
void CPThread::terminate()
{
	if(_State == 1)
	{
		// cancel only if started
		pthread_cancel(_ThreadHandle);
		_State = 2;	// set to finished
	}
}

/*
 * wait
 */
void CPThread::wait ()
{
	if(_State == 1)
	{
		if(pthread_join(_ThreadHandle, 0) != 0)
		{
			throw EThread( "Cannot join with thread" );
		}
		_State = 2;	// set to finished
	}
}

/*
 * setCPUMask
 */
bool CPThread::setCPUMask(uint64 cpuMask)
{
	return true;
}

/*
 * getCPUMask
 */
uint64 CPThread::getCPUMask()
{
	return 1;
}

/*
 * getUserName
 */
std::string CPThread::getUserName()
{
	return "Not implemented";
}


// **** Process

// The current process
CPProcess CurrentProcess;

// Get the current process
IProcess *IProcess::getCurrentProcess ()
{
	return &CurrentProcess;
}

/*
 * getCPUMask
 */
uint64 CPProcess::getCPUMask()
{
	return 1;
}

/// set the CPU mask
bool CPProcess::setCPUMask(uint64 mask)
{
	return 1;
}


} // NLMISC

#else // NL_OS_UNIX

// remove stupid VC6 warnings
void foo_p_thread_cpp() {}

#endif // NL_OS_UNIX
