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

#if 0
#include "std_header.h"
//
#include "village.h"
#include "zone_util.h"
//
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
//
#include "nel/misc/path.h"
#include "nel/misc/file.h"
//
#include "3d/shape.h"


using namespace NLMISC;
using namespace NL3D;

//**********************************************************************************************
CVillageGrid::CVillageGrid()
{
	_ZoneMinX = 0;
	_ZoneMinY = 0;

}

//**********************************************************************************************
void CVillageGrid::init(uint gridWidth, uint gridHeight, sint zoneMinX, sint zoneMinY)
{
	reset();
	nlassert(gridWidth > 0);
	nlassert(gridHeight > 0);
	_ZoneMinX = zoneMinX;
	_ZoneMinY = zoneMinY;
	VillageGrid.init(gridWidth, gridHeight);
}

//**********************************************************************************************
void CVillageGrid::reset()
{
	NLMISC::contReset(*this);
}

//**********************************************************************************************
void CVillageGrid::addVillagesFromContinent(const std::string &continentSheetName)
{
	// Load the form
	NLGEORGES::UFormLoader *loader = NLGEORGES::UFormLoader::createLoader();
	//
	std::string path  = CPath::lookup(continentSheetName, false, false);
	if (path.empty())
	{
		nlwarning("Path not found for %s.", continentSheetName.c_str());
		return;
	}
	NLGEORGES::UForm *villageForm;
	villageForm = loader->loadForm(path.c_str());
	if(villageForm != NULL)
	{
		NLGEORGES::UFormElm &rootItem = villageForm->getRootNode();
		// try to get the village list
		// Load the village list
		NLGEORGES::UFormElm *villagesItem;
		if(!(rootItem.getNodeByName (&villagesItem, "Villages") && villagesItem))
		{
			nlwarning("No villages where found in %s", continentSheetName.c_str());
			return;
		}

		// Get number of village
		uint numVillage;
		nlverify (villagesItem->getArraySize (numVillage));

		// For each village
		for(uint k = 0; k < numVillage; ++k)
		{
			NLGEORGES::UFormElm *currVillage;
			if (!(villagesItem->getArrayNode (&currVillage, k) && currVillage))
			{
				nlwarning("Couldn't get village %d in continent %s", continentSheetName.c_str(), k);
				continue;
			}
			// check that this village is in the dependency zones
			NLGEORGES::UFormElm *zoneNameItem;
			if (!currVillage->getNodeByName (&zoneNameItem, "Zone") && zoneNameItem)
			{
				nlwarning("Couldn't get zone item of village %d in continent %s", continentSheetName.c_str(), k);
				continue;
			}
			std::string zoneName;
			if (!zoneNameItem->getValue(zoneName))
			{
				nlwarning("Couldn't get zone name of village %d in continent %s", continentSheetName.c_str(), k);
				continue;
			}			
			sint zoneX, zoneY;
			if (!getZonePos(zoneName, zoneX, zoneY))
			{
				nlwarning("Zone name of village %d in continent %s is invalid", continentSheetName.c_str(), k);
				continue;
			}			
			sint villageMinX, villageMinY;
			sint villageMaxX, villageMaxY;

			
			// retrieve width & height of covered region
			uint32 regionWidth;
			uint32 regionHeight;
			float  centerX, centerY;
			if (!currVillage->getValueByName(regionWidth,  "Width")   ||
				!currVillage->getValueByName(regionHeight, "Height")  || 
				!currVillage->getValueByName(centerX,	   "CenterX") ||
				!currVillage->getValueByName(centerY,	   "CenterY"))
			{
				nlwarning("Can't retrieve region covered by village %d in continent %s", continentSheetName.c_str(), k);
				continue;
			}
			//
			villageMinX = villageMaxX = zoneX;
			villageMinY = villageMaxY = zoneY;
			// extends with bbox from center to min corner if leveldesigner forgot to enter good width & height			
			villageMaxX = std::max(villageMaxX, (sint) ((zoneX * 160.f + 2.f * centerX) / 160.f));
			villageMaxY = std::max(villageMaxY, (sint) ((zoneY * 160.f + 2.f * centerY) / 160.f));			
			//
			villageMinX -= _ZoneMinX;
			villageMaxX -= _ZoneMinX;
			villageMinY -= _ZoneMinY;
			villageMaxY -= _ZoneMinY;
			//
			CVillage village;
			if (loadVillageSheet(currVillage, continentSheetName, k, village))
			{
				//
				village.FileModificationDate = std::max(village.FileModificationDate, CFile::getFileModificationDate(path));
				//
				Villages.push_back(CVillage());
				Villages.back().swap(village);
				//
				for (sint y = villageMinY; y <= (sint) villageMaxY; ++y)
				{
					if (y < 0) continue;
					if (y >= (sint) VillageGrid.getHeight()) continue;
					for (sint x = villageMinX; x <= (sint) villageMaxX; ++x)
					{
						if (x < 0) continue;
						if (x >= (sint) VillageGrid.getWidth()) continue;
						VillageGrid(x, y).push_back(Villages.size() - 1);
					}
				}
			}
		}				
	}
	else 
	{
		nlwarning("Can't load continent form : %s", continentSheetName.c_str());
	}
}

