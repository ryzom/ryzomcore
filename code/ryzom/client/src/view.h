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



#ifndef NL_VIEW_H
#define NL_VIEW_H


/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


///////////
// USING //
///////////
using NLMISC::CVector;


////////////
// GLOBAL //
////////////
extern class CView View;


///////////
// CLASS //
///////////
/**
 * Class to manage the vision.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CView
{
public:
	float _CurrentCameraSpeed;
	float _CurrentCameraDist;
	float _CurrentCameraHeight;
	float _MaxCameraHeight;
	float _MinCameraHeight;
	float _CurrentCameraDistanceMax; //equal ClientCfg.CameraDistMax in Ryzom or ring(play/test)mode or equal ClientCfg.DMCameraDistMax in ring dm mode

protected:
	/// Vector for the view position.
	CVector _ViewPos;
	/// Vector for the view heading.
	CVector _View;
	/// Rear View
	bool	_RearView;
	/// Vector for the refine.
	CVector _RefinePos;
	/// The cameraDist due to collision
	float	_CollisionCameraDist;
	/// The Third person cluster system
	NL3D::UInstanceGroup	*_ThirPersonClusterSystem;
	/// Force Temporary FirstPersonView if too near a wall
	bool	_ForceFirstPersonView;

	/// const. move back camera target to avoid problem of camera target too near a wall
	float	_CameraCollisionDecal;
	// const the threshold is to avoid problem because of imprecision and camera near clip
	float	_CameraCollisionThreshold;

public:

	/// Constructor
	CView();

	void update();

	/// Get the user position.
	const CVector &viewPos() const {return _ViewPos;}
	/// Set the user position.
	void viewPos(const CVector &vect) {_ViewPos = vect;}

	/// Get the view (like a camera).
	const CVector &view() const {return _View;}
	/// Set the view (like a camera).
	void view(const CVector &vect) {_View = vect;}

	/// Change the height of the camera
	void changeCameraHeight(bool up, bool down);
	/// Change the distance of the camera
	void changeCameraDist(bool forward, bool backward);
	// Change the distance from the user to the camera.
	void cameraDistance(float dist);

	/// rotate the view on the left or right.
	void rotate(float ang);

	/// rotate the view vertically.
	void rotVertically(float ang);
	/// rotate the view horizontally.
	void rotHorizontally(float ang);

	// Return the current view position (rear or normal)
	CVector currentViewPos() const;
	// Return the current view (rear or normal)
	CVector currentView() const;
	// Return the current view as a quaternion
	NLMISC::CQuat currentViewQuat() const;

	// Return the current Camera Target (for 3rd person only. 1st person: return currentViewPos())
	CVector currentCameraTarget() const;

	// For debug only
	void	getCamera3rdPersonSetup(CVector &cameraStart, CVector &cameraEnd, CVector &cameraTestStart) const;

	//
	void rearView(bool r) {_RearView = r;}
	bool rearView() const {return _RearView;}

	/** Special for optimisation, set the "refinePos".
	 *	The refine Pos is used for Async Loading, Landscape refining, etc...
	 *	For optimisation, it should be the UserEntity pos, and not the view pos (else 3rd person=> slower)
	 */
	void refinePos(const CVector &pos) {_RefinePos= pos;}
	const CVector &refinePos() const {return _RefinePos;}

	// update the camera collision, must be called once per frame, after changing viewPos() and/or view()
	void updateCameraCollision();

	NL3D::UInstanceGroup *getThirdPersonClusterSystem() const {return _ThirPersonClusterSystem;}

	// true if the camera collision want to force the first person view
	bool	forceFirstPersonView() const {return _ForceFirstPersonView;}

	// update the max distance of the camera for player that want to play as dm (camera can be far)
	void setCameraDistanceMaxForDm();
	// update the max distance of the camera for player that want to play as player (camera must be near)
	void setCameraDistanceMaxForPlayer();

protected:
	// Move Forward or Backward
	void increaseCameraDist();
	void decreaseCameraDist();

	// get the input data for camera 3d person collision test
	void getCamera3rdPersonSetupInternal(CVector &cameraStart, CVector &cameraEnd, CVector &cameraTestStart, float &testStartDecal) const;

};


#endif // NL_VIEW_H

/* End of view.h */
