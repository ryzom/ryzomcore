// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "water_env_map_rdr.h"
#include "sky.h"
#include "global.h"
#include "view.h"
#include "continent_manager.h"
#include "light_cycle_manager.h"
#include "weather.h"
#include "nel/3d/viewport.h"
#include "nel/3d/u_driver.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vectord.h"

using namespace NL3D;
using namespace NLMISC;


extern CContinentManager ContinentMngr;
extern CFogState RootFogState;

H_AUTO_DECL(RZ_WaterEnvMapRDR)

static void renderEnvMapScene(UScene *scene, UCamera &cam, const CMatrix &camMatrix, float znear, float zfar)
{
	H_AUTO_USE(RZ_WaterEnvMapRDR)
	if (!scene) return;
	if (!cam.empty())
	{
		UCamera oldCam = scene->getCam();
		scene->setCam(cam);
		cam.setTransformMode(UTransformable::DirectMatrix);
		cam.setFrustum(-znear, znear, -znear, znear, znear, zfar);
		cam.setMatrix(camMatrix);
		CViewport old = scene->getViewport();
		scene->setViewport(CViewport());
		scene->beginPartRender();
		scene->renderPart((UScene::TRenderPart) (UScene::RenderOpaque | UScene::RenderTransparent));
		scene->endPartRender();
		scene->setViewport(old);
		scene->setCam(oldCam);
	}
}

void CWaterEnvMapRdr::doRender(const CMatrix &camMatrix, TGlobalAnimationTime time, UDriver &drv)
{
	H_AUTO_USE(RZ_WaterEnvMapRDR)
	drv.clearZBuffer();
	if (time != _LastRenderStartTime) // begin a new rendering of the cube
	{
		_DateForRender = CurrDate;
		_AnimationDateForRender = AnimationDate;
		_WeatherForRender = CurrWeather;
		_FogColorForRender = CurrFogColor;
		_LastRenderStartTime = time;
		_CurrCanopyCamPos = View.currentViewPos();
	}
	if (Sky)
	{
		Sky->setup(_DateForRender, _AnimationDateForRender, _WeatherForRender, _FogColorForRender, CVector(0.5f, 0.f, -0.85f).normed(), true);
		if (CurrTime != -1)
		{
			if (Sky->getScene()) Sky->getScene()->animate(CurrTime); // this will force the elapsed time to be set to  so that fx won't be animated twice
		}
	}
	drv.setCullMode(drv.getCullMode() == UDriver::CCW ? UDriver::CW : UDriver::CCW);
	// render canopy
	ContinentMngr.getFogState(CanopyFog, LightCycleManager.getLightLevel(), LightCycleManager.getLightDesc().DuskRatio, LightCycleManager.getState(), CVectorD(View.viewPos()), RootFogState);
	RootFogState.setupInDriver(drv);
	CMatrix mat = camMatrix;
	drv.setDepthRange(0.f, 0.5f);
	mat.setPos(_CurrCanopyCamPos);
	renderEnvMapScene(SceneRoot, WaterEnvMapCanopyCam, mat, SceneRootCameraZNear, SceneRootCameraZFar);
	Driver->enableFog(false);
	// render sky
	if (Sky)
	{
		drv.setDepthRange(0.5f, 1.f); // force sky scene to appear behind canopy scene
		mat.setPos(CVector(0.f, 0.f, Sky->getWaterEnvMapCameraHeight()));
		renderEnvMapScene(Sky->getScene(), WaterEnvMapSkyCam, mat, 0.1f, 1000.f);
	}
	drv.setCullMode(drv.getCullMode() == UDriver::CCW ? UDriver::CW : UDriver::CCW);
	drv.setDepthRange(0.f, 1.f);
}