//**********************************************************************************************
bool CVillageGrid::loadVillageSheet(const NLGEORGES::UFormElm *villageItem, const std::string &continentSheetName, uint villageIndex, CVillage &dest)
{
	dest.IG.clear();
	const NLGEORGES::UFormElm *igNamesItem;
	if (! (villageItem->getNodeByName (&igNamesItem, "IgList") && igNamesItem) )
	{
		nlwarning("No list of IGs was found in the continent form %s, village #%d", continentSheetName.c_str(), (int) villageIndex);
		return false;
	}

	// Get number of village
	uint numIgs;
	nlverify (igNamesItem->getArraySize (numIgs));
	const NLGEORGES::UFormElm *currIg;
	uint32 mostRecentIGDate = 0;
	for(uint l = 0; l < numIgs; ++l)
	{														
		if (!(igNamesItem->getArrayNode (&currIg, l) && currIg))
		{
			nlwarning("Couldn't get ig #%d in the continent form %s, in village #%d", l, continentSheetName.c_str(), (int) villageIndex);
			continue;
		}			
		const NLGEORGES::UFormElm *igNameItem;
		currIg->getNodeByName (&igNameItem, "IgName");
		std::string igName;
		if (!igNameItem->getValue (igName))
		{
			nlwarning("Couldn't get ig name of ig #%d in the continent form %s, in village #%d", l, continentSheetName.c_str(), (int) villageIndex);
			continue;
		}
		if (igName.empty())
		{
			nlwarning("Ig name of ig #%d in the continent form %s, in village #%d is an empty string", l, continentSheetName.c_str(), (int) villageIndex);
			continue;
		}
		// ensure .ig
		igName = CFile::getFilenameWithoutExtension(igName) + ".ig";
				

		CIGInfo igInfo;

		// add this ig
		std::string nameLookup = CPath::lookup (igName, false, true);
		if (!nameLookup.empty())
		{
			igInfo.Path = nameLookup;
			dest.IG.push_back(igInfo);
			mostRecentIGDate = std::max(mostRecentIGDate, CFile::getFileModificationDate(nameLookup));
		}
		else
		{
			nlwarning("Couldn't find ig %s in continent form %s, in village %d", igName.c_str(), continentSheetName.c_str(), (int) villageIndex);
		}
	}
	dest.FileModificationDate = mostRecentIGDate;
	return true;
}


//**********************************************************************************************
void CVillage::load(TShapeCache &shapeCache)
{
	for(std::vector<CIGInfo>::iterator it = IG.begin(); it != IG.end(); ++it)
	{
		it->load(shapeCache);
	}	
}

//**********************************************************************************************
void CIGInfo::load(TShapeCache &shapeCache)
{
	if (Loaded) return;
	Loaded = true; // even if loading fails, don't try twice to load it
	try
	{
		CIFile stream;
		stream.open(Path);
		printf(Path.c_str());
		CSmartPtr<CInstanceGroup> ig = new CInstanceGroup;
		ig->serial(stream);
		IG = ig; // commit
	}
	catch(const EStream &e)
	{
		nlwarning(e.what());
	}
	if (IG)
	{
		// complete cache
		for(uint k = 0; k < IG->getNumInstance(); ++k)
		{
			std::string shapeName = standardizeShapeName(IG->getShapeName(k));			
			if (NLMISC::toLower(CFile::getExtension(shapeName)) == "pacs_prim")
			{
				continue;
			}
			TShapeCache::iterator it = shapeCache.find(shapeName);
			CShapeInfo si;			
			bool buildOK = false;
			if (it == shapeCache.end())
			{
				CShapeStream ss;
				try
				{
					
					CIFile stream;
					std::string path = CPath::lookup(shapeName, false, false);
					if (!path.empty())
					{
						stream.open(path);
						ss.serial(stream);
						CShapeInfo si;
						si.build(*ss.getShapePointer());
						delete ss.getShapePointer();
						shapeCache[shapeName].swap(si);						
					}
				}
				catch (const EStream &e)
				{
					// shape not loaded
					nlwarning(e.what());
				}
			}			
		}				
	}
}

#endif
