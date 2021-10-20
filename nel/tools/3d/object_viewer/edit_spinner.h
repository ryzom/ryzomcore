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


#if !defined(AFX_EDIT_SPINNER_H__CFDBE30D_F8C9_432E_9F17_0AFACE143BB0__INCLUDED_)
#define AFX_EDIT_SPINNER_H__CFDBE30D_F8C9_432E_9F17_0AFACE143BB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

struct IPopupNotify;

#include "ps_wrapper.h"
#include "nel/3d/ps_plane_basis_maker.h"
#include "editable_range.h"
#include "particle_workspace.h"

class CDirectionAttr;

class CEditSpinner : public CDialog
{
// Construction
public:
	CEditSpinner(NL3D::CPSBasisSpinner *sf, CParticleWorkspace::CNode *ownerNode, CWnd *pParent, IPopupNotify *pn);

	// dtor
	~CEditSpinner();

	/// init this dialog
	void init(CWnd *pParent);

// Dialog Data
	//{{AFX_DATA(CEditSpinner)
	enum { IDD = IDD_EDITSPINNER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditSpinner)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSBasisSpinner     *_Spinner; // the spinner being edited
	CDirectionAttr			  *_DirDlg;
	CEditableRangeUInt		  *_NbSamplesDlg;
	IPopupNotify			  *_PN;
	CParticleWorkspace::CNode *_Node;

	// Generated message map functions
	//{{AFX_MSG(CEditSpinner)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG

	// wrapper to set the number of samples in the spinner
	struct CNbSampleWrapper : public IPSWrapperUInt
	{
		NL3D::CPSBasisSpinner     *S;
		uint32 get(void) const { return S->_F.getNumSamples(); }
		void set(const uint32 &val) { S->_F.setNumSamples(val); }
	} _NbSampleWrapper;


	// wrapper to set the axis of the spinner
	struct CAxisWrapper : public IPSWrapper<NLMISC::CVector>
	{
		NL3D::CPSBasisSpinner     *S;
		NLMISC::CVector get(void) const { return S->_F.getAxis(); }
		void set(const NLMISC::CVector &axis) { S->_F.setAxis(axis); }
	} _AxisWrapper;
	

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_SPINNER_H__CFDBE30D_F8C9_432E_9F17_0AFACE143BB0__INCLUDED_)
