#if !defined(AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_)
#define AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// light_group_factor.h : header file
//

#include "color_button.h"

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor dialog

class CLightGroupFactor : public CDialog
{
// Construction
public:
	CLightGroupFactor(CWnd* pParent = NULL);   // standard constructor

	void handle ();

// Dialog Data
	//{{AFX_DATA(CLightGroupFactor)
	enum { IDD = IDD_LIGHT_GROUP_FACTOR };
	CColorButton	ColorStart0;
	CColorButton	ColorStart1;
	CColorButton	ColorStart2;
	CColorButton	ColorEnd0;
	CColorButton	ColorEnd1;
	CColorButton	ColorEnd2;
	CSliderCtrl	LightGroup3Ctrl;
	CSliderCtrl	LightGroup2Ctrl;
	CSliderCtrl	LightGroup1Ctrl;
	int		LightGroup1;
	int		LightGroup2;
	int		LightGroup3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightGroupFactor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLightGroupFactor)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonColorEnd0();
	afx_msg void OnButtonColorEnd1();
	afx_msg void OnButtonColorEnd2();
	afx_msg void OnButtonColorStart0();
	afx_msg void OnButtonColorStart1();
	afx_msg void OnButtonColorStart2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void	chooseColor(CColorButton &colbut);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHT_GROUP_FACTOR_H__7214CE3B_2ADD_4355_A18C_85B112C0D093__INCLUDED_)
