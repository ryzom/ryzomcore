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

#ifndef NL_OUTPOST_H
#define NL_OUTPOST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/smart_ptr.h"
#include "client_sheets/continent_sheet.h"
#include "game_share/misc_const.h"


class CVillage;


// ***************************************************************************
/**
 * Class to manage an outpost on client:
 *		- collisions
 *		- display in 3D through a CVillage* (nb: used only to display ruins now)
 *	Yoyo: ugly: wait for static object and collision stuff
 */
class COutpost
{
public:

	COutpost();
	COutpost(const COutpost &other);
	COutpost &operator=(const COutpost &) {nlstop;/*forbidden*/return *this;}
	~COutpost();

	/// Build the outpost
	bool	setupOutpost(const CContinentParameters::CZC &zone, sint32 outpostId, CVillage *village);

	// Get the outpost Id. -1 if not setuped
	sint32	getOutpostId () const	{return _OutpostId;}

	// Set the building properties
	void	setBuildingPosition (uint building, const NLMISC::CQuat &rot, const NLMISC::CVector &position);

	// Register Collisions
	void initOutpost ();

	// Remove Collisions
	void removeOutpost ();

private:
	// Outpost building
	class CBuilding
	{
	public:
		CBuilding() {Position= NLMISC::CVector::Null; Rotation= NLMISC::CQuat::Identity;}
		NLMISC::CQuat		Rotation;
		NLMISC::CVector		Position;
	};
	CBuilding _Buildings[RZ_MAX_BUILDING_PER_OUTPOST];
	std::vector<NLPACS::UMovePrimitive *> _AddedPrims;

	// Outpost number
	sint						_OutpostId;

	// We may want to display the village as ruins
	NLMISC::CRefPtr<CVillage>	_Village;
};


#endif // NL_OUTPOST_H

/* End of outpost.h */
