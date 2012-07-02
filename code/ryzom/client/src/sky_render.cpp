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
//
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_instance.h"
//
#include "world_database_manager.h"
#include "continent_manager.h"
#include "weather_manager_client.h"
#include "weather.h"
#include "sky_render.h"
#include "sky_material_setup.h"
#include "light_cycle_manager.h"
#include "global.h"


H_AUTO_DECL(RZ_SkyRender)

using namespace NL3D;
using namespace NLMISC;

UScene    *SkyScene = NULL;
UInstance Sky = NULL;
UInstance   Sky2ndPass = NULL;
NL3D::UInstance SkyFogPart = NULL;

/////////////
// GLOBALS //
/////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Render_Sky )

extern UDriver *Driver;
extern UScene  *Scene;


//===================================================================================================
void createSkyScene()
{
	if (SkyScene) return;
	if (!Scene) return;
	// Create the scene for the sky.
	SkyScene = Driver->createScene(true);
	// enable Scene Lighting
	SkyScene->enableLightingSystem(true);
	SkyScene->setAmbientGlobal(CRGBA::Black);
}

//===================================================================================================
void deleteSkyScene()
{
	if (!SkyScene) return;
	Driver->deleteScene(SkyScene);
	SkyScene = NULL;
}

//===================================================================================================
static void applySkyMaterialSetup(UInstance instance, bool isNight, uint stage, bool skipFirstMaterial = false)
{
	H_AUTO_USE(RZ_SkyRender)
	if(instance.empty())
		return;

	// Return if there is no continent selected.
	if(ContinentMngr.cur() == 0)
		return;

	if(!isNight)
		ContinentMngr.cur()->DaySkySetup.applyToInstance(instance, stage, skipFirstMaterial);
	else
		ContinentMngr.cur()->NightSkySetup.applyToInstance(instance, stage, skipFirstMaterial);
}

//===================================================================================================
static void applySkyTex(UInstance instance, uint stage, const std::string &name)
{
	H_AUTO_USE(RZ_SkyRender)
	if (instance.empty()) return;
	if (instance.getNumMaterials() < 1) return;
	UInstanceMaterial im = instance.getMaterial(0);
	if (im.getLastTextureStage() >= (sint) stage)
	{
		if (NLMISC::nlstricmp(im.getTextureFileName(stage), name) != 0)
		{
			im.setTextureFileName(name, stage);
		}
	}
}
//===================================================================================================
/** Set alpha factors for a sky dome.
  * the constantAlpha gives the blend between 2 sky texture for the instance
  * the diffuseAlpha gives a modulate factor for the sky, which is then blended additively
  */
static void setSkyAlpha(UInstance instance, float constantAlpha, float diffuseAlpha, bool opaque)
{
	H_AUTO_USE(RZ_SkyRender)
	if (instance.empty()) return;
	CRGBA constantCol(255, 255, 255, (uint8) (constantAlpha * 255.f));
	CRGBA diffuseCol(255, 255, 255, (uint8) (diffuseAlpha * 255.f));

	uint numMat = instance.getNumMaterials();
	if (numMat == 0) return;
	for(uint k = 0; k < numMat; ++k)
	{
		instance.getMaterial(k).setConstantColor(1, constantCol);
		instance.getMaterial(k).setColor(diffuseCol);
	}

	// set the first material to be opaque
	instance.getMaterial(0).setDstBlend(opaque ? UInstanceMaterial::zero : UInstanceMaterial::one);
}


//===================================================================================================
static inline void showSkyPart(UInstance i, bool show)
{
	H_AUTO_USE(RZ_SkyRender)
	if (i.empty()) return;
	if (show) i.show();
	else i.hide();
}

//===================================================================================================
static void showSkyParts(bool firstPass, bool secondPass, bool fogPart)
{
	H_AUTO_USE(RZ_SkyRender)
	showSkyPart(Sky, firstPass);
	showSkyPart(Sky2ndPass, secondPass);
	showSkyPart(SkyFogPart, fogPart);
}

//===================================================================================================
static void renderStandardSky(float dayNight)
{
	H_AUTO_USE(RZ_SkyRender)
	showSkyParts(true, false, false);
	// normal setup : stage 0 is day and stage 1 is night, 1 pass is required
	applySkyMaterialSetup(Sky, false, 0); // duplicate current setup in both stages
	applySkyMaterialSetup(Sky, true, 1);
	setSkyAlpha(Sky, dayNight, 1.f, true); // replace mode
	SkyScene->render();
}

