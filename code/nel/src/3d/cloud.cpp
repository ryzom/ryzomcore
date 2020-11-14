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
#include "nel/3d/material.h"
#include "nel/3d/cloud.h"
#include "nel/3d/cloud_scape.h"
#include "nel/3d/noise_3d.h"
#include "nel/3d/scissor.h"
#include "nel/3d/viewport.h"
#include "nel/3d/driver.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
CCloud::CCloud (CCloudScape *pCloudScape)
{
	_CloudScape = pCloudScape;
	_Driver = _CloudScape->_Driver;
	CloudPower = 255; // Max Power
	LastCloudPower = 255;
	CloudDistAtt = 0;
	CloudDiffuse = CRGBA(255,255,255,255);
	CloudAmbient = CRGBA(120,140,160,255);
	_WaitState = 0;
	_BillSize = 0;
	_OldBillSize = 0;
	_UStart = _VStart = _WStart = NULL;
}

// ------------------------------------------------------------------------------------------------
CCloud::~CCloud()
{
	delete _UStart;
	delete _VStart;
	delete _WStart;
}

// ------------------------------------------------------------------------------------------------
void CCloud::init (uint32 nVoxelW, uint32 nVoxelH, uint32 nVoxelD, float rBaseFreq, uint32 nNbOctave)
{
	if (_UStart != NULL)
		return;

	_BaseFreq = rBaseFreq;
	_BillSize = 0;
	_OldBillSize = 0;

	_NbOctave = nNbOctave;
	_UStart = new double[_NbOctave];
	_VStart = new double[_NbOctave];
	_WStart = new double[_NbOctave];

	uint32 i;
	for (i = 0; i < _NbOctave; ++i)
	{
		_UStart[i] = ((double)rand())/RAND_MAX;
		_VStart[i] = ((double)rand())/RAND_MAX;
		_WStart[i] = ((double)rand())/RAND_MAX;
	}

	_Width = raiseToNextPowerOf2 (nVoxelW);
	_Height = raiseToNextPowerOf2 (nVoxelH);
	_Depth = raiseToNextPowerOf2 (nVoxelD);
	uint32 vdpo2 = getPowerOf2(_Depth);
	_NbW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		_NbH = 2 << (vdpo2 / 2);
	else
		_NbH = 1 << (vdpo2 / 2);

	_MemBill = NULL;
	_MemOldBill = NULL;

	float scale = 20.0f + 10.0f*((float)rand())/RAND_MAX;;
	_Size.x = scale * _Width/_Depth;
	scale = 20.0f + 10.0f*((float)rand())/RAND_MAX;;
	_Size.y = scale * _Height/_Depth;
	scale = 20.0f + 10.0f*((float)rand())/RAND_MAX;;
	_Size.z = scale * _Depth/_Depth;

}


