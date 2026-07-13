/** \file zone_lighter.h
 * Class to light zones
 *
 * $Id$
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_ZONE_LIGHTER_2001_H
#define NL_ZONE_LIGHTER_2001_H

#include "nel/misc/triangle.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/pool_memory.h"
#include "nel/misc/mutex.h"

#include "nel/3d/zone.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/patchuv_locator.h"

#include <list>

namespace NL3D 
{

class CZone;
class CPatchUVLocator;
class IShape;
class CCalcRunnable;
class CMeshGeom;
class CMeshMRMGeom;

// The zone lighter
class CZoneLighter2001
{
	friend class NL3D::CCalcRunnable;
public:
	CZoneLighter2001 ();

	// Light decription structure
	class CLightDesc
	{
	public:
		enum TOverSampling
		{
			NoOverSampling=-1,
			OverSamplingx2=0,
			OverSamplingx8=1,
			OverSamplingx32=2,
			OverSamplingx128=3,
		};

		// Default Ctor
		CLightDesc ();

		// Sun direction
		NLMISC::CVector			LightDirection;

		// Oversampling
		TOverSampling			Oversampling;

		// Grid size
		uint					GridSize;

		// Grid size
		float					GridCellSize;

		// Height field size
		float					HeightfieldSize;

		// Height field cell size
		float					HeightfieldCellSize;

		// Use sun contribution
		bool					Shadow;

		// Shadow bias. Bias use to displace the shadowed position along the normal in meter
		float					ShadowBias;

		// Softshadow
		bool					Softshadow;

		// Softshadow blur size. 0 hard shadow (fast), else size of the blur in meter
		float					SoftshadowBlurSize;

		// Size of the fallof effect of soft shadow in meter
		float					SoftshadowFallof;

		// Number of vertex in the softshadow shape
		uint					SoftshadowShapeVertexCount;

		// Use sun contribution
		bool					SunContribution;

		// Use sky contribution
		bool					SkyContribution;

		// Sky intensity [0, 1]
		float					SkyIntensity;

		// Nombrer of CPU used
		uint					NumCPU;
	};

	// A triangle used to light the zone
	class CTriangle
	{
		friend class CZoneLighter2001;
	public:
		// Ctors
		CTriangle (const NLMISC::CTriangle& triangle, uint zoneId, uint patchId, float startS, float endS, float startT, float endT)
		{
			Triangle=triangle;
			ZoneId=zoneId;
			PatchId=patchId;
			StartS=startS;
			EndS=endS;
			StartT=startT;
			EndT=endT;
		}

		CTriangle (const NLMISC::CTriangle& triangle)
		{
			Triangle=triangle;
			ZoneId=0xffffffff;
			PatchId=0xffffffff;
			StartS=0;
			EndS=0;
			StartT=0;
			EndT=0;
		}

		// The triangle
		NLMISC::CTriangle	Triangle;

		// The triangle patch info
		uint				ZoneId;
		uint				PatchId;
		float				StartS;
		float				EndS;
		float				StartT;
		float				EndT;
	private:
		NLMISC::CPlane		Plane;

		// The clipping planes
		CPlane				ClippingPlanes[3];
	};

	// A triangle list
	class CTriangleList
	{
	public:
		NLMISC::CTriangle				Triangle;
		CTriangleList					*Next;
	};

#define SHAPE_VERTICES_MAX 100
#define SHAPE_MAX 500
#define MAX_CPU_PROCESS 10

	// A shape class
	class CShape
	{
	public:
		void operator= (const CShape &shape)
		{
			NumVertex=shape.NumVertex;
			memcpy (Vertices, shape.Vertices, NumVertex*sizeof(CVector));
		}
		void scale (const CVector& center, float factor)
		{
			for (uint i=0; i<NumVertex; i++)
			{
				Vertices[i]-=center;
				Vertices[i]*=factor;
				Vertices[i]+=center;
			}
		}
		uint				NumVertex;
		CVector				Vertices[SHAPE_VERTICES_MAX];
	};

	// A shape class
	class CMultiShape
	{
	public:
		std::vector<CShape>	Shapes;
	};

	// A lumel
	class CLumelDescriptor
	{
	public:
		CLumelDescriptor ()
		{
			TriangleList=NULL;
		}
		CTriangleList					*TriangleList;
		NLMISC::CVector					Position;
		NLMISC::CVector					Normal;
		float							S;
		float							T;
	};

	// A hierachical heightfield
	class CHeightField
	{
	public:
		enum TDirection
		{
			North,
			NorthEast,
			East,
			SouthEast,
			South,
			SouthWest,
			West,
			NorthWest
		};

		// Build the heightfield
		void build (std::vector<float>& heightField, const NLMISC::CVector &origine, float cellSeize, uint width, uint height);

		// Get max height on a direction
		float getMaxHeight (const NLMISC::CVector &position, TDirection direction) const;

	private:
		std::vector<std::vector<float> >	HeightFields;
	};

	// Init the system
	void init ();

	// Light a zone
	void light (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description, 
		std::vector<CTriangle>& obstacles, std::vector<uint> &listZone);

	// Add triangles from a landscape
	void addTriangles (CLandscape &landscape, std::vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray);

	// Add triangles from a transform shape. Work only for CMesh, CMultiMesh and CMeshMRM all without skinning.
	void addTriangles (const IShape &shape, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	// Progress callback
	virtual void progress (const char *message, float progress) {};

private:
	// Add triangles from a non skinned CMeshGeom.
	void addTriangles (const CMeshGeom &meshGeom, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	// Add triangles from a non skinned CMeshMRMGeom.
	void addTriangles (const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	// One process method
	void processCalc (uint process, uint firstPatch, uint lastPatch, const CLightDesc& description);

	// Build internal zone information
	void buildZoneInformation (CLandscape &landscape, const std::vector<uint> &listZone, bool oversampling);

	// Exclude all the patch of a landscape from refine all
	void excludeAllPatchFromRefineAll (CLandscape &landscape, std::vector<uint> &listZone, bool exclude);

	// Get positions and normal from a patch
	void getPatchNormalAndPositions (std::vector<CLumelDescriptor>& lumels, CLandscape &landscape, uint zoneToLight, uint patch, 
									CPatchUVLocator *locator, bool *binded);

	// Calc sky contribution
	float calcSkyContribution (sint s, sint t, float height, float skyIntensity, const CVector& normal);


	// Get max height
	uint8 getMaxPhi (sint s, sint t, sint deltaS, sint deltaT, float heightPos);

	// Ray trace a position
	void rayTrace (const CVector& position, const CVector& normal, float s, float t, uint patchId, float &factor, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu);
		
	// Eval a normal in the neighborhood
	void getNormal (const NL3D::CPatch *pPatch, sint16 lumelS, sint16 lumelT, std::vector<NL3D::CPatchUVLocator> &locator, 
					const std::vector<NL3D::CPatch::CBindInfo> &bindInfo, const std::vector<bool> &binded, std::set<uint64>& visited, 
					float deltaS, float deltaT, uint rotation, const NL3D::CBezierPatch &bezierPatch, uint lastEdge=5);

	// Raytrace a triangle
	void rayTraceTriangle (const NLMISC::CTriangle& toOverSample, NLMISC::CVector& normal, uint order, float s, float t, float &factor, uint &tested, uint patchId);

	// Raytrace a triangle
	void testRaytrace (const CVector& position, const CVector& normal, const CPlane &plane, float s, float t, uint patchId, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu);

	// Tell if the edge lumel must be oversampled
	bool isLumelOnEdgeMustBeOversample (uint patch, uint edge, sint s, sint t, const std::vector<bool> &binded, 
										const std::vector<bool> &oversampleEdges, std::vector<CPatchUVLocator> &locator, 
										uint8 shadowed, std::vector<std::vector<uint8> >& shadowBuffer);

	// The quad grid
	CQuadGrid<const CTriangle*>					_QuadGrid[MAX_CPU_PROCESS];
	NLMISC::CMatrix								_RayBasis;
	NLMISC::CVector								_RayAdd;
	NLMISC::CVector								_LightDirection;
	uint										_ZoneToLight;
	NL3D::CLandscape							*_Landscape;
	float										_ShadowBias;
	bool										_Softshadow;
	std::vector<std::vector<uint8> >			_ShadowArray;

	// Processes
	uint										_ProcessCount;
	uint										_ProcessExited;

	// The shape
	CShape										_Shape;
	float										_ShapeArea;
	float										_ShapeRadius;
	float										_FallofDistance;
												
	// The heightfield
	std::vector<float>							_HeightField;
	sint										_HeightFieldCellCount;
	NLMISC::CVector								_OrigineHeightField;
	float										_HeightfieldCellSize;

	// Zone infos
	std::vector<CPatchInfo>						_PatchInfo;
	std::vector<CBorderVertex>					_BorderVertices;
	std::vector<std::vector<CLumelDescriptor> > _Lumels;
	std::vector<std::vector<CBezierPatch> >		_BezierPatch;
	std::vector<std::vector<std::vector<CPatchUVLocator> > >	_Locator;
	std::vector<std::vector<std::vector<CPatch::CBindInfo> > >	_BindInfo;
	std::vector<std::vector<std::vector<bool> > >	_Binded;
	std::vector<std::vector<bool> >				_OversampleEdges;
	std::map<uint, uint>						_ZoneId;
	
	// Get normal info
	const NL3D::CPatch							*_GetNormalPatch;
	NLMISC::CVector								_GetNormalNormal;
	uint										_GetNormalRadius;
	uint										_GetNormalSqRadius;
	static sint16								_GetNormalDeltaS[4];
	static sint16								_GetNormalDeltaT[4];

	// Triangle list allocator
	NLMISC::CPoolMemory<CTriangleList>			_TriangleListAllocateur;

	// Precalc
	NLMISC::CVector								_K[256][8];
};

} // NL3D


#endif // NL_ZONE_LIGHTER_2001_H

/* End of zone_lighter.h */
