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
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/misc/algo.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace STRING_MANAGER;


static CLigoConfig		LigoConfig;
static bool	RemoveOlds = false;


// ***************************************************************************
/*
 *	Interface to build the whole list of words (key id) for a specific worksheet
 */
struct IWordListBuilder
{
	virtual bool	buildWordList(std::vector<string> &allWords, string workSheetFileName) =0;
};


// ***************************************************************************
/*
 *	Specialisation of IWordListBuilder to list sheets in a directory
 */
struct CSheetWordListBuilder : public IWordListBuilder
{
	string		SheetExt;
	string		SheetPath;

	virtual bool	buildWordList(std::vector<string> &allWords, string workSheetFileName)
	{
		SheetExt= toLower(SheetExt);
		
		// verify the directory is correct
		if(!CFile::isDirectory(SheetPath))
		{
			nlwarning("Error: Directory '%s' not found. '%s' Aborted", SheetPath.c_str(), workSheetFileName.c_str());
			return false;
		}
		
		// list all files.
		std::vector<string>		allFiles;
		allFiles.reserve(100000);
		CPath::getPathContent(SheetPath, true, false, true, allFiles, NULL);
		
		// Keep only the extension we want, and remove "_" (parent)
		allWords.clear();
		allWords.reserve(allFiles.size());
		for(uint i=0;i<allFiles.size();i++)
		{
			string	fileNameWithoutExt= CFile::getFilenameWithoutExtension(allFiles[i]);
			string	extension= toLower(CFile::getExtension(allFiles[i]));
			// bad extension?
			if(extension!=SheetExt)
				continue;
			// parent?
			if(fileNameWithoutExt.empty()||fileNameWithoutExt[0]=='_')
				continue;
			// ok, add
			allWords.push_back(toLower(fileNameWithoutExt));
		}

		return true;
	}
	
};


// ***************************************************************************
/*
 *	Specialisation of IWordListBuilder to list new region/place name from .primitive
 */
struct CRegionPrimWordListBuilder : public IWordListBuilder
{
	string			PrimPath;
	vector<string>	PrimFilter;
	
