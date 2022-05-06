// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/flare_model.h"
#include "nel/3d/flare_shape.h"
#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/dru.h"
#include "nel/3d/scene.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/occlusion_query.h"
#include "nel/3d/mesh.h"
#include "nel/3d/viewport.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/common.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

CMaterial CFlareModel::_OcclusionQueryMaterial;
CMaterial CFlareModel::_DrawQueryMaterial;
bool CFlareModel::_OcclusionQuerySettuped = false;
CVertexBuffer CFlareModel::_OcclusionQueryVB;


using NLMISC::CVector;


// ********************************************************************************************************************
CFlareModel::CFlareModel()
{
	std::fill(_Intensity, _Intensity + MaxNumContext, 0.f);
	setTransparency(true);
	setOpacity(false);
	// RenderFilter: We are a flare
	_RenderFilterType= UScene::FilterFlare;
	resetOcclusionQuerries();
	std::fill(_LastRenderIntervalBegin, _LastRenderIntervalBegin + MaxNumContext, (uint64) -2);
	std::fill(_LastRenderIntervalEnd, _LastRenderIntervalEnd + MaxNumContext, (uint64) -2);
	std::fill(_NumFrameForOcclusionQuery, _NumFrameForOcclusionQuery + MaxNumContext, 1);
	Next = NULL;
}

// ********************************************************************************************************************
void CFlareModel::resetOcclusionQuerries()
{
	for(uint k = 0; k < MaxNumContext; ++k)
	{
		for(uint l = 0; l < OcclusionTestFrameDelay; ++l)
		{
			_OcclusionQuery[k][l] =  NULL;
			_DrawQuery[k][l] =  NULL;
		}
	}
}

// ********************************************************************************************************************
CFlareModel::~CFlareModel()
{
	// if driver hasn't  changed, delete all querries
	if (_LastDrv)
	{
		for(uint k = 0; k < MaxNumContext; ++k)
		{
			for(uint l = 0; l < OcclusionTestFrameDelay; ++l)
			{
				if (_OcclusionQuery[k][l])
				{
					_LastDrv->deleteOcclusionQuery(_OcclusionQuery[k][l]);
				}
				if (_DrawQuery[k][l])
				{
					_LastDrv->deleteOcclusionQuery(_DrawQuery[k][l]);
				}
			}
		}
	}
}

// ********************************************************************************************************************
void CFlareModel::registerBasic()
{
	// register the model
	CScene::registerModel(FlareModelClassId, TransformShapeId, CFlareModel::creator);
}


// write a vector in a vertex buffer
static inline void vbWrite(uint8 *&dest, const CVector &v)
{
	((float *) dest)[0] = v.x;
	((float *) dest)[1] = v.y;
	((float *) dest)[2] = v.z;
	dest += 3 * sizeof(float);
}

// write uvs in a vertex buffer
static inline void vbWrite(uint8 *&dest, float uCoord, float vCoord)
{
	((float *) dest)[0] = uCoord;
	((float *) dest)[1] = vCoord;
	dest += 2 * sizeof(float);
}

