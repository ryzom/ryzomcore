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

#ifndef NL_MESH_MULTI_LOD_INSTANCE_H
#define NL_MESH_MULTI_LOD_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"

#include "nel/3d/mesh_base_instance.h"


namespace NL3D
{

class	CMeshGeom;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshMultiLodInstanceId=NLMISC::CClassId(0x1ade6ef8, 0x75c5a84);


// ***************************************************************************
/**
 * An instance of CMeshMulitLod
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMultiLodInstance : public CMeshBaseInstance
{
public:

	/// Ctor
	CMeshMultiLodInstance ();

	/// Dstructor
	~CMeshMultiLodInstance ();

	/** Change MRM Distance setup. See CMeshBaseInstance::changeMRMDistanceSetup()
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


	enum
	{
		Lod0Blend		=	0x1,
	};

	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();

	/// Last Matrix date for Lods
	uint64			_LastLodMatrixDate;

	/// Last Lighting date for Lods
	sint64			_LastLodLightingDate;


	// return the contribution of lights (for Coarse Mesh render).
	const CLightContribution	&getLightContribution() {return _LightContribution;}

	// Override the shape coarse mesh distance (-1 if not overriden)
	void			 setCoarseMeshDist(float dist) { _CoarseMeshDistance = dist; }
	float            getCoarseMeshDist() const { return _CoarseMeshDistance; }

	// From CTransformShape
	virtual float				getNumTriangles (float distance);

	// called at instanciation
	void			initRenderFilterType();

	/// \name CTransform traverse specialisation
	// @{
	/** Additionally to std loadBalancing, it compues Lod related states
	 */
	virtual void	traverseLoadBalancing();
	// @}

private:

	/// Computed first lod to display for this distance
	uint	Lod0;
	uint	Lod1;

	/// Active blending on lod 0
	uint	Flags;

	/// Coarse mesh transformed and Lighted. NB: array allocated at instanciation. => no allocation during time.
	std::vector<uint8>		_CoarseMeshVB;
	CMeshGeom				*_LastCoarseMesh;
	uint					_LastCoarseMeshNumVertices;

	/// Computed polygon count for the load balancing result
	float	PolygonCountLod0;
	float	PolygonCountLod1;

	/// Alpha blending to use
	float	BlendFactor;

	/// Corse mesh distance (-1 is the one for the mesh is used)
	float   _CoarseMeshDistance;

	static CTransform	*creator() {return new CMeshMultiLodInstance;}
	friend	class CMeshMultiLod;

	/// get average color for Sun lighting. Get result from _LightContribution
	CRGBA			getCoarseMeshLighting();

	// Methods to fill The coarse VBuffer
	void			setUVCoarseMesh( CMeshGeom &geom, uint vtDstSize, uint dstUvOff );
	void			setPosCoarseMesh( CMeshGeom &geom, const CMatrix &matrix, uint vtDstSize );
	void			setColorCoarseMesh( CRGBA color, uint vtDstSize, uint dstColorOff );
};


} // NL3D


#endif // NL_MESH_MULTI_LOD_INSTANCE_H

/* End of mesh_multi_lod_instance.h */
