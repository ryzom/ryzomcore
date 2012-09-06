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
#include "camera_animation_manager/position_or_entity_helper.h"
#include "camera_animation_manager/camera_animation_info.h"
#include "camera_animation_manager/position_or_entity_pos_resolver.h"
#include "client_cfg.h"
#include "time_client.h"
#include "game_share/camera_anim_type_parser.h"

/// Magic camera animation step
class CCameraAnimationStepPlayerStep: public ICameraAnimationStepPlayer
{
protected:
	CPositionOrEntityHelper LookAtTarget;
	float DirectionTransitionTime;
	CPositionOrEntityHelper PositionTarget;
	float PositionTransitionTime;
	float DistanceTo;
	bool HasTurnAround;
	float TurnAroundSpeed;

public:
	CCameraAnimationStepPlayerStep()
	{
		DirectionTransitionTime = 0.f;
		PositionTransitionTime = 0.f;
		DistanceTo = 0.f;
		HasTurnAround = false;
		TurnAroundSpeed = 0.f;
	}

	/// This function is called when it's time to init the step from an impulse
	virtual bool initStep(NLMISC::CBitMemStream& bms)
	{
		bms.serial(const_cast<CPositionOrEntityHelper&>(LookAtTarget));
		if (!LookAtTarget.isPreviousPos())
		{
			NLMISC::serialDuration(bms, DirectionTransitionTime);
		}

		bms.serial(const_cast<CPositionOrEntityHelper&>(PositionTarget));
		if (!PositionTarget.isPreviousPos())
		{
			NLMISC::serialDistance(bms, DistanceTo);
			NLMISC::serialDuration(bms, PositionTransitionTime);
			bms.serialBit(const_cast<bool&>(HasTurnAround));

			if (HasTurnAround)
			{
				NLMISC::serialSpeed(bms, TurnAroundSpeed);
			}
		}

		return true;
	}

	/// Function that plays the step
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		NLMISC::CVector targetPos = resolvePositionOrEntityPosition(PositionTarget, currCamInfo);

		//////////////////////////////////////////////////////////////////////////
		// Position
		{
			float ratio = 1.f;
			if (PositionTransitionTime > 0.f)
				ratio = currCamInfo.ElapsedTimeSinceStartStep / PositionTransitionTime;
			if (ratio > 1.f)
				ratio = 1.f;

			// We compute the current position between the starting position and the final position
			NLMISC::CVector movementVector = targetPos - currCamInfo.StartCamPos;

			// We substract the distance
			float currDist = movementVector.norm();
			float substractRatio = 0.f;
			if (currDist > 0.f)
				substractRatio = (currDist - DistanceTo) / currDist;
			if ((currDist - DistanceTo) < 0.f)
				substractRatio = 0.f;
			movementVector = movementVector * substractRatio;

			// We current position is computed using the ratio and the starting position
			NLMISC::CVector offset = movementVector * ratio;
			camInfo.CamPos = currCamInfo.StartCamPos + offset;
		}

		//////////////////////////////////////////////////////////////////////////
		// Turn around
		if (HasTurnAround)
		{
			// Now we compute the current angle between our position and the target's position
			NLMISC::CVector2f currTopVector(camInfo.CamPos.x - targetPos.x, camInfo.CamPos.y - targetPos.y);
			float distance2D = currTopVector.norm();
			currTopVector.normalize();

			float angle = acosf(currTopVector.x);
			if (currTopVector.y < 0)
				angle *= -1.f;

			// We compute an angle to travail in function of the speed
			float angleOffset = DT * TurnAroundSpeed;

			float newAngle = angle + angleOffset;

			// We compute the new position for this angle
			NLMISC::CVector2f new2DPos(cosf(newAngle), sinf(newAngle));
			new2DPos = new2DPos * distance2D;

			// We integrate this new position in the world
			NLMISC::CVector newDir(new2DPos.x, new2DPos.y, camInfo.CamPos.z - targetPos.z);

			camInfo.CamPos = targetPos + newDir;
		}

		//////////////////////////////////////////////////////////////////////////
		// Look at target
		{
			float ratio = 1.f;
			if (DirectionTransitionTime > 0.f)
				ratio = currCamInfo.ElapsedTimeSinceStartStep / DirectionTransitionTime;
			if (ratio > 1.f)
				ratio = 1.f;

			// We compute the final look at direction
			NLMISC::CVector finalDir = resolvePositionOrEntityTargetDir(LookAtTarget, currCamInfo, camInfo.CamPos);

			// We get the current look at direction
			camInfo.CamLookAtDir = computeCurrentLookAtDir(ratio, currCamInfo.StartCamLookAtDir, finalDir);
		}
		//////////////////////////////////////////////////////////////////////////

		return camInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerStep, "camera_animation_step");

