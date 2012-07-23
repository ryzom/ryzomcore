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

#ifndef NL_ZONE_LIGHTER_H
#define NL_ZONE_LIGHTER_H

#include "nel/misc/triangle.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/misc/mutex.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/pool_memory.h"
#include "nel/misc/random.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/mutex.h"

#include "nel/3d/frustum.h"
#include "nel/3d/zone.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/cube_grid.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/tile_light_influence.h"

#include <list>

namespace NL3D
{

class CZone;
class CPatchUVLocator;
class IShape;
class CLightRunnable;
class CCalcLightableShapeRunnable;
class CMeshGeom;
class CMeshBase;
class CMeshMRMGeom;
class CWaterShape;
class CRenderZBuffer;
class CMaterial;

// The zone lighter
class CZoneLighter
{
	friend class NL3D::CLightRunnable;
	friend class NL3D::CRenderZBuffer;
public:
	CZoneLighter ();
	virtual ~CZoneLighter () {}

	// Light decription structure
	class CLightDesc
	{
	public:
		// Default Ctor
		CLightDesc ();

		// Grid size
		uint					GridSize;

		// Grid size
		float					GridCellSize;

		// Height field size
		float					HeightfieldSize;

		// Height field cell size
		float					HeightfieldCellSize;

		// Render shadows
		bool					Shadow;


		/// \name Sun parameters


		// Sun direction
		NLMISC::CVector			SunDirection;

		// Distance of the sun
		float					SunDistance;

		// FOV of the sun in radian
		float					SunFOV;

		// Center of the landscape pointed by the sun
		NLMISC::CVector			SunCenter;

		// Sun radius, (for softshadow sampling)
		float					SunRadius;


		/// \name ZBuffer parameters


		// Landscape ZBuffers size for all the landscape. There is one zbuffer like this one per softshadow sample.
		uint					ZBufferLandscapeSize;

		// Object ZBuffers size for all the landscape. This zbuffer is typically finer. There is only one zbuffer like this.
		uint					ZBufferObjectSize;


		/// \name Softshadows


		// Square root of the number of soft shadow samples
		uint					SoftShadowSamplesSqrt;

		// Soft shadow jitter (0 ~ 1) to smooth softshadow aliasing when sampling number is small
		float					SoftShadowJitter;

		// Use sun contribution
		bool					SunContribution;

		// Use sky contribution
		bool					SkyContribution;

		// Sky intensity [0, 1]
		float					SkyIntensity;

		// z-bias for water rendering
		float					WaterShadowBias;

		// Water ambient intensity
		float					WaterAmbient;

		// This is used to modulate the direct contribution of light to water
		float					WaterDiffuse;

		// Sky contribution for water
		bool					SkyContributionForWater;

		// True to enable modulation with water previous texture
		bool                    ModulateWaterColor;

		/// Evaluation of the max height, in meters, of the vegetables. Needed when we compute whether a tile is below or above water.
		float					VegetableHeight;

		// Nombrer of CPU used
		uint					NumCPU;
	};

	// A triangle used to light the zone
	class CTriangle
	{
		friend class CZoneLighter;
	public:
		enum TFlags
		{
			Landscape = 1,
			DoubleSided = 2,
			ClampU = 4,
			ClampV = 8,
		};

		// Ctors
		CTriangle (const NLMISC::CTriangle& triangle, bool doubleSided, const NLMISC::CBitmap *texture, bool clampU, bool clampV, float *u, float *v, uint8 alphaTestThreshold)
		{
			// Copy the triangle
			Triangle=triangle;

			// Draw it in the heightmap
			Flags = 0;

			// Double sided
			Flags |= doubleSided ? DoubleSided : 0;

			// Texture
			Texture = texture;

			// Texture used ?
			if (Texture)
			{
				// Alpha test threshold
				AlphaTestThreshold = alphaTestThreshold;

				// Flags
				Flags |= clampU ? ClampU : 0;
				Flags |= clampV ? ClampV : 0;

				// Copy U and V
				for (uint i=0; i<3; i++)
				{
					U[i] = u[i];
					V[i] = v[i];
				}
			}
		}

