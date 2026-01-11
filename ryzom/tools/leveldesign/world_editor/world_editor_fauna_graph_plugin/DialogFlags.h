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

#if !defined(AFX_DIALOGFLAGS_H__96559C1A_5580_443D_9FB0_94555B12EC28__INCLUDED_)
#define AFX_DIALOGFLAGS_H__96559C1A_5580_443D_9FB0_94555B12EC28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogFlags.h : header file
//


class CPlugin;
/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog

class CDialogFlags : public CDialog
{
// Construction
public:
	CDialogFlags(CPlugin *plugin);   // standard constructor

	enum TDisplayCondition
	{ 
		DisplayAll = 0,
		DisplayWhenSelected,
		DisplayOff
	};

	TDisplayCondition DisplayCondition;
		
// Dialog Data
	//{{AFX_DATA(CDialogFlags)
	enum { IDD = IDD_DIALOG_FLAGS };
	BOOL	m_DisplayFlags;
	BOOL	m_DisplayIndices;
	BOOL	m_DisplayTargetIndices;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPlugin  *_Plugin;

	// Generated message map functions
	//{{AFX_MSG(CDialogFlags)
	afx_msg void OnSelchangeDisplayCondition();
	afx_msg void OnDisplayFlags();
	afx_msg void OnDisplayIndices();
	afx_msg void OnDisplayTargetIndices();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGFLAGS_H__96559C1A_5580_443D_9FB0_94555B12EC28__INCLUDED_)
