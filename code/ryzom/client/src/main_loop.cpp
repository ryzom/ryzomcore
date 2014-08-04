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


/////////////
// INCLUDE //
/////////////


#include "stdpch.h"

// Misc.
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/displayer.h"
#include "nel/misc/value_smoother.h"
#include "nel/misc/geom_ext.h"
// Net
#include "nel/net/module_manager.h"
// 3D Interface.
#include "nel/3d/bloom_effect.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_cloud_scape.h"
#include "nel/3d/stereo_hmd.h"
#include "nel/3d/render_target_manager.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/fxaa.h"
// game share
#include "game_share/brick_types.h"
#include "game_share/light_cycle.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/ryzom_version.h"
#include "game_share/bot_chat_types.h"
// PACS
#include "nel/pacs/u_global_position.h"
// client sheets
#include "client_sheets/weather_function_params_sheet.h"
// std
#include <string>
// Client
#include "game_share/constants.h"
#include "main_loop.h"
#include "input.h"
#include "client_cfg.h"
#include "actions_client.h"
#include "motion/user_controls.h"
#include "entity_animation_manager.h"
#include "pacs_client.h"
#include "view.h"
#include "time_client.h"
#include "cursor_functions.h"
#include "pacs_client.h"
#include "entity_fx.h"
#include "light_cycle_manager.h"
#include "weather_manager_client.h"
#include "weather.h"
#include "game_share/time_weather_season/weather_predict.h"
#include "entities.h"
#include "net_manager.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "ig_callback.h"
//#include "fog_map.h"
#include "movie_shooter.h"
#include "sound_manager.h"
#include "graph.h"
#include "interface_v3/interface_manager.h"
#include "demo.h"
#include "color_slot_manager.h"
#include "interface_v3/input_handler_manager.h"
#include "ingame_database_manager.h"
#include "sky_render.h"
#include "prim_file.h"
#include "misc.h"
#include "interface_v3/people_interraction.h"
#include "debug_client.h"
#include "nel/gui/action_handler.h"
#include "interface_v3/action_handler_misc.h"
#include "interface_v3/action_handler_item.h"
#include "fx_manager.h"
#include "ground_fx_manager.h"
#include "string_manager_client.h"
#include "interface_v3/group_in_scene_bubble.h"
#include "game_context_menu.h"
#include "init_main_loop.h"
#include "micro_life_manager.h"
#include "timed_fx_manager.h"
#include "interface_v3/sphrase_manager.h"
#include "outpost_manager.h"
#include "sky.h" // new-style sky
#include "sky_render.h" // new-style sky
#include "interface_v3/music_player.h"
#include "permanent_ban.h"
#include "camera_recorder.h"
#include "connection.h"
#include "landscape_poly_drawer.h"
#include "nel/gui/lua_ihm.h"
#include "interface_v3/lua_ihm_ryzom.h"
#include "far_tp.h"
#include "session_browser_impl.h"
#include "bg_downloader_access.h"
#include "login_progress_post_thread.h"
#include "npc_icon.h"

// R2ED
#include "r2/editor.h"

#include "nel/misc/check_fpu.h"



#ifdef USE_WATER_ENV_MAP
	#include "water_env_map_rdr.h"
#endif

// temp
#include "precipitation.h"
#include "interface_v3/bot_chat_manager.h"
#include "string_manager_client.h"

#include "nel/gui/lua_manager.h"
#include "nel/gui/group_table.h"

// pulled from main_loop.cpp
#include "ping.h"
#include "profiling.h"
#include "camera.h"
#include "main_loop_debug.h"
#include "main_loop_temp.h"
#include "main_loop_utilities.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace NLNET;
using namespace std;





////////////
// EXTERN //
////////////
extern UDriver					*Driver;
extern UScene					*Scene;
extern UScene					*SceneRoot;
extern ULandscape				*Landscape;
extern UCloudScape				*CloudScape;
extern bool						InitCloudScape;
extern CLandscapeIGManager		LandscapeIGManager;
extern UTextContext				*TextContext;
extern CEntityAnimationManager	*EAM;
extern TTime					UniversalTime;
extern UMaterial				GenericMat;
extern UCamera					MainCam;
extern CEventsListener			EventsListener;
extern CMatrix					MainSceneViewMatrix;
extern CMatrix					InvMainSceneViewMatrix;
extern std::vector<UTextureFile*> LogoBitmaps;
extern bool						IsInRingSession;
extern std::string				UsedFSAddr;

// temp
extern NLMISC::CValueSmoother smoothFPS;
extern NLMISC::CValueSmoother moreSmoothFPS;

void loadBackgroundBitmap (TBackground background);
void destroyLoadingBitmap ();
void drawLoadingBitmap (float progress);
void updateStatReport ();

CFogState						MainFogState;
CFogState						RootFogState;


#define RYZOM_FIRST_FRAME_TO_SKIP 30


const float CANOPY_DEPTH_RANGE_START = 0.95f;
const float SKY_DEPTH_RANGE_START = 0.99f;

CRGBA ThunderColor;

float  SimulatedServerDate = 0.f;
uint64 SimulatedServerTick = 0;




/////////////
// GLOBALS //
/////////////
bool				game_exit = false;
bool				ryzom_exit = false;
bool				game_exit_request = false;
bool				ryzom_exit_request = false;
bool				paying_account_request = false;
bool paying_account_already_request = false;
bool game_exit_after_paying_account_request = false;
bool				Render = true;

float				MouseX;			// Mouse pos X for the frame.
float				MouseY;			// Mouse pos Y for the frame.
float				OldMouseX;		// Mouse pos X of the last frame.
float				OldMouseY;		// Mouse pos Y of the last frame.

uint32				Width;			// Width of the window.
uint32				Height;			// Height of the window.
uint32				OldWidth;		// Last Width of the window.
uint32				OldHeight;		// Last Height of the window.

bool				ShowInterface = true;	// Do the Chat OSD have to be displayed.
bool				DebugUIView = false;
bool				DebugUICtrl = false;
bool				DebugUIGroup = false;
std::string			DebugUIFilter;
bool				ShowHelp = false;	// Do the Help have to be displayed.
uint8				ShowInfos = 0;		// 0=no info 1=text info 2=graph info

bool				bZeroCpu = false;	// For no Cpu use if application is minimize  TODO: intercept minimize message, called by CTRL + Z at this

bool				MovieShooterSaving= false;	// Are we in Shooting mode?


bool				DisplayWeatherFunction = false;

// temp
float				DelayBeforeCloudUpdate = 0.f; // delay in s before cloud state must be recomputed
const float			CloudUpdatePeriod = 45.f; // period used for clouds update

TScreenshotRequest  ScreenshotRequest = ScreenshotRequestNone;


// First frames to skip
bool				FirstFrame = false;
uint				SkipFrame = 0;

// temp : for timed fxs test
bool				ShowTimedFX = false;
CTimedFXManager::TDebugDisplayMode	ShowTimedFXMode = CTimedFXManager::NoText;

//CGraph				FpsGraph ("frame rate (fps)", 10.0f, 110.0f, 100.0f, 100.0f, CRGBA(128,0,0,128), 1000, 60.0f);
//CGraph				SpfGraph ("mspf", 10.0f, 10.0f, 100.0f, 100.0f, CRGBA(0,128,0,128), 0, 800.0f);
// TestYoyo
//CGraph CameraThirPersonGraph ("Camera Thir Person", 300.0f, 460.0f, 200.0f, 200.0f, CRGBA(0,64,128,128), 0, 5.0f, 1);

// DEBUG
bool				PACSBorders = false;
bool				DebugClusters = false;
CVector				LastDebugClusterCameraThirdPersonStart= CVector::Null;
CVector				LastDebugClusterCameraThirdPersonEnd= CVector::Null;
CVector				LastDebugClusterCameraThirdPersonTestStart= CVector::Null;
CVector				LastDebugClusterCameraThirdPersonPelvisPos= CVector::Null;
CVector				LastDebugClusterCameraThirdPersonResult= CVector::Null;
bool				LastDebugClusterCameraThirdPersonForceFPV= false;
bool				SoundBox = false;
CPing				Ping;
sint				CompassMode = 0;	// 0: compass with regard to the user front, 1: to the camera direction.

float				BanMsgCountdown = 0.f;
const float         BanMsgRepeatTime = 5.f;

CGameContextMenu	GameContextMenu;





static CRefPtr<CCDBNodeLeaf> s_FpsLeaf;
static CRefPtr<CCDBNodeLeaf> s_UiDirectionLeaf;


// Profile
/*
0 : AllMeshNoVP
1 : AllMeshVP
2 : FX
3 : Landscape
4 : Vegetable
5 : Skeleton
6 : Water
7 : Cloud
8 : CoarseMesh*/
bool				Filter3D[RYZOM_MAX_FILTER_3D] = {true, true, true, true, true, true, true, true, true, true};
bool				Scene_Profile= false;



// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Check_Actions )
H_AUTO_DECL ( RZ_Client_Main_Loop )
H_AUTO_DECL ( RZ_Client_Main_Loop_Zero_Cpu )
H_AUTO_DECL ( RZ_Client_Main_Loop_Cursor )
H_AUTO_DECL ( RZ_Client_Main_Loop_Time_Update )
H_AUTO_DECL ( RZ_Client_Main_Loop_Selection_FX )
H_AUTO_DECL ( RZ_Client_Main_Loop_Sky_And_Weather )
H_AUTO_DECL ( RZ_Client_Main_Loop_Render_Root )
H_AUTO_DECL ( RZ_Client_Main_Loop_Render_Main )
H_AUTO_DECL ( RZ_Client_Main_Loop_Anim_Cloud_Scape )
H_AUTO_DECL ( RZ_Client_Main_Loop_Update_Cloud_Scape )
H_AUTO_DECL ( RZ_Client_Main_Loop_Render_Cloud_Scape )
H_AUTO_DECL ( RZ_Client_Main_Loop_Debug )
H_AUTO_DECL ( RZ_Client_Main_Loop_Guild_Symbol )
H_AUTO_DECL ( RZ_Client_Main_Loop_Render_Thunder )
H_AUTO_DECL ( RZ_Client_Main_Loop_Interface )
H_AUTO_DECL ( RZ_Client_Main_Loop_Sound )
H_AUTO_DECL ( RZ_Client_Main_Loop_Net )

///////////////
// FUNCTIONS //
///////////////

//update the sound manager (listener pos, user walk/run sound...)
void updateSound();

void displaySpecialTextProgress(const char *text);

void endMovieShooting();
void updateMovieShooting();

void updateLightDesc();
void updateClouds();

void displayPACSBorders();
void displayPACSPrimitive();
void displaySoundBox();
void displayDebugClusters();

void displaySceneProfiles();

// validate current dialogs (end them if the player is too far from its interlocutor)
void validateDialogs(const CGameContextMenu &gcm);


// ***************************************************************************

enum TSkyMode { NoSky, OldSky, NewSky };
static TSkyMode s_SkyMode = NoSky;

void preRenderNewSky ()
{
	CSky &sky = ContinentMngr.cur()->CurrentSky;
	// setup fog, lighting & sky object. We use the same light direction than with the main scene
	CClientDate cd = SmoothedClientDate;
	if (ClientCfg.R2EDEnabled && R2::getEditor().getFixedLighting())
	{
		cd.Hour = 12.f;
	}
	sky.setup(cd, SmoothedClientDate, WeatherManager.getWeatherValue(), MainFogState.FogColor, Scene->getSunDirection(), false);
}

void commitCamera()
{
	// Set the sky camera
	if (s_SkyMode == NewSky)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		// setup camera
		CFrustum frust = MainCam.getFrustum();
		UCamera camSky = sky.getScene()->getCam();
		sky.getScene()->setViewport(Scene->getViewport());
		camSky.setTransformMode(UTransform::DirectMatrix);
		// must have our own Far!!!
		frust.Far= SkyCameraZFar;
		camSky.setFrustum(frust);
		CMatrix skyCameraMatrix;
		skyCameraMatrix.identity();
		skyCameraMatrix= MainCam.getMatrix();
		skyCameraMatrix.setPos(CVector::Null);
		camSky.setMatrix(skyCameraMatrix);
	}

	// Set The Root Camera
	UCamera camRoot = SceneRoot->getCam();
	if(!camRoot.empty())
	{
		// Update Camera Position/Rotation.
		//camRoot.setPos(View.currentViewPos());
		//camRoot.setRotQuat(View.currentViewQuat());
		camRoot.setPos(MainCam.getPos());
		camRoot.setRotQuat(MainCam.getRotQuat());
	}
}

// ***************************************************************************

void	beginRenderCanopyPart()
{
	SceneRoot->beginPartRender();
}
void	endRenderCanopyPart()
{
	SceneRoot->endPartRender(false);
}

void	beginRenderMainScenePart()
{
	Scene->beginPartRender();
}
void	endRenderMainScenePart()
{
	Scene->endPartRender(true);
}

void	beginRenderSkyPart()
{
	if (s_SkyMode == NewSky)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		sky.getScene()->beginPartRender();
	}
}
void	endRenderSkyPart()
{
	if (s_SkyMode == NewSky)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		sky.getScene()->endPartRender(false);
	}
}


