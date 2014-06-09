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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include "nel/misc/mutex.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/debug.h"

#include <SDL.h>

namespace NLMISC {

// ***************************************************************************

CMutex::CMutex()
{
	m_SDLMutex = SDL_CreateMutex();
}

CMutex::CMutex(const std::string &name)
{
	m_SDLMutex = SDL_CreateMutex();
}

CMutex::~CMutex()
{
	SDL_DestroyMutex(m_SDLMutex);
}

void CMutex::enter()
{
	SDL_LockMutex(m_SDLMutex);
}

void CMutex::leave()
{
	SDL_UnlockMutex(m_SDLMutex);
}

// ***************************************************************************

CAtomicLock::CAtomicLock() : m_SDLSpinLock(0)
{
	
}

CAtomicLock::~CAtomicLock()
{
	
}

void CAtomicLock::init()
{
	m_SDLSpinLock = 0;
}

void CAtomicLock::enter()
{
	SDL_AtomicLock(&m_SDLSpinLock);
}

void CAtomicLock::leave()
{
	SDL_AtomicUnlock(&m_SDLSpinLock);
}

// ***************************************************************************

} // NLMISC
