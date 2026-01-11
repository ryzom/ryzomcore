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

#ifndef NL_VISUAL_COLLISION_MESH_H
#define NL_VISUAL_COLLISION_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"


namespace NL3D
{


class IDriver;
class CShadowMap;
class CShadowMapProjector;
class CMaterial;


// ***************************************************************************
/**
 * Collision mesh used for camera collision for instance
 *	Additionally used for ShadowMap receiving
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CVisualCollisionMesh : public NLMISC::CRefCount
{
public:
	// for receiveShadowMap
	class CShadowContext
	{
	public:
		IDriver				*Driver;
		CShadowMap			*ShadowMap;
		NLMISC::CVector		CasterPos;
		NLMISC::CAABBox		ShadowWorldBB;
		CShadowMapProjector &ShadowMapProjector;
		CMaterial			&ShadowMaterial;
		CIndexBuffer		&IndexBuffer;

	public:
		CShadowContext(CMaterial &mat, CIndexBuffer &ib, CShadowMapProjector &smp) :
		  ShadowMapProjector(smp), ShadowMaterial(mat), IndexBuffer(ib)
		{
			Driver= NULL;
			ShadowMap= NULL;
		}
	};

public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/


	/// Constructor
	CVisualCollisionMesh();

	/** build. NB: fails if too much vertices/triangles (>=65536) or if 0 vertices / triangles
	 *	\param vbForShadowRender: a RefPtr is kept on this VB for shadow rendering
	 */
	bool		build(const std::vector<NLMISC::CVector> &vertices, const std::vector<uint32> &triangles, CVertexBuffer &vbForShadowRender);

	/// get collision with camera. [0,1] value
	float		getCameraCollision(const NLMISC::CMatrix &instanceMatrix, class CCameraCol &camCol);

	/// compute the world bbox of an instance
	NLMISC::CAABBox	computeWorldBBox(const NLMISC::CMatrix &instanceMatrix);

	/// receive a shadowMap. render in driver the triangles that intersect the shadow
	void		receiveShadowMap(const NLMISC::CMatrix &instanceMatrix, const CShadowContext &shadowContext);

	// get vertices of the mesh
	const std::vector<NLMISC::CVector> &getVertices() const { return _Vertices; }

	// get triangles of the mesh
	const std::vector<uint16> &getTriangles() const { return _Triangles; }


// *********************
private:

	// A Static Grid Container. Only 65535 elements max can be inserted
	class	CStaticGrid
	{
		/* ***********************************************
		 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
		 *	It can be loaded/called through CAsyncFileManager for instance
		 * ***********************************************/

	public:
		// create
		void	create(uint nbQuads, uint nbElts, const NLMISC::CAABBox &gridBBox);

		// add an element (bbox shoudl be included in gridBBox from create() )
		void	add(uint16 id, const NLMISC::CAABBox &bbox);

		// compile
		void	compile();

		// return the list of elements intersected. NB: the vector is enlarged to max, but real selection size is in the  return value
		uint	select(const NLMISC::CAABBox &bbox, std::vector<uint16> &res);

	private:
		struct CEltBuild
		{
			uint32	X0,Y0;
			uint32	X1,Y1;
		};
		// point to GridData
		struct CCase
		{
			uint32	Start, NumElts;
		};
		uint32										_GridSizePower;
		uint32										_GridSize;
		NLMISC::CVector								_GridPos;
		NLMISC::CVector								_GridScale;
		// The Grid
		NLMISC::CObjectVector<CCase, false>			_Grid;
		// The raw elt data
		NLMISC::CObjectVector<uint16, false>		_GridData;
		// Used at build only
		NLMISC::CObjectVector<CEltBuild, false>		_EltBuild;
		uint32										_GridDataSize;

		// For Fast selection
		uint32										_ItSession;
		// For each element the session id. if same than _ItSession, then already inserted
		NLMISC::CObjectVector<uint32, false>		_Sessions;
	};

private:

	// Data
	std::vector<NLMISC::CVector>	_Vertices;
	std::vector<uint16>				_Triangles;
	// The Local Triangle Quadgrid
	CStaticGrid						_QuadGrid;

	// For ShadowMap receiving. Point to the original Mesh VB (should be in AGP)
	NLMISC::CRefPtr<CVertexBuffer>	_VertexBuffer;
};


} // NL3D


#endif // NL_VISUAL_COLLISION_MESH_H

/* End of visual_collision_mesh.h */
