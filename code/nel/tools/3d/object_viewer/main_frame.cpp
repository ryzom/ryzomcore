// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// main_frame.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "main_frame.h"
#include "set_value_dlg.h"
#include "particle_dlg.h"
#include "about_dialog.h"
#include "choose_frame_delay.h"
#include "choose_bg_color_dlg.h"
#include "choose_sun_color_dlg.h"
#include "day_night_dlg.h"
#include "water_pool_editor.h"
#include "vegetable_dlg.h"
#include "global_wind_dlg.h"
#include "sound_anim_dlg.h"
#include "fog_dlg.h"
#include "scene_rot_dlg.h"
#include "skeleton_scale_dlg.h"
#include "light_group_factor.h"
#include "tune_mrm_dlg.h"
#include "snapshot_tool_dlg.h"
#include "nel/misc/file.h"
#include "nel/3d/nelu.h"
#include "nel/3d/mesh.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/skeleton_model.h"


using namespace NLMISC;
using namespace NL3D;


void CSceneDlgMouseListener::addToServer (NLMISC::CEventServer& server)
{
	server.addListener (EventMouseDownId, this);
}

void CSceneDlgMouseListener::releaseFromServer (NLMISC::CEventServer& server)
{
	server.removeListener (EventMouseDownId, this);
}

