// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "object.h"

// Pacs includes
#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>

// Misc includes
#include <nel/misc/vectord.h>
#include <nel/misc/quat.h>
#include <nel/misc/path.h> 

// 3d includes
#include <nel/3d/u_scene.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_instance_material.h>

#define BRAKE_FORCE (-0.1) // (m.s-2)
#define GRAVITY_FORCE (-15.0)
#define SHOCK_ABSORB (1.0)		//

using namespace NL3D;
using namespace NLNET;
using namespace NLMISC;
using namespace NLPACS;

// ***************************************************************************

CObjectDyn::CObjectDyn (double width, double depth, double height, double orientation, const CVectorD& pos, 
				 const CVectorD& speed, bool obstacle, UMoveContainer &container, UScene &scene, 
				 UMovePrimitive::TReaction reaction, NLPACS::UMovePrimitive::TTrigger trigger,
				 uint8 worldImage, uint8 nbImage, uint8 insertWorldImage)
{
	// Create a box instance
	_Instance = scene.createInstance(CPath::lookup("rectangle.shape"));
	if(_Instance.empty())
	{
		nlerror("Failed to load shape: %s", CPath::lookup("rectangle.shape").c_str());
	}

	// Freezed
	Freezed = reaction == UMovePrimitive::DoNothing;
	
	// Setup the instance
	if (_Instance.getNumMaterials())
	{
		uint i;
		for (i=0; i<_Instance.getNumMaterials(); i++)
		{
			UInstanceMaterial material = _Instance.getMaterial(i);
			if (trigger != UMovePrimitive::NotATrigger)
			{
				// material.setBlend(true);
				// material.setBlendFunc(UInstanceMaterial::srcalpha, UInstanceMaterial::invsrcalpha);
				material.setDiffuse (CRGBA(255,255,255,255));
				// material.setOpacity(128);
			}
			else
			{
				if (Freezed)
				{
					material.setDiffuse (CRGBA(128,0,0,255));
					material.setOpacity(255);
				}
				else
				{
				}
			}
		}
	}

	// Setup the instance
	_Instance.setScale (CVectorD (width, depth, height));
	_Instance.setRotQuat (CQuat (CVectorD (0, 0, 1), (float)orientation));
	
	// Create a collision volume
	_MovePrimitive = container.addCollisionablePrimitive (worldImage, nbImage);
	_MovePrimitive->insertInWorldImage (insertWorldImage);

	// Setup the collision primitive
	_MovePrimitive->setPrimitiveType (UMovePrimitive::_2DOrientedBox);
	_MovePrimitive->setHeight ((float)height);
	_MovePrimitive->setOrientation (orientation, insertWorldImage);
	_MovePrimitive->setSize ((float)width, (float)depth);

	 // This primitive is an obstacle (it blocks others)
	_MovePrimitive->setObstacle (obstacle);
	
	// Setup reaction type
	_MovePrimitive->setReactionType (reaction);
	
	// Setup absorption value
	_MovePrimitive->setAbsorbtion (0.9f);

	// Setup user data
	_MovePrimitive->UserData=(uint64)this;

	// Setup trigger type
	_MovePrimitive->setTriggerType (trigger);

	// Set pos and speed
	setPos (pos);
	setSpeed (speed);

	_MovePrimitive->setGlobalPosition (pos, insertWorldImage);
}

// ***************************************************************************

