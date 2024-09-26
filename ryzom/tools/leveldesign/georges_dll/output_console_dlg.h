// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#if !defined(AFX_OUTPUT_CONSOLE_DLG_H__CE12A786_6B9B_41E1_A87E_8E19E6C708AE__INCLUDED_)
#define AFX_OUTPUT_CONSOLE_DLG_H__CE12A786_6B9B_41E1_A87E_8E19E6C708AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// output_console_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COutputConsoleDlg dialog

class COutputConsoleDlg : public CDialog
{
// Construction
public:
	COutputConsoleDlg(CWnd* pParent = NULL);   // standard constructor

	// Output string
	void outputString (const char *message);

	// From CDialog
	void	OnOK () {}
	void	OnCancel ();

// Dialog Data
	//{{AFX_DATA(COutputConsoleDlg)
	enum { IDD = IDD_OUTPUT_CONSOLE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputConsoleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COutputConsoleDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUT_CONSOLE_DLG_H__CE12A786_6B9B_41E1_A87E_8E19E6C708AE__INCLUDED_)
