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

#include "nel/misc/types_nl.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef NL_OS_WINDOWS
#	include <io.h>
#	include <direct.h>
#endif

#include <vector>
#include <string>

#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include "nel/misc/common.h"


using namespace std;
using namespace NLMISC;

// ---------------------------------------------------------------------------

class CWildCard
{
public:
	string		Expression;
	bool		Not;
};
std::vector<CWildCard>	WildCards;

// ---------------------------------------------------------------------------

bool keepFile (const char *fileName)
{
	uint i;
	bool ifPresent = false;
	bool ifTrue = false;
	string file = toLower(CFile::getFilename (fileName));
	for (i=0; i<WildCards.size(); i++)
	{
		if (WildCards[i].Not)
		{
			// One ifnot condition met and the file is not added
			if (testWildCard(file.c_str(), WildCards[i].Expression.c_str()))
				return false;
		}
		else
		{
			ifPresent = true;
			ifTrue |= testWildCard(file.c_str(), WildCards[i].Expression.c_str());
		}
	}

	return !ifPresent || ifTrue;
}

// ---------------------------------------------------------------------------

struct BNPFile
{
	string Name;
	uint32 Size;
	uint32 Pos;
};

struct BNPHeader
{
	vector<BNPFile>			Files;
	uint32					OffsetFromBeginning;

	// Append the header to the big file
	bool append (const string &filename)
	{
		FILE *f = fopen (filename.c_str(), "ab");
		if (f == NULL) return false;

		uint32 nNbFile = (uint32)Files.size();

		// value to be serialized
		uint32 nNbFile2 = nNbFile;

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nNbFile2);
#endif

