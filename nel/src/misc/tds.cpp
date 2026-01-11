// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/tds.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// *********************************************************

CTDS::CTDS ()
{
	/* Please no assert in the constructor because it is called by the NeL memory allocator constructor */
#ifdef NL_OS_WINDOWS
	_Handle = TlsAlloc ();
	TlsSetValue (_Handle, NULL);
#else // NL_OS_WINDOWS
//	nldebug("CTDS::CTDS...");
	nlverify(pthread_key_create (&_Key, NULL) == 0);
//	nldebug("CTDS::CTDS : create a new key %u", _Key);
	pthread_setspecific(_Key, NULL);
#endif // NL_OS_WINDOWS
}

// *********************************************************

CTDS::~CTDS ()
{
#ifdef NL_OS_WINDOWS
	nlverify (TlsFree (_Handle) != 0);
#else // NL_OS_WINDOWS
//	nldebug("CTDS::~CTDS : deleting key %u", _Key);
	nlverify (pthread_key_delete (_Key) == 0);
#endif // NL_OS_WINDOWS
}

// *********************************************************

void *CTDS::getPointer () const
{
#ifdef NL_OS_WINDOWS
	return TlsGetValue (_Handle);
#else // NL_OS_WINDOWS
//	nldebug("CTDS::getPointer for key %u...", _Key);
	void *ret = pthread_getspecific (_Key);
//	nldebug("CTDS::getPointer returing value %p", ret);
	return ret;
#endif // NL_OS_WINDOWS
}

// *********************************************************

void CTDS::setPointer (void* pointer)
{
#ifdef NL_OS_WINDOWS
	nlverify (TlsSetValue (_Handle, pointer) != 0);
#else // NL_OS_WINDOWS
//	nldebug("CTDS::setPointer for key %u to value %p", _Key, pointer);
	nlverify (pthread_setspecific (_Key, pointer) == 0);
#endif // NL_OS_WINDOWS
}

// *********************************************************

} // NLMISC