// ***************************************************************************************************************************
// Render a part of the canopy
static void renderCanopyPart(UScene::TRenderPart renderPart)
{
	H_AUTO_USE ( RZ_Client_Main_Loop_Render_Root )
	Driver->setDepthRange(CANOPY_DEPTH_RANGE_START, SKY_DEPTH_RANGE_START);

	ContinentMngr.getFogState(CanopyFog, LightCycleManager.getLightLevel(), LightCycleManager.getLightDesc().DuskRatio, LightCycleManager.getState(), View.viewPos(), RootFogState);
	RootFogState.setupInDriver(*Driver);

	// Render the root scene
	SceneRoot->renderPart(renderPart);
}

// ***************************************************************************************************************************
// Render a part of the main scene
static void renderMainScenePart(UScene::TRenderPart renderPart)
{
	H_AUTO_USE ( RZ_Client_Main_Loop_Render_Main )
	Driver->setDepthRange(0.f, CANOPY_DEPTH_RANGE_START);
	if(ClientCfg.Fog == false )
	{
			Driver->enableFog (false);
	}
	else
	{
		MainFogState.setupInDriver (*Driver);
	}
	Scene->renderPart(renderPart);
}


// ***************************************************************************************************************************
// Render a part of the sky
static void renderSkyPart(UScene::TRenderPart renderPart)
{
	nlassert(s_SkyMode != NoSky);
	Driver->setDepthRange(SKY_DEPTH_RANGE_START, 1.f);
	Driver->enableFog(false);
	if (s_SkyMode == NewSky)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		sky.getScene()->renderPart(renderPart);
	}
	else
	{
		// old style sky
		renderSky(LightCycleManager, MainFogState.FogColor);
	}
	#ifdef RENDER_CLOUDS
		if (CloudScape != NULL && Filter3D[FilterCloud])
		{
			H_AUTO_USE( RZ_Client_Main_Loop_Render_Cloud_Scape )
				CloudScape->render ();
		}
	#endif
}

// ***************************************************************************************************************************
// Utility to force full detail
struct CForceFullDetail
{
public:
	void backup()
	{
		maxFullDetailChar = Scene->getMaxSkeletonsInNotCLodForm();
		oldBalancingMode = Scene->getPolygonBalancingMode();
		oldSkyBalancingMode = UScene::PolygonBalancingOff;
		UScene *skyScene = getSkyScene();
		if (skyScene) oldSkyBalancingMode = skyScene->getPolygonBalancingMode();
	}
	void set()
	{
		Scene->setMaxSkeletonsInNotCLodForm(1000000);
		Scene->setPolygonBalancingMode(UScene::PolygonBalancingOff);
		UScene *skyScene = getSkyScene();
		if (skyScene) skyScene->setPolygonBalancingMode(UScene::PolygonBalancingOff);
	}
	void restore()
	{
		Scene->setMaxSkeletonsInNotCLodForm(maxFullDetailChar);
		Scene->setPolygonBalancingMode(oldBalancingMode);
		UScene *skyScene = getSkyScene();
		if (skyScene) skyScene->setPolygonBalancingMode(oldSkyBalancingMode);
	}
private:
	uint maxFullDetailChar;
	UScene::TPolygonBalancingMode oldBalancingMode;
	UScene::TPolygonBalancingMode oldSkyBalancingMode;
};
static CForceFullDetail s_ForceFullDetail;

void clearBuffers()
{
	if (Render)
	{
		if (Driver->getPolygonMode() == UDriver::Filled)
		{
			Driver->clearZBuffer();
		}

		// Sky is used to clear the frame buffer now, but if in line or point polygon mode, we should draw it
		if (Driver->getPolygonMode() != UDriver::Filled)
		{
			if (!Driver->isLost())
			{
				Driver->clearBuffers (CRGBA(127, 127, 127));
			}
		}
	}
	else
	{
		Driver->clearBuffers(ClientCfg.BGColor);
	}
}

void renderScene(bool forceFullDetail, bool bloom)
{
	CTextureUser *effectRenderTarget = NULL;
	if (bloom)
	{
		// set bloom parameters before applying bloom effect
		CBloomEffect::getInstance().setSquareBloom(ClientCfg.SquareBloom);
		CBloomEffect::getInstance().setDensityBloom((uint8)ClientCfg.DensityBloom);

		// init effect render target
		Driver->beginDefaultRenderTarget();
	}
	if (forceFullDetail)
	{
		s_ForceFullDetail.backup();
		s_ForceFullDetail.set();
	}
	clearBuffers();
	renderScene();
	if (forceFullDetail)
	{
		s_ForceFullDetail.restore();
	}
	if (bloom)
	{
		// apply bloom effect
		CBloomEffect::getInstance().applyBloom();

		// draw final result to backbuffer
		Driver->endDefaultRenderTarget(Scene);
	}
}

// ***************************************************************************************************************************

void updateWaterEnvMap()
{
	#ifdef USE_WATER_ENV_MAP
	if (WaterEnvMapRefCount > 0) // water env map needed
	{
		if (!WaterEnvMap)
		{
			CSky &sky = ContinentMngr.cur()->CurrentSky;
			if (sky.getScene())
			{
				nlassert(WaterEnvMapSkyCam.empty());
				WaterEnvMapSkyCam = sky.getScene()->createCamera(); // deleted in unselect
				nlassert(WaterEnvMapCanopyCam.empty());
				WaterEnvMapCanopyCam = SceneRoot->createCamera(); // deleted in unselect
				// Create water env map if not already created
				WaterEnvMap = Driver->createWaterEnvMap();
				if(WaterEnvMap)
				{
					WaterEnvMap->init(128, 256, ClientCfg.WaterEnvMapUpdateTime);
					WaterEnvMap->setWaterEnvMapRenderCallback(&WaterEnvMapRdr);
					Scene->setWaterEnvMap(WaterEnvMap);
				}
			}
		}
		WaterEnvMapRdr.CurrDate = SmoothedClientDate;
		WaterEnvMapRdr.AnimationDate = SmoothedClientDate;
		if (ClientCfg.R2EDEnabled && R2::getEditor().getFixedLighting())
		{
			WaterEnvMapRdr.CurrDate.Hour = 12.f;
		}
		WaterEnvMapRdr.CurrFogColor = MainFogState.FogColor;
		WaterEnvMapRdr.CurrTime = TimeInSec - FirstTimeInSec;
		WaterEnvMapRdr.CurrWeather = WeatherManager.getWeatherValue();
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		WaterEnvMap->setAlpha(sky.getWaterEnvMapAlpha());
		Scene->updateWaterEnvMaps(TimeInSec - FirstTimeInSec);
	}
	#endif
}

// ***************************************************************************************************************************

void updateWeather()
{
	H_AUTO_USE ( RZ_Client_Main_Loop_Sky_And_Weather )

	//HeightGrid.update(Scene->getCam().getPos());

	// update description of light cycle
	updateLightDesc();

	// server driven weather mgt
	updateDBDrivenWeatherValue();

	// Update the weather manager
	updateWeatherManager(MainCam.getMatrix(), ContinentMngr.cur());

	// compute thunder color
	ThunderColor.modulateFromui(WeatherManager.getCurrWeatherState().ThunderColor, (uint) (256.f * WeatherManager.getThunderLevel()));

	// Update the lighting
	LightCycleManager.setHour(DayNightCycleHour, WeatherManager, ThunderColor);

	#ifdef RENDER_CLOUDS
	if (Filter3D[FilterCloud])
	{
		H_AUTO_USE ( RZ_Client_Main_Loop_Update_Cloud_Scape );
		updateClouds();
	}
	#endif

	ContinentMngr.getFogState(MainFog, LightCycleManager.getLightLevel(), LightCycleManager.getLightDesc().DuskRatio, LightCycleManager.getState(), View.viewPos(), MainFogState);

	// TODO: ZBuffer clear was originally before this, but should not be necessary normally.
	// The anim function renders new clouds. Ensure this does not break.
	// These are old-style nel clouds.

	#ifdef RENDER_CLOUDS
	if (CloudScape != NULL && Filter3D[FilterCloud])
	{
		H_AUTO_USE ( RZ_Client_Main_Loop_Anim_Cloud_Scape );

		Driver->enableFog (false);

		// Force polygon mode to filled
		NL3D::UDriver::TPolygonMode oldMode = Driver->getPolygonMode();
		Driver->setPolygonMode(NL3D::UDriver::Filled);

		CloudScape->anim (DT); // WARNING this function work with screen

		Driver->enableFog (true);

		// Reset backuped polygon mode
		Driver->setPolygonMode(oldMode);
	}
	#endif

	// Update new sky
	s_SkyMode = NoSky;
	if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
	{
		if(Driver->getPolygonMode() == UDriver::Filled)
		{
			if (Filter3D[FilterSky])
			{
				CSky &sky = ContinentMngr.cur()->CurrentSky;
				if (sky.getScene())
				{
					s_SkyMode = NewSky;
					sky.getScene()->animate(TimeInSec-FirstTimeInSec);
					// Setup the sky camera
					preRenderNewSky();
				}
				else
				{
					s_SkyMode = OldSky;
				}
			}
		}
	}
}

// ***************************************************************************************************************************
// Render all scenes
void renderScene()
{
	// Update Filter Flags
	Scene->enableElementRender(UScene::FilterAllMeshNoVP, Filter3D[FilterMeshNoVP]);
	Scene->enableElementRender(UScene::FilterAllMeshVP, Filter3D[FilterMeshVP]);
	Scene->enableElementRender(UScene::FilterFX, Filter3D[FilterFXs]);
	Scene->enableElementRender(UScene::FilterLandscape, Filter3D[FilterLandscape]);
	Scene->enableElementRender(UScene::FilterSkeleton, Filter3D[FilterSkeleton]);
	Scene->enableElementRender(UScene::FilterWater, Filter3D[FilterWater]);
	Scene->enableElementRender(UScene::FilterCoarseMesh, Filter3D[FilterCoarseMesh]);

	// profile this frame?
	if(Scene_Profile)
	Scene->profileNextRender();

	// initialisation of polygons renderer
	CLandscapePolyDrawer::getInstance().beginRenderLandscapePolyPart();

	// Start Part Rendering
	beginRenderCanopyPart();
	beginRenderMainScenePart();
	beginRenderSkyPart();
	// Render part
	// WARNING: always must begin rendering with at least UScene::RenderOpaque,
	// else dynamic shadows won't work
	renderCanopyPart(UScene::RenderOpaque);
	renderMainScenePart(UScene::RenderOpaque);

	// render of polygons on landscape
	CLandscapePolyDrawer::getInstance().renderLandscapePolyPart();

	if (s_SkyMode != NoSky) renderSkyPart((UScene::TRenderPart) (UScene::RenderOpaque | UScene::RenderTransparent));
	renderCanopyPart((UScene::TRenderPart) (UScene::RenderTransparent | UScene::RenderFlare));
	renderMainScenePart((UScene::TRenderPart) (UScene::RenderTransparent | UScene::RenderFlare));
	if (s_SkyMode == NewSky) renderSkyPart(UScene::RenderFlare);
	// End Part Rendering
	endRenderSkyPart();
	endRenderMainScenePart();
	endRenderCanopyPart();

	// reset depth range
	Driver->setDepthRange(0.f, CANOPY_DEPTH_RANGE_START);
}


// ***************************************************************************
class CMusicFader
{
public:
	uint	NFrameSkip;
	float	TotalTime;
	bool	Done;

public:
	CMusicFader(uint nframeToSkip, float fadeTime)
	{
		NFrameSkip= nframeToSkip;
		// avoid div by zero
		fadeTime= max(fadeTime, 0.01f);
		TotalTime= fadeTime;
		Done= false;
	}

	void	fade();
};
void	CMusicFader::fade()
{
	// ended?
	if(NFrameSkip==0 && Done)
		return;

	// else fade
	if(NFrameSkip==0)
	{
		// stop music (slow fade out of 3 secondes)
		if(SoundMngr)
			SoundMngr->stopMusic(uint(TotalTime*1000));
		Done= true;
	}
	else
		NFrameSkip--;
}