		if (fwrite (&nNbFile2, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		for (uint32 i = 0; i < nNbFile; ++i)
		{
			uint8 nStringSize = (uint8)Files[i].Name.size();
			if (fwrite (&nStringSize, 1, 1, f) != 1)
			{
				fclose(f);
				return false;
			}

			if (fwrite (Files[i].Name.c_str(), 1, nStringSize, f) != nStringSize)
			{
				fclose(f);
				return false;
			}

			uint32 nFileSize = Files[i].Size;

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(nFileSize);
#endif

			if (fwrite (&nFileSize, sizeof(uint32), 1, f) != 1)
			{
				fclose(f);
				return false;
			}

			uint32 nFilePos = Files[i].Pos;

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(nFilePos);
#endif

			if (fwrite (&nFilePos, sizeof(uint32), 1, f) != 1)
			{
				fclose(f);
				return false;
			}
		}

		uint32 nOffsetFromBeginning = OffsetFromBeginning;

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

		if (fwrite (&nOffsetFromBeginning, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		fclose (f);
		return true;
	}

	// Read the header from a big file
	bool read (const string &filename)
	{
		FILE *f = fopen (filename.c_str(), "rb");
		if (f == NULL) return false;

		nlfseek64 (f, 0, SEEK_END);
		uint32 nFileSize=CFile::getFileSize (filename);
		nlfseek64 (f, nFileSize-sizeof(uint32), SEEK_SET);

		uint32 nOffsetFromBeginning;
		if (fread (&nOffsetFromBeginning, sizeof(uint32), 1, f) != 1)
		{
			fclose (f);
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

		if (nlfseek64 (f, nOffsetFromBeginning, SEEK_SET) != 0)
		{
			fclose (f);
			return false;
		}
		
		uint32 nNbFile;
		if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
		{
			fclose (f);
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nNbFile);
#endif

		for (uint32 i = 0; i < nNbFile; ++i)
		{
			uint8 nStringSize;
			char sName[256];
			if (fread (&nStringSize, 1, 1, f) != 1)
			{
				fclose (f);
				return false;
			}
			if (fread (sName, 1, nStringSize, f) != nStringSize)
			{
				fclose (f);
				return false;
			}
			sName[nStringSize] = 0;
			BNPFile tmpBNPFile;
			tmpBNPFile.Name = sName;
			if (fread (&tmpBNPFile.Size, sizeof(uint32), 1, f) != 1)
			{
				fclose (f);
				return false;
			}

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(tmpBNPFile.Size);
#endif

			if (fread (&tmpBNPFile.Pos, sizeof(uint32), 1, f) != 1)
			{
				fclose (f);
				return false;
			}

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(tmpBNPFile.Pos);
#endif

			Files.push_back (tmpBNPFile);
		}

		fclose (f);
		return true;
	}
};

string gDestBNPFile;
BNPHeader gBNPHeader;

// ---------------------------------------------------------------------------
void append(const string &filename1, const string &filename2, uint32 sizeToRead)
{
	FILE *f1 = fopen(filename1.c_str(), "ab");
	FILE *f2 = fopen(filename2.c_str(), "rb");
	if (f1 == NULL) return;
	if (f2 == NULL) { fclose(f1); return; }
	
	uint8 *ptr = new uint8[sizeToRead];
	if (fread (ptr, sizeToRead, 1, f2) != 1)
		nlwarning("%s read error", filename2.c_str());
	if (fwrite (ptr, sizeToRead, 1, f1) != 1)
		nlwarning("%s write error", filename1.c_str());
	delete [] ptr;
	
	fclose(f2);
	fclose(f1);
}

// ---------------------------------------------------------------------------
bool i_comp(const string &s0, const string &s1)
{
	return nlstricmp (CFile::getFilename(s0).c_str(), CFile::getFilename(s1).c_str()) < 0;
}

void packSubRecurse ()
{
	vector<string>	pathContent;

	string cp = CPath::getCurrentPath();
	printf ("Treating directory : %s\n", cp.c_str());
	CPath::getPathContent(cp, true, false, true, pathContent);

	// Sort filename
	sort (pathContent.begin(), pathContent.end(), i_comp);

	uint i;
	for (i=0; i<pathContent.size(); i++)
	{
		if (keepFile (pathContent[i].c_str()))
		{
			BNPFile ftmp;

			// Check if we can read the source file
			FILE *f = fopen (pathContent[i].c_str(), "rb");
			if (f != NULL)
			{
				fclose (f);
				ftmp.Name = CFile::getFilename(pathContent[i]);
				ftmp.Size = CFile::getFileSize(pathContent[i]);
				ftmp.Pos = gBNPHeader.OffsetFromBeginning;
				gBNPHeader.Files.push_back(ftmp);
				gBNPHeader.OffsetFromBeginning += ftmp.Size;
				append(gDestBNPFile, pathContent[i].c_str(), ftmp.Size);
				printf("adding %s\n", pathContent[i].c_str());
			}
			else
			{
				printf("error cannot open %s\n", pathContent[i].c_str());
			}
		}
	}
}

// ---------------------------------------------------------------------------
void unpack (const string &dirName)
{
	FILE *bnp = fopen (gDestBNPFile.c_str(), "rb");
	FILE *out;
	if (bnp == NULL)
		return;

	for (uint32 i = 0; i < gBNPHeader.Files.size(); ++i)
	{
		BNPFile &rBNPFile = gBNPHeader.Files[i];
		string filename = dirName + "/" + rBNPFile.Name;
		out = fopen (filename.c_str(), "wb");
		if (out != NULL)
		{
			nlfseek64 (bnp, rBNPFile.Pos, SEEK_SET);
			uint8 *ptr = new uint8[rBNPFile.Size];
			if (fread (ptr, rBNPFile.Size, 1, bnp) != 1)
				nlwarning("%s read error", filename.c_str());
			if (fwrite (ptr, rBNPFile.Size, 1, out) != 1)
				nlwarning("%s write error", filename.c_str());
			fclose (out);
			delete [] ptr;
		}
	}
	fclose (bnp);
}

// ---------------------------------------------------------------------------
void usage()
{
	printf ("USAGE : \n");
	printf ("   bnp_make /p <directory_name> [<destination_path>] [<destination_filename>] [option] ... [option]\n");
	printf ("   option : \n");
	printf ("      -if wildcard : add the file if it matches the wilcard (at least one 'if' conditions must be met for a file to be adding)\n");
	printf ("      -ifnot wildcard : add the file if it doesn't match the wilcard (all the 'ifnot' conditions must be met for a file to be adding)\n");
	printf (" Pack the directory to a bnp file\n");
	printf ("   bnp_make /u <bnp_file>\n");
	printf (" Unpack the bnp file to a directory\n");
	printf ("   bnp_make /l <bnp_file>\n");
	printf (" List the files contained in the bnp file\n");
}

// ---------------------------------------------------------------------------

uint readOptions (int nNbArg, char **ppArgs)
{
	uint i;
	uint optionCount = 0;
	for (i=0; i<(uint)nNbArg; i++)
	{
		// If ?
		if ((strcmp (ppArgs[i], "-if") == 0) && ((i+1)<(uint)nNbArg))
		{
			CWildCard card;
			card.Expression = toLower(string(ppArgs[i+1]));
			card.Not = false;
			WildCards.push_back (card);
			optionCount += 2;
		}
		// If not ?
		if ((strcmp (ppArgs[i], "-ifnot") == 0) && ((i+1)<(uint)nNbArg))
		{
			CWildCard card;
			card.Expression = toLower(string(ppArgs[i+1]));
			card.Not = true;
			WildCards.push_back (card);
			optionCount += 2;
		}
	}
	return optionCount;
}

// ---------------------------------------------------------------------------
int main (int nNbArg, char **ppArgs)
{
	NLMISC::CApplicationContext myApplicationContext;

	if (nNbArg < 3)
	{
		usage();
		return -1;
	}

	if ((strcmp(ppArgs[1], "/p") == 0) || (strcmp(ppArgs[1], "/P") == 0) ||
		(strcmp(ppArgs[1], "-p") == 0) || (strcmp(ppArgs[1], "-P") == 0))
	{
		// Pack a directory
		uint count = readOptions (nNbArg, ppArgs);
		nNbArg -= count;

		// Read options

		string sCurDir;

		if (nNbArg >= 4)
		{
			// store current path
			sCurDir = CPath::getCurrentPath();

			// go to the dest path
			string sDestDir;
			if (CPath::setCurrentPath(ppArgs[3]))
			{
				sDestDir = CPath::getCurrentPath();

				bool tmp = CPath::setCurrentPath(sCurDir.c_str());
				// restore current path, should not failed
				nlassert (tmp); // removed in release

				// go to the source dir
				if (CPath::setCurrentPath(ppArgs[2]))
				{
					sCurDir = CPath::getCurrentPath();

					gDestBNPFile = CPath::standardizePath(sDestDir);

					if(nNbArg == 5)
					{
						gDestBNPFile += ppArgs[4];
						// add ext if necessary
						if (string(ppArgs[4]).find(".") == string::npos)
							gDestBNPFile += string(".bnp");
					}
					else
					{
						const char *pos = strrchr (sCurDir.c_str(), '/');
						if (pos != NULL)
						{
							gDestBNPFile += string(pos+1);
						}
						// get the dest file name
						gDestBNPFile += string(".bnp");
					}
				}
				else
				{
					nlwarning ("ERROR (bnp_make) : can't set current directory to %s", ppArgs[2]);
					return -1;
				}
			}
			else
			{
				nlwarning ("ERROR (bnp_make) : can't set current directory to %s", ppArgs[3]);
				return -1;
			}
		}
		else
		{
			if (chdir (ppArgs[2]) == -1)
			{
				nlwarning ("ERROR (bnp_make) : can't set current directory to %s", ppArgs[2]);
				return -1;
			}
			//getcwd (sCurDir, MAX_PATH);
			gDestBNPFile = CPath::getCurrentPath() + string(".bnp");
		}
		
		remove (gDestBNPFile.c_str());
		gBNPHeader.OffsetFromBeginning = 0;	
		packSubRecurse();
		gBNPHeader.append (gDestBNPFile);
		return 0;
	}

	if ((strcmp(ppArgs[1], "/u") == 0) || (strcmp(ppArgs[1], "/U") == 0) ||
		(strcmp(ppArgs[1], "-u") == 0) || (strcmp(ppArgs[1], "-U") == 0))
	{
		string::size_type i;
		string path;
		gDestBNPFile = ppArgs[2];
		if ((gDestBNPFile.rfind('/') != string::npos) || (gDestBNPFile.rfind('/') != string::npos))
		{
			string::size_type pos = gDestBNPFile.rfind('/');
			if (pos == string::npos)
				pos = gDestBNPFile.rfind('/');
			for (i = 0; i <= pos; ++i)
				path += gDestBNPFile[i];
			string wholeName = gDestBNPFile;
			gDestBNPFile = "";
			for (; i < wholeName.size(); ++i)
				gDestBNPFile += wholeName[i];
			if (CPath::setCurrentPath(path.c_str()))
			{
				path = CPath::getCurrentPath();
			}
			else
			{
				nlwarning ("ERROR (bnp_make) : can't set current directory to %s", path.c_str());
				return -1;
			}
		}
		if (stricmp (gDestBNPFile.c_str()+gDestBNPFile.size()-4, ".bnp") != 0)
		{
			gDestBNPFile += ".bnp";
		}
		string dirName;
		for (i = 0; i < gDestBNPFile.size()-4; ++i)
			dirName += gDestBNPFile[i];
		// Unpack a bnp file
		if (!gBNPHeader.read (gDestBNPFile))
			return -1;

		//mkdir (dirName.c_str());
		CFile::createDirectory(dirName);

		unpack (dirName);

		return 0;
	}

	if ((strcmp(ppArgs[1], "/l") == 0) || (strcmp(ppArgs[1], "/L") == 0) ||
		(strcmp(ppArgs[1], "-l") == 0) || (strcmp(ppArgs[1], "-L") == 0))
	{
		string::size_type i;
		string path;
		gDestBNPFile = ppArgs[2];
		if ((gDestBNPFile.rfind('/') != string::npos) || (gDestBNPFile.rfind('/') != string::npos))
		{
			string::size_type pos = gDestBNPFile.rfind('/');
			if (pos == string::npos)
				pos = gDestBNPFile.rfind('/');
			for (i = 0; i <= pos; ++i)
				path += gDestBNPFile[i];
			string wholeName = gDestBNPFile;
			gDestBNPFile = "";
			for (; i < wholeName.size(); ++i)
				gDestBNPFile += wholeName[i];
			if (CPath::setCurrentPath(path.c_str()))
			{
				path = CPath::getCurrentPath();
			}
			else
			{
				nlwarning ("ERROR (bnp_make) : can't set current directory to %s", path.c_str());
				return -1;
			}
		}
		if (stricmp (gDestBNPFile.c_str()+gDestBNPFile.size()-4, ".bnp") != 0)
		{
			gDestBNPFile += ".bnp";
		}
		string dirName;
		for (i = 0; i < gDestBNPFile.size()-4; ++i)
			dirName += gDestBNPFile[i];

		// Unpack a bnp file
		if (!gBNPHeader.read (gDestBNPFile))
			return -1;

		for (i = 0; i < gBNPHeader.Files.size(); ++i)
			printf ("%s\n", gBNPHeader.Files[i].Name.c_str());

		return 0;
	}

	usage ();
	return -1;
}
