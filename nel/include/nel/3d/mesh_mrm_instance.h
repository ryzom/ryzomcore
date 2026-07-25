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
	virtual ~CMeshMRMInstance() NL_OVERRIDE;


	/// \name Skinning Behavior.
	// @{
	/// I can be skinned if the mesh is.
	virtual	bool	isSkinnable() const NL_OVERRIDE;

	/// Called when the skin is applied on the skeleton
	virtual	void	setApplySkin(bool state) NL_OVERRIDE;

	/// Called for lod character coloring.
	virtual const std::vector<sint32>			*getSkinBoneUsage() const NL_OVERRIDE;

	/// Called for more precise clipping.
	virtual const std::vector<NLMISC::CBSphere>	*getSkinBoneSphere() const NL_OVERRIDE;

	/// Implementation of the renderSkin
	virtual void	renderSkin(float alphaMRM) NL_OVERRIDE;

	// Implementation of SkinGrouping
	virtual	bool			supportSkinGrouping() const NL_OVERRIDE;
	virtual	sint			renderSkinGroupGeom(float alphaMRM, uint remainingVertices, uint8 *dest) NL_OVERRIDE;
	virtual	void			renderSkinGroupPrimitives(uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex) NL_OVERRIDE;
	virtual	void			renderSkinGroupSpecularRdrPass(uint rdrPassId) NL_OVERRIDE;

	virtual	bool			supportShadowSkinGrouping() const NL_OVERRIDE;
	virtual	sint			renderShadowSkinGeom(uint remainingVertices, uint8 *vbDest) NL_OVERRIDE;
	virtual	void			renderShadowSkinPrimitives(CMaterial &castMat, IDriver *drv, uint baseVertex) NL_OVERRIDE;

	virtual	bool			supportGPUSkinning() const NL_OVERRIDE;
	virtual	void			renderGPUSkin(float alphaMRM, CSkeletonModel *skeleton) NL_OVERRIDE;
	virtual	CVertexProgram	*getGPUSkinVP() const NL_OVERRIDE;

	virtual	bool			supportIntersectSkin() const NL_OVERRIDE;
	virtual	bool			intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D) NL_OVERRIDE;

	/// Called for edition purpose (slow call O(NVertex))
	virtual bool			getSkinBoneBBox(NLMISC::CAABBox &bbox, uint boneId) NL_OVERRIDE;

	// @}


	/// \name Load balancing methods
	// @{

	/** Change MRM Distance setup. See CMeshBaseInstance::changeMRMDistanceSetup()
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest) NL_OVERRIDE;

	virtual	const	CMRMLevelDetail		*getMRMLevelDetail() const NL_OVERRIDE;

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
