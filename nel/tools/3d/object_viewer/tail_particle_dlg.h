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

#if !defined(AFX_TAIL_PARTICLE_DLG_H__87722D81_F7C8_4837_96F2_96FCE342EF54__INCLUDED_)
#define AFX_TAIL_PARTICLE_DLG_H__87722D81_F7C8_4837_96F2_96FCE342EF54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "particle_workspace.h"

namespace NL3D
{
	struct CPSTailParticle;
}

/////////////////////////////////////////////////////////////////////////////
// CTailParticleDlg dialog

class CTailParticleDlg : public CDialog
{
// Construction
public:
	CTailParticleDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSTailParticle *tp);   // standard constructor

	void init(CWnd *pParent, sint x, sint y);
// Dialog Data
	//{{AFX_DATA(CTailParticleDlg)
	enum { IDD = IDD_TAIL_PARTICLE };
	CComboBox	m_TailShape;
	CButton	m_TailPersistAfterDeathCtrl;
	BOOL	m_TailFade;	
	BOOL	m_TailPersistAfterDeath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTailParticleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the particle being edited
	NL3D::CPSTailParticle	  *_TailParticle;
	CParticleWorkspace::CNode *_Node;

	// Generated message map functions
	//{{AFX_MSG(CTailParticleDlg)
	afx_msg void OnTailFade();	
	afx_msg void OnTailPersistAfterDeath();
	afx_msg void OnSelchangeTailShape();
	afx_msg void OnPaint();
	afx_msg void OnSelchangeRibbonOrientation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TAIL_PARTICLE_DLG_H__87722D81_F7C8_4837_96F2_96FCE342EF54__INCLUDED_)
