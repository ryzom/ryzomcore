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

#if !defined(AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_)
#define AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeDlg.h : header file
//
#include "resource.h"
#include "color_button.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg dialog

class CTypeDlg : public CDialog
{
// Construction
public:
	CTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTypeDlg)
	enum { IDD = IDD_TYPE };
	CColorButton ButtonColor;

	CRGBA ButtonColorValue;
	CString	EditName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeDlg)
	afx_msg void OnButtoncolor();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_)
