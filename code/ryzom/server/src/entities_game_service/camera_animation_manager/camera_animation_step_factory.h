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

#ifndef RY_CAMERAANIMATIONSTEPFACTORY_H
#define RY_CAMERAANIMATIONSTEPFACTORY_H

#include "nel/ligo/primitive.h"
#include <string>
#include <vector>

#include "camera_animation_manager/camera_animation_modifier_factory.h"

/************************************************************************/
/* Interface for camera animation steps.
 * It has to be able to parse the step from the primitive.
 * And also to generate a small script to send to the client
 */
/************************************************************************/
class ICameraAnimationStep
{
public:
	/// This function is called when it's time to parse the primitive to load the camera animation step
	virtual bool parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename) = 0;
	/// Function that adds a camera animation modifier to this step
	void addModifier(ICameraAnimationModifier* modifier);

	/// Function that returns the duration of the step (in seconds)
	virtual float getDuration() const = 0;
	/// Function that returns the name of the step
	virtual std::string getStepName() const = 0;

	/// Function that sends the animation step the to client
	virtual void sendAnimationStep(NLMISC::CBitMemStream& bms) = 0;

	/// Function that send all information about a step to the client (this includes modifiers)
	void sendAnimationFullStep(NLMISC::CBitMemStream& bms);

protected:
	// The list of modifiers
	std::vector<ICameraAnimationModifier*> Modifiers;
};

/************************************************************************/
/* Factory class that can instanciate the correct camera animation step handler.
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class ICameraAnimationStepFactory
{
public:
	/// Function that will instanciate the correct camera animation step
	static ICameraAnimationStep* parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename, const std::string& stepType);
protected:

	/// Functions used to be able to create the camera animation steps
	static void init();
	virtual ICameraAnimationStep * instanciate() = 0;
	static std::vector<std::pair<std::string, ICameraAnimationStepFactory*> >* Entries;
};

// Define used to register the different types of camera animation steps
#define CAMERA_ANIMATION_REGISTER_STEP(_class_,_name_) \
class _class_##CameraAnimationStepFactory : public ICameraAnimationStepFactory \
{\
public:\
	_class_##CameraAnimationStepFactory()\
{\
	init();\
	std::string str = std::string(_name_); \
	for (uint i = 0; i < (*Entries).size(); i++ ) \
{\
	if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
}\
	(*Entries).push_back( std::make_pair( str, this ) );\
}\
	ICameraAnimationStep * instanciate()\
{ \
	return new _class_;\
} \
};\
	static _class_##CameraAnimationStepFactory* _class_##CameraAnimationStepFactoryInstance = new _class_##CameraAnimationStepFactory;

#define CAMERA_ANIMATION_STEP_NAME(name) \
std::string getStepName() const \
{ \
	return name; \
}

#endif /* RY_CAMERAANIMATIONSTEPFACTORY_H */
