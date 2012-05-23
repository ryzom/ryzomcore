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
#include "fog_map.h"
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
#include "interface_v3/action_handler.h"
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
#include "interface_v3/lua_ihm.h"
#include "interface_v3/lua_ihm_ryzom.h"
#include "far_tp.h"
#include "session_browser_impl.h"
#include "bg_downloader_access.h"
#include "login_progress_post_thread.h"
#include "npc_icon.h"

// R2ED
#include "r2/editor.h"

#include "nel/misc/check_fpu.h"

// TMP TMP
#include "interface_v3/ctrl_polygon.h"
// TMP TMP
#include "game_share/scenario_entry_points.h"
#include "nel/3d/driver.h"
#include "nel/3d/texture_file.h"

#include "nel/3d/packed_world.h"
#include "nel/3d/packed_zone.h"
#include "nel/3d/driver_user.h"


#ifdef USE_WATER_ENV_MAP
	#include "water_env_map_rdr.h"
#endif

// temp
#include "precipitation.h"
#include "interface_v3/bot_chat_manager.h"
#include "string_manager_client.h"

#include "interface_v3/lua_manager.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace NLNET;
using namespace std;


// TMP TMP
static void viewportToScissor(const CViewport &vp, CScissor &scissor)
{
	scissor.X = vp.getX();
	scissor.Y = vp.getY();
	scissor.Width = vp.getWidth();
	scissor.Height = vp.getHeight();
}


////////////
// EXTERN //
////////////
extern std::set<std::string>	LodCharactersNotFound;
extern UDriver					*Driver;
extern IMouseDevice				*MouseDevice;
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
extern uint32					NbDatabaseChanges;
extern CMatrix					MainSceneViewMatrix;
extern CMatrix					InvMainSceneViewMatrix;
extern std::vector<UTextureFile*> LogoBitmaps;
extern bool						IsInRingSession;
extern std::string				UsedFSAddr;

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



///////////
// CLASS //
///////////
/**
 * Class to manage the ping computed with the database.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2003
 */
class CPing : public ICDBNode::IPropertyObserver
{
private:
	uint32	_Ping;
	bool	_RdyToPing;

public:
	// Constructor.
	CPing() {_Ping = 0; _RdyToPing = true;}
	// Destructor.
	~CPing() {;}

	// Add an observer on the database for the ping.
	void init()
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if(pIM)
		{
			CCDBNodeLeaf *pNodeLeaf = pIM->getDbProp("SERVER:DEBUG_INFO:Ping", false);
			if(pNodeLeaf)
			{
				ICDBNode::CTextId textId;
				pNodeLeaf->addObserver(this, textId);
				//	nlwarning("CPing: cannot add the observer");
			}
			else
				nlwarning("CPing: 'SERVER:DEBUG_INFO:Ping' does not exist.");
		}
	}

	// Release the observer on the database for the ping.
	void release()
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if(pIM)
		{
			CCDBNodeLeaf *pNodeLeaf = pIM->getDbProp("SERVER:DEBUG_INFO:Ping", false);
			if(pNodeLeaf)
			{
				ICDBNode::CTextId textId;
				pNodeLeaf->removeObserver(this, textId);
			}
			else
				nlwarning("CPing: 'SERVER:DEBUG_INFO:Ping' does not exist.");
		}
	}

	// Method called when the ping message is back.
	virtual void update(ICDBNode* node)
	{
		CCDBNodeLeaf *leaf = safe_cast<CCDBNodeLeaf *>(node);
		uint32 before = (uint32)leaf->getValue32();
		uint32 current = (uint32)(0xFFFFFFFF & ryzomGetLocalTime());
		if(before > current)
		{
			//nlwarning("DB PING Pb before '%u' after '%u'.", before, current);
			if(ClientCfg.Check)
				nlstop;
		}
		_Ping = current - before;
		_RdyToPing = true;
	}

	// return the ping in ms.
	uint32 getValue() {return _Ping;}

	void rdyToPing(bool rdy) {_RdyToPing = rdy;}
	bool rdyToPing() const {return _RdyToPing;}
};

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
bool				DebugUICell = false;
bool				DebugUIView = false;
bool				DebugUICtrl = false;
bool				DebugUIGroup = false;
std::string			DebugUIFilter;
bool				ShowHelp = false;	// Do the Help have to be displayed.
uint8				ShowInfos = 0;		// 0=no info 1=text info 2=graph info

bool				bZeroCpu = false;	// For no Cpu use if application is minimize  TODO: intercept minimize message, called by CTRL + Z at this

bool				Profiling = false;			// Are we in Profile mode?
uint				ProfileNumFrame = 0;
bool				WantProfiling = false;
bool				ProfilingVBLock = false;
bool				WantProfilingVBLock = false;

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




NLMISC::CValueSmoother smoothFPS;
NLMISC::CValueSmoother moreSmoothFPS(64);


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
// Display some debug infos.
void displayDebug();
void displayDebugFps();
void displayDebugUIUnderMouse();
// Display an Help.
void displayHelp();

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

void buildCameraClippingPyramid (vector<CPlane> &planes)
{
	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	const CFrustum &frustum  = MainCam.getFrustum();
	InvMainSceneViewMatrix = MainCam.getMatrix();
	MainSceneViewMatrix = InvMainSceneViewMatrix;
	MainSceneViewMatrix.invert();

	CVector		lb(frustum.Left,  frustum.Near, frustum.Bottom );
	CVector		lt(frustum.Left,  frustum.Near, frustum.Top    );
	CVector		rb(frustum.Right, frustum.Near, frustum.Bottom );
	CVector		rt(frustum.Right, frustum.Near, frustum.Top    );

	CVector		lbFar(frustum.Left,  ClientCfg.CharacterFarClip, frustum.Bottom);
	CVector		ltFar(frustum.Left,  ClientCfg.CharacterFarClip, frustum.Top   );
	CVector		rtFar(frustum.Right, ClientCfg.CharacterFarClip, frustum.Top   );

	planes.resize (4);
	// planes[0].make(lbFar, ltFar, rtFar);
	planes[0].make(pfoc, lt, lb);
	planes[1].make(pfoc, rt, lt);
	planes[2].make(pfoc, rb, rt);
	planes[3].make(pfoc, lb, rb);

	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	// Here, ViewMatrix== CamMatrix-1. Hence the following formula.
	uint i;

	for (i = 0; i < 4; i++)
	{
		planes[i] = planes[i]*MainSceneViewMatrix;
	}
}


//---------------------------------------------------
// Test Profiling and run?
//---------------------------------------------------
void	testLaunchProfile()
{
	if(!WantProfiling)
		return;

	// comes from ActionHandler
	WantProfiling= false;

#ifdef _PROFILE_ON_
	if( !Profiling )
	{
		// start the bench.
		NLMISC::CHTimer::startBench();
		ProfileNumFrame = 0;
		Driver->startBench();
		if (SoundMngr)
			SoundMngr->getMixer()->startDriverBench();
		// state
		Profiling= true;
	}
	else
	{
		// end the bench.
		if (SoundMngr)
			SoundMngr->getMixer()->endDriverBench();
		NLMISC::CHTimer::endBench();
		Driver->endBench();


		// Display and save profile to a File.
		CLog	log;
		CFileDisplayer	fileDisplayer(NLMISC::CFile::findNewFile(getLogDirectory() + "profile.log"));
		CStdDisplayer	stdDisplayer;
		log.addDisplayer(&fileDisplayer);
		log.addDisplayer(&stdDisplayer);
		// diplay
		NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(&log, CHTimer::TotalTime, true, 48, 2);
		NLMISC::CHTimer::displayHierarchical(&log, true, 48, 2);
		NLMISC::CHTimer::displayByExecutionPath(&log, CHTimer::TotalTime);
		NLMISC::CHTimer::display(&log, CHTimer::TotalTime);
		NLMISC::CHTimer::display(&log, CHTimer::TotalTimeWithoutSons);
		Driver->displayBench(&log);

		if (SoundMngr)
			SoundMngr->getMixer()->displayDriverBench(&log);

		// state
		Profiling= false;
	}
#endif	// #ifdef _PROFILE_ON_
}


