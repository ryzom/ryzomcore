/** \file zone_lighter.cpp
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

#include "zone_lighter_2001.h"
#include "nel/3d/landscape.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/transform_shape.h"

#include "nel/misc/common.h"
#include "nel/misc/thread.h"

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
	
// ***************************************************************************


CZoneLighter2001::CZoneLighter2001 () : _TriangleListAllocateur(100000)
{
	
}
	
// ***************************************************************************

void CZoneLighter2001::init ()
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

float CZoneLighter2001::calcSkyContribution (sint s, sint t, float height, float skyIntensity, const CVector& normal)
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
	uint			_FirstPatch;
	uint			_LastPatch;
	uint			_Process;
	CZoneLighter2001	*_ZoneLighter;
	const CZoneLighter2001::CLightDesc	*_Description;
public:
	// Ctor
	CCalcRunnable (uint process, CZoneLighter2001 *zoneLighter, uint firstPatch, uint lastPatch, const CZoneLighter2001::CLightDesc *description)
	{
		_ZoneLighter=zoneLighter;
		_Process=process;
		_FirstPatch=firstPatch;
		_LastPatch=lastPatch;
		_Description=description;
	}

	// Run method
	void run()
	{
		_ZoneLighter->processCalc (_Process, _FirstPatch, _LastPatch, *_Description);
		_ZoneLighter->_ProcessExited++;
	}
};

// ***************************************************************************

void CZoneLighter2001::light (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description, std::vector<CTriangle>& obstacles, vector<uint> &listZone)
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
#ifdef NL_OS_WINDOWS
		// Automatique detection

		// System info structure
		SYSTEM_INFO info;

		// Fill it
		GetSystemInfo (&info);

		// Get number of proc
		_ProcessCount=info.dwNumberOfProcessors;
#else // NL_OS_WINDOWS

		// Get number of proc
		_ProcessCount=1;

#endif // NL_OS_WINDOWS
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
		CMatrix tmp=_RayBasis;
		tmp.invert ();

		uint cpu;
		for (cpu=0; cpu<_ProcessCount; cpu++)
		{
			_QuadGrid[cpu].changeBase (tmp);

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
			CZoneLighter2001::CTriangle& triangle=obstacles[triangleId];

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

			// Look for the min coordinate
			CVector minv;
			minv.minof (triangle.Triangle.V0, triangle.Triangle.V1);
			minv.minof (minv, triangle.Triangle.V2);

			// Look for the max coordinate
			CVector maxv;
			maxv.maxof (triangle.Triangle.V0, triangle.Triangle.V1);
			maxv.maxof (maxv, triangle.Triangle.V2);

			// Insert in the quad grid
			for (cpu=0; cpu<_ProcessCount; cpu++)
				_QuadGrid[cpu].insert (minv, maxv, &triangle);

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
		buildZoneInformation (landscape, listZone, description.Oversampling!=CLightDesc::NoOverSampling);

	}

	// Number of patch
	uint patchCount=_PatchInfo.size();

	// Patch by thread
	uint patchCountByThread = patchCount/_ProcessCount;
	patchCountByThread++;

	// Patch to allocate
	uint firstPatch=0;

	_ProcessExited=0;

	// Lunch threads
	for (uint process=1; process<_ProcessCount; process++)
	{
		// Last patch
		uint lastPatch=firstPatch+patchCountByThread;
		if (lastPatch>patchCount)
			lastPatch=patchCount;

		// Create a thread
		IThread *pThread=IThread::create (new CCalcRunnable (process, this, firstPatch, lastPatch, &description));

		// New first patch
		firstPatch=lastPatch;

		// Lunch
		pThread->start();
	}

	// My thread
	uint lastPatch=firstPatch+patchCountByThread;
	if (lastPatch>patchCount)
		lastPatch=patchCount;
	CCalcRunnable thread (0, this, firstPatch, lastPatch, &description);
	thread.run();

	// Wait for others processes
	while (_ProcessExited!=_ProcessCount)
	{
		nlSleep (10);
	}

	// Rebuild the zone

	// Progress bar
	progress ("Compress the lightmap", 0.5);

	// Debug: dump the RAW (uncompressed) lumels before CZone::build packs them —
	// needed to run the era block codec externally for byte-exact comparisons.
	if (const char *rawPath = getenv ("NL2001_DUMP_RAW_LUMELS"))
	{
		FILE *rawFile = fopen (rawPath, "wb");
		if (rawFile)
		{
			uint32 rawPatchCount = (uint32)_PatchInfo.size ();
			fwrite (&rawPatchCount, 4, 1, rawFile);
			for (uint rp = 0; rp < _PatchInfo.size (); ++rp)
			{
				uint8 ro = (uint8)_PatchInfo[rp].OrderS, rt = (uint8)_PatchInfo[rp].OrderT;
				fwrite (&ro, 1, 1, rawFile);
				fwrite (&rt, 1, 1, rawFile);
				uint32 rawLumelCount = (uint32)_PatchInfo[rp].Lumels.size ();
				fwrite (&rawLumelCount, 4, 1, rawFile);
				if (rawLumelCount)
					fwrite (&_PatchInfo[rp].Lumels[0], 1, rawLumelCount, rawFile);
			}
			fclose (rawFile);
		}
	}

	output.build (_ZoneToLight, _PatchInfo, _BorderVertices);
}

// ***************************************************************************

void CZoneLighter2001::processCalc (uint process, uint firstPatch, uint lastPatch, const CLightDesc& description)
{
	// *** Raytrace each patches

	// Pointer on the zone
	CZone *pZone=_Landscape->getZone (_ZoneToLight);

	// For each patch
	if (description.Shadow)
	{
		// Shape array
		CMultiShape *shapeArray=new CMultiShape;
		CMultiShape *shapeArrayTmp=new CMultiShape;
		shapeArray->Shapes.reserve (SHAPE_MAX);
		shapeArrayTmp->Shapes.reserve (SHAPE_MAX);

		for (uint patch=firstPatch; patch<lastPatch; patch++)
		{
			// Progress bar
			if (process==0)
				progress ("Raytrace", (float)(patch-firstPatch)/(float)(lastPatch-firstPatch));

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
		}
		delete shapeArray;
		delete shapeArrayTmp;
	}
	else
	{
		for (uint patch=firstPatch; patch<lastPatch; patch++)
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
	}

	// *** Antialising
	
	// Id of this zone in the array
	uint zoneNumber=_ZoneId[_ZoneToLight];

	// Enabled ?
	if ((description.Shadow)&&(description.Oversampling!=CLightDesc::NoOverSampling))
	{
		// For each patch
		uint patch;
		for (patch=firstPatch; patch<lastPatch; patch++)
		{
			// Progress bar
			if (process==0)
				progress ("Antialiasing", (float)(patch-firstPatch)/(float)(lastPatch-firstPatch));
				
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
				//bool shadowed=shadowPatch[s+t*orderLumelS];
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
	}

	// *** Lighting
	
	// For each patch
	uint patch;
	for (patch=firstPatch; patch<lastPatch; patch++)
	{
		// Progress bar
		if (process==0)
			progress ("Lighting", (float)(patch-firstPatch)/(float)(lastPatch-firstPatch));

		// Get a patch pointer
		const CPatch *pPatch=(const_cast<const CZone*>(pZone))->getPatch (patch);

		// Get the patch info
		CPatchInfo &patchInfo=_PatchInfo[patch];

		// ** Pointer on arries
		vector<bool> &binded=_Binded[zoneNumber][patch];
		vector<bool> &oversampleEdges=_OversampleEdges[patch];
		vector<CPatchUVLocator> &locator=_Locator[zoneNumber][patch];
		std::vector<CLumelDescriptor> &lumels=_Lumels[patch];

		// Go for light each lumel
		for (uint lumel=0; lumel<lumels.size(); lumel++)
		{
			// Sky contribution
			float skyContribution;

			// Calc sky contribution
			if (description.SkyContribution)
			{
				// Calc position 
				CVector &position=lumels[lumel].Position;
				float s=(position.x-_OrigineHeightField.x)/_HeightfieldCellSize;
				float t=(position.y-_OrigineHeightField.y)/_HeightfieldCellSize;
				sint sInt=(sint)(floor (s+0.5f));
				sint tInt=(sint)(floor (t+0.5f));

				// Bilinear
				float skyContributionTab[2][2];
				skyContributionTab[0][0] = calcSkyContribution (sInt-1, tInt-1, position.z, description.SkyIntensity, lumels[lumel].Normal);
				skyContributionTab[1][0] = calcSkyContribution (sInt, tInt-1, position.z, description.SkyIntensity, lumels[lumel].Normal);
				skyContributionTab[1][1] = calcSkyContribution (sInt, tInt, position.z, description.SkyIntensity, lumels[lumel].Normal);
				skyContributionTab[0][1] = calcSkyContribution (sInt-1, tInt, position.z, description.SkyIntensity, lumels[lumel].Normal);
				
				float sFact=s+0.5f-sInt;
				float tFact=t+0.5f-tInt;
				skyContribution = (skyContributionTab[0][0]*(1.f-sFact) + skyContributionTab[1][0]*sFact)*(1.f-tFact) +
					(skyContributionTab[0][1]*(1.f-sFact) + skyContributionTab[1][1]*sFact)*tFact;
			}
			else
				skyContribution=0.f;

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
	}
}

// ***************************************************************************

uint8 CZoneLighter2001::getMaxPhi (sint s, sint t, sint deltaS, sint deltaT, float heightPos)
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

void CZoneLighter2001::testRaytrace (const CVector& position, const CVector& normal, const CPlane &plane, float s, float t, uint patchId, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu)
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

void CZoneLighter2001::rayTrace (const CVector& position, const CVector& normal, float s, float t, uint patchId, float &factor, CMultiShape &shape, CMultiShape &shapeTmp, uint cpu)
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

void CZoneLighter2001::rayTraceTriangle (const NLMISC::CTriangle& toOverSample, CVector& normal, uint order, float s, float t, float &factor, uint &tested, uint patch)
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
		NLMISC::CTriangle subTri;
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

bool CZoneLighter2001::isLumelOnEdgeMustBeOversample (uint patch, uint edge, sint s, sint t, const vector<bool> &binded, 
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
			return (shadowBuffer[patchOut->getPatchId()][ss+(patchOut->getOrderS()<<2)*tt]!=shadowed);
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


sint16 CZoneLighter2001::_GetNormalDeltaS[4]={ -1, 0, 1, 0 };
sint16 CZoneLighter2001::_GetNormalDeltaT[4]={ 0, 1, 0, -1 };

// ***************************************************************************

void CZoneLighter2001::getNormal (const CPatch *pPatch, sint16 lumelS, sint16 lumelT, vector<CPatchUVLocator> &locator, 
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
							uint newEdge;
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

void CZoneLighter2001::addTriangles (CLandscape &landscape, vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray)
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
		triangleArray.push_back (CTriangle (NLMISC::CTriangle (face->VBase->Pos, face->VLeft->Pos, face->VRight->Pos), 
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

void CZoneLighter2001::addTriangles (const IShape &shape, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
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

void CZoneLighter2001::addTriangles (const CMeshGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
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

void CZoneLighter2001::addTriangles (const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
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

void CZoneLighter2001::excludeAllPatchFromRefineAll (CLandscape &landscape, vector<uint> &listZone, bool exclude)
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

void CZoneLighter2001::buildZoneInformation (CLandscape &landscape, const vector<uint> &listZone, bool oversampling)
{
	// Bool visit
	vector<vector<uint> > visited;

	// Zone count
	uint zoneCount=listZone.size();

	// Resize arries
	_Locator.resize (zoneCount);
	_Binded.resize (zoneCount);
	_BindInfo.resize (zoneCount);

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
			_BezierPatch.resize(patchCount);
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

			// *** Center coordinates
			float centerS=(face->PVLeft.getS()+face->PVRight.getS())/2.f;
			float centerT=(face->PVLeft.getT()+face->PVRight.getT())/2.f;
			CVector centerPos=(face->VLeft->Pos+face->VRight->Pos)/2.f;

			// *** Base Coordinates
			CVector pos[14];
			pos[0]=face->VBase->Pos;		// p0
			pos[1]=face->VRight->Pos;
			pos[2]=face->VLeft->Pos;		// p2
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
		uint lumelCount=lumels.size();

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
			plane.make (face->VBase->Pos, face->VLeft->Pos, face->VRight->Pos);
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
		uint lumelCount=lumels.size();

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

CZoneLighter2001::CLightDesc::CLightDesc ()
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

