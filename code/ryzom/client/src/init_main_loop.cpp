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

/////////////
// INCLUDE //
/////////////

#include <curl/curl.h>

// Misc.
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/big_file.h"
// 3D Interface.
#include "nel/3d/bloom_effect.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_landscape.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_cloud_scape.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/3d/u_water_env_map.h"
// Sound
#include "nel/sound/u_audio_mixer.h"
// Client
#include "init_main_loop.h"
#include "input.h"
#include "client_cfg.h"
#include "entities.h"
#include "entity_animation_manager.h"
#include "actions_client.h"

#include "interface_v3/interface_manager.h"
#include "interface_v3/people_interraction.h"

#include "nel/gui/view_bitmap.h"

#include "nel/gui/interface_link.h"
#include "cursor_functions.h"
#include "pacs_client.h"
#include "ig_client.h"
#include "light_cycle_manager.h"
#include "weather_manager_client.h"
#include "weather.h"
#include "nel/3d/u_cloud_scape.h"
#include "view.h"
#include "time_client.h"
#include "connection.h"
#include "sheet_manager.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "continent.h"
#include "sky_render.h"
#include "nel/gui/group_editbox.h"
#include "interface_v3/inventory_manager.h"
#include "interface_v3/bot_chat_page_all.h"
#include "main_loop.h"

#include "net_manager.h"
#include "ig_callback.h"
#include "lod_character_user_manager.h"
//#include "color_slot_manager.h"
#include "teleport.h"
#include "movie_shooter.h"
#include "interface_v3/input_handler_manager.h"

#include "time_client.h"
#include "auto_anim.h"
#include "release.h"
#include "ig_season_callback.h"
#include "ground_fx_manager.h"
#include "animation_fx_misc.h"
#include "attack_list.h"
#include "water_env_map_rdr.h"
// Sound
#include "nel/sound/sound_anim_manager.h"
// Game share
#include "game_share/ryzom_version.h"
#include "game_share/light_cycle.h"
#include "sound_manager.h"
#include "precipitation_clip_grid.h"

#include "nel/misc/check_fpu.h"

#include "landscape_poly_drawer.h"
#include "session_browser_impl.h"
#include "nel/gui/lua_manager.h"


// ProgressBar steps in init main loop
#define BAR_STEP_INIT_MAIN_LOOP 22

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;


////////////
// EXTERN //
////////////
namespace R2
{
	extern bool ReloadUIFlag;
}

extern EGSPD::CSeason::TSeason	ManualSeasonValue;
UTextureFile			*LoadingBitmap = NULL;
UTextureFile			*LoadingBitmapFull = NULL;
UMaterial				LoadingMaterial;
UMaterial				LoadingMaterialFull = NULL;
std::string				LoadingBitmapFilename;
uint64					StartInitTime = 0;
uint64					StartPlayTime = 0;


// texture for the logos
std::vector<UTextureFile*> LogoBitmaps;

// Use the ESCAPE key during loading, in dev version only

bool						UseEscapeDuringLoading = USE_ESCAPE_DURING_LOADING;


// ***************************************************************************
// MipMap level setup. Don't modify.
#define	ENTITY_TEXTURE_COARSE_LEVEL		3
#define	ENTITY_TEXTURE_NORMAL_LEVEL		1
#define	ENTITY_TEXTURE_HIGH_LEVEL		0
// Size in Mo of the cache for entity texturing.
#define	ENTITY_TEXTURE_NORMAL_MEMORY	10
#define	ENTITY_TEXTURE_HIGH_MEMORY		40

// Don't Modify, set true for debug purpose only
const bool	DBG_DisablePreloadShape= false;


///////////////
// Functions //
///////////////

struct CStatThread : public NLMISC::IRunnable
{
	CStatThread()
	{
		//nlinfo("ctor CStatThread");
	}

	string referer;