//===================================================================================================
/** setup a sky dome for a blend between night & day, with eventually a weather blend
  */
static void setupSkyDomeTextures(UInstance instance, const std::string &weatherDay, const std::string &weatherNight)
{
	H_AUTO_USE(RZ_SkyRender)
	if (instance.empty()) return;
	// setup stage 0 (day)
	if (weatherDay.empty())
	{
		applySkyMaterialSetup(instance, false, 0);
	}
	else
	{
		applySkyMaterialSetup(instance, false, 0, true);
		applySkyTex(instance, 0, weatherDay);
	}
	// setup stage 1 (night)
	if (weatherNight.empty())
	{
		applySkyMaterialSetup(instance, true, 1);
	}
	else
	{
		applySkyMaterialSetup(instance, true, 1, true);
		applySkyTex(instance, 1, weatherNight);
	}
}

//===================================================================================================
/** Set constant color for all material of an instance
  */
static void setInstanceConstantColor(UInstance instance, CRGBA color)
{
	H_AUTO_USE(RZ_SkyRender)
	if (instance.empty()) return;
	uint numMat = instance.getNumMaterials();
	for(uint k = 0; k < numMat; ++k)
	{
		instance.getMaterial(k).setConstantColor(0, color);
	}
}

//===================================================================================================
/** remove all textures from an instance (to free memory..)
  */
static void removeInstanceTextures(UInstance instance)
{
	H_AUTO_USE(RZ_SkyRender)
	if (instance.empty()) return;
	uint numMat = instance.getNumMaterials();
	for(uint k = 0; k < numMat; ++k)
	{
		sint numStages = instance.getMaterial(k).getLastTextureStage() + 1;
		for(sint l = 0; l < numStages; ++l)
		{
			if (instance.getMaterial(k).isTextureFile((uint) l))
			{
				instance.getMaterial(k).setTextureFileName("", (uint) l);
			}
		}
	}
}


