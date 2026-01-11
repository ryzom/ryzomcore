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

#if !defined(AFX_PLUGINSELECTOR_H__3BA56BCC_F700_4C1E_9C56_E34561C8DB61__INCLUDED_)
#define AFX_PLUGINSELECTOR_H__3BA56BCC_F700_4C1E_9C56_E34561C8DB61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlugInSelector.h : header file
//

#include <vector>
#include <string>

typedef std::string (*TInfoFunc) (void);
typedef bool (*TAnalyseFunc)( const std::vector<const char *>&, std::string&, std::string& );


/////////////////////////////////////////////////////////////////////////////
// CPlugInSelector dialog

class CPlugInSelector : public CDialog
{
// Construction
public:
	CPlugInSelector(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlugInSelector)
	enum { IDD = IDD_PLUGINSELECTOR_DIALOG };
	CListBox	m_PlugInListBox;
	//}}AFX_DATA


	void		setPluginList( const std::vector<CString>& cont ) { Dlls = &cont; }

	const std::vector<CString>	*Dlls;
	HINSTANCE					LibInst;
	TAnalyseFunc				AnalyseFunc;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlugInSelector)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlugInSelector)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList1();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINSELECTOR_H__3BA56BCC_F700_4C1E_9C56_E34561C8DB61__INCLUDED_)
