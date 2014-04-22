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



#include "stdpch.h"

#include <vector>

#include "nel/3d/animation_time.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_bone.h"
#include "nel/misc/command.h"

#include "game_share/draw_section.h"

#include "animated_scene_object.h"
#include "user_entity.h"
#include "view.h"
#include "entities.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;

// some extern of client
extern CUserEntity	*UserEntity;
extern UScene		*Scene;


// map of anilated scene object
TMapCAnimatedSceneObject AnimatedSceneObject;


// Constructor for moving object, create entity (mesh, skeleton) and build animation on it
CAnimatedSceneObject::CAnimatedSceneObject( const string& ObjectName, const string& ClusterIG, const string& MeshName, const string& SkeletonName, const list < string >& Animations, const CVector& Position, const CQuat& Rotation, float Distance )
{
	_ObjectName = ObjectName;
	_ClusterIG = ClusterIG;
	_MeshName = MeshName;
	_SkeletonName = SkeletonName;

	_Position = Position;
	_Rotation = Rotation;
	_Distance = Distance * Distance;	//square distance for optimize distance calculation

	// create play list manager and animation set
	_PlayListManager = Scene->createPlayListManager();
	_AnimationSet = Driver->createAnimationSet();

	//fill animation set
	for( list< string >::const_iterator it = Animations.begin(); it != Animations.end(); ++it )
	{
		uint idAnim;
		try
		{
			idAnim = _AnimationSet->addAnimation( ( *it + string(".anim")).c_str(), (*it).c_str() );
		}
		catch(Exception &)
		{
			idAnim = UAnimationSet::NotFound;
			if( idAnim == UAnimationSet::NotFound)
			{
				nlwarning( "CAnimatedSceneObject::CAnimatedSceneObject Animation %s not found for object %s", (*it).c_str(), _ObjectName.c_str() );
				continue;
			}
		}
		_MapAnimation.insert( make_pair( *it, idAnim ) );
	}

	// build animation set
	_AnimationSet->build();

	// create playlist
	_PlayList = _PlayListManager->createPlayList( _AnimationSet );

	_Status = SAnimationStatus();
	_Status.RandomAnim = 1;
	_Status.ApplyDisplacement = 0;

	// load instance and bind to skeleton
	_Instance = Scene->createInstance( _MeshName );
	if( _Instance == NULL )
	{
		nlerror("CAnimatedSceneObject::CAnimatedSceneObject createInstance %s returned 0 -> no shape.", _MeshName.c_str() );
	}
	else
	{
		_Transform = _Instance;
	}

	// load skeleton, bind mesh and init position, rotation, cluster
	if(!_SkeletonName.empty())
	{
		_Skeleton = Scene->createSkeleton(_SkeletonName);
		if( _Skeleton == NULL )
		{
			nlerror("CAnimatedSceneObject::CAnimatedSceneObject createSkeleton %s returned 0 -> no skeleton.", _SkeletonName.c_str() );
		}
		_Transform = _Skeleton;
		_Skeleton->bindSkin( _Instance );
		// Enable Shadow
		if(ClientCfg.Shadows)
		{
			_Skeleton->enableCastShadowMap(true);
		}
	}

	// register skeleton in playlist
	_PlayList->registerTransform( _Transform );

	// Initialise squeleton initial state
	if( _MapAnimation.begin() != _MapAnimation.end() )
	{
		_PlayList->setAnimation( 0, (*_MapAnimation.begin()).second );
		_PlayList->setTimeOrigin(0, 0 );
		_PlayList->setSpeedFactor( 0, 1 );
		_PlayListManager->animate( 0 );
	}

	_Transform->setClusterSystem( IGCity[_ClusterIG] );
	_Transform->setPos( _Position );
	_Transform->setRotQuat( _Rotation );

	_IdAnimationPlayed = -1;
}


