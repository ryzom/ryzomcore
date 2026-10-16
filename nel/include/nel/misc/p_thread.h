// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_P_THREAD_H
#define NL_P_THREAD_H

#include "types_nl.h"

#ifdef NL_OS_UNIX

#include "thread.h"
#include <pthread.h>


namespace NLMISC {


/**
 * Posix Thread
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CPThread : public IThread
{
public:
	enum TThreadState
	{
		ThreadStateNone,
		ThreadStateRunning,
		ThreadStateFinished,
	};

	/// Constructor
	CPThread( IRunnable *runnable, uint32 stackSize);

	virtual ~CPThread() NL_OVERRIDE;

	virtual void start() NL_OVERRIDE;
	virtual bool isRunning() NL_OVERRIDE;
	virtual void terminate() NL_OVERRIDE;
	virtual void wait() NL_OVERRIDE;
	virtual bool setCPUMask(uint64 cpuMask) NL_OVERRIDE;
	virtual uint64 getCPUMask() NL_OVERRIDE;
	virtual void setPriority(TThreadPriority priority) NL_OVERRIDE;
	virtual std::string getUserName() NL_OVERRIDE;

	virtual IRunnable *getRunnable() NL_OVERRIDE
	{
		return Runnable;
	}

	/// Internal use
	IRunnable	*Runnable;

	TThreadState	_State;
	pthread_t	_ThreadHandle;

private:
	uint32		_StackSize;
};

/**
 * Posix Process
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CPProcess : public IProcess
{
public:
	virtual ~CPProcess() NL_OVERRIDE {}
	virtual uint64 getCPUMask() NL_OVERRIDE;
	virtual bool setCPUMask(uint64 mask) NL_OVERRIDE;
};

} // NLMISC


#endif // NL_OS_UNIX

#endif // NL_P_THREAD_H

/* End of p_thread.h */
