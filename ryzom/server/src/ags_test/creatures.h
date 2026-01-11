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




#ifndef AGST_CREATURE_H
#define AGST_CREATURE_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

// Game share
#include "nel/misc/sheet_id.h"

namespace AGS_TEST
{

/**
 * Singleton containing database on information on creatures
 * \author Sadge
 * \author Nevrax France
 * \date 2002
 */
class CCreatures
{
public:
	class CCreatureRecord
	{
	public:
		CCreatureRecord () : _walkSpeed(1.3f), _runSpeed(6.0f) {}

		float _walkSpeed;
		float _runSpeed;

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (_walkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (_runSpeed, "Basics.MovementSpeeds.RunSpeed");
		}

		void serial (NLMISC::IStream &s)
		{
			s.serial (_walkSpeed, _runSpeed);
		}

		static uint getVersion () { return 1; }
	};

	// load the creature data from the george files
	static void init();

	// display the creature data for all known creature types
	static void display();

	// get a creature data record from the database
	static const CCreatureRecord *lookup( NLMISC::CSheetId id );

private:
	// prohibit cnstructor as this is a singleton
	CCreatures();

	static std::map<NLMISC::CSheetId,CCreatureRecord> _creatures;
	static bool _initialised;
};

} // end of namespace AGS_TEST

#endif // AGST_CREATURE_H
/* End of creatures.h */
