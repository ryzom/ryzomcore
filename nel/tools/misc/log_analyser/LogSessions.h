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

#include <vector>

#if !defined(AFX_LOGSESSIONS_H__24A056CC_9FE8_4A90_B3FF_DAF2BF4929F0__INCLUDED_)
#define AFX_LOGSESSIONS_H__24A056CC_9FE8_4A90_B3FF_DAF2BF4929F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogSessions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogSessions dialog

class CLogSessions : public CDialog
{
// Construction
public:
	CLogSessions(CWnd* pParent = NULL);   // standard constructor

	///
	void		addLogSession( const CString& line );

	///
	void		clear() { Sessions.clear(); }

	///
	CString		getStartDate() const;

	std::vector<CString> Sessions;
	std::vector<CString> Filenames;
	
// Dialog Data
	//{{AFX_DATA(CLogSessions)
	enum { IDD = IDD_LOGSESSIONS };
	CListBox	m_ListBox;
	CString	m_StartDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogSessions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLogSessions)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGSESSIONS_H__24A056CC_9FE8_4A90_B3FF_DAF2BF4929F0__INCLUDED_)
