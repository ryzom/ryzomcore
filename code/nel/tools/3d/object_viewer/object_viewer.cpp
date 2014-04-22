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

#include "std_afx.h"

#undef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllexport )

#include <vector>


#include "object_viewer.h"

#include "nel/3d/nelu.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/text_context.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/init_3d.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/animation_playlist.h"
#include "nel/3d/track_keyframer.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/seg_remanence.h"

#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"

#include "nel/sound/u_audio_mixer.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/ps_util.h"


#include "nel/pacs/global_retriever.h"



#include "select_movie_size.h"
#include "editable_range.h"
#include "range_manager.h"
#include "located_properties.h"
#include "color_button.h"
#include "particle_dlg.h"
#include "resource.h"
#include "main_frame.h"
#include "sound_system.h"
#include "scheme_manager.h"
#include "day_night_dlg.h"
#include "water_pool_editor.h"
#include "vegetable_dlg.h"
#include "dialog_progress.h"
#include "select_string.h"
#include "global_wind_dlg.h"
#include "sound_anim_dlg.h"
#include "light_group_factor.h"
#include "choose_bg_color_dlg.h"
#include "choose_sun_color_dlg.h"
#include "choose_frame_delay.h"
#include "skeleton_scale_dlg.h"
#include "graph.h"
#include "tune_mrm_dlg.h"


using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLSOUND;
using namespace NLPACS;



				

static char SDrive[256];
static char SDir[256];

uint SkeletonUsedForSound = 0xFFFFFFFF;
CSoundContext SoundContext;


//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp

BEGIN_MESSAGE_MAP(CObject_viewerApp, CWinApp)
	//{{AFX_MSG_MAP(CObject_viewerApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp construction

CObject_viewerApp::CObject_viewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CObject_viewerApp object

CObject_viewerApp theApp;


bool CObjectViewer::_InstanceRunning = false;

// ***************************************************************************

class CObjView : public CView
{
public:
	CObjView() 
	{
		MainFrame=NULL;	
	};
	virtual ~CObjView() {}
	virtual void OnDraw (CDC *) {}
	afx_msg BOOL OnEraseBkgnd(CDC* pDC) 
	{ 
		return FALSE; 
	}
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if ((CNELU::Driver) && MainFrame)
			MainFrame->DriverWindowProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
			
		return CView::WindowProc(message, wParam, lParam);
	}
	DECLARE_DYNCREATE(CObjView)
	CMainFrame	*MainFrame;
};

// ***************************************************************************

IMPLEMENT_DYNCREATE(CObjView, CView)


// ***************************************************************************

void animateCNELUScene (CCloudScape *cs, uint64 deltaTime = 0)
{
	if (!cs) return;
	static sint64 firstTime = NLMISC::CTime::getLocalTime();
	static sint64 lastTime = NLMISC::CTime::getLocalTime();
	if (deltaTime == 0)
	{
		deltaTime = NLMISC::CTime::getLocalTime() - lastTime;
	}
	lastTime += deltaTime;
	float fdelta = 0.001f * (float) (lastTime - firstTime);
	CNELU::Scene->animate ( fdelta);
	cs->anim (fdelta, CNELU::Scene->getCam());
}

// ***************************************************************************

CObjectViewer::CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_SceneRoot= NULL;
	
	init3d ();
	
	// vl: is it really useful? i moved it from ov.exe init
	CScene::registerBasics ();

	registerSerial3d();

	_MainFrame = NULL;
	_SlotDlg=NULL;
	_AnimationSetDlg=NULL;
	_AnimationDlg=NULL;
	_ParticleDlg = NULL;
	_FontGenerator = NULL;
	_VegetableLandscape= NULL;
	_VegetableCollisionManager= NULL;
	_VegetableCollisionEntity= NULL;
	_CameraFocal = 75.f; // default value for the focal
	_SelectedObject = 0xffffffff;
	_LightGroupDlg = NULL;
	_ChooseFrameDelayDlg = NULL;
	_ChooseBGColorDlg = NULL;
	_DayNightDlg = NULL;
	_WaterPoolDlg = NULL;
	_SoundAnimDlg = NULL;
	_VegetableDlg = NULL;
	_GlobalWindDlg = NULL;
	_SkeletonScaleDlg = NULL;
	_TuneMRMDlg= NULL;


	// no frame delay is the default
	_FrameDelay = 0;

	// Hotspot color
	_HotSpotColor.R=255;
	_HotSpotColor.G=255;
	_HotSpotColor.B=0;
	_HotSpotColor.A=255;

	_BackGroundColor = CRGBA::Black;

	// Hotspot size
	_HotSpotSize=10.f;

	_Wpm = &NL3D::GetWaterPoolManager();

	_GlobalRetriever= NULL;
	_ObjectLightTest= NULL;

	_CharacterScalePos= 1;
	_CurrentCamera = -1;
	_Direct3d = false;
	_Fog = false;
	_FogStart = 0;
	_FogEnd = 1;
	_FogColor = NLMISC::CRGBA::Black;

	_FXUserMatrix.identity();

	_FXMatrixVisible = false;
	_FXUserMatrixVisible = false;
	_SceneMatrixVisible = false;
	_OcclusionTestMeshsVisible = false;
	_CS = NULL;
}

// ***************************************************************************
std::string CObjectViewer::getModulePath() const
{
	// Get the configuration file path (located in same directory as module)
	HMODULE hModule = AfxGetInstanceHandle();
	nlassert(hModule); // shouldn't be null now anymore in any case
	nlassert(hModule != GetModuleHandle(NULL)); // if this is dll, the module handle can't be same as exe
	char sModulePath[256];
	int res = GetModuleFileName(hModule, sModulePath, 256); nlassert(res);
	nldebug("Object viewer module path is '%s'", sModulePath);
	_splitpath (sModulePath, SDrive, SDir, NULL, NULL);
	_makepath (sModulePath, SDrive, SDir, "object_viewer", ".cfg");
	return sModulePath;
}


// ***************************************************************************
void CObjectViewer::loadDriverName()
{
	// Load the config file
	CConfigFile cf;
	cf.load (getModulePath());
	CConfigFile::CVar *var = cf.getVarPtr("driver");
	_Direct3d = var && (var->asString() == "direct3d");
}

// ***************************************************************************
void CObjectViewer::loadConfigFile()
{
	// Load object_viewer.ini
	try
	{
		// Load the config file
		CConfigFile cf;
		cf.load (getModulePath());
		
		try
		{
			// Add search pathes
			CConfigFile::CVar &search_pathes = cf.getVar ("search_pathes");
			for (uint i=0; i<(uint)search_pathes.size(); i++)
				CPath::addSearchPath (search_pathes.asString(i));
		}
		catch(EUnknownVar &)
		{}

		try
		{
			// Add recusrive search pathes
			CConfigFile::CVar &recursive_search_pathes = cf.getVar ("recursive_search_pathes");
			for (uint i=0; i<(uint)recursive_search_pathes.size(); i++)
				CPath::addSearchPath (recursive_search_pathes.asString(i), true, false);
		}
		catch(EUnknownVar &)
		{}

		// Add extension remapping
		try
		{
			CConfigFile::CVar &extensions_remapping = cf.getVar ("extensions_remapping");
			if (extensions_remapping.size()%2 != 0)
			{
				nlwarning ("extensions_remapping must have a multiple of 2 entries (ex: extensions_remapping={\"dds\",\"tga\"};)");
			}
			else
			{
				for (uint i=0; i<(uint)extensions_remapping.size(); i+=2)
					CPath::remapExtension(extensions_remapping.asString(i), extensions_remapping.asString(i+1), true);
			}
		}
		catch (EUnknownVar &)
		{
		}
	
		// debug, display path
		//CPath::display();

		// set the sound banks and sample banks
		try
		{
/*			CConfigFile::CVar &var = cf.getVar("sound_path");
			string soundPath = var.asString();

			var = cf.getVar("soundbanks");
			for (uint i=0; i<(uint)var.size(); i++)
			{
				string dir = soundPath;
				dir.append("/").append(var.asString(i).c_str());
				CSoundSystem::addSoundBank(dir);
			}
*/
			{
				CConfigFile::CVar &var = cf.getVar("sample_path");
				string samplePath(var.asString());
				CSoundSystem::setSamplePath(samplePath);
			}

			{
				CConfigFile::CVar &var = cf.getVar("packed_sheet_path");
				string packedSheetPath(var.asString());
				CSoundSystem::setPackedSheetPath(packedSheetPath);
			}

			/*CConfigFile::CVar var = cf.getVar("samplebanks");
			for (uint i=0; i<(uint)var.size(); i++)
				CSoundSystem::addSampleBank(var.asString(i).c_str());*/
		}
		catch (EUnknownVar &)
		{
			//::MessageBox(NULL, "warning : 'sample_path' or 'packed_sheet_path' variable not defined.\nSound will not work properly.", "Objectviewer.cfg", MB_OK|MB_ICONEXCLAMATION);
		}

		// load the camera focal
		try
		{
			CConfigFile::CVar &camera_focal = cf.getVar("camera_focal");
			_CameraFocal = camera_focal.asFloat();
		}
		catch (EUnknownVar &)
		{
		}


		// load Scene light setup.
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_enabled");
			_SceneLightEnabled = var.asInt() !=0 ;
		}
		catch (EUnknownVar &)
		{
			_SceneLightEnabled= false;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_ambiant");
			_SceneLightSunAmbiant.R = var.asInt(0);
			_SceneLightSunAmbiant.G = var.asInt(1);
			_SceneLightSunAmbiant.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunAmbiant= NLMISC::CRGBA::Black;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_diffuse");
			_SceneLightSunDiffuse.R = var.asInt(0);
			_SceneLightSunDiffuse.G = var.asInt(1);
			_SceneLightSunDiffuse.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunDiffuse= NLMISC::CRGBA::White;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_specular");
			_SceneLightSunSpecular.R = var.asInt(0);
			_SceneLightSunSpecular.G = var.asInt(1);
			_SceneLightSunSpecular.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunSpecular= NLMISC::CRGBA::White;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_dir");
			_SceneLightSunDir.x = var.asFloat(0);
			_SceneLightSunDir.y = var.asFloat(1);
			_SceneLightSunDir.z = var.asFloat(2);
			_SceneLightSunDir.normalize();
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunDir.set(0, 1, -1);
			_SceneLightSunDir.normalize();
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("object_light_test");
			_ObjectLightTestShape= var.asString();
		}
		catch (EUnknownVar &)
		{
		}		

		// Load vegetable Landscape cfg.
		loadVegetableLandscapeCfg(cf);

		// load automatfic animations
		try
		{
			CConfigFile::CVar &var = cf.getVar("automatic_animation_path");
			std::auto_ptr<CAnimationSet> as(new CAnimationSet);
			//
			bool loadingOk = as->loadFromFiles(var.asString(),true ,"anim",true);	
			//
			if (!loadingOk)
			{
				//::MessageBox(NULL, "Warning : Unable to load all automatic animation", "Error", MB_OK | MB_ICONEXCLAMATION);
				nlwarning("Unable to load all automatic animation");
			}
			CNELU::Scene->setAutomaticAnimationSet(as.release());
		}
		catch (EUnknownVar &)
		{
			//::MessageBox(NULL, "No automatic animation path specified, please set 'automatic_animation_path'", "warning", MB_OK);
			nlwarning("No automatic animation path specified");
		}

		// load CharacterScalePos
		try
		{
			CConfigFile::CVar &var = cf.getVar("character_scale_pos");
			_CharacterScalePos= var.asFloat();
		}
		catch (EUnknownVar &)
		{
		}		

		// Fog
		CConfigFile::CVar *var = cf.getVarPtr("fog");
		if (var)
			_Fog = var->asInt() != 0;
		var = cf.getVarPtr("fog_start");
		if (var)
			_FogStart = var->asFloat();
		var = cf.getVarPtr("fog_end");
		if (var)
			_FogEnd = var->asFloat();
		var = cf.getVarPtr("fog_color");
		if (var)
			_FogColor = CRGBA ((uint8)(var->asInt(0)), (uint8)(var->asInt(1)), (uint8)(var->asInt(2)));

		// Clouds
		_CSS.NbCloud = 0;
		if (var = cf.getVarPtr("cloud_count"))
			_CSS.NbCloud = var->asInt();
		if (var = cf.getVarPtr("cloud_diffuse"))
		{
			_CSS.Diffuse.R = var->asInt(0);
			_CSS.Diffuse.G = var->asInt(1);
			_CSS.Diffuse.B = var->asInt(2);
		}
		if (var = cf.getVarPtr("cloud_ambient"))
		{
			_CSS.Ambient.R = var->asInt(0);
			_CSS.Ambient.G = var->asInt(1);
			_CSS.Ambient.B = var->asInt(2);
		}
		if (var = cf.getVarPtr("cloud_speed"))
			_CSS.CloudSpeed = var->asFloat();
		if (var = cf.getVarPtr("cloud_update_period"))
			_CSS.TimeToChange = var->asFloat();
		if (var = cf.getVarPtr("cloud_wind_speed"))
			_CSS.WindSpeed = var->asFloat();
	}
	catch (Exception& e)
	{
		::MessageBox (NULL, e.what(), "Objectviewer.cfg", MB_OK|MB_ICONEXCLAMATION);
	}
}


// ***************************************************************************
// properly remove a single window
static void removeWindow(CWnd *wnd)
{
	if (!wnd) return;
	wnd->DestroyWindow();
	delete wnd;
}

// ***************************************************************************
CObjectViewer::~CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	removeWindow(_MainFrame);			
	removeWindow(_SlotDlg);	
	removeWindow(_AnimationSetDlg);	
	removeWindow(_AnimationDlg);	
	removeWindow(_DayNightDlg);	
	removeWindow(_WaterPoolDlg);	
	removeWindow(_SoundAnimDlg);	
	removeWindow(_LightGroupDlg);	
	removeWindow(_ChooseBGColorDlg);	
	removeWindow(_VegetableDlg);	
	removeWindow(_GlobalWindDlg);	
	removeWindow(_SkeletonScaleDlg);	
	removeWindow(_TuneMRMDlg);	
	delete _FontGenerator;	
}

