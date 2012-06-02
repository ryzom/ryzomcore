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

#if !defined(AFX_SOUND_ANIM_DLG_H__0D6C3D3A_BCBB_4EEA_8E35_27EAD9500A1C__INCLUDED_)
#define AFX_SOUND_ANIM_DLG_H__0D6C3D3A_BCBB_4EEA_8E35_27EAD9500A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "sound_anim_view.h"
#include "resource.h"

class CObjectViewer;
class CAnimationDlg;

namespace NLSOUND
{
	class CSoundAnimation;
	class CSoundAnimMarker;
}

class CSoundAnimDlg : public CDialog
{
// Construction
public:
	CSoundAnimDlg(CObjectViewer* objView, CAnimationDlg* animDlg, CWnd* pParent = NULL); 

	void						setAnimTime(float animStart, float animEnd);
	void						handle();
	void						refresh(BOOL update)	{ _AnimView.refresh(update); }
	void						updateScroll(uint pos, uint min, uint max);
	void						selectMarker(NLSOUND::CSoundAnimMarker* marker);

protected:

	void						updateSounds();

	CObjectViewer				*_ObjView;
	CAnimationDlg				*_AnimationDlg;
	CSoundAnimView				_AnimView;
	NLSOUND::CSoundAnimMarker	*_SelectedMarker;

public:

	// VC++ wizardry

// Dialog Data
	//{{AFX_DATA(CSoundAnimDlg)
	enum { IDD = IDD_SOUND_ANIM_DLG };
	//}}AFX_DATA


// Overrides
	//{{AFX_VIRTUAL(CSoundAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSoundAnimDlg)
	afx_msg void OnAddSound();
	afx_msg void OnRemoveSound();
	afx_msg void OnSave();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnMark();
	afx_msg void OnDelete();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUND_ANIM_DLG_H__0D6C3D3A_BCBB_4EEA_8E35_27EAD9500A1C__INCLUDED_)
