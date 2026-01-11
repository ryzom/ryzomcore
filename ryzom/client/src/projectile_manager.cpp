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
#include "projectile_manager.h"
#include "animation_fx.h"
#include "fx_manager.h"
#include "character_cl.h"
#include "entities.h"
#include "time_client.h"
//
#include "animation_fx.h"
//
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_bone.h"
//

H_AUTO_DECL(RZ_ProjectileManager)

static const float BOMB_MISSILE_PARABOLA_HEIGHT_FOR_100_METERS = 10.f;

// user params for projectile part
static const float ProjectileUserParams[MAGICFX::NUM_SPELL_POWER][4] =
{
	{ 0.f, 0.f, 0.f, 0.f },
	{ 1.f, 0.f, 0.f, 0.f },
	{ 1.f, 1.f, 0.f, 0.f },
	{ 1.f, 1.f, 1.f, 0.f },
	{ 1.f, 1.f, 1.f, 1.f }
};

// user params for impact part
static const float ImpactUserParams[MAGICFX::NUM_SPELL_POWER][4] =
{
	{ 0.f, 0.f, 0.f, 0.f },
	{ 0.f, 1.f, 0.f, 0.f },
	{ 1.f, 1.f, 0.f, 0.f },
	{ 1.f, 1.f, 1.f, 0.f },
	{ 1.f, 1.f, 1.f, 1.f }
};




// *****************************************************************************************
CProjectileManager &CProjectileManager::getInstance()
{
	H_AUTO_USE(RZ_ProjectileManager)
	static CProjectileManager manager;
	return manager;
}

// *****************************************************************************************
const float *CProjectileManager::getProjectileFXUserParams(uint power)
{
	if (power < 1 || power > MAGICFX::NUM_SPELL_POWER) return NULL;
	return &ProjectileUserParams[power - 1][0];
}


// *****************************************************************************************
void CProjectileManager::addProjectileToQueue(const CProjectileBuild &pb)
{
	H_AUTO_USE(RZ_ProjectileManager)
	_ProjectileQueue.insert(pb);
}

// *****************************************************************************************
void CProjectileManager::addProjectile(const CProjectileBuild &pb)
{
	H_AUTO_USE(RZ_ProjectileManager)
	//
	_ActiveProjectiles.push_front(CProjectile());
	CProjectile &proj                 = _ActiveProjectiles.front();
	proj.AttackInfo                   = pb.AttackInfo;
	proj.ProjectileAspect             = pb.ProjectileAspect;
	proj.ImpactAspect                 = pb.ImpactAspect;
	proj.ProjectileAimingPoint        = pb.ProjectileAimingPoint;
	proj.LocalizedImpact              = pb.LocalizedImpact;
	proj.Target                       = pb.Target;
	proj.Mode                         = pb.Mode;
	proj.TargetBoneVisibleLastFrame   = true;
	proj.StartDate                    = pb.StartDate;
	proj.EndDate                      = pb.EndDate;
	proj.MagicResist                  = pb.MagicResist;
	proj.PlayImpactAnim               = pb.PlayImpactAnim;
	proj.LetProjectileStickedOnTarget = pb.LetProjectileStickedOnTarget;
	proj.TargeterInfo				  = pb.TargeterInfo;
	proj.ForcePlayImpact              = pb.ForcePlayImpact;
	// eval start position
	CCharacterCL *casterEntity = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(pb.TargeterInfo.Slot));
	if (casterEntity)
	{
		proj.LastEntityOrientation = casterEntity->front();
		evalFXPosition(&pb.TargeterInfo.StickMode, *casterEntity, proj.TargeterInfo.DefaultPos, pb.TargeterInfo.StickOffset);
		// play	additionnal cast fx
		if (pb.CastAspect)
		{
			casterEntity->playCastFX(pb.CastAspect, pb.CastPower);
		}
	}
	else
	{
		proj.LastEntityOrientation = CVector::I;
	}
	CEntityCL *targetEntity = EntitiesMngr.entity(pb.Target.Slot);
	if (targetEntity)
	{
		evalFXPosition(&(pb.ProjectileAimingPoint), *targetEntity, proj.Target.TargetPos);
	}
	// else ... if target entity is not valid, default pos given in the target is used

	// create actual fxs (unless hit is direct)
	if (pb.EndDate > pb.StartDate)
	{
		if (pb.ProjectileAspect)
		{
			if (pb.AttackInfo.Intensity >= 1 && pb.AttackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER)
			{
				const CAnimationFXSet &projectile = *(pb.ProjectileAspect);
				for(uint k = 0; k < projectile.FX.size(); ++k)
				{
					const CAnimationFX &sheet = projectile.FX[k];
					proj.FX[k] = sheet.createMatchingInstance(ProjectileUserParams[pb.AttackInfo.Intensity - 1]);
					if (!proj.FX[k].empty())
					{
						// Force transform to be linked to the root, and not clusterized
						// This is because we don't compute the cluster system the projectile is in, and as a result,
						// projectile can't be seen when player is in a cluster system.
						// To compute the cluster system we would need a PACS primitive, and thus collisions.
						proj.FX[k].setForceClipRoot(true);
						//
						proj.FX[k].setTransformMode(NL3D::UTransform::DirectMatrix);
					}
				}
			}
		}
	}
	float dist = (proj.Target.TargetPos - proj.TargeterInfo.DefaultPos).norm();
	// eval height of parabola for 'bomb' missile (depends on distance)
	proj.ParabolaHeight = dist * 0.01f * BOMB_MISSILE_PARABOLA_HEIGHT_FOR_100_METERS;
}

