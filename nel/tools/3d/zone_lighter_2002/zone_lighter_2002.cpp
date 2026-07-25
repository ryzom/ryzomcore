/** \file 3d/zone_lighter.cpp
 * Class to light zones
 *
 * $Id$
 */

/* Copyright, 2000 Nevrax Ltd.
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



#include "zone_lighter_2002.h"
#include "nel/3d/landscape.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/texture_file.h"





#include "nel/misc/common.h"
#include "nel/misc/thread.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"


// Define this to use hardware soft shadows
//#define HARDWARE_SOFT_SHADOWS

#ifdef HARDWARE_SOFT_SHADOWS

#include "nel/3d/u_driver.h"

#endif // HARDWARE_SOFT_SHADOWS


#ifdef NL_OS_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"
#  include "winbase.h"
#  ifdef min
#    undef min
#  endif
#  ifdef max
#    undef max
#  endif
#endif // NL_OS_WINDOWS

using namespace NLMISC;
using namespace NL3D;
using namespace std;

#ifdef HARDWARE_SOFT_SHADOWS

UDriver *drv=NULL;

#define LIGHT_BUFFER_SIZE 16

#endif // HARDWARE_SOFT_SHADOWS
	

// Bad coded: don't set too big else it allocates too much memory.
#define NL3D_ZONE_LIGHTER_CUBE_GRID_SIZE 16


// ***************************************************************************


CZoneLighter2002::CZoneLighter2002 () : _PatchComputed ("PatchComputed"), _TriangleListAllocateur(100000)
{
	
}
	
// ***************************************************************************

void CZoneLighter2002::init ()
{
	// Precalc some values
	for (uint i=0; i<8; i++)
	{
		// Precalc sinP and cosP
		float sinP=(float)(sin((Pi/4)*(i+0.5))-sin((Pi/4)*(i-0.5)));
		float cosP=(float)(cos((Pi/4)*(i-0.5))-cos((Pi/4)*(i+0.5)));

		for (uint phi=0; phi<256; phi++)
		{
			// Real phi
			float fPhi=(float)((Pi/2)*phi/256.0);

			// Tmp result
			float tmp0=(float)(fPhi-sin(2*fPhi)/2);
			float tmp1=(float)sin(fPhi);

			// Calc K
			_K[phi][i].set (tmp0*sinP, tmp0*cosP, (float)((Pi/4)*tmp1*tmp1));
		}
	}

#ifdef HARDWARE_SOFT_SHADOWS
	if (!drv)
	{
		// Mode
		UDriver::CMode mode (LIGHT_BUFFER_SIZE, LIGHT_BUFFER_SIZE, 32, true);
		drv=UDriver::createDriver ();
		drv->setDisplay (mode);
		drv->setMatrixMode2D11 ();
	}
#endif // HARDWARE_SOFT_SHADOWS
}

// ***************************************************************************

// N - NW - W - SW - S - SE - E - NE
static const sint deltaDirection[8][2]=
{
	{1, 0},
	{1, 1},
	{0, 1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
	{0, -1},
	{1, -1},
};

// ***************************************************************************

float CZoneLighter2002::calcSkyContribution (sint s, sint t, float height, float skyIntensity, const CVector& normal) const
{
	// Sky contribution
	float skyContribution;

	// Calc k
	CVector k (0, 0, 0);

	// For the height direction
	for (uint i=0; i<8; i++)
	{
		// Get phi for this point
		uint8 phi=getMaxPhi (s, t, deltaDirection[i][0], deltaDirection[i][1], height);

		// Add to k
		k+=_K[phi][i];
	}

	// Finalize sky contribution
	skyContribution=(float)(skyIntensity*(normal*k)/(2*Pi));
	clamp (skyContribution, 0.f, 1.f);
	return skyContribution;
}

// ***************************************************************************

void NEL3DCalcBase (CVector &direction, CMatrix& matrix)
{
	direction.normalize();
	CVector		I=(fabs(direction*CVector(1.f,0,0))>0.99)?CVector(0.f,1.f,0.f):CVector(1.f,0.f,0.f);
	CVector		K=-direction;
	CVector		J=K^I;
	J.normalize();
	I=J^K;
	I.normalize();
	matrix.identity();
	matrix.setRot(I,J,K, true);
}

// ***************************************************************************

class NL3D::CCalcRunnable : public IRunnable
{
	// Members
	uint			_Process;
	CZoneLighter2002	*_ZoneLighter;
	const CZoneLighter2002::CLightDesc	*_Description;

public:
	IThread			*Thread;

public:
	// Ctor
	CCalcRunnable (uint process, CZoneLighter2002 *zoneLighter, const CZoneLighter2002::CLightDesc *description)
	{
		_ZoneLighter = zoneLighter;
		_Process = process;
		_Description = description;
	}

	// Run method
	void run() NL_OVERRIDE
	{
		// Set the processor mask
		uint64 mask = IProcess::getCurrentProcess()->getCPUMask ();

		// Mask must not be NULL
		nlassert (mask != 0);

		if (mask != 0)
		{
			uint i=0;
			uint count = 0;
			while (1)
			{
				if (mask & (1<<i))
				{
					if (count == _Process)
						break;
					count++;
				}
				i++;
				if (i==64)
					i = 0;
			}
			
			// Set the CPU mask
			Thread->setCPUMask (1<<i);
		}

		_ZoneLighter->processCalc (_Process, *_Description);
		_ZoneLighter->_ProcessExited++;
	}
};


// ***************************************************************************
class NL3D::CCalcLightableShapeRunnable : public IRunnable
{
public:
	CCalcLightableShapeRunnable(uint process,
								CZoneLighter2002 *zoneLighter,
								const CZoneLighter2002::CLightDesc *description,
								CZoneLighter2002::TShapeVect *shapeToLit,
								uint firstShape,
								uint lastShape
								)
		: 
		  _ZoneLighter(zoneLighter), 
		  _Description(description),
		  _ShapesToLit(shapeToLit),
		  _FirstShape(firstShape),
		  _LastShape(lastShape),
		  _Process(process)
	{
	}
	void run() NL_OVERRIDE
	{
		_ZoneLighter->processLightableShapeCalc(_Process, _ShapesToLit, _FirstShape, _LastShape, *_Description);
		_ZoneLighter->_ProcessExited++;
	}
private:
	CZoneLighter2002						*_ZoneLighter;
	const CZoneLighter2002::CLightDesc		*_Description;
	CZoneLighter2002::TShapeVect	*_ShapesToLit;
	uint								_FirstShape, _LastShape;
	uint								_Process;

};

// ***************************************************************************

void CZoneLighter2002::light (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description, std::vector<CTriangle>& obstacles, vector<uint> &listZone)
{
	/*
	 * Lighting algorithm
	 * ------------------
	 *
	 * - Create a quad grid to store shadow casting triangles
	 * - Create a heightfield used for global illumination. Cells are initialized with -FLT_MAX
	 * - Insert each shadow casting triangles in the quad grid and fill the heightfield's cells overlapped by the bounding box of the triangle with
	 * the max height of the triangle if its height is > than the current height in the heightfield's cell.
	 * - 
	 */

	// Backup thread mask
	IThread *currentThread = IThread::getCurrentThread ();
	uint64 threadMask = currentThread->getCPUMask();
	currentThread->setCPUMask (1);

	// Calc the ray basis
	_LightDirection=description.LightDirection;
	NEL3DCalcBase (_LightDirection, _RayBasis);

	// Zone to light
	_ZoneToLight=zoneToLight;

	// Landscape 
	_Landscape=&landscape;

	// Process count
	_ProcessCount=description.NumCPU;
	if (_ProcessCount==0)
	{
		// Create a doomy thread
		IProcess *pProcess=IProcess::getCurrentProcess ();
		_CPUMask = pProcess->getCPUMask();
		_ProcessCount = 0;
		uint64 i;
		for (i=0; i<64; i++)
		{
			if (_CPUMask&((uint64)1<<i))
				_ProcessCount++;
		}
	}
	if (_ProcessCount>MAX_CPU_PROCESS)
		_ProcessCount=MAX_CPU_PROCESS;

	// Number of CPUS used
	printf ("Number of CPU used: %d\n", _ProcessCount);

	// Fallof distance
	_FallofDistance=description.SoftshadowFallof;

	// Shadow bias
	_ShadowBias=description.ShadowBias;

	// Resize the shape array
	_Shape.NumVertex=description.SoftshadowShapeVertexCount;

	// Softshadow ?
	_Softshadow=description.Softshadow;

	// Radius of the shape
	_ShapeRadius=description.SoftshadowBlurSize;
	_RayAdd=_RayBasis.getI();
	_RayAdd+=_RayBasis.getJ();
	_RayAdd.normalize();
	_RayAdd*=1.5f*_ShapeRadius;
	
	// Build the shape
	uint i;
	for (i=0; i<_Shape.NumVertex; i++)
	{
		// Shape is a smapled circle
		float angle=(float)((float)i*2*Pi/_Shape.NumVertex);
		_Shape.Vertices[i]=_RayBasis*CVector (_ShapeRadius*(float)cos (angle), _ShapeRadius*(float)sin (angle), 0);
	}

	// Calculate the area of the shape
	_ShapeArea=0;
	for (i=0; i<_Shape.NumVertex; i++)
	{
		// Sum area of each triangle
		_ShapeArea+=(_Shape.Vertices[i]^_Shape.Vertices[(i+1)%_Shape.NumVertex]).norm();
	}

	// Zone pointer
	CZone *pZone=landscape.getZone (_ZoneToLight);
	if (pZone)
	{
		// Change the quadGrid basis
		CMatrix invRayBasis=_RayBasis;
		invRayBasis.invert ();

		uint cpu;
		for (cpu=0; cpu<_ProcessCount; cpu++)
		{
			_QuadGrid[cpu].changeBase (invRayBasis);

			// Init the quadGrid
			_QuadGrid[cpu].create (description.GridSize, description.GridCellSize);
		}
		
		// Init the heightfield
		_HeightfieldCellSize=description.HeightfieldCellSize;
		_HeightFieldCellCount=(sint)(description.HeightfieldSize/_HeightfieldCellSize);
		nlassert (_HeightFieldCellCount!=0);
		const CAABBoxExt &zoneBB=pZone->getZoneBB();
		_OrigineHeightField=zoneBB.getCenter ()-CVector (description.HeightfieldSize/2, description.HeightfieldSize/2, 0);
		_HeightField.resize (_HeightFieldCellCount*_HeightFieldCellCount, -FLT_MAX);

		// Fill the quadGrid and the heightField
		uint size=obstacles.size();
		for (uint triangleId=0; triangleId<size; triangleId++)
		{
			// Progress bar
			if ( (triangleId&0xff) == 0)
				progress ("Build quadtree and heightfield", (float)triangleId/(float)size);

			// Triangle ref
			CZoneLighter2002::CTriangle& triangle=obstacles[triangleId];

			// Calc the plane
			triangle.Plane.make (triangle.Triangle.V0, triangle.Triangle.V1, triangle.Triangle.V2);

			// Calc the clipping plane
			CVector edgeDirection[3];
			CVector point[3];
			point[0]=triangle.Triangle.V0;
			edgeDirection[0]=triangle.Triangle.V1-triangle.Triangle.V0;
			point[1]=triangle.Triangle.V1;
			edgeDirection[1]=triangle.Triangle.V2-triangle.Triangle.V1;
			point[2]=triangle.Triangle.V2;
			edgeDirection[2]=triangle.Triangle.V0-triangle.Triangle.V2;
			
			// Flip plane ?
			bool flip=((triangle.Plane.getNormal()*(-_LightDirection))<0);

			// For each plane
			for (uint edge=0; edge<3; edge++)
			{
				// Plane normal
				edgeDirection[edge]=edgeDirection[edge]^(-_LightDirection);
				edgeDirection[edge].normalize();
				if (flip)
					edgeDirection[edge]=-edgeDirection[edge];

				// Make a plane
				triangle.ClippingPlanes[edge].make (edgeDirection[edge], point[edge]);
			}

			// Look for the min coordinate, in the RayBasis
			CVector irbMinv;
			CVector		irbV0= invRayBasis * triangle.Triangle.V0;
			CVector		irbV1= invRayBasis * triangle.Triangle.V1;
			CVector		irbV2= invRayBasis * triangle.Triangle.V2;
			irbMinv.minof (irbV0, irbV1);
			irbMinv.minof (irbMinv, irbV2);

			// Look for the max coordinate, in the RayBasis
			CVector irbMaxv;
			irbMaxv.maxof (irbV0, irbV1);
			irbMaxv.maxof (irbMaxv, irbV2);

			// Insert in the quad grid
			for (cpu=0; cpu<_ProcessCount; cpu++)
				// Set the coord in World Basis.
				_QuadGrid[cpu].insert (_RayBasis * irbMinv, _RayBasis * irbMaxv, &triangle);


			// Look for the min coordinate, in World Basis
			CVector minv;
			minv.minof (triangle.Triangle.V0, triangle.Triangle.V1);
			minv.minof (minv, triangle.Triangle.V2);

			// Look for the max coordinate, in World Basis
			CVector maxv;
			maxv.maxof (triangle.Triangle.V0, triangle.Triangle.V1);
			maxv.maxof (maxv, triangle.Triangle.V2);


			// Lanscape tri ?
			if (triangle.ZoneId!=0xffffffff)
			{
				// Fill the heightfield
				sint minX=std::max (0, (sint)floor (0.5f+(minv.x-_OrigineHeightField.x)/_HeightfieldCellSize));
				sint maxX=std::min (_HeightFieldCellCount, (sint)floor (0.5f+(maxv.x-_OrigineHeightField.x)/_HeightfieldCellSize));
				sint minY=std::max (0, (sint)floor (0.5f+(minv.y-_OrigineHeightField.y)/_HeightfieldCellSize));
				sint maxY=std::min (_HeightFieldCellCount, (sint)floor (0.5f+(maxv.y-_OrigineHeightField.y)/_HeightfieldCellSize));

				// Calc position in the heightfield
				for (sint y=minY; y<maxY; y++)
				for (sint x=minX; x<maxX; x++)
				{
					// Valid position, try to insert it
					if (maxv.z>_HeightField[x+y*_HeightFieldCellCount])
					{
						// New height in this cell
						_HeightField[x+y*_HeightFieldCellCount]=maxv.z;
					}
				}
			}
		}

		// Retrieve the zone to fill its shaded value
		pZone->retrieve (_PatchInfo, _BorderVertices);

		// Number of patch
		uint patchCount=_PatchInfo.size();

		// Bit array to know if the lumel is shadowed
		if (description.Shadow)
			_ShadowArray.resize (patchCount);

		// A lumel vector by patch
		vector<vector<CLumelDescriptor>	> lumels;
		lumels.resize (patchCount);

		// Build zone informations
		buildZoneInformation (landscape,
							  listZone,
							  description.Oversampling!=CLightDesc::NoOverSampling,
							  description);

	}

	// Number of patch
	uint patchCount=_PatchInfo.size();

	// Reset patch count
	{
		NLMISC::CSynchronized<std::vector<bool> >::CAccessor access (&_PatchComputed);
		access.value().resize (0);
		access.value().resize (patchCount, false);
	}

	// Patch by thread
	uint patchCountByThread = patchCount/_ProcessCount;
	patchCountByThread++;

	// Patch to allocate
	uint firstPatch=0;
	_NumberOfPatchComputed = 0;

	_ProcessExited=0;

	// Set the thread state
	_LastPatchComputed.resize (_ProcessCount);

	// Launch threads
	for (uint process=1; process<_ProcessCount; process++)
	{
		// Last patch
		uint lastPatch=firstPatch+patchCountByThread;
		if (lastPatch>patchCount)
			lastPatch=patchCount;

		// Last patch computed
		_LastPatchComputed[process] = firstPatch;

		// Create a thread
		CCalcRunnable *runnable = new CCalcRunnable (process, this, &description);
		IThread *pThread=IThread::create (runnable);
		runnable->Thread = pThread;
		
		// New first patch
		firstPatch=lastPatch;

		// Launch
		pThread->start();
	}

	// My thread
	uint lastPatch=firstPatch+patchCountByThread;
	if (lastPatch>patchCount)
		lastPatch=patchCount;
	_LastPatchComputed[0] = firstPatch;
	CCalcRunnable thread (0, this, &description);
	thread.Thread = currentThread;
	thread.run();

	// Wait for others processes
	while (_ProcessExited!=_ProcessCount)
	{
		nlSleep (10);
	}

	// Reset old thread mask
	currentThread->setCPUMask (threadMask);

	// Progress bar
	progress ("Compute Influences of PointLights", 0.f);

	// Compute PointLight influences on zone.
	// Some precalc.
	compilePointLightRT(description.GridSize, description.GridCellSize, obstacles, 
		description.Shadow || description.Softshadow );
	// Influence patchs and get light list of interest
	std::vector<CPointLightNamed>	listPointLight;
	processZonePointLightRT(listPointLight);


	// Rebuild the zone

	// Progress bar
	progress ("Compress the lightmap", 0.6f);

	// Build, with list of lights.
	CZoneInfo	zinfo;
	zinfo.ZoneId= _ZoneToLight;
	zinfo.Patchs= _PatchInfo;
	zinfo.BorderVertices= _BorderVertices;
	zinfo.PointLights= listPointLight;
	output.build (zinfo);

	/// copy the tiles flags from the zone to light to the output zone
	copyTileFlags(output, *(landscape.getZone(zoneToLight)));

	/// Perform lightning of some ig's of the current zone (if any)
	lightShapes(zoneToLight, description);
}


