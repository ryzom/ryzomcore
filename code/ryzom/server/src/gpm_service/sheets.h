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



#ifndef RY_CMS_SHEETS_H
#define RY_CMS_SHEETS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sheet_id.h"

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
		CSheet(): WalkSpeed(1.3f), RunSpeed(6.0f), Radius(0.5f), Height(2.0f), BoundingRadius(0.5), Scale(1.0f) {}

		float	WalkSpeed;
		float	RunSpeed;
		float	Radius;				// pacs primitive's radius
		float	Height;				// pacs primitive's height
		float	BoundingRadius;		// fighting radius
		float	Scale;				// entity scale

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
			form->getRootNode ().getValueByName (Radius, "Collision.CollisionRadius");
			form->getRootNode ().getValueByName (Height, "Collision.Height");
			form->getRootNode ().getValueByName (BoundingRadius, "Collision.BoundingRadius");
			form->getRootNode ().getValueByName (Scale, "3d data.Scale");
		}

		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed, Radius, Height, BoundingRadius, Scale);
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

	static std::map<NLMISC::CSheetId,CSheet> _sheets;
	static bool _initialised;
};


#endif // RY_CMS_SHEETS_H
