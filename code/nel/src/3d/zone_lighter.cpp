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

#include "std3d.h"

#include "nel/3d/zone_lighter.h"
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
#include "nel/misc/algo.h"


#ifdef NL_OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#	include <windows.h>
#	include <winbase.h>
#endif // NL_OS_WINDOWS

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// Define this to render the zbuffers into a bitmap zbuffer and save it into a jpeg
// #define SAVE_ZBUFFER "c:/temp"

#define DEFAULT_JITTER 0.4f
#define DEFAULT_ZBUFFER_LANDSCAPE_SIZE 32768
#define DEFAULT_ZBUFFER_OBJECT_SIZE (32768*3)
#define DEFAULT_SUN_DISTANCE 50000
#define DEFAULT_SUN_FOV (NLMISC::Pi/6)
#define DEFAULT_SUN_CENTER (CVector (0, 0, 0))
#define DEFAULT_SUN_RADIUS 5000
#define DEFAULT_SUN_SRQT_SAMPLES 4

// Bad coded: don't set too big else it allocates too much memory.
#define NL3D_ZONE_LIGHTER_CUBE_GRID_SIZE 16

// ***************************************************************************
/*

Documentation:


	To light a zone, you must first adding shadow caster triangles using the addTriangle() methods.
	Triangles can come from landscape zones or IG meshes. Then call the lighting process with ligth().

	addTriangle ()
		- Add landscape triangles to shadow caster triangle list
			- Tesselate the landscape to shadow accuracy (2 meters)
		- Add others triangles to shadow caster triangle list  (trees, building)
			- AlphaTest textures can be used here

	light ()
		The lighting process uses a software zbuffers render to compute shadow attenuation of the zone.
		CRenderZBuffer () (multithread)
			- Render shadow caster triangles into the light zbuffers for shadows. Each z value is tested and
			written in the pixel and in the 8 neighbor pixels.
			- There is a zbuffer per landscape softshadow sample and an additionnal zbuffer for objects. So
			landscape triangles cast softshadows and object (trees and building) cast antialiased shadows.

		- Render shadow caster triangles into a heightfield used for radiosity

		buildZoneInformation ()

			- Tesselate the landscape to shadow accuracy (2 meters)

			- Compute lumel positions.
				Lumel position is the average of lumel triangles center overlapping
				the lumel but using the shadow accuracy triangle position because
				we need the same triangles than shadow caster polygons.
				Border lumel position are extended to fit the patch border.

			- Tesselate to lumel accuracy (0.5 meter)

			- Compute lumel normal
				- Lumel normal is the average of lumel triangles normals. The normals
				comes from the lumel accuracy to get more precise lighting. So normals
				are interpolated from the center of the lumel but they will be rendered
				on the patch border. Unlike the position, we can extand the normal to the
				border without loosing normal precision because normal interpolation is
				aligned with the tesselation. So we need some border normal smoothing.

			- Border normal smoothing
				- Normals on the border of the patches are smoothed with neighbor normals.

		CLightRunnable () (multithread)

			- For each patches and for each lumels

				attenuation ()
					- Compute shadow attenuation
						- Get an antialised attenuation value in each lansdcape softshadow zbuffer.
						Average it with jitter.
						- Get an antialised attenuation value from the object zbuffer.
						- Return the smaller value of the both.

				- Compute sun lighting (dot product)

				getSkyContribution ()
					- Compute sky lighting (radiosity)
						- Algorithm
							- Get the lumel position in the heightfield
							- Lookup in the 8 2d directions for max height
							- Compute an approximation of the sky surface visible from
							the lumel position
				- Store final lumel luminosity

*/

// ***************************************************************************

inline float easineasout(float x)
{
 float y;
 // cubic tq f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
 float x2=x*x;
 float x3=x2*x;
 y= -2*x3 + 3*x2;
 return y;
}

// ***************************************************************************

inline void transformVectorToZBuffer (const CZoneLighter::CZBuffer& zbuffer, const CVector &world, CVector &projected)
{
	projected = zbuffer.WorldToZBuffer * world;
	float temp = projected.z;
	projected.z = projected.y;
	projected.y = -temp;
	projected = zbuffer.WorldToZBufferFrustum.project (projected);

	// Scale to zbuffer size
	projected.x *= zbuffer.ZBufferPixelSize;
	projected.y *= zbuffer.ZBufferPixelSize;
	projected.z = temp;
}

// ***********************************************************

static const sint DeltaZ[9][2]=
{
	{0, 0},
	{-1, 0},
	{1, 0},
	{0, -1},
	{0, 1},
	{-1, -1},
	{1, 1},
	{1, -1},
	{-1, 1},
};

// ***************************************************************************

inline float testZPercentageCloserFilter (float x, float y, float z, CZoneLighter::CZBuffer &zbuffer, const CZoneLighter::CLightDesc &description, bool &zBufferOverflowFlag)
{
	// See "Rendering Antialiased Shadows With Depth Maps" Reeves, Salesint, Cook, ACM 1987

	// Bilinear filtering

	float biliValues[2][2];

	float ix = (float)floor (x-0.5f);
	float factorX = x - (ix+0.5f);
	nlassert (factorX>=0);
	nlassert (factorX<=1);

	float iy = (float)floor (y-0.5f);
	float factorY = y - (iy+0.5f);
	nlassert (factorY>=0);
	nlassert (factorY<=1);

	sint dx, dy;
	for (dy=0; dy<2; dy++)
	for (dx=0; dx<2; dx++)
	{
		const sint fx = dx + (sint)ix;
		const sint fy = dy + (sint)iy;
		if ((fx >= 0) && (fx < zbuffer.LocalZBufferWidth) && (fy >= 0) && (fy < zbuffer.LocalZBufferHeight))
		{
			const float zRed = zbuffer.Pixels[fx + (zbuffer.LocalZBufferHeight - 1 - fy) * zbuffer.LocalZBufferWidth];

			biliValues[dx][dy] = (zRed < (-z)) ? 0.f : 1.f;
		}
		else
		{
			biliValues[dx][dy] = 1;
			zBufferOverflowFlag = true;
		}
	}

	// Bilinear
	return (biliValues[0][0] * (1 - factorX) + biliValues[1][0] * factorX) * (1 - factorY) +
		(biliValues[0][1] * (1 - factorX) + biliValues[1][1] * factorX) * factorY;
}

// ***************************************************************************

CZoneLighter::CZoneLighter () : _PatchComputed ("PatchComputed")
{
}

// ***************************************************************************

void CZoneLighter::init ()
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

	// Init some containers
	_ZBufferOverflow = false;
	_Bitmaps.clear ();
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

float CZoneLighter::calcSkyContribution (sint s, sint t, float height, float skyIntensity, const CVector& normal) const
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
	skyContribution=(float)(skyIntensity*(normal*k)/(2*Pi));
	clamp (skyContribution, 0.f, 1.f);
	return skyContribution;
}

// ***************************************************************************

void NEL3DCalcBase (CVector &direction, CMatrix& matrix)
{
	direction.normalize();
	CVector		K=-direction;
	CVector		I=CVector::K^K;
	CVector		J=K^I;
	J.normalize();
	I=J^K;
	I.normalize();
	matrix.identity();
	matrix.setRot(I,J,K, true);
}

// ***************************************************************************

void setCPUMask (IThread *thread, uint process)
{
	// Set the processor mask
	uint64 mask = IProcess::getCurrentProcess()->getCPUMask ();

	// Mask must not be NULL
	nlassert (mask != 0);

	if (mask != 0)
	{
		uint i=0;
		uint count = 0;
		for(;;)
		{
			if (mask & (UINT64_CONSTANT(1)<<i))
			{
				if (count == process)
					break;
				count++;
			}
			i++;
			if (i==64)
				i = 0;
		}

		// Set the CPU mask
		thread->setCPUMask (1<<i);
	}
}

// ***************************************************************************

class NL3D::CLightRunnable : public IRunnable
{
	// Members
	uint			_Process;
	CZoneLighter	*_ZoneLighter;
	const CZoneLighter::CLightDesc	*_Description;

public:
	IThread			*Thread;

public:
	// Ctor
	CLightRunnable (uint process, CZoneLighter *zoneLighter, const CZoneLighter::CLightDesc *description)
	{
		_ZoneLighter = zoneLighter;
		_Process = process;
		_Description = description;
	}

