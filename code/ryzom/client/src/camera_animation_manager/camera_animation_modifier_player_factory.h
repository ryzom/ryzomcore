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

#ifndef RY_CAMERAANIMATIONMODIFIERPLAYERFACTORY_H
#define RY_CAMERAANIMATIONMODIFIERPLAYERFACTORY_H

#include <string>
#include <vector>
#include "nel/misc/bit_mem_stream.h"
#include "camera_animation_manager/camera_animation_info.h"

/************************************************************************/
/* Interface for camera animation modifiers.
 * It has to be able to parse the modifier from an impulse and to play it
 */
/************************************************************************/
class ICameraAnimationModifierPlayer
{
public:
	/// This function is called when it's time to init the modifier from an impulse
	virtual bool initModifier(NLMISC::CBitMemStream& impulse) = 0;

	/// Function that updates the modifier
	/// currCamInfo contains information about the current camera position and look at position
	/// The function must return the new camera information
	virtual TCameraAnimationOutputInfo updateModifier(const TCameraAnimationInputInfo& currCamInfo) = 0;

	/// Function called when the modifier is stopped
	virtual void stopModifier() = 0;

protected:
};

/************************************************************************/
/* Factory class that can instanciate the correct camera animation modifier player.
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class ICameraAnimationModifierPlayerFactory
{
public:
	/// Function that will instanciate the correct camera animation modifier player in function of the modifier name
	static ICameraAnimationModifierPlayer* initModifier(const std::string& name, NLMISC::CBitMemStream& impulse);
protected:

	/// Functions used to be able to create the camera animation modifiers players
	static void init();
	virtual ICameraAnimationModifierPlayer* instanciate() = 0;
	static std::vector<std::pair<std::string, ICameraAnimationModifierPlayerFactory*> >* Entries;
};

// Define used to register the different types of camera animation modifiers players
#define CAMERA_ANIMATION_REGISTER_MODIFIER_PLAYER(_class_,_name_) \
class _class_##CameraAnimationModifierPlayerFactory : public ICameraAnimationModifierPlayerFactory \
{\
public:\
	_class_##CameraAnimationModifierPlayerFactory()\
{\
	init();\
	std::string str = std::string(_name_); \
	for (uint i = 0; i < (*Entries).size(); i++ ) \
{\
	if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
}\
	(*Entries).push_back( std::make_pair( str, this ) );\
}\
	ICameraAnimationModifierPlayer* instanciate()\
{ \
	return new _class_;\
} \
};\
	static _class_##CameraAnimationModifierPlayerFactory* _class_##CameraAnimationModifierPlayerFactoryInstance = new _class_##CameraAnimationModifierPlayerFactory;


#endif /* RY_CAMERAANIMATIONMODIFIERPLAYERFACTORY_H */
