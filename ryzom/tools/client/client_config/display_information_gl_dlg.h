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

#if !defined(AFX_DISPLAY_INFORMATION_GL_DLG_H__296CBB2A_5856_4C1D_94AD_8509265543E2__INCLUDED_)
#define AFX_DISPLAY_INFORMATION_GL_DLG_H__296CBB2A_5856_4C1D_94AD_8509265543E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// display_information_gl_dlg.h : header file
//
#include "base_dialog.h"

// ***************************************************************************
// CDisplayInformationGlDlg dialog
// ***************************************************************************

class CDisplayInformationGlDlg : public CBaseDialog
{
// Construction
public:
	CDisplayInformationGlDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisplayInformationGlDlg)
	enum { IDD = IDD_DISPLAY_INFO };
	CStatic	VersionCtrl;
	CStatic	VendorCtrl;
	CStatic	RendererCtrl;
	CListBox	Extensions;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayInformationGlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDisplayInformationGlDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAY_INFORMATION_GL_DLG_H__296CBB2A_5856_4C1D_94AD_8509265543E2__INCLUDED_)