void CSceneDlgMouseListener::operator ()(const CEvent& event)
{	
	if (event == EventMouseDownId)
	{
		CEventMouse* mouseEvent=(CEventMouse*)&event;
		if (mouseEvent->Button == rightButton)
		{
			const CEvent3dMouseListener &ml = ObjViewerDlg->getMouseListener() ;

			CMenu  menu ;
			CMenu* subMenu ;
	
			menu.LoadMenu(IDR_MOVE_ELEMENT) ;

			

			menu.CheckMenuItem(ID_ENABLE_ELEMENT_XROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::xAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;
			menu.CheckMenuItem(ID_ENABLE_ELEMENT_YROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::yAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;
			menu.CheckMenuItem(ID_ENABLE_ELEMENT_ZROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::zAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;


			subMenu = menu.GetSubMenu(0);    
			nlassert(subMenu) ;

			// compute the screen coordinate from the main window

			HWND wnd = (HWND) CNELU::Driver->getDisplay() ;
			nlassert(::IsWindow(wnd)) ;
			RECT r ;
			::GetWindowRect(wnd, &r) ;						

			sint x = r.left + (sint) (mouseEvent->X * (r.right - r.left)) ;
			sint y = r.top + (sint) ((1.f - mouseEvent->Y) * (r.bottom - r.top)) ;
			
			::TrackPopupMenu(subMenu->m_hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, SceneDlg->m_hWnd, NULL) ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

//IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

CMainFrame::CMainFrame( CObjectViewer *objView, winProc windowProc )
{
	DriverWindowProc=windowProc;
	ObjView=objView;
	AnimationWindow=false;
	AnimationSetWindow=false;
	MixerSlotsWindow=false;
	ParticlesWindow=false;
	DayNightWindow=false;
	WaterPoolWindow=false;
	VegetableWindow=false;
	GlobalWindWindow= false;
	SoundAnimWindow=false;
	LightGroupWindow=false;
	ChooseFrameDelayWindow=false;
	ChooseBGColorWindow=false;
	ChooseSunColorWindow=false;
	SkeletonScaleWindow= false;
	TuneMRMWindow= false;
	MouseMoveType= MoveCamera;
	MoveMode=ObjectMode;
	X=true;
	Y=true;
	Z=true;
	Euler=false;
	GlobalWindPower= 1.f;
	FogActive = false;
	FogStart  = 0.f;
	FogEnd    = 100.f;
	_LastSceneRotX= 0.0f;
	_LastSceneRotY= 0.0f;
	_LastSceneRotZ= 0.0f;

	_RightButtonMouseListener.ObjViewerDlg = ObjView ;
	_RightButtonMouseListener.SceneDlg = this ;
	_RightButtonMouseListener.addToServer(CNELU::EventServer) ;
}

CMainFrame::~CMainFrame()
{	
	_RightButtonMouseListener.releaseFromServer (CNELU::EventServer);
}


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(IDC_RELOAD_TEXTURES, OnReloadTextures)
	ON_COMMAND(ID_CLEAR, OnClear)
	ON_COMMAND(ID_EDIT_MOVEELEMENT, OnEditMoveelement)
	ON_COMMAND(ID_EDIT_MOVE_FX, OnEditMoveFX)
	ON_COMMAND(ID_EDIT_MOVE_FX_USER_MATRIX, OnEditMoveFXUserMatrix)
	ON_COMMAND(ID_EDIT_X, OnEditX)
	ON_COMMAND(ID_EDIT_Y, OnEditY)
	ON_COMMAND(ID_EDIT_Z, OnEditZ)
	ON_COMMAND(ID_ENABLE_ELEMENT_XROTATE, OnEnableElementXrotate)
	ON_COMMAND(ID_ENABLE_ELEMENT_YROTATE, OnEnableElementYrotate)
	ON_COMMAND(ID_ENABLE_ELEMENT_ZROTATE, OnEnableElementZrotate)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_LOADCONFIG, OnFileLoadconfig)
	ON_COMMAND(IDM_SNAPSHOT_TOOL, OnSnapShotTool)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVECONFIG, OnFileSaveconfig)	
	ON_COMMAND(ID_VIEW_FIRSTPERSONMODE, OnViewFirstpersonmode)
	ON_COMMAND(ID_VIEW_CAMERAMODE, OnViewCamera)
	ON_COMMAND(ID_VIEW_OBJECTMODE, OnViewObjectmode)
	ON_COMMAND(ID_VIEW_RESET_CAMERA, OnResetCamera)
	ON_COMMAND(ID_VIEW_SETBACKGROUND, OnViewSetbackground)
	ON_COMMAND(ID_VIEW_SETMOVESPEED, OnViewSetmovespeed)
	ON_COMMAND(IDM_ACTIVATE_FOG, OnActivateFog)
	ON_COMMAND(IDM_SETUP_FOG, OnSetupFog)
	ON_COMMAND(ID_WINDOW_ANIMATION, OnWindowAnimation)
	ON_COMMAND(ID_WINDOW_ANIMATIONSET, OnWindowAnimationset)
	ON_COMMAND(ID_WINDOW_MIXERSSLOTS, OnWindowMixersslots)
	ON_COMMAND(ID_WINDOW_PARTICLES, OnWindowParticles)
	ON_COMMAND(ID_WINDOW_DAYNIGHT, OnWindowDayNight)
	ON_COMMAND(ID_WINDOW_WATER_POOL, OnWindowWaterPool)
	ON_COMMAND(ID_WINDOW_ANIMSOUND, OnWindowSoundAnim)
	ON_COMMAND(ID_WINDOW_CHOOSE_FRAME_DELAY, OnWindowChooseFrameDelay)	
	ON_COMMAND(ID_WINDOW_CHOOSE_BG_COLOR, OnWindowChooseBGColor)
	ON_COMMAND(ID_WINDOW_CHOOSE_SUN_COLOR, OnWindowChooseSunColor)
	ON_COMMAND(ID_SCENE_SETLIGHTGROUPFACTOR, OnSetLightGroupFactor)
	ON_COMMAND(IDM_SHOW_SCENE_MATRIX, OnShowSceneMatrix)
	ON_COMMAND(iDM_SHOW_OCCLUSION_TEST_MESHS, OnShowOcclusionTestMeshs)
	ON_COMMAND(IDM_SHOW_FX_MATRIX, OnShowFXMatrix)
	ON_COMMAND(IDM_SHOW_FX_USER_MATRIX, OnShowFXUserMatrix)
	ON_UPDATE_COMMAND_UI(IDM_SHOW_SCENE_MATRIX, OnUpdateShowSceneMatrix)
	ON_UPDATE_COMMAND_UI(IDM_SHOW_FX_MATRIX, OnUpdateShowFXMatrix)
	ON_UPDATE_COMMAND_UI(IDM_SHOW_FX_USER_MATRIX, OnUpdateShowFXUserMatrix)
	ON_UPDATE_COMMAND_UI(iDM_SHOW_OCCLUSION_TEST_MESHS, OnUpdateShowOcclusionTestMeshs)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ANIMATION, OnUpdateWindowAnimation)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ANIMATIONSET, OnUpdateWindowAnimationset)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_MIXERSSLOTS, OnUpdateWindowMixersslots)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PARTICLES, OnUpdateWindowParticles)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_DAYNIGHT, OnUpdateWindowDayNight)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_WATER_POOL, OnUpdateWindowWaterPool)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ANIMSOUND, OnUpdateWindowSoundAnim)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CHOOSE_FRAME_DELAY, OnUpdateWindowChooseFrameDelay)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CHOOSE_BG_COLOR, OnUpdateWindowBGColor)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CHOOSE_SUN_COLOR, OnUpdateWindowSunColor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTMODE, OnUpdateViewObjectmode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTPERSONMODE, OnUpdateViewFirstpersonmode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERAMODE, OnUpdateViewCamera)
	ON_UPDATE_COMMAND_UI(ID_EDIT_X, OnUpdateEditX)
	ON_UPDATE_COMMAND_UI(ID_EDIT_Y, OnUpdateEditY)
	ON_UPDATE_COMMAND_UI(ID_EDIT_Z, OnUpdateEditZ)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVEELEMENT, OnUpdateEditMoveelement)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_FX, OnUpdateEditMoveFX)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_FX_USER_MATRIX, OnUpdateEditMoveFXUserMatrix)
	ON_UPDATE_COMMAND_UI(ID_SCENE_SETLIGHTGROUPFACTOR, OnUpdateWindowLightGroup)
	ON_COMMAND(ID_HELP_ABOUTOBJECTVIEWER, OnHelpAboutobjectviewer)	
	ON_COMMAND(IDM_REMOVE_ALL_INSTANCES_FROM_SCENE, OnRemoveAllInstancesFromScene)	
	ON_COMMAND_RANGE(IDM_ACTIVATE_TEXTURE_SET_1, IDM_ACTIVATE_TEXTURE_SET_8, OnActivateTextureSet)
	ON_COMMAND(IDM_SHUFFLE_TEXTURE_SET, OnShuffleTextureSet)	
	ON_COMMAND(ID_WINDOW_VEGETABLE, OnWindowVegetable)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_VEGETABLE, OnUpdateWindowVegetable)
	ON_COMMAND(ID_WINDOW_GLOBALWIND, OnWindowGlobalwind)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_GLOBALWIND, OnUpdateWindowGlobalwind)
	ON_COMMAND(ID_EDIT_MOVE_OBJECT_LIGHT_TEST, OnEditMoveObjectLightTest)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_OBJECT_LIGHT_TEST, OnUpdateEditMoveObjectLightTest)
	ON_COMMAND(ID_EDIT_MOVECAMERA, OnEditMovecamera)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVECAMERA, OnUpdateEditMovecamera)
	ON_COMMAND(ID_EDIT_MOVESCENE, OnEditMovescene)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVESCENE, OnUpdateEditMovescene)
	ON_COMMAND(ID_VIEW_RESET_SCENE_ROOT, OnViewResetSceneRoot)
	ON_COMMAND(ID_VIEW_RESET_FX_ROOT, OnViewResetFXRoot)
	ON_COMMAND(IDM_RESET_FX_USER_MATRIX, OnViewResetFXUserMatrix)	
	ON_COMMAND(ID_VIEW_SET_SCENE_ROTATION, OnViewSetSceneRotation)
	ON_COMMAND(ID_SHOOT_SCENE, OnShootScene)
	ON_COMMAND(ID_WINDOW_SKELETON_SCALE, OnWindowSkeletonScale)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SKELETON_SCALE, OnUpdateWindowSkeletonScale)
	ON_COMMAND(ID_WINDOW_TUNE_MRM, OnWindowTuneMRM)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TUNE_MRM, OnUpdateWindowTuneMRM)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_SCENE_CAMERA_FIRST, ID_SCENE_CAMERA_LAST, OnSceneCamera)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SCENE_CAMERA_FIRST, ID_SCENE_CAMERA_LAST, OnUpdateSceneCamera)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (CNELU::Driver)
		DriverWindowProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
		
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

