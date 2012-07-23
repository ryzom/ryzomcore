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

#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/sstring.h>
#include <nel/misc/path.h>
#include <nel/misc/algo.h>

#include <nel/misc/config_file.h>
#include <nel/misc/bitmap.h>

#include <iostream>

using namespace NLMISC;
using namespace std;

CSString REP_SRC; //= "R:\\code\\ryzom\\src_v2\\client\\";
CSString REP_GUI; //= "R:\\code\\ryzom\\data\\gamedev\\interfaces_v3\\";
CSString REP_ITEM; //= "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\Game_elem\\items\\";
CSString REP_MISSION; //= "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\Game_elem\\mission\\";
CSString REP_SITEM; //= "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\game_element\\sitem\\";
CSString REP_SBRICK; //= "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\game_element\\sbrick\\";
CSString REP_DFN; //= "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\DFN\\";
CSString REP_FACTION;// = "R:\\code\\ryzom\\data_leveldesign\\leveldesign\\World\\factions\\";

typedef vector<string> StringVector;
StringVector listeIcones;
map<CSString, StringVector> citations;

bool addUsedIcons = true;
bool addFreeIcons = true;


// Fill listeIcones from source directories
void findIcons(CConfigFile::CVar *var)
{
	nlassert(var != NULL);

	// clear vector
	listeIcones.clear();
	vector<string> files;

	// Scan each directory
	for (uint i=0 ; i<var->size() ; i++)
	{
		// clear files list
		files.clear();

		// get files list
		CPath::getPathContent(CPath::standardizePath(var->asString(i)), false, false, true, files);
		
		// Convert each file
		for (uint j=0 ; j<files.size() ; j++)
		{
			// get filename
			string iconName = CFile::getFilename(files[j]);

			// store icon's name
			listeIcones.push_back(iconName);
			citations[iconName].push_back(iconName);
		}
	}
}

string remap2Jpg(const string &s)
{
	// change extension from TGA to JPG
	string jpg(s);
	string::size_type n = jpg.find(".tga");
	nlassert(n != string::npos);
	jpg.erase(n);
	jpg += ".jpg";

	return jpg;
}

void writeString(COFile &f, const string &s)
{
	f.serialBuffer((uint8*)s.c_str(), (uint)s.size());
}

void writeHTMLline(COFile &f, const string &icon, const StringVector files, uint sizeLimit)
{
	// discard if needed
	if ((files.size() == 1 && !addFreeIcons) || (files.size() > 1 && !addUsedIcons))
		return;

	// begin line
	writeString(f, "<tr>");

	// write icon as jpg
	writeString(f, "<th><img src=\"");
	writeString(f, "images\\");
	writeString(f, remap2Jpg(files[0]));
	writeString(f, "\"></th>");

	// fix limit based on configuration file
	uint n;
	if (sizeLimit == 0)
		n = (uint)files.size();
	else
		n = sizeLimit > files.size() ? (uint)files.size() : sizeLimit;

	// write each file using this icon
	for (uint i=0 ; i<n ; i++)
			writeString(f, "<td>" + files[i] + "</td>");

	// end line
	writeString(f, "</tr>\n");
}

void generateHTML(const string &name, uint sizeLimit)
{
	COFile f;
	nlassert(f.open(name, false, true));

	// write header
	writeString(f, "<html>\n<head>\n<title>Nevrax - Ryzom icons</title>\n</title>\n\n<body><table border=1>\n");

	// write lines
	for (map<CSString, StringVector>::iterator it = citations.begin(); it != citations.end(); ++it)
	{
		string str = it->second[0];
		writeHTMLline(f, str, it->second, sizeLimit);
	}

	// write end
	writeString(f, "</table></body>\n</html>");

	f.close();
}

void tga2Jpg(const string &tga, const string &jpg)
{
	// read TGA
	CIFile fTga;
	CBitmap im;
	nlassert(fTga.open(tga));
	nlassert(im.load(fTga));
	fTga.close();

	// write JPG
	COFile fJpg;
	nlassert(fJpg.open(jpg));
	nlassert(im.writeJPG(fJpg));
	fJpg.close();
}

void convertImages(CConfigFile::CVar *var)
{
	nlassert(var != NULL);

	// store current path
	string curPath = CPath::standardizePath(CPath::getCurrentPath());

	// Create 'images' directory in the root folder if not present
	if (!CFile::isExists("images"))
		CFile::createDirectory("images");

	// Scan each directory
	for (uint i=0 ; i<var->size() ; i++)
	{
		string path = CPath::standardizePath(var->asString(i));
		vector<string> files;
		CPath::getPathContent(path, false, false, true, files);
		
		// Convert each file
		for (uint j=0 ; j<files.size() ; j++)
		{
			// get filenames
			string file = CFile::getFilename(files[j]);
			string src = path + file;
			string dst = remap2Jpg(curPath + "images/" + file);

			cout << "Copying file (" << i+1 << "/" << var->size() << " - " << j+1 << " / " << files.size() << ") : " << file << " ... ";

			// Convert the file
			if (!CFile::isExists(dst))
			{
				tga2Jpg(src, dst);
				cout << " OK\n";
			}
			else
			{
				cout << "skipped\n";
			}
		}
	}
}

bool endsWith( const CSString& s, const CSString& substring )
{
	return ( s.right( (uint)substring.size() ) == substring );
}

