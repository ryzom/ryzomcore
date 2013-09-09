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

#include "nel/misc/vector_2f.h"
#include "nel/misc/vector_h.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/water_model.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/water_height_map.h"
#include "nel/3d/dru.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/texture_emboss.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/water_env_map.h"


using NLMISC::CVector2f;


namespace NL3D {

// for normal rendering
CMaterial CWaterModel::_WaterMat;
// for simple rendering
CMaterial CWaterModel::_SimpleWaterMat;
const uint WATER_MODEL_DEFAULT_NUM_VERTICES = 5000;

NLMISC::CRefPtr<IDriver> CWaterModel::_CurrDrv;



// TMP
volatile bool forceWaterSimpleRender = false;

//=======================================================================
void CWaterModel::setupVertexBuffer(CVertexBuffer &vb, uint numWantedVertices, IDriver *drv)
{
	if (!numWantedVertices) return;
	if (vb.getNumVertices() == 0 || drv != _CurrDrv) // not setupped yet, or driver changed ?
	{
		vb.setNumVertices(0);
		vb.setName("Water");
		vb.setPreferredMemory(CVertexBuffer::AGPPreferred, false);
		if (drv->supportWaterShader())
		{
			vb.setVertexFormat(CVertexBuffer::PositionFlag);
		}
		else
		{
			vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
		}
		_CurrDrv = drv;
	}
	uint numVerts = std::max(numWantedVertices, WATER_MODEL_DEFAULT_NUM_VERTICES);
	if (numVerts > vb.getNumVertices())
	{
		const uint vb_INCREASE_SIZE = 1000;
		numVerts = vb_INCREASE_SIZE * ((numVerts + (vb_INCREASE_SIZE - 1)) / vb_INCREASE_SIZE); // snap size
		vb.setNumVertices((uint32) numVerts);
	}
}

//=======================================================================
CWaterModel::CWaterModel()
{
	setOpacity(false);
	setTransparency(true);
	setOrderingLayer(1);
	// RenderFilter: We are a SegRemanece
	_RenderFilterType= UScene::FilterWater;
	_Prev = NULL;
	_Next = NULL;
	_MatrixUpdateDate = 0;
}

//=======================================================================
CWaterModel::~CWaterModel()
{
	CScene *scene = getOwnerScene();
	if (scene && scene->getWaterCallback())
	{
		nlassert(Shape);
		CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
		scene->getWaterCallback()->waterSurfaceRemoved(ws->getUseSceneWaterEnvMap(0) || ws->getUseSceneWaterEnvMap(1));
	}
	// should be already unlinked, but security
	unlink();
}

//=======================================================================
void CWaterModel::registerBasic()
{
	CScene::registerModel(WaterModelClassId, TransformShapeId, CWaterModel::creator);
}


//=======================================================================
ITrack* CWaterModel::getDefaultTrack (uint valueId)
{
	nlassert(Shape);
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	switch (valueId)
	{
		case PosValue:			return ws->getDefaultPos(); break;
		case ScaleValue:		return ws->getDefaultScale(); break;
		case RotQuatValue:		return ws->getDefaultRotQuat(); break;
		default: // delegate to parent
			return CTransformShape::getDefaultTrack(valueId);
		break;
	}
}


//=======================================================================
uint32	CWaterModel::getWaterHeightMapID() const
{
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	return ws->_WaterPoolID;
}

//=======================================================================
float	CWaterModel::getHeightFactor() const
{
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	return ws->_WaveHeightFactor;
}


//=======================================================================
float   CWaterModel::getHeight(const CVector2f &pos)
{
	CWaterShape *ws		 = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(ws->_WaterPoolID);
	const float height   = whm.getHeight(pos);
	return height * ws->_WaveHeightFactor + this->getPos().z;
}

//=======================================================================
float   CWaterModel::getAttenuatedHeight(const CVector2f &pos, const NLMISC::CVector &viewer)
{
	CWaterShape *ws		 = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(ws->_WaterPoolID);
	const float maxDist = whm.getUnitSize() * (whm.getSize() >> 1);
	const NLMISC::CVector planePos(pos.x, pos.y, this->getMatrix().getPos().z);
	const float userDist = (planePos - viewer).norm();

	if (userDist > maxDist)
	{
		return this->getMatrix().getPos().z;
	}
	else
	{
		const float height   = whm.getHeight(pos);
		return ws->_WaveHeightFactor * height * (1.f - userDist / maxDist) + this->getMatrix().getPos().z;
	}
}


//=======================================================================

// perform a bilinear on 4 values
//   0---1
//   |   |
//   3---2

/*
static float inline BilinFilter(float v0, float v1, float v2, float v3, float u, float v)
{
	float g = v * v3 + (1.f - v) * v0;
	float h = v * v2 + (1.f - v) * v1;
	return u * h + (1.f - u) * g;
}
*/


//=======================================================================

/// store a value in a water vertex buffer, and increment the pointer
/*
static void inline FillWaterVB(uint8 *&vbPointer, float x, float y, float z, float nx, float ny)
{
	* (float *) vbPointer = x;
	((float *) vbPointer)[1] = y;
	((float *) vbPointer)[2] = z;
	*((float *) (vbPointer + 3 * sizeof(float))) = nx;
	*((float *) (vbPointer + 4 * sizeof(float))) = ny;
	vbPointer += 5 * sizeof(float);
}
*/

// ***************************************************************************************************************
/*
#ifdef NL_OS_WINDOWS
	__forceinline
#endif
static void SetupWaterVertex(  sint  qLeft,
							   sint  qRight,
							   sint  qUp,
							   sint  qDown,
							   sint  qSubLeft,
							   sint  qSubDown,
							   const NLMISC::CVector &inter,
							   float invWaterRatio,
							   sint  doubleWaterHeightMapSize,
							   CWaterHeightMap &whm,
							   uint8 *&vbPointer,
							   float offsetX,
							   float offsetY
							   )
{
	const float wXf = invWaterRatio * (inter.x + offsetX);
	const float wYf = invWaterRatio * (inter.y + offsetY);

	sint wx = (sint) floorf(wXf);
	sint wy = (sint) floorf(wYf);



	if (!
		 (wx >= qLeft && wx < qRight && wy < qUp &&  wy >= qDown)
	   )
	{
		// no perturbation is visible
		FillWaterVB(vbPointer, inter.x, inter.y, 0, 0, 0);
	}
	else
	{


		// filter height and gradient at the given point
		const sint stride = doubleWaterHeightMapSize;


		const uint xm	  = (uint) (wx - qSubLeft);
		const uint ym	  = (uint) (wy - qSubDown);
		const sint offset = xm + stride * ym;
		const float			  *ptWater     = whm.getPointer()	  + offset;


			float deltaU = wXf - wx;
			float deltaV = wYf - wy;
			//nlassert(deltaU >= 0.f && deltaU <= 1.f  && deltaV >= 0.f && deltaV <= 1.f);

			const float			  *ptWaterPrev = whm.getPrevPointer()  + offset;



			float g0x, g1x, g2x, g3x;  // x gradient for current
			float g0xp, g1xp, g2xp, g3xp;

			float gradCurrX, gradCurrY;

			float g0y, g1y, g2y, g3y; // y gradient for previous map
			float g0yp, g1yp, g2yp, g3yp;

			float gradPrevX, gradPrevY;

			/// curr gradient

			g0x = ptWater[ 1] - ptWater[ - 1];
			g1x = ptWater[ 2] - ptWater[ 0 ];
			g2x = ptWater[ 2 + stride] - ptWater[ stride];
			g3x = ptWater[ 1 + stride] - ptWater[ - 1 + stride];

			gradCurrX = BilinFilter(g0x, g1x, g2x, g3x, deltaU, deltaV);


			g0y = ptWater[ stride] - ptWater[ - stride];
			g1y = ptWater[ stride + 1] - ptWater[ - stride + 1];
			g2y = ptWater[ (stride << 1) + 1] - ptWater[ 1];
			g3y = ptWater[ (stride << 1)] - ptWater[0];

			gradCurrY = BilinFilter(g0y, g1y, g2y, g3y, deltaU, deltaV);

			/// prev gradient

			g0xp = ptWaterPrev[ 1] - ptWaterPrev[ - 1];
			g1xp = ptWaterPrev[ 2] - ptWaterPrev[ 0  ];
			g2xp = ptWaterPrev[ 2 + stride] - ptWaterPrev[ + stride];
			g3xp = ptWaterPrev[ 1 + stride] - ptWaterPrev[ - 1 + stride];

			gradPrevX = BilinFilter(g0xp, g1xp, g2xp, g3xp, deltaU, deltaV);


			g0yp = ptWaterPrev[ stride] - ptWaterPrev[ - stride];
			g1yp = ptWaterPrev[ stride + 1] - ptWaterPrev[ - stride + 1];
			g2yp = ptWaterPrev[ (stride << 1) + 1] - ptWaterPrev[ 1 ];
			g3yp = ptWaterPrev[ (stride << 1)] - ptWaterPrev[ 0 ];

			gradPrevY = BilinFilter(g0yp, g1yp, g2yp, g3yp, deltaU, deltaV);


			/// current height
			float h = BilinFilter(ptWater[ 0 ], ptWater[ + 1], ptWater[ 1 + stride], ptWater[stride], deltaU, deltaV);

			/// previous height
			float hPrev = BilinFilter(ptWaterPrev[ 0 ], ptWaterPrev[ 1], ptWaterPrev[ 1 + stride], ptWaterPrev[stride], deltaU, deltaV);


			float timeRatio = whm.getBufferRatio();


			FillWaterVB(vbPointer, inter.x, inter.y, timeRatio * h + (1.f - timeRatio) * hPrev,
						4.5f * (timeRatio * gradCurrX + (1.f - timeRatio) * gradPrevX),
						4.5f * (timeRatio * gradCurrY + (1.f - timeRatio) * gradPrevY)
					   );

			//NLMISC::CVector2f *ptGrad  = whm.getGradPointer() + offset;
	}
}
*/


// *****************************************************************************************************
/*
static void DrawPoly2D(CVertexBuffer &vb, IDriver *drv, const NLMISC::CMatrix &mat, const NLMISC::CPolygon &p)
{
	uint k;

	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		for (k = 0; k < p.Vertices.size(); ++k)
		{
			NLMISC::CVector tPos = mat * NLMISC::CVector(p.Vertices[k].x, p.Vertices[k].y, 0);
			vba.setValueFloat3Ex (WATER_VB_POS, k, tPos.x, tPos.y, tPos.z);
			vba.setValueFloat2Ex (WATER_VB_DX,  k, 0, 0);
		}
	}
	static CIndexBuffer ib;
	ib.setNumIndexes(3 * p.Vertices.size());
	{
		CIndexBufferReadWrite ibaWrite;
		ib.lock (ibaWrite);
		uint32 *ptr = ibaWrite.getPtr();
		for (k = 0; k < p.Vertices.size() - 2; ++k)
		{
			ptr[ k * 3      ] = 0;
			ptr[ k * 3  + 1 ] = k + 1;
			ptr[ k * 3  + 2 ] = k + 2;
		}
	}
	drv->activeIndexBuffer(ib);
	drv->renderSimpleTriangles(0, p.Vertices.size() - 2);
}
*/


// ***************************************************************************************************************
/*
void	CWaterModel::traverseRender()
{
	H_AUTO( NL3D_Water_Render );

	CRenderTrav					&renderTrav		= getOwnerScene()->getRenderTrav();
	IDriver						*drv			= renderTrav.getDriver();


	#ifndef FORCE_SIMPLE_WATER_RENDER
		if (!drv->supportWaterShader())
	#endif
	{
		doSimpleRender(drv);
		return;
	}

	CWaterShape					*shape			= NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);


	if (shape->_GridSizeTouched)
	{
		shape->setupVertexBuffer();
	}

	// inverted object world matrix
	//NLMISC::CMatrix invObjMat = getWorldMatrix().inverted();

	// viewer pos in world space
	const NLMISC::CVector &obsPos =  renderTrav.CamPos;

	// camera matrix in world space
	const NLMISC::CMatrix &camMat = renderTrav.CamMatrix;

	// view matrix (inverted cam matrix)
	const NLMISC::CMatrix &viewMat = renderTrav.ViewMatrix;

	// compute the camera matrix such as there is no rotation around the y axis
	NLMISC::CMatrix camMatUp;
	ComputeUpMatrix(camMat.getJ(), camMatUp, camMat);
	camMatUp.setPos(camMat.getPos());

	const NLMISC::CMatrix matViewUp = camMatUp.inverted();

	// plane z pos in world
	const float zHeight =  getWorldMatrix().getPos().z;

	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();

	const float invNumStepX = 1.f / numStepX;
	const float invNumStepY = 1.f / numStepY;

	const uint rotBorderSize = (shape->_MaxGridSize + (shape->_XGridBorder << 1) - numStepX) >> 1;

	const sint isAbove = obsPos.z > zHeight ? 1 : 0;


	#ifdef NO_WATER_TESSEL
		const float transitionDist	= renderTrav.Near * 0.99f;
	#else
		const float transitionDist	= shape->_TransitionRatio   * renderTrav.Far;
	#endif


	NLMISC::CMatrix modelMat;
	modelMat.setPos(NLMISC::CVector(obsPos.x, obsPos.y, zHeight));
	drv->setupModelMatrix(modelMat);

	//==================//
	// material setup   //
	//==================//

	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(shape->_WaterPoolID);

	setupMaterialNVertexShader(drv, shape, obsPos, isAbove > 0, whm.getUnitSize() * (whm.getSize() >> 1), zHeight);


	drv->setupMaterial(CWaterModel::_WaterMat);

	sint numPass = drv->beginMaterialMultiPass();
	nlassert(numPass == 1); // for now, we assume water is always rendered in a single pass !
	drv->setupMaterialPass(0);


	//setAttenuationFactor(drv, false, obsPos, camMat.getJ(), farDist);
	//disableAttenuation(drv);


	//================================//
	//	Vertex buffer setup           //
	//================================//

	drv->activeVertexBuffer(shape->_VB);

	//================================//
	//	tesselated part of the poly   //
	//================================//

	if (_ClippedPoly.Vertices.size())
	{
		//======================================//
		// Polygon projection on the near plane //
		//======================================//

		static NLMISC::CPolygon2D projPoly; // projected poly
		projPoly.Vertices.resize(_ClippedPoly.Vertices.size());
		const float Near = renderTrav.Near;


		const float xFactor = numStepX * Near / (renderTrav.Right - renderTrav.Left);
		const float xOffset = numStepX * (-renderTrav.Left / (renderTrav.Right - renderTrav.Left)) + 0.5f;
		const float yFactor = numStepY * Near  / (renderTrav.Bottom - renderTrav.Top);
		const float yOffset = numStepY * (-renderTrav.Top / (renderTrav.Bottom - renderTrav.Top)) - 0.5f * isAbove;

		const NLMISC::CMatrix projMat =  matViewUp * getWorldMatrix();
		uint k;
		for (k = 0; k < _ClippedPoly.Vertices.size(); ++k)
		{
			// project points in the view
			NLMISC::CVector t = projMat * _ClippedPoly.Vertices[k];
			float invY = 1.f / t.y;
			projPoly.Vertices[k].set(xFactor * t.x * invY + xOffset, yFactor * t.z * invY + yOffset);
		}

		//=============================================//
		// compute borders of poly at a low resolution //
		//=============================================//

		NLMISC::CPolygon2D::TRasterVect rasters;
		sint startY;
		projPoly.computeBorders(rasters, startY);

		if (rasters.size())
		{
			//===========================//
			// perform Water animation   //
			//===========================//

			const float WaterRatio = whm.getUnitSize();
			const float invWaterRatio = 1.f / WaterRatio;
			const uint  WaterHeightMapSize = whm.getSize();
			const uint  doubleWaterHeightMapSize = (WaterHeightMapSize << 1);


			sint64 idate = getOwnerScene()->getHrcTrav().CurrentDate;



			if (idate != whm.Date)
			{
				whm.setUserPos((sint) (obsPos.x * invWaterRatio) - (WaterHeightMapSize >> 1),
					   (sint) (obsPos.y * invWaterRatio) - (WaterHeightMapSize >> 1)
					  );
				nlassert(getOwnerScene()); // this object should have been created from a CWaterShape!
				whm.animate((float) (getOwnerScene()->getEllapsedTime()));
				whm.Date = idate;
			}

			//float startDate = (float) (1000.f * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()));

			//=====================================//
			//	compute heightmap useful area      //
			//=====================================//

			// We don't store a heighmap for a complete Water area
			// we just consider the height of Water columns that are near the observer
			//
			// Compute a quad in Water height field space that contains the useful heights
			// This helps us to decide whether we should do a lookup in the height map

			sint mapPosX, mapPosY;

			/// get the pos used in the height map (may not be the same than our current pos, has it is taken in account
			/// every 'PropagationTime' second
 			whm.getUserPos(mapPosX, mapPosY);

			const uint mapBorder = 3;

			const sint qRight = (sint) mapPosX + WaterHeightMapSize - mapBorder;
				  sint qLeft  = (sint) mapPosX;
			const sint qUp    = (sint) mapPosY + WaterHeightMapSize - mapBorder;
				  sint qDown  = (sint) mapPosY;

			/// Compute the origin of the area of Water covered by the height map. We use this to converted from object space to 2d map space
			const sint qSubLeft = qLeft - (uint)  qLeft % WaterHeightMapSize;
			const sint qSubDown = qDown - (uint)  qDown % WaterHeightMapSize;

			qLeft += mapBorder;
			qDown += mapBorder;

			//==============================================//
			// setup rays to be traced, and their increment //
			//==============================================//


			// compute  camera rays in world space
			NLMISC::CVector currHV = renderTrav.Left * camMatUp.getI() + renderTrav.Near * camMatUp.getJ() + renderTrav.Top * camMatUp.getK(); // current border vector, incremented at each line
			NLMISC::CVector currV; // current ray vector
			NLMISC::CVector xStep = (renderTrav.Right - renderTrav.Left) * invNumStepX * camMatUp.getI();	   // xStep for the ray vector
			NLMISC::CVector yStep = (renderTrav.Bottom - renderTrav.Top) * invNumStepY * camMatUp.getK();    // yStep for the ray vector

			//===============================================//
			//				perform display                  //
			//===============================================//

			// scale currHV at the top of the poly
			currHV += (startY - 0.5f  * isAbove) * yStep;

			// current index buffer used. We swap each time a row has been drawn
			CIndexBuffer *currIB = &CWaterShape::_IBUpDown, *otherIB = &CWaterShape::_IBDownUp;


			sint vIndex = 0; // index in vertices

			// current raster position
			sint oldStartX, oldEndX, realStartX, realEndX;
			//float invNearWidth = numStepX / (renderTrav.Right - renderTrav.Left);

				//nlinfo("size = %d, maxSize = ", rasters.size(), numStepY);


			const uint wqHeight = rasters.size();
			if (wqHeight)
			{
				// denominator of the intersection equation
				const float denom = - obsPos.z + zHeight;
				// test the upper raster
				// if it is above the horizon, we modify it to reach the correct location
				const float horizonEpsilon = 10E-4f; // we must be a little below the horizon

				// distance from the viewer along the traced ray
				float t;

				NLMISC::CPolygon2D::TRasterVect::const_iterator it = rasters.begin();
				for (uint l = 0; l <= wqHeight; ++l)
				{
					//nlinfo("start = %d, end = %d", it->first, it->second);
					const sint startX = it->first;
					const sint endX   = (it->second + 1);

					nlassert(startX >= - (sint) rotBorderSize);
					nlassert(endX  <= (sint) (numStepX + rotBorderSize));

					if (l != 0)
					{
						realStartX = std::min(startX, oldStartX);
						realEndX = std::max(endX, oldEndX);
					}
					else
					{
						realStartX = startX;
						realEndX =   endX;
					}


					// current view vector
					currV   = currHV + (realStartX - 0.5f) * xStep;

					if (l == 0)
					{
						if (isAbove)
						{
							// test whether the first row is out of horizon.
							// if this is the case, we make a correction
							if (denom * currV.z <= 0)
							{
								// correct for the first line only by adding a y offset
								currV += yStep * ((denom > 0 ? horizonEpsilon : - horizonEpsilon)   - currV.z) / yStep.z;
							}

							// now, for the transition, check whether the first raster does not go over the transition dist

							t = denom / currV.z;
							const float VJ = camMat.getJ() * currV;
							if ( t * VJ >  transitionDist)
							{
								float delta = (1.f / yStep.z) * ( denom * VJ / transitionDist - currV.z);
								// correct the first line to reach that position
								currV += delta * yStep;
							}
						}
					}


					{
						CVertexBufferReadWrite vba;
						shape->_VB.lock (vba);
						uint8 *vbPointer = (uint8 *) vba.getVertexCoordPointer() + shape->_VB.getVertexSize() * (vIndex + realStartX + rotBorderSize);


						for (sint k = realStartX; k <= realEndX; ++k)
						{
							t =   denom / currV.z;
							// compute intersection with plane
							NLMISC::CVector inter = t * currV;
							inter.z += obsPos.z;
							SetupWaterVertex(qLeft, qRight, qUp, qDown, qSubLeft, qSubDown, inter, invWaterRatio, doubleWaterHeightMapSize, whm, vbPointer, obsPos.x, obsPos.y);
							currV += xStep;
						}
					}

					if (l != 0) // 2 line of the ib done ?
					{
						sint count = oldEndX - oldStartX;
						if (count > 0)
						{
							drv->activeIndexBuffer(*currIB);
							drv->renderSimpleTriangles((oldStartX + rotBorderSize) * 6, 2 * count );
						}
					}

					oldStartX = startX;
					oldEndX   = endX;
					currHV    += yStep;
					vIndex    = (numStepX + 2 * rotBorderSize + 1) - vIndex; // swap first row and second row
					std::swap(currIB, otherIB);
					if (l < (wqHeight - 1))
					{
 						++it;
					}
					else
					{
						if (!isAbove)
						{
							// last line
							// test whether we are out of horizon
							if (denom * currHV.z <= 0)
							{
								// correct for the first line only by adding a y offset
								currHV += yStep * ((denom > 0 ? horizonEpsilon : - horizonEpsilon)  - currHV.z) / yStep.z;
							}

							// now, for the transition, check whether the first raster does not go over the transition dist

							t = denom / currHV.z;
							const float VJ = camMat.getJ() * currHV;
							if ( t * VJ >  transitionDist)
							{
								float delta = (1.f / yStep.z) * ( denom * VJ / transitionDist - currHV.z);
								// correct the first line to reach that position
								currHV += delta * yStep;
							}
						}

					}
				}

			}
			//nlinfo("display: %f ms", (float) (1000.f * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()) - startDate));
		}
	}

	//=========================================//
	//			display end poly               //
	//=========================================//

	if (_EndClippedPoly.Vertices.size() != 0)
	{

		CMatrix xform = _WorldMatrix;
		xform.movePos(NLMISC::CVector(- obsPos.x, - obsPos.y, _WorldMatrix.getPos().z));
		DrawPoly2D(shape->_VB, drv, xform, _EndClippedPoly);
	}

	drv->endMaterialMultiPass();


	drv->activeVertexProgram(NULL);

}
*/

// ***********************
// Water MATERIAL SETUP //
// ***********************
/*
void CWaterModel::setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist, float zHeight)
{
	static bool matSetupped = false;
	if (!matSetupped)
	{
		_WaterMat.setLighting(false);
		_WaterMat.setDoubleSided(true);
		_WaterMat.setColor(NLMISC::CRGBA::White);
		_WaterMat.setBlend(true);
		_WaterMat.setSrcBlend(CMaterial::srcalpha);
		_WaterMat.setDstBlend(CMaterial::invsrcalpha);
		_WaterMat.setZWrite(true);
		_WaterMat.setShader(CMaterial::Water);
	}


	const uint cstOffset = 4; // 4 places for the matrix
	NLMISC::CVectorH cst[13];


	//=========================//
	//	setup Water material   //
	//=========================//

	CWaterModel::_WaterMat.setTexture(0, shape->_BumpMap[0]);
	CWaterModel::_WaterMat.setTexture(1, shape->_BumpMap[1]);
	CWaterModel::_WaterMat.setTexture(3, shape->_ColorMap);

	CScene *scene = getOwnerScene();
	if (!above && shape->_EnvMap[1])
	{
		if (shape->_UsesSceneWaterEnvMap[1])
		{
			if (scene->getWaterEnvMap())
			{
				CWaterModel::_WaterMat.setTexture(2, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[1]);
			}
		}
		else
		{
			CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[1]);
		}
	}
	else
	{
		if (shape->_UsesSceneWaterEnvMap[0])
		{
			if (scene->getWaterEnvMap())
			{
				CWaterModel::_WaterMat.setTexture(2, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[0]);
			}
		}
		else
		{
			CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[0]);
		}
	}

	shape->envMapUpdate();

	const uint alphaMapStage = 3;
	if (shape->_ColorMap)
	{
		//WaterMat.setTexture(alphaMapStage, shape->_ColorMap);
		//if (shape->_ColorMap->supportSharing()) nlinfo(shape->_ColorMap->getShareName().c_str());


		// setup 2x3 matrix for lookup in diffuse map
		updateDiffuseMapMatrix();
		cst[13 - cstOffset].set(_ColorMapMatColumn0.x, _ColorMapMatColumn1.x, 0, _ColorMapMatColumn0.x * obsPos.x + _ColorMapMatColumn1.x * obsPos.y + _ColorMapMatPos.x);
		cst[14 - cstOffset].set(_ColorMapMatColumn0.y, _ColorMapMatColumn1.y, 0, _ColorMapMatColumn0.y * obsPos.x + _ColorMapMatColumn1.y * obsPos.y + _ColorMapMatPos.y);
	}
	else
	{
		cst[13 - cstOffset].set(0, 0, 0, 0);
		cst[14 - cstOffset].set(0, 0, 0, 0);
	}

	cst[16 - cstOffset].set(0.1f, 0.1f, 0.1f, 0.1f); // used to avoid imprecision when performing a RSQ to get distance from the origin
	// cst[16 - cstOffset].set(0.0f, 0.0f, 0.0f, 0.0f); // used to avoid imprecision when performing a RSQ to get distance from the origin

	cst[5  - cstOffset].set(0.f, 0.f, 0.f, 0.f); // claping negative values to 0

	// slope of attenuation of normal / height with distance
	const float invMaxDist = shape->_WaveHeightFactor / maxDist;
	cst[6  - cstOffset].set(invMaxDist, shape->_WaveHeightFactor, 0, 0);

	/// set matrix
	drv->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);
	drv->setConstantFog(18);

	// retrieve current time
	float date  = 0.001f * (NLMISC::CTime::getLocalTime() & 0xffffff); // must keep some precision.
	// set bumpmaps pos
	cst[9  - cstOffset].set(fmodf(obsPos.x * shape->_HeightMapScale[0].x, 1.f) + fmodf(date * shape->_HeightMapSpeed[0].x, 1.f), fmodf(shape->_HeightMapScale[0].y * obsPos.y, 1.f) + fmodf(date * shape->_HeightMapSpeed[0].y, 1.f), 0.f, 1.f); // bump map 0 offset
	cst[10  - cstOffset].set(shape->_HeightMapScale[0].x, shape->_HeightMapScale[0].y, 0, 0); // bump map 0 scale
	cst[11  - cstOffset].set(fmodf(shape->_HeightMapScale[1].x * obsPos.x, 1.f) + fmodf(date * shape->_HeightMapSpeed[1].x, 1.f), fmodf(shape->_HeightMapScale[1].y * obsPos.y, 1.f) + fmodf(date * shape->_HeightMapSpeed[1].y, 1.f), 0.f, 1.f); // bump map 1 offset
	cst[12  - cstOffset].set(shape->_HeightMapScale[1].x, shape->_HeightMapScale[1].y, 0, 0); // bump map 1 scale

	cst[4  - cstOffset].set(1.f, 1.f, 1.f, 1.f); // use with min man, and to get the 1 constant
	cst[7  - cstOffset].set(0, 0, obsPos.z - zHeight, 1.f);
	cst[8  - cstOffset].set(0.5f, 0.5f, 0.f, 1.f); // used to scale reflected ray into the envmap

	/// set all our constants in one call
	drv->setConstant(4, sizeof(cst) / sizeof(cst[0]), (float *) &cst[0]);

	shape->initVertexProgram();
	bool result;

	//if (useBumpedVersion)
	//{
	//	if (!useEMBM)
	//	{
	//		result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump2Diffuse).get())
	//										: drv->activeVertexProgram((shape->_VertexProgramBump2).get());
	//	}
	//	else
	//	{
	//		result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump1Diffuse).get())
	//										: drv->activeVertexProgram((shape->_VertexProgramBump1).get());
	//	}
	//}
	//else
	//{
	//	result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramNoBumpDiffuse).get())
	//								: drv->activeVertexProgram((shape->_VertexProgramNoBump).get());
	//}

	//result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump2Diffuse).get())
	//										: drv->activeVertexProgram((shape->_VertexProgramBump2).get());
	//
	//if (!result) nlwarning("no vertex program setuped");
}
*/



void CWaterModel::setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float zHeight)
{
	static bool matSetupped = false;
	if (!matSetupped)
	{
		_WaterMat.setLighting(false);
		_WaterMat.setDoubleSided(true);
		_WaterMat.setColor(NLMISC::CRGBA::White);
		_WaterMat.setBlend(true);
		_WaterMat.setSrcBlend(CMaterial::srcalpha);
		_WaterMat.setDstBlend(CMaterial::invsrcalpha);
		_WaterMat.setZWrite(true);
		_WaterMat.setShader(CMaterial::Water);
	}
	const uint cstOffset = 5; // 4 places for the matrix
	NLMISC::CVectorH cst[13];
	//=========================//
	//	setup Water material   //
	//=========================//
	shape->initVertexProgram();
	CVertexProgram *program = shape->_ColorMap ? CWaterShape::_VertexProgramNoWaveDiffuse : CWaterShape::_VertexProgramNoWave;
	drv->activeVertexProgram(program);
	// TODO_VP_MATERIAL
	CWaterModel::_WaterMat.setTexture(0, shape->_BumpMap[0]);
	CWaterModel::_WaterMat.setTexture(1, shape->_BumpMap[1]);
	CWaterModel::_WaterMat.setTexture(3, shape->_ColorMap);
	CScene *scene = getOwnerScene();
	if (!above && shape->_EnvMap[1])
	{
		if (shape->_UsesSceneWaterEnvMap[1])
		{
			if (scene->getWaterEnvMap())
			{
				CWaterModel::_WaterMat.setTexture(2, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[1]);
			}
		}
		else
		{
			CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[1]);
		}
	}
	else
	{
		if (shape->_UsesSceneWaterEnvMap[0])
		{
			if (scene->getWaterEnvMap())
			{
				CWaterModel::_WaterMat.setTexture(2, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[0]);
			}
		}
		else
		{
			CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[0]);
		}
	}
	shape->envMapUpdate();
	if (shape->_ColorMap)
	{
		// setup 2x3 matrix for lookup in diffuse map
		updateDiffuseMapMatrix();
		cst[11].set(_ColorMapMatColumn0.x, _ColorMapMatColumn1.x, 0, _ColorMapMatColumn0.x * obsPos.x + _ColorMapMatColumn1.x * obsPos.y + _ColorMapMatPos.x);
		cst[12].set(_ColorMapMatColumn0.y, _ColorMapMatColumn1.y, 0, _ColorMapMatColumn0.y * obsPos.x + _ColorMapMatColumn1.y * obsPos.y + _ColorMapMatPos.y);
	}
	/// set matrix
	drv->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);
	// retrieve current time
	double date  = scene->getCurrentTime();
	// set bumpmaps pos
	cst[6].set(fmodf(obsPos.x * shape->_HeightMapScale[0].x, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[0].x, 1), fmodf(shape->_HeightMapScale[0].y * obsPos.y, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[0].y, 1), 0.f, 1.f); // bump map 0 offset
	cst[5].set(shape->_HeightMapScale[0].x, shape->_HeightMapScale[0].y, 0, 0); // bump map 0 scale
	cst[8].set(fmodf(shape->_HeightMapScale[1].x * obsPos.x, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[1].x, 1), fmodf(shape->_HeightMapScale[1].y * obsPos.y, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[1].y, 1), 0.f, 1.f); // bump map 1 offset
	cst[7].set(shape->_HeightMapScale[1].x, shape->_HeightMapScale[1].y, 0, 0); // bump map 1 scale
	cst[9].set(0, 0, obsPos.z - zHeight, 1.f);
	cst[10].set(0.5f, 0.5f, 0.f, 1.f); // used to scale reflected ray into the envmap
	/// set all our constants in one call
	drv->setConstant(cstOffset, sizeof(cst) / sizeof(cst[0]) - cstOffset, (float *) &cst[cstOffset]);
	drv->setConstantFog(4);
}

