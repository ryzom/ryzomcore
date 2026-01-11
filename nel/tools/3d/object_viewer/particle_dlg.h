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


#if !defined(AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_)
#define AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// particle_dlg.h : header file
//


#include "object_viewer.h"
#include "particle_tree_ctrl.h"
#include "nel/misc/matrix.h"
#include "particle_workspace.h"

namespace NL3D
{
	class CParticleSystem;
	class CParticleSystemModel;	
	class CFontManager;
	class CFontGenerator;
};


class CStartStopParticleSystem;
class CSceneDlg;
class CParticleTreeCtrl;
class CMainFrame;
class CAnimationDlg;




/////////////////////////////////////////////////////////////////////////////
// CParticleDlg dialog

class CParticleDlg : public CDialog, public CObjectViewer::IMainLoopCallBack
{
// Construction
public:
	CParticleDlg::CParticleDlg(class CObjectViewer* main, CWnd *pParent, CMainFrame* mainFrame, CAnimationDlg *animDLG);
	~CParticleDlg();
	void setRightPane(CWnd *pane);
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(CParticleDlg)
	enum { IDD = IDD_PARTICLE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation

public:
	// the tree for viewing the system
	CParticleTreeCtrl *ParticleTreeCtrl;
	// inherited from CObjectViewer::IMainLoopCallBack
	void goPostRender();
	void goPreRender() {}
public:
	friend class CParticleTreeCtrl;	
	/** Active a new node of the workspace
	  * Current active node is ready for edition.
	  * Its bbox is displayed.	  
	  */
	void	setActiveNode(CParticleWorkspace::CNode *node);
	// Get the node of the workspace that is currently active
	CParticleWorkspace::CNode *getActiveNode() const { return _ActiveNode; }	
	// Get the particle system model that is currently active
	NL3D::CParticleSystemModel *getActivePSM() const { return _ActiveNode ? _ActiveNode->getPSModel() : NULL; }
	// get matrix of current fx
	const NLMISC::CMatrix &getPSMatrix() const;
	// set matrix of current fx
	void setPSMatrix(const NLMISC::CMatrix &mat);
	// get world matrix of current fx
	const NLMISC::CMatrix &getPSWorldMatrix() const;	
	// set world matrix of current fx
	void setPSWorldMatrix(const NLMISC::CMatrix &mat);	
	// move the current selected element using the given matrix
	void moveElement(const NLMISC::CMatrix &mat);	
	// get the matrix of the current selected element selected, or identity if there's none
	NLMISC::CMatrix getElementMatrix(void) const;	
	// the scene dialog
	CMainFrame *MainFrame;
	// the fonts used for particle edition
	NL3D::CFontManager *FontManager;
	NL3D::CFontGenerator *FontGenerator;
	// get the current right pane of the editor (NULL if none)
	CWnd *getRightPane(void) { return CurrentRightPane; }
	const CWnd *getRightPane(void) const { return CurrentRightPane; }
	// auto bbox for fx
	void  setAutoBBox(bool enable) { _AutoUpdateBBox = enable; }
	bool  getAutoBBox() const { return 	_AutoUpdateBBox; }
	// reset the auto bbox
	void  resetAutoBBox() {	_EmptyBBox = true; }
	// get the object viewer instance
	CObjectViewer *getObjectViewer() const { return _ObjView; }
	/** Stick the current edited fx to a skeleton.	  
	  * This also reset the fx matrix, and prevent from changing it.
	  */
	void stickPSToSkeleton(CParticleWorkspace::CNode *node,
		                   NL3D::CSkeletonModel *skel,
						   uint bone,
						   const std::string &parentSkelName, // -> saved in the workspace
						   const std::string &parentBoneName
						  );
	// unstick the current edited fx from its parent skeleton (if there's one)
	void unstickPSFromSkeleton(CParticleWorkspace::CNode *node);
	// return true is the current edited fx is sticked to a skeleton.
	bool isPSStickedToSkeleton() const { return _ActiveNode != NULL ? _ActiveNode->getParentSkel() != NULL : false; }
	CParticleWorkspace	*getParticleWorkspace() const { return _PW; }	
	// get a model from a ps pointer. The ps must belong to the workspace
	NL3D::CParticleSystemModel *getModelFromPS(NL3D::CParticleSystem *ps) const;
	CStartStopParticleSystem *StartStopDlg;
	// set text of the status bar
	void setStatusBarText(CString &str);
	// Check if the current workspace has been modified, and prompt the user to save it if so
	void checkModifiedWorkSpace();
	// Load a new particle workspace (without asking if current workspace has been modified)
	void loadWorkspace(const std::string &fullPath);	
protected:
	CStatusBar					 _StatusBar;
	CObjectViewer				*_ObjView;
	CParticleWorkspace			*_PW;
	// currently active node of the workspace
	CParticleWorkspace::CNode	*_ActiveNode;

	// the current system that is being edited
	//NL3D::CParticleSystem *_CurrPS;

	// the current model that holds our system
	//NL3D::CParticleSystemModel *_CurrSystemModel;		

	// the system bbox must be updated automatically
	bool						_AutoUpdateBBox;
	
	// the last computed bbox for the system
	bool						_EmptyBBox;
	NLMISC::CAABBox			    _CurrBBox;
	

	// the current right pane of the editor
	CWnd   *CurrentRightPane;
	sint32 CurrRightPaneWidth, CurrRightPaneHeight;

	CRect getTreeRect(int cx, int cy) const;


	// Generated message map functions
	//{{AFX_MSG(CParticleDlg)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCreateNewPsWorkspace();
	afx_msg void OnLoadPSWorkspace();
	afx_msg void OnSaveAllPsWorkspace();
	afx_msg void OnSavePsWorkspace();
	afx_msg void OnViewPsFilename();	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	// Save the workspace structure
	void saveWorkspaceStructure();
	// Save the workspace content
	void saveWorkspaceContent(bool askToSaveModifiedPS);
	// Cloase the workspace without saving or asking the user
	void closeWorkspace();
public:
	/** Save a single particle system
	  * \param askToContinueWhenError If an error occurs, the user will be asked if loading must continue (if there are other ps to save)
	  * \return true if loading should continue for remaining ps, false if loading failed and if user didn't want to continue loading	  
	  */
	bool savePS(HWND parent, CParticleWorkspace::CNode &ps, bool askToContinueWhenError);
	bool savePSAs(HWND parent, CParticleWorkspace::CNode &ps, const std::string &fullPath, bool askToContinueWhenError);
	// Enum to describe the behaviour of loadPS
	enum TLoadPSBehav
	{ 
		Silent,					// loading error are not prompted
		ReportError,				// report error
		ReportErrorSkippable     // report error and ask if the user want errors to be reported again
	};
	/** Try to load a particle system. Prompt msg as necessary for error loading
	  * \param behav Behaviour in case of error
	  * \return true if the user want see further error (e.g he hasn't checked the 'don't show egain' message box.
	  */
	bool loadPS(HWND parent, CParticleWorkspace::CNode &ps, TLoadPSBehav behav);
	//
	uint computeStatusBarWidth() const;
	void updateMenu();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_)