void ProcessDirectory( const CSString& dir, const StringVector& extensions )
{
	CSString data;
	int nbResults = 0;
	int sixieme;
	vector<string> files;

	
	printf( "%s\n", dir.c_str() );
	CPath::getPathContent ( dir.c_str(), true, false, true, files );

	sixieme = (int)files.size() / 6;

	printf( "%u files are processed", (uint) files.size() );

	for (uint32 i=0; i<files.size(); ++i)
	{
		bool extOK = false;
		uint numExt = 0;
		
		while ( ( !extOK ) && ( numExt < extensions.size() ) )
		{
			extOK = endsWith( files[i], extensions[numExt] );
			numExt++;
		}
		
		if ( extOK )
		{
			data.readFromFile( files[i] );

			// Don't parse LOG
			string::size_type n = data.find("<LOG>");
			if (n != CSString::npos)
				data.erase(n);

			data = data.toLower();

			for ( uint it=0; it<listeIcones.size(); it++ )
			{
				if ( data.contains( CSString(listeIcones[it]).toLower().c_str() ) )
				{
					string fileName = CFile::getFilename( files[i] );
					citations[ listeIcones[it] ].push_back( fileName );
					nbResults++;
				}
			}
		}

		if ( i%sixieme == 0 )
			printf( "." );
	}
	
	printf( " %d results found\n\n", nbResults );
}

int main()
{
	StringVector extensions;

	new CApplicationContext();

	CSString REP_SRC, REP_GUI, REP_LEVEL_DESIGN;
	int searchSrcClient, searchGui, searchLevelDesign;
	uint sizeLimit = 5;

	CConfigFile cf;
	CConfigFile::CVar *var;
	cf.load("icon_search.cfg");

	var = cf.getVarPtr("SearchClientSrc");
	nlassert(var);
	searchSrcClient = var->asInt();

	var = cf.getVarPtr("ClientSrcPath");
	nlassert(var);
	REP_SRC = var->asString();

	var = cf.getVarPtr("SearchGUI");
	nlassert(var);
	searchGui = var->asInt();

	var = cf.getVarPtr("GUIPath");
	nlassert(var);
	REP_GUI = var->asString();

	var = cf.getVarPtr("SearchLevelDesign");
	nlassert(var);
	searchLevelDesign = var->asInt();

	var = cf.getVarPtr("LevelDesignPath");
	nlassert(var);
	REP_LEVEL_DESIGN = var->asString();

	var = cf.getVarPtr("IconPath");
	nlassert(var);
	convertImages(var);
	findIcons(var);

	var = cf.getVarPtr("SizeLimit");
	if (var)
		sizeLimit = (uint)var->asInt();

	var = cf.getVarPtr("AddUsedIcons");
	if (var)
		addUsedIcons = var->asBool();

	var = cf.getVarPtr("AddFreeIcons");
	if (var)
		addFreeIcons = var->asBool();

	REP_ITEM = REP_LEVEL_DESIGN + "leveldesign\\Game_elem\\items\\";
	REP_MISSION = REP_LEVEL_DESIGN + "leveldesign\\Game_elem\\mission\\";
	REP_SITEM = REP_LEVEL_DESIGN + "leveldesign\\game_element\\sitem\\";
	REP_SBRICK = REP_LEVEL_DESIGN + "leveldesign\\game_element\\sbrick\\";
	REP_DFN = REP_LEVEL_DESIGN + "leveldesign\\DFN\\";
	REP_FACTION = REP_LEVEL_DESIGN + "leveldesign\\World\\factions\\";

	var = cf.getVarPtr("Wildcard");
	if (var)
	for (uint i=0 ; i<var->size() ; i++)
	for (uint it=0; it<listeIcones.size(); it++)
	{
		string wild = var->asString(i);
		if (testWildCard(listeIcones[it], wild))
		{
			citations[listeIcones[it]].push_back("Matched with: " + wild);
		}
	}

	printf( "\nProcessing Directories\n----------------------\n\n" );


	// CLIENT SRC FILES
	if ( searchSrcClient )
	{
		extensions.push_back( "cpp" );
		extensions.push_back( "h" );
		ProcessDirectory( REP_SRC, extensions );
	}
	
	// GUI FILES
	if ( searchGui )
	{
		extensions.clear();
		extensions.push_back( "xml" );
		ProcessDirectory( REP_GUI, extensions );
	}
	
	if ( searchLevelDesign )
	{
		// ITEM FILES
		extensions.clear();
		extensions.push_back( "item" );
		ProcessDirectory( REP_ITEM, extensions );

		// MISSION FILES
		extensions.clear();
		extensions.push_back( "mission_icon" );
		ProcessDirectory( REP_MISSION, extensions );
		
		// SITEM FILES
		extensions.clear();
		extensions.push_back( "sitem" );
		ProcessDirectory( REP_SITEM, extensions );

		// SBRICK FILES
		extensions.clear();
		extensions.push_back( "sbrick" );
		ProcessDirectory( REP_SBRICK, extensions );

		// DFN FILES
		extensions.clear();
		extensions.push_back( "dfn" );
		ProcessDirectory( REP_DFN, extensions );

		// FACTION FILES
		extensions.clear();
		extensions.push_back( "faction" );
		ProcessDirectory( REP_FACTION, extensions );
	}

	generateHTML("icons.html", sizeLimit);
}
