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
#include "object_viewer.h"
#include "start_stop_particle_system.h"
#include "located_properties.h"
#include "select_string.h"
//
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_sound.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/particle_system_model.h"
#include "particle_dlg.h"


#include  <algorithm>

/*static char trace_buf[200];
#define NL_TRACE sprintf(trace_buf, "%d", __LINE__); \
				::MessageBox(NULL, trace_buf, NULL, MB_OK);
*/

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog

//******************************************************************************************************
CStartStopParticleSystem::CStartStopParticleSystem(CParticleDlg *particleDlg,
												   CAnimationDlg *animationDLG)
	: CDialog(CStartStopParticleSystem::IDD, particleDlg),
	  _ParticleDlg(particleDlg),
	  _State(Stopped),	  
	  _LastCurrNumParticles(-1),
	  _LastMaxNumParticles(-1),
	  _LastNumWantedFaces(-1),
	  _LastSystemDate(-1.f),
	  _AutoRepeat(false),
	  _AnimationDLG(animationDLG),
	  _LastSceneAnimFrame(0.f),
	  _ActiveNode(NULL)
{
	nlassert(particleDlg && particleDlg->getObjectViewer());
	particleDlg->getObjectViewer()->registerMainLoopCallBack(this);
	//{{AFX_DATA_INIT(CStartStopParticleSystem)
	m_DisplayBBox = TRUE;
	m_SpeedSliderPos = 100;	
	m_DisplayHelpers = FALSE;
	m_LinkPlayToScenePlay = FALSE;
	m_TriggerAnim = _T("");
	//}}AFX_DATA_INIT
}

//******************************************************************************************************
CStartStopParticleSystem::~CStartStopParticleSystem()
{
	nlassert(_ParticleDlg && _ParticleDlg->getObjectViewer());
	_ParticleDlg->getObjectViewer()->removeMainLoopCallBack(this);
}

//******************************************************************************************************
bool CStartStopParticleSystem::isBBoxDisplayEnabled()
{
	UpdateData();
	return m_DisplayBBox ? true : false;
}
	
//******************************************************************************************************
void CStartStopParticleSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartStopParticleSystem)
	DDX_Control(pDX, IDC_START_MULTIPLE_PICTURE, m_StartMultiplePicture);
	DDX_Control(pDX, IDC_PAUSE_PICTURE, m_PausePicture);
	DDX_Control(pDX, IDC_STOP_PICTURE, m_StopPicture);
	DDX_Control(pDX, IDC_START_PICTURE, m_StartPicture);
	DDX_Check(pDX, IDC_DISPLAY_BBOX, m_DisplayBBox);	
	DDX_Slider(pDX, IDC_ANIM_SPEED, m_SpeedSliderPos);
	DDX_Check(pDX, IDC_DISPLAY_HELPERS, m_DisplayHelpers);
	DDX_Check(pDX, IDC_LINK_PLAY_TO_SCENE_PLAY, m_LinkPlayToScenePlay);
	DDX_Text(pDX, IDC_TRIGGER_ANIM, m_TriggerAnim);
	//}}AFX_DATA_MAP


}


