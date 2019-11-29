// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/streamed_package.h"
#include "nel/misc/seven_zip.h"

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

std::string SourceDirectory;
std::string PackageFileName;
std::string StreamDirectory;

CStreamedPackage Package;

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
void usage()
{
	printf ("USAGE : \n");
	printf ("   snp_make -p <directory_name> <package_file> <stream_directory> [option] ... [option]\n");
	printf ("   option : \n");
	printf ("      -if wildcard : add the file if it matches the wilcard (at least one 'if' conditions must be met for a file to be adding)\n");
	printf ("      -ifnot wildcard : add the file if it doesn't match the wilcard (all the 'ifnot' conditions must be met for a file to be adding)\n");
	printf (" Pack the directory to a snp file\n");
	printf ("   snp_make -l <package_file>\n");
	printf (" List the files contained in the snp file\n");
}

// ---------------------------------------------------------------------------

void generateLZMA(const std::string &sourceFile, const std::string &outputFile)
{
	NLMISC::packLZMA(sourceFile, outputFile);

	/*
	std::string cmd="lzma e ";
	cmd+=" "+sourceFile+" "+outputFile;
	nlinfo("executing system command: %s",cmd.c_str());
#ifdef NL_OS_WINDOWS
	_spawnlp(_P_WAIT, "lzma.exe","lzma.exe", "e", sourceFile.c_str(), outputFile.c_str(), NULL);
#else // NL_OS_WINDOWS
	sint error = system (cmd.c_str());
	if (error)
		nlwarning("'%s' failed with error code %d", cmd.c_str(), error);
#endif // NL_OS_WINDOWS
	*/
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
		if (nNbArg < 5)
		{
			usage();
			return -1;
		}

		SourceDirectory = ppArgs[2];
		PackageFileName = ppArgs[3];
		StreamDirectory = ppArgs[4];
		readOptions(nNbArg, ppArgs);
		
		nldebug("Make streamed package: '%s'", PackageFileName.c_str());

		if (CFile::fileExists(PackageFileName))
		{
			nldebug("Update existing package");
			try
			{
				CIFile fi;
				fi.open(PackageFileName);
				fi.serial(Package);
			}
			catch (Exception &e)
			{
				nlwarning("ERROR (snp_make) : serial exception: '%s'", e.what());
				return -1;
			}
		}
		else
		{
			nldebug("New package");
		}

		std::vector<std::string> pathContent; // contains full pathnames
		std::vector<std::string> nameContent; // only filename
		CPath::getPathContent(SourceDirectory, true, false, true, pathContent);
		nameContent.reserve(pathContent.size());
		for (std::vector<std::string>::size_type i = 0; i < pathContent.size(); ++i)
		{
			const std::string &file = pathContent[i];
			if (keepFile(file.c_str()))
			{
				std::string fileName = NLMISC::toLower(CFile::getFilename(file));
				// nldebug("File: '%s' ('%s')", file.c_str(), fileName.c_str());
				nameContent.push_back(fileName);
				nlassert(nameContent.size() == (i + 1));
			}
			else
			{
				// Not included in this package
				pathContent.erase(pathContent.begin() + i);
				--i;
			}
		}

		std::vector<sint> packageIndex; // index of file in package
		packageIndex.resize(pathContent.size(), -1);

		for (CStreamedPackage::TEntries::size_type i = 0; i < Package.Entries.size(); ++i)
		{
			const CStreamedPackage::CEntry &entry = Package.Entries[i];
			
			sint foundIndex = -1; // find index in found file list
			for (std::vector<std::string>::size_type j = 0; j < pathContent.size(); ++j)
			{
				if (nameContent[j] == entry.Name)
				{
					foundIndex = j;
					break;
				}
			}

			if (foundIndex < 0)
			{
				nlinfo("File no longer exists: '%s'", entry.Name.c_str());
				Package.Entries.erase(Package.Entries.begin() + i);
				--i;
			}
			else
			{
				// File still exists, map it
				packageIndex[foundIndex] = i;
			}
		}

		for (std::vector<std::string>::size_type i = 0; i < pathContent.size(); ++i)
		{
			sint pidx = packageIndex[i];
			const std::string &name = nameContent[i];
			const std::string &path = pathContent[i];

			if (pidx < 0)
			{
				nlinfo("File added: '%s'", name.c_str());
				pidx = Package.Entries.size();
				Package.Entries.push_back(CStreamedPackage::CEntry());
				Package.Entries[pidx].Name = name;
				Package.Entries[pidx].LastModified = 0;
				Package.Entries[pidx].Size = 0;
			}
			else
			{
				nlinfo("File check for changes: '%s'", name.c_str());
			}

			CStreamedPackage::CEntry &entry = Package.Entries[pidx];

			std::string targetLzmaOld; // in case lzma wasn't made make sure it exists a second run
			CStreamedPackage::makePath(targetLzmaOld, entry.Hash);
			targetLzmaOld = StreamDirectory + targetLzmaOld + ".lzma";

			uint32 lastModified = CFile::getFileModificationDate(path);
			uint32 fileSize = CFile::getFileSize(path);
			if (lastModified > entry.LastModified || fileSize != entry.Size || !CFile::fileExists(targetLzmaOld))
			{
				entry.LastModified = lastModified;

				nlinfo("Calculate file hash");
				CHashKey hash = getSHA1(path, true);
				/*nldebug("%s", hash.toString().c_str());
				std::string hashPath;
				CStreamedPackage::makePath(hashPath, hash);
				nldebug("%s", hashPath.c_str());*/

				if (hash == entry.Hash && fileSize == entry.Size)
				{
					// File has not changed
				}
				else
				{
					nlinfo("File changed");
					entry.Hash = hash;
					entry.Size = fileSize;
				}

				std::string targetLzma; // in case lzma wasn't made make sure it exists a second run
				CStreamedPackage::makePath(targetLzma, entry.Hash);
				targetLzma = StreamDirectory + targetLzma + ".lzma";

				if (!CFile::fileExists(targetLzma))
				{
					// make the compressed file
					nlinfo("%s -> %s", path.c_str(), targetLzma.c_str());
					CFile::createDirectoryTree(CFile::getPath(targetLzma));
					generateLZMA(path, targetLzma);
				}
			}
		}

		try
		{
			nldebug("Store package '%s'", PackageFileName.c_str());
			COFile fo;
			fo.open(PackageFileName);
			fo.serial(Package);
		}
		catch (Exception &e)
		{
			nlwarning("ERROR (snp_make) : serial exception: '%s'", e.what());
			return -1;
		}

		return 0;
	}	

	if ((strcmp(ppArgs[1], "/l") == 0) || (strcmp(ppArgs[1], "/L") == 0) ||
		(strcmp(ppArgs[1], "-l") == 0) || (strcmp(ppArgs[1], "-L") == 0))
	{
		PackageFileName = ppArgs[2];
		if (!CFile::fileExists(PackageFileName))
		{
			nlwarning("ERROR (snp_make) : package doesn't exist: '%s'", PackageFileName.c_str());
			return -1;
		}

		try
		{
			CIFile fi;
			fi.open(PackageFileName);
			fi.serial(Package);
		}
		catch (Exception &e)
		{
			nlwarning("ERROR (snp_make) : serial exception: '%s'", e.what());
			return -1;
		}

		for (CStreamedPackage::TEntries::const_iterator it(Package.Entries.begin()), end(Package.Entries.end()); it != end; ++it)
		{
			const CStreamedPackage::CEntry &entry = (*it);

			printf("List files in '%s'", PackageFileName.c_str());
			printf("%s { Hash: '%s', Size: '%u', LastModified: '%u' }", entry.Name.c_str(), entry.Hash.toString().c_str(), entry.Size, entry.LastModified);
		}
		
		return 0;
	}

	usage ();
	return -1;
}
