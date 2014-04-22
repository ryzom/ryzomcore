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

#if !defined(AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_)
#define AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeManagerDlg.h : header file
//
#include "main_frm.h"
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg dialog

class CTypeManagerDlg : public CDialog
{

	std::vector<SType>	LocalTypes;

// Construction
public:
	CTypeManagerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTypeManagerDlg)
	enum { IDD = IDD_TYPEMANAGER };
	CListBox	ListType;
	//}}AFX_DATA

	void set (const std::vector<SType> &types);
	const std::vector<SType> get ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeManagerDlg)
	afx_msg void OnAddtype();
	afx_msg void OnEdittype();
	afx_msg void OnRemovetype();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_)