	// Run method
	void run()
	{
		// Set the CPU mask
		setCPUMask (Thread, _Process);

		_ZoneLighter->processCalc (_Process, *_Description);
		_ZoneLighter->_ProcessExited++;
	}
};


// ***************************************************************************

class NL3D::CRenderZBuffer : public IRunnable
{
	// Members
	uint			_Process;
	CZoneLighter	*_ZoneLighter;

	// The lighting decription
	const CZoneLighter::CLightDesc	*_Description;

	// Triangles to render
	uint			_FirstTriangle;
	uint			_NumTriangle;
	const vector<CZoneLighter::CTriangle>		*_Triangles;

public:
	IThread			*Thread;

public:
	// Ctor
	CRenderZBuffer (uint process, CZoneLighter *zoneLighter, const CZoneLighter::CLightDesc	*description, uint firstTriangle, uint numTriangle, const vector<CZoneLighter::CTriangle> *triangles)
	{
		_ZoneLighter = zoneLighter;
		_Description = description;
		_Process = process;
		_FirstTriangle = firstTriangle;
		_NumTriangle = numTriangle;
		_Triangles = triangles;
	}

	// Run method
	virtual void run ();
};

// ***************************************************************************

#define CLIPPED_TOP 1
#define CLIPPED_BOTTOM 2
#define CLIPPED_RIGHT 3
#define CLIPPED_LEFT 4
#define CLIPPED_ALL (CLIPPED_TOP|CLIPPED_BOTTOM|CLIPPED_LEFT|CLIPPED_RIGHT)

void RenderTriangle (const CZoneLighter::CTriangle &triangle, const CZoneLighter::CLightDesc &description, CPolygon2D::TRasterVect &borders,
					CFastMutex &mutex, CZoneLighter::CZBuffer &zbuffer, uint radius)
{
	// *** Transform it in the zbuffer basis

	// 2d polygon used for rasteriation
	CPolygon2D zBasis;
	zBasis.Vertices.resize (3);

	// 3d polygon used for the gradient
	NLMISC::CTriangle gradientTriangle;

	// One over z value
	float	ooz[3];

	// Clipping
	uint8 in = 0;

	// For each vertex
	for (uint j=0; j<3; j++)
	{
		// Pointer on the vector
		const CVector *pt = (&triangle.Triangle.V0)+j;
		CVector *ptDest = (&gradientTriangle.V0)+j;

		// Transform it in the zbuffer basis
		transformVectorToZBuffer (zbuffer, *pt, *ptDest);

		// Clip
		if (ptDest->x >= zbuffer.LocalZBufferXMin)
			in |= CLIPPED_LEFT;
		if (ptDest->x <= zbuffer.LocalZBufferXMax)
			in |= CLIPPED_RIGHT;
		if (ptDest->y >= zbuffer.LocalZBufferYMin)
			in |= CLIPPED_TOP;
		if (ptDest->y <= zbuffer.LocalZBufferYMax)
			in |= CLIPPED_BOTTOM;

		// Set the 2d points
		zBasis.Vertices[j].x = ptDest->x - (float)zbuffer.LocalZBufferXMin;
		zBasis.Vertices[j].y = ptDest->y - (float)zbuffer.LocalZBufferYMin;
		ooz[j] = 1.f / ptDest->z;

		// No z
		ptDest->z = 0;
	}

	// Not clipped ?
	if (in == CLIPPED_ALL)
	{
		// Rasterise
		sint minimumY;
		borders.clear ();
		zBasis.computeBorders (borders, minimumY);

		// Compute the gradient for one over z
		CVector ozzGradient;
		gradientTriangle.computeGradient (ooz[0], ooz[1], ooz[2], ozzGradient);

		// Need uv ?
		bool needUV = triangle.Texture != NULL;

		// Compute the gradient for uv
		CVector uGradient;
		CVector vGradient;
		if (needUV)
		{
			gradientTriangle.computeGradient (triangle.U[0], triangle.U[1], triangle.U[2], uGradient);
			gradientTriangle.computeGradient (triangle.V[0], triangle.V[1], triangle.V[2], vGradient);
		}

		// Texture information
		uint width=0;
		uint height=0;
		const CObjectVector<uint8> *pixels = 0;
		if (needUV)
		{
			// Get pixels
			pixels = &triangle.Texture->getPixels ();

			// Get width and height
			width = triangle.Texture->getWidth ();
			height = triangle.Texture->getHeight ();
		}

		// For each scanlines
		sint y = std::max (minimumY, 0);
		sint yMax = std::min ((sint)(minimumY+borders.size ()), zbuffer.LocalZBufferHeight);
		for (; y<yMax; y++)
		{
			// Ref on the raster
			const CPolygon2D::TRaster &raster = borders[y-minimumY];

			// Gradient y for ooz, u and v
			const float deltaY = (float)y - zBasis.Vertices[0].y;
			const float oozGradientY = deltaY * ozzGradient.y;
			float uGradientY=0.0f;
			float vGradientY=0.0f;
			if (needUV)
			{
				uGradientY = deltaY * uGradient.y;
				vGradientY = deltaY * vGradient.y;
			}

			// Clip it
			sint x = std::max (raster.first, 0);
			sint xMax = std::min (raster.second+1, zbuffer.LocalZBufferWidth);
			for (; x<xMax; x++)
			{
				// Gradient x for ooz, u and v
				const float deltaX = (float)x - zBasis.Vertices[0].x;
				const float oozGradientX = deltaX * ozzGradient.x;
				float uGradientX=0.0f;
				float vGradientX=0.0f;
				if (needUV)
				{
					uGradientX = deltaX * uGradient.x;
					vGradientX = deltaX * vGradient.x;
				}

				// Calc z
				float z = - 1.f / (ooz[0] + oozGradientX + oozGradientY);

				// Calc u & v
				float u;
				float v;
				bool alphaTest = true;
				if (needUV)
				{
					// Compute uv
					u = triangle.U[0] + uGradientX + uGradientY;
					v = triangle.V[0] + vGradientX + vGradientY;

					// Clamp or wrap ?
					if (triangle.Flags & CZoneLighter::CTriangle::ClampU)
						clamp (u, 0.f, 1.f);
					else
						u -= (float)floor (u);
					if (triangle.Flags & CZoneLighter::CTriangle::ClampV)
						clamp (v, 0.f, 1.f);
					else
						v -= (float)floor (v);

					// Lookup in the texture
					u *= width;
					v *= height;
					clamp (u, 0, width-1);
					clamp (v, 0, height-1);
					uint8 alpha = ((const CRGBA*)&((*pixels)[(((uint)u)+((uint)v)*width)*sizeof (CRGBA)]))->A;

					// Alpha test
					alphaTest = alpha >= triangle.AlphaTestThreshold;
				}

				// Good alpha test ?
				if (alphaTest)
				{
					// Enter the mutex
					mutex.enter ();

					// Write Z around
					uint d;
					for (d=0; d<radius; d++)
					{
						// Ref in the zbuffer
						sint fx = x + DeltaZ[d][0];
						sint fy = y + DeltaZ[d][1];
						if ( (fx >= 0) && (fx < zbuffer.LocalZBufferWidth) && (fy >= 0) && (fy < zbuffer.LocalZBufferHeight) )
						{
							float &zValue = zbuffer.Pixels[fx+(zbuffer.LocalZBufferHeight-fy-1)*zbuffer.LocalZBufferWidth];

							// Z test
							if (z < zValue)
							{
								// Render z in zbuffer
								zValue = z;
							}
						}
					}

					// Leave the mutex
					mutex.leave ();
				}
			}
		}
	}
}