//---------------------------------------------------
// Test ProfilingVBLock and run?
//---------------------------------------------------
void	testLaunchProfileVBLock()
{
	// If running, must stop for this frame.
	if(ProfilingVBLock)
	{
		vector<string>	strs;
		Driver->endProfileVBHardLock(strs);
		nlinfo("Profile VBLock");
		nlinfo("**************");
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		ProfilingVBLock= false;

		// Display additional info on allocated VBHards
		nlinfo("VBHard list");
		nlinfo("**************");
		Driver->profileVBHardAllocation(strs);
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		strs.clear();
		Driver->endProfileIBLock(strs);
		nlinfo("Profile Index Buffer Lock");
		nlinfo("**************");
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		ProfilingVBLock= false;

		// Display additional info on allocated VBHards
		/*
		nlinfo("Index Buffer list");
		nlinfo("**************");
		Driver->profileIBAllocation(strs);
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		*/
	}

	// comes from ActionHandler
	if(WantProfilingVBLock)
	{
		WantProfilingVBLock= false;
		ProfilingVBLock= true;
		Driver->startProfileVBHardLock();
		Driver->startProfileIBLock();
	}
}


//---------------------------------------------------
// update the camera perspective setup
//---------------------------------------------------
void	updateCameraPerspective()
{
	float	fov, aspectRatio;
	computeCurrentFovAspectRatio(fov, aspectRatio);

	// change the perspective of the scene
	if(!MainCam.empty())
		MainCam.setPerspective(fov, aspectRatio, CameraSetupZNear, ClientCfg.Vision);
	// change the perspective of the root scene
	if(SceneRoot)
	{
		UCamera cam= SceneRoot->getCam();
		cam.setPerspective(fov, aspectRatio, SceneRootCameraZNear, SceneRootCameraZFar);
	}
}

//---------------------------------------------------
// Compare ClientCfg and LastClientCfg to know what we must update
//---------------------------------------------------
void updateFromClientCfg()
{
	CClientConfig::setValues();
	ClientCfg.IsInvalidated = false;

	// GRAPHICS - GENERAL
	//---------------------------------------------------
	if ((ClientCfg.Windowed != LastClientCfg.Windowed)	||
		(ClientCfg.Width != LastClientCfg.Width)		||
		(ClientCfg.Height != LastClientCfg.Height)		||
		(ClientCfg.Depth != LastClientCfg.Depth)		||
		(ClientCfg.Frequency != LastClientCfg.Frequency))
	{
		setVideoMode(UDriver::CMode(ClientCfg.Width, ClientCfg.Height, (uint8)ClientCfg.Depth,
									ClientCfg.Windowed, ClientCfg.Frequency));
	}

	if (ClientCfg.DivideTextureSizeBy2 != LastClientCfg.DivideTextureSizeBy2)
	{
		if (ClientCfg.DivideTextureSizeBy2)
			Driver->forceTextureResize(2);
		else
			Driver->forceTextureResize(1);
	}

	//---------------------------------------------------
	if (ClientCfg.WaitVBL != LastClientCfg.WaitVBL)
	{
		if(ClientCfg.WaitVBL)
			Driver->setSwapVBLInterval(1);
		else
			Driver->setSwapVBLInterval(0);
	}

	// GRAPHICS - LANDSCAPE
	//---------------------------------------------------
	if (ClientCfg.LandscapeThreshold != LastClientCfg.LandscapeThreshold)
	{
		if (Landscape) Landscape->setThreshold(ClientCfg.getActualLandscapeThreshold());
	}

	//---------------------------------------------------
	if (ClientCfg.LandscapeTileNear != LastClientCfg.LandscapeTileNear)
	{
		if (Landscape) Landscape->setTileNear(ClientCfg.LandscapeTileNear);
	}

	//---------------------------------------------------
	if (Landscape)
	{
		if (ClientCfg.Vision != LastClientCfg.Vision)
		{
			if (!ClientCfg.Light)
			{
				// Not in an indoor ?
				if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
				{
					// Refresh All Zone in streaming according to the refine position
					vector<string>		zonesAdded;
					vector<string>		zonesRemoved;
					const R2::CScenarioEntryPoints::CCompleteIsland *ci = R2::CScenarioEntryPoints::getInstance().getCompleteIslandFromCoords(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y));
					Landscape->refreshAllZonesAround(View.refinePos(), ClientCfg.Vision + ExtraZoneLoadingVision, zonesAdded, zonesRemoved, ProgressBar, ci ? &(ci->ZoneIDs) : NULL);
					LandscapeIGManager.unloadArrayZoneIG(zonesRemoved);
					LandscapeIGManager.loadArrayZoneIG(zonesAdded);
				}
			}
		}
	}

	//---------------------------------------------------
	if (ClientCfg.Vision != LastClientCfg.Vision || ClientCfg.FoV!=LastClientCfg.FoV ||
		ClientCfg.Windowed != LastClientCfg.Windowed || ClientCfg.ScreenAspectRatio != LastClientCfg.ScreenAspectRatio )
	{
		updateCameraPerspective();
	}

	//---------------------------------------------------
	if (Landscape)
	{
		if (ClientCfg.MicroVeget != LastClientCfg.MicroVeget)
		{
			if(ClientCfg.MicroVeget)
			{
				// if configured, enable the vegetable and load the texture.
				Landscape->enableVegetable(true);
				// Default setup. TODO later by gameDev.
				Landscape->setVegetableWind(CVector(0.5, 0.5, 0).normed(), 0.5, 1, 0);
				// Default setup. should work well for night/day transition in 30 minutes.
				// Because all vegetables will be updated every 20 seconds => 90 steps.
				Landscape->setVegetableUpdateLightingFrequency(1/20.f);
				// Density (percentage to ratio)
				Landscape->setVegetableDensity(ClientCfg.MicroVegetDensity/100.f);
			}
			else
			{
				Landscape->enableVegetable(false);
			}
		}
	}

	//---------------------------------------------------
	if (ClientCfg.MicroVegetDensity != LastClientCfg.MicroVegetDensity)
	{
		// Density (percentage to ratio)
		if (Landscape) Landscape->setVegetableDensity(ClientCfg.MicroVegetDensity/100.f);
	}

	// GRAPHICS - SPECIAL EFFECTS
	//---------------------------------------------------
	if (ClientCfg.FxNbMaxPoly != LastClientCfg.FxNbMaxPoly)
	{
		if (Scene->getGroupLoadMaxPolygon("Fx") != ClientCfg.FxNbMaxPoly)
			Scene->setGroupLoadMaxPolygon("Fx", ClientCfg.FxNbMaxPoly);
	}

	//---------------------------------------------------
	if (ClientCfg.Cloud != LastClientCfg.Cloud)
	{
		if (ClientCfg.Cloud)
		{
			InitCloudScape = true;
			CloudScape = Scene->createCloudScape();
		}
		else
		{
			if (CloudScape != NULL)
				Scene->deleteCloudScape(CloudScape);
			CloudScape = NULL;
		}
	}

	//---------------------------------------------------
	if (ClientCfg.CloudQuality != LastClientCfg.CloudQuality)
	{
		if (CloudScape != NULL)
			CloudScape->setQuality(ClientCfg.CloudQuality);
	}

	//---------------------------------------------------
	if (ClientCfg.CloudUpdate != LastClientCfg.CloudUpdate)
	{
		if (CloudScape != NULL)
			CloudScape->setNbCloudToUpdateIn80ms(ClientCfg.CloudUpdate);
	}

	//---------------------------------------------------
	if (ClientCfg.Shadows != LastClientCfg.Shadows)
	{
		// Enable/Disable Receive on Landscape
		if(Landscape)
		{
			Landscape->enableReceiveShadowMap(ClientCfg.Shadows);
		}
		// Enable/Disable Cast for all entities
		for(uint i=0;i<EntitiesMngr.entities().size();i++)
		{
			CEntityCL	*ent= EntitiesMngr.entities()[i];
			if(ent)
				ent->updateCastShadowMap();
		}
	}

	// GRAPHICS - CHARACTERS
	//---------------------------------------------------
	if (ClientCfg.SkinNbMaxPoly != LastClientCfg.SkinNbMaxPoly)
	{
		if (Scene->getGroupLoadMaxPolygon("Skin") != ClientCfg.SkinNbMaxPoly)
			Scene->setGroupLoadMaxPolygon("Skin", ClientCfg.SkinNbMaxPoly);
	}

	//---------------------------------------------------
	if (ClientCfg.NbMaxSkeletonNotCLod != LastClientCfg.NbMaxSkeletonNotCLod )
	{
		Scene->setMaxSkeletonsInNotCLodForm(ClientCfg.NbMaxSkeletonNotCLod);
	}

	//---------------------------------------------------
	if (ClientCfg.CharacterFarClip != LastClientCfg.CharacterFarClip)
	{
		// Nothing to do
	}

	//---------------------------------------------------
	if (ClientCfg.HDEntityTexture != LastClientCfg.HDEntityTexture)
	{
		// Don't reload Texture, will be done at next Game Start
	}

	// INTERFACE works


	// INPUTS
	//---------------------------------------------------
	if (ClientCfg.CursorSpeed != LastClientCfg.CursorSpeed)
		SetMouseSpeed (ClientCfg.CursorSpeed);

	if (ClientCfg.CursorAcceleration != LastClientCfg.CursorAcceleration)
		SetMouseAcceleration (ClientCfg.CursorAcceleration);

	if (ClientCfg.HardwareCursor != LastClientCfg.HardwareCursor)
	{
		if (ClientCfg.HardwareCursor != IsMouseCursorHardware())
		{
			InitMouseWithCursor (ClientCfg.HardwareCursor);
		}
	}


	// SOUND
	//---------------------------------------------------
	bool	mustReloadSoundMngrContinent= false;

	// disable/enable sound?
	if (ClientCfg.SoundOn != LastClientCfg.SoundOn)
	{
		if (SoundMngr && !ClientCfg.SoundOn)
		{
			nlwarning("Deleting sound manager...");
			delete SoundMngr;
			SoundMngr = NULL;
		}
		else if (SoundMngr == NULL && ClientCfg.SoundOn)
		{
			nlwarning("Creating sound manager...");
			SoundMngr = new CSoundManager();
			try
			{
				SoundMngr->init(NULL);
			}
			catch(const Exception &e)
			{
				nlwarning("init : Error when creating 'SoundMngr' : %s", e.what());
				SoundMngr = 0;
			}

			// re-init with good SFX/Music Volume
			if(SoundMngr)
			{
				SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
				SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);
			}
		}
		else
		{
			nlwarning("Sound config error !");
		}

		mustReloadSoundMngrContinent= true;
	}

	// change EAX?
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.UseEax != LastClientCfg.UseEax) )
	{
		SoundMngr->reset();

		mustReloadSoundMngrContinent= true;
	}

	// change SoundForceSoftwareBuffer?
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.SoundForceSoftwareBuffer != LastClientCfg.SoundForceSoftwareBuffer) )
	{
		SoundMngr->reset();

		mustReloadSoundMngrContinent= true;
	}

	// change MaxTrack? don't reset
	if ( SoundMngr && LastClientCfg.SoundOn &&
		(ClientCfg.MaxTrack != LastClientCfg.MaxTrack))
	{
		SoundMngr->getMixer()->changeMaxTrack(ClientCfg.MaxTrack);
	}

	// change SoundFX Volume? don't reset
	if (SoundMngr && ClientCfg.SoundSFXVolume != LastClientCfg.SoundSFXVolume)
	{
		SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
	}

	// change Game Music Volume? don't reset
	if (SoundMngr && ClientCfg.SoundGameMusicVolume != LastClientCfg.SoundGameMusicVolume)
	{
		SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);
	}

	// reload only if active and reseted
	if (mustReloadSoundMngrContinent && SoundMngr && ContinentMngr.cur() && !ContinentMngr.cur()->Indoor && UserEntity)
	{
		SoundMngr->loadContinent(ContinentMngr.getCurrentContinentSelectName(), UserEntity->pos());
	}

	// Ok backup the new clientcfg
	LastClientCfg = ClientCfg;
}

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