// *****************************************************************************************
void CProjectileManager::updateProjectileQueue()
{
	H_AUTO_USE(RZ_ProjectileManager)
	std::multiset<CProjectileBuild>::iterator it = _ProjectileQueue.begin();
	while (it != _ProjectileQueue.end() && it->StartDate <= TimeInSec)
	{
		addProjectile(*it); // create projectile from its description
		_ProjectileQueue.erase(it);
		it = _ProjectileQueue.begin();
	}
}


// *****************************************************************************************
void CProjectileManager::update()
{
	H_AUTO_USE(RZ_ProjectileManager)
	// remove shutting down projectiles
	for(std::list<CProjectile>::iterator it = _ToShutDownProjectiles.begin(); it != _ToShutDownProjectiles.end(); ++it)
	{
		if (it->LetProjectileStickedOnTarget)
		{
			CCharacterCL  *target = NULL;
			if (it->Target.Slot != CLFECOMMON::INVALID_SLOT)
			{
				target = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(it->Target.Slot));
			}
			it->shutDown(target);
		}
		else
		{
			it->shutDown(NULL);
		}
	}
	_ToShutDownProjectiles.clear();
	//
	updateProjectileQueue();
	// update each projectile, and removes from list when impact occurs
	std::list<CProjectile>::iterator it = _ActiveProjectiles.begin();
	while (it != _ActiveProjectiles.end())
	{
		std::list<CProjectile>::iterator tmpIt = it;
		++ it;
		CProjectile &proj = *tmpIt;
		// Eval impact pos
		proj.updateTargetPos();
		CEntityCL *target = EntitiesMngr.entity(proj.Target.Slot);
		const CVector &impactPos = proj.Target.TargetPos;

		// compute position on segment joining start point to impact
		bool impact = false;
		double totalDeltaT = proj.EndDate - proj.StartDate;
		double deltaT = TimeInSec - proj.StartDate;
		double segmentRatio = totalDeltaT != 0 ? deltaT / totalDeltaT
										: 1;
		if (segmentRatio >= 1.f)
		{
			impact = true;
			segmentRatio = 1.f;
		}
		CVector deltaPos = impactPos - proj.TargeterInfo.DefaultPos;
		CVector projPos = proj.TargeterInfo.DefaultPos + (float) segmentRatio * deltaPos;
		// see what minimum distance should have been travelled
		float minDist = MAGICFX::PROJECTILE_SPEED * (float) deltaT;
		if (minDist * minDist > (projPos - proj.TargeterInfo.DefaultPos).sqrnorm())
		{
			float maxDist = deltaPos.norm();
			if (minDist > maxDist)
			{
				projPos = impactPos;
				impact = true;
				segmentRatio = 1.f;
			}
			else
			{
				segmentRatio = maxDist != 0.f ? minDist / maxDist : 1.f;
			}
		}
		////////////////////
		// update fx pos  //
		////////////////////

		// build fx matrix from the speed of projectile
		// For a bomb, Pos(t) = t * E + (1 - t) * S + h * (0 0 -4t^2 + 4t) with S = start point, E = end point, h is the height of parabola, and t ranging from 0 to 1
		// so The speed direction at a given date t is Speed(t) = dPos(t)/dt = E - S + h * (0 0 -8t + 4)
		// NB : anim track is not taken in account
		NLMISC::CVector I;
		if (proj.Mode == MAGICFX::Bomb)
		{
			I.set(deltaPos.x, deltaPos.y, deltaPos.z + proj.ParabolaHeight * (4.f - 8.f * (float) segmentRatio));
			I.normalize();
		}
		else
		{
			I = deltaPos.normed();
		}
		NLMISC::CVector K = (CVector::K - (CVector::K * I) * I).normed();
		NLMISC::CVector J = K ^ I;
		NLMISC::CMatrix m;
		m.setRot(I, J, K, true);

		// apply track (use track of first fx)
		if (proj.ProjectileAspect && !proj.ProjectileAspect->FX.empty())
		{
			NL3D::UTrack *track = proj.ProjectileAspect->FX[0].PosTrack;
			if (track)
			{
				NLMISC::CVector trackPos;
				if (track->interpolate((float) deltaT, trackPos))
				{
					projPos += m.mulVector(trackPos);
				}
			}
		}
		// if projectile is a bomb, uses a parabola for the trajectory
		if (proj.Mode == MAGICFX::Bomb)
		{
			// for simplicity, the parabola reach its max at the middle of the trajectory
			// so we got something like f(x) = -4x^2 + 4x
			projPos.z += (float) (proj.ParabolaHeight * (-4 * segmentRatio * segmentRatio + 4 * segmentRatio));
		}
		m.setPos(projPos);
		proj.setMatrix(m);
		// see if target reached
		if (impact || TimeInSec > proj.EndDate)
		{
			// stop projectiles and add them to the fx manager so that they are removed when there are no particle lefts
			//proj.shutDown(dynamic_cast<CCharacterCL *>(target));

			static std::vector<NL3D::UParticleSystemInstance> fxInstances;
			fxInstances.clear();
			// play impact on the target instantly
			if (target)
			{
				CCharacterCL *ccl = dynamic_cast<CCharacterCL *>(target);
				if (ccl)
				{
					if (proj.AttackInfo.Intensity >= 1 && proj.AttackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER)
					{
						if (!proj.MagicResist)
						{
							if (proj.ImpactAspect)
							{
								CAttachedFX::CBuildInfo bi;
								for (uint k = 0; k < proj.ImpactAspect->FX.size(); ++k)
								{
									CFXStickMode replaceStickMode = proj.ImpactAspect->FX[k].Sheet->StickMode;
									bi.StickMode = NULL;
									if (proj.LocalizedImpact)
									{
										nlassert(proj.ImpactAspect->Sheet);
										// impact must have same localization than impact damages (when the sheet allows it)
										if (proj.ImpactAspect->Sheet->CanReplaceStickMode[k])
										{
											replaceStickMode.Mode = proj.ProjectileAimingPoint.Mode;
										}
										if (proj.ImpactAspect->Sheet->CanReplaceStickBone[k])
										{
											replaceStickMode.UserBoneName = proj.ProjectileAimingPoint.UserBoneName;
										}
										bi.StickMode = &replaceStickMode;
									}
									CAttachedFX::TSmartPtr fx = new CAttachedFX;
									bi.Sheet = &(proj.ImpactAspect->FX[k]);
									fx->create(*ccl, bi, proj.TargeterInfo);
									if (!fx->FX.empty())
									{
										/** Force to create each instance, even if not visible, so that the impact won't be deleted at start because it isn't visible..
										  * Though this isn't needed in other cases, spell impact can be quite long. and must remain in sync with the action
										  */
										fx->FX.forceInstanciate();
										for(uint l = 0; l < 4; ++l)
										{
											fx->FX.setUserParam(l, ImpactUserParams[proj.AttackInfo.Intensity - 1][l]);
										}
										ccl->attachFX(fx);
									}
								}



								if (proj.PlayImpactAnim)
								{
									// if the target is idle, play 'impact' animation on it
									if (ccl->animState(MOVE) == CAnimationStateSheet::Idle || proj.ForcePlayImpact)
									{
										ccl->setAnim(CAnimationStateSheet::Impact);
									}
								}
							}
						}
						else
						{
							// play resist instead
							CAttachedFX::TSmartPtr  fx = new CAttachedFX;
							CAttachedFX::CBuildInfo bi;
							bi.Sheet = getResistFX(proj.AttackInfo.Intensity);
							fx->create(*ccl, bi, proj.TargeterInfo);
							if (!fx->FX.empty())
							{
								fx->FX.forceInstanciate(); // TODO : if the fx isn't too long, is it useful to force to create it ?
								ccl->attachFX(fx);
							}
						}
					}
				}
				// play physcial damage fx (if any)
				if (proj.AttackInfo.PhysicalImpactIntensity != 0)
				{
					ccl->meleeImpact(proj.AttackInfo);
				}
			}
			else
			{
				if (proj.AttackInfo.Intensity >= 1 && proj.AttackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER)
				{
					if (!proj.MagicResist)
					{
						if (proj.ImpactAspect)
						{
							// no entity to spawn fx on, so create them in place
							for(uint k = 0; k <  proj.ImpactAspect->FX.size(); ++k)
							{
								NL3D::UParticleSystemInstance ps = proj.ImpactAspect->FX[k].createMatchingInstance(ImpactUserParams[proj.AttackInfo.Intensity - 1]);
								if (!ps.empty())
								{
									ps.forceInstanciate();
									ps.setPos(impactPos);
									// add to fx manager for fx to be removed when finished
									FXMngr.addFX(ps);
								}
							}
						}
					}
					else
					{
						NL3D::UParticleSystemInstance ps = getResistFX(proj.AttackInfo.Intensity)->createMatchingInstance();
						if (!ps.empty())
						{
							ps.forceInstanciate(); // TODO : resist can be can be quite short, so may be not useful to force its instanciation ?
							ps.setPos(impactPos);
							// add to fx manager for fx to be removed when finished
							FXMngr.addFX(ps);
						}
					}
				}
				// no target, so no impact
			}
			// put in shutdown list (shutdown occurs at next frame)
			_ToShutDownProjectiles.splice(_ToShutDownProjectiles.begin(), _ActiveProjectiles, tmpIt);
		}
	}
}

