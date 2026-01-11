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

#if !defined(AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_)
#define AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// custom.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Custom dialog

class Custom : public CDialog
{
// Construction
public:
	Custom(CWnd* pParent = NULL);   // standard constructor
	
	void Free(void);
	CComboBox *clist;
	__int64 flag;
	int		mode; //0 : or, 1 : and
	int		bOk;

private:
	int		nButton;
	CButton *buttonList;
	CStatic *staticList;
	CFont font;

// Dialog Data
	//{{AFX_DATA(Custom)
	enum { IDD = IDD_CUSTOM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Custom)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Custom)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_)
