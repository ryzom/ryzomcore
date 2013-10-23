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
#include "nel/misc/debug.h"
#include "nel/misc/file.h"

#include "game_share/huffman.h"

#include <string>
#include <map>
#include <fstream>

using namespace std;
using namespace NLMISC;


/**
 * CStringInfos
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct CStringInfos
{
	/// string id
	string Id;

	/// string
	string Str;

	/// occurence of the string
	uint32 Occurence;

	/**
	 * Default constructor
	 */
	CStringInfos() : Occurence(1) { }

};



//-----------------------------------------------
//	main
//
//-----------------------------------------------
sint main( sint argc, char ** argv )
{
	uint currentVersion = 1;

	map<string,CStringInfos> base;

	if( argc < 3 )
	{
		printf("Create a file associating a string id with a string and its Huffman code\n\n");
		printf("OCC2HUFF <string file> [<string file> ...] <occ file>\n");
		return 1;
	}

	// open the id string association file(s)
	sint i;
	for( i = 1; i < argc - 1; i++ )
	{
		printf("Reading string association file '%s'...\n",argv[i]);

		ifstream input1(argv[i], ios::in);
		if( !input1.is_open() )
		{
			nlwarning("can't open the file %s",argv[i]);
			return 1;
		}
		
		// read the tokens and create the string infos
		while( !input1.eof() )
		{
			// read a line
			string line;
			getline(input1,line,'\n');

			// test the line ( there must be at least 2 '"',remove comments if exist )
			sint32 idx = line.find_first_of("#");
			bool hasComments = false;
			if( idx != -1 )
			{
				line = line.substr(0,idx);
				hasComments = true;
			}
			if( line.size() == 0 )
			{
				continue;
			}
			if( line.find_first_of("\"") == -1 )
			{
				if( !hasComments )
				{
					nlwarning("Missing string value in the string '%s'",line.c_str());
					return 1;
				}
			}
			if( line.find_first_of("\"") == line.find_last_of("\"") )
			{
				if( !hasComments )
				{
					nlwarning("Missing a delimiter \" in the string '%s'",line.c_str());
					return 1;
				}
			}
			
			// extract string id and string
			idx = line.find_first_of(" \t");
			if( idx != -1 )
			{
				CStringInfos si;	
				si.Id = line.substr(0,idx);

				sint32 startIdx = line.find_first_of("\"");
				sint32 endIdx = line.find_last_of("\"");
				si.Str = line.substr(startIdx+1,endIdx-startIdx-1);

				// add string infos
				map<string,CStringInfos>::iterator itStr = base.find( si.Id );
				if( itStr == base.end() )
				{
					base.insert( make_pair(si.Id,si) );
				}
				else
				{
					nlwarning("The string %s already exists !",si.Id.c_str());
				}
			}
		}
		input1.close();
	}

	// open the id occurence association file
	string occfilename = argv[argc-1];
	printf("Reading occurence file '%s'...\n",occfilename.c_str());
	ifstream input2(occfilename.c_str(), ios::in);
	if( !input2.is_open() )
	{
		nlwarning("Can't open the file %s, set all occurences to 1",argv[argc-1]);
	}
	else
	{
		// read the tokens and update the string infos with occurences
		while( !input2.eof() )
		{
			// read a line
			string line;
			getline(input2,line,'\n');
			
			// test the line
			sint32 idx = line.find_first_of("#");
			if( idx != string::npos )
			{
				line = line.substr(0,idx);
			}
			if( line.size() == 0 )
			{
				continue;
			}
			char * buffer = new char[line.size()+1];
			strcpy(buffer,line.c_str()); 

			// extract string id and occurence
			char * token;
			string stoken;
			token = strtok(buffer," \t");
			if( token != NULL )
			{
				stoken = string( token );
				map<string,CStringInfos>::iterator itStr = base.find( stoken );
				if( itStr != base.end() )
				{
					token = strtok(NULL," \t");
					(*itStr).second.Occurence = atoi( token );
					if( (*itStr).second.Occurence == 0 )
					{
						nlwarning("The occurence of string '%s' is 0 (problem with occurence ?: '%s'), set it to 1",(*itStr).second.Str.c_str(),token);
						(*itStr).second.Occurence = 1;
					}
				}
				else
				{
					nlwarning("The string '%s' is in the .occ but in the txt files!",token);
				}
			}
			delete buffer;
		}
		input2.close();

		ofstream output2(occfilename.c_str(), ios::app);
		if (output2.is_open())
		{
			map<string,CStringInfos>::iterator itBase;
			for( itBase = base.begin(); itBase != base.end(); ++itBase )
			{
				if((*itBase).second.Occurence == 0)
				{
					output2 << (*itBase).first << " 1" <<endl;
					(*itBase).second.Occurence = 1;
				}
			}
			output2.close ();
		}
	}

	// build the Huffman tree
	printf("Building Huffman tree...\n");
	CHuffman huff;
	map<string,CStringInfos>::iterator itBase;
	for( itBase = base.begin(); itBase != base.end(); ++itBase )
	{
		huff.add( (*itBase).first,(*itBase).second.Occurence );
	}
	huff.build();


	// open the output file
	string outputFileName = "chat_static.cdb";
	COFile output( outputFileName );

	// save id|string|occurence
	printf("Writing binary file '%s'...\n",outputFileName.c_str());
	output.serialVersion(currentVersion);
	uint32 count = base.size();
	output.serial( count );
	vector<bool> code;
	for( itBase = base.begin(); itBase != base.end(); ++itBase )
	{
		if( (*itBase).second.Occurence > 0 )
		{
			huff.getCode( (*itBase).first, code );
			output.serial( (*itBase).second.Id );
			output.serial( (*itBase).second.Str );
			output.serial( (*itBase).second.Occurence );
			output.serialCont( code );
		}
	}

	// TEST
	/*printf("Writing debug text file...\n");
	FILE * outputTest = fopen("chat_static_base_test.log","wt");
	for( itBase = base.begin(); itBase != base.end(); ++itBase )
	{
		fprintf(outputTest,"id: %s  str: %s  occ: %d\n",(*itBase).second.Id.c_str(),(*itBase).second.Str.c_str(), (*itBase).second.Occurence );
	}
	fclose(outputTest);*/

	printf("Process complete.\n");

	return 0;	
}
