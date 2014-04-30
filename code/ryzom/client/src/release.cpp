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
#include "nel/misc/debug.h"
#include "nel/misc/async_file_manager.h"
#include "nel/misc/system_utils.h"
// 3D Interface.
#include "nel/3d/bloom_effect.h"
#include "nel/3d/fasthls_modifier.h"
#include "nel/3d/particle_system_manager.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/3d/stereo_hmd.h"
// Client
#include "global.h"
#include "release.h"
#include "actions.h"
#include "ig_client.h"
#include "entities.h"
#include "net_manager.h"
#include "pacs_client.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "sound_manager.h"
#include "weather.h"
#include "weather_manager_client.h"
#include "prim_file.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/people_interraction.h"
#include "connection.h"
#include "commands.h"
#include "timed_fx_manager.h"
#include "interface_v3/sphrase_manager.h"
#include "interface_v3/chat_text_manager.h"
#include "interface_v3/dbctrl_sheet.h"
#include "projectile_manager.h"
#include "init_main_loop.h"
#include "fx_manager.h"
#include "animation_fx_misc.h"
#include "fx_manager.h"
#include "micro_life_manager.h"
#include "attack_list.h"
#include "auto_anim.h"
#include "string_manager_client.h"
#include "precipitation_clip_grid.h"
#include "interface_v3/music_player.h"
#include "login.h"
#include "actions_client.h"
#include "login_progress_post_thread.h"
//
#include "r2/editor.h"
#include "nel/misc/big_file.h"
#include "nel/net/module_manager.h"
#include "game_share/fame.h"
#include "interface_v3/sbrick_manager.h"
#include "interface_v3/skill_manager.h"
#include "sheet_manager.h"
#include "interface_v3/macrocmd_manager.h"
#include "game_share/scenario_entry_points.h"
#include "interface_v3/bar_manager.h"
#include "landscape_poly_drawer.h"
#include "game_share/visual_slot_manager.h"
#include "door_manager.h"
#include "interface_v3/encyclopedia_manager.h"
#include "faction_war_manager.h"
#include "interface_v3/interface_ddx.h"
#include "bg_downloader_access.h"
#include "nel/gui/lua_manager.h"


///////////
// USING //
///////////
using namespace NL3D;
using namespace NLMISC;

////////////
// EXTERN //
////////////
extern UDriver					*Driver;
extern UScene					*Scene;
extern UCloudScape				*CloudScape;
extern UVisualCollisionManager	*CollisionManager;
extern CEventsListener			EventsListener;				// Inputs Manager
extern ULandscape				*Landscape;
extern UCamera					MainCam;
extern UScene					*SceneRoot;
extern UScene					*SkyScene;
extern UInstanceGroup			*BackgroundIG;
extern bool						LastScreenSaverEnabled;
extern bool						IsInRingSession;
extern bool noUserChar;
extern bool userChar;
extern bool serverReceivedReady;
extern bool CharNameValidArrived;


extern void releaseContextualCursor();
extern void selectTipsOfTheDay (uint tips);

///////////////
// FUNCTIONS //
///////////////

// ***************************************************************************
// 3D element release, called from both releaseMainLoopReselect() and releaseMainLoop()
static void releaseMainLoopScenes()
{
	if(!Driver)
		return;

	// Delete the main scene
	if(Scene)
	{
		// Release water envmap
#ifdef USE_WATER_ENV_MAP
		Driver->deleteWaterEnvMap(WaterEnvMap);
#endif

		// Release FX manager
		CTimedFXManager::getInstance().reset();

		WeatherManager.release();

		// Release the landscape. NB: all pending async loading zones are deleted here
		if (Landscape)
		{
			Scene->deleteLandscape (Landscape);
			Landscape = NULL;
		}

		// Release the collision manager
		Scene->deleteVisualCollisionManager(CollisionManager);
		CollisionManager = NULL;

		// release cloud scape
		if (CloudScape)
		{
			Scene->deleteCloudScape(CloudScape);
			CloudScape = NULL;
		}

		// remove the scene from the sound lib
		if (SoundMngr != NULL)
			SoundMngr->getMixer()->initClusteredSound((UScene*)NULL, 0.01f, 100.0f, 1.0f);

		// Stop any async loading. Actually all should have been stop before
		CAsyncFileManager::terminate();

		// Release the scene.
		Driver->deleteScene(Scene);
		Scene = NULL;
		MainCam = NULL;

		// remove scene from bloom
		CBloomEffect::getInstance().setScene(NULL);
	}

	// Delete the scene with the Big Root
	if (SceneRoot)
	{
		if (BackgroundIG)
		{
			BackgroundIG->removeFromScene (*SceneRoot);
			SceneRoot->deleteInstanceGroup (BackgroundIG);
			BackgroundIG = NULL;

		}
		Driver->deleteScene(SceneRoot);
		SceneRoot = NULL;
	}
}

