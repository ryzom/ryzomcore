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

#if !defined(AFX_COLLISION_ZONE_DLG_H__45AFAC35_D67B_40AB_85B6_7A472B7C4B30__INCLUDED_)
#define AFX_COLLISION_ZONE_DLG_H__45AFAC35_D67B_40AB_85B6_7A472B7C4B30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif
namespace NL3D
{
	class CPSZone ;
}

#include "ps_wrapper.h"
#include "dialog_stack.h"
#include "editable_range.h"
#include "particle_workspace.h"
//
#include "nel/3d/ps_zone.h"


class CParticleDlg;

/////////////////////////////////////////////////////////////////////////////
// CCollisionZoneDlg dialog

class CCollisionZoneDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CCollisionZoneDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSZone *zone, CParticleDlg *particleDlg) ;   // standard constructor

	void init(sint x, sint y, CWnd *pParent) ;
// Dialog Data
	//{{AFX_DATA(CCollisionZoneDlg)
	enum { IDD = IDD_ZONE };
	CComboBox	m_CollisionBehaviour;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCollisionZoneDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CParticleWorkspace::CNode *_Node;
	CParticleDlg              *_ParticleDlg;
	// the collision zone being edited
	NL3D::CPSZone             *_Zone ;
	// edition of the bounce factor
	CEditableRangeFloat       *_BounceFactorDlg ;
	// Generated message map functions
	//{{AFX_MSG(CCollisionZoneDlg)
	afx_msg void OnSelchangeCollisionBehaviour();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// wrapper to set the bounce factor of a collision zone
	struct CBounceFactorWrapper : public IPSWrapperFloat
	{
		// the zone being wrapped
		NL3D::CPSZone *Z ;
		float get(void) const { return Z->getBounceFactor() ; }
		void set(const float &v) { Z->setBounceFactor(v) ; }
	} _BounceFactorWrapper ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLLISION_ZONE_DLG_H__45AFAC35_D67B_40AB_85B6_7A472B7C4B30__INCLUDED_)
