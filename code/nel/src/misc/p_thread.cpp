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

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>

#ifdef NL_OS_UNIX

#include "nel/misc/p_thread.h"

#include <sched.h>
#include <pwd.h>

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

/* Key for thread specific storage holding IThread pointer. */
static pthread_key_t threadSpecificKey;

/* Special thread type representing the main thread. */
struct CPMainThread : public CPThread
{
	CPMainThread() : CPThread(NULL, 0)
	{
		if(pthread_key_create(&threadSpecificKey, NULL) != 0)
			throw EThread("cannot create thread specific storage key.");

		if(pthread_setspecific(threadSpecificKey, this) != 0)
			throw EThread("cannot set main thread ptr in thread specific storage.");
	}

	~CPMainThread()
	{
		if(pthread_key_delete(threadSpecificKey) != 0)
			throw EThread("cannot delete thread specific storage key.");
	}
};

/* Holds the thread instance representing the main thread. */
static CPMainThread mainThread = CPMainThread();

/*
 * The IThread static creator
 */
IThread *IThread::create( IRunnable *runnable, uint32 stackSize)
{
	return new CPThread( runnable, stackSize );
}

/*
 * Get the current thread
 */
IThread *IThread::getCurrentThread ()
{
	return (IThread *)pthread_getspecific(threadSpecificKey);
}

/*
 * Thread beginning
 */
static void *ProxyFunc( void *arg )
{
	CPThread *parent = (CPThread*)arg;

	// Set this thread's thread specific storage to IThread instance pointer
	if(pthread_setspecific(threadSpecificKey, parent) != 0)
		throw EThread("cannot set thread ptr in thread specific storage.");

	// Allow to terminate the thread without cancellation point
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

	// Run the code of the thread
	parent->Runnable->run();

	{
		pthread_t thread_self = pthread_self();
		// Make sure the parent still cares
		// If this thread was replaced with a new thread (which should not happen),
		// and the IThread object has been deleted, this will likely crash.
		if (parent->_ThreadHandle == thread_self)
			parent->_State = CPThread::ThreadStateFinished;
		else
			throw EThread("Thread ended after being detached, this should not happen");
	}

	// Allow some clean
//	pthread_exit(0);
	return NULL;
}



/*
 * Constructor
 */
CPThread::CPThread(IRunnable *runnable, uint32 stackSize)
	:	Runnable(runnable),
		_State(ThreadStateNone),
		_StackSize(stackSize)
{}


/*
 * Destructor
 */
CPThread::~CPThread()
{
	terminate(); // force the end of the thread if not already ended

	if (_State != ThreadStateNone)
		pthread_detach(_ThreadHandle); // free allocated resources only if it was created
}

/*
 * start
 */
void CPThread::start()
{
	pthread_attr_t tattr;
	int ret;

	if (_StackSize != 0)
	{
		/* initialized with default attributes */
		ret = pthread_attr_init(&tattr);

		/* setting the size of the stack also */
		ret = pthread_attr_setstacksize(&tattr, _StackSize);
	}

	bool detach_old_thread = false;
	pthread_t old_thread_handle;
	if (_State != ThreadStateNone)
	{
		if (_State == ThreadStateRunning)
		{
			// I don't know if this behaviour is allowed, but neither thread implementations
			// check the start function, and both simply let the existing running thread for what it is...
			// From now on, this is not allowed.
			throw EThread("Starting a thread that is already started, existing thread will continue running, this should not happen");
		}
		detach_old_thread = true;
		old_thread_handle = _ThreadHandle;
	}

	if (pthread_create(&_ThreadHandle, _StackSize != 0 ? &tattr : NULL, ProxyFunc, this) != 0)
	{
		throw EThread("Cannot start new thread");
	}
	_State = ThreadStateRunning;

	if (detach_old_thread)
	{
		// Docs don't say anything about what happens when pthread_create is called with existing handle referenced.
		if (old_thread_handle == _ThreadHandle)
			throw EThread("Thread handle did not change, this should not happen");
		// Don't care about old thread, free resources when it terminates.
		pthread_detach(old_thread_handle);
	}
}

bool CPThread::isRunning()
{
	return _State == ThreadStateRunning;
}

/*
 * terminate
 */
