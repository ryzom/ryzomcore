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

#include "camera_animation_manager/camera_animation_modifier_factory.h"
#include "game_share/position_or_entity_type.h"

#include "nel/misc/sheet_id.h"

/////////////////////////////////////////////////////////////////////////////
/// This animation modifier shakes the camera. The parameter is
/// - strength
class CCameraAnimationModifierShake : public ICameraAnimationModifier
{
protected:
	float Strength;

public:
	CCameraAnimationModifierShake()
	{
		Strength = 0.f;
	}

	virtual bool parseModifier(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		std::string value;

		// We get the strength
		if (!prim->getPropertyByName("strength", value))
		{
			nlwarning("<CCameraAnimationModifierShake parseModifier> impossible to get the strength property of the basic modifier in primitive : %s", filename.c_str());
			return false;
		}
		if (!NLMISC::fromString(value, Strength))
		{
			nlwarning("<CCameraAnimationModifierShake parseModifier> impossible to convert the string : %s, in float in the basic modifier in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}

		return true;
	}

	virtual void sendCameraModifier(NLMISC::CBitMemStream& bms)
	{
		bms.serial(const_cast<float&>(Strength));
	}

	CAMERA_ANIMATION_MODIFIER_NAME("camera_modifier_shake");
};
CAMERA_ANIMATION_REGISTER_MODIFIER(CCameraAnimationModifierShake, "camera_modifier_shake");

/////////////////////////////////////////////////////////////////////////////
/// This animation modifier plays a sound. The parameters are
/// - sound_name
/// - sound_position
class CCameraAnimationModifierSoundTrigger : public ICameraAnimationModifier
{
protected:
	TPositionOrEntity SoundPos;
	NLMISC::CSheetId SoundId;

public:
	CCameraAnimationModifierSoundTrigger()
	{
		SoundPos = "";
		SoundId = NLMISC::CSheetId::Unknown;
	}

	virtual bool parseModifier(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		std::string value;

		// We get the sound name
		if (!prim->getPropertyByName("sound_name", value))
		{
			nlwarning("<CCameraAnimationModifierSoundTrigger parseModifier> impossible to get the sound_name property of the basic modifier in primitive : %s", filename.c_str());
			return false;
		}
		SoundId = NLMISC::CSheetId(value);
		if (SoundId == NLMISC::CSheetId::Unknown)
		{
			nlwarning("<CCameraAnimationModifierSoundTrigger parseModifier> sheetid not found for sound name %s in the basic modifier in primitive : %s", value.c_str(), filename.c_str());
			return false;
		}
		// We get the sound position
		if (!prim->getPropertyByName("sound_position", value))
		{
			nlwarning("<CCameraAnimationModifierSoundTrigger parseModifier> impossible to get the sound_position property of the basic modifier in primitive : %s", filename.c_str());
			return false;
		}
		SoundPos = value;

		return true;
	}

	virtual void sendCameraModifier(NLMISC::CBitMemStream& bms)
	{
		bms.serial(const_cast<TPositionOrEntity&>(SoundPos));
		bms.serial(const_cast<NLMISC::CSheetId&>(SoundId));
	}

	CAMERA_ANIMATION_MODIFIER_NAME("sound_trigger");
};
CAMERA_ANIMATION_REGISTER_MODIFIER(CCameraAnimationModifierSoundTrigger, "sound_trigger");