void NL3D::CRenderZBuffer::run()
{
	// Set the CPU mask
	setCPUMask (Thread, _Process);

	// Span array
	CPolygon2D::TRasterVect borders;

	// For each triangles
	uint i;
	for (i=_FirstTriangle; i<_FirstTriangle+_NumTriangle; i++)
	{
		// Triangle reference
		const CZoneLighter::CTriangle &triangle = (*_Triangles)[i];

		// Keep backface and doublesided polygons
		if ((triangle.Flags & CZoneLighter::CTriangle::DoubleSided) || ((triangle.getPlane ().getNormal() * _ZoneLighter->_SunDirection) > 0))
		{
			// Landscape triangle ?
			if (triangle.Flags & CZoneLighter::CTriangle::Landscape)
			{
				// For each landscape zbuffer
				uint sample;
				const uint samples = _Description->SoftShadowSamplesSqrt*_Description->SoftShadowSamplesSqrt;
				for (sample=0; sample<samples; sample++)
				{
					RenderTriangle (triangle, *_Description, borders, _ZoneLighter->_Mutex, _ZoneLighter->_ZBufferLandscape[sample], 9);
				}
			}
			else
			{
				// Render in a high resolution zbuffer
				RenderTriangle (triangle, *_Description, borders, _ZoneLighter->_Mutex, _ZoneLighter->_ZBufferObject, 1);
			}
		}
		_ZoneLighter->_NumberOfPatchComputed++;
	}

	// Exit
	_ZoneLighter->_ProcessExited++;
}

// ***************************************************************************

class NL3D::CCalcLightableShapeRunnable : public IRunnable
{
public:
	CCalcLightableShapeRunnable(uint process,
								CZoneLighter *zoneLighter,
								const CZoneLighter::CLightDesc *description,
								CZoneLighter::TShapeVect *shapeToLit,
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
	void run()
	{
		_ZoneLighter->processLightableShapeCalc(_Process, _ShapesToLit, _FirstShape, _LastShape, *_Description);
		_ZoneLighter->_ProcessExited++;
	}
private:
	CZoneLighter						*_ZoneLighter;
	const CZoneLighter::CLightDesc		*_Description;
	CZoneLighter::TShapeVect	*_ShapesToLit;
	uint								_FirstShape, _LastShape;
	uint								_Process;

};

// ***************************************************************************

void draw2dLine (CBitmap &bitmap, float x0, float y0, float x1, float y1, const CRGBA &color)
{
	static vector< std::pair<sint, sint> > lines;
	drawFullLine (x0, y0, x1, y1, lines);

	// Bitmap pixels
	CRGBA *pixels = (CRGBA*)&(bitmap.getPixels ()[0]);

	// Bitmap size
	sint width = (sint)bitmap.getWidth ();
	sint height = (sint)bitmap.getHeight ();

	// Draw the line
	uint i;
	for (i=0; i<lines.size (); i++)
	{
		sint x = lines[i].first;
		sint y = lines[i].second;

		// Clip
		if ( (x >= 0) && (x < width) && (y >= 0) && (y < height) )
		{
			pixels[x+(height-y-1)*width] = color;
		}
	}
}

// ***************************************************************************

void InitZBuffer (CZoneLighter::CZBuffer &zbuffer, const CVector &SunPosition, const CMatrix &rayBasis, const CAABBoxExt &zoneBB, uint zBufferPixelSize, const CZoneLighter::CLightDesc& description)
{
	// Clac the zbuffer world size
	const float zBufferWorldSize = (float)(tan (description.SunFOV/2)*description.SunDistance*2);

	// ** Compute the zbuffer basis
	zbuffer.WorldToZBuffer.identity ();

	zbuffer.WorldToZBuffer = rayBasis;
	zbuffer.WorldToZBuffer.setPos (SunPosition);
	zbuffer.WorldToZBuffer.invert ();
	zbuffer.WorldToZBufferFrustum.init ((float)zBufferWorldSize, (float)zBufferWorldSize, description.SunDistance, description.SunDistance*2);

	// Zbuffer size
	zbuffer.ZBufferPixelSize = zBufferPixelSize;

	// Evaluate the size of the local zbuffer

	// The zone bounding box
	CVector bMin = zoneBB.getMin ();
	CVector bMax = zoneBB.getMax ();
	transformVectorToZBuffer (zbuffer, CVector (bMin.x, bMax.y, bMin.z), zbuffer.BoundingBoxVectors[0]);
	transformVectorToZBuffer (zbuffer, CVector (bMin.x, bMin.y, bMin.z), zbuffer.BoundingBoxVectors[1]);
	transformVectorToZBuffer (zbuffer, CVector (bMax.x, bMin.y, bMin.z), zbuffer.BoundingBoxVectors[2]);
	transformVectorToZBuffer (zbuffer, CVector (bMax.x, bMax.y, bMin.z), zbuffer.BoundingBoxVectors[3]);
	transformVectorToZBuffer (zbuffer, CVector (bMin.x, bMax.y, bMax.z), zbuffer.BoundingBoxVectors[4]);
	transformVectorToZBuffer (zbuffer, CVector (bMin.x, bMin.y, bMax.z), zbuffer.BoundingBoxVectors[5]);
	transformVectorToZBuffer (zbuffer, CVector (bMax.x, bMin.y, bMax.z), zbuffer.BoundingBoxVectors[6]);
	transformVectorToZBuffer (zbuffer, CVector (bMax.x, bMax.y, bMax.z), zbuffer.BoundingBoxVectors[7]);

	// Get the min and max
	zbuffer.LocalZBufferXMin = 0x7fffffff;
	zbuffer.LocalZBufferYMin = 0x7fffffff;
	zbuffer.LocalZBufferXMax = 0x80000000;
	zbuffer.LocalZBufferYMax = 0x80000000;
	zbuffer.LocalZBufferZMin = FLT_MAX;
	zbuffer.LocalZBufferZMax = -FLT_MAX;
	uint j;
	for (j=0; j<8; j++)
	{
		sint minX = (sint)floor (zbuffer.BoundingBoxVectors[j].x);
		sint maxX = (sint)ceil (zbuffer.BoundingBoxVectors[j].x);
		sint minY = (sint)floor (zbuffer.BoundingBoxVectors[j].y);
		sint maxY = (sint)ceil (zbuffer.BoundingBoxVectors[j].y);
		if (minX<zbuffer.LocalZBufferXMin)
			zbuffer.LocalZBufferXMin = minX;
		if (maxX>zbuffer.LocalZBufferXMax)
			zbuffer.LocalZBufferXMax = maxX;
		if (minY<zbuffer.LocalZBufferYMin)
			zbuffer.LocalZBufferYMin = minY;
		if (maxY>zbuffer.LocalZBufferYMax)
			zbuffer.LocalZBufferYMax = maxY;
		if ((-zbuffer.BoundingBoxVectors[j].z)<zbuffer.LocalZBufferZMin)
			zbuffer.LocalZBufferZMin = -zbuffer.BoundingBoxVectors[j].z;
		if ((-zbuffer.BoundingBoxVectors[j].z)>zbuffer.LocalZBufferZMax)
			zbuffer.LocalZBufferZMax = -zbuffer.BoundingBoxVectors[j].z;
	}

	// Expand the zbuffer
	zbuffer.LocalZBufferXMax++;
	zbuffer.LocalZBufferXMin--;
	zbuffer.LocalZBufferYMax++;
	zbuffer.LocalZBufferYMin--;

	zbuffer.LocalZBufferWidth = zbuffer.LocalZBufferXMax-zbuffer.LocalZBufferXMin;
	zbuffer.LocalZBufferHeight = zbuffer.LocalZBufferYMax-zbuffer.LocalZBufferYMin;

	// Resize and clear the zbuffer
	zbuffer.Pixels.resize (0);
	zbuffer.Pixels.resize (zbuffer.LocalZBufferWidth*zbuffer.LocalZBufferHeight, FLT_MAX);
}

// ***************************************************************************

#ifdef SAVE_ZBUFFER
void SaveZBuffer (CZoneLighter::CZBuffer &zbuffer, const char *filename)
{
	// Resize the bitmap
	CBitmap bitmap;
	bitmap.resize (zbuffer.LocalZBufferWidth, zbuffer.LocalZBufferHeight, CBitmap::Luminance);

	// Get pixels
	CObjectVector<uint8> &pixels = bitmap.getPixels ();

	// Draw it
	uint samples = zbuffer.LocalZBufferWidth*zbuffer.LocalZBufferHeight;
	for (uint i=0; i<samples; i++)
	{
		// Get the value
		float value = (zbuffer.Pixels[i] - zbuffer.LocalZBufferZMin) * 255 / (zbuffer.LocalZBufferZMax - zbuffer.LocalZBufferZMin);
		clamp (value, 0, 255);
		pixels[i] = (uint8)value;
	}

	// Convert to RGBA
	bitmap.convertToType (CBitmap::RGBA);

	// Draw some red lines
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[0].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[0].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[1].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[1].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[0].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[0].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[3].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[3].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[2].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[2].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[1].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[1].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[2].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[2].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[3].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[3].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[4].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[4].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[5].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[5].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[4].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[4].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[7].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[7].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[6].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[6].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[5].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[5].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[6].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[6].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[7].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[7].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[0].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[0].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[4].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[4].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[1].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[1].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[5].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[5].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[2].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[2].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[6].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[6].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);
	draw2dLine (bitmap, zbuffer.BoundingBoxVectors[3].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[3].y-(float)zbuffer.LocalZBufferYMin, zbuffer.BoundingBoxVectors[7].x-(float)zbuffer.LocalZBufferXMin, zbuffer.BoundingBoxVectors[7].y-(float)zbuffer.LocalZBufferYMin, CRGBA::Red);

