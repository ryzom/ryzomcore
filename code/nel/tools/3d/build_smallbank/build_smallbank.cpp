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

// NeL include
#include "nel/misc/file.h"
#include "nel/3d/tile_bank.h"

using namespace NL3D;
using namespace NLMISC;

int main(int argc, char* argv[])
{
	// Arg ?
	if (argc<3)
	{
		// Doc
		printf ("build_smallbank [input.bank] [output.smallbank] [new_absolute_path]\n");
	}
	else
	{
		try
		{
			// Load the bank
			CTileBank bank;

			// Input file
			CIFile input;
			if (input.open (argv[1]))
			{
				// Serial the bank
				bank.serial (input);

				// Make a small bank
				bank.cleanUnusedData ();

				// Absolute path ?
				if (argc>3)
					bank.setAbsPath (argv[3]);

				// Output file
				COFile output;
				if (output.open (argv[2]))
				{
					// Serial the bank
					bank.serial (output);
				}
				else
				{
					// Error
					nlwarning ("ERROR can't open the file %s for writing", argv[2]);
				}
			}
			else
			{
				// Error
				nlwarning ("ERROR can't open the file %s for reading", argv[1]);
			}

		}
		catch (const Exception& e)
		{
			// Error
			nlwarning ("ERROR fatal error: %s", e.what());
		}
	}
	return 0;
}