/*/// Basic camera animation step that has generic values
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
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		// We don't change the position
		camInfo.CamPos = currCamInfo.StartCamPos;

		float ratio = currCamInfo.ElapsedTimeSinceStartStep / getDuration();

		// We compute the final look at direction
		NLMISC::CVector finalDir = resolvePositionOrEntityPosition(LookAtPos) - camInfo.CamPos;

		// We get the current look at direction
		camInfo.CamLookAtDir = computeCurrentLookAtDir(ratio, currCamInfo.StartCamLookAtDir, finalDir);

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
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		float ratio = currCamInfo.ElapsedTimeSinceStartStep / getDuration();

		// We compute the current position between the starting position and the final position
		NLMISC::CVector movementVector = resolvePositionOrEntityPosition(EndPos) - currCamInfo.StartCamPos;

		// If the position is an entity, we substract a minimum distance to the entity
		if (EndPos.isEntityId())
		{
			float currDist = movementVector.norm();
			float substractRatio = (currDist - ClientCfg.CameraAnimMinEntityDistance) / currDist;
			if ((currDist - ClientCfg.CameraAnimMinEntityDistance) < 0.f)
				substractRatio = 0.f;
			movementVector = movementVector * substractRatio;
		}

		// We current position is computed using the ratio and the starting position
		NLMISC::CVector offset = movementVector * ratio;
		camInfo.CamPos = currCamInfo.StartCamPos + offset;

		// Now we compute the current look at direction
		NLMISC::CVector finalDir = resolvePositionOrEntityPosition(LookAtPos) - camInfo.CamPos;

		// We get the current look at direction
		camInfo.CamLookAtDir = computeCurrentLookAtDir(ratio, currCamInfo.StartCamLookAtDir, finalDir);

		return camInfo;
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
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		// We compute the distance between the entity to follow and our current position
		NLMISC::CVector entityPos = resolvePositionOrEntityPosition(EntityToFollow);
		NLMISC::CVector distanceVec = entityPos - currCamInfo.CamPos;

		float distance = distanceVec.norm();

		// We compute the difference with the expected distance
		float difference = distance - DistanceToEntity;

		// We compute the distance we can travel
		float travellingDistance = DT * ClientCfg.MaxCameraAnimationSpeed * (difference < 0.f ? -1.f : 1.f);

		// We check if we are not going to far
		if (abs(travellingDistance) > abs(difference))
			travellingDistance = difference;

		// We normalize the vector from our position to the entity
		distanceVec.normalize();
		// We multiply this vector with the distance to travel
		distanceVec = distanceVec * travellingDistance;

		// We add this vector to our position to know our new position
		camInfo.CamPos = currCamInfo.CamPos + distanceVec;

		// We compute the final look at direction
		NLMISC::CVector finalDir = resolvePositionOrEntityPosition(LookAtPos) - camInfo.CamPos;
		finalDir.normalize();

		// We get the current look at direction
		camInfo.CamLookAtDir = finalDir;

		return camInfo;
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
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		// We compute the distance between the point to turn around and our current position
		NLMISC::CVector pointPos = resolvePositionOrEntityPosition(PointToTurnAround);
		NLMISC::CVector distanceVec = pointPos - currCamInfo.CamPos;

		float distance = distanceVec.norm();

		// We compute the difference with the expected distance
		float difference = distance - DistanceToPoint;

		// We compute the distance we can travel
		float travellingDistance = DT * ClientCfg.MaxCameraAnimationSpeed * (difference < 0.f ? -1.f : 1.f);

		// We check if we are not going to far
		if (abs(travellingDistance) > abs(difference))
			travellingDistance = difference;

		// We normalize the vector from our position to the point
		distanceVec.normalize();
		// We multiply this vector with the distance to travel
		distanceVec = distanceVec * travellingDistance;

		// We add this vector to our position to know our new position
		NLMISC::CVector newPos = currCamInfo.CamPos + distanceVec;

		// Now we compute the current angle between our position and the point's position
		NLMISC::CVector2f currTopVector(newPos.x - pointPos.x, newPos.y - pointPos.y);
		float distance2D = currTopVector.norm();
		currTopVector.normalize();

		float angle = acosf(currTopVector.x);
		if (currTopVector.y < 0)
			angle *= -1.f;

		// We compute an angle to travail in function of the speed
		float angleOffset = DT * Speed;

		float newAngle = angle + angleOffset;

		// We compute the new position for this angle
		NLMISC::CVector2f new2DPos(cosf(newAngle), sinf(newAngle));
		new2DPos = new2DPos * distance2D;

		// We integrate this new position in the world
		NLMISC::CVector newDir(new2DPos.x, new2DPos.y, newPos.z - pointPos.z);

		camInfo.CamPos = pointPos + newDir;

		// We compute the final look at direction
		NLMISC::CVector finalDir = resolvePositionOrEntityPosition(LookAtPos) - camInfo.CamPos;
		finalDir.normalize();

		// We get the current look at direction
		camInfo.CamLookAtDir = finalDir;

		return camInfo;
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
	virtual TCameraAnimationOutputInfo updateStep(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo camInfo;

		float ratio = currCamInfo.ElapsedTimeSinceStartStep / getDuration();

		// We compute the current position between the starting position and the final position
		NLMISC::CVector movementVector = resolvePositionOrEntityPosition(currCamInfo.AnimStartCamPos) - currCamInfo.StartCamPos;

		// We current position is computed using the ratio and the starting position
		NLMISC::CVector offset = movementVector * ratio;
		camInfo.CamPos = currCamInfo.StartCamPos + offset;

		// We get the current look at direction
		camInfo.CamLookAtDir = computeCurrentLookAtDir(ratio, currCamInfo.StartCamLookAtDir, currCamInfo.AnimStartCamLookAtDir);

		return camInfo;
	}

	virtual void stopStep()
	{
	}
};
CAMERA_ANIMATION_REGISTER_STEP_PLAYER(CCameraAnimationStepPlayerReturn, "camera_animation_return");*/