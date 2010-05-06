#if !defined(AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogEditList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogEditList dialog

class CDialogEditList : public CDialog
{
// Construction
public:
	CDialogEditList(CWnd* pParent = NULL);   // standard constructor

	virtual void OnInit ()=0;
	virtual void OnOk ()=0;

// Dialog Data
	//{{AFX_DATA(CDialogEditList)
	enum { IDD = IDD_EDIT_LIST };
	CListBox	m_ctrlList;
	CComboBox	m_ctrlCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogEditList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogEditList)
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGEDITLIST_H__69173FE0_A38B_11D4_9CD4_0050DAC3A412__INCLUDED_)
