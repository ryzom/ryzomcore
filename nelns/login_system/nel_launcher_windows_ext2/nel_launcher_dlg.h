// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_NEL_LAUNCHER_DLG_H__05FFC179_FE66_4135_9F0E_9D5973E8A92C__INCLUDED_)
#define AFX_NEL_LAUNCHER_DLG_H__05FFC179_FE66_4135_9F0E_9D5973E8A92C__INCLUDED_

#include <string>

#include <nel/misc/config_file.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


extern NLMISC::CConfigFile ConfigFile;
extern bool VerboseLog;

extern void quit();

/////////////////////////////////////////////////////////////////////////////
// CNeLLauncherDlg dialog

class CNeLLauncherDlg : public CDialog
{
// Construction
public:
	CNeLLauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CNeLLauncherDlg)
	enum { IDD = IDD_NEL_LAUNCHER_DIALOG };
	CProgressCtrl	PatchProgress;
	CListCtrl	ShardsList;
	CString	Login;
	CString	Password;
	CString	PatchStatus;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeLLauncherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	uint32 SelectedShardId;

	std::string GetRegKeyValue(const std::string &Entry);
	void SetRegKey(const std::string &ValueName, const std::string &Value);
	void ConnectToShard();
	void LaunchClient(const std::string &cookie, const std::string &addr);
	
	// Generated message map functions
	//{{AFX_MSG(CNeLLauncherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLogin();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDblclkShards(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConnect();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	afx_msg void OnSignup();
	afx_msg void OnAtysnews();
	afx_msg void OnPlayguide();
	afx_msg void OnReleasenote();
	afx_msg void OnLivesupport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEL_LAUNCHER_DLG_H__05FFC179_FE66_4135_9F0E_9D5973E8A92C__INCLUDED_)
