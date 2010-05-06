#if !defined(AFX_SCENE_ROT_DLG_H__AE312CB9_13F2_44E6_8AEB_B5D557B91C69__INCLUDED_)
#define AFX_SCENE_ROT_DLG_H__AE312CB9_13F2_44E6_8AEB_B5D557B91C69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// scene_rot_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSceneRotDlg dialog

class CSceneRotDlg : public CDialog
{
// Construction
public:
	CSceneRotDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSceneRotDlg)
	enum { IDD = IDD_SCENE_ROT };
	CString	RotX;
	CString	RotY;
	CString	RotZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneRotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSceneRotDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENE_ROT_DLG_H__AE312CB9_13F2_44E6_8AEB_B5D557B91C69__INCLUDED_)
