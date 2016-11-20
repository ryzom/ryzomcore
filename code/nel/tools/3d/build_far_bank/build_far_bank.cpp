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

#include "nel/misc/file.h"
#include "nel/3d/tile_bank.h"
#include "nel/3d/tile_far_bank.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#define BUILD_FAR_BANK_VERSION "1.0"

#if defined (NL_OS_WINDOWS)
#define STAT _stat
#else // NL_OS_WINDOWS
#define STAT stat
#endif // NL_OS_WINDOWS

// Return true if f2 must be recomputed with f1, else return false.
// return true if date f1 > date f2
// If f1 doesn't exist, returns false
// If f2 doesn't exist, returns true
bool recompute (const char* f1, const char* f2)
{
	struct STAT buf1;
	struct STAT buf2;
	if (STAT (f1, &buf1)!=0)
		return false;
	if (STAT (f2, &buf2)!=0)
		return true;
	return buf1.st_mtime > buf2.st_mtime;
}

// Fill tile far pixel with this bitmap
bool fillTileFar (uint tile, const char* sName, CTileFarBank::TFarType type, CTileFarBank& farBank, bool _256, uint8 rotate)
{
	// Progress message
	printf ("Computing %s...\n", sName);

	// Create a stream
	CIFile inputBitmap;
	if (inputBitmap.open (sName))
	{
		try
		{
			// Load the texture
			CBitmap bitmap;
			bitmap.load (inputBitmap);

			// Convert to RGBA
			bitmap.convertToType (CBitmap::RGBA);

			// Rotate
			uint8 rot=rotate;
			while (rot!=0)
			{
				bitmap.rotateCCW ();
				rot--;
			}

			// Get bitmap size
			uint width=bitmap.getWidth();
			uint height=bitmap.getHeight();


			// Check size..
			if ((!((_256&&(width==256)&&(height==256))||((!_256)&&(width==128)&&(height==128)))))
			{
				// New size
				width = height = _256?256:128;

				// Output a warning message
				if ((type!=CTileFarBank::alpha)||(width!=64)||(height!=64))
					nlwarning ("WARNING resize %s to %d x %d\n", sName, width, height);

				// Resample the picture
				bitmap.resample ( width, height);
			}

			// Build mipmaps
			bitmap.buildMipMaps ();

			// Get the tile
			CTileFarBank::CTileFar* pFarTile=farBank.getTile (tile);

			// Copy arrays
			pFarTile->setPixels (type, CTileFarBank::order0, (CRGBA*)&bitmap.getPixels (5)[0], (width>>5)*(height>>5));
			pFarTile->setPixels (type, CTileFarBank::order1, (CRGBA*)&bitmap.getPixels (6)[0], (width>>6)*(height>>6));
			pFarTile->setPixels (type, CTileFarBank::order2, (CRGBA*)&bitmap.getPixels (7)[0], (width>>7)*(height>>7));

			// Ok.
			return true;
		}
		catch (const Exception& except)
		{
			nlwarning ("ERROR %s\n", except.what());
		}
	}
	else
	{
		nlwarning ("ERROR can't open bitmap %s for reading\n", sName);
	}
	return false;
}