void CPThread::terminate()
{
	if (_State == ThreadStateRunning)
	{
		// cancel only if started
		pthread_cancel(_ThreadHandle);
		_State = ThreadStateFinished; // set to finished
	}
}

/*
 * wait
 */
void CPThread::wait ()
{
	if (_State == ThreadStateRunning)
	{
		int error = pthread_join(_ThreadHandle, 0);
		switch (error)
		{
		case 0:
			break;
		case EINVAL:
			throw EThread("Thread is not joinable");
		case ESRCH:
			throw EThread("No thread found with this id");
		case EDEADLK:
			throw EThread("Deadlock detected or calling thread waits for itself");
		default:
			throw EThread("Unknown thread join error");
		}
		if(_State != ThreadStateFinished)
			throw EThread("Thread did not finish, this should not happen");
	}
}

/*
 * setCPUMask
 */
bool CPThread::setCPUMask(uint64 cpuMask)
{
#ifdef __USE_GNU

	nlwarning("This code does not work. May cause a segmentation fault...");

	sint res = pthread_setaffinity_np(_ThreadHandle, sizeof(uint64), (const cpu_set_t*)&cpuMask);

	if (res)
	{
		nlwarning("pthread_setaffinity_np() returned %d", res);
		return false;
	}

	return true;

#else // __USE_GNU

	return false;

#endif // __USE_GNU
}

/*
 * getCPUMask
 */
uint64 CPThread::getCPUMask()
{
#ifdef __USE_GNU

	nlwarning("This code does not work. May cause a segmentation fault...");

	uint64 cpuMask = 0;

	sint res = pthread_getaffinity_np(_ThreadHandle, sizeof(uint64), (cpu_set_t*)&cpuMask);

	if (res)
	{
		nlwarning("pthread_getaffinity_np() returned %d", res);
		return 0;
	}

	return cpuMask;

#else // __USE_GNU

	return 0;

#endif // __USE_GNU
}

void CPThread::setPriority(TThreadPriority priority)
{
	// TODO: Test this
	sched_param sp;
	switch (priority)
	{
	case ThreadPriorityHigh:
	{
		int minPrio = sched_get_priority_min(SCHED_FIFO);
		int maxPrio = sched_get_priority_max(SCHED_FIFO);
		sp.sched_priority = ((maxPrio - minPrio) / 4) + minPrio;
		pthread_setschedparam(_ThreadHandle, SCHED_FIFO, &sp);
		break;
	}
	case ThreadPriorityHighest:
	{
		int minPrio = sched_get_priority_min(SCHED_FIFO);
		int maxPrio = sched_get_priority_max(SCHED_FIFO);
		sp.sched_priority = ((maxPrio - minPrio) / 2) + minPrio;
		pthread_setschedparam(_ThreadHandle, SCHED_FIFO, &sp);
		break;
	}
	default:
		sp.sched_priority = 0;
		pthread_setschedparam(_ThreadHandle, SCHED_OTHER, &sp);
	}
}

/*
 * getUserName
 */
std::string CPThread::getUserName()
{
	struct passwd *pw = getpwuid(getuid());

	if (!pw)
		return "";

	return pw->pw_name;
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
#ifdef __USE_GNU

	uint64 cpuMask = 0;
	sint res = sched_getaffinity(getpid(), sizeof(uint64), (cpu_set_t*)&cpuMask);

	if (res)
	{
		nlwarning("sched_getaffinity() returned %d, errno = %d: %s", res, errno, strerror(errno));
		return 0;
	}

	return cpuMask;

#else // __USE_GNU

	return 0;

#endif // __USE_GNU
}

/// set the CPU mask
bool CPProcess::setCPUMask(uint64 cpuMask)
{
#ifdef __USE_GNU

	sint res = sched_setaffinity(getpid(), sizeof(uint64), (const cpu_set_t*)&cpuMask);

	if (res)
	{
		nlwarning("sched_setaffinity() returned %d, errno = %d: %s", res, errno, strerror(errno));
		return false;
	}

	return true;

#else // __USE_GNU

	return false;

#endif // __USE_GNU
}


} // NLMISC

#else // NL_OS_UNIX

// remove stupid VC6 warnings
void foo_p_thread_cpp() {}

#endif // NL_OS_UNIX