// Constructor for moving object, it not create entity (mesh, skeleton) but build animation on pre-exist entity
CAnimatedSceneObject::CAnimatedSceneObject( const CEntityId& Id, const list < std::string >& Animations )
{
	_Id = Id;
	_ObjectName = "";
	_ClusterIG = "";
	_MeshName = "";
	_SkeletonName = "";
	_Distance = 0;

	_Skeleton = NULL;
	_Instance = NULL;

	CEntityCL *entity = getEntity(Id);
	if(entity)
	{
		_Skeleton = *entity->skeleton();
		_Transform = _Skeleton;

		_Position = _Skeleton->getPos();
		_Rotation = _Skeleton->getRotQuat();

		// create play list manager and animation set
		_PlayListManager = Scene->createPlayListManager();
		_AnimationSet = Driver->createAnimationSet();

		//fill animation set
		for( list< string >::const_iterator its = Animations.begin(); its != Animations.end(); ++its )
		{
			uint idAnim;
			try
			{
				idAnim = _AnimationSet->addAnimation( ( *its + string(".anim") ).c_str(), (*its).c_str() );
			}
			catch(Exception &)
			{
				idAnim = UAnimationSet::NotFound;
				if( idAnim == UAnimationSet::NotFound)
				{
					nlwarning( "CAnimatedSceneObject::CAnimatedSceneObject Animation %s not found for entity CEntityId(%u,%f)", (*its).c_str(), Id.Type, (double)(sint64)Id.Id );
					continue;
				}
			}
			_MapAnimation.insert( make_pair( *its, idAnim ) );
		}

		// build animation set
		_AnimationSet->build();

		// create playlist
		_PlayList = _PlayListManager->createPlayList( _AnimationSet );

		_Status = SAnimationStatus();
		_Status.SequenceAnim = 1;
		_Status.ApplyDisplacement = 1;

		// register skeleton in playlist
		_PlayList->registerTransform( *entity->skeleton() );

		// Snap to ground(the snap check if it's a flyer).
		entity->pacsPos(_Position);
		entity->snapToGround();
		_Position = entity->pos();

		CMatrix current;
		current.identity ();
		current.setRot (_Rotation);

		// Rotation 90 degrees
		CMatrix rot90;
		rot90.identity ();
		rot90.rotateZ (-(float)Pi/2);
		current *= rot90;

		_Transform->unfreezeHRC();

		_Transform->setPos(_Position);
		_Transform->setRotQuat(current.getRot());

		_Transform->freezeHRC();

		_IdAnimationPlayed = -1;

		if( _Id.Type == RYZOMID::npc )
		{
			sendEndSequenceMessage( _Id, 1 );
		}
	}
	else
	{
		_PlayList = NULL;
		_AnimationSet = NULL;
		_PlayListManager = NULL;
	}
}


// Constructor for moving object, init by UTransform
CAnimatedSceneObject::CAnimatedSceneObject( const CEntityId& Id, UTransform *Transform, const list < std::string >& Animations, bool displacement )
{
	_Id = Id;
	_ObjectName = "";
	_ClusterIG = "";
	_MeshName = "";
	_SkeletonName = "";
	_Distance = 0;

	_Transform = Transform;
	_Instance = NULL;
	_Skeleton = NULL;
	_PlayList = NULL;

	_Position = _Transform->getPos();
	_Rotation = _Transform->getRotQuat();

	// create play list manager and animation set
	_PlayListManager = Scene->createPlayListManager();
	_AnimationSet = Driver->createAnimationSet();

	//fill animation set
	for( list< string >::const_iterator its = Animations.begin(); its != Animations.end(); ++its )
	{
		uint idAnim;
		try
		{
			idAnim = _AnimationSet->addAnimation( ( *its + string(".anim") ).c_str(), (*its).c_str() );
		}
		catch(Exception &)
		{
			idAnim = UAnimationSet::NotFound;
			if( idAnim == UAnimationSet::NotFound)
			{
				nlwarning( "CAnimatedSceneObject::CAnimatedSceneObject Animation %s not found for entity CEntityId(%u,%f)", (*its).c_str(), Id.Type, (double)(sint64)Id.Id );
				continue:
			}
		}
		_MapAnimation.insert( make_pair( *its, idAnim ) );
	}

	_Status = SAnimationStatus();
	_Status.SequenceAnim = 1;
	_Status.ApplyDisplacement = displacement;

	if( Animations.begin() != Animations.end() )
	{
		// build animation set
		_AnimationSet->build();

		// create playlist
		_PlayList = _PlayListManager->createPlayList( _AnimationSet );

		// register skeleton in playlist
		_PlayList->registerTransform( _Transform );

		_IdAnimationPlayed = -1;
	}
}


