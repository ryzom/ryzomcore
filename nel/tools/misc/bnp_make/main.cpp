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
#include "nel/misc/big_file.h"
#include "nel/misc/cmd_args.h"


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

bool keepFile (const std::string &fileName)
{
	uint i;
	bool ifPresent = false;
	bool ifTrue = false;
	string file = toLowerAscii(CFile::getFilename (fileName));
	for (i=0; i<WildCards.size(); i++)
	{
		if (WildCards[i].Not)
		{
			// One ifnot condition met and the file is not added
			if (testWildCard(file, WildCards[i].Expression))
				return false;
		}
		else
		{
			ifPresent = true;
			ifTrue |= testWildCard(file, WildCards[i].Expression);
		}
	}

	return !ifPresent || ifTrue;
}

// ---------------------------------------------------------------------------

NLMISC::CBigFile::BNP gBNPHeader;

// ---------------------------------------------------------------------------
bool i_comp(const string &s0, const string &s1)
{
	return nlstricmp (CFile::getFilename(s0).c_str(), CFile::getFilename(s1).c_str()) < 0;
}

bool packSubRecurse(const std::string &srcDirectory)
{
	vector<string>	pathContent;

	printf ("Treating directory: %s\n", srcDirectory.c_str());
	CPath::getPathContent(srcDirectory, true, false, true, pathContent);

	if (pathContent.empty()) return true;

	// Sort filename
	sort (pathContent.begin(), pathContent.end(), i_comp);

	// check for files with same name
	for(uint i = 1, len = pathContent.size(); i < len; ++i)
	{
		if (toLowerAscii(CFile::getFilename(pathContent[i-1])) == toLowerAscii(CFile::getFilename(pathContent[i])))
		{
			nlwarning("File %s is not unique in BNP!", CFile::getFilename(pathContent[i]).c_str());
			return false;
		}
	}

	for (uint i=0; i<pathContent.size(); ++i)
	{
		if (keepFile(pathContent[i]))
		{
			if (gBNPHeader.appendFile(pathContent[i]))
			{
				printf("Adding %s\n", pathContent[i].c_str());
			}
			else
			{
				printf("Error: cannot open %s\n", pathContent[i].c_str());
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
int main(int argc, char **argv)
{
	NLMISC::CApplicationContext myApplicationContext;

	NLMISC::CCmdArgs args;

	args.addArg("p", "pack", "", "Pack the directory to a BNP file");
	args.addArg("u", "unpack", "", "Unpack the BNP file to a directory");
	args.addArg("l", "list", "", "List the files contained in the BNP file");
	args.addArg("o", "output", "destination", "Output directory or file");
	args.addArg("i", "if", "wildcard", "Add the file if it matches the wilcard (at least one 'if' conditions must be met for a file to be adding)", false);
	args.addArg("n", "ifnot", "wildcard", "Add the file if it doesn't match the wilcard (all the 'ifnot' conditions must be met for a file to be adding)", false);
	args.addArg("", "list-verbose", "", "List files using 'pos size name' format");
	args.addArg("", "extract", "name", "Extract file(s) from BNP into --output");
	args.addAdditionalArg("input", "Input directory or BNP file depending on command");

	if (!args.parse(argc, argv)) return 1;

	if (args.haveArg("p"))
	{
		std::vector<std::string> filters;
			
		// If ?
		filters = args.getLongArg("if");

		for (uint i = 0; i < filters.size(); ++i)
		{
			CWildCard card;
			card.Expression = toLowerAscii(filters[i]);
			card.Not = false;
			WildCards.push_back(card);
		}

		// If not ?
		filters = args.getLongArg("ifnot");

		for (uint i = 0; i < filters.size(); ++i)
		{
			CWildCard card;
			card.Expression = toLowerAscii(filters[i]);
			card.Not = true;
			WildCards.push_back(card);
		}

		// Pack a directory
		std::string srcDirectory = args.getAdditionalArg("input").front();

		if (!CFile::isDirectory(srcDirectory) || !CFile::isExists(srcDirectory))
		{
			nlwarning("Error: %s doesn't exist or is not a directory!", srcDirectory.c_str());
		}

		// Output directory or filename
		if (args.haveArg("o"))
		{
			gBNPHeader.BigFileName = args.getArg("o").front();

			if (CFile::isDirectory(gBNPHeader.BigFileName))
			{
				gBNPHeader.BigFileName = CPath::standardizePath(gBNPHeader.BigFileName) + CFile::getFilename(srcDirectory);
			}
		}
		else
		{
			gBNPHeader.BigFileName = CFile::getFilename(srcDirectory);
		}

		if (CFile::getExtension(gBNPHeader.BigFileName) != "bnp")
			gBNPHeader.BigFileName += ".bnp";

		CFile::deleteFile(gBNPHeader.BigFileName);

		if (!packSubRecurse(srcDirectory)) return 1;

		return gBNPHeader.appendHeader() ? 0:-1;
	}

	if (args.haveArg("u"))
	{
		gBNPHeader.BigFileName = args.getAdditionalArg("input").front();

		std::string dirName;

		// Output directory or filename
		if (args.haveArg("o"))
		{
			dirName = args.getArg("o").front();
		}
		else
		{
			dirName = CFile::getFilenameWithoutExtension(gBNPHeader.BigFileName);
		}

		// Unpack a bnp file
		return gBNPHeader.unpack(dirName) ? 0:-1;
	}

	if (args.haveArg("l"))
	{
		gBNPHeader.BigFileName = args.getAdditionalArg("input").front();

		// Read header of BNP file
		if (!gBNPHeader.readHeader()) return -1;

		for (uint i = 0; i < gBNPHeader.SFiles.size(); ++i)
		{
			printf("%s\n", gBNPHeader.SFiles[i].Name.c_str());
		}

		return 0;
	}

	if (args.haveLongArg("list-verbose"))
	{
		gBNPHeader.BigFileName = args.getAdditionalArg("input").front();

		// Read header of BNP file
		if (!gBNPHeader.readHeader()) return -1;

		for (uint i = 0; i < gBNPHeader.SFiles.size(); ++i)
		{
			printf("%u %u %s\n", gBNPHeader.SFiles[i].Pos, gBNPHeader.SFiles[i].Size, gBNPHeader.SFiles[i].Name.c_str());
		}

		return 0;
	}

	// --extract <name>
	if (args.haveLongArg("extract") && !args.getLongArg("extract").empty())
	{
		std::string bnpName = args.getAdditionalArg("input").front();
		CBigFile::getInstance().add(bnpName, BF_ALWAYS_OPENED);

		// Output directory or filename
		if (!args.haveArg("o") || args.getArg("o").empty())
		{
			nlerror("Output file or directory not set");
		}

		std::string srcName = args.getLongArg("extract").front();
		std::string dstName = args.getArg("o").front();
		if (CFile::fileExists(dstName) && CFile::isDirectory(dstName))
		{
			dstName += "/" + srcName;
		}

		CIFile inFile;
		// bnpName without path
		if (!inFile.open(CFile::getFilename(bnpName) + "@" + srcName))
		{
			nlerror("Unable to open '%s' for reading", inFile.getStreamName().c_str());
		}

		COFile outFile;
		if (!outFile.open(dstName))
		{
			nlerror("Unable to open '%s' for writing", outFile.getStreamName().c_str());
		}

		std::string buf;
		inFile.readAll(buf);
		outFile.serialBuffer((uint8 *)&buf[0], buf.size());

		return 0;
	}

	args.displayHelp();
	return -1;
}
