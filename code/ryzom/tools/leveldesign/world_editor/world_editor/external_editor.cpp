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

// dialog_properties.cpp : implementation file
//

#include "stdafx.h"
#include <direct.h>

using namespace std;
using namespace NLLIGO;
using namespace NLMISC;

// ***************************************************************************

bool EditExternalText (const std::string &editor, std::string &text, const std::string &ext)
{
	bool status = false;
	// Create a temporary file
	char dir[512];
	if (getcwd (dir, 512))
	{
		// Build a temporary filename
		string tempFilename;
		uint i = 0;
		do
			tempFilename = string(dir)+"/~tmp"+toString (i++)+"."+ext;
		while (NLMISC::CFile::isExists(tempFilename));

		// Fill the temp file
		bool saved = false;
		FILE *file = fopen (tempFilename.c_str(), "w");
		if (file)
		{
			saved = fputs (text.c_str(), file) != EOF;
			fclose (file);
		}

		// Hide the file
		SetFileAttributes (tempFilename.c_str(), FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM);

		// Open the temp file with a text editor
		if (saved)
		{
			STARTUPINFO         si;
			PROCESS_INFORMATION pi;
			memset(&si, 0, sizeof(si));
			memset(&pi, 0, sizeof(pi));
			si.cb = sizeof(si);
			char cmdLine[1024];
			strncpy (cmdLine, ("\""+editor+"\" \""+tempFilename+"\"").c_str(), sizeof(cmdLine)-1);
			if (CreateProcess(editor.c_str (), cmdLine, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi))
			{
				if (WaitForSingleObject (pi.hProcess, INFINITE) == WAIT_OBJECT_0)
				{
					// Open the file..
					std::string tempText;
					FILE *file = fopen (tempFilename.c_str(), "r");
					if (file)
					{
						// Read the new file
						char buffer[513];
						int red;
						while (red=fread (buffer, 1, 512, file))
						{
							buffer[red] = 0;
							tempText += buffer;
						}
						fclose (file);

						// Return the text
						text = tempText;
						status = true;
					}
				}
			}
		}

		// Delete the file
		NLMISC::CFile::deleteFile (tempFilename);
	}
	return status;
}