BEGIN_MESSAGE_MAP(CStartStopParticleSystem, CDialog)
	//{{AFX_MSG_MAP(CStartStopParticleSystem)
	ON_BN_CLICKED(IDC_START_PICTURE, OnStartSystem)
	ON_BN_CLICKED(IDC_STOP_PICTURE, OnStopSystem)
	ON_BN_CLICKED(IDC_PAUSE_PICTURE, OnPause)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_ANIM_SPEED, OnReleasedcaptureAnimSpeed)
	ON_BN_CLICKED(IDC_DISPLAY_HELPERS, OnDisplayHelpers)
	ON_BN_CLICKED(IDC_ENABLE_AUTO_COUNT, OnEnableAutoCount)
	ON_BN_CLICKED(IDC_RESET_COUNT, OnResetCount)
	ON_BN_CLICKED(IDC_AUTOREPEAT, OnAutoRepeat)
	ON_BN_CLICKED(IDC_LINK_PLAY_TO_SCENE_PLAY, OnLinkPlayToScenePlay)
	ON_BN_CLICKED(IDC_LINK_TO_SKELETON, OnLinkToSkeleton)
	ON_BN_CLICKED(IDC_UNLINK_FROM_SKELETON, OnUnlinkFromSkeleton)
	ON_BN_CLICKED(IDC_START_MULTIPLE_PICTURE, OnStartMultipleSystem)
	ON_BN_CLICKED(IDC_BROWSE_ANIM, OnBrowseAnim)
	ON_BN_CLICKED(IDC_CLEAR_ANIM, OnClearAnim)
	ON_BN_CLICKED(IDC_RESTICK_ALL, OnRestickAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem message handlers

//******************************************************************************************************
BOOL CStartStopParticleSystem::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	HBITMAP bm[4];		
	bm[0] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_START_SYSTEM));
	bm[1] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_STOP_SYSTEM));
	bm[2] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PAUSE_SYSTEM));	
	bm[3] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_START_MULTIPLE_SYSTEM));
	m_StartPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[0]);
	m_StopPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[1]);
	m_PausePicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[2]);
	m_StartMultiplePicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[3]);
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_ANIM_SPEED);
	sl->SetRange(0, 100);
	setSpeedSliderValue(1.f);
	forceActiveNode(NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//******************************************************************************************************
void CStartStopParticleSystem::OnStartSystem() 
{
	start();	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnStartMultipleSystem() 
{
	startMultiple();	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnStopSystem() 
{			
	stop();	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnPause() 
{
	pause();
}


//******************************************************************************************************
void CStartStopParticleSystem::updateUIFromState()
{
	// update actions buttons
	switch(_State)
	{
		case Stopped:
			m_StartPicture.EnableWindow(_ActiveNode != NULL);
			m_PausePicture.EnableWindow(FALSE);
			m_StopPicture.EnableWindow(FALSE);
			m_StartMultiplePicture.EnableWindow(TRUE);
		break;
		case RunningSingle:
			m_StartPicture.EnableWindow(FALSE);
			m_PausePicture.EnableWindow(TRUE);
			m_StopPicture.EnableWindow(TRUE);
			m_StartMultiplePicture.EnableWindow(FALSE);
		break;
		case RunningMultiple:
			m_StartPicture.EnableWindow(FALSE);
			m_PausePicture.EnableWindow(TRUE);
			m_StopPicture.EnableWindow(TRUE);
			m_StartMultiplePicture.EnableWindow(FALSE);
		break;
		case PausedSingle:
			m_StartPicture.EnableWindow(TRUE);
			m_PausePicture.EnableWindow(FALSE);
			m_StopPicture.EnableWindow(TRUE);
			m_StartMultiplePicture.EnableWindow(FALSE);
		break;
		case PausedMultiple:
			m_StartPicture.EnableWindow(FALSE);
			m_PausePicture.EnableWindow(FALSE);
			m_StopPicture.EnableWindow(TRUE);
			m_StartMultiplePicture.EnableWindow(TRUE);
		break;	
		default:
			nlassert(0);
		break;
	}
	if (!getCurrPS())
	{	
		GetDlgItem(IDC_ACTIVE_PS)->SetWindowText(getStrRsc(IDS_NO_ACTIVE_PS));
		GetDlgItem(IDC_STICK_BONE)->SetWindowText(_T(""));
		GetDlgItem(IDC_ENABLE_AUTO_COUNT)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESET_COUNT)->EnableWindow(FALSE);
		((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(0);
		// hide display of particle numbers
		GetDlgItem(IDC_NUM_PARTICLES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NUM_ASKED_FACES)->ShowWindow(SW_HIDE);	
		GetDlgItem(IDC_SYSTEM_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISPLAY_BBOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_LINK_TO_SKELETON)->EnableWindow(FALSE);
		GetDlgItem(IDC_UNLINK_FROM_SKELETON)->EnableWindow(FALSE);
		GetDlgItem(IDC_ACTIVE_PS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BROWSE_ANIM)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISPLAY_HELPERS)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLEAR_ANIM)->EnableWindow(FALSE);
		m_TriggerAnim.Empty();
	}
	else
	{	
		if (!_ActiveNode->getParentSkelName().empty())
		{
			GetDlgItem(IDC_STICK_BONE)->SetWindowText(nlUtf8ToTStr(_ActiveNode->getParentBoneName() + "." + _ActiveNode->getParentBoneName()));
		}
		else
		{
			GetDlgItem(IDC_STICK_BONE)->SetWindowText(_T(""));
		}		
		GetDlgItem(IDC_ACTIVE_PS)->SetWindowText(nlUtf8ToTStr(_ActiveNode->getFilename()));
		GetDlgItem(IDC_ENABLE_AUTO_COUNT)->EnableWindow(TRUE);
		((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(getCurrPS()->getAutoCountFlag() ? 1 : 0);
		GetDlgItem(IDC_RESET_COUNT)->EnableWindow((_ActiveNode->getPSPointer()->getAutoCountFlag() && !_ActiveNode->getResetAutoCountFlag()) ? TRUE : FALSE);
		GetDlgItem(IDC_NUM_PARTICLES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NUM_ASKED_FACES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SYSTEM_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DISPLAY_BBOX)->EnableWindow(TRUE);
		GetDlgItem(IDC_LINK_TO_SKELETON)->EnableWindow(TRUE);
		GetDlgItem(IDC_UNLINK_FROM_SKELETON)->EnableWindow(TRUE);
		GetDlgItem(IDC_ACTIVE_PS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BROWSE_ANIM)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISPLAY_HELPERS)->EnableWindow(isRunning());
		GetDlgItem(IDC_CLEAR_ANIM)->EnableWindow(!_ActiveNode->getTriggerAnim().empty());
		m_TriggerAnim = _ActiveNode->getTriggerAnim().c_str();		
	}
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::start()
{
	UpdateData();
	switch(_State)
	{
		case Stopped:
			if (_ActiveNode)
			{	
				if (checkHasLoop(*_ActiveNode)) return;
				play(*_ActiveNode);	
				nlassert(_PlayingNodes.empty());
				_PlayingNodes.push_back(_ActiveNode);
			}
			GetDlgItem(IDC_RESET_COUNT)->EnableWindow(TRUE);
		break;
		case RunningSingle:
			// no-op
		return;
		break;
		case RunningMultiple:
			stop();
			start();
		break;
		case PausedSingle:
			if (_ActiveNode)
			{
				unpause(*_ActiveNode);
			}
		break;
		case PausedMultiple:
			for(uint k = 0; k < _PlayingNodes.size(); ++k)
			{
				if (_PlayingNodes[k])
				{
					unpause(*_PlayingNodes[k]);
				}
			}
			stop();
			start();
		break;
		default:
			nlassert(0);
		break;
	}
	_State = RunningSingle;
	updateUIFromState();	
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = true;
		}
	}	
}

//******************************************************************************************************
void CStartStopParticleSystem::startMultiple()
{
	UpdateData();
	switch(_State)
	{
		case Stopped:
		{			
			CParticleWorkspace *pws = _ParticleDlg->getParticleWorkspace();
			if (!pws) return;
			nlassert(_PlayingNodes.empty());
			for(uint k = 0; k < pws->getNumNode(); ++k)
			{
				if (pws->getNode(k)->isLoaded())
				{				
					if (checkHasLoop(*pws->getNode(k))) return;
				}
			}
			for(uint k = 0; k < pws->getNumNode(); ++k)
			{
				if (pws->getNode(k)->isLoaded())
				{
					// really start the node only if there's no trigger anim
					if (pws->getNode(k)->getTriggerAnim().empty())
					{					
						play(*pws->getNode(k));
					}
					_PlayingNodes.push_back(pws->getNode(k));
				}
			}
			GetDlgItem(IDC_RESET_COUNT)->EnableWindow(TRUE);
		}
		break;
		case PausedSingle:
		case RunningSingle:
			stop();
			startMultiple();
		break;
		case RunningMultiple:
			// no-op
			return;
		break;			
		case PausedMultiple:
			for(uint k = 0; k < _PlayingNodes.size(); ++k)
			{
				if (_PlayingNodes[k])
				{
					unpause(*_PlayingNodes[k]);
				}
			}			
		break;
		default:
			nlassert(0);
		break;
	}
	_State = RunningMultiple;
	updateUIFromState();	
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = true;
		}
	}	
}

//******************************************************************************************************
void CStartStopParticleSystem::stop()
{
	switch(_State)
	{
		case Stopped:
			// no-op
			return;
		case RunningSingle:
		case RunningMultiple:			
		case PausedSingle:		
		case PausedMultiple:
			for(uint k = 0; k < _PlayingNodes.size(); ++k)
			{
				if (_PlayingNodes[k])
				{				
					stop(*_PlayingNodes[k]);
				}
			}
			_PlayingNodes.clear();
		break;		
		default:
			nlassert(0);
		break;
	}
	_State = Stopped;	
	if (IsWindow(*this)) updateUIFromState();
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = false;
		}
	}	
}

