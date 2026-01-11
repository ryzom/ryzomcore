#if !defined(AFX_MSGDLG_H__E3909EC7_953D_4037_BDEF_6FE0A44CD143__INCLUDED_)
#define AFX_MSGDLG_H__E3909EC7_953D_4037_BDEF_6FE0A44CD143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgDlg.h : header file
//
#include "PictureHlp.h"

/////////////////////////////////////////////////////////////////////////////
// CMsgDlg dialog

class CMsgDlg : public CDialog
{
// Construction
public:
	CMsgDlg(CWnd* pParent = NULL);   // standard constructor
	CMsgDlg(CString csTitle, CString csMsg, CWnd* pParent = NULL);   // standard constructor

private:
	CString		m_csTitle;
	CString		m_csMsg;
	CFont		m_fMsg;
	CFont		m_fTitle;
	CBrush		m_brush;
	CPictureHlp	m_pictBG;
	
// Dialog Data
	//{{AFX_DATA(CMsgDlg)
	enum { IDD = IDD_MSG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMsgDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGDLG_H__E3909EC7_953D_4037_BDEF_6FE0A44CD143__INCLUDED_)
