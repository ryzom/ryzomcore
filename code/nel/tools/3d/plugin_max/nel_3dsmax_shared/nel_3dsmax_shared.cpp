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

#include "stdafx.h"
#include "../nel_patch_lib/rpo.h"
#include "nel_3dsmax_shared.h"
#include "nel/misc/app_context.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	// initialize nel context
	if (!NLMISC::INelContext::isContextInitialised())
	{
		GetSharedNelContext();
		nldebug("NeL 3ds Max Shared: DllMain");
	}

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void init ()
{
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// The static allocator
static CPatchAllocator Allocator;

NEL_3DSMAX_SHARED_API CPatchAllocator &GetAllocator ()
{
	// Init fonction
	init ();

	return Allocator;
}

NEL_3DSMAX_SHARED_API NLMISC::INelContext &GetSharedNelContext()
{
	if (!NLMISC::INelContext::isContextInitialised())
	{
		new NLMISC::CApplicationContext();
		NLMISC::createDebug();
	}
	return NLMISC::INelContext::getInstance();
}
