#if !defined(AFX_BARTABSWND_H__98F09036_59B0_4A99_B4B8_4F726BF8AF68__INCLUDED_)
#define AFX_BARTABSWND_H__98F09036_59B0_4A99_B4B8_4F726BF8AF68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BarTabsWnd.h : header file
//
#define TAB_W	60
#define TAB_H	25

/////////////////////////////////////////////////////////////////////////////
// CBarTabsWnd window
class CTabsObserver
{
public:
	CTabsObserver()	{}

	virtual void OnTab(int iTab)	{}
};

class CBarTabsWnd : public CWnd
{
public:
	CBarTabsWnd();
	void	AddTab(int iID, int iIDFocus);
	int		GetNbTabs();
	void	SetFocusPos(int iTab);
	int		GetFocusPos();
	void	Move(int iX, int iY);
	void	SetObserver(CTabsObserver* pobs);
	void	Reset();

// Attributes
private:
	int				m_iTab;
	CTabsObserver*	m_pobs;
	CPtrArray		m_parrBmp;
	CPtrArray		m_parrBmpFocus;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBarTabsWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBarTabsWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBarTabsWnd)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BARTABSWND_H__98F09036_59B0_4A99_B4B8_4F726BF8AF68__INCLUDED_)
