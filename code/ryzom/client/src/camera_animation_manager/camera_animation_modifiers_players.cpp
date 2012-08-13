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

#include "camera_animation_manager/camera_animation_modifier_player_factory.h"
#include "game_share/position_or_entity_type.h"
#include "camera_animation_manager/camera_animation_info.h"
#include "camera_animation_manager/position_or_entity_pos_resolver.h"
#include "global.h"
#include "sound_manager.h"
#include "nel/misc/random.h"
#include "nel/misc/time_nl.h"
#include "time_client.h"

/////////////////////////////////////////////////////////////////////////////
/// This animation modifier shakes the camera. The parameter is
/// - strength
class CCameraAnimationModifierPlayerShake : public ICameraAnimationModifierPlayer
{
protected:
	float Strength;

	float _Dir;
	float _WaitTime;
	float _CurrTime;
	NLMISC::CVector _CurrVec;

public:
	CCameraAnimationModifierPlayerShake()
	{
		Strength = 0.f;

		_Dir = 1.f;
		_CurrTime = 0.f;
		_WaitTime = 0.f;
	}

	/// This function is called when it's time to init the modifier from an impulse
	virtual bool initModifier(NLMISC::CBitMemStream& impulse)
	{
		impulse.serial(const_cast<float&>(Strength));

		_Dir = 1.f;
		_CurrTime = 0.f;
		_WaitTime = 0.f;

		return true;
	}

	/// Function that plays the modifier
	virtual TCameraAnimationOutputInfo updateModifier(const TCameraAnimationInputInfo& currCamInfo)
	{
		TCameraAnimationOutputInfo output;

		_CurrTime += DT;

		if (_CurrTime >= _WaitTime)
		{
			/*NLMISC::CRandom rnd;
			rnd.srand((sint32)NLMISC::CTime::getLocalTime());*/

			// We find a normal vector to the look at dir
			NLMISC::CVector newVec(currCamInfo.CamLookAtDir.y * -1.f, currCamInfo.CamLookAtDir.x, currCamInfo.CamLookAtDir.z);
			newVec.normalize();

			newVec = newVec * (Strength * _Dir);
			_Dir *= -1.f;

			_CurrVec = newVec;

			_CurrTime = 0.f;
			_WaitTime = 0.1f;
		}

		output.CamLookAtDir = currCamInfo.CamLookAtDir;
		output.CamPos = currCamInfo.CamPos + _CurrVec;

		return output;
	}

	virtual void stopModifier()
	{
	}
};
CAMERA_ANIMATION_REGISTER_MODIFIER_PLAYER(CCameraAnimationModifierPlayerShake, "camera_modifier_shake");

/////////////////////////////////////////////////////////////////////////////
/// This animation modifier plays a sound. The parameters are
/// - sound_name
/// - sound_position
class CCameraAnimationModifierPlayerSoundTrigger : public ICameraAnimationModifierPlayer
{
protected:
	TPositionOrEntity SoundPos;
	NLMISC::CSheetId SoundId;

public:
	CCameraAnimationModifierPlayerSoundTrigger()
	{
		SoundId = NLMISC::CSheetId::Unknown;
	}

	/// This function is called when it's time to init the modifier from an impulse
	virtual bool initModifier(NLMISC::CBitMemStream& impulse)
	{
		impulse.serial(const_cast<TPositionOrEntity&>(SoundPos));
		impulse.serial(const_cast<NLMISC::CSheetId&>(SoundId));

		// We play the sound once here
		NLMISC::CVector pos = resolvePositionOrEntityPosition(SoundPos);
		SoundMngr->spawnSource(SoundId, pos);

		return true;
	}

	/// Function that plays the modifier
	virtual TCameraAnimationOutputInfo updateModifier(const TCameraAnimationInputInfo& currCamInfo)
	{
		return currCamInfo.toOutput();
	}

	virtual void stopModifier()
	{
	}
};
CAMERA_ANIMATION_REGISTER_MODIFIER_PLAYER(CCameraAnimationModifierPlayerSoundTrigger, "sound_trigger");