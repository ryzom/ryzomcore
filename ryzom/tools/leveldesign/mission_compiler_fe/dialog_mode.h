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

#if !defined(AFX_DIALOG_MODE_H__2662605B_D96D_4739_9FCE_FF639513FEFD__INCLUDED_)
#define AFX_DIALOG_MODE_H__2662605B_D96D_4739_9FCE_FF639513FEFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dialog_mode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogMode dialog

class CDialogMode : public CDialog
{
// Construction
public:
	CDialogMode(CWnd* pParent = NULL);   // standard constructor

	TToolMode	Mode;

// Dialog Data
	//{{AFX_DATA(CDialogMode)
	enum { IDD = IDD_DIALOG_MODE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogMode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogMode)
	afx_msg void OnModeCompile();
	afx_msg void OnModePublish();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOG_MODE_H__2662605B_D96D_4739_9FCE_FF639513FEFD__INCLUDED_)