// ***************************************************************************

void CObjectViewer::initCamera ()
{
	// Camera
	CFrustum frustrum;
	uint32 width, height;
	CNELU::Driver->getWindowSize (width, height);
	frustrum.initPerspective( _CameraFocal *(float)Pi/180.f, height != 0 ? (float)width/(float)height : 0.f, 0.1f, 1000.f);
	CNELU::Camera->setFrustum (frustrum);

	// Others camera
	uint i;
	for (i=0; i<_Cameras.size (); i++)
	{
		frustrum.initPerspective( _ListInstance[_Cameras[i]]->Camera->getFov(), height != 0 ? (float)width/(float)height : 0.f, 0.1f, 1000.f);
		_ListInstance[_Cameras[i]]->Camera->setFrustum (frustrum);
	}
}

// ***************************************************************************

bool CObjectViewer::initUI (HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	

	// initialize NeL context if needed
	if (!NLMISC::INelContext::isContextInitialised())
		new NLMISC::CApplicationContext;

	// The fonts manager
	_FontManager.setMaxMemory(2000000);

	// The windows path
	uint dSize = ::GetWindowsDirectory(NULL, 0);
	nlverify(dSize);
	char *wd = new char[dSize];	
	nlverify(::GetWindowsDirectory(wd, dSize));
	_FontPath=wd;
	_FontPath+="\\fonts\\arial.ttf";

	// The font generator
	_FontGenerator = new NL3D::CFontGenerator ( _FontPath );
	delete[] wd;

	// The viewport
	CViewport viewport;

	// Create the icon
	HICON hIcon = (HICON)LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
		16, 16, 0);
	
	// load name of the driver from the config file
	loadDriverName();

	// Create a doomy driver
	IDriver *driver= _Direct3d?CDRU::createD3DDriver():CDRU::createGlDriver();

	// Get parent window
	CWnd parentWnd;
	CWnd *parentWndPtr=NULL;
	if (parent)
	{
		parentWnd.Attach (parent);
		parentWndPtr=&parentWnd;
	}

	// Create the main frame
	_MainFrame = new CMainFrame (this, (winProc) driver->getWindowProc());

	// Read register
	_MainFrame->registerValue (true);

	// Create the window
	_MainFrame->CFrameWnd::Create (AfxRegisterWndClass(0, 0, NULL, hIcon), 
		"NeL object viewer", 0x00cfc000, /*WS_OVERLAPPEDWINDOW,*/ CFrameWnd::rectDefault, parentWndPtr,
		MAKEINTRESOURCE(IDR_OBJECT_VIEWER_MENU), 0x00000300 /*WS_EX_ACCEPTFILES*/ /*|WS_EX_CLIENTEDGE*/);

	// Detach the hwnd
	parentWnd.Detach ();

	// Delete doomy driver
	delete driver;

	// Create a cwnd
	getRegisterWindowState (_MainFrame, REGKEY_OBJ_VIEW_OPENGL_WND, true);
	_MainFrame->ActivateFrame ();
	_MainFrame->ShowWindow (SW_SHOW);
	_MainFrame->UpdateWindow();

	// Context to open a view
	CCreateContext context;
	context.m_pCurrentDoc=NULL;
	context.m_pCurrentFrame=_MainFrame;
	context.m_pLastView=NULL;
	context.m_pNewDocTemplate=NULL;
	context.m_pNewViewClass=RUNTIME_CLASS(CObjView);

	// Create a view
	CObjView *view = (CObjView*)_MainFrame->CreateView (&context);
	view->ShowWindow (SW_SHOW);
	_MainFrame->SetActiveView(view);
	view = (CObjView*)_MainFrame->GetActiveView();
	view->MainFrame = _MainFrame;

	_MainFrame->ShowWindow (SW_SHOW);
	
	// Init NELU
	if (!CNELU::init (640, 480, viewport, 32, true, view->m_hWnd, false, _Direct3d))
	{
		return false;
	}	
	//CNELU::init (640, 480, viewport, 32, true, _MainFrame->m_hWnd);
	CNELU::Scene->setPolygonBalancingMode(CScene::PolygonBalancingClamp);

	// load the config file
	loadConfigFile();

	// Set the fog
	CNELU::Driver->enableFog (_Fog);
	CNELU::Driver->setupFog (_FogStart, _FogEnd, _FogColor);

	// init sound	
	CSoundSystem::initSoundSystem ();

	// Create a root.
	_SceneRoot= (CTransform*)CNELU::Scene->createModel(NL3D::TransformId);

	// Init default lighting seutp.
	setupSceneLightingSystem(_SceneLightEnabled, _SceneLightSunDir, _SceneLightSunAmbiant, _SceneLightSunDiffuse, _SceneLightSunSpecular);

	// Camera
	initCamera ();

	_MainFrame->OnResetCamera();

	// Create animation set dialog
	_AnimationSetDlg=new CAnimationSetDlg (this, _MainFrame);
	_AnimationSetDlg->Create (IDD_ANIMATION_SET);
	getRegisterWindowState (_AnimationSetDlg, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG, false);

	// Create animation set dialog
	_AnimationDlg=new CAnimationDlg (this, _MainFrame);
	_AnimationDlg->Create (IDD_ANIMATION);
	getRegisterWindowState (_AnimationDlg, REGKEY_OBJ_VIEW_ANIMATION_DLG, false);

	// Create the main dialog
	_SlotDlg=new CMainDlg (this, _MainFrame);
	_SlotDlg->Create (IDD_MAIN_DLG);
	getRegisterWindowState (_SlotDlg, REGKEY_OBJ_VIEW_SLOT_DLG, false);

	// Create particle dialog
	_ParticleDlg=new CParticleDlg (this, _MainFrame, _MainFrame, _AnimationDlg);
	_ParticleDlg->Create (IDD_PARTICLE);
	getRegisterWindowState (_ParticleDlg, REGKEY_OBJ_PARTICLE_DLG, false);

	// Create water pool editor dialog
	_WaterPoolDlg = new CWaterPoolEditor(_Wpm, _MainFrame);
	_WaterPoolDlg->Create (IDD_WATER_POOL);
	getRegisterWindowState (_WaterPoolDlg, REGKEY_OBJ_WATERPOOL_DLG, false);

	// Create day night dialog
	_DayNightDlg = new CDayNightDlg (this, _MainFrame);
	_DayNightDlg->Create (IDD_DAYNIGHT);
	getRegisterWindowState (_DayNightDlg, REGKEY_OBJ_DAYNIGHT_DLG, false);

	// Create vegetable dialog
	_VegetableDlg=new CVegetableDlg (this, _MainFrame);
	_VegetableDlg->Create (IDD_VEGETABLE_DLG);
	getRegisterWindowState (_VegetableDlg, REGKEY_OBJ_VIEW_VEGETABLE_DLG, false);

	// Create global wind dialog
	_GlobalWindDlg= new CGlobalWindDlg (this, _MainFrame);
	_GlobalWindDlg->Create(IDD_GLOBAL_WIND);
	getRegisterWindowState (_GlobalWindDlg, REGKEY_OBJ_GLOBAL_WIND_DLG, false);

	// Create sound animation editor dialog
	_SoundAnimDlg = new CSoundAnimDlg(this, _AnimationDlg, _MainFrame);
	_SoundAnimDlg->Create (IDD_SOUND_ANIM_DLG, _MainFrame);
	getRegisterWindowState (_SoundAnimDlg, REGKEY_OBJ_SOUND_ANIM_DLG, false);

	// Create light group editor dialog
	_LightGroupDlg = new CLightGroupFactor(_MainFrame);
	_LightGroupDlg->Create (IDD_LIGHT_GROUP_FACTOR, _MainFrame);
	getRegisterWindowState (_LightGroupDlg, REGKEY_OBJ_LIGHT_GROUP_DLG, false);

	// Create frame delay window
	_ChooseFrameDelayDlg = new CChooseFrameDelay(this, _MainFrame);
	_ChooseFrameDelayDlg->Create(IDD_CHOOSE_FRAME_DELAY, _MainFrame);
	getRegisterWindowState (_ChooseFrameDelayDlg, REGKEY_CHOOSE_FRAME_DELAY_DLG, false);

	// Set backgroupnd color
	setBackGroundColor(_MainFrame->BgColor);

	// Create bg color window (must create after the background color has been set)
	_ChooseBGColorDlg = new CChooseBGColorDlg(this, _MainFrame);
	_ChooseBGColorDlg->Create(IDD_CHOOSE_BG_COLOR, _MainFrame);
	getRegisterWindowState (_ChooseBGColorDlg, REGKEY_CHOOSE_BG_COLOR_DLG, false);

	// Create bg color window (must create after the background color has been set)
	_ChooseSunColorDlg = new CChooseSunColorDlg(CNELU::Scene, _MainFrame);
	_ChooseSunColorDlg->Create(IDD_CHOOSE_SUN_COLOR, _MainFrame);
	getRegisterWindowState (_ChooseSunColorDlg, REGKEY_CHOOSE_SUN_COLOR_DLG, false);

	// Create skeleton scale dlg
	_SkeletonScaleDlg = new CSkeletonScaleDlg(this, _MainFrame);
	_SkeletonScaleDlg->Create(IDD_SKELETON_SCALE_DLG, _MainFrame);
	getRegisterWindowState (_SkeletonScaleDlg, REGKEY_SKELETON_SCALE_DLG, false);

	// Create tune mrm dlg
	_TuneMRMDlg = new CTuneMrmDlg(this, CNELU::Scene, _MainFrame);
	_TuneMRMDlg->Create(IDD_TUNE_MRM_DLG, _MainFrame);
	getRegisterWindowState (_TuneMRMDlg, REGKEY_TUNE_MRM_DLG, false);
	
	
	_MainFrame->update ();
	

	// Set current frame
	setAnimTime (0.f, 100.f);

	// Add mouse listener to event server
	_MouseListener.addToServer(CNELU::EventServer);
		
	CNELU::Driver->activate ();

	// Enable sum of vram
	CNELU::Driver->enableUsedTextureMemorySum ();

	char sModulePath[256];
	// load the scheme bank if one is present		
	CIFile iF;
	::_makepath (sModulePath, SDrive, SDir, "default", ".scb");		
	if (iF.open(sModulePath))
	{
		try
		{
			iF.serial(SchemeManager);
		}
		catch (NLMISC::EStream &e)
		{
			::MessageBox(NULL, ("Unable to load the default scheme bank file : "  + std::string(e.what())).c_str(), "Object Viewer", MB_ICONEXCLAMATION);
		}
	}
	iF.close();
	
	// try to load a default config file for the viewer (for anitmation and particle edition setup)
	::_makepath (sModulePath, SDrive, SDir, "default", ".ovcgf");
	if (iF.open (sModulePath))
	{
		try
		{
			serial (iF);
		}
		catch (Exception& e)
		{
			::MessageBox (NULL, (std::string("error while loading default.ovcgf : ") + e.what()).c_str(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}

	// Create the cloud scape
	_CS = new CCloudScape(CNELU::Driver);
	_CS->init (&_CSS);	

	return true;
}

// ***************************************************************************

void CObjectViewer::addTransformation (CMatrix &current, CAnimation *anim, float begin, float end, ITrack *posTrack, ITrack *rotquatTrack, 
									   ITrack *nextPosTrack, ITrack *nextRotquatTrack, bool removeLast)
{
	// In place ?
	if (_AnimationDlg->Inplace)
	{
		// Just identity
		current.identity();
	}
	else
	{
		// Remove the start of the animation
		CQuat rotEnd (0,0,0,1);
		CVector posEnd (0,0,0);
		if (rotquatTrack)
		{
			// Interpolate the rotation
			rotquatTrack->interpolate (end, rotEnd);
		}
		if (posTrack)
		{
			// Interpolate the position
			posTrack->interpolate (end, posEnd);
		}

		// Add the final rotation and position
		CMatrix tmp;
		tmp.identity ();
		tmp.setRot (rotEnd);
		tmp.setPos (posEnd);

		// Incremental ?
		if (_AnimationDlg->IncPos)
			current *= tmp;
		else
			current = tmp;

		if (removeLast)
		{
			CQuat rotStart (0,0,0,1);
			CVector posStart (0,0,0);
			if (nextRotquatTrack)
			{
				// Interpolate the rotation
				nextRotquatTrack->interpolate (begin, rotStart);
			}
			if (nextPosTrack)
			{
				// Interpolate the position
				nextPosTrack->interpolate (begin, posStart);
			}
			// Remove the init rotation and position of the next animation
			tmp.identity ();
			tmp.setRot (rotStart);
			tmp.setPos (posStart);
			tmp.invert ();
			current *= tmp;

			// Normalize the mt
			CVector I = current.getI ();
			CVector J = current.getJ ();
			I.z = 0;
			J.z = 0;
			J.normalize ();
			CVector K = I^J;
			K.normalize ();
			I = J^K;
			I.normalize ();
			tmp.setRot (I, J, K);
			tmp.setPos (current.getPos ());
			current = tmp;
		}
	}
}

// ***************************************************************************

void CObjectViewer::setupPlaylist (float time)
{
	// Update animation dlg

	// Gor each object
	uint i;
	for (i=0; i<_ListInstance.size(); i++)
	{
		// Empty with playlist
		uint j;
		for (j=0; j<CChannelMixer::NumAnimationSlot; j++)
		{
			// Empty slot
			_ListInstance[i]->Playlist.setAnimation (j, CAnimationPlaylist::empty);
		}

		// With channel mixer ?
		if (_AnimationSetDlg->UseMixer)
		{
			// Setup from slots
			_ListInstance[i]->setAnimationPlaylist (getFrameRate ());

			// A playlist
			_ListInstance[i]->Playlist.setupMixer (_ListInstance[i]->ChannelMixer, _AnimationDlg->getTime());
		}
		else
		{			
			// Some animation in the list ?
			if (_ListInstance[i]->Saved.PlayList.size()>0)
			{
				// Index choosed
				uint choosedIndex = 0xffffffff;

				// Track here
				bool there = false;

				// Current matrix
				CMatrix	current;
				current.identity ();

				// Current animation
				CAnimation *anim = NULL;
				ITrack *posTrack = NULL;
				ITrack *rotquatTrack = NULL;

				// Try channel animationset
				anim = _ListInstance[i]->AnimationSet.getAnimation (_ListInstance[i]->AnimationSet.getAnimationIdByName (_ListInstance[i]->Saved.PlayList[0]));
				if (anim)
				{
					posTrack = (ITrack *)anim->getTrackByName ("pos");
					rotquatTrack = (ITrack *)anim->getTrackByName ("rotquat");
				}
				there = posTrack || rotquatTrack;

				// Accumul time
				float startTime=0;
				float endTime=anim->getEndTime()-anim->getBeginTime();

				// Animation index
				uint index = 0;

				// Get animation used in the list
				while (time>=endTime)
				{
					// Next animation
					index++;
					if (index<_ListInstance[i]->Saved.PlayList.size())
					{
						// Pointer on the animation
						CAnimation *newAnim=_ListInstance[i]->AnimationSet.getAnimation (_ListInstance[i]->AnimationSet.getAnimationIdByName(_ListInstance[i]->Saved.PlayList[index]));
						ITrack *newPosTrack = (ITrack *)newAnim->getTrackByName ("pos");
						ITrack *newRotquatTrack = (ITrack *)newAnim->getTrackByName ("rotquat");

						// Add the transformation
						addTransformation (current, anim, newAnim->getBeginTime(), anim->getEndTime(), posTrack, rotquatTrack, newPosTrack, newRotquatTrack, true);

						// Pointer on the animation
						anim = newAnim;
						posTrack = newPosTrack;
						rotquatTrack = newRotquatTrack;

						// Add start time
						startTime = endTime;
						endTime = startTime + anim->getEndTime()-anim->getBeginTime();

					}
					else
					{
						// Add the transformation
						addTransformation (current, anim, 0, anim->getEndTime(), posTrack, rotquatTrack, NULL, NULL, false);

						break;
					}
				}

				// Time cropped ?
				if (index>=_ListInstance[i]->Saved.PlayList.size())
				{
					// Yes
					index--;

					// Good index
					choosedIndex = _ListInstance[i]->AnimationSet.getAnimationIdByName (_ListInstance[i]->Saved.PlayList[index]);
					anim=_ListInstance[i]->AnimationSet.getAnimation (choosedIndex);

					// End time for last anim
					startTime = anim->getEndTime () - time;
				}
				else
				{
					// No

					// Add the transformation
					addTransformation (current, anim, 0, anim->getBeginTime() + time - startTime, posTrack, rotquatTrack, NULL, NULL, false);

					// Good index
					choosedIndex = _ListInstance[i]->AnimationSet.getAnimationIdByName (_ListInstance[i]->Saved.PlayList[index]);				

					// Get the animation
					anim=_ListInstance[i]->AnimationSet.getAnimation (choosedIndex);

					// Final time
					startTime -= anim->getBeginTime ();
				}

				// Set the slot		
				_ListInstance[i]->Playlist.setTimeOrigin (0, startTime);
				_ListInstance[i]->Playlist.setWrapMode (0, CAnimationPlaylist::Clamp);
				_ListInstance[i]->Playlist.setStartWeight (0, 1, 0);
				_ListInstance[i]->Playlist.setEndWeight (0, 1, 1);
				_ListInstance[i]->Playlist.setAnimation (0, choosedIndex);

				// Setup the channel
				_ListInstance[i]->Playlist.setupMixer (_ListInstance[i]->ChannelMixer, _AnimationDlg->getTime());

				// Setup the pos and rot for this shape
				if (there)
				{
					CVector		pos= current.getPos();
					// Get a skeleton model
					CSkeletonModel *skelModel=dynamic_cast<CSkeletonModel*>(_ListInstance[i]->TransformShape);
					// If a  skeleton model
					if(skelModel)
					{
						// scale animated pos value with the CFG scale
						pos*= _CharacterScalePos;
					}

					if (_ListInstance[i]->TransformShape)
					{
						_ListInstance[i]->TransformShape->setPos (pos);
						_ListInstance[i]->TransformShape->setRotQuat (current.getRot());
					}
					if (_ListInstance[i]->Camera)
					{
						_ListInstance[i]->Camera->setPos (pos);
						_ListInstance[i]->Camera->setRotQuat (current.getRot());
					}
				}
			}
			else
			{
				if (_ListInstance[i]->TransformShape)
				{
					CMeshBase *meshBase = dynamic_cast<CMeshBase *> ((IShape*)_ListInstance[i]->TransformShape->Shape);
					if (meshBase)
					{
						_ListInstance[i]->TransformShape->setPos (meshBase->getDefaultPos ()->getDefaultValue ());
						_ListInstance[i]->TransformShape->setRotQuat (meshBase->getDefaultRotQuat ()->getDefaultValue ());
						_ListInstance[i]->TransformShape->setScale (meshBase->getDefaultScale ()->getDefaultValue ());
					}
					else
					{
						/*_ListInstance[i]->TransformShape->setPos (CVector::Null);
						_ListInstance[i]->TransformShape->setRotQuat (CQuat::Identity);
						_ListInstance[i]->TransformShape->setScale (1, 1, 1);*/
					}
				}
				if (_ListInstance[i]->Camera)
				{
					_ListInstance[i]->Camera->setPos (_ListInstance[i]->Camera->getDefaultPos ()->getDefaultValue ());
					_ListInstance[i]->Camera->setTargetPos (_ListInstance[i]->Camera->getDefaultTargetPos ()->getDefaultValue ());
				}
			}
		}
	}
}


// tool funct : this if a window has another window as a parent (recursive)
// if parent == true, this return true
static bool isParentWnd(HWND parent, HWND son)
{
	if (parent == son) return true;
	HWND directParent = GetParent (son);
	if (!directParent) return false;
	if (directParent == parent) return true;
	return isParentWnd(parent, directParent);
}


// ***************************************************************************
void CObjectViewer::go ()
{
	nlassert(!_InstanceRunning); // this shouldn't be called if an instance of the viewer is running.
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_InstanceRunning = true;
	CGraph graph("ms", 10, 10, 200, 100, CRGBA(64, 64, 64), 20, 200);

	do
	{		
		_CrtCheckMemory();
		if (isParentWnd(_MainFrame->m_hWnd, GetForegroundWindow()))
		{
 			CNELU::Driver->activate ();			

			// Handle animation
			_AnimationDlg->handle ();

			// Handle sound animation
			_SoundAnimDlg->handle ();

			// Handle sound animation
			_LightGroupDlg->handle ();

			// Setup the channel mixer
			_AnimationSetDlg->UpdateData ();

			// Setup the play list
			setupPlaylist (_AnimationDlg->getTime());

			// Eval sound tracks
			evalSoundTrack (_AnimationDlg->getLastTime(), _AnimationDlg->getTime());

			// Animate the automatic animation in the scene
			//CNELU::Scene->animate( (float) + NLMISC::CTime::ticksToSecond( NLMISC::CTime::getPerformanceTime() ) );

			
			// Eval channel mixer for transform
			for (uint i=0; i<_ListInstance.size(); i++)
				
			_ListInstance[i]->ChannelMixer.eval (false);

			animateCNELUScene (_CS);			

			// Clear the buffers
			CNELU::clearBuffers(_BackGroundColor);			

			
			//if (_CS) _CS->setDebugQuad(true);

			// call of callback list
			{
				std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end());
				for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end(); ++it)
				{
					(*it)->goPreRender();
				}
			}
			// Render the CS
			if (_CS) _CS->render ();			
			
			// Draw the scene		
			CNELU::Scene->render();	
			
			// call of callback list
			{
				std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end());
				for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end(); ++it)
				{
					(*it)->goPostRender();
				}
			}
			
			if (_OcclusionTestMeshsVisible)
			{				
				CNELU::Scene->renderOcclusionTestMeshs();
			}

			// Profile polygon count
			CPrimitiveProfile in, out;
			CNELU::Driver->profileRenderedPrimitives (in, out);

			// Draw the hotSpot
			if (_MainFrame->MoveMode == CMainFrame::ObjectMode)
			{
				float radius=_HotSpotSize/2.f;
				CNELU::Driver->setupModelMatrix (CMatrix::Identity);
				CDRU::drawLine (_MouseListener.getHotSpot()+CVector (radius, 0, 0), _MouseListener.getHotSpot()+CVector (-radius, 0, 0), _HotSpotColor, *CNELU::Driver);
				CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, radius, 0), _MouseListener.getHotSpot()+CVector (0, -radius, 0), _HotSpotColor, *CNELU::Driver);
				CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, 0, radius), _MouseListener.getHotSpot()+CVector (0, 0, -radius), _HotSpotColor, *CNELU::Driver);
			}

			// Test some keys
			if (CNELU::AsyncListener.isKeyPushed(KeyF3))
			{
				// Change render mode
				switch (CNELU::Driver->getPolygonMode())
				{
				case IDriver::Filled:
					CNELU::Driver->setPolygonMode (IDriver::Line);
					break;
				case IDriver::Line:
					CNELU::Driver->setPolygonMode (IDriver::Point);
					break;
				case IDriver::Point:
					CNELU::Driver->setPolygonMode (IDriver::Filled);
					break;
				}
			}

			// draw various matrix
			if (_FXMatrixVisible) drawFXMatrix();
			if (_FXUserMatrixVisible) drawFXUserMatrix();
			if (_SceneMatrixVisible) drawSceneMatrix();			
			
			// draw Skeleton Scale Dlg selection
			if(_SkeletonScaleDlg)
				_SkeletonScaleDlg->drawSelection();

			// Test Window Keys
			bool	keyWndOk= false;
			if (CNELU::AsyncListener.isKeyPushed(Key1))
				_MainFrame->OnWindowAnimation(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key2))
				_MainFrame->OnWindowAnimationset(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key3))
				_MainFrame->OnWindowMixersslots(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key4))
				_MainFrame->OnWindowParticles(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key5))
				_MainFrame->OnWindowDayNight(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key6))
				_MainFrame->OnWindowWaterPool(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key7))
				_MainFrame->OnWindowVegetable(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key8))
				_MainFrame->OnWindowGlobalwind(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(Key9))
				_MainFrame->OnWindowSoundAnim(), keyWndOk= true;
			if (CNELU::AsyncListener.isKeyPushed(KeyO))
				_MainFrame->OnFileOpen(), keyWndOk= true;

			// Reload texture ?
			if (CNELU::AsyncListener.isKeyPushed(KeyR))
				_MainFrame->OnReloadTextures();

			// If some window activated, reset the focus to the main wnd.
			if(keyWndOk)
				_MainFrame->SetActiveWindow();

			// Calc FPS
			static sint64 lastTime=NLMISC::CTime::getPerformanceTime ();
			sint64 newTime=NLMISC::CTime::getPerformanceTime ();
			sint64 timeDiff = newTime - lastTime;
			float fps = timeDiff > 0 ? (float)(1.0 / NLMISC::CTime::ticksToSecond (newTime-lastTime)) : 1000.0f;
			lastTime=newTime;
			char msgBar[1024];
			uint nbPlayingSources, nbSources;
			if (CSoundSystem::getAudioMixer())
			{
				nbPlayingSources = CSoundSystem::getAudioMixer()->getUsedTracksCount();
				nbSources = CSoundSystem::getAudioMixer()->getPlayingSourcesCount();
			}
			else
			{
				nbPlayingSources = nbSources = NULL;
			}

			// Display std info.
			sprintf (msgBar, "%s - Nb tri: %d -Texture used (Mo): %5.2f - Texture allocated (Mo): %5.2f - Distance: %5.0f - Sounds: %d/%d - Fps: %03.1f",
				_Direct3d?"Direct3d":"OpenGL",
							in.NLines+in.NPoints+in.NQuads*2+in.NTriangles+in.NTriangleStrips, (float)CNELU::Driver->getUsedTextureMemory () / (float)(1024*1024), 
							(float)CNELU::Driver->profileAllocatedTextureMemory () / (float)(1024*1024), 
							(_SceneCenter-CNELU::Camera->getMatrix().getPos()).norm(),						 
							nbPlayingSources,
							nbSources,
							fps
							);
			// Display
			_MainFrame->StatusBar.SetWindowText (msgBar);

			// Display Vegetable info.
			if(_VegetableDlg!=NULL)
			{
				if(_VegetableLandscape != NULL)
				{
					char vegetMsgBar[1024];
					sprintf (vegetMsgBar, "%d", _VegetableLandscape->Landscape.getNumVegetableFaceRendered());
					_VegetableDlg->StaticPolyCount.SetWindowText(vegetMsgBar);
				}
				else
				{
					_VegetableDlg->StaticPolyCount.SetWindowText("0");
				}
			}

			// graph disabled for now..
			// graph.addValue(CNELU::Scene->getEllapsedTime() * 1000.f);
			// graph.renderGraph();

			// Swap the buffers
			CNELU::swapBuffers();

			// Select the good camera
			if (_MainFrame->MoveMode == CMainFrame::CameraMode)
			{
				sint cameraId = getCurrentCamera ();
				if (cameraId != -1)
				{
					CInstanceInfo *info = getInstance(getCameraInstance (cameraId));
					nlassert (info->Camera);
					CNELU::Scene->setCam (info->Camera);
				}
			}
			else
			{
				CNELU::Scene->setCam (CNELU::Camera);
			}

			if (_MainFrame->MoveMode == CMainFrame::ObjectMode)
			{
				_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
			}
			else
			{
				_MouseListener.setMouseMode (CEvent3dMouseListener::firstPerson);
				_MouseListener.setSpeed (_MainFrame->MoveSpeed);
			}

		

			// Reset camera aspect ratio
			initCamera ();

			if (_MainFrame->isMoveElement())
			{
				// for now we apply a transform on the selected object in the particle system			
				_ParticleDlg->moveElement(_MouseListener.getModelMatrix());
			}
			else if (_MainFrame->isMoveFX())
			{
				_ParticleDlg->setPSWorldMatrix(_MouseListener.getModelMatrix());
			}
			else if (_MainFrame->isMoveFXUserMatrix())
			{
				setFXUserMatrix(_MouseListener.getModelMatrix());
			}
			else if (_MainFrame->isMoveObjectLightTest())
			{
				_ObjectLightTestMatrix= _MouseListener.getModelMatrix();
			}
			else if (_MainFrame->isMoveSceneRoot())
			{
				_SceneRoot->setTransformMode (ITransformable::DirectMatrix);
				_SceneRoot->setMatrix (_MouseListener.getModelMatrix());
			}
			else
			{
				nlassert(_MainFrame->isMoveCamera());

				// New matrix from camera
				CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
				CNELU::Camera->setMatrix (_MouseListener.getViewMatrix());

				// Vegetable: manage collision snapping if wanted and possible
				if(_VegetableSnapToGround && _VegetableLandscape)
				{
					// get matrix from camera.
					CMatrix	matrix= CNELU::Camera->getMatrix();
					// snap To ground.
					CVector	pos= matrix.getPos();
					// if succes to snap to ground
					if(_VegetableCollisionEntity->snapToGround(pos))
					{
						pos.z+= _VegetableSnapHeight;
						matrix.setPos(pos);
						// reset the moveListener and the camera.
						_MouseListener.setMatrix(matrix);
						CNELU::Camera->setMatrix(matrix);
					}
				}
			}


			// Update lighting test Dynamic object position
			if(_ObjectLightTest && _GlobalRetriever)
			{
				// Get the position of the object snapped.
				UGlobalPosition		gPos= _GlobalRetriever->retrievePosition(_ObjectLightTestMatrix.getPos());
				CVector	pos= _GlobalRetriever->getGlobalPosition(gPos);
				_ObjectLightTest->setPos(pos);
				_ObjectLightTest->setRotQuat(_ObjectLightTestMatrix.getRot());

				// Update the matrix and the mouseListener.
				if (_MainFrame->isMoveObjectLightTest())
				{
					_ObjectLightTestMatrix.setPos(pos);
					_MouseListener.setModelMatrix(_ObjectLightTestMatrix);
				}

				// Update the logicInfo so lighting is well computed.
				_ObjectLightTestLogicInfo.GPos= gPos;
			}

			// Pump message from the server
			CNELU::EventServer.pump();

			// Pump others message for the windows
			MSG	msg;
			while ( PeekMessage(&msg, NULL,0,0,PM_REMOVE) )
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			
			CSoundSystem::setListenerMatrix(_MouseListener.getViewMatrix());
			CSoundSystem::poll();

			

			// simulate frame delay
			if (_FrameDelay)
			{
				NLMISC::nlSleep(_FrameDelay);
			}


			// Save last time
			_LastTime=_AnimationDlg->getTime();
			theApp.OnIdle (0);
		}
		else
		{
			// Traditionnal message loop
			MSG	msg;
			while (GetMessage( &msg, NULL, 0, 0) == TRUE)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				if (!IsWindow (_MainFrame->m_hWnd))
					break;

				// Get the foreground window				
				if (isParentWnd(_MainFrame->m_hWnd, GetForegroundWindow()))
					break;
			}
		}		
	}
	while (!CNELU::AsyncListener.isKeyPushed(KeyESCAPE)&&CNELU::Driver->isActive());
	_InstanceRunning = false;
}