// *************************************************************************************
void CZoneLighter2002::copyTileFlags(CZone &destZone, const CZone &srcZone)
{
	nlassert(destZone.getZoneId() == srcZone.getZoneId());
	for (sint k = 0; k < srcZone.getNumPatchs(); ++k)
	{
		destZone.copyTilesFlags(k, srcZone.getPatch(k));
	}
}

// ***************************************************************************
float CZoneLighter2002::getSkyContribution(const CVector &pos, const CVector &normal, float skyIntensity) const
{	
	float s=(pos.x-_OrigineHeightField.x)/_HeightfieldCellSize;
	float t=(pos.y-_OrigineHeightField.y)/_HeightfieldCellSize;
	sint sInt=(sint)(floor (s+0.5f));
	sint tInt=(sint)(floor (t+0.5f));

	// Bilinear
	float skyContributionTab[2][2];
	skyContributionTab[0][0] = calcSkyContribution (sInt-1, tInt-1, pos.z, skyIntensity, normal);
	skyContributionTab[1][0] = calcSkyContribution (sInt, tInt-1, pos.z, skyIntensity, normal);
	skyContributionTab[1][1] = calcSkyContribution (sInt, tInt, pos.z, skyIntensity, normal);
	skyContributionTab[0][1] = calcSkyContribution (sInt-1, tInt, pos.z, skyIntensity, normal);
	
	float sFact=s+0.5f-sInt;
	float tFact=t+0.5f-tInt;
	return (skyContributionTab[0][0]*(1.f-sFact) + skyContributionTab[1][0]*sFact)*(1.f-tFact) +
		(skyContributionTab[0][1]*(1.f-sFact) + skyContributionTab[1][1]*sFact)*tFact;	
}


// ***************************************************************************
void CZoneLighter2002::processCalc (uint process, const CLightDesc& description)
{
	// *** Raytrace each patches

	// Pointer on the zone
	CZone *pZone=_Landscape->getZone (_ZoneToLight);

	// Get a patch
	uint patch = getAPatch (process);
	while (patch != 0xffffffff)
	{
		// For each patch
		if (description.Shadow)
		{
			// Shape array
			CMultiShape *shapeArray=new CMultiShape;
			CMultiShape *shapeArrayTmp=new CMultiShape;
			shapeArray->Shapes.reserve (SHAPE_MAX);
			shapeArrayTmp->Shapes.reserve (SHAPE_MAX);

			// Lumels
			std::vector<CLumelDescriptor> &lumels=_Lumels[patch];
		
			// Lumel count
			uint lumelCount=lumels.size();
			CPatchInfo &patchInfo=_PatchInfo[patch];
			nlassert (patchInfo.Lumels.size()==lumelCount);

			// Resize shadow array
			_ShadowArray[patch].resize (lumelCount);

			// For each lumel
			for (uint lumel=0; lumel<lumelCount; lumel++)
			{
				float factor=0;
				rayTrace (lumels[lumel].Position, lumels[lumel].Normal, lumels[lumel].S, lumels[lumel].T, patch, factor, *shapeArray, *shapeArrayTmp, process);
				patchInfo.Lumels[lumel]=(uint)(factor*255);
			}
			delete shapeArray;
			delete shapeArrayTmp;
		}
		else
		{
			// Lumels
			std::vector<CLumelDescriptor> &lumels=_Lumels[patch];
		
			// Lumel count
			uint lumelCount=lumels.size();
			CPatchInfo &patchInfo=_PatchInfo[patch];
			nlassert (patchInfo.Lumels.size()==lumelCount);

			// For each lumel
			for (uint lumel=0; lumel<lumelCount; lumel++)
			{
				// Not shadowed
				patchInfo.Lumels[lumel]=255;
			}
		}

		// *** Antialising
		
		// Id of this zone in the array
		uint zoneNumber=_ZoneId[_ZoneToLight];

		// Enabled ?
		if ((description.Shadow)&&(description.Oversampling!=CLightDesc::NoOverSampling))
		{
			// Get a patch pointer
			const CPatch *pPatch=(const_cast<const CZone*>(pZone))->getPatch (patch);

			// Get the patch info
			CPatchInfo &patchInfo=_PatchInfo[patch];

			// Get order of the patch
			uint orderLumelS=pPatch->getOrderS()<<2;
			uint orderLumelT=pPatch->getOrderT()<<2;

			// ** Pointer on arries
			vector<bool> &binded=_Binded[zoneNumber][patch];
			vector<bool> &oversampleEdges=_OversampleEdges[patch];
			vector<CPatchUVLocator> &locator=_Locator[zoneNumber][patch];
			std::vector<CLumelDescriptor> &lumels=_Lumels[patch];

			// Shadow array
			vector<uint8> &shadowPatch=_ShadowArray[patch];

			// Go for each lumel
			for (uint t=0; t<orderLumelT; t++)
			for (uint s=0; s<orderLumelS; s++)
			{
				// Over sample this lumel
				bool oversample=false;
				uint8 shadowed=shadowPatch[s+t*orderLumelS];

				// Left..
				if (s==0)
				{
					// Edge test
					oversample=isLumelOnEdgeMustBeOversample (patch, 0, s, t, binded, oversampleEdges, locator, shadowed, _ShadowArray);
				}
				else
				{
					// Internal test
					oversample=(shadowed!=shadowPatch[(s-1)+t*orderLumelS]);
				}

				// Bottom..
				if (!oversample)
				{
					if (t==(orderLumelT-1))
					{
						// Edge test
						oversample=isLumelOnEdgeMustBeOversample (patch, 1, s, t, binded, oversampleEdges, locator, shadowed, _ShadowArray);
					}
					else
					{
						// Internal test
						oversample=(shadowed!=shadowPatch[s+(t+1)*orderLumelS]);
					}

					// Right..
					if (!oversample)
					{
						if (s==(orderLumelS-1))
						{
							// Edge test
							oversample=isLumelOnEdgeMustBeOversample (patch, 2, s, t, binded, oversampleEdges, locator, shadowed, _ShadowArray);
						}
						else
						{
							// Internal test
							oversample=(shadowed!=shadowPatch[(s+1)+t*orderLumelS]);
						}

						// Top..
						if (!oversample)
						{
							if (t==0)
							{
								// Edge test
								oversample=isLumelOnEdgeMustBeOversample (patch, 3, s, t, binded, oversampleEdges, locator, shadowed, _ShadowArray);
							}
							else
							{
								// Internal test
								oversample=(shadowed!=shadowPatch[s+(t-1)*orderLumelS]);
							}
						}
					}
				}

				// Must oversample ?
				if (oversample)
				{
					// LumelId
					uint lumel=s+t*orderLumelS;

					// Lighting
					float factor=0;

					// Number of ray clipped
					uint tested=0;

					// For each triangle
					CTriangleList *list=lumels[lumel].TriangleList;
					while (list!=NULL)
					{
						// Raytrace this triangle							
						rayTraceTriangle (list->Triangle, lumels[lumel].Normal, description.Oversampling, lumels[lumel].S, lumels[lumel].T, factor, tested, patch);

						// Next triangle
						list=list->Next;
					}

					// Set new shadow value
					nlassert (tested!=0);
					if (tested!=0)
						patchInfo.Lumels[lumel]=(uint)(255.f*factor/(float)tested);
				}
			}
		}

		// *** Lighting
		
		// Get the patch info
		CPatchInfo &patchInfo=_PatchInfo[patch];

		// ** Pointer on arries
		std::vector<CLumelDescriptor> &lumels=_Lumels[patch];

		// Go for light each lumel
		for (uint lumel=0; lumel<lumels.size(); lumel++)
		{
			// Sky contribution
			float skyContribution;
				
			if (description.SkyContribution)
			{								
				skyContribution = getSkyContribution(lumels[lumel].Position, lumels[lumel].Normal, description.SkyIntensity);
			}
			else
			{
				skyContribution = 0.f;
			}

			// Sun contribution
			float sunContribution;
			if (description.SunContribution)
			{
				sunContribution=(-lumels[lumel].Normal*_LightDirection)-skyContribution;
				clamp (sunContribution, 0.f, 1.f);
			}
			else
				sunContribution=0;

			// Final lighting
			sint finalLighting=(sint)(255.f*(((float)patchInfo.Lumels[lumel])*sunContribution/255.f+skyContribution));
			clamp (finalLighting, 0, 255);
			patchInfo.Lumels[lumel]=finalLighting;
		}

		// Next patch
		patch = getAPatch (process);
	}
}

// ***************************************************************************

uint8 CZoneLighter2002::getMaxPhi (sint s, sint t, sint deltaS, sint deltaT, float heightPos) const
{
	// Start position
	s+=deltaS;
	t+=deltaT;

	// Distance increment
	float stepDistance=CVector (deltaS*_HeightfieldCellSize, deltaT*_HeightfieldCellSize,0).norm ();

	// Current distance
	float distance=stepDistance;

	// Max height
	float maxHeight=0;
	float maxTanTeta=0;

	// For all the line
	while ((s<_HeightFieldCellCount)&&(t<_HeightFieldCellCount)&&(s>=0)&&(t>=0))
	{
		// Get height
		float height=_HeightField[s+t*_HeightFieldCellCount];
		height-=heightPos;

		// Better ?
		if (height>maxHeight)
		{
			// Calc sin teta
			float tanTeta=height/distance;
			nlassert (tanTeta>=0);

			// Better ?
			if (tanTeta>maxTanTeta)
			{
				// New max height
				maxHeight=height;
				maxTanTeta=tanTeta;
			}
		}
		s+=deltaS;
		t+=deltaT;
		distance+=stepDistance;
	}

	// return phi
	float teta=(float)atan (maxTanTeta);
	nlassert (teta>=0);
	nlassert (teta<=Pi/2);
	clamp (teta, 0.f, (float)Pi/2);
	sint res=(sint)((Pi/2-teta)*256/(Pi/2));
	clamp (res, 0, 255);
	return (uint8)res;
}