// ------------------------------------------------------------------------------------------------
void CCloud::generate (CNoise3d &noise)
{
	float dU, dV, dW;
	uint32 nOct;
	CQuadUV qc;

	//nldebug("calling CCloud::generate(...)");

	// Active the render target
	_Driver->setRenderTarget (_CloudTexTmp->Tex, 0, 0, _Width*_NbW, _Height*_NbH);
	_Driver->setFrustum (0, (float)_CloudTexTmp->Tex->getWidth(), 0, (float)_CloudTexTmp->Tex->getHeight(), -1, 1, false);

	// Setup the matrices view&model, viewport and frustum
	setMode2D ();

	// Clear background
	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	uint32 nVSize = rVB.getVertexSize ();
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		CVector *pVertices = vba.getVertexCoordPointer (0);
		*pVertices = CVector(0.0f,				0.0f,				0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)_NbW*_Width,0.0f,				0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)_NbW*_Width,(float)_NbH*_Height,0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector(0.0f,				(float)_NbH*_Height,0.0f);
		_CloudScape->_MatClear.setColor (CRGBA(0,0,0,0));
	}

	_Driver->activeVertexBuffer (rVB);
	_Driver->renderRawQuads (_CloudScape->_MatClear, 0, 1);

	// Create cloud from noise
	for (nOct = 0; nOct < _NbOctave; ++nOct)
	{
		dU = (_BaseFreq*((float)_Width)/noise.getWidth())*(1<<nOct);
		dV = (_BaseFreq*((float)_Height)/noise.getHeight())*(1<<nOct);
		dW = (_BaseFreq*((float)_Depth)/noise.getDepth())*(1<<nOct);


		noise.renderGrid (_NbW, _NbH, _Width, _Height,
						(float)_UStart[nOct], (float)_VStart[nOct], (float)_WStart[nOct], dU, dV, dW,
						1.0f/(2<<nOct));

		/* This is the same thing as a renderGrid which is optimized to do that
		qc.Uv0 = CUV((float)_UStart[nOct],		(float)_VStart[nOct]);
		qc.Uv1 = CUV((float)_UStart[nOct]+dU,	(float)_VStart[nOct]);
		qc.Uv2 = CUV((float)_UStart[nOct]+dU,	(float)_VStart[nOct]+dV);
		qc.Uv3 = CUV((float)_UStart[nOct],		(float)_VStart[nOct]+dV);
		uint32 i,j;
		for (j = 0; j < _NbH; ++j)
		{
			for (i = 0; i < _NbW; ++i)
			{
				qc.V0 = CVector((float)i*_Width,		(float)j*_Height,		0.0f);
				qc.V1 = CVector((float)(i+1)*_Width,	(float)j*_Height,		0.0f);
				qc.V2 = CVector((float)(i+1)*_Width,	(float)(j+1)*_Height,	0.0f);
				qc.V3 = CVector((float)i*_Width,		(float)(j+1)*_Height,	0.0f);
				noise.render (qc, (float)_WStart[nOct]+dW*(i+(float)j*_NbW)/(((float)_NbW)*_NbH), 1.0f/(2<<nOct));
			}
		}*/
		noise.flush ();
	}

	// Apply attenuation texture (not needed to resetup position again (done when clearing to black))
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		CUV *pUV = vba.getTexCoordPointer (0, 0);
		pUV->U = 0.0f;		pUV->V = 0.0f;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1.0f;		pUV->V = 0.0f;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1.0f;		pUV->V = 1.0f;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 0.0f;		pUV->V = 1.0f;
	}
	uint8 colpow = (uint8)(255-(((uint32)CloudPower*(255-(uint32)CloudDistAtt)) / 255));
	_CloudTexClamp->ToClamp.setColor (CRGBA(255, 255, 255, colpow));
	_Driver->activeVertexBuffer (rVB);
	_Driver->renderRawQuads (_CloudTexClamp->ToClamp, 0, 1);

	// Restore render target
	_Driver->setRenderTarget (NULL);

	_CloudTexTmp->Tex->setFilterMode (ITexture::Nearest, ITexture::NearestMipMapOff);
}


