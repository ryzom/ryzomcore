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

#if !defined(AFX_FIND_PRIMITIVE_DLG_H__CBD80497_7501_4021_97EB_64ADD1C65146__INCLUDED_)
#define AFX_FIND_PRIMITIVE_DLG_H__CBD80497_7501_4021_97EB_64ADD1C65146__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// find_primitive_dlg.h : header file
//

#include "world_editor_doc.h"

/////////////////////////////////////////////////////////////////////////////
// CFindPrimitiveDlg dialog

class CFindPrimitiveDlg : public CDialog
{
// Construction
public:
	CFindPrimitiveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindPrimitiveDlg)
	enum { IDD = IDD_FIND_PRIMITIVE };
	
	static	CString	Property;
	static	CString	Value;
	static	CString	ReplaceText;
	static	int		SelectionOnly;

	CString	PrimitiveName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindPrimitiveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// The locator
	CDatabaseLocatorPointer	_Locator;
	bool					_End;

	void replace(bool all);

	// Generated message map functions
	//{{AFX_MSG(CFindPrimitiveDlg)
	afx_msg void OnFindNext();
	afx_msg void OnReplace();
	afx_msg void OnReplaceAll();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void SetSelection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIND_PRIMITIVE_DLG_H__CBD80497_7501_4021_97EB_64ADD1C65146__INCLUDED_)