	// Render it
	COFile outputZFile;

	// Open it
	if (outputZFile.open (filename))
	{
		// Save the new zone
		try
		{
			// Convert to RGBA
			bitmap.convertToType (CBitmap::RGBA);

			// Save it
			bitmap.writeJPG (outputZFile, 128);
		}
		catch (const Exception& except)
		{
			// Error message
			nlwarning ("ERROR writing %s: %s\n", filename, except.what());
		}
	}
	else
	{
		// Error can't open the file
		nlwarning ("ERROR Can't open %s for writing\n", filename);
	}
}
#endif // SAVE_ZBUFFER

// ***************************************************************************

void FilterZBuffer (CZoneLighter::CZBuffer &zbuffer, uint filterRadius)
{
	// Resize the temp buffer
	static std::vector<float> tempPixels;
	tempPixels = zbuffer.Pixels;

	sint x, y;
	for (y=0; y<zbuffer.LocalZBufferHeight; y++)
	for (x=0; x<zbuffer.LocalZBufferWidth; x++)
	{
		// The Value
		float &newValue = tempPixels[x+y*zbuffer.LocalZBufferWidth];

		uint n;
		for (n=1; n<filterRadius; n++)
		{
			const sint fx = x + DeltaZ[n][0];
			const sint fy = y + DeltaZ[n][1];

			// Clip
			if ( (fx>=0) && (fx < zbuffer.LocalZBufferWidth) && (fy>=0) && (fy < zbuffer.LocalZBufferHeight) )
			{
				const float &testValue = zbuffer.Pixels[fx+fy*zbuffer.LocalZBufferWidth];
				if (testValue < newValue)
					newValue = testValue;
			}
		}

	}

	// Copy the new zbuffer
	zbuffer.Pixels = tempPixels;
}

// ***************************************************************************