// ***************************************************************************

void CObjectViewer::releaseUI ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Release particles
	removeWindow(_ChooseFrameDelayDlg);	
	removeWindow(_ParticleDlg);	

	// release sound
	CSoundSystem::releaseSoundSystem();

	if (CNELU::Driver->isActive())
	{
		// register window position
		if (CNELU::Driver->getDisplay())
		{
			setRegisterWindowState (_MainFrame, REGKEY_OBJ_VIEW_OPENGL_WND);
		}
	}

	// Write register
	if (_MainFrame)
	{	
		_MainFrame->registerValue (false);
		// Remove the main frame
		if (::IsWindow(*_MainFrame))
		{
			_MainFrame->DestroyWindow();
		}
		delete _MainFrame;
		_MainFrame = NULL;
	}

	// Release the emitter from the server
	_MouseListener.removeFromServer (CNELU::EventServer);

	// exit

	// remove first possibly created collisions objects.
	if(_VegetableCollisionEntity)
	{
		_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
		_VegetableCollisionEntity= NULL;
	}
	if(_VegetableCollisionManager)
	{
		delete _VegetableCollisionManager;
		_VegetableCollisionManager= NULL;
	}

	// delete Landscape
	if(_VegetableLandscape)
	{
		CNELU::Scene->deleteModel(_VegetableLandscape);
		_VegetableLandscape= NULL;
	}

	

	// Release all instances and all Igs.
	removeAllInstancesFromScene();

	// Remove

	// Create the cloud scape
	delete _CS;
	_CS = NULL;

	// release Root
	CNELU::Scene->deleteModel(_SceneRoot);
	_SceneRoot= NULL;

	// release other 3D.
	CNELU::release();

	
}

