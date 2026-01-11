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

#if !defined(AFX_DISPLAY_INFORMATION_D3D_DLG_H__90FE554C_8224_4A68_94CE_65D71FED52F8__INCLUDED_)
#define AFX_DISPLAY_INFORMATION_D3D_DLG_H__90FE554C_8224_4A68_94CE_65D71FED52F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// display_information_d3d_dlg.h : header file
//

#include "base_dialog.h"

/////////////////////////////////////////////////////////////////////////////
// CDisplayInformationD3DDlg dialog

class CDisplayInformationD3DDlg : public CBaseDialog
{
// Construction
public:
	CDisplayInformationD3DDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisplayInformationD3DDlg)
	enum { IDD = IDD_DISPLAY_D3D };
	CStatic	DriverVersion;
	CStatic	Driver;
	CStatic	Description;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayInformationD3DDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDisplayInformationD3DDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAY_INFORMATION_D3D_DLG_H__90FE554C_8224_4A68_94CE_65D71FED52F8__INCLUDED_)