// ***************************************************************************

#define AllFront 0
#define AllBack 1
#define Clipped 2

// ***************************************************************************

void CZoneLighter2002::testRaytrace (const CVector& position, const CVector& normal, const CPlane &plane, float s, float t, uint patchId, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu)
{
	 // Clear the selection of the quad tree
	_QuadGrid[cpu].clearSelection ();

	// Light position
	CVector lightPos=position-(_LightDirection*1000.f);

	// Select an element with the X axis as a 3d ray
	_QuadGrid[cpu].select (lightPos-_RayAdd, lightPos+_RayAdd);

	// Tmp
	CShape back;
	CShape front;
	CShape copy;

#ifdef HARDWARE_SOFT_SHADOWS

	// Vector unit
	float unit=2*_ShapeRadius;

	// Make a scale matrix
	CMatrix lumelScale;
	lumelScale.identity ();
	lumelScale.scale (unit);

	// Get the ray basis
	CMatrix lumelBasis=_RayBasis*lumelScale;

	// Change origine in the top left corner
	lumelBasis.setPos (position-lumelBasis.getI()/2-lumelBasis.getJ()/2);

	// Inverse this matrix
	lumelBasis.invert ();

#endif // HARDWARE_SOFT_SHADOWS

	// For each triangle selected
	CQuadGrid<const CTriangle*>::CIterator it=_QuadGrid[cpu].begin();
	while (it!=_QuadGrid[cpu].end())
	{
		// Source vector
		CVector source=position;

		// Same triangle ?
		if (
			((*it)->PatchId==patchId)&&
			((*it)->ZoneId==_ZoneToLight)&&
			((*it)->StartS<=s)&&
			((*it)->StartT<=t)&&
			((*it)->EndS>=s)&&
			((*it)->EndT>=t)
			)
			source+=(normal*_ShadowBias);

		// Blur ?
		if (!_Softshadow)
		{
			// Hit position
			CVector hit;

			// Intersect
			if ((*it)->Triangle.intersect (source, lightPos, hit, (*it)->Plane))
			{
				// Clear the shape list
				shape.Shapes.resize (0);
				break;
			}
		}
		else
		{
			// Triangle clippable ?
			const NLMISC::CTriangle &triangle=(*it)->Triangle;

			// Clip the ray over the triangle
			float edgeFactor[3]=
			{
				((((triangle.V0+triangle.V1)/2) - source)*-_LightDirection)/_FallofDistance,
				((((triangle.V1+triangle.V2)/2) - source)*-_LightDirection)/_FallofDistance,
				((((triangle.V2+triangle.V0)/2) - source)*-_LightDirection)/_FallofDistance,
			};
			float oOEdgeFactor[3];
			bool scaleEdge[3];
			uint edgeFlags[3];
			bool oneNotBack=false;
			uint i;
			for (i=0; i<3; i++)
			{
				// Edge factor
				if (edgeFactor[i]<0)
					// Polygon behing
					break;
				if (edgeFactor[i]>1)
				{
					scaleEdge[i]=false;
					edgeFactor[i]=1;
				}
				else
				{
					scaleEdge[i]=true;
					oOEdgeFactor[i]=1/edgeFactor[i];
				}

				// Distance from clipping plane
				float distance=(*it)->ClippingPlanes[i]*source;

				// Clipping distance
				float clipDist=edgeFactor[i]*_ShapeRadius;

				// Clip this distance
				if (distance<-clipDist)
				{
					// Back
					edgeFlags[i]=AllBack;
				}
				else if (distance>clipDist)
					// Front
					break;
				else
				{
					// Clipped
					edgeFlags[i]=Clipped;
					oneNotBack=true;
				}
			}

			// Not front clipped
			if (i==3)
			{
#ifdef HARDWARE_SOFT_SHADOWS
				// Transform this triangle in lumel basis
				CVector v[3] = { lumelBasis*triangle.V0, lumelBasis*triangle.V1, lumelBasis*triangle.V2 };

				// Draw the triangle
				drv->drawTriangle (v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, CRGBA(0, 0, 0, 2));
				drv->drawTriangle (v[0].x, v[0].y, v[2].x, v[2].y, v[1].x, v[1].y, CRGBA(0, 0, 0, 2));

#else // HARDWARE_SOFT_SHADOWS
				// All back ?
				if (oneNotBack)
				{
					uint backupSize=shape.Shapes.size();
					for (uint s=0; s<backupSize; s++)
					{
						// Reset out list
						shapeTmp.Shapes.resize (0);
						back = shape.Shapes[s];

						// Clip this shape with the triangle (3 planes)
						for (i=0; i<3; i++)
						{
							// All back ?
							if (edgeFlags[i]==AllBack)
								// Yes, next
								continue;

							// Security
							if (back.NumVertex > (SHAPE_VERTICES_MAX-10) )
								break;

							// Scale down this shape
							if (scaleEdge[i])
								back.scale (source, edgeFactor[i]);

							// Copy the back buffer
							copy=back;

							// Clipping plane
							const CPlane &clippingPlane=(*it)->ClippingPlanes[i];

							// Reset back and front
							back.NumVertex=0;
							front.NumVertex=0;

							// Clip
							if(copy.NumVertex>2)
							{
								// Previous vertex
								uint prev=copy.NumVertex-1;

								// Previous front ?
								bool previousFront=(clippingPlane*copy.Vertices[prev] >= 0);

								// For each vertex
								for (uint cur=0;cur<copy.NumVertex;cur++)
								{
									// Current vertex front ?
									bool currentFront=(clippingPlane*copy.Vertices[cur] >= 0);
									if ( currentFront )
									{
										// Previous vertex back ?
										if ( !previousFront )
										{
											// Ok, intersect
											front.Vertices[front.NumVertex]= clippingPlane.intersect(copy.Vertices[prev],copy.Vertices[cur]);
											back.Vertices[back.NumVertex++]= front.Vertices[front.NumVertex];
											front.NumVertex++;
										}
										// Store new vertex front
										front.Vertices[front.NumVertex++]=copy.Vertices[cur];
									}
									else
									{
										// Previous vertex front ?
										if ( previousFront )
										{
											front.Vertices[front.NumVertex]= clippingPlane.intersect(copy.Vertices[prev],copy.Vertices[cur]);
											back.Vertices[back.NumVertex++]= front.Vertices[front.NumVertex];
											front.NumVertex++;
										}
										back.Vertices[back.NumVertex++]=copy.Vertices[cur];
									}
									prev=cur;
									previousFront=currentFront;
								}
							}

							// Scale up this shape
							if (scaleEdge[i])
							{
								back.scale (source, oOEdgeFactor[i]);
								front.scale (source, oOEdgeFactor[i]);
							}

							// Some vertices front ?
							if (front.NumVertex!=0)
							{
								// Front vertices ?
								if (back.NumVertex==0)
									// Nothing else to clip
									break;
							}
							else
							{
								// All vertices are back
								// Pass entire triangle to next plane
								continue;
							}

							// Code is clipped
							// res is the front shape, so it is out
							// Last plane ?
							shapeTmp.Shapes.push_back (front);
						}
						if (i==3)
						{
							// Merge list..
							if (shapeTmp.Shapes.empty())
							{
								// Erase this entry
								shape.Shapes[s].NumVertex=0;
							}
							else
							{
								// Copy first element
								shape.Shapes[s]=shapeTmp.Shapes[0];

								// Insert others
								uint size=shapeTmp.Shapes.size();
								for (uint t=1; t<size; t++)
								{
									// Append new shapes
									shape.Shapes.push_back (shapeTmp.Shapes[t]);
								}
							}
						}
					}
				}
				else
				{
					// Clear all the ray
					shape.Shapes.resize (0);
				}
#endif // HARDWARE_SOFT_SHADOWS
			}
		}

		// Next
		it++;
	}
}

// ***************************************************************************

void CZoneLighter2002::rayTrace (const CVector& position, const CVector& normal, float s, float t, uint patchId, float &factor, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu)
{
	// Resize the shape list
	shape.Shapes.resize (1);

	// Ref on the cshape
	CShape &shp=shape.Shapes[0];

	// Copy the shape
	shp=_Shape;

	// Translate the shape
	sint j;
	for (j=0; j<(sint)shp.NumVertex; j++)
	{
		shp.Vertices[j]+=position;
	}

	// Build a clipping plane
	CPlane plane;
	plane.make (-_LightDirection, position);

#ifdef HARDWARE_SOFT_SHADOWS

	// Clear all pixels in green
	drv->clearRGBABuffer (CRGBA (0, 255, 0, 0));

#endif // HARDWARE_SOFT_SHADOWS

	// Go!
	testRaytrace (position, normal, plane, s, t, patchId, shape, shapeTmp, cpu);

#ifdef HARDWARE_SOFT_SHADOWS
	
	// Download frame buffer
	static CBitmap bitmap;
	drv->getBufferPart (bitmap, CRect (0, 0, LIGHT_BUFFER_SIZE, LIGHT_BUFFER_SIZE));
	nlassert (bitmap.getWidth()==LIGHT_BUFFER_SIZE);
	nlassert (bitmap.getHeight()==LIGHT_BUFFER_SIZE);

	// Pixels
	bitmap.convertToType (CBitmap::RGBA);

	// RGBA pointer
	CRGBA *pixels=(CRGBA*)&bitmap.getPixels ()[0];

	// Average pixel
	factor=0;
	for (uint p=0; p<LIGHT_BUFFER_SIZE*LIGHT_BUFFER_SIZE; p++)
	{
		factor+=pixels[p].G;
	}
	factor/=(float)(255*LIGHT_BUFFER_SIZE*LIGHT_BUFFER_SIZE);

#else // HARDWARE_SOFT_SHADOWS
	// Calc the surface ratio
	uint size=shape.Shapes.size();
	for (uint i=0; i<size; i++)
	{
		// For each shape
		CShape &vect=shape.Shapes[i];

		for (j=1; j<(sint)vect.NumVertex-1; j++)
		{
			// Sum the area
			factor+=((vect.Vertices[j]-vect.Vertices[0])^(vect.Vertices[j+1]-vect.Vertices[0])).norm();
		}
	}

	factor/=_ShapeArea;
#endif // HARDWARE_SOFT_SHADOWS
}

// ***************************************************************************

void CZoneLighter2002::rayTraceTriangle (const NLMISC::CTriangle& toOverSample, CVector& normal, uint order, float s, float t, float &factor, uint &tested, uint patch)
{
	// Ok ?
	if (order==0)
	{
		// Ray !
		tested++;
		//rayTrace (-_LightDirection+(toOverSample.V0+toOverSample.V1+toOverSample.V2)/3, normal, s, t, patch, factor);
	}
	else
	{
		// Subdivide the triangle
		CVector v0V1=toOverSample.V0;
		v0V1+=toOverSample.V1;
		v0V1/=2;
		CVector v0V2=toOverSample.V0;
		v0V2+=toOverSample.V2;
		v0V2/=2;
		CVector v1V2=toOverSample.V1;
		v1V2+=toOverSample.V2;
		v1V2/=2;
		rayTraceTriangle (NLMISC::CTriangle (toOverSample.V0, v0V1, v0V2), normal, order-1, s, t, factor, tested, patch);
		rayTraceTriangle (NLMISC::CTriangle (toOverSample.V1, v1V2, v0V1), normal, order-1, s, t, factor, tested, patch);
		rayTraceTriangle (NLMISC::CTriangle (toOverSample.V2, v0V2, v1V2), normal, order-1, s, t, factor, tested, patch);
		rayTraceTriangle (NLMISC::CTriangle (v0V1, v1V2, v0V2), normal, order-1, s, t, factor, tested, patch);
	}
}

// ***************************************************************************

bool CZoneLighter2002::isLumelOnEdgeMustBeOversample (uint patch, uint edge, sint s, sint t, const vector<bool> &binded, 
												  const vector<bool> &oversampleEdges, vector<CPatchUVLocator> &locator, 
												  uint8 shadowed, vector<vector<uint8> >& shadowBuffer)
{
	// Must force oversampling of this edge ?
	if (oversampleEdges[edge])
		return true;
	else
	{
		// binded ?
		if (binded[edge])
		{
			// Lumel coord
			CVector2f lumelCoord (((float)(s+_GetNormalDeltaS[edge])+0.5f)/4.f, ((float)(t+_GetNormalDeltaT[edge])+0.5f)/4.f);
			uint otherPatch=locator[edge].selectPatch(lumelCoord);

			// Get uv
			CVector2f neighborUV;
			CPatch *patchOut;
			locator[edge].locateUV (lumelCoord, otherPatch, patchOut, neighborUV);
			
			// Is the same shadowed flag ?
			sint ss=(sint)(neighborUV.x*4.f);
			sint tt=(sint)(neighborUV.y*4.f);
			// Era UB guard: locateUV can land on a patch of a NEIGHBOR zone (or UV==1.0
			// overflow) while shadowBuffer only covers the zone to light — the 2002 build
			// read out of bounds here at zone borders. Treat those as "no oversample"
			// (borders are weld-order territory anyway).
			if (!patchOut || patchOut->getZone()->getZoneId()!=(sint)_ZoneToLight)
				return false;
			uint neighborId=patchOut->getPatchId();
			uint neighborIndex=(uint)ss+((uint)patchOut->getOrderS()<<2)*(uint)tt;
			if (neighborId>=shadowBuffer.size() || neighborIndex>=shadowBuffer[neighborId].size())
				return false;
			return (shadowBuffer[neighborId][neighborIndex]!=shadowed);
		}
		else
		{
			// Not oversample if not binded
			return false;
		}
	}
}

