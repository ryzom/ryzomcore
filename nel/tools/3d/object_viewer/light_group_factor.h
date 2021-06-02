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

#if !defined(AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_)
#define AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// light_group_factor.h : header file
//

#include "color_button.h"

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor dialog

class CLightGroupFactor : public CDialog
{
// Construction
public:
	CLightGroupFactor(CWnd* pParent = NULL);   // standard constructor

	void handle ();

// Dialog Data
	//{{AFX_DATA(CLightGroupFactor)
	enum { IDD = IDD_LIGHT_GROUP_FACTOR };
	CColorButton	ColorStart0;
	CColorButton	ColorStart1;
	CColorButton	ColorStart2;
	CColorButton	ColorEnd0;
	CColorButton	ColorEnd1;
	CColorButton	ColorEnd2;
	CSliderCtrl	LightGroup3Ctrl;
	CSliderCtrl	LightGroup2Ctrl;
	CSliderCtrl	LightGroup1Ctrl;
	int		LightGroup1;
	int		LightGroup2;
	int		LightGroup3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightGroupFactor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLightGroupFactor)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonColorEnd0();
	afx_msg void OnButtonColorEnd1();
	afx_msg void OnButtonColorEnd2();
	afx_msg void OnButtonColorStart0();
	afx_msg void OnButtonColorStart1();
	afx_msg void OnButtonColorStart2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void	chooseColor(CColorButton &colbut);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_)
