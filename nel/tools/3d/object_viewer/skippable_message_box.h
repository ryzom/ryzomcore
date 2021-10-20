// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_SKIPPABLE_MESSAGE_BOX_H__3311E633_C1A9_4994_9054_94094A8219B5__INCLUDED_)
#define AFX_SKIPPABLE_MESSAGE_BOX_H__3311E633_C1A9_4994_9054_94094A8219B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// skippable_message_box.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSkippableMessageBox dialog


// A skippabmle message box with user contenyt & caption
// The user can check an option to not see the dialog again
// this can be querried by calling 'getBypassFlag'
class CSkippableMessageBox : public CDialog
{
// Construction
public:
	CSkippableMessageBox(const CString &caption, const CString &content, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSkippableMessageBox)
	enum { IDD = IDD_SKIPPABLE_MESSAGE_BOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Test whether the user has checked the 'don't show again' check box
	bool getBypassFlag() const { return _BypassFlag; }	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkippableMessageBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool	_BypassFlag;
	CString _Caption;
	CString _Content;
	// Generated message map functions
	//{{AFX_MSG(CSkippableMessageBox)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKIPPABLE_MESSAGE_BOX_H__3311E633_C1A9_4994_9054_94094A8219B5__INCLUDED_)
