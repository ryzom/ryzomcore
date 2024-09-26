// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// georges_exe.cpp : Defines the entry point for the application.
//
#include <windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "nel/misc/debug.h"
#include "../georges_dll/georges_interface.h"

using namespace std;

int APIENTRY nltWinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPTSTR lpCmdLine, int nCmdShow)
{
	// Init the NeL application context
	NLMISC::CApplicationContext context;

	// Create a logic editor
	IGeorges* pGeorges = IGeorges::getInterface();

	if (pGeorges)
	{
		// Init ui
		pGeorges->initUI(nCmdShow, true);

 		string cmd = NLMISC::tStrToUtf8(lpCmdLine);
 		if(!cmd.empty())
 		{
 			nlinfo("Using command line '%s'", cmd.c_str());
 			pGeorges->LoadDocument(cmd.c_str());
 		}

		// Go
		pGeorges->go();

		// Release ui
		// pGeorges->releaseUI ();

		// Delete the pointer
		IGeorges::releaseInterface (pGeorges);
	}

	return 0;
}
