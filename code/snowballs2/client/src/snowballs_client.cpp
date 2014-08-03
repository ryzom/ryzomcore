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

#include <nel/misc/types_nl.h>
#include "snowballs_client.h"
#include "snowballs_config.h"

// STL includes
#include <ctime>
#include <string>
#include <vector>
#include <stdio.h>
#if defined(NL_OS_WINDOWS)
#	include <direct.h>
#	include <tchar.h>
#endif

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/md5.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/command.h>
#include <nel/misc/config_file.h>
#include <nel/net/login_client.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/bloom_effect.h>
#if SBCLIENT_DEV_STEREO
#	include <nel/3d/stereo_render.h>
#endif /* #if SBCLIENT_DEV_STEREO */
#include <nel/3d/stereo_hmd.h>

// Project includes
#include "pacs.h"
#include "radar.h"
#include "graph.h"
#include "camera.h"
#include "compass.h"
#include "commands.h"
#include "entities.h"
#include "network.h"
#include "landscape.h"
#include "animation.h"
#include "interface.h"
#include "lens_flare.h"
#include "mouse_listener.h"
#include "configuration.h"
#include "internationalization.h"
#include "game_time.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;

namespace SBCLIENT {

/*******************************************************************
 *                             GLOBALS                             *
 *******************************************************************/

void CGlobals::assertNull()
{
	// extra verification against stupid typos
	nlassert(!Driver);
	nlassert(!Scene);
	nlassert(!TextContext);
	nlassert(!ConfigFile);
	nlassert(!Landscape);
	nlassert(!MouseListener);
}

// The 3d driver
NL3D::UDriver *Driver = NULL; // core
// This is the main scene
NL3D::UScene *Scene = NULL; // ingame
// This variable is used to display text on the screen
NL3D::UTextContext *TextContext = NULL; // core
// This class contains all variables that are in the snowballs_client.cfg
NLMISC::CConfigFile *ConfigFile = NULL; // core
NL3D::ULandscape *Landscape = NULL; // ingame
// This class is used to handle mouse/keyboard input for camera movement
C3dMouseListener *MouseListener = NULL; // ingame
//// Log file
//CFileDisplayer FileDisplayer; // main

// stuff not initialized
// The previous and current frame dates
NLMISC::TLocalTime LocalTime;
NLMISC::TLocalTime LocalTimeDelta;
// NLMISC::TLocalTime ServerTime;
// NLMISC::TLocalTime ServerTimeDelta;
// NLMISC::TGameTime GameTime;
// NLMISC::TGameTime GameTimeDelta;
// NLMISC::TGameCycle GameCycle;
// NLMISC::TGameCycle GameCycleDelta;
NL3D::TGlobalAnimationTime AnimationTime;
NL3D::TAnimationTime AnimationTimeDelta;
float FramesPerSecond;
float FramesPerSecondSmooth;

// Stuff for connection
ucstring Login;
static string FSAddr, Cookie;

/*******************************************************************
 *                        SNOWBALLS CLIENT                         *
 *******************************************************************/

static CFileDisplayer *_FileDisplayer = NULL;
static const uint8 GameStateLoad = 0, GameStateUnload = 1, GameStateReset = 2, GameStateExit = 3,
GameStateLogin = 4, GameStateOnline = 5, GameStateOffline = 6;

// true if the commands(chat) interface must be display. This variable is set automatically with the config file
static bool ShowCommands; // ingame
// if true, the mouse can't go out the client window(work only on Windows)
static bool CaptureState = false; // ingame
// Set NextGameState to switch the current game state
static uint8 CurrentGameState = -1, NextGameState = 0; // state
// To know which data has been loaded
static bool LoadedCore = false, LoadedIngame = false, LoadedLogin = false,
LoadedOnline = false, LoadedOffline = false; // state

#if SBCLIENT_DEV_STEREO
static IStereoRender *_StereoRender = NULL;
#endif /* #if SBCLIENT_DEV_STEREO */

static bool s_EnableBloom = false;

//
// Prototypes
//

void initLoadingState();
void releaseLoadingState();
void renderLoadingState(const char *state, bool logo3d);
void updateLoadingState(const char *state, bool network, bool information);
void displayLoadingState(const char *state);
void renderLoadingState(ucstring state, bool logo3d);
void updateLoadingState(ucstring state, bool network, bool information);
void displayLoadingState(ucstring state);
void update3dLogo();
void renderInformation();
void switchGameState();
void loopLogin();
void loopIngame();
void initCore();
void initLogin();
void initIngame();
void initOnline();
void initOffline();
void releaseCore();
void releaseLogin();
void releaseIngame();
void releaseOnline();
void releaseOffline();
void cbGraphicsDriver(CConfigFile::CVar &var);
void cbSquareBloom(CConfigFile::CVar &var);
void cbDensityBloom(CConfigFile::CVar &var);
void cbEnableBloom(CConfigFile::CVar &var);

//
// Functions
//

void switchGameState()
{
SwitchNextGameState:
	nlinfo("Switching to the next game state");
	if (CurrentGameState == NextGameState)
	{
		nlwarning("NextGameState wasn't changed");
	}
	else
	{
		switch(CurrentGameState)
		{
		case GameStateOnline:
			releaseOnline(); // always release online after switch
			break;
		case GameStateOffline:
			releaseOffline(); // always releaes offline after switch
			break;
		}
		switch(NextGameState)
		{
		case GameStateLoad:
			try
			{
				initCore(); // core is required
			}
			catch (NLMISC::Exception e)
			{
#ifdef NL_OS_WINDOWS
				MessageBox(NULL, e.what(), "NeL Exception", MB_OK | MB_ICONSTOP);
#else
				printf("%s\n", e.what());
#endif
				return; // exit if driver loading failed
			}
			break;
		case GameStateUnload:
			displayLoadingState("Unloading");
			releaseLogin(); // release all
			releaseIngame();
			break;
		case GameStateReset:
			displayLoadingState("Reset");
			releaseLogin(); // release all
			releaseIngame();
			releaseCore();
			break;
		case GameStateExit:
			displayLoadingState("See you later!");
			releaseLogin(); // release all
			releaseIngame();
			releaseCore();
			break;
		case GameStateLogin:
			initCore(); // core is required
			initLogin(); // login is required
			break;
		case GameStateOnline:
			initCore(); // core is required
			releaseLogin(); //login can be released
			initIngame(); // ingame is required
			initOnline(); // connection is required
			break;
		case GameStateOffline:
			initCore(); // core is required
			releaseLogin(); //login can be released
			initIngame(); // ingame is required
			initOffline(); // offline entity required
			break;
		}
	}
	CurrentGameState = NextGameState;
	switch(CurrentGameState)
	{
	case GameStateLoad: // switch to the default state
		NextGameState = GameStateLogin;
		break;
	case GameStateUnload: // test state, switch back to load for default
		NextGameState = GameStateLoad;
		break;
	case GameStateReset: // used to reset everything
		NextGameState = GameStateLoad;
		break;
	case GameStateExit: // exit the loop
		return;
	case GameStateLogin: // loop the login screen
		loopLogin(); // must loop by itself
		break;
	case GameStateOnline: // start offline if not online
		if (!LoadedOnline)
		{
			NextGameState = GameStateOffline;
			break;
		}
	case GameStateOffline: // loop ingame
		loopIngame(); // must loop by itself
		break;
	}
	goto SwitchNextGameState;
}

void initCore()
{
	if (!LoadedCore)
	{
		LoadedCore = true;
		// Seed the randomizer
		srand(uint(time(0)));
		// Load configuration file, set paths, extension remapping
		CConfiguration::init();
		// Load language file
		CInternationalization::init();
		// Start timing system
		CGameTime::init();
		// Set the ShowCommands with the value set in the client config file
		ShowCommands = ConfigFile->getVar("ShowCommands").asInt() == 1;
		// Create a driver
		Driver = UDriver::createDriver(0, ConfigFile->getVar("OpenGL").asInt() == 0);
		// Create the window with config file values
		Driver->setDisplay(UDriver::CMode(ConfigFile->getVar("ScreenWidth").asInt(),
			ConfigFile->getVar("ScreenHeight").asInt(),
			ConfigFile->getVar("ScreenDepth").asInt(),
			(ConfigFile->getVar("OpenGL").asInt() == 1 ? true : ConfigFile->getVar("ScreenFull").asInt()==0)));
		// Set the cache size for the font manager(in bytes)
		Driver->setFontManagerMaxMemory(2097152);
		// Create a Text context for later text rendering
		displayLoadingState("Initialize Text");
		if (ConfigFile->getVar("OpenGL").asInt() == 1)
			Driver->setMode(UDriver::CMode(ConfigFile->getVar("ScreenWidth").asInt(),
				ConfigFile->getVar("ScreenHeight").asInt(),
				ConfigFile->getVar("ScreenDepth").asInt(),
				ConfigFile->getVar("ScreenFull").asInt()==0));
		TextContext = Driver->createTextContext(CPath::lookup(ConfigFile->getVar("FontName").asString()));
		TextContext->setShaded(true);
		TextContext->setKeep800x600Ratio(false);
		// You can't call displayLoadingState() before init the loading state system
		displayLoadingState("Initialize Loading");
		initLoadingState();
		// Initialize sound for loading screens etc
//#ifdef NL_OS_WINDOWS
//		displayLoadingState("Initialize Sound");
//		initSound();
//		playMusic(SBCLIENT_MUSIC_WAIT);
//#endif
		// Required for 3d rendering (3d nel logo etc)
		displayLoadingState("Initialize Light");
		initLight();
#if SBCLIENT_DEV_STEREO
		displayLoadingState("Initialize Stereo Renderer");
		_StereoRender = Driver->createStereoRender();
		_StereoRender->setMode("AnaglyphRedCyan");
		//_StereoRender->setMode("SideBySideHalf");
		//_StereoRender->setMode("Mono");
#endif /* #if SBCLIENT_DEV_STEREO */

		ConfigFile->setCallback("OpenGL", cbGraphicsDriver);
	}
}

void initLogin()
{
	if (!LoadedLogin)
	{
		LoadedLogin = true;
	}
}

void initIngame()
{
	if (!LoadedIngame)
	{
		LoadedIngame = true;
//#ifdef NL_OS_WINDOWS
//		playMusic(SBCLIENT_MUSIC_WAIT);
//#endif
		displayLoadingState("Initialize");

		// Create a scene
		Scene = Driver->createScene(false);

		// initialize bloom effect
		CBloomEffect::instance().setDriver(Driver);
		CBloomEffect::instance().setScene(Scene);
		CBloomEffect::instance().init();
		CConfiguration::setAndCallback("SquareBloom", cbSquareBloom);
		CConfiguration::setAndCallback("DensityBloom", cbDensityBloom);
		CConfiguration::setAndCallback("EnableBloom", cbEnableBloom);
		// Init the landscape using the previously created UScene
		displayLoadingState("Initialize Landscape");
		initLandscape();
		// Init the pacs
		displayLoadingState("Initialize PACS ");
		initPACS();
		// Init the aiming system
		displayLoadingState("Initialize Aiming ");
		initAiming();
		// Init the user camera
		displayLoadingState("Initialize Camera ");
		initCamera();
		// Create a 3D mouse listener
		displayLoadingState("Initialize MouseListener ");
		MouseListener = new C3dMouseListener();
		MouseListener->addToServer(Driver->EventServer);
		MouseListener->setCamera(Camera);
		MouseListener->setHotSpot(CVectorD(0,0,0));
		MouseListener->setFrustrum(Camera.getFrustum());
		MouseListener->setMatrix(Camera.getMatrix());
		MouseListener->setSpeed(PlayerSpeed);
		initMouseListenerConfig();
		// Init interface
		displayLoadingState("Initialize Interface ");
		initInterface();
		// Init radar
		displayLoadingState("Initialize Radar ");
		initRadar();
		// Init compass
		displayLoadingState("Initialize Compass ");
		initCompass();
		// Init graph
		displayLoadingState("Initialize Graph ");
		initGraph();
		// Init the command control
		displayLoadingState("Initialize Commands ");
		initCommands();
		// Init the entities prefs
		displayLoadingState("Initialize Entities ");
		initEntities();
		// Init animation system
		displayLoadingState("Initialize Animation ");
		initAnimation();
		// Init the lens flare
		displayLoadingState("Initialize LensFlare ");
		initLensFlare();
		// Init the sky
		displayLoadingState("Initialize Sky ");
		initSky();

		// Init the mouse so it's trapped by the main window.
		Driver->showCursor(false);
		Driver->setCapture(true);
		Driver->setMousePos(0.5f, 0.5f);
	}
}

void initOnline()
{
	if (LoadedOnline) return;
//#ifdef NL_OS_WINDOWS
//	playMusic(SBCLIENT_MUSIC_WAIT);
//#endif

	displayLoadingState("Connecting");

	// connect to the server
	nlinfo("Try to connect to FS addr '%s' and identify with the cookie '%s'", FSAddr.c_str(), Cookie.c_str());
	initNetwork(Cookie, FSAddr);

	while (Self == NULL) // wait for position etc from server
		updateLoadingState(ucstring("Connecting"), true, true);

	displayLoadingState("Load Landscape");
	loadAllZonesAround();

	displayLoadingState("Ready!");

//#ifdef NL_OS_WINDOWS
//	playMusic(SBCLIENT_MUSIC_BACKGROUND);
//#endif
	LoadedOnline = true;
}

void initOffline()
{
	if (!LoadedOffline)
	{
		LoadedOffline = true;
//#ifdef NL_OS_WINDOWS
//		playMusic(SBCLIENT_MUSIC_WAIT);
//#endif

		uint32 id = NextEID++;
		Login = ucstring("Entity" + toString(id));

		// Creates the self entity
		displayLoadingState("Creating offline entity");
		CVector startPoint = CVector(ConfigFile->getVar("StartPoint").asFloat(0), ConfigFile->getVar("StartPoint").asFloat(1), ConfigFile->getVar("StartPoint").asFloat(2));
		addEntity(id, Login.toUtf8(), CEntity::Self, startPoint, startPoint);

		displayLoadingState("Load Landscape");
		loadAllZonesAround();

		// Display a local welcome message
		addLine(">>>>> Welcome to Snowballs!");
		addLine(">>>>> Press SHIFT-ESC to exit the game.");

		displayLoadingState("Ready!");

//#ifdef NL_OS_WINDOWS
//		playMusic(SBCLIENT_MUSIC_BACKGROUND);
//#endif
	}
}

void releaseCore()
{
	if (LoadedCore)
	{
		LoadedCore = false;
		// Release configuration callbacks
		CConfiguration::dropCallback("OpenGL");

#if SBCLIENT_DEV_STEREO
		// Release stereo render
		Driver->deleteStereoRender(_StereoRender);
		_StereoRender = NULL;
#endif /* #if SBCLIENT_DEV_STEREO */

		// Release the sun
		releaseLight();
		// Release the loading state textures
		releaseLoadingState();
		// Release the sound
//#ifdef NL_OS_WINDOWS
//		releaseSound();
//#endif
		// Release the text context
		Driver->deleteTextContext(TextContext);
		TextContext = NULL;
		// Release the 3d driver
		Driver->release();
		delete Driver;
		Driver = NULL;

		// Release timing system
		CGameTime::init();
		// Release language file
		CInternationalization::init();
		// Release the configuration
		CConfiguration::release();
	}
}

void releaseLogin()
{
	if (LoadedLogin)
	{
		LoadedLogin = false;
	}
}

void releaseIngame()
{
	if (LoadedIngame)
	{
		LoadedIngame = false;

		// Release the mouse cursor
		if (CaptureState)
		{
			Driver->setCapture(false);
			Driver->showCursor(true);
		}

		// Release all before quit

		releaseSky();
		releaseLensFlare();
		releaseRadar();
		releaseCommands();
		releaseEntities();
		releaseGraph();
		releaseCompass();
		releaseInterface();
		releaseNetwork();
		releaseAnimation();
		releaseMouseListenerConfig();
		releaseCamera();
		releaseAiming();
		releasePACS();
		releaseLandscape();
		// Release the mouse listener
		MouseListener->removeFromServer(Driver->EventServer);
		delete MouseListener;
		MouseListener = NULL;

		// release bloom effect
		CConfiguration::dropCallback("SquareBloom");
		CConfiguration::dropCallback("DensityBloom");
		CBloomEffect::instance().releaseInstance();

		Driver->deleteScene(Scene);
		Scene = NULL;
	}
}

void releaseOnline()
{
	if (LoadedOnline)
	{
		LoadedOnline = false;
		releaseNetwork();
		deleteAllEntities();
	}
}

void releaseOffline()
{
	if (LoadedOffline)
	{
		LoadedOffline = false;
		deleteAllEntities();
	}
}

void loopLogin()
{
//#ifdef NL_OS_WINDOWS
//	playMusic(SBCLIENT_MUSIC_LOGIN);
//#endif
	// todo: login screen, move this stuff to a button or something
	displayLoadingState("Login");
	if (ConfigFile->getVar("Local").asInt() == 0)
	{
		// Only attempt to directly log in if we haven't been passed a cookie already.
		if(Cookie.empty() || FSAddr.empty())
		{
            if (ConfigFile->getVar("UseDirectClient").asInt() == 1)
            {
                string result;
                string LSHost(ConfigFile->getVar("LSHost").asString());
                Login = ConfigFile->getVar("Login").asString();
                string Password = ConfigFile->getVar("Password").asString();
                CHashKeyMD5 hk = getMD5((uint8 *)Password.c_str(), (uint32)Password.size());
                string CPassword = hk.toString();
                nlinfo("The crypted password is %s", CPassword.c_str());
                string Application = ConfigFile->getVar("ClientApplication").asString();
                sint32 sid = ConfigFile->getVar("ShardId").asInt();

                // 1/ Authenticate
                updateLoadingState(ucstring("Authenticate"), false, false);
                result = CLoginClient::authenticateBegin(LSHost, Login, CPassword, Application);
                if (!result.empty()) goto AuthenticateFail;
                while (CLoginClient::authenticateUpdate(result))
                    updateLoadingState(ucstring("Authenticate"), false, false);
                if (!result.empty()) goto AuthenticateFail;
                goto AuthenticateSuccess;

AuthenticateFail:
                nlinfo("*** Authenticate failed '%s' ***", result.c_str());
                for (TLocalTime t = 0; t < 5.000; t += LocalTimeDelta)
                    updateLoadingState(ucstring("Authenticate failed: ") + ucstring(result), false, false);
                NextGameState = GameStateOffline;
                return;

AuthenticateSuccess:
                nlinfo("%d Shards are available:", CLoginClient::ShardList.size());
                for (uint i = 0; i < CLoginClient::ShardList.size(); i++)
                {
                    nlinfo("    ShardId %3d: %s(%d online players)", CLoginClient::ShardList[i].Id, CLoginClient::ShardList[i].Name.toUtf8().c_str(), CLoginClient::ShardList[i].NbPlayers);
                }

                // 2/ Select shard
                updateLoadingState(ucstring("Select shard"), false, false);
                result = CLoginClient::selectShardBegin(sid);
                if (!result.empty()) goto SelectFail;
                while (CLoginClient::selectShardUpdate(result, FSAddr, Cookie))
                    updateLoadingState(ucstring("Select shard"), false, false);
                if (!result.empty()) goto SelectFail;
                goto SelectSuccess;

SelectFail:
                nlinfo("*** Connection to the shard failed '%s' ***", result.c_str());
                for (TLocalTime t = 0; t < 5.000; t += LocalTimeDelta)
                    updateLoadingState(ucstring("Select shard failed: ") + ucstring(result), false, false);
                NextGameState = GameStateOffline;
                return;

SelectSuccess:;
            }
	    }
		NextGameState = GameStateOnline;
		return;
	}
	NextGameState = GameStateOffline;
	return;
}

void loopIngame()
{
	while (CurrentGameState == NextGameState)
	{
		if (!Driver->isActive()) { NextGameState = GameStateExit; return; }

		// call all update functions
		// 01. Update Utilities (configuration etc)
		CConfiguration::updateUtilities(); // update configuration files

		// 02. Update Time (deltas)
		CGameTime::updateTime();
		CGameTime::advanceTime(1.0);

		// 03. Update Incoming (network, receive messages)
		updateNetwork();

		// 04. Update Input (keyboard controls, etc)
		Driver->EventServer.pump(); // Pump user input messages
		MouseListener->update();
		MouseListener->updateCamera();

		// 05. Update Weather (sky, snow, wind, fog, sun)
		animateSky(LocalTimeDelta);

		// 06. Update Landscape (async zone loading near entity)
		updateLandscape(); // Update the landscape

		// ... Update Animations (TEST)
		updateAnimation();

		// 07. Update Entities (collisions and actions)
		//                           - Move Other Entities (move//, animations, etc)
		//                           - Update Self Collision (move)
		//                           - Update Self Entity (animations//, etc)
		updateEntities(); // Update network messages FIXME_NETWORK_OUTGOING

		// 08. Update Animations (playlists)
		Scene->animate(AnimationTime); // Set new animation date

		// 09. Update Camera (depends on entities)
		updateCamera();
		if (StereoHMD) StereoHMD->updateCamera(0, &Camera);

		// 10. Update Interface (login, ui, etc)
		// ...

		// 11. Update Sound (sound driver)
//#ifdef NL_OS_WINDOWS
//		updateSound(); // Update the sound
//#endif

		// 12. Update Outgoing (network, send new position etc)
		// ...

		// 13. Update Debug (stuff for dev)
		// ...

		// if driver is lost (d3d) do nothing for a while
		if (Driver->isLost()) nlSleep(10);
		else
		{
			uint i = 0;
			uint bloomStage = 0;
			while ((!StereoDisplay && i == 0) || (StereoDisplay && StereoDisplay->nextPass()))
			{
				++i;
				if (StereoDisplay)
				{
					const CViewport &vp = StereoDisplay->getCurrentViewport();
					Driver->setViewport(vp);
					Scene->setViewport(vp);
					SkyScene->setViewport(vp);
					StereoDisplay->getCurrentFrustum(0, &Camera);
					StereoDisplay->getCurrentFrustum(0, &SkyCamera);
					StereoDisplay->getCurrentMatrix(0, &Camera);
				}

				if (StereoDisplay)
				{
					StereoDisplay->beginRenderTarget();
				}
				
				if (!StereoDisplay || StereoDisplay->wantClear())
				{

					/*if (s_EnableBloom)
					{
						nlassert(bloomStage == 0);
						CBloomEffect::instance().initBloom(); // start bloom effect (just before the first scene element render)
						bloomStage = 1;
					}*/

					// 01. Render Driver (background color)
					Driver->clearBuffers(CRGBA(0, 0, 127)); // clear all buffers, if you see this blue there's a problem with scene rendering
				}

				if (!StereoDisplay || StereoDisplay->wantScene())
				{				
					// 02. Render Sky (sky scene)
					updateSky(); // Render the sky scene before the main scene

					// 04. Render Scene (entity scene)
					Scene->render(); // Render

					// 05. Render Effects (flare)
					if (!StereoHMD) updateLensFlare(); // Render the lens flare (left eye stretched with stereo...)
				}

				if (!StereoDisplay || StereoDisplay->wantInterface3D())
				{
					/*if (s_EnableBloom && bloomStage == 1)
					{
						// End the actual bloom effect visible in the scene.
						if (StereoDisplay) Driver->setViewport(NL3D::CViewport());
						CBloomEffect::instance().endBloom();
						if (StereoDisplay) Driver->setViewport(StereoDisplay->getCurrentViewport());
						bloomStage = 2;
					}*/

					// 06. Render Interface 3D (player names)
					// ... 
				}

				if (!StereoDisplay || StereoDisplay->wantInterface2D())
				{
					/*if (s_EnableBloom && bloomStage == 2)
					{
						// End bloom effect system after drawing the 3d interface (z buffer related).
						if (StereoDisplay) Driver->setViewport(NL3D::CViewport());
						CBloomEffect::instance().endInterfacesDisplayBloom();
						if (StereoDisplay) Driver->setViewport(StereoDisplay->getCurrentViewport());
						bloomStage = 0;
					}*/

					// 07. Render Interface 2D (chatboxes etc, optionally does have 3d)
					updateCompass(); // Update the compass
					updateRadar(); // Update the radar
					updateGraph(); // Update the radar
					if (ShowCommands) updateCommands(); // Update the commands panel
					renderEntitiesNames(); // Render the name on top of the other players
					updateInterface(); // Update interface
					renderInformation();
					if (!StereoDisplay) update3dLogo(); // broken with stereo

					// 08. Render Debug (stuff for dev)
					// ...
				}

				if (StereoDisplay)
				{
					StereoDisplay->endRenderTarget();
				}
			}

			// 09. Render Buffer
			Driver->swapBuffers();
		}

		// begin various extra keys stuff ...

		if (Driver->AsyncListener.isKeyDown(KeySHIFT) && Driver->AsyncListener.isKeyPushed(KeyESCAPE))
		{
			// Shift Escape -> quit
			NextGameState = GameStateExit;
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF3))
		{
			// F3 -> toggle wireframe/solid
			UDriver::TPolygonMode p = Driver->getPolygonMode();
			p = UDriver::TPolygonMode(((int)p+1)%3);
			Driver->setPolygonMode(p);
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF4))
		{
			// F4 -> clear the command(chat) output
			clearCommands();
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF5))
		{
			// F5 -> display/hide the commands(chat) interface
			ShowCommands = !ShowCommands;
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF6))
		{
			// F6 -> display/hide the radar interface
			RadarState =(RadarState + 1) % 3;
		}
		else if (Driver->AsyncListener.isKeyDown(KeyF7))
		{
			// F7 -> radar zoom out
			RadarDistance += 50;
		}
		else if (Driver->AsyncListener.isKeyDown(KeyF8))
		{
			// F8 -> radar zoom in
			RadarDistance -= 50;
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF9))
		{
			// F9 -> release/capture the mouse cursor
			if (!CaptureState)
			{
				Driver->setCapture(false);
				Driver->showCursor(true);
				MouseListener->removeFromServer(Driver->EventServer);
			}
			else
			{
				Driver->setCapture(true);
				Driver->showCursor(false);
				MouseListener->addToServer(Driver->EventServer);
			}
			CaptureState = !CaptureState;
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF10))
		{
			// F10 -> switch beteen online and offline
			if (isOnline()) NextGameState = GameStateOffline;
			else NextGameState = GameStateLogin;
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF11))
		{
			// F11 -> reset the PACS global position of the self entity(in case of a pacs failure :-\)
			if (Self != NULL)
				resetEntityPosition(Self->Id);
		}
		else if (Driver->AsyncListener.isKeyPushed(KeyF12))
		{
			// F12 -> take a screenshot
			CBitmap btm;
			Driver->getBuffer(btm);
			string filename = CFile::findNewFile("screenshot.tga");
			COFile fs(filename);
			btm.writeTGA(fs,24,false);
			nlinfo("Screenshot '%s' saved", filename.c_str());
		}

		// end of various keys
	}
}

