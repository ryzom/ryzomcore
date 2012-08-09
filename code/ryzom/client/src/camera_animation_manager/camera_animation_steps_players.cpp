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

#include "camera_animation_manager/camera_animation_step_player_factory.h"
#include "game_share/position_or_entity_type.h"
#include "camera_animation_manager/camera_animation_info.h"
#include "camera_animation_manager/position_or_entity_pos_resolver.h"


/// Basic camera animation step that has generic values
/// - look_at_position
/// - text
/// - duration
class CCameraAnimationStepPlayerBasic : public ICameraAnimationStepPlayer
{
protected:
	TPositionOrEntity LookAtPos;
	std::string Text;
	float Duration;

public:
	CCameraAnimationStepPlayerBasic()
	{
		Text = "";
		Duration = 0.f;
	}

	virtual float getDuration() const
	{
		return Duration;
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		impulse.serial(const_cast<TPositionOrEntity&>(LookAtPos));
		impulse.serial(const_cast<std::string&>(Text));
		impulse.serial(const_cast<float&>(Duration));
		
		return true;
	}

	virtual void stopStep()
	{
	}

}; // This class must not be registered because it's a base class

/////////////////////////////////////////////////////////////////////////////
/// Static camera animation step (that does not have specific variables)
class CCameraAnimationStepPlayerStatic : public CCameraAnimationStepPlayerBasic
{
public:
	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		CCameraAnimationStepPlayerBasic::initStep(impulse);
		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationInfo updateStep(const TCameraAnimationInfo& currCamInfo)
	{
		TCameraAnimationInfo camInfo;

		// We don't change the position
		camInfo.CamPos = currCamInfo.CamPos;

		float ratio = currCamInfo.ElapsedTimeSinceStartStep / getDuration();

		// We compute the starting look at direction
		NLMISC::CVector startDir = currCamInfo.CamLookAtDir - currCamInfo.CamPos;

		// We compute the final look at direction
		NLMISC::CVector finalDir = resolvePositionOrEntityPosition(LookAtPos) - currCamInfo.CamPos;

		// We get the current look at direction
		camInfo.CamLookAtDir = computeCurrentLookAtDir(ratio, camInfo.CamPos, startDir, finalDir);

		return camInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerStatic, "camera_animation_static");

/////////////////////////////////////////////////////////////////////////////
/// Go to camera animation step
/// - end_position
class CCameraAnimationStepPlayerGoTo: public CCameraAnimationStepPlayerBasic
{
protected:
	TPositionOrEntity EndPos;

public:
	CCameraAnimationStepPlayerGoTo()
	{
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		CCameraAnimationStepPlayerBasic::initStep(impulse);

		impulse.serial(const_cast<TPositionOrEntity&>(EndPos));

		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationInfo updateStep(const TCameraAnimationInfo& currCamInfo)
	{
		return currCamInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerGoTo, "camera_animation_go_to");

/////////////////////////////////////////////////////////////////////////////
/// Follow entity camera animation step
/// - entity_to_follow
/// - distance_to_entity
class CCameraAnimationStepPlayerFollowEntity: public CCameraAnimationStepPlayerBasic
{
protected:
	TPositionOrEntity EntityToFollow;
	float DistanceToEntity;

public:
	CCameraAnimationStepPlayerFollowEntity()
	{
		DistanceToEntity = 0.f;
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		CCameraAnimationStepPlayerBasic::initStep(impulse);

		impulse.serial(const_cast<TPositionOrEntity&>(EntityToFollow));
		impulse.serial(const_cast<float&>(DistanceToEntity));

		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationInfo updateStep(const TCameraAnimationInfo& currCamInfo)
	{
		return currCamInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerFollowEntity, "camera_animation_follow_entity");

/////////////////////////////////////////////////////////////////////////////
/// Turn around camera animation step
/// - point_to_turn_around
/// - distance_to_point
/// - speed
class CCameraAnimationStepPlayerTurnAround: public CCameraAnimationStepPlayerBasic
{
protected:
	TPositionOrEntity PointToTurnAround;
	float DistanceToPoint;
	float Speed;

public:
	CCameraAnimationStepPlayerTurnAround()
	{
		DistanceToPoint = 0.f;
		Speed = 0.f;
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		CCameraAnimationStepPlayerBasic::initStep(impulse);

		impulse.serial(const_cast<TPositionOrEntity&>(PointToTurnAround));
		impulse.serial(const_cast<float&>(DistanceToPoint));
		impulse.serial(const_cast<float&>(Speed));

		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationInfo updateStep(const TCameraAnimationInfo& currCamInfo)
	{
		return currCamInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerTurnAround, "camera_animation_turn_around");

/////////////////////////////////////////////////////////////////////////////
/// Animation step that returns to the starting position. It directly inherits from the interface because it
/// does not need all the parameters of a basic step
/// - duration
class CCameraAnimationStepPlayerReturn : public ICameraAnimationStepPlayer
{
protected:
	float Duration;

public:
	CCameraAnimationStepPlayerReturn()
	{
		Duration = 0.f;
	}

	virtual float getDuration() const
	{
		return Duration;
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& impulse)
	{
		impulse.serial(const_cast<float&>(Duration));

		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationInfo updateStep(const TCameraAnimationInfo& currCamInfo)
	{
		return currCamInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerReturn, "camera_animation_return");