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
#include "nel/3d/water_env_map_user.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{
//////////////////////
// CWaterEnvMapUser //
//////////////////////

// ***********************************************************************************
void CWaterEnvMapUser::init(uint cubeMapSize, uint projection2DSize, TGlobalAnimationTime updateTime)
{
	EnvMap.init(cubeMapSize, projection2DSize, updateTime, *(EnvMap.Driver->getDriver()));
}

//////////////////////////////
// CWaterEnvMapRenderHelper //
//////////////////////////////


// ***********************************************************************************
void CWaterEnvMapRenderHelper::render(TFace face, TGlobalAnimationTime time, UDriver &drv)
{
	CMatrix mat;
	using NLMISC::CVector;
	switch(face)
	{
		case IWaterEnvMapRender::positive_x: mat.setRot(CVector::J, -CVector::I, -CVector::K); break;
		case IWaterEnvMapRender::negative_x: mat.setRot(-CVector::J, CVector::I, -CVector::K); break;
		case IWaterEnvMapRender::positive_y: mat.setRot(CVector::I, CVector::J, -CVector::K); break;
		case IWaterEnvMapRender::negative_y: mat.setRot(-CVector::I, -CVector::J, -CVector::K); break;
		case IWaterEnvMapRender::positive_z: mat.setRot(-CVector::I, CVector::K, -CVector::J); break;
		case IWaterEnvMapRender::negative_z: mat.setRot(-CVector::I, -CVector::K, CVector::J); break;
		default:
			nlassert(0);
			break;
	}
	doRender(mat, time, drv);
}


//////////////////////////////////
// CWaterEnvMapRenderFromUScene //
//////////////////////////////////

// ***********************************************************************************
CWaterEnvMapRenderFromUScene::CWaterEnvMapRenderFromUScene()
{
	_Scene = NULL;
	_ZNear = 0.1f;
	_ZFar = 1000.f;
	_RenderPart = UScene::RenderAll;
}

// ***********************************************************************************
void CWaterEnvMapRenderFromUScene::setScene(UScene *scene, UCamera cam)
{
	_Scene = scene;
	_Cam = cam;
	if (_Scene)
	{
		nlassert(!_Cam.empty());
	}
}


// ***********************************************************************************
void CWaterEnvMapRenderFromUScene::doRender(const CMatrix &camMatrix, TGlobalAnimationTime time, UDriver &drv)
{
	if (!_Scene) return;
	preRender(time, drv);
	UCamera oldCam = _Scene->getCam();
	if (_Cam.empty()) return;
	_Scene->setCam(_Cam);
	_Cam.setTransformMode(UTransformable::DirectMatrix);
	nlassert(!_Cam.empty());
	_Cam.setFrustum(-_ZNear, _ZNear, -_ZNear, _ZNear, _ZNear, _ZFar);
	drv.setCullMode(drv.getCullMode() == UDriver::CCW ? UDriver::CW : UDriver::CCW);
	CMatrix mat = camMatrix;
	mat.setPos(_CamPos);
	_Cam.setMatrix(mat);
	CViewport old = _Scene->getViewport();
	_Scene->setViewport(CViewport());
	_Scene->beginPartRender();
	_Scene->renderPart(_RenderPart);
	_Scene->endPartRender();
	_Scene->setViewport(old);
	drv.setCullMode(drv.getCullMode() == UDriver::CCW ? UDriver::CW : UDriver::CCW);
	_Scene->setCam(oldCam);
	postRender(time, drv);
}

// ***********************************************************************************
void CWaterEnvMapUser::invalidate()
{
	EnvMap.invalidate();
}

} // NL3D