//uint32 MainLoopCounter = 0;


// ***************************************************************************
enum TSkyMode { NoSky, OldSky, NewSky };


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

void	beginRenderSkyPart(TSkyMode skyMode)
{
	if(skyMode == NewSky)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		sky.getScene()->beginPartRender();
	}
}
void	endRenderSkyPart(TSkyMode skyMode)
{
	if(skyMode == NewSky)
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

	// Set The Root Camera
	UCamera camRoot = SceneRoot->getCam();
	if(!camRoot.empty())
	{
		// Update Camera Position/Rotation.
		camRoot.setPos(View.currentViewPos());
		camRoot.setRotQuat(View.currentView());
	}
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
static void renderSkyPart(UScene::TRenderPart renderPart, TSkyMode skyMode)
{
	nlassert(skyMode != NoSky);
	Driver->setDepthRange(SKY_DEPTH_RANGE_START, 1.f);
	Driver->enableFog(false);
	if (skyMode == NewSky)
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
// Render all scenes
void renderAll(bool forceFullDetail)
{
	if (ClientCfg.Bloom)
	{
		// set bloom parameters before applying bloom effect
		CBloomEffect::getInstance().setSquareBloom(ClientCfg.SquareBloom);
		CBloomEffect::getInstance().setDensityBloom((uint8)ClientCfg.DensityBloom);
		// init bloom
		CBloomEffect::getInstance().initBloom();
	}

	// backup old balancing mode
	uint maxFullDetailChar = Scene->getMaxSkeletonsInNotCLodForm();
	UScene *skyScene = getSkyScene();
	UScene::TPolygonBalancingMode oldBalancingMode = Scene->getPolygonBalancingMode();
	UScene::TPolygonBalancingMode oldSkyBalancingMode = UScene::PolygonBalancingOff;
	if (skyScene)
	{
		oldSkyBalancingMode = skyScene->getPolygonBalancingMode();
	}
	// disable load balancing for that frame only if asked
	if (forceFullDetail)
	{
		Scene->setMaxSkeletonsInNotCLodForm(1000000);
		Scene->setPolygonBalancingMode(UScene::PolygonBalancingOff);
		if (skyScene)
		{
			skyScene->setPolygonBalancingMode(UScene::PolygonBalancingOff);
		}
	}

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

		if (Driver->getPolygonMode() == UDriver::Filled)
		{
			Driver->clearZBuffer();
		}

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
		// Sky is used to clear the frame buffer now, but if in line or point polygon mode, we should draw it
		if (Driver->getPolygonMode() != UDriver::Filled)
		{
			if (!Driver->isLost())
			{
				Driver->clearBuffers (CRGBA(127, 127, 127));
			}
		}
	}
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

	TSkyMode skyMode = NoSky;
	if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
	{
		if(Driver->getPolygonMode() == UDriver::Filled)
		{
			if (Filter3D[FilterSky])
			{
				CSky &sky = ContinentMngr.cur()->CurrentSky;
				if (sky.getScene())
				{
					skyMode = NewSky;
					sky.getScene()->animate(TimeInSec-FirstTimeInSec);
					// Setup the sky camera
					preRenderNewSky();
				}
				else
				{
					skyMode = OldSky;
				}
			}
		}
	}

	// initialisation of polygons renderer
	CLandscapePolyDrawer::getInstance().beginRenderLandscapePolyPart();

	// Start Part Rendering
	beginRenderCanopyPart();
	beginRenderMainScenePart();
	beginRenderSkyPart(skyMode);
	// Render part
	// WARNING: always must begin rendering with at least UScene::RenderOpaque,
	// else dynamic shadows won't work
	renderCanopyPart(UScene::RenderOpaque);
	renderMainScenePart(UScene::RenderOpaque);

	// render of polygons on landscape
	CLandscapePolyDrawer::getInstance().renderLandscapePolyPart();

	if (skyMode != NoSky) renderSkyPart((UScene::TRenderPart) (UScene::RenderOpaque | UScene::RenderTransparent), skyMode);
	renderCanopyPart((UScene::TRenderPart) (UScene::RenderTransparent | UScene::RenderFlare));
	renderMainScenePart((UScene::TRenderPart) (UScene::RenderTransparent | UScene::RenderFlare));
	if (skyMode == NewSky) renderSkyPart(UScene::RenderFlare, skyMode);
	// End Part Rendering
	endRenderSkyPart(skyMode);
	endRenderMainScenePart();
	endRenderCanopyPart();

	// reset depth range
	Driver->setDepthRange(0.f, CANOPY_DEPTH_RANGE_START);

	// restore load balancing mode
	if (forceFullDetail)
	{
		Scene->setMaxSkeletonsInNotCLodForm(maxFullDetailChar);
		Scene->setPolygonBalancingMode(oldBalancingMode);
		if (skyScene)
		{
			skyScene->setPolygonBalancingMode(oldSkyBalancingMode);
		}
	}

	// apply bloom effect
	if (ClientCfg.Bloom)
		CBloomEffect::getInstance().endBloom();
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

	CInterfaceGroup		*group= dynamic_cast<CInterfaceGroup*>(pIM->getElementFromId("ui:interface:free_trial_game_quitting"));
	if(group)
	{
		// if Free trial
		if(paying_account_request)
		{
			// if no current modal window, or if not the quit window
			if(group != pIM->getModalWindow())
			{
				// disable
				pIM->disableModalWindow();
				pIM->enableModalWindow(NULL, group);
			}
		}

		else
		{
			// if the current modal window is the quit window, disable
			if(group == pIM->getModalWindow())
			{
				// disable
				pIM->disableModalWindow();
			}
		}
	}

	group= dynamic_cast<CInterfaceGroup*>(pIM->getElementFromId("ui:interface:game_quitting"));
	if(group)
	{
		// if exit request
		if(game_exit_request && !paying_account_request)
		{
			// if no current modal window, or if not the quit window
			if(group != pIM->getModalWindow())
			{
				// disable
				pIM->disableModalWindow();
				pIM->enableModalWindow(NULL, group);

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
			if(group == pIM->getModalWindow())
			{
				// disable
				pIM->disableModalWindow();
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
			if (ig) im->setDefaultCaptureKeyboard(ig);
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
	GameContextMenu;
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

	CInterfaceManager::getInstance()->executeLuaScript("game:onMainLoopBegin()");


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
			// If an action handler execute. NB: MUST BE DONE BEFORE ANY THING ELSE PROFILE CRASH!!!!!!!!!!!!!!!!!
			testLaunchProfile();

			// Test and may run a VBLock profile (only once)
			testLaunchProfileVBLock();

			// Stop the Outgame music, with fade effect
			outgameFader.fade();

			// update quit feature
			updateGameQuitting();

			// Start Bench
			H_AUTO_USE ( RZ_Client_Main_Loop )


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
			CInterfaceManager::getInstance()->flushObserverCalls();
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
			CInterfaceManager::getInstance()->flushObserverCalls();
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
			if(!pIMinstance->getViewRenderer().isMinimized())
			{
				// Get the cursor instance
				CViewPointer *cursor = pIMinstance->getPointer();
				if(cursor)
				{
					// Get the pointer position (in pixel)
					sint32 x, y;
					cursor->getPointerPos(x, y);

					uint32 w, h;
					CViewRenderer &viewRender = pIMinstance->getViewRenderer();
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
			CInterfaceManager::getInstance()->flushObserverCalls();
			bool prevDatabaseInitStatus = IngameDbMngr.initInProgress();
			IngameDbMngr.setChangesProcessed();
			bool newDatabaseInitStatus = IngameDbMngr.initInProgress();
			if ((!newDatabaseInitStatus) && prevDatabaseInitStatus)
			{
				// When database received, activate allegiance buttons (for neutral state) in fame window
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CInterfaceGroup	*group = dynamic_cast<CInterfaceGroup*>(pIM->getElementFromId("ui:interface:fame:content:you"));
				if (group)
					group->updateAllLinks();
				// send a msg to lua for specific ui update
				pIM->executeLuaScript("game:onInGameDbInitialized()");
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
		CInterfaceManager::getInstance()->flushObserverCalls();

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
		MainCam.setPos(currViewPos);;
		MainCam.setRotQuat(View.currentView());


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


		//////////////////////////
		// RENDER THE FRAME  3D //
		//////////////////////////
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

		// Set the matrix in 3D Mode.
		Driver->setMatrixMode3D(MainCam);


		// R2ED pre render update
		if (ClientCfg.R2EDEnabled)
		{
			R2::getEditor().updateBeforeRender();
		}


		// Position the camera to prepare the render
		if (!ClientCfg.Light)
		{

			// Render
			if(Render)
			{
				#ifdef USE_WATER_ENV_MAP
				if (WaterEnvMapRefCount > 0) // water env map needed
				{
					if (!WaterEnvMap)
					{
						CSky &sky = ContinentMngr.cur()->CurrentSky;
						if (sky.getScene())
						{
							WaterEnvMapSkyCam = sky.getScene()->createCamera(); // deleted in unselect
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
				renderAll(ScreenshotRequest != ScreenshotRequestNone && ClientCfg.ScreenShotFullDetail); // nb : force full detail if a screenshot is asked

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
			}
			else
			{
				Driver->clearBuffers(ClientCfg.BGColor);
			}

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

		}
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

		// R2ED pre render update
		if (ClientCfg.R2EDEnabled)
		{
			// IMPORTANT : this should be called after CEntitiesMngr::updatePostRender() because
			// entity may be added / removed there !
			R2::getEditor().updateAfterRender();
		}

		// Update FXs (remove them).
		FXMngr.update();

		// Render the stat graphs if needed
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Debug )
			CGraph::render (ShowInfos);
		}

		// Render in 2D Mode to display 2D Interfaces and 2D texts.
		Driver->setMatrixMode2D11();

		// draw a big quad to represent thunder strokes
		if (Render && WeatherManager.getThunderLevel() != 0.f)
		{
			H_AUTO_USE ( RZ_Client_Main_Loop_Render_Thunder )
			Driver->drawQuad(0, 0, 1, 1, ThunderColor);

			// TODO : boris : add sound here !
		}

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
			if(!ClientCfg.Light && ClientCfg.Bloom)
				CBloomEffect::getInstance().endInterfacesDisplayBloom();
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
						CCDBNodeLeaf*pNL = pIMinstance->getDbProp("UI:VARIABLES:FPS");
						pNL->setValue64((sint64)(1.f/deltaTime));
					}
				}

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
			}
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
			if (dumpValidPolys)
			{
				struct CPolyDisp : public CInterfaceElementVisitor
				{
					virtual void visitCtrl(CCtrlBase *ctrl)
					{
						CCtrlPolygon *cp = dynamic_cast<CCtrlPolygon *>(ctrl);
						if (cp)
						{
							sint32 cornerX, cornerY;
							cp->getParent()->getCorner(cornerX, cornerY, cp->getParentPosRef());
							for(sint32 y = 0; y < (sint32) Screen.getHeight(); ++y)
							{
								for(sint32 x = 0; x < (sint32) Screen.getWidth(); ++x)
								{
									if (cp->contains(CVector2f((float) (x - cornerX), (float) (y - cornerY))))
									{
										((CRGBA *) &Screen.getPixels()[0])[x + (Screen.getHeight() - 1 - y) * Screen.getWidth()] = CRGBA::Magenta;
									}
								}
							}
						}
					}
					CBitmap Screen;
				} polyDisp;
				Driver->getBuffer(polyDisp.Screen);
				CInterfaceManager::getInstance()->visit(&polyDisp);
				COFile output("poly.tga");
				polyDisp.Screen.writeTGA(output);
				dumpValidPolys = false;
			};

			// TMP TMP
			static volatile bool dumpColPolys = false;
			if (dumpColPolys)
			{
				CPackedWorld *pw = R2::getEditor().getIslandCollision().getPackedIsland();
				if (pw)
				{
					static CMaterial material;
					static CMaterial wiredMaterial;
					static CMaterial texturedMaterial;
					static CVertexBuffer vb;
					static bool initDone = false;
					if (!initDone)
					{
						vb.setVertexFormat(CVertexBuffer::PositionFlag);
						vb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
						material.initUnlit();
						material.setDoubleSided(true);
						material.setZFunc(CMaterial::lessequal);
						wiredMaterial.initUnlit();
						wiredMaterial.setDoubleSided(true);
						wiredMaterial.setZFunc(CMaterial::lessequal);
						wiredMaterial.setColor(CRGBA(255, 255, 255, 250));
						wiredMaterial.texEnvOpAlpha(0, CMaterial::Replace);
						wiredMaterial.texEnvArg0Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
						wiredMaterial.setBlend(true);
						wiredMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
						texturedMaterial.initUnlit();
						texturedMaterial.setDoubleSided(true);
						texturedMaterial.setZFunc(CMaterial::lessequal);
						initDone = true;
					}
					// just add a projected texture
					R2::getEditor().getIslandCollision().loadEntryPoints();
					R2::CScenarioEntryPoints &sep = R2::CScenarioEntryPoints::getInstance();
					CVectorD playerPos = UserEntity->pos();
					R2::CScenarioEntryPoints::CCompleteIsland *island = sep.getCompleteIslandFromCoords(CVector2f((float) playerPos.x, (float) playerPos.y));
					static CSString currIsland;
					if (island && island->Island != currIsland)
					{
						currIsland = island->Island;
						CTextureFile *newTex = new CTextureFile(currIsland + "_sp.tga");
						newTex->setWrapS(ITexture::Clamp);
						newTex->setWrapT(ITexture::Clamp);
						texturedMaterial.setTexture(0, newTex);
						texturedMaterial.texEnvOpRGB(0, CMaterial::Replace);
						texturedMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
						texturedMaterial.setTexCoordGen(0, true);
						texturedMaterial.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);
						CMatrix mat;
						CVector scale((float) (island->XMax -  island->XMin),
									  (float) (island->YMax -  island->YMin), 0.f);
						scale.x = 1.f / favoid0(scale.x);
						scale.y = 1.f / favoid0(scale.y);
						scale.z = 0.f;
						mat.setScale(scale);
						mat.setPos(CVector(- island->XMin * scale.x, - island->YMin * scale.y, 0.f));
						//
						CMatrix uvScaleMat;
						//
						uint texWidth = (uint) (island->XMax -  island->XMin);
						uint texHeight = (uint) (island->YMax -  island->YMin);
						float UScale = (float) texWidth / 	raiseToNextPowerOf2(texWidth);
						float VScale = (float) texHeight / raiseToNextPowerOf2(texHeight);
						//
						uvScaleMat.setScale(CVector(UScale, - VScale, 0.f));
						uvScaleMat.setPos(CVector(0.f, VScale, 0.f));
						//
						texturedMaterial.enableUserTexMat(0, true);
						texturedMaterial.setUserTexMat(0, uvScaleMat * mat);
					}
					const CFrustum &frust = MainCam.getFrustum();

					//
					IDriver *driver = ((CDriverUser  *) Driver)->getDriver();

					driver->enableFog(true);
					const CRGBA clearColor = CRGBA(0, 0, 127, 0);
					driver->setupFog(frust.Far * 0.8f, frust.Far, clearColor);
					CViewport vp;
					vp.init(0.f, 0.f, 1.f, 1.f);
					driver->setupViewport(vp);
					CScissor scissor;
					viewportToScissor(vp, scissor);
					driver->setupScissor(scissor);
					//
					driver->setFrustum(frust.Left, frust.Right, frust.Bottom, frust.Top, frust.Near, frust.Far, frust.Perspective);
					driver->setupViewMatrix(MainCam.getMatrix().inverted());
					driver->setupModelMatrix(CMatrix::Identity);
					//
					//
					const CVector localFrustCorners[8] =
					{
						CVector(frust.Left, frust.Near, frust.Top),
						CVector(frust.Right, frust.Near, frust.Top),
						CVector(frust.Right, frust.Near, frust.Bottom),
						CVector(frust.Left, frust.Near, frust.Bottom),
						CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
						CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Top * frust.Far / frust.Near),
						CVector(frust.Right * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near),
						CVector(frust.Left  * frust.Far / frust.Near, frust.Far, frust.Bottom * frust.Far / frust.Near)
					};
					// roughly compute covered zones
					//
					/*
					sint frustZoneMinX = INT_MAX;
					sint frustZoneMaxX = INT_MIN;
					sint frustZoneMinY = INT_MAX;
					sint frustZoneMaxY = INT_MIN;
					for(uint k = 0; k < sizeofarray(localFrustCorners); ++k)
					{
						CVector corner = camMat * localFrustCorners[k];
						sint zoneX = (sint) (corner.x / 160.f) - zoneMinX;
						sint zoneY = (sint) floorf(corner.y / 160.f) - zoneMinY;
						frustZoneMinX = std::min(frustZoneMinX, zoneX);
						frustZoneMinY = std::min(frustZoneMinY, zoneY);
						frustZoneMaxX = std::max(frustZoneMaxX, zoneX);
						frustZoneMaxY = std::max(frustZoneMaxY, zoneY);
					}
					*/

					const uint TRI_BATCH_SIZE = 10000; // batch size for rendering
					static std::vector<TPackedZoneBaseSPtr> zones;
					zones.clear();
					pw->getZones(zones);
					for(uint k = 0; k < zones.size(); ++k)
					{
						zones[k]->render(vb, *driver, texturedMaterial, wiredMaterial, MainCam.getMatrix(), TRI_BATCH_SIZE, localFrustCorners);
					}
				}
			}

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
			CInterfaceProperty prop;
			prop.readDouble("UI:VARIABLES:DIRECTION"," ");
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

		// Enter a network loop during the FarTP process, without doing the whole real main loop.
		// This code must remain at the very end of the main loop.
		if(LoginSM.getCurrentState() == CLoginStateMachine::st_enter_far_tp_main_loop)
		{
			CInterfaceManager::getInstance()->executeLuaScript("game:onFarTpStart()");
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

			CInterfaceManager::getInstance()->executeLuaScript("game:onFarTpEnd()");
		}

	} // end of main loop

	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (CLuaManager::getInstance().getLuaState())
	{
		CInterfaceManager::getInstance()->executeLuaScript("game:onMainLoopEnd()");
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

		CInterfaceManager::getInstance()->setDefaultCaptureKeyboard(NULL);

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
// displayDebug :
// Display some debug infos.
//---------------------------------------------------
void displayDebugFps()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.DebugFontColor);

	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;
	// Ms per frame
	{
		float spf = smoothFPS.getSmoothValue ();
		// Ms per frame
		TextContext->printfAt(0.1f, line, "FPS %.1f ms - %.1f fps", spf*1000, 1.f/spf);
		line-= lineStep;
		// More Smoothed Ms per frame
		spf = moreSmoothFPS.getSmoothValue ();
		TextContext->printfAt(0.1f, line, "Smoothed FPS %.1f ms - %.1f fps", spf*1000, 1.f/spf);
		line-= lineStep;
	}
}

static NLMISC::CRefPtr<CInterfaceElement> HighlightedDebugUI;

// displayDebug :
// Display information about ui elements that are under the mouse
//---------------------------------------------------
void displayDebugUIUnderMouse()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);



	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// for now only accessible with R2ED
	if (ClientCfg.R2EDEnabled)
	{
		TextContext->setColor(CRGBA::Cyan);
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+A) to cycle prev");
		line-= lineStep;
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+Q) to cycle next");
		line-= lineStep;
		TextContext->printfAt(0.1f, line, "Press default key (ctrl+shift+W) to inspect element");
		line-= 2 * lineStep;
	}
	//
	const vector<CCtrlBase *> &rICL = pIM->getCtrlsUnderPointer ();
	const vector<CInterfaceGroup *> &rIGL = pIM->getGroupsUnderPointer ();
	// If previous highlighted element is found in the list, then keep it, else reset to first element
	if (std::find(rICL.begin(), rICL.end(), HighlightedDebugUI) == rICL.end() &&
		std::find(rIGL.begin(), rIGL.end(), HighlightedDebugUI) == rIGL.end())
	{
		if (!rICL.empty())
		{
			HighlightedDebugUI = rICL[0];
		}
		else
		if (!rIGL.empty())
		{
			HighlightedDebugUI = rIGL[0];
		}
		else
		{
			HighlightedDebugUI = NULL;
		}
	}
	//
	TextContext->setColor(CRGBA::Green);
	TextContext->printfAt(0.1f, line, "Controls under cursor ");
	line -= lineStep * 1.4f;
	TextContext->printfAt(0.1f, line, "----------------------");
	line -= lineStep;
	for(uint k = 0; k < rICL.size(); ++k)
	{
		if (rICL[k])
		{
			TextContext->setColor(rICL[k] != HighlightedDebugUI ? ClientCfg.DebugFontColor : CRGBA::Red);
			TextContext->printfAt(0.1f, line, "id = %s, address = 0x%p, parent = 0x%p", rICL[k]->getId().c_str(), rICL[k], rICL[k]->getParent());
		}
		else
		{
			TextContext->setColor(CRGBA::Blue);
			TextContext->printfAt(0.1f, line, "<NULL> control found !!!");
		}
		line-= lineStep;
	}
	//
	TextContext->setColor(CRGBA::Green);
	TextContext->printfAt(0.1f, line, "Groups under cursor ");
	line -= lineStep * 1.4f;
	TextContext->printfAt(0.1f, line, "----------------------");
	line-= lineStep;
	for(uint k = 0; k < rIGL.size(); ++k)
	{
		if (rIGL[k])
		{
			TextContext->setColor(rIGL[k] != HighlightedDebugUI ? ClientCfg.DebugFontColor : CRGBA::Red);
			TextContext->printfAt(0.1f, line, "id = %s, address = 0x%p, parent = 0x%p", rIGL[k]->getId().c_str(), rIGL[k], rIGL[k]->getParent());
		}
		else
		{
			TextContext->setColor(CRGBA::Blue);
			TextContext->printfAt(0.1f, line, "<NULL> group found !!!");
		}
		line-= lineStep;
	}
}