// ***************************************************************************

float easineasout(float x)
{
 float y;
 // cubic tq f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
 float x2=x*x;
 float x3=x2*x;
 y= -2*x3 + 3*x2;
 return y;
}

// ***************************************************************************

float easineasoutC2(float x)
{
 float y;
 // 5-nome tq f(0)=0, f'(0)=0, f''(0)=0, f(1)=1, f'(1)=0, f''(1)=0.
 float x3=x*x*x;
 float x4=x3*x;
 float x5=x4*x;
 y= 6*x5 -15*x4 +10*x3;
 return y;
}

// ***************************************************************************


sint16 CZoneLighter2002::_GetNormalDeltaS[4]={ -1, 0, 1, 0 };
sint16 CZoneLighter2002::_GetNormalDeltaT[4]={ 0, 1, 0, -1 };

// ***************************************************************************

void CZoneLighter2002::getNormal (const CPatch *pPatch, sint16 lumelS, sint16 lumelT, vector<CPatchUVLocator> &locator, 
								 const vector<CPatch::CBindInfo> &bindInfo, const vector<bool> &binded, set<uint64>& visited, 
								 float deltaS, float deltaT, uint rotation, const CBezierPatch &bezierPatch, uint lastEdge)
{
	// Build a desc srructure
	uint64 id=(uint64)lumelS|(((uint64)lumelT)<<16)|(((uint64)pPatch->getPatchId())<<32)|(((uint64)pPatch->getZone()->getZoneId())<<48);

	// Insert it
	if (visited.insert (id).second)
	{
		// Clip
		float sqDist=deltaS*deltaS+deltaT*deltaT;
		if ( sqDist < 1 )
		{
			// Continue...

			sint orderSx4=pPatch->getOrderS()<<2;
			sint orderTx4=pPatch->getOrderT()<<2;

			sint16 _GetNormalBorderS[4]={ 0, -10, 1, -10 };
			sint16 _GetNormalBorderT[4]={ -10, 1, -10, 0 };
			_GetNormalBorderS[2]=orderSx4-1;
			_GetNormalBorderT[1]=orderTx4-1;

			// Add normal
			_GetNormalNormal+=bezierPatch.evalNormal ( ((float)lumelS+0.5f)/(float)orderSx4, ((float)lumelT+0.5f)/(float)orderTx4 );

			// For the four neighbors
			for (uint edge=0; edge<4; edge++)
			{
				// Not last edge ?
				if (edge!=lastEdge)
				{
					// Direction
					uint globalDirection=(edge+(4-rotation))&0x3;

					// Neighbor
					if ( (lumelS==_GetNormalBorderS[edge]) || (lumelT==_GetNormalBorderT[edge]) )
					{
						// Binded ?
						bool bind=binded[edge];
						bool smooth=pPatch->getSmoothFlag (edge);
						if (bind&&smooth)
						{
							// Lumel coord
							CVector2f lumelCoord ( ((float)(lumelS+_GetNormalDeltaS[edge])+0.5f)/4, 
								((float)(lumelT+_GetNormalDeltaT[edge])+0.5f)/4 );

							// Get neighbor pixel
							uint otherPatch=locator[edge].selectPatch(lumelCoord);

							// Get uv
							CVector2f neighborUV;
							CPatch *patchOut;
							locator[edge].locateUV (lumelCoord, otherPatch, patchOut, neighborUV);

							// New coordinates
							sint16 newLumelS=(sint16)(4.f*neighborUV.x);
							sint16 newLumelT=(sint16)(4.f*neighborUV.y);

							// Zone id
							uint16 patchId=patchOut->getPatchId();
							uint16 zoneId=_ZoneId[patchOut->getZone()->getZoneId ()];

							// Get edge
							uint newEdge=0;
							uint i;
							for (i=0; i<=(uint)bindInfo[edge].NPatchs; i++)
							{
								// Good patch ?
								if (bindInfo[edge].Next[i]==patchOut)
								{
									// Get its edge
									newEdge=bindInfo[edge].Edge[i];
									break;
								}
							}
							
							// Rotation 
							uint newRotation=(2-edge+rotation+newEdge)&0x3;

							// Must found it
							nlassert (i!=(uint)bindInfo[edge].NPatchs);

							// Get the bezier patch
							CBezierPatch &NewBezierPatch=_BezierPatch[zoneId][patchId];

							// Next lumel
							getNormal (patchOut, newLumelS, newLumelT, _Locator[zoneId][patchId], _BindInfo[zoneId][patchId], 
								_Binded[zoneId][patchId], visited, deltaS+_GetNormalDeltaS[globalDirection], 
								deltaT+_GetNormalDeltaT[globalDirection], newRotation, NewBezierPatch, newEdge);
						}
					}
					else
					{
						// Left internal
						getNormal (pPatch, lumelS+_GetNormalDeltaS[edge], lumelT+_GetNormalDeltaT[edge], locator, bindInfo, binded, visited, 
							deltaS+_GetNormalDeltaS[globalDirection], deltaT+_GetNormalDeltaT[globalDirection], rotation, bezierPatch, (edge+2)&0x3);
					}
				}
			}
		}
	}
}

// ***************************************************************************

void CZoneLighter2002::addTriangles (CLandscape &landscape, vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray)
{
	// Set all to refine
	excludeAllPatchFromRefineAll (landscape, listZone, false);

	// Setup the landscape
	landscape.setThreshold (0);
	landscape.setTileMaxSubdivision (order);

	// Refine it
	landscape.refineAll (CVector (0, 0, 0));

	// Dump tesselated triangles
	std::vector<const CTessFace*> leaves;
	landscape.getTessellationLeaves(leaves);

	// Number of leaves
	uint leavesCount=leaves.size();

	// Reserve the array
	triangleArray.reserve (triangleArray.size()+leavesCount);

	// Scan each leaves
	for (uint leave=0; leave<leavesCount; leave++)
	{
		// Leave
		const CTessFace *face=leaves[leave];

		// Start and end coordinate
		float startS=min (min (face->PVBase.getS(), face->PVLeft.getS()), face->PVRight.getS());
		float endS=max (max (face->PVBase.getS(), face->PVLeft.getS()), face->PVRight.getS());
		float startT=min (min (face->PVBase.getT(), face->PVLeft.getT()), face->PVRight.getT());
		float endT=max (max (face->PVBase.getT(), face->PVLeft.getT()), face->PVRight.getT());

		// Add a triangle
		triangleArray.push_back (CTriangle (NLMISC::CTriangle (face->VBase->EndPos, face->VLeft->EndPos, face->VRight->EndPos), 
			face->Patch->getZone()->getZoneId(), face->Patch->getPatchId(), startS ,endS, startT, endT));
	}

	// Setup the landscape
	landscape.setThreshold (1000);
	landscape.setTileMaxSubdivision (0);

	// Remove all triangles
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
	landscape.refineAll (CVector (0, 0, 0));
}

// ***************************************************************************

void CZoneLighter2002::addTriangles (const IShape &shape, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
{
	// Cast to CMesh
	const CMesh *mesh=dynamic_cast<const CMesh*>(&shape);

	// Cast to CMeshMultiLod
	const CMeshMultiLod *meshMulti=dynamic_cast<const CMeshMultiLod*>(&shape);

	// Cast to CMeshMultiLod
	const CMeshMRM *meshMRM=dynamic_cast<const CMeshMRM*>(&shape);

	// It is a mesh ?
	if (mesh)
	{
		// Add its triangles
		addTriangles (mesh->getMeshGeom (), modelMT, triangleArray);
	}
	// It is a CMeshMultiLod ?
	else if (meshMulti)
	{
		// Get the first geommesh
		const IMeshGeom *meshGeom=&meshMulti->getMeshGeom (0);

		// Dynamic cast
		const CMeshGeom *geomMesh=dynamic_cast<const CMeshGeom*>(meshGeom);
		if (geomMesh)
		{
			addTriangles (*geomMesh, modelMT, triangleArray);
		}

		// Dynamic cast
		const CMeshMRMGeom *mrmGeomMesh=dynamic_cast<const CMeshMRMGeom*>(meshGeom);
		if (mrmGeomMesh)
		{
			addTriangles (*mrmGeomMesh, modelMT, triangleArray);
		}
	}
	// It is a CMeshMultiLod ?
	else if (meshMRM)
	{
		// Get the first lod mesh geom
		addTriangles (meshMRM->getMeshGeom (), modelMT, triangleArray);
	}
}

// ***************************************************************************

void CZoneLighter2002::addTriangles (const CMeshGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
{
	// Get the vertex buffer (modern accessor; era code read raw pointers — same positions)
	const CVertexBuffer &vb=meshGeom.getVertexBuffer();
	CVertexBufferRead vba;
	vb.lock (vba);

	// For each matrix block
	uint numBlock=meshGeom.getNbMatrixBlock();
	for (uint block=0; block<numBlock; block++)
	{
		// For each render pass
		uint numRenderPass=meshGeom.getNbRdrPass(block);
		for (uint pass=0; pass<numRenderPass; pass++)
		{
			// Get the primitive block
			const CIndexBuffer &primitive=meshGeom.getRdrPassPrimitiveBlock ( block, pass);
			CIndexBufferRead iba;
			primitive.lock (iba);

			// Dump triangles
			uint numTri=primitive.getNumIndexes ()/3;
			for (uint tri=0; tri<numTri; tri++)
			{
				uint32 i0, i1, i2;
				if (iba.getFormat() == CIndexBuffer::Indices32)
				{
					const uint32 *triIndex= (const uint32 *) iba.getPtr ();
					i0=triIndex[tri*3]; i1=triIndex[tri*3+1]; i2=triIndex[tri*3+2];
				}
				else
				{
					const uint16 *triIndex= (const uint16 *) iba.getPtr ();
					i0=triIndex[tri*3]; i1=triIndex[tri*3+1]; i2=triIndex[tri*3+2];
				}
				CVector v0=modelMT*(*vba.getVertexCoordPointer (i0));
				CVector v1=modelMT*(*vba.getVertexCoordPointer (i1));
				CVector v2=modelMT*(*vba.getVertexCoordPointer (i2));
				triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2)));
			}
		}
	}
}

// ***************************************************************************

void CZoneLighter2002::addTriangles (const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
{
	// Get the vertex buffer (modern accessor)
	const CVertexBuffer &vb=meshGeom.getVertexBuffer();
	CVertexBufferRead vba;
	vb.lock (vba);

	// For each render pass
	uint numRenderPass=meshGeom.getNbRdrPass(0);
	for (uint pass=0; pass<numRenderPass; pass++)
	{
		// Get the primitive block
		const CIndexBuffer &primitive=meshGeom.getRdrPassPrimitiveBlock ( 0, pass);
		CIndexBufferRead iba;
		primitive.lock (iba);

		// Dump triangles
		uint numTri=primitive.getNumIndexes ()/3;
		for (uint tri=0; tri<numTri; tri++)
		{
			uint32 i0, i1, i2;
			if (iba.getFormat() == CIndexBuffer::Indices32)
			{
				const uint32 *triIndex= (const uint32 *) iba.getPtr ();
				i0=triIndex[tri*3]; i1=triIndex[tri*3+1]; i2=triIndex[tri*3+2];
			}
			else
			{
				const uint16 *triIndex= (const uint16 *) iba.getPtr ();
				i0=triIndex[tri*3]; i1=triIndex[tri*3+1]; i2=triIndex[tri*3+2];
			}
			CVector v0=modelMT*(*vba.getVertexCoordPointer (i0));
			CVector v1=modelMT*(*vba.getVertexCoordPointer (i1));
			CVector v2=modelMT*(*vba.getVertexCoordPointer (i2));
			triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2)));
		}
	}
}

// ***************************************************************************

void CZoneLighter2002::excludeAllPatchFromRefineAll (CLandscape &landscape, vector<uint> &listZone, bool exclude)
{
	// For each zone
	for (uint zone=0; zone<listZone.size(); zone++)
	{
		// Get num patches
		uint patchCount=landscape.getZone(listZone[zone])->getNumPatchs();

		// For each patches
		for (uint patch=0; patch<patchCount; patch++)
		{
			// Exclude all the patches from refine all
			landscape.excludePatchFromRefineAll (listZone[zone], patch, exclude);
		}
	}
}

// ***************************************************************************

