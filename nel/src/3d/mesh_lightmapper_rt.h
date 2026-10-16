// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020-2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_MESH_LIGHTMAPPER_RT_H
#define NL_MESH_LIGHTMAPPER_RT_H

// Private header of the CMeshLightmapper port (source: nel_mesh_lib calc_lm_rt.h).
// Faithful port — the Max scene walk (getAllSelectedNode/getAllNodeInScene/addNode) is
// replaced by iteration over the CLightmapScene occluder list; everything else verbatim.

#include "nel/3d/mesh.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/lightmap_scene.h"

#include "nel/misc/vector.h"
#include "nel/misc/triangle.h"
#include "nel/misc/plane.h"
#include "nel/misc/uv.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace NL3D
{

// The port keeps the original code verbatim: SLightBuild is the scene-graph light record.
typedef CLightmapLight SLightBuild;

// ***********************************************************************************************
// Light representation for the raytrace
class CRTLight
{

public:

	enum EType { RTLightAmbient, RTLightPoint, RTLightDir, RTLightSpot };
	EType Type;
	NLMISC::CVector Position;
	NLMISC::CVector Direction;

	float rSoftShadowRadius;
	float rSoftShadowConeLength;
};

// ***********************************************************************************************
/** A ray is a cone/cylinder coming from the vertex to evaluate against a directional light
 *	The cone start at vertex, and end at a certain distance, then start the cylinder.
 *	This is to allow fast grid lookup acceleration (a "cone only" shape may produce a to big radius for grid lookup with large scene)
 */
class CRTRay
{

public:

	void initDirectionnal	(uint32 nNbSide, const NLMISC::CVector &vVertex, const NLMISC::CVector &lightDir, float rRadius, float rDistCyl);

	void clip	(const NLMISC::CTriangle& t); // Warning : t is transformed with the _InvVertexMat matrix

	float getArea ();

private:

	NLMISC::CMatrix			_InvVertexMat;
	float					_DistCyl;
	// Cone and Cylinder Clipping pyramids
	enum	{NumConePlanes= 2, NumCylinderPlanes= 1};
	NLMISC::CPlane			_ConePyramid[NumConePlanes];
	NLMISC::CPlane			_CylinderPyramid[NumCylinderPlanes];

public:

	// Representation of a convex shape
	struct SConvexShape
	{
		std::vector<NLMISC::CUV> Vertices;
	};

	std::vector<SConvexShape> Shapes;

private:

	void	clipProjected(const NLMISC::CVector &v0, const NLMISC::CVector &v1, const NLMISC::CVector &v2);

	bool isShapeMustBeClippedByTriangle (SConvexShape&scs, NLMISC::CUV tri[3]);
	void clipShape (SConvexShape& ShapeIn, NLMISC::CUV Tri[3], std::vector<SConvexShape> &ShapesOut);
	NLMISC::CUV getLineIntersection (const NLMISC::CUV &l1p1, const NLMISC::CUV &l1p2,
									const NLMISC::CUV &l2p1, const NLMISC::CUV &l2p2);

	void weldVertices (SConvexShape&scs);
};
// ***********************************************************************************************
// An element of the accelerators
struct SGridCell
{
	NL3D::CMesh::CFace* pF;
	NL3D::CMesh::CMeshBuild* pMB;
	NL3D::CMeshBase::CMeshBaseBuild* pMBB;
};

// ***********************************************************************************************
// Light accelerator interface to speed up raytrace
class IRTLightAccel
{
public:
	virtual ~IRTLightAccel() { }

	// Creation
	virtual void		insert (NLMISC::CTriangle &tri, SGridCell &cell) = 0;

	// Selection
	virtual void		select (NLMISC::CVector &v) = 0;
	virtual void		select (NLMISC::CVector &v, float rRadius) = 0;
	virtual SGridCell	getSel() = 0;
	virtual void		nextSel() = 0;
	virtual bool		isEndSel() = 0;
};

// ***********************************************************************************************
class CRTWorld
{
	// The world defined by basics meshes (world-space clones, owned) and their node names
	std::vector<NL3D::CMesh::CMeshBuild*>			vMB;
	std::vector<NL3D::CMeshBase::CMeshBaseBuild*>	vMBB;
	std::vector<std::string>						vNodeName;

	// Lights in the world
	std::vector<IRTLightAccel*>				vLightAccel;
	std::vector<CRTLight>					vLight;

	NLMISC::CVector	GlobalTrans;

public:

	CRTWorld ();
	~CRTWorld();

	/** Build the raytrace world from the scene-graph occluder list (replaces the original's
	 *	Max scene walk). Every occluder not excluded by name is cloned to world space; the
	 *	receiver geometry itself is added through the same filters (its own pristine build).
	 */
	void build	(std::vector<SLightBuild> &AllLights, const NLMISC::CVector &trans,
				const CLightmapScene &scene, const std::set<std::string> &excludeNames,
				const std::string &includeName,
				const CMesh::CMeshBuild &includeMB, const CMeshBase::CMeshBaseBuild &includeMBB);

	// Raytrace the vertex vVertex from light nLightNb
	NLMISC::CRGBAF raytrace (NLMISC::CVector &vVertex, sint32 nLightNb, uint8& rtVal, bool bSoftShadow);

private:

	// All we need for the raytrace

	void testCell	(NLMISC::CRGBAF &retValue, SGridCell &cell, NLMISC::CVector &vLightPos,
					NLMISC::CVector &vVertexPos, uint8& rtVal);

	// All we need for the build

	/// Clone one scene mesh into the world if it casts shadows and interacts with a light
	void addMesh (const std::string &name, const CMesh::CMeshBuild &mb,
				const CMeshBase::CMeshBaseBuild &mbb, std::vector<SLightBuild> &AllLights);

	bool intersectionTriangleSphere (NLMISC::CTriangle &t, NLMISC::CBSphere &s);

	bool intersectionSphereCylinder (NLMISC::CBSphere &s, NLMISC::CVector &cyCenter,
									NLMISC::CVector &cyDir, float cyRadius);

	bool isInteractionWithLight (SLightBuild &rSLB, NLMISC::CAABBox &meshBox);

	bool isInteractionLightMesh (SLightBuild &rSLB, NL3D::CMesh::CMeshBuild &rMB, NL3D::CMeshBase::CMeshBaseBuild &rMBB);
};


// ***********************************************************************************************
// Light accelerator for directionnal lights
class CRTLightAccelDir : public IRTLightAccel
{

public:

	float rMin, rMax;	// distance min and max from the light position to clip all rays
						// the distance is given in the direction of the light direction

public:

	// Creation
	CRTLightAccelDir();
	virtual ~CRTLightAccelDir();

	void		create (int nSize, float rRadius, NLMISC::CVector &vDirection);
	void		insert (NLMISC::CTriangle &tri, SGridCell &cell);

	// Selection
	void		select (NLMISC::CVector &v);
	void		select (NLMISC::CVector &v, float rRadius);
	SGridCell	getSel ();
	void		nextSel ();
	bool		isEndSel ();

private:

	NL3D::CQuadGrid<SGridCell> grid;
	NLMISC::CMatrix invMat;
	NL3D::CQuadGrid<SGridCell>::CIterator itSel;

};

// ***********************************************************************************************
// Light accelerator for point and spot lights
class CRTLightAccelPoint : public IRTLightAccel
{

public :

	// Creation
	CRTLightAccelPoint ();
	virtual ~CRTLightAccelPoint ();

	void		create (int nSize);
	void		insert (NLMISC::CTriangle &tri, SGridCell &cell);

	// Selection
	void		select (NLMISC::CVector &v);
	void		select (NLMISC::CVector &v, float rRadius);
	SGridCell	getSel ();
	void		nextSel ();
	bool		isEndSel ();

private:

	enum gridPos { kUp = 0, kDown, kLeft, kRight, kFront, kBack };
	NL3D::CQuadGrid<SGridCell> _Grids[6];

	struct SSelector
	{
		sint32 nSelGrid;
		NL3D::CQuadGrid<SGridCell>::CIterator itSel;
	};

	uint32					_CurSel;
	std::vector<SSelector>	_Selection;

private:

	void project	(NLMISC::CTriangle &tri, NLMISC::CPlane pyr[4], NLMISC::CPlane &gridPlane,
					sint32 nGridNb, SGridCell &cell);

};

} // NL3D

#endif // NL_MESH_LIGHTMAPPER_RT_H

/* End of mesh_lightmapper_rt.h */