		// Ctor for lansdcape triangles
		CTriangle (const NLMISC::CTriangle& triangle)
		{
			// Copy the triangle
			Triangle=triangle;

			// Draw it in the heightmap
			Flags = Landscape;

			// Texture
			Texture = NULL;
		}

		// The triangle
		NLMISC::CTriangle	Triangle;

		// Flags
		uint8				Flags;
		uint8				AlphaTestThreshold;

		// UV
		float				U[3];
		float				V[3];

		// Texture
		const NLMISC::CBitmap		*Texture;

		// Other info
		const CPlane		&getPlane() const {return _Plane;}
	private:
		NLMISC::CPlane		_Plane;
	};

#define SHAPE_VERTICES_MAX 100
#define SHAPE_MAX 500
#define MAX_CPU_PROCESS 10

	// A lumel
	class CLumelDescriptor
	{
	public:
		CLumelDescriptor ()
		{
		}
		NLMISC::CVector					Position;
		NLMISC::CVector					Normal;
		float							S;
		float							T;
	};

	// A lumel corner
	class CLumelCorner
	{
	public:
		CLumelCorner ()
		{
		}
		NLMISC::CVector					Position;
	};

	// The ZBuffer struct
	class CZBuffer
	{
	public:
		std::vector<float>						Pixels;
		NL3D::CFrustum							WorldToZBufferFrustum;
		NLMISC::CMatrix							WorldToZBuffer;
		sint									LocalZBufferXMin;
		sint									LocalZBufferXMax;
		sint									LocalZBufferYMin;
		sint									LocalZBufferYMax;
		float									LocalZBufferZMin;
		float									LocalZBufferZMax;
		sint									LocalZBufferWidth;
		sint									LocalZBufferHeight;
		sint									ZBufferPixelSize;
		NLMISC::CVector							BoundingBoxVectors[8];
	};

	// A hierarchical heightfield
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

	// patch a zone, to compute only the tile water states.
	void computeTileFlagsOnly (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description,
		std::vector<uint> &listZone);

	// Add triangles from a landscape
	void addTriangles (CLandscape &landscape, std::vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray);

