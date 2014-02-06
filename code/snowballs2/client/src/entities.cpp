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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <cmath>
#include <map>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/displayer.h>
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>
#include <nel/3d/u_skeleton.h>

#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_visual_collision_entity.h>

#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_global_position.h>

#include "snowballs_client.h"
#include "entities.h"
#include "pacs.h"
#include "animation.h"
#include "camera.h"
#ifdef NL_OS_WINDOWS
#include "sound.h"
#endif
#include "mouse_listener.h"
#include "landscape.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

namespace SBCLIENT {

//
// Variables
//

// A map of entities. All entities are later referred by their unique id
map<uint32, CEntity>	Entities;

CEntity					*Self = NULL;

// The size of the world, in meter
float					WorldWidth = 20*160;
float					WorldHeight = 6*160;

// Entity Id, only used offline
uint32					NextEID = 1000000;

// The speed settings
float					PlayerSpeed = 10.0f;	// 6.5 km/h
float					SnowballSpeed = 15.0f;	// 36 km/h

// these variables are set with the config file

// Setup for the name up the character
float					EntityNameSize;
CRGBA					EntityNameColor;

bool _TestCLS = false;


// Set the state of the entity (Appear, Normal, Disappear)
void CEntity::setState (TState state)
{
	State = state;
	StateStartTime = LocalTime;
}


// Get an map iterator on a entity, specified by its id
EIT findEntity (uint32 eid, bool needAssert)
{
	EIT entity = Entities.find (eid);
	if (entity == Entities.end () && needAssert)
	{
		nlerror ("Entity %u not found", eid);
	}
	return entity;
}

// -- -- things like Creature, Effect, Scenery seem more flexible than Self, Other, Snowball
// -- -- random keywords: entitybehavior (animations), entityinteraction (targetable, menu, )
// Creates an entity, given its id, its type (Self, Other, Snowball), its start and server positions.
void addEntity (uint32 eid, std::string name, CEntity::TType type, const CVector &startPosition, const CVector &serverPosition)
{
//	nlinfo ("adding entity %u", eid);

	// Check that the entity doesn't exist yet
	EIT eit = findEntity (eid, false);
	if (eit != Entities.end ())
	{
		nlerror ("Entity %d already exist", eid);
	}

	// Create a new entity
	eit = (Entities.insert (make_pair (eid, CEntity()))).first;
	CEntity	&entity = (*eit).second;

	// Check that in the case the entity newly created is a Self, there's not a Self yet.
	if (type == CEntity::Self)
	{
		if (Self != NULL)
			nlerror("Self entity already created");

		Self = &entity;
	}

	// Set the entity up
	entity.Id = eid;
	entity.Type = type;
	entity.Name = name;
	entity.Position = startPosition;
	entity.Angle = 0.0f;
	entity.ServerPosition = serverPosition;
	entity.VisualCollisionEntity = VisualCollisionManager->createEntity();

	// setup the move primitive and the mesh instance depending on the type of entity
	switch (type)
	{
	case CEntity::Self:
		// create a move primitive associated to the entity
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		// it's a cylinder
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		// the entity should slide against obstacles
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		// do not generate event if there is a collision
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		// which entity should collide against me
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SnowballCollisionBit+StaticCollisionBit);
		// the self collision bit
		entity.MovePrimitive->setOcclusionMask(SelfCollisionBit);
		// the self is an obstacle
		entity.MovePrimitive->setObstacle(true);
		// the size of the cylinder
		entity.MovePrimitive->setRadius(1.0f);
		entity.MovePrimitive->setHeight(1.8f);
		// only use one world image, so use insert in world image 0
		entity.MovePrimitive->insertInWorldImage(0);
		// retrieve the start position of the entity
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);

		// create instance of the mesh character
		entity.Instance = Scene->createInstance ("gnu.shape");
		entity.Skeleton = Scene->createSkeleton ("gnu.skel");
		// use the instance on the skeleton
		entity.Skeleton.bindSkin (entity.Instance);

		// Allow the skeleton to cast shadows.
		entity.Skeleton.enableCastShadowMap(true);

		entity.Instance.hide ();

		entity.Angle = MouseListener->getOrientation();

