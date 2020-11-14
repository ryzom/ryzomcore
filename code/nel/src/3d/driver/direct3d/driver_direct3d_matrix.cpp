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

#include "stddirect3d.h"

#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

void CDriverD3D::updateMatrices ()
{
	H_AUTO_D3D(CDriver3D_updateMatrices );
	// Update view model matrix
	D3DXMatrixMultiply (&_D3DModelView, &(_MatrixCache[remapMatrixIndex (D3DTS_WORLD)].Matrix), &(_MatrixCache[remapMatrixIndex (D3DTS_VIEW)].Matrix));

	// Update view model projection matrix
	_D3DModelViewProjection = _D3DModelView;
	D3DXMatrixMultiply (&_D3DModelViewProjection, &_D3DModelViewProjection, &(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)].Matrix));

	// Update the inverted view model matrix
	D3DXMatrixInverse (&_D3DInvModelView, NULL, &_D3DModelView);

	// Update the normalize state
	setRenderState (D3DRS_NORMALIZENORMALS, (_UserViewMtx.hasScalePart() || _UserModelMtx.hasScalePart() || _ForceNormalize)?TRUE:FALSE);
}

// ***************************************************************************

void CDriverD3D::updateProjectionMatrix ()
{
	H_AUTO_D3D(CDriver3D_updateProjectionMatrix );
	float left = _FrustumLeft;
	float right = _FrustumRight;
	float top = _FrustumTop;
	float bottom = _FrustumBottom;

	if (_RenderTarget.Texture)
		swap (bottom, top);

	// Get the render target size
	uint32 clientWidth;
	uint32 clientHeight;
	getRenderTargetSize (clientWidth, clientHeight);

	// In D3D, the center of the first screen pixel is [0.0,0.0]. Is NeL it is [0.5,0.5]
	const float addW = (right-left)/(2*(_Viewport.getWidth() * (float)clientWidth));
	const float addH = (bottom-top)/(2*(_Viewport.getHeight() * (float)clientHeight));

	left += addW;
	right += addW;
	top += addH;
	bottom += addH;

	D3DXMATRIX projection;
	if (_FrustumPerspective)
	{
		D3DXMatrixPerspectiveOffCenterLH (&projection, left, right, bottom, top, _FrustumZNear, _FrustumZFar);
	}
	else
	{
		D3DXMatrixOrthoOffCenterLH (&projection, left, right, bottom, top, _FrustumZNear, _FrustumZFar);
	}
	setMatrix (D3DTS_PROJECTION, projection);

	// Backup znear and zfar for zbias setup
	_OODeltaZ = 1 / (_FrustumZFar - _FrustumZNear);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	H_AUTO_D3D(CDriverD3D_setFrustum)
	_FrustumLeft = left;
	_FrustumRight = right;
	_FrustumTop = top;
	_FrustumBottom = bottom;
	_FrustumZNear = znear;
	_FrustumZFar = zfar;
	_FrustumPerspective = perspective;
	updateProjectionMatrix ();
}


// ***************************************************************************

void CDriverD3D::setFrustumMatrix(CMatrix &frustumMatrix)
{
	H_AUTO_D3D(CDriverD3D_setFrustum)

	frustumMatrix.transpose();
	setMatrix (D3DTS_PROJECTION, D3DXMATRIX(frustumMatrix.get()));
}

// ***************************************************************************

CMatrix CDriverD3D::getFrustumMatrix()
{
	H_AUTO_D3D(CDriverD3D_getFrustum)

	CMatrix frustumMatrix;
	frustumMatrix.set((float *)_MatrixCache[D3DTS_PROJECTION].Matrix.m);
	frustumMatrix.transpose();

	return frustumMatrix;
}

// ***************************************************************************

