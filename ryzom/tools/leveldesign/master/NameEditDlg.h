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

#ifndef __REGION_PROPERTIES_DLG__
#define __REGION_PROPERTIES_DLG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewRegion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NewRegion dialog

class CNameEditDlg : public CDialog
{
// Construction
public:
	CNameEditDlg (CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewRegion)
	enum { IDD = IDD_NAME_EDIT };
	CString	Title;
	CString	Comment;
	CString	Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewRegion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewRegion)
	virtual BOOL OnInitDialog ();
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __REGION_PROPERTIES_DLG__
