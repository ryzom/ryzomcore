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

#if !defined(AFX_SYSTEM_INFORMATION_DLG_H__1640EAF7_D7B9_4A7D_9F07_B4F5D943D22D__INCLUDED_)
#define AFX_SYSTEM_INFORMATION_DLG_H__1640EAF7_D7B9_4A7D_9F07_B4F5D943D22D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// system_information_dlg.h : header file
//

#include "base_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemInformationDlg dialog

class CSystemInformationDlg : public CBaseDialog
{
// Construction
public:
	CSystemInformationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSystemInformationDlg)
	enum { IDD = IDD_SYSTEM_INFO };
	CStatic	VideoCtrl;
	CStatic	PhysicalMemoryCtrl;
	CStatic	VideoDriverCtrl;
	CStatic	OSCtrl;
	CStatic	CPUCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemInformationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemInformationDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEM_INFORMATION_DLG_H__1640EAF7_D7B9_4A7D_9F07_B4F5D943D22D__INCLUDED_)
