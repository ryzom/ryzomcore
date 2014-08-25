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

#include "nel/misc/types_nl.h"
#include "nel/misc/co_task.h"
#include "nel/misc/tds.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/debug.h"

// Flag to use thread instead of coroutine primitives (i.e windows fibers or gcc context)
#ifndef NL_OS_WINDOWS
#define NL_USE_THREAD_COTASK
#endif
// flag to activate debug message
//#define NL_GEN_DEBUG_MSG

#ifdef NL_GEN_DEBUG_MSG
#define NL_CT_DEBUG nldebug
#else
#define NL_CT_DEBUG while(0)nldebug
#endif

#if defined(NL_USE_THREAD_COTASK)
	#ifndef __GNUC__
	#pragma message(NL_LOC_MSG "Using threaded coroutine")
	#endif
	# include "nel/misc/thread.h"
#else //NL_USE_THREAD_COTASK
// some platform specifics
#if defined (NL_OS_WINDOWS)
//# define _WIN32_WINNT 0x0500
# define NL_WIN_CALLBACK CALLBACK
// Visual .NET won't allow Fibers for a Windows version older than 2000. However the basic features are sufficient for us, we want to compile them for all Windows >= 95
# if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0400)
#  ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
#  endif
#  define _WIN32_WINNT 0x0400
# endif

#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#elif defined (NL_OS_UNIX)
#	define NL_WIN_CALLBACK
#	include <ucontext.h>
#else
#	error "Coroutine task are not supported yet by your platform, do it ?"
#endif
#endif //NL_USE_THREAD_COTASK

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

	// platform specific data
#if  defined(NL_USE_THREAD_COTASK)
	struct TCoTaskData : public IRunnable
#else //NL_USE_THREAD_COTASK
	struct TCoTaskData