// ***************************************************************************

void setRegisterWindowState (const CWnd *pWnd, const char* keyName)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, keyName, &hKey)==ERROR_SUCCESS)
	{
		RECT rect;
		pWnd->GetWindowRect (&rect);
		RegSetValueEx(hKey, "Left", 0, REG_DWORD, (LPBYTE)&rect.left, 4);
		RegSetValueEx(hKey, "Right", 0, REG_DWORD, (LPBYTE)&rect.right, 4);
		RegSetValueEx(hKey, "Top", 0, REG_DWORD, (LPBYTE)&rect.top, 4);
		RegSetValueEx(hKey, "Bottom", 0, REG_DWORD, (LPBYTE)&rect.bottom, 4);
	}
}

// ***************************************************************************

void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=4;
		DWORD type;
		RECT rect;
		RegQueryValueEx (hKey, "Left", 0, &type, (LPBYTE)&rect.left, &len);
		RegQueryValueEx (hKey, "Right", 0, &type, (LPBYTE)&rect.right, &len);
		RegQueryValueEx (hKey, "Top", 0, &type, (LPBYTE)&rect.top, &len);
		RegQueryValueEx (hKey, "Bottom", 0, &type, (LPBYTE)&rect.bottom, &len);

		// Set window pos
		pWnd->SetWindowPos (NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOOWNERZORDER|SWP_NOZORDER|
			(resize?0:SWP_NOSIZE));
	}
}

// ***************************************************************************

void CObjectViewer::setAnimTime (float animStart, float animEnd)
{
	// Dispatch the command
	_SlotDlg->setAnimTime (animStart, animEnd);
	_AnimationDlg->setAnimTime (animStart, animEnd);
	_SoundAnimDlg->setAnimTime (animStart, animEnd);
}

// ***************************************************************************


void CObjectViewer::resetSlots (uint instance)
{
	// Reset the animation set
	_ListInstance[instance]->AnimationSet.reset ();

	// Set no animation in slot UI
	for (uint j=0; j<NL3D::CChannelMixer::NumAnimationSlot; j++)
		_ListInstance[instance]->Saved.SlotInfo[j].Animation = "";

	// Reset the animation list
	_ListInstance[instance]->Saved.AnimationFileName.clear ();

	// Reset the play list
	_ListInstance[instance]->Saved.PlayList.clear ();

	// Reset the skeleton list
	_ListInstance[instance]->Saved.SWTFileName.clear ();

	// Update 
	_AnimationSetDlg->refresh (TRUE);
	_SlotDlg->refresh (TRUE);
	_SoundAnimDlg->refresh (TRUE);
}

// ***************************************************************************

void CObjectViewer::reinitChannels ()
{
	// Add all the instance in the channel mixer
	for (uint i=0; i<_ListInstance.size(); i++)
	{
		// Reset the channels
		_ListInstance[i]->ChannelMixer.resetChannels ();

		// Setup animation set
		_ListInstance[i]->ChannelMixer.setAnimationSet (&(_ListInstance[i]->AnimationSet));

		// Register the transform (but not if it has automatic animation, as a channel mixer has been created for us)
		bool autoAnim = false;
		if (dynamic_cast<CMeshBaseInstance *>(_ListInstance[i]->TransformShape))
		{
			CMeshBase *mb = NLMISC::safe_cast<CMeshBase *>( (IShape *) static_cast<CMeshBaseInstance *>(_ListInstance[i]->TransformShape)->Shape );
			autoAnim = mb->getAutoAnim();
		}

		if (!autoAnim)
		{
			if (_ListInstance[i]->TransformShape)
				_ListInstance[i]->TransformShape->registerToChannelMixer (&(_ListInstance[i]->ChannelMixer), "");
			if (_ListInstance[i]->Camera)
				_ListInstance[i]->Camera->registerToChannelMixer (&(_ListInstance[i]->ChannelMixer), "");
		}
	}

	// Enable / disable channels
	enableChannels ();
}

// ***************************************************************************

void CObjectViewer::enableChannels ()
{
	// Disable some channels
	_AnimationSetDlg->UpdateData ();
	bool enable = (_AnimationSetDlg->UseMixer == 1);

	// Add all the instance in the channel mixer
	for (uint i=0; i<_ListInstance.size(); i++)
	{
		// Get the pos and rot channel id
		uint posId = _ListInstance[i]->AnimationSet.getChannelIdByName ("pos");
		uint rotQuatId = _ListInstance[i]->AnimationSet.getChannelIdByName ("rotquat");
		uint rotEulerId = _ListInstance[i]->AnimationSet.getChannelIdByName ("roteuler");

		if (posId != CAnimationSet::NotFound)
			_ListInstance[i]->ChannelMixer.enableChannel (posId, enable);
		if (rotQuatId != CAnimationSet::NotFound)
			_ListInstance[i]->ChannelMixer.enableChannel (rotQuatId, enable);
		if (rotEulerId != CAnimationSet::NotFound)
			_ListInstance[i]->ChannelMixer.enableChannel (rotEulerId, enable);
	}
}

// ***************************************************************************

float CObjectViewer::getFrameRate ()
{
	return _AnimationDlg->Speed;
}

// ***************************************************************************

string getFilename (const string &file)
{
	// if the direct file exist, return it
	if (NLMISC::CFile::fileExists(file))
		return file;

	string path = NLMISC::CFile::getFilename(file);
	path = CPath::lookup (path, false, false, false);
	if (path.empty())
		path = file;
	return path;
}

// ***************************************************************************

