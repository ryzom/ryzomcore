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


#if !defined(AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_)
#define AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

using NLMISC::CRGBA ;




/////////////////////////////////////////////////////////////////////////////
// CColorButton window

class CColorButton : public CButton
{

// Construction
public:
	CColorButton();

// Attributes
public:
	// set a nex color for the button
	void setColor(CRGBA col) { _Color = col ; /*Invalidate() ;*/ }

	// get the color of the button
	CRGBA getColor(void) const { return _Color ; }


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorButton();

	// register this custom control to window
	static void CColorButton::registerClass(void) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


	// current color of the button
	CRGBA _Color ;


	// the event proc for basic cbutton...
	static WNDPROC _BasicButtonWndProc ;

	// a hook to create the dialog
	static LRESULT CALLBACK EXPORT WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) ;


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_)
