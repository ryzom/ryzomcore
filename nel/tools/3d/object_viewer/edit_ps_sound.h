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


//

#if !defined(AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_)
#define AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ps_wrapper.h"
#include "nel/3d/ps_sound.h"
#include "editable_range.h"
#include "particle_workspace.h"

namespace NLSOUND
{
	class UAudioMixer;
}


class CAttribDlgFloat;


/// particle system sound system initialisation
extern void setPSSoundSystem(NLSOUND::UAudioMixer *am);

/// release the particle system sound system
extern void releasePSSoundSystem(void);






/////////////////////////////////////////////////////////////////////////////
// CEditPSSound dialog

class CEditPSSound : public CDialog
{
// Construction
public:
	CEditPSSound(CParticleWorkspace::CNode *ownerNode, NL3D::CPSSound *sound);   // standard constructor

	~CEditPSSound();


	void init(CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CEditPSSound)
	enum { IDD = IDD_SOUND };
	CString	m_SoundName;
	BOOL	m_Spawn;
	BOOL	m_Mute;
	BOOL	m_KeepOriginalPitch;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPSSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSSound            *_Sound;				// the sound being edited	
	CAttribDlgFloat           *_GainDlg;			// dlg to tune sounds gain
	CAttribDlgFloat		      *_PitchDlg;			// dlg to tune sounds pitch
	CEditableRangeFloat		  *_PercentDlg;		    // dlg to tune the percent of sound emissions
	CParticleWorkspace::CNode *_Node;

	// Generated message map functions
	//{{AFX_MSG(CEditPSSound)
	afx_msg void OnBrowseSound();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSoundName();
	afx_msg void OnSpawn();
	afx_msg void OnPlaySound();
	afx_msg void OnMute();
	afx_msg void OnKeepOriginalPitch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	/////////////////////////////////////////
	// wrapper to set the gain of sounds //
	/////////////////////////////////////////
	struct CGainWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const { return S->getGain(); }
		void set(const float &v) { S->setGain(v); }
		scheme_type *getScheme(void) const { return S->getGainScheme(); }
		void setScheme(scheme_type *s) { S->setGainScheme(s); }
	} _GainWrapper;
	////////////////////////////////////////////
	// wrapper to set the pitch of sounds	  //
	////////////////////////////////////////////
	struct CPitchWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const { return S->getPitch(); }
		void set(const float &v) { S->setPitch(v); }
		scheme_type *getScheme(void) const { return S->getPitchScheme(); }
		void setScheme(scheme_type *s) { S->setPitchScheme(s); }
	} _PitchWrapper;
	//////////////////////////////////////////////////////
	// wrapper to set the percentage of sound emissions //
	//////////////////////////////////////////////////////
	struct CEmissionPercentWrapper : public IPSWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const { return S->getEmissionPercent(); }
		void  set(const float &v) { S->setEmissionPercent(v); }	
	} _EmissionPercentWrapper;

	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_)
