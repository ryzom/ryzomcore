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

#ifndef NL_MESH_MRM_INSTANCE_H
#define NL_MESH_MRM_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/material.h"
#include "nel/3d/animated_material.h"


namespace NL3D
{


class CMeshMRM;
class CMeshMRMGeom;
class CRawSkinNormalCache;
class CShiftedTriangleCache;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshMRMInstanceId=NLMISC::CClassId(0xec608f3, 0x1111c33);


// ***************************************************************************
/**
 * An instance of CMeshMRM.
 * no special traverse, since same functionnality as CMeshBaseInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMRMInstance : public CMeshBaseInstance
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

protected:
	/// Constructor
	CMeshMRMInstance()
	{
		_RawSkinCache= NULL;
		_ShiftedTriangleCache= NULL;
	}
	/// Destructor
	virtual ~CMeshMRMInstance();


	/// \name Skinning Behavior.
	// @{
	/// I can be skinned if the mesh is.
	virtual	bool	isSkinnable() const;

	/// Called when the skin is applied on the skeleton
	virtual	void	setApplySkin(bool state);

	/// Called for lod character coloring.
	virtual const std::vector<sint32>			*getSkinBoneUsage() const;

	/// Called for more precise clipping.
	virtual const std::vector<NLMISC::CBSphere>	*getSkinBoneSphere() const;

	/// Implementation of the renderSkin
	virtual void	renderSkin(float alphaMRM);

	// Implementation of SkinGrouping
	virtual	bool			supportSkinGrouping() const;
	virtual	sint			renderSkinGroupGeom(float alphaMRM, uint remainingVertices, uint8 *dest);
	virtual	void			renderSkinGroupPrimitives(uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex);
	virtual	void			renderSkinGroupSpecularRdrPass(uint rdrPassId);

	virtual	bool			supportShadowSkinGrouping() const;
	virtual	sint			renderShadowSkinGeom(uint remainingVertices, uint8 *vbDest);
	virtual	void			renderShadowSkinPrimitives(CMaterial &castMat, IDriver *drv, uint baseVertex);

	virtual	bool			supportIntersectSkin() const;
	virtual	bool			intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D);

	/// Called for edition purpose (slow call O(NVertex))
	virtual bool			getSkinBoneBBox(NLMISC::CAABBox &bbox, uint boneId);

	// @}


	/// \name Load balancing methods
	// @{

	/** Change MRM Distance setup. See CMeshBaseInstance::changeMRMDistanceSetup()
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	virtual	const	CMRMLevelDetail		*getMRMLevelDetail() const;

	// @}

	// called at instanciation
	void			initRenderFilterType();

// *************************
private:
	static CTransform	*creator() {return new CMeshMRMInstance;}
	friend	class CMeshMRM;
	friend	class CMeshMRMGeom;

	/// Used by CMeshMRMGeom. This a cache for skinning objects, for skinning optimisation
	CRawSkinNormalCache		*_RawSkinCache;
	/// Reset the RawSkin Info.
	void					clearRawSkinCache();

	/// Used by CMeshMRMGeom. This a cache for skinning objects, for skinning optimisation
	CShiftedTriangleCache	*_ShiftedTriangleCache;
	/// Reset the _ShiftedTriangleCache Info.
	void					clearShiftedTriangleCache();

};



} // NL3D


#endif // NL_MESH_MRM_INSTANCE_H

/* End of mesh_mrm_instance.h */