#endif //NL_USE_THREAD_COTASK
	{
#if  defined(NL_USE_THREAD_COTASK)
		/// The thread id for the co task
//		TThreadId	*_TaskThreadId;
		/// The parent thread id
//		TThreadId	*_ParentThreadId;

		// the thread of the task
		IThread				*_TaskThread;
		/// The mutex of the task task
		CFastMutex			_TaskMutex;

		CCoTask				*_CoTask;

		// set by master, cleared by task
		volatile bool		_ResumeTask;
		// set by task, cleared by master
		volatile bool		_TaskHasYield;


		TCoTaskData(CCoTask *task)
			:	_TaskThread(NULL),
				_CoTask(task),
				_ResumeTask(false),
				_TaskHasYield(false)
		{
		}

		virtual ~TCoTaskData()
		{
			NL_CT_DEBUG("CoTaskData : ~TCoTaskData %p : deleting cotask data", this);
			if (_TaskThread != NULL)
			{
				NL_CT_DEBUG("CoTask : ~TCoTaskData (%p) waiting for thread termination", this);

				// waiting for thread to terminate
				_TaskThread->wait();

				delete _TaskThread;
				_TaskThread = NULL;
			}
		}

		void run();

#else //NL_USE_THREAD_COTASK
#if defined (NL_OS_WINDOWS)
		/// The fiber pointer for the task fiber
		LPVOID	_Fiber;
		/// The fiber pointer of the main (or master, or parent, as you want)
		LPVOID	_ParentFiber;
#elif defined (NL_OS_UNIX)
		/// The coroutine stack pointer (allocated memory)
		uint8		*_Stack;
		/// The task context
		ucontext_t	_Ctx;
		/// The main (or master or parent, as you want) task context
		ucontext_t	_ParentCtx;
#endif
#endif //NL_USE_THREAD_COTASK

#if !defined(NL_USE_THREAD_COTASK)
		/** task bootstrap function
		 *	NB : this function is in this structure because of the
		 *	NL_WIN_CALLBACK symbol that need <windows.h> to be defined, so
		 *	to remove it from the header, I moved the function here
		 *	(otherwise, it should be declared in the CCoTask class as
		 *	a private member)
		 */
		static void NL_WIN_CALLBACK startFunc(void* param)
		{
			CCoTask *task = reinterpret_cast<CCoTask*>(param);

			NL_CT_DEBUG("CoTask : task %p start func called", task);

			try
			{
				// run the task
				task->run();
			}
			catch(...)
			{
				nlwarning("CCoTask::startFunc : the task has generated an unhandled exeption and will terminate");
			}

			task->_Finished = true;

			NL_CT_DEBUG("CoTask : task %p finished, entering infinite yield loop (waiting destruction)", task);

			// nothing more to do
			for (;;)
				// return to parent task
				task->yield();
		}
#endif //NL_USE_THREAD_COTASK
	};

	/** Management of current task in a thread.
	 *	This class is used to store and retrieve the current
	 *	CCoTask pointer in the current thread.
	 *	It is build upon the SAFE_SINGLETON paradigm, making it
	 *	safe to use with NeL DLL.
	 *	For windows platform, this singleton also hold the
	 *	fiber pointer of the current thread. This is needed because
	 *	of the bad design of the fiber API before Windows XP.
	 */
	class CCurrentCoTask
	{
		NLMISC_SAFE_SINGLETON_DECL(CCurrentCoTask);

		/// A thread dependent storage to hold by thread coroutine info
		CTDS	_CurrentTaskTDS;

#if defined (NL_OS_WINDOWS)
		/// A Thread dependent storage to hold fiber pointer.
		CTDS	_ThreadMainFiber;
#endif

		CCurrentCoTask()
		{}

	public:
		/// Set the current task for the calling thread
		void setCurrentTask(CCoTask *task)
		{
			NL_CT_DEBUG("CoTask : setting current co task to %p", task);
			_CurrentTaskTDS.setPointer(task);
		}

		/// retrieve the current task for the calling thread
		CCoTask *getCurrentTask()
		{
			return reinterpret_cast<CCoTask*>(_CurrentTaskTDS.getPointer());
		}
#if defined (NL_OS_WINDOWS) && !defined(NL_USE_THREAD_COTASK)
		void setMainFiber(LPVOID fiber)
		{
			_ThreadMainFiber.setPointer(fiber);
		}

		/** Return the main fiber for the calling thread. Return NULL if
		 *	the thread has not been converted to fiber.
		 */
		LPVOID getMainFiber()
		{
			return _ThreadMainFiber.getPointer();
		}
#endif
	};

	NLMISC_SAFE_SINGLETON_IMPL(CCurrentCoTask);

	CCoTask *CCoTask::getCurrentTask()
	{
		return CCurrentCoTask::getInstance().getCurrentTask();
	}


	CCoTask::CCoTask(uint stackSize)
		: _Started(false),
		_TerminationRequested(false),
		_Finished(false)
	{
		NL_CT_DEBUG("CoTask : creating task %p", this);
#if defined(NL_USE_THREAD_COTASK)
		// allocate platform specific data storage
		_PImpl = new TCoTaskData(this);
//		_PImpl->_TaskThreadId = 0;
//		_PImpl->_ParentThreadId = 0;
		nlunreferenced(stackSize);
#else //NL_USE_THREAD_COTASK
		// allocate platform specific data storage
		_PImpl = new TCoTaskData;
		nlunreferenced(stackSize);
#if defined (NL_OS_WINDOWS)
		_PImpl->_Fiber = NULL;
		_PImpl->_ParentFiber = NULL;
		nlunreferenced(stackSize);
#elif defined(NL_OS_UNIX)
		// allocate the stack
		_PImpl->_Stack = new uint8[stackSize];
#endif
#endif //NL_USE_THREAD_COTASK
	}

	CCoTask::~CCoTask()
	{
		NL_CT_DEBUG("CoTask : deleting task %p", this);
		_TerminationRequested = true;

		if (_Started)
		{
			while (!_Finished)
				resume();
		}

#if defined(NL_USE_THREAD_COTASK)

#else //NL_USE_THREAD_COTASK
#if defined (NL_OS_WINDOWS)
		if (_PImpl->_Fiber)
		{
			DeleteFiber(_PImpl->_Fiber);
		}
#elif defined(NL_OS_UNIX)
		// free the stack
		delete [] _PImpl->_Stack;
#endif
#endif //NL_USE_THREAD_COTASK

		// free platform specific storage
		delete _PImpl;
	}

	void CCoTask::start()
	{
		NL_CT_DEBUG("CoTask : Starting task %p", this);
		nlassert(!_Started);

		_Started = true;

#if defined(NL_USE_THREAD_COTASK)

		// create the thread
		_PImpl->_TaskThread = IThread::create(_PImpl);

		NL_CT_DEBUG("CoTask : start() task %p entering mutex", this);
		// get the mutex
		_PImpl->_TaskMutex.enter();
		NL_CT_DEBUG("CoTask : start() task %p mutex entered", this);

		// set the resume flag to true
		_PImpl->_ResumeTask = true;

		// start the thread
		_PImpl->_TaskThread->start();

		NL_CT_DEBUG("CoTask : start() task %p leaving mutex", this);
		// leave the mutex
		_PImpl->_TaskMutex.leave();

		// wait until the task has yield
		for (;;)
		{
			// give up the time slice to the co task
			nlSleep(0);
			NL_CT_DEBUG("CoTask : start() task %p entering mutex", this);
			// get the mutex
			_PImpl->_TaskMutex.enter();
			NL_CT_DEBUG("CoTask : start() task %p mutex entered", this);

			if (!_PImpl->_TaskHasYield)
			{
				// not finished
				NL_CT_DEBUG("CoTask : start() task %p has not yield, leaving mutex", this);
				// leave the mutex
				_PImpl->_TaskMutex.leave();
			}
			else
			{
				break;
			}
		}

		// clear the yield flag
		_PImpl->_TaskHasYield = false;

		NL_CT_DEBUG("CoTask : start() task %p has yield", this);

		// in the treaded mode, there is no need to call resume() inside start()

#else //NL_USE_THREAD_COTASK
  #if defined (NL_OS_WINDOWS)

		LPVOID mainFiber = CCurrentCoTask::getInstance().getMainFiber();

		if (mainFiber == NULL)
		{
			// we need to convert this thread to a fiber
			mainFiber = ConvertThreadToFiber(NULL);

			if (mainFiber == NULL)
			{
				DWORD dw = GetLastError();
#if defined(ERROR_ALREADY_FIBER)
				if (dw == ERROR_ALREADY_FIBER) nlerror("ConvertThreadToFiber ERROR_ALREADY_FIBER: "
					"If you are using nel in dynamic libraries, you should have a 'pure "
					"nel library' entry point, see definition of NLMISC_DECL_PURE_LIB");
				else
#endif
				nlerror("ConvertThreadToFiber error %u", dw);
			}

			CCurrentCoTask::getInstance().setMainFiber(mainFiber);
		}

		_PImpl->_ParentFiber = mainFiber;
		_PImpl->_Fiber = CreateFiber(NL_TASK_STACK_SIZE, TCoTaskData::startFunc, this);
		nlassert(_PImpl->_Fiber != NULL);
  #elif defined (NL_OS_UNIX)
		// store the parent ctx
		nlverify(getcontext(&_PImpl->_ParentCtx) == 0);
		// build the task context
		nlverify(getcontext(&_PImpl->_Ctx) == 0);

		// change the task context
		_PImpl->_Ctx.uc_stack.ss_sp = _PImpl->_Stack;
		_PImpl->_Ctx.uc_stack.ss_size = NL_TASK_STACK_SIZE;

		_PImpl->_Ctx.uc_link = NULL;
		_PImpl->_Ctx.uc_stack.ss_flags = 0;

		makecontext(&_PImpl->_Ctx, reinterpret_cast<void (*)()>(TCoTaskData::startFunc), 1, this);
  #endif
		resume();
#endif //NL_USE_THREAD_COTASK
	}

	void CCoTask::yield()
	{
		NL_CT_DEBUG("CoTask : task %p yield", this);
		nlassert(_Started);
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() == this);
#if defined(NL_USE_THREAD_COTASK)

		// set the yield flag
		_PImpl->_TaskHasYield = true;

		// release the mutex
		NL_CT_DEBUG("CoTask : yield() task %p leaving mutex", this);
		_PImpl->_TaskMutex.leave();

		// now, wait until the resume flag is set
		for (;;)
		{
			// give up the time slice to the master thread
			nlSleep(0);
			// And get back the mutex for waiting for next resume (this should lock)
			NL_CT_DEBUG("CoTask : yield() task %p entering mutex", this);
			_PImpl->_TaskMutex.enter();
			NL_CT_DEBUG("CoTask : yield() task %p mutex entered", this);

			if (!_PImpl->_ResumeTask)
			{
				// not time to resume, release the mutex and sleep
				NL_CT_DEBUG("CoTask : yield() task %p not time to resume, leaving mutex", this);
				_PImpl->_TaskMutex.leave();
//				nlSleep(0);
			}
			else
				break;
		}

		// clear the resume flag
		_PImpl->_ResumeTask = false;