// ------------------------------------------------------------------------------------------------
void CCloud::light ()
{

	uint32 i, j;

	// Destination position for lighting accumulation buffer from (0, 0) size (_Width, _Height)
	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	uint32 nVSize = rVB.getVertexSize ();
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		CVector *pVertices = vba.getVertexCoordPointer (0);
		*pVertices = CVector((float)0.0f,	(float)0.0f,	0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)1.f,	(float)0.0f,	0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)1.f,	(float)1.f,		0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)0.0f,	(float)1.f,		0.0f);
	}

	// Active the render target
	_Driver->setRenderTarget (_CloudTexTmp->TexBuffer, 0, 0, _Width, _Height);

	// Setup the matrices view&model, viewport and frustum
	setMode2D ();

	// Set the frustum and viewport on all the billboards to clear them
	_Driver->setFrustum (0, 1, 0, 1, -1, 1, false);
	CViewport viewport;
	viewport.init (0, 0, 1, 1);
	_Driver->setupViewport (viewport);

	// Clear the screen accumulator for lighting
	CloudDiffuse.A = 255;
	// _CloudScape->_MatClear.setColor (CloudDiffuse);
	_Driver->activeVertexBuffer (rVB);
	_Driver->clear2D (CloudDiffuse);

	CUV *pUV;
	// Lighting : render the alpha of one layer into rgb of the screen
	float oneOverNbW = 1.0f/_NbW;
	float oneOverNbH = 1.0f/_NbH;
	uint32 previ = 0, prevj = 0;
	_Driver->activeVertexBuffer (rVB);
	for (j = 0; j < _NbH; ++j)
	{
		for (i = 0; i < _NbW; ++i)
		{
			// Active the render target
			_Driver->setRenderTarget (_CloudTexTmp->TexBuffer, 0, 0, _Width, _Height);

			// Set the frustum and viewport on the current billboard
			viewport.init (0, 0, 1, 1);
			_Driver->setupViewport (viewport);

			// Add the alpha of the previous layer into the RGB of the destination
			if ((i+j) > 0)
			{
				{
					CVertexBufferReadWrite vba;
					rVB.lock (vba);

					_Driver->setColorMask (true, true, true, false);
					pUV = vba.getTexCoordPointer (0, 0);
					pUV->U = previ*oneOverNbW;		pUV->V = prevj*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = (previ+1)*oneOverNbW;	pUV->V = prevj*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = (previ+1)*oneOverNbW;	pUV->V = (prevj+1)*oneOverNbH;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = previ*oneOverNbW;		pUV->V = (prevj+1)*oneOverNbH;
				}

				_Driver->renderRawQuads (_CloudTexTmp->ToLightRGB, 0, 1);
			}
			// Replace the alpha of the destination by the alpha of the current layer
			_Driver->setColorMask (false, false, false, true);

			{
				CVertexBufferReadWrite vba;
				rVB.lock (vba);

				pUV = vba.getTexCoordPointer (0, 0);
				pUV->U = i*oneOverNbW;		pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = (j+1)*oneOverNbH;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = i*oneOverNbW;		pUV->V = (j+1)*oneOverNbH;
			}

			_Driver->renderRawQuads (_CloudTexTmp->ToLightAlpha, 0, 1);

			// Copy the billboard
			_Driver->setColorMask (true, true, true, true);

			/* The copy target trick. If not available, render the temporary texture in Tex2 */
			if (!_Driver->copyTargetToTexture (_CloudTexTmp->Tex2, i*_Width, j*_Height, 0, 0, 0, 0, 0))
			{
				_Driver->setRenderTarget (_CloudTexTmp->Tex2, i*_Width, j*_Height, _Width, _Height);
				viewport.init ((float)i*oneOverNbW, (float)j*oneOverNbH, oneOverNbW, oneOverNbH);
				_Driver->setupViewport (viewport);

				{
					CVertexBufferReadWrite vba;
					rVB.lock (vba);

					pUV = vba.getTexCoordPointer (0, 0);
					pUV->U = 0;		pUV->V = 0;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = 1;		pUV->V = 0;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = 1;		pUV->V = 1;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
					pUV->U = 0;		pUV->V = 1;
				}

				_Driver->renderRawQuads (_CloudTexTmp->MatCopy, 0, 1);
			}

			previ = i;
			prevj = j;
		}
	}
	_Driver->setRenderTarget (NULL);

	_CloudTexTmp->Tex->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
}


