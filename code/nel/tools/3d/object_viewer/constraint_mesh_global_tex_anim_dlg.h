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



#if !defined(AFX_CONSTRAINT_MESH_GLOBAL_TEX_ANIM_DLG_H__8676F219_18E6_4626_BE10_5A429D1EA0CD__INCLUDED_)
#define AFX_CONSTRAINT_MESH_GLOBAL_TEX_ANIM_DLG_H__8676F219_18E6_4626_BE10_5A429D1EA0CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "edit_ex.h"


namespace NL3D
{
	class CPSConstraintMesh;
}

class CConstraintMeshGlobalTexAnimDlg : public CDialog, public CEditEx::IListener
{
// Construction
public:
	CConstraintMeshGlobalTexAnimDlg(NL3D::CPSConstraintMesh *cm, uint stage, CWnd* pParent = NULL);   // standard constructor


	void init(uint x, uint y, CWnd *pParent);

// Dialog Data
	//{{AFX_DATA(CConstraintMeshGlobalTexAnimDlg)
	enum { IDD = IDD_CONSTRAINT_MESH_GLOBAL_TEX_ANIM_DLG };
	CEditEx	m_VStartCtrl;
	CEditEx	m_UStartCtrl;
	CEditEx	m_UScaleStartCtrl;
	CEditEx	m_WRotSpeedCtrl;
	CEditEx	m_WRotAccelCtrl;
	CEditEx	m_VSpeedCtrl;
	CEditEx	m_VScaleStartCtrl;
	CEditEx	m_VScaleSpeedCtrl;
	CEditEx	m_VScaleAccelCtrl;
	CEditEx	m_UScaleSpeedCtrl;
	CEditEx	m_VAccelCtrl;
	CEditEx	m_UScaleAccel;
	CEditEx	m_UAccelCtrl;
	CEditEx	m_USpeedCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConstraintMeshGlobalTexAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSConstraintMesh *_CM; // the constraint mesh being edited
	uint					 _Stage; // the stage being edited
	// Generated message map functions
	//{{AFX_MSG(CConstraintMeshGlobalTexAnimDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//// inherited from CEditEx::IListener
	virtual void editExValueChanged(CEditEx *ctrl);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONSTRAINT_MESH_GLOBAL_TEX_ANIM_DLG_H__8676F219_18E6_4626_BE10_5A429D1EA0CD__INCLUDED_)
