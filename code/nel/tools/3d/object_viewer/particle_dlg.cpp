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


// particle_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "particle_dlg.h"
#include "editable_range.h"
#include "located_properties.h"
#include "particle_system_edit.h"
#include "skippable_message_box.h"
#include "main_frame.h"
//
// TODO : remove these include when the test system will be removed
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/ps_zone.h"
#include "nel/3d/ps_color.h"
#include "nel/3d/ps_float.h"
#include "nel/3d/ps_int.h"
#include "nel/3d/ps_plane_basis_maker.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_grouped.h"
#include "nel/3d/nelu.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
//
#include "nel/misc/file.h"
#include "start_stop_particle_system.h"
//
#include "save_options_dlg.h"
#include "create_file_dlg.h"

using namespace NL3D;

//**************************************************************************************************************************
CParticleDlg::CParticleDlg(class CObjectViewer* main, CWnd *pParent, CMainFrame* mainFrame, CAnimationDlg *animDLG)
	: CDialog(CParticleDlg::IDD, pParent),
	  MainFrame(mainFrame),
	  CurrentRightPane(NULL),
	  _ActiveNode(NULL),
	  _ObjView(main),
	  _EmptyBBox(true),
	  _AutoUpdateBBox(false),
	  _PW(NULL)

{
	//{{AFX_DATA_INIT(CParticleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	nlverify (FontManager = main->getFontManager());
	nlverify (FontGenerator = main->getFontGenerator());
	NL3D::CParticleSystem::setSerializeIdentifierFlag(true); // serialize identifiers for edition	
	ParticleTreeCtrl = new CParticleTreeCtrl(this);
	StartStopDlg = new CStartStopParticleSystem(this, animDLG);
	/** register us, so that our 'go' method will be called
	  * this gives us a chance to display a bbox when needed
	  */
	_ObjView->registerMainLoopCallBack(this);	
}


//**************************************************************************************************************************
BOOL CParticleDlg::Create( UINT nIDTemplate, CWnd* pParentWnd /*= NULL*/ )
{
	if (!CDialog::Create(nIDTemplate, pParentWnd)) return FALSE;	
	return TRUE;
}

//**************************************************************************************************************************
void CParticleDlg::moveElement(const NLMISC::CMatrix &mat)
{
	ParticleTreeCtrl->moveElement(mat);
}

//**************************************************************************************************************************
NLMISC::CMatrix CParticleDlg::getElementMatrix(void) const
{
	return ParticleTreeCtrl->getElementMatrix();
}

//**************************************************************************************************************************
CParticleDlg::~CParticleDlg()
{
	_ObjView->removeMainLoopCallBack(this);	
	delete ParticleTreeCtrl;
	delete CurrentRightPane;
	delete StartStopDlg;
	if (_PW) _PW->setModificationCallback(NULL);
	delete _PW;
}

//**************************************************************************************************************************
void CParticleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticleDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParticleDlg, CDialog)
	//{{AFX_MSG_MAP(CParticleDlg)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_CHAR()
	ON_COMMAND(IDM_CREATE_NEW_PS_WORKSPACE, OnCreateNewPsWorkspace)
	ON_COMMAND(IDM_LOAD_PS_WORKSPACE, OnLoadPSWorkspace)
	ON_COMMAND(IDM_SAVE_ALL_PS_WORKSPACE, OnSaveAllPsWorkspace)
	ON_COMMAND(IDM_SAVE_PS_WORKSPACE, OnSavePsWorkspace)
	ON_COMMAND(IDM_VIEW_PS_FILENAME, OnViewPsFilename)	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//**************************************************************************************************************************
void CParticleDlg::OnDestroy() 
{
	checkModifiedWorkSpace();	
	if (CurrentRightPane)
	{
		CurrentRightPane->DestroyWindow();
		delete CurrentRightPane;
		CurrentRightPane = NULL;
	}
	setRegisterWindowState (this, REGKEY_OBJ_PARTICLE_DLG);
	CDialog::OnDestroy();		
}