#else //NL_USE_THREAD_COTASK
		CCurrentCoTask::getInstance().setCurrentTask(NULL);
#if defined (NL_OS_WINDOWS)
		SwitchToFiber(_PImpl->_ParentFiber);
#elif defined (NL_OS_UNIX)
		// swap to the parent context
		nlverify(swapcontext(&_PImpl->_Ctx, &_PImpl->_ParentCtx) == 0);
#endif
#endif //NL_USE_THREAD_COTASK

		NL_CT_DEBUG("CoTask : task %p have been resumed", this);
	}

	void CCoTask::resume()
	{
		NL_CT_DEBUG("CoTask : resuming task %p", this);
		nlassert(CCurrentCoTask::getInstance().getCurrentTask() != this);
		if (!_Started)
			start();
		else if (!_Finished)
		{
			nlassert(_Started);

#if defined(NL_USE_THREAD_COTASK)

			// set the resume flag to true
			_PImpl->_ResumeTask = true;
			_PImpl->_TaskHasYield = false;
			// Release the mutex
			NL_CT_DEBUG("CoTask : resume() task %p leaving mutex", this);
			_PImpl->_TaskMutex.leave();
			// wait that the task has started
			while (_PImpl->_ResumeTask)
				nlSleep(0);

			NL_CT_DEBUG("CoTask : resume() task %p is started, waiting yield", this);
			// ok the task has started
			// now wait for task to yield
			for (;;)
			{
				// give up the time slice to the co task
				nlSleep(0);

				// acquire the mutex
				NL_CT_DEBUG("CoTask : resume() task %p entering mutex", this);
				_PImpl->_TaskMutex.enter();
				NL_CT_DEBUG("CoTask : resume() task %p mutex entered", this);

				if (!_PImpl->_TaskHasYield)
				{
					NL_CT_DEBUG("CoTask : resume() task %p still not yielding, leaving mutex", this);
					_PImpl->_TaskMutex.leave();
					// give the focus to another thread before acquiring the mutex
//					nlSleep(0);
				}
				else
				{
					// the task has yield
					break;
				}
			}

			// clear the yield flag
			_PImpl->_TaskHasYield = false;

#else // NL_USE_THREAD_COTASK
			CCurrentCoTask::getInstance().setCurrentTask(this);
#if defined (NL_OS_WINDOWS)
			SwitchToFiber(_PImpl->_Fiber);
#elif defined (NL_OS_UNIX)
			// swap to the parent context
			nlverify(swapcontext(&_PImpl->_ParentCtx, &_PImpl->_Ctx) == 0);
#endif
#endif //NL_USE_THREAD_COTASK
		}

		NL_CT_DEBUG("CoTask : task %p has yield", this);
	}

	/// wait until the task terminate
	void CCoTask::wait()
	{
		NL_CT_DEBUG("CoTask : waiting for task %p to terminate", this);
		// resume the task until termination
		while (!_Finished)
			resume();
	}

