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

#ifndef RY_EGS_STATIC_CONTINENT_H
#define RY_EGS_STATIC_CONTINENT_H

//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"


class CStaticWorld
{
public:
	/// Serialisation
	void serial(class NLMISC::IStream &f);
	
	/// read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	
	// return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	
	/// called when the sheet is removed
	void removed() {}


	std::vector<

};



#endif // RY_EGS_STATIC_CONTINENT_H

/* End of egs_static_continent.h */






















