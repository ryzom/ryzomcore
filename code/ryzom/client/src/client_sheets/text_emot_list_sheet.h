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

#ifndef CL_TEXT_EMOT_LIST_SHEET_H
#define CL_TEXT_EMOT_LIST_SHEET_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"

#include "entity_sheet.h"
#include "animation_set_list_sheet.h"

/////////////
// CLASSES //
/////////////
/**
 * List of emots (text + anim)
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date April 2004
 */
class CTextEmotListSheet : public CEntitySheet
{
public:

	struct STextEmot
	{
		std::string Path;
		std::string Anim;
		std::string EmoteId;
		std::string FXToSpawn;
		float		FXSpawnDelay;
		float		FXSpawnDist;
		bool		UsableFromClientUI;

		STextEmot()
		{
			FXSpawnDelay = 0.f;
			FXSpawnDist  = 0.5f;
			UsableFromClientUI = true;
		}

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Path);
			f.serial(Anim);
			f.serial(EmoteId);
			f.serial(FXToSpawn);
			f.serial(FXSpawnDelay);
			f.serial(FXSpawnDist);
			f.serial(UsableFromClientUI);
		}
	};

	std::vector<STextEmot> TextEmotList;

public:
	/// Constructor
	CTextEmotListSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


#endif // CL_TEXT_EMOT_LIST_SHEET_H

/* End of text_emot_list_sheet.h */
