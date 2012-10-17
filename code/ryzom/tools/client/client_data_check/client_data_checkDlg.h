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

// client_data_checkDlg.h : header file
//

#if !defined(AFX_CLIENT_DATA_CHECKDLG_H__7790303F_B248_4E03_A038_DBF0A911836C__INCLUDED_)
#define AFX_CLIENT_DATA_CHECKDLG_H__7790303F_B248_4E03_A038_DBF0A911836C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkDlg dialog

class CClient_data_checkDlg : public CDialog
{
// Construction
public:
	CClient_data_checkDlg(class CClient_data_checkApp *appParent, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CClient_data_checkDlg)
	enum { IDD = IDD_CLIENT_DATA_CHECK_DIALOG };
	CButton	CancelButton;
	CListBox	ListBox;
	CString	TextStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClient_data_checkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClient_data_checkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	class CClient_data_checkApp		*_AppParent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_DATA_CHECKDLG_H__7790303F_B248_4E03_A038_DBF0A911836C__INCLUDED_)