void CZoneLighter2002::buildZoneInformation (CLandscape &landscape, const vector<uint> &listZone, bool oversampling, const CLightDesc &lightDesc)
{
	// Bool visit
	vector<vector<uint> > visited;

	// Zone count
	uint zoneCount=listZone.size();

	// Resize arries
	_Locator.resize (zoneCount);
	_Binded.resize (zoneCount);
	_BindInfo.resize (zoneCount);
	_BezierPatch.resize (zoneCount);

	// For each zone
	for (uint zone=0; zone<zoneCount; zone++)
	{
		// Get num patches
		uint patchCount=landscape.getZone(listZone[zone])->getNumPatchs();

		// Insert zone id
		_ZoneId.insert (map<uint, uint>::value_type (listZone[zone], zone));

		// This is the zone to light ?
		if (listZone[zone]==_ZoneToLight)
		{
			// Resize the arraies
			_Lumels.resize(patchCount);
//			_BezierPatch.resize(patchCount);
			_OversampleEdges.resize(patchCount);
			visited.resize(patchCount);
		}

		// Common arries
		_Locator[zone].resize(patchCount);
		_Binded[zone].resize(patchCount);
		_BindInfo[zone].resize(patchCount);
		_BezierPatch[zone].resize(patchCount);

		// For each patch
		uint patch;
		for (patch=0; patch<patchCount; patch++)
		{
			// Get a patch pointer
			const CPatch* pPatch=(const_cast<const CZone*>(landscape.getZone(listZone[zone])))->getPatch (patch);

			// Progress bar
			progress ("Scan all patches", (float)patch/(float)patchCount);

			// Get pointer on arries
			vector<bool> &binded=_Binded[zone][patch];
			vector<CPatch::CBindInfo> &bindInfo=_BindInfo[zone][patch];
			vector<CPatchUVLocator> &locator=_Locator[zone][patch];
			CBezierPatch &bezierPatch=_BezierPatch[zone][patch];
			binded.resize (4, false);
			bindInfo.resize (4);
			locator.resize (4);

			// Contruct the patch
			bezierPatch=*pPatch->unpackIntoCache();

			// Same zone ?
			if (listZone[zone]==_ZoneToLight)
			{
				// oversample this edge
				_OversampleEdges[patch].resize (4, false);
			}

			// *** Build bind info

			// *** Build neighboorhood information
			uint edge;
			for (edge=0; edge<4; edge++)
			{
				// Bond neighbor
				pPatch->getBindNeighbor (edge, bindInfo[edge]);

				// Patch binded
				if (bindInfo[edge].NPatchs>0)
				{
					// This edeg is binded
					binded[edge]=true;

					// Same zone ?
					if ((listZone[zone]==_ZoneToLight)&&(bindInfo[edge].Zone->getZoneId()!=_ZoneToLight))
					{
						// oversample this edge
						_OversampleEdges[patch][edge]=true;
					}
					locator[edge].build (pPatch, edge, bindInfo[edge]);
				}
				else
				{
					if (listZone[zone]==_ZoneToLight)
					{
						// oversample this edge
						_OversampleEdges[patch][edge]=true;
					}
				}
			}

			// This is the zone to light ?
			if (listZone[zone]==_ZoneToLight)
			{
				// *** Resize lumel array for this patch

				// Get patch order
				uint orderS=pPatch->getOrderS();
				uint orderT=pPatch->getOrderT();

				// Number of lumels
				uint lumelCount=orderS*orderT*16;

				// Resize the lumel descriptor
				CLumelDescriptor descriptor;
				descriptor.Normal.set (0,0,0);
				descriptor.Position.set (0,0,0);
				descriptor.S=0;
				descriptor.T=0;
				_Lumels[patch].resize (lumelCount, descriptor);
				visited[patch].resize (lumelCount, 0);

				// *** Unexclude this patch

				// Exclude all the patches from refine all
				landscape.excludePatchFromRefineAll (listZone[zone], patch, false);
			}
			else
			{
				// Exclude all the patches from refine all
				landscape.excludePatchFromRefineAll (listZone[zone], patch, true);
			}
		}
	}

	// *** Now tesselate this zone to shadow accuracy

	// Setup the landscape
	landscape.setThreshold (0);
	landscape.setTileMaxSubdivision (0);

	// Refine all
	progress ("Refine landscape to shadow accuracy", 0.5f);
	landscape.refineAll (CVector (0, 0, 0));

	// Get tesselated faces
	std::vector<const CTessFace*> leaves;
	landscape.getTessellationLeaves(leaves);
	

	
	
	if (_WaterShapes.size() != 0) // any water shape in this zone ?
	{
		/// make a quad grid of each water shape				
		makeQuadGridFromWaterShapes(landscape.getZone(_ZoneToLight)->getZoneBB().getAABBox());

		/// check for each tile if it is above / below water
		computeTileFlagsForPositionTowardWater(lightDesc, leaves);
	}
	else
	{
		setTileFlagsToDefault(leaves);
	}
	

	// Id of this zone in the array
	uint zoneNumber=_ZoneId[_ZoneToLight];

	// Scan each leaves
	uint leavesCount=leaves.size();
	uint leave;
	for (leave=0; leave<leavesCount; leave++)
	{
		// Progress bar
		if ( (leave&0xff) == 0)
			progress ("Precompute lumel position", (float)leave/(float)leavesCount);

		// Leave
		const CTessFace *face=leaves[leave];

		// Get zone id
		if (face->Patch->getZone()->getZoneId()==_ZoneToLight)
		{
			// Get a patch pointer
			const CPatch* pPatch=face->Patch;

			// Get order
			uint orderS=pPatch->getOrderS();
			uint orderT=pPatch->getOrderT();

			// *** Base Coordinates
			// era wrote pos[14] one past a pos[14] declaration (UB that era MSVC
			// absorbed in stack padding); pos[15] is the intended and fixed size,
			// same as the modern lighter.
			CVector pos[15];
			pos[0]=face->VBase->EndPos;		// p0
			pos[1]=face->VRight->EndPos;
			pos[2]=face->VLeft->EndPos;		// p2
			pos[3]=(pos[1]+pos[2])/2;
			pos[4]=(pos[0]+pos[1])/2;				// p4
			pos[5]=(pos[0]+pos[2])/2;
			pos[6]=(pos[0]+pos[3])/2;				// p6
			pos[7]=(pos[2]+pos[3])/2;
			pos[8]=(pos[1]+pos[3])/2;				// p8
			pos[9]=(pos[0]+pos[4])/2;
			pos[10]=(pos[1]+pos[4])/2;				// p10
			pos[11]=(pos[0]+pos[5])/2;
			pos[12]=(pos[2]+pos[5])/2;				// p12
			pos[13]=(pos[3]+pos[5])/2;
			pos[14]=(pos[3]+pos[4])/2;				// p14

			float s0=face->PVBase.getS();
			float s1=face->PVRight.getS();
			float s2=face->PVLeft.getS();
			float s3=(s1+s2)/2;
			float s4=(s0+s1)/2;
			float s5=(s0+s2)/2;
			float s6=(s4+s5)/2;
			float s7=(s2+s3)/2;
			float s8=(s1+s3)/2;

			float t0=face->PVBase.getT();
			float t1=face->PVRight.getT();
			float t2=face->PVLeft.getT();
			float t3=(t1+t2)/2;
			float t4=(t0+t1)/2;
			float t5=(t0+t2)/2;
			float t6=(t4+t5)/2;
			float t7=(t2+t3)/2;
			float t8=(t1+t3)/2;

			// *** Interpolated value
			CVector interpolatedP[10]=
			{
				(pos[0]+pos[6])/2,
				(pos[4]+pos[6])/2,
				(pos[4]+pos[8])/2,
				(pos[1]+pos[8])/2,
				(pos[5]+pos[6])/2,
				(pos[3]+pos[6])/2,
				(pos[3]+pos[8])/2,
				(pos[5]+pos[7])/2,
				(pos[3]+pos[7])/2,
				(pos[2]+pos[7])/2,
			};
			
			float interpolatedS[10]=
			{
				(s0+s6)/2,
				(s4+s6)/2,
				(s4+s8)/2,
				(s1+s8)/2,
				(s5+s6)/2,
				(s3+s6)/2,
				(s3+s8)/2,
				(s5+s7)/2,
				(s3+s7)/2,
				(s2+s7)/2,
			};
			
			float interpolatedT[10]=
			{
				(t0+t6)/2,
				(t4+t6)/2,
				(t4+t8)/2,
				(t1+t8)/2,
				(t5+t6)/2,
				(t3+t6)/2,
				(t3+t8)/2,
				(t5+t7)/2,
				(t3+t7)/2,
				(t2+t7)/2,
			};

			static sint8 triangle[10][2][3]=
			{
				{{0, 11, 6}, {0, 6, 9}},
				{{9, 6, 4}, {4, 6, 14}},
				{{4, 14, 8}, {4, 8, 10}},
				{{10, 8, 1}, {-1, -1, -1}},
				{{11, 5, 6}, {5, 13, 6}},
				{{6, 13, 3}, {6, 3, 14}},
				{{3, 8, 14}, {-1, -1, -1}},
				{{5, 12, 7}, {5, 7, 13}},
				{{7, 3, 13}, {-1, -1, -1}},
				{{12, 2, 7}, {-1, -1, -1}}
			};

			for (uint i=0; i<10; i++)
			{
				uint s=(uint)((float)orderS*4*interpolatedS[i]);
				uint t=(uint)((float)orderT*4*interpolatedT[i]);

				/*nlassert (s>=0);
				nlassert (s<orderS*4);
				nlassert (t>=0);
				nlassert (t<orderT*4);*/

				if ((s>=0)&&(s<orderS*4)&&(t>=0)&&(t<orderT*4))
				{
					// Triangle index
					uint index=s+t*orderS*4;

					// Ge tthe patch id
					uint patchId=pPatch->getPatchId();

					// Get lumel array
					vector<CLumelDescriptor> &lumels=_Lumels[patchId];

					// Visited
					visited[patchId][index]++;

					// Position
					lumels[index].Position+=interpolatedP[i];

					// Triangle
					if (oversampling)
					{
						// Triangle list
						CTriangleList *next=lumels[index].TriangleList;

						// What triangle ?
						uint numTriangle;
						switch (i)
						{
						case 3:
						case 6:
						case 8:
						case 9:
							// Single triangle
							numTriangle=1;
							break;
						default:
							// Two triangles
							numTriangle=2;
							break;
						}

						// Add triangles
						for (uint tri=0; tri<numTriangle; tri++)
						{
							// one triangle
							lumels[index].TriangleList=_TriangleListAllocateur.allocate ();
							lumels[index].TriangleList->Triangle=NLMISC::CTriangle (pos[triangle[i][tri][0]], pos[triangle[i][tri][1]], pos[triangle[i][tri][2]]);
							lumels[index].TriangleList->Next=next;
						}
					}
				}
			}
		}
	}

	// *** Now, finalise patch informations for shadow source positions

	// For each patches
	uint patchCount=landscape.getZone(_ZoneToLight)->getNumPatchs();
	uint patch;
	for (patch=0; patch<patchCount; patch++)
	{
		// Info
		progress ("Finalize lumel positions", (float)patch/(float)patchCount);

		// *** Resize lumel array for this patch

		// Get a patch pointer
		const CPatch* pPatch=(const_cast<const CZone*>(landscape.getZone(_ZoneToLight)))->getPatch (patch);
		uint orderS=pPatch->getOrderS();
		uint orderT=pPatch->getOrderT();

		// Get lumel array
		vector<CLumelDescriptor> &lumels=_Lumels[patch];

		// *** Compute an interpolated normal

		// Renormalize
		nlassert (isPowerOf2 (orderS));
		nlassert (isPowerOf2 (orderT));
		uint powerS=getPowerOf2 (orderS);
		uint powerT=getPowerOf2 (orderT);
		uint lumelS=4<<powerS;
		uint lumelT=4<<powerT;

		for (uint t=0; t<lumelT; t++)
		for (uint s=0; s<lumelS; s++)
		{
			// Lumel index
			uint lumelIndex=s+t*lumelS;

			// *** Number of visit
			uint visitedCount=visited[patch][lumelIndex];
			
			// Some lumel have not been found in tesselation
			//nlassert ((visitedCount==1)||(visitedCount==2));

			// If visited, renormalise other values
			if (visitedCount)
			{
				// Normalise position
				lumels[lumelIndex].Position/=(float)visitedCount;
			}

			// Not visited for next pass
			visited[patch][lumelIndex]=false;
		}
	}

	// *** Now tesselate this zone to shadow accuracy

	// Setup the landscape
	landscape.setThreshold (0);
	landscape.setTileMaxSubdivision (4);

	// Refine all
	progress ("Refine landscape to lumels", 0.5f);
	landscape.refineAll (CVector (0, 0, 0));

	// Get tesselated faces
	leaves.clear ();
	landscape.getTessellationLeaves(leaves);

	// Scan each leaves
	leavesCount=leaves.size();
	for (leave=0; leave<leavesCount; leave++)
	{
		// Progress bar
		if ( (leave&0xff) == 0)
			progress ("Precompute tesselation", (float)leave/(float)leavesCount);

		// Leave
		const CTessFace *face=leaves[leave];

		// Get zone id
		if (face->Patch->getZone()->getZoneId()==_ZoneToLight)
		{
			// Get a patch pointer
			const CPatch* pPatch=face->Patch;

			// Get order
			uint orderS=pPatch->getOrderS();
			uint orderT=pPatch->getOrderT();

			// Coordinates
			float fS=(face->PVBase.getS()+face->PVLeft.getS()+face->PVRight.getS())/3.f;
			float fT=(face->PVBase.getT()+face->PVLeft.getT()+face->PVRight.getT())/3.f;
			uint s=(uint)((float)orderS*4*fS);
			uint t=(uint)((float)orderT*4*fT);
			nlassert (s>=0);
			nlassert (s<orderS*4);
			nlassert (t>=0);
			nlassert (t<orderT*4);

			// Triangle index
			uint index=s+t*orderS*4;

			// Ge tthe patch id
			uint patchId=pPatch->getPatchId();

			// Get lumel array
			vector<CLumelDescriptor> &lumels=_Lumels[patchId];

			// Visited
			visited[patchId][index]++;

			// Lumel s and t
			lumels[index].S+=fS;
			lumels[index].T+=fT;

			// Normal
			CPlane plane;
			plane.make (face->VBase->EndPos, face->VLeft->EndPos, face->VRight->EndPos);
			lumels[index].Normal+=plane.getNormal();
		}
	}

	// *** Now, finalise patch informations

	// For each patches
	patchCount=landscape.getZone(_ZoneToLight)->getNumPatchs();
	for (patch=0; patch<patchCount; patch++)
	{
		// Info
		progress ("Finalize patches", (float)patch/(float)patchCount);

		// *** Resize lumel array for this patch

		// Get a patch pointer
		const CPatch* pPatch=(const_cast<const CZone*>(landscape.getZone(_ZoneToLight)))->getPatch (patch);
		uint orderS=pPatch->getOrderS();
		uint orderT=pPatch->getOrderT();

		// Get lumel array
		vector<CLumelDescriptor> &lumels=_Lumels[patch];

		// *** Compute an interpolated normal

		// Get pointer on arries
		vector<bool> &binded=_Binded[zoneNumber][patch];
		vector<CPatchUVLocator> &locator=_Locator[zoneNumber][patch];
		vector<CPatch::CBindInfo> &bindInfo=_BindInfo[zoneNumber][patch];
		CBezierPatch &bezierPatch=_BezierPatch[zoneNumber][patch];

		// Renormalize
		nlassert (isPowerOf2 (orderS));
		nlassert (isPowerOf2 (orderT));
		uint powerS=getPowerOf2 (orderS);
		uint powerT=getPowerOf2 (orderT);
		uint lumelS=4<<powerS;
		uint lumelT=4<<powerT;

		// Sample edge normal
		CVector normals[NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1][4];
		uint sFixed[4] = { 0, 0xffffffff, lumelS-1, 0xffffffff };
		uint tFixed[4] = { 0xffffffff, lumelT-1, 0xffffffff, 0 };
		float sOri[4] = { 0, -1, (float)lumelS, -1 };
		float tOri[4] = { -1, (float)lumelT, -1, 0 };
		for (uint edge=0; edge<4; edge++)
		{
			// s and t
			uint count=(edge&1)?lumelS:lumelT;
			for (uint lumel=0; lumel<=count; lumel++)
			{
				// Start coordinates
				float origineS;
				float origineT;
				uint startS;
				uint startT;
				if (edge&1)
				{
					if (lumel==count)
						startS=count-1;
					else
						startS=lumel;
					startT=tFixed[edge];
					origineS=(float)lumel;
					origineT=tOri[edge];
				}
				else
				{
					if (lumel==count)
						startT=count-1;
					else
						startT=lumel;
					startS=sFixed[edge];
					origineT=(float)lumel;
					origineS=sOri[edge];
				}
				_GetNormalNormal=CVector::Null;
				set<uint64> visitedLumels;
				getNormal (pPatch, startS, startT, locator, bindInfo, binded, visitedLumels, 
					startS+0.5f-origineS, startT+0.5f-origineT, 0, bezierPatch);
				_GetNormalNormal.normalize ();
				normals[lumel][edge]=_GetNormalNormal;
			}

			// Smooth the corners
#define BLUR_SIZE 4
			for (uint i=1; i<BLUR_SIZE; i++)
			{
				float value=(float)i/BLUR_SIZE;
				value=easineasout(value);
				normals[i][edge]=normals[0][edge]*(1-value)+normals[i][edge]*value;
				normals[i][edge].normalize();
				normals[count-i][edge]=normals[count][edge]*(1-value)+normals[count-i][edge]*value;
				normals[count-i][edge].normalize();
			}
		}
		
		for (uint t=0; t<lumelT; t++)
		for (uint s=0; s<lumelS; s++)
		{
			// Lumel index
			uint lumelIndex=s+t*lumelS;

			// *** Calc the smoothed normal
			
			// For each edge
			CVector normalS=bezierPatch.evalNormal (((float)s+0.5f)/(float)lumelS, ((float)t+0.5f)/(float)lumelT);
			float sFactor=0;
			CVector normalT=normalS;
			float tFactor=0;
			bool sGood=false, tGood=false;
			if (s<BLUR_SIZE)
			{
				sGood=true;
				// Average the two normals
				CVector average=normals[t][0];
				average+=normals[t+1][0];
				average/=2;

				// Blend
				float value=s+0.5f;
				sFactor=BLUR_SIZE-value;
				value/=BLUR_SIZE;
				value=easineasout(value);
				normalS=(normalS*value+average*(1-value));
				normalS.normalize();
			}
			if (s>=lumelS-BLUR_SIZE)
			{
				sGood=true;
				// Average the two normals
				CVector average=normals[t][2];
				average+=normals[t+1][2];
				average/=2;

				// Blend
				float value=s+0.5f;
				sFactor=BLUR_SIZE-(lumelS-value);
				value=(lumelS-value)/BLUR_SIZE;
				value=easineasout(value);
				normalS=(normalS*value+average*(1-value));
				normalS.normalize();
			}
			if (t<BLUR_SIZE)
			{
				tGood=true;
				// Average the two normals
				CVector average=normals[s][3];
				average+=normals[s+1][3];
				average/=2;

				// Blend
				float value=t+0.5f;
				tFactor=BLUR_SIZE-value;
				value/=BLUR_SIZE;
				value=easineasout(value);
				normalT=(normalT*value+average*(1-value));
				normalT.normalize();
			}
			if (t>=lumelT-BLUR_SIZE)
			{
				tGood=true;
				// Average the two normals
				CVector average=normals[s][1];
				average+=normals[s+1][1];
				average/=2;

				// Blend
				float value=t+0.5f;
				tFactor=BLUR_SIZE-(lumelT-value);
				value=((lumelT)-value)/BLUR_SIZE;
				value=easineasout(value);
				normalT=(normalT*value+average*(1-value));
				normalT.normalize();
			}

			// The smooth normal
			CVector smoothNormal;

			if ((sGood)&&(tGood))
			{
				if ((sFactor!=BLUR_SIZE)||(tFactor!=BLUR_SIZE))
					smoothNormal=normalS*(BLUR_SIZE-tFactor)+normalT*(BLUR_SIZE-sFactor);
				else
					smoothNormal=normalS+normalT;
			}
			else if (sGood)
				smoothNormal=normalS;
			else
				smoothNormal=normalT;

			// Normalize it
			smoothNormal.normalize();

			// The pure normal
			CVector purNormal=bezierPatch.evalNormal (((float)s+0.5f)/(float)lumelS, ((float)t+0.5f)/(float)lumelT);

			// Normalize the noisy normal
			lumels[lumelIndex].Normal.normalize();

			// Final normal
			lumels[lumelIndex].Normal=lumels[lumelIndex].Normal-purNormal+smoothNormal;
			lumels[lumelIndex].Normal.normalize ();

			// *** Number of visit
			uint visitedCount=visited[patch][lumelIndex];
			
			// Some lumel have not been found in tesselation
			//nlassert (visitedCount==2);

			// If visited, renormalise other values
			if (visitedCount)
			{
				// Normalise position
				lumels[lumelIndex].S/=(float)visitedCount;
				lumels[lumelIndex].T/=(float)visitedCount;
			}
		}
	}
}