// *****************************************************************************************
CProjectileManager::CProjectile::CProjectile()
{
	H_AUTO_USE(RZ_ProjectileManager)
	for(uint k = 0; k < MaxNumFX; ++k) FX[k] = NULL;
	TargetBoneRelativePosBlendFactor = 0.f;
	TargetBoneID = -1;
	ParabolaHeight = 0.f;
	MagicResist = false;
	LocalizedImpact = false;
}

// *****************************************************************************************
CProjectileManager::CProjectile::~CProjectile()
{
	H_AUTO_USE(RZ_ProjectileManager)
	if (Scene)
	{
		for(uint k = 0; k < MaxNumFX; ++k)
		{
			if (!FX[k].empty())
			{
				Scene->deleteInstance(FX[k]);
				FX[k] = NULL;
			}
		}
	}
}

// *****************************************************************************************
void CProjectileManager::evalFXPosition(const CFXStickMode *stickMode, CEntityCL &entity, NLMISC::CVector &result, const NLMISC::CVector &additionnalOffset /* = NLMISC::CVector::Null*/)
{
	H_AUTO_USE(RZ_ProjectileManager)
	// Force to eval animation if necessary
	entity.forceEvalAnim();

	if (!stickMode)
	{
		CFXStickMode sm;
		evalFXPosition(&sm, entity, result);
	}
	else
	{
		switch(stickMode->Mode)
		{
			case CFXStickMode::UserBone:
			{
				if (entity.skeleton())
				{
					sint boneID = entity.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(stickMode->UserBoneName));
					if (boneID != -1)
					{
						if (!entity.skeleton()->isBoneComputed(boneID))
						{
							entity.skeleton()->forceComputeBone(boneID);
						}
						NL3D::UBone bone = entity.skeleton()->getBone(boneID);
						const NLMISC::CMatrix &worldMat = bone.getLastWorldMatrixComputed();
						result = worldMat.getPos() + worldMat.mulVector(additionnalOffset);
						return;
					}
					else
					{
						nlwarning("Invalid bone : %s. Using entity pos instead", NLMISC::CStringMapper::unmap(stickMode->UserBoneName).c_str());
					}
				}
				else
				{
					nlwarning("Can't retrieve entity skeleton. Using entity pos instead of bone pos", NLMISC::CStringMapper::unmap(stickMode->UserBoneName).c_str());
				}
				// as default, use entity position
				entity.pos().copyTo(result);
				result += additionnalOffset;
			}
			break;
			default:
			{
				// use entity position
				entity.pos().copyTo(result);
				result += additionnalOffset;
			}
			break;
		}
	}
}

