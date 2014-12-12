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

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#include "nel/gui/action_handler.h"
#include "../misc.h"
#include "../prim_file.h"
#include "../graph.h"
#include "../motion/user_controls.h"
#include "../ig_callback.h"
#include "../movie_shooter.h"
#include "../weather.h"
#include "../weather_manager_client.h"
#include "../continent_manager.h"
#include "../user_entity.h"
#include "../connection.h"

using namespace NLGUI;

////////////
// GLOBAL //
////////////
extern bool					Render;
extern bool					WantProfiling;			// Do we want a CPU profile?
extern bool					WantProfilingVBLock;			// Do we want a VBLock profile?
extern bool					PACSBorders;
extern bool					DebugClusters;
extern bool					SoundBox;
extern uint8				ShowInfos;
extern UDriver				*Driver;
extern CUserControls		UserControls;
extern CIGCallback			*IGCallbacks;
extern CLandscapeIGManager	LandscapeIGManager;
extern bool					MovieShooterSaving;
extern bool					Filter3D[RYZOM_MAX_FILTER_3D];
extern ULandscape			*Landscape;
extern EGSPD::CSeason::TSeason		ManualSeasonValue;
extern CContinentManager	ContinentMngr;
extern NL3D::UScene			*Scene;
extern UCamera				MainCam;
extern bool                 ForceTrueWeatherValue;
extern bool					InitCloudScape;
extern bool					DisplayWeatherFunction;
extern bool					FreezeGraph;
extern bool					MovieShooterReplay;
extern bool					MovieShooterSave;

void	endMovieShooting();


// ***************************************************************************
class CAHDisplayInfos : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// can only be used by devs and CSR or in local mode
#if FINAL_VERSION
		if( ClientCfg.Local || hasPrivilegeDEV() || hasPrivilegeSGM() || hasPrivilegeGM() || hasPrivilegeSG() || hasPrivilegeEM() || hasPrivilegeVG() )
#endif
		{
			ShowInfos = (ShowInfos+1)%6;

			CGraph::Display = (ShowInfos != 0);
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHDisplayInfos, "display_infos");


/**********************************************************************************************************
*																										  *
*										move player handlers  actions									  *
*																										  *
***********************************************************************************************************/

#if !FINAL_VERSION
// ------------------------------------------------------------------------------------------------
class CAHProfile : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		WantProfiling= true;
	}
};
REGISTER_ACTION_HANDLER (CAHProfile, "profile");

// ------------------------------------------------------------------------------------------------
class CAHProfileVBLock : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		WantProfilingVBLock= true;
	}
};
REGISTER_ACTION_HANDLER (CAHProfileVBLock, "profile_vblock");

// ------------------------------------------------------------------------------------------------
class CAHProfileFillRate : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		static	bool	minimized= false;
		minimized= !minimized;
		if(minimized)
		{
			CViewport	vp;
			vp.init(0, 0, 0.1f, 0.1f);
			Scene->setViewport(vp);
		}
		else
		{
			CViewport	vp;
			vp.initFullScreen();
			Scene->setViewport(vp);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHProfileFillRate, "profile_fillrate");

// ------------------------------------------------------------------------------------------------
class CAHProfileTexture : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if(Driver)
		{
			vector<string>	res;
			Driver->profileTextureUsage(res);

			// Display and save profile to a File.
			CLog	log;
			CFileDisplayer	fileDisplayer(NLMISC::CFile::findNewFile(getLogDirectory() + "texture.log"));
			CStdDisplayer	stdDisplayer;
			log.addDisplayer(&fileDisplayer);
			log.addDisplayer(&stdDisplayer);

			log.displayRawNL("********* TEXTURE PROFILE **********");
			for(uint i=0;i<res.size();i++)
			{
				log.displayRawNL(res[i].c_str());
			}
			log.displayRawNL("************************************");
		}
	}
};
REGISTER_ACTION_HANDLER (CAHProfileTexture, "profile_texture");


// ------------------------------------------------------------------------------------------------
class CAHTogglePACSBorders : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		PACSBorders = !PACSBorders;
	}
};
REGISTER_ACTION_HANDLER (CAHTogglePACSBorders, "pacs_borders");

// ------------------------------------------------------------------------------------------------
class CAHToggleDebugClusters : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		DebugClusters = !DebugClusters;
	}
};
REGISTER_ACTION_HANDLER (CAHToggleDebugClusters, "debug_clusters");

/*// ------------------------------------------------------------------------------------------------
class CAHToggleSoundBox : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		SoundBox = !SoundBox;
	}
};
REGISTER_ACTION_HANDLER (CAHToggleSoundBox, "sound_box");
*/
// ***************************************************************************
class CAHTest : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		// For test
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHTest, "test");

// ***************************************************************************
class CAHRenderMode : public IActionHandler
{
	void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		switch(Driver->getPolygonMode())
		{
		case UDriver::Filled:
			Driver->setPolygonMode(UDriver::Line);
			break;
		case UDriver::Line:
			Driver->setPolygonMode(UDriver::Point);
			break;
		case UDriver::Point:
			Driver->setPolygonMode(UDriver::Filled);
			break;
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHRenderMode, "render_mode");

// ***************************************************************************
class CAHToggleRender : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		Render = !Render;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleRender, "toggle_render");