// Reset intial pos and rot matrix
void CAnimatedSceneObject::resetInitialPos( void )
{
	_IdAnimationPlayed = -1;

	// Current Matrix
	_Position = _Transform->getPos();
	_Rotation = _Transform->getRotQuat();

	CEntityCL *entity = getEntity(_Id);
	if(entity)
	{
		// Snap to ground(the snap check if it's a flyer).
		entity->pacsPos(_Position);
		entity->snapToGround();
		_Position = entity->pos();
	}

	CMatrix current;
	current.identity ();
	current.setRot (_Rotation);

	// Rotation 90 degrees
	CMatrix rot90;
	rot90.identity ();
	rot90.rotateZ (-(float)Pi/2);
	current *= rot90;

	_Transform->unfreezeHRC();

	_Transform->setPos(_Position);
	_Transform->setRotQuat(current.getRot());

	_Transform->freezeHRC();
}


// Destructor
CAnimatedSceneObject::~CAnimatedSceneObject()
{
	if( _Instance != NULL && !_MeshName.empty() )
	{
		Scene->deleteInstance( _Instance );
	}

	if( _Skeleton != NULL && !_SkeletonName.empty() )
	{
		Scene->deleteSkeleton( _Skeleton );
	}

	if( _PlayListManager != NULL )
	{
		if( _PlayList != NULL )
		{
			_PlayListManager->deletePlayList( _PlayList );
		}
		if( _AnimationSet != NULL )
		{
			Driver->deleteAnimationSet( _AnimationSet );
		}
		Scene->deletePlayListManager( _PlayListManager );
	}

	_MapAnimation.clear();
	_SequenceAnimation.clear();
}


// Start an animation, if object have more than one animation, it choose one randomly
void CAnimatedSceneObject::StartRandomAnimation( double time, float speedFactor )
{
	if( _Transform != NULL )
	{
		_Transform->unfreezeHRC();
	}

	uint numAnimation = _MapAnimation.size();
	numAnimation = ( rand() * numAnimation ) / RAND_MAX;
	map< string, uint >::iterator it = _MapAnimation.begin();
	for(uint i = 0; i < numAnimation; ++i ) ++it;
	numAnimation = (*it).second;

	_PlayList->setAnimation( 0, numAnimation );
	_IdAnimationPlayed = numAnimation;
	_StartAnimationTime = time;
	_OldAnimationTime = _StartAnimationTime;
	_PlayList->setTimeOrigin(0, _StartAnimationTime );
	_PlayList->setSpeedFactor( 0, speedFactor );

	if( _Transform != NULL )
	{
		_Position = _Transform->getPos();
		_Rotation = _Transform->getRotQuat();

		CEntityCL *entity = getEntity(_Id);
		if(entity)
		{
			// Snap to ground(the snap check if it's a flyer).
			entity->pacsPos(_Position);
			entity->snapToGround();
			_Position = entity->pos();
		}

 		// Read start animation rotation and position
 		_PreviousRotation = CQuat::Identity;
 		_PreviousTranslation = CVector::Null;

		// read an animation for init matrix
		UTrack* Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathRotQuat" );
		if( Track == NULL )
		{
			Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathRotQuat" );
		}
		if( Track == NULL )
		{
			Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "rotquat" );
		}
		if( Track != NULL )
		{
			Track->interpolate( 0, _PreviousRotation );
		}

		Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathPos" );
		if( Track == NULL )
		{
			Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathPos" );
		}
		if( Track == NULL )
		{
			Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "pathpos" );
		}
		if( Track != NULL )
		{
			Track->interpolate( 0, _PreviousTranslation );
		}

		// Animation Matrix
		CMatrix animation;
		animation.identity();
		animation.setPos (_PreviousTranslation);
		animation.setRot (_PreviousRotation);


		// reset orientation of skeleton
		CMatrix current;
		current.identity();
		current.setRot( _Rotation );
		CVector I, J, K;
		I = current.getJ();
		I.z=0;
		I.normalize ();
		K.set (0,0,1);
		J=K^I;
		current.setRot (I, J, K);
		current.setPos (_Position);

		_AnimTransformation = current;

		if(entity)
		{
			if(!entity->flyer())
			{
				_AnimTransformation.setPos( _AnimTransformation.getPos() - CVectorD( 0, 0, entity->pos().z ) );
			}
		}
	}
}


