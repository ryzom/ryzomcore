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
#include "nel/misc/matrix.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_transform.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_bone.h"
#include "attached_fx.h"
#include "character_cl.h"
#include "user_entity.h"
#include "entities.h"
#include "time_client.h"
#include "fx_manager.h"

extern NL3D::UScene *Scene;


using namespace NLMISC;
using namespace NL3D;

extern CUserEntity	*UserEntity;

// *************************************************************************************
CAttachedFX::CAttachedFX()
{
	clear();
}

// *************************************************************************************
CAttachedFX::~CAttachedFX()
{
	clear();
}

// ***********************************************************************************************************************
void CAttachedFX::clear()
{
	if (!FX.empty())
	{
		if (StickMode == CFXStickMode::SpawnPermanent)
		{
			FXMngr.addFX(FX, 100.f);
		}
		else
		{
			Scene->deleteInstance(FX);
		}
	}
	FX = NULL;
	AniFX = NULL;
	TimeOutDate = FX_MANAGER_DEFAULT_TIMEOUT;
	StickMode = CFXStickMode::Follow;
	MaxAnimCount = 0;
	UserBoneID = ~0;
	TargeterUserBoneID = 0xff;
}

// ***********************************************************************************************************************
void CAttachedFX::create(CCharacterCL					&parent,
						 const CBuildInfo				&buildInfo,
					     const CTargeterInfo			&targeterInfo
						)
{
	clear();
	if (!buildInfo.Sheet) return;
	UParticleSystemInstance instance = buildInfo.Sheet->createMatchingInstance();
	// TODO nico : user params are not in the buildInfo, but are set by createMatchingInstance then by the caller (they could be set directly by createMatchingInstance)
	if (instance.empty()) return;
	create(parent, instance, buildInfo, targeterInfo);
}

// ***********************************************************************************************************************
void CAttachedFX::create(CCharacterCL					 &parent,
						 NL3D::UParticleSystemInstance   instance,
						 const CBuildInfo				 &buildInfo,
						 const CTargeterInfo			 &targeterInfo
					    )
{
	nlassert(buildInfo.Sheet);
	TimeOutDate = buildInfo.TimeOut;
	const CFXStickMode *stickMode = buildInfo.StickMode ? buildInfo.StickMode : &buildInfo.Sheet->Sheet->StickMode;
	if (!instance.empty())
	{
		instance.setClusterSystem(parent.getClusterSystem());
		instance.setTransformMode(NL3D::UTransformable::DirectMatrix);
		instance.show();
	}
	FX = instance;
	AniFX = buildInfo.Sheet;
	StickMode = stickMode->Mode;
	SpawnTime = TimeInSec;
	MaxAnimCount = buildInfo.MaxNumAnimCount;
	TargeterInfo = targeterInfo;
	switch(stickMode->Mode)
	{
		case CFXStickMode::StaticMatrix:
		{
			const CMatrix &staticMatrix = buildInfo.StaticMatrix ? *buildInfo.StaticMatrix : NLMISC::CMatrix::Identity;
			instance.setMatrix(staticMatrix);
			SpawnPos = staticMatrix.getPos();
		}
		break;
		case CFXStickMode::Spawn:
		case CFXStickMode::SpawnPermanent:
		{
			parent.alignFX(instance, parent.getScalePos());
			SpawnPos = parent.pos() + buildInfo.StickOffset;
			if (stickMode->Mode == CFXStickMode::SpawnPermanent)
			{
				instance.forceInstanciate();
			}
		}
		break;
		case CFXStickMode::UserBone:
		{
			if (parent.skeleton())
			{
				sint boneID = parent.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(stickMode->UserBoneName));
				UserBoneID = boneID;
				if (boneID != -1)
				{
					parent.skeleton()->stickObjectEx(instance, boneID, true);
					NLMISC::CMatrix mat = instance.getMatrix();
					mat.scale(buildInfo.Sheet->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
					mat.setPos(buildInfo.StickOffset);
					if (!instance.empty()) instance.setMatrix(mat);
					SpawnPos = buildInfo.StickOffset;
					// if parent is hidden, then force to compute the bone at least once
					if (parent.skeleton()->getVisibility() == NL3D::UTransform::Hide)
					{
						parent.forceEvalAnim();
						// force to compute fx at least once
						parent.skeleton()->forceComputeBone(UserBoneID);
					}
					break;
				}
			}
			// bone not found or no skeleton
			if (!parent.instance().empty())
			{
				instance.parent(parent.instance());
				SpawnPos = buildInfo.StickOffset;
			}
			else
			{
				// just spawn at position of entity
				SpawnPos = parent.pos() + buildInfo.StickOffset;
			}
		}
		break;
		case CFXStickMode::UserBoneOrientedTowardTargeter:
			if (parent.skeleton())
			{
				UserBoneID = parent.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(stickMode->UserBoneName));
				if (UserBoneID == 0xff)
				{
					nlwarning("Bad bone name : %s", NLMISC::CStringMapper::unmap(stickMode->UserBoneName).c_str());
				}
			}
			else
			{
				UserBoneID = 0xff;
			}
			SpawnPos = buildInfo.StickOffset;
			update(parent, CMatrix::Identity);
		break;
		case CFXStickMode::UserBoneRay:
			if (parent.skeleton())
			{
				UserBoneID = parent.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(stickMode->UserBoneName));
				if (UserBoneID == 0xff)
				{
					nlwarning("Bad bone name : %s", NLMISC::CStringMapper::unmap(stickMode->UserBoneName).c_str());
				}
			}
			else
			{
				UserBoneID = 0xff;
			}
			SpawnPos = buildInfo.StickOffset;
			TargeterUserBoneID =0xff;
			if (targeterInfo.StickMode.UserBoneName != 0)
			{
				CEntityCL *targeter =  EntitiesMngr.entity(TargeterInfo.Slot);
				if (parent.skeleton() && targeter && targeter->skeleton())
				{
					TargeterUserBoneID = parent.skeleton()->getBoneIdByName(NLMISC::CStringMapper::unmap(TargeterInfo.StickMode.UserBoneName));
				}
			}
			update(parent, CMatrix::Identity);
		break;
		case CFXStickMode::OrientedTowardTargeter:
			SpawnPos = buildInfo.StickOffset;
			update(parent, CMatrix::Identity);
		break;
		default: // -> stick fx	in 'Follow' mode
			parent.alignFX(instance, parent.getScalePos());
			SpawnPos = buildInfo.StickOffset;
		break;
	}
}