// ********************************************************************************************************************
void	CFlareModel::traverseRender()
{
	CRenderTrav			&renderTrav = getOwnerScene()->getRenderTrav();
	if (renderTrav.isCurrentPassOpaque()) return;
	IDriver				*drv  = renderTrav.getDriver();
	nlassert(drv);
	// For now, don't render flare if occlusion query is not supported (direct read of z-buffer is far too slow)
	if (!drv->supportOcclusionQuery()) return;
	if (drv != _LastDrv)
	{
		// occlusion queries have been deleted by the driver
		resetOcclusionQuerries();
		_LastDrv = drv;
	}
	uint flareContext = _Scene ? _Scene->getFlareContext() : 0;
	// transform the flare on screen
	const CVector	upt = getWorldMatrix().getPos(); // untransformed pos
	const CVector	pt = renderTrav.ViewMatrix * upt;
	if (pt.y <= renderTrav.Near)
	{
		return; // flare behind us
	}
	nlassert(Shape);
	CFlareShape *fs = NLMISC::safe_cast<CFlareShape *>((IShape *) Shape);
    if (pt.y > fs->getMaxViewDist())
	{
		return;	// flare too far away
	}
	float distIntensity;
	if (fs->getFlareAtInfiniteDist())
	{
		distIntensity   = 1.f;
	}
	else
	{
		// compute a color ratio for attenuation with distance
		const float distRatio = pt.y / fs->getMaxViewDist();
		distIntensity = distRatio > fs->getMaxViewDistRatio() ? 1.f - (distRatio - fs->getMaxViewDistRatio()) / (1.f - fs->getMaxViewDistRatio()) : 1.f;
	}
	//
	uint32 width, height;
	drv->getWindowSize(width, height);
	// Compute position on screen
	const float middleX = .5f * (renderTrav.Left + renderTrav.Right);
	const float middleZ = .5f * (renderTrav.Bottom + renderTrav.Top);
	const sint xPos = (width>>1) + (sint) (width * (((renderTrav.Near * pt.x) / pt.y) - middleX) / (renderTrav.Right - renderTrav.Left));
	const sint yPos = (height>>1) - (sint) (height * (((renderTrav.Near * pt.z) / pt.y) - middleZ) / (renderTrav.Top - renderTrav.Bottom));
	// See if the flare was inside the frustum during the last frame
	// We can't use the scene frame counter because a flare can be rendered in several viewport during the same frame
	// The swapBuffer counter is called only once per frame
	uint64 currFrame = drv->getSwapBufferCounter();
	//
	bool visibilityRetrieved = false;
	float visibilityRatio = 0.f;
	// if driver support occlusion query mechanism, use it
	CMesh *occlusionTestMesh = NULL;
	if (_Scene->getShapeBank())
	{
		occlusionTestMesh = fs->getOcclusionTestMesh(*_Scene->getShapeBank());
	}
	if (drv->supportOcclusionQuery())
	{
		bool issueNewQuery = true;
		IOcclusionQuery *lastOQ = _OcclusionQuery[flareContext][OcclusionTestFrameDelay - 1];
		IOcclusionQuery *lastDQ = _DrawQuery[flareContext][OcclusionTestFrameDelay - 1];
		if (_LastRenderIntervalEnd[flareContext] + 1 == currFrame)
		{
			if (_LastRenderIntervalEnd[flareContext] - _LastRenderIntervalBegin[flareContext] >= OcclusionTestFrameDelay - 1)
			{
				// occlusion test are possibles if at least OcclusionTestFrameDelay frames have ellapsed
				if (lastOQ)
				{
					switch(lastOQ->getOcclusionType())
					{
						case IOcclusionQuery::NotAvailable:
							issueNewQuery = false;
							++ _NumFrameForOcclusionQuery[flareContext];
						break;
						case IOcclusionQuery::Occluded:
							visibilityRetrieved = true;
							visibilityRatio = 0.f;
						break;
						case IOcclusionQuery::NotOccluded:
							if (occlusionTestMesh)
							{
								if (lastDQ)
								{
									if (lastDQ->getOcclusionType() != IOcclusionQuery::NotAvailable)
									{
										visibilityRetrieved = true;
										// eval the percentage of samples that are visible
										//nlinfo("%d / %d", lastOQ->getVisibleCount(), lastDQ->getVisibleCount());
										visibilityRatio = (float) lastOQ->getVisibleCount() / (float) lastDQ->getVisibleCount();
										NLMISC::clamp(visibilityRatio, 0.f, 1.f);
									}
								}
								else
								{
									visibilityRetrieved = true;
									visibilityRatio = 1.f;
								}
							}
							else
							{
								// visibility test is done on a single point
								visibilityRetrieved = true;
								visibilityRatio = 1.f;
							}
						break;
					}
				}
			}
		}
		if (issueNewQuery)
		{
			// shift the queries list
			for(uint k = OcclusionTestFrameDelay - 1; k > 0; --k)
			{
				_OcclusionQuery[flareContext][k] = _OcclusionQuery[flareContext][k - 1];
				_DrawQuery[flareContext][k] = _DrawQuery[flareContext][k - 1];
			}
			_OcclusionQuery[flareContext][0] = lastOQ;
			_DrawQuery[flareContext][0] = lastDQ;
			if (occlusionTestMesh)
			{
				occlusionTest(*occlusionTestMesh, *drv);
			}
			else
			{
				// Insert in list of waiting flare. Don't do it now to avoid repeated setup of test material (a material that don't write to color/zbuffer,
				// and that is used for the sole purpose of the occlusion query)
				_Scene->insertInOcclusionQueryList(this);
			}
		}
	}
	else
	{
		_NumFrameForOcclusionQuery[flareContext] = 1;
		visibilityRetrieved = true;
		// The device doesn't support asynchronous query -> must read the z-buffer directly in a slow fashion
		CViewport vp;
		drv->getViewport(vp);
		// Read z-buffer value at the pos we are
		static std::vector<float> v(1);
		NLMISC::CRect rect((sint32) (vp.getX() * width + vp.getWidth() * xPos),
						   (sint32) (vp.getY() * height + vp.getHeight() * (height - yPos)), 1, 1);
		drv->getZBufferPart(v, rect);
		// Project in screen space
		float z = (float) (1.0 - (1.0 / pt.y - 1.0 / renderTrav.Far) / (1.0 /renderTrav.Near - 1.0 / renderTrav.Far));
		//
		float depthRangeNear, depthRangeFar;
		drv->getDepthRange(depthRangeNear, depthRangeFar);
		z = (depthRangeFar - depthRangeNear) * z + depthRangeNear;
		if (v.empty() || z > v[0]) // test against z-buffer
		{
			visibilityRatio = 0.f;
		}
		else
		{
			visibilityRatio = 1.f;
		}
	}
	// Update render interval
//	nlwarning("frame = %d, last frame = %d", (int) currFrame, (int) _LastRenderIntervalEnd[flareContext]);
	if (_LastRenderIntervalEnd[flareContext] + 1 != currFrame)
	{
		//nlwarning("*");
		_Intensity[flareContext] = 0.f;
		_LastRenderIntervalBegin[flareContext] = currFrame;
	}
	_LastRenderIntervalEnd[flareContext] = currFrame;
	// Update intensity depending on visibility
	if (visibilityRetrieved)
	{
		nlassert(visibilityRatio >= 0.f);
		nlassert(visibilityRatio <= 1.f);
		_NumFrameForOcclusionQuery[flareContext] = 1; // reset number of frame needed to do the occlusion query
		if (visibilityRatio < _Intensity[flareContext])
		{
			float p = fs->getPersistence();
			if (p == 0.f)
			{
				_Intensity[flareContext] = visibilityRatio; // instant update
			}
			else
			{
				_Intensity[flareContext] -= 1.f / p * (float)_Scene->getEllapsedTime() * (float) _NumFrameForOcclusionQuery[flareContext];
				if (_Intensity[flareContext] < visibilityRatio)
				{
					_Intensity[flareContext] = visibilityRatio;
				}
			}
			//nlwarning("intensity update < of %x : %f", (int) this, _Intensity[flareContext]);
		}
		else if (visibilityRatio > _Intensity[flareContext])
		{
			float p = fs->getPersistence();
			if (p == 0.f)
			{
				_Intensity[flareContext] = visibilityRatio; // instant update
			}
			else
			{
				//nlwarning("num frame = %d, currFrame = %d, ", (int) _NumFrameForOcclusionQuery[flareContext], (int) currFrame);
				_Intensity[flareContext] += 1.f / p * (float)_Scene->getEllapsedTime() * (float) _NumFrameForOcclusionQuery[flareContext];
				if (_Intensity[flareContext] > visibilityRatio)
				{
					_Intensity[flareContext] = visibilityRatio;
				}
			}
			//nlwarning("intensity update > of %x : %f", (int) this, _Intensity[flareContext]);
		}
	}
	if (_Intensity[flareContext] == 0.f) return;
	//
	static CMaterial material;
	static CVertexBuffer vb;
	static bool setupDone = false;
	if (!setupDone)
	{
		material.setBlend(true);
		material.setBlendFunc(CMaterial::one, CMaterial::one);
		material.setZWrite(false);
		material.setZFunc(CMaterial::always);
		material.setLighting(false);
		material.setDoubleSided(true);

		// setup vertex buffer
		vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
		vb.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
		vb.setNumVertices(4);
		vb.setName("CFlareModel");
		{
			CVertexBufferReadWrite vba;
			vb.lock (vba);

			vba.setTexCoord(0, 0, NLMISC::CUV(1, 0));
			vba.setTexCoord(1, 0, NLMISC::CUV(1, 1));
			vba.setTexCoord(2, 0, NLMISC::CUV(0, 1));
			vba.setTexCoord(3, 0, NLMISC::CUV(0, 0));
		}
		setupDone = true;
	}
	// setup driver
	drv->activeVertexProgram(NULL);
	drv->activePixelProgram(NULL);
	drv->activeGeometryProgram(NULL);
	drv->setupModelMatrix(fs->getLookAtMode() ? CMatrix::Identity : getWorldMatrix());
	// we don't change the fustrum to draw 2d shapes : it is costly, and we need to restore it after the drawing has been done
	// we setup Z to be (near + far) / 2, and setup x and y to get the screen coordinates we want
	const float zPos             = 0.5f * (renderTrav.Near + renderTrav.Far);
	const float zPosDivNear      = zPos / renderTrav.Near;
	// compute the coeff so that x = ax * px + bx; y = ax * py + by
	const float aX = ( (renderTrav.Right - renderTrav.Left) / (float) width) * zPosDivNear;
	const float bX = zPosDivNear * (middleX - 0.5f * (renderTrav.Right - renderTrav.Left));
	//
	const float aY = - ( (renderTrav.Top - renderTrav.Bottom) / (float) height) * zPosDivNear;
	const float bY = zPosDivNear * (middleZ + 0.5f * (renderTrav.Top - renderTrav.Bottom));
	const CVector I = renderTrav.CamMatrix.getI();
	const CVector J = renderTrav.CamMatrix.getJ();
	const CVector K = renderTrav.CamMatrix.getK();
	//
	CRGBA		 col;
	CRGBA        flareColor = fs->getColor();
	const float norm = sqrtf((float) (((xPos - (width>>1)) * (xPos - (width>>1)) + (yPos - (height>>1))*(yPos - (height>>1)))))
						   / (float) (width>>1);
	// check for dazzle and draw it
	/*if (fs->hasDazzle())
	{
		if (norm < fs->getDazzleAttenuationRange())
		{
			float dazzleIntensity = 1.f - norm / fs->getDazzleAttenuationRange();
			CRGBA dazzleColor = fs->getDazzleColor();
			col.modulateFromui(dazzleColor, (uint) (255.f * _Intensity * dazzleIntensity));
			material.setColor(col);
			material.setTexture(0, NULL);

			const CVector dazzleCenter = renderTrav.CamPos + zPos * J;
			const CVector dI = (width>>1) * aX * I;
			const CVector dK = (height>>1) * bX * K;

			vb.setVertexCoord(0, dazzleCenter + dI + dK);
			vb.setVertexCoord(1, dazzleCenter + dI - dK);
			vb.setVertexCoord(2, dazzleCenter - dI - dK);
			vb.setVertexCoord(3, dazzleCenter - dI + dK);

			drv->renderRawQuads(material, 0, 1);
		}
	}	*/
	if (!fs->getAttenuable() )
	{
		col.modulateFromui(flareColor, (uint) (255.f * distIntensity * _Intensity[flareContext]));
	}
	else
	{
		if (norm > fs->getAttenuationRange() || fs->getAttenuationRange() == 0.f)
		{
			return; // nothing to draw;
		}
		col.modulateFromui(flareColor, (uint) (255.f * distIntensity * _Intensity[flareContext] * (1.f - norm / fs->getAttenuationRange() )));
	}
	col.modulateFromColor(col, getMeanColor());
	if (col == CRGBA::Black) return; // not visible
	material.setColor(col);
	CVector scrPos; // vector that will map to the center of the flare on screen
	// process each flare
	// delta for each new Pos
	const float dX = fs->getFlareSpacing() * ((sint) (width >> 1) - xPos);
	const float dY = fs->getFlareSpacing() * ((sint) (height >> 1) - yPos);
	ITexture *tex;
	// special case for first flare
	tex = fs->getTexture(0);
	if (tex)
	{
		{
			CVertexBufferReadWrite vba;
			vb.lock (vba);
			float size;
			if (fs->getScaleWhenDisappear())
			{
				size = _Intensity[flareContext] * fs->getSize(0) + (1.f - _Intensity[flareContext]) * fs->getSizeDisappear();
			}
			else
			{
				size = fs->getSize(0);
			}
			CVector rI, rK;
			if (fs->getFirstFlareKeepSize())
			{
				size *= renderTrav.Near * (getWorldMatrix().getPos() - renderTrav.CamMatrix.getPos()) * J;
			}
			if (fs->getAngleDisappear() == 0.f)
			{
				if (fs->getLookAtMode())
				{
					rI = I;
					rK = K;
				}
				else
				{
					rI = NLMISC::CVector::I;
					rK = NLMISC::CVector::K;
				}
			}
			else
			{
				float angle = (1.f - _Intensity[flareContext]) * fs->getAngleDisappear() * (float) (NLMISC::Pi / 180);
				float cosTheta = cosf(angle);
				float sinTheta = sinf(angle);
				if (fs->getLookAtMode())
				{
					rI = cosTheta * I + sinTheta * K;
					rK = -sinTheta * I + cosTheta * K;
				}
				else
				{
					rI.set(cosTheta, 0.f, sinTheta);
					rK.set(-sinTheta, 0.f, cosTheta);
				}
			}
			uint8 *vbPtr = (uint8 *) vba.getVertexCoordPointer();
			CHECK_VBA_RANGE(vba, vbPtr, vb.getVertexSize());
			if (fs->getLookAtMode())
			{
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, upt + size * (rI + rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 1.f, 0.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, upt + size * (rI - rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 1.f, 1.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, upt + size * (-rI - rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 0.f, 1.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, upt + size * (-rI + rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 0.f, 0.f); // uvs
			}
			else
			{
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, size * (rI + rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 1.f, 0.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, size * (rI - rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 1.f, 1.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, size * (-rI - rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 0.f, 1.f); // uvs
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, size * (-rI + rK));
				CHECK_VBA(vba, vbPtr); vbWrite(vbPtr, 0.f, 0.f); // uvs
			}
		}
		material.setTexture(0, tex);
		drv->activeVertexBuffer(vb);
		drv->renderRawQuads(material, 0, 1);
	}
	if (fs->_LookAtMode)
	{
		drv->setupModelMatrix(CMatrix::Identity); // look at mode is applied only to first flare
	}
	for (uint k = 1; k < MaxFlareNum; ++k)
	{
		tex = fs->getTexture(k);
		if (tex)
		{
			// compute vector that map to the center of the flare
			scrPos = (aX * (xPos + dX * fs->getRelativePos(k)) + bX) * I
				     +  zPos * J + (aY * (yPos + dY * fs->getRelativePos(k)) + bY) * K + renderTrav.CamMatrix.getPos();

			{
				CVertexBufferReadWrite vba;
				vb.lock (vba);
				uint8 *vbPtr = (uint8 *) vba.getVertexCoordPointer();
				float size = fs->getSize(k) * zPos * renderTrav.Near;
				vbWrite(vbPtr, scrPos + size * (I + K));
				vbWrite(vbPtr, 1.f, 0.f); // uvs
				vbWrite(vbPtr, scrPos + size * (I - K));
				vbWrite(vbPtr, 1.f, 1.f); // uvs
				vbWrite(vbPtr, scrPos + size * (-I - K));
				vbWrite(vbPtr, 0.f, 1.f); // uvs
				vbWrite(vbPtr, scrPos + size * (-I + K));
				vbWrite(vbPtr, 0.f, 0.f); // uvs
			}
			material.setTexture(0, tex);
			drv->activeVertexBuffer(vb);
			drv->renderRawQuads(material, 0, 1);
		}
	}
}


// ********************************************************************************************************************
void CFlareModel::initStatics()
{
	if (!_OcclusionQuerySettuped)
	{
		// setup materials
		_OcclusionQueryMaterial.initUnlit();
		_OcclusionQueryMaterial.setZWrite(false);
		_DrawQueryMaterial.initUnlit();
		_DrawQueryMaterial.setZWrite(false);
		_DrawQueryMaterial.setZFunc(CMaterial::always);
		// setup vbs
		_OcclusionQueryVB.setVertexFormat(CVertexBuffer::PositionFlag);
		_OcclusionQueryVB.setName("CFlareModel::_OcclusionQueryVB");
		_OcclusionQueryVB.setPreferredMemory(CVertexBuffer::RAMVolatile, false); // use ram to avoid stall, and don't want to setup a VB per flare!
		_OcclusionQueryVB.setNumVertices(1);
		_OcclusionQuerySettuped = true;
	}
}

// ********************************************************************************************************************
void CFlareModel::updateOcclusionQueryBegin(IDriver *drv)
{
	nlassert(drv);
	drv->activeVertexProgram(NULL);
	drv->activePixelProgram(NULL);
	drv->activeGeometryProgram(NULL);
	drv->setupModelMatrix(CMatrix::Identity);
	initStatics();
	drv->setColorMask(false, false, false, false); // don't write any pixel during the test

}

// ********************************************************************************************************************
void CFlareModel::updateOcclusionQueryEnd(IDriver *drv)
{
	drv->setColorMask(true, true, true, true);
}

// ********************************************************************************************************************
void CFlareModel::updateOcclusionQuery(IDriver *drv)
{
	nlassert(drv);
	nlassert(drv == _LastDrv); // driver shouldn't change during CScene::render
	// allocate a new occlusion if nit already done
	nlassert(_Scene);
	IOcclusionQuery	*oq = _OcclusionQuery[_Scene->getFlareContext()][0];
	if (!oq)
	{
		nlassert(drv->supportOcclusionQuery());
		oq = drv->createOcclusionQuery();
		if (!oq) return;
		_OcclusionQuery[_Scene->getFlareContext()][0] = oq;
	}
	{
		CVertexBufferReadWrite vbrw;
		_OcclusionQueryVB.lock(vbrw);
		*vbrw.getVertexCoordPointer(0) = getWorldMatrix().getPos();
	}
	drv->activeVertexBuffer(_OcclusionQueryVB);
	oq->begin();
	// draw a single point
	drv->renderRawPoints(_OcclusionQueryMaterial, 0, 1);
	oq->end();
}

// ********************************************************************************************************************
void CFlareModel::renderOcclusionMeshPrimitives(CMesh &mesh, IDriver &drv)
{
	uint numMatrixBlock = mesh.getNbMatrixBlock();
	for(uint k = 0; k < numMatrixBlock; ++k)
	{
		uint numRdrPass = mesh.getNbRdrPass(k);
		for(uint l = 0; l < numRdrPass; ++l)
		{
			CIndexBuffer &ib = const_cast<CIndexBuffer &>(mesh.getRdrPassPrimitiveBlock(k, l));
			drv.activeIndexBuffer(ib);
			drv.renderSimpleTriangles(0, ib.getNumIndexes() / 3);
		}
	}
}

// ********************************************************************************************************************
void CFlareModel::setupOcclusionMeshMatrix(IDriver &drv, CScene &scene) const
{
	nlassert(Shape);
	CFlareShape *fs = NLMISC::safe_cast<CFlareShape *>((IShape *) Shape);
	if (fs->getOcclusionTestMeshInheritScaleRot())
	{
		drv.setupModelMatrix(getWorldMatrix());
	}
	else
	{
		nlassert(scene.getCam());
		CMatrix m = scene.getCam()->getWorldMatrix();
		m.setPos(getWorldMatrix().getPos());
		drv.setupModelMatrix(m);
	}
}

// ********************************************************************************************************************
void CFlareModel::occlusionTest(CMesh &mesh, IDriver &drv)
{
	nlassert(_Scene);
	initStatics();
	IOcclusionQuery	*oq = _OcclusionQuery[_Scene->getFlareContext()][0];
	if (!oq)
	{
		nlassert(drv.supportOcclusionQuery());
		oq = drv.createOcclusionQuery();
		if (!oq) return;
		_OcclusionQuery[_Scene->getFlareContext()][0] = oq;
	}
	IOcclusionQuery	*dq = _DrawQuery[_Scene->getFlareContext()][0];
	if (!dq)
	{
		nlassert(drv.supportOcclusionQuery());
		dq = drv.createOcclusionQuery();
		if (!dq) return;
		_DrawQuery[_Scene->getFlareContext()][0] = dq;
	}
	drv.setColorMask(false, false, false, false); // don't write any pixel during the test
	drv.activeVertexProgram(NULL);
	drv.activePixelProgram(NULL);
	drv.activeGeometryProgram(NULL);
	setupOcclusionMeshMatrix(drv, *_Scene);
	drv.activeVertexBuffer(const_cast<CVertexBuffer &>(mesh.getVertexBuffer()));
	// query drawn count
	drv.setupMaterial(_OcclusionQueryMaterial);
	oq->begin();
	renderOcclusionMeshPrimitives(mesh, drv);
	oq->end();
	// query total count
	drv.setupMaterial(_DrawQueryMaterial);
	dq->begin();
	renderOcclusionMeshPrimitives(mesh, drv);
	dq->end();
	drv.setColorMask(true, true, true, true); // restore pixel writes
}

// ********************************************************************************************************************
void CFlareModel::renderOcclusionTestMesh(IDriver &drv)
{
	nlassert(_Scene);
	if (!_Scene->getShapeBank()) return;
	nlassert(Shape);
	CFlareShape *fs = NLMISC::safe_cast<CFlareShape *>((IShape *) Shape);
	CMesh *occlusionTestMesh = fs->getOcclusionTestMesh(*_Scene->getShapeBank());
	if (!occlusionTestMesh) return;
	setupOcclusionMeshMatrix(drv, *_Scene);
	drv.activeVertexBuffer(const_cast<CVertexBuffer &>(occlusionTestMesh->getVertexBuffer()));
	renderOcclusionMeshPrimitives(*occlusionTestMesh, drv);
}




} // NL3D






















