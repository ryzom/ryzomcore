// memlog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <nel/misc/common.h>
#include <map>

using namespace std;

class CLogEntry
{
public:
	uint32			Start;
	uint32			End;
	uint16			Line;
	std::string		Category;
	std::string		Filename;
};

bool readString (string &str, FILE *file)
{
	int c;
	while (c = fgetc (file))
	{
		if (c == EOF)
			return false;
		str += c;
	}
	return true;
}

int main(int argc, char* argv[])
{
	if (argc>1)
	{
		char buffer[512];
		while (gets (buffer))
		{
			uint32 address;
			if (strcmp (buffer, "help") == 0)
			{
				printf ("Commands available:\n");
				printf ("help                     | Show this help\n");
				printf ("address [memoryaddress]  | Show history of this address\n");
				printf ("\n");
				printf ("memoryaddress must be like this: 8b125df\n");
			}
			else if (sscanf (buffer, "address %x", &address) == 1)
			{
				printf ("0x%x history:\n", address);
				// Read the memory log
				FILE *file = fopen (argv[1], "rb");
				if (file)
				{
					uint32 size;
					if (fread (&size, sizeof(uint32), 1, file) != 1)
						break;

					while (1)
					{
						uint32 start;
						if (fread (&start, sizeof(uint32), 1, file) != 1)
							break;
						string category;
						if (!readString (category, file))
							break;
					
						if (start <= address && address < (start+size))
							printf ("0x%x (%d bytes) %s\n", start, size, category.c_str ());
					}
					fclose (file);
					printf ("done\n");

				}
				else
				{
					fprintf (stderr, "Can't open the file %s for reading\n", argv[1]);
				}
			}
			else
			{
				printf ("Command unknown, try help\n");
			}
		}
	}
	else
	{
		printf ("memlog [filename.memlog]\n");
	}

	return 0;
}
