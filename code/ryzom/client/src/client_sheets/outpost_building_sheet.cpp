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
#include "outpost_building_sheet.h"

#include "nel/georges/u_form_elm.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// ****************************************************************************
COutpostBuildingSheet::TOBType COutpostBuildingSheet::fromString( const string & str )
{
	if (str == "TownHall") return OB_TownHall;
	if (str == "Driller") return OB_Driller;
	return OB_Empty;
}

// ****************************************************************************
string COutpostBuildingSheet::toString( TOBType type )
{
	if (type == OB_TownHall) return "TownHall";
	if (type == OB_Driller) return "Driller";
	return "Empty";
}

// ****************************************************************************
COutpostBuildingSheet::COutpostBuildingSheet()
{
	Type = CEntitySheet::OUTPOST_BUILDING;

	CostDapper = 0;
	CostTime = 0;
	MPLevelOfHighestExtractRate = 0;
	IdIconMain = 0;
	IdIconBack = 0;
	IdIconOver = 0;
	IdIconText = 0;
}

// ****************************************************************************
void COutpostBuildingSheet::build(const NLGEORGES::UFormElm &root)
{
	string sType;
	root.getValueByName (sType, "type");
	OBType = fromString(sType);
	root.getValueByName(CostDapper, "cost_dapper");
	root.getValueByName(CostTime, "cost_time");

	// Driller
	Mps.clear();
	if (OBType == OB_Driller)
	{
		MPLevelOfHighestExtractRate= 50;

		const UFormElm *pDriller;
		if (root.getNodeByName(&pDriller, "driller") && pDriller)
		{
			// Get Mps sheets
			const UFormElm *pMp;
			uint32 i = 0;
			while (pDriller->getNodeByName(&pMp, ("mp" + NLMISC::toString(i)).c_str()) && pMp)
			{
				string sTmp;
				pMp->getValueByName(sTmp, "name");
				CSheetId	sheetId(sTmp);
				if(sheetId!=CSheetId::Unknown)
					Mps.push_back(sheetId);
				i++;
			}

			// Get best mp level
			float	bestRate= 0.f;
			for(uint i=50;i<=250;i+=50)
			{
				float	tmp= 0;
				if(pDriller->getValueByName(tmp, NLMISC::toString("quality_%03d", i).c_str()))
				{
					if(tmp>bestRate)
					{
						bestRate= tmp;
						MPLevelOfHighestExtractRate= i;
					}
				}
			}
		}
		else
		{
			debug("key 'driller' not found.");
		}
	}

	// Get the icon associated.
	string IconMain;
	if(!root.getValueByName (IconMain, "icon"))
		debug("key 'icon' not found.");
	IconMain = strlwr (IconMain);
	IdIconMain = ClientSheetsStrings.add(IconMain);

	// Get the icon associated.
	string IconBack;
	if(!root.getValueByName (IconBack, "icon background"))
		debug("key 'icon background' not found.");
	IconBack = strlwr (IconBack);
	IdIconBack = ClientSheetsStrings.add(IconBack);

	// Get the icon associated.
	string IconOver;
	if(!root.getValueByName (IconOver, "icon overlay"))
		debug("key 'icon overlay' not found.");
	IconOver = strlwr (IconOver);
	IdIconOver = ClientSheetsStrings.add(IconOver);

	// Get the icon text associated.
	string IconText;
	if(!root.getValueByName (IconText, "text overlay"))
		debug("key 'text overlay' not found.");
	IconText = strlwr (IconText);
	IdIconText = ClientSheetsStrings.add(IconText);
}

// ****************************************************************************
void COutpostBuildingSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(OBType);
	f.serial(CostDapper);
	f.serial(CostTime);
	f.serial(MPLevelOfHighestExtractRate);
	f.serialCont(Mps);
	ClientSheetsStrings.serial(f, IdIconMain);
	ClientSheetsStrings.serial(f, IdIconBack);
	ClientSheetsStrings.serial(f, IdIconOver);
	ClientSheetsStrings.serial(f, IdIconText);
}

