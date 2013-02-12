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

#ifndef NL_LOAD_BALANCING_TRAV_H
#define NL_LOAD_BALANCING_TRAV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/3d/trav_scene.h"
#include "nel/misc/value_smoother.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class CClipTrav;
class CLoadBalancingTrav;
class CTransform;


// ***************************************************************************
/**
 * A LoadBalancing Group. Models are owned by a group.
 *	Groups are created in CLoadBalancingTrav.
 *
 * \sa CScene CLoadBalancingTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLoadBalancingGroup
{
public:
	// see CScene.
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing };

public:
	// Name of the group.
	std::string		Name;

public:
	CLoadBalancingGroup();

	void				setNbFaceWanted(uint nFaces) {_NbFaceWanted= nFaces;}
	uint				getNbFaceWanted() const {return _NbFaceWanted;}

	float				getNbFaceAsked () const {return _NbFacePass0;}

public:
	// ONLY FOR MODEL TRAVERSING.
	void				addNbFacesPass0(float v) {_NbFacePass0+= v;}

	/// Compute the number of face to be rendered for thismodel, according to the number of faces he want to draw
	float				computeModelNbFace(float faceIn)	{return faceIn * _FaceRatio;}

private:
	friend class	CLoadBalancingTrav;

	// DefaultGroup means LoadBalancing disabled.
	bool				_DefaultGroup;

	// The number of faces count in Pass0.
	float				_NbFacePass0;

	// The number of face the user want
	uint				_NbFaceWanted;

	// use this ratio into Pass 1 to reduce faces.
	float				_FaceRatio;

	// To smooth the faceRatio
	NLMISC::CValueSmoother		_ValueSmoother;
	// Balancing Mode
	TPolygonBalancingMode		_PrecPolygonBalancingMode;

	// as it sounds..
	void				computeRatioAndSmooth(TPolygonBalancingMode polMode);
};



// ***************************************************************************
/**
 * The LoadBalancing traversal. It needs a camera setup (see CTravCameraScene).
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLoadBalancingTrav : public CTravCameraScene
{
public:

	/// Constructor
	CLoadBalancingTrav();


	/// \name ITrav/ITravScene Implementation.
	//@{
	void				traverse();
	//@}


	/// \name LoadBalancing mgt.
	//@{

	// The PolygonBalancingMode
	typedef CLoadBalancingGroup::TPolygonBalancingMode	TPolygonBalancingMode;
	TPolygonBalancingMode	PolygonBalancingMode;


	/** Set the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	void				setGroupNbFaceWanted(const std::string &group, uint nFaces);

	/** Get the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	uint				getGroupNbFaceWanted(const std::string &group);


	/** Get the last face count asked from the instances before reduction. only for the given group
	 *	return 0 if the Group does not exist.
	 */
	float				getGroupNbFaceAsked (const std::string &group) const;

	/// Get the last face count asked from the instances before reduction. Sum of all groups
	float				getNbFaceAsked () const;

	//@}


public:
	// ONLY FOR MODEL TRAVERSING.
	uint				getLoadPass() {return _LoadPass;}

	CLoadBalancingGroup	*getDefaultGroup() {return _DefaultGroup;}

	// Get a group by name, create if needed.
	CLoadBalancingGroup	*getOrCreateGroup(const std::string &group);


	// For clipTrav. cleared at beginning of CClipTrav::traverse
	void				clearVisibleList();

	// For ClipTrav only. NB: list is cleared at beginning of traverse().
	void				addVisibleModel(CTransform *model)
	{
		_VisibleList[_CurrentNumVisibleModels]= model;
		_CurrentNumVisibleModels++;
	}

	// for createModel().
	void				reserveVisibleList(uint numModels);

// **************
private:
	// Pass: 0 (compute faceCount from all models) or 1 (setup wanted faceCount).
	uint				_LoadPass;

	// The sum of all Pass0 groups.
	float				_SumNbFacePass0;

	// The loadBalancing balance only visible objects.
	void				traverseVisibilityList();


	// The groups.
	CLoadBalancingGroup	*_DefaultGroup;
	typedef	std::map<std::string, CLoadBalancingGroup>	TGroupMap;
	typedef	TGroupMap::iterator							ItGroupMap;
	TGroupMap			_GroupMap;

	// traverse list of model visible and useful to loadBalance.
	std::vector<CTransform*>	_VisibleList;
	uint32						_CurrentNumVisibleModels;

};


} // NL3D


#endif // NL_LOAD_BALANCING_TRAV_H

/* End of load_balancing_trav.h */