void CZoneLighter::light (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description, std::vector<CTriangle>& obstacles, vector<uint> &listZone)
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
	_SunDirection=description.SunDirection;
	NEL3DCalcBase (_SunDirection, _RayBasis);

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

	// Number of obstacle polygones
	nlinfo ("Obstacle polygones : %u", (uint)obstacles.size ());

	// Number of CPUS used
	nlinfo ("Number of CPU used: %u", _ProcessCount);

	// Zone pointer
	CZone *pZone=landscape.getZone (_ZoneToLight);
	if (pZone)
	{
		// *** Compute center of the object

		// Get the zone bounding box
		const CAABBoxExt &zoneBB=pZone->getZoneBB();

		// Get the center
		CVector center = zoneBB.getCenter ();

		// *** Compute planes
		const uint size=(uint)obstacles.size();
		uint triangleId;
		for (triangleId=0; triangleId<size; triangleId++)
		{
			// Triangle ref
			CZoneLighter::CTriangle& triangle=obstacles[triangleId];

			// Calc the plane
			triangle._Plane.make (triangle.Triangle.V0, triangle.Triangle.V1, triangle.Triangle.V2);
		}

		// Create landscape zbuffers
		_ZBufferLandscape.resize (description.SoftShadowSamplesSqrt*description.SoftShadowSamplesSqrt);

		uint sampleX;
		uint sampleY;
		for (sampleY=0; sampleY<description.SoftShadowSamplesSqrt; sampleY++)
		for (sampleX=0; sampleX<description.SoftShadowSamplesSqrt; sampleX++)
		{
			// *** Render the light zbuffer
			CZBuffer &zbuffer = _ZBufferLandscape[sampleX + sampleY*description.SoftShadowSamplesSqrt];

			// Delta pos for area light
			float deltaX = ( (float)sampleX + 0.5f  - (float)description.SoftShadowSamplesSqrt / 2.f) / (float)description.SoftShadowSamplesSqrt;
			float deltaY = ( (float)sampleY + 0.5f  - (float)description.SoftShadowSamplesSqrt / 2.f) / (float)description.SoftShadowSamplesSqrt;
			CVector lightPos = _RayBasis.getI () * ((float)description.SunRadius * deltaX) + _RayBasis.getJ () * ((float)description.SunRadius * deltaY);
			lightPos = description.SunCenter - (description.SunDirection * description.SunDistance) + lightPos;

			InitZBuffer (zbuffer, lightPos, _RayBasis, zoneBB, description.ZBufferLandscapeSize, description);
			nlinfo ("Zbuffer %d size : %d x %d", sampleX+sampleY*description.SoftShadowSamplesSqrt, zbuffer.LocalZBufferWidth, zbuffer.LocalZBufferHeight);
		}


		// *** Init the zbuffer for the vegetation
		CVector lightPos = description.SunCenter - (description.SunDirection * description.SunDistance);
		InitZBuffer (_ZBufferObject, lightPos, _RayBasis, zoneBB, description.ZBufferObjectSize, description);
		nlinfo ("Zbuffer object size : %d x %d", _ZBufferObject.LocalZBufferWidth, _ZBufferObject.LocalZBufferHeight);


		// Compute the zbuffer in multi thread
		_ProcessExited = 0;

		// Number of triangle to render per thread
		uint numTriangle = ((uint)obstacles.size () / _ProcessCount) + 1;

		// First triangle for the thread
		uint firstTriangle = 0;

		// Count
		_NumberOfPatchComputed = 0;

		for (uint process=0; process<_ProcessCount; process++)
		{
			// Get list of triangles to render
			uint lastTriangle=firstTriangle+numTriangle;
			if (lastTriangle>obstacles.size ())
				lastTriangle=(uint)obstacles.size ();

			// Create a thread
			CRenderZBuffer *runnable = new CRenderZBuffer (process, this, &description, firstTriangle, lastTriangle - firstTriangle, &obstacles);
			IThread *pThread=IThread::create (runnable);
			runnable->Thread = pThread;

			// New first patch
			firstTriangle = lastTriangle;

			// Launch
			pThread->start();
		}

		// Wait for others processes
		while (_ProcessExited!=_ProcessCount)
		{
			nlSleep (1000);

			// Call the progress callback
			progress ("Render triangles", (float)_NumberOfPatchComputed/(float)obstacles.size());
		}

		// * Save the zbuffer
		uint sample;
		const uint samples = description.SoftShadowSamplesSqrt*description.SoftShadowSamplesSqrt;
#ifdef SAVE_ZBUFFER
		for (sample=0; sample<samples; sample++)
		{
			// *** The zbuffer
			CZBuffer &zbuffer = _ZBufferLandscape[sample];

			string zbufferFilename = SAVE_ZBUFFER"/zbuffer_landscape_" + toString (sample) + ".jpg";

			SaveZBuffer (zbuffer, zbufferFilename.c_str ());
		}

		// Save the object zbuffer
		SaveZBuffer (_ZBufferObject, SAVE_ZBUFFER"/zbuffer_object.jpg");
#endif // SAVE_ZBUFFER

		// *** Filter the zbuffer
		for (sample=0; sample<samples; sample++)
		{
			// For landscape zbuffer, expand the z to neighbor
			FilterZBuffer (_ZBufferLandscape[sample], 5);
		}

		// Change the quadGrid basis
		CMatrix invRayBasis=_RayBasis;
		invRayBasis.invert ();

		// Init the heightfield
		_HeightfieldCellSize=description.HeightfieldCellSize;
		_HeightFieldCellCount=(sint)(description.HeightfieldSize/_HeightfieldCellSize);
		nlassert (_HeightFieldCellCount!=0);
		_OrigineHeightField=zoneBB.getCenter ()-CVector (description.HeightfieldSize/2, description.HeightfieldSize/2, 0);
		_HeightField.resize (_HeightFieldCellCount*_HeightFieldCellCount, -FLT_MAX);

		// Fill the quadGrid and the heightField
		for (triangleId=0; triangleId<size; triangleId++)
		{
			// Progress bar
			if ( (triangleId&0xff) == 0)
				progress ("Build quadtree and heightfield", (float)triangleId/(float)size);

			// Triangle ref
			CZoneLighter::CTriangle& triangle=obstacles[triangleId];

			// Look for the min coordinate, in World Basis
			CVector minv;
			minv.minof (triangle.Triangle.V0, triangle.Triangle.V1);
			minv.minof (minv, triangle.Triangle.V2);

			// Look for the max coordinate, in World Basis
			CVector maxv;
			maxv.maxof (triangle.Triangle.V0, triangle.Triangle.V1);
			maxv.maxof (maxv, triangle.Triangle.V2);

			// Lanscape tri ?
			if (triangle.Flags & CTriangle::Landscape)
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
		uint patchCount=(uint)_PatchInfo.size();

		// Bit array to know if the lumel is shadowed
		if (description.Shadow)
			_ShadowArray.resize (patchCount);

		// A lumel vector by patch
		vector<vector<CLumelDescriptor>	> lumels;
		lumels.resize (patchCount);

		// Build zone information
		buildZoneInformation (landscape,
							  listZone,
							  description);

	}

	// Number of patch
	uint patchCount=(uint)_PatchInfo.size();

	// Reset patch count
	{
		CSynchronized<std::vector<bool> >::CAccessor access (&_PatchComputed);
		access.value().resize (0);
		access.value().resize (patchCount, false);
	}

	// Patch by thread
	uint patchCountByThread = patchCount/_ProcessCount;
	patchCountByThread++;

	// Patch to allocate
	uint firstPatch=0;
	_NumberOfPatchComputed = 0;

	// Reset exited process
	_ProcessExited=0;

	// Set the thread state
	_LastPatchComputed.resize (_ProcessCount);

	// Launch threads
	uint process;
	for (process=0; process<_ProcessCount; process++)
	{
		// Last patch
		uint lastPatch=firstPatch+patchCountByThread;
		lastPatch %= patchCount;

		// Last patch computed
		_LastPatchComputed[process] = firstPatch;

		// Create a thread
		CLightRunnable *runnable = new CLightRunnable (process, this, &description);
		IThread *pThread=IThread::create (runnable);
		runnable->Thread = pThread;

		// New first patch
		firstPatch=lastPatch;

		// Launch
		pThread->start();
	}

	// Wait for others processes
	while (_ProcessExited!=_ProcessCount)
	{
		nlSleep (1000);

		// Call the progress callback
		progress ("Lighting patches", (float)_NumberOfPatchComputed/(float)_PatchInfo.size());
	}

	// Reset old thread mask
	currentThread->setCPUMask (threadMask);

	// overflow ?
	if (_ZBufferOverflow)
		nlwarning ("Error : zbuffer overflow");

	// Progress bar
	progress ("Compute Influences of PointLights", 0.f);

	// Compute PointLight influences on zone.
	// Some precalc.
	compilePointLightRT(description.GridSize, description.GridCellSize, obstacles, description.Shadow);
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
void CZoneLighter::copyTileFlags(CZone &destZone, const CZone &srcZone)
{
	nlassert(destZone.getZoneId() == srcZone.getZoneId());
	nlassert(destZone.getNumPatchs() == srcZone.getNumPatchs());
	for (sint k = 0; k < srcZone.getNumPatchs(); ++k)
	{
		destZone.copyTilesFlags(k, srcZone.getPatch(k));
	}
}

// ***************************************************************************
float CZoneLighter::getSkyContribution(const CVector &pos, const CVector &normal, float skyIntensity) const
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
void CZoneLighter::processCalc (uint process, const CLightDesc& description)
{
	// *** Raytrace each patches

	// Get a patch
	uint patch = getAPatch (process);
	while (patch != 0xffffffff)
	{
		// For each patch
		if (description.Shadow)
		{
			// Lumels
			std::vector<CLumelDescriptor> &lumels=_Lumels[patch];

			// Lumel count
			uint lumelCount=(uint)lumels.size();
			CPatchInfo &patchInfo=_PatchInfo[patch];
			nlassert (patchInfo.Lumels.size()==lumelCount);

			// Resize shadow array
			_ShadowArray[patch].resize (lumelCount);

			// For each lumel
			for (uint lumel=0; lumel<lumelCount; lumel++)
			{
				float factor=0;
				factor = attenuation (lumels[lumel].Position, description);
				patchInfo.Lumels[lumel]=(uint)(factor*255);
			}
		}
		else
		{
			// Lumels
			std::vector<CLumelDescriptor> &lumels=_Lumels[patch];

			// Lumel count
			uint lumelCount=(uint)lumels.size();
			CPatchInfo &patchInfo=_PatchInfo[patch];
			nlassert (patchInfo.Lumels.size()==lumelCount);

			// For each lumel
			for (uint lumel=0; lumel<lumelCount; lumel++)
			{
				// Not shadowed
				patchInfo.Lumels[lumel]=255;
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
				sunContribution=(-lumels[lumel].Normal*_SunDirection)-skyContribution;
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

uint8 CZoneLighter::getMaxPhi (sint s, sint t, sint deltaS, sint deltaT, float heightPos) const
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

bool CZoneLighter::isLumelOnEdgeMustBeOversample (uint patch, uint edge, sint s, sint t, const vector<bool> &binded,
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


sint16 CZoneLighter::_GetNormalDeltaS[4]={ -1, 0, 1, 0 };
sint16 CZoneLighter::_GetNormalDeltaT[4]={ 0, 1, 0, -1 };

// ***************************************************************************

void CZoneLighter::getNormal (const CPatch *pPatch, sint16 lumelS, sint16 lumelT, vector<CPatchUVLocator> &locator,
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

void CZoneLighter::addTriangles (CLandscape &landscape, vector<uint> &listZone, uint order, std::vector<CTriangle>& triangleArray)
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
	uint leavesCount=(uint)leaves.size();

	// Reserve the array
	triangleArray.reserve (triangleArray.size()+leavesCount);

	// Scan each leaves
	for (uint leave=0; leave<leavesCount; leave++)
	{
		// Leave
		const CTessFace *face=leaves[leave];

		// Add a triangle
		triangleArray.push_back (CTriangle (NLMISC::CTriangle (face->VBase->EndPos, face->VLeft->EndPos, face->VRight->EndPos)));
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

void CZoneLighter::addTriangles (const IShape &shape, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
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
		addTriangles (*mesh, mesh->getMeshGeom (), modelMT, triangleArray);
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
			addTriangles (*meshMulti, *geomMesh, modelMT, triangleArray);
		}

		// Dynamic cast
		const CMeshMRMGeom *mrmGeomMesh=dynamic_cast<const CMeshMRMGeom*>(meshGeom);
		if (mrmGeomMesh)
		{
			addTriangles (*meshMulti, *mrmGeomMesh, modelMT, triangleArray);
		}
	}
	// It is a CMeshMultiLod ?
	else if (meshMRM)
	{
		// Get the first lod mesh geom
		addTriangles (*meshMRM, meshMRM->getMeshGeom (), modelMT, triangleArray);
	}
}

// ***************************************************************************

void CZoneLighter::addTriangles (const CMeshBase &meshBase, const CMeshGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
{
	// Get the vertex buffer
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

			// Get the material
			const CMaterial &material = meshBase.getMaterial (meshGeom.getRdrPassMaterial ( block, pass));

			// ** Get the bitmap

			// Texture information, not NULL only if texture is used for alpha test
			CBitmap *texture;
			bool clampU;
			bool clampV;
			uint8 alphaTestThreshold;
			bool doubleSided;
			if (getTexture (material, texture, clampU, clampV, alphaTestThreshold, doubleSided))
			{
				// Dump triangles
				CIndexBufferRead iba;
				primitive.lock (iba);
				if (iba.getFormat() == CIndexBuffer::Indices32)
				{
					const uint32* triIndex= (const uint32*) iba.getPtr ();
					uint numTri=primitive.getNumIndexes ()/3;
					uint tri;
					for (tri=0; tri<numTri; tri++)
					{
						// Vertex
						CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
						CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
						CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

						// UV
						float u[3] = { 0.f };
						float v[3] = { 0.f };
						for (uint i=0; i<3; i++)
						{
							// Get UV coordinates
							const float *uv = (const float*)vba.getTexCoordPointer (triIndex[tri*3+i], 0);
							if (uv)
							{
								// Copy it
								u[i] = uv[0];
								v[i] = uv[1];
							}
						}

						// Make a triangle
						triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), doubleSided, texture, clampU, clampV, u, v,
							alphaTestThreshold));
					}
				}
				else
				{
					const uint16* triIndex=(const uint16*)iba.getPtr ();
					uint numTri=primitive.getNumIndexes ()/3;
					uint tri;
					for (tri=0; tri<numTri; tri++)
					{
						// Vertex
						CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
						CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
						CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

						// UV
						float u[3] = { 0.f };
						float v[3] = { 0.f };
						for (uint i=0; i<3; i++)
						{
							// Get UV coordinates
							const float *uv = (const float*)vba.getTexCoordPointer (triIndex[tri*3+i], 0);
							if (uv)
							{
								// Copy it
								u[i] = uv[0];
								v[i] = uv[1];
							}
						}

						// Make a triangle
						triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), doubleSided, texture, clampU, clampV, u, v,
							alphaTestThreshold));
					}
				}
			}
		}
	}
}

