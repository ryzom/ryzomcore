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

#include "formFlora.h"

#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "nel/georges/u_form_elm.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// ---------------------------------------------------------------------------
void SFormFlora::build (UFormElm &item)
{
	IncludePatats.clear ();
	ExcludePatats.clear ();
	PlantInstances.clear ();

	// Read the Include_patats field
	UFormElm *pElt;
	if (item.getNodeByName (&pElt, "Include_patats") && pElt)
	{
		uint size;
		nlverify (pElt->getArraySize (size));
		for (uint i=0; i<size; i++)
		{
			string value;
			pElt->getArrayValue (value, i);
			IncludePatats.push_back (value);
		}
	}

	// Read the Include_patats field
	if (item.getNodeByName (&pElt, "Exclude_patats") && pElt)
	{
		uint size;
		nlverify (pElt->getArraySize (size));
		for (uint i=0; i<size; i++)
		{
			string value;
			pElt->getArrayValue (value, i);
			ExcludePatats.push_back (value);
		}
	}

	// Read the Plants field
	if (item.getNodeByName (&pElt, "Plants") && pElt)
	{
		uint size;
		nlverify (pElt->getArraySize (size));
		for (uint i=0; i<size; i++)
		{
			// Get the struct
			UFormElm *pArrayElt;
			if (pElt->getArrayNode (&pArrayElt, i) && pArrayElt)
			{
				SPlantInstance piTmp;
				pArrayElt->getValueByName (piTmp.Name, "File name");
				pArrayElt->getValueByName (piTmp.Density, "Density");
				pArrayElt->getValueByName (piTmp.Falloff, "Falloff");
				PlantInstances.push_back (piTmp);
			}
		}
	}

	item.getValueByName (JitterPos, "Jitter_Pos");

	item.getValueByName (ScaleMin, "Scale_Min");

	item.getValueByName (ScaleMax, "Scale_Max");

	item.getValueByName (PutOnWater, "Put_On_Water");

	item.getValueByName (WaterHeight, "Water_Height");

	item.getValueByName (RandomSeed, "Random_Seed");
}
