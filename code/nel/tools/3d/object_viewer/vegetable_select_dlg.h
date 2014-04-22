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

#if !defined(AFX_VEGETABLE_SELECT_DLG_H__C47655F0_ED57_428C_A3FB_0EA0E83F5EE6__INCLUDED_)
#define AFX_VEGETABLE_SELECT_DLG_H__C47655F0_ED57_428C_A3FB_0EA0E83F5EE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_select_dlg.h : header file
//


class	CVegetableDlg;

/////////////////////////////////////////////////////////////////////////////
// CVegetableSelectDlg dialog

class CVegetableSelectDlg : public CDialog
{
// Construction
public:
	CVegetableSelectDlg(CVegetableDlg *vegetableDlg, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVegetableSelectDlg)
	enum { IDD = IDD_VEGETABLE_SELECT_OTHER };
	CListBox	VegetableList;
	int		VegetableSelected;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVegetableSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// dlg to get list of vegetagbles.
	CVegetableDlg		*_VegetableDlg;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_SELECT_DLG_H__C47655F0_ED57_428C_A3FB_0EA0E83F5EE6__INCLUDED_)