void CObjectViewer::serial (NLMISC::IStream& f)
{
	// version 4: include particle workspace infos
	// serial "OBJV_CFG"
	f.serialCheck (NELID("VJBO"));
	f.serialCheck (NELID("GFC_"));

	// serial the version
	int ver=f.serialVersion (4);
	if (ver>=4)
	{
		f.serial(ParticleWorkspaceFilename);
	}
	if (ver>=3)
	{
		// Read the configuration file
		if (f.isReading())
		{
			if (ver <=3)
			{			
				ParticleWorkspaceFilename = "";
			}
			// First instance
			uint firstInstance = (uint)_ListInstance.size();

			// Read information
			std::vector<CInstanceSave> readed;
			f.serialCont (readed);

			// Merge
			for (uint i=0; i<readed.size(); i++)
			{
				try
				{
					// Instance loaded
					uint instance = 0xffffffff;

					if (readed[i].Camera)
					{
						instance = addCamera (readed[i].CameraInfo, readed[i].ShapeFilename.c_str());
					}
					else
					{
						// Load the shape
						CIFile input;
						string path = getFilename (readed[i].ShapeFilename);
						if (input.open (path))
						{
							// Serial a shape
							CShapeStream serialShape;
							serialShape.serial (input);

							// Is a skeleton ?
							if (readed[i].IsSkeleton)
							{
								// Add the skel
								instance = addSkel (serialShape.getShapePointer(), readed[i].ShapeFilename.c_str());
								SkeletonUsedForSound = instance;
							}
							else
							{
								// Add the mesh
								if (readed[i].SkeletonId != 0xffffffff)
									instance = addMesh (serialShape.getShapePointer(), readed[i].ShapeFilename.c_str(), readed[i].SkeletonId + firstInstance, (readed[i].BindBoneName=="")?NULL:readed[i].BindBoneName.c_str());
								else
									instance = addMesh (serialShape.getShapePointer(), readed[i].ShapeFilename.c_str(), 0xffffffff, (readed[i].BindBoneName=="")?NULL:readed[i].BindBoneName.c_str());
							}
						}
						else
						{
							// Error message
							char message[512];
							smprintf (message, 512, "File not found %s", readed[i].ShapeFilename.c_str());
							_MainFrame->MessageBox (message, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

							// Stop loading
							break;
						}
					}

					// Check instance number
 					nlassert (instance == (firstInstance+i));

					// Load animations
					for (uint anim=0; anim<readed[i].AnimationFileName.size(); anim++)
					{
						string path = getFilename (readed[i].AnimationFileName[anim]);
						loadAnimation (path.c_str(), instance);
					}

					// Load SWT
					for (uint swt=0; swt<readed[i].SWTFileName.size(); swt++)
					{
						string path = getFilename (readed[i].SWTFileName[swt]);
						loadSWT (path.c_str(), instance);
					}

					// Set the playlist
					_ListInstance[instance]->Saved.PlayList = readed[i].PlayList;

					// Set the slot information
					for (uint slot=0; slot<NL3D::CChannelMixer::NumAnimationSlot; slot++)
						_ListInstance[instance]->Saved.SlotInfo[slot] = readed[i].SlotInfo[slot];
				}
				catch (Exception &e)
				{
					// Error message
					char message[512];
					smprintf (message, 512, "Error loading shape %s: %s", readed[i].ShapeFilename.c_str(), e.what());
					_MainFrame->MessageBox (message, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

					// Stop loading
					break;
				}
			}

			// Init channels
			reinitChannels ();

			// Read the selection
			uint32 selection;
			f.serial (selection);
			if (selection+firstInstance < _ListInstance.size())
			{
				_SelectedObject = selection+firstInstance;
			}

			// Invalidate dialogs
			_AnimationSetDlg->refresh (TRUE);
			_SlotDlg->refresh (TRUE);
			_SoundAnimDlg->refresh (TRUE);
		}
		else
		{
			// Build information
			std::vector<CInstanceSave> readed (_ListInstance.size());
			for (uint instance=0; instance<_ListInstance.size(); instance++)
			{
				// Copy the save information
				readed[instance] = _ListInstance[instance]->Saved;
			}

			// Save the configuration
			f.serialCont (readed);

			// Write the selection
			f.serial (_SelectedObject);
		}
	}
}

// ***************************************************************************

bool CObjectViewer::loadInstanceGroup(const char *igFilename)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];

	// Add search path for the mesh
	_splitpath (igFilename, drive, dir, NULL, NULL);
	_makepath (path, drive, dir, NULL, NULL);
	CPath::addSearchPath (path);

	
	// Open a file
	CIFile file;
	if (file.open (igFilename))
	{		
		// Shape pointer
		NL3D::CInstanceGroup	*ig= new NL3D::CInstanceGroup;	

		try
		{
			// Stream it
			file.serial(*ig);

			// Append the ig.
			addInstanceGroup(ig);
		}
		catch (Exception& e)
		{
			// clean
			delete ig;
			_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			return false;
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", igFilename);
		_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	

	return true;		
}



// ***************************************************************************

bool CObjectViewer::loadMesh (std::vector<std::string> &meshFilename, const char* skeleton)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];

	// Add search path for the skeleton
	if (skeleton)
	{
		_splitpath (skeleton, drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		CPath::addSearchPath (path);
	}

	// Open a file
	CIFile file;

	// Shape pointer
	IShape *shapeSkel=NULL;
	uint skelIndex = 0xffffffff;
	NL3D::CSkeletonModel *transformSkel=NULL;

	// Skel error ?
	bool skelError=false;

	// Continue ?
	if (skeleton&&(strcmp (skeleton, "")!=0))
	{

		// Open a file
		if (file.open (skeleton))
		{
			// Sream a shape
			CShapeStream streamShape;
			try
			{
				// Stream it
				streamShape.serial (file);

				// Add the shape
				shapeSkel=streamShape.getShapePointer();
			}
			catch (Exception& e)
			{
				_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

				// error
				skelError=true;
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for reading.", meshFilename);
			_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

			// error
			skelError=true;
		}
	}

	// Skeleton error ?
	if (skelError)
		return false;

	// Skeleton used ?
	bool skelUsed = false;

	// Index of the shape
	uint lastShape = 0xffffffff;

	// For each meshes
	for (uint i=0; i<meshFilename.size(); i++)
	{
		// Filename
		const char *fileName = meshFilename[i].c_str();

		// Add search path for the mesh
		_splitpath (fileName, drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		CPath::addSearchPath (path);

		// Shape pointer
		IShape *shapeMesh=NULL;

		if (file.open (fileName))
		{
			// Sream a shape
			CShapeStream streamShape;
			try
			{
				// Stream it
				streamShape.serial (file);

				// Add the shape
				shapeMesh=streamShape.getShapePointer();
			}
			catch (Exception& e)
			{
				_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
				continue;
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
			_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			continue;
		}

		// Add the skel shape
		if (shapeSkel&&(!skelUsed))
		{
			// Add the skel
			skelIndex = addSkel (shapeSkel, skeleton);
			if (skelIndex != 0xffffffff)
			{
				skelUsed = true;
				transformSkel = dynamic_cast<CSkeletonModel*>(_ListInstance[skelIndex]->TransformShape);
				nlassert (transformSkel);
			}
		}

		// Add the skel shape
		if (shapeMesh)
		{
			// Get the object name
			lastShape = addMesh (shapeMesh, fileName, skelIndex);
		}
	}

	// Skel not used ?
	if ((!skelUsed)&&shapeSkel)
	{
		// Remove it
		delete shapeSkel;
	}

	// Select the skeleton
	if (skelIndex != 0xffffffff)
		_SelectedObject = skelIndex;
	else if (lastShape != 0xffffffff)
		_SelectedObject = lastShape;

	// Update windows
	_AnimationSetDlg->refresh (TRUE);
	_SlotDlg->refresh (TRUE);
	_SoundAnimDlg->refresh (TRUE);

	return true;
}

// ***************************************************************************

void CObjectViewer::resetCamera ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_MainFrame->OnResetCamera();
}

// ***************************************************************************

uint CObjectViewer::addMesh (NL3D::IShape* pMeshShape, const char* meshName, uint skelIndex, const char* bindSkelName, bool createInstance)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	if (CNELU::ShapeBank->getPresentState(meshName)!=CShapeBank::NotPresent)
	{
		delete pMeshShape;
	}
	else
		CNELU::ShapeBank->add (meshName, CSmartPtr<IShape> (pMeshShape));

	// Must create the instance?
	if(createInstance)
	{
		// Create a model and add it to the scene
		CTransformShape	*pTrShape=CNELU::Scene->createInstance (meshName);
		nlassert (pTrShape);

		// link to the root for manipulation
		_SceneRoot->hrcLinkSon(pTrShape);

		// Get the real shape used by the instance.
		pMeshShape= pTrShape->Shape;

		// Set the rot model
		if (_MainFrame->Euler)
			pTrShape->setTransformMode (ITransformable::RotEuler);
		else
			pTrShape->setTransformMode (ITransformable::RotQuat);

		// Store the transform shape pointer
		CInstanceInfo *iInfo = new CInstanceInfo;
		iInfo->TransformShape= pTrShape;

		// Store the name of the shape
		iInfo->MustDelete = true;
		iInfo->Saved.ShapeFilename = meshName;
		iInfo->Saved.SkeletonId = skelIndex;
		_ListInstance.push_back (iInfo);	

		// *** Bind to the skeleton

		// Get a mesh instance
		CMeshBaseInstance  *meshInstance=dynamic_cast<CMeshBaseInstance*>(pTrShape);

		// Bind the mesh
		if (skelIndex != 0xffffffff)
		{
			// Get the skeleton
			NL3D::CSkeletonModel *transformSkel = dynamic_cast<CSkeletonModel*>(_ListInstance[skelIndex]->TransformShape);
			nlassert (transformSkel);
			
			// It is a skinned mesh ?
			CMesh *mesh = dynamic_cast<CMesh *>(pMeshShape);
			CMeshMRM *meshMrm = dynamic_cast<CMeshMRM *>(pMeshShape);
			CMeshMRMSkinned *meshMrmSkinned = dynamic_cast<CMeshMRMSkinned *>(pMeshShape);
			if ( (mesh && mesh->getMeshGeom().isSkinned()) || 
				(meshMrm && meshMrm->getMeshGeom().isSkinned()) ||
				meshMrmSkinned)
			{
				// Bind to skeleton
				transformSkel->bindSkin (meshInstance);
			}
			else
			{
				// Bind bone name
				uint bindBone = 0xffffffff;
				std::string boneName = "";

				// Name is passed, look for bone 
				if (bindSkelName)
				{
					// Make a list of bones
					uint bone;
					for (bone=0; bone<transformSkel->Bones.size(); bone++)
					{
						if (transformSkel->Bones[bone].getBoneName() == bindSkelName)
						{
							bindBone = bone;
							boneName = bindSkelName;
							break;
						}
					}
				}

				// Found ? No, look for a bind bone
				if (bindBone == 0xffffffff)
				{
					// Make a list of bones
					vector<string> listBones;
					uint bone;
					for (bone=0; bone<transformSkel->Bones.size(); bone++)
						listBones.push_back (transformSkel->Bones[bone].getBoneName());

					// Get name of the mesh
					char nameMesh[512];
					_splitpath (meshName, NULL, NULL, nameMesh, NULL);

					// Select a bones
					std::string message = "Select a bone to stick " + string (nameMesh);
					CSelectString dialogSelect (listBones, message.c_str(), _MainFrame, false);
					if (dialogSelect.DoModal ()==IDOK)
					{
						// Select your bones
						bindBone = dialogSelect.Selection;
						boneName = listBones[dialogSelect.Selection];
					}
				}

				// Selected ?
				if (bindBone != 0xffffffff)
				{
					transformSkel->stickObject (pTrShape, bindBone);
					/*meshInstance->setPos (CVector::Null);
					meshInstance->setRotQuat (CQuat::Identity);
					meshInstance->setScale (1, 1, 1);*/
				}

				// Set the bone name
				iInfo->Saved.BindBoneName = boneName;
			}
		}

		// Return the instance index
		return (uint)_ListInstance.size()-1;
	}
	else
		return 0xffffffff;
}


// ***************************************************************************
bool CObjectViewer::chooseBone(const std::string &caption, NL3D::CSkeletonModel *&skel, uint &boneIndex, std::string *skelName /*= NULL*/, std::string *boneName /*= NULL*/)
{
	for(uint k = 0; k < _ListInstance.size(); ++k)
	{
		NL3D::CSkeletonModel *transformSkel = dynamic_cast<CSkeletonModel*>(_ListInstance[k]->TransformShape);
		if (transformSkel)
		{
			// Make a list of bones
			vector<string> listBones;
			uint bone;
			for (bone=0; bone<transformSkel->Bones.size(); bone++)
			{
				listBones.push_back (transformSkel->Bones[bone].getBoneName());
			}
			CSelectString dialogSelect (listBones, caption.c_str(), _MainFrame, false);
			if (dialogSelect.DoModal ()==IDOK)
			{
				boneIndex = dialogSelect.Selection; 
				skel = transformSkel;
				if (skelName)
				{
					*skelName = _ListInstance[k]->Saved.ShapeFilename;
				}
				if (boneName)
				{
					*boneName = safe_cast<NL3D::CSkeletonModel *>(_ListInstance[k]->TransformShape)->Bones[boneIndex].getBoneName();
				}
				return true;
			}
			return false;			
		}
	}
	return false;
}

// ***************************************************************************
bool CObjectViewer::isSkeletonPresent() const
{
	for(uint k = 0; k < _ListInstance.size(); ++k)
	{
		NL3D::CSkeletonModel *transformSkel = dynamic_cast<CSkeletonModel*>(_ListInstance[k]->TransformShape);
		if (transformSkel) return true;
	}
	return false;
}

// ***************************************************************************

uint CObjectViewer::addCamera (const NL3D::CCameraInfo &cameraInfo, const char* cameraName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// link to the root for manipulation
	CCamera *pCamera = (CCamera*)CNELU::Scene->createModel (CameraId);
	_SceneRoot->hrcLinkSon(pCamera);

	// Build the camera
	pCamera->build (cameraInfo);

	// Store the transform shape pointer
	CInstanceInfo *iInfo = new CInstanceInfo;
	iInfo->Camera = pCamera;

	// Store the name of the shape
	iInfo->MustDelete = true;
	iInfo->Saved.ShapeFilename = cameraName;
	iInfo->Saved.SkeletonId = 0xffffffff;
	iInfo->Saved.CameraInfo = cameraInfo;
	iInfo->Saved.Camera = true;
	_ListInstance.push_back (iInfo);
	_Cameras.push_back ((uint)_ListInstance.size()-1);

	// Reinit camera
	initCamera ();

	// Return the instance index
	return (uint)_ListInstance.size()-1;
}

// ***************************************************************************

uint CObjectViewer::addSkel (NL3D::IShape* pSkelShape, const char* skelName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	if (CNELU::ShapeBank->getPresentState(skelName)!=CShapeBank::NotPresent)
	{
		delete pSkelShape;
	}
	else
		CNELU::ShapeBank->add (skelName, CSmartPtr<IShape> (pSkelShape));

	// Create a model and add it to the scene
	CTransformShape	*pTrShape=CNELU::Scene->createInstance (skelName);
	nlassert (pTrShape);

	// link to the root for manipulation
	_SceneRoot->hrcLinkSon(pTrShape);

	// Get the real shape used by the instance.
	pSkelShape= pTrShape->Shape;

	// Get a skeleton model
	CSkeletonModel *skelModel=dynamic_cast<CSkeletonModel*>(pTrShape);

	// Is a skel ?
	if (skelModel)
	{
		// Set the rot model
		if (_MainFrame->Euler)
			pTrShape->setTransformMode (ITransformable::RotEuler);
		else
			pTrShape->setTransformMode (ITransformable::RotQuat);

		// Store the transform shape pointer
		CInstanceInfo *iInfo = new CInstanceInfo;
		iInfo->TransformShape = skelModel;

		// Store the name of the shape
		iInfo->MustDelete = true;
		iInfo->Saved.ShapeFilename = skelName;
		iInfo->Saved.IsSkeleton = true;
		iInfo->Saved.SkeletonId = 0xffffffff;
		_ListInstance.push_back (iInfo);	

		// set this skeleton for Skeleton Scale edition
		_SkeletonScaleDlg->setSkeletonToEdit(skelModel, skelName);

		// Return the instance
		return (uint)_ListInstance.size()-1;
	}
	return 0xffffffff;
}

// ***************************************************************************

IObjectViewer* IObjectViewer::getInterface (int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if (version!=OBJECT_VIEWER_VERSION)
	{
		MessageBox (NULL, "Bad version of object_viewer.dll.", "NeL object viewer", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CObjectViewer;
}

// ***************************************************************************

void IObjectViewer::releaseInterface (IObjectViewer* view)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete view;
}

// ***************************************************************************

void CObjectViewer::setSingleAnimation (NL3D::CAnimation* pAnim, const char* name, uint instance)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (instance < _ListInstance.size())
	{
		// Set active 
		_SelectedObject = instance;

		// Add the animation
		addAnimation (pAnim, (name+std::string(".anim")).c_str(), name, instance);

		// Add the animation to the animationSet
		_AnimationSetDlg->UpdateData (TRUE);
		_AnimationSetDlg->UseMixer = 1;
		_AnimationSetDlg->UpdateData (FALSE);

		// Set the animation in the first slot
		_ListInstance[instance]->Saved.SlotInfo[0].Animation = name;
		_ListInstance[instance]->Saved.SlotInfo[0].Skeleton = "";
		_ListInstance[instance]->Saved.SlotInfo[0].Offset = 0;
		_ListInstance[instance]->Saved.SlotInfo[0].StartTime = (int)(pAnim->getBeginTime()*_AnimationDlg->Speed);
		_ListInstance[instance]->Saved.SlotInfo[0].EndTime = (int)(pAnim->getEndTime()*_AnimationDlg->Speed);
		_ListInstance[instance]->Saved.SlotInfo[0].StartBlend = 1.f;
		_ListInstance[instance]->Saved.SlotInfo[0].EndBlend = 1.f;
		_ListInstance[instance]->Saved.SlotInfo[0].Enable = true;
		for (uint i=1; i<CChannelMixer::NumAnimationSlot; i++)
			_ListInstance[instance]->Saved.SlotInfo[i].Enable = false;

		// Update dialog box
		_AnimationSetDlg->refresh (TRUE);
		_SlotDlg->refresh (TRUE);
		_SoundAnimDlg->refresh(TRUE);

		// Reinit
		reinitChannels ();
	}
}

// ***************************************************************************

void CObjectViewer::setAutoAnimation (NL3D::CAnimationSet* pAnimSet)
{
	CNELU::Scene->setAutomaticAnimationSet (pAnimSet);
}

// ***************************************************************************

void CObjectViewer::setAmbientColor (const NLMISC::CRGBA& color)
{
	CNELU::Driver->setAmbientColor (color);

	// Setup also Scene lighting system here, even if not used.
	CNELU::Scene->setAmbientGlobal(color);
}

// ***************************************************************************

void CObjectViewer::setLight (unsigned char id, const NL3D::CLight& light)
{
	CNELU::Driver->enableLight (id);
	CNELU::Driver->setLight (id, light);
}

// ***************************************************************************


/** add an object that will be notified each time a frame is processed
  * \see removeMainLoopCallBack()
  */
void CObjectViewer::registerMainLoopCallBack(IMainLoopCallBack *i)
{
//	nlassert(std::find(_CallBackList.begin(), _CallBackList.end(), i) == _CallBackList.begin()); // the object was register twice !!
	_CallBackList.push_back(i);
}

/// remove an object that was registered with registerMainLoopCallBack()
void CObjectViewer::removeMainLoopCallBack(IMainLoopCallBack *i)
{
	std::vector<IMainLoopCallBack *>::iterator it = std::find(_CallBackList.begin(), _CallBackList.end(), i);
	nlassert(it  != _CallBackList.end()); // this object wasn't registered
	_CallBackList.erase(it);	
}

// ***************************************************************************
void CObjectViewer::activateTextureSet(uint index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	std::vector<CInstanceInfo*>::iterator it;
	for (it = _ListInstance.begin(); it != _ListInstance.end(); ++it)
	{
		NL3D::CTransformShape	*trShape= (*it)->TransformShape;
		if (dynamic_cast<NL3D::CMeshBaseInstance *>(trShape))
		{
			static_cast<NL3D::CMeshBaseInstance *>(trShape)->selectTextureSet(index);
		}		
	}	
}

// ***************************************************************************
void CObjectViewer::shuffleTextureSet()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	std::vector<CInstanceInfo*>::iterator it;
	for (it = _ListInstance.begin(); it != _ListInstance.end(); ++it)
	{
		NL3D::CTransformShape	*trShape= (*it)->TransformShape;
		if (dynamic_cast<NL3D::CMeshBaseInstance *>(trShape))
		{
			static_cast<NL3D::CMeshBaseInstance *>(trShape)->selectTextureSet(rand() % 8);
		}		
	}	
}


// ***************************************************************************

void CObjectViewer::removeAllInstancesFromScene()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Remove all stand alone TransformShapes.
	for(uint instance=0; instance<_ListInstance.size(); instance++)
	{
		delete _ListInstance[instance];
	}

	// Remove all stand alone TransformShapes.
	_ListInstance.clear();
	_Cameras.clear();
	_CurrentCamera = -1;
	_SelectedObject = 0xffffffff;

	// Remove added/loaded igs and their instances.
	for(uint igId=0; igId<_ListIG.size(); igId++)
	{
		// remove instances.
		_ListIG[igId]->removeFromScene(*CNELU::Scene);
		// free up the ig.
		delete _ListIG[igId];
	}
	_ListIG.clear();

	// clear dynamic lighting test
	_GlobalRetriever= NULL;
	CNELU::Scene->deleteInstance(_ObjectLightTest);
	_ObjectLightTest= NULL;

	// Reset mesh cache
	CNELU::ShapeBank->reset();

	// Invalidate dialogs
	if (CNELU::Driver->isActive())
	{
		_AnimationSetDlg->refresh (TRUE);
		_SlotDlg->refresh (TRUE);
		_SoundAnimDlg->refresh (TRUE);
	}

	// remove any skeleton edited
	if(_InstanceRunning)
		_SkeletonScaleDlg->setSkeletonToEdit(NULL, "");
}


