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

#include "nel/3d/ps_face_look_at.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"
#include "nel/3d/ps_iterator.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/fast_floor.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


/** vector giving the orientation of look at
  */
struct CLookAtAlign
{
	CVector I;
	CVector K;
};


uint64 PSLookAtRenderTime = 0;


//////////////////////////////////
// CPSFaceLookAt implementation //
//////////////////////////////////


/** Well, we could have put a method template in CPSFaceLookAt, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSFaceLookAtHelper
{
public:
	/** compute orientation vectors depending on speed
      */
	template <class T>
	static void computeOrientationVectors(T speedIt, const CVector &I, const CVector &K, CLookAtAlign *dest, uint size)
	{
		NL_PS_FUNC(CPSFaceLookAtHelper_computeOrientationVectors)
		nlassert(size > 0);
		const CLookAtAlign *endDest = dest + size;
		do
		{
			// tmp unoptimized slow version
			CVector normedSpeed = (*speedIt).normed();
			float iProj = normedSpeed * I;
			float kProj = normedSpeed * K;
			dest->I = iProj * I + kProj * K;
			dest->K = (- kProj * I + iProj * K).normed();
			++ speedIt;
			++ dest;
		}
		while(dest != endDest);
	}

	/** Draw look at and align them on motion
	  */
	template <class T>
	static void drawLookAtAlignOnMotion(T it, T speedIt, CPSFaceLookAt &la, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;
		nlassert(la._Owner);
		IDriver *driver = la.getDriver();

		if (la._ColorScheme)
		{
			la._ColorScheme->setColorType(driver->getVertexColorFormat());
		}

		CVertexBuffer &vb = la.getNeededVB(*driver);
		la.updateMatBeforeRendering(driver, vb);

		la._Owner->incrementNbDrawnParticles(size); // for benchmark purpose
		la.setupDriverModelMatrix();
		//driver->activeVertexBuffer(vb);
		const CVector I = la.computeI();
		const CVector K = la.computeK();
		const float *rotTable = CPSRotated2DParticle::getRotTable();
		// for each the particle can be constantly rotated or have an independant rotation for each particle
		// number of face left, and number of face to process at once
		uint32 leftToDo = size, toProcess;
		float pSizes[CPSQuad::quadBufSize]; // the sizes to use
		float pSecondSizes[CPSQuad::quadBufSize]; // the second sizes to use
		uint8 laAlignRaw[sizeof(CLookAtAlign) * CPSQuad::quadBufSize]; // orientation computed from motion for each particle
		CLookAtAlign *laAlign = (CLookAtAlign *) laAlignRaw; // cast to avoid unilined ctor calls
		float *currentSize;
		uint32 currentSizeStep = la._SizeScheme ? 1 : 0;
		// point the vector part in the current vertex
		uint8 *ptPos;
		// strides to go from one vertex to another one
		const uint32 stride = vb.getVertexSize();
		if (!la._Angle2DScheme)
		{
			// constant rotation case
			do
			{
				toProcess = leftToDo <= (uint32) CPSQuad::quadBufSize ? leftToDo : (uint32) CPSQuad::quadBufSize;
				vb.setNumVertices(4 * toProcess);
				// restart at the beginning of the vertex buffer
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				ptPos = (uint8 *) vba.getVertexCoordPointer();
				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size- leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}
				computeOrientationVectors(speedIt, I, K, laAlign, toProcess);
				speedIt = speedIt + toProcess;
				const CLookAtAlign *currAlign = laAlign;

				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep, *driver);
				T endIt = it + toProcess;
				if (!la._IndependantSizes)
				{
					const uint32 tabIndex = (((uint32) la._Angle2D) & 0xff) << 2;
					CVector v1;
					CVector v2;
					// TODO : optimize if necessary
					while (it != endIt)
					{
						v1 = rotTable[tabIndex] * currAlign->I + rotTable[tabIndex + 1] * currAlign->K;
						v2 = rotTable[tabIndex + 2] * currAlign->I + rotTable[tabIndex + 3] * currAlign->K;
						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;
						ptPos += stride;

						++it;
						++currAlign;
						currentSize += currentSizeStep;
					}
				}
				else // independant sizes
				{
					float *currentSize2;
					float secondSize;
					uint32 currentSizeStep2;
					if (la._SecondSize.getSizeScheme())
					{
						currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
						currentSizeStep2 = 1;
					}
					else
					{
						secondSize = la._SecondSize.getSize();
						currentSize2 = &secondSize;
						currentSizeStep2 = 0;
					}
					CVector v1;
					CVector v2;
					// TODO : optimize if necessary
					while (it != endIt)
					{
						v1 = CPSUtil::getCos((sint32) la._Angle2D) * currAlign->I  + CPSUtil::getSin((sint32) la._Angle2D) * currAlign->K;
						v2 = - CPSUtil::getSin((sint32) la._Angle2D) * currAlign->I + CPSUtil::getCos((sint32) la._Angle2D) * currAlign->K;
						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;
						++it;
						++currAlign;
						currentSize += currentSizeStep;
						currentSize2 += currentSizeStep2;
					}
				}
				// uint64 startTick = NLMISC::CTime::getPerformanceTime();
				vba.unlock();
				driver->activeVertexBuffer(vb);
				driver->renderRawQuads(la._Mat, 0, toProcess);
				// PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;
				leftToDo -= toProcess;
			}
			while (leftToDo);
		}
		else
		{
			float pAngles[CPSQuad::quadBufSize]; // the angles to use
			float *currentAngle;
			do
			{
				toProcess = leftToDo <= (uint32) CPSQuad::quadBufSize ? leftToDo : (uint32) CPSQuad::quadBufSize;
				vb.setNumVertices(4 * toProcess);
				// restart at the beginning of the vertex buffer
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				ptPos = (uint8 *) vba.getVertexCoordPointer();
				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size - leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}
				computeOrientationVectors(speedIt, I, K, laAlign, toProcess);
				speedIt = speedIt + toProcess;
				const CLookAtAlign *currAlign = laAlign;
				currentAngle = (float *) la._Angle2DScheme->make(la._Owner, size - leftToDo, pAngles, sizeof(float), toProcess, true, srcStep);
				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep, *driver);
				T endIt = it + toProcess;
				CVector v1, v2;
				NLMISC::OptFastFloorBegin();
				if (!la._IndependantSizes)
				{
					while (it != endIt)
					{
						const uint32 tabIndex = ((NLMISC::OptFastFloor(*currentAngle)) & 0xff) << 2;
						// lets avoid some ctor calls
						v1.x = *currentSize * (rotTable[tabIndex] * currAlign->I.x + rotTable[tabIndex + 1] * currAlign->K.x);
						v1.y = *currentSize * (rotTable[tabIndex] * currAlign->I.y + rotTable[tabIndex + 1] * currAlign->K.y);
						v1.z = *currentSize * (rotTable[tabIndex] * currAlign->I.z + rotTable[tabIndex + 1] * currAlign->K.z);

						v2.x = *currentSize * (rotTable[tabIndex + 2] * currAlign->I.x + rotTable[tabIndex + 3] * currAlign->K.x);
						v2.y = *currentSize * (rotTable[tabIndex + 2] * currAlign->I.y + rotTable[tabIndex + 3] * currAlign->K.y);
						v2.z = *currentSize * (rotTable[tabIndex + 2] * currAlign->I.z + rotTable[tabIndex + 3] * currAlign->K.z);

						CHECK_VERTEX_BUFFER(vb, ptPos);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride3);

						((CVector *) ptPos)->x  = (*it).x  + v1.x;
						((CVector *) ptPos)->y  = (*it).y  + v1.y;
						((CVector *) ptPos)->z = (*it).z  + v1.z;
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  + v2.x;
						((CVector *) ptPos)->y  = (*it).y  + v2.y;
						((CVector *) ptPos)->z = (*it).z  + v2.z;
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v1.x;
						((CVector *) ptPos)->y  = (*it).y  - v1.y;
						((CVector *) ptPos)->z = (*it).z  - v1.z;
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v2.x;
						((CVector *) ptPos)->y  = (*it).y  - v2.y;
						((CVector *) ptPos)->z = (*it).z  - v2.z;
						ptPos += stride;

						++it;
						++ currAlign;
						currentSize += currentSizeStep;
						++currentAngle;
					}
				}
				else // independant size, and non-constant rotation
				{

					float *currentSize2;
					float secondSize;
					uint32 currentSizeStep2;
					if (la._SecondSize.getSizeScheme())
					{
						currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
						currentSizeStep2 = 1;
					}
					else
					{
						secondSize = la._SecondSize.getSize();
						currentSize2 = &secondSize;
						currentSizeStep2 = 0;
					}

					float cosAngle, sinAngle;
					while (it != endIt)
					{
						cosAngle = CPSUtil::getCos((sint32) *currentAngle);
						sinAngle = CPSUtil::getSin((sint32) *currentAngle);
						v1 = cosAngle * currAlign->I  + sinAngle * currAlign->K;
						v2 = - sinAngle * currAlign->I + cosAngle * currAlign->K;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;
						++it;
						++currentAngle;
						++ currAlign;
						currentSize  += currentSizeStep;
						currentSize2 += currentSizeStep2;
					}
				}
				NLMISC::OptFastFloorEnd();
				//tmp
				// uint64 startTick = NLMISC::CTime::getPerformanceTime();
				vba.unlock();
				driver->activeVertexBuffer(vb);
				driver->renderRawQuads(la._Mat, 0, toProcess);
				// PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;
				leftToDo -= toProcess;
			}
			while (leftToDo);
		}
		PARTICLES_CHECK_MEM;
	}

	/** render look at, but dont align on motion
	  */
	template <class T>
	static void drawLookAt(T it, T speedIt, CPSFaceLookAt &la, uint size, uint32 srcStep)
	{
		//uint64 startTick = NLMISC::CTime::getPerformanceTime();
		PARTICLES_CHECK_MEM;
		nlassert(la._Owner);
		IDriver *driver = la.getDriver();

		if (la._ColorScheme)
		{
			la._ColorScheme->setColorType(driver->getVertexColorFormat());
		}

		CVertexBuffer &vb = la.getNeededVB(*driver);
		la.updateMatBeforeRendering(driver, vb);

		la._Owner->incrementNbDrawnParticles(size); // for benchmark purpose
		la.setupDriverModelMatrix();
		//driver->activeVertexBuffer(vb);
		CVector I;
		CVector J;
		CVector K;
		if (!la._AlignOnZAxis)
		{
			I = la.computeI();
			J = la.computeJ();
			K = la.computeK();
		}
		else
		{
			I = la.computeIWithZAxisAligned();
			K = la.computeKWithZAxisAligned();
			J = K ^ I;
		}
		const float *rotTable = CPSRotated2DParticle::getRotTable();
		// for each the particle can be constantly rotated or have an independant rotation for each particle
		// number of face left, and number of face to process at once
		uint32 leftToDo = size, toProcess;
		float pSizes[CPSQuad::quadBufSize]; // the sizes to use
		float pSecondSizes[CPSQuad::quadBufSize]; // the second sizes to use
		float *currentSize;
		uint32 currentSizeStep = la._SizeScheme ? 1 : 0;
		// point the vector part in the current vertex
		uint8 *ptPos;
		// strides to go from one vertex to another one
		const uint32 stride = vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2;
		//PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;
		if (!la._Angle2DScheme)
		{
			// constant rotation case
			do
			{
				toProcess = leftToDo <= (uint32) CPSQuad::quadBufSize ? leftToDo : (uint32) CPSQuad::quadBufSize;
				vb.setNumVertices(4 * toProcess);
				// restart at the beginning of the vertex buffer
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				ptPos = (uint8 *) vba.getVertexCoordPointer();
				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size- leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}

				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep, *driver);
				T endIt = it + toProcess;
				if (la._MotionBlurCoeff == 0.f)
				{
					if (!la._IndependantSizes)
					{
						const uint32 tabIndex = (((uint32) la._Angle2D) & 0xff) << 2;
						const CVector v1 = rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K;
						const CVector v2 = rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K;
						if (currentSizeStep)
						{
							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x;
								((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y;
								((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + *currentSize * v2.x;
								((CVector *) ptPos)->y = (*it).y  + *currentSize * v2.y;
								((CVector *) ptPos)->z = (*it).z  + *currentSize * v2.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x;
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y;
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;
								ptPos += stride;

								++it;
								currentSize += currentSizeStep;
							}
						}
						else
						{
							// constant size
							const CVector myV1 = *currentSize * v1;
							const CVector myV2 = *currentSize * v2;

							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + myV1.x;
								((CVector *) ptPos)->y = (*it).y  + myV1.y;
								((CVector *) ptPos)->z = (*it).z  + myV1.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + myV2.x;
								((CVector *) ptPos)->y = (*it).y  + myV2.y;
								((CVector *) ptPos)->z = (*it).z  + myV2.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - myV1.x;
								((CVector *) ptPos)->y = (*it).y  - myV1.y;
								((CVector *) ptPos)->z = (*it).z  - myV1.z;
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - myV2.x;
								((CVector *) ptPos)->y = (*it).y  - myV2.y;
								((CVector *) ptPos)->z = (*it).z  - myV2.z;
								ptPos += stride;
								++it;
							}
						}
					}
					else // independant sizes
					{
						const CVector v1 = CPSUtil::getCos((sint32) la._Angle2D) * I  + CPSUtil::getSin((sint32) la._Angle2D) * K;
						const CVector v2 = - CPSUtil::getSin((sint32) la._Angle2D) * I + CPSUtil::getCos((sint32) la._Angle2D) * K;

						float *currentSize2;
						float secondSize;
						uint32 currentSizeStep2;
						if (la._SecondSize.getSizeScheme())
						{
							currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
							currentSizeStep2 = 1;
						}
						else
						{
							secondSize = la._SecondSize.getSize();
							currentSize2 = &secondSize;
							currentSizeStep2 = 0;
						}


						while (it != endIt)
						{
							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;
							((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;
							((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;
							((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;
							((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;
							++it;
							currentSize += currentSizeStep;
							currentSize2 += currentSizeStep2;
						}
					}
					//tmp
					//uint64 startTick = NLMISC::CTime::getPerformanceTime();
					vba.unlock();
					driver->activeVertexBuffer(vb);
					driver->renderRawQuads(la._Mat, 0, toProcess);
					//PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;
				}
				else
				{
					// perform motion, blur, we need an iterator on speed
					// independant sizes and rotation not supported for now with motion blur
					const CVector v1 = I + K;
					const CVector v2 = K - I;
					CVector startV, endV, mbv1, mbv1n, mbv12, mbv2;
					// norme of the v1 vect
					float n;
					const float epsilon  = 10E-5f;
					const float normEpsilon  = 10E-6f;

					CMatrix tMat = la.getViewMat()  *  la._Owner->getLocalToWorldMatrix();

					while (it != endIt)
					{
						// project the speed in the projection plane
						// this give us the v1 vect
						startV = tMat * *it ;
						endV = tMat * (*it + *speedIt);
						if (startV.y > epsilon || endV.y > epsilon)
						{
							if (startV.y < epsilon)
							{
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									startV = endV + (endV.y - epsilon) / (endV.y - startV.y) * (startV - endV);
								}
								startV.y = epsilon;
							}
							else if (endV.y < epsilon)
							{
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									endV = startV + (startV.y - epsilon) / (startV.y - endV.y) * (endV - startV);
								}
								endV.y = epsilon;
							}

							mbv1 = (startV.x / startV.y - endV.x / endV.y) * I
											+ (startV.z / startV.y - endV.z / endV.y) * K ;

							n = mbv1.norm();
							if (n > la._Threshold)
							{
								mbv1 *= la._Threshold / n;
								n = la._Threshold;
							}
							if (n > normEpsilon)
							{
								mbv1n = mbv1 / n;
								mbv2 = *currentSize * (J ^ mbv1n);
								mbv12 = -*currentSize * mbv1n;
								mbv1 *= *currentSize * (1 + la._MotionBlurCoeff * n * n) / n;

								*(CVector *) ptPos = *it - mbv2;
								*(CVector *) (ptPos + stride) = *it  + mbv1;
								*(CVector *) (ptPos + stride2) = *it + mbv2;
								*(CVector *) (ptPos + stride3) = *it + mbv12;


								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - mbv2.x;
								((CVector *) ptPos)->y = (*it).y  - mbv2.y;
								((CVector *) ptPos)->z = (*it).z  - mbv2.z;

								CHECK_VERTEX_BUFFER(vb, ptPos + stride);
								((CVector *) (ptPos + stride))->x = (*it).x  + mbv1.x;
								((CVector *) (ptPos + stride))->y = (*it).y  + mbv1.y;
								((CVector *) (ptPos + stride))->z = (*it).z  + mbv1.z;

								CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
								((CVector *) (ptPos + stride2))->x = (*it).x  + mbv2.x;
								((CVector *) (ptPos + stride2))->y = (*it).y  + mbv2.y;
								((CVector *) (ptPos + stride2))->z = (*it).z  + mbv2.z;


								CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
								((CVector *) (ptPos + stride3))->x = (*it).x  + mbv12.x;
								((CVector *) (ptPos + stride3))->y = (*it).y  + mbv12.y;
								((CVector *) (ptPos + stride3))->z = (*it).z  + mbv12.z;

							}
							else // speed too small, we must avoid imprecision
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;

								CHECK_VERTEX_BUFFER(vb, ptPos + stride);
								((CVector *) (ptPos + stride))->x = (*it).x  + *currentSize * v1.x;
								((CVector *) (ptPos + stride))->y = (*it).y  + *currentSize * v1.y;
								((CVector *) (ptPos + stride))->z = (*it).z  + *currentSize * v1.z;

								CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
								((CVector *) (ptPos + stride2))->x = (*it).x  + *currentSize * v2.x;
								((CVector *) (ptPos + stride2))->y = (*it).y  + *currentSize * v2.y;
								((CVector *) (ptPos + stride2))->z = (*it).z  + *currentSize * v2.z;


								CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
								((CVector *) (ptPos + stride3))->x = (*it).x  - *currentSize * v1.x;
								((CVector *) (ptPos + stride3))->y = (*it).y  - *currentSize * v1.y;
								((CVector *) (ptPos + stride3))->z = (*it).z  - *currentSize * v1.z;
							}
						}
						else
						{

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;

							CHECK_VERTEX_BUFFER(vb, ptPos + stride);
							((CVector *) (ptPos + stride))->x = (*it).x  + *currentSize * v1.x;
							((CVector *) (ptPos + stride))->y = (*it).y  + *currentSize * v1.y;
							((CVector *) (ptPos + stride))->z = (*it).z  + *currentSize * v1.z;

							CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
							((CVector *) (ptPos + stride2))->x = (*it).x  + *currentSize * v2.x;
							((CVector *) (ptPos + stride2))->y = (*it).y  + *currentSize * v2.y;
							((CVector *) (ptPos + stride2))->z = (*it).z  + *currentSize * v2.z;


							CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
							((CVector *) (ptPos + stride3))->x = (*it).x  - *currentSize * v1.x;
							((CVector *) (ptPos + stride3))->y = (*it).y  - *currentSize * v1.y;
							((CVector *) (ptPos + stride3))->z = (*it).z  - *currentSize * v1.z;
						}

						ptPos += stride4;
						++it;
						++speedIt;
						currentSize += currentSizeStep;
					}
					//uint64 startTick = NLMISC::CTime::getPerformanceTime();
					vba.unlock();
					driver->activeVertexBuffer(vb);
					driver->renderRawQuads(la._Mat, 0, toProcess);
					//PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;
				}
				leftToDo -= toProcess;
			}
			while (leftToDo);
		}
		else
		{
			float pAngles[CPSQuad::quadBufSize]; // the angles to use
			float *currentAngle;
			do
			{
				toProcess = leftToDo <= (uint32) CPSQuad::quadBufSize ? leftToDo : (uint32) CPSQuad::quadBufSize;
				vb.setNumVertices(4 * toProcess);
				// restart at the beginning of the vertex buffer
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				ptPos = (uint8 *) vba.getVertexCoordPointer();
				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size - leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}
				currentAngle = (float *) la._Angle2DScheme->make(la._Owner, size - leftToDo, pAngles, sizeof(float), toProcess, true, srcStep);
				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep, *driver);
				/*
				static bool fakeColors = false;
				if (fakeColors)
				{
					uint8 *col = (uint8 *) vba.getColorPointer();
					uint left = toProcess;
					while(left--)
					{
						* (CRGBA *) col = CRGBA::Red;
						col += stride;
						* (CRGBA *) col = CRGBA::Red;
						col += stride;
						* (CRGBA *) col = CRGBA::Red;
						col += stride;
						* (CRGBA *) col = CRGBA::Red;
						col += stride;
					}
				}
				*/
				//nlinfo("======= %s", la._Name.c_str());
				T endIt = it + toProcess;
				CVector v1, v2;
				NLMISC::OptFastFloorBegin();
				if (!la._IndependantSizes)
				{
					while (it != endIt)
					{
						const uint32 tabIndex = ((NLMISC::OptFastFloor(*currentAngle)) & 0xff) << 2;
						// lets avoid some ctor calls
						v1.x = *currentSize * (rotTable[tabIndex] * I.x + rotTable[tabIndex + 1] * K.x);
						v1.y = *currentSize * (rotTable[tabIndex] * I.y + rotTable[tabIndex + 1] * K.y);
						v1.z = *currentSize * (rotTable[tabIndex] * I.z + rotTable[tabIndex + 1] * K.z);

						v2.x = *currentSize * (rotTable[tabIndex + 2] * I.x + rotTable[tabIndex + 3] * K.x);
						v2.y = *currentSize * (rotTable[tabIndex + 2] * I.y + rotTable[tabIndex + 3] * K.y);
						v2.z = *currentSize * (rotTable[tabIndex + 2] * I.z + rotTable[tabIndex + 3] * K.z);

						CHECK_VERTEX_BUFFER(vb, ptPos);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride3);

						((CVector *) ptPos)->x  = (*it).x  + v1.x;
						((CVector *) ptPos)->y  = (*it).y  + v1.y;
						((CVector *) ptPos)->z = (*it).z  + v1.z;
						//nlinfo("** %f, %f, %f", ((CVector *) ptPos)->x, ((CVector *) ptPos)->y, ((CVector *) ptPos)->z);
						ptPos += stride;



						((CVector *) ptPos)->x  = (*it).x  + v2.x;
						((CVector *) ptPos)->y  = (*it).y  + v2.y;
						((CVector *) ptPos)->z = (*it).z  + v2.z;
						//nlinfo("%f, %f, %f", ((CVector *) ptPos)->x, ((CVector *) ptPos)->y, ((CVector *) ptPos)->z);
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v1.x;
						((CVector *) ptPos)->y  = (*it).y  - v1.y;
						((CVector *) ptPos)->z = (*it).z  - v1.z;
						//nlinfo("%f, %f, %f", ((CVector *) ptPos)->x, ((CVector *) ptPos)->y, ((CVector *) ptPos)->z);
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v2.x;
						((CVector *) ptPos)->y  = (*it).y  - v2.y;
						((CVector *) ptPos)->z = (*it).z  - v2.z;
						//nlinfo("%f, %f, %f", ((CVector *) ptPos)->x, ((CVector *) ptPos)->y, ((CVector *) ptPos)->z);
						ptPos += stride;

						++it;
						currentSize += currentSizeStep;
						++currentAngle;
					}
				}
				else // independant size, and non-constant rotation
				{

					float *currentSize2;
					float secondSize;
					uint32 currentSizeStep2;
					if (la._SecondSize.getSizeScheme())
					{
						currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
						currentSizeStep2 = 1;
					}
					else
					{
						secondSize = la._SecondSize.getSize();
						currentSize2 = &secondSize;
						currentSizeStep2 = 0;
					}

					float cosAngle, sinAngle;
					while (it != endIt)
					{
						cosAngle = CPSUtil::getCos((sint32) *currentAngle);
						sinAngle = CPSUtil::getSin((sint32) *currentAngle);
						v1 = cosAngle * I  + sinAngle * K;
						v2 = - sinAngle * I + cosAngle * K;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;
						++it;
						++currentAngle;
						currentSize  += currentSizeStep;
						currentSize2 += currentSizeStep2;
					}
				}
				NLMISC::OptFastFloorEnd();
				//tmp
				// uint64 startTick = NLMISC::CTime::getPerformanceTime();
				vba.unlock();
				driver->activeVertexBuffer(vb);
				driver->renderRawQuads(la._Mat, 0, toProcess);
				//PSLookAtRenderTime += NLMISC::CTime::getPerformanceTime() - startTick;*/
				leftToDo -= toProcess;
			}
			while (leftToDo);
		}
		PARTICLES_CHECK_MEM;
	}
};

///===========================================================================================
void CPSFaceLookAt::draw(bool opaque)
{
//	if (!FilterPS[2]) return;
	NL_PS_FUNC(CPSFaceLookAt_draw)
	PARTICLES_CHECK_MEM;
	if (!_Owner->getSize()) return;
	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);
	if (!numToProcess) return;

	if (step == (1 << 16))
	{
		if (!_AlignOnMotion)
		{
			CPSFaceLookAtHelper::drawLookAt(_Owner->getPos().begin(),
											_Owner->getSpeed().begin(),
											*this,
											numToProcess,
											step
										   );
		}
		else
		{
			CPSFaceLookAtHelper::drawLookAtAlignOnMotion(_Owner->getPos().begin(),
											             _Owner->getSpeed().begin(),
											             *this,
											             numToProcess,
											             step
										                );
		}
	}
	else
	{
		if (!_AlignOnMotion)
		{
			CPSFaceLookAtHelper::drawLookAt(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
											TIteratorVectStep1616(_Owner->getSpeed().begin(), 0, step),
											*this,
											numToProcess,
											step
										   );
		}
		else
		{
			CPSFaceLookAtHelper::drawLookAtAlignOnMotion(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
													     TIteratorVectStep1616(_Owner->getSpeed().begin(), 0, step),
											             *this,
											             numToProcess,
											             step
										                );
		}
	}
	PARTICLES_CHECK_MEM;
}

