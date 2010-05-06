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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc.
#include "nel/misc/debug.h"
// 3D Interface.
//#include "nel/3d/u_driver.h"
// Client
#include "sheets_packer_release.h"


///////////
// USING //
///////////
//using namespace NL3D;
using namespace NLMISC;


////////////
// EXTERN //
////////////
//extern UDriver	*Driver;


///////////////
// FUNCTIONS //
///////////////
//---------------------------------------------------
// release :
// Release all the memory.
//---------------------------------------------------
void release()
{	
	// Delete the driver.
/*	if(Driver)
	{
		// Release Scene, textcontexts, materials, ...
		Driver->release();

		// Delete the driver.
		delete Driver;
		Driver = 0;
	}
*/
	DebugLog->removeDisplayer ("CLIENT.LOG");
	InfoLog->removeDisplayer ("CLIENT.LOG");
	WarningLog->removeDisplayer ("CLIENT.LOG");
	ErrorLog->removeDisplayer ("CLIENT.LOG");
	AssertLog->removeDisplayer ("CLIENT.LOG");
}// release //
