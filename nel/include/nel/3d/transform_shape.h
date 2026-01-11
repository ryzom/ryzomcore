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

#ifndef NL_TRANSFORM_SHAPE_H
#define NL_TRANSFORM_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/3d/transform.h"
#include "nel/3d/shape.h"
#include "nel/3d/load_balancing_trav.h"
#include <vector>
#include "nel/3d/fast_ptr_list.h"


namespace NL3D
{


using NLMISC::CSmartPtr;
using NLMISC::CPlane;


class	CRenderTrav;
class	CMRMLevelDetail;
class	CMaterial;
class	CQuadGridClipCluster;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformShapeId=NLMISC::CClassId(0x1e6115e6, 0x63502517);


// ***************************************************************************
/**
 * A transform which "is an instance of"/"point to" a IShape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransformShape : public CTransform
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

public:
	/// The shape, the object instancied.
	CSmartPtr<IShape>		Shape;


	/** Get the untransformed AABBox of the mesh. NULL (gtSize()==0) if no mesh.
	 */
	virtual void		getAABBox(NLMISC::CAABBox &bbox) const;

	/** Get the count of material in this transform shape
	 */
	virtual uint		getNumMaterial () const;

	/** Get a material of the transform shape
	 */
	virtual const CMaterial	*getMaterial (uint materialId) const;

	/** Get a material of the transform shape
	 */
	virtual CMaterial	*getMaterial (uint materialId);

	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance want render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't
	  * have to be continus.
	  */
	virtual float		getNumTriangles (float distance);

	/** get an approximation of the number of triangles this instance should render.
	 * This method is valid only for IShape classes (in render()), after LoadBalancing traversal is performed.
	 * NB: It is not guaranted that this instance will render those number of triangles.
	 */
	float				getNumTrianglesAfterLoadBalancing() {return _NumTrianglesAfterLoadBalancing;}

	/// If the model support MRM, return the level detail setup. default is return NULL.
	virtual	const	CMRMLevelDetail		*getMRMLevelDetail() const {return NULL;}

	// @}


	/// \name Mesh Block Render Tools
	// @{
	/// setup lighting for this instance into driver. The traverseRender().
	void				changeLightSetup(CRenderTrav *rdrTrav);
	// @}

	/// Test if there is a start/stop caps in the objects (some fxs such as remanence)
	virtual bool		canStartStop() { return false; }
	// For instance that have a start/stop caps
	virtual void		start() {}
	// For instance that have a start/stop caps
	virtual void		stop()  {}
	// For instance that have a start/stop caps
	virtual bool		isStarted() const { return false; }

	// Get the model distmax. At startup it is setupped with the shape value
	float               getDistMax() const { return _DistMax; }
	// Set the model distmax.
	void                setDistMax(float distMax) { _DistMax = distMax; }


	/// true if the model is linked to a quadCluster
	bool				isLinkToQuadCluster() const {return _QuadClusterListNode.isLinked();}


	/// \name CTransform traverse specialisation
	// @{
	virtual	bool	clip();
	virtual void	traverseLoadBalancing();
	virtual void	traverseRender();
	virtual	void	profileRender();
	// @}

	// Lighting: get the center of the AABBox of the model by default
	virtual	void		getLightHotSpotInWorld(CVector &modelPos, float &modelRadius) const;
	// return the contribution of lights (for traverseRender()).
	CLightContribution	&getLightContribution() { return _LightContribution;}


protected:
	/// Constructor
	CTransformShape();
	/// Destructor
	virtual ~CTransformShape() {}

	/** For deriver who wants to setup their own current lightContribution setup (as skeleton).
	 *	Must call changeLightSetup() so change are effectively made in driver
	 */
	void			setupCurrentLightContribution(CLightContribution *lightContrib, bool useLocalAtt);

	/// special feature for CQuadGridClipManager. remove from it.
	virtual	void	unlinkFromQuadCluster();

private:
	static CTransform	*creator() {return new CTransformShape;}

	float			_NumTrianglesAfterLoadBalancing;


private:
	/* The Activated lightContribution, and localAttenuation setup for this instance.
		This may be our lightContribution, or our ancestore skeleton contribution.
	*/
	CLightContribution		*_CurrentLightContribution;
	// true If this instance use localAttenuation.
	bool					_CurrentUseLocalAttenuation;
private:
	friend	class	CQuadGridClipCluster;
	friend	class	CQuadGridClipClusterListDist;

	// The max dist for this model.
	float                   _DistMax;


	/// \name Clip Traversal
	// @{
	// Link to QuadGridCluster
	CFastPtrListNode		_QuadClusterListNode;
	// @}

	/// \name LoadBalancing Traversal
	// @{
	// The number of face computed in Pass0.
	float					_FaceCount;

	void		traverseLoadBalancingPass0();
	void		traverseLoadBalancingPass1();
	// @}

};



} // NL3D


#endif // NL_TRANSFORM_SHAPE_H

/* End of transform_shape.h */
