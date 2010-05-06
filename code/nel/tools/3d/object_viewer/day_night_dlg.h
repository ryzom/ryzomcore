#if !defined(AFX_DAYNIGHTDLG_H__F46D276D_0B8A_4747_AD85_2C702C673F4A__INCLUDED_)
#define AFX_DAYNIGHTDLG_H__F46D276D_0B8A_4747_AD85_2C702C673F4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

class CObjectViewer;


/////////////////////////////////////////////////////////////////////////////
// CDayNightDlg dialog

class CDayNightDlg : public CDialog
{
// Construction
public:
	CDayNightDlg(CObjectViewer *viewer, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDayNightDlg)
	enum { IDD = IDD_DAYNIGHT };
	CScrollBar	m_DayNightCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDayNightDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CObjectViewer *_ObjView;
	// Generated message map functions
	//{{AFX_MSG(CDayNightDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAYNIGHTDLG_H__F46D276D_0B8A_4747_AD85_2C702C673F4A__INCLUDED_)