// *****************************************************************************************
sint CProjectileManager::CProjectile::getImpactBoneID(CEntityCL &target)
{
	H_AUTO_USE(RZ_ProjectileManager)
	if (TargetBoneID != -1) return TargetBoneID;
	if (!target.skeleton()) return -1;
	sint TargetBoneID = target.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(ProjectileAimingPoint.UserBoneName));
	return TargetBoneID;
}

// *****************************************************************************************
void CProjectileManager::CProjectile::updateTargetPos()
{
	H_AUTO_USE(RZ_ProjectileManager)
	NLMISC::CVector &impactPos = Target.TargetPos;
	CEntityCL *target = EntitiesMngr.entity(Target.Slot);
	if (target)
	{
		if (ProjectileAimingPoint.Mode == CFXStickMode::UserBone ||
			ProjectileAimingPoint.Mode == CFXStickMode::UserBoneOrientedTowardTargeter
		   )
		{
			bool targetBoneVisible = false;
			if (!target->getLastClip())
			{
				// target visible
				sint impactBoneID = getImpactBoneID(*target);
				if (impactBoneID != -1)
				{
					if (target->skeleton() && target->skeleton()->isBoneComputed(impactBoneID))
					{
						NL3D::UBone bone = target->skeleton()->getBone(impactBoneID);
						if (TargetBoneRelativePosBlendFactor == 0.f)
						{
							impactPos = bone.getLastWorldMatrixComputed().getPos();
						}
						else
						{
							// do blend with previous clipped position
							// express last clipped position in world (taking in account the ne orientation & position of model)
							const NLMISC::CVector &relPos = ClippedTargetBoneRelativePos;
							const NLMISC::CVector &front = target->front();
							NLMISC::CVector newEntityPos;
							NLMISC::CVector clippedPos;
							target->pos().copyTo(newEntityPos);
							clippedPos.set(newEntityPos.x + front.y * relPos.x - front.x * relPos.y,
										   newEntityPos.y + front.x * relPos.x + front.y * relPos.y,
										   newEntityPos.z + relPos.z);
							// do the blend
							const NLMISC::CVector &visiblePos = bone.getLastWorldMatrixComputed().getPos();
							impactPos  = TargetBoneRelativePosBlendFactor * clippedPos
										 + (1.f - TargetBoneRelativePosBlendFactor) * visiblePos;
							// decrease blend factor over time
							const float BLEND_FATCOR_DECREASE_OVER_TIME = 3.f;
							TargetBoneRelativePosBlendFactor -= DT * BLEND_FATCOR_DECREASE_OVER_TIME;
							if (TargetBoneRelativePosBlendFactor <= 0.f) TargetBoneRelativePosBlendFactor = 0.f;
						}
						targetBoneVisible = true;
					}
				}
			}
			if (!targetBoneVisible)
			{
				// the target bone is not visible -> use last computed bone position (and make it relative to the parent skeleton)
				if (TargetBoneVisibleLastFrame)
				{
					// the relative position of the bone in its skeleton hasn't been computed yet (because the bone was visible at last frame or because it is the first visible frame)
					const NLMISC::CVector &front = LastEntityOrientation;
					const NLMISC::CVector deltaPos = impactPos - target->lastFramePos().asVector();
					// we except that front vector of the entity is normed and (thus its z is 0)
					ClippedTargetBoneRelativePos.set(front.y * deltaPos.x - front.x * deltaPos.y, deltaPos.x * front.x + deltaPos.y * front.y, deltaPos.z);
				}
				// Snap the target model if necessary. Note that the skeleton may be visible the bone may have been not computed (because of clod)
				if (target->getLastClip())
				{
					target->snapToGround();
				}
				// compute new position
				const NLMISC::CVector &relPos = ClippedTargetBoneRelativePos;
				const NLMISC::CVector &front = target->front();
				NLMISC::CVector newPos;
				target->pos().copyTo(newPos);
				impactPos.set(newPos.x + front.y * relPos.x - front.x * relPos.y,
							  newPos.y + front.x * relPos.x + front.y * relPos.y,
							  newPos.z + relPos.z);
				TargetBoneRelativePosBlendFactor = 1.f;
			}

			TargetBoneVisibleLastFrame = targetBoneVisible;
			LastEntityOrientation = target->front();
		}
		else
		{
			// just uses entity pos and snap if necessary
			if (target->getLastClip())
			{
				target->snapToGround(); // TODO : avoid to do this each frame for targetted non-visible entities ?
			}
			target->pos().copyTo(impactPos);
		}
	}
	// else use value already present in impactPos
}

