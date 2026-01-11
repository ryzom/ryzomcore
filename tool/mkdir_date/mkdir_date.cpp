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

int main(int argc, char* argv[])
{
	// Help
	if (argc<2)
	{
		printf ("mkdir_date [directory]");
		return 0;
	}

	// Get the time
	time_t aclock;
	time(&aclock);

	// Get time information
	const struct tm *timeptr=localtime(&aclock);

	// Format the string
	char date[512];
	strftime( date, sizeof(date), "%Y %m %d %Hh%M", timeptr);

	// Name of the directory
	char directory[512];
	sprintf (directory, "%s %s", argv[1], date);

	// Create a directory
	return _mkdir( directory )==0;
}