void renderInformation()
{
	// Display if we are online or offline
	TextContext->setHotSpot(UTextContext::TopRight);
	TextContext->setColor(isOnline()?CRGBA(0, 255, 0):CRGBA(255, 0, 0));
	TextContext->setFontSize(18);
	TextContext->printfAt(0.99f, 0.99f, isOnline() ? "Online" : "Offline");

	// Display the frame rate
	TextContext->setHotSpot(UTextContext::TopLeft);
	TextContext->setColor(CRGBA(255, 255, 255, 255));
	TextContext->setFontSize(14);
	TextContext->printfAt(0.01f, 0.99f, "%.2f(%.2f)fps %.3fs", FramesPerSecondSmooth, FramesPerSecond, (float)LocalTimeDelta);
	CVector camPos = Camera.getMatrix().getPos();
	TextContext->printfAt(0.01f, 0.89f, "CAM POS: %.3f %.3f %.3f", camPos.x, camPos.y, camPos.z);

	// one more frame
	FpsGraph.addValue(1.0f);
	SpfGraph.addOneValue((float)LocalTimeDelta);
}

//
// Configuration callbacks
//

void cbGraphicsDriver(CConfigFile::CVar &var)
{
	// -- give ingame warning or something instead =)
	NextGameState = GameStateReset;
}

