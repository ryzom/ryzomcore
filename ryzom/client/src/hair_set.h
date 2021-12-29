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



#ifndef CL_HAIR_SET_H
#define CL_HAIR_SET_H

#include "game_share/people.h"

namespace NLMISC
{
	class IProgressCallback;
}

/** Records which hair item are valid for one race
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CHairSet
{
public:
	/** Init the hair item after all the sheets have been loaded
	  */
	void init (NLMISC::IProgressCallback &progress);

	// remove all infos about hairs
	void clear();

	/** get the number of hair item for the given race
	  */
	uint getNumHairItem(EGSPD::CPeople::TPeople people) const;

	/** get hair item id in sheet manager for the race
	  * or -1 if index is invalid
	  */
	sint getHairItemId(EGSPD::CPeople::TPeople people, uint index) const;

	// Tells whether this id in the sheet manager  is the id of a hair item
	bool isHairItemId(uint id) const;

	// Gives the people for this hair id in the sheet manager, or unknown if not found
	EGSPD::CPeople::TPeople getPeopleFromHairItemID(uint id) const;


	/** Get the index of an hair item form its id in the sheet manager, or -1 if not found
	  */
	sint getHairItemFromId(EGSPD::CPeople::TPeople people, uint Id) const;

private:
	enum EPeople { Fyros = 0, Matis, Tryker, Zorai, NumPeople, DontKnow };
	typedef std::vector<uint> TIntArray;
private:
	TIntArray _Hairs[NumPeople]; // 4 races
private:
	static EPeople convPeople(EGSPD::CPeople::TPeople people);
	static EGSPD::CPeople::TPeople convPeople(EPeople people);

};


// the only instance of hair_set

extern CHairSet HairSet;

#endif
