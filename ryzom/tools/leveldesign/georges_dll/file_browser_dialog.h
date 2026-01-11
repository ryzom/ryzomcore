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

#if !defined(AFX_FILE_BROWSER_DIALOG_H__89ACFC97_4AF1_4A4D_A7F7_199D1F9B2BFB__INCLUDED_)
#define AFX_FILE_BROWSER_DIALOG_H__89ACFC97_4AF1_4A4D_A7F7_199D1F9B2BFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// file_browser_dialog.h : header file
//

#include "file_tree_view.h"

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserDialog dialog

class CFileBrowserDialog : public CDialog
{
// Construction
public:
	CFileBrowserDialog(CWnd* pParent = NULL);   // standard constructor
	~CFileBrowserDialog();   // standard constructor

	// Open currently selected document
	void openDocument ();
		
	// From CDialog
	void	OnOK ();
	void	OnCancel ();

	// Refresh the list
	void	refresh ();

	// Set tree sorted by name or type
	void	setSortedByType (bool sortedByType);

// Dialog Data
	//{{AFX_DATA(CFileBrowserDialog)
	enum { IDD = IDD_FILE_BROWSER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowserDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFileTreeCtrl	TreeCtrlType;
	CFileTreeCtrl	TreeCtrlDfn;
	CFileTreeCtrl	TreeCtrlForm;

	// Generated message map functions
	//{{AFX_MSG(CFileBrowserDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickTabFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeTabFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILE_BROWSER_DIALOG_H__89ACFC97_4AF1_4A4D_A7F7_199D1F9B2BFB__INCLUDED_)