// ***************************************************************************
void CObjectViewer::enableFXs(bool enabled)
{
	// Stand alone fxs
	for(uint instance=0; instance<_ListInstance.size(); instance++)
	{
		NL3D::CSegRemanence *sr = dynamic_cast<NL3D::CSegRemanence *>(_ListInstance[instance]->TransformShape);
		if (sr)
		{
			if (enabled) sr->start();
			else sr->stop();
		}
	}
	// remanences in igs
	for(uint igId = 0; igId < _ListIG.size(); ++igId)
	{
		for(uint k = 0; k < _ListIG[igId]->_Instances.size(); ++k)
		{
			NL3D::CSegRemanence *sr = dynamic_cast<NL3D::CSegRemanence *>(_ListIG[igId]->_Instances[k]);
			if (sr)
			{
				if (enabled) sr->start();
				else sr->stop();
			}
		}	
	}	
}

// ***************************************************************************

void CObjectViewer::evalSoundTrack (float lastTime, float currentTime)
{
	// Gor each object
	for (uint i = 0; i < _ListInstance.size(); i++)
	{
		// Some animation in the list ?
		if (_ListInstance[i]->Saved.PlayList.size() > 0)
		{
			// Accumul time
			float startTime = 0;
			float endTime = 0;

			// Get start time of the animation that starts before the current time
			for (uint index = 0; index < _ListInstance[i]->Saved.PlayList.size(); index++)
			{
				// Pointer on the animation
				string& name = _ListInstance[i]->Saved.PlayList[index];
				CAnimation *anim = _ListInstance[i]->AnimationSet.getAnimation (_ListInstance[i]->AnimationSet.getAnimationIdByName(name));

				// Add start time
				startTime = endTime;
				endTime = startTime + anim->getEndTime()-anim->getBeginTime();

				if ((startTime <= currentTime) && (currentTime < endTime))
				{
					// setup the sound context

					DWORD tab[] = {IDC_ARG0, IDC_ARG1, IDC_ARG2, IDC_ARG3, };
					
					for (uint i = 0; i < 4; i++)
					{
						CEdit *edit = (CEdit*) _SoundAnimDlg->GetDlgItem(tab[i]);
						nlassert(edit);
						char str[1024];
						edit->GetLine(0, str, 1024);
						SoundContext.Args[i] = atoi (str);
					}

					// get the position of the skel if a skel is available
					if (SkeletonUsedForSound != 0xFFFFFFFF)
					{
						const CMatrix &m = _ListInstance[SkeletonUsedForSound]->TransformShape->getMatrix();
						SoundContext.Position = m.getPos();
					}
					else
					{
						SoundContext.Position = CVector::Null;
					}

					CSoundSystem::playAnimation(name, lastTime - startTime, currentTime - startTime, SoundContext);
				}
			}
		}
	}
}

// ***************************************************************************

/*
void CObjectViewer::evalSoundTrack (float lastTime, float currentTime)
{
	if (lastTime!=currentTime)
	{
		// For each objects
		for (uint instance=0; instance<_ListInstance.size(); instance++)
		{
			// For each channel of the mixer
			for (uint slot=0; slot<CChannelMixer::NumAnimationSlot; slot++)
			{
				// Anim id
				uint animId=_ListInstance[instance]->Playlist.getAnimation (slot);

				// Channel actif ?
				if (animId!=CAnimationPlaylist::empty)
				{
					// Get the animation
					CAnimation *anim=_ListInstance[instance]->AnimationSet.getAnimation (animId);
					nlassert (anim);

					// Get the sound track
					uint trackId=anim->getIdTrackByName ("NoteTrack");
					if (trackId!=CAnimation::NotFound)
					{
						// Get the track
						ITrack *track=anim->getTrack (trackId);
						nlassert (track);

						// Dynamic cast
						UTrackKeyframer *soundTrackKF = dynamic_cast<UTrackKeyframer *>(track);
						if (soundTrackKF)
						{
							// Sound keys
							std::vector<TAnimationTime> result;

							// Get local begin and endTime
							TAnimationTime localLastTime = _ListInstance[instance]->Playlist.getLocalTime (slot, lastTime, _ListInstance[instance]->AnimationSet);
							TAnimationTime localCurrentTime = _ListInstance[instance]->Playlist.getLocalTime (slot, currentTime, _ListInstance[instance]->AnimationSet);

							// Good interval
							if (localLastTime<=localCurrentTime)
							{
								// Get keys in this interval
								soundTrackKF->getKeysInRange(localLastTime, localCurrentTime, result);
							}
							else
							{
								// Get begin and last time
								TAnimationTime beginTime=track->getBeginTime ();
								TAnimationTime endTime=track->getEndTime ();

								// Time must have been clamped
								nlassert (localCurrentTime<=endTime);
								nlassert (localLastTime>=beginTime);

								// Get keys to the end
								soundTrackKF->getKeysInRange(localCurrentTime, endTime, result);

								// Get keys at the beginning
								soundTrackKF->getKeysInRange(beginTime, localLastTime, result);
							}

							// Process sounds
							NLSOUND::UAudioMixer *audioMixer = CSoundSystem::getAudioMixer ();
							if( audioMixer )
							{	
								vector<TAnimationTime>::iterator itResult;
								for( itResult = result.begin(); itResult != result.end(); ++itResult ) 
								{
									string soundName;
									double keyTime = *itResult;
									nlinfo("keyTime = %f  result size : %d",*itResult,result.size());
									
									if( !track->interpolate( *itResult, soundName) )
									{
										nlwarning("The key at offset %f is not a string",*itResult);
									}
									else
									{
										// if there are step sounds
										if( soundName == "step" )
										{
 											// need to spawn a sound linked to the anim
											string dummySound = "PAShommecourseappartdur1a";
											USource *source = audioMixer->createSource (dummySound.c_str() , true );
											if (source)
											{
												source->setPos (CVector::Null);
												source->play ();
 												nlinfo ("launching dummy sound %s for the step event", dummySound.c_str());
											}
											else
											{
	 											nlwarning ("sound not found for the step event: '%s'", dummySound.c_str());
											}
										}
 										else if (soundName.find ("snd_") != string::npos)
 										{
 											// need to spawn a sound linked to the anim
											USource *source = audioMixer->createSource ( soundName.c_str(), true );
											if (source)
											{
												source->setPos (CVector::Null);
												source->play ();
 												nlinfo ("launching sound for anim event from notetrack '%s'", soundName.c_str());
											}
											else
											{
	 											nlwarning ("sound not found: '%s'", soundName.c_str());
											}
 										}
 										else
 										{
 											nlwarning ("unknown notetrack event: '%s'", soundName.c_str());
 										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
*/


// ***************************************************************************
uint CObjectViewer::addInstanceGroup(NL3D::CInstanceGroup *ig)
{
	// First instance
	uint first = (uint)_ListInstance.size();

	// Add all models to the scene		
	ig->addToScene(*CNELU::Scene, CNELU::Driver);
	// Unfreeze all objects from HRC.
	ig->unfreezeHRC();

	// link the root of the IG to our root, for scene rotation
	ig->linkRoot(*CNELU::Scene, _SceneRoot);

	// Keep a reference on them, but they'll be destroyed by IG.
	for (uint k = 0; k < ig->getNumInstance(); ++k)
	{
		CInstanceInfo *iInfo = new CInstanceInfo;
		iInfo->TransformShape = ig->_Instances[k];
		iInfo->Saved.ShapeFilename = ig->_InstancesInfos[k].Name;
		iInfo->MustDelete = false;
		_ListInstance.push_back (iInfo);
	}

	// Add the ig to the list.
	_ListIG.push_back(ig);

	// Return first instance
	return first;
}

// ***************************************************************************
void CObjectViewer::setupSceneLightingSystem(bool enable, const NLMISC::CVector &sunDir, NLMISC::CRGBA sunAmbiant, NLMISC::CRGBA sunDiffuse, NLMISC::CRGBA sunSpecular)
{
	CNELU::Scene->enableLightingSystem(enable);

	// Setup sun.
	CNELU::Scene->setSunAmbient(sunAmbiant);
	CNELU::Scene->setSunDiffuse(sunDiffuse);
	CNELU::Scene->setSunSpecular(sunSpecular);
	CNELU::Scene->setSunDirection(sunDir);
}


// ***************************************************************************
void CObjectViewer::enableDynamicObjectLightingTest(NLPACS::CGlobalRetriever *globalRetriever, NL3D::CInstanceGroup *ig)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// first delete the instance
	if(_ObjectLightTest)
	{
		CNELU::Scene->deleteInstance(_ObjectLightTest);
		_ObjectLightTest= NULL;
	}

	_GlobalRetriever= globalRetriever;

	// if enabled
	if(_GlobalRetriever)
	{
		nlassert(ig);

		// this mesh is the dynamic one to move around.
		_ObjectLightTest= CNELU::Scene->createInstance(_ObjectLightTestShape);
		if(_ObjectLightTest!=NULL)
		{
			// link to the root for manipulation
			_SceneRoot->hrcLinkSon(_ObjectLightTest);

			// setup the matrix.
			_ObjectLightTestMatrix= _ObjectLightTest->getMatrix();
			// setup the logic info.
			_ObjectLightTestLogicInfo.GlobalRetriever= _GlobalRetriever;
			_ObjectLightTestLogicInfo.Ig= ig;
			// Default the GPos to uninitialized
			_ObjectLightTestLogicInfo.GPos.InstanceId= -1;
			_ObjectLightTest->setLogicInfo(&_ObjectLightTestLogicInfo);
		}
		else
		{
			if (!_ObjectLightTestShape.empty())
			{			
				string	str= string("Path not found for Light Test Shape: ") + _ObjectLightTestShape;
				::MessageBox(NULL, str.c_str(), "Dynamic Object Light Test", MB_OK|MB_ICONEXCLAMATION);
			}
			// disable.
			_ObjectLightTest= NULL;
			_GlobalRetriever= NULL;
		}
	}
}


