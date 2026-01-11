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

#if !defined(AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_)
#define AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DicSplashScreen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDicSplashScreen dialog

class CDicSplashScreen : public CDialog
{
// Construction
public:
	CDicSplashScreen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDicSplashScreen)
	enum { IDD = IDD_SplashScreen };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDicSplashScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDicSplashScreen)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DICSPLASHSCREEN_H__7DF0565F_D1BE_4FBA_B8E9_6DDDEEBBAF0E__INCLUDED_)