// Start next animation
void CAnimatedSceneObject::StartNextAnimation( double time, float speedFactor )
{
	if( _SequenceAnimation.begin() != _SequenceAnimation.end() )
	{
		if( _Transform != NULL )
		{
			_Transform->unfreezeHRC();
		}

		string animName = *_SequenceAnimation.begin();
		_SequenceAnimation.pop_front();

		map< string, uint >::iterator it = _MapAnimation.find( animName );
		if( it != _MapAnimation.end() )
		{
			uint idAnim = (*it).second;

			_PlayList->setAnimation( 0, idAnim );
			_IdAnimationPlayed = idAnim;
			_StartAnimationTime = time;
			_OldAnimationTime = _StartAnimationTime;
			_PlayList->setTimeOrigin(0, _StartAnimationTime );
			_PlayList->setSpeedFactor( 0, speedFactor );

			if( _Transform != NULL )
			{
				_Position = _Transform->getPos();
				_Rotation = _Transform->getRotQuat();

				CEntityCL *entity = getEntity(_Id);
				if(entity)
				{
					// Snap to ground(the snap check if it's a flyer).
					entity->pacsPos(_Position);
					entity->snapToGround();
					_Position = entity->pos();
				}

 				// Read start animation rotation and position
 				_PreviousRotation = CQuat::Identity;
 				_PreviousTranslation = CVector::Null;

				// read an animation for init matrix
				UTrack* Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathRotQuat" );
				if( Track == NULL )
				{
					Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathRotQuat" );
				}
				if( Track == NULL )
				{
					Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "rotquat" );
				}
				if( Track != NULL )
				{
					Track->interpolate( 0, _PreviousRotation );
				}

				Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathPos" );
				if( Track == NULL )
				{
					Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathPos" );
				}
				if( Track == NULL )
				{
					Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "pathpos" );
				}
				if( Track != NULL )
				{
					Track->interpolate( 0, _PreviousTranslation );
				}

				// Animation Matrix
				CMatrix animation;
				animation.identity();
				animation.setPos (_PreviousTranslation);
				animation.setRot (_PreviousRotation);

				// reset orientation of skeleton
				CMatrix current;
				current.identity();
				current.setRot( _Rotation );
				CVector I, J, K;
				I = current.getJ();
				I.z=0;
				I.normalize ();
				K.set (0,0,1);
				J=K^I;
				current.setRot (I, J, K);
				current.setPos (_Position);

				_AnimTransformation = current;

				if(entity)
				{
					if(!entity->flyer() )
					{
						_AnimTransformation.setPos( _AnimTransformation.getPos() - CVectorD( 0, 0, entity->pos().z ) );
					}
				}
			}
		}

		if( _SequenceAnimation.size() == 1 )
		{
			sendEndSequenceMessage( _Id, idSequence() );
		}
	}
	else
	{
		//StartAnimation( string("FY_HOM_idlecycle") );
	}
}