// Go go go !
int main (int argc, char **argv)
{
	NLMISC::CApplicationContext myApplicationContext;

	// Options
	bool useCurrentPath=false;
	bool forceRecomputation=false;
	bool outputVersion=false;
	bool outputHelp=false;
	string postfix;
	string rootDir;

	// If no argu, show help
	if (argc==1)
		outputHelp=true;

	int nFirstGoodArgu=3;

	// Correct number of argu..
	// Check for options
	for (int n=1; n<argc; n++)
	{
		if (strcmp (argv[n], "-r")==0 || strcmp (argv[n], "-R")==0)
			useCurrentPath=true;
		if (strcmp (argv[n], "-f")==0 || strcmp (argv[n], "-F")==0)
			forceRecomputation=true;
		if (strcmp (argv[n], "-v")==0 || strcmp (argv[n], "-V")==0)
			outputVersion=true;
		if (strcmp (argv[n], "-?")==0)
			outputHelp=true;
		if (strncmp (argv[n], "-d", 2)==0)
		{
			rootDir = argv[n];
			rootDir = rootDir.substr (2, rootDir.size ()-2);
			rootDir = CPath::standardizePath (rootDir);
		}
		if (strncmp (argv[n], "-p", 2)==0)
		{
			postfix = argv[n];
			postfix = postfix.substr (2, postfix.size ()-2);
		}
	}

	// Output version?
	if (outputVersion)
		printf ("Version %s\n", BUILD_FAR_BANK_VERSION);

	// Output help?
	if (outputHelp)
	{
		// Ok, some help if not enough argu
		printf (
			"build_far_bank [input.bank][output.farbank][-r][-f][-v][-?]\n"
			"options:\n"
			"\t-d#: change the root directory of the small bank. # is the new directory\n"
			"\t-p#: postfix tiles filename by #\n"
			"\t-r: load the bitmaps from the current directory\n"
			"\t-f: force recomputation of all the tiles\n"
			"\t-v: print the version\n"
			"\t-?: print help\n"
			);
	}

	if (argc>=3)
	{

		// Count number of tiles
		int tileCount=0;
		int tileComputed=0;

		// Open input file
		CIFile inputFile;
		if (inputFile.open(argv[1]))
		{
			// Get some incoming exceptions
			try
			{
				// Ok, create a far tile bank
				CTileFarBank farBank;

				// Try to open input file
				CIFile inputFarBank;
				if (inputFarBank.open(argv[2]))
				{
					// Serial the bank in input
					farBank.serial (inputFarBank);
				}

				// Force recomputation ?
				if (recompute (argv[1], argv[2]))
				{
					// Progress message
					printf ("%s have been modified, recompute all the tiles...\n", argv[1]);
					forceRecomputation=true;
				}

				// Close this file
				inputFarBank.close ();

				// Progress message
				printf ("Reading the bank...\n");

				// Create a bank
				CTileBank bank;

				// Serialize the input bank
				bank.serial (inputFile);

				// Path relative
				if (useCurrentPath)
					bank.makeAllPathRelative ();

				// Change root dir ?
				if (!rootDir.empty ())
					bank.setAbsPath (rootDir);

				// Postfix tiles ?
				if (!postfix.empty ())
					bank.postfixTileFilename (postfix.c_str ());

				// Resize far bank
				farBank.setNumTile (bank.getTileCount());

				// Scan each tiles...
				int tile;
				for (tile=0; tile<bank.getTileCount(); tile++)
				{
					// Get a pointer on this tile
					CTile *pTile=bank.getTile(tile);

					// Delete pixels
					bool bDeleteDiffuse=true;
					bool bDeleteAdditive=true;
					bool bDeleteAlpha=true;

					// Tile not free ?
					if (!pTile->isFree())
					{
						// Get infos about the tile
						sint tileSet;
						sint number;
						CTileBank::TTileType type;
						bank.getTileXRef (tile, tileSet, number, type);

						// Is a 256 ?
						bool _256=(type==CTileBank::_256x256);

						// Diffuse bitmap filled ?
						if (!pTile->getRelativeFileName (CTile::diffuse).empty())
						{
							// File exist ?
							string tileFilename = bank.getAbsPath()+CPath::standardizePath(pTile->getRelativeFileName (CTile::diffuse), false);
							if (CFile::fileExists(tileFilename))
							{
								// Recompute it?
								if (recompute (tileFilename.c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, tileFilename.c_str(), CTileFarBank::diffuse, farBank, _256, 0))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteDiffuse=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;

									printf ("Skipping %s...\n", tileFilename.c_str());
									bDeleteDiffuse=false;
								}
							}
						}

						// Additive bitmap filled ?
						if (!pTile->getRelativeFileName (CTile::additive).empty())
						{
							// File exist ?
							string tileFilename = bank.getAbsPath()+CPath::standardizePath(pTile->getRelativeFileName (CTile::additive), false);
							if (CFile::fileExists(tileFilename))
							{
								// Recompute it?
								if (recompute (tileFilename.c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, tileFilename.c_str(), CTileFarBank::additive, farBank, _256, 0))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteAdditive=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;

									printf ("Skipping %s...\n", tileFilename.c_str());
									bDeleteAdditive=false;
								}
							}
						}

						// Alpha bitmap filled ?
						if (!pTile->getRelativeFileName (CTile::alpha).empty())
						{
							// File exist ?
							string tileFilename = bank.getAbsPath()+CPath::standardizePath(pTile->getRelativeFileName (CTile::alpha), false);
							if (CFile::fileExists(tileFilename))
							{
								// Recompute it?
								if (recompute (tileFilename.c_str(), argv[2])||forceRecomputation)
								{
									// Fill infos
									if (fillTileFar (tile, tileFilename.c_str(), CTileFarBank::alpha, farBank, _256, pTile->getRotAlpha()))
									{
										// One more tile
										tileCount++;

										tileComputed++;
										bDeleteAlpha=false;
									}
								}
								else
								{
									// One more tile
									tileCount++;

									printf ("Skipping %s...\n", tileFilename.c_str());
									bDeleteAlpha=false;
								}
							}
							else
							{
								nlwarning ("ERROR tile file not found %s\n", tileFilename.c_str ());
							}
						}
					}

					// Delete diffuse pixels?
					if (bDeleteDiffuse)
						farBank.getTile (tile)->erasePixels (CTileFarBank::diffuse);

					// Delete additif pixels?
					if (bDeleteAdditive)
						farBank.getTile (tile)->erasePixels (CTileFarBank::additive);
				}

				// Open output file
				COFile outputFile;
				if (outputFile.open(argv[2]))
				{
					// Progress message
					printf ("Writing %s...\n", argv[2]);

					// Writing the bank
					farBank.serial (outputFile);

					// Progress message
					printf ("%d far tiles found.\n", tileCount);
					printf ("%d far tiles computed.\n", tileComputed);
				}
				else	// Open failed
				{
					nlwarning ("ERROR Can't open file %s for writing\n", argv[2]);
				}
			}
			catch (const Exception& except)
			{
				nlwarning ("ERROR %s\n", except.what());
			}
		}
		else	// Open failed
		{
			nlwarning ("ERROR Can't open file %s for reading\n", argv[1]);
		}
	}

	// exit
	return 0;
}
