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

#if !defined(AFX_SETTINGS_DIALOG_H__585A784F_E3C3_4B1B_9DCA_929001C14056__INCLUDED_)
#define AFX_SETTINGS_DIALOG_H__585A784F_E3C3_4B1B_9DCA_929001C14056__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// settings_dialog.h : header file
//

#include "nel/misc/config_file.h"

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog

class CSettingsDialog : public CDialog
{
// Construction
public:
	CSettingsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDialog)
	enum { IDD = IDD_SETTINGS };
	CString	RootSearchPath;
	UINT	RememberListSize;
	CString	DefaultDfn;
	CString	DefaultType;
	CString	TypeDfnSubDirectory;
	UINT	MaxUndo;
	BOOL	StartExpanded;
	//}}AFX_DATA

	NLMISC::CConfigFile		ConfigFile;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsDialog)
	afx_msg void OnChangeRootSearchPath();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowse();
	afx_msg void OnChangeRememberListSize();
	afx_msg void OnChangeDefaultDfn();
	afx_msg void OnChangeDefaultType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGS_DIALOG_H__585A784F_E3C3_4B1B_9DCA_929001C14056__INCLUDED_)