//**************************************************************************************************************************
BOOL CParticleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CRect r;
	GetWindowRect(&r);	
	ParticleTreeCtrl->Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
							   | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_EDITLABELS 
							   | TVS_DISABLEDRAGDROP , r, this, 0x1005);	
	ParticleTreeCtrl->init();
	ParticleTreeCtrl->ShowWindow(SW_SHOW);
	StartStopDlg->Create(IDD_PARTICLE_SYSTEM_START_STOP, this);	
	// create menu bar that allow to create / load a particle workspace
	CMenu menu;
	menu.LoadMenu(MAKEINTRESOURCE(IDR_PARTICLE_DLG_MENU));
	this->SetMenu(&menu);
	menu.Detach();		
	updateMenu();
	//
	_StatusBar.Create(this);
	UINT indicators = ID_PS_EDITOR_STATUS;
	_StatusBar.SetIndicators(&indicators, 1);		
	_StatusBar.SetPaneInfo(0, ID_PS_EDITOR_STATUS, SBPS_NORMAL, computeStatusBarWidth());
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_PS_EDITOR_STATUS);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//**************************************************************************************************************************
void CParticleDlg::setStatusBarText(CString &str)
{
	_StatusBar.SetPaneText(0, str, TRUE);
}

//**************************************************************************************************************************
void CParticleDlg::OnSize(UINT nType, int cx, int cy) 
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	bool blocked = false;
	if (ParticleTreeCtrl->m_hWnd && this->m_hWnd)
	{	
		CRect r = getTreeRect(cx, cy);			
		ParticleTreeCtrl->MoveWindow(&r);
		if (CurrentRightPane)
		{								
			CurrentRightPane->MoveWindow(r.right + 10, r.top, r.right + CurrRightPaneWidth + 10, r.top + CurrRightPaneHeight);
		}			
		CDialog::OnSize(nType, cx, cy);	
		if (IsWindow(_StatusBar))
		{					
			_StatusBar.SetPaneInfo(0, ID_PS_EDITOR_STATUS, SBPS_NORMAL, computeStatusBarWidth());
			RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_PS_EDITOR_STATUS);
		}
	}
}

//**************************************************************************************************************************
CRect CParticleDlg::getTreeRect(int cx, int cy) const
{
	const uint ox = 0, oy = 10;

	if (CurrentRightPane)
	{		
		CRect res(ox, oy, cx - CurrRightPaneWidth - 10, cy - 20); 
		return res;
	}
	else
	{
		CRect res(ox, oy, cx - 10, cy - 20);
		return res;
	}
}

//**************************************************************************************************************************
void CParticleDlg::setRightPane(CWnd *pane)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (CurrentRightPane)
	{
		CurrentRightPane->DestroyWindow();
	}
	delete CurrentRightPane;
	CurrentRightPane = pane;
	RECT r;	
	if (pane)
	{
		
		pane->ShowWindow(SW_SHOW);
	
	
		CurrentRightPane->GetClientRect(&r);

		CurrRightPaneWidth = r.right;
		CurrRightPaneHeight = r.bottom;
	
	}

	GetClientRect(&r);
	this->SendMessage(WM_SIZE, SIZE_RESTORED, r.right + (r.bottom << 16));	
	GetWindowRect(&r);
	this->MoveWindow(&r);
	if (CurrentRightPane)
	{
		CurrentRightPane->Invalidate();
	}
	this->Invalidate();
	ParticleTreeCtrl->Invalidate();
}

//**************************************************************************************************************************
LRESULT CParticleDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	if (message == WM_GETMINMAXINFO)
	{
		sint cx = 150, cy = 150;
		if (CurrentRightPane)
		{
			RECT r;
			CurrentRightPane->GetClientRect(&r);
			cx += CurrRightPaneWidth;
			if (cy < (CurrRightPaneHeight + 20) ) cy = CurrRightPaneHeight + 20;
		}


		MINMAXINFO *inf = 	(MINMAXINFO *) lParam;
		inf->ptMinTrackSize.x = cx;
		inf->ptMinTrackSize.y = cy;

	}
	
	return CDialog::WindowProc(message, wParam, lParam);
	
}

