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


#if !defined(AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_)
#define AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "edit_attrib_dlg.h"
#include "ps_wrapper.h"
#include "popup_notify.h"

namespace NL3D
{
	class CPSDirection;
}

/** This dialog helps to choose from several preset directions, or to choose a custom one.
  * This also allow to bind the direction to a global variable when it is supported
  */
class CDirectionAttr : public CEditAttribDlg, public IPopupNotify
{
// Construction
public:	
	// construct the dialog.
	CDirectionAttr(const std::string &id);   // standard constructor

	// inherited from CEditAttribDlg
	virtual void init(uint32 x, uint32 y, CWnd *pParent);
	//
	void setWrapper(IPSWrapper<NLMISC::CVector> *wrapper) { _Wrapper = wrapper; }

	/** The CPSDirection object is used to see if a global variable can be bound to the direction.
	  * When set to NULL it has no effect (the default)
	  */
	void setDirectionWrapper(NL3D::CPSDirection *wrapper) { _DirectionWrapper = wrapper; }	

	BOOL EnableWindow( BOOL bEnable = TRUE );


// Dialog Data
	//{{AFX_DATA(CDirectionAttr)
	enum { IDD = IDD_DIRECTION_ATTR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirectionAttr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	friend class CDirectionEdit; 

	// a dialog that we may create for custom direction edition
	class CDirectionEdit *_DirectionDlg;

	// inherited from IPopupNotify
	void childPopupClosed(CWnd *);
	
	IPSWrapper<NLMISC::CVector> *_Wrapper;
	NL3D::CPSDirection			*_DirectionWrapper;

	// Generated message map functions
	//{{AFX_MSG(CDirectionAttr)
	afx_msg void OnVectI();
	afx_msg void OnVectJ();
	afx_msg void OnVectK();
	afx_msg void OnVectMinusI();
	afx_msg void OnVectMinusJ();
	afx_msg void OnVectMinusK();
	afx_msg void OnCustomDirection();
	afx_msg void OnDestroy();
	afx_msg void OnGlobalDirection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_)