	virtual bool	buildWordList(std::vector<string> &allWords, string workSheetFileName)
	{
		// verify the directory is correct
		if(!CFile::isDirectory(PrimPath))
		{
			nlwarning("Error: Directory '%s' not found. '%s' Aborted", PrimPath.c_str(), workSheetFileName.c_str());
			return false;
		}
		
		// list all files.
		std::vector<string>		allFiles;
		allFiles.reserve(100000);
		CPath::getPathContent(PrimPath, true, false, true, allFiles, NULL);
		
		// parse all primitive that match the filter
		allWords.clear();
		allWords.reserve(100000);
		// to avoid duplicate
		set<string>		allWordSet;
		for(uint i=0;i<allFiles.size();i++)
		{
			string	fileName= CFile::getFilename(allFiles[i]);
			// filter don't match?
			bool	oneMatch= false;
			for(uint filter=0;filter<PrimFilter.size();filter++)
			{
				if(testWildCard(fileName, PrimFilter[filter]))
					oneMatch= true;
			}
			if(!oneMatch)
				continue;

			// ok, read the file
			CPrimitives PrimDoc;
			CPrimitiveContext::instance().CurrentPrimitive = &PrimDoc;
			if (!loadXmlPrimitiveFile(PrimDoc, allFiles[i], LigoConfig))
			{
				nlwarning("Error: cannot open file '%s'. '%s' Aborted", allFiles[i].c_str(), workSheetFileName.c_str());
				CPrimitiveContext::instance().CurrentPrimitive = NULL;
				return false;
			}
			CPrimitiveContext::instance().CurrentPrimitive = NULL;
			
			// For all primitives of interest
			const char	*listClass[]= {"continent", "region", "place", "stable", 
				"teleport_destination", "room_template"};
			const char	*listProp[]= {"name", "name", "name", "name", 
				"place_name", "place_name"};
			const uint	numListClass= sizeof(listClass)/sizeof(listClass[0]);
			const uint	numListProp= sizeof(listProp)/sizeof(listProp[0]);
			nlctassert(numListProp==numListClass);
			for(uint cid=0;cid<numListClass;cid++)
			{
				// parse the whole hierarchy
				TPrimitiveClassPredicate predCont(listClass[cid]);
				CPrimitiveSet<TPrimitiveClassPredicate> setPlace;
				TPrimitiveSet placeRes;
				setPlace.buildSet(PrimDoc.RootNode, predCont, placeRes);
				// for all found
				for (uint placeId= 0; placeId < placeRes.size(); ++placeId)
				{
					string primName;
					if(placeRes[placeId]->getPropertyByName(listProp[cid], primName) && !primName.empty())
					{
						primName= toLower(primName);
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
};


// ***************************************************************************
void	extractNewWords(string workSheetFileName, string columnId, IWordListBuilder &wordListBuilder)
{
	uint	i;

	// **** Load the excel sheet
	// load
	TWorksheet		workSheet;
	if(!loadExcelSheet(workSheetFileName, workSheet, true))
	{
		nlwarning("Error reading '%s'. Aborted", workSheetFileName.c_str());
		return;
	}
	// get the key column index
	uint	keyColIndex = 0;
	
	if(!workSheet.findCol(columnId, keyColIndex))
	{
		nlwarning("Error: Don't find the column '%s'. '%s' Aborted", columnId.c_str(), workSheetFileName.c_str());
		return;
	}
	// get the name column index
	uint	nameColIndex;
	if(!workSheet.findCol(ucstring("name"), nameColIndex))
	{
		nlwarning("Error: Don't find the column 'name'. '%s' Aborted", workSheetFileName.c_str());
		return;
	}
	// Make a copy of this worksheet, with strlwr on the key
	// Yoyo: I prefer not modify the original worksheet (don't know what bad side effect it can have....)
	TWorksheet		workSheetLwr= workSheet;
	for(i=0;i<workSheetLwr.size();i++)
	{
		ucstring	key= workSheetLwr.getData(i, keyColIndex);
		workSheetLwr.setData(i, keyColIndex, toLower(key));
	}
	

	// **** List all words with the builder given
	std::vector<string>		allWords;
	if(!wordListBuilder.buildWordList(allWords, workSheetFileName))
		return;


	// **** Append new one to the worksheet
	uint	nbAdd= 0;
	for(i=0;i<allWords.size();i++)
	{
		string	keyName= allWords[i];
		uint rowIdx;
		// search in the key lowred worksheet (avoid case bugs (they do exist...))
		if (!workSheetLwr.findRow(keyColIndex, keyName, rowIdx))
		{
			// we need to add the entry. Add it to the 2 workSheet to maintain coherence (avoid non unique etc...)
			rowIdx = workSheetLwr.size();
			// add to the workSheetLwr
			workSheetLwr.resize(workSheetLwr.size()+1);
			workSheetLwr.setData(rowIdx, keyColIndex, keyName);
			workSheetLwr.setData(rowIdx, nameColIndex, string("<GEN>")+keyName);
			// add to the workSheet
			workSheet.resize(workSheet.size()+1);
			workSheet.setData(rowIdx, keyColIndex, keyName);
			workSheet.setData(rowIdx, nameColIndex, string("<GEN>")+keyName);
			
			nbAdd++;
		}
	}


	// **** Remove no more present ones (and log)
	uint	nbRemove= 0;
	if(RemoveOlds)
	{
		// Build as a set
		std::set<string>	allWordSet;
		for(i=0;i<allWords.size();i++)
			allWordSet.insert(allWords[i]);
		// For all rows, append to a copy if not erased
		TWorksheet	tmpCopy, tmpCopyLwr;
		nlassert(workSheet.ColCount==workSheetLwr.ColCount);
		nlassert(workSheet.size()==workSheetLwr.size());
		tmpCopy.setColCount(workSheet.ColCount);
		tmpCopy.resize(workSheet.size());
		tmpCopyLwr.setColCount(workSheet.ColCount);
		tmpCopyLwr.resize(workSheet.size());
		uint	dstRowId=0;
		for(i=0;i<workSheet.size();i++)
		{
			string	keyStr= workSheetLwr.getData(i, keyColIndex).toString();
			// if first line, or if the key (lwred) is found in the list of files
			if(i==0 || allWordSet.find(keyStr)!=allWordSet.end())
			{
				tmpCopy.Data[dstRowId]= workSheet.Data[i];
				tmpCopyLwr.Data[dstRowId]= workSheetLwr.Data[i];
				dstRowId++;
			}
			else
			{
				nbRemove++;
				// log
				NLMISC::InfoLog->displayRawNL("'%s': '%s' entry erased at line '%d'.", workSheetFileName.c_str(), 
					keyStr.c_str(), i);
			}
		}
		// resize to correct new size
		tmpCopy.resize(dstRowId);
		tmpCopyLwr.resize(dstRowId);

		// copy back
		workSheet= tmpCopy;
		workSheetLwr= tmpCopyLwr;
	}


	// **** Save
	if(nbAdd==0 && nbRemove==0)
	{
		if(RemoveOlds)
			NLMISC::InfoLog->displayRawNL("'%s': No deprecated entry found.", workSheetFileName.c_str());
		NLMISC::InfoLog->displayRawNL("'%s': No new entry found.", workSheetFileName.c_str());
		// Don't save
	}
	else
	{
		if(RemoveOlds)
			NLMISC::InfoLog->displayRawNL("'%s': %d deprecated entry erased.", workSheetFileName.c_str(), nbRemove);
		NLMISC::InfoLog->displayRawNL("'%s': %d new entry found.", workSheetFileName.c_str(), nbAdd);
		// Save the not lowered worksheet
		ucstring s = prepareExcelSheet(workSheet);
		try
		{
			CI18N::writeTextFile(workSheetFileName.c_str(), s, false);
		}
		catch (const Exception &e)
		{
			nlwarning("cannot save file: '%s'. Reason: %s", workSheetFileName.c_str(), e.what());
		}
	}
}


// ***************************************************************************
int extractNewSheetNames(int argc, char *argv[])
{
	// **** read the parameters
	for (int i=2; i<argc; ++i)
	{
		string s = argv[i];
		if (s == "-r")
		{
			// active remove mode
			RemoveOlds = true;
		}
		else
		{
			nlwarning("Unknow option '%s'", argv[i]);
			return -1;
		}
	}
	
	// **** avoid some flood
	NLMISC::createDebug();
	NLMISC::DebugLog->addNegativeFilter("numCol changed to");
	NLMISC::InfoLog->addNegativeFilter("CPath::addSearchPath");
	

	// **** read the configuration file
	CConfigFile	cf;
	cf.load("bin/translation_tools.cfg");
	CConfigFile::CVar &paths = cf.getVar("Paths");
	CConfigFile::CVar &pathNoRecurse= cf.getVar("PathsNoRecurse");
	CConfigFile::CVar &ligoClassFile= cf.getVar("LigoClassFile");

	// parse path
	for (uint i=0; i<paths.size(); ++i)
	{
		CPath::addSearchPath(paths.asString(i), true, false);
	}
	for (uint i=0; i<pathNoRecurse.size(); ++i)
	{
		CPath::addSearchPath(pathNoRecurse.asString(i), false, false);
	}
	
	// init ligo config once
	string ligoPath = CPath::lookup(ligoClassFile.asString(), true, true);
	LigoConfig.readPrimitiveClass(ligoPath.c_str(), false);
	NLLIGO::Register();
	CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;
	
	
	
	// **** Parse all the different type of sheets
	const char	*sheetDefs[]=
	{
		// 1st is the name of the worksheet file. 
		// 2nd is the Key column identifier. 
		// 3rd is the sheet extension
		// 4th is the directory where to find new sheets
		"work/item_words_wk.txt",		"item ID",		"sitem",		"l:/leveldesign/game_element/sitem",
		"work/creature_words_wk.txt",	"creature ID",	"creature",		"l:/leveldesign/game_elem/creature/fauna",	// take fauna only because other are special
		"work/sbrick_words_wk.txt",		"sbrick ID",	"sbrick",		"l:/leveldesign/game_element/sbrick",
		"work/sphrase_words_wk.txt",	"sphrase ID",	"sphrase",		"l:/leveldesign/game_element/sphrase",
	};
	uint	numSheetDefs= sizeof(sheetDefs) / (4*sizeof(sheetDefs[0]));
	
	// For all different type of sheet
	for(uint i=0;i<numSheetDefs;i++)
	{
		CSheetWordListBuilder	builder;
		builder.SheetExt= sheetDefs[i*4+2];
		builder.SheetPath= sheetDefs[i*4+3];
		extractNewWords(sheetDefs[i*4+0], sheetDefs[i*4+1], builder);
	}


	// **** Parse place and region names
	{
		// build place names
		CRegionPrimWordListBuilder	builder;
		builder.PrimPath= "l:/primitives";
		builder.PrimFilter.push_back("region_*.primitive");
		builder.PrimFilter.push_back("indoors_*.primitive");
		extractNewWords("work/place_words_wk.txt", "placeId", builder);
	}


	return 0;
}



