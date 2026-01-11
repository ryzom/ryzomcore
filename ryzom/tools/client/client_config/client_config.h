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

// client_config.h : main header file for the CLIENT_CONFIG application
//

#if !defined(AFX_CLIENT_CONFIG_H__5ABEECBC_CE23_47C4_BD2F_8EBD81F203F3__INCLUDED_)
#define AFX_CLIENT_CONFIG_H__5ABEECBC_CE23_47C4_BD2F_8EBD81F203F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define CONFIG_FILE_NAME "client.cfg"
#define CONFIG_DEFAULT_FILE_NAME "client_default.cfg"

/////////////////////////////////////////////////////////////////////////////
// CClientConfigApp:
// See client_config.cpp for the implementation of this class
//

class CClientConfigApp : public CWinApp
{
public:
	CClientConfigApp();

	// Error message
	void	error (const ucstring &message);
	bool	yesNo (const ucstring &message);

	// Data modified ?
	bool	Modified;
	bool	Localized;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientConfigApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClientConfigApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CClientConfigApp theApp;

// Helper to set text in a window
// fallback to ascii set if the OS doesn't support unicode (windows 95/98/me)
void setWindowText(HWND hwnd, LPCWSTR lpText);



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_CONFIG_H__5ABEECBC_CE23_47C4_BD2F_8EBD81F203F3__INCLUDED_)
