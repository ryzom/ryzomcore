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

#if !defined(AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_)
#define AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// global_wind_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGlobalWindDlg dialog

class CGlobalWindDlg : public CDialog
{
// Construction
public:
	CGlobalWindDlg(CObjectViewer *objViewer, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalWindDlg)
	enum { IDD = IDD_GLOBAL_WIND };
	CStatic	StaticPower;
	CSliderCtrl	SliderPower;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalWindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGlobalWindDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSliderGlobalWindPower(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CObjectViewer	*_ObjViewer;

	// update sliders and static according to objViewer wind setup
	void	updateView();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_)
