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




#ifndef CL_PROJECTILE_MANAGER_H
#define CL_PROJECTILE_MANAGER_H

#include "nel/misc/vector.h"
#include "nel/3d/u_particle_system_instance.h"
#include "game_share/magic_fx.h"
#include "animation_fx.h"
#include "attack_info.h"
#include "attached_fx.h"
#include <set>




namespace NL3D
{
	class UParticleSystemInstance;
}


class CEntityCL;
class CAnimationFX;
class CCharacterCL;

/** Target of a projectile
  */
class CProjectileTarget
{
public:
	uint				Slot;			   // Slot of the target, if invalid, 'TargetPos' is used instead
	NLMISC::CVector		TargetPos;		   // position of the impact (if target slot invalid now, or later)
};




/** describes a single projectile
  * should be use with a call to CProjectileManager::createProjectileSet
  */
class CProjectileBuild
{
public:
	CAttackInfo						AttackInfo;
	MAGICFX::TSpellMode				Mode;                   // mode of the projectile (trajectory's shape depends on it)
	const CAnimationFXSet			*CastAspect;            // additionnal cast created when projectile is thrown
	uint							CastPower;              // applies to CastAspect only
	const CAnimationFXSet			*ProjectileAspect;
	const CAnimationFXSet			*ImpactAspect;
	CFXStickMode					ProjectileAimingPoint;  // the point on target that the projectile will try to reach (not necessarily the point where impact fx is played)
	bool							LocalizedImpact;
	CProjectileTarget				Target;
	double							StartDate;
	double							EndDate;
	bool							MagicResist;  // did the target resist to magic attack ?
	bool							PlayImpactAnim;
	bool							ForcePlayImpact;
	bool							LetProjectileStickedOnTarget;
	CAttachedFX::CTargeterInfo		TargeterInfo;
public:
	CProjectileBuild() : CastAspect(NULL),
	                     ProjectileAspect(NULL),
						 ImpactAspect(NULL),
						 ForcePlayImpact(false)
	{}
};
// for insertion in map / set
inline bool operator < (const CProjectileBuild &lhs, const CProjectileBuild &rhs)
{
	return lhs.StartDate < rhs.StartDate;
}


/** Class that manage all projectiles.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CProjectileManager
{
public:
	// get the unique instance of this class
	static CProjectileManager &getInstance();

	/** Add a new projectile in the queue. The 'StartDate' field gives its date of creation
	  */
	void addProjectileToQueue(const CProjectileBuild &pb);

	// tells the manager that a slot has become invalid
	void entityRemoved(uint slot);

	// update all projectiles
	void update();

	// reset the manager (this delete all projectiles)
	void reset();

	// eval position of a fx on an entity for a given stick mode
	static void	evalFXPosition(const CFXStickMode *stickMode, CEntityCL &entity, NLMISC::CVector &result, const NLMISC::CVector &additionnalOffset = NLMISC::CVector::Null);

	// return a set of 4 fx user param values for a projectile depending on its power
	static const float *getProjectileFXUserParams(uint power);

//////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** A projectile
	  * It targets an entity.
	  * When the entity is reached by the projectile, it creates impact on it (or play the resist effect)
	  * (bomb, chain & spray)
	  */
	class CProjectile
	{
	public:
		enum { MaxNumFX = CAnimationFXSet::MaxNumFX };
	public:
		CAttackInfo						AttackInfo;
		// aspect of projectile & impact
		const CAnimationFXSet			*ProjectileAspect;
		const CAnimationFXSet			*ImpactAspect;
		CFXStickMode					ProjectileAimingPoint;
		bool							LocalizedImpact;
		// instances of fx that represents the projectile
		NL3D::UParticleSystemInstance	FX[MaxNumFX];
		// mode of the projectile (used to compute trajectory)
		MAGICFX::TSpellMode				Mode;
		//
		NLMISC::CVector					LastEntityOrientation;				  // this matrix is updated when the target is a bone
		NLMISC::CVector					ClippedTargetBoneRelativePos;         // relative position of the target bone to the skeleton when it was last clipped
		bool							TargetBoneVisibleLastFrame;			  // Was target bone visible during the previous frame ?
		float							TargetBoneRelativePosBlendFactor;     // Blend factor from target bone relative pos (0.f) to clipped target bone relative pos (1.f)
																			  // If the target was clipped and is now visible, its bones may not have been updated for long.
																			  // So we blend between the clipped pos and the visible pos to avoid a 'jump' in the trajectory of the projectile
		//
		CProjectileTarget				Target;
		sint							TargetBoneID;				    // cache target bone id (if impact point is sticked to a bone of the target)
		//
		float							ParabolaHeight;         // height of parabola for bomb missiles
		double							StartDate;
		double							EndDate;
		//
		bool							MagicResist;			// did the target resist ? if so gives the power for resist (0-> no resist)
		bool							PlayImpactAnim;         // impact anim is not necessary for curative spells
		bool							ForcePlayImpact;
		bool							LetProjectileStickedOnTarget;
		//
		CAttachedFX::CTargeterInfo		TargeterInfo;

	public:
		// ctor
		CProjectile();
		// dtor
		~CProjectile();
		// get the target bone (impact point must be sticked to a bone of the target), or NULL if not found
		sint getImpactBoneID(CEntityCL &target);
		// update position of the target
		void updateTargetPos();
		// set position of all fx in the projectile
		void setMatrix(const NLMISC::CMatrix &mat);
		/** Shutdown all fxs and add them to the fx manager if entity == NULL (so that their instance is deleted
		  * when there are no particle left)
		  * If entity provided is not NULL, then the entity will take ownership of shuttingdown FX (useful
		  * if there's an effect triggered at impact in the projectile itself, and if that effect is sticked
		  * to some bone of the target)
		  */
		void shutDown(CCharacterCL *target);
		//
		void playImpactAnimOnAddtionnalTargets();
	};	//


	// set of projectile to create later
	std::list<CProjectile>					_ActiveProjectiles;		// currently active projectiles
	std::list<CProjectile>					_ToShutDownProjectiles;	// Projectiles that must be shutdown.
																	// We can't directly shut projectile down at the impact, because
																	// they can create trail -> final position (impact pos) is reached and, then emitters
																	// are shutdown -> the last segment of the particle trail between 'last pos' and 'impact pos'
	                                                                // isn't generated at fx update, simply because emitters are off, and this create the impression
	                                                                // that the projectile at stopped too early, so we delay the shutdown of one frame.

	std::multiset<CProjectileBuild>			_ProjectileQueue;		// projectiles that remains to be built (sorted by spawn date)


private:
	// update projectile queue, creating projectiles that need to be.
	void updateProjectileQueue();
	// create a projectile from its description, and add to the list of projectiles
	void addProjectile(const CProjectileBuild &pb);
	// clear a list of projectile (and delet fx instantlty)
	static void	resetProjList(std::list<CProjectile> &projList);
	//
	const CAnimationFX *getResistFX(uint level);
};

#endif