// ------------------------------------------------------------------------------------------------
void CCloud::reset (NL3D::CCamera *pViewer)
{
	if (_BillSize != 4)
	{
		_BillSize = 4;
		_MemBill = new uint8[4*_BillSize*_BillSize];
		_TexBill = new CTextureMem (_MemBill, 4*_BillSize*_BillSize, true, false, _BillSize, _BillSize);
		_TexBill->setWrapS (ITexture::Clamp);
		_TexBill->setWrapT (ITexture::Clamp);
		_TexBill->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
		_TexBill->generate();
		_TexBill->setReleasable (false);
		_TexBill->setRenderTarget (true);
	}
	if (_OldBillSize != 4)
	{
		_OldBillSize = 4;
		_MemOldBill = new uint8[4*_OldBillSize*_OldBillSize];
		_TexOldBill = new CTextureMem (_MemOldBill, 4*_OldBillSize*_OldBillSize, true, false, _OldBillSize, _OldBillSize);
		_TexOldBill->setWrapS (ITexture::Clamp);
		_TexOldBill->setWrapT (ITexture::Clamp);
		_TexOldBill->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
		_TexOldBill->generate();
		_TexOldBill->setReleasable (false);
		_TexOldBill->setRenderTarget (true);
	}

	// Clear background
	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		uint32 nVSize = rVB.getVertexSize ();
		CVector *pVertices = vba.getVertexCoordPointer (0);
		*pVertices = CVector(0.0f, 0.0f, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector(5.0f, 0.0f, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector(5.0f, 5.0f, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector(0.0f, 5.0f, 0.0f);
		_CloudScape->_MatClear.setColor (CRGBA(0,0,0,0));
	}
	_Driver->activeVertexBuffer (rVB);

	_Driver->setRenderTarget (_TexBill, 0, 0, 4, 4);
	setMode2D ();
	_Driver->setFrustum (0, 4, 0, 4, -1, 1, false);
	_Driver->renderRawQuads (_CloudScape->_MatClear, 0, 1);
	_Driver->setRenderTarget (_TexOldBill, 0, 0, 4, 4);
	setMode2D ();
	_Driver->renderRawQuads (_CloudScape->_MatClear, 0, 1);
	_Driver->setRenderTarget (NULL);

//	CMatrix CamMat = pViewer->getMatrix();
//	CVector Viewer = CamMat.getPos();
	CVector Viewer = CVector(0,0,0);
	CVector Center = CVector (_Pos.x+_Size.x/2, _Pos.y+_Size.y/2, _Pos.z+_Size.z/2);
	CVector Size = _Size;
	CVector I, J, K;
	float Left, Right, Top, Bottom, Near, Far;

	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);

	_BillOldCenter = _BillCenter;
	_BillViewer = Viewer;
	_BillCenter = Center;

	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);

	_BillOldCenter = _BillCenter;
	_BillViewer = Viewer;
	_BillCenter = Center;

}

// ------------------------------------------------------------------------------------------------
void CCloud::anim (double dt, double dt2)
{
	for (uint32 nOct = 0; nOct < _NbOctave; ++nOct)
	{
		_UStart[nOct] += dt*(1<<nOct) / 5000.0;
		_VStart[nOct] += dt*(1<<nOct) / 5000.0;
		_WStart[nOct] += dt*(1<<nOct) / 5000.0;
	}
	//_Pos.x += dt2;
	//Time += dt2;
}


// ------------------------------------------------------------------------------------------------
void CCloud::disp ()
{
	CQuadUV qc;
	qc.Uv0 = CUV(0.0f, 0.0f);
	qc.Uv1 = CUV(1.0f, 0.0f);
	qc.Uv2 = CUV(1.0f, 1.0f);
	qc.Uv3 = CUV(0.0f, 1.0f);

	////////////////////////////////////////////////////////////////////////
	CScissor s;
	s.initFullScreen();
	_Driver->setupScissor (s);
	_Driver->setupViewport (CViewport());
	_Driver->setFrustum (0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, false);
	CVector		I(1,0,0);
	CVector		J(0,0,1);
	CVector		K(0,-1,0);
	CMatrix ViewMatrix;
	ViewMatrix.identity();
	ViewMatrix.setRot(I,J,K, true);
	_Driver->setupViewMatrix(ViewMatrix);
	_Driver->setupModelMatrix(CMatrix::Identity);
	////////////////////////////////////////////////////////////////////////

	uint32 w = _NbW*_Width;
	uint32 h = _NbH*_Height;
	qc.V0 = CVector(0.0f/800.0f,	0.0f/600.0f,	0.0f);
	qc.V1 = CVector(w/800.0f,		0.0f/600.0f,	0.0f);
	qc.V2 = CVector(w/800.0f,		h/600.0f,		0.0f);
	qc.V3 = CVector(0.0f/800.0f,	h/600.0f,		0.0f);
	static CMaterial *dispMat = NULL;
	if (dispMat == NULL)
	{
		dispMat = new CMaterial;
		dispMat->initUnlit();
		dispMat->setTexture(0, _CloudTexTmp->Tex);
		dispMat->texEnvOpRGB(0, CMaterial::Replace);
		//dispMat->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcAlpha);
		dispMat->texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		dispMat->setZFunc(CMaterial::always);
		dispMat->setZWrite(false);
		dispMat->setDoubleSided(true);
		dispMat->setBlend (false);
	}

	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		vba.setVertexCoord (0, qc.V0);
		vba.setVertexCoord (1, qc.V1);
		vba.setVertexCoord (2, qc.V2);
		vba.setVertexCoord (3, qc.V3);
		vba.setTexCoord (0, 0, qc.Uv0);
		vba.setTexCoord (1, 0, qc.Uv1);
		vba.setTexCoord (2, 0, qc.Uv2);
		vba.setTexCoord (3, 0, qc.Uv3);
	}
	_Driver->activeVertexBuffer (rVB);
	_Driver->renderRawQuads (*dispMat, 0, 1);
}


