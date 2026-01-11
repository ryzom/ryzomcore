#if !defined(AFX_WEBDLG_H__8B645BB5_9C54_48EA_9EA3_24F613895BAF__INCLUDED_)
#define AFX_WEBDLG_H__8B645BB5_9C54_48EA_9EA3_24F613895BAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WebDlg.h : header file
//

#include "WebBrowser2.h"
//#include "LoadingPageDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CWebDlg dialog
class CNel_launcherDlg;

class CWebDlg : public CDialog
{
// Construction
public:
	CWebDlg(CWnd* pParent = NULL);   // standard constructor
	VARIANT ExecuteScript(CString csCode, CString csLanguage);
	VARIANT ExecuteJavascript(CString csCode);
	void OpenUrl(const std::string &url);
	void OpenUrl(CString csUrl);

private:
	CWebBrowser2	m_explore;
	CBrush			m_brushBG;
	CString			m_csUrl;

//	CLoadingPageDlg	m_dlgLoading;

// Dialog Data
	//{{AFX_DATA(CWebDlg)
	enum { IDD = IDD_WEB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWebDlg)
	afx_msg void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	afx_msg void OnDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNavigateComplete2Explorer1(LPDISPATCH pDisp, VARIANT FAR* URL);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEBDLG_H__8B645BB5_9C54_48EA_9EA3_24F613895BAF__INCLUDED_)
