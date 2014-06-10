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
#include "nel/misc/thread.h"
#include "nel/misc/debug.h"

#include <SDL_thread.h>

namespace NLMISC {

CThread::CThread(IRunnable *runnable) : m_IsRunning(false), m_SDLThread(NULL), m_Runnable(runnable)
{

}

CThread::~CThread ()
{
	/*
	NOTE: Detach not supported, due to m_IsRunning which is referenced by the running thread
	FIX: Use a thread-safe reference counted object for the m_IsRunning flag
	if (m_SDLThread)
	{
		SDL_DetachThread(m_SDLThread);
	}
	NOTE: Currently use wait, which is much saner behaviour.
	*/

	wait();
}

void CThread::start()
{
	nlassert(!m_SDLThread);
	nlassert(!m_IsRunning);
	std::string name;
	m_Runnable->getName(name);
	m_IsRunning = true;
	m_SDLThread = SDL_CreateThread(run, name.c_str(), (void *)this);
}

int CThread::run(void *ptr)
{
	NLMISC::CThread *thread = static_cast<NLMISC::CThread *>(ptr);
	thread->m_Runnable->run();
	thread->m_IsRunning = false;
	return 0;
}

void CThread::setPriority(TThreadPriority priority)
{
	switch (priority)
	{
	case ThreadPriorityLow:
		SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
		break;
	case ThreadPriorityNormal:
		SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
		break;
	case ThreadPriorityHigh:
		SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
		break;
	default:
		nlerror("Invalid thread priority");
		break;
	}
}

bool CThread::isRunning()
{
	return m_IsRunning;
}

int CThread::wait()
{
	m_WaitMutex.enter();
	if (m_SDLThread)
	{
		SDL_WaitThread(m_SDLThread, &m_ThreadResult);
		m_SDLThread = NULL;
		nlassert(!m_IsRunning);
	}
	m_WaitMutex.leave();
	return m_ThreadResult;
}

IRunnable *CThread::getRunnable()
{
	return m_Runnable;
}

} // NLMISC