volatile bool TempResetShapeBankOnRetCharSelect = false;

// ***************************************************************************
// Release all the memory before come back to out game (real thing!)
void	releaseMainLoopReselect()
{
	ProgressBar.release();

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// save keys loaded and interface cfg (not done in releaseMainLoop() because done at end of mainLoop()...)
	pIM->uninitInGame0();

	// alredy called from farTPMainLoop()
	// --R2::getEditor().autoConfigRelease(IsInRingSession);

	// Pause any user played music
	MusicPlayer.pause();

	// only really needed at exit
	// --STRING_MANAGER::CStringManagerClient::instance()->flushStringCache();

	// Remove all entities.
	if (Driver)
	{
		nldebug("RCSR1: %u textures", Driver->getTotalAsyncTextureSizeAsked());
	}

	EntitiesMngr.release();

	if (Driver)
	{
		nldebug("RCSR2: %u textures", Driver->getTotalAsyncTextureSizeAsked());
	}

	// Reset Fx manager (must be done after EntitiesMngr.release()) Important because may still point to 3D elements
	FXMngr.reset();

	// Interface release
	CInterfaceManager::getInstance()->uninitInGame1();

	// Remove key / action / some interface stuff (done through CInterfaceManager::destroy() in releaseMainLoop I think)
	ActionsContext.removeAllCombos();
	EditActions.releaseAllKeyNoRunning();
	Actions.releaseAllKeyNoRunning();
	CWidgetManager::getInstance()->getParser()->removeAllTemplates();
	CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
	CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
	CWidgetManager::getInstance()->setCapturePointerRight(NULL);

	// Yoyo: Don't release attack list manager, because I think it only owns static data (and 3D data created from Driver, not Scenes)
	// Note that in initMainLoop(), CAttackListManager::getInstance().init() will do nothing (since already created and not released here)
	// --CAttackListManager::getInstance().release();

	// Still release the AnimFXMisc, because btw will still be released at AnimFXMisc.init() time init initMainLoop()
	AnimFXMisc.release();

	// Don't care!
	// --selectTipsOfTheDay (rand());

	// Reset the continent manager. NB: btw will be done at initMainLoop() (from preloadSheets)
	// and important to do before release of timedfx manager
	ContinentMngr.reset();

	// Remove all projectile
	CProjectileManager::getInstance().reset();

	// Remove micro-life (btw done at each CContinent::unselect())
	CMicroLifeManager::getInstance().release();

	// not really needed (static GuildMat Material created from Driver)
	// --CDBCtrlSheet::release ();

	// Release the Entities Animation Manager (Yoyo: fuckingly important because keep a pointer
	// on a _PlayListManager that is created from Scene)
	if (Driver)
	{
		nldebug("RCSR3: %u textures", Driver->getTotalAsyncTextureSizeAsked());
	}

	CEntityAnimationManager::delInstance();
	EAM= NULL;

	if (Driver)
	{
		nldebug("RCSR4: %u textures", Driver->getTotalAsyncTextureSizeAsked());
	}

	// Not necessary I think because owns only static data (string + function ptrs)
	// --releaseContextualCursor();

	// Release 3D components
	if(Driver)
	{
		// Release Scene, SceneRoot and their elements
		// Yoyo: important to do it else leak because recreated/reloaded in initMainLoop() on same pointer (=> old one not released)!!!
		releaseMainLoopScenes();

		// release the auto animation (btw done in initMainLoop()...)
		releaseAutoAnimation();

		// Don't Release the shape bank! => optimisation: preloading of character/objects shapes will not be redone!
		if (ClientCfg.ResetShapeBankOnRetCharSelect)
		{
			Driver->getShapeBank()->reset();
			nldebug("RCSR5: %u textures", Driver->getTotalAsyncTextureSizeAsked());
		}

	}

	// Release FX manager
	CTimedFXManager::getInstance().reset();

	// Don't Purge memory because PointLight may still be present in other Scenes (sky scene...) for instance
	// --UDriver::purgeMemory();

	// String manager: remove all waiting callbacks and removers
	// (if some interface stuff has not received its string yet, its remover will get useless)
	STRING_MANAGER::CStringManagerClient::release( false );

	// release titles info
	CSkillManager::getInstance()->uninitInGame();

	// Ugly globals
	userChar = false;
	noUserChar = false;
	serverReceivedReady = false;
	CharNameValidArrived = false;
	UserCharPosReceived = false;
	SabrinaPhraseBookLoaded = false;

	// Unlink the net manager
	NetMngr.setDataBase (NULL);

	// reset the client database and clear all observers. must do this while we are disconnected!
	// First remove the auto copy observers
	pIM->releaseServerToLocalAutoCopyObservers();
	// Then remove the SERVER and LOCAL database (NB: "UI" node was removed by uninitIngame1())
	ICDBNode::CTextId serverId("SERVER"), localId("LOCAL");
	NLGUI::CDBManager::getInstance()->getDB()->removeNode(serverId);
	NLGUI::CDBManager::getInstance()->getDB()->removeNode(localId);
	nlassert(IngameDbMngr.getNodePtr()==NULL);	// actually it is the "SERVER" node kept by CRefPtr => should be NULL
	IngameDbMngr.clear();						// still important for CDBBranch statics data release
	// NB: "SERVER" and "LOCAL" node will be recreated by initMainLoop


	// Don't destroy the whole interface manager. Hope it is not important....
	// --CInterfaceManager::destroy ();

	// Leave Connection stuff to farTPMainLoop
	// --NetMngr.....
}