// get all element under the mouse in a single vector
static void getElementsUnderMouse(vector<CInterfaceElement *> &ielem)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	const vector<CCtrlBase *> &rICL = pIM->getCtrlsUnderPointer();
	const vector<CInterfaceGroup *> &rIGL = pIM->getGroupsUnderPointer();
	ielem.clear();
	ielem.insert(ielem.end(), rICL.begin(), rICL.end());
	ielem.insert(ielem.end(), rIGL.begin(), rIGL.end());
}

class CHandlerDebugUiPrevElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		vector<CInterfaceElement *> ielem;
		getElementsUnderMouse(ielem);
		for(uint k = 0; k < ielem.size(); ++k)
		{
			if (HighlightedDebugUI == ielem[k])
			{
				HighlightedDebugUI = ielem[k == 0 ? ielem.size() - 1 : k - 1];
				return;
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiPrevElementUnderMouse, "debug_ui_prev_element_under_mouse");

class CHandlerDebugUiNextElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		vector<CInterfaceElement *> ielem;
		getElementsUnderMouse(ielem);
		for(uint k = 0; k < ielem.size(); ++k)
		{
			if (HighlightedDebugUI == ielem[k])
			{
				HighlightedDebugUI = ielem[(k + 1) % ielem.size()];
				return;
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiNextElementUnderMouse, "debug_ui_next_element_under_mouse");

class CHandlerDebugUiDumpElementUnderMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (HighlightedDebugUI == NULL) return;
		CLuaState *lua = CLuaManager::getInstance().getLuaState();
		if (!lua) return;
		CLuaStackRestorer lsr(lua, 0);
		CLuaIHMRyzom::pushUIOnStack(*lua, HighlightedDebugUI);
		lua->pushValue(LUA_GLOBALSINDEX);
		CLuaObject env(*lua);
		env["inspect"].callNoThrow(1, 0);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDebugUiDumpElementUnderMouse, "debug_ui_inspect_element_under_mouse");


//---------------------------------------------------
// displayDebug :
// Display some debug infos.
//---------------------------------------------------
void displayDebug()
{
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.DebugFontColor);

	// TOP LEFT //
	//----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 0.9f;
	// FPS and Ms per frame
	{
		// smooth across frames.
		double deltaTime = smoothFPS.getSmoothValue ();
		// FPS and Ms per frame
		if(deltaTime != 0.f)
			TextContext->printfAt(0.f, line,"%.1f fps", 1.f/deltaTime);
		else
			TextContext->printfAt(0.f, line,"%.1f fps", 0.f);
		TextContext->printfAt(0.1f, line, "%d ms", (uint)(deltaTime*1000));
	}
	line -= lineStep;
	line -= lineStep;

	// USER
	// Front
	TextContext->printfAt(0.0f, line, "  %f (%f,%f,%f) front", atan2(UserEntity->front().y, UserEntity->front().x), UserEntity->front().x, UserEntity->front().y, UserEntity->front().z);
	line -= lineStep;
	// Dir
	TextContext->printfAt(0.0f, line, "  %f (%f,%f,%f) dir", atan2(UserEntity->dir().y, UserEntity->dir().x), UserEntity->dir().x, UserEntity->dir().y, UserEntity->dir().z);
	line -= lineStep;
	// NB Stage
	TextContext->printfAt(0.0f, line, "  NB Stage: %d", UserEntity->nbStage());
	line -= lineStep;
	// NB Animation FXs still remaining in the remove list.
	TextContext->printfAt(0.0f, line, "  NB FXs to remove: %d", UserEntity->nbAnimFXToRemove());
	line -= lineStep;
	// Mode.
	TextContext->printfAt(0.0f, line, "  Mode: %d (%s)", (sint)UserEntity->mode(), MBEHAV::modeToString(UserEntity->mode()).c_str());
	line -= lineStep;
	// Behaviour.
	TextContext->printfAt(0.0f, line, "  Behaviour: %d (%s)", (sint)UserEntity->behaviour(), MBEHAV::behaviourToString(UserEntity->behaviour()).c_str());
	line -= lineStep;
	// Display the target mount.
	TextContext->printfAt(0.0f, line, "  Mount: %d", UserEntity->mount());
	line -= lineStep;
	// Display the target rider.
	TextContext->printfAt(0.0f, line, "  Rider: %d", UserEntity->rider());
	line -= lineStep;
	// Display the current animation name.
	TextContext->printfAt(0.0f, line, "  Current Animation Name: %s", UserEntity->currentAnimationName().c_str());
	line -= lineStep;
	// Display the current move animation set name.
	TextContext->printfAt(0.0f, line, "  Current AnimationSet Name (MOVE): %s", UserEntity->currentAnimationSetName(MOVE).c_str());
	line -= lineStep;
	// Display Missing Animations
	if(::CAnimation::MissingAnim.empty() == false)
	{
		TextContext->printfAt(0.0f, line, "  '%u' Missing Animations, 1st: '%s'", ::CAnimation::MissingAnim.size(), (*(::CAnimation::MissingAnim.begin())).c_str());
		line -= lineStep;
	}
	// Display Missing LoD
	if(LodCharactersNotFound.empty() == false)
	{
		TextContext->printfAt(0.0f, line, "  '%u' Missing LoD, 1st: '%s'", LodCharactersNotFound.size(), (*(LodCharactersNotFound.begin())).c_str());
		line -= lineStep;
	}

	// Watched Entity
	line -= lineStep;
	// Now Displaying the selection.
	TextContext->printfAt(0.0f, line, "--*** Watched entity ***--");
	line -= lineStep;
	// Display information about the debug entity slot.
	if(WatchedEntitySlot != CLFECOMMON::INVALID_SLOT)
	{
		// Get a pointer on the target.
		CEntityCL *watchedEntity = EntitiesMngr.entity(WatchedEntitySlot);
		if(watchedEntity)
		{
			// Display Debug Information about the Selection.
			watchedEntity->displayDebug(0.0f, line, -lineStep);

			// Distance of the target
			CVectorD diffvector = UserEntity->pos() - watchedEntity->pos();
			TextContext->printfAt(0.0f, line, "  Distance: %10.2f (Manhattan: %.2f)", diffvector.norm(), fabs(diffvector.x) + fabs(diffvector.y) );
			line -= lineStep;
		}
		// Target not allocated
		else
		{
			TextContext->printfAt(0.0f, line, "Not allocated (%d)", WatchedEntitySlot);
			line -= lineStep;
		}
	}
	// No Target
	else
	{
		TextContext->printfAt(0.0f, line, "None");
		line -= lineStep;
	}

	/* Ca rame grave !

	  uint nMem = NLMEMORY::GetAllocatedMemory();
	line -= lineStep;
	TextContext->printfAt(0.0f, line, "Mem Used: %d",nMem);*/

	// 3D Filters information:
#ifdef _PROFILE_ON_
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "3D Filters:");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "MeshNoVP: %s", Filter3D[FilterMeshNoVP]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "MeshVP: %s", Filter3D[FilterMeshVP]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "FXs: %s", Filter3D[FilterFXs]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	if (Landscape)
	{
		TextContext->printfAt(0.0f, line, "Landscape: %s", Filter3D[FilterLandscape]?"Ok":"NOT RENDERED!");
		line-= lineStep;
	}
	else
	{
		TextContext->printfAt(0.0f, line, "Landscape not enabled");
	}
	TextContext->printfAt(0.0f, line, "Vegetable: %s", Filter3D[FilterVegetable]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Skeleton: %s", Filter3D[FilterSkeleton]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Water: %s", Filter3D[FilterWater]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Cloud: %s", Filter3D[FilterCloud]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "CoarseMesh: %s", Filter3D[FilterCoarseMesh]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Sky: %s", Filter3D[FilterSky]?"Ok":"NOT RENDERED!");
	line-= lineStep;
	// Materials Infos
	TextContext->printfAt(0.0f, line, "SetupedMatrix: %d", Driver->profileSetupedModelMatrix() );
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "SetupedMaterials: %d", Driver->profileSetupedMaterials() );
	line-= lineStep;
	// Display camera cluster system
	TextContext->printfAt(0.0f, line, "ClusterSystem: %p", MainCam.getClusterSystem() );
	line-= 2 * lineStep;
	// Lua stuffs
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	TextContext->printfAt(0.0f, line, "Lua mem (kb) : %d / %d", CLuaManager::getInstance().getLuaState()->getGCCount(),  CLuaManager::getInstance().getLuaState()->getGCThreshold());
	line-= lineStep;
	TextContext->printfAt(0.0f, line, "Lua stack size = %d", CLuaManager::getInstance().getLuaState()->getTop());
	line-= lineStep;

