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



#ifndef RY_EVS_SHEETS_H
#define RY_EVS_SHEETS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

///Nel Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"


/**
 * Singleton containing database on information for actors
 * \author Sadge
 * \author Nevrax France
 * \date 2002
 */
class CSheets
{
public:
	class CSheet
	{
	public:
		CSheet(): WalkSpeed(1.3f), RunSpeed(6.0f), Radius(0.5f), Height(2.0f), BoundingRadius(0.5f), Width(1.0f), Length(1.0), DistBip01tofront(0.5f), DistBip01toback(-0.5f), DistBip01tomid(0.0f) {}

		float	WalkSpeed;
		float	RunSpeed;
		float	Radius;				// pacs primitive's radius
		float	Height;				// pacs primitive's height
		float	BoundingRadius;		// fighting radius
		float	Width;
		float	Length;
		float	DistBip01tofront;
		float	DistBip01toback;
		float	DistBip01tomid;

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
			form->getRootNode ().getValueByName (Radius, "Collision.CollisionRadius");
			form->getRootNode ().getValueByName (Height, "Collision.Height");
			form->getRootNode ().getValueByName (BoundingRadius, "Collision.BoundingRadius");

			form->getRootNode ().getValueByName (Width, "Collision.Width");
			form->getRootNode ().getValueByName (Length, "Collision.Length");

			form->getRootNode ().getValueByName (DistBip01tofront, "Collision.Dist Bip01 to front");
			form->getRootNode ().getValueByName (DistBip01toback, "Collision.Dist Bip01 to back");
			form->getRootNode ().getValueByName (DistBip01tomid, "Collision.Dist Bip01 to mid");		}

		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed, Radius, Height, BoundingRadius);
			s.serial (Width, Length);
			s.serial (DistBip01tofront, DistBip01toback, DistBip01tomid);
		}

		static uint getVersion () { return 1; }

		void removed() {}
	};

	// load the creature data from the george files
	static void init();

	// display the creature data for all known creature types
	static void display();

	//
	static void release() {}

	// get a data record from the database
	static const CSheet *lookup( NLMISC::CSheetId id );

private:
	// prohibit cnstructor as this is a singleton
	CSheets();

	static std::map<NLMISC::CSheetId,CSheet> _Sheets;
	static bool _Initialised;
};


#endif // RY_EVS_SHEETS_H