// Start animation cooresponding to numAnim
void CAnimatedSceneObject::StartAnimation( string& nameAnim, double time, float speedFactor )
{
	if( _Transform != NULL )
	{
		_Transform->unfreezeHRC();
	}

	map< string, uint >::iterator it = _MapAnimation.find( nameAnim );
	if( it != _MapAnimation.end() )
	{
		uint numAnimation = (*it).second;

		_PlayList->setAnimation( 0, numAnimation );
		_IdAnimationPlayed = numAnimation;
		_StartAnimationTime = time;
		_OldAnimationTime = _StartAnimationTime;
		_PlayList->setTimeOrigin(0, _StartAnimationTime );
		_PlayList->setSpeedFactor( 0, speedFactor );


		if( _Transform != NULL )
		{
			_Position = _Transform->getPos();
			_Rotation = _Transform->getRotQuat();

			CEntityCL *entity = getEntity(_Id);
			if(entity)
			{
				// Snap to ground(the snap check if it's a flyer).
				entity->pacsPos(_Position);
				entity->snapToGround();
				_Position = entity->pos();
			}

 			// Read start animation rotation and position
 			_PreviousRotation = CQuat::Identity;
 			_PreviousTranslation = CVector::Null;

			// read an animation for init matrix
			UTrack* Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathRotQuat" );
			if( Track == NULL )
			{
				Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathRotQuat" );
			}
			if( Track == NULL )
			{
				Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "rotquat" );
			}
			if( Track != NULL )
			{
				Track->interpolate( 0, _PreviousRotation );
			}

			Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathPos" );
			if( Track == NULL )
			{
				Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathPos" );
			}
			if( Track == NULL )
			{
				Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "pathpos" );
			}
			if( Track != NULL )
			{
				Track->interpolate( 0, _PreviousTranslation );
			}

			// Animation Matrix
			CMatrix animation;
			animation.identity();
			animation.setPos (_PreviousTranslation);
			animation.setRot (_PreviousRotation);

			// reset orientation of skeleton
			CMatrix current;
			current.identity();
			current.setRot( _Rotation );
			CVector I, J, K;
			I = current.getJ();
			I.z=0;
			I.normalize ();
			K.set (0,0,1);
			J=K^I;
			current.setRot (I, J, K);
			current.setPos (_Position);

			_AnimTransformation = current;

			if(entity)
			{
				if( !entity->flyer() )
				{
					_AnimTransformation.setPos( _AnimTransformation.getPos() - CVectorD( 0, 0, entity->pos().z ) );
				}
			}
		}
	}
}


// remove the initial animation offset
void CAnimatedSceneObject::removeOffsetAnimation ()
{
 	// Read start animation rotation and position
 	_PreviousRotation = CQuat::Identity;
 	_PreviousTranslation = CVector::Null;

	// read an animation for init matrix
	UTrack* Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathRotQuat" );
	if( Track == NULL )
	{
		Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathRotQuat" );
	}
	if( Track == NULL )
	{
		Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "rotquat" );
	}
	if( Track != NULL )
	{
		Track->interpolate( 0, _PreviousRotation );
	}

	Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathPos" );
	if( Track == NULL )
	{
		Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathPos" );
	}
	if( Track == NULL )
	{
		Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "pathpos" );
	}
	if( Track != NULL )
	{
		Track->interpolate( 0, _PreviousTranslation );
	}

	// Remove first frame animation from transform
	CVector position = _Transform->getPos ();
	CQuat rotation = _Transform->getRotQuat ();

	// Build current matrix
	CMatrix current;
	current.identity ();
	current.setPos (position);
	current.setRot (rotation);

	// Build invert first frame matrix
	CMatrix firstFrame;
	firstFrame.identity ();
	firstFrame.setPos (_PreviousTranslation);
	firstFrame.setRot (_PreviousRotation);
	firstFrame.invert ();

	// Remove the first frame animation
	current *= firstFrame;

	// Rotation 90 degrees
	CMatrix rot90;
	rot90.identity ();
	rot90.rotateZ (-(float)Pi/2);
	current *= rot90;

	// Set transform
	_Transform->setPos (current.getPos());
	_Transform->setRotQuat (current.getRot());
}


// update (add to end list) Animation sequence with sequence
void CAnimatedSceneObject::updateAnimationSequence( const list< string >& sequence )
{
	for( list< string >::const_iterator it = sequence.begin(); it != sequence.end(); ++it )
	{
		_SequenceAnimation.push_back( (*it) );
	}
}