#endif

	// TOP LEFT //
	//-----------//
	TextContext->setHotSpot(UTextContext::TopLeft);
	line = 1.f;
	string str;
#if FINAL_VERSION
	str = "FV";
#else
	str = "DEV";
#endif
	if(ClientCfg.ExtendedCommands)
		str += "_E";
	str += " "RYZOM_VERSION;
	TextContext->printfAt(0.f, line, "Version %s", str.c_str());

	// TOP MIDDLE //
	//------------//
	TextContext->setHotSpot(UTextContext::MiddleTop);
	line = 1.f;
	// Motion Mode
	TextContext->printfAt(0.5f, line, "%s", UserControls.modeStr().c_str());
	line -= lineStep;

	// TOP RIGHT //
	//-----------//
	TextContext->setHotSpot(UTextContext::TopRight);
	line = 1.f;
	//// 3D Infos
	// Video mem allocated.
	TextContext->printfAt(1.f, line, "Video mem. : %f", Driver->profileAllocatedTextureMemory()/(1024.f*1024.f));
	line -= lineStep;
	// Video mem used since last swapBuffers().
	TextContext->printfAt(1.f, line, "Video mem. since last swap buffer: %f", Driver->getUsedTextureMemory()/(1024.f*1024.f));
	line -= lineStep;
	// Get the last face count asked from the main scene before reduction.
	TextContext->printfAt(1.f, line, "Nb Skin Face Asked: %f", Scene->getGroupNbFaceAsked("Skin"));
	line -= lineStep;
	TextContext->printfAt(1.f, line, "Nb Fx Face Asked: %f", Scene->getGroupNbFaceAsked("Fx"));
	line -= lineStep;
	// All Triangles In
	CPrimitiveProfile pIn;
	CPrimitiveProfile pOut;
	Driver->profileRenderedPrimitives(pIn, pOut);
	TextContext->printfAt(1.f, line, "Tri In : %d", pIn.NTriangles+2*pIn.NQuads);
	line -= lineStep;
	// All Triangles Out
	TextContext->printfAt(1.f, line, "Tri Out : %d", pOut.NTriangles+2*pIn.NQuads);
	line -= lineStep;
	// Current Cluster
	string strPos;
	// Check there is a PACS Primitive before using it.
	if(UserEntity->getPrimitive() && GR)
	{
		UGlobalPosition gPos;
		UserEntity->getPrimitive()->getGlobalPosition(gPos, dynamicWI);
		string strPos = GR->getIdentifier(gPos);
	}
	else
		strPos = "No Primitive";
	TextContext->printfAt(1.f, line, "Cluster : %s", strPos.c_str());
	line -= lineStep;
	//// SOUND Infos
	line -= lineStep;
	if(SoundMngr)
	{
		TextContext->printfAt(1.f, line, "Sound source instance: %u", SoundMngr->getSourcesInstanceCount());
		line -= lineStep;
		TextContext->printfAt(1.f, line, "Logical playing SoundSource: %u", SoundMngr->getMixer()->getPlayingSourcesCount ());
		line -= lineStep;
		TextContext->printfAt(1.f, line, "Audio tracks: %u/%u", SoundMngr->getMixer()->getUsedTracksCount(), SoundMngr->getMixer()->getPolyphony());
		line -= lineStep;
		if (SoundMngr->getMixer()->getMutedPlayingSourcesCount() > 0)
		{
			TextContext->printfAt(1.f, line, "Source muted: %u !", SoundMngr->getMixer()->getMutedPlayingSourcesCount());
			line -= lineStep;
		}
		TextContext->printfAt(1.f, line, "Samples in memory: %g MB", SoundMngr->getLoadingSamplesSize() / (1024.0f*1024.0f));
		line -= lineStep;

	}

	// BOTTOM RIGHT //
	//--------------//
	TextContext->setHotSpot(UTextContext::BottomRight);
	line = 0.f;
	//// POSITION
	CVector postmp = View.viewPos();
	// Pos
	TextContext->printfAt(1.f, line, "Position : %d %d %d",(int)postmp.x,(int)postmp.y,(int)postmp.z);
	line += lineStep;
	// Body Heading
	TextContext->printfAt(1.f, line, "Front : %.2f %.2f %.2f", UserEntity->front().x, UserEntity->front().y, UserEntity->front().z);
	line += lineStep;
	// Speed
	TextContext->printfAt(1.f, line, "Speed : %.2f", (float) UserEntity->speed());
	line += lineStep;
	// Zone
	if (!ClientCfg.Light)
	{
		if (Landscape)
		{
			TextContext->printfAt(1.f, line, "Zone: %s", Landscape->getZoneName(postmp).c_str());
			line += lineStep;
		}
	}
	// Prim File
	string primFile = PrimFiles.getCurrentPrimitive ();
	if (!primFile.empty ())
	{
		TextContext->printfAt(1.f, line, "Prim File: %s", primFile.c_str ());
		line += lineStep;
	}

	//// CONNECTION
	line += lineStep;
	// Ryzom Day.
	TextContext->printfAt(1.f, line, "Ryzom Day : %d", RT.getRyzomDay());
	line += lineStep;
	// hour in the game
	float dayNightCycleHour = (float)RT.getRyzomTime();
	TextContext->printfAt(1.f, line, "Ryzom Time : %2u:%02u", int(dayNightCycleHour), int((dayNightCycleHour-int(dayNightCycleHour))*60.0f));
	line += lineStep;
	// light hour in the game, used to display te day/night
	TextContext->printfAt(1.f, line, "Ryzom Light Time : %2u:%02u (%s)", int(DayNightCycleHour), int((DayNightCycleHour-int(DayNightCycleHour))*60.0f), LightCycleManager.getStateString().c_str());
	line += lineStep;
	// Server GameCycle
	TextContext->printfAt(1.f, line, "Server GameCycle : %u", (uint)NetMngr.getCurrentServerTick());
	line += lineStep;
	// Current GameCycle
	TextContext->printfAt(1.f, line, "Current GameCycle : %u", (uint)NetMngr.getCurrentClientTick());
	line += lineStep;
	// Current GameCycle
	TextContext->printfAt(1.f, line, "Ms per Cycle : %d", NetMngr.getMsPerTick());
	line += lineStep;
	// Packet Loss
	TextContext->printfAt(1.f, line, "Packet Loss : %.1f %%", NetMngr.getMeanPacketLoss()*100.0f);
	line += lineStep;
	// Packet Loss
	TextContext->printfAt(1.f, line, "Packets Lost : %u", NetMngr.getTotalLostPackets());
	line += lineStep;
	// Mean Upload
	TextContext->printfAt(1.f, line, "Mean Upld : %.3f kbps", NetMngr.getMeanUpload());
	line += lineStep;
	// Mean Download
	TextContext->printfAt(1.f, line, "Mean Dnld : %.3f kbps", NetMngr.getMeanDownload());
	line += lineStep;

	// Mean Download
	TextContext->printfAt(1.f, line, "Nb in Vision : %d(%d,%d,%d)",
		EntitiesMngr.nbEntitiesAllocated(),
		EntitiesMngr.nbUser(),
		EntitiesMngr.nbPlayer(),
		EntitiesMngr.nbChar());
	line += lineStep;

	// Number of database changes
	TextContext->printfAt(1.f, line, "DB Changes : %u", NbDatabaseChanges );
	line += lineStep;

	// Ping
	TextContext->printfAt(1.f, line, "DB Ping : %u ms", Ping.getValue());
	line += lineStep;





	// Manual weather setup
	{
		if(ContinentMngr.cur())	// Only usable if there is a continent loaded.
		{
			if (!ForceTrueWeatherValue)
			{
				const CWeatherFunction &wf = ContinentMngr.cur()->WeatherFunction[CurrSeason];
				float wv;
				if (ClientCfg.ManualWeatherSetup)
				{
					wv = std::max(wf.getNumWeatherSetups() - 1, 0u) * ManualWeatherValue;
				}
				else
				{
					wv = std::max(wf.getNumWeatherSetups() - 1, 0u) * ::getBlendedWeather(RT.getRyzomDay(), RT.getRyzomTime(), *WeatherFunctionParams, ContinentMngr.cur()->WeatherFunction);
				}
				const CWeatherSetup *ws = wf.getWeatherSetup((uint) floorf(wv));
				std::string name0 = ws ? NLMISC::CStringMapper::unmap(ws->SetupName) : "???";
				ws = wf.getWeatherSetup(std::min((uint) (floorf(wv) + 1), wf.getNumWeatherSetups() - 1));
				std::string name1 = ws ? NLMISC::CStringMapper::unmap(ws->SetupName) : "???";
				TextContext->printfAt(1.f, line, "Weather value : %.02f : %s -> %s", ws ? wv : 0.f, name0.c_str(), name1.c_str());
				line += lineStep;
			}
			else
			{
				TextContext->printfAt(1.f, line, "Weather value : %.02f", WeatherManager.getWeatherValue() * std::max(ContinentMngr.cur()->WeatherFunction[CurrSeason].getNumWeatherSetups() - 1, 0u));
				line += lineStep;
				TextContext->printfAt(1.f, line, "TEST WEATHER FUNCTION");
				line += lineStep;
			}
			// season
			TextContext->printfAt(1.f, line, "Season : %s", EGSPD::CSeason::toString(CurrSeason).c_str());
			line += lineStep;
		}
	}

	// fog dist
	if (ContinentMngr.cur())
	{
		TextContext->printfAt(1.f, line, "Continent fog min near = %.1f, max far = %.1f", ContinentMngr.cur()->FogStart, ContinentMngr.cur()->FogEnd);
		line += lineStep;
		CFogState tmpFog;
		ContinentMngr.getFogState(MainFog, LightCycleManager.getLightLevel(), LightCycleManager.getLightDesc().DuskRatio, LightCycleManager.getState(), View.viewPos(), tmpFog);
		TextContext->printfAt(1.f, line, "Continent fog curr near = %.1f, curr far = %.1f", tmpFog.FogStartDist, tmpFog.FogEndDist);
		line += lineStep;
	}
	const CWeatherState &ws = WeatherManager.getCurrWeatherState();
	TextContext->printfAt(1.f, line, "Weather fog near = %.1f, far = %.1f", ws.FogNear[MainFog], ws.FogFar[MainFog]);
	line += lineStep;
	TextContext->printfAt(1.f, line, "Final fog near = %.1f, far = %.1f", MainFogState.FogStartDist, MainFogState.FogEndDist);
	line += lineStep;
	float left, right, bottom, top, znear, zfar;
	Scene->getCam().getFrustum(left, right, bottom, top, znear, zfar);
	TextContext->printfAt(1.f, line, "Clip near = %.1f, far = %.1f", znear, zfar);
	line += lineStep;

	// Connection states
	TextContext->printfAt(1.f, line, "State : %s", NetMngr.getConnectionStateCStr() );
	line += lineStep;