//================================================
void CWaterModel::setupSimpleRender(CWaterShape *shape, const NLMISC::CVector &obsPos, bool above)
{
	// rendering of water when no vertex / pixel shaders are available
	static bool init = false;
	if (!init)
	{
		// setup the material, no special shader is used here
		_SimpleWaterMat.setLighting(false);
		_SimpleWaterMat.setDoubleSided(true);
		_SimpleWaterMat.setColor(NLMISC::CRGBA::White);

		_SimpleWaterMat.setBlend(true);
		_SimpleWaterMat.setSrcBlend(CMaterial::srcalpha);
		_SimpleWaterMat.setDstBlend(CMaterial::invsrcalpha);
		_SimpleWaterMat.setZWrite(true);
		_SimpleWaterMat.setShader(CMaterial::Normal);

		// stage 0
		_SimpleWaterMat.texEnvOpRGB(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvOpAlpha(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

		// stage 1
		_SimpleWaterMat.texEnvOpRGB(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvOpAlpha(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
		_SimpleWaterMat.texEnvArg1RGB(0, CMaterial::Previous, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg1Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

		init = true;
	}
	// envmap is always present and is in stage 0
	CScene *scene = getOwnerScene();
	if (!above && shape->_EnvMap[1])
	{
		if (shape->_UsesSceneWaterEnvMap[1])
		{
			if (scene->getWaterEnvMap())
			{
				_SimpleWaterMat.setTexture(0, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				_SimpleWaterMat.setTexture(0, shape->_EnvMap[1]);
			}
		}
		else
		{
			_SimpleWaterMat.setTexture(0, shape->_EnvMap[1]);
		}
	}
	else
	{
		if (shape->_UsesSceneWaterEnvMap[0])
		{
			if (scene->getWaterEnvMap())
			{
				_SimpleWaterMat.setTexture(0, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				_SimpleWaterMat.setTexture(0, shape->_EnvMap[0]);
			}
		}
		else
		{
			_SimpleWaterMat.setTexture(0, shape->_EnvMap[0]);
		}
	}
	//
	if (shape->_ColorMap == NULL)
	{
		// version with no color map
		if (!_EmbossTexture)
		{
			_EmbossTexture = new CTextureEmboss;
			_EmbossTexture->setSlopeFactor(4.f);
		}
		if (shape->_BumpMap[1] && shape->_BumpMap[1]->isBumpMap())
		{
			CTextureBump *bm = static_cast<CTextureBump *>((ITexture *) shape->_BumpMap[1]);
			if (bm->getHeightMap())
			{
				_EmbossTexture->setHeightMap(bm->getHeightMap());
			}
		}
		_SimpleWaterMat.setTexture(1, _EmbossTexture);
		_SimpleWaterMat.setTexCoordGen(1, true);
		_SimpleWaterMat.setTexCoordGenMode(1, CMaterial::TexCoordGenObjectSpace);
		double date  = scene->getCurrentTime();
		CMatrix texMat;
		texMat.scale(CVector(shape->_HeightMapScale[1].x, shape->_HeightMapScale[1].y, 1.f));
		texMat.setPos(CVector(fmodf(shape->_HeightMapScale[1].x * obsPos.x, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[1].x, 1),
			          fmodf(shape->_HeightMapScale[1].y * obsPos.y, 1.f) + (float) fmod(date * shape->_HeightMapSpeed[1].y, 1),
			          1.f)
					 );
		_SimpleWaterMat.enableUserTexMat(1, true);
		_SimpleWaterMat.setUserTexMat(1, texMat);
	}
	else
	{
		updateDiffuseMapMatrix();
		// version with a color map : it remplace the emboss texture
		_SimpleWaterMat.setTexture(1, shape->_ColorMap);
		_SimpleWaterMat.setTexCoordGen(1, true);
		_SimpleWaterMat.setTexCoordGenMode(1, CMaterial::TexCoordGenObjectSpace);
		CMatrix texMat;
		/*
		float mat[16] =
		{
			_ColorMapMatColumn0.x, _ColorMapMatColumn1.x, 0, _ColorMapMatColumn0.x * obsPos.x + _ColorMapMatColumn1.x * obsPos.y + _ColorMapMatPos.x},
			_ColorMapMatColumn0.y, _ColorMapMatColumn1.y, 0, _ColorMapMatColumn0.y * obsPos.x + _ColorMapMatColumn1.y * obsPos.y + _ColorMapMatPos.y,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		}
		*/
		float mat[16] =
		{
			_ColorMapMatColumn0.x, _ColorMapMatColumn0.y, 0.f, 0.f,
			_ColorMapMatColumn1.x, _ColorMapMatColumn1.y, 0.f, 0.f,
			0.f,                   0.f,                   1.f, 0.f,
			_ColorMapMatColumn0.x * obsPos.x + _ColorMapMatColumn1.x * obsPos.y + _ColorMapMatPos.x, _ColorMapMatColumn0.y * obsPos.x + _ColorMapMatColumn1.y * obsPos.y + _ColorMapMatPos.y, 0.f, 1.f
		};
		texMat.set(mat);
		_SimpleWaterMat.enableUserTexMat(1, true);
		_SimpleWaterMat.setUserTexMat(1, texMat);
	}
}


//================================================
void CWaterModel::computeClippedPoly()
{
	CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	const std::vector<CPlane>	&worldPyramid   = getOwnerScene()->getClipTrav().WorldFrustumPyramid;
	_ClippedPoly.Vertices.resize(shape->_Poly.Vertices.size());
	uint k;
	for (k = 0; k < shape->_Poly.Vertices.size(); ++k)
	{
		_ClippedPoly.Vertices[k].set(shape->_Poly.Vertices[k].x,
									 shape->_Poly.Vertices[k].y,
									 0.f
								    );
	}
	/*
	NLMISC::CPlane plvect[6];
	const NLMISC::CMatrix &viewMat = clipTrav.ViewMatrix;

	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();
	// Build the view pyramid. We need to rebuild it because we use a wider one to avoid holes on the border of the screen due to water animation
	float centerX = 0.5f  * (clipTrav.Right + clipTrav.Left);
	const float fRight = centerX + (clipTrav.Right -  centerX) * (-(float) CWaterShape::_XGridBorder +  (float) numStepX) / numStepX;
	const float fLeft = centerX + (clipTrav.Left -  centerX) * (-(float) CWaterShape::_XGridBorder +  (float) numStepX) / numStepX;
	float centerY = 0.5f  * (clipTrav.Bottom + clipTrav.Top);
	const float fTop   = centerY + (clipTrav.Top - centerY)  * (-(float) CWaterShape::_YGridBorder +  (float) numStepY) / numStepY;
	const float fBottom   = centerY + (clipTrav.Bottom - centerY)  * (-(float) CWaterShape::_YGridBorder +  (float) numStepY) / numStepY;
	// build pyramid corners
	const float nearDist	    = clipTrav.Near;
	const float farDist			= clipTrav.Far;
	//
	const NLMISC::CVector		pfoc(0,0,0);
	const NLMISC::CVector		lb( fLeft,  nearDist, fBottom );
	const NLMISC::CVector		lt( fLeft,  nearDist, fTop  );
	const NLMISC::CVector		rb( fRight,  nearDist, fBottom );
	const NLMISC::CVector		rt(fRight,	nearDist, fTop  );
	const NLMISC::CVector		lbfarDist(fLeft, farDist, fBottom);
	const NLMISC::CVector		ltfarDist(fLeft, farDist, fTop );
	const NLMISC::CVector		rtfarDist(fRight , farDist, fTop  );
	//
	plvect[0].make(lt, lb, rt);							// near plane
	plvect[1].make(lbfarDist, ltfarDist, rtfarDist);    // far plane
	plvect[2].make(pfoc, lt, lb);
	plvect[3].make(pfoc, rt, lt);
	plvect[4].make(pfoc, rb, rt);
	plvect[5].make(pfoc, lb, rb);
	const NLMISC::CMatrix pyramidMat = viewMat * getWorldMatrix();
	for (k = 0; k < worldPyramid.size(); ++k)
	{
		plvect[k] = plvect[k] * pyramidMat; // put the plane in object space
	}
	_ClippedPoly.clip(plvect, 6);
	*/
	static std::vector<CPlane> tp;
	tp.resize(worldPyramid.size());
	for(uint k = 0; k < tp.size(); ++k)
	{
		tp[k] = worldPyramid[k] * getWorldMatrix();
	}
	_ClippedPoly.clip(tp);
}

// ***********************************************************************************************************
void CWaterModel::unlink()
{
	if (!_Prev)
	{
		nlassert(!_Next);
		return;
	}
	if (_Next)
	{
		_Next->_Prev = _Prev;
	}
	*_Prev = _Next;
	_Next = NULL;
	_Prev = NULL;
}

// ***********************************************************************************************************
void CWaterModel::link()
{
	nlassert(_Next == NULL);
	CScene *scene = getOwnerScene();
	nlassert(scene);
	CRenderTrav &rt = scene->getRenderTrav();
	_Prev = &rt._FirstWaterModel;
	_Next = rt._FirstWaterModel;
	if (_Next)
	{
		_Next->_Prev = &_Next;
	}
	rt._FirstWaterModel = this;
}



// ***********************************************************************************************************
uint CWaterModel::getNumWantedVertices()
{
	H_AUTO( NL3D_Water_Render );
	nlassert(!_ClippedPoly.Vertices.empty());
	//
	CRenderTrav					&renderTrav		= getOwnerScene()->getRenderTrav();
	if (!renderTrav.Perspective || forceWaterSimpleRender) return 0;
	// viewer pos in world space
	const NLMISC::CVector &obsPos = renderTrav.CamPos;
	// view matrix (inverted cam matrix)
	const NLMISC::CMatrix &viewMat = renderTrav.ViewMatrix;
	// plane z pos in world
	const float zHeight =  getWorldMatrix().getPos().z;
	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();
	NLMISC::CMatrix modelMat;
	modelMat.setPos(NLMISC::CVector(obsPos.x, obsPos.y, zHeight));
	static NLMISC::CPolygon2D projPoly; // projected poly
	projPoly.Vertices.resize(_ClippedPoly.Vertices.size());
	// factor to project to grid units
	const float xFactor = numStepX * renderTrav.Near / (renderTrav.Right - renderTrav.Left);
	const float yFactor = numStepY * renderTrav.Near  / (renderTrav.Top - renderTrav.Bottom);
	// project poly on near plane
	const NLMISC::CMatrix &projMat =  viewMat * getWorldMatrix();
	uint k;
	for (k = 0; k < _ClippedPoly.Vertices.size(); ++k)
	{
		// project points in the view
		NLMISC::CVector t = projMat * _ClippedPoly.Vertices[k];
		float invY = 1.f / t.y;
		projPoly.Vertices[k].set(xFactor * t.x * invY, yFactor * t.z * invY);
	}
	// compute grid cells that are entirely inside
	projPoly.computeInnerBorders(_Inside, _MinYInside);
	// compute grid cells that are touched
	static NLMISC::CPolygon2D::TRasterVect border;
	sint minYBorder;
	projPoly.computeOuterBorders(border, minYBorder);
	// border - inside -> gives grid cells that must be clipped to fit the shape boundaries
	// Make sure that rasters  array for inside has the same size that raster array for borders (by inserting NULL rasters)
	sint height = (sint)border.size();
	if (_Inside.empty())
	{
		_MinYInside = minYBorder;
	}
	sint bottomGap = (sint)(border.size() - _Inside.size());
	_Inside.resize(height);
	nlassert(minYBorder == _MinYInside);

	nlassert(bottomGap >= 0);
	if (bottomGap)
	{
		for(sint y = height - bottomGap; y < height; ++y)
		{
			nlassert (y >= 0 && y < (sint)_Inside.size());
			_Inside[y].first =  border[y].first;
			_Inside[y].second = border[y].first - 1; // insert null raster
		}
	}
	//
	for(sint y = 0; y < height - bottomGap; ++y)
	{
		if (_Inside[y].first > _Inside[y].second)
		{
			nlassert (y >= 0 && y < (sint)_Inside.size());
			_Inside[y].first =  border[y].first;
			_Inside[y].second = border[y].first - 1;
		}
		else if (border[y].first > border[y].second)
		{
			nlassert (y >= 0 && y < (sint)_Inside.size());
			border[y].first = _Inside[y].first;
			border[y].second = _Inside[y].first - 1;
		}
	}
	// compute clip planes
	static std::vector<CPlane> clipPlanes;

	const CVector2f *prevVert = &projPoly.Vertices.back();
	const CVector2f *currVert = &projPoly.Vertices.front();
	uint numVerts = (uint)projPoly.Vertices.size();
	bool ccw = projPoly.isCCWOriented();
	clipPlanes.resize(numVerts);
	for(uint k = 0; k < numVerts; ++k)
	{
		NLMISC::CVector v0;
		NLMISC::CVector v1;
		NLMISC::CVector v2;
		v0.set(prevVert->x, prevVert->y, 0.f);
		v1.set(currVert->x, currVert->y, 0.f);
		v2.set(prevVert->x, prevVert->y, (*currVert - *prevVert).norm());
		clipPlanes[k].make(v0, v1, v2);
		if (!ccw)
		{
			clipPlanes[k].invert();
		}
		prevVert = currVert;
		++ currVert;
	}
	// compute clipped tris
	_ClippedTriNumVerts.clear();
	_ClippedTris.clear();
	static NLMISC::CPolygon clipPoly;
	uint totalNumVertices = 0;
	// compute number of vertices for whole grid cells
	for(sint k = 0; k < (sint) border.size(); ++k)
	{
		// left clipped blocks
		for (sint x = border[k].first; x < _Inside[k].first; ++x)
		{
			clipPoly.Vertices.resize(4);
			clipPoly.Vertices[0].set((float) x, (float) (k + _MinYInside), 0.f);
			clipPoly.Vertices[1].set((float) (x + 1), (float) (k + _MinYInside), 0.f);
			clipPoly.Vertices[2].set((float) (x + 1), (float) (k + _MinYInside + 1), 0.f);
			clipPoly.Vertices[3].set((float) x, (float) (k + _MinYInside + 1), 0.f);
			clipPoly.clip(clipPlanes);
			if (!clipPoly.Vertices.empty())
			{
				// backup result (will be unprojected later)
				_ClippedTriNumVerts.push_back((uint)clipPoly.Vertices.size());
				uint prevSize = (uint)_ClippedTris.size();
				_ClippedTris.resize(_ClippedTris.size() + clipPoly.Vertices.size());
				std::copy(clipPoly.Vertices.begin(), clipPoly.Vertices.end(), _ClippedTris.begin() + prevSize); // append to packed list
				totalNumVertices += ((uint)clipPoly.Vertices.size() - 2) * 3;
			}
		}
		// middle block, are not clipped, but count the number of wanted vertices
		if (_Inside[k].first <= _Inside[k].second)
		{
			totalNumVertices += 6 * (_Inside[k].second - _Inside[k].first + 1);
		}
		// right clipped blocks
		for (sint x = _Inside[k].second + 1; x <= border[k].second; ++x)
		{
			clipPoly.Vertices.resize(4);
			clipPoly.Vertices[0].set((float) x, (float)  (k + _MinYInside), 0.f);
			clipPoly.Vertices[1].set((float) (x + 1), (float)  (k + _MinYInside), 0.f);
			clipPoly.Vertices[2].set((float) (x + 1), (float)  (k + _MinYInside + 1), 0.f);
			clipPoly.Vertices[3].set((float) x, (float)  (k + _MinYInside + 1), 0.f);
			clipPoly.clip(clipPlanes);
			if (!clipPoly.Vertices.empty())
			{
				// backup result (will be unprojected later)
				_ClippedTriNumVerts.push_back((uint)clipPoly.Vertices.size());
				uint prevSize = (uint)_ClippedTris.size();
				_ClippedTris.resize(_ClippedTris.size() + clipPoly.Vertices.size());
				std::copy(clipPoly.Vertices.begin(), clipPoly.Vertices.end(), _ClippedTris.begin() + prevSize); // append to packed list
				totalNumVertices += ((uint)clipPoly.Vertices.size() - 2) * 3;
			}
		}
	}
	return totalNumVertices;
}

// ***********************************************************************************************************
uint CWaterModel::fillVB(void *datas, uint startTri, IDriver &drv)
{
	H_AUTO( NL3D_Water_Render );
	if (drv.supportWaterShader())
	{
		return fillVBHard(datas, startTri);
	}
	else
	{
		return fillVBSoft(datas, startTri);
	}
}



static const double WATER_WAVE_SPEED = 1.7;
static const double WATER_WAVE_SCALE = 0.05;
static const double WATER_WAVE_FREQ = 0.3;
static const float WATER_WAVE_ATTEN = 0.2f;




// compute single water vertex in software mode
static
#ifndef NL_DEBUG
	inline
#endif
void computeWaterVertexSoft(float px, float py, CVector &pos, CVector2f &envMapTexCoord, const CVector &camI, const CVector &camJ, const CVector &camK, float denom, double date, const CVector &camPos)
{
	CVector d = px * camI + py * camK + camJ;
	//nlassert(d.z > 0.f);
	float intersectionDist = denom / d.z;
	pos.x  = intersectionDist * d.x;
	pos.y  = intersectionDist * d.y;
	pos.z  = 0.f;
	//
	CVector R(- pos.x,
		      - pos.y,
		      - denom
		     );
	float dist = R.norm();
	if (dist)
	{
		R /= dist;
	}
	envMapTexCoord.set(- 0.5f * R.x + 0.5f, - 0.5f * R.y + 0.5f);
	if (dist)
	{
		float invDist = 1.f / (WATER_WAVE_ATTEN * dist);
		if (invDist > 1.f) invDist = 1.f;
		// TODO : optimize cos if need (for now there are not much call per frame ...)
		envMapTexCoord.x += (float) (invDist * WATER_WAVE_SCALE * (float) cos(date + WATER_WAVE_FREQ * (camPos.x + pos.x)));
	}
}

// ***********************************************************************************************************
uint CWaterModel::fillVBSoft(void *datas, uint startTri)
{
	_StartTri = (uint32) startTri;
	CRenderTrav			  &renderTrav		= getOwnerScene()->getRenderTrav();
	const NLMISC::CMatrix &camMat = renderTrav.CamMatrix;
	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();
	CVector camI = camMat.getI() * (1.f / numStepX) * (renderTrav.Right - renderTrav.Left) / renderTrav.Near;
	CVector camJ = camMat.getJ();
	CVector camK = camMat.getK() * (1.f / numStepY) * (renderTrav.Top - renderTrav.Bottom) / renderTrav.Near;
	float obsZ = camMat.getPos().z;
	float denom = getWorldMatrix().getPos().z - obsZ;
	uint8 *dest = (uint8 *) datas + startTri * 3 * WATER_VERTEX_SOFT_SIZE;
	/*NLMISC::CVector eye = renderTrav.CamPos;
	eye.z -= getWorldMatrix().getPos().z; 	*/
	NLMISC::CVector eye(0.f, 0.f, - denom);
	CVector R;
	CScene *scene = getOwnerScene();
	double date = WATER_WAVE_SPEED * scene->getCurrentTime();
	if (!_ClippedTriNumVerts.empty())
	{
		const CVector2f *currVert =  &_ClippedTris.front();
		static std::vector<CVector> unprojectedTriSoft;
		static std::vector<CVector2f> envMap;
		for(uint k = 0; k < _ClippedTriNumVerts.size(); ++k)
		{
			unprojectedTriSoft.resize(_ClippedTriNumVerts[k]);
			envMap.resize(_ClippedTriNumVerts[k]);
			uint numVerts = _ClippedTriNumVerts[k];
			for(uint l = 0; l < _ClippedTriNumVerts[k]; ++l)
			{
				computeWaterVertexSoft(currVert->x, currVert->y, unprojectedTriSoft[l], envMap[l], camI, camJ, camK, denom, date, camMat.getPos());
				++ currVert;
			}
			for(uint l = 0; l < numVerts - 2; ++l)
			{
				*(CVector *) dest = unprojectedTriSoft[0];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[0];
				dest += sizeof(float[2]);
				*(CVector *) dest = unprojectedTriSoft[l + 1];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[l + 1];
				dest += sizeof(float[2]);
				*(CVector *) dest = unprojectedTriSoft[l + 2];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[l + 2];
				dest += sizeof(float[2]);
			}
		}
	}
	//	TODO : optimize if needed
	for(sint k = 0; k < (sint) _Inside.size(); ++k)
	{
		sint y = k + _MinYInside;
		CVector proj[4];
		CVector2f envMap[4];
		if (_Inside[k].first <= _Inside[k].second)
		{
			// middle block, are not clipped, but count the number of wanted vertices
			for(sint x = _Inside[k].first; x <= _Inside[k].second; ++x)
			{
				computeWaterVertexSoft((float) x, (float) y, proj[0], envMap[0], camI, camJ, camK, denom, date, camMat.getPos());
				computeWaterVertexSoft((float)(x + 1), (float) y, proj[1], envMap[1], camI, camJ, camK, denom, date, camMat.getPos());
				computeWaterVertexSoft((float) (x + 1), (float) (y + 1), proj[2], envMap[2], camI, camJ, camK, denom, date, camMat.getPos());
				computeWaterVertexSoft((float) x, (float) (y + 1), proj[3], envMap[3], camI, camJ, camK, denom, date, camMat.getPos());
				//
				*(CVector *) dest = proj[0];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[0];
				dest += sizeof(float[2]);
				*(CVector *) dest = proj[2];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[2];
				dest += sizeof(float[2]);
				*(CVector *) dest = proj[1];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[1];
				dest += sizeof(float[2]);
				*(CVector *) dest = proj[0];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[0];
				dest += sizeof(float[2]);
				*(CVector *) dest = proj[3];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[3];
				dest += sizeof(float[2]);
				*(CVector *) dest = proj[2];
				dest += sizeof(float[3]);
				*(CVector2f *) dest = envMap[2];
				dest += sizeof(float[2]);
			}
		}
	}
	nlassert((dest - (uint8 * ) datas) % (3 * WATER_VERTEX_SOFT_SIZE) == 0);
	uint endTri = (uint)(dest - (uint8 * ) datas) / (3 * WATER_VERTEX_SOFT_SIZE);
	_NumTris = endTri - _StartTri;
	return endTri;
}

// compute single water vertex for hardware render
static
#ifndef NL_DEBUG
inline
#endif
void computeWaterVertexHard(float px, float py, CVector &pos, const CVector &camI, const CVector &camJ, const CVector &camK, float denom)
{
	CVector d = px * camI + py * camK + camJ;
	float intersectionDist = denom / d.z;
	pos.x  = intersectionDist * d.x;
	pos.y  = intersectionDist * d.y;
	pos.z  = 0.f;
}

// ***********************************************************************************************************
uint CWaterModel::fillVBHard(void *datas, uint startTri)
{
	_StartTri = (uint32) startTri;
	CRenderTrav			  &renderTrav		= getOwnerScene()->getRenderTrav();
	const NLMISC::CMatrix &camMat = renderTrav.CamMatrix;
	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();
	CVector camI = camMat.getI() * (1.f / numStepX) * (renderTrav.Right - renderTrav.Left) / renderTrav.Near;
	CVector camJ = camMat.getJ();
	CVector camK = camMat.getK() * (1.f / numStepY) * (renderTrav.Top - renderTrav.Bottom) / renderTrav.Near;
	float obsZ = camMat.getPos().z;
	float denom = getWorldMatrix().getPos().z - obsZ;
	uint8 *dest = (uint8 *) datas + startTri * WATER_VERTEX_HARD_SIZE * 3;
	if (!_ClippedTriNumVerts.empty())
	{
		const CVector2f *currVert =  &_ClippedTris.front();
		static std::vector<CVector> unprojectedTri;
		for(uint k = 0; k < _ClippedTriNumVerts.size(); ++k)
		{
			unprojectedTri.resize(_ClippedTriNumVerts[k]);
			uint numVerts = _ClippedTriNumVerts[k];
			for(uint l = 0; l < _ClippedTriNumVerts[k]; ++l)
			{
				computeWaterVertexHard(currVert->x, currVert->y, unprojectedTri[l], camI, camJ, camK, denom);
				++ currVert;
			}
			for(uint l = 0; l < numVerts - 2; ++l)
			{
				*(CVector *) dest = unprojectedTri[0];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = unprojectedTri[l + 1];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = unprojectedTri[l + 2];
				dest += WATER_VERTEX_HARD_SIZE;
			}
		}
	}
	//	TODO : optimize if needed
	for(sint k = 0; k < (sint) _Inside.size(); ++k)
	{
		sint y = k + _MinYInside;
		CVector proj[4];
		if (_Inside[k].first <= _Inside[k].second)
		{
			// middle block, are not clipped, but count the number of wanted vertices
			for(sint x = _Inside[k].first; x <= _Inside[k].second; ++x)
			{
				computeWaterVertexHard((float) x, (float) y, proj[0], camI, camJ, camK, denom);
				computeWaterVertexHard((float) (x + 1), (float) y, proj[1], camI, camJ, camK, denom);
				computeWaterVertexHard((float) (x + 1), (float) (y + 1), proj[2], camI, camJ, camK, denom);
				computeWaterVertexHard((float) x, (float) (y + 1), proj[3], camI, camJ, camK, denom);
				//
				*(CVector *) dest = proj[0];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = proj[2];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = proj[1];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = proj[0];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = proj[3];
				dest += WATER_VERTEX_HARD_SIZE;
				*(CVector *) dest = proj[2];
				dest += WATER_VERTEX_HARD_SIZE;
			}
		}
	}
	nlassert((dest - (uint8 * ) datas) % (3 * WATER_VERTEX_HARD_SIZE) == 0);
	uint endTri = (uint)(dest - (uint8 * ) datas) / (3 * WATER_VERTEX_HARD_SIZE);
	_NumTris = endTri - _StartTri;
	return endTri;
}





// ***************************************************************************************************************
void	CWaterModel::traverseRender()
{
	H_AUTO( NL3D_Water_Render );

	CRenderTrav					&renderTrav		= getOwnerScene()->getRenderTrav();
	IDriver						*drv			= renderTrav.getDriver();
	CWaterShape					*shape			= NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	const NLMISC::CVector		&obsPos         = renderTrav.CamPos;
	const float					zHeight         =  getWorldMatrix().getPos().z;

	if (!renderTrav.Perspective || forceWaterSimpleRender)
	{
		// not supported, simple uniform render
		drv->setupModelMatrix(getWorldMatrix());
		static CMaterial waterMat;
		static bool initDone = false;
		if (!initDone)
		{
			waterMat.initUnlit();
			waterMat.setBlend(true);
			waterMat.setSrcBlend(CMaterial::srcalpha);
			waterMat.setDstBlend(CMaterial::invsrcalpha);
			waterMat.setBlend(true);
			waterMat.setDoubleSided(true);
			waterMat.setLighting(false);
		}
		waterMat.setColor(shape->computeEnvMapMeanColor());
		static std::vector<NLMISC::CTriangleUV> tris;
		const NLMISC::CPolygon2D &poly = shape->getShape();
		tris.clear();
		for(sint k = 0; k < (sint) poly.Vertices.size() - 2; ++k)
		{
			NLMISC::CTriangleUV truv;
			truv.V0.set(poly.Vertices[0].x, poly.Vertices[0].y, 0.f);
			truv.V1.set(poly.Vertices[k + 1].x, poly.Vertices[k + 1].y, 0.f);
			truv.V2.set(poly.Vertices[k + 2].x, poly.Vertices[k + 2].y, 0.f);
			tris.push_back(truv);
		}
		CDRU::drawTrianglesUnlit(tris, waterMat, *drv);
	}
	else
	{
		NLMISC::CMatrix modelMat;
		modelMat.setPos(NLMISC::CVector(obsPos.x, obsPos.y, zHeight));
		drv->setupModelMatrix(modelMat);
		bool isAbove = obsPos.z > getWorldMatrix().getPos().z;
		CVertexBuffer &vb = renderTrav.Scene->getWaterVB();
		if (drv->supportWaterShader())
		{
			setupMaterialNVertexShader(drv, shape, obsPos, isAbove, zHeight);
			nlassert(vb.getNumVertices() > 0);
			drv->activeVertexBuffer(vb);
			drv->renderRawTriangles(CWaterModel::_WaterMat, _StartTri, _NumTris);
			drv->activeVertexProgram(NULL);
		}
		else
		{
			setupSimpleRender(shape, obsPos, isAbove);
			drv->activeVertexBuffer(vb);
			drv->activeVertexProgram(NULL);
			drv->renderRawTriangles(CWaterModel::_SimpleWaterMat, _StartTri, _NumTris);
		}
	}
}



// ***********************************************************************************************************
bool CWaterModel::clip()
{
	H_AUTO( NL3D_Water_Render );
	CRenderTrav			&renderTrav= getOwnerScene()->getRenderTrav();
	if (renderTrav.CamPos.z == getWorldMatrix().getPos().z) return false;
	if(Shape)
	{
		computeClippedPoly();
		if (_ClippedPoly.Vertices.empty()) return false;
		// unlink from water model list
		unlink();
		// link into water model list
		link();
		return true;
	}
	else
		return false;
}


/*
// struct used to build vertices for the simple shader
struct CSimpleVertexInfo
{
	NLMISC::CVector XFormPos;
	NLMISC::CUV     UV;
};
*/

// ***********************************************************************************************************
/*
void CWaterModel::doSimpleRender(IDriver *drv)
{
	if (_ClippedPoly.Vertices.empty()) return;
	// rendering of water when no vertex / pixel shaders are available
	CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CRenderTrav	&renderTrav		= getOwnerScene()->getRenderTrav();
	static bool init = false;
	if (!init)
	{
		// setup the material, no special shader is used here
		_SimpleWaterMat.setLighting(false);
		_SimpleWaterMat.setDoubleSided(true);
		_SimpleWaterMat.setColor(NLMISC::CRGBA::White);

		_SimpleWaterMat.setBlend(true);
		_SimpleWaterMat.setSrcBlend(CMaterial::srcalpha);
		_SimpleWaterMat.setDstBlend(CMaterial::invsrcalpha);
		_SimpleWaterMat.setZWrite(true);
		_SimpleWaterMat.setShader(CMaterial::Normal);

		// stage 0
		_SimpleWaterMat.texEnvOpRGB(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvOpAlpha(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

		// stage 1
		_SimpleWaterMat.texEnvOpRGB(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvOpAlpha(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
		_SimpleWaterMat.texEnvArg1RGB(0, CMaterial::Previous, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg1Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

		// setup the vb : one position & two tex coords
		_SimpleRenderVB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
		init = true;
	}

	const NLMISC::CMatrix &worldMatrix = getWorldMatrix();
	const NLMISC::CVector &obsPos = renderTrav.CamPos;

	// setup the material
	bool isAbove = obsPos.z > worldMatrix.getPos().z;

	// envmap is always present and is in stage 0
	CScene *scene = getOwnerScene();
	if (!isAbove && shape->_EnvMap[1])
	{
		if (shape->_UsesSceneWaterEnvMap[1])
		{
			if (scene->getWaterEnvMap())
			{
				_SimpleWaterMat.setTexture(0, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				_SimpleWaterMat.setTexture(0, shape->_EnvMap[1]);
			}
		}
		else
		{
			_SimpleWaterMat.setTexture(0, shape->_EnvMap[1]);
		}
	}
	else
	{
		if (shape->_UsesSceneWaterEnvMap[0])
		{
			if (scene->getWaterEnvMap())
			{
				_SimpleWaterMat.setTexture(0, scene->getWaterEnvMap()->getEnvMap2D());
			}
			else
			{
				_SimpleWaterMat.setTexture(0, shape->_EnvMap[0]);
			}
		}
		else
		{
			_SimpleWaterMat.setTexture(0, shape->_EnvMap[0]);
		}
	}
	//
	static std::vector<CSimpleVertexInfo> verts;
	static CIndexBuffer indices;
	//
	NLMISC::CPolygon2D &poly = shape->_Poly;
	uint numVerts = poly.Vertices.size();
	uint k;
	//
	if (shape->_ColorMap == NULL)
	{
		// version with no color map
		if (!_EmbossTexture)
		{
			_EmbossTexture = new CTextureEmboss;
			_EmbossTexture->setSlopeFactor(4.f);
		}
		if (shape->_BumpMap[1] && shape->_BumpMap[1]->isBumpMap())
		{
			CTextureBump *bm = static_cast<CTextureBump *>((ITexture *) shape->_BumpMap[1]);
			if (bm->getHeightMap())
			{
				_EmbossTexture->setHeightMap(bm->getHeightMap());
			}
		}
		_SimpleWaterMat.setTexture(1, _EmbossTexture);
		_SimpleRenderVB.setNumVertices(numVerts);
		// retrieve current time
		float date  = 0.001f * (NLMISC::CTime::getLocalTime() & 0xffffff); // must keep some precision.
		// Compute tex coordinates for emboss first.
		// On some 3D chip, textures coords can't grow too mush or texture filtering loose accuracy.
		// So we must keep texCoord as low as possible.
		//
		verts.resize(numVerts);
		for(k = 0; k < numVerts; ++k)
		{
			verts[k].XFormPos = worldMatrix * NLMISC::CVector(poly.Vertices[k].x, poly.Vertices[k].y ,0.f);
			verts[k].UV.U = shape->_HeightMapScale[0].x * verts[k].XFormPos.x + date * shape->_HeightMapSpeed[0].x;
			verts[k].UV.V = shape->_HeightMapScale[0].y * verts[k].XFormPos.y + date * shape->_HeightMapSpeed[0].y;
		}
		// get min tex coords
		float minU = verts[0].UV.U;
		float minV = verts[0].UV.V;
		for(k = 1; k < numVerts; ++k)
		{
			minU = std::min(minU, verts[k].UV.U);
			minV = std::min(minV, verts[k].UV.V);
		}
		//
		minU = floorf(minU);
		minV = floorf(minV);
		//
		CVertexBufferReadWrite vba;
		_SimpleRenderVB.lock (vba);
		uint8 *data = (uint8 *) vba.getVertexCoordPointer();
		for(k = 0; k < numVerts; ++k)
		{
			((NLMISC::CVector *) data)->set(poly.Vertices[k].x, poly.Vertices[k].y, 0.f);
			data += sizeof(NLMISC::CVector);
			// texture coord 0 is reflected vector into envmap
			// xform position in world space to compute the reflection
			CVector surfToEye = (obsPos - verts[k].XFormPos).normed();
			// we assume that normal is (0, 0, 1)
			* (float *) data = 0.5f - 0.5f * surfToEye.x;
			((float *) data)[1] = 0.5f  - 0.5f * surfToEye.y;
			data += sizeof(float[2]);
			// texture coord 1 is the embossed map
			* (float *) data = verts[k].UV.U - minU;
			((float *) data)[1] = verts[k].UV.V - minV;
			data += sizeof(float[2]);
		}
	}
	else
	{
		// version with a color map : it remplace the emboss texture
		_SimpleWaterMat.setTexture(1, shape->_ColorMap);
		_SimpleRenderVB.setNumVertices(numVerts);
		CVertexBufferReadWrite vba;
		_SimpleRenderVB.lock (vba);
		//
		uint8 *data = (uint8 *) vba.getVertexCoordPointer();
		for(k = 0; k < numVerts; ++k)
		{
			* (NLMISC::CVector *) data = poly.Vertices[k];
			data += sizeof(CVector);
			// texture coord 0 is reflected vector into envmap
			// xform position in world space to compute the reflection
			NLMISC::CVector xformPos = worldMatrix * poly.Vertices[k];
			NLMISC::CVector surfToEye = (obsPos - xformPos).normed();
			// we assume that normal is (0, 0, 1)
			* (float *) data = 0.5f - 0.5f * surfToEye.x;
			((float *) data)[1] = 0.5f * - 0.5f * surfToEye.y;
			data += sizeof(float[2]);
			// texture coord 1 is the color map
			* (float *) data = shape->_ColorMapMatColumn0.x * xformPos.x + shape->_ColorMapMatColumn1.x * xformPos.y + shape->_ColorMapMatPos.x;
			((float *) data)[1] = shape->_ColorMapMatColumn0.y * xformPos.x + shape->_ColorMapMatColumn1.y * xformPos.y + shape->_ColorMapMatPos.y;
			data += sizeof(float[2]);
		}
	}

	drv->activeVertexProgram(NULL);
	drv->setupModelMatrix(worldMatrix);
	drv->activeVertexBuffer(_SimpleRenderVB);

	// create an index buffer to do the display
	indices.setNumIndexes((numVerts - 2) * 3);
	{
		CIndexBufferReadWrite ibaWrite;
		indices.lock (ibaWrite);
		uint32 *ptr = ibaWrite.getPtr();
		for(k = 0; k < (numVerts - 2); ++k)
		{

			ptr[ k * 3      ] = 0;
			ptr[ k * 3  + 1 ] = k + 1;
			ptr[ k * 3  + 2 ] = k + 2;
		}
	}
	drv->setupMaterial(_SimpleWaterMat);
	drv->activeIndexBuffer(indices);
	drv->renderSimpleTriangles(0, numVerts - 2);
}
*/

// ***********************************************************************************************************
void CWaterModel::updateDiffuseMapMatrix(bool force /* = false*/)
{
	if (compareMatrixDate(_MatrixUpdateDate) ||force)
	{
		CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
		if (shape)
		{
			_MatrixUpdateDate = getMatrixDate();
			// update the uv matrix
			CMatrix uvMat;
			uvMat.setRot(CVector(shape->_ColorMapMatColumn0.x, shape->_ColorMapMatColumn0.y, 0.f),
						 CVector(shape->_ColorMapMatColumn1.x, shape->_ColorMapMatColumn1.y, 0.f),
						 CVector(shape->_ColorMapMatPos.x, shape->_ColorMapMatPos.y, 1.f));
			CMatrix xformMat;
			CMatrix invMat = this->getWorldMatrix().inverted();
			xformMat.setRot(CVector(invMat.getI().x, invMat.getI().y, 0.f),
							CVector(invMat.getJ().x, invMat.getJ().y, 0.f),
							CVector(invMat.getPos().x, invMat.getPos().y, 1.f));
			uvMat = uvMat * xformMat;
			_ColorMapMatColumn0.set(uvMat.getI().x, uvMat.getI().y);
			_ColorMapMatColumn1.set(uvMat.getJ().x, uvMat.getJ().y);
			_ColorMapMatPos.set(uvMat.getK().x, uvMat.getK().y);
		}
	}
}

// ***************************************************************************
void CWaterModel::debugDumpMem(void* &clippedPolyBegin, void* &clippedPolyEnd)
{
	clippedPolyBegin= (void*)(&*_ClippedPoly.Vertices.begin());
	clippedPolyEnd= (void*)(&*_ClippedPoly.Vertices.end());
}

// ***************************************************************************
void CWaterModel::debugClearClippedPoly()
{
	_ClippedPoly.Vertices.clear();
}

//=======================================================================================
//							wave maker implementation
//=======================================================================================

CWaveMakerModel::CWaveMakerModel() : _Time(0)
{
	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);
}

//================================================

void CWaveMakerModel::registerBasic()
{
	CScene::registerModel(WaveMakerModelClassId, TransformShapeId, CWaveMakerModel::creator);
}

//================================================

ITrack* CWaveMakerModel::getDefaultTrack (uint valueId)
{
	nlassert(Shape);
	CWaveMakerShape *ws = NLMISC::safe_cast<CWaveMakerShape *>((IShape *) Shape);
	switch (valueId)
	{
	case PosValue:			return ws->getDefaultPos(); break;
	default: // delegate to parent
		return CTransformShape::getDefaultTrack(valueId);
		break;
	}
}

//================================================
void	CWaveMakerModel::traverseAnimDetail()
{
	CTransformShape::traverseAnimDetail();
	nlassert(getOwnerScene());
	/// get the shape
	CWaveMakerShape *wms = NLMISC::safe_cast<CWaveMakerShape *>((IShape *) Shape);
	const NLMISC::CVector	worldPos = getWorldMatrix().getPos();
	const CVector2f pos2d(worldPos.x, worldPos.y);
	/// get the water height map
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(wms->_PoolID);
	// get the time delta
	const TAnimationTime deltaT  = std::min(getOwnerScene()->getEllapsedTime(), (TAnimationTime) whm.getPropagationTime());
	_Time += deltaT;
	if (!wms->_ImpulsionMode)
	{
		whm.perturbate(pos2d, wms->_Intensity * cosf(2.f / wms->_Period * (float) NLMISC::Pi * _Time), wms->_Radius);
	}
	else
	{
		if (_Time > wms->_Period)
		{
			_Time -= wms->_Period;
			whm.perturbate(pos2d, wms->_Intensity, wms->_Radius);
		}
	}
}

} // NL3D