// ***************************************************************************
void	CObjectViewer::COVLogicInfo::getStaticLightSetup(NLMISC::CRGBA sunAmbient, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, CRGBA &ambient)
{
	Ig->getStaticLightSetup(sunAmbient, GlobalRetriever->getLocalRetrieverId(GPos), 
		GPos.LocalPosition.Surface,
		GPos.LocalPosition.Estimation, 
		pointLightList, sunContribution, ambient);
}


// ***************************************************************************
// ***************************************************************************
// Vegetable Landscape Part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CObjectViewer::loadVegetableLandscapeCfg(NLMISC::CConfigFile &cf)
{
	// vegetable display is true by default.
	_VegetableEnabled= true;
	_VegetableSnapToGround= true;


	// Load landscape setup
	// --------------
	try
	{
		// tileBank setup.
		CConfigFile::CVar &tileBank = cf.getVar("veget_tile_bank");
		_VegetableLandscapeTileBank= tileBank.asString();
		CConfigFile::CVar &tileFarBank = cf.getVar("veget_tile_far_bank");
		_VegetableLandscapeTileFarBank= tileFarBank.asString();
		// zone list.
		_VegetableLandscapeZoneNames.clear();
		CConfigFile::CVar &zones = cf.getVar("veget_landscape_zones");
		for (uint i=0; i<(uint)zones.size(); i++)
			_VegetableLandscapeZoneNames.push_back(zones.asString(i).c_str());
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeTileBank.clear();
		_VegetableLandscapeTileFarBank.clear();
		_VegetableLandscapeZoneNames.clear();
	}


	// Load Landscape params.
	// --------------
	// threshold
	try
	{
		CConfigFile::CVar &thre= cf.getVar("veget_landscape_threshold");
		_VegetableLandscapeThreshold= thre.asFloat();
		// clamp to avoid divide/0.
		_VegetableLandscapeThreshold= max(_VegetableLandscapeThreshold, 0.001f);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeThreshold= 0.003f;
	}
	// tilenear
	try
	{
		CConfigFile::CVar &tileNear= cf.getVar("veget_landscape_tile_near");
		_VegetableLandscapeTileNear= tileNear.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeTileNear= 50;
	}
	// ambient
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_landscape_ambient");
		_VegetableLandscapeAmbient.R= color.asInt(0);
		_VegetableLandscapeAmbient.G= color.asInt(1);
		_VegetableLandscapeAmbient.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeAmbient.set(80, 80, 80);
	}
	// diffuse
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_landscape_diffuse");
		_VegetableLandscapeDiffuse.R= color.asInt(0);
		_VegetableLandscapeDiffuse.G= color.asInt(1);
		_VegetableLandscapeDiffuse.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeDiffuse.set(255, 255, 255);
	}
	// Snapping
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_landscape_snap_height");
		_VegetableSnapHeight= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableSnapHeight= 1.70f;
	}


	// Load Vegetable params.
	// --------------

	// vegetable texture
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_texture");
		_VegetableTexture= var.asString();
	}
	catch (EUnknownVar &)
	{
		_VegetableTexture= "";
	}

	// vegetable ambient
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_ambient");
		_VegetableAmbient.R= color.asInt(0);
		_VegetableAmbient.G= color.asInt(1);
		_VegetableAmbient.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableAmbient.set(80, 80, 80);
	}
	// vegetable diffuse
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_diffuse");
		// setup to behave correclty ie as maxLightFactor:
		sint	R= color.asInt(0) - _VegetableAmbient.R;	clamp(R, 0, 255);	_VegetableDiffuse.R= R;
		sint	G= color.asInt(1) - _VegetableAmbient.G;	clamp(G, 0, 255);	_VegetableDiffuse.G= G;
		sint	B= color.asInt(2) - _VegetableAmbient.B;	clamp(B, 0, 255);	_VegetableDiffuse.B= B;
	}
	catch (EUnknownVar &)
	{
		sint	R= 255 - _VegetableAmbient.R;	clamp(R, 0, 255);	_VegetableDiffuse.R= R;
		sint	G= 255 - _VegetableAmbient.G;	clamp(G, 0, 255);	_VegetableDiffuse.G= G;
		sint	B= 255 - _VegetableAmbient.B;	clamp(B, 0, 255);	_VegetableDiffuse.B= B;
	}
	// vegetable lightDir
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_light_dir");
		_VegetableLightDir.x= var.asFloat(0);
		_VegetableLightDir.y= var.asFloat(1);
		_VegetableLightDir.z= var.asFloat(2);
		_VegetableLightDir.normalize();
	}
	catch (EUnknownVar &)
	{
		_VegetableLightDir.set(0, 1, -1);
		_VegetableLightDir.normalize();
	}

	// windDir
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_dir");
		_VegetableWindDir.x= var.asFloat(0);
		_VegetableWindDir.y= var.asFloat(1);
		_VegetableWindDir.z= var.asFloat(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableWindDir.x= 0.5f;
		_VegetableWindDir.y= 0.5f;
		_VegetableWindDir.z= 0;
	}
	// windFreq
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_freq");
		_VegetableWindFreq= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindFreq= 0.5;
	}
	// windPower
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_power");
		_VegetableWindPower= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindPower= 1;
	}
	// windBendMin
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_bend_min");
		_VegetableWindBendMin= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindBendMin= 0;
	}


}


// ***************************************************************************
bool		CObjectViewer::createVegetableLandscape()
{
	// If not already done.
	if(!_VegetableLandscape)
	{
		// create the landscape.
		_VegetableLandscape= static_cast<CLandscapeModel*>(CNELU::Scene->createModel(LandscapeModelId));

		// Create a Progress Dialog.
		CDialogProgress		dlgProgress;
		dlgProgress.Create(CDialogProgress::IDD, _MainFrame);
		dlgProgress.ShowWindow(true);

		try
		{
			if(_VegetableLandscapeTileBank=="")
			{
				throw Exception("Landscape CFG not fully defined");
			}

			// Load The Bank files (copied from CLandscapeUser :) ).
			// ================
			// progress
			dlgProgress.ProgressText.SetWindowText("Loading TileBanks...");
			dlgProgress.ProgressBar.SetPos(0);
			// load
			CIFile bankFile(CPath::lookup(_VegetableLandscapeTileBank));
			_VegetableLandscape->Landscape.TileBank.serial(bankFile);
			_VegetableLandscape->Landscape.TileBank.makeAllPathRelative();
			_VegetableLandscape->Landscape.TileBank.makeAllExtensionDDS();
			_VegetableLandscape->Landscape.TileBank.setAbsPath ("");

			// progress
			dlgProgress.ProgressBar.SetPos(50);
			// load
			CIFile farbankFile(CPath::lookup(_VegetableLandscapeTileFarBank));
			_VegetableLandscape->Landscape.TileFarBank.serial(farbankFile);
			if ( ! _VegetableLandscape->Landscape.initTileBanks() )
			{
				nlwarning( "You need to recompute bank.farbank for the far textures" );
			}
			bankFile.close();
			farbankFile.close();


			// flushTiles.
			// ================
			if(CNELU::Driver)
			{
				// progress
				dlgProgress.ProgressText.SetWindowText("Loading Tiles...");
				dlgProgress.ProgressBar.SetPos(0);

				// count nbText to load.
				sint	ts;
				sint	nbTextTotal= 0;
				for (ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
				{
					CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					nbTextTotal+= tileSet->getNumTile128();
					nbTextTotal+= tileSet->getNumTile256();
					nbTextTotal+= CTileSet::count;
				}

				// load.
				sint	nbTextDone= 0;
				for (ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
				{
					CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					sint tl;
					for (tl=0; tl<tileSet->getNumTile128(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTile128(tl), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
					for (tl=0; tl<tileSet->getNumTile256(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTile256(tl), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
					for (tl=0; tl<CTileSet::count; tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTransition(tl)->getTile (), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
				}
			}


			// misc setup.
			// ================
			_VegetableLandscape->Landscape.setThreshold(_VegetableLandscapeThreshold);
			_VegetableLandscape->Landscape.setTileNear(_VegetableLandscapeTileNear);
			_VegetableLandscape->Landscape.setupStaticLight(_VegetableLandscapeDiffuse, _VegetableLandscapeAmbient, 1);
			_VegetableLandscape->Landscape.loadVegetableTexture(_VegetableTexture);
			_VegetableLandscape->Landscape.setupVegetableLighting(_VegetableAmbient, _VegetableDiffuse, _VegetableLightDir);
			_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);


			// Load the zones.
			// ================
			// landscape recentering.
			bool	zoneLoaded= false;
			CAABBox	landscapeBBox;
			// progress
			dlgProgress.ProgressText.SetWindowText("Loading Zones...");
			dlgProgress.ProgressBar.SetPos(0);
			uint	nbZones= (uint)_VegetableLandscapeZoneNames.size();
			for(uint i=0; i<nbZones;i++)
			{
				// open the file
				CIFile	zoneFile(CPath::lookup(_VegetableLandscapeZoneNames[i]));
				CZone	zone;
				// load
				zoneFile.serial(zone);
				// append to landscape
				_VegetableLandscape->Landscape.addZone(zone);
				// progress
				dlgProgress.ProgressBar.SetPos(i*100/nbZones);

				// Add to the bbox.
				if(!zoneLoaded)
				{
					zoneLoaded= true;
					landscapeBBox.setCenter(zone.getZoneBB().getCenter());
				}
				else
					landscapeBBox.extend(zone.getZoneBB().getCenter());
			}

			// After All zone loaded, recenter the mouse listener on the landscape.
			if(zoneLoaded)
			{
				CMatrix		matrix;
				_MouseListener.setHotSpot(landscapeBBox.getCenter());
				matrix.setPos(landscapeBBox.getCenter());
				matrix.rotateX(-(float)Pi/4);
				matrix.translate(CVector(0,-1000,0));
				_MouseListener.setMatrix(matrix);
			}

			// Create collisions objects.
			_VegetableCollisionManager= new CVisualCollisionManager;
			_VegetableCollisionManager->setLandscape(&_VegetableLandscape->Landscape);
			_VegetableCollisionEntity= _VegetableCollisionManager->createEntity();
		}
		catch (Exception &e)
		{
			// close the progress dialog
			dlgProgress.DestroyWindow();

			MessageBox(_MainFrame->m_hWnd, e.what(), "Failed to Load landscape", MB_OK | MB_APPLMODAL);

			// remove first possibly created collisions objects.
			if(_VegetableCollisionEntity)
			{
				_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
				_VegetableCollisionEntity= NULL;
			}
			if(_VegetableCollisionManager)
			{
				delete _VegetableCollisionManager;
				_VegetableCollisionManager= NULL;
			}

			// remove the landscape
			CNELU::Scene->deleteModel(_VegetableLandscape);
			_VegetableLandscape= NULL;

			return false;
		}

		// close the progress dialog
		dlgProgress.DestroyWindow();
	}

	return true;
}


// ***************************************************************************
void		CObjectViewer::showVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->show();
	}
}

// ***************************************************************************
void		CObjectViewer::hideVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->hide();
	}
}


// ***************************************************************************
void		CObjectViewer::enableLandscapeVegetable(bool enable)
{
	// update
	_VegetableEnabled= enable;

	// update view.
	if(_VegetableLandscape)
	{
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}


// ***************************************************************************
void		CObjectViewer::refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc)
{
	// if landscape is displayed.
	if(_VegetableLandscape)
	{
		// first disable the vegetable, to delete any vegetation
		_VegetableLandscape->Landscape.enableVegetable(false);

		// Then change all the tileSet of all the TileBanks.
		for (sint ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
		{
			CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
			// change the vegetableTileDesc of this tileSet.
			tileSet->setTileVegetableDesc(tvdesc);
		}

		// re-Enable the vegetable (if wanted).
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}


// ***************************************************************************
void		CObjectViewer::setVegetableWindPower(float w)
{
	_VegetableWindPower= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}
// ***************************************************************************
void		CObjectViewer::setVegetableWindBendStart(float w)
{
	_VegetableWindBendMin= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}
// ***************************************************************************
void		CObjectViewer::setVegetableWindFrequency(float w)
{
	_VegetableWindFreq= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}


// ***************************************************************************
void		CObjectViewer::snapToGroundVegetableLandscape(bool enable)
{
	// update
	_VegetableSnapToGround= enable;
}

// ***************************************************************************
CInstanceInfo::CInstanceInfo ()
{
	TransformShape = NULL;
	Camera = NULL;
	MustDelete = false;
}

// ***************************************************************************
CInstanceInfo::~CInstanceInfo ()
{
	if (MustDelete)
	{
		if (TransformShape)
			CNELU::Scene->deleteInstance (TransformShape);
		if (Camera)
			CNELU::Scene->deleteModel (Camera);
	}
}

// ***************************************************************************
CSlotInfo::CSlotInfo ()
{
	StartTime = 0;
	EndTime = 0;
	Offset = 0;
	StartBlend = 1;
	EndBlend = 1;
	Smoothness = 1;
	SpeedFactor = 1;
	ClampMode = 0;
	SkeletonInverted = false;
	Enable = true;
}

// ***************************************************************************
void CSlotInfo::serial (NLMISC::IStream &f)
{
	f.serialVersion (0);
	f.serial (Animation);
	f.serial (Skeleton);
	f.serial (Offset);
	f.serial (StartTime);
	f.serial (EndTime);
	f.serial (StartBlend);
	f.serial (EndBlend);
	f.serial (Smoothness);
	f.serial (SpeedFactor);
	f.serial (ClampMode);
	f.serial (SkeletonInverted);
	f.serial (Enable);
}

// ***************************************************************************
uint CObjectViewer::getEditedObject ()
{
	return _SelectedObject;
}

// ***************************************************************************
void CObjectViewer::setEditedObject (uint selected)
{
	_SelectedObject=selected;
}

// ***************************************************************************
CInstanceInfo *CObjectViewer::getInstance (uint instance)
{
	return _ListInstance[instance];
}

// ***************************************************************************
uint CObjectViewer::getNumInstance () const
{
	return (uint)_ListInstance.size ();
}

// ***************************************************************************
void CInstanceInfo::setAnimationPlaylist (float frameRate)
{
	for (uint id=0; id<NL3D::CChannelMixer::NumAnimationSlot; id++)
	{
		if (Saved.SlotInfo[id].Enable)
		{
			// Set the animation
			uint animId = AnimationSet.getAnimationIdByName (Saved.SlotInfo[id].Animation);
			if (animId == CAnimationSet::NotFound)
				Playlist.setAnimation (id, CAnimationPlaylist::empty);
			else			
				Playlist.setAnimation (id, animId);

			// Set the skeleton weight
			uint skelId = AnimationSet.getSkeletonWeightIdByName (Saved.SlotInfo[id].Skeleton);
			if (skelId == CAnimationSet::NotFound)
				Playlist.setSkeletonWeight (id, CAnimationPlaylist::empty, false);
			else
				Playlist.setSkeletonWeight (id, skelId, Saved.SlotInfo[id].SkeletonInverted);

			// Set others values
			Playlist.setTimeOrigin (id, Saved.SlotInfo[id].Offset/frameRate);
			Playlist.setSpeedFactor (id, Saved.SlotInfo[id].SpeedFactor);
			Playlist.setStartWeight (id, Saved.SlotInfo[id].StartBlend, Saved.SlotInfo[id].StartTime/frameRate);
			Playlist.setEndWeight (id, Saved.SlotInfo[id].EndBlend, Saved.SlotInfo[id].EndTime/frameRate);
			Playlist.setWeightSmoothness (id, Saved.SlotInfo[id].Smoothness);

			// Switch between wrap modes
			switch (Saved.SlotInfo[id].ClampMode)
			{
			case 0:
				Playlist.setWrapMode (id, CAnimationPlaylist::Clamp);
				break;
			case 1:
				Playlist.setWrapMode (id, CAnimationPlaylist::Repeat);
				break;
			case 2:
				Playlist.setWrapMode (id, CAnimationPlaylist::Disable);
				break;
			}
		}
	}
}

// ***************************************************************************
CInstanceSave::CInstanceSave ()
{
	SkeletonId = 0xffffffff;
	IsSkeleton = false;
	Camera = false;
}

// ***************************************************************************
void CInstanceSave::serial (NLMISC::IStream &f)
{
	// Serial a version
	sint ver = f.serialVersion (1);

	// Play list of this object
	f.serialCont (PlayList);

	// Slot info for this object
	nlassert (NL3D::CChannelMixer::NumAnimationSlot == 8);
	for (uint slot=0; slot<8; slot++)
		// Serial the slot information
		f.serial (SlotInfo[slot]);

	// Input file
	f.serial (ShapeFilename);

	// Skeleton id
	f.serial (SkeletonId);

	// Bind bone name
	f.serial (BindBoneName);

	// Is a skeleton
	f.serial (IsSkeleton);

	// Animation input file
	f.serialCont (AnimationFileName);

	// Skeleton weight input file
	f.serialCont (SWTFileName);

	// Is a camera
	if (ver>=1)
	{
		f.serial (Camera);
		f.serial (CameraInfo);
	}
	else if (f.isReading ())
		Camera = false;
}

// ***************************************************************************
void CObjectViewer::refreshAnimationListeners()
{
	_SoundAnimDlg->refresh (TRUE);
}

// ***************************************************************************
void CObjectViewer::addAnimation (NL3D::CAnimation* anim, const char* filename, const char* name, uint instance)
{
	// Add an animation
	uint id = _ListInstance[instance]->AnimationSet.addAnimation (name, anim);
	_ListInstance[instance]->Saved.AnimationFileName.push_back (filename);

	// Rebuild the animationSet
	_ListInstance[instance]->AnimationSet.build ();
	if(CNELU::Driver && CNELU::ShapeBank)
	{
		_ListInstance[instance]->AnimationSet.preloadSSSShapes(*CNELU::Driver, *CNELU::ShapeBank);
	}

	_SoundAnimDlg->refresh (TRUE);
}

// ***************************************************************************
void CObjectViewer::loadAnimation (const char* fileName, uint instance)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Make an animation
		CAnimation *anim=new CAnimation;

		// Serial it
		anim->serial (file);

		// Add the animation
		addAnimation (anim, fileName, name, instance);
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}
}

// ***************************************************************************
void CObjectViewer::loadSWT (const char* fileName, uint instance)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Get the skeleton pointer
		CSkeletonWeight* skel=new CSkeletonWeight;

		// Serial it
		skel->serial (file);

		// Add an animation
		_ListInstance[instance]->AnimationSet.addSkeletonWeight (name, skel);

		// Add the filename in the list
		_ListInstance[instance]->Saved.SWTFileName.push_back (fileName);
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}
}

