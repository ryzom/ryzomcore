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



#if !defined(AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_)
#define AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// value_gradient_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg dialog

struct IPopupNotify;

#include "ps_wrapper.h"
#include "attrib_list_box.h"
#include "editable_range.h"
#include "particle_workspace.h"

class CEditAttribDlg;


/** This struct serves as an interface to manage the gradient.
 *  Deriver should provide the following methods.
 */

struct IValueGradientDlgClient
{
	/** must provide a dialog for the edition of one value
	 *  ONLY ONE DIALOG WILL BE QUERRIED AT A TIME. the previous dialog is detroyed before this is called again
	 * \param index the index of the value in the dialog
	 * \grad the dlg that called this method (deriver can ask a redraw then)
	 */
	virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad, CParticleWorkspace::CNode *ownerNode) = 0;

	/// this enumerate the action that we can apply on a gradient
	enum TAction { Add, Insert, Delete, Up, Down };

	/// a function that can add, remove, or insert a new element in the gradient
	virtual void modifyGradient(TAction, uint index) = 0;

	/// a function that can display a value in a gradient, with the given offset
	virtual void displayValue(CDC *dc, uint index, sint x, sint y)  = 0;

	// return the number of values in a scheme
	virtual uint32 getSchemeSize(void) const  = 0;

	// get the number of interpolation step
	virtual uint32 getNbSteps(void) const = 0;

	// set the number of interpolation steps
	virtual void setNbSteps(uint32 value) = 0;

	/// dtor
	~IValueGradientDlgClient() {}
};


class CValueGradientDlg : public CDialog
{

public:


	/** construct the dialog. The user must provides an interface of type IValueGradientDlgClient	
	 * and a pointer to the parent window
	 * \param destroyClientInterface When set to true, dlete will be called on the client interface when this obj dtor is called.
	 * \param canTuneNbStage The gradient is sampled, and the number of intermediate values can be tuned. 
	 *        When this is set to false, this is disabled.
	 * \param minSize The minimum number of elements that the gradient must have.
	 */

	CValueGradientDlg(IValueGradientDlgClient *clientInterface,
					  CParticleWorkspace::CNode *ownerNode,
		              bool destroyClientInterface,
					  CWnd* pParent,
					  IPopupNotify *pn,
					  bool canTuneNbStages = true,
					  uint minSize = 2
					 );

	/// invalidate the gradient list box
	void invalidateGrad(void);

	// non modal display
	void init(CWnd *pParent);

	/// dtor
	~CValueGradientDlg();

// Dialog Data
	//{{AFX_DATA(CValueGradientDlg)
	enum { IDD = IDD_GRADIENT_DLG };
	CStatic	m_NoSamples;
	CAttribListBox	m_GradientList;
	CStatic	m_Value;
	CButton	m_RemoveCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueGradientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the minimum number of element in the gradient
	uint					 _MinSize;
	// false to disable the dialog that control the number of stages between each value
	bool					 _CanTuneNbStages;
	IValueGradientDlgClient *_ClientInterface;
	bool					_DestroyClientInterface;	
	// the dialog for edition of the current value
	CEditAttribDlg			*_EditValueDlg;
	// the dialog to edit the current number of step for gradient interpolation
	CEditableRangeUInt		*_NbStepDlg;
	// the current size of the gradient
	uint					_Size;
	// interface to tells the parent that we have been closed
	IPopupNotify			*_PN;
	// Owner node
	CParticleWorkspace::CNode *_Node;

	// Generated message map functions
	//{{AFX_MSG(CValueGradientDlg)
	afx_msg void OnAddValue();
	afx_msg void OnInsertValue();
	afx_msg void OnRemoveValue();
	afx_msg void OnValueDown();
	afx_msg void OnValueUp();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGradientList();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// a wrapper to tune the number of step
	struct CNbStepWrapper :public IPSWrapperUInt
	{
		// the interface that was passed to the dialog this struct is part of
		IValueGradientDlgClient *I;	
		uint32 get(void) const { return I->getNbSteps(); }
		void set(const uint32 &nbSteps) { I->setNbSteps(nbSteps); }

	} _NbStepWrapper;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_)
