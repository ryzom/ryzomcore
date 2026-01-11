#if !defined(AFX_CONDITIONSVIEW_H__92023B7C_0AE1_4A06_B272_6CF88976DA52__INCLUDED_)
#define AFX_CONDITIONSVIEW_H__92023B7C_0AE1_4A06_B272_6CF88976DA52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConditionsView.h : header file
//

#include <afxtempl.h> // CMap
#include <afxcview.h> // CTreeView base class

#include "Condition.h"

typedef CArray<HTREEITEM,HTREEITEM&>	HTreeItemArray;
typedef CList<POSITION,POSITION&>		TPositionList;
typedef CMap<HTREEITEM,HTREEITEM&,CConditionNode *, CConditionNode *&> CMapHTreeItemToPtr;



/////////////////////////////////////////////////////////////////////////////
// CConditionsView view

class CConditionsView : public CTreeView
{
protected:
	CConditionsView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CConditionsView)

// Attributes
protected:
	// map HTREEITEM to the adress of the associated CConditionNode object
	CMapHTreeItemToPtr		m_mapItemToNode;

	CCondition		*		m_pSelectedCondition;
	CCondition		*		m_pDragCondition;
	CConditionNode	*		m_pSelectedNode;

	CImageList*				m_pDragImage;
	HTREEITEM				m_hitemDrag,m_hitemDrop;
							
	BOOL					m_bRDragging;
	BOOL					m_bRDragCanceled;

// Operations
public:

	// expand/collapse the tree ctrl
	void expand(UINT = TVE_EXPAND, HTREEITEM = NULL);

	CImageList* CreateDragImageEx(HTREEITEM hItem);

	HMENU GetRDragMenu();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConditionsView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	afx_msg void OnMoveAsChild();
	afx_msg void OnMoveAsSibling();
	afx_msg void OnCopyAsChild();
	afx_msg void OnCopyAsSibling(); 

// Implementation
protected:
	virtual ~CConditionsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CConditionsView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONDITIONSVIEW_H__92023B7C_0AE1_4A06_B272_6CF88976DA52__INCLUDED_)
