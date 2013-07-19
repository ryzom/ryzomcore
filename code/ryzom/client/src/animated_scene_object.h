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



#ifndef CL_ANIMATED_SCENE_OBJECT_H
#define CL_ANIMATED_SCENE_OBJECT_H

#include <string>
#include <map>
#include <list>
#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/quat.h"
#include "nel/misc/time_nl.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_animation.h"
#include "nel/net/unitime.h"
#include "ig_client.h"

//#include "game_share/cst_loader.h"

 /**
 * CAnimatedSceneObject manage no move animated scene object
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedSceneObject
{
public:

	struct SAnimationStatus
	{
		uint32	LoopAnim : 1;
		uint32  CodeAnim : 1;
		uint32	RandomAnim : 1;
		uint32	SequenceAnim : 1;

		uint32	ApplyDisplacement : 1;
		uint32	AnimPlayed : 1;

		SAnimationStatus( void )
		{
			LoopAnim = false;
			CodeAnim = false;
			RandomAnim = false;
			SequenceAnim = false;

			AnimPlayed = false;
			ApplyDisplacement = false;
		}
	};

	/// Constructor for random anim
	CAnimatedSceneObject( const std::string& ObjectName, const string& ClusterIG, const std::string& MeshName, const std::string& SkeletonName, const std::list < std::string >& Animations, const NLMISC::CVector& Position, const NLMISC::CQuat& Rotation, float Distance );

	/// Constructor for moving object, it not create entity (mesh, skeleton) but build animation on pre-exist entity
	CAnimatedSceneObject( const NLMISC::CEntityId& Id, const std::list < std::string >& Animations );

	// Constructor for moving object, init by UTransform
	CAnimatedSceneObject( const NLMISC::CEntityId& Id, NL3D::UTransform *Transform, const std::list < std::string >& Animations, bool displacement = true );

	/// Destructor
	~CAnimatedSceneObject();

	// Start animation
	void InitApplyDisplacement ( void );
	void EndApplyDisplacement ( void );

	void StartRandomAnimation( double time, float speedFactor = 1.0f );
	void StartNextAnimation( double time, float speedFactor = 1.0f );
	void StartAnimation( string& nameAnim, double time, float speedFactor = 1.0f );
	void removeOffsetAnimation ();

	// Add an animation to sequence
	void AddAnimationToSequence( const std::string& AnimName );

	// Process logic and displacement and setup visual animation
	void applyObjectAnimationTimeslice( double time );

	// manage animation start, if they are many animation, we choose one randomly (TODO: add control to animation launched and manage some mode (loop, repeat number, sequences..)
	void manageObjectAnimation( double Time, NLMISC::CVector userPos );

	// update (add to end list) Animation sequence with sequence
	void updateAnimationSequence( const std::list< std::string >& sequence );

	// return Id of object
	inline const NLMISC::CEntityId& getId( void ) { return _Id; }

	// Return or Set Id of animation sequence
	inline void idSequence( uint32 id) { _IdSequence = id; }
	inline uint32 idSequence( void ) { return _IdSequence; }

	inline void idAnimation( sint32 i ) { _IdAnimationPlayed = i; }

	// return a reference on status
	inline SAnimationStatus& getStatus( void) { return _Status; }

	// Apply rotation with compute delta rotion since last call
	void applyDeltaRotationAndPosition( NLMISC::CQuat& rot, NLMISC::CVector& trans );

	// Reset list animation
	inline void resetSequence( void ) { _SequenceAnimation.clear(); }

	// break current animation
	inline void breakCurrentAnimation ( void ) { _IdAnimationPlayed = -1; }

	// Reset intial pos and rot matrix
	void resetInitialPos( void );

	// Process sound logic temporary
	void processLogicTemp( double startOffset, double endOffset );

private:
	NLMISC::CEntityId						_Id;
	uint32									_IdSequence;
	std::string								_ObjectName;
	std::string								_ClusterIG;
	std::string								_MeshName;
	std::string								_SkeletonName;
	std::map< std::string, uint >			_MapAnimation;
	std::list< std::string >				_SequenceAnimation;
	NLMISC::CVectorD						_Position;
	NLMISC::CQuat							_Rotation;
	float									_Distance;
	SAnimationStatus						_Status;
	double									_StartAnimationTime;
	sint									_IdAnimationPlayed;
	NLMISC::CVector							_PreviousTranslation;
	NLMISC::CQuat							_PreviousRotation;
	double									_OldAnimationTime;
	// Only used in applydisplacement mode
	NLMISC::CMatrix							_AnimTransformation;	// Only used in applydisplacement mode


	NL3D::UPlayListManager					*_PlayListManager;
	NL3D::UAnimationSet						*_AnimationSet;
	NL3D::UPlayList							*_PlayList;
	NL3D::UInstance							_Instance;
	NL3D::USkeleton							_Skeleton;
	NL3D::UTransform						_Transform;
};

typedef std::map< std::string, CAnimatedSceneObject * > TMapCAnimatedSceneObject;
extern TMapCAnimatedSceneObject AnimatedSceneObject;

void addAnimatedSceneObject( const std::string& AnimatedSceneObjectName, const string& ClusterIG, const std::string& MeshName, const std::string& SkeletonName, const std::list <std::string>& Animations, const NLMISC::CVector& Position, const NLMISC::CQuat& Rotation, float Distance );
void addEntityClAnimatedSceneObject( const NLMISC::CEntityId& Id, const std::list < std::string >& Animations );
void addUknowTypeSceneObject( const NLMISC::CEntityId& id, NL3D::UTransform *Transform, const std::list < std::string >& Animations, bool displacement = true );
void updateAnimationSequence( const NLMISC::CEntityId& id, const std::list< std::string >& sequence, uint32 idSequence );
void deleteAnimatedSceneObject( const std::string& AnimatedSceneObjectName );
void manageAnimatedSceneObject( double Time );
void sendEndSequenceMessage( const NLMISC::CEntityId& id, uint32 idSequence );
// Reset animation for the entity and break the current one.
void resetAnimatedSceneObject(const NLMISC::CEntityId& id);

#endif // CL_ANIMATED_SCENE_OBJECT_H

/* End of animated_scene_object.h */
