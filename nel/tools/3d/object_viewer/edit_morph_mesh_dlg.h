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

#if !defined(AFX_EDIT_MORPH_MESH_DLG_H__62813786_A4E1_47E1_9EEF_4D169F270483__INCLUDED_)
#define AFX_EDIT_MORPH_MESH_DLG_H__62813786_A4E1_47E1_9EEF_4D169F270483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSConstraintMesh;
}

#include "ps_wrapper.h"
#include "dialog_stack.h"
#include "particle_workspace.h"

struct IPopupNotify;
class  CParticleDlg;
/////////////////////////////////////////////////////////////////////////////
// CEditMorphMeshDlg dialog

class CEditMorphMeshDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CEditMorphMeshDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSConstraintMesh *cm, CWnd* pParent, CParticleDlg  *particleDlg, IPopupNotify *pn = NULL);   // standard constructor

	

	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CEditMorphMeshDlg)
	enum { IDD = IDD_EDIT_MORPH_MESH };
	CListBox	m_MeshList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMorphMeshDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	

protected:
	CParticleWorkspace::CNode *_Node;
	NL3D::CPSConstraintMesh   *_CM; // the constraint mesh being edited
	IPopupNotify			  *_PN; // a window to notify when this dialog is destroyed
	CParticleDlg			  *_ParticleDlg;

	/// open a file dialog to get the mesh name
	bool getShapeNameFromDlg(std::string &name);

	/// fill the mesh list with the mesh names in the object being edited
	void updateMeshList();

	// update dialog msg to say that mesh are incompatibles
	void updateValidFlag();
	
	std::string getShapeDescStr(uint shapeIndex, sint numVerts) const;
	

	// Generated message map functions
	//{{AFX_MSG(CEditMorphMeshDlg)
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnChange();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnInsert();
	afx_msg void OnUp();
	afx_msg void OnDown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	/// wrapper for the morph scheme
	struct CMorphSchemeWrapper : IPSSchemeWrapperFloat, IPSWrapperFloat
	{
		NL3D::CPSConstraintMesh *CM;
		virtual float get(void) const;
		virtual void set(const float &);
		virtual scheme_type *getScheme(void) const;
		virtual void setScheme(scheme_type *s);
	} _MorphSchemeWrapper;
	void touchPSState();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_MORPH_MESH_DLG_H__62813786_A4E1_47E1_9EEF_4D169F270483__INCLUDED_)
