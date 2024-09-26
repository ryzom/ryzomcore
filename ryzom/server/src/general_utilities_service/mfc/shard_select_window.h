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

#if !defined(AFX_SHARD_SELECT_WINDOW_H__C3A4EE4F_3A1C_4894_8078_810C8468B0A3__INCLUDED_)
#define AFX_SHARD_SELECT_WINDOW_H__C3A4EE4F_3A1C_4894_8078_810C8468B0A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// shard_select_window.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShardSelectWindow dialog

class CShardSelectWindow : public CDialog
{
// Construction
public:
	CShardSelectWindow(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShardSelectWindow)
	enum { IDD = IDD_DIALOG5 };
	CComboBox	ShardNameCombo;
	CString	ShardName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShardSelectWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShardSelectWindow)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHARD_SELECT_WINDOW_H__C3A4EE4F_3A1C_4894_8078_810C8468B0A3__INCLUDED_)
