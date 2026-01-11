#if !defined(AFX_PROGRESSDLG_H__D403A82B_08B3_46C2_964B_910B249761FE__INCLUDED_)
#define AFX_PROGRESSDLG_H__D403A82B_08B3_46C2_964B_910B249761FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//

#include "BarWnd.h"
#include "PictureHlp.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
// Construction
public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	void Show(BOOL bShow = TRUE);
	void SetRange(int iRange);
	void UpdatePos(int iPos);
	void UpdateMsg(CString csMsg);

// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_PROGRESS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

private:
	CBarWnd		m_wndBar;
	CBrush		m_brushBG;
	CFont		m_font;
	int			m_iClr;
	CPictureHlp	m_pictBG;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__D403A82B_08B3_46C2_964B_910B249761FE__INCLUDED_)
