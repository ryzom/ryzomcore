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

/// Basic camera animation step that has generic values
/// - name
/// - look_at_position
/// - text
/// - duration
class CCameraAnimationStepBasic : public ICameraAnimationStep
{
protected:
	std::string LookAtPos;
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
}; // This class must not be registered because it a base class

/// Static camera animation step (that does not have specific variables)
class CCameraAnimationStepStatic : public CCameraAnimationStepBasic
{
public:
	bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename)
	{
		if (!CCameraAnimationStepBasic::parseStep(prim, filename))
		{
			nlwarning("<CCameraAnimationStepStatic parseStep> impossible to parse the basic part of the step in primitive : %s", filename.c_str());
			return false;
		}
		return true;
	}
};
CAMERA_ANIMATION_REGISTR_STEP(CCameraAnimationStepStatic, "camera_animation_static");
