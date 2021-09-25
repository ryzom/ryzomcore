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
#include "add_on_manager.h"


using namespace NLMISC;
using namespace std;

// ***************************************************************************
CAddOnManager	InterfaceAddOnManager;


// ***************************************************************************
CAddOnManager::CAddOnManager()
{
}


// ***************************************************************************
void	CAddOnManager::addSearchFiles(const std::string &path, const std::string &posFilterList, const std::string &negFilterList, NLMISC::IProgressCallback *progressCallBack)
{
	/*
		NB: this is mainly a copy of CPath::addSearchPath(), with filter added.
	*/

	// *** Build filter list
	std::vector<std::string>	posFilters, negFilters;
	splitString(posFilterList, ";", posFilters);
	splitString(negFilterList, ";", negFilters);
	if(posFilters.empty())
		return;


	// *** AddSearch Path, with Filter features
	// Progress bar
	if (progressCallBack)
	{
		progressCallBack->progress (0);
		progressCallBack->pushCropedValues (0, 0.5f);
	}

	// find all files in the path and subpaths
	string newPath = CPath::standardizePath(path);
	vector<string> filesToProcess;
	CPath::getPathContent (newPath, true, false, true, filesToProcess, progressCallBack);

	// Progress bar
	if (progressCallBack)
	{
		progressCallBack->popCropedValues ();
		progressCallBack->progress (0.5);
		progressCallBack->pushCropedValues (0.5f, 1);
	}

	// add them in the map
	for (uint f = 0; f < filesToProcess.size(); f++)
	{
		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->progress ((float)f/(float)filesToProcess.size());
			progressCallBack->pushCropedValues ((float)f/(float)filesToProcess.size(), (float)(f+1)/(float)filesToProcess.size());
		}

		string filename = CFile::getFilename (filesToProcess[f]);
		string filepath = CFile::getPath (filesToProcess[f]);

		// positive Filter
		bool	ok= false;
		for(uint i=0;i<posFilters.size();i++)
		{
			if(testWildCard(filename, posFilters[i]))
			{
				ok= true;
				break;
			}
		}

		// negative filter
		if(ok)
		{
			for(uint i=0;i<negFilters.size();i++)
			{
				if(testWildCard(filename, negFilters[i]))
				{
					ok= false;
					break;
				}
			}
		}

		// If ok, add it to set and to path
		if(ok)
		{
			_FileSet.insert(filename);
			CPath::addSearchFile (filesToProcess[f], false, "", progressCallBack);
		}

		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
		}
	}

	// Progress bar
	if (progressCallBack)
	{
		progressCallBack->popCropedValues ();
	}
}


// ***************************************************************************
void	CAddOnManager::getFiles(const std::string &filterList, std::vector<std::string> &files)
{
	files.clear();

	// get wildcard list
	std::vector<string>	wildcards;
	splitString(filterList, ";", wildcards);
	if(wildcards.empty())
		return;

	// test all files
	std::set<std::string>::const_iterator	it= _FileSet.begin();
	for(;it!=_FileSet.end();it++)
	{
		// if only one wildcard match, ok
		for(uint i=0;i<wildcards.size();i++)
		{
			if(testWildCard(*it, wildcards[i]))
			{
				files.push_back(*it);
				break;
			}
		}
	}
}


