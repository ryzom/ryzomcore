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

#ifndef CHARACTER_SUMMARY_H
#define CHARACTER_SUMMARY_H

#include "nel/misc/ucstring.h"

#include "player_visual_properties.h"
#include "people.h"
#include "roles.h"
#include "character_title.h"
#include "far_position.h"

/**
 * CCharacterSummary
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
struct CCharacterSummary
{
	CCharacterSummary()
	{
		Mainland = TSessionId(0);
		Name.erase();
		Location = 0;
		VisualPropA = 0;
		VisualPropB = 0;
		VisualPropC = 0;
		People = EGSPD::CPeople::Unknown;
		Title = CHARACTER_TITLE::NB_CHARACTER_TITLE;
		CharacterSlot = 255;
		InRingSession = false;
		HasEditSession = false;
		InNewbieland = false;
	}

	/// mainland
	TSessionId Mainland;

	/// name
	ucstring Name;

	/// Localisation
	uint32 Location;

	/// visual property for appearance
	SPropVisualA VisualPropA;
	SPropVisualB VisualPropB;
	SPropVisualC VisualPropC;

	EGSPD::CPeople::TPeople People;

	NLMISC::CSheetId  SheetId;

	CHARACTER_TITLE::ECharacterTitle Title;

	uint8 CharacterSlot;

	bool  InRingSession;
	bool HasEditSession;
	bool InNewbieland;

	/// serialisation coming from a stream (net message)
	void serial(class NLMISC::IStream &f) throw (NLMISC::EStream);
};

#endif
