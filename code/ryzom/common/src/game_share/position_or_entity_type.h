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

#ifndef RY_POSITIONORENTITYTYPE_H
#define RY_POSITIONORENTITYTYPE_H

#include "nel/misc/entity_id.h"
#include "nel/misc/vector.h"
#include "nel/misc/stream.h"

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class that can contain either an entity id or a position             */
/************************************************************************/ 
class TPositionOrEntity
{
public:
	TPositionOrEntity()
	{
		_isPosition = -1;
	}

	TPositionOrEntity(const NLMISC::CVector& position)
	{
		_isPosition = 1;
		Position = position;
	}

	TPositionOrEntity(const TDataSetIndex& eid)
	{
		_isPosition = 0;
		EntityId = eid;
	}

	TPositionOrEntity(const TPositionOrEntity& c)
	{
		_isPosition = c._isPosition;
		if (c.isPosition())
			Position = c.getPosition();
		else
			EntityId = c.getEntityId();
	}

	TPositionOrEntity& operator=(const TPositionOrEntity& c)
	{
		_isPosition = c._isPosition;
		if (c.isPosition())
			Position = c.getPosition();
		else if (c.isEntityId())
			EntityId = c.getEntityId();

		return *this;
	}

	bool operator==(const TPositionOrEntity& c)
	{
		if (_isPosition != c._isPosition)
			return false;
		if (isPosition())
			return Position == c.getPosition();
		else if (isEntityId())
			return EntityId == c.getEntityId();
		return true;
	}

	void setPosition(const NLMISC::CVector& position)
	{
		_isPosition = 1;
		Position = position;
	}

	void setEntityId(const TDataSetIndex& eid)
	{
		_isPosition = 0;
		EntityId = eid;
	}

	bool isPosition() const
	{
		return _isPosition == 1;
	}

	bool isEntityId() const
	{
		return _isPosition == 0;
	}

	NLMISC::CVector getPosition() const
	{
		if (!isPosition())
			return NLMISC::CVector();
		return Position;
	}

	TDataSetIndex getEntityId() const
	{
		if (!isEntityId())
			return TDataSetIndex();
		return EntityId;
	}

	bool isValid() const
	{
		return isPosition() || isEntityId();
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(_isPosition);
		if (isPosition())
			f.serial(Position);
		else if (isEntityId())
			f.serial(EntityId);
	}

private:
	char _isPosition;
	NLMISC::CVector	Position;
	TDataSetIndex EntityId;
};


#endif /* RY_POSITIONORENTITYTYPE_H */