//===================================================================================================
void renderSky(const CLightCycleManager &lcm, NLMISC::CRGBA fogColor)
{
	H_AUTO_USE(RZ_SkyRender)
	H_AUTO_USE ( RZ_Client_Render_Sky )

	static bool secondPassUsed = true; // tells if a second pass was needed at the previous rendering
	if (!SkyScene || MainCam.empty() || !Scene) return;
	//
	float lightLevel = lcm.getLightLevel();
	float duskRatio  = lcm.getLightDesc().DuskRatio;
	//
	Driver->enableFog(false);
	// Render the Sky.
	CFrustum frust = MainCam.getFrustum();
	UCamera camSky = SkyScene->getCam();
	camSky.setTransformMode(UTransform::DirectMatrix);
	// must have our own Far!!!
	frust.Far= SkyCameraZFar;
	camSky.setFrustum(frust);
	CMatrix skyCameraMatrix;
	skyCameraMatrix.identity();
	skyCameraMatrix= MainCam.getMatrix();
	skyCameraMatrix.setPos(CVector::Null);
	camSky.setMatrix(skyCameraMatrix);
	SkyScene->setViewport(Scene->getViewport());
	bool isNight = lightLevel > 0.5f;
	if (Sky.empty()) return;
	// See in the weather setup if there is a weather related skyDome
	const CWeatherState &ws = WeatherManager.getCurrWeatherState();

	bool twoPassDone = false;

	// build a set of 4 background between which to blend depending on weather & hour
	//                  00    01
	// bg for setup 0    +-----+    ---> background depending on hour (day, dusk, night)
	//                   |     |
	//                   |     |
	//  bg for setup 1   +-----+
	//                   10    11
	const std::string *bg00, *bg01, *bg10, *bg11;
	float blendFactor; // blendFactor for time
	switch (lcm.getState())
	{
		case CLightCycleManager::DayToNight:
			if (lightLevel <= duskRatio)
			{
				blendFactor = duskRatio != 0 ? lightLevel / duskRatio : 0.f;
				bg00 = &ws.DayBackgroundFileName1;
				bg01 = &ws.DuskBackgroundFileName1;
				bg10 = &ws.DayBackgroundFileName2;
				bg11 = &ws.DuskBackgroundFileName2;
			}
			else
			{
				blendFactor = duskRatio != 1.f ? (lightLevel - duskRatio) / (1.f - duskRatio) : 0.f;
				bg00 = &ws.DuskBackgroundFileName1;
				bg01 = &ws.NightBackgroundFileName1;
				bg10 = &ws.DuskBackgroundFileName2;
				bg11 = &ws.NightBackgroundFileName2;
			}
		break;
		default: // not a day->night transition, so no 'dusk' step.
			blendFactor = lightLevel;
			bg00 = &ws.DayBackgroundFileName1;
			bg01 = &ws.NightBackgroundFileName1;
			bg10 = &ws.DayBackgroundFileName2;
			bg11 = &ws.NightBackgroundFileName2;
		break;
	}

	// Do not draw the sky in indoor continents
	if (!ContinentMngr.cur()->Indoor)
	{
		if (lightLevel == 0.f || lightLevel == 1.f)
		{
			const std::string &bg1 = isNight ? *bg01 : *bg00;
			const std::string &bg2 = isNight ? *bg11 : *bg10;
			if (!bg1.empty() || !bg2.empty())
			{

				if (!bg1.empty())
				{
					if (!bg2.empty()) // transition between 2 weather sky domes ?
					{
						applySkyMaterialSetup(Sky, isNight, 0, true); // duplicate current setup in both stages
						applySkyMaterialSetup(Sky, isNight, 1, true);
						// override sky dome texture with current texture
						applySkyTex(Sky, 0, bg1);
						applySkyTex(Sky, 1, bg2);
						//
					}
					else
					{
						// transition from bad weather sky to normal sky
						applySkyMaterialSetup(Sky, isNight, 0, true); // duplicate current setup in both stages
						applySkyMaterialSetup(Sky, isNight, 1);
						applySkyTex(Sky, 0, bg1);
					}
				}
				else
				{
					// transition from normal weather to bad weather
					applySkyMaterialSetup(Sky, isNight, 0); // duplicate current setup in both stages
					applySkyMaterialSetup(Sky, isNight, 1, true);
					applySkyTex(Sky, 1, bg2);
				}
				showSkyParts(true, false, false);
				setSkyAlpha(Sky, ws.BlendFactor, 1.f, true);
				SkyScene->render();
			}
			else
			{
				renderStandardSky(blendFactor);
			}
		}
		else // We are in a transition between day and night
		{
			// if there's one weather texture, multipass rendering is needed
			if (   bg00->empty()
				&& bg01->empty()
				&& bg10->empty()
				&& bg11->empty())
			{
				// no weather texture
				renderStandardSky(blendFactor);
			}
			else // 2 pass rendering
			{
				// setup textures of the skydomes
				setupSkyDomeTextures(Sky, *bg00, *bg01);
				setupSkyDomeTextures(Sky2ndPass, *bg10, *bg11);
				setSkyAlpha(Sky, blendFactor, 1.f - ws.BlendFactor, true /* opaque */);
				setSkyAlpha(Sky2ndPass, blendFactor, ws.BlendFactor, false /* alpha additif */);
				showSkyParts(true, false, false);
				SkyScene->render();
				showSkyParts(false, true, false);
				SkyScene->render();
				twoPassDone = true;
				secondPassUsed = true;
			}
		}
		// Blend the fog part
		if (!SkyFogPart.empty())
		{
			showSkyParts(false, false, true);
			setInstanceConstantColor(SkyFogPart, fogColor);
			// set texture matrix for first material to get the right height for fog
			if (SkyFogPart.getNumMaterials() > 0)
			{
				UInstanceMaterial im = SkyFogPart.getMaterial(0);
				im.enableUserTexMat(0);
				CMatrix uvMat;
				uvMat.scale(CVector(1.f, ws.FogGradientFactor != 0.f ? 1.f / ws.FogGradientFactor : 0.f, 1.f)); // scale the fog part
				im.setUserTexMat(0, uvMat);
			}
			SkyScene->render();
		}
		// release textures for second pass if it is not needed anymore
		if (!twoPassDone && secondPassUsed)
		{
			removeInstanceTextures(Sky2ndPass);
			secondPassUsed = false;
		}
	}
}

