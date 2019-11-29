// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2011  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#define EXPORT_GET_ALLOCATOR

#include "PO2RPO.h"
#include "nel/misc/debug.h"
#include "nel/misc/app_context.h"
#include "../nel_3dsmax_shared/nel_3dsmax_shared.h"
#include <maxversion.h>

extern ClassDesc2* GetPO2RPODesc();
extern ClassDesc* GetRPODesc();

using namespace NLMISC;

HINSTANCE hInstance = NULL;
int controlsInit = FALSE;

// ------------------------------------------------------------------------------------------------------------------------------------------------

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if (!NLMISC::INelContext::isContextInitialised())
	{
		new NLMISC::CLibraryContext(GetSharedNelContext());
		nldebug("NeL Export: DllMain");
	}

	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		// Hang on to this DLL's instance handle.
		hInstance = hinstDLL;
		DisableThreadLibraryCalls(hInstance);
	}
	if (!NLMISC::INelContext::isContextInitialised())
		new NLMISC::CApplicationContext();
	nlassert(hInstance);
	if (!controlsInit) 
	{
		// This method has been deprecated.
		controlsInit = TRUE;
#if MAX_VERSION_MAJOR < 14
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
#endif
		InitCommonControls();			// Initialize Win95 controls
	}
	return (TRUE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

/// This function returns a pointer to an object (Class ClassDesc2) called a 
/// class Descriptor for each plugin class in the DLL. This class descriptor 
/// object describes the properties of each plugin class and a way to 
/// allocate an instance of the class in memory. 
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetPO2RPODesc();
		case 1: return GetRPODesc();
		//case 1: return GetRykolPatchMeshDesc();
		default: return 0;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------


