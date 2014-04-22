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

#ifndef NL_NAME_DLG_H
#define NL_NAME_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "nel/misc/diff_tool.h"

class CNameDlg : public CDialog
{
// Construction
public:
	CNameDlg(CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNameDlg)
	enum { IDD = IDD_NAME };
	CString		m_nameFilter;
	CString		m_assignGn;
	CString		m_assignIg;
	CListBox	m_searchList;
	CListBox	m_idList;
	//}}AFX_DATA

	// list of selected primitives
	std::list<NLLIGO::IPrimitive*>	m_sel;

	// Worksheet : "bot_names.txt"
	STRING_MANAGER::TWorksheet		m_botNames;

	// Worksheet : "title_words_wk.txt"
	STRING_MANAGER::TWorksheet		m_fcts;

	// maps from edit boxes to data
	std::map<int, uint>				m_listToName;
	std::map<int, uint>				m_listToId;

	// data directory
	std::string						m_dataDir;


// Overrides
	//{{AFX_VIRTUAL(CNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void setSelection(std::list<NLLIGO::IPrimitive*> sel);

protected:
	void updateSearchList();
	void updateSelectList();
	void updateAssignBox();
	void checkNewGn();
	void checkAssignBtn();
	void setName(const ucstring &id, const ucstring &gn, const ucstring &ig);
	

	// Generated message map functions
	//{{AFX_MSG(CNameDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeNameFilter();
	afx_msg void OnChangeNameEboxGn();
	afx_msg void OnChangeNameEboxIg();
	afx_msg void OnSelNameSearch();
	afx_msg void OnBtnAssign();
	afx_msg void OnBtnReset();
	afx_msg void OnDblClkId();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // NL_NAME_DLG_H
