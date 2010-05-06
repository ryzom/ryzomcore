#if !defined(AFX_SELECT_ROTATION_H__BDAD4C40_FD02_11D4_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SELECT_ROTATION_H__BDAD4C40_FD02_11D4_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// select_rotation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SelectRotation dialog

class SelectRotation : public CDialog
{
// Construction
public:
	SelectRotation(CWnd* pParent = NULL);   // standard constructor

	int	RotSelected;
// Dialog Data
	//{{AFX_DATA(SelectRotation)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectRotation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SelectRotation)
	afx_msg void OnRot0();
	afx_msg void OnRot1();
	afx_msg void OnRot2();
	afx_msg void OnRot3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECT_ROTATION_H__BDAD4C40_FD02_11D4_9CD4_0050DAC3A412__INCLUDED_)
