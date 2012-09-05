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

#ifndef RY_POSITIONORENTITYTYPEHELPER_H
#define RY_POSITIONORENTITYTYPEHELPER_H


#include "game_share/position_or_entity_type.h"



//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class that can contain either an entity id or a position             */
/************************************************************************/ 
class CPositionOrEntityHelper: public CPositionOrEntity
{
public:

	CPositionOrEntityHelper(): CPositionOrEntity()
	{
	}

	CPositionOrEntityHelper(const NLMISC::CVector& position): CPositionOrEntity(position)
	{
	}

	CPositionOrEntityHelper(const TDataSetIndex& eid): CPositionOrEntity(eid)
	{
	}

	CPositionOrEntityHelper(PositionOrEntityType type): CPositionOrEntity(type)
	{
	}

	CPositionOrEntityHelper(const CPositionOrEntity& c): CPositionOrEntity(c)
	{
	}
	
	/************************************************************************/
	/* Creates a PositionOrEntity instance from a string                    */
	/************************************************************************/
	static CPositionOrEntityHelper fromString(const std::string& s);

	static const CPositionOrEntityHelper Invalid;

	// Declares abstract methods
	/// This function returns the difference between the player's position and the specified position
	NLMISC::CVector getDiffPos(const NLMISC::CVector& targetPos) const;

	/// This function returns the target position given the difference between the player's position and this target position
	NLMISC::CVector setPositionFromDiffPos(const NLMISC::CVector& diffPos);

};


#endif /* RY_POSITIONORENTITYTYPEHELPER_H */
