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

#if !defined(AFX_BAR_H__1AEEE6F6_7AE1_49AC_A3CB_576C63140C06__INCLUDED_)
#define AFX_BAR_H__1AEEE6F6_7AE1_49AC_A3CB_576C63140C06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// bar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBar window

class CBar : public CWnd
{
// Construction
public:
	CBar();

// Attributes
public:

	BOOL Create (CRect &rect, CWnd *parent);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBar)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BAR_H__1AEEE6F6_7AE1_49AC_A3CB_576C63140C06__INCLUDED_)