// ***************************************************************************
class CAHSwitchConsoleDisplay : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//		ChatOutput.switchDisplay();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHSwitchConsoleDisplay, "switch_console_display");

// ***************************************************************************
class CAHToggleFly : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Change to AI Mode.
		if(UserControls.mode() != CUserControls::AIMode)
			UserControls.mode(CUserControls::AIMode);
		// Leave the AI Mode.
		else
			UserEntity->viewMode(UserEntity->viewMode());
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleFly, "toggle_fly");

// ***************************************************************************
class CAHReloadLandscapeIg : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (Landscape)
		{
			if (IGCallbacks) IGCallbacks->deleteIGs();
			LandscapeIGManager.reloadAllIgs ();
			nlassert(IGCallbacks);
			std::vector<UInstanceGroup *> igs;
			LandscapeIGManager.getAllIG(igs);
			if (IGCallbacks) IGCallbacks->addIGs(igs);
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHReloadLandscapeIg, "reload_landscape_ig");

// ***************************************************************************
class CAHMemoryReport : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
#ifndef NL_USE_DEFAULT_MEMORY_MANAGER
#ifndef NL_HEAP_ALLOCATION_NDEBUG
//		NLMEMORY::StatisticsReport ("memory_stat.csv", false);
#endif // NL_HEAP_ALLOCATION_NDEBUG
#endif // NL_USE_DEFAULT_MEMORY_MANAGER
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHMemoryReport, "memory_report");

// ***************************************************************************
class CAHTogglePrimitive: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		PrimFiles.toggleShowHide ();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHTogglePrimitive, "toggle_primitive");

// ***************************************************************************
class CAHPrimBrowseUp : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		PrimFiles.loadNext ();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHPrimBrowseUp, "primitive_up");

// ***************************************************************************
class CAHPrimBrowseDown : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		PrimFiles.loadPrevious ();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHPrimBrowseDown, "primitive_down");

// ***************************************************************************
class CAHToggleMovieRecorder : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
#ifdef _MOVIE_SHOOTER_ON_
		if (!MovieShooterSave && !MovieShooterReplay)
		{
			if(!MovieShooter.enabled())
			{
				Driver->systemMessageBox("MovieShooter not enabled", "MovieShooter");
			}
			else
			{
				if( !MovieShooterSaving )
				{
					// reset the movie, and start a new one
					MovieShooter.resetMovie();
					// state
					MovieShooterSaving= true;
				}
				else
				{
					// save the movie
					endMovieShooting();
				}
			}
		}
#endif // _MOVIE_SHOOTER_ON_
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleMovieRecorder, "toggle_movie_recorder");

// ***************************************************************************
class CAHReplayMovie : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (!MovieShooterSave && !MovieShooterSaving)
			MovieShooterReplay = true;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHReplayMovie, "replay_movie");

// ***************************************************************************
class CAHSaveMovie : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (!MovieShooterReplay && !MovieShooterSaving)
			MovieShooterSave = true;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHSaveMovie, "save_movie");

// ***************************************************************************
class CAHToggleFilter3D : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
#ifdef	_PROFILE_ON_

		uint32 filter;
		fromString(Params, filter);

		if (filter <= RYZOM_MAX_FILTER_3D)
		{
			Filter3D[filter] = !Filter3D[filter];
			if (filter == FilterVegetable)
			{
				if (Landscape) Landscape->enableVegetable(Filter3D[FilterVegetable]);
			}
		}
#endif	// _PROFILE_ON_
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleFilter3D, "toggle_3d_filter");

// ***************************************************************************
class CAHToggleSeason: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ClientCfg.ManualWeatherSetup)
		{
			ManualSeasonValue = (EGSPD::CSeason::TSeason) (ManualSeasonValue + 1);
			if (ManualSeasonValue == EGSPD::CSeason::Invalid) ManualSeasonValue = EGSPD::CSeason::Spring;
			class CDummyProgress : public NLMISC::IProgressCallback
			{
				void progress (float /* value */) {}
			};
			CDummyProgress progress;
			ContinentMngr.select(UserEntity->pos(), progress);
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleSeason, "toggle_season");

// ***************************************************************************
class CAHReloadSeason: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// reload all parameters for weather
		loadWorldLightCycle(CSheetId::Unknown); // FIXME
		loadWeatherFunctionParams();
		WeatherManager.init();
		ContinentMngr.reloadWeather();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHReloadSeason, "reload_season");


// ***************************************************************************
class CAHWindTest : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		NL3D::UInstance inst = Scene->createInstance("wind.ps");
		if (!inst.empty())
		{
			inst.setPos(MainCam.getPos());
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHWindTest, "wind_test");


// ***************************************************************************
class CAHReelWeather : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		ForceTrueWeatherValue = !ForceTrueWeatherValue;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHReelWeather, "real_weather");


// ***************************************************************************
class CAHUpdateClouds: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		InitCloudScape = true;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHUpdateClouds, "update_clouds");


// ***************************************************************************
class CAHToggleWeatherFunction: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		DisplayWeatherFunction = !DisplayWeatherFunction;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleWeatherFunction, "toggle_weather_function");


// ***************************************************************************
class CAHToggleFreezeGraph: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		FreezeGraph = !FreezeGraph;
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleFreezeGraph, "toggle_freeze_graph");
#endif // !FINAL_VERSION
