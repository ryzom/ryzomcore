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

// mkdir_date.cpp : Defines the entry point for the console application.
//

#include <time.h>
#include <stdio.h>
#include <direct.h> 
#include <string.h> 

int main(int argc, char* argv[])
{
	// Help
	bool keepExt = false;
	bool help = ( (argc<3) || (argc>4) );
	if (argc == 4)
	{
		if (strcmp (argv[3], "-keepExt") == 0)
			keepExt = true;
		else
			help = true;
	}

	if (help)
	{
		printf ("ren_date [old_name] [new_base_name] [-keepExt]");
		return 0;
	}

	// Get the time
	time_t aclock;
	time(&aclock);

	// Get time information
	const struct tm *timeptr=localtime(&aclock);

	// Extension
	char ext[512];

	// Empty string 
	strcpy (ext, "");

	// Keep the extension
	if (keepExt)
	{
		// Get the extension
		char *point = strrchr (argv[2], '.');
		if (point)
		{
			strcpy (ext, point);
			*point = 0;
		}
	}

	// Format the string
	char date[512];
	strftime( date, sizeof(date), "%Y %m %d %Hh%M", timeptr);

	// Name of the directory
	char directory[512];
	sprintf (directory, "%s %s%s", argv[2], date, ext);

	// Create a directory
	return rename ( argv[1], directory )==0;
}