#if  defined(NL_USE_THREAD_COTASK)
	void TCoTaskData::run()
	{
		NL_CT_DEBUG("CoTask : entering TCoTaskData::run for task %p", _CoTask);
		// set the current task
		CCurrentCoTask::getInstance().setCurrentTask(_CoTask);
		// Set the task as running
//		_Running = true;
		NL_CT_DEBUG("CoTask : TCoTaskData::run() task %p entering mutex", this);
		// Acquire the task mutex
		_TaskMutex.enter();
		NL_CT_DEBUG("CoTask : TCoTaskData::run mutex aquired, calling '_CoTask->run()' for task %p", _CoTask);

		// clear the resume flag
		_CoTask->_PImpl->_ResumeTask = false;

		// run the task
		_CoTask->run();

		// mark the task has yielding
		_CoTask->_PImpl->_TaskHasYield = true;
		// mark the task has finished
		_CoTask->_Finished = true;

		// nothing more to do, just return to terminate the thread
		NL_CT_DEBUG("CoTask : leaving TCoTaskData::run for task %p", _CoTask);

		NL_CT_DEBUG("CoTask : TCoTaskData::run() task %p leaving mutex", this);
		// Release the parent mutex
		_TaskMutex.leave();

	}
#endif //NL_USE_THREAD_COTASK

	void CCoTask::requestTerminate()
	{
		_TerminationRequested = true;
	}

	void CCoTask::sleep(uint milliseconds)
	{
		nlassert(getCurrentTask() == this); // called outside run() !
		TTime startTime = CTime::getLocalTime();
		while(!isTerminationRequested())
		{
			TTime currTime = CTime::getLocalTime();
			if (currTime - startTime >= milliseconds) break;
			yield();
		}
	}

} // namespace NLMISC