void CMainFrame::update ()
{
	ObjView->_AnimationDlg->ShowWindow (AnimationWindow?SW_SHOW:SW_HIDE);
	ObjView->_AnimationSetDlg->ShowWindow (AnimationSetWindow?SW_SHOW:SW_HIDE);
	ObjView->_SlotDlg->ShowWindow (MixerSlotsWindow?SW_SHOW:SW_HIDE);
	ObjView->_ParticleDlg->ShowWindow (ParticlesWindow?SW_SHOW:SW_HIDE);
	ObjView->_DayNightDlg->ShowWindow (DayNightWindow?SW_SHOW:SW_HIDE);
	ObjView->_WaterPoolDlg->ShowWindow (WaterPoolWindow?SW_SHOW:SW_HIDE);
	ObjView->_VegetableDlg->ShowWindow (VegetableWindow?SW_SHOW:SW_HIDE);
	ObjView->_GlobalWindDlg->ShowWindow (GlobalWindWindow?SW_SHOW:SW_HIDE);
	ObjView->_SoundAnimDlg->ShowWindow (SoundAnimWindow?SW_SHOW:SW_HIDE);
	ObjView->_LightGroupDlg->ShowWindow (LightGroupWindow?SW_SHOW:SW_HIDE);
	ObjView->_ChooseFrameDelayDlg->ShowWindow (ChooseFrameDelayWindow?SW_SHOW:SW_HIDE);
	ObjView->_ChooseBGColorDlg->ShowWindow (ChooseBGColorWindow?SW_SHOW:SW_HIDE);
	ObjView->_ChooseSunColorDlg->ShowWindow (ChooseSunColorWindow?SW_SHOW:SW_HIDE);
	ObjView->_SkeletonScaleDlg->ShowWindow (SkeletonScaleWindow?SW_SHOW:SW_HIDE);
	ObjView->_TuneMRMDlg->ShowWindow (TuneMRMWindow?SW_SHOW:SW_HIDE);
}

// ***************************************************************************

void CMainFrame::registerValue (bool read)
{
	if (read)
	{
		// Get value from the register
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
		{
			DWORD len=sizeof (BOOL);
			DWORD type;
			NLMISC::CRGBA bgCol ;
			RegQueryValueEx (hKey, _T("ViewAnimation"), 0, &type, (LPBYTE)&AnimationWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewAnimationSet"), 0, &type, (LPBYTE)&AnimationSetWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewSlots"), 0, &type, (LPBYTE)&MixerSlotsWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewParticles"), 0, &type, (LPBYTE)&ParticlesWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewDayNight"), 0, &type, (LPBYTE)&DayNightWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewWaterPool"), 0, &type, (LPBYTE)&WaterPoolWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewVegetable"), 0, &type, (LPBYTE)&VegetableWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewGlobalWind"), 0, &type, (LPBYTE)&GlobalWindWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewSoundAnimWind"), 0, &type, (LPBYTE)&GlobalWindWindow, &len);
			len=sizeof (float);
			RegQueryValueEx (hKey, _T("MoveSpeed"), 0, &type, (LPBYTE)&MoveSpeed, &len);
			len=sizeof (uint);
			RegQueryValueEx (hKey, _T("ObjectMode"), 0, &type, (LPBYTE)&MoveMode, &len);
			len=sizeof(NLMISC::CRGBA) ;
			RegQueryValueEx (hKey, _T("BackGroundColor"), 0, &type, (LPBYTE)&BgColor, &len);
			len=sizeof (float);
			RegQueryValueEx (hKey, _T("GlobalWindPower"), 0, &type, (LPBYTE)&GlobalWindPower, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewChooseFrameDelay"), 0, &type, (LPBYTE)&ChooseFrameDelayWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewChooseBGColor"), 0, &type, (LPBYTE)&ChooseBGColorWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewChooseSunColor"), 0, &type, (LPBYTE)&ChooseSunColorWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewSkeletonScaleWindow"), 0, &type, (LPBYTE)&SkeletonScaleWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, _T("ViewTuneMRMWindow"), 0, &type, (LPBYTE)&TuneMRMWindow, &len);
		}
	}
	else
	{
		HKEY hKey;
		if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, &hKey)==ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, _T("ViewAnimation"), 0, REG_BINARY, (LPBYTE)&AnimationWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewAnimationSet"), 0, REG_BINARY, (LPBYTE)&AnimationSetWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewSlots"), 0, REG_BINARY, (LPBYTE)&MixerSlotsWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewParticles"), 0, REG_BINARY, (LPBYTE)&ParticlesWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewDayNight"), 0, REG_BINARY, (LPBYTE)&DayNightWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewWaterPool"), 0, REG_BINARY, (LPBYTE)&WaterPoolWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewDayNight"), 0, REG_BINARY, (LPBYTE)&DayNightWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewVegetable"), 0, REG_BINARY, (LPBYTE)&VegetableWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewGlobalWind"), 0, REG_BINARY, (LPBYTE)&GlobalWindWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewSoundAnimWind"), 0, REG_BINARY, (LPBYTE)&SoundAnimWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewLightGroupWind"), 0, REG_BINARY, (LPBYTE)&LightGroupWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewChooseFrameDelay"), 0, REG_BINARY, (LPBYTE)&ChooseFrameDelayWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewChooseBGColor"), 0, REG_BINARY, (LPBYTE)&ChooseBGColorWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewChooseSunColor"), 0, REG_BINARY, (LPBYTE)&ChooseSunColorWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("MoveSpeed"), 0, REG_BINARY, (LPBYTE)&MoveSpeed, sizeof(float));
			RegSetValueEx(hKey, _T("ObjectMode"), 0, REG_BINARY, (LPBYTE)&MoveMode, sizeof(uint));
			RegSetValueEx(hKey, _T("BackGroundColor"), 0, REG_BINARY, (LPBYTE)&BgColor, sizeof(NLMISC::CRGBA));
			RegSetValueEx(hKey, _T("GlobalWindPower"), 0, REG_BINARY, (LPBYTE)&GlobalWindPower, sizeof(float));
			RegSetValueEx(hKey, _T("ViewSkeletonScaleWindow"), 0, REG_BINARY, (LPBYTE)&SkeletonScaleWindow, sizeof(bool));
			RegSetValueEx(hKey, _T("ViewTuneMRMWindow"), 0, REG_BINARY, (LPBYTE)&TuneMRMWindow, sizeof(bool));
		}
	}
}