//**************************************************************************************************************************
void CParticleDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	StartStopDlg->ShowWindow(bShow);	
		
}

//**************************************************************************************************************************
void CParticleDlg::goPostRender()
{
	NL3D::CParticleSystem *currPS = _ActiveNode ? _ActiveNode->getPSPointer() : NULL;
	if (!currPS) return;
	if (StartStopDlg->isBBoxDisplayEnabled() && currPS)
	{
		NL3D::CNELU::Driver->setupModelMatrix(currPS->getSysMat());		
		if (_AutoUpdateBBox)
		{
			NLMISC::CAABBox currBBox;
			currPS->forceComputeBBox(currBBox);
			if (_EmptyBBox)
			{
				_EmptyBBox = false;
				_CurrBBox = currBBox;
			}
			else
			{			
				NL3D::CPSUtil::displayBBox(NL3D::CNELU::Driver, _CurrBBox, CRGBA::Blue);			
				_CurrBBox = NLMISC::CAABBox::computeAABBoxUnion(currBBox, _CurrBBox);
			}
			currPS->setPrecomputedBBox(_CurrBBox);
		}	
		else
		{
			currPS->getLastComputedBBox(_CurrBBox);
		}
		NL3D::CPSUtil::displayBBox(NL3D::CNELU::Driver, _CurrBBox, currPS->getAutoComputeBBox() ? CRGBA::White : CRGBA::Red);
	}
	// copy user matrix into current fx
	nlassert(_ObjView);
	_ActiveNode->getPSModel()->setUserMatrix(_ObjView->getFXUserMatrix());
}

//**************************************************************************************************************************
void CParticleDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == (UINT) 'p' || nChar == (UINT) 'P' || nChar == (UINT) ' ')
	{
		// simulate a start / stop on the system
		StartStopDlg->toggle();
	}
	
	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

//**************************************************************************************************************************
const NLMISC::CMatrix &CParticleDlg::getPSMatrix() const
{
	if (!getActivePSM()) return NLMISC::CMatrix::Identity;		
	return getActivePSM()->getMatrix();
}

//**************************************************************************************************************************
const NLMISC::CMatrix &CParticleDlg::getPSWorldMatrix() const
{
	if (!getActivePSM()) return NLMISC::CMatrix::Identity;	
	return getActivePSM()->getWorldMatrix();
}

//**************************************************************************************************************************
void CParticleDlg::setPSMatrix(const NLMISC::CMatrix &mat)
{
	if (!getActivePSM()) return;		
	getActivePSM()->setMatrix(mat);
}

//**************************************************************************************************************************
void CParticleDlg::setPSWorldMatrix(const NLMISC::CMatrix &mat)
{
	if (!getActivePSM()) return;	
	CMatrix invParentMat =  getActivePSM()->getMatrix() * getActivePSM()->getWorldMatrix().inverted();
	invParentMat.normalize(CMatrix::XYZ);
	CMatrix newMat = invParentMat * mat;
	newMat.normalize(CMatrix::XYZ);
	getActivePSM()->setMatrix(newMat);
}


//**************************************************************************************************************************
void CParticleDlg::stickPSToSkeleton(CParticleWorkspace::CNode *node,
									 NL3D::CSkeletonModel *skel,
									 uint bone,
									 const std::string &parentSkelName,
									 const std::string &parentBoneName)
{
	if (!node) return;	
	node->stickPSToSkeleton(skel, bone, parentSkelName, parentBoneName);
	if (skel)
	{
		if (_ObjView->getMainFrame()->MouseMoveType == CMainFrame::MoveFX)
		{
			_ObjView->getMainFrame()->OnEditMovecamera();
		}		
		_ObjView->getMainFrame()->ToolBar.Invalidate();		
	}
}

//**************************************************************************************************************************
void CParticleDlg::unstickPSFromSkeleton(CParticleWorkspace::CNode *node)
{
	if (!node) return;
	node->unstickPSFromSkeleton();	
	_ObjView->getMainFrame()->ToolBar.Invalidate();		
}

