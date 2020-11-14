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

#if !defined(AFX_ENTER_NAME_H__A736DB78_36B8_49D8_AAB9_FEF984F5E96B__INCLUDED_)
#define AFX_ENTER_NAME_H__A736DB78_36B8_49D8_AAB9_FEF984F5E96B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gus_mfc_select_with_text.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CGusMfcSelectWithText dialog

class CGusMfcSelectWithText : public CDialog
{
// Construction
public:
	CGusMfcSelectWithText(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGusMfcSelectWithText)
	enum { IDD = IDD_DIALOG1 };
	CComboBox	Combo;
	CString	Text;
	CString	ComboValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGusMfcSelectWithText)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGusMfcSelectWithText)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTER_NAME_H__A736DB78_36B8_49D8_AAB9_FEF984F5E96B__INCLUDED_)