// ***************************************************************************
void	updateGameQuitting()
{
	static	sint64	firstTimeLostConnection= 0;

	// Yoyo: prefer now leave the user press "Quit Now" if don't want to wait server
	/*
	// if want quiting, and if server stalled, quit now
	if(game_exit_request)
	{
		// abort until 10 seconds if connexion lost
		if(!NetMngr.getConnectionQuality())
		{
			if(!firstTimeLostConnection)
				firstTimeLostConnection= T1;
		}
		else
		{
			firstTimeLostConnection= 0;
		}

		// if connexion lost until 10 seconds
		if(firstTimeLostConnection && T1-firstTimeLostConnection > 10000)
		{
			game_exit= true;
			ryzom_exit= ryzom_exit_request;
		}
	}
	else
	{
		// reset
		firstTimeLostConnection= 0;
	}*/

	// update the window
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:free_trial_game_quitting"));
	if(group)
	{
		// if Free trial
		if(paying_account_request)
		{
			// if no current modal window, or if not the quit window
			if(group != CWidgetManager::getInstance()->getModalWindow())
			{
				// disable
				CWidgetManager::getInstance()->disableModalWindow();
				CWidgetManager::getInstance()->enableModalWindow(NULL, group);
			}
		}

		else
		{
			// if the current modal window is the quit window, disable
			if(group == CWidgetManager::getInstance()->getModalWindow())
			{
				// disable
				CWidgetManager::getInstance()->disableModalWindow();
			}
		}
	}

	group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_quitting"));
	if(group)
	{
		// if exit request
		if(game_exit_request && !paying_account_request)
		{
			// if no current modal window, or if not the quit window
			if(group != CWidgetManager::getInstance()->getModalWindow())
			{
				// disable
				CWidgetManager::getInstance()->disableModalWindow();
				CWidgetManager::getInstance()->enableModalWindow(NULL, group);

				bool farTPing = FarTP.isFarTPInProgress();
				// Far TP: skipping not allowed (because we can't duplicate the avatar...), anyway the quit button would quit the game (no far tp)
				CInterfaceElement *quitNowButton = group->getElement(group->getId() + ":indent_middle:ryzom");
				if (quitNowButton)
					quitNowButton->setActive(!farTPing);
				// From ring session or from mainland's ring access point: cancelling not allowed (would lead to inconsistencies with the SU & DSS)
				CInterfaceElement *backToGame = group->getElement(group->getId() + ":indent_middle:cancel");
				if (backToGame)
					backToGame->setActive(!(farTPing && (IsInRingSession || FarTP.isFastDisconnectGranted())));
				CInterfaceElement *quittingRules = group->getElement(group->getId() + ":text_mean");
				if (quittingRules)
					quittingRules->setActive(!farTPing);
				CInterfaceElement *quittingText = group->getElement(group->getId() + ":text");
				if (quittingText)
					quittingText->setActive(!farTPing);
			}
		}
		// else
		else
		{
			// if the current modal window is the quit window, disable
			if(group == CWidgetManager::getInstance()->getModalWindow())
			{
				// disable
				CWidgetManager::getInstance()->disableModalWindow();
			}
		}
	}
}


void setDefaultChatWindow(CChatWindow *defaultChatWindow)
{
	if (defaultChatWindow)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (defaultChatWindow->getContainer())
		{
			CInterfaceGroup *ig = defaultChatWindow->getContainer()->getGroup("eb");
			if (ig) CWidgetManager::getInstance()->setDefaultCaptureKeyboard(ig);
		}
	}
}

void updateDayNightCycleHour()
{
	if (ClientCfg.R2EDEnabled && R2::getEditor().getFixedLighting())
	{
		DayNightCycleHour = 12.f;
	}
	else
	{
		// if there's a forced time, apply it
		if( ForcedDayNightCycleHour < 0 )
			DayNightCycleHour	= (float)RT.getRyzomTime();
		else
			DayNightCycleHour	= ForcedDayNightCycleHour;
	}
}


