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

#if !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
#define AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "object_viewer.h"
#include "animation_dlg.h"
#include "ps_initial_pos.h"
#include "particle_workspace.h"
//
#include "nel/misc/vector.h"
#include "nel/misc/smart_ptr.h"
//
namespace NL3D
{
	class CParticleSystem;
	class CPSLocated;
	class CPSLocatedBindable;
	struct IPSMover;
}
class CParticleDlg;
class CAnimationDlg;


/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog

class CStartStopParticleSystem : public CDialog, public CObjectViewer::IMainLoopCallBack
{
// Construction
public:
	// state of the dialog
	enum TState { Stopped, RunningSingle, RunningMultiple, PausedSingle, PausedMultiple };
	CStartStopParticleSystem(CParticleDlg *particleDlg, CAnimationDlg *animationDLG);   // standard constructor
	// dtor
	~CStartStopParticleSystem();
	// Set currently active node of the workspace.
	void setActiveNode(CParticleWorkspace::CNode *activeNode);
	// Get current state
	TState getState() const { return _State; }
	/// return true if one or several system are being played
	bool isRunning() const { return _State == RunningSingle || _State == RunningMultiple; }
	/** Return true if a system is paused.
	  * Must call only if running
	  */
	bool isPaused() const 
	{ 			
		return 	_State == PausedSingle || _State == PausedMultiple;
	}
	/// force the playing systems to stop
	void stop();
	/// force the active system to start
	void start();	
	/// start all systems
	void startMultiple();
	/// toggle between start and stop
	void toggle();	
	// pause the playing systems
	void pause();
	/// This remove any memorized instance from the system
	// void reset();
	/// return true when the display bbox button is pressed...
	bool isBBoxDisplayEnabled();
	// enable / disbale auto-count
	void enableAutoCount(bool enable);	
	// reset the autocount the next time the system will be started	
	void resetAutoCount(CParticleWorkspace::CNode *node, bool reset = true);		
// Dialog Data
	//{{AFX_DATA(CStartStopParticleSystem)
	enum { IDD = IDD_PARTICLE_SYSTEM_START_STOP };
	CButton	m_StartMultiplePicture;
	CButton	m_PausePicture;
	CButton	m_StopPicture;
	CButton	m_StartPicture;
	BOOL	m_DisplayBBox;	
	int		m_SpeedSliderPos;
	BOOL	m_DisplayHelpers;
	BOOL	m_LinkPlayToScenePlay;
	CString	m_TriggerAnim;
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartStopParticleSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:

	// Generated message map functions
	//{{AFX_MSG(CStartStopParticleSystem)
	virtual BOOL OnInitDialog();
	afx_msg void OnStartSystem();
	afx_msg void OnStopSystem();
	afx_msg void OnPause();
	afx_msg void OnReleasedcaptureAnimSpeed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDisplayHelpers();
	afx_msg void OnEnableAutoCount();
	afx_msg void OnResetCount();
	afx_msg void OnAutoRepeat();
	afx_msg void OnLinkPlayToScenePlay();
	afx_msg void OnLinkToSkeleton();
	afx_msg void OnUnlinkFromSkeleton();
	afx_msg void OnStartMultipleSystem();
	afx_msg void OnBrowseAnim();
	afx_msg void OnClearAnim();
	afx_msg void OnRestickAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// current state
	TState _State;	
	// Last number of particle that was displayed (keep this to avoid flickering)
	sint _LastCurrNumParticles;
	sint _LastMaxNumParticles;
	sint _LastNumWantedFaces;
	// last displayed date for the system
	float _LastSystemDate;
	//
	bool _AutoRepeat;
	//
	float _LastSceneAnimFrame;

	// the dialog that own this dialog
	CParticleDlg  *_ParticleDlg;
	CAnimationDlg *_AnimationDLG;	
	//CPSInitialPos _SystemInitialPos;
	CParticleWorkspace::CNode *_ActiveNode;

	// list of fxs that are currently playing
	typedef std::vector<NLMISC::CRefPtr<CParticleWorkspace::CNode> > TNodeVect;
	TNodeVect _PlayingNodes;
	// set of of fx that are being paused
	
private:
	void setSpeedSliderValue(float value);
	// From CObjectViewer::IMainLoopCallBack : update display of number of particles
	virtual void goPreRender();
	virtual void goPostRender() {}
	NL3D::CParticleSystem      *getCurrPS() const { return _ActiveNode ? _ActiveNode->getPSPointer() : NULL; }
	NL3D::CParticleSystemModel *getCurrPSModel() const { return _ActiveNode ? _ActiveNode->getPSModel() : NULL; }
	// change speed of all ps
	void setEllapsedTimeRatio(float value);
	void forceActiveNode(CParticleWorkspace::CNode *activeNode);
	/** helper : play a node and update tree
	  * NB : this doesn't update the '_PlayingNodes' list
	  * NB : no check is done on the state
	  * NB : the ps must have no loops
	  * \return True if the node could be played. May not be the case if the node has loops
	  */
	void play(CParticleWorkspace::CNode &node);
	void unpause(CParticleWorkspace::CNode &node);
	void pause(CParticleWorkspace::CNode &node);
	/** helper : stop a node, and update tree
	  * NB : this doesn't update the '_PlayingNodes' list
	  * NB : no check is done on the state
	  */
	void stop(CParticleWorkspace::CNode &node);
	// update ui to reflect the current state
	void updateUIFromState();
	// check is a node has loops
	bool checkHasLoop(CParticleWorkspace::CNode &node);
	// check if a node is inserted in the running list (it may be paused)
	bool isRunning(CParticleWorkspace::CNode *node);
	//
	void restartAllFX();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
