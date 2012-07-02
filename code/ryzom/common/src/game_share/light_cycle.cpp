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
#include "light_cycle.h"
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form.h"




//============================================================
CSeasonLightCycle::CSeasonLightCycle() : DayHour(7),
                                         DayToDuskHour(19),
										 DuskToNightHour(20),
										 NightHour(21),
										 NightToDayHour(5)
{
}


//============================================================
/// Tool fct : get a value from a light cycle form, and display a warning if not available
template <class T>
static void getLightCycleValue(const NLGEORGES::UFormElm &item, T &dest, const char *name)
{
	nlassert(name);
	if (!item.getValueByName(dest, name)) nlwarning("Can't get field %s in a season_light_cycle sheet!", name);
}

//============================================================
void CSeasonLightCycle::build(const NLGEORGES::UFormElm &item)
{
	getLightCycleValue(item, DayHour, "DayHour");
	getLightCycleValue(item, DayToDuskHour, "DayToDuskHour");
	getLightCycleValue(item, DuskToNightHour, "DuskToNightHour");
	getLightCycleValue(item, NightHour, "NightHour");
	getLightCycleValue(item, NightToDayHour, "NightToDayHour");
}

//============================================================
void CLightCycle::build(const NLGEORGES::UFormElm &item)
{
	// Load each season
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		const NLGEORGES::UFormElm *seasonsItem;
		if (item.getNodeByName(&seasonsItem, EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason) k).c_str()) && seasonsItem)
		{
			SeasonLightCycle[k].build(*seasonsItem);
		}
		else nlwarning("CLightCycle::build : can't get infos about %s", EGSPD::CSeason::toString((EGSPD::CSeason::TSeason) k).c_str());
	}
	getLightCycleValue(item, RealDayLength, "RealDayLenght");
	getLightCycleValue(item, MaxNumColorSteps, "MaxNumColorSteps");
}

//============================================================
void CLightCycle::build(const char *sheetName)
{
	NLGEORGES::UFormLoader *loader = NLGEORGES::UFormLoader::createLoader();
	NLMISC::CSmartPtr<NLGEORGES::UForm> form = loader->loadForm(sheetName);
	if (form)
	{
		build(form->getRootNode());
	}
	else
	{
		nlwarning("Can't load form %s", sheetName);
	}
	NLGEORGES::UFormLoader::releaseLoader(loader);
}

//============================================================
CLightCycle::CLightCycle() : RealDayLength(3000),
							 NumHours(24),
							 MaxNumColorSteps(128)
{
}