// ***************************************************************************
//---------------------------------------------------
// mainLoop :
// Main loop of the application (displayer, input, ...).
// Return true to exit the game, false to return to character selection
//---------------------------------------------------
bool mainLoop()
{
	resetIngameTime ();

	NLMISC::TTime initStart = ryzomGetLocalTime();
	NLMISC::TTime initLast = initStart;
	NLMISC::TTime initCurrent = initLast;

	game_exit = false;
	game_exit_request = false;
	paying_account_request = false;
	paying_account_already_request = false;
	game_exit_after_paying_account_request = false;
	FarTP.setMainLoopEntered();

	nlinfo ("Starting main loop...", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

	// Get the Width and Height of the window and set the Old values..
	Driver->getWindowSize(Width, Height);
	OldWidth = Width; OldHeight = Height;

	CGraph::init( Driver );
	CGraph::Display = false;

	T1		= ryzomGetLocalTime (); //(sint64)CGDTime::getTime ();	// \todo GUIGUI : VOIR COMMENT MANAGER LE TEMPS
	TSend	= ((T1+DTSend)/DTSend)*DTSend;

	// initialize the structure for the ping.
	Ping.init();

	// initialize screenshots directory
	initScreenshot();

	// Call a function for a demo to init.

	if (ClientCfg.Local)
	{
		if (!ClientCfg.Light)
			initDemo();
	}

	// Get the Connection State.
	CNetworkConnection::TConnectionState lastConnectionState = CNetworkConnection::Connected;
	CNetworkConnection::TConnectionState connectionState = NetMngr.getConnectionState();

	updateLightDesc();

	SetMouseFreeLook ();
	SetMouseCursor ();
	// Set the cursor.
	ContextCur.context("STAND BY");

	// set the default box for keyboard
	setDefaultChatWindow(PeopleInterraction.ChatGroup.Window);


	// Init GameContextMenu.
	GameContextMenu.init("");

	// Active inputs
	Actions.enable(true);
	EditActions.enable(true);

	// For stoping the outgame music, start after 30 frames, and duration of 3 seconds
	CMusicFader		outgameFader(60, 3);


	// check for banned player
	if (testPermanentBanMarkers())
	{
		setPermanentBanMarkers(true); // re-set any marker that could have been removed by the trouble maker
		applyPermanentBanPunishment();
		PermanentlyBanned = true;
	}

	{
		CNiceInputAuto niceInputs;
		// R2 editor and modules
		R2::getEditor().autoConfigInit(IsInRingSession);

		if (ClientCfg.BeepWhenLaunched)
		{
			beep( 680, 400 );
			beep( 440, 400 );
			Driver->showWindow();
		}

		FPU_CHECKER_ONCE

		CurrSeason = computeCurrSeason();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		nlinfo ("PROFILE: %d seconds (%d total) for Starting main loop", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

		if (ClientCfg.R2EDEnabled && !ClientCfg.Local)
		{
			R2::getEditor().waitScenario();
		}

		CSessionBrowserImpl::getInstance().init(CLuaManager::getInstance().getLuaState());
	}

	CLuaManager::getInstance().executeLuaScript("game:onMainLoopBegin()");


	if (StartInitTime != 0)
	{
		CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_InGameEntry, "login_step_game_entry&load_time=" + toString((NLMISC::CTime::getLocalTime() - StartInitTime) / 1000)));
		StartInitTime = 0;
	}

	ProgressBar.finish();

	// Main loop. If the window is no more Active -> Exit.
	while( !UserEntity->permanentDeath()
		&& !game_exit )
	{
		// If an action handler execute. NB: MUST BE DONE BEFORE ANY THING ELSE PROFILE CRASH!!!!!!!!!!!!!!!!!
		testLaunchProfile();

		// Test and may run a VBLock profile (only once)
		testLaunchProfileVBLock();

		// Start Bench
		H_AUTO_USE ( RZ_Client_Main_Loop )

		if (isBGDownloadEnabled())
		{
			CBGDownloaderAccess &bgDownloader = CBGDownloaderAccess::getInstance();
			// if the ui is frozen, wait for a few frame to allow client to do first frame load,
			// without having to compete with the downloader frame
			if (!FirstFrame && bgDownloader.isDownloaderUIFrozen() && SkipFrame == 0)
			{
				unpauseBGDownloader();
			}
		}

		FPU_CHECKER_ONCE

		EGSPD::CSeason::TSeason newLocalSeason = computeCurrSeason();
		if (newLocalSeason != CurrSeason)
		{
			LoadingBackground= TeleportKaravanBackground;
			beginLoading (LoadingBackground);
			extern void selectTipsOfTheDay (uint tips);
			selectTipsOfTheDay (rand());
			UseEscapeDuringLoading = false;
			//
			#define BAR_STEP_TP 2
			ProgressBar.reset (BAR_STEP_TP);
			ucstring nmsg("Loading...");
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			ProgressBar.progress(0);
			ContinentMngr.select(UserEntity->pos(), ProgressBar);
			ProgressBar.finish();
			CurrSeason = newLocalSeason;
		}

		if (ClientCfg.R2EDEnabled)
		{
			if (R2::ResetWanted)
			{
				R2::getEditor().reset();
				R2::ResetWanted = false;
			}
			/*
			if (R2::ResetScenarioWanted)
			{
				R2::getEditor().resetScenario();
				R2::ResetScenarioWanted = false;
			}
			if (R2::ReloadScenarioWanted)
			{
				R2::getEditor().reloadScenario();
				R2::ReloadScenarioWanted = false;
			}
			*/
		}


		if (PermanentlyBanned)
		{
			if (UserEntity)
			{
				UserEntity->runVelocity(0);
				UserEntity->walkVelocity(0);
			}
			BanMsgCountdown -= DT;
			if (BanMsgCountdown < 0.f)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				ucstring msg = CI18N::get("msgPermanentlyBanned");
				string cat = getStringCategory(msg, msg);
				pIM->displaySystemInfo(msg, cat);
				BanMsgCountdown	 = BanMsgRepeatTime;
			}
		}

		{

			// Stop the Outgame music, with fade effect
			outgameFader.fade();

			// update quit feature
			updateGameQuitting();

			// update module manager
			NLNET::IModuleManager::getInstance().updateModules();

			if (ClientCfg.R2EDEnabled)
			{
				R2::getEditor().getDMC().flushActions();

				if (UserEntity) { UserEntity->updateNpcContolSpeed(); }

			}

			// update outpost stuff
			OutpostManager.update();


			// flush observers
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
		}


		EventsListener.setUIHandledButtonMask(NLMISC::noButton);

		// Fast mode.
		if(bZeroCpu)
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Zero_Cpu )

			// Grab Inputs.
			CInputHandlerManager::getInstance()->pumpEventsNoIM();


			// NetWork Update.
			NetMngr.update();
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
			// lets some CPU.
			NetMngr.send();
			nlSleep(100);
			// End of the frame.




			continue;
		}

		CSessionBrowserImpl::getInstance().update();

		//////////////////////////
		// INITIALIZE THE FRAME //
		//////////////////////////
		CInterfaceManager *pIMinstance;
		{

			if (ClientCfg.IsInvalidated)
				updateFromClientCfg();


			// Update the event listener
			EventsListener.update ();



			EventsListener.updateMouseSmoothing(); // IMPORTANT: this should be called before updateClientTime && events handling in the frame


			// Update Time.
			updateClientTime();


			// Grab Inputs.
			CInputHandlerManager::getInstance()->pumpEvents();

			CLandscapePolyDrawer::getInstance().deletePolygons();
			CDecalRenderList::getInstance().clearRenderList();


			// Update the Interface Manager Events.
			pIMinstance = CInterfaceManager::getInstance();


			// NB: must update frame events, even if ShowInterface==0, else may OutOfMemory (cause vector<> never cleared).
			pIMinstance->updateFrameEvents ();


			if ((ContinentMngr.cur()) && (UserEntity != NULL))
				ContinentMngr.cur()->FoW.explore((float)UserEntity->pos().x, (float)UserEntity->pos().y);

			// Check if the window size has changed.
			OldWidth = Width; OldHeight = Height;
			Driver->getWindowSize(Width, Height);


			// if yes, must update the camera perspective
			if(OldWidth!=Width || OldHeight!=Height)
				updateCameraPerspective();

			// Get Mouse Position.
			OldMouseX = MouseX; OldMouseY = MouseY;

			updateBGDownloaderUI();
		}

		// Get the pointer pos
		if(pIMinstance)
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Cursor )

			// Change only if screen is not minimized
			if(!CViewRenderer::getInstance()->isMinimized())
			{
				// Get the cursor instance
				CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
				if(cursor)
				{
					// Get the pointer position (in pixel)
					sint32 x, y;
					cursor->getPointerPos(x, y);

					uint32 w, h;
					CViewRenderer &viewRender = *CViewRenderer::getInstance();
					viewRender.getScreenSize(w, h);

					if(w)
						MouseX = (float)x/(float)w;
					else
						MouseX = 0;
					if(h)
						MouseY = (float)y/(float)h;
					else
						MouseY = 0;
				}
			}
		}

		///////////////////////
		// PROCESS THE FRAME //
		///////////////////////
		// NetWork Update.

		{

			NetMngr.update();
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();
			bool prevDatabaseInitStatus = IngameDbMngr.initInProgress();
			IngameDbMngr.setChangesProcessed();
			bool newDatabaseInitStatus = IngameDbMngr.initInProgress();
			if ((!newDatabaseInitStatus) && prevDatabaseInitStatus)
			{
				// When database received, activate allegiance buttons (for neutral state) in fame window
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CInterfaceGroup	*group = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:fame:content:you"));
				if (group)
					group->updateAllLinks();
				// send a msg to lua for specific ui update
				CLuaManager::getInstance().executeLuaScript("game:onInGameDbInitialized()");
			}
		}


		// For Debug (after netmngr update). Log entity stage change.
		EntitiesMngr.logStageChange(T1);

		// Update Ryzom Time.
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Time_Update )
			if (!ClientCfg.Local)
			{
				if(NetMngr.getCurrentServerTick() > LastGameCycle)
					RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
			}
			else if (ClientCfg.SimulateServerTick)
			{
				SimulatedServerDate += DT;
				uint numTicks = (uint) floor(SimulatedServerDate * 10);
				SimulatedServerTick += numTicks;
				SimulatedServerDate = (float)((double)SimulatedServerDate - (double) numTicks * 0.1);
				RT.updateRyzomClock((uint32)SimulatedServerTick, ryzomGetLocalTime() * 0.001);
			}


			updateDayNightCycleHour();

		}

		updateSmoothedTime();

		if (!ClientCfg.Light)
		{
			// Call a function for a demo to update.
			if(ClientCfg.Local)
			{
				updateDemo( (double)(T1-T0)*0.001 );
			}
		}

		// R2ED pre render update
		if (ClientCfg.R2EDEnabled)
		{
			R2::getEditor().updatePreCamera();
		}

		/*
		 *	Update position of all the primitives. Make a PACS move.
		 */
		EntitiesMngr.updatePreCamera();

		// Snap the user entity on the ground
		UserEntity->snapToGround();


		// update bot chat
		CBotChatManager::getInstance()->update();
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// updateItemEdition
		CInterfaceItemEdition::getInstance()->update();

		/*
		 *	Update user controls and compute the camera position
		 */
		UserControls.update();

		// Update Landscape RefineCenter
		if (Landscape) Landscape->setRefineCenterUser(View.refinePos());

		// Update camera recorder, possibly replacing current view
		updateCameraRecorder();

		// Update Camera Position/Orientation.
		CVector currViewPos = View.currentViewPos();
		MainCam.setTransformMode(UTransformable::RotQuat);
		MainCam.setPos(currViewPos);
		MainCam.setRotQuat(View.currentViewQuat());
		if (StereoHMD)
		{
			CMatrix camMatrix;
			camMatrix.translate(MainCam.getMatrix().getPos());
			CVector dir = MainCam.getMatrix().getJ();
			dir.z = 0;
			dir.normalize();
			if (dir.y < 0)
				camMatrix.rotateZ(float(NLMISC::Pi+asin(dir.x)));
			else
				camMatrix.rotateZ(float(NLMISC::Pi+NLMISC::Pi-asin(dir.x)));

			StereoHMD->setInterfaceMatrix(camMatrix);

			NLMISC::CQuat hmdOrient = StereoHMD->getOrientation();
			// NLMISC::CMatrix camMatrix = MainCam.getMatrix();
			NLMISC::CMatrix hmdMatrix;
			hmdMatrix.setRot(hmdOrient);
			NLMISC::CMatrix posMatrix; // minimal head modeling, will be changed in the future
			posMatrix.translate(StereoHMD->getEyePosition());
			NLMISC::CMatrix mat = ((camMatrix * hmdMatrix) * posMatrix);
			MainCam.setPos(mat.getPos());
			MainCam.setRotQuat(mat.getRot());

			if (true) // TODO: ClientCfg.Headphone
			{
				// NOTE: non-(StereoHMD+Headphone) impl in user_entity.cpp
				SoundMngr->setListenerPos(mat.getPos()); // TODO: Move ears back ... :)
				SoundMngr->setListenerOrientation(mat.getJ(), mat.getK());
			}
		}
		if (StereoDisplay)
		{
			StereoDisplay->updateCamera(0, &MainCam);
			if (SceneRoot)
			{
				UCamera cam = SceneRoot->getCam();
				StereoDisplay->updateCamera(1, &cam);
			}
		}

		// see if camera is below water (useful for sort order)
		if (ContinentMngr.cur())
		{
			float waterHeight;
			bool splashEnabled;
			if (ContinentMngr.cur()->WaterMap.getWaterHeight(CVector2f(currViewPos.x, currViewPos.y), waterHeight, splashEnabled))
			{
				// camera is above / below a water surface
				Scene->setLayersRenderingOrder(currViewPos.z > waterHeight);
				UserEntity->setOrderingLayer(currViewPos.z > waterHeight ? 0 : 2);
			}
			else
			{
				UserEntity->setOrderingLayer(2);
				Scene->setLayersRenderingOrder(true);
			}
		}

		// Build the camera clipping planes
		vector<CPlane> planes;
		buildCameraClippingPyramid (planes);

		/*
		 *	Clip the entities.
		 *	Update display for visible entities.
		 *  Update display for clipped entities at a lower frequency.
		 */
		static uint clippedUpdateTime = 0;
		EntitiesMngr.updatePostCamera(clippedUpdateTime, planes, MainCam.getPos());
		clippedUpdateTime++;
		clippedUpdateTime&=RZ_CLIPPED_UPDATE_TIME_MASK;

		// Update the position for the vision.
		NetMngr.setReferencePosition(UserEntity->pos());

		// For Debug (after entities update). Log entity stage change.
		EntitiesMngr.logStageChange(T1);

		if (!ClientCfg.Light)
		{
			// Animate all the playlists
			EAM->setup (TimeInSec);

		}

		// update the sound of the player (walk, run....) if sound is allocated.
		updateSound();

		if (Landscape)
		{
			if (!ClientCfg.Light)
			{
				// Not in an indoor ?
				if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
				{
					// Load Zone in streaming according to the refine position (not necessarily the User Position);
					string	zoneAdded, zoneRemoved;
					const R2::CScenarioEntryPoints::CCompleteIsland *ci = R2::CScenarioEntryPoints::getInstance().getCompleteIslandFromCoords(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y));
					Landscape->refreshZonesAround(View.refinePos(), ClientCfg.Vision + ExtraZoneLoadingVision, zoneAdded, zoneRemoved, ci ? &(ci->ZoneIDs) : NULL);
					LandscapeIGManager.loadZoneIG(zoneAdded);
					LandscapeIGManager.unloadZoneIG(zoneRemoved);
				}
			}
		}

		// Update PACS
		if(GR)
			GR->refreshLrAround (View.refinePos(), LRRefeshRadius);

		// load / unload streamable obj (villages ...)
		if (ClientCfg.VillagesEnabled)
		{
			ContinentMngr.updateStreamable(View.refinePos());
		}

		if (!ClientCfg.Light)
		{
			// Load / unload streaming Instances textures.
			Driver->updateAsyncTexture();

			// Manage Fx
			manageFxEntities();

			// Animate all systems in scene.
			Scene->animate(TimeInSec-FirstTimeInSec);

		}


		if (!ClientCfg.Light)
		{
			CClientDate newDate(RT.getRyzomDay(), DayNightCycleHour);
			if (newDate < CTimedFXManager::getInstance().getDate() ||
				abs((sint32)RT.getRyzomDay() - CTimedFXManager::getInstance().getDate().Day) > 1)
			{
				// The manager make the assumption that no more than one day can occurs between 2 ticks
				// This only happens when date is changed manually
				if (IGCallbacks)
				{
					IGCallbacks->changeSeason(); // the season doesn't change, but this force fxs to be recreated
				}
			}
			CTimedFXManager::getInstance().update(newDate, CurrSeason, Scene->getCam().getPos());

			CProjectileManager::getInstance().update();

			// temp temp : for debug
			//TestGroundFX.update();
		}


		// Set the right camera cluster.
		if(GR)
		{
			UInstanceGroup *pPlayerClusterSystem = NULL;

			// Normal Mode
			if(UserControls.mode() != CUserControls::ThirdMode)
			{
				// get the Pacs global position of the camera
				UGlobalPosition gPos;
				if((UserControls.mode() != CUserControls::AIMode)
					&& UserEntity->getPrimitive())
					UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
				else
					gPos = GR->retrievePosition(View.viewPos());

				// get the cluster IG associated to this pacs position
				pPlayerClusterSystem = getCluster(gPos);
				MainCam.setClusterSystem(pPlayerClusterSystem);

				// important to update this each frame, for shadow map consideration against the "matis serre bug"
				CollisionManager->setPlayerInside(pPlayerClusterSystem!=NULL);
			}
			// Camera 3rd person complex mode
			else
			{
				UGlobalPosition gPos;
				if(UserEntity->getPrimitive())
					UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
				// get the cluster IG associated to this pacs position
				pPlayerClusterSystem = getCluster(gPos);

				// set the one found in CView::updateCameraCollision()
				MainCam.setClusterSystem(View.getThirdPersonClusterSystem());

				// important to update this each frame, for shadow map consideration against the "matis serre bug"
				CollisionManager->setPlayerInside(pPlayerClusterSystem!=NULL);

				// For debug only
				View.getCamera3rdPersonSetup(LastDebugClusterCameraThirdPersonStart,
					LastDebugClusterCameraThirdPersonEnd,
					LastDebugClusterCameraThirdPersonTestStart);
				LastDebugClusterCameraThirdPersonResult= View.currentViewPos();
				LastDebugClusterCameraThirdPersonPelvisPos= View.viewPos() + CVector(0.f,0.f,1.f);
				LastDebugClusterCameraThirdPersonForceFPV= View.forceFirstPersonView();
				// TestYoyo
				//CameraThirPersonGraph.addOneValue ((startPos - endPos).norm());
			}

			// If we are flushing open all doors
			if (SkipFrame > 0)
			{
				// update only the cluster system where the player is!
				if (pPlayerClusterSystem != NULL)
				{
					static vector<string> PortalsName;
					PortalsName.clear();
					pPlayerClusterSystem->getDynamicPortals(PortalsName);
					for (uint32 i = 0; i < PortalsName.size(); ++i)
						pPlayerClusterSystem->setDynamicPortal (PortalsName[i], true);
				}
			}
		}


		// R2ED pre render update
		if (ClientCfg.R2EDEnabled)
		{
			R2::getEditor().updateBeforeRender();
		}

		if (!ClientCfg.Light)
		{
			// Render
			if(Render)
			{
				// Update water env map (happens when continent changed etc)
				updateWaterEnvMap();

				// Update weather
				updateWeather();
			}
		}

		uint i = 0;
		bool effectRender = false;
		CTextureUser *effectRenderTarget = NULL;
		bool haveEffects = Render && Driver->getPolygonMode() == UDriver::Filled
			&& (ClientCfg.Bloom || FXAA);
		bool defaultRenderTarget = false;
		if (haveEffects)
		{
			if (!StereoDisplay)
			{
				Driver->beginDefaultRenderTarget();
				defaultRenderTarget = true;
			}
			if (ClientCfg.Bloom)
			{
				CBloomEffect::getInstance().setSquareBloom(ClientCfg.SquareBloom);
				CBloomEffect::getInstance().setDensityBloom((uint8)ClientCfg.DensityBloom);
			}
		}
		while ((!StereoDisplay && i == 0) || (StereoDisplay && StereoDisplay->nextPass()))
		{
			++i;
			///////////////////
			// SETUP CAMERAS //
			///////////////////

			if (StereoDisplay)
			{
				// modify cameras for stereo display
				const CViewport &vp = StereoDisplay->getCurrentViewport();
				Driver->setViewport(vp);
				nlassert(Scene);
				Scene->setViewport(vp);
				if (SceneRoot)
				{
					SceneRoot->setViewport(vp);
				}
				//MainCam.setTransformMode(UTransformable::DirectMatrix);
				StereoDisplay->getCurrentMatrix(0, &MainCam);
				StereoDisplay->getCurrentFrustum(0, &MainCam);
				if (SceneRoot)
				{
					// matrix updated during commitCamera from maincam
					UCamera cam = SceneRoot->getCam();
					StereoDisplay->getCurrentFrustum(1, &cam);
				}
			}

			// Commit camera changes
			commitCamera();

			//////////////////////////
			// RENDER THE FRAME  3D //
			//////////////////////////

			bool stereoRenderTarget = (StereoDisplay != NULL) && StereoDisplay->beginRenderTarget();

			if (!StereoDisplay || StereoDisplay->wantClear())
			{
				if (Render)
				{
					effectRender = haveEffects;
				}

				// Clear buffers
				clearBuffers();
			}

			if (!StereoDisplay || StereoDisplay->wantScene())
			{
				if (!ClientCfg.Light)
				{
					// Render
					if(Render)
					{
						// nb : force full detail if a screenshot is asked
						// todo : move outside render code
						bool fullDetail = ScreenshotRequest != ScreenshotRequestNone && ClientCfg.ScreenShotFullDetail;
						if (fullDetail)
						{
							s_ForceFullDetail.backup();
							s_ForceFullDetail.set();
						}

						// Render scene
						renderScene();

						if (fullDetail)
						{
							s_ForceFullDetail.restore();
						}
					}
				}
			}

			if (!StereoDisplay || StereoDisplay->wantInterface3D())
			{
				if (!ClientCfg.Light)
				{
					// Render
					if (Render)
					{
						if (effectRender)
						{
							if (StereoDisplay) Driver->setViewport(NL3D::CViewport());
							UCamera	pCam = Scene->getCam();
							Driver->setMatrixMode2D11();
							if (FXAA) FXAA->applyEffect();
							if (ClientCfg.Bloom) CBloomEffect::instance().applyBloom();
							Driver->setMatrixMode3D(pCam);
							if (StereoDisplay) Driver->setViewport(StereoDisplay->getCurrentViewport());
							effectRender = false;
						}

						// for that frame and
						// tmp : display height grid
						//static volatile bool displayHeightGrid = true;
						/*if (displayHeightGrid)
						{
							HeightGrid.display(*Driver);
						}*/
						// display results?
						if(Scene_Profile)
						{
							displaySceneProfiles();
							Scene_Profile= false;
						}
						// Render the primitives
						{
							H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
							PrimFiles.display (*Driver);
						}
					} /* if (Render) */

					// Draw Extra 3D Objects
					Driver->setMatrixMode3D(MainCam);
					Driver->setModelMatrix(CMatrix::Identity);

					// Display PACS borders.
					if (PACSBorders)
					{
						H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
						displayPACSBorders();
						displayPACSPrimitive();
					}

					// display Sound box
					if (SoundBox)
					{
						H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
						displaySoundBox();
					}

					// display Debug of Clusters
					if (DebugClusters)
					{
						H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
						displayDebugClusters();
					}
				} /* if (!ClientCfg.Light) */
				else
				{
		//			static UTextureFile *backgroundBitmap = NULL;
		//			if (backgroundBitmap == NULL)
		//				backgroundBitmap = Driver->createTextureFile("temp_background.tga");
		//			Driver->setMatrixMode2D11();
		//			Driver->drawBitmap (0.f, 0.f, 1024.f/1024.f, 1024.f/768.f, (UTexture&)*backgroundBitmap);
		//			Driver->setMatrixMode3D(MainCam);

					Driver->clearBuffers(CRGBA (0,0,0,0));
					displayPACSBorders();
					displayPACSPrimitive();
				}

				if (!ClientCfg.Light && !Landscape)
				{
					displayPACSBorders();
				}

				// Display some things not in the scene like the name, the entity path, etc.
				EntitiesMngr.updatePostRender();

				// Render the stat graphs if needed
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
					CGraph::render (ShowInfos);
				}

			} /* if (!StereoDisplay || StereoDisplay->wantInterface3D()) */

			if (!StereoDisplay || StereoDisplay->wantInterface2D())
			{
				// Render in 2D Mode to display 2D Interfaces and 2D texts.
				Driver->setMatrixMode2D11();

				// draw a big quad to represent thunder strokes
				/*if (Render && WeatherManager.getThunderLevel() != 0.f)
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Render_Thunder )
					Driver->drawQuad(0, 0, 1, 1, ThunderColor);

					// TODO : boris : add sound here !
					// Needs more explosions
				}*/

				// Update the contextual menu
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Interface )

					// Update the game cursor.
					ContextCur.check();
					GameContextMenu.update();

					// validate dialogs
					validateDialogs(GameContextMenu);

					// Display interface v3
					Driver->enableFog (false);
					if (!Driver->isLost())
					{
						if(ShowInterface)
							pIMinstance->updateFrameViews (MainCam);
						if(DebugUIView)
							pIMinstance->displayUIViewBBoxs(DebugUIFilter);
						if(DebugUICtrl)
							pIMinstance->displayUICtrlBBoxs(DebugUIFilter);
						if(DebugUIGroup)
							pIMinstance->displayUIGroupBBoxs(DebugUIFilter);
					}

					// special case in OpenGL : all scene has been display in render target,
					// now, final texture is display with a quad
					/*if (!ClientCfg.Light && ClientCfg.Bloom && Render && bloomStage == 2) // NO VR BLOOMZ
					{
						// End bloom effect system after drawing the 3d interface (z buffer related).
						if (StereoDisplay) Driver->setViewport(NL3D::CViewport());
						CBloomEffect::instance().endInterfacesDisplayBloom();
						if (StereoDisplay) Driver->setViewport(StereoDisplay->getCurrentViewport());
						bloomStage = 0;
					}*/
				}

				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
					if (!Driver->isLost())
					{
						// If show information is Active.
						if(ShowInfos == 1)
							displayDebug();

						// If show information is Active.
						if(ShowInfos == 2)
							displayNetDebug();

						// If show information is Active.
						if(ShowInfos == 4)
							displayDebugFps();

						// If show information is Active.
						if(ShowInfos == 5)
							displayDebugUIUnderMouse();

						// If show information is Active.
						displayStreamingDebug();

						// If Show Help is active -> Display an help.
						if(ShowHelp)
							displayHelp();

						// Yoyo: indicate profiling state
						if( Profiling )
							displaySpecialTextProgress("Profiling");

						// Display frame rate

						// Create a shadow when displaying a text.
						TextContext->setShaded(true);
						// Set the font size.
						TextContext->setFontSize(10);
						// Set the text color
						TextContext->setColor(CRGBA(255,255,255));

						// temporary values for conversions
						float x, y, width, height;

						for(uint i = 0; i < ClientCfg.Logos.size(); i++)
						{
							std::vector<string> res;
							explode(ClientCfg.Logos[i],std::string(":"), res);
							if(res.size()==9 && i<LogoBitmaps.size() && LogoBitmaps[i]!=NULL)
							{
								fromString(res[5], x);
								fromString(res[6], y);
								fromString(res[7], width);
								fromString(res[8], height);
								Driver->drawBitmap(x/(float)ClientCfg.Width, y/(float)ClientCfg.Height, width/(float)ClientCfg.Width, height/(float)ClientCfg.Height, *LogoBitmaps[i]);
							}
						}
					}
				}

				// FPS
				{
					static TTicks oldTick = CTime::getPerformanceTime();
					TTicks newTick = CTime::getPerformanceTime();
					double deltaTime = CTime::ticksToSecond (newTick-oldTick);
					oldTick = newTick;
					smoothFPS.addValue((float)deltaTime);
					moreSmoothFPS.addValue((float)deltaTime);
					deltaTime = smoothFPS.getSmoothValue ();
					if (deltaTime > 0.0)
					{
						CCDBNodeLeaf *pNL = s_FpsLeaf ? &*s_FpsLeaf
							: &*(s_FpsLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:FPS"));
						pNL->setValue64((sint64)(1.f/deltaTime));
					}
				}

				// R2ED post render update
				if (ClientCfg.R2EDEnabled)
				{
					// IMPORTANT : this should be called after CEntitiesMngr::updatePostRender() because
					// entity may be added / removed there !
					R2::getEditor().updateAfterRender();
				}

				// Update FXs (remove them).
				FXMngr.update();

				// Detect disconnection / server down: display information text
				// but keep the rendering so that the player can remember where he is
				// and what he was doing. He can't move because the connection quality returns false.

				if ((connectionState == CNetworkConnection::Disconnect) && (lastConnectionState != CNetworkConnection::Disconnect) && (!FarTP.isFarTPInProgress()))
				{
					UserControls.stopFreeLook(); // let the player click on Exit
					pIMinstance->messageBoxWithHelp(CI18N::get("uiDisconnected"));

					// If we have started a Far TP sequence and are waiting for onServerQuitOK()
					// from the EGS, resume the sequence because the EGS is down and won't reply.
					FarTP.onServerQuitOk();
				}

				// Yoyo: MovieShooter.
				if(MovieShooterSaving)
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Debug )

					// Add the buffer frame to the movie.
					if(!MovieShooter.addFrame(TimeInSec, Driver))
					{
						// Fail to add the frame => abort.
						endMovieShooting();
					}
					else
					{
						// Ok, just add a display.
						displaySpecialTextProgress("MovieShooting");
					}
				}

				if (isRecordingCamera())
				{
					displaySpecialTextProgress("CameraRecording");
				}

				// Temp for weather test
				if (ClientCfg.ManualWeatherSetup && ContinentMngr.cur() && ContinentMngr.cur()->WeatherFunction)
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
					static float displayHourDelta = 0.04f; // static for edition during debug..

					// Display weather function
					if (DisplayWeatherFunction)
					{
						uint64 currDay = RT.getRyzomDay();
						float currHour = DayNightCycleHour;
						float singleHourDelta = fmodf(currHour, 1.f);
						uint32 wndWidth, wndHeight;
						Driver->getWindowSize(wndWidth, wndHeight);
						Driver->setMatrixMode2D(CFrustum(0, 800, 600, 0, 0, 1, false));
						const float lineHeight = 100.f;

						// draw the weather function
						for(uint x = 0; x < wndWidth; ++x)
						{
							float weatherValue;
							if(ContinentMngr.cur())
								weatherValue = ::getBlendedWeather(currDay, currHour, *WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction);
							else
								weatherValue = ::getBlendedWeather(currDay, currHour, *WeatherFunctionParams, 0);

							NLMISC::clamp(weatherValue, 0.f, 1.f);
							CRGBA seasonToColor[EGSPD::CSeason::Invalid] =
							{
								CRGBA::Green,
								CRGBA::Yellow,
								CRGBA::Red,
								CRGBA::Blue
							};
							Driver->drawLine((float) x, 0.f, (float) x, lineHeight * weatherValue, seasonToColor[CRyzomTime::getSeasonByDay((uint32)currDay)]);
							currHour += displayHourDelta;
							if (currHour >= 24.f)
							{
								++currDay;
								currHour -= 24.f;
							}
							singleHourDelta += displayHourDelta;
							if (singleHourDelta >= 1.f)
							{
								singleHourDelta -= 1.f;
								Driver->drawLine((float) x, 100.f, (float) x, 130, CRGBA::Red);
							}
						}

						if(ContinentMngr.cur())
						{
							// draw lines for current weather setups
							uint numWeatherSetups = ContinentMngr.cur()->WeatherFunction[CurrSeason].getNumWeatherSetups();
							for (uint y = 0; y < numWeatherSetups; ++y)
							{
								float py = lineHeight * (y / (float) numWeatherSetups);
								Driver->drawLine(0.f, py, 800.f, py, CRGBA::Magenta);
							}
						}
					}

					// Ctrl+ & Ctrl- change the weather value
					if (Actions.valide ("inc_time"))
					{
						ManualWeatherValue += DT * 0.04f;
					}
					if (Actions.valide ("dec_time"))
					{
						ManualWeatherValue -= DT * 0.04f;
					}
					NLMISC::clamp(ManualWeatherValue, 0.f, 1.f);

					if (ForcedDayNightCycleHour < 0) // if time is forced then can't change it manually ...
					{
						// Ctrl-K increase hour
						if (Actions.valide ("inc_hour"))
						{
							RT.increaseTickOffset( (uint32)(2000 * displayHourDelta) );
							RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
						}

						// Ctrl-L decrease hour
						if (Actions.valide ("dec_hour"))
						{
							RT.decreaseTickOffset( (uint32)(2000 * displayHourDelta) );
							RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
							CTimedFXManager::getInstance().setDate(CClientDate(RT.getRyzomDay(), (float) RT.getRyzomTime()));
							if (IGCallbacks)
							{
								IGCallbacks->changeSeason(); // the season doesn't change, but this force fxs to be recreated
							}
						}
					}

					// Ctrl-M generate statistics in a file
						/*
					if (Actions.valide ("weather_stats"))
					{
						// Only usable if there is a continent loaded.
						if(ContinentMngr.cur())
							CPredictWeather::generateWeatherStats("weather_stats.csv", WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction);
					}*/

					// Ctrl-B decrease display factor
					if (Actions.valide ("dec_display_factor"))
					{
						displayHourDelta *= 0.90f;
					}
					// Ctrl-J increase display factor
					if (Actions.valide ("inc_display_factor"))
					{
						displayHourDelta *= 1.1f;
						displayHourDelta = std::min(1000.f, displayHourDelta);
					}
				}

				// Ctrl-AltGR-Z show timed FXs
				if (ShowTimedFX)
				{
					if (!Driver->isLost())
					{
						CTimedFXManager::getInstance().displayFXBoxes(ShowTimedFXMode);
					}
				}

		#if !FINAL_VERSION
					CVector2f camPos(Scene->getCam().getPos().x, Scene->getCam().getPos().y);
					if (!ClientCfg.Light)
					{
						if (DisplayMicroLifeZones)
						{
							CMicroLifeManager::getInstance().renderMLZones(camPos);
						}
					}
					if (DisplayWaterMap)
					{
						if (ContinentMngr.cur())
						{
							ContinentMngr.cur()->WaterMap.render(camPos);
						}
					}
				#endif

				#ifdef NL_DEBUG
					if (!ClientCfg.Light)
					{
						if (DisplayMicroLifeActiveTiles)
						{
							CMicroLifeManager::getInstance().renderActiveTiles();
						}
					}
				#endif
				// tmp : debug of ground fxs
				//TestGroundFX.displayFXBoxes();

				// Temp for sound debug
				{
					H_AUTO_USE ( RZ_Client_Main_Loop_Debug )

					if (SoundMngr != 0)
					{
						static bool drawSound = false;
		 				static float camHeigh = 150.0f;

		#if FINAL_VERSION
						if (ClientCfg.ExtendedCommands)
		#endif
							if (Actions.valide ("draw_sound"))
								drawSound = !drawSound;

						if (Actions.valide ("inc_camera_height"))
							camHeigh -= 10.0f;
						if (Actions.valide ("dec_camera_height"))
							camHeigh += 10.0f;

						if (drawSound)
							SoundMngr->drawSounds(camHeigh);
					}
				}
			} /* if (!StereoDisplay || StereoDisplay->wantInterface2D()) */

			if (StereoDisplay)
			{
				StereoDisplay->endRenderTarget();
			}
		} /* stereo pass */

		if (defaultRenderTarget)
		{
			// draw final result to backbuffer
			Driver->endDefaultRenderTarget(Scene);
		}

		// Draw to screen.
		static CQuat MainCamOri;
		if (FirstFrame)
		{
			// Frame to skip before swap buffer
			SkipFrame = RYZOM_FIRST_FRAME_TO_SKIP;
			FirstFrame = false;
			MainCam.getRotQuat(MainCamOri);
		}

		if (SkipFrame == 0)
		{
			if (StartPlayTime == 0)
			{
				StartPlayTime = NLMISC::CTime::getLocalTime();
			}
			// Start background sound play now !  (nb: restarted if load just ended, or if sound re-enabled)
			if (SoundMngr)
			{
				H_AUTO_USE ( RZ_Client_Main_Loop_Sound )
				SoundMngr->playBackgroundSound();
			}

			// Fade in Game Sound now (before endLoading)
			if(SoundMngr)
			{
				// fade out loading music
				if(LoadingMusic==SoundMngr->getEventMusicPlayed())
					SoundMngr->stopEventMusic(LoadingMusic, CSoundManager::LoadingMusicXFade);
				// fade in game sound
				SoundMngr->fadeInGameSound(ClientCfg.SoundTPFade);
			}

			// end loading (if previous load)
			endLoading ();

			// if a screenshot request was made then do it now
			switch(ScreenshotRequest)
			{
				case ScreenshotRequestTGA:
					screenShotTGA();
					ScreenshotRequest = ScreenshotRequestNone;
				break;
				case ScreenshotRequestJPG:
					screenShotJPG();
					ScreenshotRequest = ScreenshotRequestNone;
				break;
				case ScreenshotRequestPNG:
					screenShotPNG();
					ScreenshotRequest = ScreenshotRequestNone;
				break;
				default:
				break;
			}

			// TMP TMP
			static volatile bool dumpValidPolys = false;
			if (dumpValidPolys) { tempDumpValidPolys(); dumpValidPolys = false; }

			// TMP TMP
			static volatile bool dumpColPolys = false;
			if (dumpColPolys) { tempDumpColPolys(); }

			if (ClientCfg.R2EDEnabled)
			{
				R2::getEditor().updateBeforeSwapBuffer();
			}

			Driver->swapBuffers();

			if(Profiling)
			{
				++ ProfileNumFrame;
				if (ProfileNumFrame == ClientCfg.NumFrameForProfile)
				{
					WantProfiling = true;
				}
			}

			// If the device is lost then no rendering will occur, so let some time to other applications
			if (Driver->isLost())
			{
				nlSleep(50);
				nldebug("lost device");
			}
		}
		else
		{
			SkipFrame--;

			// Turn the camera to make a 360 degree turn
//			UTransformable::TTransformMode m = MainCam.getTransformMode();
			if (SkipFrame == 0)
			{
				MainCam.setRotQuat(MainCamOri);
			}
			else
			{
				CMatrix mat = CMatrix::Identity;
				mat.setRot(MainCamOri);
				mat.rotateZ(2*(float)Pi*((float)(SkipFrame)/(float)RYZOM_FIRST_FRAME_TO_SKIP));
				CQuat qTmp;
				mat.getRot(qTmp);
				MainCam.setRotQuat(qTmp);
			}
		}


		// Force the client to sleep a bit.
		if(ClientCfg.Sleep >= 0)
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
			nlSleep(ClientCfg.Sleep);
		}

		//++MainLoopCounter;

		// Send new data Only when server tick changed.
		if(NetMngr.getCurrentServerTick() > LastGameCycle)
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Net )
			// Put here things you have to send to the server only once per tick like user position.
			// UPDATE COMPASS
			NLMISC::CCDBNodeLeaf *node = s_UiDirectionLeaf ? (&*s_UiDirectionLeaf)
				: &*(s_UiDirectionLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DIRECTION"));
			CInterfaceProperty prop;
			prop.setNodePtr(node);
			if(CompassMode == 1)
			{
				double camDir = atan2(View.view().y, View.view().x);
				prop.setDouble(camDir);
			}
			else
				prop.setDouble(atan2(UserEntity->front().y, UserEntity->front().x));
			// Update the server with our position and orientation.
			{
				CBitMemStream out;
				if(UserEntity->sendToServer(out))
					NetMngr.push(out);
			}
			// Give information to the server about the combat position (ability to strike).
			{
				CBitMemStream out;
				if(UserEntity->msgForCombatPos(out))
					NetMngr.push(out);
			}

			// Create the message for the server to move the user (except in combat mode).
			if(Ping.rdyToPing())
			{
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream("DEBUG:PING", out))
				{
					const TTime mask = 0xFFFFFFFF;
					uint32 localTime = (uint32)(mask&ryzomGetLocalTime ());
					out.serial(localTime);
					NetMngr.push(out);

					Ping.rdyToPing(false);
				}
				else
					nlwarning("mainloop: unknown message named 'DEBUG:PING'.");
			}

			// Send the Packet.
			NetMngr.send(NetMngr.getCurrentServerTick());
			// Update the Last tick received from the server.
			LastGameCycle = NetMngr.getCurrentServerTick();
		}

		if (ClientCfg.AutoReloadFiles)
		{
			// Check for files update.
			CFile::checkFileChange();

			// Check for configuration files update.
			CConfigFile::checkConfigFiles();
		}

		// Get the Connection State.
		lastConnectionState = connectionState;
		connectionState = NetMngr.getConnectionState();

		// Update movie shooter
		updateMovieShooting();

		// Update the bubble manager
		InSceneBubbleManager.update();

		// Update the NPC icon system
		CNPCIconCache::getInstance().update();

		// Update Phrase Manager
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();
		pPM->updateEquipInvalidation(NetMngr.getCurrentServerTick());
		pPM->updateAllActionRegen();

		// Update ingame duration and stat report sending
		updateStatReport ();

		// Update the music player
		MusicPlayer.update ();

		// Memory Debug
		if (ClientCfg.CheckMemoryEveryNFrame != -1)
		{
			static int frameToSkip = ClientCfg.CheckMemoryEveryNFrame;
			if (frameToSkip == 0)
			{
				frameToSkip = ClientCfg.CheckMemoryEveryNFrame;
				//NLMEMORY::CheckHeap (true);
			}
			else
				frameToSkip--;
		}

		///////////////
		// FAR_TP -> //
		///////////////
		// Enter a network loop during the FarTP process, without doing the whole real main loop.
		// This code must remain at the very end of the main loop.
		if(LoginSM.getCurrentState() == CLoginStateMachine::st_enter_far_tp_main_loop)
		{
			CLuaManager::getInstance().executeLuaScript("game:onFarTpStart()");
			// Will loop the network until the end of the relogging process
			FarTP.farTPmainLoop();

			if( FarTP.isReselectingChar() )
			{
				if ( game_exit ) // check if the user has decided to quit
					break;

				// we have just completed init main loop, after reselecting character
				//	repeat the steps before the main loop itself

				// pre main loop in mainLoop
				resetIngameTime ();

				game_exit = false;
				game_exit_request = false;
				FarTP.setMainLoopEntered();

				// Get the Width and Height of the window and set the Old values..
				Driver->getWindowSize(Width, Height);
				OldWidth = Width; OldHeight = Height;

				CGraph::init( Driver );
				CGraph::Display = false;

				T1		= ryzomGetLocalTime();
				TSend	= ((T1+DTSend)/DTSend)*DTSend;

				SetMouseFreeLook ();
				SetMouseCursor ();
				// Set the cursor.
				ContextCur.context("STAND BY");

				// set the default box for keyboard
				CChatWindow *defaultChatWindow;
				if (ClientCfg.R2EDEnabled)
				{
					defaultChatWindow = PeopleInterraction.DebugInfo;
				}
				else
				{
					defaultChatWindow = PeopleInterraction.ChatGroup.Window;
				}
				setDefaultChatWindow(defaultChatWindow);

				// Init GameContextMenu.
				GameContextMenu.init("");

				// Active inputs
				Actions.enable(true);
				EditActions.enable(true);

				// For stoping the outgame music, start after 30 frames, and duration of 3 seconds
//				CMusicFader	outgameFader(60, 3);

				// check for banned player
				if (testPermanentBanMarkers())
				{
					setPermanentBanMarkers(true); // re-set any marker that could have been removed by the trouble maker
					applyPermanentBanPunishment();
					PermanentlyBanned = true;
				}
			}

			// Short reinit of the main loop after farTP or character reselection
			Ping.init();
			updateLightDesc();

			// R2ED enabled ?
			R2::getEditor().autoConfigInit(IsInRingSession);

			CurrSeason = computeCurrSeason();

			// Get the Connection State (must be done after any Far TP to prevent the uiDisconnected box to be displayed)
			lastConnectionState = CNetworkConnection::Connected;
			connectionState = NetMngr.getConnectionState();

			CLuaManager::getInstance().executeLuaScript("game:onFarTpEnd()");
		}
		///////////////
		// <- FAR_TP //
		///////////////

	} // end of main loop

	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (CLuaManager::getInstance().getLuaState())
	{
		CLuaManager::getInstance().executeLuaScript("game:onMainLoopEnd()");
	}

	// Stop Running Profiles (kick result)
	if(Profiling)
	{
		WantProfiling= false;
		Profiling= false;
		CHTimer::endBench();
		Driver->endBench();
	}
	if(ProfilingVBLock)
	{
		WantProfilingVBLock= false;
		ProfilingVBLock= false;
		vector<string>	strs;
		Driver->endProfileVBHardLock(strs);
	}

	if ( ! FarTP.isReselectingChar() ) // skip some parts if the user wants to quit in the middle of a char reselect
	{
		// Release the structure for the ping.
		Ping.release ();

		// Disable inputs
		Actions.enable(false);
		EditActions.enable(false);

		CWidgetManager::getInstance()->setDefaultCaptureKeyboard(NULL);

		// Interface saving
		CInterfaceManager::getInstance()->uninitInGame0();

		/////////////////////////////////
		// Display the end background. //
		/////////////////////////////////
		// Create the loading texture.
		loadBackgroundBitmap (EndBackground);

//	TTime endTime = ryzomGetLocalTime () + (TTime)(ClientCfg.EndScreenTimeOut*1000.f);
//	do
//	{
//		// Grab Inputs.
//		CInputHandlerManager::getInstance()->pumpEventsNoIM();
			// Display the background
			drawLoadingBitmap (0);
			// Display to screen.
			Driver->swapBuffers();
//	} while(ryzomGetLocalTime () < endTime);

		// Destroy the Loading Background.
		destroyLoadingBitmap ();

		IngameDbMngr.resetInitState();
	}

	ryzom_exit = true;

	return ryzom_exit || (Driver == NULL) || (!Driver->isActive ());
}// mainLoop //