// *****************************************************************************************
void CProjectileManager::CProjectile::setMatrix(const NLMISC::CMatrix &pos)
{
	H_AUTO_USE(RZ_ProjectileManager)
	for(uint k = 0; k < MaxNumFX; ++k)
	{
		if (!FX[k].empty())
		{
			FX[k].setMatrix(pos);
		}
	}
}

static void convertFreeFXToAttachedFX(NL3D::UParticleSystemInstance  instance,
								     CCharacterCL &holder,
								     float timeOut,
								     const CAnimationFX *aspect,
								     const CFXStickMode &stickMode)
{
	instance.setForceClipRoot(false);
	CAttachedFX::TSmartPtr af = new CAttachedFX;
	CAttachedFX::CTargeterInfo ti;
	CAttachedFX::CBuildInfo bi;
	bi.Sheet = aspect;
	bi.StickMode = &stickMode;
	bi.TimeOut = timeOut;
	af->create(holder, instance, bi, ti);
	holder.attachFX(af);
}

// *****************************************************************************************
void CProjectileManager::CProjectile::shutDown(CCharacterCL *target)
{
	H_AUTO_USE(RZ_ProjectileManager)
	for(uint k = 0; k < MaxNumFX; ++k)
	{
		if (!FX[k].empty())
		{
			const float FX_DEFAULT_TIMEOUT = 15.f;
			const float FX_ERROR_TIMEOUT = 3.f;
			// must set user matrix of target, if an emitter emit 'on death' using 'user' matrix
			if (target)
			{
				NLMISC::CMatrix userMatrix;
				target->buildAlignMatrix(userMatrix);
				FX[k].forceSetUserMatrix(userMatrix);
			}
			if (!FX[k].removeByID(NELID("STOP")) && !FX[k].removeByID(NELID("main")))
			{
				FX[k].activateEmitters(false);
				//nlwarning("Projectile with a particle system that has no 'STOP' emitter");
				if (target)
				{
					convertFreeFXToAttachedFX(FX[k], *target, FX_ERROR_TIMEOUT, ProjectileAspect ? &ProjectileAspect->FX[k] : NULL, ProjectileAimingPoint);
				}
				else
				{
					FXMngr.addFX(FX[k], FX_ERROR_TIMEOUT); // use  short timeout to avoid problems
				}
			}
			else
			{
				if (target)
				{
					convertFreeFXToAttachedFX(FX[k], *target, FX_DEFAULT_TIMEOUT, ProjectileAspect ? &ProjectileAspect->FX[k] : NULL, ProjectileAimingPoint);
				}
				else
				{
					FXMngr.addFX(FX[k], FX_DEFAULT_TIMEOUT); // gives a time out to avoid remaining particles if the fx main emitter has not been flaged with 'STOP'
				}
			}
			FX[k] = NULL;
		}
	}
}