	void get(const std::string &url)
	{
		//nlinfo("get '%s'", url.c_str());
		CURL *curl = curl_easy_init();
		if(!curl) return;
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
//		curl_easy_setopt(curl, CURLOPT_USERAGENT, "unknown");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.0.10) Gecko/2009042316 Firefox/3.0.10 (.NET CLR 3.5.30729)");
		curl_easy_setopt(curl, CURLOPT_REFERER, string("http://www.ryzom.com/"+referer).c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		//curl_global_cleanup();
	}

	std::string randomString()
	{
		std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		std::string s;
		for (int i = 0; i < 32; i++)
		{
			s += chars[uint(frand(float(chars.size())))];
		}
		return s;
	}

	void addParam(std::string &params, const std::string &name, const std::string &val)
	{
		if(val.empty()) return;
		if(!params.empty()) params += "&";
		params += name+"="+val;
	}

	string cookie()
	{
		string name;
		if(UserEntity && !UserEntity->getEntityName().toString().empty())
			name = UserEntity->getEntityName().toString();

		std::string userid = toString("u%d", NetMngr.getUserId())+name;
		return toUpper(getMD5((const uint8 *)userid.c_str(), (uint32)userid.size()).toString());
	}

	// return true if we sent the connect because we have all information
	bool connect()
	{
		//nlinfo("connect");
		if(!UserEntity || UserEntity->getEntityName().toString().empty())
			return false;

		referer = ContinentMngr.getCurrentContinentSelectName();

		std::string params;
		addParam(params, "ra", randomString());
		std::string session = toString("%d%d", NetMngr.getUserId(), CTime::getSecondsSince1970());
		addParam(params, "sessioncookie", toUpper(getMD5((const uint8 *)session.c_str(), (uint32)session.size()).toString()));
		addParam(params, "cookie", cookie());
		addParam(params, "browsertoken", "X");
		addParam(params, "platformtoken", "Win32");
		addParam(params, "language", CI18N::getCurrentLanguageCode());
		addParam(params, "page", "");
		addParam(params, "pagetitle", referer);
		addParam(params, "screen", toString("%dx%d", ClientCfg.ConfigFile.getVar("Width").asInt(), ClientCfg.ConfigFile.getVar("Height").asInt()));
		addParam(params, "referer", "http%3A%2F%2Fwww.ryzom.com%2F"+referer);
		time_t rawtime;
		struct tm * timeinfo;
		char buffer [80];
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (buffer,80,"%H%%3A%M", timeinfo);
		addParam(params, "localtime", buffer);
		addParam(params, "cv_name", UserEntity->getEntityName().toUtf8());
		//addParam(params, "cv_email", "");
		//addParam(params, "cv_avatar", "");
		addParam(params, "cv_Userid", toString(NetMngr.getUserId()));
		extern TSessionId HighestMainlandSessionId;
		string shard;
		switch(HighestMainlandSessionId.asInt())
		{
		case 101: shard = "Aniro"; break;	// fr
		case 102: shard = "Leanon"; break;	// de
		case 103: shard = "Arispotle"; break;	// en
		case 301: shard = "Yubo"; break;	// yubo
		default: shard= "unknown"; break;
		}
		addParam(params, "cv_Shard", shard);
		get("http://ryzom.com.woopra-ns.com/visit/"+params);
		return true;
	}

	void ping()
	{
		//nlinfo("ping");
		std::string params;
		addParam(params, "cookie", cookie());
		addParam(params, "ra", randomString());
		get("http://ryzom.com.woopra-ns.com/ping/"+params);
	}

	void run()
	{
		//nlinfo("run CStatThread");
		bool connected = false;
		uint t = 0;
		while (true)
		{
//			connect();
//			nlSleep(60*1000);
			// connect the first time and every 5 minutes
			if(!connected || t == 5*60/20)
			{
				connected = connect();
				t = 0;
			}
			nlSleep(20*1000);
			if(connected) ping();
			t++;
		}
	}
};

bool startStat = false;

void startStatThread()
{
	if(startStat)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		//nlinfo("startStatThread");
		CStatThread *statThread = new CStatThread();
		IThread	*thread = IThread::create (statThread);
		nlassert (thread != NULL);
		thread->start ();
	}
}

//---------------------------------------------------
// Generic method to wait for Server Initialisation messages:
//---------------------------------------------------
inline void	waitForNetworkMessage(bool &var)
{
	while(!var)
	{
		// Event server get events
		CInputHandlerManager::getInstance()->pumpEventsNoIM();
		// Update network.
		NetMngr.update();
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();
		// Send dummy info
		NetMngr.send();
		// Do not take all the CPU.
		nlSleep(100);
	}
}

//---------------------------------------------------
// Wait for the user position and ring information
//---------------------------------------------------
void waitForUserCharReceived()
{
	if ((!ClientCfg.Local)/*ace&&(!ClientCfg.Light)*/)
	{
		// Get the position from the server (will fill either UserEntity or UserEntityInitPos/Front)
		waitForNetworkMessage(UserCharPosReceived);
	}
	else
	{
		// Get the position from the cfg.
		if (UserEntity)
		{
			UserEntity->pos(ClientCfg.Position);
			UserEntity->front(ClientCfg.Heading);
		}
		else
		{
			UserEntityInitPos = ClientCfg.Position;
			UserEntityInitFront = ClientCfg.Heading;
		}
	}
	// Display the start position for the user.
	const CVectorD& p = UserEntity ? UserEntity->pos() : UserEntityInitPos;
	//nlinfo("Start Position: %f %f %f", p.x, p.y, p.z);
}


//---------------------------------------------------
// getRyzomTime :
// Get the user position at the beginning.
//---------------------------------------------------
inline void getRyzomTime()
{
	/*
	// Get the position from the server.
	if(!ClientCfg.Local && !ClientCfg.Light)
	{
		while(RT.getRyzomTime() == 0)
		{
			// Event server get events
			CInputHandlerManager::getInstance()->pumpEventsNoIM();
			// Update network.
			NetMngr.update();
			CCDBNodeBranch::flushObserversCalls();
			// Send dummy info
			NetMngr.send();
			// Do not take all the CPU.
			nlSleep(100);
		}
	}
	*/
}// getRyzomTime //


//---------------------------------------------------
// getRyzomTime :
// Get the phrase book at the beginning.
//---------------------------------------------------
inline void	getSabrinaPhraseBook()
{
	// Get the position from the server.
	if ((!ClientCfg.Local)/*ace&&(!ClientCfg.Light)*/)
	{
		waitForNetworkMessage(SabrinaPhraseBookLoaded);
	}
}


//----------------------------------
// Init the weather / day/night mgt
//----------------------------------
static void initWeather()
{
	WeatherManager.init();
	// Load description of light cycles for each season.
	loadWorldLightCycle();
	// Load global weather function parameters
	loadWeatherFunctionParams();
	//
	LightCycleManager.create();
	// direction of wind remains the same
	WeatherManager.setWindDir(CVector::I);
}