//---------------------------------------------------
// Just Display some text with ... anim at some place.
//---------------------------------------------------
void	displaySpecialTextProgress(const char *text)
{
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(12);
	// Set the text color
	TextContext->setColor(CRGBA(255,255,255));

	TextContext->setHotSpot(UTextContext::BottomLeft);

	// cool .... anim.
	const	uint	MaxDot= 8;
	static	uint	counter=0;
	counter= (counter+1) % (MaxDot+1);
	// format counter.
	char	str[MaxDot+1];
	uint	i;
	for(i=0;i<counter;i++)
		str[i]= '.';
	str[counter]= 0;
	// display text
	TextContext->printfAt(0.05f,0.80f,"%s%s", text, str );
}


//---------------------------------------------------
// MovieShooter methods
//---------------------------------------------------


void	endMovieShooting()
{
	MovieShooterSaving= false;
}

bool MovieShooterReplay = false;
bool MovieShooterSave = false;

void	updateMovieShooting()
{
#ifdef _MOVIE_SHOOTER_ON_
	if (MovieShooterReplay)
	{
		if(!MovieShooter.enabled())
		{
			Driver->systemMessageBox("MovieShooter not enabled", "MovieShooter");
		}
		else
		{
			if( !MovieShooterSaving )
			{
				try
				{
					MovieShooter.replayMovie(Driver, TextContext);
				}
				catch (const Exception &e)
				{
					Driver->systemMessageBox(e.what(), "MovieShooter");
				}
			}
		}
		MovieShooterReplay = false;
	}

	if (MovieShooterSave)
	{
		if(!MovieShooter.enabled())
		{
			Driver->systemMessageBox("MovieShooter not enabled", "MovieShooter");
		}
		else
		{
			if( !MovieShooterSaving )
			{
				try
				{
					// Create a New Dir
					string	theDir;
					uint	num= 0;
					do
					{
						num++;
						char	tmp[256];
						sprintf(tmp, "%03d", num);
						theDir= ClientCfg.MovieShooterPath + "/" + "Movie" + tmp;
					}
					while( CFile::isDirectory(theDir) );
					// create the dir
					CFile::createDirectory(theDir);

					// Save the movie.
					MovieShooter.saveMovie(Driver, TextContext, theDir.c_str(), ClientCfg.MovieShooterFramePeriod, ClientCfg.MovieShooterBlend, ClientCfg.MovieShooterPrefix.c_str());
				}
				catch (const Exception &e)
				{
					Driver->systemMessageBox(e.what(), "MovieShooter");
				}
			}
		}
		MovieShooterSave = false;
	}
#endif // _MOVIE_SHOOTER_ON_
}


