#if !defined(AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_)
#define AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_veget_set.h : header file
//

#include <string>

class SelectionTerritoire;

/////////////////////////////////////////////////////////////////////////////
// CChooseVegetSet dialog

class CChooseVegetSet : public CDialog
{
// Construction
public:
	CChooseVegetSet(SelectionTerritoire* pParent, const std::string &oldFile);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseVegetSet)
	enum { IDD = IDD_CHOOSE_VEGET };
	CButton	Name;
	//}}AFX_DATA

	SelectionTerritoire		*Parent;
	std::string				FileName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseVegetSet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseVegetSet)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_VEGET_SET_H__EA817A1B_515D_4FD8_984F_35155195041C__INCLUDED_)