void cbSquareBloom(CConfigFile::CVar &var)
{
	CBloomEffect::instance().setSquareBloom(var.asBool());
}

void cbDensityBloom(CConfigFile::CVar &var)
{
	CBloomEffect::instance().setDensityBloom((uint8)(var.asInt() & 0xFF));
}

void cbEnableBloom(CConfigFile::CVar &var)
{
	s_EnableBloom = var.asBool();
}

//
// Loading state procedure
//

static UTextureFile *NelLogo = NULL;
// static UTextureFile *NevraxLogo = NULL;
static UTextureFile *SnowballsBackground = NULL;
static float ScreenWidth, ScreenHeight;
// The logo 3D objects
static UScene				*LogoScene = NULL;
static UInstance			Logo = NULL;

void initLoadingState()
{
	NelLogo = Driver->createTextureFile("nel128.tga");
	// NevraxLogo = Driver->createTextureFile("nevrax.tga");
	SnowballsBackground = Driver->createTextureFile("snowbg.tga");
	uint32 width, height;
	Driver->getWindowSize(width, height);
	ScreenWidth =(float)width;
	ScreenHeight =(float)height;

	//
	// Setup the logo scene
	//

	LogoScene = Driver->createScene(false);

	CViewport v;
	v.init(0.0f, 0.80f, 0.2f, 0.2f);
	LogoScene->setViewport(v);

	Logo = LogoScene->createInstance("nel_logo.shape");
	Logo.setPos(0.0f, 3.0f, 0.0f);
	Logo.setTransformMode(UTransformable::RotEuler);
}

