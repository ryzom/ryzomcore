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


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
// Client
#include "view.h"
#include "entities.h"
#include "time_client.h"
#include "pacs_client.h"
// 3d
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_instance_group.h"
#include "nel/pacs/u_global_position.h"
#include "motion/user_controls.h"


using NL3D::UVisualCollisionManager;
extern UVisualCollisionManager				*CollisionManager;

using namespace NLMISC;
using namespace std;


////////////
// GLOBAL //
////////////
CView View;


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CView :
// Constructor.
//-----------------------------------------------
CView::CView()
{
	_ViewPos = CVector::Null;
	_View = CVector::Null;
	_RefinePos = CVector::Null;
	_RearView=false;
	_CurrentCameraDist = 0.0f;
	_CurrentCameraSpeed = 0.0f;
	_CurrentCameraHeight = 2.0f;
	_MaxCameraHeight = 2.2f;
	_MinCameraHeight = 1.0f;
	_CollisionCameraDist = FLT_MAX;
	_ThirPersonClusterSystem= NULL;
	_ForceFirstPersonView= false;
	// For 3rd person camera collision
	_CameraCollisionDecal= 0.1f;
	_CameraCollisionThreshold= 0.2f;
	_CurrentCameraDistanceMax = ClientCfg.CameraDistMax;

}// CView //

//-----------------------------------------------
//-----------------------------------------------
void CView::update()
{
	if(UserEntity->viewMode() == CUserEntity::FirstPV)
	{
		_CurrentCameraSpeed = 0.0f;
		_CurrentCameraDist = 0.0f;
		_CollisionCameraDist = FLT_MAX;
		_ForceFirstPersonView= false;
	}
	else
	{
		////////////////////////////
		// Update Camera Distance //
		if(ClientCfg.CameraDistance > _CurrentCameraDist)
		{
			// Acceleration
			_CurrentCameraSpeed += ClientCfg.CameraAccel*DT;
			// Speed Limit.
			clamp(_CurrentCameraSpeed, ClientCfg.CameraSpeedMin, ClientCfg.CameraSpeedMax);
			// Adjust Camera Distance
			_CurrentCameraDist += _CurrentCameraSpeed*DT;
			if(_CurrentCameraDist > ClientCfg.CameraDistance)
				_CurrentCameraDist = ClientCfg.CameraDistance;
		}
		else if(ClientCfg.CameraDistance < _CurrentCameraDist)
		{
			// Acceleration
			_CurrentCameraSpeed -= ClientCfg.CameraAccel*DT;
			// Speed Limit.
			clamp(_CurrentCameraSpeed, -ClientCfg.CameraSpeedMax, -ClientCfg.CameraSpeedMin);
			// Adjust Camera Distance
			_CurrentCameraDist += _CurrentCameraSpeed*DT;
			if(_CurrentCameraDist < ClientCfg.CameraDistance)
				_CurrentCameraDist = ClientCfg.CameraDistance;
		}
		//////////////////////////
		// Update Camera Height //
		if(ClientCfg.CameraHeight > _CurrentCameraHeight)
		{
			// Acceleration
			_CurrentCameraSpeed += ClientCfg.CameraAccel*DT;
			// Speed Limit.
			clamp(_CurrentCameraSpeed, ClientCfg.CameraSpeedMin, ClientCfg.CameraSpeedMax);
			// Adjust Camera Distance
			_CurrentCameraHeight += _CurrentCameraSpeed*DT;
			if(_CurrentCameraHeight > ClientCfg.CameraHeight)
				_CurrentCameraHeight = ClientCfg.CameraHeight;
		}
		else if(ClientCfg.CameraHeight < _CurrentCameraHeight)
		{
			// Acceleration
			_CurrentCameraSpeed -= ClientCfg.CameraAccel*DT;
			// Speed Limit.
			clamp(_CurrentCameraSpeed, -ClientCfg.CameraSpeedMax, -ClientCfg.CameraSpeedMin);
			// Adjust Camera Distance
			_CurrentCameraHeight += _CurrentCameraSpeed*DT;
			if(_CurrentCameraHeight < ClientCfg.CameraHeight)
				_CurrentCameraHeight = ClientCfg.CameraHeight;
		}
	}
}// update //

//-----------------------------------------------
// currentViewPos :
// Set the user position.
//-----------------------------------------------
CVector CView::currentViewPos() const
{
	// clamp to the collisioned camera distance
	float	minCamDist= min(_CurrentCameraDist, _CollisionCameraDist);

	if(_RearView)
	{
		CVector v;
		if(UserEntity->viewMode() == CUserEntity::FirstPV || _ForceFirstPersonView)
		{
			CVector headPos;
			UserEntity->getHeadPos(headPos);
			return headPos;
		}
		else
		{
			// get the reverted view
			CVector v;
			v.x = -UserEntity->front().x;
			v.y = -UserEntity->front().y;
			v.z = 0.0f;
			v.normalize();
			// pos
			return UserEntity->pos() + CVector(0.0f, 0.0f, 2.0f) - v*minCamDist;
		}
	}
	else
	{
		if(UserEntity->viewMode() == CUserEntity::FirstPV || _ForceFirstPersonView)
			return _ViewPos + CVector(0.0f, 0.0f, UserEntity->eyesHeight());
		else
			return _ViewPos + CVector(0.0f, 0.0f, _CurrentCameraHeight) - _View*minCamDist;
	}
}// currentViewPos //

