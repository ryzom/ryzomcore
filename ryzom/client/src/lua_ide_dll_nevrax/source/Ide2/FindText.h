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

#if !defined(AFX_FINDTEXT_H__3EADBB50_16D1_4818_B42B_2BD1841DE9C5__INCLUDED_)
#define AFX_FINDTEXT_H__3EADBB50_16D1_4818_B42B_2BD1841DE9C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindText.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindText dialog

class CFindText : public CDialog
{
// Construction
public:
	CFindText(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindText)
	enum { IDD = IDD_FINDTEXT };
	CEdit	m_TextToFindCtrl;
	BOOL	m_MatchCase;
	BOOL	m_WholeWord;
	BOOL	m_RegExp;
	CString	m_TextToFind;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindText)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindText)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDTEXT_H__3EADBB50_16D1_4818_B42B_2BD1841DE9C5__INCLUDED_)