// ***********************************************************************************************************************
void CAttachedFX::evalTargeterStickPos(NLMISC::CVector &dest) const
{
	CEntityCL *targeter = EntitiesMngr.entity(TargeterInfo.Slot);
	if (!targeter)
	{
		dest = TargeterInfo.DefaultPos;
		return;
	}
	switch(TargeterInfo.StickMode.Mode)
	{
		case  CFXStickMode::Spawn:
		case CFXStickMode::SpawnPermanent:
		case  CFXStickMode::Follow:
		case  CFXStickMode::FollowNoRotation:
		case  CFXStickMode::OrientedTowardTargeter:
			dest = targeter->pos();
		break;
		case  CFXStickMode::StaticMatrix:
			nlwarning("Not implemented"); // this case is not used for now
			dest = targeter->pos();
		break;
		case CFXStickMode::UserBoneOrientedTowardTargeter:
		case CFXStickMode::UserBoneRay:
		case CFXStickMode::UserBone:
			if (targeter->skeleton() && TargeterUserBoneID != 0xff)
			{
				const UBone bone = targeter->skeleton()->getBone(TargeterUserBoneID);
				targeter->forceEvalAnim();
				targeter->skeleton()->forceComputeBone(TargeterUserBoneID);
				dest = bone.getLastWorldMatrixComputed() * TargeterInfo.StickOffset;
			}
			else
			{
				dest = TargeterInfo.DefaultPos;
			}
		break;
	};
}

