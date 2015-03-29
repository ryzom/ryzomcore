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

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
void CDriverGL3::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	H_AUTO_OGL(CDriverGL3_setFrustum);

	if (perspective)
		_GLProjMat.frustum(left, right, bottom, top, znear, zfar);
	else
		_GLProjMat.ortho(left, right, bottom, top, znear, zfar);

	_OODeltaZ = 1 / (zfar - znear);

}

// ***************************************************************************

void CDriverGL3::setFrustumMatrix(CMatrix &frustumMatrix)
{
	H_AUTO_OGL(CDriverGL3_setFrustum)

	_GLProjMat = frustumMatrix;
}

// ***************************************************************************

CMatrix CDriverGL3::getFrustumMatrix()
{
	H_AUTO_OGL(CDriverGL3_getFrustum)

	return _GLProjMat;
}

// ***************************************************************************
void CDriverGL3::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	H_AUTO_OGL(CDriverGL3_setupViewMatrixEx)
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	_ViewMtx = mtx;
	// Reset the viewMtx position.
	_ViewMtx.setPos(CVector::Null);
	_PZBCameraPos= cameraPos;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
}


// ***************************************************************************
void CDriverGL3::setupViewMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL3_setupViewMatrix)
	_UserViewMtx= mtx;

	_ViewMtx = mtx;
	// Just set the PZBCameraPos to 0.
	_PZBCameraPos= CVector::Null;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
}

// ***************************************************************************
CMatrix CDriverGL3::getViewMatrix(void) const
{
	H_AUTO_OGL(CDriverGL3_getViewMatrix)
	return _UserViewMtx;
}

// ***************************************************************************
void CDriverGL3::setupModelMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL3_setupModelMatrix)
	// profiling
	_NbSetupModelMatrixCall++;

	// Put the matrix in the opengl eye space, and store it.
	CMatrix		mat= mtx;
	// remove first the _PZBCameraPos
	mat.setPos(mtx.getPos() - _PZBCameraPos);
	_ModelViewMatrix= _ViewMtx*mat;
}

} // NLDRIVERGL3
} // NL3D