///===========================================================================================
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex) : CPSQuad(tex),
                                                        _MotionBlurCoeff(0.f),
														_Threshold(0.5f),
														_IndependantSizes(false),
														_AlignOnMotion(false),
														_AlignOnZAxis(false)
{
	NL_PS_FUNC(CPSFaceLookAt_CPSFaceLookAt)
	_SecondSize.Owner = this;
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("LookAt");
}

///===========================================================================================
void CPSFaceLookAt::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSFaceLookAt_newElement)
	CPSQuad::newElement(info);
	newAngle2DElement(info);
}

///===========================================================================================
void CPSFaceLookAt::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSFaceLookAt_deleteElement)
	CPSQuad::deleteElement(index);
	deleteAngle2DElement(index);
}

///===========================================================================================
void CPSFaceLookAt::resize(uint32 capacity)
{
	NL_PS_FUNC(CPSFaceLookAt_resize)
	nlassert(capacity < (1 << 16));
	CPSQuad::resize(capacity);
	resizeAngle2D(capacity);
}


///===========================================================================================
void CPSFaceLookAt::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSFaceLookAt_serial)
	// version 4 : added 'align on z-axis' flag
	// version 3 : added 'align on motion' flag
	sint ver = f.serialVersion(4);
	CPSQuad::serial(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial(_MotionBlurCoeff);
	if (_MotionBlurCoeff != 0)
	{
		f.serial(_Threshold);
	}
	if (ver > 1)
	{
		f.serial(_IndependantSizes);
		if (_IndependantSizes)
		{
			_SecondSize.serialSizeScheme(f);
		}
	}
	if (ver >= 3)
	{
		f.serial(_AlignOnMotion);
	}
	if (ver >= 4)
	{
		f.serial(_AlignOnZAxis);
	}
	if (f.isReading())
	{
		init();
	}
}

} // NL3D
