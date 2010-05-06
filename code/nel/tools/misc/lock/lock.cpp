// lock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <nel/misc/types_nl.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/common.h>
#include <stdio.h>

using namespace NLMISC;

int main(int argc, char* argv[])
{
	if (argc <3)
	{
		printf ("lock.exe [filein] [fleout]\n\t");
	}
	else
	{
		// Random
		srand ((int)CTime::getLocalTime ());

		// Open the filein
		while (1)
		{
			// Rename the file
			if (rename(argv[1], argv[2]) == 0)
			{
				return 1;
			}
			else
			{
				// Sleep..
				printf ("Locked. Waiting for lock to be released\n");
				uint sleepTime = (rand () * 5000) / RAND_MAX;
				nlSleep (sleepTime);
			}
		}

		// Open the fileout
		FILE *fileOut = fopen (argv[2], "w");
	}

	return 0;
}
