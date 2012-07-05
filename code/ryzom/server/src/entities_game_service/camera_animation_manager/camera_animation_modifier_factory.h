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

#ifndef RY_CAMERAANIMATIONMODIFIERFACTORY_H
#define RY_CAMERAANIMATIONMODIFIERFACTORY_H

#include "nel/ligo/primitive.h"
#include <string>

/************************************************************************/
/* Interface for camera animation modifiers.
 * For simplicity, sounds triggers and considered like modifiers too.
 * It has to be able to parse it's parameters from the primitive.
 * And also to generate a small script to send to the client
 */
/************************************************************************/
class ICameraAnimationModifier
{
public:
	/// This function is called when it's time to parse the primitive to load the camera animation modifier
	virtual bool parseModifier(const NLLIGO::IPrimitive* prim, const std::string& filename) = 0;

	/// Function called to send the modifier to the client
	virtual void sendCameraModifier(NLMISC::CBitMemStream& bms) = 0;

	/// Function to get the name of the modifier
	virtual std::string getModifierName() const = 0;

	/// Function called to send the modifier to the client (including its name)
	void sendCameraFullModifier(NLMISC::CBitMemStream& bms);
};

/************************************************************************/
/* Factory class that can instanciate the correct camera animation modifier handler.
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class ICameraAnimationModifierFactory
{
public:
	/// Function that will instanciate the correct camera animation step
	static ICameraAnimationModifier* parseModifier(const NLLIGO::IPrimitive* prim, const std::string& filename, const std::string& modifierType);	
protected:

	/// Functions used to be able to create the camera animation modifiers
	static void init();
	virtual ICameraAnimationModifier * instanciate() = 0;
	static std::vector<std::pair<std::string, ICameraAnimationModifierFactory*> >* Entries;
};

// Define used to register the different types of camera animation modifiers
#define CAMERA_ANIMATION_REGISTER_MODIFIER(_class_,_name_) \
class _class_##CameraAnimationModifierFactory : public ICameraAnimationModifierFactory \
{\
public:\
	_class_##CameraAnimationModifierFactory()\
{\
	init();\
	std::string str = std::string(_name_); \
	for (uint i = 0; i < (*Entries).size(); i++ ) \
{\
	if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
}\
	(*Entries).push_back( std::make_pair( str, this ) );\
}\
	ICameraAnimationModifier * instanciate()\
{ \
	return new _class_;\
} \
};\
	static _class_##CameraAnimationModifierFactory* _class_##CameraAnimationModifierFactoryInstance = new _class_##CameraAnimationModifierFactory;

#define CAMERA_ANIMATION_MODIFIER_NAME(name) \
	std::string getModifierName() const \
{ \
	return name; \
}

#endif /* RY_CAMERAANIMATIONMODIFIERFACTORY_H */
