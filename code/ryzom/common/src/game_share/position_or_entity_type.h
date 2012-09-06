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
#include "game_share/camera_anim_type_parser.h"

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* Class that can contain either an entity id or a position             */
/************************************************************************/ 
class CPositionOrEntity
{
public:
	enum PositionOrEntityType
	{
		EUnknown = -1,
		EPosition = 0,
		EEntity,
		EPreviousPos,
		EReturnPos
	};

	CPositionOrEntity()
	{
		_type = EUnknown;
	}

	CPositionOrEntity(const NLMISC::CVector& position)
	{
		_type = EPosition;
		Position = position;
	}

	CPositionOrEntity(const TDataSetIndex& eid)
	{
		_type = EEntity;
		EntityId = eid;
	}

	CPositionOrEntity(PositionOrEntityType type)
	{
		_type = type;
	}

	CPositionOrEntity(const CPositionOrEntity& c)
	{
		_type = c._type;
		if (c.isPosition())
			Position = c.getPosition();
		else
			EntityId = c.getEntityId();
	}

	CPositionOrEntity& operator=(const CPositionOrEntity& c)
	{
		_type = c._type;
		if (c.isPosition())
			Position = c.getPosition();
		else if (c.isEntityId())
			EntityId = c.getEntityId();

		return *this;
	}

	bool operator==(const CPositionOrEntity& c)
	{
		if (_type != c._type)
			return false;
		if (isPosition())
			return Position == c.getPosition();
		else if (isEntityId())
			return EntityId == c.getEntityId();
		return true;
	}

	void setPosition(const NLMISC::CVector& position)
	{
		_type = EPosition;
		Position = position;
	}

	void setEntityId(const TDataSetIndex& eid)
	{
		_type = EEntity;
		EntityId = eid;
	}

	void setPreviousPos()
	{
		_type = EPreviousPos;
	}

	void setReturnPos()
	{
		_type = EReturnPos;
	}

	bool isPosition() const
	{
		return _type == EPosition;
	}

	bool isEntityId() const
	{
		return _type == EEntity;
	}

	bool isPreviousPos() const
	{
		return _type == EPreviousPos;
	}

	bool isReturnPos() const
	{
		return _type == EReturnPos;
	}

	NLMISC::CVector getPosition() const
	{
		if (!isPosition())
			return NLMISC::CVector();
		return Position;
	}

	/// This function returns the difference between the player's position and the specified position
	/*virtual NLMISC::CVector getDiffPos(const NLMISC::CVector& targetPos) const = 0;

	/// This function returns the target position given the difference between the player's position and this target position
	virtual NLMISC::CVector setPositionFromDiffPos(const NLMISC::CVector& diffPos) = 0;*/

	TDataSetIndex getEntityId() const
	{
		if (!isEntityId())
			return TDataSetIndex();
		return EntityId;
	}

	bool isValid() const
	{
		return _type != EUnknown;
	}

	void serial(NLMISC::CBitMemStream &f)
	{
		if (f.isReading())
		{
			uint32 t = 0;
			f.serial(t, 2);
			_type = (PositionOrEntityType)t;
		}
		else
		{
			uint32 t = (uint32)_type;
			f.serial(t, 2);
		}
		if (isPosition())
		{
			if (f.isReading())
			{
				/*NLMISC::CVector diffPos = NLMISC::CVector();
				NLMISC::serialPositionDifference(f, diffPos);
				setPositionFromDiffPos(diffPos);*/
				f.serial(Position);
			}
			else
			{
				/*NLMISC::CVector diffPos = getDiffPos(getPosition());
				NLMISC::serialPositionDifference(f, diffPos);*/
				f.serial(Position);
			}
		}
		else if (isEntityId())
			f.serial(EntityId);
	}

protected:
	PositionOrEntityType _type;
	NLMISC::CVector	Position;
	TDataSetIndex EntityId;
};


#endif /* RY_POSITIONORENTITYTYPE_H */