// *****************************************************************************************
void CProjectileManager::reset()
{
	H_AUTO_USE(RZ_ProjectileManager)
	_ProjectileQueue.clear();
	_ActiveProjectiles.clear();
	_ToShutDownProjectiles.clear();
}

// *****************************************************************************************
void CProjectileManager::entityRemoved(uint slot)
{
	// TODO nico : adding anchor objects  (that can be "CRefPtr'ed" ...) to define rays / projectile paths would be a more elegant solution now (and a CAnchorRef object to dlte anchor automatically)

	H_AUTO_USE(RZ_ProjectileManager)
	CEntityCL *entity = EntitiesMngr.entity(slot);
	if (!entity) return;
	// projectile queue.
	for(std::multiset<CProjectileBuild>::iterator it = _ProjectileQueue.begin(); it != _ProjectileQueue.end(); ++it)
	{
		// note : we do not change field that would modify ordering in the set (but const_cast is needed there anyway ..)
		CProjectileBuild &pb = const_cast<CProjectileBuild &>(*it);
		if (it->TargeterInfo.Slot == slot)
		{
			// eval start pos
			evalFXPosition(&it->TargeterInfo.StickMode, *entity, pb.TargeterInfo.DefaultPos); // note : insertion in the set depends not on this field, hence the const_cast
			pb.TargeterInfo.Slot = CLFECOMMON::INVALID_SLOT;
		}
		if (it->Target.Slot == slot)
		{
			// eval target pos
			evalFXPosition(&pb.ProjectileAimingPoint, *entity, pb.Target.TargetPos); // note : insertion in the set depends not on this field, hence the const_cast
			pb.TargeterInfo.Slot = CLFECOMMON::INVALID_SLOT;
		}
	}
	// currently active projectile.
	for(std::list<CProjectile>::iterator it = _ActiveProjectiles.begin(); it != _ActiveProjectiles.end(); ++it)
	{
		if (it->Target.Slot == slot)
		{
			// eval target pos
			evalFXPosition(&it->ProjectileAimingPoint, *entity, it->Target.TargetPos);
			it->Target.Slot = CLFECOMMON::INVALID_SLOT;
		}
		// caster slot
		if (it->TargeterInfo.Slot == slot)
		{
			it->TargeterInfo.Slot = CLFECOMMON::INVALID_SLOT;
		}
	}
}

