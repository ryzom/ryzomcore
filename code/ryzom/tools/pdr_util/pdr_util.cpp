// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include <stdio.h>
#include <vector>
#include "game_share/persistent_data.h"

using namespace std;
using namespace NLMISC;

int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext context;

	if (argc == 1)
	{
		printf("Usage : %s [-s<sheet_id_path>] [-x|-b|-t] [-o<outputFileName>] inputFileName\n", argv[0]);
		printf("  -s : must contains the path where the sheet_id.bin can be found\n");
		printf("       You could ignore this param is the sheet_id.bin can be found in the local directory\n");
		printf("  -x : convert a binary pdr to XML format (exclude -b -t)\n");
		printf("  -t : convert a binary pdr to text line format (exclude -b -x)\n");
		printf("  -b : convert a XML or text pdr to binary format (exclude -x -t)\n");
		printf("  -o : output filename\n");
		return -1;
	}

	enum TConvMode
	{
		cm_undefined,
		cm_to_xml,
		cm_to_binary,
		cm_to_txt,
	};

	enum TSourceFormat
	{
		sf_undefined,
		sf_xml,
		sf_txt
	};

	TConvMode	mode = cm_undefined;
	TSourceFormat	sourceFormat = sf_undefined;
	string		sheetIdPath;
	string		fileName;
	vector<string> filenames;
	string		outputFileName;

	vector<string>	args(argv, argv+argc);

	for (uint i=1; i<args.size(); ++i)
	{
		if (args[i].size() >= 2 && args[i][0] == '-')
		{
			string paramValue = args[i].substr(2);

			switch (args[i][1])
			{
			case 's':
				sheetIdPath = paramValue;
				break;
			case 'x':
				mode = cm_to_xml;
				break;
			case 't':
				mode = cm_to_txt;
				break;
			case 'b':
				mode = cm_to_binary;
				break;
			case 'o':
				outputFileName = paramValue;
				break;
			default:
				fprintf(stderr, "Unknown parameter '%s'", args[i].c_str());
				return -1;
			}
		}
		else if(!args[i].empty())
		{
			filenames.push_back(args[i]);
		}
/*
		if (i == args.size()-1)
		{
			// last param, must be the filename
			if (args[i].empty() || args[i][0] == '-')
			{
				fprintf(stderr, "Invalid or missing filename '%s'\n", args[i].c_str());
				return -1;
			}

			fileName = args[i];
		}
		else
		{
			if (args[i].empty() || args[i][0] != '-' || args[i].size() < 2)
			{
				fprintf(stderr, "Invalid param '%s'\n", args[i].c_str());
				return -1;
			}

			string paramValue = args[i].substr(2);

			switch (args[i][1])
			{
			case 's':
				sheetIdPath = paramValue;
				break;
			case 'x':
				mode = cm_to_xml;
				break;
			case 't':
				mode = cm_to_txt;
				break;
			case 'b':
				mode = cm_to_binary;
				break;
			case 'o':
				outputFileName = paramValue;
			default:
				fprintf(stderr, "Unknown parameter '%s'", args[i].c_str());
				return -1;
			}
		}
*/
	}

	// init the sheet id
	if (!sheetIdPath.empty())
	{
		CPath::addSearchPath(sheetIdPath, false, false);
	}

	CSheetId::init(false);

	for(uint f = 0; f < filenames.size(); f++)
	{
		fileName = filenames[f];

		if (!CFile::isExists(fileName))
		{
			fprintf(stderr, "Couldn't find file '%s'", fileName.c_str());
//		return -1;
			continue;
		}

		if (mode == cm_undefined)
		{
			// try to automatically determine conversion mode
			if (fileName.find(".xml") == (fileName.size() - 4))
			{
				printf("Choosing XML->BINARY conversion mode");
				mode = cm_to_binary;
			}
			else if (fileName.find(".txt") == (fileName.size() -4))
			{
				printf("Choosing TXT->BINARY conversion mode");
				mode = cm_to_binary;
			}
			else if (fileName.find(".bin") == (fileName.size() -4))
			{
				printf("Choosing BINARY->XML conversion mode");
				mode = cm_to_xml;
			}
			else
			{
				fprintf(stderr, "Missing conversion mode flag (-x|-b|-t) and can't deduce mode from filename extension");
//				return -1;
				continue;
			}
		}

		// determine source format when concerting to binary
		if (mode == cm_to_binary)
		{
			if (fileName.find(".xml") == (fileName.size() - 4))
			{
				printf("Source file is in XML format");
				sourceFormat = sf_xml;
			}
/*		else if (fileName.find(".txt") == (fileName.size() -4))
		{
			printf("Source file is in TXT format");
			sourceFormat = sf_txt;
		}
*/		else
			{
				fprintf(stderr, "Invalid source format, only support '.xml' files");
//				return -1;
				continue;
			}
		}

		if (outputFileName.empty() || filenames.size() > 1)
		{
			// build a output name
			outputFileName = fileName;
			string inExt;
			string outExt;

			if (mode == cm_to_binary)
			{
				if (sourceFormat == sf_txt)
					inExt = ".txt";
				else
					inExt = ".xml";
				outExt = ".bin";
			}
			else if (mode == cm_to_xml)
			{
				outExt = ".xml";
				inExt = ".bin";
			}
			else if (mode == cm_to_txt)
			{
				outExt = ".txt";
				inExt = ".bin";
			}

			if (outputFileName.find(inExt) == (outputFileName.size()-inExt.size()))
			{
				// remove input ext from output filename
				outputFileName = outputFileName.substr(0, outputFileName.size()-inExt.size());
			}
			// append output extension
			outputFileName += outExt;
		}

		static CPersistentDataRecord	pdr;
		pdr.clear();

		switch(mode)
		{
		case cm_to_binary:
			if (sourceFormat == sf_txt)
			{
				printf("Converting from txt to bin is currently unpossible ! use xml format");
//				return -1;
				continue;
//			printf("Converting '%s' (TXT) to '%s' (BINARY)\n", fileName.c_str(), outputFileName.c_str() );
			}
			else
				printf("Converting '%s' (XML) to '%s' (BINARY)\n", fileName.c_str(), outputFileName.c_str() );
			if (!pdr.readFromTxtFile(fileName.c_str()))
				goto failureRead;
			if (!pdr.writeToBinFile(outputFileName.c_str()))
				goto failureWrite;
			break;

		case cm_to_xml:
			printf("Converting '%s' (BINARY) to '%s' (XML)\n", fileName.c_str(), outputFileName.c_str() );
			if (!pdr.readFromBinFile(fileName.c_str()))
				goto failureRead;
			if (!pdr.writeToTxtFile(outputFileName.c_str(), CPersistentDataRecord::XML_STRING))
				goto failureWrite;
			break;

		case cm_to_txt:
			printf("Converting '%s' (BINARY) to '%s' (TXT)\n", fileName.c_str(), outputFileName.c_str() );
			if (!pdr.readFromBinFile(fileName.c_str()))
				goto failureRead;
			if (!pdr.writeToTxtFile(outputFileName.c_str(), CPersistentDataRecord::LINES_STRING))
				goto failureWrite;
			break;
		default:
			break;
		}

//		return 0;
		continue;

failureRead:
		fprintf(stderr, "Error while reading '%s', conversion aborted", fileName.c_str());
//		return -1;
		continue;
failureWrite:
		fprintf(stderr, "Error while writing '%s', conversion aborted", outputFileName.c_str());
//		return -1;
		continue;
	}

}