//	UGlobalPosition globalPos;
//	UserEntity->getPrimitive()->getGlobalPosition(globalPos, dynamicWI);
//	uint32 material = GR->getMaterial( globalPos );
//	TextContext->printfAt(0.5f,0.5f,"Material : %d Gpos=(inst=%d,surf=%d,x=%.2f,y=%.2f",material, globalPos.InstanceId, globalPos.LocalPosition.Surface, globalPos.LocalPosition.Estimation.x, globalPos.LocalPosition.Estimation.y);

	// No more shadow when displaying a text.
	TextContext->setShaded(false);
}// displayDebug //

//-----------------------------------------------
// Macro to Display a Text
//-----------------------------------------------
#define DISP_TEXT(x, text)                    \
	/* Display the text at the right place */ \
	TextContext->printfAt(x, line, text);     \
	/* Change the line */                     \
	line += lineStep;                         \

//---------------------------------------------------
// displayHelp :
// Display an Help.
//---------------------------------------------------
void displayHelp()
{
	float line     = 1.f;
	float lineStep = -ClientCfg.HelpLineStep;

	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.HelpFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.HelpFontColor);


	line = 1.f;
	TextContext->setHotSpot(UTextContext::TopLeft);
	DISP_TEXT(0.0f, "SHIFT + F1 : This Menu")
	DISP_TEXT(0.0f, "SHIFT + F2 : Display Debug Infos")
	DISP_TEXT(0.0f, "SHIFT + F3 : Wire mode");
	DISP_TEXT(0.0f, "SHIFT + F4 : Do not Render the Scene");
	DISP_TEXT(0.0f, "SHIFT + F5 : Toogle Display OSD interfaces");
