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
#include <memory>

#include "continent_container.h"

#include <memory>

#include "nel/misc/aabbox.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/command.h"
#include <nel/misc/algo.h>

#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

#include "light_ig_loader.h"


using namespace std;
using namespace NLMISC;
using namespace NLPACS;
using namespace NLGEORGES;


// Constructor
CContinentContainer::CContinentContainer()
{
}

//
void	CContinentContainer::init(uint gridWidth, uint gridHeight, double primitiveMaxSize, uint nbWorldImages, const string packedSheetsDirectory, double cellSize, bool loadPacsPrims)
{
	_GridWidth = gridWidth;
	_GridHeight = gridHeight;
	_PrimitiveMaxSize = primitiveMaxSize;
	_NbWorldImages = nbWorldImages;
	_CellSize = cellSize;
	_LoadPacsPrims = loadPacsPrims;

	std::vector<std::string> filters;
	filters.push_back("continent");
	loadForm(filters, packedSheetsDirectory+"continents.packed_sheets", _SheetMap);
}

//
void	CContinentContainer::loadContinent(string name, string file, sint index, bool allowAutoSpawn)
{
	nlinfo("loadContinent(\"%s\", \"%s\", %d)", name.c_str(), file.c_str(), index);

	// check if the continent is already loaded
/*	{
		TContinentContainer::iterator first(_Continents.begin()), last(_Continents.end());
		for (; first != last; ++first)
		{
			if (first->Name == name)
			{
				nlinfo("loadContinent(\"%s\", \"%s\", %d) : continent already loaded, ignoring second load.", name.c_str(), file.c_str(), index);
				return;
			}
		}
	}
*/
	nlassert(index >= 0);

	TSheetMap::iterator	its, found = _SheetMap.end();

	for (its=_SheetMap.begin(); its!=_SheetMap.end(); ++its)
	{
		if (NLMISC::toLower((*its).second.Name) == NLMISC::toLower(name+".continent") ||
			NLMISC::toLower((*its).second.PacsRBank) == NLMISC::toLower(name+".rbank"))
		{
			if (found == _SheetMap.end())
			{
				found = its;
			}
			else
			{
				nlinfo("Found 2 different possible continent sheets for %s: %s and %s, continent is not loaded twice", name.c_str(), (*its).second.Name.c_str(), (*found).second.Name.c_str());
			}
		}
	}

	if (found != _SheetMap.end())
	{
		name = (*found).second.Name;
		file = CFile::getFilenameWithoutExtension((*found).second.PacsRBank);
	}
	else
	{
		nlwarning("Couldn't find continent sheet for '%s', use name instead", name.c_str());
	}

	for (uint i=0; i<_Continents.size(); ++i)
	{
		if (_Continents[i].Name == name)
		{
			nlinfo("Continent '%s' already loaded, loading aborted for this new continent.", name.c_str());
			return;
		}
	}

	if ((sint)_Continents.size() <= index)
		_Continents.resize(index+1);

	if (_Continents[index].RetrieverBank != NULL ||
		_Continents[index].GlobalRetriever != NULL ||
		_Continents[index].MoveContainer != NULL)
	{
		nlwarning("Init retriever bank failed, index %d already used by continent '%s'", index, _Continents[index].Name.c_str());
		return;
	}

	_Continents[index].Name = name;
	_Continents[index].AllowAutoSpawn = allowAutoSpawn;

	string	filename;

	// load the rbank
	filename = file+".rbank";
	_Continents[index].RetrieverBank = URetrieverBank::createRetrieverBank ( filename.c_str(), true );
	if( _Continents[index].RetrieverBank == NULL )
	{
		nlwarning("Init retriever bank failed, file load is %s", filename.c_str() );
		return;
	}

	// load the gr
	filename = file+".gr";
	_Continents[index].GlobalRetriever = UGlobalRetriever::createGlobalRetriever ( filename.c_str(), _Continents[index].RetrieverBank );
	if( _Continents[index].GlobalRetriever == NULL )
	{
		nlwarning("Init global retriever failed, file load is %s", filename.c_str() );
		URetrieverBank::deleteRetrieverBank(_Continents[index].RetrieverBank);
		_Continents[index].RetrieverBank = NULL;
		return;
	}

	uint	gw = _GridWidth;
	uint	gh = _GridHeight;

	if (_CellSize != 0.0)
	{
		CAABBox		cbox = _Continents[index].GlobalRetriever->getBBox();

		gw = (uint)(cbox.getHalfSize().x*2.0 / _CellSize) + 1;
		gh = (uint)(cbox.getHalfSize().y*2.0 / _CellSize) + 1;
	}

	// create the move container
	/// \todo Ben : correct init for the move container cells count
	_Continents[index].MoveContainer = UMoveContainer::createMoveContainer ( _Continents[index].GlobalRetriever, gw, gh, _PrimitiveMaxSize, _NbWorldImages);

	if( _Continents[index].MoveContainer == NULL )
	{
		nlwarning("Init Move container failed, continent %s", name.c_str());
		URetrieverBank::deleteRetrieverBank(_Continents[index].RetrieverBank);
		UGlobalRetriever::deleteGlobalRetriever(_Continents[index].GlobalRetriever);
		_Continents[index].GlobalRetriever = NULL;
		_Continents[index].RetrieverBank = NULL;
		_Continents[index].MoveContainer = NULL;
	}

	_Continents[index].MoveContainer->setAsStatic(0);

	nlinfo("Loaded continent, initialized move container to %dx%d cells", gw, gh);

	if (found != _SheetMap.end())
		loadPacsPrims((*found).second, _Continents[index].MoveContainer);
}


