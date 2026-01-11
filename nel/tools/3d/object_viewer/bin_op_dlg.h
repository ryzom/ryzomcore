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


#if !defined(AFX_BIN_OP_DLG_H__1EF9C3C2_6D2B_4E4D_817E_1CA094EBF287__INCLUDED_)
#define AFX_BIN_OP_DLG_H__1EF9C3C2_6D2B_4E4D_817E_1CA094EBF287__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "attrib_dlg.h"
#include "nel/3d/ps_attrib_maker_bin_op.h"

class CBinOpDlg : public CDialog
{
// Construction
public:
	CBinOpDlg(IPopupNotify *pn, CWnd* pParent = NULL);   // standard constructor

	virtual void  init(CWnd *pParent) = 0 ;

	void create(CWnd *pParent);

	// called when a new operator has been selected 
	virtual void newOp(uint32 op) = 0 ;
	
// Dialog Data
	//{{AFX_DATA(CBinOpDlg)
	enum { IDD = IDD_BIN_OP };
	CComboBox	m_BinOp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBinOpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPopupNotify *_PN;

	// Generated message map functions
	//{{AFX_MSG(CBinOpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBinOp();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/** construct a dialog that allow to edit a binary operator that produce argument of a particle system  
  */
template <class T> class CBinOpDlgT : public CBinOpDlg
{
public:
	/// ctruct the given dialog from the given scheme that owns memory
	CBinOpDlgT(NL3D::CPSAttribMakerBinOp<T> *editedScheme, CAttribDlgT<T> **attrbDlg, IPopupNotify *pn, HBITMAP bitmapToDisplay)
		:  CBinOpDlg(pn), _BitmapToDisplay(bitmapToDisplay), _EditedScheme(editedScheme)
	{
		for (uint k = 0 ; k < 2 ; ++k)
		{
			nlassert(attrbDlg);
			_AttrbDlg[k] = attrbDlg[k];
			_SchemeWrapper[k].S = _EditedScheme ;
			_SchemeWrapper[k].Index =  k ;
		}
	}

	/// call this before performing DoModal and the like...
	void init(CWnd *pParent)
	{
		CBinOpDlg::create(pParent);
		uint k ;
		for (k = 0 ; k < 2 ; ++k)
		{			
			_AttrbDlg[k]->disableConstantValue() ;
			_AttrbDlg[k]->setWrapper(&_DummyWrapper) ;
			_AttrbDlg[k]->setSchemeWrapper(&_SchemeWrapper[k]) ;
			_AttrbDlg[k]->init(_BitmapToDisplay, 10, 10 + k * 155, this) ;
		}

		// init the operator list box
		static const TCHAR *ops[] = { _T("selectArg1"), _T("selectArg2"), _T("modulate"), _T("add"), _T("subtract") };
		for (k = 0 ; k < (uint) NL3D::CPSBinOp::last ; ++k)
		{
			// SchemeWrapper[k].S reference the atriobute maker being edited
			if 	(_EditedScheme->supportOp( (NL3D::CPSBinOp::BinOp) k))
			{
				sint index = m_BinOp.AddString(ops[k]);
				m_BinOp.SetItemData(index, k);
				if ((uint) _EditedScheme->getOp() == k)
				{
					m_BinOp.SetCurSel(k) ;
				}
			}
		}
		UpdateData(FALSE) ;
	}

	~CBinOpDlgT()
	{
		for (uint k = 0 ; k < 2 ; ++k)
		{
			if (_AttrbDlg[k]) _AttrbDlg[k]->DestroyWindow() ;
			delete _AttrbDlg[k] ;
		}
	}	

protected:
	
	NL3D::CPSAttribMakerBinOp<T> *_EditedScheme ;
	

	// the bitmap displayed onthe left
	HBITMAP _BitmapToDisplay ;

	/// the dialogs that allow us to edit the schemes
	CAttribDlgT<T> *_AttrbDlg[2] ;

	/// a wrapper to edit the scheme (which himself owns a scheme !!)
	struct CSchemeWrapper : public IPSSchemeWrapper<T>
	{
		NL3D::CPSAttribMakerBinOp<T> *S ;
		uint Index ;
		virtual scheme_type *getScheme(void) const { return S->getArg(Index) ; }
		virtual void setScheme(scheme_type *s) { S->setArg(Index, s) ; } ;
	} _SchemeWrapper[2] ;

	/// a dummy wrapper for constant value. This shouldn't be called , however
	struct CDummyWrapper : public IPSWrapper<T>
	{
		T get(void) const { nlassert(false) ; return T() ; }
		void set(const T &) { nlassert(false) ; }
	} _DummyWrapper ;
		

	void newOp(uint32 op)
	{
		nlassert(_EditedScheme) ;
		_EditedScheme->setOp((NL3D::CPSBinOp::BinOp) op) ;
	}
	
} ;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIN_OP_DLG_H__1EF9C3C2_6D2B_4E4D_817E_1CA094EBF287__INCLUDED_)
