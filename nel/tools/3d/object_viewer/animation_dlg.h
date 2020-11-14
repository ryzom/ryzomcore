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

#if !defined(AFX_ANIMATION_DLG_H__A9ECE123_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_ANIMATION_DLG_H__A9ECE123_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// animation_dlg.h : header file
//

#include "nel/misc/types_nl.h"
#include "nel/3d/animation_time.h"

/////////////////////////////////////////////////////////////////////////////
// CAnimationDlg dialog

class CAnimationDlg : public CDialog
{
// Construction
public:
	CAnimationDlg(class CObjectViewer * main, CWnd* pParent = NULL);   // standard constructor

	BOOL  EnableWindow(BOOL enable = TRUE);
// Dialog Data
	//{{AFX_DATA(CAnimationDlg)
	enum { IDD = IDD_ANIMATION };
	CButton	FRWCtrl;
	CButton	FFWCtrl;
	CSliderCtrl	TimeLineCtrl;
	CButton	PlayCtrl;
	CButton	StopCtrl;
	float	End;
	float	Speed;
	float	Start;
	BOOL	Loop;
	int		UICurrentFrame;
	BOOL	Inplace;
	BOOL	IncPos;
	//}}AFX_DATA

	bool					Playing;
	uint64					LastTime;
	CObjectViewer*			Main;
	float					CurrentFrame;
	float					LastFrame;

	void setCurrentFrame (float currentFrame);
	void handle ();
	void setAnimTime (float animStart, float animEnd);
	void updateBar ();
	NL3D::TAnimationTime getTime ();
	NL3D::TAnimationTime getLastTime ();
	float getSpeed() { return Speed; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL



// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimationDlg)
	afx_msg void OnEnd();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnChangeCurrentFrame();
	afx_msg void OnChangeEndEdit();
	afx_msg void OnChangeSpeed();
	afx_msg void OnStart();
	afx_msg void OnChangeStartEdit();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATION_DLG_H__A9ECE123_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
