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




//////////////
// INCLUDES //
//////////////
#include "stdpch.h"
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#if defined(NL_OS_WINDOWS)
#include <windows.h>
#include <shellapi.h>
#endif

// game share
#include "game_share/ryzom_version.h"

// Client
#include "sheets_packer_init.h"


///////////
// USING //
///////////
using namespace NLMISC;


/////////////
// GLOBALS //
/////////////
static uint32 Version = 1;	// Client Version.


///////////////
// FUNCTIONS //
///////////////


//---------------------------------------------------
// MAIN :
// Entry for the Apllication.
//---------------------------------------------------
int main(int argc, char **argv)
{
	/////////////////////////////////
	// Initialize the application. //
	try
	{
		// If the init fail -> return Failure.
		if(!init())
			return EXIT_FAILURE;
	}
	catch(const EFatalError &) { return EXIT_FAILURE; /* nothing to do */ }
	catch(const Exception &e)
	{
		try
		{
			nlerror ("Initialization of the application failed : %s", e.what());
		}
		catch(const EFatalError &)
		{
			// nothing to do
		}

		// Failure -> Exit.
		return EXIT_FAILURE;
	}

	/////////////////////////////
	// Release all the memory. //
	try
	{
		release();
	}
	catch(const EFatalError &) { return EXIT_FAILURE; /* nothing to do */ }
	catch(const Exception &e)
	{
		try
		{
			nlerror ("Release of the application failed: %s", e.what());
		}
		catch(const EFatalError &)
		{
			// nothing to do
		}

		// Failure -> Exit.
		return EXIT_FAILURE;
	}


	// EXIT of the Application.
	return EXIT_SUCCESS;
}// main //