// ***************************************************************************

CZoneLighter2002::CLightDesc::CLightDesc ()
{
	LightDirection.set (1, 1, -1);
	GridSize=512;
	GridCellSize=4;
	HeightfieldSize=200;
	HeightfieldCellSize=20;
	SkyContribution=true;
	SkyIntensity=0.25;
	ShadowBias=0.5f;
	SoftshadowBlurSize=1.f;
	SoftshadowFallof=10.f;
	SoftshadowShapeVertexCount=4;
	Oversampling=OverSamplingx32;
}

// ***************************************************************************
void CZoneLighter2002::addLightableShape(IShape *shape, const NLMISC::CMatrix& MT)
{
	CShapeInfo lsi;
	lsi.MT = MT;
	lsi.Shape = shape;
	_LightableShapes.push_back(lsi);
}


// ***************************************************************************
bool CZoneLighter2002::isLightableShape(IShape &shape)
{
	/// for now, the only shape that we lit are water shapes
	if (dynamic_cast<CWaterShape *>(&shape) != NULL)
	{
		// check that this water surface has a diffuse map that is a CTextureFile (we must be able to save it !)
		CWaterShape *ws = static_cast<CWaterShape *>(&shape);
		const ITexture *tex = ws->getColorMap();
		if (dynamic_cast<const CTextureFile *>(tex) != NULL)
		{
			return ws->isLightMappingEnabled();
		}
	}
	return false;
}

// ***************************************************************************
void CZoneLighter2002::lightShapes(uint zoneID, const CLightDesc& description)
{
	/// compute light for the lightable shapes in the given zone
	if (_LightableShapes.size() == 0) return;	

	uint numShapePerThread = 1 + (_LightableShapes.size() / _ProcessCount);
	uint currShapeIndex = 0;
	uint process = 0;
	_ProcessExited = 0;

	_NumLightableShapesProcessed = 0;


	progress("Processing lightable shapes", 0);
	
	for (uint k = 0; k < _LightableShapes.size(); ++k, ++process)
	{
		uint lastShapeIndex = currShapeIndex + numShapePerThread;
		lastShapeIndex = std::min((uint)_LightableShapes.size(), lastShapeIndex);		
		IThread *pThread = IThread::create (new CCalcLightableShapeRunnable(process, this, &description, &_LightableShapes, currShapeIndex, lastShapeIndex));
		pThread->start();
		currShapeIndex = lastShapeIndex;
	}

	/// wait for other process
	while (_ProcessExited != _ProcessCount)
	{
		nlSleep (10);
	}

}



// ***************************************************************************

void CZoneLighter2002::processLightableShapeCalc (uint process,
											  TShapeVect *shapesToLit,
											  uint firstShape,
											  uint lastShape,
											  const CLightDesc& description)
{
	CMultiShape *shapeArray=new CMultiShape;
	CMultiShape *shapeArrayTmp=new CMultiShape;
	shapeArray->Shapes.reserve (SHAPE_MAX);
	shapeArrayTmp->Shapes.reserve (SHAPE_MAX);

	// for each lightable shape
	for (uint k = firstShape; k < lastShape; ++k)
	{		
		nlassert(isLightableShape(* (*shapesToLit)[k].Shape)); // make sure it is a lightable shape		
		lightSingleShape((*shapesToLit)[k], *shapeArray, *shapeArrayTmp, description, process);	
	}

	delete shapeArray;
	delete shapeArrayTmp;	
}


// ***************************************************************************
void CZoneLighter2002::lightSingleShape(CShapeInfo &si, CMultiShape &shape, CMultiShape &shapeTmp, const CLightDesc& description, uint cpu)
{
	/// we compute the lighting for one single shape
	if (dynamic_cast<CWaterShape *>(si.Shape))
	{
		lightWater(* static_cast<CWaterShape *>(si.Shape), si.MT, shape, shapeTmp, description, cpu);
		
	}
	++_NumLightableShapesProcessed;
	progress("Processing lightable shapes", (float) _NumLightableShapesProcessed / _LightableShapes.size());
	return;	
}



// ***************************************************************************
// utility function to get the directory of a fileName
static std::string getDir (const std::string& path)
{
	char tmpPath[512];
	strcpy (tmpPath, path.c_str());
	char* slash=strrchr (tmpPath, '/');
	if (!slash)
	{
		slash=strrchr (tmpPath, '\\');
	}

	if (!slash)
		return "";

	slash++;
	*slash=0;
	return tmpPath;
}


