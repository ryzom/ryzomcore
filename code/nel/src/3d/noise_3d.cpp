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
#include "nel/3d/noise_3d.h"
#include "nel/3d/driver.h"

using namespace NLMISC;

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
CNoise3d::CNoise3d (IDriver *pDriver)
{
	_Mem = NULL;
	_Mat = NULL;
	_OffS = NULL;
	_Driver = pDriver;
	_NbVertices = 0;
	_IsDriverSupportCloudSinglePass = pDriver->supportCloudRenderSinglePass();
}

// ------------------------------------------------------------------------------------------------
CNoise3d::~CNoise3d ()
{
	//	delete _Mem; // done by CTertureMem destructor
	delete _Mat;
	delete [] _OffS;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::init (uint32 w, uint32 h, uint32 d)
{
	uint32 i;

	if (_Mem != NULL)
		return;

	w = raiseToNextPowerOf2 (w);
	h = raiseToNextPowerOf2 (h);
	d = raiseToNextPowerOf2 (d);
	if (w > 64) w = 64;
	if (h > 64) h = 64;
	if (d > 64) d = 64;
	if (w < 4) w = 4;
	if (h < 4) h = 4;
	if (d < 4) d = 4;
	_Width = w;
	_Height = h;
	_Depth = d;
	uint32 vdpo2 = getPowerOf2(_Depth);

	_NbSliceW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		_NbSliceH = 2 << (vdpo2 / 2);
	else
		_NbSliceH = 1 << (vdpo2 / 2);

	_ScaleW = 1.0f / _NbSliceW;
	_ScaleH = 1.0f / _NbSliceH;

	_Mem = new uint8[w*h*d];

	// Create initial noise
	for (i = 0; i < (w*h*d); i++)
		_Mem[i] = (uint8)(256.0f*rand()/RAND_MAX);

	_OffS = new CUV [_Depth];
	for (i = 0; i < _Depth; i++)
	{
		_OffS[i].U = ((float)rand())/RAND_MAX;
		_OffS[i].V = ((float)rand())/RAND_MAX;
	}

	_Tex = new CTextureMem (_Mem, _Width*_NbSliceW*_Height*_NbSliceH, true, false, _Width*_NbSliceW, _Height*_NbSliceH,
							CBitmap::Alpha);
	_Tex->setWrapS(ITexture::Repeat);
	_Tex->setWrapT(ITexture::Repeat);
	_Tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	_Tex->touch();
	_Tex->generate();

	if (_IsDriverSupportCloudSinglePass)
	{
		_Mat = new CMaterial();
		_Mat->initUnlit();
		_Mat->setShader (CMaterial::Cloud);
		_Mat->setTexture (0, _Tex);
		_Mat->setTexture (1, _Tex);
		_Mat->setColor (CRGBA(255,255,255,255));
		_Mat->setBlend (true);
		_Mat->setBlendFunc(CMaterial::one, CMaterial::one);
		_Mat->setZFunc (CMaterial::always);
		_Mat->setZWrite (false);
		_VertexBuffer.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag |
										CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
	}
	else
	{
		_Mat = new CMaterial();
		_Mat->initUnlit();
		_Mat->setShader (CMaterial::Normal);
		_Mat->setTexture (0, _Tex);
		_Mat->setColor (CRGBA(255,255,255,255));
		_Mat->setBlend (true);
		_Mat->setBlendFunc(CMaterial::one, CMaterial::one);
		_Mat->setZFunc (CMaterial::always);
		_Mat->setZWrite (false);
		_VertexBuffer.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag |
										CVertexBuffer::TexCoord0Flag);
	}
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::render2passes (CQuadUV &qc, float wpos, float alpha)
{
	// For the moment we do it in 2 passes : because wpos is a position between slice we have to do :
	// [ At0*wpos+At1*(1-wpos) ] * alpha
	// this is done like that :
	// At0*[wpos*alpha] + At1*[(1-wpos)*alpha]

	wpos = fmodf (wpos, 1.0f);
	uint32 nSlice1 = (uint32)(wpos * _Depth), nSlice2;
	if (nSlice1 == (_Depth-1))
		nSlice2 = 0;
	else
		nSlice2 = 1 + nSlice1;
	// If wpos is just on slice1 alpha must be one
	float alphaPos = 1.0f - _Depth*(wpos - (((float)nSlice1) / _Depth));

	if (_NbVertices == _VertexBuffer.getNumVertices())
	{
		_VertexBuffer.setNumVertices(_NbVertices+8);
	}

	CVertexBufferReadWrite vba;
	_VertexBuffer.lock (vba);

	uint32 nVSize = _VertexBuffer.getVertexSize ();
	CVector *pVertices = vba.getVertexCoordPointer(_NbVertices);
	*pVertices = qc.V0; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V1; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V2; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V3; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V0; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V1; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V2; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V3;

	CUV *pUV = vba.getTexCoordPointer (_NbVertices, 0);
	*pUV = CUV(qc.Uv0.U*_ScaleW+_OffS[nSlice1].U, qc.Uv0.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv1.U*_ScaleW+_OffS[nSlice1].U, qc.Uv1.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv2.U*_ScaleW+_OffS[nSlice1].U, qc.Uv2.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv3.U*_ScaleW+_OffS[nSlice1].U, qc.Uv3.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );

	*pUV = CUV(qc.Uv0.U*_ScaleW+_OffS[nSlice2].U, qc.Uv0.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv1.U*_ScaleW+_OffS[nSlice2].U, qc.Uv1.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv2.U*_ScaleW+_OffS[nSlice2].U, qc.Uv2.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv3.U*_ScaleW+_OffS[nSlice2].U, qc.Uv3.V*_ScaleH+_OffS[nSlice2].V);

	uint8 finalAlpha = (uint8)(255*alphaPos*alpha);

	// todo hulud d3d vertex color RGBA / BGRA
	uint8 *pColA = (uint8*)vba.getColorPointer(_NbVertices) + 3;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;

	finalAlpha = (uint8)(255*(1.0f-alphaPos)*alpha);

	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha; pColA = ((uint8*)pColA) + nVSize;
	*pColA = finalAlpha;

	_NbVertices += 8;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::render (CQuadUV &qc, float wpos, float intensity)
{
	// [ At0*wpos+At1*(1-wpos) ] * alpha

	if (!_IsDriverSupportCloudSinglePass)
	{
		render2passes (qc, wpos, intensity);
		return;
	}

	_Intensity = intensity;

	wpos = wpos - floorf (wpos);
	uint32 nSlice1 = (uint32)(wpos * _Depth), nSlice2;
	if (nSlice1 == (_Depth-1))
		nSlice2 = 0;
	else
		nSlice2 = 1 + nSlice1;
	// If wpos is just on slice1 alpha must be one
	float alphaPos = 1.0f - _Depth*(wpos - (((float)nSlice1) / _Depth));

	uint8 nAlphaPos = (uint8)(255*alphaPos);

	uint32 nVSize = _VertexBuffer.getVertexSize ();

	if (_NbVertices == _VertexBuffer.getNumVertices())
	{
		_VertexBuffer.setNumVertices (_NbVertices+4);
	}

	CVertexBufferReadWrite vba;
	_VertexBuffer.lock (vba);

	CVector *pVertices = vba.getVertexCoordPointer(_NbVertices);
	*pVertices = qc.V0; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V1; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V2; pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
	*pVertices = qc.V3;

	CUV *pUV = vba.getTexCoordPointer (_NbVertices, 0);
	*pUV = CUV(qc.Uv0.U/_NbSliceW+_OffS[nSlice1].U, qc.Uv0.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv1.U/_NbSliceW+_OffS[nSlice1].U, qc.Uv1.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv2.U/_NbSliceW+_OffS[nSlice1].U, qc.Uv2.V*_ScaleH+_OffS[nSlice1].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv3.U/_NbSliceW+_OffS[nSlice1].U, qc.Uv3.V*_ScaleH+_OffS[nSlice1].V);

	pUV = vba.getTexCoordPointer (_NbVertices, 1);
	*pUV = CUV(qc.Uv0.U*_ScaleW+_OffS[nSlice2].U, qc.Uv0.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv1.U*_ScaleW+_OffS[nSlice2].U, qc.Uv1.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv2.U*_ScaleW+_OffS[nSlice2].U, qc.Uv2.V*_ScaleH+_OffS[nSlice2].V);
	pUV = (CUV*)( ((uint8*)pUV) + nVSize );
	*pUV = CUV(qc.Uv3.U*_ScaleW+_OffS[nSlice2].U, qc.Uv3.V*_ScaleH+_OffS[nSlice2].V);

	// todo hulud d3d vertex color RGBA / BGRA
	uint8 *pColA = (uint8*)vba.getColorPointer(_NbVertices) + 3;
	*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
	*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
	*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
	*pColA = nAlphaPos;

	_NbVertices += 4;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::renderGrid (uint32 nbw, uint32 nbh, uint32 w, uint32 h,
					float UStart, float VStart, float WStart, float dU, float dV, float dW, float intensity)
{

	if (!_IsDriverSupportCloudSinglePass)
	{
		renderGrid2passes (nbw, nbh, w, h, UStart, VStart, WStart, dU, dV, dW, intensity);
		return;
	}

	_Intensity = intensity;

	uint32 i, j, nSlice1, nSlice2;
	float wpos, oneOverNbWNbH = 1.0f / (nbw*nbh);
	CVector *pVertices;
	CUV *pUV0, *pUV1;
	uint8 *pColA, nAlphaPos;
	uint32 nVSize = _VertexBuffer.getVertexSize ();

	if (_VertexBuffer.getNumVertices() < nbw*nbh*4)
	{
		_VertexBuffer.setNumVertices (nbw*nbh*4);
	}

	dU = (UStart+dU) /_NbSliceW;
	dV = (VStart+dV) /_NbSliceH;
	UStart = UStart / _NbSliceW;
	VStart = VStart / _NbSliceH;

	CVertexBufferReadWrite vba;
	_VertexBuffer.lock (vba);

	pVertices = vba.getVertexCoordPointer(0);
	pUV0 = vba.getTexCoordPointer (0, 0);
	pUV1 = vba.getTexCoordPointer (0, 1);
	// todo hulud d3d vertex color RGBA / BGRA
	pColA = (uint8*)vba.getColorPointer(0) + 3;

	for (j = 0; j < nbh; ++j)
	{
		for (i = 0; i < nbw; ++i)
		{
			wpos = (float)WStart+dW*(i+(float)j*nbw)*oneOverNbWNbH;
			wpos = wpos - floorf (wpos);
			nSlice1 = (uint32)(wpos * _Depth);
			if (nSlice1 == (_Depth-1))
				nSlice2 = 0;
			else
				nSlice2 = 1 + nSlice1;
			// If wpos is just on slice1 alpha must be one
			nAlphaPos = (uint8)( 255*(1.0f - _Depth*(wpos - (((float)nSlice1) / _Depth))) );

			*pVertices = CVector((float)i*w,	 (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)i*w,	 (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );

			pUV0->U = UStart+_OffS[nSlice1].U;	pUV0->V = VStart+_OffS[nSlice1].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice1].U;		pUV0->V = VStart+_OffS[nSlice1].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice1].U;		pUV0->V = dV+_OffS[nSlice1].V; 		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = UStart+_OffS[nSlice1].U;	pUV0->V = dV+_OffS[nSlice1].V;		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );

			pUV1->U = UStart+_OffS[nSlice2].U;	pUV1->V = VStart+_OffS[nSlice2].V;	pUV1 = (CUV*)( ((uint8*)pUV1) + nVSize );
			pUV1->U = dU+_OffS[nSlice2].U;		pUV1->V = VStart+_OffS[nSlice2].V;	pUV1 = (CUV*)( ((uint8*)pUV1) + nVSize );
			pUV1->U = dU+_OffS[nSlice2].U;		pUV1->V = dV+_OffS[nSlice2].V; 		pUV1 = (CUV*)( ((uint8*)pUV1) + nVSize );
			pUV1->U = UStart+_OffS[nSlice2].U;	pUV1->V = dV+_OffS[nSlice2].V;		pUV1 = (CUV*)( ((uint8*)pUV1) + nVSize );

			*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nAlphaPos; pColA = ((uint8*)pColA) + nVSize;
		}
	}

	_NbVertices = nbw*nbh*4;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::renderGrid2passes (uint32 nbw, uint32 nbh, uint32 w, uint32 h,
					float UStart, float VStart, float WStart, float dU, float dV, float dW, float intensity)
{
	uint32 i, j, nSlice1, nSlice2;
	float wpos, oneOverNbWNbH = 1.0f / (nbw*nbh);
	CVector *pVertices;
	CUV *pUV0;
	uint8 *pColA, nFinalAlpha;
	uint32 nVSize = _VertexBuffer.getVertexSize ();

	if (_VertexBuffer.getNumVertices() < 2*nbw*nbh*4)
	{
		_VertexBuffer.setNumVertices (2*nbw*nbh*4);
	}

	dU = (UStart+dU) /_NbSliceW;
	dV = (VStart+dV) /_NbSliceH;
	UStart = UStart / _NbSliceW;
	VStart = VStart / _NbSliceH;

	CVertexBufferReadWrite vba;
	_VertexBuffer.lock (vba);

	pVertices = vba.getVertexCoordPointer(0);
	pUV0 = vba.getTexCoordPointer (0, 0);
	// todo hulud d3d vertex color RGBA / BGRA
	pColA = (uint8*)vba.getColorPointer(0) + 3;

	for (j = 0; j < nbh; ++j)
	{
		for (i = 0; i < nbw; ++i)
		{
			wpos = (float)WStart+dW*(i+(float)j*nbw)*oneOverNbWNbH;
			wpos = fmodf (wpos, 1.0f);
			nSlice1 = (uint32)(wpos * _Depth);
			if (nSlice1 == (_Depth-1))
				nSlice2 = 0;
			else
				nSlice2 = 1 + nSlice1;
			// If wpos is just on slice1 alpha must be one
			float alphaPos = 1.0f - _Depth*(wpos - (((float)nSlice1) / _Depth));

			*pVertices = CVector((float)i*w,	 (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)i*w,	 (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)i*w,	 (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)j*h,	 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)(i+1)*w, (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );
			*pVertices = CVector((float)i*w,	 (float)(j+1)*h, 0.0f); pVertices = (CVector*)( ((uint8*)pVertices) + nVSize );

			pUV0->U = UStart+_OffS[nSlice1].U;	pUV0->V = VStart+_OffS[nSlice1].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice1].U;		pUV0->V = VStart+_OffS[nSlice1].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice1].U;		pUV0->V = dV+_OffS[nSlice1].V; 		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = UStart+_OffS[nSlice1].U;	pUV0->V = dV+_OffS[nSlice1].V;		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );

			pUV0->U = UStart+_OffS[nSlice2].U;	pUV0->V = VStart+_OffS[nSlice2].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice2].U;		pUV0->V = VStart+_OffS[nSlice2].V;	pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = dU+_OffS[nSlice2].U;		pUV0->V = dV+_OffS[nSlice2].V; 		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );
			pUV0->U = UStart+_OffS[nSlice2].U;	pUV0->V = dV+_OffS[nSlice2].V;		pUV0 = (CUV*)( ((uint8*)pUV0) + nVSize );

			nFinalAlpha = (uint8)(255*alphaPos*intensity);

			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;

			nFinalAlpha = (uint8)(255*(1.0f-alphaPos)*intensity);

			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
			*pColA = nFinalAlpha; pColA = ((uint8*)pColA) + nVSize;
		}
	}
	_NbVertices = 2*4*nbw*nbh;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::flush ()
{
	if (!_IsDriverSupportCloudSinglePass)
	{
		flush2passes ();
		return;
	}

	_Mat->setColor(CRGBA(0,0,0,(uint8)(255*_Intensity)));
	_Driver->activeVertexBuffer (_VertexBuffer);
	_Driver->renderRawQuads (*_Mat, 0, _NbVertices/4);
	_NbVertices = 0;
}

// ------------------------------------------------------------------------------------------------
void CNoise3d::flush2passes ()
{
	_Driver->activeVertexBuffer (_VertexBuffer);
	_Driver->renderRawQuads (*_Mat, 0, _NbVertices/4);
	_NbVertices = 0;
}

// Accessors
// ------------------------------------------------------------------------------------------------
uint32 CNoise3d::getWidth ()
{
	return _Width;
}

// ------------------------------------------------------------------------------------------------
uint32 CNoise3d::getHeight ()
{
	return _Height;
}

// ------------------------------------------------------------------------------------------------
uint32 CNoise3d::getDepth ()
{
	return _Depth;
}

} // namespace NL3D

