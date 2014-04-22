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
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"


using namespace std;
using namespace NLMISC;

int main(int argc, char *argv[])
{
	new NLMISC::CApplicationContext;

	CConfigFile	cf;

	uint32	firstFreeIndex = 0;

	// load the config file
	cf.load("primitive_id_assignator.cfg");

	// get the list of extension
	CConfigFile::CVar &ext = cf.getVar("Extension");

	// get the list of filter
	CConfigFile::CVar &filters = cf.getVar("Filters");

	// Add the search paths
	CConfigFile::CVar &sp = cf.getVar("SearchPath");

	for (uint i=0; i<sp.size(); ++i)
	{
		CPath::addSearchPath(sp.asString(i), true, false);
	}

	// get the index file.
	CConfigFile				indexFile;
	map<string, uint32>		indexMap;
	CConfigFile::CVar &vif = cf.getVar("IndexFile");
	if (!CFile::isExists(vif.asString()))
	{
		// build a default config file
		string fileName = vif.asString();
		FILE *fp = fopen(fileName.c_str(), "wt");
		if (fp == NULL)
		{
			nlwarning("Can't open file '%s' for writing",
				fileName.c_str());
			return -1;
		}
		fprintf(fp, "Files = {};\n");
		fclose(fp);

	}
	indexFile.load(vif.asString());

	// parse the index file
	CConfigFile::CVar &fl = indexFile.getVar("Files");
	for (uint i=0; i<(fl.size())/2; i++)
	{
		string fileName;
		uint32 index;

		fileName = fl.asString(i*2);
		index = fl.asInt(i*2+1);

		if (indexMap.find(fileName) != indexMap.end())
		{
			nlwarning("Association for file '%s' already exist as index %u, new association with index %u will be lost",
				fileName.c_str(),
				index,
				indexMap[fileName]);
		}
		else
		{
			indexMap[fileName] = index;

			if (index >= firstFreeIndex)
				firstFreeIndex = index + 1;
		}
	}

	// scan the search path
	for (uint i=0; i<ext.size(); ++i)
	{
		vector<string> files;
		CPath::getFileList(ext.asString(i), files);

		for (uint i=0; i<files.size(); ++i)
		{
			string fileName(CFile::getFilename(files[i]));
			if (indexMap.find(fileName) == indexMap.end())
			{
				// check the first on full path
				bool filtered = false;
				string path = CPath::lookup(fileName);
				for (uint i=0; i<filters.size(); ++i)
				{
					if (path.find(filters.asString(i)) != string::npos)
					{
						// skip this file
						nlinfo("File '%s' filtered out by rule '%s' (in '%s')",
							fileName.c_str(),
							filters.asString(i).c_str(),
							path.c_str());
						filtered = true;
						continue;
					}
				}
				if (!filtered)
				{
					// this file is not index, add it! 
					indexMap[fileName] = firstFreeIndex++;

					nlinfo("Indexing file '%s' with index %u",
						fileName.c_str(),
						indexMap[fileName]);
				}
			}
		}
	}


	// rebuild the config file

	indexFile.clearVars();
	map<string, uint32>::iterator first(indexMap.begin()), last(indexMap.end());
	for (uint i=0; first != last; ++first, ++i)
	{
		if (i == 0)
			fl.forceAsString(first->first);
		else
			fl.setAsString(first->first, i*2);
		fl.setAsString(toString("%u", first->second), i*2+1);
	}

	nlinfo("Writing index with %u files indexed",
		fl.size());

	indexFile.save();

}