// ***************************************************************************
// Release all the memory before come back to out game.
// Yoyo: actually, because of change by AJM, this method is called only when the user quit the app.
// see releaseMainLoopReselect() for the actual method called for reselection
// Btw the 2 methods should have strong similarities
void releaseMainLoop(bool closeConnection)
{
	ProgressBar.release();

	// Release R2 editor if applicable
	R2::getEditor().autoConfigRelease(IsInRingSession);

	// Pause any user played music
	MusicPlayer.pause();

	// flush the server string cache
	STRING_MANAGER::CStringManagerClient::instance()->flushStringCache();

	// Remove all entities.
	EntitiesMngr.release();

	// Reset Fx manager (must be done after EntitiesMngr.release())
	FXMngr.reset();

	// Interface release
	CInterfaceManager::getInstance()->uninitInGame1();

	// release attack list manager
	CAttackListManager::getInstance().release();

	AnimFXMisc.release();

	// Change the tips
	selectTipsOfTheDay (rand());

	// Reset the continent manager
	ContinentMngr.reset();

	// Remove all projectile
	CProjectileManager::getInstance().reset();

	if (Landscape)
	{
		//Landscape->removeTileCallback(&HeightGrid);
	}

	// Remove micro-life
	CMicroLifeManager::getInstance().release();

	// CCtrlSheetInfo release
	CDBCtrlSheet::release ();

	// Release the Entities Animation Manager
	CEntityAnimationManager::delInstance();
	EAM= NULL;

	// Release the cursors
	releaseContextualCursor();


	// Release 3D
	if(Driver)
	{
		// Release Scene, SceneRoot and their elements
		releaseMainLoopScenes();

		// release the auto animation
		releaseAutoAnimation();

		// Release the shape bank
		Driver->getShapeBank()->reset();
	}

	// Release FX manager
	CTimedFXManager::getInstance().reset();

	// Purge memory
	UDriver::purgeMemory();

	// Unlink the net manager
	NetMngr.setDataBase (NULL);

	// Send a msg to server
	if(!ClientCfg.Local)
	{
		if (closeConnection)
			NetMngr.disconnect();
		else
			NetMngr.quit();
/*
		// Quit game, return to select character
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:QUIT_GAME", out))
		{
			NetMngr.push(out);
		}
		else
			nlwarning(" unknown message name 'CONNECTION:QUIT_GAME'");
*/

		// to be sure server crash is not fault of client
		ConnectionReadySent= false;
	}

}// release //

// ***************************************************************************
// Called when Quit from OutGame
void releaseOutGame()
{
	CBGDownloaderAccess::getInstance().release();

	ProgressBar.release();

	// flush the server string cache
	STRING_MANAGER::CStringManagerClient::instance()->flushStringCache();

	// Disconnect the client from the server.
	NetMngr.disconnect();

	// Interface release
	CInterfaceManager::getInstance()->uninitOutGame();

	// delete the sound manager
	if(SoundMngr)
	{
		delete SoundMngr;
		SoundMngr = 0;
	}

	// Delete the driver.
	if(Driver)
	{
		// Stop any async loading. Actually nohting should be async loaded in outgame.
		CAsyncFileManager::terminate();

		// If there is a scene.
		if(Scene)
		{
			// Release the scene.
			Driver->deleteScene(Scene);
			Scene = 0;
		}

		// Remove the Actions listener from the Events Server.
		EventsListener.removeFromServer(CInputHandlerManager::getInstance()->FilteredEventServer);

		// Release Bloom
		CBloomEffect::releaseInstance();

		// Release Scene, textcontexts, materials, ...
		Driver->release();

		// Delete the driver.
		delete Driver;
		Driver = 0;
	}

	ContinentMngr.reset();
}

