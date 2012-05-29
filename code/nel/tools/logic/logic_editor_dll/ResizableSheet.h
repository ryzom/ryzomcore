#if !defined(AFX_RESIZABLESHEET_H__INCLUDED_)
#define AFX_RESIZABLESHEET_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000 by Paolo Messina
// (ppescher@yahoo.com)
//
// Free for non-commercial use.
// You may change the code to your needs,
// provided that credits to the original 
// author is given in the modified files.
//  
/////////////////////////////////////////////////////////////////////////////
// ResizableSheet.h : header file
//

class CResizableSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CResizableSheet)

// Construction
public:
	CResizableSheet();
	CResizableSheet(UINT nIDCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
	CResizableSheet(LPCTSTR pszCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
private:
	// flags
	BOOL m_bEnableSaveRestore;
	BOOL m_bSavePage;

	// internal status
	CString m_sSection;			// section name and
	CString m_sEntry;			// entry for save/restore

	BOOL m_bInitDone;			// if all internal vars initialized

	// layout variables
	SIZE m_szLayoutPage;
	SIZE m_szLayoutTabLine;		// tab control or wizard line
	SIZE m_szLayoutButton[7];	// each index corresponds to a possible button

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizableSheet)
	public:
	virtual BOOL OnInitDialog();
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResizableSheet();

// used internally
private:
	void PresetLayout();
	void Construct();
	void LoadWindowRect();
	void SaveWindowRect();
	void ArrangeLayout();
	
// callable from derived classes
protected:
	void SetMaximizedRect(const CRect& rc);		// set window rect when maximized
	void ResetMaximizedRect();					// reset to default maximized rect
	void EnableSaveRestore(LPCTSTR pszSection,
		LPCTSTR pszEntry, BOOL bWithPage = FALSE);	// section and entry in app's profile
	int GetMinWidth();	// minimum width to display all buttons


// Generated message map functions
protected:
	//{{AFX_MSG(CResizableSheet)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg void OnPageChanged();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// AFX_RESIZABLESHEET_H__INCLUDED_