void releaseLoadingState()
{
	LogoScene->deleteInstance(Logo); Logo = NULL;
	Driver->deleteScene(LogoScene); LogoScene = NULL;

	Driver->deleteTextureFile(NelLogo); NelLogo = NULL;
	// Driver->deleteTextureFile(NevraxLogo); NevraxLogo = NULL;
	Driver->deleteTextureFile(SnowballsBackground); SnowballsBackground = NULL;
}

void renderLoadingState(const char *state, bool logo3d)
{
	renderLoadingState(ucstring(state), logo3d);
}
void renderLoadingState(ucstring state, bool logo3d)
{
	if (!Driver) return;

	Driver->clearBuffers(CRGBA(0,0,0));

	Driver->setMatrixMode2D(CFrustum(0.0f, ScreenWidth, 0.0f, ScreenHeight, 0.0f, 1.0f, false));
	
	if (SnowballsBackground != NULL)
	{
		float imageAspect = 16.0f / 9.0f; // 1.777
		float minTransform = 4.0f / 3.0f; // 1.333
		float screenAspect = ScreenWidth / ScreenHeight;
		float transform = screenAspect > imageAspect ? 1.0f : (screenAspect < minTransform ? imageAspect / minTransform : imageAspect / screenAspect);
		float newWidth = ScreenWidth * transform;
		Driver->drawBitmap((newWidth - ScreenWidth) * -0.5f, 0, newWidth, ScreenHeight, *SnowballsBackground);
	}
	
	/*if (logo3d) update3dLogo();
	else */ if (NelLogo != NULL) Driver->drawBitmap(16, 32 - 42, 128, 128, *NelLogo);
	// if (NevraxLogo != NULL) Driver->drawBitmap(ScreenWidth - 128 - 16, 32, 128, 16, *NevraxLogo);
	
	if (!TextContext) return;

	TextContext->setColor(CRGBA(255, 255, 255));
	TextContext->setHotSpot(UTextContext::MiddleMiddle);

	TextContext->setFontSize(40);
	TextContext->printAt(0.5f, 0.5f, ucstring("Welcome to Snowballs!"));

	TextContext->setFontSize(30);
	TextContext->printAt(0.5f, 0.2f, state);

	TextContext->setHotSpot(UTextContext::BottomRight);
	TextContext->setFontSize(15);
#if (FINAL_VERSION == 1)
	TextContext->printAt(0.99f, 0.01f, ucstring("Final Version"));
#else
	TextContext->printAt(0.99f, 0.01f, ucstring("(compiled " __DATE__ " " __TIME__ ")"));
#endif

	TextContext->setHotSpot(UTextContext::BottomLeft);
	TextContext->setFontSize(15);
#if defined(NL_DEBUG_FAST)
	ucstring version = ucstring("DebugFast Version");
#elif defined(NL_DEBUG)
	ucstring version = ucstring("Debug Version");
#elif defined(NL_RELEASE)
	ucstring version = ucstring("Release Version");
#elif defined(NL_RELEASE_DEBUG)
	ucstring version = ucstring("ReleaseDebug Version");
#else
	ucstring version = ucstring("Unknown Version");
#endif
	TextContext->printAt(0.01f, 0.01f, version);
}