// ***************************************************************************

bool CZoneLighter::getTexture (const CMaterial &material, CBitmap *&result, bool &clampU, bool &clampV, uint8 &alphaTestThreshold, bool &doubleSided)
{
	// Texture information, not NULL only if texture is used for alpha test
	result = NULL;
	clampU = false;
	clampV = false;

	// Alpha test threashold
	float alphaTestThresholdF = material.getAlphaTestThreshold () * 255;
	clamp (alphaTestThresholdF, 0.f, 255.f);
	alphaTestThreshold = (uint8)alphaTestThresholdF;

	// Drawable material ?
	if (material.getBlend ())
		return false;

	// Use alpha test ?
	if (material.getAlphaTest ())
	{
		// Get the texture
		ITexture *texture = material.getTexture (0);

		// Is texture shared ?
		if (texture && texture->supportSharing ())
		{
			// Share name
			string name = texture->getShareName();

			// Texture exist ?
			std::map<string, NLMISC::CBitmap>::iterator ite = _Bitmaps.find (name);
			if (ite != _Bitmaps.end ())
			{
				// Yes
				result = &(ite->second);
			}
			else
			{
				// No, add it
				ite = _Bitmaps.insert (std::map<string, NLMISC::CBitmap>::value_type (name, CBitmap())).first;
				result = &(ite->second);

				// Generate the texture
				texture->generate ();

				// Convert to RGBA
				texture->convertToType (CBitmap::RGBA);

				// Copy it
				*result = *texture;

				// Release the texture
				texture->release ();
			}

			// Wrap flags
			clampU = texture->getWrapS () == ITexture::Clamp;
			clampV = texture->getWrapT () == ITexture::Clamp;
		}
	}

	// Get double sided flag
	doubleSided = material.getDoubleSided ();
	return true;
}

// ***************************************************************************

void CZoneLighter::addTriangles (const CMeshBase &meshBase, const CMeshMRMGeom &meshGeom, const CMatrix& modelMT, std::vector<CTriangle>& triangleArray)
{
	// Get the vertex buffer
	const CVertexBuffer &vb=meshGeom.getVertexBuffer();
	CVertexBufferRead vba;
	vb.lock (vba);

	// For each render pass
	uint numRenderPass=meshGeom.getNbRdrPass(0);
	for (uint pass=0; pass<numRenderPass; pass++)
	{
		// Get the primitive block
		const CIndexBuffer &primitive=meshGeom.getRdrPassPrimitiveBlock ( 0, pass);

		// Get the material
		const CMaterial &material = meshBase.getMaterial (meshGeom.getRdrPassMaterial (0, pass));

		// ** Get the bitmap

		// Texture information, not NULL only if texture is used for alpha test
		CBitmap *texture;
		bool clampU;
		bool clampV;
		uint8 alphaTestThreshold;
		bool doubleSided;
		if (getTexture (material, texture, clampU, clampV, alphaTestThreshold, doubleSided))
		{
			// Dump triangles
			CIndexBufferRead iba;
			primitive.lock (iba);
			const uint32* triIndex= (const uint32 *) iba.getPtr ();
			uint numTri=primitive.getNumIndexes ()/3;
			uint tri;
			for (tri=0; tri<numTri; tri++)
			{
				// Vertex
				CVector v0=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3]));
				CVector v1=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+1]));
				CVector v2=modelMT*(*vba.getVertexCoordPointer (triIndex[tri*3+2]));

				// UV
				float u[3] = { 0.f };
				float v[3] = { 0.f };
				for (uint i=0; i<3; i++)
				{
					// Get UV coordinates
					float *uv = (float*)vba.getTexCoordPointer (triIndex[tri*3+i], 0);
					if (uv)
					{
						// Copy it
						u[i] = uv[0];
						v[i] = uv[1];
					}
				}

				// Make a triangle
				triangleArray.push_back (CTriangle (NLMISC::CTriangle (v0, v1, v2), doubleSided, texture, clampU, clampV, u, v,
					alphaTestThreshold));
			}
		}
	}
}

// ***************************************************************************

void CZoneLighter::excludeAllPatchFromRefineAll (CLandscape &landscape, vector<uint> &listZone, bool exclude)
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

const sint8 CZoneLighter::TriangleIndexes[10][2][3] =
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

// ***************************************************************************

// lumel vertex ID, tesselation edge ID (0 for base<->right, 1 for base<->left), tesselation edge vertex 0, tesselation edge vertex 1
const sint8 CZoneLighter::VertexThanCanBeSnappedOnABorder[8][4] =
{
	{0, 0, 0, 9},
	{1, 0, 9, 4},
	{2, 0, 4, 10},
	{3, 0, 10, 1},
	{0, 1, 0, 11},
	{4, 1, 11, 5},
	{7, 1, 5, 12},
	{9, 1, 12, 2},
};

// ***************************************************************************

// lumel vertex ID, tesselation corner vertex
const sint8 CZoneLighter::VertexThanCanBeSnappedOnACorner[3][2] =
{
	{0, 0},
	{3, 1},
	{9, 2},
};

// ***************************************************************************