// ***************************************************************************

void CMainFrame::OnResetCamera() 
{
	// One object found at least
	bool found=false;
	
	// Pointer on the CMesh;
	CVector hotSpot=CVector (0,0,0);

	// Reset the radius
	float radius=10.f;

	// Look for a first mesh
	uint m;
	for (m=0; m<ObjView->_ListInstance.size(); m++)
	{
		CTransformShape *pTransform=dynamic_cast<CTransformShape*>(ObjView->_ListInstance[m]->TransformShape);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			// Get bounding box
			CAABBox boundingBox;
			pShape->getAABBox(boundingBox);
			
			if (!pSkelModel)
			{
				// Reset the hotspot
				hotSpot=pTransform->getMatrix()*boundingBox.getCenter();

				// Reset the radius
				radius=boundingBox.getRadius();
				radius=pTransform->getMatrix().mulVector (CVector (radius, 0, 0)).norm();
				found=true;
				m++;
				break;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Ok, it is the root.
					hotSpot=pSkelModel->Bones[0].getMatrix()*boundingBox.getCenter();

					// Reset the radius
					radius=boundingBox.getRadius();
					radius=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius, 0, 0)).norm();
					found=true;
					m++;
					break;
				}
			}
		}
	}

	// For each model in the list
	for (; m<ObjView->_ListInstance.size(); m++)
	{
		// Pointer on the CTransformShape;
		CTransformShape *pTransform=dynamic_cast<CTransformShape*>(ObjView->_ListInstance[m]->TransformShape);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			// New radius and hotSpot
			CVector hotSpot2;
			float radius2;
			bool setuped=false;

			// Get the bounding box
			CAABBox boundingBox;
			pShape->getAABBox(boundingBox);

			if (!pSkelModel)
			{
				// Get the hotspot
				hotSpot2=pTransform->getMatrix()*boundingBox.getCenter();

				// Get the radius
				radius2=boundingBox.getRadius();
				radius2=pTransform->getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

				// Ok found it
				setuped=true;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Get the hotspot
					hotSpot2=pSkelModel->Bones[0].getMatrix()*boundingBox.getCenter();

					// Get the radius
					radius2=boundingBox.getRadius();
					radius2=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

					// Ok found it
					setuped=true;
				}
			}

			if (setuped)
			{
				// *** Merge with previous

				// Get vector center to center
				CVector vect=hotSpot-hotSpot2;
				vect.normalize();
				
				// Get the right position
				CVector right=hotSpot+vect*radius;
				if ((right-hotSpot2).norm()<radius2)
					right=hotSpot2+vect*radius2;
				
				// Get the left position
				CVector left=hotSpot2-vect*radius2;
				if ((left-hotSpot).norm()<radius)
					left=hotSpot-vect*radius;

				// Get new center
				hotSpot=(left+right)/2.f;

				// Get new size
				radius=(left-right).norm()/2.f;
			}
		}
	}

	// Setup scene center
	ObjView->_SceneCenter=hotSpot;

	// Setup camera
	CNELU::Camera->lookAt (hotSpot+CVector(0.57735f,0.57735f,0.57735f)*radius, hotSpot);

	// Setup mouse listener
	ObjView->_MouseListener.setMatrix (CNELU::Camera->getMatrix());
	ObjView->_MouseListener.setFrustrum (CNELU::Camera->getFrustum());
	ObjView->_MouseListener.setViewport (CViewport());
	ObjView->_MouseListener.setHotSpot (hotSpot);
	ObjView->_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);

	// reset ObjectLightTest.
	ObjView->_ObjectLightTestMatrix.setPos(hotSpot);
}

void CMainFrame::OnClear() 
{
	// *** Clear the scene.

	// Remove all the instance
	ObjView->removeAllInstancesFromScene();
	ToolBar.Invalidate();
}

void CMainFrame::OnReloadTextures()
{
	// Reload all the textures
	ObjView->reloadTextures ();
}

void CMainFrame::OnEditX() 
{
	X^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::xAxis, X!=0);
}

void CMainFrame::OnEditY() 
{
	Y^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::yAxis, Y!=0);
}

void CMainFrame::OnEditZ() 
{
	Z^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::zAxis, Z!=0);
}

void CMainFrame::OnEnableElementXrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::xAxis);
}

void CMainFrame::OnEnableElementYrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::yAxis);
}

void CMainFrame::OnEnableElementZrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::zAxis);
}

void CMainFrame::OnFileExit() 
{
	DestroyWindow ();
}