//**************************************************************************************************************************
bool CParticleDlg::savePS(HWND parent, CParticleWorkspace::CNode &psNode, bool askToContinueWhenError)
{
	return savePSAs(parent, psNode, psNode.getFullPath(), askToContinueWhenError);
}

//**************************************************************************************************************************
bool CParticleDlg::savePSAs(HWND parent, CParticleWorkspace::CNode &psNode ,const std::string &fullPath, bool askToContinueWhenError)
{
	nlassert(psNode.getPSPointer());
	if (psNode.getResetAutoCountFlag() && psNode.getPSPointer()->getAutoCountFlag())
	{		
		MessageBox(psNode.getFilename().c_str() + getStrRsc(IDS_AUTO_COUNT_ERROR), getStrRsc(IDS_WARNING), MB_ICONEXCLAMATION);
		return false;
	}
	StartStopDlg->stop();
	try
	{	
		psNode.savePSAs(fullPath);
		psNode.setModified(false);
		setStatusBarText(CString(fullPath.c_str()) + " " + getStrRsc(IDS_SAVED));
	}
	catch (NLMISC::Exception &e)
	{
		if (askToContinueWhenError)
		{		
			int result = ::MessageBox(parent, CString(e.what()) + getStrRsc(IDS_CONTINUE_SAVING) , getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
			return result == MB_OK;
		}
		else
		{
			::MessageBox(parent, e.what(), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
			return false;
		}
	}
	return true;
}

//**************************************************************************************************************************
bool CParticleDlg::loadPS(HWND parent, CParticleWorkspace::CNode &psNode, TLoadPSBehav behav)
{
	bool loadingOK = true;	
	try
	{	
		if (!psNode.loadPS())
		{
			loadingOK = false;
			switch(behav)
			{
				case Silent: return false; // no op
				case ReportError:					
					::MessageBox(parent, (LPCTSTR) (CString(psNode.getFilename().c_str()) + " : " + getStrRsc(IDS_COULDNT_INSTANCIATE_PS)), getStrRsc(IDS_ERROR), MB_OK|MB_ICONEXCLAMATION);
					return true;
				break;
				case ReportErrorSkippable:
				{
					CSkippableMessageBox mb(getStrRsc(IDS_ERROR), CString(psNode.getFilename().c_str()) + " : " + getStrRsc(IDS_COULDNT_INSTANCIATE_PS), CWnd::FromHandle(parent));
					mb.DoModal();
					return !mb.getBypassFlag();
				}
				break;
				default:
					nlassert(0);
				break;
			}			
		}
		else
		{
			setStatusBarText(CString(psNode.getFullPath().c_str()) + " " + getStrRsc(IDS_LOADED));
		}
	}
	catch (NLMISC::Exception &e)
	{
		switch(behav)
		{
			case Silent: return false; // no op
			case ReportError:	
				::MessageBox(parent, e.what(), getStrRsc(IDS_ERROR), MB_OK|MB_ICONEXCLAMATION);				
				return true;
			break;
			case ReportErrorSkippable:
			{
				CSkippableMessageBox mb(getStrRsc(IDS_ERROR), CString(e.what()), CWnd::FromHandle(parent));
				mb.DoModal();
				return !mb.getBypassFlag();
			}
			break;
			default:
				nlassert(0);
			break;
		}		
	}
	if (psNode.getPSPointer()->hasLoop())
	{		
		localizedMessageBox(parent, IDS_FX_HAS_LOOP, IDS_WARNING, MB_OK|MB_ICONEXCLAMATION);
	}	
	return behav != Silent;
}


//**************************************************************************************************************************
void CParticleDlg::checkModifiedWorkSpace()
{
	if (_PW)
	{
		// see if current tree has been changed				
		if (_PW->isModified())
		{			
			int result = localizedMessageBox(*this, IDS_PS_WORKSPACE_MODIFIED, IDS_PARTICLE_EDITOR, MB_YESNO|MB_ICONQUESTION);
			if (result == IDYES)
			{
				saveWorkspaceStructure();
			}
		}
		if (_PW->isContentModified())
		{			
			saveWorkspaceContent(true);			
		}
	}
}

//**************************************************************************************************************************
void CParticleDlg::closeWorkspace()
{
	setActiveNode(NULL);
	ParticleTreeCtrl->setActiveNode(NULL);
	ParticleTreeCtrl->reset();
	delete _PW;
	_PW = NULL;
}

//**************************************************************************************************************************
void CParticleDlg::OnCreateNewPsWorkspace() 
{		
	checkModifiedWorkSpace();
	// ask name of the new workspace to create
	CCreateFileDlg cf(getStrRsc(IDS_CHOOSE_WORKSPACE_NAME), "", "pws");
	INT_PTR result = cf.DoModal();
	if (result = IDOK)
	{			
		if (cf.touchFile())
		{				
			CParticleWorkspace *newPW = new CParticleWorkspace;
			newPW->setModificationCallback(ParticleTreeCtrl);
			newPW->init(_ObjView, cf.getFullPath(), _ObjView->getFontManager(), _ObjView->getFontGenerator());
			// save empty workspace
			try
			{		
				newPW->save();
			}
			catch(NLMISC::EStream &e)
			{
				MessageBox(e.what(), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
			}
			closeWorkspace();			
			_PW = newPW;
			ParticleTreeCtrl->buildTreeFromWorkSpace(*_PW);
		}
	}

}

//**************************************************************************************************************************
void CParticleDlg::OnLoadPSWorkspace() 
{
	checkModifiedWorkSpace();
	static const char BASED_CODE szFilter[] = "particle workspaces(*.pws)|*.pws||";
	CFileDialog fd( TRUE, ".pws", "*.pws", 0, szFilter);
	INT_PTR result = fd.DoModal();
	if (result != IDOK) return;
	loadWorkspace((LPCTSTR) fd.GetPathName());
}

//**************************************************************************************************************************
void CParticleDlg::loadWorkspace(const std::string &fullPath)
{
	// Add to the path
	std::auto_ptr<CParticleWorkspace> newPW(new CParticleWorkspace);
	newPW->init(_ObjView, fullPath, _ObjView->getFontManager(), _ObjView->getFontGenerator());
	newPW->setModificationCallback(ParticleTreeCtrl);
	// save empty workspace
	try
	{		
		newPW->load();
		setStatusBarText(CString(newPW->getFilename().c_str()) + " " + getStrRsc(IDS_LOADED));
	}
	catch(NLMISC::EStream &e)
	{
		MessageBox(e.what(), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
		setStatusBarText(CString(e.what()));
		return;
	}	
	// try to load each ps
	CParticleWorkspace::CNode *firstLoadedNode = NULL;
	bool displayErrorMsg = true;
	for(uint k = 0; k < newPW->getNumNode(); ++k)
	{
		
		displayErrorMsg = loadPS(*this, *newPW->getNode(k), displayErrorMsg ? ReportErrorSkippable : Silent);		
		if (newPW->getNode(k)->isLoaded() && !firstLoadedNode)
		{
			firstLoadedNode = newPW->getNode(k);
		}			
	}	
	closeWorkspace();
	_PW = newPW.release();
	ParticleTreeCtrl->buildTreeFromWorkSpace(*_PW);	
	setActiveNode(firstLoadedNode);
	ParticleTreeCtrl->setActiveNode(firstLoadedNode);
	ParticleTreeCtrl->expandRoot();
	setStatusBarText(getStrRsc(IDS_READY));
}

//**************************************************************************************************************************
void CParticleDlg::OnSaveAllPsWorkspace() 
{
	saveWorkspaceStructure();
	saveWorkspaceContent(false);
}

//**************************************************************************************************************************
void CParticleDlg::OnSavePsWorkspace() 
{
	saveWorkspaceStructure();
	saveWorkspaceContent(true);
}

//**************************************************************************************************************************
void CParticleDlg::saveWorkspaceStructure()
{
	nlassert(_PW);
	try
	{
		_PW->save();		
		setStatusBarText(CString(_PW->getFilename().c_str()) + " " + getStrRsc(IDS_SAVED));
	}
	catch(NLMISC::EStream &e)
	{
		localizedMessageBox(*this, e.what(), IDS_ERROR, MB_ICONEXCLAMATION);
		setStatusBarText(CString(e.what()));
	}
}


//**************************************************************************************************************************
void CParticleDlg::saveWorkspaceContent(bool askToSaveModifiedPS)
{
	StartStopDlg->stop();
	bool saveAll = !askToSaveModifiedPS;
	// check each component of the tree
	for(uint k = 0; k < _PW->getNumNode(); ++k)
	{
		if (_PW->getNode(k)->isModified())
		{				
			if (saveAll)
			{
				bool keepSaving = savePS(*this, *_PW->getNode(k), k != _PW->getNumNode());
				if (!keepSaving) break;
			}
			else
			{
				// ask if the user wants to save the ps, or save all ps
				CString mess;					
				mess = CString(_PW->getNode(k)->getFilename().c_str()) + getStrRsc(IDS_SAVE_MODIFIED_PS);
				CSaveOptionsDlg sop(getStrRsc(IDS_SAVE_FILE), mess, this);
				sop.DoModal();
				bool saveThisFile = false;
				bool stop = false;
				switch(sop.getChoice())
				{
					case CSaveOptionsDlg::Yes:		saveThisFile = true;  break;
					case CSaveOptionsDlg::No:		saveThisFile = false; break;
					case CSaveOptionsDlg::SaveAll:	saveAll =	   true;  break;
					case CSaveOptionsDlg::Stop:		stop =		   true;  break;
					default: nlassert(0);
				}
				if (stop) break;
				if (saveAll || saveThisFile)
				{
					bool keepSaving = savePS(*this, *_PW->getNode(k), k != _PW->getNumNode());
					if (!keepSaving) break;
				}
			}				
		}
	}
	setStatusBarText(getStrRsc(IDS_READY));
}

//**************************************************************************************************************************
void CParticleDlg::setActiveNode(CParticleWorkspace::CNode *node)
{
	if (node == _ActiveNode) return;	
	_ActiveNode = node;
	StartStopDlg->setActiveNode(node);		
	if(MainFrame->isMoveFX())
	{
		if (node)
		{	
			_ObjView->getMouseListener().setModelMatrix(node->getPSModel()->getMatrix());	
		}
		else
		{
			_ObjView->getMouseListener().setModelMatrix(NLMISC::CMatrix::Identity);
		}
	}
	else
	if(MainFrame->isMoveFXUserMatrix())
	{
		if (node)
		{
			_ObjView->getMouseListener().setModelMatrix(node->getPSModel()->getUserMatrix());
		}
		else
		{
			_ObjView->getMouseListener().setModelMatrix(NLMISC::CMatrix::Identity);
		}
	}
}

//**************************************************************************************************************************
NL3D::CParticleSystemModel *CParticleDlg::getModelFromPS(NL3D::CParticleSystem *ps) const
{
	if (!ps) return	NULL;
	if (!_PW) return NULL;
	CParticleWorkspace::CNode *node = _PW->getNodeFromPS(ps);
	if (!node) return NULL;
	return node->getPSModel();
}

//**************************************************************************************************************************
uint CParticleDlg::computeStatusBarWidth() const
{
	nlassert(ParticleTreeCtrl);
	CRect tcRect;
	ParticleTreeCtrl->GetClientRect(&tcRect);
	if (!CurrentRightPane) return (uint) std::max((sint) tcRect.Width() - 16, (sint) 0);
	return (uint) std::max((sint) tcRect.Width() - 4, (sint) 0);
}

//**************************************************************************************************************************
void CParticleDlg::OnViewPsFilename() 
{
	ParticleTreeCtrl->setViewFilenameFlag(!ParticleTreeCtrl->getViewFilenameFlag());
	updateMenu();
}

//**************************************************************************************************************************
void CParticleDlg::updateMenu()
{
	if (!ParticleTreeCtrl) return;
	CMenu *menu = GetMenu();
	if (!menu) return;
	// update the view menu
	menu->CheckMenuItem(IDM_VIEW_PS_FILENAME, MF_BYCOMMAND|(ParticleTreeCtrl->getViewFilenameFlag() ? MF_CHECKED : 0));
}

 