// *****************************************************************************************
const CAnimationFX *CProjectileManager::getResistFX(uint level)
{
	H_AUTO_USE(RZ_ProjectileManager)
	// TMP TMP : harcoded datas for the resist spell
	static const char *impactResistFXName = "mag_def_impact_resist.ps";

	// user params of the resist fx for each level
	static float resistFXUserParams[MAGICFX::NUM_SPELL_POWER][4] =
	{
		{ 0.f, 1.f, 0.f, 0.f},
		{ 0.f, 1.f, 0.f, 0.f},
		{ 1.f, 1.f, 0.f, 0.f},
		{ 1.f, 1.f, 1.f, 0.f},
		{ 1.f, 1.f, 1.f, 1.f}
	};

	// sheet of the resist spell (hardcoded for now)
	static CAnimationFXSheet impactResistSheet[MAGICFX::NUM_SPELL_POWER] =
	{
		CAnimationFXSheet(impactResistFXName, resistFXUserParams[0]),
		CAnimationFXSheet(impactResistFXName, resistFXUserParams[1]),
		CAnimationFXSheet(impactResistFXName, resistFXUserParams[2]),
		CAnimationFXSheet(impactResistFXName, resistFXUserParams[3]),
		CAnimationFXSheet(impactResistFXName, resistFXUserParams[4])
	};

	static CAnimationFX impactResistFX[MAGICFX::NUM_SPELL_POWER];

	static bool init = false;

	if (!init)
	{
		for(uint k = 0; k < MAGICFX::NUM_SPELL_POWER; ++k)
		{
			impactResistFX[k].init(&impactResistSheet[k], NULL);
		}
		init = true;
	}
	if (level >= 1 && level <= MAGICFX::NUM_SPELL_POWER)
	{
		return &impactResistFX[level - 1];
	}
	return NULL;
}


































