void displayLoadingState(const char *state)
{
	displayLoadingState(ucstring(state));
}
void displayLoadingState(ucstring state)
{
	CGameTime::updateTime();
	renderLoadingState(state, false);
	Driver->swapBuffers();
	//Driver->EventServer.pump();
}

void updateLoadingState(const char *state, bool network, bool information)
{
	updateLoadingState(ucstring(state), network, information);
}
void updateLoadingState(ucstring state, bool network, bool information)
{
	CGameTime::updateTime(); // important that time is updated here!!!
//#ifdef NL_OS_WINDOWS
//	updateSound();
//#endif
	renderLoadingState(state, true);
	if (information) renderInformation();
	if (network) updateNetwork();
	Driver->swapBuffers();
	Driver->EventServer.pump();
}

void update3dLogo()
{
	Driver->clearZBuffer();
	static float angle = 0.0;
	angle += 2.0f * (float)LocalTimeDelta;
	Logo.setRotEuler(0.0f, 0.0f, angle);
	LogoScene->animate(AnimationTime);
	LogoScene->render();
}

void CSnowballsClient::init()
{
	// Sanity check
	CGlobals::assertNull();
}

bool CSnowballsClient::run()
{
	// Run the game state loop switcher
	switchGameState();
	return true;
}

void CSnowballsClient::release()
{
	// Make sure everything's released
	releaseOnline();
	releaseOffline();
	releaseIngame();
	releaseLogin();
	releaseCore();
	CGlobals::assertNull();
}

} /* namespace SBCLIENT */