// ***************************************************************************
CMainDlg *CObjectViewer::getSlotDlg ()
{
	return _SlotDlg;
}

// ***************************************************************************
void CObjectViewer::reloadTextures ()
{
	// For each instances
	uint numInstance = getNumInstance ();
	uint instance;
	for (instance=0; instance<numInstance; instance++)
	{
		// Get the info
		CInstanceInfo *info = getInstance (instance);

		// For each material
		if (info->TransformShape)
		{
			uint numMaterial = info->TransformShape->getNumMaterial ();
			uint mat;
			for (mat=0; mat<numMaterial; mat++)
			{
				// Get the material
				CMaterial *material = info->TransformShape->getMaterial (mat);

				// For each texture
				int tex;
				for (tex=0; tex<IDRV_MAT_MAXTEXTURES; tex++)
				{
					ITexture *texture = material->getTexture (tex);

					// Touch it!
					if (texture)
					{
						CNELU::Driver->invalidateShareTexture (*texture);
					}
				}
			}
		}
	}
}

// ***************************************************************************
// ***************************************************************************
// Global wind part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CObjectViewer::setGlobalWindPower(float w)
{
	if(_MainFrame)
	{
		clamp(w, 0.f, 1.f);
		_MainFrame->GlobalWindPower= w;
		CNELU::Scene->setGlobalWindPower(w);
	}
}


// ***************************************************************************
float		CObjectViewer::getGlobalWindPower() const
{
	if(_MainFrame)
		return _MainFrame->GlobalWindPower;
	else
		return 1.f;
}


void		CObjectViewer::shootScene() 
{
	static const char BASED_CODE szFilter[] = "Targa Files (*.tga)|*.tga|Jpeg Files (*.jpg)|*.jpg|All Files (*.*)|*.*||";
	CFileDialog fileDlg ( FALSE, ".tga", "*.tga", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal () == IDOK)
	{
		// Choose the size
		CSelectMovieSize movieSize;
		if (movieSize.DoModal () == IDOK)
		{
			// Resize the window
			RECT window;
			_MainFrame->GetWindowRect (&window);
			uint32 width;
			uint32 height;
			CNELU::Driver->getWindowSize (width, height);
			window.right += movieSize.Width - width;
			window.bottom += movieSize.Height - height;
			_MainFrame->SetWindowPos (NULL, 0, 0, window.right-window.left, window.bottom-window.top, SWP_NOMOVE|SWP_NOZORDER);
			

			// Swap the buffers
			CNELU::swapBuffers();
			CNELU::Driver->setupViewport (CViewport ());

			// The file name
			string filename = NLMISC::CFile::getFilenameWithoutExtension ((const char*)fileDlg.GetPathName ());
			string extension = NLMISC::CFile::getExtension ((const char*)fileDlg.GetPathName ());

			// The file name without extension
			bool jpeg = toLower (extension) == "jpg";

			// Activate the driver
 			CNELU::Driver->activate ();

			// Bitmap for shoot
			NLMISC::CBitmap shoot;

			// For each frame
			sint i;
			for (i=(sint)_AnimationDlg->Start; i<=(sint)_AnimationDlg->End; i++)
			{
				// Set the time
				_AnimationDlg->setCurrentFrame ((float)i);

				// Setup the play list
				setupPlaylist (_AnimationDlg->getTime());

				// Animate the automatic animation in the scene
				animateCNELUScene (_CS, (uint64)(1000.f / _AnimationDlg->Speed));

				// Eval channel mixer for transform
				for (uint j=0; j<_ListInstance.size(); j++)
					_ListInstance[j]->ChannelMixer.eval (false);

				// Clear the buffers
				CNELU::clearBuffers (_BackGroundColor);

				// call of callback list
				{
					std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end());
					for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end(); ++it)
					{
						(*it)->goPreRender();
					}
				}
				// Render the CS
				if (_CS) _CS->render ();			
			
				// Draw the scene		
				CNELU::Scene->render();	
				
				// call of callback list
				{
					std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end());
					for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end(); ++it)
					{
						(*it)->goPostRender();
					}
				}
				


				// Swap the buffers
				CNELU::swapBuffers();

				// Get the buffer
				CNELU::Driver->getBuffer (shoot);
				shoot.flipV ();

				// Save it
				char num[12];
				smprintf (num, 12, "%04d", i);
				string filenamefinal = filename+num+string (".")+extension;
				try
				{
					NLMISC::COFile output;
					if (output.open (filenamefinal))
					{
						if (jpeg)
							shoot.writeJPG ( output, 255 );
						else
							shoot.writeTGA ( output, 32 );
					}
					else
					{
						_MainFrame->MessageBox (("Can't open the file "+filenamefinal+" for writing.").c_str (), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
						break;
					}
				}
				catch (Exception &e)
				{
					_MainFrame->MessageBox (("Error during writing of the file "+filenamefinal+" : "+(string)e.what ()).c_str (), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
					break;
				}
			}
		}
	}
}

void CObjectViewer::drawFXUserMatrix()

{
	static std::string fxUserMatrixStr;
	static bool stringRetrieved = false;
	if (!stringRetrieved)
	{	
		CString fxUserMatrix;
		fxUserMatrix.LoadString(IDS_FX_USER_MATRIX);
		fxUserMatrixStr = (LPCTSTR) fxUserMatrix;
		stringRetrieved = true;
	}
	nlassert(_ParticleDlg);
	drawNamedMatrix(getFXUserMatrix(), fxUserMatrixStr, NLMISC::CRGBA::Red, 0.2f, 10.f);
}

void CObjectViewer::drawFXMatrix()
{
	static std::string fxStr;
	static bool stringRetrieved = false;
	if (!stringRetrieved)
	{	
		CString fx;
		fx.LoadString(IDS_FX_MATRIX);
		fxStr = (LPCTSTR) fx;
		stringRetrieved = true;
	}
	drawNamedMatrix(_ParticleDlg->getPSWorldMatrix(), fxStr, NLMISC::CRGBA::Blue, -0.2f, 10.f);
}

void CObjectViewer::drawSceneMatrix()
{
	static std::string sceneMatrixStr;
	static bool stringRetrieved = false;
	if (!stringRetrieved)
	{	
		CString sceneMatrix;
		sceneMatrix.LoadString(IDS_SCENE_MATRIX);
		sceneMatrixStr = (LPCTSTR) sceneMatrix;
		stringRetrieved = true;
	}
	drawNamedMatrix(_SceneRoot->getMatrix(), sceneMatrixStr, NLMISC::CRGBA::White, 0.f, 10.f);
}


void CObjectViewer::drawNamedMatrix(const NLMISC::CMatrix &matrix, const std::string &name, NLMISC::CRGBA color, float textZOffset, float testSize)
{
	CPSUtil::displayBasis(CNELU::Driver, matrix, NLMISC::CMatrix::Identity, 1.f, *_FontGenerator, _FontManager);	
	CPSUtil::print(CNELU::Driver, name, *_FontGenerator, _FontManager, matrix.getPos() + NLMISC::CVector(0.f, 0.f, textZOffset), testSize,  color);
}



sint CObjectViewer::getCurrentCamera () const
{
	return _CurrentCamera;
}
	
void CObjectViewer::setCurrentCamera (sint currentCamera)
{
	nlassert ((currentCamera == -1) ||(currentCamera < (sint)_Cameras.size ()));
	_CurrentCamera = currentCamera;
}
	
uint CObjectViewer::getCameraInstance (uint cameraId) const
{
	return _Cameras[cameraId];
}

uint CObjectViewer::getNumCamera () const
{
	return (uint)_Cameras.size ();
}

int localizedMessageBox(HWND parentWindow, int messageStringID, int captionStringID, UINT nType)
{
	
	CString caption;
	CString mess;
	caption.LoadString(captionStringID);
	mess.LoadString(messageStringID);
	return MessageBox(parentWindow, (LPCTSTR) mess, (LPCTSTR) caption, nType);
	// TODO : replace older call to ::MessageBox in the object viewer with that function
}

int localizedMessageBox(HWND parentWindow, const char *message, int captionStringID, UINT nType)
{
	CString caption;	
	caption.LoadString(captionStringID);	
	return MessageBox(parentWindow, message, (LPCTSTR) caption, nType);
}

CString getStrRsc(uint stringID)
{
	CString str;
	str.LoadString(stringID);
	return str;
}

bool browseFolder(const CString &caption, CString &destFolder, HWND parent)
{
	char chosenPath[MAX_PATH];
	// browse folder	
	BROWSEINFO bi;		
	bi.hwndOwner = parent;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = chosenPath;
	bi.lpszTitle = (LPCTSTR) caption;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_EDITBOX;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.iImage = 0;
	LPITEMIDLIST result = SHBrowseForFolder(&bi);
	if (result != NULL && SHGetPathFromIDList(result, chosenPath))
	{
		destFolder = chosenPath;
		return true;
	}
	return false;
}


