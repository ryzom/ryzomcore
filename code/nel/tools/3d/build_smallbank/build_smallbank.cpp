// build_smallbank.cpp : build a small bank.
//

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
		catch (Exception& e)
		{
			// Error
			nlwarning ("ERROR fatal error: %s", e.what());
		}
	}
	return 0;
}