//******************************************************************************************************
void CStartStopParticleSystem::pause()
{
	switch(_State)
	{
		case Stopped:
			// no-op
			return;
		case RunningSingle:
			if (_ActiveNode)
			{			
				pause(*_ActiveNode);				
			}
			_State = PausedSingle;
			updateUIFromState();
		break;
		case RunningMultiple:					
			for(uint k = 0; k < _PlayingNodes.size(); ++k)
			{
				if (_PlayingNodes[k])
				{				
					pause(*_PlayingNodes[k]);
				}				
			}
			_State = PausedMultiple;
			updateUIFromState();
		break;
		case PausedSingle:		
		case PausedMultiple:
			// no-op
			return;
		default:
			nlassert(0);
		break;
	}
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = false;
		}
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::toggle()
{
	switch(_State)
	{
		case Stopped:		
		break;
		case RunningSingle:
			pause();
		break;
		case RunningMultiple:
			pause();			
		break;
		case PausedSingle:
			start();
		break;
		case PausedMultiple:
			startMultiple();
		break;	
		default:
			nlassert(0);
		break;
	}
}


///////////////////////////////////

//******************************************************************************************************
void CStartStopParticleSystem::setSpeedSliderValue(float value)
{
	m_SpeedSliderPos = (int) (value * 100);
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnReleasedcaptureAnimSpeed(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_ANIM_SPEED);	
	setEllapsedTimeRatio(m_SpeedSliderPos * 0.01f);
	*pResult = 0;		
}


