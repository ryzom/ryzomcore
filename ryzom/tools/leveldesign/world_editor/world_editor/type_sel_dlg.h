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

#if !defined(AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_)
#define AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeSelDlg.h : header file
//

#include "main_frm.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg dialog

class CTypeSelDlg : public CDialog
{
// Construction
public:

	CTypeSelDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data

	std::vector<SType> *_TypesInit;
	std::string _TypeSelected;

	//{{AFX_DATA(CTypeSelDlg)
	enum { IDD = IDD_TYPESEL };
	CListBox	TypeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeSelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeSelDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_)