//---------------------------------------------------
// updateSound :
// update the listener pos and user walk/run/idle sound (stereo)
//---------------------------------------------------
void updateSound()
{
	if(SoundMngr)
	{
		SoundMngr->update ();
	}
}// updateSound //




//===================================================================================================
void updateLightDesc()
{
	if (!ClientCfg.Light)
	{
		// Update the lighting description (when season change, or for first setup)
		static bool init = false;
		EGSPD::CSeason::TSeason season = CurrSeason;
		if (!init || season != CurrSeason)
		{
			CLightCycleDesc desc;
			buildLightCycleDesc(desc, CurrSeason);
			LightCycleManager.setLightDesc(desc);
			init = true;
		}
		if (CurrSeason != season)
		{
			CurrSeason = season;
			// also update seasonal fxs
			if (IGCallbacks)
			{
				IGCallbacks->changeSeason();
			}
		}
	}
}

//===================================================================================================
static void updateCloudScape(const CCloudState &desc, const CWeatherContext &wc, float /* wind */, float dayNight, float updateDelay, bool mustInit)
{
	if (!CloudScape) return;
	SCloudScapeSetup css;
	NLMISC::clamp(dayNight, 0.f, 1.f);
	css.CloudSpeed   = desc.DiffusionSpeed;
	float duskRatio = LightCycleManager.getLightDesc().DuskRatio;
	switch(LightCycleManager.getState())
	{
		case CLightCycleManager::DayToNight:
			if (dayNight < duskRatio) // day->dusk
			{
				float blendFactor = duskRatio != 0 ? dayNight / duskRatio : 0.f;
				css.Ambient.blendFromui(desc.AmbientDay, desc.AmbientDusk, (uint) (256.f * blendFactor));
				css.Diffuse.blendFromui(desc.DiffuseDay, desc.DiffuseDusk, (uint) (256.f * blendFactor));
			}
			else // dusk->night
			{
				float blendFactor = duskRatio != 1 ? (dayNight - duskRatio) / (1.f - duskRatio) : 0.f;
				css.Ambient.blendFromui(desc.AmbientDusk, desc.AmbientNight, (uint) (256.f * blendFactor));
				css.Diffuse.blendFromui(desc.DiffuseDusk, desc.DiffuseNight, (uint) (256.f * blendFactor));
			}
		break;
		default: // not a day->night transition, so no step for dusk
			css.Ambient.blendFromui(desc.AmbientDay, desc.AmbientNight, (uint) (256.f * dayNight));
			css.Diffuse.blendFromui(desc.DiffuseDay, desc.DiffuseNight, (uint) (256.f * dayNight));
		break;

	}
	css.NbCloud      = desc.NumClouds;
	css.WindSpeed    = WeatherManager.getCurrWeatherState().WindIntensity * wc.WFP->CloudWindSpeedFactor + wc.WFP->CloudMinSpeed;
	if (mustInit)
	{
		css.TimeToChange = 0;
		CloudScape->init(&css);
		CloudScape->setNbCloudToUpdateIn80ms (ClientCfg.CloudUpdate);
		CloudScape->setQuality (ClientCfg.CloudQuality);
	}
	else
	{
		css.TimeToChange = updateDelay;
		CloudScape->set(css);
	}
}

