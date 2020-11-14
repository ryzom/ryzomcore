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


#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
void	buildRaceAnimNames(std::vector<string> &raceAnimNames, const std::string &animName)
{
	if(animName.compare(0, 3, "fy_")!=0)
	{
		nlwarning("ERROR: all .anim must begin with fy_");
		exit(-1);
	}

	raceAnimNames.resize(4);
	raceAnimNames[0]= animName;
	raceAnimNames[1]= animName;
	raceAnimNames[2]= animName;
	raceAnimNames[3]= animName;
	raceAnimNames[0].replace(0, 3, "fy_");
	raceAnimNames[1].replace(0, 3, "ma_");
	raceAnimNames[2].replace(0, 3, "tr_");
	raceAnimNames[3].replace(0, 3, "zo_");

	// Force ""
	for(uint i=0;i<raceAnimNames.size();i++)
	{
		raceAnimNames[i]= string("\"") + raceAnimNames[i] + string("\"");
	}
}

// ***************************************************************************
string::size_type	findAnimName(const string &lineLwr, const std::vector<string> &raceAnimNames)
{
	// line Must contains Name="filename", else CAN BE A LOAD CHAR ANIMATION!!!!
	if(lineLwr.find("name=\"filename\"")==string::npos)
		return -1;

	// in the animset, the original file can be a "tr_" ... Not necessarily a "fy_"
	for(uint i=0;i<raceAnimNames.size();i++)
	{
		string::size_type	pos= lineLwr.find(raceAnimNames[i]);
		if(pos!=string::npos)
			return pos;
	}
	return string::npos;
}

// ***************************************************************************
void	appendRaceAnim(vector<string> &animSetText, uint startBlock, const vector<string> &copyText, uint nameLineInBlock, uint nameIndexInLine, const string &raceAnimName)
{
	// add empty space
	animSetText.insert(animSetText.begin()+startBlock, copyText.size(), string());

	// Fill line by line
	for(uint i=0;i<copyText.size();i++)
	{
		string	line= copyText[i];

		// If this is the line we have to change the anim name
		if(i==nameLineInBlock)
			line.replace(nameIndexInLine, raceAnimName.size(), raceAnimName);

		// If this is the line we have to specify the race node
		if(i==copyText.size()-2)
		{
			string	peopleEnum;
			if(raceAnimName.compare(1,3,"fy_")==0)
				peopleEnum= "Fyros";
			else if(raceAnimName.compare(1,3,"ma_")==0)
				peopleEnum= "Matis";
			else if(raceAnimName.compare(1,3,"tr_")==0)
				peopleEnum= "Tryker";
			else if(raceAnimName.compare(1,3,"zo_")==0)
				peopleEnum= "Zorai";
			else
			{
				nlwarning("ERROR: generated anim can be only fy_, ma_, zo_, tr_. Ask a coder");
				exit(-1);
			}
			line=toString("          <ATOM Name=\"Race Restriction\" Value=\"%s\"/>", peopleEnum.c_str());
		}

		// copy
		animSetText[startBlock+i]= line;
	}
}

