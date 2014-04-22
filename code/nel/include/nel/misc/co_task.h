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

#ifndef NL_CO_TASK_H
#define NL_CO_TASK_H

#include "types_nl.h"
//#include <vector>

namespace NLMISC
{
#if defined (NL_OS_WINDOWS)
# define NL_WIN_CALLBACK CALLBACK
#elif defined (NL_OS_UNIX)
# define NL_WIN_CALLBACK
#endif

	// Default to 8KB stack for tasks
	const unsigned int	NL_TASK_STACK_SIZE = 8*1024;

	// forward def for platform specific data
	struct TCoTaskData;

	/** The coroutine task class encapsulate the coroutines detail and provide
	 *	an easy to use simple master/slave coroutine model.
	 *	The concept is that the main thread is the 'master' (or parent) coroutine
	 *	and that the task is run by a slave coroutine.
	 *	Therefore, you can 'start' the task, 'yield' the focus from the task to it's
	 *	parent, 'resume' the task from the parent, check for termination of the task,
	 *	and/or wait for it.
	 *
	 *	Note that for safety reasons, the CCoTask do not provide mean to for
	 *	the termination of the task. Like for threads, 'killing' a task while
	 *	destroy the task stack without calling destructor of any object create
	 *	on the stack. This can lead to memory leaks or atomic destruction of
	 *	earth (if used inside an ICBM control program).
	 *	So, as a rule of thumb, (valid for thread as well), you should always
	 *	design your code so your coroutines are cleanly terminated before
	 *	your program end.
	 *
	 *	If you don't know about coroutines, a short description follow :
	 *		* Coroutines are some sort of multi-threading
	 *		* Coroutines are not preemptive, it's the application code that choose
	 *			task swapping point
	 *		* thus, coroutines don't need heavy synchronization (like mutex)
	 *		* coroutines are said to be lighter than thread during context switch
	 *		* coroutines don't replace preemptives threads, they have their own application domain
	 *
	 *
	 *	Please note that this class is really simple (compared to what can be done with coroutines)
	 *	but match the need for a very simple mean to have two task running side by side with
	 *	predefined sync point.
	 *	You can build the same think using thread and mutex, but it will be a lot more complex
	 *	to build and debug.
	 *
	 *  A simple sample :
	 *	CMyTask : public CCoTask
	 *	{
	 *	public:
	 *		void run()
	 *		{
	 *			for (uint i=0; i<7; ++i)
	 *			{
	 *				printf("CoTask : %i\n", i)
	 *				// leave control to parent task
	 *				yield();
	 *			}
	 *		}
	 *	};
	 *
	 *	uint main()
	 *	{
	 *		CMyTask task;
	 *		// start the task, block until task terminate or call 'yield'
	 *		task.resume();
	 *
	 *		for (uint i=0; i<5; ++i)
	 *		{
	 *			printf("Main : %i\n", i);
	 *			// let the task run a bit
	 *			task.resume();
	 *		}
	 *
	 *		// wait for task completion
	 *		task.wait();
	 *	}
	 *
	 *	This little proggy will output the following :
	 *	*********** Output *************
	 *	CoTask : 1
	 *	Main : 1
	 *	CoTask : 2
	 *	Main : 2
	 *	CoTask : 3
	 *	Main : 3
	 *	CoTask : 4
	 *	Main : 4
	 *	CoTask : 5
	 *	CoTask : 6
	 *	*********** End of output ********
	 *
	 *
	 */
	class CCoTask
	{
		/// Flag stating if the task is started or not
		bool	_Started;
		/// Flag statig if the task should terminate as soon as possible
		bool	_TerminationRequested;
		/// Flag stating if the task is finished (run() have returned)
		bool	_Finished;

		/// Pointer on internal platform specific data
		TCoTaskData	*_PImpl;

		friend struct TCoTaskData;

		/// Coroutine bootstrap function
		void start();
	public:

		/** Get the current task object.
		 *	Return NULL if the current thread context is not in a CCoTask coroutine
		 */
		static CCoTask *getCurrentTask();

		/** Constructor with stack size for the task.
		 *	The default stack size is 8 KB because it sound cool and because
		 *	I found many coroutine code that use a 8 KB stack.
		 *	If you need to start many (more than some 10th) tasks with very
		 *	little stack usage, you could reduce you coroutine memory overhead
		 *	by lowering the stack size.
		 */
		CCoTask(uint stackSize = NL_TASK_STACK_SIZE);
		/** Destructor. If the task is running, set the termination requested flag
		 *	and resume the task until it terminate.
		 *	If you task is badly designed, your destructor will never return, waiting
		 *	indefinitely for the task to terminate.
		 */
		virtual ~CCoTask();

		/* Start or resume task execution.
		 * If called from the current task context, do nothing (execution continue in the
		 * current task)
		 */
		void resume();

		/// to call from the task, yield execution focus to parent task
		void yield();

		/** Check if task is started.
		 *	A task is not started until you call resume().
		 */
		bool isStarted()
		{
			return _Started;
		}
		/// check for task completion
		bool isFinished()
		{
			return _Finished;
		}

		/// parent task ask for task ending (run function should check this and terminate asap)
		void requestTerminate();

		/** check if termination request have been called (mainly used by task user code
		 *	to check for terminating the task on request).
		 */
		bool isTerminationRequested()
		{
			return _TerminationRequested;
		}

		/** Called by parent task, wait until the task terminate. Note obviously that this call can lead to an
		 *	infinite wait if the task function is not willing to terminate itself.
		 */
		void wait();

		/** the run method to implement by the derived class. This is where
		 *	you put the co routine code.
		 *	Coroutine terminate when this method return.
		 */
		virtual void run() =0;

		/** Wait (using 'yield') until some amount of time (in milliseconds) has ellapsed, or until termination is requested.
		 *  This should be called inside this task 'run()', else an assertion is raised
		 */
		void sleep(uint milliseconds);


	};


} // namespace NLMISC

#endif // NL_CO_TASK_H
