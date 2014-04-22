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

// georges_plugin_test.cpp : Defines the initialization routines for the DLL.
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include "sound_plugin.h"


using namespace NLGEORGES;
using namespace std;


// ***************************************************************************

NLMISC::CLibraryContext *LibContext = NULL;


__declspec( dllexport ) IEditPlugin *IGeorgesEditGetInterface (int version, NLGEORGES::IEdit *globalInterface, NLMISC::INelContext &nelContext)
{
	// init the library nel context
	if (LibContext == NULL)
	{
		LibContext = new NLMISC::CLibraryContext(nelContext);
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// Same version ?
	if (version == NLGEORGES_PLUGIN_INTERFACE_VERSION)
	{
		try 
		{
			return new CSoundPlugin(globalInterface);
		}
		catch (std::exception &e)
		{
			string reason = e.what();
			MessageBox (NULL, reason.c_str(), "Sound plugin", MB_OK);
			return NULL;
		}
	}
	else
	{
		MessageBox (NULL, "Plugin version invalid.", "Sound plugin for georges editor", MB_OK|MB_ICONEXCLAMATION);
		return NULL;
	}

}
