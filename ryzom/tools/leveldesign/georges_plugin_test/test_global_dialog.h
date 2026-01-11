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

#if !defined(AFX_TEST_GLOBAL_DIALOG_H__7979954A_B56B_46BB_BD90_F69FD2EE52F7__INCLUDED_)
#define AFX_TEST_GLOBAL_DIALOG_H__7979954A_B56B_46BB_BD90_F69FD2EE52F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// test_global_dialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestGlobalDialog dialog

class CTestGlobalDialog : public CDialog
{
// Construction
public:
	CTestGlobalDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTestGlobalDialog)
	enum { IDD = IDD_TEST_GLOBAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	class MyPlugin	*Plugin;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestGlobalDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestGlobalDialog)
	afx_msg void OnCreateDoc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEST_GLOBAL_DIALOG_H__7979954A_B56B_46BB_BD90_F69FD2EE52F7__INCLUDED_)