void end();
#ifdef NL_OS_WINDOWS
#	ifdef _UNICODE
#		define tstring wstring
#	else
#		define tstring string
#	endif
sint WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
sint main(int argc, char **argv)
#endif
{
	// must do this to allow deallocation when closing with X
	atexit(end);

#ifdef NL_OS_WINDOWS

	// extract the 2 first param (argv[1] and argv[2]) it must be cookie and addr

	string cmd = cmdline;
	string::size_type pos1 = cmd.find_first_not_of (' ');
	string::size_type pos2;
	if (pos1 != string::npos)
	{
		pos2 = cmd.find (' ', pos1);
		if(pos2 != string::npos)
		{
			SBCLIENT::Cookie = cmd.substr (pos1, pos2-pos1);

			pos1 = cmd.find_first_not_of (' ', pos2);
			if (pos1 != string::npos)
			{
				pos2 = cmd.find (' ', pos1);
				if(pos2 == string::npos)
				{
					SBCLIENT::FSAddr = cmd.substr (pos1);
				}
				else if (pos1 != pos2)
				{
					SBCLIENT::FSAddr = cmd.substr (pos1, pos2-pos1);
				}
			}
		}
	}

#else

	if (argc>=3)
	{
		SBCLIENT::Cookie = argv[1];
		SBCLIENT::FSAddr = argv[2];
	}

#endif

	nlinfo ("cookie '%s' addr '%s'", SBCLIENT::Cookie.c_str (), SBCLIENT::FSAddr.c_str());


#if defined(NL_OS_WINDOWS) && !FINAL_VERSION
	// ensure paths are ok before powering up nel
	{
		OutputDebugString("    ********************************    \n");
		OutputDebugString("    *        DEVELOPER MODE        *    \n");
		OutputDebugString("    ********************************    \n\n");
		FILE *f = _tfopen(_T(SBCLIENT_CONFIG_FILE_DEFAULT), _T("r"));
		if (!f)
		{
			f = _tfopen(_T(SBCLIENT_CONFIG_FILE), _T("r"));
			if (!f)
			{
				OutputDebugString("    ********************************    \n");
				OutputDebugString("    *  CHANGING WORKING DIRECTORY  *    \n");
				OutputDebugString("    ********************************    \n\n");
				char cwd[256];
				_tgetcwd(cwd, 256);
				tstring workdir(cwd);
				workdir = "R:\\build\\devw_x86\\bin\\Debug\\";
				_tchdir(workdir.c_str());
				f = _tfopen(_T(SBCLIENT_CONFIG_FILE_DEFAULT), _T("r"));
				if (!f)
				{
					f = _tfopen(_T(SBCLIENT_CONFIG_FILE), _T("r"));
					if (!f)
					{
						OutputDebugString("    ********************************    \n");
						OutputDebugString("    *    DEFAULT CONFIG MISSING    *    \n");
						OutputDebugString("    ********************************    \n\n");
						return EXIT_FAILURE;
					}
				}
			}
		}
		fclose(f);
	}
#endif

	// go nel!
	{
		// use log.log if NEL_LOG_IN_FILE and SBCLIENT_USE_LOG_LOG defined as 1
		createDebug(NULL, SBCLIENT_USE_LOG_LOG, false);

	#if SBCLIENT_USE_LOG
		// create snowballs_client.log
		// filedisplayer only deletes the 001 etc
		if (SBCLIENT_ERASE_LOG && CFile::isExists(SBCLIENT_LOG_FILE))
			CFile::deleteFile(SBCLIENT_LOG_FILE);
		// initialize the log file
		SBCLIENT::_FileDisplayer = new CFileDisplayer();
		SBCLIENT::_FileDisplayer->setParam(SBCLIENT_LOG_FILE, SBCLIENT_ERASE_LOG);
		DebugLog->addDisplayer(SBCLIENT::_FileDisplayer);
		InfoLog->addDisplayer(SBCLIENT::_FileDisplayer);
		WarningLog->addDisplayer(SBCLIENT::_FileDisplayer);
		AssertLog->addDisplayer(SBCLIENT::_FileDisplayer);
		ErrorLog->addDisplayer(SBCLIENT::_FileDisplayer);
	#endif

		nlinfo("Welcome to NeL!");
	}

	SBCLIENT::CSnowballsClient::init();
	exit(SBCLIENT::CSnowballsClient::run() ? EXIT_SUCCESS : EXIT_FAILURE);
	return EXIT_FAILURE;
}
void end()
{
	SBCLIENT::CSnowballsClient::release();
	nlinfo("See you later!");

#if SBCLIENT_USE_LOG
	DebugLog->removeDisplayer(SBCLIENT::_FileDisplayer);
	InfoLog->removeDisplayer(SBCLIENT::_FileDisplayer);
	WarningLog->removeDisplayer(SBCLIENT::_FileDisplayer);
	AssertLog->removeDisplayer(SBCLIENT::_FileDisplayer);
	ErrorLog->removeDisplayer(SBCLIENT::_FileDisplayer);
	delete SBCLIENT::_FileDisplayer; SBCLIENT::_FileDisplayer = NULL;
#endif
}

