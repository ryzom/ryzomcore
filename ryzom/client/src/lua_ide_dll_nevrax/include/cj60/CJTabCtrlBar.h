// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// CJTabCtrlBar.h : header file
//
// DevStudio Style Resizable Docking Tab Control Bar.
//
// The code contained in this file is based on the original
// CSizingTabCtrlBar class written by Dirk Clemens,
//		mailto:dirk_clemens@hotmail.com
//		http://www.codeguru.com/docking/sizing_tabctrl_bar.shtml
//
// Copyright (c) 1998-99 Kirk Stowell   
//		mailto:kstowell@codejockeys.com
//		http://www.codejockeys.com/kstowell/
//
// This source code may be used in compiled form in any way you desire. 
// Source file(s) may be redistributed unmodified by any means PROVIDING
// they are not sold for profit without the authors expressed written consent,
// and providing that this notice and the authors name and all copyright
// notices remain intact. If the source code is used in any commercial
// applications then a statement along the lines of:
//
// "Portions Copyright (c) 1998-99 Kirk Stowell" must be included in the
// startup banner, "About" box or printed documentation. An email letting
// me know that you are using it would be nice as well. That's not much to ask
// considering the amount of work that went into this.
//
// This software is provided "as is" without express or implied warranty. Use
// it at your own risk! The author accepts no liability for any damage/loss of
// business that this product may cause.
//
// ==========================================================================  
//
// Acknowledgements:
//	<>  To Dirk Clemens (dirk_clemens@hotmail.com) for his CSizingTabCtrlBar
//		class, which is where the idea for this came from.
//	<>  Many thanks to all of you, who have encouraged me to update my articles
//		and code, and who sent in bug reports and fixes.
//  <>  Many thanks Zafir Anjum (zafir@codeguru.com) for the tremendous job that
//      he has done with codeguru, enough can not be said!
//	<>  Many thanks to Microsoft for making the source code availiable for MFC. 
//		Since most of this work is a modification from existing classes and 
//		methods, this library would not have been possible.
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	17 Oct 1998	- Initial re-write and release.
//          1.01    03 Jan 1999 - Application freezing bug fixed
//                                by LiangYiBin.Donald(mailto:lybd@yahoo.com)
//			1.02	17 Jan 1999 - Added helper class CCJTabCtrl to eliminate
//								  re-painting problems such as when the app
//								  is minimized then restored.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CJTABCTRLBAR_H__
#define __CJTABCTRLBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>
#include "CJControlBar.h"

typedef struct
{
	CWnd *pWnd;
	char szLabel[32];
}TCB_ITEM;

/////////////////////////////////////////////////////////////////////////////
// CCJTabCtrlBar tab control bar

class CCJTabCtrl;
class AFX_EXT_CLASS CCJTabCtrlBar : public CCJControlBar
{
// Construction
public:
	CCJTabCtrlBar();

// Attributes
public:
protected:
	int				m_nActiveTab;
	CFont			m_TabFont;
	CCJTabCtrl*		m_pTabCtrl;
	CToolTipCtrl*	m_pToolTip;
	CView*			m_pActiveView;
	CList <TCB_ITEM *,TCB_ITEM *> m_views;

// Operations
public:
	BOOL AddView(LPCTSTR lpszLabel, CRuntimeClass *pViewClass, CCreateContext *pContext=NULL);
	void RemoveView(int nView);
	void SetActiveView(int nNewTab);
	void SetActiveView(CRuntimeClass *pViewClass);
	CView* GetActiveView();
	CView* GetView(int nView);
	CView* GetView(CRuntimeClass *pViewClass);
	CImageList* SetTabImageList(CImageList *pImageList);
	BOOL ModifyTabStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags);

// Overrides
public:
    // ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCJTabCtrlBar)
	//}}AFX_VIRTUAL
	virtual void GetChildRect(CRect &rect);

// Implementation
public:
	virtual ~CCJTabCtrlBar();

// Generated message map functions
protected:
	//{{AFX_MSG(CCJTabCtrlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult) ;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __CJTABCTRLBAR_H__