		// setup final parameters
		entity.Speed = PlayerSpeed;
		entity.Particule = Scene->createInstance ("appear.ps");
		entity.setState (CEntity::Appear);
		playAnimation (entity, LogInAnim);
		playAnimation (entity, IdleAnim);

		break;
	case CEntity::Other:
		entity.MovePrimitive = MoveContainer->addCollisionablePrimitive(0, 1);
		entity.MovePrimitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
		entity.MovePrimitive->setReactionType(UMovePrimitive::Slide);
		entity.MovePrimitive->setTriggerType(UMovePrimitive::NotATrigger);
		entity.MovePrimitive->setCollisionMask(OtherCollisionBit+SelfCollisionBit+SnowballCollisionBit);
		entity.MovePrimitive->setOcclusionMask(OtherCollisionBit);
		entity.MovePrimitive->setObstacle(true);
		entity.MovePrimitive->setRadius(1.0f);
		entity.MovePrimitive->setHeight(1.8f);
		entity.MovePrimitive->insertInWorldImage(0);
		entity.MovePrimitive->setGlobalPosition(CVectorD(startPosition.x, startPosition.y, startPosition.z), 0);

		entity.Instance = Scene->createInstance ("gnu.shape");
		entity.Skeleton = Scene->createSkeleton ("gnu.skel");
		entity.Skeleton.bindSkin (entity.Instance);
		entity.Instance.hide ();

		entity.Speed = PlayerSpeed;
		entity.Particule = Scene->createInstance ("appear.ps");
		entity.setState (CEntity::Appear);
		playAnimation (entity, LogInAnim);
		playAnimation (entity, IdleAnim);

		break;
	case CEntity::Snowball:
		entity.MovePrimitive = NULL;

		// allows collision snapping to the ceiling
		entity.VisualCollisionEntity->setCeilMode(true);

		entity.Instance = Scene->createInstance ("snowball.shape");
		entity.Skeleton = NULL;
		entity.Speed = SnowballSpeed;

		// -- -- riiiiight
//#ifdef NL_OS_WINDOWS
//		playSound (entity, SoundId);
//#endif
		entity.setState (CEntity::Normal);
		break;
	}

	if (!entity.Skeleton.empty())
		entity.Skeleton.setPos (startPosition);

	entity.Instance.setPos (startPosition);

// todo sound
//	if (entity.Source != NULL)
//		entity.Source->setPosition (startPosition);

	if (!entity.Particule.empty())
		entity.Particule.setPos (startPosition);

}

// effectively remove the entity (don't play animation)
void deleteEntity (CEntity &entity)
{
	if (!entity.Particule.empty())
	{
		Scene->deleteInstance (entity.Particule);
		entity.Particule = NULL;
	}

	deleteAnimation (entity);

	if (!entity.Skeleton.empty())
	{
		entity.Skeleton.detachSkeletonSon (entity.Instance);
		Scene->deleteSkeleton (entity.Skeleton);
		entity.Skeleton = NULL;
	}

	if (!entity.Instance.empty())
	{
		Scene->deleteInstance (entity.Instance);
		entity.Instance = NULL;
	}

	if (entity.VisualCollisionEntity != NULL)
	{
		VisualCollisionManager->deleteEntity (entity.VisualCollisionEntity);
		entity.VisualCollisionEntity = NULL;
	}

	if (entity.MovePrimitive != NULL)
	{
		MoveContainer->removePrimitive(entity.MovePrimitive);
		entity.MovePrimitive = NULL;
	}

//#ifdef NL_OS_WINDOWS
//	deleteSound (entity);
//#endif

//	nlinfo ("Remove the entity %u from the Entities list", entity.Id);
	EIT eit = findEntity (entity.Id);
	Entities.erase (eit);
}


// Remove an entity specified by its id
// The entity passes into the Disappear state
void removeEntity (uint32 eid)
{
//	nlinfo ("removing entity %u", eid);

	// look for the entity
	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	// if there is a particle system linked, delete it
	if (!entity.Particule.empty())
	{
		Scene->deleteInstance (entity.Particule);
		entity.Particule = NULL;
	}

//	if (entity.Type == CEntity::Other)
	{

		entity.Particule = Scene->createInstance("disappear.ps");
		entity.Particule.setPos (entity.Position);
	}

	playAnimation (entity, LogOffAnim, true);
	entity.setState (CEntity::Disappear);
}

