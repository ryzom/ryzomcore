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
#include "continent_manager_build.h"
#include "sheet_manager.h"

// Misc
#include "nel/misc/types_nl.h"

// std.
#include <vector>
#include <map>
#include <string>

// Ligo
#include "nel/ligo/primitive_utils.h"

// ***************************************************************************

extern NLLIGO::CLigoConfig LigoConfig;

// ***************************************************************************

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************

struct sTmpContinent
{
	vector<CContLandMark> ContLandMarks;
	NLLIGO::CPrimZone	Zone;
	NLMISC::CVector2f	ZoneCenter;

	sTmpContinent() { ZoneCenter.x = ZoneCenter.y = 0.0f; }
};

// ***************************************************************************
// Convert a primitive to a landmark
void primitiveToLM(CContLandMark &lm, IPrimitive *p)
{
	lm.Zone.VPoints.clear();
	string primName;
	p->getPropertyByName("name", primName);
	lm.TitleTextID = primName;
	CPrimVector *pvVect = p->getPrimVector();
	if (pvVect != NULL)
		for (uint32 j = 0; j < p->getNumVector(); ++j)
			lm.Zone.VPoints.push_back(CPrimVector(CVector2f(pvVect[j].x,pvVect[j].y)));

	lm.Pos = lm.getZoneCenter();
}

