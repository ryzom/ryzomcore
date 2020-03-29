#if !defined(AFX_ERROR_BOX_DLG_H__B64622BC_239A_44D5_BD12_74E7BE6DE5B4__INCLUDED_)
#define AFX_ERROR_BOX_DLG_H__B64622BC_239A_44D5_BD12_74E7BE6DE5B4__INCLUDED_



#include "nel/misc/ucstring.h"

/////////////////////////////////////////////////////////////////////////////
// CErrorBoxDlg dialog

class CErrorBoxDlg : public CDialog
{
// Construction
public:
	CErrorBoxDlg(const ucstring &errorMsg,const ucstring &resumeMsg, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CErrorBoxDlg)
	enum { IDD = IDD_ERROR_BOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	ucstring _ErrorMsg;
	ucstring _ResumeMsg; //Message displayed in the "Resume Label"
	// Generated message map functions
	//{{AFX_MSG(CErrorBoxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ERROR_BOX_DLG_H__B64622BC_239A_44D5_BD12_74E7BE6DE5B4__INCLUDED_)
