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

// bug_reportDlg.h : header file
//

#if !defined(AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_)
#define AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBug_reportDlg dialog

class CBug_reportDlg : public CDialog
{
// Construction
public:
	CBug_reportDlg(CWnd* pParent = NULL);	// standard constructor

	std::string get (int nID);
	void		set (int nID, const std::string &val, bool enable = true);
	void		setFocus (int nID);
	void		setDumpFilename (const std::string &dump);

	std::string askEmail ();
	std::string checkEmail (const std::string &email);
	void createConfigFile ();

	void sendReport (bool withScreenshot);

	std::string	SMTPServer, ToEmail;

// Dialog Data
	//{{AFX_DATA(CBug_reportDlg)
	enum { IDD = IDD_BUG_REPORT_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBug_reportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBug_reportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSendreport();
	afx_msg void OnClear();
	afx_msg void OnAttachfile();
	afx_msg void OnNoattach();
	afx_msg void OnSendreportwithss();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_)
