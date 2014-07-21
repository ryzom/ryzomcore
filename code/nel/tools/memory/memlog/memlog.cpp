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

#include <nel/misc/types_nl.h>
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
	while ((c = fgetc (file)))
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
		while (fgets(buffer, 512, stdin))
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

#ifdef NL_BIG_ENDIAN
					NLMISC_BSWAP32(size);
#endif

					while (1)
					{
						uint32 start;
						if (fread (&start, sizeof(uint32), 1, file) != 1)
							break;

#ifdef NL_BIG_ENDIAN
						NLMISC_BSWAP32(start);
#endif

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
