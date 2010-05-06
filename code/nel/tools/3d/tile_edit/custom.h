#if !defined(AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_)
#define AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// custom.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Custom dialog

class Custom : public CDialog
{
// Construction
public:
	Custom(CWnd* pParent = NULL);   // standard constructor
	
	void Free(void);
	CComboBox *clist;
	__int64 flag;
	int		mode; //0 : or, 1 : and
	int		bOk;

private:
	int		nButton;
	CButton *buttonList;
	CStatic *staticList;
	CFont font;

// Dialog Data
	//{{AFX_DATA(Custom)
	enum { IDD = IDD_CUSTOM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Custom)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Custom)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOM_H__C97BB227_0040_4D31_9B6E_D90892BE6E9C__INCLUDED_)