//===================================================================================================
void updateClouds()
{
	// build a weather context
	CWeatherContext wc;
	wc.GR   = GR;
	if(ContinentMngr.cur())
		wc.WF = ContinentMngr.cur()->WeatherFunction;
	else
		wc.WF = NULL;

	if (ClientCfg.ManualWeatherSetup && !ForceTrueWeatherValue)
	{
		// Try to update the clouds quickly for manual test
		if (CloudScape)
		{
			CCloudState cs;
			WeatherManager.computeCloudState(ManualWeatherValue, CurrSeason, cs, wc.WF);
			updateCloudScape(cs, wc, WeatherManager.getCurrWeatherState().WindIntensity, LightCycleManager.getLightLevel(), 1.f, InitCloudScape);
			InitCloudScape = false;
		}
	}
	else
	{
		// update the clouds
		if (InitCloudScape)
		{
			InitCloudScape = false;
			// 1 ) set current state
			CCloudState cs;
			WeatherManager.computeCloudState(RT.getRyzomDay(), DayNightCycleHour, wc, cs);
			updateCloudScape(cs, wc, WeatherManager.getCurrWeatherState().WindIntensity, LightCycleManager.getLightLevel(), 0.f, true);
			// 2 )set next state
			// compute date of next update
			const CLightCycleDesc &lcd = LightCycleManager.getLightDesc();
			float updateDelay = 0.f;
			if (lcd.RealDayLength != 0.f)
			{
				 updateDelay = CloudUpdatePeriod / lcd.RealDayLength * lcd.NumHours;
			}
			WeatherManager.computeCloudState(RT.getRyzomDay(), DayNightCycleHour + updateDelay, wc, cs);
			updateCloudScape(cs, wc, WeatherManager.getCurrWeatherState().WindIntensity, LightCycleManager.getLightLevel(), 0.f, true);
			DelayBeforeCloudUpdate = CloudUpdatePeriod;
		}
		else
		{
			DelayBeforeCloudUpdate -= DT;
			if (DelayBeforeCloudUpdate <= 0.f)
			{
				DelayBeforeCloudUpdate += CloudUpdatePeriod;
				if (DelayBeforeCloudUpdate <= 0.f)
				{
					DelayBeforeCloudUpdate = CloudUpdatePeriod;
				}
				// set next state
				// compute date of next update
				const CLightCycleDesc &lcd = LightCycleManager.getLightDesc();
				float updateDelay = 0.f;
				if (lcd.RealDayLength != 0.f)
				{
					 updateDelay = DelayBeforeCloudUpdate / lcd.RealDayLength * lcd.NumHours;
				}
				CCloudState cs;
				WeatherManager.computeCloudState(RT.getRyzomDay(), DayNightCycleHour + updateDelay, wc, cs);
				updateCloudScape(cs, wc, WeatherManager.getCurrWeatherState().WindIntensity, LightCycleManager.getLightLevel(), updateDelay, false);
			}
		}
	}
}


//-----------------------------------------------
// displayPACSBorders :
// Display Borders from PACS.
//-----------------------------------------------
void displayPACSBorders()
{
	static std::vector<std::pair<NLMISC::CLine, uint8> > edges;
	if(UserEntity->getPrimitive())
	{
		UGlobalPosition gPos;
		UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
		if (GR) GR->getBorders(gPos, edges);

		CLineColor line;
		// ...
		for(uint i=0; i<edges.size(); ++i)
		{
			line = (edges[i].first);
			// Choose the color according to the edge type.
			switch(edges[i].second)
			{
			// Block
			case 0:
				line.Color0 = CRGBA::Red;
				line.Color1 = CRGBA::Red;
				break;
			// Surmountable
			case 1:
				line.Color0 = CRGBA::Green;
				line.Color1 = CRGBA::Green;
				break;
			// Link
			case 2:
				line.Color0 = CRGBA::Yellow;
				line.Color1 = CRGBA::Yellow;
				break;
			// Waterline
			case 3:
				line.Color0 = CRGBA::Blue;
				line.Color1 = CRGBA::Blue;
				break;
			case 4: // Exterior
				line.Color0 = CRGBA::Magenta;
				line.Color1 = CRGBA::Magenta;
				break;
			case 5: // Exterior door
				line.Color0 = CRGBA(127, 127, 127);
				line.Color1 = CRGBA(127, 127, 127);
				break;
			// Unknown
			default:
				line.Color0 = CRGBA::White;
				line.Color1 = CRGBA::White;
				break;
			}
			// Draw the line.
			Driver->drawLine(line, GenericMat);
		}
	}
	// Clear the vetor.
	edges.clear();
}// displayPACSBorders //

const uint PacsBoxPointCount = 24;

CVector PacsBox[PacsBoxPointCount] =
{
	CVector(	-0.5f,	-0.5f,	0),	CVector(	0.5f,	-0.5f,	0),
	CVector(	0.5f,	-0.5f,	0),	CVector(	0.5f,	0.5f,	0),
	CVector(	0.5f,	0.5f,	0),	CVector(	-0.5f,	0.5f,	0),
	CVector(	-0.5f,	0.5f,	0),	CVector(	-0.5f,	-0.5f,	0),

	CVector(	-0.5f,	-0.5f,	1),	CVector(	0.5f,	-0.5f,	1),
	CVector(	0.5f,	-0.5f,	1),	CVector(	0.5f,	0.5f,	1),
	CVector(	0.5f,	0.5f,	1),	CVector(	-0.5f,	0.5f,	1),
	CVector(	-0.5f,	0.5f,	1),	CVector(	-0.5f,	-0.5f,	1),

	CVector(	-0.5f,	-0.5f,	0),	CVector(	-0.5f,	-0.5f,	1),
	CVector(	0.5f,	-0.5f,	0),	CVector(	0.5f,	-0.5f,	1),
	CVector(	0.5f,	0.5f,	0),	CVector(	0.5f,	0.5f,	1),
	CVector(	-0.5f,	0.5f,	0),	CVector(	-0.5f,	0.5f,	1),
};

const uint PacsCylPointCount = 48;

CVector PacsCyl[PacsCylPointCount] =
{
	CVector(	0,			1,			0),CVector(	0.7071067f,	0.7071067f,	0),
	CVector(	0.7071067f,	0.7071067f,	0),CVector(	1,			0,			0),
	CVector(	1,			0,			0),CVector(	0.7071067f,	-0.7071067f,0),
	CVector(	0.7071067f,	-0.7071067f,0),CVector(	0,			-1,			0),
	CVector(	0,			-1,			0),CVector(	-0.7071067f,-0.7071067f,0),
	CVector(	-0.7071067f,-0.7071067f,0),CVector(	-1,			0,			0),
	CVector(	-1,			0,			0),CVector(	-0.7071067f,0.7071067f,	0),
	CVector(	-0.7071067f,0.7071067f,	0),CVector(	0,			1,			0),

	CVector(	0,			1,			1),CVector(	0.7071067f,	0.7071067f,	1),
	CVector(	0.7071067f,	0.7071067f,	1),CVector(	1,			0,			1),
	CVector(	1,			0,			1),CVector(	0.7071067f,	-0.7071067f,1),
	CVector(	0.7071067f,	-0.7071067f,1),CVector(	0,			-1,			1),
	CVector(	0,			-1,			1),CVector(	-0.7071067f,-0.7071067f,1),
	CVector(	-0.7071067f,-0.7071067f,1),CVector(	-1,			0,			1),
	CVector(	-1,			0,			1),CVector(	-0.7071067f,0.7071067f,	1),
	CVector(	-0.7071067f,0.7071067f,	1),CVector(	0,			1,			1),

	CVector(	0,			1,			1),CVector(	0,			1,			0),
	CVector(	0.7071067f,	0.7071067f,	1),CVector(	0.7071067f,	0.7071067f,	0),
	CVector(	1,			0,			1),CVector(	1,			0,			0),
	CVector(	0.7071067f,	-0.7071067f,1),CVector(	0.7071067f,	-0.7071067f,0),
	CVector(	0,			-1,			1),CVector(	0,			-1,			0),
	CVector(	-0.7071067f,-0.7071067f,1),CVector(	-0.7071067f,-0.7071067f,0),
	CVector(	-1,			0,			1),CVector(	-1,			0,			0),
	CVector(	-0.7071067f,0.7071067f,	1),CVector(	-0.7071067f,0.7071067f,	0),
};