void CZoneLighter::buildZoneInformation (CLandscape &landscape, const vector<uint> &listZone, const CLightDesc &lightDesc)
{
	// Bool visit
	vector<vector<uint> > visited;

	// Zone count
	uint zoneCount=(uint)listZone.size();

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
//			_LumelCorners.resize(patchCount);
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
				uint lumelCount = orderS*orderT*16;

				// Resize the lumel descriptor
				CLumelDescriptor descriptor;
				descriptor.Normal.set (0,0,0);
				descriptor.Position.set (0,0,0);
				descriptor.S=0;
				descriptor.T=0;
				_Lumels[patch].resize (lumelCount, descriptor);
				visited[patch].resize (lumelCount, 0);
//				_LumelCorners[patch].resize (lumelCornerCount);


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

	// *** Now tesselate this zone to shadow casters accuracy

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
	uint leavesCount=(uint)leaves.size();
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

			// Does the border are snapped ?
			uint sBase = (uint)floor ((float)orderS * face->PVBase.getS() + 0.5);
			uint tBase = (uint)floor ((float)orderT * face->PVBase.getT() + 0.5);
			uint sLeft = (uint)floor ((float)orderS * face->PVLeft.getS() + 0.5);
			uint tLeft = (uint)floor ((float)orderT * face->PVLeft.getT() + 0.5);
			uint sRight = (uint)floor ((float)orderS * face->PVRight.getS() + 0.5);
			uint tRight = (uint)floor ((float)orderT * face->PVRight.getT() + 0.5);
			bool snapedLeft[2]=
			{
				(sBase == 0) && (sRight == 0),
				(sBase == 0) && (sLeft == 0),
			};
			bool snapedRight[2]=
			{
				(sBase == orderS) && (sRight == orderS),
				(sBase == orderS) && (sLeft == orderS),
			};
			bool snapedTop[2]=
			{
				(tBase == 0) && (tRight == 0),
				(tBase == 0) && (tLeft == 0),
			};
			bool snapedBottom[2]=
			{
				(tBase == orderT) && (tRight == orderT),
				(tBase == orderT) && (tLeft == orderT),
			};
			bool snapedBorder[2]=
			{
				snapedLeft[0]||snapedRight[0]||snapedTop[0]||snapedBottom[0],
				snapedLeft[1]||snapedRight[1]||snapedTop[1]||snapedBottom[1],
			};

			bool snapedCorner[3]=
			{
				((sBase == 0) && ((tBase == 0) || (tBase == orderT))) ||
				((sBase == orderS) && ((tBase == 0) || (tBase == orderT))),
				((sRight == 0) && ((tRight == 0) || (tRight == orderT))) ||
				((sRight == orderS) && ((tRight == 0) || (tRight == orderT))),
				((sLeft == 0) && ((tLeft == 0) || (tLeft == orderT))) ||
				((sLeft == orderS) && ((tLeft == 0) || (tLeft == orderT))),
			};

			// Snap on the border
			uint i;
			for (i=0; i<8; i++)
			{
				// Snaped on left ?
				if (snapedBorder[VertexThanCanBeSnappedOnABorder[i][1]])
				{
					// Compute the border vertex
					interpolatedP[VertexThanCanBeSnappedOnABorder[i][0]] = (pos[VertexThanCanBeSnappedOnABorder[i][2]]
						+ pos[VertexThanCanBeSnappedOnABorder[i][3]])/2;
				}
			}

			// Snap on the corner
			for (i=0; i<3; i++)
			{
				// Snaped on a corner ?
				uint tesselCornerIndex = VertexThanCanBeSnappedOnACorner[i][1];
				if ( snapedCorner[tesselCornerIndex] )
				{
					// Compute the border vertex
					interpolatedP[VertexThanCanBeSnappedOnACorner[i][0]] = pos[tesselCornerIndex];
				}
			}

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

			for (i=0; i<10; i++)
			{
				sint s=(sint)((float)orderS*4*interpolatedS[i]);
				sint t=(sint)((float)orderT*4*interpolatedT[i]);

				if ((s>=0)&&(s<(sint)orderS*4)&&(t>=0)&&(t<(sint)orderT*4))
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
				}
			}
		}
	}

	// *** Now, finalise patch information for shadow source positions

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

		// *** Average position

		// Renormalize
		nlassert (isPowerOf2 (orderS));
		nlassert (isPowerOf2 (orderT));
		uint lumelS=4<<getPowerOf2 (orderS);
		uint lumelT=4<<getPowerOf2 (orderT);

		for (uint t=0; t<lumelT; t++)
		for (uint s=0; s<lumelS; s++)
		{
			// Lumel index
			uint lumelIndex=s+t*lumelS;

			// *** Number of visit
			uint visitedCount=visited[patch][lumelIndex];

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

	// *** Now tesselate this zone to shadow receivers accuracy

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
	leavesCount=(uint)leaves.size();
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
			//nlassert (s>=0);
			nlassert (s<orderS*4);
			//nlassert (t>=0);
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

	// *** Now, finalise patch information

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
void CZoneLighter::computeTileFlagsOnly (CLandscape &landscape, CZone& output, uint zoneToLight, const CLightDesc& description,
						   std::vector<uint> &listZone)
{
	// Zone to light
	_ZoneToLight=zoneToLight;

	// Landscape
	_Landscape=&landscape;


	// Zone count
	uint zoneCount=(uint)listZone.size();

	// For each zone
	for (uint zone=0; zone<zoneCount; zone++)
	{
		// Get num patches
		uint patchCount=landscape.getZone(listZone[zone])->getNumPatchs();

		// Insert zone id
		_ZoneId.insert (map<uint, uint>::value_type (listZone[zone], zone));

		// For each patch
		uint patch;
		for (patch=0; patch<patchCount; patch++)
		{
			// Progress bar
			progress ("Scan all patches", (float)patch/(float)patchCount);

			// This is the zone to light ?
			if (listZone[zone]==_ZoneToLight)
			{
				// unExclude all the patches from refine all
				landscape.excludePatchFromRefineAll (listZone[zone], patch, false);
			}
			else
			{
				// Exclude all the patches from refine all
				landscape.excludePatchFromRefineAll (listZone[zone], patch, true);
			}
		}
	}

	// *** Now tesselate this zone to max accuracy

	// Setup the landscape
	landscape.setThreshold (0);
	landscape.setTileMaxSubdivision (0);

	// Refine all
	progress ("Refine landscape to maximum", 0.5f);
	landscape.refineAll (CVector (0, 0, 0));

	// Get tesselated faces
	std::vector<const CTessFace*> leaves;
	landscape.getTessellationLeaves(leaves);


	// compute only the water states
	if (_WaterShapes.size() != 0) // any water shape in this zone ?
	{
		/// make a quad grid of each water shape
		makeQuadGridFromWaterShapes(landscape.getZone(_ZoneToLight)->getZoneBB().getAABBox());

		/// check for each tile if it is above / below water
		computeTileFlagsForPositionTowardWater(description, leaves);
	}
	else
	{
		setTileFlagsToDefault(leaves);
	}

	/// verify that the zonew and the zonel (output) are compatible
	bool	ok= true;
	CZone	&zonew= *(landscape.getZone(zoneToLight));
	if(zonew.getNumPatchs() == output.getNumPatchs())
	{
		// verify for each patch that the tile array are same
		for(uint i=0;i<(uint)zonew.getNumPatchs();i++)
		{
			const CPatch	&p0= *const_cast<const CZone&>(zonew).getPatch(i);
			const CPatch	&p1= *const_cast<const CZone&>(output).getPatch(i);
			if( p0.getOrderS()!=p1.getOrderS() || p0.getOrderT()!=p1.getOrderT() )
			{
				ok= false;
				break;
			}
		}
	}
	else
		ok= false;

	// can't copy tile flags
	if(!ok)
		throw Exception("The input zonew, and output zonel are too different: not same patchs!!");

	/// copy the tiles flags from the zone to light to the output zone
	copyTileFlags(output, zonew);
}

// ***************************************************************************

CZoneLighter::CLightDesc::CLightDesc ()
{
	SunDirection.set (1, 1, -1);
	GridSize=512;
	GridCellSize=4;
	HeightfieldSize=200;
	HeightfieldCellSize=20;
	SkyContribution=true;
	SkyIntensity=0.25;

	ZBufferLandscapeSize = DEFAULT_ZBUFFER_LANDSCAPE_SIZE;
	ZBufferObjectSize = DEFAULT_ZBUFFER_OBJECT_SIZE;
	SoftShadowJitter = DEFAULT_JITTER;
	SunDistance = DEFAULT_SUN_DISTANCE;
	SunFOV = (float)DEFAULT_SUN_FOV;
	SunCenter = DEFAULT_SUN_CENTER;
	SunRadius = DEFAULT_SUN_RADIUS;
	SoftShadowSamplesSqrt = DEFAULT_SUN_SRQT_SAMPLES;
}

// ***************************************************************************
void CZoneLighter::addLightableShape(IShape *shape, const NLMISC::CMatrix& MT)
{
	CShapeInfo lsi;
	lsi.MT = MT;
	lsi.Shape = shape;
	_LightableShapes.push_back(lsi);
}


// ***************************************************************************
bool CZoneLighter::isLightableShape(IShape &shape)
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
void CZoneLighter::lightShapes(uint zoneID, const CLightDesc& description)
{
	/// compute light for the lightable shapes in the given zone
	if (_LightableShapes.size() == 0) return;

	uint numShapePerThread = 1 + ((uint)_LightableShapes.size() / _ProcessCount);
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

void CZoneLighter::processLightableShapeCalc (uint process,
											  TShapeVect *shapesToLit,
											  uint firstShape,
											  uint lastShape,
											  const CLightDesc& description)
{
	// for each lightable shape
	for (uint k = firstShape; k < lastShape; ++k)
	{
		nlassert(isLightableShape(* (*shapesToLit)[k].Shape)); // make sure it is a lightable shape
		lightSingleShape((*shapesToLit)[k], description, process);
	}
}


// ***************************************************************************
void CZoneLighter::lightSingleShape(CShapeInfo &si, const CLightDesc& description, uint cpu)
{
	/// we compute the lighting for one single shape
	if (dynamic_cast<CWaterShape *>(si.Shape))
	{
		lightWater(* static_cast<CWaterShape *>(si.Shape), si.MT, description, cpu);
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
void CZoneLighter::lightWater(CWaterShape &ws, const CMatrix &MT, const CLightDesc& description, uint cpu)
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

		CObjectVector<uint8> &pixs8 = diffuseTex->getPixels();
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
					factor = attenuation (pos, description);
				}
				else
				{
					factor = - NLMISC::CVector::K * description.SunDirection;
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
			catch (const NLMISC::Exception &)
			{
				nlwarning("Zone lighter : while lighting a water shape, writing %s failed! ", texFileName.c_str());
			}
		}
	}
	catch(const NLMISC::Exception &e)
	{
		nlwarning("Water shape lighting failed !");
		nlwarning(e.what());
	}
}

// ***********************************************************
void CZoneLighter::addWaterShape(CWaterShape *shape, const NLMISC::CMatrix &MT)
{
	/// make sure it hasn't been inserted twice
	CShapeInfo ci;
	ci.Shape = shape;
	ci.MT = MT;
	_WaterShapes.push_back(ci);
}

// ***********************************************************
void CZoneLighter::makeQuadGridFromWaterShapes(NLMISC::CAABBox zoneBBox)
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


	uint count = 0, totalCount = (uint)_WaterShapes.size();

	/// now, insert all water shapes
	for (TShapeVect::iterator it = _WaterShapes.begin(); it != _WaterShapes.end(); ++it, ++count)
	{
		/// get the current shape bbox in the world
		it->Shape->getAABBox(tmpBox);
		NLMISC::CAABBox currBB = NLMISC::CAABBox::transformAABBox(it->MT, tmpBox);

		/// test if it intesect the zone bbox
		if (zoneBBox.intersect(currBB))
		{
			_WaterShapeQuadGrid.insert(currBB.getMin(), currBB.getMax(), *it);
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
CZoneLighter::CPointLightRT::CPointLightRT()
{
	RefCount= 0;
}


// ***************************************************************************
bool	CZoneLighter::CPointLightRT::testRaytrace(const CVector &v)
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
void			CZoneLighter::addStaticPointLight(const CPointLightNamed &pln)
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
void			CZoneLighter::compilePointLightRT(uint gridSize, float gridCellSize, std::vector<CTriangle>& obstacles, bool doShadow)
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
		// Point lights ?
		if (!_StaticPointLights.empty ())
		{
			// For all obstacles, Fill a quadGrid.
			// ===========
			CQuadGrid<CTriangle*>	obstacleGrid;
			obstacleGrid.create(gridSize, gridCellSize);
			uint	size= (uint)obstacles.size();
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
					// Select only obstacle Faces around the light. Other are not useful
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

						// Triangle bounding box
						CAABBox	triBbox;
						triBbox.setCenter (tri.Triangle.V0);
						triBbox.extend (tri.Triangle.V1);
						triBbox.extend (tri.Triangle.V2);

						// Triangle in the light
						if (triBbox.intersect (bbox))
						{
							// Test BackFace culling. Only faces which are BackFace the point light are inserted.
							// This is to avoid AutoOccluding problems
							if( tri.getPlane() * plRT.BSphere.Center < 0)
							{
								// Insert the triangle in the CubeGrid
								plRT.FaceCubeGrid.insert( tri.Triangle, &tri);
							}
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
bool	CZoneLighter::CPredPointLightToPoint::operator() (CPointLightRT *pla, CPointLightRT *plb) const
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
void			CZoneLighter::processZonePointLightRT(vector<CPointLightNamed> &listPointLight)
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

// ***********************************************************
// ***********************************************************
// TileFlagsForPositionTowardWater
// ***********************************************************
// ***********************************************************


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

// ***********************************************************
void CZoneLighter::computeTileFlagsForPositionTowardWater(const CLightDesc &lightDesc,
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

	uint triCount = 0, totalTriCount = (uint)tessFaces.size();

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

		/// test whether we've seen face(s) from this tile before
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

	uint tileCount = 0, totalTileCount = (uint)tiles.size();

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
			CWaterShape		*waterShape= safe_cast<CWaterShape*>((*qgIt).Shape);
			waterShape->getShapeInWorldSpace(waterPoly, (*qgIt).MT);
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

// ***********************************************************

void CZoneLighter::setTileFlagsToDefault(std::vector<const CTessFace*> &tessFaces)
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

// ***********************************************************

uint CZoneLighter::getAPatch (uint process)
{
	// Accessor
	CSynchronized<std::vector<bool> >::CAccessor access (&_PatchComputed);

	// Current index
	uint index = _LastPatchComputed[process];
	uint firstIndex = index;

	nlassert(index < _PatchInfo.size());

	if (access.value().size() == 0)
		// no more patches
		return 0xffffffff;

	while (access.value()[index])
	{
		// Next patch
		index++;

		// Last patch ?
		if (index == _PatchInfo.size())
			index = 0;

		// First ?
		if (firstIndex == index)
			// no more patches
			return 0xffffffff;
	}

	// Visited
	access.value()[index] = true;

	// Last index
	_LastPatchComputed[process] = index;
	_NumberOfPatchComputed++;

	// Return the index
	return index;
}

// ***********************************************************

float CZoneLighter::attenuation (const CVector &pos, const CZoneLighter::CLightDesc &description)
{
	// Clipped ?

	// *** Landscape attenuation

	// Current value
	float averageAttenuation = 0;
	float randomSum = 0;

	// For each sample
	uint sample;
	const uint samples = description.SoftShadowSamplesSqrt*description.SoftShadowSamplesSqrt;
	for (sample=0; sample<samples; sample++)
	{
		// The zbuffer
		CZBuffer &zbuffer = _ZBufferLandscape[sample];

		// Get position in z buffer
		CVector zPos;
		transformVectorToZBuffer (zbuffer, pos, zPos);

		// Get the z
		float random = (float)_Random.rand () * description.SoftShadowJitter + _Random.RandMax * (1.f - description.SoftShadowJitter);
		averageAttenuation += random * testZPercentageCloserFilter (zPos.x-(float)zbuffer.LocalZBufferXMin, zPos.y-(float)zbuffer.LocalZBufferYMin, zPos.z, zbuffer, description, _ZBufferOverflow);
		randomSum += random;
	}

	// Average landscape attenuation
	averageAttenuation /= randomSum;



	// *** Attenuation in the object zbuffer

	// Get position in z buffer
	CVector zPos;
	transformVectorToZBuffer (_ZBufferObject, pos, zPos);

	const float objectAttenuation = testZPercentageCloserFilter (zPos.x-(float)_ZBufferObject.LocalZBufferXMin, zPos.y-(float)_ZBufferObject.LocalZBufferYMin, zPos.z, _ZBufferObject, description, _ZBufferOverflow);


	// *** Return the min of the both
	return std::min (objectAttenuation, averageAttenuation);
}

// ***********************************************************


