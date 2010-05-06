#if !defined(AFX_DIALOG_PROGRESS_H__868D678B_085A_4D6F_B547_FC103152419D__INCLUDED_)
#define AFX_DIALOG_PROGRESS_H__868D678B_085A_4D6F_B547_FC103152419D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dialog_progress.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogProgress dialog

class CDialogProgress : public CDialog
{
// Construction
public:
	CDialogProgress(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDialogProgress)
	enum { IDD = IDD_DIALOG_PROGRESS };
	CStatic	ProgressText;
	CProgressCtrl	ProgressBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogProgress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogProgress)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOG_PROGRESS_H__868D678B_085A_4D6F_B547_FC103152419D__INCLUDED_)
