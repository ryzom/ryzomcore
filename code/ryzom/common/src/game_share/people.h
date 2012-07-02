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

#ifndef RY_PEOPLE_H
#define RY_PEOPLE_H


#include "body.h"
#include "people_pd.h"

namespace EGSPD
{


inline bool isPlayableRace(CPeople::TPeople people)
{
	return ( people >= CPeople::Playable && people < CPeople::EndPlayable );
}
inline bool isHumanoid(CPeople::TPeople people)
{
	return ( people >= CPeople::Humanoid && people < CPeople::EndHumanoid );
}

inline bool isCreature(CPeople::TPeople people)
{
	return ( people >= CPeople::Creature && people < CPeople::EndCreature );
}


inline bool isFauna(CPeople::TPeople people)
{
	return ( people >= CPeople::Fauna && people < CPeople::EndFauna );
}

inline bool isFlora(CPeople::TPeople people)
{
	return ( people >= CPeople::Flora && people < CPeople::EndFlora );
}

inline bool isGoo(CPeople::TPeople people)
{
	return ( people >= CPeople::Goo && people < CPeople::EndGoo );
}

/// get body type of given race
BODY::TBodyType getBodyType(CPeople::TPeople people);


/// test if given race match classification type
bool testClassificationType(CPeople::TPeople people, CClassificationType::TClassificationType type);

/// get all matching classification types for given race, fill types with mathcing types
void getMatchingClassificationType(CPeople::TPeople people, std::vector<CClassificationType::TClassificationType> &types);

inline const char* getFameFromPeople(CPeople::TPeople people)
{
	switch(people)
	{
	case CPeople::Fyros:		return "fyros";
	case CPeople::Matis:		return "matis";
	case CPeople::Tryker:		return "tryker";
	case CPeople::Zorai:		return "zorai";
	default:					break;
	}
	return "invalid";
}

inline uint8 getCivilisationId(CPeople::TPeople people)
{
	if ( people >= CPeople::Playable && people < CPeople::EndPlayable )
		return ( uint8 ) people;
	return 0xFF;
}

/// Return the pet type, 1 for Mount, 2 for Packer (otherwise 0)
inline uint	getPetType( CPeople::TPeople peopleType )
{
 	switch ( peopleType )
 	{
 	case CPeople::MektoubPacker: return 2;
 	case CPeople::MektoubMount: return 1;
 	default: return 0;
 	}
}


};// namespace GSPEOPLE





#endif // RY_PEOPLE_H

/* End of people.h */
