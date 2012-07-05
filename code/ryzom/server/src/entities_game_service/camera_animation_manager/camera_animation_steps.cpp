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
// 

#include "camera_animation_manager/camera_animation_step_factory.h"

typedef std::string TPositionOrEntity;

/// Basic camera animation step that has generic values
/// - look_at_position
/// - text
/// - duration
class CCameraAnimationStepBasic : public ICameraAnimationStep
{
protected:
	TPositionOrEntity LookAtPos;
	std::string Text;
	float Duration;

public:
	CCameraAnimationStepBasic()
	{
		LookAtPos = "";
		Text = "";
		Duration = 0.f;
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		std::string value;

		// We get the look at position
		if (!prim->getPropertyByName("look_at_position", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the look_at_position property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		LookAtPos = value;

		// We get the text
		if (!prim->getPropertyByName("text", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the text property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		Text = value;

		// We get the duration
		if (!prim->getPropertyByName("duration", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the duration property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, Duration))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{
		bms.serial(const_cast<TPositionOrEntity&>(LookAtPos));
		bms.serial(const_cast<std::string&>(Text));
		bms.serial(const_cast<float&>(Duration));
	}

	virtual float getDuration() const
	{
		return Duration;
	}
}; // This class must not be registered because it a base class

/////////////////////////////////////////////////////////////////////////////
/// Static camera animation step (that does not have specific variables)
class CCameraAnimationStepStatic : public CCameraAnimationStepBasic
{
public:
	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		if (!CCameraAnimationStepBasic::parseStep(prim, filename))
		{
			nlwarning("<CCameraAnimationStepStatic parseStep> impossible to parse the basic part of the step in primitive : %s", filename.c_str());
			return false;
		}
		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{
		CCameraAnimationStepBasic::sendAnimationStep(bms);
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepStatic, "camera_animation_static");

/////////////////////////////////////////////////////////////////////////////
/// Go to camera animation step
/// - end_position
class CCameraAnimationStepGoTo: public CCameraAnimationStepBasic
{
protected:
	TPositionOrEntity EndPos;

public:
	CCameraAnimationStepGoTo()
	{
		EndPos = "";
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		if (!CCameraAnimationStepBasic::parseStep(prim, filename))
		{
			nlwarning("<CCameraAnimationStepGoTo parseStep> impossible to parse the basic part of the step in primitive : %s", filename.c_str());
			return false;
		}

		std::string value;

		// We get the end position
		if (!prim->getPropertyByName("end_position", value))
		{
			nlwarning("<CCameraAnimationStepGoTo parseStep> impossible to get the end_position property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		EndPos = value;

		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{
		CCameraAnimationStepBasic::sendAnimationStep(bms);
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepGoTo, "camera_animation_go_to");

/////////////////////////////////////////////////////////////////////////////
/// Follow entity camera animation step
/// - entity_to_follow
/// - distance_to_entity
class CCameraAnimationStepFollowEntity: public CCameraAnimationStepBasic
{
protected:
	TPositionOrEntity EntityToFollow;
	float DistanceToEntity;

public:
	CCameraAnimationStepFollowEntity()
	{
		EntityToFollow = "";
		DistanceToEntity = 0.f;
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		if (!CCameraAnimationStepBasic::parseStep(prim, filename))
		{
			nlwarning("<CCameraAnimationStepFollowEntity parseStep> impossible to parse the basic part of the step in primitive : %s", filename.c_str());
			return false;
		}

		std::string value;

		// We get the entity to follow
		if (!prim->getPropertyByName("entity_to_follow", value))
		{
			nlwarning("<CCameraAnimationStepFollowEntity parseStep> impossible to get the entity_to_follow property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		EntityToFollow = value;

		// We get the distance to the entity
		if (!prim->getPropertyByName("distance_to_entity", value))
		{
			nlwarning("<CCameraAnimationStepFollowEntity parseStep> impossible to get the distance_to_entity property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, DistanceToEntity))
		{
			nlwarning("<CCameraAnimationStepFollowEntity parseStep> impossible to convert the string : %s, in float in the follow entity step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{
		CCameraAnimationStepBasic::sendAnimationStep(bms);
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepFollowEntity, "camera_animation_follow_entity");

/////////////////////////////////////////////////////////////////////////////
/// Turn around camera animation step
/// - point_to_turn_around
/// - distance_to_point
/// - speed
class CCameraAnimationStepTurnAround: public CCameraAnimationStepBasic
{
protected:
	TPositionOrEntity PointToTurnAround;
	float DistanceToPoint;
	float Speed;

public:
	CCameraAnimationStepTurnAround()
	{
		PointToTurnAround = "";
		DistanceToPoint = 0.f;
		Speed = 0.f;
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		if (!CCameraAnimationStepBasic::parseStep(prim, filename))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to parse the basic part of the step in primitive : %s", filename.c_str());
			return false;
		}

		std::string value;

		// We get the point to turn around
		if (!prim->getPropertyByName("point_to_turn_around", value))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to get the point_to_turn_around property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		PointToTurnAround = value;

		// We get the distance to the point
		if (!prim->getPropertyByName("distance_to_point", value))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to get the distance_to_point property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, DistanceToPoint))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to convert the string : %s, in float in the follow entity step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		// We get the speed
		if (!prim->getPropertyByName("speed", value))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to get the speed property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, Speed))
		{
			nlwarning("<CCameraAnimationStepTurnAround parseStep> impossible to convert the string : %s, in float in the follow entity step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{
		CCameraAnimationStepBasic::sendAnimationStep(bms);
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepTurnAround, "camera_animation_turn_around");

/////////////////////////////////////////////////////////////////////////////
/// Animation step that returns to the starting position. It directly inherits from the interface because it
/// does not need all the parameters of a basic step
/// - duration
class CCameraAnimationStepReturn : public ICameraAnimationStep
{
protected:
	float Duration;

public:
	CCameraAnimationStepReturn()
	{
		Duration = 0.f;
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		std::string value;

		// We get the duration
		if (!prim->getPropertyByName("duration", value))
		{
			nlwarning("<CCameraAnimationStepReturn parseStep> impossible to get the duration property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, Duration))
		{
			nlwarning("<CCameraAnimationStepReturn parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		return true;
	}

	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms)
	{

	}

	virtual float getDuration() const
	{
		return Duration;
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepReturn, "camera_animation_return");