	// Add triangles from a transform shape. Work only for CMesh, CMultiMesh and CMeshMRM all without skinning.
	void addTriangles (const IShape &shape, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	/** Some shape (water shapes for now) can be lit.
	  * This add such a shape to the process of lighting.
	  * \see isLightableShape()
	  */
	void addLightableShape(IShape *shape, const NLMISC::CMatrix& modelMT);

	/// Add a water shape. This is needed to decide whether tiles are above / below water
	void addWaterShape(CWaterShape *shape, const NLMISC::CMatrix &MT);

	/// get the number of water shapes added
	uint getNumWaterShape() const {return (uint)_WaterShapes.size();}

	/// check whether a shape is lightable.
	static bool isLightableShape(IShape &shape);

	// Progress callback
	virtual void progress (const char *message, float progress) {}

	// Compute shadow attenuation
	float attenuation (const CVector &pos, const CZoneLighter::CLightDesc &description);

	/// \name Static PointLights mgt.
	//@{

	/// Append a static point light to compute. call at setup stage (before light() ).
	void			addStaticPointLight(const CPointLightNamed &pln);

	//@}

private:
	friend class CCalcLightableShapeRunnable;
	// Add triangles from a non skinned CMeshGeom.
	void addTriangles (const CMeshBase &meshBase, const CMeshGeom &meshGeom, const NLMISC::CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	// Add triangles from a non skinned CMeshMRMGeom.
	void addTriangles (const CMeshBase &meshBase, const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray);

	// One process method
	void processCalc (uint process, const CLightDesc& description);

	// Build internal zone information
	void buildZoneInformation (CLandscape &landscape, const std::vector<uint> &listZone, const CLightDesc &lightDesc);

	// Exclude all the patch of a landscape from refine all
	void excludeAllPatchFromRefineAll (CLandscape &landscape, std::vector<uint> &listZone, bool exclude);

	// Get positions and normal from a patch
	void getPatchNormalAndPositions (std::vector<CLumelDescriptor>& lumels, CLandscape &landscape, uint zoneToLight, uint patch,
									CPatchUVLocator *locator, bool *binded);

	// Calc sky contribution. Used by getSkyContribution
	float calcSkyContribution (sint s, sint t, float height, float skyIntensity, const CVector& normal) const;

	/// compute the sky contribution at the given position
	float getSkyContribution(const CVector &pos, const CVector &normal, float SkyIntensity) const;


	// Get max height
	uint8 getMaxPhi (sint s, sint t, sint deltaS, sint deltaT, float heightPos) const;

	// Eval a normal in the neighborhood
	void getNormal (const NL3D::CPatch *pPatch, sint16 lumelS, sint16 lumelT, std::vector<NL3D::CPatchUVLocator> &locator,
					const std::vector<NL3D::CPatch::CBindInfo> &bindInfo, const std::vector<bool> &binded, std::set<uint64>& visited,
					float deltaS, float deltaT, uint rotation, const NL3D::CBezierPatch &bezierPatch, uint lastEdge=5);

	// Tell if the edge lumel must be oversampled
	bool isLumelOnEdgeMustBeOversample (uint patch, uint edge, sint s, sint t, const std::vector<bool> &binded,
										const std::vector<bool> &oversampleEdges, std::vector<CPatchUVLocator> &locator,
										uint8 shadowed, std::vector<std::vector<uint8> >& shadowBuffer);

	/// Struct describing the position of a lightable shape
	struct  CShapeInfo
	{
		IShape			*Shape;
		NLMISC::CMatrix MT;
	};
	/// A vector of lightable shapes
	typedef std::vector<CShapeInfo>	TShapeVect;


	/// Launch a set of threads to perform lighting of lightable shapes
	void lightShapes(uint zoneID, const CLightDesc& description);

	/// Process lighting for a set of lightable shapes. This is called by the threads created by lightShapes().
	void processLightableShapeCalc (uint process,
									TShapeVect *shapeToLit,
									uint firstShape,
									uint lastShape,
									const CLightDesc& description);

	/// Compute the lighting for a single lightable shape
	void lightSingleShape(CShapeInfo &lsi, const CLightDesc& description, uint cpu);

	/// Compute the lighting for a water shape
	void lightWater(CWaterShape &ws, const CMatrix &MT, const CLightDesc& description, uint cpu);

	/** Make a quad grid of all the water shapes that where registered by calling addWaterShape()
	  * The vector of water shapes is released then
	  * \param bbox the bbox of the zone containing the water shapes
	  */
	void makeQuadGridFromWaterShapes(NLMISC::CAABBox zoneBBox);


	/** For each tile of the current zone, check whether it below or above water.
	  * The result is stored in the flags of the tile.
	  * The quadtree is removed then.
	  */
	void computeTileFlagsForPositionTowardWater(const CLightDesc &lightDesc,
												std::vector<const CTessFace*> &tessFaces
												);


	/** If no water surface overlap the zone, so we set all the flags to 'AboveWater", or don't change them if they
	  * were set to 'DisableVegetable'
	  */
	void setTileFlagsToDefault(std::vector<const CTessFace*> &tessFaces);

	/** This copy the flags of the tiles from the source zone to a dest zone (result of the lighting).
	  * This is needed beacuse these flags are updated to say whether a given tile is above  / below water
	  * IMPORTANT : the source and destination zones must match of course...
	  */
	static void copyTileFlags(CZone &destZone, const CZone &srcZone);

	// Get texture from a material for alpha test
	bool getTexture (const CMaterial &material, NLMISC::CBitmap *&result, bool &clampU, bool &clampV, uint8 &alphaTestThreshold, bool &doubleSided);

	// Give a thread a patch to compute
	uint getAPatch (uint process);

	// The quad grid
	CQuadGrid<const CTriangle*>					_QuadGrid[MAX_CPU_PROCESS];
	NLMISC::CMatrix								_RayBasis;
	NLMISC::CVector								_SunDirection;
	uint										_ZoneToLight;
	NL3D::CLandscape							*_Landscape;
	float										_ShadowBias;
	bool										_Softshadow;
	std::vector<std::vector<uint8> >			_ShadowArray;

	// Processes
	NLMISC::CSynchronized<std::vector<bool> >	_PatchComputed;
	std::vector<uint>							_LastPatchComputed;
	uint										_NumberOfPatchComputed;
	uint										_ProcessCount;
	uint64										_CPUMask;
	volatile uint								_ProcessExited;

	// *** Bitmap sharing
	std::map<std::string, NLMISC::CBitmap>		_Bitmaps;

	// *** The zbuffer

	// ZBuffer mutex
	NLMISC::CFastMutex							_Mutex;

public:
	// Zbuffer pixels in meters
	std::vector<CZBuffer>						_ZBufferLandscape;
	CZBuffer									_ZBufferObject;

private:
	// ZBuffer lookup overflow
	bool										_ZBufferOverflow;

	// Random generator
	NLMISC::CRandom								_Random;

	// The heightfield
	std::vector<float>							_HeightField;
	sint										_HeightFieldCellCount;
	NLMISC::CVector								_OrigineHeightField;
	float										_HeightfieldCellSize;

	class CLumelCorners
	{
	};

	// Zone infos
	std::vector<CPatchInfo>						_PatchInfo;
	std::vector<CBorderVertex>					_BorderVertices;
	std::vector<std::vector<CLumelDescriptor> > _Lumels;
	//std::vector<std::vector<CLumelCorners> >	_LumelCorners;
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

	// Precalc
	NLMISC::CVector								_K[256][8];


	/// \name Static PointLights mgt.
	//@{

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
		// Final id of the pointLight in the Zone.
		uint					DstId;

		CPointLightRT();

		/** Tells if a point is visible from this light. NB: test first if in BSphere
		 *	If occluded or out of radius, return false, else return true.
		 *	Also Skip if the light is an Ambient, and skip if the light is a spot and if the position is out of the cone
		 */
		bool		testRaytrace(const CVector &v);
	};


	/// For sort()
	struct		CPredPointLightToPoint
	{
		CVector		Point;

		bool	operator() (CPointLightRT *pla, CPointLightRT *plb) const;
	};


	/// An UnCompressed TileLightInfluence.
	struct	CTileLightInfUnpack
	{
		CPointLightRT	*Light[CTileLightInfluence::NumLightPerCorner];
		float			LightFactor[CTileLightInfluence::NumLightPerCorner];
	};

	/// A patch with UnCompressed TileInfluences.
	struct	CPatchForPL
	{
		uint	OrderS, OrderT;
		uint	WidthTLI, HeightTLI;
		std::vector<CTileLightInfUnpack>		TileLightInfluences;
	};

	/// List of PointLights
	std::vector<CPointLightRT>		_StaticPointLights;
	/// QuadGrid of PointLights. Builded from _StaticPointLights
	CQuadGrid<CPointLightRT*>		_StaticPointLightQuadGrid;


	/// Fill CubeGrid, and set PointLightRT in _StaticPointLightQuadGrid.
	void			compilePointLightRT(uint gridSize, float gridCellSize, std::vector<CTriangle>& obstacles, bool doShadow);

	/** Process the zone, ie process _PatchInfo.
	 *	MultiCPU: not done for now. Be aware of CPointLightRT::RefCount!!!!
	 */
	void			processZonePointLightRT(std::vector<CPointLightNamed> &listPointLight);

	//@}



	/// lightable shapes
	TShapeVect									_LightableShapes;
	uint										_NumLightableShapesProcessed;

	/** List of all the water shapes in the zone. We need them to check whether the tiles are above / below water, or if theyr intersect water
	  */
	TShapeVect									_WaterShapes;

	typedef CQuadGrid<CShapeInfo>				TWaterShapeQuadGrid;

	TWaterShapeQuadGrid							_WaterShapeQuadGrid;

	/// Some constants
	static const sint8 TriangleIndexes[10][2][3];
	static const sint8 VertexThanCanBeSnappedOnABorder[8][4];
	static const sint8 VertexThanCanBeSnappedOnACorner[3][2];
};

} // NL3D


#endif // NL_ZONE_LIGHTER_H

/* End of zone_lighter.h */