//******************************************************************************************************
void CStartStopParticleSystem::OnDisplayHelpers() 
{
	UpdateData();
	if (!_ActiveNode) return;	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnEnableAutoCount() 
{		
	nlassert(_ActiveNode);
	stop();
	bool autoCount = ((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->GetCheck() != 0;
	if (autoCount)
	{	
		CString caption;
		CString mess;
		caption.LoadString(IDS_PARTICLE_SYSTEM_EDITOR);
		mess.LoadString(IDS_ENABLE_AUTOCOUNT);
		if (MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_OKCANCEL) != IDOK)
		{
			((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(0);
			return;
		}
		resetAutoCount(_ActiveNode);
	}
	enableAutoCount(autoCount);		
}

//******************************************************************************************************
void CStartStopParticleSystem::resetAutoCount(CParticleWorkspace::CNode *node, bool reset /* = true */)
{
	if (!node) return;	
	if (node->getResetAutoCountFlag() == reset) return;
	node->setResetAutoCountFlag(reset);
	if (node == _ActiveNode)	
	{	
		GetDlgItem(IDC_RESET_COUNT)->EnableWindow((_ActiveNode->getPSPointer()->getAutoCountFlag() && !reset) ? TRUE : FALSE);
	}
	node->setModified(true);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnResetCount() 
{
	stop();
	resetAutoCount(_ActiveNode);	
}

//******************************************************************************************************
void CStartStopParticleSystem::enableAutoCount(bool enable)

{	
	if (!_ActiveNode) return;
	if (enable == _ActiveNode->getPSPointer()->getAutoCountFlag()) return;
	nlassert(getCurrPS());
	((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(enable ? 1 : 0);
	_ActiveNode->getPSPointer()->setAutoCountFlag(enable);
	_ActiveNode->setModified(true);
	GetDlgItem(IDC_RESET_COUNT)->EnableWindow(FALSE);
	CLocatedProperties *lp =dynamic_cast<CLocatedProperties *>(_ParticleDlg->getRightPane());
	if (lp)
	{
		lp->getParticleCountDlg()->EnableWindow(enable ? FALSE : TRUE);
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::restartAllFX()
{
	if (_State == RunningMultiple || _State == PausedMultiple)
	{
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{					
				stop(*_PlayingNodes[k]);
			}
		}
	}
	else
	{	
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{					
				stop(*_PlayingNodes[k]);
				play(*_PlayingNodes[k]);
			}
		}
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::goPreRender()
{	
	if (!_ActiveNode) return;
	NL3D::CParticleSystem *ps = _ActiveNode->getPSPointer();
	sint currNumParticles = (sint) ps->getCurrNumParticles();
	sint maxNumParticles = (sint) ps->getMaxNumParticles();
	// if linked with scene animation, restart if animation ends
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{		
		// start animation for the scene too
		if (_AnimationDLG && isRunning())
		{
			if (_LastSceneAnimFrame > _AnimationDLG->CurrentFrame) // did animation restart ?
			{			
				restartAllFX();
			}
			_LastSceneAnimFrame = _AnimationDLG->CurrentFrame;
		}
	}
	else
	if (_AutoRepeat && !m_LinkPlayToScenePlay) // auto repeat feature
	{
		if (isRunning())
		{		
			bool allFXFinished = true;
			bool fxStarted = false;
			for(uint k = 0; k < _PlayingNodes.size(); ++k)
			{
				if (_PlayingNodes[k])
				{					
					if (isRunning(_PlayingNodes[k]))
					{					
						fxStarted = true;
						if (_PlayingNodes[k]->getPSPointer()->getSystemDate() <= _PlayingNodes[k]->getPSPointer()->evalDuration())
						{												
							allFXFinished = false;
							break;						
						}
						else
						{
							if (_PlayingNodes[k]->getPSPointer()->getCurrNumParticles() != 0)
							{						
								allFXFinished = false;
								break;
							}
						}
					}
				}
			}
			if (fxStarted && allFXFinished)
			{
				restartAllFX();
			}
		}
	}
	if (_State == RunningMultiple || _State == PausedMultiple)
	{
		std::set<std::string> currAnims;
		CObjectViewer *ov = _ParticleDlg->getObjectViewer();
		for(uint k = 0; k < ov->getNumInstance(); ++k)
		{
			CInstanceInfo *ci = ov->getInstance(k);
			uint animIndex = ci->Playlist.getAnimation(0);
			if (animIndex != NL3D::CAnimationSet::NotFound)
			{			
				std::string animName = ci->AnimationSet.getAnimationName(animIndex);
				if (!animName.empty())
				{
					currAnims.insert(animName);
				}
			}
		}				
		// check fx that have a trigger anim
		for(uint k = 0; k < _PlayingNodes.size(); ++k)
		{
			if (_PlayingNodes[k])
			{							
				if (!isRunning(_PlayingNodes[k]) || !_PlayingNodes[k]->getPSModel()->hasActiveEmitters())
				{
					// see if chosen anim is currently running
					if (_PlayingNodes[k]->getTriggerAnim().empty() || currAnims.count(_PlayingNodes[k]->getTriggerAnim()))
					{
						// if the fx was shutting down, stop then restart it
						if (!_PlayingNodes[k]->getPSModel()->hasActiveEmitters())
						{
							nlassert(isRunning(_PlayingNodes[k]));
							stop(*_PlayingNodes[k]);
						}
						// yes -> trigger the fx	
						play(*_PlayingNodes[k]);
					}
					else if (!_PlayingNodes[k]->getPSPointer()->hasParticles()) // fx is being shut down, stop it when necessary
					{					
						stop(*_PlayingNodes[k]); // no more particles so stop the system
					}					
				}
				else
				{					
					if (!_PlayingNodes[k]->getTriggerAnim().empty())
					{
						if (_PlayingNodes[k]->getPSModel()->hasActiveEmitters())
						{
							// see if anim if already playing. If this is not the case then shutdown the emitters
							if (!currAnims.count(_PlayingNodes[k]->getTriggerAnim()))
							{
								_PlayingNodes[k]->getPSModel()->activateEmitters(false);
							}
						}
					}					
				}
			}
		}
	}
	if (_ActiveNode)
	{	
		// display number of particles for the currently active node
		if (currNumParticles != _LastCurrNumParticles || maxNumParticles != _LastMaxNumParticles)
		{	
			CString numParts;	
			numParts.LoadString(IDS_NUM_PARTICLES);
			numParts += CString(NLMISC::toString("%d / %d",(int) currNumParticles, (int) maxNumParticles).c_str());
			GetDlgItem(IDC_NUM_PARTICLES)->SetWindowText((LPCTSTR) numParts);
			_LastCurrNumParticles = currNumParticles;
			_LastMaxNumParticles = maxNumParticles;
		}
		// display max number of wanted faces
		NLMISC::CMatrix camMat = ps->getScene()->getCam()->getMatrix();
		sint numWantedFaces = (uint) ps->getWantedNumTris((ps->getSysMat().getPos() - camMat.getPos()).norm());
		if (numWantedFaces != _LastNumWantedFaces)
		{	
			CString numWF;
			numWF.LoadString(IDS_NUM_WANTED_FACES);
			numWF += CString(NLMISC::toString("%d",(int) numWantedFaces).c_str());
			GetDlgItem(IDC_NUM_ASKED_FACES)->SetWindowText((LPCTSTR) numWF);
			_LastNumWantedFaces = numWantedFaces;		
		}
		// display system date
		if (ps->getSystemDate() != _LastSystemDate)
		{
			_LastSystemDate = ps->getSystemDate();
			CString sysDate;	
			sysDate.LoadString(IDS_SYSTEM_DATE);
			sysDate += CString(NLMISC::toString("%.2f s",_LastSystemDate).c_str());
			GetDlgItem(IDC_SYSTEM_DATE)->SetWindowText((LPCTSTR) sysDate);
		}	
	}
	if (_ParticleDlg)
	{
		CParticleWorkspace *pws = _ParticleDlg->getParticleWorkspace();
		if (pws)
		{
			for(uint k = 0; k < pws->getNumNode(); ++k)
			{
				if (pws->getNode(k)->isLoaded())
				{				
					if (pws->getNode(k) == _ActiveNode)
					{
						pws->getNode(k)->getPSModel()->enableDisplayTools(!isRunning(pws->getNode(k)) || m_DisplayHelpers);
					}
					else
					{
						pws->getNode(k)->getPSModel()->enableDisplayTools(false);
					}
					// hide / show the node
					if (_State == RunningMultiple || _State == PausedMultiple)
					{
						if (isRunning(pws->getNode(k)))
						{
							pws->getNode(k)->getPSModel()->show();
						}
						else
						{
							pws->getNode(k)->getPSModel()->hide();
						}
					}
					else
					{
						if (pws->getNode(k) == _ActiveNode)
						{
							pws->getNode(k)->getPSModel()->show();
						}
						else
						{
							pws->getNode(k)->getPSModel()->hide();
						}
					}
				}
			}
		}
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::OnAutoRepeat() 
{
	_AutoRepeat = ((CButton *) GetDlgItem(IDC_AUTOREPEAT))->GetCheck() != 0;
}

//******************************************************************************************************
void CStartStopParticleSystem::OnLinkPlayToScenePlay() 
{	
	// There are 2 play buttons : - one to activate play for the scene animation.
	//                            - one to activate play for the fxs.
	//  When this method is called, the 'play' button of the scene is controlled by the 'play' button of the particle editor
	//  and thus is not accessible.

	UpdateData(TRUE);
	stop();
	_AnimationDLG->setCurrentFrame(_AnimationDLG->Start);
	_LastSceneAnimFrame = _AnimationDLG->Start;
	_AnimationDLG->Playing = false;
	_AnimationDLG->Loop = TRUE;
	_AnimationDLG->UpdateData(FALSE);
	_AnimationDLG->EnableWindow(!m_LinkPlayToScenePlay);	
	GetDlgItem(IDC_ANIM_SPEED)->EnableWindow(!m_LinkPlayToScenePlay);
	if (m_LinkPlayToScenePlay) 
	{
		m_SpeedSliderPos = 100;
		setEllapsedTimeRatio(1.f);
	}
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnLinkToSkeleton() 
{
	if (!_ActiveNode) return;
	CObjectViewer *ov = _ParticleDlg->getObjectViewer();
	if (!ov->isSkeletonPresent())
	{
		CString caption;
		CString mess;
		caption.LoadString(IDS_ERROR);
		mess.LoadString(IDS_NO_SKELETON_IN_SCENE);
		MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_ICONEXCLAMATION);
		return;
	}
	CString chooseBoneForPS;
	chooseBoneForPS.LoadString(IDS_CHOOSE_BONE_FOR_PS);
	NL3D::CSkeletonModel *skel;
	uint boneIndex;
	std::string parentSkelName;
	std::string parentBoneName;
	if (ov->chooseBone(NLMISC::tStrToUtf8(chooseBoneForPS), skel, boneIndex, &parentSkelName, &parentBoneName))
	{
		_ParticleDlg->stickPSToSkeleton(_ActiveNode, skel, boneIndex, parentSkelName, parentBoneName);
	}
	updateUIFromState();
}

//******************************************************************************************************
void CStartStopParticleSystem::OnUnlinkFromSkeleton() 
{
	if (!_ActiveNode) return;
	if (!_ParticleDlg->isPSStickedToSkeleton())
	{
		CString caption;
		CString mess;
		caption.LoadString(IDS_WARNING);
		mess.LoadString(IDS_NOT_STICKED_TO_SKELETON);
		return;
	}
	_ParticleDlg->unstickPSFromSkeleton(_ActiveNode);
	updateUIFromState();
}

//******************************************************************************************************
void CStartStopParticleSystem::setActiveNode(CParticleWorkspace::CNode *activeNode)
{
	if (activeNode == _ActiveNode) return;
	forceActiveNode(activeNode);
}

//******************************************************************************************************
void CStartStopParticleSystem::forceActiveNode(CParticleWorkspace::CNode *activeNode)
{
	UpdateData();
	bool wasRunning = _State == RunningSingle;
	if (wasRunning)
	{	
		stop();
	}		
	_ActiveNode = activeNode;	
	updateUIFromState();
	if (wasRunning && _ActiveNode)
	{
		start();		
	}	
}

//******************************************************************************************************
void CStartStopParticleSystem::setEllapsedTimeRatio(float value)
{
	CParticleWorkspace *pw = _ParticleDlg->getParticleWorkspace();
	if (!pw) return;
	for(uint k = 0; k < pw->getNumNode(); ++k)
	{
		if (pw->getNode(k)->isLoaded())
		{
			pw->getNode(k)->getPSModel()->setEllapsedTimeRatio(value);
		}
	}
}

//******************************************************************************************************
bool CStartStopParticleSystem::checkHasLoop(CParticleWorkspace::CNode &node)
{
	nlassert(node.isLoaded());
	if (!node.getPSPointer()->hasLoop()) return false;	
	MessageBox(CString(node.getFilename().c_str()) + " : " + getStrRsc(IDS_FX_HAS_LOOP), getStrRsc(IDS_WARNING), MB_ICONEXCLAMATION);
	return true;	
}

//******************************************************************************************************
void CStartStopParticleSystem::play(CParticleWorkspace::CNode &node)
{
	if (isRunning(&node)) return;
	// NB : node must be stopped, no check is done
	nlassert(node.isLoaded());	
	// if node not started, start it				
	node.memorizeState();
	// enable the system to take the right date from the scene	
	node.getPSModel()->enableAutoGetEllapsedTime(true);		
	node.getPSPointer()->setSystemDate(0.f);
	node.getPSPointer()->reactivateSound();	
	node.getPSModel()->activateEmitters(true);
	if (node.getPSPointer()->getAutoCountFlag())
	{
		if (node.getResetAutoCountFlag())
		{
			// reset particle size arrays
			node.getPSPointer()->matchArraySize();
		}
		resetAutoCount(&node, false);	
	}		
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_ANIM_SPEED);	
	node.getPSModel()->setEllapsedTimeRatio(m_SpeedSliderPos * 0.01f);
	_ParticleDlg->ParticleTreeCtrl->suppressLocatedInstanceNbItem(node, 0);				
}

//******************************************************************************************************
void CStartStopParticleSystem::unpause(CParticleWorkspace::CNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.getPSModel()->enableAutoGetEllapsedTime(true);
}

//******************************************************************************************************
void CStartStopParticleSystem::pause(CParticleWorkspace::CNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.getPSModel()->enableAutoGetEllapsedTime(false);
	node.getPSModel()->setEllapsedTime(0.f);
}

//******************************************************************************************************
void CStartStopParticleSystem::stop(CParticleWorkspace::CNode &node)
{
	if (!isRunning(&node)) return;
	nlassert(node.isLoaded());
	node.restoreState();
	_ParticleDlg->ParticleTreeCtrl->rebuildLocatedInstance(node);
	node.getPSModel()->enableAutoGetEllapsedTime(false);	
	node.getPSModel()->setEllapsedTime(0.f);	
	node.getPSModel()->activateEmitters(true);	
	node.getPSPointer()->stopSound();	
}

//******************************************************************************************************
bool CStartStopParticleSystem::isRunning(CParticleWorkspace::CNode *node)
{
	nlassert(node);
	return node->isStateMemorized();	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnBrowseAnim() 
{
	/*
	nlassert(_ActiveNode);
	CChooseAnimation ca;
	std::set<std::string> animSet;
	CObjectViewer *ov = _ParticleDlg->getObjectViewer();
	for(uint k = 0; k < ov->getNumInstance(); ++k)
	{
		CInstanceInfo *ii = ov->getInstance(k);
		for(uint l = 0; l < ii->AnimationSet.getNumAnimation(); ++l)
		{
			animSet.insert(ii->AnimationSet.getAnimationName(l));
		}
	}
	std::vector<std::string> animList(animSet.begin(), animSet.end());
	ca.init(animList);
	if (ca.DoModal() == IDOK)
	{
		m_TriggerAnim =ca.getSelectedAnim().c_str();
		_ActiveNode->setTriggerAnim((LPCTSTR) m_TriggerAnim);
		GetDlgItem(IDC_CLEAR_ANIM)->EnableWindow(!_ActiveNode->getTriggerAnim().empty());
	}	
	_ParticleDlg->ParticleTreeCtrl->updateCaption(*_ActiveNode);
	UpdateData(FALSE);
	*/
	nlassert(_ActiveNode);	
	std::set<std::string> animSet;
	CObjectViewer *ov = _ParticleDlg->getObjectViewer();
	for(uint k = 0; k < ov->getNumInstance(); ++k)
	{
		CInstanceInfo *ii = ov->getInstance(k);
		for(uint l = 0; l < ii->AnimationSet.getNumAnimation(); ++l)
		{
			animSet.insert(ii->AnimationSet.getAnimationName(l));
		}
	}
	std::vector<std::string> animList(animSet.begin(), animSet.end());
	CSelectString st(animList, NLMISC::tStrToUtf8(getStrRsc(IDS_SELECT_ANIMATION)), this, false);
	if (st.DoModal() == IDOK && st.Selection != -1)
	{
		m_TriggerAnim = animList[st.Selection].c_str();
		_ActiveNode->setTriggerAnim(NLMISC::tStrToUtf8(m_TriggerAnim));
		GetDlgItem(IDC_CLEAR_ANIM)->EnableWindow(!_ActiveNode->getTriggerAnim().empty());
	}	
	_ParticleDlg->ParticleTreeCtrl->updateCaption(*_ActiveNode);
	UpdateData(FALSE);
}


//******************************************************************************************************
void CStartStopParticleSystem::OnClearAnim() 
{
	// TODO: Add your control notification handler code here
	m_TriggerAnim.Empty();
	_ActiveNode->setTriggerAnim("");	
	_ParticleDlg->ParticleTreeCtrl->updateCaption(*_ActiveNode);
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnRestickAll() 
{
	if (_ParticleDlg->getParticleWorkspace())
	{
		_ParticleDlg->getParticleWorkspace()->restickAllObjects(_ParticleDlg->getObjectViewer());
	}
}
