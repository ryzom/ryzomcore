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



#ifndef RY_SPHRASE_SHEET_H
#define RY_SPHRASE_SHEET_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "entity_sheet.h"

#define SPHRASE_MAX_BRICK	100

// ***************************************************************************
/**
 * New Sabrina Brick Sheet def.
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2003 September
 */
class CSPhraseSheet : public CEntitySheet
{
public:
	// All these values are sheet id
	std::vector<NLMISC::CSheetId>	Bricks;
	// False if it is an upgrade phrase for instance. true means it can be memorized
	bool							Castable;
	// True if it can be shown in the ActionProgression window
	bool							ShowInActionProgression;
	// True if it can be shown in the ActionProgression window (only if all bricks learn)
	bool							ShowInAPOnlyIfLearnt;

public:

	CSPhraseSheet ()
	{
		Type = CEntitySheet::SPHRASE;
		Castable= true;
		ShowInActionProgression= true;
		ShowInAPOnlyIfLearnt= false;
	}

	virtual void build (const NLGEORGES::UFormElm &root);

	virtual void serial (NLMISC::IStream &s) throw(NLMISC::EStream)
	{
		s.serialCont (Bricks);
		s.serial(Castable);
		s.serial(ShowInActionProgression);
		s.serial(ShowInAPOnlyIfLearnt);
	}

	// Valid if Bricks not empty and all Bricks sheetId != NULL.
	bool	isValid() const;
};


#endif // RY_SPHRASE_SHEET_H

/* End of sphrase_sheet.h */
