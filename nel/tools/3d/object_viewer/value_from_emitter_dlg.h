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

#if !defined(AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_)
#define AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	template <typename T> class CPSAttribMakerMemory ;
}

#include "attrib_dlg.h"
#include "ps_wrapper.h"
#include "nel/3d/ps_attrib_maker.h"


struct IPopupNotify;

/////////////////////////////////////////////////////////////////////////////
// CValueFromEmitterDlg dialog

class CValueFromEmitterDlg : public CDialog
{
// Construction
public:
	CValueFromEmitterDlg(IPopupNotify *pn, CWnd* pParent = NULL);   // standard constructor

	virtual void  init(CWnd *pParent) = 0 ;
	void		  create(CWnd *parent);
// Dialog Data
	//{{AFX_DATA(CValueFromEmitterDlg)
	enum { IDD = IDD_MEMORY_VALUE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueFromEmitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPopupNotify	*_PN;
	// Generated message map functions
	//{{AFX_MSG(CValueFromEmitterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/** construct a dialog that allow to edit a scheme used for initial attribute generation in a particle  
  */
template <class T> class CValueFromEmitterDlgT : public CValueFromEmitterDlg
{
public:		
	CValueFromEmitterDlgT(NL3D::CPSAttribMakerMemory<T> *editedScheme, CAttribDlgT<T> *srcDlg, IPopupNotify *pn, HBITMAP bitmapToDisplay)
		: CValueFromEmitterDlg(pn), _BitmapToDisplay(bitmapToDisplay), _AttrbDlg(srcDlg)
	{
		nlassert(srcDlg);
		_SchemeWrapper.S = editedScheme ;
	}	
	// inherited from CValueFromEmitterDlg
	void init(CWnd *pParent)
	{
		CValueFromEmitterDlg::create(pParent);				
		_AttrbDlg->disableConstantValue() ;
		_AttrbDlg->setWrapper(&_DummyWrapper) ;
		_AttrbDlg->setSchemeWrapper(&_SchemeWrapper) ;
		_AttrbDlg->init(_BitmapToDisplay, 10, 10, this) ;		
	}

	~CValueFromEmitterDlgT()
	{
		if (_AttrbDlg) _AttrbDlg->DestroyWindow() ;
		delete _AttrbDlg ;
	}	

protected:	
	// the bitmap displayed onthe left
	HBITMAP _BitmapToDisplay ;

	/// the dialog that allow us to edit the scheme
	CAttribDlgT<T> *_AttrbDlg ;

	/// a wrapper to edit the scheme (which himself owns a scheme !!)
	struct CSchemeWrapper : public IPSSchemeWrapper<T>
	{
		NL3D::CPSAttribMakerMemory<T> *S ;
		virtual scheme_type *getScheme(void) const { return S->getScheme() ; }
		virtual void setScheme(scheme_type *s) { S->setScheme(s) ; } ;
	} _SchemeWrapper ;

	/// a dummy wrapper for constant value. This shouldn't be called , however
	struct CDummyWrapper : public IPSWrapper<T>
	{
		T get(void) const { nlassert(false) ; return T() ; }
		void set(const T &) { nlassert(false) ; }
	} _DummyWrapper ;
		
	
} ;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.




#endif // !defined(AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_)
