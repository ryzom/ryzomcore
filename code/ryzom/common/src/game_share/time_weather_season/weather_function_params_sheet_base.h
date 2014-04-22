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



#ifndef RY_WEATHER_FUNCTION_PARAMS_SHEET_BASE_H
#define RY_WEATHER_FUNCTION_PARAMS_SHEET_BASE_H

#include "nel/misc/types_nl.h"

namespace NLGEORGES
{
	class UForm;
	class UFormElm;

}

namespace NLMISC
{
	class IStream;
	struct EStream;
	class CSheetId;
}
class CWeatherFunctionParamsSheetBase
{
public:
	uint32  DayLength;   // length of day, in hours
	uint32  CycleLength; // length of a cycle, in hours
	//
	float   MinThunderPeriod; // Min thunder period, in s.
	float   ThunderLength; // Length of a thunder strike, in s.
	//
	float   CloudWindSpeedFactor;
	float   CloudMinSpeed;

	// ctor
	CWeatherFunctionParamsSheetBase();
	//
	void build(const NLGEORGES::UFormElm &item);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	//
	void build(const std::string &sheetName);

	//
	void readGeorges (const NLGEORGES::UForm *form, const NLMISC::CSheetId &sheetId);
	void removed() const {}
	static uint32 getVersion() { return 0; }
};







#endif