// Command to quit the client
NLMISC_COMMAND(sb_quit,"quit the client","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;

	log.displayNL("Exit requested");

	SBCLIENT::NextGameState = SBCLIENT::GameStateExit;

	return true;
}

NLMISC_COMMAND(sb_offline, "go offline", "")
{
	if (args.size() != 0) return false;
	SBCLIENT::NextGameState = SBCLIENT::GameStateOffline;
	return true;
}

NLMISC_COMMAND(sb_unload, "unload game", "")
{
	if (args.size() != 0) return false;
	SBCLIENT::NextGameState = SBCLIENT::GameStateUnload;
	return true;
}

NLMISC_COMMAND(sb_reset, "reset game", "")
{
	if (args.size() != 0) return false;
	SBCLIENT::NextGameState = SBCLIENT::GameStateReset;
	return true;
}

NLMISC_COMMAND(sb_login, "go to the login screen", "")
{
	if (args.size() != 0) return false;
	SBCLIENT::NextGameState = SBCLIENT::GameStateLogin;
	return true;
}

#if SBCLIENT_DEV_MEMLEAK
// enable memory leak checks, trick to get _CrtSetBreakAlloc in before main
#define DEBUG_ALLOC_HOOK
#if defined(NL_OS_WINDOWS) && defined(NL_DEBUG)
#if defined(DEBUG_ALLOC_HOOK)
int debugAllocHook(int allocType, void *userData, size_t size, int
	blockType, long requestNumber, const unsigned char *filename, int
	lineNumber);
