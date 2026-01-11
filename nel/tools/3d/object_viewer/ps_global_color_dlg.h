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


#if !defined(AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_)
#define AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "dialog_stack.h"
#include "ps_wrapper.h"
#include "particle_workspace.h"

namespace NL3D
{
	class CParticleSystem;
}

struct IPopupNotify;

class CPSGlobalColorDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CPSGlobalColorDlg(CParticleWorkspace::CNode *ownerNode, IPopupNotify *pn, CWnd* pParent = NULL);   // standard constructor

	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CPSGlobalColorDlg)
	enum { IDD = IDD_GLOBAL_COLOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPSGlobalColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CParticleWorkspace::CNode *_Node;	
	IPopupNotify			  *_PN;
	// Generated message map functions
	//{{AFX_MSG(CPSGlobalColorDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	struct CGlobalColorWrapper : public IPSSchemeWrapperRGBA
	{
		NL3D::CParticleSystem *PS;
		virtual scheme_type *getScheme(void) const;
		virtual void setScheme(scheme_type *s);
	}
	_GlobalColorWrapper;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_)
