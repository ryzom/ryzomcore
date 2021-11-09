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

#ifndef NL_MESH_INSTANCE_H
#define NL_MESH_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/material.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"


namespace NL3D
{


class CMesh;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshInstanceId=NLMISC::CClassId(0x6bfe0a34, 0x23b26dc9);


// ***************************************************************************
/**
 * An instance of CMesh.
 * no special traverse, since same functionnality as CMeshBaseInstance.
 *	NB: CMeshInstance support shadowMap casting ONLY for an easy Demo / Test. No Lod mgt, and No good shadow clip.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshInstance : public CMeshBaseInstance
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

public:

protected:
	/// Constructor
	CMeshInstance();

	/// Destructor
	virtual ~CMeshInstance();

	/// \name Skinning Behavior.
	// @{
	/// I can be skinned if the mesh is.
	virtual	bool	isSkinnable() const;

	/// Call when the skin is applied on the skeleton
	virtual	void	setApplySkin(bool state);

	/// Called for lod character coloring.
	virtual const std::vector<sint32>	*getSkinBoneUsage() const;

	/// Called for more precise clipping.
	virtual const std::vector<NLMISC::CBSphere>	*getSkinBoneSphere() const;

	/// Implementation of the renderSkin
	virtual void	renderSkin(float alphaMRM);

	/// Skin intersection?
	virtual	bool			supportIntersectSkin() const;
	virtual	bool			intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D);

	// @}

	/// \name Special Traverse Feature.
	// @{
	// @}

	// called at instanciation
	void			initRenderFilterType();

	/// \name ShadowMap Behavior.
	// @{
	virtual	void		generateShadowMap(const CVector &lightDir);
	virtual	CShadowMap	*getShadowMap();
	virtual bool		computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB);
	virtual void		renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat);
	// @}

private:
	static CTransform	*creator() {return new CMeshInstance;}
	friend	class CMesh;

	// MeshInstances can generate Shadow Map
	CShadowMap			*_ShadowMap;

	void			updateShadowMap(IDriver *driver);
protected:
	virtual void			createShadowMap();
	virtual void			deleteShadowMap();
};




} // NL3D


#endif // NL_MESH_INSTANCE_H

/* End of mesh_instance.h */