// Process logic and displacement and setup visual animation
void CAnimatedSceneObject::applyObjectAnimationTimeslice( double time )
{
	_PlayListManager->animate( time );

	// Apply displacement if mode set and displacement animation exist
	if( _Status.ApplyDisplacement == 1 )
	{
		CQuat rot = CQuat::Identity;
		CVector translation = CVector::Null;
		// Read position and rotation in appropriate UTrack
		UTrack* Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathRotQuat" );
		if( Track == NULL )
		{
			Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathRotQuat" );
		}
		if( Track == NULL )
		{
			Track = _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "rotquat" );
		}
		if( Track != NULL )
		{
			Track->interpolate( (float)(time - _StartAnimationTime) * _PlayList->getSpeedFactor( 0 ), rot );
		}

		Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "PathPos" );
		if( Track == NULL )
		{
			Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "Bip01.PathPos" );
		}
		if( Track == NULL )
		{
			Track =	_AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( "pathpos" );
		}
		if( Track != NULL )
		{
			Track->interpolate( (float)(time - _StartAnimationTime) * _PlayList->getSpeedFactor( 0 ), translation );
		}
		applyDeltaRotationAndPosition( rot, translation);
	}

	processLogicTemp( (_OldAnimationTime - _StartAnimationTime) * _PlayList->getSpeedFactor( 0 ), (time - _StartAnimationTime) * _PlayList->getSpeedFactor( 0 ) );
	_OldAnimationTime = time;
}


// manage object animation
// TODO: manage other type animation than trigger distance and sequence list
void CAnimatedSceneObject::manageObjectAnimation( double Time, CVector userPos )
{
	double overLoop = 0;

	if( _IdAnimationPlayed != -1 )
	{
		if(  Time > ( _StartAnimationTime + (double) _AnimationSet->getAnimation( _IdAnimationPlayed )->getEndTime() / _PlayList->getSpeedFactor( 0 ) ) )
		{
			overLoop = Time - _StartAnimationTime - (double) _AnimationSet->getAnimation( _IdAnimationPlayed )->getEndTime() / _PlayList->getSpeedFactor( 0 );
/*			nlinfo("overLoop %f  Time %f  Start %f  getEndTime() %f  speedFactor() %f",
				overLoop,
				Time,
				_StartAnimationTime,
				_AnimationSet->getAnimation( _IdAnimationPlayed )->getEndTime(),
				_PlayList->getSpeedFactor( 0 ));
*/
			nlassert( overLoop >= 0 );

			// play through logic and movement of end of last animation before animation change
			nlassert( Time-overLoop >= _StartAnimationTime );
			applyObjectAnimationTimeslice(Time-overLoop);
			nlassert( Time >= _StartAnimationTime );

			if( _Status.LoopAnim )
			{
				if( _StartAnimationTime == 0 )
				{
					overLoop = 0;
				}
				_StartAnimationTime = Time - overLoop;

				nlassert( Time >= _StartAnimationTime );

				_PlayList->setTimeOrigin( 0, _StartAnimationTime );
			}
			else
			{
				removeOffsetAnimation ();
				if( _Transform != NULL )
				{
					_Transform->freezeHRC();
				}
				_IdAnimationPlayed = -1;
			}
		}
	}

	if( _IdAnimationPlayed == -1 )
	{
		if( _Status.CodeAnim != 1 )
		{
			if( ( _Distance == 0 ) || ( ( userPos - _Position ).sqrnorm() < _Distance ) )
			{
				if( _Status.RandomAnim == 1 )
				{
					StartRandomAnimation( Time - overLoop );
				}
				else if ( _Status.SequenceAnim == 1 )
				{
					StartNextAnimation( Time - overLoop );
				}
			}
		}
		else
		{
			float angle = (float)((((uint32)Time * 1000)%(uint32)(1000*2*Pi))*0.001);
			CMatrix rot;
			rot.identity();
			rot.rotateZ(angle);
/*
			CQuat rot = CQuat (CVector (0, 0, 1), angle) * _Transform->getRotQuat();
			rot.normalize();
*/
			_Transform->setRotQuat( rot.getRot() );
		}
	}

	if( _IdAnimationPlayed != -1 )
	{
		// play through anim visuals and logic and movement for the timeslice
		applyObjectAnimationTimeslice(Time);
	}
}