//
void	CContinentContainer::removeContinent(sint index)
{
	nlassert(index >= 0);

	if (index >= (sint)_Continents.size() ||
		(_Continents[index].RetrieverBank == NULL &&
		_Continents[index].GlobalRetriever == NULL &&
		_Continents[index].MoveContainer == NULL))
	{
		//nlwarning("Can't remove continent, index %d not used", index);
		return;
	}

	nlinfo("Remove continent %d '%s'... Entities shouldn't point any longer to this continent !", index, _Continents[index].Name.c_str());

	_Continents[index].Name.clear();

	if (_Continents[index].MoveContainer != NULL)
		UMoveContainer::deleteMoveContainer(_Continents[index].MoveContainer);
	_Continents[index].MoveContainer = NULL;

	if (_Continents[index].GlobalRetriever != NULL)
		UGlobalRetriever::deleteGlobalRetriever(_Continents[index].GlobalRetriever);
	_Continents[index].GlobalRetriever = NULL;

	if (_Continents[index].RetrieverBank != NULL)
		URetrieverBank::deleteRetrieverBank(_Continents[index].RetrieverBank);
	_Continents[index].RetrieverBank = NULL;
}


//
void	CContinentContainer::initPacsPrim(const string &path)
{
	vector<string> fileNames;

	if (CFile::fileExists(CPath::lookup(path, false, false)))
	{
		nlinfo("Peeking into '%s' file for pacs_prim files", path.c_str());
		CIFile	primFile;
		if (primFile.open(CPath::lookup(path, false, false)))
		{
			char	primbuffer[1024];
			while (!primFile.eof())
			{
				primFile.getline(primbuffer, 1024);
				fileNames.push_back(CPath::lookup(primbuffer, false, false));
			}
		}
		else
		{
			nlwarning("Couldn't open file '%s' to load pacs_prims", path.c_str());
		}
	}
	else if (CFile::isExists(path))
	{
		nlinfo("Peeking into '%s' directory for pacs_prim files", path.c_str());
		//CPath::getPathContent(path, true, false, true, fileNames);
		CPath::addSearchPath(path, true, false);
		CPath::getFileList("pacs_prim", fileNames);
	}
	else
	{
		nlwarning("CContinentContainer: can't initPacsPrim(), path '%s' for pacs primitives not found", path.c_str());
		return;
	}

	nlinfo("%d file found at lookup", fileNames.size());

	//
	uint	k;
	uint	numPrims = 0;
	for(k=0; k<fileNames.size(); ++k)
	{
		// check extension
		if (NLMISC::toLower(CFile::getExtension(fileNames[k])) != "pacs_prim")
		{
			// not a pacs primitive, skip it..			
			continue;
		}
		try
		{		
			string	ppName = NLMISC::toLower(CFile::getFilenameWithoutExtension(fileNames[k]));

			if (_PacsPrimMap.find(ppName) != _PacsPrimMap.end())
				continue;

			std::auto_ptr<UPrimitiveBlock>	pb(UPrimitiveBlock::createPrimitiveBlockFromFile(CPath::lookup(fileNames[k], false)));
			UPrimitiveBlock*	ptr = pb.release();
			if (ptr != NULL)
			{
				_PacsPrimMap[ppName] = ptr;
				++numPrims;
			}
			else
			{
				nlwarning("Couldn't load prim block '%s'", fileNames[k].c_str());
			}
		}
		catch (const NLMISC::EStream &e)
		{
			nlwarning("Couldn't load Pacs Primitive Block file '%s': %s", fileNames[k].c_str(), e.what());
		}
	}

	nlinfo("%d primitive blocs initialised", numPrims);
}


