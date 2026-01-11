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

#ifndef NL_INSTANCE_LIGHTER_H
#define NL_INSTANCE_LIGHTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/triangle.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/landscape.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/cube_grid.h"
#include "nel/3d/ig_surface_light_build.h"


namespace NL3D
{


// ***************************************************************************
/**
 * A class to precompute "StaticSetup" lighting for instances in an InstanceGroup.
 *	Class "inspired" from CZoneLighter :)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceLighter
{
public:

	enum	{MaxOverSamples= 16};

public:

	// Light Options decription structure
	class CLightDesc
	{
	public:
		// Default Ctor
		CLightDesc ();

		// Disable Sun contribution: ie set 0 to all contributions of sun on instances and surfaces?? false by default
		bool					DisableSunContribution;

		// Sun direction
		NLMISC::CVector			LightDirection;

		// Grid size
		uint					GridSize;

		// Grid size
		float					GridCellSize;

		// Project shadows on instances.
		bool					Shadow;

		// Number of samples For sun shadowing only. Must be: 0 (disable), 2, 4, 8, 16
		uint					OverSampling;

		// This is a user shapeMap.
		std::map<std::string, IShape*>				UserShapeMap;

	};

	// A triangle used to light the zone
	class CTriangle
	{
		friend class CInstanceLighter;
	public:
		// Ctors
		CTriangle (const NLMISC::CTriangle& triangle, sint instanceId)
		{
			Triangle=triangle;
			InstanceId= instanceId;
		}

		// The triangle
		NLMISC::CTriangle	Triangle;

		// which instance owns this triangle. -1 if none (landscape or others ig).
		sint				InstanceId;

		// Other info
		const NLMISC::CPlane		&getPlane() const {return Plane;}

	private:
		NLMISC::CPlane		Plane;
	};


public:

	/**	Tool method which take a single IG, and do all the god job to light this one, with no other dependencies
	 *	NB: it uses instLighter passed, init() ing it. It use lightDesc.UserShapeMap or it load Shape in directory lightDesc.ShapePath
	 */
	static	void	lightIgSimple(CInstanceLighter &instLighter, const CInstanceGroup &igIn, CInstanceGroup &igOut, const CLightDesc &lightDesc, const char *igName);

public:

	/// Constructor
	CInstanceLighter();
	/// Destructor
	virtual ~CInstanceLighter() {}

	// Init the system
	void init ();

	/** Light an InstanceGroup
	 *	igOut has different PointLights than igIn. eg: if a pointLight do not light anything, then it is not
	 *	present in igOut.
	 *	NB: shapes are used to retrieve useful info on them (center of AABBox ...) . They are taken from
	 *	lightDesc.UserShapeMap, or loaded from lightDesc.ShapePath if not found.
	 *	\param landscape if !NULL use this Landscape SunContribution, looking landscape faces under each instance,
	 *	for faster computing, and to get influence of Sky. NB: this landscape does not have to be tesselated,
	 *	but all Zones that lies under igIn should be loaded in.
	 *	\param igSurfaceLightBuild if !NULL, light() will compute igOut.IGSurfaceLight, else it is just cleared.
	 */
	void light (const CInstanceGroup &igIn, CInstanceGroup &igOut, const CLightDesc &lightDesc, std::vector<CTriangle>& obstacles,
		CLandscape *landscape= NULL, CIGSurfaceLightBuild *igSurfaceLightBuild= NULL);


	// Add triangles from a landscape
	static void addTriangles (CLandscape &landscape, std::vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray);

	/** Add triangles from a transform shape. Work only for CMesh, CMultiMesh and CMeshMRM all without skinning.
	 *	Give An instanceId!=-1 only for instances of the IG to compute. This is to avoid auto-shadowing.
	 */
	static void addTriangles (const IShape &shape, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId);

	// Progress callback
	virtual void progress (const char * /* message */, float /* progress */) {}


	/// \name Static PointLights mgt.
	//@{

	/** Append a static point light to compute. call at setup stage (before light() ).
	 *	NB: you must append all PointLights of intersets, even ones from the IG to compute.
	 */
	void			addStaticPointLight(const CPointLightNamed &pln, const char *igName);

	//@}


// ********************************
private:


	static void addTriangles (const CMeshGeom &meshGeom, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId);
	static void addTriangles (const CMeshMRMGeom &meshGeom, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray, sint instanceId);
	static void excludeAllPatchFromRefineAll (CLandscape &landscape, std::vector<uint> &listZone, bool exclude);


	// light / rayTrace against sun.
	void	computeSunContribution(const CLightDesc &lightDesc, std::vector<CTriangle>& obstacles, CLandscape *landscape);