void displayPACSPrimitive()
{
	std::vector<const UMovePrimitive*> movePrimitives;

	// if no continent selected, skip
	if(!PACS)
		return;

	PACS->getPrimitives(movePrimitives);

	uint i;
	for (i=0; i<movePrimitives.size(); i++)
	{
		// World image
		const UMovePrimitive *prim = movePrimitives[i];
		uint8 wI = prim->getFirstWorldImageV();

		// Distance
		CVector position = prim->getFinalPosition(wI);
		if ((position-UserEntity->pos()).sqrnorm() < (200*200))
		{
			// Choose a color
			CLineColor line;
			if (prim->isCollisionable())
			{
				// Static collision
				if (prim->getReactionType() == UMovePrimitive::DoNothing)
				{
					line.Color0 = CRGBA::Red;
				}
				else
				{
					line.Color0 = CRGBA::Yellow;
				}
			}
			else
			{
				// Trigger
				line.Color0 = CRGBA::White;
			}
			line.Color1 = line.Color0;

			// Lines
			CVector *lines;

			// Line count
			uint linecount;

			// Transform matrix
			CMatrix scale;
			scale.identity();
			CMatrix rot;
			rot.identity();

			// Draw the primitive
			if (prim->getPrimitiveType() == UMovePrimitive::_2DOrientedBox)
			{
				lines = PacsBox;
				linecount = PacsBoxPointCount/2;
				float width;
				float depth;
				prim->getSize (width, depth);
				scale.scale(CVector (width, depth, prim->getHeight()));
				rot.rotateZ((float)prim->getOrientation(wI));
			}
			else
			{
				lines = PacsCyl;
				linecount = PacsCylPointCount/2;
				float radius = prim->getRadius ();
				scale.scale(CVector (radius, radius, prim->getHeight()));
			}

			CMatrix pos;
			pos.identity();
			pos.setPos (position);
			pos = pos*rot*scale;

			// Draw the primitive
			uint j;
			for (j=0; j<linecount; j++)
			{
				line.V0 = pos * lines[j*2];
				line.V1 = pos * lines[j*2+1];

				// Draw the line.
				Driver->drawLine(line, GenericMat);
			}
		}
	}
}

//-----------------------------------------------
// displaySoundBox :
//-----------------------------------------------
void displaySoundBox()
{
	if (SoundMngr != 0)
	{
		SoundMngr->drawSounds(50.f);
	}
}


//-----------------------------------------------
// displaySceneProfiles();
// nlinfo the Scene Profile results
//-----------------------------------------------
string		buildStrVBFormat(uint32 format)
{
	// Yoyo: Uggly: hardcoded :)
	string	res;

	if(format & 0x0001)
		res+= "Vertex";
	if(format & 0x0002)
		res+= "|Normal";
	if(format & 0x0004)
		res+= "|TexCoord0";
	if(format & 0x0008)
		res+= "|TexCoord1 ";
	if(format & 0x0010)
		res+= "|TexCoord2";
	if(format & 0x0020)
		res+= "|TexCoord3 ";
	if(format & 0x0040)
		res+= "|TexCoord4 ";
	if(format & 0x0080)
		res+= "|TexCoord5 ";
	if(format & 0x0100)
		res+= "|TexCoord6 ";
	if(format & 0x0200)
		res+= "|TexCoord7";
	if(format & 0x0400)
		res+= "|PrimaryColor";
	if(format & 0x0800)
		res+= "|SecondaryColor";
	if(format & 0x1000)
		res+= "|Weight";
	if(format & 0x2000)
		res+= "|PaletteSkin";
	if(format & 0x4000)
		res+= "|Fog";

	return res;
}
void		displaySceneProfiles()
{
	// **** Scene Profile
	UScene::CBenchResults	res;
	Scene->getProfileResults(res);

	static	uint id=0;
	nlinfo("****** Scene Profile Result: %d *******", id );
	id++;

	// Display Mesh PerVertexFormat Benchs
	nlinfo(" * Mesh Per VertexFormat:");
	std::map<uint32, uint32>::iterator	it;
	for(it=res.MeshProfileTriVBFormat.begin();it!=res.MeshProfileTriVBFormat.end();it++)
	{
		// build the format
		string	format= buildStrVBFormat(it->first);
		nlinfo("   NumTris: %5d. Format: %s", it->second, format.c_str() );
	}

	// Display MeshMRM PerVertexFormat Benchs
	nlinfo(" * MeshMRM Per VertexFormat:");
	for(it=res.MeshMRMProfileTriVBFormat.begin();it!=res.MeshMRMProfileTriVBFormat.end();it++)
	{
		// build the format
		string	format= buildStrVBFormat(it->first);
		nlinfo("   NumTris: %5d. Format: %s", it->second, format.c_str() );
	}

	// Display BlockRendering Information
	nlinfo(" * Mesh BlockRender Info:");
	nlinfo("   NumMeshRdrNormal: %d", res.NumMeshRdrNormal);
	nlinfo("   NumMeshRdrBlock: %d", res.NumMeshRdrBlock);
	nlinfo("   NumMeshRdrBlockWithVBHeap: %d", res.NumMeshRdrBlockWithVBHeap);
	nlinfo("   NumMeshRdrNormal TriCount: %d", res.NumMeshTriRdrNormal);
	nlinfo("   NumMeshRdrBlock TriCount: %d", res.NumMeshTriRdrBlock);
	nlinfo("   NumMeshRdrBlockWithVBHeap TriCount: %d", res.NumMeshTriRdrBlockWithVBHeap);

	nlinfo(" * MeshMRM BlockRender Info:");
	nlinfo("   NumMeshMRMRdrNormal: %d", res.NumMeshMRMRdrNormal);
	nlinfo("   NumMeshMRMRdrBlock: %d", res.NumMeshMRMRdrBlock);
	nlinfo("   NumMeshMRMRdrBlockWithVBHeap: %d", res.NumMeshMRMRdrBlockWithVBHeap);
	nlinfo("   NumMeshMRMRdrNormal TriCount: %d", res.NumMeshMRMTriRdrNormal);
	nlinfo("   NumMeshMRMRdrBlock TriCount: %d", res.NumMeshMRMTriRdrBlock);
	nlinfo("   NumMeshMRMRdrBlockWithVBHeap TriCount: %d", res.NumMeshMRMTriRdrBlockWithVBHeap);

	// Display VBHard usage Information
	nlinfo(" * VBHard usage Info:");
	nlinfo("   NumMeshVBufferStd: %d", res.NumMeshVBufferStd);
	nlinfo("   NumMeshVBufferHard: %d", res.NumMeshVBufferHard);
	nlinfo("   NumMeshMRMVBufferStd: %d", res.NumMeshMRMVBufferStd);
	nlinfo("   NumMeshMRMVBufferHard: %d", res.NumMeshMRMVBufferHard);


	nlinfo("****** END Scene Profile Result *******");

	// **** Additionaly, display QuadGridClipManager profile
	Scene->profileQuadGridClipManager();

	// **** Additionaly, display List of Landscape IG Loaded
	if (Landscape)
	{
		nlinfo("****** Land IG Profile *******");
		std::vector<std::pair<UInstanceGroup *, std::string> >	igList;
		LandscapeIGManager.getAllIGWithNames(igList);
		// For all ig
		for(uint i=0;i<igList.size();i++)
		{
			string	igName= CFile::getFilenameWithoutExtension(igList[i].second);
			if( LandscapeIGManager.isIGAddedToScene(igName) )
				nlinfo("%s", igList[i].second.c_str() );
		}

		nlinfo("****** END Land IG Profile *******");
	}

}

//-----------------------------------------------
// validateDialogs();
// validate current dialogs by checking the distance between the two talking entities. End dialogs if they are too far
//-----------------------------------------------
void validateDialogs(const CGameContextMenu &gcm)
{

	if ( UserEntity->trader() != CLFECOMMON::INVALID_SLOT )
	{
		CEntityCL * trader = EntitiesMngr.entity(UserEntity->trader());
		if (trader)
		{
			CVectorD vect1 = trader->pos();
			CVectorD vect2 = UserEntity->pos();

			double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);

			if (gcm.isBuilding())
			{
				if (distanceSquare > MaxTalkingOutpostBuildingDistSquare)
				{
					// Prevent also the Server of ending bot chat
					if (CBotChatManager::getInstance()->getCurrPage())
						CBotChatManager::getInstance()->endDialog();
				}
			}
			else
			{
				if(distanceSquare > MaxTalkingDistSquare)
				{
					// Prevent also the Server of ending bot chat
					if(CBotChatManager::getInstance()->getCurrPage())
						CBotChatManager::getInstance()->endDialog();
				}
			}
		}
	}
}

NLMISC_DYNVARIABLE(float, FPS, "The second smoothed frame rate per second")
{
	if (get) *pointer = 1.0f/smoothFPS.getSmoothValue ();
}

// show hide all the debuging of ui
NLMISC_COMMAND(debugUI, "Debug the ui : show/hide quads of bboxs and hotspots", "debugUI 1 or 0")
{
	if (args.size() > 1) return false;
	bool on = true;
	DebugUIFilter.clear();
	if (args.size() == 1)
	{
		if(!args[0].empty() && !isdigit(args[0][0]))
		{
			on= true;
			DebugUIFilter= args[0];
		}
		else
			fromString(args[0], on);
	}

	CGroupCell::setDebugUICell( on );
	DebugUIView = on;
	DebugUICtrl = on;
	DebugUIGroup = on;
	return true;
}


// show hide the debuging of ui
NLMISC_COMMAND(debugUIView, "Debug the ui : show/hide quads of bboxs and hotspots for views", "")
{
	DebugUIView = !DebugUIView;
	return true;
}

// show hide the debuging of ui
NLMISC_COMMAND(debugUICtrl, "Debug the ui : show/hide quads of bboxs and hotspots for ctrl", "")
{
	DebugUICtrl = !DebugUICtrl;
	return true;
}

// show hide the debuging of ui
NLMISC_COMMAND(debugUIGroup, "Debug the ui : show/hide quads of bboxs and hotspots for group", "")
{
	DebugUIGroup = !DebugUIGroup;
	return true;
}

// show hide the debuging of cells
NLMISC_COMMAND(debugUICell, "Debug the ui : show/hide quads of bboxs for cells", "")
{
	CGroupCell::setDebugUICell( !CGroupCell::getDebugUICell() );
	return true;
}

// Set weather value
NLMISC_COMMAND(setWeatherValue, "Set weather value", "")
{
	if (args.size() != 1) return false;
	float value;
	fromString(args[0], value);
	ManualWeatherValue = value / (ContinentMngr.cur()->WeatherFunction[CurrSeason].getNumWeatherSetups() - 1);
	return true;
}

class CHandlerDebugUIGroup : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		DebugUIGroup = !DebugUIGroup;
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUIGroup, "debug_ui_group");

class CHandlerDebugUICtrl : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		DebugUICtrl = !DebugUICtrl;
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUICtrl, "debug_ui_ctrl");

class CHandlerDebugUIView : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		DebugUIView = !DebugUIView;
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUIView, "debug_ui_view");


class CAHShowTimedFX : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		if (ShowTimedFX)
		{
			ShowTimedFXMode = (CTimedFXManager::TDebugDisplayMode) (ShowTimedFXMode + 1);
			if (ShowTimedFXMode == CTimedFXManager::DebugModeCount)
			{
				ShowTimedFX = false;
			}
		}
		else
		{
			ShowTimedFX = true;
			ShowTimedFXMode = CTimedFXManager::NoText;
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHShowTimedFX, "show_timed_fx");


// ***************************************************************************
void	displayDebugClusters()
{
	// Get the Cluster system where the player (not the camera!) lies
	UGlobalPosition gPos;
	if(UserEntity->getPrimitive())
	UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
	// get the cluster IG associated to this pacs position
	UInstanceGroup	*ig = getCluster(gPos);

	// Then display debug for it!
	if(ig)
		ig->displayDebugClusters(Driver, TextContext);

	// Draw the last camera 3rd person setuped
	CLineColor line;

	// start to test start
	line.V0= LastDebugClusterCameraThirdPersonStart;
	line.V1= LastDebugClusterCameraThirdPersonTestStart;
	line.Color0= CRGBA::Blue;
	line.Color1= CRGBA::Green;
	Driver->drawLine(line, GenericMat);

	// test start to result
	line.V0= LastDebugClusterCameraThirdPersonTestStart;
	if(LastDebugClusterCameraThirdPersonForceFPV)
		line.V1= LastDebugClusterCameraThirdPersonEnd;
	else
		line.V1= LastDebugClusterCameraThirdPersonResult;
	line.Color0= CRGBA::Red;
	line.Color1= CRGBA::Green;
	Driver->drawLine(line, GenericMat);

	// result to end
	if(!LastDebugClusterCameraThirdPersonForceFPV)
	{
		line.V0= LastDebugClusterCameraThirdPersonResult;
		line.V1= LastDebugClusterCameraThirdPersonEnd;
		line.Color0= CRGBA::Yellow;
		line.Color1= CRGBA::Green;
		Driver->drawLine(line, GenericMat);
	}

	// pelvis pos to test start
	line.V0= LastDebugClusterCameraThirdPersonPelvisPos;
	line.V1= LastDebugClusterCameraThirdPersonTestStart;
	line.Color0= CRGBA::Magenta;
	line.Color1= CRGBA::Green;
	Driver->drawLine(line, GenericMat);

}


// ***************************************************************************
void inGamePatchUncompleteWarning()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CLuaManager::getInstance().executeLuaScript("bgdownloader:inGamePatchUncompleteWarning()");
	/*
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:bg_downloader"));
	if (gc)
	{
		gc->setActive(true);
		CWidgetManager::getInstance()->setTopWindow(gc);
		gc->enableBlink(2);
	}
	im->messageBoxWithHelp(CI18N::get("uiBGD_InGamePatchIncomplete"));
	im->displaySystemInfo(CI18N::get("uiBGD_InGamePatchIncompleteBC"), "BC");
	*/
}