// ------------------------------------------------------------------------------------------------
void CCloud::dispXYZ (CMaterial *pMat)
{
	CQuadUV qc;
	uint32 i,j;

	float oneOverNbW = 1.0f / _NbW;
	float oneOverNbH = 1.0f / _NbH;
	float oneOverNbWNbH = 1.0f / (_NbW*_NbH);
	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	uint32 nVSize = rVB.getVertexSize ();
	CVector *pVertices;
	CUV *pUV;
	_Driver->activeVertexBuffer (rVB);

	if (pMat == NULL)
		return;

	for (j = 0; j < _NbH; ++j)
	{
		for (i = 0; i < _NbW; ++i)
		{
			uint32 d = i+j*_NbW;

			{
				CVertexBufferReadWrite vba;
				rVB.lock (vba);

				pVertices = vba.getVertexCoordPointer (0);
				*pVertices = CVector(_Pos.x,			_Pos.y,			_Pos.z+_Size.z*(_NbW*_NbH-d)*oneOverNbWNbH); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
				*pVertices = CVector(_Pos.x+_Size.x,	_Pos.y,			_Pos.z+_Size.z*(_NbW*_NbH-d)*oneOverNbWNbH); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
				*pVertices = CVector(_Pos.x+_Size.x,	_Pos.y+_Size.y,	_Pos.z+_Size.z*(_NbW*_NbH-d)*oneOverNbWNbH); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
				*pVertices = CVector(_Pos.x,			_Pos.y+_Size.y,	_Pos.z+_Size.z*(_NbW*_NbH-d)*oneOverNbWNbH);

				pUV = vba.getTexCoordPointer (0, 0);
				pUV->U = i*oneOverNbW;		pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = (j+1)*oneOverNbH;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = i*oneOverNbW;		pUV->V = (j+1)*oneOverNbH;

				pUV = vba.getTexCoordPointer (0, 1);
				pUV->U = i*oneOverNbW;		pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = j*oneOverNbH;		pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = (i+1)*oneOverNbW;	pUV->V = (j+1)*oneOverNbH;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
				pUV->U = i*oneOverNbW;		pUV->V = (j+1)*oneOverNbH;
			}

			_Driver->renderRawQuads (*pMat, 0, 1);
		}
	}

	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		CVector *pVertices = vba.getVertexCoordPointer (0);
		*pVertices = CVector((float)0.25f,	0, (float)0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)0.75f,	0, (float)0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)0.75f,	0, (float)0.75f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = CVector((float)0.25f,	0, (float)0.75f);
	}
}