#endif
class CEnableCrtDebug
{
public:
	CEnableCrtDebug()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(0);
#if defined(DEBUG_ALLOC_HOOK)
		LastSize = 0;
		_CrtSetAllocHook(debugAllocHook);
#endif
	}
#if defined(DEBUG_ALLOC_HOOK)
	size_t LastSize;
#endif
};
static CEnableCrtDebug _EnableCrtDebug;
#if defined(DEBUG_ALLOC_HOOK)
int debugAllocHook(int allocType, void *userData, size_t size, int
	blockType, long requestNumber, const unsigned char *filename, int
	lineNumber)
{
	if (allocType == _HOOK_ALLOC)
	{
		//if (requestNumber == 14806)
		//	_CrtSetBreakAlloc(14809);
		//if (_EnableCrtDebug.LastSize == 4 && size == 40 && requestNumber > 291000 && requestNumber < 292000)
		//	_CrtDbgBreak();
		//if (_EnableCrtDebug.LastSize == 36 && size == 112 && requestNumber > 300000)
		//	_CrtDbgBreak();
		_EnableCrtDebug.LastSize = size;
	}
	return TRUE;
}
#endif
#endif

// trick to clean up nel memory trash! (todo: unload of loaded dynamic nel libs (nel drivers) in linux)
#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/dynloadlib.h>
#include <nel/misc/object_arena_allocator.h>
#include <nel/misc/class_registry.h>
#include <nel/misc/async_file_manager.h>
#include <nel/misc/big_file.h>
#include <nel/3d/particle_system_manager.h>
#include <nel/3d/particle_system_shape.h>
#ifdef NL_OS_WINDOWS
#include <dbghelp.h>
BOOL CALLBACK EnumerateLoadedModulesProc(PCSTR ModuleName, ULONG ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	// free nel libraries (cannot call nlwarning etc here, so nlGetProcAddress etc does not work)
    HMODULE hModule = GetModuleHandle(ModuleName);
	if (GetProcAddress(hModule, NL_MACRO_TO_STR(NLMISC_PURE_LIB_ENTRY_POINT)))
		FreeLibrary(hModule);
	return TRUE;
}
#endif
struct yy_buffer_state;
extern void cf_switch_to_buffer(yy_buffer_state *new_buffer);
extern yy_buffer_state *cf_create_buffer(FILE *file, int size);
extern void cf_delete_buffer(yy_buffer_state *b);
class CCleanupNeL
{
public:
	CCleanupNeL() : _CfBufferState(NULL)
	{
		_CfBufferState = cf_create_buffer(NULL, 16384);
		cf_switch_to_buffer(_CfBufferState);
	}
	~CCleanupNeL()
	{
#ifdef NL_OS_WINDOWS
		// must unload all other nel modules before killing nel context (or they will crash in static destructors)
		EnumerateLoadedModules(GetCurrentProcess(), EnumerateLoadedModulesProc, NULL); // todo: linux version
#endif

		// delete most stuff
		NL3D::CParticleSystemShape::releaseInstance();
		NLMISC::CAsyncFileManager::terminate();
		NL3D::CParticleSystemManager::release();
		NLMISC::CBigFile::releaseInstance();
		NLMISC::CClassRegistry::release();
		delete &NLMISC::CObjectArenaAllocator::getDefaultAllocator();
		cf_delete_buffer(_CfBufferState); _CfBufferState = NULL;

#ifdef NL_OS_WINDOWS
		// delete context related stuff (must unload all dynamic nel libs first)
		NLMISC::destroyDebug();
		if (NLMISC::INelContext::isContextInitialised())
		{
			delete &NLMISC::INelContext::getInstance();
			delete &NLMISC::CInstanceCounterManager::getInstance();
			delete &NLMISC::CCommandRegistry::getInstance();
		}
		NLMISC::CLog::releaseProcessName();
#endif
	}
private:
	yy_buffer_state *_CfBufferState;
};
CCleanupNeL _CleanupNeL;
#endif

/* end of file */