//
void	CContinentContainer::loadPacsPrims(const CSheet &sheet, NLPACS::UMoveContainer *moveContainer)
{
	vector<string>	igs = sheet.ListIG;

	string			igFilename = CPath::lookup(sheet.LandscapeIG, false);

	if(!igFilename.empty())
	{
		CIFile	igFile;
		if (igFile.open(igFilename))
		{
			char	igbuffer[1024];
			while (!igFile.eof())
			{
				igFile.getline(igbuffer, 1024);
				if(strlen(igbuffer) > 0)
					igs.push_back(igbuffer);
			}
		}
		else
		{
			nlwarning("Couldn't open file '%s' to instantiate landscape pacs_prims", igFilename.c_str());
		}
	}
	else
	{
		nlwarning("Couldn't find file '%s' to instantiate landscape pacs_prims", sheet.LandscapeIG.c_str());
	}

	nlinfo("Loading igs for continent %s", sheet.Name.c_str());

	uint	numAddedPrimBlocs = 0;
	uint	numFoundIgs = 0;
	uint	i;
	for (i=0; i<igs.size(); ++i)
	{
		CLightIGLoader	igLoader;

		try
		{
			igLoader.loadIG(CFile::getFilenameWithoutExtension(igs[i])+".ig");

			++numFoundIgs;

			uint numInstances = igLoader.getNumInstance();	
			for(uint k = 0; k < numInstances; ++k)
			{
				TPacsPrimMap::iterator pbIt;

				string	shapeName = NLMISC::toLower(CFile::getFilenameWithoutExtension(igLoader.getShapeName(k)));
				string	instanceName = NLMISC::toLower(CFile::getFilenameWithoutExtension(igLoader.getInstanceName(k)));

				bool	isTrigger = false;
				bool	isZC = false;

				if ((pbIt = _PacsPrimMap.find(shapeName)) != _PacsPrimMap.end() ||
					// nice hardcoded trick that allows graphists to spawn ghost collisions in ZC
					// when shapename is like 'bat_zc_0?', spaw a pacs prim bloc called gen_bt_col_ext, so nice I just shit my pants
					(isZC = (testWildCard(shapeName.c_str(), "bat_zc_0?") && shapeName != "bat_zc_00" && (pbIt = _PacsPrimMap.find("gen_bt_col_ext")) != _PacsPrimMap.end())) ||	// the magic hack
					(isTrigger = ((pbIt = _PacsPrimMap.find(instanceName)) != _PacsPrimMap.end())))
				{
					if (_LoadPacsPrims || isTrigger)
					{
						// compute orientation and position
						CMatrix						instanceMatrix;
						igLoader.getInstanceMatrix(k, instanceMatrix);
						CVector						pos;
						float						angle;
						UMoveContainer::getPACSCoordsFromMatrix(pos, angle, instanceMatrix);
						// insert the matching primitive block
						vector<UMovePrimitive*>		insertedPrimitives;
						moveContainer->addCollisionnablePrimitiveBlock(pbIt->second, 0, 1, &insertedPrimitives, angle, pos, true);

						if (isTrigger)
						{
							uint	i;
							for (i=0; i<insertedPrimitives.size(); ++i)
							{
								UMovePrimitive	*prim = insertedPrimitives[i];
								uint64			id = prim->UserData;
								uint			triggerId = (uint)((prim->UserData & 0xffff0000) >> 16);
								_TriggerMap[triggerId] = prim->getFinalPosition(0);
							}
						}

						++numAddedPrimBlocs;
					}
				}
			}
		}
		catch(const Exception &e)
		{
			nlwarning("Failed to load IG '%s': %s", igs[i].c_str(), e.what());
		}
	}

	nlinfo("Loaded %d IGs", numFoundIgs);
	nlinfo("Added %d primitive blocs", numAddedPrimBlocs);
}