void CDriverD3D::setupViewMatrix(const CMatrix& mtx)
{
	H_AUTO_D3D(CDriverD3D_setupViewMatrix)
	// Remember the view matrix
	_UserViewMtx= mtx;
	_PZBCameraPos= CVector::Null;

	// Set the driver matrix
	D3DXMATRIX view;
	NL_D3D_MATRIX (view, mtx);

	// Pass to directx matrix basis
	swap (view._12, view._13);
	swap (view._22, view._23);
	swap (view._32, view._33);
	swap (view._42, view._43);

	setMatrix (D3DTS_VIEW, view);

	// Set the spacular matrix
	CMatrix specularTex;
	specularTex = mtx;
	specularTex.setPos(CVector(0.0f,0.0f,0.0f));
	specularTex.invert();
	NL_D3D_MATRIX (_D3DSpecularWorldTex, specularTex);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	H_AUTO_D3D(CDriverD3D_setupViewMatrixEx)
	// Remeber the view matrix
	_UserViewMtx= mtx;
	_PZBCameraPos= cameraPos;

	// Set the driver matrix
	D3DXMATRIX view;
	NL_D3D_MATRIX (view, mtx);

	// Pass to directx matrix basis
	swap (view._12, view._13);
	swap (view._22, view._23);
	swap (view._32, view._33);
	swap (view._42, view._43);

	// Reset the viewMtx position.
	view._41 = 0;
	view._42 = 0;
	view._43 = 0;

	setMatrix (D3DTS_VIEW, view);

	// Set the spacular matrix
	CMatrix specularTex;
	specularTex = mtx;
	NL_D3D_MATRIX (_D3DSpecularWorldTex, specularTex);
	swap (_D3DSpecularWorldTex._12, _D3DSpecularWorldTex._13);
	swap (_D3DSpecularWorldTex._22, _D3DSpecularWorldTex._23);
	swap (_D3DSpecularWorldTex._32, _D3DSpecularWorldTex._33);
	swap (_D3DSpecularWorldTex._42, _D3DSpecularWorldTex._43);
	_D3DSpecularWorldTex._41 = 0;
	_D3DSpecularWorldTex._42 = 0;
	_D3DSpecularWorldTex._43 = 0;

	D3DXMatrixInverse ( &_D3DSpecularWorldTex, NULL, &_D3DSpecularWorldTex);
	swap (_D3DSpecularWorldTex._12, _D3DSpecularWorldTex._13);
	swap (_D3DSpecularWorldTex._22, _D3DSpecularWorldTex._23);
	swap (_D3DSpecularWorldTex._32, _D3DSpecularWorldTex._33);
	swap (_D3DSpecularWorldTex._42, _D3DSpecularWorldTex._43);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setupModelMatrix(const CMatrix& mtx)
{
	H_AUTO_D3D(CDriverD3D_setupModelMatrix)
	// Stats
	_NbSetupModelMatrixCall++;

	// Remeber the model matrix
	_UserModelMtx= mtx;

	D3DXMATRIX world;
	NL_D3D_MATRIX (world, mtx);

	// Remove from position the camera position
	world._41 -= _PZBCameraPos.x;
	world._42 -= _PZBCameraPos.y;
	world._43 -= _PZBCameraPos.z;

	setMatrix (D3DTS_WORLD, world);

	updateMatrices ();
}

// ***************************************************************************

CMatrix CDriverD3D::getViewMatrix() const
{
	H_AUTO_D3D(CDriverD3D_getViewMatrix)
	return _UserViewMtx;
}

// ***************************************************************************

void CDriverD3D::forceNormalize(bool normalize)
{
	H_AUTO_D3D(CDriverD3D_forceNormalize)
	_ForceNormalize = normalize;
	updateMatrices ();
}

// ***************************************************************************

bool CDriverD3D::isForceNormalize() const
{
	H_AUTO_D3D(CDriverD3D_isForceNormalize)
	return _RenderStateCache[D3DRS_NORMALIZENORMALS].Value != FALSE;
}



// ***************************************************************************

void CDriverD3D::setupScissor (const class CScissor& scissor)
{
	H_AUTO_D3D(CDriverD3D_setupScissor )
	if (!_ScissorTouched &&
		_Scissor.X == scissor.X &&
		_Scissor.Y == scissor.Y &&
		_Scissor.Width == scissor.Width &&
		_Scissor.Height == scissor.Height
		) return;
	nlassert (_DeviceInterface);

	// Get viewport
	_ScissorTouched = false;
	float x= scissor.X;
	float width= scissor.Width;
	float height= scissor.Height;

	if(x==0 && x==0 && width==1 && height==1)
	{
		setRenderState (D3DRS_SCISSORTESTENABLE, FALSE);
	}
	else
	{

		float y= scissor.Y;

		if (_HWnd)
		{
			// Get the render target size
			uint32 clientWidth;
			uint32 clientHeight;
			getRenderTargetSize (clientWidth, clientHeight);

			// Setup d3d scissor

			RECT rect;
			rect.left=(int)floor((float)clientWidth * x + 0.5f);
			clamp (rect.left, 0, (int)clientWidth);
			if (_RenderTarget.Texture)
				rect.top=(int)floor((float)clientHeight* y + 0.5f);
			else
				rect.top=(int)floor((float)clientHeight* (1-y-height) + 0.5f);
			clamp (rect.top, 0, (int)clientHeight);

			rect.right=(int)floor((float)clientWidth * (x+width) + 0.5f );
			clamp (rect.right, 0, (int)clientWidth);
			if (_RenderTarget.Texture)
				rect.bottom=(int)floor((float)clientHeight* (y+height) + 0.5f);
			else
				rect.bottom=(int)floor((float)clientHeight* (1-y) + 0.5f);
			clamp (rect.bottom, 0, (int)clientHeight);

			{
				H_AUTO_D3D(CDriverD3D_setupScissorDevice )
				_DeviceInterface->SetScissorRect (&rect);
			}
			setRenderState (D3DRS_SCISSORTESTENABLE, TRUE);

		}
	}

	// Backup the scissor
	_Scissor = scissor;
}

// ***************************************************************************

void CDriverD3D::setupViewport (const class CViewport& viewport)
{
	H_AUTO_D3D(CDriverD3D_setupViewport )
	if (_HWnd == NULL)
		return;

	// Get the render target size
	uint32 clientWidth;
	uint32 clientHeight;
	getRenderTargetSize (clientWidth, clientHeight);

	// Get viewport
	float x;
	float y;
	float width;
	float height;
	viewport.getValues (x, y, width, height);

	// Get integer values
	int ix=(int)((float)clientWidth*x);
	clamp (ix, 0, (int)clientWidth);
	int iy;
	if (_RenderTarget.Texture)
		iy=(int)((float)clientHeight*y);
	else
		iy=(int)((float)clientHeight*(1.f-(y+height)));
	clamp (iy, 0, (int)clientHeight);
	int iwidth=(int)((float)clientWidth*width);
	clamp (iwidth, 0, (int)clientWidth-ix);
	int iheight=(int)((float)clientHeight*height);
	clamp (iheight, 0, (int)clientHeight-iy);

	// Setup D3D viewport
	_D3DViewport.X = ix;
	_D3DViewport.Y = iy;
	_D3DViewport.Width = iwidth;
	_D3DViewport.Height = iheight;
	_D3DViewport.MinZ = _DepthRangeNear;
	_D3DViewport.MaxZ = _DepthRangeFar;
	_DeviceInterface->SetViewport (&_D3DViewport);

	// Backup the viewport
	_Viewport = viewport;

	updateProjectionMatrix ();
}

// ***************************************************************************
void CDriverD3D::setDepthRange(float znear, float zfar)
{
	H_AUTO_D3D(CDriverD3D_setDepthRange)
	nlassert(znear != zfar);
	if (_HWnd == NULL)
		return;
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
	NL_D3D_CACHE_TEST(CacheTest_DepthRange, znear != _DepthRangeNear || zfar != _DepthRangeFar)
#endif
	{
		_DepthRangeNear = znear;
		_DepthRangeFar = zfar;
		_D3DViewport.MinZ = _DepthRangeNear;
		_D3DViewport.MaxZ = _DepthRangeFar;
		_DeviceInterface->SetViewport (&_D3DViewport);
	}
}

// ***************************************************************************
void CDriverD3D::getDepthRange(float &znear, float &zfar) const
{
	H_AUTO_D3D(CDriverD3D_getDepthRange)
	znear = _DepthRangeNear;
	zfar = _DepthRangeFar;
}

// ***************************************************************************

void CDriverD3D::getViewport(CViewport &viewport)
{
	H_AUTO_D3D(CDriverD3D_getViewport)
	viewport = _Viewport;
}

// ***************************************************************************


} // NL3D





















