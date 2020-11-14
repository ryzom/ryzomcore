#if !defined(AFX_TEXTURED_PROGRESS_CTRL_H__F8A8A5F1_C0E6_40B1_88FF_1CA46491C338__INCLUDED_)
#define AFX_TEXTURED_PROGRESS_CTRL_H__F8A8A5F1_C0E6_40B1_88FF_1CA46491C338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// textured_progress_ctrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTexturedProgressCtrl window

class CTexturedProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CTexturedProgressCtrl();
	/** Init progress ctrl background & foreground bitmaps
	  * NB : The original bitmap handle are detached from 'bg' & 'fg'
	  */
	void init(CBitmap &bg, CBitmap &fg);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturedProgressCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTexturedProgressCtrl();


	CBitmap _BG, _FG;
	bool    _Initialized;

	// Generated message map functions
protected:
	//{{AFX_MSG(CTexturedProgressCtrl)
	afx_msg void OnPaint();	
	afx_msg void OnNcPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURED_PROGRESS_CTRL_H__F8A8A5F1_C0E6_40B1_88FF_1CA46491C338__INCLUDED_)