	void	dilateLightingOnSurfaceCells();

private:
	struct	CPointLightRT;

	struct	CInstanceInfo
	{
		// Center of the mesh to compute lighting.
		CVector			CenterPos;

		// Pos of samples, for overSampling. if none, OverSamples[0]== CenterPos
		CVector			OverSamples[MaxOverSamples];

		// Temp light which influence this instance.
		CPointLightRT	*Light[CInstanceGroup::NumStaticLightPerInstance];

		// Temp Ambient light which influence this instance.
		CPointLightRT	*LocalAmbientLight;
	};

	// Instance of the current ig to be lighted.
	std::vector<CInstanceGroup::CInstance>	_Instances;
	// Instance Lighting Info of the current ig to be lighted.
	std::vector<CInstanceInfo>				_InstanceInfos;
	// The current instance computed. Used to skip it in raytracing
	sint									_CurrentInstanceComputed;
	// CIGSurfaceLight Info.
	CIGSurfaceLightBuild					*_IGSurfaceLightBuild;
	// The RetrieverGridMap currently builded.
	CIGSurfaceLight::TRetrieverGridMap		_IGRetrieverGridMap;


	/// A PointLight struct to test raytracing.
	struct	CPointLightRT
	{
		CPointLightNamed		PointLight;
		float					OODeltaAttenuation;
		// BBox of the pointLight
		NLMISC::CBSphere		BSphere;

		// Faces that may occlude the light. Only Back Faces (from the light pov) are inserted
		CCubeGrid<const CTriangle*>		FaceCubeGrid;
		// Number of TileLightInfluences which use this PointLight.
		uint					RefCount;
		// Final id of the pointLight in the Ig.
		uint					DstId;

		CPointLightRT();

		/** Tells if a point is visible from this light. NB: test first if in BSphere.
		 *	If occluded or out of radius, return false, else return true.
		 *	if instanceComputed>=0, then skip obstacles with InstanceId==instanceComputed.
		 *	Also Skip if the light is an Ambient, and skip if the light is a spot and if the position is out of the cone
		 */
		bool		testRaytrace(const CVector &v, sint instanceComputed= -1);
	};


	/// For sort()
	struct		CPredPointLightToPoint
	{
		CVector		Point;

		bool	operator() (CPointLightRT *pla, CPointLightRT *plb) const;
	};


	/// List of PointLights
	std::vector<CPointLightRT>		_StaticPointLights;
	/// QuadGrid of PointLights. Builded from _StaticPointLights
	CQuadGrid<CPointLightRT*>		_StaticPointLightQuadGrid;


	/// Fill CubeGrid, and set PointLightRT in _StaticPointLightQuadGrid.
	void			compilePointLightRT(uint gridSize, float gridCellSize, std::vector<CTriangle>& obstacles, bool doShadow);

	/** Process the IG, ie process _InstanceInfos. Also process SurfaceLightGrid
	 *	MultiCPU: not done for now. Be aware of CPointLightRT::RefCount!!!!
	 */
	void			processIGPointLightRT(std::vector<CPointLightNamed> &listPointLight);


private:

	/// \name Cell Iteration. ie iteration on _IGRetrieverGridMap cells and _IGSurfaceLightBuild cells
	// @{

	void			beginCell();
	void			nextCell();
	bool			isEndCell();
	// get current cell iterated.
	CSurfaceLightGrid::CCellCorner		&getCurrentCell();
	// get current cellInfo iterated.
	CIGSurfaceLightBuild::CCellCorner	&getCurrentCellInfo();
	// Neighboring of the cell
	bool			isCurrentNeighborCellInSurface(sint xnb, sint ynb);
	CSurfaceLightGrid::CCellCorner		&getCurrentNeighborCell(sint xnb, sint ynb);
	CIGSurfaceLightBuild::CCellCorner	&getCurrentNeighborCellInfo(sint xnb, sint ynb);

	uint			getCurrentCellNumber() const {return _ItCurrentCellNumber;}
	uint			getTotalCellNumber() const	{return _TotalCellNumber;}
	void			progressCell(const char *message);

	// Iteration Data.
	CIGSurfaceLight::ItRetrieverGridMap			_ItRetriever;
	CIGSurfaceLightBuild::ItRetrieverGridMap	_ItRetrieverInfo;
	uint										_ItSurfId;
	uint										_ItCellId;
	bool										_IsEndCell;
	uint										_ItCurrentCellNumber;
	uint										_TotalCellNumber;
	float										_LastCellProgress;

	// @}

};


} // NL3D


#endif // NL_INSTANCE_LIGHTER_H

/* End of instance_lighter.h */