// ------------------------------------------------------------------------------------------------
// in ; viewer, center
// out I,J,K, left,right,top,bottom,near,far
void CCloud::calcBill (const CVector &Viewer, const CVector &Center, const CVector &Size, CVector &I, CVector &J, CVector &K,
				float &Left, float &Right, float &Top, float &Bottom, float &Near, float &Far)
{

	CVector ViewDir = Center - Viewer;
	ViewDir.normalize();

	Left = 1000.0f;
	Right = -1000.0f;
	Top = -1000.0f;
	Bottom = 1000.0f;
	Near = 1000.0f;
	Far = -1000.0f;

	if (fabsf(Center.y-Viewer.y) > fabsf(Center.z-Viewer.z))
	{
		K.set(0, 0, 1);
		J= ViewDir;
		I= J^K;
		K= I^J;
	}
	else
	{
		K.set(0, 1, 0);
		J= ViewDir;
		I= J^K;
		K= I^J;
	}
	I.normalize();
	J.normalize();
	K.normalize();

	CMatrix mat;
	mat.identity();
	mat.setRot(I,J,K, true);
	mat.setPos(CVector(Viewer.x, Viewer.y, Viewer.z));
	mat.invert();

	uint32 i, j, k;
	for (i = 0; i < 2; ++i)
	for (j = 0; j < 2; ++j)
	for (k = 0; k < 2; ++k)
	{
		CVector v;
		if (i == 0) v.x = Center.x-Size.x/2; else v.x = Center.x+Size.x/2;
		if (j == 0) v.y = Center.y-Size.y/2; else v.y = Center.y+Size.y/2;
		if (k == 0) v.z = Center.z-Size.z/2; else v.z = Center.z+Size.z/2;
		v = mat.mulPoint(v);
		if (v.y < Near)		Near = v.y;
		if (v.y > Far)		Far = v.y;
	}

	for (i = 0; i < 2; ++i)
	for (j = 0; j < 2; ++j)
	for (k = 0; k < 2; ++k)
	{
		CVector v;
		if (i == 0) v.x = Center.x-Size.x/2; else v.x = Center.x+Size.x/2;
		if (j == 0) v.y = Center.y-Size.y/2; else v.y = Center.y+Size.y/2;
		if (k == 0) v.z = Center.z-Size.z/2; else v.z = Center.z+Size.z/2;
		v = mat.mulPoint(v);
		v.x = v.x / (v.y/Near);
		v.z = v.z / (v.y/Near);
		if (v.x < Left)		Left = v.x;
		if (v.x > Right)	Right = v.x;
		if (v.z < Bottom)	Bottom = v.z;
		if (v.z > Top)		Top = v.z;
	}
}


