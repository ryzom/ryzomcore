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

#if !defined(AFX_UPLOAD_WINDOW_H__B7E0FE64_6036_4E15_A4C8_4C20DBB45C0C__INCLUDED_)
#define AFX_UPLOAD_WINDOW_H__B7E0FE64_6036_4E15_A4C8_4C20DBB45C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// upload_window.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUploadWindow dialog

class CUploadWindow : public CDialog
{
// Construction
public:
	CUploadWindow(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUploadWindow)
	enum { IDD = IDD_DIALOG4 };
	CComboBox	EventCombo;
	CComboBox	ShardCombo;
	CString	Shard;
	CString	Event;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUploadWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUploadWindow)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPLOAD_WINDOW_H__B7E0FE64_6036_4E15_A4C8_4C20DBB45C0C__INCLUDED_)
