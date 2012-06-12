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
};
CAMERA_ANIMATION_REGISTR_MODIFIER(CCameraAnimationModifierShake, "camera_modifier_shake");

/////////////////////////////////////////////////////////////////////////////
/// This animation modifier plays a sound. The parameters are
/// - sound_name
/// - sound_position
class CCameraAnimationModifierSoundTrigger : public ICameraAnimationModifier
{
protected:
	std::string SoundName;
	std::string SoundPos;

public:
	CCameraAnimationModifierSoundTrigger()
	{
		SoundName = "";
		SoundPos = "";
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
		SoundName = value;
		// We get the sound position
		if (!prim->getPropertyByName("sound_position", value))
		{
			nlwarning("<CCameraAnimationModifierSoundTrigger parseModifier> impossible to get the sound_position property of the basic modifier in primitive : %s", filename.c_str());
			return false;
		}
		SoundPos = value;

		return true;
	}
};
CAMERA_ANIMATION_REGISTR_MODIFIER(CCameraAnimationModifierSoundTrigger, "sound_trigger");