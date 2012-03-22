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

#include "extract_new_sheet_names.h"

namespace TranslationManager
{

// ***************************************************************************
/*
 *	Specialisation of IWordListBuilder to list sheets in a directory
 */

bool CSheetWordListBuilder::buildWordList(std::vector<std::string> &allWords, std::string workSheetFileName)
{
	SheetExt = NLMISC::toLower(SheetExt);
	// verify the directory is correct
	if(!NLMISC::CFile::isDirectory(SheetPath))
	{
		nlwarning("Error: Directory '%s' not found. '%s' Aborted", SheetPath.c_str(), workSheetFileName.c_str());
		return false;
	}

	// list all files.
	std::vector<std::string> allFiles;
	allFiles.reserve(100000);
	NLMISC::CPath::getPathContent(SheetPath, true, false, true, allFiles, NULL);

	// Keep only the extension we want, and remove "_" (parent)
	allWords.clear();
	allWords.reserve(allFiles.size());
	for(size_t i = 0; i < allFiles.size(); i++)
	{
		std::string fileNameWithoutExt = NLMISC::CFile::getFilenameWithoutExtension(allFiles[i]);
		std::string extension = NLMISC::toLower(NLMISC::CFile::getExtension(allFiles[i]));

		// bad extension?
		if(extension!=SheetExt)
			continue;

		// parent?
		if(fileNameWithoutExt.empty() || fileNameWithoutExt[0] == '_')
			continue;

		// ok, add
		allWords.push_back(NLMISC::toLower(fileNameWithoutExt));
	}
	return true;
}

// ***************************************************************************
/*
 *	Specialisation of IWordListBuilder to list new region/place name from .primitive
 */
bool CRegionPrimWordListBuilder::buildWordList(std::vector<std::string> &allWords, std::string workSheetFileName)
{
	// verify the directory is correct
	if(!NLMISC::CFile::isDirectory(PrimPath))
	{
		nlwarning("Error: Directory '%s' not found. '%s' Aborted", PrimPath.c_str(), workSheetFileName.c_str());
		return false;
	}

	// list all files.
	std::vector<std::string> allFiles;
	allFiles.reserve(100000);
	NLMISC::CPath::getPathContent(PrimPath, true, false, true, allFiles, NULL);

	// parse all primitive that match the filter
	allWords.clear();
	allWords.reserve(100000);
	// to avoid duplicate
	std::set<std::string> allWordSet;
	for(size_t i = 0; i < allFiles.size(); i++)
	{
		std::string fileName = NLMISC::CFile::getFilename(allFiles[i]);
		// filter don't match?
		bool oneMatch= false;
		for(size_t filter = 0; filter < PrimFilter.size(); filter++)
		{
			if(NLMISC::testWildCard(fileName, PrimFilter[filter]))
				oneMatch= true;
		}
		if(!oneMatch)
			continue;

		// ok, read the file
		NLLIGO::CPrimitives PrimDoc;
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &PrimDoc;
		if (!NLLIGO::loadXmlPrimitiveFile(PrimDoc, allFiles[i], LigoConfig))
		{
			nlwarning("Error: cannot open file '%s'. '%s' Aborted", allFiles[i].c_str(), workSheetFileName.c_str());
			NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;
			return false;
		}
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

		// For all primitives of interest
		const char *listClass[] = {"continent", "region", "place", "stable",
								   "teleport_destination", "room_template"
								  };

		const char *listProp[] = {"name", "name", "name", "name",
								  "place_name", "place_name"
								 };

		const uint numListClass= sizeof(listClass)/sizeof(listClass[0]);
		const uint numListProp= sizeof(listProp)/sizeof(listProp[0]);
		nlctassert(numListProp == numListClass);
		for(uint cid = 0; cid < numListClass; cid++)
		{
			// parse the whole hierarchy
			NLLIGO::TPrimitiveClassPredicate predCont(listClass[cid]);
			NLLIGO::CPrimitiveSet<NLLIGO::TPrimitiveClassPredicate> setPlace;
			NLLIGO::TPrimitiveSet placeRes;
			setPlace.buildSet(PrimDoc.RootNode, predCont, placeRes);

			// for all found
			for (size_t placeId = 0; placeId < placeRes.size(); ++placeId)
			{
				std::string primName;
				if(placeRes[placeId]->getPropertyByName(listProp[cid], primName) && !primName.empty())
				{
					primName = NLMISC::toLower(primName);
					// avoid duplicate
					if(allWordSet.insert(primName).second)
					{
						allWords.push_back(primName);
					}
				}
			}
		}
	}
	return true;
}

}