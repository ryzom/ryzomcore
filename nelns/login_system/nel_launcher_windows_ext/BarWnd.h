#if !defined(AFX_BARWND_H__E1377B0D_4F0B_4F4C_8A65_FCB6D90EF963__INCLUDED_)
#define AFX_BARWND_H__E1377B0D_4F0B_4F4C_8A65_FCB6D90EF963__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BarWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBarWnd window

class CBarWnd : public CWnd
{
// Construction
public:
	CBarWnd();

// Attributes
public:
	int	m_iPos;
	int	m_iRange;

// Operations
public:
	void SetRange(int iRange);
	void UpdatePos(int iPos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBarWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBarWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBarWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BARWND_H__E1377B0D_4F0B_4F4C_8A65_FCB6D90EF963__INCLUDED_)
