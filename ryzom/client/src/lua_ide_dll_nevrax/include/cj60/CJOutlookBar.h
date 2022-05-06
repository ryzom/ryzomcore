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

// CJOutlookBar.h : header file
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
//	<>  Thanks to Shekar Narayanan (ShekarNarayanan@Hotmail.com) for his article
//		'A Cool Looking Menu For Easier Navigation' which is where I got the idea
//		for this class from.
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
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CJOUTLOOKBAR_H__
#define __CJOUTLOOKBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// CContentItems - Container class for menu items.

class AFX_EXT_CLASS CContentItems
{
protected:
	UINT m_nImageID;
	CString m_csText;

public:
	CContentItems( UINT nID, CString str ) :
		m_nImageID( nID ), m_csText( str )
	{
	}

	void operator = ( CContentItems& pItems )
	{
		m_nImageID = pItems.m_nImageID;
		m_csText = pItems.m_csText;
	}

	UINT GetImageID() { return m_nImageID; }
	CString GetText() { return m_csText;  }
};

#define WM_OUTLOOKBAR_NOTIFY	(WM_USER + 1)
#define OBM_ITEMCLICK			1

/////////////////////////////////////////////////////////////////////////////
// CJOutlookBar class

class AFX_EXT_CLASS CCJOutlookBar : public CListBox
{
	DECLARE_DYNAMIC(CCJOutlookBar)

// Construction
public:
	CCJOutlookBar();

// Attributes
public:
	CWnd*			p_Owner;
    CPoint          m_point;
	CImageList		m_ImageListNormal;
	CImageList		m_ImageList;
	CContentItems*  m_pContents;
    int             m_nNumItems;
	int             m_nIndex;
    bool            m_bHilight;
    bool            m_bLBDown;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCJOutlookBar)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetItems( CContentItems* pItems, int nNumItems );
	void SetOwner( CWnd* pWnd) { p_Owner = pWnd; }
	virtual ~CCJOutlookBar();

// Generated message map functions
protected:
	//{{AFX_MSG(CCJOutlookBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __CJOUTLOOKBAR_H__

