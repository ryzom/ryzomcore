#if !defined(AFX_GETVAL_H__DD545A1E_2DA6_4D2B_A698_79FA197F9931__INCLUDED_)
#define AFX_GETVAL_H__DD545A1E_2DA6_4D2B_A698_79FA197F9931__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetVal.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// GetVal dialog


class GetVal : public CDialog
{
// Construction
public:
	char *name;
	int NameOk;
	GetVal(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GetVal)
	enum { IDD = IDD_GETSTR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GetVal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GetVal)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETVAL_H__DD545A1E_2DA6_4D2B_A698_79FA197F9931__INCLUDED_)