//	DISP_TEXT(0.0f, "SHIFT + F6 : Not used");
	DISP_TEXT(0.0f, "SHIFT + F7 : Compass Mode (User/Camera)");
	DISP_TEXT(0.0f, "SHIFT + F8 : Camera Mode (INSERT to change your position)");
	DISP_TEXT(0.0f, "SHIFT + F9 : Free Mouse");
	DISP_TEXT(0.0f, "SHIFT + F10 : Take a Screen Shot (+CTRL) for jpg");
//	DISP_TEXT(0.0f, "SHIFT + F11 : Test");
	DISP_TEXT(0.0f, "SHIFT + ESCAPE : Quit");
	DISP_TEXT(0.0f, "SHIFT + C : First/Third Person View");

	line = 1.f;
	TextContext->setHotSpot(UTextContext::TopRight);
	DISP_TEXT(1.0f, "UP : FORWARD");
	DISP_TEXT(1.0f, "DOWN : BACKWARD");
	DISP_TEXT(1.0f, "LEFT : ROTATE LEFT");
	DISP_TEXT(1.0f, "RIGHT : ROTATE RIGHT");
	DISP_TEXT(1.0f, "CTRL + LEFT : STRAFE LEFT");
	DISP_TEXT(1.0f, "CTRL + RIGHT : STRAFE RIGHT");
	DISP_TEXT(1.0f, "END : Auto Walk");
	DISP_TEXT(1.0f, "DELETE : Walk/Run");
	DISP_TEXT(1.0f, "PG UP : Look Up");
	DISP_TEXT(1.0f, "PG DOWN : Look Down");
