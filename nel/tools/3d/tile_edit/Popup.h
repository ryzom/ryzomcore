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

#if !defined(AFX_POPUP_H__13727C3D_1734_422A_B9CC_60330EDB7A4C__INCLUDED_)
#define AFX_POPUP_H__13727C3D_1734_422A_B9CC_60330EDB7A4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Popup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Popup window

class Popup : public CWnd
{
// Construction
public:
	Popup();
	virtual LRESULT DefWindowProc(UINT message,WPARAM wParam,LPARAM lParam);
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Popup)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~Popup();

	// Generated message map functions
protected:
	//{{AFX_MSG(Popup)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUP_H__13727C3D_1734_422A_B9CC_60330EDB7A4C__INCLUDED_)
