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

#if !defined(AFX_SPLASHSCREEN_H__9CC775DA_1173_415D_AC2C_D3CDF05265F6__INCLUDED_)
#define AFX_SPLASHSCREEN_H__9CC775DA_1173_415D_AC2C_D3CDF05265F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// splash_screen.h : header file
//
#include "resource.h"
#include "nel/misc/progress_callback.h"
/////////////////////////////////////////////////////////////////////////////
// CSplashScreen dialog

class CSplashScreen : public CDialog, public NLMISC::IProgressCallback
{
// Construction
public:
	CSplashScreen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSplashScreen)
	enum { IDD = IDD_SPLASHSCREEN };
	CString	m_load_list;
	//}}AFX_DATA

	virtual void progress (float progressValue);
	void addLine(std::string newLine);
	void close();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplashScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSplashScreen)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLASHSCREEN_H__9CC775DA_1173_415D_AC2C_D3CDF05265F6__INCLUDED_)
