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

// client_configDlg.h : header file
//

#if !defined(AFX_CLIENT_CONFIGDLG_H__B16BC0AD_3679_4AFF_BC6C_3AB03CBC2948__INCLUDED_)
#define AFX_CLIENT_CONFIGDLG_H__B16BC0AD_3679_4AFF_BC6C_3AB03CBC2948__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bar.h"
#include "database.h"
#include "general_dlg.h"
#include "display_dlg.h"
#include "display_details_dlg.h"
#include "display_advanced_dlg.h"
#include "system_information_dlg.h"
#include "display_information_gl_dlg.h"
#include "display_information_d3d_dlg.h"
#include "sound_dlg.h"

// ***************************************************************************
// CClient_configDlg dialog
// ***************************************************************************

enum
{
	TreeId = 255,
};

// ***************************************************************************

class CClient_configDlg : public CDialog
{
// Construction
public:
	CClient_configDlg(CWnd* pParent = NULL);	// standard constructor

	// Bitmaps
	CBitmap		Bitmaps[BitmapCount];

	// Fonts
	CFont		BarFont;

	// Controls
	CStatic		Left;
	CStatic		Icon;
	CBar		Top;
	CBar		Bottom;
	CTreeCtrl	Tree;

	// Dialogs
	CDialog		*Dialogs[PageCount];
	CGeneralDlg	GeneralDlg;
	CDisplayDlg	DisplayDlg;
		CDisplayDetailsDlg	DisplayDetailsDlg;
		CDisplayAdvancedDlg	DisplayAdvancedDlg;
	CSoundDlg	SoundDlg;
	CSystemInformationDlg	SystemInformationDlg;
		CDisplayInformationGlDlg	DisplayInformationGLDlg;
		CDisplayInformationD3DDlg	DisplayInformationD3DDlg;

	// Large label top bar string
	ucstring	TopLargeLabel;

	// Methods
	void InvalidateBar ();

	// Overrided
	virtual 
	BOOL UpdateData ( BOOL bSaveAndValidate );

	void setPage (uint pageId);
	void translateTree ();
	void changeLanguage (const char *language);

// Dialog Data
	//{{AFX_DATA(CClient_configDlg)
	enum { IDD = IDD_CLIENT_CONFIG_DIALOG };
	CButton	ApplyCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClient_configDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	uint	_CurrentPage;

	// Hwnd to stringId map
	std::map<HWND, std::string>		_HwndMap;

	// Generated message map functions
	//{{AFX_MSG(CClient_configDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnApply();
	afx_msg void OnDefault();
	afx_msg void OnLaunch();
	//}}AFX_MSG
public:
	virtual void OnCancel();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_CONFIGDLG_H__B16BC0AD_3679_4AFF_BC6C_3AB03CBC2948__INCLUDED_)
