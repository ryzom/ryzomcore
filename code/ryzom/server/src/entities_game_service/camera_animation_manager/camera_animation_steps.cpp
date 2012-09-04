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
#include "game_share/position_or_entity_type.h"
#include "camera_animation_manager/position_or_entity_type_helper.h"
#include "game_share/camera_anim_type_parser.h"


/// Basic camera animation step that has generic values
class CCameraAnimationStep : public ICameraAnimationStep
{
protected:
	std::string LookAtTarget;
	float DirectionTransitionTime;
	std::string PositionTarget;
	float PositionTransitionTime;
	float DistanceTo;
	bool HasTurnAround;
	float TurnAroundSpeed;

	std::string Text;
	float Duration;

public:
	CCameraAnimationStep()
	{
		LookAtTarget = "";
		DirectionTransitionTime = 0.f;
		PositionTarget = "";
		PositionTransitionTime = 0.f;
		DistanceTo = 0.f;
		HasTurnAround = false;
		TurnAroundSpeed = 0.f;
		Text = "";
		Duration = 0.f;
	}

	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		std::string value;

		// We get the look at target
		if (!prim->getPropertyByName("look_at_target", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the look_at_target property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		LookAtTarget = value;

		// We get the direction_transition_time
		if (!prim->getPropertyByName("direction_transition_time", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the direction_transition_time property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, DirectionTransitionTime))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		// We get the position_target
		if (!prim->getPropertyByName("position_target", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the position_target property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		PositionTarget = value;

		// We get the position_transition_time
		if (!prim->getPropertyByName("position_transition_time", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the position_transition_time property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, PositionTransitionTime))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		// We get the distance_to
		if (!prim->getPropertyByName("distance_to", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the distance_to property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, DistanceTo))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		// We get the has_turn_around
		if (!prim->getPropertyByName("has_turn_around", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the has_turn_around property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, HasTurnAround))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in bool in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		// We get the turn_around_speed
		if (!prim->getPropertyByName("turn_around_speed", value))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to get the turn_around_speed property of the basic step in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, TurnAroundSpeed))
		{
			nlwarning("<CCameraAnimationStepBasic parseStep> impossible to convert the string : %s, in float in the basic step in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

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
		TPositionOrEntity posLookAtTarget = CPositionOrEntityHelper::fromString(LookAtTarget);
		if (posLookAtTarget == CPositionOrEntityHelper::Invalid)
		{
			nlerror("<CCameraAnimationStepBasic parseStep> invalid LookAtTarget %s", LookAtTarget.c_str());
		}

		bms.serial(const_cast<TPositionOrEntity&>(posLookAtTarget));
		if (!posLookAtTarget.isPreviousPos())
		{
			NLMISC::serialDuration(bms, DirectionTransitionTime);
		}

		TPositionOrEntity posPosTarget = CPositionOrEntityHelper::fromString(PositionTarget);
		if (posPosTarget == CPositionOrEntityHelper::Invalid)
		{
			nlerror("<CCameraAnimationStepBasic parseStep> invalid PositionTarget %s", PositionTarget.c_str());
		}

		bms.serial(const_cast<TPositionOrEntity&>(posPosTarget));
		if (!posPosTarget.isPreviousPos())
		{
			NLMISC::serialDistance(bms, DistanceTo);
			NLMISC::serialDuration(bms, PositionTransitionTime);
			bms.serialBit(const_cast<bool&>(HasTurnAround));

			if (HasTurnAround)
			{
				NLMISC::serialSpeed(bms, TurnAroundSpeed);
			}
		}
	}

	virtual float getDuration() const
	{
		return Duration;
	}

	CAMERA_ANIMATION_STEP_NAME("camera_animation_step");
};
CAMERA_ANIMATION_REGISTER_STEP(CCameraAnimationStep, "camera_animation_step");
