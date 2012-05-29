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
#include "village_sheet.h"
//
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;


//=====================================================================================================================
/**Tool fct to extract a value from a sheet, and to display a warning if it failed.
  */
template <class T> static void GetVillageFormValue(const NLGEORGES::UFormElm &item, T &destItem, const char *name)
{
	nlassert(name);
	if (!item.getValueByName(destItem, name)) nlwarning("VillageSheet : can't get %s value.", name);
}

//=======================================================================
void CVillageIG::build(const NLGEORGES::UFormElm &item)
{
	GetVillageFormValue(item, IgName, "IgName");
	GetVillageFormValue(item, ParentName, "ParentName");
}

//=======================================================================
void CVillageSheet::build(const NLGEORGES::UFormElm &item)
{
	GetVillageFormValue(item, Zone, "Zone");
	GetVillageFormValue(item, Altitude, "Altitude");
	GetVillageFormValue(item, ForceLoadDist, "ForceLoadDist");
	GetVillageFormValue(item, LoadDist, "LoadDist"),
	GetVillageFormValue(item, UnloadDist, "UnloadDist");
	GetVillageFormValue(item, CenterX, "CenterX");
	GetVillageFormValue(item, CenterY, "CenterY");
	GetVillageFormValue(item, Width, "Width");
	GetVillageFormValue(item, Height, "Height");
	GetVillageFormValue(item, Rotation, "Rotation");

	const UFormElm *elm;
	if(item.getNodeByName (&elm, "IgList") && elm)
	{
		// Get number of ig
		uint numIGs;
		nlverify (elm->getArraySize (numIGs));
		IGs.resize(numIGs);

		// For each ig
		for(uint k = 0; k < numIGs; ++k)
		{
			const UFormElm *igForm;
			if (elm->getArrayNode (&igForm, k) && igForm)
			{
				IGs[k].build(*igForm);
			}
		}
	}
}
//=======================================================================
void CVillageSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Zone);
	f.serial(Altitude);
	f.serial(ForceLoadDist);
	f.serial(LoadDist);
	f.serial(UnloadDist);
	f.serial(CenterX);
	f.serial(CenterY);
	f.serial(Width);
	f.serial(Height);
	f.serial(Rotation);
	f.serial(Name);
	f.serialCont(IGs);
}

CVillageSheet::CVillageSheet() : Altitude(0),
								 ForceLoadDist(0),
								 LoadDist(0),
								 UnloadDist(0),
								 CenterX(0),
								 CenterY(0),
								 Width(0),
								 Height(0),
								 Rotation(0)
{
}
