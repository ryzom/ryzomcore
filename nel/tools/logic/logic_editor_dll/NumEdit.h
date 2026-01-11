#if !defined(AFX_NUMEDIT_H__3EA072E7_D976_48D5_B9F9_D9553E38E3E5__INCLUDED_)
#define AFX_NUMEDIT_H__3EA072E7_D976_48D5_B9F9_D9553E38E3E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window

class CNumEdit : public CEdit
{
// Construction
public:
	CNumEdit();

	BOOL checkValidity();
// Attributes

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNumEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	LRESULT OnPaste(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMEDIT_H__3EA072E7_D976_48D5_B9F9_D9553E38E3E5__INCLUDED_)