// ***************************************************************************
void	makeAnimByRace(const std::string &animSetFile, const std::vector<string> &animList)
{
	// *** Read the animset file.
	CIFile	iFile;
	iFile.open(animSetFile, true);
	// Read all text
	static vector<string>	animSetText;
	animSetText.clear();
	while(!iFile.eof())
	{
		char	tmp[50000];
		iFile.getline(tmp, 50000);
		animSetText.push_back(tmp);
	}
	iFile.close();


	bool	someChangeDone= false;

	// *** For each possible anim
	for(uint i=0;i<animList.size();i++)
	{
		// get the possible anim file name (lowered)
		static vector<string>	raceAnimNames;
		raceAnimNames.clear();
		buildRaceAnimNames(raceAnimNames, toLower(CFile::getFilename(animList[i])));

		// For each line of the animSet
		uint	lastStructLine= 0;
		bool	raceRestrictionFound= false;
		for(uint j=0;j<animSetText.size();)
		{
			string	line= animSetText[j];
			string	lineLwr= toLower(line);

			// Find <LOG> TAg? => stop
			if(line.find("<LOG>")!=string::npos)
				break;

			// Find a STRUCT start?
			if(line.find("<STRUCT>")!=string::npos)
			{
				lastStructLine= j;
				raceRestrictionFound= false;
			}
			
			// Find a RaceRestriction?
			if( line.find("Name=\"Race Restriction\"")!=string::npos )
				raceRestrictionFound= true;

			// Find the anim name?
			string::size_type	nameIndexInLine= findAnimName(lineLwr, raceAnimNames);
			if(nameIndexInLine!=string::npos)
			{
				// Find the enclosing struct
				nlassert(lastStructLine!=0);
				uint	startBlock= lastStructLine;
				uint	nameLineInBlock= j-startBlock;
				uint	endBlock= 0;
				for(uint k=j+1;k<animSetText.size();k++)
				{
					string	line= animSetText[k];

					// Find a RaceRestriction?
					if( line.find("Name=\"Race Restriction\"")!=string::npos )
						raceRestrictionFound= true;

					// end of block?
					if(line.find("</STRUCT>")!=string::npos)
					{
						// endBlock is exclusive 
						endBlock= k+1;
						break;
					}
				}

				// if not found, abort
				if(endBlock==0)
					break;

				// if a raceRestriction has been found, no op (already done)
				if(raceRestrictionFound)
				{
					j= endBlock;
				}
				else
				{
					// LOG
					InfoLog->displayRawNL("%s: Specifying %s by race", 
						CFile::getFilename(animSetFile).c_str(), 
						CFile::getFilename(animList[i]).c_str());

					// *** Start a copy paste ^^
					// Copy
					static vector<string>	copyText;
					copyText.clear();
					for(uint k=startBlock;k<endBlock;k++)
					{
						// add an empty line before </STRUCT>, for race selection node (filled later)
						if(k==endBlock-1)
							copyText.push_back(string());
						copyText.push_back(animSetText[k]);
					}

					// erase this part
					animSetText.erase(animSetText.begin()+startBlock, animSetText.begin()+endBlock);
					uint	nextBlock= startBlock;

					// Append for each race
					for(uint k=0;k<raceAnimNames.size();k++)
					{
						appendRaceAnim(animSetText, nextBlock, copyText, nameLineInBlock, (uint)nameIndexInLine, raceAnimNames[k]);
						// nextBlock is then shifted
						nextBlock+= (uint)copyText.size();
					}

					someChangeDone= true;

					// *** then let j point to next block
					j= nextBlock;
				}
			}
			else
			{
				j++;
			}
		}
	}

	// *** Write the animset file.
	if(someChangeDone)
	{
		COFile	oFile;
		oFile.open(animSetFile, false, true);
		// Write all text
		for(uint i=0;i<animSetText.size();i++)
		{
			string	str= animSetText[i];
			str+= "\n";
			oFile.serialBuffer((uint8*)str.c_str(), (uint)str.size());
		}
	}
}



// ***************************************************************************
int usage()
{
	printf("Usage: make_anim_by_race  new_anim_dir  animset_dir");
	return -1;
}


// ***************************************************************************
int main(int argc, char *argv[])
{
	NLMISC::createDebug();

	// make_anim_by_race new_anim_dir animset_dir
	if(argc!=3)
		return usage();
	string animDir= argv[1];
	string animSetDir= argv[2];

	// Get the list of .anim to make by race
	vector<string>	files;
	CPath::getPathContent(animDir, false, false, true, files);
	// Filter .anim
	vector<string>	animList;
	InfoLog->displayRawNL("");
	InfoLog->displayRawNL("********************");
	InfoLog->displayRawNL("**** .anim list ****");
	InfoLog->displayRawNL("********************");
	for(uint i=0;i<files.size();i++)
	{
		if(testWildCard(files[i], "*.anim"))
		{
			animList.push_back(files[i]);
			InfoLog->displayRawNL(animList.back().c_str());
		}

	}

	// Get the list of .animset to make by race
	files.clear();
	CPath::getPathContent(animSetDir, true, false, true, files);
	vector<string>	animSetList;
	InfoLog->displayRawNL("");
	InfoLog->displayRawNL("*****************************");
	InfoLog->displayRawNL("**** .animation_set list ****");
	InfoLog->displayRawNL("*****************************");
	for(uint i=0;i<files.size();i++)
	{
		if(testWildCard(files[i], "*.animation_set"))
		{
			animSetList.push_back(files[i]);
			InfoLog->displayRawNL(animSetList.back().c_str());
		}
	}
	
	InfoLog->displayRawNL("");
	InfoLog->displayRawNL("**************************");
	InfoLog->displayRawNL("**** Starting Process ****");
	InfoLog->displayRawNL("**************************");
	// For each animset, test if can replace some anim
	for(uint i=0;i<animSetList.size();i++)
	{
		makeAnimByRace(animSetList[i], animList);
	}

	return 0;
}

