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

#if !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
#define AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 


namespace NL3D
{
	class CEvent3dMouseListener ;
}

namespace NLMISC
{
	class CVector ;
}

#include "nel/3d/ps_edit.h"
#include "ps_wrapper.h"
#include "editable_range.h"
#include "particle_workspace.h"


class CDirectionAttr ;
class CParticleDlg;

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg dialog

class CPSMoverDlg : public CDialog
{
// Construction
public:
	// construct the object with a pointer to the item being edited, and to the mouse listener to update its model matrix
	CPSMoverDlg(CParticleWorkspace::CNode *ownerNode, CWnd *parent, NL3D::CEvent3dMouseListener *ml,  NL3D::CPSLocated *editedLocated, uint32 editedLocatedIndex);   // standard constructor

	// dtor
	~CPSMoverDlg() ;

// Dialog Data
	//{{AFX_DATA(CPSMoverDlg)
	enum { IDD = IDD_PS_MOVER };
	CListBox	m_SubComponentCtrl;
	CString	m_X;
	CString	m_Y;
	CString	m_Z;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPSMoverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	// position has to be updated (for mouse edition)
	void updatePosition(void) ;


	void init(CParticleDlg	*parent) ;

	// get the current moving interface, or NULL, if the selected object has no IPSMover interface
	NL3D::IPSMover *getMoverInterface(void)  ;

	// get the located being edited
	NL3D::CPSLocated *getLocated(void) { return _EditedLocated ; }
	const NL3D::CPSLocated *getLocated(void) const { return _EditedLocated ; }

	// get the index of the current edited item
	uint32 getLocatedIndex(void) const { return _EditedLocatedIndex ; }

	// ghet the current located bindable being edited, or null
	NL3D::CPSLocatedBindable *getLocatedBindable(void) ;
	

// Implementation
protected:
	CParticleWorkspace::CNode *_Node;
	CParticleDlg			  *_ParticleDlg;
	NL3D::CPSLocated		  *_EditedLocated ;
	uint32 _EditedLocatedIndex ;
	NL3D::CEvent3dMouseListener *_MouseListener ;


	CEditableRangeFloat *_Scale, *_XScale, *_YScale, *_ZScale ;

	CStatic  *_ScaleText, *_XScaleText, *_YScaleText, *_ZScaleText ;

	CDirectionAttr *_DirectionDlg ;

	// this generate control for scaling
	void createScaleControls(void) ;

	// delete scale controls if presents
	void cleanScaleCtrl(void) ;


	// Generated message map functions
	//{{AFX_MSG(CPSMoverDlg)
	afx_msg void OnUpdateXpos();
	afx_msg void OnUpdateYpos();
	afx_msg void OnUpdateZpos();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSubComponent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	// wrappers to scale objects
	struct CUniformScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).x ; }
		void set(const float &v) { M->setScale(Index, v) ; }
	} _UniformScaleWrapper ;

	/// wrapper to scale the X coordinate
	struct CXScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).x ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(s, v.y, v.z)) ; 
		}
	} _XScaleWrapper ;

	/// wrapper to scale the Y coordinate
	struct CYScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).y ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, s, v.z) ) ; 
		}
	} _YScaleWrapper ;

	/// wrapper to scale the Z coordinate
	struct CZScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).z ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, v.y, s) ) ; 
		}
	} _ZScaleWrapper ;

	/// wrapper for direction
	struct CDirectionWrapper : public IPSWrapper<NLMISC::CVector>
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		NLMISC::CVector get(void) const { return M->getNormal(Index) ; }
		void set(const NLMISC::CVector &v) { M->setNormal(Index, v) ; }


	} _DirectionWrapper ;
	 

	// update the mouse listener position when the user entered a value with the keyboard
	void CPSMoverDlg::updateListener(void) ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