void	removeAllEntitiesExceptUs ()
{
	EIT		eit, nexteit;

	// move entities
	for (eit = Entities.begin (); eit != Entities.end (); )
	{
		nexteit = eit; nexteit++;

		CEntity	&entity = (*eit).second;

		if (entity.Type != CEntity::Self)
		{
			// effectively remove the entity (don't play animation)
			deleteEntity (entity);
		}

		eit = nexteit;
	}
}

void deleteAllEntities()
{
	EIT eit, nexteit;
	for (eit = Entities.begin(); eit != Entities.end(); )
	{
		nexteit = eit; nexteit++;
		CEntity	&entity = (*eit).second;
		deleteEntity (entity);
		eit = nexteit;
	}
	Self = NULL;
}





// What to do when the entity appears
void stateAppear (CEntity &entity)
{
	// after 1 second, show the instance
	if (LocalTime > entity.StateStartTime + 1.0)
	{
		if (entity.Instance.getVisibility () != UTransform::Show)
			entity.Instance.show ();
	}

	// after 5 seconds, delete the particle system (if any)
	// and pass the entity into the Normal state
	if (LocalTime > entity.StateStartTime + 3.0)
	{
		if (!entity.Particule.empty())
		{
			Scene->deleteInstance (entity.Particule);
			entity.Particule = NULL;
		}

		entity.setState (CEntity::Normal);
	}

	if (entity.MovePrimitive != NULL)
		entity.MovePrimitive->move(CVector(0,0,0), 0);
}

// What to do when the entity disappears
void stateDisappear (CEntity &entity)
{
	// after 1 second, remove the mesh and all collision stuff
	if (LocalTime > entity.StateStartTime + 1.0)
	{
		if (entity.Instance.getVisibility () != UTransform::Hide)
		{
			if (entity.Type == CEntity::Self)
			{
				if (Self == NULL)
					nlerror("Self entity doesn't exist");
				Self = NULL;
			}

			entity.Instance.hide ();
		}
	}

	// after 5 seconds, remove the particle system and the entity entry
	if (LocalTime > entity.StateStartTime + 3.0)
	{
		deleteEntity (entity);
	}
	else
	{
		if (entity.MovePrimitive != NULL)
			entity.MovePrimitive->move(CVector(0,0,0), 0);
	}
}