// ***************************************************************************
// utility function to get a file name fdrom a path
static std::string getName (const std::string& path)
{
	std::string dir=getDir (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *name=tmpPath;
	nlassert (dir.length()<=strlen(tmpPath));
	name+=dir.length();

	char* point=strrchr (name, '.');
	if (point)
		*point=0;

	return name;
}


// ***************************************************************************
// utility function to get the extension of a fileName
static std::string getExt (const std::string& path)
{
	std::string dir = getDir (path);
	std::string name = getName (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *ext=tmpPath;
	nlassert (dir.length()+name.length()<=strlen(tmpPath));
	ext+=dir.length()+name.length();

	return ext;
}


// ***************************************************************************
void CZoneLighter2002::lightWater(CWaterShape &ws, const CMatrix &MT, CMultiShape &shape, CMultiShape &shapeTmp, const CLightDesc& description, uint cpu)
{	
	try
	{	
		/// get the diffuse map
		CTextureFile *diffuseTex = NLMISC::safe_cast<CTextureFile *>(ws.getColorMap());
		std::string texFileName = CPath::lookup(diffuseTex->getFileName());
		diffuseTex->generate();
		const uint width = diffuseTex->getWidth();
		const uint height = diffuseTex->getHeight();	
		
		/// build a matrix to convert from water space to uv space
		NLMISC::CMatrix worldSpaceToUVs;
		NLMISC::CVector2f col0, col1, pos;
		ws.getColorMapMat(col0, col1, pos);
		worldSpaceToUVs.setRot(NLMISC::CVector(col0.x * width, col0.y * height, 0),
							   NLMISC::CVector(col1.x * width, col1.y * height, 0),
							   NLMISC::CVector::K);
		worldSpaceToUVs.setPos(NLMISC::CVector(pos.x * width, pos.y * height, 0));		

		/// get min and max uvs
		NLMISC::CPolygon p;
		ws.getShapeInWorldSpace(p);

		float minU, maxU;
		float minV, maxV;

		NLMISC::CVector uvs = worldSpaceToUVs * p.Vertices[0];
		minU = maxU = uvs.x;
		minV = maxV = uvs.y;


		for (uint k = 1; k < (uint) p.getNumVertices(); ++k)
		{
			uvs = worldSpaceToUVs * p.Vertices[k];
			minU = std::min(uvs.x, minU);
			minV = std::min(uvs.y, minV);
			maxU = std::max(uvs.x, maxU);
			maxV = std::max(uvs.y, maxV);	
		}
		
		
	

		sint iMinU = (sint) minU;
		sint iMaxU = (sint) maxU;
		sint iMinV = (sint) minV;
		sint iMaxV = (sint) maxV;

		NLMISC::clamp(iMinU, 0, (sint) width);
		NLMISC::clamp(iMaxU, 0, (sint) width);
		NLMISC::clamp(iMinV, 0, (sint) height);
		NLMISC::clamp(iMaxV, 0, (sint) height);

		// matrix to go from uv space to worldspace
		NLMISC::CMatrix UVSpaceToWorldSpace = worldSpaceToUVs.inverted();

		NLMISC::CObjectVector<uint8> &pixs8 = diffuseTex->getPixels();
		NLMISC::CRGBA *rgbPixs = (NLMISC::CRGBA *) &pixs8[0];

	
		/// raytrace each texel
		for (sint x = iMinU; x < iMaxU; ++x)
		{
			for (sint y = iMinV; y < iMaxV; ++y)
			{
				float factor;
				NLMISC::CVector pos = UVSpaceToWorldSpace * NLMISC::CVector( x + 0.5f, y + 0.5f, 0 ) 
					+ description.WaterShadowBias * NLMISC::CVector::K;
				if (description.Shadow)
				{
					rayTrace(pos, NLMISC::CVector::K, 0, 0, -1, factor, shape, shapeTmp, cpu);
				}
				else
				{
					factor = - NLMISC::CVector::K * description.LightDirection;
				}
				clamp(factor, 0.f, 1.f);
				factor = factor * description.WaterDiffuse + description.WaterAmbient;
				if (description.SkyContributionForWater)
				{
					factor += getSkyContribution(pos, NLMISC::CVector::K, description.SkyIntensity);
				}
				clamp(factor, 0.f, 1.f);
				uint intensity = (uint8) (255 * factor);
				NLMISC::CRGBA srcCol(intensity,
									 intensity,
									 intensity,
									  255);

				if (!description.ModulateWaterColor)
				{
					rgbPixs[x + y * width] = srcCol;
				}
				else
				{
					NLMISC::CRGBA &col = rgbPixs[x + y * width];
					col.modulateFromColor(col, srcCol);
				}
			}
		}
	
		/// now, save the result
		if (getExt(texFileName) != ".tga")
		{
			nlwarning("Zone lighter : error when lighting a water surface : input bitmap is not a tga file");
		}
		else
		{
			try
			{
				COFile of;
				of.open(texFileName);
				diffuseTex->writeTGA(of, 24);
				of.close();
			}
			catch (NLMISC::Exception &)
			{
				nlwarning("Zone lighter : while lighting a water shape, writing %s failed! ", texFileName.c_str());
			}
		}
	}
	catch(NLMISC::Exception &e)
	{
		nlwarning("Water shape lighting failed !");
		nlwarning(e.what());
	}
}

///***********************************************************
void CZoneLighter2002::addWaterShape(CWaterShape *shape, const NLMISC::CMatrix &MT)
{
	/// make sure it hasn't been inserted twice
	CShapeInfo ci;
	ci.Shape = shape;
	ci.MT = MT;
	_WaterShapes.push_back(ci);
}

///***********************************************************
void CZoneLighter2002::makeQuadGridFromWaterShapes(NLMISC::CAABBox zoneBBox)
{
	if (!_WaterShapes.size()) return;

	NLMISC::CAABBox tmpBox;

	/// the number of cells we want in the quad grid
	const uint numCells = 16;

	/// get the dimension
	float width  = zoneBBox.getMax().x - zoneBBox.getMin().x;
	float height = zoneBBox.getMax().y - zoneBBox.getMin().y;

	float dim = std::max(width, height);


	/// init the quad grid
	_WaterShapeQuadGrid.create(numCells, dim / numCells);
	

	uint count = 0, totalCount = _WaterShapes.size();

	/// now, insert all water shapes
	for (TShapeVect::iterator it = _WaterShapes.begin(); it != _WaterShapes.end(); ++it, ++count)
	{
		/// get the current shape bbox in the world
		it->Shape->getAABBox(tmpBox);
		NLMISC::CAABBox currBB = NLMISC::CAABBox::transformAABBox(it->MT, tmpBox);

		/// test if it intesect the zone bbox
		if (zoneBBox.intersect(currBB))
		{
			_WaterShapeQuadGrid.insert(currBB.getMin(), currBB.getMax(), NLMISC::safe_cast<CWaterShape *>(it->Shape));
		}
		progress("Building quadtree from water surfaces", (float) count / totalCount);
	}

	/// free the vector of water shapes
	NLMISC::contReset(_WaterShapes);
}


//==================================================================

/// a struct that helps us to know which tile we've processed
struct CTileOfPatch
{
	uint8		TileId;
	CPatch		*Patch;
	CTileOfPatch();
	CTileOfPatch(uint8 tileId, CPatch *patch) : TileId(tileId), Patch(patch)
	{		
	}	
};



// ***************************************************************************
// ***************************************************************************
// Static point lights.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CZoneLighter2002::CPointLightRT::CPointLightRT()
{
	RefCount= 0;
}


// ***************************************************************************
bool	CZoneLighter2002::CPointLightRT::testRaytrace(const CVector &v)
{
	CVector	dummy;

	if(!BSphere.include(v))
		return false;

	// If Ambient light, just skip
	if(PointLight.getType()== CPointLight::AmbientLight)
		return false;

	// If SpotLight verify in angle radius.
	if(PointLight.getType()== CPointLight::SpotLight)
	{
		float	att= PointLight.computeLinearAttenuation(v);
		if (att==0)
			return false;
	}

	// Select in the cubeGrid
	FaceCubeGrid.select(v);
	// For all faces selected
	while(!FaceCubeGrid.isEndSel())
	{
		const CTriangle	*tri= FaceCubeGrid.getSel();

		// If intersect, the point is occluded.
		if( tri->Triangle.intersect(BSphere.Center, v, dummy, tri->getPlane()) )
			return false;

		// next
		FaceCubeGrid.nextSel();
	}

	// Ok the point is visilbe from the light
	return true;
}


// ***************************************************************************
void			CZoneLighter2002::addStaticPointLight(const CPointLightNamed &pln)
{
	// build the plRT.
	CPointLightRT	plRT;
	plRT.PointLight= pln;
	// compute plRT.OODeltaAttenuation
	plRT.OODeltaAttenuation= pln.getAttenuationEnd() - pln.getAttenuationBegin();
	if(plRT.OODeltaAttenuation <=0 )
		plRT.OODeltaAttenuation= 0;
	else
		plRT.OODeltaAttenuation= 1.0f / plRT.OODeltaAttenuation;
	// compute plRT.BSphere
	plRT.BSphere.Center= pln.getPosition();
	plRT.BSphere.Radius= pln.getAttenuationEnd();
	// NB: FaceCubeGrid will be computed during light()

	// add the plRT
	_StaticPointLights.push_back(plRT);

}


// ***************************************************************************
void			CZoneLighter2002::compilePointLightRT(uint gridSize, float gridCellSize, std::vector<CTriangle>& obstacles, bool doShadow)
{
	uint	i;

	// Fill the quadGrid of Lights.
	// ===========
	_StaticPointLightQuadGrid.create(gridSize, gridCellSize);
	for(i=0; i<_StaticPointLights.size();i++)
	{
		CPointLightRT	&plRT= _StaticPointLights[i];

		// Compute the bbox of the light
		CAABBox		bbox;
		bbox.setCenter(plRT.BSphere.Center);
		float	hl= plRT.BSphere.Radius;
		bbox.setHalfSize(CVector(hl,hl,hl));

		// Insert the pointLight in the quadGrid.
		_StaticPointLightQuadGrid.insert(bbox.getMin(), bbox.getMax(), &plRT);
	}


	// Append triangles to cubeGrid ??
	if(doShadow)
	{
		// For all obstacles, Fill a quadGrid.
		// ===========
		CQuadGrid<CTriangle*>	obstacleGrid;
		obstacleGrid.create(gridSize, gridCellSize);
		uint	size= obstacles.size();
		for(i=0; i<size; i++)
		{
			// bbox of triangle
			CAABBox	bbox;
			bbox.setCenter(obstacles[i].Triangle.V0);
			bbox.extend(obstacles[i].Triangle.V1);
			bbox.extend(obstacles[i].Triangle.V2);
			// insert triangle in quadGrid.
			obstacleGrid.insert(bbox.getMin(), bbox.getMax(), &obstacles[i]);
		}


		// For all PointLights, fill his CubeGrid
		// ===========
		for(i=0; i<_StaticPointLights.size();i++)
		{
			// progress
			progress ("Compute Influences of PointLights", 0.5f*i / (float)(_StaticPointLights.size()-1));

			CPointLightRT	&plRT= _StaticPointLights[i];
			// Create the cubeGrid
			plRT.FaceCubeGrid.create(plRT.PointLight.getPosition(), NL3D_ZONE_LIGHTER_CUBE_GRID_SIZE);

			// AmbiantLIghts: do nothing.
			if(plRT.PointLight.getType()!=CPointLight::AmbientLight)
			{
				// Select only obstacle Faces around the light. Other are not usefull
				CAABBox	bbox;
				bbox.setCenter(plRT.PointLight.getPosition());
				float	hl= plRT.PointLight.getAttenuationEnd();
				bbox.setHalfSize(CVector(hl,hl,hl));
				obstacleGrid.select(bbox.getMin(), bbox.getMax());

				// For all faces, fill the cubeGrid.
				CQuadGrid<CTriangle*>::CIterator	itObstacle;
				itObstacle= obstacleGrid.begin();
				while( itObstacle!=obstacleGrid.end() )
				{
					CTriangle	&tri= *(*itObstacle);
					// Test BackFace culling. Only faces which are BackFace the point light are inserted.
					// This is to avoid AutoOccluding problems
					if( tri.getPlane() * plRT.BSphere.Center < 0)
					{
						// Insert the triangle in the CubeGrid
						plRT.FaceCubeGrid.insert( tri.Triangle, &tri);
					}

					itObstacle++;
				}
			}

			// Compile the CubeGrid.
			plRT.FaceCubeGrid.compile();

			// And Reset RefCount.
			plRT.RefCount= 0;
		}
	}
	// else, just build empty grid
	else
	{
		for(i=0; i<_StaticPointLights.size();i++)
		{
			// progress
			progress ("Compute Influences of PointLights", 0.5f*i / (float)(_StaticPointLights.size()-1));

			CPointLightRT	&plRT= _StaticPointLights[i];
			// Create a dummy empty cubeGrid => no rayTrace :)
			plRT.FaceCubeGrid.create(plRT.PointLight.getPosition(), 4);

			// Compile the CubeGrid.
			plRT.FaceCubeGrid.compile();

			// And Reset RefCount.
			plRT.RefCount= 0;
		}
	}

}


// ***************************************************************************
bool	CZoneLighter2002::CPredPointLightToPoint::operator() (CPointLightRT *pla, CPointLightRT *plb) const
{
	float	ra= (pla->BSphere.Center - Point).norm();
	float	rb= (plb->BSphere.Center - Point).norm();
	float	infA= (pla->PointLight.getAttenuationEnd() - ra) * pla->OODeltaAttenuation;
	float	infB= (plb->PointLight.getAttenuationEnd() - rb) * plb->OODeltaAttenuation;
	// return which light impact the most.
	// If same impact
	if(infA==infB)
		// return nearest
		return ra < rb;
	else
		// return better impact
		return  infA > infB;
}

// ***************************************************************************
void			CZoneLighter2002::processZonePointLightRT(vector<CPointLightNamed> &listPointLight)
{
	uint	i;
	vector<CPointLightRT*>		lightInfs;
	lightInfs.reserve(1024);

	// clear result list
	listPointLight.clear();

	// zoneToLight
	CZone	*zoneToLight= _Landscape->getZone(_ZoneToLight);
	if(!zoneToLight)
		return;

	// Build patchForPLs
	//===========
	vector<CPatchForPL>		patchForPLs;
	patchForPLs.resize(_PatchInfo.size());
	for(i=0; i<patchForPLs.size(); i++)
	{
		// Get OrderS/OrderT
		patchForPLs[i].OrderS= _PatchInfo[i].OrderS;
		patchForPLs[i].OrderT= _PatchInfo[i].OrderT;
		// resize TileLightInfluences
		uint	w= patchForPLs[i].WidthTLI= patchForPLs[i].OrderS/2 +1 ;
		uint	h= patchForPLs[i].HeightTLI= patchForPLs[i].OrderT/2 +1;
		patchForPLs[i].TileLightInfluences.resize(w*h);
	}


	// compute each TileLightInfluence
	//===========
	for(i=0; i<patchForPLs.size(); i++)
	{
		// progress
		progress ("Compute Influences of PointLights", 0.5f + 0.5f*i / (float)patchForPLs.size());

		CPatchForPL		&pfpl= patchForPLs[i];
		const CPatch	*patch= const_cast<const CZone*>(zoneToLight)->getPatch(i);

		uint	x, y;
		for(y= 0; y<pfpl.HeightTLI; y++)
		{
			for(x= 0; x<pfpl.WidthTLI; x++)
			{
				// compute the point and normal (normalized) where the TLI lies.
				//---------
				CVector		pos, normal;
				float		s, t;
				s= (float)x / (pfpl.WidthTLI-1);
				t= (float)y / (pfpl.HeightTLI-1);
				// Compute the Vertex, with Noise information (important for accurate raytracing).
				pos= patch->computeVertex(s, t);
				// Use UnNoised normal from BezierPatch, because the lighting does not need to be so precise.
				CBezierPatch	*bp= patch->unpackIntoCache();
				normal= bp->evalNormal(s, t);
				

				// Compute Which light influences him.
				//---------
				lightInfs.clear();
				// Search possible lights around the position.
				_StaticPointLightQuadGrid.select(pos, pos);
				// For all of them, get the ones which touch this point.
				CQuadGrid<CPointLightRT*>::CIterator	it= _StaticPointLightQuadGrid.begin();
				while(it != _StaticPointLightQuadGrid.end())
				{
					CPointLightRT	*pl= *it;

					// a light influence a TLI only if this one is FrontFaced to the light !!
					if( ( pl->BSphere.Center - pos ) * normal > 0)
					{
						// Add 5cm else it fails in some case where ( pl->BSphere.Center - pos ) * normal is 
						// nearly 0 and the point should be occluded.
						const float	deltaY= 0.05f;
						CVector	posToRT= pos + normal * deltaY;
						// Test if really in the radius of the light, if no occlusion, and if in SpotAngle
						if( pl->testRaytrace(posToRT) )
						{
							// Ok, add the light to the lights which influence the TLI 
							lightInfs.push_back(pl);
						}
					}

					// next
					it++;
				}

				// Choose the Best ones.
				//---------
				CPredPointLightToPoint	predPLTP;
				predPLTP.Point= pos;
				// sort.
				sort(lightInfs.begin(), lightInfs.end(), predPLTP);
				// truncate.
				lightInfs.resize( min((uint)lightInfs.size(), (uint)CTileLightInfluence::NumLightPerCorner) );


				// For each of them, fill TLI
				//---------
				CTileLightInfUnpack		tli;
				uint					lightInfId;
				for(lightInfId=0; lightInfId<lightInfs.size(); lightInfId++)
				{
					CPointLightRT	*pl= lightInfs[lightInfId];

					// copy light.
					tli.Light[lightInfId]= pl;
					// Compute light Diffuse factor.
					CVector		dir= pl->BSphere.Center - pos;
					dir.normalize();
					tli.LightFactor[lightInfId]= dir * normal;
					clamp(tli.LightFactor[lightInfId], 0.f, 1.f);
					// modulate by light attenuation.
					tli.LightFactor[lightInfId]*= pl->PointLight.computeLinearAttenuation(pos);

					// Inc RefCount of the light.
					pl->RefCount++;
				}
				// Reset any empty slot to NULL.
				for(; lightInfId<CTileLightInfluence::NumLightPerCorner; lightInfId++)
				{
					tli.Light[lightInfId]= NULL;
				}


				// Set TLI in patch.
				//---------
				pfpl.TileLightInfluences[y*pfpl.WidthTLI + x]= tli;
			}
		}
	}


	// compress and setup _PatchInfo with compressed data.
	//===========
	uint	plId= 0;
	// Process each pointLights
	for(i=0; i<_StaticPointLights.size(); i++)
	{
		CPointLightRT	&plRT= _StaticPointLights[i];
		// If this light is used.
		if(plRT.RefCount > 0)
		{
			// Must Copy it into Zone.
			listPointLight.push_back(plRT.PointLight);
			plRT.DstId= plId++;
			// If index >= 255, too many lights (NB: => because 255 is a NULL code).
			if(plId>=0xFF)
			{
				throw Exception("Too many Static Point Lights influence the zone!!");
			}
		}
	}

	// For each patch, compress TLI in PatchInfo.
	for(i=0; i<patchForPLs.size(); i++)
	{
		CPatchForPL		&pfpl= patchForPLs[i];
		CPatchInfo		&pInfo= _PatchInfo[i];

		uint	w= pfpl.WidthTLI;
		uint	h= pfpl.HeightTLI;

		// Fill  pInfo.TileLightInfluences
		pInfo.TileLightInfluences.resize(w*h);
		uint	x, y;
		for(y= 0; y<h; y++)
		{
			for(x= 0; x<w; x++)
			{
				uint	tliId= y*w + x;
				// For all light slot
				for(uint lightId= 0; lightId<CTileLightInfluence::NumLightPerCorner; lightId++)
				{
					CTileLightInfUnpack		&tliSrc= pfpl.TileLightInfluences[tliId];
					CTileLightInfluence		&tliDst= pInfo.TileLightInfluences[tliId];
					if(tliSrc.Light[lightId] == NULL)
					{
						// Mark as unused.
						tliDst.Light[lightId]= 0xFF;
					}
					else
					{
						// Get index.
						tliDst.Light[lightId]= tliSrc.Light[lightId]->DstId;
						// Get Diffuse Factor.
						tliDst.setDiffuseLightFactor(lightId, (uint8)(tliSrc.LightFactor[lightId]*255));
					}
				}
			}
		}

	}

}

///***********************************************************
///***********************************************************
// TileFlagsForPositionTowardWater
///***********************************************************
///***********************************************************


//==================================================================
/// for map insertion of CTileOfPatch structs
static inline bool operator < (const CTileOfPatch &lhs, const CTileOfPatch &rhs)
{
	return lhs.Patch == rhs.Patch  ?
		   lhs.TileId < rhs.TileId :
		   lhs.Patch  < rhs.Patch;	
};

/// A set of tiles from patch and their bbox
typedef std::map<CTileOfPatch, NLMISC::CAABBox> TTileOfPatchMap;

///***********************************************************
void CZoneLighter2002::computeTileFlagsForPositionTowardWater(const CLightDesc &lightDesc,
														  std::vector<const CTessFace*> &tessFaces														  
														  )
{	
	uint numTileAbove     = 0;
	uint numTileBelow     = 0;
	uint numTileIntersect = 0;
	
	/// the tiles that we have setupped so far...
	TTileOfPatchMap tiles;

	///////////////////////////////////////////
	//  First, build the bbox for all tiles  //
	///////////////////////////////////////////

	uint triCount = 0, totalTriCount = tessFaces.size();	

	nlinfo("Dealing with %d tessFaces", tessFaces.size());
	for (std::vector<const CTessFace*>::iterator it = tessFaces.begin(); it != tessFaces.end(); ++it, ++triCount)
	{
		/// does the face belong to the zone to light ?
		if ((*it)->Patch->getZone()->getZoneId() != _ZoneToLight) continue;
		/// if the tile flags say that micro vegetation is disabled, just skip that
		if ((*it)->Patch->Tiles[(*it)->TileId].getVegetableState() == CTileElement::VegetableDisabled)
			continue;

		CTileOfPatch top((*it)->TileId, (*it)->Patch);
		TTileOfPatchMap::iterator tileIt = tiles.find(top);

		/// test wether we've seen face(s) from this tile before
		if (tileIt == tiles.end()) // first time ?
		{
			/// build a bbox for this face
			NLMISC::CAABBox b;
			b.setMinMax((*it)->VBase->EndPos, (*it)->VLeft->EndPos);
			b.extend((*it)->VRight->EndPos);
			b.extend(b.getMax() + lightDesc.VegetableHeight * NLMISC::CVector::K); // adds vegetable height			
			tiles[top] = b;
		}
		else // extends the bbox with the given face
		{
			NLMISC::CAABBox &b = tileIt->second;			
			b.extend((*it)->VBase->EndPos);
			b.extend((*it)->VRight->EndPos);
			b.extend((*it)->VLeft->EndPos);
		}

		if ((triCount % 100) == 0)
		{
			progress("Building bbox from tiles", (float) triCount / totalTriCount);
		}
	}

	progress("Building bbox from tiles", 1.f);



	////////////////////////////////////////////////////
	// Now, check each tile bbox against water shapes //
	////////////////////////////////////////////////////	
	NLMISC::CPolygon   waterPoly;
	NLMISC::CPolygon2D tilePoly;
	tilePoly.Vertices.resize(4);

	uint tileCount = 0, totalTileCount = tiles.size();	

	for (TTileOfPatchMap::iterator tileIt = tiles.begin(); tileIt != tiles.end(); ++tileIt, ++tileCount)
	{
		const NLMISC::CVector v0 = tileIt->second.getMin();
		const NLMISC::CVector v1 = tileIt->second.getMax();

		/// build a top view from the bbox
		tilePoly.Vertices[0].set(v0.x, v0.y); 
		tilePoly.Vertices[1].set(v1.x, v0.y); 
		tilePoly.Vertices[2].set(v1.x, v1.y); 
		tilePoly.Vertices[3].set(v0.x, v1.y); 

		/// Select the candidate water shape from the quad grid
		_WaterShapeQuadGrid.clearSelection();
		_WaterShapeQuadGrid.select(tileIt->second.getMin(), tileIt->second.getMax());

		CTileElement &te = tileIt->first.Patch->Tiles[tileIt->first.TileId]; // alias to the current tile element

		/// test more accurate intersection for each water shape
		TWaterShapeQuadGrid::CIterator qgIt;
		for (qgIt = _WaterShapeQuadGrid.begin(); qgIt != _WaterShapeQuadGrid.end(); ++qgIt)
		{
			
			(*qgIt)->getShapeInWorldSpace(waterPoly);
			NLMISC::CPolygon2D poly(waterPoly);
			if (poly.intersect(tilePoly)) // above or below a water surface ?		
			{
				/// height of water 
				float waterHeight = waterPoly.Vertices[0].z;

				if (v1.z < waterHeight)
				{
					// below
					te.setVegetableState(CTileElement::UnderWater);
					//nlassert(te.getVegetableState() == CTileElement::UnderWater);
					++ numTileBelow;
				}
				else if (v0. z > waterHeight)
				{
					// above
					te.setVegetableState(CTileElement::AboveWater);
					//nlassert(te.getVegetableState() == CTileElement::AboveWater);
					++ numTileAbove;
				}
				else
				{
					// intersect water
					te.setVegetableState(CTileElement::IntersectWater);
					//nlassert(te.getVegetableState() == CTileElement::IntersectWater);
					++ numTileIntersect;
				}
				break;
			}
		}

		if (qgIt == _WaterShapeQuadGrid.end()) // no intersection found ? if yes it's above water
		{
			te.setVegetableState(CTileElement::AboveWater);	
			//nlassert(te.getVegetableState() == CTileElement::AboveWater);
			++ numTileAbove;
		}

		if ((tileCount % 50) == 0)
		{
			progress("Computing tile position towards water", (float) tileCount / totalTileCount);
		}
	}

	progress("Computing tile position towards water", 1.f);

	nlinfo(" %d tiles are above water.", numTileAbove);
	nlinfo(" %d tiles are below water.", numTileBelow);
	nlinfo(" %d tiles intersect water.", numTileIntersect);



	/// delete the quadgrid now
	NLMISC::contReset(_WaterShapeQuadGrid);
}

///***********************************************************
void CZoneLighter2002::setTileFlagsToDefault(std::vector<const CTessFace*> &tessFaces)
{
	/// We may setup a tile several time, but this doesn't matter here...
	for (std::vector<const CTessFace*>::iterator it = tessFaces.begin(); it != tessFaces.end(); ++it)
	{
		if ((*it)->Patch->getZone()->getZoneId() != _ZoneToLight) continue;
		CTileElement &te = (*it)->Patch->Tiles[(*it)->TileId];
		if (te.getVegetableState() != CTileElement::VegetableDisabled)
		{
			te.setVegetableState(CTileElement::AboveWater);
		}
	}
}


///***********************************************************
uint CZoneLighter2002::getAPatch (uint process)
{
	// Accessor
	NLMISC::CSynchronized<std::vector<bool> >::CAccessor access (&_PatchComputed);

	// Current index
	uint index = _LastPatchComputed[process];
	uint firstIndex = index;

	if (access.value().size() == 0)
		// no more patches
		return 0xffffffff;

	while (access.value()[index])
	{
		// Next patch
		index++;

		// First ?
		if (firstIndex == index)
			// no more patches
			return 0xffffffff;

		// Last patch ?
		if (index == _PatchInfo.size())
			index = 0;
	}

	// Visited
	access.value()[index] = true;

	// Last index
	_LastPatchComputed[process] = index;
	_NumberOfPatchComputed++;

	// Print
	progress ("Lighting patches", (float)_NumberOfPatchComputed/(float)_PatchInfo.size());

	// Return the index
	return index;
}
