#if !defined(AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_)
#define AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMovieSize.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectMovieSize dialog

class CSelectMovieSize : public CDialog
{
// Construction
public:
	CSelectMovieSize(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectMovieSize)
	enum { IDD = IDD_SELECT_SIZE };
	UINT	Width;
	UINT	Height;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMovieSize)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMovieSize)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMOVIESIZE_H__14091B63_19A8_44D4_8EB2_0CD15A4AFD8D__INCLUDED_)