void stateNormal (CEntity &entity)
{
	double	dt = LocalTimeDelta;
	CVector	oldPos;
	CVector	newPos;

	oldPos = entity.Position;
	CVector	pDelta = entity.Position - entity.ServerPosition;
	CVector	pDeltaOri = pDelta;
	pDelta.z = 0.0f;

	// -- -- simple random bots =)  share with server

	// find a new random server position
	if (entity.Type == CEntity::Other && entity.AutoMove)
	{
		switch (entity.BotState)
		{
		case 0:
			// choose something to do
			if (frand(1.0f) > 0.5f)
				entity.BotState = 5;
			else
				entity.BotState = 2;
			break;
		case 1:
			// walk
			if (pDelta.norm() < 0.1f || LocalTime - entity.BotStateStart > 3.000)
			{
				// reached the point
				entity.BotState = 0;
			}
			else
			{
//				entity.IsWalking = true;
//				entity.IsAiming = false;
			}
			break;
		case 2:
			// aim
			entity.IsWalking = false;
			entity.IsAiming = true;
			entity.BotStateStart = LocalTime;
			entity.BotState = 3;
			break;
		case 3:
			// wait to shoot
			entity.IsWalking = false;
			entity.IsAiming = true;
			if (LocalTime - entity.BotStateStart > 1.000)
			{
				entity.BotState = 4;
				entity.BotStateStart = LocalTime;
				CVector	AimingPosition = entity.Position+CVector(0.0f, 0.0f, 2.0f);
				CVector	direction = CVector((float)(cos(entity.Angle)), (float)(sin(entity.Angle)), 0.3f).normed();
				CVector	AimedTarget = getTarget(AimingPosition,
												direction,
												100);
				shotSnowball(NextEID++, entity.Id, AimingPosition, AimedTarget, SnowballSpeed, 3.0f);
			}
			break;
		case 4:
			// shoot
			entity.IsWalking = false;
			entity.IsAiming = false;
			if (LocalTime - entity.BotStateStart > 1.000)
			{
				entity.BotState = 5;
				entity.BotStateStart = LocalTime;
			}
			break;
		case 5:
			// choose a direction
			float EntityMaxSpeed = 10.0f;
			entity.AuxiliaryAngle += frand(1.5f)-0.75f;
			entity.ServerPosition += CVector((float)cos(entity.AuxiliaryAngle),
											 (float)sin(entity.AuxiliaryAngle),
											 0.0f)*EntityMaxSpeed;
			entity.BotState = 1;
			entity.BotStateStart = LocalTime;
			break;
		}
	}


	if (entity.Type == CEntity::Snowball && LocalTime >= entity.Trajectory.getStopTime())
	{
/*
		CVector	tp(1140,-833,30);
		nlinfo("dist=%f", (entity.Position-tp).norm());
		if ((entity.Position-tp).norm()<30.0f)
		{
			static UInstance t = NULL;
			if (t != NULL)
			{
				Scene->deleteInstance (t);
			}
			t = Scene->createInstance("pills.ps");
			t->setScale (10,10,10);
			CVector tp2 = tp;
			tp2.z+=20;
			t->setPos (tp2);
			nlinfo("touche");
		}
*/
		removeEntity(entity.Id);
	}



	// control the character animation
	if (entity.Type != CEntity::Snowball)
	{
		if (entity.IsAiming && !entity.WasAiming)
		{
			// start aiming
			playAnimation (entity, PrepareSnowBall, true);
			playAnimation (entity, PrepareSnowBallCycle, false);
		}
		else if (entity.WasAiming && !entity.IsAiming)
		{
/*			// end aiming
			playAnimation (entity, ThrowSnowball, true);

			if (entity.IsWalking)
				playAnimation (entity, WalkAnim);
			else
				playAnimation (entity, IdleAnim);
*/		}
		else if (entity.WasAiming && entity.IsAiming)
		{
			// currently aiming => do northing
		}
		else if (!entity.WasWalking && entity.IsWalking)
		{
			playAnimation (entity, PrepareWalkAnim, true);
			playAnimation (entity, WalkAnim);
		}
		else if (entity.WasWalking && !entity.IsWalking)
		{
			playAnimation (entity, IdleAnim, true);
		}

		entity.WasAiming = entity.IsAiming;
		entity.WasWalking = entity.IsWalking;
	}


	entity.ImmediateSpeed = CVector::Null;

	if (entity.Type == CEntity::Self)
	{
		// get new position
		newPos = MouseListener->getPosition();
		// get new orientation
		entity.Angle = MouseListener->getOrientation();

		// Interpolate the character orientation towards the server angle
		// for smoother movements
		float	sweepDistance = entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle;
		if (sweepDistance > (float)Pi)
		{
			sweepDistance -= (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle += (float)Pi*2.0f;
		}
		if (sweepDistance < -(float)Pi)
		{
			sweepDistance += (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle -= (float)Pi*2.0f;
		}
		float	sweepAngle = 4.0f*sweepDistance;
		entity.InterpolatedAuxiliaryAngle += (float)(sweepAngle*dt);
		if ((entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle)*sweepAngle <= 0.0)
			entity.InterpolatedAuxiliaryAngle = entity.AuxiliaryAngle;
		entity.Angle += entity.InterpolatedAuxiliaryAngle;

		// tell the move container how much the entity should move
		if (entity.IsWalking)
		{
			entity.ImmediateSpeed = (newPos-oldPos)/(float)dt;
			if (_TestCLS) entity.MovePrimitive->setGlobalPosition(newPos, 0);
			else entity.MovePrimitive->move(entity.ImmediateSpeed, 0);
		}
	}
	else if (entity.Type == CEntity::Other)
	{
		// go to the server position with linear interpolation
		// -- -- useful for speed limiting on frontend service
		// -- -- random note: also, get rid of the position service, 
		//       and move the snowball physics to a more useful service

		// Interpolate orientation for smoother motions
		// AuxiliaryAngle -> the server imposed angle
		// InterpolatedAuxiliaryAngle -> the angle showed by the entity
		float	sweepDistance = entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle;
		if (sweepDistance > (float)Pi)
		{
			sweepDistance -= (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle += (float)Pi*2.0f;
		}
		if (sweepDistance < -(float)Pi)
		{
			sweepDistance += (float)Pi*2.0f;
			entity.InterpolatedAuxiliaryAngle -= (float)Pi*2.0f;
		}
		float	sweepAngle = 4.0f*sweepDistance;
		entity.InterpolatedAuxiliaryAngle += (float)(sweepAngle*dt);
		if ((entity.AuxiliaryAngle-entity.InterpolatedAuxiliaryAngle)*sweepAngle <= 0.0)
			entity.InterpolatedAuxiliaryAngle = entity.AuxiliaryAngle;

		entity.Angle = entity.InterpolatedAuxiliaryAngle;

//		if (entity.IsWalking)
		if (pDelta.norm() > 0.1f)
		{
			pDelta.normalize();
			entity.ImmediateSpeed = pDelta*(-entity.Speed);
			entity.MovePrimitive->move(entity.ImmediateSpeed, 0);
			entity.IsWalking = true;
		}
		else
		{
			entity.IsWalking = false;
		}
	}
	else if (entity.Type == CEntity::Snowball)
	{
		// go to the server position using trajectory interpolation
		CVector newPos = entity.Trajectory.eval(LocalTime);
		if (newPos != entity.Position)
		{
			entity.Position = entity.Trajectory.eval(LocalTime);
			entity.Instance.show ();
		}
	}
	else
	{
		// automatic speed
		newPos = oldPos;
	}
}





void updateEntities ()
{
	// compute the delta t that has elapsed since the last update (in seconds)
	double	dt = LocalTimeDelta;
	EIT		eit, nexteit;

	// move entities
	for (eit = Entities.begin (); eit != Entities.end (); )
	{
		nexteit = eit; nexteit++;

		CEntity	&entity = (*eit).second;

		switch (entity.State)
		{
		case CEntity::Appear:
			stateAppear (entity);
			break;
		case CEntity::Normal:
			stateNormal (entity);
			break;
		case CEntity::Disappear:
			stateDisappear (entity);
			break;
		default:
			nlstop;
			break;
		}

		eit = nexteit;
	}

	// evaluate collisions
	MoveContainer->evalCollision(dt, 0);

	// snap entities to the ground
	for (eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;
		UGlobalPosition	gPos;

		if (entity.MovePrimitive != NULL)
		{
			// get the global position in pacs coordinates system
			entity.MovePrimitive->getGlobalPosition(gPos, 0);
			// convert it in a vector 3d
			entity.Position = GlobalRetriever->getGlobalPosition(gPos);
			// get the good z position
			gPos.LocalPosition.Estimation.z = 0.0f;
			entity.Position.z = GlobalRetriever->getMeanHeight(gPos);

			// check position retrieving
/*
			UGlobalPosition gPosCheck;
			gPosCheck = GlobalRetriever->retrievePosition(entity.Position);
			if (gPos.InstanceId != gPosCheck.InstanceId ||
				gPos.LocalPosition.Surface != gPosCheck.LocalPosition.Surface)
			{
				nlwarning("Checked UGlobalPosition differs from store");
//				gPos.InstanceId = gPosCheck.InstanceId;
//				gPos.LocalPosition.Surface = gPosCheck.LocalPosition.Surface;
			}
*/
			// snap to the ground
			if (!GlobalRetriever->isInterior(gPos))
				entity.VisualCollisionEntity->snapToGround(entity.Position);

			if (entity.Type == CEntity::Other &&
				(entity.ServerPosition-entity.Position)*entity.ImmediateSpeed < 0.0f)
			{
//				nlinfo("detected over entity %d", entity.Id);
				entity.ServerPosition.z = entity.Position.z;
				entity.Position = entity.ServerPosition;
				if (!GlobalRetriever->isInterior(gPos))
					entity.VisualCollisionEntity->snapToGround(entity.Position);
				entity.MovePrimitive->setGlobalPosition(CVectorD(entity.Position.x, entity.Position.y, entity.Position.z), 0);
			}

		}

		if (!entity.Instance.empty())
		{
			CVector	jdir;
			switch (entity.Type)
			{
			case CEntity::Self:
				jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);
				break;
			case CEntity::Other:
				jdir = CVector(-(float)cos(entity.Angle), -(float)sin(entity.Angle), 0.0f);
				break;
			case CEntity::Snowball:
				jdir = entity.Trajectory.evalSpeed(LocalTime).normed();
				break;
			}

			if (!entity.Skeleton.empty())
			{
				entity.Skeleton.setPos(entity.Position);
				entity.Skeleton.setRotQuat(jdir);
			}

			entity.Instance.setPos(entity.Position);
			entity.Instance.setRotQuat(jdir);
		}

// todo sound
//		if (entity.Source != NULL)
//			entity.Source->setPosition (entity.Position);

		if (!entity.Particule.empty())
		{
			entity.Particule.setPos(entity.Position);
		}

		if (entity.Type == CEntity::Self)
		{
			MouseListener->setPosition(entity.Position);
		}
	}
}

// Draw entities names up the characters
void renderEntitiesNames ()
{
	// Setup the driver in matrix mode
	Driver->setMatrixMode3D (Camera);
	// Setup the drawing context
	TextContext->setHotSpot (UTextContext::MiddleTop);
	TextContext->setColor (EntityNameColor);
	TextContext->setFontSize ((uint32)EntityNameSize);
	//
	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;
		if (!entity.Instance.empty() && entity.Type == CEntity::Other)
		{
			CMatrix		mat = Camera.getMatrix();
			mat.setPos(entity.Position + CVector(0.0f, 0.0f, 4.0f));
			mat.scale(10.0f);
			TextContext->render3D(mat, entity.Name);
		}
	}
}


// The entities preferences callback
void cbUpdateEntities (CConfigFile::CVar &var)
{
	if (var.Name == "EntityNameColor") EntityNameColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "EntityNameSize") EntityNameSize = var.asFloat ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initEntities()
{
	ConfigFile->setCallback ("EntityNameColor", cbUpdateEntities);
	ConfigFile->setCallback ("EntityNameSize", cbUpdateEntities);

	cbUpdateEntities (ConfigFile->getVar ("EntityNameColor"));
	cbUpdateEntities (ConfigFile->getVar ("EntityNameSize"));
}

void releaseEntities()
{
	// Remove config file callbacks
	ConfigFile->setCallback("EntityNameColor", NULL);
	ConfigFile->setCallback("EntityNameSize", NULL);

	// Delete all entities (should already have been called normally)
	deleteAllEntities();
}


// Reset the pacs position of an entity, in case pacs went wrong
void	resetEntityPosition(uint32 eid)
{
	uint32 sbid = NextEID++;
	EIT eit = findEntity (eid);

	CEntity	&entity = (*eit).second;

	UGlobalPosition	gPos;
	// get the global position
	gPos = GlobalRetriever->retrievePosition(entity.Position);
	// convert it in a vector 3d
	entity.Position = GlobalRetriever->getGlobalPosition(gPos);
	// get the good z position
	gPos.LocalPosition.Estimation.z = 0.0f;
	entity.Position.z = GlobalRetriever->getMeanHeight(gPos);

	// snap to the ground
	if (entity.VisualCollisionEntity != NULL && !GlobalRetriever->isInterior(gPos))
		entity.VisualCollisionEntity->snapToGround(entity.Position);

	if (entity.MovePrimitive != NULL)
		entity.MovePrimitive->setGlobalPosition(CVector(entity.Position.x, entity.Position.y, entity.Position.z), 0);
}


void	shotSnowball(uint32 sid, uint32 eid, const CVector &start, const CVector &target, float speed, float deflagRadius)
{
	// get direction
	CVector direction = (target-start).normed();

	// create a new snowball entity
	addEntity(sid, "", CEntity::Snowball, start, target);
	EIT sit = findEntity (sid);
	CEntity	&snowball = (*sit).second;

	snowball.AutoMove = 1;
	snowball.Trajectory.init(start, target, speed, LocalTime + 1.000);
	snowball.Instance.hide();

	EIT eit = findEntity (eid);
	CEntity	&entity = (*eit).second;

	// end aiming
	playAnimation (entity, ThrowSnowball, true);

	if (entity.IsWalking)
	{
		playAnimation (entity, PrepareWalkAnim, true);
		playAnimation (entity, WalkAnim);
	}
	else
		playAnimation (entity, IdleAnim);

}


//
// Commands
//

NLMISC_COMMAND(remove_entity,"remove a local entity","<eid>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;

	uint32 eid = (uint32)atoi(args[0].c_str());
	removeEntity (eid);

	return true;
}


NLMISC_COMMAND(add_entity,"add a local entity","<nb_entities> <auto_update>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	uint nb = (uint)atoi(args[0].c_str());

	for (uint i = 0; i < nb ; i++)
	{
		uint32 eid = NextEID++;
		CVector start(ConfigFile->getVar("StartPoint").asFloat(0), ConfigFile->getVar("StartPoint").asFloat(1), ConfigFile->getVar("StartPoint").asFloat(2));
		addEntity (eid, "Entity"+toString(eid), CEntity::Other, start, start);
		EIT eit = findEntity (eid);
		(*eit).second.AutoMove = atoi(args[1].c_str()) == 1;
	}

	return true;
}

NLMISC_COMMAND(speed,"set the player speed","[<entity_id>] <speed in km/h>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() == 1)
	{
		float speed = min( max( (float)atof(args[0].c_str()), 0.1f ), 200.0f ); // speed range in km/h
		if (Self != NULL)
		{
			MouseListener->setSpeed( speed / 3.6f );
			Self->Speed = speed / 3.6f;
		}
	}
	else if(args.size() == 2)
	{
		float speed = min( max( (float)atof(args[1].c_str()), 0.1f ), 200.0f ); // speed range in km/h

		uint eid = (uint)atoi(args[0].c_str());
		EIT eit = findEntity (eid);
		CEntity	&entity = (*eit).second;

		entity.Speed = speed / 3.6f;
		if (entity.Type == CEntity::Self)
		{
			MouseListener->setSpeed(entity.Speed);
		}
	}
	return true;
}

NLMISC_COMMAND(goto, "go to a given position", "<x> <y>")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;

	if (Self == NULL) return false;

	CEntity	&entity = *Self;

	float	x, y;

	x = (float)atof(args[0].c_str());
	y = (float)atof(args[1].c_str());

	//
	if (entity.MovePrimitive != NULL && entity.VisualCollisionEntity != NULL)
	{
		UGlobalPosition	gPos;
		entity.MovePrimitive->setGlobalPosition(CVectorD(x, y, 0.0f), 0);
		// get the global position in pacs coordinates system
		entity.MovePrimitive->getGlobalPosition(gPos, 0);
		// convert it in a vector 3d
		entity.Position = GlobalRetriever->getGlobalPosition(gPos);
		// get the good z position
		gPos.LocalPosition.Estimation.z = 0.0f;
		entity.Position.z = GlobalRetriever->getMeanHeight(gPos);
		// snap to the ground
		if (!GlobalRetriever->isInterior(gPos))
		   entity.VisualCollisionEntity->snapToGround( entity.Position );

		if (entity.Type == CEntity::Self)
		{
			MouseListener->setPosition(entity.Position);
		}
	}

	return true;
}

NLMISC_COMMAND(entities, "display all entities info", "")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;

	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&e = (*eit).second;
		log.displayNL("%s %u (k%u) %s %d", (Self==&e)?"*":" ", e.Id, (*eit).first, e.Name.c_str(), e.Type);
	}
	return true;
}

NLMISC_COMMAND(test_cls, "test the collision service, disables collision test on self", "")
{
	_TestCLS = !_TestCLS;
	return true;
}

} /* namespace SBCLIENT */

/* end of file */
