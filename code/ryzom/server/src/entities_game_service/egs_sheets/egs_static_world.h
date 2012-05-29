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



#ifndef EGS_STATIC_WORLD_H
#define EGS_STATIC_WORLD_H

//Nel georges
#include "nel/georges/u_form.h"


/**
 * class used to store the world organisation ( continent ids, ... )
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CStaticWorld
{
public:
	/// ctor
	CStaticWorld(){}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serialCont( Continents );
	}
	/// Removed
	void removed() {}
	
	/// continent in the world (indexed by CONTINENT::TContinent)
	std::vector< NLMISC::CSheetId > Continents;
};

/**
 * class used to store a continent organisation
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CStaticContinent
{
public:
	/// ctor
	CStaticContinent(){}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serialCont(Outposts);
	}
	/// Removed
	void removed() {}

	/// index of the outposts in this continent
	std::vector<uint8> Outposts;
};


#endif // EGS_STATIC_WORLD_H

/* End of egs_static_world.h */

