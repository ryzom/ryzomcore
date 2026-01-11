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

#include "nel/misc/debug.h"
#include <windows.h>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	// Help ?
	if (argc<3)
	{
		printf ("exec_timeout [timeout(ms)] [prog.exe] [arg0] [arg1]...\nReturn 0:error, 1:ok, 2:timeout\n");
		return 0;
	}
	else
	{
		// String
		string argu;

		// Build the argus
		for (int i=2; i<argc; i++)
		{
			argu += argv[i];
			if (i<argc-1)
				argu += " ";
		}

		// Process information
		PROCESS_INFORMATION process_info;

		// Timeout
		DWORD timeout = atoi (argv[1]);

		// Startup info
		STARTUPINFO startupInfo;
		GetStartupInfo (&startupInfo);

		// Exec
		if (CreateProcess( NULL, (char*)(argu.c_str()), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &process_info))
		{
			// Return message
			switch (WaitForSingleObject ( process_info.hProcess, timeout))
			{
			case WAIT_OBJECT_0:
				return 1;
			case WAIT_ABANDONED:
				return 0;
			case WAIT_TIMEOUT:
				if (TerminateProcess (process_info.hProcess, 0))
				{
					nlwarning ("ERROR: Timeout in process %s", (char*)(argu.c_str()));
					return 2;
				}
				else
					nlwarning ("ERROR: Error while terminate current process %s", (char*)(argu.c_str()));
					return 0;
			}
		}
		else
			nlwarning ("ERROR: Error can't exec process %s", (char*)(argu.c_str()));
			return 0;
	}

	// Ok
	return 1;
}