//-----------------------------------------------
// currentView :
//-----------------------------------------------
CVector CView::currentView() const
{
	if(_RearView)
	{
		CVector v;
		v.x = -UserEntity->front().x;
		v.y = -UserEntity->front().y;
		v.z = 0.0f;
		v.normalize();
		return v;
	}
	else
		return _View;
}// currentView //

NLMISC::CQuat CView::currentViewQuat() const
{
	CMatrix mat;
	mat.setRot(CVector::I, currentView(), CVector::K);
	mat.normalize(CMatrix::YZX);
	return mat.getRot();
}

//-----------------------------------------------
// currentCameraTarget :
//-----------------------------------------------
CVector CView::currentCameraTarget() const
{
	if(UserEntity->viewMode() == CUserEntity::FirstPV || _ForceFirstPersonView)
		return currentViewPos();
	else
	{
		if(_RearView)
		{
			return UserEntity->pos() + CVector(0.0f, 0.0f, 2.0f);
		}
		else
		{
			return _ViewPos + CVector(0.0f, 0.0f, _CurrentCameraHeight);
		}
	}
}

// update the max distance of the camera for player that want to play as dm (camera can be far)
void CView::setCameraDistanceMaxForDm()
{
	_CurrentCameraDistanceMax =  ClientCfg.DmCameraDistMax;
	if (!ClientCfg.FPV)
	{
		cameraDistance( std::min(ClientCfg.CameraDistance, _CurrentCameraDistanceMax) );
	}
}


// update the max distance of the camera for player that want to play as player (camera must be near)
void CView::setCameraDistanceMaxForPlayer()
{
	_CurrentCameraDistanceMax =  ClientCfg.CameraDistMax;
	if (!ClientCfg.FPV)
	{
		cameraDistance( std::min(ClientCfg.CameraDistance, _CurrentCameraDistanceMax) );
	}
}

//-----------------------------------------------
// cameraDistance :
// Change the distance from the user to the camera.
//-----------------------------------------------
void CView::cameraDistance(float dist)
{
	// Internal View
	if((dist < ClientCfg.CameraDistMin) && (dist <= ClientCfg.CameraDistance))
	{
		if (UserEntity && !UserEntity->isDead())
			UserEntity->viewMode(CUserEntity::FirstPV);
		ClientCfg.CameraDistance = dist;
	}
	// External View
	else
	{
		if (UserEntity && !UserEntity->isDead())
			UserEntity->viewMode(CUserEntity::ThirdPV);
		dist = std::max(dist, ClientCfg.CameraDistMin);
		ClientCfg.CameraDistance = std::min(dist, _CurrentCameraDistanceMax);
	}
}// cameraDistance //

//-----------------------------------------------
// changeCameraHeight
// Change the height of the camera
//-----------------------------------------------
void CView::changeCameraHeight(bool up, bool down)
{
	// If the user is not inside a building.
	if(!UserEntity->forceIndoorFPV())
	{
		if(up)
		{
			ClientCfg.CameraHeight += 0.5f;
		}
		else if(down)
		{
			ClientCfg.CameraHeight -= 0.2f;
		}
		// height limit.
		clamp(ClientCfg.CameraHeight, _MinCameraHeight, _MaxCameraHeight);
	}
}// changeCameraHeight //

//-----------------------------------------------
// changeCameraDist :
// Change the distance of the camera
//-----------------------------------------------
void CView::changeCameraDist(bool forward, bool backward)
{
	// If the user is not inside a building.
	if(!UserEntity->forceIndoorFPV())
	{
		if(forward)
			decreaseCameraDist();
		else if(backward)
			increaseCameraDist();
	}
}// changeCameraDist //

//-----------------------------------------------
// increaseCameraDist :
// Increase the distance between the user and the camera
//-----------------------------------------------
void CView::increaseCameraDist()
{
	// FPV -> just switch to TPV
	if(UserEntity->viewMode() == CUserEntity::FirstPV)
		cameraDistance(std::max(ClientCfg.CameraDistance, ClientCfg.CameraDistMin));
	// Backward
	else
		cameraDistance(ClientCfg.CameraDistance+ClientCfg.CameraDistStep);
}// increaseCameraDist //

//-----------------------------------------------
// decreaseCameraDist :
// Decrease the distance between the user and the camera
//-----------------------------------------------
void CView::decreaseCameraDist()
{
	// Forward only in third-person view
	if(UserEntity->viewMode() != CUserEntity::FirstPV)
		cameraDistance(ClientCfg.CameraDistance-ClientCfg.CameraDistStep);
}// decreaseCameraDist //

