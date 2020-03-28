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

#if !defined(AFX_TEST_LOCAL_DIALOG_H__D6FE10F8_A2E7_4166_AD85_D8C05B599E2A__INCLUDED_)
#define AFX_TEST_LOCAL_DIALOG_H__D6FE10F8_A2E7_4166_AD85_D8C05B599E2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// test_local_dialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestLocalDialog dialog

class CTestLocalDialog : public CDialog
{
// Construction
public:
	CTestLocalDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTestLocalDialog)
	enum { IDD = IDD_TEST_LOCAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	class MyDocumentPlugin	*Plugin;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestLocalDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestLocalDialog)
	afx_msg void OnRefresh();
	afx_msg void OnSet();
	afx_msg void OnSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEST_LOCAL_DIALOG_H__D6FE10F8_A2E7_4166_AD85_D8C05B599E2A__INCLUDED_)