//---------------------------------------------------
// initMainLoop :
// Initialize the main loop.
//
// If you add something in this function, check CFarTP,
// some kind of reinitialization might be useful over there.
//---------------------------------------------------
void initMainLoop()
{
	StartInitTime = NLMISC::CTime::getLocalTime();

	Driver->clearBuffers(CRGBA::Black);
	Driver->swapBuffers();
	CNiceInputAuto niceInputs;
	bool WantStartupProfiling = false;

#ifdef _PROFILE_ON_
	if( WantStartupProfiling )
	{
		// start the bench.
		NLMISC::CHTimer::startBench();
		Driver->startBench();
	}
#endif	// 	_PROFILE_ON_
{ // profile init mainloop

	H_AUTO ( RZ_Client_InitMainLoop );

	FPU_CHECKER_ONCE

	NLMISC::TTime initStart = ryzomGetLocalTime();
	NLMISC::TTime initLast = initStart;
	NLMISC::TTime initCurrent = initLast;

	// Progress bar for init_main_loop()
	ProgressBar.reset (BAR_STEP_INIT_MAIN_LOOP);
	ucstring nmsg;

	FPU_CHECKER_ONCE

	// Get the interface manager
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	{
		H_AUTO(RZUnInOut)
		pIM->uninitOutGame();
	}

	initLast = initCurrent;
	initCurrent = ryzomGetLocalTime();
	//nlinfo ("PROFILE: %d seconds (%d total) for Uninitializing outgame", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

	// Create the game interface database
	{
		H_AUTO(InitRZDB)

		// Initialize the Database.
		nmsg = "Initializing XML Database ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		IngameDbMngr.init(CPath::lookup("database.xml"), ProgressBar);
		ICDBNode::CTextId textId("SERVER");
		if( NLGUI::CDBManager::getInstance()->getDB()->getNode(textId, false) )
			NLGUI::CDBManager::getInstance()->getDB()->removeNode(textId);
		NLGUI::CDBManager::getInstance()->getDB()->attachChild(IngameDbMngr.getNodePtr(),"SERVER");

		// Set the database
		NetMngr.setDataBase (IngameDbMngr.getNodePtr());

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing XML database", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	// Create interface database
	{
		H_AUTO(InitRZUI)

		// Initialize interface v3 should be done after IngameDbMngr.init()
		nmsg = "Initializing Interface Database ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		// Add the LOCAL branch
		ICDBNode::CTextId textId("LOCAL");
		if( NLGUI::CDBManager::getInstance()->getDB()->getNode(textId, false) )
			NLGUI::CDBManager::getInstance()->getDB()->removeNode(textId);
		pIM->createLocalBranch(CPath::lookup("local_database.xml"), ProgressBar);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing interface", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		// Ask and receive the user position to start (Olivier: moved here because needed by initInGame())
		nmsg = "Awaiting Start Position ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		waitForUserCharReceived();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Awaiting start position", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	FPU_CHECKER_ONCE

	// display text messages defined in client.cfg
	ProgressBar.ApplyTextCommands = true;

	// Starting to load data.
	nmsg = "Loading data ...";
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

	{
		H_AUTO(InitRZSound)

		// Initialize Sound System
		if(NLSOUND::CSoundAnimManager::instance() == 0)
			nlwarning("initMainLoop : Sound System not Initialized.");

		// During load of the game, fade completely out SFX, and leave outgame music
		// When the game will begin, it will fade in slowly
		if(SoundMngr)
			SoundMngr->setupFadeSound(0.f, 1.f);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing sound", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		H_AUTO(InitRZEntty)
		// Initializing Entities Manager.
		EntitiesMngr.release();
		EntitiesMngr.initialize(256);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Entities manager", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		H_AUTO(InitRZScene)
		// Creating Scene.
		nmsg = "Creating Scene ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		Scene = Driver->createScene(false);
		if(Scene == 0)
			nlerror("initMainLoop : Cannot create a Scene.");

		// use this scene for bloom effect
		CBloomEffect::getInstance().setScene(Scene);

		CLandscapePolyDrawer::getInstance().initLandscapePolyDrawingCallback();


		// init ground fx manager
		EntitiesMngr.getGroundFXManager().init(Scene, ClientCfg.GroundFXMaxDist, ClientCfg.GroundFXMaxNB, ClientCfg.GroundFXCacheSize);

		// Get the main camera and check if valid.
		MainCam = Scene->getCam();
		if(MainCam.empty())
			nlerror("initMainLoop: Cannot Create the main camera.");

		// setup load balancing
		Scene->setPolygonBalancingMode(UScene::PolygonBalancingClamp);
		Scene->setGroupLoadMaxPolygon("Skin", ClientCfg.SkinNbMaxPoly);
		Scene->setGroupLoadMaxPolygon("Fx", ClientCfg.FxNbMaxPoly);
		Scene->setMaxSkeletonsInNotCLodForm(ClientCfg.NbMaxSkeletonNotCLod);
		// enable Scene Lighting
		Scene->enableLightingSystem(true);
		Scene->setAmbientGlobal(CRGBA::Black);

		// Setup the global Wind from cfg.
		Scene->setGlobalWindPower(ClientCfg.GlobalWindPower);
		Scene->setGlobalWindDirection(ClientCfg.GlobalWindDirection);

		// init the clustered sound system
		if (SoundMngr != NULL)
			SoundMngr->getMixer()->initClusteredSound(Scene, 0.01f, 100.0f, 1.0f);

		// Create the background scene
		SceneRoot = Driver->createScene(true);
		// enable Scene Lighting
		SceneRoot->enableLightingSystem(true);
		SceneRoot->setAmbientGlobal(CRGBA::Black);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Creating scene", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	FPU_CHECKER_ONCE

	{
		H_AUTO(InitRZAnim)

		// Initialize automatic animation
		releaseAutoAnimation();
		initAutoAnimation();

		// Animate the scene once
		Scene->animate(0);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing animation", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	UCamera cam2 = SceneRoot->getCam();
	float	fov;
	float	aspectRatio;
	{
		H_AUTO(InitRZColl)
			computeCurrentFovAspectRatio(fov, aspectRatio);
		cam2.setPerspective(fov, aspectRatio, SceneRootCameraZNear, SceneRootCameraZFar);
		cam2.setTransformMode(UTransform::RotQuat);

		// Initialize the timer.
		T1 = ryzomGetLocalTime ();

		// Create the collision manager (Continent has to do some init with it..)
		CollisionManager = Scene->createVisualCollisionManager();
		if(CollisionManager == 0)
			nlwarning("initMainLoop: createVisualCollisionManager had returned 0.");
		// Set this collision manager the one the scene must use for Shadow Reception on Buildings
		Scene->setVisualCollisionManagerForShadow(CollisionManager);
	}

	{
		nmsg = "Initialize Entity Animation Manager ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		if(!ClientCfg.Light)
		{
			#if FINAL_VERSION == 0
				if (ClientCfg.EAMEnabled)
			#endif
			{
				H_AUTO(InitRZEAM)
				// Create the Entity Animation Manager
				EAM = CEntityAnimationManager::getInstance();
				EAM->load(ProgressBar);
			}
		}

		// init attack list manager
		CAttackListManager::getInstance().init();

		// init misc. anim fxs
		if(!ClientCfg.Light && Scene)
		{
			H_AUTO(InitRZAnimFXMisc)
				UAnimationSet *as = Driver->createAnimationSet();
			AnimFXMisc.init("anim_fx_misc.id_to_string_array", as, true);
		}

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Entity Animation Manager", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		H_AUTO(RZInIn)
		// Parse the interface InGame
		nmsg = "Building Interface ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		//nlinfo("****** InGame Interface Parsing and Init START ******");
		pIM->initInGame(); // must be called after waitForUserCharReceived() because Ring information is used by initInGame()

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing ingame", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

		//nlinfo("****** InGame Interface Parsing and Init END ******");
	}

	{
		// Get the sheet for the user from the CFG.
		// Initialize the user and add him into the entity manager.
		// DO IT AFTER: Database, Collision Manager, PACS, scene, animations loaded.
		CSheetId userSheet(ClientCfg.UserSheet);
		TNewEntityInfo emptyEntityInfo;
		emptyEntityInfo.reset();
		EntitiesMngr.create(0, userSheet.asInt(), emptyEntityInfo);
		nlinfo("Created the user with the sheet %s", ClientCfg.UserSheet.c_str());

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing User sheet", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	FPU_CHECKER_ONCE

	if (ClientCfg.Light)
	{
		WarningLog->addNegativeFilter ("findCollisionChains");
		WarningLog->addNegativeFilter ("Primitives have moved");
	}

	{
		H_AUTO(InitRZWeath)
		initWeather();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Weather", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		// Creating Landscape.
		H_AUTO(InitRZLand)

		#if FINAL_VERSION == 0
			if (ClientCfg.LandscapeEnabled)
		#endif
		{
			nmsg = "Creating Landscape ...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			Landscape = Scene->createLandscape();
			if(Landscape == 0)
				nlerror("initMainLoop : Cannot create a Landscape.");

			if (!ClientCfg.Light)
			{
				// Setting Landscape UpdateLighting Frequency
				// Default setup: 1/20. should work well for night/day transition in 30 minutes.
				//	Because all patchs will be updated every 20 seconds => 90 steps.
				//	But enable it only when we enter Night/Day transition, because landscape updateLighting is
				//	still slow (+10% if 1/20).
				// Hence disable update lighting by default
				Landscape->setUpdateLightingFrequency(0);

				// Enable Additive tiles.
				Landscape->enableAdditive(true);

				// Enable Shadows.
				if(ClientCfg.Shadows)
				{
					Landscape->enableReceiveShadowMap(true);
				}

				// RefineCenter not auto, because ThirdPerson => refine slower
				Landscape->setRefineCenterAuto(false);
			}
		}


		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Landscape", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	if (!ClientCfg.Light)
	{
		// Create the cloud scape
		if (ClientCfg.Cloud)
		{
			CloudScape = Scene->createCloudScape();
			if (!CloudScape)
			{
				nlwarning("Can't create cloud scape");
			}
		}

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Cloudscape", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	FPU_CHECKER_ONCE

	{
		// setup good day / season before ig are added.
		RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
		updateDayNightCycleHour();
		StartupSeason =  CurrSeason = RT.getRyzomSeason();
		RT.updateRyzomClock(NetMngr.getCurrentServerTick(), ryzomGetLocalTime() * 0.001);
		updateDayNightCycleHour();
		ManualSeasonValue = RT.getRyzomSeason();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing season", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	{
		// timed fx manager
		H_AUTO(InitRZTimedFX)

		nmsg = "Initializing Timed FX...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		CTimedFXManager::getInstance().reset();
		CTimedFXManager::getInstance().init(Scene, CClientDate(RT.getRyzomDay(), (float) RT.getRyzomTime()),
			WorldLightCycle.NumHours, 15.45646f, ClientCfg.MaxNumberOfTimedFXInstances, 2.f,  // roughly, the size of a tile
			300.f );	// no env fx should be seen farther than that

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Timed FX", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}


	{
		H_AUTO(InitRZWorld)

		// Initialize World and select the right continent.
			nmsg = "Loading World ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		ContinentMngr.load();
		ContinentMngr.select(UserEntity->pos(), ProgressBar);

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Loading continent", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}


	{
		// Initialize the collision manager.
		if (Landscape)
		{
			H_AUTO(InitRZColMg)
			nmsg = "Initializing collision manager ...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			CollisionManager->setLandscape( Landscape );

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing collision manager", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}
	}

	FPU_CHECKER_ONCE

	// Creating Load systems.
	if (!ClientCfg.Light)
	{
		nmsg = "Creating LOD managers ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		// init the LodCharacter Manager. Must do it before SheetMngr.load()
		{
			H_AUTO(InitRZLod)
			LodCharacterUserManager.init();
		}

		// init the hlsBankManager.
		{
			H_AUTO(InitRZHls)
			try
			{
				// load hlsBank
				Driver->loadHLSBank("characters.hlsbank");
			}
			catch(const Exception &e)
			{
				nlwarning("Can't load HLSBank: %s", e.what());
			}
			// setup according to client
			if(ClientCfg.HDTextureInstalled)
			{
				if(ClientCfg.HDEntityTexture)
				{
					// setup "v2 texture" (or 512*512)
					Driver->setupAsyncTextureLod(ENTITY_TEXTURE_COARSE_LEVEL, ENTITY_TEXTURE_HIGH_LEVEL);
					// Allow a big cache for them (should be on 128 Mo card only)
					Driver->setupMaxTotalAsyncTextureSize(ENTITY_TEXTURE_HIGH_MEMORY*1024*1024);
				}
				else
				{
					// setup "v1 texture" (or 256*256)
					Driver->setupAsyncTextureLod(ENTITY_TEXTURE_COARSE_LEVEL, ENTITY_TEXTURE_NORMAL_LEVEL);
					// Allow a big cache for them
					Driver->setupMaxTotalAsyncTextureSize(ENTITY_TEXTURE_NORMAL_MEMORY*1024*1024);
				}
			}
			else
			{
				/* setup "v1 texture" (or 256*256), but take into account that they are already stored as V1...
					=> remove 1 from the right shit (=> 0,2)
				*/
				Driver->setupAsyncTextureLod(ENTITY_TEXTURE_COARSE_LEVEL-1, ENTITY_TEXTURE_NORMAL_LEVEL-1);
				// Allow a big cache for them
				Driver->setupMaxTotalAsyncTextureSize(ENTITY_TEXTURE_NORMAL_MEMORY*1024*1024);
			}
		}

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Creating LOD managers", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	FPU_CHECKER_ONCE


	// PreLoad Fauna and Characters
	if (!ClientCfg.Light && ClientCfg.PreCacheShapes)
	{
		ucstring nmsg("Loading character shapes ...");
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );


		// Add a PreLoad Cache the shape bank, with a maximum cache Size => never deleted
		const string	PreLoadCacheName= "PreLoad";
		Driver->getShapeBank()->addShapeCache(PreLoadCacheName);
		Driver->getShapeBank()->setShapeCacheSize(PreLoadCacheName, 1000000);

		{
			H_AUTO(InitRZCharacters)
			// **** Load Characters shapes from BNP
			if( CBigFile::getInstance().isBigFileAdded("characters.bnp") )
			{
				ProgressBar.progress (0);
				ProgressBar.pushCropedValues (0, 0.25f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "characters.bnp", "*.shape", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.25f);
				ProgressBar.pushCropedValues (0.25f,0.5f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "characters.bnp", "*.skel", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
			// else Load Characters shapes from the new BNP
			else if( CBigFile::getInstance().isBigFileAdded("characters_shapes.bnp") )
			{
				ProgressBar.progress (0);
				ProgressBar.pushCropedValues (0, 0.25f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "characters_shapes.bnp", "*.shape", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.25f);
				ProgressBar.pushCropedValues (0.25f,0.5f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "characters_skeletons.bnp", "*.skel", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
			// else load from shapes dir
			else
			{
				ProgressBar.progress (0);
				ProgressBar.pushCropedValues (0, 0.25f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheName, "data/3d/common/characters/shapes", "*.shape", false, &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.25f);
				ProgressBar.pushCropedValues (0.25f,0.5f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheName, "data/3d/common/characters/skeletons", "*.skel", false, &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
		}

		{
			H_AUTO(InitRZFauna)
			// **** Same for fauna
			if( CBigFile::getInstance().isBigFileAdded("fauna.bnp") )
			{
				ProgressBar.pushCropedValues (0.5f, 0.75f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "fauna.bnp", "*.shape", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.75f);
				ProgressBar.pushCropedValues (0.75f,1);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "fauna.bnp", "*.skel", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
			// else load from shapes dir
			else if( CBigFile::getInstance().isBigFileAdded("fauna_shapes.bnp") )
			{
				ProgressBar.pushCropedValues (0.5f, 0.75f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "fauna_shapes.bnp", "*.shape", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.75f);
				ProgressBar.pushCropedValues (0.75f,1);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheName, "fauna_skeletons.bnp", "*.skel", &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
			// else load from shapes dir
			else
			{
				ProgressBar.pushCropedValues (0.5f, 0.75f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheName, "data/3d/common/fauna/shapes", "*.shape", false, &ProgressBar);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.75f);
				ProgressBar.pushCropedValues (0.75f,1);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheName, "data/3d/common/fauna/skeletons", "*.skel", false, &ProgressBar);
				}
				ProgressBar.popCropedValues ();
			}
		}

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Loading characters shapes", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

		{
			bool preloadFXTextures = true;
			H_AUTO(InitRZFX)
			// **** Same for fx
			nmsg = "Load FX ...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			const string	PreLoadCacheNameFX= "PreLoadFX";
			Driver->getShapeBank()->addShapeCache(PreLoadCacheNameFX);
			Driver->getShapeBank()->setShapeCacheSize(PreLoadCacheNameFX, 1000000);
			if( CBigFile::getInstance().isBigFileAdded("sfx.bnp") )
			{
				ProgressBar.pushCropedValues (0.0f, 0.5f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheNameFX, "sfx.bnp", "*.ps", &ProgressBar, preloadFXTextures);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.5f);
				ProgressBar.pushCropedValues (0.5f,1);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheNameFX, "sfx.bnp", "*.shape", &ProgressBar, preloadFXTextures);
				}
				ProgressBar.popCropedValues ();
			}
			// else load from shapes dir
			else
			{
				ProgressBar.pushCropedValues (0.0f, 0.5f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheNameFX, "data/3d/common/sfx", "*.ps", true, &ProgressBar, preloadFXTextures);
				}
				ProgressBar.popCropedValues ();
				ProgressBar.progress (0.5f);
				ProgressBar.pushCropedValues (0.5f,1);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheNameFX, "data/3d/common/sfx", "*.shape", true, &ProgressBar, preloadFXTextures);
				}
				ProgressBar.popCropedValues ();
			}

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Loading FX", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}
		{
			bool preloadObjectTextures = true;
			H_AUTO(InitObject)
			// **** Same for objects
			nmsg = "Load Objects ...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			const string	PreLoadCacheNameObjects= "PreLoadObjects";
			Driver->getShapeBank()->addShapeCache(PreLoadCacheNameObjects);
			Driver->getShapeBank()->setShapeCacheSize(PreLoadCacheNameObjects, 1000000);
			if( CBigFile::getInstance().isBigFileAdded("objects.bnp") )
			{
				ProgressBar.pushCropedValues (0.0f, 1.f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromBNP(PreLoadCacheNameObjects, "objects.bnp", "*.shape", &ProgressBar, preloadObjectTextures);
				}
				ProgressBar.popCropedValues ();
			}
			// else load from shapes dir
			else
			{
				ProgressBar.pushCropedValues (0.0f, 1.f);
				if(!DBG_DisablePreloadShape)
				{
					Driver->getShapeBank()->preLoadShapesFromDirectory(PreLoadCacheNameObjects, "data/3d/common/objects", "*.shape", true, &ProgressBar, preloadObjectTextures);
				}
				ProgressBar.popCropedValues ();
			}

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Loading object shapes", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}
	}
	else
	{
		nmsg = "";
		ProgressBar.newMessage (nmsg);
		ProgressBar.newMessage (nmsg);
	}


	FPU_CHECKER_ONCE

	{
		H_AUTO(InitRZOldIt)

		// Initialize Contextual Cursor.
		nmsg = "Initializing Contextual Cursor ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		initContextualCursor();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Contextual Cursor", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}

	TextContext->setColor( CRGBA(255,255,255) );
	{
		H_AUTO(InitRZIg)
		// Load Instance Group.
		nmsg = "Initializing Instances group ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		initIG();

		initLast = initCurrent;
		initCurrent = ryzomGetLocalTime();
		//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Instances group", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
	}


	// Initialize some other parameters.
	nmsg = "Initializing other parameters ...";
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

	// Set the Main Camera.
	MainCam.setPerspective(fov, aspectRatio, CameraSetupZNear, ClientCfg.Vision);
	MainCam.setTransformMode(UTransform::RotQuat);

	// Initialize the Landscape Parameters.
	if (!ClientCfg.Light)
	{
		if (Landscape)
		{
			Landscape->setTileNear(ClientCfg.LandscapeTileNear);	// Set Tile Near.
			Landscape->setThreshold(ClientCfg.getActualLandscapeThreshold());	// Set Threshold.
		}
	}

	// Set the ambient.
	Driver->setAmbientColor(CRGBA(0,0,0));

	{
		H_AUTO(InitRZUI)

		// temp : update all interface links
		CInterfaceLink::updateAllLinks();

		// Initialize new interfaces.
		nmsg = "Initialize New Interfaces ...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		//CDebugInit dbg;
		//dbg.init(&Driver->EventServer);
		CWidgetManager::getInstance()->activateMasterGroup("ui:login", false);
		CWidgetManager::getInstance()->activateMasterGroup("ui:interface", true);

	}

	if ( ClientCfg.R2EDEnabled )
	{
		R2::ReloadUIFlag = true; // make sure the R2 UI gets reloaded
	}

	initLast = initCurrent;
	initCurrent = ryzomGetLocalTime();
	//nlinfo ("PROFILE: %d seconds (%d total) for Initializing other parameters", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

	// Display Launching Message.
	nmsg = "Sending \"Ready\" ...";
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
	// Network Mode
	if(!ClientCfg.Local)
	{
		{
			H_AUTO(InitRZNetwk)

			// Update Network till current tick increase.
			LastGameCycle = NetMngr.getCurrentServerTick();
			while(LastGameCycle == NetMngr.getCurrentServerTick())
			{
				// Event server get events
				CInputHandlerManager::getInstance()->pumpEventsNoIM();
				// Update Network.
				NetMngr.update();
				IngameDbMngr.flushObserverCalls();
				NLGUI::CDBManager::getInstance()->flushObserverCalls();
			}

			// Set the LastGameCycle
			LastGameCycle = NetMngr.getCurrentServerTick();

			// Create the message for the server to create the character.
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:READY", out))
			{
				// transmit language to IOS
				out.serial(ClientCfg.LanguageCode);
				NetMngr.push(out);
				NetMngr.send(NetMngr.getCurrentServerTick());
			}
			else
				nlwarning("initMainLoop : unknown message name : 'CONNECTION:READY'.");
		}
	}

	// to be sure server crash is not fault of client
	ConnectionReadySent= true;

	initLast = initCurrent;
	initCurrent = ryzomGetLocalTime();
	//nlinfo ("PROFILE: %d seconds (%d total) for Sending \"Ready\"", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

	// Display Launching Message.
	nmsg = "Launching ...";
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

//	NLMEMORY::CheckHeap (true);

	// Re-initialise the mouse (will be now in hardware mode, if required)
	InitMouseWithCursor (ClientCfg.HardwareCursor); // the return value of enableLowLevelMouse() has already been tested at startup

	// Re-initialise the keyboard, now in low-level mode, if required
	// NB nico : done at end of loading
/*
	if (!ClientCfg.DisableDirectInput)
	{
		Driver->enableLowLevelKeyboard (true); // the return value has already been tested at startup
	}*/

	SetMouseCursor ();
	SetMouseSpeed (ClientCfg.CursorSpeed);
	SetMouseAcceleration (ClientCfg.CursorAcceleration);

	FPU_CHECKER_ONCE

	// To reset the inputs.
	CInputHandlerManager::getInstance()->pumpEventsNoIM();

	// Set the default edit box for the enter key
//	if (PeopleInterraction.MainChat.Window)
//		CWidgetManager::getInstance()->setCaptureKeyboard(PeopleInterraction.MainChat.Window->getEditBox());
	if (PeopleInterraction.ChatGroup.Window)
	{
		CGroupEditBox	*eb= dynamic_cast<CGroupEditBox*>(PeopleInterraction.ChatGroup.Window->getEditBox());
		CWidgetManager::getInstance()->setCaptureKeyboard(eb);
		// For user help, set a default input string.
		// NB: must do it after interface loadConfig, else it is reseted
		// NB: it is reseted also on first mode switch
		if(eb)
			eb->setDefaultInputString(CI18N::get("uiDefaultChatInput"));
	}
	else
		CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
	CWidgetManager::getInstance()->setCaptureKeyboard(NULL); // previous set editbox becomes '_OldCaptureKeyboard'

	// Some init after connection ready sent
	if(BotChatPageAll && (!ClientCfg.R2EDEnabled))
		BotChatPageAll->initAfterConnectionReady();

	initLast = initCurrent;
	initCurrent = ryzomGetLocalTime();
	//nlinfo ("PROFILE: %d seconds (%d total) for Launching", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);

	nlinfo ("PROFILE: %d seconds for init main loop", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

	ProgressBar.ApplyTextCommands = false;
} // profile init mainloop
#ifdef _PROFILE_ON_
	if( WantStartupProfiling )
	{
		// end the bench.
		NLMISC::CHTimer::endBench();
		Driver->endBench();

		// Display and save profile to a File.
		CLog	log;
		CFileDisplayer	fileDisplayer(NLMISC::CFile::findNewFile(getLogDirectory() + "profile_startup.log"));
		log.addDisplayer(&fileDisplayer);
		// display
		NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(&log, CHTimer::TotalTime, true, 48, 2);
		NLMISC::CHTimer::display(&log, CHTimer::TotalTime);
		Driver->displayBench(&log);
	}
#endif	// 	_PROFILE_ON_


	// init CSessionBrowserImpl
	CSessionBrowserImpl::getInstance().init(CLuaManager::getInstance().getLuaState());

	// active/desactive welcome window
	initWelcomeWindow();

	// active/desactive bloom config interface
	initBloomConfigUI();

	// popup to offer hardware cursor activation
	initHardwareCursor();

	// disable woopra stats
	//startStatThread();

}// initMainLoop //


// ***************************************************************************

void destroyLoadingBitmap ()
{
	if (LoadingBitmap && Driver)
	{
		// Destroy the Loading Background.
		Driver->deleteTextureFile(LoadingBitmap);
		LoadingBitmap = NULL;
		LoadingBitmapFilename = "";
		LoadingMaterial.setTexture (0, NULL);
	}
	if (LoadingBitmapFull && Driver)
	{
		// Destroy the Loading Background.
		Driver->deleteTextureFile(LoadingBitmapFull);
		LoadingBitmapFull = NULL;
		LoadingMaterialFull.setTexture (0, NULL);

	}
}

// ***************************************************************************

void loadBackgroundBitmap (TBackground background)
{
	string name = CFile::getFilenameWithoutExtension (ClientCfg.Launch_BG);
	string ext = CFile::getExtension (ClientCfg.Launch_BG);
	string filename;

	if (frand(2.0) < 1)
		filename = name+"_0."+ext;
	else
		filename = name+"_1."+ext;
	switch (background)
	{
	case ElevatorBackground:
		filename = ClientCfg.Elevator_BG;
		break;
	case TeleportKamiBackground:
		filename = ClientCfg.TeleportKami_BG;
		break;
	case TeleportKaravanBackground:
		filename = ClientCfg.TeleportKaravan_BG;
		break;
	case ResurectKamiBackground:
		filename = ClientCfg.ResurectKami_BG;
		break;
	case ResurectKaravanBackground:
		filename = ClientCfg.ResurectKaravan_BG;
		break;
	case EndBackground:
		filename = ClientCfg.End_BG;
		break;
	case IntroNevrax:
		filename = ClientCfg.IntroNevrax_BG;
		break;
	case IntroNVidia:
		filename = ClientCfg.IntroNVidia_BG;
		break;
	case LoadBackground:
		if(FreeTrial)
			filename = ClientCfg.LoadingFreeTrial_BG;
		else
			filename = ClientCfg.Loading_BG;
		break;
	default:
		break;
	}

	// Setup the materials
	if (LoadingMaterial.empty())
	{
		LoadingMaterial = Driver->createMaterial();
		LoadingMaterial.initUnlit();
	}
	if (LoadingMaterialFull.empty())
	{
		LoadingMaterialFull = Driver->createMaterial();
		LoadingMaterialFull.initUnlit();
		LoadingMaterialFull.setAlphaTest (true);
	}

	// Bitmap is not the same ?
	if ((filename != LoadingBitmapFilename) && Driver)
	{
		destroyLoadingBitmap ();
		LoadingBitmapFilename = filename;

		// Build a background filename
		name = CFile::getFilenameWithoutExtension (filename);
		ext = CFile::getExtension (filename);

		if (!CPath::lookup (name+"_0."+ext, false, false).empty())
		{
			LoadingBitmap = Driver->createTextureFile(name+"_0."+ext);
			LoadingBitmapFull = Driver->createTextureFile(name+"_1."+ext);
		}
		else
		{
			LoadingBitmap = Driver->createTextureFile(filename);
			LoadingBitmapFull = Driver->createTextureFile(filename);
		}
		LoadingMaterial.setTexture (0, LoadingBitmap);
		LoadingMaterialFull.setTexture (0, LoadingBitmapFull);
		nlassert(LoadingBitmap);
		nlassert(LoadingBitmapFull);
		LoadingBitmap->setAllowDegradation(false);
		LoadingBitmapFull->setAllowDegradation(false);
	}
	for(uint i = 0; i < ClientCfg.Logos.size(); i++)
	{
		std::vector<string> res;
		explode(ClientCfg.Logos[i], std::string(":"), res);
		if(res.size()==9) LogoBitmaps.push_back(Driver->createTextureFile(res[0]));
	}

	FPU_CHECKER_ONCE
}

// ***************************************************************************

void beginLoading (TBackground background)
{
	LoadingContinent = NULL;
	loadBackgroundBitmap (background);
}

// ***************************************************************************

void endLoading ()
{
	destroyLoadingBitmap ();
}

// ***************************************************************************

void setLoadingContinent (CContinent *continent)
{
	LoadingContinent = continent;
}

// ***************************************************************************

void initWelcomeWindow()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceGroup* welcomeWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:welcome_info"));
	if(welcomeWnd)
	{
		bool welcomeDbProp  = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:WELCOME")->getValueBool();
		CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
		welcomeWnd->setActive((sb.CurrentJoinMode!=CFarTP::LaunchEditor) && welcomeDbProp);
	}
}

// ***************************************************************************

void initHardwareCursor(bool secondCall)
{
	CInterfaceManager * pIM = CInterfaceManager::getInstance();
	CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();

	string nodeName = "UI:SAVE:HARDWARE_CURSOR";
	CCDBNodeLeaf * node= NLGUI::CDBManager::getInstance()->getDbProp(nodeName);

	if(node)
	{
		if(!ClientCfg.HardwareCursor && !node->getValueBool())
		{
			// if save cfg file, just crush HardwareCursor variable value
			if(ClientCfg.SaveConfig)
			{
				ClientCfg.writeBool("HardwareCursor", true);
				ClientCfg.ConfigFile.save ();
				ClientCfg.IsInvalidated = true;
			}
			// else, only the first time after this patch, open popup to propose hardare cursor mode
			else
			{
				CInterfaceGroup * cursorWnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:hardware_cursor"));
				if(cursorWnd)
				{
					cursorWnd->setActive((sb.CurrentJoinMode!=CFarTP::LaunchEditor) || secondCall);
					CWidgetManager::getInstance()->setTopWindow(cursorWnd);
					cursorWnd->updateCoords();
					cursorWnd->center();
				}
			}
		}

		if(!node->getValueBool() && ((sb.CurrentJoinMode!=CFarTP::LaunchEditor) || secondCall))
		{
			node->setValueBool(true);
		}
	}
	else
	{
		nlwarning("setDbProp(): '%s' dbProp Not found", nodeName.c_str());
	}
}

// ***************************************************************************
void initBloomConfigUI()
{
	bool supportBloom = Driver->supportBloomEffect();

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCtrlBaseButton* button = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_config:content:fx:bloom_gr:bloom:c"));
	if(button)
	{
		button->setFrozen(!supportBloom);
	}

	button = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_config:content:fx:bloom_gr:square_bloom:c"));
	if(button)
	{
		button->setFrozen(!supportBloom);
	}

	CCtrlScroll * scroll = dynamic_cast<CCtrlScroll*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_config:content:fx:bloom_gr:density_bloom:c"));
	if(scroll)
	{
		scroll->setFrozen(!supportBloom);
	}

	CInterfaceGroup* group = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_config:content:fx:bloom_gr"));

	if(!supportBloom)
	{
		if(group)
			group->setDefaultContextHelp(CI18N::get("uiFxTooltipBloom"));

		ClientCfg.writeBool("Bloom", false);
		ClientCfg.writeBool("SquareBloom", false);
		ClientCfg.writeInt("DensityBloom", 0);
	}
	else
	{
		if(group)
			group->setDefaultContextHelp(ucstring(""));
	}
}