// ***********************************************************************************************************************
void CAttachedFX::update(CCharacterCL &parent, const NLMISC::CMatrix &alignMatrix)
{
	if (AniFX && !FX.empty())
	{
		NLMISC::CVector trackPos;
		// see if fx has a track applied on it
		if (AniFX->PosTrack)
		{
			// eval current pos
			AniFX->PosTrack->interpolate((float) (TimeInSec - SpawnTime), trackPos);
		}
		else
		{
			trackPos.set(0.f, 0.f, 0.f);
		}
		// apply pos depending on mode
		switch(StickMode)
		{
			case CFXStickMode::UserBone:
			{
				FX.setClusterSystem(parent.getClusterSystem());
				if (!AniFX->PosTrack)
				{
					// no track for fx,
					if (parent.skeleton() && UserBoneID != 0xff)
					{
						if (parent.skeleton()->getVisibility() == UTransform::Hide)
						{
							// if user no visible, force to compute the bone.
							parent.forceEvalAnim();
							parent.skeleton()->forceComputeBone(UserBoneID);
						}
					}
					return;
				}
				if (parent.skeleton())
				{
					if (UserBoneID != 0xff)
					{
						CMatrix mat = FX.getMatrix();
						mat.setPos(trackPos + SpawnPos);
						mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
						FX.setMatrix(mat);
						if (&parent == UserEntity && UserEntity->skeleton())
						{
							if (UserEntity->skeleton()->getVisibility() == UTransform::Hide)
							{
								// if user no visible, force to compute the bone.
								parent.forceEvalAnim();
								UserEntity->skeleton()->forceComputeBone(UserBoneID);
							}
						}
					}
				}
				// no skeleton or bone not found
				if (!parent.instance().empty())
				{
					CMatrix mat = FX.getMatrix();
					mat.setPos(trackPos + SpawnPos);
					mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
					FX.setMatrix(mat);
				}
				else
				{
					// no skeleton, no instance
					CMatrix mat = FX.getMatrix();
					mat.setPos(trackPos + SpawnPos + parent.pos());
					mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
					FX.setMatrix(mat);
				}
			}
			break;
			case CFXStickMode::Follow:
				// just change local pos
				parent.alignFX(FX, alignMatrix, AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f, trackPos + SpawnPos);
			break;
			case CFXStickMode::FollowNoRotation:
			{
				// just update the pos
				CMatrix mat = FX.getMatrix();
				mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
				mat.setPos(trackPos + parent.pos().asVector());
				FX.setMatrix(mat);
				FX.setClusterSystem(parent.getClusterSystem());
			}
			break;
			case CFXStickMode::StaticObjectCastRay:
			{
				// if not animated need no updates
				if (!AniFX->PosTrack) return;
				nlwarning("Not implemented");
			}
			break;
			case CFXStickMode::Spawn:
			case CFXStickMode::SpawnPermanent:
			{
				if (!AniFX->PosTrack) return;
				// put in local basis and offset spawn pos
				CMatrix mat = FX.getMatrix();
				mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
				mat.setPos(SpawnPos + FX.getMatrix().mulVector(trackPos));
				FX.setMatrix(mat);
				// Do not update the cluster system because fx stays in place
			}
			break;
			case CFXStickMode::OrientedTowardTargeter:
			{
				CEntityCL *targeter = EntitiesMngr.entity(TargeterInfo.Slot);
				if (targeter)
				{
					CVectorD orientD = parent.pos() - targeter->pos();
					CVector  J((float) orientD.x, (float) orientD.y, 0.f); // project on XY plane
					J.normalize();
					CVector I = J ^ CVector::K;
					CMatrix mat;
					mat.setRot(I, J, CVector::K);
					mat.setScale(AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f);
					mat.setPos(trackPos + parent.pos().asVector());
					FX.setMatrix(mat);
					FX.setClusterSystem(parent.getClusterSystem());
				}
			}
			break;
			case CFXStickMode::UserBoneOrientedTowardTargeter:
			{
				CEntityCL *targeter = EntitiesMngr.entity(TargeterInfo.Slot);
				if (targeter)
				{
					CVector orientD;
					CMatrix orientMat;
					if (UserBoneID == 0xff || !parent.skeleton())
					{
						// bone not found -> use parent position instead
						orientD = parent.pos() - targeter->pos();
						orientMat.setPos(trackPos + parent.pos());
					}
					else
					{
						const UBone bone = parent.skeleton()->getBone(UserBoneID);
						parent.forceEvalAnim();
						parent.skeleton()->forceComputeBone(UserBoneID);
						const CMatrix &wm = bone.getLastWorldMatrixComputed();
						// compute orientation toward targeter, and build a matrix from it
						orientD = wm.getPos() - targeter->pos().asVector();
						orientMat.setPos(trackPos + wm.getPos());
					}
					CVector  J(orientD.x, orientD.y, 0.f); // project on XY plane
					J.normalize();
					CVector I = J ^ CVector::K;
					float scale = AniFX->Sheet->ScaleFX ? parent.getScaleRef() : 1.f;
					orientMat.setRot(scale * I, scale * J, scale * CVector::K, true);
					FX.setMatrix(orientMat);
					FX.setClusterSystem(parent.getClusterSystem());
				}
			}
			break;
			case CFXStickMode::UserBoneRay:
			{
				CVector aimingPoint;
				evalTargeterStickPos(aimingPoint);
				CVector startPoint = CVector::Null;
				if (UserBoneID != 0xff && parent.skeleton())
				{
					const UBone bone = parent.skeleton()->getBone(UserBoneID);
					parent.forceEvalAnim();
					parent.skeleton()->forceComputeBone(UserBoneID);
					startPoint = bone.getLastWorldMatrixComputed().getPos();
				}
				CMatrix rayMat;
				CVector ray = aimingPoint - startPoint;
				CVector I = ray.normed();
				CVector K = (CVector::K - (I * CVector::K) * I).normed();
				CVector J = K ^ I;
				if (AniFX)
				{
					I *= ray.norm() / AniFX->Sheet->RayRefLength;
				}
				rayMat.setRot(I, J, K);
				rayMat.setPos(startPoint + 0.5f * AniFX->Sheet->RayRefLength * I);
				FX.setMatrix(rayMat);
				// don't clusterize ray, because it can be quite large
				FX.setForceClipRoot(true);

			}

		}
	}
}
