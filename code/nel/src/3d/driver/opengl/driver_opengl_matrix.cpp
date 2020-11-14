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

#include "stdopengl.h"
#include "driver_opengl.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************
void CDriverGL::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	H_AUTO_OGL(CDriverGL_setFrustum);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (perspective)
	{
		glFrustum(left,right,bottom,top,znear,zfar);
	}
	else
	{
		glOrtho(left,right,bottom,top,znear,zfar);
	}

	_ProjMatDirty = true;

	// Backup znear and zfar for zbias setup
	_OODeltaZ = 1 / (zfar - znear);

	glMatrixMode(GL_MODELVIEW);
}

// ***************************************************************************

void CDriverGL::setFrustumMatrix(CMatrix &frustumMatrix)
{
	H_AUTO_OGL(CDriverGL_setFrustum)
	glMatrixMode(GL_PROJECTION);

	glLoadMatrixf(((GLfloat*)frustumMatrix.get()));

	glMatrixMode(GL_MODELVIEW);
}

// ***************************************************************************

CMatrix CDriverGL::getFrustumMatrix()
{
	H_AUTO_OGL(CDriverGL_getFrustum)

	glMatrixMode(GL_PROJECTION);

	CMatrix frustumMatrix;
	float frustum[16];
	glGetFloatv(GL_PROJECTION_MATRIX, ((GLfloat*)&frustum));
	frustumMatrix.set(frustum);

	glMatrixMode(GL_MODELVIEW);

	return frustumMatrix;
}

// ***************************************************************************
void CDriverGL::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	H_AUTO_OGL(CDriverGL_setupViewMatrixEx)
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Reset the viewMtx position.
	_ViewMtx.setPos(CVector::Null);
	_PZBCameraPos= cameraPos;

	// Anything that depend on the view martix must be updated.
	_LightSetupDirty= true;
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;
	// All lights must be refresh.
	for(uint i=0;i<MaxLight;i++)
		_LightDirty[i]= true;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
	_SpecularTexMtx = changeBasis *	_SpecularTexMtx;
}


// ***************************************************************************
void CDriverGL::setupViewMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL_setupViewMatrix)
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Just set the PZBCameraPos to 0.
	_PZBCameraPos= CVector::Null;

	// Anything that depend on the view martix must be updated.
	_LightSetupDirty= true;
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;
	// All lights must be refresh.
	for(uint i=0;i<MaxLight;i++)
		_LightDirty[i]= true;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
	_SpecularTexMtx = changeBasis *	_SpecularTexMtx;

}

// ***************************************************************************
CMatrix CDriverGL::getViewMatrix(void) const
{
	H_AUTO_OGL(CDriverGL_getViewMatrix)
	return _UserViewMtx;
}

// ***************************************************************************
void CDriverGL::setupModelMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL_setupModelMatrix)
	// profiling
	_NbSetupModelMatrixCall++;


	// Dirt flags.
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;


	// Put the matrix in the opengl eye space, and store it.
	CMatrix		mat= mtx;
	// remove first the _PZBCameraPos
	mat.setPos(mtx.getPos() - _PZBCameraPos);
	_ModelViewMatrix= _ViewMtx*mat;
}

// ***************************************************************************
void CDriverGL::doRefreshRenderSetup()
{
	H_AUTO_OGL(CDriverGL_doRefreshRenderSetup)
	// Check if the light setup has been modified first
	if (_LightSetupDirty)
		// Recompute light setup
		cleanLightSetup ();

	// Check light setup is good
	nlassert (_LightSetupDirty==false);


	// Check if must update the modelViewMatrix
	if( _ModelViewMatrixDirty )
	{
		// By default, the first model matrix is active
		glLoadMatrixf( _ModelViewMatrix.get() );
		// enable normalize if matrix has scale.
		enableGlNormalize( _ModelViewMatrix.hasScalePart() || _ForceNormalize );
		// clear.
		_ModelViewMatrixDirty= false;
	}

	// render setup is cleaned.
	_RenderSetupDirty= false;
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