//-----------------------------------------------
// getCamera3rdPersonSetup:
//-----------------------------------------------
void CView::getCamera3rdPersonSetup(CVector &cameraStart, CVector &cameraEnd, CVector &cameraTestStart) const
{
	float	testStartDecal;
	getCamera3rdPersonSetupInternal(cameraStart, cameraEnd, cameraTestStart, testStartDecal);
}

//-----------------------------------------------
// getCamera3rdPersonSetupInternal:
//-----------------------------------------------
void CView::getCamera3rdPersonSetupInternal(CVector &cameraStart, CVector &cameraEnd, CVector &cameraTestStart, float &testStartDecal) const
{
	// get the camera path
	if(_RearView)
	{
		CVector v;
		v.x = - UserEntity->front().x;
		v.y = - UserEntity->front().y;
		v.z = 0.f;
		v.normalize();
		cameraStart= UserEntity->pos()+CVector(0.f,0.f,2.f);
		// add the threshold, to avoid discontinuity at f=1, when col start
		cameraEnd= cameraStart - v*(_CurrentCameraDist+_CameraCollisionThreshold);
	}
	else
	{
		cameraStart= _ViewPos + CVector(0.0f, 0.0f, _CurrentCameraHeight);
		// add the threshold, to avoid discontinuity at f=1, when col start
		cameraEnd= cameraStart - _View*(_CurrentCameraDist+_CameraCollisionThreshold);
	}

	// Avoid problem when player too near a wall. move back the start test
	CVector	vDir= cameraEnd - cameraStart;
	float	len = vDir.norm();
	testStartDecal= min(len, _CameraCollisionDecal);
	vDir.normalize();
	cameraTestStart= cameraStart + vDir*testStartDecal;
}

//-----------------------------------------------
// updateCameraCollision :
//-----------------------------------------------
void CView::updateCameraCollision()
{
	H_AUTO(RZ_Client_updateCameraCollision)

	// the radius of the cylinder to test
	const	float	colRadius= 0.4f;

	// default
	_CollisionCameraDist= FLT_MAX;
	bool	oldForceFPV= _ForceFirstPersonView;
	_ForceFirstPersonView= false;

	// if third person mode and CollisionManager available
	if( UserEntity->viewMode() != CUserEntity::FirstPV &&
		UserControls.mode() != CUserControls::AIMode &&
		CollisionManager)
	{
		// Get the Cluster system where the player (not the camera!) lies
		NLPACS::UGlobalPosition gPos;
		if(UserEntity->getPrimitive())
			UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
		// get the cluster IG associated to this pacs position
		NL3D::UInstanceGroup *pPlayerClusterSystem = getCluster(gPos);

		// For "matis serre bug", suppose the player is "inside" if walk on a cluster system
		CollisionManager->setPlayerInside(pPlayerClusterSystem!=NULL);

		// **** Compute the camera collision ray
		// get the camera path
		CVector	cameraStart, cameraEnd, cameraTestStart;
		float	testStartDecal;
		getCamera3rdPersonSetupInternal(cameraStart, cameraEnd, cameraTestStart, testStartDecal);

		// **** First do a single ray test from user to cameraTestStart
		// do the test only against landscape, to avoid problems
		// Use an approximate "pelvis", instead of getHeadPos(), because sometimes, even the head of the player
		// can enter in landscape. Thus this case will be tested, but the player will still see inside the landscape
		CVector pelvisPos;
		pelvisPos= _ViewPos + CVector(0.f, 0.f, 1.f);
		// if collide, then force first person view
		if(CollisionManager->getRayCollision(pelvisPos, cameraTestStart, true))
		{
			_ForceFirstPersonView= true;
		}
		else
		{
			_ForceFirstPersonView= false;

			// **** Clamp the camera according to collision 3D.
			float	f= CollisionManager->getCameraCollision(cameraTestStart, cameraEnd, colRadius, true);

			// if some collision found
			if(f<1)
			{
				// re-add the decalStart
				_CollisionCameraDist= testStartDecal +  (cameraEnd-cameraTestStart).norm() * f;
				// remove the threshold
				_CollisionCameraDist-= _CameraCollisionThreshold;
				clamp(_CollisionCameraDist, 0, _CurrentCameraDist);
			}

			// **** Ensure the position in cluster system
			// get the pos from compute above
			cameraStart= currentCameraTarget();
			cameraEnd= currentViewPos();

			// parse this ray against the cluster system
			CVector	precCameraEnd= cameraEnd;
			_ThirPersonClusterSystem= Scene->findCameraClusterSystemFromRay(pPlayerClusterSystem, cameraStart, cameraEnd);

			// Then modify the Camera distance
			if(precCameraEnd!=cameraEnd)
			{
				_CollisionCameraDist= (cameraEnd - cameraStart).norm();
				clamp(_CollisionCameraDist, 0, _CurrentCameraDist);
			}
		}

	}

	// if difference of mode, must update userentity
	if(oldForceFPV!=_ForceFirstPersonView)
		UserEntity->updateVisualDisplay();
}

