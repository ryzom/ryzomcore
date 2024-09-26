// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef CL_INIT_H
#define CL_INIT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/progress_callback.h"

namespace NLMISC
{
	class IProgressCallback;
}

// Initialize the log
void initLog();

// Initialize the application before login step
void prelogInit();

// Initialize the application after login step
void postlogInit();

void initStreamedPackageManager(NLMISC::IProgressCallback &progress);
void addSearchPaths(NLMISC::IProgressCallback &progress);
void addPreDataPaths(NLMISC::IProgressCallback &progress);

void ExitClientError (const char *format, ...);


#endif // CL_INIT_H

/* End of init.h */
