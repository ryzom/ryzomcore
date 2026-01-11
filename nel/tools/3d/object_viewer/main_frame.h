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

#if !defined(AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// main_frame.h : header file
//

class CSceneDlgMouseListener : public NLMISC::IEventListener
{
public:
	class CObjectViewer	*ObjViewerDlg ;	
	class CMainFrame *SceneDlg ;
	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);
	void releaseFromServer (NLMISC::CEventServer& server);

protected:
	virtual void operator ()(const class NLMISC::CEvent& event) ;

} ;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame frame

typedef void (*winProc)(NL3D::IDriver *drv, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class CMainFrame : public CFrameWnd
{
	//DECLARE_DYNCREATE(CMainFrame)
public:
	CMainFrame( CObjectViewer *objView, winProc );
	virtual ~CMainFrame();

// Attributes
public:

	enum	TMouseMove {MoveCamera=0, MoveSceneRoot, MoveElement, MoveObjectLightTest, MoveFX, MoveFXUserMatrix };
	enum	TCameraMode {FirstMode=0, ObjectMode, CameraMode };

	CStatusBar		StatusBar;
	CToolBar		ToolBar;

	winProc			DriverWindowProc;
	CObjectViewer	*ObjView;

	bool			ShowInfo;
	bool			AnimationWindow;
	bool			AnimationSetWindow;
	bool			MixerSlotsWindow;
	bool			ParticlesWindow;
	bool		    DayNightWindow;
	bool		    WaterPoolWindow;
	bool		    VegetableWindow;
	bool		    GlobalWindWindow;
	bool		    SoundAnimWindow;
	bool		    LightGroupWindow;
	bool			ChooseFrameDelayWindow;
	bool			ChooseBGColorWindow;
	bool			ChooseSunColorWindow;
	bool			SkeletonScaleWindow;
	bool			TuneMRMWindow;
	TMouseMove		MouseMoveType;
	bool			X;
	bool			Y;
	bool			Z;
	uint			MoveMode;
	float			MoveSpeed;
	NLMISC::CRGBA	BgColor;
	bool			Euler;
	float			GlobalWindPower;
	bool			FogActive;
	float			FogStart;
	float			FogEnd;

	void update ();
	void registerValue (bool update=true);

	bool			isMoveCamera() const {return MouseMoveType==MoveCamera;}
	bool			isMoveSceneRoot() const {return MouseMoveType==MoveSceneRoot;}
	bool			isMoveElement() const {return MouseMoveType==MoveElement;}
	bool			isMoveFX() const {return MouseMoveType==MoveFX;}
	bool			isMoveFXUserMatrix() const {return MouseMoveType==MoveFXUserMatrix;}
	bool			isMoveObjectLightTest() const {return MouseMoveType==MoveObjectLightTest;}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
public:
	// Generated message map functions
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnReloadTextures();
	afx_msg void OnClear();
	afx_msg void OnEditMoveelement();
	afx_msg void OnEditMoveFX();
	afx_msg void OnEditMoveFXUserMatrix();
	afx_msg void OnEditX();
	afx_msg void OnEditY();
	afx_msg void OnEditZ();
	afx_msg void OnEnableElementXrotate();
	afx_msg void OnEnableElementYrotate();
	afx_msg void OnEnableElementZrotate();
	afx_msg void OnFileExit();
	afx_msg void OnFileLoadconfig();
	afx_msg void OnFileOpen();	
	afx_msg void OnFileSaveconfig();
	afx_msg void OnViewFirstpersonmode();
	afx_msg void OnViewCamera();
	afx_msg void OnViewObjectmode();
	afx_msg void OnResetCamera();
	afx_msg void OnViewSetbackground();
	afx_msg void OnViewSetmovespeed();
	afx_msg void OnActivateFog();
	afx_msg void OnSetupFog();
	afx_msg void OnWindowAnimation();
	afx_msg void OnWindowAnimationset();
	afx_msg void OnWindowMixersslots();
	afx_msg void OnWindowParticles();
	afx_msg void OnWindowDayNight();
	afx_msg void OnWindowWaterPool();
	afx_msg void OnWindowSoundAnim();
	afx_msg void OnWindowChooseFrameDelay();
	afx_msg void OnWindowChooseBGColor();
	afx_msg void OnWindowChooseSunColor();
	afx_msg void OnSetLightGroupFactor();
	afx_msg void OnShowSceneMatrix();
	afx_msg void OnShowOcclusionTestMeshs();
	afx_msg void OnShowFXMatrix();
	afx_msg void OnShowFXUserMatrix();
	afx_msg void OnUpdateShowSceneMatrix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowFXMatrix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowFXUserMatrix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowOcclusionTestMeshs(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnUpdateWindowAnimation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowAnimationset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowMixersslots(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowParticles(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowDayNight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowWaterPool(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowSoundAnim(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowChooseFrameDelay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowBGColor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowSunColor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectmode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFirstpersonmode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewCamera(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZ(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMoveelement(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMoveFX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMoveFXUserMatrix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowLightGroup(CCmdUI* pCmdUI);
	afx_msg void OnHelpAboutobjectviewer();	
	afx_msg void OnRemoveAllInstancesFromScene();
	afx_msg void OnActivateTextureSet(UINT nID);
	afx_msg void OnShuffleTextureSet();
	afx_msg void OnWindowVegetable();
	afx_msg void OnUpdateWindowVegetable(CCmdUI* pCmdUI);
	afx_msg void OnWindowGlobalwind();
	afx_msg void OnUpdateWindowGlobalwind(CCmdUI* pCmdUI);
	afx_msg void OnEditMoveObjectLightTest();
	afx_msg void OnUpdateEditMoveObjectLightTest(CCmdUI* pCmdUI);
	afx_msg void OnEditMovecamera();
	afx_msg void OnUpdateEditMovecamera(CCmdUI* pCmdUI);
	afx_msg void OnEditMovescene();
	afx_msg void OnUpdateEditMovescene(CCmdUI* pCmdUI);
	afx_msg void OnViewResetSceneRoot();
	afx_msg void OnViewResetFXRoot();
	afx_msg void OnViewResetFXUserMatrix();
	afx_msg void OnViewSetSceneRotation();
	afx_msg void OnShootScene();
	afx_msg void OnWindowSkeletonScale();
	afx_msg void OnUpdateWindowSkeletonScale(CCmdUI* pCmdUI);
	afx_msg void OnWindowTuneMRM();
	afx_msg void OnUpdateWindowTuneMRM(CCmdUI* pCmdUI);
	afx_msg void OnSnapShotTool();
	//}}AFX_MSG
	afx_msg void OnSceneCamera(UINT id);
	afx_msg void OnUpdateSceneCamera(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CSceneDlgMouseListener _RightButtonMouseListener ;

	// The default behaviour of CFrameWnd::PostNcDestroy() is to call 'delete this'. We dont want that behaviour, we want the object viewer to call this
	virtual void PostNcDestroy()
	{
		// do nothing
	}

private:
	float			_LastSceneRotX;
	float			_LastSceneRotY;
	float			_LastSceneRotZ;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_)