// Apply animation rotation and translation
void CAnimatedSceneObject::applyDeltaRotationAndPosition( CQuat& rot, CVector& trans )
{
	if( _Transform != NULL )
	{
		// Animation Matrix
		CMatrix animation;
		animation.identity();
		animation.setPos (trans);
		animation.setRot (rot);

		// Compose matrix
		CMatrix composed;
		composed = _AnimTransformation * animation;

		_Rotation = composed.getRot();
		_Position = composed.getPos();

		CEntityCL *entity = getEntity(_Id);
		if(entity)
		{
// \todo GUIGUI : TO CHECK
#if 1
			if( !entity->flyer() )
				_Position.z += entity->pos().z;

			entity->pacsPos(_Position);
			entity->snapToGround();

			if( !entity->flyer() )
				_Position.z = entity->pos().z + composed.getPos().z;
// Old version
#else
			if( !entity->flyer() )
			{
				_Position.z += entity->pos().z;
				entity->pacsPos(_Position);
				entity->snapToGround();
				_Position.z = entity->pos().z + composed.getPos().z;
			}
			else
			{
				entity->pos(_Position);
			}
#endif
		}

		_Transform->setRotQuat(_Rotation);
		_Transform->setPos(_Position);
	}
}


// Instanciate an object and add it to map
void addAnimatedSceneObject( const string& AnimatedSceneObjectName, const string& ClusterIG, const string& MeshName, const string& SkeletonName, const list <string>& Animations, const CVector& Position, const CQuat& Rotation, float Distance )
{
	if( AnimatedSceneObject.find( AnimatedSceneObjectName ) == AnimatedSceneObject.end() )
	{
		CAnimatedSceneObject * o = new CAnimatedSceneObject( AnimatedSceneObjectName, ClusterIG, MeshName, SkeletonName, Animations, Position, Rotation, Distance );

		if( o != NULL )
		{
			AnimatedSceneObject.insert( make_pair( AnimatedSceneObjectName, o) );
		}
	}
}

void addEntityClAnimatedSceneObject( const CEntityId& Id, const list < string >& Animations)
{
	string test = string( "CEntityId") + toString( Id.Type) + toString( Id.Id);
	if( AnimatedSceneObject.find( string( "CEntityId") + toString( Id.Type) + toString( Id.Id) ) == AnimatedSceneObject.end() )
	{
		CAnimatedSceneObject * o = new CAnimatedSceneObject( Id, Animations );

		if( o != NULL )
		{
			AnimatedSceneObject.insert( make_pair( string( "CEntityId") + toString( Id.Type) + toString( Id.Id), o) );
		}
	}
}

void addUknowTypeSceneObject( const CEntityId& Id, NL3D::UTransform *Transform, const std::list < std::string >& Animations, bool displacement )
{
	if( AnimatedSceneObject.find( string( "CEntityId") + toString( Id.Type) + toString( Id.Id) ) == AnimatedSceneObject.end() )
	{
		CAnimatedSceneObject * o = new CAnimatedSceneObject( Id, Transform, Animations, displacement );

		if( o != NULL )
		{
			AnimatedSceneObject.insert( make_pair( string( "CEntityId") + toString( Id.Type) + toString( Id.Id), o) );
		}
	}
}

// manage animated object
void manageAnimatedSceneObject( double Time )
{
	for( TMapCAnimatedSceneObject::iterator it = AnimatedSceneObject.begin(); it != AnimatedSceneObject.end(); ++it )
	{
		(*it).second->manageObjectAnimation( Time, View.viewPos() );
	}
}

// delete
void deleteAnimatedSceneObject( const string& AnimatedSceneObjectName )
{
	TMapCAnimatedSceneObject::iterator it = AnimatedSceneObject.find( AnimatedSceneObjectName );
	if( it != AnimatedSceneObject.end() )
	{
		delete (*it).second;
		AnimatedSceneObject.erase( it );
	}
}

// update sequence animation
void updateAnimationSequence( const CEntityId& id, const list< string >& sequence, uint32 idSequence )
{
	TMapCAnimatedSceneObject::iterator it;

	for( it = AnimatedSceneObject.begin(); it != AnimatedSceneObject.end(); ++it )
	{
		if( (*it).second->getId() == id )
		{
			break;
		}
	}

	if( it != AnimatedSceneObject.end() )
	{
		(*it).second->updateAnimationSequence( sequence );
		(*it).second->idSequence( idSequence );
	}
}