void CMainFrame::OnFileLoadconfig() 
{
	// Update UI
	update ();

	// Create a dialog
	static TCHAR BASED_CODE szFilter[] = _T("NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||");
	CFileDialog fileDlg( TRUE, _T(".ovcgf"), _T("*.ovcgf"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		CIFile file;
		if (file.open(tStrToUtf8(fileDlg.GetPathName())))
		{
			try
			{
				ObjView->serial (file);
				if (!ObjView->ParticleWorkspaceFilename.empty())
				{
					CParticleDlg *pd = ObjView->getParticleDialog();
					pd->checkModifiedWorkSpace();
					pd->loadWorkspace(ObjView->ParticleWorkspaceFilename);										
					if (pd->getParticleWorkspace())
					{
						pd->getParticleWorkspace()->restickAllObjects(ObjView);
					}
				}
			}
			catch (const Exception& e)
			{
				MessageBox(nlUtf8ToTStr(e.what()), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			CString msg;
			msg.Format(_T("Can't open the file %s for reading."), (LPCTSTR)fileDlg.GetPathName());
			MessageBox (msg, _T("NeL object viewer"), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

#include <dlgs.h>       // for standard control IDs for commdlg

void CMainFrame::OnFileOpen() 
{
	// update UI
	update ();

	// Create a dialog
	static TCHAR BASED_CODE szFilter[] = 
		_T("All NeL Files (*.shape;*.ps;*.ig)\0*.shape;*.ps;*.ig\0")
		_T("NeL Shape Files (*.shape)\0*.shape\0")
		_T("NeL Particule System Files (*.ps)\0*.ps\0")
		_T("NeL Instance Group Files (*.ig)\0*.ig\0")
		_T("All Files (*.*)\0*.*\0\0");

	// Filename buffer
	TCHAR buffer[65535];
	buffer[0]=0;

	OPENFILENAME openFile;
	memset (&openFile, 0, sizeof (OPENFILENAME));
	openFile.lStructSize = sizeof (OPENFILENAME);
	openFile.hwndOwner = this->m_hWnd;
    openFile.lpstrFilter = szFilter;
    openFile.nFilterIndex = 0;
    openFile.lpstrFile = buffer;
    openFile.nMaxFile = 65535;
    openFile.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER;
    openFile.lpstrDefExt = _T("*.shape;*.ig;*.ps");
	

	if (GetOpenFileName(&openFile))
	{		
		// Build an array of name
		std::vector<std::string> meshFilename;

		// Filename pointer
		TCHAR *c=buffer;

		// Read the path
		CString path = buffer;
		if (path.GetLength()>openFile.nFileOffset)
		{
			// Double zero at the end
			c[path.GetLength()+1]=0;

			// Path is empty
			path.Empty();
		}
		else
		{
			// Adda slash
			path += "\\";

			// Look for the next string
			while (*(c++)) {}
		}

		// For each file selected
		while (*c)
		{
			// File name
			char filename[256];
			char *ptr=filename;

			// Read a file name
			while (*c)
			{
				*(ptr++)=*(c++);
			}
			*ptr=0;
			c++;

			// File name
			CString name = path + filename;

			// file is an ig ?
			if (name.Find(_T(".ig")) != -1)
			{
				// Load the instance group
				if (ObjView->loadInstanceGroup (tStrToUtf8(name)))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
			else
			{
				// Add it in the array
				meshFilename.push_back (tStrToUtf8(name));
			}
		}

		// Some mesh to load ?
		if ( !meshFilename.empty() )
		{	
			// Create a dialog for the skel
			static TCHAR BASED_CODE szFilter2[] = _T("NeL Skeleton Files (*.skel)|*.skel|All Files (*.*)|*.*||");
			CFileDialog fileDlg2 ( TRUE, _T(".skel"), _T("*.skel"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter2);
			if (fileDlg2.DoModal()==IDOK)
			{
				// Load the shape with a skeleton
				if (ObjView->loadMesh (meshFilename, tStrToUtf8(fileDlg2.GetPathName())))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
			else
			{
				// Load the shape without skeleton
				if (ObjView->loadMesh (meshFilename, ""))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
		}
	}
}

void CMainFrame::OnFileSaveconfig() 
{
	// Update UI
	update ();

	// Create a dialog
	static TCHAR BASED_CODE szFilter[] = _T("NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||");
	CFileDialog fileDlg( FALSE, _T(".ovcgf"), _T("*.ovcgf"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{		
		ObjView->ParticleWorkspaceFilename.clear();
		CParticleWorkspace *pw = ObjView->getParticleDialog()->getParticleWorkspace();
		if (pw && pw->getNumNode() != 0)
		{
			if (localizedMessageBox(*this, IDS_INCLUDE_PARTICLE_WORKSPACE_INFOS, IDS_OBJECT_VIEWER, MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				ObjView->ParticleWorkspaceFilename = pw->getFilename();
			}
		}
		// Open the file
		COFile file;
		if (file.open (tStrToUtf8(fileDlg.GetPathName())))
		{
			try
			{
				ObjView->serial(file);
			}
			catch (const Exception& e)
			{
				MessageBox(nlUtf8ToTStr(e.what()), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			CString msg;
			msg.Format(_T("Can't open the file %s for writing"), (LPCTSTR)fileDlg.GetPathName());
			MessageBox (msg, _T("NeL object viewer"), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}



void CMainFrame::OnViewFirstpersonmode() 
{
	MoveMode=FirstMode;
	ToolBar.Invalidate ();
}

void CMainFrame::OnViewObjectmode() 
{
	MoveMode=ObjectMode;
	ToolBar.Invalidate ();
}

void CMainFrame::OnViewCamera() 
{
	MoveMode=CameraMode;
	ToolBar.Invalidate ();
}

void CMainFrame::OnViewSetbackground() 
{
	static COLORREF colTab[16] = { 0, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
								   , 0x7f7f7f, 0xff7f7f, 0x7fff7f, 0xffff7f, 0x7f7fff, 0xff7fff, 0x7fffff, 0xff7f00 } ;	
	BgColor = ObjView->getBackGroundColor() ;
	CHOOSECOLOR cc ;
	cc.lStructSize = sizeof(CHOOSECOLOR) ;
	cc.hwndOwner = this->m_hWnd ;
	cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN  ;	
	cc.rgbResult = RGB(BgColor.R, BgColor.G, BgColor.B) ;
	cc.lpCustColors = colTab ;

	if (::ChooseColor(&cc) == IDOK)
	{		
		BgColor.R = (uint8) (cc.rgbResult & 0xff) ;
		BgColor.G = (uint8) ((cc.rgbResult & 0xff00) >> 8) ;
		BgColor.B = (uint8) ((cc.rgbResult & 0xff0000) >> 16) ;
	
		ObjView->setBackGroundColor(BgColor) ;
	}	
}

void CMainFrame::OnViewSetmovespeed() 
{
	// Set value
	CSetValueDlg valueDlg;

	// Set default value
	valueDlg.Value=toString (MoveSpeed).c_str();
	valueDlg.Title="Select your move speed";

	// Open dialog
	if (valueDlg.DoModal ()==IDOK)
	{
		// Get deflaut value
		NLMISC::fromString(tStrToUtf8(valueDlg.Value), MoveSpeed);
	}
}

void CMainFrame::OnActivateFog() 
{	
	FogActive = !FogActive;
	if (FogActive)
	{			
		CNELU::Driver->setupFog(FogStart, FogEnd, ObjView->getBackGroundColor());
		CNELU::Driver->enableFog(true);
	}
	else
	{
		CNELU::Driver->enableFog(false);
	}
	CMenu *menu = GetMenu();
	menu->CheckMenuItem(IDM_ACTIVATE_FOG, MF_BYCOMMAND | (FogActive ? MF_CHECKED : MF_UNCHECKED));
}

void CMainFrame::OnSetupFog() 
{	
	CFogDlg fogDlg;
	fogDlg.setFogStart(FogStart);
	fogDlg.setFogEnd(FogEnd);

	if (fogDlg.DoModal() == IDOK)
	{
		FogStart = fogDlg.getFogStart();
		FogEnd = fogDlg.getFogEnd();
		NLMISC::clamp(FogStart, 0.f, 1000.f);
		NLMISC::clamp(FogEnd, 0.f, 1000.f);
		CNELU::Driver->setupFog(FogStart, FogEnd, ObjView->getBackGroundColor());
	}
}


void CMainFrame::OnShowSceneMatrix()
{
	ObjView->setSceneMatrixVisible(!ObjView->getSceneMatrixVisible());
}

void CMainFrame::OnShowFXMatrix()
{
	ObjView->setFXMatrixVisible(!ObjView->getFXMatrixVisible());
}

void CMainFrame::OnShowFXUserMatrix()
{
	ObjView->setFXUserMatrixVisible(!ObjView->getFXUserMatrixVisible());
}

void CMainFrame::OnShowOcclusionTestMeshs()
{
	ObjView->setOcclusionTestMeshsVisible(!ObjView->getOcclusionTestMeshsVisible());
}


void CMainFrame::OnUpdateShowSceneMatrix(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(ObjView->getSceneMatrixVisible());
}

void CMainFrame::OnUpdateShowFXMatrix(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(ObjView->getFXMatrixVisible());
}

void CMainFrame::OnUpdateShowFXUserMatrix(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(ObjView->getFXUserMatrixVisible());
}

void CMainFrame::OnUpdateShowOcclusionTestMeshs(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(ObjView->getOcclusionTestMeshsVisible());
}



void CMainFrame::OnWindowAnimation() 
{
	AnimationWindow^=true;
	update ();
}

void CMainFrame::OnWindowAnimationset() 
{
	AnimationSetWindow^=true;
	update ();
}

void CMainFrame::OnWindowMixersslots() 
{
	MixerSlotsWindow^=true;
	update ();
}

void CMainFrame::OnWindowParticles() 
{
	ParticlesWindow^=true;
	update ();
}

void CMainFrame::OnWindowDayNight() 
{
	DayNightWindow^=true;
	update ();
}

void CMainFrame::OnWindowVegetable() 
{
	VegetableWindow^=true;
	update ();
}


void CMainFrame::OnWindowWaterPool() 
{
	WaterPoolWindow^=true;
	update ();
}


void CMainFrame::OnWindowGlobalwind() 
{
	GlobalWindWindow^= true;
	update ();
}

void CMainFrame::OnWindowSoundAnim() 
{
	SoundAnimWindow^= true;
	update ();
}

void CMainFrame::OnWindowChooseFrameDelay() 
{
	ChooseFrameDelayWindow^= true;
	update ();
}

void CMainFrame::OnWindowChooseBGColor() 
{
	ChooseBGColorWindow^= true;
	update ();
}

void CMainFrame::OnWindowChooseSunColor() 
{
	ChooseSunColorWindow^= true;
	update ();
}




void CMainFrame::OnSetLightGroupFactor() 
{
	LightGroupWindow^= true;
	update ();
}

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	/*ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,*/
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Parent create
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create tool bar
	if (!ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!ToolBar.LoadToolBar(IDR_TOOL_EDIT))
	{
		return -1;      // fail to create
	}

	// Create the status bar
	StatusBar.Create (this);
	StatusBar.SetIndicators (indicators,
		  sizeof(indicators)/sizeof(UINT));

	// Docking
	ToolBar.SetButtonStyle (0, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (1, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (2, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (4, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (5, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (6, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (8, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (9, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (10, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (11, TBBS_CHECKGROUP);
	ToolBar.EnableDocking(CBRS_ALIGN_ANY);

	InitialUpdateFrame (NULL, TRUE);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&ToolBar);

	return 0;
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CMainFrame::OnUpdateWindowAnimation(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (AnimationWindow);
}

void CMainFrame::OnUpdateWindowAnimationset(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (AnimationSetWindow);
}

void CMainFrame::OnUpdateWindowMixersslots(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MixerSlotsWindow);
}

void CMainFrame::OnUpdateWindowParticles(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ParticlesWindow);
}

void CMainFrame::OnUpdateWindowDayNight(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (DayNightWindow);
}

void CMainFrame::OnUpdateWindowWaterPool(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (WaterPoolWindow);
}

void CMainFrame::OnUpdateWindowVegetable(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (VegetableWindow);
}

void CMainFrame::OnUpdateWindowGlobalwind(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GlobalWindWindow);
}

void CMainFrame::OnUpdateWindowSoundAnim(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (SoundAnimWindow);
}

void CMainFrame::OnUpdateWindowChooseFrameDelay(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (ChooseFrameDelayWindow);
}

void CMainFrame::OnUpdateWindowBGColor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (ChooseBGColorWindow);
}

void CMainFrame::OnUpdateWindowSunColor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck (ChooseSunColorWindow);
}

void CMainFrame::OnUpdateWindowLightGroup(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (LightGroupWindow);
}

void CMainFrame::OnUpdateViewObjectmode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MoveMode == ObjectMode);
}

void CMainFrame::OnUpdateViewFirstpersonmode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MoveMode == FirstMode);
}

void CMainFrame::OnUpdateViewCamera(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MoveMode == CameraMode);
	pCmdUI->Enable (ObjView->getNumCamera () != 0);
}

void CMainFrame::OnUpdateEditX(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (X);
}

void CMainFrame::OnUpdateEditY(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (Y);
}

void CMainFrame::OnUpdateEditZ(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (Z);
}

void CMainFrame::OnHelpAboutobjectviewer() 
{
	CAboutDialog about;
	about.DoModal();
}


///===========================================================================================
void CMainFrame::OnRemoveAllInstancesFromScene()
{
	if (MessageBox(_T("Delete all instances from scene ?"), _T("Object Viewer"), MB_YESNO) == IDYES)
	{
		ObjView->removeAllInstancesFromScene();
		
		// Reset the camera
		OnResetCamera();

		// Touch the channel mixer
		ObjView->reinitChannels ();
	}
}

///===========================================================================================
void CMainFrame::OnActivateTextureSet(UINT nID)
{	
	/// convert ID to index
	static const uint convIndex[] =
	{ 
		0, 3, 1, 2, 4, 5, 6, 7	
	};

	ObjView->activateTextureSet(convIndex[nID - IDM_ACTIVATE_TEXTURE_SET_1]);	
}


void CMainFrame::OnShuffleTextureSet()
{
	ObjView->shuffleTextureSet();
}


// ***************************************************************************
// ***************************************************************************
// Mouse Edit Mode
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CMainFrame::OnEditMovecamera() 
{
	// no op if already the case
	if(isMoveCamera())
		return;

	MouseMoveType= MoveCamera;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(false) ;
	ObjView->getMouseListener().enableTranslateXYInWorld(false);
}

void CMainFrame::OnEditMovescene() 
{
	// no op if already the case
	if(isMoveSceneRoot())
		return;

	MouseMoveType= MoveSceneRoot;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(true) ;
	ObjView->getMouseListener().enableTranslateXYInWorld(false);
	ObjView->getMouseListener().setModelMatrix(ObjView->_SceneRoot->getMatrix()) ;
	// Each move must be multiplied by identity
	ObjView->getMouseListener().setModelMatrixTransformMove(CMatrix::Identity);
}

void CMainFrame::OnEditMoveelement() 
{
	// no op if already the case
	if(isMoveElement())
		return;

	MouseMoveType= MoveElement;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(true) ;
	ObjView->getMouseListener().enableTranslateXYInWorld(false);
	ObjView->getMouseListener().setModelMatrix(ObjView->getParticleDialog()->getElementMatrix()) ;
	// Each move must be multiplied by inverse of scene root matrix.
	//ObjView->getMouseListener().setModelMatrixTransformMove(ObjView->_SceneRoot->getMatrix().inverted());
	ObjView->getMouseListener().setModelMatrixTransformMove(CMatrix::Identity);

	/*ctrl->EnableXCtrl.EnableWindow(MoveElement) ;
	EnableYCtrl.EnableWindow(MoveElement) ;
	EnableZCtrl.EnableWindow(MoveElement) ;*/
}

void CMainFrame::OnEditMoveFX() 
{
	// no op if already the case
	if(isMoveFX())
		return;

	MouseMoveType= MoveFX;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(true) ;
	ObjView->getMouseListener().enableTranslateXYInWorld(false);
	ObjView->getMouseListener().setModelMatrix(ObjView->getParticleDialog()->getPSWorldMatrix()) ;
	// Each move must be multiplied by inverese of scene root matrix.
	//ObjView->getMouseListener().setModelMatrixTransformMove(ObjView->_SceneRoot->getMatrix().inverted());
	ObjView->getMouseListener().setModelMatrixTransformMove(CMatrix::Identity);

	/*ctrl->EnableXCtrl.EnableWindow(MoveElement) ;
	EnableYCtrl.EnableWindow(MoveElement) ;
	EnableZCtrl.EnableWindow(MoveElement) ;*/
}

void CMainFrame::OnEditMoveFXUserMatrix() 
{
	// no op if already the case
	if(isMoveFXUserMatrix())
		return;

	MouseMoveType= MoveFXUserMatrix;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(true) ;
	ObjView->getMouseListener().enableTranslateXYInWorld(false);
	ObjView->getMouseListener().setModelMatrix(ObjView->getFXUserMatrix());
	// Each move must be multiplied by inverese of scene root matrix.
	//ObjView->getMouseListener().setModelMatrixTransformMove(ObjView->_SceneRoot->getMatrix().inverted());
	ObjView->getMouseListener().setModelMatrixTransformMove(CMatrix::Identity);

	/*ctrl->EnableXCtrl.EnableWindow(MoveElement) ;
	EnableYCtrl.EnableWindow(MoveElement) ;
	EnableZCtrl.EnableWindow(MoveElement) ;*/
}



void CMainFrame::OnEditMoveObjectLightTest() 
{
	// no op if already the case
	if(isMoveObjectLightTest())
		return;

	MouseMoveType= MoveObjectLightTest;
	UpdateData() ;
	ToolBar.Invalidate ();

	ObjView->getMouseListener().enableModelMatrixEdition(true) ;
	// Better to move in XY world plane.
	ObjView->getMouseListener().enableTranslateXYInWorld(true);
	ObjView->getMouseListener().setModelMatrix(ObjView->_ObjectLightTestMatrix) ;
	// Each move must be multiplied by inverese of scene root matrix.
	ObjView->getMouseListener().setModelMatrixTransformMove(ObjView->_SceneRoot->getMatrix().inverted());
}

// ***************************************************************************
void CMainFrame::OnUpdateEditMovecamera(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveCamera());
}

void CMainFrame::OnUpdateEditMovescene(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveSceneRoot());
}

void CMainFrame:: OnUpdateEditMoveelement(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveElement());
}

void CMainFrame::OnUpdateEditMoveFX(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveFX());
	if (ObjView->getParticleDialog())
	{	
		pCmdUI->Enable(!ObjView->getParticleDialog()->isPSStickedToSkeleton());
	}
}

void CMainFrame::OnUpdateEditMoveFXUserMatrix(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveFXUserMatrix());	
}


void CMainFrame::OnUpdateEditMoveObjectLightTest(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (isMoveObjectLightTest());
}


void CMainFrame::OnViewResetSceneRoot() 
{
	CMatrix	ident;
	ObjView->_SceneRoot->setTransformMode(ITransformable::DirectMatrix);
	ObjView->_SceneRoot->setMatrix(ident);

	if(isMoveSceneRoot())
	{
		ObjView->_MouseListener.setModelMatrix (ident);
	}
}

// ***************************************************************************
void CMainFrame::OnViewResetFXRoot() 
{
	CMatrix	ident;
	ObjView->_ParticleDlg->setPSMatrix(ident);
	if(isMoveFX())
	{
		ObjView->_MouseListener.setModelMatrix (ident);
	}
}

// ***************************************************************************
void CMainFrame::OnViewResetFXUserMatrix() 
{
	CMatrix	ident;
	ObjView->setFXUserMatrix(ident);
	if(isMoveFXUserMatrix())
	{
		ObjView->_MouseListener.setModelMatrix (ident);
	}
}


// ***************************************************************************
void CMainFrame::OnViewSetSceneRotation() 
{
	CSceneRotDlg	sceneRotDlg(this);
	sceneRotDlg.RotX= toString(_LastSceneRotX).c_str();
	sceneRotDlg.RotY= toString(_LastSceneRotY).c_str();
	sceneRotDlg.RotZ= toString(_LastSceneRotZ).c_str();
	if (sceneRotDlg.DoModal() == IDOK)
	{
		// read value.
		NLMISC::fromString(tStrToUtf8(sceneRotDlg.RotX), _LastSceneRotX);
		NLMISC::fromString(tStrToUtf8(sceneRotDlg.RotY), _LastSceneRotY);
		NLMISC::fromString(tStrToUtf8(sceneRotDlg.RotZ), _LastSceneRotZ);

		float	rotx= degToRad(_LastSceneRotX);
		float	roty= degToRad(_LastSceneRotY);
		float	rotz= degToRad(_LastSceneRotZ);

		CMatrix	mat;
		mat.rotate(CVector(rotx, roty, rotz), CMatrix::ZXY);
		ObjView->_SceneRoot->setTransformMode(ITransformable::DirectMatrix);
		ObjView->_SceneRoot->setMatrix(mat);

		if(isMoveSceneRoot())
		{
			ObjView->_MouseListener.setModelMatrix (mat);
		}
	}
}

// ***************************************************************************

void CMainFrame::OnShootScene() 
{
	ObjView->shootScene ();
}

// ***************************************************************************

void CMainFrame::OnSceneCamera (UINT id)
{
	MoveMode = CameraMode;
	ObjView->setCurrentCamera (id-ID_SCENE_CAMERA_FIRST);
	ToolBar.UpdateDialogControls (this, TRUE);
	UpdateDialogControls (this, TRUE);
	UpdateData (FALSE);
}

// ***************************************************************************

void CMainFrame::OnUpdateSceneCamera(CCmdUI* pCmdUI) 
{
	bool checked = ObjView->getCurrentCamera () == (sint)(pCmdUI->m_nID - ID_SCENE_CAMERA_FIRST);
	bool exist = (pCmdUI->m_nID - ID_SCENE_CAMERA_FIRST) < ObjView->getNumCamera ();
	pCmdUI->SetCheck ((checked && exist)?TRUE:FALSE);
	pCmdUI->Enable (exist?TRUE:FALSE);
	if (exist)
	{
		CInstanceInfo *instance = ObjView->getInstance (ObjView->getCameraInstance (pCmdUI->m_nID - ID_SCENE_CAMERA_FIRST));
		nlassert (instance->Camera);
		std::string text = NLMISC::toString("Camera %s", instance->Saved.ShapeFilename.c_str());
		pCmdUI->SetText(nlUtf8ToTStr(text));
	}
	else
	{
		pCmdUI->SetText (_T("No camera"));
	}
}

// ***************************************************************************


void CMainFrame::OnWindowSkeletonScale() 
{
	SkeletonScaleWindow^= true;
	update ();
}

void CMainFrame::OnUpdateWindowSkeletonScale(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (SkeletonScaleWindow);
}

// ***************************************************************************

void CMainFrame::OnWindowTuneMRM() 
{
	TuneMRMWindow^= true;
	update ();
}

void CMainFrame::OnUpdateWindowTuneMRM(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (TuneMRMWindow);
}

void CMainFrame::OnSnapShotTool()
{
	CSnapshotToolDlg snapshotTool(ObjView);
	snapshotTool.DoModal();
}


