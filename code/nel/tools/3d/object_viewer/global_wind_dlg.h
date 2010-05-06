#if !defined(AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_)
#define AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// global_wind_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGlobalWindDlg dialog

class CGlobalWindDlg : public CDialog
{
// Construction
public:
	CGlobalWindDlg(CObjectViewer *objViewer, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalWindDlg)
	enum { IDD = IDD_GLOBAL_WIND };
	CStatic	StaticPower;
	CSliderCtrl	SliderPower;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalWindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGlobalWindDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSliderGlobalWindPower(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CObjectViewer	*_ObjViewer;

	// update sliders and static according to objViewer wind setup
	void	updateView();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBAL_WIND_DLG_H__E98702D3_6CD0_4B43_B44D_A0663D168D18__INCLUDED_)
