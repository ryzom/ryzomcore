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



#ifndef NL_MODULE_MANAGER_H
#define NL_MODULE_MANAGER_H

#include <vector>
#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/mutex.h"
#include "nel/misc/thread.h"
#include "nel/misc/debug.h"


typedef void	(*TModuleExecCallback)();

/**
 * A simple function call stacker
 * \warning When there are several module, the Windows Task Manager shows the service taking
 * 100% of CPU but it remains nice to other programs. See waitAllReady() for more information.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CModuleManager : public NLMISC::IRunnable
{
private:
	/// The maximum number of modules
	static uint									_MaxModules;

	/// The mutexes associated to each module
	static NLMISC::CMutex						*_ModMutexes;

	/// The managers currently used
	static std::vector<CModuleManager*>			_RegisteredManagers;


	/// The name (for display) of this manager
	std::string									_StackName;

	/// The Id of the manager
	uint										_Id;

	/// Run on an independent thread
	bool										_Independent;

	/// The current cycle (same for all managers, used for synchro.)
	volatile uint64								_Cycle;

	/// Complete coutner (used for synchro, as cycle restart.)
	volatile uint64								_CompleteCycle;

	/// The type of an item (module or wait)
	enum TExecutionType
	{
		Module = 0,
		Wait
	};

	/// An execution item
	struct CExecutionItem
	{
		TExecutionType			Type;
		uint					Id;
		TModuleExecCallback		Cb;
	};

	/// The stack of execution items
	std::vector<CExecutionItem>					_ExecutionStack;

	/// The list of modules per manager
	std::vector<uint>							_ExecutedModules;


	/// The thread associated to this manager
	NLMISC::IThread								*_Thread;

	/// @name The stop flags
	//@{
	volatile bool								_StopThread;
	volatile bool								_ThreadStopped;
	//@}

private:
	/// Constructor. WARNING, never create module manager from another manager !!
	CModuleManager(const CModuleManager &mod) { nlerror("FEMMAN: forbidden constructor used!"); }

	/// WARNING, never initialize module manager from another manager !!
	CModuleManager	& operator = (const CModuleManager &mod) { nlerror("FEMMAN: forbidden operator = used!");}

public:

	/// Inits the whole manager structure, and setup the maximum number of usable modules
	static void		init(uint maxModules);

	/// Releases the whole manager structure.
	static void		release();


	///

	/// Starts all managers at the same time
	static void		startAll();

	/// Stop all managers before timeout (if can't, the threads are hard terminated.)
	static void		stopAll(NLMISC::TTime timeout = 2000);

	/// Resets the whole managers list
	static void		resetManagers();




	/// Constructor
	CModuleManager(const char *name = NULL, bool independent = false);

	/// Destructor
	~CModuleManager();


	/// Adds a new module to this manager (id must be unique.)
	void			addModule(uint id, TModuleExecCallback cb);

	/// Adds a wait for a module (id doesn't have to be unique.)
	void			addWait(uint id);


	/// Starts the manager loop, independantly from the other managers
	void			start();

	/// Run the execution stack only once
	void			runOnce();

	/** Stops the manager loop
	 * \param blockingMode if set, the stop will wait for timeout before terminating the thread.
	 */
	void			stop(bool blockingMode=true, NLMISC::TTime timeout = 2000);



	/// The run() method from the runnable interface. Not to be called it.
	virtual void	run();

private:

	///
	static void		resetCycle();

	///
	static bool		allReady();

	///
	static bool		allComplete();

	///
	static bool		allStopped();



	///
	void			executeStack();

	///
	void			enterMutexes();


	///
	void			stepCycle()
	{
		++_Cycle;
	}


	///
	void			waitAllReady()
	{
		//nldebug("FEMMAN: [%s] waiting for all modules to increase cycle", _StackName.c_str());
		while (!allReady())
			NLMISC::nlSleep(0);

		/* Warning (Windows platform): this sleep(0) will make the service show 100% of CPU in the
		 * Windows task manager. In fact, if there is idle time, the sleep(0) makes the system give
		 * the CPU back to the same service. This service is still nice to other programs.
		 * See also waitAllComplete().
		 */
	}

	///
	void			completeCycle()
	{
		++_CompleteCycle;
	}

	///
	void			waitAllComplete()
	{
		//nldebug("FEMMAN: [%s] waiting for all modules to restart cycle", _StackName.c_str());
		while (!allComplete())
			NLMISC::nlSleep(0);

		/* Warning (Windows platform) about sleep(0): see waitAllReady().
		 */
	}
};


#endif // NL_MODULE_MANAGER_H

/* End of module_manager.h */