void releaseStereoDisplayDevice()
{
	if (StereoDisplay)
	{
		delete StereoDisplay;
		StereoDisplay = NULL;
		StereoHMD = NULL;
	}
	IStereoDisplay::releaseAllLibraries();
}

// ***************************************************************************
// final release : Release before exit.
void release()
{
	if (StartPlayTime != 0)
	{
		CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_GameExit, "login_step_game_exit&play_time=" + toString((NLMISC::CTime::getLocalTime() - StartPlayTime) / 1000)));
	}

	CBGDownloaderAccess::getInstance().release();

	ProgressBar.release();

	R2::getEditor().release();
	R2::CEditor::releaseInstance();

	// flush the server string cache
	STRING_MANAGER::CStringManagerClient::instance()->flushStringCache();
	STRING_MANAGER::CStringManagerClient::release(true);

	// restore screensaver state
	CSystemUtils::enableScreensaver(LastScreenSaverEnabled);

	// release PACS primitives
	deletePrimitiveBlocks();

	// Release the commands
	releaseCommands();

	// Exit config file stuff
	ClientCfg.release ();

	// Disconnect the client from the server.
	NetMngr.disconnect();

	// delete the sound manager
	if(SoundMngr)
	{
		delete SoundMngr;
		SoundMngr = 0;
	}

	// Release the Entities Animation Manager
	CEntityAnimationManager::delInstance();
	EAM= NULL;

	nldebug("VR [C]: VR Shutting down");
	releaseStereoDisplayDevice();

	// Delete the driver.
	if(Driver)
	{
		// Release the prim
		PrimFiles.release (*Driver);

		if (TextContext != NULL)
			Driver->deleteTextContext(TextContext);
		TextContext = NULL;

		// Release Bloom
		CBloomEffect::releaseInstance();

		// Release Scene, textcontexts, materials, ...
		Driver->release();

		// Delete the driver.
		delete Driver;
		Driver = 0;
	}

	NetMngr.getConnection().close();
	HttpClient.disconnect();

	// Remove the Actions listener from the Events Server.
	EventsListener.removeFromServer(CInputHandlerManager::getInstance()->FilteredEventServer);

	IDisplayer *clientLogDisplayer = ErrorLog->getDisplayer("CLIENT.LOG");
	if( clientLogDisplayer )
	{
		DebugLog->removeDisplayer (clientLogDisplayer);
		InfoLog->removeDisplayer (clientLogDisplayer);
		WarningLog->removeDisplayer (clientLogDisplayer);
		ErrorLog->removeDisplayer (clientLogDisplayer);
		AssertLog->removeDisplayer (clientLogDisplayer);
		delete clientLogDisplayer;
	}

	CSheetId::uninit();

	// shutdown a few other singletons
	CLoginProgressPostThread::releaseInstance();
	CAttackListManager::releaseInstance();
	CFactionWarManager::release();
	CEncyclopediaManager::releaseInstance();
	CDoorManager::releaseInstance();
	NL3D::CParticleSystemManager::release();
	CUserCommand::release();
	CStaticFames::releaseInstance();
	CSPhraseManager::releaseInstance(); // must release before BrickManager, SkillManager
	CSBrickManager::releaseInstance();
	CSkillManager::releaseInstance();
	CVisualSlotManager::releaseInstance();
	CEntityAnimationManager::delInstance();
	CBarManager::releaseInstance();
	CInterfaceManager::destroy();
	CDDXManager::releaseInstance();
	R2::CObjectSerializer::releaseInstance();
	NLMISC::CBigFile::getInstance().removeAll();
	NLMISC::CBigFile::releaseInstance();
	NL3D::CFastHLSModifier::releaseInstance();
	CLandscapePolyDrawer::releaseInstance();
	NL3D::CParticleSystemShape::releaseInstance();
	NLMISC::CPath::releaseInstance();
	SheetMngr.release();
//	releaseWeather();	// AJM FIXME conflicting ownership with SheetManager
	R2::CScenarioEntryPoints::releaseInstance();
	CMacroCmdManager::releaseInstance();
	CInputHandlerManager::releaseInstance();
	ICDBNode::releaseStringMapper();
	CClassRegistry::release();
	CReflectSystem::release();
	CInterfaceExpr::release();
	CPdrTokenRegistry::releaseInstance();
	NLNET::IModuleManager::releaseInstance();
	delete &CLuaManager::getInstance();
	NLGUI::CDBManager::release();
	CWidgetManager::release();
	



#if FINAL_VERSION
	// openURL ("http://ryzom.com/exit/");
#endif

}// release //
