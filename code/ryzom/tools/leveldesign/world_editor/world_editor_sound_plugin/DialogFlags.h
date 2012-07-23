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

#if !defined(AFX_DAILOGFLAGS_H__0D9E26EE_5A7F_4CFA_AED0_4470B8F0E3EC__INCLUDED_)
#define AFX_DAILOGFLAGS_H__0D9E26EE_5A7F_4CFA_AED0_4470B8F0E3EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailogFlags.h : header file
//

#include "stdafx.h"
#include "resource.h"
#include "nel/sound/u_audio_mixer.h"


class CSoundPlugin;

/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog

class CDialogFlags : public CDialog
{
// Construction
public:
	CDialogFlags(NLSOUND::UAudioMixer *mixer, CWnd* pParent = NULL);   // standard constructor

	void	init(CSoundPlugin *plugin);
	void	lostPositionControl();

	void	setMixer(NLSOUND::UAudioMixer *mixer)	{	_Mixer = mixer;}

//	uint	BgControlId[32];

	std::vector<std::pair<std::string, uint> >	SampleBanks;

	uint	ScrollPos;


// Dialog Data
	//{{AFX_DATA(CDialogFlags)
	enum { IDD = IDD_DIALOG_FLAGS };
	CListCtrl	_SbList;
	CButton	CheckMoveEar;
	CButton	BtnPlayStop;
	CString	SampleSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL


// Implementation
	afx_msg void OnClose();
protected:


	CSoundPlugin							*_Plugin;
	NLSOUND::UAudioMixer					*_Mixer;
	NLSOUND::UAudioMixer::TBackgroundFlags	_Flags;
	
	// Generated message map functions
	//{{AFX_MSG(CDialogFlags)
	afx_msg void OnButtonPlayStop();
	afx_msg void OnCheckMoveEar();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnReload();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILOGFLAGS_H__0D9E26EE_5A7F_4CFA_AED0_4470B8F0E3EC__INCLUDED_)
