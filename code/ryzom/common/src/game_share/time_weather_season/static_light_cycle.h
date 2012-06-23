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



#ifndef RY_STATIC_LIGHT_CYCLE_H
#define RY_STATIC_LIGHT_CYCLE_H

#include "nel/misc/types_nl.h"
#include "nel/georges/load_form.h"

/**
 * Class containing the data used to manage day cycles ( read from sheets )
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CStaticLightCycle
{
public:

	struct SLightCycle
	{
		float DayHour;
		float DayToDuskHour;
		float DuskToNightHour;
		float NightHour;
		float NightToDayHour;

		/// serialize
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( DayHour );
			f.serial( DayToDuskHour );
			f.serial( DuskToNightHour );
			f.serial( NightHour );
			f.serial( NightToDayHour );
		}
	};

	std::vector< SLightCycle > LightCycles;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont( LightCycles );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1; }

	/// called when the sheet is removed
	void removed() {}
};


#endif // RY_STATIC_LIGHT_CYCLE_H

/* End of static_light_cycle.h */