// Reset animation for the entity and break the current one.
void resetAnimatedSceneObject(const CEntityId& id)
{
	for(TMapCAnimatedSceneObject::iterator it = AnimatedSceneObject.begin(); it != AnimatedSceneObject.end(); ++it )
	{
		if((*it).second->getId() == id)
		{
			(*it).second->breakCurrentAnimation();
			(*it).second->resetSequence();
			(*it).second->resetInitialPos();
			break;
		}
	}
}

void sendEndSequenceMessage( const CEntityId& id, uint32 idSequence )
{
	SEND_MSG_MACRO
	(
		"ANIME_BAR_MAN_END",
		SAnimeBarManEnd end;
		end.Id = id;
		end.SlotNumber = idSequence;
		msg.serial(end);,
		;,
		;
	)
}


//---------------------------------------------------
// processLogic :
//
//---------------------------------------------------
void CAnimatedSceneObject::processLogicTemp( double startOffset, double endOffset )
{
	static uint32 previousSoundIndex = 0;

	// get the logical animation data across the current anim segment
	UTrack * soundTrack = NULL;
	vector<CAnimationTime> result;

//	EAM->processLogic( _AnimationsGlobalId[MOVING], startOffset, endOffset, soundTrack, result);

	// get infos about the anim

	char *soundTrackName = "NoteTrack";
	soundTrack =  _AnimationSet->getAnimation( _IdAnimationPlayed )->getTrackByName( soundTrackName );
	if( soundTrack != NULL )
	{
		UTrackKeyframer * soundTrackKF = dynamic_cast<UTrackKeyframer *>(soundTrack);
		if( soundTrackKF == NULL )
		{
			nlerror("The track %s is not a key framer track",soundTrackName);
		}

		soundTrackKF->getKeysInRange((float)startOffset, (float)endOffset, result);
	}

	// process sounds
	if( SoundMngr )
	{
		if( soundTrack )
		{
			vector<CAnimationTime>::iterator itResult;
			for( itResult = result.begin(); itResult != result.end(); ++itResult )
			{
				string soundName;
				double keyTime = *itResult;
				nlinfo("keyTime = %f  result size : %d",*itResult,result.size());

				if( !soundTrack->interpolate( *itResult, soundName) )
				{
					nlwarning("The key at offset %f is not a string",*itResult);
				}
				else
				{
/*					// if there are step sounds
					if( soundName == "step" )
					{
						// get the material
						UGlobalPosition globalPos;
						getPrimitive()->getGlobalPosition(globalPos, dynamicWI);
						uint32 material = GR->getMaterial( globalPos );

						// choose the sound type soft/hard
						bool soft;
						if( Type.TypeInfo.Sex == CTypeEntity::male )
							soft = false;
						if( Type.TypeInfo.Sex == CTypeEntity::female )
							soft = true;

						vector<string> sounds;
						uint32 soundSourceId;

						// choose the sound  for the move type
						if( _AnimationsStateKey[MOVING] == "walk" )
						{
							if( SoundMngr->getSounds( material, WALK, soft, sounds ) )
							{
								string name = chooseRandom(sounds,previousSoundIndex );
								SoundMngr->spawnSound( name , pos ());
							}
							else
							{
								nlwarning("<CEntityCL::updatePos>  :  there is no walk step sound for the material %d",material);
 								SoundMngr->spawnSource( "Fwrks_Clac_1", pos() );
							}
						}
						_SoundSourcesId.push_back( soundSourceId );
					}
 					else*/ if (soundName.find ("snd_") != string::npos)
 					{
 						// need to spawn a sound linked to the anim
 						SoundMngr->spawnSource( soundName, _Position );
 						nlwarning ("launching sound for anim event from notetrack '%s'", soundName.c_str());
 					}
 					else
 					{
 						nlwarning ("unknown notetrack event: '%s'", soundName.c_str());
 					}
				}
			}
		}
	}

} // processAnimSounds //