// ***************************************************************************
bool buildLMConts(const std::string &worldSheet, const std::string &primitivesPath, const std::string &dataPath)
{
	bool bRebuild = false;

	// Load the name of all continents

	map<string, sTmpContinent*> AllContinents;	// Map with all continents.
	{
		CEntitySheet *sheet = SheetMngr.get(CSheetId(worldSheet));

		if (!sheet || sheet->type() != CEntitySheet::WORLD)
		{
			nlerror("World sheet not found or bad type");
			return false;
		}

		CWorldSheet *ws = (CWorldSheet *) sheet;

		// Copy datas from the sheet
		for (uint32 i = 0; i < ws->ContLocs.size(); ++i)
		{
			const SContLoc &clTmp = ws->ContLocs[i];
			sTmpContinent *pCont = new sTmpContinent;
			AllContinents.insert(make_pair(clTmp.SelectionName, pCont));
		}

	}

	// Get all region_*.primitive files

	vector<string> vRegionFiles;
	{
		vector<string> vTmp;
		CPath::getPathContent(primitivesPath, true, false, true, vTmp);
		for (uint32 i = 0; i < vTmp.size(); ++i)
		{
			string filename = CFile::getFilename(vTmp[i]);
			string ext = CFile::getExtension(vTmp[i]);
			if (strnicmp(filename.c_str(), "region_", 7) == 0)
				if (stricmp(ext.c_str(), "primitive") == 0)
					vRegionFiles.push_back(vTmp[i]);
		}
	}

	// If the packed file do not exists
	string sPackedFileName = CPath::lookup(LM_PACKED_FILE, false);
	if (sPackedFileName.empty())
	{
		sPackedFileName = NLMISC::CPath::standardizePath(dataPath) + LM_PACKED_FILE;
		bRebuild = true;
	}
	else
	{
		// Check the date of region files with the packed file
		uint32 nPackedDate = CFile::getFileModificationDate(sPackedFileName);
		uint32 nRegionDate;
		for (uint32 i = 0; i < vRegionFiles.size(); ++i)
		{
			nRegionDate = CFile::getFileModificationDate(vRegionFiles[i]);
			if (nRegionDate > nPackedDate)
				bRebuild = true;
		}

		string swmf = CPath::lookup(WORLD_MAP_FILE);
		nRegionDate = CFile::getFileModificationDate(swmf);
		if (nRegionDate > nPackedDate)
			bRebuild = true;
	}

	if (bRebuild)
	{
		map<uint32, string> aliasToRegionMap;

		nlinfo("Rebuilding landmarks for all continents");
		for (uint32 nRegion = 0; nRegion < vRegionFiles.size(); ++nRegion)
		{
			CPrimitives PrimDoc;
			CPrimitiveContext::instance().CurrentPrimitive = &PrimDoc;
			if (!loadXmlPrimitiveFile(PrimDoc, vRegionFiles[nRegion], LigoConfig))
			{
				nlwarning("cannot open %s file", vRegionFiles[nRegion].c_str());
				CPrimitiveContext::instance().CurrentPrimitive = NULL;
				continue;
			}

			CPrimitiveContext::instance().CurrentPrimitive = NULL;

			// Select all nodes continent
			TPrimitiveClassPredicate predCont("continent");
			CPrimitiveSet<TPrimitiveClassPredicate> setCont;
			TPrimitiveSet vContRes;
			setCont.buildSet(PrimDoc.RootNode, predCont, vContRes);
			for (uint32 nCont = 0; nCont < vContRes.size(); ++nCont)
			{
				string contName;
				vContRes[nCont]->getPropertyByName("id", contName);
				map<string,sTmpContinent*>::iterator it = AllContinents.find(contName);
				if (it != AllContinents.end())
				{
					uint32 i;
					sTmpContinent *pCont = it->second;
					CContLandMark lm;
					CPrimitiveSet<TPrimitiveClassPredicate> genSet;
					TPrimitiveSet vGenRes;

					// Load the surrounding zone of the continent
					primitiveToLM(lm, vContRes[nCont]);
					if (lm.Zone.VPoints.size() != 0)
						pCont->Zone = lm.Zone;
					else
						nlwarning("continent %s do not contain any Zone (used for select)", contName.c_str());
					pCont->ZoneCenter = CContLandMark::getZoneCenter(pCont->Zone);

					// Get all the regions
					TPrimitiveClassPredicate predReg("region");
					genSet.buildSet(vContRes[nCont], predReg, vGenRes);
					for (i = 0; i < vGenRes.size(); ++i)
					{
						lm.Type = CContLandMark::Region;
						primitiveToLM(lm, vGenRes[i]);
						if (lm.Zone.VPoints.size() != 0)
							pCont->ContLandMarks.push_back(lm);
						else
							nlwarning("region %s do not contain any point", lm.TitleTextID.c_str());


						// get alias and region name
						uint32 alias = 0;
						string primName, primAlias;
						vGenRes[i]->getPropertyByName("name", primName);

						TPrimitiveClassPredicate pred("alias");
						IPrimitive *aliasNode = getPrimitiveChild(const_cast<IPrimitive*>(vGenRes[i]), pred);
						if (aliasNode)
						{
							CPrimAlias *pa = dynamic_cast<CPrimAlias*>(aliasNode);
							alias = pa->getFullAlias();
						}

						// associate alias to region
						aliasToRegionMap[alias]= primName;
						//nlinfo("aliasToRegionMap -- name = %s : %u", primName.c_str(), alias);
					}

					// Get all cities and place (lieudit) and outpost
					TPrimitiveClassPredicate predPlace("place");
					vGenRes.clear();
					genSet.buildSet(vContRes[nCont], predPlace, vGenRes);
					for (i = 0; i < vGenRes.size(); ++i)
					{
						string placeType, disp;
						vGenRes[i]->getPropertyByName("displayed", disp);
						if (disp != "true") continue; // Does the place is displayed

						vGenRes[i]->getPropertyByName("place_type", placeType);

						lm.Type = CContLandMark::Place;
						if (placeType == "Capital")			lm.Type = CContLandMark::Capital;
						else if (placeType == "Village")	lm.Type = CContLandMark::Village;
						else if (placeType == "Outpost")	lm.Type = CContLandMark::Outpost;
						else if (placeType == "Locality")	lm.Type = CContLandMark::Place;
						else if (placeType == "Street")		lm.Type = CContLandMark::Street;

						primitiveToLM(lm, vGenRes[i]);
						if (lm.Zone.VPoints.size() != 0)
							pCont->ContLandMarks.push_back(lm);
						else
							nlwarning("place %s do not contain any point", lm.TitleTextID.c_str());
					}

					// Get all stables
					TPrimitiveClassPredicate predStable("stable");
					vGenRes.clear();
					genSet.buildSet(vContRes[nCont], predStable, vGenRes);
					for (i = 0; i < vGenRes.size(); ++i)
					{
						lm.Type = CContLandMark::Stable;
						primitiveToLM(lm, vGenRes[i]);
						if (lm.Zone.VPoints.size() != 0)
							pCont->ContLandMarks.push_back(lm);
						else
							nlwarning("stable %s do not contain any point", lm.TitleTextID.c_str());
					}
				}
				else
				{
					nlwarning("cannot find continent %s in %s", contName.c_str(), worldSheet.c_str());
				}
			}
		}
		// Load the world map file
		vector<CContLandMark>	WorldMap;
		{
			CPrimitives PrimDoc;
			CPrimitiveContext::instance().CurrentPrimitive = &PrimDoc;

			string swmf = CPath::lookup(WORLD_MAP_FILE);
			if (!loadXmlPrimitiveFile(PrimDoc, swmf, LigoConfig))
			{
				nlwarning("cannot open %s file (%s)", WORLD_MAP_FILE, swmf.c_str());
			}
			else
			{
				// Get all the regions
				CContLandMark lm;
				TPrimitiveClassPredicate predReg("region");
				CPrimitiveSet<TPrimitiveClassPredicate> setReg;
				TPrimitiveSet vRes;
				setReg.buildSet(PrimDoc.RootNode, predReg, vRes);
				for (uint32 i = 0; i < vRes.size(); ++i)
				{
					lm.Type = CContLandMark::Region;
					primitiveToLM(lm, vRes[i]);
					if (lm.Zone.VPoints.size() != 0)
						WorldMap.push_back(lm);
					else
						nlwarning("region %s do not contain any point", lm.TitleTextID.c_str());
				}
			}
			CPrimitiveContext::instance().CurrentPrimitive = NULL;

		}
		// Ok save the file
		COFile f;
		if (f.open(sPackedFileName))
		{
			uint32 nNbCont = 0;
			map<string,sTmpContinent*>::iterator it = AllContinents.begin();
			while (it != AllContinents.end())
			{
				nNbCont++;
				it++;
			}
			sint ver= f.serialVersion(1);
			f.serial(nNbCont);
			it = AllContinents.begin();
			while (it != AllContinents.end())
			{
				sTmpContinent *pCont = it->second;
				string sContName = it->first;
				f.serial(sContName);
				// save the continent shapes
				f.serial(pCont->Zone);
				f.serial(pCont->ZoneCenter);
				f.serialCont(pCont->ContLandMarks);
				it++;
			}
			f.serialCont(WorldMap);
			if (ver >= 1)
				f.serialCont(aliasToRegionMap);
		}
	}

	map<string,sTmpContinent*>::iterator it = AllContinents.begin();
	while (it != AllContinents.end())
	{
		sTmpContinent *pCont = it->second;
		delete pCont;
		it++;
	}

	return true;
}