// ------------------------------------------------------------------------------------------------
// Create the billboard (in the screen at pos (NbW*Width, 0)
void CCloud::genBill (CCamera *pCam, uint32 nBillSize)
{
	// Render target

	// Compute the Bill
	uint32 sizeTMP = _OldBillSize;
	uint8 *MemTMP = _MemOldBill;
	CSmartPtr<CTextureMem>	TexTMP = _TexOldBill;

	_OldBillSize = _BillSize;
	_MemOldBill = _MemBill;
	_TexOldBill = _TexBill;
	_BillSize = sizeTMP;
	_MemBill = MemTMP;
	_TexBill = TexTMP;

	// Check the new size of the billboard
	if (nBillSize != _BillSize)
	{
		_BillSize = nBillSize;
		_MemBill = new uint8[4*_BillSize*_BillSize];
		_TexBill = new CTextureMem (_MemBill, 4*_BillSize*_BillSize, true, false, _BillSize, _BillSize);
		//for (i = 0; i < 4*_BillSize*_BillSize; ++i) _MemBill[i] = (uint8)i;
		_TexBill->setWrapS (ITexture::Clamp);
		_TexBill->setWrapT (ITexture::Clamp);
		_TexBill->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
		_TexBill->setReleasable (false);
		_TexBill->setRenderTarget (true);
		_TexBill->generate();
	}

	// Render target size
	uint32 textureW = _TexBill->getWidth();
	uint32 textureH = _TexBill->getHeight();
	_Driver->setRenderTarget (_TexBill, 0, 0, _BillSize, _BillSize);

	CViewport viewport, viewportOLD;
	viewportOLD.initFullScreen();
	viewport.init(0.0f, 0.0f, ((float)_BillSize+1)/((float)textureW), ((float)_BillSize+1)/((float)textureH));
	_Driver->setupViewport (viewport);

	//CMatrix CamMat = pCam->getMatrix();
	//CVector Viewer = CamMat.getPos();
	CVector Viewer = CVector (0,0,0);
	CVector Center = CVector (_Pos.x+_Size.x/2, _Pos.y+_Size.y/2, _Pos.z+_Size.z/2);
	CVector Size = _Size;
	CVector I, J, K;
	float Left, Right, Top, Bottom, Near, Far;

	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);

	CMatrix mat;
	mat.identity();
	mat.setRot(I,J,K, true);
	mat.setPos(CVector(Viewer.x, Viewer.y, Viewer.z));
	mat.invert();

	// Clear background for cloud creation
	_Driver->setFrustum(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, false);
	_Driver->setupViewMatrix (CMatrix::Identity);
	_Driver->setupModelMatrix (CMatrix::Identity);

	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	uint32 nVSize = rVB.getVertexSize ();
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);
		{
			CVector *pVertices = vba.getVertexCoordPointer (0);
			*pVertices = CVector(0.0f,	0.0f,	0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector(1.0f,	0.0f,	0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector(1.0f,	0.0f,	1.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector(0.0f,	0.0f,	1.0f);
		}
	}

	_CloudScape->_MatClear.setColor (CRGBA(0,0,0,0));
	_Driver->activeVertexBuffer (rVB);
	_Driver->renderRawQuads (_CloudScape->_MatClear, 0, 1);

	// Render
	_Driver->setFrustum(Left, Right, Bottom, Top, Near, Far);
	_Driver->setupViewMatrix(mat);
	_Driver->setupModelMatrix (CMatrix::Identity);

	_CloudTexTmp->ToBill.setColor (CloudAmbient);

	dispXYZ (&_CloudTexTmp->ToBill);

	// Restore render target
	_Driver->setRenderTarget (NULL);

	// This is the end of render to texture like so reset all stuff
	_Driver->setupViewport (viewportOLD);

	_BillOldCenter = _BillCenter;
	_BillViewer = Viewer;
	_BillCenter = Center;

	if (_WaitState > 0)
		_WaitState = _WaitState - 1;

	_LastX = _Pos.x;
}


// ------------------------------------------------------------------------------------------------
void CCloud::dispBill (CCamera *pCam)
{
//	CMatrix CamMat = pCam->getMatrix();
//	CVector Viewer = CamMat.getPos();
	CVector Viewer = CVector (0,0,0);
	CVector Center = CVector (_Pos.x+_Size.x/2, _Pos.y+_Size.y/2, _Pos.z+_Size.z/2);
	CVector Size = _Size;

	// Prepare vertices.
	CQuadUV qc;

	CVector I, J, K;
	float Left, Right, Top, Bottom, Near, Far;

	if ((_MemBill == NULL) || (_MemOldBill == NULL))
		return;

	if (_WaitState > 0)
		return;

	if (Time > FuturTime)
		Time = FuturTime;

	// take old computed bill.
	Viewer= _BillViewer;
/*		Center= _BillCenter*((float)(Trans)/(float)TransTotal) +
			_BillOldCenter*((float)(TransTotal-Trans)/(float)TransTotal);

	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);


	CVector	lct = Viewer + J*Near;
	qc.V0 = lct + I*Left	+ K*Bottom;
	qc.V1 = lct + I*Right	+ K*Bottom;
	qc.V2 = lct + I*Right	+ K*Top;
	qc.V3 = lct + I*Left	+ K*Top;*/

	Center= _BillCenter;
	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);
	CVector	lct = Viewer + J*Near;
	CQuadUV qc0;
	qc0.V0 = lct + I*Left	+ K*Bottom;
	qc0.V1 = lct + I*Right	+ K*Bottom;
	qc0.V2 = lct + I*Right	+ K*Top;
	qc0.V3 = lct + I*Left	+ K*Top;

	Center= _BillOldCenter;
	calcBill (Viewer, Center, Size, I, J, K, Left, Right, Top, Bottom, Near, Far);
	lct = Viewer + J*Near;
	CQuadUV qc1;
	qc1.V0 = lct + I*Left	+ K*Bottom;
	qc1.V1 = lct + I*Right	+ K*Bottom;
	qc1.V2 = lct + I*Right	+ K*Top;
	qc1.V3 = lct + I*Left	+ K*Top;

	float	a0= ((float)(Time)/(float)FuturTime);
	float	a1= (float)(FuturTime-Time)/(float)FuturTime;
	qc.V0= qc0.V0*a0 + qc1.V0*a1;
	qc.V1= qc0.V1*a0 + qc1.V1*a1;
	qc.V2= qc0.V2*a0 + qc1.V2*a1;
	qc.V3= qc0.V3*a0 + qc1.V3*a1;

	qc.Uv0 = CUV(0,	0);
	qc.Uv1 = CUV(1,	0);
	qc.Uv2 = CUV(1,	1);
	qc.Uv3 = CUV(0,	1);

	// Display TexBill with intensity : Trans / TransTotal
	// and TexOldBill  with intensity : (TransTotal-Trans) / TransTotal

	_CloudScape->_MatBill.setTexture (0, _TexOldBill);
	_CloudScape->_MatBill.setTexture (1, _TexBill);
	_CloudScape->_MatBill.setColor (CRGBA(255, 255, 255, (uint8)(255*((float)Time/(float)FuturTime))));
	CVertexBuffer &rVB = _CloudScape->_VertexBuffer;
	{
		CVertexBufferReadWrite vba;
		rVB.lock (vba);

		uint32 nVSize = rVB.getVertexSize ();
		CVector *pVertices = vba.getVertexCoordPointer (0);
		*pVertices = qc.V0; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = qc.V1; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = qc.V2; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
		*pVertices = qc.V3;

		CUV *pUV = vba.getTexCoordPointer (0, 0);
		pUV->U = 0;	pUV->V = 0;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1;	pUV->V = 0;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1;	pUV->V = 1;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 0;	pUV->V = 1;

		pUV = vba.getTexCoordPointer (0, 1);
		pUV->U = 0;	pUV->V = 0;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1;	pUV->V = 0;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 1;	pUV->V = 1;	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
		pUV->U = 0;	pUV->V = 1;
	}

	_Driver->activeVertexBuffer (rVB);
	_Driver->renderRawQuads (_CloudScape->_MatBill, 0, 1);

	//nlinfo ("ok");


	// Debug
	if (_CloudScape->isDebugQuadEnabled())
	{
		static CMaterial *mTmp = NULL;
		if (mTmp == NULL)
		{
			mTmp = new CMaterial();
			mTmp->setBlend(false);
			mTmp->setDoubleSided(true);
		}
		/*if (_BillSize <= 4)
			mTmp->setColor(CRGBA(0,127,0,255));
		else if (_BillSize == 8)
			mTmp->setColor(CRGBA(0,255,0,255));
		else if (_BillSize == 16)
			mTmp->setColor(CRGBA(127,255,0,255));
		else if (_BillSize == 32)
			mTmp->setColor(CRGBA(255,255,0,255));
		else if (_BillSize == 64)
			mTmp->setColor(CRGBA(255,127,0,255));
		else if (_BillSize == 128)
			mTmp->setColor(CRGBA(255,0,0,255));
		else if (_BillSize == 256)
			mTmp->setColor(CRGBA(127,0,0,255));*/

			if (FuturTime <= 4)
			mTmp->setColor(CRGBA(0,127,0,255));
		else if (FuturTime <= 8)
			mTmp->setColor(CRGBA(0,255,0,255));
		else if (FuturTime <= 12)
			mTmp->setColor(CRGBA(127,255,0,255));
		else if (FuturTime <= 16)
			mTmp->setColor(CRGBA(255,255,0,255));
		else if (FuturTime <= 20)
			mTmp->setColor(CRGBA(255,127,0,255));
		else
			mTmp->setColor(CRGBA(255,0,0,255));

		_Driver->setPolygonMode(IDriver::Line);
		_Driver->renderRawQuads (*mTmp, 0, 1);
		_Driver->setPolygonMode(IDriver::Filled);
	}

}


// ------------------------------------------------------------------------------------------------
void CCloud::setMode2D ()
{
	CVector	I(1,0,0), J(0,0,1), K(0,-1,0);
	CMatrix ViewMatrix;
	ViewMatrix.identity ();
	ViewMatrix.setRot (I,J,K, true);
	CScissor Scissor;
	Scissor.initFullScreen();
	_Driver->setupScissor (Scissor);
	_Driver->setupViewport (CViewport());
	_Driver->setupViewMatrix (ViewMatrix);
	_Driver->setupModelMatrix (CMatrix::Identity);
}

} // namespace NL3D

