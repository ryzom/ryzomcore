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



#include "stdpch.h"
#include "primitive_cfg.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

using namespace NLMISC;
using namespace std;

//	std::vector<std::string>			CPrimitiveCfg::_AllPrimitives;
std::vector<std::string>			CPrimitiveCfg::_MapNames;
std::map<std::string, std::vector<std::string> >	CPrimitiveCfg::_Maps;
std::map<std::string, std::vector<std::string> >	CPrimitiveCfg::_ContinentFiles;

void CPrimitiveCfg::addPrimitive(std::vector<std::string>	&vectorlist, const	std::string &str)
{
	for (uint32 i=0;i<vectorlist.size();i++)
		if (vectorlist[i]==str)
			return;

	vectorlist.push_back(str);
}


std::string CPrimitiveCfg::getContinentNameOf(const std::string &fileName)
{
	CPrimitiveCfg::readPrimitiveCfg();
	string name = CFile::getFilename(fileName);

	std::map<std::string, std::vector<std::string> >::iterator first(_ContinentFiles.begin()), last(_ContinentFiles.end());
	for	(; first != last; ++first)
		if	(find(first->second.begin(),first->second.end(), name)	!= first->second.end())
			return	first->first;
		
	return std::string();
}

// dumb routine to simplify repetitive text parsing code
inline bool isWhiteSpace(char c)
{
	return (c==' ' || c=='\t');
}

// -- stringToWordAndTail() --
// The following routine splits a text string into a keyword and a tail.
// A white space is used as separator between keyword and tail
// All leading and trailing ' ' and '\t' round keyword and tail characters are stripped
// If no keyword is found routine retuns false (keyword and tail retain previous content)
inline bool stringToWordAndTail(const std::string &input,std::string &word, std::string &tail)
{
	uint i=0, j;

	// skip white space
	while (i<input.size() && isWhiteSpace(input[i])) ++i;		// i points to start of word

	// look for the end of the word
	for (j=i;j<input.size() && !isWhiteSpace(input[j]);) ++j;	// j points to next character after word
	
	// if no word found then give up
	if (j==i) return false;

	// copy out the word 
	word=input.substr(i,j-i);

	// find the end of the tail text
	for (i=(uint)input.size();i>j && isWhiteSpace(input[i-1]);) --i;	// i points to character after end of tail text

	// find start of tail text
	do { ++j; } while(j<i && isWhiteSpace(input[j]));			// j points to start of tail text

	// copy out the tail (or clear if no tail found in input)
	if (j<i)
		tail=input.substr(j,i-j);
	else
		tail.clear();

	return true;
}

void CPrimitiveCfg::readPrimitiveCfg(bool forceReload)
{
	if	(	!forceReload
		&&	!_MapNames.empty())
		return;

//	_AllPrimitives.clear();
	_MapNames.clear();
	_Maps.clear();

	std::string filename = CPath::lookup("primitives.cfg");
	if (filename.empty())
	{
		nlwarning("Can't find the primitive configuration file 'primitives.cfg'");
		return;
	}

	try
	{
		CConfigFile	cfg;
		cfg.load(filename.c_str());

		CConfigFile::CVar var = cfg.getVar("PrimitiveFiles");

		std::vector<std::string> CurrentMapNames;
		set<string>	mapNames;

		std::vector<std::string>	*_CurrentContinentFiles=NULL;

		for (uint32 i=0;i<var.size();++i)
		{
//			uint32 j,k;

			// get the next string from the config file entry and make sure its not empty
			const	std::string s=var.asString(i);

			std::string keyword;
			std::string tail;
			{
				if (s.empty())
					continue;
				stringToWordAndTail(s, keyword, tail);
			}
			
			
//			// skip opening white space	and make sure there's not just white space
//			for (j=0;j<s.size() && (s[j]==' ' || s[j]=='\t');++j);
//			if (j>=s.size())
//				continue;
//			// separate the first word (we need it to know what to do next)
//			for (k=j;k<s.size() && s[k]!=' ' && s[k]!='\t';++k);
//			std::string keyword=s.substr(j,k-j);
//
//			// separate out the tail, pruning trailing blanks
//			// ignore leading blanks
//			for (j=k;j<s.size() && (s[j]==' ' || s[j]=='\t');++j);
//			// ignore trailing spaces
//			for (k=s.size()-1;s[k]==' '||s[k]=='\t';--k);
//			// if we found some non-blank text take a copy in a 'tail' variable 
//			std::string tail;
//			if (k>=j) tail=s.substr(j,k-j+1);


			// do something depending on the keyword found earlier
			if (nlstricmp(keyword,"MAPEND")==0)
			{
				// remove the last map entry
				CurrentMapNames.pop_back();
			}
			else if (nlstricmp(keyword,"CONTINENT")==0)
			{
				_CurrentContinentFiles=&_ContinentFiles[tail];
			}
			else if (nlstricmp(keyword,"MAP")==0)
			{
				// store the map name for use later
				CurrentMapNames.push_back(tail);
				mapNames.insert(tail);
				_Maps.insert(make_pair(tail, vector<string>()));
				if (_CurrentContinentFiles)
					addPrimitive(*_CurrentContinentFiles, tail);	//	bad but not too ..
			}
			else if (nlstricmp(keyword,"FILE")==0)
			{
				// if our file name is > 0 characters long then add it to the vector
//				if (j<=k)
				{
					string filename = tail;	//s.substr(j,k-j+1);
					if (CFile::getExtension(filename).empty())
						filename+=".primitive";

					for	(uint32 mapInd=0;mapInd<CurrentMapNames.size();mapInd++)
						addPrimitive(_Maps[CurrentMapNames[mapInd]], filename);

					if	(_CurrentContinentFiles)
						addPrimitive(*_CurrentContinentFiles, filename);
//					else
//						nlwarning("Not Continents specified for %s",filename.c_str());
				}

			}
			else if (nlstricmp(keyword,"INCLUDE")==0)
			{
				// if our file name is > 0 characters long then add it to the vector
//				if (j<=k)
				{
					string includeName = tail;	//s.substr(j,k-j+1);

					if	(_Maps.find(includeName)==_Maps.end())
					{
						nlwarning("PrimitiveCfg: Include %s failed, not defined.",includeName.c_str());
					}
					else
					{
						std::vector<std::string>	&vectorlist=_Maps[includeName];
						for	(uint32 primInd=0;primInd<vectorlist.size();primInd++)
						{
							const	string	&primitiveFileName=vectorlist[primInd];

							for	(uint32 mapInd=0;mapInd<CurrentMapNames.size();mapInd++)
								addPrimitive(_Maps[CurrentMapNames[mapInd]], primitiveFileName);
							if	(_CurrentContinentFiles)
								addPrimitive(*_CurrentContinentFiles, primitiveFileName);
							else
								nlwarning("Not Continents specified for %s",includeName.c_str());
						}

					}

				}

			}
			else
				nlwarning("Unknown keyword in PrimitiveFiles at line: '%s'",s.c_str());
		}
		// fill the maps names vector
		_MapNames.insert(_MapNames.begin(), mapNames.begin(), mapNames.end());
	}
	catch(...)
	{
		nlwarning("Error reading or parsing the primitive configuration file '%s'", filename.c_str());
	}
}