CObjectDyn::CObjectDyn (double diameter, double height, const CVectorD& pos, const CVectorD& speed, 
	bool obstacle, UMoveContainer &container, UScene &scene, UMovePrimitive::TReaction reaction, 
	NLPACS::UMovePrimitive::TTrigger trigger, uint8 worldImage, uint8 nbImage, uint8 insertWorldImage)
{
	// Create a box instance
	_Instance = scene.createInstance(CPath::lookup("cylinder.shape"));
	if(_Instance.empty())
	{
		nlerror("Failed to load shape: %s",CPath::lookup("cylinder.shape").c_str());
	}

	// Freezed
	Freezed = reaction == UMovePrimitive::DoNothing;
	
	// Setup the instance
	if (_Instance.getNumMaterials())
	{
		uint i;
		for (i=0; i<_Instance.getNumMaterials(); i++)
		{
			UInstanceMaterial material = _Instance.getMaterial(i);
			if (trigger != UMovePrimitive::NotATrigger)
			{
				// material.setBlend(true);
				// material.setBlendFunc(UInstanceMaterial::srcalpha, UInstanceMaterial::invsrcalpha);
				material.setDiffuse (CRGBA(255,255,255));
				// material.setOpacity(128);
			}
			else
			{
				if (Freezed)
				{
					material.setDiffuse (CRGBA(128,0,0));
					material.setOpacity(255);
				}
				else
				{
				}
			}
		}
	}
	
	// Setup the instance
	_Instance.setScale (CVectorD (diameter, diameter, height));

	// Create a collision volume
	_MovePrimitive = container.addCollisionablePrimitive (worldImage, nbImage);
	_MovePrimitive->insertInWorldImage (insertWorldImage);

	// Setup the primitive
	_MovePrimitive->setPrimitiveType (UMovePrimitive::_2DOrientedCylinder);
	_MovePrimitive->setHeight ((float)height);
	_MovePrimitive->setRadius ((float)diameter/2.f);

	// This primitive is an obstacle (it blocks others)
	_MovePrimitive->setObstacle (obstacle);

	// Setup reaction type
	_MovePrimitive->setReactionType (reaction);

	// Setup reaction type
	_MovePrimitive->setAbsorbtion (0.9f);

	// Setup user data
	_MovePrimitive->UserData=(uint64)this;

	// Setup trigger type
	_MovePrimitive->setTriggerType (trigger);

	// Set pos and speed
	setPos (pos);
	setSpeed (speed);
	_MovePrimitive->setGlobalPosition (pos, insertWorldImage);
}

// ***************************************************************************

void CObjectDyn::tryMove (double deltaTime, UMoveContainer &container, uint8 worldImage)
{
	// New speed
	if ( (_MovePrimitive->getSpeed(worldImage).norm()>0.f) || _Speed.norm()>0.f )
	{
		// Brake
		CVectorD newSpeed=_Speed;
		newSpeed.normalize();
		newSpeed*=BRAKE_FORCE*deltaTime;
		newSpeed+=_Speed;

		// Stop ?
		if (_Speed*newSpeed<=0)
			newSpeed=CVectorD (0,0,0);

		// Set speed
		setSpeed (newSpeed);

		// Try this move
		_MovePrimitive->move (_Speed, worldImage);
		_TryMove=true;
	}
	else
		_TryMove=false;

}

// ***************************************************************************

void CObjectDyn::doMove (double deltaTime, uint8 worldImage)
{
	// New speed
	setSpeed (_MovePrimitive->getSpeed(worldImage));

	// New position
	setPos (_MovePrimitive->getFinalPosition (worldImage));
}

// ***************************************************************************

void CObjectDyn::setPos (const CVectorD& pos)
{
	_Position=pos;
	if (!_Instance.empty())
		_Instance.setPos (pos);
}

// ***************************************************************************

void CObjectDyn::setGlobalPos (UGlobalPosition& gpos, CVectorD& pos, uint8 worldimage)
{
	// Place the primitive the first time
	_MovePrimitive->setGlobalPosition (gpos, worldimage);
	setPos (pos);
}

// ***************************************************************************

void CObjectDyn::setSpeed (const CVectorD& speed)
{
	_Speed=speed;
}

// ***************************************************************************

void CObjectDyn::remove (NLPACS::UMoveContainer &container, UScene &scene)
{
	// Remove from container
	container.removePrimitive (_MovePrimitive);

	// Remove instance
	if (!_Instance.empty())
		scene.deleteInstance (_Instance);
}

// ***************************************************************************

