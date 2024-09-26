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

#if !defined(AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_)
#define AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMovieSize.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize dialog

class CSelectMovieSize : public CDialog
{
// Construction
public:
	CSelectMovieSize(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectMovieSize)
	enum { IDD = IDD_SELECT_SIZE };
	UINT	Width;
	UINT	Height;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMovieSize)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMovieSize)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_)