//	DISP_TEXT(1.0f, "CTRL + I : Inventory");
//	DISP_TEXT(1.0f, "CTRL + C : Spells composition interface");
//	DISP_TEXT(1.0f, "CTRL + S : Memorized Spells interface");
	DISP_TEXT(1.0f, "CTRL + B : Show/Hide PACS Borders");
	DISP_TEXT(1.0f, "CTRL + P : Player target himself");
	DISP_TEXT(1.0f, "CTRL + D : Unselect target");
	DISP_TEXT(1.0f, "CTRL + TAB : Next Chat Mode (say/shout");
	DISP_TEXT(1.0f, "CTRL + R : Reload Client.cfg File");
//	DISP_TEXT(1.0f, "CTRL + N : Toggle Night / Day lighting");
	DISP_TEXT(1.0f, "CTRL + F2 : Profile on / off");
	DISP_TEXT(1.0f, "CTRL + F3 : Movie Shooter record / stop");
	DISP_TEXT(1.0f, "CTRL + F4 : Movie Shooter replay");
	DISP_TEXT(1.0f, "CTRL + F5 : Movie Shooter save");
#ifndef NL_USE_DEFAULT_MEMORY_MANAGER
	DISP_TEXT(1.0f, "CTRL + F6 : Save memory stat report");
#endif // NL_USE_DEFAULT_MEMORY_MANAGER
	DISP_TEXT(1.0f, "CTRL + F7 : Show / hide prim file");
	DISP_TEXT(1.0f, "CTRL + F8 : Change prim file UP");
	DISP_TEXT(1.0f, "CTRL + F9 : Change prim file DOWN");

	// No more shadow when displaying a text.
	TextContext->setShaded(false);
}// displayHelp //


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
	DebugUICell = on;
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
	DebugUICell = !DebugUICell;
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
	im->executeLuaScript("bgdownloader:inGamePatchUncompleteWarning()");
	/*
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(im->getElementFromId("ui:interface:bg_downloader"));
	if (gc)
	{
		gc->setActive(true);
		im->setTopWindow(gc);
		gc->enableBlink(2);
	}
	im->messageBoxWithHelp(CI18N::get("uiBGD_InGamePatchIncomplete"));
	im->displaySystemInfo(CI18N::get("uiBGD_InGamePatchIncompleteBC"), "BC");
	*/
}
